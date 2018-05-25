/*
 * netoperation.c
 *
 *  Created on: 2016/04/05
 *      Author: toshio
 */

#include "controldata.h"

#include "netoperation.h"
#include "http.h"
#include "pack.h"
#include "e2rom.h"
#include "w5500.h"
#include "datarom.h"
#include "timer32.h"
#include "ontimer.h"
#include "wdt.h"
#include "netdevice.h"
#include "operation.h"
#include "version.h"
#include "relay.h"


#define	NET_OPE_CODE_RUNSTOP		"ope_runstop"
#define	NET_OPE_CODE_RUNSTOP_LNG	1

#define	NET_OPE_CODE_RUNMODE		"ope_runmode"
#define	NET_OPE_CODE_RUNMODE_LNG	1

#define	NET_OPE_CODE_BASE			"ope_base"
#define	NET_OPE_CODE_BASE_LNG		((_CNTL_SUII - _CNTL_ONDO_L) + 1)

#define	NET_OPE_CODE_ONCHO			"ope_oncho"
#define	NET_OPE_CODE_ONCHO_LNG		((_CNTL_ONCHO_DELAY - _CNTL_ONCHO_HOSEI) + 1)

#define	NET_OPE_CODE_ONDOSETTEI		"ope_ondosettei"
#define	NET_OPE_CODE_ONDOSETTEI_LNG	((_CNTL_ONDO_SETTEI_H - _CNTL_ONDO_SETTEI_L) + 1)

#define	NET_OPE_CODE_SUII			"ope_suii"
#define	NET_OPE_CODE_SUII_LNG		((_CNTL_MIZUNUKI_TIME_STP - _CNTL_SUII_DEVICE) + 1)

#define	NET_OPE_CODE_NET			"ope_net"
#define	NET_OPE_CODE_NET_LNG		((_CNTL_SERVER3 - _CNTL_DHCP_ONOFF) + 1)



/*
 * 追加
 * 給水監視時間
 * ドライブリレー時間
 *
 * Version 1.05 2006/12/10 T.Ogawa
 */
/*
#define	NET_OPE_CODE_TERMNO			"ope_termno"
#define	NET_OPE_CODE_TERMNO_LNG		((_CNTL_TERM_NO_H - _CNTL_TERM_NO_L) + 1)
*/

#define	NET_OPE_CODE_SUPPLYMON		"ope_supplymonit"
#define	NET_OPE_CODE_SUPPLYMON_LNG	1

#define	NET_OPE_CODE_DRIVETIME		"ope_drivetime"
#define	NET_OPE_CODE_DRIVETIME_LNG	1
//--------------------------------------------------
/*
 * 追加
 * リモコンモード
 *
 * Version 1.06 2006/12/23 T.Ogawa
 */
#define	NET_OPE_CODE_RC				"ope_rcmode"
#define	NET_OPE_CODE_RC_LNG			1
//--------------------------------------------------

#define	NET_OPE_CODE_CLOCK			"ope_clock"
#define	NET_OPE_CODE_CLOCK_LNG		((_CNTL_STOP_TIMER_H - _CNTL_CLOCK_CNTL) + 1)


#define	NET_OPE_CODE_STATUS			"ope_status"
#define	NET_OPE_CODE_STATUS_LNG		((_CNTL_ERROR_STATE - _CNTL_RC) + 1)

#define	NET_OPE_CODE_VERSION		"ope_version"
#define	NET_OPE_CODE_VERSION_LNG	8

#define	NET_OPE_CODE_ROMREAD		"ope_romread"
#define	NET_OPE_CODE_ROMREAD_LNG	1

#define	NET_OPE_CODE_ROMWRITE		"ope_romwrite"
#define	NET_OPE_CODE_ROMWRITE_LNG	1

#define	NET_OPE_CODE_HTTPREQTIME		"ope_httpreqtime"
#define	NET_OPE_CODE_HTTPREQTIME_LNG	2

/*
 * Version1.06K 2017/09/11 T.Ogawa
 * リレー出力リクエスト追加
 */
#define	NET_OPE_CODE_RELAY			"ope_relay"
#define	NET_OPE_CODE_RELAY_LNG		1

/*
 * Version1.06L
 * 強制給水フラグ
 * このフラグは循環運転中のみ有効となる。
 */
#define	NET_OPE_CODE_FWS			"ope_fws"
#define	NET_OPE_CODE_FWS_LNG		1

/*
 * 通常モニタデータ
 */
#define	NET_OPE_CODE_GETSTATE		"ope_getstate"
#define	NET_OPE_CODE_GETSTATE_LNG	12

