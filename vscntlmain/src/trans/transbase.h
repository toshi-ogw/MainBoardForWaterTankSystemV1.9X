/*
 * transbase.h
 *
 *  Created on: 2015/12/05
 *      Author: toshio
 */

#ifndef TR_TRANSBASE_H_
#define TR_TRANSBASE_H_

#include "chip.h"
#include <cr_section_macros.h>

typedef struct {
	uint8_t	address;
	uint8_t	command;
	uint8_t	code;
	uint8_t	length;
	int		status;
	unsigned char *pBuffer;
	int		(*pFunc)(uint8_t address, uint8_t command, uint8_t code, unsigned char *pBuffer, uint8_t length);
} TR_SENDRECEIVE_PACK, *PTR_SENDRECEIVE_PACK;

#define	RS_CODE_STX				0x02
#define	RS_CODE_ETX				0x03
#define	RS_CODE_ACK				0x06
#define	RS_CODE_NAK				0x15

#define	_RESPONSE_POS_STX		0
#define	_RESPONSE_POS_ADDRESS	1
#define	_RESPONSE_POS_COM		2
#define	_RESPONSE_POS_CODE		3
#define	_RESPONSE_POS_LNG_H		4
#define	_RESPONSE_POS_LNG_L		5
#define	_RESPONSE_POS_TEXT		6

#define	_ADDRESS_PRIMARY		'0'
#define	_ADDRESS_SECONDARY		'1'
#define	_GLOBAL_ADDRESS			'Z'


void Init_TransBase(void);
void TR_InitSendReceivePack(PTR_SENDRECEIVE_PACK pPack);
void TR_InitSend(PTR_SENDRECEIVE_PACK pTxPack, PTR_SENDRECEIVE_PACK pRxPack);
int TR_NonBlockSend(void);
int TR_NonBlockReceive(void);
int TR_NonBlockTrans(void);


#define	_TR_STATE_IDLE				0
#define	_TR_STATE_SENDING			10
#define	_TR_STATE_RECEIVEING		20
#define	_TR_STATE_COMPLETE			30
#define	_TR_STATE_LENGTH_ERROR		-1
#define	_TR_STATE_CTR_ERROR			-2
#define	_TR_STATE_TIMEOUT_ERROR		-10

#endif /* TR_TRANSBASE_H_ */
