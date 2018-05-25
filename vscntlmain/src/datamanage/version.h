/*
 * version.h
 *
 *  Created on: 2016/12/23
 *      Author: toshio
 */

#ifndef VERSION_H_
#define VERSION_H_

#include "chip.h"
#include <cr_section_macros.h>

void SetNetVersion(unsigned char *pVerBuffer, unsigned char *pRevBuffer);
void SetRCVersion(unsigned char *pVerBuffer, unsigned char *pRevBuffer);

#endif /* VERSION_H_ */
