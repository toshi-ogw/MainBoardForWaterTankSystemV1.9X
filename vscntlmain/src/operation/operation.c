/*
 * operation.c
 *
 *  Created on: 2015/12/14
 *      Author: toshio
 */

#include "controldata.h"
#include "operation.h"
#include "dmanage.h"
#include "relay.h"
#include "timer32.h"
#include "sensordrive.h"
#include "errorcode.h"
#include "netoperation.h"
#include "ontimer.h"
#include "contactor.h"
#include "powerled.h"
#include "datarom.h"
#include "tick.h"
#include "wdt.h"

#define	_OPE_STATUS_IDLE		0		// 停止中
#define	_OPE_STATUS_START		1		// 運転開始
#define	_OPE_STATUS_DRAINAGE	2		// 抜水中
#define	_OPE_STATUS_SUPPLY		3		// 給水中
#define	_OPE_STATUS_RUNSUPPLY	4		// 給水中(循環運転中)
#define	_OPE_STATUS_RUNNING		5		// 循環運転中
#define	_OPE_STATUS_ERROR		6		// エラー処理中

/*
 * オペレーションステータス
 */

uint8_t	_OperationStatus = _OPE_STATUS_IDLE;

#define	SET_OPSTATE_IDLE		(_OperationStatus = _OPE_STATUS_IDLE)
#define	SET_OPSTATE_START		(_OperationStatus = _OPE_STATUS_START)
#define	SET_OPSTATE_DRAINAGE	(_OperationStatus = _OPE_STATUS_DRAINAGE)
#define	SET_OPSTATE_SUPPLY		(_OperationStatus = _OPE_STATUS_SUPPLY)
#define	SET_OPSTATE_RUNSUPPLY	(_OperationStatus = _OPE_STATUS_RUNSUPPLY)
#define	SET_OPSTATE_RUNNING		(_OperationStatus = _OPE_STATUS_RUNNING)
#define	SET_OPSTATE_ERROR		(_OperationStatus = _OPE_STATUS_ERROR)

/*
 * 監視タイマー
 */
// 抜水タイマー
PTIMER_BLOCK	_pOP_DrainageOnStartTimer = NULL;
PTIMER_BLOCK	_pOP_DrainageOnStopTimer = NULL;
PTIMER_BLOCK	_pOP_FullWaterTimer = NULL;


/* Version1.06B
 * 温度監視無視タイマー
 */
void TemperatureNotRatedHandler(void);

PTIMER_BLOCK	_pOP_TemperatureNotRatedTimer = NULL;
bool			_bIsTemperatureNotRated = false;

// 温度監視無視中？
#define	IS_TEMPERATURE_NOT_RATED	_bIsTemperatureNotRated



/*
 * Version 1.05 2016/12/12 T.Ogawa
 * 停止時のドライブリレーON時間
 */
PTIMER_BLOCK	_pOP_DriveRelayTimer = NULL;

/*
 * Version 1.05 2016/12/12 T.Ogawa
 * 給水監視 時間
 *
 * 1.給水開始時にタイマ値をセットしいネーブルする
 * 2.タイムアップしたら水位をチェックしEEだったらエラーにする（給水は停止しない）
 * 3.タイムアップしたら水位をチェックしEEじゃなかったら再度タイマ値（）をセットし監視を続ける
 * 4.満水になったら監視を終了する
 *
 */
PTIMER_BLOCK	_pOP_SupplyMonitorTimer	= NULL;

void DriveRelayTimerHandler(void);
void SupplyMonitorHandler(void);

/*
 * scan time
 */
uint32_t	CurrentScanTime = 0;
uint32_t	MaxScanTime = 0;

/*
 * ヒーター加熱中フラグ
 */
bool bHeatingNow = false;

void StartDrainage(void);
void StartWaterSupply(void);
void DrainageOnStartTimerHandler(void);
void DrainageOnStopTimerHandler(void);
void FullWaterTimerHandler(void);
void TemperatureMonitoring(void);

void Shutdown(bool bDriveRelay);

void Ope_Error(void);

void StopDrainageOnStart(void);
void StopDrainageOnStop(void);



/*
 * Version1.06K 2017/09/08 T,Ogawa
 * 足湯コントロール
 */
void FootBathControll() {

	uint8_t Suii = CNTL_DataSet[_CNTL_SUII_LEVEL];


	if (IsFootBathMode()) {
		/*
		 * 足湯モードであっても循環運転中以外は何もしない
		 */
		if (_OperationStatus == _OPE_STATUS_RUNSUPPLY || _OperationStatus == _OPE_STATUS_RUNNING) {
			/*
			 * Foot Bathモードであれば...
			 */
			if (Suii == _WATERLEVEL_EE) {
				// 渇水レベル->Drainを閉める
				SET_RELAY_DRAINAGE(_RELEAY_ON);
			}
			else {
				// 渇水じゃ無い->Drainを開ける
				SET_RELAY_DRAINAGE(_RELEAY_OFF);
			}
		}
	}
}


