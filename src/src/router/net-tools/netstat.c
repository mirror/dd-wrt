/*
 * netstat    This file contains an implementation of the command
 *              that helps in debugging the networking modules.
 *
 * NET-TOOLS    A collection of programs that form the base set of the
 *              NET-3 Networking Distribution for the LINUX operating
 *              system.
 *
 * Version:     $Id: netstat.c,v 1.43 2001/04/15 14:41:17 pb Exp $
 *
 * Authors:     Fred Baumgarten, <dc6iq@insu1.etec.uni-karlsruhe.de>
 *              Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
 *              Phil Packer, <pep@wicked.demon.co.uk>
 *              Johannes Stille, <johannes@titan.os.open.de>
 *              Bernd Eckenfels, <net-tools@lina.inka.de>
 *              Phil Blundell <philb@gnu.org>
 *              Tuan Hoang <tqhoang@bigfoot.com>
 *
 * Tuned for NET3 by:
 *              Alan Cox, <A.Cox@swansea.ac.uk>
 *              Copyright (c) 1993  Fred Baumgarten
 *
 * Modified:
 *
 *960116 {1.01} Bernd Eckenfels:        verbose, cleanups
 *960204 {1.10} Bernd Eckenfels:        aftrans, usage, new route_info, 
 *                                      DLFT_AF
 *960204 {1.11} Bernd Eckenfels:        netlink support
 *960204 {1.12} Bernd Eckenfels:        route_init()
 *960215 {1.13} Bernd Eckenfels:        netlink_print honors HAVE_
 *960217 {1.14} Bernd Eckenfels:        masq_info from Jos Vos and 
 *                                      ax25_info from Jonathan Naylor.
 *960218 {1.15} Bernd Eckenfels:        ipx_info rewritten, -e for tcp/ipx
 *960220 {1.16} Bernd Eckenfels:        minor output reformats, -a for -x
 *960221 {1.17} Bernd Eckenfels:        route_init->getroute_init
 *960426 {1.18} Bernd Eckenfels:        new RTACTION, SYM/NUM, FIB/CACHE
 *960517 {1.19} Bernd Eckenfels:        usage() spelling fix and --unix inode, 
 *                                      ':' is part of sock_addr for --inet
 *960822 {x.xx} Frank Strauss:          INET6 support
 *
 *970406 {1.33} Philip Copeland         Added snmp reporting support module -s
 *                                      code provided by Andi Kleen
 *                                      (relly needs to be kernel hooked but 
 *                                      this will do in the meantime)
 *                                      minor header file misplacement tidy up.
 *980815 {1.xx} Stephane Fillod:       X.25 support
 *980411 {1.34} Arnaldo Carvalho        i18n: catgets -> gnu gettext, substitution
 *                                      of sprintf for snprintf
 *10/1998	Andi Kleen              Use new interface primitives.
 *990101 {1.36}	Bernd Eckenfels		usage updated to include -s and -C -F,
 *					fixed netstat -rC output (lib/inet_gr.c)
 *					removed broken NETLINK Support
 *					fixed format for /proc/net/udp|tcp|raw
 *					added -w,-t,-u TcpExt support to -s
 *990131 {1.37} Jan Kratochvil          added -p for prg_cache() & friends
 *                                      Flames to <short@ucw.cz>.
 *              Tuan Hoang              added IGMP support for IPv4 and IPv6
 *
 *990420 {1.38} Tuan Hoang              removed a useless assignment from igmp_do_one()
 *20010404 {1.39} Arnaldo Carvalho de Melo - use setlocale
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 *
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <netdb.h>
#include <paths.h>
#include <pwd.h>
#include <getopt.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <dirent.h>

#include "net-support.h"
#include "pathnames.h"
#include "version.h"
#include "config.h"
#include "intl.h"
#include "sockets.h"
#include "interface.h"
#include "util.h"

#define PROGNAME_WIDTH 20

#if !defined(s6_addr32) && defined(in6a_words)
#define s6_addr32 in6a_words	/* libinet6			*/
#endif

/* prototypes for statistics.c */
void parsesnmp(int, int, int);
void inittab(void);

typedef enum {
    SS_FREE = 0,		/* not allocated                */
    SS_UNCONNECTED,		/* unconnected to any socket    */
    SS_CONNECTING,		/* in process of connecting     */
    SS_CONNECTED,		/* connected to socket          */
    SS_DISCONNECTING		/* in process of disconnecting  */
} socket_state;

#define SO_ACCEPTCON    (1<<16)	/* performed a listen           */
#define SO_WAITDATA     (1<<17)	/* wait data to read            */
#define SO_NOSPACE      (1<<18)	/* no space to write            */

#define DFLT_AF "inet"

#define FEATURE_NETSTAT
#include "lib/net-features.h"

char *Release = RELEASE, *Version = "netstat 1.42 (2001-04-15)", *Signature = "Fred Baumgarten, Alan Cox, Bernd Eckenfels, Phil Blundell, Tuan Hoang and others";


#define E_READ  -1
#define E_IOCTL -3

int flag_int = 0;
int flag_rou = 0;
int flag_mas = 0;
int flag_sta = 0;

int flag_all = 0;
int flag_lst = 0;
int flag_cnt = 0;
int flag_deb = 0;
int flag_not = 0;
int flag_cf  = 0;
int flag_opt = 0;
int flag_raw = 0;
int flag_tcp = 0;
int flag_udp = 0;
int flag_igmp= 0;
int flag_rom = 0;
int flag_exp = 1;
int flag_prg = 0;
int flag_arg = 0;
int flag_ver = 0;

FILE *procinfo;

#define INFO_GUTS1(file,name,proc)			\
  procinfo = fopen((file), "r");			\
  if (procinfo == NULL) {				\
    if (errno != ENOENT) {				\
      perror((file));					\
      return -1;					\
    }							\
    if (flag_arg || flag_ver)				\
      ESYSNOT("netstat", (name));			\
    if (flag_arg)					\
      rc = 1;						\
  } else {						\
    do {						\
      if (fgets(buffer, sizeof(buffer), procinfo))	\
        (proc)(lnr++, buffer);				\
    } while (!feof(procinfo));				\
    fclose(procinfo);					\
  }

#if HAVE_AFINET6
#define INFO_GUTS2(file,proc)				\
  lnr = 0;						\
  procinfo = fopen((file), "r");		       	\
  if (procinfo != NULL) {				\
    do {						\
      if (fgets(buffer, sizeof(buffer), procinfo))	\
	(proc)(lnr++, buffer);				\
    } while (!feof(procinfo));				\
    fclose(procinfo);					\
  }
#else
#define INFO_GUTS2(file,proc)
#endif

#define INFO_GUTS3					\
 return rc;

#define INFO_GUTS6(file,file6,name,proc)		\
 char buffer[8192];					\
 int rc = 0;						\
 int lnr = 0;						\
 if (!flag_arg || flag_inet) {				\
    INFO_GUTS1(file,name,proc)				\
 }							\
 if (!flag_arg || flag_inet6) {				\
    INFO_GUTS2(file6,proc)				\
 }							\
 INFO_GUTS3

#define INFO_GUTS(file,name,proc)			\
 char buffer[8192];					\
 int rc = 0;						\
 int lnr = 0;						\
 INFO_GUTS1(file,name,proc)				\
 INFO_GUTS3

#define PROGNAME_WIDTHs PROGNAME_WIDTH1(PROGNAME_WIDTH)
#define PROGNAME_WIDTH1(s) PROGNAME_WIDTH2(s)
#define PROGNAME_WIDTH2(s) #s

#define PRG_HASH_SIZE 211

static struct prg_node {
    struct prg_node *next;
    int inode;
    char name[PROGNAME_WIDTH];
} *prg_hash[PRG_HASH_SIZE];

static char prg_cache_loaded = 0;

#define PRG_HASHIT(x) ((x) % PRG_HASH_SIZE)

#define PROGNAME_BANNER "PID/Program name"

#define print_progname_banner() do { if (flag_prg) printf("%-" PROGNAME_WIDTHs "s"," " PROGNAME_BANNER); } while (0)

