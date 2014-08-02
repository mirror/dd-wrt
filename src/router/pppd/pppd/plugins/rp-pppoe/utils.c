#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <syslog.h>

void dbglog(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vsyslog(LOG_DEBUG, fmt, ap);
    vfprintf(stderr, fmt, ap);
    fputs("\n", stderr);
    va_end(ap);
}

void info(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vsyslog(LOG_INFO, fmt, ap);
    vfprintf(stderr, fmt, ap);
    fputs("\n", stderr);
    va_end(ap);
}

void warn(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vsyslog(LOG_WARNING, fmt, ap);
    vfprintf(stderr, fmt, ap);
    fputs("\n", stderr);
    va_end(ap);
}

void error(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vsyslog(LOG_ERR, fmt, ap);
    vfprintf(stderr, fmt, ap);
    fputs("\n", stderr);
    va_end(ap);
}

void fatal(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vsyslog(LOG_ERR, fmt, ap);
    vfprintf(stderr, fmt, ap);
    fputs("\n", stderr);
    va_end(ap);
    exit(1);
}

