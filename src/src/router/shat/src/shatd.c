/*
 * Copyright (C) 2003-2005 Maxina GmbH - Jordan Hrycaj
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 * Author: Jordan Hrycaj <jordan@mjh.teddy.net.com>
 *
 * $Id: shatd.c,v 1.70 2005/05/29 15:02:26 jordan Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/ether.h>
#include <pwd.h>

#define _GNU_SOURCE
#include <getopt.h>

#include "do.h"
#include "device.h"
#include "pool.h"
#include "lookup.h"
#include "arp.h"
#include "ip2ether.h"
#include "cleanup.h"
#include "ctrl.h"
#include "comhandler.h"

/* ----------------------------------------------------------------------- *
 * variables and definitions
 * ----------------------------------------------------------------------- */

#ifdef _DEBUG
#define _MAIN_DEBUG
#endif

typedef
struct _desc {
    DO       *loop ;
    ARP       *arp ;
    IN_DEV   *idev ;
    TUN_DEV  *tdev ;
    IP2E     *ip2e ;
    LOOKUP *lookup ;
    RANGE   *range ;
    REGISTER *dhcp ;
} SESSION ;


extern char *optarg;
extern int optind, opterr, optopt;

static jmp_buf go_here;           /* jump buffer for long jumps */

/* ----------------------------------------------------------------------- *
 * Fiddling around with options ...
 * ----------------------------------------------------------------------- */

#define __arg1of4(a,b,c,d)    a
#define __arg2of4(a,b,c,d)    b
#define __arg3of4(a,b,c,d)    c
#define __arg4of4(a,b,c,d)    d
#define __str(x)              # x
#define str(x)                __str (x)

/* I am fed up with option name and option key management using strings
   and letters at several places - so keep them together -- jordan */

#define opt_name(x)         x (__arg1of4)
#define opt_key(x)          x (__arg2of4)
#define opt_argtype(x)      x (__arg3of4)
#define opt_info(x)         x (__arg4of4)

#define OPT_VERSION(x)      x ("version",       'V', 0,           "print version and exit")
#define OPT_VERBOSE(x)      x ("verbose",       'v', 0,           "print extra diagnostic messages")
#define OPT_HELP(x)         x ("help",          'h', 0,           "instead of a manual page")
#define OPT_SYSLOG(x)       x ("syslog",        's', "FACILITY",  "use syslog with the given facility")
#define OPT_NOTIMEPFX(x)    x ("no-timestamp",  'n', 0,           "do not prefix log data by a time stamp")
#define OPT_DETACH(x)       x ("detach",        'd', 0,           "daemon mode, put yourself in the background")
#define OPT_PID_FILE(x)     x ("pid-file",      'p', "FILE",      "save the process pid")
#define OPT_CHROOT(x)       x ("chroot",        'r', "DIR",       "chroot to DIR after initialisation (will not restore /proc settings)")
#ifndef DISCARD_CMD_SOCKET
#define OPT_GROUPID(x)      x ("groupid",       'g', "GID",       "allow this group to connect and send admin requests")
#endif

#define OPT_LAN_DEVICE(x)   x ("lan-device",    'i', "NAME",      "the LAN interface name (default=" DEFAULT_INBOUND ")")
#define OPT_SHAT_DEVICE(x)  x ("shat-device",   'I', "NAME",      "the TUN interface name (default=" DEFAULT_TUNBOUND ")")
#define OPT_POOL(x)         x ("shat-ip-pool",  'P', "<IP-LIST>", "the IP address pool for SHAT")
#define OPT_EXCLUDE(x)      x ("exclude-ip",    'X', "<IP-LIST>", "ignore these addresses (default: all LAN interface addresses)")
#define OPT_SHAT_ADDRESS(x) x ("shat-address",  'A', "IP",        "the TUN interface address (default=" TUN_DEFAULT_IP ")")

#define OPT_LOCAL(x)        x ("local-ip",      'L', "<IP-LIST>", "TCP/IP scan protection (default: the LAN interface network)")
#define OPT_SPF_REPLIES(x)  x ("spoof-replies", 'N', "NUMBER",    "number of ARP spoof replies (at least 1, default=" str (ARP_SPOOF_REPLIES) ")")
#define OPT_SPF_DELAY(x)    x ("spoof-delay",   'S', "SECONDS",   "seconds between consecutive replies (at least 3, default=" str (ARP_SPOOF_REP_INTV) ")")
#define OPT_SPF_DEPTH(x)    x ("spoof-depth",   'Q', "NUMBER",    "number of entries for ARP spoofing (at least 5, default=" str (ARP_BATCH_DEFAULT_LENGTH) ")")
#define OPT_ATTRACT_BY(x)   x ("attract-by",    'M', "MAC",       "the MAC address for ARP spoofing (default=" ARP_SPOOF_DEFAULT_MAC ")")

