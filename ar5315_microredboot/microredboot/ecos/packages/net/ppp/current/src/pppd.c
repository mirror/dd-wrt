//==========================================================================
//
//      src/pppd.c
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Portions created by Nick Garnett are
// Copyright (C) 2003 eCosCentric Ltd.
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
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD, 
// FreeBSD or other sources, and are covered by the appropriate
// copyright disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================

/*
 * main.c - Point-to-Point Protocol main module
 *
 * Copyright (c) 1989 Carnegie Mellon University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by Carnegie Mellon University.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
//static char rcsid[] = "$FreeBSD: src/usr.sbin/pppd/main.c,v 1.19.6.1 2002/07/30 19:17:27 nectar Exp $";
#endif

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <cyg/ppp/syslog.h>
#include <sys/param.h>
#include <netdb.h>
#include <sys/types.h>
#ifndef __ECOS
#include <sys/wait.h>
#endif
#include <sys/time.h>
#ifndef __ECOS
#include <sys/resource.h>
#endif
#include <sys/stat.h>
#include <sys/socket.h>

#define MAXPATHLEN PATH_MAX

#include "cyg/ppp/pppd.h"
#include "cyg/ppp/magic.h"
#include "cyg/ppp/fsm.h"
#include "cyg/ppp/lcp.h"
#include "cyg/ppp/ipcp.h"
#include "cyg/ppp/upap.h"
#include "cyg/ppp/chap.h"
#include "cyg/ppp/ccp.h"
#include "cyg/ppp/ppp_io.h"

#ifdef CBCP_SUPPORT
#include "cbcp.h"
#endif

#if defined(SUNOS4)
extern char *strerror();
#endif

#ifdef IPX_CHANGE
#include "ipxcp.h"
#endif /* IPX_CHANGE */
#ifdef AT_CHANGE
#include "atcp.h"
#endif

/* options */

#define option_error(msg) db_printf("Option error: %s\n", msg )


/*
 * Variables set by command-line options.
 */

int	debug = 1;		/* Debug flag */
int	kdebugflag = 1;	        /* Tell kernel to print debug messages */
int	default_device = 0;	/* Using /dev/tty or equivalent */
char	devnam[MAXPATHLEN] = "/dev/ser0";	/* Device name */
int     flowctl = CYG_PPP_FLOWCTL_HARDWARE; /* flow control */
int	modem  = 0;		/* Use modem control lines */
cyg_serial_baud_rate_t	inspeed = 0;    	/* Input/Output speed requested */
u_int32_t netmask = 0;	        /* IP netmask to set on interface */
int	lockflag = 0;	        /* Create lock file to lock the serial dev */
char	*connector = NULL;	/* Script to establish physical link */
char	*disconnector = NULL;	/* Script to disestablish physical link */
char	*welcomer = NULL;	/* Script to welcome client after connection */
int	max_con_attempts = 10;  /* Maximum number of times to try dialing */
int	maxconnect = 0;	        /* Maximum connect time (seconds) */
char	user[MAXNAMELEN] = "nickg";	/* Our name for authenticating ourselves */
char	passwd[MAXSECRETLEN] = "xsecretx";	/* Password for PAP */
int	auth_required = 0;	/* Peer is required to authenticate */
int	proxyarp = 0;	        /* Set up proxy ARP entry for peer */
int	persist = 0;	        /* Reopen link after it goes down */
int	uselogin = 0;	        /* Use /etc/passwd for checking PAP */
int	lcp_echo_interval = 60; /* Interval between LCP echo-requests */
int	lcp_echo_fails = 4;	/* Tolerance to unanswered echo-requests */
char	our_name[20];	        /* Our name for authentication purposes */
char	remote_name[20];	/* Peer's name for authentication */
int	explicit_remote = 1;    /* remote_name specified with remotename opt */
int	usehostname = 0;	/* Use hostname for our_name */
int	disable_defaultip = 0;  /* Don't use hostname for default IP adrs */
//int	demand = 0;		/* Do dial-on-demand */
char	*ipparam = NULL;	/* Extra parameter for ip up/down scripts */
int	cryptpap = 0;	        /* Others' PAP passwords are encrypted */
int	idle_time_limit = 60;   /* Shut down link if idle for this long */
int	holdoff = 0;	        /* Dead time before restarting */
int	refuse_pap = 0;	        /* Don't wanna auth. ourselves with PAP */
int	refuse_chap = 0;	/* Don't wanna auth. ourselves with CHAP */

