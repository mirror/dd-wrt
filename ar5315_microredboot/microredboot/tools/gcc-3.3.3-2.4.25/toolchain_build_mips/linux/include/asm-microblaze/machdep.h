/*
 * include/asm-microblaze/machdep.h -- Machine-dependent definitions
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

#ifndef __MICROBLAZE_MACHDEP_H__
#define __MICROBLAZE_MACHDEP_H__

#include <linux/config.h>

/* chips */
#ifdef CONFIG_MICROBLAZE
#include <asm/microblaze.h>
#endif

/* platforms */
#ifdef CONFIG_MBVANILLA
#include <asm/mbvanilla.h>
#endif
#ifdef CONFIG_MBEGRET
#include <asm/mbegret.h>
#endif

#endif /* __MICROBLAZE_MACHDEP_H__ */
