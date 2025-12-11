/* Fetch live process registers from TID.
   Copyright (C) 2024 CIP United Inc.
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
#if (defined(mips) || defined(__mips) || defined(__mips__) || defined(MIPS) || defined(__MIPS__)) && defined(__linux__)
# include <sys/user.h>
# include <sys/ptrace.h>
#include <asm/ptrace.h>
#endif

#define BACKEND mips_
#include "libebl_CPU.h"


bool
mips_set_initial_registers_tid (pid_t tid __attribute__ ((unused)),
			  ebl_tid_registers_t *setfunc __attribute__ ((unused)),
				  void *arg __attribute__ ((unused)))
{
#if (!defined(mips) && !defined(__mips) && !defined(__mips__) && !defined(MIPS) && !defined(__MIPS__)) || !defined(__linux__)
  return false;
#else /* __mips__ */
/* For PTRACE_GETREGS */

  struct pt_regs gregs;
  if (ptrace (PTRACE_GETREGS, tid, 0, &gregs) != 0)
    return false;
  if (! setfunc (-1, 1, (Dwarf_Word *) &gregs.cp0_epc, arg))
    return false;
  return setfunc (0, 32, (Dwarf_Word *) &gregs.regs[0], arg);
#endif /* __mips__ */
}