const char    **script;               /* Chat connection script */


/* interface vars */
char ifname[32];		/* Interface name */
static int ifunit;		/* Interface unit number */

char *progname;			/* Name of this program */
char cyg_ppp_hostname[MAXNAMELEN];	/* Our hostname */
time_t		etime,stime;	/* End and Start time */
int		minutes;	/* connection duration */

cyg_io_handle_t tty_handle;     /* IO handle on serial stream */
mode_t tty_mode = -1;		/* Original access permissions to tty */
int baud_rate;			/* Actual bits/second for serial device */
int hungup;			/* terminal has been hung up */
int privileged;			/* we're running as real uid root */
int need_holdoff;		/* need holdoff period before restarting */

int phase;                      /* where the link is at */
int kill_link;
int open_ccp_flag;

char **script_env;		/* Env. variable values for scripts */
int s_env_nalloc;		/* # words avail at script_env */

u_char outpacket_buf[PPP_MRU+PPP_HDRLEN]; /* buffer for outgoing packet */
u_char inpacket_buf[PPP_MRU+PPP_HDRLEN]; /* buffer for incoming packet */

char *no_ppp_msg = "Sorry - this system lacks PPP kernel support\n";
cyg_ppp_stats_t cyg_ppp_stats;

/* Prototypes for procedures local to this file. */

static void cleanup __P((void));
static void close_tty __P((void));
static void get_input __P((void));
static void calltimeout __P((void));
static struct timeval *timeleft __P((struct timeval *));
//static void holdoff_end __P((void *));
static void pr_log __P((void *, char *, ...));

extern	char	*ttyname __P((int));
extern	char	*getlogin __P((void));
int main __P((int, char *[]));

#ifdef ultrix
#undef	O_NONBLOCK
#define	O_NONBLOCK	O_NDELAY
#endif

#ifdef ULTRIX
#define setlogmask(x)
#endif

/*
 * PPP Data Link Layer "protocol" table.
 * One entry per supported protocol.
 * The last entry must be NULL.
 */
struct protent *protocols[] = {
    &lcp_protent,
#ifdef CYGPKG_PPP_PAP
    &pap_protent,
#endif
#ifdef CYGPKG_PPP_CHAP    
    &chap_protent,
#endif
#ifdef CBCP_SUPPORT
    &cbcp_protent,
#endif
    &ipcp_protent,
    &ccp_protent,
#ifdef IPX_CHANGE
    &ipxcp_protent,
#endif
#ifdef AT_CHANGE
    &atcp_protent,
#endif
    NULL
};


