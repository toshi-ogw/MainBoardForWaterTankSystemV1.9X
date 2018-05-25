/*
 * i2c.h
 *
 *  Created on: 2015/10/21
 *      Author: toshio
 */

#ifndef I2C_H_
#define I2C_H_

#include "chip.h"
#include <cr_section_macros.h>

#define SPEED_100KHZ         100000
#define SPEED_400KHZ         400000

void Init_I2C(uint32_t clockrate);

#define	Send_I2C(id, buff, len)	Chip_I2C_MasterSend(I2C0, id, buff, len)
#define	Read_I2C(id, buff, len)	Chip_I2C_MasterSend(I2C0, id, buff, len)

#endif /* I2C_H_ */
