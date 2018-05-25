/*
 * netoperation.h
 *
 *  Created on: 2016/04/05
 *      Author: toshio
 */

#ifndef NET_NETOPERATION_H_
#define NET_NETOPERATION_H_


#include "chip.h"
#include <cr_section_macros.h>
#include "http.h"

void Init_NetDevice(void);
void Init_HTTP_Request(void);

int8_t NetOPEProcessing(void *pDat);
PNET_LISTEN_DATA Init_Listen(void);
void HTTP_RequestFunc(void);

extern volatile uint8_t HTTP_RequestSW;

#define	_HTTP_REPORT	{ HTTP_RequestSW = true; }

#endif /* NET_NETOPERATION_H_ */
