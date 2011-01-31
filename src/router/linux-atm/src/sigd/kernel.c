/* kernel.c - Processing of incoming kernel messages */
 
/* Written 1995-2000 by Werner Almesberger, EPFL-LRC/ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#include <atm.h>
#include <linux/atmsvc.h>
#include <linux/atmdev.h>

#include "atmd.h"
#include "uni.h"
#include "qlib.h"
#include <q.out.h>

#include "proto.h"
#include "sap.h"
#include "io.h"
#include "policy.h"
#include "timeout.h"


#define COMPONENT "KERNEL"


#ifdef MULTIPOINT
int send_dropparty(SOCKET *sock)
{
	Q_DSC dsc;
	int size;

	q_create(&dsc, q_buffer, MAX_Q_MSG);
	q_assign(&dsc, QF_msg_type, ATM_MSG_DROP_PARTY);
	q_assign(&dsc, QF_call_ref, sock->call_ref);
	q_assign(&dsc, QF_ep_ref, sock->ep_ref);
	q_assign(&dsc, QF_cause, ATM_CV_NORMAL_UNSPEC);
	if ((size = q_close(&dsc)) < 0)
		return -EINVAL;

	to_signaling(sock->sig, q_buffer, size);
	START_TIMER(sock, T398);
	return 0;
}


int send_addparty(SOCKET *sock, struct sockaddr_atmsvc *svc)
{
    SOCKET *walk = NULL;
    Q_DSC dsc;
    struct sockaddr_atmsvc *local = NULL;
    int error = 0,size,max_ep_ref = 0;
    char buf[80];

    diag(COMPONENT, DIAG_INFO, "send_addparty");
    q_create(&dsc,q_buffer,MAX_Q_MSG);

    /* think this next line is already done */
    /* sock->sig = route_remote(&sock->remote); */

    /* set the calling party address */
    if (sock->sig && sock->sig->mode == sm_switch) local = &sock->local;
    if (atmsvc_addr_in_use(sock->local) && !local) {
	sock->sig = route_local(&sock->local);
	if (sock->sig) local = &sock->local;
	else {
	    error = -EADDRNOTAVAIL;
	    diag(COMPONENT,DIAG_WARN,"local address no longer available");
	}
    }
    else {
	if (!sock->sig) error = -EHOSTUNREACH;
	else {
	    if (!local) local = get_local(sock->sig);
	    if (!local) local = get_local(NULL);
	    if (local) sock->local = *local;
	    else {
		error = -EADDRNOTAVAIL;
		diag(COMPONENT,DIAG_ERROR,"no local address");
	    }
	}
    }
    if (local) {
	if (*local->sas_addr.pub) {
	    q_assign(&dsc,QF_cgpn_plan,ATM_NP_E164);
	    q_assign(&dsc,QF_cgpn_type,ATM_TON_INTRNTNL);
	    q_write(&dsc,QF_cgpn,(void *) local->sas_addr.pub,
	      strlen(local->sas_addr.pub));
	}
	else {
	    q_assign(&dsc,QF_cgpn_plan,ATM_NP_AEA);
	    q_assign(&dsc,QF_cgpn_type,ATM_TON_UNKNOWN);
	    q_write(&dsc,QF_cgpn,(void *) local->sas_addr.prv,ATM_ESA_LEN);
	}
    }

    /* what to do if error == -EADDRNOTAVAIL ? */

    /* find the largest endpoint reference in the list */
    for (walk = sockets; walk->next; walk = walk->next) {
	if ((walk->call_ref == sock->call_ref) &&
		(walk->ep_ref > max_ep_ref)) max_ep_ref = walk->ep_ref;
    }
    sock->ep_ref = max_ep_ref + 1;

    q_assign(&dsc,QF_call_ref,sock->call_ref);
    q_assign(&dsc,QF_aal_type,5); /* AAL 5 */
    q_assign(&dsc,QF_msg_type,ATM_MSG_ADD_PARTY);

    diag(COMPONENT, DIAG_INFO, "send_addparty: sap_encode");
    if (atm2text(buf,MAX_ATM_ADDR_LEN+1,(struct sockaddr *)svc, pretty) >= 0)
	    diag(COMPONENT, DIAG_INFO, "send_addparty -> %s", buf);
    else
	    diag(COMPONENT, DIAG_INFO, "send_addparty: invalid nsap address");
    error = sap_encode(&dsc,svc,&sock->sap,NULL,sock->sig->uni,sock->sig->max_rate);
    diag(COMPONENT, DIAG_INFO, "send_addparty: sap_encode error = %d", error);

    q_assign(&dsc, QF_ep_ref, sock->ep_ref);
    error = 0;
    if ((size = q_close(&dsc)) < 0) {
	error = -EINVAL;
    } else if (!error) {
		to_signaling(sock->sig,q_buffer,size);
		new_state(sock,ss_connecting);
		return sock->ep_ref;
    }
    return error;
}
#endif


