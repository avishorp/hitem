// ADC related functions (hit detect & battery)

#ifndef __ANALOG_H__
#define __ANALOG_H__

#include "time.h"
#include <simplelink.h>

void AnalogInit();
void AnalogTask();
systime_t AnalogGetHitTime();
int AnalogGetBatteryVoltage(_u16* raw);
int AnalogGetBatteryVoltageBlocking();
void AnalogSetThreshold(_u16 threshold, _u16 debounce);



#endif
