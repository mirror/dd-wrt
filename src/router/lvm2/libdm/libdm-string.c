/*
 * Copyright (C) 2006-2015 Red Hat, Inc. All rights reserved.
 *
 * This file is part of the device-mapper userspace tools.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "libdm/misc/dmlib.h"

#include <ctype.h>
#include <stdarg.h>
#include <math.h>  /* fabs() */
#include <float.h> /* DBL_EPSILON */

/*
 * consume characters while they match the predicate function.
 */
static char *_consume(char *buffer, int (*fn) (int))
{
	while (*buffer && fn(*buffer))
		buffer++;

	return buffer;
}

static int _isword(int c)
{
	return !isspace(c);
}

/*
 * Split buffer into NULL-separated words in argv.
 * Returns number of words.
 */
int dm_split_words(char *buffer, unsigned max,
		   unsigned ignore_comments __attribute__((unused)),
		   char **argv)
{
	unsigned arg;

	for (arg = 0; arg < max; arg++) {
		buffer = _consume(buffer, isspace);
		if (!*buffer)
			break;

		argv[arg] = buffer;
		buffer = _consume(buffer, _isword);

		if (*buffer) {
			*buffer = '\0';
			buffer++;
		}
	}

	return arg;
}

/*
 * Remove hyphen quoting from a component of a name.
 * NULL-terminates the component and returns start of next component.
 */
static char *_unquote(char *component)
{
	char *c = component;
	char *o = c;
	char *r;

	while (*c) {
		if (*(c + 1)) {
			if (*c == '-') {
				if (*(c + 1) == '-')
					c++;
				else
					break;
			}
		}
		*o = *c;
		o++;
		c++;
	}

	r = (*c) ? c + 1 : c;
	*o = '\0';

	return r;
}

int dm_split_lvm_name(struct dm_pool *mem, const char *dmname,
		      char **vgname, char **lvname, char **layer)
{
	if (!vgname || !lvname || !layer) {
		log_error(INTERNAL_ERROR "dm_split_lvm_name: Forbidden NULL parameter detected.");
		return 0;
	}

	if (mem && (!dmname || !(*vgname = dm_pool_strdup(mem, dmname)))) {
		log_error("Failed to duplicate lvm name.");
		return 0;
	} else if (!*vgname) {
		log_error("Missing lvm name for split.");
		return 0;
	}

	_unquote(*layer = _unquote(*lvname = _unquote(*vgname)));

	return 1;
}

/*
 * On error, up to glibc 2.0.6, snprintf returned -1 if buffer was too small;
 * From glibc 2.1 it returns number of chars (excl. trailing null) that would 
 * have been written had there been room.
 *
 * dm_snprintf reverts to the old behaviour.
 */
int dm_snprintf(char *buf, size_t bufsize, const char *format, ...)
{
	int n;
	va_list ap;

	va_start(ap, format);
	n = vsnprintf(buf, bufsize, format, ap);
	va_end(ap);

	if (n < 0 || ((unsigned) n >= bufsize))
		return -1;

	return n;
}

const char *dm_basename(const char *path)
{
	const char *p = strrchr(path, '/');

	return p ? p + 1 : path;
}

int dm_vasprintf(char **result, const char *format, va_list aq)
{
	int i, n, size = 16;
	va_list ap;
	char *buf = dm_malloc(size);

	*result = 0;

	if (!buf)
		return -1;

	for (i = 0;; i++) {
		va_copy(ap, aq);
		n = vsnprintf(buf, size, format, ap);
		va_end(ap);

		if (0 <= n && n < size)
			break;

		dm_free(buf);
		/* Up to glibc 2.0.6 returns -1 */
		size = (n < 0) ? size * 2 : n + 1;
		if (!(buf = dm_malloc(size)))
			return -1;
	}

	if (i > 1) {
		/* Reallocating more then once? */
		if (!(*result = dm_strdup(buf))) {
			dm_free(buf);
			return -1;
		}
		dm_free(buf);
	} else
		*result = buf;

	return n + 1;
}

