/*
 * netdevice.c
 *
 *  Created on: 2016/05/19
 *      Author: toshio
 */

#include "netdevice.h"
#include "controldata.h"
#include "datarom.h"


uint8_t MacAddress[6];

/*
 * デバイスを初期化
 */
void Init_NetDevice(void)
{
	/*
	 * デバイス設定
	 */
	Init_W5500();

	/*
	 * 設定ロムからMACアドレスを取得
	 */
	GetMACAddress(MacAddress);

	W5500_SetAddress(MacAddress, &CNTL_DataSet[_CNTL_IP0], &CNTL_DataSet[_CNTL_MASK0], &CNTL_DataSet[_CNTL_GATEWAY0]);
}

/*
 * デバイスを再初期化
 */
void Restart_NetDevice(void)
{
	/*
	 * デバイス設定
	 */
	Reset_W5500();

	/*
	 * 設定ロムからMACアドレスを取得
	 */
	GetMACAddress(MacAddress);

	W5500_SetAddress(MacAddress, &CNTL_DataSet[_CNTL_IP0], &CNTL_DataSet[_CNTL_MASK0], &CNTL_DataSet[_CNTL_GATEWAY0]);
}

/*
 * デバイスのチェック
 */
bool NetDeviceCheck(void)
{
	uint8_t ADbuff[6];

	/*
	 * IP アドレス
	 */
	W5500_ReadBuffer(W5500_COM_SOURCE_IP_ADDRESS, ADbuff, 4);
	if (ADbuff[0] != CNTL_DataSet[_CNTL_IP0] || ADbuff[1] != CNTL_DataSet[_CNTL_IP1] || ADbuff[2] != CNTL_DataSet[_CNTL_IP2] || ADbuff[3] != CNTL_DataSet[_CNTL_IP3]) {
		return false;
	}

	/*
	 * DEVICE macアドレス
	 */
	W5500_ReadBuffer(W5500_COM_SOURCE_HARDWARE_ADDRESS, ADbuff, 6);
	if (ADbuff[0] != MacAddress[0] || ADbuff[1] != MacAddress[1] || ADbuff[2] != MacAddress[2] || ADbuff[3] != MacAddress[3] || ADbuff[4] != MacAddress[4] || ADbuff[5] != MacAddress[5]) {
		return false;
	}

	return true;
}
