/*
 * utils.c             General purpose utilities
 *
 * Copyright (c) 2001-2005 Thomas Graf <tgraf@suug.ch>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <bmon/bmon.h>
#include <bmon/conf.h>
#include <bmon/utils.h>

void * xcalloc(size_t n, size_t s)
{
	void *d = calloc(n, s);

	if (NULL == d) {
		fprintf(stderr, "xalloc: Out of memory\n");
		exit(ENOMEM);
	}

	return d;
}

void * xrealloc(void *p, size_t s)
{
	void *d = realloc(p, s);

	if (NULL == d) {
		fprintf(stderr, "xrealloc: Out of memory!\n");
		exit(ENOMEM);
	}

	return d;
}

void xfree(void *d)
{
	if (d)
		free(d);
}

inline float ts_to_float(timestamp_t *src)
{
	return (float) src->tv_sec + ((float) src->tv_usec / 1000000.0f);
}

inline void float_to_ts(timestamp_t *dst, float src)
{
	dst->tv_sec = (time_t) src;
	dst->tv_usec = (src - ((float) ((time_t) src))) * 1000000.0f;
}

inline void ts_add(timestamp_t *dst, timestamp_t *src1, timestamp_t *src2)
{
	dst->tv_sec = src1->tv_sec + src2->tv_sec;
	dst->tv_usec = src1->tv_usec + src2->tv_usec;

	if (dst->tv_usec >= 1000000) {
		dst->tv_sec++;
		dst->tv_usec -= 1000000;
	}
}

inline void ts_sub(timestamp_t *dst, timestamp_t *src1, timestamp_t *src2)
{
	dst->tv_sec = src1->tv_sec - src2->tv_sec;
	dst->tv_usec = src1->tv_usec - src2->tv_usec;
	if (dst->tv_usec <= -1000000) {
		dst->tv_sec--;
		dst->tv_usec += 1000000;
	}
}

inline int ts_le(timestamp_t *a, timestamp_t *b)
{
	if (a->tv_sec > b->tv_sec)
		return 0;

	if (a->tv_sec < b->tv_sec || a->tv_usec <= b->tv_usec)
		return 1;
	
	return 0;
}

inline void update_ts(timestamp_t *dst)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	dst->tv_sec = tv.tv_sec;
	dst->tv_usec = tv.tv_usec;
}
	

float time_diff(timestamp_t *t1, timestamp_t *t2)
{
	timestamp_t ts;
	ts_sub(&ts, t2, t1);
	return ts_to_float(&ts);
}

float diff_now(timestamp_t *t1)
{
	timestamp_t now;
	update_ts(&now);
	return time_diff(t1, &now);
}

static inline b_cnt_t __divisor(int type, int exp)
{
	static b_cnt_t cache[2][32];

	if (exp) {
		if (!cache[get_use_si()][exp]) {
			cache[get_use_si()][exp] =
				(b_cnt_t) pow(get_use_si() ||
					       type == U_NUMBER ? 1000.0f : 1024.0f, exp);
		}
		return cache[get_use_si()][exp];
	} else
		return 1;
}

static inline char * __unit(int type, int exp)
{
	static char *u[2][3][32] = {
		[0] = { /* IEC */
			[U_BITS]   = { " b ", "Kib", "Mib", "Gib", "Tib" },
			[U_BYTES]  = { " B ", "KiB", "MiB", "GiB", "TiB" },
			[U_NUMBER] = { " ", "K", "M", "G", "T" },
		},
		[1] = { /* SI */
			[U_BITS]   = { "  b", "Kb ", "Mb ", "Gb ", "Tb " },
			[U_BYTES]  = { " B ", "KB ", "MB ", "GB ", "TB " },
			[U_NUMBER] = { " ", "K", "M", "G", "T" },
		}
	};

	return u[get_use_si()][type][exp];
	
}

b_cnt_t get_divisor(b_cnt_t hint, int unit, char **dst_unit, int *prec)
{
	int yunit = get_y_unit();

	if (prec)
		*prec = 2;

	if (yunit == Y_DYNAMIC) {
		if (hint >= __divisor(unit, 3)) {
			*dst_unit = __unit(unit, 3);
			return __divisor(unit, 3);
		} else if (hint >= __divisor(unit, 2)) {
			*dst_unit = __unit(unit, 2);
			return __divisor(unit, 2);
		} else if (hint >= __divisor(unit, 1)) {
			*dst_unit = __unit(unit, 1);
			return __divisor(unit, 1);
		} else {
			*dst_unit = __unit(unit, 0);
			if (prec)
				*prec = 0;
			return 1;
		}
	} else {
		*dst_unit = __unit(unit, yunit);
		return __divisor(unit, yunit);
	}

	return 1;
}

double cancel_down(b_cnt_t l, int unit, char **dst_unit, int *prec)
{
	return ((double) l / (double) get_divisor(l, unit, dst_unit, prec));
}

const char * xinet_ntop(struct sockaddr *src, char *dst, socklen_t cnt)
{
	void *s;
	int family;

	if (src->sa_family == AF_INET6) {
		s = &((struct sockaddr_in6 *) src)->sin6_addr;
		family = AF_INET6;
	} else if (src->sa_family == AF_INET) {
		s = &((struct sockaddr_in *) src)->sin_addr;
		family = AF_INET;
	} else
		return NULL;

	return inet_ntop(family, s, dst, cnt);
}

b_cnt_t parse_size(const char *str)
{
	char *p;
	b_cnt_t l = strtol(str, &p, 0);
	if (p == str)
		return -1;

	if (*p) {
		if (!strcasecmp(p, "kb") || !strcasecmp(p, "k"))
			l *= 1024;
		else if (!strcasecmp(p, "gb") || !strcasecmp(p, "g"))
			l *= 1024*1024*1024;
		else if (!strcasecmp(p, "gbit"))
			l *= 1024*1024*1024/8;
		else if (!strcasecmp(p, "mb") || !strcasecmp(p, "m"))
			l *= 1024*1024;
		else if (!strcasecmp(p, "mbit"))
			l *= 1024*1024/8;
		else if (!strcasecmp(p, "kbit"))
			l *= 1024/8;
		else if (strcasecmp(p, "b") != 0)
			return -1;
	}

	return l;
}

#ifndef HAVE_STRDUP
char *strdup(const char *s)
{
	char *t = xcalloc(1, strlen(s) + 1);
	memcpy(t, s, strlen(s));
	return s;
}
#endif
