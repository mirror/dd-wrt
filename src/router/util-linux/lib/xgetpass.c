/*
 * A function to read the passphrase either from the terminal or from
 * an open file descriptor.
 *
 * Public domain.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include "c.h"
#include "xgetpass.h"

char *xgetpass(int pfd, const char *prompt)
{
	char *p = NULL, *n;
	int x = 0, y = 0;

        if (pfd < 0) {
		/* read from terminal */
		p = getpass(prompt);
		if(!p) return NULL;
		/* caller insists on free()ing this pointer, so make sure it is such */
		n = strdup(p);
		y = strlen(p);
		if(y) memset(p, 0, y); /* erase original */
		/* return a free()able copy, or 0 if strdup() failed */
		return n;
	}

	do {
		if(y >= (x - 1)) {
			x += 128;
			/* Must enforce some max limit here -- this code   */
			/* runs as part of mount, and mount is setuid root */
			/* and has used mlockall(MCL_CURRENT | MCL_FUTURE) */
			if(x > (4*1024)) {
				error_out:
				if(p) {
					memset(p, 0, y);
					free(p);
				}
				return NULL;
			}
			n = malloc(x);
			if(!n) goto error_out;
			if(p) {
				memcpy(n, p, y);
				memset(p, 0, y);
				free(p);
			}
			p = n;
		}
		if(read(pfd, p + y, 1) != 1) break;
		if((p[y] == '\n') || !p[y]) break;
		y++;
	} while(1);
	if(p) p[y] = 0;
	return p;
}
