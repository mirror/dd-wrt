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

typedef struct kmem_zone {
	int	zone_unitsize;	/* Size in bytes of zone unit           */
	char	*zone_name;	/* tag name                             */
	int	allocated;	/* debug: How many currently allocated  */
} kmem_zone_t;

extern kmem_zone_t *kmem_zone_init(int, char *);
extern void	*kmem_zone_alloc(kmem_zone_t *, int);
extern void	*kmem_zone_zalloc(kmem_zone_t *, int);
extern int	kmem_zone_destroy(kmem_zone_t *);

static inline void
kmem_cache_free(kmem_zone_t *zone, void *ptr)
{
	zone->allocated--;
	free(ptr);
}

extern void	*kmem_alloc(size_t, int);
extern void	*kmem_alloc_large(size_t, int);
extern void	*kmem_zalloc(size_t, int);

static inline void
kmem_free(void *ptr) {
	free(ptr);
}

extern void	*kmem_realloc(void *, size_t, int);

#endif
