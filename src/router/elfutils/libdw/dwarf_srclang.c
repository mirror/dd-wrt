/* Return source language attribute of DIE.
   Copyright (C) 2003-2010 Red Hat, Inc.
   Copyright (C) 2025 Mark J. Wielaard <mark@klomp.org>
   This file is part of elfutils.
   Written by Ulrich Drepper <drepper@redhat.com>, 2003.

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


static int srclang_to_language (Dwarf_Word srclang,
				Dwarf_Word *lname,
				Dwarf_Word *lversion)
{
  switch (srclang)
    {
    case DW_LANG_C89:
      *lname = DW_LNAME_C;
      *lversion = 198912;
      return 0;
    case DW_LANG_C:
      *lname = DW_LNAME_C;
      *lversion = 0;
      return 0;
    case DW_LANG_Ada83:
      *lname = DW_LNAME_Ada;
      *lversion = 1983;
      return 0;
    case DW_LANG_C_plus_plus:
      *lname = DW_LNAME_C_plus_plus;
      *lversion = 199711;
      return 0;
    case DW_LANG_Cobol74:
      *lname = DW_LNAME_Cobol;
      *lversion = 1974;
      return 0;
    case DW_LANG_Cobol85:
      *lname = DW_LNAME_Cobol;
      *lversion = 1985;
      return 0;
    case DW_LANG_Fortran77:
      *lname = DW_LNAME_Fortran;
      *lversion = 1977;
      return 0;
    case DW_LANG_Fortran90:
      *lname = DW_LNAME_Fortran;
      *lversion = 1990;
      return 0;
    case DW_LANG_Pascal83:
      *lname = DW_LNAME_Pascal;
      *lversion = 1983;
      return 0;
    case DW_LANG_Modula2:
      *lname = DW_LNAME_Modula2;
      *lversion = 0;
      return 0;
    case DW_LANG_Java:
      *lname = DW_LNAME_Java;
      *lversion = 0;
      return 0;
    case DW_LANG_C99:
      *lname = DW_LNAME_C;
      *lversion = 199901;
      return 0;
    case DW_LANG_Ada95:
      *lname = DW_LNAME_Ada;
      *lversion = 1995;
      return 0;
    case DW_LANG_Fortran95:
      *lname = DW_LNAME_Fortran;
      *lversion = 1995;
      return 0;
    case DW_LANG_PLI:
      *lname = DW_LNAME_PLI;
      *lversion = 0;
      return 0;
    case DW_LANG_ObjC:
      *lname = DW_LNAME_ObjC;
      *lversion = 0;
      return 0;
    case DW_LANG_ObjC_plus_plus:
      *lname = DW_LNAME_ObjC_plus_plus;
      *lversion = 0;
      return 0;
    case DW_LANG_UPC:
      *lname = DW_LNAME_UPC;
      *lversion = 0;
      return 0;
    case DW_LANG_D:
      *lname = DW_LNAME_D;
      *lversion = 0;
      return 0;
    case DW_LANG_Dylan:
      *lname = DW_LNAME_Dylan;
      *lversion = 0;
      return 0;
    case DW_LANG_Python:
      *lname = DW_LNAME_Python;
      *lversion = 0;
      return 0;
    case DW_LANG_OpenCL:
      *lname = DW_LNAME_OpenCL_C;
      *lversion = 0;
      return 0;
    case DW_LANG_Go:
      *lname = DW_LNAME_Go;
      *lversion = 0;
      return 0;
    case DW_LANG_Modula3:
      *lname = DW_LNAME_Modula3;
      *lversion = 0;
      return 0;
    case DW_LANG_Haskell:
      *lname = DW_LNAME_Haskell;
      *lversion = 0;
      return 0;
    case DW_LANG_C_plus_plus_03:
      *lname = DW_LNAME_C_plus_plus;
      *lversion = 199711; /* This is really just c++98. */
      return 0;
    case DW_LANG_C_plus_plus_11:
      *lname = DW_LNAME_C_plus_plus;
      *lversion = 201103;
      return 0;
    case DW_LANG_OCaml:
      *lname = DW_LNAME_OCaml;
      *lversion = 0;
      return 0;
    case DW_LANG_Rust:
      *lname = DW_LNAME_Rust;
      *lversion = 0;
      return 0;
    case DW_LANG_C11:
      *lname = DW_LNAME_C;
      *lversion = 201112;
      return 0;
    case DW_LANG_Swift:
      *lname = DW_LNAME_Swift;
      *lversion = 0;
      return 0;
    case DW_LANG_Julia:
      *lname = DW_LNAME_Julia;
      *lversion = 0;
      return 0;
    case DW_LANG_C_plus_plus_14:
      *lname = DW_LNAME_C_plus_plus;
      *lversion = 201402;
      return 0;
    case DW_LANG_Fortran03:
      *lname = DW_LNAME_Fortran;
      *lversion = 2003;
      return 0;
    case DW_LANG_Fortran08:
      *lname = DW_LNAME_Fortran;
      *lversion = 2008;
      return 0;
    case DW_LANG_RenderScript:
      *lname = DW_LNAME_RenderScript;
      *lversion = 0;
      return 0;
    case DW_LANG_BLISS:
      *lname = DW_LNAME_BLISS;
      *lversion = 0;
      return 0;
    case DW_LANG_Kotlin:
      *lname = DW_LNAME_Kotlin;
      *lversion = 0;
      return 0;
    case DW_LANG_Zig:
      *lname = DW_LNAME_Zig;
      *lversion = 0;
      return 0;
    case DW_LANG_Crystal:
      *lname = DW_LNAME_Crystal;
      *lversion = 0;
      return 0;
    case DW_LANG_C_plus_plus_17:
      *lname = DW_LANG_C_plus_plus;
      *lversion = 201703;
      return 0;
    case DW_LANG_C_plus_plus_20:
      *lname = DW_LANG_C_plus_plus;
      *lversion = 202002;
      return 0;
    case DW_LANG_C17:
      *lname = DW_LNAME_C;
      *lversion = 201710;
      return 0;
    case DW_LANG_Fortran18:
      *lname = DW_LNAME_Fortran;
      *lversion = 2018;
      return 0;
    case DW_LANG_Ada2005:
      *lname = DW_LNAME_Ada;
      *lversion = 2005;
      return 0;
    case DW_LANG_Ada2012:
      *lname = DW_LNAME_Ada;
      *lversion = 2012;
      return 0;
    case DW_LANG_HIP:
      *lname = DW_LNAME_HIP;
      *lversion = 0;
      return 0;
    case DW_LANG_Assembly:
    case DW_LANG_Mips_Assembler:
      *lname = DW_LNAME_Assembly;
      *lversion = 0;
      return 0;
    case DW_LANG_C_sharp:
      *lname = DW_LNAME_C_sharp;
      *lversion = 0;
      return 0;
    case DW_LANG_Mojo:
      *lname = DW_LNAME_Mojo;
      *lversion = 0;
      return 0;
    case DW_LANG_GLSL:
      *lname = DW_LNAME_GLSL;
      *lversion = 0;
      return 0;
    case DW_LANG_GLSL_ES:
      *lname = DW_LNAME_GLSL_ES;
      *lversion = 0;
      return 0;
    case DW_LANG_HLSL:
      *lname = DW_LNAME_HLSL;
      *lversion = 0;
      return 0;
    case DW_LANG_OpenCL_CPP:
      *lname = DW_LNAME_OpenCL_CPP;
      *lversion = 0;
      return 0;
    case DW_LANG_CPP_for_OpenCL:
      *lname = DW_LNAME_CPP_for_OpenCL;
      *lversion = 0;
      return 0;
    case DW_LANG_SYCL:
      *lname = DW_LNAME_SYCL;
      *lversion = 0;
      return 0;
    case DW_LANG_C_plus_plus_23:
      *lname = DW_LNAME_C_plus_plus;
      *lversion = 202302;
      return 0;
    case DW_LANG_Odin:
      *lname = DW_LNAME_Odin;
      *lversion = 0;
      return 0;
    case DW_LANG_P4:
      *lname = DW_LNAME_P4;
      *lversion = 0;
      return 0;
    case DW_LANG_Metal:
      *lname = DW_LNAME_Metal;
      *lversion = 0;
      return 0;
    case DW_LANG_C23:
      *lname = DW_LNAME_C;
      *lversion = 202311;
      return 0;
    case DW_LANG_Fortran23:
      *lname = DW_LNAME_Fortran;
      *lversion = 2023;
      return 0;
    case DW_LANG_Ruby:
      *lname = DW_LNAME_Ruby;
      *lversion = 0;
      return 0;
    case DW_LANG_Move:
      *lname = DW_LNAME_Move;
      *lversion = 0;
      return 0;
    case DW_LANG_Hylo:
      *lname = DW_LNAME_Hylo;
      *lversion = 0;
      return 0;
    case DW_LANG_V:
      *lname = DW_LNAME_V;
      *lversion = 0;
      return 0;
    case DW_LANG_Algol68:
      *lname = DW_LNAME_Algol68;
      *lversion = 0;
      return 0;
    case DW_LANG_Nim:
      *lname = DW_LNAME_Nim;
      *lversion = 0;
      return 0;
    default:
      __libdw_seterrno (DWARF_E_UNKNOWN_LANGUAGE);
      return -1;
    }
}

