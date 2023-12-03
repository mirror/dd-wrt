#ifndef __DAWN_UTILS_H
#define __DAWN_UTILS_H

#include <stdint.h>
#include <syslog.h>
#include <pthread.h>

/**
 * Check if a string is greater than another one.
 * @param str
 * @param str_2
 * @return
 */
int string_is_greater(char *str, char *str_2);


/*
**  Log handling for dawn process
*/
#define DAWNLOG_DEST_SYSLOG 0 // Send log output to syslog...
#define DAWNLOG_DEST_STDIO  1 // ... or stdout / stderr as appropriate

#define DAWNLOG_PERROR  0x08  // Bit flag to signal inclusion of errno from system calls

#define DAWNLOG_PRIMASK 0x07  // Bitmask to obtain only priority value

#define DAWNLOG_ERROR   5  // Serious malfunction / unexpected behaviour - eg: OS resource exhaustion
#define DAWNLOG_WARNING 4  // Something appears wrong, but recoverable - eg: data structures inconsistent
#define DAWNLOG_ALWAYS  3  // Standard behaviour always worth reporting - should be very low frequency messages
#define DAWNLOG_INFO    2  // Reporting on standard behaviour - should be comprehensible to user
#define DAWNLOG_TRACE   1  // More info to help trace where algorithms may be going wrong
#define DAWNLOG_DEBUG   0  // Deeper tracing to fix bugs

#define DAWNLOG_COMPILE_MIN DAWNLOG_DEBUG // Messages lower than this priority are not compiled
#define DAWNLOG_COMPILING(level) (level >= DAWNLOG_COMPILE_MIN)

#define dawnlog_perror(s, ...) dawnlog(DAWNLOG_ERROR|DAWNLOG_PERROR, "%s()=%s@%d %s - " s, __func__, dawnlog_basename(__FILE__), __LINE__, dawnlog_pbuf, ##__VA_ARGS__)
#define dawnlog_error(fmt, ...) dawnlog(DAWNLOG_ERROR, "%s()=%s@%d " fmt, __func__, dawnlog_basename(__FILE__), __LINE__, ##__VA_ARGS__)

#define dawnlog_warning(fmt, ...) dawnlog(DAWNLOG_WARNING, fmt, ##__VA_ARGS__)

#define dawnlog_always(fmt, ...) dawnlog(DAWNLOG_ALWAYS, fmt, ##__VA_ARGS__)

#if DAWNLOG_COMPILING(DAWNLOG_INFO)
#define dawnlog_info(fmt, ...) dawnlog(DAWNLOG_INFO, fmt, ##__VA_ARGS__)
#else
#define dawnlog_info(fmt, ...)
#endif

// Use the ..._func variants to get source code position added automatically: function, filename, line
#if DAWNLOG_COMPILING(DAWNLOG_TRACE)
#define dawnlog_trace(fmt, ...) dawnlog(DAWNLOG_TRACE, fmt, ##__VA_ARGS__)
#define dawnlog_trace_func(fmt, ...) dawnlog(DAWNLOG_TRACE, "%s()=%s@%d "  fmt, __func__, dawnlog_basename(__FILE__), __LINE__, ##__VA_ARGS__)
#else
#define dawnlog_trace(fmt, ...)
#define dawnlog_trace_func(fmt, ...)
#endif

#if DAWNLOG_COMPILING(DAWNLOG_DEBUG)
#define dawnlog_debug(fmt, ...) dawnlog(DAWNLOG_DEBUG, fmt, ##__VA_ARGS__)
#define dawnlog_debug_func(fmt, ...) dawnlog(DAWNLOG_DEBUG, "%s()=%s@%d "  fmt, __func__, dawnlog_basename(__FILE__), __LINE__, ##__VA_ARGS__)
#else
#define dawnlog_debug(fmt, ...)
#define dawnlog_debug_func(fmt, ...)
#endif

extern char dawnlog_pbuf[];  // Buffer for errno conversion for dawnlog_perror()

/**
 * Set the output target for dawnlog()
 * @param logdest: DAWNLOG_DEST_*
 */
void dawnlog_dest(int logdest);

/**
 * Minimum priority level to be logged
 * @param level: A priority level
 */
void dawnlog_minlevel(int level);

/**
 * Check whether a priority level would be actually logged to allow callers
 * to skip "expensive" preparation such as string manipulation or loops
 * @param level
 * @return TRUE if the priority level would be logged
 */
int dawnlog_showing(int level);

/**
 * Log a message.
 * @param level
 * @param fmt
 * @return
 */
void dawnlog(int level, const char* fmt, ...);

/**
 * Return pointer to filename part of full path.
 * @param file
 * @return
 */
const char* dawnlog_basename(const char* file);

/**
 ** Wrap mutex operations to help track down mis-matches
 */
#if DAWNLOG_COMPILING(DAWNLOG_DEBUG)
#define DAWN_MUTEX_WRAP
#endif

#ifdef DAWN_MUTEX_WRAP
 // Log that a mutex managed resource is being accessed so that messages can be scrutinised for bugs
 // Place these liberally near any code that accesses or retains a pointer to resources that could go away
#define dawn_mutex_require(m) _dawn_mutex_require(m, __FILE__, __LINE__)
void _dawn_mutex_require(pthread_mutex_t* m, char* f, int l);

// Log that a mutex managed resource is being locked so that messages can be scrutinised for bugs
#define dawn_mutex_lock(m) _dawn_mutex_lock(m, __FILE__, __LINE__)
int _dawn_mutex_lock(pthread_mutex_t* m, char* f, int l);

// Log that a mutex managed resource is being unlocked so that messages can be scrutinised for bugs
#define dawn_mutex_unlock(m) _dawn_mutex_unlock(m, __FILE__, __LINE__)
int _dawn_mutex_unlock(pthread_mutex_t* m, char* f, int l);
#else
#define dawn_mutex_require(m)
#define dawn_mutex_lock(m) pthread_mutex_lock(m)
#define dawn_mutex_unlock(m) pthread_mutex_unlock(m)
#endif

#endif
