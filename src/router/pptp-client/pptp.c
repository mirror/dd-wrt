/*
	an implementation of the PPTP protocol
	Copyright (C) 2000  Free Software Foundation

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	pptp.c ... client shell to launch call managers, data handlers, and
	the pppd from the command line.
*/

#include <sys/types.h>
#include <sys/socket.h>
#if defined(__FreeBSD__)
#include <libutil.h>
#elif defined(__NetBSD__) || defined(__OpenBSD__)
#include <util.h>
#elif defined(__APPLE__)
#include <util.h>
#elif defined (__SVR4) && defined (__sun)
#else
#include <pty.h>
#endif
#ifdef USER_PPP
#include <fcntl.h>
#endif
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/param.h>
#if defined(__APPLE__)
#include "getopt.h"
#else
#include <getopt.h>
#endif
#include <limits.h>
#include "config.h"
#include "pptp_callmgr.h"
#include "pptp_gre.h"
#include "version.h"
#if defined(__linux__)
#include <sys/prctl.h>
#else
#include "inststr.h"
#endif
#include "util.h"
#include "pptp_quirks.h"
#include "pqueue.h"
#include "pptp_options.h"
#include "pptp_compat.h"

#ifndef PPPD_BINARY
#define PPPD_BINARY "pppd"
#endif

int syncppp = 0;
int log_level = 1;
int disable_buffer = 0;
int test_type = 0;
int test_rate = 100;

struct in_addr get_ip_address(char *name);
int open_callmgr(struct in_addr inetaddr, char *phonenr, int argc,char **argv,char **envp, int pty_fd, int gre_fd);
void launch_callmgr(struct in_addr inetaddr, char *phonenr, int argc,char **argv,char **envp);
int get_call_id(int sock, pid_t gre, pid_t pppd, 
		 u_int16_t *call_id, u_int16_t *peer_call_id);
void launch_pppd(char *ttydev, int argc, char **argv);

/*** print usage and exit *****************************************************/
void usage(char *progname)
{
    printf(
            "%s\n"

"Copyright (C) 2000  Free Software Foundation\n\n"

"This program comes with ABSOLUTELY NO WARRANTY; for details see source.\n"
"This is free software, and you are welcome to redistribute it under certain\n"
"conditions; see source for details.\n\n"

            "Usage:\n"
            "  %s <hostname> [<pptp options>] [[--] <pppd options>]\n"
            "\n"
            "Or using pppd's pty option: \n"
            "  pppd pty \"%s <hostname> --nolaunchpppd <pptp options>\"\n"
            "\n"
            "Available pptp options:\n"
            "  --version        Display version number and exit\n"
            "  --phone <number>	Pass <number> to remote host as phone number\n"
            "  --nolaunchpppd	Do not launch pppd, for use as a pppd pty\n"
            "  --quirks <quirk>	Work around a buggy PPTP implementation\n"
            "			Currently recognised values are BEZEQ_ISRAEL only\n"
            "  --debug		Run in foreground (for debugging with gdb)\n"
            "  --sync		Enable Synchronous HDLC (pppd must use it too)\n"
            "  --timeout <secs>	Time to wait for reordered packets (0.01 to 10 secs)\n"
	    "  --nobuffer		Disable packet buffering and reordering completely\n"
	    "  --idle-wait		Time to wait before sending echo request\n"
            "  --max-echo-wait		Time to wait before giving up on lack of reply\n"
            "  --logstring <name>	Use <name> instead of 'anon' in syslog messages\n"
            "  --localbind <addr>	Bind to specified IP address instead of wildcard\n"
            "  --loglevel <level>	Sets the debugging level (0=low, 1=default, 2=high)\n"
            "  --test-type <type>	Damage the packet stream by reordering\n"
            "  --test-rate <n>		Do the test every n packets\n",

            version, progname, progname);
    log("%s called with wrong arguments, program not started.", progname);
    exit(1);
}

