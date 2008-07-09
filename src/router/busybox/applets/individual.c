/* Minimal wrapper to build an individual busybox applet.
 *
 * Copyright 2005 Rob Landley <rob@landley.net
 *
 * Licensed under GPL version 2, see file LICENSE in this tarball for details
 */

const char *applet_name;

#include <stdio.h>
#include <stdlib.h>
#include "usage.h"


#ifdef HAVE_DHCPFWD
  extern int dhcpforward_main(int argc,char *argv[]);
#endif

int main(int argc, char **argv)
{
  char *base = strrchr (argv[0], '/');
  base = base ? base + 1 : argv[0];
	applet_name = argv[0];

	return APPLET_main(argc,argv);
}

#ifndef HAVE_NOMESSAGE
void bb_show_usage(void)
{
 	fputs(APPLET_full_usage "\n", stdout);

	exit(EXIT_FAILURE);
}
#endif