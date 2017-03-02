
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

#define LED_GPIO_SET(v) GPIOPinWrite(LED_GPIO_BASE, LED_GPIO_MASK, v*LED_GPIO_MASK)

///// LED Pattern definitions
typedef struct {
	color_t color;
	systime_t period;
} pattern_point_t;

typedef struct {
	const pattern_point_t* schedule;
	int length;
	int repetitive;
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
				1,                        // repetitive
				50                        // intensity
		},
		// PATTERN_RED_GREEN
		{
				g_tPatternRedGreen,	      // schedule
				PLEN(g_tPatternRedGreen), // length
				1,                        // repetitive
				50                        // intensity
		},
		// PATTERN_RED_PULSE
		{
				g_tPatternRedPulse,	      // schedule
				PLEN(g_tPatternRedPulse), // length
				1,                        // repetitive
				70                        // intensity
		},
		// PATTERN_GREEN_PULSE
		{
				g_tPatternGreenPulse,	    // schedule
				PLEN(g_tPatternGreenPulse), // length
				1,                          // repetitive
				70                          // intensity
		},
		// PATTERN_BLIMP
		{
				g_tPatternBlimp,   	        // schedule
				PLEN(g_tPatternBlimp),      // length
				1,                          // repetitive
				70                          // intensity
		},
		// PATTERN_COLOR_CHIRP
		{
				g_tPatternChirp,   	        // schedule
				PLEN(g_tPatternChirp),      // length
				1,                          // repetitive
				70                          // intensity
		},

};


// Globals
color_t g_iForegroundColor;
int g_iForegroundIntensity;
pattern_t const *g_pCurrentPattern ;
int g_iCurrentPatternPtr;

// Local Forwards
void _LEDSetColorWorker(color_t color, int intensity);
void _UpdateDutyCycle(unsigned long ulBase, unsigned long ulTimer,
                     unsigned char ucLevel);


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
                GPIOPinWrite(0x40007000, 0x1, v & 0x01);
                v >>= 1;
            }
        }
    }
}

void _LEDPrepareData(_u8 r, _u8 g, _u8 b)
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

    _LEDTransmit(raw, 8);
}


void LEDInit()
{
    LED_GPIO_SET(1);
    UtilsDelay(1000);

    int i = 0;
    while(1) {

        _LEDPrepareData((i & 0x03)*6, ((i & 0x0c) >> 2)*6, ((i & 0x30) >> 4) *6);
        //_LEDPrepareData((i & 0x01)*6, ((i & 0x02) >> 1)*6, ((i & 0x04) >> 2) *6);

        i++;

        if ((i&0x07)==1)
            UtilsDelay(3000000*5);

    UtilsDelay(3000000*5);

    }

}


void LEDSetColor(color_t color, int intensity)
{
	// Set the color
	g_iForegroundColor = color;
	g_iForegroundIntensity = intensity;

	// Disable possibly pending pattern
	g_pCurrentPattern = 0;

	// Set the actual LED drive
	_LEDSetColorWorker(color, intensity);
}

void LEDSetPattern(int pattern)
{
	// Set the current pattern pointer
	g_pCurrentPattern = &g_tLEDPatterns[pattern];
	g_iCurrentPatternPtr = 0;

	// Apply the first point
	_LEDSetColorWorker(g_pCurrentPattern->schedule[g_iCurrentPatternPtr].color, g_pCurrentPattern->intensity);
	TimeSetTimeout(0, g_pCurrentPattern->schedule[g_iCurrentPatternPtr].period);
}

void LEDCriticalSignal(int len)
{
	int i;
   	for(i=0; i < len; i++) {
    	  	LEDSetColor(COLOR_RED, 20);
    	    UtilsDelay(1000000);
    	  	LEDSetColor(COLOR_NONE, 20);
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
				if (g_pCurrentPattern->repetitive)
					// Repetitive pattern - replay
					g_iCurrentPatternPtr = 0;
				else {
					// Non-repetitive pattern - set to foreground color
					_LEDSetColorWorker(g_iForegroundColor, g_iForegroundIntensity);
					return;
				}
			}

			// Apply the next point
			_LEDSetColorWorker(g_pCurrentPattern->schedule[g_iCurrentPatternPtr].color, g_pCurrentPattern->intensity);
			TimeSetTimeout(0, g_pCurrentPattern->schedule[g_iCurrentPatternPtr].period);
		}
	}
}

void _LEDSetColorWorker(color_t color, int intensity)
{
	// Calculate the PWM values
	int red = (color & 0xff) * intensity / 100;
	int green = ((color >> 8) & 0xff) * intensity / 100;
	int blue = ((color >> 16) & 0xff) * intensity / 100;

	// Set the timers
    _UpdateDutyCycle(TIMERA2_BASE, TIMER_B, green);
    _UpdateDutyCycle(TIMERA3_BASE, TIMER_A, blue);
    _UpdateDutyCycle(TIMERA3_BASE, TIMER_B, red);
}

void _UpdateDutyCycle(unsigned long ulBase, unsigned long ulTimer,
                     unsigned char ucLevel)
{
    //
    // Match value is updated to reflect the new dutycycle settings
    //
    MAP_TimerMatchSet(ulBase,ulTimer,(ucLevel*DUTYCYCLE_GRANULARITY));
}
