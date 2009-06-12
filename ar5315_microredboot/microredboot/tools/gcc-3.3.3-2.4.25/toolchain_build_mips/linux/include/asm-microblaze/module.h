/*
 * include/asm-microblaze/module.h -- Architecture-specific module hooks
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

#ifndef __MICROBLAZE_MODULE_H__
#define __MICROBLAZE_MODULE_H__

#define arch_init_modules(x)	((void)0)
#define module_arch_init(x)	(0)
#define module_map(sz)		vmalloc (sz)
#define module_unmap(sz)	vfree (sz)

#endif /* __MICROBLAZE_MODULE_H__ */