#define OPT_IDLE_TIMER(x)   x ("idle-timer",    't', "SECS",      "idle timeout (default=" str (IDLE_TIMEOUT) ")")

#define OPT_NULL(x)         x (0,               0 , 0,            0)


#define LONG_OPTION_RECORD(x,y)  {opt_name (x), y, 0, opt_key (x)}
static const
struct option long_options [] = {
    LONG_OPTION_RECORD (OPT_VERSION,      0),
    LONG_OPTION_RECORD (OPT_VERBOSE,      0),
    LONG_OPTION_RECORD (OPT_NOTIMEPFX,    0),
#   ifndef DISCARD_INFO
    LONG_OPTION_RECORD (OPT_HELP,         0),
#   endif /* DISCARD_INFO */
#ifndef DISCARD_CMD_SOCKET
    LONG_OPTION_RECORD (OPT_GROUPID,      1),
#endif
    LONG_OPTION_RECORD (OPT_DETACH,       0),
    LONG_OPTION_RECORD (OPT_CHROOT,       1),
    LONG_OPTION_RECORD (OPT_PID_FILE,     1),
    LONG_OPTION_RECORD (OPT_IDLE_TIMER,   1),
    LONG_OPTION_RECORD (OPT_LAN_DEVICE,   1),
    LONG_OPTION_RECORD (OPT_SHAT_DEVICE,  1),
    LONG_OPTION_RECORD (OPT_POOL,         1),
    LONG_OPTION_RECORD (OPT_EXCLUDE,      1),
    LONG_OPTION_RECORD (OPT_LOCAL,        1),
    LONG_OPTION_RECORD (OPT_ATTRACT_BY,   1),
    LONG_OPTION_RECORD (OPT_SPF_REPLIES,  1),
    LONG_OPTION_RECORD (OPT_SPF_DELAY,    1),
    LONG_OPTION_RECORD (OPT_SPF_DEPTH,    1),
    LONG_OPTION_RECORD (OPT_SHAT_ADDRESS, 1),
    LONG_OPTION_RECORD (OPT_SYSLOG,       1),
    LONG_OPTION_RECORD (OPT_NULL,         0)
};
#undef LONG_OPTION_RECORD


static const
char short_options [] = {
    opt_key (OPT_VERSION),
    opt_key (OPT_VERBOSE),
    opt_key (OPT_NOTIMEPFX),
#   ifndef DISCARD_INFO
    opt_key (OPT_HELP),
#   endif /* DISCARD_INFO */
#   ifndef DISCARD_CMD_SOCKET
    opt_key (OPT_GROUPID),      ':',
#   endif
    opt_key (OPT_DETACH),
    opt_key (OPT_CHROOT),       ':',
    opt_key (OPT_PID_FILE),     ':',
    opt_key (OPT_IDLE_TIMER),   ':',
    opt_key (OPT_LAN_DEVICE),   ':',
    opt_key (OPT_SHAT_DEVICE),  ':',
    opt_key (OPT_POOL),         ':',
    opt_key (OPT_EXCLUDE),      ':',
    opt_key (OPT_LOCAL),        ':',
    opt_key (OPT_ATTRACT_BY),   ':',
    opt_key (OPT_SPF_REPLIES),  ':',
    opt_key (OPT_SPF_DELAY),    ':',
    opt_key (OPT_SPF_DEPTH),    ':',
    opt_key (OPT_SHAT_ADDRESS), ':',
    opt_key (OPT_SYSLOG),       ':',
    opt_key (OPT_NULL)
} ;

/* ----------------------------------------------------------------------- *
 * Timeout signal handler
 * ----------------------------------------------------------------------- */
static void s_go_here (int n) { longjmp (go_here, 1); }

/* ----------------------------------------------------------------------- *
 * call back functions
 * ----------------------------------------------------------------------- */

static
void idle (void *dsc) {
    SESSION *s = dsc ;

    if (verbosity & LOG_IDLE_LOOPING) {
        logger (LOG_INFO, "Idle handler.");
        show_pool_records (s->lookup) ;
    }

    arp_pending (s->arp);

    maintenance (s->lookup);

    // s_go_here (0); /* teminate now - debugging only */
}


