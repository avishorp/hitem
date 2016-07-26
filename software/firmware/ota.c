
// Implements OTA functionality

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <simplelink.h>

#include "ota.h"
#include "tftp/tftp.h"
#include "console.h"
#include "version.h"

#define METADATA_MAX_FILE_SIZE 2048

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



void ProcessOTAMetadata(unsigned long remoteIP, unsigned short port, const ota_metadata_t* omd, unsigned int size)
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

#ifdef 0
	// DEBUG - Dump the OTA metadata
	ConsolePrintf("Number of files: %d\n\r", omd->file_count);
	int i;
	for(i = 0; i < omd->file_count; i++) {
		ConsolePrintf("File No. %d\n\r", i);
		ConsolePrintf("  Source Filename: %s\n\r", omd->files[i].source_filename);
		ConsolePrintf("  Dest Filename: %s\n\r", omd->files[i].dest_filename);
		ConsolePrintf("  Size: %d\n\r", omd->files[i].file_size);
	}
#endif

	ConsolePrintf("OTA Upgrade contains %d files\n\r", omd->file_count);
	int j;
	for(j = 0; j < omd->file_count; j++) {
		unsigned short tftp_error;
		int r;

		ConsolePrintf("Requesting %s\n\r", omd->files[j].source_filename);
		r = sl_TftpRecv(remoteIP, port, omd->files[j].source_filename, omd->files[j].dest_filename, &omd->files[j].file_size, &tftp_error, 1);
		if (r < 0) {
			ConsolePrintf("TFTP returned error %s, aborting\n\r", TFTPErrorStr(r));
			return;
		}
	}

#ifdef 0
	_i32 hh;
	char bb[100];
	sl_FsOpen("test.txt", FS_MODE_OPEN_READ, 0, &hh);
	sl_FsRead(hh, 0, &bb, 100);
	sl_FsClose(hh, NULL, NULL, 0);
	ConsolePrint(bb);
#endif

}

void OTAExec(unsigned long remoteIP, unsigned short port, const char* filename)
{
	ConsolePrint("************ Starting OTA ************\n\r");

	// Step 1: Download to memory the package description file
	char* metadata_buffer = (char*)malloc(METADATA_MAX_FILE_SIZE);
	unsigned long metadata_size = METADATA_MAX_FILE_SIZE;
	unsigned short tftp_error;

	if (!metadata_buffer) {
		ConsolePrint("Can't allocate a buffer for the descriptor\n\r");
		goto ABORT;
	}

	ConsolePrint("Requesting descriptor file\n\r");
	int r = sl_TftpRecv(remoteIP, port, filename, metadata_buffer, &metadata_size, &tftp_error, 0);

	if (r < 0) {
		if ((r == TFTPERROR_ERRORREPLY) && (metadata_size > 0) && (metadata_size < (METADATA_MAX_FILE_SIZE-1))) {
			metadata_buffer[metadata_size] = 0;
			ConsolePrintf("TFTP Server returned error: %s\n\r", metadata_buffer);
		}
		else
			ConsolePrintf("TFTP returned error %s, aborting\n\r", TFTPErrorStr(r));

		goto ABORT;
	}
	else {
		if (metadata_size > 0) {
			// Good descriptor has been received
			ProcessOTAMetadata(remoteIP, port, (ota_metadata_t*)metadata_buffer, metadata_size);
		}
		else {
			ConsolePrint("Descriptor file too big, aborting\n\r");
			goto ABORT;
		}
	}

ABORT:
	if (metadata_buffer)
		free(metadata_buffer);

	return;

}