static int send_setup(SOCKET *sock)
{
    static unsigned long call_ref = 0;
    struct sockaddr_atmsvc *local;
    SOCKET *walk;
    Q_DSC dsc;
    int error,size;

    do {
	if (++call_ref == 0x7fffff) call_ref = 1;
	for (walk = sockets; walk; walk = walk->next)
	    if (walk->call_ref == call_ref) break;
    }
    while (walk);
    sock->call_ref = call_ref;
    q_create(&dsc,q_buffer,MAX_Q_MSG);
    local = NULL;
    error = 0;
    sock->sig = route_remote(&sock->remote);
    if (sock->sig && sock->sig->mode == sm_switch) local = &sock->local;
    if (atmsvc_addr_in_use(sock->local) && !local) {
	sock->sig = route_local(&sock->local);
	if (sock->sig) local = &sock->local;
	else {
	    error = -EADDRNOTAVAIL;
	    diag(COMPONENT,DIAG_WARN,"local address no longer available");
	}
    }
    else {
	if (!sock->sig) error = -EHOSTUNREACH;
	else {
	    if (!local) local = get_local(sock->sig);
	    if (!local) local = get_local(NULL);
	    if (local) sock->local = *local;
	    else {
		error = -EADDRNOTAVAIL;
		diag(COMPONENT,DIAG_ERROR,"no local address");
	    }
	}
    }
    if (local) {
	if (*local->sas_addr.pub) {
	    q_assign(&dsc,QF_cgpn_plan,ATM_NP_E164);
	    q_assign(&dsc,QF_cgpn_type,ATM_TON_INTRNTNL);
	    q_write(&dsc,QF_cgpn,(void *) local->sas_addr.pub,
	      strlen(local->sas_addr.pub));
	}
	else {
	    q_assign(&dsc,QF_cgpn_plan,ATM_NP_AEA);
	    q_assign(&dsc,QF_cgpn_type,ATM_TON_UNKNOWN);
	    q_write(&dsc,QF_cgpn,(void *) local->sas_addr.prv,ATM_ESA_LEN);
	}
    }
    q_assign(&dsc,QF_call_ref,call_ref);
    q_assign(&dsc,QF_msg_type,ATM_MSG_SETUP);
    q_assign(&dsc,QF_aal_type,5); /* AAL 5 */
#if defined(UNI30) || defined(DYNAMIC_UNI)
    if (!error && sock->sig->uni == S_UNI30)
	q_assign(&dsc,QF_aal_mode,1); /* Message mode - LANE seems to really
					 want this */
#endif
    q_assign(&dsc,QF_sscs_type,0); /* unspecified - LANE wants this */
    if (!error)
	error = sap_encode(&dsc,&sock->remote,&sock->sap,&sock->qos,
	  sock->sig->uni,sock->sig->max_rate);
    q_assign(&dsc,QF_bearer_class,16); /* BCOB-X */
#ifndef MULTIPOINT
    q_assign(&dsc,QF_upcc,0); /* p2p */
#else
    if (sock->ct == p2p) {
	diag(COMPONENT, DIAG_INFO, "setting configuration to point-to-point");
	q_assign(&dsc,QF_upcc,0); /* p2p */
    } else if (sock->ct == p2mp) {
	diag(COMPONENT, DIAG_INFO, "setting configuration to multipoint");
	q_assign(&dsc, QF_upcc, 1);   /* p2mp */
	q_assign(&dsc, QF_ep_ref, 0); /* initial setup always leaf 0 */
	sock->ep_ref = 0;
    }
#endif
#if !defined(UNI30) || defined(DYNAMIC_UNI)
    if (!error && sock->sig->uni != S_UNI30)
	q_assign(&dsc,QF_qos_cs,Q2931_CS_ITU);
#endif
    q_assign(&dsc,QF_qos_fw,0); /* QOS 0 */
    q_assign(&dsc,QF_qos_bw,0);
    if (!error && sock->sig->mode != sm_user) {
	if (!atmpvc_addr_in_use(sock->pvc)) {
	    int vpci,vci;

	    if (sock->sig->mode == sm_switch)
		diag(COMPONENT,DIAG_FATAL,"No CI allocator (use -A)");
	    vpci = 0;
	    sock->pvc.sap_addr.itf = get_itf(sock->sig,&vpci);
	    sock->pvc.sap_addr.vpi = vpci;
	    vci = get_vci(sock->pvc.sap_addr.itf);
	    if (vci < 0) {
		(void) q_close(&dsc);
		return vci;
	    }
	    sock->pvc.sap_addr.vci = vci;
	}
	q_assign(&dsc,QF_vpi,sock->pvc.sap_addr.vpi);
	q_assign(&dsc,QF_vci,sock->pvc.sap_addr.vci);
    }
    if ((size = q_close(&dsc)) < 0) error = -EINVAL;
    else if (!error) to_signaling(sock->sig,q_buffer,size);
    return error;
}


