/*
 * ontimer.c
 *
 *  Created on: 2016/04/07
 *      Author: toshio
 */

#include "baseclock.h"
#include "controldata.h"
#include "timer32.h"
#include "ontimer.h"

/*
 * baseclock.cで定義
 */
extern uint8_t	OnTimer_ON_Complete;
extern uint8_t	OnTimer_OFF_Complete;

/*
 * タイマー設定をチェックし結果を返す
 *
 * 	return ==  0 停止
 * 	return ==  1 運転
 * 	return == -1 何もしない
 */
int8_t OnTimerCheck(void)
{
	uint32_t	StartTime, StopTime;
	int8_t		Result;

	/*
	 * 設定されているか確認
	 */
	if (CNTL_DataSet[_CNTL_CLOCK_CNTL] == 0) {
		/*
		 * タイマー設定されていない。
		 * 何もしないで帰る
		 */
		return -1;
	}
	StartTime = (CNTL_DataSet[_CNTL_START_TIMER_H] * 256 + CNTL_DataSet[_CNTL_START_TIMER_L]) * 60;
	StopTime  = (CNTL_DataSet[_CNTL_STOP_TIMER_H] *256 + CNTL_DataSet[_CNTL_STOP_TIMER_L]) * 60;

	/*
	 * 何もしない設定
	 */
	Result = -1;

	if (StartTime < StopTime) {
		/*
		 * 開始時間の後に停止時間が来る場合
		 */
		if (OnTimer_ON_Complete == 0 && m_BaseClock >= StartTime) {
			OnTimer_ON_Complete		= 1;
			Result					= 1;
		}
		if (OnTimer_OFF_Complete == 0 && m_BaseClock >= StopTime) {
			OnTimer_OFF_Complete	= 1;
			Result					= 0;
		}
	}
	else if (StartTime > StopTime) {
		/*
		 * 開始時間の前に停止時間が来る場合
		 */
		if (OnTimer_OFF_Complete == 0 && m_BaseClock >= StopTime) {
			OnTimer_OFF_Complete	= 1;
			Result					= 0;
		}
		if (OnTimer_ON_Complete == 0 && m_BaseClock >= StartTime) {
			OnTimer_ON_Complete		= 1;
			Result					= 1;
		}
	}

	if (Result >= 0) {
		/*
		 * ここでセットしちゃえ！
		 */
		if (CNTL_DataSet[_CNTL_RUN] != Result) {
			/*
			 * デバッグブレークしたいから
			 */
			CNTL_DataSet[_CNTL_RUN] = Result;
		}
	}

	return Result;
}
