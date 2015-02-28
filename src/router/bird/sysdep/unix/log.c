/*
 *	BIRD Library -- Logging Functions
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/**
 * DOC: Logging
 *
 * The Logging module offers a simple set of functions for writing
 * messages to system logs and to the debug output. Message classes
 * used by this module are described in |birdlib.h| and also in the
 * user's manual.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>

#include "nest/bird.h"
#include "nest/cli.h"
#include "nest/mrtdump.h"
#include "lib/string.h"
#include "lib/lists.h"
#include "lib/unix.h"



#ifdef USE_PTHREADS

#include <pthread.h>

static pthread_mutex_t log_mutex;
static inline void log_lock(void) { pthread_mutex_lock(&log_mutex); }
static inline void log_unlock(void) { pthread_mutex_unlock(&log_mutex); }

static pthread_t main_thread;
void main_thread_init(void) { main_thread = pthread_self(); }
static int main_thread_self(void) { return pthread_equal(pthread_self(), main_thread); }

#else

static inline void log_lock(void) {  }
static inline void log_unlock(void) {  }
void main_thread_init(void) { }
static int main_thread_self(void) { return 1; }

#endif

#ifdef NEED_PRINTF

static FILE *dbgf;
static list *current_log_list;
static char *current_syslog_name; /* NULL -> syslog closed */

#ifdef HAVE_SYSLOG
#include <sys/syslog.h>

static int syslog_priorities[] = {
  LOG_DEBUG,
  LOG_DEBUG,
  LOG_DEBUG,
  LOG_INFO,
  LOG_ERR,
  LOG_WARNING,
  LOG_ERR,
  LOG_ERR,
  LOG_CRIT,
  LOG_CRIT
};
#endif

static char *class_names[] = {
  "???",
  "DBG",
  "TRACE",
  "INFO",
  "RMT",
  "WARN",
  "ERR",
  "AUTH",
  "FATAL",
  "BUG"
};

/**
 * log_commit - commit a log message
 * @class: message class information (%L_DEBUG to %L_BUG, see |lib/birdlib.h|)
 *
 * This function writes a message prepared in the log buffer to the
 * log file (as specified in the configuration). The log buffer is
 * reset after that. The log message is a full line, log_commit()
 * terminates it.
 *
 * The message class is an integer, not a first char of a string like
 * in log(), so it should be written like *L_INFO.
 */
void
log_commit(int class, buffer *buf)
{
  struct log_config *l;

  if (buf->pos == buf->end)
    strcpy(buf->end - 100, " ... <too long>");

  log_lock();
  WALK_LIST(l, *current_log_list)
    {
      if (!(l->mask & (1 << class)))
	continue;
      if (l->fh)
	{
	  if (l->terminal_flag)
	    fputs("bird: ", l->fh);
	  else
	    {
	      byte tbuf[TM_DATETIME_BUFFER_SIZE];
	      tm_format_datetime(tbuf, &config->tf_log, now);
	      fprintf(l->fh, "%s <%s> ", tbuf, class_names[class]);
	    }
	  fputs(buf->start, l->fh);
	  fputc('\n', l->fh);
	  fflush(l->fh);
	}
#ifdef HAVE_SYSLOG
      else
	syslog(syslog_priorities[class], "%s", buf->start);
#endif
    }
  log_unlock();

  /* cli_echo is not thread-safe, so call it just from the main thread */
  if (main_thread_self())
    cli_echo(class, buf->start);

  buf->pos = buf->start;
}

int buffer_vprint(buffer *buf, const char *fmt, va_list args);

static void
vlog(int class, const char *msg, va_list args)
{
  buffer buf;
  LOG_BUFFER_INIT(buf);
  buffer_vprint(&buf, msg, args);
  log_commit(class, &buf);
}


/**
 * log - log a message
 * @msg: printf-like formatting string with message class information
 * prepended (%L_DEBUG to %L_BUG, see |lib/birdlib.h|)
 *
 * This function formats a message according to the format string @msg
 * and writes it to the corresponding log file (as specified in the
 * configuration). Please note that the message is automatically
 * formatted as a full line, no need to include |\n| inside.
 * It is essentially a sequence of log_reset(), logn() and log_commit().
 */