static int send_connect(SOCKET *sock)
{
    Q_DSC dsc;
    int size;

    q_create(&dsc,q_buffer,MAX_Q_MSG);
    q_assign(&dsc,QF_msg_type,ATM_MSG_CONNECT);
    q_assign(&dsc,QF_call_ref,sock->call_ref);
    if (sock->ep_ref >= 0) q_assign(&dsc,QF_ep_ref,sock->ep_ref);
    if (sock->ep_ref <= 0) { /* no AAL parameters if ep_ref present and != 0 */
	q_assign(&dsc,QF_aal_type,5);
#if defined(UNI30) || defined(DYNAMIC_UNI)
	if (sock->sig->uni == S_UNI30)
	    q_assign(&dsc,QF_aal_mode,1); /* Message mode - LANE seems to
					     really want this */
#endif
	q_assign(&dsc,QF_sscs_type,0); /* unspecified - LANE wants this */
	q_assign(&dsc,QF_fw_max_sdu,sock->qos.rxtp.max_sdu);
	q_assign(&dsc,QF_bw_max_sdu,sock->qos.txtp.max_sdu);
    }
    if ((size = q_close(&dsc)) >= 0) to_signaling(sock->sig,q_buffer,size);
    return 0;
}


#if defined(Q2963_1) || defined(DYNAMIC_UNI)

static void send_modify_request(SOCKET *sock)
{
    Q_DSC dsc;
    int size;

    q_create(&dsc,q_buffer,MAX_Q_MSG);
    q_assign(&dsc,QF_msg_type,ATM_MSG_MODIFY_REQ);
    q_assign(&dsc,QF_call_ref,sock->call_ref);
    if (sock->new_qos.txtp.traffic_class)
	q_assign(&dsc,QF_fw_pcr_01,SELECT_TOP_PCR(sock->new_qos.txtp));
    if (sock->new_qos.rxtp.traffic_class)
	q_assign(&dsc,QF_bw_pcr_01,SELECT_TOP_PCR(sock->new_qos.rxtp));
    if ((size = q_close(&dsc)) >= 0) to_signaling(sock->sig,q_buffer,size);
}


