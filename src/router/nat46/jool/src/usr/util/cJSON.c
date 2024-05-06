/*
 * Copyright (c) 2009 Dave Gamble
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
 * cJSON
 * JSON parser in C.
 */

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>
#include "usr/util/cJSON.h"

static const char *ep;

const char *cJSON_GetErrorPtr(void)
{
	return ep;
}

static int cJSON_strcasecmp(const char *s1, const char *s2)
{
	if (!s1)
		return (s1 == s2) ? 0 : 1;
	if (!s2)
		return 1;
	for (; tolower(*s1) == tolower(*s2); ++s1, ++s2)
		if (*s1 == 0)
			return 0;
	return tolower(*(const unsigned char *)s1)
			- tolower(*(const unsigned char *)s2);
}

static void *(*cJSON_malloc)(size_t sz) = malloc;
static void (*cJSON_free)(void *ptr) = free;

/* Internal constructor. */
static cJSON *cJSON_New_Item(void)
{
	cJSON* node = (cJSON*)cJSON_malloc(sizeof(cJSON));
	if (node)
		memset(node, 0, sizeof(cJSON));
	return node;
}

/* Delete a cJSON structure. */
void cJSON_Delete(cJSON *c)
{
	cJSON *next;
	while (c) {
		next = c->next;
		if (!(c->type & cJSON_IsReference) && c->child)
			cJSON_Delete(c->child);
		if (!(c->type & cJSON_IsReference) && c->valuestring)
			cJSON_free(c->valuestring);
		if (!(c->type & cJSON_StringIsConst) && c->string)
			cJSON_free(c->string);
		cJSON_free(c);
		c = next;
	}
}

static void assign_cjson_number(cJSON *item, double num)
{
	item->type = cJSON_Number;

	item->valuedouble = num;
	item->numflags = VALUENUM_DBL;

	if (INT_MIN <= num && num <= INT_MAX) {
		item->valueint = (int)num;
		item->numflags |= VALUENUM_INT;
	}

	if (0 <= num && num <= UINT_MAX) {
		item->valueuint = (unsigned int)num;
		item->numflags |= VALUENUM_UINT;
	}
}

/*
 * Parse the input text to generate a number, and populate the result into item.
 */
static const char *parse_number(cJSON *item, const char *num)
{
	double n = 0, sign = 1, scale = 0;
	int subscale = 0, signsubscale = 1;

	/* Has sign? */
	if (*num == '-') {
		sign = -1;
		num++;
	}

	/* is zero */
	if (*num == '0')
		num++;

	/* Number? */
	if (*num >= '1' && *num <= '9') {
		do {
			n = (n * 10.0) + (*num++ - '0');
		} while (*num >= '0' && *num <= '9');
	}

	/* Fractional part? */
	if (*num == '.' && num[1] >= '0' && num[1] <= '9') {
		num++;
		do {
			n = (n * 10.0) + (*num++ - '0');
			scale--;
		} while (*num >= '0' && *num <= '9');
	}

	/* Exponent? */
	if (*num == 'e' || *num == 'E') {
		num++;
		if (*num == '+') {
			num++;
		} else if (*num == '-') { /* With sign? */
			signsubscale = -1;
			num++;
		}
		while (*num >= '0' && *num <= '9') { /* Number? */
			subscale = (subscale * 10) + (*num++ - '0');
		}
	}

	/* number = +/- number.fraction * 10^+/- exponent */
	n = sign * n * pow(10.0, (scale + subscale * signsubscale));

	assign_cjson_number(item, n);
	return num;
}

typedef struct {
	char *buffer;
	int length;
	int offset;
} printbuffer;

static unsigned parse_hex4(const char *str)
{
	unsigned h = 0;
	if (*str >= '0' && *str <= '9')
		h += (*str) - '0';
	else if (*str >= 'A' && *str <= 'F')
		h += 10 + (*str) - 'A';
	else if (*str >= 'a' && *str <= 'f')
		h += 10 + (*str) - 'a';
	else
		return 0;
	h = h << 4;
	str++;
	if (*str >= '0' && *str <= '9')
		h += (*str) - '0';
	else if (*str >= 'A' && *str <= 'F')
		h += 10 + (*str) - 'A';
	else if (*str >= 'a' && *str <= 'f')
		h += 10 + (*str) - 'a';
	else
		return 0;
	h = h << 4;
	str++;
	if (*str >= '0' && *str <= '9')
		h += (*str) - '0';
	else if (*str >= 'A' && *str <= 'F')
		h += 10 + (*str) - 'A';
	else if (*str >= 'a' && *str <= 'f')
		h += 10 + (*str) - 'a';
	else
		return 0;
	h = h << 4;
	str++;
	if (*str >= '0' && *str <= '9')
		h += (*str) - '0';
	else if (*str >= 'A' && *str <= 'F')
		h += 10 + (*str) - 'A';
	else if (*str >= 'a' && *str <= 'f')
		h += 10 + (*str) - 'a';
	else
		return 0;
	return h;
}

