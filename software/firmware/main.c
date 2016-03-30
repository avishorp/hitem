// CC3200 Basic CCS setup - main file
//
// License: GPL v3

#include <stdlib.h>
#include <string.h>

// Simplelink includes
#include "simplelink.h"

// driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"
#include "utils.h"
#include "gpio.h"
#include "timer.h"

#include "pinmux/pin_mux_config.h"
#include "hw_memmap.h"

#include "console.h"
#include "led.h"

extern void (* const g_pfnVectors[])(void);

// Local Forwards
static void BoardInit(void);
void MainLoop();


#define WELCOME_MESSAGE ">>>> hit'em! <<<<\n\r"

int main(void) {
    // Board Initialization
    BoardInit();

    // Configure the pinmux settings for the peripherals exercised
    PinMuxConfig();

    // Initialize the console output
    ConsoleInit();
    ConsolePrint(WELCOME_MESSAGE);

    // Initialize the leds
    LEDInit();

    // LED test
    LEDSetColor(COLOR_RED, 70);
    UtilsDelay(3000000);
    LEDSetColor(COLOR_GREEN, 70);
    UtilsDelay(3000000);
    LEDSetColor(COLOR_BLUE, 70);
    UtilsDelay(3000000);
    LEDSetColor(COLOR_NONE, 70);

    // Initialize SimpleLink
    long lRet = -1;
    lRet = sl_Start(0, 0, 0);
    if (lRet) {
    	ConsolePrint("SimpleLink initialized\n\r");
    }
    else {
    	ConsolePrint("! [FATAL] SimpleLink initialization failed\n\r");
    	LEDSetColor(COLOR_RED, 40);

    	// If the initialization failed, we have no point doing anything else!
    	while(1);
    }

    // All initialization done! Start running.
    MainLoop();
}

void MainLoop() {
	while(1);
}

static void BoardInit(void)
{
	// Initialize interrupt table (see startup_ccs.c)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);

    // Enable Processor
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}