#define PRG_LOCAL_ADDRESS "local_address"
#define PRG_INODE	 "inode"
#define PRG_SOCKET_PFX    "socket:["
#define PRG_SOCKET_PFXl (strlen(PRG_SOCKET_PFX))
#define PRG_SOCKET_PFX2   "[0000]:"
#define PRG_SOCKET_PFX2l  (strlen(PRG_SOCKET_PFX2))


#ifndef LINE_MAX
#define LINE_MAX 4096
#endif

#define PATH_PROC	   "/proc"
#define PATH_FD_SUFF	"fd"
#define PATH_FD_SUFFl       strlen(PATH_FD_SUFF)
#define PATH_PROC_X_FD      PATH_PROC "/%s/" PATH_FD_SUFF
#define PATH_CMDLINE	"cmdline"
#define PATH_CMDLINEl       strlen(PATH_CMDLINE)
/* NOT working as of glibc-2.0.7: */
#undef  DIRENT_HAVE_D_TYPE_WORKS

static void prg_cache_add(int inode, char *name)
{
    unsigned hi = PRG_HASHIT(inode);
    struct prg_node **pnp,*pn;

    prg_cache_loaded=2;
    for (pnp=prg_hash+hi;(pn=*pnp);pnp=&pn->next) {
	if (pn->inode==inode) {
	    /* Some warning should be appropriate here
	       as we got multiple processes for one i-node */
	    return;
	}
    }
    if (!(*pnp=malloc(sizeof(**pnp)))) 
	return;
    pn=*pnp;
    pn->next=NULL;
    pn->inode=inode;
    if (strlen(name)>sizeof(pn->name)-1) 
	name[sizeof(pn->name)-1]='\0';
    strcpy(pn->name,name);
}

static const char *prg_cache_get(int inode)
{
    unsigned hi=PRG_HASHIT(inode);
    struct prg_node *pn;

    for (pn=prg_hash[hi];pn;pn=pn->next)
	if (pn->inode==inode) return(pn->name);
    return("-");
}

static void prg_cache_clear(void)
{
    struct prg_node **pnp,*pn;

    if (prg_cache_loaded == 2)
	for (pnp=prg_hash;pnp<prg_hash+PRG_HASH_SIZE;pnp++)
	    while ((pn=*pnp)) {
		*pnp=pn->next;
		free(pn);
	    }
    prg_cache_loaded=0;
}

static void extract_type_1_socket_inode(const char lname[], long * inode_p) {

    /* If lname is of the form "socket:[12345]", extract the "12345"
       as *inode_p.  Otherwise, return -1 as *inode_p.
       */

    if (strlen(lname) < PRG_SOCKET_PFXl+3) *inode_p = -1;
    else if (memcmp(lname, PRG_SOCKET_PFX, PRG_SOCKET_PFXl)) *inode_p = -1;
    else if (lname[strlen(lname)-1] != ']') *inode_p = -1;
    else {
        char inode_str[strlen(lname + 1)];  /* e.g. "12345" */
        const int inode_str_len = strlen(lname) - PRG_SOCKET_PFXl - 1;
        char *serr;

        strncpy(inode_str, lname+PRG_SOCKET_PFXl, inode_str_len);
        inode_str[inode_str_len] = '\0';
        *inode_p = strtol(inode_str,&serr,0);
        if (!serr || *serr || *inode_p < 0 || *inode_p >= INT_MAX) 
            *inode_p = -1;
    }
}



static void extract_type_2_socket_inode(const char lname[], long * inode_p) {

    /* If lname is of the form "[0000]:12345", extract the "12345"
       as *inode_p.  Otherwise, return -1 as *inode_p.
       */

    if (strlen(lname) < PRG_SOCKET_PFX2l+1) *inode_p = -1;
    else if (memcmp(lname, PRG_SOCKET_PFX2, PRG_SOCKET_PFX2l)) *inode_p = -1;
    else {
        char *serr;

        *inode_p=strtol(lname + PRG_SOCKET_PFX2l,&serr,0);
        if (!serr || *serr || *inode_p < 0 || *inode_p >= INT_MAX) 
            *inode_p = -1;
    }
}



static void prg_cache_load(void)
{
    char line[LINE_MAX],eacces=0;
    int procfdlen,fd,cmdllen,lnamelen;
    char lname[30],cmdlbuf[512],finbuf[PROGNAME_WIDTH];
    long inode;
    const char *cs,*cmdlp;
    DIR *dirproc=NULL,*dirfd=NULL;
    struct dirent *direproc,*direfd;

    if (prg_cache_loaded || !flag_prg) return;
    prg_cache_loaded=1;
    cmdlbuf[sizeof(cmdlbuf)-1]='\0';
    if (!(dirproc=opendir(PATH_PROC))) goto fail;
    while (errno=0,direproc=readdir(dirproc)) {
#ifdef DIRENT_HAVE_D_TYPE_WORKS
	if (direproc->d_type!=DT_DIR) continue;
#endif
	for (cs=direproc->d_name;*cs;cs++)
	    if (!isdigit(*cs)) 
		break;
	if (*cs) 
	    continue;
	procfdlen=snprintf(line,sizeof(line),PATH_PROC_X_FD,direproc->d_name);
	if (procfdlen<=0 || procfdlen>=sizeof(line)-5) 
	    continue;
	errno=0;
	dirfd=opendir(line);
	if (! dirfd) {
	    if (errno==EACCES) 
		eacces=1;
	    continue;
	}
	line[procfdlen] = '/';
	cmdlp = NULL;
	while ((direfd = readdir(dirfd))) {
#ifdef DIRENT_HAVE_D_TYPE_WORKS
	    if (direfd->d_type!=DT_LNK) 
		continue;
#endif
	    if (procfdlen+1+strlen(direfd->d_name)+1>sizeof(line)) 
		continue;
	    memcpy(line + procfdlen - PATH_FD_SUFFl, PATH_FD_SUFF "/",
		   PATH_FD_SUFFl+1);
	    strcpy(line + procfdlen + 1, direfd->d_name);
	    lnamelen=readlink(line,lname,sizeof(lname)-1);
            lname[lnamelen] = '\0';  /*make it a null-terminated string*/

            extract_type_1_socket_inode(lname, &inode);

            if (inode < 0) extract_type_2_socket_inode(lname, &inode);

            if (inode < 0) continue;

	    if (!cmdlp) {
		if (procfdlen - PATH_FD_SUFFl + PATH_CMDLINEl >= 
		    sizeof(line) - 5) 
		    continue;
		strcpy(line + procfdlen-PATH_FD_SUFFl, PATH_CMDLINE);
		fd = open(line, O_RDONLY);
		if (fd < 0) 
		    continue;
		cmdllen = read(fd, cmdlbuf, sizeof(cmdlbuf) - 1);
		if (close(fd)) 
		    continue;
		if (cmdllen == -1) 
		    continue;
		if (cmdllen < sizeof(cmdlbuf) - 1) 
		    cmdlbuf[cmdllen]='\0';
		if ((cmdlp = strrchr(cmdlbuf, '/'))) 
		    cmdlp++;
		else 
		    cmdlp = cmdlbuf;
	    }

	    snprintf(finbuf, sizeof(finbuf), "%s/%s", direproc->d_name, cmdlp);
	    prg_cache_add(inode, finbuf);
	}
	closedir(dirfd); 
	dirfd = NULL;
    }
    if (dirproc) 
	closedir(dirproc);
    if (dirfd) 
	closedir(dirfd);
    if (!eacces) 
	return;
    if (prg_cache_loaded == 1) {
    fail:
	fprintf(stderr,_("(No info could be read for \"-p\": geteuid()=%d but you should be root.)\n"),
		geteuid());
    }
    else
	fprintf(stderr, _("(Not all processes could be identified, non-owned process info\n"
			 " will not be shown, you would have to be root to see it all.)\n"));
}

