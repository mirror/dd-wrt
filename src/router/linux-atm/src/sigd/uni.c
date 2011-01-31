/* uni.c - Processing of incoming UNI signaling messages */
 
/* Written 1995-2000 by Werner Almesberger, EPFL-LRC/ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include <atm.h>
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
#include "trace.h"


#define COMPONENT "UNI"


extern const char *cause_text[]; /* from mess.c */


static Q_DSC in_dsc;
static TIMER *t309 = NULL;


static unsigned short cvt_ep_ref(SIG_ENTITY *sig,unsigned short ep_ref)
{
    return sig->uni == S_UNI30 ? ep_ref & 0x7fff : ep_ref ^ 0x8000;
}


int send_call_proceeding(SOCKET *sock)
{
    Q_DSC dsc;
    int size;

    q_create(&dsc,q_buffer,MAX_Q_MSG);
    q_assign(&dsc,QF_msg_type,ATM_MSG_CALL_PROC);
    q_assign(&dsc,QF_call_ref,sock->call_ref);
    if (sock->sig->mode == sm_net) {
	int vpci,vci;

	sock->pvc.sap_family = AF_ATMPVC;
	vpci = sock->sig->signaling_pvc.sap_addr.itf;
	sock->pvc.sap_addr.itf = get_itf(sock->sig,&vpci);
	sock->pvc.sap_addr.vpi = vpci;
	vci = get_vci(sock->pvc.sap_addr.itf);
	if (vci < 0) {
	    (void) q_close(&dsc);
	    return vci;
	}
	sock->pvc.sap_addr.vci = vci;
    }
    if (sock->sig->mode != sm_user) {
	q_assign(&dsc,QF_vpi,sock->pvc.sap_addr.vpi);
	q_assign(&dsc,QF_vci,sock->pvc.sap_addr.vci);
    }
    if (sock->ep_ref >= 0) q_assign(&dsc,QF_ep_ref,sock->ep_ref);
    if ((size = q_close(&dsc)) >= 0) to_signaling(sock->sig,q_buffer,size);
    return 0;
}


static void setup_call(SIG_ENTITY *sig,unsigned long call_ref)
{
    SOCKET *sock,*this,**walk;
    struct sockaddr_atmsvc in_addr;
    struct atm_sap in_sap;
    struct atm_qos in_qos;
    unsigned int problem;
    int i;

    problem = sap_decode(&in_dsc,&in_addr,&in_sap,&in_qos,sig->uni);
    if (problem) {
	send_release_complete(sig,call_ref,IE_PB_CAUSE(problem),
	  IE_PB_IE(problem));
	return;
    }
    if (!atmsvc_addr_in_use(in_addr)) {
	send_release_complete(sig,call_ref,ATM_CV_UNALLOC);
	return;
    }
    if (!allow(&in_addr,ACL_IN)) {
	send_release_complete(sig,call_ref,ATM_CV_REJ_CLIR);
	return;
    }
    this = new_sock(kptr_null);
    this->sig = sig;
    sock = lookup_sap(&in_addr,&in_sap,&in_qos,&this->local,&this->sap,
      &this->qos,0);
    if (!sock) {
	free_sock(this);
	send_release_complete(sig,call_ref,ATM_CV_INCOMP_DEST);
	return;
    }
    this->state = sig->mode == sm_net ? ss_proceeding : ss_indicated;
    this->call_state = cs_in_proc;
    this->call_ref = call_ref;
    if (q_present(&in_dsc,QF_ep_ref))
	this->ep_ref = cvt_ep_ref(sig,q_fetch(&in_dsc,QF_ep_ref));
#ifdef CISCO
    else
#endif
    if (sig->mode == sm_net) {
	int error;

	error = send_call_proceeding(this);
	if (error) {
	    free_sock(this);
	    send_release_complete(sig,call_ref,ATM_CV_NO_CI);
	    return;
	}
    }
    /* if (sock->local) *this->local->sas_addr = sock->local->sas_addr; ??? */
    diag(COMPONENT,DIAG_DEBUG,"AAL type %ld",q_fetch(&in_dsc,QF_aal_type));
    if (sig->mode == sm_user) { /* already set by send_call_proceeding */
	int vpci;

	vpci = q_fetch(&in_dsc,QF_vpi);
	this->pvc.sap_family = AF_ATMPVC;
	this->pvc.sap_addr.itf = get_itf(sig,&vpci);
	this->pvc.sap_addr.vpi = vpci;
	this->pvc.sap_addr.vci = q_fetch(&in_dsc,QF_vci);
    }
    diag(COMPONENT,DIAG_DEBUG,"ITF.VPI.VCI: %d.%d.%d",this->pvc.sap_addr.itf,
      this->pvc.sap_addr.vpi,this->pvc.sap_addr.vci);
    if (q_present(&in_dsc,QF_cgpn)) { /* should handle E.164 too */
	char buffer[MAX_ATM_ADDR_LEN+1];
	int plan;

	plan = q_fetch(&in_dsc,QF_cgpn_plan);
	switch (plan) {
	    case ATM_NP_AEA:
		i = q_read(&in_dsc,QF_cgpn,(void *) this->remote.sas_addr.prv,
		  ATM_ESA_LEN);
		break;
	    case ATM_NP_E164:
		i = q_read(&in_dsc,QF_cgpn,(void *) this->remote.sas_addr.pub,
		  ATM_E164_LEN);
		break;
	    default:
		diag(COMPONENT,DIAG_WARN,"Ignoring cgpn with unrecognized "
		  "numbering plan 0x%x\n",plan);
		i = 0;
	}
	if (i) {
	    this->remote.sas_family = AF_ATMSVC;
	    if (atm2text(buffer,MAX_ATM_ADDR_LEN+1,
	      (struct sockaddr *) &this->remote,pretty) < 0)
		strcpy(buffer,"<invalid address>");
	    diag(COMPONENT,DIAG_DEBUG,"Incoming call from %s",buffer);
	}
    }
    send_kernel(kptr_null,sock->id,as_indicate,0,&this->pvc,&this->remote,
      &in_addr,&this->sap,&this->qos);
    for (walk = &sock->listen; *walk; walk = &(*walk)->listen);
    *walk = this;
    diag(COMPONENT,DIAG_DEBUG,"SE vpi.vci=%d.%d",this->pvc.sap_addr.vpi,
      this->pvc.sap_addr.vci);
}


