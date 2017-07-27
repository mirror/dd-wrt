/*
 * dsock.c - Solaris socket processing functions for lsof
 */


/*
 * Copyright 1994 Purdue Research Foundation, West Lafayette, Indiana
 * 47907.  All rights reserved.
 *
 * Written by Victor A. Abell
 *
 * This software is not subject to any license of the American Telephone
 * and Telegraph Company or the Regents of the University of California.
 *
 * Permission is granted to anyone to use this software for any purpose on
 * any computer system, and to alter it and redistribute it freely, subject
 * to the following restrictions:
 *
 * 1. Neither the authors nor Purdue University are responsible for any
 *    consequences of the use of this software.
 *
 * 2. The origin of this software must not be misrepresented, either by
 *    explicit claim or by omission.  Credit to the authors and Purdue
 *    University must appear in documentation and sources.
 *
 * 3. Altered versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 4. This notice may not be removed or altered.
 */

#ifndef lint
static char copyright[] =
"@(#) Copyright 1994 Purdue Research Foundation.\nAll rights reserved.\n";
static char *rcsid = "$Id: dsock.c,v 1.31 2011/08/07 22:53:42 abe Exp $";
#endif


#include "lsof.h"

#if	solaris>=110000
#include <inet/ipclassifier.h>
#endif	/* solaris>=110000 */


#if	defined(HAS_LIBCTF) && solaris>=110000
/*
 * Icmp_t, rts_t and udp_t structure support for Solaris >=11 via libctf
 *
 * These structure definitions may look like kernel structures, but they
 * are not.  They have been defined to have member names that duplicate
 * those used by the kernel that are of interest to lsof.  Member valuess
 * are obtained via the CTF library, libctf.
 *
 * Robert Byrnes developed the CTF library access code and contributed it
 * to lsof.
 */

/*
 * Icmp_t internal structure definition
 */

typedef struct icmp_s {
	uint_t icmp_state;		/* TPI state */
	KA_T *icmp_connp;		/* connection structure pointer */
	in6_addr_t icmp_bound_v6src;	/* Explicitely bound to address */
	in6_addr_t icmp_v6src;		/* Source address of this stream */
	union {
	    uint_t icmp_dummy;
	    uint_t
		icmp_Debug : 1,	   	/* SO_DEBUG option */
		icmp_dontroute : 1,	/* SO_DONTROUTE option */
		icmp_broadcast : 1,	/* SO_BROADCAST option */
		icmp_reuseaddr : 1,	/* SO_REUSEADDR option */
		icmp_useloopback : 1,	/* SO_USELOOPBACK option */
		icmp_hdrincl : 1,	/* IP_HDRINCL option, etc. */
		icmp_dgram_errind : 1,	/* SO_DGRAM_ERRIND option */
		icmp_pad : 25;		/* pad to bit 31 */
	} icmp_debug;			/* This name identifies a single bit
					 * variable of the kernel's union, but
					 * CTF won't read individual bit
					 * variables, so for CTF's purposes
					 * it is declared as a uint_t union,
					 * named by the first bit variable of
					 * the kernel union, whose address CTF
					 * groks. */
} icmp_t;


/*
 * Rts_t internal structure definition
 */

typedef struct rts_s {
	uint_t rts_state;		/* Provider interface state */

# if	defined(HAS_CONN_NEW)
	KA_T *rts_connp;		/* connection structure pointer */
# endif	/* defined(HAS_CONN_NEW) */

	union {
	    uint_t rts_dummy;
	    uint_t
		rts_Debug : 1,		/* SO_DEBUG option */
		rts_dontroute : 1,	/* SO_DONTROUTE option */
		rts_broadcast : 1,	/* SO_BROADCAST option */
		rts_reuseaddr : 1,	/* SO_REUSEADDR option */
		rts_useloopback : 1,	/* SO_USELOOPBACK option */
		icmp_pad : 27;		/* padding to bit 31 */
	} rts_debug;			/* This name identifies a single bit
					 * variable, but CTF won't read
					 * individual bit variables, so for
					 * CTF's purposes it is declared as a
					 * uint_t union, named by its first
					 * bit variable, whose address CTF
					 * groks. */
} rts_t;

/*
 * Udp_t internal structure definition
 */

typedef struct udp { uint_t udp_state;		/* TPI state */
	in_port_t udp_port;		/* port bound to this stream */
	in_port_t udp_dstport;		/* connected port */
	in6_addr_t udp_v6src;		/* source address of this stream */
	in6_addr_t udp_v6dst;		/* connected destination */
	ushort_t udp_ipversion;		/* version -- IPV[46]_VERSION */
	KA_T *udp_connp; 		/* connection structure pointer */
	uint_t udp_bits;		/* socket option bits */
} udp_t;


/*
 * CTF definitions for icmp_t, rts_t and udp_t
 */

static int	IRU_ctfs = 0;		/* CTF initialization status for
					 * icmp_t, rts_t and udp_t */

# if	defined(_LP64)
#define	IRU_MOD_FORMAT "/kernel/%s/genunix"
#else	/* !defined(_LP64) */
#define	IRU_MOD_FORMAT "/kernel/genunix"
#endif	/* defined(_LP64) */

					/* genunix pathname template to which
					 * the kernel's instruction type set
					 * is added for CTF access to icmp_t,
					 * rts_t and udp_t */


/*
 * Icmp_t, rts_t and udp_t access definitions and structures
 */

#define	ICMP_T_TYPE_NAME "icmp_t"

static	CTF_member_t icmp_t_members[] = {
    CTF_MEMBER(icmp_state),
#define	MX_icmp_state			0

# if	defined(HAS_CONN_NEW)

    CTF_MEMBER(icmp_connp),
#define	MX_icmp_connp			1
# else	/* !defined(HAS_CONN_NEW) */
    CTF_MEMBER(icmp_bound_v6src),
#define	MX_icmp_bound_v6src		1

    CTF_MEMBER(icmp_v6src),
#define	MX_icmp_v6src			2

    CTF_MEMBER(icmp_debug),
#define	MX_icmp_debug			3
# endif	/* defined(HAS_CONN_NEW) */

    { NULL, 0 }
};


#define	RTS_T_TYPE_NAME "rts_t"

static	CTF_member_t rts_t_members[] = {
    CTF_MEMBER(rts_state),
#define	MX_rts_state			0

# if	defined(HAS_CONN_NEW)
    CTF_MEMBER(rts_connp),
#define	MX_rts_connp			1
# else	/* !defined(HAS_CONN_NEW) */

    CTF_MEMBER(rts_debug),
#define	MX_rts_debug			1
# endif	/* defined(HAS_CONN_NEW) */

    { NULL, 0 }
};


#define UDP_T_TYPE_NAME "udp_t"

static	CTF_member_t udp_t_members[] = {
    CTF_MEMBER(udp_state),
#define	MX_udp_state			0

    CTF_MEMBER(udp_connp),
#define	MX_udp_connp			1

# if	!defined(HAS_CONN_NEW)
    CTF_MEMBER(udp_port),
#define	MX_udp_port			2

    CTF_MEMBER(udp_dstport),
#define	MX_udp_dstport			3

    CTF_MEMBER(udp_v6src),
#define	MX_udp_v6src			4

    CTF_MEMBER(udp_v6dst),
#define	MX_udp_v6dst			5

    CTF_MEMBER(udp_ipversion),
#define	MX_udp_ipversion		6

    CTF_MEMBER(udp_bits),
#define	MX_udp_bits			7
# endif	/* !defined(HAS_CONN_NEW) */

    { NULL, 0 }
};


/*
 * CTF icmp_t, rts_t and udp_t request table
 */

static CTF_request_t IRU_requests[] = {
    { ICMP_T_TYPE_NAME,		icmp_t_members },
    { RTS_T_TYPE_NAME,		rts_t_members },
    { UDP_T_TYPE_NAME,		udp_t_members },
    { NULL,			NULL }
};


/*
 * Icmp_t, rts_t and udp_t function prototypes
 */

_PROTOTYPE(static int read_icmp_t,(KA_T va, KA_T ph, KA_T ia, icmp_t *ic));
_PROTOTYPE(static int read_rts_t,(KA_T va, KA_T ph, KA_T ra, rts_t *rt));
_PROTOTYPE(static int read_udp_t,(KA_T ua, udp_t *uc));
#endif	/* defined(HAS_LIBCTF) && solaris>=110000 */


#if	solaris<80000 || defined(HAS_IPCLASSIFIER_H)
/*
 * Make sure the tcpb structure is always defined.
 */

typedef struct tcpb {
	int dummy;
} tcpb_t;
#endif	/* solaris<80000 || defined(HAS_IPCLASSIFIER_H) */

#if	defined(HASIPv6)

/*
 * IPv6_2_IPv4()  -- macro to define the address of an IPv4 address contained
 *		     in an IPv6 address
 */

#define IPv6_2_IPv4(v6)	(((uint8_t *)((struct in6_addr *)v6)->s6_addr)+12)

/*
 * IPv_ADDR_UNSPEC() -- macro to test an IP[46] address for an unspecified
 *			address value
 */

#define IPv_ADDR_UNSPEC(af, p) \
    (((af) == AF_INET6) ? (IN6_IS_ADDR_UNSPECIFIED((struct in6_addr *)p)) \
			: (((struct in_addr *)(p))->s_addr == INADDR_ANY))
#else	/* !defined(HASIPv6) */

/*
 * IPv_ADDR_UNSPEC() -- IPv4-only form of macro to test for an unspecified
 *			address value
 */

#define	IPv_ADDR_UNSPEC(af, p) (((struct in_addr *)(p))->s_addr == INADDR_ANY)

