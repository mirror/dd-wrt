#ifndef __ARCH_FRIONOMMU_CACHE_H
#define __ARCH_FRIONOMMU_CACHE_H

/* bytes per L1 cache line */
#define        L1_CACHE_BYTES  32	/* BlackFin loads 32 bytes for cache */

// For speed we do need to align these ...MaTed---
//  But include/linux/cache.h does this for us if we DO not define ...MaTed---
#define __cacheline_aligned	/***** maybe no need this   Tony *****/
// We don't set  
#define ____cacheline_aligned

#endif
