// Main application loop implementation

#include <string.h>
#include <stdlib.h>

// Simplelink includes
#include "simplelink.h"

// Peripheral Lib includes
#include "hw_types.h"
#include "hw_memmap.h"
#include "rom_map.h"
#include "utils.h"
#include "pin.h"
#include "gpio.h"
#include "prcm.h"

// Local includes
#include "mainloop.h"
#include "console.h"
#include "error.h"
#include "led.h"
#include "protocol.h"
#include "time.h"
#include "settings.h"
#include "time.h"
#include "analog.h"
#include "config.h"
#include "statedef.h"
#include "version.h"
#include "ota.h"

// Type & Constant definitions
DEF_STATE(DOCONNECT)     // Instruct SimpleLink to connect to the WiFi Network
DEF_STATE(WAITCONNECT)   // Wait for WLAN connection to be established
DEF_STATE(WAITFORIP)     // Wait for IP address
DEF_STATE(SENDDISCOVERY)  // Send discovery message
DEF_STATE(WAITDISCOVERY)  // Send discovery message
DEF_STATE(DOCONNECTSRV)   // Connect the server
DEF_STATE(DOWELCOME)      // Send welcome message
DEF_STATE(READY)          // Ready state
DEF_STATE(CLEANUP)        // Clean opened data and then retry connecting
DEF_STATE(SLEEP)          // Go to sleep


#define STATUS_CONNECTED  0
#define STATUS_IPACQUIRED 1
#define STATUS_HIT        2

#define GET_STATUS(b) (g_ulStatus & (1 << b))
#define SET_STATUS(b) g_ulStatus = (g_ulStatus | (1 << b))
#define CLEAR_STATUS(b) g_ulStatus = (g_ulStatus & ~(1 << b))

#define DISCOVERY_MAGIC      "HTEM"
#define DISCOVERY_MAGIC_LEN  4

typedef struct {
	char magic[DISCOVERY_MAGIC_LEN];
						  // Magic code
	version_t fw_version; // Current firmware version
	long lBoardNumber;    // Board serial number
	long lPersonaliry;    // Unit personality
} discovery_req_t;

typedef struct {
	char magic[DISCOVERY_MAGIC_LEN];
						  // Magic code
	version_t fw_version;
	_u16 srv_port;
	_u16 tftp_port;
	char fw_filename[32];
} discovery_resp_t;

// Global Variables
static pState_t g_tState;
static unsigned long g_ulStatus;
static _i16 g_iDiscoverySocket;
static _i16 g_iCmdSocket;
static _i16 g_iSyncSocket;
static _u32 g_iMyIP;
static unsigned long g_iMyNetMask;
static discovery_req_t g_discoveryRequest;
static _u16 g_iSrvPort;

static SlSockAddrIn_t g_tBroadcastAddr;
static SlSockAddrIn_t g_tServerAddr;

systime_t g_iSyncTime;
systime_t g_iSyncTimeSched;

systime_t g_iLastBatteryReportTime;

// Local forwards
void SocketCleanup();
void PinInterruptHandler();


// Functions
void MainLoopInit(const appConfig_t* config)
{
	g_tState = STATE_DOCONNECT;
	g_ulStatus = 0;
	g_iSyncTimeSched = NULL_TIME;

	g_iCmdSocket = -1;
	g_iSyncSocket = -1;
	g_iDiscoverySocket = -1;

	g_iLastBatteryReportTime = 0;

	// Initialize static broadcast address
	g_tBroadcastAddr.sin_family = SL_AF_INET;
	g_tBroadcastAddr.sin_port = sl_Htons(ConfigGet()->wlan.iDiscPort);

	// Create the discovery request message
	memcpy(&g_discoveryRequest.magic, DISCOVERY_MAGIC, 4);
	VersionGet(&(g_discoveryRequest.fw_version));
	g_discoveryRequest.lBoardNumber = config->board.lBoardNumber;
	g_discoveryRequest.lPersonaliry = config->board.lPersonality;
}

