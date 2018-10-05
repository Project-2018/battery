#ifndef BATTERY_HPP
#define BATTERY_HPP

#define CHGVOLTAGE_STABLE_LIMIT	0.2f

typedef enum{
	BATTERY_DISCHARGE		= 0,
	BATTERY_RELAXED			= 1,
	BATTERY_BEGIN_CHRAGE	= 2,
	BATTERY_CHARGE 			= 3,
	BATTERY_CHARGE_FINISHED	= 4
}BatteryState_t;

typedef enum{
	BATTERY_OK,
	BATTERY_ERROR
}BatteryInit_t;

typedef struct{
	float LookUpVoltageTable[11];
	float LookUpPercentageTable[11];
	float MinCurrent;
	float MinChgVoltage;
	float ChgReadyVoltage;
	uint16_t MinTimeBeforecalib;
	uint16_t TimeToFullChargeInMin;
	float (*GetBatteryVoltage)();
	float (*GetBatteryCurrent)();
	float (*GetChargeVoltage)();
}BatteryConfig_t;


BatteryState_t GetBatteryState(void);
uint16_t GetChargeTimeLeftMin(void);
uint8_t GetStateOfCharge(void);
float* GetSOCptr(void);
bool IsDischargeAllowed(void);
bool IsBatteryInCharging(void);


#endif