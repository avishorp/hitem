// Endpoint-Server protocol implementation

#include "protocol.h"
#include "config.h"
#include "led.h"
#include "console.h"
#include "time.h"

// Color table
const color_t g_iColorTable[] = {
	COLOR_NONE,
	COLOR_RED,
	COLOR_GREEN,
	COLOR_BLUE,
	COLOR_ORANGE,
	COLOR_PURPLE,
	COLOR_LGTGREEN,
	COLOR_TURKIZ,
	COLOR_YELLOW,
	COLOR_WHITE,
	COLOR_PINK
};
#define NUM_COLORS (sizeof(g_iColorTable)/sizeof(color_t))

// Local Forwards
_u8 _CalcChecksum(const message_t* msg);
void _ProcessIngressMessage(const message_t* msg);

// Globals
static _u8 g_cMessageBuffer[MESSAGE_LENGTH];
static systime_t g_iSyncTime;

void ProtocolInit()
{
	memset(&g_cMessageBuffer, 0, MESSAGE_LENGTH);
	g_iSyncTime = NULL_TIME;
}

void ProtocolParse(const char* buf, int len)
{
	int i, j;
	for(i=0; i < len; i++) {
		// Push every new character into the buffer and shift it left
		for(j=0; j < len-1; j++)
			g_cMessageBuffer[j] = g_cMessageBuffer[j+1];
		g_cMessageBuffer[len-1] = buf[i];

		//ConsolePrintf("buf: %2x %2x %2x %2x %2x %2x %2x\n\r",
		//	g_cMessageBuffer[0], g_cMessageBuffer[1], g_cMessageBuffer[2],
		//	g_cMessageBuffer[3], g_cMessageBuffer[4], g_cMessageBuffer[5], g_cMessageBuffer[6]);

		// Check the validity of the message
		message_t* msg = (message_t*)g_cMessageBuffer;
		_u8 checksum = _CalcChecksum(msg);
		if (msg->prolog == MSG_PROLOG) {
			if (msg->checksum == checksum) {
				// Valid message
				_ProcessIngressMessage(msg);
			}
			else
				ConsolePrintf("Received massage with invalid checksum: expected %d, got %d\n\r", checksum, msg->checksum);
		}
	}
}

_i16 ProtocolSendWelcome(_i16 sock)
{
	message_t msg;
	const appConfig_t* config = ConfigGet();

	// Prepare the message
	memset(&msg, 0, sizeof(message_t));
	msg.prolog = MSG_PROLOG;
	msg.type = MSG_TYPE_WELCOME;
	msg.payload.welcome.endpoint_id = config->lBoardNumber;
	msg.payload.welcome.endpoint_personality = config->lPersonaliry;
	msg.checksum = _CalcChecksum(&msg);

	// Send it
	return sl_Send(sock, &msg, sizeof(message_t), 0);
}

_i16 ProtocolSendSyncResp(_i16 sock, systime_t time)
{
	message_t msg;

	// Prepare the message
	memset(&msg, 0, sizeof(message_t));
	msg.prolog = MSG_PROLOG;
	msg.type = MSG_TYPE_SYNC_RSP;
	msg.payload.timestamp = time;
	msg.checksum = _CalcChecksum(&msg);

	// Send it
	return sl_Send(sock, &msg, sizeof(message_t), 0);
}

_i16 ProtocolSendHit(_i16 sock, systime_t time)
{
	message_t msg;

	// Prepare the message
	memset(&msg, 0, sizeof(message_t));
	msg.prolog = MSG_PROLOG;
	msg.type = MSG_TYPE_HIT;
	msg.payload.timestamp = time;
	msg.checksum = _CalcChecksum(&msg);

	// Send it
	return sl_Send(sock, &msg, sizeof(message_t), 0);
}


systime_t ProtocolGetSyncTime()
{
	systime_t t = g_iSyncTime;
	g_iSyncTime = NULL_TIME;

	return t;
}

_u8 _CalcChecksum(const message_t* msg)
{
	_u8 checksum = 0;
	int i;
	for(i=0; i<(sizeof(message_t)-1); i++)
		checksum += ((_u8*)(msg))[i];

	return checksum;
}

void _ProcessIngressMessage(const message_t* msg)
{
	if (msg->type == MSG_TYPE_SET_COLOR) {
		_u8 cc = msg->payload.set_color.color_code;
		if (cc <= (NUM_COLORS-1))
			LEDSetColor(g_iColorTable[cc], msg->payload.set_color.intensity);
		else
			ConsolePrintf("Invalid color code received: %d\n\r");
	}
	else if (msg->type == MSG_TYPE_INDICATE) {
		// The only valid values are between 2 and 5
		_u8 pcd = msg->payload.indicate.indication_code;
		if ((pcd >= 2) && (pcd <= 5))
			LEDSetPattern(pcd);
		else
			ConsolePrintf("Invalid pattern code received: %d\n\r", pcd);
	}
	else if (msg->type == MSG_TYPE_SYNC_REQ) {
		// Sync request - generate sync response
		g_iSyncTime = TimeGetSystime();
	}
	else
		ConsolePrintf("Invalid message type received: %d\n\r", msg->type);
}

