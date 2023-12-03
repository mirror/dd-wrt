#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <stdarg.h>
#include <syslog.h>
#include <stdio.h>
#include <inttypes.h>

#include "utils.h"

int string_is_greater(char *str, char *str_2) {

    int length_1 = strlen((char *) str);
    int length_2 = strlen((char *) str_2);

    int length = length_1 < length_2 ? length_1 : length_2;

    for (int i = 0; i < length; i++) {
        if (str[i] > str_2[i]) {
            return 1;
        }
        if (str[i] < str_2[i]) {
            return 0;
        }
    }
    return length_1 > length_2;
}

// Size implied by https://man7.org/linux/man-pages/man3/strerror.3.html
char dawnlog_pbuf[1024] = "NOT SET";

static int _logdest = DAWNLOG_DEST_SYSLOG; // Assume daemon, logging to syslog
static int _logmin = DAWNLOG_ALWAYS;      // Anything of lower priority is suppressed

void dawnlog_dest(int logdest)
{
    _logdest = logdest;
}

void dawnlog_minlevel(int level)
{
    // Don't allow certain prioriites to be suppressed
    if (level < DAWNLOG_ALWAYS)
    {
        _logmin = level;
    }
}

int dawnlog_showing(int level)
{
    return(level >= _logmin);
}

void dawnlog(int level, const char* fmt, ...)
{

    if ((level & DAWNLOG_PRIMASK) >= _logmin)
    {
        // Attempt to replicate what perror() does...
        // dawnlog_buf is already referenced by macro expanded format string, so set it to a value
        if ((level & DAWNLOG_PERROR) == DAWNLOG_PERROR)
        {
            strerror_r(errno, dawnlog_pbuf, 1024);
        }

        va_list ap;
        va_start(ap, fmt);

        int sl = LOG_NOTICE;  // Should always be mapped to a different value
        char *iotag = "default: ";

        switch (level)
        {
        case DAWNLOG_ERROR:
            sl = LOG_ERR;
            iotag = "error: ";
            break;
        case DAWNLOG_WARNING:
            sl = LOG_WARNING;
            iotag = "warning: ";
            break;
        case DAWNLOG_ALWAYS:
            sl = LOG_INFO;
            iotag = "info: ";
            break;
        case DAWNLOG_INFO:
            sl = LOG_INFO;
            iotag = "info: ";
            break;
        case DAWNLOG_TRACE:
            sl = LOG_DEBUG;
            iotag = "trace: ";
            break;
        case DAWNLOG_DEBUG:
            sl = LOG_DEBUG;
            iotag = "debug: ";
            break;
        }

        if (_logdest == DAWNLOG_DEST_SYSLOG)
        {
            vsyslog(sl, fmt, ap);
        }
        else
        {
            int l = strlen(fmt);
            if (l)
            {
                FILE* f = (level == DAWNLOG_ERROR || level == DAWNLOG_WARNING) ? stderr : stdout;

                fprintf(f, "%s", iotag);
                vfprintf(f, fmt, ap);

                // Messages created for syslog() may not have a closing newline, so add one if using stdio
                if (fmt[l - 1] != '\n')
                    fprintf(f, "\n");
            }
        }

        va_end(ap);
    }
}

/* Return pointer to filename part of full path */
const char* dawnlog_basename(const char* file)
{
    char* xfile = strrchr(file, '/');

    return(xfile ? xfile + 1 : file);
}


#ifdef DAWN_MUTEX_WRAP
// Log that a mutex managed resource is being accessed so that messages can be scrutinised for bugs
void _dawn_mutex_require(pthread_mutex_t* m, char* f, int l)
{
    // Rely on compile time hack that we can see pthread_t is unsigned long on this dev platform
    if (sizeof(pthread_t) == sizeof(unsigned long))
    {
        pthread_t moi = pthread_self();

        if (0 == pthread_mutex_trylock(m))
        {
            pthread_mutex_unlock(m);
            dawnlog_warning("MUTEX require = %" PRIXPTR "@%s:%d[%ul] - appears to be UNLOCKED!", m, dawnlog_basename(f), l, moi);
        }
        else
            dawnlog_debug("MUTEX require = %" PRIXPTR "@%s:%d[%ul]", m, dawnlog_basename(f), l, moi);
    }

    return;
}

// Log that a mutex managed resource is being locked so that messages can be scrutinised for bugs
int _dawn_mutex_lock(pthread_mutex_t* m, char* f, int l)
{
    // Rely on compile time hack that we can see pthread_t is unsigned long on this dev platform
    if (sizeof(pthread_t) == sizeof(unsigned long))
    {
        pthread_t moi = pthread_self();

        dawnlog_debug("MUTEX    lock = %" PRIXPTR "@%s:%d[%ul]", m, dawnlog_basename(f), l, moi);
    }
    else
        dawnlog_debug("MUTEX    lock = %" PRIXPTR "@%s:%d", m, dawnlog_basename(f), l);

    return pthread_mutex_lock(m);
}

// Log that a mutex managed resource is being unlocked so that messages can be scrutinised for bugs
int _dawn_mutex_unlock(pthread_mutex_t* m, char* f, int l)
{
    // Rely on compile time hack that we can see pthread_t is unsigned long on this dev platform
    if (sizeof(pthread_t) == sizeof(unsigned long))
    {
        pthread_t moi = pthread_self();

        dawnlog_debug("MUTEX  unlock = %" PRIXPTR "@%s:%d[%ul]", m, dawnlog_basename(f), l, moi);
    }
    else
        dawnlog_debug("MUTEX  unlock = %" PRIXPTR "@%s:%d", m, dawnlog_basename(f), l);

    return pthread_mutex_unlock(m);
}
#endif
