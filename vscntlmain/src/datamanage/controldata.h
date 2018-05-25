/*
 * controldata.h
 *
 *  Created on: 2015/12/05
 *      Author: toshio
 */

#ifndef DATAMANAGE_CONTROLDATA_H_
#define DATAMANAGE_CONTROLDATA_H_

#include "chip.h"
#include <cr_section_macros.h>

extern	uint8_t	CNTL_DataSet[];

#define	_CNTL_RUN				0	// 運転・停止
#define	_CNTL_ONDO_L			1	// 現在温度　下位
#define	_CNTL_ONDO_H			2	// 現在温度　上位
#define	_CNTL_SUII_LEVEL		3	// 現在水位　電極使用時
#define	_CNTL_SUII				4	// 現在水位　水圧計使用時

//--------------------------------------------------------------------------------------------------- _CNTL_RUNMODE 〜 _CNTL_MIZUNUKI_TIME_H INITIAL_A
#define	_CNTL_RUNMODE			5	// 運転モード

#define	_CNTL_ONCHO_HOSEI		6	// 温調補正値
#define	_CNTL_ONCHO_UPPER		7	// 温調範囲　上限
#define	_CNTL_ONCHO_LOWER		8	// 温調範囲　下限
#define	_CNTL_ONCHO_CYCLE		9	// 温調測定周期
#define	_CNTL_ONCHO_DELAY		10	// 温調測定遅延（設定温度になった後の動作までのディレイ）

#define	_CNTL_ONDO_SETTEI_L		11	// 温度設定値　下位
#define	_CNTL_ONDO_SETTEI_H		12	// 温度設定値　上位

#define	_CNTL_SUII_DEVICE		13	// 水位測定デバイス
#define	_CNTL_SUII_HIJYU_L		14	// 比重下位
#define	_CNTL_SUII_HIJYU_H		15	// 比重上位
#define	_CNTL_SUII_KEISU_L		16	// 係数下位
#define	_CNTL_SUII_KEISU_H		17	// 係数上位
#define	_CNTL_SUII_CYCLE		18	// 水位測定周期
#define	_CNTL_SUII_L_DELAY		19	// 低　水位測定遅延（設定水位になった後の動作までのディレイ）
#define	_CNTL_SUII_M_DELAY		20	// 中　水位測定遅延（設定水位になった後の動作までのディレイ）
#define	_CNTL_SUII_H_DELAY		21	// 高　水位測定遅延（設定水位になった後の動作までのディレイ）
#define	_CNTL_SUII_L_SETTEI		22	// 水位設定（水圧デバイス使用時）
#define	_CNTL_SUII_M_SETTEI		23	// 水位設定（水圧デバイス使用時）
#define	_CNTL_SUII_H_SETTEI		24	// 水位設定（水圧デバイス使用時）
#define	_CNTL_MANSUI_TIME		25	// 満水監視時間
#define	_CNTL_MIZUNUKI_TIME_STA	26	// 運転開始時抜水時間 (秒単位)
#define	_CNTL_MIZUNUKI_TIME_STP	27	// 運転停止時排水時間（分単位)

//--------------------------------------------------------------------------------------------------- _CNTL_DHCP_ONOFF 〜 _CNTL_TERM_NO_H INITIAL_B
#define	_CNTL_DHCP_ONOFF		28	// DHCP
#define	_CNTL_IP0				29	// IP Address
#define	_CNTL_IP1				30	// IP Address
#define	_CNTL_IP2				31	// IP Address
#define	_CNTL_IP3				32	// IP Address
#define	_CNTL_MASK0				33	// MASK Address
#define	_CNTL_MASK1				34	// MASK Address
#define	_CNTL_MASK2				35	// MASK Address
#define	_CNTL_MASK3				36	// MASK Address
#define	_CNTL_GATEWAY0			37	// GateWay
#define	_CNTL_GATEWAY1			38	// GateWay
#define	_CNTL_GATEWAY2			39	// GateWay
#define	_CNTL_GATEWAY3			40	// GateWay
#define	_CNTL_DNS10				41	// DNS1
#define	_CNTL_DNS11				42	// DNS1
#define	_CNTL_DNS12				43	// DNS1
#define	_CNTL_DNS13				44	// DNS1
#define	_CNTL_SERVER0			45	// Server IP
#define	_CNTL_SERVER1			46	// Server IP
#define	_CNTL_SERVER2			47	// Server IP
#define	_CNTL_SERVER3			48	// Server IP

