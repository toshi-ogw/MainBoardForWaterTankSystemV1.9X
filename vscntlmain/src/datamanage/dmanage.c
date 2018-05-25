/*
 * dmanage.c
 *
 *  Created on: 2015/12/08
 *      Author: toshio
 */

#include "dmanage.h"
#include "controldata.h"
#include "sensordrive.h"
#include "e2rom.h"
#include "datarom.h"
#include "baseclock.h"
#include "ontimer.h"
#include "netoperation.h"
#include "version.h"

#define	_REMOTE_TERMINAL_MAX	5
typedef struct {
	bool	bAvailable;
	uint8_t	adress;
} REMOTE_TERM, *PREMOTE_TERM;


typedef struct {
	bool	bIsNotice;		// 通知がある場合はtrue;
	uint8_t	address;		// 通知対象リモコンアドレス
	uint8_t	code;			// 通知対象エリア
	uint8_t	length;			// データ長
	unsigned char *pBuffer;	// データ
} NOTIFICATION_CHANGE_ITEM, *PNOTIFICATION_CHANGE_ITEM;


NOTIFICATION_CHANGE_ITEM	_NoticeItem;

REMOTE_TERM	_Remote_Terminal[_REMOTE_TERMINAL_MAX];

TR_SENDRECEIVE_PACK	_RC_TXPack;
TR_SENDRECEIVE_PACK	_RC_RXPack;

#define	_DM_BUFFER_SIZE	32
unsigned char _DM_Buffer[_DM_BUFFER_SIZE];

/*
 * Version 1.06 2016/12/23 T.Ogawa
 */
#define	_IDAT_BUFFER_SIZE	16
unsigned char _IDAT_Buffer[_IDAT_BUFFER_SIZE];
#define	_IDAT_FREE_VERPOS	0
#define	_IDAT_FREE_REVPOS	4
#define	_IDAT_FREE_TOP		8


void InitDataManage(void)
{
	int i;

	_NoticeItem.bIsNotice	= false;

	for (i = 0; i < _REMOTE_TERMINAL_MAX; i++) {
		_Remote_Terminal[i].bAvailable = false;
	}

	Init_TransBase();

}

void RC_Find(void)
{

}

/*
 * GETコマンドの回答処理関数
 */