void MainLoopExec()
{
	// TODO: Check the integrity of the state handle
	if (g_tState->stateSigniture != STATE_SIGNITURE) {
		FatalError("ILLEGAL STATE");
	}

	// Execute the state handler
	pState_t next = g_tState->stateHandler();

	if (next != 0) {
		// State change
		ConsolePrintf(">> State switch to %s\n\r", next->stateName);
		g_tState = next;
	}

}

//////////////////////////////////////////////////////////
///////////////////// State Handlers /////////////////////
//////////////////////////////////////////////////////////

SlSecParams_t secParams = {0};

// Connect to the WLAN
STATE_HANDLER(DOCONNECT)
{
	ConsolePrintf("Trying to connect to %s\n\r", ConfigGet()->wlan.sESSID);

	memset(&g_tServerAddr, 0, sizeof(g_tServerAddr));

    secParams.Key = (signed char *)(ConfigGet()->wlan.sPassword);
    secParams.KeyLen = strlen(ConfigGet()->wlan.sPassword);
    secParams.Type = SL_SEC_TYPE_WPA_WPA2;

    long lRetVal = sl_WlanConnect((signed char *)(ConfigGet()->wlan.sESSID),
                           strlen((const char *)(ConfigGet()->wlan.sESSID)), 0, &secParams, 0);
    if (lRetVal < 0)
    	ConsolePrintf("Error %d", lRetVal);

    LEDSetPattern(PATTERN_RED_BLUE);

    TimeSetTimeout(1, 10000);

    return STATE_WAITCONNECT;
}

// Wait for WLAN connection to be established
STATE_HANDLER(WAITCONNECT)
{
	if (GET_STATUS(STATUS_CONNECTED)) {
		ConsolePrint("Connected\n\r");
		return STATE_WAITFORIP;
	}
	else if (TimeGetEvent(1)) {
		// Connection timeout - go to sleep
		return STATE_SLEEP;
	}
	return 0;
}

// Wait for IP address to be acquired
STATE_HANDLER(WAITFORIP)
{
	if (!GET_STATUS(STATUS_CONNECTED))
		return STATE_CLEANUP;

	if (GET_STATUS(STATUS_IPACQUIRED)) {
/*
		// Retrieve the Netmask
		_u8 ConfigOpt = 0, ConfigLen = sizeof(SlNetCfgIpV4DhcpClientArgs_t);
		_i32 Status;
		static SlNetCfgIpV4DhcpClientArgs_t Dhcp;
		Status = sl_NetCfgGet(SL_IPV4_DHCP_CLIENT,&ConfigOpt,&ConfigLen,(_u8 *)&Dhcp);
		if( Status )
		{
		  ConsolePrint("Failed retrieving the DHCP NetMask");
		  return STATE_CLEANUP;
		}
		g_iMyNetMask = Dhcp.Mask;
*/

		// TODO: sl_NetCfgGet hangs, so I use constant netmask
		g_iMyNetMask = (255 << 24) + (255 << 16) + (255 << 8);

#ifdef DEBUG_UDP_PORT
		// Connect report port (to broadcast address
		ConsoleConnectUDP(g_iMyIP | (~(g_iMyNetMask)), DEBUG_UDP_PORT);
#endif

		return STATE_SENDDISCOVERY;
	}

	return 0;
}

