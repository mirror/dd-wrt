/*
 * dsock.c -- Darwin socket processing functions for libproc-based lsof
 */


/*
 * Portions Copyright 2005 Apple Computer, Inc.  All rights reserved.
 *
 * Copyright 2005 Purdue Research Foundation, West Lafayette, Indiana
 * 47907.  All rights reserved.
 *
 * Written by Allan Nathanson, Apple Computer, Inc., and Victor A.
 * Abell, Purdue University.
 *
 * This software is not subject to any license of the American Telephone
 * and Telegraph Company or the Regents of the University of California.
 *
 * Permission is granted to anyone to use this software for any purpose on
 * any computer system, and to alter it and redistribute it freely, subject
 * to the following restrictions:
 *
 * 1. Neither the authors, nor Apple Computer, Inc. nor Purdue University
 *    are responsible for any consequences of the use of this software.
 *
 * 2. The origin of this software must not be misrepresented, either
 *    by explicit claim or by omission.  Credit to the authors, Apple
 *    Computer, Inc. and Purdue University must appear in documentation
 *    and sources.
 *
 * 3. Altered versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 4. This notice may not be removed or altered.
 */


#ifndef lint
static char copyright[] =
"@(#) Copyright 2005 Apple Computer, Inc. and Purdue Research Foundation.\nAll rights reserved.\n";
static char *rcsid = "$Id: dsock.c,v 1.7 2012/04/10 16:41:04 abe Exp $";
#endif


#include "lsof.h"


/*
 * IPv6_2_IPv4()  -- macro to define the address of an IPv4 address contained
 *                 in an IPv6 address
 */

#define	IPv6_2_IPv4(v6)	(((uint8_t *)((struct in6_addr *)v6)->s6_addr)+12)


/*
 * process_socket() -- process socket file
 */