void Init_Operation(void)
{
	/*
	 *  Modify July,4th 2016
	 *  運転開始時に抜水時間（秒単位）を設定することにしたが
	 *  運転停止時にも排水時間（分単位）を設定できるように変更
	 *  今までは抜水時間をワードサイズで設定していたが上限をバイトサイズ（２５５秒）までとし
	 *  上位バイトを排水時間（分単位）で指定できるように変更する
	 */
	_pOP_DrainageOnStartTimer	= AllocTimer0Task(CNTL_DataSet[_CNTL_MIZUNUKI_TIME_STA], DrainageOnStartTimerHandler);
	_pOP_DrainageOnStopTimer	= AllocTimer0Task(CNTL_DataSet[_CNTL_MIZUNUKI_TIME_STP], DrainageOnStopTimerHandler);
	_pOP_FullWaterTimer			= AllocTimer0Task((CNTL_DataSet[_CNTL_MANSUI_TIME] * 360), FullWaterTimerHandler);

	/*
	 * Version 1.05 2016/12/12 T.Ogawa
	 * 監視タイマを追加
	 */
	_pOP_DriveRelayTimer		= AllocTimer0Task(CNTL_DataSet[_CNTL_DRIVETIME], DriveRelayTimerHandler);
	_pOP_SupplyMonitorTimer		= AllocTimer0Task(CNTL_DataSet[_CNTL_SUPPLYTIME], SupplyMonitorHandler);

	/* Version1.06B
	 * 温度監視無視タイマー
	 * 時間はドライブタイムと共通
	 */
	_pOP_TemperatureNotRatedTimer = AllocTimer0Task(CNTL_DataSet[_CNTL_DRIVETIME], TemperatureNotRatedHandler);

}

void StartTemperatureNotRated(void) {
	/*
	 * タイマーを停止する
	 */
	DisableTimerTask(_pOP_TemperatureNotRatedTimer);

	/*
	 * タイマー値を設定
	 */
	SetTimerTime(_pOP_TemperatureNotRatedTimer, CNTL_DataSet[_CNTL_DRIVETIME]);

	_bIsTemperatureNotRated = true;
	EnableTimerTask(_pOP_TemperatureNotRatedTimer);
}

/* Version1.06B
 * 温度監視無視タイマー
 * 時間はドライブタイムと共通
 */
void TemperatureNotRatedHandler(void) {
	/*
	 * タイマーを停止する
	 */
	DisableTimerTask(_pOP_TemperatureNotRatedTimer);
	_bIsTemperatureNotRated = false;
}

/*
 * 運転開始時にドライブリレーを初期化する
 * 要するに停止時のドライブリレーオンをキャンセルする
 */
void DriveRelayCancel(void)
{
	/*
	 * DriveRelay Timerを停止
	 */
	DisableTimerTask(_pOP_DriveRelayTimer);
	/*
	 *  念の為ドライブOFF
	 */
	SET_RELAY_DRIVE(_RELEAY_OFF);
}

/*
 * 運転停止時にドライブリレーのオン時間を決める
 * 設定時間分だけドライブリレーをオンする
 */
void SetDriveRelayOffTime(void)
{
	unsigned char time = CNTL_DataSet[_CNTL_DRIVETIME];

	if (time == 0) {
		/*
		 * オン時間無し
		 */
		/*
		 *  ドライブOFF
		 */
		SET_RELAY_DRIVE(_RELEAY_OFF);
		return;
	}
	/*
	 * 念の為DriveRelay Timerを停止
	 */
	DisableTimerTask(_pOP_DriveRelayTimer);
	/*
	 * タイマーセット
	 */
	SetTimerTime(_pOP_DriveRelayTimer, time);
	EnableTimerTask(_pOP_DriveRelayTimer);
	/*
	 *  念の為ドライブON
	 */
	SET_RELAY_DRIVE(_RELEAY_ON);
}

void DriveRelayTimerHandler(void)
{
	/*
	 * タイマー停止
	 */
	DisableTimerTask(_pOP_DriveRelayTimer);

	/*
	 * Drive Relayオフ
	 */
	/*
	 *  ドライブOFF
	 */
	SET_RELAY_DRIVE(_RELEAY_OFF);
}

void SetSupplyMonitor(void)
{
	unsigned char time = CNTL_DataSet[_CNTL_SUPPLYTIME];
	/*
	 * 念の為停止
	 */
	DisableTimerTask(_pOP_SupplyMonitorTimer);

	if (time == 0) {
		/*
		 * 設定時間が０分なら監視を行わない
		 */
		return;
	}
	/*
	 * 監視タイマ設定
	 */
	SetTimerTime(_pOP_SupplyMonitorTimer, time * 60);
	EnableTimerTask(_pOP_SupplyMonitorTimer);
}

void ResetSupplyMonitor(void)
{
	/*
	 * 念の為停止
	 */
	DisableTimerTask(_pOP_SupplyMonitorTimer);
}

