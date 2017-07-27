/*
 * dsock.c - /dev/kmem-based HP-UX socket processing functions for lsof
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
static char *rcsid = "$Id: dsock.c,v 1.20 2005/08/08 19:50:23 abe Exp $";
#endif

#if	defined(HPUXKERNBITS) && HPUXKERNBITS>=64
#define _INO_T
typedef int ino_t;
#define _TIME_T
typedef int time_t;
#endif	/* defined(HPUXKERNBITS) && HPUXKERNBITS>=64 */

#include "lsof.h"

#if     HPUXV>=800 && defined(HPUX_CCITT)
#include <x25/x25addrstr.h>
#include <x25/x25stat.h>
#include <x25/x25str.h>
#include <x25/x25config.h>
#include <x25/x25L3.h>
#endif	/* HPUXV>=800 && defined(HPUX_CCITT) */


/*
 * Local definitions
 */

#if	defined(HASTCPOPT)
#define	TF_NODELAY	0x1		/* TCP_NODELAY (Nagle algorithm) */
#endif	/* defined(HASTCPOPT) */


#if	HPUXV>=1030
/*
 * print_tcptpi() - print TCP/TPI info
 */

void
print_tcptpi(nl)
	int nl;				/* 1 == '\n' required */
{
	char *cp = (char *)NULL;
	char  sbuf[128];
	int i, t;
	int ps = 0;
	unsigned int u;

	if (Ftcptpi & TCPTPI_STATE) {
	    switch ((t = Lf->lts.type)) {
	    case 0:				/* TCP */
		switch ((i = Lf->lts.state.i)) {
		case TCPS_CLOSED:
		    cp = "CLOSED";
		    break;
		case TCPS_IDLE:
		    cp = "IDLE";
		    break;
		case TCPS_BOUND:
		    cp = "BOUND";
		    break;
		case TCPS_LISTEN:
		    cp = "LISTEN";
		    break;
		case TCPS_SYN_SENT:
		    cp = "SYN_SENT";
		    break;
		case TCPS_SYN_RCVD:
		    cp = "SYN_RCVD";
		    break;
		case TCPS_ESTABLISHED:
		    cp = "ESTABLISHED";
		    break;
		case TCPS_CLOSE_WAIT:
		    cp = "CLOSE_WAIT";
		    break;
		case TCPS_FIN_WAIT_1:
		    cp = "FIN_WAIT_1";
		    break;
		case TCPS_CLOSING:
		    cp = "CLOSING";
		    break;
		case TCPS_LAST_ACK:
		    cp = "LAST_ACK";
		    break;
		case TCPS_FIN_WAIT_2:
		    cp = "FIN_WAIT_2";
		    break;
		case TCPS_TIME_WAIT:
		    cp = "TIME_WAIT";
		    break;
		default:
		    (void) snpf(sbuf, sizeof(sbuf), "UknownState_%d", i);
		    cp = sbuf;
		}
		break;
	    case 1:				/* TPI */
		switch ((u = Lf->lts.state.ui)) {
		case TS_UNINIT:
		    cp = "Uninitialized";
		    break;
		case TS_UNBND:
		    cp = "Unbound";
		    break;
		case TS_WACK_BREQ:
		    cp = "Wait_BIND_REQ_Ack";
		    break;
		case TS_WACK_UREQ:
		    cp = "Wait_UNBIND_REQ_Ack";
		    break;
		case TS_IDLE:
		    cp = "Idle";
		    break;
		case TS_WACK_OPTREQ:
		    cp = "Wait_OPT_REQ_Ack";
		    break;
		case TS_WACK_CREQ:
		    cp = "Wait_CONN_REQ_Ack";
		    break;
		case TS_WCON_CREQ:
		    cp = "Wait_CONN_REQ_Confirm";
		    break;
		case TS_WRES_CIND:
		    cp = "Wait_CONN_IND_Response";
		    break;
		case TS_WACK_CRES:
		    cp = "Wait_CONN_RES_Ack";
		    break;
		case TS_DATA_XFER:
		    cp = "Wait_Data_Xfr";
		    break;
		case TS_WIND_ORDREL:
		    cp = "Wait_Read_Release";
		    break;
		case TS_WREQ_ORDREL:
		    cp = "Wait_Write_Release";
		    break;
		case TS_WACK_DREQ6:
		case TS_WACK_DREQ7:
		case TS_WACK_DREQ9:
		case TS_WACK_DREQ10:
		case TS_WACK_DREQ11:
		    cp = "Wait_DISCON_REQ_Ack";
		    break;
		case TS_WACK_ORDREL:
		    cp = "Internal";
		    break;
		default:
		    (void) snpf(sbuf, sizeof(sbuf), "UNKNOWN_TPI_STATE_%u", u);
		    cp = sbuf;
		}
	    }
	    if (Ffield)
		(void) printf("%cST=%s%c", LSOF_FID_TCPTPI, cp, Terminator);
	    else {
		putchar('(');
		(void) fputs(cp, stdout);
	    }
	    ps++;
	}

# if	defined(HASTCPTPIQ)
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
# endif	/* defined(HASTCPTPIQ) */

#if	defined(HASSOOPT)
	if (Ftcptpi & TCPTPI_FLAGS) {
	    int opt;

	    if ((opt = Lf->lts.opt)
	    ||  Lf->lts.qlens || Lf->lts.qlims || Lf->lts.rbszs || Lf->lts.sbsz
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

		if (Lf->lts.qlens) {
		    (void) printf("%cQLEN=%u", sep, Lf->lts.qlen);
		    sep = ',';
		}
		if (Lf->lts.qlims) {
		    (void) printf("%cQLIM=%u", sep, Lf->lts.qlim);
		    sep = ',';
		}

# if	defined(SO_REUSEADDR)
		if (opt & SO_REUSEADDR) {
		    (void) printf("%cREUSEADDR", sep);
		    opt &= ~SO_REUSEADDR;
		    sep = ',';
		}
# endif	/* defined(SO_REUSEADDR) */

# if	defined(SO_REUSEPORT)
		if (opt & SO_REUSEPORT) {
		    (void) printf("%cREUSEPORT", sep);
		    opt &= ~SO_REUSEPORT;
		    sep = ',';
		}
# endif	/* defined(SO_REUSEPORT) */

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

		if (Lf->lts.msss) {
		    (void) printf("%cMSS=%lu", sep, Lf->lts.mss);
		    sep = ',';
		}

#  if	defined(TF_NODELAY)
		if (topt & TF_NODELAY) {
		    (void) printf("%cNODELAY", sep);
		    topt &= ~TF_NODELAY;
		    sep = ',';
		}
#  endif	/* defined(TF_NODELAY) */

		if (topt)
		    (void) printf("%cUNKNOWN=%#x", sep, topt);
		if (Ffield)
		    putchar(Terminator);
	    }
	}
# endif	/* defined(HASTCPOPT) */

# if	defined(HASTCPTPIW)
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
# endif	/* defined(HASTCPTPIW) */

	if (Ftcptpi && !Ffield && ps)
	    putchar(')');
	if (nl)
	    putchar('\n');
}
#endif	/* HPUXV>=1030 */


