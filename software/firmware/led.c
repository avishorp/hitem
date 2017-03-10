
#include "led.h"
#include "time.h"

// driverlib includes
#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_ints.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"
#include "timer.h"
#include "utils.h"
#include "gpio.h"

#include <simplelink.h>

#define TIMER_INTERVAL_RELOAD   40035 /* =(255*157) */
#define DUTYCYCLE_GRANULARITY   157

#define LED_GPIO_WRITE(v) GPIOPinWrite(LED_GPIO_BASE, LED_GPIO_MASK, v)
#define LED_GPIO_SET(v) LED_GPIO_WRITE(v*LED_GPIO_MASK)

///// Color Table
typedef struct {
    _u8 r;
    _u8 g;
    _u8 b;
} color_t;

color_t color_table[] = {
  { 0, 0, 0 },             // COLOR_NONE
  { 0xff, 0x00, 0x00 },    // COLOR_RED
  { 0, 0xff, 0 },          // COLOR_GREEN
  { 0, 0, 0xff },          // COLOR_BLUE
  { 250, 60, 0 },          // COLOR_ORANGE
  { 250, 0, 142 },         // COLOR_PURPLE
  { 110, 250, 73 },        // COLOR_LGTGREEN
  { 0, 225, 130 },         // COLOR_TURKIZ
  { 250, 153, 0 },         // COLOR_YELLOW
  { 250, 191, 102 },       // COLOR_WHITE
  { 250, 90, 102 }         // COLOR_PINK
};

#define COLOR_TABLE_LENGTH (sizeof(color_table)/sizeof(color_t))

///// LED Pattern definitions
typedef struct {
	unsigned int color;
	systime_t period;
} pattern_point_t;

typedef struct {
	const pattern_point_t* schedule;
	int length;
	int intensity;
} pattern_t;

#define PLEN(p) (sizeof(p)/sizeof(pattern_point_t))

const pattern_point_t g_tPatternRedBlue[] = {
		{ COLOR_RED, 100 },
		{ COLOR_NONE, 500 },
		{ COLOR_BLUE, 100 },
		{ COLOR_NONE, 500 }
};

const pattern_point_t g_tPatternRedGreen[] = {
		{ COLOR_RED, 100 },
		{ COLOR_NONE, 500 },
		{ COLOR_GREEN, 100 },
		{ COLOR_NONE, 500 }
};

const pattern_point_t g_tPatternRedPulse[] = {
		{ COLOR_RED, 200 },
		{ COLOR_NONE, 200 }
};

const pattern_point_t g_tPatternGreenPulse[] = {
		{ COLOR_GREEN, 200 },
		{ COLOR_NONE, 200 }
};

const pattern_point_t g_tPatternBlimp[] = {
		{ COLOR_ORANGE, 100 },
		{ COLOR_NONE, 1000 },
		{ COLOR_PURPLE, 100 },
		{ COLOR_NONE, 1000 },
		{ COLOR_LGTGREEN, 100 },
		{ COLOR_NONE, 1000 },
		{ COLOR_TURKIZ, 100 },
		{ COLOR_NONE, 1000 },
		{ COLOR_YELLOW, 100 },
		{ COLOR_NONE, 1000 },
		{ COLOR_WHITE, 100 },
		{ COLOR_NONE, 1000 }

};

const pattern_point_t g_tPatternChirp[] = {
		{ COLOR_NONE, 150 },
		{ COLOR_ORANGE, 100 },
		{ COLOR_RED, 100 },
		{ COLOR_PURPLE, 100 },
		{ COLOR_LGTGREEN, 100 },
		{ COLOR_TURKIZ, 100 },
		{ COLOR_YELLOW, 100 },
		{ COLOR_WHITE, 100 },
		{ COLOR_GREEN, 100 },
		{ COLOR_BLUE, 100 },
		{ COLOR_PINK, 100 }
};



const pattern_t g_tLEDPatterns[] = {
        // PATTERN_RED_BLUE
		{
				g_tPatternRedBlue,	      // schedule
				PLEN(g_tPatternRedBlue),  // length
				50                        // intensity
		},
		// PATTERN_RED_GREEN
		{
				g_tPatternRedGreen,	      // schedule
				PLEN(g_tPatternRedGreen), // length
				50                        // intensity
		},
		// PATTERN_RED_PULSE
		{
				g_tPatternRedPulse,	      // schedule
				PLEN(g_tPatternRedPulse), // length
				70                        // intensity
		},
		// PATTERN_GREEN_PULSE
		{
				g_tPatternGreenPulse,	    // schedule
				PLEN(g_tPatternGreenPulse), // length
				70                          // intensity
		},
		// PATTERN_BLIMP
		{
				g_tPatternBlimp,   	        // schedule
				PLEN(g_tPatternBlimp),      // length
				70                          // intensity
		},
		// PATTERN_COLOR_CHIRP
		{
				g_tPatternChirp,   	        // schedule
				PLEN(g_tPatternChirp),      // length
				70                          // intensity
		},

};


