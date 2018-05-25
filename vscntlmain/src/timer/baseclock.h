/*
 * baseclock.h
 *
 *  Created on: 2016/03/10
 *      Author: toshio
 */

#ifndef TIMER_BASECLOCK_H_
#define TIMER_BASECLOCK_H_



#include "chip.h"
#include <cr_section_macros.h>

void OnTimerRestartCheck(void);

void Init_BaseClock(void);
void UpdateBaseClock(uint32_t time);

void SetBaseClock(void);
void GetBaseClock(void);

typedef union {
	uint16_t	word;
	uint8_t		byte[2];
	struct {
		uint16_t	time	: 12;
		uint16_t	dummy0	: 1;
		uint16_t	dummy1	: 1;
		uint16_t	wait	: 1;
		uint16_t	run		: 1;
	};
} ALARM_DATA, *PALARM_DATA;

typedef union {
	uint8_t	byte;
	struct {
		uint8_t		start_timer	: 1;
		uint8_t		stop_tumer	: 1;
		uint8_t		etc			: 6;
	};
} ALARM_CNTL_DATA, *PALARM_CNTL_DATA;;


extern uint32_t	m_BaseClock;

#endif /* TIMER_BASECLOCK_H_ */
