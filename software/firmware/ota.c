
// Implements OTA functionality

#include <stdlib.h>
#include <stdio.h>
#include <simplelink.h>

#include "ota.h"
#include "tftp/tftp.h"
#include "console.h"

#define DESCRIPTOR_FILE_NAME "hitem_fw.json"
#define DESCRIPTOR_MAX_FILE_SIZE 2048

typedef struct {
	int code;
	const char* description;
} error_code_t;

#define MKCODE(code) { code, #code }
const error_code_t error_table[] = {
		MKCODE(TFTPERROR_SOCKET),
		MKCODE(TFTPERROR_FAILED),
	    MKCODE(TFTPERROR_ERRORREPLY),
        MKCODE(TFTPERROR_BADPARAM),
        MKCODE(TFTPERROR_RESOURCES),
        MKCODE(TFTPERROR_OPCODE_FAILED),
        MKCODE(TFTPERROR_DATA_FAILED)
};


const char* _TFTPErrorStr(int code) {
	int i;
	static char _unknown_error_buf[20];

	for(i = 0; i < (sizeof(error_table)/sizeof(error_code_t)); i++) {
		if (code == error_table[i].code) {
			return error_table[i].description;
		}
	}
	sprintf(_unknown_error_buf, "UNKNOWN %d", code);
	return _unknown_error_buf;
}

void OTAExec(unsigned long remoteIP, unsigned short port)
{
	ConsolePrint("************ Starting OTA ************\n\r");

	// Step 1: Download to memory the package description file
	char* descriptor_buffer = (char*)malloc(DESCRIPTOR_MAX_FILE_SIZE);
	unsigned long descriptor_size = DESCRIPTOR_MAX_FILE_SIZE;
	unsigned short tftp_error;

	if (!descriptor_buffer) {
		ConsolePrint("Can't allocate a buffer for the descriptor\n\r");
		goto ABORT;
	}

	ConsolePrint("Requesting descriptor file\n\r");
	int r = sl_TftpRecv(remoteIP, port, DESCRIPTOR_FILE_NAME, descriptor_buffer, &descriptor_size, &tftp_error);

	if (r < 0) {
		if ((r == TFTPERROR_ERRORREPLY) && (descriptor_size > 0) && (descriptor_size < (DESCRIPTOR_MAX_FILE_SIZE-1))) {
			descriptor_buffer[descriptor_size] = 0;
			ConsolePrintf("TFTP Server returned error: %s\n\r", descriptor_buffer);
		}
		else
			ConsolePrintf("TFTP returned error %s, aborting\n\r", _TFTPErrorStr(r));

		goto ABORT;
	}
	else {
		if (descriptor_size > 0) {
			// Good descriptor has been received
			descriptor_buffer[descriptor_size] = 0;
			ConsolePrint("######################################################1\n\r");
			ConsolePrint(descriptor_buffer);
			ConsolePrint("######################################################2\n\r");
			ConsolePrintf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \n\r", descriptor_buffer[0],descriptor_buffer[1],descriptor_buffer[2],descriptor_buffer[3],descriptor_buffer[4],descriptor_buffer[5],descriptor_buffer[6],descriptor_buffer[7],descriptor_buffer[8],descriptor_buffer[9]);
		}
		else {
			ConsolePrint("Descriptor file too big, aborting\n\r");
			goto ABORT;
		}
	}
	return;

ABORT:
	return;

}
