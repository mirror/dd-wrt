/* awrite.c - send AAL5 PDU */

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


static void usage(const char *name)
{
    fprintf(stderr,"usage: %s [itf.]vpi.vci data [ offset ] \n",name);
    exit(1);
}


int main(int argc,char **argv)
{
    struct sockaddr_atmpvc addr;
    struct atm_qos qos;
    int s,size,offset;

    if (argc != 3 && argc != 4) usage(argv[0]);
    if ((s = socket(PF_ATMPVC,SOCK_DGRAM,0)) < 0) {
	perror("socket");
	return 1;
    }
    memset(&addr,0,sizeof(addr));
    if (text2atm(argv[1],(struct sockaddr *) &addr,sizeof(addr),T2A_PVC) < 0)
	usage(argv[0]);
    offset = argc == 3 ? 0 : atoi(argv[3]);
    memset(&qos,0,sizeof(qos));
    qos.aal = ATM_AAL5;
    qos.txtp.traffic_class = ATM_UBR;
    qos.txtp.max_sdu = strlen(argv[2])-offset;
    if (setsockopt(s,SOL_ATM,SO_ATMQOS,&qos,sizeof(qos)) < 0) {
	perror("setsockopt SO_ATMQOS");
	return 1;
    }
    if (connect(s,(struct sockaddr *) &addr,sizeof(addr)) < 0) {
	perror("connect");
	return 1;
    }
    size = write(s,argv[2]+offset,strlen(argv[2])-offset);
    printf("%d",size);
    if (size < 0) printf(" (%s)",strerror(errno));
    putchar('\n');
    return 0;
}