externC void
cyg_pppd_main(CYG_ADDRWORD arg)
{
    int i;
    struct timeval timo;
    struct protent *protp;
    int connect_attempts = 0;

    phase = PHASE_INITIALIZE;

    for (i = 0; (protp = protocols[i]) != NULL; ++i)
        (*protp->init)(0);

    cyg_ppp_options_install( ((struct tty *)arg)->options );
    
    if (!ppp_available()) {
	option_error(no_ppp_msg);
	exit(1);
    }
    
    /*
     * Initialize system-dependent stuff and magic number package.
     */
    sys_init();
    magic_init();
    if (debug)
	setlogmask(LOG_UPTO(LOG_DEBUG));

   
    for (;;) {

	need_holdoff = 1;

        {
            Cyg_ErrNo err;
            while ((err = cyg_io_lookup(devnam, &tty_handle)) < 0) {
                if (err != 0)
                    syslog(LOG_ERR, "Failed to open %s: %d", devnam,err);
            }

#ifdef CYGOPT_IO_SERIAL_SUPPORT_LINE_STATUS
            if( modem )
            {
                cyg_uint32 len = sizeof(ppp_tty.serial_callbacks);
                ppp_tty.serial_callbacks.fn = cyg_ppp_serial_callback;
                ppp_tty.serial_callbacks.priv = (CYG_ADDRWORD)&ppp_tty;

                
                err = cyg_io_set_config( tty_handle,
                                         CYG_IO_SET_CONFIG_SERIAL_STATUS_CALLBACK,
                                         &ppp_tty.serial_callbacks,
                                         &len);

                if( err != 0 ) {
                    syslog(LOG_ERR, "cyg_io_set_config(serial callbacks): %d",err);
                    die(1);
                }

            }
#endif
        }

	hungup = 0;
	kill_link = 0;

	/* set line speed, flow control, etc.; clear CLOCAL if modem option */
	set_up_tty(tty_handle, 0);

        if( script != NULL )
        {
            if( !cyg_ppp_chat( devnam, script ) )
            {
                connect_attempts++;
                goto fail;
            }
        }

#ifdef CYGOPT_IO_SERIAL_SUPPORT_LINE_STATUS
        if( modem )
        {
            while( !ppp_tty.carrier_detected )
                cyg_thread_delay(100);
        }
#endif
        
	connect_attempts = 0;	/* we made it through ok */

	/* set up the serial device as a ppp interface */
	establish_ppp(tty_handle);

        syslog(LOG_INFO, "Using interface ppp%d", ifunit);
        (void) sprintf(ifname, "ppp%d", ifunit);
        
	/*
	 * Start opening the connection and wait for
	 * incoming events (reply, timeout, etc.).
	 */
	syslog(LOG_NOTICE, "Connect: %s <--> %s", ifname, devnam);
	stime = time((time_t *) NULL);
        
	lcp_lowerup(0);
	lcp_open(0);		/* Start protocol */
	for (phase = PHASE_ESTABLISH; phase != PHASE_DEAD; ) {
	    wait_input(timeleft(&timo));
	    calltimeout();
	    get_input();
	    if (kill_link) {
		lcp_close(0, "User request");
		kill_link = 0;
	    }
	    if (open_ccp_flag) {
		if (phase == PHASE_NETWORK) {
		    ccp_fsm[0].flags = OPT_RESTART; /* clears OPT_SILENT */
		    (*ccp_protent.open)(0);
		}
		open_ccp_flag = 0;
	    }
	}

	clean_check();
	disestablish_ppp(tty_handle);

    fail:        
	if (tty_handle != 0)
	    close_tty();
        
	/* limit to retries? */
	if (max_con_attempts)
	    if (connect_attempts >= max_con_attempts)
		break;

	if (!persist)
	    die(1);

#if 0
	if (holdoff > 0 && need_holdoff) {
	    phase = PHASE_HOLDOFF;
	    TIMEOUT(holdoff_end, NULL, holdoff);
	    do {
		wait_time(timeleft(&timo));
		calltimeout();
		if (kill_link) {
		    if (!persist)
			die(0);
		    kill_link = 0;
		    phase = PHASE_DORMANT; /* allow signal to end holdoff */
		}

	    } while (phase == PHASE_HOLDOFF);
	}
#endif
        
    }

    die(0);
}

/*
 * get_input - called when incoming data is available.
 */
