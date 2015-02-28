/*
 *	BIRD Library
 *
 *	(c) 1998--2004 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_BIRDLIB_H_
#define _BIRD_BIRDLIB_H_

#include "timer.h"
#include "alloca.h"

/* Ugly structure offset handling macros */

#define OFFSETOF(s, i) ((size_t) &((s *)0)->i)
#define SKIP_BACK(s, i, p) ((s *)((char *)p - OFFSETOF(s, i)))
#define BIRD_ALIGN(s, a) (((s)+a-1)&~(a-1))

/* Utility macros */

#define MIN_(a,b) (((a)<(b))?(a):(b))
#define MAX_(a,b) (((a)>(b))?(a):(b))

#ifndef PARSER
#undef MIN
#undef MAX
#define MIN(a,b) MIN_(a,b)
#define MAX(a,b) MAX_(a,b)
#endif

#define ABS(a)   ((a)>=0 ? (a) : -(a))
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(*(a)))

#ifndef NULL
#define NULL ((void *) 0)
#endif

#ifndef IPV6
#define IP_VERSION 4
#else
#define IP_VERSION 6
#endif


/* Macros for gcc attributes */

#define NORET __attribute__((noreturn))
#define UNUSED __attribute__((unused))


/* Microsecond time */

typedef s64 btime;

#define S_	*1000000
#define MS_	*1000
#define US_	*1
#define TO_S	/1000000
#define TO_MS	/1000
#define TO_US	/1

#ifndef PARSER
#define S	S_
#define MS	MS_
#define US	US_
#endif


/* Rate limiting */

struct tbf {
  bird_clock_t timestamp;		/* Last update */
  u16 count;				/* Available tokens */
  u16 burst;				/* Max number of tokens */
  u16 rate;				/* Rate of replenishment */
  u16 mark;				/* Whether last op was limited */
};

/* Default TBF values for rate limiting log messages */
#define TBF_DEFAULT_LOG_LIMITS { .rate = 1, .burst = 5 }

void tbf_update(struct tbf *f);

static inline int
tbf_limit(struct tbf *f)
{
  tbf_update(f);

  if (!f->count)
  {
    f->mark = 1;
    return 1;
  }

  f->count--;
  f->mark = 0;
  return 0;
}


/* Logging and dying */

typedef struct buffer {
  byte *start;
  byte *pos;
  byte *end;
} buffer;

#define STACK_BUFFER_INIT(buf,size)		\
  do {						\
    buf.start = alloca(size);			\
    buf.pos = buf.start;			\
    buf.end = buf.start + size;			\
  } while(0)

#define LOG_BUFFER_INIT(buf)			\
  STACK_BUFFER_INIT(buf, LOG_BUFFER_SIZE)

#define LOG_BUFFER_SIZE 1024

#define log log_msg

#ifdef NEED_PRINTF
void log_commit(int class, buffer *buf);
void log_msg(char *msg, ...);
void log_rl(struct rate_limit *rl, char *msg, ...);
void die(char *msg, ...) NORET;
void bug(char *msg, ...) NORET;
void debug(char *msg, ...);		/* Printf to debug output */
#else
#define log_commit(class, buf) do { } while(0)
#define log_msg(msg, ...) do { } while(0)
#define log_rl(rl, msg, ...) do { } while(0)
#define die(msg, ...) do {  exit(-1); } while(0)
#define bug(msg, ...) do {  exit(-1); } while(0)
#define debug(msg, ...) do { } while(0)
#endif

#define L_DEBUG "\001"			/* Debugging messages */
#define L_TRACE "\002"			/* Protocol tracing */
#define L_INFO "\003"			/* Informational messages */
#define L_REMOTE "\004"			/* Remote protocol errors */
#define L_WARN "\005"			/* Local warnings */
#define L_ERR "\006"			/* Local errors */
#define L_AUTH "\007"			/* Authorization failed etc. */
#define L_FATAL "\010"			/* Fatal errors */
#define L_BUG "\011"			/* BIRD bugs */


/* Debugging */

#if defined(LOCAL_DEBUG) || defined(GLOBAL_DEBUG)
#define DBG(x, y...) debug(x, ##y)
#else
#define DBG(x, y...) do { } while(0)
#endif

#ifdef DEBUGGING
#define ASSERT(x) do { if (!(x)) bug("Assertion `%s' failed at %s:%d", #x, __FILE__, __LINE__); } while(0)
#else
#define ASSERT(x) do { } while(0)
#endif

/* Pseudorandom numbers */

u32 random_u32(void);

#endif
