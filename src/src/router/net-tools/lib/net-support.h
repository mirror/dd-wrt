/*
 * lib/support.h      This file contains the definitions of what is in the
 *                      support library.  Most of all, it defines structures
 *                      for accessing support modules, and the function proto-
 *                      types.
 *
 * NET-LIB      A collection of functions used from the base set of the
 *              NET-3 Networking Distribution for the LINUX operating
 *              system. (net-tools, net-drivers)
 *
 * Version:     lib/net-support.h 1.35 (1996-01-01)
 *
 * Maintainer:  Bernd 'eckes' Eckenfels, <net-tools@lina.inka.de>
 *
 * Author:      Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
 *              Copyright 1993 MicroWalt Corporation
 *
 * Modifications:
 *960125 {1.20} Bernd Eckenfels:        reformated, layout
 *960202 {1.30} Bernd Eckenfels:        rprint in aftype
 *960206 {1.31} Bernd Eckenfels:        route_init
 *960219 {1.32} Bernd Eckenfels:        type for ap->input()
 *960322 {1.33} Bernd Eckenfels:        activate_ld and const in get_hwtype
 *960413 {1.34} Bernd Eckenfels:        new RTACTION suport
 *990101 {1.35} Bernd Eckenfels:	print_(hw|af)list support, added kerneldefines
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 */
#include <sys/socket.h>

/* This structure defines protocol families and their handlers. */
struct aftype {
    char *name;
    char *title;
    int af;
    int alen;
    char *(*print) (unsigned char *);
    char *(*sprint) (struct sockaddr *, int numeric);
    int (*input) (int type, char *bufp, struct sockaddr *);
    void (*herror) (char *text);
    int (*rprint) (int options);
    int (*rinput) (int typ, int ext, char **argv);

    /* may modify src */
    int (*getmask) (char *src, struct sockaddr * mask, char *name);

    int fd;
    char *flag_file;
};

extern struct aftype *aftypes[];

/* This structure defines hardware protocols and their handlers. */
struct hwtype {
    char *name;
    char *title;
    int type;
    int alen;
    char *(*print) (unsigned char *);
    int (*input) (char *, struct sockaddr *);
    int (*activate) (int fd);
    int suppress_null_addr;
};


extern struct hwtype *get_hwtype(const char *name);
extern struct hwtype *get_hwntype(int type);
extern void          print_hwlist(int type);
extern struct aftype *get_aftype(const char *name);
extern struct aftype *get_afntype(int type);
extern void          print_aflist(int type);
extern int           hw_null_address(struct hwtype *hw, void *addr);

extern int getargs(char *string, char *arguments[]);

extern int get_socket_for_af(int af);

extern void getroute_init(void);
extern void setroute_init(void);
extern void activate_init(void);
extern int route_info(const char *afname, int flags);
extern int route_edit(int action, const char *afname, int flags, char **argv);
extern int activate_ld(const char *hwname, int fd);

#define RTACTION_ADD   1
#define RTACTION_DEL   2
#define RTACTION_HELP  3
#define RTACTION_FLUSH 4
#define RTACTION_SHOW  5

#define FLAG_EXT       3		/* AND-Mask */
#define FLAG_NUM_HOST  4
#define FLAG_NUM_PORT  8
#define FLAG_NUM_USER 16
#define FLAG_NUM     (FLAG_NUM_HOST|FLAG_NUM_PORT|FLAG_NUM_USER)
#define FLAG_SYM      32
#define FLAG_CACHE    64
#define FLAG_FIB     128
#define FLAG_VERBOSE 256

extern int ip_masq_info(int numeric_host, int numeric_port, int ext);

extern int INET_rprint(int options);
extern int INET6_rprint(int options);
extern int DDP_rprint(int options);
extern int IPX_rprint(int options);
extern int NETROM_rprint(int options);
extern int AX25_rprint(int options);
extern int X25_rprint(int options);

extern int INET_rinput(int action, int flags, char **argv);
extern int INET6_rinput(int action, int flags, char **argv);
extern int DDP_rinput(int action, int flags, char **argv);
extern int IPX_rinput(int action, int flags, char **argv);
extern int NETROM_rinput(int action, int flags, char **argv);
extern int AX25_rinput(int action, int flags, char **argv);
extern int X25_rinput(int action, int flags, char **argv);

extern int aftrans_opt(const char *arg);
extern void aftrans_def(char *tool, char *argv0, char *dflt);

extern char *get_sname(int socknumber, char *proto, int numeric);

extern int flag_unx;
extern int flag_ipx;
extern int flag_ax25;
extern int flag_ddp;
extern int flag_netrom;
extern int flag_x25;
extern int flag_inet;
extern int flag_inet6;

extern char afname[];

