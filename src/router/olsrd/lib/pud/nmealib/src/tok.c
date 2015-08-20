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

#include <nmea/tok.h>

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NMEA_TOKS_COMPARE   1
#define NMEA_TOKS_PERCENT   2
#define NMEA_TOKS_WIDTH     3
#define NMEA_TOKS_TYPE      4

/** number conversion buffer size */
#define NMEA_CONVSTR_BUF    64

/**
 * Calculate crc control sum of a string.
 * If the string starts with a '$' then that character is skipped as per
 * the NMEA spec.
 *
 * @param s the string
 * @param len the length of the string
 * @return the crc
 */
int nmea_calc_crc(const char *s, const int len) {
  int chksum = 0;
  int it = 0;

  if (s[it] == '$')
    it++;

  for (; it < len; it++)
    chksum ^= (int) s[it];

  return chksum;
}

/**
 * Convert string to an integer
 *
 * @param s the string
 * @param len the length of the string
 * @param radix the radix of the numbers in the string
 * @return the converted number, or 0 on failure
 */
int nmea_atoi(const char *s, int len, int radix) {
	char *tmp_ptr;
	char buff[NMEA_CONVSTR_BUF];
	long res = 0;

	if (len < NMEA_CONVSTR_BUF) {
		memcpy(&buff[0], s, len);
		buff[len] = '\0';
		res = strtol(&buff[0], &tmp_ptr, radix);
	}

	return (int) res;
}

/**
 * Convert string to a floating point number
 *
 * @param s the string
 * @param len the length of the string
 * @return the converted number, or 0 on failure
 */
double nmea_atof(const char *s, const int len) {
	char *tmp_ptr;
	char buff[NMEA_CONVSTR_BUF];
	double res = 0;

	if (len < NMEA_CONVSTR_BUF) {
		memcpy(&buff[0], s, len);
		buff[len] = '\0';
		res = strtod(&buff[0], &tmp_ptr);
	}

	return res;
}

/**
 * Formating string (like standart printf) with CRC tail (*CRC)
 *
 * @param s the string buffer to printf into
 * @param len the size of the string buffer
 * @param format the string format to use
 * @return the number of printed characters
 */
int nmea_printf(char *s, int len, const char *format, ...) {
	int retval;
	int add = 0;
	va_list arg_ptr;

	if (len <= 0)
		return 0;

	va_start(arg_ptr, format);

	retval = vsnprintf(s, len, format, arg_ptr);

	if (retval > 0) {
		add = snprintf(s + retval, len - retval, "*%02x\r\n", nmea_calc_crc(s + 1, retval - 1));
	}

	retval += add;

	if (retval < 0 || retval > len) {
		memset(s, ' ', len);
		retval = len;
	}

	va_end(arg_ptr);

	return retval;
}

/**
 * Analyse a string (specific for NMEA sentences)
 *
 * @param s the string
 * @param len the length of the string
 * @param format the string format to use
 * @return the number of scanned characters
 */
int nmea_scanf(const char *s, int len, const char *format, ...) {
	const char *beg_tok;
	const char *end_buf = s + len;

	va_list arg_ptr;
	int tok_type = NMEA_TOKS_COMPARE;
	int width = 0;
	const char *beg_fmt = 0;
	int snum = 0, unum = 0;

	int tok_count = 0;
	void *parg_target;

	va_start(arg_ptr, format);

	for (; *format && s < end_buf; format++) {
		switch (tok_type) {
		case NMEA_TOKS_COMPARE:
			if ('%' == *format)
				tok_type = NMEA_TOKS_PERCENT;
			else if (*s++ != *format)
				goto fail;
			break;
		case NMEA_TOKS_PERCENT:
			width = 0;
			beg_fmt = format;
			tok_type = NMEA_TOKS_WIDTH;
			/* no break */
		case NMEA_TOKS_WIDTH:
			if (isdigit(*format))
				break;
			{
				/* No need to do 'tok_type = NMEA_TOKS_TYPE' since we'll do a fall-through */
				if (format > beg_fmt)
					width = nmea_atoi(beg_fmt, (int) (format - beg_fmt), 10);
			}
			/* no break */
		case NMEA_TOKS_TYPE:
			beg_tok = s;

			if (!width && ('c' == *format || 'C' == *format) && *s != format[1])
				width = 1;

			if (width) {
				if (s + width <= end_buf)
					s += width;
				else
					goto fail;
			} else {
				if (!format[1] || (0 == (s = (char *) memchr(s, format[1], end_buf - s))))
					s = end_buf;
			}

			if (s > end_buf)
				goto fail;

			tok_type = NMEA_TOKS_COMPARE;
			tok_count++;

			parg_target = 0;
			width = (int) (s - beg_tok);

			switch (*format) {
			case 'c':
			case 'C':
				parg_target = (void *) va_arg(arg_ptr, char *);
				if (width && 0 != (parg_target))
					*((char *) parg_target) = *beg_tok;
				break;
			case 's':
			case 'S':
				parg_target = (void *) va_arg(arg_ptr, char *);
				if (width && 0 != (parg_target)) {
					memcpy(parg_target, beg_tok, width);
					((char *) parg_target)[width] = '\0';
				}
				break;
			case 'f':
			case 'g':
			case 'G':
			case 'e':
			case 'E':
				parg_target = (void *) va_arg(arg_ptr, double *);
				if (width && 0 != (parg_target))
					*((double *) parg_target) = nmea_atof(beg_tok, width);
				break;
			default:
				break;
			}
			;

			if (parg_target)
				break;
			if (0 == (parg_target = (void *) va_arg(arg_ptr, int *)))
				break;
			if (!width)
				break;

			switch (*format) {
			case 'd':
			case 'i':
				snum = nmea_atoi(beg_tok, width, 10);
				memcpy(parg_target, &snum, sizeof(int));
				break;
			case 'u':
				unum = nmea_atoi(beg_tok, width, 10);
				memcpy(parg_target, &unum, sizeof(unsigned int));
				break;
			case 'x':
			case 'X':
				unum = nmea_atoi(beg_tok, width, 16);
				memcpy(parg_target, &unum, sizeof(unsigned int));
				break;
			case 'o':
				unum = nmea_atoi(beg_tok, width, 8);
				memcpy(parg_target, &unum, sizeof(unsigned int));
				break;
			default:
				goto fail;
			}
			;

			break;

		default:
			break;
		};
	}

	fail:

	va_end(arg_ptr);

	return tok_count;
}
