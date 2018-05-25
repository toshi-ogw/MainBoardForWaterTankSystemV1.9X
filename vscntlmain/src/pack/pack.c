/*
 * pack.c
 *
 *  Created on: 2016/04/03
 *      Author: toshio
 */

#include "pack.h"

int32_t decStr2Value(uint8_t *pstr, uint8_t length)
{
	uint8_t i;
	uint32_t Value;

	Value = 0;
	for (i = 0; i < length; i++) {
		if (*pstr < '0' || *pstr > '9') {
			/*
			 * 変換できない！
			 */
			return -1;
		}
		Value *= 10;
		Value += (*pstr - '0');
		pstr++;
	}

	return Value;
}

/*
 *
 */
uint8_t *Value2decStr(uint8_t *pstr, uint8_t ByteValue)
{
	uint8_t i, c;
	uint8_t Buf[3];

	c = 0;
	Buf[c] = (ByteValue / 100);
	if (Buf[c] > 0) {
		Buf[c] += '0';
		c++;
	}
	ByteValue -= ((ByteValue / 100) * 100);
	Buf[c] = (ByteValue / 10);
	if (Buf[c] > 0) {
		Buf[c] += '0';
		c++;
	}
	Buf[c] = (ByteValue % 10) + '0';
	c++;

	for (i = 0; i < c; i++) {
		*pstr = Buf[i];
		pstr++;
	}

	return pstr;
}

int32_t hexStr2Value(uint8_t *pstr, uint8_t length)
{
	uint8_t i;
	uint32_t Value;

	Value = 0;
	for (i = 0; i < length; i++) {
		if (*pstr >= '0' && *pstr <= '9') {
			Value *= 16;
			Value += (*pstr - '0');
		}
		else if (*pstr >= 'A' && *pstr <= 'F') {
			Value *= 16;
			Value += ((*pstr - 'A') + 10);
		}
		else if (*pstr >= 'a' && *pstr <= 'f') {
			Value *= 16;
			Value += ((*pstr - 'a') + 10);
		}
		else {
			/*
			 * 変換できやしないぜ
			 */
			return -1;
		}
		pstr++;
	}

	return Value;
}

uint8_t hexTable[16]="0123456789ABCDEF";
/*
 *
 */
uint8_t *Value2hexStr(uint8_t *poutstr, uint8_t *pinstr, uint8_t length)
{
	uint8_t i;

	for (i = 0; i < length; i++) {
		*poutstr = hexTable[*pinstr >> 4];
		poutstr++;
		*poutstr = hexTable[*pinstr & 0xf];
		poutstr++;
		pinstr++;
	}

	return poutstr;
}

/*
 * Version 1.06 2016/12/23 T.Ogawa
 */
void SetStrData(unsigned char *pBuffer, char *pstrText, int maxLength)
{
	int i;

	for (i = 0; i < maxLength; i++) {
		if (*pstrText == 0) {
			break;
		}
		*pBuffer = *pstrText;

		pBuffer++;
		pstrText++;
	}
}

