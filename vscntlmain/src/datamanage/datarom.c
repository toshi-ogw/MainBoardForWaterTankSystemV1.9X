/*
 * datarom.c
 *
 *  Created on: 2016/02/12
 *      Author: toshio
 */

#include "datarom.h"
#include "controldata.h"
#include "e2rom.h"

/*
 * Version1.06J
 * 2017/08/22
 */
#define	ROM_WORK_SIZE	32
uint8_t	RomWork_W[ROM_WORK_SIZE];
uint8_t	RomWork_R[ROM_WORK_SIZE];

uint8_t	DataRomImage[_CNTL_DATA_MAX];

uint16_t DataRomSum16(uint8_t *pImage)
{
	int i;
	uint16_t sum;
	uint16_t *pImage16;

	pImage16 = (uint16_t *)pImage;

	sum = 0;

	for (i = 0; i < _CNTL_DATA_MAX / 2; i++) {
		sum += *pImage16;
		pImage16++;
	}

	return sum;
}

void DataRomAddChecker(uint8_t *pImage)
{
	uint16_t sum0, sum1;

	CNTL_DataSet[_CNTL_MAGIC0]			= 'O';
	CNTL_DataSet[_CNTL_MAGIC1]			= 'W';
	CNTL_DataSet[_CNTL_SERVER_VERSION]	= 'I';
	CNTL_DataSet[_CNTL_SUML]	= 0;
	CNTL_DataSet[_CNTL_SUMH]	= 0;
	CNTL_DataSet[_CNTL_SUMPL]	= 0;
	CNTL_DataSet[_CNTL_SUMPH]	= 0;
	sum0 = DataRomSum16(CNTL_DataSet);
	sum1 = 0 - (sum0 + sum0);

	CNTL_DataSet[_CNTL_SUML]	= (uint8_t)(sum0 & 0xff);
	CNTL_DataSet[_CNTL_SUMH]	= (uint8_t)((sum0 >> 8) & 0xff);
	CNTL_DataSet[_CNTL_SUMPL]	= (uint8_t)(sum1 & 0xff);
	CNTL_DataSet[_CNTL_SUMPH]	= (uint8_t)((sum1 >> 8) & 0xff);

	sum0 = DataRomSum16(CNTL_DataSet);
}

bool DataRomChecker(uint8_t *pImage)
{
	uint16_t sum;

	sum = DataRomSum16(pImage);
	if (sum != 0) {
		return false;
	}

	if (*(pImage + _CNTL_MAGIC0) != 'O' || *(pImage + _CNTL_MAGIC1)	!= 'W') {
		return false;
	}

	return true;
}

bool CompareDataRom(void)
{
	int i;

	Read_E2Rom(0, DataRomImage, _CNTL_DATA_MAX);

	for (i = 0; i < _CNTL_DATA_MAX; i++) {
		if (CNTL_DataSet[i] != DataRomImage[i]) {
			return false;
		}
	}

	return true;
}


/*
 * データ保存
 *
 * 処理時間は実測で60mSec以下
 */
bool ParameterSuspend(void)
{
	int i;


	DataRomAddChecker(CNTL_DataSet);

	for (i = 0; i < _ROM_WRITE_RETRY; i++) {
		Write_E2Rom(0, CNTL_DataSet, _CNTL_DATA_MAX);
		if (CompareDataRom()) {
			/*
			 * Version1.06J
			 * 2017/08/22
			 * 通常のROMへの書込みが成功した時にのみ予備に書き込む
			 */
			MaintainNetParam((const uint8_t *)&CNTL_DataSet[_CNTL_IP0]);
			return true;
		}
	}


	return false;
}

/*
 * データ復帰
 *
 * 処理時間は実測で6mSec以下
 */