static void
get_input()
{
    int len, i;
    u_char *p;
    u_short protocol;
    struct protent *protp;

    p = inpacket_buf;	/* point to beginning of packet buffer */

    len = read_packet(inpacket_buf);

    if (len < 0)
	return;

    if (len == 0) {
	etime = time((time_t *) NULL);
	minutes = (etime-stime)/60;
	syslog(LOG_NOTICE, "Modem hangup, connected for %d minutes", (minutes >1) ? minutes : 1);
	hungup = 1;
	lcp_lowerdown(0);	/* serial link is no longer available */
	link_terminated(0);
	return;
    }

    if (debug /*&& (debugflags & DBG_INPACKET)*/)
	log_packet(p, len, "rcvd ", LOG_DEBUG);

    if (len < PPP_HDRLEN) {
	MAINDEBUG((LOG_INFO, "io(): Received short packet."));
	return;
    }

    p += 2;				/* Skip address and control */
    GETSHORT(protocol, p);
    len -= PPP_HDRLEN;

    /*
     * Toss all non-LCP packets unless LCP is OPEN.
     */
    if (protocol != PPP_LCP && lcp_fsm[0].state != OPENED) {
	MAINDEBUG((LOG_INFO,
		   "get_input: Received non-LCP packet when LCP not open."));
	return;
    }

    /*
     * Until we get past the authentication phase, toss all packets
     * except LCP, LQR and authentication packets.
     */
    if (phase <= PHASE_AUTHENTICATE
	&& !(protocol == PPP_LCP || protocol == PPP_LQR
	     || protocol == PPP_PAP || protocol == PPP_CHAP)) {
	MAINDEBUG((LOG_INFO, "get_input: discarding proto 0x%x in phase %d",
		   protocol, phase));
	return;
    }

    /*
     * Upcall the proper protocol input routine.
     */
    for (i = 0; (protp = protocols[i]) != NULL; ++i) {
	if (protp->protocol == protocol && protp->enabled_flag) {
	    (*protp->input)(0, p, len);
	    return;
	}
        if (protocol == (protp->protocol & ~0x8000) && protp->enabled_flag
	    && protp->datainput != NULL) {
	    (*protp->datainput)(0, p, len);
	    return;
	}
    }

    if (debug)
    	syslog(LOG_WARNING, "Unsupported protocol (0x%x) received", protocol);
    lcp_sprotrej(0, p - PPP_HDRLEN, len + PPP_HDRLEN);
}


/*
 * quit - Clean up state and exit (with an error indication).
 */
void 
quit()
{
    die(1);
}

/*
 * die - like quit, except we can specify an exit status.
 */
void
die(status)
    int status;
{
db_printf("%s called\n", __PRETTY_FUNCTION__);    
#ifndef __ECOS
    cleanup();
    syslog(LOG_INFO, "Exit.");
    exit(status);
#else
    externC void sys_exit(void);
    cleanup();    
    sys_exit();
#endif
}

/*
 * cleanup - restore anything which needs to be restored before we exit
 */
/* ARGSUSED */
static void
cleanup()
{
db_printf("%s called\n", __PRETTY_FUNCTION__);    
    sys_cleanup();

    if (tty_handle != 0)
	close_tty();
}

/*
 * close_tty - restore the terminal device and close it.
 */
static void
close_tty()
{
    disestablish_ppp(tty_handle);

#ifndef __ECOS
    /* drop dtr to hang up */
    if (modem) {
	setdtr(ttyfd, FALSE);
	/*
	 * This sleep is in case the serial port has CLOCAL set by default,
	 * and consequently will reassert DTR when we close the device.
	 */
	sleep(1);
    }
#endif
    
    db_printf("%s called\n", __PRETTY_FUNCTION__);
    
    restore_tty(tty_handle);

    if( ppp_tty.t_handle != 0 )
    {
        cyg_ppp_pppclose( &ppp_tty, 0 );
        ppp_tty.t_handle = 0;
    }

    tty_handle = 0;
}


struct	callout {
    struct timeval	c_time;		/* time at which to call routine */
    void		*c_arg;		/* argument to routine */
    void		(*c_func) __P((void *)); /* routine */
    struct		callout *c_next;
};

static struct callout *callout = NULL;	/* Callout list */
static struct timeval timenow;		/* Current time */

/*
 * timeout - Schedule a timeout.
 *
 * Note that this timeout takes the number of seconds, NOT hz (as in
 * the kernel).
 */
void
cyg_ppp_timeout(void (*func) __P((void *)),
                void *arg,
                int time)
{
    struct callout *newp, *p, **pp;
  
    MAINDEBUG((LOG_DEBUG, "Timeout %lx:%lx in %d seconds.",
	       (long) func, (long) arg, time));
    
    /*
     * Allocate timeout.
     */
    if ((newp = (struct callout *) malloc(sizeof(struct callout))) == NULL) {
	syslog(LOG_ERR, "Out of memory in timeout()!");
	die(1);
    }
    newp->c_arg = arg;
    newp->c_func = func;
    gettimeofday(&timenow, NULL);
    newp->c_time.tv_sec = timenow.tv_sec + time;
    newp->c_time.tv_usec = timenow.tv_usec;
  
    /*
     * Find correct place and link it in.
     */
    for (pp = &callout; (p = *pp); pp = &p->c_next)
	if (newp->c_time.tv_sec < p->c_time.tv_sec
	    || (newp->c_time.tv_sec == p->c_time.tv_sec
		&& newp->c_time.tv_usec < p->c_time.tv_sec))
	    break;
    newp->c_next = p;
    *pp = newp;
}


