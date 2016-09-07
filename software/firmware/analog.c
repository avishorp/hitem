
#include "analog.h"
#include "settings.h"
#include "console.h"

// Peripheral lib
#include "hw_memmap.h"
#include "rom_map.h"
#include "adc.h"

typedef enum {
	HIT_MODE_IDLE,
	HIT_MODE_THRESH,
	HIT_MODE_HOLD
} hit_mode_e;

// Global Variables
static int g_iBatteryLevel;
static struct {
	int counter;
	int last_value;
	hit_mode_e mode;
	systime_t timestamp;
} g_tHitDetect;


void AnalogInit()
{
	// Hit detection data
	g_tHitDetect.counter = 0;
	g_tHitDetect.last_value = 0;
	g_tHitDetect.timestamp = NULL_TIME;
	g_tHitDetect.mode = HIT_MODE_IDLE;

	// Enable the PIEZO & VSENSE channels
	MAP_ADCChannelEnable(ADC_BASE, ADC_CHANNEL_PIEZO);

	// Enable the ADC block
	MAP_ADCEnable(ADC_BASE);

}

int g;
void AnalogTask()
{
	// Hit sensor (Piezo)
	/////////////////////
	int level = MAP_ADCFIFOLvlGet(ADC_BASE, ADC_CHANNEL_PIEZO);
	if (level > 0) {
		int value = (MAP_ADCFIFORead(ADC_BASE, ADC_CHANNEL_PIEZO) >> 2);

#if 0
		g = (g + 1) % 2000;
		if (g == 0)
			ConsolePrintf("ADC=%d\n\r", value);
#endif

		switch(g_tHitDetect.mode) {

		case HIT_MODE_IDLE:
			if (value > HIT_THRESHOLD) {
				// Initial hit level threshold has been crossed
				g_tHitDetect.mode = HIT_MODE_THRESH;
				g_tHitDetect.last_value = value;
			}
			break;

		case HIT_MODE_THRESH:
			if (value > g_tHitDetect.last_value)
				// The hit pulse is still rising
				g_tHitDetect.last_value = value;
			else {
				// The received pulse is smaller than the previously
				// measured value, the pulse has passed its peak
				g_tHitDetect.mode = HIT_MODE_HOLD;
				g_tHitDetect.timestamp = TimeGetSystime();
				g_tHitDetect.counter = HIT_DEBOUNCE_B;
			}
			break;

		case HIT_MODE_HOLD:
			// While in HOLD mode, we're not detecting additional hit pulses
			g_tHitDetect.counter--;
			if (g_tHitDetect.counter == 0)
				// Hold time done
				g_tHitDetect.mode = HIT_MODE_IDLE;
			break;
		}

	}
}

systime_t AnalogGetHitTime()
{
	systime_t t = g_tHitDetect.timestamp;
	g_tHitDetect.timestamp = NULL_TIME;

	return t;
}