/* Parse the input text into an unescaped cstring, and populate item. */
static const unsigned char firstByteMark[7] = {
		0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC
};

static const char *parse_string(cJSON *item, const char *str)
{
	const char *ptr = str + 1;
	char *ptr2;
	char *out;
	int len = 0;
	unsigned uc, uc2;
	if (*str != '\"') {
		ep = str;
		return 0;
	} /* not a string! */

	while (*ptr != '\"' && *ptr && ++len)
		if (*ptr++ == '\\')
			ptr++; /* Skip escaped quotes. */

	/* This is how long we need for the string, roughly. */
	out = (char*)cJSON_malloc(len + 1);
	if (!out)
		return 0;

	ptr = str + 1;
	ptr2 = out;
	while (*ptr != '\"' && *ptr) {
		if (*ptr != '\\') {
			*ptr2++ = *ptr++;
			continue;
		}

		ptr++;
		switch (*ptr) {
		case 'b':
			*ptr2++ = '\b';
			break;
		case 'f':
			*ptr2++ = '\f';
			break;
		case 'n':
			*ptr2++ = '\n';
			break;
		case 'r':
			*ptr2++ = '\r';
			break;
		case 't':
			*ptr2++ = '\t';
			break;
		case 'u': /* transcode utf16 to utf8. */
			uc = parse_hex4(ptr + 1);
			ptr += 4; /* get the unicode char. */

			if ((uc >= 0xDC00 && uc <= 0xDFFF) || uc == 0)
				break; /* check for invalid.	*/

			/* UTF16 surrogate pairs. */
			if (uc >= 0xD800 && uc <= 0xDBFF) {
				if (ptr[1] != '\\' || ptr[2] != 'u') {
					/* missing second-half of surrogate. */
					break;
				}
				uc2 = parse_hex4(ptr + 3);
				ptr += 6;
				if (uc2 < 0xDC00 || uc2 > 0xDFFF) {
					/* invalid second-half of surrogate. */
					break;
				}
				uc = (((uc & 0x3FF) << 10) | (uc2 & 0x3FF))
						+ 0x10000;
			}

			len = 4;
			if (uc < 0x80)
				len = 1;
			else if (uc < 0x800)
				len = 2;
			else if (uc < 0x10000)
				len = 3;
			ptr2 += len;

			switch (len) {
			case 4:
				*--ptr2 = ((uc | 0x80) & 0xBF);
				uc >>= 6;
			case 3:
				*--ptr2 = ((uc | 0x80) & 0xBF);
				uc >>= 6;
			case 2:
				*--ptr2 = ((uc | 0x80) & 0xBF);
				uc >>= 6;
			case 1:
				*--ptr2 = (uc | firstByteMark[len]);
			}
			ptr2 += len;
			break;
		default:
			*ptr2++ = *ptr;
			break;
		}
		ptr++;
	}
	*ptr2 = 0;
	if (*ptr == '\"')
		ptr++;
	item->valuestring = out;
	item->type = cJSON_String;
	return ptr;
}

/* Predeclare these prototypes. */
static const char *parse_value(cJSON *item, const char *value);
static const char *parse_array(cJSON *item, const char *value);
static const char *parse_object(cJSON *item, const char *value);

/* Utility to jump whitespace and cr/lf */
static const char *skip(const char *in)
{
	while (in && *in && (unsigned char)*in <= 32)
		in++;
	return in;
}