#define	NET_OPE_MODE_R		0
#define	NET_OPE_MODE_W		1

#define	NET_OPE_MODE_R_ARG	"GET /spot"
#define	NET_OPE_MODE_W_ARG	"GET /felis"


typedef struct {
	uint8_t		*pName;
	uint8_t		index;
	uint16_t	length;
	uint8_t		(*CoProcessing)(uint8_t mode, uint8_t index, uint8_t length, uint8_t *pdata);
} NET_OPE_CODE_TABLE, *PNET_OPE_CODE_TABLE;

uint8_t NetOPE_ReadWrite(uint8_t mode, uint8_t index, uint8_t length, uint8_t *pdata);
uint8_t NetOPE_ReadOnly(uint8_t mode, uint8_t index, uint8_t length, uint8_t *pdata);

uint8_t NetOPE_Clock(uint8_t mode, uint8_t index, uint8_t length, uint8_t *pdata);
uint8_t NetOPE_Version(uint8_t mode, uint8_t index, uint8_t length, uint8_t *pdata);
uint8_t NetOPE_RomRead(uint8_t mode, uint8_t index, uint8_t length, uint8_t *pdata);
uint8_t NetOPE_RomWrite(uint8_t mode, uint8_t index, uint8_t length, uint8_t *pdata);
uint8_t NetOPE_HTTPReqTime(uint8_t mode, uint8_t index, uint8_t length, uint8_t *pdata);

/*
 * Version1.06K 2017/09/11 T.Ogawa
 */
uint8_t NetOPE_Relay(uint8_t mode, uint8_t index, uint8_t length, uint8_t *pdata);

/*
 * Version1.06L 2018/02/05 T.Ogawa
 */
uint8_t NetOPE_ForceWaterSupply(uint8_t mode, uint8_t index, uint8_t length, uint8_t *pdata);
uint8_t NetOPE_ReadRREStatus(uint8_t mode, uint8_t index, uint8_t length, uint8_t *pdata);

uint8_t	NetOPE_GetState(uint8_t mode, uint8_t index, uint8_t length, uint8_t *pdata);

NET_OPE_CODE_TABLE	NetOparationTable[] = {
		{	(uint8_t *)NET_OPE_CODE_RUNSTOP,		_CNTL_RUN,				NET_OPE_CODE_RUNSTOP_LNG,		NetOPE_ReadWrite	},
		{	(uint8_t *)NET_OPE_CODE_RUNMODE,		_CNTL_RUNMODE,			NET_OPE_CODE_RUNMODE_LNG,		NetOPE_ReadWrite	},
		{	(uint8_t *)NET_OPE_CODE_BASE,			_CNTL_ONDO_L,			NET_OPE_CODE_BASE_LNG,			NetOPE_ReadOnly		},
		{	(uint8_t *)NET_OPE_CODE_ONCHO,			_CNTL_ONCHO_HOSEI,		NET_OPE_CODE_ONCHO_LNG,			NetOPE_ReadWrite	},
		{	(uint8_t *)NET_OPE_CODE_ONDOSETTEI,		_CNTL_ONDO_SETTEI_L,	NET_OPE_CODE_ONDOSETTEI_LNG,	NetOPE_ReadWrite	},
		{	(uint8_t *)NET_OPE_CODE_SUII,			_CNTL_SUII_DEVICE,		NET_OPE_CODE_SUII_LNG,			NetOPE_ReadWrite	},
		{	(uint8_t *)NET_OPE_CODE_NET,			_CNTL_DHCP_ONOFF,		NET_OPE_CODE_NET_LNG,			NetOPE_ReadWrite	},
/*
 * Version 1.05 2016/12/10 T,Ogawa
 */
//		{	(uint8_t *)NET_OPE_CODE_TERMNO,			_CNTL_TERM_NO_H,		NET_OPE_CODE_TERMNO_LNG,		NetOPE_ReadWrite	},

		{	(uint8_t *)NET_OPE_CODE_SUPPLYMON,		_CNTL_SUPPLYTIME,		NET_OPE_CODE_SUPPLYMON_LNG,		NetOPE_ReadWrite	},
		{	(uint8_t *)NET_OPE_CODE_DRIVETIME,		_CNTL_DRIVETIME,		NET_OPE_CODE_DRIVETIME_LNG,		NetOPE_ReadWrite	},
/*
 * Version 1.06 2016/12/23 T,Ogawa
 */
		{	(uint8_t *)NET_OPE_CODE_RC,				_CNTL_RC,				NET_OPE_CODE_RC_LNG,			NetOPE_ReadWrite	},

		{	(uint8_t *)NET_OPE_CODE_CLOCK,			_CNTL_CLOCK_CNTL,		NET_OPE_CODE_CLOCK_LNG,			NetOPE_Clock		},
		{	(uint8_t *)NET_OPE_CODE_STATUS,			_CNTL_RC,				NET_OPE_CODE_STATUS_LNG,		NetOPE_ReadRREStatus},
		{	(uint8_t *)NET_OPE_CODE_VERSION,		0,						NET_OPE_CODE_VERSION_LNG,		NetOPE_Version		},
		{	(uint8_t *)NET_OPE_CODE_ROMREAD,		0,						NET_OPE_CODE_ROMREAD_LNG,		NetOPE_RomRead		},
		{	(uint8_t *)NET_OPE_CODE_ROMWRITE,		0,						NET_OPE_CODE_ROMWRITE_LNG,		NetOPE_RomWrite		},
		{	(uint8_t *)NET_OPE_CODE_HTTPREQTIME,	0,						NET_OPE_CODE_HTTPREQTIME_LNG,	NetOPE_HTTPReqTime	},
/*
 * Version 1.06K 2017/09/11 T.Ogawa
 */
		{	(uint8_t *)NET_OPE_CODE_RELAY,			0,						NET_OPE_CODE_RELAY_LNG,			NetOPE_Relay	},
/*
 * Version 1.06L 2018/02/05 T.Ogawa
 */
		{	(uint8_t *)NET_OPE_CODE_FWS,			0,						NET_OPE_CODE_FWS_LNG,			NetOPE_ForceWaterSupply	},
		{	(uint8_t *)NET_OPE_CODE_GETSTATE,		0,						NET_OPE_CODE_GETSTATE_LNG,		NetOPE_GetState	},
		{	(uint8_t *)0,							0							}
};



