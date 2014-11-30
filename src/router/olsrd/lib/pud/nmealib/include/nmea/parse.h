/*
 * This file is part of nmealib.
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

#ifndef __NMEA_PARSE_H__
#define __NMEA_PARSE_H__

#include <nmea/sentence.h>

#include <stdbool.h>
#include <stddef.h>

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

const char * isInvalidNMEACharacter(const char * c);
const char * nmea_parse_sentence_has_invalid_chars(const char * s, const size_t len);

enum nmeaPACKTYPE nmea_parse_get_sentence_type(const char *s, const int len);

int nmea_parse_GPGGA(const char *s, const int len, bool has_checksum, nmeaGPGGA *pack);
int nmea_parse_GPGSA(const char *s, const int len, bool has_checksum, nmeaGPGSA *pack);
int nmea_parse_GPGSV(const char *s, const int len, bool has_checksum, nmeaGPGSV *pack);
int nmea_parse_GPRMC(const char *s, const int len, bool has_checksum, nmeaGPRMC *pack);
int nmea_parse_GPVTG(const char *s, const int len, bool has_checksum, nmeaGPVTG *pack);

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __NMEA_PARSE_H__ */