bool ParameterResume(void)
{
	int i;
	int retry;

	//-- Version 1.6A->1.6B
	/*
	Read_E2Rom(0, DataRomImage, _CNTL_DATA_MAX);
	if (!DataRomChecker(DataRomImage)) {
		return false;
	}

	for (i = 0; i < _CNTL_DATA_MAX; i++) {
		CNTL_DataSet[i] = DataRomImage[i];
	}
	*/
	retry = _ROM_READ_RETRY;

	while(1) {
		Read_E2Rom(0, DataRomImage, _CNTL_DATA_MAX);
		if (DataRomChecker(DataRomImage)) {
			break;
		}
		retry--;
		if (retry <= 0) {
			/*
			 * Version1.06J
			 * 2017/08/22
			 * ネットワークだけ救済する
			 */
			ResumeNetParam(&CNTL_DataSet[_CNTL_IP0]);
			/*
			 * Version1.06J
			 * 2017/08/22
			 * 以前の運転状態を取得する
			 */
			CNTL_DataSet[_CNTL_RUN] = ResumeRunStop();
			return false;
		}
	}

	for (i = 0; i < _CNTL_DATA_MAX; i++) {
		CNTL_DataSet[i] = DataRomImage[i];
	}
	/*
	 * Version1.06J
	 * 2017/08/22
	 * 正常に読込が出来たら念の為IPを予備へ書き込んでおく（値が異なる場合のみ）
	 */
	MaintainNetParam((const uint8_t *)&CNTL_DataSet[_CNTL_IP0]);

	/*
	 * Version1.06J
	 * 2017/08/22
	 * 以前の運転状態を取得する
	 */
	CNTL_DataSet[_CNTL_RUN] = ResumeRunStop();

	return true;
}

/*
 * ポートアドレスを取得
 */
uint16_t GetCurrentPortNumber(void)
{
	uint16_t port;
	uint8_t bf[2];

	Read_E2Rom(_ROM_INCDATA_ADRS, bf, 2);

	port = bf[1];
	port <<= 8;
	port |= bf[0];

	return port;
}

/*
 * ポートアドレスを保存
 */
void SetCurrentPortNumber(uint16_t pn)
{
	uint8_t bf[2];

	bf[0] = (pn & 0xff);
	pn >>= 8;
	bf[1] = (pn & 0xff);

	Write_E2Rom(_ROM_INCDATA_ADRS, bf, 2);
}
/*
 * ReBoot Countを取得
 */
uint16_t GetCurrentRebootCount(void)
{
	uint16_t cnt;
	uint8_t bf[2];

	Read_E2Rom(_ROM_REBOOTCNT_ADRS, bf, 2);

	cnt = bf[1];
	cnt <<= 8;
	cnt |= bf[0];

	if (cnt == 0xffff) {
		cnt = 0;
	}

	return cnt;
}

/*
 * ReBoot Countを保存
 */
void SetCurrentRebootCount(uint16_t cnt)
{
	uint8_t bf[2];

	bf[0] = (cnt & 0xff);
	cnt >>= 8;
	bf[1] = (cnt & 0xff);

	Write_E2Rom(_ROM_REBOOTCNT_ADRS, bf, 2);
}

/*
 * MACアドレス取得
 */
void GetMACAddress(uint8_t *pMac)
{
	Read_E2Rom(_ROM_MAC_ADRS, pMac, 6);
}





/*
 *	2017/08/22
 *	Version1.06J
 */
uint16_t CalcDataRomSum16(uint8_t *pImage, uint8_t size)
{
	uint16_t sum;
	uint16_t *pImage16;
	uint8_t	i;

	pImage16 = (uint16_t *)pImage;

	sum = 0;

	for (i = 0; i < size / 2; i++) {
		sum += *pImage16;
		pImage16++;
	}

	return sum;
}

/*
 * 2017/08/22 Version 1.06J
 * Data Reserve BackUp
 */
uint8_t *AcquireFromReserveParam(uint8_t romadr, uint8_t size)
{
	uint16_t	sum;
	uint8_t		i;

	for (i = 0; i < _ROM_WRITE_RETRY; i++) {
		Read_E2Rom(romadr, RomWork_R, size + 4);
		/*
		 * Sum Check
		 */
		sum = CalcDataRomSum16(RomWork_R, size);
		if (	RomWork_R[size + 0]	== (sum & 0xff) &&
				RomWork_R[size + 1]	== ((sum >> 8) & 0xff) &&
				RomWork_R[size + 2]	== (~sum & 0xff) &&
				RomWork_R[size + 3]	== ((~sum >> 8) & 0xff)) {
				/*
				 * sum OK and copy
				 */
				return RomWork_R;
		}
	}

	return NULL;
}