int dm_asprintf(char **result, const char *format, ...)
{
	int r;
	va_list ap;
	va_start(ap, format);
	r = dm_vasprintf(result, format, ap);
	va_end(ap);
	return r;
}

/*
 * Count occurences of 'c' in 'str' until we reach a null char.
 *
 * Returns:
 *  len - incremented for each char we encounter.
 *  count - number of occurrences of 'c' and 'c2'.
 */
static void _count_chars(const char *str, size_t *len, int *count,
			 const int c1, const int c2)
{
	const char *ptr;

	for (ptr = str; *ptr; ptr++, (*len)++)
		if (*ptr == c1 || *ptr == c2)
			(*count)++;
}

/*
 * Count occurrences of 'c' in 'str' of length 'size'.
 *
 * Returns:
 *   Number of occurrences of 'c'
 */
unsigned dm_count_chars(const char *str, size_t len, const int c)
{
	size_t i;
	unsigned count = 0;

	for (i = 0; i < len; i++)
		if (str[i] == c)
			count++;

	return count;
}

/*
 * Length of string after escaping double quotes and backslashes.
 */
size_t dm_escaped_len(const char *str)
{
	size_t len = 1;
	int count = 0;

	_count_chars(str, &len, &count, '\"', '\\');

	return count + len;
}

/*
 * Copies a string, quoting orig_char with quote_char.
 * Optionally also quote quote_char.
 */
static void _quote_characters(char **out, const char *src,
			      const int orig_char, const int quote_char,
			      int quote_quote_char)
{
	while (*src) {
		if (*src == orig_char ||
		    (*src == quote_char && quote_quote_char))
			*(*out)++ = quote_char;

		*(*out)++ = *src++;
	}
}

static void _unquote_one_character(char *src, const char orig_char,
				   const char quote_char)
{
	char *out;
	char s, n;

	/* Optimise for the common case where no changes are needed. */
	while ((s = *src++)) {
		if (s == quote_char &&
		    ((n = *src) == orig_char || n == quote_char)) {
			out = src++;
			*(out - 1) = n;

			while ((s = *src++)) {
				if (s == quote_char &&
				    ((n = *src) == orig_char || n == quote_char)) {
					s = n;
					src++;
				}
				*out = s;
				out++;
			}

			*out = '\0';
			return;
		}
	}
}

/*
 * Unquote each character given in orig_char array and unquote quote_char
 * as well. Also save the first occurrence of each character from orig_char
 * that was found unquoted in arr_substr_first_unquoted array. This way we can
 * process several characters in one go.
 */
static void _unquote_characters(char *src, const char *orig_chars,
				size_t num_orig_chars,
				const char quote_char,
				char *arr_substr_first_unquoted[])
{
	char *out = src;
	char c, s, n;
	unsigned i;

	while ((s = *src++)) {
		for (i = 0; i < num_orig_chars; i++) {
			c = orig_chars[i];
			if (s == quote_char &&
			    ((n = *src) == c || n == quote_char)) {
				s = n;
				src++;
				break;
			}
			if (arr_substr_first_unquoted && (s == c) &&
			    !arr_substr_first_unquoted[i])
				arr_substr_first_unquoted[i] = out;
		};
		*out++ = s;
	}

	*out = '\0';
}

/*
 * Copies a string, quoting hyphens with hyphens.
 */
static void _quote_hyphens(char **out, const char *src)
{
	_quote_characters(out, src, '-', '-', 0);
}

/*
 * <vg>-<lv>-<layer> or if !layer just <vg>-<lv>.
 */
