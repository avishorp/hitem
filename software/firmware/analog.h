// ADC related functions (hit detect & battery)

#ifndef __ANALOG_H__
#define __ANALOG_H__

#include "time.h"

void AnalogInit();
void AnalogTask();
systime_t AnalogGetHitTime();
int AnalogGetBatteryVoltage();

#endif
