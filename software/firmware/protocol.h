// Endpoint-server communication protocol
//
// The communication between the endpoint and the server is done over TCP/IP channel.
// Every message (in either direction) has a fixed format:
//   - 1 byte prolog (constant 0x85)
//   - 1 byte message type
//   - 4 bytes payload
//   - 1 byte checksum - calculated over all the bytes except for the checksum itself
//
// Message Types:
//   * WELCOME [Endpoint->Server] - Sent by the endpoint to the server. Includes the endpoint identification
//               number and personality code (hammer or hat)
//   * SET_COLOR [Server->Endpoint] - Sets the endpoint color
//   * INDICATE [Server->Endpoint] - Generates short term indications
//   * SYNC_RSP [Endpoint->Server] - Sync response (the sync request is broadcasted on
//                a different UDP channel to all endpoints). Includes the timestamp captured
//                when the sync request was received.


#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <simplelink.h>
#include "time.h"

#define MSG_PROLOG "HTEM"
#define MSG_TYPE_WELCOME    1
#define MSG_TYPE_SET_COLOR  2
#define MSG_TYPE_INDICATE   3
#define MSG_TYPE_SYNC_RSP   4
#define MSG_TYPE_SYNC_REQ   5
#define MSG_TYPE_HIT        6
#define MSG_TYPE_BAT_REPORT 7
#define MSG_TYPE_SET_THRESH 8
#define MSG_TYPE_KEEPALIVE  99

typedef struct  {
	_u8 prolog[4];
	_u8 type;
	union {
		// Raw data
		_u8 raw[4];

		// WELCOME payload
		struct {
			_u8 endpoint_id;
			_u8 endpoint_personality;
			//_u8 padding[2];
		} welcome;

		// SET_COLOR payload
		struct {
			_u8 color_code;
			_u8 intensity;
			//_u8 padding[2];
		} set_color;

		// INDICATE payload
		struct {
			_u8 indication_code;
			_u8 padding[3];
		} indicate;

		// SYNC_RSP/HIT payload
		_u32 timestamp;

		// BAT_REPORT payload
		struct {
			_u16 battery_voltage;
			_u16 battery_raw;
		} bat_report;

		// SET_THRESH payload
		struct {
			_u16 threshold;
			_u16 debounce_power;
		} set_threshold;
	} payload;
	_u8 checksum;
} __attribute__((packed)) message_t;

#define MESSAGE_LENGTH (sizeof(message_t))

void ProtocolInit();
void ProtocolParse(const char* buf, int len);
_i16 ProtocolSendWelcome(_i16 sock);
_i16 ProtocolSendSyncResp(_i16 sock, systime_t stime);
_i16 ProtocolSendHit(_i16 sock, systime_t time);
_i16 ProtocolSendBatReport(_i16 sock, int voltage, _u16 raw);
_i16 ProtocolSendKeepalive(_i16 sock);
systime_t ProtocolGetSyncTime();



#endif
