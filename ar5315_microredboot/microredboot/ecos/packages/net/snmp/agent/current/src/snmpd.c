//==========================================================================
//
//      ./agent/current/src/snmpd.c
//
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//####UCDSNMPCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from the UCD-SNMP
// project,  <http://ucd-snmp.ucdavis.edu/>  from the University of
// California at Davis, which was originally based on the Carnegie Mellon
// University SNMP implementation.  Portions of this software are therefore
// covered by the appropriate copyright disclaimers included herein.
//
// The release used was version 4.1.2 of May 2000.  "ucd-snmp-4.1.2"
// -------------------------------------------
//
//####UCDSNMPCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    hmt
// Contributors: hmt
// Date:         2000-05-30
// Purpose:      Port of UCD-SNMP distribution to eCos.
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================
/********************************************************************
       Copyright 1989, 1991, 1992 by Carnegie Mellon University

			  Derivative Work -
Copyright 1996, 1998, 1999, 2000 The Regents of the University of California

			 All Rights Reserved

Permission to use, copy, modify and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appears in all copies and
that both that copyright notice and this permission notice appear in
supporting documentation, and that the name of CMU and The Regents of
the University of California not be used in advertising or publicity
pertaining to distribution of the software without specific written
permission.

CMU AND THE REGENTS OF THE UNIVERSITY OF CALIFORNIA DISCLAIM ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL CMU OR
THE REGENTS OF THE UNIVERSITY OF CALIFORNIA BE LIABLE FOR ANY SPECIAL,
INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
FROM THE LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*********************************************************************/
/*
 * snmpd.c - rrespond to SNMP queries from management stations
 *
 */
/***********************************************************
	Copyright 1988, 1989 by Carnegie Mellon University

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of CMU not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

CMU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
CMU BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
******************************************************************/
#include <config.h>

#include <stdio.h>
#include <errno.h>
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/types.h>
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#if TIME_WITH_SYS_TIME
# ifdef WIN32
#  include <sys/timeb.h>
# else
#  include <sys/time.h>
# endif
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#elif HAVE_WINSOCK_H
#include <winsock.h>
#endif
#if HAVE_NET_IF_H
#include <net/if.h>
#endif
#if HAVE_INET_MIB2_H
#include <inet/mib2.h>
#endif
#if HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#if HAVE_SYS_FILE_H
#include <sys/file.h>
#endif
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#include <signal.h>
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifndef FD_SET
typedef long    fd_mask;
#define NFDBITS (sizeof(fd_mask) * NBBY)        /* bits per mask */
#define FD_SET(n, p)    ((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define FD_CLR(n, p)    ((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define FD_ISSET(n, p)  ((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)      memset((p), 0, sizeof(*(p)))
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include "asn1.h"
#include "snmp_api.h"
#include "snmp_impl.h"
#include "system.h"
#include "read_config.h"
#include "snmp.h"
#include "mib.h"
#include "m2m.h"
#include "snmp_vars.h"
#include "agent_read_config.h"
#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
#include "snmpv3.h"
#endif
#include "callback.h"
#include "snmp_alarm.h"
#include "default_store.h"
#include "mib_module_config.h"

#include "snmp_client.h"
#include "snmpd.h"
#include "var_struct.h"
#include "mibgroup/struct.h"
#include "mibgroup/util_funcs.h"
#include "snmp_debug.h"
#include "mib_modules.h"

#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
#include "snmpusm.h"
#endif
#include "tools.h"
#include "lcd_time.h"

#include "transform_oids.h"

#include "snmp_agent.h"
#include "agent_trap.h"
#include "ds_agent.h"
#include "agent_read_config.h"
#include "snmp_logging.h"

#include "version.h"

#include "mib_module_includes.h"

/*
 * Globals.
 */
#ifdef USE_LIBWRAP
#include <tcpd.h>

int allow_severity	 = LOG_INFO;
int deny_severity	 = LOG_WARNING;
#endif  /* USE_LIBWRAP */

#define TIMETICK         500000L
#define ONE_SEC         1000000L

static int 	log_addresses	 = 0;
int 		snmp_dump_packet;
static int      running          = 1;
static int	reconfig	 = 0;

