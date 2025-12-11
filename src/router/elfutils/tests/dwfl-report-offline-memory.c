/* Test program for dwfl_report_offline_memory.
   Copyright (C) 2022 Google LLC
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

#include <config.h>

#include <assert.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <locale.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include ELFUTILS_HEADER(dwfl)
#include ELFUTILS_HEADER(elf)
#include <gelf.h>


static const Dwfl_Callbacks offline_callbacks =
  {
    .find_debuginfo = INTUSE(dwfl_standard_find_debuginfo),
    .section_address = INTUSE(dwfl_offline_section_address),
  };

static int
count_modules (Dwfl_Module *mod __attribute__ ((unused)),
	       void **userdata __attribute__ ((unused)),
	       const char *name __attribute__ ((unused)),
	       Dwarf_Addr base __attribute__ ((unused)), void *arg)
{
  unsigned long long *counter = arg;
  ++(*counter);
  return DWARF_CB_OK;
}

static int
count_sections (Elf *elf)
{
  int result = 0;
  Elf_Scn *section = NULL;
  GElf_Shdr header;
  while ((section = elf_nextscn (elf, section)) != NULL)
    {
      assert (gelf_getshdr (section, &header) != NULL);
      result += 1;
    }
  return result;
}

int
main (int argc, char **argv)
{
  /* We use no threads here which can interfere with handling a stream.  */
  (void) __fsetlocking (stdout, FSETLOCKING_BYCALLER);

  /* Set locale.  */
  (void) setlocale (LC_ALL, "");

  if (argc != 4)
    error (-1, 0,
	   "usage: dwfl_report_offline_memory [filename] "
	   "[expected number of modules] "
	   "[expected number of sections]");

  const char *fname = argv[1];
  int fd = open (fname, O_RDONLY);
  if (fd < 0)
    error (-1, 0, "can't open file %s: %s", fname, strerror (errno));
  off_t size = lseek (fd, 0, SEEK_END);
  if (size < 0)
    error (-1, 0, "can't lseek file %s: %s", fname, strerror (errno));
  lseek (fd, 0, SEEK_SET);
  char *data = malloc (size);
  if (data == NULL)
    error (-1, 0, "can't malloc: %s", strerror (errno));
  ssize_t bytes_read = read (fd, data, size);
  assert (bytes_read == size);
  close (fd);

  Dwfl *dwfl = dwfl_begin (&offline_callbacks);
  assert (dwfl != NULL);

  Dwfl_Module *mod;

  /* Check error handling by suppling zero data */
  mod = dwfl_report_offline_memory(dwfl, argv[1], argv[1], NULL, 0);
  assert(mod == NULL);

  mod = dwfl_report_offline_memory(dwfl, argv[1], argv[1], data, size);
  assert (mod != NULL);
  dwfl_report_end (dwfl, NULL, NULL);

  unsigned long long number_of_modules = 0;
  ptrdiff_t offset =
      dwfl_getmodules (dwfl, &count_modules, &number_of_modules, 0);
  if (offset < 0)
    error (1, 0, "dwfl_getmodules: %s", dwfl_errmsg (-1));
  assert (offset == 0);

  char *endptr;
  unsigned long long expected_number_of_modules =
      strtoull (argv[2], &endptr, 0);
  assert (endptr && !*endptr);
  assert (number_of_modules == expected_number_of_modules);

  GElf_Addr loadbase = 0;
  Elf *elf = dwfl_module_getelf (mod, &loadbase);
  int number_of_sections = count_sections (elf);
  int expected_number_of_sections = atoi (argv[3]);
  assert (number_of_sections == expected_number_of_sections);

  dwfl_end (dwfl);
  free (data);

  return 0;
}