int RC_GetResponseFunc(uint8_t address, uint8_t command, uint8_t code, unsigned char *pBuffer, uint8_t length)
{
    unsigned char *pDest = NULL;
    uint8_t i, Length = 0;

	switch(code) {
		case	_CODE_RUNSTOP:
			Length	= 1;
			pDest	= (unsigned char *)&CNTL_DataSet[_CNTL_RUN];
			break;
		case	_CODE_RUNMODE:
			Length	= 1;
			pDest	= (unsigned char *)&CNTL_DataSet[_CODE_RUNMODE];
			break;
		case	_CODE_BASE:
			Length	= (_CNTL_SUII - _CNTL_ONDO_L) + 1;
			pDest	= (unsigned char *)&CNTL_DataSet[_CNTL_ONDO_L];
			break;
		case	_CODE_ONCHO:
			Length	= (_CNTL_ONCHO_DELAY - _CNTL_ONCHO_HOSEI) + 1;
			pDest	= (unsigned char *)&CNTL_DataSet[_CNTL_ONCHO_HOSEI];
			break;
		case	_CODE_ONDOSETTEI:
			Length	= (_CNTL_ONDO_SETTEI_H - _CNTL_ONDO_SETTEI_L) + 1;
			pDest	= (unsigned char *)&CNTL_DataSet[_CNTL_ONDO_SETTEI_L];
			break;
		case	_CODE_SUII:
			Length	= (_CNTL_MIZUNUKI_TIME_STP - _CNTL_SUII_DEVICE) + 1;
			pDest	= (unsigned char *)&CNTL_DataSet[_CNTL_SUII_DEVICE];
			break;
		case	_CODE_NET:
			Length	= (_CNTL_SERVER3 - _CNTL_DHCP_ONOFF) + 1;
			pDest	= (unsigned char *)&CNTL_DataSet[_CNTL_DHCP_ONOFF];
			break;
/*
 * Version 1.05 2016/12/10 T.Ogawa
		case	_CODE_TERMNO:
			Length	= (_CNTL_TERM_NO_H - _CNTL_TERM_NO_L) + 1;
			pDest	= (unsigned char *)&CNTL_DataSet[_CNTL_TERM_NO_L];
			break;
*/
			case	_CODE_CLOCK:
			Length	= (_CNTL_STOP_TIMER_H - _CNTL_CLOCK_CNTL) + 1;
			pDest	= (unsigned char *)&CNTL_DataSet[_CNTL_CLOCK_CNTL];
   			/*
    		 * 時計設定
    		*/
    		SetBaseClock();
			break;
		case	_CODE_SET_PRESS:
			Length	= 1;
			pDest	= (unsigned char *)&CNTL_DataSet[_CNTL_SUII_H_SETTEI];
			break;
		case	_CODE_SET_ALARM:
			Length	= (_CNTL_STOP_TIMER_H - _CNTL_CLOCK_CNTL) + 1;
			pDest	= (unsigned char *)&CNTL_DataSet[_CNTL_CLOCK_CNTL];
			// 時計データは更新しない
			*(pBuffer + 1) = CNTL_DataSet[_CNTL_CLOCK0];
			*(pBuffer + 2) = CNTL_DataSet[_CNTL_CLOCK1];
			*(pBuffer + 3) = CNTL_DataSet[_CNTL_CLOCK2];
			*(pBuffer + 4) = CNTL_DataSet[_CNTL_CLOCK3];
			break;

			/*
			 * Version 1.05 2016/12/10 T,Ogawa
			 *
			 * 追加
			 */
   		case	_CODE_SUPPLYMON:
   			Length	= 1;
   			pDest	= (unsigned char *)&CNTL_DataSet[_CNTL_SUPPLYTIME];
   			break;

   		case	_CODE_DRIVETIME:
   			Length	= 1;
   			pDest	= (unsigned char *)&CNTL_DataSet[_CNTL_DRIVETIME];
   			break;

			/*
			 * Version 1.06 2016/12/23 T,Ogawa
			 *
			 * 追加
			 */
   		case	_CODE_RC:
   			Length	= 1;
   			pDest	= (unsigned char *)&CNTL_DataSet[_CNTL_RC];
   			break;


		default:
			break;
	}

	for (i = 0; i < Length; i++) {
		*pDest = *pBuffer;
		pBuffer++;
		pDest++;
	}

	/*
	 * タイマー設定時には完了フラグ見直し
	 */
	if (code == _CODE_SET_ALARM) {
		OnTimerRestartCheck();
	}

	return _TR_STATE_COMPLETE;
}

/*
 * SETコマンドの回答処理関数
 */
int RC_SetResponseFunc(uint8_t address, uint8_t command, uint8_t code, unsigned char *pBuffer, uint8_t length)
{

	return _TR_STATE_COMPLETE;
}

/*
 * Polコマンドの回答処理関数
 */
