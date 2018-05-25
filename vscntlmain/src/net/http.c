/*
 * http.c
 *
 *  Created on: 2016/04/01
 *      Author: toshio
 */

#include "http.h"
#include "tick.h"
#include "pack.h"
#include "string.h"
#include "netdevice.h"

#define	HTTP_CONNECT_RETRY_CNT	3

//uint8_t	*pstrRequestHeader =	(uint8_t *)"GET /get_para.php?name=zz HTTP/1.0\r\n\r\n";
//uint8_t rxBuffer[256];



/*
 * HTTP_Requestのステータス
 */
int8_t HTTP_RequestStatus = HTTP_REQ_IDLE;


/*
 * HTTPサーバーへGETするたびにJSTを更新しとく
 */
uint32_t	HTTP_JST_Time = 0;

uint32_t	dbgConnectTime;
uint32_t	dbgSendTime;
uint32_t	dbgRecvTime;

/*
 * デバイスリスタート回数
 */

uint8_t	_DeviceRestartCount = 0;


/*
 * W5500をハードリセット
 */
void HTTP_HardReset(void)
{
	 /*
	  *  デバイスハードリセット
	 */
	Restart_NetDevice();
	HTTP_ListenReset();
	/*
	 * デバイスリスタート回数
	 */
	_DeviceRestartCount++;

}

int16_t HTTP_GetStatus(uint8_t *pBuffer)
{
	uint8_t *pCode = pBuffer + strlen("HTTP/1.X ");
	int16_t StatusCode;
	int i;

	StatusCode = 0;

	for (i = 0; i < 3; i++) {
		if (*pCode < '0' || *pCode > '9') {
			return -1;
		}
		StatusCode *= 10;
		StatusCode += (*pCode - '0');
		pCode++;
	}

	return StatusCode;
}

uint8_t *HTTP_GetTag(uint8_t *pTag, uint8_t *pBuffer, uint32_t length)
{
	uint8_t *pConst = pTag;
	uint8_t *pBuff	= pBuffer;
	uint32_t i;

	for (i = 0; i < length; i++) {
		if (*pTag == *pBuff) {
			pConst = pTag;
			while(true) {
				if (*pConst == 0 && *pBuff == ' ') {
					/*
					 * 見つかった
					 */
					return (pBuff + 1);
				}
				else if (*pConst != *pBuff) {
					/*
					 * 違う
					 */
					break;
				}
				pConst++;
				pBuff++;
			}
		}
		pBuff++;
	}

	return 0;
}

uint8_t *HTTP_GetPhrase(uint8_t *pPhrase, uint8_t *pBuffer, uint32_t length)
{
	uint8_t *pConst = pPhrase;
	uint8_t *pBuff	= pBuffer;
	uint32_t i;

	for (i = 0; i < length; i++) {
		if (*pPhrase == *pBuff) {
			pConst = pPhrase;
			while(true) {
				if (*pConst == 0) {
					/*
					 * 見つかった
					 */
					return pBuff;
				}
				else if (*pConst != *pBuff) {
					/*
					 * 違う
					 */
					break;
				}
				pConst++;
				pBuff++;
			}
		}
		pBuff++;
	}

	return 0;
}

/*
 * 時間を秒で取得するGMT or UTCと決め付け！
 */
int32_t HTTP_GetBaseClock(uint8_t *pBuffer, uint32_t length)
{
	uint8_t *pDateline;
	uint8_t *pTime;
	int32_t BaseTime, dt;

	pDateline = HTTP_GetTag((uint8_t *)"Date:", pBuffer, length);

	if (!pDateline) {
		/*
		 * なんか違う
		 */
		return -1;
	}

	/*
	 * 時刻位置を見つける
	 */
	pTime = pDateline;
	while(true) {
		if (pTime >= (pBuffer + length)) {
			/*
			 * 見つからん！
			 */
			return -1;
		}
		if (*pTime == ':') {
			/*
			 * 見つかった
			 */
			/*
			 * 時間
			 */
			dt = decStr2Value(pTime - 2, 2);
			if (dt < 0) {
				/*
				 * エラーじゃ
				 */
				return -1;
			}
			BaseTime = dt * 60 * 60;
			/*
			 * 分
			 */
			dt = decStr2Value(pTime + 1, 2);
			if (dt < 0) {
				/*
				 * エラーじゃ
				 */
				return -1;
			}
			BaseTime += dt * 60;
			/*
			 * 秒
			 */
			dt = decStr2Value(pTime + 4, 2);
			if (dt < 0) {
				/*
				 * エラーじゃ
				 */
				return -1;
			}
			BaseTime += dt;
			/*
			 * GMT -> JST
			 */
			BaseTime += 9 * 60 * 60;
			break;
		}
		pTime++;
	}

	return BaseTime;
}

