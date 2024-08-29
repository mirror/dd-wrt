/*
 * netstat    This file contains an implementation of the command
 *              that helps in debugging the networking modules.
 *
 * NET-TOOLS    A collection of programs that form the base set of the
 *              NET-3 Networking Distribution for the LINUX operating
 *              system.
 *
 * Version:     $Id: netstat.c,v 1.73 2011-04-20 01:35:22 ecki Exp $
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
 *20081201 {1.42} Brian Micek           added -L|--udplite options for RFC 3828
 *20020722 {1.51} Thomas Preusser       added SCTP over IPv4 support
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
#include <sys/stat.h>
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
#include "proc.h"

#if HAVE_SELINUX
#include <selinux/selinux.h>
#endif

#if HAVE_AFBLUETOOTH
#include <bluetooth/bluetooth.h>
#endif

#define PROGNAME_WIDTH 20
#define SELINUX_WIDTH 50

#if !defined(s6_addr32) && defined(in6a_words)
#define s6_addr32 in6a_words	/* libinet6			*/
#endif

/* prototypes for statistics.c */
void parsesnmp(int, int, int, int);
void parsesnmp6(int, int, int);

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

static char *Release = RELEASE, *Signature = "Fred Baumgarten, Alan Cox, Bernd Eckenfels, Phil Blundell, Tuan Hoang, Brian Micek and others";


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
int flag_sctp= 0;
int flag_udp = 0;
int flag_udplite = 0;
int flag_igmp= 0;
int flag_rom = 0;
int flag_exp = 1;
int flag_wide= 0;
int flag_prg = 0;
int flag_arg = 0;
int flag_noprot = 0;
int flag_ver = 0;
int flag_l2cap = 0;
int flag_rfcomm = 0;
int flag_selinux = 0;

FILE *procinfo;

#define INFO_GUTS1(file,name,proc,prot)			\
  procinfo = proc_fopen((file));			\
  if (procinfo == NULL) {				\
    if (errno != ENOENT && errno != EACCES) {		\
      perror((file));					\
      return -1;					\
    }							\
    if (!flag_noprot && (flag_arg || flag_ver))		\
      ESYSNOT("netstat", (name));			\
    if (!flag_noprot && flag_arg)			\
      rc = 1;						\
  } else {						\
    do {						\
      if (fgets(buffer, sizeof(buffer), procinfo))	\
        (proc)(lnr++, buffer,prot);			\
    } while (!feof(procinfo));				\
    fclose(procinfo);					\
  }

#if HAVE_AFINET6
#define INFO_GUTS2(file,proc,prot)			\
  lnr = 0;						\
  procinfo = proc_fopen((file));		       	\
  if (procinfo != NULL) {				\
    do {						\
      if (fgets(buffer, sizeof(buffer), procinfo))	\
	(proc)(lnr++, buffer,prot);			\
    } while (!feof(procinfo));				\
    fclose(procinfo);					\
  }
#else
#define INFO_GUTS2(file,proc,prot)
#endif

#define INFO_GUTS3					\
 return rc;

#define INFO_GUTS6(file,file6,name,proc,prot4,prot6)	\
 char buffer[8192];					\
 int rc = 0;						\
 int lnr = 0;						\
 if (!flag_arg || flag_inet) {				\
    INFO_GUTS1(file,name,proc,prot4)			\
 }							\
 if (!flag_arg || flag_inet6) {				\
    INFO_GUTS2(file6,proc,prot6)			\
 }							\
 INFO_GUTS3

#define INFO_GUTS(file,name,proc,prot)			\
 char buffer[8192];					\
 int rc = 0;						\
 int lnr = 0;						\
 INFO_GUTS1(file,name,proc,prot)			\
 INFO_GUTS3

#define PROGNAME_WIDTHs PROGNAME_WIDTH1(PROGNAME_WIDTH)
#define PROGNAME_WIDTH1(s) PROGNAME_WIDTH2(s)
#define PROGNAME_WIDTH2(s) #s

#define SELINUX_WIDTHs SELINUX_WIDTH1(SELINUX_WIDTH)
#define SELINUX_WIDTH1(s) SELINUX_WIDTH2(s)
#define SELINUX_WIDTH2(s) #s

#define PRG_HASH_SIZE 211

static struct prg_node {
    struct prg_node *next;
    unsigned long inode;
    char name[PROGNAME_WIDTH];
    char scon[SELINUX_WIDTH];
} *prg_hash[PRG_HASH_SIZE];

static char prg_cache_loaded = 0;

#define PRG_HASHIT(x) ((x) % PRG_HASH_SIZE)

#define PROGNAME_BANNER "PID/Program name"
#define SELINUX_BANNER "Security Context"

#define print_progname_banner() do { if (flag_prg) printf(" %-" PROGNAME_WIDTHs "s",PROGNAME_BANNER); } while (0)

#define print_selinux_banner() do { if (flag_selinux) printf("%-" SELINUX_WIDTHs "s"," " SELINUX_BANNER); } while (0)

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

static void prg_cache_add(unsigned long inode, char *name, const char *scon)
{
    unsigned hi = PRG_HASHIT(inode);
    struct prg_node **pnp,*pn;

    prg_cache_loaded = 2;
    for (pnp = prg_hash + hi; (pn = *pnp); pnp = &pn->next) {
	if (pn->inode == inode) {
	    /* Some warning should be appropriate here
	       as we got multiple processes for one i-node */
	    return;
	}
    }
    if (!(*pnp = malloc(sizeof(**pnp))))
	return;
    pn = *pnp;
    pn->next = NULL;
    pn->inode = inode;
    safe_strncpy(pn->name, name, sizeof(pn->name));

    {
	int len = (strlen(scon) - sizeof(pn->scon)) + 1;
	if (len > 0)
            safe_strncpy(pn->scon, &scon[len + 1], sizeof(pn->scon));
	else
            safe_strncpy(pn->scon, scon, sizeof(pn->scon));
    }

}

static const char *prg_cache_get(unsigned long inode)
{
    unsigned hi = PRG_HASHIT(inode);
    struct prg_node *pn;

    for (pn = prg_hash[hi]; pn; pn = pn->next)
	if (pn->inode == inode)
	    return (pn->name);
    return ("-");
}

static const char *prg_cache_get_con(unsigned long inode)
{
    unsigned hi = PRG_HASHIT(inode);
    struct prg_node *pn;

    for (pn = prg_hash[hi]; pn; pn = pn->next)
	if (pn->inode == inode)
	    return (pn->scon);
    return ("-");
}

static void prg_cache_clear(void)
{
    struct prg_node **pnp,*pn;

    if (prg_cache_loaded == 2)
	for (pnp = prg_hash; pnp < prg_hash + PRG_HASH_SIZE; pnp++)
	    while ((pn = *pnp)) {
		*pnp = pn->next;
		free(pn);
	    }
    prg_cache_loaded = 0;
}

static void wait_continous(void)
{
    fflush(stdout);
    sleep(1);
}