int RC_PolResponseFunc(uint8_t address, uint8_t command, uint8_t code, unsigned char *pBuffer, uint8_t length)
{
    unsigned char *pDest = NULL;
    uint8_t i, Length = 0;

    if (command == _RESPONSE_REQ || command == _RESPONSE_WNT) {
    	switch(code) {
			case	_CODE_RUNSTOP:
				Length	= 1;
				pDest	= (unsigned char *)&CNTL_DataSet[_CNTL_RUN];
				break;
			case	_CODE_RUNMODE:
				Length	= 1;
				pDest	= (unsigned char *)&CNTL_DataSet[_CNTL_RUNMODE];
				break;
			case	_CODE_BASE:
    			Length	= (_CNTL_SUII - _CNTL_ONDO_L) + 1;
    			pDest	= (unsigned char *)&CNTL_DataSet[_CNTL_ONDO_L];
    			break;
    		case	_CODE_ONCHO:
    			Length	= (_CNTL_ONCHO_DELAY - _CNTL_ONCHO_HOSEI) + 1;
    			pDest	= (unsigned char *)&CNTL_DataSet[_CNTL_ONCHO_HOSEI];
    			break;
    		case	_CODE_ONDOSETTEI:
    			Length	= (_CNTL_ONDO_SETTEI_H - _CNTL_ONDO_SETTEI_L) + 1;
    			pDest	= (unsigned char *)&CNTL_DataSet[_CNTL_ONDO_SETTEI_L];
    			break;
    		case	_CODE_SUII:
    			Length	= (_CNTL_MIZUNUKI_TIME_STP - _CNTL_SUII_DEVICE) + 1;
    			pDest	= (unsigned char *)&CNTL_DataSet[_CNTL_SUII_DEVICE];
    			break;
    		case	_CODE_NET:
    			Length	= (_CNTL_SERVER3 - _CNTL_DHCP_ONOFF) + 1;
    			pDest	= (unsigned char *)&CNTL_DataSet[_CNTL_DHCP_ONOFF];
    			break;
/*
 * Version 1.05 2016/12/10 T.Ogawa
    		case	_CODE_TERMNO:
    			Length	= (_CNTL_TERM_NO_H - _CNTL_TERM_NO_L) + 1;
    			pDest	= (unsigned char *)&CNTL_DataSet[_CNTL_TERM_NO_L];
    			break;
*/
    		case	_CODE_CLOCK:
    			Length	= (_CNTL_STOP_TIMER_H - _CNTL_CLOCK_CNTL) + 1;
    			CNTL_DataSet[_CNTL_CLOCK_CNTL]	= *(pBuffer + 0);
    			CNTL_DataSet[_CNTL_CLOCK0]		= *(pBuffer + 1);
    			CNTL_DataSet[_CNTL_CLOCK1]		= *(pBuffer + 2);
    			CNTL_DataSet[_CNTL_CLOCK2]		= *(pBuffer + 3);
    			CNTL_DataSet[_CNTL_CLOCK3]		= *(pBuffer + 4);
    			/*
    			 * 時計設定
    			 */
    			SetBaseClock();
    			break;
    		case	_CODE_SET_PRESS:
    			Length	= 1;
    			pDest	= (unsigned char *)&CNTL_DataSet[_CNTL_SUII_H_SETTEI];
    			break;
    		case	_CODE_SET_ALARM:
    			Length	= (_CNTL_STOP_TIMER_H - _CNTL_CLOCK_CNTL) + 1;
    			pDest	= (unsigned char *)&CNTL_DataSet[_CNTL_CLOCK_CNTL];
    			// 時計データは更新しない
    			*(pBuffer + 1) = CNTL_DataSet[_CNTL_CLOCK0];
    			*(pBuffer + 2) = CNTL_DataSet[_CNTL_CLOCK1];
    			*(pBuffer + 3) = CNTL_DataSet[_CNTL_CLOCK2];
    			*(pBuffer + 4) = CNTL_DataSet[_CNTL_CLOCK3];
    			break;

    			/*
    			 * Version 1.05 2016/12/10 T,Ogawa
    			 *
    			 * 追加
    			 */
    		case	_CODE_SUPPLYMON:
    			Length	= 1;
    			pDest	= (unsigned char *)&CNTL_DataSet[_CNTL_SUPPLYTIME];
    			break;

    		case	_CODE_DRIVETIME:
    			Length	= 1;
    			pDest	= (unsigned char *)&CNTL_DataSet[_CNTL_DRIVETIME];
    			break;

    			/*
    			 * Version 1.06 2016/12/23 T,Ogawa
    			 *
    			 * 追加
    			 */
    		case	_CODE_RC:
    			Length	= 1;
    			pDest	= (unsigned char *)&CNTL_DataSet[_CNTL_RC];
    			break;

    			/*
    			 * Version 1.06L 2018/02/09 T,Ogawa
    			 *
    			 * 追加
    			 */
    		case	_CODE_FWS_OFF:
    			Length	= 0;
    			pDest = NULL;
    			m_ForceWaterSupply = 0;
    			_HTTP_REPORT;
    			break;

    		case	_CODE_FWS_ON:
   				Length	= 0;
   				pDest = NULL;
    			m_ForceWaterSupply = 1;
    			_HTTP_REPORT;
    			break;

    		default:
    			break;
    	}

    	if (pDest) {
    		/*
    		 * 他のリモコンへデータ通知
    		 */
    		_NoticeItem.pBuffer 	= pDest;
    		_NoticeItem.length		= Length;
    		_NoticeItem.address		= _GLOBAL_ADDRESS;
    		_NoticeItem.code		= code;
    		_NoticeItem.bIsNotice	= true;

    		/*
    		 * _RESPONSE_WNTの時はリモコンがデータを要求しているので
    		 * データコピーを行わない
    		 */
    		if (command == _RESPONSE_REQ) {
    			/*
    			 * データ設定
    			 */
    			for (i = 0; i < Length; i++) {
    				*pDest = *pBuffer;
    				pBuffer++;
    				pDest++;
    			}
    			/*
    			 * タイマー設定時には完了フラグ見直し
    			 */
    			if (code == _CODE_SET_ALARM) {
    				OnTimerRestartCheck();
    			}

    			/*
    			 *  HTTP 報告開始追加
    			 *
    			 *  Version ZVersion製品版1.04 2016/09/13 T.Ogawa
    			 */
    			_HTTP_REPORT;
    		}
    	}
    }
    else if (command == _RESPONSE_INT) {
    	/*
    	 * 初期化データ要求
    	 */
    	switch(code) {
    		case _CODE_INITIAL_A:
    		{
    			_NoticeItem.pBuffer 	= (unsigned char *)&CNTL_DataSet[_CNTL_RUNMODE];
    			_NoticeItem.length		= (_CNTL_MIZUNUKI_TIME_STP - _CNTL_RUNMODE) + 1;
     			_NoticeItem.code		= _CODE_INITIAL_A;
       			_NoticeItem.address		= address;
    		}
    		break;
    		case _CODE_INITIAL_B:
    		{
    			_NoticeItem.pBuffer 	= (unsigned char *)&CNTL_DataSet[_CNTL_DHCP_ONOFF];
/*
 * Version 1.05 2016/12/10 T.Ogawa

    			_NoticeItem.length		= (_CNTL_TERM_NO_H - _CNTL_DHCP_ONOFF) + 1;
*/
    			_NoticeItem.length		= (_CNTL_DRIVETIME - _CNTL_DHCP_ONOFF) + 1;

    			_NoticeItem.code		= _CODE_INITIAL_B;
       			_NoticeItem.address		= address;
    		}
    		break;

    		/*
    		 * Version 1.06 2016/12/23 T.Ogawa
    		 */
    		case _CODE_INITIAL_C:
    		{
    			SetRCVersion(&_IDAT_Buffer[_IDAT_FREE_VERPOS], &_IDAT_Buffer[_IDAT_FREE_REVPOS]);

    			/*
    			 * Version 1.06J	2017/08/22 T.Ogawa
    			 * Version 1.06K    2017/09/11 T.Ogawa
    			 */
    			//_IDAT_Buffer[_IDAT_FREE_REVPOS + 0] = '0';
    			//_IDAT_Buffer[_IDAT_FREE_REVPOS + 1] = '0';
    			//_IDAT_Buffer[_IDAT_FREE_REVPOS + 2] = '6';
    			//_IDAT_Buffer[_IDAT_FREE_REVPOS + 3] = 'K';

    			_IDAT_Buffer[_IDAT_FREE_TOP] = CNTL_DataSet[_CNTL_RC];

    			_NoticeItem.pBuffer 	= _IDAT_Buffer;
    			_NoticeItem.length		= _IDAT_FREE_TOP + 1;

    			_NoticeItem.code		= _CODE_INITIAL_C;
       			_NoticeItem.address		= address;
    		}
    		break;
   	}
		_NoticeItem.bIsNotice	= true;
    }

	return _TR_STATE_COMPLETE;
}