static void send_modify_ack(SOCKET *sock)
{
    Q_DSC dsc;
    int size;

    q_create(&dsc,q_buffer,MAX_Q_MSG);
    q_assign(&dsc,QF_msg_type,ATM_MSG_MODIFY_ACK);
    q_assign(&dsc,QF_call_ref,sock->call_ref);
    if (sock->qos.rxtp.traffic_class != ATM_NONE &&
      sock->qos.rxtp.max_pcr < sock->new_qos.rxtp.max_pcr) {
	q_assign(&dsc,QF_type_of_report,ATM_TOR_MOD_CONF);
	START_TIMER(sock,T361);
    }
    if ((size = q_close(&dsc)) >= 0) to_signaling(sock->sig,q_buffer,size);
}

#endif


static void dispatch(SOCKET *sock,struct atmsvc_msg *msg)
{
    int error;

    switch (msg->type) {
	case as_bind: /* only in NULL state */
	    if (sock) break;
	    if (msg->svc.sas_family != AF_ATMSVC) {
		SEND_ERROR(msg->vcc,-EAFNOSUPPORT);
		return;
	    }
	    if (!atmsvc_addr_in_use(msg->svc))
#ifdef BE_PICKY_ABOUT_BINDING_LOCAL_WILDCARD_ADDRESSES
		if (!get_local(NULL)) SEND_ERROR(msg->vcc,-EADDRNOTAVAIL);
		else
#endif
		send_kernel(msg->vcc,kptr_null,as_okay,0,NULL,NULL,
		  get_local(NULL),NULL,NULL);
	    else {
		if (!route_local(&msg->svc)) {
		    SEND_ERROR(msg->vcc,-EADDRNOTAVAIL);
		    return;
		}
		send_kernel(msg->vcc,kptr_null,as_okay,0,NULL,NULL,NULL,NULL,
		  NULL);
	    }
	    return;
	case as_connect: /* NULL state only */
	    if (sock) break;
	    if (!allow(&msg->svc,ACL_OUT)) {
		SEND_ERROR(msg->vcc,-EACCES);
		return;
	    }
	    sock = new_sock(msg->vcc);
	    sock->local = msg->local;
	    sock->remote = msg->svc;
	    sock->qos = msg->qos;
	    sock->sap = msg->sap;
	    sock->state = ss_connecting;
	    sock->pvc = msg->pvc;
#ifdef MULTIPOINT
	    sock->ct = p2p;
	    if (msg->session > 0)
		sock->ct = p2mp;
#endif
	    error = send_setup(sock);
	    if (error) {
		SEND_ERROR(msg->vcc,error);
		free_sock(sock);
		return;
	    }
	    START_TIMER(sock,T303);
	    new_state(sock,ss_connecting);
	    return;
	case as_accept:
	    if (sock->state == ss_zombie) {
		SEND_ERROR(msg->vcc,-ECONNABORTED); /* -ERESTARTSYS ? */
		free_sock(sock);
		return;
	    }
	    if (sock->state != ss_indicated && sock->state != ss_proceeding)
		break;
	    if (sock->state == ss_indicated && sock->sig->mode != sm_user)
		diag(COMPONENT,DIAG_FATAL,"No CI allocator (use -A)");
	    error = send_connect(sock);
	    if (!error) {
		START_TIMER(sock,T313);
		new_state(sock,ss_accepting);
		return;
	    }
	    SEND_ERROR(sock->id,error);
	    send_release(sock,0); /* @@@ */
	    START_TIMER(sock,T308_1);
	    new_state(sock,ss_wait_rel);
	    return;
	case as_reject: /* ZOMBIE, INDICATED, or PROCEEDING */
	    switch (sock->state) {
		case ss_indicated:
		    send_release_complete(sock->sig,sock->call_ref,
		      ATM_CV_CALL_REJ);
		    /* fall through */
		case ss_zombie:
		    free_sock(sock);
		    return;
		case ss_proceeding:
		    send_release(sock,ATM_CV_CALL_REJ);
			/* @@@ should use msg->reply */
		    START_TIMER(sock,T308_1);
		    new_state(sock,ss_wait_rel);
		    return;
		default:
		    break;
	    }
	    break;
	case as_listen: /* NULL */
	    if (sock) break;
	    if (msg->svc.sas_family != AF_ATMSVC) {
		SEND_ERROR(msg->vcc,-EAFNOSUPPORT);
		return;
	    }
	    if (msg->qos.aal != ATM_AAL5) {
		SEND_ERROR(msg->vcc,-EINVAL);
		return;
	    }
	    if (lookup_sap(&msg->svc,&msg->sap,&msg->qos,NULL,NULL,NULL,1)) {
		SEND_ERROR(msg->vcc,-EADDRINUSE);
		return;
	    }
	    sock = new_sock(msg->vcc);
	    sock->local = msg->svc;
	    sock->sap = msg->sap;
	    sock->qos = msg->qos;
	    send_kernel(sock->id,kptr_null,as_okay,0,NULL,NULL,NULL,NULL,NULL);
	    sock->state = ss_listening;
	    return;
	case as_close: /* all but INDICATED, PROCEEDING, ZOMBIE, and WAIT_REL */
	    if (sock && (sock->state == ss_indicated ||
	      sock->state == ss_proceeding || sock->state == ss_zombie ||
	      sock->state == ss_wait_rel)) break;
	    switch (sock ? sock->state : ss_null) {
		case ss_listening:
		    send_close(sock);
		    if (sock->listen) new_state(sock,ss_listen_zombie);
		    else free_sock(sock);
		    return;
		case ss_zombie:
		    send_close(sock);
		    /* fall through */
		case ss_wait_close:	/* network requested close */
		    free_sock(sock);
		    /* fall through */
		case ss_null:
		case ss_rel_req:
		    return;
		case ss_connecting:
		case ss_accepting:
#if defined(Q2963_1) || defined(DYNAMIC_UNI)
		case ss_mod_req:
#endif
		    STOP_TIMER(sock);
		    /* fall through */
#if defined(Q2963_1) || defined(DYNAMIC_UNI)
		case ss_mod_lcl:
		case ss_mod_rcv:
		case ss_mod_fin_ok:
		case ss_mod_fin_fail:
		case ss_mod_fin_ack:
#endif
		case ss_connected:
#if defined(Q2963_1) || defined(DYNAMIC_UNI)
		    if (timer_handler(sock->conn_timer) == on_T361)
			STOP_TIMER(sock);
#endif
		    if (sock->state == ss_connected)
			diag(COMPONENT,DIAG_INFO,"Active close (CR 0x%06X)",
			  sock->call_ref);
		    send_release(sock,
#ifdef DYNAMIC_UNI
		      sock->sig->uni != S_UNI30 ? ATM_CV_NORMAL_CLEAR :
		      ATM_CV_NORMAL_UNSPEC
#else
#if defined(UNI31) || defined(UNI40)
		      ATM_CV_NORMAL_CLEAR
#else
		      ATM_CV_NORMAL_UNSPEC
#endif
#endif
		      );
		    START_TIMER(sock,T308_1);
		    new_state(sock,ss_rel_req);
		    return;
		case ss_rel_ind:
		    send_release_complete(sock->sig,sock->call_ref,0); /* @@@ */
		    diag(COMPONENT, DIAG_INFO, "ss_rel_ind");
		    free_sock(sock);
		    return;
		default:
		    break;
	    }
	    break;
	case as_identify:
	    if (sock->state != ss_indicated && sock->state != ss_proceeding)
		break;
	    if (!atmpvc_addr_in_use(msg->pvc)) {
		if (sock->sig->mode == sm_switch)
		    diag(COMPONENT,DIAG_FATAL,"No CI allocator (use -A)");
		return;
	    }
	    if (sock->sig->mode == sm_net)
		diag(COMPONENT,DIAG_FATAL,"CI allocation role conflict");
	    sock->pvc = msg->pvc;
	    if (send_call_proceeding(sock))
		diag(COMPONENT,DIAG_FATAL,"s_c_p failed");
	    new_state(sock,ss_proceeding);
	    return;
#if defined(Q2963_1) || defined(DYNAMIC_UNI)
	case as_modify:
	    if (sock && !(sock->sig->uni & S_Q2963_1)) {
		send_kernel(sock->id,kptr_null,as_okay,-ENOPROTOOPT,NULL,NULL,
		  NULL,NULL,NULL);
		return;
	    }
	    if (sock && (sock->state == ss_mod_lcl ||
	      sock->state == ss_mod_req || sock->state == ss_mod_rcv ||
	      sock->state == ss_mod_fin_ok || sock->state == ss_mod_fin_fail ||
	      sock->state == ss_mod_fin_ack)) {
		send_kernel(sock->id,kptr_null,as_okay,-EALREADY,NULL,NULL,
		  NULL,NULL,NULL);
		return;
	    }
	    if (!sock || sock->state != ss_connected || !sock->owner) {
		send_kernel(sock->id,kptr_null,as_okay,-EBADFD,NULL,NULL,NULL,
		  NULL,NULL);
		return;
	    }
	    if (sock->qos.txtp.traffic_class != msg->qos.txtp.traffic_class ||
	      sock->qos.rxtp.traffic_class != msg->qos.rxtp.traffic_class ||
	      (sock->qos.txtp.traffic_class &&
	      (sock->qos.txtp.max_sdu != msg->qos.txtp.max_sdu)) ||
	      (sock->qos.rxtp.traffic_class &&
	      (sock->qos.rxtp.max_sdu != msg->qos.rxtp.max_sdu))) {
	/* @@@ may do more checking */
		send_kernel(sock->id,kptr_null,as_okay,-EINVAL,NULL,NULL,NULL,
		  NULL,NULL);
		return;
	    }
	    sock->new_qos = msg->qos;
	    send_kernel(sock->id,kptr_null,as_modify,
	      ATM_MF_INC_RSV | ATM_MF_DEC_SHP,NULL,NULL,NULL,NULL,&msg->qos);
	    new_state(sock,ss_mod_lcl);
	    return;
	case as_okay:
	    switch (sock ? sock->state : ss_null) {
		case ss_mod_lcl:
		    send_modify_request(sock);
		    START_TIMER(sock,T360);
		    new_state(sock,ss_mod_req);
		    return;
		case ss_mod_rcv:
		    send_modify_ack(sock);
		    sock->qos = sock->new_qos;
		    new_state(sock,ss_connected);
		    return;
		case ss_mod_fin_ok:
		    send_kernel(sock->id,kptr_null,as_okay,0,NULL,NULL,NULL,
		      NULL,NULL);
		    new_state(sock,ss_connected);
		    return;
		case ss_mod_fin_fail:
		    send_kernel(sock->id,kptr_null,as_okay,sock->error,NULL,
		      NULL,NULL,NULL,NULL);
		    sock->error = 0;
		    /* fall through */
		case ss_mod_fin_ack:
		    new_state(sock,ss_connected);
		    /* fall through */
		default:
		    return; /* ignore stray as_okay */
	    }
	case as_error:
	    switch (sock ? sock->state : ss_null) {
		case ss_mod_lcl:
		    send_kernel(sock->id,kptr_null,as_okay,msg->reply,NULL,
		      NULL,NULL,NULL,NULL);
		    new_state(sock,ss_connected);
		    return;
		case ss_mod_rcv:
		    send_modify_reject(sock,ATM_CV_RES_UNAVAIL);
		    new_state(sock,ss_connected);
		    return;
		case ss_mod_fin_ok:
		    diag(COMPONENT,DIAG_ERROR,"QOS commit failed");
		    send_kernel(sock->id,kptr_null,as_okay,0,NULL,NULL,NULL,
		      NULL,NULL);
		    /* @@@ clear call instead ? */
		    new_state(sock,ss_connected);
		    return;
		case ss_mod_fin_fail:
		    diag(COMPONENT,DIAG_ERROR,"QOS rollback failed");
		    send_kernel(sock->id,kptr_null,as_okay,sock->error,NULL,
		      NULL,NULL,NULL,NULL);
		    sock->error = 0;
		    /* @@@ clear call instead ? */
		    new_state(sock,ss_connected);
		    return;
		case ss_mod_fin_ack:
		    /* @@@ maybe we should even clear the call now */
		    diag(COMPONENT,DIAG_ERROR,"QOS commit failed");
		    new_state(sock,ss_connected);
		    return;
		default:
		    return; /* ignore stray as_error */
	    }
#endif
#ifdef MULTIPOINT
	case as_dropparty: {
		int leaves;

		/* This should send a RELEASE if there is only
		 * one party left on the call
		 */

		if (sock == NULL) {
			/* leaf was probably not found */
			send_kernel(msg->vcc, kptr_null, as_dropparty, -1,
				    NULL, NULL, NULL, NULL, NULL);
			return;
		}
		leaves = count_leaves(&sock->id);
		if ((sock->state == ss_connected) ||
		    (sock->state == ss_connecting)) {
			/* rely on short circuit boolean eval */
			if ((leaves > 1) && (send_dropparty(sock) < 0)) {
				send_kernel(sock->id, kptr_null, as_error, 0,
					    NULL, NULL, NULL, NULL, NULL);
			} else if (leaves > 1) {
				new_state(sock, ss_rel_req);
			} else if (leaves == 1) {
				send_release(sock, sock->sig->uni
					!= S_UNI30 ? ATM_CV_NORMAL_CLEAR :
					ATM_CV_NORMAL_UNSPEC);
			}
		} else {
			send_kernel(sock->id, kptr_null, as_error, 0,
				    NULL, NULL, NULL, NULL, NULL);
		}
		return;
	}

	case as_addparty:
		{
		int leafid = -1;
		SOCKET *ns = NULL;

		if (sock == NULL) {
			diag(COMPONENT, DIAG_INFO,
				"*sock is null.  Not adding party.");
		} else {
			/* Create a new socket for keeping state information
			 * that has the same "id" and call reference value
			 * as *sock, but should have a different endpoint
			 * reference.  Need to make sure that all routines
			 * that search for SOCKETS by id also compare endpoint
			 * references */

			ns = new_sock(msg->vcc);
			ns->call_ref = sock->call_ref;

			ns->local = msg->local;
			ns->remote = msg->svc;
			ns->qos = msg->qos;
			ns->sap = msg->sap;
			ns->state = ss_connecting;
			ns->pvc = msg->pvc;
			ns->sig = route_remote(&ns->remote);
			leafid = send_addparty(ns, &msg->svc);
			START_TIMER(ns,T399);
		}

		if (leafid > 0) {
			/* don't do anything here because we will notify
			 * the kernel of success or failure after the
			 * network responds with ATM_MSG_ADD_PARTY_ACK or
			 * ATM_MSG_ADD_PARTY_REJ */
		} else {
			/* notify kernel of failure because the addparty
			 * request was never sent */
			diag(COMPONENT, DIAG_INFO, "send_addparty failed");
			send_kernel(ns->id, kptr_null, as_error, 0,
				    NULL, NULL, NULL, NULL, NULL);
		}
		return;
		}
#endif
	default:
    	    diag(COMPONENT,DIAG_WARN,"invalid message %d",(int) msg->type);
	    return;
    }
    diag(COMPONENT,DIAG_WARN,"message %s is incompatible with state %s (%d)",
      as_name[msg->type],state_name[sock ? sock->state : ss_null],
      (int) (sock ? sock->state : ss_null));
}


