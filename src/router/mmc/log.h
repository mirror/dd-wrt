#ifndef __LOG_H__
	#define __LOG_H__

	void log_message(int level, const char * fmt, va_list args);
	void log_fatal(const char * fmt, ...);
	void log_error(const char * fmt, ...);
	void log_warn(const char * fmt, ...);
	void log_info(const char * fmt, ...);
#if LOG_LEVEL>=4
	void log_debug(const char * fmt, ...);
#endif
#if LOG_LEVEL>=5
	void log_trace(const char * fmt, ...);
#endif
#endif

#define LOG_LEVEL_FATAL 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARN 2
#define LOG_LEVEL_INFO 3
#define LOG_LEVEL_DEBUG 4
#define LOG_LEVEL_TRACE 5

