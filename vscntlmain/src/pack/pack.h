/*
 * pack.h
 *
 *  Created on: 2016/04/03
 *      Author: toshio
 */

#ifndef PACK_PACK_H_
#define PACK_PACK_H_

#include "chip.h"
#include <cr_section_macros.h>

int32_t decStr2Value(uint8_t *pstr, uint8_t length);
int32_t hexStr2Value(uint8_t *pstr, uint8_t length);

uint8_t *Value2decStr(uint8_t *pstr, uint8_t ByteValue);
uint8_t *Value2hexStr(uint8_t *poutstr, uint8_t *pinstr, uint8_t length);

/*
 * Version 1.06 2016/12/23 T.Ogawa
 */
void SetStrData(unsigned char *pBuffer, char *pstrText, int maxLength);


#endif /* PACK_PACK_H_ */
