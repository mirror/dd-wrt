/* proto.c - Common protocol functions and structures */
 
/* Written 1995-2000 by Werner Almesberger, EPFL-LRC/ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "atmd.h"
#include "uni.h"
#include "qlib.h"
#include <q.out.h>

#include "io.h"
#include "proto.h"
#include "sap.h"


#define COMPONENT "SIGD"


const char *state_name[] = { /* formatting aligned with STATE */
	"<invalid>",	"ss_null",	"ss_listening",	"ss_connecting",
	"ss_connected",	"ss_indicated",	"ss_accepting",	"ss_zombie",
	"ss_wait_rel",	"ss_wait_close","ss_rel_req",	"ss_rel_ind",
	"ss_proceeding","ss_listen_zombie",
	"ss_mod_lcl",	"ss_mod_req",	"ss_mod_rcv",	"ss_mod_fin_ack",
	"ss_mod_fin_ok","ss_mod_fin_fail"
	};

const char *cs_name[] = {
	"NULL",		"CALL_INIT",	"<invalid>",	"OUT_PROC",
	"<invalid>",	"<invalid>",	"CALL_PRES",	"<invalid>",
	"CONN_REQ",	"IN_PROC",	"ACTIVE",	"REL_REQ",
	"REL_IND",	"MOD_REQ",	"MOD_RCV" };

const char *as_name[] = { "<invalid>","as_bind","as_connect","as_accept",
  "as_reject","as_listen","as_okay","as_error","as_indicate","as_close",
  "as_itf_notify","as_modify","as_identify","as_terminate",
#ifdef MULTPOINT
  "as_addparty", "as_dropparty",
#endif
  };

const CALL_STATE state_map[] = { /* formatting aligned with STATE */
	cs_null,	cs_null,	cs_null,	cs_call_init,
	cs_active,	cs_in_proc,	cs_conn_req,	cs_null,
	cs_rel_req,	cs_null,	cs_rel_req,	cs_rel_ind,
	cs_in_proc,	cs_null,
#ifdef Q2963_1
	cs_active,	cs_mod_req,	cs_mod_rcv,	cs_active,
	cs_active,	cs_active
#endif
	};

const PARTY_STATE eps_map[] = {
	ps_null,	ps_add_init,	ps_null,	ps_add_init,	/* 0 */
	ps_null,	ps_null,	ps_add_recv,	ps_null,	/* 4 */
	ps_active,	ps_add_recv,	ps_active,	ps_active,	/* 8 */
	ps_active };							/*12 */


atm_kptr_t kptr_null;
SIG_ENTITY *entities = &_entity;
SOCKET *sockets = NULL;
unsigned char q_buffer[MAX_Q_MSG];


SOCKET *new_sock(atm_kptr_t id)
{
    SOCKET *sock;
 
    sock = alloc_t(SOCKET);
    sock->sig = NULL;
    sock->state = ss_invalid;
    memset(&sock->pvc,0,sizeof(sock->pvc));
    sock->qos.txtp.traffic_class = sock->qos.rxtp.traffic_class = ATM_UBR;
#if defined(Q2963_1) || defined(DYNAMIC_UNI)
    memset(&sock->new_qos, 0, sizeof(sock->new_qos));
    sock->owner = 0;
#endif
    sock->id = id;
    memset(&sock->local,0,sizeof(sock->local));
    memset(&sock->remote,0,sizeof(sock->remote));
    memset(&sock->sap,0,sizeof(sock->sap));
    sock->error = 0;
    sock->call_state = cs_null;
    sock->ep_ref = -1;
    sock->conn_timer = NULL;
    sock->listen = NULL;
    sock->next = sockets;
    sockets = sock;
    return sock;
}


void free_sock(SOCKET *sock)
{
    SOCKET **walk;

    diag(COMPONENT,DIAG_DEBUG,"freeing socket %s@%p",
      kptr_print(&sock->id),sock);
    for (walk = &sockets; *walk != sock; walk = &(*walk)->next);
    if (!*walk)
	diag(COMPONENT,DIAG_FATAL,
	  "INTERNAL ERROR: freeing non-existing socket %s",
	  kptr_print(&sock->id));
    *walk = sock->next;
    if (sock->conn_timer) {
	diag(COMPONENT,DIAG_ERROR,"socket %s has timer (%p) running",
	  kptr_print(&sock->id),sock->conn_timer->callback);
	stop_timer(sock->conn_timer);
    }
    if (sock->listen)
        diag(COMPONENT,DIAG_ERROR,"socket %s has non-empty listen queue",
	  kptr_print(&sock->id));
    sock->state = ss_invalid;
    free(sock);
}

#ifdef MULTIPOINT
/* don't call free_sock() after free_leaves() -- this function frees the
 * root node as well */