void SupplyMonitorHandler(void)
{
	/*
	 * 一旦タイマーを停止する
	 */
	DisableTimerTask(_pOP_SupplyMonitorTimer);
	/*
	 * 水位を確認
	 */
	uint8_t Suii = CNTL_DataSet[_CNTL_SUII_LEVEL];
	if (Suii == _WATERLEVEL_E1) {
		/*
		 * 満水なら終了
		 */
		return;
	}
	if (Suii == _WATERLEVEL_EE) {
		/*
		 * 給水エラー
		 */
		/*
		 * Version1.06->Version1.06A
		 * エラー番号を修正！_ERRORCODE_WATERLEVEL_SENSOR->_ERRORCODE_WATER_EMPTY
		 *
		 */
		//CNTL_DataSet[_CNTL_ERROR_STATE] |= _ERRORCODE_WATERLEVEL_SENSOR;
		CNTL_DataSet[_CNTL_ERROR_STATE] |= _ERRORCODE_WATER_EMPTY;
		/*
		 * エラーになったら給水（その他）を停止して復帰はしない
		 */
		SET_RELAY_HEATER(_RELEAY_OFF);
		SET_RELAY_WATER(_RELEAY_OFF);
		SET_RELAY_PUMP(_RELEAY_OFF);
		return;
	}
	else {
		/*
		 * 給水エラークリア
		 */
		CNTL_DataSet[_CNTL_ERROR_STATE] &= ~_ERRORCODE_WATERLEVEL_SENSOR;
	}

	unsigned char time = CNTL_DataSet[_CNTL_SUPPLYTIME];
	if (time == 0) {
		/*
		 * 設定時間が０分なら監視を行わない
		 */
		return;
	}
	/*
	 * 監視タイマ設定
	 */
	SetTimerTime(_pOP_SupplyMonitorTimer, time * 60);
	EnableTimerTask(_pOP_SupplyMonitorTimer);
}

/*
 * 運転開始時の抜水タイマーハンドラー
 */
void DrainageOnStartTimerHandler(void)
{
	/*
	 * タイマー停止
	 */
	DisableTimerTask(_pOP_DrainageOnStartTimer);

	/*
	 * 抜水バルブを閉じる
	 * Heater -> Do not care
	 * Pump -> Do Not Care
	 */
	SET_RELAY_DRAINAGE(_RELEAY_OFF);

	/*
	 *  HTTP 報告開始追加
	 *
	 *  Version ZVersion製品版1.04 2016/09/19 T.Ogawa
	 */
	_HTTP_REPORT;
}

/*
 * 運転停止時の抜水タイマーハンドラー
 */
void DrainageOnStopTimerHandler(void)
{
	/*
	 * タイマー停止
	 */
	DisableTimerTask(_pOP_DrainageOnStopTimer);

	/*
	 *  ドライブOFF
	 *
	 * Version 1.05 2016/12/12 T.Ogawa
	 *
	 * Driver Relay 処理で行うのでコメントアウト
	 *
	 */
	//SET_RELAY_DRIVE(_RELEAY_OFF);

	/*
	 * 抜水バルブを閉じる
	 * Heater -> Do not care
	 * Pump -> Do Not Care
	 */
	SET_RELAY_DRAINAGE(_RELEAY_OFF);

	/*
	 *  HTTP 報告開始追加
	 *
	 *  Version ZVersion製品版1.04 2016/09/19 T.Ogawa
	 */

	/*
	 * Drive Relay処理を行う
	 * Version 1.05 2016/12/12 T.Ogawa
	 */
	SetDriveRelayOffTime();

	_HTTP_REPORT;
}

/*
 * 満水監視タイマーハンドラー
 */
void FullWaterTimerHandler(void)
{
	/*
	 *  監視タイマー停止
	 */
	DisableTimerTask(_pOP_FullWaterTimer);
	/*
	 * 給水バルブを閉じる
	 */
	SET_RELAY_WATER(_RELEAY_OFF);

	/*
	 * 念の為循環ポンプを停止しヒーターを止める
	 */
	SET_RELAY_HEATER(_RELEAY_OFF);
	SET_RELAY_PUMP(_RELEAY_OFF);
	/*
	 * エラーステータスを設定
	 */
	/*
	 * 満水エラー
	 */
	/*
	 * Version1.06F 2017/06/12 エラー間違い！
	 */
//	CNTL_DataSet[_CNTL_ERROR_STATE] |= _ERRORCODE_WATERLEVEL_SENSOR;
	CNTL_DataSet[_CNTL_ERROR_STATE] |= _ERRORCODE_WATER_EMPTY;
	SET_OPSTATE_ERROR;

	/*
	 *  HTTP 報告開始追加
	 *
	 *  Version ZVersion製品版1.04 2016/09/19 T.Ogawa
	 */
	_HTTP_REPORT;
}
/*
 * 運転開始時の抜水タイマー開始
 */
void StartDrainageOnStart(void)
{
	/*
	 * 停止直後かもしれないから
	 * 念の為運転停止時の抜水タイマーを停止しておく
	 */
	StopDrainageOnStop();
	/*
	 * 抜水バルブを閉じる（念の為）
	 */
	SET_RELAY_DRAINAGE(_RELEAY_OFF);

	/*
	 * 時間設定が０だったら閉じて帰る
	 */
	if (CNTL_DataSet[_CNTL_MIZUNUKI_TIME_STA] > 0) {
		/*
		 * 抜水を開始する
		 */
		/*
		 * Version 1.05 2016/121/13 SetTimerTimeに変更
		 */
		//_pOP_DrainageOnStartTimer->preTime = (CNTL_DataSet[_CNTL_MIZUNUKI_TIME_STA]);
		SetTimerTime(_pOP_DrainageOnStartTimer, CNTL_DataSet[_CNTL_MIZUNUKI_TIME_STA]);
		/*
		 * 抜水タイマースタート
		 */
		EnableTimerTask(_pOP_DrainageOnStartTimer);

		/*
		 * 抜水バルブを開く
		 */
		SET_RELAY_DRAINAGE(_RELEAY_ON);
	}

	/*
	 * 給水開始
	 */
	StartWaterSupply();

	/*
	 * 給水中ステータス
	 */
	SET_OPSTATE_SUPPLY;

	/*
	 *  HTTP 報告開始追加
	 *
	 *  Version ZVersion製品版1.04 2016/09/19 T.Ogawa
	 */
	_HTTP_REPORT;
}

