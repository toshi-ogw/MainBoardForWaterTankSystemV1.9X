/*
 * ads1015.c
 *
 *  Created on: 2015/10/21
 *      Author: toshio
 */
#include "chip.h"
#include <cr_section_macros.h>
#include "ads1015.h"

int Read_Adc(uint8_t reg, uint16_t *pvalue)
{
	int res;

	union {
		uint8_t		byte[2];
		uint16_t	word;
	} conv;
	uint8_t bt;

	res = Chip_I2C_MasterSend(I2C0, ADS1015_DEVICEID, &reg, 1);
	res = Chip_I2C_MasterRead(I2C0, ADS1015_DEVICEID, &conv.byte[0], 2);

	bt = conv.byte[0];
	conv.byte[0] = conv.byte[1];
	conv.byte[1] = bt;


	*pvalue = conv.word;

	return res;
}

int Write_Adc(uint8_t reg, uint16_t value)
{
	int res;

	uint8_t	wdat[3];

	union {
		uint8_t		byte[2];
		uint16_t	word;
	} conv;

	conv.word = value;
	wdat[0] = reg;
	wdat[1] = conv.byte[1];
	wdat[2] = conv.byte[0];

	res = Chip_I2C_MasterSend(I2C0, ADS1015_DEVICEID, wdat, 3);

	return res;
}

int16_t StartConversion_Adc(uint16_t config)
{
	uint16_t state;
	int16_t	value;

	// performing Check
	while(1) {
		Read_Adc(ADS1015_CONFIGN_REGISTER, &state);

		if (state & ADS1015_CONFIG_BEGIN_CONV) {
			break;
		}
	}

	// performing start
	Write_Adc(ADS1015_CONFIGN_REGISTER, config | ADS1015_CONFIG_BEGIN_CONV);

	// performing Check
	while(1) {
		Read_Adc(ADS1015_CONFIGN_REGISTER, &state);

		if (state & ADS1015_CONFIG_BEGIN_CONV) {
			break;
		}
	}

	// read conversion Register
	Read_Adc(ADS1015_CONVERSION_REGISTER, (uint16_t*)&value);

	return (value / 16);
}
