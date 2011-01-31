/* switch.c - Handles signaling an ATM switch */

/* Written 1997-1998 by Roman Pletka, EPFL SSC */
/* Modified 1998-2000 by Werner Almesberger, EPFL ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <atm.h>

#include "atmd.h"
#include "fab.h"
#include "dispatch.h"
#include "sig.h"
#include "route.h"
#include "proto.h"


#define COMPONENT "RELAY"
#define CONFIG_FILE "switch.conf"


extern int yyparse(void);
extern FILE *yyin;


static void from_fab(CALL *call,int cause,void *more,void *user)
{
    printf("%p: fab returns cause %d\n",call,cause);
    print_call(call);

    switch (call->state) {
	case cs_indicated:
	    if (!cause) {
		/* send connect to called and enter state cs_rm_accepted */
		send_identify(call);
		send_connect(call);
		new_state(call,cs_rm_accepted);
	    }
	    else {
		/* send as_reject to caller and enter state cs_invalid */
		send_reject_not_id(call->in.sig,-EREMOTEIO); /* @@@  use cause*/
		new_state(call,cs_invalid);
		free_call(call);
	    }
	    break;
	case cs_called_accepted:
	    if (!cause) {
		/* send accept to caller */
		send_accept(call);
		new_state(call,cs_rm_accepted2);
	    }
	    else {
		/* send reject to caller and close to called */
		send_reject(call,-EREMOTEIO); /* @@@ use cause */
		send_close(call,CALLED);
		new_state(call,cs_rejected);
	    }
	    break;
	case cs_rejecting:
	    if (cause) free_call(call);
	    else {
		/* free resources */
		new_state(call, cs_free_rm);
		fab_op(call,RM_FREE,NULL,from_fab,NULL);
	    }
	    break;
	case cs_free_rm:
	    if (cause) printf("Error: RM couldn't free resources\n");
	    free_call(call);
	    break;
	default:
	    diag(COMPONENT,DIAG_FATAL,"invalid state for fab callback");
	    break;
    }
}


static void from_caller(CALL *call,struct atmsvc_msg *msg)
{
    switch (call->state) {
	case cs_rm_accepted2:
	    /* msg from: caller(error,okay) or called(close) */
	    switch(msg->type) {
		case as_okay: /* complete the call */
		    new_state(call, cs_connected);
		    return;
		case as_error:
		    /* send close called */
		    send_close(call, CALLED);
		    new_state(call, cs_caller_error);
		    return;
		default:
		    break;
	    }
	    break;
	case cs_connected:
	    if (msg->type != as_close) break;
	    send_close(call, CALLER);
	    send_close(call, CALLED);
	    new_state(call, cs_caller_closed);
	    return;
	case cs_called_closed:
	    switch(msg->type) {
		case as_error: 
		    new_state(call, cs_free_rm);
		    fab_op(call,RM_FREE,NULL,from_fab,NULL);
		    break;
		case as_okay:
		    send_close(call, CALLER);
		    new_state(call, cs_caller_closing);
		    break;
		default:
		    break;
	    }
	    break;
	case cs_called_closed2:
	case cs_caller_closing:
	    if (msg->type != as_close) break;
	    new_state(call, cs_free_rm);
	    fab_op(call,RM_FREE,NULL,from_fab,NULL);
	    return;
	default:
	    break;
    }
    diag(COMPONENT,DIAG_FATAL,"invalid combination");
}


static void from_called(CALL *call,struct atmsvc_msg *msg)
{
    switch (call->state) {
	case cs_rm_accepted:
	    switch(msg->type) {
		case as_okay: 
		    /* save msg content in call */
		    call->out.qos = msg->qos;
		    new_state(call,cs_called_accepted);
		    fab_op(call,RM_CLAIM(_RM_ANY),&msg->qos,from_fab,NULL);
		    return;
		case as_error:
		    send_reject(call,msg->reply);
		    new_state(call,cs_invalid);
		    free_call(call);
		    return;
		default:
		    break;
	    }
	    break;
	case cs_rm_accepted2:
	    /* msg from: caller(error,okay) or called(close) */
	    if (msg->type != as_close) break;
	    /* send close to called */
	    send_close(call, CALLED);
	    new_state(call, cs_called_closed);
	    return;
	case cs_rejected:
	    /* wait for close msg from called */
	    if (msg->type != as_close) break;
	    free_call(call);
	    return;
	case cs_called_accepted:
	    if (msg->type != as_close) break;
	    /* send reject to caller and send close to called */
	    send_reject(call, msg->reply);
	    send_close(call,CALLED);
	    new_state(call,cs_rejecting);
	    return;
	case cs_connected:
	    if (msg->type != as_close) break;
	    send_close(call, CALLER);
	    send_close(call, CALLED);
	    new_state(call, cs_called_closed2);
	    return;
	case cs_caller_error:
	case cs_caller_closed:
	    if (msg->type != as_close) break;
	    new_state(call, cs_free_rm);
	    fab_op(call,RM_FREE,NULL,from_fab,NULL);
	    return;
	default:
	    break;
    }
    diag(COMPONENT,DIAG_FATAL,"invalid combination");
}


