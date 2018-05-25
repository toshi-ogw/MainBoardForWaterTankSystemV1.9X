/*
 * w5500.c
 *
 *  Created on: 2016/03/23
 *      Author: toshio
 */

#include "w5500.h"



#define	W5500_RESET_PORT			0
#define	W5500_RESET					7
#define	W5500_INT_PORT				0
#define	W5500_INT					3

#define LPC_SSP           LPC_SSP0


/*
 * 割込みハンドラ内か否か
 */
volatile bool bIsInterrupt = false;

void Interrupt__Disable(void) {
	if (!bIsInterrupt) {
		__disable_irq();
	}
}

void Interrupt__Enable(void) {
	if (!bIsInterrupt) {
		__enable_irq();
	}
}

#define	ENTER_IRQ	bIsInterrupt = true;
#define	EXIT_IRQ	bIsInterrupt = false;

static void Init_W5500_PinMux(void)
{

	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_8, (IOCON_FUNC1 | IOCON_MODE_INACT));	/* MISO0 */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_9, (IOCON_FUNC1 | IOCON_MODE_INACT));	/* MOSI0 */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_6, (IOCON_FUNC2 | IOCON_MODE_INACT));	/* SCK0 */
	Chip_IOCON_PinLocSel(LPC_IOCON, IOCON_SCKLOC_PIO0_6);


	// SSEL0を出力設定
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_2, (IOCON_FUNC0 | IOCON_MODE_INACT));	/* SSEL0 */
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 2);

	// RESETを出力設定
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_7, (IOCON_FUNC0 | IOCON_MODE_INACT));	/* PIO0_7 used for RSTn */
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, W5500_RESET_PORT, W5500_RESET);

	// INTを入力設定
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_3, (IOCON_FUNC0 | IOCON_MODE_INACT));	/* PIO0_3 used for INTn */
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, W5500_INT_PORT, W5500_INT);

}

#if 0
void PIOINT0_IRQHandler(void)
{
	ENTER_IRQ;

	uint8_t ir, irs;



	ir = GetCOMSocketInterrupt();
	irs = GetSocketInterrupt(0);


	SetSocketInterrupt(0, irs);
	SetCOMSocketInterrupt(ir);

	Chip_GPIO_ClearInts(LPC_GPIO, W5500_INT_PORT, (1 << W5500_INT));

	EXIT_IRQ;
}
#endif

void W5500_Select(void)
{
	// SSEL=LOW
	Chip_GPIO_SetPinState(LPC_GPIO, 0, 2, false);	// SSEL(CS)
}

void W5500_Deselect(void)
{
	// SSEL=HI
	Chip_GPIO_SetPinState(LPC_GPIO, 0, 2, true);	// SSEL(CS)
}

void Reset_W5500(void)
{
	uint16_t tmp;

	// SSEL=HI
	Chip_GPIO_SetPinState(LPC_GPIO, 0, 2, true);	// SSEL(CS)

	Chip_GPIO_SetPinState(LPC_GPIO, W5500_RESET_PORT, W5500_RESET, false);
	/*
	 * 実測で500us以上
	 */
	tmp = 2000;
	while(tmp--);
	Chip_GPIO_SetPinState(LPC_GPIO, W5500_RESET_PORT, W5500_RESET, true);
	/*
	 * 実測で1ms以上
	 */
	tmp = 4000;
	while(tmp--);
}


void Init_W5500(void)
{
	Init_W5500_PinMux();

	Reset_W5500();

	Chip_SSP_Init(LPC_SSP);

	// 割込み使用しない。ブロッキングでRead/Write
	Chip_SSP_SetFormat(LPC_SSP, SSP_BITS_8, SSP_FRAMEFORMAT_SPI, SSP_CLOCK_MODE0);
	Chip_SSP_SetMaster(LPC_SSP, true);		// Master Mode
	Chip_SSP_SetBitRate(LPC_SSP,   400000); // 400KBps
//	Chip_SSP_SetBitRate(LPC_SSP, 10000000); // 10MHz
	Chip_SSP_Enable(LPC_SSP);

#if 0
	/*
	 * 割込み処理
	 */
	Chip_GPIO_SetPinModeEdge(LPC_GPIO, W5500_INT_PORT, (1 << W5500_INT));
	Chip_GPIO_SetEdgeModeSingle(LPC_GPIO, W5500_INT_PORT, (1 << W5500_INT));
	Chip_GPIO_SetModeLow(LPC_GPIO, W5500_INT_PORT, (1 << W5500_INT));
	Chip_GPIO_EnableInt(LPC_GPIO, W5500_INT_PORT, (1 << W5500_INT));

	NVIC_EnableIRQ(EINT0_IRQn);

	SetCOMSocketInterruptMask(0x01);
#endif

}