#define AFTRANS_OPTS \
	{"ax25",	0,	0,	1}, \
       {"x25",         0,      0,      1}, \
	{"ip",		0,	0,	1}, \
	{"ipx",         0,	0,	1}, \
	{"appletalk",	0,	0,	1}, \
	{"netrom",	0,	0,	1}, \
	{"inet",	0,	0,	1}, \
	{"inet6",	0,	0,	1}, \
	{"ddp",		0,	0,	1}, \
	{"unix",	0,	0,	1}, \
	{"tcpip",	0,	0,	1}
#define AFTRANS_CNT 11

#define EINTERN(file, text) fprintf(stderr, \
	_("%s: Internal Error `%s'.\n"),file,text);

#define ENOSUPP(A,B)	fprintf(stderr,\
                                _("%s: feature `%s' not supported.\n" \
				  "Please recompile `net-tools' with "\
				  "newer kernel source or full configuration.\n"),A,B)

#define ESYSNOT(A,B)	fprintf(stderr, _("%s: no support for `%s' on this system.\n"),A,B)

#define E_NOTFOUND	8
#define E_SOCK		7
#define E_LOOKUP	6
#define E_VERSION	5
#define E_USAGE		4
#define E_OPTERR	3
#define E_INTERN	2
#define E_NOSUPP	1


/* ========== Kernel Defines =============
 * Since it is not a good idea to depend on special kernel sources for the headers
 * and since the libc6 Headers are not always up to date, we keep a copy of the
 * most often used Flags in this file. We realy need a way to keep them up-to-date.
 * Perhaps anybody knows how the glibc2 folk is doing it? -ecki
 */

/* Keep this ins sync with /usr/src/linux/include/linux/rtnetlink.h */
#define RTNH_F_DEAD            1       /* Nexthop is dead (used by multipath)  */
#define RTNH_F_PERVASIVE       2       /* Do recursive gateway lookup  */
#define RTNH_F_ONLINK          4       /* Gateway is forced on link    */

/* Keep this in sync with /usr/src/linux/include/linux/in_route.h */
#define RTCF_DEAD       RTNH_F_DEAD
#define RTCF_ONLINK     RTNH_F_ONLINK
/* #define RTCF_NOPMTUDISC RTM_F_NOPMTUDISC */
#define RTCF_NOTIFY     0x00010000
#define RTCF_DIRECTDST  0x00020000
#define RTCF_REDIRECTED 0x00040000
#define RTCF_TPROXY     0x00080000
#define RTCF_FAST       0x00200000
#define RTCF_MASQ       0x00400000
#define RTCF_SNAT       0x00800000
#define RTCF_DOREDIRECT 0x01000000
#define RTCF_DIRECTSRC  0x04000000
#define RTCF_DNAT       0x08000000
#define RTCF_BROADCAST  0x10000000
#define RTCF_MULTICAST  0x20000000
#define RTCF_REJECT     0x40000000
#define RTCF_LOCAL      0x80000000

/* Keep this in sync with /usr/src/linux/include/linux/ipv6_route.h */
#ifndef RTF_DEFAULT
#define RTF_DEFAULT     0x00010000      /* default - learned via ND     */
#endif
#define RTF_ALLONLINK   0x00020000      /* fallback, no routers on link */
#ifndef RTF_ADDRCONF
#define RTF_ADDRCONF    0x00040000      /* addrconf route - RA          */
#endif
#define RTF_NONEXTHOP   0x00200000      /* route with no nexthop        */
#define RTF_EXPIRES     0x00400000
#define RTF_CACHE       0x01000000      /* cache entry                  */
#define RTF_FLOW        0x02000000      /* flow significant route       */
#define RTF_POLICY      0x04000000      /* policy route                 */
#define RTF_LOCAL       0x80000000

/* Keep this in sync with /usr/src/linux/include/linux/route.h */
#define RTF_UP          0x0001          /* route usable                 */
#define RTF_GATEWAY     0x0002          /* destination is a gateway     */
#define RTF_HOST        0x0004          /* host entry (net otherwise)   */
#define RTF_REINSTATE   0x0008          /* reinstate route after tmout  */
#define RTF_DYNAMIC     0x0010          /* created dyn. (by redirect)   */
#define RTF_MODIFIED    0x0020          /* modified dyn. (by redirect)  */
#define RTF_MTU         0x0040          /* specific MTU for this route  */
#ifndef RTF_MSS
#define RTF_MSS         RTF_MTU         /* Compatibility :-(            */
#endif
#define RTF_WINDOW      0x0080          /* per route window clamping    */
#define RTF_IRTT        0x0100          /* Initial round trip time      */
#define RTF_REJECT      0x0200          /* Reject route                 */

/* this is a 2.0.36 flag from /usr/src/linux/include/linux/route.h */
#define RTF_NOTCACHED   0x0400          /* this route isn't cached        */

#ifdef HAVE_AFECONET
#ifndef AF_ECONET
#define AF_ECONET       19      /* Acorn Econet */
#endif
#endif

/* End of lib/support.h */