#if	defined(DTYPE_LLA)
/*
 * process_lla() - process link level access socket file
 */

void
process_lla(la)
	KA_T la;			/* link level CB address in kernel */
{
	char *ep;
	struct lla_cb lcb;
	size_t sz;

	(void) snpf(Lf->type, sizeof(Lf->type), "lla");
	Lf->inp_ty = 2;
	enter_dev_ch(print_kptr(la, (char *)NULL, 0));
/*
 * Read link level access control block.
 */
	if (!la || kread((KA_T)la, (char *)&lcb, sizeof(lcb))) {
	    (void) snpf(Namech, Namechl, "can't read LLA CB (%s)",
		print_kptr(la, (char *)NULL, 0));
	    enter_nm(Namech);
	    return;
	}
/*
 * Determine access mode.
 */
	if ((lcb.lla_flags & LLA_FWRITE | LLA_FREAD) == LLA_FWRITE)
	    Lf->access = 'w';
	else if ((lcb.lla_flags & LLA_FWRITE | LLA_FREAD) == LLA_FREAD)
	    Lf->access = 'r';
	else if (lcb.lla_flags & LLA_FWRITE | LLA_FREAD)
	    Lf->access = 'u';
/*
 * Determine the open mode, if possible.
 */
	if (lcb.lla_flags & LLA_IS_ETHER)
	    (void) snpf(Lf->iproto, sizeof(Lf->iproto), "Ether");
	else if (lcb.lla_flags & (LLA_IS_8025|LLA_IS_SNAP8025|LLA_IS_FA8025)) {
	    (void) snpf(Lf->iproto, sizeof(Lf->iproto), "802.5");
	    if (lcb.lla_flags & LLA_IS_SNAP8025)
		(void) snpf(Namech, Namechl, "SNAP");
	    else if (lcb.lla_flags & LLA_IS_FA8025)
		(void) snpf(Namech, Namechl, "function address");
	}
/*
 * Add any significant flags.
 */
	if (lcb.lla_flags & ~(LLA_FWRITE | LLA_FREAD)) {
	    ep = endnm(&sz);
	    (void) snpf(ep, sz, "%s(flags = %#x)",
		(ep == Namech) ? "" : " ",
		lcb.lla_flags);
	}
	if (Namech[0])
	    enter_nm(Namech);
}
#endif	/* DTYPE_LLA */


