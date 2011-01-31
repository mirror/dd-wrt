/* common.c - Common functions */
 
/* Written 1995 by Werner Almesberger, EPFL-LRC */

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "common.h"


void *alloc(size_t size)
{
    void *n;

    n = malloc(size);
    if (n) return n;
    perror("malloc");
    exit(1);
}


char *stralloc(const char *str)
{
    char *n;

    n = strdup(str);
    if (n) return n;
    perror("malloc");
    exit(1);
}


void die(const char *fmt,...)
{
    va_list ap;

    fflush(stdout);
    va_start(ap,fmt);
    vfprintf(stderr,fmt,ap);
    va_end(ap);
    fputc('\n',stderr);
    exit(1);
}