uint8_t W5500_Read(uint32_t AddrSel)
{
	uint8_t	ret;
	uint8_t	spi_data[3];

	/*
	 * 割込み禁止
	 */
	_DISABLE_IRQ;

	W5500_Select();

	AddrSel |= (W5500_RWB_READ | W5500_VARIABLE_DATA_LENGTH);

	spi_data[0] = (AddrSel & 0x00FF0000) >> 16;
	spi_data[1] = (AddrSel & 0x0000FF00) >> 8;
	spi_data[2] = (AddrSel & 0x000000FF) >> 0;
	Chip_SSP_WriteFrames_Blocking(LPC_SSP0, spi_data, 3);
	Chip_SSP_ReadFrames_Blocking(LPC_SSP0, &ret, 1);

	W5500_Deselect();

	/*
	 * 割込み許可
	 */
	_ENABLE_IRQ;

	return ret;
}

void W5500_Write(uint32_t AddrSel, uint8_t wb)
{
	uint8_t	spi_data[4];

	/*
	 * 割込み禁止
	 */
	_DISABLE_IRQ;

	W5500_Select();

	AddrSel |= (W5500_RWB_WRITE | W5500_VARIABLE_DATA_LENGTH);

	spi_data[0] = (AddrSel & 0x00FF0000) >> 16;
	spi_data[1] = (AddrSel & 0x0000FF00) >> 8;
	spi_data[2] = (AddrSel & 0x000000FF) >> 0;
	spi_data[3] = wb;
	Chip_SSP_WriteFrames_Blocking(LPC_SSP0, spi_data, 4);

	W5500_Deselect();

	/*
	 * 割込み許可
	 */
	_ENABLE_IRQ;
}

void W5500_ReadBuffer(uint32_t AddrSel, uint8_t *pBuf, uint16_t len)
{
	uint8_t	spi_data[3];

	/*
	 * 割込み禁止
	 */
	_DISABLE_IRQ;

	W5500_Select();

	AddrSel |= (W5500_RWB_READ | W5500_VARIABLE_DATA_LENGTH);

	spi_data[0] = (AddrSel & 0x00FF0000) >> 16;
	spi_data[1] = (AddrSel & 0x0000FF00) >> 8;
	spi_data[2] = (AddrSel & 0x000000FF) >> 0;
	Chip_SSP_WriteFrames_Blocking(LPC_SSP0, spi_data, 3);
	Chip_SSP_ReadFrames_Blocking(LPC_SSP0, pBuf, len);

	W5500_Deselect();

	/*
	 * 割込み許可
	 */
	_ENABLE_IRQ;
}

void W5500_WriteBuffer(uint32_t AddrSel, uint8_t *pBuf, uint16_t len)
{
	uint8_t	spi_data[3];

	/*
	 * 割込み禁止
	 */
	_DISABLE_IRQ;

	W5500_Select();

	AddrSel |= (W5500_RWB_WRITE | W5500_VARIABLE_DATA_LENGTH);

	spi_data[0] = (AddrSel & 0x00FF0000) >> 16;
	spi_data[1] = (AddrSel & 0x0000FF00) >> 8;
	spi_data[2] = (AddrSel & 0x000000FF) >> 0;
	Chip_SSP_WriteFrames_Blocking(LPC_SSP0, spi_data, 3);
	Chip_SSP_WriteFrames_Blocking(LPC_SSP0, pBuf, len);

	W5500_Deselect();
	/*
	 * 割込み許可
	 */
	_ENABLE_IRQ;
}

uint32_t GetAddressSelect(uint8_t sn, uint32_t Address)
{
	sn = ((sn * 4) + 1);

	return (Address | (sn << 3));
}