#endif	/* !defined(HASIPv6) */

#if	defined(HASTCPOPT)
# if	solaris==20600
#include <netinet/tcp.h>
# endif	/* solaris==20600 */
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>

# if	defined(TH_TIMER_NEEDED)
#define	ACK_TIMER	TH_TIMER_NEEDED
# else
#  if	defined(TH_ACK_TIMER_NEEDED)
#define	ACK_TIMER	TH_ACK_TIMER_NEEDED
#  endif	/* defined(TH_ACK_TIMER_NEEDED) */
# endif	/* defined(TH_TIMER_NEEDED */
#endif	/* defined(HASTCPOPT) */

#if	defined(HASSOOPT)
# if	solaris<100000
#define	KEEPALIVE_INTERVAL	tcp_keepalive_intrvl
# else	/* solaris>=100000 */
#define	KEEPALIVE_INTERVAL	tcp_ka_last_intrvl
# endif	/* solaris<100000 */
#endif	/* defined(HASSOOPT) */


/*
 * Local function prototypes
 */

_PROTOTYPE(static void save_TCP_size,(tcp_t *tc));
_PROTOTYPE(static void save_TCP_states,(tcp_t *tc, caddr_t *fa, tcpb_t *tb,
					caddr_t *xp));


/*
 * build_IPstates() -- build the TCP and UDP state tables
 */

void
build_IPstates()
{
	if (!TcpSt) {
	    (void) enter_IPstate("TCP", "CLOSED", TCPS_CLOSED);
	    (void) enter_IPstate("TCP", "IDLE", TCPS_IDLE);
	    (void) enter_IPstate("TCP", "BOUND", TCPS_BOUND);
	    (void) enter_IPstate("TCP", "LISTEN", TCPS_LISTEN);
	    (void) enter_IPstate("TCP", "SYN_SENT", TCPS_SYN_SENT);
	    (void) enter_IPstate("TCP", "SYN_RCVD", TCPS_SYN_RCVD);
	    (void) enter_IPstate("TCP", "ESTABLISHED", TCPS_ESTABLISHED);
	    (void) enter_IPstate("TCP", "CLOSE_WAIT", TCPS_CLOSE_WAIT);
	    (void) enter_IPstate("TCP", "FIN_WAIT_1", TCPS_FIN_WAIT_1);
	    (void) enter_IPstate("TCP", "CLOSING", TCPS_CLOSING);
	    (void) enter_IPstate("TCP", "LAST_ACK", TCPS_LAST_ACK);
	    (void) enter_IPstate("TCP", "FIN_WAIT_2", TCPS_FIN_WAIT_2);
	    (void) enter_IPstate("TCP", "TIME_WAIT", TCPS_TIME_WAIT);
	    (void) enter_IPstate("TCP", (char *)NULL, 0);
	}
	if (!UdpSt) {
	    (void) enter_IPstate("UDP", "Unbound", TS_UNBND);
	    (void) enter_IPstate("UDP", "Wait_BIND_REQ_Ack", TS_WACK_BREQ);
	    (void) enter_IPstate("UDP", "Wait_UNBIND_REQ_Ack", TS_WACK_UREQ);
	    (void) enter_IPstate("UDP", "Idle", TS_IDLE);
	    (void) enter_IPstate("UDP", "Wait_OPT_REQ_Ack", TS_WACK_OPTREQ);
	    (void) enter_IPstate("UDP", "Wait_CONN_REQ_Ack", TS_WACK_CREQ);
	    (void) enter_IPstate("UDP", "Wait_CONN_REQ_Confirm", TS_WCON_CREQ);
	    (void) enter_IPstate("UDP", "Wait_CONN_IND_Response", TS_WRES_CIND);
	    (void) enter_IPstate("UDP", "Wait_CONN_RES_Ack", TS_WACK_CRES);
	    (void) enter_IPstate("UDP", "Wait_Data_Xfr", TS_DATA_XFER);
	    (void) enter_IPstate("UDP", "Wait_Read_Release", TS_WIND_ORDREL);
	    (void) enter_IPstate("UDP", "Wait_Write_Release", TS_WREQ_ORDREL);
	    (void) enter_IPstate("UDP", "Wait_DISCON_REQ_Ack", TS_WACK_DREQ6);
	    (void) enter_IPstate("UDP", "Wait_DISCON_REQ_Ack", TS_WACK_DREQ7);
	    (void) enter_IPstate("UDP", "Wait_DISCON_REQ_Ack", TS_WACK_DREQ9);
	    (void) enter_IPstate("UDP", "Wait_DISCON_REQ_Ack", TS_WACK_DREQ10);
	    (void) enter_IPstate("UDP", "Wait_DISCON_REQ_Ack", TS_WACK_DREQ11);
	    (void) enter_IPstate("UDP", (char *)NULL, 0);
	}
}


/*
 * print_tcptpi() - print TCP/TPI info
 */

