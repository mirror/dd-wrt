/* arp.c - ARP state machine */
 
/* Written 1995-2000 by Werner Almesberger, EPFL-LRC/ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h> /* for linux/if_arp.h */
#include <netinet/in.h> /* for ntohs, etc. */
#define _LINUX_NETDEVICE_H /* very crude hack for glibc2 */
#include <linux/types.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <atm.h>
#include <linux/atmarp.h>
#include <linux/atmclip.h>

#include "atmd.h"
#include "atmarp.h"
#include "atmarpd.h"

#include "table.h"
#include "itf.h"
#include "io.h"
#include "arp.h"


#define COMPONENT "ARP"


#define SEC	 1000000
#define T_REPLY	    (1*SEC)	/* 1 sec */
#define R_REPLY	    5		/* 5 retries */
#ifdef FREQUENT_REVAL
#define T_CREVAL    (30*SEC)	/* 30 sec, client */
#define T_SREVAL    (35*SEC)	/* 35 sec, server */
#else
#define T_CREVAL   (600*SEC)	/* 10 min, client (RFC1577: <= 15 min) */
#define T_SREVAL  (1500*SEC)	/* 25 min, server (RFC1577: >= 20 min) */
#endif
#define R_CREVAL    0		/* no retries */
#define R_SREVAL    0		/* no retries */
#define T_RETRY	    (1*SEC)	/* 1 sec */
#define R_RETRY	    25000	/* enough try keep trying during a long
				   weekend */
#define T_REGISTER  (5*SEC)	/* 5 sec, registration with ATMARP server */
#define R_REGISTER  0		/* no explicit retries */

#define MAX_DELAY   (300*SEC)	/* never wait more than 5 min for anything */


#define ENTER_TIMER_HANDLER(entry) entry->timer = NULL
#define START_TIMER(entry,tmr) ({ \
  if (entry->timer) stop_timer(entry->timer); \
  entry->timeout = T_##tmr; \
  entry->timer = start_timer(entry->timeout,timer_expiration,entry); \
  entry->retries = R_##tmr; })
#define STOP_TIMER(entry) ({ \
  if (entry->timer) { \
    stop_timer(entry->timer); \
    entry->timer = NULL; \
  } })


static void discard_vcc(VCC *vcc)
{
    if (do_close(vcc->fd))
	diag(COMPONENT,DIAG_ERROR,"close: %s",strerror(errno));
    if (vcc->entry) Q_REMOVE(vcc->entry->vccs,vcc);
    else Q_REMOVE(unidirectional_vccs,vcc);
    free(vcc);
}


void discard_vccs(ENTRY *entry)
{
    VCC *vcc,*next;

    for (vcc = entry->vccs; vcc; vcc = next) {
	next = vcc->next;
	discard_vcc(vcc);
    }
}


void discard_entry(ENTRY *entry)
{
    if (entry->notify)
	diag(COMPONENT,DIAG_FATAL,"discard_entry: entry %p had notifications");
    STOP_TIMER(entry);
    discard_vccs(entry);
    if (!entry->itf) Q_REMOVE(unknown_incoming,entry);
    else {
	if (entry == entry->itf->arp_srv) entry->itf->arp_srv = NULL;
	Q_REMOVE(entry->itf->table,entry);
    }
    if (entry->addr) free(entry->addr);
    free(entry);
}


/*
 * Returns:
 *  <0  there's no ARP server
 *   0  ARP server connection establishment in progress
 *  >0  ready to send ARP_REQUEST
 */


static int want_arp_srv(const ITF *itf)
{
    VCC *vcc;
    int fd;

    if (!itf->arp_srv) return -1;
    for (vcc = itf->arp_srv->vccs; vcc; vcc = vcc->next)
	if (!vcc->connecting) return 1;
    if (itf->arp_srv->vccs) return 0;
    if ((fd = connect_vcc((struct sockaddr *) itf->arp_srv->addr,
      &itf->arp_srv->qos,itf->arp_srv->sndbuf,CLIP_DEFAULT_IDLETIMER)) < 0)
	return 0;
    vcc = alloc_t(VCC);
    vcc->active = 1;
    vcc->connecting = 1;
    vcc->fd = fd;
    vcc->entry = itf->arp_srv;
    Q_INSERT_HEAD(itf->arp_srv->vccs,vcc);
    return 0;
}


static void timer_expiration(void *user);


void vcc_detach(ENTRY *entry)
{
    ENTRY *walk;

    /* immediately try to bring it up again if this was the connection to the
       ATMARP server and we still need it. Should delay this in case the ARP
       server has a real problem. @@@ */
    /* UPDATE: the delay might work now ... */
    if (entry->itf && entry->itf->arp_srv == entry)
	for (walk = entry->itf->table; walk; walk = walk->next) {
	    if (walk->state == as_resolv) break;
	if (walk) {
	    if (entry->state == as_valid) START_TIMER(entry,REGISTER);
	    /* (void) want_arp_srv(entry->itf); */
	    return;
	}
    }
    if (!entry->vccs && entry->state == as_invalid && !(entry->flags &
      ATF_PERM)) discard_entry(entry);
}


