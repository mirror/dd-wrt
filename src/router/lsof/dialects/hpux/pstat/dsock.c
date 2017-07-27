/*
 * dsock.c -- pstat-based HP-UX socket and stream processing functions for lsof
 */


/*
 * Copyright 1999 Purdue Research Foundation, West Lafayette, Indiana
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
"@(#) Copyright 1999 Purdue Research Foundation.\nAll rights reserved.\n";
static char *rcsid = "$Id";
#endif


#include "lsof.h"


/*
 * Local function prototypes
 */

#if	defined(PS_STR_XPORT_DATA)
_PROTOTYPE(static void make_sock,(struct pst_fileinfo2 *f,
				  struct pst_stream *sh,
				  struct pst_socket *s));
#endif	/* defined(PS_STR_XPORT_DATA) */

_PROTOTYPE(static void printpsproto,(uint32_t p));


/*
 * Local macros
 */

#if	defined(HASIPv6)

/*
 * IPv6_2_IPv4()  -- macro to define the address of an IPv4 address contained
 *		     in an IPv6 address
 */

#define IPv6_2_IPv4(v6)	(((uint8_t *)((struct in6_addr *)v6)->s6_addr)+12)
#endif	/* defined(HASIPv6) */


/*
 * build_IPstates() -- build the TCP and UDP state tables
 */

void
build_IPstates()
{
	if (!TcpSt) {
	    (void) enter_IPstate("TCP", "CLOSED", PS_TCPS_CLOSED);
	    (void) enter_IPstate("TCP", "IDLE", PS_TCPS_IDLE);
	    (void) enter_IPstate("TCP", "BOUND", PS_TCPS_BOUND);
	    (void) enter_IPstate("TCP", "LISTEN", PS_TCPS_LISTEN);
	    (void) enter_IPstate("TCP", "SYN_SENT", PS_TCPS_SYN_SENT);
	    (void) enter_IPstate("TCP", "SYN_RCVD", PS_TCPS_SYN_RCVD);
	    (void) enter_IPstate("TCP", "ESTABLISHED", PS_TCPS_ESTABLISHED);
	    (void) enter_IPstate("TCP", "CLOSE_WAIT", PS_TCPS_CLOSE_WAIT);
	    (void) enter_IPstate("TCP", "FIN_WAIT_1", PS_TCPS_FIN_WAIT_1);
	    (void) enter_IPstate("TCP", "CLOSING", PS_TCPS_CLOSING);
	    (void) enter_IPstate("TCP", "LAST_ACK", PS_TCPS_LAST_ACK);
	    (void) enter_IPstate("TCP", "FIN_WAIT_2", PS_TCPS_FIN_WAIT_2);
	    (void) enter_IPstate("TCP", "TIME_WAIT", PS_TCPS_TIME_WAIT);
	    (void) enter_IPstate("TCP", (char *)NULL, 0);
	}
	if (!UdpSt) {
	    (void) enter_IPstate("UDP", "Uninitialized", PS_TS_UNINIT);
	    (void) enter_IPstate("UDP", "Unbound", PS_TS_UNBND);
	    (void) enter_IPstate("UDP", "Wait_BIND_REQ_Ack", PS_TS_WACK_BREQ);
	    (void) enter_IPstate("UDP", "Wait_UNBIND_REQ_Ack", PS_TS_WACK_UREQ);
	    (void) enter_IPstate("UDP", "Idle", PS_TS_IDLE);
	    (void) enter_IPstate("UDP", "Wait_OPT_REQ_Ack", PS_TS_WACK_OPTREQ);
	    (void) enter_IPstate("UDP", "Wait_CONN_REQ_Ack", PS_TS_WACK_CREQ);
	    (void) enter_IPstate("UDP", "Wait_CONN_REQ_Confirm",
		PS_TS_WCON_CREQ);
	    (void) enter_IPstate("UDP", "Wait_CONN_IND_Response",
		PS_TS_WRES_CIND);
	    (void) enter_IPstate("UDP", "Wait_CONN_RES_Ack", PS_TS_WACK_CRES);
	    (void) enter_IPstate("UDP", "Wait_Data_Xfr", PS_TS_DATA_XFER);
	    (void) enter_IPstate("UDP", "Wait_Read_Release", PS_TS_WIND_ORDREL);
	    (void) enter_IPstate("UDP", "Wait_Write_Release",
		PS_TS_WREQ_ORDREL);
	    (void) enter_IPstate("UDP", "Wait_DISCON_REQ_Ack",
		PS_TS_WACK_DREQ6);
	    (void) enter_IPstate("UDP", "Wait_DISCON_REQ_Ack",
		PS_TS_WACK_DREQ7);
	    (void) enter_IPstate("UDP", "Wait_DISCON_REQ_Ack",
		PS_TS_WACK_DREQ9);
	    (void) enter_IPstate("UDP", "Wait_DISCON_REQ_Ack",
		PS_TS_WACK_DREQ10);
	    (void) enter_IPstate("UDP", "Wait_DISCON_REQ_Ack",
		PS_TS_WACK_DREQ11);
	    (void) enter_IPstate("UDP", "Internal", PS_TS_WACK_ORDREL);
	    (void) enter_IPstate("UDP", (char *)NULL, 0);
	}
}


#if	defined(PS_STR_XPORT_DATA)
/*
 * make_sock() -- make a socket from the eXPORT data in a stream's head
 */