/*
 * 運転開始時の抜水停止
 */
void StopDrainageOnStart(void)
{
	DisableTimerTask(_pOP_DrainageOnStartTimer);

	/*
	 * 抜水バルブを閉じる
	 */
	SET_RELAY_DRAINAGE(_RELEAY_OFF);

	/*
	 *  HTTP 報告開始追加
	 *
	 *  Version ZVersion製品版1.04 2016/09/19 T.Ogawa
	 */
	_HTTP_REPORT;
}

/*
 * 運転停止時の抜水タイマー開始
 *
 * Version 1.05 2015/12/12 T,Ogawa
 *
 * 戻り値をBoolに変更
 *
 * true  : 排水処理あり
 * false : 排水処理無し
 */
bool StartDrainageOnStop(void)
{
	/*
	 * スタート直後かもしれないから
	 * 念の為運転開始時の抜水タイマーを停止しておく
	 */
	StopDrainageOnStart();

	/*
	 * 時間設定が０だったら閉じて帰る
	 */
	if (CNTL_DataSet[_CNTL_MIZUNUKI_TIME_STP] == 0) {
		/*
		 * Drive Relay処理を行う
		 * Version 1.05 2016/12/12 T.Ogawa
		 */
		SetDriveRelayOffTime();
		return false;
	}

	/*
	 * 抜水を開始する
	 */
	/*
	 * Version 1.05 2016/121/13 SetTimerTimeに変更
	 */
	//_pOP_DrainageOnStopTimer->preTime = (CNTL_DataSet[_CNTL_MIZUNUKI_TIME_STP] * 60);
	SetTimerTime(_pOP_DrainageOnStopTimer, CNTL_DataSet[_CNTL_MIZUNUKI_TIME_STP] * 60);

	/*
	 * 抜水タイマースタート
	 */
	EnableTimerTask(_pOP_DrainageOnStopTimer);

	/*
	 *  ドライブON
	 */
	SET_RELAY_DRIVE(_RELEAY_ON);

	/*
	 * 抜水バルブを開く
	 */
	SET_RELAY_DRAINAGE(_RELEAY_ON);

	/*
	 * ステータスは変更しない
	 * _OPE_STATUS_IDLE
	 */

	/*
	 *  HTTP 報告開始追加
	 *
	 *  Version ZVersion製品版1.04 2016/09/19 T.Ogawa
	 */
	_HTTP_REPORT;

	return true;
}

/*
 * 運転停止時の抜水停止
 */
void StopDrainageOnStop(void)
{
	/*
	 * タイマーを停止する
	 */
	DisableTimerTask(_pOP_DrainageOnStopTimer);

	/*
	 * 抜水バルブを閉じる
	 */
	SET_RELAY_DRAINAGE(_RELEAY_OFF);

	/*
	 *  HTTP 報告開始追加
	 *
	 *  Version ZVersion製品版1.04 2016/09/19 T.Ogawa
	 */
	_HTTP_REPORT;
}

/*
 * 給水開始
 */
void StartWaterSupply(void)
{
	/*
	 *  念の為監視タイマー停止
	 */
	DisableTimerTask(_pOP_FullWaterTimer);
	/*
	 * 満水監視時間を設定
	 */
	/*
	 * Version 1.05 2016/121/13 SetTimerTimeに変更
	 */
	//_pOP_FullWaterTimer->preTime	= (CNTL_DataSet[_CNTL_MANSUI_TIME] * 360);
	SetTimerTime(_pOP_FullWaterTimer, CNTL_DataSet[_CNTL_MANSUI_TIME] * 360);
	/*
	 * 給水監視タイマースタート
	 */
	EnableTimerTask(_pOP_FullWaterTimer);

	/*
	 * 給水監視開始
	 *
	 * Version 1.05 2016/12/13 T.Ogawa
	 */
	SetSupplyMonitor();

	/*
	 * 給水バルブを開く
	 */
	SET_RELAY_WATER(_RELEAY_ON);

	/*
	 *  HTTP 報告開始追加
	 *
	 *  Version ZVersion製品版1.04 2016/09/19 T.Ogawa
	 */
	_HTTP_REPORT;
}

/*
 * 給水終了
 */
void StopWaterSupply(void)
{
	/*
	 *  監視タイマー停止
	 */
	DisableTimerTask(_pOP_FullWaterTimer);
	/*
	 * 給水バルブを閉じる
	 */
	SET_RELAY_WATER(_RELEAY_OFF);

	/*
	 * 給水監視停止
	 *
	 * Version 1.05 2016/12/13 T.Ogawa
	 */
	ResetSupplyMonitor();


	/*
	 *  HTTP 報告開始追加
	 *
	 *  Version ZVersion製品版1.04 2016/09/19 T.Ogawa
	 */
	_HTTP_REPORT;
}