void RC_Get(uint8_t adrs, uint8_t item, unsigned char *pBuffer, uint8_t length)
{
	TR_InitSendReceivePack(&_RC_TXPack);
	TR_InitSendReceivePack(&_RC_RXPack);

	_RC_TXPack.address	= adrs;
	_RC_TXPack.command	= _COMMAND_GET;
	_RC_TXPack.code		= item;
	_RC_TXPack.length	= 0;
	_RC_TXPack.pBuffer	= NULL;

	_RC_RXPack.length	= length;
	_RC_RXPack.pBuffer	= pBuffer;
	_RC_RXPack.pFunc	= RC_GetResponseFunc;

	{
		uint8_t i;
		for (i = 0; i < length; i++) {
			*(pBuffer + i) = 0xff;
		}
	}

	TR_InitSend(&_RC_TXPack, &_RC_RXPack);
}

void RC_Set(uint8_t adrs, uint8_t item, unsigned char *pBuffer, uint8_t length)
{
	TR_InitSendReceivePack(&_RC_TXPack);
	TR_InitSendReceivePack(&_RC_RXPack);

	_RC_TXPack.address	= adrs;
	_RC_TXPack.command	= _COMMAND_SET;
	_RC_TXPack.code		= item;
	_RC_TXPack.length	= length;
	_RC_TXPack.pBuffer	= pBuffer;

	_RC_RXPack.length	= 0;
	_RC_RXPack.pBuffer	= NULL;
	_RC_RXPack.pFunc	= RC_SetResponseFunc;

	TR_InitSend(&_RC_TXPack, &_RC_RXPack);
}

