/*
 * dsock.c - SCO UnixWare socket processing functions for lsof
 */


/*
 * Copyright 1996 Purdue Research Foundation, West Lafayette, Indiana
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
"@(#) Copyright 1996 Purdue Research Foundation.\nAll rights reserved.\n";
static char *rcsid = "$Id: dsock.c,v 1.16 2005/08/13 16:21:41 abe Exp $";
#endif


#define	TCPSTATES		/* activate tcpstates[] */
#include "lsof.h"


/*
 * Local function prototypes
 */

#if	UNIXWAREV>=70101 && UNIXWAREV<70103
_PROTOTYPE(static struct sockaddr_un *find_unix_sockaddr_un,(KA_T ka));
#endif	/* UNIXWAREV>=70101 && UNIXWAREV<70103 */


/*
 * print_tcptpi() - print TCP/TPI info
 */

void
print_tcptpi(nl)
	int nl;				/* 1 == '\n' required */
{
	char buf[128];
	char *cp = (char *)NULL;
	int ps = 0;
	int s;

	if (Ftcptpi & TCPTPI_STATE) {
	    s = Lf->lts.state.i;
	    switch (Lf->lts.type) {
	    case 0:
		if (s < 0 || s >= TCP_NSTATES) {
		    (void) snpf(buf, sizeof(buf), "UNKNOWN_TCP_STATE_%d", s);
		    cp = buf;
		} else
		    cp = tcpstates[s];
		break;
	    case 1:
		switch (s) {
		case TS_UNBND:
		    cp = "TS_UNBND";
		    break;
		case TS_WACK_BREQ:
		    cp = "TS_WACK_BREQ";
		    break;
		case TS_WACK_UREQ:
		    cp = "TS_WACK_UREQ";
		    break;
		case TS_IDLE:
		    cp = "TS_IDLE";
		    break;
		case TS_WACK_OPTREQ:
		    cp = "TS_WACK_OPTREQ";
		    break;
		case TS_WACK_CREQ:
		    cp = "TS_WACK_CREQ";
		    break;
		case TS_WCON_CREQ:
		    cp = "TS_WCON_CREQ";
		    break;
		case TS_WRES_CIND:
		    cp = "TS_WRES_CIND";
		    break;
		case TS_WACK_CRES:
		    cp = "TS_WACK_CRES";
		    break;
		case TS_DATA_XFER:
		    cp = "TS_DATA_XFER";
		    break;
		case TS_WIND_ORDREL:
		    cp = "TS_WIND_ORDREL";
		    break;
		case TS_WREQ_ORDREL:
		    cp = "TS_WREQ_ORDREL";
		    break;
		case TS_WACK_DREQ6:
		    cp = "TS_WACK_DREQ6";
		    break;
		case TS_WACK_DREQ7:
		    cp = "TS_WACK_DREQ7";
		    break;
		case TS_WACK_DREQ9:
		    cp = "TS_WACK_DREQ9";
		    break;
		case TS_WACK_DREQ10:
		    cp = "TS_WACK_DREQ10";
		    break;
		case TS_WACK_DREQ11:
		    cp = "TS_WACK_DREQ11";
		    break;
		default:
		    (void) snpf(buf, sizeof(buf), "UNKNOWN_TPI_STATE_%d", s);
		    cp = buf;
		}
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

# if	defined(SO_ACCEPTCONN)
		if (opt & SO_ACCEPTCONN) {
		    (void) printf("%cACCEPTCONN", sep);
		    opt &= ~SO_ACCEPTCONN;
		    sep = ',';
		}
# endif	/* defined(SO_ACCEPTCONN) */

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

# if	defined(SO_IMASOCKET)
		if (opt & SO_IMASOCKET) {
		    (void) printf("%cIMASOCKET", sep);
		    opt &= ~SO_IMASOCKET;
		    sep = ',';
		}
# endif	/* defined(SO_IMASOCKET) */

# if	defined(SO_KEEPALIVE)
		if (opt & SO_KEEPALIVE) {
		    (void) printf("%cKEEPALIVE", sep);
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

# if	defined(SO_LISTENING)
		if (opt & SO_LISTENING) {
		    (void) printf("%cLISTENING", sep);
		    opt &= ~SO_LISTENING;
		    sep = ',';
		}
# endif	/* defined(SO_LISTENING) */

# if	defined(SO_MGMT)
		if (opt & SO_MGMT) {
		    (void) printf("%cMGMT", sep);
		    opt &= ~SO_MGMT;
		    sep = ',';
		}
# endif	/* defined(SO_MGMT) */

# if	defined(SO_OOBINLINE)
		if (opt & SO_OOBINLINE) {
		    (void) printf("%cOOBINLINE", sep);
		    opt &= ~SO_OOBINLINE;
		    sep = ',';
		}
# endif	/* defined(SO_OOBINLINE) */

# if	defined(SO_ORDREL)
		if (opt & SO_ORDREL) {
		    (void) printf("%cORDREL", sep);
		    opt &= ~SO_ORDREL;
		    sep = ',';
		}
# endif	/* defined(SO_ORDREL) */

# if	defined(SO_PARALLELSVR)
		if (opt & SO_PARALLELSVR) {
		    (void) printf("%cPARALLELSVR", sep);
		    opt &= ~SO_PARALLELSVR;
		    sep = ',';
		}
# endif	/* defined(SO_PARALLELSVR) */

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

# if	defined(SO_RDWR)
		if (opt & SO_RDWR) {
		    (void) printf("%cRDWR", sep);
		    opt &= ~SO_RDWR;
		    sep = ',';
		}
# endif	/* defined(SO_RDWR) */

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

# if	defined(SO_SEMA)
		if (opt & SO_SEMA) {
		    (void) printf("%cSEMA", sep);
		    opt &= ~SO_SEMA;
		    sep = ',';
		}
# endif	/* defined(SO_SEMA) */

		if (Lf->lts.sbszs) {
		    (void) printf("%cSNDBUF=%lu", sep, Lf->lts.sbsz);
		    sep = ',';
		}

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

# if	defined(SS_ASYNC)
		if (ss & SS_ASYNC) {
		    (void) printf("%cASYNC", sep);
		    ss &= ~SS_ASYNC;
		    sep = ',';
		}
# endif	/* defined(SS_ASYNC) */

# if	defined(SS_CANTRCVMORE)
		if (ss & SS_CANTRCVMORE) {
		    (void) printf("%cCANTRCVMORE", sep);
		    ss &= ~SS_CANTRCVMORE;
		    sep = ',';
		}
# endif	/* defined(SS_CANTRCVMORE) */

# if	defined(SS_CANTSENDMORE)
		if (ss & SS_CANTSENDMORE) {
		    (void) printf("%cCANTSENDMORE", sep);
		    ss &= ~SS_CANTSENDMORE;
		    sep = ',';
		}
# endif	/* defined(SS_CANTSENDMORE) */

# if	defined(SS_IGNERR)
		if (ss & SS_IGNERR) {
		    (void) printf("%cIGNERR", sep);
		    ss &= ~SS_IGNERR;
		    sep = ',';
		}
# endif	/* defined(SS_IGNERR) */

# if	defined(SS_ISBOUND)
		if (ss & SS_ISBOUND) {
		    (void) printf("%cISBOUND", sep);
		    ss &= ~SS_ISBOUND;
		    sep = ',';
		}
# endif	/* defined(SS_ISBOUND) */

# if	defined(SS_ISCONNECTED)
		if (ss & SS_ISCONNECTED) {
		    (void) printf("%cISCONNECTED", sep);
		    ss &= ~SS_ISCONNECTED;
		    sep = ',';
		}
# endif	/* defined(SS_ISCONNECTED) */

# if	defined(SS_ISCONNECTING)
		if (ss & SS_ISCONNECTING) {
		    (void) printf("%cISCONNECTING", sep);
		    ss &= ~SS_ISCONNECTING;
		    sep = ',';
		}
# endif	/* defined(SS_ISCONNECTING) */

# if	defined(SS_ISDISCONNECTING)
		if (ss & SS_ISDISCONNECTING) {
		    (void) printf("%cISDISCONNECTING", sep);
		    ss &= ~SS_ISDISCONNECTING;
		    sep = ',';
		}
# endif	/* defined(SS_ISDISCONNECTING) */

# if	defined(SS_NBIO)
		if (ss & SS_NBIO) {
		    (void) printf("%cNBIO", sep);
		    ss &= ~SS_NBIO;
		    sep = ',';
		}
# endif	/* defined(SS_NBIO) */

# if	defined(SS_NODELETE)
		if (ss & SS_NODELETE) {
		    (void) printf("%cNODELETE", sep);
		    ss &= ~SS_NODELETE;
		    sep = ',';
		}
# endif	/* defined(SS_NODELETE) */

# if	defined(SS_NOGHOST)
		if (ss & SS_NOGHOST) {
		    (void) printf("%cNOGHOST", sep);
		    ss &= ~SS_NOGHOST;
		    sep = ',';
		}
# endif	/* defined(SS_NOGHOST) */

# if	defined(SS_NOINPUT)
		if (ss & SS_NOINPUT) {
		    (void) printf("%cNOINPUT", sep);
		    ss &= ~SS_NOINPUT;
		    sep = ',';
		}
# endif	/* defined(SS_NOINPUT) */

# if	defined(SS_NOFDREF)
		if (ss & SS_NOFDREF) {
		    (void) printf("%cNOFDREF", sep);
		    ss &= ~SS_NOFDREF;
		    sep = ',';
		}
# endif	/* defined(SS_NOFDREF) */

# if	defined(SS_PRIV)
		if (ss & SS_PRIV) {
		    (void) printf("%cPRIV", sep);
		    ss &= ~SS_PRIV;
		    sep = ',';
		}
# endif	/* defined(SS_PRIV) */

# if	defined(SS_RCVATMARK)
		if (ss & SS_RCVATMARK) {
		    (void) printf("%cRCVATMARK", sep);
		    ss &= ~SS_RCVATMARK;
		    sep = ',';
		}
# endif	/* defined(SS_RCVATMARK) */

# if	defined(SS_SETRCV)
		if (ss & SS_SETRCV) {
		    (void) printf("%cSETRCV", sep);
		    ss &= ~SS_SETRCV;
		    sep = ',';
		}
# endif	/* defined(SS_SETRCV) */

# if	defined(SS_SETSND)
		if (ss & SS_SETSND) {
		    (void) printf("%cSETSND", sep);
		    ss &= ~SS_SETSND;
		    sep = ',';
		}
# endif	/* defined(SS_SETSND) */

# if	defined(SS_ZOMBIE)
		if (ss & SS_ZOMBIE) {
		    (void) printf("%cZOMBIE", sep);
		    ss &= ~SS_ZOMBIE;
		    sep = ',';
		}
# endif	/* defined(SS_ZOMBIE) */

		if (ss)
		    (void) printf("%cUNKNOWN=%#x", sep, ss);
		if (Ffield)
		    putchar(Terminator);
	    }
	}
#endif	/* defined(HASSOSTATE) */

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

# if	defined(TF_HAVEACKED)
		if (topt & TF_HAVEACKED) {
		    (void) printf("%cHAVEACKED", sep);
		    topt &= ~TF_HAVEACKED;
		    sep = ',';
		}
# endif	/* defined(TF_HAVEACKED) */

# if	defined(TF_HAVECLOSED)
		if (topt & TF_HAVECLOSED) {
		    (void) printf("%cHAVECLOSED", sep);
		    topt &= ~TF_HAVECLOSED;
		    sep = ',';
		}
# endif	/* defined(TF_HAVECLOSED) */

# if	defined(TF_IOLOCK)
		if (topt & TF_IOLOCK) {
		    (void) printf("%cIOLOCK", sep);
		    topt &= ~TF_IOLOCK;
		    sep = ',';
		}
# endif	/* defined(TF_IOLOCK) */

		if (Lf->lts.msss) {
		    (void) printf("%cMSS=%lu", sep, Lf->lts.mss);
		    sep = ',';
		}

# if	defined(TF_MAXSEG_TSTMP)
		if (topt & TF_MAXSEG_TSTMP) {
		    (void) printf("%cMAXSEG_TSTMP", sep);
		    topt &= ~TF_MAXSEG_TSTMP;
		    sep = ',';
		}
# endif	/* defined(TF_MAXSEG_TSTMP) */

# if	defined(TF_NEEDCLOSE)
		if (topt & TF_NEEDCLOSE) {
		    (void) printf("%cNEEDCLOSE", sep);
		    topt &= ~TF_NEEDCLOSE;
		    sep = ',';
		}
# endif	/* defined(TF_NEEDCLOSE) */

# if	defined(TF_NEEDIN)
		if (topt & TF_NEEDIN) {
		    (void) printf("%cNEEDIN", sep);
		    topt &= ~TF_NEEDIN;
		    sep = ',';
		}
# endif	/* defined(TF_NEEDIN) */

# if	defined(TF_NEEDOUT)
		if (topt & TF_NEEDOUT) {
		    (void) printf("%cNEEDOUT", sep);
		    topt &= ~TF_NEEDOUT;
		    sep = ',';
		}
# endif	/* defined(TF_NEEDOUT) */

# if	defined(TF_NEEDTIMER)
		if (topt & TF_NEEDTIMER) {
		    (void) printf("%cNEEDTIMER", sep);
		    topt &= ~TF_NEEDTIMER;
		    sep = ',';
		}
# endif	/* defined(TF_NEEDTIMER) */

# if	defined(TF_NODELACK)
		if (topt & TF_NODELACK) {
		    (void) printf("%cNODELACK", sep);
		    topt &= ~TF_NODELACK;
		    sep = ',';
		}
# endif	/* defined(TF_NODELACK) */

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

# if	defined(TF_RCVD_SCALE)
		if (topt & TF_RCVD_SCALE) {
		    (void) printf("%cRCVD_SCALE", sep);
		    topt &= ~TF_RCVD_SCALE;
		    sep = ',';
		}
# endif	/* defined(TF_RCVD_SCALE) */

# if	defined(TF_RCVD_TSTMP)
		if (topt & TF_RCVD_TSTMP) {
		    (void) printf("%cRCVD_TSTMP", sep);
		    topt &= ~TF_RCVD_TSTMP;
		    sep = ',';
		}
# endif	/* defined(TF_RCVD_TSTMP) */

# if	defined(TF_REQ_SCALE)
		if (topt & TF_REQ_SCALE) {
		    (void) printf("%cREQ_SCALE", sep);
		    topt &= ~TF_REQ_SCALE;
		    sep = ',';
		}
# endif	/* defined(TF_REQ_SCALE) */

# if	defined(TF_REQ_TSTMP)
		if (topt & TF_REQ_TSTMP) {
		    (void) printf("%cREQ_TSTMP", sep);
		    topt &= ~TF_REQ_TSTMP;
		    sep = ',';
		}
# endif	/* defined(TF_REQ_TSTMP) */

# if	defined(TF_SACK_PERMIT)
		if (topt & TF_SACK_PERMIT) {
		    (void) printf("%cSACK_PERMIT", sep);
		    topt &= ~TF_SACK_PERMIT;
		    sep = ',';
		}
# endif	/* defined(TF_SACK_PERMIT) */

# if	defined(TF_SENTFIN)
		if (topt & TF_SENTFIN) {
		    (void) printf("%cSENTFIN", sep);
		    topt &= ~TF_SENTFIN;
		    sep = ',';
		}
# endif	/* defined(TF_SENTFIN) */

# if	defined(TF_USERCLOSE)
		if (topt & TF_USERCLOSE) {
		    (void) printf("%cUSERCLOSE", sep);
		    topt &= ~TF_USERCLOSE;
		    sep = ',';
		}
# endif	/* defined(TF_USERCLOSE) */

		if (topt)
		    (void) printf("%cUNKNOWN=%#x", sep, topt);
		if (Ffield)
		    putchar(Terminator);
	    }
	}
