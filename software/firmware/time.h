// Timing functions

#ifndef __TIME_H__
#define __TIME_H__

// Number of independent soft timers
#define NUM_TIMERS 2

typedef unsigned long systime_t;

#define NULL_TIME ((systime_t)(-1))

// Initialize the timing system
void TimeInit();
void TimeTask();
systime_t TimeGetSystime();
int TimeGetEvent(int timer);
void TimeSetTimeout(int timer, systime_t period);

#endif
