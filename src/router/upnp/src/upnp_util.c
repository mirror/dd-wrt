/*
 * Broadcom UPnP module utilities
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: upnp_util.c,v 1.8 2008/06/19 06:22:59 Exp $
 */
#include <upnp.h>
#include <errno.h>

static const char cb64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char cd64[] = "|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

/* 
 * Base64 block encoding,
 * encode 3 8-bit binary bytes as 4 '6-bit' characters
 */
void upnp_base64_encode_block(unsigned char in[3], unsigned char out[4], int len)
{
	out[0] = cb64[in[0] >> 2];
	out[1] = cb64[((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4)];
	out[2] = (unsigned char)(len > 1 ? cb64[((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6)] : '=');
	out[3] = (unsigned char)(len > 2 ? cb64[in[2] & 0x3f] : '=');
}

/*
 * Base64 encode a stream adding padding and line breaks as per spec.
 * input	- stream to encode
 * inputlen	- length of the input stream
 * target	- stream encoded with null ended.
 *
 * Returns The length of the encoded stream.
 */
int upnp_base64_encode(unsigned char *input, const int inputlen, unsigned char *target)
{
	unsigned char *out;
	unsigned char *in;

	out = target;
	in = input;

	if (input == NULL || inputlen == 0)
		return 0;

	while ((in + 3) <= (input + inputlen)) {
		upnp_base64_encode_block(in, out, 3);
		in += 3;
		out += 4;
	}

	if ((input + inputlen) - in == 1) {
		upnp_base64_encode_block(in, out, 1);
		out += 4;
	} else {
		if ((input + inputlen) - in == 2) {
			upnp_base64_encode_block(in, out, 2);
			out += 4;
		}
	}

	*out = 0;
	return (int)(out - target);
}

/*
 * Base64 block encoding,
 * Decode 4 '6-bit' characters into 3 8-bit binary bytes
 */
void upnp_decode_block(unsigned char in[4], unsigned char out[3])
{
	out[0] = (unsigned char)(in[0] << 2 | in[1] >> 4);
	out[1] = (unsigned char)(in[1] << 4 | in[2] >> 2);
	out[2] = (unsigned char)(((in[2] << 6) & 0xc0) | in[3]);
}

/*
 * Decode a base64 encoded stream discarding padding, line breaks and noise.
 * input 	- stream to decode
 * inputlen	- length of the input stream
 * target	- stream decoded with null ended.
 *
 * Returns The length of the decoded stream.
 */
int upnp_base64_decode(unsigned char *input, const int inputlen, unsigned char *target)
{
	unsigned char *inptr;
	unsigned char *out;
	unsigned char v;
	unsigned char in[4];
	int i, len;

	if (input == NULL || inputlen == 0)
		return 0;

	out = target;
	inptr = input;

	while (inptr <= (input + inputlen)) {
		for (len = 0, i = 0; i < 4 && inptr <= (input + inputlen); i++) {
			v = 0;
			while (inptr <= (input + inputlen) && v == 0) {
				v = (unsigned char)*inptr;
				inptr++;

				v = (unsigned char)((v < 43 || v > 122) ? 0 : cd64[v - 43]);
				if (v) {
					v = (unsigned char)((v == '$') ? 0 : v - 61);
				}
			}

			if (inptr <= (input + inputlen)) {
				len++;

				if (v) {
					in[i] = (unsigned char)(v - 1);
				}
			} else {
				in[i] = 0;
			}
		}

		if (len) {
			upnp_decode_block(in, out);
			out += len - 1;
		}
	}

	*out = 0;
	return (int)(out - target);
}

/* Get current GMT time */
int gmt_time(char *time_str)
{
	struct tm btime;
	time_t curr_time;

	static char *day_name[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

	static char *mon_name[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

	curr_time = time(0);
	gmtime_r(&curr_time, &btime);

	sprintf(time_str, "%.3s, %.2d %.3s %d %.2d:%.2d:%.2d GMT", day_name[btime.tm_wday], btime.tm_mday, mon_name[btime.tm_mon],
		1900 + btime.tm_year, btime.tm_hour, btime.tm_min, btime.tm_sec);

	return 0;
}

/* Translate value to string according to data type */
void translate_value(UPNP_CONTEXT *context, UPNP_VALUE *value)
{
	int len;
	char *buf = value->val.str;

	switch (value->type) {
	case UPNP_TYPE_STR:
		break;

	case UPNP_TYPE_BOOL:
		value->val.bool2 = (value->val.bool2 ? 1 : 0);
		sprintf(buf, "%d", value->val.bool2);
		break;

	case UPNP_TYPE_I1:
		sprintf(buf, "%d", value->val.i1);
		break;

	case UPNP_TYPE_I2:
		sprintf(buf, "%d", value->val.i2);
		break;

	case UPNP_TYPE_I4:
		sprintf(buf, "%ld", value->val.i4);
		break;

	case UPNP_TYPE_UI1:
		sprintf(buf, "%u", value->val.ui1);
		break;

	case UPNP_TYPE_UI2:
		sprintf(buf, "%u", value->val.ui2);
		break;

	case UPNP_TYPE_UI4:
		sprintf(buf, "%lu", value->val.ui4);
		break;

	case UPNP_TYPE_BIN_BASE64:
		len = upnp_base64_encode((unsigned char *)value->val.data, value->len, (unsigned char *)context->head_buffer);
		if (len > 0)
			strlcpy(buf, context->head_buffer, sizeof(value->val.str));
		else
			*buf = '\0';
		break;

	default:
		/* should not be reached */
		*buf = '\0';
		break;
	}

	return;
}

/* Convert value from string according to data type */
int convert_value(UPNP_CONTEXT *context, UPNP_VALUE *value)
{
	int len;
	int ival;
	unsigned int uval;

	switch (value->type) {
	case UPNP_TYPE_STR:
		value->len = strlen(value->val.str) + 1;
		break;

	case UPNP_TYPE_BOOL:
		/* 0, false, no for false; 1, true, yes for true */
		if (strcmp(value->val.str, "0") == 0 || strcmp(value->val.str, "false") == 0 || strcmp(value->val.str, "no") == 0) {
			value->val.bool2 = 0;
		} else if (strcmp(value->val.str, "1") == 0 || strcmp(value->val.str, "true") == 0 ||
			   strcmp(value->val.str, "yes") == 0) {
			value->val.bool2 = 1;
		} else {
			return -1;
		}

		value->len = 1;
		break;

	case UPNP_TYPE_I1:
		ival = atoi(value->val.str);
		if ((ival & 0xffffff00) != 0)
			return -1;

		value->val.i1 = ival;
		value->len = 1;
		break;

	case UPNP_TYPE_I2:
		ival = atoi(value->val.str);
		if ((ival & 0xffff0000) != 0)
			return -1;

		value->val.i2 = ival;
		value->len = 2;
		break;

	case UPNP_TYPE_I4:
		ival = atoi(value->val.str);
		value->val.i4 = ival;
		value->len = 4;
		break;

	case UPNP_TYPE_UI1:
		uval = strtoul(value->val.str, NULL, 10);
		if (uval > 0xff)
			return -1;

		value->val.ui1 = uval;
		value->len = 1;
		break;

	case UPNP_TYPE_UI2:
		uval = strtoul(value->val.str, NULL, 10);
		if (uval > 0xffff)
			return -1;

		value->val.ui2 = uval;
		value->len = 2;
		break;

	case UPNP_TYPE_UI4:
		uval = strtoul(value->val.str, NULL, 10);

		value->val.ui4 = uval;
		value->len = 4;
		break;

	case UPNP_TYPE_BIN_BASE64:
		len = upnp_base64_decode((unsigned char *)value->val.str, strlen(value->val.str),
					 (unsigned char *)context->head_buffer);
		if (len <= 0)
			return -1;

		memcpy(value->val.data, context->head_buffer, len);
		value->len = len;
		break;

	default:
		break;
	}

	return 0;
}

/* 
 * Search input argument list for a arguement
 * and return its value.
 */
IN_ARGUMENT *upnp_get_in_argument(IN_ARGUMENT *in_arguments, char *arg_name)
{
	while (in_arguments) {
		if (strcmp(in_arguments->name, arg_name) == 0)
			break;

		in_arguments = in_arguments->next;
	}

	return in_arguments;
}

/* 
 * Search output argument list for a arguement
 * and return its value.
 */
OUT_ARGUMENT *upnp_get_out_argument(OUT_ARGUMENT *out_arguments, char *arg_name)
{
	while (out_arguments) {
		if (strcmp(out_arguments->name, arg_name) == 0)
			break;

		out_arguments = out_arguments->next;
	}

	return out_arguments;
}

void upnp_host_addr(unsigned char *host_addr, struct in_addr ipaddr, unsigned short port)
{
	register unsigned long addr = ntohl(ipaddr.s_addr);

	sprintf((char *)host_addr, "%lu.%lu.%lu.%lu", (addr >> 24) & 0xff, (addr >> 16) & 0xff, (addr >> 8) & 0xff, addr & 0xff);

	if (port != 80) {
		char myport[sizeof(":65535")];

		sprintf(myport, ":%d", port);
		strcat((char *)host_addr, myport);
	}

	return;
}
