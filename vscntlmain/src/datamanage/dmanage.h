/*
 * dmanage.h
 *
 *  Created on: 2015/12/08
 *      Author: toshio
 */

#ifndef DMANAGE_H_
#define DMANAGE_H_

#include "transbase.h"
#include "transcode.h"

#define	_DATA_VERSION	0x01

void InitDataManage(void);
void RC_Get(uint8_t adrs, uint8_t item, unsigned char *pBuffer, uint8_t length);
void RC_Set(uint8_t adrs, uint8_t item, unsigned char *pBuffer, uint8_t length);
void RC_Pol(uint8_t adrs, unsigned char *pBuffer, uint8_t length);
void RC_SendStatus(void);
void InitialParameterSend(void);
void dManage(void);

bool IsFootBathMode(void);
bool IsCoolingWaterMode(void);
uint8_t CoolingWaterOffsetTemperature(void);


#endif /* DMANAGE_H_ */
