/* Copyright (C) 2002, 2004, 2005, 2007 Red Hat, Inc.
   This file is part of elfutils.
   Written by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include <fcntl.h>
#include <libelf.h>
#include ELFUTILS_HEADER(dw)
#include <stdio.h>
#include <unistd.h>
#include "../libdw/libdwP.h"

static void
print_dirs_and_files (Dwarf_Files *files, const char *const *dirs,
		      size_t nfiles, size_t ndirs)
{
  if (dirs[0] == NULL)
    puts (" dirs[0] = (null)");
  else
    printf (" dirs[0] = \"%s\"\n", dirs[0]);
  for (size_t i = 1; i < ndirs; ++i)
    printf (" dirs[%zu] = \"%s\"\n", i, dirs[i]);

  for (size_t i = 0; i < nfiles; ++i)
    printf (" file[%zu] = \"%s\"\n", i,
	    dwarf_filesrc (files, i, NULL, NULL));
}


int
main (int argc, char *argv[])
{
  int result = 0;
  int cnt = argc - 1;

  int fd = open (argv[cnt], O_RDONLY);

  Dwarf *dbg = dwarf_begin (fd, DWARF_C_READ);
  if (dbg == NULL)
    {
      printf ("%s not usable\n", argv[cnt]);
      result = 1;
      if (fd != -1)
	close (fd);
      goto out;
    }

  Dwarf_Off o = 0;
  Dwarf_Off ncu;
  size_t cuhl;

  /* Just inspect the first CU.  */
  if (dwarf_nextcu (dbg, o, &ncu, &cuhl, NULL, NULL, NULL) != 0)
    {
      printf ("%s: cannot get CU\n", argv[cnt]);
      result = 1;
      goto out;
    }

  Dwarf_Die die_mem;
  Dwarf_Die *die = dwarf_offdie (dbg, o + cuhl, &die_mem);

  if (die == NULL)
    {
      printf ("%s: cannot get CU die\n", argv[cnt]);
      result = 1;
      goto out;
    }

  Dwarf_Files *files;
  size_t nfiles;

  /* The files from DW_LNE_define_file should not be included
     until dwarf_getsrclines is called.  */
  if (dwarf_getsrcfiles (die, &files, &nfiles) != 0)
    {
      printf ("%s: cannot get files\n", argv[cnt]);
      result = 1;
      goto out;
    }

  if (die->cu->lines != NULL)
    {
      printf ("%s: dwarf_getsrcfiles should not get lines\n", argv[cnt]);
      result = 1;
      goto out;
    }

  const char *const *dirs;
  size_t ndirs;
  if (dwarf_getsrcdirs (files, &dirs, &ndirs) != 0)
    {
      printf ("%s: cannot get include directories\n", argv[cnt]);
      result = 1;
      goto out;
    }

  /* Print file info without files from DW_LNE_define_file.  */
  print_dirs_and_files (files, dirs, nfiles, ndirs);

  Dwarf_Lines *lines;
  size_t nlines;

  /* Reading the line program should add the new files.  */
  if (dwarf_getsrclines (die, &lines, &nlines) != 0)
    {
      printf ("%s: cannot get lines\n", argv[cnt]);
      result = 1;
      goto out;
    }

  Dwarf_Files *updated_files;
  size_t num_updated_files;

  /* Get the new files.  */
  if (dwarf_getsrcfiles (die, &updated_files, &num_updated_files) != 0)
    {
      printf ("%s: cannot get files\n", argv[cnt]);
      result = 1;
      goto out;
    }

  const char *const *updated_dirs;
  size_t num_updated_dirs;

  /* The dirs shouldn't change but verify that getsrcdirs still works.  */
  if (dwarf_getsrcdirs (updated_files, &updated_dirs, &num_updated_dirs) != 0)
    {
      printf ("%s: cannot get include directories\n", argv[cnt]);
      result = 1;
      goto out;
    }

  /* Verify that we didn't invalidate the old file info.  */
  print_dirs_and_files (files, dirs, nfiles, ndirs);

  /* Print all files including those from DW_LNE_define_file.  */
  print_dirs_and_files (updated_files, updated_dirs,
			num_updated_files, num_updated_dirs);

out:
  dwarf_end (dbg);
  close (fd);

  return result;
}