static void from_listening(SIGNALING_ENTITY *sig,struct atmsvc_msg *msg)
{
    SIGNALING_ENTITY *out;
    CALL *call;

    /* try to find a route */
    out = find_route(&msg->svc,&msg->local,&msg->qos);
    if (!out) {
	send_reject_not_id(sig,-EHOSTUNREACH);
	return;
    }
    /* now work starts... */
    call = new_call();
    /* set up caller side */
    call->in.sig = sig;
    if (atmpvc_addr_in_use(msg->pvc)) call->in.pvc = msg->pvc;
    else {
	call->in.pvc.sap_addr.itf = sig->itf;
	call->in.pvc.sap_addr.vpi = ATM_VPI_ANY;
	call->in.pvc.sap_addr.vci = ATM_VCI_ANY;
    }
    call->in.pvc.sap_family = AF_ATMPVC;
    call->in.svc = msg->svc;
    call->in.qos = msg->qos;
    /* set up what little we know about the called side */
    call->out.sig = out;
    call->out.pvc.sap_family = AF_ATMPVC;
    call->out.pvc.sap_addr.itf = out->itf;
    call->out.pvc.sap_addr.vpi = ATM_VPI_ANY;
    call->out.pvc.sap_addr.vci = ATM_VCI_ANY;
    call->out.svc = msg->local;
    call->sap = msg->sap;
    new_state(call,cs_indicated);
    fab_op(call,RM_RSV(_RM_ANY),&msg->qos,from_fab,NULL);
    /*
     * This is bogus. txtp and rxtp are exchanged on the input and the output
     * side. This can be fixed by defining txtp/rxtp as meaning "forward" and
     * "backward", respectively, in the switch. I guess that's what I'll do.
     */
}


int from_sigd(SIGNALING_ENTITY *sig,struct atmsvc_msg *msg)
{
    if (msg->type == as_indicate) from_listening(sig,msg);
    else {
	CALL *call;
	unsigned long source;

	call = demux_in(&source,msg);
	print_msg(msg,call,source);
	print_call(call);
	switch (source) {
	    case CALLER:
		from_caller(call,msg);
		break;
	    case CALLED:
		from_called(call,msg);
		break;
	    default:
		diag(COMPONENT,DIAG_FATAL,"unrecognized source %d\n",source);
	}
    }
    return 0;
}


/*****************************************************************************/
/*   M A I N                                                                 */
/*****************************************************************************/


static void usage(const char *name)
{
    fprintf(stderr,"usage: %s [ -b ] [ -c config_file ] [ -d ]\n",name);
    exit(1);
}


int main(int argc, char *argv[])
{
    const char *config_file;
    int background;
    int c;

    background = 0;
    config_file = CONFIG_FILE;
    while ((c = getopt(argc,argv,"bc:d")) != EOF)
	switch (c) {
	    case 'b':
		background = 1;
		break;
	    case 'c':
		config_file = optarg;
		break;
	    case 'd':
		set_verbosity(NULL,DIAG_DEBUG);
		break;
	    default:
		usage(argv[0]);
	}
    if (argc != optind) usage(argv[0]);
    dsp_init(); /* initialize dispatcher */
    /*
     * Later: call fab_something to scan all ports and launch atmsigds.
     * For now, everything is handled by static configuration.
     */
    if (!(yyin = fopen(config_file,"r")))
	diag(COMPONENT,DIAG_FATAL,"%s: %s",config_file,strerror(errno));
    if (yyparse())
	diag(COMPONENT,DIAG_FATAL,"Error in config file. - Aborting.");
    fab_start(sig_notify);
    if (background) {
	pid_t pid;

	pid = fork();
	if (pid < 0)
	    diag(COMPONENT,DIAG_FATAL,"fork: %s",strerror(errno));
	if (pid) {
	    diag(COMPONENT,DIAG_DEBUG,"Backgrounding (PID %d)",pid);
	    exit(0);
	}
    }
    while (1) dsp_poll();
}
