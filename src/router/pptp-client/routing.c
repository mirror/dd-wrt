/*
    routing.c, manipulating routing table for PPTP Client
    Copyright (C) 2006  James Cameron <quozl@us.netrek.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "routing.h"

/* route to the server */
char *route;

/*

Design discussion.

The primary task of this module is to add a host route to the PPTP
server so that the kernel continues to deliver PPTP control and data
connection packets to the server despite the new PPP interface that is
created.  The flag --no-host-route is to disable this (not yet implemented).

A secondary task may be to implement all-to-tunnel routing if the
appropriate flag is specified on the command line.  The flag
--route-all is to implement this (not yet implemented).

Caveat.

It is not possible from the "ip route" command to determine if a host
route already exists, so it isn't practical to put the routing table
back exactly as it was.

We have a choice of either leaving our route lying around, or
destroying a route that the user had pre-arranged.  Both are
unfortunate.  The flag --remove-host-route is to remove the route
regardless (not yet implemented).

*/

void routing_init(char *ip) {
  char buf[256];
  snprintf(buf, 255, "/bin/ip route get %s", ip);
  FILE *p = popen(buf, "r");
  fgets(buf, 255, p);
  /* TODO: check for failure of fgets */
  route = strdup(buf);
  pclose(p);
  /* TODO: check for failure of command */
}

void routing_start() {
  char buf[256];
  snprintf(buf, 255, "/bin/ip route replace %s", route);
  FILE *p = popen(buf, "r");
  pclose(p);
}

void routing_end() {
  char buf[256];
  snprintf(buf, 255, "/bin/ip route delete %s", route);
  FILE *p = popen(buf, "r");
  pclose(p);
}