static int extract_type_1_socket_inode(const char lname[], unsigned long * inode_p) {

    /* If lname is of the form "socket:[12345]", extract the "12345"
       as *inode_p.  Otherwise, return -1 as *inode_p.
       */

    if (strlen(lname) < PRG_SOCKET_PFXl+3) return(-1);

    if (memcmp(lname, PRG_SOCKET_PFX, PRG_SOCKET_PFXl)) return(-1);
    if (lname[strlen(lname)-1] != ']') return(-1);

    {
        char inode_str[strlen(lname + 1)];  /* e.g. "12345" */
        const int inode_str_len = strlen(lname) - PRG_SOCKET_PFXl - 1;
        char *serr;

        strncpy(inode_str, lname+PRG_SOCKET_PFXl, inode_str_len);
        inode_str[inode_str_len] = '\0';
        *inode_p = strtoul(inode_str, &serr, 0);
        if (!serr || *serr || *inode_p == ~0)
            return(-1);
    }
    return(0);
}



static int extract_type_2_socket_inode(const char lname[], unsigned long * inode_p) {

    /* If lname is of the form "[0000]:12345", extract the "12345"
       as *inode_p.  Otherwise, return -1 as *inode_p.
       */

    if (strlen(lname) < PRG_SOCKET_PFX2l+1) return(-1);
    if (memcmp(lname, PRG_SOCKET_PFX2, PRG_SOCKET_PFX2l)) return(-1);

    {
        char *serr;

        *inode_p = strtoul(lname + PRG_SOCKET_PFX2l, &serr, 0);
        if (!serr || *serr || *inode_p == ~0)
            return(-1);
    }
    return(0);
}




static void prg_cache_load(void)
{
    char line[LINE_MAX], eacces=0;
    int procfdlen, fd, cmdllen, lnamelen;
    char lname[30], cmdlbuf[512], finbuf[PROGNAME_WIDTH];
    unsigned long inode;
    const char *cs, *cmdlp;
    DIR *dirproc = NULL, *dirfd = NULL;
    struct dirent *direproc, *direfd;
#if HAVE_SELINUX
    security_context_t scon = NULL;
#endif

    if (prg_cache_loaded || !flag_prg) return;
    prg_cache_loaded = 1;
    cmdlbuf[sizeof(cmdlbuf) - 1] = '\0';
    if (!(dirproc=opendir(PATH_PROC))) goto fail;
    while (errno = 0, direproc = readdir(dirproc)) {
	for (cs = direproc->d_name; *cs; cs++)
	    if (!isdigit(*cs))
		break;
	if (*cs)
	    continue;
	procfdlen = snprintf(line,sizeof(line),PATH_PROC_X_FD,direproc->d_name);
	if (procfdlen <= 0 || procfdlen >= sizeof(line) - 5)
	    continue;
	errno = 0;
	dirfd = opendir(line);
	if (! dirfd) {
	    if (errno == EACCES)
		eacces = 1;
	    continue;
	}
	line[procfdlen] = '/';
	cmdlp = NULL;
	while ((direfd = readdir(dirfd))) {
           /* Skip . and .. */
           if (!isdigit(direfd->d_name[0]))
               continue;
	    if (procfdlen + 1 + strlen(direfd->d_name) + 1 > sizeof(line))
		continue;
	    memcpy(line + procfdlen - PATH_FD_SUFFl, PATH_FD_SUFF "/",
		   PATH_FD_SUFFl + 1);
        safe_strncpy(line + procfdlen + 1, direfd->d_name,
                        sizeof(line) - procfdlen - 1);
	    lnamelen = readlink(line, lname, sizeof(lname) - 1);
	    if (lnamelen == -1)
		    continue;
            lname[lnamelen] = '\0';  /*make it a null-terminated string*/

            if (extract_type_1_socket_inode(lname, &inode) < 0)
              if (extract_type_2_socket_inode(lname, &inode) < 0)
                continue;

	    if (!cmdlp) {
		if (procfdlen - PATH_FD_SUFFl + PATH_CMDLINEl >=
		    sizeof(line) - 5)
		    continue;
        safe_strncpy(line + procfdlen - PATH_FD_SUFFl, PATH_CMDLINE,
                        sizeof(line) - procfdlen + PATH_FD_SUFFl);
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
		if (cmdlbuf[0] == '/' && (cmdlp = strrchr(cmdlbuf, '/')))
		    cmdlp++;
		else
		    cmdlp = cmdlbuf;
	    }

	    snprintf(finbuf, sizeof(finbuf), "%s/%s", direproc->d_name, cmdlp);
#if HAVE_SELINUX
	    if (getpidcon(atoi(direproc->d_name), &scon) == -1) {
		    scon=xstrdup("-");
	    }
	    prg_cache_add(inode, finbuf, scon);
	    freecon(scon);
#else
	    prg_cache_add(inode, finbuf, "-");
#endif
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

    f = proc_fopen(_PATH_PROCNET_NR);
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
    if (fgets(buffer, 256, f))
	/* eat line */;

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

#if HAVE_AFROSE
static const char * const rose_state[] =
{
    N_("LISTENING"),
    N_("CONN SENT"),
    N_("DISC SENT"),
    N_("ESTABLISHED"),
};

static int rose_info(void)
{
    FILE *f;
    char buffer[256], dev[6];
    int ret, st, lci, neigh;
    char src_addr[10], src_call[9], dest_addr[10], dest_call[9];

    f = fopen(_PATH_PROCNET_ROSE, "r");
    if (f == NULL) {
	if (errno != ENOENT) {
	    perror(_PATH_PROCNET_ROSE);
	    return (-1);
	}
	if (flag_arg || flag_ver)
	    ESYSNOT("netstat", "AF ROSE");
	if (flag_arg)
	    return (1);
	else
	    return (0);
    }
    printf(_("Active ROSE sockets\n"));
    printf(_("dest_addr   dest_call  src_addr    src_call  dev   lci neigh   state\n"));
    if (fgets(buffer, 256, f))
	/* eat line */;

    while (fgets(buffer, 256, f)) {
	ret = sscanf(buffer, "%s %s %s %s %s %d %d %d",
		dest_addr, dest_call, src_addr, src_call, dev, &lci, &neigh, &st);
	if (ret != 8) {
	    printf(_("Problem reading data from %s\n"), _PATH_PROCNET_ROSE);
	    continue;
	}
	printf("%-10s  %-9s  %-10s  %-9s %-5s %3d %5d   %s\n",
		dest_addr, dest_call, src_addr, src_call, dev, lci, neigh, _(rose_state[st]));
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
	    printf(" %-10s ", pw->pw_name);
	else
	    printf(" %-10d ", uid);
	printf("%-10lu",inode);
    }
    if (flag_prg)
	printf(" %-" PROGNAME_WIDTHs "s",prg_cache_get(inode));
    if (flag_selinux)
	printf(" %-" SELINUX_WIDTHs "s",prg_cache_get_con(inode));

    if (flag_opt)
	printf(" %s", timers);
    putchar('\n');
}

static void igmp_do_one(int lnr, const char *line,const char *prot)
{
    char mcast_addr[128];
    struct sockaddr_storage sas;
    struct sockaddr_in *sin = (struct sockaddr_in *)&sas;
#if HAVE_AFINET6
    char addr6[INET6_ADDRSTRLEN];
    struct in6_addr in6;
    extern struct aftype inet6_aftype;
#endif
    const struct aftype *ap;
    static int idx_flag = 0;
    static int igmp6_flag = 0;
    static char device[16];
    int num, idx, refcnt;
    char* offset;

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
	    inet6_aftype.input(1, addr6, &sas);
	    sas.ss_family = AF_INET6;
	} else {
	    fprintf(stderr, _("warning, got bogus igmp6 line %d.\n"), lnr);
	    return;
	}

	if ((ap = get_afntype(sas.ss_family)) == NULL) {
	    fprintf(stderr, _("netstat: unsupported address family %d !\n"),
		    sas.ss_family);
	    return;
	}
	safe_strncpy(mcast_addr, ap->sprint(&sas, flag_not & FLAG_NUM_HOST),
		sizeof(mcast_addr));
	printf("%-15s %-6d %s\n", device, refcnt, mcast_addr);
#endif
    } else {    /* IPV4 */
#if HAVE_AFINET
	if (line[0] != '\t') {
	    if (idx_flag) {
		if ((num = sscanf(line, "%d\t%15c", &idx, device)) < 2) {
		    fprintf(stderr, _("warning, got bogus igmp line %d.\n"), lnr);
		    return;
		}
	    } else {
		if ((num = sscanf(line, "%15c", device)) < 1 ) {
		    fprintf(stderr, _("warning, got bogus igmp line %d.\n"), lnr);
		    return;
		}
	    }

	    offset = strrchr(device, ':');
	    if (offset)
		*offset = 0;

	    return;
	} else if ( line[0] == '\t' ) {
	    if ( (num = sscanf(line, "\t%8[0-9A-Fa-f] %d", mcast_addr, &refcnt)) < 2 ) {
		fprintf(stderr, _("warning, got bogus igmp line %d.\n"), lnr);
		return;
	    }
	    sscanf(mcast_addr, "%X", &sin->sin_addr.s_addr);
	    sas.ss_family = AF_INET;
	} else {
	    fprintf(stderr, _("warning, got bogus igmp line %d.\n"), lnr);
	    return;
	}

	if ((ap = get_afntype(sas.ss_family)) == NULL) {
	    fprintf(stderr, _("netstat: unsupported address family %d !\n"),
		    sas.ss_family);
	    return;
	}
	safe_strncpy(mcast_addr, ap->sprint(&sas, flag_not & FLAG_NUM_HOST),
		sizeof(mcast_addr));
	printf("%-15s %-6d %s\n", device, refcnt, mcast_addr );
#endif
    }    /* IPV4 */
}

#if HAVE_AFX25
static int x25_info(void)
{
       FILE *f=proc_fopen(_PATH_PROCNET_X25);
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
       if(!f)
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
       if (fgets(buffer,256,f))
               /* eat line */;
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
	       igmp_do_one, "igmp", "igmp6");
}