static void put_ip(unsigned char **here,uint32_t ip,unsigned char *len)
{
    if (!ip) {
	*len = 0;
	return;
    }
    /*ip = htonl(ip); - no keep 'em in network byte order */
    memcpy(*here,&ip,4);
    *len = 4;
    (*here) += 4;
}


static void put_addr(unsigned char **here,struct sockaddr_atmsvc *addr,
  unsigned char *num_tl,unsigned char *sub_tl)
{
    if (!addr) return;
    if (addr->sas_family != AF_ATMSVC) return;
    if (!*addr->sas_addr.pub) *sub_tl = 0;
    else {
	*num_tl = strlen(addr->sas_addr.pub) | TL_E164;
	memcpy(*here,addr->sas_addr.pub,*num_tl & TL_LEN);
	(*here) += *num_tl & TL_LEN;
	num_tl = sub_tl;
    }
    if (!*addr->sas_addr.prv) *num_tl = 0;
    else {
	*num_tl = ATM_ESA_LEN;
	memcpy(*here,addr->sas_addr.prv,ATM_ESA_LEN);
	(*here) += ATM_ESA_LEN;
    }
}


static const unsigned char llc_oui_arp[] = {
	0xaa,	/* DSAP: non-ISO */
	0xaa,	/* SSAP: non-ISO */
	0x03,	/* Ctrl: Unnumbered Information Command PDU */
	0x00,	/* OUI: EtherType */
	0x00,
	0x00,
	0x08,	/* ARP protocol */
	0x06 };


static void send_arp(int fd,unsigned short op,uint32_t local_ip,
  struct sockaddr_atmsvc *local_addr,uint32_t remote_ip,
  struct sockaddr_atmsvc *remote_addr)
{
    struct sockaddr_atmsvc local;
    struct atmarphdr *hdr;
    unsigned char *buffer,*here;

    if (!local_addr) {
	if (get_local(fd,&local) < 0) return;
	local_addr = &local;
    }
    buffer = alloc(MAX_ATMARP_SIZE+RFC1483LLC_LEN);
    memcpy(buffer,llc_oui_arp,RFC1483LLC_LEN);
    hdr = (struct atmarphdr *) (buffer+RFC1483LLC_LEN);
    memset(hdr,0,MAX_ATMARP_SIZE);
    hdr->ar_hrd = htons(ARPHRD_ATM);
    hdr->ar_pro = htons(ETH_P_IP);
    hdr->ar_op = htons(op);
    here = hdr->data;
    put_addr(&here,local_addr,&hdr->ar_shtl,&hdr->ar_sstl);
    put_ip(&here,local_ip,&hdr->ar_spln);
    put_addr(&here,remote_addr,&hdr->ar_thtl,&hdr->ar_tstl);
    put_ip(&here,remote_ip,&hdr->ar_tpln);
    send_packet(fd,buffer,here-buffer);
    free(buffer);
}


static void arp_request(ITF *itf,uint32_t ip)
{
    VCC *vcc;

    diag(COMPONENT,DIAG_DEBUG,"sending ARP request");
    if (!itf->arp_srv) {
	diag(COMPONENT,DIAG_ERROR,"no ARP server");
	return;
    }
    for (vcc = itf->arp_srv->vccs; vcc; vcc = vcc->next)
	if (!vcc->connecting) break;
    if (!vcc) {
	diag(COMPONENT,DIAG_ERROR,"ARP server has no usable VCC");
	return;
    }
    send_arp(vcc->fd,ARPOP_REQUEST,itf->local_ip,NULL,ip,NULL);
}


static void inarp_for_itf(const VCC *vcc,const ITF *itf)
{
    if (itf->local_ip) {
	diag(COMPONENT,DIAG_DEBUG,"  for itf %d",itf->number);
	assert(vcc->entry);
	send_arp(vcc->fd,ARPOP_InREQUEST,itf->local_ip,NULL,0,
	  vcc->entry->svc ? vcc->entry->addr : NULL);
    }
}


static void inarp_request(ENTRY *entry)
{
    VCC *vcc;
    ITF *itf;

    diag(COMPONENT,DIAG_DEBUG,"sending an InARP request for each IP interface");
    for (vcc = entry->vccs; vcc; vcc = vcc->next) {
	if (vcc->connecting) continue;
	if (entry->itf) inarp_for_itf(vcc,entry->itf);
	else for (itf = itfs; itf; itf = itf->next) inarp_for_itf(vcc,itf);
    }
}


static void arp_nak(VCC *vcc,uint32_t src_ip,uint32_t tgt_ip,
  struct sockaddr_atmsvc *tgt_addr)
{
    diag(COMPONENT,DIAG_DEBUG,"sending ARP NAK");
    send_arp(vcc->fd,ARPOP_NAK,tgt_ip,tgt_addr,src_ip,NULL);
}


static void arp_reply(VCC *vcc,uint32_t src_ip,
  struct sockaddr_atmsvc *src_addr,uint32_t tgt_ip,
  struct sockaddr_atmsvc *tgt_addr)
{
    diag(COMPONENT,DIAG_DEBUG,"sending ARP reply");
    send_arp(vcc->fd,ARPOP_REPLY,src_ip,src_addr,tgt_ip,tgt_addr);
}


