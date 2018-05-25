/*
 * sensordrive.h
 *
 *  Created on: 2015/12/14
 *      Author: toshio
 */

#ifndef SENSOR_SENSORDRIVE_H_
#define SENSOR_SENSORDRIVE_H_

#include "chip.h"
#include <cr_section_macros.h>

extern	bool bTemperatureSensUpdate;
extern	bool bSuiiSensUpdate;


#define	IS_TEMPERATURE_SENS_UPDATE	bTemperatureSensUpdate
#define	IS_SUII_SENS_UPDATE			bSuiiSensUpdate

void InitSensorDrive(void);
bool MeasurementTemperature(void);
bool MeasurementWaterLevel(void);

void StartMeasurement(void);
void StopMeasurement(void);

void Measurement(void);

#define	_WATERLEVEL_ERR	0	// センサー異常
#define	_WATERLEVEL_E1	1	// 満水
#define	_WATERLEVEL_E2	2	// 注水
#define	_WATERLEVEL_E3	3	// 低水
#define	_WATERLEVEL_EE	4	// 渇水

#define	_WATERLEVEL_DEVICE_ELECTRODE	0
#define	_WATERLEVEL_DEVICE_PRESSURE		1

#endif /* SENSOR_SENSORDRIVE_H_ */
