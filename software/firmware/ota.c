
// Implements OTA functionality

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <simplelink.h>

#include "ota.h"
#include "tftp/tftp.h"
#include "console.h"
#include "version.h"

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

// OTA Metadata - File entry
typedef struct {
	char source_filename[32];  // Source filename (to fetch using TFTP)
	char dest_filename[32];    // Destination filename (to write to storage)
	_u32 file_size;                  // File size in bytes
	_u8 checksum[16];                // MD5 Checksum
	_u16 reserved;                   // Unused
} __attribute__((packed)) ota_file_entry_t;

typedef struct {
	char magic[4];
	version_t version;
	_u8 file_count;
	ota_file_entry_t files[0];
} __attribute__((packed)) ota_metadata_t;


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

void ProcessOTAMetadata(const ota_metadata_t* omd, unsigned int size)
{
	// First, validate the magic code is correct
	if (memcmp("HTEM", omd->magic, 4) != 0) {
		ConsolePrint("Incorrect OTA Metadata header received (Incorrect Magic Code)\n\r");
	}

	// Check that the size of the metadata received makes sense
	unsigned int expected_size = sizeof(ota_metadata_t) + omd->file_count * sizeof(ota_file_entry_t);
	if (size != expected_size) {
		ConsolePrintf("Incorrect OTA Metadata header received (Incorrect size - expected %d, got %d)", expected_size, size);
		return;
	}

	// DEBUG - Dump the OTA metadata
	ConsolePrintf("Number of files: %d\n\r", omd->file_count);
	int i;
	for(i = 0; i < omd->file_count; i++) {
		ConsolePrintf("File No. %d\n\r", i);
		ConsolePrintf("  Source Filename: %s\n\r", omd->files[i].source_filename);
		ConsolePrintf("  Dest Filename: %s\n\r", omd->files[i].dest_filename);
		ConsolePrintf("  Size: %d\n\r", omd->files[i].file_size);
	}
}

void OTAExec(unsigned long remoteIP, unsigned short port, const char* filename)
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
	int r = sl_TftpRecv(remoteIP, port, filename, descriptor_buffer, &descriptor_size, &tftp_error);

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
			ProcessOTAMetadata((ota_metadata_t*)descriptor_buffer, descriptor_size);
		}
		else {
			ConsolePrint("Descriptor file too big, aborting\n\r");
			goto ABORT;
		}
	}

ABORT:
	if (descriptor_buffer)
		free(descriptor_buffer);

	return;

}
