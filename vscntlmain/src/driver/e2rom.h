/*
 * e2rom.h
 *
 *  Created on: 2015/10/21
 *      Author: toshio
 */

#ifndef E2ROM_H_
#define E2ROM_H_

#include "i2c.h"

#define	E2ROM_DEVICEID	0x50

int Read_E2Rom(uint8_t adrs, uint8_t *buff, uint8_t len);
int Write_E2Rom(uint8_t adrs, uint8_t *buff, uint8_t len);


#endif /* E2ROM_H_ */
