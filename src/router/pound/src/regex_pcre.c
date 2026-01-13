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
#if HAVE_PCRE_H
# include <pcre.h>
#elif HAVE_PCRE_PCRE_H
# include <pcre/pcre.h>
#else
#  error "You have libpcre, but the header files are missing. Use --disable-pcre"
#endif

struct pcre_pattern
{
  pcre *pcre;
  size_t nsub;
  char const *errmsg;
  int erroff;
};

static int
pcre_pattern_init (GENPAT strpat)
{
  strpat->data = xzalloc (sizeof (struct pcre_pattern));
  return 0;
}

static int
pcre_pattern_compile (void *gp_data, const char *pattern, int pflags)
{
  struct pcre_pattern *pre = gp_data;
  int flags = 0;
  int nsub;

  if (pflags & GENPAT_ICASE)
    flags |= PCRE_CASELESS;
  if (pflags & GENPAT_MULTILINE)
    flags |= PCRE_MULTILINE;

  pre->pcre = pcre_compile (pattern, flags, &pre->errmsg, &pre->erroff, 0);

  if (pre->pcre == NULL)
    return -1;

  if (pcre_fullinfo (pre->pcre, NULL, PCRE_INFO_CAPTURECOUNT, &nsub))
    nsub = 0;
  else
    nsub++;
  pre->nsub = nsub;

  return 0;
}

static char const *
pcre_pattern_error (void *gp_data, size_t *off)
{
  struct pcre_pattern *pre = gp_data;
  *off = pre->erroff;
  return pre->errmsg;
}

static int
pcre_pattern_exec (void *gp_data, const char *subj, size_t n, POUND_REGMATCH *prm)
{
  struct pcre_pattern *pre = gp_data;
  int rc;
  int ovsize;
  int *ovector;

  ovsize = pre->nsub * 3;
  ovector = calloc (ovsize, sizeof (ovector[0]));
  if (!ovector)
    return -1;

  rc = pcre_exec (pre->pcre, 0, subj, strlen (subj), 0, 0, ovector, ovsize);
  if (rc > 0)
    {
      size_t i, j;

      /* Collect captured substrings */
      if (n > rc)
	n = rc;

      for (i = j = 0; i < n; i++, j += 2)
	{
	  prm[i].rm_so = ovector[j];
	  prm[i].rm_eo = ovector[j+1];
	}
    }
  free (ovector);

  return rc < 0;
}

static size_t
pcre_pattern_nsub (void *gp_data)
{
  struct pcre_pattern *pre = gp_data;
  return pre->nsub;
}

static void
pcre_pattern_free (void *gp_data)
{
  if (gp_data)
    {
      struct pcre_pattern *pre = gp_data;
      pcre_free (pre->pcre);
      free (pre);
    }
}

struct genpat_defn pcre_genpat_defn =
  {
    .gp_init = pcre_pattern_init,
    .gp_compile = pcre_pattern_compile,
    .gp_error = pcre_pattern_error,
    .gp_exec = pcre_pattern_exec,
    .gp_nsub = pcre_pattern_nsub,
    .gp_free = pcre_pattern_free
  };
