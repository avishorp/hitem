// Hit'em unit-specific configuration

#ifndef __CONFIG_H__
#define __CONFIG_H__

#define PERSONALITY_HAT     1
#define PERSONALITY_HAMMER  2

extern const char s_personality_hammer[];
extern const char s_personality_hat[];

#define PERSONALITY_STR(p) (((p) == PERSONALITY_HAT)? s_personality_hat : s_personality_hammer)

typedef struct {
	long lBoardNumber;      // Board serial number
	long lPersonality;      // Unit personality
} board_config_t;

typedef struct {
	char sESSID[30];        // WLAN ESSID
	char sPassword[30];     // WLAN Password
	unsigned long iDiscPort;// Discovery port number
} wlan_config_t;

typedef struct {
	board_config_t     board;
	wlan_config_t      wlan
} appConfig_t;

long ConfigInit();
const appConfig_t* ConfigGet();


#endif
