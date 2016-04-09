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

#define DISCOVERY_MAGIC "HTEM"

//typedef struct appState_t;
typedef struct appState_t* (*stateHandler_t)();
//typedef void* (*stateHandler_t)();
typedef struct {
	const char* stateName;
	const stateHandler_t stateHandler;
} appState_t;
typedef appState_t* pAppState_t;

#define STATE_HANDLER(st) static struct appState_t* _StateHandle_ ## st()
#define BEGIN_STATE_TABLE \
	const appState_t g_StateTable[] = {
#define END_STATE_TABLE \
    };
#define DEF_STATE(st) \
	STATE_HANDLER(st); \
	appState_t __STATE_ ## st = { #st, &_StateHandle_ ## st }; \
	static const pAppState_t STATE_ ## st = &__STATE_ ## st;

#define STATE_HANDLER(st) static struct appState_t* _StateHandle_ ## st()

// Type & Constant definitions
//BEGIN_STATE_TABLE
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

//END_STATE_TABLE
#define NUM_STATES 10

#define STATUS_CONNECTED  0
#define STATUS_IPACQUIRED 1
#define STATUS_HIT        2

#define GET_STATUS(b) (g_ulStatus & (1 << b))
#define SET_STATUS(b) g_ulStatus = (g_ulStatus | (1 << b))
#define CLEAR_STATUS(b) g_ulStatus = (g_ulStatus & ~(1 << b))

// Global Variables
pAppState_t* g_stateTable;
static appState_t* g_tState;
static unsigned long g_ulStatus;
static _i16 g_iDiscoverySocket;
static _i16 g_iCmdSocket;
static _i16 g_iSyncSocket;
static _u32 g_iMyIP;

static SlSockAddrIn_t g_tBroadcastAddr;
static SlSockAddrIn_t g_tServerAddr;

systime_t g_iSyncTime;
systime_t g_iSyncTimeSched;

// Local forwards
void SocketCleanup();
void PinInterruptHandler();


// Functions
void MainLoopInit(const appConfig_t* config)
{
	g_tState = STATE_DOCONNECT;
	g_ulStatus = 0;
	g_iSyncTimeSched = NULL_TIME;

	// TODO: Must do it more elegantly
	g_stateTable = (pAppState_t*)malloc(sizeof(appState_t)*NUM_STATES);
	g_stateTable[0] = STATE_DOCONNECT;
	g_stateTable[1] = STATE_WAITCONNECT;
	g_stateTable[2] = STATE_WAITFORIP;
	g_stateTable[3] = STATE_SENDDISCOVERY;
	g_stateTable[4] = STATE_WAITDISCOVERY;
	g_stateTable[5] = STATE_DOCONNECTSRV;
	g_stateTable[6] = STATE_DOWELCOME;
	g_stateTable[7] = STATE_READY;
	g_stateTable[8] = STATE_CLEANUP;
	g_stateTable[9] = STATE_SLEEP;

	g_iCmdSocket = -1;
	g_iSyncSocket = -1;
	g_iDiscoverySocket = -1;

	// Initialize static broadcast address
	g_tBroadcastAddr.sin_family = SL_AF_INET;
	g_tBroadcastAddr.sin_port = sl_Htons(ConfigGet()->iDiscPort);

}

void MainLoopExec()
{
	// Scan for the state handler
	int i;
	for(i=0; i < NUM_STATES; i++) {
		if (g_stateTable[i] == g_tState) {
			// Invoke the function handler
			pAppState_t next = (pAppState_t)g_stateTable[i]->stateHandler();
			if (next != 0) {
				// State change
				ConsolePrintf(">> State switch to %s\n\r", next->stateName);
				g_tState = next;
			}
			return;
		}
	}

	ConsolePrint("ILLEGAL STATE");
	while(1);
}

//////////////////////////////////////////////////////////
///////////////////// State Handlers /////////////////////
//////////////////////////////////////////////////////////

SlSecParams_t secParams = {0};

// Connect to the WLAN
STATE_HANDLER(DOCONNECT)
{
	ConsolePrintf("Trying to connect to %s\n\r", ConfigGet()->sESSID);

	memset(&g_tServerAddr, 0, sizeof(g_tServerAddr));

    secParams.Key = (signed char *)(ConfigGet()->sPassword);
    secParams.KeyLen = strlen(ConfigGet()->sPassword);
    secParams.Type = SL_SEC_TYPE_WPA_WPA2;

    long lRetVal = sl_WlanConnect((signed char *)(ConfigGet()->sESSID),
                           strlen((const char *)(ConfigGet()->sESSID)), 0, &secParams, 0);
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

	if (GET_STATUS(STATUS_IPACQUIRED))
		return STATE_SENDDISCOVERY;

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

	static const char sDiscoveryMsg[] = DISCOVERY_MAGIC;

	// Adjust broadcast address according to allocated IP
	g_tBroadcastAddr.sin_addr.s_addr = sl_Htonl(g_iMyIP | (~(ConfigGet()->iNetmask)));

	sl_SendTo(g_iDiscoverySocket, sDiscoveryMsg, sizeof(sDiscoveryMsg), 0, (SlSockAddr_t*)&g_tBroadcastAddr, sizeof(SlSockAddrIn_t));

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

	char rbuf[10];

	_i16 asize = sizeof(SlSockAddrIn_t);
	_i16 lRet = sl_RecvFrom(g_iDiscoverySocket, &rbuf, sizeof(rbuf), 0,
			(SlSockAddr_t*)&g_tServerAddr, (SlSocklen_t*)&asize);

	if (lRet > 0) {
		if ((lRet >= strlen(DISCOVERY_MAGIC)) && (strncmp(rbuf, DISCOVERY_MAGIC, strlen(DISCOVERY_MAGIC))==0)) {
			// Return packet is valid
			// Adjust server port
			g_tServerAddr.sin_port = sl_Htons(ConfigGet()->iSrvPort);
			return STATE_DOCONNECTSRV;
		}
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
		addr.sin_port = sl_Htons(g_tAppConfig->iSrvPort);
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
	// Receive data from sync socket
	lRet = sl_Recv(g_iSyncSocket, buf, sizeof(buf), 0);
	if (lRet > 0) {
		ConsolePrint("***SYNC***\n\r");
	}
	else if (lRet != SL_EAGAIN) {
		ConsolePrintf("sl_Recv [g_iSyncSocket]: %d\n\r", lRet);
	}
#endif

	// Handle sync requests
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


	return 0;
}

STATE_HANDLER(CLEANUP)
{
	SocketCleanup();

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
