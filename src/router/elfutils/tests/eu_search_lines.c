/* Test program for eu_search_lines.
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
#include <fcntl.h>
#include <inttypes.h>
#include <libelf.h>
#include ELFUTILS_HEADER(dw)
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

static Dwarf *dbg;

static void *thread_work (void *arg)
{
  int *res = (int *) arg;

  *res = 0;
  Dwarf_Off cuoff = 0;
  Dwarf_Off old_cuoff = 0;
  size_t hsize;
  Dwarf_Off ao;
  uint8_t asz;
  uint8_t osz;
  while (dwarf_nextcu (dbg, cuoff, &cuoff, &hsize, &ao, &asz, &osz) == 0)
    {
      printf ("cuhl = %zu, o = %llu, asz = %hhu, osz = %hhu, ncu = %llu\n",
	     hsize, (unsigned long long int) ao,
	     asz, osz, (unsigned long long int) cuoff);

      // Get the DIE for the CU.
      Dwarf_Die die;
      if (dwarf_offdie (dbg, old_cuoff + hsize, &die) == NULL)
	{
	  printf ("cannot get CU die\n");
	  *res = 1;
	  break;
	}

      old_cuoff = cuoff;

      Dwarf_Lines *lb;
      size_t nlb;
      if (dwarf_getsrclines (&die, &lb, &nlb) != 0)
	{
	  printf ("cannot get lines\n");
	  *res = 1;
	  break;
	}

      printf (" %zu lines\n", nlb);

      for (size_t i = 0; i < nlb; ++i)
	{
	  Dwarf_Line *l = dwarf_onesrcline (lb, i);
	  if (l == NULL)
	    {
	      printf ("cannot get individual line\n");
	      *res = 1;
	      break;
	    }

	  Dwarf_Addr addr;
	  if (dwarf_lineaddr (l, &addr) != 0)
	    addr = 0;

	  const char *file = dwarf_linesrc (l, NULL, NULL);
	  int line;

	  if (dwarf_lineno (l, &line) != 0)
	    line = 0;

	  printf ("%" PRIx64 ": %s:%d:", (uint64_t) addr, file ? : "???", line);

	  // Getting the file path through the Dwarf_Files should
	  // result in the same path.
	  Dwarf_Files *files;
	  size_t idx;
	  if (dwarf_line_file (l, &files, &idx) != 0)
	    {
	      printf ("cannot get file from line (%zd): %s\n",
		      i, dwarf_errmsg (-1));
	      *res = 1;
	      break;
	    }

	  const char *path = dwarf_filesrc (files, idx, NULL, NULL);
	  if ((path == NULL && file != NULL) ||
	      (path != NULL && file == NULL) ||
	      (strcmp (file, path) != 0))
	    {
	      printf ("line %zd srcline (%s) != file srcline (%s)\n",
		      i, file ? : "???", path ? : "???");
	      *res = 1;
	      break;
	    }

	  int column;

	  if (dwarf_linecol (l, &column) != 0)
	    column = 0;
	  if (column >= 0)
	    printf ("%d:", column);

	  bool is_stmt;
	  if (dwarf_linebeginstatement (l, &is_stmt) != 0)
	    is_stmt = false;
	  bool end_sequence;
	  if (dwarf_lineendsequence (l, &end_sequence) != 0)
	    end_sequence = false;
	  bool basic_block;
	  if (dwarf_lineblock (l, &basic_block) != 0)
	    basic_block = false;
	  bool prologue_end;
	  if (dwarf_lineprologueend (l, &prologue_end) != 0)
	    prologue_end = false;
	  bool epilogue_begin;
	  if (dwarf_lineepiloguebegin (l, &epilogue_begin) != 0)
	    epilogue_begin = false;

	  printf (" is_stmt:%s, end_seq:%s, bb:%s, prologue:%s, epilogue:%s\n",
		  is_stmt ? "yes" : "no", end_sequence ? "yes" : "no",
		  basic_block ? "yes" : "no", prologue_end ? "yes" : "no",
		  epilogue_begin ? "yes" : "no");
	}
    }

  return NULL;
}

int main (int argc, char *argv[])
{
  if (argc != 2)
    {
      printf ("Usage: %s <filename>\n", argv[0]);
      return 1;
    }

  char *name = argv[1];
  int fd = open (name, O_RDONLY);
  if (fd < 0)
    {
      printf ("%s not usable: %s\n", name, dwarf_errmsg (-1));
      return 1;
    }

  dbg = dwarf_begin (fd, DWARF_C_READ);
  if (dbg == NULL)
    {
      printf ("%s not usable: %s\n", name, dwarf_errmsg (-1));
      close (fd);
      return 1;
    }

  printf ("checking %s\n", name);

  int num_threads = 4;
  pthread_t threads[num_threads];
  int thread_results[num_threads];

  for (int i = 0; i < num_threads; ++i)
    {
      if (pthread_create (&threads[i], NULL, thread_work,
			  &thread_results[i]) != 0)
	{
	  perror ("Failed to create thread");

	  for (int j = 0; j < i; j++)
	    pthread_cancel (threads[j]);

	  dwarf_end (dbg);
	  close (fd);
	  return 1;
	}
    }

  int res = 0;
  for (int i = 0; i < num_threads; ++i)
    {
      if (pthread_join (threads[i], NULL) != 0)
	{
	  perror ("Failed to join thread");
	  res = 1;
	  break;
	}

      res |= thread_results[i];
    }

  dwarf_end (dbg);
  close (fd);

  return res;
}
