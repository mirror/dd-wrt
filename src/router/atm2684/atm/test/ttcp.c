/*
 *	T T C P . C
 *
 * Test TCP connection.  Makes a connection on port 5013
 * and transfers fabricated buffers or data copied from stdin.
 *
 * Usable on 4.2, 4.3, and 4.1a systems by defining one of
 * BSD42 BSD43 (BSD41a)
 * Machines using System V with BSD sockets should define SYSV.
 *
 * Modified for operation under 4.2BSD, 18 Dec 84
 *      T.C. Slattery, USNA
 * Minor improvements, Mike Muuss and Terry Slattery, 16-Oct-85.
 * Modified in 1989 at Silicon Graphics, Inc.
 *	catch SIGPIPE to be able to print stats when receiver has died 
 *	for tcp, don't look for sentinel during reads to allow small transfers
 *	increased default buffer size to 8K, nbuf to 2K to transfer 16MB
 *	moved default port to 5013, beyond IPPORT_USERRESERVED
 *	make sinkmode default because it is more popular, 
 *		-s now means don't sink/source 
 *	count number of read/write system calls to see effects of 
 *		blocking from full socket buffers
 *	for tcp, -D option turns off buffered writes (sets TCP_NODELAY sockopt)
 *	buffer alignment options, -A and -O
 *	print stats in a format that's a bit easier to use with grep & awk
 *	for SYSV, mimic BSD routines to use most of the existing timing code
 * Modified by Steve Miller of the University of Maryland, College Park
 *	-b sets the socket buffer size (SO_SNDBUF/SO_RCVBUF)
 * Modified Sept. 1989 at Silicon Graphics, Inc.
 *	restored -s sense at request of tcs@brl
 * Modified Oct. 1991 at Silicon Graphics, Inc.
 *	use getopt(3) for option processing, add -f and -T options.
 *	SGI IRIX 3.3 and 4.0 releases don't need #define SYSV.
 * Modified April 1994 by John Lin (lin@cs.purdue.edu)
 *      Add CPU_USAGE complier option and move the "gettimeofday" calls.
 * Heavily modified since 1995 by Werner Almesberger, EPFL ICA
 *      Various ATM-related changes
 *
 * Distribution Status -
 *      Public Domain.  Distribution Unlimited.
 */
#ifndef lint
static char RCSid[] = "ttcp.c $Revision: 1.12 $";
#endif

/*#define CPU_USAGE*/	/* print out CPU usage numbers? */
#define BSD43
/* #define BSD42 */
/* #define BSD41a */
/* #define SYSV */	/* required on SGI IRIX releases before 3.3 */

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>		/* struct timeval */
#include <atm.h>
#include <atmsap.h>

#if defined(SYSV)
#include <sys/times.h>
#include <sys/param.h>
struct rusage {
    struct timeval ru_utime, ru_stime;
};
#define RUSAGE_SELF 0
#else
#include <sys/resource.h>
#endif

struct sockaddr_in sinme;
struct sockaddr_in sinhim;
struct sockaddr_in frominet;
struct sockaddr_atmsvc satm;
struct atm_qos qos;

int domain, fromlen;
int fd;				/* fd of network socket */

int buflen = 8 * 1024;		/* length of buffer */
char *buf;			/* ptr to dynamic buffer */
int nbuf = 2 * 1024;		/* number of buffers to send in sinkmode */

int bufoffset = 0;		/* align buffer to this */
int bufalign = 16*1024;		/* modulo this */

int atm = 0;			/* 0 = INET, !0 = ATM */
int pcr = 0;
char *qos_spec = NULL;

int udp = 0;			/* 0 = tcp, !0 = udp */
int options = 0;		/* socket options */
int one = 1;                    /* for 4.3 BSD style setsockopt() */
unsigned short port = 5013;	/* TCP port number */
char *host;			/* ptr to name of host */
int trans;			/* 0=receive, !0=transmit mode */
int sinkmode = 0;		/* 0=normal I/O, !0=sink/source mode */
int verbose = 0;		/* 0=print basic info, 1=print cpu rate, proc
				 * resource usage. */
