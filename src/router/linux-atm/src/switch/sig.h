/* sig.h - signaling entity handling */

/* Written 1998-2000 by Werner Almesberger, EPFL ICA */


#ifndef SIG_H
#define SIG_H

#include <atm.h>
#include <linux/atmsvc.h>


typedef struct _signaling_entity {
    int s;			/* socket */
    const char *command;	/* command to start sigd; NULL if none */
    const char *path;		/* path to the Unix domain socket */
    struct sockaddr_atmpvc pvc;	/* signaling VC; itf = -1 if not used */
    short itf;			/* interface we manage */
    struct _signaling_entity *next;
    struct _call *call;		/* used to route VCI 5 to signaling */
} SIGNALING_ENTITY;


SIGNALING_ENTITY *sig_vc(const char *command,const char *path,int itf);
void sig_send(SIGNALING_ENTITY *sig,struct atmsvc_msg *msg);
void sig_notify(int itf,int up);

/*
 * sig_start can be called by fab_start if fab_start has no knowledge of
 * ports, e.g. if ports are virtual and pre-configured.
 */

void sig_start_all(void (*port_notify)(int number,int up));

#endif
