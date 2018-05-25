/*
 * powerled.c
 *
 *  Created on: 2016/04/07
 *      Author: toshio
 */

#include "powerled.h"
#include "timer32.h"
#include "controldata.h"

#define	POWERLED_PORT	1
#define	POWERLED_BIT	4

PTIMER_BLOCK	pPowerLEDTimer;

#define	POWERLED_TIMER	2

static void Init_PowerLED_PinMux(void)
{
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_4,	(IOCON_FUNC0 | IOCON_MODE_INACT | MD_DIGMODE));	/* PIO1_4 used for Power LED */
	/*
	 * 出力設定
	 */
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, POWERLED_PORT, POWERLED_BIT);
}

volatile uint8_t PowerLEDState = 0;

void PowerLEDTimerHadler(void)
{
	uint8_t Led;

	PowerLEDState++;
	PowerLEDState &= 0x01;

	Led = PowerLEDState;

	if (!CNTL_DataSet[_CNTL_ERROR_STATE]) {
		/*
		 * エラーじゃない時は点灯しっぱなし
		 */
		Led = 0;
	}

	Chip_GPIO_SetPinState(LPC_GPIO, POWERLED_PORT, POWERLED_BIT, Led);
}

void Init_PowerLED(void)
{
	Init_PowerLED_PinMux();

	Chip_GPIO_SetPinState(LPC_GPIO, POWERLED_PORT, POWERLED_BIT, 0);

	// LED点滅タイマ
	pPowerLEDTimer = AllocTimer1Task(POWERLED_TIMER, PowerLEDTimerHadler);
	EnableTimerTask(pPowerLEDTimer);

}