uint8_t *pstrReadOKResponseLine0			= (uint8_t *)"HTTP/1.0 200 OK\r\nContent-Length: ";
uint8_t	*pstrReadOKResponseLine1			= (uint8_t *)"\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n";

uint8_t *pstrWriteOKResponseLine			= (uint8_t *)"HTTP/1.0 200 OK\r\nContent-Length: 2\r\nConnection: close\r\nContent-Type: text/html\r\n\r\nOK\r\n";
uint8_t *pstrWriteNGResponseLine			= (uint8_t *)"HTTP/1.0 200 OK\r\nContent-Length: 2\r\nConnection: close\r\nContent-Type: text/html\r\n\r\nNG\r\n";

uint8_t *pstrCommandNotFoundResponseLine	= (uint8_t *)"HTTP/1.0 200 OK\r\nContent-Length: 19\r\nConnection: close\r\nContent-Type: text/html\r\n\r\nCommand Not Found\r\n";

uint8_t *pstrBadResponseLine				= (uint8_t *)"HTTP/1.0 400 Bad Request\r\n\r\n";

uint8_t txHTMLBuffer[256];
uint8_t rxHTMLBuffer[256];

NET_LISTEN_DATA	opeData;


/*
 * HTTP Request周期
 */
#define	HTTP_REQUEST_TIME_DEF		60
#define	HTTP_REQUEST_ERRORTIME_DEF	10

volatile uint8_t HTTP_RequestSW = false;
PTIMER_BLOCK pHTTP_RequestTimer;
PTIMER_BLOCK pHTTP_RequestErrorTimer;


/*
 * Listen初期化
 */
PNET_LISTEN_DATA Init_Listen(void)
{
	opeData.nSocket		= 1;
	opeData.Port		= 80;
	opeData.Processing	= NetOPEProcessing;
	opeData.ptxBuffer	= txHTMLBuffer;
	opeData.txLength	= sizeof(txHTMLBuffer);
	opeData.prxBuffer 	= rxHTMLBuffer;
	opeData.szrxBuffer	= sizeof(rxHTMLBuffer);
	opeData.rxLength	= 0;

	return &opeData;
}

/*
 * 送信バッファ設定
 */
uint32_t SetSendHTMLBuffer(uint8_t *ptxBuffer, uint8_t lngContent, uint8_t *pBuffer)
{
	uint32_t Length = 0;
	uint8_t *pDum;

	uint8_t *pOKLine0 = pstrReadOKResponseLine0;
	uint8_t *pOKLine1 = pstrReadOKResponseLine1;

	while(true) {
		if (*pOKLine0 == 0) {
			break;
		}
		*ptxBuffer = *pOKLine0;
		ptxBuffer++;
		pOKLine0++;
		Length++;
	}

	pDum = Value2decStr(ptxBuffer, lngContent * 2);

	Length += (pDum - ptxBuffer);
	ptxBuffer = pDum;

	while(true) {
		if (*pOKLine1 == 0) {
			break;
		}
		*ptxBuffer = *pOKLine1;
		ptxBuffer++;
		pOKLine1++;
		Length++;
	}

	ptxBuffer = Value2hexStr(ptxBuffer, pBuffer, lngContent);
	Length += (lngContent * 2);

	return Length;
}

