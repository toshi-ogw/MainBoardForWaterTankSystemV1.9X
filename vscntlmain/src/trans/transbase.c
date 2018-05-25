/*
 * transbase.c
 *
 *  Created on: 2015/12/05
 *      Author: toshio
 */

#include "../trans/transbase.h"

#include "timer32.h"
#include "uart.h"
#include "../trans/crc.h"



/*
 * リクエスト用バッファ
 */
#define	REQUEST_BUFFER_SIZE	128
unsigned char RequestBuffer[REQUEST_BUFFER_SIZE];

/*
 * レスポンス用バッファ
 */
#define	RESPONSE_BUFFER_SIZE	128
unsigned char ResponseBuffer[RESPONSE_BUFFER_SIZE];
uint8_t	_ResponseBufferPointer = 0;

/*
 * 受信タイマー
 */
PTIMER_BLOCK _pReceiveTimer;

#define	_REVEIVE_TIMEOUT	20
bool bReveiveTimeOut = false;

#define	IS_REVCEIVE_TIMEOUT		bReveiveTimeOut
#define	ENABLE_RECEIVE_TIMER	{	bReveiveTimeOut = false; EnableTimerTask(_pReceiveTimer); }
#define	DISABLE_REVEIVE_TIMER	{	DisableTimerTask(_pReceiveTimer); }



// リトライカウンタ初期値
#define	TRANS_RETRY_COUNT	5

typedef struct {
	int state;
	int cntRetry;
	PTR_SENDRECEIVE_PACK	pTxPack;
	PTR_SENDRECEIVE_PACK	pRxPack;
} TR_SENDRECEIVE_STATE, *PTR_SENDRECEIVE_STATE;

TR_SENDRECEIVE_STATE	_TR_Status;

/*
 * バイト値を文字に変換する
 * １バイトー＞変換後は２バイト
 *
 * 例　0x23 -> 0x32,0x33
 *
 * 戻り値は正常ならば次のバッファーポンター
 * エラーの場合はNULLを返す
 */
unsigned char *Val2Byte(unsigned char *pBuff, uint8_t val)
{
	int i;

	for (i = 0; i < 2; i++) {
		if ((val & 0xf0) < 0xa0) {
			*pBuff = (val >> 4) + '0';
		}
		else {
			*pBuff = (val >> 4) - 0x0a + 'A';
		}
		val <<= 4;
		pBuff++;
	}

	return pBuff;
}

/*
 * 16進数値文字２文字を１バイトの数値に変換する
 *
 * 例　'A','3'-> 0xa4
 *
 * 戻り値は正常ならば次のバッファーポンター
 * エラーの場合はNULLを返す
 */
unsigned char *Byte2Val(unsigned char *pBuff, uint8_t *pval)
{
	int i;
	uint8_t ch;


	for (i = 0; i < 2; i++) {
		if (*pBuff >= '0' && *pBuff <='9') {
			ch = *pBuff - '0';
		}
		else if (*pBuff >= 'A' && *pBuff <='F') {
			ch = *pBuff - 'A';
			ch += 10;
		}
		else {
			// Error!!
			return NULL;
		}
		pBuff++;
		*pval <<= 4;
		*pval |= ch;
	}

	return pBuff;
}

void ReceiveTimer(void)
{
	DISABLE_REVEIVE_TIMER;
	IS_REVCEIVE_TIMEOUT = true;
}

/*
 * 初期化
 */
void Init_TransBase(void)
{
	_TR_Status.state = _TR_STATE_IDLE;
	_TR_Status.pRxPack = NULL;
	_TR_Status.pTxPack = NULL;

	IS_REVCEIVE_TIMEOUT = false;
	_pReceiveTimer = AllocTimer1Task(_REVEIVE_TIMEOUT, ReceiveTimer);

}

/*
#define	_RESPONSE_POS_STX		0
#define	_RESPONSE_POS_ADDRESS	1
#define	_RESPONSE_POS_COM		2
#define	_RESPONSE_POS_CODE		3
#define	_RESPONSE_POS_LNG_H		4
#define	_RESPONSE_POS_LNG_L		5
#define	_RESPONSE_POS_TEXT		6
*/

void TR_InitSendReceivePack(PTR_SENDRECEIVE_PACK pPack)
{
	pPack->address	= 0;
	pPack->command	= 0;
	pPack->code		= 0;
	pPack->length	= 0;
	pPack->status	= 0;
	pPack->pBuffer	= NULL;
	pPack->pFunc	= NULL;
}