static int
language_to_srclang (Dwarf_Word lname, Dwarf_Word lversion, Dwarf_Word *value)
{
  switch (lname)
    {
    case DW_LNAME_Ada:
      if (lversion <= 1983)
	*value = DW_LANG_Ada83;
      else if (lversion <= 1995)
	*value = DW_LANG_Ada95;
      else if (lversion <= 2005)
	*value = DW_LANG_Ada2005;
      else
	*value = DW_LANG_Ada2012;
      return 0;
    case DW_LNAME_BLISS:
      *value = DW_LANG_BLISS;
      return 0;
    case DW_LNAME_C:
      if (lversion == 0)
	*value = DW_LANG_C;
      else if (lversion <= 198912)
	*value = DW_LANG_C89;
      else if (lversion <= 199901)
	*value = DW_LANG_C99;
      else if (lversion <= 201112)
	*value = DW_LANG_C11;
      else if (lversion <= 201710)
	*value = DW_LANG_C17;
      else
	*value = DW_LANG_C23;
      return 0;
    case DW_LNAME_C_plus_plus:
      if (lversion <= 199711)
	*value = DW_LANG_C_plus_plus;
      else if (lversion <= 201103)
	*value = DW_LANG_C_plus_plus_11;
      else if (lversion <= 201402)
	*value = DW_LANG_C_plus_plus_14;
      else if (lversion <= 201703)
	*value = DW_LANG_C_plus_plus_17;
      else if (lversion <= 202002)
	*value = DW_LANG_C_plus_plus_20;
      else
	*value = DW_LANG_C_plus_plus_23;
      return 0;
    case DW_LNAME_Cobol:
      if (lversion <= 1974)
	*value = DW_LANG_Cobol74;
      else
	*value = DW_LANG_Cobol85;
      return 0;
    case DW_LNAME_Crystal:
      *value = DW_LANG_Crystal;
      return 0;
    case DW_LNAME_D:
      *value = DW_LANG_D;
      return 0;
    case DW_LNAME_Dylan:
      *value = DW_LANG_Dylan;
      return 0;
    case DW_LNAME_Fortran:
      if (lversion <= 1977)
	*value = DW_LANG_Fortran77;
      else if (lversion <= 1990)
	*value = DW_LANG_Fortran90;
      else if (lversion <= 1995)
	*value = DW_LANG_Fortran95;
      else if (lversion <= 2003)
	*value = DW_LANG_Fortran03;
      else if (lversion <= 2008)
	*value = DW_LANG_Fortran08;
      else if (lversion <= 2018)
	*value = DW_LANG_Fortran18;
      else
	*value = DW_LANG_Fortran23;
      return 0;
    case DW_LNAME_Go:
      *value = DW_LANG_Go;
      return 0;
    case DW_LNAME_Haskell:
      *value = DW_LANG_Haskell;
      return 0;
    case DW_LNAME_Java:
      *value = DW_LANG_Java;
      return 0;
    case DW_LNAME_Julia:
      *value = DW_LANG_Julia;
      return 0;
    case DW_LNAME_Kotlin:
      *value = DW_LANG_Kotlin;
      return 0;
    case DW_LNAME_Modula2:
      *value = DW_LANG_Modula2;
      return 0;
    case DW_LNAME_Modula3:
      *value = DW_LANG_Modula3;
      return 0;
    case DW_LNAME_ObjC:
      *value = DW_LANG_ObjC;
      return 0;
    case DW_LNAME_ObjC_plus_plus:
      *value = DW_LANG_ObjC_plus_plus;
      return 0;
    case DW_LNAME_OCaml:
      *value = DW_LANG_OCaml;
      return 0;
    case DW_LNAME_OpenCL_C:
      *value = DW_LANG_OpenCL;
      return 0;
    case DW_LNAME_Pascal:
      *value = DW_LANG_Pascal83;
      return 0;
    case DW_LNAME_PLI:
      *value = DW_LANG_PLI;
      return 0;
    case DW_LNAME_Python:
      *value = DW_LANG_Python;
      return 0;
    case DW_LNAME_RenderScript:
      *value = DW_LANG_RenderScript;
      return 0;
    case DW_LNAME_Rust:
      *value = DW_LANG_Rust;
      return 0;
    case DW_LNAME_Swift:
      *value = DW_LANG_Swift;
      return 0;
    case DW_LNAME_UPC:
      *value = DW_LANG_UPC;
      return 0;
    case DW_LNAME_Zig:
      *value = DW_LANG_Zig;
      return 0;
    case DW_LNAME_Assembly:
      /* DW_LANG_Assembler is not as good for compatibility.  */
      *value = DW_LANG_Mips_Assembler;
      return 0;
    case DW_LNAME_C_sharp:
      *value = DW_LANG_C_sharp;
      return 0;
    case DW_LNAME_Mojo:
      *value = DW_LANG_Mojo;
      return 0;
    case DW_LNAME_GLSL:
      *value = DW_LANG_GLSL;
      return 0;
    case DW_LNAME_GLSL_ES:
      *value = DW_LANG_GLSL_ES;
      return 0;
    case DW_LNAME_HLSL:
      *value = DW_LANG_HLSL;
      return 0;
    case DW_LNAME_OpenCL_CPP:
      *value = DW_LANG_OpenCL_CPP;
      return 0;
    case DW_LNAME_CPP_for_OpenCL:
      *value = DW_LANG_CPP_for_OpenCL;
      return 0;
    case DW_LNAME_SYCL:
      *value = DW_LANG_SYCL;
      return 0;
    case DW_LNAME_Ruby:
      *value = DW_LANG_Ruby;
      return 0;
    case DW_LNAME_Move:
      *value = DW_LANG_Move;
      return 0;
    case DW_LNAME_Hylo:
      *value = DW_LANG_Hylo;
      return 0;
    case DW_LNAME_HIP:
      *value = DW_LANG_HIP;
      return 0;
    case DW_LNAME_Odin:
      *value = DW_LANG_Odin;
      return 0;
    case DW_LNAME_P4:
      *value = DW_LANG_P4;
      return 0;
    case DW_LNAME_Metal:
      *value = DW_LANG_Metal;
      return 0;
    case DW_LNAME_V:
      *value = DW_LANG_V;
      return 0;
    case DW_LNAME_Algol68:
      *value = DW_LANG_Algol68;
      return 0;
    case DW_LNAME_Nim:
      *value = DW_LANG_Nim;
      return 0;
    default:
      __libdw_seterrno (DWARF_E_UNKNOWN_LANGUAGE);
      return -1;
    }
}

