/*
 * dsock.c - AIX socket processing functions for lsof
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
static char *rcsid = "$Id: dsock.c,v 1.24 2008/10/21 16:14:18 abe Exp $";
#endif


#include "lsof.h"


/*
 * We include <sys/domain.h> here instead of "dlsof.h" for gcc's benefit.
 * Its loader can't handle the multiple CONST u_char arrays declared in
 * <net/net_globals.h> -- e.g., etherbroadcastaddr[].  (<sys/domain.h>
 * #include's <net/net_globals.h>.)
 */

#include <net/netopt.h>
#include <sys/domain.h>


/*
 * process_socket() - process socket file
 */

void
process_socket(sa)
	KA_T sa;			/* socket address in kernel */
{
	struct domain d;
	unsigned char *fa = (unsigned char *)NULL;
	int fam;
	int fp, lp, uo;
	struct gnode g;
	struct l_ino i;
	struct inpcb inp;
	int is = 0;
	unsigned char *la = (unsigned char *)NULL;
	struct protosw p;
	struct socket s;
	struct tcpcb t;
	int ts = 0;
	int tsn, tsnx;
	struct unpcb uc, unp;
	struct sockaddr_un *ua = (struct sockaddr_un *)NULL;
	struct sockaddr_un un;
	struct vnode v;
	struct mbuf mb;
/*
 * Set socket file variables.
 */
	(void) snpf(Lf->type, sizeof(Lf->type), "sock");
	Lf->inp_ty = 2;
/*
 * Read socket and protocol switch structures.
 */
	if (!sa) {
	    enter_nm("no socket address");
	    return;
	}
	if (kread(sa, (char *) &s, sizeof(s))) {
	    (void) snpf(Namech, Namechl, "can't read socket struct from %s",
		print_kptr(sa, (char *)NULL, 0));
	    enter_nm(Namech);
	    return;
	}
	if (!s.so_type) {
	    enter_nm("no socket type");
	    return;
	}
	if (!s.so_proto
	||  kread((KA_T)s.so_proto, (char *)&p, sizeof(p))) {
	    (void) snpf(Namech, Namechl, "can't read protocol switch from %s",
		print_kptr((KA_T)s.so_proto, (char *)NULL, 0));
	    enter_nm(Namech);
	    return;
	}
/*
 * Save size information.
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

#if	defined(HASTCPTPIQ)
	Lf->lts.rq = s.so_rcv.sb_cc;
	Lf->lts.sq = s.so_snd.sb_cc;
	Lf->lts.rqs = Lf->lts.sqs = 1;
#endif	/* defined(HASTCPTPIQ) */

#if	defined(HASSOOPT)
	Lf->lts.ltm = (unsigned int)s.so_linger;
	Lf->lts.opt = (unsigned int)s.so_options;
	Lf->lts.pqlen = (unsigned int)s.so_q0len;
	Lf->lts.qlen = (unsigned int)s.so_qlen;
	Lf->lts.qlim = (unsigned int)s.so_qlimit;
	Lf->lts.rbsz = (unsigned long)s.so_rcv.sb_mbmax;
	Lf->lts.sbsz = (unsigned long)s.so_snd.sb_mbmax;
	Lf->lts.pqlens = Lf->lts.qlens = Lf->lts.qlims = Lf->lts.rbszs
		       = Lf->lts.sbszs = (unsigned char)1;
#endif	/* defined(HASSOOPT) */

#if	defined(HASSOSTATE)
	Lf->lts.ss = (unsigned int)s.so_state;
#endif	/* defined(HASSOSTATE) */

/*
 * Process socket by the associated domain family.
 */
	if (!p.pr_domain
	||  kread((KA_T)p.pr_domain, (char *)&d, sizeof(d))) {
	    (void) snpf(Namech, Namechl, "can't read domain struct from %s",
		print_kptr((KA_T)p.pr_domain, (char *)NULL, 0));
	    enter_nm(Namech);
	    return;
	}
	switch ((fam = d.dom_family)) {
/*
 * Process an Internet domain socket.
 */
	case AF_INET:

#if	defined(HASIPv6)
	case AF_INET6:
#endif	/* defined(HASIPv6) */

	/*
	 * Read protocol control block.
	 */
	    if (!s.so_pcb
	    ||  kread((KA_T) s.so_pcb, (char *) &inp, sizeof(inp))) {
		if (!s.so_pcb) {
		    (void) snpf(Namech, Namechl, "no PCB%s%s",
			(s.so_state & SS_CANTSENDMORE) ? ", CANTSENDMORE" : "",
			(s.so_state & SS_CANTRCVMORE)  ? ", CANTRCVMORE"  : "");
		} else {
		    (void) snpf(Namech, Namechl, "can't read inpcb at %s",
			print_kptr((KA_T)s.so_pcb, (char *)NULL, 0));
		}
		enter_nm(Namech);
		return;
	    }
	    if (p.pr_protocol == IPPROTO_TCP) {

	    /*
	     * If this is a TCP socket, read its control block.
	     */
		if (inp.inp_ppcb
	        &&  !kread((KA_T)inp.inp_ppcb, (char *)&t, sizeof(t)))
		{
		    ts = 1;
		    tsn = (int)t.t_state;
		    tsnx = tsn + TcpStOff;
		}
	    }
	    if (ts
	    &&  (TcpStIn || TcpStXn)
	    &&  (tsnx >= 0) && (tsnx < TcpNstates)
	    ) {

	    /*
	     * Check TCP state name inclusion and exclusions.
	     */
		if (TcpStXn) {
		    if (TcpStX[tsnx]) {
			Lf->sf |= SELEXCLF;
			return;
		    }
		}
		if (TcpStIn) {
		    if (TcpStI[tsnx])
			TcpStI[tsnx] = 2;
		    else {
			Lf->sf |= SELEXCLF;
			return;
		    }
		}
	    }
	    if (Fnet) {

	    /*
	     * Set SELNET flag for the file, as requested.
	     */
		if (!FnetTy
		||  ((FnetTy == 4) && (fam == AF_INET))

#if	defined(HASIPv6)
		||  ((FnetTy == 6) && (fam == AF_INET6))
#endif	/* defined(HASIPv6) */
		)

		    Lf->sf |= SELNET;
	    }
	    printiproto(p.pr_protocol);

#if	defined(HASIPv6)
	    (void) snpf(Lf->type, sizeof(Lf->type),
			fam == AF_INET ? "IPv4" : "IPv6");
#else	/* !defined(HASIPv6) */
	    (void) snpf(Lf->type, sizeof(Lf->type), "inet");
#endif	/* defined(HASIPv6) */

	/*
	 * Save Internet socket information.
	 */
	    enter_dev_ch(print_kptr((KA_T)(inp.inp_ppcb ? inp.inp_ppcb
							: s.so_pcb),
				    (char *)NULL, 0));

#if	defined(HASIPv6)
	/*
	 * If this is an IPv6 (AF_INET6) socket and IPv4 compatibility
	 * mode is enabled, use the IPv4 address, change the family
	 * indicator from AF_INET6 to AF_INET.  Otherwise, use the IPv6
	 * address.  Don't ignore empty addresses.
	 */
	    if (fam == AF_INET6) {
		if (inp.inp_flags & INP_COMPATV4) {
		    fam = AF_INET;
		    la = (unsigned char *)&inp.inp_laddr;
		} else
		    la = (unsigned char *)&inp.inp_laddr6;
	    } else
#endif	/* defined(HASIPv6) */

		la = (unsigned char *)&inp.inp_laddr;
	    lp = (int)ntohs(inp.inp_lport);
	    if (fam == AF_INET
	    &&  (inp.inp_faddr.s_addr != INADDR_ANY || inp.inp_fport != 0)) {
		fa =  (unsigned char *)&inp.inp_faddr;
		fp = (int)ntohs(inp.inp_fport);
	    }

#if	defined(HASIPv6)
	    else if (fam == AF_INET6) {

	    /*
	     * If this is an IPv6 (AF_INET6) socket and IPv4 compatibility
	     * mode is enabled, use the IPv4 address, change the family
	     * indicator from AF_INET6 to AF_INET.  Otherwise, use the IPv6
	     * address.  Ignore empty addresses.
	     */
		if (inp.inp_flags & INP_COMPATV4) {
		    fam = AF_INET;
		    if (inp.inp_faddr.s_addr != INADDR_ANY
		    || inp.inp_fport != 0)
		    {
			fa = (unsigned char *)&inp.inp_faddr;
			fp = (int)ntohs(inp.inp_fport);
		    }
		} else {
		    if (!IN6_IS_ADDR_UNSPECIFIED(&inp.inp_faddr6)) {
			fa = (unsigned char *)&inp.inp_faddr6;
			fp = (int)ntohs(inp.inp_fport);
		    }
		}
	    }
#endif	/* defined(HASIPv6) */

	    if (fa || la)
		(void) ent_inaddr(la, lp, fa, fp, fam);
	    if (ts) {
		Lf->lts.type = 0;
		Lf->lts.state.i = tsn;

#if	defined(HASSOOPT)
		Lf->lts.kai = (unsigned int)t.t_timer[TCPT_KEEP];
#endif	/* defined(HASSOOPT) */

#if	defined(HASTCPOPT)
		Lf->lts.mss = (unsigned long)t.t_maxseg;
		Lf->lts.msss = (unsigned char)1;
		Lf->lts.topt = (unsigned int)t.t_flags;
#endif	/* defined(HASTCPOPT) */

	    }
	    break;
/*
 * Process a ROUTE domain socket.
 */
	case AF_ROUTE:
	    (void) snpf(Lf->type, sizeof(Lf->type), "rte");
	    if (s.so_pcb)
		enter_dev_ch(print_kptr((KA_T)(s.so_pcb), (char *)NULL, 0));
	    else
		(void) snpf(Namech, Namechl, "no protocol control block");
	    if (!Fsize)
		Lf->off_def = 1;
	    break;
/*
 * Process a Unix domain socket.
 */
	case AF_UNIX:
	    if (Funix)
		Lf->sf |= SELUNX;
	    (void) snpf(Lf->type, sizeof(Lf->type), "unix");
	/*
	 * Read Unix protocol control block and the Unix address structure.
	 */
	    enter_dev_ch(print_kptr(sa, (char *)NULL, 0));
	    if (kread((KA_T) s.so_pcb, (char *)&unp, sizeof(unp))) {
		(void) snpf(Namech, Namechl, "can't read unpcb at %s",
		    print_kptr((KA_T)s.so_pcb, (char *)NULL, 0));
		break;
	    }
	    if ((struct socket *)sa != unp.unp_socket) {
		(void) snpf(Namech, Namechl, "unp_socket (%s) mismatch",
		    print_kptr((KA_T)unp.unp_socket, (char *)NULL, 0));
		break;
	    }
	    if (unp.unp_addr) {
		if (kread((KA_T) unp.unp_addr, (char *)&mb, sizeof(mb))) {
		    (void) snpf(Namech, Namechl, "can't read unp_addr at %s",
			print_kptr((KA_T)unp.unp_addr, (char *)NULL, 0));
		    break;
		}

#if	AIXV>=3200
		uo = (int)(mb.m_hdr.mh_data - (caddr_t)unp.unp_addr);
		if ((uo + sizeof(struct sockaddr)) <= sizeof(mb))
		    ua = (struct sockaddr_un *)((char *)&mb + uo);
		else {
		    if (mb.m_hdr.mh_data
		    &&  !kread((KA_T)mb.m_hdr.mh_data, (char *)&un, sizeof(un))
		    ) {
			ua = &un;
		    }
		}
#else	/* AIXV<3200 */
		ua = (struct sockaddr_un *)(((char *)&mb) + mb.m_off);
#endif	/* AIXV>=3200 */

	    }
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
			if (kread((KA_T)unp.unp_conn, (char *)&uc, sizeof(uc)))
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
	/*
	 * Read any associated vnode and then read its gnode and inode.
	 */
	    g.gn_type = VSOCK;
	    if (unp.unp_vnode
	    &&  !readvnode((KA_T)unp.unp_vnode, &v)) {
		if (v.v_gnode
		&&  !readgnode((KA_T)v.v_gnode, &g)) {
		    Lf->lock = isglocked(&g);
		    if (g.gn_type == VSOCK && g.gn_data
		    && !readlino(&g, &i))
			is = 1;
		}
	    }
	/*
	 * Print Unix socket information.
	 */
	    if (is) {
		Lf->dev = i.dev;
		Lf->dev_def = i.dev_def;
		if (Lf->dev_ch) {
		    (void) free((FREE_P *)Lf->dev_ch);
		    Lf->dev_ch = (char *)NULL;
		}
		Lf->inode = (INODETYPE)i.number;
		Lf->inp_ty = i.number_def;
	    }
	    if (ua->sun_path[0]) {
		if (mb.m_len > sizeof(struct sockaddr_un))
		    mb.m_len = sizeof(struct sockaddr_un);
		*((char *)ua + mb.m_len - 1) = '\0';
		if (Sfile && is_file_named(ua->sun_path, VSOCK, 0, 0))
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