#if defined (__SVR4) && defined (__sun)
struct in_addr localbind = { INADDR_ANY };
#else
struct in_addr localbind = { INADDR_NONE };
#endif
static int signaled = 0;

/*** do nothing signal handler ************************************************/
void do_nothing(int sig)
{ 
    /* do nothing signal handler. Better than SIG_IGN. */
    signaled = sig;
}

sigjmp_buf env;

/*** signal handler ***********************************************************/
void sighandler(int sig)
{
    siglongjmp(env, 1);
}

/*** report statistics signal handler (SIGUSR1) *******************************/
void sigstats(int sig)
{
    syslog(LOG_NOTICE, "GRE statistics:\n");
#define LOG(name,value) syslog(LOG_NOTICE, name "\n", stats .value)
    LOG("rx accepted  = %d", rx_accepted);
    LOG("rx lost      = %d", rx_lost);
    LOG("rx under win = %d", rx_underwin);
    LOG("rx over  win = %d", rx_overwin);
    LOG("rx buffered  = %d", rx_buffered);
    LOG("rx OS errors = %d", rx_errors);
    LOG("rx truncated = %d", rx_truncated);
    LOG("rx invalid   = %d", rx_invalid);
    LOG("rx acks      = %d", rx_acks);
    LOG("tx sent      = %d", tx_sent);
    LOG("tx failed    = %d", tx_failed);
    LOG("tx short     = %d", tx_short);
    LOG("tx acks      = %d", tx_acks);
    LOG("tx oversize  = %d", tx_oversize);
    LOG("round trip   = %d usecs", rtt);
#undef LOG
}

/*** main *********************************************************************/
/* TODO: redesign to avoid longjmp/setjmp.  Several variables here
   have a volatile qualifier to silence warnings from gcc < 3.0.
   Remove the volatile qualifiers if longjmp/setjmp are removed.
   */
