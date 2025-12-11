/* Return one of the sources lines of a CU.
   Copyright (C) 2024 Red Hat, Inc.
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

#include <sys/stat.h>

#include "libdwflP.h"
#include "libdwP.h"

int
dwfl_set_sysroot (Dwfl *dwfl, const char *sysroot)
{
  if (!sysroot)
    {
      free (dwfl->sysroot);
      dwfl->sysroot = NULL;
      return 0;
    }

  char *r, *s;
  r = realpath (sysroot, NULL);
  if (!r)
    return -1;

  int rc;
  struct stat sb;

  rc = stat (r, &sb);
  if (rc < 0 || !S_ISDIR (sb.st_mode))
    {
      errno = EINVAL;
      return -1;
    }

  rc = asprintf (&s, "%s/", r);
  if (rc < 0)
    {
      errno = ENOMEM;
      return -1;
    }

  free (dwfl->sysroot);
  free (r);

  dwfl->sysroot = s;
  return 0;
}

INTDEF (dwfl_set_sysroot)
