/* Test program for eu_search_macros
   Copyright (C) 2023 Red Hat, Inc.
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
   along with this program.  If not, see<http://www.gnu.org/licenses/>.  */

#include <config.h>
#include ELFUTILS_HEADER(dw)
#include <dwarf.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdatomic.h>
#include <pthread.h>
#include <unistd.h>
#include <error.h>

static void *thread_work (void * arg);
static int mac (Dwarf_Macro *macro, void *arg);
static int include (Dwarf_Off macoff, ptrdiff_t token);

static Dwarf *dbg;
static ptrdiff_t cuoff;

static void *thread_work (void *arg __attribute__ ((unused)))
{
  Dwarf_Die cudie_mem, *cudie = dwarf_offdie (dbg, cuoff, &cudie_mem);
  for (ptrdiff_t off = DWARF_GETMACROS_START;
       (off = dwarf_getmacros (cudie, mac, dbg, off));)
    {
      if (off != 0)
	{
	  error (EXIT_FAILURE, 0, "dwarf_getmacros: %s",
		 dwarf_errmsg (dwarf_errno ()));
	}
    }

  return NULL;
}

static int include (Dwarf_Off macoff, ptrdiff_t token)
{
  while ((token = dwarf_getmacros_off (dbg, macoff, mac, dbg, token)) != 0)
    {
      if (token == -1)
	{
	  puts (dwarf_errmsg (dwarf_errno ()));
	  return 1;
	}
    }
  return 0;
}

static int
mac (Dwarf_Macro *macro, void *arg __attribute__ ((unused)))
{
  unsigned int opcode;
  dwarf_macro_opcode (macro, &opcode);
  switch (opcode)
    {
    case DW_MACRO_import:
      {
	Dwarf_Attribute at;
	if (dwarf_macro_param (macro, 0, &at) != 0)
	  return DWARF_CB_ABORT;

	Dwarf_Word w;
	if (dwarf_formudata (&at, &w) != 0)
	  return DWARF_CB_ABORT;
	if (include (w, DWARF_GETMACROS_START) != 0)
	  return DWARF_CB_ABORT;
	break;
      }

    case DW_MACRO_start_file:
      {
	Dwarf_Files *files;
	size_t nfiles;
	if (dwarf_macro_getsrcfiles (dbg, macro, &files, &nfiles) < 0)
	  {
	    printf ("dwarf_macro_getsrcfiles: %s\n",
		   dwarf_errmsg (dwarf_errno ()));
	    return DWARF_CB_ABORT;
	  }

	Dwarf_Word w = 0;
	if (dwarf_macro_param2 (macro, &w, NULL) != 0)
	  return DWARF_CB_ABORT;

        if (dwarf_filesrc (files, (size_t) w, NULL, NULL) == NULL)
	  return DWARF_CB_ABORT;
	break;
      }

    case DW_MACINFO_define:
    case DW_MACRO_define_strp:
      {
	const char *value;
	if (dwarf_macro_param2(macro, NULL, &value) != 0)
	  return DWARF_CB_ABORT;
	break;
      }

    case DW_MACINFO_undef:
    case DW_MACRO_undef_strp:
      break;

    default:
      {
	size_t paramcnt;
	if (dwarf_macro_getparamcnt (macro, &paramcnt) != 0)
	  return DWARF_CB_ABORT;
	break;
      }
    }

  return DWARF_CB_OK;
}

int main (int argc, char *argv[])
{
  assert (argc == 3);
  const char *name = argv[1];

  int fd = open (name, O_RDONLY);
  dbg = dwarf_begin (fd, DWARF_C_READ);
  cuoff = strtol (argv[2], NULL, 0);

  int num_threads = 4;
  pthread_t threads[num_threads];

  for (int i = 0; i < num_threads; i++)
    {
      if (pthread_create (&threads[i], NULL, thread_work, NULL) != 0)
	{
	  perror ("Failed to create thread");

	  for (int j = 0; j < i; j++)
	    pthread_cancel (threads[j]);

	  dwarf_end (dbg);
	  close (fd);
	  return 1;
	}
    }

  for (int i = 0; i < num_threads; i++)
    {
      if (pthread_join (threads[i], NULL) != 0)
	{
	  perror ("Failed to join thread");
	}
    }

  dwarf_end (dbg);
  close (fd);

  return 0;
}
