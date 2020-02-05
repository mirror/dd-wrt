/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * Copyright (C) Miroslav Lichvar  2011-2014, 2018
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 **********************************************************************

  =======================================================================

  Module to handle logging of diagnostic information
  */

#include "config.h"

#include "sysincl.h"

#include "conf.h"
#include "logging.h"
#include "util.h"

/* This is used by DEBUG_LOG macro */
int log_debug_enabled = 0;

/* ================================================== */
/* Flag indicating we have initialised */
static int initialised = 0;

static FILE *file_log;
static int system_log = 0;

static int parent_fd = 0;

#define DEBUG_LEVEL_PRINT_FUNCTION 2
#define DEBUG_LEVEL_PRINT_DEBUG 2
static int debug_level = 0;

struct LogFile {
  const char *name;
  const char *banner;
  FILE *file;
  unsigned long writes;
};

static int n_filelogs = 0;

/* Increase this when adding a new logfile */
#define MAX_FILELOGS 6

static struct LogFile logfiles[MAX_FILELOGS];

/* ================================================== */
/* Init function */

void
LOG_Initialise(void)
{
  initialised = 1;
  file_log = stderr;
}

/* ================================================== */
/* Fini function */

void
LOG_Finalise(void)
{
  if (system_log)
    closelog();

  if (file_log)
    fclose(file_log);

  LOG_CycleLogFiles();

  initialised = 0;
}

/* ================================================== */

static void log_message(int fatal, LOG_Severity severity, const char *message)
{
  if (system_log) {
    int priority;
    switch (severity) {
      case LOGS_DEBUG:
        priority = LOG_DEBUG;
        break;
      case LOGS_INFO:
        priority = LOG_INFO;
        break;
      case LOGS_WARN:
        priority = LOG_WARNING;
        break;
      case LOGS_ERR:
        priority = LOG_ERR;
        break;
      case LOGS_FATAL:
        priority = LOG_CRIT;
        break;
      default:
        assert(0);
    }
    syslog(priority, fatal ? "Fatal error : %s" : "%s", message);
  } else if (file_log) {
    fprintf(file_log, fatal ? "Fatal error : %s\n" : "%s\n", message);
  }
}

/* ================================================== */

void LOG_Message(LOG_Severity severity,
#if DEBUG > 0
                 int line_number, const char *filename, const char *function_name,
#endif
                 const char *format, ...)
{
  char buf[2048];
  va_list other_args;
  time_t t;
  struct tm *tm;

  if (!system_log && file_log) {
    /* Don't clutter up syslog with timestamps and internal debugging info */
    time(&t);
    tm = gmtime(&t);
    if (tm) {
      strftime(buf, sizeof (buf), "%Y-%m-%dT%H:%M:%SZ", tm);
      fprintf(file_log, "%s ", buf);
    }
#if DEBUG > 0
    if (debug_level >= DEBUG_LEVEL_PRINT_FUNCTION)
      fprintf(file_log, "%s:%d:(%s) ", filename, line_number, function_name);
#endif
  }

  va_start(other_args, format);
  vsnprintf(buf, sizeof(buf), format, other_args);
  va_end(other_args);

  switch (severity) {
    case LOGS_DEBUG:
    case LOGS_INFO:
    case LOGS_WARN:
    case LOGS_ERR:
      log_message(0, severity, buf);
      break;
    case LOGS_FATAL:
      log_message(1, severity, buf);

      /* Send the message also to the foreground process if it is
         still running, or stderr if it is still open */
      if (parent_fd > 0) {
        if (write(parent_fd, buf, strlen(buf) + 1) < 0)
          ; /* Not much we can do here */
      } else if (system_log && parent_fd == 0) {
        system_log = 0;
        log_message(1, severity, buf);
      }
      break;
    default:
      assert(0);
  }
}

/* ================================================== */

void
LOG_OpenFileLog(const char *log_file)
{
  FILE *f;

  if (log_file) {
    f = fopen(log_file, "a");
    if (!f)
      LOG_FATAL("Could not open log file %s", log_file);
  } else {
    f = stderr;
  }

  /* Enable line buffering */
  setvbuf(f, NULL, _IOLBF, BUFSIZ);

  if (file_log && file_log != stderr)
    fclose(file_log);

  file_log = f;
}


/* ================================================== */

void
LOG_OpenSystemLog(void)
{
  system_log = 1;
  openlog("chronyd", LOG_PID, LOG_DAEMON);
}

/* ================================================== */

void LOG_SetDebugLevel(int level)
{
  debug_level = level;
  if (level >= DEBUG_LEVEL_PRINT_DEBUG) {
    log_debug_enabled = 1;
  }
}

/* ================================================== */

void
LOG_SetParentFd(int fd)
{
  parent_fd = fd;
  if (file_log == stderr)
    file_log = NULL;
}

/* ================================================== */

void
LOG_CloseParentFd()
{
  if (parent_fd > 0)
    close(parent_fd);
  parent_fd = -1;
}

/* ================================================== */

LOG_FileID
LOG_FileOpen(const char *name, const char *banner)
{
  assert(n_filelogs < MAX_FILELOGS);

  logfiles[n_filelogs].name = name;
  logfiles[n_filelogs].banner = banner;
  logfiles[n_filelogs].file = NULL;
  logfiles[n_filelogs].writes = 0;

  return n_filelogs++;
}

/* ================================================== */

void
LOG_FileWrite(LOG_FileID id, const char *format, ...)
{
  va_list other_args;
  int banner;

  if (id < 0 || id >= n_filelogs || !logfiles[id].name)
    return;

  if (!logfiles[id].file) {
    char filename[512], *logdir = CNF_GetLogDir();

    if (logdir[0] == '\0') {
      LOG(LOGS_WARN, "logdir not specified");
      logfiles[id].name = NULL;
      return;
    }

    if (snprintf(filename, sizeof(filename), "%s/%s.log",
                 logdir, logfiles[id].name) >= sizeof (filename) ||
        !(logfiles[id].file = fopen(filename, "a"))) {
      LOG(LOGS_WARN, "Could not open log file %s", filename);
      logfiles[id].name = NULL;
      return;
    }

    /* Close on exec */
    UTI_FdSetCloexec(fileno(logfiles[id].file));
  }

  banner = CNF_GetLogBanner();
  if (banner && logfiles[id].writes++ % banner == 0) {
    char bannerline[256];
    int i, bannerlen;

    bannerlen = strlen(logfiles[id].banner);

    for (i = 0; i < bannerlen; i++)
      bannerline[i] = '=';
    bannerline[i] = '\0';

    fprintf(logfiles[id].file, "%s\n", bannerline);
    fprintf(logfiles[id].file, "%s\n", logfiles[id].banner);
    fprintf(logfiles[id].file, "%s\n", bannerline);
  }

  va_start(other_args, format);
  vfprintf(logfiles[id].file, format, other_args);
  va_end(other_args);
  fprintf(logfiles[id].file, "\n");

  fflush(logfiles[id].file);
}

/* ================================================== */

void
LOG_CycleLogFiles(void)
{
  LOG_FileID i;

  for (i = 0; i < n_filelogs; i++) {
    if (logfiles[i].file)
      fclose(logfiles[i].file);
    logfiles[i].file = NULL;
    logfiles[i].writes = 0;
  }
}

/* ================================================== */