static const char *sctp_socket_state_str(int state)
{
    if (state >= 0 && state < ARRAY_SIZE(tcp_state))
	return tcp_state[state];
    else {
	static char state_str_buf[64];
	sprintf(state_str_buf, "UNKNOWN(%d)", state);
	return state_str_buf;
    }
}

static const struct aftype *process_sctp_addr_str(const char *addr_str, struct sockaddr_storage *sas)
{
    if (strchr(addr_str,':')) {
#if HAVE_AFINET6
	extern struct aftype inet6_aftype;
	/* Demangle what the kernel gives us */
	struct in6_addr in6;
	char addr6_str[INET6_ADDRSTRLEN];
	unsigned u0, u1, u2, u3, u4, u5, u6, u7;
	sscanf(addr_str, "%04X:%04X:%04X:%04X:%04X:%04X:%04X:%04X",
	       &u0, &u1, &u2, &u3, &u4, &u5, &u6, &u7);
	in6.s6_addr16[0] = htons(u0);
	in6.s6_addr16[1] = htons(u1);
	in6.s6_addr16[2] = htons(u2);
	in6.s6_addr16[3] = htons(u3);
	in6.s6_addr16[4] = htons(u4);
	in6.s6_addr16[5] = htons(u5);
	in6.s6_addr16[6] = htons(u6);
	in6.s6_addr16[7] = htons(u7);

	inet_ntop(AF_INET6, &in6, addr6_str, sizeof(addr6_str));
	inet6_aftype.input(1, addr6_str, sas);
	sas->ss_family = AF_INET6;
#endif
    } else {
	struct sockaddr_in *sin = (struct sockaddr_in *)sas;
	sin->sin_addr.s_addr = inet_addr(addr_str);
	sas->ss_family = AF_INET;
    }
    return get_afntype(sas->ss_family);
}

static void sctp_eps_do_one(int lnr, char *line, const char *proto)
{
    char buffer[1024];
    int state, port;
    int uid;
    unsigned long inode;
    const struct aftype *ap;
    struct sockaddr_storage localsas;
    const char *sst_str;
    const char *lport_str;
    const char *uid_str;
    const char *inode_str;
    char *laddrs_str;

    if (lnr == 0) {
	/* ENDPT     SOCK   STY SST HBKT LPORT   UID INODE LADDRS */
	return;
    }
    strtok(line, " \t\n");	/* skip endpt */
    strtok(0, " \t\n");		/* skip sock */
    strtok(0, " \t\n");		/* skip sty */
    sst_str = strtok(0, " \t\n");
    strtok(0, " \t\n");		/* skip hash bucket */
    lport_str = strtok(0, " \t\n");
    uid_str = strtok(0, " \t\n");
    inode_str = strtok(0, " \t\n");
    laddrs_str = strtok(0, "\t\n");

    if (!sst_str || !lport_str || !uid_str || !inode_str) {
	fprintf(stderr, _("warning, got bogus sctp eps line.\n"));
	return;
    }
    state = atoi(sst_str);
    port = atoi(lport_str);
    uid = atoi(uid_str);
    inode = strtoul(inode_str,0,0);

    const char *this_local_addr;
    int first = 1;
    char local_port[16];
    snprintf(local_port, sizeof(local_port), "%s",
        get_sname(htons(port), proto, flag_not & FLAG_NUM_PORT));
    for (this_local_addr = strtok(laddrs_str, " \t\n");
         this_local_addr;
         this_local_addr = strtok(0, " \t\n")) {
	char local_addr[64];
	ap = process_sctp_addr_str(this_local_addr, &localsas);
	if (ap)
	    safe_strncpy(local_addr, ap->sprint(&localsas, flag_not), sizeof(local_addr));
	else
	    sprintf(local_addr, _("unsupported address family %d"), localsas.ss_family);

	if (first)
	    printf("sctp                ");
	else
	    printf("\n                    ");
	sprintf(buffer, "%s:%s", local_addr, local_port);
	printf("%-47s", buffer);
	printf(" %-11s", first ? sctp_socket_state_str(state) : "");
	first = 0;
    }
    finish_this_one(uid, inode, "");
}

