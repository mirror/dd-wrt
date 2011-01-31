/* sig.c - signaling entity handling */

/* Written 1998 by Werner Almesberger, EPFL ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <linux/atmsvc.h>

#include "atmd.h"

#include "dispatch.h"
#include "proto.h"
#include "sig.h"
#include "route.h"
#include "fab.h"


#define COMPONENT "SIG"


static SIGNALING_ENTITY *entities = NULL;


static int sig_check_listen(SIGNALING_ENTITY *sig,struct atmsvc_msg *msg)
{
    return msg->type == as_okay ? 0 : msg->reply;
}


static int sig_recv(SIGNALING_ENTITY *sig,
  int (*handler)(SIGNALING_ENTITY *sig,struct atmsvc_msg *msg))
{
    char buf[sizeof(struct atmsvc_msg)+1];
    int len;

    len = read(sig->s,buf,sizeof(buf));
    if (len == sizeof(struct atmsvc_msg))
	return handler(sig,(struct atmsvc_msg *) buf);
    if (len < 0) 
	diag(COMPONENT,DIAG_ERROR,"read isp msg: %s",strerror(errno));
    else diag(COMPONENT,DIAG_ERROR,"bad isp msg: %d != %d",len,
	  sizeof(struct atmsvc_msg));
    return -1;
}


static void sig_data(int fd,void *sig)
{
    (void) sig_recv(sig,from_sigd);
}


SIGNALING_ENTITY *sig_vc(const char *command,const char *path,int itf)
{
    SIGNALING_ENTITY *sig;

    sig = alloc_t(SIGNALING_ENTITY);
    sig->command = command;
    sig->path = path;
    sig->pvc.sap_addr.itf = -1;
    sig->itf = itf;
    sig->next = entities;
    entities = sig;
    return sig;
}


void sig_send(SIGNALING_ENTITY *sig,struct atmsvc_msg *msg)
{
    int len;

    len = write(sig->s,msg,sizeof(*msg));
    if (len == sizeof(*msg)) return;
    if (len < 0) diag(COMPONENT,DIAG_ERROR,"write isp msg: %s",strerror(errno));
    else diag(COMPONENT,DIAG_ERROR,"bad isp msg write: %d != %d",len,
	  sizeof(*msg));
}


static void up_callback(CALL *call,int cause,void *more,void *user)
{
    SIGNALING_ENTITY *sig = user;
    int error;

    if (cause) {
	diag(COMPONENT,DIAG_ERROR,"up_callback: error (cause %d)",cause);
	return;
    }
    if (sig->command) system(sig->command);
    sig->s = un_attach(sig->path);
    if (sig->s < 0)
	diag(COMPONENT,DIAG_FATAL,"un_attach %s: %s",sig->path,strerror(errno));
    send_listen(sig);
    error = sig_recv(sig,sig_check_listen);
    if (error) diag(COMPONENT,DIAG_FATAL,"listen failed: %s",strerror(error));
    dsp_fd_add(sig->s,sig_data,sig);
    route_sig(sig,&sig->call->out.pvc,1);
}


static void remove_entity(SIGNALING_ENTITY *sig)
{
    struct atmsvc_msg msg;

    msg.type = as_terminate;
    sig_send(sig,&msg);
    dsp_fd_remove(sig->s);
    (void) close(sig->s);
}


static void down_callback(CALL *call,int cause,void *more,void *user)
{
    if (cause)
	diag(COMPONENT,DIAG_ERROR,"down_callback: error (cause %d)",cause);
}


void sig_notify(int itf,int up)
{
    SIGNALING_ENTITY *sig;

    for (sig = entities; sig; sig = sig->next)
	if (sig->itf == itf) break;
    if (!sig) {
	diag(COMPONENT,DIAG_ERROR,"%s notification for unknown interface %d",
	  up ? "up" : "down",itf);
	return;
    }
    if (sig->pvc.sap_addr.itf == -1) return;
    if (up) {
	struct atm_qos qos;

	sig->call = new_call();
	sig->call->in.pvc = sig->pvc;
	sig->call->out.pvc.sap_addr.itf = sig->itf;
	sig->call->out.pvc.sap_addr.vpi = 0;
	sig->call->out.pvc.sap_addr.vci = 5;
	memset(&qos,0,sizeof(qos));
	qos.txtp.traffic_class = qos.rxtp.traffic_class = ATM_UBR;
	fab_op(sig->call,RM_CLAIM(_RM_ANY),&qos,up_callback,sig);
    }
    else {
	route_sig(sig,&sig->call->out.pvc,0);
	remove_entity(sig);
	fab_op(sig->call,RM_FREE,NULL,down_callback,NULL);
	free_call(sig->call);
    }
}


void sig_start_all(void (*port_notify)(int number,int up))
{
    SIGNALING_ENTITY *sig;

    for (sig = entities; sig; sig = sig->next) {
	sig->call = NULL;
	up_callback(NULL,0,NULL,sig);
    }
}
