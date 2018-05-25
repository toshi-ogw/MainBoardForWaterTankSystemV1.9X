/*
 * temperature.c
 *
 *  Created on: 2015/10/26
 *      Author: toshio
 */

#include "temperature.h"
#include "thermistor.h"
#include "ads1015.h"


double Read_Temperature(void)
{
	double Volt;
	Volt = StartConversion_Adc(ADS1015_CONFIG_AIN0_GND | ADS1015_CONFIG_FS_4096 | ADS1015_CONFIG_SINGLE_MODE | ADS1015_CONFIG_FS_1600SPS | ADS1015_CONFIG_DSIABLE_COMP);




	double R = thermistor_VoltageToResistance((Volt * 2) / 1000);
	double T = thermistor_GetTemperature(R);

	return T;
}

const char* TemperatureToString(double t)
{
	static char Buffer[6];
	int TVal = (int)(t * 10);

	Buffer[0] = (TVal / 1000) + '0';
	TVal -= (TVal / 1000) * 1000;
	Buffer[1] = (TVal / 100) + '0';
	TVal -= (TVal / 100) * 100;
	Buffer[2] = (TVal / 10) + '0';
	TVal -= (TVal / 10) * 10;
	Buffer[3] = '.';
	Buffer[4] = (TVal % 10) + '0';
	Buffer[5] = 0;

	return Buffer;
}
