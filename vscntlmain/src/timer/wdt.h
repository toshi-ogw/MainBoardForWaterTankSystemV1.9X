/*
 * wdt.h
 *
 *  Created on: 2016/05/18
 *      Author: toshio
 */

#ifndef TIMER_WDT_H_
#define TIMER_WDT_H_

#include "chip.h"
#include <cr_section_macros.h>

void WDT_Start(uint32_t time);
void WDT_Kick(void);

void BootDataManage(void);

extern uint32_t		WDT_CurrentCount;
extern uint16_t		ReBoot_Count;
extern uint8_t		Reboot_Status;


#endif /* TIMER_WDT_H_ */
