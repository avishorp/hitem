// Temporary file - data should be retrieved from a file

#include "config.h"

appConfig_t g_tempAppConfig = {
		0, // lBoardBumber
		PERSONALITY_HAMMER, // lPersonality
		"xxxx", // sESSID
		"xxxx", // sPassword
		24333, // iDiscPort
		0xffffff00 // iNetmask
};


long ConfigInit()
{
	return 0;
}

const appConfig_t* ConfigGet()
{
	return &g_tempAppConfig;
}