void
print_tcptpi(nl)
	int nl;				/* 1 == '\n' required */
{
	char *cp = (char *)NULL;
	char  sbuf[128];
	int i;
	int ps = 0;
	unsigned int u;

	if (Ftcptpi & TCPTPI_STATE) {
	    switch (Lf->lts.type) {
	    case 0:				/* TCP */
		if (!TcpSt)
		    (void) build_IPstates();
		if ((i = Lf->lts.state.i + TcpStOff) < 0 || i >= TcpNstates) {
		    (void) snpf(sbuf, sizeof(sbuf), "UNKNOWN_TCP_STATE_%d",
			Lf->lts.state.i);
		    cp = sbuf;
		} else
		    cp = TcpSt[i];
		break;
	    case 1:				/* TPI */
		if (!UdpSt)
		    (void) build_IPstates();
		if ((u = Lf->lts.state.ui + UdpStOff) < 0 || u >= UdpNstates) {
		    (void) snpf(sbuf, sizeof(sbuf), "UNKNOWN_UDP_STATE_%u",
			Lf->lts.state.ui);
		    cp = sbuf;
		} else
		    cp = UdpSt[u];
	    }
	    if (cp) {
		if (Ffield)
		    (void) printf("%cST=%s%c", LSOF_FID_TCPTPI, cp, Terminator);
		else {
		    putchar('(');
		    (void) fputs(cp, stdout);
		}
		ps++;
	    }
	}

#if	defined(HASTCPTPIQ)
	if (Ftcptpi & TCPTPI_QUEUES) {
	    if (Lf->lts.rqs) {
		if (Ffield)
		    putchar(LSOF_FID_TCPTPI);
		else {
		    if (ps)
			putchar(' ');
		    else
			putchar('(');
		}
		(void) printf("QR=%lu", Lf->lts.rq);
		if (Ffield)
		    putchar(Terminator);
		ps++;
	    }
	    if (Lf->lts.sqs) {
		if (Ffield)
			putchar(LSOF_FID_TCPTPI);
		else {
		    if (ps)
			putchar(' ');
		    else
			putchar('(');
		}
		(void) printf("QS=%lu", Lf->lts.sq);
		if (Ffield)
		    putchar(Terminator);
		ps++;
	    }
	}
#endif	/* defined(HASTCPTPIQ) */

#if	defined(HASSOOPT)
	if (Ftcptpi & TCPTPI_FLAGS) {
	    int opt;

	    if ((opt = Lf->lts.opt)
	    ||  Lf->lts.pqlens || Lf->lts.qlens || Lf->lts.qlims
	    ||  Lf->lts.rbszs  || Lf->lts.sbsz
	    ) {
		char sep = ' ';

		if (Ffield)
		    sep = LSOF_FID_TCPTPI;
		else if (!ps)
		    sep = '(';
		(void) printf("%cSO", sep);
		ps++;
		sep = '=';

# if	defined(SO_BROADCAST)
		if (opt & SO_BROADCAST) {
		    (void) printf("%cBROADCAST", sep);
		    opt &= ~SO_BROADCAST;
		    sep = ',';
		}
# endif	/* defined(SO_BROADCAST) */

# if	defined(SO_DEBUG)
		if (opt & SO_DEBUG) {
		    (void) printf("%cDEBUG", sep);
		    opt &= ~ SO_DEBUG;
		    sep = ',';
		}
# endif	/* defined(SO_DEBUG) */

# if	defined(SO_DGRAM_ERRIND)
		if (opt & SO_DGRAM_ERRIND) {
		    (void) printf("%cDGRAM_ERRIND", sep);
		    opt &= ~SO_DGRAM_ERRIND;
		    sep = ',';
		}
# endif	/* defined(SO_DGRAM_ERRIND) */

# if	defined(SO_DONTROUTE)
		if (opt & SO_DONTROUTE) {
		    (void) printf("%cDONTROUTE", sep);
		    opt &= ~SO_DONTROUTE;
		    sep = ',';
		}
# endif	/* defined(SO_DONTROUTE) */

# if	defined(SO_KEEPALIVE)
		if (opt & SO_KEEPALIVE) {
		    (void) printf("%cKEEPALIVE", sep);
		    if (Lf->lts.kai)
			(void) printf("=%d", Lf->lts.kai);
		    opt &= ~SO_KEEPALIVE;
		    sep = ',';
		}
# endif	/* defined(SO_KEEPALIVE) */

# if	defined(SO_LINGER)
		if (opt & SO_LINGER) {
		    (void) printf("%cLINGER", sep);
		    if (Lf->lts.ltm)
			(void) printf("=%d", Lf->lts.ltm);
		    opt &= ~SO_LINGER;
		    sep = ',';
		}
# endif	/* defined(SO_LINGER) */

# if	defined(SO_OOBINLINE)
		if (opt & SO_OOBINLINE) {
		    (void) printf("%cOOBINLINE", sep);
		    opt &= ~SO_OOBINLINE;
		    sep = ',';
		}
# endif	/* defined(SO_OOBINLINE) */

		if (Lf->lts.pqlens) {
		    (void) printf("%cPQLEN=%u", sep, Lf->lts.pqlen);
		    sep = ',';
		}
		if (Lf->lts.qlens) {
		    (void) printf("%cQLEN=%u", sep, Lf->lts.qlen);
		    sep = ',';
		}
		if (Lf->lts.qlims) {
		    (void) printf("%cQLIM=%u", sep, Lf->lts.qlim);
		    sep = ',';
		}
		if (Lf->lts.rbszs) {
		    (void) printf("%cRCVBUF=%lu", sep, Lf->lts.rbsz);
		    sep = ',';
		}

# if	defined(SO_REUSEADDR)
		if (opt & SO_REUSEADDR) {
		    (void) printf("%cREUSEADDR", sep);
		    opt &= ~SO_REUSEADDR;
		    sep = ',';
		}
# endif	/* defined(SO_REUSEADDR) */

		if (Lf->lts.sbszs) {
		    (void) printf("%cSNDBUF=%lu", sep, Lf->lts.sbsz);
		    sep = ',';
		}

# if	defined(SO_TIMESTAMP)
		if (opt & SO_TIMESTAMP) {
		    (void) printf("%cTIMESTAMP", sep);
		    opt &= ~SO_TIMESTAMP;
		    sep = ',';
		}
# endif	/* defined(SO_TIMESTAMP) */

# if	defined(SO_USELOOPBACK)
		if (opt & SO_USELOOPBACK) {
		    (void) printf("%cUSELOOPBACK", sep);
		    opt &= ~SO_USELOOPBACK;
		    sep = ',';
		}
# endif	/* defined(SO_USELOOPBACK) */

		if (opt)
		    (void) printf("%cUNKNOWN=%#x", sep, opt);
		if (Ffield)
		    putchar(Terminator);
	    }
	}
#endif	/* defined(HASSOOPT) */

#if	defined(HASTCPOPT)
	if (Ftcptpi & TCPTPI_FLAGS) {
	    int topt;

	    if ((topt = Lf->lts.topt) || Lf->lts.msss) {
		char sep = ' ';

		if (Ffield)
		    sep = LSOF_FID_TCPTPI;
		else if (!ps)
		    sep = '(';
		(void) printf("%cTF", sep);
		ps++;
		sep = '=';

# if	defined(TF_ACKNOW)
		if (topt & TF_ACKNOW) {
		    (void) printf("%cACKNOW", sep);
		    topt &= ~TF_ACKNOW;
		    sep = ',';
		}
# endif	/* defined(TF_ACKNOW) */

# if	defined(TF_DELACK)
		if (topt & TF_DELACK) {
		    (void) printf("%cDELACK", sep);
		    topt &= ~TF_DELACK;
		    sep = ',';
		}
# endif	/* defined(TF_DELACK) */

		if (Lf->lts.msss) {
		    (void) printf("%cMSS=%lu", sep, Lf->lts.mss);
		    sep = ',';
		}

# if	defined(TF_NODELAY)
		if (topt & TF_NODELAY) {
		    (void) printf("%cNODELAY", sep);
		    topt &= ~TF_NODELAY;
		    sep = ',';
		}
# endif	/* defined(TF_NODELAY) */

# if	defined(TF_NOOPT)
		if (topt & TF_NOOPT) {
		    (void) printf("%cNOOPT", sep);
		    topt &= ~TF_NOOPT;
		    sep = ',';
		}
# endif	/* defined(TF_NOOPT) */

# if	defined(TF_SENTFIN)
		if (topt & TF_SENTFIN) {
		    (void) printf("%cSENTFIN", sep);
		    topt &= ~TF_SENTFIN;
		    sep = ',';
		}
# endif	/* defined(TF_SENTFIN) */

		if (topt)
		    (void) printf("%cUNKNOWN=%#x", sep, topt);
		if (Ffield)
		    putchar(Terminator);
	    }
	}
#endif	/* defined(HASTCPOPT) */

#if	defined(HASTCPTPIW)
	if (Ftcptpi & TCPTPI_WINDOWS) {
	    if (Lf->lts.rws) {
		if (Ffield)
			putchar(LSOF_FID_TCPTPI);
		else {
		    if (ps)
			putchar(' ');
		    else
			putchar('(');
		}
		(void) printf("WR=%lu", Lf->lts.rw);
		if (Ffield)
		    putchar(Terminator);
		ps++;
	    }
	    if (Lf->lts.wws) {
		if (Ffield)
			putchar(LSOF_FID_TCPTPI);
		else {
		    if (ps)
			putchar(' ');
		    else
			putchar('(');
		}
		(void) printf("WW=%lu", Lf->lts.ww);
		if (Ffield)
		    putchar(Terminator);
		ps++;
	    }
	}
#endif	/* defined(HASTCPTPIW) */

	if (Ftcptpi && !Ffield && ps)
	    putchar(')');
	if (nl)
	    putchar('\n');
}

#if	solaris>=110000
/*
 * procss_VSOCK() -- process a VSOCK socket
 */

# if	defined(HAS_CONN_NEW)
/*
 * Adjust for changes in the conn_s structure, introduced at OpenSolaris
 * level b134.
 */

#define	conn_ulp conn_proto
#define conn_rem V4_PART_OF_V6(connua_v6addr.connua_faddr)
#define conn_src V4_PART_OF_V6(connua_v6addr.connua_laddr)
# endif	/* defined(HAS_CONN_NEW) */

int
process_VSOCK(va, v, so)
	KA_T va;			/* containing vnode address */
	struct vnode *v;		/* pointer to containing vnode */
	struct sonode *so;		/* pointer to socket's sonode */
{
	int af;				/* address family */
	struct conn_s cs;		/* connection info */
	unsigned char *fa = (unsigned char *)NULL;
					/* foreign address */
	u_short fp = (u_short)0;	/* foreign port */
	u_short lp;			/* local port */
	icmp_t ic;			/* ICMP control structure */
	KA_T ka;			/* temporary kernel address */
	unsigned char *la = (unsigned char *)NULL;
					/* local address */
	KA_T pha;			/* protocol handle address */
	rts_t rt;			/* AF_ROUTE control structure */
	int s;				/* state */
	unsigned char *ta = (unsigned char *)NULL;
					/* temporary address */
	char tbuf[32], tbuf1[32];	/* temporary buffers */
	tcp_t tc;			/* TCP control structure */
	tcph_t th;			/* TCP header structure */

# if	defined(HAS_CONN_NEW)
	struct ip_xmit_attr_s xa;
	caddr_t *xp = (caddr_t *)NULL;
# else	/* !defined(HAS_CONN_NEW) */
	tcph_t *tha = (tcph_t *)NULL;	/* TCP header structure address */
# endif	/* defined(HAS_CONN_NEW) */

