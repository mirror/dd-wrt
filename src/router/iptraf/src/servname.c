/***

servname.c	- lookup module for TCP and UDP service names based on
		  port numbers

Copyright (c) Gerard Paul Java 1998

This software is open source; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License in the included COPYING file for
details.

***/

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>

void servlook(int servnames, unsigned int port, unsigned int protocol,
              char *target, int maxlen)
{
    static struct servent *sve;

    bzero(target, maxlen + 1);

    if (servnames) {
        if (protocol == IPPROTO_TCP)
            sve = getservbyport(port, "tcp");
        else
            sve = getservbyport(port, "udp");

        if (sve != NULL) {
            strncpy(target, sve->s_name, maxlen);
        } else {
            sprintf(target, "%u", ntohs(port));
        }
    } else {
        sprintf(target, "%u", ntohs(port));
    }
}