static
void resched (void *dsc, int timeout) {  /* reschedule idle timer */
    SESSION *s = dsc ;

    if (verbosity & LOG_IDLE_LOOPING)
        logger (LOG_INFO, "Rescheduling the idle timer: %d.", timeout);

    /* the scheduler wants millisecs while the arp handler knows seconds */
    do_resched (s->loop, timeout * 1000);
}


static
void add_route (struct in_addr from,
                struct in_addr   to,
                void             *p) {
    SESSION *s = p;
    ip4addr_t from_ip = ntohl (from.s_addr);
    ip4addr_t   to_ip = ntohl (to.s_addr);

    /* add route, exit on error */
    tunnel_route (s->tdev, from_ip, to_ip, 1, 1);
}

#ifdef _MAIN_DEBUG
static
void delete_route (struct in_addr from,
                   struct in_addr   to,
                   void             *p) {
    SESSION *s = p;
    ip4addr_t from_ip = ntohl (from.s_addr);
    ip4addr_t   to_ip = ntohl (to.s_addr);

    /* delete route, don't exit on error */
    tunnel_route (s->tdev, from_ip, to_ip, 0, 0);
}
#endif

/* ----------------------------------------------------------------------- *
 * helpers
 * ----------------------------------------------------------------------- */

static
void cpry (void) {
    logger (LOG_INFO, "SHAT - Source Hardware Address Translation (Lingua Franca for IP)");
    logger (LOG_INFO, "Copyright (c) 2003-2005 Maxina GmbH -- GPL-2 licenced");
    logger (LOG_INFO, "Version " VERSION " -- Jordan Hrycaj <jordan@mjh.teddy-net.com>");
}

#ifndef DISCARD_INFO
static
void usage_line (const char indent,
                 const  char *name,
                 char          key,
                 const char   *arg,
                 const char  *info) {
    char longopt [100], format [30] ;
    strcpy (longopt, name) ;
    if (arg) { strcat (longopt, "="); strcat (longopt, arg); }
    sprintf (format, "%%%us-%%c, --%%-24s %%s\n", indent);
    fprintf (stderr, format, "", key, longopt, info);
}
#endif /* DISCARD_INFO */


static
void usage (const char *info, const char *opt) {

#   ifndef DISCARD_INFO
    if (info != 0) {
        fprintf (stderr, "\n%s: ERROR: %s", progname, info);
        if (opt) fprintf (stderr, " (option arg was %s)", opt);
        fprintf (stderr, "\n");
    }

#   define SHOW_OPTION(indent,opt)  usage_line (indent,            \
                                                opt_name    (opt), \
                                                opt_key     (opt), \
                                                opt_argtype (opt), \
                                                opt_info    (opt)  )

    fprintf (stderr, "\nUsage: %s [options]\n", progname);

    fputs ("\nGeneral options:", stderr);
    SHOW_OPTION ( 4, OPT_VERBOSE);
    SHOW_OPTION (20, OPT_HELP);
    SHOW_OPTION (20, OPT_SYSLOG);
    SHOW_OPTION (20, OPT_CHROOT);
    SHOW_OPTION (20, OPT_NOTIMEPFX);
    SHOW_OPTION (20, OPT_DETACH);
    SHOW_OPTION (20, OPT_PID_FILE);
    SHOW_OPTION (20, OPT_LAN_DEVICE);
#   ifndef DISCARD_CMD_SOCKET
    SHOW_OPTION (20, OPT_GROUPID);
#   endif

    fputs ("\nSHAT processing:", stderr);
    SHOW_OPTION ( 4, OPT_SHAT_DEVICE);
    SHOW_OPTION (20, OPT_POOL);
    SHOW_OPTION (20, OPT_SHAT_ADDRESS);
    SHOW_OPTION (20, OPT_EXCLUDE);

    fputs ("\nTCP/IP scan shield:", stderr);
    SHOW_OPTION ( 1, OPT_LOCAL);
    SHOW_OPTION (20, OPT_SPF_REPLIES);
    SHOW_OPTION (20, OPT_SPF_DELAY);
    SHOW_OPTION (20, OPT_SPF_DEPTH);
    SHOW_OPTION (20, OPT_ATTRACT_BY);

    fputs ("\nMiscellaneous:", stderr);
    SHOW_OPTION ( 6, OPT_IDLE_TIMER);
 
    if (info == 0 && opt == 0) fputs
        ("\n"
         "An <IP-LIST> is a comma separated list of IP-RANGES, or the word NULL\n"
         "or NO.  An IP-RANGE can be written as FROMIP-TOIP, FROMIP+NUMIP, or\n"
         "as FROMIP/NETMASK.  Using the option argument NULL or NO, the list\n"
         "will be disabled and no default applies.\n"
         "\n"
         "All the items FROMIP and TOIP are IP addresses in octet notation\n"
         "(four decimal numbers and dots), the item NUMIP is a decimal number,\n"
         "and NETMASK can be written as the number of leading bits (Cisco\n"
         "notation) or as a netmask in octet notation.\n\n"
         "",  stderr);