void SetHTTP_RequestStatus(int8_t stat)
{
	HTTP_RequestStatus = stat;
}

int8_t GetHTTP_RequestStatus(void)
{
	return HTTP_RequestStatus;
}

int8_t HTTP_Request(PHTTP_REQUEST_VALUE pHTTPRequest)
{
	int32_t Status;

	switch(HTTP_RequestStatus) {
		case HTTP_REQ_IDLE:
			break;
		case HTTP_REQ_COMPLETE:
		case HTTP_REQ_ABORT:
			break;
		case HTTP_REQ_CONNECT:
		{
			/*
			 * 接続中
			 */
			pHTTPRequest->Result = nonBlockingConnect(pHTTPRequest->Socket, pHTTPRequest->pSrvAddress, pHTTPRequest->port);
			if (pHTTPRequest->Result == SOCK_CONNECT_COMPLETE) {
				/*
				 * 接続完了
				 */
				HTTP_RequestStatus = HTTP_REQ_SEND;
			}
			else if (pHTTPRequest->Result == SOCK_CONNECT_ABORT) {
				/*
				 * 接続エラー
				 */
				HTTP_RequestStatus = HTTP_REQ_ABORT;
				/*
				 * W5500を初期化
				 */
				HTTP_HardReset();

				return HTTP_RequestStatus;
			}
			else {
				/*
				 * 一旦帰る（接続中）
				 */
				return HTTP_RequestStatus;
			}
		}
		case HTTP_REQ_SEND:
		{
			/*
			 * 送信ステータス
			 */
			/*
			 * HTTPリクエスト送信
			 */
			Status = send(pHTTPRequest->Socket,  pHTTPRequest->ptxBuffer, pHTTPRequest->txLength);

			if (Status < 0) {
				/*
				 * エラー
				 */
				HTTP_RequestStatus = HTTP_REQ_ABORT;
				return HTTP_RequestStatus;
			}
			else if (Status > 0) {
				/*
				 * 送信完了〜次は受信ステータス
				 */
				HTTP_RequestStatus = HTTP_REQ_RECV;
			}
			else {
				/*
				 * 一旦帰る（送信中）
				 */
				return HTTP_RequestStatus;
			}
		}
		case HTTP_REQ_RECV:
		{
			/*
			 * 受信
			 */
			pHTTPRequest->ReceivedLength = recv_for_Client(pHTTPRequest->Socket, pHTTPRequest->prxBuffer, pHTTPRequest->rxLength);
			if (pHTTPRequest->ReceivedLength > 0) {
				/*
				 * 受信完了
				 */
				HTTP_RequestStatus = HTTP_REQ_DISCONNECT;
				pHTTPRequest->HTTPStatusCode = HTTP_GetStatus(pHTTPRequest->prxBuffer);

				/*
				 * JSTを更新しておく
				 */
				HTTP_JST_Time = HTTP_GetBaseClock(pHTTPRequest->prxBuffer, pHTTPRequest->rxLength);
			}
			else {
				return HTTP_RequestStatus;
			}
		}
		case HTTP_REQ_DISCONNECT:
		{
			int8_t rc = nonBlockingDisconnect(pHTTPRequest->Socket);

			if (rc == SOCK_DISCONNECT_COMPLETE) {
				HTTP_RequestStatus = HTTP_REQ_COMPLETE;
			}
			else if (rc == SOCK_DISCONNECT_ABORT) {
				HTTP_RequestStatus = HTTP_REQ_ABORT;
			}
		}
	}

	return HTTP_RequestStatus;
}

