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

#include "posFile.h"

/* Plugin includes */
#include "pud.h"
#include "configTools.h"

/* OLSR includes */

/* System includes */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <regex.h>
#include <sys/stat.h>

/** the maximal length of a line that is read from the file */
#define LINE_LENGTH 256

/** regular expression describing a comment */
static const char * regexCommentString = "^([[:space:]]*|[[:space:]#]+.*)$";

/** regular expression describing a key/value pair */
static const char * regexNameValueString =
		"^[[:space:]]*([^[:space:]]+)[[:space:]]*=[[:space:]]*([^[:space:]]+)[[:space:]]*$";

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
 * Initialises the positionFile reader.
 * @return true upon success, false otherwise
 */
bool startPositionFile(void) {
	if (started) {
		return true;
	}

	if (regcomp(&regexComment, regexCommentString, REG_EXTENDED | REG_ICASE)) {
		pudError(false, "Could not compile regex \"%s\"", regexCommentString);
		return false;
	}

	if (regcomp(&regexNameValue, regexNameValueString, REG_EXTENDED | REG_ICASE)) {
		pudError(false, "Could not compile regex \"%s\"", regexNameValueString);
		regfree(&regexComment);
		return false;
	}

	memset(&cachedStat, 0, sizeof(cachedStat));

	started = true;
	return true;
}

/**
 * Cleans up the positionFile reader.
 */
void stopPositionFile(void) {
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
		pudError(false, "Regex match failed: %s", msgbuf);
	}

	return false;
}

/** the buffer in which to store a line read from the file */
static char line[LINE_LENGTH];

/**
 * Read the position file
 * @param fileName the filename
 * @param nmeaInfo the NMEA data
 */
