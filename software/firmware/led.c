
#include "led.h"

// driverlib includes
#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_ints.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"
#include "timer.h"

#define TIMER_INTERVAL_RELOAD   40035 /* =(255*157) */
#define DUTYCYCLE_GRANULARITY   157

void UpdateDutyCycle(unsigned long ulBase, unsigned long ulTimer,
                     unsigned char ucLevel)
{
    //
    // Match value is updated to reflect the new dutycycle settings
    //
    MAP_TimerMatchSet(ulBase,ulTimer,(ucLevel*DUTYCYCLE_GRANULARITY));
}


void LEDInit()
{
	// Green - PWM5 (Timer2/B)
    MAP_TimerConfigure(TIMERA2_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PWM));
    MAP_TimerPrescaleSet(TIMERA2_BASE, TIMER_B, 0);
    MAP_TimerLoadSet(TIMERA2_BASE, TIMER_B, TIMER_INTERVAL_RELOAD);
    MAP_TimerMatchSet(TIMERA2_BASE, TIMER_B, TIMER_INTERVAL_RELOAD);
    MAP_TimerControlLevel(TIMERA2_BASE, TIMER_B, 1);

	MAP_TimerEnable(TIMERA2_BASE, TIMER_B);

	//  Blue - PWM6 (Timer3/A) & Red - PWM7 (Timer3/B)
    MAP_TimerConfigure(TIMERA3_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PWM | TIMER_CFG_A_PWM));
    MAP_TimerPrescaleSet(TIMERA3_BASE, TIMER_A, 0);
    MAP_TimerPrescaleSet(TIMERA3_BASE, TIMER_B, 0);
    MAP_TimerLoadSet(TIMERA3_BASE, TIMER_A, TIMER_INTERVAL_RELOAD);
    MAP_TimerLoadSet(TIMERA3_BASE, TIMER_B, TIMER_INTERVAL_RELOAD);
    MAP_TimerMatchSet(TIMERA3_BASE, TIMER_A, TIMER_INTERVAL_RELOAD);
    MAP_TimerMatchSet(TIMERA3_BASE, TIMER_B, TIMER_INTERVAL_RELOAD);
    MAP_TimerControlLevel(TIMERA3_BASE, TIMER_A, 1);
    MAP_TimerControlLevel(TIMERA3_BASE, TIMER_B, 1);


	MAP_TimerEnable(TIMERA3_BASE, TIMER_A);
	MAP_TimerEnable(TIMERA3_BASE, TIMER_B);
}

void LEDSetColor(int color, int intensity)
{
	// Calculate the PWM values
	int red = (color & 0xff) * intensity / 100;
	int green = ((color >> 8) & 0xff) * intensity / 100;
	int blue = ((color >> 16) & 0xff) * intensity / 100;

	// Set the timers
    UpdateDutyCycle(TIMERA2_BASE, TIMER_B, green);
    UpdateDutyCycle(TIMERA3_BASE, TIMER_A, blue);
    UpdateDutyCycle(TIMERA3_BASE, TIMER_B, red);
}

