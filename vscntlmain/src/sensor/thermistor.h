/*
 * thermistor.h
 *
 *  Created on: 2015/10/25
 *      Author: toshio
 */

#ifndef THERMISTOR_H_
#define THERMISTOR_H_


#include "chip.h"
#include <cr_section_macros.h>
#include "float.h"
#include "math.h"

void thermistor_SetParam(uint16_t b, uint16_t r0, uint16_t t0);
double thermistor_GetTemperature(double r);
double thermistor_VoltageToResistance(double voltage);

#endif /* THERMISTOR_H_ */
