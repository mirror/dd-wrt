/*
 * dsock.c - SCO OpenServer socket processing functions for lsof
 */


/*
 * Copyright 1995 Purdue Research Foundation, West Lafayette, Indiana
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
"@(#) Copyright 1995 Purdue Research Foundation.\nAll rights reserved.\n";
static char *rcsid = "$Id: dsock.c,v 1.15 2004/03/10 23:52:12 abe Exp $";
#endif


#include "lsof.h"


/*
 * process_socket() - process socket
 */

void
process_socket(i)
	struct inode *i;		/* inode pointer */
{
	char *cp;
	struct domain d;
	unsigned char *fa = (unsigned char *)NULL;
	int fam, j, k;
	int fp, lp;
	unsigned char *la = (unsigned char *)NULL;
	int p;
	struct inpcb pcb;
	short pcbs = 0;
	short  udpsf, udpsl;
	struct socket s;
	KA_T sa, spa;
	struct stdata sd;
	struct queue sh;
	short shs = 0;
	struct tcpcb t;
	short ts = 0;
	struct udpdev udp;
	short udptm = 0;

#if	OSRV<500
	struct sockaddr_in *si;
#else	/* OSRV>=500 */
	struct sockaddr_in si;
	struct un_dev ud;
#endif	/* OSRV<500 */
	