#if HAVE_AFNETROM
static const char *netrom_state[] =
{
    N_("LISTENING"),
    N_("CONN SENT"),
    N_("DISC SENT"),
    N_("ESTABLISHED")
};

static int netrom_info(void)
{
    FILE *f;
    char buffer[256], dev[16];
    int st, vs, vr, sendq, recvq, ret;

    f = fopen(_PATH_PROCNET_NR, "r");
    if (f == NULL) {
	if (errno != ENOENT) {
	    perror(_PATH_PROCNET_NR);
	    return (-1);
	}
	if (flag_arg || flag_ver)
	    ESYSNOT("netstat", "AF NETROM");
	if (flag_arg)
	    return (1);
	else
	    return (0);
    }
    printf(_("Active NET/ROM sockets\n"));
    printf(_("User       Dest       Source     Device  State        Vr/Vs    Send-Q  Recv-Q\n"));
    fgets(buffer, 256, f);

    while (fgets(buffer, 256, f)) {
	buffer[9] = 0;
	buffer[19] = 0;
	buffer[29] = 0;
	ret = sscanf(buffer + 30, "%s %*x/%*x %*x/%*x %d %d %d %*d %*d/%*d %*d/%*d %*d/%*d %*d/%*d %*d/%*d %*d %d %d %*d",
	       dev, &st, &vs, &vr, &sendq, &recvq);
	if (ret != 6) {
	    printf(_("Problem reading data from %s\n"), _PATH_PROCNET_NR);
	    continue;
	}
	printf("%-9s  %-9s  %-9s  %-6s  %-11s  %03d/%03d  %-6d  %-6d\n",
	       buffer, buffer + 10, buffer + 20,
	       dev,
	       _(netrom_state[st]),
	       vr, vs, sendq, recvq);
    }
    fclose(f);
    return 0;
}
#endif

/* These enums are used by IPX too. :-( */
enum {
    TCP_ESTABLISHED = 1,
    TCP_SYN_SENT,
    TCP_SYN_RECV,
    TCP_FIN_WAIT1,
    TCP_FIN_WAIT2,
    TCP_TIME_WAIT,
    TCP_CLOSE,
    TCP_CLOSE_WAIT,
    TCP_LAST_ACK,
    TCP_LISTEN,
    TCP_CLOSING			/* now a valid state */
};

#if HAVE_AFINET || HAVE_AFINET6

static const char *tcp_state[] =
{
    "",
    N_("ESTABLISHED"),
    N_("SYN_SENT"),
    N_("SYN_RECV"),
    N_("FIN_WAIT1"),
    N_("FIN_WAIT2"),
    N_("TIME_WAIT"),
    N_("CLOSE"),
    N_("CLOSE_WAIT"),
    N_("LAST_ACK"),
    N_("LISTEN"),
    N_("CLOSING")
};

static void finish_this_one(int uid, unsigned long inode, const char *timers)
{
    struct passwd *pw;

    if (flag_exp > 1) {
	if (!(flag_not & FLAG_NUM_USER) && ((pw = getpwuid(uid)) != NULL))
	    printf("%-10s ", pw->pw_name);
	else
	    printf("%-10d ", uid);
	printf("%-10ld ",inode);
    }
    if (flag_prg)
	printf("%-" PROGNAME_WIDTHs "s",prg_cache_get(inode));
    if (flag_opt)
	printf("%s", timers);
    putchar('\n');
}

static void igmp_do_one(int lnr, const char *line)
{
    char mcast_addr[128];
#if HAVE_AFINET6
    struct sockaddr_in6 mcastaddr;
    char addr6[INET6_ADDRSTRLEN];
    struct in6_addr in6;
    extern struct aftype inet6_aftype;
#else
    struct sockaddr_in mcastaddr;
#endif
    struct aftype *ap;
    static int idx_flag = 0;
    static int igmp6_flag = 0;
    static char device[16];
    int num, idx, refcnt;

    if (lnr == 0) {
	/* IPV6 ONLY */
	/* igmp6 file does not have any comments on first line */
	if ( strstr( line, "Device" ) == NULL ) {
	    igmp6_flag = 1;
	} else {
	    /* IPV4 ONLY */
	    /* 2.1.x kernels and up have Idx field */
	    /* 2.0.x and below do not have Idx field */
	    if ( strncmp( line, "Idx", strlen("Idx") ) == 0 )
		idx_flag = 1;
	    else
		idx_flag = 0;
	    return;
	}
    }

    if (igmp6_flag) {    /* IPV6 */
#if HAVE_AFINET6
	num = sscanf( line, "%d %15s %64[0-9A-Fa-f] %d", &idx, device, mcast_addr, &refcnt );
	if (num == 4) {
	    /* Demangle what the kernel gives us */
	    sscanf(mcast_addr, "%08X%08X%08X%08X",
		   &in6.s6_addr32[0], &in6.s6_addr32[1],
           &in6.s6_addr32[2], &in6.s6_addr32[3]);
	    in6.s6_addr32[0] = htonl(in6.s6_addr32[0]);
	    in6.s6_addr32[1] = htonl(in6.s6_addr32[1]);
	    in6.s6_addr32[2] = htonl(in6.s6_addr32[2]);
	    in6.s6_addr32[3] = htonl(in6.s6_addr32[3]);
        inet_ntop(AF_INET6, &in6, addr6, sizeof(addr6));
	    inet6_aftype.input(1, addr6, (struct sockaddr *) &mcastaddr);
	    mcastaddr.sin6_family = AF_INET6;
	} else {
	    fprintf(stderr, _("warning, got bogus igmp6 line %d.\n"), lnr);
	    return;
	}

	if ((ap = get_afntype(((struct sockaddr *) &mcastaddr)->sa_family)) == NULL) {
	    fprintf(stderr, _("netstat: unsupported address family %d !\n"),
		    ((struct sockaddr *) &mcastaddr)->sa_family);
	    return;
	}
	safe_strncpy(mcast_addr, ap->sprint((struct sockaddr *) &mcastaddr, 
				      flag_not), sizeof(mcast_addr));
	printf("%-15s %-6d %s\n", device, refcnt, mcast_addr);
#endif
    } else {    /* IPV4 */
#if HAVE_AFINET
	if (line[0] != '\t') {
	    if (idx_flag) {
		if ((num = sscanf( line, "%d\t%10c", &idx, device)) < 2) {
		    fprintf(stderr, _("warning, got bogus igmp line %d.\n"), lnr);
		    return;
		}
	    } else {
		if ( (num = sscanf( line, "%10c", device )) < 1 ) {
		    fprintf(stderr, _("warning, got bogus igmp line %d.\n"), lnr);
		    return;
		}
	    }
	    device[10] = '\0';
	    return;
	} else if ( line[0] == '\t' ) {
	    if ( (num = sscanf(line, "\t%8[0-9A-Fa-f] %d", mcast_addr, &refcnt)) < 2 ) {
		fprintf(stderr, _("warning, got bogus igmp line %d.\n"), lnr);
		return;
	    }
	    sscanf( mcast_addr, "%X",
		    &((struct sockaddr_in *) &mcastaddr)->sin_addr.s_addr );
	    ((struct sockaddr *) &mcastaddr)->sa_family = AF_INET;
	} else {
	    fprintf(stderr, _("warning, got bogus igmp line %d.\n"), lnr);
	    return;
	}
	
	if ((ap = get_afntype(((struct sockaddr *) &mcastaddr)->sa_family)) == NULL) {
	    fprintf(stderr, _("netstat: unsupported address family %d !\n"),
		    ((struct sockaddr *) &mcastaddr)->sa_family);
	    return;
	}
	safe_strncpy(mcast_addr, ap->sprint((struct sockaddr *) &mcastaddr, 
				      flag_not), sizeof(mcast_addr));
	printf("%-15s %-6d %s\n", device, refcnt, mcast_addr );
#endif
    }    /* IPV4 */
}