static void send_status(SIG_ENTITY *sig,SOCKET *sock,unsigned long call_ref,
  unsigned char cause,...)
{
    va_list ap;
    Q_DSC dsc;
    int size;

    q_create(&dsc,q_buffer,MAX_Q_MSG);
    q_assign(&dsc,QF_msg_type,ATM_MSG_STATUS);
    if (sock) {
	q_assign(&dsc,QF_call_ref,sock->call_ref);
	q_assign(&dsc,QF_call_state,(int) sock->call_state);
	if (sock->ep_ref >= 0) {
	    q_assign(&dsc,QF_ep_ref,sock->ep_ref);
	    q_assign(&dsc,QF_ep_state,eps_map[sock->call_state]);
	}
    }
    else {
	q_assign(&dsc,QF_call_ref,call_ref);
	q_assign(&dsc,QF_call_state,0); /* U0 - Null / REST 0 - Null */
    }
    q_assign(&dsc,QF_cause,cause);
    va_start(ap,cause);
    switch (cause) {
	case ATM_CV_UNKNOWN_MSG_TYPE:
	case ATM_CV_INCOMP_MSG:
	    q_assign(&dsc,QF_bad_msg_type,va_arg(ap,unsigned int));
	    break;
	case ATM_CV_MAND_IE_MISSING:
	case ATM_CV_INVALID_IE:
	    {
		unsigned char ie;

		ie = va_arg(ap,unsigned int);
		q_write(&dsc,QF_ie_id6,&ie,1);
		break;
	    }
	default:
	    ;
    }
    va_end(ap);
    if ((size = q_close(&dsc)) >= 0) to_signaling(sig,q_buffer,size);
}


static void send_status_enq(SOCKET *sock)
{
    Q_DSC dsc;
    int size;

    q_create(&dsc,q_buffer,MAX_Q_MSG);
    q_assign(&dsc,QF_msg_type,ATM_MSG_STATUS_ENQ);
    q_assign(&dsc,QF_call_ref,sock->call_ref);
    if (sock->ep_ref >= 0) q_assign(&dsc,QF_ep_ref,sock->ep_ref);
    if ((size = q_close(&dsc)) >= 0) to_signaling(sock->sig,q_buffer,size);
    /* @@@ should start T322 */
}


