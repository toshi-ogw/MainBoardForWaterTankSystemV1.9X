/*
 * socket.h
 *
 *  Created on: 2016/03/22
 *      Author: toshio
 */


#ifndef NET_SOCKET_H_
#define NET_SOCKET_H_

#include "chip.h"
#include <cr_section_macros.h>

#include "w5500.h"


#define	SOCK_TCP		0x01
#define	SOCK_UDP		0x02
#define	SOCK_MACRAW		0x04

// -- Sn_CR
#define COMMAND_SOCK_OPEN            0x01
#define COMMAND_SOCK_LISTEN          0x02
#define COMMAND_SOCK_CONNECT         0x04
#define COMMAND_SOCK_DISCON          0x08
#define COMMAND_SOCK_CLOSE           0x10
#define COMMAND_SOCK_SEND            0x20
#define COMMAND_SOCK_SEND_MAC        0x21
#define COMMAND_SOCK_SEND_KEEP       0x22
#define COMMAND_SOCK_RECV            0x40

//-- Sn_IR
#define Sn_IR_SENDOK                 0x10
#define Sn_IR_TIMEOUT                0x08
#define Sn_IR_RECV                   0x04
#define Sn_IR_DISCON                 0x02
#define Sn_IR_CON                    0x01

//-- Sn_SR
#define STATUS_SOCK_CLOSED           0x00
#define STATUS_SOCK_INIT             0x13
#define STATUS_SOCK_LISTEN           0x14
#define STATUS_SOCK_SYNSENT          0x15
#define STATUS_SOCK_SYNRECV          0x16
#define STATUS_SOCK_ESTABLISHED      0x17
#define STATUS_SOCK_FIN_WAIT         0x18
#define STATUS_SOCK_CLOSING          0x1A
#define STATUS_SOCK_TIME_WAIT        0x1B
#define STATUS_SOCK_CLOSE_WAIT       0x1C
#define STATUS_SOCK_LAST_ACK         0x1D
#define STATUS_SOCK_UDP              0x22
#define STATUS_SOCK_MACRAW           0x42




#define SOCK_OK               1        ///< Result is OK about socket process.
#define SOCK_BUSY             0        ///< Socket is busy on processing the operation. Valid only Non-block IO Mode.
#define SOCK_FATAL            -1000    ///< Result is fatal error about socket process.

#define SOCK_ERROR            0
#define SOCKERR_SOCKNUM       (SOCK_ERROR - 1)     ///< Invalid socket number
#define SOCKERR_SOCKOPT       (SOCK_ERROR - 2)     ///< Invalid socket option
#define SOCKERR_SOCKINIT      (SOCK_ERROR - 3)     ///< Socket is not initialized
#define SOCKERR_SOCKCLOSED    (SOCK_ERROR - 4)     ///< Socket unexpectedly closed.
#define SOCKERR_SOCKMODE      (SOCK_ERROR - 5)     ///< Invalid socket mode for socket operation.
#define SOCKERR_SOCKFLAG      (SOCK_ERROR - 6)     ///< Invalid socket flag
#define SOCKERR_SOCKSTATUS    (SOCK_ERROR - 7)     ///< Invalid socket status for socket operation.
#define SOCKERR_ARG           (SOCK_ERROR - 10)    ///< Invalid argrument.
#define SOCKERR_PORTZERO      (SOCK_ERROR - 11)    ///< Port number is zero
#define SOCKERR_IPINVALID     (SOCK_ERROR - 12)    ///< Invalid IP address
#define SOCKERR_TIMEOUT       (SOCK_ERROR - 13)    ///< Timeout occurred
#define SOCKERR_DATALEN       (SOCK_ERROR - 14)    ///< Data length is zero or greater than buffer max size.
#define SOCKERR_BUFFER        (SOCK_ERROR - 15)    ///< Socket buffer is not enough for data communication.

#define SOCKFATAL_PACKLEN     (SOCK_FATAL - 1)     ///< Invalid packet length. Fatal Error.


/*
 * 接続ステータス
 */
#define	SOCK_CONNECT_STATUS_IDLE	0
#define	SOCK_WAIT_FOR_CONNECT		1
#define	SOCK_CONNECT_COMPLETE		2
#define	SOCK_CONNECT_ABORT			-1

/*
 * 切断ステータス
 */
#define	SOCK_DISCONNECT_STATUS_IDLE		0
#define	SOCK_WAIT_FOR_DISCONNECT		1
#define	SOCK_DISCONNECT_COMPLETE		2
#define	SOCK_DISCONNECT_ABORT			-1


int8_t socket(uint8_t sn, uint8_t protocol, uint16_t port, uint8_t flag);
int8_t close_socket(uint8_t);
int8_t listen(uint8_t sn);
int8_t nonBlockingConnect(uint8_t sn, uint8_t * addr, uint16_t port);
int8_t nonBlockingDisconnect(uint8_t sn);

int8_t connect(uint8_t sn, uint8_t * addr, uint16_t port);
int8_t disconnect(uint8_t sn);
int32_t send(uint8_t sn, uint8_t *pbuf, uint16_t len);
int32_t recv(uint8_t sn, uint8_t *pbuf, uint16_t len);
int32_t recv_for_Client(uint8_t sn, uint8_t *pBuffer, uint16_t len);

int32_t sendString(uint8_t sn, uint8_t *pString);

extern uint16_t gSurcePortNumber;

#endif /* NET_SOCKET_H_ */