#if HAVE_AFX25
static int x25_info(void)
{
       FILE *f=fopen(_PATH_PROCNET_X25, "r");
       char buffer[256],dev[16];
       int st,vs,vr,sendq,recvq,lci;
       static char *x25_state[5]=
       {
               "LISTENING",
               "SABM_SENT",
               "DISC_SENT",
               "ESTABLISHED",
               "RECOVERY"
       };
       if(!(f=fopen(_PATH_PROCNET_X25, "r")))
       {
               if (errno != ENOENT) {
                       perror(_PATH_PROCNET_X25);
                       return(-1);
               }
               if (flag_arg || flag_ver)
                       ESYSNOT("netstat","AF X25");
               if (flag_arg)
                       return(1);
               else
                       return(0);
       }
       printf( _("Active X.25 sockets\n"));
       /* IMHO, Vr/Vs is not very usefull --SF */
       printf( _("Dest         Source          Device  LCI  State        Vr/Vs  Send-Q  Recv-Q\n"));
       fgets(buffer,256,f);
       while(fgets(buffer,256,f))
       {
               buffer[10]=0;
               buffer[20]=0;
               sscanf(buffer+22,"%s %d %d %d %d %*d %*d %*d %*d %*d %*d %d %d %*d",
                       dev,&lci,&st,&vs,&vr,&sendq,&recvq);
               if (!(flag_all || lci))
                       continue;
               printf("%-15s %-15s %-7s %-3d  %-11s  %02d/%02d  %-6d  %-6d\n",
                       buffer,buffer+11,
                       dev,
                       lci,
                       x25_state[st],
                       vr,vs,sendq,recvq);
       }
       fclose(f);
       return 0;               
}
#endif

static int igmp_info(void)
{
    INFO_GUTS6(_PATH_PROCNET_IGMP, _PATH_PROCNET_IGMP6, "AF INET (igmp)",
	       igmp_do_one);
}

static void tcp_do_one(int lnr, const char *line)
{
    unsigned long rxq, txq, time_len, retr, inode;
    int num, local_port, rem_port, d, state, uid, timer_run, timeout;
    char rem_addr[128], local_addr[128], timers[64], buffer[1024], more[512];
    struct aftype *ap;
#if HAVE_AFINET6
    struct sockaddr_in6 localaddr, remaddr;
    char addr6[INET6_ADDRSTRLEN];
    struct in6_addr in6;
    extern struct aftype inet6_aftype;
#else
    struct sockaddr_in localaddr, remaddr;
#endif

    if (lnr == 0)
	return;

    num = sscanf(line,
    "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %ld %512s\n",
		 &d, local_addr, &local_port, rem_addr, &rem_port, &state,
		 &txq, &rxq, &timer_run, &time_len, &retr, &uid, &timeout, &inode, more);

    if (strlen(local_addr) > 8) {
#if HAVE_AFINET6
	/* Demangle what the kernel gives us */
	sscanf(local_addr, "%08X%08X%08X%08X",
	       &in6.s6_addr32[0], &in6.s6_addr32[1],
           &in6.s6_addr32[2], &in6.s6_addr32[3]);
	inet_ntop(AF_INET6, &in6, addr6, sizeof(addr6));
	inet6_aftype.input(1, addr6, (struct sockaddr *) &localaddr);
	sscanf(rem_addr, "%08X%08X%08X%08X",
	       &in6.s6_addr32[0], &in6.s6_addr32[1],
	       &in6.s6_addr32[2], &in6.s6_addr32[3]);
	inet_ntop(AF_INET6, &in6, addr6, sizeof(addr6));
	inet6_aftype.input(1, addr6, (struct sockaddr *) &remaddr);
	localaddr.sin6_family = AF_INET6;
	remaddr.sin6_family = AF_INET6;
#endif
    } else {
	sscanf(local_addr, "%X",
	       &((struct sockaddr_in *) &localaddr)->sin_addr.s_addr);
	sscanf(rem_addr, "%X",
	       &((struct sockaddr_in *) &remaddr)->sin_addr.s_addr);
	((struct sockaddr *) &localaddr)->sa_family = AF_INET;
	((struct sockaddr *) &remaddr)->sa_family = AF_INET;
    }

    if (num < 11) {
	fprintf(stderr, _("warning, got bogus tcp line.\n"));
	return;
    }
    if ((ap = get_afntype(((struct sockaddr *) &localaddr)->sa_family)) == NULL) {
	fprintf(stderr, _("netstat: unsupported address family %d !\n"),
		((struct sockaddr *) &localaddr)->sa_family);
	return;
    }
    if (state == TCP_LISTEN) {
	time_len = 0;
	retr = 0L;
	rxq = 0L;
	txq = 0L;
    }
    safe_strncpy(local_addr, ap->sprint((struct sockaddr *) &localaddr, 
					flag_not), sizeof(local_addr));
    safe_strncpy(rem_addr, ap->sprint((struct sockaddr *) &remaddr, flag_not),
		 sizeof(rem_addr));
    if (flag_all || (flag_lst && !rem_port) || (!flag_lst && rem_port)) {
	snprintf(buffer, sizeof(buffer), "%s",
		 get_sname(htons(local_port), "tcp",
			   flag_not & FLAG_NUM_PORT));

	if ((strlen(local_addr) + strlen(buffer)) > 22)
	    local_addr[22 - strlen(buffer)] = '\0';

	strcat(local_addr, ":");
	strcat(local_addr, buffer);
	snprintf(buffer, sizeof(buffer), "%s",
		 get_sname(htons(rem_port), "tcp", flag_not & FLAG_NUM_PORT));

	if ((strlen(rem_addr) + strlen(buffer)) > 22)
	    rem_addr[22 - strlen(buffer)] = '\0';

	strcat(rem_addr, ":");
	strcat(rem_addr, buffer);
	timers[0] = '\0';

	if (flag_opt)
	    switch (timer_run) {
	    case 0:
		snprintf(timers, sizeof(timers), _("off (0.00/%ld/%d)"), retr, timeout);
		break;

	    case 1:
		snprintf(timers, sizeof(timers), _("on (%2.2f/%ld/%d)"),
			 (double) time_len / HZ, retr, timeout);
		break;

	    case 2:
		snprintf(timers, sizeof(timers), _("keepalive (%2.2f/%ld/%d)"),
			 (double) time_len / HZ, retr, timeout);
		break;

	    case 3:
		snprintf(timers, sizeof(timers), _("timewait (%2.2f/%ld/%d)"),
			 (double) time_len / HZ, retr, timeout);
		break;

	    default:
		snprintf(timers, sizeof(timers), _("unkn-%d (%2.2f/%ld/%d)"),
			 timer_run, (double) time_len / HZ, retr, timeout);
		break;
	    }
	printf("tcp   %6ld %6ld %-23s %-23s %-12s",
	       rxq, txq, local_addr, rem_addr, _(tcp_state[state]));

	finish_this_one(uid,inode,timers);
    }
}

static int tcp_info(void)
{
    INFO_GUTS6(_PATH_PROCNET_TCP, _PATH_PROCNET_TCP6, "AF INET (tcp)",
	       tcp_do_one);
}

