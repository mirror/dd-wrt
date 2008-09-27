/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas T�nnesen(andreto@olsr.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 *
 * * Redistributions of source code must retain the above copyright 
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright 
 *   notice, this list of conditions and the following disclaimer in 
 *   the documentation and/or other materials provided with the 
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its 
 *   contributors may be used to endorse or promote products derived 
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */


#ifndef _OLSR_DEFS
#define _OLSR_DEFS

/* Common includes */
#include <sys/time.h>
#include <sys/times.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "olsr_protocol.h"
#include "olsr_cfg.h"

extern const char olsrd_version[];
extern const char build_date[]; 
extern const char build_host[];

#ifndef OLSRD_GLOBAL_CONF_FILE
#define OLSRD_CONF_FILE_NAME	"olsrd.conf"
#define OLSRD_GLOBAL_CONF_FILE	"/etc/" OLSRD_CONF_FILE_NAME
#endif

#define	MAXMESSAGESIZE		1500	/* max broadcast size */
#define UDP_IPV4_HDRSIZE        28
#define UDP_IPV6_HDRSIZE        62

#define MIN_PACKET_SIZE(ver)	((int)(sizeof(olsr_u8_t) * (((ver) == AF_INET) ? 4 : 7)))

/* Debug helper macro */
#ifdef DEBUG
#define olsr_debug(lvl, format, args...) do {                           \
    OLSR_PRINTF(lvl, "%s (%s:%d): ", __func__, __FILE__, __LINE__);     \
    OLSR_PRINTF(lvl, (format), ##args);                                 \
  } while (0)
#endif

extern FILE *debug_handle;

#ifdef NODEBUG
#define OLSR_PRINTF(lvl, format, args...) do { } while(0)
#else
#define OLSR_PRINTF(lvl, format, args...) do {                    \
    if((olsr_cnf->debug_level >= (lvl)) && debug_handle)          \
      fprintf(debug_handle, (format), ##args);                    \
  } while (0)
#endif

/*
 * Provides a timestamp s1 milliseconds in the future according
 * to system ticks returned by times(2)
*/
#define GET_TIMESTAMP(s1)	(now_times + ((s1) / olsr_cnf->system_tick_divider))

/* Compute the time in milliseconds when a timestamp will expire. */
#define TIME_DUE(s1)   ((int)((s1) * olsr_cnf->system_tick_divider) - now_times)

/* Returns TRUE if a timestamp is expired */
#define TIMED_OUT(s1)	((int)((s1) - now_times) < 0)


#define ARRAYSIZE(x)	(sizeof(x)/sizeof(*(x)))
#ifndef MAX
#define MAX(x,y)	((x) > (y) ? (x) : (y))
#endif
#ifndef MIN
#define MIN(x,y)	((x) < (y) ? (x) : (y))
#endif

#define INLINE inline __attribute__((always_inline))

/*
 * A somewhat safe version of strncpy and strncat. Note, that
 * BSD/Solaris strlcpy()/strlcat() differ in implementation, while
 * the BSD compiler prints out a warning if you use plain strcpy().
 */
 
static INLINE char *strscpy(char *dest, const char *src, size_t size)
{
	register size_t l = 0;
#if !defined(NODEBUG) && defined(DEBUG)
	if (sizeof(dest) == size) fprintf(stderr, "Warning: probably sizeof(pointer) in strscpy(%p, %s, %d)!\n", dest, src, size);
	if (NULL == dest) fprintf(stderr, "Warning: dest is NULL in strscpy!\n");
	if (NULL == src) fprintf(stderr, "Warning: src is NULL in strscpy!\n");
#endif
	if (NULL != dest && NULL != src)
	{
		/* src does not need to be null terminated */
		if (0 < size--) while(l < size && 0 != src[l]) l++;
		dest[l] = 0;
	}
	return strncpy(dest, src, l);
}

static INLINE char *strscat(char *dest, const char *src, size_t size)
{
	register size_t l = strlen(dest);
	return strscpy(dest + l, src, size > l ? size - l : 0);
}

/*
 * Queueing macros
 */

/* First "argument" is NOT a pointer! */

#define QUEUE_ELEM(pre, new) do { \
    (pre).next->prev = (new);         \
    (new)->next = (pre).next;         \
    (new)->prev = &(pre);             \
    (pre).next = (new);               \
  } while (0)

#define DEQUEUE_ELEM(elem) do { \
    (elem)->prev->next = (elem)->next;     \
    (elem)->next->prev = (elem)->prev;     \
  } while (0)


#define CLOSE(fd)  do { close(fd); (fd) = -1; } while (0)

/*
 * Global olsrd configuragtion
 */
extern struct olsrd_config *olsr_cnf;

/* Timer data */
extern clock_t now_times; /* current idea of times(2) reported uptime */

#if defined WIN32
extern olsr_bool olsr_win32_end_request;
extern olsr_bool olsr_win32_end_flag;
#endif

/*
 * a wrapper around times(2). times(2) has the problem, that it may return -1
 * in case of an err (e.g. EFAULT on the parameter) or immediately before an
 * overrun (though it is not en error) just because the jiffies (or whatever
 * the underlying kernel calls the smallest accountable time unit) are
 * inherently "unsigned" (and always incremented).
 */
unsigned long olsr_times(void);

/*
 *IPC functions
 *These are moved to a plugin soon
 * soon... duh!
 */

int
ipc_init(void);

#if 0
int
ipc_input(int);
#endif

int
shutdown_ipc(void);

int
ipc_output(struct olsr *);

#endif