static void send_connect_ack(SOCKET *sock)
{
    Q_DSC dsc;
    int size;

    q_create(&dsc,q_buffer,MAX_Q_MSG);
    q_assign(&dsc,QF_msg_type,ATM_MSG_CONN_ACK);
    q_assign(&dsc,QF_call_ref,sock->call_ref);
    if ((size = q_close(&dsc)) >= 0) to_signaling(sock->sig,q_buffer,size);
}


static void send_restart_ack(SIG_ENTITY *sig,unsigned long call_ref,int vpi,
  int vci)
{
    Q_DSC dsc;
    int size;

    q_create(&dsc,q_buffer,MAX_Q_MSG);
    q_assign(&dsc,QF_msg_type,ATM_MSG_REST_ACK);
    q_assign(&dsc,QF_call_ref,call_ref);
    if (!vpi && !vci) q_assign(&dsc,QF_rst_class,ATM_RST_ALL_VC);
    else {
	q_assign(&dsc,QF_rst_class,ATM_RST_IND_VC);
	q_assign(&dsc,QF_vpi,vpi);
	q_assign(&dsc,QF_vci,vci);
    }
    if ((size = q_close(&dsc)) >= 0) to_signaling(sig,q_buffer,size);
}


static void send_drop_party_ack(SIG_ENTITY *sig,unsigned long call_ref,
  unsigned short ep_ref,unsigned char cause)
{
    Q_DSC dsc;
    int size;

    q_create(&dsc,q_buffer,MAX_Q_MSG);
    q_assign(&dsc,QF_msg_type,ATM_MSG_DROP_PARTY_ACK);
    q_assign(&dsc,QF_call_ref,call_ref);
    q_assign(&dsc,QF_ep_ref,ep_ref);
    q_assign(&dsc,QF_cause,cause);
    if ((size = q_close(&dsc)) >= 0) to_signaling(sig,q_buffer,size);
}


#if defined(Q2963_1) || defined(DYNAMIC_UNI)

static void send_conn_avail(SOCKET *sock)
{
    Q_DSC dsc;
    int size;

    q_create(&dsc,q_buffer,MAX_Q_MSG);
    q_assign(&dsc,QF_msg_type,ATM_MSG_CONN_AVAIL);
    q_assign(&dsc,QF_call_ref,sock->call_ref);
    if ((size = q_close(&dsc)) >= 0) to_signaling(sock->sig,q_buffer,size);
}

#endif


