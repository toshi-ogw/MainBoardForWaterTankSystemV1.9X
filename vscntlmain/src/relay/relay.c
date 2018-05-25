/*
 * relay.c
 *
 *  Created on: 2015/10/21
 *      Author: toshio
 */

#include "chip.h"
#include <cr_section_macros.h>
#include "relay.h"

static void Init_Relay_PinMux(void)
{
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_5,	(IOCON_FUNC0 | IOCON_MODE_INACT));	/* PIO1_5 used for HEATER RELAY */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_9,	(IOCON_FUNC0 | IOCON_MODE_INACT));	/* PIO1_9 used for WATER RELEAY */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_10,	(IOCON_FUNC0 | IOCON_MODE_INACT));	/* PIO1_10 used for PUMP RELEAY */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_11,	(IOCON_FUNC0 | IOCON_MODE_INACT));	/* PIO1_11 used for DRAINGE */

	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_11,	(IOCON_FUNC1 | IOCON_DIGMODE_EN));	/* PIO0_11 used for DRIVE */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_0,	(IOCON_FUNC0 | IOCON_MODE_INACT));	/* PIO2_0 used for ERROR */
}

void Init_Relay(void)
{
	Init_Relay_PinMux();

	Chip_GPIO_SetPinDIROutput(LPC_GPIO, BD_RELAY_PORT, 			BD_RELAY_HEATER);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, BD_RELAY_PORT, 			BD_RELAY_WATER);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, BD_RELAY_PORT, 			BD_RELAY_PUMP);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, BD_RELAY_PORT, 			BD_RELAY_DRAINAGE);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, BD_RELAY_DRIVE_PORT,	BD_RELAY_DRIVE);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, BD_RELAY_ERROR_PORT,	BD_RELAY_ERROR);

	// ALL OFF
	SetAllRelay(_RELEAY_OFF);
}

void SetAllRelay(bool setting)
{
	Chip_GPIO_SetPinState(LPC_GPIO, BD_RELAY_PORT, 			BD_RELAY_HEATER,	setting);
	Chip_GPIO_SetPinState(LPC_GPIO, BD_RELAY_PORT, 			BD_RELAY_WATER, 	setting);
	Chip_GPIO_SetPinState(LPC_GPIO, BD_RELAY_PORT, 			BD_RELAY_PUMP, 		setting);
	Chip_GPIO_SetPinState(LPC_GPIO, BD_RELAY_PORT, 			BD_RELAY_DRAINAGE, 	setting);
	Chip_GPIO_SetPinState(LPC_GPIO, BD_RELAY_DRIVE_PORT,	BD_RELAY_DRIVE, 	setting);
	Chip_GPIO_SetPinState(LPC_GPIO, BD_RELAY_ERROR_PORT,	BD_RELAY_ERROR, 	setting);
}

void SetRelay(uint8_t port, uint8_t pin, bool setting)
{
	Chip_GPIO_SetPinState(LPC_GPIO, port, pin,	setting);
}