static void inarp_reply(VCC *vcc,uint32_t ip,struct sockaddr_atmsvc *addr)
{
    ITF *itf;

    diag(COMPONENT,DIAG_DEBUG,"sending InARP reply");
    itf = lookup_itf_by_ip(ip);
    if (!itf) {
	diag(COMPONENT,DIAG_ERROR,"InARP request is unroutable");
	return;
    }
    send_arp(vcc->fd,ARPOP_InREPLY,itf->local_ip,NULL,ip,addr);
}


static void send_notifications(ENTRY *entry,int success)
{
    NOTIFY *this;

    while ((this = entry->notify)) {
	entry->notify = this->next;
	notify(&this->ctx,entry->ip,success ? entry : NULL);
	free(this);
    }
}


static void revalidate(ENTRY *entry)
{
    entry->state = as_resolv;
    if (want_arp_srv(entry->itf) <= 0) return;
    arp_request(entry->itf,entry->ip);
    START_TIMER(entry,REPLY);
}


static void timer(ENTRY *entry)
{
    switch (entry->state) {
	case as_resolv:
	    arp_request(entry->itf,entry->ip);
	    break;
	case as_invalid:
	    if (entry->vccs) inarp_request(entry);
	    else if (!(entry->flags & ATF_ARPSRV))
		    diag(COMPONENT,DIAG_INFO,"as_invalid, no VCCs, but not "
		      "ARP server entry");
			/* this is a valid condition if we're the ATMARP server
			   and the remote disconnected */
		    else if (!want_arp_srv(entry->itf)) STOP_TIMER(entry);
	    break;
	case as_valid:
	    if (entry->flags & ATF_ARPSRV) {
		if (!want_arp_srv(entry->itf)) STOP_TIMER(entry);
		break;
	    }
	    /* fall through */
	default:
	    diag(COMPONENT,DIAG_FATAL,"timer in state %d",entry->state);
    }
}


static void timeout(ENTRY *entry)
{
    VCC *vcc,*next;

    entry->timer = NULL;
    switch (entry->state) {
	case as_resolv:
	    send_notifications(entry,0);
	    if ((entry->flags & ATF_ARPSRV) && !entry->vccs) {
		if (entry->itf) want_arp_srv(entry->itf);
		break;
	    }
	    if (!entry->vccs && !(entry->flags & (ATF_PERM | ATF_ARPSRV)))
		discard_entry(entry);
	    else entry->state = as_invalid;
	    break;
	case as_valid:
	    if (!entry->vccs && !(entry->flags & (ATF_PERM | ATF_ARPSRV)) &&
	      entry->itf->arp_srv) {
		discard_entry(entry);
		break;
	    }
	    for (vcc = entry->vccs; vcc; vcc = next) {
		next = vcc->next;
		if (!vcc->connecting)
		    if (set_ip(vcc->fd,0) < 0) {
			diag(COMPONENT,DIAG_ERROR,"set_ip(0): %s",
			  strerror(errno));
			disconnect_vcc(vcc);
		    }
	    }
	    if (entry->svc && entry->itf->arp_srv &&
	      !(entry->flags & ATF_ARPSRV)) revalidate(entry);
	    else {
		inarp_request(entry);
		START_TIMER(entry,REPLY);
		entry->state = as_invalid;
	    }
	    break;
	case as_invalid:
	    if (!entry->svc) {
		inarp_request(entry);
		START_TIMER(entry,REPLY);
	    }
	    else if ((!entry->itf || !entry->itf->arp_srv) &&
		  !(entry->flags & ATF_PERM)) discard_entry(entry);
	    break;
	default:
	    diag(COMPONENT,DIAG_FATAL,"timed out in state %s",
	      entry_state_name[entry->state]);
    }
}


static void timer_expiration(void *user)
{
    ENTRY *entry;
    unsigned char *ipp;

    entry = user;
    ipp = (unsigned char *) &entry->ip;
    diag(COMPONENT,DIAG_DEBUG,"TIMEOUT (entry %d.%d.%d.%d), %d retries",ipp[0],
      ipp[1],ipp[2],ipp[3],entry->retries);
    if (!entry->retries) {
	entry->timer = NULL;
	timeout(entry);
    }
    else {
	entry->retries--;
	entry->timeout *= 2;
	if (entry->timeout > MAX_DELAY) entry->timeout = MAX_DELAY;
	entry->timer = start_timer(entry->timeout,timer_expiration,entry);
	timer(entry);
    }
}


static void connect_me(ENTRY *entry)
{
    VCC *vcc;
    int fd;

    assert(entry->addr->sas_family == AF_ATMSVC);
    if ((fd = connect_vcc((struct sockaddr *) entry->addr,&entry->qos,
      entry->sndbuf,CLIP_DEFAULT_IDLETIMER)) < 0) return;
    vcc = alloc_t(VCC);
    vcc->active = 1;
    vcc->connecting = 1;
    vcc->fd = fd;
    vcc->entry = entry;
    Q_INSERT_HEAD(entry->vccs,vcc);
}