#endif	/* defined(HASTCPOPT) */

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

	if (!Ffield && ps)
	    putchar(')');
	if (nl)
	    putchar('\n');
}


/*
 * process_socket() - process socket
 */

void
process_socket(pr, q)
	char *pr;			/* protocol name */
	struct queue *q;		/* queue at end of stream */
{
	unsigned char *fa = (unsigned char *)NULL;
	int fp, ipv, lp;
	struct inpcb inp;
	unsigned char *la = (unsigned char *)NULL;
	struct tcpcb t;
	int tcp = 0;
	short ts = 0;
	int udp = 0;

/*
 * Process protocol specification.
 */
	Lf->inp_ty = 2;
	(void) snpf(Lf->iproto, sizeof(Lf->iproto), "%s", pr);
	Lf->is_stream = 0;
	if (strcasecmp(pr, "TCP") == 0) {
	    ipv = 4;
	    tcp = 1;
	} else if (strcasecmp(pr, "UDP") == 0) {
	    ipv = 4;
	    udp = 1;
	}

#if	defined(HASIPv6)
	else if (strcasecmp(pr, "TCP6") == 0) {
	    ipv = 6;
	    tcp = 1;
	    Lf->iproto[3] = '\0';
	} else if (strcasecmp(pr, "UDP6") == 0) {
	    ipv = 6;
	    udp = 1;
	    Lf->iproto[3] = '\0';
	}
#endif	/* defined(HASIPv6) */