// Send a discovery request packet
STATE_HANDLER(SENDDISCOVERY)
{
	if (!GET_STATUS(STATUS_CONNECTED))
		return STATE_CLEANUP;

	// Create socket
	if (g_iDiscoverySocket < 0) {
		SlSockNonblocking_t nb;
		nb.NonblockingEnabled = 1;

		g_iDiscoverySocket = sl_Socket(AF_INET, SOCK_DGRAM, 0);
		if (g_iDiscoverySocket < 0)
			FatalError("Discovery socket creation failed: %d", g_iDiscoverySocket);
		sl_SetSockOpt(g_iDiscoverySocket, SL_SOL_SOCKET, SL_SO_NONBLOCKING, &nb, sizeof(SlSockNonblocking_t));
	}

	// Adjust broadcast address according to allocated IP
	g_tBroadcastAddr.sin_addr.s_addr = sl_Htonl(g_iMyIP | (~(g_iMyNetMask)));
	sl_SendTo(g_iDiscoverySocket, &g_discoveryRequest, sizeof(discovery_req_t), 0, (SlSockAddr_t*)&g_tBroadcastAddr, sizeof(SlSockAddrIn_t));

    LEDSetPattern(PATTERN_RED_GREEN);

    TimeSetTimeout(1, DISCOVERY_TIMEOUT);

	return STATE_WAITDISCOVERY;
}

// Wait for discovery response packet, and set the server
// IP address accordingly
STATE_HANDLER(WAITDISCOVERY)
{
	if (!GET_STATUS(STATUS_CONNECTED))
		return STATE_CLEANUP;

	discovery_resp_t rbuf;

	memset((void*)&rbuf, 0, sizeof(rbuf));
	_i16 asize = sizeof(SlSockAddrIn_t);
	_i16 lRet = sl_RecvFrom(g_iDiscoverySocket, (_u8*)&rbuf, sizeof(rbuf), 0,
			(SlSockAddr_t*)&g_tServerAddr, (SlSocklen_t*)&asize);

	if (lRet > 0) {
		// Got response. Must be in the length of a discovery response and start with the magic string
		if ((lRet == sizeof(discovery_resp_t)) && (strncmp(rbuf.magic, DISCOVERY_MAGIC, DISCOVERY_MAGIC_LEN)==0)) {
			// Return packet is valid
			ConsolePrintf("Got discovery response\n\r");

			// Adjust server port
			g_tServerAddr.sin_port = sl_Htons(g_iSrvPort);

			char server_version_str[20];
			VersionToString(&rbuf.fw_version, server_version_str);

			// Get the unit's FW version abd print it
			version_t current_version;
			VersionGet(&current_version);
			char current_version_str[20];
			VersionToString(&current_version, current_version_str);

			ConsolePrintf("Current FW version is %s, server has version %s\n\r", current_version_str, server_version_str);

			if (VersionGreaterThan(&rbuf.fw_version, &current_version)) {
				ConsolePrintf("Starting OTA Process\n\r");
				if (OTAExec(ntohl(g_tServerAddr.sin_addr.s_addr), rbuf.tftp_port, rbuf.fw_filename))
					return STATE_SLEEP;
			}

			// Set the server port
			g_iSrvPort = rbuf.srv_port;

			return STATE_DOCONNECTSRV;
		}
		else { ConsolePrintf("len=%d magic=%s filename=%s\n\r", lRet, rbuf.magic, rbuf.fw_filename); }
	}
	else if (lRet == SL_EAGAIN) {
		// Check that timeout hasn't occured
		if (TimeGetEvent(1))
			return STATE_SENDDISCOVERY;
	}
	else {
		// Something went wrong
		ConsolePrintf("sl_RecvFrom returned %d\n\r", (int)lRet);
	}

	return 0;
}

