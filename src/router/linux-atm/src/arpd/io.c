/* io.c - I/O operations */
 
/* Written 1995-2000 by Werner Almesberger, EPFL-LRC/ICA */
 
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <netinet/in.h>
#include <atm.h>
#include <linux/atmclip.h> /* for CLIP_DEFAULT_IDLETIMER */
#include <linux/atmarp.h>
#define _LINUX_NETDEVICE_H /* glibc2 */
#include <linux/types.h>
#include <linux/if_arp.h>

#include "atmd.h"
#include "atmarpd.h"

#include "table.h"
#include "arp.h"
#include "itf.h"
#include "io.h"


#define COMPONENT "IO"


struct timeval now;


static int kernel,incoming,inet,unix_sock;


/* ----- kernel interface -------------------------------------------------- */


static void open_kernel(void)
{
    struct sockaddr_atmsvc addr;
    struct atm_qos qos;
    struct atm_sap sap;

    if ((kernel = socket(PF_ATMSVC,SOCK_DGRAM,0)) < 0)
	diag(COMPONENT,DIAG_FATAL,"socket: %s",strerror(errno));
    if (ioctl(kernel,ATMARPD_CTRL,0) < 0)
	diag(COMPONENT,DIAG_FATAL,"ioctl ATMARPD_CTRL: %s",strerror(errno));
    if ((incoming = socket(PF_ATMSVC,SOCK_DGRAM,0)) < 0)
	diag(COMPONENT,DIAG_FATAL,"socket: %s",strerror(errno));
    memset(&qos,0,sizeof(qos));
    qos.aal = ATM_AAL5;
    qos.rxtp.traffic_class = qos.txtp.traffic_class = ATM_ANYCLASS;
    if (setsockopt(incoming,SOL_ATM,SO_ATMQOS,&qos,sizeof(qos)) < 0)
	diag(COMPONENT,DIAG_FATAL,"setsockopt SO_ATMQOS: %s",strerror(errno));
    memset(&sap,0,sizeof(sap));
    sap.blli[0].l2_proto = ATM_L2_ISO8802;
    sap.blli[0].l3_proto = ATM_L3_NONE;
    if (setsockopt(incoming,SOL_ATM,SO_ATMSAP,&sap,sizeof(sap)) < 0)
	diag(COMPONENT,DIAG_FATAL,"setsockopt SO_ATMSAP: %s",strerror(errno));
    memset(&addr,0,sizeof(addr));
    addr.sas_family = AF_ATMSVC;
    if (bind(incoming,(struct sockaddr *) &addr,sizeof(addr)) >= 0) {
	if (listen(incoming,5) < 0)
	    diag(COMPONENT,DIAG_FATAL,"listen: %s",strerror(errno));
    }
    else {
	if (errno != EUNATCH)
	    diag(COMPONENT,DIAG_FATAL,"bind: %s",strerror(errno));
	diag(COMPONENT,DIAG_WARN,"SVCs are not available");
	(void) close(incoming);
	incoming = -1;
    }
    if ((inet = socket(PF_INET,SOCK_DGRAM,0)) < 0)
	diag(COMPONENT,DIAG_FATAL,"socket: %s",strerror(errno));
}


static void recv_kernel(void)
{
    struct atmarp_ctrl ctrl;
    int size;

    size = read(kernel,&ctrl,sizeof(ctrl));
    if (size < 0) {
	diag(COMPONENT,DIAG_ERROR,"read kernel: %s",strerror(errno));
	return;
    }
    switch (ctrl.type) {
	case act_need:
	    need_ip(ctrl.itf_num,ctrl.ip);
	    break;
	case act_up:
	    itf_up(ctrl.itf_num);
	    break;
	case act_down:
	    itf_down(ctrl.itf_num);
	    break;
	case act_change:
	    itf_change(ctrl.itf_num);
	    break;
	default:
	    diag(COMPONENT,DIAG_ERROR,"invalid control msg type 0x%x",
	      ctrl.type);
    }
}


static void close_kernel(void)
{
    if (incoming >= 0) (void) close(incoming);
    (void) close(kernel); /* may get major complaints from the kernel ... */
    (void) close(inet);
}


/* ----- atmarp (maintenance) interface ------------------------------------ */


static void open_unix(void)
{
    unix_sock = un_create(ATMARP_SOCKET_PATH,0600);
    if (unix_sock < 0)
	diag(COMPONENT,DIAG_FATAL,"un_create: %s",strerror(errno));
}