/*
 * untimeout - Unschedule a timeout.
 */
void
cyg_ppp_untimeout(void (*func) __P((void *)),
                  void *arg)
{
    struct callout **copp, *freep;
  
    MAINDEBUG((LOG_DEBUG, "Untimeout %lx:%lx.", (long) func, (long) arg));
  
    /*
     * Find first matching timeout and remove it from the list.
     */
    for (copp = &callout; (freep = *copp); copp = &freep->c_next)
	if (freep->c_func == func && freep->c_arg == arg) {
	    *copp = freep->c_next;
	    (void) free((char *) freep);
	    break;
	}
}


/*
 * calltimeout - Call any timeout routines which are now due.
 */
static void
calltimeout()
{
    struct callout *p;

    while (callout != NULL) {
	p = callout;

	if (gettimeofday(&timenow, NULL) < 0) {
	    syslog(LOG_ERR, "Failed to get time of day: %m");
	    die(1);
	}
	if (!(p->c_time.tv_sec < timenow.tv_sec
	      || (p->c_time.tv_sec == timenow.tv_sec
		  && p->c_time.tv_usec <= timenow.tv_usec)))
	    break;		/* no, it's not time yet */
	callout = p->c_next;
	(*p->c_func)(p->c_arg);
	free((char *) p);
    }
}


/*
 * timeleft - return the length of time until the next timeout is due.
 */
static struct timeval *
timeleft(tvp)
    struct timeval *tvp;
{
    if (callout == NULL)
	return NULL;

    gettimeofday(&timenow, NULL);
    tvp->tv_sec = callout->c_time.tv_sec - timenow.tv_sec;
    tvp->tv_usec = callout->c_time.tv_usec - timenow.tv_usec;
    if (tvp->tv_usec < 0) {
	tvp->tv_usec += 1000000;
	tvp->tv_sec -= 1;
    }
    if (tvp->tv_sec < 0)
	tvp->tv_sec = tvp->tv_usec = 0;

    return tvp;
}

/*
 * log_packet - format a packet and log it.
 */

char line[256];			/* line to be logged accumulated here */
char *linep;

void
log_packet(p, len, prefix, level)
    u_char *p;
    int len;
    char *prefix;
    int level;
{
    strcpy(line, prefix);
    linep = line + strlen(line);
    format_packet(p, len, pr_log, NULL);
    if (linep != line)
	syslog(level, "%s", line);
}

/*
 * format_packet - make a readable representation of a packet,
 * calling `printer(arg, format, ...)' to output it.
 */
void
format_packet(p, len, printer, arg)
    u_char *p;
    int len;
    void (*printer) __P((void *, char *, ...));
    void *arg;
{
    int i, n;
    u_short proto;
    u_char x;
    struct protent *protp;

    if (len >= PPP_HDRLEN && p[0] == PPP_ALLSTATIONS && p[1] == PPP_UI) {
	p += 2;
	GETSHORT(proto, p);
	len -= PPP_HDRLEN;
	for (i = 0; (protp = protocols[i]) != NULL; ++i)
	    if (proto == protp->protocol)
		break;
	if (protp != NULL) {
	    printer(arg, "[%s", protp->name);
	    n = (*protp->printpkt)(p, len, printer, arg);
	    printer(arg, "]");
	    p += n;
	    len -= n;
	} else {
	    printer(arg, "[proto=0x%x]", proto);
	}
    }

    for (; len > 0; --len) {
	GETCHAR(x, p);
	printer(arg, " %.2x", x);
    }
}

