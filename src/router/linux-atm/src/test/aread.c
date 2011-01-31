/* aread.c - receive AAL5 PDU */

/* Written 1995-1998 by Werner Almesberger, EPFL-LRC/ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <atm.h>


#define BSIZE 1024


static void usage(const char *name)
{
    fprintf(stderr,"usage: %s [-c] [itf.]vpi.vci\n",name);
    exit(1);
}


int main(int argc,char **argv)
{
    const char *name;
    struct sockaddr_atmpvc addr;
    struct atm_qos qos;
    int chars,s;

    name = argv[0];
    chars = argc == 3 && !strcmp(argv[1],"-c");
    if (chars) {
	argc--;
	argv++;
    }
    if (argc != 2) usage(name);
    if ((s = socket(PF_ATMPVC,SOCK_DGRAM,0)) < 0) {
	perror("socket");
	return 1;
    }
    memset(&addr,0,sizeof(addr));
    if (text2atm(argv[1],(struct sockaddr *) &addr,sizeof(addr),
      T2A_PVC | T2A_UNSPEC | T2A_WILDCARD) < 0) usage(name);
    memset(&qos,0,sizeof(qos));
    qos.aal = ATM_AAL5;
    qos.rxtp.traffic_class = ATM_UBR;
    qos.rxtp.max_sdu = BSIZE;
    if (setsockopt(s,SOL_ATM,SO_ATMQOS,&qos,sizeof(qos)) < 0) {
	perror("setsockopt SO_ATMQOS");
	return 1;
    }
    if (bind(s,(struct sockaddr *) &addr,sizeof(addr)) < 0) {
	perror("bind");
	return 1;
    }
    while (1) {
	unsigned char buf[BSIZE+4096];
	unsigned char *start;
	int size,i;

	/* Make sure the buffer is aligned. This can be trivially extended to
	   play with alignments. */
	start = (unsigned char *) (((unsigned long) buf+4095) & ~4095UL);
	size = read(s,start,BSIZE);
	printf("%d",size);
	if (size < 0) printf(" (%s)",strerror(errno));
	printf(": ");
	for (i = 0; i < size; i++)
	    if (chars)
		if (start[i] > ' ' && start[i] < 127) printf(" %c",start[i]);
		else printf(" \\%03o",start[i]);
	    else printf(" %02X",start[i]);
	putchar('\n');
    }
}
