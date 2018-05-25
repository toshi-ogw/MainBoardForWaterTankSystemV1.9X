/*
 * electrode.c
 *
 *  Created on: 2015/12/14
 *      Author: toshio
 */

#include "electrode.h"
#include "controldata.h"
#include "tick.h"
#include "timer32.h"

/*
 * 電極 極性切替及び測定時間(msec)tickタイマー（10mSec単位）を使用する
 */
PTICKTIMER_BLOCK pElectrodeTimer;

/*
 * ディレイタイマー
 */
PTIMER_BLOCK pElectrodeDelayTimer;

#define	ElectrobeCycle	1

#define	_WATERSENS_PORT		1
#define	_WATERSENS_L_BIT	0
#define	_WATERSENS_M_BIT	1
#define	_WATERSENS_H_BIT	2

#define	_WATERDRIVE_PORT	3
#define	_WATERDRIVE0_BIT	4
#define	_WATERDRIVE1_BIT	5

WATER_SENS_VALUE	_bWaterSens_L;
WATER_SENS_VALUE	_bWaterSens_M;
WATER_SENS_VALUE	_bWaterSens_H;


static void Init_Electrode_PinMux(void)
{
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO3_4, (IOCON_FUNC0 | IOCON_MODE_INACT));/* Water Sens Drive0 */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO3_5, (IOCON_FUNC0 | IOCON_MODE_INACT));/* Water Sens Drive1 */


	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_0, (IOCON_FUNC1 | IOCON_DIGMODE_EN));/* Water Sens L */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_1, (IOCON_FUNC1 | IOCON_DIGMODE_EN));/* Water Sens M */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_2, (IOCON_FUNC1 | IOCON_DIGMODE_EN));/* Water Sens H */
}


/*
 * 水位電極測定ハンドラ
 */
int _ElectrodeState = 0;

void ElectrodeSensingProbe(PWATER_SENS_VALUE pProbe, bool bValue) {

	if (pProbe->bPrevSens == bValue) {
		if (pProbe->Time == 0) {
			// 一定時間経過しても値が変化しなかったので
			// 現在の値をセンサー値とする
			pProbe->bSens = bValue;
		}
	}
	else {
		// センサー値がパタパタしてたらタイマークリア
		// 減算は1分単位で別タイマで行う！
		pProbe->Time = pProbe->preTimer;
	}
	// 更新
	pProbe->bPrevSens = bValue;
}

void ElectrodeSensingHandler(void)
{
	/*
	 * 一旦OFF
	 */
	switch(_ElectrodeState) {
		case 0:
			Chip_GPIO_SetPinState(LPC_GPIO, _WATERDRIVE_PORT, _WATERDRIVE0_BIT, true);
			Chip_GPIO_SetPinState(LPC_GPIO, _WATERDRIVE_PORT, _WATERDRIVE1_BIT, false);
			_ElectrodeState++;
			break;
		case 1:
			// ディレイタイマ設定
			_bWaterSens_L.preTimer = CNTL_DataSet[_CNTL_SUII_L_DELAY];
			_bWaterSens_M.preTimer = CNTL_DataSet[_CNTL_SUII_M_DELAY];
			_bWaterSens_H.preTimer = CNTL_DataSet[_CNTL_SUII_H_DELAY];
			// プローブ
			ElectrodeSensingProbe(&_bWaterSens_L, ~Chip_GPIO_GetPinState(LPC_GPIO, _WATERSENS_PORT, _WATERSENS_L_BIT) & 0x01);
			ElectrodeSensingProbe(&_bWaterSens_M, ~Chip_GPIO_GetPinState(LPC_GPIO, _WATERSENS_PORT, _WATERSENS_M_BIT) & 0x01);
			ElectrodeSensingProbe(&_bWaterSens_H, ~Chip_GPIO_GetPinState(LPC_GPIO, _WATERSENS_PORT, _WATERSENS_H_BIT) & 0x01);
			_ElectrodeState++;
			break;
		case 2:
			Chip_GPIO_SetPinState(LPC_GPIO, _WATERDRIVE_PORT, _WATERDRIVE0_BIT, false);
			Chip_GPIO_SetPinState(LPC_GPIO, _WATERDRIVE_PORT, _WATERDRIVE1_BIT, true);
			_ElectrodeState++;
			break;
		default:
			_ElectrodeState = 0;
			break;
	}
}

