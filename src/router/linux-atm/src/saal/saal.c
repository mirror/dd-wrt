/* saal.c - SAAL = SSCF+SSCOP */

/* Written 1995 by Werner Almesberger, EPFL-LRC */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "sscop.h"
#include "saal.h"


void saal_pdu(SAAL_DSC *dsc,void *buffer,int length)
{
    sscop_pdu(&dsc->sscop,buffer,length);
}