bool readPositionFile(char * fileName, NmeaInfo * nmeaInfo) {
	int fd;
	struct stat statBuf;
	FILE * fp = NULL;
	void * mtim;
	unsigned int lineNumber = 0;

	char * name = NULL;
	char * value = NULL;

	NmeaInfo result;
	bool retval = false;

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

	nmeaInfoClear(&result);
	nmeaTimeSet(&result.utc, &result.present, NULL);
	result.sig = POSFILE_DEFAULT_SIG;
	result.fix = POSFILE_DEFAULT_FIX;
	result.hdop = POSFILE_DEFAULT_HDOP;
	result.vdop = POSFILE_CALCULATED_VDOP(result.hdop);
	result.pdop = POSFILE_CALCULATED_PDOP(result.hdop);
	result.latitude = POSFILE_DEFAULT_LAT;
	result.longitude = POSFILE_DEFAULT_LON;
	result.elevation = POSFILE_DEFAULT_ELV;
	result.speed = POSFILE_DEFAULT_SPEED;
	result.track = POSFILE_DEFAULT_TRACK;
	result.mtrack = POSFILE_DEFAULT_MTRACK;
	result.magvar = POSFILE_DEFAULT_MAGVAR;

	memcpy(&cachedStat.timeStamp, mtim, sizeof(cachedStat.timeStamp));

	while (fgets(line, LINE_LENGTH, fp)) {
		regmatch_t pmatch[regexNameValuematchCount];

		lineNumber++;

		if (regexMatch(&regexComment, line, 0, NULL)) {
			continue;
		}

		if (!regexMatch(&regexNameValue, line, regexNameValuematchCount, pmatch)) {
			pudError(false, "Position file \"%s\", line %d uses invalid syntax: %s", fileName, lineNumber, line);
			goto out;
		}

		/* determine name/value */
		name = &line[pmatch[1].rm_so];
		line[pmatch[1].rm_eo] = '\0';
		value = &line[pmatch[2].rm_so];
		line[pmatch[2].rm_eo] = '\0';

		if (!strncasecmp(POSFILE_NAME_SIG, name, sizeof(line))) {
			if (!strncasecmp(POSFILE_VALUE_SIG_BAD, value, sizeof(line))) {
				result.sig = NMEALIB_SIG_INVALID;
			} else if (!strncasecmp(POSFILE_VALUE_SIG_LOW, value, sizeof(line))) {
				result.sig = NMEALIB_SIG_FIX;
			} else if (!strncasecmp(POSFILE_VALUE_SIG_MID, value, sizeof(line))) {
				result.sig = NMEALIB_SIG_DIFFERENTIAL;
			} else if (!strncasecmp(POSFILE_VALUE_SIG_HIGH, value, sizeof(line))) {
				result.sig = NMEALIB_SIG_SENSITIVE;
			} else {
				pudError(false, "Position file \"%s\", line %d uses an invalid value for \"%s\","
						" valid values are [%s|%s\%s|%s]", fileName, lineNumber, POSFILE_NAME_SIG,
						POSFILE_VALUE_SIG_BAD, POSFILE_VALUE_SIG_LOW, POSFILE_VALUE_SIG_MID, POSFILE_VALUE_SIG_HIGH);
				goto out;
			}
			nmeaInfoSetPresent(&result.present, NMEALIB_PRESENT_SIG);
		} else if (!strncasecmp(POSFILE_NAME_FIX, name, sizeof(line))) {
			if (!strncasecmp(POSFILE_VALUE_FIX_BAD, value, sizeof(line))) {
				result.fix = NMEALIB_FIX_BAD;
			} else if (!strncasecmp(POSFILE_VALUE_FIX_2D, value, sizeof(line))) {
				result.fix = NMEALIB_FIX_2D;
			} else if (!strncasecmp(POSFILE_VALUE_FIX_3D, value, sizeof(line))) {
				result.fix = NMEALIB_FIX_3D;
			} else {
				pudError(false, "Position file \"%s\", line %d uses an invalid value for \"%s\","
						" valid values are [%s\%s|%s]", fileName, lineNumber, POSFILE_NAME_FIX, POSFILE_VALUE_FIX_BAD,
						POSFILE_VALUE_FIX_2D, POSFILE_VALUE_FIX_3D);
				goto out;
			}
			nmeaInfoSetPresent(&result.present, NMEALIB_PRESENT_FIX);
		} else if (!strncasecmp(POSFILE_NAME_HDOP, name, sizeof(line))) {
			double val;
			if (!readDouble(POSFILE_NAME_HDOP, value, &val)) {
				goto out;
			}

			result.hdop = val;
			result.vdop = POSFILE_CALCULATED_VDOP(result.hdop);
			result.pdop = POSFILE_CALCULATED_PDOP(result.hdop);
			nmeaInfoSetPresent(&result.present, NMEALIB_PRESENT_HDOP);
			nmeaInfoSetPresent(&result.present, NMEALIB_PRESENT_VDOP);
			nmeaInfoSetPresent(&result.present, NMEALIB_PRESENT_PDOP);
		} else if (!strncasecmp(POSFILE_NAME_LAT, name, sizeof(line))) {
			double val;
			if (!readDouble(POSFILE_NAME_LAT, value, &val)) {
				goto out;
			}

			result.latitude = val;
			nmeaInfoSetPresent(&result.present, NMEALIB_PRESENT_LAT);
		} else if (!strncasecmp(POSFILE_NAME_LON, name, sizeof(line))) {
			double val;
			if (!readDouble(POSFILE_NAME_LON, value, &val)) {
				goto out;
			}

			result.longitude = val;
			nmeaInfoSetPresent(&result.present, NMEALIB_PRESENT_LON);
		} else if (!strncasecmp(POSFILE_NAME_ELV, name, sizeof(line))) {
			double val;
			if (!readDouble(POSFILE_NAME_ELV, value, &val)) {
				goto out;
			}

			result.elevation = val;
			nmeaInfoSetPresent(&result.present, NMEALIB_PRESENT_ELV);
		} else if (!strncasecmp(POSFILE_NAME_SPEED, name, sizeof(line))) {
			double val;
			if (!readDouble(POSFILE_NAME_SPEED, value, &val)) {
				goto out;
			}

			result.speed = val;
			nmeaInfoSetPresent(&result.present, NMEALIB_PRESENT_SPEED);
		} else if (!strncasecmp(POSFILE_NAME_TRACK, name, sizeof(line))) {
			double val;
			if (!readDouble(POSFILE_NAME_TRACK, value, &val)) {
				goto out;
			}

			result.track = val;
			nmeaInfoSetPresent(&result.present, NMEALIB_PRESENT_TRACK);
		} else if (!strncasecmp(POSFILE_NAME_MTRACK, name, sizeof(line))) {
			double val;
			if (!readDouble(POSFILE_NAME_MTRACK, value, &val)) {
				goto out;
			}

			result.mtrack = val;
			nmeaInfoSetPresent(&result.present, NMEALIB_PRESENT_MTRACK);
		} else if (!strncasecmp(POSFILE_NAME_MAGVAR, name, sizeof(line))) {
			double val;
			if (!readDouble(POSFILE_NAME_MAGVAR, value, &val)) {
				goto out;
			}

			result.magvar = val;
			nmeaInfoSetPresent(&result.present, NMEALIB_PRESENT_MAGVAR);
		} else {
			pudError(false, "Position file \"%s\", line %d uses an invalid option \"%s\","
					" valid options are [%s|%s|%s|%s|%s|%s|%s|%s|%s|%s]", fileName, lineNumber, name, POSFILE_NAME_SIG,
					POSFILE_NAME_FIX, POSFILE_NAME_HDOP, POSFILE_NAME_LAT, POSFILE_NAME_LON, POSFILE_NAME_ELV,
					POSFILE_NAME_SPEED, POSFILE_NAME_TRACK, POSFILE_NAME_MTRACK, POSFILE_NAME_MAGVAR);
			goto out;
		}
	}

	fclose(fp);
	fp = NULL;

	result.smask = POSFILE_DEFAULT_SMASK;
	nmeaInfoSetPresent(&result.present, NMEALIB_PRESENT_SMASK);

	nmeaInfoSanitise(&result);
	nmeaInfoUnitConversion(&result, true);

	memcpy(nmeaInfo, &result, sizeof(result));
	retval = true;

	out: if (fp) {
		fclose(fp);
	}
	if (fd >= 0) {
		close(fd);
	}
	return retval;
}
