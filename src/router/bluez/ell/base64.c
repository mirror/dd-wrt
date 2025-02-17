/*
 * Embedded Linux library
 * Copyright (C) 2015  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdint.h>

#include "utf8.h"
#include "base64.h"
#include "private.h"
#include "useful.h"

#include <stdio.h>

LIB_EXPORT uint8_t *l_base64_decode(const char *in, size_t in_len,
					size_t *n_written)
{
	const char *ptr, *in_end = in + in_len;
	const char *base64_end = NULL;
	uint8_t *out_buf, *out;
	int base64_len = 0, pad_len = 0;
	uint16_t reg = 0;

	for (ptr = in; ptr < in_end; ptr++) {
		if (l_ascii_isspace(*ptr))
			/* Whitespace */
			continue;
		else if (*ptr == '=') {
			/* Final padding */
			if (!pad_len)
				base64_end = ptr;

			pad_len++;
		} else if (!pad_len && (l_ascii_isalnum(*ptr) || *ptr == '+' ||
					*ptr == '/'))
			/* Base64 character */
			base64_len++;
		else
			/* Bad character */
			return NULL;
	}

	if (ptr != in_end)
		return NULL;

	if ((base64_len & 3) == 1)
		/* Invalid length */
		return NULL;

	if (pad_len != align_len(base64_len, 4) - base64_len)
		return NULL;

	/* No padding */
	if (!base64_end)
		base64_end = ptr;

	*n_written = base64_len * 3 / 4;
	out_buf = l_malloc(*n_written);

	out = out_buf;
	base64_len = 0;

	for (ptr = in; ptr < base64_end; ptr++) {
		if (l_ascii_isspace(*ptr))
			/* Whitespace */
			continue;

		/* Base64 character */
		reg <<= 6;
		if (l_ascii_isupper(*ptr))
			reg |= *ptr - 'A' + 0;
		else if (l_ascii_islower(*ptr))
			reg |= *ptr - 'a' + 26;
		else if (l_ascii_isdigit(*ptr))
			reg |= *ptr - '0' + 52;
		else if (*ptr == '+')
			reg |= 62;
		else if (*ptr == '/')
			reg |= 63;

		if ((base64_len & 3) == 1)
			*out++ = reg >> 4;
		else if ((base64_len & 3) == 2)
			*out++ = reg >> 2;
		else if ((base64_len & 3) == 3)
			*out++ = reg >> 0;

		base64_len++;
	}

	return out_buf;
}

LIB_EXPORT char *l_base64_encode(const uint8_t *in, size_t in_len, int columns)
{
	const uint8_t *in_end = in + in_len;
	char *out_buf, *out;
	size_t out_len;
	uint32_t reg;
	uint8_t idx;
	int i, pad = 4;
	int col = 0;

	/* For simplicity allow multiples of 4 only */
	if (columns & 3)
		return NULL;

	out_len = (in_len + 2) / 3 * 4;

	if (columns && out_len)
		out_len += (out_len - 4) / columns;

	out_buf = l_malloc(out_len + 1);

	out = out_buf;

	while (in < in_end) {
		reg = *in++ << 16;

		if (in < in_end)
			reg |= *in++ << 8;
		else
			pad--;

		if (in < in_end)
			reg |= *in++ << 0;
		else
			pad--;

		if (columns && col == columns) {
			*out++ = '\n';
			col = 0;
		}
		col += 4;

		for (i = 0; i < pad; i++) {
			idx = (reg >> 18) & 63;
			reg <<= 6;

			if (idx < 26)
				*out++ = idx + 'A';
			else if (idx < 52)
				*out++ = idx - 26 + 'a';
			else if (idx < 62)
				*out++ = idx - 52 + '0';
			else
				*out++ = (idx == 62) ? '+' : '/';
		}
	}

	for (; pad < 4; pad++)
		*out++ = '=';

	*out = '\0';

	return out_buf;
}