/*
 * process_socket() - process socket
 */

void
process_socket(sa)
	KA_T sa;			/* socket address in kernel */
{
	unsigned char *fa = (unsigned char *)NULL;
	char *ep, tbuf[32];
	int fam;
	int fp, mbl, lp;
	unsigned char *la = (unsigned char *)NULL;
	struct protosw p;
	struct socket s;
	size_t sz;
	struct unpcb uc, unp;
	struct sockaddr_un *ua = (struct sockaddr_un *)NULL;
	struct sockaddr_un un;

#if	HPUXV>=800
	struct domain d;

# if	defined(HPUX_CCITT)
	int i;
	struct x25pcb xp;
	struct x25pcb_extension xpe;
# endif	/* defined(HPUX_CCITT) */

# if	HPUXV<1030
	struct mbuf mb;
	struct inpcb inp;
	struct rawcb raw;
	struct tcpcb t;
# else	/* HPUXV>=1030 */
	struct datab db;
	static char *dbf = (char *)NULL;
	static int dbl = 0;
	struct msgb mb;
	struct sockbuf rb, sb;
# endif	/* HPUXV<1030 */
#endif	/* HPUXV>=800 */

	(void) snpf(Lf->type, sizeof(Lf->type), "sock");
	Lf->inp_ty = 2;
/*
 * Read socket structure.
 */
	if (!sa) {
	    enter_nm("no socket address");
	    return;
	}
	if (kread((KA_T) sa, (char *) &s, sizeof(s))) {
	    (void) snpf(Namech, Namechl, "can't read socket struct from %s",
		print_kptr(sa, (char *)NULL, 0));
	    enter_nm(Namech);
	    return;
	}
/*
 * Read protocol switch and domain structure (HP-UX 8 and above).
 */
	if (!s.so_type) {
	    (void) snpf(Namech, Namechl, "no socket type");
	    enter_nm(Namech);
	    return;
	}
	if (!s.so_proto
	||  kread((KA_T) s.so_proto, (char *) &p, sizeof(p))) {
	    (void) snpf(Namech, Namechl, "no protocol switch");
	    enter_nm(Namech);
	    return;
	}

#if	HPUXV>=800
	if (kread((KA_T) p.pr_domain, (char *) &d, sizeof(d))) {
	    (void) snpf(Namech, Namechl, "can't read domain struct from %s",
		print_kptr((KA_T)p.pr_domain, (char *)NULL, 0));
	    enter_nm(Namech);
	    return;
	}
#endif	/* HPUXV>=800 */

#if	HPUXV<1030
/*
 * Save size information for HP-UX < 10.30.
 */
 	if (Fsize) {
	    if (Lf->access == 'r')
		Lf->sz = (SZOFFTYPE)s.so_rcv.sb_cc;
	    else if (Lf->access == 'w')
		Lf->sz = (SZOFFTYPE)s.so_snd.sb_cc;
	    else
		Lf->sz = (SZOFFTYPE)(s.so_rcv.sb_cc + s.so_snd.sb_cc);
	    Lf->sz_def = 1;
	} else
	    Lf->off_def = 1;

# if	defined(HASTCPTPIQ)
	Lf->lts.rq = s.so_rcv.sb_cc;
	Lf->lts.sq = s.so_snd.sb_cc;
	Lf->lts.rqs = Lf->lts.sqs = 1;
# endif	/* defined(HASTCPTPIQ) */
#endif	/* HPUXV<1030 */

/*
 * Process socket by the associated domain family.
 */

#if	HPUXV>=800
	switch ((fam = d.dom_family))
#else	/* HPUXV<800 */
	switch ((fam = p.pr_family))
#endif	/* HPUXV>=800 */

	{

#if	HPUXV>=800 && HPUXV<1030 && defined(HPUX_CCITT)
/*
 * Process an HP-UX [89].x CCITT X25 domain socket.
 */
	case AF_CCITT:
	    if (Fnet)
		Lf->sf |= SELNET;
	    (void) snpf(Lf->type, sizeof(Lf->type), "x.25");
	    (void) snpf(Lf->iproto, sizeof(Lf->iproto), "%.*s", IPROTOL,
		"CCITT");
	/*
	 * Get the X25 PCB and its extension.
	 */
	    if (!s.so_pcb
	    ||  kread((KA_T)s.so_pcb, (char *)&xp, sizeof(xp))) {
		(void) snpf(Namech, Namechl, "can't read x.25 pcb at %s",
		    print_kptr((KA_T)s.so_pcb, (char *)NULL, 0));
		enter_nm(Namech);
		return;
	    }
	    enter_dev_ch(print_kptr((KA_T)s.so_pcb, (char *)NULL, 0));
	    if (!xp.x25pcb_extend
	    ||  kread((KA_T)xp.x25pcb_extend, (char *)&xpe, sizeof(xpe))) {
		(void) snpf(Namech, Namechl,
		    "can't read x.25 pcb (%s) extension at %s",
		    print_kptr((KA_T)s.so_pcb, tbuf, sizeof(tbuf)),
		    print_kptr((KA_T)xp.x25pcb_extend, (char *)NULL, 0));
		enter_nm(Namech);
		return;
	    }
	/*
	 * Format local address.
	 */
	    for (i = 0; i < xpe.x25pcbx_local_addr.x25hostlen/2; i++) {
		ep = endnm(&sz);
		(void) snpf(ep, sz, "%02x", xpe.x25pcbx_local_addr.x25_host[i]);
	    }
	    if (i*2 != xpe.x25pcbx_local_addr.x25hostlen) {
		ep = endnm(&sz);
		(void) snpf(ep, sz, "%01x",
		    xpe.x25pcbx_local_addr.x25_host[i] >> 4);
	    }
	/*
	 * Display the virtual connection number, if it's defined.
	 */
	    if (xp.x25pcb_vcn >= 0) {
		ep = endnm(&sz);
		(void) snpf(ep, sz, ":%d", xp.x25pcb_vcn + 1);
	    }
	/*
	 * Format peer address, if there is one.
	 */
	    if (xpe.x25pcbx_peer_addr.x25hostlen > 0) {
		ep = endnm(&sz);
		(void) snpf(ep, sz, "->");
		for (i = 0; i < xpe.x25pcbx_peer_addr.x25hostlen/2; i++) {
		    ep = endnm(&sz);
		    (void) snpf(ep, sz, "%02x",
			xpe.x25pcbx_peer_addr.x25_host[i]);
		}
		if (i*2 != xpe.x25pcbx_peer_addr.x25hostlen) {
		    ep = endnm(&sz);
		    (void) snpf(ep, sz, "%01x",
			xpe.x25pcbx_peer_addr.x25_host[i] >> 4);
		}
	    }
	    enter_nm(Namech);
	    break;
#endif	/* HPUXV>=800 && HPUXV<1030 && defined(HPUX_CCITT) */

/*
 * Process an Internet domain socket.
 */
	case AF_INET:
	    if (Fnet)
		Lf->sf |= SELNET;
	    (void) snpf(Lf->type, sizeof(Lf->type), "inet");
	    printiproto(p.pr_protocol);

#if	HPUXV>=1030
	/*
	 * Handle HP-UX 10.30 and above socket streams.
	 */
	    if (s.so_sth) {

		KA_T ip, pcb;
		char *pn = (char *)NULL;
	    /*
	     * Read module information.
	     */
		if (read_mi((KA_T)s.so_sth, &ip, &pcb, &pn))
		    return;
		if (ip && pcb) {

		/*
		 * If IP and TCP or UDP modules are present, process as a
		 * stream socket.
		 */
		    process_stream_sock(ip, pcb, pn, VNON);
		    return;
		}
	    /*
	     * If an IP module's PCB address is present, print it as the
	     * device characters.
	     */

		if (ip && !Lf->dev_def)
		    enter_dev_ch(print_kptr(ip, (char *)NULL, 0));
		if (!strlen(Namech)) {

		/*
		 * If there are no NAME field characters, enter an error
		 * message.
		 */
		    if (!ip) {
			(void) snpf(Namech, Namechl,
			    "no IP module for stream socket");
		    } else {
			(void) snpf(Namech, Namechl,
			    "no TCP/UDP module for stream socket");
		    }
		}
		enter_nm(Namech);
		return;
	    }
#else	/* HPUXV<1030 */

	/*
	 * Read protocol control block.
	 */
	    if (!s.so_pcb) {
		enter_nm("no protocol control block");
		return;
	    }
	    if (s.so_type == SOCK_RAW) {

	    /*
	     * Print raw socket information.
	     */
		if (kread((KA_T)s.so_pcb, (char *)&raw, sizeof(raw))
		||  (struct socket *)sa != (struct socket *)raw.rcb_socket) {
		    (void) snpf(Namech, Namechl, "can't read rawcb at %s",
			print_kptr((KA_T)s.so_pcb, (char *)NULL, 0));
		    enter_nm(Namech);
		    return;
		}
		enter_dev_ch(print_kptr((KA_T)(raw.rcb_pcb ? raw.rcb_pcb
							   : s.so_pcb),
					(char *)NULL, 0));
		if (raw.rcb_laddr.sa_family == AF_INET)
		    la = (unsigned char *)&raw.rcb_laddr.sa_data[2];
		else if (raw.rcb_laddr.sa_family)
		    printrawaddr(&raw.rcb_laddr);
		if (raw.rcb_faddr.sa_family == AF_INET)
		    fa = (unsigned char *)&raw.rcb_faddr.sa_data[2];
		else if (raw.rcb_faddr.sa_family) {
		    ep = endnm(&sz);
		    (void) snpf(ep, sz, "->");
		    printrawaddr(&raw.rcb_faddr);
		}
		if (fa || la)
		    (void) ent_inaddr(la, -1, fa, -1, AF_INET);
	    } else {

	    /*
	     * Print Internet socket information.
	     */
		if (kread((KA_T)s.so_pcb, (char *)&inp, sizeof(inp))) {
		    (void) snpf(Namech, Namechl, "can't read inpcb at %s",
			print_kptr((KA_T)s.so_pcb, (char *)NULL, 0));
		    enter_nm(Namech);
		    return;
		}
		enter_dev_ch(print_kptr((KA_T)(inp.inp_ppcb ? inp.inp_ppcb
							    : s.so_pcb),
					(char *)NULL, 0));
		la = (unsigned char *)&inp.inp_laddr;
		lp = (int)ntohs(inp.inp_lport);
		if (inp.inp_faddr.s_addr != INADDR_ANY || inp.inp_fport != 0) {
		    fa = (unsigned char *)&inp.inp_faddr;
		    fp = (int)ntohs(inp.inp_fport);
		}
		if (fa || la)
		    (void) ent_inaddr(la, lp, fa, fp, AF_INET);
		if (p.pr_protocol == IPPROTO_TCP && inp.inp_ppcb
		&&  kread((KA_T)inp.inp_ppcb, (char *)&t, sizeof(t)) == 0) {
		    Lf->lts.type = 0;
		    Lf->lts.state.i = (int)t.t_state;
		}
	    }
	    break;
#endif	/* HPUXV>=1030 */

/*
 * Process a Unix domain socket.
 */
	case AF_UNIX:
	    if (Funix)
		Lf->sf |= SELUNX;
	    (void) snpf(Lf->type, sizeof(Lf->type), "unix");

#if	HPUXV>=1030
/*
 * Save size information for HP-UX 10.30 and above.
 */
 	    if (Fsize) {
		if (!s.so_rcv
		||  kread((KA_T)s.so_rcv, (char *)&rb, sizeof(rb)))
		    rb.sb_cc = 0;
		if (!s.so_snd
		||  kread((KA_T)s.so_snd, (char *)&sb, sizeof(sb)))
		    sb.sb_cc = 0;
		if (Lf->access == 'r')
		    Lf->sz = (SZOFFTYPE)rb.sb_cc;
		else if (Lf->access == 'w')
		    Lf->sz = (SZOFFTYPE)sb.sb_cc;
		else
		    Lf->sz = (SZOFFTYPE)(rb.sb_cc + sb.sb_cc);
		Lf->sz_def = 1;
	    } else
		Lf->off_def = 1;
#endif	/* HPUXV>=1030 */

	/*
	 * Read Unix protocol control block and the Unix address structure.
	 */
	    enter_dev_ch(print_kptr(sa, (char *)NULL, 0));
	    if (kread((KA_T) s.so_pcb, (char *) &unp, sizeof(unp))) {
		(void) snpf(Namech, Namechl, "can't read unpcb at %s",
		    print_kptr((KA_T)s.so_pcb, (char *)NULL, 0));
		break;
	    }
	    if ((struct socket *)sa != unp.unp_socket) {
		(void) snpf(Namech, Namechl, "unp_socket (%s) mismatch",
		    print_kptr((KA_T)unp.unp_socket, (char *)NULL, 0));
		break;
	    }

#if	HPUXV<1030
	/*
	 * Read UNIX domain socket address information for HP-UX below 10.30.
	 */
	    if (unp.unp_addr) {
		if (kread((KA_T) unp.unp_addr, (char *) &mb, sizeof(mb))) {
		    (void) snpf(Namech, Namechl, "can't read unp_addr at %s",
			print_kptr((KA_T)unp.unp_addr, (char *)NULL, 0));
		    break;
		}
		ua = (struct sockaddr_un *)(((char *)&mb) + mb.m_off);
		mbl = mb.m_len;
	    }
#else	/* HPUXV>=1030 */
	/*
	 * Obtain UNIX domain socket address information for HP-UX 10.30 and
	 * above.
	 */
	    if (unp.unp_ino) {
		Lf->inode = (INODETYPE)unp.unp_ino;
		Lf->inp_ty = 1;
	    }
	    ua = (struct sockaddr_un *)NULL;
	    mbl = 0;
	    if (unp.unp_addr
	    &&  kread((KA_T)unp.unp_addr, (char *)&mb, sizeof(mb)) == 0
	    &&  mb.b_datap
	    &&  kread((KA_T)mb.b_datap, (char *)&db, sizeof(db)) == 0) {
		if (db.db_base) {
		    if (dbl < (db.db_size + 1)) {
			dbl = db.db_size + 1;
			if (dbf)
			    dbf = (char *)realloc((MALLOC_P *)dbf,
						  (MALLOC_S) dbl);
			else
			    dbf = (char *)malloc((MALLOC_S)dbl);
			if (!dbf) {
			    (void) fprintf(stderr,
				"%s: no space (%d) for UNIX socket address\n",
				Pn, dbl);
			    Exit(1);
			}
		    }
		    if (kread((KA_T)db.db_base, dbf, db.db_size) == 0) {
			mbl = db.db_size;
			dbf[mbl] = '\0';
			ua = (struct sockaddr_un *)dbf;
		    }
		}
	    }
#endif	/* HPUXV>=1030 */

	    if (!ua) {
		ua = &un;
		(void) bzero((char *)ua, sizeof(un));
		ua->sun_family = AF_UNSPEC;
	    }
	/*
	 * Print information on Unix socket that has no address bound
	 * to it, although it may be connected to another Unix domain
	 * socket as a pipe.
	 */
	    if (ua->sun_family != AF_UNIX) {
		if (ua->sun_family == AF_UNSPEC) {
		    if (unp.unp_conn) {
			if (kread((KA_T)unp.unp_conn, (char *) &uc, sizeof(uc)))
			    (void) snpf(Namech, Namechl,
				"can't read unp_conn at %s",
				print_kptr((KA_T)unp.unp_conn,(char *)NULL,0));
			else
			    (void) snpf(Namech, Namechl, "->%s",
				print_kptr((KA_T)uc.unp_socket,(char *)NULL,0));
		    } else
			(void) snpf(Namech, Namechl, "->(none)");
		} else
		    (void) snpf(Namech, Namechl, "unknown sun_family (%d)",
			ua->sun_family);
		break;
	    }
	    if (ua->sun_path[0]) {
		if (mbl >= sizeof(struct sockaddr_un))
		    mbl = sizeof(struct sockaddr_un) - 1;
		*((char *)ua + mbl) = '\0';
		if (Sfile && is_file_named(ua->sun_path, 0))
		    Lf->sf |= SELNM;
		if (!Namech[0])
		    (void) snpf(Namech, Namechl, "%s", ua->sun_path);
	    } else
		(void) snpf(Namech, Namechl, "no address");
		break;
	default:
	    printunkaf(fam, 1);
	}
	if (Namech[0])
	    enter_nm(Namech);
}


