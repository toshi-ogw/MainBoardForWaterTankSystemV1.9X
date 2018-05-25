/*
 * crc.c
 *
 *  Created on: 2015/11/30
 *      Author: toshio
 */

#include "../trans/crc.h"

#include <limits.h>


#define	MSB_CRC8	(0x85)	// x8 + x7 + x2 + x0

unsigned char GetCRC8(const void *buff, size_t size)
{
	int i;
    unsigned char *p = (unsigned char *)buff;
    unsigned char crc8;

    for (crc8 = 0x00; size != 0; size--) {
        crc8 ^= *p++;

        for (i = 0 ; i < CHAR_BIT ; i++) {
            if (crc8 & 0x80) {
                crc8 <<= 1; crc8 ^= MSB_CRC8;
            }
            else {
                crc8 <<= 1;
            }
        }
    }
    return crc8;
}
