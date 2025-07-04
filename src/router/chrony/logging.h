/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2002
 * Copyright (C) Miroslav Lichvar  2013-2015
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

  Header file for diagnostic logging module

  */

#ifndef GOT_LOGGING_H
#define GOT_LOGGING_H

#include "sysincl.h"

/* Line logging macros.  If the compiler is GNU C, we take advantage of
   being able to get the function name also. */

#ifdef __GNUC__
#define FUNCTION_NAME __FUNCTION__
#define FORMAT_ATTRIBUTE_PRINTF(str, first) __attribute__ ((format (printf, str, first)))
#else
#define FUNCTION_NAME ""
#define FORMAT_ATTRIBUTE_PRINTF(str, first)
#endif

#if DEBUG > 0
#define LOG_MESSAGE(severity, ...) \
  LOG_Message(severity, __LINE__, __FILE__, FUNCTION_NAME, __VA_ARGS__)
#else
#define LOG_MESSAGE(severity, ...) \
  LOG_Message(severity, __VA_ARGS__)
#endif

#define DEBUG_LOG(...) \
  do { \
    if (DEBUG && log_min_severity == LOGS_DEBUG) \
      LOG_MESSAGE(LOGS_DEBUG, __VA_ARGS__); \
  } while (0)

#define LOG_FATAL(...) \
  do { \
    LOG_MESSAGE(LOGS_FATAL, __VA_ARGS__); \
    exit(1); \
  } while (0)

#define LOG(severity, ...) LOG_MESSAGE(severity, __VA_ARGS__)

/* Definition of severity */
typedef enum {
  LOGS_DEBUG = -1,
  LOGS_INFO = 0,
  LOGS_WARN,
  LOGS_ERR,
  LOGS_FATAL,
} LOG_Severity;

/* Minimum severity of messages to be logged */
extern LOG_Severity log_min_severity;

/* Init function */
extern void LOG_Initialise(void);

/* Fini function */
extern void LOG_Finalise(void);

/* Line logging function */
#if DEBUG > 0
FORMAT_ATTRIBUTE_PRINTF(5, 6)
extern void LOG_Message(LOG_Severity severity, int line_number, const char *filename,
                        const char *function_name, const char *format, ...);
#else
FORMAT_ATTRIBUTE_PRINTF(2, 3)
extern void LOG_Message(LOG_Severity severity, const char *format, ...);
#endif

/* Set the minimum severity of a message to be logged or printed to terminal.
   If the severity is LOGS_DEBUG and DEBUG is enabled, all messages will be
   prefixed with the filename, line number, and function name. */
extern void LOG_SetMinSeverity(LOG_Severity severity);

/* Get the minimum severity */
extern LOG_Severity LOG_GetMinSeverity(void);

/* Flags for info messages that should be logged only in specific contexts */
typedef enum {
  LOGC_Command = 1,
  LOGC_SourceFile = 2,
} LOG_Context;

/* Modify current contexts */
extern void LOG_SetContext(LOG_Context context);
extern void LOG_UnsetContext(LOG_Context context);

/* Get severity depending on the current active contexts: INFO if they contain
   at least one of the specified contexts, DEBUG otherwise */
extern LOG_Severity LOG_GetContextSeverity(LOG_Context contexts);

/* Set a prefix for debug messages */
extern void LOG_SetDebugPrefix(const char *prefix);

/* Log messages to a file instead of stderr, or stderr again if NULL */
extern void LOG_OpenFileLog(const char *log_file);

/* Log messages to syslog instead of stderr */
extern void LOG_OpenSystemLog(void);

/* Stop using stderr and send fatal message to the foreground process */
extern void LOG_SetParentFd(int fd);

/* Send a message to the foreground process */
extern int LOG_NotifyParent(const char *message);

/* Close the pipe to the foreground process */
extern void LOG_CloseParentFd(void);

/* File logging functions */

typedef int LOG_FileID;

extern LOG_FileID LOG_FileOpen(const char *name, const char *banner);

FORMAT_ATTRIBUTE_PRINTF(2, 3)
extern void LOG_FileWrite(LOG_FileID id, const char *format, ...);

extern void LOG_CycleLogFiles(void);

#endif /* GOT_LOGGING_H */
