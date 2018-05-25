/*
 * timer32.h
 *
 *  Created on: 2015/10/24
 *      Author: toshio
 */

#ifndef TIMER32_H_
#define TIMER32_H_

#include "chip.h"
#include <cr_section_macros.h>

/*
 *  Modify July,6th 2016
 *  タイマー領域を節約
 */
/*
 * Version 1.05 2016/12/12
 * 監視タイマ追加により２個追加
 */
/*
 * Version 1.06B 2017/04/15
 * 温度監視無視　運転開始から10秒間は温度監視無視機能追加
 */
#define	_TIMER0_MAX	14

/*
 * Version 1.06F 2017/06/12
 * コンタクトエラー監視用で+1
 */
#define	_TIMER1_MAX	7

typedef struct {
	bool		bInUse;
	bool		bAvailable;
	uint32_t	preTime;
	uint32_t	valTime;
	void		(*pTimeupFunc)(void);
} TIMER_BLOCK, *PTIMER_BLOCK;

void Init_TIMER32_0(void);
void Init_TIMER32_1(void);
void Wait1Sec(int32_t tim);


PTIMER_BLOCK AllocTimer0Task(uint32_t time, void	(*pTimeupFunc)(void));
PTIMER_BLOCK AllocTimer1Task(uint32_t time, void	(*pTimeupFunc)(void));

void EnableTimerTask(PTIMER_BLOCK pBlock);
void DisableTimerTask(PTIMER_BLOCK pBlock);
void FreeTimerTask(PTIMER_BLOCK pBlock);

/*
 *	Version 1.05 2016/12/12 T.Ogawa
 *	やっぱりタイマ値が変更できたら良いと思って
 */
bool SetTimerTime(PTIMER_BLOCK pBlock, uint32_t time);

#endif /* TIMER32_H_ */
