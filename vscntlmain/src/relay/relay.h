/*
 * relay.h
 *
 *  Created on: 2015/10/21
 *      Author: toshio
 */

#ifndef RELAY_H_
#define RELAY_H_

#include "controldata.h"

#define	_RELEAY_ON	1
#define	_RELEAY_OFF	0

//-- Relay IO Port
#define	BD_RELAY_PORT		1
#define	BD_RELAY_HEATER		5
#define	BD_RELAY_WATER		9
#define	BD_RELAY_PUMP		10
#define	BD_RELAY_DRAINAGE	11

//-- Drive Relay Port
#define	BD_RELAY_DRIVE_PORT	0
#define	BD_RELAY_DRIVE		11

//-- Error Relay Port
#define	BD_RELAY_ERROR_PORT	2
#define	BD_RELAY_ERROR		0

void Init_Relay(void);
void SetAllRelay(bool setting);
void SetRelay(uint8_t port, uint8_t pin, bool setting);

#define	SET_RELAY_HEATER(sw)	{ SetRelay(BD_RELAY_PORT,		BD_RELAY_HEATER, sw);	(sw ? (CNTL_DataSet[_CNTL_RELAY_STATE] |= _CNTL_RY_BIT_HEATER)	 : (CNTL_DataSet[_CNTL_RELAY_STATE] &= ~_CNTL_RY_BIT_HEATER)); }
#define	SET_RELAY_WATER(sw)		{ SetRelay(BD_RELAY_PORT,		BD_RELAY_WATER, sw);	(sw ? (CNTL_DataSet[_CNTL_RELAY_STATE] |= _CNTL_RY_BIT_WATER)	 : (CNTL_DataSet[_CNTL_RELAY_STATE] &= ~_CNTL_RY_BIT_WATER)); }
#define	SET_RELAY_PUMP(sw)		{ SetRelay(BD_RELAY_PORT,		BD_RELAY_PUMP, sw);		(sw ? (CNTL_DataSet[_CNTL_RELAY_STATE] |= _CNTL_RY_BIT_PUMP) 	 : (CNTL_DataSet[_CNTL_RELAY_STATE] &= ~_CNTL_RY_BIT_PUMP)); }
#define	SET_RELAY_DRAINAGE(sw)	{ SetRelay(BD_RELAY_PORT,		BD_RELAY_DRAINAGE, sw);	(sw ? (CNTL_DataSet[_CNTL_RELAY_STATE] |= _CNTL_RY_BIT_DRAINAGE) : (CNTL_DataSet[_CNTL_RELAY_STATE] &= ~_CNTL_RY_BIT_DRAINAGE)); }
#define	SET_RELAY_DRIVE(sw)		{ SetRelay(BD_RELAY_DRIVE_PORT,	BD_RELAY_DRIVE, sw);	(sw ? (CNTL_DataSet[_CNTL_RELAY_STATE] |= _CNTL_RY_BIT_DRIVE)	 : (CNTL_DataSet[_CNTL_RELAY_STATE] &= ~_CNTL_RY_BIT_DRIVE)); }
#define	SET_RELAY_ERROR(sw)		{ SetRelay(BD_RELAY_ERROR_PORT,	BD_RELAY_ERROR, sw);	(sw ? (CNTL_DataSet[_CNTL_RELAY_STATE] |= _CNTL_RY_BIT_ERROR)	 : (CNTL_DataSet[_CNTL_RELAY_STATE] &= ~_CNTL_RY_BIT_ERROR)); }

#define	IS_RELAY_ON_HEATER		(CNTL_DataSet[_CNTL_RELAY_STATE] & _CNTL_RY_BIT_HEATER) ? true : false
#define	IS_RELAY_ON_WATER		(CNTL_DataSet[_CNTL_RELAY_STATE] & _CNTL_RY_BIT_WATER) ? true : false
#define	IS_RELAY_ON_PUMP		(CNTL_DataSet[_CNTL_RELAY_STATE] & _CNTL_RY_BIT_PUMP) ? true : false
#define	IS_RELAY_ON_DRAINAGE	(CNTL_DataSet[_CNTL_RELAY_STATE] & _CNTL_RY_BIT_DRAINAGE) ? true : false
#define	IS_RELAY_ON_DRIVE		(CNTL_DataSet[_CNTL_RELAY_STATE] & _CNTL_RY_BIT_DRIVE) ? true : false
#define	IS_RELAY_ON_ERROR		(CNTL_DataSet[_CNTL_RELAY_STATE] & _CNTL_RY_BIT_ERROR) ? true : false

#endif /* RELAY_H_ */
