#include "ch.h"
#include "hal.h"
#include "battery.h"

BatteryConfig_t *BatteryConfig;

BatteryState_t BatteryState = BATTERY_RELAXED;

float StateOfCharge = 0.0f;

uint16_t CalibCounter = 0;

uint16_t ChargeTimeLeftMin = 0;

uint16_t SecCounter = 0;

float multiMap(float val, float* _in, float* _out, uint16_t size)
{
  // take care the value is within range
  // val = constrain(val, _in[0], _in[size-1]);
  if (val <= _in[0]) return _out[0];
  if (val >= _in[size-1]) return _out[size-1];

  // search right interval
  uint8_t pos = 1;  // _in[0] allready tested
  while(val > _in[pos]) pos++;

  // this will handle all exact "points" in the _in array
  if (val == _in[pos]) return _out[pos];

  // interpolate in the right segment for the rest
  return (val - _in[pos-1]) * (_out[pos] - _out[pos-1]) / (_in[pos] - _in[pos-1]) + _out[pos-1];
}

void CalibrateSOC(float voltage){
  float val = multiMap(voltage, &BatteryConfig->LookUpVoltageTable[0], &BatteryConfig->LookUpPercentageTable[0], 11);
  StateOfCharge = val / 100;
}

static THD_WORKING_AREA(waBattManager, 128);
static THD_FUNCTION(BattManager, arg) {
  systime_t time;                   

  (void)arg;
  chRegSetThreadName("BattManager");

  float BattCurrent = 0.0f;
  float ChgVoltage = 0.0f;
  float BattVoltage = BatteryConfig->GetBatteryVoltage();

  CalibrateSOC(BattVoltage);

  /* Reader thread loop.*/
  time = chVTGetSystemTime();
  while (TRUE) {

  	BattCurrent = BatteryConfig->GetBatteryCurrent();
  	ChgVoltage = BatteryConfig->GetChargeVoltage();
  	BattVoltage = BatteryConfig->GetBatteryVoltage();

  	if(ChgVoltage <= BatteryConfig->MinChgVoltage){
	  	
	  	if(BattCurrent > BatteryConfig->MinCurrent)
	  		BatteryState = BATTERY_DISCHARGE;
	  	else
	  		BatteryState = BATTERY_RELAXED;

  	}

  	if(ChgVoltage > BatteryConfig->MinChgVoltage && BatteryState != BATTERY_CHARGE){
  		BatteryState = BATTERY_BEGIN_CHRAGE;
  	}


  	switch (BatteryState){
  		case BATTERY_RELAXED:
  			CalibCounter++;
  			if(CalibCounter > BatteryConfig->MinTimeBeforecalib){
  				//Calibration process
          CalibrateSOC(BattVoltage);
  				CalibCounter = 0;
  			}



  		break;
  		case BATTERY_DISCHARGE:
  			CalibCounter = 0;

  		break;
  		case BATTERY_BEGIN_CHRAGE:
  			SecCounter = 0;
  			ChargeTimeLeftMin = (uint16_t)((1.0f - StateOfCharge) * ((float)BatteryConfig->TimeToFullChargeInMin * (1 / (ChgVoltage / 10.0f)))); 	//Calculate charge left time
  			BatteryState = BATTERY_CHARGE;
        CalibCounter = BatteryConfig->MinTimeBeforecalib;

  		break;
  		case BATTERY_CHARGE:
  			SecCounter++;
  			if(SecCounter > 600){
  				SecCounter = 0;
  				if(ChargeTimeLeftMin != 0)
  					ChargeTimeLeftMin--;
  			}

  			if(BattVoltage > BatteryConfig->ChgReadyVoltage){
  				BatteryState = BATTERY_CHARGE_FINISHED;
  			}

  		break;
  		case BATTERY_CHARGE_FINISHED:
  			SecCounter = 0;
        CalibrateSOC(BattVoltage);

  		break;
  	}



    /*
     *  Timing period is 10 ms
     */
    chThdSleepUntil(time += TIME_MS2I(100));
  }
}


BatteryInit_t InitBatteryManagement(BatteryConfig_t *_config){

	BatteryConfig = _config;

	chThdCreateStatic(waBattManager, sizeof(waBattManager), NORMALPRIO, BattManager, NULL);

	return BATTERY_OK;

}


BatteryState_t GetBatteryState(void){
	return BatteryState;
}

uint16_t GetChargeTimeLeftMin(void){
	return ChargeTimeLeftMin;
}

uint8_t GetStateOfCharge(void){
  return (uint8_t)(StateOfCharge * 100);
}

float* GetSOCptr(void){
  return &StateOfCharge;
}