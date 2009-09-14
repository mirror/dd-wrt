/*
 *	BIRD -- Unix Timers
 *
 *	(c) 1998 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_TIMER_H_
#define _BIRD_TIMER_H_

#include <time.h>

#include "lib/resource.h"

typedef time_t bird_clock_t;		/* Use instead of time_t */

typedef struct timer {
  resource r;
  void (*hook)(struct timer *);
  void *data;
  unsigned randomize;			/* Amount of randomization */
  unsigned recurrent;			/* Timer recurrence */
  node n;				/* Internal link */
  bird_clock_t expires;			/* 0=inactive */
} timer;

timer *tm_new(pool *);
void tm_start(timer *, unsigned after);
void tm_stop(timer *);
void tm_dump_all(void);

extern bird_clock_t now; 		/* Relative, monotonic time in seconds */
extern bird_clock_t now_real;		/* Time in seconds since fixed known epoch */

bird_clock_t tm_parse_date(char *);	/* Convert date to bird_clock_t */
bird_clock_t tm_parse_datetime(char *);	/* Convert date to bird_clock_t */
void tm_format_date(char *, bird_clock_t);	/* Convert bird_clock_t to date */
#define TM_DATE_BUFFER_SIZE 12		/* Buffer size required by tm_format_date */
void tm_format_datetime(char *, bird_clock_t);	/* Convert bird_clock_t to date + time */
#define TM_DATETIME_BUFFER_SIZE 64	/* Buffer size required by tm_format_datetime */
void tm_format_reltime(char *, bird_clock_t);	/* Convert bird_clock_t to relative datetime string */
#define TM_RELTIME_BUFFER_SIZE 12	/* Buffer size required by tm_format_reltime */

#ifdef TIME_T_IS_64BIT
#define TIME_INFINITY 0x7fffffffffffffff
#else
#ifdef TIME_T_IS_SIGNED
#define TIME_INFINITY 0x7fffffff
#else
#define TIME_INFINITY 0xffffffff
#endif
#endif

#endif
