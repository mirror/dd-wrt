#include "configuration.h"

/* Plugin includes */
#include "sgwDynSpeed.h"

/* OLSR includes */

/* System includes */
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <assert.h>

/*
 * Utility functions
 */

/**
 Read an unsigned long long number from a value string

 @param valueName
 the name of the value
 @param value
 the string to convert to a number
 @param valueNumber
 a pointer to the location where to store the number upon successful conversion

 @return
 - true on success
 - false otherwise
 */
static bool readULL(const char * valueName, const char * value, unsigned long long * valueNumber) {
	char * endPtr = NULL;
	unsigned long long valueNew;

	errno = 0;
	valueNew = strtoull(value, &endPtr, 10);

	if (!((endPtr != value) && (*value != '\0') && (*endPtr == '\0')) || (errno == ERANGE)) {
		/* invalid conversion */
		sgwDynSpeedError(true, "Configured %s (%s) could not be converted to a number", valueName, value);
		return false;
	}

	*valueNumber = valueNew;

	return true;
}

/*
 * speedFile
 */

/** The speedFile buffer */
static char speedFile[PATH_MAX];

/** True when the speedFile is set */
static bool speedFileSet = false;

/**
 @return
 The speed file filename. returns NULL if not set
 */
char * getSpeedFile(void) {
	if (!speedFileSet) {
		return NULL;
	}

	return &speedFile[0];
}

/**
 Set the speed file filename.

 @param value
 The value of the speed file filename to set
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setSpeedFile(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	size_t valueLength;

	if (!value) {
		speedFileSet = false;
		return false;
	}

	valueLength = strlen(value);
	if (!valueLength) {
		speedFileSet = false;
		return false;
	}

	if (valueLength > (PATH_MAX - 1)) {
		sgwDynSpeedError(false, "Configured %s is too long, maximum length is"
				" %u, current length is %lu", SGWDYNSPEED_SPEEDFILE_NAME, PATH_MAX, (unsigned long) valueLength);
		return true;
	}

	strcpy(&speedFile[0], value);
	speedFileSet = true;

	return false;
}

/*
 * speedFilePeriod
 */

/** The speedFilePeriod value (milliseconds) */
unsigned long long speedFilePeriod = SGWDYNSPEED_SPEEDFILEPERIOD_DEFAULT;

/**
 @return
 The speedFilePeriod (in milliseconds)
 */
unsigned long long getSpeedFilePeriod(void) {
	return speedFilePeriod;
}

/**
 Set the speedFilePeriod

 @param value
 The speedFilePeriod (a number in string representation)
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setSpeedFilePeriod(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = SGWDYNSPEED_SPEEDFILEPERIOD_NAME;
	unsigned long long speedFilePeriodNew;

	assert(value != NULL);

	if (!readULL(valueName, value, &speedFilePeriodNew)) {
		return true;
	}

	if ((speedFilePeriodNew < SGWDYNSPEED_SPEEDFILEPERIOD_MIN)
			|| (speedFilePeriodNew > SGWDYNSPEED_SPEEDFILEPERIOD_MAX)) {
		sgwDynSpeedError(false, "Configured %s (%llu) is outside of"
				" valid range %llu-%llu", valueName, speedFilePeriodNew, SGWDYNSPEED_SPEEDFILEPERIOD_MIN,
				SGWDYNSPEED_SPEEDFILEPERIOD_MAX);
		return true;
	}

	speedFilePeriod = speedFilePeriodNew;

	return false;
}
