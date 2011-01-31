/* saaldump.c - ATM signaling message dumper */

/* Written 1997-1999 by Werner Almesberger, EPFL-LRC/ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <atm.h>

#include "pdu.h"
#define DUMP_MODE
#include "qlib.h"


#define MAX_ITEM  2048 /* longest string emitted by q.dump */


static int interval = 0; /* display absolute time by default */
static int quiet = 0; /* decode Q.2931 messages by default */
static int new_line = 1;


static void sscop_pdu_diag(int severity,const char *fmt,...)
{
    va_list ap;

    va_start(ap,fmt);
    printf("  %s",severity == SP_ERROR ? "ERROR: " : "");
    vprintf(fmt,ap);
    putchar('\n');
    va_end(ap);
}


void qd_dump(const char *msg,...)
{
    char buf[MAX_ITEM];
    va_list ap;
    int len;

    if (new_line) printf("    ");
    va_start(ap,msg);
    len = vsprintf(buf,msg,ap);
    va_end(ap);
    if (len > MAX_ITEM) {
	fprintf(stderr,"FATAL ERROR: buffer too small (%d < %d)\n",MAX_ITEM,
	  len);
	exit(1);
    }
    printf("%s",buf);
    new_line = len ? buf[len-1] == '\n' : 0;
}


void qd_report(int severity,const char *msg,...)
{
    va_list ap;

    if (severity > Q_ERROR) return;
    printf("%s    ERROR: ",new_line ? "" : "\n");
    va_start(ap,msg);
    vprintf(msg,ap);
    va_end(ap);
    putchar('\n');
    new_line = 1;
}

static void analyze(unsigned char *pdu,int size,struct timeval stamp)
{
    static struct timeval last;
    static int first = 1;
    PDU_VARS;

    if (first || !interval) {
	printf("%2d:%02d:%02d ",(int) ((stamp.tv_sec/3600) % 24),
	  (int) ((stamp.tv_sec/60) % 60),(int) (stamp.tv_sec % 60));
	if (interval) {
	    first = 0;
	    last = stamp;
	}
    }
    else {
	struct timeval diff;

	diff.tv_sec = stamp.tv_sec-last.tv_sec;
	diff.tv_usec = stamp.tv_usec-last.tv_usec;
	while (diff.tv_usec < 0) {
	    diff.tv_usec += 1000000;
	    diff.tv_sec--;
	}
	last = stamp;
	printf("%05u.%02u",(unsigned) diff.tv_sec,
	  (unsigned) diff.tv_usec/10000);
    }
    if (!DECOMPOSE_PDU(NULL,pdu,size)) {
	PRINT_PDU("",pdu);
	if (type == SSCOP_SD && !quiet) {
	    Q_DSC dsc;

	    qd_open(&dsc,pdu,length);
	    qd_close(&dsc);
	}
    }
}


static void usage(const char *name)
{
    fprintf(stderr,"usage: %s [-i] [-q] [itf.]vpi.vci\n",name);
    exit(1);
}


int main(int argc,char **argv)
{
    char buf[ATM_MAX_AAL5_PDU];
    struct sockaddr_atmpvc addr;
    struct atm_qos qos;
    char *name;
    int c,s,size;

    name = argv[0];
    while ((c = getopt(argc,argv,"iq")) != EOF)
	switch (c) {
	    case 'i':
		interval = 1;
		break;
	    case 'q':
		quiet = 1;
		break;
	    default:
		usage(name);
	}
    if (argc != optind+1) usage(name);
    if ((s = socket(PF_ATMPVC,SOCK_DGRAM,0)) < 0) {
	perror("socket");
	return 1;
    }
    memset(&addr,0,sizeof(addr));
    if (text2atm(argv[optind],(struct sockaddr *) &addr,sizeof(addr),T2A_PVC)
      < 0) usage(name);
    memset(&qos,0,sizeof(qos));
    qos.aal = ATM_AAL5;
    qos.rxtp.traffic_class = ATM_UBR;
    qos.rxtp.max_sdu = ATM_MAX_AAL5_PDU;
    if (setsockopt(s,SOL_ATM,SO_ATMQOS,&qos,sizeof(qos)) < 0) {
	perror("setsockopt SO_ATMQOS");
	return 1;
    }
    if (bind(s,(struct sockaddr *) &addr,sizeof(addr)) < 0) {
	perror("bind");
	return 1;
    }
    pdu_diag = sscop_pdu_diag;
    qd_start();
    while ((size = read(s,buf,ATM_MAX_AAL5_PDU)) > 0) {
	struct timeval stamp;

	if (ioctl(s,SIOCGSTAMP,&stamp) < 0) {
	    perror("ioctl SIOCGSTAMP");
	    return 1;
	}
	analyze(buf,size,stamp);
	fflush(stdout);
    }
    perror("read");
    return 1;
}
