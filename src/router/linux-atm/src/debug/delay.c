/* delay.c - Simplistic AAL5-level software delay line */

/* Written 1996-2000 by Werner Almesberger, EPFL-LRC/ICA */


/*
 * BUGS:
 *  - delay increases with load
 *  - processing delay is far too big (always measure with ping first)
 *  - horrifying command-line syntax
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>

#include "atm.h"


typedef struct _packet {
    struct timeval due;
    int size;
    struct _packet *next;
    char data[1];
} PACKET;

typedef struct _link {
    int in,out;
    struct timeval delay;
    PACKET *queue,*last;
    struct _link *next;
} LINK;


static LINK *links = NULL;
static fd_set in;
static int fds = 0;


#define LESS(a,b) ((a).tv_sec < (b).tv_sec || ((a).tv_sec == (b).tv_sec && \
  (a).tv_usec < (b).tv_usec))


static void loop(void)
{
    LINK *lnk;
    PACKET *p;
    struct timeval now,next,delta,*to;
    fd_set curr;
    char *buffer;
    int ready,size;

    if (!(buffer = malloc(sizeof(PACKET)-1+ATM_MAX_AAL5_PDU+4095))) {
	perror("buffer");
	exit(1);
    }
    buffer = (char *) (((unsigned long) buffer+4095-(sizeof(PACKET)-1)) &
      ~4095);
    if (gettimeofday(&now,NULL) < 0) {
	perror("gettimeofday");
	exit(1);
    }
    while (1) {
	curr = in;
	for (lnk = links; lnk; lnk = lnk->next)
	    if (lnk->queue) break;
	if (!lnk) to = NULL;
	else {
	    for (next = lnk->queue->due; lnk; lnk = lnk->next)
		if (lnk->queue && LESS(lnk->queue->due,next))
		    next = lnk->queue->due;
	    delta.tv_sec = next.tv_sec-now.tv_sec;
	    delta.tv_usec = next.tv_usec-now.tv_usec;
	    if (delta.tv_usec < 0) {
		delta.tv_sec--;
		delta.tv_usec += 1000000;
	    }
	    if (delta.tv_usec > 0) {
		delta.tv_sec++;
		delta.tv_usec -= 1000000;
	    }
	    if (delta.tv_sec < 0) delta.tv_sec = delta.tv_usec = 0;
	    to = &delta;
	}
	if ((ready = select(fds,&curr,NULL,NULL,to)) < 0) {
	    perror("select");
	    exit(1);
	}
	if (gettimeofday(&now,NULL) < 0) {
	    perror("gettimeofday");
	    exit(1);
	}
	if (ready)
	    for (lnk = links; lnk; lnk = lnk->next)
		while (1) {
		    size = read(lnk->in,buffer,ATM_MAX_AAL5_PDU);
		    if (size < 0) {
			if (errno == EAGAIN) break;
			else {
			    perror("read");
			    exit(1);
			}
		    }
		    if (size > 0) {
			if (!(p = malloc(sizeof(PACKET)-1+size))) {
			    perror("malloc");
			    exit(1);
			}
			memcpy(p->data,buffer,size);
			p->size = size;
			p->due.tv_sec = now.tv_sec+lnk->delay.tv_sec;
			p->due.tv_usec = now.tv_usec+lnk->delay.tv_usec;
			if (p->due.tv_usec > 1000000) {
			    p->due.tv_sec++;
			    p->due.tv_usec -= 1000000;
			}
			p->next = NULL;
			if (lnk->queue) lnk->last->next = p;
			else lnk->queue = p;
			lnk->last = p;
		    }
		}
	for (lnk = links; lnk; lnk = lnk->next)
	    while (lnk->queue && LESS(lnk->queue->due,now)) {
		p = lnk->queue;
		lnk->queue = p->next;
		if ((size = write(lnk->out,p->data,p->size)) < 0) {
		    perror("write");
		    exit(1);
		}
		if (p->size != size)
		    fprintf(stderr,"short write: %d < %d\n",size,p->size);
		free(p);
	    }
    }
}


static int setup(char *spec,int tx)
{
    struct sockaddr_atmpvc addr;
    struct atm_qos qos;
    char *here;
    int fd;

    if (!(here = strchr(spec,','))) {
	memset(&qos,0,sizeof(qos));
	if (tx) qos.txtp.traffic_class = ATM_UBR;
	else qos.rxtp.traffic_class = ATM_UBR;
    }
    else {
	*here = 0;
	if (text2qos(here+1,&qos,0) < 0) {
	    fprintf(stderr,"invalid QOS: %s\n",here+1);
	    exit(1);
	}
    }
    if (tx) qos.rxtp.traffic_class = ATM_NONE;
    else qos.txtp.traffic_class = ATM_NONE;
    qos.aal = ATM_AAL5;
    if (text2atm(spec,(struct sockaddr *) &addr,sizeof(addr),
      T2A_PVC | T2A_NAME) < 0) {
	fprintf(stderr,"invalid PVC: %s\n",spec);
	exit(1);
    }
    if ((fd = socket(PF_ATMPVC,SOCK_DGRAM,0)) < 0) {
	perror("socket");
	exit(1);
    }
    if (setsockopt(fd,SOL_ATM,SO_ATMQOS,&qos,sizeof(qos)) < 0) {
	perror("setsockopt SO_ATMQOS");
	exit(1);
    }
    if (bind(fd,(struct sockaddr *) &addr,sizeof(addr)) < 0) {
	perror("bind");
	exit(1);
    }
    return fd;
}


static void usage(const char *name)
{
    fprintf(stderr,"usage: %s path ...\n",name);
    fprintf(stderr,"   path :== vc/vc/delay\n");
    fprintf(stderr,"   vc :== [itf.]vpi.vci[,qos_spec]\n");
    fprintf(stderr,"   delay :== timeunit\n");
    fprintf(stderr,"   unit :== s | ms\n");
    exit(1);
}


int main(int argc,char **argv)
{
    LINK *dsc;
    const char *name;
    unsigned long delay;
    char *end,*here;

    name = argv[0];
    FD_ZERO(&in);
    if (argc < 2) usage(name);
    while (argc > 1) {
	argc--;
	argv++;
	if (!(dsc = malloc(sizeof(LINK)))) {
	    perror("malloc");
	    return 1;
	}
	if (!(here = strtok(*argv,"/"))) usage(name);
	dsc->in = setup(here,0);
	FD_SET(dsc->in,&in);
	if (dsc->in >= fds) fds = dsc->in+1;
	if (fcntl(dsc->in,F_SETFL,O_NONBLOCK) < 0) {
	    perror("fcntl");
	    return 1;
	}
	if (!(here = strtok(NULL,"/"))) usage(name);
	dsc->out = setup(here,1);
	if (!(here = strtok(NULL,"/"))) usage(name);
	delay = strtoul(here,&end,10);
	switch (*end) {
	    case 's':
		dsc->delay.tv_sec = delay;
		dsc->delay.tv_usec = 0;
		break;
	    case 'm':
		dsc->delay.tv_sec = delay/1000;
		dsc->delay.tv_usec = (delay % 1000)*1000;
		if (*++end != 's') usage(name);
		break;
	    default:
		usage(name);
	}
	if (end[1]) usage(name);
	dsc->queue = dsc->last = NULL;
	dsc->next = links;
	links = dsc;
    }
    loop();
    return 0;
}
