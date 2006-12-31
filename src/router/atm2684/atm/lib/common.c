/* common.c - Common functions */
 
/* Written 1995-1999 by Werner Almesberger, EPFL-LRC */
 

#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>

#include "atmd.h"


void *alloc(size_t size)
{
    void *n;

    n = malloc(size);
    if (n) return n;
    perror("malloc");
    exit(1);
}


uint32_t read_netl(void *p)
{
    unsigned char *_p = p;

    return (_p[0] << 24) | (_p[1] << 16) | (_p[2] << 8) | _p[3];
}
