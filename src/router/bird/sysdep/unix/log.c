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

#include "nest/bird.h"
#include "nest/cli.h"
#include "lib/string.h"
#include "lib/lists.h"
#include "lib/unix.h"

static FILE *dbgf = NULL;
static list *current_log_list;
static list init_log_list;

bird_clock_t rate_limit_time = 5;
int rate_limit_count = 5;

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

static void
vlog(int class, char *msg, va_list args)
{
  char buf[1024];
  struct log_config *l;

  if (bvsnprintf(buf, sizeof(buf)-1, msg, args) < 0)
    bsprintf(buf + sizeof(buf) - 100, " ... <too long>");

  WALK_LIST(l, *current_log_list)
    {
      if (!(l->mask & (1 << class)))
	continue;
      if (l->fh)
	{
	  time_t now = time(NULL);
	  struct tm *tm = localtime(&now);

	  if (l->terminal_flag)
	    fputs("bird: ", l->fh);
	  else
	    {
	      fprintf(l->fh, "%02d-%02d-%04d %02d:%02d:%02d <%s> ",
		       tm->tm_mday,
		       tm->tm_mon+1,
		       tm->tm_year+1900,
		       tm->tm_hour,
		       tm->tm_min,
		       tm->tm_sec,
		       class_names[class]);
	    }
	  fputs(buf, l->fh);
	  fputc('\n', l->fh);
	  fflush(l->fh);
	}
#ifdef HAVE_SYSLOG
      else
	syslog(syslog_priorities[class], "%s", buf);
#endif
    }
  cli_echo(class, buf);
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
log_rl(struct rate_limit *rl, char *msg, ...)
{
  int class = 1;
  va_list args;

  bird_clock_t delta = now - rl->timestamp;
  if ((0 <= delta) && (delta < rate_limit_time))
    {
      rl->count++;
    }
  else
    {
      rl->timestamp = now;
      rl->count = 1;
    }

  if (rl->count > rate_limit_count)
    return;

  va_start(args, msg);
  if (*msg >= 1 && *msg <= 8)
    class = *msg++;
  vlog(class, msg, args);
  if (rl->count == rate_limit_count)
    vlog(class, "...", args);
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
bdebug(char *msg, ...)
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

void
log_init(int debug, int init)
{
  static struct log_config lc_stderr = { mask: ~0, terminal_flag: 1 };

  init_list(&init_log_list);
  current_log_list = &init_log_list;

#ifdef HAVE_SYSLOG
  if (!debug)
    {
      static struct log_config lc_syslog = { mask: ~0 };
      openlog("bird", LOG_CONS | LOG_NDELAY, LOG_DAEMON);
      add_tail(current_log_list, &lc_syslog.n);
      if (!init)
	return;
    }
#endif

  lc_stderr.fh = stderr;
  add_tail(current_log_list, &lc_stderr.n);
}

void
log_switch(int debug, list *l)
{
  if (EMPTY_LIST(*l))
    log_init(debug, 0);
  else
    current_log_list = l;
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
