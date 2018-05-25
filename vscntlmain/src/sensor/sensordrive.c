/*
 * sensordrive.c
 *
 *  Created on: 2015/12/14
 *      Author: toshio
 */

#include "electrode.h"
#include "sensordrive.h"
#include "controldata.h"
#include "timer32.h"
#include "i2c.h"
#include "temperature.h"
#include "pressure.h"


#define	WATERLEVEL_MAX	100
#define	WATERLEVEL_MIN	2


/*
 * 温度更新タイマー
 */
PTIMER_BLOCK pTemperatureSensTimer;

/*
 * 水位センサータイマー
 */
PTIMER_BLOCK pWaterLevelSensTimer;

/*
 * 温度更新フラグ
 */
bool bTemperatureSensUpdate = false;

/*
 * 水位更新フラグ
 */
bool bSuiiSensUpdate		= false;

/*
 * 水圧センサー感度タイマー
 */
PTIMER_BLOCK pPressureSensitivityTimer;

/*
 * 水圧センサー感度
 */
typedef struct {
	uint8_t	Level;			// 設定値
	uint8_t	Timer;			// タイマー値
	uint8_t	PresetTimer;	// タイマー設定値
} PRESSURE_LEVEL_DATA, *PPRESSURE_LEVEL_DATA;

typedef struct {
	uint8_t				CurrentLevel;
	uint8_t				PrevCmeter;
	PRESSURE_LEVEL_DATA	E;
	PRESSURE_LEVEL_DATA	L;
	PRESSURE_LEVEL_DATA	M;
	PRESSURE_LEVEL_DATA	H;
} PRESSURE_LEVEL, *PPRESSURE_LEVEL;

PRESSURE_LEVEL Puressure_Level;


/*
 * 温度センサー測定ハンドラ
 */
void TemperatureSensingHandler(void)
{
	bTemperatureSensUpdate = true;
    DisableTimerTask(pTemperatureSensTimer);
}

/*
 * 水位センサー測定ハンドラ
 */
void WaterLevelSensingHandler(void)
{
	bSuiiSensUpdate = true;
    DisableTimerTask(pWaterLevelSensTimer);
}

/*
 * 水圧センサー測定感度タイマーハンドラ(１秒周期)
 */
void PressureSensitivityHandler(void)
{
	if (Puressure_Level.E.Timer > 0) {
		Puressure_Level.E.Timer--;
	}
	if (Puressure_Level.L.Timer > 0) {
		Puressure_Level.L.Timer--;
	}
	if (Puressure_Level.M.Timer > 0) {
		Puressure_Level.M.Timer--;
	}
	if (Puressure_Level.H.Timer > 0) {
		Puressure_Level.H.Timer--;
	}
}

/*
 * 温度測定
 */
bool MeasurementTemperature(void)
{
	bool bRet = false;

	if (bTemperatureSensUpdate) {
		uint16_t temp = (uint16_t)(Read_Temperature() * 10) ;

		/*
		 * 補正値を設定する
		 */
		temp += (int)((int)(CNTL_DataSet[_CNTL_ONCHO_HOSEI]) - 10);

		CNTL_DataSet[_CNTL_ONDO_L] = (temp & 0xff);
		CNTL_DataSet[_CNTL_ONDO_H] = ((temp >> 8) & 0xff);


		/*
		 * 更新フラグクリア
		 */
		bTemperatureSensUpdate = false;

	    bRet = true;

		/*
		 * 周期を再設定（変更が反映）
		 */
		pTemperatureSensTimer->preTime = CNTL_DataSet[_CNTL_ONCHO_CYCLE];
	    EnableTimerTask(pTemperatureSensTimer);
	}

	return bRet;
}

/*
 * 水圧センサー測定初期化
 */