	char *ty;			/* TCP type */
	udp_t uc;			/* local UDP control structure */
/*
 * Read VSOCK's connection information.  Enter its address as the protocol
 * control block device address.
 */
	if (!(pha = (KA_T)so->so_proto_handle))
	    return(0);
	if (kread(pha, (char *)&cs, sizeof(cs))) {
	    (void) snpf(Namech, Namechl,
		"vnode at %s; snode at %s; can't read proto handle at: %s",
		print_kptr(va, tbuf, sizeof(tbuf)),
		print_kptr((KA_T)v->v_data, tbuf1, sizeof(tbuf1)),
		print_kptr(pha, (char *)NULL, 0));
	    Namech[Namechl - 1] = '\0';
	    enter_nm(Namech);
	    return(1);
	}
	enter_dev_ch(print_kptr(pha, (char *)NULL, 0));
/*
 * Process connection info by protocol.
 */
	switch ((af = so->so_family)) {
	case AF_INET:
	case AF_INET6:

	/*
	 * Set INET type -- IPv4 or IPv6.
	 */
	    if (af == AF_INET)
		ty = "IPv4";
	    else
		ty = "IPv6";
	    (void) snpf(Lf->type, sizeof(Lf->type), ty);

	    switch (cs.conn_ulp) {
	    case IPPROTO_TCP:

	    /*
	     * Process TCP socket; read its control structure.
	     */
		if (!(ka = (KA_T)cs.conn_proto_priv.cp_tcp)
		||  kread(ka, (char *)&tc, sizeof(tc))
		) {
		    (void) snpf(Namech, Namechl - 1,
			"can't read TCP socket's control structure: %s",
			print_kptr((KA_T)ka, (char *)NULL, 0));
		    Namech[Namechl - 1] = '\0';
		    enter_nm(Namech);
		    return(1);
		}
	    /*
	     * Set TCP protcol name in Lf->iproto[].
	     */
		(void) snpf(Lf->iproto, IPROTOL - 1, "%s", "TCP");
		Lf->iproto[IPROTOL - 1] = '\0';
		Lf->inp_ty = 2;
	    /*
	     * Check for TCP state inclusion or exclusion.
	     */
		if (TcpNstates) {
		    if ((s = (int)tc.tcp_state + TcpStOff) < TcpNstates) {
			if (TcpStXn) {
			    if (TcpStX[s]) {
				Lf->sf |= SELEXCLF;
				return(1);
			    }
			}
			if (TcpStIn) {
			    if (TcpStI[s]) {
				TcpStI[s] = 2;
				Lf->sf |= SELNET;
			    } else {
				Lf->sf |= SELEXCLF;
				return(1);
			    }
			}
		    }
		}
	    /*
	     * Set network file selection status.
	     */
		if (Fnet) {
		    if (!FnetTy
		    ||  ((FnetTy == 4) && (af == AF_INET))
		    ||  ((FnetTy == 6) && (af == AF_INET6))
		    ) {
			Lf->sf |= SELNET;
		    }
		}
	    /*
	     * Save local and remote (foreign) TCP address.
	     */
		if (af == AF_INET6) {
		    ta = (unsigned char *)&cs.connua_v6addr.connua_faddr;
		    la = (unsigned char *)&cs.connua_v6addr.connua_laddr;
		} else {
		    ta = (unsigned char *)&cs.conn_rem;
		    la = (unsigned char *)&cs.conn_src;
		}
		if (!IPv_ADDR_UNSPEC(af, ta) || (u_short)cs.conn_fport) {
		    fa = ta;
		    fp = (u_short)cs.conn_fport;
		}
		if ((af == AF_INET6)
		&&  ((la && IN6_IS_ADDR_V4MAPPED((struct in6_addr *)la))
		||  ((fa && IN6_IS_ADDR_V4MAPPED((struct in6_addr *)fa))))
		) {

		/*
		 * Convert IPv4 addresses in IPv6 structures to IPv4 addresses
		 * in IPv4 structures.  Change the address family to AF_INET.
		 */
		    if (la)
			la = (unsigned char *)IPv6_2_IPv4(la);
		    if (fa)
			fa = (unsigned char *)IPv6_2_IPv4(fa);
		    af = AF_INET;
		}
		lp = (u_short)cs.conn_lport;
		(void) ent_inaddr(la, (int)ntohs(lp), fa, (int)ntohs(fp), af);
	    /*
	     * Save TCP state information.
	     */

# if	defined(HAS_CONN_NEW)
		if ((ka = (KA_T)cs.conn_ixa)
		&&  !kread(ka, (char *)&xa, sizeof(xa))
		) {
		    xp = (caddr_t *)&xa;
		}
		(void) save_TCP_states(&tc, (caddr_t *)&cs, (tcpb_t *)NULL, xp);
# else	/* !defined(HAS_CONN_NEW) */
		if (tc.tcp_tcp_hdr_len
		&&  (ka = (KA_T)tc.tcp_tcph)
		&&  !kread(ka, (char *)&th, sizeof(th))
		) {
		    tha = &th;
		}
		(void) save_TCP_states(&tc, (caddr_t *)tha, (tcpb_t *)NULL,
				       (caddr_t *)NULL);
# endif	/* defined(HAS_CONN_NEW) */

		Lf->lts.type = 0;
		Lf->lts.state.i = (int)tc.tcp_state;
	    /*
	     * Save TCP size information.
	     */
		(void) save_TCP_size(&tc);
		break;
	    case IPPROTO_UDP:

	    /*
	     * Process UDP socket; read its control structure.
	     */
		if (!(ka = (KA_T)cs.conn_proto_priv.cp_udp)
		||  kread(ka, (char *)&uc, sizeof(uc))
		) {
		    (void) snpf(Namech, Namechl - 1,
			"can't read UDP socket's control structure: %s",
			print_kptr((KA_T)ka, (char *)NULL, 0));
		    Namech[Namechl - 1] = '\0';
		    enter_nm(Namech);
		    return(1);
		}
	    /*
	     * Set UDP protcol name in Lf->iproto[].
	     */
		(void) snpf(Lf->iproto, IPROTOL - 1, "%s", "UDP");
		Lf->iproto[IPROTOL - 1] = '\0';
		Lf->inp_ty = 2;
	    /*
	     * Check for UDP state inclusion or exclusion.
	     */
		if (UdpNstates) {
		    if ((s = (int)uc.udp_state + TcpStOff) < UdpNstates) {
			if (UdpStXn) {
			    if (UdpStX[s]) {
				Lf->sf |= SELEXCLF;
				return(1);
			    }
			}
			if (UdpStIn) {
			    if (UdpStI[s]) {
				UdpStI[s] = 2;
				Lf->sf |= SELNET;
			    } else {
				Lf->sf |= SELEXCLF;
				return(1);
			    }
			}
		    }
		}
	    /*
	     * Set network file selection status.
	     */
		if (Fnet) {
		    if (!FnetTy
		    ||  ((FnetTy == 4) && (af == AF_INET))
		    ||  ((FnetTy == 6) && (af == AF_INET6))
		    ) {
			Lf->sf |= SELNET;
		    }
		}
	    /*
	     * Save local and remote (foreign) UDP address.
	     */
		if (af == AF_INET6) {
		    ta = (unsigned char *)&cs.connua_v6addr.connua_faddr;
		    la = (unsigned char *)&cs.connua_v6addr.connua_laddr;
		} else {
		    ta = (unsigned char *)&cs.conn_rem;
		    la = (unsigned char *)&cs.conn_src;
		}
		if (!IPv_ADDR_UNSPEC(af, ta) || (u_short)cs.conn_fport) {
		    fa = ta;
		    fp = (u_short)cs.conn_fport;
	    	}
		lp = (u_short)cs.conn_lport;
		(void) ent_inaddr(la, (int)ntohs(lp), fa, (int)ntohs(fp), af);
	    /*
	     * Save UDP state and size information.
	     */
		if (!Fsize)
		    Lf->off_def = 1;
		Lf->lts.type = 1;
		Lf->lts.state.ui = (unsigned int)uc.udp_state;

# if	defined(HASSOOPT)
	    /*
	     * Save UDP flags.
	     */
		if (Ftcptpi & TCPTPI_FLAGS) {
		    union {
			uint_t flags;
			uint_t 
			    udpb_debug : 1,	   /* SO_DEBUG option */
			    udpb_dontroute : 1,	   /* SO_DONTROUTE option */
			    udpb_broadcast : 1,	   /* SO_BROADCAST option */
			    udpb_reuseaddr : 1,	   /* SO_REUSEADDR option */
			    udpb_useloopback : 1,  /* SO_USELOOPBACK option */
			    udpb_dgram_errind : 1, /* SO_DGRAM_ERRIND option */
			    udpb_pad : 26;	   /* pad to bit 31 */
		    } ucf;

		    ucf.flags = uc.udp_bits;
		    if (ucf.udpb_debug)
			Lf->lts.opt |= SO_DEBUG;
		    if (ucf.udpb_dontroute)
		        Lf->lts.opt |= SO_DONTROUTE;
		    if (ucf.udpb_broadcast)
		        Lf->lts.opt |= SO_BROADCAST;
		    if (ucf.udpb_reuseaddr)
			Lf->lts.opt |= SO_REUSEADDR;
		    if (ucf.udpb_useloopback)
			Lf->lts.opt |= SO_USELOOPBACK;
		    if (ucf.udpb_dgram_errind)
			Lf->lts.opt |= SO_DGRAM_ERRIND;
		}
# endif	/* defined(HASSOOPT) */

		break;
	    case IPPROTO_ICMP:
	    case IPPROTO_ICMPV6:

	    /*
	     * Process ICMP or ICMP6 socket.
	     *
	     * Set protocol name.
	     */
		if (cs.conn_ulp == IPPROTO_ICMP)
		    ty = "ICMP";
		else
		    ty = "ICMP6";
		(void) snpf(Lf->iproto, IPROTOL - 1, "%s", ty);
		Lf->iproto[IPROTOL - 1] = '\0';
		Lf->inp_ty = 2;
	    /*
	     * Read the ICMP control structure.
	     */
		if (read_icmp_t(va, pha, (KA_T)cs.conn_proto_priv.cp_icmp, &ic))
		    return(1);
	    /*
	     * Save ICMP size and state information.
	     */
		if (!Fsize)
		    Lf->off_def = 1;
		Lf->lts.type = 1;
		Lf->lts.state.ui = (unsigned int)ic.icmp_state;
	    /*
	     * Set network file selection status.
	     */
		if (Fnet) {
		    if (!FnetTy
		    ||  ((FnetTy == 4) && (af == AF_INET))
		    ||  ((FnetTy == 6) && (af == AF_INET6))
		    ) {
			Lf->sf |= SELNET;
		    }
		}
	    /*
	     * Save addresses.
	     */
		ta = (af == AF_INET6) ? (unsigned char *)&ic.icmp_bound_v6src
		   :  (unsigned char *)&V4_PART_OF_V6(ic.icmp_bound_v6src);
		if (!IPv_ADDR_UNSPEC(af, ta))
		    la = ta;
		ta = (af == AF_INET6) ? (unsigned char *)&ic.icmp_v6src
		   :  (unsigned char *)&V4_PART_OF_V6(ic.icmp_v6src);
		if (!IPv_ADDR_UNSPEC(af, ta))
		    fa = ta;
		if (la || fa)
		    (void)ent_inaddr(la, 0, fa, 0, af);

# if	defined(HASSOOPT)
	    /*
	     * Save ICMP flags.
	     */
		if (Ftcptpi & TCPTPI_FLAGS) {
		    if (ic.icmp_debug.icmp_Debug)
			Lf->lts.opt |= SO_DEBUG;
		    if (ic.icmp_debug.icmp_dontroute)
		        Lf->lts.opt |= SO_DONTROUTE;
		    if (ic.icmp_debug.icmp_broadcast)
		        Lf->lts.opt |= SO_BROADCAST;
		    if (ic.icmp_debug.icmp_reuseaddr)
			Lf->lts.opt |= SO_REUSEADDR;
		    if (ic.icmp_debug.icmp_useloopback)
			Lf->lts.opt |= SO_USELOOPBACK;
		    if (ic.icmp_debug.icmp_dgram_errind)
			Lf->lts.opt |= SO_DGRAM_ERRIND;
		}
# endif	/* defined(HASSOOPT) */

		break;
	    default:
		(void) snpf(Namech, Namechl - 1,
		    "unsupported conn_s AF_INET%s protocol: %u",
		    (af == AF_INET6) ? "6" : "",
		    (unsigned int)cs.conn_ulp);
		Namech[Namechl - 1] = '\0';
		enter_nm(Namech);
		return(1);
	    }
	    break;
	case AF_ROUTE:

	/*
	 * Set INET type -- IPv4 or IPv6.
	 */
	    if (af == AF_INET)
		ty = "IPv4";
	    else
		ty = "IPv6";
	    (void) snpf(Lf->type, sizeof(Lf->type), ty);
	/*
	 * Set protocol name.
	 */
	    (void) strncpy(Lf->iproto, "ROUTE", IPROTOL - 1);
	    Lf->iproto[IPROTOL - 1] = '\0';
	    Lf->inp_ty = 2;

	/*
	 * Read routing control structure.
	 */
	    if (read_rts_t(va, pha, (KA_T)cs.conn_proto_priv.cp_rts, &rt))
		return(1);
	/*
	/*
	 * Save AF_ROUTE size and state information.
	 */
	    if (!Fsize)
		Lf->off_def = 1;
	    Lf->lts.type = 1;
	    Lf->lts.state.i = (int)rt.rts_state;
	/*
	 * Set network file selection status.
	 */
	    if (Fnet) {
		if (!FnetTy
		||  ((FnetTy == 4) && (af == AF_INET))
		||  ((FnetTy == 6) && (af == AF_INET6))
		) {
		    Lf->sf |= SELNET;
		}
	    }

# if	defined(HASSOOPT)
	/*
	 * Save ROUTE flags.
	 */
	    if (Ftcptpi & TCPTPI_FLAGS) {
		if (rt.rts_debug.rts_Debug)
		    Lf->lts.opt |= SO_DEBUG;
		if (rt.rts_debug.rts_dontroute)
		    Lf->lts.opt |= SO_DONTROUTE;
		if (rt.rts_debug.rts_broadcast)
		    Lf->lts.opt |= SO_BROADCAST;
		if (rt.rts_debug.rts_reuseaddr)
		   Lf->lts.opt |= SO_REUSEADDR;
		if (rt.rts_debug.rts_useloopback)
		    Lf->lts.opt |= SO_USELOOPBACK;
	    }
# endif	/* defined(HASSOOPT) */

	    break;
	default:
	    (void) printiproto((int)cs.conn_ulp);
	    (void) snpf(Namech, Namechl - 1, "unsupported socket family: %u",
		so->so_family);
	    Namech[Namechl - 1] = '\0';
	    enter_nm(Namech);
	    Lf->inp_ty = 2;
	}
	return(1);
}
#endif	/* solaris>=110000*/