NEW_VERSION (dwarf_srclang, ELFUTILS_0.143)
int
dwarf_srclang (Dwarf_Die *die)
{
  Dwarf_Attribute attr_mem;
  Dwarf_Word value;

  int res = INTUSE(dwarf_formudata) (INTUSE(dwarf_attr_integrate)
				     (die, DW_AT_language, &attr_mem),
				     &value) == 0 ? (int) value : -1;
  if (res == -1)
    {
      res = INTUSE(dwarf_formudata) (INTUSE(dwarf_attr_integrate)
				     (die, DW_AT_language_name, &attr_mem),
				     &value);
      if (res == 0)
	{
	  Dwarf_Word lname = value;
	  Dwarf_Word lversion;
	  res = INTUSE(dwarf_formudata) (INTUSE(dwarf_attr_integrate)
					 (die, DW_AT_language_version,
					  &attr_mem), &value);
	  lversion = (res == 0) ? value : 0;
	  res = language_to_srclang (lname, lversion, &value);
	  if (res == 0)
	    res = (int) value;
	}
    }

  return res;
}
NEW_INTDEF (dwarf_srclang)
OLD_VERSION (dwarf_srclang, ELFUTILS_0.122)

int
dwarf_language (Dwarf_Die *cudie, Dwarf_Word *lname, Dwarf_Word *lversion)
{
  Dwarf_Attribute attr;
  Dwarf_Word val;

  int res = INTUSE(dwarf_formudata) (INTUSE(dwarf_attr_integrate)
				     (cudie, DW_AT_language_name, &attr),
				     &val);
  if (res == 0)
    {
      *lname = val;
      if (lversion != NULL)
	{
	  /* We like to get the version, but given we already have the
	     lang, we will ignore errors here and just return zero as
	     version.  */
	  res = INTUSE(dwarf_formudata) (INTUSE(dwarf_attr_integrate)
					 (cudie, DW_AT_language_version,
					  &attr), &val);
	  *lversion = (res == 0) ? val : 0;
	}
    }
  else
    {
      /* Try the get the old style pre DWARF6 DW_AT_LANG and translate
	 that to the new language name/version style.  */
      res = INTUSE(dwarf_formudata) (INTUSE(dwarf_attr_integrate)
				     (cudie, DW_AT_language, &attr), &val);
      if (res == 0)
	{
	  Dwarf_Word version;
	  res = srclang_to_language (val, lname, (lversion == NULL
						  ? &version : lversion));
	}
    }

  return res;
}
INTDEF (dwarf_language)