#define	_CNTL_CLOCK_CNTL		49	// 時計コントロール
#define	_CNTL_CLOCK0			50	// 時計 0xXXXT
#define	_CNTL_CLOCK1			51	// 時計 0xXXTX
#define	_CNTL_CLOCK2			52	// 時計 0xXTXX
#define	_CNTL_CLOCK3			53	// 時計 0xTXXX
#define	_CNTL_START_TIMER_L		54	// 運転開始タイマー下位
#define	_CNTL_START_TIMER_H		55	// 運転開始タイマー上位
#define	_CNTL_STOP_TIMER_L		56	// 運転停止タイマー下位
#define	_CNTL_STOP_TIMER_H		57	// 運転停止タイマー上位

/*
#define	_CNTL_TERM_NO_L			58	// コントローラ端末番号　下位
#define	_CNTL_TERM_NO_H			59	// コントローラ端末番号　上位
*/

#define	_CNTL_SUPPLYTIME		58	// 給水監視タイマ（給水の初期を関しEEを超えるまで監視）（分単位）
#define	_CNTL_DRIVETIME			59	// 停止からドライブリレーをオフるまでの時間を設定（秒単位）

#define	_CNTL_RC				60	// セカンダリリモコンあり？
#define	_CNTL_RELAY_STATE		61	// リレーステータス
#define	_CNTL_ERROR_STATE		62	// エラーステータス
#define	_CNTL_MAGIC0			63	// magic code
#define	_CNTL_MAGIC1			64	// magic code
#define	_CNTL_SERVER_VERSION	65	// Version Code
#define	_CNTL_SUML				66	// SUM LO
#define	_CNTL_SUMH				67	// SUM HI
#define	_CNTL_SUMPL				68	// SUMP LO
#define	_CNTL_SUMPH				69	// SUMP HI
// STOPPER
#define	_CNTL_DATA_MAX			70

/*
 * 運転モード
 */
#define	_CNTL_RUNMODE_NORMAL					0
#define	_CNTL_RUNMODE_NO_TEMPERATURE_CONTROL	1
#define	_CNTL_RUNMODE_NORMAL_WASHING			2


#define	CNTL_DATA_TEMPERATURE			((CNTL_DataSet[_CNTL_ONDO_H] << 8) | CNTL_DataSet[_CNTL_ONDO_L])
#define	CNTL_DATA_TEMPERATURE_LO_CHECK	(CNTL_DATA_TEMPERATURE - CNTL_DataSet[_CNTL_ONCHO_LOWER])
#define	CNTL_DATA_TEMPERATURE_HI_CHECK	(CNTL_DATA_TEMPERATURE + CNTL_DataSet[_CNTL_ONCHO_UPPER])
#define	CNTL_DATA_WATERLEVEL			CNTL_DataSet[_CNTL_SUII_LEVEL]


/*
 * リレーステータス
 */
#define	_CNTL_RY_BIT_HEATER		0b00000001
#define	_CNTL_RY_BIT_WATER		0b00000010
#define	_CNTL_RY_BIT_PUMP		0b00000100
#define	_CNTL_RY_BIT_DRAINAGE	0b00001000
#define	_CNTL_RY_BIT_ERROR		0b00010000
#define	_CNTL_RY_BIT_DRIVE		0b10000000

/*
 * リレーステータスに付加するがリレーの状態を表すものでは無い
 */
#define	_CNTL_RY_BIT_FWS		0b01000000

/*
 * Version1.06L
 * 強制給水フラグ
 * このフラグは循環運転中のみ有効となる。
 *
 *
 */
extern uint8_t m_ForceWaterSupply;

#endif /* DATAMANAGE_CONTROLDATA_H_ */
