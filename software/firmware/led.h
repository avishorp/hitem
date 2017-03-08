// LED control

#define LED_GPIO_BASE GPIOA3_BASE
#define LED_GPIO_MASK 0x1

// Predefined colors
#define COLOR_NONE      0
#define COLOR_RED		1
#define COLOR_GREEN     2
#define COLOR_BLUE      3
#define COLOR_ORANGE    4
#define COLOR_PURPLE    5
#define COLOR_LGTGREEN  6
#define COLOR_TURKIZ    7
#define COLOR_YELLOW    8
#define COLOR_WHITE     9
#define COLOR_PINK      10

#define NUM_COLORS 11


// Predefined patterns
#define PATTERN_RED_BLUE    0  // Red-Blue blinking in 0.5s period
#define PATTERN_RED_GREEN   1  // Red-Green blinking in 0.5s period
#define PATTERN_RED_PULSE   2  // Two short pulses of red
#define PATTERN_GREEN_PULSE 3  // Two short pulses of green
#define PATTERN_BLIMP       4  // Multiple short pulses of various colors, with spacing
#define PATTERN_COLOR_CHIRP 5  // 1-sec roll of multiple colors

void LEDInit();
void LEDTask();
void LEDSetColor(unsigned int index, int intensity);
void LEDSetPattern(int pattern);
void LEDCriticalSignal(int len);
