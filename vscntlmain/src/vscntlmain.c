/*
===============================================================================
 Name        : vscntlmain.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>

#include "system.h"
#include "relay.h"
#include "uart.h"
#include "operation.h"

#include "wdt.h"

// TODO: insert other include files here

// TODO: insert other definitions and declarations here
unsigned char buff[10] = {0xa0, 0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9 };
unsigned char rxBuffer[20];

unsigned char testBuffer[32];

int main(void) {

#if defined (__USE_LPCOPEN)
    // Read clock settings and update SystemCoreClock variable
    SystemCoreClockUpdate();
#if !defined(NO_BOARD_LIB)
    // Set up and initialize all required blocks and
    // functions related to the board hardware
    Board_Init();
    // Set the LED to the state of "On"
    Board_LED_Set(0, true);
#endif
#endif

    //Debug!!
    //ElectrodeDebug();

    // TODO: insert code here
    System_Init();

    while(0) {
    	SendRB_UART("A", 8);
    }

#if 0
    uint32_t wsts ;

    WDT_Start(10);

    while(1) {
    	wsts = Chip_WWDT_GetStatus(LPC_WWDT);
    }
#endif

/*
    RC_Get('0', '0', testBuffer, sizeof(testBuffer));
    while(1) {
    	dManage();
    }
*/
//    Wait(500);
/*
    int tts = 0;

    while(1){
    	if (IS_WATERSENS_L) {
    		tts = 1;
    	}
    	if (IS_WATERSENS_M) {
    		tts = 2;
    	}
    	if (IS_WATERSENS_H) {
    		tts = 3;
    	}
    	Measurement();

    }

*/
   /*
     RC_Pol('0', '0', testBuffer, sizeof(testBuffer));
     while(1) {
    	 dManage();
     }



    testBuffer[0] = 0x00;
    testBuffer[1] = 0xa0;
    testBuffer[2] = 0x01;
    testBuffer[3] = 0;
    testBuffer[4] = 0;

    RC_Set('0', '0', testBuffer, 5);

    while(1) {
    	dManage();
    }


*/

#if 0
    SET_RELAY_HEATER(_RELEAY_OFF);
	SET_RELAY_WATER(_RELEAY_OFF);
	SET_RELAY_PUMP(_RELEAY_OFF);
	SET_RELAY_DRAINAGE(_RELEAY_OFF);
	SET_RELAY_ERROR(_RELEAY_OFF);

	// ドライブON
	SET_RELAY_DRIVE(_RELEAY_ON);

	SET_RELAY_HEATER(_RELEAY_ON);
	SET_RELAY_WATER(_RELEAY_ON);
	SET_RELAY_PUMP(_RELEAY_ON);
	SET_RELAY_DRAINAGE(_RELEAY_ON);
	SET_RELAY_ERROR(_RELEAY_ON);
#endif

	Operation();

    // Force the counter to be placed into memory
    volatile static int i = 0 ;
    // Enter an infinite loop, just incrementing a counter
    while(1) {
        i++ ;
    }
    return 0 ;
}
