// Temporary file - data should be retrieved from a file

#include <string.h>

#include "config.h"
#include "error.h"
#include "console.h"

#include "simplelink.h"

const _u8 CONFIG_FILENAME[] = "config.bin";
const _u8 WLAN_CRED_FILENAME[] = "wlan.bin";

extern const char s_personality_hammer[] = "Hammer";
extern const char s_personality_hat[] = "Hat";


static appConfig_t g_tAppConfig;

long _LoadBoardConfig()
{
	// The configuration data is read from a file named "config.bin"

	// Open the file
	_i32 handle = 0;
	_i32 ret = sl_FsOpen(CONFIG_FILENAME, FS_MODE_OPEN_READ, NULL, &handle);
	if (ret < 0)
		FatalError("Failed opening configuration file: %d\n\r", ret);

	// Read the data
	ret = sl_FsRead(handle, 0, (_u8*)&g_tAppConfig.board, sizeof(board_config_t));
	if (ret < 0)
		FatalError("Failed reading configuration file: %d\n\r", ret);

	// Close the file
	ret = sl_FsClose(handle, NULL, NULL, 0);
	if (ret < 0)
		FatalError("Failed closing configuration file: %d\n\r", ret);

	return 0;
}

long _LoadWLANConfig()
{
	// The configuration data is read from a file named "wlan.bin"

	// Open the file
	_i32 handle = 0;
	_i32 ret = sl_FsOpen(WLAN_CRED_FILENAME, FS_MODE_OPEN_READ, NULL, &handle);
	if (ret < 0)
		FatalError("Failed opening configuration file: %d\n\r", ret);

	// Read the data
	ret = sl_FsRead(handle, 0, (_u8*)&g_tAppConfig.wlan, sizeof(wlan_config_t));
	if (ret < 0)
		FatalError("Failed reading configuration file: %d\n\r", ret);

	// Close the file
	ret = sl_FsClose(handle, NULL, NULL, 0);
	if (ret < 0)
		FatalError("Failed closing configuration file: %d\n\r", ret);

	return 0;
}

long ConfigInit()
{
	long ret;
	ret = _LoadBoardConfig();
	if (ret < 0)
		// Error occured
		return ret;

	ret = _LoadWLANConfig();
	if (ret < 0)
		// Error occured
		return ret;

				// Print the configuration
	ConsolePrint("Unit configuration:\n\r");
	ConsolePrintf("   Unit ID: %d\n\r", g_tAppConfig.board.lBoardNumber);
	ConsolePrintf("   Personality: %s\n\r", PERSONALITY_STR(g_tAppConfig.board.lPersonality));
	ConsolePrintf("   WLAN SSID: %s\n\r", g_tAppConfig.wlan.sESSID);
	ConsolePrintf("   WLAN Password: %s\n\r", g_tAppConfig.wlan.sPassword);
	ConsolePrintf("   Discovery port: %d\n\r", g_tAppConfig.wlan.iDiscPort);

	return 0;
}

const appConfig_t* ConfigGet()
{
	return &g_tAppConfig;
}