static void udp_do_one(int lnr, const char *line)
{
    char buffer[8192], local_addr[64], rem_addr[64];
    char *udp_state, timers[64], more[512];
    int num, local_port, rem_port, d, state, timer_run, uid, timeout;
#if HAVE_AFINET6
    struct sockaddr_in6 localaddr, remaddr;
    char addr6[INET6_ADDRSTRLEN];
    struct in6_addr in6;
    extern struct aftype inet6_aftype;
#else
    struct sockaddr_in localaddr, remaddr;
#endif
    struct aftype *ap;
    unsigned long rxq, txq, time_len, retr, inode;

    if (lnr == 0)
	return;

    more[0] = '\0';
    num = sscanf(line,
		 "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %ld %512s\n",
		 &d, local_addr, &local_port,
		 rem_addr, &rem_port, &state,
	  &txq, &rxq, &timer_run, &time_len, &retr, &uid, &timeout, &inode, more);

    if (strlen(local_addr) > 8) {
#if HAVE_AFINET6
	sscanf(local_addr, "%08X%08X%08X%08X",
	       &in6.s6_addr32[0], &in6.s6_addr32[1],
	       &in6.s6_addr32[2], &in6.s6_addr32[3]);
	inet_ntop(AF_INET6, &in6, addr6, sizeof(addr6));
	inet6_aftype.input(1, addr6, (struct sockaddr *) &localaddr);
	sscanf(rem_addr, "%08X%08X%08X%08X",
	       &in6.s6_addr32[0], &in6.s6_addr32[1],
	       &in6.s6_addr32[2], &in6.s6_addr32[3]);
	inet_ntop(AF_INET6, &in6, addr6, sizeof(addr6));
	inet6_aftype.input(1, addr6, (struct sockaddr *) &remaddr);
	localaddr.sin6_family = AF_INET6;
	remaddr.sin6_family = AF_INET6;
#endif
    } else {
	sscanf(local_addr, "%X",
	       &((struct sockaddr_in *) &localaddr)->sin_addr.s_addr);
	sscanf(rem_addr, "%X",
	       &((struct sockaddr_in *) &remaddr)->sin_addr.s_addr);
	((struct sockaddr *) &localaddr)->sa_family = AF_INET;
	((struct sockaddr *) &remaddr)->sa_family = AF_INET;
    }

    retr = 0L;
    if (!flag_opt)
	more[0] = '\0';

    if (num < 10) {
	fprintf(stderr, _("warning, got bogus udp line.\n"));
	return;
    }
    if ((ap = get_afntype(((struct sockaddr *) &localaddr)->sa_family)) == NULL) {
	fprintf(stderr, _("netstat: unsupported address family %d !\n"),
		((struct sockaddr *) &localaddr)->sa_family);
	return;
    }
    switch (state) {
    case TCP_ESTABLISHED:
	udp_state = _("ESTABLISHED");
	break;

    case TCP_CLOSE:
	udp_state = "";
	break;

    default:
	udp_state = _("UNKNOWN");
	break;
    }

#if HAVE_AFINET6
#define notnull(A) (((A.sin6_family == AF_INET6) && \
	 ((A.sin6_addr.s6_addr32[0]) ||            \
	  (A.sin6_addr.s6_addr32[1]) ||            \
	  (A.sin6_addr.s6_addr32[2]) ||            \
	  (A.sin6_addr.s6_addr32[3]))) ||          \
	((A.sin6_family == AF_INET) &&             \
	 ((struct sockaddr_in *) &A)->sin_addr.s_addr))
#else
#define notnull(A) (A.sin_addr.s_addr)
#endif

    if (flag_all || (notnull(remaddr) && !flag_lst) || (!notnull(remaddr) && flag_lst))
    {
        safe_strncpy(local_addr, ap->sprint((struct sockaddr *) &localaddr, 
					    flag_not), sizeof(local_addr));
	snprintf(buffer, sizeof(buffer), "%s",
		 get_sname(htons(local_port), "udp",
			   flag_not & FLAG_NUM_PORT));
	if ((strlen(local_addr) + strlen(buffer)) > 22)
	    local_addr[22 - strlen(buffer)] = '\0';
	strcat(local_addr, ":");
	strcat(local_addr, buffer);

	snprintf(buffer, sizeof(buffer), "%s",
		 get_sname(htons(rem_port), "udp", flag_not & FLAG_NUM_PORT));
        safe_strncpy(rem_addr, ap->sprint((struct sockaddr *) &remaddr, 
					  flag_not), sizeof(rem_addr));
	if ((strlen(rem_addr) + strlen(buffer)) > 22)
	    rem_addr[22 - strlen(buffer)] = '\0';
	strcat(rem_addr, ":");
	strcat(rem_addr, buffer);

	timers[0] = '\0';
	if (flag_opt)
	    switch (timer_run) {
	    case 0:
		snprintf(timers, sizeof(timers), _("off (0.00/%ld/%d)"), retr, timeout);
		break;

	    case 1:
	    case 2:
		snprintf(timers, sizeof(timers), _("on%d (%2.2f/%ld/%d)"), timer_run, (double) time_len / 100, retr, timeout);
		break;

	    default:
		snprintf(timers, sizeof(timers), _("unkn-%d (%2.2f/%ld/%d)"), timer_run, (double) time_len / 100,
			 retr, timeout);
		break;
	    }
	printf("udp   %6ld %6ld %-23s %-23s %-12s",
	       rxq, txq, local_addr, rem_addr, udp_state);

	finish_this_one(uid,inode,timers);
    }
}

static int udp_info(void)
{
    INFO_GUTS6(_PATH_PROCNET_UDP, _PATH_PROCNET_UDP6, "AF INET (udp)",
	       udp_do_one);
}

static void raw_do_one(int lnr, const char *line)
{
    char buffer[8192], local_addr[64], rem_addr[64];
    char timers[64], more[512];
    int num, local_port, rem_port, d, state, timer_run, uid, timeout;
#if HAVE_AFINET6
    struct sockaddr_in6 localaddr, remaddr;
    char addr6[INET6_ADDRSTRLEN];
    struct in6_addr in6;
    extern struct aftype inet6_aftype;
#else
    struct sockaddr_in localaddr, remaddr;
#endif
    struct aftype *ap;
    unsigned long rxq, txq, time_len, retr, inode;

    if (lnr == 0)
	return;

    more[0] = '\0';
    num = sscanf(line,
		 "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %ld %512s\n",
		 &d, local_addr, &local_port, rem_addr, &rem_port, &state,
	  &txq, &rxq, &timer_run, &time_len, &retr, &uid, &timeout, &inode, more);

    if (strlen(local_addr) > 8) {
#if HAVE_AFINET6
	sscanf(local_addr, "%08X%08X%08X%08X",
	       &in6.s6_addr32[0], &in6.s6_addr32[1],
           &in6.s6_addr32[2], &in6.s6_addr32[3]);
    inet_ntop(AF_INET6, &in6, addr6, sizeof(addr6));
	inet6_aftype.input(1, addr6, (struct sockaddr *) &localaddr);
	sscanf(rem_addr, "%08X%08X%08X%08X",
	       &in6.s6_addr32[0], &in6.s6_addr32[1],
           &in6.s6_addr32[2], &in6.s6_addr32[3]);
    inet_ntop(AF_INET6, &in6, addr6, sizeof(addr6));
	inet6_aftype.input(1, addr6, (struct sockaddr *) &remaddr);
	localaddr.sin6_family = AF_INET6;
	remaddr.sin6_family = AF_INET6;
#endif
    } else {
	sscanf(local_addr, "%X",
	       &((struct sockaddr_in *) &localaddr)->sin_addr.s_addr);
	sscanf(rem_addr, "%X",
	       &((struct sockaddr_in *) &remaddr)->sin_addr.s_addr);
	((struct sockaddr *) &localaddr)->sa_family = AF_INET;
	((struct sockaddr *) &remaddr)->sa_family = AF_INET;
    }
#if HAVE_AFINET6
    if ((ap = get_afntype(localaddr.sin6_family)) == NULL) {
	fprintf(stderr, _("netstat: unsupported address family %d !\n"), localaddr.sin6_family);
	return;
    }
#else
    if ((ap = get_afntype(localaddr.sin_family)) == NULL) {
	fprintf(stderr, _("netstat: unsupported address family %d !\n"), localaddr.sin_family);
	return;
    }
#endif

    if (!flag_opt)
	more[0] = '\0';

    if (num < 10) {
	fprintf(stderr, _("warning, got bogus raw line.\n"));
	return;
    }

    if (flag_all || (notnull(remaddr) && !flag_lst) || (!notnull(remaddr) && flag_lst))
    {
	snprintf(buffer, sizeof(buffer), "%s",
		 get_sname(htons(local_port), "raw",
			   flag_not & FLAG_NUM_PORT));
        safe_strncpy(local_addr, ap->sprint((struct sockaddr *) &localaddr, 
					    flag_not), sizeof(local_addr));
	if ((strlen(local_addr) + strlen(buffer)) > 22)
	    local_addr[22 - strlen(buffer)] = '\0';
	strcat(local_addr, ":");
	strcat(local_addr, buffer);

	snprintf(buffer, sizeof(buffer), "%s",
		 get_sname(htons(rem_port), "raw", flag_not & FLAG_NUM_PORT));
        safe_strncpy(rem_addr, ap->sprint((struct sockaddr *) &remaddr, 
					  flag_not), sizeof(rem_addr));
	if ((strlen(rem_addr) + strlen(buffer)) > 22)
	    rem_addr[22 - strlen(buffer)] = '\0';
	strcat(rem_addr, ":");
	strcat(rem_addr, buffer);

	timers[0] = '\0';
	if (flag_opt)
	    switch (timer_run) {
	    case 0:
		snprintf(timers, sizeof(timers), _("off (0.00/%ld/%d)"), retr, timeout);
		break;

	    case 1:
            case 2:
		snprintf(timers, sizeof(timers), _("on%d (%2.2f/%ld/%d)"), timer_run, (double) time_len / 100,
			 retr, timeout);
		break;

	    default:
		snprintf(timers, sizeof(timers), _("unkn-%d (%2.2f/%ld/%d)"),
			 timer_run, (double) time_len / 100,
			 retr, timeout);
		break;
	    }
	printf("raw   %6ld %6ld %-23s %-23s %-12d",
	       rxq, txq, local_addr, rem_addr, state);

	finish_this_one(uid,inode,timers);
    }
}

