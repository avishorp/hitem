// SimpleLink Event handlers

#include "simplelink.h"

// General Event Handler
void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent)
{
//    UART_PRINT("[GENERAL EVENT] - ID=[%d] Sender=[%d]\n\n",
//               pDevEvent->EventData.deviceEvent.status,
//               pDevEvent->EventData.deviceEvent.sender);
}

void SimpleLinkSockEventHandler(SlSockEvent_t* pSockEvent)
{

}

void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent)
{

}

void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent)
{

}


// HTTP Server callback - not used
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pHttpEvent,
                                  SlHttpServerResponse_t *pHttpResponse)
{
}


// HTTP Server Event Handler - not used
_SlEventPropogationStatus_e sl_Provisioning_HttpServerEventHdl(
                            SlHttpServerEvent_t    *apSlHttpServerEvent,
                            SlHttpServerResponse_t *apSlHttpServerResponse)
{
	// Unused in this application
	return EVENT_PROPAGATION_CONTINUE;
}

// NetApp Provisioning Event Handler - not used
_SlEventPropogationStatus_e sl_Provisioning_NetAppEventHdl(SlNetAppEvent_t *apNetAppEvent)
{
	// Unused in this application
	return EVENT_PROPAGATION_CONTINUE;
}

// WLAN Provisioning Event Handler - not used
_SlEventPropogationStatus_e sl_Provisioning_WlanEventHdl(SlWlanEvent_t *apEventInfo)
{
	// Unused in this application
	return EVENT_PROPAGATION_CONTINUE;
}

