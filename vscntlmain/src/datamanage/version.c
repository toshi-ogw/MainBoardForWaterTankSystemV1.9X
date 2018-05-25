/*
 * version.c
 *
 *  Created on: 2016/12/23
 *      Author: toshio
 */

#include "version.h"

const unsigned char	_DT_Version[]		= {0x00, 0x00, 0x00, 0x01};
const unsigned char	_DT_Revision[]		= {0x00, 0x00, 0x00, 0x06};
/*
 * Version1.06K 2017/09/11 T.Ogawa
 */
/*
 * Version1.96M 2018/05/25 T.Ogawa 温泉タンク仕様
 */
const unsigned char	_DT_RCVersion[]		= "0001";
const unsigned char	_DT_RCRevision[]	= "096M";

void SetNetVersion(unsigned char *pVerBuffer, unsigned char *pRevBuffer)
{
	int i = 0;

	for (i = 0; i < 4; i++) {
		*pVerBuffer = _DT_Version[i];
		*pRevBuffer = _DT_Revision[i];
		pVerBuffer++;
		pRevBuffer++;
	}
}

void SetRCVersion(unsigned char *pVerBuffer, unsigned char *pRevBuffer)
{
	int i = 0;

	for (i = 0; i < 4; i++) {
		/*
		 * Version1.06K 2017/09/11 T.Ogawa
		 */
		*pVerBuffer = _DT_RCVersion[i];
		*pRevBuffer = _DT_RCRevision[i];
		pVerBuffer++;
		pRevBuffer++;
	}
}
