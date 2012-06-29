/*
 * This file is part of nmealib.
 *
 * Copyright (c) 2011 Ferry Huberts
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <nmea/util.h>

#include <string.h>
#include <stdio.h>

/**
 * Determine whether the given string contains characters that are not allowed
 * for fields in an NMEA string.
 *
 * @param str
 * The string to check
 * @param strName
 * The name of the string to report when invalid characters are encountered
 * @param report
 * A pointer to a buffer in which to place the report string when an invalid
 * nmea character is detected
 * @param reportSize
 * The size of the report buffer
 *
 * @return
 * - true when the string has invalid characters
 * - false otherwise
 */
bool nmea_string_has_invalid_chars(const char * str, const char * strName,
		char * report, size_t reportSize) {
	static const char invalidChars[] = { '$', '*', ',', '!', '\\', '^', '~' };
	static const char * invalidCharsNames[] = { "sentence delimiter ($)",
			"checksum field delimiter (*)", "comma (,)", "exclamation mark (!)",
			"backslash (\\)", "^ (^)", "tilde (~)" };

	size_t i;
	size_t j;

	if (!str) {
		return false;
	}

	for (i = 0; i < strlen(str); i++) {
		char c = str[i];

		if ((c < 32) || (c > 126)) {
			if (report) {
				snprintf((char*) report, reportSize, "Configured %s (%s),"
						" character %lu, can not contain non-printable"
						" characters (codes outside the range [32, 126])",
						strName, str, (unsigned long)i + 1);
				report[reportSize - 1] = '\0';
			}
			return true;
		}

		for (j = 0; j < sizeof(invalidChars); j++) {
			if (c == invalidChars[j]) {
				if (report) {
					snprintf((char *) report, reportSize, "Configured %s (%s),"
							" character %lu, can not contain %s characters",
							strName, str, (unsigned long)i + 1, invalidCharsNames[j]);
					report[reportSize - 1] = '\0';
				}
				return true;
			}
		}
	}

	return false;
}