int main(int argc, char **argv, char **envp)
{
    struct in_addr inetaddr;
    volatile int callmgr_sock = -1;
    char ttydev[PATH_MAX];
    int pty_fd, tty_fd, gre_fd, rc;
    volatile pid_t parent_pid, child_pid;
    u_int16_t call_id, peer_call_id;
    char buf[128];
    int pppdargc;
    char **pppdargv;
    char phonenrbuf[65]; /* maximum length of field plus one for the trailing
                          * '\0' */
    char * volatile phonenr = NULL;
    volatile int launchpppd = 1, debug = 0;

    while(1){ 
        /* structure with all recognised options for pptp */
        static struct option long_options[] = {
            {"phone", 1, 0, 0},  
            {"nolaunchpppd", 0, 0, 0},  
            {"quirks", 1, 0, 0},
            {"debug", 0, 0, 0},
            {"sync", 0, 0, 0},
            {"timeout", 1, 0, 0},
            {"logstring", 1, 0, 0},
            {"localbind", 1, 0, 0},
            {"loglevel", 1, 0, 0},
	    {"nobuffer", 0, 0, 0},
	    {"idle-wait", 1, 0, 0},
	    {"max-echo-wait", 1, 0, 0},
	    {"version", 0, 0, 0},
	    {"test-type", 1, 0, 0},
	    {"test-rate", 1, 0, 0},
            {0, 0, 0, 0}
        };
        int option_index = 0;
        int c;
        c = getopt_long (argc, argv, "", long_options, &option_index);
        if (c == -1) break;  /* no more options */
        switch (c) {
            case 0: 
                if (option_index == 0) { /* --phone specified */
                    /* copy it to a buffer, as the argv's will be overwritten
                     * by inststr() */
                    strncpy(phonenrbuf,optarg,sizeof(phonenrbuf));
                    phonenrbuf[sizeof(phonenrbuf) - 1] = '\0';
                    phonenr = phonenrbuf;
                } else if (option_index == 1) {/* --nolaunchpppd specified */
                    launchpppd = 0;
                } else if (option_index == 2) {/* --quirks specified */
                    if (set_quirk_index(find_quirk(optarg)))
                        usage(argv[0]);
                } else if (option_index == 3) {/* --debug */
                    debug = 1;
                } else if (option_index == 4) {/* --sync specified */
                    syncppp = 1;
                } else if (option_index == 5) {/* --timeout */
                    float new_packet_timeout = atof(optarg);
                    if (new_packet_timeout < 0.0099 ||
                            new_packet_timeout > 10) {
                        printf( "Packet timeout %s (%f) out of range: "
                                "should be between 0.01 and 10 seconds\n",
                                optarg, new_packet_timeout);
                        log("Packet timeout %s (%f) out of range: should be"
                                "between 0.01 and 10 seconds", optarg,
                                new_packet_timeout);
                        exit(2);
                    } else {
                        packet_timeout_usecs = new_packet_timeout * 1000000;
                    }
                } else if (option_index == 6) {/* --logstring */
                    log_string = strdup(optarg);
                } else if (option_index == 7) {/* --localbind */ 
                    if (inet_pton(AF_INET, optarg, (void *) &localbind) < 1) {
                        printf( "Local bind address %s invalid\n", 
				optarg);
                        log("Local bind address %s invalid\n", optarg);
                        exit(2);
                    }
                } else if (option_index == 8) { /* --loglevel */
                    log_level = atoi(optarg);
                    if (log_level < 0 || log_level > 2)
                        usage(argv[0]);
                } else if (option_index == 9) { /* --nobuffer */
		    disable_buffer = 1;
                } else if (option_index == 10) { /* --idle-wait */
                    int x = atoi(optarg);
                    if (x < 0) {
                        printf( "--idle-wait must not be negative\n");
                        log("--idle-wait must not be negative\n");
                        exit(2);
                    } else {
                        idle_wait = x;
                    }
                } else if (option_index == 11) { /* --max-echo-wait */
                    int x = atoi(optarg);
                    if (x < 0) {
                        printf( "--max-echo-wait must not be negative\n");
                        log("--max-echo-wait must not be negative\n");
                        exit(2);
                    } else {
                        max_echo_wait = x;
                    }
		    printf( "--max-echo-wait ignored, not yet implemented\n");
                 } else if (option_index == 12) { /* --version */
 		    printf( "%s\n", version);
 		    exit(0);
 		} else if (option_index == 13) { /* --test-type */
 		    test_type = atoi(optarg);
 		} else if (option_index == 14) { /* --test-rate */
 		    test_rate = atoi(optarg);
                }
                break;
            case '?': /* unrecognised option */
                /* fall through */
            default:
		usage(argv[0]);
        }
        if (c == -1) break;  /* no more options for pptp */
    }

    /* at least one argument is required */
    if (argc <= optind)
        usage(argv[0]);

    /* Get IP address for the hostname in argv[1] */
    inetaddr = get_ip_address(argv[optind]);
    optind++;

    /* Find the ppp options, extract phone number */
    pppdargc = argc - optind;
    pppdargv = argv + optind;
    log("The synchronous pptp option is %sactivated\n", syncppp ? "" : "NOT ");

    /* Now we have the peer address, bind the GRE socket early,
       before starting pppd. This prevents the ICMP Unreachable bug
       documented in <1026868263.2855.67.camel@jander> */
    gre_fd = pptp_gre_bind(inetaddr);
    if (gre_fd < 0) {
        close(callmgr_sock);
        fatal("Cannot bind GRE socket, aborting.");
    }

    /* Find an open pty/tty pair. */
    if(launchpppd){
        rc = openpty (&pty_fd, &tty_fd, ttydev, NULL, NULL);
        if (rc < 0) { 
            close(callmgr_sock); 
            fatal("Could not find free pty.");
        }

        /* fork and wait. */
        signal(SIGUSR1, do_nothing); /* don't die */
        signal(SIGCHLD, do_nothing); /* don't ignore SIGCHLD */
        parent_pid = getpid();
        switch (child_pid = fork()) {
            case -1:
                fatal("Could not fork pppd process");
            case 0: /* I'm the child! */
                close (tty_fd);
                signal(SIGUSR1, SIG_DFL);
                child_pid = getpid();
                break;
            default: /* parent */
                close (pty_fd);
                /*
                 * There is still a very small race condition here.  If a signal
                 * occurs after signaled is checked but before pause is called,
                 * things will hang.
                 */
                if (!signaled) {
                    pause(); /* wait for the signal */
                }
 
                if (signaled == SIGCHLD)
                    fatal("Child process died");
 
                launch_pppd(ttydev, pppdargc, pppdargv); /* launch pppd */
                perror("Error");
                fatal("Could not launch pppd");
        }
    } else { /* ! launchpppd */
        pty_fd = tty_fd = STDIN_FILENO;
        /* close unused file descriptor, that is redirected to the pty */
        close(STDOUT_FILENO);
        child_pid = getpid();
        parent_pid = 0; /* don't kill pppd */
    }

    do {
        /*
         * Open connection to call manager (Launch call manager if necessary.)
         */
        callmgr_sock = open_callmgr(inetaddr, phonenr, argc, argv, envp,
		pty_fd, gre_fd);
        /* Exchange PIDs, get call ID */
    } while (get_call_id(callmgr_sock, parent_pid, child_pid, 
                &call_id, &peer_call_id) < 0);

    /* Send signal to wake up pppd task */
    if (launchpppd) {
        kill(parent_pid, SIGUSR1);
        sleep(2);
        /* become a daemon */
        if (!debug && daemon(0, 0) != 0) {
            perror("daemon");
        }
    } else {
        /* re-open stderr as /dev/null to release it */
        file2fd("/dev/null", "wb", STDERR_FILENO);
    }

    char *tty_name = ttyname(tty_fd);
    snprintf(buf, sizeof(buf), "pptp: GRE-to-PPP gateway on %s",
              tty_name ? tty_name : "(null)");
#ifdef PR_SET_NAME
    rc = prctl(PR_SET_NAME, "pptpgw", 0, 0, 0);
    if (rc != 0) perror("prctl");
#else
    inststr(argc, argv, envp, buf);
#endif
    if (sigsetjmp(env, 1)!= 0) goto shutdown;

    signal(SIGINT,  sighandler);
    signal(SIGTERM, sighandler);
    signal(SIGKILL, sighandler);
    signal(SIGCHLD, sighandler);
    signal(SIGUSR1, sigstats);

    /* Do GRE copy until close. */
    pptp_gre_copy(call_id, peer_call_id, pty_fd, gre_fd);

shutdown:
    /* on close, kill all. */
    if(launchpppd)
        kill(parent_pid, SIGTERM);
    close(pty_fd);
    close(callmgr_sock);
    exit(0);
}