char *dm_build_dm_name(struct dm_pool *mem, const char *vgname,
		       const char *lvname, const char *layer)
{
	size_t len = 1;
	int hyphens = 1;
	char *r, *out;

	_count_chars(vgname, &len, &hyphens, '-', 0);
	_count_chars(lvname, &len, &hyphens, '-', 0);

	if (layer && *layer) {
		_count_chars(layer, &len, &hyphens, '-', 0);
		hyphens++;
	}

	len += hyphens;

	if (!(r = dm_pool_alloc(mem, len))) {
		log_error("build_dm_name: Allocation failed for %" PRIsize_t
			  " for %s %s %s.", len, vgname, lvname, layer);
		return NULL;
	}

	out = r;
	_quote_hyphens(&out, vgname);
	*out++ = '-';
	_quote_hyphens(&out, lvname);

	if (layer && *layer) {
		/* No hyphen if the layer begins with _ e.g. _mlog */
		if (*layer != '_')
			*out++ = '-';
		_quote_hyphens(&out, layer);
	}
	*out = '\0';

	return r;
}

char *dm_build_dm_uuid(struct dm_pool *mem, const char *uuid_prefix, const char *lvid, const char *layer)
{
	char *dmuuid;
	size_t len;

	if (!layer)
		layer = "";

	len = strlen(uuid_prefix) + strlen(lvid) + strlen(layer) + 2;

	if (!(dmuuid = dm_pool_alloc(mem, len))) {
		log_error("build_dm_name: Allocation failed for %" PRIsize_t
			  " %s %s.", len, lvid, layer);
		return NULL;
	}

	sprintf(dmuuid, "%s%s%s%s", uuid_prefix, lvid, (*layer) ? "-" : "", layer);

	return dmuuid;
}

/*
 * Copies a string, quoting double quotes with backslashes.
 */
char *dm_escape_double_quotes(char *out, const char *src)
{
	char *buf = out;

	_quote_characters(&buf, src, '\"', '\\', 1);
	*buf = '\0';

	return out;
}

/*
 * Undo quoting in situ.
 */
void dm_unescape_double_quotes(char *src)
{
	_unquote_one_character(src, '\"', '\\');
}

/*
 * Unescape colons and "at" signs in situ and save the substrings
 * starting at the position of the first unescaped colon and the
 * first unescaped "at" sign. This is normally used to unescape
 * device names used as PVs.
 */
void dm_unescape_colons_and_at_signs(char *src,
				     char **substr_first_unquoted_colon,
				     char **substr_first_unquoted_at_sign)
{
	const char *orig_chars = ":@";
	char *arr_substr_first_unquoted[] = {NULL, NULL, NULL};

	_unquote_characters(src, orig_chars, 2, '\\', arr_substr_first_unquoted);

	if (substr_first_unquoted_colon)
		*substr_first_unquoted_colon = arr_substr_first_unquoted[0];

	if (substr_first_unquoted_at_sign)
		*substr_first_unquoted_at_sign = arr_substr_first_unquoted[1];
}

int dm_strncpy(char *dest, const char *src, size_t n)
{
	if (memccpy(dest, src, 0, n))
		return 1;

	if (n > 0)
		dest[n - 1] = '\0';

	return 0;
}

/* Test if the doubles are close enough to be considered equal */
static int _close_enough(double d1, double d2)
{
	return fabs(d1 - d2) < DBL_EPSILON;
}

#define BASE_UNKNOWN 0
#define BASE_SHARED 1
#define BASE_1024 8
#define BASE_1000 15
#define BASE_SPECIAL 21
#define NUM_UNIT_PREFIXES 6
#define NUM_SPECIAL 3

#define SIZE_BUF 128

