
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

// Local includes
#include "error.h"
#include "console.h"
#include "led.h"

// Peripheral Lib includes
#include "hw_types.h"
#include "hw_memmap.h"
#include "rom_map.h"
#include "utils.h"
#include "prcm.h"
#include "adc.h"

// Simplelink includes
#include "simplelink.h"

void FatalError(const char *pcFormat, ...)
{
	char umsg[128];
	char emsg[128];

	va_list list;

	// Format the user message
    va_start(list,pcFormat);
    vsnprintf(umsg, sizeof(umsg), pcFormat, list);
    va_end(list);

    // Add standatd header
    snprintf(emsg, sizeof(emsg), "!!! [FATAL] %s\n\r", umsg);

    // Print it
	ConsolePrint(emsg);

    // Stop the processing and blink the red LED
	int i;
   for(i=0; i < 30; i++) {
    	LEDSetColor(COLOR_RED, 40);
        UtilsDelay(5000000);
    	LEDSetColor(COLOR_NONE, 40);
        UtilsDelay(5000000);
    }

   // Go to sleep
   DoSleep();
}

void DoSleep()
{
	int i;
	for(i = 100; i > 0; i -= 10) {
		LEDSetColor(COLOR_RED, i);
	    UtilsDelay(1500000);
	}
	LEDSetColor(COLOR_NONE, 0);

	// Stop SimpLink
	sl_Stop(0);

	// Disable the ADC (it interferes with the wake up GPIO)
	MAP_ADCDisable(ADC_BASE);


	// Set-up GPIO pin 4 as a wake-up source
    PRCMHibernateWakeupSourceEnable(PRCM_HIB_GPIO4);
    PRCMHibernateWakeUpGPIOSelect(PRCM_HIB_GPIO4, PRCM_HIB_RISE_EDGE);

    ConsolePrintf("Going to sleep...\n\r");
    UtilsDelay(100000);

	// This is the place from which no CC3200 has ever returned
    PRCMHibernateEnter();
}
