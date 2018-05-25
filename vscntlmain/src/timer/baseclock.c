/*
 * baseclock.c
 *
 *  Created on: 2016/03/10
 *      Author: toshio
 */


#include "baseclock.h"
#include "controldata.h"
#include "timer32.h"



PTIMER_BLOCK m_pClockBlock;
uint32_t	m_BaseClock = 0;

/*
 * ここで定義は綺麗じゃないが
 */
uint8_t	OnTimer_ON_Complete		= 1;
uint8_t	OnTimer_OFF_Complete	= 1;


/*
 * タイマーリスタートチェック
 * 再起動時やタイマー設定時に完了フラグを仕切りなおす
 */

void OnTimerRestartCheck(void)
{
	uint32_t	StartTime, StopTime;

	StartTime = (CNTL_DataSet[_CNTL_START_TIMER_H] * 256 + CNTL_DataSet[_CNTL_START_TIMER_L]) * 60;
	StopTime  = (CNTL_DataSet[_CNTL_STOP_TIMER_H] *256 + CNTL_DataSet[_CNTL_STOP_TIMER_L]) * 60;

	/*
	 * 一旦クリア
	 */
	OnTimer_ON_Complete		= 0;
	OnTimer_OFF_Complete	= 0;

	/*
	 * 既に時間が過ぎていれば無効化する
	 */
	if (m_BaseClock >= StartTime) {
		OnTimer_ON_Complete = 1;
	}
	if (m_BaseClock >= StopTime) {
		OnTimer_OFF_Complete = 1;
	}
}


void BaseClock_Reflesh(void)
{
	m_BaseClock++;

	if (m_BaseClock > (60 * 60 * 24)) {
		m_BaseClock				= 0;

		/*
		 * タイマー完了フラグをクリア
		 */
		OnTimer_ON_Complete		= 0;
		OnTimer_OFF_Complete	= 0;

	}
}

void UpdateBaseClock(uint32_t time)
{
	/*
	 * 23時59分59秒での時間変更は行わない
	 */
	while(1) {
		if (m_BaseClock != ((60 * 60 * 24) - 1)) {
			break;
		}
	}

	m_BaseClock = time;
	/*
	 * タイマーの完了フラグを更新
	 */
	OnTimerRestartCheck();
}

void SetBaseClock(void)
{
	uint32_t dt;


	dt = CNTL_DataSet[_CNTL_CLOCK3];
	dt <<= 8;
	dt |= CNTL_DataSet[_CNTL_CLOCK2];
	dt <<= 8;
	dt |= CNTL_DataSet[_CNTL_CLOCK1];
	dt <<= 8;
	dt |= CNTL_DataSet[_CNTL_CLOCK0];

	UpdateBaseClock(dt);
}

void  GetBaseClock(void)
{
	uint32_t dt;

	dt = m_BaseClock;

	CNTL_DataSet[_CNTL_CLOCK0] = (dt & 0xff);
	dt >>= 8;
	CNTL_DataSet[_CNTL_CLOCK1] = (dt & 0xff);
	dt >>= 8;
	CNTL_DataSet[_CNTL_CLOCK2] = (dt & 0xff);
	dt >>= 8;
	CNTL_DataSet[_CNTL_CLOCK3] = (dt & 0xff);
}

uint8_t dummydata[10] = {
	0x00,
	0x00,
	0x12,
	0x34
};

void Init_BaseClock(void)
{

/*
	ALARM_DATA Alm;

	Alm.byte[0] = dummydata[2];
	Alm.byte[1] = dummydata[3];
*/

	m_BaseClock = 0;
	m_pClockBlock = AllocTimer0Task(0, BaseClock_Reflesh);
	EnableTimerTask(m_pClockBlock);
}


