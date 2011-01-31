/* timeout.c - Processing of signaling timeout events */
 
/* Written 1995-2000 by Werner Almesberger, EPFL-LRC/ICA */
 
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <assert.h>

#include <atm.h>
#include <linux/atmdev.h>

#include "atmd.h"

#include "uni.h"
#include "proto.h"
#include "timeout.h"


#define COMPONENT "UNI"


static void complain(SOCKET *sock,const char *timer)
{
    diag(COMPONENT,DIAG_FATAL,"Timer %s expired in incompatible state %s",
      timer,state_name[sock->state]);
}

#ifdef MULTIPOINT
void on_T398(void *user)	/* DROPPARTY */
{
	SOCKET *sock = user;

	if (sock == NULL) return;
	SEND_ERROR(sock->id, -ETIMEDOUT);
	/* should retry once and then give up */
	sock->conn_timer = NULL;
	free_sock(sock);
}

void on_T399(void *user)	/* ADDPARTY */
{
    SOCKET *sock = user;

    if (sock == NULL) return;
    diag(COMPONENT,DIAG_INFO,"Timer T399 expired in state %s",
         state_name[sock->state]);
    SEND_ERROR(sock->id, -ETIMEDOUT);
    sock->conn_timer = NULL;
    free_sock(sock);
}
#endif

void on_T303(void *user) /* CONNECTING */
{
    SOCKET *sock = user;

    diag(COMPONENT,DIAG_DEBUG,"T303 on %s",kptr_print(&sock->id));
    if (sock->state != ss_connecting) complain(sock,"T303");
    SEND_ERROR(sock->id,-ETIMEDOUT);
    sock->conn_timer = NULL;
    free_sock(sock);
}


void on_T308_1(void *user) /* WAIT_REL or REL_REQ */
{
    SOCKET *sock = user;

    diag(COMPONENT,DIAG_DEBUG,"T308_1 on %s",kptr_print(&sock->id));
    if (sock->state != ss_wait_rel && sock->state != ss_rel_req)
	complain(sock,"T308_1");
    send_release(sock,ATM_CV_TIMER_EXP,308); /* @@@ ? */
    sock->conn_timer = NULL;
    START_TIMER(sock,T308_2);
}


void on_T308_2(void *user) /* WAIT_REL or REL_REQ */
{
    SOCKET *sock = user;

    diag(COMPONENT,DIAG_WARN,"Trouble: T308_2 has expired");
    if (sock->state != ss_wait_rel && sock->state != ss_rel_req)
	complain(sock,"T308_2");
    sock->conn_timer = NULL;
    if (sock->state == ss_rel_req) send_close(sock);
    free_sock(sock);
}


void on_T309(void *user)
{
    diag(COMPONENT,DIAG_DEBUG,"T309 has expired");
    clear_all_calls_on_T309(user);
}


void on_T310(void *user)
{
    on_T303(user);
    diag(COMPONENT,DIAG_DEBUG,"(it's actually T310)");
}


void on_T313(void *user) /* ACCEPTING */
{
    SOCKET *sock = user;

    diag(COMPONENT,DIAG_DEBUG,"T313 on %s",kptr_print(&sock->id));
    if (sock->state != ss_accepting) complain(sock,"T313");
    send_release(sock,ATM_CV_TIMER_EXP,313);
    sock->conn_timer = NULL;
    START_TIMER(sock,T308_1);
    new_state(sock,ss_rel_req);
}


#if defined(Q2963_1) || defined(DYNAMIC_UNI)

void on_T360(void *user)
{
    SOCKET *sock = user;

    diag(COMPONENT,DIAG_DEBUG,"T360 on %s",kptr_print(&sock->id));
    if (sock->state != ss_mod_req) complain(sock,"T360");
    send_release(sock,ATM_CV_TIMER_EXP,360);
    sock->conn_timer = NULL;
    START_TIMER(sock,T308_1);
    new_state(sock,ss_rel_req);
}


void on_T361(void *user)
{
    SOCKET *sock = user;

    diag(COMPONENT,DIAG_DEBUG,"T361 on %s",kptr_print(&sock->id));
    if (sock->state != ss_connected) complain(sock,"T361");
    sock->qos = sock->new_qos;
    send_kernel(sock->id,kptr_null,as_modify,ATM_MF_SET,NULL,NULL,NULL,NULL,
      &sock->qos);
    sock->conn_timer = NULL;
    new_state(sock,ss_mod_fin_ack);
}

#endif
