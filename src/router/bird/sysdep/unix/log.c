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

static FILE *dbgf;
static list *current_log_list;
static char *current_syslog_name; /* NULL -> syslog closed */

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

#define LOG_BUFFER_SIZE 1024
static char log_buffer[LOG_BUFFER_SIZE];
static char *log_buffer_pos;
static int log_buffer_remains;

const char *log_buffer_ptr = log_buffer;


/**
 * log_reset - reset the log buffer
 *
 * This function resets a log buffer and discards buffered
 * messages. Should be used before a log message is prepared
 * using logn().
 */
void
log_reset(void)
{
  log_buffer_pos = log_buffer;
  log_buffer_remains = LOG_BUFFER_SIZE;
  log_buffer[0] = 0;
}

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
log_commit(int class)
{
  struct log_config *l;

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
	  fputs(log_buffer, l->fh);
	  fputc('\n', l->fh);
	  fflush(l->fh);
	}
#ifdef HAVE_SYSLOG
      else
	syslog(syslog_priorities[class], "%s", log_buffer);
#endif
    }
  cli_echo(class, log_buffer);

  log_reset();
}

static void
log_print(const char *msg, va_list args)
{
  int i;

  if (log_buffer_remains == 0)
    return;

  i=bvsnprintf(log_buffer_pos, log_buffer_remains, msg, args);
  if (i < 0)
    {
      bsprintf(log_buffer + LOG_BUFFER_SIZE - 100, " ... <too long>");
      log_buffer_remains = 0;
      return;
    }

  log_buffer_pos += i;
  log_buffer_remains -= i;
}


static void
vlog(int class, const char *msg, va_list args)
{
  log_reset();
  log_print(msg, args);
  log_commit(class);
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

/**
 * logn - prepare a partial message in the log buffer
 * @msg: printf-like formatting string (without message class information)
 *
 * This function formats a message according to the format string @msg
 * and adds it to the log buffer. Messages in the log buffer are
 * logged when the buffer is flushed using log_commit() function. The
 * message should not contain |\n|, log_commit() also terminates a
 * line.
 */
void
logn(char *msg, ...)
{
  va_list args;

  va_start(args, msg);
  log_print(msg, args);
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
