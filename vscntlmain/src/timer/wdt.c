/*
 * wdt.c
 *
 *  Created on: 2016/05/18
 *      Author: toshio
 */

#include "wdt.h"
#include "datarom.h"

uint32_t	WDT_CurrentCount = 0;
uint16_t	ReBoot_Count = 0;
uint8_t		Reboot_Status = 0;

void WDT_IRQHandler(void)
{
	uint32_t wdtStatus = Chip_WWDT_GetStatus(LPC_WWDT);

	if (wdtStatus & WWDT_WDMOD_WDTOF) {
		while(Chip_WWDT_GetStatus(LPC_WWDT) & WWDT_WDMOD_WDTOF) {
			Chip_WWDT_ClearStatusFlag(LPC_WWDT, WWDT_WDMOD_WDTOF);
		}
		Chip_WWDT_Start(LPC_WWDT);	/* Needs restart */
	}
}

void WDT_Start(uint32_t time)
{
	NVIC_EnableIRQ(WDT_IRQn);

	Chip_WWDT_Init(LPC_WWDT);
	/*
	 * ソースはIRC(12MHz)
	 */
	Chip_Clock_SetWDTClockSource(SYSCTL_CLKOUTSRC_IRC, 4);

	/*
	 * WDはデフォで1/4プリスケーラ。クロックソースで1/4してるから結局12MHzの1/16
	 */
	uint32_t clk = SYSCTL_IRC_FREQ / 16;

	/*
	 * WDTのタイムアウト値設定
	 */
	Chip_WWDT_SetTimeOut(LPC_WWDT, clk * time);

	/*
	 * モード設定
	 */
	Chip_WWDT_SetOption(LPC_WWDT, WWDT_WDMOD_WDEN | WWDT_WDMOD_WDRESET);

	/*
	 * スタート
	 */
	Chip_WWDT_Feed(LPC_WWDT);


}

void WDT_Kick(void)
{
	WDT_CurrentCount = Chip_WWDT_GetCurrentCount(LPC_WWDT);
	Chip_WWDT_Feed(LPC_WWDT);
}

/*
 * ReBoot ステータス取得
 */
void BootDataManage(void)
{
	//uint32_t www ;

	Reboot_Status = Chip_SYSCTL_GetSystemRSTStatus();
	if (Reboot_Status & SYSCTL_RST_WDT) {
		/*
		 * WDT On Boot!!
		 */
		Chip_SYSCTL_ClearSystemRSTStatus(SYSCTL_RST_WDT);
		/*
		 * カウントアップ
		 */
		ReBoot_Count = GetCurrentRebootCount();
		ReBoot_Count++;
		SetCurrentRebootCount(ReBoot_Count);
	}
	else {
		/*
		 * Power On Boot!!
		 */
		ReBoot_Count = 0;
		SetCurrentRebootCount(ReBoot_Count);
	}
}
