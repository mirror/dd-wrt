/*
 * include/asm-microblaze/sigcontext.h -- Signal contexts
 *
 *  Copyright (C) 2003  John Williams <jwilliams@itee.uq.edu.au>
 *  Copyright (C) 2001  NEC Corporation
 *  Copyright (C) 2001  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of this
 * archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 * Microblaze port by John Williams
 */

#ifndef __MICROBLAZE_SIGCONTEXT_H__
#define __MICROBLAZE_SIGCONTEXT_H__

#include <asm/ptrace.h>

struct sigcontext
{
	struct pt_regs 	regs;
	unsigned long	oldmask;
};

#endif /* __MICROBLAZE_SIGCONTEXT_H__ */
