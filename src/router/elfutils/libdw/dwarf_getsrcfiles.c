/* Return source file information of CU.
   Copyright (C) 2004, 2005, 2013, 2015, 2018 Red Hat, Inc.
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
#include "libdwP.h"


int
dwarf_getsrcfiles (Dwarf_Die *cudie, Dwarf_Files **files, size_t *nfiles)
{
  if (cudie == NULL)
    return -1;
  if (! is_cudie (cudie))
    {
      __libdw_seterrno (DWARF_E_NOT_CUDIE);
      return -1;
    }

  int res = -1;
  struct Dwarf_CU *const cu = cudie->cu;
  mutex_lock (cudie->cu->src_lock);

  /* Get the information if it is not already known.  */
  if (cu->files == NULL)
    {
      /* For split units there might be a simple file table (without lines).
	 If not, use the one from the skeleton.  */
      if (cu->unit_type == DW_UT_split_compile
	  || cu->unit_type == DW_UT_split_type)
	{
	  /* We tried, assume we fail...  */
	  cu->files = (void *) -1;

	  /* See if there is a .debug_line section, for split CUs
	     the table is at offset zero.  */
	  if (cu->dbg->sectiondata[IDX_debug_line] != NULL)
	    {
	      Dwarf_Off dwp_off;
	      if (INTUSE(dwarf_cu_dwp_section_info) (cu, DW_SECT_LINE,
						     &dwp_off, NULL) == 0)
		{
		  /* We are only interested in the files, the lines will
		     always come from the skeleton.  */
		  res = __libdw_getsrcfiles (cu->dbg, dwp_off,
					     __libdw_getcompdir (cudie),
					     cu->address_size, &cu->files);
		}
	    }
	  else
	    {
	      Dwarf_CU *skel = __libdw_find_split_unit (cu);
	      if (skel != NULL)
		{
		  Dwarf_Die skeldie = CUDIE (skel);
		  res = INTUSE(dwarf_getsrcfiles) (&skeldie, files, nfiles);
		  cu->files = skel->files;
		}
	    }
	}
      else
	{
	  /* The die must have a statement list associated.  */
	  Dwarf_Attribute stmt_list_mem;
	  Dwarf_Attribute *stmt_list = INTUSE(dwarf_attr) (cudie, DW_AT_stmt_list,
							   &stmt_list_mem);

	  Dwarf_Off debug_line_offset;
	  if (__libdw_formptr (stmt_list, IDX_debug_line, DWARF_E_NO_DEBUG_LINE,
			       NULL, &debug_line_offset) == NULL)
	    {
	      mutex_unlock (cudie->cu->src_lock);
	      return -1;
	    }

	  res = __libdw_getsrcfiles (cu->dbg, debug_line_offset,
				     __libdw_getcompdir (cudie),
				     cu->address_size, &cu->files);
	}
    }
  else if (cu->files != (void *) -1l)
    /* We already have the information.  */
    res = 0;

  if (likely (res == 0))
    {
      assert (cu->files != NULL && cu->files != (void *) -1l);
      *files = cu->files;
      if (nfiles != NULL)
	*nfiles = cu->files->nfiles;
    }

  mutex_unlock (cudie->cu->src_lock);
  return res;
}
INTDEF (dwarf_getsrcfiles)
