/*
 * include/linux/memleak.h
 *
 * Copyright (C) 2006 ARM Limited
 * Written by Catalin Marinas <catalin.marinas@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef __MEMLEAK_H
#define __MEMLEAK_H

#include <linux/stddef.h>

#ifdef CONFIG_DEBUG_MEMLEAK

struct memleak_offset {
	unsigned long offset;
	unsigned long size;
	unsigned long member_size;
};

/* if offsetof(type, member) is not a constant known at compile time,
 * just use 0 instead since we cannot add it to the
 * .init.memleak_offsets section
 */
#define memleak_offsetof(type, member)				\
	(__builtin_constant_p(offsetof(type, member)) ?		\
	 offsetof(type, member) : 0)

#define DECLARE_MEMLEAK_OFFSET(name, type, member)		\
	static const struct memleak_offset			\
	__attribute__ ((__section__ (".init.memleak_offsets")))	\
	__attribute_used__ __memleak_offset__##name = {		\
		memleak_offsetof(type, member),			\
		sizeof(type),					\
		sizeof(((type *)0)->member)			\
	}

extern void memleak_init(void);
extern void memleak_alloc(const void *ptr, size_t size, int ref_count);
extern void memleak_free(const void *ptr);
extern void memleak_padding(const void *ptr, unsigned long offset, size_t size);
extern void memleak_not_leak(const void *ptr);
extern void memleak_ignore(const void *ptr);
extern void memleak_scan_area(const void *ptr, unsigned long offset, size_t length);
extern void memleak_insert_aliases(struct memleak_offset *ml_off_start,
				   struct memleak_offset *ml_off_end);

#define memleak_erase(ptr)	do { (ptr) = NULL; } while (0)
#define memleak_container(type, member)	{			\
	DECLARE_MEMLEAK_OFFSET(container_of, type, member);	\
}

#else

#define DECLARE_MEMLEAK_OFFSET(name, type, member)

#define memleak_init()
#define memleak_alloc(ptr, size, ref_count)
#define memleak_free(ptr)
#define memleak_padding(ptr, offset, size)
#define memleak_not_leak(ptr)
#define memleak_ignore(ptr)
#define memleak_scan_area(ptr, offset, length)
#define memleak_insert_aliases(ml_off_start, ml_off_end)
#define memleak_erase(ptr)
#define memleak_container(type, member)

#endif	/* CONFIG_DEBUG_MEMLEAK */

#endif	/* __MEMLEAK_H */
