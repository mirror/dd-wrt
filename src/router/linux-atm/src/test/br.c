/* br.c - block read */

/* Written 1995-2000 by Werner Almesberger, EPFL-LRC/ICA */

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


#define BSIZE 8192


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
	ssize_t size;

	start = (unsigned char *) (((unsigned long) buf+4095) & ~4095U);
	size = read(s,start,BSIZE);
	if (size > 0 && write(1,start,size) != size) return 1;
    }
}
