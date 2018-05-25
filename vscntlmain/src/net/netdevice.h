/*
 * netdevice.h
 *
 *  Created on: 2016/05/19
 *      Author: toshio
 */

#ifndef NET_NETDEVICE_H_
#define NET_NETDEVICE_H_

#include "chip.h"
#include <cr_section_macros.h>

#include "w5500.h"


void Init_NetDevice(void);
void Restart_NetDevice(void);
bool NetDeviceCheck(void);

#endif /* NET_NETDEVICE_H_ */
