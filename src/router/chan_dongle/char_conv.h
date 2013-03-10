/*
   Copyright (C) 2010 bg <bg_one@mail.ru>
*/
#ifndef CHAN_DONGLE_CHAR_CONV_H_INCLUDED
#define CHAN_DONGLE_CHAR_CONV_H_INCLUDED

#include <sys/types.h>			/* ssize_t size_t */
#include "export.h"			/* EXPORT_DECL EXPORT_DEF */

/* encoding types of strings to/from device */
/* for simplefy first 3 values same as in PDU DCS bits 3..2 */
/* NOTE: order is magic see definition of recoders in char_conv.c */
typedef enum {
	STR_ENCODING_7BIT_HEX		= 0,	/* 7bit encoding */
	STR_ENCODING_8BIT_HEX,			/* 8bit encoding */
	STR_ENCODING_UCS2_HEX,			/* UCS-2 in hex like PDU */
/* TODO: check its really 7bit input from device */
	STR_ENCODING_7BIT,			/* 7bit ASCII  no need recode to utf-8 */
//	STR_ENCODING_8BIT,			/* 8bit */
//	STR_ENCODING_UCS2,			/* UCS2 */
	STR_ENCODING_UNKNOWN,			/* still unknown */
} str_encoding_t;

typedef enum {
	RECODE_DECODE	=	0,		/* from encoded to UTF-8 */
	RECODE_ENCODE				/* from UTF-8 to encoded */
} recode_direction_t;

/* recode in both directions */
EXPORT_DECL ssize_t str_recode(recode_direction_t dir, str_encoding_t encoding, const char* in, size_t in_length, char* out, size_t out_size);

EXPORT_DECL int parse_hexdigit(int hex);
EXPORT_DECL str_encoding_t get_encoding(recode_direction_t hint, const char * in, size_t in_length);

#endif /* CHAN_DONGLE_CHAR_CONV_H_INCLUDED */
