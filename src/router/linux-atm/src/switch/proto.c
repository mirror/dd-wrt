/* proto.c - Common protocol functions and structures */
 
/* Written 1997-1998 by Roman Pletka, EPFL-SSC */
/* Modified 1998,2000 by Werner Almesberger, EPFL ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <atm.h>

#include "atmd.h"
#include "sig.h"
#include "fab.h"
#include "proto.h"


#define COMPONENT "SWITCH"

static const char *as_msgs[] = {
		      "as_catch_null", "as_bind", "as_connect",
		      "as_accept", "as_reject","as_listen",
		      "as_okay", "as_error", "as_indicate",
		      "as_close", "as_itf_notify", "as_modify",
		      "as_identify"};
static const char *cs_states[]= {
		       "cs_invalid", "cs_null", "cs_listening",
		       "cs_connected", "cs_indicated", "cs_called_accepted",
		       "cs_rm_accepted", "cs_rm_accepted2", "cs_caller_error",
		       "cs_rejected", "cs_rejected2", "cs_caller_closing",
		       "cs_called_closed", "cs_called_closed2", "cs_free_rm",
		       "cs_rejecting", "cs_will_close", "cs_call_indicated",
		       "cs_caller_closed" };
static const char *sources[4] = {"CALLER","CALLED","RM"};


CALL *new_call(void)
{
    CALL *call;
 
    call = alloc_t(CALL);
    memset(call,0,sizeof(CALL));
    call->state = cs_invalid;
    fab_init(call);
    return call;
}


void free_call(CALL *call)
{
    fab_destroy(call);
    call->state = cs_invalid;
    free(call);
    printf("Call 0x%p killed\n",call);
}


void new_state(CALL *call,STATE state)
{
    call->state = state;
    print_state(call);
}


void send_listen(SIGNALING_ENTITY *sig)
{
    struct atmsvc_msg msg;

    memset(&msg,0,sizeof(msg));
    /* compose the message */
    msg.type = as_listen;
    *(unsigned long *) &msg.vcc = (unsigned long) sig;
    msg.svc.sas_family = AF_ATMSVC;
    msg.qos.aal = ATM_AAL5;
    msg.qos.txtp.traffic_class = msg.qos.rxtp.traffic_class = ATM_ANYCLASS;
    /* msg.sap ; */
    sig_send(sig,&msg);
}


void send_identify(CALL *call)
{
    struct atmsvc_msg msg;

    /*  this is always sent to caller */
    memset(&msg,0,sizeof(msg));
  
    /* compose the message */
    msg.type = as_identify;
    *(unsigned long *) &msg.vcc = (unsigned long) call | CALLER;
    *(unsigned long *) &msg.listen_vcc = (unsigned long) call->in.sig;
  
    /* We have to complete the message (vci,vpi..) */
    msg.pvc = call->in.pvc;
    sig_send(call->in.sig,&msg);
}


void send_connect(CALL *call)
{
    struct atmsvc_msg msg;

    /*  this is always sent to called */
    memset(&msg,0,sizeof(msg));
    /* compose the message */
    msg.type = as_connect;
    *(unsigned long *) &msg.vcc = (unsigned long) call | CALLED;
      /* some kind of magic... */
    msg.local = call->in.svc;
    msg.qos.aal = call->in.qos.aal; /* or should we rather use out.qos ? @@@ */
    msg.qos.txtp = call->in.qos.rxtp;
    msg.qos.rxtp = call->in.qos.txtp;
    msg.svc = call->out.svc;
    msg.sap = call->sap;
    /* we have to give VCI/VPI */
    msg.pvc = call->out.pvc;
    sig_send(call->out.sig,&msg);
}


void send_reject(CALL *call, int err_code)
{
    struct atmsvc_msg msg;

    /*  this is always sent to caller */
    memset(&msg,0,sizeof(msg));
    msg.type = as_reject;
    *(unsigned long *) &msg.vcc = (unsigned long) call | CALLER; 
    msg.reply = err_code;
    sig_send(call->in.sig,&msg);
}


void send_reject_not_id(SIGNALING_ENTITY *sig, int err_code)
{
    struct atmsvc_msg msg;

    /*  this is always sent to caller */
    memset(&msg,0,sizeof(msg));
    msg.type = as_reject;
    *(unsigned long *) &msg.listen_vcc = (unsigned long) sig; 
    msg.reply = err_code;
    sig_send(sig,&msg);
}


void send_close(CALL *call,int dest)
{
    struct atmsvc_msg msg;

    memset(&msg,0,sizeof(msg));
    msg.type = as_close;
    *(unsigned long *) &msg.vcc = (unsigned long) call | dest;
      /* dest: CALLER or CALLED */
    /* msg.reply = ??!! */  
    sig_send(dest == CALLER ? call->in.sig : call->out.sig,&msg);
}


void send_accept(CALL *call)
{
    struct atmsvc_msg msg;

    memset(&msg,0,sizeof(msg));
    msg.type = as_accept;
    *(unsigned long *) &msg.vcc = (unsigned long) call | CALLER;
    sig_send(call->in.sig,&msg);
}


/*****************************************************************************/
/* Demultiplexing with magic number: caller - called - rm                    */
/*****************************************************************************/

CALL *demux_in(unsigned long *srce, struct atmsvc_msg *msg)
{
  /* The multiplexing informations are in the 3 least significant bits
     of the call pointer. We can do this, because the compiler aligns
     memory reservation to pointers with  3 ls-bits = 0. 
     */
    *srce = *(unsigned long *) &msg->vcc & 3;
    return (CALL *) (*(unsigned long *) &msg->vcc & ~3);
}

/*****************************************************************************/
/* Debugging functions                                                       */
/*****************************************************************************/
void print_msg(struct atmsvc_msg *msg, CALL *call,unsigned long source) {
  
  printf("Msg '%s' received from %s vcc=%s for call 0x%p, listen: %s\n",
	 as_msgs[msg->type], sources[source], kptr_print(&msg->vcc), call,
	 kptr_print(&msg->listen_vcc));
}
void print_state(CALL *call) {
  printf("    Call 0x%p entered state '%s'\n", 
	 call , cs_states[call->state]);
}
void print_call(CALL *call) {
  printf("    Call 0x%p in state %s, caller-id:%p, called-id:%p\n",
	 call, cs_states[call->state], 
	 call->in.sig, call->out.sig);
}
