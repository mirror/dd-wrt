/* bw.c - block write */

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
    fprintf(stderr,"usage: %s [itf.]vpi.vci [ blocks ]\n",name);
    exit(1);
}


int main(int argc,char **argv)
{
    struct sockaddr_atmpvc addr;
    struct atm_qos qos;
    char buffer[BSIZE];
    int blocks,s;
    ssize_t size;

    if (argc != 2 && argc != 3) usage(argv[0]);
    if (argc == 2) blocks = 0;
    else blocks = atoi(argv[2]);
    if ((s = socket(PF_ATMPVC,SOCK_DGRAM,0)) < 0) {
	perror("socket");
	return 1;
    }
    memset(&addr,0,sizeof(addr));
    if (text2atm(argv[1],(struct sockaddr *) &addr,sizeof(addr),T2A_PVC) < 0)
	usage(argv[0]);
    memset(&qos,0,sizeof(qos));
    qos.aal = ATM_AAL5;
    qos.txtp.traffic_class = ATM_UBR;
    qos.txtp.max_sdu = BSIZE;
    if (setsockopt(s,SOL_ATM,SO_ATMQOS,&qos,sizeof(qos)) < 0) {
	perror("setsockopt SO_ATMQOS");
	return 1;
    }
    if (connect(s,(struct sockaddr *) &addr,sizeof(addr)) < 0) {
	perror("bind");
	return 1;
    }
    if (blocks)
	while (blocks--) (void) write(s,buffer,BSIZE);
    else while ((size = read(0,buffer,BSIZE)) > 0)
	    (void) write(s,buffer,size);
    return 0;
}
