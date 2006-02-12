/*
 * lib/x25_sr.c	This file contains an implementation of the "X.25"
 *		route change support functions.
 *
 * Version:	@(#)x25_sr.c	1.00	08/15/98
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

#include "net-features.h"

extern     struct aftype   x25_aftype;

static int skfd = -1;


static int usage(void)
{
  fprintf(stderr,"Usage: x25_route [-v] del Target[/mask] [dev] If\n");
  fprintf(stderr,"       x25_route [-v] add Target[/mask] [dev] If\n");
  return(E_USAGE);
}


static int X25_setroute(int action, int options, char **args)
{
  struct x25_route_struct rt;
  struct sockaddr_x25 sx25;
  char target[128];
  signed int sigdigits;

  if (*args == NULL)
	return(usage());

  strcpy(target, *args++);

  /* Clean out the x25_route_struct structure. */
  memset((char *) &rt, 0, sizeof(struct x25_route_struct));


  if ((sigdigits = x25_aftype.input(0, target, (struct sockaddr *)&sx25)) < 0) {
	x25_aftype.herror(target);
	return (1);
  }
  rt.sigdigits=sigdigits;

  /* x25_route_struct.address isn't type struct sockaddr_x25, Why? */
  memcpy(&rt.address, &sx25.sx25_addr, sizeof(x25_address));

  while (*args) {
	if (!strcmp(*args,"device") || !strcmp(*args,"dev")) {
		args++;
		if (!*args)
			return(usage());
	} else
		if (args[1])
			return(usage());
	if (rt.device[0])
		return(usage());
	strcpy(rt.device, *args);
	args++;
  }
  if (rt.device[0]=='\0')
	return(usage());

  /* sanity checks.. */
	if (rt.sigdigits > 15) {
		fprintf(stderr, _("route: bogus netmask %d\n"), rt.sigdigits);
		return(E_OPTERR);
	}

	if (rt.sigdigits > strlen(rt.address.x25_addr)) {
		fprintf(stderr, _("route: netmask doesn't match route address\n"));
		return(E_OPTERR);
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
  	return(usage());
  }	
  if (options & FLAG_CACHE) {
  	fprintf(stderr,"Modifying `x25' routing cache not supported\n");
  	return(usage());
  }	
  if ((*args == NULL) || (action == RTACTION_HELP))
	return(usage());
  
  return(X25_setroute(action, options, args));
}
#endif	/* HAVE_AFX25 */
