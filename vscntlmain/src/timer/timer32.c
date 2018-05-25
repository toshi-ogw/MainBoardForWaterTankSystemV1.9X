/*
 * timer32.c
 *
 *  Created on: 2015/10/24
 *      Author: toshio
 */

#include "timer32.h"



TIMER_BLOCK	Timer0_Block[_TIMER0_MAX];	// 1Sec Timer
TIMER_BLOCK	Timer1_Block[_TIMER1_MAX];	// 100mSec Timer

PTIMER_BLOCK AllocTimerTask(PTIMER_BLOCK pBlock, int maxBlock, uint32_t time, void	(*pTimeupFunc)(void));


int32_t	__1SecTimer = 0;
void CountDown1Sec(void)
{
	if (__1SecTimer) {
		__1SecTimer--;
	}
}

int32_t	__100mSecTimer = 0;
void CountDown1mSec(void)
{
	if (__100mSecTimer) {
		__100mSecTimer--;
	}
}


void Wait1Sec(int32_t tim)
{
	__1SecTimer = tim;
	while(__1SecTimer) {
		__WFI();
	}
}

void Wait100mSec(int32_t tim)
{
	__100mSecTimer = tim;
	while(__100mSecTimer) {
		__WFI();
	}
}


void TIMER32_0_IRQHandler(void)
{
	int i;

	PTIMER_BLOCK pBlock	= Timer0_Block;

	if (Chip_TIMER_MatchPending(LPC_TIMER32_0, 0)) {
		Chip_TIMER_ClearMatch(LPC_TIMER32_0, 0);

		for (i = 0; i < _TIMER0_MAX; i++) {
			if (pBlock->bAvailable) {
				if (pBlock->valTime > 0) {
					pBlock->valTime--;
				}
				else {
					if (pBlock->pTimeupFunc) {
						pBlock->pTimeupFunc();
					}
					if (pBlock->bAvailable) {
						pBlock->valTime = pBlock->preTime;
					}
				}
			}
			pBlock++;
		}
	}
}

void TIMER32_1_IRQHandler(void)
{
	int i;

	PTIMER_BLOCK pBlock	= Timer1_Block;

	if (Chip_TIMER_MatchPending(LPC_TIMER32_1, 0)) {
		Chip_TIMER_ClearMatch(LPC_TIMER32_1, 0);

		for (i = 0; i < _TIMER1_MAX; i++) {
			if (pBlock->bAvailable) {
				if (pBlock->valTime > 0) {
					pBlock->valTime--;
				}
				else {
					if (pBlock->pTimeupFunc) {
						pBlock->pTimeupFunc();
					}
					if (pBlock->bAvailable) {
						pBlock->valTime = pBlock->preTime;
					}
				}
			}
			pBlock++;
		}
	}
}

/*
 * 32bit Timer0
 *
 * -- 1Sec Timer
 * -- For LED
 * -- For Key Scan
 */
void Init_TIMER32_0(void)
{
	int i;

	for (i = 0; i < _TIMER0_MAX; i++) {
		FreeTimerTask(&Timer0_Block[i]);
	}

	PTIMER_BLOCK pBlk = AllocTimerTask(Timer0_Block, _TIMER0_MAX, 0, CountDown1Sec);
	EnableTimerTask(pBlk);

	Chip_TIMER_Init(LPC_TIMER32_0);

	uint32_t timerFreq = Chip_Clock_GetSystemClockRate();

	Chip_TIMER_Reset(LPC_TIMER32_0);
	Chip_TIMER_MatchEnableInt(LPC_TIMER32_0, 0);
	Chip_TIMER_SetMatch(LPC_TIMER32_0, 0, (timerFreq));
	Chip_TIMER_ResetOnMatchEnable(LPC_TIMER32_0, 0);
	Chip_TIMER_Enable(LPC_TIMER32_0);

	/* Enable timer interrupt */
	NVIC_ClearPendingIRQ(TIMER_32_0_IRQn);
	NVIC_EnableIRQ(TIMER_32_0_IRQn);
}



/*
 * 32bit Timer1
 *
 * -- 100mSec Timer
 * -- For LED
 * -- For Key Scan
 */
void Init_TIMER32_1(void)
{
	int i;

	for (i = 0; i < _TIMER1_MAX; i++) {
		FreeTimerTask(&Timer1_Block[i]);
	}

	PTIMER_BLOCK pBlk = AllocTimerTask(Timer1_Block, _TIMER1_MAX, 0, CountDown1mSec);
	EnableTimerTask(pBlk);

	Chip_TIMER_Init(LPC_TIMER32_1);

	uint32_t timerFreq = Chip_Clock_GetSystemClockRate() / 10;

	Chip_TIMER_Reset(LPC_TIMER32_1);
	Chip_TIMER_MatchEnableInt(LPC_TIMER32_1, 0);
	Chip_TIMER_SetMatch(LPC_TIMER32_1, 0, (timerFreq));
	Chip_TIMER_ResetOnMatchEnable(LPC_TIMER32_1, 0);
	Chip_TIMER_Enable(LPC_TIMER32_1);

	/* Enable timer interrupt */
	NVIC_ClearPendingIRQ(TIMER_32_1_IRQn);
	NVIC_EnableIRQ(TIMER_32_1_IRQn);
}




PTIMER_BLOCK AllocTimerTask(PTIMER_BLOCK pBlock, int maxBlock, uint32_t time, void	(*pTimeupFunc)(void))
{
	int i;

	for (i = 0; i < maxBlock; i++) {
		if (!pBlock->bInUse) {
			pBlock->bAvailable	= false;
			pBlock->pTimeupFunc	= pTimeupFunc;
			pBlock->preTime		= time;
			pBlock->valTime		= time;
			pBlock->bInUse		= true;
			return pBlock;
		}
		pBlock++;
	}

	return NULL;
}

/*
 *	Version 1.05 2016/12/12 T.Ogawa
 *	やっぱりタイマ値が変更できたら良いと思って
 */
bool SetTimerTime(PTIMER_BLOCK pBlock, uint32_t time)
{
	bool bAvKeep;

	if (!pBlock) {
		return false;
	}

	bAvKeep = pBlock->bAvailable;
	pBlock->bAvailable = false;

	pBlock->preTime = time;
	pBlock->valTime = time;

	pBlock->bAvailable = bAvKeep;

	return true;
}



PTIMER_BLOCK AllocTimer0Task(uint32_t time, void	(*pTimeupFunc)(void))
{
	return AllocTimerTask(Timer0_Block, _TIMER0_MAX, time, pTimeupFunc);
}

PTIMER_BLOCK AllocTimer1Task(uint32_t time, void	(*pTimeupFunc)(void))
{
	return AllocTimerTask(Timer1_Block, _TIMER1_MAX, time, pTimeupFunc);
}

void EnableTimerTask(PTIMER_BLOCK pBlock)
{
	if (pBlock) {
		pBlock->valTime = pBlock->preTime;
		pBlock->bAvailable	= true;
	}
}

void DisableTimerTask(PTIMER_BLOCK pBlock)
{
	if (pBlock) {
		pBlock->bAvailable	= false;
	}
}

void FreeTimerTask(PTIMER_BLOCK pBlock)
{
	if (pBlock) {
		pBlock->bAvailable	= false;
		pBlock->bInUse		= false;
		pBlock->pTimeupFunc	= NULL;
		pBlock->preTime		= 0;
		pBlock->valTime		= 0;
	}
}