int nodelay = 0;		/* set TCP_NODELAY socket option */
int b_flag = 0;			/* use mread() */
int sockbufsize = 0;		/* socket buffer size to use */
char fmt = 'K';			/* output format: k = kilobits, K = kilobytes,
				 *  m = megabits, M = megabytes, 
				 *  g = gigabits, G = gigabytes */
int touchdata = 0;		/* access data after reading */
static struct timeval start_time;	/* Time at which timing started */
static struct timeval stop_time;	/* Time at which timing stopped */
static struct rusage ru0;		/* Resource utilization at the start */

struct hostent *addr;
extern int errno;
extern int optind;
extern char *optarg;

char Usage[] = "\
Usage: ttcp -t [-options] host [ < in ]\n\
       ttcp -r [-options > out]\n\
Common options:\n\
	-l ##	length of bufs read from or written to network (default 8192)\n\
	-u	use UDP instead of TCP\n\
	-p ##	port number to send to or listen at (default 5001)\n\
	-s	-t: source a pattern to network\n\
		-r: sink (discard) all data from network\n\
	-A	align the start of buffers to this modulus (default 16384)\n\
	-O	start buffers at this offset from the modulus (default 0)\n\
	-v	verbose: print more statistics\n\
	-d	set SO_DEBUG socket option\n\
	-b ##	set socket buffer size (if supported)\n\
	-f X	format for rate: k,K = kilo{bit,byte}; m,M = mega; g,G = giga\n\
	-a	use native ATM instead of UDP/TCP\n\
Options specific to -t:\n\
	-n##	number of source bufs written to network (default 2048)\n\
	-D	don't buffer TCP writes (sets TCP_NODELAY socket option)\n\
	-C	disable (UDP) checksums\n\
	-P X	use the specified QOS for the ATM connection. If X is only\n\
		a number, the following QOS spec is assumed: cbr:pcr=X\n\
	-S X	use the specified value for the TOS byte. Without -S, the\n\
		system default is used.\n\
Options specific to -r:\n\
	-B	for -s, only output full blocks as specified by -l (for TAR)\n\
	-T	\"touch\": access each byte as it's read\n\
";	

char stats[128];
double nbytes;			/* bytes on net */
unsigned long numCalls;		/* # of I/O system calls */
double cput, realt;		/* user, real time (seconds) */

void err();
void mes();
int pattern();
void prep_timer();
double read_timer();
int Nread();
int Nwrite();
void delay();
int mread();
char *outfmt();
static void prusage();
static void tvadd();
static void tvsub();
static void psecs();

void
sigpipe()
{
}


/*-------------------------------------------------------------------------
 * main - 
 *-------------------------------------------------------------------------
 */