struct addrCache {
    in_addr_t	addr;
    int		status;
#define UNUSED	0
#define USED	1
#define OLD	2
};

#define ADDRCACHE 10

static struct addrCache	addrCache[ADDRCACHE];
static int		lastAddrAge = 0;

extern char **argvrestartp;
extern char  *argvrestart;
extern char  *argvrestartname;

#define NUM_SOCKETS	32

#ifdef USING_SMUX_MODULE
static int sdlist[NUM_SOCKETS], sdlen = 0;
int smux_listen_sd;
#endif /* USING_SMUX_MODULE */

/*
 * Prototypes.
 */
int snmp_read_packet (int);
int snmp_input (int, struct snmp_session *, int, struct snmp_pdu *, void *);
int main (int, char **);
static void SnmpTrapNodeDown (void);
static int receive(void);
int snmp_check_packet(struct snmp_session*, snmp_ipaddr);
int snmp_check_parse(struct snmp_session*, struct snmp_pdu*, int);

#if !defined(__ECOS)
static void usage(char *prog)
{
	printf("\nUsage:  %s [-h] [-v] [-f] [-a] [-d] [-V] [-P PIDFILE] [-q] [-D] [-p NUM] [-L] [-l LOGFILE] [-r]",prog);
#if HAVE_UNISTD_H
	printf(" [-u uid] [-g gid]");
#endif
	printf("\n");
	printf("\n\tVersion:  %s\n",VersionInfo);
	printf("\tAuthor:   Wes Hardaker\n");
	printf("\tEmail:    ucd-snmp-coders@ucd-snmp.ucdavis.edu\n");
	printf("\n-h\t\tThis usage message.\n");
	printf("-H\t\tDisplay configuration file directives understood.\n");
	printf("-v\t\tVersion information.\n");
	printf("-f\t\tDon't fork from the shell.\n");
	printf("-a\t\tLog addresses.\n");
	printf("-d\t\tDump sent and received UDP SNMP packets\n");
	printf("-V\t\tVerbose display\n");
	printf("-P PIDFILE\tUse PIDFILE to store process id\n");
	printf("-q\t\tPrint information in a more parsable format (quick-print)\n");
	printf("-D\t\tTurn on debugging output\n");
	printf("-p NUM\t\tRun on port NUM instead of the default:  161\n");
	printf("-x SOCKADDR\tBind AgentX to this address\n");
	printf("-c CONFFILE\tRead CONFFILE as a configuration file.\n");
	printf("-C\t\tDon't read the default configuration files.\n");
	printf("-L\t\tPrint warnings/messages to stdout/err\n");
	printf("-s\t\tLog warnings/messages to syslog\n");
	printf("-A\t\tAppend to the logfile rather than truncating it.\n");
	printf("-r Don't exit if root only accessible files can't be opened\n");
	printf("-l LOGFILE\tPrint warnings/messages to LOGFILE\n");
	printf("\t\t(By default LOGFILE=%s)\n",
#ifdef LOGFILE
			LOGFILE
#else
			"none"
#endif
	      );
#if HAVE_UNISTD_H
	printf("-g \t\tChange to this gid after opening port\n");
	printf("-u \t\tChange to this uid after opening port\n");
#endif
	printf("\n");
	exit(1);
}
#endif // !defined(__ECOS)

	RETSIGTYPE
SnmpdShutDown(int a)
{
	running = 0;
}

#ifdef SIGHUP
	RETSIGTYPE
SnmpdReconfig(int a)
{
	reconfig = 1;
	signal(SIGHUP, SnmpdReconfig);
}
#endif

#ifdef SIGUSR1
extern void dump_registry( void );
	RETSIGTYPE
SnmpdDump(int a)
{
	dump_registry();
	signal(SIGUSR1, SnmpdDump);
}
#endif


	static void
SnmpTrapNodeDown(void)
{
    send_easy_trap (SNMP_TRAP_ENTERPRISESPECIFIC, 2);
    /* XXX  2 - Node Down #define it as NODE_DOWN_TRAP */
}

