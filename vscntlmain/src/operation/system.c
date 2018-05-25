/*
 * system.c
 *
 *  Created on: 2015/12/14
 *      Author: toshio
 */

#include "system.h"
#include "uart.h"
#include "tick.h"
#include "timer32.h"
#include "i2c.h"
#include "baseclock.h"
#include "relay.h"
#include "socket.h"
#include "http.h"
#include "electrode.h"
#include "sensordrive.h"
#include "dmanage.h"
#include "netoperation.h"
#include "string.h"
#include "netoperation.h"


/*
     uint8_t mac[] = {0x00, 0x08, 0xdc, 0x1f, 0x16, 0x05};
     uint8_t mask[] = {255, 255, 255, 0};
//     uint8_t mask[] = {0, 0, 0, 0};
     uint8_t ip[] = {192, 168, 0, 188};
     uint8_t gt[] = {192, 168, 0, 1};
     uint8_t dbs[] = {192, 168, 0, 53};

     uint8_t sakura[] = {160,16,237,92};
     uint8_t orgwin[] = {133,242,136,46};

     uint8_t	*pstrResponse =	(uint8_t *)"HTTP/1.0 200 OK\r\n"
								"Content-Length: 5\r\n"
								"Connection: close\r\n"
								"Content-Type: text/html\r\n\r\n"
								"abcdef\r\n";
*/

void System_Init(void)
{
	/* --------------------------------------------------------------------
	 * -- ドライバー系初期化
	 * --------------------------------------------------------------------
	 */
	/*
	 * タイマー初期化
	 */
    Init_Tick();
    Init_TIMER32_0();
    Init_TIMER32_1();

    /*
     * UART初期化
     */
    Init_UART(600);

    /*
	 * I2C初期化
	 */
    Init_I2C(SPEED_100KHZ);

#if 0

  int8_t sn = socket(0, SOCK_TCP, 0, 0);
  int8_t status;

  while(true) {
	  status = HTTP_Request(sn,  sakura, (uint8_t *)"/", 80);
	  //status = HTTP_GETRequest(sn,  sakura, "/", 80);

	  if (status == HTTP_REQ_COMPLETE || status == HTTP_REQ_ABORT) {
		  close_socket(sn);
		  socket(0, SOCK_TCP, 0, 0);
	  }
  }
#endif
    //--------------------------------------------------------------------------------

	/*
	 * リレー初期化
	 */
    Init_Relay();

    /*
     * 水位電極初期化
     */
    Init_Electrode();

    /*
     * 時計初期化
     */
    Init_BaseClock();

	/* --------------------------------------------------------------------
	 * -- ミドル系初期化
	 * --------------------------------------------------------------------
	 */

    /*
     * センサー駆動初期化
     */
    InitSensorDrive();

    /*
     * データー管理初期化
     */
    InitDataManage();
}
