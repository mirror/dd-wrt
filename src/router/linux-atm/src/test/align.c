/* align.c - Exercise PDU mis-alignment handling by the NIC */

/* Written 1997,1998 by Werner Almesberger, EPFL-LRC/ICA */

/*
 * This program requires a kernel modification: if the first byte of the PDU
 * to send is a small decimal digit N < 8, the first N bytes of the PDU are
 * removed and the start address is shifted accordingly. This forces
 * mis-alignment of the PDU.
 *
 * The expected network configuration is a loopback between the TXer and the
 * RXer of the NIC under test.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <atm.h>


#define MAX_SDU    20000
#define MAX_OFFSET 8


static void handler(int dummy)
{
    (void) signal(SIGALRM,&handler);
}


static void torture(int s)
{
    static unsigned long done[MAX_SDU*MAX_OFFSET/8/sizeof(unsigned long)];
    unsigned char in[MAX_SDU],out[MAX_SDU];
    unsigned char ch;
    int todo;

    todo = MAX_SDU*MAX_OFFSET-(MAX_OFFSET+1)*MAX_OFFSET/2;
    handler(0);
    while (todo) {
	int length,offset;
	int word,bit;
	int i,sent,got;

	length = (random() % MAX_SDU)+1;
	offset = random() % MAX_OFFSET;
	ch = random() & 0xff;
	bit = (length-1)*MAX_OFFSET+offset;
	word = bit/sizeof(unsigned long)/8;
	bit &= sizeof(unsigned long)*8-1;
	if (length <= offset || (done[word] & (1 << bit))) continue;
	out[0] = offset+'0';
	for (i = 1; i < length; i++) out[i] = ch++;
	sent = write(s,out,length);
	if (sent < 0) {
	    perror("write");
	    exit(1);
	}
	if (sent != length) {
	    fprintf(stderr,"bad write: %d != %d\n",sent,length);
	    exit(1);
	}
	alarm(1);
	got = read(s,in,length);
	alarm(0);
	if (got < 0 && errno == EINTR) {
	    fprintf(stderr,"timed out at length %d, offset %d\n",length,offset);
	    continue;
	}
	if (got < 0) {
	    perror("read");
	    exit(1);
	}
	if (got != length-offset) {
	    fprintf(stderr,"bad read: %d != %d-%d\n",got,length,offset);
	    exit(1);
	}
	if (memcmp(out+offset,in,length-offset)) {
	    fprintf(stderr,"bad compare: length %d, offset %d\n",length,
		offset);
	    exit(1);
	}
	done[word] |= 1 << bit;
	todo--;
	if (!(todo % 100)) fprintf(stderr,"%6d\r",todo);
    }
}


static void usage(const char *name)
{
    fprintf(stderr,"usage: %s [itf.]vpi.vci\n",name);
    exit(1);
}


int main(int argc,char **argv)
{
    struct sockaddr_atmpvc addr;
    struct atm_qos qos;
    int s;

    if (argc != 2) usage(argv[0]);
    if ((s = socket(PF_ATMPVC,SOCK_DGRAM,0)) < 0) {
	perror("socket");
	return 1;
    }
    memset(&addr,0,sizeof(addr));
    if (text2atm(argv[1],(struct sockaddr *) &addr,sizeof(addr),T2A_PVC) < 0)
	usage(argv[0]);
    memset(&qos,0,sizeof(qos));
    qos.aal = ATM_AAL5;
    qos.txtp.traffic_class = qos.rxtp.traffic_class = ATM_UBR;
    qos.txtp.max_sdu = qos.rxtp.max_sdu = MAX_SDU;
    if (setsockopt(s,SOL_ATM,SO_ATMQOS,&qos,sizeof(qos)) < 0) {
	perror("setsockopt SO_ATMQOS");
	return 1;
    }
    if (connect(s,(struct sockaddr *) &addr,sizeof(addr)) < 0) {
	perror("connect");
	return 1;
    }
    srandom(0); /* we want it to be deterministic */
    torture(s);
    return 0;
}