void W5500_SetAddress(uint8_t *pmac, uint8_t *pip, uint8_t *pmask, uint8_t *pgate)
{
	W5500_WriteBuffer(W5500_COM_SOURCE_HARDWARE_ADDRESS, pmac, 6);
	W5500_WriteBuffer(W5500_COM_SUBNET_MASK_ADDRESS, pmask, 4);
	W5500_WriteBuffer(W5500_COM_SOURCE_IP_ADDRESS, pip, 4);
	W5500_WriteBuffer(W5500_COM_GATEWAY_ADDRESS, pgate, 4);

	uint8_t mmm[10];
	W5500_ReadBuffer(W5500_COM_SOURCE_HARDWARE_ADDRESS, mmm, 6);
	W5500_ReadBuffer(W5500_COM_SOURCE_IP_ADDRESS, mmm, 6);

}

void SetRetryTime(uint16_t time) {
	uint8_t bf[2];

	//
	bf[0] = (uint8_t)(time >> 8);
	bf[1] = (uint8_t)(time & 0xff);

	W5500_WriteBuffer(W5500_COM_RETRY_TIME, bf, 2);

}

void SetRetryCount(uint8_t count) {

	W5500_Write(W5500_COM_RETRY_COUNT, count);
}

void SetPHYConfig(uint8_t py) {

	W5500_Write(W5500_COM_PHY_CONFIGURATION, py);
}

uint8_t GetPHYConfig(void) {

	return W5500_Read(W5500_COM_PHY_CONFIGURATION);
}


void SetCOMSocketInterrupt(uint8_t sn_int) {

	W5500_Write(W5500_COM_SOCKET_INTERRUPT, sn_int);
}

void SetCOMSocketInterruptMask(uint8_t mask) {

	W5500_Write(W5500_COM_SOCKET_INTERRUPT_MASK, mask);
}

uint8_t GetCOMSocketInterrupt(void) {

	return W5500_Read(W5500_COM_SOCKET_INTERRUPT);
}

uint8_t GetCOMSocketInterruptMask(void) {

	return W5500_Read(W5500_COM_SOCKET_INTERRUPT_MASK);
}

void SetSocketMode(uint8_t sn, uint8_t md) {

	W5500_Write(GetAddressSelect(sn, W5500_SOCK_MODE), md);

}

void SetSocketCommand(uint8_t sn, uint8_t cm) {

	W5500_Write(GetAddressSelect(sn, W5500_SOCK_COMMAND), cm);

}

void SetSocketInterrupt(uint8_t sn, uint8_t it) {

	W5500_Write(GetAddressSelect(sn, W5500_SOCK_INTERRUPT), it);

}

void SetSocketStatus(uint8_t sn, uint8_t st) {

	W5500_Write(GetAddressSelect(sn, W5500_SOCK_STATUS), st);

}

void SetSocketSourcePort(uint8_t sn, uint16_t port) {
	uint8_t bf[2];

	// 上下逆
	bf[0] = (uint8_t)(port >> 8);
	bf[1] = (uint8_t)(port & 0xff);

	W5500_WriteBuffer(GetAddressSelect(sn, W5500_SOCK_SOURCE_PORT), bf, 2);

}

void SetSocketDestnationHardwareAddress(uint8_t sn, uint8_t *pMac) {

	W5500_WriteBuffer(GetAddressSelect(sn, W5500_SOCK_DESTINATION_HARDWARE_ADDRESS), pMac, 6);

}

void SetSocketDestnationIP(uint8_t sn, uint8_t *pIp) {

	W5500_WriteBuffer(GetAddressSelect(sn, W5500_SOCK_DESTINATION_IP_ADDRESS), pIp, 4);

}

void SetSocketDestnationPort(uint8_t sn, uint16_t port) {

	uint8_t bf[2];

	// 上下逆
	bf[0] = (uint8_t)(port >> 8);
	bf[1] = (uint8_t)(port & 0xff);

	W5500_WriteBuffer(GetAddressSelect(sn, W5500_SOCK_DESTINATION_PORT), bf, 2);

}

void SetSocketMaximumSegmentSize(uint8_t sn, uint16_t szSeg) {

	uint8_t bf[2];

	// ��ʁE���ʂ��t�Ȃ��Ƃɒ���
	bf[0] = (uint8_t)(szSeg >> 8);
	bf[1] = (uint8_t)(szSeg & 0xff);

	W5500_WriteBuffer(GetAddressSelect(sn, W5500_SOCK_MAXIMUM_SEGMENT_SIZE), bf, 2);

}