#   undef SHOW_OPTION
#   endif /* DISCARD_INFO */

    exit (1);
}


#ifndef DISCARD_INFO
static
void help (void) {
    fputs ("\n", stderr);
    syslog_init (0, 0); /* no timestamps with the introduction */
    cpry ();
    fputs
        ("\n"
         "The SHAT daemon listens on the LAN interface for clients that\n"
         "need to be serviced. Any IP address will be accepted that meets\n"
         "certain criteria.  The client is serviced by layer 2/Ethernet II,\n"
         "so there is no need for the client to enable DHCP.\n"

         "\nTypical scenario:\n"
         " - INTERNET on the outbound interface eth0\n"
         " - clients on the LAN interface eth1 (probably DHCP support)\n"
         " - SHAT runs between the LAN interface eth1 and the TUN interface "
             DEFAULT_TUNBOUND "\n"
         " - a firewall filters between the TUN and the INTERNET interface\n"
         " - the --" opt_name (OPT_POOL)
             " list limits the number of simultaneous SHAT clients\n"

         "\nSome general restrictions apply:\n"
         " - only Ethernet II is supported (the mainstream protocol)\n"
         " - the MAC address must be unique (in theory, it is)\n"

         "\nRestrictions on the client IP address:\n"
         " - the IP address must be different from the --" opt_name (OPT_EXCLUDE)
             " list\n"
         " - the IP address must be different from the --" opt_name (OPT_LOCAL)
             " list\n"

         "\nTCP/IP scan protection:\n"
         " - IP addresses from the --" opt_name (OPT_LOCAL)
             " list are hardly accessible\n"

         "\nHow SHAT works:\n"
         " - Incoming Ethernet frames from the LAN interface are detected\n"
         "   and accepted or immediately discarded.  For an accepted Ethernet\n"
         "   frame, the IP payload is extracted and the source IP address is\n"
         "   replaced by an IP address from the pool. The frame is then\n"
         "   forwarded to a particular TUN interface (which is typically\n"
         "   called " DEFAULT_TUNBOUND ").\n"
         " - Incoming IP frames from the TUN interface are detected and accepted\n"
         "   or immediately discarded. When accepted, the IP target address is\n"
         "   replaced by the original user IP address and an Ethernet header is\n"
         "   added. The frame is then forwarded to the LAN interface.\n"

         "\nHow scan protection works:\n"
         " - Before a system is accessed, the MAC address needs to be found out.\n"
         "   So the the SHAT gateway will shout the answer to any asking system\n"
         "   convincing it to believe what the SHAT gateway tells - finally\n"
         "   causing the connecting system to end up in nowhere land (configurable).\n"
         " - Most clients try to map the surrounding network in order to detect\n"
         "   duplicate IP addresses.  This works as long as the client uses ARP\n"
         "   request with the own address as target IP and the source IP also the\n"
         "   own address, or zero.\n"

         "\n"
         "",  stderr);
}
#endif /* DISCARD_INFO */