/*
 * process_socket() - process Solaris socket
 */

void
process_socket(sa, ty)
	KA_T sa;			/* stream's data address in kernel */
	char *ty;			/* socket type name */
{
	int af;
	unsigned char *fa = (unsigned char *)NULL;
	int fp = 0;
	int i, lp;

#if	solaris<110000
# if	solaris>=100000 && defined(HAS_IPCLASSIFIER_H)
	struct conn_s ic;
#define	ipc_v6laddr	conn_srcv6
#define	ipc_v6faddr	conn_remv6
#define	ipc_fport	conn_fport
#define	ipc_lport	conn_lport
# else	/* solaris<100000 || !defined(HAS_IPCLASSIFIER_H) */
	struct ipc_s ic;
# endif	/* solaris>=100000 && defined(HAS_IPCLASSIFIER_H) */
#else	/* solaris>=110000 */
	struct conn_s cs;
#endif	/* solaris<110000 */

	int ics = 0;
	unsigned char *la = (unsigned char *)NULL;
	struct module_info mi;
	KA_T ka;
	u_short p;
	KA_T pcb = (KA_T)NULL;
	struct queue q;
	struct qinit qi;
	KA_T qp;
	u_short *s;
	struct stdata sd;
	unsigned char *ta;
	char tbuf[32];

#if	solaris<20600
	struct tcp_s {			/* should come from kernel source
					 * file ../uts/common/inet/tcp.c */

# if	solaris>=20400
	    struct tcp_s *d1[8];
# endif	/* solaris>=20400 */

# if	defined(P101318) && P101318>=32
	    struct tcp_s *d1[6];
# endif	/* defined(P101318) && P101318>=32 */

	    int tcp_state;
	    queue_t *d3[2];
	    mblk_t *d4[2];
	    u_long d5;
	    mblk_t *d6;
	    u_long d7;
	    u_long tcp_snxt;	/* Senders next seq num */
	    u_long tcp_suna;	/* Sender unacknowledged */
	    u_long tcp_swnd;	/* Senders window (relative to suna) */
	    u_long d8[5];
	    int tcp_hdr_len;	/* combined TCP/IP header length */
	    tcph_t *tcp_tcph;	/* pointer to combined header */
	    int d9;
	    unsigned int d10;
	    int d11;
	    mblk_t *d12;
	    long d13;
	    mblk_t *d14;
	    u_long d15;

# if	solaris<20400 && (!defined(P101318) || P101318<32)
	    mblk_t *d16;
# endif	/* solaris<20400 && (!defined(P101318) || P101318<32) */

	    unsigned int d17;
	    u_long tcp_rnxt;	/* Seq we expect to recv next */
	    u_long tcp_rwnd;	/* Current receive window */
	    u_long d18;
	    long d19[2];
	    mblk_t *d20[4];
	    u_long d21[5];
	    long d22[3];

# if	solaris<20500
	    u_long d23[2];
	    u_long tcp_rack;	/* Seq # we have acked */
# else	/* solaris>=20500 */
	    u_long d23[3];
# endif	/* solaris<20500 */

# if	solaris<20400
	    u_long d24[28];
# else	/* solaris>=20400 */
#  if	solaris<20500
	    u_long d24[67];
#  else	/* solaris>=20500 */
#   if	solaris<20501
	    u_long d25[6];
#   else	/* solaris>=20501 */
	    u_long d25[8];
#   endif	/* solaris<20501 */
	    u_long tcp_rack;	/* Seq # we have acked */
#   if	solaris<20501
	    u_long d26[29];
#   else	/* solaris>=20501 */
	    u_long d26[33];
#   endif	/* solaris>=20501 */
#  endif	/* solaris<20500 */
# endif	/* solaris<20400 */

	    iph_t tcp_iph;
	} tc;
#else	/* solaris>=20600 */
	struct tcp_s tc;
#endif	/* solaris<20600 */

#if	solaris>=80000 && !defined(HAS_IPCLASSIFIER_H)
	tcpb_t	tcb;
#endif	/* solaris>=80000 && !defined(HAS_IPCLASSIFIER_H) */

	tcpb_t *tcbp = (tcpb_t *)NULL;
	int tcs = 0;
	tcph_t th;
	tcph_t *tha = (tcph_t *)NULL;

#if	solaris<110000
	struct ud_s {			/* should come from kernel source
					 * file ../uts/common/inet/udp.c */
	    uint udp_state;		/* TPI state */
	    unsigned char d1[2];
	    unsigned char udp_port[2];	/* port bound to this stream */
	    unsigned char udp_src[4];	/* source address of this stream */
	} uc;
#else	/* solaris>=110000 */
	udp_t uc;			/* UDP control structure */
#endif	/* solaris<110000 */
	int ucs = 0;

#if	defined(HASIPv6)
	if (strrchr(ty, '6')) {
	    (void) snpf(Lf->type, sizeof(Lf->type), "IPv6");
	    af = AF_INET6;
	} else {
	    (void) snpf(Lf->type, sizeof(Lf->type), "IPv4");
	    af = AF_INET;
	}
#else	/* !defined(HASIPv6) */
	(void) snpf(Lf->type, sizeof(Lf->type), "inet");
	af = AF_INET;
#endif	/* defined(HASIPv6) */

/*
 * Set network file selection status.
 */
	if (Fnet) {
	    if (!FnetTy
	    ||  ((FnetTy == 4) && (af == AF_INET))

#if	defined(HASIPv6)
	    ||  ((FnetTy == 6) && (af == AF_INET6))
#endif	/* defined(HASIPv6) */

	    ) {
		if (!TcpStIn && !UdpStIn)
		    Lf->sf |= SELNET;
	    }
	}
	Lf->inp_ty = 2;
/*
 * Convert type to upper case protocol name.
 */
	if (ty) {
	    for (i = 0; (ty[i] != '\0') && (i < IPROTOL) && (i < 3); i++) {
		if (islower((unsigned char)ty[i]))
		    Lf->iproto[i] = toupper((unsigned char)ty[i]);
		else
		    Lf->iproto[i] = ty[i];
	    }
	} else
	    i = 0;
	Lf->iproto[i] = '\0';
/*
 * Read stream queue entries to obtain private IP, TCP, and UDP structures.
 */
	if (!sa || readstdata(sa, &sd))
	    qp = (KA_T)NULL;
	else
	    qp = (KA_T)sd.sd_wrq;
	for (i = 0; qp && i < 20; i++, qp = (KA_T)q.q_next) {
	    if (kread(qp, (char *)&q, sizeof(q)))
		break;
	    if ((ka = (KA_T)q.q_qinfo) == (KA_T)NULL
	    ||  kread(ka, (char *)&qi, sizeof(qi)))
		continue;
	    if ((ka = (KA_T)qi.qi_minfo) == (KA_T)NULL
	    ||  kread(ka, (char *)&mi, sizeof(mi))
	    ||  (ka = (KA_T)mi.mi_idname) == (KA_T)NULL)
		continue;
	    if (kread(ka, (char *)&tbuf, sizeof(tbuf) - 1))
		continue;
	    if ((pcb = (KA_T)q.q_ptr) == (KA_T)NULL)
		continue;

#if	solaris<110000
	    if (strncasecmp(tbuf, "IP",  2) == 0) {
		if (kread(pcb, (char *)&ic, sizeof(ic)) == 0)
		    ics = 1;
		continue;
	    }
#endif	/* solaris<110000 */

	    if (strncasecmp(tbuf, "TCP", 3) == 0) {

#if	solaris<=90000 || !defined(HAS_IPCLASSIFIER_H)
		if (!kread((KA_T)pcb, (char *)&tc, sizeof(tc)))

# if	solaris>=80000
		{
		    if (tc.tcp_base
		    &&  !kread((KA_T)tc.tcp_base, (char *)&tcb, sizeof(tcb))) {
			tcs = 1;
			tcbp = &tcb;
		    }
		    tc.tcp_base = &tcb;		/* support for macros */
		    tcb.tcpb_tcp = &tc;		/* support for macros */
		}
# else	/* solaris<80000 */
		    tcs = 1;
# endif	/* solaris>=80000 */
#else	/* solaris>90000 && defined(HAS_IPCLASSIFIER_H) */
# if	solaris>=110000
		if (!kread(pcb, (char *)&cs, sizeof(cs))
		&&  (cs.conn_ulp == IPPROTO_TCP)
		) {
		    ics = 1;
		    if ((ka = (KA_T)cs.conn_proto_priv.cp_tcp)
		    &&  !kread(ka, (char *)&tc, sizeof(tc))
		    ) {
			tcs = 1;
		    }
		}
# else	/* solaris<110000 */
		if (!kread((KA_T)pcb, (char *)&ic, sizeof(ic))
		&&  ic.conn_tcp
		&&  !kread((KA_T)ic.conn_tcp, (char *)&tc, sizeof(tc))
		) {
		    ics = tcs = 1;
		}
# endif	/* solaris>=110000 */
#endif        /* solaris<=90000 || !defined(HAS_IPCLASSIFIER_H) */

		if (tcs && TcpNstates) {
		    int s = (int)tc.tcp_state + TcpStOff;
		/*
		 * Check for TCP state inclusion or exclusion.
		 */

		    if (s < TcpNstates) {
			if (TcpStXn) {
			    if (TcpStX[s]) {
				Lf->sf &= ~SELNET;
				Lf->sf |= SELEXCLF;
				return;
			    }
			}
			if (TcpStIn) {
			    if (TcpStI[s]) {
				TcpStI[s] = 2;
				Lf->sf |= SELNET;
			    } else {
				Lf->sf &= ~SELNET;
				Lf->sf |= SELEXCLF;
				return;
			    }
			}
		    }
		}
		if (!(Lf->sf & SELNET) && !TcpStIn && UdpStIn) {
		    if (Fnet) {
			if (!FnetTy
			||  (FnetTy == 4) && (af == AF_INET)

#if	defined(HASIPv6)
			||  (FnetTy == 6) && (af == AF_INET6)
#endif 	/* defined(HASIPv6) */

			) {
			    Lf->sf |= SELNET;
			}
		    }
		}
		continue;
	    }
	    if (strncasecmp(tbuf, "UDP", 3) == 0) {

#if	solaris<110000
		if (kread(pcb, (char *)&uc, sizeof(uc)) == 0)
		    ucs = 1;
#else	/* solaris>=110000 */
		if (!kread(pcb, (char *)&cs, sizeof(cs))
		&&  (cs.conn_ulp == IPPROTO_UDP)
		) {
		    ics = 1;
		    if ((ka = (KA_T)cs.conn_proto_priv.cp_udp)
		    &&  !read_udp_t(ka, &uc)
		    ) {
			ucs = 1;
		    }
		}
#endif	/* solaris<110000 */

		if (ucs && UdpNstates) {
		    unsigned int s = (unsigned int)uc.udp_state + UdpStOff;
		/*
		 * Check for UDP state inclusion or exclusion.
		 */

		    if (s < UdpNstates) {
			if (UdpStXn) {
			    if (UdpStX[s]) {
				Lf->sf &= ~SELNET;
				Lf->sf |= SELEXCLF;
				return;
			    }
			}
			if (UdpStIn) {
			    if (UdpStI[s]) {
				UdpStI[s] = 2;
				Lf->sf |= SELNET;
			    } else {
				Lf->sf |= SELEXCLF;
				return;
			    }
			}
		    }
		}
		if (!(Lf->sf & SELNET) && TcpStIn && !UdpStIn) {
		    if (Fnet) {
			if (!FnetTy
			||  (FnetTy == 4) && (af == AF_INET)

#if	defined(HASIPv6)
			||  (FnetTy == 6) && (af == AF_INET6)
#endif 	/* defined(HASIPv6) */

			) {
			    Lf->sf |= SELNET;
			}
		    }
		}
		continue;
	    }
	}
	if (ics) {

	/*
	 * Print stream head's q_ptr address as protocol control block address.
	 */
	    if (pcb)
		enter_dev_ch(print_kptr(pcb, (char *)NULL, 0));
	    if (strncmp(Lf->iproto, "UDP", 3) == 0) {

	/*
	 * Save UDP address and TPI state.
	 */

#if	solaris<20600
		la = (unsigned char *)&ic.ipc_udp_addr;
		p = (u_short)ic.ipc_udp_port;
#else	/* solaris>=20600 */
# if	solaris>=110000
		af = (uc.udp_ipversion == IPV6_VERSION) ? AF_INET6 : AF_INET;
		la = (af == AF_INET6) ? (unsigned char *)&uc.udp_v6src
		   :  (unsigned char *)&V4_PART_OF_V6(uc.udp_v6src);
		p = (u_short)uc.udp_port;
# else	/* solaris<110000 */
#  if	defined(HASIPv6)
		la = (af == AF_INET6) ? (unsigned char *)&ic.ipc_v6laddr
		   :  (unsigned char *)IPv6_2_IPv4(&ic.ipc_v6laddr);
#  else	/* !defined(HASIPv6 */
		la = (unsigned char *)&ic.ipc_laddr;
#  endif	/* defined(HASIPv6) */

		p = (u_short)ic.ipc_lport;
# endif	/* solaris>=110000 */
#endif	/* solaris<20600 */

#if	solaris<110000
		if (IPv_ADDR_UNSPEC(af, la) && !p && ucs) {

		/*
		 * If the ipc_s structure has no local address, use
		 * the port in the ud_s structure.
		 */
		    s = (u_short *)&uc.udp_port[0];
		    p = *s;
		}

# if	defined(HASIPv6)
		if ((af == AF_INET6) && la
		&&  IN6_IS_ADDR_V4MAPPED((struct in6_addr *)la)) {

		/*
		 * Convert a local IPv4 address in an IPv6 structure to an IPv4
		 * address in an IPv4 structure.  Change the address family to
		 * AF_INET.
		 */
		    la = (unsigned char *)IPv6_2_IPv4(la);
		    af = AF_INET;
		}
# endif	/* defined(HASIPv6) */
#endif	/* solaris<110000 */

		(void) ent_inaddr(la, (int)ntohs(p), (unsigned char *)NULL,
				  -1, af);
		if (!Fsize)
		    Lf->off_def = 1;
		if (ucs) {
		    Lf->lts.type = 1;
		    Lf->lts.state.ui = (unsigned int)uc.udp_state;
		}
	    } else if (strncmp(Lf->iproto, "TCP", 3) == 0) {
		if (ics) {

	    /*
	     * Save TCP address.
	     */

#if	solaris<20400
		    la = (unsigned char *)&ic.ipc_tcp_addr[0];
		    p = (u_short)ic.ipc_tcp_addr[5];
#else	/* solaris>=20400 */
# if	solaris<20600
		    la = (unsigned char *)&ic.ipc_tcp_laddr;
		    p = (u_short)((short *)&ic.ipc_tcp_ports)[1];
# else	/* solaris>=20600 */
#  if	solaris>=110000
		    la = (af == AF_INET6)
		       ? (unsigned char *)&cs.connua_v6addr.connua_laddr
		       : (unsigned char *)&cs.conn_src;
	            lp = cs.conn_lport;
#  else	/* solaris<110000 */
#   if	defined(HASIPv6)
		    la = (af == AF_INET6) ? (unsigned char *)&ic.ipc_v6laddr
		       :  (unsigned char *)IPv6_2_IPv4(&ic.ipc_v6laddr);
#   else		/* !defined(HASIPv6 */
		    la = (unsigned char *)&ic.ipc_laddr;
#   endif	/* defined(HASIPv6) */

		    p = (u_short)ic.ipc_lport;
#  endif	/* solaris>=110000 */
# endif	/* solaris<20600 */
#endif	/* solaris<20400 */

#if	solaris<110000
		    if (IPv_ADDR_UNSPEC(af, la) && !p && tcs) {

		    /*
		     * If the ipc_s structure has no local address, use the
		     * local address in the stream's tcp_iph structure (except
		     * for Solaris 2.4), and the port number in the stream's
		     * tcph structure.
		     */

# if	solaris!=20400 && solaris<80000
			la = (unsigned char *)&tc.tcp_iph.iph_src[0];
# else	/* solaris==20400 || solaris<80000 */
#  if	solaris>=100000 && defined(HAS_IPCLASSIFIER_H)
			la = (af == AF_INET6) ? (unsigned char *)&ic.conn_srcv6
			   :  (unsigned char *)IPv6_2_IPv4(&ic.conn_srcv6);
#  else	/* solaris<100000 || !defined(HAS_IPCLASSIFIER_H) */
#   if	solaris>=80000
#    if	defined(HASIPv6)
			la = (af == AF_INET6)
			   ? (unsigned char *)&tcb.tcpb_ip_src_v6
			   :  (unsigned char *)IPv6_2_IPv4(&tcb.tcpb_ip_src_v6);
#    else	/* !defined(HASIPv6) */
			la = (unsigned char *)&tcb.tcpb_ip_src;
#    endif	/* defined(HASIPv6) */
#   endif	/* solaris>=80000 */
#  endif	/* solaris>=100000 && defined(HAS_IPCLASSIFIER_H) */
# endif	/* solaris!=20400 && !defined(HASIPv6) */

			if (tc.tcp_hdr_len && tc.tcp_tcph
			&&  !kread((KA_T)tc.tcp_tcph, (char *)&th, sizeof(th))
			) {
			    tha = &th;
			    s = (u_short *)&th.th_lport[0];
			    p = *s;
			}
		    }
#endif	/* solaris<110000 */

		    lp = (int)ntohs(p);

#if	solaris<20400
		    if ((int)ic.ipc_tcp_addr[2] != INADDR_ANY
		    ||  ic.ipc_tcp_addr[4] != 0)
		    {
			fa = (unsigned char *)&ic.ipc_tcp_addr[2];
			fp = (int)ntohs(ic.ipc_tcp_addr[4]);
		    }
#else	/* solaris>=20400 */
# if	solaris<20600
		    if ((int)ic.ipc_tcp_faddr != INADDR_ANY
		    ||  ((u_short *) &ic.ipc_tcp_ports)[0] != 0)
		    {
			fa = (unsigned char *)&ic.ipc_tcp_faddr;
			fp = (int)ntohs(((u_short *)&ic.ipc_tcp_ports)[0]);
		    }
# else	/* solaris>=20600 */

#  if	solaris>=110000
		    ta = (af == AF_INET6)
		       ? (unsigned char *)&cs.connua_v6addr.connua_faddr
		       : (unsigned char *)&cs.conn_rem;
		    if (!IPv_ADDR_UNSPEC(af, ta) || ((u_short)cs.conn_fport)) {
			fa = ta;
			fp = (u_short)cs.conn_fport;
		    }
#  else	/* solaris<110000 */
#   if	defined(HASIPv6)
		    ta = (af == AF_INET6) ? (unsigned char *)&ic.ipc_v6faddr
		       :  (unsigned char *)IPv6_2_IPv4(&ic.ipc_v6faddr);
#   else	/* !defined(HASIPv6) */
		    ta = (unsigned char *)&ic.ipc_faddr;
#   endif	/* defined(HASIPv6) */

		    if (!IPv_ADDR_UNSPEC(af, ta) || ((u_short)ic.ipc_fport)) {
			fa = ta;
			fp = (int)ntohs(((u_short)ic.ipc_fport));
		    }
#  endif	/* solaris>=110000 */
# endif	/* solaris<20600 */
#endif	/* solaris <20400 */

#if	defined(HASIPv6)
		    if ((af == AF_INET6)
		    &&  ((la && IN6_IS_ADDR_V4MAPPED((struct in6_addr *)la))
		    ||  ((fa && IN6_IS_ADDR_V4MAPPED((struct in6_addr *)fa))))
		    ) {

		    /*
		     * Convert IPv4 addresses in IPv6 structures to IPv4
		     * addresses in IPv4 structures.  Change the address
		     * family to AF_INET.
		     */
			if (la)
			    la = (unsigned char *)IPv6_2_IPv4(la);
			if (fa)
			    fa = (unsigned char *)IPv6_2_IPv4(fa);
			af = AF_INET;
		    }
#endif	/* defined(HASIPv6) */

		    if (fa || la)
			(void) ent_inaddr(la, lp, fa, fp, af);
		}
	    /*
	     * Save TCP state information.
	     */
		if (tcs) {
		    (void) save_TCP_states(&tc, (caddr_t *)tha, tcbp,
					   (caddr_t *)NULL);
		    Lf->lts.type = 0;
		    Lf->lts.state.i = (int)tc.tcp_state;
		}
	    /*
	     * Save TCP size information.
	     */

		if (tcs)
		    (void) save_TCP_size(&tc);
	    }
	} else
	    (void) strcat(Namech, "no TCP/UDP/IP information available");
/*
 * Enter name characters if there are some.
 */
	if (Namech[0])
	    enter_nm(Namech);
}