static void
pr_log __V((void *arg, char *fmt, ...))
{
    int n;
    va_list pvar;
    char buf[256];

#if __STDC__
    va_start(pvar, fmt);
#else
    void *arg;
    char *fmt;
    va_start(pvar);
    arg = va_arg(pvar, void *);
    fmt = va_arg(pvar, char *);
#endif

    n = vfmtmsg(buf, sizeof(buf), fmt, pvar);
    va_end(pvar);

    if (linep + n + 1 > line + sizeof(line)) {
	syslog(LOG_DEBUG, "%s", line);
	linep = line;
    }
    strcpy(linep, buf);
    linep += n;
}

/*
 * print_string - print a readable representation of a string using
 * printer.
 */
void
print_string(p, len, printer, arg)
    char *p;
    int len;
    void (*printer) __P((void *, char *, ...));
    void *arg;
{
    int c;

    printer(arg, "\"");
    for (; len > 0; --len) {
	c = *p++;
	if (' ' <= c && c <= '~') {
	    if (c == '\\' || c == '"')
		printer(arg, "\\");
	    printer(arg, "%c", c);
	} else {
	    switch (c) {
	    case '\n':
		printer(arg, "\\n");
		break;
	    case '\r':
		printer(arg, "\\r");
		break;
	    case '\t':
		printer(arg, "\\t");
		break;
	    default:
		printer(arg, "\\%.3o", c);
	    }
	}
    }
    printer(arg, "\"");
}

/*
 * novm - log an error message saying we ran out of memory, and die.
 */
void
novm(msg)
    char *msg;
{
    syslog(LOG_ERR, "Virtual memory exhausted allocating %s\n", msg);
    die(1);
}

/*
 * fmtmsg - format a message into a buffer.  Like sprintf except we
 * also specify the length of the output buffer, and we handle
 * %r (recursive format), %m (error message) and %I (IP address) formats.
 * Doesn't do floating-point formats.
 * Returns the number of chars put into buf.
 */
int
fmtmsg __V((char *buf, int buflen, char *fmt, ...))
{
    va_list args;
    int n;

#if __STDC__
    va_start(args, fmt);
#else
    char *buf;
    int buflen;
    char *fmt;
    va_start(args);
    buf = va_arg(args, char *);
    buflen = va_arg(args, int);
    fmt = va_arg(args, char *);
#endif
    n = vfmtmsg(buf, buflen, fmt, args);
    va_end(args);
    return n;
}

/*
 * vfmtmsg - like fmtmsg, takes a va_list instead of a list of args.
 */
#define OUTCHAR(c)	(buflen > 0? (--buflen, *buf++ = (c)): 0)

