// Hit'em unit-specific configuration

#ifndef __CONFIG_H__
#define __CONFIG_H__

#define PERSONALITY_HAT     1
#define PERSONALITY_HAMMER  2

typedef struct {
	long lBoardNumber;      // Board serial number
	long lPersonaliry;      // Unit personality
	char sESSID[30];        // WLAN ESSID
	char sPassword[20];     // WLAN Password
	unsigned long iDiscPort;// Discovery port number
	unsigned long iNetmask; // Network mask
	unsigned long iSrvPort; // Endpoint server port
} appConfig_t;

long ConfigInit();
const appConfig_t* ConfigGet();


#endif