#if	solaris>=110000
/*
 * read_icmp_t() - read connections icmp_t info
 */

static int
read_icmp_t(va, ph, ia, ic)
	KA_T va;			/* containing vnode kernel address */
	KA_T ph;			/* containing protocol handle kernel
					 * address */
	KA_T ia;			/* icmp_t structure's kernel address */
	icmp_t *ic;			/* local icmp_t receiver */
{
	char tbuf[32], tbuf1[32];	/* print_kptr() temporary buffers */

# if	defined(HAS_CONN_NEW)
	struct conn_s cs;		/* connection structure */
	KA_T ka;			/* kernel address */

	zeromem((char *)ic, sizeof(icmp_t));
# endif	/* defined(HAS_CONN_NEW) */

	(void) CTF_init(&IRU_ctfs, IRU_MOD_FORMAT, IRU_requests);
	if (!ia
	||  CTF_MEMBER_READ(ia, ic, icmp_t_members, icmp_state)

# if	defined(HAS_CONN_NEW)
	||  CTF_MEMBER_READ(ia, ic, icmp_t_members, icmp_connp)
# else	/* !defined(HAS_CONN_NEW) */
	||  CTF_MEMBER_READ(ia, ic, icmp_t_members, icmp_bound_v6src)
	||  CTF_MEMBER_READ(ia, ic, icmp_t_members, icmp_v6src)
	||  CTF_MEMBER_READ(ia, ic, icmp_t_members, icmp_debug)
# endif	/* defined(HAS_CONN_NEW) */

	) {
	    (void) snpf(Namech, Namechl - 1,
		"vnode at %s; proto handle at %s; can't read icmp_t at %s",
		print_kptr(va, tbuf, sizeof(tbuf)),
		print_kptr(ph, tbuf1, sizeof(tbuf1)),
		print_kptr(ia, (char *)NULL, 0));
	    Namech[Namechl - 1] = '\0';
	    enter_nm(Namech);
	    return(1);
	}

# if	defined(HAS_CONN_NEW)
	if ((ka = (KA_T)ic->icmp_connp)
	&& !kread(ka, (char *)&cs, sizeof(cs)))
	{
	    struct ip_xmit_attr_s xa;

	/*
	 * Complete the icmp_t structure from the conn_s structure.
	 */
	    ic->icmp_bound_v6src = cs.conn_bound_addr_v6;
	    ic->icmp_v6src = cs.conn_saddr_v6;
	    ic->icmp_debug.icmp_Debug = cs.conn_debug;
	    ic->icmp_debug.icmp_broadcast = cs.conn_broadcast;
	    ic->icmp_debug.icmp_reuseaddr = cs.conn_reuseaddr;
	    ic->icmp_debug.icmp_useloopback = cs.conn_useloopback;
	    ic->icmp_debug.icmp_dgram_errind = cs.conn_dgram_errind;
	    if ((ka = (KA_T)cs.conn_ixa)
	    &&  !kread(ka, (char *)&xa, sizeof(xa))
	    ) {
		ic->icmp_debug.icmp_dontroute = (xa.ixa_flags & IXAF_DONTROUTE)
					      ? 1 : 0;
	    }

	}
# endif	/* defined(HAS_CONN_NEW) */

	return(0);
}


