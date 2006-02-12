/*
 * lib/getargs.c      General argument parser.
 *
 * Version:     $Id: getargs.c,v 1.3 1998/11/15 20:09:43 freitag Exp $
 *
 * Author:      Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
 *              Copyright 1993,1994 MicroWalt Corporation
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 */
#include "config.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "net-support.h"
#include "pathnames.h"


/* Split the input string into multiple fields. */
int getargs(char *string, char *arguments[])
{
    int len = strlen(string); 
    char temp[len+1];
    char *sp, *ptr;
    int i, argc;
    char want;

    /*
     * Copy the string into a buffer.  We may have to modify
     * the original string because of all the quoting...
     */
    sp = string;
    i = 0;
    strcpy(temp, string);
    ptr = temp;

    /*
     * Look for delimiters ("); if present whatever
     * they enclose will be considered one argument.
     */
    while (*ptr != '\0' && i < 31) {
	/* Ignore leading whitespace on input string. */
	while (*ptr == ' ' || *ptr == '\t')
	    ptr++;

	/* Set string pointer. */
	arguments[i++] = sp;

	/* Check for any delimiters. */
	if (*ptr == '"' || *ptr == '\'') {
	    /*
	     * Copy the string up to any whitespace OR the next
	     * delimiter. If the delimiter was escaped, skip it
	     * as it if was not there.
	     */
	    want = *ptr++;
	    while (*ptr != '\0') {
		if (*ptr == want && *(ptr - 1) != '\\') {
		    ptr++;
		    break;
		}
		*sp++ = *ptr++;
	    }
	} else {
	    /* Just copy the string up to any whitespace. */
	    while (*ptr != '\0' && *ptr != ' ' && *ptr != '\t')
		*sp++ = *ptr++;
	}
	*sp++ = '\0';

	/* Skip trailing whitespace. */
	if (*ptr != '\0') {
	    while (*ptr == ' ' || *ptr == '\t')
		ptr++;
	}
    }
    argc = i;
    while (i < 32)
	arguments[i++] = (char *) NULL;
    return (argc);
}
