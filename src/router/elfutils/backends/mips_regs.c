/* Register names and numbers for mips DWARF.
   Copyright (C) 2006 Red Hat, Inc.
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

#define BACKEND mips_
#include "libebl_CPU.h"
#include <system.h>
ssize_t
mips_register_info (Ebl *ebl __attribute__ ((unused)),
		      int regno, char *name, size_t namelen,
		      const char **prefix, const char **setname,
		      int *bits, int *type)
{
  if (name == NULL)
    return 72;

  if (regno < 0 || regno > 71 || namelen < 4)
    return -1;

  *prefix = "$";
  if (regno < 38)
    {
      *setname = "integer";
      *type = DW_ATE_signed;
      *bits = 32;
    }
  else
    {
      *setname = "FPU";
      *type = DW_ATE_float;
      *bits = 64;
    }

  if (regno < 32)
    {
      if (regno < 10)
	{
	  name[0] = regno + '0';
	  namelen = 1;
	}
      else
	{
	  name[0] = (regno / 10) + '0';
	  name[1] = (regno % 10) + '0';
	  namelen = 2;
	}
      if (regno == 28 || regno == 29 || regno == 31)
	*type = DW_ATE_address;
    }
  else if (regno == 32)
    {
      return stpcpy (name, "lo") + 1 - name;
    }
  else if (regno == 33)
    {
      return stpcpy (name, "hi") + 1 - name;
    }
  else if (regno == 34)
    {
      return stpcpy (name, "pc") + 1 - name;
    }
  else if (regno == 35)
    {
      *type = DW_ATE_address;
      return stpcpy (name, "bad") + 1 - name;
    }
  else if (regno == 36)
    {
      return stpcpy (name, "sr") + 1 - name;
    }
  else if (regno == 37)
    {
      *type = DW_ATE_address;
      return stpcpy (name, "cause") + 1 - name;
    }
  else if (regno < 70)
    {
      name[0] = 'f';
      if (regno < 38 + 10)
      {
	name[1] = (regno - 38) + '0';
	namelen = 2;
      }
      else
      {
	name[1] = (regno - 38) / 10 + '0';
	name[2] = (regno - 38) % 10 + '0';
	namelen = 3;
      }
    }
  else if (regno == 70)
    {
      return stpcpy (name, "fsr") + 1 - name;
    }
  else if (regno == 71)
    {
      return stpcpy (name, "fir") + 1 - name;
    }

  name[namelen++] = '\0';
  return namelen;
}