static void dispatch_listen(SOCKET *sock,struct atmsvc_msg *msg)
{
    SOCKET *next;

    if (!sock) {
	diag(COMPONENT,DIAG_WARN,"message %s is incompatible with state %s "
	  "(%d)",as_name[msg->type],state_name[ss_null],ss_null);
	return;
    }
    if (!(next = sock->listen)) {
	diag(COMPONENT,DIAG_WARN,
	  "socket 0x%lx got accept/reject/identify with empty listen queue",
	  msg->vcc);
	return;
    }
    sock->listen = next->listen;
    if (sock->state == ss_listen_zombie && !sock->listen) free_sock(sock);
    next->listen = NULL;
    next->id = msg->vcc;
    dispatch(next,msg);
}


void sync_addr(VPCI *vpci)
{
    char buf[MAX_ATM_ADDR_LEN+1];
    LOCAL_ADDR *walk;

    (void) get_addr(vpci->itf,vpci->local_addr);
    for (walk = vpci->local_addr; walk->state != ls_unused; walk++) {
	if (atm2text(buf,MAX_ATM_ADDR_LEN+1,(struct sockaddr *)
	  &walk->addr,pretty) < 0) strcpy(buf,"<invalid>");
	switch (walk->state) {
	    case ls_added:
		diag(COMPONENT,DIAG_INFO,"Added local ATM address %s at "
		  "itf %d",buf,vpci->itf);
		walk->state = ls_same;
		break;
	    case ls_removed:
		diag(COMPONENT,DIAG_INFO,"Removed local ATM address %s at "
		  "itf %d",buf,vpci->itf);
/*
 *		walk->state = ls_unused;
 *
 * This is probably wrong. What if we delete an entry from the middle of
 * the list ? @@@
 */
		/* @@@ delete SVCs using that address ? */
		break;
	    default:
		break;
	}
    }
}


