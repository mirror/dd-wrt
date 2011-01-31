/* saal.h - SAAL user interface */

/* Written 1995 by Werner Almesberger, EPFL-LRC */


#ifndef SAAL_H
#define SAAL_H

#include "sscf.h"
#include "sscop.h"


#define SAAL_DSC SSCF_DSC
#define SAAL_USER_OPS SSCF_USER_OPS
#define start_saal start_sscf
#define stop_saal stop_sscf
#define saal_estab_req sscf_estab_req
#define saal_rel_req sscf_rel_req
#define saal_send sscf_send
#define saal_unitdata sscf_unitdata

void saal_pdu(SAAL_DSC *dsc,void *buffer,int length);

#endif