const char *dm_size_to_string(struct dm_pool *mem, uint64_t size,
			      char unit_type, int use_si_units, 
			      uint64_t unit_factor, int include_suffix, 
			      dm_size_suffix_t suffix_type)
{
	unsigned base = BASE_UNKNOWN;
	unsigned s;
	int precision;
	double d;
	uint64_t byte = UINT64_C(0);
	uint64_t units = UINT64_C(1024);
	char *size_buf = NULL;
	char new_unit_type = '\0', unit_type_buf[2];
	const char *prefix = "";
	const char * const size_str[][3] = {
		/* BASE_UNKNOWN */
		{"         ", "   ", " "},	/* [0] */

		/* BASE_SHARED - Used if use_si_units = 0 */
		{" Exabyte", " EB", "E"},	/* [1] */
		{" Petabyte", " PB", "P"},	/* [2] */
		{" Terabyte", " TB", "T"},	/* [3] */
		{" Gigabyte", " GB", "G"},	/* [4] */
		{" Megabyte", " MB", "M"},	/* [5] */
		{" Kilobyte", " KB", "K"},	/* [6] */
		{" Byte    ", " B", "B"},	/* [7] */

		/* BASE_1024 - Used if use_si_units = 1 */
		{" Exbibyte", " EiB", "e"},	/* [8] */
		{" Pebibyte", " PiB", "p"},	/* [9] */
		{" Tebibyte", " TiB", "t"},	/* [10] */
		{" Gibibyte", " GiB", "g"},	/* [11] */
		{" Mebibyte", " MiB", "m"},	/* [12] */
		{" Kibibyte", " KiB", "k"},	/* [13] */
		{" Byte    ", " B", "b"},	/* [14] */

		/* BASE_1000 - Used if use_si_units = 1 */
		{" Exabyte",  " EB", "E"},	/* [15] */
		{" Petabyte", " PB", "P"},	/* [16] */
		{" Terabyte", " TB", "T"},	/* [17] */
		{" Gigabyte", " GB", "G"},	/* [18] */
		{" Megabyte", " MB", "M"},	/* [19] */
		{" Kilobyte", " kB", "K"},	/* [20] */

		/* BASE_SPECIAL */
		{" Byte    ", " B ", "B"},	/* [21] (shared with BASE_1000) */
		{" Units   ", " Un", "U"},	/* [22] */
		{" Sectors ", " Se", "S"},	/* [23] */
	};

	if (!(size_buf = dm_pool_alloc(mem, SIZE_BUF))) {
		log_error("no memory for size display buffer");
		return "";
	}

	if (!use_si_units) {
		/* Case-independent match */
		for (s = 0; s < NUM_UNIT_PREFIXES; s++)
			if (toupper((int) unit_type) ==
			    *size_str[BASE_SHARED + s][2]) {
				base = BASE_SHARED;
				break;
			}
	} else {
		/* Case-dependent match for powers of 1000 */
		for (s = 0; s < NUM_UNIT_PREFIXES; s++)
			if (unit_type == *size_str[BASE_1000 + s][2]) {
				base = BASE_1000;
				break;
			}

		/* Case-dependent match for powers of 1024 */
		if (base == BASE_UNKNOWN)
			for (s = 0; s < NUM_UNIT_PREFIXES; s++)
			if (unit_type == *size_str[BASE_1024 + s][2]) {
				base = BASE_1024;
				break;
			}
	}

	if (base == BASE_UNKNOWN)
		/* Check for special units - s, b or u */
		for (s = 0; s < NUM_SPECIAL; s++)
			if (toupper((int) unit_type) ==
			    *size_str[BASE_SPECIAL + s][2]) {
				base = BASE_SPECIAL;
				break;
			}

	if (size == UINT64_C(0)) {
		if (base == BASE_UNKNOWN)
			s = 0;
		sprintf(size_buf, "0%s", include_suffix ? size_str[base + s][suffix_type] : "");
		return size_buf;
	}

	size *= UINT64_C(512);

	if (base != BASE_UNKNOWN) {
		if (!unit_factor) {
			unit_type_buf[0] = unit_type;
			unit_type_buf[1] = '\0';
			if (!(unit_factor = dm_units_to_factor(&unit_type_buf[0], &new_unit_type, 1, NULL)) ||
			    unit_type != new_unit_type) {
				/* The two functions should match (and unrecognised units get treated like 'h'). */
				log_error(INTERNAL_ERROR "Inconsistent units: %c and %c.", unit_type, new_unit_type);
				return "";
			}
		}
		byte = unit_factor;
	} else {
		/* Human-readable style */
		if (unit_type == 'H' || unit_type == 'R') {
			units = UINT64_C(1000);
			base = BASE_1000;
		} else {
			units = UINT64_C(1024);
			base = BASE_1024;
		}

		if (!use_si_units)
			base = BASE_SHARED;

		byte = units * units * units * units * units * units;

		for (s = 0; s < NUM_UNIT_PREFIXES && size < byte; s++)
			byte /= units;

		if ((s < NUM_UNIT_PREFIXES) &&
		    ((unit_type == 'R') || (unit_type == 'r'))) {
			/* When the rounding would cause difference, add '<' prefix
			 * i.e.  2043M is more then 1.9949G prints <2.00G
			 * This version is for 2 digits fixed precision */
			d = 100. * (double) size / byte;
			if (!_close_enough(floorl(d), nearbyintl(d)))
				prefix = "<";
		}

		include_suffix = 1;
	}

	/* FIXME Make precision configurable */
	switch (toupper(*size_str[base + s][DM_SIZE_UNIT])) {
	case 'B':
	case 'S':
		precision = 0;
		break;
	default:
		precision = 2;
	}

	snprintf(size_buf, SIZE_BUF, "%s%.*f%s", prefix, precision,
		 (double) size / byte, include_suffix ? size_str[base + s][suffix_type] : "");

	return size_buf;
}