void SetSocketTOS(uint8_t sn, uint8_t tos) {

	W5500_Write(GetAddressSelect(sn, W5500_SOCK_IP_TOS), tos);

}

void SetSocketTTL(uint8_t sn, uint8_t ttl) {


	W5500_Write(GetAddressSelect(sn, W5500_SOCK_IP_TTL), ttl);

}

void SetSocketReceiveBufferSize(uint8_t sn, uint8_t size) {

	W5500_Write(GetAddressSelect(sn, W5500_SOCK_RECEIVE_BUFFER_SIZE), size);

}

void SetSocketTransmitBufferSize(uint8_t sn, uint8_t size) {

	W5500_Write(GetAddressSelect(sn, W5500_SOCK_TRANSMIT_BUFFER_SIZE), size);

}

void SetSocketTXFreeSize(uint8_t sn, uint16_t size) {
	uint8_t bf[2];

	// ��ʁE���ʂ��t�Ȃ��Ƃɒ���
	bf[0] = (uint8_t)(size >> 8);
	bf[1] = (uint8_t)(size & 0xff);

	W5500_WriteBuffer(GetAddressSelect(sn, W5500_SOCK_TX_FREE_SIZE), bf, 2);

}

void SetSocketTXReadPointer(uint8_t sn, uint16_t pointer) {
	uint8_t bf[2];

	// ��ʁE���ʂ��t�Ȃ��Ƃɒ���
	bf[0] = (uint8_t)(pointer >> 8);
	bf[1] = (uint8_t)(pointer & 0xff);

	W5500_WriteBuffer(GetAddressSelect(sn, W5500_SOCK_TX_READ_POINTER), bf, 2);

}

void SetSocketTXWritePointer(uint8_t sn, uint16_t pointer) {
	uint8_t bf[2];

	// ��ʁE���ʂ��t�Ȃ��Ƃɒ���
	bf[0] = (uint8_t)(pointer >> 8);
	bf[1] = (uint8_t)(pointer & 0xff);

	W5500_WriteBuffer(GetAddressSelect(sn, W5500_SOCK_TX_WRITE_POINTER), bf, 2);

}
void SetSocketRXReceivedSize(uint8_t sn, uint16_t size) {
	uint8_t bf[2];

	// ��ʁE���ʂ��t�Ȃ��Ƃɒ���
	bf[0] = (uint8_t)(size >> 8);
	bf[1] = (uint8_t)(size & 0xff);

	W5500_WriteBuffer(GetAddressSelect(sn, W5500_SOCK_RX_RECEIVED_SIZE), bf, 2);

}

void SetSocketRXReadPointer(uint8_t sn, uint16_t pointer) {
	uint8_t bf[2];

	// ��ʁE���ʂ��t�Ȃ��Ƃɒ���
	bf[0] = (uint8_t)(pointer >> 8);
	bf[1] = (uint8_t)(pointer & 0xff);

	W5500_WriteBuffer(GetAddressSelect(sn, W5500_SOCK_RX_READ_POINTER), bf, 2);

}

void SetSocketRXWritePointer(uint8_t sn, uint16_t pointer) {
	uint8_t bf[2];

	// ��ʁE���ʂ��t�Ȃ��Ƃɒ���
	bf[0] = (uint8_t)(pointer >> 8);
	bf[1] = (uint8_t)(pointer & 0xff);

	W5500_WriteBuffer(GetAddressSelect(sn, W5500_SOCK_RX_WRITE_POINTER), bf, 2);

}

void SetSocketInterruptMask(uint8_t sn, uint8_t mask) {

	W5500_Write(GetAddressSelect(sn, W5500_SOCK_INTERRUPT_MASK), mask);

}

void SetSocketFragmentOffsetinIPHeader(uint8_t sn, uint16_t flag) {
	uint8_t bf[2];

	// ��ʁE���ʂ��t�Ȃ��Ƃɒ���
	bf[0] = (uint8_t)(flag >> 8);
	bf[1] = (uint8_t)(flag & 0xff);

	W5500_WriteBuffer(GetAddressSelect(sn, W5500_SOCK_FRAGMENT_OFFSET_IN_IP_HEADER), bf, 2);

}

void SetSocketKeepAliveTimer(uint8_t sn, uint8_t time) {

	W5500_Write(GetAddressSelect(sn, W5500_SOCK_KEEP_ALIVE_TIMER), time);

}