static int raw_info(void)
{
    INFO_GUTS6(_PATH_PROCNET_RAW, _PATH_PROCNET_RAW6, "AF INET (raw)",
	       raw_do_one);
}

#endif


#if HAVE_AFUNIX

#define HAS_INODE 1

static void unix_do_one(int nr, const char *line)
{
    static int has = 0;
    char path[MAXPATHLEN], ss_flags[32];
    char *ss_proto, *ss_state, *ss_type;
    int num, state, type, inode;
    void *d;
    unsigned long refcnt, proto, flags;

    if (nr == 0) {
	if (strstr(line, "Inode"))
	    has |= HAS_INODE;
	return;
    }
    path[0] = '\0';
    num = sscanf(line, "%p: %lX %lX %lX %X %X %d %s",
		 &d, &refcnt, &proto, &flags, &type, &state, &inode, path);
    if (num < 6) {
	fprintf(stderr, _("warning, got bogus unix line.\n"));
	return;
    }
    if (!(has & HAS_INODE))
	snprintf(path,sizeof(path),"%d",inode);

    if (!flag_all) {
    	if ((state == SS_UNCONNECTED) && (flags & SO_ACCEPTCON)) {
    		if (!flag_lst)
    			return;
    	} else {
    		if (flag_lst)
    			return;
    	}
    }

    switch (proto) {
    case 0:
	ss_proto = "unix";
	break;

    default:
	ss_proto = "??";
    }

    switch (type) {
    case SOCK_STREAM:
	ss_type = _("STREAM");
	break;

    case SOCK_DGRAM:
	ss_type = _("DGRAM");
	break;

    case SOCK_RAW:
	ss_type = _("RAW");
	break;

    case SOCK_RDM:
	ss_type = _("RDM");
	break;

    case SOCK_SEQPACKET:
	ss_type = _("SEQPACKET");
	break;

    default:
	ss_type = _("UNKNOWN");
    }

    switch (state) {
    case SS_FREE:
	ss_state = _("FREE");
	break;

    case SS_UNCONNECTED:
	/*
	 * Unconnected sockets may be listening
	 * for something.
	 */
	if (flags & SO_ACCEPTCON) {
	    ss_state = _("LISTENING");
	} else {
	    ss_state = "";
	}
	break;

    case SS_CONNECTING:
	ss_state = _("CONNECTING");
	break;

    case SS_CONNECTED:
	ss_state = _("CONNECTED");
	break;

    case SS_DISCONNECTING:
	ss_state = _("DISCONNECTING");
	break;

    default:
	ss_state = _("UNKNOWN");
    }

    strcpy(ss_flags, "[ ");
    if (flags & SO_ACCEPTCON)
	strcat(ss_flags, "ACC ");
    if (flags & SO_WAITDATA)
	strcat(ss_flags, "W ");
    if (flags & SO_NOSPACE)
	strcat(ss_flags, "N ");

    strcat(ss_flags, "]");

    printf("%-5s %-6ld %-11s %-10s %-13s ",
	   ss_proto, refcnt, ss_flags, ss_type, ss_state);
    if (has & HAS_INODE)
	printf("%-6d ",inode);
    else
	printf("-      ");
    if (flag_prg)
	printf("%-" PROGNAME_WIDTHs "s",(has & HAS_INODE?prg_cache_get(inode):"-"));
    puts(path);
}

static int unix_info(void)
{

    printf(_("Active UNIX domain sockets "));
    if (flag_all)
	printf(_("(servers and established)"));
    else {
      if (flag_lst)
	printf(_("(only servers)"));
      else
	printf(_("(w/o servers)"));
    }

    printf(_("\nProto RefCnt Flags       Type       State         I-Node"));
    print_progname_banner();
    printf(_(" Path\n"));	/* xxx */

    {
	INFO_GUTS(_PATH_PROCNET_UNIX, "AF UNIX", unix_do_one);
    }
}
#endif


#if HAVE_AFAX25
static int ax25_info(void)
{
    FILE *f;
    char buffer[256], buf[16];
    char *src, *dst, *dev, *p;
    int st, vs, vr, sendq, recvq, ret;
    int new = -1;		/* flag for new (2.1.x) kernels */
    static char *ax25_state[5] =
    {
	N_("LISTENING"),
	N_("SABM SENT"),
	N_("DISC SENT"),
	N_("ESTABLISHED"),
	N_("RECOVERY")
    };
    if (!(f = fopen(_PATH_PROCNET_AX25, "r"))) {
	if (errno != ENOENT) {
	    perror(_PATH_PROCNET_AX25);
	    return (-1);
	}
	if (flag_arg || flag_ver)
	    ESYSNOT("netstat", "AF AX25");
	if (flag_arg)
	    return (1);
	else
	    return (0);
    }
    printf(_("Active AX.25 sockets\n"));
    printf(_("Dest       Source     Device  State        Vr/Vs    Send-Q  Recv-Q\n"));
    while (fgets(buffer, 256, f)) {
	if (new == -1) {
	    if (!strncmp(buffer, "dest_addr", 9)) {
		new = 0;
		continue;	/* old kernels have a header line */
	    } else
		new = 1;
	}
	/*
	 * In a network connection with no user socket the Snd-Q, Rcv-Q
	 * and Inode fields are empty in 2.0.x and '*' in 2.1.x
	 */
	sendq = 0;
	recvq = 0;
	if (new == 0) {
	    dst = buffer;
	    src = buffer + 10;
	    dst[9] = 0;
	    src[9] = 0;
	    ret = sscanf(buffer + 20, "%s %d %d %d %*d %*d/%*d %*d/%*d %*d/%*d %*d/%*d %*d/%*d %*d %*d %*d %d %d %*d",
		   buf, &st, &vs, &vr, &sendq, &recvq);
	    if (ret != 4 && ret != 6) {
		printf(_("Problem reading data from %s\n"), _PATH_PROCNET_AX25);
		continue;
	    }
	    dev = buf;
	} else {
	    p = buffer;
	    while (*p != ' ') p++;
	    p++;
	    dev = p;
	    while (*p != ' ') p++;
	    *p++ = 0;
	    src = p;
	    while (*p != ' ') p++;
	    *p++ = 0;
	    dst = p;
	    while (*p != ' ') p++;
	    *p++ = 0;
	    ret = sscanf(p, "%d %d %d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %d %d %*d",
		   &st, &vs, &vr, &sendq, &recvq);
	    if (ret != 3 && ret != 5) {
		    printf(_("problem reading data from %s\n"), _PATH_PROCNET_AX25);
		    continue;
	    }
	    /*
	     * FIXME: digipeaters should be handled somehow.
	     * For now we just strip them.
	     */
	    p = dst;
	    while (*p && *p != ',') p++;
	    *p = 0;
	}
	printf("%-9s  %-9s  %-6s  %-11s  %03d/%03d  %-6d  %-6d\n",
	       dst, src,
	       dev,
	       _(ax25_state[st]),
	       vr, vs, sendq, recvq);
    }
    fclose(f);
    return 0;
}
#endif