void notify(const UN_CTX *ctx,uint32_t ip,const ENTRY *entry)
{
    struct atmarp_req reply;

    memset(&reply,0,sizeof(reply));
    reply.type = art_query;
    reply.ip = ip;
    if (entry && entry->addr) reply.addr = *entry->addr;
    if (un_send(ctx,&reply,sizeof(reply)) < 0)
        diag(COMPONENT,DIAG_WARN,"notify: %s",strerror(errno));
}


static void recv_unix(void)
{
    UN_CTX ctx;
    struct atmarp_req req;
    int len,reply;

    len = un_recv(&ctx,unix_sock,&req,sizeof(req));
    if (len < 0) {
	diag(COMPONENT,DIAG_ERROR,"recv_unix: %s",strerror(errno));
	return;
    }
    if (len != sizeof(req)) {
	diag(COMPONENT,DIAG_ERROR,"bad unix read: %d != %d",len,sizeof(req));
	return;
    }
    switch (req.type) {
	case art_create:
	    reply = ioctl(kernel,SIOCMKCLIP,req.itf);
	    if (reply >= 0) itf_create(reply);
	    break;
	case art_qos:
	case art_set:
	case art_delete:
	    reply = arp_ioctl(&req);
	    break;
	case art_table:
	    reply = table_update();
	    break;
	case art_query:
	    query_ip(&ctx,req.ip);
	    return;
	default:
	    diag(COMPONENT,DIAG_ERROR,"invalid request msg type 0x%x",req.type);
	    reply = -EINVAL;
    }
    if (un_send(&ctx,&reply,sizeof(reply)) < 0)
	diag(COMPONENT,DIAG_ERROR,"un_send: %s",strerror(errno));
}


static void close_unix(void)
{
    (void) close(unix_sock);
    (void) unlink(ATMARP_SOCKET_PATH);
}


/* ----- common part ------------------------------------------------------- */


#define MAX_BUFFER 1024


static fd_set rset,cset;


int do_close(int fd)
{
    int result;

    result = close(fd);
    FD_CLR(fd,&rset); /* we might open a new fd with the same number, so ... */
    FD_CLR(fd,&cset);
    return result;
}


static void recv_vcc(VCC *vcc)
{
    unsigned char buffer[MAX_BUFFER];
    int size;

    size = read(vcc->fd,buffer,MAX_BUFFER);
    if (!size) {
	disconnect_vcc(vcc);
	return;
    }
    if (size < 0) {
	diag(COMPONENT,DIAG_ERROR,"read vcc: %s",strerror(errno));
	disconnect_vcc(vcc);
	return;
    }
    if (debug) {
	int i;
	for (i = 0; i < size; i++) printf("%02X ",buffer[i]);
	printf("\n");
    }
    incoming_arp(vcc,(struct atmarphdr *) buffer,size);
}


static void drain_vcc(VCC *vcc)
{
    unsigned char buffer[MAX_BUFFER];
    char line[80]; /* actually, it's only 7+16*3+1 */
    int size;
    int i;

    size = read(vcc->fd,buffer,MAX_BUFFER);
    if (!size) {
	disconnect_vcc(vcc);
	return;
    }
    if (size < 0) {
	diag(COMPONENT,DIAG_ERROR,"read vcc: %s",strerror(errno));
	disconnect_vcc(vcc);
	return;
    }
    diag(COMPONENT,DIAG_WARN,"drain_vcc: unexpected message on "
      "unidirectional (RSVP?) VCC %p:",vcc);
    for (i = 0; i < size; i++) {
	if (!(i & 15)) {
	    if (i) diag(COMPONENT,DIAG_WARN,"%s",line);
	    sprintf(line,"  %04x:",i);
	    *line = 0;
	}
	sprintf(strchr(line,0)," %02x",buffer[i]);
    }
    diag(COMPONENT,DIAG_WARN,"%s",line);
}