main(argc,argv)
int argc;
char **argv;
{
    struct timeval td;
    unsigned long addr_tmp;
    const char *port_name = NULL,*tos = NULL;
    int c;
    double mbps;
int no_check = 0;

    if (argc < 2) goto usage;

    while ((c = getopt(argc, argv, "adrstuvBDTb:f:l:n:p:P:S:A:O:C")) != -1) {
	switch (c) {

	  case 'a':
	    atm = udp = 1;
	    break;
	  case 'B':
	    b_flag = 1;
	    break;
	  case 't':
	    trans = 1;
	    break;
	  case 'r':
	    trans = 0;
	    break;
	  case 'C':
		no_check = 1;
		break;
	  case 'd':
	    options |= SO_DEBUG;
	    break;
	  case 'D':
#ifdef TCP_NODELAY
	    nodelay = 1;
#else
	    fprintf(stderr, 
		    "ttcp: -D option ignored: TCP_NODELAY socket option not supported\n");
#endif
	    break;
	  case 'n':
	    nbuf = atoi(optarg);
	    break;
	  case 'l':
	    buflen = atoi(optarg);
	    break;
	  case 's':
	    sinkmode = !sinkmode;
	    break;
	  case 'p':
	    port_name = optarg;
	    break;
	  case 'P':
	    qos_spec = optarg;
	    break;
	  case 'S':
	    tos = optarg;
	    break;
	  case 'u':
	    udp = 1;
	    break;
	  case 'v':
	    verbose = 1;
	    break;
	  case 'A':
	    bufalign = atoi(optarg);
	    break;
	  case 'O':
	    bufoffset = atoi(optarg);
	    break;
	  case 'b':
#if defined(SO_SNDBUF) || defined(SO_RCVBUF)
	    sockbufsize = atoi(optarg);
#else
	    fprintf(stderr, 
		    "ttcp: -b option ignored: SO_SNDBUF/SO_RCVBUF socket options not supported\n");
#endif
	    break;
	  case 'f':
	    fmt = *optarg;
	    break;
	  case 'T':
	    touchdata = 1;
	    break;

	  default:
	    goto usage;
	}
    }

    if (port_name)
	if (atm) goto usage;
	else {
	    struct servent *se;

	    se = getservbyname(port_name,udp ? "udp" : "tcp");
	    if (se) port = ntohs(se->s_port);
	    else {
		const char *end;

		port = strtoul(port_name,&end,0);
		if (*end) goto usage;
	    }
	}     

    host = argv[optind];

    if (atm) {
	char *end;

	memset(&satm,0,sizeof(satm));
	if (!host) satm.sas_family = AF_ATMSVC;
	else if (text2atm(host,(struct sockaddr *) &satm,
	      sizeof(satm),T2A_PVC | T2A_SVC | T2A_NAME) < 0) {
		fprintf(stderr,"invalid ATM address (PVC or SVC expected)\n");
		exit(1);
	    }
	memset(&qos,0,sizeof(qos));
	qos.rxtp.max_sdu = qos.txtp.max_sdu = buflen;
	if (qos_spec && ((pcr = strtol(qos_spec,&end,10)), *end)) {
	    if (text2qos(qos_spec,&qos,T2Q_DEFAULTS) < 0) {
		fprintf(stderr,"invalid QOS specification\n");
		exit(1);
	    }
	}
	else {
	    if (!qos_spec) pcr = 0;
	    if (!trans) qos.rxtp.traffic_class = ATM_UBR;
	    else if (!pcr) qos.txtp.traffic_class = ATM_UBR;
		else {
		    qos.txtp.traffic_class = ATM_CBR;
		    qos.txtp.max_pcr = pcr;
		}
	}
	if (!qos.aal) qos.aal = ATM_AAL5;
    }
    if (!atm && trans)  {
	/* xmitr */
	if (optind == argc)
	    goto usage;
	bzero((char *)&sinhim, sizeof(sinhim));
	if (atoi(host) > 0 )  {
	    /* Numeric */
	    sinhim.sin_family = AF_INET;
#if defined(cray)
	    addr_tmp = inet_addr(host);
	    sinhim.sin_addr = addr_tmp;
#else
	    sinhim.sin_addr.s_addr = inet_addr(host);
#endif
	} else {
	    if ((addr=gethostbyname(host)) == NULL)
		err("bad hostname");
	    sinhim.sin_family = addr->h_addrtype;
	    bcopy(addr->h_addr,(char*)&addr_tmp, addr->h_length);
#if defined(cray)
	    sinhim.sin_addr = addr_tmp;
#else
	    sinhim.sin_addr.s_addr = addr_tmp;
#endif /* cray */
	}
	sinhim.sin_port = htons(port);
	sinme.sin_port = 0;		/* free choice */
    } else {
	/* rcvr */
	sinme.sin_port =  htons(port);
    }


    if (udp && buflen < 5) {
	buflen = 5;			/* send more than the sentinel size */
    }

    if ((buf = (char *)malloc(buflen+bufalign)) == (char *)NULL)
	err("malloc");
    
    if (bufalign != 0)
	buf +=(bufalign - ((int)buf % bufalign) + bufoffset) % bufalign;

    if (trans) {
	fprintf(stdout,
		"ttcp-t: buflen=%d, nbuf=%d, align=%d/%d, port=%d",
		buflen, nbuf, bufalign, bufoffset, port);
	
	if (sockbufsize)
	    fprintf(stdout, ", sockbufsize=%d", sockbufsize);
	
	fprintf(stdout, "  %s  -> %s\n", atm?"atm":udp?"udp":"tcp", host);
    } else {
	fprintf(stdout,
		"ttcp-r: buflen=%d, nbuf=%d, align=%d/%d, port=%d",
 		buflen, nbuf, bufalign, bufoffset, port);
	
	if (sockbufsize)
	    fprintf(stdout, ", sockbufsize=%d", sockbufsize);
	
	fprintf(stdout, "  %s\n", atm?"atm":udp?"udp":"tcp");
    }

    if ((fd = socket(atm ? satm.sas_family : AF_INET,
        udp?SOCK_DGRAM:SOCK_STREAM,0)) < 0) err("socket");
    
    mes("socket");
    if (atm) {
	if (setsockopt(fd,SOL_ATM,SO_ATMQOS,&qos,sizeof(qos)) < 0)
	    err("setsockopt SO_ATMQOS");
    }
    else if (setsockopt(fd, SOL_SOCKET, SO_NO_CHECK, &no_check, sizeof(long)) <
	    0) err("setsockopt: no_check");
#ifdef SO_ATMSAP
    if (atm && satm.sas_family == AF_ATMSVC) {
	struct atm_sap sap;

	memset(&sap,0,sizeof(sap));
	sap.bhli.hl_type = ATM_HL_VENDOR;
	sap.bhli.hl_length = 7;
	memcpy(sap.bhli.hl_info,TTCP_HLT_VS_ID,7);
	if (setsockopt(fd,SOL_ATM,SO_ATMSAP,&sap,sizeof(sap)) < 0)
	    err("setsockopt SO_ATMSAP");
    }
#endif
    /* set socket buffer size */
#if defined(SO_SNDBUF) || defined(SO_RCVBUF)
    if (sockbufsize) {
	int len;

	if (trans) {
	    /* set send socket buffer if we are transmitting */    
	    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &sockbufsize,
			   sizeof sockbufsize) < 0)
		err("setsockopt: sndbuf");
	    len = sizeof sockbufsize;
	    if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &sockbufsize, &len) < 0)
		perror("getsockopt: sndbuf");
	    mes("sndbuf");
	} else {
	    /* set receive socket buffer if we are receiving */    
	    if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &sockbufsize,
			   sizeof sockbufsize) < 0)
		err("setsockopt: rcvbuf");
	    len = sizeof sockbufsize;
	    if (getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &sockbufsize, &len) < 0)
		perror("getsockopt: rcvbuf");
	    mes("rcvbuf");
	}
	printf("real buffer size = %d\n",sockbufsize);
    }