static void uni_call(SOCKET *sock,unsigned char mid)
{
    char buffer[MAX_ATM_ADDR_LEN+1];
    int error;

    switch (mid) {
	case ATM_MSG_STATUS: /* 5.5.6.12 */
	    {
		CALL_STATE state;

		/*
		 * NOTE: T322 isn't implemented yet, but when it is, make sure
		 *	 to only stop it on STATUS iff the cause is
		 *	 ATM_CV_RESP_STAT_ENQ. Supplementary services break if
		 *	 you stop on any STATUS.
		 */
		state = q_fetch(&in_dsc,QF_call_state);
		if (state == cs_null) break; /* clear call */
		if (sock->call_state == cs_rel_req || sock->call_state ==
		  cs_rel_ind) return;
		if (state != sock->call_state)
		    diag(COMPONENT,DIAG_WARN,"STATUS %s received in state %s",
		      cs_name[state],cs_name[sock->call_state]);
	    }
	    return;
	default:
	    ;
    }
    switch (mid) {
	case ATM_MSG_CALL_PROC: /* CONNECTING, WAIT_REL, REL_REQ */
	    if (sock->state == ss_wait_rel || sock->state == ss_rel_req) {
		send_status(sock->sig,sock,0,ATM_CV_INCOMP_MSG,
		  ATM_MSG_CALL_PROC);
		return;
	    }
	    if (sock->state != ss_connecting) break;
	    /* check for 2nd CALL_PROC @@@ */
	    STOP_TIMER(sock);
	    if (q_present(&in_dsc,QG_conn_id)) {
		int vpci;

		vpci = q_fetch(&in_dsc,QF_vpi);
		sock->pvc.sap_family = AF_ATMPVC;
		sock->pvc.sap_addr.itf = get_itf(sock->sig,&vpci);
		sock->pvc.sap_addr.vpi = vpci;
		sock->pvc.sap_addr.vci = q_fetch(&in_dsc,QF_vci);
		diag(COMPONENT,DIAG_DEBUG,"ITF.VPI.VCI: %d.%d.%d",
		  sock->pvc.sap_addr.itf,sock->pvc.sap_addr.vpi,
		  sock->pvc.sap_addr.vci);
	    }
	    START_TIMER(sock,T310);
	    sock->call_state = cs_out_proc;
	    return;
	case ATM_MSG_CONNECT: /* CONNECTING, REL_REQ */
	    if (sock->state == ss_rel_req) {
		send_status(sock->sig,sock,0,ATM_CV_INCOMP_MSG,ATM_MSG_CONNECT);
		return;
	    }
	    if (sock->state != ss_connecting) break;
	    STOP_TIMER(sock);
	    if (q_present(&in_dsc,QG_conn_id)) {
		int vpci;

		vpci = q_fetch(&in_dsc,QF_vpi);
		sock->pvc.sap_family = AF_ATMPVC;
		sock->pvc.sap_addr.itf = get_itf(sock->sig,&vpci);
		sock->pvc.sap_addr.vpi = vpci;
		sock->pvc.sap_addr.vci = q_fetch(&in_dsc,QF_vci);
		diag(COMPONENT,DIAG_DEBUG,"ITF.VPI.VCI: %d/%d.%d",
		  sock->pvc.sap_addr.itf,sock->pvc.sap_addr.vpi,
		  sock->pvc.sap_addr.vci);
	    }
	    error = 0;
	    if (!sock->pvc.sap_addr.vpi && !sock->pvc.sap_addr.vci)
		error = -EPROTO;
	    /* more problems */
	    if (error) {
		set_error(sock,error);
		send_release(sock,0); /* @@@ cause follows reason ??? */
		START_TIMER(sock,T308_1);
		new_state(sock,ss_rel_req);
		return;
	    }
	    send_connect_ack(sock);
	    /* @@@ fill in sock->remote */
	    /* @@@ fill in traffic parameters */
	    send_kernel(sock->id,kptr_null,as_okay,0,&sock->pvc,NULL,
	      &sock->local,&sock->sap,&sock->qos);
	    new_state(sock,ss_connected);
#if defined(Q2963_1) || defined(DYNAMIC_UNI)
	    sock->owner = 1;
#endif
	    if (atm2text(buffer,MAX_ATM_ADDR_LEN+1,(struct sockaddr *)
	      &sock->remote,0) < 0) strcpy(buffer,"<invalid>");
	    diag(COMPONENT,DIAG_INFO,"Active open succeeded (CR 0x%06X, "
	      "ID %s, to %s)",sock->call_ref,kptr_print(&sock->id),buffer);
	    return;
	case ATM_MSG_CONN_ACK: /* ACCEPTING, WAIT_REL, REL_REQ */
	    diag(COMPONENT,DIAG_DEBUG,"CA vpi.vci=%d.%d",
	      sock->pvc.sap_addr.vpi,sock->pvc.sap_addr.vci);
	    if (sock->state == ss_wait_rel || sock->state == ss_rel_req) {
		send_status(sock->sig,sock,0,ATM_CV_INCOMP_MSG,
		  ATM_MSG_CONN_ACK);
		return;
	    }
	    if (sock->state != ss_accepting) break;
	    STOP_TIMER(sock);
	    send_kernel(sock->id,kptr_null,as_okay,0,NULL,NULL,&sock->local,
	      &sock->sap,NULL);
	    new_state(sock,ss_connected);
#if defined(Q2963_1) || defined(DYNAMIC_UNI)
	    sock->owner = 0;
#endif
	    if (atm2text(buffer,MAX_ATM_ADDR_LEN+1, (struct sockaddr *)
	      &sock->remote,0) < 0) strcpy(buffer,"<invalid>");
	    diag(COMPONENT,DIAG_INFO,"Passive open succeeded (CR 0x%06X, "
	      "ID %s, from %s)",sock->call_ref,kptr_print(&sock->id),buffer);
	    return;
	case ATM_MSG_RELEASE: /* all states */
	    {
		unsigned char cause;

		cause = q_fetch(&in_dsc,QF_cause);
		diag(COMPONENT,DIAG_DEBUG,"Cause %d (%s)",cause,cause > 127 ?
		  "invalid cause" : cause_text[cause]);
	    }
	    switch (sock->state) {
		case ss_connecting:
		    set_error(sock,-ECONNREFUSED);
		    /* fall through */
		case ss_accepting:
		    set_error(sock,-ECONNRESET); /* ERESTARTSYS ? */
		    send_release_complete(sock->sig,sock->call_ref,0);
		    SEND_ERROR(sock->id,sock->error);
		    STOP_TIMER(sock);
		    free_sock(sock);
		    return;
		case ss_rel_req:
		    send_close(sock);
		    /* fall through */
		case ss_wait_rel:
		    STOP_TIMER(sock);
		    free_sock(sock);
		    return;
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
		    diag(COMPONENT,DIAG_INFO,"Passive close (CR 0x%06X)",
		      sock->call_ref);
#if defined(Q2963_1) || defined(DYNAMIC_UNI)
		    if (timer_handler(sock->conn_timer) == on_T361)
			STOP_TIMER(sock);
#endif
		    send_close(sock);
		    new_state(sock,ss_rel_ind);
		    return;
		case ss_indicated:
		    /* fall through */
		case ss_proceeding:
		    send_release_complete(sock->sig,sock->call_ref,0);
		    new_state(sock,ss_zombie);
		    /* fall through */
		case ss_rel_ind:
		    return;
		default:
		    send_release_complete(sock->sig,sock->call_ref,0);
			/* @@@ should be ATM_CV_INCOMP_MSG */
		    break;
	    }
	    break;
	case ATM_MSG_RESTART:
		set_error(sock,-ENETRESET);
		/* fall through */
	case ATM_MSG_STATUS: /* fall through when clearing */
	case ATM_MSG_REL_COMP: /* basically any state (except LISTENING and
				  ZOMBIE) */
	    {
		unsigned char cause;

		if (mid != ATM_MSG_REL_COMP || !q_present(&in_dsc,QF_cause))
		    cause = 0;
		else {
		    cause = q_fetch(&in_dsc,QF_cause);
		    diag(COMPONENT,DIAG_DEBUG,"Cause %d (%s)",cause,
		      cause > 127 ? "invalid cause" : cause_text[cause]);
		}
		switch (sock->state) {
		    case ss_connecting:
			set_error(sock,cause == ATM_CV_UNALLOC ?
			  -EADDRNOTAVAIL : cause == ATM_CV_RES_UNAVAIL ||
#if defined(UNI31) || defined(UNI40) || defined(DYNAMIC_UNI)
			  cause == ATM_CV_UCR_UNAVAIL_NEW ||
#endif
			  cause == ATM_CV_NO_ROUTE_DEST ? -EHOSTUNREACH :
			  cause == ATM_CV_NUM_CHANGED ? -EREMCHG :
			  cause == ATM_CV_DEST_OOO ? -EHOSTDOWN :
			  -ECONNREFUSED);
			/* fall through */
		    case ss_accepting:
			set_error(sock,-ECONNRESET); /* ERESTARTSYS ? */
			SEND_ERROR(sock->id,sock->error);
			STOP_TIMER(sock);
			free_sock(sock);
			return;
		    case ss_rel_req:
			send_close(sock);
			/* fall through */
		    case ss_wait_rel:
			STOP_TIMER(sock);
#ifdef MULTIPOINT
			/* Need to free all endpoints if this is a pmp call */
			if (sock->ct == p2mp)
				free_leaves(&sock->id);
			else
#endif
				free_sock(sock);
			return;
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
			diag(COMPONENT,DIAG_INFO,"Passive close (CR 0x%06X)",
			  sock->call_ref);
#if defined(Q2963_1) || defined(DYNAMIC_UNI)
			if (timer_handler(sock->conn_timer) == on_T361)
			    STOP_TIMER(sock);
#endif
			send_close(sock);
			/* fall through */
		    case ss_rel_ind:
			new_state(sock,ss_wait_close);
			return;
		    case ss_indicated:
			/* fall through */
		    case ss_proceeding:
			new_state(sock,ss_zombie);
			return;
		    default:
			break;
		}
		break; /* fail */
	    }
	case ATM_MSG_ALERTING:
	    /*
	     * We basically ignore this junk message, except for the connection
	     * identifier it may carry.
	     */
	    if (q_present(&in_dsc,QG_conn_id)) {
		int vpci;

		vpci = q_fetch(&in_dsc,QF_vpi);
		sock->pvc.sap_family = AF_ATMPVC;
		sock->pvc.sap_addr.itf = get_itf(sock->sig,&vpci);
		sock->pvc.sap_addr.vpi = vpci;
		sock->pvc.sap_addr.vci = q_fetch(&in_dsc,QF_vci);
		diag(COMPONENT,DIAG_DEBUG,"ITF.VPI.VCI: %d.%d.%d",
		  sock->pvc.sap_addr.itf,sock->pvc.sap_addr.vpi,
		  sock->pvc.sap_addr.vci);
	    }
	    return;
	case ATM_MSG_NOTIFY:
		/* silently ignore this junk */
	    return;
#ifdef MULTIPOINT
	case ATM_MSG_ADD_PARTY_REJ:
	    STOP_TIMER(sock);
	    send_drop_party_ack(sock->sig,sock->call_ref,sock->ep_ref,ATM_CV_CALL_REJ);
	    send_kernel(sock->id, kptr_null, as_addparty, -1,
			NULL, NULL, NULL, NULL, NULL);
	    diag(COMPONENT, DIAG_INFO, "addparty failed");
		 /* If no other leaves, should close entire call */
	    free_sock(sock);
	    return;
	case ATM_MSG_ADD_PARTY_ACK:
	    STOP_TIMER(sock);
	    send_kernel(sock->id, kptr_null, as_addparty, sock->ep_ref,
			NULL, NULL, NULL, NULL, NULL);
	    diag(COMPONENT, DIAG_INFO, "addparty succeeded");
	    new_state(sock, ss_connected);
	    return;
	case ATM_MSG_DROP_PARTY_ACK:
	    STOP_TIMER(sock);
	    send_kernel(sock->id, kptr_null, as_dropparty, 0,
			NULL, NULL, NULL, NULL, NULL);
	    diag(COMPONENT, DIAG_INFO, "dropparty succeeded");
	    free_sock(sock);
	    return;
        case ATM_MSG_DROP_PARTY:
	    diag(COMPONENT, DIAG_INFO, "received dropparty"); 
	    switch (sock->state) {
		case ss_connecting:
		case ss_connected:
		case ss_rel_req:
		case ss_wait_rel:
		    send_drop_party_ack(sock->sig, sock->call_ref, sock->ep_ref,
					ATM_CV_NORMAL_CLEAR);
		    break;
		default:
		    send_drop_party_ack(sock->sig, sock->call_ref, sock->ep_ref, 0);
	    }
	    free_sock(sock);
	    return;
#endif
#if defined(Q2963_1) || defined(DYNAMIC_UNI)
/*
 * Buglet ahead: should actually test "call_state"
 */
	case ATM_MSG_MODIFY_REQ:
	    if (!(sock->sig->uni & S_Q2963_1)) goto _default;
	    if (sock->state != ss_connected || sock->owner) break;
	    sock->new_qos = sock->qos;
	    if (q_present(&in_dsc,QF_fw_pcr_01))
		sock->new_qos.rxtp.max_pcr = q_fetch(&in_dsc,QF_fw_pcr_01);
	    if (q_present(&in_dsc,QF_bw_pcr_01))
		sock->new_qos.txtp.max_pcr = q_fetch(&in_dsc,QF_bw_pcr_01);
	    send_kernel(sock->id,kptr_null,as_modify,
	      ATM_MF_INC_RSV | ATM_MF_DEC_RSV | ATM_MF_DEC_SHP,
	      NULL,NULL,NULL,NULL,&sock->new_qos);
	    new_state(sock,ss_mod_rcv);
	    return;
	case ATM_MSG_MODIFY_ACK:
	    if (!(sock->sig->uni & S_Q2963_1)) goto _default;
	    if (sock->state != ss_mod_req) break;
	    STOP_TIMER(sock);
	    sock->qos = sock->new_qos;
	    if (q_present(&in_dsc,QG_bbrt)) send_conn_avail(sock);
	    send_kernel(sock->id,kptr_null,as_modify,ATM_MF_SET,NULL,NULL,NULL,
	      NULL,&sock->qos);
	    new_state(sock,ss_mod_fin_ok);
	    return;
	case ATM_MSG_MODIFY_REJ:
	    if (!(sock->sig->uni & S_Q2963_1)) goto _default;
	    if (sock->state != ss_mod_req) break;
	    STOP_TIMER(sock);
	    sock->error = -EAGAIN;
	    send_kernel(sock->id,kptr_null,as_modify,ATM_MF_SET,NULL,NULL,NULL,
	      NULL,&sock->qos);
	    new_state(sock,ss_mod_fin_fail);
	    return;
	case ATM_MSG_CONN_AVAIL:
	    if (!(sock->sig->uni & S_Q2963_1)) goto _default;
	    if (sock->state != ss_connected || sock->owner) break;
	    STOP_TIMER(sock);
	    send_kernel(sock->id,kptr_null,as_modify,ATM_MF_SET,NULL,NULL,NULL,
	      NULL,&sock->qos);
	    new_state(sock,ss_mod_fin_ack);
	    return;
	_default: /* jump here if we don't want to understand a message */
#endif
	default:
	    diag(COMPONENT,DIAG_WARN,"Bad signaling message %d",mid);
	    send_status(sock->sig,sock,0,ATM_CV_UNKNOWN_MSG_TYPE,mid);
	    return;
    }
    diag(COMPONENT,DIAG_WARN,
      "Signaling message %s is incompatible with state %s/%s (%d?%d)",
      mid2name(mid),state_name[sock->state],cs_name[sock->call_state],
      (int) sock->state,(int) sock->call_state);
    send_status(sock->sig,sock,0,ATM_CV_INCOMP_MSG,mid);
}