// Globals
unsigned int g_iForegroundColor;
int g_iForegroundIntensity;
pattern_t const *g_pCurrentPattern ;
int g_iCurrentPatternPtr;

// Local Forwards
void _LEDSetColorWorker(color_t color, int intensity);

// Transmits raw encoded bits to the LEDs at constant
// rate. The data for each LED comprises of 3*32 bits which
// are shifted out to the data pin. The sequence repeats itself
// n times to set the color of all the LEDs in the chian.
void _LEDTransmit(_u32* raw, int repeat)
{
    // Reset
    LED_GPIO_SET(1);
    UtilsDelay(800);
    LED_GPIO_SET(0);
    UtilsDelay(800);

    int j, k, r;
    _u32 v;
    for(r = 0; r < repeat; r++) {
        for (k=0; k < 3; k++) {
            v = raw[k];

            for(j=0; j < 32; j++) {
                LED_GPIO_WRITE(v);
                v >>= 1;
            }
        }
    }
}

// Converts an RGB triplet to a bitstream and transmits
// it to all LEDs (all LEDs have the same color)
//
// Each bit in each color is converted to 4 bits which are
// shifted out to the DIN input of a WS2812B LED chain. Each
// 4 bit group encodes a single bit in the NRZ encoding scheme
// used by the LEDs
void _LEDSendData(_u8 r, _u8 g, _u8 b, int n)
{
    _u32 raw[3];
    _u32 grb = (g << 16) + (r << 8) + b;

    int j, k;
    for(k=0; k < 3; k++) {
        raw[k] = 0;

        for(j = 0; j < 8; j++) {
            if (grb & 0x800000)
                // 1
                raw[k] |= (0b0111) << (j*4);
            else
                // 0
                raw[k] |= (0b0001) << (j*4);

            grb <<= 1;
        }

    }

    _LEDTransmit(raw, n);
}

void _LEDSetColorImpl(unsigned int index, int intensity)
{
    if (index > (COLOR_TABLE_LENGTH-1))
        // Invalid color
        return;

    // Calculate the PWM values
    int red = color_table[index].r * intensity / 100;
    int green = color_table[index].g * intensity / 100;
    int blue = color_table[index].b * intensity / 100;

    _LEDSendData(red, green, blue, 8);
}


void LEDInit()
{
    LED_GPIO_SET(1);
    UtilsDelay(1000);

    int i = 0;
}


void LEDSetColor(unsigned int color, int intensity)
{
	// Set the color
	g_iForegroundColor = color;
	g_iForegroundIntensity = intensity;

	// Disable possibly pending pattern
	g_pCurrentPattern = 0;

	// Set the actual LED drive
	_LEDSetColorImpl(color, intensity);
}

void LEDSetPattern(int pattern)
{
	// Set the current pattern pointer
	g_pCurrentPattern = &g_tLEDPatterns[pattern];
	g_iCurrentPatternPtr = 0;

	// Apply the first point
	_LEDSetColorImpl(g_pCurrentPattern->schedule[g_iCurrentPatternPtr].color, g_pCurrentPattern->intensity);
	TimeSetTimeout(0, g_pCurrentPattern->schedule[g_iCurrentPatternPtr].period);
}

void LEDCriticalSignal(int len)
{
	int i;
   	for(i=0; i < len; i++) {
    	  	_LEDSetColorImpl(COLOR_RED, 20);
    	    UtilsDelay(1000000);
    	  	_LEDSetColorImpl(COLOR_NONE, 20);
    	    UtilsDelay(1000000);
   	}
}

void LEDTask()
{
	// If a current pattern is active
	if (g_pCurrentPattern) {
		// Check if a timer event has occured
		if (TimeGetEvent(0)) {
			g_iCurrentPatternPtr++;
			if (g_iCurrentPatternPtr >= g_pCurrentPattern->length) {
				// End of pattern schedure
					g_iCurrentPatternPtr = 0;
			}
			// Apply the next point
			_LEDSetColorImpl(g_pCurrentPattern->schedule[g_iCurrentPatternPtr].color, g_pCurrentPattern->intensity);
			TimeSetTimeout(0, g_pCurrentPattern->schedule[g_iCurrentPatternPtr].period);
		}
	}
}

