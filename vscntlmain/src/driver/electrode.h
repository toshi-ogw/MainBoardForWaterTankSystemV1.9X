/*
 * electrode.h
 *
 *  Created on: 2015/12/14
 *      Author: toshio
 */

#ifndef DRIVER_ELECTRODE_H_
#define DRIVER_ELECTRODE_H_

#include "chip.h"
#include <cr_section_macros.h>


typedef struct {
	bool	bSens;		// 現在の値
	bool	bPrevSens;	// 前回の値
	uint8_t	Time;		// ディレイ
	uint8_t preTimer;	// ディレイ設定値
} WATER_SENS_VALUE, *PWATER_SENS_VALUE;

extern	WATER_SENS_VALUE	_bWaterSens_L;
extern	WATER_SENS_VALUE	_bWaterSens_M;
extern	WATER_SENS_VALUE	_bWaterSens_H;

#define	IS_WATERSENS_L	_bWaterSens_L.bSens
#define	IS_WATERSENS_M	_bWaterSens_M.bSens
#define	IS_WATERSENS_H	_bWaterSens_H.bSens

void Init_Electrode(void);
void ElectrodeStaticSens(void);
void ElectrodeDebug(void);

#endif /* DRIVER_ELECTRODE_H_ */
