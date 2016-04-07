
#include "analog.h"
#include "settings.h"
#include "console.h"

// Peripheral lib
#include "hw_memmap.h"
#include "rom_map.h"
#include "adc.h"


// Global Variables
static int g_iBatteryLevel;
static struct {
	int counter;
	int mode;
	systime_t timestamp;
} g_tHitDetect;


void AnalogInit()
{
	// Hit detection data
	g_tHitDetect.counter = 0;
	g_tHitDetect.timestamp = NULL_TIME;
	g_tHitDetect.mode = 0;

	// Enable the PIEZO & VSENSE channels
	MAP_ADCChannelEnable(ADC_BASE, ADC_CHANNEL_PIEZO);

	// Enable the ADC block
	MAP_ADCEnable(ADC_BASE);

}

void AnalogTask()
{
	// Hit sensor (Piezo)
	/////////////////////
	int level = MAP_ADCFIFOLvlGet(ADC_BASE, ADC_CHANNEL_PIEZO);
	if (level > 0) {
		int value = (MAP_ADCFIFORead(ADC_BASE, ADC_CHANNEL_PIEZO) >> 2);
		if (value > HIT_THRESHOLD) {
			if (g_tHitDetect.mode == 0) {
				g_tHitDetect.timestamp = TimeGetSystime();
				g_tHitDetect.mode = 1;
				g_tHitDetect.counter = HIT_DEBOUNCE_B;
				ConsolePrint("hit ");
			}
		}
		else {
			if (g_tHitDetect.mode == 1) {
				if (g_tHitDetect.counter > 0)
					g_tHitDetect.counter--;

				if (g_tHitDetect.counter == 0) {
					g_tHitDetect.mode = 0;
				}
			}
		}
	}
}

systime_t AnalogGetHitTime()
{
	systime_t t = g_tHitDetect.timestamp;
	g_tHitDetect.timestamp = NULL_TIME;

	return t;
}