void free_leaves(atm_kptr_t *id)
{
	SOCKET **walk = &sockets, *target = NULL;
	while (*walk != NULL) {
		target = *walk;
		walk = &((*walk)->next);
		if (kptr_eq(&target->id, id)) {
			diag(COMPONENT, DIAG_INFO, "freeing leaf %s:%d",
			     kptr_print(&target->id), target->ep_ref);
			free_sock(target);
		}
	}
}

int count_leaves(atm_kptr_t *id)
{
	SOCKET *walk = sockets;

	int count = 0;
	if (walk)
		for (; walk; walk = walk->next)
			if(kptr_eq(&walk->id, id))
				count++;
	return count;
}
#endif

void new_state(SOCKET *sock,STATE state)
{
    diag(COMPONENT,DIAG_DEBUG,"socket %s enters state %s (UNI %s)",
      kptr_print(&sock->id),state_name[state],cs_name[state_map[state]]);
    sock->state = state;
    sock->call_state = state_map[state];
}


SOCKET *lookup_sap(const struct sockaddr_atmsvc *addr,
  const struct atm_sap *sap,const struct atm_qos *qos,
  struct sockaddr_atmsvc *res_addr,struct atm_sap *res_sap,
  struct atm_qos *res_qos,int exact_match)
{
    SOCKET *walk,*wildcard;
    int new_wc;

    new_wc = !atmsvc_addr_in_use(*addr);
    wildcard = NULL;
    for (walk = sockets; walk; walk = walk->next) {
	if (walk->state != ss_listening || !sap_compat(&walk->local,
	  addr,res_addr,&walk->sap,sap,res_sap,&walk->qos,qos,res_qos))
	    continue;
	if (atmsvc_addr_in_use(walk->local)) return walk;
	else if (exact_match) {
		if (new_wc) return walk;
	    }
	    else wildcard = walk;
    }
    return wildcard;
}


const char *mid2name(unsigned char mid)
{
    switch (mid) {
	case ATM_MSG_NATIONAL:
	    return "National specific message escape";
	case ATM_MSG_SETUP:
	    return "SETUP";
	case ATM_MSG_ALERTING:
	    return "ALERTING";
	case ATM_MSG_CALL_PROC:
	    return "CALL_PROCEEDING";
	case ATM_MSG_CONNECT:
	    return "CONNECT";
	case ATM_MSG_CONN_ACK:
	    return "CONNECT_ACK";
	case ATM_MSG_RESTART:
	    return "RESTART";
	case ATM_MSG_RELEASE:
	    return "RELEASE";
	case ATM_MSG_REST_ACK:
	    return "REST_ACK";
	case ATM_MSG_REL_COMP:
	    return "REL_COMP";
	case ATM_MSG_NOTIFY:
	    return "NOTIFY";
	case ATM_MSG_STATUS_ENQ:
	    return "STATUS_ENQ";
	case ATM_MSG_STATUS:
	    return "STATUS";
	case ATM_MSG_ADD_PARTY:
	    return "ADD_PARTY";
	case ATM_MSG_ADD_PARTY_ACK:
	    return "ADD_PARTY_ACK";
	case ATM_MSG_ADD_PARTY_REJ:
	    return "ADD_PARTY_REJECT";
	case ATM_MSG_PARTY_ALERT:
	    return "PARTY_ALERTING";
	case ATM_MSG_DROP_PARTY:
	    return "DROP_PARTY";
	case ATM_MSG_DROP_PARTY_ACK:
	    return "DROP_PARTY_ACK";
	case ATM_MSG_MODIFY_REQ:
	    return "MODIFY_REQUEST";
	case ATM_MSG_MODIFY_ACK:
	    return "MODIFY_ACK";
	case ATM_MSG_MODIFY_REJ:
	    return "MODIFY_REJECT";
	case ATM_MSG_CONN_AVAIL:
	    return "CONN_AVAIL";
	case ATM_MSG_LEAF_FAILURE:
	    return "LEAF SETUP FAIL";
	case ATM_MSG_LEAF_REQUEST:
	    return "LEAF SETUP REQ";
	case ATM_MSG_RESERVED:
	    return "Reserved...";
	default:
	    return "???";
    }
}


void send_kernel(atm_kptr_t vcc,atm_kptr_t listen_vcc,
  enum atmsvc_msg_type type,int reply,const struct sockaddr_atmpvc *pvc,
  const struct sockaddr_atmsvc *svc,const struct sockaddr_atmsvc *local,
  const struct atm_sap *sap,const struct atm_qos *qos)
{
    struct atmsvc_msg *msg;

    msg = alloc_t(struct atmsvc_msg);
    msg->vcc = vcc;
    msg->listen_vcc = listen_vcc;
    msg->type = type;
    msg->reply = reply;
    if (pvc) msg->pvc = *pvc;
    else memset(&msg->pvc,0,sizeof(msg->pvc));
    if (sap) msg->sap = *sap;
    else memset(&msg->sap,0,sizeof(msg->sap));
    if (qos) msg->qos = *qos;
    else memset(&msg->qos,0,sizeof(msg->qos));
    if (local) msg->local = *local;
    else memset(&msg->local,0,sizeof(msg->local));
    if (svc) msg->svc = *svc;
    else memset(&msg->svc,0,sizeof(msg->svc));
    to_kernel(msg);
    free(msg);
}


