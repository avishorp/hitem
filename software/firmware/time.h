// Timing functions

#ifndef __TIME_H__
#define __TIME_H__

// Number of independent soft timers
#define NUM_TIMERS 2

typedef unsigned long systime_t;

// Initialize the timing system
void TimeInit();
void TimeTask();
systime_t TimeGetSystime();
int TimeGetEvent(int timer);
void TimeSetTimeout(int timer, systime_t period);

#endif
