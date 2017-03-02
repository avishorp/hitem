// LED control

typedef unsigned long color_t;

#define LED_GPIO_BASE GPIOA3_BASE
#define LED_GPIO_MASK 0x1

#define RGB(r, g, b) (r + (g << 8) + (b << 16))

// Predefined colors
#define COLOR_NONE      RGB(0x00, 0x00, 0x00)
#define COLOR_RED		RGB(0xff, 0x00, 0x00)
#define COLOR_GREEN     RGB(0x00, 0xff, 0x00)
#define COLOR_BLUE      RGB(0x00, 0x00, 0xff)

#define COLOR_ORANGE    RGB(250, 60, 0)
#define COLOR_PURPLE    RGB(250, 0, 142)
#define COLOR_LGTGREEN  RGB(110, 250, 73)
#define COLOR_TURKIZ    RGB(0, 225, 130)
#define COLOR_YELLOW    RGB(250, 153, 0)
#define COLOR_WHITE     RGB(250, 191, 102)
#define COLOR_PINK      RGB(250, 90, 102)

// Predefined patterns
#define PATTERN_RED_BLUE    0  // Red-Blue blinking in 0.5s period
#define PATTERN_RED_GREEN   1  // Red-Green blinking in 0.5s period
#define PATTERN_RED_PULSE   2  // Two short pulses of red
#define PATTERN_GREEN_PULSE 3  // Two short pulses of green
#define PATTERN_BLIMP       4  // Multiple short pulses of various colors, with spacing
#define PATTERN_COLOR_CHIRP 5  // 1-sec roll of multiple colors

void LEDInit();
void LEDTask();
void LEDSetColor(color_t color, int intensity);
void LEDSetPattern(int pattern);
void LEDCriticalSignal(int len);