void from_kernel(struct atmsvc_msg *msg,int size)
{
    void (*dispatcher)(SOCKET *,struct atmsvc_msg *);
    SOCKET *curr;

    if (msg->type == as_itf_notify) {
	itf_reload(msg->pvc.sap_addr.itf);
	return;
    }
    if (msg->type == as_terminate) {
#if 0 /* need to pass some ID ... @@@ */
	if (mode != sm_switch) {
	    diag(COMPONENT,DIAG_ERROR,"Ignoring as_terminate received in %s "
	      "mode",mode == sm_user ? "USER" : "NETWORK");
	    return;
	}
	clear_all_calls(sig);
#endif
	stop = 1;
	diag(COMPONENT,DIAG_INFO,"Going down on as_terminate");
	/*
	 * Fix this - we need to shut it down more gracefully.
	 */
	return;
    }
    if (!kptr_eq(&msg->listen_vcc,&kptr_null) && (msg->type == as_accept ||
      msg->type == as_reject || msg->type == as_identify)) {
	dispatcher = dispatch_listen;
	for (curr = sockets; curr; curr = curr->next)
	    if (kptr_eq(&msg->listen_vcc,&curr->id) &&
	      (curr->state == ss_listening || curr->state == ss_listen_zombie))
		break;
    }
#ifdef MULTIPOINT
    else if (msg->type == as_dropparty) {
	/* check the endpoint reference while we search */
	dispatcher = dispatch;
	for (curr = sockets; curr; curr = curr->next) {
		if (kptr_eq(&msg->vcc, &curr->id) &&
		    (curr->ep_ref == msg->reply)) break;
	}
	if (curr == NULL) diag(COMPONENT, DIAG_INFO, "leaf not found");
    }
#endif
    else {
	dispatcher = dispatch;
	for (curr = sockets; curr; curr = curr->next)
	    if (kptr_eq(&msg->vcc,&curr->id)) break;
    }
    diag(COMPONENT,DIAG_DEBUG,"FROM KERNEL: %s for socket %p (%s/%s) "
      "in state %s",as_name[msg->type],curr,kptr_print(&msg->vcc),
      kptr_print(&msg->listen_vcc),state_name[curr ? curr->state : ss_null]);
    dispatcher(curr,msg);
}