static void adjust_qos(ITF *itf,struct atm_qos *qos,int null_encap)
{
    if (!qos->txtp.max_sdu)
	qos->txtp.max_sdu = RFC1626_MTU+(null_encap ? 0 : RFC1483LLC_LEN);
    if (!qos->rxtp.max_sdu)
	qos->rxtp.max_sdu = RFC1626_MTU+(null_encap ? 0 : RFC1483LLC_LEN);
}


/*
 * Return -1 if VCC has been closed
 */

static int learn(VCC *vcc,uint32_t ip,struct sockaddr_atmsvc *addr)
{
    ENTRY *entry;
    ITF *itf;
    VCC *walk,*next;
    unsigned char *ipp;
    int result = 0;

    if (!ip) return 0;
    ipp = (unsigned char *) &ip;
    itf = lookup_itf_by_ip(ip);
    if (!itf) {
	diag(COMPONENT,DIAG_ERROR,"got unroutable IP address %d.%d.%d.%d",
	  ipp[0],ipp[1],ipp[2],ipp[3]);
	return 0;
    }
    entry = lookup_ip(itf,ip);
    assert(!vcc || vcc->entry);
    /*
     * If the entry on which we received the update isn't dangling but it
     * doesn't correspond to the one with the address, ...
     */
    if (entry && vcc && vcc->entry->itf && entry != vcc->entry) {
	diag(COMPONENT,DIAG_DEBUG,"collision on %d.%d.%d.%d",ipp[0],ipp[1],
	  ipp[2],ipp[3]);
	return 0;
    }
    /*
     * If the entry on which we received the update is dangling and we found
     * an entry that already describes that IP address, ...
     */
    if (entry && vcc && !vcc->entry->itf) {
	if (!entry->svc) {
	    diag(COMPONENT,DIAG_ERROR,"attempt to overwrite PVC for IP "
	      "%d.%d.%d.%d",ipp[0],ipp[1],ipp[2],ipp[3]);
	    return 0;
	}
	STOP_TIMER(vcc->entry);
	Q_REMOVE(unknown_incoming,vcc->entry);
	free(vcc->entry);
	vcc->entry = entry;
	Q_INSERT_HEAD(entry->vccs,vcc);
	set_sndbuf(vcc);
	entry->flags &= ~ATF_NOVC;
	assert(!vcc->connecting);
	if (set_ip(vcc->fd,ip) < 0) {
	    diag(COMPONENT,DIAG_ERROR,"set_ip: %s",strerror(errno));
	    disconnect_vcc(vcc);
	    vcc = NULL;
	    result = -1;
	}
    }
    /*
     * If we still don't have an entry, we try to use the entry that already
     * belongs to the VCC (InARP), or we create a new one (ARP).
     */
    if (!entry) {
	if (vcc) {
	    entry = vcc->entry;
	    if (!entry->itf) {
		Q_REMOVE(unknown_incoming,entry);
		entry->sndbuf = itf->sndbuf;
		set_sndbuf(vcc);
	    }
	    else if (entry->ip && entry->ip != ip && (entry->flags & ATF_PERM)
		  && !(entry->flags & ATF_ARPSRV)) {
		    diag(COMPONENT,DIAG_ERROR,"ignoring attempt to change IP "
		      "address of permanent entry (to %d.%d.%d.%d)",ipp[0],
		      ipp[1],ipp[2],ipp[3]);
		    return result;
		}
	}
	else {
	    entry = alloc_entry(1);
	    entry->flags = ATF_PUBL;
	}
    }
    if (!atmsvc_addr_in_use(*addr)) addr = NULL;
    if (entry->addr && addr && (entry->flags & ATF_PERM) &&
      !atm_equal((struct sockaddr *) entry->addr,(struct sockaddr *) addr,0,0))
      {
	diag(COMPONENT,DIAG_ERROR,"ignoring attempt to change ATM address of "
	  "permanent entry");
	return result;
    }
    if (entry->state == as_valid && entry->ip == ip && (!addr || (entry->addr
      && atm_equal((struct sockaddr *) entry->addr,(struct sockaddr *) addr,0,
      0)))) return result; /* no news */
    STOP_TIMER(entry);
    if (entry->ip != ip) send_notifications(entry,0);
    entry->ip = ip;
    if (!entry->itf) {
	entry->itf = itf;
	/* @@@
	 * Need to fix this is in case we allow entries without a valid IP
	 * address but with a pre-set QOS, e.g. a VC on a given PVC with an
	 * unknown remote end.
	 */
	entry->qos = entry->itf->qos;
	adjust_qos(entry->itf,&entry->qos,0);
	Q_INSERT_HEAD(itf->table,entry);
    }
    if (entry->itf != itf)
	diag(COMPONENT,DIAG_ERROR,"interesting, interface has changed ... "
	  "(%d -> %d)",entry->itf->number,itf->number);
    if (addr) {
	if (!entry->addr) entry->addr = alloc(sizeof(*addr));
	*entry->addr = *addr;
	if (merge) {
	    ENTRY *incoming;

	    while ((incoming = lookup_incoming(addr))) {
		STOP_TIMER(incoming);
		Q_REMOVE(unknown_incoming,incoming);
		incoming->vccs->entry = entry;
		Q_INSERT_HEAD(entry->vccs,incoming->vccs);
		set_sndbuf(incoming->vccs);
		free(incoming);
	    }
	}
    }
    for (walk = entry->vccs; walk; walk = next) {
	next = walk->next;
	if (!walk->connecting)
	    if (set_ip(walk->fd,ip) < 0) {
		diag(COMPONENT,DIAG_ERROR,"set_ip: %s",strerror(errno));
		disconnect_vcc(walk);
		if (walk == vcc) result = -1;
	    }
    }
    if (entry->state != as_valid) {
	if (!entry->vccs && itf->arp_srv && !(entry->flags & ATF_NOVC))
	    connect_me(entry);
	send_notifications(entry,1);
    }
    if ((entry->flags & ATF_ARPSRV) || !(entry->flags & ATF_PERM)) {
	if (entry->itf->arp_srv) START_TIMER(entry,CREVAL);
	else START_TIMER(entry,SREVAL);
    }
    entry->state = as_valid;
    return result;
}