static void accept_new(void)
{
    char buffer[MAX_ATM_ADDR_LEN+1];
    struct sockaddr_atmsvc addr;
    struct atm_qos qos;
    ENTRY *entry;
    VCC *vcc;
    int fd,error;
    socklen_t len,size;

    len = sizeof(addr);
    if ((fd = accept(incoming,(struct sockaddr *) &addr,&len)) < 0) {
	error = errno;
	diag(COMPONENT,DIAG_ERROR,"accept: %s",strerror(errno));
	if (error == EUNATCH) {
	    diag(COMPONENT,DIAG_WARN,"disabling SVCs");
	    (void) close(incoming);
	    incoming = -1;
	}
	return;
    }
    /* the following code probably belongs to arp.c ... */
    if (atm2text(buffer,MAX_ATM_ADDR_LEN+1,(struct sockaddr *) &addr,pretty) <
      0) strcpy(buffer,"<atm2text error>");
    diag(COMPONENT,DIAG_DEBUG,"Incoming call from %s",buffer);
    size = sizeof(qos);
    if (getsockopt(fd,SOL_ATM,SO_ATMQOS,&qos,&size) < 0)
	diag(COMPONENT,DIAG_FATAL,"getsockopt SO_ATMQOS: %s",strerror(errno));
    if (size != sizeof(qos))
	diag(COMPONENT,DIAG_FATAL,"SO_ATMQOS: size %d != %d",size,sizeof(qos));
    if (ioctl(fd,ATMARP_MKIP,qos.txtp.traffic_class == ATM_NONE ? 0 :
      CLIP_DEFAULT_IDLETIMER) < 0) {
        diag(COMPONENT,DIAG_ERROR,"ioctl ATMARP_MKIP: %s",strerror(errno));
        (void) do_close(fd);
        return;
    }
    vcc = alloc_t(VCC);
    vcc->active = 0;
    vcc->connecting = 0;
    vcc->fd = fd;
    if (qos.txtp.traffic_class == ATM_NONE) {
	vcc->entry = NULL;
	incoming_unidirectional(vcc);
	Q_INSERT_HEAD(unidirectional_vccs,vcc);
	return;
    }
    if (merge) {
	ITF *itf;

	for (itf = itfs; itf; itf = itf->next) {
	    entry = lookup_addr(itf,&addr);
	    if (entry) {
		vcc->entry = entry;
		Q_INSERT_HEAD(entry->vccs,vcc);
		if (entry->state == as_valid) {
		    if (set_ip(vcc->fd,entry->ip) < 0) {
			diag(COMPONENT,DIAG_ERROR,"set_ip: %s",
			  strerror(errno));
			disconnect_vcc(vcc);
		    }
		    else set_sndbuf(vcc);
		}
		return;
	    }
	}
    }
    entry = alloc_entry(1);
    entry->state = as_invalid;
    entry->addr = alloc_t(struct sockaddr_atmsvc);
    *entry->addr = addr;
    entry->flags = ATF_PUBL;
    Q_INSERT_HEAD(unknown_incoming,entry);
    vcc->entry = entry;
    Q_INSERT_HEAD(entry->vccs,vcc);
    incoming_call(vcc);
}


int connect_vcc(struct sockaddr *remote,const struct atm_qos *qos,int sndbuf,
  int timeout)
{
    int fd,error,flags;

    if (remote->sa_family == AF_ATMSVC && incoming < 0) return -EUNATCH;
    if ((fd = socket(remote->sa_family,SOCK_DGRAM,0)) < 0) {
	error = -errno;
	diag(COMPONENT,DIAG_ERROR,"socket: %s",strerror(errno));
	return error;
    }
    if (setsockopt(fd,SOL_ATM,SO_ATMQOS,qos,sizeof(*qos)) < 0) {
	error = -errno;
	diag(COMPONENT,DIAG_ERROR,"setsockopt SO_ATMQOS: %s",strerror(errno));
	return error;
    }
    if (sndbuf)
	if (setsockopt(fd,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf)) < 0) {
	    error = -errno;
	    diag(COMPONENT,DIAG_ERROR,"setsockopt SO_SNDBUF: %s",
	      strerror(errno));
	    return error;
	}
    if ((flags = fcntl(fd,F_GETFL)) < 0) {
	error = -errno;
	diag(COMPONENT,DIAG_ERROR,"fcntl F_GETFL: %s",strerror(errno));
	return error;
    }
    flags |= O_NONBLOCK;
    if (fcntl(fd,F_SETFL,flags) < 0) {
	error = -errno;
	diag(COMPONENT,DIAG_ERROR,"fcntl F_GETFL: %s",strerror(errno));
	return error;
    }
    if (remote->sa_family == AF_ATMSVC) { /* @@@ that's cheating */
	struct atm_sap sap;

	memset(&sap,0,sizeof(sap));
	sap.blli[0].l2_proto = ATM_L2_ISO8802;
	sap.blli[0].l3_proto = ATM_L3_NONE;
	if (setsockopt(fd,SOL_ATM,SO_ATMSAP,&sap,sizeof(sap)) < 0) {
	    error = -errno;
	    diag(COMPONENT,DIAG_ERROR,"setsockopt SO_ATMSAP: %s",
	      strerror(errno));
	    return error;
	}
    }
    /* PVC connect never blocks */
    if (connect(fd,remote,remote->sa_family == AF_ATMPVC ?
      sizeof(struct sockaddr_atmpvc) : sizeof(struct sockaddr_atmsvc)) < 0) {
	if (errno != EINPROGRESS) {
	    error = -errno;
	    diag(COMPONENT,DIAG_ERROR,"[1]connect: %s",strerror(errno));
	    return error;
	}
	return fd;
    }
    if (ioctl(fd,ATMARP_MKIP,timeout) < 0) {
	error = -errno;
        diag(COMPONENT,DIAG_ERROR,"ioctl ATMARP_MKIP: %s",strerror(errno));
        (void) do_close(fd);
        return error;
    }
    return fd;
}