uint32_t CopySendHTMLBuffer(uint8_t *pD, uint8_t *pS)
{
	uint32_t Length = 0;

	while(1) {
		if (*pS == 0) {
			break;
		}
		*pD = *pS;
		pD++;
		pS++;
		Length++;
	}

	return Length;
}

uint8_t HexStringTOBinary(uint8_t *pinBuffer, uint16_t Length, uint8_t *poutBuffer)
{
	uint16_t i;
	int32_t dt;

	for (i = 0; i < Length; i++) {
		dt = hexStr2Value(pinBuffer, 2);
		if (dt < 0) {
			/*
			 * エラー
			 */
			return 0;
		}
		*poutBuffer = (uint8_t)dt;
		poutBuffer++;
		pinBuffer++;
		pinBuffer++;
	}

	if (*pinBuffer != 0x20) {
		/*
		 * 次が空白じゃないとエラー
		 */
		return 0;
	}
	return 1;
}


uint8_t	*pstrTest =	(uint8_t *)"HTTP/1.0 200 OK\r\n"
							"Content-Length: 5\r\n"
							"Connection: close\r\n"
							"Content-Type: text/html\r\n\r\n"
							"abcdef\r\n";

uint8_t *pstrOKtst	= (uint8_t *)"HTTP/1.0 200 OK\r\nContent-Length: 8\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n01020304";

uint8_t	ProcessParam[100];

int8_t NetOPEProcessing(void *pDat)
{
	PNET_LISTEN_DATA pListenData = (PNET_LISTEN_DATA)pDat;
	uint8_t *pParam;
	uint8_t	*pCommand;
	uint8_t mode;
	int8_t  index;


	/*
	 * 忘れがちなのでバッファを設定
	 */
	pListenData->ptxBuffer	= txHTMLBuffer;
	pListenData->txLength	= 0;

	/*
	 * GET /xxxx or GET /yyyyを検索
	 */
	pParam = HTTP_GetPhrase((uint8_t *)NET_OPE_MODE_R_ARG, pListenData->prxBuffer, pListenData->rxLength);
	if (pParam) {
		mode = NET_OPE_MODE_R;
	}
	else {
		pParam = HTTP_GetPhrase((uint8_t *)NET_OPE_MODE_W_ARG, pListenData->prxBuffer, pListenData->rxLength);
		if (!pParam) {
			/*
			 * 何も指定が無い！
			 */
			pListenData->txLength = CopySendHTMLBuffer(pListenData->ptxBuffer, pstrBadResponseLine);
			return 1;
		}
		mode = NET_OPE_MODE_W;
	}
	/*
	 * 区切り文字チェック
	 */
	if (*pParam != '?') {
		pListenData->txLength = CopySendHTMLBuffer(pListenData->ptxBuffer, pstrBadResponseLine);
		return 1;
	}
	pParam++;
	/*
	 * NET_OPE_CODE_TABLEから対応関数を検索する
	 */
	index = 0;
	while(1) {
		if (!NetOparationTable[index].pName) {
			/*
			 * 見つからなかった
			 */
			pListenData->txLength = CopySendHTMLBuffer(pListenData->ptxBuffer, pstrCommandNotFoundResponseLine);
			return 1;
		}
		pCommand = HTTP_GetPhrase(NetOparationTable[index].pName, pParam, pListenData->rxLength - 20);
		if (pCommand) {
			/*
			 * 見つかった
			 */
			break;
		}
		index++;
	}

	if (*pCommand != '=') {
		pListenData->txLength = CopySendHTMLBuffer(pListenData->ptxBuffer, pstrBadResponseLine);
		return 1;
	}

	pCommand++;

	/*
	 * もし書込モードなら引数を変換する
	 * 末尾がFFだったらEEPROMに書き込む
	 */
	if (mode == NET_OPE_MODE_W) {
		if (!HexStringTOBinary(pCommand, NetOparationTable[index].length, ProcessParam)) {
			/*
			 * 変換出来ないからエラー
			 */
			pListenData->txLength = CopySendHTMLBuffer(pListenData->ptxBuffer, pstrWriteNGResponseLine);
			return 1;
		}
	}

	/*
	 * 処理関数を呼出す
	 */
	if (!NetOparationTable[index].CoProcessing(mode, NetOparationTable[index].index, NetOparationTable[index].length, ProcessParam)) {
		pListenData->txLength = CopySendHTMLBuffer(pListenData->ptxBuffer, pstrBadResponseLine);
		return 1;
	}

	/*
	 * もし読込モードなら数値変換する
	 */
	if (mode == NET_OPE_MODE_R) {
		pListenData->txLength = SetSendHTMLBuffer(pListenData->ptxBuffer, NetOparationTable[index].length, ProcessParam);
	}
	else {
		pListenData->txLength = CopySendHTMLBuffer(pListenData->ptxBuffer, pstrWriteOKResponseLine);
	}

	return 1;
}