uint8_t GetSocketMode(uint8_t sn) {

	return W5500_Read(GetAddressSelect(sn, W5500_SOCK_MODE));

}

uint8_t GetSocketCommand(uint8_t sn) {

	return W5500_Read(GetAddressSelect(sn, W5500_SOCK_COMMAND));

}

uint8_t GetSocketInterrupt(uint8_t sn) {

	return W5500_Read(GetAddressSelect(sn, W5500_SOCK_INTERRUPT));

}

uint8_t GetSocketStatus(uint8_t sn) {

	return W5500_Read(GetAddressSelect(sn, W5500_SOCK_STATUS));

}

uint16_t GetSocketSourcePort(uint8_t sn) {

	uint16_t port;
	uint8_t bf[2];

	W5500_ReadBuffer(GetAddressSelect(sn, W5500_SOCK_SOURCE_PORT), bf, 2);

	// ��ʁE���ʂ��t�Ȃ��Ƃɒ���
	port = bf[0];
	port <<= 8;
	port |= bf[1];

	return port;
}

void GetSocketDestnationHardwareAddress(uint8_t sn, uint8_t *pMac) {

	return W5500_ReadBuffer(GetAddressSelect(sn, W5500_SOCK_DESTINATION_HARDWARE_ADDRESS), pMac, 6);

}

void GetSocketDestnationIP(uint8_t sn, uint8_t *pIp) {

	W5500_ReadBuffer(GetAddressSelect(sn,W5500_SOCK_DESTINATION_IP_ADDRESS), pIp, 4);
}

uint16_t GetSocketDestnationPort(uint8_t sn) {

	uint16_t port;
	uint8_t bf[2];

	W5500_ReadBuffer(GetAddressSelect(sn,W5500_SOCK_DESTINATION_PORT), bf, 2);

	// ��ʁE���ʂ��t�Ȃ��Ƃɒ���
	port = bf[0];
	port <<= 8;
	port |= bf[1];

	return port;

}

uint16_t GetSocketMaximumSegmentSize(uint8_t sn) {

	uint16_t szSeg;
	uint8_t bf[2];

	W5500_ReadBuffer(GetAddressSelect(sn,W5500_SOCK_MAXIMUM_SEGMENT_SIZE), bf, 2);

	// ��ʁE���ʂ��t�Ȃ��Ƃɒ���
	szSeg = bf[0];
	szSeg <<= 8;
	szSeg |= bf[1];

	return szSeg;

}

uint8_t GetSocketTOS(uint8_t sn) {

	return W5500_Read(GetAddressSelect(sn, W5500_SOCK_IP_TOS));

}

uint8_t GetSocketTTL(uint8_t sn) {

	return W5500_Read(GetAddressSelect(sn, W5500_SOCK_IP_TTL));

}

uint16_t GetSocketReceiveBufferSize(uint8_t sn) {

	return (W5500_Read(GetAddressSelect(sn, W5500_SOCK_RECEIVE_BUFFER_SIZE)) << 10);

}

uint16_t GetSocketTransmitBufferSize(uint8_t sn) {

	return (W5500_Read(GetAddressSelect(sn, W5500_SOCK_TRANSMIT_BUFFER_SIZE)) << 10) ;

}

uint16_t GetSocketTXFreeSize(uint8_t sn) {

	uint16_t size;
	uint8_t bf[2];

	W5500_ReadBuffer(GetAddressSelect(sn,W5500_SOCK_TX_FREE_SIZE), bf, 2);

	// ��ʁE���ʂ��t�Ȃ��Ƃɒ���
	size = bf[0];
	size <<= 8;
	size |= bf[1];

	return size;

}

uint16_t GetSocketTXReadPointer(uint8_t sn) {

	uint16_t pointer;
	uint8_t bf[2];

	W5500_ReadBuffer(GetAddressSelect(sn,W5500_SOCK_TX_READ_POINTER), bf, 2);

	// ��ʁE���ʂ��t�Ȃ��Ƃɒ���
	pointer = bf[0];
	pointer <<= 8;
	pointer |= bf[1];

	return pointer;

}