static void
make_sock(f, sh, s)
	struct pst_fileinfo2 *f;		/* pst_fileinfo2 */
	struct pst_stream *sh;			/* stream head */
	struct pst_socket *s;			/* constructed socket */
{
	size_t sz;
/*
 * Zero the destination pst_socket structure  and propagate its file and node
 * IDs from the stream head.  Also propagate the linger time. 
 */
	(void)memset((void *)s, 0, sizeof(struct pst_socket));
	s->pst_hi_fileid = sh->val.head.pst_hi_fileid;
	s->pst_lo_fileid = sh->val.head.pst_lo_fileid;
	s->pst_hi_nodeid = sh->val.head.pst_hi_nodeid;
	s->pst_lo_nodeid = sh->val.head.pst_lo_nodeid;
	s->pst_linger = sh->pst_str_xport_linger;
/*
 * Convert stream family to socket family and stream protocol to socket
 * protocol.
 *
 * This could be avoided if PSTAT were to use a common set of family and
 * protocol symbols.
 */
	switch (sh->pst_str_xport_family) {
	    case PS_STR_XPORT_AFINET:
		s->pst_family = PS_AF_INET;
		break;
	    case PS_STR_XPORT_AFINET6:
		s->pst_family = PS_AF_INET6;
		break;
	    default:
		s->pst_family = sh->pst_str_xport_family;
	}
	switch (sh->pst_str_xport_protocol) {
	    case PS_STR_XPORT_TCP_PROTO:
		s->pst_protocol = PS_PROTO_TCP;
		break;
	    case PS_STR_XPORT_UDP_PROTO:
		s->pst_protocol = PS_PROTO_UDP;
		break;
	    default:
		s->pst_protocol = sh->pst_str_xport_protocol;
	}
/*
 * Copy stream size information.
 */
	s->pst_qlimit = sh->pst_str_xport_qlimit;
	s->pst_qlen = sh->pst_str_xport_qlen;
	s->pst_idata = sh->pst_str_xport_idata;
	s->pst_ibufsz = sh->pst_str_xport_ibufsz;
	s->pst_rwnd = sh->pst_str_xport_rwnd;
	s->pst_swnd = sh->pst_str_xport_swnd;
	s->pst_odata = sh->pst_str_xport_odata;
	s->pst_obufsz = sh->pst_str_xport_obufsz;
/*
 * Propagate protocol state from stream symbol values to socket ones.
 *
 * This could be avoided if PSTAT were to use a common set of protocol
 * state symbols.
 */
	if (s->pst_protocol == PS_PROTO_TCP) {
	    switch (sh->pst_str_xport_pstate) {

#if	defined(PS_STR_XPORT_TCPS_CLOSED) && defined(PS_TCPS_CLOSED) \
	&& (PS_STR_XPORT_TCPS_CLOSED != PS_TCPS_CLOSED)
	    case PS_STR_XPORT_TCPS_CLOSED:
		s->pst_pstate = PS_TCPS_CLOSED;
		break;
#endif

#if	defined(PS_STR_XPORT_TCPS_IDLE) && defined(PS_TCPS_IDLE) \
	&& (PS_STR_XPORT_TCPS_IDLE != PS_TCPS_IDLE)
	    case PS_STR_XPORT_TCPS_IDLE:
		s->pst_pstate = PS_TCPS_IDLE;
		break;
#endif

#if	defined(PS_STR_XPORT_TCPS_BOUND) && defined(PS_TCPS_BOUND) \
	&& (PS_STR_XPORT_TCPS_BOUND != PS_TCPS_BOUND)
	    case PS_STR_XPORT_TCPS_BOUND:
		s->pst_pstate = PS_TCPS_BOUND;
		break;
#endif

#if	defined(PS_STR_XPORT_TCPS_LISTEN) && defined(PS_TCPS_LISTEN) \
	&& (PS_STR_XPORT_TCPS_LISTEN != PS_TCPS_LISTEN)
	    case PS_STR_XPORT_TCPS_LISTEN:
		s->pst_pstate = PS_TCPS_LISTEN;
		break;
#endif

#if	defined(PS_STR_XPORT_TCPS_SYN_SENT) && defined(PS_TCPS_SYN_SENT) \
	&& (PS_STR_XPORT_TCPS_SYN_SENT != PS_TCPS_SYN_SENT)
	    case PS_STR_XPORT_TCPS_SYN_SENT:
		s->pst_pstate = PS_TCPS_SYN_SENT;
		break;
#endif

#if	defined(PS_STR_XPORT_TCPS_SYN_RCVD) && defined(PS_TCPS_SYN_RCVD) \
	&& (PS_STR_XPORT_TCPS_SYN_RCVD != PS_TCPS_SYN_RCVD)
	    case PS_STR_XPORT_TCPS_SYN_RCVD:
		s->pst_pstate = PS_TCPS_SYN_RCVD;
		break;
#endif

#if	defined(PS_STR_XPORT_TCPS_ESTABLISHED) && defined(PS_TCPS_ESTABLISHED) \
	&& (PS_STR_XPORT_TCPS_ESTABLISHED != PS_TCPS_ESTABLISHED)
	    case PS_STR_XPORT_TCPS_ESTABLISHED:
		s->pst_pstate = PS_TCPS_ESTABLISHED;
		break;
#endif

#if	defined(PS_STR_XPORT_TCPS_CLOSE_WAIT) && defined(PS_TCPS_CLOSE_WAIT) \
	&& (PS_STR_XPORT_TCPS_CLOSE_WAIT != PS_TCPS_CLOSE_WAIT)
	    case PS_STR_XPORT_TCPS_CLOSE_WAIT:
		s->pst_pstate = PS_TCPS_CLOSE_WAIT;
		break;
#endif

#if	defined(PS_STR_XPORT_TCPS_FIN_WAIT_1) && defined(PS_TCPS_FIN_WAIT_1) \
	&& (PS_STR_XPORT_TCPS_FIN_WAIT_1 != PS_TCPS_FIN_WAIT_1)
	    case PS_STR_XPORT_TCPS_FIN_WAIT_1:
		s->pst_pstate = PS_TCPS_FIN_WAIT_1;
		break;
#endif

#if	defined(PS_STR_XPORT_TCPS_CLOSING) && defined(PS_TCPS_CLOSING) \
	&& (PS_STR_XPORT_TCPS_CLOSING != PS_TCPS_CLOSING)
	    case PS_STR_XPORT_TCPS_CLOSING:
		s->pst_pstate = PS_TCPS_CLOSING;
		break;
#endif

#if	defined(PS_STR_XPORT_TCPS_LAST_ACK) && defined(PS_TCPS_LAST_ACK) \
	&& (PS_STR_XPORT_TCPS_LAST_ACK != PS_TCPS_LAST_ACK)
	    case PS_STR_XPORT_TCPS_LAST_ACK:
		s->pst_pstate = PS_TCPS_LAST_ACK;
		break;
#endif

#if	defined(PS_STR_XPORT_TCPS_FIN_WAIT_2) && defined(PS_TCPS_FIN_WAIT_2) \
	&& (PS_STR_XPORT_TCPS_FIN_WAIT_2 != PS_TCPS_FIN_WAIT_2)
	    case PS_STR_XPORT_TCPS_FIN_WAIT_2:
		s->pst_pstate = PS_TCPS_FIN_WAIT_2;
		break;
#endif

#if	defined(PS_STR_XPORT_TCPS_TIME_WAIT) && defined(PS_TCPS_TIME_WAIT) \
	&& (PS_STR_XPORT_TCPS_TIME_WAIT != PS_TCPS_TIME_WAIT)
	    case PS_STR_XPORT_TCPS_TIME_WAIT:
		s->pst_pstate = PS_TCPS_TIME_WAIT;
		break;
#endif

	    default:
		s->pst_pstate = sh->pst_str_xport_pstate;
	    }
	} else if (s->pst_protocol == PS_PROTO_UDP) {
	    switch (sh->pst_str_xport_pstate) {

#if	defined(PS_STR_XPORT_TS_UNINIT) && defined(PS_TS_UNINIT) \
	&& (PS_STR_XPORT_TS_UNINIT != PS_TS_UNINIT)
	    case PS_STR_XPORT_TS_UNINIT:
		s->pst_pstate = PS_TS_UNINIT;
		break;
#endif

#if	defined(PS_STR_XPORT_TS_UNBND) && defined(PS_TS_UNBND) \
	&& (PS_STR_XPORT_TS_UNBND != PS_TS_UNBND)
	    case PS_STR_XPORT_TS_UNBND:
		s->pst_pstate = PS_TS_UNBND;
		break;
#endif

#if	defined(PS_STR_XPORT_TS_WACK_BREQ) && defined(PS_TS_WACK_BREQ) \
	&& (PS_STR_XPORT_TS_WACK_BREQ != PS_TS_WACK_BREQ)
	    case PS_STR_XPORT_TS_WACK_BREQ:
		s->pst_pstate = PS_TS_WACK_BREQ;
		break;
#endif

#if	defined(PS_STR_XPORT_TS_WACK_UREQ) && defined(PS_TS_WACK_UREQ) \
	&& (PS_STR_XPORT_TS_WACK_UREQ != PS_TS_WACK_UREQ)
	    case PS_STR_XPORT_TS_WACK_UREQ:
		s->pst_pstate = PS_TS_WACK_UREQ;
		break;
#endif

#if	defined(PS_STR_XPORT_TS_IDLE) && defined(PS_TS_IDLE) \
	&& (PS_STR_XPORT_TS_IDLE != PS_TS_IDLE)
	    case PS_STR_XPORT_TS_IDLE:
		s->pst_pstate = PS_TS_IDLE;
		break;
#endif

#if	defined(PS_STR_XPORT_TS_WACK_OPTREQ) && defined(PS_TS_WACK_OPTREQ) \
	&& (PS_STR_XPORT_TS_WACK_OPTREQ != PS_TS_WACK_OPTREQ)
	    case PS_STR_XPORT_TS_WACK_OPTREQ:
		s->pst_pstate = PS_TS_WACK_OPTREQ;
		break;
#endif

#if	defined(PS_STR_XPORT_TS_WACK_CREQ) && defined(PS_TS_WACK_CREQ) \
	&& (PS_STR_XPORT_TS_WACK_CREQ != PS_TS_WACK_CREQ)
	    case PS_STR_XPORT_TS_WACK_CREQ:
		s->pst_pstate = PS_TS_WACK_CREQ;
		break;
#endif

#if	defined(PS_STR_XPORT_TS_WCON_CREQ) && defined(PS_TS_WCON_CREQ) \
	&& (PS_STR_XPORT_TS_WCON_CREQ != PS_TS_WCON_CREQ)
	    case PS_STR_XPORT_TS_WCON_CREQ:
		s->pst_pstate = PS_TS_WCON_CREQ;
		break;
#endif

#if	defined(PS_STR_XPORT_TS_WRES_CIND) && defined(PS_TS_WRES_CIND) \
	&& (PS_STR_XPORT_TS_WRES_CIND != PS_TS_WRES_CIND)
	    case PS_STR_XPORT_TS_WRES_CIND:
		s->pst_pstate = PS_TS_WRES_CIND;
		break;
#endif

#if	defined(PS_STR_XPORT_TS_WACK_CRES) && defined(PS_TS_WACK_CRES) \
	&& (PS_STR_XPORT_TS_WACK_CRES != PS_TS_WACK_CRES)
	    case PS_STR_XPORT_TS_WACK_CRES:
		s->pst_pstate = PS_TS_WACK_CRES;
		break;
#endif

#if	defined(PS_STR_XPORT_TS_DATA_XFER) && defined(PS_TS_DATA_XFER) \
	&& (PS_STR_XPORT_TS_DATA_XFER != PS_TS_DATA_XFER)
	    case PS_STR_XPORT_TS_DATA_XFER:
		s->pst_pstate = PS_TS_DATA_XFER;
		break;
#endif

#if	defined(PS_STR_XPORT_TS_WIND_ORDREL) && defined(PS_TS_WIND_ORDREL) \
	&& (PS_STR_XPORT_TS_WIND_ORDREL != PS_TS_WIND_ORDREL)
	    case PS_STR_XPORT_TS_WIND_ORDREL:
		s->pst_pstate = PS_TS_WIND_ORDREL;
		break;
#endif

#if	defined(PS_STR_XPORT_TS_WREQ_ORDREL) && defined(PS_TS_WREQ_ORDREL) \
	&& (PS_STR_XPORT_TS_WREQ_ORDREL != PS_TS_WREQ_ORDREL)
	    case PS_STR_XPORT_TS_WREQ_ORDREL:
		s->pst_pstate = PS_TS_WREQ_ORDREL;
		break;
#endif

#if	defined(PS_STR_XPORT_TS_WACK_DREQ6) && defined(PS_TS_WACK_DREQ6) \
	&& (PS_STR_XPORT_TS_WACK_DREQ6 != PS_TS_WACK_DREQ6)
	    case PS_STR_XPORT_TS_WACK_DREQ6:
		s->pst_pstate = PS_TS_WACK_DREQ6;
		break;
#endif

#if	defined(PS_STR_XPORT_TS_WACK_DREQ7) && defined(PS_TS_WACK_DREQ7) \
	&& (PS_STR_XPORT_TS_WACK_DREQ7 != PS_TS_WACK_DREQ7)
	    case PS_STR_XPORT_TS_WACK_DREQ7:
		s->pst_pstate = PS_TS_WACK_DREQ7;
		break;
#endif

#if	defined(PS_STR_XPORT_TS_WACK_DREQ9) && defined(PS_TS_WACK_DREQ9) \
	&& (PS_STR_XPORT_TS_WACK_DREQ9 != PS_TS_WACK_DREQ9)
	    case PS_STR_XPORT_TS_WACK_DREQ9:
		s->pst_pstate = PS_TS_WACK_DREQ9;
		break;
#endif

#if	defined(PS_STR_XPORT_TS_WACK_DREQ10) && defined(PS_TS_WACK_DREQ10) \
	&& (PS_STR_XPORT_TS_WACK_DREQ10 != PS_TS_WACK_DREQ10)
	    case PS_STR_XPORT_TS_WACK_DREQ10:
		s->pst_pstate = PS_TS_WACK_DREQ10;
		break;
#endif

#if	defined(PS_STR_XPORT_TS_WACK_DREQ11) && defined(PS_TS_WACK_DREQ11) \
	&& (PS_STR_XPORT_TS_WACK_DREQ11 != PS_TS_WACK_DREQ11)
	    case PS_STR_XPORT_TS_WACK_DREQ11:
		s->pst_pstate = PS_TS_WACK_DREQ11;
		break;
#endif

#if	defined(PS_STR_XPORT_TS_WACK_ORDREL) && defined(PS_TS_WACK_ORDREL) \
	&& (PS_STR_XPORT_TS_WACK_ORDREL != PS_TS_WACK_ORDREL)
	    case PS_STR_XPORT_TS_WACK_ORDREL:
		s->pst_pstate = PS_TS_WACK_ORDREL;
		break;
#endif

#if	defined(PS_STR_XPORT_TS_NOSTATES) && defined(PS_TS_NOSTATES) \
	&& (PS_STR_XPORT_TS_NOSTATES != PS_TS_NOSTATES)
	    case PS_STR_XPORT_TS_NOSTATES:
		s->pst_pstate = PS_TS_NOSTATES;
		break;
#endif

	    default:
		s->pst_pstate = sh->pst_str_xport_pstate;
	    }
	} else
	    s->pst_pstate = sh->pst_str_xport_pstate;
/*
 * Now propagate the bound and remote address information from pst_stream
 * to the pst_socket structure.  Validate the copy lengths.
 */
	sz = (size_t)sh->pst_str_xport_boundaddr_len;
	if (sz > sizeof(s->pst_boundaddr))
	    sz = sizeof(s->pst_boundaddr);
	if ((s->pst_boundaddr_len = sz)) {
	    (void) memcpy((void *)s->pst_boundaddr,
			  (const void *)sh->pst_str_xport_boundaddr, sz);
	}
	sz = (size_t)sh->pst_str_xport_remaddr_len;
	if (sz > sizeof(s->pst_remaddr))
	    sz = sizeof(s->pst_remaddr);
	if ((s->pst_remaddr_len = sz)) {
	    (void) memcpy((void *)s->pst_remaddr,
			  (const void *)sh->pst_str_xport_remaddr, sz);
	}
}
#endif	/* defined(PS_STR_XPORT_DATA) */


