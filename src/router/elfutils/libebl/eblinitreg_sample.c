/* Populate process Dwfl_Frame from perf_events sample.

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

#include <libeblP.h>
#include <assert.h>

bool
ebl_sample_sp_pc (Ebl *ebl,
		  const Dwarf_Word *regs, uint32_t n_regs,
		  const int *regs_mapping, size_t n_regs_mapping,
		  Dwarf_Word *sp, Dwarf_Word *pc)
{
  assert (ebl->sample_sp_pc != NULL);
  return ebl->sample_sp_pc (regs, n_regs,
			    regs_mapping, n_regs_mapping,
			    sp, pc);
}

bool
ebl_set_initial_registers_sample (Ebl *ebl,
				  const Dwarf_Word *regs, uint32_t n_regs,
				  const int *regs_mapping, size_t n_regs_mapping,
				  ebl_tid_registers_t *setfunc,
				  void *arg)
{
  /* If set_initial_registers_sample is defined for this arch, use it.  */
  if (ebl->set_initial_registers_sample != NULL)
      return ebl->set_initial_registers_sample (regs, n_regs,
						regs_mapping, n_regs_mapping,
						setfunc, arg);

  /* If set_initial_registers_sample is unspecified, then it is safe
     to use the following generic code to populate a contiguous array
     of dwarf_regs:  */
  Dwarf_Word dwarf_regs[64];
  assert (ebl->frame_nregs < 64);
  size_t i;
  for (i = 0; i < ebl->frame_nregs; i++)
    dwarf_regs[i] = 0x0;
  for (i = 0; i < n_regs; i++)
    {
      if (i > n_regs_mapping)
	break;
      if (regs_mapping[i] < 0 || regs_mapping[i] >= (int)ebl->frame_nregs)
	continue;
      dwarf_regs[regs_mapping[i]] = regs[i];
    }
  return setfunc (0, ebl->frame_nregs, dwarf_regs, arg);
}

bool
ebl_sample_perf_regs_mapping (Ebl *ebl,
			      uint64_t perf_regs_mask, uint32_t abi,
			      const int **regs_mapping, size_t *n_regs_mapping)
{
  /* If sample_perf_regs_mapping is unsupported then PERF_FRAME_REGS_MASK is zero.  */
  assert (ebl->sample_perf_regs_mapping != NULL);
  return ebl->sample_perf_regs_mapping (ebl, perf_regs_mask, abi,
					regs_mapping, n_regs_mapping);
}

uint64_t
ebl_perf_frame_regs_mask (Ebl *ebl)
{
  /* ebl is declared NN */
  return ebl->perf_frame_regs_mask;
}
