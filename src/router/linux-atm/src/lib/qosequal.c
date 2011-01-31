/* qosequal.c - Compares QOS specifications for equality */

/* Written 1996,1999 by Werner Almesberger, EPFL-LRC/ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "atm.h"


static int tp_equal(int traffic_class,struct atm_trafprm a,struct atm_trafprm b)
{
    switch (traffic_class) {
	case ATM_NONE:
	    return 1;
	case ATM_UBR:
	    break;
	case ATM_CBR:
	    if (a.max_cdv != b.max_cdv) return 0;
	    break;
	default:
	    return -1;
    }
    if (!a.max_pcr && !a.min_pcr) a.max_pcr = ATM_MAX_PCR;
    if (!b.max_pcr && !b.min_pcr) b.max_pcr = ATM_MAX_PCR;
    if (a.max_pcr != b.max_pcr || a.pcr != b.pcr || a.min_pcr != b.min_pcr)
	return 0;
    return a.max_sdu == b.max_sdu;
}


int qos_equal(const struct atm_qos *a,const struct atm_qos *b)
{
    if (a->txtp.traffic_class != b->txtp.traffic_class) return 0;
    if (a->txtp.traffic_class == ATM_NONE) {
	if (a->rxtp.traffic_class != b->rxtp.traffic_class) return 0;
	return tp_equal(a->rxtp.traffic_class,a->rxtp,b->rxtp);
    }
    else {
	if (!tp_equal(a->txtp.traffic_class,a->txtp,b->txtp)) return 0;
	return tp_equal(a->txtp.traffic_class,a->rxtp,b->rxtp);
    }
}
