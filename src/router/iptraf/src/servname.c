/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

/***

servname.c	- lookup module for TCP and UDP service names based on
		  port numbers

***/

#include "iptraf-ng-compat.h"

void servlook(int servnames, unsigned int port, unsigned int protocol,
	      char *target, int maxlen)
{
	static struct servent *sve;

	memset(target, 0, maxlen + 1);

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
