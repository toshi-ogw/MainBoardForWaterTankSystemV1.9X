/*
 * w5500.h
 *
 *  Created on: 2016/03/23
 *      Author: toshio
 */

#ifndef DRIVER_W5500_H_
#define DRIVER_W5500_H_

#include "chip.h"
#include <cr_section_macros.h>



void Interrupt__Disable(void);
void Interrupt__Enable(void);

#define	_DISABLE_IRQ	Interrupt__Disable()
#define	_ENABLE_IRQ		Interrupt__Enable()



#define	W5500_BSD_COMMON_REG					(0 << 3)

#define	W5500_BSD_SOCKET0_REG					(1 << 3)
#define	W5500_BSD_SOCKET0_TX_BUFFER				(2 << 3)
#define	W5500_BSD_SOCKET0_RX_BUFFER				(3 << 3)

#define	W5500_BSD_SOCKET1_REG					(5 << 3)
#define	W5500_BSD_SOCKET1_TX_BUFFER				(6 << 3)
#define	W5500_BSD_SOCKET1_RX_BUFFER				(7 << 3)

#define	W5500_BSD_SOCKET2_REG					(9 << 3)
#define	W5500_BSD_SOCKET2_TX_BUFFER				(10 << 3)
#define	W5500_BSD_SOCKET2_RX_BUFFER				(11 << 3)

#define	W5500_BSD_SOCKET3_REG					(13 << 3)
#define	W5500_BSD_SOCKET3_TX_BUFFER				(14 << 3)
#define	W5500_BSD_SOCKET3_RX_BUFFER				(15 << 3)

#define	W5500_BSD_SOCKET4_REG					(17 << 3)
#define	W5500_BSD_SOCKET4_TX_BUFFER				(18 << 3)
#define	W5500_BSD_SOCKET4_RX_BUFFER				(19 << 3)

#define	W5500_BSD_SOCKET5_REG					(21 << 3)
#define	W5500_BSD_SOCKET5_TX_BUFFER				(22 << 3)
#define	W5500_BSD_SOCKET5_RX_BUFFER				(23 << 3)

#define	W5500_BSD_SOCKET6_REG					(25 << 3)
#define	W5500_BSD_SOCKET6_TX_BUFFER				(26 << 3)
#define	W5500_BSD_SOCKET6_RX_BUFFER				(27 << 3)

#define	W5500_BSD_SOCKET7_REG					(29 << 3)
#define	W5500_BSD_SOCKET7_TX_BUFFER				(30 << 3)
#define	W5500_BSD_SOCKET7_RX_BUFFER				(31 << 3)

#define	W5500_RWB_READ							(0 << 2)
#define	W5500_RWB_WRITE							(1 << 2)

#define	W5500_VARIABLE_DATA_LENGTH				0
#define	W5500_1BYTE_DATA_LENGTH					1
#define	W5500_2BYTE_DATA_LENGTH					2
#define	W5500_4BYTE_DATA_LENGTH					3



// Common Register Block Offset

