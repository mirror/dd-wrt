/*
 * utils.h             General purpose utilities
 *
 * Copyright (c) 2001-2004 Thomas Graf <tgraf@suug.ch>
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

#ifndef __BMON_UTILS_H_
#define __BMON_UTILS_H_

#include <bmon/bmon.h>

#define COPY_TS(tv1,tv2)			\
    do {					\
        (tv1)->tv_sec = (tv2)->tv_sec;		\
        (tv1)->tv_usec = (tv2)->tv_usec;	\
    } while (0)


#if __BYTE_ORDER == __BIG_ENDIAN
#define xntohll(N) (N)
#define xhtonll(N) (N)
#else
#define xntohll(N) ((((uint64_t) ntohl(N)) << 32) + ntohl(N >> 32))
#define xhtonll(N) ((((uint64_t) htonl(N)) << 32) + htonl(N >> 32))
#endif

enum {
	U_NUMBER,
	U_BYTES,
	U_BITS
};

extern float read_delta;

extern void * xcalloc(size_t, size_t);
extern void * xrealloc(void *, size_t);
extern void xfree(void *);
extern void quit (const char *, ...);

const char * xinet_ntop(struct sockaddr *, char *, socklen_t);

extern double cancel_down(b_cnt_t, int, char **, int *);
extern b_cnt_t get_divisor(b_cnt_t, int, char **, int *);

extern inline float ts_to_float(timestamp_t *);
extern inline void float_to_ts(timestamp_t *, float);

extern inline void ts_add(timestamp_t *, timestamp_t *, timestamp_t *);
extern inline void ts_sub(timestamp_t *, timestamp_t *, timestamp_t *);
extern inline int ts_le(timestamp_t *, timestamp_t *);
extern inline void update_ts(timestamp_t *);

extern float time_diff(timestamp_t *, timestamp_t *);
extern float diff_now(timestamp_t *);

extern b_cnt_t parse_size(const char *);

#ifndef HAVE_STRDUP
extern char *strdup(const char *);
#endif

#endif
