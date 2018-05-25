/*
 * uart.h
 *
 *  Created on: 2015/10/24
 *      Author: toshio
 */

#ifndef UART_H_
#define UART_H_


#include "chip.h"
#include <cr_section_macros.h>

void Init_UART(uint32_t clockrate);
uint32_t SendRB_UART(const void *data, int bytes);
uint32_t ReadRB_UART(void *data, int bytes);

#endif /* UART_H_ */
