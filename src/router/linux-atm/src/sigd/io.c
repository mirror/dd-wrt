/* io.c - I/O operations */
 
/* Written 1995-2000 by Werner Almesberger, EPFL-LRC/ICA */

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
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <atm.h>
#include <linux/atmdev.h>
#include <linux/atmsvc.h>

#include "atmd.h"
#include "uni.h"
#include "pdu.h"

#include "proto.h"
#include "io.h"
#include "trace.h"


#define COMPONENT "IO"


struct timeval now;

int stop = 0;

static int kernel = -1;
static int need_connect = 0; /* non-zero if connection to kernel isn't
				bi-directional yet */


/* ----- kernel interface -------------------------------------------------- */


static int open_kernel(void)
{
    int s;

    if ((s = socket(PF_ATMSVC,SOCK_DGRAM,0)) < 0) {
	perror("socket");
	return -1;
    }
    if (ioctl(s,ATMSIGD_CTRL,0) < 0) {
	perror("ioctl ATMSIGD_CTRL");
	return -1;
    }
    return s;
}


void open_unix(const char *path)
{
    kernel = un_create(path,0600);
    if (kernel < 0)
	diag(COMPONENT,DIAG_FATAL,"un_create %s: %s",path,strerror(errno));
    need_connect = 1;
}


static void recv_kernel(void)
{
    static unsigned char buffer[sizeof(struct atmsvc_msg)+1];
    int size;

    if (!need_connect) size = read(kernel,buffer,sizeof(buffer));
    else {
	size = un_recv_connect(kernel,buffer,sizeof(buffer));
	need_connect = 0;
    }
    if (size < 0) {
	diag(COMPONENT,DIAG_ERROR,"read kernel: %s",strerror(errno));
	return;
    }
    if (size != sizeof(struct atmsvc_msg))
	diag(COMPONENT,DIAG_FATAL,"kernel message size %d != %d",size,
	  sizeof(struct atmsvc_msg));
    trace_kernel("FROM KERNEL",(struct atmsvc_msg *) buffer);
    from_kernel((struct atmsvc_msg *) buffer,size);
}


void to_kernel(struct atmsvc_msg *msg)
{
    int wrote;

    diag("KERNEL",DIAG_DEBUG,"TO KERNEL: %s (%d) for %s/%s",
      as_name[msg->type],msg->reply,kptr_print(&msg->vcc),
      kptr_print(&msg->listen_vcc));
	/* should be "IO" ... */
    trace_kernel("TO KERNEL",msg);
    wrote = write(kernel,msg,sizeof(*msg));
    if (wrote == sizeof(*msg)) return;
    if (wrote < 0) {
	perror("kernel write");
	return;
    }
    diag(COMPONENT,DIAG_ERROR,"bad kernel write: wanted %d, wrote %d",
      sizeof(*msg),wrote);
}


static void close_kernel(void)
{
    (void) close(kernel); /* may get major complaints from the kernel ... */
}


/* ----- signaling interface ----------------------------------------------- */


static int open_signaling(SIG_ENTITY *sig)
{
    struct atm_qos qos;
    int s;

    if ((s = socket(PF_ATMPVC,SOCK_DGRAM,0)) < 0) {
	perror("socket");
	return -1;
    }
    memset(&qos,0,sizeof(qos));
    qos.aal = ATM_AAL5;
    qos.rxtp.max_sdu = qos.txtp.max_sdu = MAX_Q_MSG;
    if (sig->sig_qos) {
	if (text2qos(sig->sig_qos,&qos,T2Q_DEFAULTS) < 0) {
	    fprintf(stderr,"invalid qos: %s\n",sig->sig_qos);
	    return -1;
	}
    }
    else {
	if (sig->sig_pcr == -1)
	    qos.rxtp.traffic_class = qos.txtp.traffic_class = ATM_UBR;
	else {
	    qos.rxtp.traffic_class = qos.txtp.traffic_class = ATM_CBR;
	    qos.rxtp.min_pcr = qos.txtp.min_pcr = sig->sig_pcr;
	}
    }
    if (setsockopt(s,SOL_ATM,SO_ATMQOS,&qos,sizeof(qos)) < 0) {
	perror("setsockopt SO_ATMQOS");
	return -1;
    }
    sig->signaling_pvc.sap_family = AF_ATMPVC;
    if (bind(s,(struct sockaddr *) &sig->signaling_pvc,
      sizeof(sig->signaling_pvc)) < 0) {
	perror("bind");
	return -1;
    }
    return s;
}


