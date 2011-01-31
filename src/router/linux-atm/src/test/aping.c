/* aping.c - simple round-trip tester */

/* Written 1996,1997 by Werner Almesberger, EPFL-LRC */

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
#include <sys/time.h>
#include <atm.h>


#define SEND 25*1000
#define RECV 70*1000
#define RECOVER 10


static const char *rotor[] = { "|\r","/\r","-\r","\\\r" };


static void usage(const char *name)
{
    fprintf(stderr,"usage: %s [itf.]vpi.vci\n",name);
    exit(1);
}


int main(int argc,char **argv)
{
    struct timeval delta,now,next,fail;
    struct sockaddr_atmpvc addr;
    struct atm_qos qos;
    int s,i,len;

    if (argc != 2) usage(argv[0]);
    if ((s = socket(PF_ATMPVC,SOCK_DGRAM,0)) < 0) {
	perror("socket");
	return 1;
    }
    memset(&addr,0,sizeof(addr));
    if (text2atm(argv[1],(struct sockaddr *) &addr,sizeof(addr),
      T2A_PVC | T2A_UNSPEC | T2A_WILDCARD) < 0) usage(argv[0]);
    memset(&qos,0,sizeof(qos));
    qos.aal = ATM_AAL5;
    qos.txtp.traffic_class = ATM_UBR;
    qos.txtp.traffic_class = ATM_UBR;
    qos.txtp.max_sdu = 1;
    qos.rxtp = qos.txtp;
    if (setsockopt(s,SOL_ATM,SO_ATMQOS,&qos,sizeof(qos)) < 0) {
	perror("setsockopt SO_ATMQOS");
	return 1;
    }
    if (bind(s,(struct sockaddr *) &addr,sizeof(addr)) < 0) {
	perror("bind");
	return 1;
    }
    if (gettimeofday(&next,NULL) < 0) {
	perror("gettimeofday");
	return 1;
    }
    fail = next;
    fail.tv_sec += RECOVER;
    delta.tv_sec = delta.tv_usec = 0;
    i = 0;
    len = 1; /* length varies to make losses more visible in the kernel */
    while (1) {
	fd_set set;

	FD_ZERO(&set);
	FD_SET(s,&set);
	(void) select(s+1,&set,NULL,NULL,&delta);
	if (gettimeofday(&now,NULL) < 0) {
	    perror("gettimeofday");
	    return 1;
	}
	if (FD_ISSET(s,&set)) {
	    char dummy[2];
	    int size;

	    size = read(s,dummy,2);
	    if (size < 0) {
		perror("read");
		return 1;
	    }
	    if (size != 1) {
		fprintf(stderr,"bad RX (%d)\n",size);
#if 0
		if (size > 1) return 1;
#endif
	    }
	    fail = now;
	    fail.tv_usec += RECV;
	    while (fail.tv_usec >= 1000000) {
		fail.tv_usec -= 1000000;
		fail.tv_sec++;
	    }
	    (void) write(1,rotor[i = (i+1) & 3],2);
len = 1;
	}
	if (fail.tv_sec < now.tv_sec || (fail.tv_sec == now.tv_sec &&
	  fail.tv_usec < now.tv_usec)) {
	    fprintf(stderr,"RX timed out\n");
	    fail.tv_sec += RECOVER;
#if 0
#if 1
len++;
#else
	    len = 2 /*1+(len % 3)*/;
#endif
#endif
	}
	while (next.tv_sec < now.tv_sec || (next.tv_sec == now.tv_sec &&
	  next.tv_usec < now.tv_usec)) {
	    if (write(s,"XYZ...",len) != len) {
		perror("write");
		return 1;
	    }
	    next.tv_usec += SEND;
	    while (next.tv_usec >= 1000000) {
		next.tv_usec -= 1000000;
		next.tv_sec++;
	    }
	}
	delta.tv_sec = next.tv_sec-now.tv_sec;
	delta.tv_usec = next.tv_usec-now.tv_usec;
	while (delta.tv_usec < 0) {
	    delta.tv_sec--;
	    delta.tv_usec += 1000000;
	}
    }
    return 0;
}
