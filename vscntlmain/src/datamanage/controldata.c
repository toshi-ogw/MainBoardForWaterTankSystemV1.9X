/*
 * controldata.c
 *
 *  Created on: 2015/12/05
 *      Author: toshio
 */
#include "../datamanage/controldata.h"

uint8_t	CNTL_DataSet[_CNTL_DATA_MAX] = {
        0x00,                  // _CNTL_RUN					// 運転・停止		(def : 0)
        0xa4,                  // _CNTL_ONDO_L				// 現在温度　下位	(0)
        0x01,                  // _CNTL_ONDO_H				// 現在温度　上位	(0)
        0x00,                  // _CNTL_SUII_LEVEL			// 現在水位 水位E1 E2 E3
        0x00,                  // _CNTL_SUII				// 現在水位

		0x00,                  // _CNTL_RUNMODE				// 運転モード

          10,                  // _CNTL_ONCHO_HOSEI			// 温調補正値		(def : 10)
           5,                  // _CNTL_ONCHO_UPPER			// 温調範囲　上限	(def : 5)
           5,                  // _CNTL_ONCHO_LOWER			// 温調範囲　下限	(def : 5)
           2,                  // _CNTL_ONCHO_CYCLE			// 温調測定周期		(def : 2)
           5,                  // _CNTL_ONCHO_DELAY			// 温調測定遅延（設定温度になった後の動作までのディレイ）(def : 5)
        0x90,                  // _CNTL_ONDO_SETTEI_L		// 温度設定値　下位   (def 40.0℃ = 190h)
        0x01,                  // _CNTL_ONDO_SETTEI_H		// 温度設定値　上位

        0x01,                  // _CNTL_SUII_DEVICE			// 水位測定デバイス	(def : 0)
        0xca,                  // _CNTL_SUII_HIJYU_L		// 比重下位			(def : 9930 = 26cah)
        0x26,                  // _CNTL_SUII_HIJYU_H		// 比重上位
        0x04,                  // _CNTL_SUII_KEISU_L		// 係数下位			(def : 1028 = 404h))
        0x04,                  // _CNTL_SUII_KEISU_H		// 係数上位
           2,                  // _CNTL_SUII_CYCLE			// 水位測定周期		(def : 2)
           5,                  // _CNTL_SUII_L_DELAY		// 低　水位測定遅延（設定水位になった後の動作までのディレイ））(def : 5)
           5,                  // _CNTL_SUII_M_DELAY		// 中　水位測定遅延（設定水位になった後の動作までのディレイ））(def : 5)
           5,                  // _CNTL_SUII_H_DELAY		// 高　水位測定遅延（設定水位になった後の動作までのディレイ））(def : 5)
	      10,                  // _CNTL_SUII_L_SETTEI		// 水位設定（水圧デバイス使用時）渇水（センサーからの高さ）		(def : 10cm)
	       2,                  // _CNTL_SUII_M_SETTEI		// 水位設定（水圧デバイス使用時）給水 (満水からのオフセット)   	(def : 2cm)
		  30,                  // _CNTL_SUII_H_SETTEI		// 水位設定（水圧デバイス使用時）満水（センサー位置からの高さ）	(def : 60cm)
          60,                  // _CNTL_MANSUI_TIME			// 満水監視下位 (def : 6時間)
          10,                  // _CNTL_MIZUNUKI_TIME0		// 運転開始時抜水時間 (秒単位）(def : 10 sec)
           0,                  // _CNTL_MIZUNUKI_TIME1		// 運転停止時排水時間（分単位)（def : 0 min）

           0,                  // _CNTL_DHCP_ONOFF			// DHCP
         192,                  // _CNTL_IP0					// IP Address
         168,                  // _CNTL_IP1					// IP Address
           0,                  // _CNTL_IP2					// IP Address
          20,                  // _CNTL_IP3					// IP Address
         255,                  // _CNTL_MASK0				// MASK Address
         255,                  // _CNTL_MASK1				// MASK Address
         255,                  // _CNTL_MASK2				// MASK Address
           0,                  // _CNTL_MASK3				// MASK Address
         192,                  // _CNTL_GATEWAY0			// GateWay
         168,                  // _CNTL_GATEWAY1			// GateWay
           0,                  // _CNTL_GATEWAY2			// GateWay
           2,                  // _CNTL_GATEWAY3			// GateWay
         192,                  // _CNTL_DNS10				// DNS1
         168,                  // _CNTL_DNS11				// DNS1
           0,                  // _CNTL_DNS12				// DNS1
           2,                  // _CNTL_DNS13				// DNS1
         192,                  // _CNTL_SERVER0				// Server IP
         168,                  // _CNTL_SERVER1				// Server IP
           0,                  // _CNTL_SERVER2				// Server IP
           2,                  // _CNTL_SERVER3				// Server IP

		0x00,                  // _CNTL_CLOCK_CNTL          // 時計コントロール
		0x00,                  // _CNTL_CLOCK0				// 時計 0xXXXT
		0x00,                  // _CNTL_CLOCK1				// 時計 0xXXTX
		0x00,                  // _CNTL_CLOCK2				// 時計 0xXTXX
		0x00,                  // _CNTL_CLOCK3				// 時計 0xTXXX
		0x00,                  // _CNTL_START_TIMER_L		// 運転開始タイマー下位
		0x00,                  // _CNTL_START_TIMER_H		// 運転開始タイマー上位
		0x00,                  // _CNTL_STOP_TIMER_L		// 運転停止タイマー下位
		0x00,                  // _CNTL_STOP_TIMER_H		// 運転停止タイマー上位

/*
 * Version 1.05 2016/12/10 T.Ogawa

        0x02,                  // _CNTL_TERM_NO_L			// コントローラ端末番号
        0x00,                  // _CNTL_TERM_NO_H			// コントローラ端末番号
*/
        15,                    // _CNTL_SUPPLYTIME			// 給水監視タイマ（給水の初期を関しEEを超えるまで監視）（分単位）
        10,                    // _CNTL_DRIVETIME			// 停止からドライブリレーをオフるまでの時間を設定（秒単位）

/*
 * Version 1.06K 2017/09/08
 *
 * _CNTL_RCの構成を変更する
 *
 * b7 -- Option4
 * b6 -- Option3
 * b5 -- Option2
 * b4 -- Option1 0:標準モード　1:CoolIn
 * b3 -- Option0 0:標準モード　1:足湯モード
 * b2 -- RC Mode
 * b1 -- RC Mode
 * b0 -- RC Mode RC Mode0～7まで
 *
 */
		0x00,				   // _CNTL_RC					// リモコン数

        0x00,                  // _CNTL_RELAY_STATE			// リレーステータス
        0x00,                  // _CNTL_ERROR_STATE			// エラーステータス

        'O',                   // _CNTL_MAGIC0				// magic code
        'W',                   // _CNTL_MAGIC1				// magic code
        'I',                   // _CNTL_MAGIC2				// magic code
        0x00,                  // _CNTL_SUML				// sum LO
        0x00,                  // _CNTL_SUMH				// sum HI
        0x00,                  // _CNTL_SUMPL				// sump LO
        0x00,                  // _CNTL_SUMPH				// sump HI
};


/*
 * Version1.06L
 * 強制給水フラグ
 * このフラグは循環運転中のみ有効となる。
 *
 *
 */
uint8_t m_ForceWaterSupply = 0;

