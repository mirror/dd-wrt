/* kptr.c - Helper functions to use kernel pointer handles */

/* Written 2000 by Werner Almesberger, EPFL ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <assert.h>
#include <atm.h>

#include "atmd.h"


int kptr_eq(const atm_kptr_t *a,const atm_kptr_t *b)
{
    unsigned long *_a = (unsigned long *) a,*_b = (unsigned long *) b;

    assert(sizeof(atm_kptr_t) == 8);
    switch (sizeof(unsigned long)) {
	case 2: /* Wow, ATM on ELKS ? :-) */
	    if (_a[2] != _b[2]) return 0;
	    if (_a[3] != _b[3]) return 0;
	case 4:
	    if (_a[1] != _b[1]) return 0;
	case 8:
	    return *_a == *_b;
	default:
	    abort();
    }
}


const char *kptr_print(const atm_kptr_t *p)
{
    static char buf[KPRT_PRINT_BUFS][sizeof(atm_kptr_t)*2+1];
    static int curr_buf = 0;
    char *result;
    int i;

    result = buf[curr_buf];
    curr_buf = (curr_buf+1) & (KPRT_PRINT_BUFS-1);
    for (i = 0; i < sizeof(atm_kptr_t); i++)
	sprintf(result+2*i,"%02x",((unsigned char *) p)[i]);
    return result;
}
