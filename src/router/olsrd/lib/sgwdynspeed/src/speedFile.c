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

#include "speedFile.h"

/* Plugin includes */
#include "sgwDynSpeed.h"

/* OLSRD includes */
#include "olsr_cfg.h"
#include "gateway.h"

/* System includes */
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <regex.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <assert.h>

#define SPEED_UPLINK_NAME   "upstream"
#define SPEED_DOWNLINK_NAME "downstream"

/** the maximal length of a line that is read from the file */
#define LINE_LENGTH 256

/** regular expression describing a comment */
static const char * regexCommentString = "^([[:space:]]*|[[:space:]#]+.*)$";

/** regular expression describing a key/value pair */
static const char * regexNameValueString =
		"^[[:space:]]*([^[:space:]]+)[[:space:]]*=[[:space:]]*(.*?)[[:space:]]*$";

/** the number of matches in regexNameValueString */
static const size_t regexNameValuematchCount = 3;

/** the compiled regular expression describing a comment */
static regex_t regexComment;

/** the compiled regular expression describing a key/value pair */
static regex_t regexNameValue;

/** true when the plugin has been started */
static bool started = false;

/** type to hold the cached stat result */
typedef struct _CachedStat {
#if defined(__linux__) && !defined(__ANDROID__)
  struct timespec timeStamp; /* Time of last modification (full resolution) */
#else
  time_t timeStamp; /* Time of last modification (second resolution) */
#endif
} CachedStat;

/** the cached stat result */
static CachedStat cachedStat;

/**
 Read an unsigned long number from a value string

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
static bool readUL(const char * valueName, const char * value, unsigned long * valueNumber) {
	char * endPtr = NULL;
	unsigned long valueNew;

	assert(valueName != NULL);
	assert(value != NULL);
	assert(valueNumber != NULL);

	errno = 0;
	valueNew = strtoul(value, &endPtr, 10);

	if (!((endPtr != value) && (*value != '\0') && (*endPtr == '\0')) || (errno == ERANGE)) {
		/* invalid conversion */
		sgwDynSpeedError(false, "Value of parameter %s (%s) could not be converted to a number", valueName, value);
		return false;
	}

	*valueNumber = valueNew;

	return true;
}

/**
 * Strip EOL characters from a string
 *
 * @param str the string to strip
 * @param endindex the index of the \0 string terminator (end-of-string/strlen)
 */
static void stripEols(char * str, regoff_t endindex) {
  regoff_t len = endindex;
  while ((len > 0) && ((str[len - 1] == '\n') || (str[len - 1] == '\r'))) {
    len--;
  }
  str[len] = '\0';
}

/**
 * Initialises the speedFile reader.
 * @return true upon success, false otherwise
 */
bool startSpeedFile(void) {
	if (started) {
		return true;
	}

	if (regcomp(&regexComment, regexCommentString, REG_EXTENDED | REG_ICASE)) {
		sgwDynSpeedError(false, "Could not compile regex \"%s\"", regexCommentString);
		return false;
	}

	if (regcomp(&regexNameValue, regexNameValueString, REG_EXTENDED | REG_ICASE)) {
		sgwDynSpeedError(false, "Could not compile regex \"%s\"", regexNameValueString);
		regfree(&regexComment);
		return false;
	}

	memset(&cachedStat, 0, sizeof(cachedStat));

	started = true;
	return true;
}

/**
 * Cleans up the speedFile reader.
 */
void stopSpeedFile(void) {
	if (started) {
		regfree(&regexNameValue);
		regfree(&regexComment);
		started = false;
	}
}

/**
 * Performs a regex match
 * @param regex the compiled regex to match against
 * @param line the line to match
 * @param nmatch the number of matches to produce
 * @param pmatch the array with match information
 * @return true upon success, false otherwise
 */
static bool regexMatch(regex_t * regex, char * line, size_t nmatch, regmatch_t pmatch[]) {
	int result = regexec(regex, line, nmatch, pmatch, 0);
	if (!result) {
		return true;
	}

	if (result == REG_NOMATCH) {
		return false;
	}

	{
		char msgbuf[256];
		regerror(result, regex, msgbuf, sizeof(msgbuf));
		sgwDynSpeedError(false, "Regex match failed: %s", msgbuf);
	}

	return false;
}

