/* proto.h - Common protocol functions and structures */
 
/* Written 1997-1998 by Roman Pletka, EPFL SSC */
/* Modified 1998,2000 by Werner Almesberger, EPFL ICA */

#ifndef PROTO_H
#define PROTO_H

#define CALLER 0                /* We add this to the call pointer. It     */
#define CALLED 1                /* helps us to find out the source of the  */
#define RM     2                /* message.                                */

#include <linux/atmsvc.h>

#include "atmsap.h"
#include "atmd.h"

#include "sig.h"


typedef enum { /* call states */
	cs_invalid,         cs_null,            cs_listening,
	cs_connected,       cs_indicated,	cs_called_accepted,
	cs_rm_accepted,     cs_rm_accepted2,    cs_caller_error,
	cs_rejected,        cs_rejected2,       cs_caller_closing,
	cs_called_closed,   cs_called_closed2,  cs_free_rm,
        cs_rejecting,       cs_will_close,      cs_call_indicated,
	cs_caller_closed
    
} STATE;

typedef struct _party {
    SIGNALING_ENTITY *sig;		/* signaling entity */
    struct sockaddr_atmpvc pvc;		/* itf and CI */
    struct sockaddr_atmsvc svc;		/* remote address */
    struct atm_qos qos;			/* QOS parameters */
} PARTY;

typedef struct _call {
    STATE state;
    PARTY in;				/* caller data */
    PARTY out;				/* called data */

    struct atm_sap sap;			/* SAP (BHLI and BLLI) */

    int error;				/* error code for close */
    /* --- switch fabric control data -------------------------------------- */
    void *fab;
} CALL;

/*
 * Note that the fabric may only look at call.in.pvc, call.in.pvc,
 * call.out.pvc, call.out.qos, and call.fab. All other fields may be set to
 * arbitrary values by the signaling relay.
 */


void send_identify(CALL *call);
void send_listen(SIGNALING_ENTITY *sig);
void send_connect(CALL *call);
void send_reject(CALL *call, int err_code);
void send_reject_not_id(SIGNALING_ENTITY *sig, int err_code);
void send_close(CALL *call,int dest);
void send_accept(CALL *call);

CALL *new_call(void);
void free_call(CALL *call);
void new_state(CALL *call,STATE state);

CALL *demux_in(unsigned long *srce, struct atmsvc_msg *msg);

/* some debugging functions */
void print_msg(struct atmsvc_msg *msg, CALL *call,unsigned long source);
void print_state(CALL *call);
void print_call(CALL *call);

int from_sigd(SIGNALING_ENTITY *sig,struct atmsvc_msg *msg);

#endif
