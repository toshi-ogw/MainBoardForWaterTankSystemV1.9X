/*
 * crc.h
 *
 *  Created on: 2015/11/30
 *      Author: toshio
 */

#ifndef TRANS_CRC_H_
#define TRANS_CRC_H_

#include "chip.h"
#include <cr_section_macros.h>

unsigned char GetCRC8(const void *buff, size_t size);


#endif /* TRANS_CRC_H_ */
