/*
 * NAME
 *   getsoa - retrieve soa record of an NS zone.
 *
 * SYNOPSIS
 *   LD_PRELOAD=/path/to/libfakedns.so getsoa [-d] ZONE NS
 *
 * DESCRIPTION
 *   Queries NS for the SOA record of an NS zone and displays it as
 *   a series of space-delimited values.  Used as a helper for the
 *   self-test mode in poundharness.pl.
 *
 * OPTIONS
 *   -d     Enables adns debugging output.
 *
 * SEE ALSO
 *   fakedns.c
 *
 * LICENSE
 *   This library is part of pound testsuite.
 *   Copyright (C) 2024-2025 Sergey Poznyakoff
 *
 *   Pound is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Pound is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with pound.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <adns.h>
#include <assert.h>
#include <string.h>

char *progname;

static void
usage (FILE *fp)
{
  fprintf (stderr, "%s ZONE NS\n", progname);
}

#define NSSTR "nameserver "

#define DEFAULT_QFLAGS	       \
	(adns_qf_cname_loose | \
	 adns_qf_quoteok_query | \
	 adns_qf_quoteok_cname | \
	 adns_qf_quoteok_anshost)

/*
  getsoa ZONE NS
 */
int
main (int argc, char **argv)
{
  char *zone;
  char *nameserver = NULL;
  adns_state state;
  adns_initflags flags = adns_if_none;
  int rc, i;
  char *config_text = NULL;
  adns_answer *ans = NULL;

  progname = argv[0];

  while ((rc = getopt (argc, argv, "dh")) != EOF)
    {
      switch (rc)
	{
	case 'd':
	  flags |= adns_if_debug;
	  break;

	case 'h':
	  usage (stdout);
	  return 0;

	default:
	  return 1;
	}
    }

  argc -= optind;
  argv += optind;

  switch (argc)
    {
    case 2:
      nameserver = argv[1];
      zone = argv[0];
      break;
    default:
      fprintf (stderr, "%s: bad number of arguments\n", progname);
      usage (stderr);
      return 1;
    }

  config_text = malloc (sizeof (NSSTR) + 1 + strlen (nameserver));
  assert (config_text != NULL);
  strcat (strcpy (config_text, NSSTR), nameserver);

  rc = adns_init_strcfg (&state, flags, stderr, config_text);
  if (rc)
    {
      fprintf (stderr, "%s: can't initialize DNS state: %s",
	       progname, strerror (rc));
      return 1;
    }

  rc = adns_synchronous (state, zone, adns_r_soa, DEFAULT_QFLAGS, &ans);
  if (rc != adns_s_ok)
    {
      fprintf (stderr, "%s: %s\n", progname, adns_strerror (rc));
      return 1;
    }

  for (i = 0; i < ans->nrrs; i++)
    {
      printf ("%s. %s. %ld %ld %ld %ld %ld\n",
	      ans->rrs.soa[i].mname,
	      ans->rrs.soa[i].rname,
	      ans->rrs.soa[i].serial,
	      ans->rrs.soa[i].refresh,
	      ans->rrs.soa[i].retry,
	      ans->rrs.soa[i].expire,
	      ans->rrs.soa[i].minimum);
    }
  free (ans);
  return 0;
}
