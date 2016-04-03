
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "error.h"
#include "console.h"
#include "led.h"

#include "utils.h"

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
    while(1) {
    	LEDSetColor(COLOR_RED, 40);
        UtilsDelay(5000000);
    	LEDSetColor(COLOR_NONE, 40);
        UtilsDelay(5000000);
    }
}

