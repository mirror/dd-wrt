#ifndef SRC_MOD_COMMON_WKMALLOC_H_
#define SRC_MOD_COMMON_WKMALLOC_H_

#include <linux/slab.h>
#include "common/types.h"

void wkmalloc_add(const char *name);
void wkmalloc_rm(const char *name, void *obj);
void wkmalloc_print_leaks(void);
void wkmalloc_teardown(void);

static inline void *__wkmalloc(const char *name, size_t size, gfp_t flags)
{
	void *result;

	result = kmalloc(size, flags);
#ifdef JKMEMLEAK
	if (result)
		wkmalloc_add(name);
#endif

	return result;
}

/**
 * A "wrapped kernel memory allocation"; a wrapped kmalloc.
 */
#define wkmalloc(type, flags) __wkmalloc(#type, sizeof(type), flags)

static inline void __wkfree(const char *name, void *obj)
{
	kfree(obj);
#ifdef JKMEMLEAK
	wkmalloc_rm(name, obj);
#endif
}

#define wkfree(type, obj) __wkfree(#type, obj)

static inline void *wkmem_cache_alloc(const char *name,
		struct kmem_cache *cache, gfp_t flags)
{
	void *result;

	result = kmem_cache_alloc(cache, flags);
#ifdef JKMEMLEAK
	if (result)
		wkmalloc_add(name);
#endif

	return result;
}

static inline void wkmem_cache_free(const char *name, struct kmem_cache *cache,
		void *obj)
{
	kmem_cache_free(cache, obj);
#ifdef JKMEMLEAK
	wkmalloc_rm(name, obj);
#endif
}

#endif /* SRC_MOD_COMMON_WKMALLOC_H_ */
