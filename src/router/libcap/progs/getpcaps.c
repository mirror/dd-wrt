/*
 * Copyright (c) 1997,2008 Andrew G. Morgan  <morgan@kernel.org>
 *
 * This displays the capabilities of given target process(es).
 */

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/capability.h>

static void usage(void)
{
    fprintf(stderr,
"usage: getcaps <pid> [<pid> ...]\n\n"
"  This program displays the capabilities on the queried process(es).\n"
"  The capabilities are displayed in the cap_from_text(3) format.\n\n"
"[Copyright (c) 1997-8,2007 Andrew G. Morgan  <morgan@kernel.org>]\n"
	);
    exit(1);
}

int main(int argc, char **argv)
{
    int retval = 0;

    if (argc < 2) {
	usage();
    }

    for ( ++argv; --argc > 0; ++argv ) {
	ssize_t length;
	int pid;
	cap_t cap_d;

	pid = atoi(argv[0]);

	cap_d = cap_get_pid(pid);
	if (cap_d == NULL) {
		fprintf(stderr, "Failed to get cap's for process %d:"
			" (%s)\n", pid, strerror(errno));
		retval = 1;
		continue;
	} else {
	    char *result = cap_to_text(cap_d, &length);
	    fprintf(stderr, "Capabilities for `%s': %s\n", *argv, result);
	    cap_free(result);
	    result = NULL;
	    cap_free(cap_d);
	}
    }

    return retval;
}
