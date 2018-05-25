/*
 * thermistor.c
 *
 *  Created on: 2015/10/25
 *      Author: toshio
 */

#ifndef THERMISTOR_C_
#define THERMISTOR_C_

#include "thermistor.h"

#define	NTC_REF_VOLTAGE		4.096
#define	NTC_REF_RESISTANCE	10000

#define	NTC_DEF_B_PARAM		3435.0
#define	NTC_DEF_R0			10000.0
#define	NTC_DEF_T0			25.0

double	ntc_B	= NTC_DEF_B_PARAM;
double	ntc_R0	= NTC_DEF_R0;
double	ntc_rTS = (1 / (NTC_DEF_T0 + 273.15));

void thermistor_SetParam(uint16_t b, uint16_t r0, uint16_t t0)
{
	ntc_B	= b;
	ntc_R0	= r0;
	ntc_rTS = (1 / (t0 + 273.15));
}

double thermistor_GetTemperature(double r)
{
	double temp;

	temp = 1 / (log(r / ntc_R0) / ntc_B + ntc_rTS) - 273.15;

	return temp;
}

double thermistor_VoltageToResistance(double voltage)
{
	double Resistance;

	Resistance = (voltage / (NTC_REF_VOLTAGE - voltage)) * NTC_REF_RESISTANCE;

	return Resistance;
}

#endif /* THERMISTOR_C_ */
