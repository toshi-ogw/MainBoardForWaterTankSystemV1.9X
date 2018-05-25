/*
 * uart.c
 *
 *  Created on: 2015/10/24
 *      Author: toshio
 */

#include "uart.h"
#include "string.h"
#include "tick.h"
#include "timer32.h"

/* Transmit and receive ring buffers */
STATIC RINGBUFF_T txring, rxring;

/* Transmit and receive ring buffer sizes */
#define UART_SRB_SIZE 128	/* Send */
#define UART_RRB_SIZE 128	/* Receive */

/*
 * Verion 1.06C 2017/04/18
 */
#define	UART_MONITOR_TIME	200

/* Transmit and receive buffers */
static uint8_t rxbuff[UART_RRB_SIZE], txbuff[UART_SRB_SIZE];


// 送信中？
static bool bSending			= false;
static bool bReceiveAvalable	= true;

PTIMER_BLOCK		pUARTMonitaringTimerBlock = NULL;


#define	UART_TXMODE		Chip_GPIO_SetPinState(LPC_GPIO, 1, 8, 1)
#define	UART_RXMODE		Chip_GPIO_SetPinState(LPC_GPIO, 1, 8, 0)

bool bTX_Timeout = false;

void UART_ChangeTX(void) {
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_7, (IOCON_FUNC0 | IOCON_MODE_INACT));/* PIO MODE */
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 1, 7);
	while(1) {
	Chip_GPIO_SetPinState(LPC_GPIO, 1, 7, 0);
	Wait(1);
	Chip_GPIO_SetPinState(LPC_GPIO, 1, 7, 1);
	Wait(1);
	}
	Chip_GPIO_SetPinState(LPC_GPIO, 1, 7, 0);
	Chip_GPIO_SetPinState(LPC_GPIO, 1, 7, 1);
	Wait(10);
	UART_TXMODE;
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_7, (IOCON_FUNC1 | IOCON_MODE_INACT));/* TXD */
}

void UART_PreTX(void) {
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_7, (IOCON_FUNC0 | IOCON_MODE_INACT));/* PIO MODE */
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 1, 7);

	while(1) {
	Chip_GPIO_SetPinState(LPC_GPIO, 1, 7, 0);
	Wait(1);
	Chip_GPIO_SetPinState(LPC_GPIO, 1, 7, 1);
	Wait(1);
	}
	UART_TXMODE;
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_7, (IOCON_FUNC1 | IOCON_MODE_INACT));/* TXD */
}

void Monitaring_UART(void)
{
	/*
	 * 強制的に受信モードにする
	 */
	DisableTimerTask(pUARTMonitaringTimerBlock);	// モニタリングタイマを無効化
	// 送信終了
	bSending = false;
	// 受信バッファをクリア
	RingBuffer_Flush(&rxring);
	// 受信モードへ移行
	UART_RXMODE;
	// 受信有効
	bReceiveAvalable = true;
	// 送信失敗
	bTX_Timeout = true;
}

void UART_IRQHandler(void)
{
	Chip_UART_IRQRBHandler(LPC_USART, &rxring, &txring);
}

static void Init_UART_PinMux(void)
{
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_6, (IOCON_FUNC1 | IOCON_MODE_INACT));/* RXD */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_7, (IOCON_FUNC1 | IOCON_MODE_INACT));/* TXD */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_8, (IOCON_FUNC0 | IOCON_MODE_INACT));/* LINK */
}


void Init_UART(uint32_t clockrate)
{
	// 500mSec
	//pUARTMonitaringTimerBlock = AllocTimer1Task(10, Monitaring_UART);
	/*
	 * Verion 1.06D 2017/04/18
	 */
	pUARTMonitaringTimerBlock = AllocTimer1Task(UART_MONITOR_TIME, Monitaring_UART);

	Init_UART_PinMux();

	// LINK Output mode
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 1, 8);
	// LINK RX Mode
	UART_RXMODE;
	// LINK RX Mode
	UART_TXMODE;
	// LINK RX Mode
	UART_RXMODE;
	// LINK RX Mode
	UART_TXMODE;

	// 9600 N81
	Chip_UART_Init(LPC_USART);
	Chip_UART_SetBaud(LPC_USART, clockrate);
	Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT));
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));

	Chip_UART_TXEnable(LPC_USART);

	/* Before using the ring buffers, initialize them using the ring
	   buffer init function */
	RingBuffer_Init(&rxring, rxbuff, 1, UART_RRB_SIZE);
	RingBuffer_Init(&txring, txbuff, 1, UART_SRB_SIZE);

	/* Enable receive data and line status interrupt */
	Chip_UART_IntEnable(LPC_USART, (UART_IER_RBRINT | UART_IER_RLSINT));

	/* preemption = 1, sub-priority = 1 */
	NVIC_SetPriority(UART0_IRQn, 1);
	NVIC_EnableIRQ(UART0_IRQn);
}

uint8_t pstrDummyHead[10] = {	0x02, 0x02, 0x02, 0x02, 0x02 };
uint32_t  SendRB_UART(const void *data, int bytes)
{
	uint32_t	send_bytes, total_length;


	send_bytes = bytes;
	total_length = 0;
	while(bytes > 0) {
		if (!bSending) {
			// TX MODE
			//UART_PreTX();
			//Wait(5);
			UART_TXMODE;
			//Wait(5);
			//UART_ChangeTX();
			// 送信中に設定
			bSending = true;
			// 受信無効
			bReceiveAvalable = false;
			// Dummy Send
			Chip_UART_SendRB(LPC_USART, &txring, pstrDummyHead, 5);
		}

		send_bytes = Chip_UART_SendRB(LPC_USART, &txring, data, bytes);
		total_length += send_bytes;
		bytes -= send_bytes;
		data += send_bytes;
	}

	// 一定時間後に強制的に受信状態を設定する
	bTX_Timeout = false;
	EnableTimerTask(pUARTMonitaringTimerBlock);

	while(!bTX_Timeout) {
		if (RingBuffer_IsEmpty(&txring)) {
			// 送信完了を待つ
			while(!bTX_Timeout) {
				if ((Chip_UART_ReadLineStatus(LPC_USART) & UART_LSR_TEMT) != 0) {
					// モニタリングタイマを無効化
					DisableTimerTask(pUARTMonitaringTimerBlock);
					// 送信完了
					bSending = false;
					// 受信バッファをクリア
					RingBuffer_Flush(&rxring);
					// 受信モードへ移行
					UART_RXMODE;
					// 受信有効
					bReceiveAvalable = true;
					return total_length;
				}
			}
		}
	}

	return total_length;
}


uint32_t ReadRB_UART(void *data, int bytes)
{
	while(!bReceiveAvalable) {
		// 送信完了待ち
	}
	return Chip_UART_ReadRB(LPC_USART, &rxring, data, bytes);
}
