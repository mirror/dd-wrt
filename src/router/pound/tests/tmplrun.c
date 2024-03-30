/* This file is part of pound testsuite.
 * Copyright (C) 2023-2024 Sergey Poznyakoff
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

#include "pound.h"
#include <assert.h>
#include "json.h"

static size_t
line_start (char *text, size_t n)
{
  while (n > 0 && text[n-1] != '\n')
    n--;
  return n;
}

static size_t
line_no (char *text, size_t n)
{
  size_t i;
  
  for (i = 1;;i++)
    {
      n = line_start (text, n);
      if (n == 0)
	break;
      n--;
    }
  return i;
}

void
xnomem (void)
{
  fprintf (stderr, "%s\n", "out of memory");
  exit (1);
}

// Usage: tmpl file
int
main (int argc, char **argv)
{
  FILE *fp;
  TEMPLATE tmpl;
  size_t end;
  char buf[MAXBUF];
  int rc;
  size_t i;
  
  set_progname (argv[0]);
  
  assert (argc >= 2);
  if ((fp = fopen (argv[1], "r")) == NULL)
    {
      perror (argv[1]);
      exit (1);
    }

  rc = fread (buf, 1, sizeof (buf), fp);
  if (rc == -1)
    {
      perror ("read template");
      exit (1);
    }
  if (rc == sizeof (buf))
    {
      fprintf (stderr, "template file too big\n");
      exit (1);
    }
  else if (rc == 0)
    {
      fprintf (stderr, "template file empty\n");
      exit (1);
    }
  buf[rc] = 0;
  
  rc = template_parse (buf, &tmpl, &end);
    
  if (rc != TMPL_ERR_OK)
    {
      size_t ls = line_start (buf, end);
      size_t ln = line_no (buf, end);
      
      fprintf (stderr, "%s:%zu:%zu: %s\n", argv[1], ln, end - ls,
	       template_strerror (rc));
      exit (1);
    }

  for (i = 2; i < argc; i++)
    {
      struct json_value *jv;
      char *endp;
      
      rc = json_parse_string (argv[i], &jv, &endp);
      if (rc)
	{
	  printf ("arg %zu: error: %s, near %s\n", i, json_strerror (rc), endp);
	  continue;
	}

      template_run (tmpl, jv, stdout);
      json_value_free (jv);
    }

  template_free (tmpl);
  
  return 0;
}
