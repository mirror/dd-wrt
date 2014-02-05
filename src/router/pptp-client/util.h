/* util.h ....... error message utilities.
 *                C. Scott Ananian <cananian@alumni.princeton.edu>
 *
 * $Id: util.h,v 1.8 2011/12/19 07:15:03 quozl Exp $
 */

#ifndef INC_UTIL_H
#define INC_UTIL_H

/* log_string is an identifier for this pptp process, passed from
   command line using --log-string=X, and included with every log message.
   Useful for people with multiple pptp sessions open at a time */
extern const char * log_string;

/* log_level sets the logging verbosity. Values range from 0 (errors only)
   to 1 (errors and warnings) to 2 (high verbosity, for debugging) */
extern int    log_level;

/*void _log(const char *func, const char *file, int line, const char *format, ...)
     __attribute__ ((format (printf, 4, 5)));
void _warn(const char *func, const char *file, int line, const char *format, ...)
     __attribute__ ((format (printf, 4, 5)));
void _fatal(const char *func, const char *file, int line, const char *format, ...)
     __attribute__ ((format (printf, 4, 5))) __attribute__ ((noreturn));
#define log(format, args...) \
	_log(__FUNCTION__,__FILE__,__LINE__, format , ## args)
#define warn(format, args...) \
	_warn(__FUNCTION__,__FILE__,__LINE__, format , ## args)
#define fatal(format, args...) \
	_fatal(__FUNCTION__,__FILE__,__LINE__, format , ## args)
*/
#define log(format, args...) do { } while(0)
#define warn(format, args...) do { } while(0)
#define fatal(format, args...) do { } while(0)

int file2fd(const char *path, const char *mode, int fd);

/* signal to pipe delivery implementation */

/* create a signal pipe, returns 0 for success, -1 with errno for failure */
int sigpipe_create(void);

/* generic handler for signals, writes signal number to pipe */
void sigpipe_handler(int signum);

/* assign a signal number to the pipe */
void sigpipe_assign(int signum);

/* return the signal pipe read file descriptor for select(2) */
int sigpipe_fd(void);

/* read and return the pending signal from the pipe */
int sigpipe_read(void);

void sigpipe_close(void);

#endif /* INC_UTIL_H */
