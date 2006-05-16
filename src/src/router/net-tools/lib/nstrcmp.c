/* Copyright 1998 by Andi Kleen. Subject to the GPL. */
/* $Id: nstrcmp.c,v 1.2 1998/11/15 20:11:38 freitag Exp $ */ 
#include <ctype.h>
#include <stdlib.h>
#include "util.h"

/* like strcmp(), but knows about numbers */
int nstrcmp(const char *astr, const char *b)
{
    const char *a = astr;

    while (*a == *b) {
	if (*a == '\0')
	    return 0;
	a++;
	b++;
    }
    if (isdigit(*a)) {
	if (!isdigit(*b))
	    return -1;
	while (a > astr) {
	    a--;
	    if (!isdigit(*a)) {
		a++;
		break;
	    }
	    if (!isdigit(*b))
		return -1;
	    b--;
	}
	return atoi(a) > atoi(b) ? 1 : -1;
    }
    return *a - *b;
}