/*
 * printpsproto() -- print PSTAT protocol name
 */

static void
printpsproto(p)
	uint32_t p;			/* protocol number */
{
	int i;
	static int m = -1;
	char *s;

	switch (p) {
	case PS_PROTO_IP:
	    s = "IP";
	    break;
	case PS_PROTO_ICMP:
	    s = "ICMP";
	    break;
	case PS_PROTO_IGMP:
	    s = "IGMP";
	    break;
	case PS_PROTO_GGP:
	    s = "GGP";
	    break;
	case PS_PROTO_IPIP:
	    s = "IPIP";
	    break;
	case PS_PROTO_TCP:
	    s = "TCP";
	    break;
	case PS_PROTO_EGP:
	    s = "EGP";
	    break;
	case PS_PROTO_IGP:
	    s = "IGP";
	    break;
	case PS_PROTO_PUP:
	    s = "PUP";
	    break;
	case PS_PROTO_UDP:
	    s = "UDP";
	    break;
	case PS_PROTO_IDP:
	    s = "IDP";
	    break;
	case PS_PROTO_XTP:
	    s = "XTP";
	    break;
	case PS_PROTO_ESP:
	    s = "ESP";
	    break;
	case PS_PROTO_AH:
	    s = "AH";
	    break;
	case PS_PROTO_OSPF:
	    s = "OSPF";
	    break;
	case PS_PROTO_IPENCAP:
	    s = "IPENCAP";
	    break;
	case PS_PROTO_ENCAP:
	    s = "ENCAP";
	    break;
	case PS_PROTO_PXP:
	    s = "PXP";
	    break;
	case PS_PROTO_RAW:
	    s = "RAW";
	    break;
	default:
	    s = (char *)NULL;
	}
	if (s)
	    (void) snpf(Lf->iproto, sizeof(Lf->iproto), "%.*s", IPROTOL-1, s);
	else {
	    if (m < 0) {
		for (i = 0, m = 1; i < IPROTOL-2; i++)
		    m *= 10;
	    }
	    if (m > p)
		(void) snpf(Lf->iproto, sizeof(Lf->iproto), "%d?", p);
	    else
		(void) snpf(Lf->iproto, sizeof(Lf->iproto), "*%d?",
		    p % (m/10));
	}
}


