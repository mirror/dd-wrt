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

#ifndef __NMEA_PARSER_H__
#define __NMEA_PARSER_H__

#include <nmea/info.h>
#include <nmea/sentence.h>

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef NMEA_MAX_SENTENCE_LENGTH
  /* override the default maximum sentence length */
  #define SENTENCE_SIZE (NMEA_MAX_SENTENCE_LENGTH)
#else
  /* we need to be able to parse much longer sentences than specified in the (original) specification */
  #define SENTENCE_SIZE (4096 * 1)
#endif

typedef enum _sentence_parser_state {
  SKIP_UNTIL_START,
  READ_SENTENCE,
  READ_CHECKSUM,
  READ_EOL
} sentence_parser_state;

/**
 * NMEA frame parser structure
 */
typedef struct _sentencePARSER {
    int sentence_checksum;
    int calculated_checksum;

    char sentence_checksum_chars[2];
    char sentence_checksum_chars_count;

    char sentence_eol_chars_count;

    bool has_checksum;

    sentence_parser_state state;
} sentencePARSER;

/**
 * parsed NMEA data and frame parser state
 */
typedef struct _nmeaPARSER {
    struct {
        unsigned int length;
        char buffer[SENTENCE_SIZE];
    } buffer;

    union {
        nmeaGPGGA gpgga;
        nmeaGPGSA gpgsa;
        nmeaGPGSV gpgsv;
        nmeaGPRMC gprmc;
        nmeaGPVTG gpvtg;
    } sentence;

    sentencePARSER sentence_parser;
} nmeaPARSER;

int nmea_parser_init(nmeaPARSER *parser);
int nmea_parse(nmeaPARSER * parser, const char * s, int len, nmeaINFO * info);

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __NMEA_PARSER_H__ */
