/*
 * pressure.c
 *
 *  Created on: 2015/10/26
 *      Author: toshio
 */

#include "currentloop.h"
#include "pressure.h"
#include "controldata.h"


#define	_PRESSURE_MAX	20
#define	_ATOM_RESSURE	1013

double	pressure_max 	= _PRESSURE_MAX;
double	atom_pressure	= _ATOM_RESSURE;


void Pressure_SetParam(uint16_t press_max, uint16_t atom_press)
{
	pressure_max 	= press_max;
	atom_pressure	= atom_press;
}

double Read_Pressure(void)
{
	double Current = Read_Current();
	double WaterPress;

	if (Current <= 3 || Current >= 30) {
		// Error
		return -1;
	}

	WaterPress = pressure_max * (Current - 4) / 16;
/*
	WaterPress = (Pressure * 1000) - atom_pressure;

	if (WaterPress < 0) {
		WaterPress = 0;
	}
*/
	if (WaterPress < 0) {
		WaterPress = 0;
	}

	return WaterPress;
}

double PressureToWaterlevel(double press)
{
	double Cf;
	double Sg;
	double WaterLevel;

	Cf = (double)((CNTL_DataSet[_CNTL_SUII_KEISU_H] << 8) | CNTL_DataSet[_CNTL_SUII_KEISU_L]) / 10000;
	Sg = (double)((CNTL_DataSet[_CNTL_SUII_HIJYU_H] << 8) | CNTL_DataSet[_CNTL_SUII_HIJYU_L]) / 10000;

	/*
	 * cmスケールなので100掛けてる
	 */
	WaterLevel = ((press * Cf) / Sg) * 100;

	return WaterLevel;
}

const char* WaterLevelToString(uint16_t WaterLevel)
{
	static char Buffer[6];
	uint16_t Level = WaterLevel;

	Buffer[0] = (Level / 1000) + '0';
	Level -= (Level / 1000) * 1000;
	Buffer[1] = (Level / 100) + '0';
	Level -= (Level / 100) * 100;
	Buffer[2] = (Level / 10) + '0';
	Level -= (Level / 10) * 10;
	Buffer[3] = (Level % 10) + '0';
	Buffer[4] = 0;

	return Buffer;
}