#define	W5500_COM_MODE							(uint32_t)(0x0000 << 8)
#define	W5500_COM_GATEWAY_ADDRESS				(uint32_t)(0x0001 << 8)
#define	W5500_COM_SUBNET_MASK_ADDRESS			(uint32_t)(0x0005 << 8)
#define	W5500_COM_SOURCE_HARDWARE_ADDRESS		(uint32_t)(0x0009 << 8)
#define	W5500_COM_SOURCE_IP_ADDRESS				(uint32_t)(0x000f << 8)
#define	W5500_COM_INTERRUPT_LOW_LEVEL_TIMER		(uint32_t)(0x0013 << 8)
#define	W5500_COM_INTERRUPT						(uint32_t)(0x0015 << 8)
#define	W5500_COM_INTERRUPT_MASK				(uint32_t)(0x0016 << 8)
#define	W5500_COM_SOCKET_INTERRUPT				(uint32_t)(0x0017 << 8)
#define	W5500_COM_SOCKET_INTERRUPT_MASK			(uint32_t)(0x0018 << 8)
#define	W5500_COM_RETRY_TIME					(uint32_t)(0x0019 << 8)
#define	W5500_COM_RETRY_COUNT					(uint32_t)(0x001b << 8)
#define	W5500_COM_PPP_LCP_REQUEST_TIMER			(uint32_t)(0x001c << 8)
#define	W5500_COM_PPP_LCP_MAGIC_NUMBER			(uint32_t)(0x001d << 8)
#define	W5500_COM_PPP_DESTINATION_MAC_ADDRESS	(uint32_t)(0x001e << 8)
#define	W5500_COM_PPP_SESSION_IDENTIFICATION	(uint32_t)(0x0024 << 8)
#define	W5500_COM_PPP_MAXIMUM_SEGMENT_SIZE		(uint32_t)(0x0026 << 8)
#define	W5500_COM_UNREACHABLE_IP_ADDRESS		(uint32_t)(0x0028 << 8)
#define	W5500_COM_UNREACHABLE_PORT				(uint32_t)(0x002c << 8)
#define	W5500_COM_PHY_CONFIGURATION				(uint32_t)(0x002e << 8)
#define	W5500_COM_CHIP_VERSION					(uint32_t)(0x0039 << 8)


// Socket Register Block Offset

#define	W5500_SOCK_MODE							(uint32_t)(0x0000 << 8)
#define	W5500_SOCK_COMMAND						(uint32_t)(0x0001 << 8)
#define	W5500_SOCK_INTERRUPT					(uint32_t)(0x0002 << 8)
#define	W5500_SOCK_STATUS						(uint32_t)(0x0003 << 8)
#define	W5500_SOCK_SOURCE_PORT					(uint32_t)(0x0004 << 8)
#define	W5500_SOCK_DESTINATION_HARDWARE_ADDRESS	(uint32_t)(0x0006 << 8)
#define	W5500_SOCK_DESTINATION_IP_ADDRESS		(uint32_t)(0x000c << 8)
#define	W5500_SOCK_DESTINATION_PORT				(uint32_t)(0x0010 << 8)
#define	W5500_SOCK_MAXIMUM_SEGMENT_SIZE			(uint32_t)(0x0012 << 8)
#define	W5500_SOCK_IP_TOS						(uint32_t)(0x0015 << 8)
#define	W5500_SOCK_IP_TTL						(uint32_t)(0x0016 << 8)
#define	W5500_SOCK_RECEIVE_BUFFER_SIZE			(uint32_t)(0x001e << 8)
#define	W5500_SOCK_TRANSMIT_BUFFER_SIZE			(uint32_t)(0x001f << 8)
#define	W5500_SOCK_TX_FREE_SIZE					(uint32_t)(0x0020 << 8)
#define	W5500_SOCK_TX_READ_POINTER				(uint32_t)(0x0022 << 8)
#define	W5500_SOCK_TX_WRITE_POINTER				(uint32_t)(0x0024 << 8)
#define	W5500_SOCK_RX_RECEIVED_SIZE				(uint32_t)(0x0026 << 8)
#define	W5500_SOCK_RX_READ_POINTER				(uint32_t)(0x0028 << 8)
#define	W5500_SOCK_RX_WRITE_POINTER				(uint32_t)(0x002a << 8)
#define	W5500_SOCK_INTERRUPT_MASK				(uint32_t)(0x002c << 8)
#define	W5500_SOCK_FRAGMENT_OFFSET_IN_IP_HEADER	(uint32_t)(0x002d << 8)
#define	W5500_SOCK_KEEP_ALIVE_TIMER				(uint32_t)(0x002f << 8)



void Init_W5500(void);
void Reset_W5500(void);

void W5500_WriteBuffer(uint32_t AddrSel, uint8_t *pBuf, uint16_t len);
void W5500_ReadBuffer(uint32_t AddrSel, uint8_t *pBuf, uint16_t len);
void W5500_Write(uint32_t AddrSel, uint8_t dat);
uint8_t W5500_Read(uint32_t AddrSel);

void W5500_SetAddress(uint8_t *pmac, uint8_t *pip, uint8_t *pmask, uint8_t *pgate);