void clear_all_calls(SIG_ENTITY *sig)
{
    SOCKET *curr,*next;

    for (curr = sockets; curr; curr = next) {
	next = curr->next;
	if (curr->sig == sig && curr->call_state != cs_null)
	    uni_call(curr,ATM_MSG_RESTART);
    }
}


void clear_all_calls_on_T309(SIG_ENTITY *sig)
{
    clear_all_calls(sig);
    t309 = NULL;
}


void saal_failure(SIG_ENTITY *sig)
{
    SOCKET *curr,*next;

    trace_msg("SAAL went down");
    for (curr = sockets; curr; curr = next) {
	next = curr->next;
	if (curr->sig != sig || curr->call_state == cs_null) continue;
	if (curr->call_state != cs_active)
	    uni_call(curr,ATM_MSG_RESTART);
	else if (!t309) t309 = start_timer(T309_TIME,on_T309,sig);
    }
}


void saal_okay(SIG_ENTITY *sig)
{
    SOCKET *curr;

    trace_msg("SAAL came up");
#ifdef THOMFLEX
    /*
     * Some versions of the Thomson Thomflex 5000 won't do any signaling before
     * they get a RESTART. Whenever SAAL comes up, this may indicate that the
     * switch got booted, so we send that RESTART. We also have to clear all
     * pending connections, which isn't that nice ... Note that the rest of the
     * RESTART state machine is not implemented, so the RESTART ACKNOWLEDGE
     * will yield a warning.
     */
    {
	Q_DSC dsc;
	int size;

	clear_all_calls(sig);
	q_create(&dsc,q_buffer,MAX_Q_MSG);
	q_assign(&dsc,QF_msg_type,QMSG_RESTART);
	q_assign(&dsc,QF_call_ref,0);
	q_assign(&dsc,QF_rst_class,ATM_RST_ALL_VC);
	if ((size = q_close(&dsc)) >= 0) to_signaling(sig,q_buffer,size);
    }
#endif
    if (!t309) return;
    stop_timer(t309);
    t309 = NULL;
    for (curr = sockets; curr; curr = curr->next)
	if (curr->sig == sig && curr->call_state != cs_null)
	    send_status_enq(curr);
}