uint16_t GetSocketTXWritePointer(uint8_t sn) {

	uint16_t pointer;
	uint8_t bf[2];

	W5500_ReadBuffer(GetAddressSelect(sn,W5500_SOCK_TX_WRITE_POINTER), bf, 2);

	// ��ʁE���ʂ��t�Ȃ��Ƃɒ���
	pointer = bf[0];
	pointer <<= 8;
	pointer |= bf[1];

	return pointer;

}
uint16_t GetSocketRXReceivedSize(uint8_t sn) {

	uint16_t size;
	uint8_t bf[2];

	W5500_ReadBuffer(GetAddressSelect(sn,W5500_SOCK_RX_RECEIVED_SIZE), bf, 2);

	// ��ʁE���ʂ��t�Ȃ��Ƃɒ���
	size = bf[0];
	size <<= 8;
	size |= bf[1];

	return size;

}

uint16_t GetSocketRXReadPointer(uint8_t sn) {

	uint16_t pointer;
	uint8_t bf[2];

	W5500_ReadBuffer(GetAddressSelect(sn,W5500_SOCK_RX_READ_POINTER), bf, 2);

	// ��ʁE���ʂ��t�Ȃ��Ƃɒ���
	pointer = bf[0];
	pointer <<= 8;
	pointer |= bf[1];

	return pointer;

}

uint16_t GetSocketRXWritePointer(uint8_t sn) {

	uint16_t pointer;
	uint8_t bf[2];

	W5500_ReadBuffer(GetAddressSelect(sn,W5500_SOCK_RX_WRITE_POINTER), bf, 2);

	// ��ʁE���ʂ��t�Ȃ��Ƃɒ���
	pointer = bf[0];
	pointer <<= 8;
	pointer |= bf[1];

	return pointer;

}

uint8_t GetSocketInterruptMask(uint8_t sn) {

	return W5500_Read(GetAddressSelect(sn, W5500_SOCK_INTERRUPT_MASK));

}

uint16_t GetSocketFragmentOffsetinIPHeader(uint8_t sn) {

	int16_t head;
	uint8_t bf[2];

	W5500_ReadBuffer(GetAddressSelect(sn,W5500_SOCK_FRAGMENT_OFFSET_IN_IP_HEADER), bf, 2);

	// ��ʁE���ʂ��t�Ȃ��Ƃɒ���
	head = bf[0];
	head <<= 8;
	head |= bf[1];

	return head;

}

uint8_t GetSocketKeepAliveTimer(uint8_t sn) {

	return W5500_Read(GetAddressSelect(sn, W5500_SOCK_KEEP_ALIVE_TIMER));

}

uint16_t GetSocketSendFreeSize(uint8_t sn)
{
   uint16_t	val		= 0;
   uint16_t	val1	= 0;

   do {
	   val1 = GetSocketTXFreeSize(sn);
	   if (val1 != 0) {
		   val = GetSocketTXFreeSize(sn);
	   }
   } while (val != val1);

   return val;
}


uint16_t GetSocketReceivedSize(uint8_t sn)
{
	   uint16_t	val		= 0;
	   uint16_t	val1	= 0;

	   do {
		   val1 = GetSocketRXReceivedSize(sn);
		   if (val1 != 0) {
			   val = GetSocketRXReceivedSize(sn);
		   }
	   } while (val != val1);

	   return val;
}

void W5500_Send_Data(uint8_t sn, uint8_t *pBuffer, uint16_t len)
{
   uint16_t ptr = 0;
   uint32_t AddrSel = 0;

   if(len == 0) {
	   return;
   }

   ptr = GetSocketTXWritePointer(sn);

   AddrSel = ((uint32_t)ptr << 8) + (((sn * 4) + 2) << 3);

   W5500_WriteBuffer(AddrSel, pBuffer, len);

   ptr += len;

   SetSocketTXWritePointer(sn, ptr);
}

void W5500_Receive_Data(uint8_t sn, uint8_t *pBuffer, uint16_t len)
{
	   uint16_t ptr = 0;
	   uint32_t AddrSel = 0;

	   if(len == 0) {
		   return;
	   }

	   ptr = GetSocketRXReadPointer(sn);

	   AddrSel = ((uint32_t)ptr << 8) + (((sn * 4) + 3) << 3);

	   W5500_ReadBuffer(AddrSel, pBuffer, len);

	   ptr += len;

	   SetSocketRXReadPointer(sn, ptr);
}

void W5500_Receive_Ignore(uint8_t sn, uint16_t len)
{
   uint16_t ptr = 0;

   ptr = GetSocketRXReadPointer(sn);
   ptr += len;
   SetSocketRXReadPointer(sn,ptr);
}
