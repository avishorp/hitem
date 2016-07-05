// CC3200 Basic CCS setup - main file
//
// License: GPL v3

#include <stdlib.h>
#include <stdio.h>
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
#include "uart.h"
#include "analog.h"

#include "pinmux/pin_mux_config.h"
#include "hw_memmap.h"

#include "console.h"
#include "led.h"
#include "config.h"
#include "mainloop.h"
#include "time.h"
#include "settings.h"
#include "gpio.h"
#include "adc.h"
#include "protocol.h"
#include "error.h"

extern void (* const g_pfnVectors[])(void);

// Local Forwards
static void BoardInit();
void SimpleLinkInit();
void MainLoop();
void ADCInit();
void ADCTask();
int HitGetEvent();


#ifdef LED_ADJUST_TEST
void __LEDAdjust();
#endif


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

    // Initialize timing system
    TimeInit();

    // Initialize the protocol handler
    ProtocolInit();

    // Initialize ADC Module
    AnalogInit();

    // LED test
    LEDSetColor(COLOR_RED, 70);
    UtilsDelay(3000000);
    LEDSetColor(COLOR_GREEN, 70);
    UtilsDelay(3000000);
    LEDSetColor(COLOR_BLUE, 70);
    UtilsDelay(3000000);
    LEDSetColor(COLOR_NONE, 70);

    // Initialize SimpleLink
    SimpleLinkInit();

    // Load configuration
    ConfigInit();

    // All initialization done! Start running.
    MainLoopInit(ConfigGet());

    // Start the main application loop
#ifdef LED_ADJUST_TEST
    __LEDAdjust();
#endif
	MainLoop();

}

void MainLoop() {
	while(1) {
		// Cooperative task invocation
		MainLoopExec();
		sl_Task();
		TimeTask();
		LEDTask();
		AnalogTask();

#ifdef DEBUG_PRINT_ADC
		zz = (zz + 1) % 3000;
		if (zz == 0) {
			int level = ADCFIFOLvlGet(ADC_BASE, ADC_CHANNEL_PIEZO);
			int value = -1;
			if (level > 0)
				value = (ADCFIFORead(ADC_BASE, ADC_CHANNEL_PIEZO) >> 2);
			ConsolePrintf("ADC lvl=%d val=%d\n\r", level, value);
		}
#endif

	}
}

static void BoardInit()
{
	// Initialize interrupt table (see startup_ccs.c)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);

    // Enable Processor
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}

// Initialize SimpleLink and reset its state
//
// - Set the mode to STATION
// - Configures connection policy to Auto and AutoSmartConfig
// - Deletes all the stored profiles
// - Enables DHCP
// - Disables Scan policy
// - Sets Tx power to maximum
// - Sets power policy to normal
// - Unregister mDNS services
// - Remove all filters
void SimpleLinkInit()
{
    SlVersionFull   ver = {0};
    _WlanRxFilterOperationCommandBuff_t  RxFilterIdMask = {0};

    unsigned char ucVal = 1;
    unsigned char ucConfigOpt = 0;
    unsigned char ucConfigLen = 0;
    unsigned char ucPower = 0;

    long lRetVal = -1;
    long lMode = -1;

    lMode = sl_Start(0, 0, 0);
    if (lMode < 0)
    	FatalError("sl_Start");

    // If the device is not in station-mode, try configuring it in station-mode
    if (ROLE_STA != lMode)
    {
        if (ROLE_AP == lMode)
        {
            // If the device is in AP mode, we need to wait for this event
            // before doing anything
        	// TODO: Check if needed
//            while(!IS_IP_ACQUIRED(g_ulStatus))
//            {
//            }
        }

        // Switch to STA role and restart
        lRetVal = sl_WlanSetMode(ROLE_STA);
        if (lRetVal < 0)
        	FatalError("sl_WlanSetMode");

        lRetVal = sl_Stop(0xFF);
        if (lRetVal < 0)
        	FatalError("sl_Stop");

        lRetVal = sl_Start(0, 0, 0);
        if (lRetVal < 0)
        	FatalError("sl_Start");

        // Check if the device is in station again
        if (ROLE_STA != lRetVal)
        {
            // We don't want to proceed if the device is not coming up in STA-mode
        	FatalError("Faild setting the device mode to STA");
        }
    }



    // Get the device's version-information
    ucConfigOpt = SL_DEVICE_GENERAL_VERSION;
    ucConfigLen = sizeof(ver);
    lRetVal = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &ucConfigOpt,
                                &ucConfigLen, (unsigned char *)(&ver));
    if (lRetVal < 0)
    	FatalError("Could not get device version");

    ConsolePrintf("Host Driver Version: %s\n\r", SL_DRIVER_VERSION);
    ConsolePrintf("Build Version %d.%d.%d.%d.31.%d.%d.%d.%d.%d.%d.%d.%d\n\r",
    ver.NwpVersion[0],ver.NwpVersion[1],ver.NwpVersion[2],ver.NwpVersion[3],
    ver.ChipFwAndPhyVersion.FwVersion[0],ver.ChipFwAndPhyVersion.FwVersion[1],
    ver.ChipFwAndPhyVersion.FwVersion[2],ver.ChipFwAndPhyVersion.FwVersion[3],
    ver.ChipFwAndPhyVersion.PhyVersion[0],ver.ChipFwAndPhyVersion.PhyVersion[1],
    ver.ChipFwAndPhyVersion.PhyVersion[2],ver.ChipFwAndPhyVersion.PhyVersion[3]);

    // Set connection policy to Auto + SmartConfig
    //      (Device's default connection policy)
    lRetVal = sl_WlanPolicySet(SL_POLICY_CONNECTION,
                                SL_CONNECTION_POLICY(1, 0, 0, 0, 1), NULL, 0);
    if (lRetVal < 0)
    	FatalError("sl_WlanPolicySet");

    // Remove all profiles
    lRetVal = sl_WlanProfileDel(0xFF);
    if (lRetVal < 0)
    	FatalError("sl_WlanProfileDel");