#endif

    if (!atm || satm.sas_family == AF_ATMPVC || !trans)
	if (bind(fd, atm ? &satm : &sinme, atm ? satm.sas_family == AF_ATMPVC ?
	  sizeof(struct sockaddr_atmpvc) : sizeof(struct sockaddr_atmsvc) :
	  sizeof(sinme)) < 0)
	    err("bind");

    if (!udp || (atm && satm.sas_family == AF_ATMSVC)) {
	signal(SIGPIPE, sigpipe);
	if (trans) {
	    /* We are the client if transmitting */
	    if (options)  {
#if defined(BSD42)
		if(setsockopt(fd, SOL_SOCKET, options, 0, 0) < 0)
#else  /* BSD43 */
		    if(setsockopt(fd, SOL_SOCKET, options, &one, sizeof(one)) < 0)
#endif
			err("setsockopt");
	    }
	    
	    if (connect(fd, atm ? &satm : &sinhim, atm ? sizeof(satm) :
	      sizeof(sinhim)) < 0)
		err("connect");
	    
	    mes("connect");
#ifdef TCP_NODELAY
	    if (nodelay) {
		struct protoent *p;
		p = getprotobyname("tcp");
		if( p && setsockopt(fd, p->p_proto, TCP_NODELAY, 
				    &one, sizeof(one)) < 0)
		    err("setsockopt: nodelay");
		mes("nodelay");
	    }
	    if (atm) sleep(1); /* grr ... */
#endif
	} else {
	    /* otherwise, we are the server and 
	     * should listen for the connections
	     */

#if defined(ultrix) || defined(sgi) || 1
	    listen(fd,1);		/* workaround for alleged u4.2 bug */
#else
	    listen(fd,0);		/* allow a queue of 0 */
#endif
	    fromlen = sizeof(frominet);
	    domain = AF_INET;
	    
	    if ((fd=accept(fd, &frominet, &fromlen) ) < 0)
		err("accept");
	    
	    {
		struct sockaddr_atmsvc peer;
		int peerlen = sizeof(peer);
		if (getpeername(fd, (struct sockaddr_in *) &peer, 
				&peerlen) < 0) {
		    err("getpeername");
		}
		if (atm) {
		    char name[MAX_ATM_ADDR_LEN+1];

		    if (atm2text(name,MAX_ATM_ADDR_LEN+1,(struct sockaddr *)
		      &peer,A2T_NAME | A2T_PRETTY) < 0)
			strcpy(name,"<invalid>");
		    fprintf(stderr,"ttcp-r: accept from %s\n",name);
		}
		else fprintf(stderr,"ttcp-r: accept from %s\n",
			    inet_ntoa(((struct sockaddr_in *) &peer)->
			      sin_addr));
	    }
	    
	    if (options)  {
#if defined(BSD42)
		if (setsockopt(fd, SOL_SOCKET, options, 0, 0) < 0)
#else  /* BSD43 */
		    if (setsockopt(fd, SOL_SOCKET, options, &one, sizeof(one)) < 0)
#endif
			err("setsockopt");
	    }
	}
    }
    if (tos) {
	unsigned char tos_value;

	tos_value = strtoul(tos,NULL,0);
	if (setsockopt(fd,SOL_IP,IP_TOS,&tos_value,1) < 0)
	    err("setsockopt IP_TOS");
    }

    errno = 0;
    if (sinkmode) {      
	register int cnt;
	
	if (trans)  {
	    pattern(buf, buflen);	/* construct a data pattern */
	    if (udp)
		(void)Nwrite(fd, buf, 4); /* rcvr start */

#ifdef CPU_USAGE
	    prep_timer();		/* start timer */
#else
	    gettimeofday(&start_time, (struct timezone *)0);
#endif
	    while (nbuf-- && Nwrite(fd, buf, buflen) == buflen)
		nbytes += buflen;
	    
	    if (udp)
		(void)Nwrite(fd, buf, 4); /* rcvr end */
	}
	else { /* receive mode */
	    if (udp) {
		while ((cnt = Nread(fd, buf, buflen)) > 0)  {
		    static int going = 0;
		    if(cnt <= 4)  {
			if (going)
			    break;	/* "EOF" */
			going = 1;
#ifdef CPU_USAGE
			prep_timer();	/* start timer */
#else
			gettimeofday(&start_time, (struct timezone *)0);
#endif			
			prep_timer();
		    } else {
			nbytes += cnt;
		    }
		}
	    }
	    else { /* TCP */
#ifdef CPU_USAGE
		prep_timer();		/* start timer */
#else
		gettimeofday(&start_time, (struct timezone *)0);
#endif
		while ((cnt=Nread(fd,buf,buflen)) > 0)  {
		    nbytes += cnt;
		}
	    }
	}
    } else { /* not sink mode */
	register int cnt;
#ifdef CPU_USAGE
	prep_timer();			/* start timer */
#else
	gettimeofday(&start_time, (struct timezone *)0);
#endif
	if (trans)  {
	    while((cnt = read(0, buf, buflen)) > 0 && Nwrite(fd,buf,cnt) == cnt)
		nbytes += cnt;
	}
	else  {
	    while((cnt = Nread(fd,buf,buflen)) > 0 && write(1,buf,cnt) == cnt)
		nbytes += cnt;
	}
    }
    
    if (errno)
	err("IO");

    /* if TCP close the connection to make sure all the data sent */
    if (!udp)
	close(fd);

    gettimeofday(&stop_time, (struct timezone *)0);

    /* Get real time */
    tvsub(&td, &stop_time, &start_time);
    realt = (double)td.tv_sec + ((double)td.tv_usec / (double)1000000.0);

