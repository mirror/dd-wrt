/*
    Copyright (C) 2009 - 2010
    Artem Makhutov <artem@makhutov.org>
    http://www.makhutov.org

    Dmitry Vagin <dmitry2004@yandex.ru>

    Copyright (C) 2010 - 2011
    bg <bg_one@mail.ru>
*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>

#include <iconv.h>			/* iconv_t iconv() */
#include <string.h>			/* memcpy() */
#include <stdio.h>			/* sscanf() snprintf() */
#include <errno.h>			/* EINVAL */

#include "char_conv.h"
#include "mutils.h"			/* ITEMS_OF() */

static ssize_t convert_string (const char* in, size_t in_length, char* out, size_t out_size, char* from, char* to)
{
	ICONV_CONST char*	in_ptr = (ICONV_CONST char*) in;
	size_t			in_bytesleft = in_length;
	char*			out_ptr = out;
	size_t			out_bytesleft = out_size - 1;
	ICONV_T			cd = (ICONV_T) -1;
	ssize_t			res;

	cd = iconv_open (to, from);
	if (cd == (ICONV_T) -1)
	{
		return -2;
	}

	res = iconv (cd, &in_ptr, &in_bytesleft, &out_ptr, &out_bytesleft);
	if (res < 0)
	{
		iconv_close (cd);
		return -3;
	}

	iconv_close (cd);

	*out_ptr = '\0';

	return (out_ptr - out);
}

#/* convert 1 hex digits of PDU to byte, return < 0 on error */
EXPORT_DEF int parse_hexdigit(int hex)
{
	if(hex >= '0' && hex <= '9')
		return hex - '0';
	if(hex >= 'a' && hex <= 'f')
		return hex - 'a' + 10;
	if(hex >= 'A' && hex <= 'F')
		return hex - 'A' + 10;
	return -1;
}

static ssize_t hexstr_to_8bitchars (const char* in, size_t in_length, char* out, size_t out_size)
{
	int d1, d2;

	/* odd number of chars check */
	if (in_length & 0x1)
		return -EINVAL;

	in_length = in_length >> 1;

	if (out_size - 1 < in_length)
	{
		return -ENOMEM;
	}
	out_size = in_length;
	
	for (; in_length; --in_length)
	{
		d1 = parse_hexdigit(*in++);
		if(d1 < 0)
			return -EINVAL;
		d2 = parse_hexdigit(*in++);
		if(d2 < 0)
			return -EINVAL;
		*out++ = (d1 << 4) | d2;
	}

	*out = 0;

	return out_size;
}

static ssize_t chars8bit_to_hexstr (const char* in, size_t in_length, char* out, size_t out_size)
{
	static const char hex_table[] = "0123456789ABCDEF";
	const unsigned char *in2 = (const unsigned char *)in;	/* for save time of first & 0x0F */

	if (out_size - 1 < in_length * 2)
	{
		return -1;
	}
	out_size = in_length * 2;
	
	for (; in_length; --in_length, ++in2)
	{
		*out++ = hex_table[*in2 >> 4];
		*out++ = hex_table[*in2 & 0xF];
	}

	*out = 0;

	return out_size;
}

static ssize_t hexstr_ucs2_to_utf8 (const char* in, size_t in_length, char* out, size_t out_size)
{
	char	buf[out_size];
	ssize_t	res;

	if (out_size - 1 < in_length / 2)
	{
		return -1;
	}

	res = hexstr_to_8bitchars (in, in_length, buf, out_size);
	if (res < 0)
	{
		return res;
	}

	res = convert_string (buf, res, out, out_size, "UCS-2BE", "UTF-8");

	return res;
}

static ssize_t utf8_to_hexstr_ucs2 (const char* in, size_t in_length, char* out, size_t out_size)
{
	char	buf[out_size];
	ssize_t	res;

	if (out_size - 1 < in_length * 4)
	{
		return -1;
	}

	res = convert_string (in, in_length, buf, out_size, "UTF-8", "UCS-2BE");
	if (res < 0)
	{
		return res;
	}

	res = chars8bit_to_hexstr (buf, res, out, out_size);

	return res;
}