/*******************************************************************-o-******
 * main
 *
 * Parameters:
 *	 argc
 *	*argv[]
 *      
 * Returns:
 *	0	Always succeeds.  (?)
 *
 *
 * Setup and start the agent daemon.
 *
 * Also successfully EXITs with zero for some options.
 */
#if !defined(__ECOS)

	int
main(int argc, char *argv[])
{
	int             arg, i;
	int             ret;
	u_short         dest_port = SNMP_PORT;
	int             dont_fork = 0;
	char            logfile[SNMP_MAXBUF_SMALL];
	char           *cptr, **argvptr;
	char           *pid_file = NULL;
#if HAVE_GETPID
	FILE           *PID;
#endif
	int             dont_zero_log = 0;
	int             stderr_log=0, syslog_log=0;
	int             uid=0, gid=0;

	logfile[0]		= 0;

#ifdef LOGFILE
	strcpy(logfile, LOGFILE);
#endif


	/*
	 * usage: snmpd
	 */
	for (arg = 1; arg < argc; arg++)
          {
            if (argv[arg][0] == '-') {
              switch (argv[arg][1]) {

                case 'c':
                  if (++arg == argc)
                    usage(argv[0]);
                  ds_set_string(DS_LIBRARY_ID, DS_LIB_OPTIONALCONFIG,
                                 argv[arg]);
                  break;

                case 'C':
                    ds_set_boolean(DS_LIBRARY_ID, DS_LIB_DONT_READ_CONFIGS, 1);
                    break;

		case 'd':
                    snmp_set_dump_packet(++snmp_dump_packet);
		    ds_set_boolean(DS_APPLICATION_ID, DS_AGENT_VERBOSE, 1);
		    break;

		case 'q':
		    snmp_set_quick_print(1);
		    break;

                case 'T':
                    if (argv[arg][2] != '\0') 
                        cptr = &argv[arg][2];
                    else if (++arg>argc) {
                        fprintf(stderr,"Need UDP or TCP after -T flag.\n");
                        usage(argv[0]);
                        exit(1);
                    } else {
                        cptr = argv[arg];
                    }
                    if (strcasecmp(cptr,"TCP") == 0) {
                        ds_set_int(DS_APPLICATION_ID, DS_AGENT_FLAGS,
                                   ds_get_int(DS_APPLICATION_ID, DS_AGENT_FLAGS)
                                   | SNMP_FLAGS_STREAM_SOCKET);
                    } else if (strcasecmp(cptr,"UDP") == 0) {
                        /* default, do nothing */
                    } else {
                        fprintf(stderr,
                                "Unknown transport \"%s\" after -T flag.\n",
                                cptr);
                        usage(argv[0]);
                        exit(1);
                    }
                    break;

		case 'D':
                    debug_register_tokens(&argv[arg][2]);
		    snmp_set_do_debugging(1);
		    break;

                case 'p':
                  if (++arg == argc)
                    usage(argv[0]);
                  dest_port = atoi(argv[arg]);
                  if (dest_port <= 0)
                    usage(argv[0]);
                  break;

                case 'x':
                  if (++arg == argc)
                    usage(argv[0]);
                  ds_set_string(DS_APPLICATION_ID, DS_AGENT_X_SOCKET, argv[arg]);
                  break;

		case 'r':
                    ds_set_boolean(DS_APPLICATION_ID,
                                      DS_AGENT_NO_ROOT_ACCESS, 1);
		    break;

                case 'P':
                  if (++arg == argc)
                    usage(argv[0]);
                  pid_file = argv[arg];

                case 'a':
                      log_addresses++;
                  break;

                case 'V':
                  ds_set_boolean(DS_APPLICATION_ID, DS_AGENT_VERBOSE, 1);
                  break;

                case 'f':
                  dont_fork = 1;
                  break;

                case 'l':
                  if (++arg == argc)
                    usage(argv[0]);
                  strcpy(logfile, argv[arg]);
                  break;

                case 'L':
		    stderr_log=1;
                    break;

		case 's':
		    syslog_log=1;
		    break;

                case 'A':
                    dont_zero_log = 1;
                    break;
#if HAVE_UNISTD_H
                case 'u':
                  if (++arg == argc) usage(argv[0]);
                  uid = atoi(argv[arg]);
                  break;
                case 'g':
                  if (++arg == argc) usage(argv[0]);
                  gid = atoi(argv[arg]);
                  break;
#endif
                case 'h':
                  usage(argv[0]);
                  break;
                case 'H':
                  init_agent("snmpd");   /* register our .conf handlers */
                  init_mib_modules();
                  init_snmp("snmpd");
                  fprintf(stderr, "Configuration directives understood:\n");
                  read_config_print_usage("  ");
                  exit(0);
                case 'v':
                  printf("\nUCD-snmp version:  %s\n",VersionInfo);
                  printf("Author:            Wes Hardaker\n");
                  printf("Email:             ucd-snmp-coders@ucd-snmp.ucdavis.edu\n\n");
                  exit (0);
                case '-':
                  switch(argv[arg][2]){
                    case 'v': 
                      printf("\nUCD-snmp version:  %s\n",VersionInfo);
                      printf("Author:            Wes Hardaker\n");
                      printf("Email:             ucd-snmp-coders@ucd-snmp.ucdavis.edu\n\n");
                      exit (0);
                    case 'h':
                      usage(argv[0]);
                      exit(0);
                  }

                default:
                  printf("invalid option: %s\n", argv[arg]);
                  usage(argv[0]);
                  break;
              }
              continue;
            }
	}  /* end-for */

	/* 
	 * Initialize a argv set to the current for restarting the agent.
	 */
	argvrestartp = (char **) malloc((argc + 2) * sizeof(char *));
	argvptr = argvrestartp;
	for (i = 0, ret = 1; i < argc; i++) {
		ret += strlen(argv[i]) + 1;
	}
	argvrestart = (char *) malloc(ret);
	argvrestartname = (char *) malloc(strlen(argv[0]) + 1);
	strcpy(argvrestartname, argv[0]);
	if ( strstr(argvrestartname, "agentxd") != NULL)
          ds_set_boolean(DS_APPLICATION_ID, DS_AGENT_ROLE, SUB_AGENT);
	else
          ds_set_boolean(DS_APPLICATION_ID, DS_AGENT_ROLE, MASTER_AGENT);
	for (cptr = argvrestart, i = 0; i < argc; i++) {
		strcpy(cptr, argv[i]);
		*(argvptr++) = cptr;
		cptr += strlen(argv[i]) + 1;
	}
	*cptr = 0;
	*argvptr = NULL;


	/* 
	 * Open the logfile if necessary.
	 */

	/* Should open logfile and/or syslog based on arguments */
	if (logfile[0])
		snmp_enable_filelog(logfile, dont_zero_log);
	if (syslog_log)
		snmp_enable_syslog(); 
#ifdef BUFSIZ
	setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
#endif
    /* 
     * Initialize the world.  Detach from the shell.
     * Create initial user.
     */
#if HAVE_FORK
    if (!dont_fork && fork() != 0) {
      exit(0);
    }
#endif

#if HAVE_GETPID
    if (pid_file != NULL) {
      if ((PID = fopen(pid_file, "w")) == NULL) {
        snmp_log_perror("fopen");
        if (!ds_get_boolean(DS_APPLICATION_ID, DS_AGENT_NO_ROOT_ACCESS))
          exit(1);
      }
      else {
        fprintf(PID, "%d\n", (int)getpid());
        fclose(PID);
      }
    }
#endif

#else /* __ECOS environment: */
void snmpd( void *initfunc( void ) ) {
    int             ret;
    u_short         dest_port = SNMP_PORT;
#define stderr_log 1
#endif

    // ---------
    // En-bloc reinitialization of statics.
    running = 1;
    // ---------

    SOCK_STARTUP;
    init_agent("snmpd");		/* do what we need to do first. */
    init_mib_modules();
    

    /* start library */
    init_snmp("snmpd");

    ret = init_master_agent( dest_port,
                       snmp_check_packet,
                       snmp_check_parse );
	if( ret != 0 )
		Exit(1); /* Exit logs exit val for us */

#ifdef SIGTERM
    signal(SIGTERM, SnmpdShutDown);
#endif
#ifdef SIGINT
    signal(SIGINT, SnmpdShutDown);
#endif
#ifdef SIGHUP
    signal(SIGHUP, SnmpdReconfig);
#endif
#ifdef SIGUSR1
    signal(SIGUSR1, SnmpdDump);
#endif

    /* send coldstart trap via snmptrap(1) if possible */
    send_easy_trap (0, 0);
        
#if HAVE_UNISTD_H
	if (gid) {
		DEBUGMSGTL(("snmpd", "Changing gid to %d.\n", gid));
		if (setgid(gid)==-1) {
			snmp_log_perror("setgid failed");
        		if (!ds_get_boolean(DS_APPLICATION_ID, DS_AGENT_NO_ROOT_ACCESS))
			  exit(1);
		}
	}
	if (uid) {
		DEBUGMSGTL(("snmpd", "Changing uid to %d.\n", uid));
		if(setuid(uid)==-1) {
			snmp_log_perror("setuid failed");
        		if (!ds_get_boolean(DS_APPLICATION_ID, DS_AGENT_NO_ROOT_ACCESS))
			  exit(1);
		}
	}
#endif

	/* honor selection of standard error output */
	if (!stderr_log)
		snmp_disable_stderrlog();

	/* we're up, log our version number */
	snmp_log(LOG_INFO, "UCD-SNMP version %s\n", VersionInfo);

	memset(addrCache, 0, sizeof(addrCache));

        /*
         * Call initialization function if necessary
         */
	DEBUGMSGTL(("snmpd", "Calling initfunc().\n"));
        if ( initfunc )
            (initfunc)();

	/* 
	 * Forever monitor the dest_port for incoming PDUs.
	 */
	DEBUGMSGTL(("snmpd", "We're up.  Starting to process data.\n"));
	receive();
#include "mib_module_shutdown.h"
	DEBUGMSGTL(("snmpd", "sending shutdown trap\n"));
	SnmpTrapNodeDown();
	DEBUGMSGTL(("snmpd", "Bye...\n"));
	snmp_shutdown("snmpd");

}  /* End main() -- snmpd */

/*******************************************************************-o-******
 * receive
 *
 * Parameters:
 *      
 * Returns:
 *	0	On success.
 *	-1	System error.
 *
 * Infinite while-loop which monitors incoming messges for the agent.
 * Invoke the established message handlers for incoming messages on a per
 * port basis.  Handle timeouts.
 */
	static int
receive(void)
{
    int numfds;
    fd_set fdset;
    struct timeval	timeout, *tvp = &timeout;
    struct timeval	sched,   *svp = &sched,
			now,     *nvp = &now;
    int count, block;
#ifdef	USING_SMUX_MODULE
    int i, sd;
#endif	/* USING_SMUX_MODULE */


    /*
     * Set the 'sched'uled timeout to the current time + one TIMETICK.
     */
    gettimeofday(nvp, (struct timezone *) NULL);
    svp->tv_usec = nvp->tv_usec + TIMETICK;
    svp->tv_sec = nvp->tv_sec;
    
    while (svp->tv_usec >= ONE_SEC){
	svp->tv_usec -= ONE_SEC;
	svp->tv_sec++;
    }

    /*
     * Loop-forever: execute message handlers for sockets with data,
     * reset the 'sched'uler.
     */
    while (running) {
        if (reconfig) {
          reconfig = 0;
          snmp_log(LOG_INFO, "Reconfiguring daemon\n");
          update_config();
        }
	tvp =  &timeout;
	tvp->tv_sec = 0;
	tvp->tv_usec = TIMETICK;

	numfds = 0;
	FD_ZERO(&fdset);
        block = 0;
        snmp_select_info(&numfds, &fdset, tvp, &block);
        if (block == 1)
            tvp = NULL; /* block without timeout */

#ifdef	USING_SMUX_MODULE
		if (smux_listen_sd >= 0) {
			FD_SET(smux_listen_sd, &fdset);
			numfds = smux_listen_sd >= numfds ? smux_listen_sd + 1 : numfds;
			for (i = 0; i < sdlen; i++) {
				FD_SET(sdlist[i], &fdset);
				numfds = sdlist[i] >= numfds ? sdlist[i] + 1 : numfds;
			}
		}
#endif	/* USING_SMUX_MODULE */

	count = select(numfds, &fdset, 0, 0, tvp);

	if (count > 0){
	    snmp_read(&fdset);
	} else switch(count){
	    case 0:
                snmp_timeout();
                break;
	    case -1:
		if (errno == EINTR){
		    continue;
		} else {
                    snmp_log_perror("select");
		}
		return -1;
	    default:
		snmp_log(LOG_ERR, "select returned %d\n", count);
		return -1;
	}  /* endif -- count>0 */

#ifdef	USING_SMUX_MODULE
		/* handle the SMUX sd's */
		if (smux_listen_sd >= 0) {
			for (i = 0; i < sdlen; i++) {
				if (FD_ISSET(sdlist[i], &fdset)) {
					if (smux_process(sdlist[i]) < 0) {
						for (; i < (sdlen - 1); i++) {
							sdlist[i] = sdlist[i+1];
						}
						sdlen--;
					}
				}
			}
			/* new connection */
			if (FD_ISSET(smux_listen_sd, &fdset)) {
				if ((sd = smux_accept(smux_listen_sd)) >= 0) {
					sdlist[sdlen++] = sd;
				}
			}
		}
#endif	/* USING_SMUX_MODULE */



        /*
         * If the time 'now' is greater than the 'sched'uled time, then:
         *
         *    Check alarm and event timers.
         *    Reset the 'sched'uled time to current time + one TIMETICK.
         *    Age the cache network addresses (from whom messges have
         *        been received).
         */
        gettimeofday(nvp, (struct timezone *) NULL);

        if (nvp->tv_sec > svp->tv_sec
                || (nvp->tv_sec == svp->tv_sec && nvp->tv_usec > svp->tv_usec)){
            svp->tv_usec = nvp->tv_usec + TIMETICK;
            svp->tv_sec = nvp->tv_sec;

            while (svp->tv_usec >= ONE_SEC){
                svp->tv_usec -= ONE_SEC;
                svp->tv_sec++;
            }
            if (log_addresses && lastAddrAge++ > 600){

                lastAddrAge = 0;
                for(count = 0; count < ADDRCACHE; count++){
                    if (addrCache[count].status == OLD)
                        addrCache[count].status = UNUSED;
                    if (addrCache[count].status == USED)
                        addrCache[count].status = OLD;
                }
            }
        }  /* endif -- now>sched */

        /* run requested alarms */
        run_alarms();
        
    }  /* endwhile */

    snmp_log(LOG_INFO, "Received TERM or STOP signal...  shutting down...\n");
    return 0;

}  /* end receive() */




/*******************************************************************-o-******
 * snmp_check_packet
 *
 * Parameters:
 *	session, from
 *      
 * Returns:
 *	1	On success.
 *	0	On error.
 *
 * Handler for all incoming messages (a.k.a. packets) for the agent.  If using
 * the libwrap utility, log the connection and deny/allow the access. Print
 * output when appropriate, and increment the incoming counter.
 *
 */
int
snmp_check_packet(struct snmp_session *session,
  snmp_ipaddr from)
{
    struct sockaddr_in *fromIp = (struct sockaddr_in *)&from;

#ifdef USE_LIBWRAP
    const char *addr_string;
    /*
     * Log the message and/or dump the message.
     * Optionally cache the network address of the sender.
     */
    addr_string = inet_ntoa(fromIp->sin_addr);

    if(!addr_string) {
      addr_string = STRING_UNKNOWN;
    }
    if(hosts_ctl("snmpd", addr_string, addr_string, STRING_UNKNOWN)) {
      snmp_log(allow_severity, "Connection from %s\n", addr_string);
    } else {
      snmp_log(deny_severity, "Connection from %s REFUSED\n", addr_string);
      return(0);
    }
#endif	/* USE_LIBWRAP */

    snmp_increment_statistic(STAT_SNMPINPKTS);

    if (log_addresses || ds_get_boolean(DS_APPLICATION_ID, DS_AGENT_VERBOSE)){
	int count;
	
	for(count = 0; count < ADDRCACHE; count++){
	    if (addrCache[count].status > UNUSED /* used or old */
		&& fromIp->sin_addr.s_addr == addrCache[count].addr)
		break;
	}

	if (count >= ADDRCACHE ||
            ds_get_boolean(DS_APPLICATION_ID, DS_AGENT_VERBOSE)){
	    snmp_log(LOG_INFO, "Received SNMP packet(s) from %s\n",
                        inet_ntoa(fromIp->sin_addr));
	    for(count = 0; count < ADDRCACHE; count++){
		if (addrCache[count].status == UNUSED){
		    addrCache[count].addr = fromIp->sin_addr.s_addr;
		    addrCache[count].status = USED;
		    break;
		}
	    }
	} else {
	    addrCache[count].status = USED;
	}
    }

    return ( 1 );
}


int
snmp_check_parse( struct snmp_session *session,
    struct snmp_pdu     *pdu,
    int    result)
{
    if ( result == 0 ) {
        if ( ds_get_boolean(DS_APPLICATION_ID, DS_AGENT_VERBOSE) &&
             snmp_get_do_logging() ) {
	     char c_oid [SPRINT_MAX_LEN];
	     struct variable_list *var_ptr;
	    
	    switch (pdu->command) {
	    case SNMP_MSG_GET:
	    	snmp_log(LOG_DEBUG, "  GET message\n"); break;
	    case SNMP_MSG_GETNEXT:
	    	snmp_log(LOG_DEBUG, "  GETNEXT message\n"); break;
	    case SNMP_MSG_RESPONSE:
	    	snmp_log(LOG_DEBUG, "  RESPONSE message\n"); break;
	    case SNMP_MSG_SET:
	    	snmp_log(LOG_DEBUG, "  SET message\n"); break;
	    case SNMP_MSG_TRAP:
	    	snmp_log(LOG_DEBUG, "  TRAP message\n"); break;
	    case SNMP_MSG_GETBULK:
	    	snmp_log(LOG_DEBUG, "  GETBULK message, non-rep=%d, max_rep=%d\n",
			pdu->errstat, pdu->errindex); break;
	    case SNMP_MSG_INFORM:
	    	snmp_log(LOG_DEBUG, "  INFORM message\n"); break;
	    case SNMP_MSG_TRAP2:
	    	snmp_log(LOG_DEBUG, "  TRAP2 message\n"); break;
	    case SNMP_MSG_REPORT:
	    	snmp_log(LOG_DEBUG, "  REPORT message\n"); break;
	    }
	     
	    for ( var_ptr = pdu->variables ;
	        var_ptr != NULL ; var_ptr=var_ptr->next_variable )
	    {
                sprint_objid (c_oid, var_ptr->name, var_ptr->name_length);
                snmp_log(LOG_DEBUG, "    -- %s\n", c_oid);
	    }
	}
    	return 1;
    }
    return 0; /* XXX: does it matter what the return value is? */
}

/*******************************************************************-o-******
 * snmp_input
 *
 * Parameters:
 *	 op
 *	*session
 *	 requid
 *	*pdu
 *	*magic
 *      
 * Returns:
 *	1		On success	-OR-
 *	Passes through	Return from alarmGetResponse() when 
 *	  		  USING_V2PARTY_ALARM_MODULE is defined.
 *
 * Call-back function to manage responses to traps (informs) and alarms.
 * Not used by the agent to process other Response PDUs.
 */
int
snmp_input(int op,
	   struct snmp_session *session,
	   int reqid,
	   struct snmp_pdu *pdu,
	   void *magic)
{
    struct get_req_state *state = (struct get_req_state *)magic;
    
    if (op == RECEIVED_MESSAGE) {
	if (pdu->command == SNMP_MSG_GET) {
	    if (state->type == EVENT_GET_REQ) {
		/* this is just the ack to our inform pdu */
		return 1;
	    }
	}
    }
    else if (op == TIMED_OUT) {
	if (state->type == ALARM_GET_REQ) {
		/* Need a mechanism to replace obsolete SNMPv2p alarm */
	}
    }
    return 1;

}  /* end snmp_input() */