	(void) snpf(Lf->type, sizeof(Lf->type), "sock");
/*
 * Read socket.
 */
	if (!Socktab) {
	    (void) enter_nm("No kernel socket table");
	    return;
	}
	spa = Socktab + (GET_MIN_DEV(i->i_rdev) * sizeof(struct socket *));
	if (kread(spa, (char *)&sa, sizeof(sa))) {
	    (void) snpf(Namech, Namechl, "can't read socket pointer at %s",
		print_kptr(spa, (char *)NULL, 0));
	    enter_nm(Namech);
	}
	if (kread(sa, (char *)&s, sizeof(s))) {
	    (void) snpf(Namech, Namechl, "can't read socket structure at %s",
		print_kptr(sa, (char *)NULL, 0));
	    enter_nm(Namech);
	    return;
	}
/*
 * Read domain structure.
 */
	if (!s.so_proto.pr_domain
	||  kread((KA_T)s.so_proto.pr_domain, (char *)&d, sizeof(d))) {
	    (void) snpf(Namech, Namechl, "can't read protocol domain from %s",
		print_kptr((KA_T)s.so_proto.pr_domain, (char *)NULL, 0));
	    enter_nm(Namech);
	    return;
	}
/*
 * Process by protocol domain.
 */
	switch ((fam = d.dom_family)) {
	case AF_INET:
	    if (Fnet)
		Lf->sf |= SELNET;
	    (void) snpf(Lf->type, sizeof(Lf->type), "inet");
	    printiproto((int)s.so_proto.pr_protocol);
	    Lf->inp_ty = 2;
	/*
	 * Get protocol control block address from stream head queue structure.
	 */
	    if (s.so_stp
	    &&  !readstdata((KA_T)s.so_stp, &sd)
	    &&  !readsthead((KA_T)sd.sd_wrq, &sh))
		shs = 1;
	    if (shs && sh.q_ptr) {
		enter_dev_ch(print_kptr((KA_T)sh.q_ptr, (char *)NULL, 0));
		if (kread((KA_T)sh.q_ptr, (char *)&pcb, sizeof(pcb)) == 0)
		    pcbs = 1;
	    }
	/*
	 * Print local and remote addresses.
	 */
	    if (pcbs) {
		if (pcb.inp_ppcb && strcasecmp(Lf->iproto, "udp") == 0) {

		/*
		 * If this is a UDP socket file, get the udpdev structure
		 * at the PCB's per-protocol control block address.  It
		 * may contain a foreign address.
		 */
		    if (!kread((KA_T)pcb.inp_ppcb, (char *)&udp, sizeof(udp))) {

#if	OSRV>=500
			if (udp.ud_lsin.sin_addr.s_addr != INADDR_ANY
			||  udp.ud_lsin.sin_port != 0)
			    udpsl = 1;
			else
			    udpsl = 0;
#endif	/* OSRV>=500 */

			if (udp.ud_fsin.sin_addr.s_addr != INADDR_ANY
			||  udp.ud_fsin.sin_port != 0)
			    udpsf = 1;
			else
			    udpsf = 0;
		    }
		} else
		    udpsf = udpsl = 0;
	    /*
	     * Print the local address from the PCB.  If there is none, and if
	     * this is a 5.0.0 or greater UDP stream, and if it has a local
	     * address set, use it.
	     */
		la = (unsigned char *)&pcb.inp_laddr;
		lp = (int)ntohs(pcb.inp_lport);

#if     OSRV>=500
		if (((struct in_addr *)la)->s_addr == INADDR_ANY
		&&  lp == 0 && udpsl) {
		    la = (unsigned char *)&udp.ud_lsin.sin_addr;
		    lp = (int)ntohs(udp.ud_lsin.sin_port);
		}

#endif  /* OSRV>=500 */

	    /*
	     * Use the PCB's foreign address if it is set.  If not, and if this
	     * is a UDP socket file, use the udpdev structure's foreign address
	     * if it's set.
	     */
		if (pcb.inp_faddr.s_addr != INADDR_ANY || pcb.inp_fport != 0) {
		    fa = (unsigned char *)&pcb.inp_faddr;
		    fp = (int)ntohs(pcb.inp_fport);
		} else if (udpsf) {
		    fa = (unsigned char *)&udp.ud_fsin.sin_addr;
		    fp = (int)ntohs(udp.ud_fsin.sin_port);
		    udptm = 1;
		}
		if (la || fa) {
		    (void) ent_inaddr(la, lp, fa, fp, AF_INET);
		    if (udptm && !Lf->nma)
			(void)udp_tm(udp.ud_ftime);
		}
		if (pcb.inp_ppcb && strcasecmp(Lf->iproto, "tcp") == 0
		&&  kread((KA_T)pcb.inp_ppcb, (char *)&t, sizeof(t)) == 0) {
		    ts = 1;
		/*
		 * Save the TCP state from its control block.
		 */
		    Lf->lts.type = 0;
		    Lf->lts.state.i = (int)t.t_state;
		}
	    } else {

#if	OSRV<500
		if ((si = (struct sockaddr_in *)&s.so_name)) {
		    la = (unsigned char *)&si->sin_addr;
		    lp = (int)ntohs(si->sin_port);
		}
		if ((si = (struct sockaddr_in *)&s.so_peer)) {
		    if (si->sin_addr.s_addr != INADDR_ANY || si->sin_port != 0)
		    {
			fa = (unsigned char *)&si->sin_addr;
			fp = (int)ntohs(si->sin_port);
		    }
		}
#else	/* OSRV>=500 */
		if (s.so_name
		&&  !kread((KA_T)s.so_name, (char *)&si, sizeof(si))) {
		    la = (unsigned char *)&si.sin_addr;
		    lp = (int)ntohs(si.sin_port);
		}
		if (s.so_peer
		&&  !kread((KA_T)s.so_peer, (char *)&si, sizeof(si))) {
		    if (si.sin_addr.s_addr != INADDR_ANY || si.sin_port != 0) {
			fa = (unsigned char *)&si.sin_addr;
			fp = (int)ntohs(si.sin_port);
		    }
		}
#endif	/* OSRV<500 */

		if (la || fa)
		    (void) ent_inaddr(la, lp, fa, fp, AF_INET);
	    }
	/*
	 * Save options, sizes, states and values.
	 */

#if	defined(HASSOOPT)
	    Lf->lts.ltm = (unsigned int)s.so_linger;
	    Lf->lts.opt = (unsigned int)s.so_options;
	    Lf->lts.qlen = (unsigned int)s.so_qlen;
	    Lf->lts.qlim = (unsigned int)s.so_qlimit;
	    Lf->lts.qlens = Lf->lts.qlims = (unsigned char)1;
	    if (ts && t.t_timer[TCPT_KEEP]) {
		Lf->lts.opt |= SO_KEEPALIVE;
		Lf->lts.kai = (unsigned long)t.t_timer[TCPT_KEEP];
	    }
#endif	/* defined(HASSOOPT) */

#if	defined(HASSOSTATE)
	    Lf->lts.ss = s.so_state;
#endif	/* defined(HASSOSTATE) */


	    if (ts) {

#if	defined(HASTCPOPT)
		Lf->lts.topt = (unsigned int)t.t_flags;
		Lf->lts.mss = (unsigned long)t.t_maxseg;
		Lf->lts.msss = (unsigned char)1;
#endif	/* defined(HASTCPOPT) */

#if	defined(HASTCPTPIQ)
		Lf->lts.rq = (unsigned long)t.t_iqsize;
		Lf->lts.sq = (unsigned long)t.t_qsize;
		Lf->lts.rqs = Lf->lts.sqs = 1;
#endif	/* defined(HASTCPTPIQ) */

		if (Fsize) {
		    if (Lf->access == 'r')
			Lf->sz = (SZOFFTYPE)t.t_iqsize;
		    else if (Lf->access == 'w')
			Lf->sz = (SZOFFTYPE)t.t_qsize;
		    else
			Lf->sz = (SZOFFTYPE)(t.t_iqsize + t.t_qsize);
		    Lf->sz_def = 1;
		} else
		    Lf->off_def = 1;
	    } else if (shs) {
		if (Fsize) {
		    Lf->sz = (SZOFFTYPE)sh.q_count;
		    Lf->sz_def = 1;
		} else
		    Lf->off_def = 1;
	    } else
		Lf->off_def = 1;
	    break;

#if	OSRV>=500
	case AF_UNIX:
	    if (Funix)
		Lf->sf |= SELUNX;
	    (void) snpf(Lf->type, sizeof(Lf->type), "unix");
	/*
	 * Read Unix protocol control block and the Unix address structure.
	 */
	    enter_dev_ch(print_kptr(sa, (char *)NULL, 0));
	    Lf->off_def = 1;
	    if (s.so_stp
	    &&  !readstdata((KA_T)s.so_stp, &sd)
	    &&  !readsthead((KA_T)sd.sd_wrq, &sh)) {
		if (!sh.q_ptr
		||  kread((KA_T)sh.q_ptr, (char *)&ud, sizeof(ud)))
		{
		    (void) snpf(Namech, Namechl, "can't read un_dev from %s",
			print_kptr((KA_T)sh.q_ptr, (char *)NULL, 0));
		    break;
		}
		if (ud.so_rq)
		    enter_dev_ch(print_kptr((KA_T)ud.so_rq, (char *)NULL, 0));
		if (ud.local_addr.sun_family == AF_UNIX) {
		    Lf->inode = (unsigned long)ud.bnd_param.user_addr.inode_no;
		    Lf->inp_ty = 1;
		    ud.local_addr.sun_path[sizeof(ud.local_addr.sun_path) - 1]
			= '\0';
		    if (Sfile && is_file_named(ud.local_addr.sun_path, 0))
			Lf->sf |= SELNM;
		    if (!Namech[0])
			(void) snpf(Namech,Namechl,"%s",ud.local_addr.sun_path);
		} else if (ud.for_addr.sun_family == AF_UNIX) {
		    Lf->inode = (unsigned long)ud.bnd_param.user_addr.inode_no;
		    Lf->inp_ty = 1;
		    ud.for_addr.sun_path[sizeof(ud.for_addr.sun_path) - 1]
			= '\0';
		    if (Sfile && is_file_named(ud.for_addr.sun_path, 0))
			Lf->sf |= SELNM;
		    else
			(void) snpf(Namech,Namechl,"%s",ud.for_addr.sun_path);
		} else if (ud.other_q)
		    (void) snpf(Namech, Namechl, "->%s",
			print_kptr((KA_T)ud.other_q, (char *)NULL, 0));
	    } else
		(void) snpf(Namech, Namechl, "can't get un_dev");
	    break;
#endif	/* OSRV>=500 */

	default:
	    printunkaf(fam, 1);
	}
	enter_nm(Namech);
}