	if (Fnet && (tcp || udp)) {
	    if (!FnetTy || (FnetTy == ipv))
		Lf->sf |= SELNET;
	}

#if	defined(HASIPv6)
        (void) snpf(Lf->type, sizeof(Lf->type), (ipv == 6) ? "IPv6" : "IPv4");
#else	/* !defined(HASIPv6) */
        (void) snpf(Lf->type, sizeof(Lf->type), "inet");
#endif	/* defined(HASIPv6) */

/*
 * The PCB address is found in the private data structure at the end
 * of the queue.
 */
	if (q->q_ptr) {
	    enter_dev_ch(print_kptr((KA_T)q->q_ptr, (char *)NULL, 0));
	    if (tcp || udp) {
		if (kread((KA_T)q->q_ptr, (char *)&inp, sizeof(inp))) {
		    (void) snpf(Namech, Namechl, "can't read inpcb from %s",
			print_kptr((KA_T)q->q_ptr, (char *)NULL, 0));
		    enter_nm(Namech);
		    return;
		}
		la = (unsigned char *)&inp.inp_laddr;
		lp = (int)ntohs(inp.inp_lport);
		if (inp.inp_faddr.s_addr != INADDR_ANY || inp.inp_fport != 0) {
		    fa = (unsigned char *)&inp.inp_faddr;
		    fp = (int)ntohs(inp.inp_fport);
		}
		if (fa || la)
		    (void) ent_inaddr(la, lp, fa, fp, AF_INET);
		if (tcp) {
		    if (inp.inp_ppcb
		    &&  !kread((KA_T)inp.inp_ppcb, (char *)&t, sizeof(t))) {
			ts = 1;
			Lf->lts.type = 0;
			Lf->lts.state.i = (int)t.t_state;
		    }
		} else {
		    Lf->lts.type = 1;
		    Lf->lts.state.i = (int)inp.inp_tstate;
		} 
	    } else
		enter_nm("no address for this protocol");
	} else
	    enter_nm("no address");
/*
 * Save size information.
 */
	if (ts) {
	    if (Fsize) {

#if	UNIXWAREV>=70000
#define	t_outqsize	t_qsize
#endif	/* UNIXWAREV>=70000 */

		if (Lf->access == 'r')
		    Lf->sz = (SZOFFTYPE)t.t_iqsize;
		else if (Lf->access == 'w')
		    Lf->sz = (SZOFFTYPE)t.t_outqsize;
		else
		    Lf->sz = (SZOFFTYPE)(t.t_iqsize + t.t_outqsize);
		Lf->sz_def = 1;

	    } else
		Lf->off_def = 1;

#if	defined(HASTCPTPIQ)
		Lf->lts.rq = (unsigned long)t.t_iqsize;
		Lf->lts.sq = (unsigned long)t.t_outqsize;
		Lf->lts.rqs = Lf->lts.sqs = 1;
#endif	/* defined(HASTCPTPIQ) */

#if	defined(HASSOOPT)
	    Lf->lts.opt = (unsigned int)inp.inp_protoopt;
	    Lf->lts.ltm = (unsigned int)inp.inp_linger;
	    Lf->lts.pqlen = (unsigned int)t.t_q0len;
	    Lf->lts.qlen = (unsigned int)t.t_qlen;
	    Lf->lts.qlim = (unsigned int)t.t_qlimit;
	    Lf->lts.rbsz = (unsigned long)inp.inp_rbufsize;
	    Lf->lts.sbsz = (unsigned long)inp.inp_sbufsize;
	    Lf->lts.pqlens = Lf->lts.qlens = Lf->lts.qlims = Lf->lts.rbszs
			   = Lf->lts.sbszs = (unsigned char)1;
#endif	/* defined(HASSOOPT) */

#if	defined(HASSOSTATE)
	    Lf->lts.ss = (unsigned int)inp.inp_state;
#endif	/* defined(HASSOSTATE) */

#if	defined(HASTCPOPT)
	    Lf->lts.mss = (unsigned long)t.t_maxseg;
	    Lf->lts.msss = (unsigned char)1;
	    Lf->lts.topt = (unsigned int)t.t_flags;
#endif	/* defined(HASTCPOPT) */

	}
	else if (Fsize) {
	    Lf->sz = (SZOFFTYPE)q->q_count;
	    Lf->sz_def = 1;
	} else
	    Lf->off_def = 1;
	enter_nm(Namech);
	return;
}