static void sctp_assoc_do_one(int lnr, char *line, const char *proto)
{
    char buffer[1024];
    int state, lport,rport;
    int uid;
    unsigned rxqueue,txqueue;
    unsigned long inode;

    const struct aftype *ap;
    struct sockaddr_storage localsas, remotesas;
    const char *sst_str;
    const char *txqueue_str;
    const char *rxqueue_str;
    const char *lport_str, *rport_str;
    const char *uid_str;
    const char *inode_str;
    char *laddrs_str;
    char *raddrs_str;

    if (lnr == 0) {
	/* ASSOC     SOCK   STY SST ST HBKT ASSOC-ID TX_QUEUE RX_QUEUE UID INODE LPORT RPORT LADDRS <-> RADDRS */
	return;
    }

    strtok(line, " \t\n");	/* skip assoc */
    strtok(0, " \t\n");		/* skip sock */
    strtok(0, " \t\n");		/* skip sty */
    sst_str = strtok(0, " \t\n");
    strtok(0, " \t\n");
    strtok(0, " \t\n");		/* skip hash bucket */
    strtok(0, " \t\n");		/* skip hash assoc-id */
    txqueue_str =  strtok(0, " \t\n");
    rxqueue_str =  strtok(0, " \t\n");
    uid_str = strtok(0, " \t\n");
    inode_str = strtok(0, " \t\n");
    lport_str = strtok(0, " \t\n");
    rport_str = strtok(0, " \t\n");
    laddrs_str = strtok(0, "<->\t\n");
    raddrs_str = strtok(0, "<->\t\n");

    if (!sst_str || !txqueue_str || !rxqueue_str || !uid_str ||
        !inode_str || !lport_str || !rport_str) {
	fprintf(stderr, _("warning, got bogus sctp assoc line.\n"));
	return;
    }

    state = atoi(sst_str);
    txqueue = atoi(txqueue_str);
    rxqueue = atoi(rxqueue_str);
    uid = atoi(uid_str);
    inode = strtoul(inode_str, 0, 0);
    lport = atoi(lport_str);
    rport = atoi(rport_str);

    /*print all addresses*/
    const char *this_local_addr;
    const char *this_remote_addr;
    char *ss1, *ss2;
    int first = 1;
    char local_port[16];
    char remote_port[16];
    snprintf(local_port, sizeof(local_port), "%s",
             get_sname(htons(lport), proto,
             flag_not & FLAG_NUM_PORT));
    snprintf(remote_port, sizeof(remote_port), "%s",
             get_sname(htons(rport), proto,
             flag_not & FLAG_NUM_PORT));

    this_local_addr = strtok_r(laddrs_str, " \t\n", &ss1);
    this_remote_addr = strtok_r(raddrs_str, " \t\n", &ss2);
    while (this_local_addr || this_remote_addr) {
	char local_addr[64];
	char remote_addr[64];

	if (this_local_addr) {
	    if (this_local_addr[0] == '*') {
		/* skip * */
		this_local_addr++;
	    }
	    ap = process_sctp_addr_str(this_local_addr, &localsas);
	    if (ap)
		safe_strncpy(local_addr,
		             ap->sprint(&localsas, flag_not), sizeof(local_addr));
	    else
		sprintf(local_addr, _("unsupported address family %d"), localsas.ss_family);
	}
	if (this_remote_addr) {
	    if (this_remote_addr[0] == '*') {
		/* skip * */
		this_remote_addr++;
	    }
	    ap = process_sctp_addr_str(this_remote_addr, &remotesas);
	    if (ap)
		safe_strncpy(remote_addr,
		             ap->sprint(&remotesas, flag_not), sizeof(remote_addr));
	    else
		sprintf(remote_addr, _("unsupported address family %d"), remotesas.ss_family);
	}

	if (first)
	    printf("sctp  %6u %6u ", rxqueue, txqueue);
	else
	    printf("\n                    ");
	if (this_local_addr) {
	    if (first)
		sprintf(buffer, "%s:%s", local_addr, local_port);
	    else
		sprintf(buffer, "%s", local_addr);
	    printf("%-23s", buffer);
	} else
	    printf("%-23s", "");
	printf(" ");
	if (this_remote_addr) {
	    if (first)
		sprintf(buffer, "%s:%s", remote_addr, remote_port);
	    else
		sprintf(buffer, "%s", remote_addr);
	    printf("%-23s", buffer);
	} else
	    printf("%-23s", "");

       printf(" %-11s", first ? sctp_socket_state_str(state) : "");

       first = 0;
       this_local_addr = strtok_r(0, " \t\n", &ss1);
       this_remote_addr = strtok_r(0, " \t\n", &ss2);
    }
    finish_this_one(uid, inode, "");
}

static int sctp_info_eps(void)
{
    INFO_GUTS6(_PATH_PROCNET_SCTPEPTS, _PATH_PROCNET_SCTP6EPTS, "AF INET (sctp)",
               sctp_eps_do_one, "sctp", "sctp6");
}

static int sctp_info_assocs(void)
{
    INFO_GUTS6(_PATH_PROCNET_SCTPASSOCS, _PATH_PROCNET_SCTP6ASSOCS, "AF INET (sctp)",
               sctp_assoc_do_one, "sctp", "sctp6");
}

static int sctp_info(void)
{
    int res;

    if (flag_all || flag_lst) {
	res = sctp_info_eps();
	if (res)
	    return res;
    }

    if (flag_all || !flag_lst) {
	res = sctp_info_assocs();
    }

    return res;
}

static void addr_do_one(char *buf, size_t buf_len, size_t short_len, const struct aftype *ap,
			const struct sockaddr_storage *addr,
			int port, const char *proto
)
{
    const char *sport, *saddr;
    size_t port_len, addr_len;

    saddr = ap->sprint(addr, flag_not & FLAG_NUM_HOST);
    sport = get_sname(htons(port), proto, flag_not & FLAG_NUM_PORT);
    addr_len = strlen(saddr);
    port_len = strlen(sport);
    if (!flag_wide && (addr_len + port_len > short_len)) {
	/* Assume port name is short */
	port_len = netmin(port_len, short_len - 4);
	addr_len = short_len - port_len;
	strncpy(buf, saddr, addr_len);
	buf[addr_len] = '\0';
	strcat(buf, ":");
	strncat(buf, sport, port_len);
    } else
	snprintf(buf, buf_len, "%s:%s", saddr, sport);
}