void send_release(SOCKET *sock,unsigned char reason,...)
{
    va_list ap;
    Q_DSC dsc;
    int size;
 
    q_create(&dsc,q_buffer,MAX_Q_MSG);
    q_assign(&dsc,QF_msg_type,ATM_MSG_RELEASE);
    q_assign(&dsc,QF_call_ref,sock->call_ref);
    q_assign(&dsc,QF_cause,reason);
    va_start(ap,reason);
    switch (reason) {
	case ATM_CV_TIMER_EXP:
	    {
		char buf[4];

		sprintf(buf,"%d",va_arg(ap,int));
		q_write(&dsc,QF_timer,buf,3);
		break;
	    }
    }
    va_end(ap);
    if ((size = q_close(&dsc)) >= 0) to_signaling(sock->sig,q_buffer,size);
}


void send_release_complete(SIG_ENTITY *sig,unsigned long call_ref,
  unsigned char cause,...)
{
    va_list ap;
    Q_DSC dsc;
    int size;
 
    q_create(&dsc,q_buffer,MAX_Q_MSG);
    q_assign(&dsc,QF_msg_type,ATM_MSG_REL_COMP);
    q_assign(&dsc,QF_call_ref,call_ref);
    if (cause) {
	q_assign(&dsc,QF_cause,cause);
	va_start(ap,cause);
	switch (cause) {
	    case ATM_CV_INVALID_IE:
		{
		    unsigned char ie;

		    ie = va_arg(ap,int);
		    q_write(&dsc,QF_ie_id6,&ie,1);
		}
		break;
	}
	va_end(ap);
    }
    if ((size = q_close(&dsc)) >= 0) to_signaling(sig,q_buffer,size);
}


void send_modify_reject(SOCKET *sock,unsigned char reason)
{
    Q_DSC dsc;
    int size;
 
    q_create(&dsc,q_buffer,MAX_Q_MSG);
    q_assign(&dsc,QF_msg_type,ATM_MSG_MODIFY_REJ);
    q_assign(&dsc,QF_call_ref,sock->call_ref);
    q_assign(&dsc,QF_cause,reason);
    if ((size = q_close(&dsc)) >= 0) to_signaling(sock->sig,q_buffer,size);
}


void set_error(SOCKET *sock,int code)
{
    if (!sock->error) sock->error = code;
}


void send_close(SOCKET *sock)
{
    if (sock->error == 1234) diag(COMPONENT,DIAG_FATAL,"BUG! BUG! BUG!");
    send_kernel(sock->id,kptr_null,as_close,sock->error,NULL,NULL,NULL,NULL,
      NULL);
    sock->error = 1234;
}


void q_report(int severity,const char *msg,...)
{
    va_list ap;

    va_start(ap,msg);
    vdiag("QMSG",severity,msg,ap);
    va_end(ap);
}


int get_vci(int itf)
{
    SOCKET *walk;
    int s,vci;

    s = get_pvc(itf,&vci);
    if (s < 0) return s;
    for (walk = sockets; walk; walk = walk->next)
	if (walk->pvc.sap_addr.itf == itf && walk->pvc.sap_addr.vci == vci) {
	    vci = get_vci(itf); /* this recursion will keep all the busy ones
				   open until we return */
	    break;
	}
    (void) close(s);
    return vci;
}


void enter_vpci(SIG_ENTITY *sig,int vpci,int itf)
{
    VPCI *entry;

    for (entry = sig->vpcis; entry; entry = entry->next)
	if (entry->vpci == vpci) {
	    diag(COMPONENT,DIAG_ERROR,"ignoring duplicate VPCI %d (itf %d)",
	      vpci,itf);
	    return;
	}
    entry = alloc_t(VPCI);
    entry->vpci = vpci;
    entry->itf = itf;
    entry->local_addr[0].state = ls_unused;
    entry->next = sig->vpcis;
    sig->vpcis = entry;
}


void set_vpi_0(SIG_ENTITY *sig)
{
    VPCI *vpci;

    for (vpci = sig->vpcis; vpci; vpci = vpci->next)
	if (!vpci->vpci) return;
    enter_vpci(sig,0,sig->signaling_pvc.sap_addr.itf);
}


