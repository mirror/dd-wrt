/* debug.c - Simple debugging "switch" */

/* Written 1998-2000 by Werner Almesberger, EPFL DI-ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>

#include <atm.h>
#include <atmd.h>

#include "uni.h"
#include "../fab.h"


#define COMPONENT "FAB(debug)"


#define PRV(call) ((FAB *) (call)->fab)

int yywrap(void)
{
        return 1;
}


typedef struct _fab {
    CALL *next; /* relay.c may not keep track of calls, but WE are */
} FAB;


static CALL *calls;


void fab_option(const char *name,const char *value)
{
    diag(COMPONENT,DIAG_FATAL,"unrecognized fabric option \"%s\"",name);
}


void fab_start(void (*port_notify)(int number,int up))
{
    sig_start_all(port_notify);
}


void fab_init(CALL *call)
{
    FAB *fab;

    fab = alloc_t(FAB);
    call->fab = fab;
    fab->next = calls;
    calls = call;
}


void fab_destroy(CALL *call)
{
    CALL **walk;

    for (walk = &calls; *walk; walk = &PRV(*walk)->next)
	if (*walk == call) break;
    if (!*walk)
	diag(COMPONENT,DIAG_FATAL,"fab_destroy: call %p not found",call);
    *walk = PRV(call)->next;
    free(PRV(call));
    call->fab = NULL;
}


/*
 * This function is rather simple-minded, because it only considers a single
 * port. Should go directly to the fabric control. @@@
 */


static int vci_exists(int vci,int threshold)
{
    CALL *call;
    int found;

    found = 0;
    for (call = calls; call; call = PRV(call)->next)
	if (call->in.pvc.sap_addr.vci == vci ||
	  call->out.pvc.sap_addr.vci == vci)
	    if (++found > threshold) return 1;
    return 0;
}


static int check_ci(struct sockaddr_atmpvc *pvc)
{
    int vci;

    if (pvc->sap_addr.vpi == ATM_VPI_ANY) pvc->sap_addr.vpi = 0;
	/* that was easy :-) */
    for (vci = ATM_NOT_RSV_VCI; pvc->sap_addr.vci == ATM_VCI_ANY; vci++)
	if (!vci_exists(vci,0)) pvc->sap_addr.vci = vci;
    return !vci_exists(vci,1);
}


void fab_op(CALL *call,int op,const struct atm_qos *qos,
  void (*callback)(CALL *call,int cause,void *more,void *user),void *user)
{
    diag(COMPONENT,DIAG_INFO,"fab_op%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
      !op ? " FREE" : "",op & RM_RSV(_RM_ANY) ? " RM_RSV:" : "",
      op & RM_IN_TX ? " IN_TX" : "",op & RM_IN_RX ? " IN_RX" : "",
      op & RM_OUT_TX ? " OUT_TX" : "",op & RM_OUT_RX ? " OUT_RX" : "",
      op & RM_PATH_TX ? " PATH_TX" : "",op & RM_PATH_RX ? " PATH_RX" : "",
      op & RM_CLAIM(_RM_ANY) ? " RM_CLAIM:" : "",
      op & _RM_SHIFT(RM_IN_TX) ? " IN_TX" : "",
      op & _RM_SHIFT(RM_IN_RX) ? " IN_RX" : "",
      op & _RM_SHIFT(RM_OUT_TX) ? " OUT_TX" : "",
      op & _RM_SHIFT(RM_OUT_RX) ? " OUT_RX" : "",
      op & _RM_SHIFT(RM_PATH_TX) ? " PATH_TX" : "",
      op & _RM_SHIFT(RM_PATH_RX) ? " PATH_RX" : "");
    if (op & (RM_RSV(RM_IN) | RM_CLAIM(RM_IN)))
	if (!check_ci(&call->in.pvc)) {
	    callback(call,ATM_CV_CI_UNAVAIL,NULL,user);
	    return;
	}
    if (op & (RM_RSV(RM_OUT) | RM_CLAIM(RM_OUT)))
	if (!check_ci(&call->out.pvc)) {
	    callback(call,ATM_CV_CI_UNAVAIL,NULL,user);
	    return;
	}
    callback(call,0,NULL,user);
}
