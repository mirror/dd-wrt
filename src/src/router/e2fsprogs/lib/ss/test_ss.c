/*
 *------------------------------------------------------------------
 *
 * $Source: /usr/projects/e2fsprogs/e2fsprogs.quux/lib/ss/SCCS/s.test_ss.c $
 * $Revision: 1.13 $
 * $Date: 97/04/29 21:26:37-00:00 $
 * $State:  <unknown> $
 * $Author: tytso@mit.edu $
 * $Locker: <Not implemented> $
 *
 * $Log: <Not implemented> $
 * Revision 1.13  1997/04/29 21:26:37  tytso
 * Checkin of e2fsprogs 1.10
 *
 * Revision 1.1  1993/06/03  12:31:25  tytso
 * Initial revision
 *
 * Revision 1.1  1991/12/21  16:41:47  eichin
 * Initial revision
 *
 * Revision 1.1  1991/12/21  11:13:39  eichin
 * Initial revision
 *
 * Revision 1.2  89/01/25  07:52:27  raeburn
 * *** empty log message ***
 * 
 * Revision 1.1  88/01/23  15:50:26  raeburn
 * Initial revision
 *
 *
 *------------------------------------------------------------------
 */

#ifndef lint
static char const rcsid_test_c[] =
    "$Header: lib/ss/SCCS/s.test_ss.c 1.13 97/04/29 21:26:37-00:00 tytso@mit.edu $";
#endif /* lint */

#include <stdio.h>
#include "ss.h"

extern ss_request_table test_cmds;

#define TRUE 1
#define FALSE 0

static char def_subsystem_name[5] = "test";
static char version [4] = "1.0";
extern void ss_listen();

int main(argc, argv)
    int argc;
    char **argv;
{
    int code;
    char *argv0 = argv[0];
    char *initial_request = (char *)NULL;
    int quit = FALSE;	/* quit after processing request */
    int sci_idx;
    char *subsystem_name;

    subsystem_name = def_subsystem_name;

    for (; *argv; ++argv, --argc) {
	printf("checking arg: %s\n", *argv);
	if (!strcmp(*argv, "-prompt")) {
	    if (argc == 1) {
		fprintf(stderr,
			"No argument supplied with -prompt\n");
		exit(1);
	    }
	    argc--; argv++;
	    subsystem_name = *argv;
	}
	else if (!strcmp(*argv, "-request") || !strcmp(*argv, "-rq")) {
	    if (argc == 1) {
		fprintf(stderr,
			"No string supplied with -request.\n");
		exit(1);
	    }
	    argc--; argv++;
	    initial_request = *argv;
	}
	else if (!strcmp(*argv, "-quit"))
	    quit = TRUE;
	else if (!strcmp(*argv, "-no_quit"))
	    quit = FALSE;
	else if (**argv == '-') {
	    fprintf(stderr, "Unknown control argument %s\n",
		    *argv);
	    fprintf(stderr,
	"Usage: %s [gateway] [ -prompt name ] [ -request name ] [ -quit ]\n",
		    argv0);
	    exit(1);
	}
    }

    sci_idx = ss_create_invocation(subsystem_name, version,
				   (char *)NULL, &test_cmds, &code);
    if (code) {
	ss_perror(sci_idx, code, "creating invocation");
	exit(1);
    }

    (void) ss_add_request_table (sci_idx, &ss_std_requests, 1, &code);
    if (code) {
	ss_perror (sci_idx, code, "adding standard requests");
	exit (1);
    }

    if (!quit)
	printf("test version %s.  Type '?' for a list of commands.\n\n",
	       version);

    if (initial_request != (char *)NULL) {
	code = ss_execute_line(sci_idx, initial_request);
	if (code != 0)
	    ss_perror(sci_idx, code, initial_request);
    }
    if (!quit || code)
	(void) ss_listen (sci_idx, &code);
    exit(0);
}


void test_cmd (argc, argv)
    int argc;
    char **argv;
{
    while (++argv, --argc)
	fputs(*argv, stdout);
    putchar ('\n');
}