/*** get the ipaddress coming from the command line ***************************/
struct in_addr get_ip_address(char *name)
{
    struct in_addr retval;
    struct hostent *host = gethostbyname(name);
    if (host == NULL) {
        if (h_errno == HOST_NOT_FOUND)
            fatal("gethostbyname '%s': HOST NOT FOUND", name);
        else if (h_errno == NO_ADDRESS)
            fatal("gethostbyname '%s': NO IP ADDRESS", name);
        else
            fatal("gethostbyname '%s': name server error", name);
    }
    if (host->h_addrtype != AF_INET)
        fatal("Host '%s' has non-internet address", name);
    memcpy(&retval.s_addr, host->h_addr, sizeof(retval.s_addr));
    return retval;
}

/*** start the call manager ***************************************************/
int open_callmgr(struct in_addr inetaddr, char *phonenr, int argc, char **argv,
        char **envp, int pty_fd, int gre_fd)
{
    /* Try to open unix domain socket to call manager. */
    struct sockaddr_un where;
    const int NUM_TRIES = 3;
    int i, fd;
    pid_t pid;
    int status;
    /* Open socket */
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        fatal("Could not create unix domain socket: %s", strerror(errno));
    }
    /* Make address */
    callmgr_name_unixsock(&where, inetaddr, localbind);
    for (i = 0; i < NUM_TRIES; i++) {
        if (connect(fd, (struct sockaddr *) &where, sizeof(where)) < 0) {
            /* couldn't connect.  We'll have to launch this guy. */

            unlink (where.sun_path);	

            /* fork and launch call manager process */
            switch (pid = fork()) {
                case -1: /* failure */
                    fatal("fork() to launch call manager failed.");
                case 0: /* child */
                {
                    close (fd);
                    /* close the pty and gre in the call manager */
                    close(pty_fd);
                    close(gre_fd);
                    launch_callmgr(inetaddr, phonenr, argc, argv, envp);
                }
                default: /* parent */
                    waitpid(pid, &status, 0);
                    if (status!= 0)
                        fatal("Call manager exited with error %d", status);
                    break;
            }
            sleep(1);
        }
        else return fd;
    }
    close(fd);
    fatal("Could not launch call manager after %d tries.", i);
    return -1;   /* make gcc happy */
}

