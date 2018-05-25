/*
 * tick.h
 *
 *  Created on: 2015/10/24
 *      Author: toshio
 */

#ifndef TICK_H_
#define TICK_H_


#include "chip.h"
#include <cr_section_macros.h>

#define	_TICK_MAX	5

typedef struct {
	bool	bInUse;
	bool	bAvailable;
	uint8_t	preTime;
	uint8_t	valTime;
	void	(*pTimeupFunc)(void);
} TICKTIMER_BLOCK, *PTICKTIMER_BLOCK;

void Init_Tick(void);
void Wait(int32_t w10msec);

PTICKTIMER_BLOCK AllocTickTask(uint8_t time, void	(*pTimeupFunc)(void));

void EnableTickTask(PTICKTIMER_BLOCK pBlock);
void DisableTickTask(PTICKTIMER_BLOCK pBlock);
void FreeTickTask(PTICKTIMER_BLOCK pBlock);


extern volatile int32_t		_10mSecTickTime;
extern volatile int32_t		_ForSockTimeout;

#define	StartTimeMeasurement()	{ _10mSecTickTime = 0; }
#define	GetTimeMeasurement()	_10mSecTickTime


#endif /* TICK_H_ */