void InitPressureMeasurement(void)
{
	double 	pres;
	uint8_t Cmeter;

	Puressure_Level.L.Level			= CNTL_DataSet[_CNTL_SUII_L_SETTEI];
	Puressure_Level.L.PresetTimer	= CNTL_DataSet[_CNTL_SUII_L_DELAY];
	Puressure_Level.L.Timer			= CNTL_DataSet[_CNTL_SUII_L_DELAY];

	Puressure_Level.E.Level			= CNTL_DataSet[_CNTL_SUII_L_SETTEI];
	Puressure_Level.E.PresetTimer	= CNTL_DataSet[_CNTL_SUII_L_DELAY];
	Puressure_Level.E.Timer			= CNTL_DataSet[_CNTL_SUII_L_DELAY];

	Puressure_Level.M.Level			= (CNTL_DataSet[_CNTL_SUII_H_SETTEI] - CNTL_DataSet[_CNTL_SUII_M_SETTEI]);
	Puressure_Level.M.PresetTimer	= CNTL_DataSet[_CNTL_SUII_M_DELAY];
	Puressure_Level.M.Timer			= CNTL_DataSet[_CNTL_SUII_M_DELAY];

	Puressure_Level.H.Level			= CNTL_DataSet[_CNTL_SUII_H_SETTEI];
	Puressure_Level.H.PresetTimer	= CNTL_DataSet[_CNTL_SUII_H_DELAY];
	Puressure_Level.H.Timer			= CNTL_DataSet[_CNTL_SUII_H_DELAY];

	pres = Read_Pressure();
	Cmeter = (uint8_t)PressureToWaterlevel(pres);

	if (Cmeter >= Puressure_Level.H.Level) {
		// 満水
		Puressure_Level.CurrentLevel = _WATERLEVEL_E1;
	}
	else if (Cmeter >= Puressure_Level.M.Level) {
		// 中水
		Puressure_Level.CurrentLevel = _WATERLEVEL_E2;
	}
	else if (Cmeter < Puressure_Level.M.Level && Cmeter >= Puressure_Level.L.Level) {
		// 低水
		Puressure_Level.CurrentLevel = _WATERLEVEL_E3;
	}
	else if (Cmeter < Puressure_Level.E.Level) {
		// 渇水
		Puressure_Level.CurrentLevel = _WATERLEVEL_EE;
	}
	Puressure_Level.PrevCmeter = Cmeter;

}

/*
 * 水圧を水位に変換する
 */
uint8_t ComvPressureToLevel(void)
{
	double 	pres;
	uint8_t Cmeter;

	// 再設定
	if (	Puressure_Level.L.Level			!= CNTL_DataSet[_CNTL_SUII_L_SETTEI]
		||	Puressure_Level.L.PresetTimer	!= CNTL_DataSet[_CNTL_SUII_L_DELAY]) {
		Puressure_Level.L.Level			= CNTL_DataSet[_CNTL_SUII_L_SETTEI];
		Puressure_Level.L.PresetTimer	= CNTL_DataSet[_CNTL_SUII_L_DELAY];
		Puressure_Level.L.Timer			= CNTL_DataSet[_CNTL_SUII_L_DELAY];
		Puressure_Level.E.Level			= CNTL_DataSet[_CNTL_SUII_L_SETTEI];
		Puressure_Level.E.PresetTimer	= CNTL_DataSet[_CNTL_SUII_L_DELAY];
		Puressure_Level.E.Timer			= CNTL_DataSet[_CNTL_SUII_L_DELAY];
	}
	if (	Puressure_Level.M.Level			!= (CNTL_DataSet[_CNTL_SUII_H_SETTEI] - CNTL_DataSet[_CNTL_SUII_M_SETTEI])
		||	Puressure_Level.M.PresetTimer	!= CNTL_DataSet[_CNTL_SUII_M_DELAY]) {
		Puressure_Level.M.Level			= (CNTL_DataSet[_CNTL_SUII_H_SETTEI] - CNTL_DataSet[_CNTL_SUII_M_SETTEI]);
		Puressure_Level.M.PresetTimer	= CNTL_DataSet[_CNTL_SUII_M_DELAY];
		Puressure_Level.M.Timer			= CNTL_DataSet[_CNTL_SUII_M_DELAY];
	}
	if (	Puressure_Level.H.Level			!= CNTL_DataSet[_CNTL_SUII_H_SETTEI]
		||	Puressure_Level.H.PresetTimer	!= CNTL_DataSet[_CNTL_SUII_H_DELAY]) {
		Puressure_Level.H.Level			= CNTL_DataSet[_CNTL_SUII_H_SETTEI];
		Puressure_Level.H.PresetTimer	= CNTL_DataSet[_CNTL_SUII_H_DELAY];
		Puressure_Level.H.Timer			= CNTL_DataSet[_CNTL_SUII_H_DELAY];
	}

	pres = Read_Pressure();
	Cmeter = (uint8_t)PressureToWaterlevel(pres);

	if (Cmeter > 99) {
		Cmeter = 99;
	}
	/*
	 * 水位を記録
	 */
	CNTL_DataSet[_CNTL_SUII] = Cmeter;

	if (Cmeter >= Puressure_Level.H.Level) {
		if (Puressure_Level.PrevCmeter >= Puressure_Level.H.Level) {
			if (Puressure_Level.H.Timer == 0) {
				// 満水
				Puressure_Level.CurrentLevel = _WATERLEVEL_E1;
			}
		}
		else {
			Puressure_Level.H.Timer = Puressure_Level.H.PresetTimer;
		}
	}
	else if (Cmeter >= Puressure_Level.M.Level) {
		if (Puressure_Level.PrevCmeter >= Puressure_Level.M.Level) {
			if (Puressure_Level.M.Timer == 0) {
				// 中水
				Puressure_Level.CurrentLevel = _WATERLEVEL_E2;
			}
		}
		else {
			Puressure_Level.M.Timer = Puressure_Level.M.PresetTimer;
		}
	}
	else if (Cmeter < Puressure_Level.M.Level && Cmeter >= Puressure_Level.L.Level) {
		if (Puressure_Level.PrevCmeter < Puressure_Level.M.Level && Puressure_Level.PrevCmeter >= Puressure_Level.L.Level) {
			if (Puressure_Level.L.Timer == 0) {
				// 低水
				Puressure_Level.CurrentLevel = _WATERLEVEL_E3;
			}
		}
		else {
			Puressure_Level.L.Timer = Puressure_Level.L.PresetTimer;
		}
	}
	else if (Cmeter < Puressure_Level.E.Level) {
		if (Puressure_Level.PrevCmeter < Puressure_Level.E.Level) {
			if (Puressure_Level.E.Timer == 0) {
				// 渇水
				Puressure_Level.CurrentLevel = _WATERLEVEL_EE;
			}
		}
		else {
			Puressure_Level.E.Timer = Puressure_Level.E.PresetTimer;
		}
	}
	Puressure_Level.PrevCmeter = Cmeter;

	return Puressure_Level.CurrentLevel;
}

