/*
 * Copyright (c) 2008 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef __KMEM_H__
#define __KMEM_H__

#define KM_SLEEP	0x0001u
#define KM_NOSLEEP	0x0002u
#define KM_NOFS		0x0004u
#define KM_MAYFAIL	0x0008u
#define KM_LARGE	0x0010u

typedef struct kmem_zone {
	int	zone_unitsize;	/* Size in bytes of zone unit           */
	char	*zone_name;	/* tag name                             */
	int	allocated;	/* debug: How many currently allocated  */
} kmem_zone_t;

extern kmem_zone_t *kmem_zone_init(int, char *);
extern void	*kmem_zone_alloc(kmem_zone_t *, int);
extern void	*kmem_zone_zalloc(kmem_zone_t *, int);

static inline void
kmem_zone_free(kmem_zone_t *zone, void *ptr)
{
	zone->allocated--;
	free(ptr);
}

extern void	*kmem_alloc(size_t, int);
extern void	*kmem_zalloc(size_t, int);

static inline void 
kmem_free(void *ptr) {
	free(ptr);
}

extern void	*kmem_realloc(void *, size_t, size_t, int);

#endif
