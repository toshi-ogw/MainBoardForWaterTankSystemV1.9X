/*
 * i2c.c
 *
 *  Created on: 2015/10/21
 *      Author: toshio
 */

#include "i2c.h"



static void i2c_state_handling(I2C_ID_T id)
{
	if (Chip_I2C_IsMasterActive(id)) {
		Chip_I2C_MasterStateHandler(id);
	}
	else {
		Chip_I2C_SlaveStateHandler(id);
	}
}

void I2C_IRQHandler(void)
{
	i2c_state_handling(I2C0);
}

static void Init_I2C_PinMux(void)
{
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_4, IOCON_FUNC1 | 0);
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_5, IOCON_FUNC1 | 0);
}

void Init_I2C(uint32_t clockrate)
{
	Chip_SYSCTL_PeriphReset(RESET_I2C0);
	Init_I2C_PinMux();

	Chip_I2C_Init(I2C0);
    Chip_I2C_SetClockRate(I2C0, clockrate);
	Chip_I2C_SetMasterEventHandler(I2C0, Chip_I2C_EventHandler);
	NVIC_EnableIRQ(I2C0_IRQn);
}