/*
 * Read/Write可能
 */
uint8_t NetOPE_ReadWrite(uint8_t mode, uint8_t index, uint8_t length, uint8_t *pdata)
{
	uint8_t i;

	if (mode == NET_OPE_MODE_R) {
		/*
		 * READ Mode
		 */
		for (i = 0; i < length; i++) {
			*pdata = CNTL_DataSet[index + i];
			pdata++;
		}
	}
	else {
		/*
		 * WRITE Mode
		 */
		for (i = 0; i < length; i++) {
			CNTL_DataSet[index + i] = *pdata;
			pdata++;
		}
		/*
		 *  HTTP 報告開始追加
		 *
		 *  Version ZVersion製品版1.04 2016/09/13 T.Ogawa
		 */
		_HTTP_REPORT;
	}

	return 1;
}

/*
 * リードオンリー
 */
uint8_t NetOPE_ReadOnly(uint8_t mode, uint8_t index, uint8_t length, uint8_t *pdata)
{
	uint8_t i;

	if (mode == NET_OPE_MODE_R) {
		/*
		 * READ Mode
		 */
		for (i = 0; i < length; i++) {
			*pdata = CNTL_DataSet[index + i];
			pdata++;
		}
	}

	return 1;
}


/*
 * Version1.06L 2018/02/12 T.Ogawa
 */

uint8_t NetOPE_ReadRREStatus(uint8_t mode, uint8_t index, uint8_t length, uint8_t *pdata)
{
	if (mode == NET_OPE_MODE_R) {
		/*
		 * READ Mode
		 */
		*(pdata + 0) = CNTL_DataSet[_CNTL_RC];
		*(pdata + 1) = CNTL_DataSet[_CNTL_RELAY_STATE];
		*(pdata + 2) = CNTL_DataSet[_CNTL_ERROR_STATE];

		if (m_ForceWaterSupply) {
			*(pdata + 1) |= _CNTL_RY_BIT_FWS;
		}
	}

	return 1;
}




uint8_t NetOPE_Clock(uint8_t mode, uint8_t index, uint8_t length, uint8_t *pdata)
{
	uint8_t i;

	if (mode == NET_OPE_MODE_R) {
		/*
		 * READ Mode
		 */
		for (i = 0; i < length; i++) {
			*pdata = CNTL_DataSet[index + i];
			pdata++;
		}
	}
	else {
		/*
		 * WRITE Mode 時計の設定はしない！
		 */
		CNTL_DataSet[index + 0] = *(pdata + 0);
		CNTL_DataSet[index + 5] = *(pdata + 5);
		CNTL_DataSet[index + 6] = *(pdata + 6);
		CNTL_DataSet[index + 7] = *(pdata + 7);
		CNTL_DataSet[index + 8] = *(pdata + 8);

		/*
		 * デフォルトのタイマー起動条件を設定する
		 */
		OnTimerRestartCheck();

	}

	return 1;
}

uint8_t NetOPE_Version(uint8_t mode, uint8_t index, uint8_t length, uint8_t *pdata)
{
	/*
	 * データ長は8バイト
	 */
	if (mode == NET_OPE_MODE_R) {
		/*
		 * READ Mode
		 * ZVersion1.02			Version 0101 Revision 0107	Jly  7th 2016
		 * ZVersion1.03			Version 0001 Revision 0003	Aug  2th 2016
		 * ZVersion1.05			Version 0001 Revision 0005	Dec 10th 2016
		 *
		 */
/*
 * Version 1.06 2016/12/23 T.Ogawa

		// Version
		*(pdata + 0)	= 0x00;
		*(pdata + 1)	= 0x00;
		*(pdata + 2)	= 0x00;
		*(pdata + 3)	= 0x01;
		// Revision
		*(pdata + 4)	= 0x00;
		*(pdata + 5)	= 0x00;
		*(pdata + 6)	= 0x00;
		*(pdata + 7)	= 0x05;
*/
		SetNetVersion(pdata, pdata + 4);
	}
	return 1;
}

