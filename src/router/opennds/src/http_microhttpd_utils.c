/************************************************************************\
 * This program is free software; you can redistribute it and/or	*
 * modify it under the terms of the GNU General Public License as	*
 * published by the Free:Software Foundation; either version 2 of	*
 * the License, or (at your option) any later version.			*
 *									*
 * This program is distributed in the hope that it will be useful,	*
 * but WITHOUT ANY WARRANTY; without even the implied warranty of	*
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.		*
 * See the GNU General Public License for more details.		*
\************************************************************************/

/** @internal
 * @file http_microhttpd_utils.c
 * @brief a httpd implementation using libmicrohttpd
 * @author Copyright (C) 2015 Alexander Couzens <lynxis@fe80.eu>
 * @author Copyright (C) 2015-2023 The openNDS contributors <opennds@blue-wave.net>
 * @author Copyright (C) 2015-2023 Modifications and additions by BlueWave Projects and Services <opennds@blue-wave.net>
 */

#include <ctype.h>
#include "debug.h"

/* blen is the size of buf; slen is the length of src.	The input-string need
 * not be, and the output string will not be, null-terminated.	Returns the
 * length of the encoded string, or -1 on error (buffer overflow)
 */
int htmlentityencode(char *buf, int blen, const char *src, int slen)
{
	int i;
	int len = 0;

	for (i = 0; (i < slen) && (len < blen); i++) {

		if ((len+5) <= blen) {

			if (src[i] == '"') {
				buf[len++] = '&';
				buf[len++] = '#';
				buf[len++] = '3';
				buf[len++] = '4';
				buf[len++] = ';';

			} else if (src[i] == '#') {
				buf[len++] = '&';
				buf[len++] = '#';
				buf[len++] = '3';
				buf[len++] = '5';
				buf[len++] = ';';

			} else if (src[i] == '$') {
				buf[len++] = '&';
				buf[len++] = '#';
				buf[len++] = '3';
				buf[len++] = '6';
				buf[len++] = ';';

			} else if (src[i] == '&') {
				buf[len++] = '&';
				buf[len++] = '#';
				buf[len++] = '3';
				buf[len++] = '8';
				buf[len++] = ';';

			} else if (src[i] == '\'') {
				buf[len++] = '&';
				buf[len++] = '#';
				buf[len++] = '3';
				buf[len++] = '9';
				buf[len++] = ';';

			} else if (src[i] == '+') {
				buf[len++] = '&';
				buf[len++] = '#';
				buf[len++] = '4';
				buf[len++] = '3';
				buf[len++] = ';';

			} else if (src[i] == '<') {
				buf[len++] = '&';
				buf[len++] = '#';
				buf[len++] = '6';
				buf[len++] = '0';
				buf[len++] = ';';

			} else if (src[i] == '>') {
				buf[len++] = '&';
				buf[len++] = '#';
				buf[len++] = '6';
				buf[len++] = '2';
				buf[len++] = ';';

			} else {

				buf[len++] = src[i];
			}

		} else {

			len = -1;
			debug(LOG_ERR, "Buffer overflow in htmlentityencode");
			break;
		}
	}

	debug(LOG_DEBUG, "HTML Entity encoded string: %s, length: %d", buf, len);
	return (i == slen) ? len : -1;
}


/* blen is the size of buf; slen is the length of src. The input-string need
** not be, and the output string will not be, null-terminated. Returns the
** length of the decoded string, -1 on buffer overflow, -2 on malformed string. */
int uh_urldecode(char *buf, int blen, const char *src, int slen)
{
	int i;
	int len = 0;

#define hex(x) \
	(((x) <= '9') ? ((x) - '0') : \
		(((x) <= 'F') ? ((x) - 'A' + 10) : \
			((x) - 'a' + 10)))

	for (i = 0; (i < slen) && (len < blen); i++) {

		if (src[i] != '%') {
			buf[len++] = src[i];
			continue;
		}

		if (i + 2 >= slen || !isxdigit(src[i + 1]) || !isxdigit(src[i + 2]))
			return -2;

		buf[len++] = (char)(16 * hex(src[i+1]) + hex(src[i+2]));
		i += 2;
	}

	buf[len] = 0;

	return (i == slen) ? len : -1;
}

/* blen is the size of buf; slen is the length of src.	The input-string need
 * not be, and the output string will not be, null-terminated.	Returns the
 * length of the encoded string, or -1 on error (buffer overflow)
 */
int uh_urlencode(char *buf, int blen, const char *src, int slen)
{
	int i;
	int len = 0;
	static const char hex[] = "0123456789abcdef";

	for (i = 0; (i < slen) && (len < blen); i++) {

		if (isalnum(src[i]) || (src[i] == '-') || (src[i] == '_') ||
				(src[i] == '.') || (src[i] == '~')) {
			buf[len++] = src[i];

		} else if ((len+3) <= blen) {
			buf[len++] = '%';
			buf[len++] = hex[(src[i] >> 4) & 15];
			buf[len++] = hex[ src[i] & 15];

		} else {
			len = -1;
			debug(LOG_ERR, "Buffer overflow in uh_urlencode");
			break;
		}
	}

	debug(LOG_DEBUG, "URL encoded string: %s, length: %d", buf, len);
	return (i == slen) ? len : -1;
}


int b64_encode(char *buf, int blen, const char *src, int slen)
{
	int  i;
	int  v;
	int len = 0;
	const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	debug(LOG_DEBUG, "string to b64 encode: %s length %d", src, slen);

	for (i=0, len=0; i<slen; i+=3, len+=4) {
		if ((len+4) <= blen) {
			v = src[i];
			v = i+1 < slen ? v << 8 | src[i+1] : v << 8;
			v = i+2 < slen ? v << 8 | src[i+2] : v << 8;

			buf[len]   = b64chars[(v >> 18) & 0x3F];
			buf[len+1] = b64chars[(v >> 12) & 0x3F];

			if (i+1 < slen) {
				buf[len+2] = b64chars[(v >> 6) & 0x3F];
			} else {
				buf[len+2] = '=';
			}

			if (i+2 < slen) {
				buf[len+3] = b64chars[v & 0x3F];
			} else {
				buf[len+3] = '=';
			}
		} else {
			debug(LOG_ERR, "Buffer overflow in b64_encode");
			break;
		}
	}

	debug(LOG_DEBUG, "b64 encoded string: %s, encoded length: %d", buf, len);

	return (i == slen) ? (len + 4) : -1;
}

int uh_b64decode(char *buf, int blen, const void *src, int slen)
{
	const unsigned char *str = src;
	unsigned int cout = 0;
	unsigned int cin = 0;
	int len = 0;
	int i = 0;

	for (i = 0; (i <= slen) && (str[i] != 0); i++) {
		cin = str[i];

		if ((cin >= '0') && (cin <= '9'))
			cin = cin - '0' + 52;
		else if ((cin >= 'A') && (cin <= 'Z'))
			cin = cin - 'A';
		else if ((cin >= 'a') && (cin <= 'z'))
			cin = cin - 'a' + 26;
		else if (cin == '+')
			cin = 62;
		else if (cin == '/')
			cin = 63;
		else if (cin == '=')
			cin = 0;
		else
			continue;

		cout = (cout << 6) | cin;

		if ((i % 4) != 3)
			continue;

		if ((len + 3) >= blen)
			break;

		buf[len++] = (char)(cout >> 16);
		buf[len++] = (char)(cout >> 8);
		buf[len++] = (char)(cout);
	}

	debug(LOG_DEBUG, "b64 decoded string: %s, decoded length: %d", buf, len);
	buf[len++] = 0;
	return len;
}