/*
 * udp_tm() - compute time since UDP packet was last sent
 */

void
udp_tm(tm)
	time_t tm;			/* time when packet was sent */
{
	static char buf[32], *cp;
	time_t et, lbolt;
	MALLOC_S len;
	short hr, min, sec;
/*
 * Read the lightning bolt timer and compute the elapsed time.
 * No elapsed time is returned if:
 *	the global clock frequency variable, Hz, is negative;
 *	the lightning bolt timer is unavailable;
 *	the lightning bolt time is less than the UDP send time;
 *	the elapsed time is zero.
 */
	if (!Lbolt)
	    return;
	if (Hz < 0 
	||  kread((KA_T)Lbolt, (char *)&lbolt, sizeof(lbolt))
	||  tm >= lbolt
	||  (et = (time_t)((lbolt - tm) / Hz)) == 0)
	    return;
/*
 * If the time is 100 hours or greater, return the elapsed time as seconds.
 */
	if (et >= (100 * 60 * 60)) {
	    (void) snpf(buf, sizeof(buf), "%lds", et);
	    cp = &buf[strlen(buf)];
	} else {

	/*
	 * Convert seconds to hours, minutes and seconds.
	 */
	    hr = (short)(et / (60 * 60));
	    et %= (60 * 60);
	    min = (short)(et / 60);
	    sec = (short)(et % 60);
	    cp = buf;
	/*
 	 * Format the elapsed time and attach single character suffixes to
	 * represent the units:
	 *
	 *    `h' = hours
	 *    `m' = minutes
	 *    `s' = seconds
 	 */
	    if (hr) {
		(void) snpf(cp, sizeof(buf) - (cp - buf), "%dh", hr);
		cp += 2 + ((hr > 9) ? 1 : 0);
	    }
	    if (min) {
		(void) snpf(cp, sizeof(buf) - (cp - buf), "%dm", min);
		cp += 2 + ((min > 9) ? 1 : 0);
	    }
	    if (sec) {
		(void) snpf(cp, sizeof(buf) - (cp - buf), "%ds", sec);
		cp += 2 + ((sec > 9) ? 1 : 0);
	    }
	}
/*
 * Add the `` ago'' trailer.  Return the string's address and length.
 */
	(void) snpf(cp, sizeof(buf) - (cp - buf), " ago");
	len = (MALLOC_S)(strlen(buf) + 1);
	if (len < 2)
	    return;
	if (!(cp = (char *)malloc(len))) {
	    (void) fprintf(stderr, "%s: no space for %d character UDP time\n",
		Pn, len);
	    Exit(1);
	}
	(void) snpf(cp, len, "%s", buf);
	Lf->nma = cp;
}