/*
 * 送信はブロックしない
 * 初期化し、タイムアウトを設定したら抜ける
 * 定期的にサンプリングし受信データが存在したらTR_Receiveをコールする
 * 受信が完了したら決められたファンクションをコールする
 */
void TR_InitSend(PTR_SENDRECEIVE_PACK pTxPack, PTR_SENDRECEIVE_PACK pRxPack)
{
	uint8_t i ;

	_TR_Status.state	= _TR_STATE_SENDING;
	_TR_Status.cntRetry	= TRANS_RETRY_COUNT;

	_TR_Status.pTxPack	= pTxPack;
	_TR_Status.pRxPack	= pRxPack;

	RequestBuffer[_RESPONSE_POS_STX]		= RS_CODE_STX;
	RequestBuffer[_RESPONSE_POS_ADDRESS]	= _TR_Status.pTxPack->address;
	RequestBuffer[_RESPONSE_POS_COM]		= _TR_Status.pTxPack->command;
	RequestBuffer[_RESPONSE_POS_CODE]		= _TR_Status.pTxPack->code;

	// Length
	// binary->ascii変換するので長さが倍になる
	uint8_t lngData = _TR_Status.pTxPack->length * 2;
	Val2Byte(&RequestBuffer[_RESPONSE_POS_LNG_H], lngData);

	// Data
	unsigned char *pDatBuffer 	= _TR_Status.pTxPack->pBuffer;
	unsigned char *pReqBuffer	= &RequestBuffer[_RESPONSE_POS_TEXT];

	for (i = 0; i < _TR_Status.pTxPack->length; i++) {
		pReqBuffer = Val2Byte(pReqBuffer, *pDatBuffer);
		pDatBuffer++;
	}

	pReqBuffer = Val2Byte(pReqBuffer, GetCRC8(RequestBuffer, lngData + _RESPONSE_POS_TEXT));
	*pReqBuffer = RS_CODE_ETX;

	_TR_Status.pTxPack->length = lngData + _RESPONSE_POS_TEXT + 3;
	_TR_Status.pTxPack->status = _TR_STATE_SENDING;

	SendRB_UART(RequestBuffer, _TR_Status.pTxPack->length);
	//
	ENABLE_RECEIVE_TIMER;
}
uint8_t dd;
/*
 * 送信完了チェック
 */
int TR_NonBlockSend(void)
{
	unsigned char ch;

	if (_TR_Status.pTxPack->address == _GLOBAL_ADDRESS) {
		DISABLE_REVEIVE_TIMER;
		_TR_Status.pTxPack->status = _TR_STATE_COMPLETE;
	}
	else {
		if (ReadRB_UART(&ch, 1)) {
			if (ch == RS_CODE_ACK) {
				// ACK受信
				DISABLE_REVEIVE_TIMER;
				_TR_Status.pTxPack->status = _TR_STATE_COMPLETE;
			}
			else if (ch == RS_CODE_NAK) {
				// retry
				DISABLE_REVEIVE_TIMER;
				SendRB_UART(RequestBuffer, _TR_Status.pTxPack->length);
				ENABLE_RECEIVE_TIMER;
			}
		}
		if (IS_REVCEIVE_TIMEOUT) {
			DISABLE_REVEIVE_TIMER;
			_TR_Status.pTxPack->status = _TR_STATE_TIMEOUT_ERROR;
		}
	}
	if (_TR_Status.pTxPack->status == _TR_STATE_COMPLETE && _TR_Status.pTxPack->pFunc) {
		/*
		 * 送信時はレングスゼロ
		 */
		_TR_Status.pTxPack->status = _TR_Status.pTxPack->pFunc(_TR_Status.pTxPack->address, _TR_Status.pTxPack->command, _TR_Status.pTxPack->code, NULL, 0);
	}

	return _TR_Status.pTxPack->status;
}


/*
 * 受信
 *
 * 受信データをサンプルしあったらバッファに追加する。
 * EXTを受信したらフレームをチェックし正しければACKを送信した後
 * 任意の処理関数をコールし処理を完了する
 * データ受信中は処理をブロックすること無くリターンする。
 * 本関数は任意の場所で定期的にコールすること
 *
 */