static
unsigned parse_iprange (ip4addr_t *first,   /* stores data in host order */
                        ip4addr_t  *last,   /* stotes data in host order */
                        const char *text) { /* input text */
    long width;
    ip4addr_t mask;

    /* we need to edit this line */
    char c, *q, *t, *s = xstrdup (text);
  
    /* check for host or range or network description */
    if ((q = strchr (s, '-')) == 0 &&
        (q = strchr (s, '+')) == 0 &&
        (q = strchr (s, '/')) == 0) {
        /* single host? */
        if ((*first = ntohl (inet_addr (s))) == ntohl (INADDR_NONE))
            goto return_zero;
        *last = *first ;
        goto return_one;
    }

    /* we got a range or a network description */
    c = * q ;
    * q ++ = '\0' ;
    
    /* parse first address */
    if ((*first = ntohl (inet_addr (s))) == ntohl (INADDR_NONE))
        goto return_zero;

    /* we got an address range as <fromip>-<toip>? */
    if (c == '-')
        if ((*last = ntohl (inet_addr (q))) == ntohl (INADDR_NONE))
            goto return_zero;
        else
            goto return_one;

    /* we got an address range as <fromip>+<num>? */
    if (c == '+') {
        if ((width = strtol (q, &t, 10)) <= 0 || !t || *t)
            goto return_zero;
        *last = (*first) + width ;
        goto return_one;
    }

    /* check for cisco notation */
    if ((width = strtol (q, &t, 10)) > 0 && width <= 32 && t && !*t) {
        mask = ~((1 << (32 - width)) - 1);
    }
    else {
        /* ok, check for dot/octett notation? */
        if ((mask = ntohl (inet_addr (q))) == ntohl (INADDR_NONE) &&
            strcmp (q, "255.255.255.255") != 0)
            goto return_zero;
    }

    /* was /32 => host mask => last host == first host */
    if (mask == -1) {
        *last = *first ;
        goto return_one;
    }

    /* network: last host address is 1 below broadcast address */
    *last = ((*first) | ~mask) - 1;

    /* network: first host address is 1 above network address */
    if ((*first & ~mask) == 0) (*first) ++ ;

    /* proceed returning one */
return_one:
    free (s);
    return 1;

return_zero:
    free (s);
    return 0;
}


static
unsigned parse_ether (struct ether_addr *mac,
                      const char       *text) {
    
    if (text && *text) {
        struct ether_addr *mp = ether_aton (text) ;
        if (mp) {
            *mac = *mp;
            return 1;
        }
    }

    return 0;
}


static
void range_option (IP4_POOL     *p, 
                   const char *txt,
                   const char *inf) {

    struct in_addr ip ;
    ip4addr_t from_ip, to_ip ;
    char *buf = xstrdup (txt);
    char *s = buf, *e;
    
    /* the text may be a comma separated list */
    do {
        /* separate the leftmost token */
        if ((e = strchr (s, ',')) != 0) * e ++ = '\0';

        /* leading skip spaces */
        while (isspace (*s)) ++ s ;

        /* the parser returns addresses in host order */
        if (parse_iprange (&from_ip, &to_ip, s) == 0) {
            free (buf); 
            usage (inf, txt); /* no return */
        }
        
        do { /* store addresses in network order for easy lookup */
            ip.s_addr = htonl (from_ip) ;
            ip4_fetch (p, &ip) ;
        }
        while (++ from_ip <= to_ip) ;
    }
    while ((s = e) != 0);

    free (buf);
}


/* ----------------------------------------------------------------------- *
 * MAIN
 * ----------------------------------------------------------------------- */

#ifndef _MAIN_DEBUG
#undef main /* in case some debugging redirection applied */
#endif

