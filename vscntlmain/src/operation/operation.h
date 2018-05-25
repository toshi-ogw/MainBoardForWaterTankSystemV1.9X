/*
 * operation.h
 *
 *  Created on: 2015/12/14
 *      Author: toshio
 */

#ifndef OPERATION_OPERATION_H_
#define OPERATION_OPERATION_H_


#include "chip.h"
#include <cr_section_macros.h>

void Init_Operation(void);
void Operation(void);

/*
 * 異常温度は45℃
 */
/*
 * 温泉タンク仕様 99℃
 */
#define	ERROR_TEMPERATURE	990

/*
 * WDTタイマー
 */
#define	WDT_TIME			5

/*
 * scan time
 */
extern uint32_t	CurrentScanTime;
extern uint32_t	MaxScanTime;

#endif /* OPERATION_OPERATION_H_ */