#ifdef CPU_USAGE
    (void) read_timer(stats, sizeof(stats)); /* stop timer */
#endif
    
    if (udp && trans)  {
	(void)Nwrite(fd, buf, 4);	/* signal rcvr end */
	(void)Nwrite(fd, buf, 4);	/* rcvr end */
	(void)Nwrite(fd, buf, 4);	/* rcvr end */
	(void)Nwrite(fd, buf, 4);	/* rcvr end */
    }
    
    if (cput <= 0.0)
	cput = 0.001;
    
    if (realt <= 0.0)
	realt = 0.001;

    mbps = (double)(nbytes * 8) / realt / 1000000.0;
    fprintf(stdout,
	    "ttcp%s: %.0f bytes in %f real seconds = %s/sec (%f Mb/sec)\n",
	    trans?"-t":"-r",
	    nbytes, realt, outfmt((double)nbytes/realt), mbps);
	    

#ifdef CPU_USAGE
    fprintf(stdout,"ttcp%s: %s\n", trans?"-t":"-r", stats);
#endif
    
    if (verbose) {
	fprintf(stdout,
		"ttcp%s: %.0f bytes in %.2f CPU seconds = %s/cpu sec\n",
		trans?"-t":"-r",
		nbytes, cput, outfmt((double)nbytes/cput));

	fprintf(stdout,
		"ttcp%s: %d I/O calls, msec/call = %.2f, calls/sec = %.2f\n",
		trans?"-t":"-r",
		numCalls,
		1000.0 * realt/((double)numCalls),
		((double)numCalls)/realt);
	fprintf(stdout,
		"ttcp%s: buffer address %#x\n",
		trans?"-t":"-r",
		buf);
    }
    exit(0);

  usage:
    fprintf(stderr, Usage);
    exit(1);
}