void ElectrodeSensingDelayHandler(void)
{
	if (_bWaterSens_L.Time > 0) {
		_bWaterSens_L.Time--;
	}
	if (_bWaterSens_M.Time > 0) {
		_bWaterSens_M.Time--;
	}
	if (_bWaterSens_H.Time > 0) {
		_bWaterSens_H.Time--;
	}
}

void ElectrodeDebug(void)
{
	//uint8_t	Sens_L;
	//uint8_t	Sens_M;
	//uint8_t	Sens_H;

	Init_Electrode_PinMux();
	/*
	 * 出力モード
	 */
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, _WATERDRIVE_PORT, _WATERDRIVE0_BIT);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, _WATERDRIVE_PORT, _WATERDRIVE1_BIT);
	/*
	 * 入力モード
	 */
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, _WATERSENS_PORT, _WATERSENS_L_BIT);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, _WATERSENS_PORT, _WATERSENS_M_BIT);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, _WATERSENS_PORT, _WATERSENS_H_BIT);

	/*
	 * オールクリア
	 */
	Chip_GPIO_SetPinState(LPC_GPIO, _WATERDRIVE_PORT, _WATERDRIVE0_BIT, false);
	Chip_GPIO_SetPinState(LPC_GPIO, _WATERDRIVE_PORT, _WATERDRIVE1_BIT, false);


	while(1) {
		// ドライブを一旦オフ
		Chip_GPIO_SetPinState(LPC_GPIO, _WATERDRIVE_PORT, _WATERDRIVE0_BIT, false);
		Chip_GPIO_SetPinState(LPC_GPIO, _WATERDRIVE_PORT, _WATERDRIVE1_BIT, false);

		// センスドライブをオン
		Chip_GPIO_SetPinState(LPC_GPIO, _WATERDRIVE_PORT, _WATERDRIVE0_BIT, true);

		//Sens_L = (~Chip_GPIO_GetPinState(LPC_GPIO, _WATERSENS_PORT, _WATERSENS_L_BIT) & 0x01);
		//Sens_M = (~Chip_GPIO_GetPinState(LPC_GPIO, _WATERSENS_PORT, _WATERSENS_M_BIT) & 0x01);
		//Sens_H = (~Chip_GPIO_GetPinState(LPC_GPIO, _WATERSENS_PORT, _WATERSENS_H_BIT) & 0x01);

		// ドライブを一旦オフ
		Chip_GPIO_SetPinState(LPC_GPIO, _WATERDRIVE_PORT, _WATERDRIVE0_BIT, false);
		Chip_GPIO_SetPinState(LPC_GPIO, _WATERDRIVE_PORT, _WATERDRIVE1_BIT, false);

		// 非センスドライブをオン
		Chip_GPIO_SetPinState(LPC_GPIO, _WATERDRIVE_PORT, _WATERDRIVE1_BIT, true);
	}
}