void RC_Pol(uint8_t adrs, unsigned char *pBuffer, uint8_t length)
{
	TR_InitSendReceivePack(&_RC_TXPack);
	TR_InitSendReceivePack(&_RC_RXPack);

	_RC_TXPack.address	= adrs;
	_RC_TXPack.command	= _COMMAND_POL;
	_RC_TXPack.code		= '0';
	_RC_TXPack.length	= 0;
	_RC_TXPack.pBuffer	= NULL;

	_RC_RXPack.length	= length;
	_RC_RXPack.pBuffer	= pBuffer;
	_RC_RXPack.pFunc	= RC_PolResponseFunc;

	{
		uint8_t i;
		for (i = 0; i < length; i++) {
			*(pBuffer + i) = 0xff;
		}
	}

	TR_InitSend(&_RC_TXPack, &_RC_RXPack);
}

uint8_t StatusBuffer[16];

void RC_SendStatus(void)
{
	/*
	 *	_CNTL_RUN + _CNTL_RUNMODE + _CNTL_ONDO_L + _CNTL_ONDO_H + _CNTL_SUII_LEVEL + _CNTL_SUII + _CNTL_CLOCK_CNTL + _CNTL_CLOCK0 + _CNTL_CLOCK1 + _CNTL_CLOCK2 + _CNTL_CLOCK3 + _CNTL_RELAY_STATE + _CNTL_ERROR_STATE + _CNTL_SERVER_VERSION
	 */
	MeasurementTemperature();
	MeasurementWaterLevel();

	// 時計取得
	GetBaseClock();

	StatusBuffer[0] = CNTL_DataSet[_CNTL_RUN];
	StatusBuffer[1] = CNTL_DataSet[_CNTL_RUNMODE];
	StatusBuffer[2] = CNTL_DataSet[_CNTL_ONDO_L];
	StatusBuffer[3] = CNTL_DataSet[_CNTL_ONDO_H];
	StatusBuffer[4] = CNTL_DataSet[_CNTL_SUII_LEVEL];
	StatusBuffer[5] = CNTL_DataSet[_CNTL_SUII];
	StatusBuffer[6] = CNTL_DataSet[_CNTL_CLOCK_CNTL];
	StatusBuffer[7] = CNTL_DataSet[_CNTL_CLOCK0];
	StatusBuffer[8] = CNTL_DataSet[_CNTL_CLOCK1];
	StatusBuffer[9] = CNTL_DataSet[_CNTL_CLOCK2];
	StatusBuffer[10] = CNTL_DataSet[_CNTL_CLOCK3];
	StatusBuffer[11] = CNTL_DataSet[_CNTL_RELAY_STATE];
	StatusBuffer[12] = CNTL_DataSet[_CNTL_ERROR_STATE];


	/*
	 * Version1.06L
	 * 強制給水フラグ
	 * このフラグは循環運転中のみ有効となる。
	 */
	if (m_ForceWaterSupply) {
		StatusBuffer[11] |= _CNTL_RY_BIT_FWS;
	}


	/*
	 *  温度設定追加バージョンの区別を行う
	 *  まず、最初のバイトが @ であること
	 *
	 *  Version ZVersion製品版1.04 2016/09/13 T.Ogawa
	 */
	StatusBuffer[13] = '@';
	/*
	 *  次のバイトが バージョン であること
	 *
	 *  Version ZVersion製品版1.04 2016/09/13 T.Ogawa
	 */
	StatusBuffer[14] = _DATA_VERSION;

	/*
	 *  温度設定追加
	 *
	 *  Version ZVersion製品版1.04 2016/09/13 T.Ogawa
	 */
	StatusBuffer[15] = CNTL_DataSet[_CNTL_ONDO_SETTEI_L];
	StatusBuffer[16] = CNTL_DataSet[_CNTL_ONDO_SETTEI_H];

	StatusBuffer[17] = CNTL_DataSet[_CNTL_SERVER_VERSION];

	RC_Set(_GLOBAL_ADDRESS, _CODE_STATE, StatusBuffer, 18);
}

