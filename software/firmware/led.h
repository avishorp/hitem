// LED control

#define RGB(r, g, b) (r + (g << 8) + (b << 16))
// Predefined colors
#define COLOR_NONE      RGB(0x00, 0x00, 0x00)
#define COLOR_RED		RGB(0xff, 0x00, 0x00)
#define COLOR_GREEN     RGB(0x00, 0xff, 0x00)
#define COLOR_BLUE      RGB(0x00, 0x00, 0xff)
#define COLOR_PURPLE    RGB(0xa0, 0x00, 0xff)


void LEDInit();
void LEDSetColor(int color, int intensity);