/* Parse an object - create a new root, and populate. */
cJSON *cJSON_ParseWithOpts(const char *value, const char **return_parse_end,
		int require_null_terminated)
{
	const char *end = 0;
	cJSON *c = cJSON_New_Item();
	ep = 0;
	if (!c)
		return 0; /* memory fail */

	end = parse_value(c, skip(value));
	if (!end) {
		cJSON_Delete(c);
		return 0;
	} /* parse failure. ep is set. */

	/*
	 * if we require null-terminated JSON without appended garbage,
	 * skip and then check for a null terminator
	 */
	if (require_null_terminated) {
		end = skip(end);
		if (*end) {
			cJSON_Delete(c);
			ep = end;
			return 0;
		}
	}
	if (return_parse_end)
		*return_parse_end = end;
	return c;
}
/* Default options for cJSON_Parse */
cJSON *cJSON_Parse(const char *value)
{
	return cJSON_ParseWithOpts(value, 0, 0);
}

/* Parser core - when encountering text, process appropriately. */
static const char *parse_value(cJSON *item, const char *value)
{
	if (!value)
		return 0; /* Fail on null. */
	if (!strncmp(value, "null", 4)) {
		item->type = cJSON_NULL;
		return value + 4;
	}
	if (!strncmp(value, "false", 5)) {
		item->type = cJSON_False;
		return value + 5;
	}
	if (!strncmp(value, "true", 4)) {
		item->type = cJSON_True;
		return value + 4;
	}
	if (*value == '\"')
		return parse_string(item, value);
	if (*value == '-' || (*value >= '0' && *value <= '9'))
		return parse_number(item, value);
	if (*value == '[')
		return parse_array(item, value);
	if (*value == '{')
		return parse_object(item, value);

	ep = value;
	return 0; /* failure. */
}

/* Build an array from input text. */
static const char *parse_array(cJSON *item, const char *value)
{
	cJSON *child;
	if (*value != '[') {
		ep = value;
		return 0;
	} /* not an array! */

	item->type = cJSON_Array;
	value = skip(value + 1);
	if (*value == ']')
		return value + 1; /* empty array. */

	item->child = child = cJSON_New_Item();
	if (!item->child)
		return 0; /* memory fail */
	/* skip any spacing, get the value. */
	value = skip(parse_value(child, skip(value)));
	if (!value)
		return 0;

	while (*value == ',') {
		cJSON *new_item;
		if (!(new_item = cJSON_New_Item()))
			return 0; /* memory fail */
		child->next = new_item;
		new_item->prev = child;
		child = new_item;
		value = skip(parse_value(child, skip(value + 1)));
		if (!value)
			return 0; /* memory fail */
	}

	if (*value == ']')
		return value + 1; /* end of array */
	ep = value;
	return 0; /* malformed. */
}

/* Build an object from the text. */
static const char *parse_object(cJSON *item, const char *value)
{
	cJSON *child;
	if (*value != '{') {
		ep = value;
		return 0;
	} /* not an object! */

	item->type = cJSON_Object;
	value = skip(value + 1);
	if (*value == '}')
		return value + 1; /* empty array. */

	item->child = child = cJSON_New_Item();
	if (!item->child)
		return 0;
	value = skip(parse_string(child, skip(value)));
	if (!value)
		return 0;
	child->string = child->valuestring;
	child->valuestring = 0;
	if (*value != ':') {
		ep = value;
		return 0;
	} /* fail! */
	value = skip(parse_value(child, skip(value + 1))); /* skip any spacing, get the value. */
	if (!value)
		return 0;

	while (*value == ',') {
		cJSON *new_item;
		if (!(new_item = cJSON_New_Item()))
			return 0; /* memory fail */
		child->next = new_item;
		new_item->prev = child;
		child = new_item;
		value = skip(parse_string(child, skip(value + 1)));
		if (!value)
			return 0;
		child->string = child->valuestring;
		child->valuestring = 0;
		if (*value != ':') {
			ep = value;
			return 0;
		} /* fail! */
		value = skip(parse_value(child, skip(value + 1))); /* skip any spacing, get the value. */
		if (!value)
			return 0;
	}

	if (*value == '}')
		return value + 1; /* end of array */
	ep = value;
	return 0; /* malformed. */
}

/* Get Array size/item / object item. */
int cJSON_GetArraySize(cJSON *array)
{
	cJSON *c = array->child;
	int i = 0;
	while (c) {
		i++;
		c = c->next;
	}
	return i;
}
cJSON *cJSON_GetArrayItem(cJSON *array, int item)
{
	cJSON *c = array->child;
	while (c && item > 0) {
		item--;
		c = c->next;
	}
	return c;
}
cJSON *cJSON_GetObjectItem(cJSON *object, const char *string)
{
	cJSON *c = object->child;
	while (c && cJSON_strcasecmp(c->string, string))
		c = c->next;
	return c;
}
