/*
 * currentloop.c
 *
 *  Created on: 2015/10/26
 *      Author: toshio
 */
#include "currentloop.h"
#include "thermistor.h"
#include "ads1015.h"

#define	CL_LOADRESISTOR	200

double Read_Current(void)
{
	double Volt;
	Volt = StartConversion_Adc(ADS1015_CONFIG_AIN2_AIN3 | ADS1015_CONFIG_FS_4096 | ADS1015_CONFIG_SINGLE_MODE | ADS1015_CONFIG_FS_1600SPS | ADS1015_CONFIG_DSIABLE_COMP);

	double Current = (Volt * 2) / CL_LOADRESISTOR;

	return Current;
}