/*
    //
    // Device in station-mode. Disconnect previous connection if any
    // The function returns 0 if 'Disconnected done', negative number if already
    // disconnected Wait for 'disconnection' event if 0 is returned, Ignore
    // other return-codes
    //
    lRetVal = sl_WlanDisconnect();
    if(0 == lRetVal)
    {
        // Wait
        while(IS_CONNECTED(g_ulStatus))
        {
#ifndef SL_PLATFORM_MULTI_THREADED
              _SlNonOsMainLoopTask();
#endif
        }
    }
*/
    // Enable DHCP client
    lRetVal = sl_NetCfgSet(SL_IPV4_STA_P2P_CL_DHCP_ENABLE,1,1,&ucVal);
    if (lRetVal < 0)
    	FatalError("sl_NetCfgSet");

    // Disable scan
    ucConfigOpt = SL_SCAN_POLICY(0);
    lRetVal = sl_WlanPolicySet(SL_POLICY_SCAN , ucConfigOpt, NULL, 0);
    if (lRetVal < 0)
    	FatalError("sl_WlanPolicySet");

    // Set Tx power level for station mode
    // Number between 0-15, as dB offset from max power - 0 will set max power
    ucPower = 0;
    lRetVal = sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID,
            WLAN_GENERAL_PARAM_OPT_STA_TX_POWER, 1, (unsigned char *)&ucPower);
    if (lRetVal < 0)
    	FatalError("sl_WlanSet");

    // Set PM policy to normal
    lRetVal = sl_WlanPolicySet(SL_POLICY_PM , SL_NORMAL_POLICY, NULL, 0);
    if (lRetVal < 0)
    	FatalError("sl_WlanPolicySet");

    // Unregister mDNS services
    lRetVal = sl_NetAppMDNSUnRegisterService(0, 0);
    if (lRetVal < 0)
    	FatalError("sl_NetAppMDNSUnRegisterService");

    // Remove  all 64 filters (8*8)
    memset(RxFilterIdMask.FilterIdMask, 0xFF, 8);
    lRetVal = sl_WlanRxFilterSet(SL_REMOVE_RX_FILTER, (_u8 *)&RxFilterIdMask,
                       sizeof(_WlanRxFilterOperationCommandBuff_t));
    if (lRetVal < 0)
    	FatalError("sl_WlanRxFilterSet");

    ConsolePrint("SimpleLink Initialized successfully\n\r");

}



#ifdef LED_ADJUST_TEST
void __LEDAdjust()
{
	int r, g, b;
	char ch;
	r = 0;
	g = 0;
	b = 0;
	ConsolePrint("**** LED Color Adjustment ****\n\r");
	ConsolePrint("R - Q (+) A (-)\n\r");
	ConsolePrint("G - W (+) S (-)\n\r");
	ConsolePrint("B - E (+) D (-)\n\r\n\r");

	while(1) {
		LEDSetColor(RGB(r, g, b), 100);
		ConsolePrintf("R=%d G=%d B=%d\n\r", r, g, b);

		ch = MAP_UARTCharGet(CONSOLE);
		if (ch == 'q') r += 5;
		if (ch == 'a') r -= 5;
		if (ch == 'w') g += 5;
		if (ch == 's') g -= 5;
		if (ch == 'e') b += 5;
		if (ch == 'd') b -= 5;

	}

}
#endif

