/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

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
