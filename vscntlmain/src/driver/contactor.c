/*
 * contactor.c
 *
 *  Created on: 2016/04/07
 *      Author: toshio
 */

#include "contactor.h"
#include "controldata.h"
#include "errorcode.h"
#include "timer32.h"

#define	CONTACTOR_PORT				3
#define	CONTACTOR_BIT				2

/*
 * Version1.06F 2017/06/12
 * チャタリング防止タイマ(1秒とか2秒とか長めにする)
 */
PTIMER_BLOCK	_pContactChatTimer = NULL;
#define	CONTACTCAHT_TIME	50

/*
 * Version1.06F 2017/06/12
 * 温度監視無視タイマー
 */
void ContactChatHandler(void);

static void Init_Contactor_PinMux(void)
{
	//Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO3_2,	(IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_MODE_PULLUP));	/* PIO3_2 used for CONTACTOR INPUT */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO3_2,	(IOCON_FUNC0 | IOCON_MODE_INACT));	/* PIO3_2 used for CONTACTOR INPUT */
	/*
	 * 入力設定
	 */
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, CONTACTOR_PORT, CONTACTOR_BIT);

	/*
	 * Version1.06F 2017/06/12
	 */
	_pContactChatTimer = AllocTimer1Task(CONTACTCAHT_TIME, ContactChatHandler);
	EnableTimerTask(_pContactChatTimer);
}

void Init_Contactor(void)
{
	Init_Contactor_PinMux();
}

bool Check_Contactor(void)
{
	if (Chip_GPIO_ReadPortBit(LPC_GPIO, CONTACTOR_PORT, CONTACTOR_BIT)) {
		/*
		 * 正常
		 */
		CNTL_DataSet[_CNTL_ERROR_STATE] &= ~_ERRORCODE_CONTACTOR;

		/*
		 * Version1.06F 2017/06/12
		 */
		SetTimerTime(_pContactChatTimer, CONTACTCAHT_TIME);
	}
	else {
		/*
		 * エラー
		 */
		//CNTL_DataSet[_CNTL_ERROR_STATE] |= _ERRORCODE_CONTACTOR;
		/*
		 * 強制停止
		 */
		//CNTL_DataSet[_CNTL_RUN] == 0x00;
//		return false;

		if (CNTL_DataSet[_CNTL_ERROR_STATE] & _ERRORCODE_CONTACTOR) {
			return false;
		}
	}

	return true;
}

/*
 * Verson1.06F
 */
void ContactChatHandler(void)
{
	CNTL_DataSet[_CNTL_ERROR_STATE] |= _ERRORCODE_CONTACTOR;
}