int get_itf(SIG_ENTITY *sig,int *vpci)
{
    VPCI *best,*walk;

    best = NULL;
    for (walk = sig->vpcis; walk; walk = walk->next)
	if (walk->vpci <= *vpci && (!best || best->vpci < walk->vpci))
	    best = walk;
    if (!best) return sig->signaling_pvc.sap_addr.itf;
    *vpci -= best->vpci;
    return best->itf;
}


void init_addr(SIG_ENTITY *sig)
{
    VPCI *vpci;

    for (vpci = sig->vpcis; vpci; vpci = vpci->next) sync_addr(vpci);
}


void itf_reload(int itf)
{
    SIG_ENTITY *sig;
    VPCI *vpci;

    for (sig = entities; sig; sig = sig->next)
	for (vpci = sig->vpcis; vpci; vpci = vpci->next)
	    if (vpci->itf == itf) sync_addr(vpci);
}


static struct sockaddr_atmsvc *do_get_local(SIG_ENTITY *sig)
{
    VPCI *vpci;
    LOCAL_ADDR *local;

    for (vpci = sig->vpcis; vpci; vpci = vpci->next)
	for (local = vpci->local_addr; local->state != ls_unused; local++)
	    if (local->state == ls_same && atmsvc_addr_in_use(local->addr))
		return &local->addr;
    return NULL;

}


struct sockaddr_atmsvc *get_local(SIG_ENTITY *sig)
{
    SIG_ENTITY *entity;
    struct sockaddr_atmsvc *local;

    if (sig) return do_get_local(sig);
    for (entity = entities; entity; entity = entity->next) {
	local = do_get_local(entity);
	if (local) return local;
    }
    return NULL;
}


/*
 * The following code is stolen from switch/route.c. Eventually, all this
 * should move into a library.
 */


#include <limits.h>


typedef struct _route {
    struct sockaddr_atmsvc addr;
    int len;
    SIG_ENTITY *sig;
    struct _route *next;
} ROUTE;


static ROUTE *routes = NULL;


void add_route(SIG_ENTITY *sig,struct sockaddr_atmsvc *addr,int len)
{
    ROUTE *route;

    for (route = routes; route; route = route->next)
        if (route->len == len && (!len || atm_equal((struct sockaddr *) addr,
          (struct sockaddr *) &route->addr,len,
          AXE_PRVOPT | (len == INT_MAX ? 0 : AXE_WILDCARD))))
            diag(COMPONENT,DIAG_FATAL,"duplicate route");
    route = alloc_t(ROUTE);
    if (addr) route->addr = *addr;
    route->len = len;
    route->sig = sig;
    route->next = routes;
    routes = route;
}


SIG_ENTITY *route_remote(struct sockaddr_atmsvc *addr)
{
    ROUTE *best,*route;
    int best_len;

    if (!routes) return entities;
    best = NULL;
    best_len = -1;
    for (route = routes; route; route = route->next)
        if (route->len > best_len && (!route->len ||
          atm_equal((struct sockaddr *) addr,(struct sockaddr *) &route->addr,
          route->len,
          AXE_PRVOPT | (route->len == INT_MAX ? 0 : AXE_WILDCARD)))) {
            if (route->len == INT_MAX) return route->sig;
            best_len = route->len;
            best = route;
        }
    return best ? best->sig : NULL;
}


SIG_ENTITY *route_local(struct sockaddr_atmsvc *addr)
{
    SIG_ENTITY *sig;
    VPCI *vpci;
    LOCAL_ADDR *walk;

    /*
     * @@@ This is quite close to the truth but still not entirely correct:
     * the actual result should be the VPCI to use. While this is irrelevant
     * when the connection identifier is assigned by the peer, the VPI
     * selection should be based on the VPCI we find here if it's atmsigd who
     * assigns the connection identifier. The bottom line is that VPI > 0 plus
     * connection identifier assignment by atmsigd doesn't work yet.
     */
    for (sig = entities; sig; sig = sig->next)
	for (vpci = sig->vpcis; vpci; vpci = vpci->next)
	    for (walk = vpci->local_addr; walk->state != ls_unused; walk++)
		if (walk->state == ls_same && atm_equal((struct sockaddr *)
		  &walk->addr,(struct sockaddr *) addr,(ATM_ESA_LEN-1)*8,
		  AXE_WILDCARD))
		    return sig;
    return NULL;
}


int get_max_rate(SIG_ENTITY *sig)
{
    VPCI *vpci;
    int max;

    if (sig->mode == sm_switch) return 0; /* switches don't use ATM_MAX_PCR */
    max = 0;
    for (vpci = sig->vpcis; vpci; vpci = vpci->next) {
	int curr;

	curr = get_link_rate(vpci->itf);
	if (curr > max) max = curr;
    }
    return max;
}