void
err(s)
char *s;
{
	int	en = errno;

	fprintf(stderr,"ttcp%s: ", trans?"-t":"-r");
	errno = en;
	perror(s);
	errno = en;
	fprintf(stderr,"errno=%d\n",errno);
	exit(1);
}

void
mes(s)
char *s;
{
	fprintf(stderr,"ttcp%s: %s\n", trans?"-t":"-r", s);
}


/*-------------------------------------------------------------------------
 * pattern - 
 *-------------------------------------------------------------------------
 */
pattern(cp, cnt)
register char *cp;
register int cnt;
{
    register char c;
    c = 0;
    while( cnt-- > 0 )  {
	while( !isprint((c&0x7F)) )  c++;
	*cp++ = (c++&0x7F);
    }
}


char *
outfmt(b)
double b;
{
    static char obuf[50];
    switch (fmt) {
	case 'G':
	    sprintf(obuf, "%f GB", b / 1024.0 / 1024.0 / 1024.0);
	    break;
	default:
	case 'K':
	    sprintf(obuf, "%f KB", b / 1024.0);
	    break;
	case 'M':
	    sprintf(obuf, "%f MB", b / 1024.0 / 1024.0);
	    break;
	case 'g':
	    sprintf(obuf, "%f Gbit", b * 8.0 / 1024.0 / 1024.0 / 1024.0);
	    break;
	case 'k':
	    sprintf(obuf, "%f Kbit", b * 8.0 / 1024.0);
	    break;
	case 'm':
	    sprintf(obuf, "%f Mbit", b * 8.0 / 1024.0 / 1024.0);
	    break;
    }
    return obuf;
}



#if defined(SYSV)
/*ARGSUSED*/
static
getrusage(ignored, ru)
    int ignored;
    register struct rusage *ru;
{
    struct tms buf;

    times(&buf);

    /* Assumption: HZ <= 2147 (LONG_MAX/1000000) */
    ru->ru_stime.tv_sec  = buf.tms_stime / HZ;
    ru->ru_stime.tv_usec = ((buf.tms_stime % HZ) * 1000000) / HZ;
    ru->ru_utime.tv_sec  = buf.tms_utime / HZ;
    ru->ru_utime.tv_usec = ((buf.tms_utime % HZ) * 1000000) / HZ;
}

/*ARGSUSED*/
static 
gettimeofday(tp, zp)
    struct timeval *tp;
    struct timezone *zp;
{
    tp->tv_sec = time(0);
    tp->tv_usec = 0;
}
#endif /* SYSV */

/*
 *			P R E P _ T I M E R
 */
void
prep_timer()
{
	gettimeofday(&start_time, (struct timezone *)0);
	getrusage(RUSAGE_SELF, &ru0);
}

/*-------------------------------------------------------------------------
 * read_timer - 
 *-------------------------------------------------------------------------
 */
