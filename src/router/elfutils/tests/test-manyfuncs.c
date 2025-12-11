/* Test program for reading file with 64k+ sections and symbols.
   Copyright (C) 2025 Mark J. Wielaard <mark@klomp.org>
   This file is part of elfutils.

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   elfutils is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "system.h"

#include ELFUTILS_HEADER(elf)
#include <gelf.h>

/* Various fields (Elf_Half) can only contain up to 64k values. So
   they need to be accessed through libelf functions which know where
   the "true" value can be found. Also to get the section associated
   with a symbol will need an extended symbol table entry. Use
   elf_scnshndx to get at it and check we can get both symbol and
   associated section names.  */

static void
check_elf (const char *fname, bool use_mmap)
{
  printf ("\nfname (use_mmap: %d): %s\n", use_mmap, fname);

  int fd = open (fname, O_RDONLY);
  if (fd == -1)
    {
      printf ("cannot open `%s': %s\n", fname, strerror (errno));
      exit (1);
    }

  Elf *elf = elf_begin (fd, use_mmap ? ELF_C_READ_MMAP : ELF_C_READ, NULL);
  if (elf == NULL)
    {
      printf ("cannot create ELF descriptor: %s\n", elf_errmsg (-1));
      exit (1);
    }

  /* How many sections are there?  */
  size_t shnum;
  if (elf_getshdrnum (elf, &shnum) < 0)
    {
      printf ("elf_getshdrstrndx: %s\n", elf_errmsg (-1));
      exit (1);
    }

  if (shnum < SHN_LORESERVE)
    {
      printf ("Not enough section for test, %zd < %d\n",
	      shnum, SHN_LORESERVE);
      exit (1);
    }

  printf ("shnum: %zd\n", shnum);

  /* Get the section that contains the section header names.  Check it
     can be found and it contains the substring 'str' in its own
     name.  */
  size_t shstrndx;
  if (elf_getshdrstrndx (elf, &shstrndx) < 0)
    {
      printf ("elf_getshdrstrndx: %s\n", elf_errmsg (-1));
      exit (1);
    }

  Elf_Scn *scn = elf_getscn (elf, shstrndx);
  if (scn == NULL)
    {
      printf ("cannot get section at index %zd: %s\n", shstrndx,
	      elf_errmsg (-1));
      exit (1);
    }

  GElf_Shdr shdr_mem;
  GElf_Shdr *shdr = gelf_getshdr (scn, &shdr_mem);
  if (shdr == NULL)
    {
      printf ("cannot get header for shstrndx section: %s\n",
	      elf_errmsg (-1));
      exit (1);
    }

  /* Assume the section name contains at least the substring "str".  */
  const char *sname = elf_strptr (elf, shstrndx, shdr->sh_name);
  if (sname == NULL || strstr (sname, "str") == NULL)
    {
      printf ("Bad section name: %s\n", sname);
      exit (1);
    }

  printf ("shstrndx section name: %s\n", sname);

  /* The shstrndx section isn't a SYMTAB section, so elf_scnshndx will
     return zero.  */
  int shndx = elf_scnshndx (scn);
  if (shndx < 0)
    {
      printf ("elf_scnshndx error for shstrndx section: %s\n",
	      elf_errmsg (-1));
      exit (1);
    }
  if (shndx > 0)
    {
      printf ("elf_scnshndx not zero for shstrndx section: %d\n", shndx);
      exit (1);
    }

  /* Find the symtab and symtab_shndx by hand.  */
  size_t symtabndx = 0;
  size_t symtabstrndx = 0;
  size_t symtabsndx = 0;
  size_t scns = 1; /* scn zero is skipped. */
  Elf_Scn *symtabscn = NULL;
  Elf_Scn *xndxscn = NULL;
  scn = NULL;
  while ((scn = elf_nextscn (elf, scn)) != NULL)
    {
      if (scns != elf_ndxscn (scn))
	{
	  printf ("Unexpected elf_ndxscn %zd != %zd\n",
		  scns, elf_ndxscn (scn));
	  exit (1);
	}

      shdr = gelf_getshdr (scn, &shdr_mem);
      if (shdr == NULL)
	{
	  printf ("couldn't get shdr\n");
	  exit (1);
	}

      if (shdr->sh_type == SHT_SYMTAB)
	{
	  symtabndx = elf_ndxscn (scn);
	  symtabstrndx = shdr->sh_link;
	  symtabscn = scn;
	}

      if (shdr->sh_type == SHT_SYMTAB_SHNDX)
	{
	  symtabsndx = elf_ndxscn (scn);
	  xndxscn = scn;
	}

      /* We could break if we have both symtabndx and symtabsndx, but
	 we want to run through all sections to see if we get them
	 all and sanity check shnum.  */
      scns++;
    }

  printf ("scns: %zd\n", scns);
  if (scns != shnum)
    {
      printf ("scns (%zd) != shnum (%zd)\n", scns, shnum);
      exit (1);
    }

  printf ("symtabndx: %zd\n", symtabndx);
  if (symtabndx == 0 || symtabscn == NULL)
    {
      printf ("No SYMTAB\n");
      exit (1);
    }

  printf ("symtabsndx: %zd\n", symtabsndx);
  if (symtabsndx == 0)
    {
      printf ("No SYMTAB_SNDX\n");
      exit (1);
    }

  int scnshndx = elf_scnshndx (symtabscn);
  printf ("scnshndx: %d\n", scnshndx);
  if (scnshndx < 0)
    {
      printf ("elf_scnshndx failed: %s\n", elf_errmsg (-1));
      exit (1);
    }

  if (scnshndx == 0)
    {
      printf ("elf_scnshndx couldn't find scnshndx, returned zero\n");
      exit (1);
    }

  if ((size_t) scnshndx != symtabsndx)
    {
      printf ("elf_scnshndx found wrong scnshndx (%d),"
	      " should have been (%zd)\n", scnshndx, symtabsndx);
      exit (1);
    }

  Elf_Data *symdata = elf_getdata (symtabscn, NULL);
  if (symdata == NULL)
    {
      printf ("Couldn't elf_getdata for symtabscn: %s\n", elf_errmsg (-1));
      exit (1);
    }

  size_t nsyms = symdata->d_size / (gelf_getclass (elf) == ELFCLASS32
				    ? sizeof (Elf32_Sym)
				    : sizeof (Elf64_Sym));

  Elf_Data *xndxdata = elf_getdata (xndxscn, NULL);
  if (xndxdata == NULL)
    {
      printf ("Couldn't elf_getdata for xndxscn: %s\n", elf_errmsg (-1));
      exit (1);
    }

  /* Now for every [yz]ddddd symbol , check that it matches the
     section name (minus the starting dot).  */
  size_t yzsymcnt = 0;
  for (size_t cnt = 0; cnt < nsyms; ++cnt)
    {
      const char *sym_name;
      const char *section_name;
      GElf_Word xndx;
      GElf_Sym sym_mem;
      GElf_Sym *sym = gelf_getsymshndx (symdata, xndxdata, cnt,
					&sym_mem, &xndx);

      if (sym == NULL)
	{
	  printf ("gelf_getsymshndx failed: %s\n", elf_errmsg (-1));
	  exit (1);
	}

      if (sym->st_shndx != SHN_XINDEX)
	xndx = sym->st_shndx;

      sym_name = elf_strptr (elf, symtabstrndx, sym->st_name);
      if (sym_name == NULL)
	{
	  printf ("elf_strptr returned NULL for sym %zd: %s\n",
		  cnt, elf_errmsg (-1));
	  exit (1);
	}

      scn = elf_getscn (elf, xndx);
      if (scn == NULL)
	{
	  printf ("cannot get section at index %d: %s\n", xndx,
		  elf_errmsg (-1));
	  exit (1);
	}

      shdr = gelf_getshdr (scn, &shdr_mem);
      if (shdr == NULL)
	{
	  printf ("cannot get header for sym xndx section: %s\n",
		  elf_errmsg (-1));
	  exit (1);
	}

      section_name = elf_strptr (elf, shstrndx, shdr->sh_name);
      if (section_name == NULL)
	{
	  printf ("elf_strptr returned NULL for section: %s\n",
		  elf_errmsg (-1));
	  exit (1);
	}

      if (GELF_ST_TYPE (sym->st_info) == STT_FUNC
	  && (sym_name[0] == 'y' || sym_name[0] == 'z')
	  && strlen (sym_name) == 6)
	{
	  yzsymcnt++;
	  /* Every [yz]ddddd symbol comes from a section named after
	     the symbol prefixed with '.'.  Except for powerpc ELFv1,
	     where all symbols point into the .opd.  */
	  if ((section_name[0] != '.'
	       || strcmp (sym_name, &section_name[1]) != 0)
	      && strcmp (section_name, ".opd") != 0)
	    {
	      printf ("BAD SYM/SECTION %zd sym: %s, section: %s\n",
		      cnt, sym_name, section_name);
	      exit (1);
	    }
	}
    }

#define YZSYMCNT (size_t) (2 * 8 * 8 * 8 * 8 * 8)
  printf ("yzsymcnt: %zd\n", yzsymcnt);
  if (yzsymcnt != YZSYMCNT)
    {
      printf ("Wrong number of yzsymcnts: %zd, should be %zd\n",
	      yzsymcnt, YZSYMCNT);
      exit (1);
    }

  if (elf_end (elf) != 0)
    {
      printf ("failure in elf_end: %s\n", elf_errmsg (-1));
      exit (1);
    }

  close (fd);
}

int
main (int argc, char *argv[] __attribute__ ((unused)))
{
  elf_version (EV_CURRENT);

  if (argc < 2)
    {
      printf ("Need at least one (file name) argument\n");
      exit (1);
    }

  /* Test all file using ELF_C_READ and ELF_C_READ_MMAP.  */
  for (int i = 1; i < argc; i++)
    {
      check_elf (argv[i], false);
      check_elf (argv[i], true);
    }

  return 0;
}