uint8_t NetOPE_RomRead(uint8_t mode, uint8_t index, uint8_t length, uint8_t *pdata)
{
	if (mode == NET_OPE_MODE_R) {
		*pdata = (uint8_t)ParameterResume();
	}

	return 1;
}

uint8_t NetOPE_RomWrite(uint8_t mode, uint8_t index, uint8_t length, uint8_t *pdata)
{
	if (mode == NET_OPE_MODE_W) {
		*pdata = (uint8_t)ParameterSuspend();
	}

	return 1;
}

uint8_t NetOPE_HTTPReqTime(uint8_t mode, uint8_t index, uint8_t length, uint8_t *pdata)
{
	uint32_t Time;

	if (mode == NET_OPE_MODE_R) {
		Time = pHTTP_RequestTimer->preTime;

		*(pdata + 1) = (Time & 0xff);
		Time >>= 8;
		*pdata = (Time & 0xff);
	}
	else {
		Time = *pdata;
		Time <<= 8;
		Time |= *(pdata + 1);
		pHTTP_RequestTimer->preTime = Time;
	}

	return 1;
}
/*
 * Version1.06K 2017/09/11 T.Ogawa
 * リレー出力リクエスト。これは停止状態の時だけ有効
 */
uint8_t NetOPE_Relay(uint8_t mode, uint8_t index, uint8_t length, uint8_t *pdata)
{
	if (mode == NET_OPE_MODE_R) {
		*pdata = CNTL_DataSet[_CNTL_RELAY_STATE];
	}
	else if (CNTL_DataSet[_CNTL_RUN] == 0) {
		// 出力は停止中の時だけ有効

		// Drive
		if (*pdata & _CNTL_RY_BIT_DRIVE) {
			SET_RELAY_DRIVE(_RELEAY_ON);
		}
		else {
			SET_RELAY_DRIVE(_RELEAY_OFF);
		}

		// Heater
		if (*pdata & _CNTL_RY_BIT_HEATER) {
			SET_RELAY_HEATER(_RELEAY_ON);
		}
		else {
			SET_RELAY_HEATER(_RELEAY_OFF);
		}

		// Water
		if (*pdata & _CNTL_RY_BIT_WATER) {
			SET_RELAY_WATER(_RELEAY_ON);
		}
		else {
			SET_RELAY_WATER(_RELEAY_OFF);
		}

		// Pump
		if (*pdata & _CNTL_RY_BIT_PUMP) {
			SET_RELAY_PUMP(_RELEAY_ON);
		}
		else {
			SET_RELAY_PUMP(_RELEAY_OFF);
		}

		// Drain
		if (*pdata & _CNTL_RY_BIT_DRAINAGE) {
			SET_RELAY_DRAINAGE(_RELEAY_ON);
		}
		else {
			SET_RELAY_DRAINAGE(_RELEAY_OFF);
		}

		// Error
		if (*pdata & _CNTL_RY_BIT_ERROR) {
			SET_RELAY_ERROR(_RELEAY_ON);
		}
		else {
			SET_RELAY_ERROR(_RELEAY_OFF);
		}
		_HTTP_REPORT;
	}

	return 1;
}
/*
 * Version1.06L 2018/02/05 T.Ogawa
 * リレー出力リクエスト。これは停止状態の時だけ有効
 */
uint8_t NetOPE_ForceWaterSupply(uint8_t mode, uint8_t index, uint8_t length, uint8_t *pdata)
{
	if (mode == NET_OPE_MODE_R) {
		*pdata = m_ForceWaterSupply;
	}
	else if (CNTL_DataSet[_CNTL_RUN] == 1) {
		// 出力は停止中の時だけ有効
		m_ForceWaterSupply = (*pdata & 0x01);
		_HTTP_REPORT;
	}

	return 1;
}
/*
 * 通常のモニタで使用する（リードのみ有効）
 *  */
uint8_t NetOPE_GetState(uint8_t mode, uint8_t index, uint8_t length, uint8_t *pdata)
{
	if (mode == NET_OPE_MODE_R) {
		*(pdata + 0) = 0x18;
		*(pdata + 1) = CNTL_DataSet[_CNTL_RUN];
		*(pdata + 2) = CNTL_DataSet[_CNTL_RUNMODE];
		*(pdata + 3) = CNTL_DataSet[_CNTL_ONDO_L];
		*(pdata + 4) = CNTL_DataSet[_CNTL_ONDO_H];
		*(pdata + 5) = CNTL_DataSet[_CNTL_SUII_LEVEL];
		*(pdata + 6) = CNTL_DataSet[_CNTL_SUII];
		*(pdata + 7) = CNTL_DataSet[_CNTL_ONDO_SETTEI_L];
		*(pdata + 8) = CNTL_DataSet[_CNTL_ONDO_SETTEI_H];
		*(pdata + 9) = CNTL_DataSet[_CNTL_RC];
		*(pdata + 10) = CNTL_DataSet[_CNTL_RELAY_STATE];
		if (m_ForceWaterSupply) {
			*(pdata + 10) |= _CNTL_RY_BIT_FWS;
		}
		*(pdata + 11) = CNTL_DataSet[_CNTL_ERROR_STATE];
	}

	return 1;
}