STATE_HANDLER(DOCONNECTSRV)
{
	if (!GET_STATUS(STATUS_CONNECTED))
		return STATE_CLEANUP;

	// Close the discovery socket
	if (g_iDiscoverySocket >= 0) {
		long l = sl_Close(g_iDiscoverySocket);
		g_iDiscoverySocket = -1;
		if (l < 0)
			ConsolePrintf("Discovery socket close failed: %d\n\r", l);
	}

	SlSockNonblocking_t nb;
	nb.NonblockingEnabled = 1;

#ifndef NO_UDP_SYNC_REQ
	// Create the endpoint sync request socket
	if (g_iSyncSocket == -1) {
		g_iSyncSocket = sl_Socket(AF_INET, SOCK_DGRAM, 0);
		if (g_iSyncSocket < 0)
			FatalError("Sync socket creation failed: %d", g_iSyncSocket);
		sl_SetSockOpt(g_iSyncSocket, SL_SOL_SOCKET, SL_SO_NONBLOCKING, &nb, sizeof(SlSockNonblocking_t));

		SlSockAddrIn_t addr;
		memset(&addr, 0, sizeof(SlSockAddrIn_t));
		addr.sin_family = AF_INET;
		addr.sin_port = sl_Htons(g_iSrvPort);

		sl_Bind(g_iSyncSocket, (SlSockAddr_t*)&addr, sizeof(SlSockAddrIn_t));
	}
#endif

	// Create the endpoint command socket
	if (g_iCmdSocket == -1) {

		g_iCmdSocket = sl_Socket(AF_INET, SOCK_STREAM, 0);
		if (g_iCmdSocket < 0)
			FatalError("Command socket creation failed: %d", g_iCmdSocket);
		sl_SetSockOpt(g_iCmdSocket, SL_SOL_SOCKET, SL_SO_NONBLOCKING, &nb, sizeof(SlSockNonblocking_t));
	}

	// Connect the command socket
	long lRet = sl_Connect(g_iCmdSocket, (SlSockAddr_t*)&g_tServerAddr, sizeof(SlSockAddrIn_t));
	if (lRet == SL_EALREADY)
		return 0; // Will try again

	else if ((lRet == SL_ETIMEDOUT) || (lRet == SL_ECONNREFUSED))
		// Refused - return to discovery
		return STATE_SENDDISCOVERY;

	else if (lRet < 0)
		FatalError("sl_Connect: %d\n\r", lRet);

	return STATE_DOWELCOME;
}

STATE_HANDLER(DOWELCOME)
{
	if (!GET_STATUS(STATUS_CONNECTED))
		return STATE_CLEANUP;

	LEDSetColor(COLOR_NONE, 0);
	long lRet = ProtocolSendWelcome(g_iCmdSocket);
	if (lRet < 0) {
		ConsolePrintf("Failed sending welcome message: %d\n\r", lRet);
		return 0;
	}

	return STATE_READY;
}

STATE_HANDLER(READY)
{
	if (!GET_STATUS(STATUS_CONNECTED))
		return STATE_CLEANUP;

	char buf[10];

	// Receive data from command socket
	long lRet = sl_Recv(g_iCmdSocket, buf, sizeof(buf), 0);
	if (lRet > 0) {
		ProtocolParse(buf, lRet);
	}
	else if (lRet == 0) {
		// Socket disconnected
		sl_Close(g_iCmdSocket);
		g_iCmdSocket = -1;

		//sl_Close(g_iSyncSocket);
		//g_iSyncSocket = -1;

		return STATE_SENDDISCOVERY;
	}
	else if (lRet != SL_EAGAIN) {
		ConsolePrintf("sl_Recv [g_iCmdSocket]: %d\n\r", lRet);
	}

#ifndef NO_UDP_SYNC_REQ
	// Handle UDP Sync request
	lRet = sl_Recv(g_iSyncSocket, buf, sizeof(buf), 0);
	if (lRet > 0) {
		// Wer'e not even checking the content of the message
		ProtocolSendSyncResp(g_iCmdSocket, TimeGetSystime());
	}
	else if (lRet != SL_EAGAIN) {
		ConsolePrintf("sl_Recv [g_iSyncSocket]: %d\n\r", lRet);
	}
#endif

	// Handle TCP sync requests
	systime_t syncTime = ProtocolGetSyncTime();
	if (syncTime != NULL_TIME) {
		// Schedule the send time to +10mS
		g_iSyncTime = syncTime;
		g_iSyncTimeSched = TimeGetSystime() + 10;

		ConsolePrintf("Now is %d scheduled to %d\n\r", TimeGetSystime(), g_iSyncTimeSched);
	}
	if (TimeGetSystime() > g_iSyncTimeSched) {
		if (ProtocolSendSyncResp(g_iCmdSocket, g_iSyncTime) >= 0)
			g_iSyncTimeSched = NULL_TIME;
	}

	// Handle hit detection
	systime_t hitTime = AnalogGetHitTime();
	if (hitTime != NULL_TIME) {
		ProtocolSendHit(g_iCmdSocket, hitTime);
	}

	// Periodic Battery Report
	systime_t now = TimeGetSystime();
	if ((now - g_iLastBatteryReportTime) > BATTERY_REPORT_PERIOD) {
		int bat = AnalogGetBatteryVoltage();
		ProtocolSendBatReport(g_iCmdSocket, bat);

		g_iLastBatteryReportTime = now;
	}


	return 0;
}