/*** call the call manager main ***********************************************/
void launch_callmgr(struct in_addr inetaddr, char *phonenr, int argc,
        char**argv,char**envp) 
{
      char *my_argv[3] = { argv[0], inet_ntoa(inetaddr), phonenr };
      char buf[128];
      snprintf(buf, sizeof(buf), "pptp: call manager for %s", my_argv[1]);
#ifdef PR_SET_NAME
      int rc;
      rc = prctl(PR_SET_NAME, "pptpcm", 0, 0, 0);
      if (rc != 0) perror("prctl");
#else
      inststr(argc, argv, envp, buf);
#endif
      exit(callmgr_main(3, my_argv, envp));
}

/*** exchange data with the call manager  *************************************/
/* XXX need better error checking XXX */
int get_call_id(int sock, pid_t gre, pid_t pppd, 
		 u_int16_t *call_id, u_int16_t *peer_call_id)
{
    u_int16_t m_call_id, m_peer_call_id;
    /* write pid's to socket */
    /* don't bother with network byte order, because pid's are meaningless
     * outside the local host.
     */
    int rc;
    rc = write(sock, &gre, sizeof(gre));
    if (rc != sizeof(gre))
        return -1;
    rc = write(sock, &pppd, sizeof(pppd));
    if (rc != sizeof(pppd))
        return -1;
    rc = read(sock,  &m_call_id, sizeof(m_call_id));
    if (rc != sizeof(m_call_id))
        return -1;
    rc = read(sock,  &m_peer_call_id, sizeof(m_peer_call_id));
    if (rc != sizeof(m_peer_call_id))
        return -1;
    /*
     * XXX FIXME ... DO ERROR CHECKING & TIME-OUTS XXX
     * (Rhialto: I am assuming for now that timeouts are not relevant
     * here, because the read and write calls would return -1 (fail) when
     * the peer goes away during the process. We know it is (or was)
     * running because the connect() call succeeded.)
     * (James: on the other hand, if the route to the peer goes away, we
     * wouldn't get told by read() or write() for quite some time.)
     */
    *call_id = m_call_id;
    *peer_call_id = m_peer_call_id;
    return 0;
}

/*** execvp pppd **************************************************************/
void launch_pppd(char *ttydev, int argc, char **argv)
{
    char *new_argv[argc + 4];/* XXX if not using GCC, hard code a limit here. */
    int i = 0, j;
    new_argv[i++] = PPPD_BINARY;
#ifdef USER_PPP
    new_argv[i++] = "-direct";
    /* ppp expects to have stdin connected to ttydev */
    if ((j = open(ttydev, O_RDWR)) == -1)
        fatal("Cannot open %s: %s", ttydev, strerror(errno));
    if (dup2(j, 0) == -1)
        fatal("dup2 failed: %s", strerror(errno));
    close(j);
#else
    new_argv[i++] = ttydev;
    new_argv[i++] = "38400";
#endif
    for (j = 0; j < argc; j++)
        new_argv[i++] = argv[j];
    new_argv[i] = NULL;
    execvp(new_argv[0], new_argv);
}