#ifdef MAIN_CHECK
#include "known-dwarf.h"
#include <inttypes.h>
#include <stdio.h>

void
test_lang (const char *name, Dwarf_Word lang)
{
  printf ("Testing %s: 0x%" PRIx64 "\n", name, lang);

  Dwarf_Word lname;
  Dwarf_Word lversion;
  int res = srclang_to_language (lang, &lname, &lversion);
  if (res != 0)
    {
      printf ("srclang_to_language failed (%d) for %s\n", res, name);
      exit (-1);
    }

  Dwarf_Word rlang;
  res = language_to_srclang (lname, lversion, &rlang);
  if (res != 0)
    {
      printf ("language_to_srclang (%" PRId64 ", %" PRId64 ") failed (%d)\n",
	      lname, lversion, res);
      exit (-1);
    }

  /* Most langs should roundtrip, but there are some exceptions.  */
  switch (lang)
    {
    case DW_LANG_Assembly:
      if (rlang != DW_LANG_Mips_Assembler)
	{
	  printf ("For compatibility Assembly should go to Mips_Assembler\n");
	  exit (-1);
	}
      break;
    case DW_LANG_C_plus_plus_03:
      if (rlang != DW_LANG_C_plus_plus)
	{
	  printf ("For c++03 doesn't exist it is just c++\n");
	  exit (-1);
	}
      break;
    default:
      if (lang != rlang)
	{
	  printf ("going from srclang to lang and back gives different name "
		  "for %s (%" PRId64 " != %" PRId64 ")\n", name, lang, rlang);
	  exit (-1);
	}
    }
}

int
main (void)
{
  /* Test all known language codes.  */
#define DWARF_ONE_KNOWN_DW_LANG(NAME, CODE) test_lang (#NAME, CODE);
  DWARF_ALL_KNOWN_DW_LANG
#undef DWARF_ONE_KNOWN_DW_LANG
  return 0;
}
#endif
