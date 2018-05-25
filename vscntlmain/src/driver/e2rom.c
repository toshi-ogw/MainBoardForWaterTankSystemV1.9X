/*
 * e2rom.c
 *
 *  Created on: 2015/10/21
 *      Author: toshio
 */
#include "chip.h"
#include <cr_section_macros.h>
#include "e2rom.h"

int Read_E2Rom(uint8_t adrs, uint8_t *buff, uint8_t len)
{
	int ret ;

	ret = Chip_I2C_MasterSend(I2C0, E2ROM_DEVICEID, &adrs, 1);
	ret = Chip_I2C_MasterRead(I2C0, E2ROM_DEVICEID, buff, len);

	return ret;
}
int Write_E2Rom(uint8_t adrs, uint8_t *buff, uint8_t len)
{
	int st ;
	int i, wlen, slen;
	static uint8_t	buffer[9];

	slen = 0;
	while(1){
		buffer[0] = adrs;
		wlen = 0;
		for (i = 0; i < 8; i++) {
			if (len <= 0) {
				break;
			}
			buffer[i + 1] = *buff;
			buff++;
			len--;
			adrs++;
			wlen++;
		}
		if (wlen <=0) {
			break;
		}
		st = Chip_I2C_MasterSend(I2C0, E2ROM_DEVICEID, buffer, wlen + 1);
		if (st == wlen + 1) {
			slen += wlen;
		}
		/*
		 * ウェイト5mSec
		 */
		uint32_t dwait;
		dwait = 18000;
		while (dwait) {
			dwait--;
		}
	}

	return slen;
}
