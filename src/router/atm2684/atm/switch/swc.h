/* swc.h - Switch control interface */

/* Written 1998 by Werner Almesberger, EPFL ICA */


#ifndef SWC_H
#define SWC_H

#include <atm.h>


typedef enum {
    smt_invalid,	/* catch uninitialized variables */
    smt_get,		/* get/return n-th entry */
    smt_add,		/* add one-way VC */
    smt_del		/* remove one-way VC */
} SWC_MSG_TYPE;

typedef struct swc_msg {
    SWC_MSG_TYPE type;			/* message type */
    int n;				/* index (for tmt_get) and error code
					   (for tmt_get, tmt_set, tmt_del) */
    struct sockaddr_atmpvc in;
    struct sockaddr_atmpvc out;
    struct atm_qos qos;			/* currently unused */
} SWC_MSG;


extern void control_init(const char *path);

#endif