#define	DMANAGE_BUFFER_SIZE	32
unsigned char dManageBuffer[DMANAGE_BUFFER_SIZE];

uint8_t	dummyIP[32];

void InitialParameterSend(void)
{
	//ParameterSuspend();
	// ROMから設定値を復帰

	ParameterResume();


#if 0
	// デバッグ！
	CNTL_DataSet[_CNTL_SERVER0] = 160;
	CNTL_DataSet[_CNTL_SERVER1] = 16;
	CNTL_DataSet[_CNTL_SERVER2] = 237;
	CNTL_DataSet[_CNTL_SERVER3] = 92;
#endif

	// 時計初期化
	GetBaseClock();

	/*
	 * デフォルトのタイマー起動条件を設定する
	 */
	OnTimerRestartCheck();

#if 0
	RC_Set(_GLOBAL_ADDRESS, _CODE_RUNSTOP, &CNTL_DataSet[_CNTL_RUN], 1);
	TR_NonBlockTrans();
	RC_Set(_GLOBAL_ADDRESS, _CODE_RUNMODE, &CNTL_DataSet[_CNTL_RUNMODE], 1);
	TR_NonBlockTrans();
	RC_Set(_GLOBAL_ADDRESS, _CODE_BASE, &CNTL_DataSet[_CNTL_ONDO_L], (_CNTL_SUII - _CNTL_ONDO_L) + 1);
	TR_NonBlockTrans();
	RC_Set(_GLOBAL_ADDRESS, _CODE_ONCHO, &CNTL_DataSet[_CNTL_ONCHO_HOSEI], (_CNTL_ONCHO_DELAY - _CNTL_ONCHO_HOSEI) + 1);
	TR_NonBlockTrans();
	RC_Set(_GLOBAL_ADDRESS, _CODE_ONDOSETTEI, &CNTL_DataSet[_CNTL_ONDO_SETTEI_L], (_CNTL_ONDO_SETTEI_H - _CNTL_ONDO_SETTEI_L) + 1);
	TR_NonBlockTrans();
	RC_Set(_GLOBAL_ADDRESS, _CODE_SUII, &CNTL_DataSet[_CNTL_SUII_DEVICE], (_CNTL_MIZUNUKI_TIME_H - _CNTL_SUII_DEVICE) + 1);
	TR_NonBlockTrans();
	RC_Set(_GLOBAL_ADDRESS, _CODE_NET, &CNTL_DataSet[_CNTL_DHCP_ONOFF], (_CNTL_SERVER3 - _CNTL_DHCP_ONOFF) + 1);
	TR_NonBlockTrans();
	RC_Set(_GLOBAL_ADDRESS, _CODE_TERMNO, &CNTL_DataSet[_CNTL_TERM_NO_L], (_CNTL_TERM_NO_H - _CNTL_TERM_NO_L) + 1);
	TR_NonBlockTrans();
	RC_Set(_GLOBAL_ADDRESS, _CODE_CLOCK, &CNTL_DataSet[_CNTL_CLOCK_CNTL], (_CNTL_STOP_TIMER_H - _CNTL_CLOCK_CNTL) + 1);
	TR_NonBlockTrans();
#endif
}


uint8_t CurrentPollAddress = _ADDRESS_PRIMARY;

