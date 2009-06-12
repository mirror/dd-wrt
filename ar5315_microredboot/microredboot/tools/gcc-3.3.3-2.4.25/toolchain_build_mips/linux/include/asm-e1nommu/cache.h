#ifndef _HYPERSTONE_CACHE_H
#define _HYPERSTONE_CACHE_H

/* bytes per L1 cache line */
#define        L1_CACHE_BYTES			16 
#define        L1_CACHE_ALIGN(x)       (((x)+(L1_CACHE_BYTES-1))&~(L1_CACHE_BYTES-1))
#define        SMP_CACHE_BYTES          L1_CACHE_BYTES

#endif
