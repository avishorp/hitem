// Main application loop implementation

#include <string.h>
#include <stdlib.h>

// Simplelink includes
#include "simplelink.h"

#include "mainloop.h"
#include "console.h"
#include "error.h"

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

//END_STATE_TABLE
#define NUM_STATES 5

#define STATUS_CONNECTED  0
#define STATUS_IPACQUIRED 1

#define GET_STATUS(b) g_ulStatus = (g_ulStatus & (1 << b))
#define SET_STATUS(b) g_ulStatus = (g_ulStatus | (1 << b))
#define CLEAR_STATUS(b) g_ulStatus = (g_ulStatus & ~(1 << b))

// Global Variables
appConfig_t* g_tAppConfig;
pAppState_t* g_stateTable;
static appState_t* g_tState;
static unsigned long g_ulStatus;
static const appConfig_t* g_tConfig;
static _i16 g_iDiscoverySocket;
static _i16 g_iCmdSocket;
static _i16 g_iSyncSocket;
static _u32 g_iMyIP;

static SlSockAddrIn_t g_tBroadcastAddr;



// Functions
void MainLoopInit(const appConfig_t* config)
{
	g_tAppConfig = config;
	g_tState = STATE_DOCONNECT;
	g_ulStatus = 0;
	g_tConfig = config;

	// TODO: Must do it more elegantly
	g_stateTable = (pAppState_t*)malloc(sizeof(appState_t)*NUM_STATES);
	g_stateTable[0] = STATE_DOCONNECT;
	g_stateTable[1] = STATE_WAITCONNECT;
	g_stateTable[2] = STATE_WAITFORIP;
	g_stateTable[3] = STATE_SENDDISCOVERY;
	g_stateTable[4] = STATE_WAITDISCOVERY;

	// Create sockets
	SlSockNonblocking_t nb;
	nb.NonblockingEnabled = 1;

	g_iDiscoverySocket = sl_Socket(AF_INET, SOCK_DGRAM, 0);
	if (g_iDiscoverySocket < 0)
		FatalError("Discovery socket creation failed: %d", g_iDiscoverySocket);
	sl_SetSockOpt(g_iDiscoverySocket, SL_SOL_SOCKET, SL_SO_NONBLOCKING, &nb, sizeof(SlSockNonblocking_t));

	g_iCmdSocket = sl_Socket(AF_INET, SOCK_STREAM, 0);
	if (g_iCmdSocket < 0)
		FatalError("Command socket creation failed: %d", g_iCmdSocket);
	sl_SetSockOpt(g_iCmdSocket, SL_SOL_SOCKET, SL_SO_NONBLOCKING, &nb, sizeof(SlSockNonblocking_t));


	g_iSyncSocket = sl_Socket(AF_INET, SOCK_DGRAM, 0);
	if (g_iSyncSocket < 0)
		FatalError("Sync socket creation failed: %d", g_iSyncSocket);
	sl_SetSockOpt(g_iSyncSocket, SL_SOL_SOCKET, SL_SO_NONBLOCKING, &nb, sizeof(SlSockNonblocking_t));

	// Initialize static broadcast address
	g_tBroadcastAddr.sin_family = SL_AF_INET;
	g_tBroadcastAddr.sin_port = sl_Htons(config->iDiscPort);

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

STATE_HANDLER(DOCONNECT)
{
	ConsolePrintf("Trying to connect to %s\n\r", g_tConfig->sESSID);


    secParams.Key = (signed char *)g_tConfig->sPassword;
    secParams.KeyLen = strlen(g_tConfig->sPassword);
    secParams.Type = SL_SEC_TYPE_WPA_WPA2;

    long lRetVal = sl_WlanConnect((signed char *)g_tConfig->sESSID,
                           strlen((const char *)g_tConfig->sESSID), 0, &secParams, 0);
    if (lRetVal < 0)
    	ConsolePrintf("Error %d", lRetVal);

    return STATE_WAITCONNECT;
}

STATE_HANDLER(WAITCONNECT)
{
	if (GET_STATUS(STATUS_CONNECTED)) {
		ConsolePrint("Connected\n\r");
		return STATE_WAITFORIP;
	}
	return 0;
}

STATE_HANDLER(WAITFORIP)
{
	if (GET_STATUS(STATUS_IPACQUIRED))
		return STATE_SENDDISCOVERY;
}

STATE_HANDLER(SENDDISCOVERY)
{
	static const char sDiscoveryMsg[] = "HTEM";

	// Adjust broadcast address according to allocated IP
	g_tBroadcastAddr.sin_addr.s_addr = sl_Htonl(g_iMyIP | (~g_tAppConfig->iNetmask));

	sl_SendTo(g_iDiscoverySocket, sDiscoveryMsg, sizeof(sDiscoveryMsg), 0, (SlSockAddr_t*)&g_tBroadcastAddr, sizeof(SlSockAddrIn_t));
	return STATE_WAITDISCOVERY;
}

STATE_HANDLER(WAITDISCOVERY)
{
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
        	ConsolePrint("[WLAN EVENT] Connected");
            SET_STATUS(STATUS_CONNECTED);
            break;

        case SL_WLAN_DISCONNECT_EVENT:
        	ConsolePrint("[WLAN EVENT] Disonnected");
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