/*
 * 水位測定
 */
bool MeasurementWaterLevel(void)
{
	bool bRet = false;

	if (bSuiiSensUpdate) {
		if (CNTL_DataSet[_CNTL_SUII_DEVICE] == 0) {
			/*
			 * 電極センサーなのさ
			 */
			// 電極でも水圧でも_bWaterSens_Xを使用するのさ
			// ただ水圧の場合はCNTL_DataSet[_CNTL_SUII]に水深がセットされるのさ
			if (_bWaterSens_L.bSens == false && _bWaterSens_M.bSens == false && _bWaterSens_H.bSens == false) {
				// 渇水
				CNTL_DataSet[_CNTL_SUII_LEVEL]  = _WATERLEVEL_EE;
			}
			else if (_bWaterSens_L.bSens == true && _bWaterSens_M.bSens == false && _bWaterSens_H.bSens == false) {
				// 低水
				CNTL_DataSet[_CNTL_SUII_LEVEL]  = _WATERLEVEL_E3;
			}
			else if (_bWaterSens_L.bSens == true && _bWaterSens_M.bSens == true && _bWaterSens_H.bSens == false) {
				// 中水
				CNTL_DataSet[_CNTL_SUII_LEVEL]  = _WATERLEVEL_E2;
			}
			else if (_bWaterSens_L.bSens == true && _bWaterSens_M.bSens == true && _bWaterSens_H.bSens == true) {
				// 満水
				CNTL_DataSet[_CNTL_SUII_LEVEL]  = _WATERLEVEL_E1;
			}
			else {
				// センサー異常
				CNTL_DataSet[_CNTL_SUII_LEVEL]  = _WATERLEVEL_ERR;
			}

			/*
			 * 電極の場合は水位をゼロ
			 */
			CNTL_DataSet[_CNTL_SUII] = 0;
		}
		else {
			/*
			 * 水圧センサーなのさ
			 */
			CNTL_DataSet[_CNTL_SUII_LEVEL] = ComvPressureToLevel();
		}


		/*
		 * 更新フラグクリア
		 */
		bSuiiSensUpdate = false;

	    bRet = true;

		/*
		 * 周期を再設定（変更が反映）
		 */
		pWaterLevelSensTimer->preTime = CNTL_DataSet[_CNTL_SUII_CYCLE];
	    EnableTimerTask(pWaterLevelSensTimer);
	}

	return bRet;
}



void InitSensorDrive(void)
{
    // 温度測定タイマー設定(1秒単位)
    pTemperatureSensTimer = AllocTimer0Task(CNTL_DataSet[_CNTL_ONCHO_CYCLE], TemperatureSensingHandler);
    // 水位測定タイマー設定(1秒単位)
    pWaterLevelSensTimer = AllocTimer0Task(CNTL_DataSet[_CNTL_SUII_CYCLE], WaterLevelSensingHandler);
    // 水圧センサータイマー感度設定(1秒周期)
    pPressureSensitivityTimer = AllocTimer0Task(1, PressureSensitivityHandler);
    // 水圧センサー初期化
    InitPressureMeasurement();
}

void StartMeasurement(void)
{
    // センサー初期値取得＆センサータスクスタート


	ElectrodeStaticSens();

	/*
	 * 水圧センサー感度タイマー開始(1秒周期)
	 */
    EnableTimerTask(pPressureSensitivityTimer);

    bTemperatureSensUpdate	= true;
	bSuiiSensUpdate			= true;
	MeasurementTemperature();
	MeasurementWaterLevel();
}

void StopMeasurement(void)
{
	// センサー停止
    DisableTimerTask(pTemperatureSensTimer);
    DisableTimerTask(pWaterLevelSensTimer);
    DisableTimerTask(pPressureSensitivityTimer);
}

void Measurement(void)
{
	MeasurementTemperature();
	MeasurementWaterLevel();
}
