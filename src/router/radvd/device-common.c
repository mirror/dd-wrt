/*
 *   $Id: device-common.c,v 1.3 2002/01/02 11:01:11 psavola Exp $
 *
 *   Authors:
 *    Lars Fenneberg		<lf@elemental.net>	 
 *
 *   This software is Copyright 1996,1997 by the above mentioned author(s), 
 *   All Rights Reserved.
 *
 *   The license which is distributed with this software in the file COPYRIGHT
 *   applies to this software. If your distribution is missing this file, you
 *   may request it from <lutchann@litech.org>.
 *
 */

#include <config.h>
#include <includes.h>
#include <radvd.h>
#include <defaults.h>

int
check_device(int sock, struct Interface *iface)
{
	struct ifreq	ifr;
	
	strncpy(ifr.ifr_name, iface->Name, IFNAMSIZ-1);
	ifr.ifr_name[IFNAMSIZ-1] = '\0';

	if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0)
	{
		log(LOG_ERR, "ioctl(SIOCGIFFLAGS) failed for %s: %s", 
			iface->Name, strerror(errno));
		return (-1);
	}

	if (!(ifr.ifr_flags & IFF_UP))
	{
		log(LOG_ERR, "interface %s is not UP", iface->Name);
		return (-1);
	}
	
	if (! iface->UnicastOnly && !(ifr.ifr_flags & IFF_MULTICAST))
	{
		log(LOG_WARNING, "interface %s does not support multicast",
			iface->Name);
		log(LOG_WARNING, "   do you need to add the UnicastOnly flag?");
	}

	if (! iface->UnicastOnly && !(ifr.ifr_flags & IFF_BROADCAST))
	{
		log(LOG_WARNING, "interface %s does not support broadcast",
			iface->Name);
		log(LOG_WARNING, "   do you need to add the UnicastOnly flag?");
	}

	return 0;
}