void ElectrodeInitialSens(void)
{
	int i;

	// ドライブを一旦オフ
	Chip_GPIO_SetPinState(LPC_GPIO, _WATERDRIVE_PORT, _WATERDRIVE0_BIT, false);
	Chip_GPIO_SetPinState(LPC_GPIO, _WATERDRIVE_PORT, _WATERDRIVE1_BIT, false);

	/*
	 * センサー値初期化
	 */
	_bWaterSens_L.Time		= 0;
	_bWaterSens_L.bPrevSens	= false;
	_bWaterSens_L.bSens		= false;
	_bWaterSens_M.Time		= 0;
	_bWaterSens_M.bPrevSens	= false;
	_bWaterSens_M.bSens		= false;
	_bWaterSens_H.Time		= 0;
	_bWaterSens_H.bPrevSens	= false;
	_bWaterSens_H.bSens		= false;

	// センスドライブをオン
	Chip_GPIO_SetPinState(LPC_GPIO, _WATERDRIVE_PORT, _WATERDRIVE0_BIT, true);

	// ディレイタイマ設定
	_bWaterSens_L.preTimer 	= CNTL_DataSet[_CNTL_SUII_L_DELAY];
	_bWaterSens_M.preTimer 	= CNTL_DataSet[_CNTL_SUII_M_DELAY];
	_bWaterSens_H.preTimer 	= CNTL_DataSet[_CNTL_SUII_H_DELAY];

	for (i = 0; i < 10; i++) {
		ElectrodeSensingProbe(&_bWaterSens_L, ~Chip_GPIO_GetPinState(LPC_GPIO, _WATERSENS_PORT, _WATERSENS_L_BIT) & 0x01);
		ElectrodeSensingProbe(&_bWaterSens_M, ~Chip_GPIO_GetPinState(LPC_GPIO, _WATERSENS_PORT, _WATERSENS_M_BIT) & 0x01);
		ElectrodeSensingProbe(&_bWaterSens_H, ~Chip_GPIO_GetPinState(LPC_GPIO, _WATERSENS_PORT, _WATERSENS_H_BIT) & 0x01);
		_bWaterSens_L.Time		= 0;
		_bWaterSens_M.Time		= 0;
		_bWaterSens_H.Time		= 0;
		Wait(1);
	}

	// ドライブを一旦オフ
	Chip_GPIO_SetPinState(LPC_GPIO, _WATERDRIVE_PORT, _WATERDRIVE0_BIT, false);
	Chip_GPIO_SetPinState(LPC_GPIO, _WATERDRIVE_PORT, _WATERDRIVE1_BIT, false);

	// 非センスドライブを同じ時間だけオン（念の為）
	Chip_GPIO_SetPinState(LPC_GPIO, _WATERDRIVE_PORT, _WATERDRIVE1_BIT, true);
	Wait(10);

	// ドライブを一旦オフ
	Chip_GPIO_SetPinState(LPC_GPIO, _WATERDRIVE_PORT, _WATERDRIVE0_BIT, false);
	Chip_GPIO_SetPinState(LPC_GPIO, _WATERDRIVE_PORT, _WATERDRIVE1_BIT, false);

	/*
	 * ステートをクリア
	 */
	_ElectrodeState = 0;
}

void Init_Electrode(void)
{
	Init_Electrode_PinMux();
	/*
	 * 出力モード
	 */
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, _WATERDRIVE_PORT, _WATERDRIVE0_BIT);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, _WATERDRIVE_PORT, _WATERDRIVE1_BIT);
	/*
	 * 入力モード
	 */
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, _WATERSENS_PORT, _WATERSENS_L_BIT);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, _WATERSENS_PORT, _WATERSENS_M_BIT);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, _WATERSENS_PORT, _WATERSENS_H_BIT);

	/*
	 * オールクリア
	 */
	Chip_GPIO_SetPinState(LPC_GPIO, _WATERDRIVE_PORT, _WATERDRIVE0_BIT, false);
	Chip_GPIO_SetPinState(LPC_GPIO, _WATERDRIVE_PORT, _WATERDRIVE1_BIT, false);


	/*
	 * センサー初期値取得
	 */
	ElectrodeInitialSens();
	/*
	 * ステートをクリア
	 */
	_ElectrodeState = 0;
	/*
	 * センサーディレイタイマ
	 */
	pElectrodeDelayTimer = AllocTimer1Task(5,ElectrodeSensingDelayHandler);
	/*
	 * タイマー設定
	 */
	pElectrodeTimer = AllocTickTask(ElectrobeCycle, ElectrodeSensingHandler);

	/*
	 * ディレイ開始
	 */
	EnableTimerTask(pElectrodeDelayTimer);

	/*
	 * タイマースタート
	 */
	EnableTickTask(pElectrodeTimer);
}
void ElectrodeStaticSens(void)
{
	/*
	 * タイマー停止
	 */
	DisableTickTask(pElectrodeTimer);

	/*
	 * センサー初期値取得
	 */
	ElectrodeInitialSens();

	/*
	 * タイマースタート
	 */
	EnableTickTask(pElectrodeTimer);

}
