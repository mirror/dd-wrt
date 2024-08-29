/*
 * lib/x25_sr.c	This file contains an implementation of the "X.25"
 *		route change support functions.
 *
 * Author:	Stephane Fillod, <sfillod@charybde.gyptis.frmug.org>
 *		based on inet_sr.c
 *
 *		This program is free software; you can redistribute it
 *		and/or  modify it under  the terms of  the GNU General
 *		Public  License as  published  by  the  Free  Software
 *		Foundation;  either  version 2 of the License, or  (at
 *		your option) any later version.
 */
#include "config.h"

#if HAVE_AFX25
#include <asm/types.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/x25.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <resolv.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "version.h"
#include "net-support.h"
#include "pathnames.h"
#define  EXTERN
#if 0
#include "net-locale.h"
#endif
#include "intl.h"
#include "util.h"

#include "net-features.h"

extern     struct aftype   x25_aftype;

static int skfd = -1;


static int usage(const int rc)
{
  FILE *fp = rc ? stderr : stdout;
  fprintf(fp, "Usage: x25_route [-v] del Target[/mask] [dev] If\n");
  fprintf(fp, "       x25_route [-v] add Target[/mask] [dev] If\n");
  return(rc);
}


static int X25_setroute(int action, int options, char **args)
{
  struct x25_route_struct rt;
  struct sockaddr_storage sas;
  struct sockaddr_x25 *sx25 = (struct sockaddr_x25 *)&sas;
  char target[128];
  signed int sigdigits;

  if (*args == NULL)
	return usage(E_OPTERR);

  safe_strncpy(target, *args++, sizeof(target));

  /* Clean out the x25_route_struct structure. */
  memset((char *) &rt, 0, sizeof(rt));


  if ((sigdigits = x25_aftype.input(0, target, &sas)) < 0) {
	x25_aftype.herror(target);
	return (E_LOOKUP);
  }
  rt.sigdigits=sigdigits;

  /* this works with 2.4 and 2.6 headers struct x25_address vs. typedef */
  memcpy(&rt.address, &sx25->sx25_addr, sizeof(sx25->sx25_addr));

  while (*args) {
	if (!strcmp(*args,"device") || !strcmp(*args,"dev")) {
		args++;
		if (!*args)
			return usage(E_OPTERR);
	} else
		if (args[1])
			return usage(E_OPTERR);
	if (rt.device[0])
		return usage(E_OPTERR);
	safe_strncpy(rt.device, *args, sizeof(rt.device));
	args++;
  }
  if (rt.device[0]=='\0')
	return usage(E_OPTERR);

  /* sanity checks.. */
	if (rt.sigdigits > 15) {
		fprintf(stderr, _("route: bogus netmask %d\n"), rt.sigdigits);
		return usage(E_OPTERR);
	}

	if (rt.sigdigits > strlen(rt.address.x25_addr)) {
		fprintf(stderr, _("route: netmask doesn't match route address\n"));
		return usage(E_OPTERR);
	}

  /* Create a socket to the X25 kernel. */
  if ((skfd = socket(AF_X25, SOCK_SEQPACKET, 0)) < 0) {
	perror("socket");
	return(E_SOCK);
  }

  /* Tell the kernel to accept this route. */
  if (action==RTACTION_DEL) {
	if (ioctl(skfd, SIOCDELRT, &rt) < 0) {
		perror("SIOCDELRT");
		close(skfd);
		return(E_SOCK);
	}
  } else {
	if (ioctl(skfd, SIOCADDRT, &rt) < 0) {
		perror("SIOCADDRT");
		close(skfd);
		return(E_SOCK);
	}
  }

  /* Close the socket. */
  (void) close(skfd);
  return(0);
}

int X25_rinput(int action, int options, char **args)
{
  if (action == RTACTION_FLUSH) {
  	fprintf(stderr,"Flushing `x25' routing table not supported\n");
  	return usage(E_OPTERR);
  }
  if (options & FLAG_CACHE) {
  	fprintf(stderr,"Modifying `x25' routing cache not supported\n");
  	return usage(E_OPTERR);
  }
  if (action == RTACTION_HELP)
	return usage(E_USAGE);

  return(X25_setroute(action, options, args));
}
#endif	/* HAVE_AFX25 */