int set_ip(int fd,int ip)
{
    int error;

    if (ioctl(fd,ATMARP_SETENTRY,ip) >= 0) return 0;
    error = -errno;
    diag(COMPONENT,DIAG_ERROR,"ioctl ATMARP_SETENTRY: %s",strerror(errno));
    return error;
}


int set_encap(int fd,int mode)
{
    int error;

    if (ioctl(fd,ATMARP_ENCAP,mode) >= 0) return 0;
    error = -errno;
    diag(COMPONENT,DIAG_ERROR,"ioctl ATMARP_ENCAP: %s",strerror(errno));
    (void) do_close(fd);
    return error;
}


void set_sndbuf(VCC *vcc)
{
    if (setsockopt(vcc->fd,SOL_SOCKET,SO_SNDBUF,&vcc->entry->sndbuf,
      sizeof(int)) >= 0)
	return;
    diag(COMPONENT,DIAG_ERROR,"setsockopt SO_SNDBUF: %s",strerror(errno));
}


static void complete_connect(VCC *vcc)
{
    struct sockaddr_atmsvc dummy;

    if (!vcc->connecting)
	diag(COMPONENT,DIAG_FATAL,"connecting non-connecting VCC 0x%p",vcc);
    memset(&dummy,0,sizeof(dummy));
    if (!connect(vcc->fd,(struct sockaddr *) &dummy,sizeof(dummy))) {
	if (ioctl(vcc->fd,ATMARP_MKIP,CLIP_DEFAULT_IDLETIMER) < 0) {
	    diag(COMPONENT,DIAG_ERROR,"ioctl ATMARP_MKIP: %s",strerror(errno));
	    (void) do_close(vcc->fd);
	    vcc_failed(vcc);
	}
	vcc_connected(vcc);
    }
    else {
	diag(COMPONENT,DIAG_INFO,"[2]connect: %s",strerror(errno));
	(void) do_close(vcc->fd);
	vcc_failed(vcc);
    }
}


void poll_loop(void)
{
    ITF *itf,*next_itf;
    ENTRY *entry,*next_entry;
    VCC *vcc,*next_vcc;
    int fds,ret;

    gettimeofday(&now,NULL);
    while (1) {
	FD_ZERO(&rset);
	FD_ZERO(&cset);
	FD_SET(kernel,&rset);
	FD_SET(unix_sock,&rset);
	if (incoming >= 0) FD_SET(incoming,&rset);
	fds = incoming+1;
	if (kernel >= fds) fds = kernel+1;
	if (unix_sock >= fds) fds = unix_sock+1;
	for (itf = itfs; itf; itf = itf->next)
	    for (entry = itf->table; entry; entry = entry->next)
		for (vcc = entry->vccs; vcc; vcc = vcc->next) {
		    if (vcc->connecting) FD_SET(vcc->fd,&cset);
		    else FD_SET(vcc->fd,&rset);
		    if (vcc->fd >= fds) fds = vcc->fd+1;
		}
	for (entry = unknown_incoming; entry; entry = entry->next) {
	    if (!entry->vccs || entry->vccs->next) {
		diag(COMPONENT,DIAG_ERROR,"internal error: bad unknown entry");
		continue;
	    }
	    FD_SET(entry->vccs->fd,&rset);
	    if (entry->vccs->fd >= fds) fds = entry->vccs->fd+1;
	}
	for (vcc = unidirectional_vccs; vcc; vcc = vcc->next) {
	    FD_SET(vcc->fd,&rset);
	    if (vcc->fd >= fds) fds = vcc->fd+1;
	}
	ret = select(fds,&rset,&cset,NULL,next_timer());
/*
 * Now here's something strange: < 0.32 needed the exception mask to be NULL
 * in order to work, due to a bug in atm_select. In 0.32, this has been fixed.
 * Also, 2.1 kernels use the poll mechanism and not select, so select is
 * emulated on top of poll. Now the funny bit is that, as soon as the exception
 * set is non-NULL, when a non-blocking connect finishes, select returns one
 * but has none if the possible bits set in either rset or cset. To make things
 * even stranger, no exception is actually found in sys_select, so this must be
 * some very odd side-effect ... The work-around for now is to simply pass NULL
 * for the exception mask (which is the right thing to do anyway, but it'd be
 * nice if doing a perfectly valid variation wouldn't blow up the system ...)
 */
#if 0
{
  int i;
  for (i = 0; i < sizeof(rset); i++)
   fprintf(stderr,"%02x:%02x ",((unsigned char *) &rset)[i],
    ((unsigned char *) &cset)[i]);
  fprintf(stderr,"\n");
}
#endif
	if (ret < 0) {
	    if (errno != EINTR) perror("select");
	}
	else {
	    diag(COMPONENT,DIAG_DEBUG,"----------");
	    gettimeofday(&now,NULL);
	    if (FD_ISSET(kernel,&rset)) recv_kernel();
	    if (FD_ISSET(unix_sock,&rset)) recv_unix();
	    if (incoming >= 0 && FD_ISSET(incoming,&rset)) accept_new();
	    for (itf = itfs; itf; itf = next_itf) {
		next_itf = itf->next;
		for (entry = itf->table; entry; entry = next_entry) {
		    next_entry = entry->next;
		    for (vcc = entry->vccs; vcc; vcc = next_vcc) {
			next_vcc = vcc->next;
			if (FD_ISSET(vcc->fd,&rset)) recv_vcc(vcc);
			else if (FD_ISSET(vcc->fd,&cset))
				complete_connect(vcc);
		    }
		}
	    }
	    for (entry = unknown_incoming; entry; entry = next_entry) {
		next_entry = entry->next;
		if (FD_ISSET(entry->vccs->fd,&rset)) recv_vcc(entry->vccs);
	    }
	    for (vcc = unidirectional_vccs; vcc; vcc = next_vcc) {
		next_vcc = vcc->next;
		if (FD_ISSET(vcc->fd,&rset)) drain_vcc(vcc);
	    }
	    expire_timers();
	      /* expire timers after handling messages to make sure we don't
		 time out unnecessarily because of scheduling delays */
	}
	table_changed();
    }
}