void SetRetryTime(uint16_t time);
void SetRetryCount(uint8_t count);
void SetPHYConfig(uint8_t py);
uint8_t GetPHYConfig(void);

void SetCOMSocketInterrupt(uint8_t sn_int);
void SetCOMSocketInterruptMask(uint8_t mask);
uint8_t GetCOMSocketInterrupt(void);
uint8_t GetCOMSocketInterruptMask(void);



void SetSocketMode(uint8_t sn, uint8_t md);
void SetSocketCommand(uint8_t sn, uint8_t cm);
void SetSocketInterrupt(uint8_t sn, uint8_t it);
void SetSocketStatus(uint8_t sn, uint8_t st);
void SetSocketSourcePort(uint8_t sn, uint16_t port);
void SetSocketDestnationHardwareAddress(uint8_t sn, uint8_t *pMac);
void SetSocketDestnationIP(uint8_t sn, uint8_t *pIp);
void SetSocketDestnationPort(uint8_t sn, uint16_t port);
void SetSocketMaximumSegmentSize(uint8_t sn, uint16_t szSeg);
void SetSocketTOS(uint8_t sn, uint8_t tos);
void SetSocketTTL(uint8_t sn, uint8_t ttl);
void SetSocketReceiveBufferSize(uint8_t sn, uint8_t size);
void SetSocketTransmitBufferSize(uint8_t sn, uint8_t size);
void SetSocketTXFreeSize(uint8_t sn, uint16_t size);
void SetSocketTXReadPointer(uint8_t sn, uint16_t pointer);
void SetSocketTXWritePointer(uint8_t sn, uint16_t pointer);
void SetSocketRXReceivedSize(uint8_t sn, uint16_t size);
void SetSocketRXReadPointer(uint8_t sn, uint16_t pointer);
void SetSocketRXWritePointer(uint8_t sn, uint16_t pointer);
void SetSocketInterruptMask(uint8_t sn, uint8_t mask);
void SetSocketFragmentOffsetinIPHeader(uint8_t sn, uint16_t flag);
void SetSocketKeepAliveTimer(uint8_t sn, uint8_t time);
uint8_t GetSocketMode(uint8_t sn);
uint8_t GetSocketCommand(uint8_t sn);
uint8_t GetSocketInterrupt(uint8_t sn);
uint8_t GetSocketStatus(uint8_t sn);
uint16_t GetSocketSourcePort(uint8_t sn);
void GetSocketDestnationHardwareAddress(uint8_t sn, uint8_t *pMac);
void GetSocketDestnationIP(uint8_t sn, uint8_t *pIp);
uint16_t GetSocketDestnationPort(uint8_t sn);
uint16_t GetSocketMaximumSegmentSize(uint8_t sn);
uint8_t GetSocketTOS(uint8_t sn);
uint8_t GetSocketTTL(uint8_t sn);
uint16_t GetSocketReceiveBufferSize(uint8_t sn);
uint16_t GetSocketTransmitBufferSize(uint8_t sn);
uint16_t GetSocketTXFreeSize(uint8_t sn);
uint16_t GetSocketTXReadPointer(uint8_t sn);
uint16_t GetSocketTXWritePointer(uint8_t sn);
uint16_t GetSocketRXReceivedSize(uint8_t sn);
uint16_t GetSocketRXReadPointer(uint8_t sn);
uint16_t GetSocketRXWritePointer(uint8_t sn);
uint8_t GetSocketInterruptMask(uint8_t sn);
uint16_t GetSocketFragmentOffsetinIPHeader(uint8_t sn);
uint8_t GetSocketKeepAliveTimer(uint8_t sn);



uint16_t GetSocketSendFreeSize(uint8_t sn);
uint16_t GetSocketReceivedSize(uint8_t sn);

void W5500_Send_Data(uint8_t sn, uint8_t *pBuffer, uint16_t len);
void W5500_Receive_Data(uint8_t sn, uint8_t *pBuffer, uint16_t len);
void W5500_Receive_Ignore(uint8_t sn, uint16_t len);

void Init_NetDevice(void);
void Restart_NetDevice(void);

#endif /* DRIVER_W5500_H_ */
