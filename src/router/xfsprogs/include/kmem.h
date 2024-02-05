// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2008 Silicon Graphics, Inc.
 * All Rights Reserved.
 */
#ifndef __KMEM_H__
#define __KMEM_H__

#define KM_NOFS		0x0004u
#define KM_MAYFAIL	0x0008u
#define KM_LARGE	0x0010u
#define KM_NOLOCKDEP	0x0020u

struct kmem_cache {
	int		cache_unitsize;	/* Size in bytes of cache unit */
	int		allocated;	/* debug: How many allocated? */
	unsigned int	align;
	const char	*cache_name;	/* tag name */
	void		(*ctor)(void *);
};

typedef unsigned int __bitwise gfp_t;

#define GFP_KERNEL	((__force gfp_t)0)
#define GFP_NOFS	((__force gfp_t)0)
#define __GFP_NOFAIL	((__force gfp_t)0)
#define __GFP_NOLOCKDEP	((__force gfp_t)0)

#define __GFP_ZERO	(__force gfp_t)1

struct kmem_cache * kmem_cache_create(const char *name, unsigned int size,
		unsigned int align, unsigned int slab_flags,
		void (*ctor)(void *));

static inline struct kmem_cache *
kmem_cache_init(unsigned int size, const char *name)
{
	return kmem_cache_create(name, size, 0, 0, NULL);
}

extern void	*kmem_cache_alloc(struct kmem_cache *, gfp_t);
extern void	*kmem_cache_zalloc(struct kmem_cache *, gfp_t);
extern int	kmem_cache_destroy(struct kmem_cache *);

static inline void
kmem_cache_free(struct kmem_cache *cache, void *ptr)
{
	cache->allocated--;
	free(ptr);
}

extern void	*kmem_alloc(size_t, int);
extern void	*kvmalloc(size_t, gfp_t);
extern void	*kmem_zalloc(size_t, int);

static inline void
kmem_free(const void *ptr) {
	free((void *)ptr);
}

extern void	*krealloc(void *, size_t, int);

static inline void *kmalloc(size_t size, gfp_t flags)
{
	return kvmalloc(size, flags);
}

static inline void kfree(const void *ptr)
{
	return kmem_free(ptr);
}

#endif