static void learn_nak(uint32_t ip)
{
    ITF *itf;
    ENTRY *entry;

    if (!ip) return;
    itf = lookup_itf_by_ip(ip);
    if (!itf) return;
    entry = lookup_ip(itf,ip);
    if (!entry || entry->state != as_resolv) return;
    send_notifications(entry,0);
    if (entry->flags & ATF_PERM) return;
    if (entry->vccs) entry->state = as_invalid;
    else discard_entry(entry);
}


/*
 * Returns:
 *  <0  resolution is not possible (*entry may be clobbered)
 *   0  resolution has succeeded (entry in *entry)
 *  >0  resolution is proceeding (entry in *entry)
 */

static int resolve(ITF *itf,uint32_t ip,ENTRY **entry,int want_vc)
{
    *entry = lookup_ip(itf,ip);
    if ((!*entry || (*entry)->state != as_valid) && !itf->arp_srv)
	return -1; /* bad luck - no ARP server when we need one */
    if (*entry) {
	if (want_vc) (*entry)->flags &= ~ATF_NOVC;
	switch ((*entry)->state) {
	    case as_resolv:
		return 1; /* somebody else is already taking care of that */
	    case as_valid:
		if (!(*entry)->vccs && !((*entry)->flags & ATF_NOVC))
		    connect_me(*entry);
		return 0;
	    case as_invalid:
		if ((*entry)->svc && (*entry)->itf && (*entry)->itf->arp_srv &&
		    !((*entry)->flags & ATF_ARPSRV)) break;
		return -1;
	    default:
		diag(COMPONENT,DIAG_FATAL,"bad state %d",(*entry)->state);
	}
    }
    else {
	*entry = alloc_entry(1);
	(*entry)->flags = ATF_PUBL | (want_vc ? 0 : ATF_NOVC);
	(*entry)->ip = ip;
	(*entry)->itf = itf;
	Q_INSERT_HEAD(itf->table,*entry);
	(*entry)->qos = itf->qos;
    }
    revalidate(*entry);
    return 1;
}


void need_ip(int itf_num,uint32_t ip)
{
    ITF *itf;
    ENTRY *entry;

    diag(COMPONENT,DIAG_DEBUG,"itf %d needs %d.%d.%d.%d",itf_num,
      ((unsigned char *) &ip)[0],((unsigned char *) &ip)[1],
      ((unsigned char *) &ip)[2],((unsigned char *) &ip)[3]);
    if (!(itf = lookup_itf(itf_num))) {
	diag(COMPONENT,DIAG_ERROR,"itf %d not found",itf_num);
	return;
    }
    (void) resolve(itf,ip,&entry,1);
}


void query_ip(const UN_CTX *ctx,uint32_t ip)
{
    ITF *itf;
    ENTRY *entry;
    NOTIFY *notifier;
    int result;

    diag(COMPONENT,DIAG_DEBUG,"query for %d.%d.%d.%d",
      ((unsigned char *) &ip)[0],((unsigned char *) &ip)[1],
      ((unsigned char *) &ip)[2],((unsigned char *) &ip)[3]);
    if (!(itf = lookup_itf_by_ip(ip))) {
	diag(COMPONENT,DIAG_WARN,"itf for %d.%d.%d.%d not found",
	    ((unsigned char *) &ip)[0],((unsigned char *) &ip)[1],
	    ((unsigned char *) &ip)[2],((unsigned char *) &ip)[3]);
	notify(ctx,ip,NULL);
	return;
    }
    result = resolve(itf,ip,&entry,0);
    if (result < 0) {
	notify(ctx,ip,NULL);
	return;
    }
    if (!result) {
	notify(ctx,ip,entry);
	return;
    }
    notifier = alloc_t(NOTIFY);
    notifier->ctx = *ctx;
    notifier->next = entry->notify;
    entry->notify = notifier;
}


static void *get_addr(unsigned char **here,int len)
{
    if (!len) return NULL;
    (*here) += len;
    return *here-len;
}


