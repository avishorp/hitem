
#ifndef __VERSION_H__
#define __VERSION_H__

#include <simplelink.h>

typedef struct {
	_u8 major;
	_u8 minor;
	_u8 patch;
	_u8 reserved;
} __attribute__((packed)) version_t;

// Get the current firmware version
void VersionGet(version_t* result);

// Set the current firmware version
void VersionSet(const version_t* result);

// Compare two versions. If the left is greater than the right, return 1 otherwise
// return 0
int VersionGreaterThan(const version_t* left, const version_t* right);

// Convert a version into a string. The result buffer should be long enough to contain
// the result (MMM.NNN.PPP = 12 byte)
void VersionToString(const version_t* ver, char* result);


#endif