void TemperatureCheck(void)
{
	uint16_t Ondo		= ((CNTL_DataSet[_CNTL_ONDO_H] << 8) | CNTL_DataSet[_CNTL_ONDO_L]);

	/*
	 * 一旦クリア
	 */
	CNTL_DataSet[_CNTL_ERROR_STATE] &= ~(_ERRORCODE_TEMPERATURE | _ERRORCODE_TEMPERATURE_SENSOR);

	if (Ondo >= ERROR_TEMPERATURE) {
		/*
		 * 45℃を超えたらエラー
		 */
		/*
		 * Modify Jun,27th 2016
		 * 停止中にエラーになるのを防ぐ
		 */
		/*
		 * Version1.06B
		 * 最初しばらくは温度評価をしない
		 */
		if (!IS_TEMPERATURE_NOT_RATED) {
			/*
			 * Version1.06L 18/02/21
			 * 運転開始からではなく循環運転開始からしばらくは見ない
			 * 運転開始しても循環運転でなければ何もしない
			 */
//			if (CNTL_DataSet[_CNTL_RUN] == 0x01) {
			if (_OperationStatus == _OPE_STATUS_RUNSUPPLY || _OperationStatus == _OPE_STATUS_RUNNING) {
				CNTL_DataSet[_CNTL_ERROR_STATE] |= _ERRORCODE_TEMPERATURE;
			}
		}
	}
	else if (Ondo == 0) {
		/*
		 * 0℃は温度センサーエラー
		 */
		/*
		 * Version1.06B
		 * 最初しばらくは温度評価をしない
		 */
		if (!IS_TEMPERATURE_NOT_RATED) {
			/*
			 * Version1.06L 18/02/21
			 * 運転開始からではなく循環運転開始からしばらくは見ない
			 * 運転開始しても循環運転でなければ何もしない
			 */
//			if (CNTL_DataSet[_CNTL_RUN] == 0x01) {
			if (_OperationStatus == _OPE_STATUS_RUNSUPPLY || _OperationStatus == _OPE_STATUS_RUNNING) {
				CNTL_DataSet[_CNTL_ERROR_STATE] |= _ERRORCODE_TEMPERATURE_SENSOR;
			}
		}
	}
}

void TemperatureControl(void)
{
	uint16_t Ondo		= ((CNTL_DataSet[_CNTL_ONDO_H] << 8) | CNTL_DataSet[_CNTL_ONDO_L]);
	uint16_t PreOndo	= ((CNTL_DataSet[_CNTL_ONDO_SETTEI_H] << 8) | CNTL_DataSet[_CNTL_ONDO_SETTEI_L]);
	uint16_t PreOndo_L	= PreOndo - (20 - (2 * CNTL_DataSet[_CNTL_ONCHO_LOWER]));
	uint16_t PreOndo_H	= PreOndo + (2 * CNTL_DataSet[_CNTL_ONCHO_UPPER]);

	if (Ondo == 0 || Ondo >= ERROR_TEMPERATURE) {
		/*
		 * もしセンサーエラーや温度異常の場合はポンプヒーターをオフする
		 */
		if (IS_TEMPERATURE_NOT_RATED) {
			SET_RELAY_HEATER(_RELEAY_OFF);
		}
		else {
			SET_RELAY_PUMP(_RELEAY_OFF);
			SET_RELAY_HEATER(_RELEAY_OFF);
			return;
		}
	}

	/*
	 * ポンプON
	 */
	SET_RELAY_PUMP(_RELEAY_ON);

	/*
	 * Version1.06K 2017/09/11 T.Ogawa
	 * 水を足して冷却処理
	 * Option1を設定されている場合
	 * Drain Releayを利用する
	 */
	if (IsCoolingWaterMode()) {
		// Optionが設定されている
		if (Ondo > (PreOndo_H + CoolingWaterOffsetTemperature())) {
			/*
			 * 設定温度範囲の上限を超えたら水を出す
			 */
			SET_RELAY_DRAINAGE(_RELEAY_ON);
		}
		if (Ondo <= (PreOndo + CoolingWaterOffsetTemperature())) {
			/*
			 * 設定温度以下になったら水を止める
			 */
			SET_RELAY_DRAINAGE(_RELEAY_OFF);
		}
	}

	/*
	 * Modify Jun,27th 2016
	 * センサーエラー時は安全のためヒーターをOFFする
	 * ポンプを切らないのはもしかしたら復活するかもなので
	 */
	if (CNTL_DataSet[_CNTL_ERROR_STATE] & _ERRORCODE_TEMPERATURE_SENSOR) {
		/*
		 * センサーエラーだったらヒーターOFF
		 */
		SET_RELAY_HEATER(_RELEAY_OFF);
		/*
		 *  RETURN
		 */
		return;
	}

	/*
	 * 標準運転時以外はヒーターはOFF
	 */
	if (CNTL_DataSet[_CNTL_RUNMODE] == _CNTL_RUNMODE_NORMAL) {
		/*
		 * 温度測定
		 */
		if (Ondo >= PreOndo_H) {
			/*
			 * 上限設定値以上なら停止
			 */
			SET_RELAY_HEATER(_RELEAY_OFF);
		}
		else if (Ondo <= PreOndo_L) {
			/*
			 * 上限設定値以下ならヒーターON
			 */
			SET_RELAY_HEATER(_RELEAY_ON);
		}
	}
	else {
		/*
		 * 標準運転時以外はヒーターOFF
		 */
		SET_RELAY_HEATER(_RELEAY_OFF);
	}
}

