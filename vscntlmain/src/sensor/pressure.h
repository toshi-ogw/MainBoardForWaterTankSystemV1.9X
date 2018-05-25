/*
 * pressure.h
 *
 *  Created on: 2015/10/26
 *      Author: toshio
 */

#ifndef PRESSURE_H_
#define PRESSURE_H_

#include "chip.h"
#include <cr_section_macros.h>
#include "float.h"
#include "math.h"

double Read_Pressure(void);
double PressureToWaterlevel(double press);
const char* WaterLevelToString(uint16_t WaterLevel);

#endif /* PRESSURE_H_ */
