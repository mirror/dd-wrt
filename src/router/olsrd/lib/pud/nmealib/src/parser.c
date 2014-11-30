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

#include <nmea/parser.h>

#include <nmea/parse.h>
#include <nmea/sentence.h>
#include <nmea/conversions.h>
#include <nmea/tok.h>

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#define first_eol_char  ('\r')
#define second_eol_char ('\n')

static void reset_sentence_parser(nmeaPARSER * parser, sentence_parser_state new_state) {
  assert(parser);
  memset(&parser->sentence_parser, 0, sizeof(parser->sentence_parser));
  parser->buffer.buffer[0] = '\0';
  parser->buffer.length = 0;
  parser->sentence_parser.has_checksum = false;
  parser->sentence_parser.state = new_state;
}

static inline bool isHexChar(char c) {
  switch (tolower(c)) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
      return true;

    default:
      break;
  }

  return false;
}

/**
 * Initialise the parser.
 * Allocates a buffer.
 *
 * @param parser a pointer to the parser
 * @return true (1) - success or false (0) - fail
 */
int nmea_parser_init(nmeaPARSER *parser) {
  assert(parser);
  memset(&parser->sentence, 0, sizeof(parser->sentence));
  reset_sentence_parser(parser, SKIP_UNTIL_START);
  return 1;
}

static bool nmea_parse_sentence_character(nmeaPARSER *parser, const char * c) {
  assert(parser);

  /* always reset when we encounter a start-of-sentence character */
  if (*c == '$') {
    reset_sentence_parser(parser, READ_SENTENCE);
    parser->buffer.buffer[parser->buffer.length++] = *c;
    return false;
  }

  /* just return when we haven't encountered a start-of-sentence character yet */
  if (parser->sentence_parser.state == SKIP_UNTIL_START) {
    return false;
  }

  /* this character belongs to the sentence */

  /* check whether the sentence still fits in the buffer */
  if (parser->buffer.length >= SENTENCE_SIZE) {
    reset_sentence_parser(parser, SKIP_UNTIL_START);
    return false;
  }

  parser->buffer.buffer[parser->buffer.length++] = *c;

  switch (parser->sentence_parser.state) {
    case READ_SENTENCE:
      if (*c == '*') {
        parser->sentence_parser.state = READ_CHECKSUM;
        parser->sentence_parser.sentence_checksum_chars_count = 0;
      } else if (*c == first_eol_char) {
        parser->sentence_parser.state = READ_EOL;
        parser->sentence_parser.sentence_eol_chars_count = 1;
      } else if (isInvalidNMEACharacter(c)) {
        reset_sentence_parser(parser, SKIP_UNTIL_START);
      } else {
        parser->sentence_parser.calculated_checksum ^= (int) *c;
      }
      break;

    case READ_CHECKSUM:
      if (!isHexChar(*c)) {
        reset_sentence_parser(parser, SKIP_UNTIL_START);
      } else {
        switch (parser->sentence_parser.sentence_checksum_chars_count) {
          case 0:
            parser->sentence_parser.sentence_checksum_chars[0] = *c;
            parser->sentence_parser.sentence_checksum_chars[1] = 0;
            parser->sentence_parser.sentence_checksum_chars_count = 1;
            break;

          case 1:
            parser->sentence_parser.sentence_checksum_chars[1] = *c;
            parser->sentence_parser.sentence_checksum_chars_count = 2;
            parser->sentence_parser.sentence_checksum = nmea_atoi(parser->sentence_parser.sentence_checksum_chars, 2, 16);
            parser->sentence_parser.has_checksum = true;
            parser->sentence_parser.state = READ_EOL;
            break;

          default:
            reset_sentence_parser(parser, SKIP_UNTIL_START);
            break;
          }
      }
      break;


    case READ_EOL:
      switch (parser->sentence_parser.sentence_eol_chars_count) {
        case 0:
          if (*c != first_eol_char) {
            reset_sentence_parser(parser, SKIP_UNTIL_START);
          } else {
            parser->sentence_parser.sentence_eol_chars_count = 1;
          }
          break;

        case 1:
          if (*c != second_eol_char) {
            reset_sentence_parser(parser, SKIP_UNTIL_START);
          } else {
            parser->sentence_parser.state = SKIP_UNTIL_START;
            return (!parser->sentence_parser.sentence_checksum_chars_count
                || (parser->sentence_parser.sentence_checksum_chars_count
                    && (parser->sentence_parser.sentence_checksum == parser->sentence_parser.calculated_checksum)));
          }
          break;

        default:
          reset_sentence_parser(parser, SKIP_UNTIL_START);
          break;
      }
      break;

      /* can't occur, but keep compiler happy */
      case SKIP_UNTIL_START:
      default:
        break;

  }

  return false;
}

/**
 * Parse a string and store the results in the nmeaINFO structure
 *
 * @param parser a pointer to the parser
 * @param s the string
 * @param len the length of the string
 * @param info a pointer to the nmeaINFO structure
 * @return the number of packets that were parsed
 */
int nmea_parse(nmeaPARSER * parser, const char * s, int len, nmeaINFO * info) {
  int sentences_count = 0;
  int charIndex = 0;

  assert(parser);
  assert(s);
  assert(info);

  for (charIndex = 0; charIndex < len; charIndex++) {
    bool sentence_read_successfully = nmea_parse_sentence_character(parser, &s[charIndex]);
    if (sentence_read_successfully) {
      enum nmeaPACKTYPE sentence_type = nmea_parse_get_sentence_type(&parser->buffer.buffer[1], parser->buffer.length - 1);
      switch (sentence_type) {
        case GPGGA:
          if (nmea_parse_GPGGA(parser->buffer.buffer, parser->buffer.length, parser->sentence_parser.has_checksum, &parser->sentence.gpgga)) {
            sentences_count++;
            nmea_GPGGA2info(&parser->sentence.gpgga, info);
          }
          break;

        case GPGSA:
          if (nmea_parse_GPGSA(parser->buffer.buffer, parser->buffer.length, parser->sentence_parser.has_checksum, &parser->sentence.gpgsa)) {
            sentences_count++;
            nmea_GPGSA2info(&parser->sentence.gpgsa, info);
          }
          break;

        case GPGSV:
          if (nmea_parse_GPGSV(parser->buffer.buffer, parser->buffer.length, parser->sentence_parser.has_checksum, &parser->sentence.gpgsv)) {
            sentences_count++;
            nmea_GPGSV2info(&parser->sentence.gpgsv, info);
          }
          break;

        case GPRMC:
          if (nmea_parse_GPRMC(parser->buffer.buffer, parser->buffer.length, parser->sentence_parser.has_checksum, &parser->sentence.gprmc)) {
            sentences_count++;
            nmea_GPRMC2info(&parser->sentence.gprmc, info);
          }
          break;

        case GPVTG:
          if (nmea_parse_GPVTG(parser->buffer.buffer, parser->buffer.length, parser->sentence_parser.has_checksum, &parser->sentence.gpvtg)) {
            sentences_count++;
            nmea_GPVTG2info(&parser->sentence.gpvtg, info);
          }
          break;

        case GPNON:
        default:
          break;
      }
    }
  }

  return sentences_count;
}