/*
 * シャットダウン
 *
 * Version 1.05 2016/12/12 T.Ogawa
 *
 * Drive Relayの状態を指定できるように変更
 */
void Shutdown(bool bDriveRelay)
{
	// タイマー関係無効
	DisableTimerTask(_pOP_DrainageOnStartTimer);
	DisableTimerTask(_pOP_FullWaterTimer);

	/*
	 * 給水監視停止
	 *
	 * Version 1.05 2016/12/13 T.Ogawa
	 */
	ResetSupplyMonitor();

	SET_RELAY_HEATER(_RELEAY_OFF);
	SET_RELAY_WATER(_RELEAY_OFF);
	SET_RELAY_PUMP(_RELEAY_OFF);
	SET_RELAY_DRAINAGE(_RELEAY_OFF);

	/*
	 * Drive Relay
	 */
	SET_RELAY_DRIVE(bDriveRelay);

	/*
	 * 渇水エラークリア
	 */
	CNTL_DataSet[_CNTL_ERROR_STATE] &= ~_ERRORCODE_WATER_EMPTY;
	/*
	 * 満水エラークリア
	 */
	CNTL_DataSet[_CNTL_ERROR_STATE] &= ~_ERRORCODE_WATERLEVEL_SENSOR;
}

/*
 * エラー処理オペレーション
 */
void Ope_Error(void)
{
	SET_RELAY_HEATER(_RELEAY_OFF);
	SET_RELAY_WATER(_RELEAY_OFF);
	SET_RELAY_PUMP(_RELEAY_OFF);
	SET_RELAY_DRAINAGE(_RELEAY_OFF);

	SET_RELAY_ERROR(_RELEAY_ON);


	/*
	 * 渇水エラー復帰チェック
	 */
	if ((CNTL_DataSet[_CNTL_ERROR_STATE] & _ERRORCODE_WATER_EMPTY) == _ERRORCODE_WATER_EMPTY) {
		/*
		 * 現在渇水エラー中だったら復帰確認
		 */
		/*
		 * 水位取得
		 */
		uint8_t Suii = CNTL_DataSet[_CNTL_SUII_LEVEL];
		if (Suii != _WATERLEVEL_EE) {
			CNTL_DataSet[_CNTL_ERROR_STATE] &= ~_ERRORCODE_WATER_EMPTY;
		}
		else {
			CNTL_DataSet[_CNTL_ERROR_STATE] |= _ERRORCODE_WATER_EMPTY;
		}

		/*
		 * 他のエラーがなければ復帰
		 */
		if ((CNTL_DataSet[_CNTL_ERROR_STATE] & ~_ERRORCODE_COMMUNIACTIN) == _ERRORCODE_NOTHING) {
			_OperationStatus = _OPE_STATUS_RUNNING;
			SET_RELAY_ERROR(_RELEAY_OFF);
		}
	}

}


/*
 * 運転中・停止中ステート
 */
static uint8_t bInOperation = false;
/*
 * 運転・停止確認
 */
void Ope_RunCheck(void)
{
	if (bInOperation) {
		/*
		 * 現在運転中（停止監視）
		 */
		if (CNTL_DataSet[_CNTL_RUN] == 0x00) {
			/*
			 * Version1.06J
			 * 2017/08/22
			 * メインのデータROMエリアへの書込みを行わない
			 * またノイズの影響を極力減らす為リレーを動作する前にROM書込みを行う
			 */
			MaintainRunStop(CNTL_DataSet[_CNTL_RUN]);

			//シャットダウン
			Shutdown(_RELEAY_ON);
			// 停止中
			bInOperation = false;
			// 初期ステート
			SET_OPSTATE_IDLE;

			/*
			 * 排水開始
			 */
			StartDrainageOnStop();

			/*
			 * Version1.06J
			 * 2017/08/22
			 * RUN/STOPで全てのパラメータラ書込みをしていたのをやめる
			 */
			//ParameterSuspend();

			/*
			 *  HTTP 報告開始追加
			 *
			 *  Version ZVersion製品版1.04 2016/09/13 T.Ogawa
			 */
			_HTTP_REPORT;
		}
	}
	else {
		/*
		 * 現在停止中（運転開始待ち）
		 */
		if (CNTL_DataSet[_CNTL_RUN] == 0x01) {
			/*
			 * Version1.06J
			 * 2017/08/22
			 * メインのデータROMエリアへの書込みを行わない
			 * またノイズの影響を極力減らす為リレーを動作する前にROM書込みを行う
			 */
			MaintainRunStop(CNTL_DataSet[_CNTL_RUN]);

			// Drive Relay Cancel(念の為)
			DriveRelayCancel();
			// 運転中
			bInOperation = true;
			// STARTステート
			SET_OPSTATE_START;

			/*
			 * Version1.06J
			 * 2017/08/22
			 * RUN/STOPで全てのパラメータラ書込みをしていたのをやめる
			 */
			//ParameterSuspend();
			/*
			 *  HTTP 報告開始追加
			 *
			 *  Version ZVersion製品版1.04 2016/09/13 T.Ogawa
			 */
			_HTTP_REPORT;
		}
	}
}

/*
 * 運転開始ステート
 */