static ssize_t char_to_hexstr_7bit (const char* in, size_t in_length, char* out, size_t out_size)
{
	size_t		i;
	size_t		x = 0;
	size_t		s;
	unsigned char	c;
	unsigned char	b;
	char	buf[] = { 0x0, 0x0, 0x0 };

	x = (in_length - in_length / 8) * 2;
	if (out_size - 1 < x)
	{
		return -1;
	}

	if(in_length > 0)
	{
		in_length--;
		for (i = 0, x = 0, s = 0; i < in_length; i++)
		{
			if (s == 7)
			{
				s = 0;
				continue;
			}

			c = in[i] >> s;
			b = in[i + 1] << (7 - s);
			c = c | b;
			s++;

			snprintf (buf, sizeof(buf), "%.2X", c);

			memcpy (out + x, buf, 2);
			x = x + 2;
		}

		c = in[i] >> s;
		snprintf (buf, sizeof(buf), "%.2X", c);
		memcpy (out + x, buf, 2);
		x = x + 2;
	}
	out[x] = '\0';

	return x;
}

static ssize_t hexstr_7bit_to_char (const char* in, size_t in_length, char* out, size_t out_size)
{
	size_t		i;
	size_t		x;
	size_t		s;
	int		hexval;
	unsigned char	c;
	unsigned char	b;
	char	buf[] = { 0x0, 0x0, 0x0 };

	in_length = in_length / 2;
	x = in_length + in_length / 7;
	if (out_size - 1 < x)
	{
		return -1;
	}

	for (i = 0, x = 0, s = 1, b = 0; i < in_length; i++)
	{
		memcpy (buf, in + i * 2, 2);
		if (sscanf (buf, "%x", &hexval) != 1)
		{
			return -1;
		}

		c = ((unsigned char) hexval) << s;
		c = (c >> 1) | b;
		b = ((unsigned char) hexval) >> (8 - s);

		out[x] = c;
		x++; s++;

		if (s == 8)
		{
			out[x] = b;
			s = 1; b = 0;
			x++;
		}
	}

	out[x] = '\0';

	return x;
}

#/* */
ssize_t just_copy (const char* in, size_t in_length, char* out, size_t out_size)
{
	// FIXME: or copy out_size-1 bytes only ?
	if (in_length <= out_size - 1)
	{
		memcpy(out, in, in_length);
		out[in_length] = 0;
		return in_length;
	}
	return -ENOMEM;
}

typedef ssize_t (*coder) (const char* in, size_t in_length, char* out, size_t out_size);

/* array in order of values RECODE_*  */
static const coder recoders[][2] =
{
/* in order of values STR_ENCODING_*  */
	{ hexstr_7bit_to_char, char_to_hexstr_7bit },		/* STR_ENCODING_7BIT_HEX */
	{ hexstr_to_8bitchars, chars8bit_to_hexstr },		/* STR_ENCODING_8BIT_HEX */
	{ hexstr_ucs2_to_utf8, utf8_to_hexstr_ucs2 },		/* STR_ENCODING_UCS2_HEX */
	{ just_copy, just_copy },				/* STR_ENCODING_7BIT */
};

#/* */
EXPORT_DEF ssize_t str_recode(recode_direction_t dir, str_encoding_t encoding, const char* in, size_t in_length, char* out, size_t out_size)
{
	unsigned idx = encoding;
	if((dir == RECODE_DECODE || dir == RECODE_ENCODE) && idx < ITEMS_OF(recoders))
		return (recoders[idx][dir])(in, in_length, out, out_size);
	return -EINVAL;
}

#/* */
EXPORT_DEF str_encoding_t get_encoding(recode_direction_t hint, const char* in, size_t length)
{
	if(hint == RECODE_ENCODE)
	{
		for(; length; --length, ++in)
			if(*in & 0x80)
				return STR_ENCODING_UCS2_HEX;
		return STR_ENCODING_7BIT_HEX;
	}
	else
	{
		size_t x;
		for(x = 0; x < length; ++x)
		{
			if(parse_hexdigit(in[x]) < 0) {
				return STR_ENCODING_7BIT;
			}
		}
		// TODO: STR_ENCODING_7BIT_HEX or STR_ENCODING_8BIT_HEX or STR_ENCODING_UCS2_HEX
	}

	return STR_ENCODING_UNKNOWN;
}
