// Hit'em unit-specific configuration

#define PERSONALITY_HAT     1
#define PERSONALITY_HAMMER  2

typedef struct {
	long lBoardNumber;      // Board serial number
	long lPersonaliry;      // Unit personality
	char sESSID[20];        // WLAN ESSID
	char sPassword[20];     // WLAN Password
} appConfig_t;

long ConfigInit();
const appConfig_t* ConfigGet();
