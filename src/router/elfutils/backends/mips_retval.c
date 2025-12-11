/* Function return value location for Linux/mips ABI.
   Copyright (C) 2005 Red Hat, Inc.
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

#include <assert.h>
#include <dwarf.h>
#include <string.h>
#include <elf.h>
#include <stdio.h>

#define BACKEND mips_
#include "libebl_CPU.h"
#include "libdwP.h"
#include <stdio.h>

/* $v0 or pair $v0, $v1 */
static const Dwarf_Op loc_intreg_o32[] =
  {
    { .atom = DW_OP_reg2 }, { .atom = DW_OP_piece, .number = 4 },
    { .atom = DW_OP_reg3 }, { .atom = DW_OP_piece, .number = 4 },
  };

static const Dwarf_Op loc_intreg[] =
  {
    { .atom = DW_OP_reg2 }, { .atom = DW_OP_piece, .number = 8 },
    { .atom = DW_OP_reg3 }, { .atom = DW_OP_piece, .number = 8 },
  };
#define nloc_intreg	1
#define nloc_intregpair	4

/* $f0 (float), or pair $f0, $f1 (double).
 * f2/f3 are used for COMPLEX (= 2 doubles) returns in Fortran */
static const Dwarf_Op loc_fpreg_o32[] =
  {
    { .atom = DW_OP_regx, .number = 32 }, { .atom = DW_OP_piece, .number = 4 },
    { .atom = DW_OP_regx, .number = 33 }, { .atom = DW_OP_piece, .number = 4 },
    { .atom = DW_OP_regx, .number = 34 }, { .atom = DW_OP_piece, .number = 4 },
    { .atom = DW_OP_regx, .number = 35 }, { .atom = DW_OP_piece, .number = 4 },
  };

/* $f0, or pair $f0, $f2.  */
static const Dwarf_Op loc_fpreg[] =
  {
    { .atom = DW_OP_regx, .number = 32 }, { .atom = DW_OP_piece, .number = 8 },
    { .atom = DW_OP_regx, .number = 34 }, { .atom = DW_OP_piece, .number = 8 },
  };
#define nloc_fpreg  1
#define nloc_fpregpair 4
#define nloc_fpregquad 8

/* The return value is a structure and is actually stored in stack space
   passed in a hidden argument by the caller.  But, the compiler
   helpfully returns the address of that space in $v0.  */
static const Dwarf_Op loc_aggregate[] =
  {
    { .atom = DW_OP_breg2, .number = 0 }
  };
#define nloc_aggregate 1

int
mips_return_value_location (Dwarf_Die *functypedie, const Dwarf_Op **locp)
{
  unsigned int regsize = (gelf_getclass (functypedie->cu->dbg->elf) == ELFCLASS32 ) ? 4 : 8;
  if (!regsize)
    return -2;

  /* Start with the function's type, and get the DW_AT_type attribute,
     which is the type of the return value.  */

  Dwarf_Attribute attr_mem;
  Dwarf_Attribute *attr = dwarf_attr_integrate (functypedie, DW_AT_type, &attr_mem);
  if (attr == NULL)
    /* The function has no return value, like a `void' function in C.  */
    return 0;

  Dwarf_Die die_mem;
  Dwarf_Die *typedie = dwarf_formref_die (attr, &die_mem);
  int tag = dwarf_tag (typedie);

  /* Follow typedefs and qualifiers to get to the actual type.  */
  while (tag == DW_TAG_typedef
	 || tag == DW_TAG_const_type || tag == DW_TAG_volatile_type
	 || tag == DW_TAG_restrict_type)
    {
      attr = dwarf_attr_integrate (typedie, DW_AT_type, &attr_mem);
      typedie = dwarf_formref_die (attr, &die_mem);
      tag = dwarf_tag (typedie);
    }

  switch (tag)
    {
    case -1:
      return -1;

    case DW_TAG_subrange_type:
      if (! dwarf_hasattr_integrate (typedie, DW_AT_byte_size))
	{
	  attr = dwarf_attr_integrate (typedie, DW_AT_type, &attr_mem);
	  typedie = dwarf_formref_die (attr, &die_mem);
	  tag = dwarf_tag (typedie);
	}
      /* Fall through.  */
      FALLTHROUGH;

    case DW_TAG_base_type:
    case DW_TAG_enumeration_type:
    CASE_POINTER:
      {
	Dwarf_Word size;
	if (dwarf_formudata (dwarf_attr_integrate (typedie, DW_AT_byte_size,
					 &attr_mem), &size) != 0)
	  {
	    if (dwarf_is_pointer (tag))
	      size = regsize;
	    else
	      return -1;
	  }
	if (tag == DW_TAG_base_type)
	  {
	    Dwarf_Word encoding;
	    if (dwarf_formudata (dwarf_attr_integrate (typedie, DW_AT_encoding,
					     &attr_mem), &encoding) != 0)
	      return -1;

#define ARCH_LOC(loc, regsize) ((regsize) == 4 ? (loc ## _o32) : (loc))

	    if (encoding == DW_ATE_float)
	      {
		*locp = ARCH_LOC(loc_fpreg, regsize);
		if (size <= regsize)
		  return nloc_fpreg;

		if (size <= 2*regsize)
		  return nloc_fpregpair;

		if (size <= 4*regsize)
		  return nloc_fpregquad;

		goto aggregate;
	      }
	  }
	*locp = ARCH_LOC(loc_intreg, regsize);
	if (size <= regsize)
	  return nloc_intreg;
	if (size <= 2*regsize)
	  return nloc_intregpair;

	/* Else fall through. Shouldn't happen though (at least with gcc) */
      }
      FALLTHROUGH;

    case DW_TAG_structure_type:
    case DW_TAG_class_type:
    case DW_TAG_union_type:
    case DW_TAG_array_type:
    aggregate:
      *locp = loc_aggregate;
      return nloc_aggregate;
    case DW_TAG_unspecified_type:
      return 0;
    }

  /* XXX We don't have a good way to return specific errors from ebl calls.
     This value means we do not understand the type, but it is well-formed
     DWARF and might be valid.  */
  return -2;
}
