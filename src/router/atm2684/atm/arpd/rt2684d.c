
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

#include "atm.h"
#include "atmd.h"
#include "atmarpd.h"

#include "io.h"
#include "table.h"


#ifndef NULL
#define NULL ((void *) 0)
#endif


#define COMPONENT "ARPD"


ITF *itfs = NULL;
ENTRY *unknown_incoming = NULL;
VCC *unidirectional_vccs = NULL;

int debug;
int pretty = A2T_PRETTY | A2T_NAME | A2T_LOCAL;
int merge = 0;


static void usage(const char *name)
{
    fprintf(stderr,"usage: %s [ -b ] [ -d ] [ -l logfile ] [ -m [ -m ] ] "
      "[ -n ]\n",name);
    fprintf(stderr,"%6s %s -V\n","",name);
    exit(1);
}


#ifdef BUILD_STATIC
int rt2684d_main(int argc,char **argv)
#else
int main(int argc,char **argv)
#endif
{
    const char *dump_dir;
    int c,background;

    set_application("rt2684ctl");
    set_verbosity(NULL,DIAG_INFO);
    dump_dir = ATMARP_DUMP_DIR;
    background = 0;
    while ((c = getopt(argc,argv,"i:a:t:o:r:ebdD:l:mnpV")) != EOF)
	switch (c) {
	    case 'i':
		strcpy(local_interface, optarg);
		break;
	    case 'a':
		strcpy(local_addr, optarg);
		break;
	    case 't':
		strcpy(local_netmask, optarg);
		break;
	    case 'o':
		strcpy(local_ip, optarg);
		break;
	    case 'r':
		strcpy(remote_ip, optarg);
		break;
	    case 'e':
		vcc_encap=1;
		break;

	    case 'b':
		background = 1;
		break;
	    case 'd':
		set_verbosity(NULL,DIAG_DEBUG);
		debug = 1;
		break;
	    case 'D':
		dump_dir = optarg;
		break;
	    case 'l':
		set_logfile(optarg);
		break;
	    case 'm':
		merge = 1;
		break;
	    case 'n': /* @@@ was planned for NSAP matching */
		pretty = A2T_PRETTY;
		break;
	    case 'V':
		printf("%s\n",VERSION);
		return 0;
	    case 'p':
		/* paranoid anti-firewall-tunneling mode @@@ */
	    default:
		usage(argv[0]);
	}
    if (argc != optind) usage(argv[0]);
//    diag(COMPONENT,DIAG_INFO,"Linux ATM ARP, version " VERSION);
    if (chdir(dump_dir) < 0)
	diag(COMPONENT,DIAG_ERROR,"chdir %s: %s",dump_dir,strerror(errno));
    if (debug) (void) unlink(ATMARP_TMP_DUMP_FILE); /* avoid confusion */
    open_all();
    if (background) {
    	pid_t pid;
 
	pid = fork();
	if (pid < 0) diag(COMPONENT,DIAG_FATAL,"fork: %s",strerror(errno));
	if (pid) {
	    diag(COMPONENT,DIAG_DEBUG,"Backgrounding (PID %d)",pid);
	    exit(0);
	}
    }
    (void) table_update(); /* erase old table, if any */
    poll_loop();
    close_all();
    return 0;
}
