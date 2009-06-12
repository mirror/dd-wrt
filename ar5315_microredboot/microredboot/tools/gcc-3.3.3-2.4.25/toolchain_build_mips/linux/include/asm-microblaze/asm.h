/*
 * include/asm-microblaze/asm.h -- Macros for writing assembly code
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

#define G_ENTRY(name)							      \
   .align 4;								      \
   .globl name;								      \
   .type  name,@function;						      \
   name
#define END(name)							      \
   .size  name,.-name

#define L_ENTRY(name)							      \
   .align 4;								      \
   .type  name,@function;						      \
   name
