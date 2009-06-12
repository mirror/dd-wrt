/*
 * linux/include/asm-arm/proc-armv/io.h
 */

/*
 * The caches on some architectures aren't dma-coherent and have need to
 * handle this in software.  There are two types of operations that
 * can be applied to dma buffers.
 *
 *  - dma_cache_wback_inv(start, size) makes caches and RAM coherent by
 *    writing the content of the caches back to memory, if necessary.
 *    The function also invalidates the affected part of the caches as
 *    necessary before DMA transfers from outside to memory.
 *  - dma_cache_inv(start, size) invalidates the affected parts of the
 *    caches.  Dirty lines of the caches may be written back or simply
 *    be discarded.  This operation is necessary before dma operations
 *    to the memory.
 *  - dma_cache_wback(start, size) writes back any dirty lines but does
 *    not invalidate the cache.  This can be used before DMA reads from
 *    memory,
 */

#include <asm/proc-fns.h>

#define dma_cache_inv(start, size)				\
	do { cpu_cache_purge_area((unsigned long)(start),	\
		((unsigned long)(start)+(size))); } while (0)

#define dma_cache_wback(start, size)				\
	do { cpu_cache_wback_area((unsigned long)(start),	\
		((unsigned long)(start)+(size))); } while (0)

#define dma_cache_wback_inv(start, size)			\
	do { cpu_flush_cache_area((unsigned long)(start),	\
		((unsigned long)(start)+(size)), 0); } while (0)
