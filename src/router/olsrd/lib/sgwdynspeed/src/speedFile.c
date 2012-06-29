#include "speedFile.h"

/* Plugin includes */
#include "sgwDynSpeed.h"

/* OLSRD includes */
#include "olsr_cfg.h"

/* System includes */
#include <stddef.h>
#include <stdlib.h>
#include <regex.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>

#define SPEED_UPLINK_NAME   "upstream"
#define SPEED_DOWNLINK_NAME "downstream"

/** the maximal length of a line that is read from the file */
#define LINE_LENGTH 256

/** regular expression describing a comment */
static const char * regexCommentString = "^([[:space:]]*|[[:space:]#]+.*)$";

/** regular expression describing a key/value pair */
static const char * regexNameValueString =
		"^[[:space:]]*([^[:space:]]+)[[:space:]]*=[[:space:]]*([[:digit:]]+)[[:space:]]*$";

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
	struct timespec st_mtim; /* Time of last modification. */
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

	errno = 0;
	valueNew = strtoul(value, &endPtr, 10);

	if (!((endPtr != value) && (*value != '\0') && (*endPtr == '\0'))) {
		/* invalid conversion */
		sgwDynSpeedError(true, "Configured %s (%s) could not be converted to a number", valueName, value);
		return false;
	}

	*valueNumber = valueNew;

	return true;
}

/**
 * Initialises the speedFile reader.
 * @return true upon success, false otherwise
 */
bool startSpeedFile(void) {
	int result;

	if (started) {
		return true;
	}

	result = regcomp(&regexComment, regexCommentString, REG_EXTENDED | REG_ICASE);
	if (result) {
		char msgbuf[256];
		regerror(result, &regexComment, msgbuf, sizeof(msgbuf));
		sgwDynSpeedError(false, "Could not compile regex \"%s\": %s", regexCommentString, msgbuf);
		return false;
	}

	result = regcomp(&regexNameValue, regexNameValueString, REG_EXTENDED | REG_ICASE);
	if (result) {
		char msgbuf[256];
		regerror(result, &regexNameValue, msgbuf, sizeof(msgbuf));
		sgwDynSpeedError(false, "Could not compile regex \"%s\": %s", regexNameValueString, msgbuf);
		regfree(&regexComment);
		return false;
	}

	cachedStat.st_mtim.tv_sec = -1;
	cachedStat.st_mtim.tv_nsec = -1;

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

/**
 * Read the speed file
 * @param fileName the filename
 */
void readSpeedFile(char * fileName) {
	struct stat statBuf;
	FILE * fd = NULL;
	unsigned int lineNumber = 0;
	char * name = NULL;
	char * value = NULL;
	unsigned long uplink = DEF_UPLINK_SPEED;
	unsigned long downlink = DEF_DOWNLINK_SPEED;
	bool uplinkSet = false;
	bool downlinkSet = false;

	if (stat(fileName, &statBuf)) {
		/* could not access the file */
		goto out;
	}

	if (!memcmp(&cachedStat.st_mtim, &statBuf.st_mtim, sizeof(cachedStat.st_mtim))) {
		/* file did not change since last read */
		goto out;
	}

	fd = fopen(fileName, "r");
	if (!fd) {
		goto out;
	}

	memcpy(&cachedStat.st_mtim, &statBuf.st_mtim, sizeof(cachedStat.st_mtim));

	while (fgets(line, LINE_LENGTH, fd)) {
		regmatch_t pmatch[regexNameValuematchCount];

		lineNumber++;

		if (regexMatch(&regexComment, line, 0, NULL)) {
			continue;
		}

		if (!regexMatch(&regexNameValue, line, regexNameValuematchCount, pmatch)) {
			sgwDynSpeedError(false, "Gateway speed file \"%s\", line %d uses invalid syntax: %s", fileName, lineNumber,
					line);
			goto out;
		}

		/* determine name/value */
		name = &line[pmatch[1].rm_so];
		line[pmatch[1].rm_eo] = '\0';
		value = &line[pmatch[2].rm_so];
		line[pmatch[2].rm_eo] = '\0';

		if (!strncasecmp(SPEED_UPLINK_NAME, name, sizeof(line))) {
			if (!readUL(SPEED_UPLINK_NAME, value, &uplink)) {
				goto out;
			}
			uplinkSet = true;
		} else if (!strncasecmp(SPEED_DOWNLINK_NAME, name, sizeof(line))) {
			if (!readUL(SPEED_DOWNLINK_NAME, value, &downlink)) {
				goto out;
			}
			downlinkSet = true;
		} else {
			sgwDynSpeedError(false, "Gateway speed file \"%s\", line %d uses an invalid option \"%s\","
					" valid options are [%s|%s]", fileName, lineNumber, name, SPEED_UPLINK_NAME, SPEED_DOWNLINK_NAME);
			goto out;
		}
	}

	fclose(fd);

	if (uplinkSet) {
		olsr_cnf->smart_gw_uplink = uplink;
	}
	if (downlinkSet) {
		olsr_cnf->smart_gw_downlink = downlink;
	}

	out: return;
}
