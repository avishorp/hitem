// Temporary file - data should be retrieved from a file

#include "config.h"

appConfig_t g_tempAppConfig = {
		0, // lBoardBumber
		PERSONALITY_HAMMER, // lPersonality
		"hitem", // sESSID
		"hitem", // sPassword
};


long ConfigInit()
{
	return 0;
}

const appConfig_t* ConfigGet()
{
	return &g_tempAppConfig;
}

