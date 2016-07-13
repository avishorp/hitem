
#include <stdlib.h>
#include <stdio.h>

#include "version.h"
#include "error.h"
#include "console.h"

#define VERSION_TO_INT(v) (((v->major) << 24) + ((v->minor) << 16) + ((v->patch) << 8))

const _u8 VERSION_FILENAME[] = "version.bin";

void VersionGet(version_t* result)
{
	result->reserved = 0;

	// Open the file
	_i32 handle = -1;
	_i32 ret = sl_FsOpen(VERSION_FILENAME, FS_MODE_OPEN_READ, NULL, &handle);

	if (ret < 0) {
		// If unable to open the version file, assume version in 0.0.0, this
		// is not considered an error
		result->major = 0;
		result->minor = 0;
		result->patch = 0;
		return;
	}

	// Read the data
	ret = sl_FsRead(handle, 0, (_u8*)result, sizeof(version_t));
	if (ret < 0)
		FatalError("Failed reading version file: %d\n\r", ret);

	// Close the file
	ret = sl_FsClose(handle, NULL, NULL, 0);
	if (ret < 0)
		FatalError("Failed closing version file: %d\n\r", ret);

}


void VersionSet(version_t* ver)
{
	// Open the file
	static _i32 handle = -1;
	_i32 ret = sl_FsOpen(VERSION_FILENAME,
			FS_MODE_OPEN_CREATE(512, _FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_PUBLIC_WRITE),
			NULL, &handle);

	if (ret < 0)
		FatalError("Failed opening version file for write: %d\n\r", ret);

	// Write the data
	ret = sl_FsWrite(handle, 0, (_u8*)ver, sizeof(version_t));
	if (ret < 0)
		FatalError("Failed writing version file: %d\n\r", ret);

	// Close the file
	ret = sl_FsClose(handle, NULL, NULL, 0);
	if (ret < 0)
		FatalError("Failed closing version file (write): %d\n\r", ret);
}

int VersionGreaterThan(const version_t* left, const version_t* right) {
	return (VERSION_TO_INT(left) > VERSION_TO_INT(right));
}

// Convert a version into a string. The result buffer should be long enough to contain
// the result (MMM.NNN.PPP = 12 byte)
void VersionToString(const version_t* ver, char* result)
{
	sprintf(result, "%d.%d.%d", ver->major, ver->minor, ver->patch);
}
