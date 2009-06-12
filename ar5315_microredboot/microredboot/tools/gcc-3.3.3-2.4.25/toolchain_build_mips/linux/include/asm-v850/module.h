/*
 * include/asm-v850/module.h -- Architecture-specific module hooks
 *
 *  Copyright (C) 2001,02  NEC Corporation
 *  Copyright (C) 2001,02  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of this
 * archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 */

#ifndef __V850_MODULE_H__
#define __V850_MODULE_H__

#define arch_init_modules(x)	((void)0)
#define module_arch_init(x)	(0)
#define module_map(sz)		vmalloc (sz)
#define module_unmap(sz)	vfree (sz)

#endif /* __V850_MODULE_H__ */
