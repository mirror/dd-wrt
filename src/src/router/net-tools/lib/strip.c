/*
 * lib/strip.c	This file contains an implementation of the STRIP
 *		support functions.
 *
 * Version:	strip.c	1.20 1999/04/22 eswierk
 *
 * Author:	Stuart Cheshire <cheshire@cs.stanford.edu>
 *
 *		This program is free software; you can redistribute it
 *		and/or  modify it under  the terms of  the GNU General
 *		Public  License as  published  by  the  Free  Software
 *		Foundation;  either  version 2 of the License, or  (at
 *		your option) any later version.
 */
#include "config.h"

#if HAVE_HWSTRIP

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if_arp.h>
#include <linux/types.h>
#include <linux/if_strip.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include "net-support.h"
#include "pathnames.h"
#include "util.h"
#include "intl.h"


extern struct hwtype strip_hwtype;

static char *
pr_strip(unsigned char *ptr)
{
  static char buff[64];
  if(ptr[1])
      sprintf(buff, "%02x-%02x%02x-%02x%02x", *(ptr+1), *(ptr+2), *(ptr+3),
          *(ptr+4), *(ptr+5));
   else
      sprintf(buff, "%02x%02x-%02x%02x", *(ptr+2), *(ptr+3), *(ptr+4),
	  *(ptr+5));
  return buff;
}

static int
in_strip(char *bufp, struct sockaddr *sap)
{
  int i,i0;
  MetricomAddress *haddr = (MetricomAddress *) (sap->sa_data);


  sap->sa_family = strip_hwtype.type;

  /* figure out what the device-address should be */
  i0 = i = (bufp[0] == '*') ? 1 : 0;

  while (bufp[i] && (bufp[i] != '-'))
    i++;

  if (bufp[i] != '-')
    return -1;

  if(i-i0 == 2)
  {
     haddr->c[1] = strtol(&bufp[i0], 0, 16);
     i++;
     if(bufp[i] == 0) return -1;
  }else{
     haddr->c[1] = 0;
     i=i0;
  }
  haddr->c[2] = strtol(&bufp[i], 0, 16) >> 8;
  haddr->c[3] = strtol(&bufp[i], 0, 16) & 0xFF;

  while (bufp[i] && (bufp[i] != '-'))
    i++;

  if (bufp[i] != '-')
    return -1;

  haddr->c[4] = strtol(&bufp[i+1], 0, 16) >> 8;
  haddr->c[5] = strtol(&bufp[i+1], 0, 16) & 0xFF;
  haddr->c[0] = 0;

  return 0;
}



/* Start the STRIP encapsulation on the file descriptor. */
static int do_strip(int fd)
	{
	int disc = N_STRIP;
	if (ioctl(fd, TIOCSETD, &disc) < 0)
		{
		fprintf(stderr, "STRIP_set_disc(%d): %s\n", disc, strerror(errno));
		return(-errno);
		}
	return(0);
	}

struct hwtype strip_hwtype = {
  "strip", "Metricom Starmode IP", ARPHRD_METRICOM, sizeof(MetricomAddress),
  pr_strip, in_strip, do_strip, 0
};

#endif	/* HAVE_HWSTRIP */