static void tcp_do_one(int lnr, const char *line, const char *prot)
{
    unsigned long rxq, txq, time_len, retr, inode;
    int num, local_port, rem_port, d, state, uid, timer_run, timeout;
    char rem_addr[128], local_addr[128], timers[64];
    const struct aftype *ap;
    struct sockaddr_storage localsas, remsas;
    struct sockaddr_in *localaddr = (struct sockaddr_in *)&localsas;
    struct sockaddr_in *remaddr = (struct sockaddr_in *)&remsas;
#if HAVE_AFINET6
    char addr6[INET6_ADDRSTRLEN];
    struct in6_addr in6;
    extern struct aftype inet6_aftype;
#endif
    long clk_tck = ticks_per_second();

    if (lnr == 0)
	return;

    num = sscanf(line,
    "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %lu %*s\n",
		 &d, local_addr, &local_port, rem_addr, &rem_port, &state,
		 &txq, &rxq, &timer_run, &time_len, &retr, &uid, &timeout, &inode);

    if (num < 11) {
	fprintf(stderr, _("warning, got bogus tcp line.\n"));
	return;
    }

    if (!flag_all && ((flag_lst && rem_port) || (!flag_lst && !rem_port)))
      return;

    if (strlen(local_addr) > 8) {
#if HAVE_AFINET6
	/* Demangle what the kernel gives us */
	sscanf(local_addr, "%08X%08X%08X%08X",
	       &in6.s6_addr32[0], &in6.s6_addr32[1],
           &in6.s6_addr32[2], &in6.s6_addr32[3]);
	inet_ntop(AF_INET6, &in6, addr6, sizeof(addr6));
	inet6_aftype.input(1, addr6, &localsas);
	sscanf(rem_addr, "%08X%08X%08X%08X",
	       &in6.s6_addr32[0], &in6.s6_addr32[1],
	       &in6.s6_addr32[2], &in6.s6_addr32[3]);
	inet_ntop(AF_INET6, &in6, addr6, sizeof(addr6));
	inet6_aftype.input(1, addr6, &remsas);
	localsas.ss_family = AF_INET6;
	remsas.ss_family = AF_INET6;
#endif
    } else {
	sscanf(local_addr, "%X", &localaddr->sin_addr.s_addr);
	sscanf(rem_addr, "%X", &remaddr->sin_addr.s_addr);
	localsas.ss_family = AF_INET;
	remsas.ss_family = AF_INET;
    }

    if ((ap = get_afntype(localsas.ss_family)) == NULL) {
	fprintf(stderr, _("netstat: unsupported address family %d !\n"),
		localsas.ss_family);
	return;
    }

	addr_do_one(local_addr, sizeof(local_addr), 22, ap, &localsas, local_port, "tcp");
	addr_do_one(rem_addr, sizeof(rem_addr), 22, ap, &remsas, rem_port, "tcp");

	timers[0] = '\0';
	if (flag_opt)
	    switch (timer_run) {
	    case 0:
		snprintf(timers, sizeof(timers), _("off (0.00/%ld/%d)"), retr, timeout);
		break;

	    case 1:
		snprintf(timers, sizeof(timers), _("on (%2.2f/%ld/%d)"),
			 (double) time_len / clk_tck, retr, timeout);
		break;

	    case 2:
		snprintf(timers, sizeof(timers), _("keepalive (%2.2f/%ld/%d)"),
			 (double) time_len / clk_tck, retr, timeout);
		break;

	    case 3:
		snprintf(timers, sizeof(timers), _("timewait (%2.2f/%ld/%d)"),
			 (double) time_len / clk_tck, retr, timeout);
		break;

	    case 4:
		snprintf(timers, sizeof(timers), _("probe (%2.2f/%ld/%d)"),
			 (double) time_len / clk_tck, retr, timeout);
		break;

	    default:
		snprintf(timers, sizeof(timers), _("unkn-%d (%2.2f/%ld/%d)"),
			 timer_run, (double) time_len / clk_tck, retr, timeout);
		break;
	    }

	printf("%-4s  %6ld %6ld %-*s %-*s %-11s",
	       prot, rxq, txq, (int)netmax(23,strlen(local_addr)), local_addr, (int)netmax(23,strlen(rem_addr)), rem_addr, _(tcp_state[state]));

	finish_this_one(uid,inode,timers);
}

static int tcp_info(void)
{
    INFO_GUTS6(_PATH_PROCNET_TCP, _PATH_PROCNET_TCP6, "AF INET (tcp)",
	       tcp_do_one, "tcp", "tcp6");
}

static int notnull(const struct sockaddr_storage *sas)
{
    const struct sockaddr_in *sin = (const struct sockaddr_in *)sas;

#if HAVE_AFINET6
    const struct sockaddr_in6 *sin6 = (const struct sockaddr_in6 *)sas;

    if (sin6->sin6_family == AF_INET6) {
	return sin6->sin6_addr.s6_addr32[0] ||
		sin6->sin6_addr.s6_addr32[1] ||
		sin6->sin6_addr.s6_addr32[2] ||
		sin6->sin6_addr.s6_addr32[3];
    }
#endif

    return sin->sin_addr.s_addr;
}

