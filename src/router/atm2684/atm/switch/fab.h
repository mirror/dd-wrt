/* fab.h - Generic switch fabric interface */

/* Written 1997,1998 by Werner Almesberger, EPFL DI-ICA */


#ifndef _FAB_H
#define _FAB_H

#include "proto.h"


#define RM_FREE			0
#define RM_IN_TX		1
#define RM_IN_RX		2
#define RM_IN			(RM_IN_TX | RM_IN_RX)
#define RM_OUT_TX		4
#define RM_OUT_RX		8
#define RM_OUT			(RM_OUT_TX | RM_OUT_RX)
#define RM_PATH_TX		16
#define RM_PATH_RX		32
#define RM_PATH			(RM_PATH_TX | RM_PATH_RX)
#define _RM_ANY			(RM_IN | RM_OUT | RM_PATH)

#define _RM_SHIFT(what)		((what) << 6)
#define _RM_UNSHIFT(what)	((what) >> 6)
#define RM_RSV(what)		(what)
#define RM_CLAIM(what)		_RM_SHIFT(what)


/* --- Provided by fabric control ------------------------------------------ */

/*
 * fab_option passes an option name/value pair from the configuration file to
 * the fabric control. fab_option is invoked once for each "option" clause in
 * the configuration file. All invocations of fab_option occur before
 * fab_start.
 */

void fab_option(const char *name,const char *value);

/*
 * Initialize the fabric interface. The fabric control invokes port_notify
 * whenever a port is added to or removed from the switch. fab_start may
 * invoke port_notify before returning. port_notify(X,0) most not be invoked
 * until all fab_ops on that port have completed.
 */

void fab_start(void (*port_notify)(int number,int up));

/*
 * Initialize the fabric-specific part of a call structure, i.e. allocate a
 * fab-specific descriptor and attach it to call->fab. This function is called
 * before the first fab_op or fab_destroy.
 */

void fab_init(CALL *call);

/*
 * Destroy the fab-specific part of a call structure. This function is only
 * invoked once per call and only after any pending fab_op has completed.
 */

void fab_destroy(CALL *call);

/*
 * Allocate/change resources and set up paths in the switch fabric. fab_op may
 * be requested to operate on several parts of a call (i.e. the incoming side,
 * the outgoing side, or the path through the switch fabric) at the same time.
 * Internal scheduling is left to fab_op. Upon completion, fab_op invokes the 
 * callback function (once). fab_op may invoke the callback function before
 * returning. Only one fab_op may be in progress at a time for a call, but any
 * number of concurrent calls can be processed.
 */

void fab_op(CALL *call,int op,const struct atm_qos *qos,
  void (*callback)(CALL *call,int cause,void *more,void *user),void *user);

#endif