#if HAVE_AFIPX
static int ipx_info(void)
{
    FILE *f;
    char buf[256];
    unsigned long txq, rxq;
    unsigned int state;
    unsigned int uid;
    char *st;
    int nc;
    struct aftype *ap;
    struct passwd *pw;
    char sad[50], dad[50];
    struct sockaddr sa;
    unsigned sport = 0, dport = 0;

    if (!(f = fopen(_PATH_PROCNET_IPX, "r"))) {
	if (errno != ENOENT) {
	    perror(_PATH_PROCNET_IPX);
	    return (-1);
	}
	if (flag_arg || flag_ver)
	    ESYSNOT("netstat", "AF IPX");
	if (flag_arg)
	    return (1);
	else
	    return (0);
    }
    printf(_("Active IPX sockets\nProto Recv-Q Send-Q Local Address              Foreign Address            State"));	/* xxx */
    if (flag_exp > 1)
	printf(_(" User"));	/* xxx */
    printf("\n");
    if ((ap = get_afntype(AF_IPX)) == NULL) {
	EINTERN("netstat.c", "AF_IPX missing");
	return (-1);
    }
    fgets(buf, 255, f);

    while (fgets(buf, 255, f) != NULL) {
	sscanf(buf, "%s %s %lX %lX %d %d",
	       sad, dad, &txq, &rxq, &state, &uid);
	if ((st = rindex(sad, ':'))) {
	    *st++ = '\0';
	    sscanf(st, "%X", &sport);	/* net byt order */
	    sport = ntohs(sport);
	} else {
	    EINTERN("netstat.c", _PATH_PROCNET_IPX " sport format error");
	    return (-1);
	}
	nc = 0;
	if (strcmp(dad, "Not_Connected") != 0) {
	    if ((st = rindex(dad, ':'))) {
		*st++ = '\0';
		sscanf(st, "%X", &dport);	/* net byt order */
		dport = ntohs(dport);
	    } else {
		EINTERN("netstat.c", _PATH_PROCNET_IPX " dport format error");
		return (-1);
	    }
	} else
	    nc = 1;

	switch (state) {
	case TCP_ESTABLISHED:
	    st = _("ESTAB");
	    break;

	case TCP_CLOSE:
	    st = "";
	    break;

	default:
	    st = _("UNK.");
	    break;
	}

	/* Fetch and resolve the Source */
	(void) ap->input(4, sad, &sa);
	safe_strncpy(buf, ap->sprint(&sa, flag_not), sizeof(buf));
	snprintf(sad, sizeof(sad), "%s:%04X", buf, sport);

	if (!nc) {
	    /* Fetch and resolve the Destination */
	    (void) ap->input(4, dad, &sa);
	    safe_strncpy(buf, ap->sprint(&sa, flag_not), sizeof(buf));
	    snprintf(dad, sizeof(dad), "%s:%04X", buf, dport);
	} else
	    strcpy(dad, "-");

	printf("IPX   %6ld %6ld %-26s %-26s %-5s", txq, rxq, sad, dad, st);
	if (flag_exp > 1) {
	    if (!(flag_not & FLAG_NUM_USER) && ((pw = getpwuid(uid)) != NULL))
		printf(" %-10s", pw->pw_name);
	    else
		printf(" %-10d", uid);
	}
	printf("\n");
    }
    fclose(f);
    return 0;
}
#endif

static int iface_info(void)
{
    if (skfd < 0) {
	if ((skfd = sockets_open(0)) < 0) {
	    perror("socket");
	    exit(1);
	}
	printf(_("Kernel Interface table\n"));
    }
    if (flag_exp < 2) {
	ife_short = 1;
	printf(_("Iface   MTU Met   RX-OK RX-ERR RX-DRP RX-OVR   TX-OK TX-ERR TX-DRP TX-OVR Flg\n"));
    }

    if (for_all_interfaces(do_if_print, &flag_all) < 0) {
	perror(_("missing interface information"));
	exit(1);
    }
    if (flag_cnt)
	free_interface_list();
    else {
	close(skfd);
	skfd = -1;
    }

    return 0;
}


static void version(void)
{
    printf("%s\n%s\n%s\n%s\n", Release, Version, Signature, Features);
    exit(E_VERSION);
}


static void usage(void)
{
    fprintf(stderr, _("usage: netstat [-veenNcCF] [<Af>] -r         netstat {-V|--version|-h|--help}\n"));
    fprintf(stderr, _("       netstat [-vnNcaeol] [<Socket> ...]\n"));
    fprintf(stderr, _("       netstat { [-veenNac] -i | [-cnNe] -M | -s }\n\n"));

    fprintf(stderr, _("        -r, --route              display routing table\n"));
    fprintf(stderr, _("        -i, --interfaces         display interface table\n"));
    fprintf(stderr, _("        -g, --groups             display multicast group memberships\n"));
    fprintf(stderr, _("        -s, --statistics         display networking statistics (like SNMP)\n"));
#if HAVE_FW_MASQUERADE
    fprintf(stderr, _("        -M, --masquerade         display masqueraded connections\n\n"));
#endif
    fprintf(stderr, _("        -v, --verbose            be verbose\n"));
    fprintf(stderr, _("        -n, --numeric            don't resolve names\n"));
    fprintf(stderr, _("        --numeric-hosts          don't resolve host names\n"));
    fprintf(stderr, _("        --numeric-ports          don't resolve port names\n"));
    fprintf(stderr, _("        --numeric-users          don't resolve user names\n"));
    fprintf(stderr, _("        -N, --symbolic           resolve hardware names\n"));
    fprintf(stderr, _("        -e, --extend             display other/more information\n"));
    fprintf(stderr, _("        -p, --programs           display PID/Program name for sockets\n"));
    fprintf(stderr, _("        -c, --continuous         continuous listing\n\n"));
    fprintf(stderr, _("        -l, --listening          display listening server sockets\n"));
    fprintf(stderr, _("        -a, --all, --listening   display all sockets (default: connected)\n"));
    fprintf(stderr, _("        -o, --timers             display timers\n"));
    fprintf(stderr, _("        -F, --fib                display Forwarding Information Base (default)\n"));
    fprintf(stderr, _("        -C, --cache              display routing cache instead of FIB\n\n"));

    fprintf(stderr, _("  <Socket>={-t|--tcp} {-u|--udp} {-w|--raw} {-x|--unix} --ax25 --ipx --netrom\n"));
    fprintf(stderr, _("  <AF>=Use '-A <af>' or '--<af>'; default: %s\n"), DFLT_AF);
    fprintf(stderr, _("  List of possible address families (which support routing):\n"));
    print_aflist(1); /* 1 = routeable */
    exit(E_USAGE);
}


