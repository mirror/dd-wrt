/* Copyright (C) 2025 Red Hat, Inc.
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

#include <assert.h>
#include <fcntl.h>
#include <gelf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <system.h>

#define printf_indent(n, ...)		\
  do {					\
    for (int _i = 0; _i < (n); _i++)	\
      fputs ("    ", stdout);		\
    printf (__VA_ARGS__);		\
  } while (0)

static void
ar_extract (Elf *elf, int indent)
{
  Elf *subelf;
  Elf_Arsym *arsym;
  size_t narsym;
  Elf_Cmd cmd = ELF_C_READ;

  assert (elf_kind (elf) == ELF_K_AR);

  arsym = elf_getarsym (elf, &narsym);
  if (arsym == NULL)
    {
      printf_indent (indent, "Cannot get archive index: %s\n", elf_errmsg (-1));
      exit (1);
    }

  if (narsym != 4)
    {
      printf_indent (indent, "Incorrect number of arsyms\n");
      exit (1);
    }

  printf_indent (indent, "== symbol names ==\n");

  /* Print symbol names.  Skip the null entry at the end of arsyms.  */
  for (size_t i = 0; i < narsym - 1; ++i)
    printf_indent (indent, "%s\n", arsym[i].as_name);
  putchar ('\n');
  printf_indent (indent, "== headers ==\n");

  /* Print header names and recursively call this function on member
     archives.  */
  while ((subelf = elf_begin (-1, cmd, elf)) != NULL)
    {
      /* The the header for this element.  */
      Elf_Arhdr *arhdr = elf_getarhdr (subelf);

      if (arhdr == NULL)
	{
	  printf_indent (indent, "cannot get arhdr: %s\n", elf_errmsg (-1));
	  exit (1);
	}

      printf_indent (indent, "%s %s\n", arhdr->ar_name, arhdr->ar_rawname);

      if (elf_kind (subelf) == ELF_K_AR)
	ar_extract (subelf, indent + 1);

      cmd = elf_next (subelf);
      elf_end (subelf);
    }

  putchar ('\n');
}


int
main (int argc, char *argv[])
{
  int fd;
  Elf *elf;
  Elf_Cmd cmd;

  if (argc < 2)
    exit (1);

  /* Open the archive.  */
  fd = open (argv[1], O_RDONLY);
  if (fd == -1)
    {
      printf ("Cannot open input file: %m");
      exit (1);
    }

  /* Set the ELF version.  */
  elf_version (EV_CURRENT);

  /* Create an ELF descriptor.  */
  cmd = ELF_C_READ;
  elf = elf_begin (fd, cmd, NULL);

  if (elf == NULL)
    {
      printf ("Cannot create ELF descriptor: %s\n", elf_errmsg (-1));
      exit (1);
    }

  /* If it is no archive punt.  */
  if (elf_kind (elf) != ELF_K_AR)
    {
      printf ("`%s' is no archive\n", argv[1]);
      exit (1);
    }

  ar_extract (elf, 0);
  elf_end (elf);
  close (fd);

  return 0;
}