static void
process_socket_common(si)
	struct socket_fdinfo *si;
{
	unsigned char *fa = (unsigned char *)NULL;
	int fam, fp, lp, unl;
	unsigned char *la = (unsigned char *)NULL;

/*
 * Enter basic socket values.
 */
	(void) snpf(Lf->type, sizeof(Lf->type), "sock");
	Lf->inp_ty = 2;
/*
 * Enter basic file information.
 */
	enter_file_info(&si->pfi);
/*
 * Enable size or offset display.
 */
	if (Fsize) {
	    if (Lf->access == 'r')
		Lf->sz = (SZOFFTYPE)si->psi.soi_rcv.sbi_cc;
	    else if (Lf->access == 'w')
		Lf->sz = (SZOFFTYPE)si->psi.soi_snd.sbi_cc;
	    else
		Lf->sz = (SZOFFTYPE)(si->psi.soi_rcv.sbi_cc
		       +	     si->psi.soi_snd.sbi_cc);
	    Lf->sz_def = 1;
	} else
	    Lf->off_def = 1;

#if	defined(HASTCPTPIQ)
/*
 * Enter send and receive queue sizes.
 */
	Lf->lts.rq = si->psi.soi_rcv.sbi_cc;
	Lf->lts.sq = si->psi.soi_snd.sbi_cc;
	Lf->lts.rqs = Lf->lts.sqs = (unsigned char)1;
#endif	/* defined(HASTCPTPIQ) */

#if	defined(HASSOOPT)
/*
 * Enter socket options.
 */
	Lf->lts.ltm = (unsigned int)(si->psi.soi_linger & 0xffff);
	Lf->lts.opt = (unsigned int)(si->psi.soi_options & 0xffff);
	Lf->lts.pqlen = (unsigned int)si->psi.soi_incqlen;
	Lf->lts.qlen = (unsigned int)si->psi.soi_qlen;
	Lf->lts.qlim = (unsigned int)si->psi.soi_qlimit;
	Lf->lts.rbsz = (unsigned long)si->psi.soi_rcv.sbi_mbmax;
	Lf->lts.sbsz = (unsigned long)si->psi.soi_snd.sbi_mbmax;
	Lf->lts.pqlens = Lf->lts.qlens = Lf->lts.qlims = Lf->lts.rbszs
		       = Lf->lts.sbszs = (unsigned char)1;
#endif	/* defined(HASSOOPT) */

#if	defined(HASSOSTATE)
/*
 * Enter socket state.
 */
	Lf->lts.ss = (unsigned int)si->psi.soi_state;
#endif	/* defined(HASSOSTATE) */

/*
 * Process socket by its associated domain family.
 */
	switch ((fam = si->psi.soi_family)) {
	case AF_INET:
	case AF_INET6:

	/*
	 * Process IPv[46] sockets.
	 */
	    (void) snpf(Lf->type, sizeof(Lf->type),
			(fam == AF_INET) ? "IPv4" : "IPv6");
	    if ((si->psi.soi_kind != SOCKINFO_IN) &&
		(si->psi.soi_kind != SOCKINFO_TCP))
	    {
		break;
	    }
	/*
	 * Process TCP state inclusions and exclusions, as required.
	 */
	    if ((si->psi.soi_kind == SOCKINFO_TCP) && (TcpStXn || TcpStIn)) {
		int tsnx = (int)si->psi.soi_proto.pri_tcp.tcpsi_state
			 + TcpStOff;

		if ((tsnx >= 0) && (tsnx < TcpNstates)) {
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
	    }
	/*
	 * Process an Internet domain socket.
	 */
	    if (Fnet) {
		if (!FnetTy
		||  ((FnetTy == 4) && (fam == AF_INET))
		||  ((FnetTy == 6) && (fam == AF_INET6))
		)
		    Lf->sf |= SELNET;
	    }
	    printiproto(si->psi.soi_protocol);
	    if ((si->psi.soi_kind == SOCKINFO_TCP)
	    &&  si->psi.soi_proto.pri_tcp.tcpsi_tp)
	    {
		enter_dev_ch(print_kptr((KA_T)si->psi.soi_proto.pri_tcp.tcpsi_tp,
					(char *)NULL, 0));
	    } else
		enter_dev_ch(print_kptr((KA_T)si->psi.soi_pcb, (char *)NULL, 0));
	    if (fam == AF_INET) {

	    /*
	     * Enter IPv4 address information.
	     */
		if (si->psi.soi_kind == SOCKINFO_TCP) {

		/*
		 * Enter information for a TCP socket.
		 */
		    la = (unsigned char *)&si->psi.soi_proto.pri_tcp.tcpsi_ini.insi_laddr.ina_46.i46a_addr4;
		    lp = (int)ntohs(si->psi.soi_proto.pri_tcp.tcpsi_ini.insi_lport);
		    fa = (unsigned char *)&si->psi.soi_proto.pri_tcp.tcpsi_ini.insi_faddr.ina_46.i46a_addr4;
		    fp = (int)ntohs(si->psi.soi_proto.pri_tcp.tcpsi_ini.insi_fport);
		} else {

		/*
		 * Enter information for a non-TCP socket.
		 */
		    la = (unsigned char *)&si->psi.soi_proto.pri_in.insi_laddr.ina_46.i46a_addr4;
		    lp = (int)ntohs(si->psi.soi_proto.pri_in.insi_lport);
		    fa = (unsigned char *)&si->psi.soi_proto.pri_in.insi_faddr.ina_46.i46a_addr4;
		    fp = (int)ntohs(si->psi.soi_proto.pri_in.insi_fport);
		}
		if ((fa && (*fa == INADDR_ANY)) && !fp) {
		    fa = (unsigned char *)NULL;
		    fp = 0;
		}
	    } else {

	    /*
	     * Enter IPv6 address information
	     */
		int v4mapped = 0;

		if (si->psi.soi_kind == SOCKINFO_TCP)
		{

		/*
		 * Enter TCP socket information.
		 */
		    la = (unsigned char *)&si->psi.soi_proto.pri_tcp.tcpsi_ini.insi_laddr.ina_6;
		    lp = (int)ntohs(si->psi.soi_proto.pri_tcp.tcpsi_ini.insi_lport);
		    fa = (unsigned char *)&si->psi.soi_proto.pri_tcp.tcpsi_ini.insi_faddr.ina_6;
		    fp = (int)ntohs(si->psi.soi_proto.pri_tcp.tcpsi_ini.insi_fport);
		    if ((si->psi.soi_proto.pri_tcp.tcpsi_ini.insi_vflag & INI_IPV4) != 0)
			v4mapped = 1;
		} else {

		/*
		 * Enter non-TCP socket information.
		 */
		    la = (unsigned char *)&si->psi.soi_proto.pri_in.insi_laddr.ina_6;
		    lp = (int)ntohs(si->psi.soi_proto.pri_in.insi_lport);
		    fa = (unsigned char *)&si->psi.soi_proto.pri_in.insi_faddr.ina_6;
		    fp = (int)ntohs(si->psi.soi_proto.pri_in.insi_fport);
		    if ((si->psi.soi_proto.pri_in.insi_vflag & INI_IPV4) != 0)
			v4mapped = 1;
		}
		if (IN6_IS_ADDR_UNSPECIFIED((struct in6_addr *)fa) && !fp) {
		    fa = (unsigned char *)NULL;
		    fp = 0;
		}
		if (v4mapped) {

		/*
		 * Adjust IPv4 addresses mapped in IPv6 addresses.
		 */
		    fam = AF_INET;
		    if (la)
			la = (unsigned char *)IPv6_2_IPv4(la);
		    if (fa)
			fa = (unsigned char *)IPv6_2_IPv4(fa);
		}
	    }
	/*
	 * Enter local and remote addresses by address family.
	 */
	    if (fa || la)
		(void) ent_inaddr(la, lp, fa, fp, fam);
	    if (si->psi.soi_kind == SOCKINFO_TCP) {

	    /*
	     * Enter a TCP socket definition and its state.
	     */
		Lf->lts.type = 0;
		Lf->lts.state.i = (int)si->psi.soi_proto.pri_tcp.tcpsi_state;
	    /*
	     * Enter TCP options.
	     */

#if	defined(HASSOOPT)
		Lf->lts.kai = (unsigned int)si->psi.soi_proto.pri_tcp.tcpsi_timer[TCPT_KEEP];
#endif	/* defined(HASSOOPT) */

#if	defined(HASTCPOPT)
		Lf->lts.mss = (unsigned long)si->psi.soi_proto.pri_tcp.tcpsi_mss;
		Lf->lts.msss = (unsigned char)1;
		Lf->lts.topt = (unsigned int)si->psi.soi_proto.pri_tcp.tcpsi_flags;
#endif	/* defined(HASTCPOPT) */

	    }
	    break;
	case AF_UNIX:

	/*
	 * Process a UNIX domain socket.
	 */
	    (void) snpf(Lf->type, sizeof(Lf->type), "unix");
	    if (si->psi.soi_kind != SOCKINFO_UN)
		break;
	    if (Funix)
		Lf->sf |= SELUNX;
	    enter_dev_ch(print_kptr((KA_T)si->psi.soi_pcb, (char *)NULL, 0));
	/*
	 * Enter information on a UNIX domain socket that has no address bound
	 * to it, although it may be connected to another UNIX domain socket
	 * as a pipe.
	 */
	    if (si->psi.soi_proto.pri_un.unsi_addr.ua_sun.sun_family != AF_UNIX)
	    {
		if (si->psi.soi_proto.pri_un.unsi_addr.ua_sun.sun_family
		==  AF_UNSPEC)
		{
		    if (si->psi.soi_proto.pri_un.unsi_conn_pcb) {
			    (void) snpf(Namech, Namechl, "->%s",
				print_kptr((KA_T)si->psi.soi_proto.pri_un.unsi_conn_pcb, (char *)NULL, 0));
		    } else
			(void) snpf(Namech, Namechl, "->(none)");
		} else
		    (void) snpf(Namech, Namechl, "unknown sun_family (%d)",
			si->psi.soi_proto.pri_un.unsi_addr.ua_sun.sun_family);
		break;
	    }
	    if (si->psi.soi_proto.pri_un.unsi_addr.ua_sun.sun_path[0]) {
		unl = si->psi.soi_proto.pri_un.unsi_addr.ua_sun.sun_len - offsetof(struct sockaddr_un, sun_path);
		if ((unl < 0) || (unl >= sizeof(si->psi.soi_proto.pri_un.unsi_addr.ua_sun.sun_path)))
		    unl = sizeof(si->psi.soi_proto.pri_un.unsi_addr.ua_sun.sun_path) - 1;
		si->psi.soi_proto.pri_un.unsi_addr.ua_sun.sun_path[unl] = '\0';
		if (si->psi.soi_proto.pri_un.unsi_addr.ua_sun.sun_path[0]
		&&  Sfile
		&&  is_file_named(si->psi.soi_proto.pri_un.unsi_addr.ua_sun.sun_path, 0))
		    Lf->sf |= SELNM;
		if (si->psi.soi_proto.pri_un.unsi_addr.ua_sun.sun_path[0]
		&&  !Namech[0])
		    (void) snpf(Namech, Namechl, "%s", si->psi.soi_proto.pri_un.unsi_addr.ua_sun.sun_path);
	    } else
		(void) snpf(Namech, Namechl, "no address");
	    break;
	case AF_ROUTE:

	/*
	 * Process a ROUTE domain socket.
	 */
	    (void) snpf(Lf->type, sizeof(Lf->type), "rte");
	    if (!Fsize)
		Lf->off_def = 1;
	    break;
	case AF_NDRV:

	/*
	 * Process an NDRV domain socket.
	 */
	    (void) snpf(Lf->type, sizeof(Lf->type), "ndrv");
	    if (si->psi.soi_kind != SOCKINFO_NDRV)
		break;
	    enter_dev_ch(print_kptr((KA_T)si->psi.soi_pcb, (char *)NULL, 0));
	    si->psi.soi_proto.pri_ndrv.ndrvsi_if_name[sizeof(si->psi.soi_proto.pri_ndrv.ndrvsi_if_name) - 1] = '\0';
	    (void) snpf(Namech, Namechl, "-> %s%d",
			si->psi.soi_proto.pri_ndrv.ndrvsi_if_name,
			si->psi.soi_proto.pri_ndrv.ndrvsi_if_unit);
	    break;
	case pseudo_AF_KEY:

	/*
	 * Process an [internal] key-management function socket.
	 */
	    (void) snpf(Lf->type, sizeof(Lf->type), "key");
	    enter_dev_ch(print_kptr((KA_T)si->psi.soi_pcb, (char *)NULL, 0));
	    break;
	case AF_SYSTEM:

	/*
	 * Process a SYSTEM domain socket.
	 */
	    (void) snpf(Lf->type, sizeof(Lf->type), "systm");
	    if (si->psi.soi_kind != SOCKINFO_KERN_EVENT)
		break;
	    enter_dev_ch(print_kptr((KA_T)si->psi.soi_pcb, (char *)NULL, 0));
	    (void) snpf(Namech, Namechl, "[%x:%x:%x]",
			si->psi.soi_proto.pri_kern_event.kesi_vendor_code_filter,
			si->psi.soi_proto.pri_kern_event.kesi_class_filter,
			si->psi.soi_proto.pri_kern_event.kesi_subclass_filter);
	    break;
	case AF_PPP:

	/*
	 * Process a PPP domain socket.
	 */
	    (void) snpf(Lf->type, sizeof(Lf->type), "ppp");
	    enter_dev_ch(print_kptr((KA_T)si->psi.soi_pcb, (char *)NULL, 0));
	    break;
	default:
	    printunkaf(fam, 1);
	}
/*
 * If there are NAME column characters, enter them.
 */
	if (Namech[0])
	    enter_nm(Namech);
}


void
process_socket(pid, fd)
	int pid;			/* PID */
	int32_t fd;			/* FD */
{
	int nb;
	struct socket_fdinfo si;
/*
 * Get socket information.
 */
	nb = proc_pidfdinfo(pid, fd, PROC_PIDFDSOCKETINFO, &si, sizeof(si));
	if (nb <= 0) {
	    (void) err2nm("socket");
	    return;
	} else if (nb < sizeof(si)) {
	    (void) fprintf(stderr,
		"%s: PID %d, FD %d: proc_pidfdinfo(PROC_PIDFDSOCKETINFO);\n",
		Pn, pid, fd);
	    (void) fprintf(stderr,
		"      too few bytes; expected %ld, got %d\n",
		sizeof(si), nb);
	    Exit(1);
	}

	process_socket_common(&si);
}


#ifdef	PROC_PIDLISTFILEPORTS
void
process_fileport_socket(pid, fp)
	int pid;			/* PID */
	uint32_t fp;			/* FILEPORT */
{
	int nb;
	struct socket_fdinfo si;
/*
 * Get socket information.
 */
	nb = proc_pidfileportinfo(pid, fp, PROC_PIDFILEPORTSOCKETINFO, &si, sizeof(si));
	if (nb <= 0) {
	    (void) err2nm("socket");
	    return;
	} else if (nb < sizeof(si)) {
	    (void) fprintf(stderr,
		"%s: PID %d, FILEPORT %u: proc_pidfileportinfo(PROC_PIDFILEPORTSOCKETINFO);\n",
		Pn, pid, fp);
	    (void) fprintf(stderr,
		"      too few bytes; expected %ld, got %d\n",
		sizeof(si), nb);
	    Exit(1);
	}

	process_socket_common(&si);
}
#endif	/* PROC_PIDLISTFILEPORTS */
