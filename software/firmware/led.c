
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

void SetupTimerPWMMode(unsigned long ulBase, unsigned long ulTimer,
                       unsigned long ulConfig, unsigned char ucInvert)
{
    //
    // Set GPT - Configured Timer in PWM mode.
    //
    MAP_TimerConfigure(ulBase,ulConfig);
    MAP_TimerPrescaleSet(ulBase,ulTimer,0);

    //
    // Inverting the timer output if required
    //
    MAP_TimerControlLevel(ulBase,ulTimer,ucInvert);

    //
    // Load value set to ~0.5 ms time period
    //
    MAP_TimerLoadSet(ulBase,ulTimer,TIMER_INTERVAL_RELOAD);

    //
    // Match value set so as to output level 0
    //
    MAP_TimerMatchSet(ulBase,ulTimer,TIMER_INTERVAL_RELOAD);

}

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
	// Green - PWM5
	SetupTimerPWMMode(TIMERA2_BASE, TIMER_B,
			(TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PWM), 1);

	MAP_TimerEnable(TIMERA2_BASE, TIMER_B);

	//  Blue - PWM6
	SetupTimerPWMMode(TIMERA3_BASE, TIMER_A,
	        (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM), 1);

	MAP_TimerEnable(TIMERA3_BASE, TIMER_A);

	// Red - PWM7
	SetupTimerPWMMode(TIMERA3_BASE, TIMER_B,
			(TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PWM), 1);

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