#define	NET_OPE_STATUS_IDLE			0
#define	NET_OPE_STATUS_LISTEN		1
#define	NET_OPE_STATUS_RECEIVE		2
#define	NET_OPE_STATUS_SEND			3
#define	NET_OPE_STATUS_CLOSE		4

uint8_t	NetOPE_Status = NET_OPE_STATUS_IDLE;

#define	NET_OPERATION_RESTART_TIME	1000
int32_t NetOperationRestartTimer = 0;

void HTTP_ListenReset(void)
{
	NetOPE_Status = NET_OPE_STATUS_IDLE;
}
/*
 * 外部からの接続応答
 * 接続要求があった場合受信データを引数にProcessing()関数がコールされる
 */
void HTTP_Listen(PNET_LISTEN_DATA pOPEData)
{
	int8_t st;

	if (!NetDeviceCheck()) {
		/*
		 * 知らぬ間にリセットされてたら初期化する
		 */
		HTTP_HardReset();
	}

	switch(NetOPE_Status) {
		case NET_OPE_STATUS_IDLE:
		{
			pOPEData->nSocket = socket(pOPEData->nSocket, SOCK_TCP, pOPEData->Port, 0);
			if (listen(pOPEData->nSocket) == SOCK_OK) {
				NetOPE_Status = NET_OPE_STATUS_LISTEN;
				st = GetSocketStatus(pOPEData->nSocket);
				if (st == STATUS_SOCK_ESTABLISHED) {
					NetOPE_Status = NET_OPE_STATUS_RECEIVE;
				}
			}
			break;
		}
		case NET_OPE_STATUS_LISTEN:
		{
			st = GetSocketStatus(pOPEData->nSocket);
			if (st == STATUS_SOCK_ESTABLISHED) {
				NetOPE_Status = NET_OPE_STATUS_RECEIVE;
			}
			else if (st == STATUS_SOCK_CLOSE_WAIT || st == STATUS_SOCK_CLOSED) {
				NetOPE_Status = NET_OPE_STATUS_IDLE;
			}
			break;
		}
		case NET_OPE_STATUS_RECEIVE:
		{
			pOPEData->rxLength = recv(pOPEData->nSocket, pOPEData->prxBuffer, pOPEData->szrxBuffer);
			if (pOPEData->rxLength > 0) {
				if (pOPEData->Processing) {
					pOPEData->Processing(pOPEData);
				}
				NetOPE_Status = NET_OPE_STATUS_SEND;
			}
			else if (pOPEData->rxLength < 0) {
				/*
				 * エラーなのでリスタート
				 */
				NetOPE_Status = NET_OPE_STATUS_IDLE;
			}
			break;
		}
		case NET_OPE_STATUS_SEND:
		{
			if (pOPEData->txLength > 0) {
				st = send(pOPEData->nSocket, pOPEData->ptxBuffer, pOPEData->txLength);
				if (st > 0) {
					NetOperationRestartTimer = NET_OPERATION_RESTART_TIME;
					NetOPE_Status = NET_OPE_STATUS_CLOSE;
				}
				else if (st < 0) {
					/*
					 * エラー
					 */
					NetOPE_Status = NET_OPE_STATUS_IDLE;
				}
			}
			else {
				NetOPE_Status = NET_OPE_STATUS_IDLE;
			}
			break;
		}
		case NET_OPE_STATUS_CLOSE:
		{
			st = GetSocketInterrupt(pOPEData->nSocket);
			if (st & Sn_IR_SENDOK) {
				SetSocketInterrupt(pOPEData->nSocket, st);
				NetOPE_Status = NET_OPE_STATUS_IDLE;
			}
			else if (NetOperationRestartTimer <= 0) {
				/*
				 * 万一の処理
				 */
				NetOPE_Status = NET_OPE_STATUS_IDLE;
			}
			else {
				NetOperationRestartTimer--;
			}
			break;
		}
	}
}