HTTP_REQUEST_VALUE	HTTP_RequestData;

/*
 *
 */
void SendServer()
{
	int8_t status;

	status = GetHTTP_RequestStatus();

	if (status) {

	}
}

#define	HTTP_REQ_SOCK_NO		0
#define	HTTP_REQ_SERVER_PORT	80
#define	HTTP_LISTEN_SOCK_NO	1


uint8_t HTTPRequestTxBuffer[256];
uint8_t HTTPRequestRxBuffer[256];

uint8_t *pstrRequestHeader 	=	(uint8_t *)"GET /get_para.php?name=";
uint8_t *pstrRequestTail	=	(uint8_t *)" HTTP/1.0\r\n\r\n";

uint32_t SetHTTP_RequestData(uint8_t *poutBuffer, uint8_t *pHeader) {

	uint32_t Length = 0;
	uint8_t Work[2];

	/*
	 * ヘッダー部分
	 */
	while(1) {
		if (*pHeader == 0) {
			break;
		}
		*poutBuffer = *pHeader;
		poutBuffer++;
		pHeader++;
		Length++;
	}

	/*
	 * データ部分
	 */
	/*
	 * _CNTL_RUNから_CNTL_RUNMODEまで
	 */
	poutBuffer = Value2hexStr(poutBuffer, &CNTL_DataSet[_CNTL_RUN], 6);
	Length += 12;
	/*
	 * _CNTL_ONDO_SETTEIまで
	 */
	poutBuffer = Value2hexStr(poutBuffer, &CNTL_DataSet[_CNTL_ONDO_SETTEI_L], 2);
	Length += 4;
	/*
	 * __CNTL_SUII_DEVICE
	 */
	poutBuffer = Value2hexStr(poutBuffer, &CNTL_DataSet[_CNTL_SUII_DEVICE], 1);
	Length += 2;
	/*
	 * データ部分
	 */
	/*
	 * _CNTL_RELAY_STATEから_CNTL_ERROR_STATEまで
	 */

	/*
	 * Version1.06L
	 * 強制給水フラグ
	 * このフラグは循環運転中のみ有効となる。
	 */

//	poutBuffer = Value2hexStr(poutBuffer, &CNTL_DataSet[_CNTL_RELAY_STATE], 2);
	Work[0] = CNTL_DataSet[_CNTL_RELAY_STATE];
	Work[1] = CNTL_DataSet[_CNTL_ERROR_STATE];

	if (m_ForceWaterSupply) {
		Work[0] |= _CNTL_RY_BIT_FWS;
	}
	poutBuffer = Value2hexStr(poutBuffer, &Work[0], 2);

	Length += 4;

	/*
	 * IPアドレス
	 */
	poutBuffer = Value2hexStr(poutBuffer, &CNTL_DataSet[_CNTL_IP0], 4);
	Length += 8;


	uint8_t ADbuff[6];
	/*
	 * DEVICE IPアドレス
	 */
	W5500_ReadBuffer(W5500_COM_SOURCE_IP_ADDRESS, ADbuff, 4);
	poutBuffer = Value2hexStr(poutBuffer, ADbuff, 4);
	Length += 8;

	/*
	 * DEVICE macアドレス
	 */
	W5500_ReadBuffer(W5500_COM_SOURCE_HARDWARE_ADDRESS, ADbuff, 6);
	poutBuffer = Value2hexStr(poutBuffer, ADbuff, 6);
	Length += 12;

	/*
	 * Request Port
	 */
	poutBuffer = Value2hexStr(poutBuffer, (uint8_t *)&gSurcePortNumber, 2);
	Length += 4;

	/*
	 * デバイスリスタート回数
	 */
	poutBuffer = Value2hexStr(poutBuffer, (uint8_t *)&_DeviceRestartCount, 1);
	Length += 2;

	/*
	 * WDTカウンタ数
	 */
	poutBuffer = Value2hexStr(poutBuffer, (uint8_t *)&WDT_CurrentCount, 4);
	Length += 8;

	/*
	 * ReBoot回数
	 */
	poutBuffer = Value2hexStr(poutBuffer, (uint8_t *)&ReBoot_Count, 2);
	Length += 4;

	/*
	 * ReBoot Status
	 */
	poutBuffer = Value2hexStr(poutBuffer, (uint8_t *)&Reboot_Status, 1);
	Length += 2;

	/*
	 * Scan Time
	 */
	poutBuffer = Value2hexStr(poutBuffer, (uint8_t *)&CurrentScanTime, 4);
	Length += 8;

	/*
	 * Max Scan Time
	 */
	poutBuffer = Value2hexStr(poutBuffer, (uint8_t *)&MaxScanTime, 4);
	Length += 8;


	/*
	 *しっぽ部分
	 */
	uint8_t *pTale = pstrRequestTail;
	while(1) {
		if (*pTale == 0) {
			break;
		}
		*poutBuffer = *pTale;
		poutBuffer++;
		pTale++;
		Length++;
	}


	return Length;
}