/** the buffer in which to store a line read from the file */
static char line[LINE_LENGTH];

static bool reportedErrorsPrevious = false;

/**
 * Read the speed file
 * @param fileName the filename
 */
void readSpeedFile(char * fileName) {
	int fd;
	struct stat statBuf;
	FILE * fp = NULL;
	void * mtim;
	unsigned int lineNumber = 0;

	char * name = NULL;
	char * value = NULL;

	unsigned long uplink = DEF_UPLINK_SPEED;
	unsigned long downlink = DEF_DOWNLINK_SPEED;
	bool uplinkSet = false;
	bool downlinkSet = false;
	bool reportedErrors = false;

	fd = open(fileName, O_RDONLY);
	if (fd < 0) {
		/* could not open the file */
		memset(&cachedStat.timeStamp, 0, sizeof(cachedStat.timeStamp));
		goto out;
	}

	if (fstat(fd, &statBuf)) {
		/* could not stat the file */
		memset(&cachedStat.timeStamp, 0, sizeof(cachedStat.timeStamp));
		goto out;
	}

#if defined(__linux__) && !defined(__ANDROID__)
	mtim = &statBuf.st_mtim;
#else
	mtim = &statBuf.st_mtime;
#endif

	if (!memcmp(&cachedStat.timeStamp, mtim, sizeof(cachedStat.timeStamp))) {
		/* file did not change since last read */
		goto out;
	}

	fp = fdopen(fd, "r");
	if (!fp) {
		/* could not open the file */
		goto out;
	}

	memcpy(&cachedStat.timeStamp, mtim, sizeof(cachedStat.timeStamp));

	while (fgets(line, LINE_LENGTH, fp)) {
		regmatch_t pmatch[regexNameValuematchCount];

		lineNumber++;

		if (regexMatch(&regexComment, line, 0, NULL)) {
			continue;
		}

		if (!regexMatch(&regexNameValue, line, regexNameValuematchCount, pmatch)) {
			sgwDynSpeedError(false, "Gateway speed file \"%s\", line %d uses invalid syntax: ignored (%s)", fileName, lineNumber,
					line);
			continue;
		}

		stripEols(line, pmatch[2].rm_eo);

		/* determine name/value */
		name = &line[pmatch[1].rm_so];
		line[pmatch[1].rm_eo] = '\0';
		value = &line[pmatch[2].rm_so];
		line[pmatch[2].rm_eo] = '\0';

		if (!strncasecmp(SPEED_UPLINK_NAME, name, sizeof(line))) {
			if (!readUL(SPEED_UPLINK_NAME, value, &uplink)) {
				sgwDynSpeedError(false, "Gateway speed file \"%s\", line %d: %s value \"%s\" is not a valid number: ignored",
					fileName, lineNumber, SPEED_UPLINK_NAME, value);
				reportedErrors = true;
			} else {
				uplinkSet = true;
			}
		} else if (!strncasecmp(SPEED_DOWNLINK_NAME, name, sizeof(line))) {
			if (!readUL(SPEED_DOWNLINK_NAME, value, &downlink)) {
				sgwDynSpeedError(false, "Gateway speed file \"%s\", line %d: %s value \"%s\" is not a valid number: ignored",
					fileName, lineNumber, SPEED_DOWNLINK_NAME, value);
				reportedErrors = true;
			} else {
				downlinkSet = true;
			}
		} else {
		  if (!reportedErrorsPrevious) {
		    sgwDynSpeedError(false, "Gateway speed file \"%s\", line %d specifies an unknown option \"%s\": ignored",
		        fileName, lineNumber, name);
		    reportedErrors = true;
		  }
		}
	}

	reportedErrorsPrevious = reportedErrors;

	if (uplinkSet) {
	  smartgw_set_uplink(olsr_cnf, uplink);
	}
	if (downlinkSet) {
	  smartgw_set_downlink(olsr_cnf, downlink);
	}
	if (uplinkSet || downlinkSet) {
	  refresh_smartgw_netmask();
	}

	out: if (fp) {
		fclose(fp);
	}
	if (fd >= 0) {
		close(fd);
	}
}