static void udp_do_one(int lnr, const char *line,const char *prot)
{
    char local_addr[128], rem_addr[128];
    char *udp_state, timers[64];
    int num, local_port, rem_port, d, state, timer_run, uid, timeout;
    struct sockaddr_storage localsas, remsas;
    struct sockaddr_in *localaddr = (struct sockaddr_in *)&localsas;
    struct sockaddr_in *remaddr = (struct sockaddr_in *)&remsas;
#if HAVE_AFINET6
    char addr6[INET6_ADDRSTRLEN];
    struct in6_addr in6;
    extern struct aftype inet6_aftype;
#endif
    const struct aftype *ap;
    unsigned long rxq, txq, time_len, retr, inode;

    if (lnr == 0)
	return;

    num = sscanf(line,
		 "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %lu %*s\n",
		 &d, local_addr, &local_port,
		 rem_addr, &rem_port, &state,
	  &txq, &rxq, &timer_run, &time_len, &retr, &uid, &timeout, &inode);

    if (num < 10) {
	fprintf(stderr, _("warning, got bogus udp line.\n"));
	return;
    }

    if (strlen(local_addr) > 8) {
#if HAVE_AFINET6
	sscanf(local_addr, "%08X%08X%08X%08X",
	       &in6.s6_addr32[0], &in6.s6_addr32[1],
	       &in6.s6_addr32[2], &in6.s6_addr32[3]);
	inet_ntop(AF_INET6, &in6, addr6, sizeof(addr6));
	inet6_aftype.input(1, addr6, &localsas);
	sscanf(rem_addr, "%08X%08X%08X%08X",
	       &in6.s6_addr32[0], &in6.s6_addr32[1],
	       &in6.s6_addr32[2], &in6.s6_addr32[3]);
	inet_ntop(AF_INET6, &in6, addr6, sizeof(addr6));
	inet6_aftype.input(1, addr6, &remsas);
	localsas.ss_family = AF_INET6;
	remsas.ss_family = AF_INET6;
#endif
    } else {
	sscanf(local_addr, "%X", &localaddr->sin_addr.s_addr);
	sscanf(rem_addr, "%X", &remaddr->sin_addr.s_addr);
	localsas.ss_family = AF_INET;
	remsas.ss_family = AF_INET;
    }

    retr = 0L;

    if ((ap = get_afntype(localsas.ss_family)) == NULL) {
	fprintf(stderr, _("netstat: unsupported address family %d !\n"),
		localsas.ss_family);
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

    if (flag_all || (notnull(&remsas) && !flag_lst) || (!notnull(&remsas) && flag_lst))
    {
	addr_do_one(local_addr, sizeof(local_addr), 22, ap, &localsas, local_port, "udp");
	addr_do_one(rem_addr, sizeof(rem_addr), 22, ap, &remsas, rem_port, "udp");

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
	printf("%-5s %6ld %6ld %-23s %-23s %-11s",
	       prot, rxq, txq, local_addr, rem_addr, udp_state);

	finish_this_one(uid,inode,timers);
    }
}

static int udp_info(void)
{
    INFO_GUTS6(_PATH_PROCNET_UDP, _PATH_PROCNET_UDP6, "AF INET (udp)",
	       udp_do_one, "udp", "udp6");
}

static int udplite_info(void)
{
    INFO_GUTS6(_PATH_PROCNET_UDPLITE, _PATH_PROCNET_UDPLITE6,
               "AF INET (udplite)", udp_do_one, "udpl", "udpl6" );
}

static void raw_do_one(int lnr, const char *line,const char *prot)
{
    char local_addr[128], rem_addr[128];
    char timers[64];
    int num, local_port, rem_port, d, state, timer_run, uid, timeout;
    struct sockaddr_storage localsas, remsas;
    struct sockaddr_in *localaddr = (struct sockaddr_in *)&localsas;
    struct sockaddr_in *remaddr = (struct sockaddr_in *)&remsas;
#if HAVE_AFINET6
    char addr6[INET6_ADDRSTRLEN];
    struct in6_addr in6;
    extern struct aftype inet6_aftype;
#endif
    const struct aftype *ap;
    unsigned long rxq, txq, time_len, retr, inode;

    if (lnr == 0)
	return;

    num = sscanf(line,
		 "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %lu %*s\n",
		 &d, local_addr, &local_port, rem_addr, &rem_port, &state,
	  &txq, &rxq, &timer_run, &time_len, &retr, &uid, &timeout, &inode);

    if (num < 10) {
    	fprintf(stderr, _("warning, got bogus raw line.\n"));
	return;
    }

    if (strlen(local_addr) > 8) {
#if HAVE_AFINET6
	sscanf(local_addr, "%08X%08X%08X%08X",
	       &in6.s6_addr32[0], &in6.s6_addr32[1],
           &in6.s6_addr32[2], &in6.s6_addr32[3]);
    inet_ntop(AF_INET6, &in6, addr6, sizeof(addr6));
	inet6_aftype.input(1, addr6, &localsas);
	sscanf(rem_addr, "%08X%08X%08X%08X",
	       &in6.s6_addr32[0], &in6.s6_addr32[1],
           &in6.s6_addr32[2], &in6.s6_addr32[3]);
    inet_ntop(AF_INET6, &in6, addr6, sizeof(addr6));
	inet6_aftype.input(1, addr6, &remsas);
	localsas.ss_family = AF_INET6;
	remsas.ss_family = AF_INET6;
#endif
    } else {
	sscanf(local_addr, "%X", &localaddr->sin_addr.s_addr);
	sscanf(rem_addr, "%X", &remaddr->sin_addr.s_addr);
	localsas.ss_family = AF_INET;
	remsas.ss_family = AF_INET;
    }

    if ((ap = get_afntype(localsas.ss_family)) == NULL) {
	fprintf(stderr, _("netstat: unsupported address family %d !\n"), localsas.ss_family);
	return;
    }

    if (flag_all || (notnull(&remsas) && !flag_lst) || (!notnull(&remsas) && flag_lst))
    {
	addr_do_one(local_addr, sizeof(local_addr), 22, ap, &localsas, local_port, "raw");
	addr_do_one(rem_addr, sizeof(rem_addr), 22, ap, &remsas, rem_port, "raw");

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
	printf("%-4s  %6ld %6ld %-23s %-23s %-11d",
	       prot, rxq, txq, local_addr, rem_addr, state);

	finish_this_one(uid,inode,timers);
    }
}

static int raw_info(void)
{
    INFO_GUTS6(_PATH_PROCNET_RAW, _PATH_PROCNET_RAW6, "AF INET (raw)",
	       raw_do_one, "raw", "raw6");
}

#endif


#if HAVE_AFUNIX

#define HAS_INODE 1

static void unix_do_one(int nr, const char *line, const char *prot)
{
    static int has = 0;
    char path[MAXPATHLEN], ss_flags[32];
    char *ss_proto, *ss_state, *ss_type;
    int num, state, type;
    void *d;
    unsigned long refcnt, proto, flags, inode;

    if (nr == 0) {
	if (strstr(line, "Inode"))
	    has |= HAS_INODE;
	return;
    }
    path[0] = '\0';
    num = sscanf(line, "%p: %lX %lX %lX %X %X %lu %s",
		 &d, &refcnt, &proto, &flags, &type, &state, &inode, path);
    if (num < 6) {
	fprintf(stderr, _("warning, got bogus unix line.\n"));
	return;
    }
    if (!(has & HAS_INODE))
	snprintf(path,sizeof(path),"%lu",inode);

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

    safe_strncpy(ss_flags, "[ ", sizeof(ss_flags));
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
	printf("%-8lu",inode);
    else
	printf("-       ");
    if (flag_prg)
	printf(" %-" PROGNAME_WIDTHs "s",(has & HAS_INODE?prg_cache_get(inode):"-"));
    if (flag_selinux)
	printf(" %-" SELINUX_WIDTHs "s",(has & HAS_INODE?prg_cache_get_con(inode):"-"));

    printf(" %s\n", path);
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

    printf(_("\nProto RefCnt Flags       Type       State         I-Node  "));
    print_progname_banner();
    print_selinux_banner();
    printf(_(" Path\n"));	/* xxx */

    {
	INFO_GUTS(_PATH_PROCNET_UNIX, "AF UNIX", unix_do_one, "unix");
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
    if (!(f = proc_fopen(_PATH_PROCNET_AX25))) {
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
    const struct aftype *ap;
    struct passwd *pw;
    char sad[50], dad[50];
    struct sockaddr_storage sa;
    unsigned sport = 0, dport = 0;
    struct stat s;

    f = proc_fopen(_PATH_PROCNET_IPX_SOCKET1);
    if (!f) {
        if (errno != ENOENT) {
            perror(_PATH_PROCNET_IPX_SOCKET1);
            return (-1);
        }
        f = proc_fopen(_PATH_PROCNET_IPX_SOCKET2);

        /* We need to check for directory */
        if (f) {
            if (fstat (fileno(f), &s) == -1 ||
                !S_ISREG(s.st_mode)) {
                fclose(f);
                f=NULL;
            }
        }

        if (!f) {
            if (errno != ENOENT) {
	        perror(_PATH_PROCNET_IPX_SOCKET2);
	        return (-1);
	    }
	    if (flag_arg || flag_ver)
	        ESYSNOT("netstat", "AF IPX");
	    if (flag_arg)
	        return (1);
 	    else
	        return (0);
        }
    }
    printf(_("Active IPX sockets\nProto Recv-Q Send-Q Local Address              Foreign Address            State"));	/* xxx */
    if (flag_exp > 1)
	printf(_(" User"));	/* xxx */
    printf("\n");
    if ((ap = get_afntype(AF_IPX)) == NULL) {
	EINTERN("netstat.c", "AF_IPX missing");
	fclose(f);
	return (-1);
    }
    if (fgets(buf, 255, f))
	/* eat line */;

    while (fgets(buf, 255, f) != NULL) {
	sscanf(buf, "%s %s %lX %lX %u %u",
	       sad, dad, &txq, &rxq, &state, &uid);
	if ((st = rindex(sad, ':'))) {
	    *st++ = '\0';
	    sscanf(st, "%X", &sport);	/* net byt order */
	    sport = ntohs(sport);
	} else {
	    EINTERN("netstat.c", "ipx socket format error in source port");
	    fclose(f);
	    return (-1);
	}
	nc = 0;
	if (strcmp(dad, "Not_Connected") != 0) {
	    if ((st = rindex(dad, ':'))) {
		*st++ = '\0';
		sscanf(st, "%X", &dport);	/* net byt order */
		dport = ntohs(dport);
	    } else {
		EINTERN("netstat.c", "ipx socket format error in destination port");
		fclose(f);
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
	(void) ap->input(0, sad, &sa);
	safe_strncpy(buf, ap->sprint(&sa, flag_not & FLAG_NUM_HOST), sizeof(buf));
	snprintf(sad, sizeof(sad), "%s:%04X", buf, sport);

	if (!nc) {
	    /* Fetch and resolve the Destination */
	    (void) ap->input(0, dad, &sa);
	    safe_strncpy(buf, ap->sprint(&sa, flag_not & FLAG_NUM_HOST), sizeof(buf));
	    snprintf(dad, sizeof(dad), "%s:%04X", buf, dport);
	} else
        safe_strncpy(dad, "-", sizeof(dad));

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

#if HAVE_AFBLUETOOTH
const char *bluetooth_state(int state)
{
    switch (state) {
	case BT_CONNECTED:
	    return _("CONNECTED");
	case BT_OPEN:
	    return _("OPEN");
	case BT_BOUND:
	    return _("BOUND");
	case BT_LISTEN:
	    return _("LISTEN");
	case BT_CONNECT:
	    return _("CONNECT");
	case BT_CONNECT2:
	    return _("CONNECT2");
	case BT_CONFIG:
	    return _("CONFIG");
	case BT_DISCONN:
	    return _("DISCONN");
	case BT_CLOSED:
	    return _("CLOSED");
	default:
	    return _("UNKNOWN");
    }
}

static void l2cap_do_one(int nr, const char *line, const char *prot)
{
    char daddr[18], saddr[18];
    unsigned dtype, stype, state, psm, dcid, scid, imtu, omtu, sec_level;
    int num;
    const char *bt_state, *bt_sec_level;

    num = sscanf(line, "%17s (%u) %17s (%u) %d %d 0x%04x 0x%04x %d %d %d",
	daddr, &dtype, saddr, &stype, &state, &psm, &dcid, &scid, &imtu, &omtu, &sec_level);

    if (num != 11) {
	num = sscanf(line, "%17s %17s %d %d 0x%04x 0x%04x %d %d %d",
	    daddr, saddr, &state, &psm, &dcid, &scid, &imtu, &omtu, &sec_level);

	if (num != 9) {
	    fprintf(stderr, _("warning, got bogus l2cap line.\n"));
	    return;
	}
    }

    if (flag_lst && !(state == BT_LISTEN || state == BT_BOUND))
	return;
    if (!(flag_all || flag_lst) && (state == BT_LISTEN || state == BT_BOUND))
	return;

    bt_state = bluetooth_state(state);
    switch (sec_level) {
	case BT_SECURITY_SDP:
	    bt_sec_level = _("SDP");
	    break;
	case BT_SECURITY_LOW:
	    bt_sec_level = _("LOW");
	    break;
	case BT_SECURITY_MEDIUM:
	    bt_sec_level = _("MEDIUM");
	    break;
	case BT_SECURITY_HIGH:
	    bt_sec_level = _("HIGH");
	    break;
	default:
	    bt_sec_level = _("UNKNOWN");
    }

    printf("l2cap  %-17s %-17s %-9s %7d 0x%04x 0x%04x %7d %7d %-7s\n",
	(strcmp (daddr, "00:00:00:00:00:00") == 0 ? "*" : daddr),
	(strcmp (saddr, "00:00:00:00:00:00") == 0 ? "*" : saddr),
	bt_state, psm, dcid, scid, imtu, omtu, bt_sec_level);
}

static int l2cap_info(void)
{
    printf("%-6s %-17s %-17s %-9s %7s %-6s %-6s %7s %7s %-7s\n",
	"Proto", "Destination", "Source", "State", "PSM", "DCID", "SCID", "IMTU", "OMTU", "Security");
    INFO_GUTS(_PATH_SYS_BLUETOOTH_L2CAP, "BTPROTO L2CAP", l2cap_do_one, "l2cap");
}

static void rfcomm_do_one(int nr, const char *line, const char *prot)
{
    char daddr[18], saddr[18];
    unsigned state, channel;
    int num;
    const char *bt_state;

    num = sscanf(line, "%17s %17s %d %d", daddr, saddr, &state, &channel);
    if (num < 4) {
	fprintf(stderr, _("warning, got bogus rfcomm line.\n"));
	return;
    }

    if (flag_lst && !(state == BT_LISTEN || state == BT_BOUND))
	return;
    if (!(flag_all || flag_lst) && (state == BT_LISTEN || state == BT_BOUND))
	return;

    bt_state = bluetooth_state(state);
    printf("rfcomm %-17s %-17s %-9s %7d\n",
	(strcmp (daddr, "00:00:00:00:00:00") == 0 ? "*" : daddr),
	(strcmp (saddr, "00:00:00:00:00:00") == 0 ? "*" : saddr),
	bt_state, channel);
}

static int rfcomm_info(void)
{
    printf("%-6s %-17s %-17s %-9s %7s\n", "Proto", "Destination", "Source", "State", "Channel");
    INFO_GUTS(_PATH_SYS_BLUETOOTH_RFCOMM, "BTPROTO RFCOMM", rfcomm_do_one, "rfcomm");
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
	printf(_("Iface             MTU    RX-OK RX-ERR RX-DRP RX-OVR    TX-OK TX-ERR TX-DRP TX-OVR Flg\n"));
    }

    if (for_all_interfaces(do_if_print, &flag_all) < 0) {
	perror(_("missing interface information"));
	exit(1);
    }
    if (flag_cnt)
	if_cache_free();
    else {
	close(skfd);
	skfd = -1;
    }

    return 0;
}


static void version(void)
{
    printf("%s\n%s\n%s\n", Release, Signature, Features);
    exit(E_VERSION);
}


static void usage(int rc)
{
    FILE *fp = rc ? stderr : stdout;
    fprintf(fp, _("usage: netstat [-vWeenNcCF] [<Af>] -r         netstat {-V|--version|-h|--help}\n"));
    fprintf(fp, _("       netstat [-vWnNcaeol] [<Socket> ...]\n"));
    fprintf(fp, _("       netstat { [-vWeenNac] -i | [-cnNe] -M | -s [-6tuw] }\n\n"));

    fprintf(fp, _("        -r, --route              display routing table\n"));
    fprintf(fp, _("        -i, --interfaces         display interface table\n"));
    fprintf(fp, _("        -g, --groups             display multicast group memberships\n"));
    fprintf(fp, _("        -s, --statistics         display networking statistics (like SNMP)\n"));
#if HAVE_FW_MASQUERADE
    fprintf(fp, _("        -M, --masquerade         display masqueraded connections\n\n"));
#endif

    fprintf(fp, _("        -v, --verbose            be verbose\n"));
    fprintf(fp, _("        -W, --wide               don't truncate IP addresses\n"));
    fprintf(fp, _("        -n, --numeric            don't resolve names\n"));
    fprintf(fp, _("        --numeric-hosts          don't resolve host names\n"));
    fprintf(fp, _("        --numeric-ports          don't resolve port names\n"));
    fprintf(fp, _("        --numeric-users          don't resolve user names\n"));
    fprintf(fp, _("        -N, --symbolic           resolve hardware names\n"));
    fprintf(fp, _("        -e, --extend             display other/more information\n"));
    fprintf(fp, _("        -p, --programs           display PID/Program name for sockets\n"));
    fprintf(fp, _("        -o, --timers             display timers\n"));
    fprintf(fp, _("        -c, --continuous         continuous listing\n\n"));
    fprintf(fp, _("        -l, --listening          display listening server sockets\n"));
    fprintf(fp, _("        -a, --all                display all sockets (default: connected)\n"));
    fprintf(fp, _("        -F, --fib                display Forwarding Information Base (default)\n"));
    fprintf(fp, _("        -C, --cache              display routing cache instead of FIB\n"));
#if HAVE_SELINUX
    fprintf(fp, _("        -Z, --context            display SELinux security context for sockets\n"));
#endif

    fprintf(fp, _("\n  <Socket>={-t|--tcp} {-u|--udp} {-U|--udplite} {-S|--sctp} {-w|--raw}\n"));
    fprintf(fp, _("           {-x|--unix} --ax25 --ipx --netrom\n"));
    fprintf(fp, _("  <AF>=Use '-6|-4' or '-A <af>' or '--<af>'; default: %s\n"), DFLT_AF);
    fprintf(fp, _("  List of possible address families (which support routing):\n"));
    print_aflist(1); /* 1 = routeable */
    exit(rc);
}


int main
 (int argc, char *argv[]) {
    int i;
    int lop;
    static struct option longopts[] =
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
	{"sctp", 0, 0, 'S'},
	{"udp", 0, 0, 'u'},
        {"udplite", 0, 0, 'U'},
	{"raw", 0, 0, 'w'},
	{"unix", 0, 0, 'x'},
	{"l2cap", 0, 0, '2'},
	{"rfcomm", 0, 0, 'f'},
	{"listening", 0, 0, 'l'},
	{"all", 0, 0, 'a'},
	{"timers", 0, 0, 'o'},
	{"continuous", 0, 0, 'c'},
	{"extend", 0, 0, 'e'},
	{"programs", 0, 0, 'p'},
	{"verbose", 0, 0, 'v'},
	{"statistics", 0, 0, 's'},
	{"wide", 0, 0, 'W'},
	{"numeric", 0, 0, 'n'},
	{"numeric-hosts", 0, 0, '!'},
	{"numeric-ports", 0, 0, '@'},
	{"numeric-users", 0, 0, '#'},
	{"symbolic", 0, 0, 'N'},
	{"cache", 0, 0, 'C'},
	{"fib", 0, 0, 'F'},
	{"groups", 0, 0, 'g'},
	{"context", 0, 0, 'Z'},
	{NULL, 0, 0, 0}
    };

#if I18N
    setlocale (LC_ALL, "");
    bindtextdomain("net-tools", "/usr/share/locale");
    textdomain("net-tools");
#endif
    getroute_init();		/* Set up AF routing support */

    afname[0] = '\0';
    while ((i = getopt_long(argc, argv, "A:CFMacdeghilnNoprsStuUvVWw2fx64?Z", longopts, &lop)) != EOF)
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
	case 'W':
	    flag_wide++;
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
	case '6':
	    if (aftrans_opt("inet6"))
		exit(1);
	    break;
	case '4':
	    if (aftrans_opt("inet"))
		exit(1);
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
	case 'S':
	    flag_sctp++;
	    break;
	case 'u':
	    flag_udp++;
	    break;
        case 'U':
	    flag_udplite++;
	    break;
	case 'w':
	    flag_raw++;
	    break;
        case '2':
	    flag_l2cap++;
	    break;
        case 'f':
	    flag_rfcomm++;
	    break;
	case 'x':
	    if (aftrans_opt("unix"))
		exit(1);
	    break;
	case 'Z':
#if HAVE_SELINUX
	    if (is_selinux_enabled() <= 0) {
		fprintf(stderr, _("SELinux is not enabled on this machine.\n"));
		exit(1);
	    }
	    flag_prg++;
	    flag_selinux++;
#else
            fprintf(stderr, _("SELinux is not enabled for this application.\n"));
	    exit(1);
#endif

	    break;
	case '?':
	    usage(E_OPTERR);
	case 'h':
	    usage(E_USAGE);
	case 's':
	    flag_sta++;
	}

    if (flag_int + flag_rou + flag_mas + flag_sta > 1)
	usage(E_OPTERR);

    if ((flag_inet || flag_inet6 || flag_sta) &&
        !(flag_tcp || flag_sctp || flag_udp || flag_udplite || flag_raw))
	   flag_noprot = flag_tcp = flag_sctp = flag_udp = flag_udplite = flag_raw = 1;

    if ((flag_tcp || flag_sctp || flag_udp || flag_udplite || flag_raw || flag_igmp) &&
        !(flag_inet || flag_inet6))
        flag_inet = flag_inet6 = 1;

    if (flag_bluetooth && !(flag_l2cap || flag_rfcomm))
	   flag_l2cap = flag_rfcomm = 1;

    flag_arg = flag_tcp + flag_sctp + flag_udplite + flag_udp + flag_raw + flag_unx
        + flag_ipx + flag_ax25 + flag_netrom + flag_igmp + flag_x25 + flag_rose
	+ flag_l2cap + flag_rfcomm;

    if (flag_mas) {
#if HAVE_FW_MASQUERADE && HAVE_AFINET
#if MORE_THAN_ONE_MASQ_AF
	if (!afname[0])
        safe_strncpy(afname, DFLT_AF, sizeof(afname));
#endif
	for (;;) {
	    i = ip_masq_info(flag_not & FLAG_NUM_HOST,
			     flag_not & FLAG_NUM_PORT, flag_exp);
	    if (i || !flag_cnt)
		break;
	    wait_continous();
	}
#else
	ENOSUPP("netstat", "FW_MASQUERADE");
	i = -1;
#endif
	return (i);
    }

    if (flag_sta) {
        if (!afname[0])
            safe_strncpy(afname, DFLT_AF, sizeof(afname));

        if (!strcmp(afname, "inet")) {
#if HAVE_AFINET
            parsesnmp(flag_raw, flag_tcp, flag_udp, flag_sctp);
#else
            ENOSUPP("netstat", "AF INET");
            exit(1);
#endif
        } else if(!strcmp(afname, "inet6")) {
#if HAVE_AFINET6
            parsesnmp6(flag_raw, flag_tcp, flag_udp);
#else
            ENOSUPP("netstat", "AF INET6");
            exit(1);
#endif
        } else {
          printf(_("netstat: No statistics support for specified address family: %s\n"), afname);
          exit(1);
        }
        exit(0);
    }

    if (flag_rou) {
	int options = 0;

	if (!afname[0])
        safe_strncpy(afname, DFLT_AF, sizeof(afname));

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
            wait_continous();
	}
	return (i);
    }
    if (flag_int) {
	for (;;) {
	    i = iface_info();
	    if (!flag_cnt || i)
		break;
            wait_continous();
	}
	return (i);
    }
    for (;;) {
	if (!flag_arg || flag_tcp || flag_sctp || flag_udp || flag_udplite || flag_raw) {
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
	    print_selinux_banner();
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

	if (!flag_arg || flag_sctp) {
	    i = sctp_info();
	    if (i)
		return (i);
	}

	if (!flag_arg || flag_udp) {
	    i = udp_info();
	    if (i)
		return (i);
	}

	if (!flag_arg || flag_udplite) {
	    i = udplite_info();
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
	if (!flag_arg || flag_rose) {
#if HAVE_AFROSE
          i = rose_info();
          if (i)
            return (i);
#else
          if (flag_arg) {
            i = 1;
            ENOSUPP("netstat", "AF ROSE");
          }
#endif
        }

	if (!flag_arg || flag_l2cap || flag_rfcomm) {
#if HAVE_AFBLUETOOTH
	    printf(_("Active Bluetooth connections "));	/* xxx */

	    if (flag_all)
		printf(_("(servers and established)"));
	    else {
	      if (flag_lst)
		printf(_("(only servers)"));
	      else
		printf(_("(w/o servers)"));
	    }
	    printf("\n");
#else
	    if (flag_arg) {
		i = 1;
		ENOSUPP("netstat", "AF BLUETOOTH");
	    }
#endif
	}
#if HAVE_AFBLUETOOTH
	if (!flag_arg || flag_l2cap) {
	    i = l2cap_info();
	    if (i)
		return (i);
	}
	if (!flag_arg || flag_rfcomm) {
	    i = rfcomm_info();
	    if (i)
		return (i);
	}
#endif

	if (!flag_cnt || i)
	    break;
        wait_continous();
	prg_cache_clear();
    }
    return (i);
}
