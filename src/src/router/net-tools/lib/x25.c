/*
 * lib/x25.c	This file contains an implementation of the "X.25"
 *		support functions for the NET-3 base distribution.
 *
 * Version:	@(#)x25.c	1.00	08/15/98
 *
 * Author:	Stephane Fillod, <sfillod@charybde.gyptis.frmug.org>
 *		based on ax25.c by:
 * 		Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
 *		Copyright 1993 MicroWalt Corporation
 *
 *		This program is free software; you can redistribute it
 *		and/or  modify it under  the terms of  the GNU General
 *		Public  License as  published  by  the  Free  Software
 *		Foundation;  either  version 2 of the License, or  (at
 *		your option) any later version.
 */
#include "config.h"

#if HAVE_AFX25 || HAVE_HWX25
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/x25.h>
#include <net/if_arp.h>		/* ARPHRD_X25 */
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
#define  EXTERN
#if 0
#include "net-locale.h"
#endif
#include "intl.h"

static char X25_errmsg[128];


extern struct aftype x25_aftype;

/* is in net/x25.h, not in the public header file linux/x25.h. Why?*/
#ifndef X25_ADDR_LEN
#define X25_ADDR_LEN 16
#endif


static char *
X25_print(unsigned char *ptr)
{
  static char buff[X25_ADDR_LEN+1];

  strncpy(buff, ptr, X25_ADDR_LEN);
  buff[X25_ADDR_LEN] = '\0';
  return(buff);

}


/* Display an X.25 socket address. */
static char *
X25_sprint(struct sockaddr *sap, int numeric)
{
  if (sap->sa_family == 0xFFFF || sap->sa_family == 0)
    return( _("[NONE SET]"));
  return(X25_print(((struct sockaddr_x25 *)sap)->sx25_addr.x25_addr));
}


/*
 * return the sigdigits of the address
 */
static int
X25_input(int type, char *bufp, struct sockaddr *sap)
{
  unsigned char *ptr;
  char *p;
  unsigned int sigdigits;

  sap->sa_family = x25_aftype.af;
  ptr = ((struct sockaddr_x25 *)sap)->sx25_addr.x25_addr;


  /* Address the correct length ? */
  if (strlen(bufp)>18) {
        strcpy(X25_errmsg, _("Address can't exceed eighteen digits with sigdigits"));
#ifdef DEBUG
        fprintf(stderr, "x25_input(%s): %s !\n", X25_errmsg, orig);
#endif
        errno = EINVAL;
        return(-1);
  }


  if ((p = strchr(bufp, '/')) != NULL) {
        *p = '\0';
        sigdigits = atoi(p + 1);
  } else {
        sigdigits = strlen(bufp);
  }

  if (strlen(bufp) < 1 || strlen(bufp) > 15 || sigdigits > strlen(bufp)) {
	*p = '/';
        strcpy(X25_errmsg, _("Invalid address"));
#ifdef DEBUG
        fprintf(stderr, "x25_input(%s): %s !\n", X25_errmsg, orig);
#endif
        errno = EINVAL;
        return(-1);
  }

  strncpy(ptr, bufp, sigdigits+1);

  /* All done. */
#ifdef DEBUG
  fprintf(stderr, "x25_input(%s): ", orig);
  for (i = 0; i < sizeof(x25_address); i++)
	fprintf(stderr, "%02X ", sap->sa_data[i] & 0377);
  fprintf(stderr, "\n");
#endif

  return sigdigits;
}


/* Display an error message. */
static void
X25_herror(char *text)
{
  if (text == NULL) fprintf(stderr, "%s\n", X25_errmsg);
    else fprintf(stderr, "%s: %s\n", text, X25_errmsg);
}


static int
X25_hinput(char *bufp, struct sockaddr *sap)
{
  if (X25_input(0, bufp, sap) < 0) return(-1);
  sap->sa_family = ARPHRD_X25;
  return(0);
}


struct hwtype x25_hwtype = {
  "x25",	NULL, /*"CCITT X.25",*/		ARPHRD_X25,	X25_ADDR_LEN,
  X25_print,	X25_hinput,	NULL
};

struct aftype x25_aftype =
{   
    "x25", NULL, /*"CCITT X.25", */ AF_X25, X25_ADDR_LEN,
    X25_print, X25_sprint, X25_input, X25_herror,
    X25_rprint, X25_rinput, NULL /* getmask */,
    -1,
    "/proc/net/x25"
};


#endif	/* HAVE_xxX25 */
