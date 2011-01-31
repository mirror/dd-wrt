/* control.c - User control command processing */

/* Written 1998 by Werner Almesberger, EPFL ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <atm.h>
#include <atmd.h>

#include "fab.h"
#include "dispatch.h"
#include "swc.h"


#define COMPONENT "COMMAND"


typedef struct _user_call {
    CALL call;
    struct _user_call *next;
} USER_CALL;

typedef struct {
    USER_CALL *u_call;
    UN_CTX un_ctx;
} CONTEXT;


static int s_control = -1;
static USER_CALL *calls = NULL;


static USER_CALL *find_call(const struct sockaddr_atmpvc *in,
  const struct sockaddr_atmpvc *out)
{
    USER_CALL *u_call;

    for (u_call = calls; u_call; u_call = u_call->next)
	if (atm_equal((struct sockaddr *) in,
	  (struct sockaddr *) &u_call->call.in.pvc,0,0) &&
	  atm_equal((struct sockaddr *) out,
	  (struct sockaddr *) &u_call->call.out.pvc,0,0)) break;
    return u_call;
}


static void add_cb(CALL *call,int cause,void *more,void *user)
{
    CONTEXT *context = user;
    SWC_MSG msg;
    USER_CALL **walk;

    memset(&msg,0,sizeof(msg));
    msg.type = smt_add;
    msg.n = cause ? -EIO : 0; /* @@@ */
    if (cause) free(context->u_call);
    else {
	for (walk = &calls; *walk && *walk; walk = &(*walk)->next);
	*walk = context->u_call;
    }
    if (un_send(&context->un_ctx,&msg,sizeof(msg)) < 0)
	diag(COMPONENT,DIAG_ERROR,"control_msg: un_send: %s",strerror(errno));
}


static void del_cb(CALL *call,int cause,void *more,void *user)
{
    CONTEXT *context = user;
    SWC_MSG msg;
    USER_CALL **walk;

    memset(&msg,0,sizeof(msg));
    msg.type = smt_del;
    msg.n = cause ? -EIO : 0; /* @@@ */
    if (!cause) {
	for (walk = &calls; *walk && *walk != context->u_call;
	  walk = &(*walk)->next);
	if (!*walk)
	    diag(COMPONENT,DIAG_FATAL,"del_cb: call %p not found",
	      context->u_call);
	*walk = (*walk)->next;
	free(context->u_call);
    }
    if (un_send(&context->un_ctx,&msg,sizeof(msg)) < 0)
	diag(COMPONENT,DIAG_ERROR,"control_msg: un_send: %s",strerror(errno));
}


static void control_msg(int sock,void *dummy)
{
    CONTEXT context;
    SWC_MSG msg;
    USER_CALL *u_call;
    int len,i;

    len = un_recv(&context.un_ctx,s_control,&msg,sizeof(msg));
    if (len < 0) {
	diag(COMPONENT,DIAG_ERROR,"control_msg: un_recv: %s",strerror(errno));
	return;
    }
    if (len != sizeof(SWC_MSG))
	diag(COMPONENT,DIAG_FATAL,"control_msg: bad length (%d != %d)",len,
	  sizeof(SWC_MSG));
    switch (msg.type) {
	case smt_get:
	    /*
	     * This code only shows VCs set up using the manual configuration
	     * interface. Any VCs set up by signaling are invisible. To fix
	     * this we'll need a "list fabric" function in the fabric-specific
	     * part. (The relay doesn't maintain a list of active connections,
	     * nor should it.)
	     */
	    i = msg.n;
	    for (u_call = calls; i && u_call; u_call = u_call->next) i--;
	    if (!u_call) {
		msg.n = -ENOENT;
		break;
	    }
	    msg.in = u_call->call.in.pvc;
	    msg.out = u_call->call.out.pvc;
	    msg.qos = u_call->call.out.qos;
	    break;
	case smt_add:
	    u_call = find_call(&msg.in,&msg.out);
	    if (u_call) {
		msg.n = -EEXIST;
		break;
	    }
	    u_call = alloc_t(USER_CALL);
	    memset(u_call,0,sizeof(USER_CALL));
	    u_call->call.in.pvc = msg.in;
	    u_call->call.out.pvc = msg.out;
	    fab_init(&u_call->call);
	    context.u_call = u_call;
	    fab_op(&u_call->call,RM_CLAIM(_RM_ANY) | RM_RSV(_RM_ANY),&msg.qos,
	      add_cb,&context);
	    return;
	case smt_del:
	    u_call = find_call(&msg.in,&msg.out);
	    if (!u_call) {
		msg.n = -ENOENT;
		break;
	    }
	    context.u_call = u_call;
	    fab_op(&u_call->call,RM_FREE,NULL,del_cb,&context);
	    return;
	default:
	    diag(COMPONENT,DIAG_FATAL,"control_msg: unknown message type %d",
	      msg.type);
    }
    if (un_send(&context.un_ctx,&msg,sizeof(msg)) < 0)
	diag(COMPONENT,DIAG_ERROR,"control_msg: un_send: %s",strerror(errno));
}


void control_init(const char *path)
{
    if (s_control != -1)
	diag(COMPONENT,DIAG_FATAL,"control channel is already set");
    s_control = un_create(path,0600);
    if (s_control < 0)
	diag(COMPONENT,DIAG_FATAL,"un_create: %s",strerror(errno));
    dsp_fd_add(s_control,control_msg,NULL);
}