STATE_HANDLER(CLEANUP)
{
	SocketCleanup();

#ifdef DEBUG_UDP_PORT
	ConsoleDisconnectUDP();
#endif

	// Disconnect from WLAN
	sl_WlanDisconnect();

	// Retry connection
	return STATE_DOCONNECT;
}

STATE_HANDLER(SLEEP)
{
	SocketCleanup();

	DoSleep();

	return 0;
}


/////////////////////////////////////////////////////////////////////
///////////////////// SimpleLink Event Handlers /////////////////////
/////////////////////////////////////////////////////////////////////

void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent)
{
    switch(pWlanEvent->Event)
    {
        case SL_WLAN_CONNECT_EVENT:
        	ConsolePrint("[WLAN EVENT] Connected\n\r");
            SET_STATUS(STATUS_CONNECTED);
            break;

        case SL_WLAN_DISCONNECT_EVENT:
        	ConsolePrint("[WLAN EVENT] Disonnected\n\r");
            CLEAR_STATUS(STATUS_CONNECTED);
            CLEAR_STATUS(STATUS_IPACQUIRED);
            break;

        default:
        {
            ConsolePrintf("[WLAN EVENT] Unexpected event [0x%x]\n\r",
                       pWlanEvent->Event);
        }
        break;
    }
}

//*****************************************************************************
//
//! \brief This function handles network events such as IP acquisition, IP
//!           leased, IP released etc.
//!
//! \param[in]  pNetAppEvent - Pointer to NetApp Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent)
{
    switch(pNetAppEvent->Event)
    {
        case SL_NETAPP_IPV4_IPACQUIRED_EVENT:
        {
        	SET_STATUS(STATUS_IPACQUIRED);

            //SlIpV4AcquiredAsync_t *pEventData = NULL;

            //Ip Acquired Event Data
            //pEventData = &pNetAppEvent->EventData.ipAcquiredV4;

            //Gateway IP address
            ConsolePrintf("[NETAPP EVENT] IP Acquired: IP=%d.%d.%d.%d\n\r",
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,3),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,2),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,1),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,0));

            g_iMyIP = pNetAppEvent->EventData.ipAcquiredV4.ip;
        }
        break;

        default:
        {
            ConsolePrintf("[NETAPP EVENT] Unexpected event [0x%x] \n\r",
                       pNetAppEvent->Event);
        }
        break;
    }
}

void SimpleLinkSockEventHandler(SlSockEvent_t* pSockEvent)
{
	ConsolePrintf("[SOCK EVENT] %d\n\r", pSockEvent->Event);
}

//////////////////////////////////////////////////
/////////////////////  Misc  /////////////////////
//////////////////////////////////////////////////
void SocketCleanup()
{
	// If sockets are open, close them
	if (g_iCmdSocket > 0) {
		sl_Close(g_iCmdSocket);
		g_iCmdSocket = -1;
	}
	if (g_iSyncSocket > 0) {
		sl_Close(g_iSyncSocket);
		g_iSyncSocket = -1;
	}
	if (g_iDiscoverySocket > 0) {
		sl_Close(g_iSyncSocket);
		g_iDiscoverySocket = -1;
	}
}

void PinInterruptHandler()
{
	LEDSetColor(COLOR_WHITE, 50);
	MAP_GPIOIntClear(GPIOA0_BASE, 0xff);
}