static void set_addr(struct sockaddr_atmsvc *addr,void *num,void *sub,
  int num_tl,int sub_tl)
{
    memset(addr,0,sizeof(struct sockaddr_atmsvc));
    addr->sas_family = AF_ATMSVC;
    if (num_tl & TL_E164)
	if ((num_tl & TL_LEN) > ATM_E164_LEN)
	    diag(COMPONENT,DIAG_ERROR,"bad E.164 length (%d)",num_tl &
	      TL_LEN);
	else {
	    memcpy(addr->sas_addr.pub,num,num_tl & TL_LEN);
	    if (sub) {
		if (sub_tl != ATM_ESA_LEN) {
		    diag(COMPONENT,DIAG_ERROR,"bad ESA length (%d)",sub_tl);
		    *addr->sas_addr.pub = 0;
		}
		else memcpy(addr->sas_addr.prv,sub,ATM_ESA_LEN);
	    }
	}
    else if (num) {
	    if (num_tl != ATM_ESA_LEN)
		diag(COMPONENT,DIAG_ERROR,"bad ESA length (%d)",num_tl);
	    else memcpy(addr->sas_addr.prv,num,ATM_ESA_LEN);
	}
}


static uint32_t get_ip(unsigned char *ptr)
{
    if (!ptr) return 0;
    /* awkward, but this way we avoid bus errors on architectures that
       don't support mis-aligned accesses */
    return htonl((ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3]);
}


void incoming_arp(VCC *vcc,struct atmarphdr *hdr,int len)
{
    ITF *itf;
    ENTRY *entry;
    void *sha,*ssa,*spa,*tha,*tsa,*tpa;
    struct sockaddr_atmsvc source,target;
    uint32_t src_ip,tgt_ip;
    unsigned char *here;

    if (len < hdr->data-(unsigned char *) hdr) {
	diag(COMPONENT,DIAG_ERROR,"got truncated ARP packet (%d bytes)",len);
	return;
    }
    if (hdr->ar_hrd != htons(ARPHRD_ATM)) {
	diag(COMPONENT,DIAG_ERROR,"unknown hw protocol 0x%04x",
	  ntohs(hdr->ar_hrd));
	return;
    }
    if (hdr->ar_pro != htons(ETH_P_IP)) {
	diag(COMPONENT,DIAG_ERROR,"unknown upper protocol 0x%04x",
	  ntohs(hdr->ar_pro));
	return;
    }
    if (!(hdr->ar_shtl & TL_LEN)) hdr->ar_shtl = 0; /* paranoia */
    if (!(hdr->ar_thtl & TL_LEN)) hdr->ar_thtl = 0;
    here = hdr->data;
    sha = get_addr(&here,hdr->ar_shtl & TL_LEN);
    ssa = get_addr(&here,hdr->ar_sstl & TL_LEN);
    spa = get_addr(&here,hdr->ar_spln);
    tha = get_addr(&here,hdr->ar_thtl & TL_LEN);
    tsa = get_addr(&here,hdr->ar_tstl & TL_LEN);
    tpa = get_addr(&here,hdr->ar_tpln);
    if (here-(unsigned char *) hdr > len) {
	diag(COMPONENT,DIAG_ERROR,"message too short (got %d, need %d)",len,
	  here-(unsigned char *) hdr);
	return;
    }
    set_addr(&source,sha,ssa,hdr->ar_shtl,hdr->ar_sstl);
    set_addr(&target,tha,tsa,hdr->ar_thtl,hdr->ar_tstl);
    src_ip = get_ip(spa);
    tgt_ip = get_ip(tpa);
{
   unsigned char *ipp;
   char buffer[MAX_ATM_ADDR_LEN+1];

   ipp = (unsigned char *) &src_ip;
   diag(COMPONENT,DIAG_DEBUG,"  SRC IP: %d.%d.%d.%d",ipp[0],ipp[1],ipp[2],
     ipp[3]);
   if (atm2text(buffer,MAX_ATM_ADDR_LEN+1,(struct sockaddr *) &source,pretty)
     >= 0) diag(COMPONENT,DIAG_DEBUG,"  SRC ATM: %s",buffer);
   ipp = (unsigned char *) &tgt_ip;
   diag(COMPONENT,DIAG_DEBUG,"  DST IP: %d.%d.%d.%d",ipp[0],ipp[1],ipp[2],
     ipp[3]);
   if (atm2text(buffer,MAX_ATM_ADDR_LEN+1,(struct sockaddr *) &target,pretty)
     >= 0) diag(COMPONENT,DIAG_DEBUG,"  DST ATM: %s",buffer);
}
    switch (ntohs(hdr->ar_op)) {
	case ARPOP_REQUEST:
	    diag(COMPONENT,DIAG_DEBUG,"got ARP_REQ");
	    if (learn(vcc,src_ip,&source)) break;
	    entry = NULL;
	    itf = lookup_itf_by_ip(tgt_ip);
	    entry = itf ? lookup_ip(itf,tgt_ip) : NULL;
	    if (entry && entry->state == as_valid && (entry->flags & ATF_PUBL))
	      {
		if (entry->addr)
		    arp_reply(vcc,tgt_ip,entry->addr,src_ip,&source);
		else arp_nak(vcc,tgt_ip,src_ip,&source);
	    }
	    else {
		if (itf && itf->local_ip == tgt_ip)
		    arp_reply(vcc,tgt_ip,NULL,src_ip,&source);
		else arp_nak(vcc,tgt_ip,src_ip,&source);
	    }
	    break;
	case ARPOP_REPLY:
	    diag(COMPONENT,DIAG_DEBUG,"got ARP_REP");
	    if (!vcc->entry || !(vcc->entry->flags & ATF_ARPSRV)) {
		diag(COMPONENT,DIAG_ERROR,"got ARP response from charlatan");
		break;
	    }
	    (void) learn(NULL,src_ip,&source);
	    break;
	case ARPOP_InREQUEST:
	    diag(COMPONENT,DIAG_DEBUG,"got InARP_REQ");
	    if (!learn(vcc,src_ip,&source)) inarp_reply(vcc,src_ip,&source);
	    break;
	case ARPOP_InREPLY:
	    diag(COMPONENT,DIAG_DEBUG,"got InARP_REP");
	    (void) learn(vcc,src_ip,&source);
	    break;
	case ARPOP_NAK:
	    diag(COMPONENT,DIAG_DEBUG,"got ARP_NAK");
	    if (!vcc->entry || !(vcc->entry->flags & ATF_ARPSRV)) {
		diag(COMPONENT,DIAG_ERROR,"got ARP response from charlatan");
		break;
	    }
	    learn_nak(tgt_ip);
	    break;
	default:
	    diag(COMPONENT,DIAG_ERROR,"unrecognized ARP op 0x%x",
	      ntohs(hdr->ar_op));
    }
}


