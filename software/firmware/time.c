
#include "time.h"
#include "console.h"

// driverlib includes
#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_ints.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"
#include "timer.h"
#include "utils.h"
#include "settings.h"

#define SYSTIMER_BASE TIMERA2_BASE
#define TIMER_PRESCALAR 2

// Globals
systime_t g_iTimer[NUM_TIMERS];
systime_t g_iSysTime;
unsigned long g_iTimerEvent;
#ifdef DEBUG_TIME
int g_iTimePrint;
#endif


// Local Forwards
void _TimerInterruptHandler();

void TimeInit()
{
	int i;

	// Clear all the soft timers
	g_iSysTime = 0;
	for(i=0; i < NUM_TIMERS; i++)
		g_iTimer[i] = NULL_TIME;

	// Clear events
	g_iTimerEvent = 0;

	// Configure SYSTIMER to act as a 16 bit timer with prescalar 2 and
	// overflow every 1mS
	MAP_TimerConfigure(SYSTIMER_BASE, TIMER_CFG_SPLIT_PAIR|TIMER_CFG_B_PWM|TIMER_CFG_A_PERIODIC); // Timer B is used for the LED PWD!
	MAP_TimerLoadSet(SYSTIMER_BASE, TIMER_A, SYSCLK / TIMER_PRESCALAR / 1000);
	MAP_TimerPrescaleSet(SYSTIMER_BASE, TIMER_A, TIMER_PRESCALAR-1);

	// Register and enable the timer interrupt
	MAP_TimerIntRegister(SYSTIMER_BASE, TIMER_A, _TimerInterruptHandler);
	MAP_TimerIntEnable(SYSTIMER_BASE, TIMER_TIMA_TIMEOUT);

	// Enable the timer
	MAP_TimerEnable(SYSTIMER_BASE, TIMER_BOTH);
}

void TimeTask()
{
	int i;
	systime_t now = TimeGetSystime();

	for(i=0; i < NUM_TIMERS; i++) {
		if (now > g_iTimer[i]) {
			// Raise event
			g_iTimerEvent |= (1 << i);

			// Clear the timer
			g_iTimer[i] = NULL_TIME;
		}
	}

#ifdef DEBUG_TIME
	 g_iTimePrint = (g_iTimePrint + 1) % 5000;
	if (g_iTimePrint==0)
		ConsolePrintf("Sys=%d Timer0=%d\n\r", now, g_iTimer[0]);
#endif
}

systime_t TimeGetSystime()
{
	return g_iSysTime;
}

int TimeGetEvent(int timer)
{
	return (g_iTimerEvent & (1 << timer));
}


void TimeSetTimeout(int timer, systime_t period)
{
	// Make sure it's a valid timer
	if (timer >= NUM_TIMERS)
		return;

	// Clear the associated event
	g_iTimerEvent &= ~(1 << timer);

	// Set expiry time
	g_iTimer[timer] = TimeGetSystime() + period;
}

void _TimerInterruptHandler()
{
	TimerIntClear(SYSTIMER_BASE, TIMER_TIMA_TIMEOUT);
	g_iSysTime++;
}