int main
 (int argc, char *argv[]) {
    int i;
    int lop;
    struct option longopts[] =
    {
	AFTRANS_OPTS,
	{"version", 0, 0, 'V'},
	{"interfaces", 0, 0, 'i'},
	{"help", 0, 0, 'h'},
	{"route", 0, 0, 'r'},
#if HAVE_FW_MASQUERADE
	{"masquerade", 0, 0, 'M'},
#endif
	{"protocol", 1, 0, 'A'},
	{"tcp", 0, 0, 't'},
	{"udp", 0, 0, 'u'},
	{"raw", 0, 0, 'w'},
	{"unix", 0, 0, 'x'},
	{"listening", 0, 0, 'l'},
	{"all", 0, 0, 'a'},
	{"timers", 0, 0, 'o'},
	{"continuous", 0, 0, 'c'},
	{"extend", 0, 0, 'e'},
	{"programs", 0, 0, 'p'},
	{"verbose", 0, 0, 'v'},
	{"statistics", 0, 0, 's'},
	{"numeric", 0, 0, 'n'},
	{"numeric-hosts", 0, 0, '!'},
	{"numeric-ports", 0, 0, '@'},
	{"numeric-users", 0, 0, '#'},
	{"symbolic", 0, 0, 'N'},
	{"cache", 0, 0, 'C'},
	{"fib", 0, 0, 'F'},
	{"groups", 0, 0, 'g'},
	{NULL, 0, 0, 0}
    };

#if I18N
    setlocale (LC_ALL, "");
    bindtextdomain("net-tools", "/usr/share/locale");
    textdomain("net-tools");
#endif
    getroute_init();		/* Set up AF routing support */

    afname[0] = '\0';
    while ((i = getopt_long(argc, argv, "MCFA:acdegphinNorstuVv?wxl", longopts, &lop)) != EOF)
	switch (i) {
	case -1:
	    break;
	case 1:
	    if (lop < 0 || lop >= AFTRANS_CNT) {
		EINTERN("netstat.c", "longopts 1 range");
		break;
	    }
	    if (aftrans_opt(longopts[lop].name))
		exit(1);
	    break;
	case 'A':
	    if (aftrans_opt(optarg))
		exit(1);
	    break;
	case 'M':
	    flag_mas++;
	    break;
	case 'a':
	    flag_all++;
	    break;
	case 'l':
	    flag_lst++;
	    break;
	case 'c':
	    flag_cnt++;
	    break;

	case 'd':
	    flag_deb++;
	    break;
	case 'g':
	    flag_igmp++;
	    break;
	case 'e':
	    flag_exp++;
	    break;
	case 'p':
	    flag_prg++;
	    break;
	case 'i':
	    flag_int++;
	    break;
	case 'n':
	    flag_not |= FLAG_NUM;
	    break;
	case '!':
	    flag_not |= FLAG_NUM_HOST;
	    break;
	case '@':
	    flag_not |= FLAG_NUM_PORT;
	    break;
	case '#':
	    flag_not |= FLAG_NUM_USER;
	    break;
	case 'N':
	    flag_not |= FLAG_SYM;
	    break;
	case 'C':
	    flag_cf |= FLAG_CACHE;
	    break;
	case 'F':
	    flag_cf |= FLAG_FIB;
	    break;
	case 'o':
	    flag_opt++;
	    break;
	case 'V':
	    version();
	    /*NOTREACHED */
	case 'v':
	    flag_ver |= FLAG_VERBOSE;
	    break;
	case 'r':
	    flag_rou++;
	    break;

	case 't':
	    flag_tcp++;
	    break;

	case 'u':
	    flag_udp++;
	    break;
	case 'w':
	    flag_raw++;
	    break;
	case 'x':
	    if (aftrans_opt("unix"))
		exit(1);
	    break;
	case '?':
	case 'h':
	    usage();
	case 's':
	    flag_sta++;
	}

    if (flag_int + flag_rou + flag_mas + flag_sta > 1)
	usage();

    if ((flag_inet || flag_inet6 || flag_sta) && !(flag_tcp || flag_udp || flag_raw))
	flag_tcp = flag_udp = flag_raw = 1;

    if ((flag_tcp || flag_udp || flag_raw || flag_igmp) && !(flag_inet || flag_inet6))
        flag_inet = flag_inet6 = 1;

    flag_arg = flag_tcp + flag_udp + flag_raw + flag_unx + flag_ipx
	+ flag_ax25 + flag_netrom + flag_igmp + flag_x25;

    if (flag_mas) {
#if HAVE_FW_MASQUERADE && HAVE_AFINET
#if MORE_THAN_ONE_MASQ_AF
	if (!afname[0])
	    strcpy(afname, DFLT_AF);
#endif
	for (;;) {
	    i = ip_masq_info(flag_not & FLAG_NUM_HOST,
			     flag_not & FLAG_NUM_PORT, flag_exp);
	    if (i || !flag_cnt)
		break;
	    sleep(1);
	}
#else
	ENOSUPP("netstat.c", "FW_MASQUERADE");
	i = -1;
#endif
	return (i);
    }

    if (flag_sta) {
        inittab();
	parsesnmp(flag_raw, flag_tcp, flag_udp);
	exit(0);
    }
    
    if (flag_rou) {
	int options = 0;

	if (!afname[0])
	    strcpy(afname, DFLT_AF);

	if (flag_exp == 2)
	    flag_exp = 1;
	else if (flag_exp == 1)
	    flag_exp = 2;

	options = (flag_exp & FLAG_EXT) | flag_not | flag_cf | flag_ver;
	if (!flag_cf)
	    options |= FLAG_FIB;

	for (;;) {
	    i = route_info(afname, options);
	    if (i || !flag_cnt)
		break;
	    sleep(1);
	}
	return (i);
    }
    if (flag_int) {
	for (;;) {
	    i = iface_info();
	    if (!flag_cnt || i)
		break;
	    sleep(1);
	}
	return (i);
    }
    for (;;) {
	if (!flag_arg || flag_tcp || flag_udp || flag_raw) {
#if HAVE_AFINET
	    prg_cache_load();
	    printf(_("Active Internet connections "));	/* xxx */

	    if (flag_all)
		printf(_("(servers and established)"));
	    else {
	      if (flag_lst)
		printf(_("(only servers)"));
	      else
		printf(_("(w/o servers)"));
	    }
	    printf(_("\nProto Recv-Q Send-Q Local Address           Foreign Address         State      "));	/* xxx */
	    if (flag_exp > 1)
		printf(_(" User       Inode     "));
	    print_progname_banner();
	    if (flag_opt)
		printf(_(" Timer"));	/* xxx */
	    printf("\n");
#else
	    if (flag_arg) {
		i = 1;
		ENOSUPP("netstat", "AF INET");
	    }
#endif
	}
#if HAVE_AFINET
	if (!flag_arg || flag_tcp) {
	    i = tcp_info();
	    if (i)
		return (i);
	}
	if (!flag_arg || flag_udp) {
	    i = udp_info();
	    if (i)
		return (i);
	}
	if (!flag_arg || flag_raw) {
	    i = raw_info();
	    if (i)
		return (i);
	}

	if (flag_igmp) {
#if HAVE_AFINET6
	    printf( "IPv6/");
#endif
	    printf( _("IPv4 Group Memberships\n") );
	    printf( _("Interface       RefCnt Group\n") );
	    printf( "--------------- ------ ---------------------\n" );
	    i = igmp_info();
	    if (i)
	        return (i);
	}
#endif

	if (!flag_arg || flag_unx) {
#if HAVE_AFUNIX
	    prg_cache_load();
	    i = unix_info();
	    if (i)
		return (i);
#else
	    if (flag_arg) {
		i = 1;
		ENOSUPP("netstat", "AF UNIX");
	    }
#endif
	}
	if (!flag_arg || flag_ipx) {
#if HAVE_AFIPX
	    i = ipx_info();
	    if (i)
		return (i);
#else
	    if (flag_arg) {
		i = 1;
		ENOSUPP("netstat", "AF IPX");
	    }
#endif
	}
	if (!flag_arg || flag_ax25) {
#if HAVE_AFAX25
	    i = ax25_info();
	    if (i)
		return (i);
#else
	    if (flag_arg) {
		i = 1;
		ENOSUPP("netstat", "AF AX25");
	    }
#endif
	}
	if(!flag_arg || flag_x25) {
#if HAVE_AFX25
	    /* FIXME */
	    i = x25_info();
	    if (i)
		return(i);
#else
	    if (flag_arg) {
		i = 1;
		ENOSUPP("netstat", "AF X25");
	    }
#endif
	}
	if (!flag_arg || flag_netrom) {
#if HAVE_AFNETROM
	    i = netrom_info();
	    if (i)
		return (i);
#else
	    if (flag_arg) {
		i = 1;
		ENOSUPP("netstat", "AF NETROM");
	    }
#endif
	}
	if (!flag_cnt || i)
	    break;
	sleep(1);
	prg_cache_clear();
    }
    return (i);
}