static int ioctl_set_pvc(ITF *itf,uint32_t ip,struct sockaddr_atmpvc *addr,
  const struct atm_qos *qos,int sndbuf,int flags)
{
    ENTRY *entry;
    VCC *vcc;
    int fd,result;

    if (lookup_ip(itf,ip)) return -EEXIST; 
    if ((fd = connect_vcc((struct sockaddr *) addr,qos,sndbuf,0)) < 0)
	return fd;
    if ((result = set_ip(fd,ip)) < 0) {
	do_close(fd);
	return result;
    }
    if (flags & ATF_NULL) {
	if ((result = set_encap(fd,0)) < 0) return result;
	flags |= ATF_PERM;
    }
    entry = alloc_entry(0);
    entry->state = as_valid;
    entry->ip = ip;
    entry->qos = *qos;
    entry->sndbuf = sndbuf;
    entry->flags = flags;
    entry->itf = itf;
    vcc = alloc_t(VCC);
    vcc->active = 1;
    vcc->connecting = 0;
    vcc->fd = fd;
    vcc->entry = entry;
    if (!(flags & ATF_PERM)) START_TIMER(entry,CREVAL);
    Q_INSERT_HEAD(entry->vccs,vcc);
    Q_INSERT_HEAD(itf->table,entry);
    return 0;
}


static int ioctl_set_svc(ITF *itf,uint32_t ip,struct sockaddr_atmsvc *addr,
  const struct atm_qos *qos,int sndbuf,int flags)
{
    ENTRY *entry;

    if (flags & ATF_ARPSRV) flags |= ATF_PERM;
    if (lookup_ip(itf,ip)) return -EEXIST;
    entry = alloc_entry(1);
    entry->state = as_valid;
    entry->ip = ip;
    entry->addr = alloc_t(struct sockaddr_atmsvc);
    *entry->addr = *addr;
    entry->qos = *qos;
    entry->sndbuf = sndbuf;
    entry->flags = flags;
    if (!(flags & ATF_PERM) || (flags & ATF_ARPSRV)) {
	if (itf->arp_srv) START_TIMER(entry,CREVAL);
	else START_TIMER(entry,SREVAL);
    }
    entry->itf = itf;
    Q_INSERT_HEAD(itf->table,entry);
    if (!(flags & ATF_ARPSRV)) return 0;
    entry->state = as_invalid;
    itf->arp_srv = entry;
    (void) want_arp_srv(itf);
    return 0;
}


static int ioctl_delete(ITF *itf,uint32_t ip,int flags)
{
    ENTRY *entry,*walk,*next;

    if (!(entry = lookup_ip(itf,ip))) {
	diag(COMPONENT,DIAG_WARN,"ioctl_delete didn't find entry");
	return -ENOENT;
    }
    if ((flags ^ entry->flags) & ATF_ARPSRV) return -EINVAL;
    send_notifications(entry,0);
    if ((entry->flags & ATF_ARPSRV) && entry->itf)
	for (walk = entry->itf->table; walk; walk = next) {
	    next = walk->next;
	    if (walk != entry && walk->state == as_resolv) {
		send_notifications(walk,0);
		if (!walk->vccs && !(walk->flags & ATF_PERM))
		   /* PERM is rather unlikely here, since this would be a
		      second ARP server (only ARP servers can go as_resolv if
		      PERM), but we'll check for it anyway. */
		    discard_entry(walk);
		else {
		    STOP_TIMER(walk);
		    walk->state = as_invalid;
		}
	    }
	}
    discard_entry(entry);
    return 0;
}


static int qos_present(const struct atm_qos *qos)
{
    return qos->txtp.traffic_class || qos->rxtp.traffic_class;
}


