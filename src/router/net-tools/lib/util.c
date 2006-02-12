/* Copyright 1998 by Andi Kleen. Subject to the GPL. */ 
/* $Id: util.c,v 1.4 1998/11/17 15:17:02 freitag Exp $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>

#include "util.h"


static void oom(void)
{
    fprintf(stderr, "out of virtual memory\n");
    exit(2);
}

void *xmalloc(size_t sz)
{
    void *p = calloc(sz, 1);
    if (!p)
	oom();
    return p;
}

void *xrealloc(void *oldp, size_t sz)
{
    void *p = realloc(oldp, sz);
    if (!p)
	oom();
    return p;
}

int kernel_version(void)
{
    struct utsname uts;
    int major, minor, patch;

    if (uname(&uts) < 0)
	return -1;
    if (sscanf(uts.release, "%d.%d.%d", &major, &minor, &patch) != 3)
	return -1;
    return KRELEASE(major, minor, patch);
}


/* Like strncpy but make sure the resulting string is always 0 terminated. */  
char *safe_strncpy(char *dst, const char *src, size_t size)
{   
    dst[size-1] = '\0';
    return strncpy(dst,src,size-1);   
}