/*
 * read_rts_t() - read connections rts_t info
 */

static int
read_rts_t(va, ph, ra, rt)
	KA_T va;			/* containing vnode kernel address */
	KA_T ph;			/* containing protocol handle kernel
					 * address */
	KA_T ra;			/* rts_t structure's kernel address */
	rts_t *rt;			/* local rts_t receiver */
{
	char tbuf[32], tbuf1[32];	/* print_kptr() temporary buffers */

# if	defined(HAS_CONN_NEW)
	struct conn_s cs;		/* connextion structure */
	KA_T ka;			/* kernal address */

	zeromem((char *)rt, sizeof(rts_t));
# endif	/* defined(HAS_CONN_NEW) */

	(void) CTF_init(&IRU_ctfs, IRU_MOD_FORMAT, IRU_requests);
	if (!ra
	||  CTF_MEMBER_READ(ra, rt, rts_t_members, rts_state)

# if	defined(HAS_CONN_NEW)
	||  CTF_MEMBER_READ(ra, rt, rts_t_members, rts_connp)
# else	/* !defined(HAS_CONN_NEW) */
	||  CTF_MEMBER_READ(ra, rt, rts_t_members, rts_debug)
# endif	/* defined(HAS_CONN_NEW) */

	) {
	    (void) snpf(Namech, Namechl - 1,
		"vnode at %s; proto handle at %s; can't read rts_t at %s",
		print_kptr(va, tbuf, sizeof(tbuf)),
		print_kptr(ph, tbuf1, sizeof(tbuf1)),
		print_kptr(ra, (char *)NULL, 0));
	    Namech[Namechl - 1] = '\0';
	    enter_nm(Namech);
	    return(1);
	}

# if	defined(HAS_CONN_NEW)
	if ((ka = (KA_T)rt->rts_connp)
	&&  !kread(ka, (char *)&cs, sizeof(struct conn_s))
	) {
	    struct ip_xmit_attr_s xa;

	/*
	 * Fill in rts_debug from the connection structure.
	 */
	    rt->rts_debug.rts_Debug = cs.conn_debug;
	    rt->rts_debug.rts_broadcast = cs.conn_broadcast;
	    rt->rts_debug.rts_reuseaddr = cs.conn_reuseaddr;
	    rt->rts_debug.rts_useloopback = cs.conn_useloopback;
	    if ((ka = (KA_T)cs.conn_ixa)
	    &&  !kread(ka, (char *)&xa, sizeof(xa))
	    ) {
		rt->rts_debug.rts_dontroute = (xa.ixa_flags & IXAF_DONTROUTE)
					    ? 1 : 0;
	    }
	}

# endif	/* defined(HAS_CONN_NEW) */

	return(0);
}


