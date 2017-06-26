
#include "analog.h"
#include "settings.h"
#include "console.h"

// Peripheral lib
#include "hw_memmap.h"
#include "rom_map.h"
#include "adc.h"
#include "utils.h"


#define ADC_FULLSCALE    1460  // ADC Fullscale Voltage [mV]
#define VBAT_DIVIDER     3     // Hardware volated divider between battery input and pin
#define VBAT_AVERAGE_LEN 512

typedef enum {
	HIT_MODE_IDLE,
	HIT_MODE_THRESH,
	HIT_MODE_HOLD
} hit_mode_e;

// Global Variables
static int g_iBatteryLevel;
static int g_iBatteryRaw;
static int g_iThreshold;
static int g_iDebouncePower;

static struct {
	int counter;
	int last_value;
	hit_mode_e mode;
	systime_t timestamp;
} g_tHitDetect;
static int g_iBatteryAcc;
static int g_iBatteryAccCnt;


void AnalogInit()
{
	// Hit detection data
	g_tHitDetect.counter = 0;
	g_tHitDetect.last_value = 0;
	g_tHitDetect.timestamp = NULL_TIME;
	g_tHitDetect.mode = HIT_MODE_IDLE;

	// Battery voltage averaging
	g_iBatteryAcc = 0;
	g_iBatteryAccCnt = 0;

	// Set the threshold & debounce power to the global settings
	AnalogSetThreshold(HIT_THRESHOLD, HIT_DEBOUNCE_B);

	// Enable the PIEZO & VSENSE channels
	MAP_ADCChannelEnable(ADC_BASE, ADC_CHANNEL_PIEZO);
	MAP_ADCChannelEnable(ADC_BASE, ADC_CHANNEL_VSENSE);

	// Enable the ADC block
	MAP_ADCEnable(ADC_BASE);

	/*
	while(1) {
        int level = MAP_ADCFIFOLvlGet(ADC_BASE, ADC_CHANNEL_PIEZO);
        if (level > 0) {
            int value = (MAP_ADCFIFORead(ADC_BASE, ADC_CHANNEL_PIEZO) >> 2) & 0xfff;
            ConsolePrintf("ADC=%d\n\r", value);
            UtilsDelay(1000000);


        }
	}
	*/


}

int g = 0;
void AnalogTask()
{
	// Hit sensor (Piezo)
	/////////////////////
	int level = MAP_ADCFIFOLvlGet(ADC_BASE, ADC_CHANNEL_PIEZO);
	if (level > 0) {
		int value = (MAP_ADCFIFORead(ADC_BASE, ADC_CHANNEL_PIEZO) >> 2) & 0xfff;

#if 0
		g = (g + 1) % 2000;
		if (g == 0)
			ConsolePrintf("ADC=%d\n\r", value);
#endif

		switch(g_tHitDetect.mode) {

		case HIT_MODE_IDLE:
			if (value < g_iThreshold) {
				// Initial hit level threshold has been crossed
				g_tHitDetect.mode = HIT_MODE_HOLD;
				g_tHitDetect.timestamp = TimeGetSystime();
				g_tHitDetect.counter = g_iDebouncePower;
				g_tHitDetect.last_value = value;
				ConsolePrintf("value=%d\n", value);
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
				g_tHitDetect.counter = g_iDebouncePower;
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

	// Battery Level
	////////////////
	level = MAP_ADCFIFOLvlGet(ADC_BASE, ADC_CHANNEL_VSENSE);
	if (level > 0) {
		int value = (MAP_ADCFIFORead(ADC_BASE, ADC_CHANNEL_VSENSE) >> 2) & 0xfff;

		g_iBatteryAcc += value;
		g_iBatteryAccCnt++;

		if (g_iBatteryAccCnt == VBAT_AVERAGE_LEN) {
			// Calculate the battery voltage in mV
			// Note: After calculating, there still remains an unknown factor of 1.33. This can be either
			//       due to imput impedance of the ADC pin or lack of capacitor on that input. Anyway, wer'e
			//       only intersted at "battery low" indication so we just fix the measurment.
			_u16 raw = g_iBatteryAcc/VBAT_AVERAGE_LEN;
			int voltage = ((ADC_FULLSCALE*g_iBatteryAcc/4096)/VBAT_AVERAGE_LEN)*VBAT_DIVIDER*1333/1000;
			g_iBatteryLevel = voltage;
			g_iBatteryRaw = raw;

			g_iBatteryAcc = 0;
			g_iBatteryAccCnt = 0;
		}


		//int z = g_iBatteryAcc/g_iBatteryAccCnt;
		//g = (g + 1) % 2000;
		//if (g == 0)
		//	ConsolePrintf("VBAT=%d RAW=%d \n\r", g_iBatteryLevel, z);
	}

}

systime_t AnalogGetHitTime()
{
	systime_t t = g_tHitDetect.timestamp;
	g_tHitDetect.timestamp = NULL_TIME;

	return t;
}

int AnalogGetBatteryVoltage(_u16* raw)
{
	if (raw)
		*raw = g_iBatteryRaw;
	return g_iBatteryLevel;
}

int AnalogGetBatteryVoltageBlocking()
{
	unsigned int acc = 0;
	int count = 0;

	while (count != VBAT_AVERAGE_LEN) {
		int level = MAP_ADCFIFOLvlGet(ADC_BASE, ADC_CHANNEL_VSENSE);
		if (level > 0) {
			int value = (MAP_ADCFIFORead(ADC_BASE, ADC_CHANNEL_VSENSE) >> 2) & 0xfff;
			acc += value;
			count++;
		}
	}
	return (acc/count);

}

void AnalogSetThreshold(_u16 threshold, _u16 debounce)
{
	if (threshold > 0)
		g_iThreshold = threshold;

	if (debounce > 0)
		g_iDebouncePower = debounce;
}