void dManage(void)
{
	int status;
	static bool bSchedule = true;
	uint8_t rcMode ;

	status = TR_NonBlockTrans();
	if (status < 0) {
		/*
		 * エラー発生
		 */
		status = _TR_STATE_COMPLETE;
	}
	if (status == _TR_STATE_COMPLETE || status == _TR_STATE_IDLE) {
		// 他のリモコンへの通知があれば通知する
		if (_NoticeItem.bIsNotice) {
			_NoticeItem.bIsNotice = false;
			RC_Set(_GLOBAL_ADDRESS, _NoticeItem.code, _NoticeItem.pBuffer, _NoticeItem.length);
			if (_NoticeItem.code != _CODE_INITIAL_A && _NoticeItem.code != _CODE_INITIAL_B && _NoticeItem.code != _CODE_INITIAL_C) {
				/*
				 * Version1.06J
				 * 2017/08/22
				 * RUN/STOPは書き込まない
				 */
				if (_NoticeItem.code != _CODE_RUNSTOP) {
					// 変更をROMへ保存
					ParameterSuspend();
				}
			}
		}
		else if (bSchedule) {
			RC_SendStatus();
			bSchedule = false;
		}
		else {
			// リモコンをポーリング
			RC_Pol(CurrentPollAddress, dManageBuffer, sizeof(dManageBuffer));
			bSchedule = true;
/*
 * Version 1.05R1 2016/12/14 T.Ogawa
 * とりあえずリモコンの応答性が心配なので2台使用可能を中止する
 * 2台使いの方法は後で考えて、とりあえずエクシブ湯河原は1台バージョンで提供する
 *
			if (CurrentPollAddress == _ADDRESS_PRIMARY) {
				CurrentPollAddress = _ADDRESS_SECONDARY;
			}
			else {
				CurrentPollAddress = _ADDRESS_PRIMARY;
			}
*/
			/*
			 * Version 1.06 2016/12/23 T.Ogawa
			 * リモコンモード
			 *
			 * CNTL_DataSet[_CNTL_RC] : = 0    リモコン1台
			 * CNTL_DataSet[_CNTL_RC] : = 1    リモコン2台
			 * CNTL_DataSet[_CNTL_RC] : = 2    リモコン3台
			 */
			rcMode = (CNTL_DataSet[_CNTL_RC] & 0x07);	// <-- 上位5bitはOption Version1.06K 2017/09/08

			if (rcMode == 0) {
				/*
				 * リモコン1台モード
				 */
				CurrentPollAddress = _ADDRESS_PRIMARY;
			}
			else if (rcMode == 1) {
				/*
				 * リモコン2台モード
				 */
				if (CurrentPollAddress == _ADDRESS_PRIMARY) {
					CurrentPollAddress = _ADDRESS_SECONDARY;
				}
				else {
					CurrentPollAddress = _ADDRESS_PRIMARY;
				}
			}
			else {
				/*
				 * リモコン3台モード
				 */
				if (CurrentPollAddress == _ADDRESS_PRIMARY) {
					CurrentPollAddress = _ADDRESS_SECONDARY;
				}
				else if (CurrentPollAddress == _ADDRESS_SECONDARY) {
					CurrentPollAddress++;
				}
				else {
					CurrentPollAddress = _ADDRESS_PRIMARY;
				}
			}

		}
	}

}

#define	OPTION0_MASK	0b00001000
#define	OPTION1_MASK	0b00010000
#define	OPTION2_MASK	0b00100000
#define	OPTION3_MASK	0b01000000
#define	OPTION4_MASK	0b10000000

/*
 * Version1.06K 2017/09/08
 * 足湯モード判定
 * Option0 = 1であれば足湯モード
 */
bool IsFootBathMode(void)
{
	if (CNTL_DataSet[_CNTL_RC] &  OPTION0_MASK) {
		return true;
	}

	return false;
}

/*
 * Version1.06K 2017/09/08
 * さし水モード判定
 * Option1 = 1であれば足湯モード
 */
bool IsCoolingWaterMode(void)
{
	// 足湯モード時は使用不可
	if (CNTL_DataSet[_CNTL_RC] &  OPTION0_MASK) {
		return false;
	}
	if (CNTL_DataSet[_CNTL_RC] &  OPTION1_MASK) {
		return true;
	}

	return false;
}
/*
 * Version1.06K 2017/09/11
 * 温度のオフセット0.1℃刻み
 */
uint8_t CoolingWaterOffsetTemperature(void)
{
	uint8_t offset;

	offset = CNTL_DataSet[_CNTL_RC];
	offset >>=5;

	return offset;
}
