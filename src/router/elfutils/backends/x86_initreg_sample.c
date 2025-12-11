/* x86 stack sample register handling, pieces common to x86-64 and i386.
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

static bool
x86_sample_sp_pc (const Dwarf_Word *regs, uint32_t n_regs,
		  const int *regs_mapping, uint32_t n_regs_mapping,
		  Dwarf_Word *sp, uint sp_index /* into dwarf_regs */,
		  Dwarf_Word *pc, uint pc_index /* into dwarf_regs */)
{
  if (sp != NULL) *sp = 0;
  if (pc != NULL) *pc = 0;
#if !defined(__x86_64__)
  (void)regs;
  (void)n_regs;
  (void)regs_mapping;
  (void)n_regs_mapping;
  (void)sp;
  (void)sp_index;
  (void)pc;
  (void)pc_index;
  return false;
#else /* __x86_64__ */
  /* TODO: Register locations could be cached and rechecked on a
     fastpath without needing to loop? */
  int j, need_sp = (sp != NULL), need_pc = (pc != NULL);
  for (j = 0; (need_sp || need_pc) && n_regs_mapping > (uint32_t)j; j++)
    {
      if (n_regs < (uint32_t)j) break;
      if (need_sp && regs_mapping[j] == (int)sp_index)
	{
	  *sp = regs[j]; need_sp = false;
	}
      if (need_pc && regs_mapping[j] == (int)pc_index)
	{
	  *pc = regs[j]; need_pc = false;
	}
    }
  return (!need_sp && !need_pc);
#endif
}

static bool
x86_sample_perf_regs_mapping (Ebl *ebl,
			      uint64_t perf_regs_mask, uint32_t abi,
			      const int **regs_mapping,
			      size_t *n_regs_mapping)
{
  if (perf_regs_mask != 0 && ebl->cached_perf_regs_mask == perf_regs_mask)
    {
      *regs_mapping = ebl->cached_regs_mapping;
      *n_regs_mapping = ebl->cached_n_regs_mapping;
      return true;
    }

  /* The following facts are needed to translate x86 registers correctly:
     - perf register order seen in linux arch/x86/include/uapi/asm/perf_regs.h
       The registers array is built in the same order as the enum!
       (See the code in tools/perf/util/intel-pt.c intel_pt_add_gp_regs().)
     - EBL PERF_FRAME_REGS_MASK specifies all registers except segment and
       flags.  However, regs_mask might be a different set of registers.
       Again, regs_mask bits are in asm/perf_regs.h enum order.
     - dwarf register order seen in elfutils backends/{x86_64,i386}_initreg.c
       (matching pt_regs struct in linux arch/x86/include/asm/ptrace.h)
       and it's a fairly different register order!

     For comparison, you can study codereview.qt-project.org/gitweb?p=qt-creator/perfparser.git;a=blob;f=app/perfregisterinfo.cpp;hb=HEAD
     and follow the code which uses those tables of magic numbers.
     But it's better to follow original sources of truth for this.  */

  bool is_abi32 = (abi == PERF_SAMPLE_REGS_ABI_32);

  /* Locations of dwarf_regs in the perf_event_x86_regs enum order,
     not the regs[] array (which will include a subset of the regs):  */
  static const int regs_i386[] = {0, 2, 3, 1, 7/*sp*/, 6, 4, 5, 8/*ip*/};
  static const int regs_x86_64[] = {0, 3, 2, 1, 4, 5, 6, 7/*sp*/,
				    16/*r8 after flags+segment*/, 17, 18, 19, 20, 21, 22, 23,
				    8/*ip*/};
  const int *dwarf_to_perf = is_abi32 ? regs_i386 : regs_x86_64;

  /* Count bits and allocate regs_mapping:  */
  int j, k, kmax, count; uint64_t bit;
  for (k = 0, kmax = -1, count = 0, bit = 1;
       k < PERF_REG_X86_64_MAX; k++, bit <<= 1)
    {
      if ((bit & perf_regs_mask)) {
	count++;
	kmax = k;
      }
    }
  ebl->cached_perf_regs_mask = perf_regs_mask;
  ebl->cached_regs_mapping = (int *)calloc (count, sizeof(int));
  ebl->cached_n_regs_mapping = count;

  /* Locations of perf_regs in the regs[] array, according to
     perf_regs_mask:  */
  int perf_to_regs[PERF_REG_X86_64_MAX];
  uint64_t expected_mask = is_abi32 ?
    PERF_FRAME_REGISTERS_I386 : PERF_FRAME_REGISTERS_X86_64;
  for (j = 0, k = 0, bit = 1; k <= kmax; k++, bit <<= 1)
    {
      if ((bit & expected_mask) && (bit & perf_regs_mask))
	{
	  perf_to_regs[k] = j;
	  j++;
	}
      else
	{
	  perf_to_regs[k] = -1;
	}
    }
  if (j > (int)ebl->cached_n_regs_mapping)
      return false;

  /* Locations of perf_regs in the dwarf_regs array, according to
     perf_regs_mask and perf_to_regs[]:  */
  for (size_t i = 0; i < ebl->frame_nregs; i++)
    {
      k = dwarf_to_perf[i];
      j = perf_to_regs[k];
      if (j < 0) continue;
      ebl->cached_regs_mapping[j] = i;
    }

  *regs_mapping = ebl->cached_regs_mapping;
  *n_regs_mapping = ebl->cached_n_regs_mapping;
  return true;
}
