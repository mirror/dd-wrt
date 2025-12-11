/* Initialization of i386 specific backend library.
   Copyright (C) 2000-2009, 2013, 2017, 2025 Red Hat, Inc.
   This file is part of elfutils.
   Written by Ulrich Drepper <drepper@redhat.com>, 2000.

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

#define BACKEND		i386_
#define RELOC_PREFIX	R_386_
#include "libebl_CPU.h"
#include "libebl_PERF_FLAGS.h"

/* This defines the common reloc hooks based on i386_reloc.def.  */
#include "common-reloc.c"

Ebl *
i386_init (Elf *elf __attribute__ ((unused)),
	   GElf_Half machine __attribute__ ((unused)),
	   Ebl *eh)
{
  /* We handle it.  */
  i386_init_reloc (eh);
  HOOK (eh, reloc_simple_type);
  HOOK (eh, gotpc_reloc_check);
  HOOK (eh, core_note);
  generic_debugscn_p = eh->debugscn_p;
  HOOK (eh, debugscn_p);
  HOOK (eh, return_value_location);
  HOOK (eh, register_info);
  HOOK (eh, auxv_info);
  HOOK (eh, disasm);
  HOOK (eh, abi_cfi);
  /* gcc/config/ #define DWARF_FRAME_REGISTERS.  For i386 it is 17, why?
     (Likely an artifact of reusing that header between i386/x86_64.)  */
  eh->frame_nregs = 9;
  HOOK (eh, set_initial_registers_tid);
  /* set_initial_registers_sample is default ver */
  HOOK (eh, sample_sp_pc);
  HOOK (eh, sample_perf_regs_mapping);
  eh->perf_frame_regs_mask = PERF_FRAME_REGISTERS_I386;
  eh->cached_perf_regs_mask = 0;
  eh->cached_regs_mapping = NULL;
  eh->cached_n_regs_mapping = -1;
  HOOK (eh, unwind);

  return eh;
}
