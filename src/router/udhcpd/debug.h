#ifndef _DEBUG_H
#define _DEBUG_H

#include "libbb_udhcp.h"

#include <stdio.h>
#ifdef SYSLOG
#include <syslog.h>
#endif


#ifdef SYSLOG

#ifdef NEED_PRINTF
#define LOG(level, str, args...) syslog(level, str, ## args)
#else
#define LOG(level, str, args...)
#endif

//#else
//#ifdef DEBUG
//# define LOG(level, fmt, args...) do { \
        FILE *fp = fopen("/dev/console", "w"); \
        fprintf(fp, fmt, ## args); \
        fprintf(fp, "\n"); \
        fclose(fp); \
} while (0)
//#endif
//#endif

//# define OPEN_LOG(name) openlog(name, 0, 0)
# define OPEN_LOG(name) openlog(name, LOG_PID|LOG_NDELAY, LOG_DAEMON)
#define CLOSE_LOG() closelog()
#else
# define LOG_EMERG	"EMERGENCY!"
# define LOG_ALERT	"ALERT!"
# define LOG_CRIT	"critical!"
# define LOG_WARNING	"warning"
# define LOG_ERR	"error"
# define LOG_INFO	"info"
# define LOG_DEBUG	"debug"
# define LOG(level, str, args...) do { printf("%s, ", level); \
				printf(str, ## args); \
				printf("\n"); } while(0)
# define OPEN_LOG(name) do {;} while(0)
#define CLOSE_LOG() do {;} while(0)
#endif

#ifdef DEBUG
# undef DEBUG
# define DEBUG(level, str, args...) LOG(LOG_DEBUG, str, ## args)
//# define DEBUGGING
#else
# define DEBUG(level, str, args...) do {;} while(0)
#endif

//#define LOG

#endif
