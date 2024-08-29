/* Tolerant /proc file parser. Copyright 1998 Andi Kleen */
/* $Id: proc.c,v 1.5 2007/12/01 18:44:57 ecki Exp $ */
/* Fixme: cannot currently cope with removed fields */

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include "util.h"

/* Caller must free return string. */

char *proc_gen_fmt(const char *name, int more, FILE * fh,...)
{
    char buf[512], format[512] = "";
    char *title, *head, *hdr;
    va_list ap;

    if (!fgets(buf, (sizeof buf) - 1, fh))
	return NULL;
    strcat(buf, " ");

    va_start(ap, fh);
    title = va_arg(ap, char *);
    for (hdr = buf; hdr;) {
	while (isspace(*hdr) || *hdr == '|')
	    hdr++;
	head = hdr;
	hdr = strpbrk(hdr, "| \t\n");
	if (hdr)
	    *hdr++ = 0;

	if (!strcmp(title, head)) {
	    strcat(format, va_arg(ap, char *));
	    title = va_arg(ap, char *);
	    if (!title || !head)
		break;
	} else {
	    strcat(format, "%*s");	/* XXX */
	}
	strcat(format, " ");
    }
    va_end(ap);

    if (!more && title) {
	fprintf(stderr, "warning: %s does not contain required field %s\n",
		name, title);
	return NULL;
    }
    return xstrdup(format);
}

/*
 * this will generate a bitmask of present/missing fields in the header of
 * a /proc file.
 */
int proc_guess_fmt(const char *name, FILE *fh, ...)
{
    char buf[512];
    char *tmp;
    int flag = 0;
    va_list ap;

    if (!fgets(buf, (sizeof buf) - 1, fh))
	return -1;
    strcat(buf, "\0");
    va_start(ap, fh);
    while((tmp = va_arg(ap, char *))) {
      int f = va_arg(ap, int);
      if (strstr(buf,tmp) != 0)
        flag |= f;
    }
    va_end(ap);
    return flag;
}


FILE *proc_fopen(const char *name)
{
    static char *buffer;
    static size_t pagesz;
    FILE *fd = fopen(name, "r");

    if (fd == NULL)
      return NULL;

    if (!buffer) {
      pagesz = getpagesize();
      buffer = malloc(pagesz);
    }

    setvbuf(fd, buffer, _IOFBF, pagesz);
    return fd;
}