#if	UNIXWAREV>=70101
/*
 * process_unix_sockstr() - process a UNIX socket stream, if applicable
 */

int
process_unix_sockstr(v, na)
	struct vnode *v;		/* the stream's vnode */
	KA_T na;			/* kernel vnode address */
{
	int as;
	char *ep, tbuf[32], tbuf1[32], *ty;
	KA_T ka, sa, sh;
	struct stdata sd;
	struct ss_socket ss;
	size_t sz;

# if	UNIXWAREV<70103
	struct sockaddr_un *la, *ra;
# else	/* UNIXWAREV>=70103 */
	struct sockaddr_un la, ra;
	unsigned char las = 0;
	unsigned char ras = 0;
	int up = (int)(sizeof(la.sun_path) - 1);
/*
 * It's serious if the sizeof(sun_path) in sockaddr_un isn't greater than zero.
 */
	if (up < 0) {
	    (void) snpf(Namech, Namechl, "sizeof(sun_path) < 1 (%d)", up);
	    enter_nm(Namech);
	    return(1);
	}
# endif	/* UNIXWAREV<70103 */

/*
 * Read the stream head, if possible.
 */
	if (!(sh = (KA_T)v->v_stream))
	    return(0);
	if (readstdata(sh, &sd)) {
	    (void) snpf(Namech, Namechl,
		"vnode at %s; can't read stream head at %s",
		print_kptr(na, (char *)NULL, 0),
		print_kptr(sh, tbuf, sizeof(tbuf)));
	    enter_nm(Namech);
	    return(1);
	}
/*
 * If the stream head has pointer to a socket, read the socket structure
 */
	if (!(sa = (KA_T)sd.sd_socket))
	    return(0);
	if (kread(sa, (char *)&ss, sizeof(ss))) {
	    (void) snpf(Namech, Namechl,
		"vnode at %s; stream head at %s; can't read socket at %s",
		print_kptr(na, (char *)NULL, 0),
		print_kptr(sh, tbuf, sizeof(tbuf)),
		print_kptr(sa, tbuf1, sizeof(tbuf1)));
	    enter_nm(Namech);
	    return(1);
	}
/*
 * If the socket is bound to the PF_UNIX protocol family, process it as
 * a UNIX socket.  Otherwise, return and let the vnode be processed as a
 * stream.
 */
	if (ss.family != PF_UNIX)
	    return(0);
	(void) snpf(Lf->type, sizeof(Lf->type), "unix");
	if (Funix)
	    Lf->sf |= SELUNX;
	Lf->is_stream = 0;
	if (!Fsize)
	    Lf->off_def = 1;
	enter_dev_ch(print_kptr(sa, (char *)NULL, 0));
/*
 * Process the local address.
 */

# if	UNIXWAREV<70103
	if ((la = find_unix_sockaddr_un((KA_T)sd.sd_socket))) {
	    if (Sfile && is_file_named(la->sun_path, 0))
		Lf->sf = SELNM;
	}
# else	/* UNIXWAREV>=70103 */
	if (((as = (KA_T)ss.local_addrsz) > 0) && (ka = (KA_T)ss.local_addr))
	{
	    if (as > sizeof(la))
		as = (int)sizeof(la);
	    if (!kread(ka, (char *)&la, as)) {
		la.sun_path[up] = '\0';
		if (la.sun_path[0]) {
		    las = 1;
		    if (Sfile && is_file_named(la.sun_path, 0))
			Lf->sf = SELNM;
		}
	    }
	}
# endif	/* UNIXWAREV<70103 */

/*
 * Process the remote address.
 */

# if	UNIXWAREV<70103
	if ((ra = find_unix_sockaddr_un((KA_T)ss.conn_ux))) {
	    if (Sfile && is_file_named(ra->sun_path, 0))
		Lf->sf = SELNM;
	}
# else	/* UNIXWAREV>=70103 */
	if (((as = (KA_T)ss.remote_addrsz) > 0) && (ka = (KA_T)ss.remote_addr))
	{
	    if (as > sizeof(la))
		as = (int)sizeof(ra);
	    if (!kread(ka, (char *)&ra, as)) {
		ra.sun_path[up] = '\0';
		if (ra.sun_path[0]) {
		    ras = 1;
		    if (Sfile && is_file_named(ra.sun_path, 0))
			Lf->sf = SELNM;
		}
	    }
	}
# endif	/* UNIXWAREV<70103 */

/*
 * Start Namech[] with the service type, converted to a name, ala netstat.
 */
	switch (ss.servtype) {
	case T_COTS:
	case T_COTS_ORD:
	    ty = "stream";
	    break;
	case T_CLTS:
	    ty = "dgram";
	    break;
	default:
	    ty = (char *)NULL;
	}
	if (ty) {
	    (void) snpf(Namech, Namechl, "%s", ty);
	    ty = ":";
	} else {
	    Namech[0] = '\0';
	    ty = "";
	}
/*
 * Add names to Namech[].
 */

#if	UNIXWAREV<70103
	if (la && la->sun_path[0]) {
	    ep = endnm(&sz);
	    (void) snpf(ep, sz, "%s%s", ty, la->sun_path);
	}
#else	/* UNIXWAREV>=70103 */
	if (las) {
	    ep = endnm(&sz);
	    (void) snpf(ep, sz, "%s%s", ty, la.sun_path);
	}
#endif	/* UNIXWAREV<70103 */

	ep = endnm(&sz);

#if	UNIXWAREV<70103
	if (ra && ra->sun_path[0])
	    (void) snpf(ep, sz, "->%s", ra->sun_path);
#else	/* UNIXWAREV>=70103 */
	if (ras)
	    (void) snpf(ep, sz, "->%s", ra.sun_path);
#endif	/* UNIXWAREV<70103 */

	else if ((ka = (KA_T)ss.conn_ux))
	    (void) snpf(ep, sz, "->%s", print_kptr(ka, (char *)NULL, 0));
	if (Namech[0])
	    enter_nm(Namech);
	return(1);
}