/*
 * 2017/08/22 Version 1.06J
 * Data Reserve BackUp
 */
bool KeepInReserveParam(const uint8_t *pImage, uint8_t romadr, uint8_t size)
{
	uint8_t		i, j;
	uint16_t	sum;

	/*
	 * Copy Data
	 */
	for (i = 0; i < size; i++) {
		RomWork_W[i] = *(pImage + i);
	}

	/*
	 * Sum Check
	 */
	sum = CalcDataRomSum16(RomWork_W, size);

	/*
	 * sum
	 */
	RomWork_W[size + 0]	= (uint8_t)(sum & 0xff);
	RomWork_W[size + 1]	= (uint8_t)((sum >> 8) & 0xff);
	/*
	 * Negative sum
	 */
	sum = ~sum;
	RomWork_W[size + 2]	= (uint8_t)(sum & 0xff);
	RomWork_W[size + 3]	= (uint8_t)((sum >> 8) & 0xff);

	for (i = 0; i < _ROM_WRITE_RETRY; i++) {
		Write_E2Rom(romadr, RomWork_W, size + 4);
		if (AcquireFromReserveParam(romadr, size)) {
			for (j = 0; j < size; j++) {
				if (RomWork_W[j] != RomWork_R[j]) {
					break;
				}
			}
			if (j == size) {
				return true;
			}
		}
	}

	return false;
}

/*
 * 2017/08/22 Version 1.06J
 * RUN/STOP Maintain
 */
const uint8_t	RUN_identification[]	= "RUNI";
const uint8_t	STP_identification[]	= "STPI";

bool MaintainRunStop(uint8_t runstop)
{
	const uint8_t *pIdent;

	if (runstop) {
		pIdent = RUN_identification;
	}
	else {
		pIdent = STP_identification;
	}

	return KeepInReserveParam(pIdent, RUNSTOP_KEEPER, 4);
}

/*
 * 2017/08/22 Version 1.06J
 * RUN/STOP Resume
 */
uint8_t ResumeRunStop(void)
{
	uint8_t	dtRunStop = 0;

	if (AcquireFromReserveParam(RUNSTOP_KEEPER, 4)) {
		if (	RomWork_R[0] == RUN_identification[0] &&
				RomWork_R[1] == RUN_identification[1] &&
				RomWork_R[2] == RUN_identification[2] &&
				RomWork_R[3] == RUN_identification[3]
				) {

			dtRunStop = 1;
		}
	}

	return dtRunStop;
}

/*
 * 2017/08/22 Version 1.06J
 * Maintain NetParam
 */
bool MaintainNetParam(const uint8_t *param)
{
	uint8_t size = (KPRSVNET_SUM - KPRSVNET_IP);
	uint8_t i;
	bool	bSame = false;

	if (AcquireFromReserveParam(KPRSVNET_IP, size)) {
		bSame = true;
		for (i = 0; i < size; i++) {
			if (*(param + i) != RomWork_R[i]) {
				bSame = false;
				break;
			}
		}
	}
	if (!bSame) {
		/*
		 * 同じじゃなかった場合にのみ書き込む
		 */
		bSame = KeepInReserveParam(param, KPRSVNET_IP, size);
	}

	return bSame;
}

/*
 * 2017/08/22 Version 1.06J
 * Maintain NetParam
 */
bool ResumeNetParam(uint8_t *param)
{
	uint8_t size = (KPRSVNET_SUM - KPRSVNET_IP);
	uint8_t i;

	if (AcquireFromReserveParam(KPRSVNET_IP, size)) {
		for (i = 0; i < size; i++) {
			*(param + i) = RomWork_R[i];
		}
	}
	else {
		return false;
	}

	return true;;
}

