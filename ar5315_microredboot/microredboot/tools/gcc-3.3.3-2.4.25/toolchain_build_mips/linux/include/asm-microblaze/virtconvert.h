/*
 * include/asm-microblaze/virtconvert.h -- conversion between virtual and
 * 	physical mappings
 *
 *  Copyright (C) 2003       John Williams <jwilliams@itee.uq.edu.au>
 *  Copyright (C) 2001,2002  NEC Corporation
 *  Copyright (C) 2001,2002  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of this
 * archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 * Microblaze port by John Williams
 */

#ifndef __MICROBLAZE_VIRTCONVERT_H__
#define __MICROBLAZE_VIRTCONVERT_H__

#ifdef __KERNEL__

#include <asm/page.h>

#define mm_ptov(addr)		((void *)__phys_to_virt (addr))
#define mm_vtop(addr)		((unsigned long)__virt_to_phys (addr))
#define phys_to_virt(addr)	((void *)__phys_to_virt (addr))
#define virt_to_phys(addr)	((unsigned long)__virt_to_phys (addr))

#define __page_address(page)	(PAGE_OFFSET + (((page) - mem_map) << PAGE_SHIFT))
#define page_to_phys(page)	virt_to_phys((void *)__page_address(page))

#endif /* __KERNEL__ */

#endif /* __MICROBLAZE_VIRTCONVERT_H__ */
