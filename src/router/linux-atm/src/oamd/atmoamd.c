#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <signal.h>
#include <atm.h>

#include "io.h"

#define COMPONENT	"OAMD"

int timer, run_fsm, debug;

static void
usage(const char *name)
{
    fprintf(stderr, "usage: %s [-b] [-d]\n", name);
    fprintf(stderr, "%6s %s -V\n", "", name);
    exit(1);
}

static void
sig_handler(int signo)
{
    if (signo == SIGALRM) {
	timer++;
	run_fsm = 1;
	signal(SIGALRM, sig_handler);
	alarm(1);
    }
}

int
main(int argc, char **argv)
{
    int c, background = 0;

    set_application("atmoamd");
    set_verbosity(NULL,DIAG_INFO);

    while ((c = getopt(argc,argv,"bdV")) != EOF)
	switch (c) {
	    case 'b':
		background = 1;
		break;
	    case 'd':
		set_verbosity(NULL,DIAG_DEBUG);
		debug = 1;
		break;
	    case 'V':
		printf("%s\n",VERSION);
		return 0;
	    default:
		usage(argv[0]);
	}
    if (argc != optind) usage(argv[0]);
    diag(COMPONENT,DIAG_INFO,"Linux ATM OAM, version " VERSION);

    open_kernel();

    /* run in background */
    if (background) {
    	pid_t pid;
 
	pid = fork();
	if (pid < 0) diag(COMPONENT,DIAG_FATAL,"fork: %s",strerror(errno));
	if (pid) {
	    diag(COMPONENT,DIAG_DEBUG,"Backgrounding (PID %d)",pid);
	    exit(0);
	}
    }

    signal(SIGALRM, sig_handler);
    alarm(1);

    poll_loop();

    close_kernel();

    return 0;
}
