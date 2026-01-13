/*
 * Pound - the reverse-proxy load-balancer
 * Copyright (C) 2023-2025 Sergey Poznyakoff
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
 * along with pound.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "pound.h"
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

struct pcre2_pattern
{
  pcre2_code *code;
  size_t nsub;
  char *errmsg;
  size_t erroff;
};

static int
pcre2_pattern_init (GENPAT pat)
{
  pat->data = xzalloc (sizeof (struct pcre2_pattern));
  return 0;
}

static int
pcre2_pattern_compile (void *sp_data, const char *pattern, int pflags)
{
  struct pcre2_pattern *pre = sp_data;
  int flags = 0;
  int error_code;

  if (pflags & GENPAT_ICASE)
    flags |= PCRE2_CASELESS;
  if (pflags & GENPAT_MULTILINE)
    flags |= PCRE2_MULTILINE;

  pre->code = pcre2_compile ((PCRE2_SPTR8) pattern, strlen (pattern), flags,
			     &error_code, &pre->erroff, NULL);
  if (pre->code == NULL)
    {
      size_t errsize = 32;
      int rc;

      pre->errmsg = malloc (errsize);
      if (!pre->errmsg)
	return -1;

      while ((rc = pcre2_get_error_message (error_code, (PCRE2_UCHAR*) pre->errmsg, errsize)) ==
	     PCRE2_ERROR_NOMEMORY)
	{
	  char *p = mem2nrealloc (pre->errmsg, &errsize, 1);
	  if (!p)
	    break;
	  pre->errmsg = p;
	}

      return -1;
    }
  else
    {
      uint32_t nsub;
      if (pcre2_pattern_info (pre->code, PCRE2_INFO_CAPTURECOUNT, &nsub))
	nsub = 0;
      else
	nsub++;
      pre->nsub = nsub;
    }

  return 0;
}

static void
pcre2_pattern_free (void *sp_data)
{
  if (sp_data)
    {
      struct pcre2_pattern *pre = sp_data;
      pcre2_code_free (pre->code);
      free (pre->errmsg);
      free (pre);
    }
}

static char const *
pcre2_pattern_error (void *sp_data, size_t *off)
{
  struct pcre2_pattern *pre = sp_data;
  *off = pre->erroff;
  return pre->errmsg;
}

static size_t
pcre2_pattern_nsub (void *sp_data)
{
  struct pcre2_pattern *pre = sp_data;
  return pre->nsub;
}

static int
pcre2_pattern_exec (void *sp_data, const char *subj, size_t n, POUND_REGMATCH *prm)
{
  struct pcre2_pattern *pre = sp_data;
  int rc;
  PCRE2_SIZE *ovector;
  size_t i, j;
  pcre2_match_data *md;

  md = pcre2_match_data_create_from_pattern (pre->code, NULL);
  if (!md)
    return -1;

  rc = pcre2_match (pre->code, (PCRE2_SPTR8)subj, strlen (subj), 0, 0, md, NULL);
  if (rc < 0)
    {
      pcre2_match_data_free (md);
      return rc;
    }

  if (n > rc)
    n = rc;

  ovector = pcre2_get_ovector_pointer (md);
  for (i = j = 0; i < n; i++, j += 2)
    {
      prm[i].rm_so = ovector[j];
      prm[i].rm_eo = ovector[j+1];
    }

  pcre2_match_data_free (md);
  return 0;
}

struct genpat_defn pcre_genpat_defn =
  {
    .gp_init = pcre2_pattern_init,
    .gp_compile = pcre2_pattern_compile,
    .gp_error = pcre2_pattern_error,
    .gp_exec = pcre2_pattern_exec,
    .gp_nsub = pcre2_pattern_nsub,
    .gp_free = pcre2_pattern_free
  };