int arp_ioctl(struct atmarp_req *req)
{
    ITF *itf;
    char buffer[MAX_ATM_ADDR_LEN+1];
    unsigned char *ipp;

    diag(COMPONENT,DIAG_DEBUG,"arp_ioctl %d",req->type);
    itf = lookup_itf_by_ip(req->ip);
    if (!itf) return -EHOSTUNREACH;
    if (!(req->ip & ~itf->netmask) && !(req->flags & ATF_ARPSRV) &&
	req->type != art_qos) return -EADDRNOTAVAIL;
    ipp = (unsigned char *) &req->ip;
    switch (req->type) {
	case art_qos:
	    diag(COMPONENT,DIAG_DEBUG,"got art_qos for itf %d",itf->number);
	    if (qos_present(&req->qos)) itf->qos = req->qos;
	    if (req->sndbuf) itf->sndbuf = req->sndbuf;
	    return 0;
	case art_set:
	    if (atm2text(buffer,MAX_ATM_ADDR_LEN+1,
	      (struct sockaddr *) &req->addr,pretty) < 0) {
		diag(COMPONENT,DIAG_ERROR,"a2t fails on art_set");
		return -EINVAL;
	    }
	    diag(COMPONENT,DIAG_DEBUG,"got art_set for itf %d, IP %d.%d.%d.%d"
	      ", ATM %s, flags 0x%x",itf->number,ipp[0],ipp[1],ipp[2],ipp[3],
	      buffer,req->flags);
	    if (!qos_present(&req->qos)) req->qos = itf->qos;
	    if (!req->sndbuf) req->sndbuf = itf->sndbuf;
	    switch (req->addr.sas_family) {
		case AF_ATMPVC:	
		    adjust_qos(itf,&req->qos,req->flags & ATF_NULL);
		    return ioctl_set_pvc(itf,req->ip,
		      (struct sockaddr_atmpvc *) &req->addr,&req->qos,
		      req->sndbuf,req->flags);
		case AF_ATMSVC:
		    adjust_qos(itf,&req->qos,0);
		    return ioctl_set_svc(itf,req->ip,
		      (struct sockaddr_atmsvc *) &req->addr,&req->qos,
		      req->sndbuf,req->flags);
		default: /* not reached - atm2text complains before */
		    diag(COMPONENT,DIAG_ERROR,"art_set: bad HA AF 0x%x",
		      req->addr.sas_family);
		    return -EINVAL;
	    }
	case art_delete:
	    diag(COMPONENT,DIAG_DEBUG,"got art_delete for itf %d, IP "
	      "%d.%d.%d.%d",itf->number,ipp[0],ipp[1],ipp[2],ipp[3]);
	    return ioctl_delete(itf,req->ip,req->flags);
	default:
	    diag(COMPONENT,DIAG_ERROR,"unrecognized type %d",req->type);
	    return -EINVAL;
    }
}


void vcc_failed(VCC *vcc)
{
    ENTRY *entry,*next;

    diag(COMPONENT,DIAG_DEBUG,"failed VCC 0x%p",vcc);
    Q_REMOVE(vcc->entry->vccs,vcc);
    if (!(vcc->entry->flags & ATF_ARPSRV) || vcc->entry->vccs) {
	/* VCC is already closed */
	vcc_detach(vcc->entry);
	free(vcc);
	return;
    }
    for (entry = vcc->entry->itf->table; entry; entry = next) {
	next = entry->next;
	if (entry == vcc->entry || entry->state != as_resolv) continue;
	if (entry->vccs || (entry->flags & ATF_PERM)) entry->state = as_invalid;
	else discard_entry(entry);
    }
    START_TIMER(vcc->entry,RETRY);
    free(vcc);
}


void vcc_connected(VCC *vcc)
{
    ENTRY *entry;

    diag(COMPONENT,DIAG_DEBUG,"connected VCC 0x%p",vcc);
    if (!vcc->connecting)
	diag(COMPONENT,DIAG_FATAL,"non-connecting VCC connected");
    vcc->connecting = 0;
    if (vcc->entry->flags & ATF_ARPSRV)
	for (entry = vcc->entry->itf->table; entry; entry = entry->next)
	    if (entry->state == as_resolv) {
		START_TIMER(entry,REPLY);
		arp_request(entry->itf,entry->ip);
	    }
    inarp_request(vcc->entry);
    if (vcc->entry->state == as_valid)
	if (set_ip(vcc->fd,vcc->entry->ip) < 0) {
	    diag(COMPONENT,DIAG_ERROR,"can't set IP (%s)",strerror(errno));
	    disconnect_vcc(vcc);
	}
}


void disconnect_vcc(VCC *vcc)
{
    ENTRY *entry;

    diag(COMPONENT,DIAG_DEBUG,"disconnected VCC 0x%p",vcc);
    entry = vcc->entry;
    discard_vcc(vcc);
    if (entry) vcc_detach(entry);
}


void incoming_call(VCC *vcc)
{
    diag(COMPONENT,DIAG_DEBUG,"incoming VCC 0x%p",vcc);
    START_TIMER(vcc->entry,REPLY);
    inarp_request(vcc->entry);
}


void incoming_unidirectional(VCC *vcc)
{
    diag(COMPONENT,DIAG_DEBUG,"incoming unidirectional VCC 0x%p",vcc);
    /* don't put it into ATMARP table */
}