/*
 * read_udp_t() - read UDP control structure
 */

static int
read_udp_t(ua, uc)
	KA_T ua;			/* ucp_t kernel address */
	udp_t *uc;			/* receiving udp_t structure */
{
	(void) CTF_init(&IRU_ctfs, IRU_MOD_FORMAT, IRU_requests);
	if (!ua
	||  CTF_MEMBER_READ(ua, uc, udp_t_members, udp_state)

# if	defined(HAS_CONN_NEW)
	||  CTF_MEMBER_READ(ua, uc, udp_t_members, udp_connp)
# else	/* !defined(HAS_CONN_NEW) */
	||  CTF_MEMBER_READ(ua, uc, udp_t_members, udp_port)
	||  CTF_MEMBER_READ(ua, uc, udp_t_members, udp_dstport)
	||  CTF_MEMBER_READ(ua, uc, udp_t_members, udp_v6src)
	||  CTF_MEMBER_READ(ua, uc, udp_t_members, udp_v6dst)
	||  CTF_MEMBER_READ(ua, uc, udp_t_members, udp_ipversion)
	||  CTF_MEMBER_READ(ua, uc, udp_t_members, udp_bits)
# endif	/* defined(HAS_CONN_NEW) */

	) {
	    (void) snpf(Namech, Namechl, "can't read udp_t: %s",
			print_kptr(ua, (char *)NULL, 0));
	    Namech[Namechl - 1] = '\0';
	    enter_nm(Namech);
	    return(1);
	}
	return(0);
}
#endif	/* solaris>=110000 */


/*
 * save_TCP_size() -- save TCP size information
 */

static void

save_TCP_size(tc)
	tcp_t *tc;			/* pointer to TCP control structure */
{
	int rq, sq;

#if	defined(HASTCPTPIQ) || defined(HASTCPTPIW)
# if	defined(HASTCPTPIW)
	Lf->lts.rw = (int)tc->tcp_rwnd;
	Lf->lts.ww = (int)tc->tcp_swnd;
	Lf->lts.rws = Lf->lts.wws = 1;
# endif	/* defined(HASTCPTPIW) */

	if ((rq = (int)tc->tcp_rnxt - (int)tc->tcp_rack) < 0)
	    rq = 0;
	if ((sq = (int)tc->tcp_snxt - (int)tc->tcp_suna - 1) < 0)
	    sq  = 0;

# if	defined(HASTCPTPIQ)
	Lf->lts.rq = (unsigned long)rq;
	Lf->lts.sq = (unsigned long)sq;
	Lf->lts.rqs = Lf->lts.sqs = 1;
# endif	/* defined(HASTCPTPIQ) */

	if (Fsize) {
	    if (Lf->access == 'r')
		Lf->sz = (SZOFFTYPE)rq;
	    else if (Lf->access == 'w')
		Lf->sz = (SZOFFTYPE)sq;
	    else
		Lf->sz = (SZOFFTYPE)(rq + sq);
		Lf->sz_def = 1;
	} else
	    Lf->off_def = 1;
#else	/* !defined(HASTCPTPIQ) && !defined(HASTCPTPIW) */
	Lf->off_def = 1;
#endif	/* defined(HASTCPTPIQ) || defined(HASTCPTPIW) */

}


/*
 * save_TCP_states() - save TCP states
 */

static void
save_TCP_states(tc, fa, tb, xp)
	tcp_t *tc;			/* pointer to TCP control structure */
	caddr_t *fa;			/* flags address (may be NULL):
					 *   if HAS_CONN_NEW: conn_s *
					 *   if !CONN_HAS_NEW: tcph_t *
					 */
	tcpb_t *tb;			/* pointer to TCP base structure (may
					 * be NULL) */
	caddr_t *xp;			/* pointer to struct ip_xmit_attr_s if
					 * HAS_CONN_NEW (may be NULL) */
{
	if (!tc)
	    return;

#if	defined(HASSOOPT)
# if	defined(HAS_CONN_NEW)
	if (Ftcptpi & TCPTPI_FLAGS && fa) {
	    struct conn_s *cs = (struct conn_s *)fa;

	    if (cs->conn_broadcast)
		Lf->lts.opt |= SO_BROADCAST;
	    if (cs->conn_debug)
		Lf->lts.opt |= SO_DEBUG;
	    if (cs->conn_dgram_errind)
		Lf->lts.opt |= SO_DGRAM_ERRIND;
	    if (xp && (((ip_xmit_attr_t *)xp)->ixa_flags & IXAF_DONTROUTE))
		Lf->lts.opt |= SO_DONTROUTE;
	    if (cs->conn_keepalive) {
		Lf->lts.opt |= SO_KEEPALIVE;
		Lf->lts.kai = (unsigned int)tc->tcp_ka_interval;
	    }
	    if (cs->conn_linger) {
		Lf->lts.opt |= SO_LINGER;
		Lf->lts.ltm = (unsigned int)cs->conn_lingertime;
	    }
	    if (cs->conn_oobinline)
		Lf->lts.opt |= SO_OOBINLINE;
	    Lf->lts.pqlen = (unsigned int)tc->tcp_conn_req_cnt_q0;
	    Lf->lts.qlen = (unsigned int)tc->tcp_conn_req_cnt_q;
	    Lf->lts.qlim = (unsigned int)tc->tcp_conn_req_max;
	    Lf->lts.pqlens = Lf->lts.qlens = Lf->lts.qlims
			   = (unsigned char)1;
	    if (cs->conn_reuseaddr)
		Lf->lts.opt |= SO_REUSEADDR;
	    if (cs->conn_useloopback)
		Lf->lts.opt |= SO_USELOOPBACK;
# else	/* !defined(HAS_CONN_NEW) */
	if (Ftcptpi & TCPTPI_FLAGS) {
	    if (tc->tcp_broadcast)
		Lf->lts.opt |= SO_BROADCAST;
	    if (tc->tcp_debug)
		Lf->lts.opt |= SO_DEBUG;
	    if (tc->tcp_dgram_errind)
		Lf->lts.opt |= SO_DGRAM_ERRIND;
	    if (tc->tcp_dontroute)
		Lf->lts.opt |= SO_DONTROUTE;
	    if (tc->KEEPALIVE_INTERVAL) {
		Lf->lts.opt |= SO_KEEPALIVE;
		Lf->lts.kai = (unsigned int)tc->KEEPALIVE_INTERVAL;
	    }
	    if (tc->tcp_linger) {
		Lf->lts.opt |= SO_LINGER;
		Lf->lts.ltm = (unsigned int)tc->tcp_lingertime;
	    }
	    if (tc->tcp_oobinline)
		Lf->lts.opt |= SO_OOBINLINE;
	    Lf->lts.pqlen = (unsigned int)tc->tcp_conn_req_cnt_q0;
	    Lf->lts.qlen = (unsigned int)tc->tcp_conn_req_cnt_q;
	    Lf->lts.qlim = (unsigned int)tc->tcp_conn_req_max;
	    Lf->lts.pqlens = Lf->lts.qlens = Lf->lts.qlims
			   = (unsigned char)1;

#  if	solaris>=80000
#   if	defined(HAS_IPCLASSIFIER_H)
	    if (tc->tcp_reuseaddr)
#   else /* !defined(HAS_IPCLASSIFIER_H) */
	    if (tb && tb->tcpb_reuseaddr)
#   endif /* !defined(HAS_IPCLASSIFIER_H) */

	    Lf->lts.opt |= SO_REUSEADDR;
#  endif	/* solaris>=80000 */

	    if (tc->tcp_useloopback)
		Lf->lts.opt |= SO_USELOOPBACK;
# endif /* defined(HAS_CONN_NEW) */
#endif	/* defined(HASSOOPT) */

#if	defined(HASTCPOPT)
# if	defined(ACK_TIMER)
#  if	!defined(HAS_CONN_NEW)
	    if (fa && (((tcph_t *)fa)->th_flags[0] & ACK_TIMER))
		Lf->lts.topt |= TF_DELACK;
#  endif /* !defined(HAS_CONN_NEW) */
# endif	/* defined(ACK_TIMER) */

# if	solaris<80000 || defined(HAS_IPCLASSIFIER_H)
	    Lf->lts.mss = (unsigned long)tc->tcp_mss;
# else	/* solaris>=80000 && !defined(HAS_IPCLASSIFIER_H) */
	    if (tb)
		Lf->lts.mss = (unsigned long)tb->tcpb_mss;
# endif	/* solaris<80000 || defined(HAS_IPCLASSIFIER_H) */

	    Lf->lts.msss = (unsigned char)1;
	    if (tc->tcp_naglim == 1L)
		Lf->lts.topt |= TF_NODELAY;
	    if (tc->tcp_fin_sent)
		Lf->lts.topt |= TF_SENTFIN;
	}
#endif	/* defined(HASTCPOPT) */

}