void Ope_Start(void)
{
	/*
	 * 水位取得
	 */
	uint8_t Suii = CNTL_DataSet[_CNTL_SUII_LEVEL];

	/*
	 * 排水処理を停止しておく
	 */
	StopDrainageOnStop();

	/*
	 * 念の為全部オフっておく
	 */
	SET_RELAY_HEATER(_RELEAY_OFF);
	SET_RELAY_WATER(_RELEAY_OFF);
	SET_RELAY_PUMP(_RELEAY_OFF);
	SET_RELAY_DRAINAGE(_RELEAY_OFF);
	SET_RELAY_ERROR(_RELEAY_OFF);

	// ドライブON
	SET_RELAY_DRIVE(_RELEAY_ON);

	switch(Suii) {
		case _WATERLEVEL_E1:
			/*
			 * 満水->循環動作
			 */
			SET_OPSTATE_RUNNING;
			/*
			 * Version1.06B
			 * 最初しばらくは温度評価をしない
			 */
			StartTemperatureNotRated();
			break;
		case _WATERLEVEL_E2:
		case _WATERLEVEL_E3:
			/*
			 * 満水じゃないけど水が入ってる->給水動作
			 */
			SET_OPSTATE_SUPPLY;
			StartWaterSupply();
			break;
		case _WATERLEVEL_EE:
			/*
			 * 水が無い->抜水〜給水動作
			 */
			SET_OPSTATE_DRAINAGE;
			StartDrainageOnStart();
			break;
		/*
		 * Version1.06F 2017/06/12
		 * センサーエラー追加
		 */
		case _WATERLEVEL_ERR:
			SET_OPSTATE_ERROR;
			CNTL_DataSet[_CNTL_ERROR_STATE] |= _ERRORCODE_WATERLEVEL_SENSOR;
			break;

	}
}

/*
 * 給水中ステート
 */
void Ope_Supply(void)
{
	/*
	 * 洗浄モード時は水位制御は行わない
	 * 給水中に洗浄モードへの切替があった場合の処置
	 */
	if (CNTL_DataSet[_CNTL_RUNMODE] == _CNTL_RUNMODE_NORMAL_WASHING) {
		/*
		 * 給水停止
		 */
		StopWaterSupply();
		/*
		 * 循環運転
		 */
		SET_OPSTATE_RUNNING;
		/*
		 * Version1.06B
		 * 最初しばらくは温度評価をしない
		 */
		StartTemperatureNotRated();
		return;
	}

	/*
	 * 水位取得＆満水チェック
	 */
	if (CNTL_DataSet[_CNTL_SUII_LEVEL] == _WATERLEVEL_E1) {
		/*
		 * 満水
		 */
		StopWaterSupply();
		/*
		 * 循環運転
		 */
		SET_OPSTATE_RUNNING;
		/*
		 * Version1.06B
		 * 最初しばらくは温度評価をしない
		 */
		StartTemperatureNotRated();
	}
	else if (CNTL_DataSet[_CNTL_SUII_LEVEL] == _WATERLEVEL_EE) {
		/*
		 * Version1.06K 2017/09/08 T.Ogawa
		 * 足湯モードの場合はエラーにしない
		 */
		//if (_OperationStatus == _OPE_STATUS_RUNSUPPLY) {
		if (IsFootBathMode() == false && _OperationStatus == _OPE_STATUS_RUNSUPPLY) {
			/*
			 * 水が無い 渇水エラー
			 */
			CNTL_DataSet[_CNTL_ERROR_STATE] |= _ERRORCODE_WATER_EMPTY;
			SET_OPSTATE_ERROR;
		}
	}
	/*
	 * Version1.06F 2017/06/12 センサーエラー追加
	 */
	else if (CNTL_DataSet[_CNTL_SUII_LEVEL] == _WATERLEVEL_ERR) {
		CNTL_DataSet[_CNTL_ERROR_STATE] |= _ERRORCODE_WATERLEVEL_SENSOR;
		SET_OPSTATE_ERROR;
	}

	/*
	 * Version1.06K 2017/09/08
	 * 足湯バルブ制御
	 */
	FootBathControll();

	/*
	 * Modify Jun,27th 2016
	 * 循環運転中の給水は温調する
	 */
	if (_OperationStatus == _OPE_STATUS_RUNSUPPLY) {
		TemperatureControl();
	}
}

/*
 * 運転中ステート
 */
void Ope_Running(void)
{
	/*
	 * 水位取得
	 */
	uint8_t Suii = CNTL_DataSet[_CNTL_SUII_LEVEL];

	/*
	 * 温調開始
	 */
	TemperatureControl();

	/*
	 * 洗浄モード時は水位制御は行わない
	 */
	if (CNTL_DataSet[_CNTL_RUNMODE] == _CNTL_RUNMODE_NORMAL_WASHING) {
		/*
		 *  HTTP 報告開始追加
		 *  ポンプ保護の為、渇水だけはチェックする
		 *
		 *  Version ZVersion製品版1.04 2016/09/19 T.Ogawa
		 */
		if (Suii == _WATERLEVEL_EE) {
			/*
			 * 水が無い 渇水エラー
			 */
			CNTL_DataSet[_CNTL_ERROR_STATE] |= _ERRORCODE_WATER_EMPTY;
			SET_OPSTATE_ERROR;
		}
		return;
	}

	switch(Suii) {
		case _WATERLEVEL_E1:
			/*
			 * 満水->循環動作
			 */
			SET_OPSTATE_RUNNING;
			break;
		case _WATERLEVEL_E2:
			break;
		case _WATERLEVEL_E3:
			/*
			 * 満水じゃないけど水が入ってる->給水動作
			 */
			SET_OPSTATE_RUNSUPPLY;
			StartWaterSupply();
			break;
		case _WATERLEVEL_EE:
			/*
			 * Version1.06K 2017/09/08
			 * 足湯モード処理追加
			 */
			{
				if (IsFootBathMode()) {
					/*
					 * 足湯モードの場合はエラーにしないで給水する
					 */
					SET_OPSTATE_RUNSUPPLY;
					StartWaterSupply();
				}
				else {
					/*
					 * 水が無い 渇水エラー
					 */
					CNTL_DataSet[_CNTL_ERROR_STATE] |= _ERRORCODE_WATER_EMPTY;
					SET_OPSTATE_ERROR;
				}
			}
			break;
	}

	/*
	 * Version1.06K 2017/09/08
	 * 足湯バルブ制御
	 */
	FootBathControll();
}