void
log_msg(char *msg, ...)
{
  int class = 1;
  va_list args;

  va_start(args, msg);
  if (*msg >= 1 && *msg <= 8)
    class = *msg++;
  vlog(class, msg, args);
  va_end(args);
}

void
log_rl(struct tbf *f, char *msg, ...)
{
  int last_hit = f->mark;
  int class = 1;
  va_list args;

  /* Rate limiting is a bit tricky here as it also logs '...' during the first hit */
  if (tbf_limit(f) && last_hit)
    return;

  if (*msg >= 1 && *msg <= 8)
    class = *msg++;

  va_start(args, msg);
  vlog(class, (f->mark ? "..." : msg), args);
  va_end(args);
}

/**
 * bug - report an internal error
 * @msg: a printf-like error message
 *
 * This function logs an internal error and aborts execution
 * of the program.
 */
void
bug(char *msg, ...)
{
  va_list args;

  va_start(args, msg);
  vlog(L_BUG[0], msg, args);
  abort();
}

/**
 * bug - report a fatal error
 * @msg: a printf-like error message
 *
 * This function logs a fatal error and aborts execution
 * of the program.
 */
void
die(char *msg, ...)
{
  va_list args;

  va_start(args, msg);
  vlog(L_FATAL[0], msg, args);
  exit(1);
}

/**
 * debug - write to debug output
 * @msg: a printf-like message
 *
 * This function formats the message @msg and prints it out
 * to the debugging output. No newline character is appended.
 */
void
debug(char *msg, ...)
{
  va_list args;
  char buf[1024];

  va_start(args, msg);
  if (dbgf)
    {
      if (bvsnprintf(buf, sizeof(buf), msg, args) < 0)
	bsprintf(buf + sizeof(buf) - 100, " ... <too long>\n");
      fputs(buf, dbgf);
    }
  va_end(args);
}

static list *
default_log_list(int debug, int init, char **syslog_name)
{
  static list init_log_list;
  init_list(&init_log_list);
  *syslog_name = NULL;

#ifdef HAVE_SYSLOG
  if (!debug)
    {
      static struct log_config lc_syslog = { mask: ~0 };
      add_tail(&init_log_list, &lc_syslog.n);
      *syslog_name = bird_name;
      if (!init)
	return &init_log_list;
    }
#endif

  static struct log_config lc_stderr = { mask: ~0, terminal_flag: 1 };
  lc_stderr.fh = stderr;
  add_tail(&init_log_list, &lc_stderr.n);
  return &init_log_list;
}

void
log_switch(int debug, list *l, char *new_syslog_name)
{
  if (!l || EMPTY_LIST(*l))
    l = default_log_list(debug, !l, &new_syslog_name);

  current_log_list = l;

#ifdef HAVE_SYSLOG
  if (current_syslog_name && new_syslog_name &&
      !strcmp(current_syslog_name, new_syslog_name))
    return;

  if (current_syslog_name)
    closelog();

  if (new_syslog_name)
    openlog(new_syslog_name, LOG_CONS | LOG_NDELAY, LOG_DAEMON);

  current_syslog_name = new_syslog_name;
#endif
}



void
log_init_debug(char *f)
{
  if (dbgf && dbgf != stderr)
    fclose(dbgf);
  if (!f)
    dbgf = NULL;
  else if (!*f)
    dbgf = stderr;
  else if (!(dbgf = fopen(f, "a")))
    log(L_ERR "Error opening debug file `%s': %m", f);
  if (dbgf)
    setvbuf(dbgf, NULL, _IONBF, 0);
}

void
mrt_dump_message(struct proto *p, u16 type, u16 subtype, byte *buf, u32 len)
{
  /* Prepare header */
  put_u32(buf+0, now_real);
  put_u16(buf+4, type);
  put_u16(buf+6, subtype);
  put_u32(buf+8, len - MRTDUMP_HDR_LENGTH);

  if (p->cf->global->mrtdump_file != -1)
    write(p->cf->global->mrtdump_file, buf, len);
}
#endif