int
vfmtmsg(buf, buflen, fmt, args)
    char *buf;
    int buflen;
    char *fmt;
    va_list args;
{
    int c, i, n;
    int width, prec, fillch;
    int base, len, neg, quoted;
    unsigned long val = 0;
    char *str, *f, *buf0;
    unsigned char *p;
    char num[32];
    time_t t;
    static char hexchars[] = "0123456789abcdef";

    buf0 = buf;
    --buflen;
    while (buflen > 0) {
	for (f = fmt; *f != '%' && *f != 0; ++f)
	    ;
	if (f > fmt) {
	    len = f - fmt;
	    if (len > buflen)
		len = buflen;
	    memcpy(buf, fmt, len);
	    buf += len;
	    buflen -= len;
	    fmt = f;
	}
	if (*fmt == 0)
	    break;
	c = *++fmt;
	width = prec = 0;
	fillch = ' ';
	if (c == '0') {
	    fillch = '0';
	    c = *++fmt;
	}
	if (c == '*') {
	    width = va_arg(args, int);
	    c = *++fmt;
	} else {
	    while (isdigit(c)) {
		width = width * 10 + c - '0';
		c = *++fmt;
	    }
	}
	if (c == '.') {
	    c = *++fmt;
	    if (c == '*') {
		prec = va_arg(args, int);
		c = *++fmt;
	    } else {
		while (isdigit(c)) {
		    prec = prec * 10 + c - '0';
		    c = *++fmt;
		}
	    }
	}
	str = 0;
	base = 0;
	neg = 0;
	++fmt;
	switch (c) {
	case 'd':
	    i = va_arg(args, int);
	    if (i < 0) {
		neg = 1;
		val = -i;
	    } else
		val = i;
	    base = 10;
	    break;
	case 'o':
	    val = va_arg(args, unsigned int);
	    base = 8;
	    break;
	case 'x':
	    val = va_arg(args, unsigned int);
	    base = 16;
	    break;
	case 'p':
	    val = (unsigned long) va_arg(args, void *);
	    base = 16;
	    neg = 2;
	    break;
	case 's':
	    str = va_arg(args, char *);
	    break;
	case 'c':
	    num[0] = va_arg(args, int);
	    num[1] = 0;
	    str = num;
	    break;
	case 'm':
	    str = strerror(errno);
	    break;
	case 'I':
	    str = ip_ntoa(va_arg(args, u_int32_t));
	    break;
	case 'r':
	    f = va_arg(args, char *);
//#ifndef __powerpc__
//	    n = vfmtmsg(buf, buflen + 1, f, va_arg(args, va_list));
//#else
	    /* On the powerpc, a va_list is an array of 1 structure */
	    n = vfmtmsg(buf, buflen + 1, f, va_arg(args, void *));
//#endif
	    buf += n;
	    buflen -= n;
	    continue;
	case 't':
	    time(&t);
	    str = ctime(&t);
	    str += 4;		/* chop off the day name */
	    str[15] = 0;	/* chop off year and newline */
	    break;
	case 'v':		/* "visible" string */
	case 'q':		/* quoted string */
	    quoted = c == 'q';
	    p = va_arg(args, unsigned char *);
	    if (fillch == '0' && prec > 0) {
		n = prec;
	    } else {
		n = strlen((char *)p);
		if (prec > 0 && prec < n)
		    n = prec;
	    }
	    while (n > 0 && buflen > 0) {
		c = *p++;
		--n;
		if (!quoted && c >= 0x80) {
		    OUTCHAR('M');
		    OUTCHAR('-');
		    c -= 0x80;
		}
		if (quoted && (c == '"' || c == '\\'))
		    OUTCHAR('\\');
		if (c < 0x20 || (0x7f <= c && c < 0xa0)) {
		    if (quoted) {
			OUTCHAR('\\');
			switch (c) {
			case '\t':	OUTCHAR('t');	break;
			case '\n':	OUTCHAR('n');	break;
			case '\b':	OUTCHAR('b');	break;
			case '\f':	OUTCHAR('f');	break;
			default:
			    OUTCHAR('x');
			    OUTCHAR(hexchars[c >> 4]);
			    OUTCHAR(hexchars[c & 0xf]);
			}
		    } else {
			if (c == '\t')
			    OUTCHAR(c);
			else {
			    OUTCHAR('^');
			    OUTCHAR(c ^ 0x40);
			}
		    }
		} else
		    OUTCHAR(c);
	    }
	    continue;
	default:
	    *buf++ = '%';
	    if (c != '%')
		--fmt;		/* so %z outputs %z etc. */
	    --buflen;
	    continue;
	}
	if (base != 0) {
	    str = num + sizeof(num);
	    *--str = 0;
	    while (str > num + neg) {
		*--str = hexchars[val % base];
		val = val / base;
		if (--prec <= 0 && val == 0)
		    break;
	    }
	    switch (neg) {
	    case 1:
		*--str = '-';
		break;
	    case 2:
		*--str = 'x';
		*--str = '0';
		break;
	    }
	    len = num + sizeof(num) - 1 - str;
	} else {
	    len = strlen(str);
	    if (prec > 0 && len > prec)
		len = prec;
	}
	if (width > 0) {
	    if (width > buflen)
		width = buflen;
	    if ((n = width - len) > 0) {
		buflen -= n;
		for (; n > 0; --n)
		    *buf++ = fillch;
	    }
	}
	if (len > buflen)
	    len = buflen;
	memcpy(buf, str, len);
	buf += len;
	buflen -= len;
    }
    *buf = 0;
    return buf - buf0;
}