int main (int argc, char *argv[]) {

    struct ether_addr mac ;
    int n, c, option_inx ;

    char        *new_root =  0 ;
    int shat_easy_clients =  1 ;
    char       *facility  =  0 ;
    int           timepfx =  1 ;
    int            pid_fd = -1 ;
    int            detach =  0 ;
    int           verbose =  0 ;
    int     spoof_replies =  0 ;
    int   spoof_send_intv =  0 ;
    int  spoof_queue_size =  0 ;
    int      idle_timeout =  IDLE_TIMEOUT ;
    char          *in_dev =  0 ;
    char         *tun_dev =  0 ;
    IP4_POOL     *ip_pool =  0 ;
    IP4_POOL  *exclude_ip =  0 ;
    int     no_exclude_ip =  0 ;
    IP4_POOL    *local_ip =  0 ;
    IP4_POOL  *exlocal_ip =  0 ;
    int       no_local_ip =  0 ;
    IP4_POOL  *inbnd_addr =  0 ;
    IP4_POOL   *netw_addr =  0 ;
    struct ether_addr  *m =  0 ;
    in_addr_t    tun_addr =  0 ;
#   ifndef DISCARD_CMD_SOCKET
    unsigned       set_gid = 0 ; /* integer() sets type int */
    CTRL             *ctrl = 0 ;
    COMM             *comm = 0 ;
#   endif

    SESSION s ;
    XZERO (s);

    /* progname ::= only the last path segment */
    if ((progname = strrchr (argv [0], '/')) == 0)
        progname = argv [0] ;
    else
        ++ progname ;
    
    while ((c = getopt_long (argc, argv, short_options, 
                             long_options, &option_inx)) != -1)
        switch (c) {
            struct passwd *pwd;
#       ifndef DISCARD_CMD_SOCKET
        case opt_key (OPT_GROUPID):
            if ((integer (& set_gid, optarg) == 0 ||
                 (pwd = getpwuid (set_gid)) == 0) && 
                (pwd = getpwnam (optarg) ) == 0)
                usage ("group name dooes not exist", optarg);
            set_gid = pwd->pw_gid;
            continue ;
#       endif
        case opt_key (OPT_CHROOT):
            new_root = optarg;
            continue ;
        case opt_key (OPT_LAN_DEVICE):
            in_dev = optarg;
            continue ;
        case opt_key (OPT_SPF_DEPTH):
            if ((spoof_queue_size = atoi (optarg)) < 5)
                usage ("unacceptable arp spoof queue size", optarg);
            continue ;
        case opt_key (OPT_SPF_REPLIES):
            if ((spoof_replies = atoi (optarg)) < 1)
                usage ("unacceptable number of spoof replies", optarg);
            continue ;
        case opt_key (OPT_SPF_DELAY):
            if ((spoof_send_intv = atoi (optarg)) < 3)
                usage ("unacceptable spoof delay time", optarg);
            continue ;
        case opt_key (OPT_NOTIMEPFX):
            timepfx = 0 ;
            continue ;
        case opt_key (OPT_DETACH):
            detach = 1 ;
            continue ;
        case opt_key (OPT_PID_FILE):
            unlink (optarg); /* don't be tricked by a symlink */
            if ((pid_fd = open (optarg, O_CREAT|O_EXCL|O_WRONLY, 0644)) < 0)
                usage ("cannot open or create pid file", optarg);
            continue ;
        case opt_key (OPT_POOL):
            if (ip_pool == 0) {
                if (strcasecmp (optarg, "null") == 0 ||
                    strcasecmp (optarg, "no")   == 0) {
                    continue ;
                }
                ip_pool = ip4_pool_init ();
            }
            range_option
                (ip_pool, optarg, "error parsing ip pool");
            continue ;
        case opt_key (OPT_LOCAL):
            if (local_ip == 0) {
                if (strcasecmp (optarg, "null") == 0 ||
                    strcasecmp (optarg, "no")   == 0) {
                    no_local_ip = 1;
                    continue ;
                }
                local_ip = ip4_pool_init ();
            }
            if (exlocal_ip == 0)
                exlocal_ip = ip4_pool_init ();  /* local + exclude merged */
            range_option
                (local_ip, optarg, "error parsing local ip range");
            range_option (exlocal_ip, optarg, 0);
            continue ;
        case opt_key (OPT_EXCLUDE):
            if (exclude_ip == 0) {
                if (strcasecmp (optarg, "null") == 0 ||
                    strcasecmp (optarg, "no")   == 0) {
                    no_exclude_ip = 1;
                    continue ;
                }
                exclude_ip = ip4_pool_init ();
            }
            if (exlocal_ip == 0)
                exlocal_ip = ip4_pool_init (); /* local + exclude merged */
            range_option
                (exclude_ip, optarg, "error parsing exclude ip range");
            range_option (exlocal_ip, optarg, 0);
            continue ;
        case opt_key (OPT_ATTRACT_BY):
            m = &mac ;
            if (parse_ether (m, optarg) == 0)
                usage ("error parsing the mac address", optarg);
            continue ;
        case opt_key (OPT_SHAT_DEVICE):
            if (*(tun_dev = optarg) == 0)
                usage ("shat device must not be an empty string", optarg);
            continue ;
        case opt_key (OPT_SHAT_ADDRESS):
            if ((tun_addr = inet_addr (optarg)) == ntohl (INADDR_NONE))
                usage ("error parsiing the shat address", optarg);
            continue ;
        case opt_key (OPT_IDLE_TIMER):
            if ((idle_timeout = atoi (optarg)) < 0)
                usage ("error parsing the idle timeout", optarg);
            continue ;
        case opt_key (OPT_SYSLOG):
            facility = optarg ;
            continue ;
        case opt_key (OPT_VERSION):
            cpry ();
            exit (0);
        case opt_key (OPT_VERBOSE):
            verbose = -1 ;
            continue ;
#       ifndef DISCARD_INFO
        case '?':
            usage (0, "?") ; /* no return */
        case opt_key (OPT_HELP):
            help (); /* and usage */
#       endif /* DISCARD_INFO */
        default:
            usage (0,0) ; /* no return */
        }

    if (optind < argc) {
        fprintf
            (stderr, "%s: Unexpected non-optional arguments: ", progname);
        while (optind < argc)
            fprintf (stderr, "%s ", argv [optind++]);
        fprintf (stderr, "\n");
        exit (2);
    }

    /* neither port scan mode, nor SHAT mode ? */
    if (ip4_pool_empty (ip_pool) && no_local_ip) {
        /* change root environment */
        if (new_root && (chroot (new_root) || chdir ("/"))) {
            fprintf (stderr, "chroot failed: %s\n", strerror (errno));
            exit (2);
        }
        fprintf
            (stderr, "%s: nothing to do - sleeping forever.\n", progname);
        pause () ;
        exit (2);
    }
    
    /* we must use syslog when running in the background */
    if (detach && facility == 0) facility = "daemon" ;

    /* initialise logging subsystem */
    if (facility) {
        if (syslog_init (facility, timepfx) == 0)
            usage ("cannot open the syslog facility", facility);
        /* detach, run in the backgrond */
        if (detach) {
            if (fork()) exit (0);
            setsid();
        }
        freopen ("/dev/null", "r", stdin);
        freopen ("/dev/null", "w", stdout);
        freopen ("/dev/null", "w", stderr);
    }
    else
        syslog_init (0, timepfx);
    
    cpry ();

    /* record own process id */
    if (pid_fd >= 0) {
        char buf [20];
        sprintf (buf, "%d\n", getpid ());
        write (pid_fd, buf, strlen (buf));
        close (pid_fd);
    }

    /* initalise temporary lists */
    if (exclude_ip == 0) inbnd_addr = ip4_pool_init ();
    if (local_ip   == 0) netw_addr  = ip4_pool_init ();
    
    /* initialise interfaces */
    s.idev = inbound_if_init (in_dev, 
                              inbnd_addr,   /* register interface addr */
                              netw_addr,    /* register network addr */
                              !no_local_ip, /* scan prot. => capture all */
                              1);           /* 1: exit on error */

    s.tdev = tunnel_if_init (tun_dev,
                             tun_addr,      /* set to this address */
                             1);            /* 1: exit on error */
#   ifndef DISCARD_CMD_SOCKET
    /* initialise the server control socket before chroot */
    ctrl = ctrl_init (set_gid);
#   endif

    /* change root environment */
    if (new_root && (chroot (new_root) || chdir ("/"))) {
        logger (LOG_ERR, "chroot failed: %s", strerror (errno));
        exit (2);
    }

    /* assign/merge with defaults */
    if (inbnd_addr != 0) {
        if (no_exclude_ip == 0)
            exclude_ip = inbnd_addr;
        exlocal_ip = ip4_pool_merge (exlocal_ip, inbnd_addr);
    }
    if (netw_addr != 0) {
        if (no_local_ip)
            /* need to exclude the local range by default */
            ip4_pool_merge (exclude_ip, netw_addr);
        else 
            local_ip = netw_addr;
        exlocal_ip = ip4_pool_merge (exlocal_ip, netw_addr);
    }

    if (!no_local_ip) /* scan protection => dhcp register */
        s.dhcp = register_init (local_ip) ;
    
    /* initialise arp session */
    s.lookup = lookup_init (ip_pool, s.idev) ;
    s.arp = arp_init (s.idev,                  /* shat interface */
                      s.lookup,                /* shat pool */
                      range_init (exclude_ip), /* ignore these */
                      range_init (local_ip),   /* arp spoof range */
                      s.dhcp,                  /* registered dhcp clients */
                      m,                       /* arp spoof mac */
                      spoof_queue_size,        /* batch queue length */
                      resched, &s);            /* reschedule idle timer */

    /* initialise ip/ethernet session */
    s.ip2e = ip2ether_init (s.tdev,                  /* tun interface */
                            s.idev,                  /* shat interface */
                            s.lookup,                /* shat pool */
                            range_init (exlocal_ip), /* ignore these */
                            s.dhcp);                 /* dhcp register */
    /* add routes */
    if (shat_easy_clients) {
        s.range = range_init (ip_pool) ;
        for_known_range (s.range, add_route, &s);
    }

    /* not needed, anymore */
    ip4_pool_close (exclude_ip, 0);
    ip4_pool_close (local_ip,   0);
    ip4_pool_close (exlocal_ip, 0);

    /* tuning */
    if (spoof_replies   > 0) s.arp->spoof_replies   = spoof_replies ;
    if (spoof_send_intv > 1) s.arp->spoof_send_intv = spoof_send_intv ;

    verbosity = verbose ? -1 : ARP_REQUEST_LOG|
                                 ARP_REPLY_LOG|
                            IP2E_DHCP_REGISTER;
    
#   ifndef DISCARD_CMD_SOCKET
    /* add services ... */
    logger (LOG_INFO, "Adding control socket services ...");
    comm = comm_init ();
    if (ip_pool) {
        ctrl = ctrl_add (ctrl, "cl-mac",     comm_find_mac,    comm);
        ctrl = ctrl_add (ctrl, "cl-ip",      comm_find_ip,     comm);
        ctrl = ctrl_add (ctrl, "cl-slot",    comm_find_slot,   comm);
        ctrl = ctrl_add (ctrl, "cl-ping",    comm_ping_slot,   comm);
        ctrl = ctrl_add (ctrl, "cl-slots",   comm_list,        comm);
        ctrl = ctrl_add (ctrl, "cl-rm-slot", comm_delete_slot, comm);
        ctrl = ctrl_add (ctrl, "cl-rm-ip",   comm_delete_ip,   comm);
    }
    if (no_local_ip == 0)
        ctrl = ctrl_add (ctrl, "arp-slots", comm_reg_list, comm);
    ctrl = ctrl_add (ctrl, "noisy",      ctrl_verbosity,   ctrl);
    ctrl = ctrl_add (ctrl, "info",       comm_info,        comm);
    ctrl = ctrl_add (ctrl, "help",       ctrl_usage,       ctrl);
    comm = comm_update (comm, s.lookup, s.arp, s.ip2e, ctrl, s.dhcp);
#   endif

    if (setjmp (go_here) == 0) {
        signal (SIGINT,  s_go_here);
        signal (SIGTERM, s_go_here);

        /* initialise action loop */
        s.loop = do_init (idle, &s);

        s.loop = do_action  (s.loop,
                             arp_device (s.arp)->arp_fd,
                             (void(*)(void*))arp_catch,
                             s.arp);

        if (s.lookup) { /* s.lookup can be 0 (port scan shield mode) */
            s.loop = do_action  (s.loop,
                                 ip2ether_device (s.ip2e)->eth_fd,
                                 (void(*)(void*))ip2ether_injector,
                                 s.ip2e);

            s.loop = do_action  (s.loop,
                                 ip2ether_tundev (s.ip2e)->tun_fd,
                                 (void(*)(void*))ip2ether_xtractor,
                                 s.ip2e);
        }

#       ifndef DISCARD_CMD_SOCKET
        /* accept commands */
        s.loop = do_action  (s.loop,
                             ctrl_fd (ctrl),
                             (void(*)(void*))ctrl_handler,
                             ctrl);
#       endif
        
        for (;;) {
            n = do_loop (s.loop, 0, idle_timeout * 1000);
            if (verbose) 
                logger (LOG_INFO, "s%: do_loop -> %d\n", progname, n);
        }
        /*NOTREACHED*/
    }

#   ifdef _MAIN_DEBUG
    /* release dispatcher */
    do_destroy (s.loop);
    
    /* release ip/ethernet session */
    for_known_range (s.range, delete_route, &s);
    range_close (s.range);
    tunnel_if_close (s.tdev);
    range_close (ip2ether_exclude (s.ip2e));

    /* release arp session */
    inbound_if_close (s.idev);
    range_close (arp_exclude (s.arp));
    range_close (arp_local (s.arp));

    /* release pool */
    ip4_pool_close (lookup_close (s.lookup), free);
    register_close (s.dhcp) ;

#   ifndef DISCARD_CMD_SOCKET
    /* release the server control socket */
    ctrl_destroy (ctrl) ;
#   endif

    return 0;
#   else
    if (new_root == 0) /* cannot access /proc, otherwise */
        inbound_if_close (s.idev);
    exit (0);
#   endif /* _MAIN_DEBUG */
}

/* ----------------------------------------------------------------------- *
 * End
 * ----------------------------------------------------------------------- */
