#ifndef _IP_SET_MALLOC_H
#define _IP_SET_MALLOC_H

#ifdef __KERNEL__

/* Memory allocation and deallocation */
static size_t max_malloc_size = 0;

static inline void init_max_malloc_size(void)
{
#define CACHE(x) max_malloc_size = x;
#include <linux/kmalloc_sizes.h>
#undef CACHE
}

static inline void * ip_set_malloc_atomic(size_t bytes)
{
	if (bytes > max_malloc_size)
		return __vmalloc(bytes, GFP_ATOMIC, PAGE_KERNEL);
	else
		return kmalloc(bytes, GFP_ATOMIC);
}

static inline void * ip_set_malloc(size_t bytes)
{
	if (bytes > max_malloc_size)
		return vmalloc(bytes);
	else
		return kmalloc(bytes, GFP_KERNEL);
}

static inline void ip_set_free(void * data, size_t bytes)
{
	if (bytes > max_malloc_size)
		vfree(data);
	else
		kfree(data);
}

#endif				/* __KERNEL__ */

#endif /*_IP_SET_MALLOC_H*/
