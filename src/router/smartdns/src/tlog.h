/*
 * tinylog
 * Copyright (C) 2018-2024 Ruilin Peng (Nick) <pymumu@gmail.com>
 * https://github.com/pymumu/tinylog
 */

#ifndef TLOG_H
#define TLOG_H
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>

#ifdef __cplusplus
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
extern "C" {
#endif /*__cplusplus */

typedef enum {
    TLOG_DEBUG = 0,
    TLOG_INFO = 1,
    TLOG_NOTICE = 2,
    TLOG_WARN = 3,
    TLOG_ERROR = 4,
    TLOG_FATAL = 5,
    TLOG_OFF = 6,
    TLOG_END = 7
} tlog_level;

struct tlog_time {
    int year;
    unsigned int usec;
    unsigned char mon;
    unsigned char mday;
    unsigned char hour;
    unsigned char min;
    unsigned char sec;
} __attribute__((packed));

#ifndef TLOG_MAX_LINE_LEN
#define TLOG_MAX_LINE_LEN (1024)
#endif

/* TLOG FLAGS LIST */
/* set tlog not compress file when archive */
#define TLOG_NOCOMPRESS (1 << 0)

/* Set the segmentation mode to process the log, Used by the callback function to return a full log*/
#define TLOG_SEGMENT (1 << 1)

/*
 multiwrite: enable multi process write mode.
            NOTICE: maxlogsize in all processes must be same when enable this mode.
 */
#define TLOG_MULTI_WRITE (1 << 2)

/* Not Block if buffer is insufficient. */
#define TLOG_NONBLOCK (1 << 3)

/* enable log to screen */
#define TLOG_SCREEN (1 << 4)

/* enable support fork process */
#define TLOG_SUPPORT_FORK (1 << 5)

/* enable output to screen with color */
#define TLOG_SCREEN_COLOR (1 << 6)

/* Not output prefix  */
#define TLOG_FORMAT_NO_PREFIX (1 << 7)

struct tlog_loginfo {
    tlog_level level;
    const char *file;
    const char *func;
    int line;
    struct tlog_time time;
} __attribute__((packed));

/*
Function: Print log
level: Current log Levels
format: Log formats
*/
#ifndef BASE_FILE_NAME
#define BASE_FILE_NAME                                                       \
    (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 \
                                      : __FILE__)
#endif

#ifdef NEED_PRINTF

/* set log level */
extern int tlog_setlevel(tlog_level level);


/* get log level */
extern tlog_level tlog_getlevel(void);

struct tlog_log;
typedef struct tlog_log tlog_log;


static void tlog_set_maxlog_count(int count){return;}

static  tlog_log *tlog_get_root(void) {return NULL;}

static inline int tlog_ext(tlog_level level, const char *file, int line, const char *func, void *userptr, const char *format, ...) {return 0;}
static inline int tlog_vext(tlog_level level, const char *file, int line, const char *func, void *userptr, const char *format, va_list ap) {return 0;}

/* write buff to log file */
extern int tlog_write_log(char *buff, int bufflen);


/* enalbe log to screen */
static inline void tlog_setlogscreen(int enable) {}

/* enalbe early log to screen */
#define tlog_set_early_printf(enable, no_prefix, color)  do { } while(0)

/* set early log callback */
typedef void (*tlog_early_print_func)(struct tlog_loginfo *loginfo, const char *format, va_list ap);
static inline void tlog_reg_early_printf_callback(tlog_early_print_func callback) { return; }

/* Get log level in string */
extern const char *tlog_get_level_string(tlog_level level);

/*
Function: Initialize log module
logfile: log file.
maxlogsize: The maximum size of a single log file.
maxlogcount: Number of archived logs.
buffsize: Buffer size, zero for default (128K)
flag: read tlog flags
 */
static inline int tlog_init(const char *logfile, int maxlogsize, int maxlogcount, int buffsize, unsigned int flag) {}

/* flush pending log message, and exit tlog */
static inline void tlog_exit(void) {}
/*
customize log output format
steps:
1. define format function, function must be defined as tlog_format_func, use snprintf or vsnprintf format log to buffer
2. call tlog_reg_format_func to register format function.

read _tlog_format for example.
*/
typedef int (*tlog_format_func)(char *buff, int maxlen, struct tlog_loginfo *info, void *userptr, const char *format, va_list ap);
extern int tlog_reg_format_func(tlog_format_func func);

/* register log output callback 
 Note: info is invalid when flag TLOG_SEGMENT is not set.
 */
typedef int (*tlog_log_output_func)(struct tlog_loginfo *info, const char *buff, int bufflen, void *private_data);
extern int tlog_reg_log_output_func(tlog_log_output_func output, void *private_data);

/* enable early log to screen */
#define tlog_set_early_printf(enable, no_prefix, color)  do { } while(0)

/* set early log callback */
typedef void (*tlog_early_print_func)(struct tlog_loginfo *loginfo, const char *format, va_list ap);
extern void tlog_reg_early_printf_callback(tlog_early_print_func callback);

/* set early log output callback */

#define tlog_reg_early_printf_output_callback(callback, log_screen, private_data)  do { } while(0)

struct tlog_log;
typedef struct tlog_log tlog_log;
/*
Function: open a new log stream, handler should close by tlog_close
logfile: log file.
maxlogsize: The maximum size of a single log file.
maxlogcount: Number of archived logs.
buffsize: Buffer size, zero for default (128K)
flag: read tlog flags
return: log stream handler.
 */
static inline tlog_log *tlog_open(const char *logfile, int maxlogsize, int maxlogcount, int buffsize, unsigned int flag) { return 0;}

/* write buff to log file */
extern int tlog_write(struct tlog_log *log, const char *buff, int bufflen);

/* close log stream */
extern void tlog_close(tlog_log *log);

/*
Function: Print log to log stream
log: log stream
format: Log formats
*/
static inline int tlog_printf(tlog_log *log, const char *format, ...) {return 0;}

/*
Function: Print log to log stream with ap
log: log stream
format: Log formats
va_list: args list
*/
extern int tlog_vprintf(tlog_log *log, const char *format, va_list ap);

/* enalbe log to screen */
static void tlog_logscreen(tlog_log *log, int enable) {return;}

/* register output callback */
typedef int (*tlog_output_func)(struct tlog_log *log, const char *buff, int bufflen);
extern int tlog_reg_output_func(tlog_log *log, tlog_output_func output);

/* set private data */
extern void tlog_set_private(tlog_log *log, void *private_data);

/* get private data */
extern void *tlog_get_private(tlog_log *log);

/* get local time */
static inline int tlog_localtime(struct tlog_time *tm) {return 0;}


/* set max line size */
void tlog_set_maxline_size(struct tlog_log *log, int size);

/* set max log count */
extern void tlog_logcount(struct tlog_log *log, int count);

/*
Function: set log file and archive permission
log: log stream
file: log file permission, default is 640
archive: archive file permission, default is 440
*/

static void tlog_set_permission(struct tlog_log *log, mode_t file, mode_t archive){return ;}

#define tlog_debug(...) { if (tlog_getlevel() <= TLOG_DEBUG) syslog(LOG_DEBUG, ##__VA_ARGS__); }
#define tlog_info(...) {if (tlog_getlevel() <= TLOG_INFO) syslog(LOG_INFO, ##__VA_ARGS__);}
#define tlog_notice(...) {if (tlog_getlevel() <= TLOG_NOTICE) syslog(LOG_NOTICE, ##__VA_ARGS__);}
#define tlog_warn(...) {if (tlog_getlevel() <= TLOG_WARN) syslog(LOG_WARNING, ##__VA_ARGS__);}
#define tlog_error(...) {if (tlog_getlevel() <= TLOG_ERROR) syslog(LOG_ERR, ##__VA_ARGS__);}
#define tlog_fatal(...) {if (tlog_getlevel() <= TLOG_FATAL) syslog(LOG_EMERG, ##__VA_ARGS__);}
#define tlog_reg_log_output_func(output, data) do { } while(0)

#ifdef DEBUG
#define tlog(level, format, ...) do {	\
	switch(level)	\
	{	\
	case TLOG_INFO: \
	    tlog_info(format, ##__VA_ARGS__); \
	break; \
	case TLOG_DEBUG: \
	    tlog_debug(format,##__VA_ARGS__); \
	break; \
	case TLOG_NOTICE: \
	    tlog_notice(format,##__VA_ARGS__); \
	break; \
 	case TLOG_WARN: \
	    tlog_warn(format,##__VA_ARGS__); \
	break; \
	case TLOG_ERROR: \
	    tlog_error(format,##__VA_ARGS__); \
	break; \
	case TLOG_FATAL: \
	    tlog_fatal(format,##__VA_ARGS__); \
	break;  \
	} }while(0)

#else

#define tlog(level, format, ...) do {	\
	switch(level)	\
	{	\
	case TLOG_INFO: \
	    tlog_info(format, ##__VA_ARGS__); \
	break; \
	case TLOG_NOTICE: \
	    tlog_notice(format,##__VA_ARGS__); \
	break; \
 	case TLOG_WARN: \
	    tlog_warn(format,##__VA_ARGS__); \
	break; \
	case TLOG_ERROR: \
	    tlog_error(format,##__VA_ARGS__); \
	break; \
	case TLOG_FATAL: \
	    tlog_fatal(format,##__VA_ARGS__); \
	break;  \
	} }while(0)

#endif
#else
#define tlog(level, format, ...) tlog_ext(level, BASE_FILE_NAME, __LINE__, __func__, NULL, format, ##__VA_ARGS__)

struct tlog_log;
typedef struct tlog_log tlog_log;


static void tlog_set_maxlog_count(int count){return;}

static  tlog_log *tlog_get_root(void) {return NULL;}

static inline int tlog_ext(tlog_level level, const char *file, int line, const char *func, void *userptr, const char *format, ...) {return 0;}
static inline int tlog_vext(tlog_level level, const char *file, int line, const char *func, void *userptr, const char *format, va_list ap) {return 0;}

/* write buff to log file */
extern int tlog_write_log(char *buff, int bufflen);

/* set log level */
static inline int tlog_setlevel(tlog_level level) {return 0;}

/* enalbe log to screen */
static inline void tlog_setlogscreen(int enable) {}

typedef void (*tlog_early_print_func)(struct tlog_loginfo *loginfo, const char *format, va_list ap);
static inline void tlog_reg_early_printf_callback(tlog_early_print_func callback) {}

/* enalbe early log to screen */
#define tlog_set_early_printf(enable, no_prefix, color)  do { } while(0)

/* Get log level in string */
extern const char *tlog_get_level_string(tlog_level level);

/*
Function: Initialize log module
logfile: log file.
maxlogsize: The maximum size of a single log file.
maxlogcount: Number of archived logs.
buffsize: Buffer size, zero for default (128K)
flag: read tlog flags
 */
static inline int tlog_init(const char *logfile, int maxlogsize, int maxlogcount, int buffsize, unsigned int flag) {}

/* flush pending log message, and exit tlog */
static inline void tlog_exit(void) {}
/*
customize log output format
steps:
1. define format function, function must be defined as tlog_format_func, use snprintf or vsnprintf format log to buffer
2. call tlog_reg_format_func to register format function.

read _tlog_format for example.
*/
typedef int (*tlog_format_func)(char *buff, int maxlen, struct tlog_loginfo *info, void *userptr, const char *format, va_list ap);
extern int tlog_reg_format_func(tlog_format_func func);

/* register log output callback 
 Note: info is invalid when flag TLOG_SEGMENT is not set.
 */
typedef int (*tlog_log_output_func)(struct tlog_loginfo *info, const char *buff, int bufflen, void *private_data);
#define tlog_reg_log_output_func(output, data) do { } while(0)

struct tlog_log;
typedef struct tlog_log tlog_log;
/*
Function: open a new log stream, handler should close by tlog_close
logfile: log file.
maxlogsize: The maximum size of a single log file.
maxlogcount: Number of archived logs.
buffsize: Buffer size, zero for default (128K)
flag: read tlog flags
return: log stream handler.
 */
static inline tlog_log *tlog_open(const char *logfile, int maxlogsize, int maxlogcount, int buffsize, unsigned int flag) { return 0;}

/* write buff to log file */
extern int tlog_write(struct tlog_log *log, const char *buff, int bufflen);

/* close log stream */
extern void tlog_close(tlog_log *log);

/*
Function: Print log to log stream
log: log stream
format: Log formats
*/
static inline int tlog_printf(tlog_log *log, const char *format, ...) {return 0;}

/*
Function: Print log to log stream with ap
log: log stream
format: Log formats
va_list: args list
*/
extern int tlog_vprintf(tlog_log *log, const char *format, va_list ap);

/* enalbe log to screen */
static void tlog_logscreen(tlog_log *log, int enable) {return;}

/* register output callback */
typedef int (*tlog_output_func)(struct tlog_log *log, const char *buff, int bufflen);
static inline int tlog_reg_output_func(tlog_log *log, tlog_output_func output) { return 0; }

/* set private data */
extern void tlog_set_private(tlog_log *log, void *private_data);

/* get private data */
extern void *tlog_get_private(tlog_log *log);

/* get local time */
static inline int tlog_localtime(struct tlog_time *tm) {return 0;}


/* set max line size */
void tlog_set_maxline_size(struct tlog_log *log, int size);

/* set max log count */
extern void tlog_logcount(struct tlog_log *log, int count);

/*
Function: set log file and archive permission
log: log stream
file: log file permission, default is 640
archive: archive file permission, default is 440
*/

static void tlog_set_permission(struct tlog_log *log, mode_t file, mode_t archive){return ;}
#define tlog_debug(...)  do { } while(0)
#define tlog_info(...)  do { } while(0)
#define tlog_notice(...)  do { } while(0)
#define tlog_warn(...)  do { } while(0)
#define tlog_error(...)  do { } while(0)
#define tlog_fatal(...)  do { } while(0)
#define tlog_reg_early_printf_output_callback(callback, log_screen, private_data)  do { } while(0)

#endif

#endif // !TLOG_H
