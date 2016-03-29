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

#include "pinmux/pin_mux_config.h"
#include "hw_memmap.h"

#include "console.h"

extern void (* const g_pfnVectors[])(void);

#ifdef STRESS
#define MESSAGE "Thie is a very very very very very very very very very very very very very very very very very " \
		 	    "very very very very very very very very very very very very very very very very very very very " \
		 	    "very very very very very very very very very very very very very very very very very very very " \
				"long message"
#else
#define MESSAGE "Hello CC3200 world!\n\r"
#endif

static void BoardInit(void)
{
	// Initialize interrupt table (see startup_ccs.c)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);

    // Enable Processor
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}

int main(void) {
    // Board Initialization
    BoardInit();

    // Configure the pinmux settings for the peripherals exercised
    PinMuxConfig();

    // Initialize the console output
    ConsoleInit();


   while(1) {
	   int i;

	   ConsolePrint(MESSAGE);
	   GPIOPinWrite(GPIOA0_BASE, 0x01, 1);
	   for(i=0; i < 10000000; i++);
	   GPIOPinWrite(GPIOA0_BASE, 0x01, 0);
	   for(i=0; i < 5000000; i++);
   }

}
