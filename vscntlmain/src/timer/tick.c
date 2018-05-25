/*
 * tick.c
 *
 *  Created on: 2015/10/24
 *      Author: toshio
 */


#include "tick.h"

volatile int32_t		_10mSecTimer = 0;
volatile int32_t		_10mSecTickTime = 0;

volatile int32_t		_ForSockTimeout = 0;


TICKTIMER_BLOCK	Tick_Block[_TICK_MAX];	// 100mSec Timer

void SysTick_Handler(void)
{
	int i;
	for (i = 0; i < _TICK_MAX; i++) {
		if (Tick_Block[i].bAvailable) {
			if (Tick_Block[i].valTime > 0) {
				Tick_Block[i].valTime--;
			}
			else {
				if (Tick_Block[i].pTimeupFunc) {
					Tick_Block[i].pTimeupFunc();
				}
				if (Tick_Block[i].bAvailable) {
					Tick_Block[i].valTime = Tick_Block[i].preTime;
				}
			}
		}
	}

	if (_10mSecTimer) {
		_10mSecTimer--;
	}
	/*
	 * ソケット時間監視用
	 */
	if (_ForSockTimeout) {
		_ForSockTimeout--;
	}
	/*
	 * ひたすらインクリメント
	 * これはデバッグ計測用タイマ
	 */
	_10mSecTickTime++;
}

void Init_Tick(void)
{
	uint8_t i;
	for (i = 0; i < _TICK_MAX; i++) {
		FreeTickTask(&Tick_Block[i]);
	}

	SysTick_Config(SystemCoreClock / 100);
}

void Wait(int32_t w10msec)
{
	_10mSecTimer = w10msec;

	while(_10mSecTimer) {
		__WFI();
	}
}


PTICKTIMER_BLOCK AllocTickTask(uint8_t time, void	(*pTimeupFunc)(void))
{
	int i;

	for (i = 0; i < _TICK_MAX; i++) {
		if (!Tick_Block[i].bInUse) {
			Tick_Block[i].bAvailable	= false;
			Tick_Block[i].pTimeupFunc	= pTimeupFunc;
			Tick_Block[i].preTime		= time;
			Tick_Block[i].valTime		= time;
			Tick_Block[i].bInUse		= true;
			return &Tick_Block[i];
		}
	}

	return NULL;
}

void EnableTickTask(PTICKTIMER_BLOCK pBlock)
{
	if (pBlock) {
		pBlock->valTime = pBlock->preTime;
		pBlock->bAvailable	= true;
	}
}

void DisableTickTask(PTICKTIMER_BLOCK pBlock)
{
	if (pBlock) {
		pBlock->bAvailable	= false;
	}
}

void FreeTickTask(PTICKTIMER_BLOCK pBlock)
{
	if (pBlock) {
		pBlock->bAvailable	= false;
		pBlock->bInUse		= false;
		pBlock->pTimeupFunc	= NULL;
		pBlock->preTime		= 0;
		pBlock->valTime		= 0;
	}
}
