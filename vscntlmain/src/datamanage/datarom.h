/*
 * datarom.h
 *
 *  Created on: 2016/02/12
 *      Author: toshio
 */

#ifndef DATAMANAGE_DATAROM_H_
#define DATAMANAGE_DATAROM_H_


#include "chip.h"
#include <cr_section_macros.h>

#define	_ROM_WRITE_RETRY		5
#define	_ROM_READ_RETRY			5



/*
 * ROM Address Map
 */

/*
 * 		00h
 * 		 |
 * 		 |	CNTL_DataSet[_CNTL_DATA_MAX]
 * 		 |
 * 		45h
 * 		46h	----------
 * 		47h	----------
 * 		48h	----------
 * 		49h	----------
 * 		4ah	----------
 * 		4bh	----------
 * 		4ch	----------
 * 		4dh	----------
 * 		4eh	----------
 * 		4fh	----------
 * 		50h	RUNSTOP_KEEPER0
 * 		51h	RUNSTOP_KEEPER1
 * 		52h	RUNSTOP_KEEPER2
 * 		53h	RUNSTOP_KEEPER3
 * 		54h	RUNSTOP_SUML
 * 		55h	RUNSTOP_SUMH
 * 		56h	RUNSTOP_NSUML
 * 		57h	RUNSTOP_NSUMH
 * 		58h	----------
 * 		59h	----------
 * 		5ah	----------
 * 		5bh	----------
 * 		5ch	----------
 * 		5dh	----------
 * 		5eh	----------
 * 		5fh	----------
 * 		60h	KPRSVNET_IP0
 * 		61h	KPRSVNET_IP1
 * 		62h	KPRSVNET_IP2
 * 		63h	KPRSVNET_IP3
 * 		64h	KPRSVNET_MASK0
 * 		65h	KPRSVNET_MASK1
 * 		66h	KPRSVNET_MASK2
 * 		67h	KPRSVNET_MASK3
 * 		68h	KPRSVNET_GATEWAY0
 * 		69h	KPRSVNET_GATEWAY1
 * 		6ah	KPRSVNET_GATEWAY2
 * 		6bh	KPRSVNET_GATEWAY3
 * 		6ch	KPRSVNET_DNS0
 * 		6dh	KPRSVNET_DNS1
 * 		6eh	KPRSVNET_DNS2
 * 		6fh	KPRSVNET_DNS3
 * 		70h	KPRSVNET_SRV0
 * 		71h	KPRSVNET_SRV1
 * 		72h	KPRSVNET_SRV2
 * 		73h	KPRSVNET_SRV3
 * 		74h	KPRSVNET_SUML
 * 		75h	KPRSVNET_SUMH
 * 		76h	KPRSVNET_NSUML
 * 		77h	KPRSVNET_NSUML
 * 		78h	----------
 * 		79h	----------
 * 		7ah	----------
 * 		7bh	----------
 * 		7ch	reboot count lo
 * 		7dh	reboot count hi
 * 		7eh	port number lo
 * 		7fh	port number hi
 */

#define	RUNSTOP_KEEPER		0x50

#define	KPRSVNET_IP			0x60
#define	KPRSVNET_MASK		0x64
#define	KPRSVNET_GATEWAY	0x68
#define	KPRSVNET_DNS		0x6c
#define	KPRSVNET_SRV		0x70
#define	KPRSVNET_SUM		0x74

/*
 * MAC アドレスのアドレス
 */
#define	_ROM_MAC_ADRS			0xfa
/*
 * インクリメントデータアドレス *
 */
#define	_ROM_INCDATA_ADRS		(0x80 - 2)
/*
 * WDTインクリメントデータアドレス *
 */
#define	_ROM_REBOOTCNT_ADRS		(0x80 - 4)

bool ParameterSuspend(void);
bool ParameterResume(void);

uint16_t GetCurrentPortNumber(void);
void	 SetCurrentPortNumber(uint16_t pn);
void     GetMACAddress(uint8_t *pMac);

uint16_t GetCurrentRebootCount(void);
void SetCurrentRebootCount(uint16_t cnt);


/*
 * Version1.06J
 * 2017/08/22
 */

uint8_t *AcquireFromReserveParam(uint8_t romadr, uint8_t size);
bool KeepInReserveParam(const uint8_t *pImage, uint8_t romadr, uint8_t size);
bool MaintainRunStop(uint8_t runstop);
uint8_t ResumeRunStop(void);
bool MaintainNetParam(const uint8_t *param);
bool ResumeNetParam(uint8_t *param);

#endif /* DATAMANAGE_DATAROM_H_ */