void send_packet(int fd,void *data,int length)
{
    int wrote;

    if (debug) {
	int i;
	for (i = 0; i < length; i++)
	    printf("%02X ",((unsigned char *) data)[i]);
	printf("\n");
    }
    if ((wrote = write(fd,data,length)) == length) return;
    if (wrote < 0)
	diag(COMPONENT,DIAG_ERROR,"write: %s",strerror(errno));
    else diag(COMPONENT,DIAG_ERROR,"short write: %d < %d",wrote,length);
}


int ip_itf_info(int number,uint32_t *ip,uint32_t *netmask,int *mtu)
{
    struct ifreq req;
    unsigned char *p1,*p2;

    sprintf(req.ifr_ifrn.ifrn_name,"atm%d",number);
    if (ioctl(inet,SIOCGIFADDR,&req) < 0) {
	diag(COMPONENT,DIAG_ERROR,"ioctl SIOCGIFADDR: %s",strerror(errno));
	return -1;
    }
    *ip = ((struct sockaddr_in *) &req.ifr_ifru.ifru_addr)->sin_addr.s_addr;
    if (ioctl(inet,SIOCGIFNETMASK,&req) < 0) {
	diag(COMPONENT,DIAG_ERROR,"ioctl SIOCGIFNETMASK: %s",strerror(errno));
	return -1;
    }
    *netmask = ((struct sockaddr_in *) &req.ifr_ifru.ifru_netmask)->
      sin_addr.s_addr;
    if (ioctl(inet,SIOCGIFMTU,&req) < 0) {
	diag(COMPONENT,DIAG_ERROR,"ioctl SIOCGIFMTU: %s",strerror(errno));
	return -1;
    }
    *mtu = req.ifr_ifru.ifru_mtu;
    p1 = (unsigned char *) ip;
    p2 = (unsigned char *) netmask;
    diag(COMPONENT,DIAG_DEBUG,"ip %d.%d.%d.%d mask %d.%d.%d.%d mtu %d",
      p1[0],p1[1],p1[2],p1[3],p2[0],p2[1],p2[2],p2[3],*mtu);
    return 0;
}


int get_local(int fd,struct sockaddr_atmsvc *addr)
{
    int result;
    size_t length;

    length = sizeof(struct sockaddr_atmsvc);
    result = getsockname(fd,(struct sockaddr *) addr,&length);
    if (result < 0)
	diag(COMPONENT,DIAG_ERROR,"getsockname: %s",strerror(errno));
    return result;
}


void open_all(void)
{
    open_kernel();
    open_unix();
}


void close_all(void)
{
    close_kernel();
    close_unix();
}