static void recv_signaling(SIG_ENTITY *sig)
{
    static unsigned char buffer[MAX_Q_MSG];
    int size;

    size = read(sig->signaling,buffer,MAX_Q_MSG);
    if (size < 1) {
	perror("read signaling");
	return;
    }
    diag(COMPONENT,DIAG_DEBUG,"FROM NET (%d.%d.%d): %s PDU (%d bytes)",
      S_PVC(sig),pdu_name[size > 3 && !(size & 3) ? buffer[size-4] & 0xf : 0],
      size);
    if (debug) diag_dump(COMPONENT,DIAG_DEBUG,NULL,buffer,size);
    from_net(sig,buffer,size);
}


void to_net(SIG_ENTITY *sig,void *msg,int size)
{
    int wrote;

    diag(COMPONENT,DIAG_DEBUG,"TO NET (%d.%d.%d): %s PDU (%d bytes)",S_PVC(sig),
      pdu_name[size > 3 && !(size & 3) ? ((unsigned char *) msg)[size-4] & 0xf
      : 0],size);
    if (debug) diag_dump(COMPONENT,DIAG_DEBUG,NULL,msg,size);
    wrote = write(sig->signaling,msg,size);
    if (wrote == size) return;
    if (wrote < 0) {
	perror("signaling write");
	return;
    }
    diag(COMPONENT,DIAG_WARN,"bad signaling write: wanted %d, wrote %d",size,
      wrote);
}


static void close_signaling(SIG_ENTITY *sig)
{
    (void) close(sig->signaling);
}


/* ----- addresses --------------------------------------------------------- */


int get_addr(int itf,LOCAL_ADDR *local_addr)
{
    struct atmif_sioc req;
    struct sockaddr_atmsvc buffer[MAX_LOCAL_ADDRS];
    LOCAL_ADDR *from,*to;
    int addrs,i;

    for (from = to = local_addr; from->state != ls_unused; from++)
	if (from->state != ls_removed) {
	    from->state = ls_removed;
	    *to++ = *from;
	}
    to->state = ls_unused;
    req.number = itf;
    req.arg = buffer;
    req.length = sizeof(buffer);
    if (ioctl(entities->signaling,ATM_GETADDR,&req) < 0)
	diag(COMPONENT,DIAG_FATAL,"ioctl ATM_GETADDR yields \"%s\"",
	  strerror(errno));
    addrs = req.length/sizeof(struct sockaddr_atmsvc);
    for (i = 0; i < addrs; i++) {
	for (from = local_addr; from->state != ls_unused; from++)
	    if (atm_equal((struct sockaddr *) (buffer+i),
	      (struct sockaddr *) &from->addr,0,0)) break;
	if (from->state != ls_unused) from->state = ls_same;
	else if (to == local_addr+MAX_LOCAL_ADDRS-1)
		diag(COMPONENT,DIAG_WARN,"local address table overflow");
	    else {
		to->state = ls_added;
		to->addr = buffer[i];
		to++;
		to->state = ls_unused;
	    }
    }
    return addrs;
}


/* ----- common part ------------------------------------------------------- */


int open_all(void)
{
    SIG_ENTITY *sig,*purge;

    if (kernel == -1) kernel = open_kernel();
    if (kernel < 0) return -1;
    for (sig = entities; sig; sig = sig->next) {
	sig->signaling = open_signaling(sig);
	if (sig->signaling < 0) {
	    for (purge = entities; purge != sig; purge = purge->next)
		close_signaling(purge);
	    close_kernel();
	    return -1;
	}
    }
    return 0;
}


