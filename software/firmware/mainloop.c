// Main application loop implementation

#include <string.h>
#include <stdlib.h>

// Simplelink includes
#include "simplelink.h"

#include "mainloop.h"
#include "console.h"

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
//END_STATE_TABLE
#define NUM_STATES 4

#define STATUS_CONNECTED  0
#define STATUS_IPACQUIRED 1

#define GET_STATUS(b) g_ulStatus = (g_ulStatus & (1 << b))
#define SET_STATUS(b) g_ulStatus = (g_ulStatus | (1 << b))
#define CLEAR_STATUS(b) g_ulStatus = (g_ulStatus & ~(1 << b))

#ifdef DEBUG_STATE
#define NEXT_STATE(s) ConsolePrintf("Switch to state: %s", s); g_eState = s
#else
#define NEXT_STATE(s) g_eState = s
#endif

// Global Variables
pAppState_t* g_stateTable;
static appState_t* g_tState;
static unsigned long g_ulStatus;
static const appConfig_t* g_tConfig;

// Functions
void MainLoopInit(const appConfig_t* config)
{
	g_tState = STATE_DOCONNECT;
	g_ulStatus = 0;
	g_tConfig = config;

	// TODO: Must do it more elegantly
	g_stateTable = (pAppState_t*)malloc(sizeof(appState_t)*NUM_STATES);
	g_stateTable[0] = STATE_DOCONNECT;
	g_stateTable[1] = STATE_WAITCONNECT;
	g_stateTable[2] = STATE_WAITFORIP;
	g_stateTable[3] = STATE_SENDDISCOVERY;
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
	return 0;
}

STATE_HANDLER(SENDDISCOVERY)
{
	return 0;
}

/////////////////////////////////////////////////////////////////////
///////////////////// SimpleLink Event Handlers /////////////////////
/////////////////////////////////////////////////////////////////////

void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent)
{
	ConsolePrintf("WLANevent....\n\r");

    switch(pWlanEvent->Event)
    {
        case SL_WLAN_CONNECT_EVENT:
            SET_STATUS(STATUS_CONNECTED);
            break;

        case SL_WLAN_DISCONNECT_EVENT:
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
	ConsolePrintf("event....\n\r");
    switch(pNetAppEvent->Event)
    {
        case SL_NETAPP_IPV4_IPACQUIRED_EVENT:
        {
        	CLEAR_STATUS(STATUS_IPACQUIRED);

            //SlIpV4AcquiredAsync_t *pEventData = NULL;

            //Ip Acquired Event Data
            //pEventData = &pNetAppEvent->EventData.ipAcquiredV4;

            //Gateway IP address
            ConsolePrintf("[NETAPP EVENT] IP Acquired: IP=%d.%d.%d.%d",
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,3),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,2),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,1),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,0));
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


