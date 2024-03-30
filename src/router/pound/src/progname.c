/* This file is part of pound.
 * Copyright (C) 2022-2024 Sergey Poznyakoff
 *
 * Pound is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pound is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <pound.h>
#include <string.h>

char const *progname;

void
set_progname (char const *arg)
{
  if ((progname = strrchr (arg, '/')) == NULL)
    progname = arg;
  else
    progname++;
}

static int copyright_year = 2023;

#define VALUE_COLUMN 28

static void
print_string_values (struct string_value *values, FILE *fp)
{
  struct string_value *p;
  char const *val;

  for (p = values; p->kw; p++)
    {
      int n = fprintf (fp, "%s:", p->kw);
      if (n < VALUE_COLUMN)
	fprintf (fp, "%*s", VALUE_COLUMN-n, "");

      switch (p->type)
	{
	case STRING_CONSTANT:
	  val = p->data.s_const;
	  break;

	case STRING_INT:
	  fprintf (fp, "%d\n", p->data.s_int);
	  continue;

	case STRING_VARIABLE:
	  val = *p->data.s_var;
	  break;

	case STRING_FUNCTION:
	  val = p->data.s_func ();
	  break;

	case STRING_PRINTER:
	  p->data.s_print (fp);
	  fputc ('\n', fp);
	  continue;
	}

      fprintf (fp, "%s\n", val);
    }
}

void
print_version (struct string_value *settings)
{
  printf ("%s (%s) %s\n", progname, PACKAGE_NAME, PACKAGE_VERSION);
  printf ("Copyright (C) 2002-2010 Apsis GmbH\n");
  printf ("Copyright (C) 2018-%d Sergey Poznyakoff\n", copyright_year);
  printf ("\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n\
");
  if (settings)
    {
      printf ("\nBuilt-in defaults:\n\n");
      print_string_values (settings, stdout);
    }
}