void Operation(void)
{
	uint32_t sTime, eTime;

	/*
	 * Version1.06L
	 * 強制給水フラグ
	 */
	m_ForceWaterSupply = 0;

	/*
	 * Boot Status
	 */
    BootDataManage();

	/*
	 * WDT開始
	 */
	WDT_Start(WDT_TIME);

    /*
	 * 初期化
	 */
	Init_Operation();

	/*
	 * 初期値をリモコンに通知
	 */
	InitialParameterSend();

	/*
	 * センサー開始
	 */
	StartMeasurement();

	/*
	 * 念の為シャットダウン
	 */
	Shutdown(_RELEAY_OFF);

    /*
     * W5500 初期化
     */
    Init_NetDevice();

	/*
	 * LAN Listen 初期化
	 */
	PNET_LISTEN_DATA opeListenData = Init_Listen();
	/*
	 * HTTP_Request 初期化
	 */
	Init_HTTP_Request();

	/*
	 * コンタクタセンス初期化
	 */
	Init_Contactor();

	/*
	 * パワーLED初期化
	 */
	Init_PowerLED();

    while(1) {
    	/*
    	 * Scan Start
    	 */
    	sTime = _10mSecTickTime;

    	/*
    	 * LAN Listen
    	 */
    	HTTP_Listen(opeListenData);

    	/*
    	 * サーバーへ定期報告
    	 */
    	HTTP_RequestFunc();

    	/*
    	 * タイマーチェック
    	 */
    	OnTimerCheck();

    	/*
    	 * データマネージャ起動
    	 * リモコン等への通信を行う
    	 */
		dManage();

		/*
		 * 温度チェック
		 */
		TemperatureCheck();

    	/*
    	 * コンタクタチェック
    	 */
    	if (Check_Contactor()) {
    		/*------------------------------------------------------
    		 * オペレーションの条件判断
		 	 ------------------------------------------------------*/
    		/*
    		 * まず起動チェック
    		 */
    		Ope_RunCheck();

    		switch(_OperationStatus) {
    			case _OPE_STATUS_START:
    				// スタートオペレーション
    				Ope_Start();
    				break;
    			case _OPE_STATUS_DRAINAGE:
    				break;
    			case _OPE_STATUS_SUPPLY:
    			case _OPE_STATUS_RUNSUPPLY:
    				// 給水オペレーション
    				Ope_Supply();
    				break;
    			case _OPE_STATUS_RUNNING:
    				// 循環運転オペレーション
    				Ope_Running();
    				break;
    			case _OPE_STATUS_ERROR:
    				Ope_Error();
    				break;

    		}
    	}
    	else {
    		/*
    		 * Version1.06F 2017/06/12
    		 */
    		CNTL_DataSet[_CNTL_RUN] = 0x00;
    		Ope_RunCheck();
    	}


    	/*
    	 * Version1.06L
    	 * 強制給水フラグ
    	 * このフラグは循環運転中のみ有効となる。
    	 */
    	if (_OperationStatus == _OPE_STATUS_RUNNING){
    		if (m_ForceWaterSupply) {
    			/*
    			 * 強制給水開始
    			 */
    			SET_RELAY_WATER(_RELEAY_ON);
    		}
    		else {
    			/*
    			 * 強制給水停止
    			 */
    			SET_RELAY_WATER(_RELEAY_OFF);
    		}
    	}
    	else {
    		/*
    		 * 循環運転中じゃなかったら強制給水フラグはクリアする
    		 */
    		m_ForceWaterSupply = 0;
    	}
    	//--------------------------------------------------------- Version1.06L


    	if ((CNTL_DataSet[_CNTL_ERROR_STATE] & ~_ERRORCODE_COMMUNIACTIN) != _ERRORCODE_NOTHING) {
     		SET_RELAY_ERROR(_RELEAY_ON);
    	}
    	else {
     		SET_RELAY_ERROR(_RELEAY_OFF);
    	}

    	/*
    	 * End Scan Time
    	 */
    	eTime = _10mSecTickTime;
    	if (_10mSecTickTime > sTime) {
    		CurrentScanTime = eTime - sTime;
    		if (MaxScanTime < CurrentScanTime) {
    			MaxScanTime = CurrentScanTime;
    		}
    	}
    	/*
    	 * WDTをキック
    	 */
    	WDT_Kick();
    }
}
