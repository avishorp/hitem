// Application Settings

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

// System Clock [Hz]
#define SYSCLK               80000000

// Console Settings
#define CONSOLE_BAUD_RATE    115200
#define CONSOLE              UARTA0_BASE
#define CONSOLE_PERIPH       PRCM_UARTA0
#define CONSOLE_BUFFER_SIZE  1024
#define CONSOLE_TRUNCATE_SYM '$'

#define ADC_CHANNEL_PIEZO    ADC_CH_2
#define ADC_CHANNEL_VSENSE   ADC_CH_3

// The sensitivity to hit
#define HIT_THRESHOLD 2400
#define HIT_DEBOUNCE_A  2
#define HIT_DEBOUNCE_B  500


#define DISCOVERY_TIMEOUT     1000  // The period, in mS, in which repeated discovery
								    // messages are sent
#define WLAN_TIMEOUT          40000 // The time, in mS, the unit waits for WLAN connection

#define BATTERY_REPORT_PERIOD 5000  // The period (in mS) between battery level updates

//#define NO_UDP_SYNC_REQ

// Define to enable report messages to be sent over UDP
#define DEBUG_UDP_PORT  24600

#endif

