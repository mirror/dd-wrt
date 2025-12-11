/* MIPS ABI-specified defaults for DWARF CFI.
   Copyright (C) 2009 Red Hat, Inc.
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

#include <dwarf.h>

#define BACKEND mips_
#include "libebl_CPU.h"

int
mips_abi_cfi (Ebl *ebl __attribute__ ((unused)), Dwarf_CIE *abi_info)
{
  static const uint8_t abi_cfi[] =
    {
      DW_CFA_def_cfa, ULEB128_7 (31), ULEB128_7 (0),
      /* Callee-saved regs.  */
      DW_CFA_same_value, ULEB128_7 (16), /* s0 */
      DW_CFA_same_value, ULEB128_7 (17), /* s1 */
      DW_CFA_same_value, ULEB128_7 (18), /* s2 */
      DW_CFA_same_value, ULEB128_7 (19), /* s3 */
      DW_CFA_same_value, ULEB128_7 (20), /* s4 */
      DW_CFA_same_value, ULEB128_7 (21), /* s5 */
      DW_CFA_same_value, ULEB128_7 (22), /* s6 */
      DW_CFA_same_value, ULEB128_7 (23), /* s7 */
      DW_CFA_same_value, ULEB128_7 (28), /* gp */
      DW_CFA_same_value, ULEB128_7 (29), /* sp */
      DW_CFA_same_value, ULEB128_7 (30), /* fp */

      DW_CFA_val_offset, ULEB128_7 (29), ULEB128_7 (0),
    };

  abi_info->initial_instructions = abi_cfi;
  abi_info->initial_instructions_end = &abi_cfi[sizeof abi_cfi];
  abi_info->data_alignment_factor = 8;

  abi_info->return_address_register = 31; /* %ra */

  return 0;
}
