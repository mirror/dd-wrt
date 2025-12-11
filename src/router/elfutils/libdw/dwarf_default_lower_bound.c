/* Get the default subrange lower bound for a given language.
   Copyright (C) 2016 Red Hat, Inc.
   Copyright (C) 2024, 2025 Mark J. Wielaard <mark@klomp.org>
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
#include "libdwP.h"

/* Determine default lower bound from language, as per the DWARF5
   "Subrange Type Entries" table.  */
int
dwarf_default_lower_bound (int lang, Dwarf_Sword *result)
{
  switch (lang)
    {
    case DW_LANG_C:
    case DW_LANG_C89:
    case DW_LANG_C99:
    case DW_LANG_C11:
    case DW_LANG_C_plus_plus:
    case DW_LANG_C_plus_plus_03:
    case DW_LANG_C_plus_plus_11:
    case DW_LANG_C_plus_plus_14:
    case DW_LANG_ObjC:
    case DW_LANG_ObjC_plus_plus:
    case DW_LANG_Java:
    case DW_LANG_D:
    case DW_LANG_Python:
    case DW_LANG_UPC:
    case DW_LANG_OpenCL:
    case DW_LANG_Go:
    case DW_LANG_Haskell:
    case DW_LANG_OCaml:
    case DW_LANG_Rust:
    case DW_LANG_Swift:
    case DW_LANG_Dylan:
    case DW_LANG_RenderScript:
    case DW_LANG_BLISS:
    case DW_LANG_Kotlin:
    case DW_LANG_Zig:
    case DW_LANG_Crystal:
    case DW_LANG_C_plus_plus_17:
    case DW_LANG_C_plus_plus_20:
    case DW_LANG_C17:
    case DW_LANG_HIP:
    case DW_LANG_Assembly:
    case DW_LANG_C_sharp:
    case DW_LANG_Mojo:
    case DW_LANG_GLSL:
    case DW_LANG_GLSL_ES:
    case DW_LANG_HLSL:
    case DW_LANG_OpenCL_CPP:
    case DW_LANG_CPP_for_OpenCL:
    case DW_LANG_SYCL:
    case DW_LANG_C_plus_plus_23:
    case DW_LANG_Odin:
    case DW_LANG_P4:
    case DW_LANG_Metal:
    case DW_LANG_C23:
    case DW_LANG_Ruby:
    case DW_LANG_Move:
    case DW_LANG_Hylo:
    case DW_LANG_V:
    case DW_LANG_Nim:
      *result = 0;
      return 0;

    case DW_LANG_Ada83:
    case DW_LANG_Ada95:
    case DW_LANG_Cobol74:
    case DW_LANG_Cobol85:
    case DW_LANG_Fortran77:
    case DW_LANG_Fortran90:
    case DW_LANG_Fortran95:
    case DW_LANG_Fortran03:
    case DW_LANG_Fortran08:
    case DW_LANG_Pascal83:
    case DW_LANG_Modula2:
    case DW_LANG_Modula3:
    case DW_LANG_PLI:
    case DW_LANG_Julia:
    case DW_LANG_Fortran18:
    case DW_LANG_Ada2005:
    case DW_LANG_Ada2012:
    case DW_LANG_Fortran23:
    case DW_LANG_Algol68:
      *result = 1;
      return 0;

    /* Special case vendor Assembly variant.  */
    case DW_LANG_Mips_Assembler:
      *result = 0;
      return 0;

    default:
      __libdw_seterrno (DWARF_E_UNKNOWN_LANGUAGE);
      return -1;
    }
}
INTDEF (dwarf_default_lower_bound)

/* Determine default lower bound from language, as per the DWARF6
   https://dwarfstd.org/languages-v6.html table.  */
int
dwarf_language_lower_bound (Dwarf_Word lang, Dwarf_Sword *result)
{
  switch (lang)
    {
    case DW_LNAME_BLISS:
    case DW_LNAME_C:
    case DW_LNAME_C_plus_plus:
    case DW_LNAME_Crystal:
    case DW_LNAME_D:
    case DW_LNAME_Dylan:
    case DW_LNAME_Go:
    case DW_LNAME_Haskell:
    case DW_LNAME_Java:
    case DW_LNAME_Kotlin:
    case DW_LNAME_ObjC:
    case DW_LNAME_ObjC_plus_plus:
    case DW_LNAME_OCaml:
    case DW_LNAME_OpenCL_C:
    case DW_LNAME_Python:
    case DW_LNAME_RenderScript:
    case DW_LNAME_Rust:
    case DW_LNAME_Swift:
    case DW_LNAME_UPC:
    case DW_LNAME_Zig:
    case DW_LNAME_Assembly:
    case DW_LNAME_C_sharp:
    case DW_LNAME_Mojo:
    case DW_LNAME_GLSL:
    case DW_LNAME_GLSL_ES:
    case DW_LNAME_HLSL:
    case DW_LNAME_OpenCL_CPP:
    case DW_LNAME_CPP_for_OpenCL:
    case DW_LNAME_SYCL:
    case DW_LNAME_Ruby:
    case DW_LNAME_Move:
    case DW_LNAME_Hylo:
    case DW_LNAME_HIP:
    case DW_LNAME_Odin:
    case DW_LNAME_P4:
    case DW_LNAME_Metal:
    case DW_LNAME_V:
    case DW_LNAME_Nim:
      *result = 0;
      return 0;

    case DW_LNAME_Ada:
    case DW_LNAME_Cobol:
    case DW_LNAME_Fortran:
    case DW_LNAME_Julia:
    case DW_LNAME_Modula2:
    case DW_LNAME_Modula3:
    case DW_LNAME_Pascal:
    case DW_LNAME_PLI:
    case DW_LNAME_Algol68:
      *result = 1;
      return 0;

    default:
      __libdw_seterrno (DWARF_E_UNKNOWN_LANGUAGE);
      return -1;
    }
}
INTDEF (dwarf_language_lower_bound)
