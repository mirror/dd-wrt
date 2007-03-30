#include <linux/slab.h>
#include <stdarg.h>
#include "config.h"
#include "log.h"

void log_message(int level, const char * fmt, va_list args) {
	unsigned char buff[1024];
	
	char * log_fmt = kmalloc(strlen(fmt)+256, GFP_KERNEL);
	*log_fmt = '\0';
	if (level == LOG_LEVEL_FATAL) strcat(log_fmt, "[FATAL] ");
	else if (level == LOG_LEVEL_ERROR) strcat(log_fmt, "[ERROR] ");
	else if (level == LOG_LEVEL_WARN) strcat(log_fmt, "[WARN] ");
	else if (level == LOG_LEVEL_INFO) strcat(log_fmt, "[INFO] ");
	else if (level == LOG_LEVEL_DEBUG) strcat(log_fmt, "[DEBUG] ");
	else if (level == LOG_LEVEL_TRACE) strcat(log_fmt, "[DEBUG] ");
	strcat(log_fmt, fmt);
	strcat(log_fmt, "\n");
	
	vsprintf(buff, log_fmt, args);

	kfree(log_fmt);
	
	printk(buff);
}

void log_fatal(const char * fmt, ...) {
	va_list args;
	
	va_start(args, fmt);
	log_message(LOG_LEVEL_FATAL, fmt, args);
	va_end(args);
}

void log_error(const char * fmt, ...) {
	va_list args;
	
	va_start(args, fmt);
	log_message(LOG_LEVEL_ERROR, fmt, args);
	va_end(args);
}

void log_warn(const char * fmt, ...) {
	va_list args;
	
	va_start(args, fmt);
	log_message(LOG_LEVEL_WARN, fmt, args);
	va_end(args);
}

void log_info(const char * fmt, ...) {
	va_list args;
	
	va_start(args, fmt);
	log_message(LOG_LEVEL_INFO, fmt, args);
	va_end(args);
}

#if LOG_LEVEL>=4
void log_debug(const char * fmt, ...) {
	va_list args;
	
	va_start(args, fmt);
	log_message(LOG_LEVEL_DEBUG, fmt, args);
	va_end(args);
}
#endif

#if LOG_LEVEL>=5
void log_trace(const char * fmt, ...) {
	va_list args;
	
	va_start(args, fmt);
	log_message(LOG_LEVEL_TRACE, fmt, args);
	va_end(args);
}
#endif

