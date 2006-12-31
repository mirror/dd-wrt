/* sdu2cr.c - Converts SDU sizes and SDU counts to cell counts */

/* Written 1996-1998 by Werner Almesberger, EPFL-LRC/ICA */


#include <limits.h>

#include "atm.h"


int sdu2cell(int s,int sizes,const int *sdu_size,int *num_sdu)
{
    struct atm_qos qos;
    int trailer,total,cells;
    int size,i;

    size = sizeof(qos);
    if (getsockopt(s,SOL_AAL,SO_ATMQOS,&qos,&size) < 0) return -1;
    switch (qos.aal) {
	case ATM_AAL0:
	    trailer = 0;
	    break;
	case ATM_AAL5:
	    trailer = ATM_AAL5_TRAILER;
	    break;
	default:
	    return -1;
    }
    total = 0;
    for (i = 0; i < sizes; i++) {
	cells = (trailer+*sdu_size+ATM_CELL_PAYLOAD-1)/ATM_CELL_PAYLOAD;
	if (INT_MAX/cells < *num_sdu) return -1;
	cells *= *num_sdu;
	if (INT_MAX-cells < total) return -1;
	total += cells;
	sdu_size++;
	num_sdu++;
    }
    return total;
}