void close_all(void)
{
    SIG_ENTITY *sig;

    close_kernel();
    for (sig = entities;sig; sig = sig->next) close_signaling(sig);
}


void init_current_time(void)
{
    gettimeofday(&now,NULL);
}


void poll_loop(void)
{
    SIG_ENTITY *sig;
    fd_set perm,set;
    int fds,ret;

    FD_ZERO(&perm);
    FD_SET(kernel,&perm);
    fds = kernel+1;
    for (sig = entities; sig; sig = sig->next) {
	FD_SET(sig->signaling,&perm);
	if (fds <= sig->signaling) fds = sig->signaling+1;
    }
    gettimeofday(&now,NULL);
    while (!stop) {
	set = perm;
	poll_signals();
	/*
	 * Here we have a small race condition: if a signal is delivered after
	 * poll_signals tests for it but before select sleeps, we miss that
	 * signal. If it is sent again, we're of course likely to get it. This
	 * isn't worth fixing, because those signals are only used for
	 * debugging anyway.
	 */
	ret = select(fds,&set,NULL,NULL,next_timer());
	if (ret < 0) {
	    if (errno != EINTR) perror("select");
	}
	else {
	    diag(COMPONENT,DIAG_DEBUG,"----------");
	    gettimeofday(&now,NULL);
	    if (FD_ISSET(kernel,&set)) recv_kernel();
	    for (sig = entities; sig; sig = sig->next)
		if (FD_ISSET(sig->signaling,&set)) recv_signaling(sig);
	    expire_timers();
	      /* expire timers after handling messges to make sure we don't
		 time out unnecessarily because of scheduling delays */
	}
    }
}


/*
 * The allocation strategy could be improved as follows: we should try
 * vci = prev_vci++  first and only resort to ATM_VCI_ANY if that fails several
 * times (and we should actually skip over those which are in use by SVCs. This
 * way we avoid using VCIs that just became available. Doing it "right" seems
 * to be getting complex, though.
 */


int get_pvc(int itf,int *vci)
{
    struct sockaddr_atmpvc addr;
    struct atm_qos qos;
    int s,error;

    if ((s = socket(PF_ATMPVC,SOCK_DGRAM,0)) < 0)
        diag(COMPONENT,DIAG_FATAL,"get_pvc: %s",strerror(errno));
    memset(&qos,0,sizeof(qos));
    qos.aal = ATM_AAL5;
    qos.rxtp.traffic_class = qos.txtp.traffic_class = ATM_UBR;
    qos.rxtp.max_sdu = qos.txtp.max_sdu = 1; /* smallest possible SDU size */
    if (setsockopt(s,SOL_ATM,SO_ATMQOS,&qos,sizeof(qos)) < 0)
	diag(COMPONENT,DIAG_FATAL,"setsockopt SO_ATMQOS: %s",strerror(errno));
    memset(&addr,0,sizeof(addr));
    addr.sap_family = AF_ATMPVC;
    addr.sap_addr.itf = itf;
    addr.sap_addr.vpi = 0; /* @@@ */
    addr.sap_addr.vci = ATM_VCI_ANY;
    error = 0;
    if (bind(s,(struct sockaddr *) &addr,sizeof(addr)) < 0) error = errno;
    else {
	socklen_t size;

	size = sizeof(addr);
	if (getsockname(s,(struct sockaddr *) &addr,&size) < 0)
	    diag(COMPONENT,DIAG_FATAL,"get_pvc: %s",strerror(errno));
	*vci = addr.sap_addr.vci;
	return s;
    }
    (void) close(s);
    return -error;
}


int get_link_rate(int itf)
{
    struct atmif_sioc req;
    int rate;

    req.number = itf;
    req.arg = &rate;
    req.length = sizeof(rate);
    if (ioctl(entities->signaling,ATM_GETLINKRATE,&req) < 0)
	diag(COMPONENT,DIAG_FATAL,"ioctl ATM_GETLINKRATE yields \"%s\"",
	  strerror(errno));
    return rate;
}