static void process_uni(SIG_ENTITY *sig,void *msg)
{
    SOCKET *curr;
    unsigned long call_ref;
    unsigned short ep_ref;
    unsigned char mid;

    call_ref = q_fetch(&in_dsc,QF_call_ref)^0x800000;
    mid = q_fetch(&in_dsc,QF_msg_type);
    if (mid == ATM_MSG_REST_ACK) return;
    if (mid == ATM_MSG_RESTART) { /* 5.5.5.2 */
	int rst_class;

	rst_class = q_fetch(&in_dsc,QF_rst_class);
	switch (rst_class) {
	    case ATM_RST_IND_VC:
		{
		    int vpi,vci;

		    if (!q_present(&in_dsc,QG_conn_id)) {
			send_status(sig,NULL,call_ref,ATM_CV_MAND_IE_MISSING,
			  ATM_IE_CONN_ID);
			return;
		    }
		    vpi = q_fetch(&in_dsc,QF_vpi);
		    vci = q_fetch(&in_dsc,QF_vci);
		    for (curr = sockets; curr; curr = curr->next)
			if (curr->sig == sig && curr->pvc.sap_addr.vpi == vpi &&
			  curr->pvc.sap_addr.vci == vci) break;
		    if (!curr) {
			send_status(sig,NULL,call_ref,ATM_CV_INVALID_IE,
			  ATM_IE_CONN_ID);
			return;
		    }
		    uni_call(curr,mid);
		    send_restart_ack(sig,call_ref,vpi,vci);
		}
		break;
	    case ATM_RST_ALL_VC:
		clear_all_calls(sig);
		send_restart_ack(sig,call_ref,0,0);
		break;
	    default:
		send_status(sig,NULL,call_ref,ATM_CV_INVALID_IE,ATM_IE_RESTART);
	}
	return;
    }
    if (!(call_ref & 0x7fffff)) {
	diag(COMPONENT,DIAG_ERROR,"unrecognized global call ref");
	return;
    }

#ifdef MULTIPOINT
    if (q_present(&in_dsc,QF_ep_ref)) {
	    ep_ref = cvt_ep_ref(sig,q_fetch(&in_dsc,QF_ep_ref));
	    for (curr = sockets; curr; curr = curr->next)
		if (curr->sig == sig &&
		    curr->call_ref == call_ref &&
		    curr->ep_ref == ep_ref) break;
    } else
#endif
	    for (curr = sockets; curr; curr = curr->next)
		if (curr->sig == sig && curr->call_ref == call_ref) break;
    diag(COMPONENT,DIAG_DEBUG,"FROM SAAL %d.%d.%d: %s (0x%02X) CR 0x%06lx for "
      "%s",S_PVC(sig),mid2name(((unsigned char *) msg)[5]),
      ((unsigned char *) msg)[5],call_ref,curr ? kptr_print(&curr->id) : "?");
    if (mid == ATM_MSG_SETUP) {
	if (!curr) setup_call(sig,call_ref);
	return;
    }
    if (mid == ATM_MSG_STATUS_ENQ) {
	send_status(sig,curr,call_ref,ATM_CV_RESP_STAT_ENQ);
	return;
    }
    if (curr && q_present(&in_dsc,QF_ep_ref) && mid != ATM_MSG_ADD_PARTY &&
      mid != ATM_MSG_DROP_PARTY_ACK) {
	ep_ref = cvt_ep_ref(sig,q_fetch(&in_dsc,QF_ep_ref));
	if (curr->ep_ref != ep_ref) {
	    send_drop_party_ack(sig,call_ref,ep_ref,ATM_CV_INV_EPR);
	    return;
	}
    }
    if (!curr || curr->call_state == cs_null) {
	if (mid == ATM_MSG_REL_COMP) return;
	if (mid != ATM_MSG_STATUS)
	    send_release_complete(sig,call_ref,ATM_CV_INV_CR);
	else if (q_fetch(&in_dsc,QF_call_state) != (int) cs_null)
		send_release_complete(sig,call_ref,ATM_CV_INCOMP_MSG);
	return;
    }
    uni_call(curr,mid);
}


static void abort_call(SIG_ENTITY *sig,unsigned char *msg,int size)
{
    SOCKET *curr;
    unsigned long call_ref;

    if (size < 6) {
	diag(COMPONENT,DIAG_ERROR,"message too short (%d bytes)",size);
	return;
    }
    /* hope that at least the call ref is okay ... */
    call_ref = ((msg[3] << 16) | (msg[4] << 8) | msg[5])^0x800000;
    diag(COMPONENT,DIAG_ERROR,"can't parse message - aborting the call "
      "(CR 0x%06lx)",call_ref);
    for (curr = sockets; curr; curr = curr->next)
        if (curr->sig == sig && curr->call_ref == call_ref) {
	    uni_call(curr,ATM_MSG_RESTART);
	    break;
	}
    send_release_complete(sig,call_ref,ATM_CV_PROTOCOL_ERROR);
}


void to_uni(SIG_ENTITY *sig,void *msg,int size)
{
    if (q_open(&in_dsc,msg,size) < 0) {
	abort_call(sig,msg,size);
	return;
    }
    process_uni(sig,msg);
    if (q_close(&in_dsc) < 0)
	diag(COMPONENT,DIAG_ERROR,"q_close returned <0 in to_uni");
}