# if	UNIXWAREV<70103
/*
 * find_unix_sockaddr_un() -- find UNIX socket address structure
 *
 */

static struct sockaddr_un *
find_unix_sockaddr_un(ka)
	KA_T ka;			/* socket's kernel address */
{
	static struct soreq *al = (struct soreq *)NULL;
	static int alct = 0;
	int i;

	if (!al) {
	    MALLOC_S alen, len;
	    char *ch = (char *)NULL;
	    int ct, pct;
	    struct strioctl ioc;
	    int sock = -1;

	    if (alct < 0)
		return((struct sockaddr_un *)NULL);
	/*
	 * If there has been no attempt to acquire the  address list yet,
	 * do so.
	 *
	 * Get a SOCK_STREAM PF_UNIX socket descriptor and use ioctl() to
	 * send a stream message to acquire the list of PF_UNIX addresses.
	 */
	    if ((sock = socket (PF_UNIX, SOCK_STREAM, 0)) < 0) {

	    /*
	     * Some error was detected.  Return allocated resources and
	     * indicate that no further attempts need be made.
	     */

find_err_exit:

		alct = -1;
		if (sock >= 0)
		    close(sock);
		if (ch)
		    (void) free((FREE_P *)ch);
		return((struct sockaddr_un *)NULL);
	    }
	/*
	 * Read the address list.  Before starting, get an estimate of its
	 * size and add a small safety margin.
	 */
	    if ((ct = ioctl(sock, SI_UX_COUNT, 0)) < 0)
		goto find_err_exit;
	    ct += 32;
	    pct = 0;
	    do {
		if (ct > pct) {

		/*
		 * If the previously allocated space is insufficient,
		 * or if none has been allocated, allocate space.
		 */
		    alen = (MALLOC_S)(ct * sizeof(struct soreq));
		    if (ch)
			ch = (char *)realloc((MALLOC_P *)ch, alen);
		    else
			ch = (char *)malloc(alen);
		    if (!ch)
			goto find_err_exit;
		    pct = ct;
		}
	   /*
	    * Read the address list into the allocated space.
	    */
		ioc.ic_cmd = SI_UX_LIST;
		ioc.ic_dp = ch;
		ioc.ic_len = (int)alen;
		ioc.ic_timout = 0;
		if ((ct = ioctl(sock, I_STR, &ioc)) < 0)
		    goto find_err_exit;
	    } while (ct > pct);
	/*
	 * The list has been acquired.  Free any excess space pre-allocated to
	 * it, then save its address.   Close the stream socket.
	 */
	    alct = ct;
	    if ((len = (MALLOC_S)(alct * sizeof(struct soreq))) < alen) {
		if (!(ch = (char *)realloc((MALLOC_P *)ch, len)))
		    goto find_err_exit;
	    }
	    al = (struct soreq *)ch;
	    close(sock);
	}
/*
 * Search a previously acquired address list, based on the supplied kernel
 * socket address.  If an entry is found, return a pointer to it, making
 * sure the path it contains is terminated.
 */
	for (i = 0; i < alct; i++) {
	    if ((KA_T)al[i].so_addr == ka)
		break;
	}
	if (i >= alct || !al[i].sockaddr.sun_path[0])
	    return((struct sockaddr_un *)NULL);
	al[i].sockaddr.sun_path[(int)(sizeof(al[i].sockaddr.sun_path) - 1)]
	    = '\0';
	return(&al[i].sockaddr);
}
# endif	/* UNIXWAREV<70103 */
#endif	/* UNIXWAREV>=70101 */
