/*
 * http.h
 *
 *  Created on: 2016/04/01
 *      Author: toshio
 */

#ifndef NET_HTTP_H_
#define NET_HTTP_H_

#include "chip.h"
#include <cr_section_macros.h>

#include "socket.h"
#include "baseclock.h"


uint8_t *HTTP_GetPhrase(uint8_t *pPhrase, uint8_t *pBuffer, uint32_t length);
uint8_t *HTTP_GetTag(uint8_t *pTag, uint8_t *pBuffer, uint32_t length);


/*
 * 　HTTPステータス
 */
#define	HTTP_REQ_IDLE			0
#define	HTTP_REQ_START			1
#define	HTTP_REQ_CONNECT		2
#define	HTTP_REQ_SEND			3
#define	HTTP_REQ_RECV			4
#define	HTTP_REQ_DISCONNECT		5
#define	HTTP_REQ_COMPLETE		6
#define	HTTP_REQ_ABORT			-1

typedef struct {
	uint8_t Socket;
	uint8_t *pSrvAddress;
	uint8_t *pUrn;
	uint16_t port;
	uint32_t txLength;
	uint32_t rxLength;
	uint8_t	*ptxBuffer;
	uint8_t	*prxBuffer;
	uint32_t ReceivedLength;
	int16_t HTTPStatusCode;
	int16_t Result;
} HTTP_REQUEST_VALUE, *PHTTP_REQUEST_VALUE;

void SetHTTP_RequestStatus(int8_t stat);
int8_t GetHTTP_RequestStatus(void);
int8_t HTTP_Request(PHTTP_REQUEST_VALUE pHTTPRequest);

/*
 * 	Listenデータ
 */
typedef struct {
	int8_t		nSocket;					// ソケット番号
	uint16_t 	Port;						// ポート番号
	uint8_t		*ptxBuffer;					// 送信バッファ
	int32_t		txLength;					// 送信データ長
	uint8_t		*prxBuffer;					// 受信バッファ
	int32_t		szrxBuffer;					// 受信バッファサイズ
	int32_t		rxLength;					// 受信データ長
	int8_t		(*Processing)(void *pDat);	// 処理関数
} NET_LISTEN_DATA, *PNET_LISTEN_DATA;

void HTTP_ListenReset(void);
void HTTP_Listen(PNET_LISTEN_DATA pOPEData);

extern	uint32_t	HTTP_JST_Time;

#define	BASE_CLOCK_REFLESH	UpdateBaseClock(HTTP_JST_Time)

/*
 * デバイスリスタート回数
 */
extern uint8_t	_DeviceRestartCount;


#endif /* NET_HTTP_H_ */
