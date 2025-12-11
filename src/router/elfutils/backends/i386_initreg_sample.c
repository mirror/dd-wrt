/* Populate process registers from a linux perf_events sample.
   Copyright (C) 2025 Red Hat, Inc.
   This file is part of elfutils.

   This file is free software; you can redistribute it and/or modify
   it under the terms of either

     * the GNU Lesser General Public License as published by the Free
       Software Foundation; either version 3 of the License, or (at
       your option) any later version

   or

     * the GNU General Public License as published by the Free
       Software Foundation; either version 2 of the License, or (at
       your option) any later version

   or both in parallel, as here.

   elfutils is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received copies of the GNU General Public License and
   the GNU Lesser General Public License along with this program.  If
   not, see <http://www.gnu.org/licenses/>.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <assert.h>
#if (defined __i386__ || defined __x86_64__) && defined(__linux__)
# include <linux/perf_event.h>
# include <asm/perf_regs.h>
#endif

#define BACKEND i386_
#include "libebl_CPU.h"
#include "libebl_PERF_FLAGS.h"
#if (defined __i386__ || defined __x86_64__) && defined(__linux__)
# include "x86_initreg_sample.c"
# define HAVE_X86_INITREG_SAMPLE
#endif

bool
i386_sample_sp_pc (const Dwarf_Word *regs, uint32_t n_regs,
                   const int *regs_mapping, uint32_t n_regs_mapping,
                   Dwarf_Word *sp, Dwarf_Word *pc)
{
#ifdef HAVE_X86_INITREG_SAMPLE
  /* XXX for dwarf_regs indices, compare i386_initreg.c */
  return x86_sample_sp_pc (regs, n_regs, regs_mapping, n_regs_mapping,
			   sp, 4 /* index of sp in dwarf_regs */,
			   pc, 8 /* index of pc in dwarf_regs */);
#else
  (void) regs;
  (void) n_regs;
  (void) regs_mapping;
  (void) n_regs_mapping;
  (void) sp;
  (void) pc;
  return false;
#endif
}

bool
i386_sample_perf_regs_mapping (Ebl *ebl,
			       uint64_t perf_regs_mask, uint32_t abi,
			       const int **regs_mapping,
			       size_t *n_regs_mapping)
{
#ifdef HAVE_X86_INITREG_SAMPLE
  return x86_sample_perf_regs_mapping (ebl, perf_regs_mask, abi,
				       regs_mapping, n_regs_mapping);
#else
  (void) ebl;
  (void) perf_regs_mask;
  (void) abi;
  (void) regs_mapping;
  (void) n_regs_mapping;
  return false;
#endif
}