int TR_NonBlockReceive(void)
{
	unsigned char ch, *pBuffer, Crc0, Crc1;
	uint8_t lngResData;

	if (ReadRB_UART(&ch, 1)) {
		// STXであればバッファポインタを初期化
		if (ch == RS_CODE_STX) {
			_ResponseBufferPointer = 0;
			_TR_Status.pRxPack->status = _TR_STATE_RECEIVEING;
		}
		// 受信データを保存
		if (_ResponseBufferPointer < RESPONSE_BUFFER_SIZE) {
			ResponseBuffer[_ResponseBufferPointer] = ch;
			_ResponseBufferPointer++;
			// もしETXならフレームチェックを行う
			if (ch == RS_CODE_ETX) {
				_TR_Status.pRxPack->address	= ResponseBuffer[_RESPONSE_POS_ADDRESS];
				_TR_Status.pRxPack->command	= ResponseBuffer[_RESPONSE_POS_COM];
				_TR_Status.pRxPack->code	= ResponseBuffer[_RESPONSE_POS_CODE];

				pBuffer = Byte2Val(&ResponseBuffer[_RESPONSE_POS_LNG_H], &lngResData);
				if (!pBuffer || (lngResData / 2) > _TR_Status.pRxPack->length) {
					// エラー
					_TR_Status.pRxPack->status = _TR_STATE_LENGTH_ERROR;
					return _TR_Status.pRxPack->status;
				}
				// CRC チェック
				Crc0 = GetCRC8(ResponseBuffer, _ResponseBufferPointer - 3);
				Byte2Val(&ResponseBuffer[_ResponseBufferPointer - 3], &Crc1);
				if (Crc0 != Crc1) {
					_TR_Status.pRxPack->status = _TR_STATE_CTR_ERROR;
					return _TR_Status.pRxPack->status;
				}
				// 受信TEXTをバッファに変換セットする
				if (lngResData) {
					uint8_t i;
					unsigned char *pText = (unsigned char *)&ResponseBuffer[_RESPONSE_POS_TEXT];
					for (i = 0; i < (lngResData / 2); i++) {
						pText = Byte2Val(pText, _TR_Status.pRxPack->pBuffer + i);
					}
				}
				// ACKを送信
				// 化けることがあるので２度送っておく
				ch = RS_CODE_ACK;
				SendRB_UART(&ch, 1);
				SendRB_UART(&ch, 1);
				// 念の為バッファポインタをリセット
				_ResponseBufferPointer = 0;
				// 処理関数をコール
				if (_TR_Status.pRxPack->pFunc) {
					_TR_Status.pRxPack->status = _TR_Status.pRxPack->pFunc(_TR_Status.pRxPack->address, _TR_Status.pRxPack->command, _TR_Status.pRxPack->code, _TR_Status.pRxPack->pBuffer, (lngResData / 2));
				}
				else {
					_TR_Status.pRxPack->status = _TR_STATE_COMPLETE;
				}

			}
		}
	}
	else if (IS_REVCEIVE_TIMEOUT) {
		_TR_Status.pRxPack->status = _TR_STATE_TIMEOUT_ERROR;
	}

	return _TR_Status.pRxPack->status;
}

int TR_NonBlockTrans(void)
{
	if (_TR_Status.state == _TR_STATE_SENDING) {
		_TR_Status.state = TR_NonBlockSend();
		if (_TR_Status.state == _TR_STATE_COMPLETE && _TR_Status.pTxPack->address != _GLOBAL_ADDRESS) {
			_TR_Status.pRxPack->status 	= _TR_STATE_RECEIVEING;
			_TR_Status.state			= _TR_Status.pRxPack->status;
			// タイマー起動
			ENABLE_RECEIVE_TIMER;
		}
	}
	else if (_TR_Status.state == _TR_STATE_RECEIVEING) {
		_TR_Status.state = TR_NonBlockReceive();
		if (_TR_Status.state < 0 && _TR_Status.state != _TR_STATE_TIMEOUT_ERROR) {
			// エラー
			if (_TR_Status.cntRetry > 0) {
				_TR_Status.cntRetry--;
				_TR_Status.pRxPack->status	= _TR_STATE_RECEIVEING;
				_TR_Status.state			= _TR_STATE_RECEIVEING;
				// NAKを送信
				// 化けることがあるので２度送る
				unsigned char ch = RS_CODE_NAK;
				SendRB_UART(&ch, 1);
				SendRB_UART(&ch, 1);

				// Version 1.06B
				// タイマー停止
				DISABLE_REVEIVE_TIMER;
				// タイマー起動
				ENABLE_RECEIVE_TIMER;

			}
		}
	}

	return _TR_Status.state;
}