/*
 * 定期報告周期を管理
 */
void HTTP_RequestTimerHandler(void)
{
	HTTP_RequestSW = true;
}
/*
 * エラー時には周期を短縮
 */
void HTTP_RequestErrorTimerHandler(void)
{
	HTTP_RequestSW = true;
}

void Init_HTTP_Request(void)
{
	// 定期報告タイマ
	pHTTP_RequestTimer = AllocTimer0Task(HTTP_REQUEST_TIME_DEF, HTTP_RequestTimerHandler);
	EnableTimerTask(pHTTP_RequestTimer);
	// エラー報告タイマ
	pHTTP_RequestErrorTimer = AllocTimer0Task(HTTP_REQUEST_ERRORTIME_DEF, HTTP_RequestErrorTimerHandler);
	DisableTimerTask(pHTTP_RequestErrorTimer);
	/*
	 * 初期は有効
	 */
	HTTP_RequestSW = true;
}

void HTTP_RequestFunc(void)
{
	int8_t status;

	/*
	 *
	 */
/*
 *------- 箱根芦ノ湖での通信エラー問題対策
*/
#if 0
	if (CNTL_DataSet[_CNTL_ERROR_STATE]) {
		/*
		 * エラー発生中ならエラー報告用タイマを起動
		 */
		if (pHTTP_RequestErrorTimer->bAvailable == false) {
			EnableTimerTask(pHTTP_RequestErrorTimer);
			HTTP_RequestSW = true;
		}
	}
	else {
		/*
		 * エラーがなくなったら報告用タイマを停止
		 */
		DisableTimerTask(pHTTP_RequestErrorTimer);
	}
#endif

	status = HTTP_Request(&HTTP_RequestData);

	if (HTTP_RequestSW == true && status == HTTP_REQ_IDLE) {
		/*
		 *  リクエストスイッチオフ
		 *
		 *  Version ZVersion製品版1.04 2016/09/18 T.Ogawa
		 */

		HTTP_RequestSW = false;

		HTTP_RequestData.Socket = socket(HTTP_REQ_SOCK_NO, SOCK_TCP, 0, 0);

		HTTP_RequestData.pSrvAddress	= &CNTL_DataSet[_CNTL_SERVER0];
		HTTP_RequestData.pUrn			= 0;
		HTTP_RequestData.port			= HTTP_REQ_SERVER_PORT;
		HTTP_RequestData.ptxBuffer		= HTTPRequestTxBuffer;
		HTTP_RequestData.txLength		= SetHTTP_RequestData(HTTPRequestTxBuffer, pstrRequestHeader);
		HTTP_RequestData.prxBuffer		= HTTPRequestRxBuffer;
		HTTP_RequestData.rxLength		= sizeof(HTTPRequestRxBuffer);
		HTTP_RequestData.ReceivedLength	= 0;
		SetHTTP_RequestStatus(HTTP_REQ_CONNECT);
	}
	if (status == HTTP_REQ_COMPLETE || status == HTTP_REQ_ABORT) {
		close_socket(HTTP_RequestData.Socket);
		SetHTTP_RequestStatus(HTTP_REQ_IDLE);
		if (status == HTTP_REQ_COMPLETE) {
			/*
			 * ベースクロック更新
			 */
			BASE_CLOCK_REFLESH;
		}
		/*
		 *  リクエストスイッチオフ取り消し
		 *  ここでオフするとリクエストが消える場合がある
		 *
		 *  Version ZVersion製品版1.04 2016/09/18 T.Ogawa
		 */
		//HTTP_RequestSW = false;
	}
}