double read_timer(str,len)
char *str;
{
    struct rusage ru1;
    struct timeval tend, tstart, td;
    char line[132];
    
    getrusage(RUSAGE_SELF, &ru1);
    prusage(&ru0, &ru1, &stop_time, &start_time, line);
    (void)strncpy( str, line, len );
    
    /* Get CPU time (user+sys) */
    tvadd( &tend, &ru1.ru_utime, &ru1.ru_stime );
    tvadd( &tstart, &ru0.ru_utime, &ru0.ru_stime );
    tvsub( &td, &tend, &tstart );
    cput = td.tv_sec + ((double)td.tv_usec) / 1000000;
    if( cput < 0.00001 )  cput = 0.00001;
    return( cput );
}

static void
prusage(r0, r1, e, b, outp)
	register struct rusage *r0, *r1;
	struct timeval *e, *b;
	char *outp;
{
	struct timeval tdiff;
	register time_t t;
	register char *cp;
	register int i;
	int ms;

	t = (r1->ru_utime.tv_sec-r0->ru_utime.tv_sec)*100+
	    (r1->ru_utime.tv_usec-r0->ru_utime.tv_usec)/10000+
	    (r1->ru_stime.tv_sec-r0->ru_stime.tv_sec)*100+
	    (r1->ru_stime.tv_usec-r0->ru_stime.tv_usec)/10000;
	ms =  (e->tv_sec-b->tv_sec)*100 + (e->tv_usec-b->tv_usec)/10000;

#define END(x)	{while(*x) x++;}
#if defined(SYSV)
	cp = "%Uuser %Ssys %Ereal %P";
#else
#if defined(sgi)		/* IRIX 3.3 will show 0 for %M,%F,%R,%C */
	cp = "%Uuser %Ssys %Ereal %P %Mmaxrss %F+%Rpf %Ccsw";
#else
	cp = "%Uuser %Ssys %Ereal %P %Xi+%Dd %Mmaxrss %F+%Rpf %Ccsw";
#endif
#endif
	for (; *cp; cp++)  {
		if (*cp != '%')
			*outp++ = *cp;
		else if (cp[1]) switch(*++cp) {

		case 'U':
			tvsub(&tdiff, &r1->ru_utime, &r0->ru_utime);
			sprintf(outp,"%d.%01d", tdiff.tv_sec, tdiff.tv_usec/100000);
			END(outp);
			break;

		case 'S':
			tvsub(&tdiff, &r1->ru_stime, &r0->ru_stime);
			sprintf(outp,"%d.%01d", tdiff.tv_sec, tdiff.tv_usec/100000);
			END(outp);
			break;

		case 'E':
			psecs(ms / 100, outp);
			END(outp);
			break;

		case 'P':
			sprintf(outp,"%d%%", (int) (t*100 / ((ms ? ms : 1))));
			END(outp);
			break;

#if !defined(SYSV)
		case 'W':
			i = r1->ru_nswap - r0->ru_nswap;
			sprintf(outp,"%d", i);
			END(outp);
			break;

		case 'X':
			sprintf(outp,"%d", t == 0 ? 0 : (r1->ru_ixrss-r0->ru_ixrss)/t);
			END(outp);
			break;

		case 'D':
			sprintf(outp,"%d", t == 0 ? 0 :
			    (r1->ru_idrss+r1->ru_isrss-(r0->ru_idrss+r0->ru_isrss))/t);
			END(outp);
			break;

		case 'K':
			sprintf(outp,"%d", t == 0 ? 0 :
			    ((r1->ru_ixrss+r1->ru_isrss+r1->ru_idrss) -
			    (r0->ru_ixrss+r0->ru_idrss+r0->ru_isrss))/t);
			END(outp);
			break;

		case 'M':
			sprintf(outp,"%d", r1->ru_maxrss/2);
			END(outp);
			break;

		case 'F':
			sprintf(outp,"%d", r1->ru_majflt-r0->ru_majflt);
			END(outp);
			break;

		case 'R':
			sprintf(outp,"%d", r1->ru_minflt-r0->ru_minflt);
			END(outp);
			break;

		case 'I':
			sprintf(outp,"%d", r1->ru_inblock-r0->ru_inblock);
			END(outp);
			break;

		case 'O':
			sprintf(outp,"%d", r1->ru_oublock-r0->ru_oublock);
			END(outp);
			break;
		case 'C':
			sprintf(outp,"%d+%d", r1->ru_nvcsw-r0->ru_nvcsw,
				r1->ru_nivcsw-r0->ru_nivcsw );
			END(outp);
			break;
#endif /* !SYSV */
		}
	}
	*outp = '\0';
}

