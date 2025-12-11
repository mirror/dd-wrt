/* Get the section index of the extended section index table.
   Copyright (C) 2007 Red Hat, Inc.
   Copyright (C) 2025 Mark J. Wielaard <mark@klomp.org>
   This file is part of elfutils.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2007.

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

#include "libelfP.h"


int
elf_scnshndx (Elf_Scn *scn)
{
  size_t scnndx;
  GElf_Shdr shdr_mem;
  GElf_Shdr *shdr;
  Elf *elf;
  Elf_Scn *nscn;

  if (scn == NULL)
    return -1;

  scnndx = scn->index;
  elf = scn->elf;

  shdr = gelf_getshdr (scn, &shdr_mem);
  if (shdr == NULL)
    return -1;

  /* Only SYMTAB sections can have a SHNDX section.  */
  if (shdr->sh_type != SHT_SYMTAB)
    return 0;

  /* By convention the SHT_SYMTAB_SHNDX section is right after the the
     SHT_SYMTAB section, so start there.  */
  nscn = scn;
  while ((nscn = elf_nextscn (elf, nscn)) != NULL)
    {
      shdr = gelf_getshdr (nscn, &shdr_mem);
      if (shdr == NULL)
	return -1;

      if (shdr->sh_type == SHT_SYMTAB_SHNDX && shdr->sh_link == scnndx)
	return nscn->index;
    }

  /* OK, not found, start from the top.  */
  nscn = NULL;
  while ((nscn = elf_nextscn (elf, nscn)) != NULL
	 && nscn->index != scnndx)
    {
      shdr = gelf_getshdr (nscn, &shdr_mem);
      if (shdr == NULL)
	return -1;

      if (shdr->sh_type == SHT_SYMTAB_SHNDX && shdr->sh_link == scnndx)
	return nscn->index;
    }

  /* No shndx found, but no errors.  */
  return 0;
}
INTDEF(elf_scnshndx)
