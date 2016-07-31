
// Implements OTA functionality

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <simplelink.h>

#include "hw_types.h"
#include "prcm.h"

#include "ota.h"
#include "tftp/tftp.h"
#include "console.h"
#include "version.h"

#define METADATA_MAX_FILE_SIZE 2048
#define NUM_RETRIES            5
#define COPY_BUFFER_SIZE       512

const char tempFilename[] = "download.tmp";

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


// Copy the given file to the source.
int MoveFile(const char* sourceFilename, const char* destFilename)
{
 	SlFsFileInfo_t source_info;
 	int r;

 	// Get the source file length
 	r = sl_FsGetInfo((const _u8*)sourceFilename, NULL, &source_info);
 	if (r <  0)
 		return r;

 	// Create the destination file
	_i32 dest_handle = -1;
	r = sl_FsOpen((const unsigned char*)destFilename,
			FS_MODE_OPEN_CREATE(source_info.AllocatedLen, _FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_PUBLIC_WRITE),
			NULL, &dest_handle);
	if (r < 0)
		return r;

	// Open the source file for read
	_i32 source_handle = -1;
	r = sl_FsOpen((const unsigned char*)sourceFilename, FS_MODE_OPEN_READ, NULL, &source_handle);
	if (r < 0) {
		sl_FsClose(dest_handle, NULL, NULL, 0);
		return r;
	}

	unsigned long remaining = source_info.FileLen;
	unsigned long offset = 0;

	_u8* copy_buffer = malloc(COPY_BUFFER_SIZE);
	if (!copy_buffer) {
		sl_FsClose(source_handle, NULL, NULL, 0);
		sl_FsClose(dest_handle, NULL, NULL, 0);
		return -1;
	}

	while (remaining > 0) {
		unsigned long copy_size = (remaining >= COPY_BUFFER_SIZE)? COPY_BUFFER_SIZE : remaining;

		// Read a block from the source file
		r = sl_FsRead(source_handle, offset, copy_buffer, copy_size);
		if (r < 0) {
			sl_FsClose(source_handle, NULL, NULL, 0);
			sl_FsClose(dest_handle, NULL, NULL, 0);
			free(copy_buffer);
			return r;
		}

		// Write the block to the destination file
		r = sl_FsWrite(dest_handle, offset, copy_buffer, copy_size);
		if (r < 0) {
			sl_FsClose(source_handle, NULL, NULL, 0);
			sl_FsClose(dest_handle, NULL, NULL, 0);
			free(copy_buffer);
			return r;
		}

		offset += copy_size;
		remaining -= copy_size;
	}

	// Close the files
	sl_FsClose(source_handle, NULL, NULL, 0);
	sl_FsClose(dest_handle, NULL, NULL, 0);
	free(copy_buffer);

	// Delete the source file
	sl_FsDel((const unsigned char*)sourceFilename, NULL);

	return 0;

}

int ProcessOTAMetadata(unsigned long remoteIP, unsigned short port, const ota_metadata_t* omd, unsigned int size)
{
	// First, validate the magic code is correct
	if (memcmp("HTEM", omd->magic, 4) != 0) {
		ConsolePrint("Incorrect OTA Metadata header received (Incorrect Magic Code)\n\r");
		return 0;
	}

	// Check that the size of the metadata received makes sense
	unsigned int expected_size = sizeof(ota_metadata_t) + omd->file_count * sizeof(ota_file_entry_t);
	if (size != expected_size) {
		ConsolePrintf("Incorrect OTA Metadata header received (Incorrect size - expected %d, got %d)", expected_size, size);
		return 0;
	}

#ifdef PRINT_OTA_METADATA
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
		int retries = NUM_RETRIES;

		while (retries > 0) {
			ConsolePrintf("Requesting %s\n\r", omd->files[j].source_filename);
			r = sl_TftpRecv(remoteIP, port, omd->files[j].source_filename, (char*)tempFilename, (unsigned long*)&omd->files[j].file_size, &tftp_error, 1, omd->files[j].checksum);
			if (r < 0) {
				ConsolePrintf("TFTP returned error %s, retrying\n\r", TFTPErrorStr(r));
				retries--;
			}
			else {
				// Success
				r = MoveFile(tempFilename, omd->files[j].dest_filename);

				// Move the file to its final filename
				if (r < 0) {
					ConsolePrint("Failed moving file, aborting.\n\r");
					return 0;
				}
				break;
			}
		}

		if (retries == 0) {
			ConsolePrint("Upgrade failed, aborting\n\r");
			return 0;
		}
	}

	// Finally, set the current version number to the one of the new FW
	VersionSet(&omd->version);

	return 1;

}

int OTAExec(unsigned long remoteIP, unsigned short port, const char* filename)
{
	ConsolePrint("************ Starting OTA ************\n\r");

	int rc = 1;
	int r = 0;

	// Step 1: Download to memory the package description file
	char* metadata_buffer = (char*)malloc(METADATA_MAX_FILE_SIZE);
	unsigned long metadata_size = METADATA_MAX_FILE_SIZE;
	unsigned short tftp_error;

	if (!metadata_buffer) {
		ConsolePrint("Can't allocate a buffer for the descriptor\n\r");
		rc = 0;
		goto ABORT;
	}

	ConsolePrint("Requesting descriptor file\n\r");
	r = sl_TftpRecv(remoteIP, port, filename, metadata_buffer, &metadata_size, &tftp_error, 0, NULL);

	if (r < 0) {
		if ((r == TFTPERROR_ERRORREPLY) && (metadata_size > 0) && (metadata_size < (METADATA_MAX_FILE_SIZE-1))) {
			metadata_buffer[metadata_size] = 0;
			ConsolePrintf("TFTP Server returned error: %s\n\r", metadata_buffer);
		}
		else
			ConsolePrintf("TFTP returned error %s, aborting\n\r", TFTPErrorStr(r));

		rc = 0;
		goto ABORT;
	}
	else {
		if (metadata_size > 0) {
			// Good descriptor has been received
			rc = ProcessOTAMetadata(remoteIP, port, (ota_metadata_t*)metadata_buffer, metadata_size);
		}
		else {
			ConsolePrint("Descriptor file too big, aborting\n\r");
			rc = 0;
			goto ABORT;
		}
	}


ABORT:
	if (metadata_buffer)
		free(metadata_buffer);

	return rc;

}