static void
tvadd(tsum, t0, t1)
	struct timeval *tsum, *t0, *t1;
{

	tsum->tv_sec = t0->tv_sec + t1->tv_sec;
	tsum->tv_usec = t0->tv_usec + t1->tv_usec;
	if (tsum->tv_usec > 1000000)
		tsum->tv_sec++, tsum->tv_usec -= 1000000;
}


/*-------------------------------------------------------------------------
 * tvsub - tdiff = t1 - t0 
 *-------------------------------------------------------------------------
 */
static void tvsub(tdiff, t1, t0)
	struct timeval *tdiff, *t1, *t0;
{

    tdiff->tv_sec = t1->tv_sec - t0->tv_sec;
    tdiff->tv_usec = t1->tv_usec - t0->tv_usec;
    if (tdiff->tv_usec < 0) {
	tdiff->tv_sec--;
	tdiff->tv_usec += 1000000;
    }
}

static void
psecs(l,cp)
long l;
register char *cp;
{
	register int i;

	i = l / 3600;
	if (i) {
		sprintf(cp,"%d:", i);
		END(cp);
		i = l % 3600;
		sprintf(cp,"%d%d", (i/60) / 10, (i/60) % 10);
		END(cp);
	} else {
		i = l;
		sprintf(cp,"%d", i / 60);
		END(cp);
	}
	i %= 60;
	*cp++ = ':';
	sprintf(cp,"%d%d", i / 10, i % 10);
}

/*
 *			N R E A D
 */
Nread( fd, buf, count )
int fd;
char *buf;
int count;
{
	struct sockaddr_in from;
	int len = sizeof(from);
	register int cnt;
	if( udp )  {
#if 0
		cnt = recvfrom( fd, buf, count, 0, &from, &len );
#else
		cnt = recv( fd, buf, count, 0);
#endif
		numCalls++;
	} else {
		if( b_flag )
			cnt = mread( fd, buf, count );	/* fill buf */
		else {
			cnt = read( fd, buf, count );
			numCalls++;
		}
		if (touchdata && cnt > 0) {
			register int c = cnt, sum;
			register char *b = buf;
			while (c--)
				sum += *b++;
		}
	}
	return(cnt);
}

/*-------------------------------------------------------------------------
 * Nwrite - 
 *-------------------------------------------------------------------------
 */
Nwrite(fd, buf, count)
int fd;
char *buf;
int count;
{
    register int cnt;
    
    if (udp)  {
      again:
	if (atm) cnt = write(fd, buf, count);
	else cnt = sendto(fd, buf, count, 0, &sinhim, sizeof(sinhim));
	numCalls++;
	if ( cnt<0 && errno == ENOBUFS )  {
	    delay(18000);
	    errno = 0;
	    goto again;
	}
    } else {
	cnt = write(fd, buf, count);
	numCalls++;
    }
if (cnt < 0) perror("WA:write");
    return(cnt);
}

void
delay(us)
{
	struct timeval tv;

	tv.tv_sec = 0;
	tv.tv_usec = us;
	(void)select( 1, (char *)0, (char *)0, (char *)0, &tv );
}

/*
 *			M R E A D
 *
 * This function performs the function of a read(II) but will
 * call read(II) multiple times in order to get the requested
 * number of characters.  This can be necessary because
 * network connections don't deliver data with the same
 * grouping as it is written with.  Written by Robert S. Miles, BRL.
 */
int
mread(fd, bufp, n)
int fd;
register char	*bufp;
unsigned	n;
{
	register unsigned	count = 0;
	register int		nread;

	do {
		nread = read(fd, bufp, n-count);
		numCalls++;
		if(nread < 0)  {
			perror("ttcp_mread");
			return(-1);
		}
		if(nread == 0)
			return((int)count);
		count += (unsigned)nread;
		bufp += nread;
	 } while(count < n);

	return((int)count);
}