#if	HPUXV>=1030
/*
 * process_stream_sock() - process stream socket
 */

void
process_stream_sock(ip, pcb, pn, vt)
	KA_T ip;			/* IP module's q_ptr */
	KA_T pcb;			/* protocol's q_ptr */
	char *pn;			/* protocol name */
	enum vtype vt;			/* vnode type */
{
	unsigned char *fa = (unsigned char *)NULL;
	char *ep;
	int fp, lp, rq, sq;
	struct ipc_s ic;
	unsigned char *la = (unsigned char *)NULL;
	size_t sz;
	u_short pt;
	struct tcp_s tc;
	tcph_t th;
	struct udp_s ud;
/*
 * Set file type and protocol.  If AF_INET selection is in effect, set its flag.
 */
	if (Fnet)
	    Lf->sf |= SELNET;
	(void) snpf(Lf->type, sizeof(Lf->type), "inet");
	if (pn) {
	    (void) snpf(Lf->iproto, sizeof(Lf->iproto), pn);
	    Lf->inp_ty = 2;
	} else if (Sfile && (vt != VNON) && Lf->dev_def && (Lf->inp_ty == 1)) {

	/*
	 * If the protocol name isn't known and this stream socket's vnode type
	 * isn't VNON, the stream socket will be handled mostly as a stream.
	 * Thus, a named file check is appropriate.
	 */
	    if (is_file_named((char *)NULL, (vt == VCHR) ? 1 : 0))
		Lf->sf |= SELNM;
	}
/*
 * Get IP structure.
 */
	*Namech = '\0';
	if (!ip || kread(ip, (char *)&ic, sizeof(ic))) {
	    ep = endnm(&sz);
	    (void) snpf(ep, sz, "%scan't read IP control structure from %s",
		sz ? " " : "", print_kptr(ip, (char *)NULL, 0));
	    enter_nm(Namech);
	    return;
	}
	if (!Lf->dev_def)
	    enter_dev_ch(print_kptr(ip, (char *)NULL, 0));
/*
 * Check for protocol control block address.  Enter if non-NULL and clear
 * device definition flag.
 */
	if (!pcb) {
	    ep = endnm(&sz);
	    (void) snpf(ep, sz, "%ssocket stream has no TCP or UDP module",
		sz ? " " : "");
	    enter_nm(Namech);
	    return;
	}
/*
 * Select processing by protocol name.
 */
	if (pn && !strcmp(pn, "TCP")) {

	/*
	 * Process TCP socket.
	 */
	    if (kread(pcb, (char *)&tc, sizeof(tc))) {
		ep = endnm(&sz);
		(void) snpf(ep, sz, "%scan't read TCP PCB from %s",
		    sz ? " " : "", print_kptr(pcb, (char *)NULL, 0));
		enter_nm(Namech);
		return;
	    }
	/*
	 * Save TCP address.
	 */
	    la = (unsigned char *)&ic.ipc_tcp_laddr;
	    pt = (u_short)ic.ipc_tcp_lport;
	    if (((struct in_addr *)la)->s_addr == INADDR_ANY && pt == 0) {

	    /*
	     * If the ipc_s structure has no local address, use the local
	     * address in its tcp_iph structure, and the port number in its
	     * tcph structure.
	     */
		la = (unsigned char *)&tc.tcp_u.tcp_u_iph.iph_src[0];
		if (tc.tcp_hdr_len && tc.tcp_tcph
		&&  kread((KA_T)tc.tcp_tcph, (char *)&th, sizeof(th))
		== 0)
		    pt = (u_short)th.th_lport;
	    }
	    lp = (int)ntohs(pt);
	    if ((int)ic.ipc_tcp_faddr != INADDR_ANY
	    || (u_short)ic.ipc_tcp_fport != 0)
	    {
		fa = (unsigned char *)&ic.ipc_tcp_faddr;
		fp = (int)ntohs((u_short)ic.ipc_tcp_fport);
	    }
	    if (fa || la)
		(void) ent_inaddr(la, lp, fa, fp, AF_INET);
	/*
	 * Save TCP state and size information.
	 */
	    Lf->lts.type = 0;
	    Lf->lts.state.i = (int)tc.tcp_state;

# if	defined(HASTCPTPIQ) || defined(HASTCPTPIW)
#  if	defined(HASTCPTPIW)
	    Lf->lts.rw = (int)tc.tcp_rwnd;
	    Lf->lts.ww = (int)tc.tcp_swnd;
	    Lf->lts.rws = Lf->lts.wws = 1;
#  endif	/* defined(HASTCPTPIW) */

	    if ((rq = (int)tc.tcp_rnxt - (int)tc.tcp_rack - 1) < 0)
		rq = 0;
	    if ((sq = (int)tc.tcp_snxt - (int)tc.tcp_suna - 1) < 0)
		sq  = 0;

#  if	defined(HASTCPTPIQ)
	    Lf->lts.rq = (unsigned long)rq;
	    Lf->lts.sq = (unsigned long)sq;
	    Lf->lts.rqs = Lf->lts.sqs = 1;
#  endif	/* defined(HASTCPTPIQ) */

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

# else	/* !defined(HASTCPTPIQ) && !defined(HASTCPTPIW) */
	    if (!Fsize)
	        Lf->off_def = 1;
# endif	/* defined(HASTCPTPIQ) || defined(HASTCPTPIW) */

# if	defined(HASTCPOPT)
	    if (Ftcptpi & TCPTPI_FLAGS) {

	    /*
	     * Save TCP options and values..
	     */
		if (tc.tcp_naglim == (uint)1)
		    Lf->lts.topt |= TF_NODELAY;
		Lf->lts.mss = (unsigned long)tc.tcp_mss;
		Lf->lts.msss = (unsigned char)1;
	    }
# endif	/* defined(HASTCPOPT) */

# if	defined(HASSOOPT)
	    if (Ftcptpi & TCPTPI_FLAGS) {

	    /*
	     * Save socket options.
	     */
		if (tc.tcp_broadcast)
		    Lf->lts.opt |= SO_BROADCAST;
		if (tc.tcp_so_debug)
		    Lf->lts.opt |= SO_DEBUG;
		if (tc.tcp_dontroute)
		    Lf->lts.opt |= SO_DONTROUTE;
		if (tc.tcp_keepalive_intrvl
		&&  (tc.tcp_keepalive_intrvl != 7200000)
		) {
		    Lf->lts.opt |= SO_KEEPALIVE;
		    Lf->lts.kai = (unsigned int)tc.tcp_keepalive_intrvl;
		}
		if (tc.tcp_lingering) {
		    Lf->lts.opt |= SO_LINGER;
		    Lf->lts.ltm = (unsigned int)tc.tcp_linger;
		}
		if (tc.tcp_oobinline)
		    Lf->lts.opt |= SO_OOBINLINE;
		if (tc.tcp_reuseaddr)
		    Lf->lts.opt |= SO_REUSEADDR;
		if (tc.tcp_reuseport)
		    Lf->lts.opt |= SO_REUSEPORT;
		if (tc.tcp_useloopback)
		    Lf->lts.opt |= SO_USELOOPBACK;
		Lf->lts.qlen = (unsigned int)tc.tcp_conn_ind_cnt;
		Lf->lts.qlim = (unsigned int)tc.tcp_conn_ind_max;
		if (Lf->lts.qlen || Lf->lts.qlim)
		    Lf->lts.qlens = Lf->lts.qlims = (unsigned char)1;
	    }
# endif	/* defined(HASSOOPT) */

	    Namech[0] = '\0';
	    return;
	} else if (pn && !strcmp(pn, "UDP")) {

	/*
	 * Process UDP socket.
	 */
	    if (kread(pcb, (char *)&ud, sizeof(ud))) {
		ep = endnm(&sz);
		(void) snpf(ep, sz, "%scan't read UDP PCB from %s",
		    sz ? " " : "", print_kptr(pcb, (char *)NULL, 0));
		enter_nm(Namech);
		return;
	    }
	/*
	 * Save UDP address and TPI state.
	 */
	    la = (unsigned char *)&ic.ipc_udp_addr;
	    pt = (u_short)ic.ipc_udp_port;
	    if (((struct in_addr *)la)->s_addr == INADDR_ANY && pt == 0) {

	    /*
	     * If the ipc_s structure has no local address, use the one in the
	     * udp_s structure.
	     */
		pt = (u_short)ud.udp_port[0];
	    }
	    (void) ent_inaddr(la, (int)ntohs(pt), (unsigned char *)NULL,
		-1, AF_INET);
	    if (!Fsize)
		Lf->off_def = 1;
	    Lf->lts.type = 1;
	    Lf->lts.state.ui = (unsigned int)ud.udp_state;
	    Namech[0] = '\0';
	    return;
	} else {

	/*
	 * Record an unknown protocol.
	 */
	    ep = endnm(&sz);
	    (void) snpf(ep, sz, "%sunknown stream protocol: %s",
		sz ? " " : "", pn ? pn : "NUll");
	}
	if (Namech[0])
	    enter_nm(Namech);
}
#endif	/* HPUXV>=1030 */
