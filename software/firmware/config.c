// Temporary file - data should be retrieved from a file

#include <string.h>

#include "config.h"
#include "error.h"
#include "console.h"

#include "simplelink.h"

const _u8 CONFIG_FILENAME[] = "config.bin";

static appConfig_t g_tAppConfig;

long ConfigInit()
{
	// The configuration data is read from a file named "config.bin"

	// Open the file
	_i32 handle = 0;
	_i32 ret = sl_FsOpen(CONFIG_FILENAME, FS_MODE_OPEN_READ, NULL, &handle);
	if (ret < 0)
		FatalError("Failed opening configuration file: %d\n\r", ret);

	// Read the data
	ret = sl_FsRead(handle, 0, (_u8*)&g_tAppConfig, 80);
	if (ret < 0)
		FatalError("Failed reading configuration file: %d\n\r", ret);

	// Close the file
	ret = sl_FsClose(handle, NULL, NULL, 0);
	if (ret < 0)
		FatalError("Failed closing configuration file: %d\n\r", ret);

	// Print the configuration
	ConsolePrint("Unit configuration:\n\r");
	ConsolePrintf("   Unit ID: %d\n\r", g_tAppConfig.lBoardNumber);
	ConsolePrintf("   Personality: %s\n\r", g_tAppConfig.lPersonaliry == PERSONALITY_HAMMER? "Hammer" : "Hat");
	ConsolePrintf("   WLAN SSID: %s\n\r", g_tAppConfig.sESSID);
	ConsolePrintf("   WLAN Password: %s\n\r", g_tAppConfig.sPassword);
	ConsolePrintf("   Discovery port: %d\n\r", g_tAppConfig.iDiscPort);
	ConsolePrintf("   Server port: %d\n\r", g_tAppConfig.iSrvPort);

	return 0;
}

const appConfig_t* ConfigGet()
{
	return &g_tAppConfig;
}