/*
 * print_tcptpi() -- print TCP/TPI info
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
		    (void) snpf(sbuf, sizeof(sbuf), "UknownState_%d",
			Lf->lts.state.i);
		    cp = sbuf;
		} else
		    cp = TcpSt[i];
		break;
	    case 1:				/* UDP */
		if (!UdpSt)
		    (void) build_IPstates();
		if ((u = Lf->lts.state.ui + UdpStOff) > UdpNstates) {
		    (void) snpf(sbuf, sizeof(sbuf), "UNKNOWN_TPI_STATE_%u",
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

	    if ((opt = Lf->lts.opt) || Lf->lts.qlens || Lf->lts.qlims) {
		char sep = ' ';

		if (Ffield)
		    sep = LSOF_FID_TCPTPI;
		else if (!ps)
		    sep = '(';
		(void) printf("%cSO", sep);
		ps++;
		sep = '=';

# if	defined(PS_SO_ACCEPTCONN)
		if (opt & PS_SO_ACCEPTCONN) {
		    (void) printf("%cACCEPTCONN", sep);
		    opt &= ~PS_SO_ACCEPTCONN;
		    sep = ',';
		}
# endif	/* defined(PS_SO_ACCEPTCONN) */

# if	defined(PS_SO_BROADCAST)
		if (opt & PS_SO_BROADCAST) {
		    (void) printf("%cBROADCAST", sep);
		    opt &= ~PS_SO_BROADCAST;
		    sep = ',';
		}
# endif	/* defined(PS_SO_BROADCAST) */

# if	defined(PS_SO_DEBUG)
		if (opt & PS_SO_DEBUG) {
		    (void) printf("%cDEBUG", sep);
		    opt &= ~PS_SO_DEBUG;
		    sep = ',';
		}
# endif	/* defined(PS_SO_DEBUG) */

# if	defined(PS_SO_DONTROUTE)
		if (opt & PS_SO_DONTROUTE) {
		    (void) printf("%cDONTROUTE", sep);
		    opt &= ~PS_SO_DONTROUTE;
		    sep = ',';
		}
# endif	/* defined(PS_SO_DONTROUTE) */

# if	defined(PS_SO_GETIFADDR)
		if (opt & PS_SO_GETIFADDR) {
		    (void) printf("%cGETIFADDR", sep);
		    opt &= ~PS_SO_GETIFADDR;
		    sep = ',';
		}
# endif	/* defined(PS_SO_GETIFADDR) */

# if	defined(PS_SO_INPCB_COPY)
		if (opt & PS_SO_INPCB_COPY) {
		    (void) printf("%cINPCB_COPY", sep);
		    opt &= ~PS_SO_INPCB_COPY;
		    sep = ',';
		}
# endif	/* defined(PS_SO_INPCB_COPY) */

# if	defined(PS_SO_KEEPALIVE)
		if (opt & PS_SO_KEEPALIVE) {
		    (void) printf("%cKEEPALIVE", sep);
		    if (Lf->lts.kai)
			(void) printf("=%d", Lf->lts.kai);
		    opt &= ~PS_SO_KEEPALIVE;
		    sep = ',';
		}
# endif	/* defined(PS_SO_KEEPALIVE) */

# if	defined(PS_SO_LINGER)
		if (opt & PS_SO_LINGER) {
		    (void) printf("%cLINGER", sep);
		    if (Lf->lts.ltm)
			(void) printf("=%d", Lf->lts.ltm);
		    opt &= ~PS_SO_LINGER;
		    sep = ',';
		}
# endif	/* defined(PS_SO_LINGER) */

# if	defined(PS_SO_OOBINLINE)
		if (opt & PS_SO_OOBINLINE) {
		    (void) printf("%cOOBINLINE", sep);
		    opt &= ~PS_SO_OOBINLINE;
		    sep = ',';
		}
# endif	/* defined(PS_SO_OOBINLINE) */

# if	defined(PS_SO_PMTU)
		if (opt & PS_SO_PMTU) {
		    (void) printf("%cPMTU", sep);
		    opt &= ~PS_SO_PMTU;
		    sep = ',';
		}
# endif	/* defined(PS_SO_PMTU) */

		if (Lf->lts.qlens) {
		    (void) printf("%cQLEN=%u", sep, Lf->lts.qlen);
		    sep = ',';
		}
		if (Lf->lts.qlims) {
		    (void) printf("%cQLIM=%u", sep, Lf->lts.qlim);
		    sep = ',';
		}

# if	defined(PS_SO_REUSEADDR)
		if (opt & PS_SO_REUSEADDR) {
		    (void) printf("%cREUSEADDR", sep);
		    opt &= ~PS_SO_REUSEADDR;
		    sep = ',';
		}
# endif	/* defined(PS_SO_REUSEADDR) */

# if	defined(PS_SO_REUSEPORT)
		if (opt & PS_SO_REUSEPORT) {
		    (void) printf("%cREUSEPORT", sep);
		    opt &= ~PS_SO_REUSEPORT;
		    sep = ',';
		}
# endif	/* defined(PS_SO_REUSEPORT) */

# if	defined(PS_SO_USELOOPBACK)
		if (opt & PS_SO_USELOOPBACK) {
		    (void) printf("%cUSELOOPBACK", sep);
		    opt &= ~PS_SO_USELOOPBACK;
		    sep = ',';
		}
# endif	/* defined(PS_SO_USELOOPBACK) */

		if (opt)
		    (void) printf("%cUNKNOWN=%#x", sep, opt);
		if (Ffield)
		    putchar(Terminator);
	    }
	}
#endif	/* defined(HASSOOPT) */

#if	defined(HASSOSTATE)
	if (Ftcptpi & TCPTPI_FLAGS) {
	    int ss;

	    if ((ss = Lf->lts.ss)) {
		char sep = ' ';

		if (Ffield)
		    sep = LSOF_FID_TCPTPI;
		else if (!ps)
		    sep = '(';
		(void) printf("%cSS", sep);
		ps++;
		sep = '=';

# if	defined(PS_SS_ASYNC)
		if (ss & PS_SS_ASYNC) {
		    (void) printf("%cASYNC", sep);
		    ss &= ~PS_SS_ASYNC;
		    sep = ',';
		}
# endif	/* defined(PS_SS_ASYNC) */

# if	defined(PS_SS_BOUND)
		if (ss & PS_SS_BOUND) {
		    (void) printf("%cBOUND", sep);
		    ss &= ~PS_SS_BOUND;
		    sep = ',';
		}
# endif	/* defined(PS_SS_BOUND) */

# if	defined(PS_SS_CANTRCVMORE)
		if (ss & PS_SS_CANTRCVMORE) {
		    (void) printf("%cCANTRCVMORE", sep);
		    ss &= ~PS_SS_CANTRCVMORE;
		    sep = ',';
		}
# endif	/* defined(PS_SS_CANTRCVMORE) */

# if	defined(PS_SS_CANTSENDMORE)
		if (ss & PS_SS_CANTSENDMORE) {
		    (void) printf("%cCANTSENDMORE", sep);
		    ss &= ~PS_SS_CANTSENDMORE;
		    sep = ',';
		}
# endif	/* defined(PS_SS_CANTSENDMORE) */

# if	defined(PS_SS_ISCONNECTED)
		if (ss & PS_SS_ISCONNECTED) {
		    (void) printf("%cISCONNECTED", sep);
		    ss &= ~PS_SS_ISCONNECTED;
		    sep = ',';
		}
# endif	/* defined(PS_SS_ISCONNECTED) */

# if	defined(PS_SS_ISCONNECTING)
		if (ss & PS_SS_ISCONNECTING) {
		    (void) printf("%cISCONNECTING", sep);
		    ss &= ~PS_SS_ISCONNECTING;
		    sep = ',';
		}
# endif	/* defined(PS_SS_ISCONNECTING) */

# if	defined(PS_SS_ISDISCONNECTI)
		if (ss & PS_SS_ISDISCONNECTI) {
		    (void) printf("%cISDISCONNECTI", sep);
		    ss &= ~PS_SS_ISDISCONNECTI;
		    sep = ',';
		}
# endif	/* defined(PS_SS_ISDISCONNECTI) */

# if	defined(PS_SS_INTERRUPTED)
		if (ss & PS_SS_INTERRUPTED) {
		    (void) printf("%cINTERRUPTED", sep);
		    ss &= ~PS_SS_INTERRUPTED;
		    sep = ',';
		}
# endif	/* defined(PS_SS_INTERRUPTED) */

# if	defined(PS_SS_NBIO)
		if (ss & PS_SS_NBIO) {
		    (void) printf("%cNBIO", sep);
		    ss &= ~PS_SS_NBIO;
		    sep = ',';
		}
# endif	/* defined(PS_SS_NBIO) */

# if	defined(PS_SS_NOFDREF)
		if (ss & PS_SS_NOFDREF) {
		    (void) printf("%cNOFDREF", sep);
		    ss &= ~PS_SS_NOFDREF;
		    sep = ',';
		}
# endif	/* defined(PS_SS_NOFDREF) */

# if	defined(PS_SS_NOUSER)
		if (ss & PS_SS_NOUSER) {
		    (void) printf("%cNOUSER", sep);
		    ss &= ~PS_SS_NOUSER;
		    sep = ',';
		}
# endif	/* defined(PS_SS_NOUSER) */

# if	defined(PS_SS_NOWAIT)
		if (ss & PS_SS_NOWAIT) {
		    (void) printf("%cNOWAIT", sep);
		    ss &= ~PS_SS_NOWAIT;
		    sep = ',';
		}
# endif	/* defined(PS_SS_NOWAIT) */

# if	defined(PS_SS_PRIV)
		if (ss & PS_SS_PRIV) {
		    (void) printf("%cPRIV", sep);
		    ss &= ~PS_SS_PRIV;
		    sep = ',';
		}
# endif	/* defined(PS_SS_PRIV) */

# if	defined(PS_SS_RCVATMARK)
		if (ss & PS_SS_RCVATMARK) {
		    (void) printf("%cRCVATMARK", sep);
		    ss &= ~PS_SS_RCVATMARK;
		    sep = ',';
		}
# endif	/* defined(PS_SS_RCVATMARK) */

# if	defined(PS_SS_XOPEN_EXT1)
		if (ss & PS_SS_XOPEN_EXT1) {
		    (void) printf("%cXOPEN_EXT1", sep);
		    ss &= ~PS_SS_XOPEN_EXT1;
		    sep = ',';
		}
# endif	/* defined(PS_SS_XOPEN_EXT1) */

		if (ss)
		    (void) printf("%cUNKNOWN=%#x", sep, ss);
		if (Ffield)
		    putchar(Terminator);
	    }
	}
#endif	/* defined(HASSOSTATE) */

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


/*
 * process_socket() -- process socket
 */

void
process_socket(f, s)
	struct pst_fileinfo2 *f;		/* file information */
	struct pst_socket *s;			/* optional socket information
						 * NULL == none */
{
	int af, err, fp, lp, tx;
	char buf[1024], tbuf[32];
	unsigned char *fa = (unsigned char *)NULL;
	unsigned char *la = (unsigned char *)NULL;
	size_t len;
	KA_T na, nau;
	char *nma = (char *)NULL;
	struct pst_filedetails pd;
	struct sockaddr_in *sa;
	int sx;

#if	defined(HASIPv6)
	struct sockaddr_in6 *sa6;
#endif	/* defined(HASIPv6) */

	struct sockaddr_un *ua;
/*
 * Read socket info, as required, so that the protocol state names can be
 * tested as soon as possible.
 */
	if (!s) {
	    if (!(s = read_sock(f))) {
		(void) snpf(Namech, Namechl,
		    "can't read pst_socket%s%s", errno ? ": " : "",
		    errno ? strerror(errno) : "");
		(void) enter_nm(Namech);
		return;
	    }
	}
/*
 * Collect protocol details so the protocol state name might be tested,
 * as requested by options.
 */
	switch (s->pst_family) {
	case PS_AF_INET:
	    af = 4;
	    break;

#if	defined(HASIPv6)
	case PS_AF_INET6:
	    af = 6;
	    break;
#endif	/* defined(HASIPv6) */

	default:
	    af = -1;
	}
	switch (s->pst_protocol) {
	case PS_PROTO_TCP:
	    sx = (int)s->pst_pstate + TcpStOff;
	    tx = 0;
	    break;
	case PS_PROTO_UDP:
	    sx = (unsigned int)s->pst_pstate + UdpStOff;
	    tx = 1;
	    break;
	default:
	    sx = tx = -1;
	}
/*
 * Test the protocol state and name, setting the SELNET flag where possible.
 */
	switch (tx) {
	case 0:					/* TCP */
	    if (TcpStXn) {

	    /*
	     * Check for TCP state exclusion.
	     */
		if (sx >= 0 && sx < TcpNstates) {
		    if (TcpStX[sx]) {
			Lf->sf |= SELEXCLF;
			return;
		    }
		}
	    }
	    if (TcpStIn) {
		if (sx >= 0 && sx < TcpNstates) {
		    if (TcpStI[sx])
			TcpStI[sx] = 2;
		    else {
			Lf->sf |= SELEXCLF;
			return;
		    }
		}
	    }
	    break;
	case 1:					/* UDP */
	    if (UdpStXn) {

	    /*
	     * Check for UDP state exclusion.
	     */
		if (sx >= 0 && sx < UdpNstates) {
		    if (UdpStX[sx]) {
			Lf->sf |= SELEXCLF;
			return;
		    }
		}
	    }
	    if (UdpStIn) {
		if (sx >= 0 && sx < UdpNstates) {
		    if (UdpStI[sx])
			UdpStI[sx] = 2;
		    else {
			Lf->sf |= SELEXCLF;
			return;
		    }
		}
	    }
	    break;
	}
/*
 * Set default type.
 */
	(void) snpf(Lf->type, sizeof(Lf->type), "sock");
	Lf->inp_ty = 2;
/*
 * Generate and save node ID.
 */
	na = (KA_T)(((KA_T)(f->psf_hi_nodeid & 0xffffffff) << 32)
	   |        (KA_T)(f->psf_lo_nodeid & 0xffffffff));

#if	defined(HASFSTRUCT)
	if (na && (Fsv & FSV_NI)) {
	    if (na) {
		Lf->fna = na;
		Lf->fsv |= FSV_NI;
	    }
	}
#endif	/* defined(HASFSTRUCT) */

/*
 * Save size information, as requested.
 */
	if (Fsize) {
	    if (Lf->access == 'r')
		Lf->sz = (SZOFFTYPE)s->pst_idata;
	    else if (Lf->access == 'w')
		Lf->sz = (SZOFFTYPE)s->pst_odata;
	    else
		Lf->sz = (SZOFFTYPE)(s->pst_idata + s->pst_odata);
	    Lf->sz_def = 1;
	} else
	    Lf->off_def = 1;
	
#if     defined(HASTCPTPIQ)
/*
 * Enter queue sizes.
 */
	switch (s->pst_family) {
	case PS_AF_INET:
	case PS_AF_INET6:
	    Lf->lts.rq = (unsigned long)s->pst_idata;
	    Lf->lts.sq = (unsigned long)s->pst_odata;
	    Lf->lts.rqs = Lf->lts.sqs = (unsigned char)1;
	}
#endif  /* defined(HASTCPTPIQ) */

#if	defined(HASSOOPT)
/*
 * Enter socket options.
 */
	Lf->lts.opt = (unsigned int)s->pst_options;
	Lf->lts.ltm = (unsigned int)s->pst_linger;
	Lf->lts.qlen = (unsigned int)s->pst_qlen;
	Lf->lts.qlim = (unsigned int)s->pst_qlimit;
	Lf->lts.qlens = Lf->lts.qlims = (unsigned char)1;
#endif	/* defined(HASSOOPT) */

#if	defined(HASSOSTATE)
/*
 * Enter socket state flags.
 */
	Lf->lts.ss = (unsigned int)s->pst_state;
#endif	/* defined(HASSOSTATE) */

#if	defined(HASTCPTPIW)
/*
 * Enter window sizes.
 */
	switch (s->pst_family) {
	case PS_AF_INET:
	case PS_AF_INET6:
	    Lf->lts.rw = (unsigned long)s->pst_rwnd;
	    Lf->lts.ww = (unsigned long)s->pst_swnd;
	    Lf->lts.rws = Lf->lts.wws = (unsigned char)1;
	}
#endif	/* defined(HASTCPTPIW) */

/*
 * Process socket by the associated domain family.
 */
	switch (s->pst_family) {
	case PS_AF_INET:
	    if (Fnet && (!FnetTy || (FnetTy != 6)))
		Lf->sf |= SELNET;
	    (void) snpf(Lf->type, sizeof(Lf->type),
	    
#if	defined(HASIPv6)
		"IPv4"
#else	/* !defined(HASIPv6) */
		"inet"
#endif	/* defined(HASIPv6) */

	    );
	    printpsproto(s->pst_protocol);
	    enter_dev_ch(print_kptr(na, (char *)NULL, 0));
	    switch (s->pst_protocol) {
	    case PS_PROTO_TCP:
		Lf->lts.type = 0;
		Lf->lts.state.i = (int)s->pst_pstate;
		break;
	    case PS_PROTO_UDP:
		Lf->lts.type = 1;
		Lf->lts.state.ui = (unsigned int)s->pst_pstate;
	    }
	/*
	 * Enter local and remote addresses, being careful to generate
	 * proper IPv4 address alignment by copying, since IPv4 addresses
	 * may not be properly aligned in pst_boundaddr[] and pst_remaddr[].
	 */
	    if ((size_t)s->pst_boundaddr_len == sizeof(struct sockaddr_in)) {
		sa = (struct sockaddr_in *)s->pst_boundaddr;
		la = (unsigned char *)&sa->sin_addr;
		lp = (int)htons(sa->sin_port);
	    }
	    if ((size_t)s->pst_remaddr_len == sizeof(struct sockaddr_in)) {
		sa = (struct sockaddr_in *)s->pst_remaddr;
		fp = (int)htons(sa->sin_port);
		if ((sa->sin_addr.s_addr != INADDR_ANY) || fp)
		    fa = (unsigned char *)&sa->sin_addr;
	    }
	    if (fa || la)
		(void) ent_inaddr(la, lp, fa, fp, AF_INET);
	    break;

#if	defined(HASIPv6)
	case PS_AF_INET6:
	    af = AF_INET6;
	    if (Fnet && (!FnetTy || (FnetTy != 4)))
		Lf->sf |= SELNET;
	    (void) snpf(Lf->type, sizeof(Lf->type), "IPv6");
	    printpsproto(s->pst_protocol);
	    enter_dev_ch(print_kptr(na, (char *)NULL, 0));
	    switch (s->pst_protocol) {
	    case PS_PROTO_TCP:
		Lf->lts.type = 0;
		Lf->lts.state.i = (int)s->pst_pstate;
		break;
	    case PS_PROTO_UDP:
		Lf->lts.type = 1;
		Lf->lts.state.ui = (unsigned int)s->pst_pstate;
	    }
	/*
	 * Enter local and remote addresses, being careful to generate
	 * proper IPv6 address alignment by copying, since IPv6 addresses
	 * may not be properly aligned in pst_boundaddr[] and pst_remaddr[].
	 */
	    if ((size_t)s->pst_boundaddr_len == sizeof(struct sockaddr_in6)) {
		sa6 = (struct sockaddr_in6 *)s->pst_boundaddr;
		la = (unsigned char *)&sa6->sin6_addr;
		lp = (int)htons(sa6->sin6_port);
	    }
	    if ((size_t)s->pst_remaddr_len == sizeof(struct sockaddr_in6)) {
		sa6 = (struct sockaddr_in6 *)s->pst_remaddr;
		if ((fp = (int)htons(sa6->sin6_port))
		||  !IN6_IS_ADDR_UNSPECIFIED(&sa6->sin6_addr))
		    fa = (unsigned char *)&sa6->sin6_addr;
	    }
	    if (la || fa) {
		if ((la && IN6_IS_ADDR_V4MAPPED((struct in6_addr *)la))
		||  (fa && IN6_IS_ADDR_V4MAPPED((struct in6_addr *)fa)))
		{
		    if (la)
			la = (unsigned char *)IPv6_2_IPv4(la);
		    if (fa)
			fa = (unsigned char *)IPv6_2_IPv4(fa);
		    af = AF_INET;
		}
	    }
	    if (fa || la)
		(void) ent_inaddr(la, lp, fa, fp, af);
	    break;
#endif	/* defined(HASIPv6) */

	case PS_AF_UNIX:
	    if (Funix)
		Lf->sf |= SELUNX;
	    (void) snpf(Lf->type, sizeof(Lf->type), "unix");
	    if (((len = (size_t)s->pst_boundaddr_len) > 0)
	    &&  (len <= sizeof(struct sockaddr_un)))
	    {
		ua = (struct sockaddr_un *)s->pst_boundaddr;
		if (ua->sun_path[0]) {

		/*
		 * The AF_UNIX socket has a bound address (file path).
		 *
		 * Save it.  If there is a low nodeid, put that in
		 * parentheses after the name.  If there is a low peer
		 * nodeid, put that in the parentheses, too.
		 */
		    s->pst_boundaddr[PS_ADDR_SZ - 1] = '\0';
		    if (s->pst_lo_nodeid) {
			(void) snpf(buf, sizeof(buf), "(%s%s%s)",
			    print_kptr((KA_T)s->pst_lo_nodeid,
				       tbuf, sizeof(tbuf)),
			    s->pst_peer_lo_nodeid ? "->" : "",
			    s->pst_peer_lo_nodeid ?
			    		print_kptr((KA_T)s->pst_peer_lo_nodeid,
						   (char *)NULL, 0)
					: ""
			);
			len = strlen(buf) + 1;
			if (!(nma = (char *)malloc((MALLOC_S)len))) {
			    (void) fprintf(stderr,
				"%s: no unix nma space(1): PID %ld, FD %s",
				Pn, (long)Lp->pid, Lf->fd);
			}
			(void) snpf(nma, len, "%s", buf);
			Lf->nma = nma;
		    }
		/*
		 * Read the pst_filedetails for the bound address and process
		 * them as for a regular file.  The already-entered file type,
		 * file name, size or offset, and name appendix will be
		 * preserved.
		 */
		    if ((nau = read_det(&f->psf_fid, f->psf_hi_fileid,
					f->psf_lo_fileid, f->psf_hi_nodeid,
					f->psf_lo_nodeid, &pd)))
		    {
			enter_nm(ua->sun_path);
			(void) process_finfo(&pd, &f->psf_fid, &f->psf_id, nau);
			return;
		    } else {

		    /*
		     * Couldn't read file details.  Erase any name appendix.
		     * Put the socket nodeid in the DEVICE column, put the
		     * bound address (path) in the NAME column, and build
		     * a new name appendix with the peer address.  Add an
		     * error message if pstat_getfiledetails() set errno to
		     * something other than ENOENT.
		     */
			if ((err = errno) == ENOENT)
			    err = 0;
			if (nma) {
			    (void) free((MALLOC_P *)nma);
			    Lf->nma = (char *)NULL;
			}
			if (s->pst_lo_nodeid) {
	    		    enter_dev_ch(print_kptr((KA_T)s->pst_lo_nodeid,
					 (char *)NULL, 0));
			}
			(void) snpf(Namech, Namechl, "%s", ua->sun_path);
			if (err || s->pst_peer_lo_nodeid) {
			    (void) snpf(buf, sizeof(buf),
				"%s%s%s%s%s%s%s",
				err ? "(Error: " : "",
				err ? strerror(err) : "",
				err ? ")" : "",
				(err && s->pst_peer_lo_nodeid) ? " " : "",
				s->pst_peer_lo_nodeid ? "(->" : "",
				s->pst_peer_lo_nodeid ?
					print_kptr((KA_T)s->pst_peer_lo_nodeid,
						   (char *)NULL, 0)
				:	"",
				s->pst_peer_lo_nodeid ? ")" : ""
			    );
			    len = strlen(buf) + 1;
			    if (!(nma = (char *)malloc((MALLOC_S)len))) {
				(void) fprintf(stderr,
				    "%s: no unix nma space(2): PID %ld, FD %s",
				    Pn, (long)Lp->pid, Lf->fd);
			    }
			    (void) snpf(nma, len, "%s", buf);
			    Lf->nma = nma;
			}
			if (Sfile && is_file_named(ua->sun_path, 0))
			    Lf->sf |= SELNM;
			break;
		    }
		}
	    }
	/*
	 * If the UNIX socket has no bound address (file path), display the
	 * low nodeid in the DEVICE column and the peer's low nodeid in the
	 * NAME column.
	 */
	    if (s->pst_peer_lo_nodeid) {
		(void) snpf(Namech, Namechl, "->%s",
		    print_kptr((KA_T)s->pst_peer_lo_nodeid, (char *)NULL, 0));
	    }
	    if (s->pst_lo_nodeid)
		enter_dev_ch(print_kptr((KA_T)s->pst_lo_nodeid,(char *)NULL,0));
	    break;
	default:
	    (void) snpf(Namech, Namechl, "unsupported family: AF_%d",
		s->pst_family);
	}
	if (Namech[0])
	    enter_nm(Namech);
}


/*
 * process_stream() -- process stream
 */

void
process_stream(f, ckscko)
	struct pst_fileinfo2 *f;		/* pst_fileinfo2 */
	int ckscko;				/* socket file only checking
						 * if 1 */
{
	struct clone *cl;
	char *cp;
	struct l_dev *dp = (struct l_dev *)NULL;
	int hx, i, ncx, nsn, nsr;
	size_t nb, nl;
	KA_T na;
	static int nsa = 0;
	dev_t rdev;
	static struct pst_stream *s = (struct pst_stream *)NULL;
	struct pst_socket sck;
	static size_t sz = sizeof(struct pst_stream);

#if	!defined(PS_STR_XPORT_DATA)
/*
 * If socket file only checking is enabled and this HP-UX PSTAT instance
 * doesn't support TCP or UDP stream eXPORT data, return without further
 * action.
 */
	if (ckscko == 1)
		return;
#endif	/* !defined(PS_STR_XPORT_DATA) */

/*
 * Generate and save node ID.
 */
	na = (KA_T)(((KA_T)(f->psf_hi_nodeid & 0xffffffff) << 32)
	   |		(KA_T)(f->psf_lo_nodeid & 0xffffffff));

#if	defined(HASFSTRUCT)
	if (na && (Fsv & FSV_NI)) {
	    Lf->fna = na;
	    Lf->fsv |= FSV_NI;
	}
#endif	/* defined(HASFSTRUCT) */

/*
 * Enter type.
 */
	switch (f->psf_ftype) {
	case PS_TYPE_STREAMS:
	    cp = "STR";
	    break;
	case PS_TYPE_SOCKET:
	    if (f->psf_subtype == PS_SUBTYPE_SOCKSTR) {
		cp = "STSO";
		break;
	    }
	    /* fall through */
	default:
	    cp = "unkn";
	}
	(void) snpf(Lf->type, sizeof(Lf->type), "%s", cp);
/*
 * Allocate sufficient space for stream structures, then read them.
 */
	if ((nsn = f->psf_nstrentt) && (nsn >= nsa)) {
	    nb = (size_t)(nsn * sizeof(struct pst_stream));
	    if (s)
		s = (struct pst_stream *)realloc((MALLOC_P *)s, nb);
	    else
		s = (struct pst_stream *)malloc(nb);
	    if (!s) {
		(void) fprintf(stderr,
		    "%s: no space for %ld pst_stream bytes\n", Pn, (long)nb);
		Exit(1);
	    }
	    nsa = nsn;
	}
	errno = 0;
	if ((nsr = pstat_getstream(s, sz, (size_t)nsn, 0, &f->psf_fid)) < 1) {
	    if (nsn) {
		(void) snpf(Namech, Namechl,
		    "can't read %d stream structures%s%s", nsn,
		    errno ? ": " : "", errno ? strerror(errno) : "");
		enter_nm(Namech);
	    } else
		enter_nm("no stream structures present");
	    return;
	}
/*
 * Find the stream head.
 */
	for (hx = 0; hx < nsn; hx++) {
	    if (s[hx].type == PS_STR_HEAD)
		break;
	}
	if (hx >= nsn) { 
	    enter_nm("no stream head located");
	    return;
	}
/*
 * Make sure the stream head's fileid and nodeid match the ones in the
 * pst_fileino2 structure.
 */
	if ((f->psf_hi_fileid != s[hx].val.head.pst_hi_fileid)
	 |  (f->psf_lo_fileid != s[hx].val.head.pst_lo_fileid)
	 |  (f->psf_hi_nodeid != s[hx].val.head.pst_hi_nodeid)
	 |  (f->psf_lo_nodeid != s[hx].val.head.pst_lo_nodeid)) {
	    enter_nm("no matching stream data available");
	    return;
	}

#if	defined(PS_STR_XPORT_DATA)
/*
 * See if this stream has eXPORT data available and is a TCP or
 * UDP stream.
 */
	if ((s[hx].pst_extn_flags & PS_STR_XPORT_DATA)
	&&  ((s[hx].pst_str_xport_protocol == PS_STR_XPORT_TCP_PROTO)
	||   (s[hx].pst_str_xport_protocol == PS_STR_XPORT_UDP_PROTO))
	) {

	/*
	 * Make a socket from the eXPORT data and process it.
	 */
	    (void) make_sock(f, &s[hx], &sck);
	    (void) process_socket(f, &sck);
	    return;
	} else if (ckscko || Selinet) {

	/*
	 * If socket file or Internet file only processing is enabled, return.
	 */
	    return;
	}
#endif	/* defined(PS_STR_XPORT_DATA) */

/*
 * Enter size from stream head's structure, if requested.
 */
	if (Fsize) {
	    if (Lf->access == 'r') {
		Lf->sz = (SZOFFTYPE)s[hx].val.head.pst_rbytes;
		Lf->sz_def = 1;
	    } else if (Lf->access == 'w') {
		Lf->sz = (SZOFFTYPE)s[hx].val.head.pst_wbytes;
		Lf->sz_def = 1;
	    } else if (Lf->access == 'u') {
		Lf->sz = (SZOFFTYPE)s[hx].val.head.pst_rbytes
		       + (SZOFFTYPE)s[hx].val.head.pst_wbytes;
		Lf->sz_def = 1;
	    }
	}
/*
 * Get the the device number from the stream head.
 *
 * If the stream is a clone:
 *
 *	if there's a clone list, search it for the device, based on the stream
 *	    head's minor device number only;
 *	if there's no clone list, search Devtp[], using a device number made
 *	    from the stream head's major and minor device numbers;
 *	set the printable clone device number to one whose major device number
 *	    is the stream head's minor device number, and whose minor device
 *	    number is the stream head's device sequence number.
 *
 * If the stream isn't a clone, make the device number from the stream head's
 * major and minor numbers, and look up the non-clone device number in Devtp[].
 */
	if (!Sdev)
	    readdev(0);
	if (s[hx].val.head.pst_flag & PS_STR_ISACLONE) {
	    if (HaveCloneMaj && (CloneMaj == s[hx].val.head.pst_dev_major)) {
		for (cl = Clone; cl; cl = cl->next) {
		    if (GET_MIN_DEV(Devtp[cl->dx].rdev)
		    ==  s[hx].val.head.pst_dev_minor)
		    {
			dp = &Devtp[cl->dx];
			break;
		    }
		}
	    } else {
		rdev = makedev(s[hx].val.head.pst_dev_major,
			       s[hx].val.head.pst_dev_minor);
		dp = lkupdev(&DevDev, &rdev, 0, 1);
	    }
	    rdev = makedev(s[hx].val.head.pst_dev_minor,
			   s[hx].val.head.pst_dev_seq);
	} else {
	    rdev = makedev(s[hx].val.head.pst_dev_major,
			   s[hx].val.head.pst_dev_minor);
	    dp = lkupdev(&DevDev, &rdev, 0, 1);
	}
	Lf->dev = DevDev;
	Lf->rdev = rdev;
	Lf->dev_def = Lf->rdev_def = 1;
/*
 * If the device was located, enter the device name and save the node number.
 *
 * If the device wasn't located, save a positive file ID number from the
 * pst_fileinfo as a node number.
 */
	if (dp) {
	    (void) snpf(Namech, Namechl, "%s", dp->name);
	    ncx = strlen(Namech);
	    Lf->inode = (INODETYPE)dp->inode;
	    Lf->inp_ty = 1;
	} else {
	    ncx = (size_t)0;
	    if (f->psf_id.psf_fileid > 0) {
		Lf->inode = (INODETYPE)f->psf_id.psf_fileid;
		Lf->inp_ty = 1;
	    }
	}
/*
 * Enter stream module names.
 */
	for (i = 1; i < nsr; i++) {
	    if (!(nl = strlen(s[i].val.module.pst_name)))
		continue;
	    if (ncx) {
		if ((ncx + 2) > (Namechl - 1))
		    break;
		(void) snpf(&Namech[ncx], Namechl - ncx, "->");
		ncx += 2;
	    }
	    if ((ncx + nl) > (Namechl - 1))
		break;
	    (void) snpf(Namech+ncx,Namechl-ncx,"%s",s[i].val.module.pst_name);
	    ncx += nl;
	}
/*
 * Set node type.
 *
 * Set offset defined if file size not requested or if no size was
 * obtained from the stream head.
 */
	Lf->ntype = N_STREAM;
	Lf->is_stream = 1;
	if (!Fsize || (Fsize && !Lf->sz_def))
	    Lf->off_def = 1;
/*
 * Test for specified file.
 */
	if ((f->psf_subtype == PS_SUBTYPE_CHARDEV)
	||  (f->psf_subtype == PS_SUBTYPE_BLKDEV))
	    i = 1;
	else
	    i = 0;
	if (Sfile && is_file_named((char *)NULL, i))
	    Lf->sf |= SELNM;
/*
 * Enter any name characters.
 */
	if (Namech[0])
	    enter_nm(Namech);
}


/*
 * read_sock() -- read pst_socket info for file
 */

struct pst_socket *
read_sock(f)
	struct pst_fileinfo2 *f;		/* file information */
{
	static struct pst_socket s;

	errno = 0;
	if (f) {
	    if (pstat_getsocket(&s, sizeof(s), &f->psf_fid) > 0
	    &&  f->psf_hi_fileid == s.pst_hi_fileid
	    &&  f->psf_lo_fileid == s.pst_lo_fileid
	    &&  f->psf_hi_nodeid == s.pst_hi_nodeid
	    &&  f->psf_lo_nodeid == s.pst_lo_nodeid)
		return(&s);
	}
	return((struct pst_socket *)NULL);
}