uint64_t dm_units_to_factor(const char *units, char *unit_type,
			    int strict, const char **endptr)
{
	char *ptr = NULL;
	uint64_t v;
	double custom_value = 0;
	uint64_t multiplier;

	if (endptr)
		*endptr = units;

	if (isdigit(*units)) {
		custom_value = strtod(units, &ptr);
		if (ptr == units)
			return 0;
		v = (uint64_t) strtoull(units, NULL, 10);
		if (_close_enough((double) v, custom_value))
			custom_value = 0;	/* Use integer arithmetic */
		units = ptr;
	} else
		v = 1;

	/* Only one units char permitted in strict mode. */
	if (strict && units[0] && units[1])
		return 0;

	if (v == 1)
		*unit_type = *units;
	else
		*unit_type = 'U';

	switch (*units) {
	case 'h':
	case 'H':
	case 'r':
	case 'R':
		multiplier = v = UINT64_C(1);
		*unit_type = *units;
		break;
	case 'b':
	case 'B':
		multiplier = UINT64_C(1);
		break;
#define KILO UINT64_C(1024)
	case 's':
	case 'S':
		multiplier = (KILO/2);
		break;
	case 'k':
		multiplier = KILO;
		break;
	case 'm':
		multiplier = KILO * KILO;
		break;
	case 'g':
		multiplier = KILO * KILO * KILO;
		break;
	case 't':
		multiplier = KILO * KILO * KILO * KILO;
		break;
	case 'p':
		multiplier = KILO * KILO * KILO * KILO * KILO;
		break;
	case 'e':
		multiplier = KILO * KILO * KILO * KILO * KILO * KILO;
		break;
#undef KILO
#define KILO UINT64_C(1000)
	case 'K':
		multiplier = KILO;
		break;
	case 'M':
		multiplier = KILO * KILO;
		break;
	case 'G':
		multiplier = KILO * KILO * KILO;
		break;
	case 'T':
		multiplier = KILO * KILO * KILO * KILO;
		break;
	case 'P':
		multiplier = KILO * KILO * KILO * KILO * KILO;
		break;
	case 'E':
		multiplier = KILO * KILO * KILO * KILO * KILO * KILO;
		break;
#undef KILO
	default:
		return 0;
	}

	if (endptr)
		*endptr = units + 1;

	if (_close_enough(custom_value, 0.))
		return v * multiplier; /* Use integer arithmetic */
	else
		return (uint64_t) (custom_value * multiplier);
}
