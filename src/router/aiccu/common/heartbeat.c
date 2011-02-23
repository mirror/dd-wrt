/**********************************************************
 SixXS - Automatic IPv6 Connectivity Configuration Utility
***********************************************************
 Copyright 2003-2005 SixXS - http://www.sixxs.net
***********************************************************
 common/heartbeat.c - Heartbeat Code
***********************************************************
 $Author: jeroen $
 $Id: heartbeat.c,v 1.9 2006-12-21 14:08:50 jeroen Exp $
 $Date: 2006-12-21 14:08:50 $
**********************************************************/

#include "heartbeat.h"
#include "aiccu.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#if defined(SOL_SOCKET) && defined(SO_BINDTODEVICE)
#include <net/if.h>
#endif

/**************************************
  Functions
**************************************/

/* Get a socket and determine the new IP address */
SOCKET heartbeat_socket(
	uint32_t *address_changed,
	int bStaticTunnel,
	const char *sIPv4Interface,
	char **sIPv4Local,
	const char *sIPv4POP,
	const char *sIPv4LocalResolve)
{
	SOCKET		sockfd;
	struct sockaddr	sa;
	socklen_t	socklen;
	char		local_ipv4[NI_MAXHOST];
#if defined(SOL_SOCKET) && defined(SO_BINDTODEVICE)
	struct ifreq	interface;
#endif
	struct addrinfo	hints, *res, *ressave;

	D(dolog(LOG_DEBUG, "heartbeat_socket() - Address is %s\n", *sIPv4Local));

	if (address_changed) *address_changed = 0;

	/* Get ourselves a nice IPv4 socket */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
	{
		dolog(LOG_ERR, "Couldn't open a socket for determining current IPv4 address\n");
		return -1;
	}

#if defined(SOL_SOCKET) && defined(SO_BINDTODEVICE)
	/*
	 * We don't have to bind to a device if this
	 * is a static tunnel, this allows running
	 * the heartbeat client as non-root.
	 */
	if (!bStaticTunnel)
	{
		/*
		 * Only allowed as root, but we need root rights anyways
		 * to (re)configure the tunnel
		 */

		/* Bind to the underlying IPv4 device */
		memset(&interface, 0, sizeof(interface));
		strncpy(interface.ifr_ifrn.ifrn_name, sIPv4Interface, IFNAMSIZ);
		if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE,
			(char *)&interface, sizeof(interface)) == -1)
		{
			dolog(LOG_ERR, "Couldn't bind to device \"%s\"\n", sIPv4Interface);
			close(sockfd);

			/* We return a -1, thus the app will keep beating */
			return -1;
		}
	}
#else
	/* Make compiler happy and 'use' the param */
	sIPv4Interface = sIPv4Interface;
#endif

	/*
	 * connect to the remote port
	 * this causes us to be able to use normal write()
	 * and also gives us the ability to find out the local IP
	 */
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	/* Get the POP IPv4 into a sockaddr */
	if (getaddrinfo(sIPv4POP, HEARTBEAT_PORT, &hints, &res) != 0)
	{
		dolog(LOG_ERR, "Couldn't resolve POP ip %s\n", sIPv4POP);
		closesocket(sockfd);

		/* We return a -1, thus the app will keep beating */
		return -1;
	}

	ressave = res;

	while (res)
	{
		if (connect(sockfd, res->ai_addr, (unsigned int)res->ai_addrlen) == 0) break;
		res = res->ai_next;
	}
	freeaddrinfo(ressave);
	if (res == NULL)
	{
		dolog(LOG_ERR, "Failed to connect() to remote side\n");
		closesocket(sockfd);
		/* We return a -1, thus the app will keep beating */
		return -1;
	}

	/* Normal operation, find out our local IPv4 address */
	if (sIPv4LocalResolve == NULL)
	{
		/* Figure out our local IP */
		socklen = sizeof(sa);
		if (getsockname(sockfd, &sa, &socklen) == -1)
		{
			dolog(LOG_WARNING, "Couldn't get local socketaddress\n");
			closesocket(sockfd);
			return -1;
		}

		if (getnameinfo((struct sockaddr *)&sa, sizeof(sa),
				local_ipv4, sizeof(local_ipv4),
			NULL, 0,
			NI_NUMERICHOST) != 0)
		{
			dolog(LOG_WARNING, "Couldn't get local IP\n");
			closesocket(sockfd);
			return -1;
		}
	}
	else
	{
		/*
		 * this causes us to be able to use normal write()
		 * and also gives us the ability to find out the local IP
		 */
		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_DGRAM;
		
		/* Get the POP IPv4 into a sockaddr */
		if (getaddrinfo(sIPv4LocalResolve, NULL, &hints, &res) != 0)
		{
			dolog(LOG_ERR, "Couldn't resolve POP IPv4 %s\n", sIPv4POP);
			/* We return a -1, thus the app will keep beating */
			return -1;
		}
		ressave = res;
		while (res)
		{
			if (getnameinfo(res->ai_addr, (socklen_t)res->ai_addrlen,
				local_ipv4, sizeof(local_ipv4),
				NULL, 0,
				NI_NUMERICHOST) == 0)
			{
				break;
			}
			dolog(LOG_WARNING, "Couldn't get local IP\n");
			res = res->ai_next;
		}
		freeaddrinfo(ressave);
	}

	/* Did the IPv4 address change? */
	if (*sIPv4Local == NULL ||
		strcmp(*sIPv4Local, local_ipv4) != 0)
	{
		if (*sIPv4Local) free(*sIPv4Local);
		*sIPv4Local = strdup(local_ipv4);

		dolog(LOG_DEBUG, "heartbeat_socket() - IPv4 : %s\n", *sIPv4Local);

		if (!bStaticTunnel)
		{
			/* Run a script to change the address */
			if (address_changed) *address_changed = 1;
		}
	}
	D(else dolog(LOG_DEBUG, "heartbeat_socket() - Address stays %s\n", *sIPv4Local));

	/* Return it */
	return sockfd;
}

/* Send a heartbeat */
int heartbeat_send(SOCKET sockfd, char *sIPv4Local, char *sIPv6Local, char *sPassword, bool bBehindNAT)
{
	struct MD5Context	md5;
	unsigned char		*p, our_digest[20], *pn = our_digest, buf[1000];
	time_t			time_tee;
	int			i;

	time_tee = time(NULL);

	/* Create the string to send including our password */
	snprintf((char *)buf, sizeof(buf), "HEARTBEAT TUNNEL %s %s %ld %s",
		sIPv6Local,
		(bBehindNAT ? "sender" : sIPv4Local),
		(long int)time_tee, sPassword);

	/* Generate a MD5 */
	MD5Init(&md5);
	MD5Update(&md5, buf, (unsigned int)strlen((char *)buf));
	MD5Final(our_digest, &md5);

	/* Overwrite it without password */
	p = buf;
	p += snprintf((char *)buf, sizeof(buf)-17, "HEARTBEAT TUNNEL %s %s %ld ",
		sIPv6Local,
		(bBehindNAT == 1 ? "sender" : sIPv4Local),
		(long int)time_tee);

	/* append the digest */
	for (i = 0; i < 16; i++)
	{
		snprintf((char *)p, 3, (const char *)"%02x", *pn++);
		p+=2;
	}
	*p = '\0';

	/* Send the heartbeat */
	send(sockfd, (const char *)buf, (unsigned int)strlen((const char *)buf),0);

	dolog(LOG_DEBUG, "[HB] %s\n", buf);

	return 0;
}

char *heartbeat_getlocalIP(struct TIC_Tunnel *hTunnel)
{
	bool address_changed = false;
	char *ipv4_local = NULL;

	SOCKET sockfd = heartbeat_socket(&address_changed, 0, "",
		&ipv4_local,
		hTunnel->sIPv4_POP,
		NULL);
	if (sockfd >= 0) closesocket(sockfd);

	dolog(LOG_DEBUG, "Local IPv4 address: %s\n", ipv4_local);

	return ipv4_local;
}

/* 
 * Other code can call this every once in a while
 * and it will take care of everything ("everything?" "everything!")
 */
void heartbeat_beat(struct TIC_Tunnel *hTunnel)
{
	uint32_t address_changed = 0;
	SOCKET sockfd = -1;

	D(dolog(LOG_DEBUG, "heartbeat_beat() - Beating from %s\n", hTunnel->sIPv4_Local);)

	sockfd = heartbeat_socket(&address_changed, 0, "",
		&hTunnel->sIPv4_Local,
		hTunnel->sIPv4_POP,
		NULL);
	if (sockfd >= 0)
	{
		if (address_changed == 1)
		{
			D(dolog(LOG_DEBUG, "heartbeat_beat() - Address changed to %s\n", hTunnel->sIPv4_Local);)
			aiccu_reconfig(hTunnel);
		}
		heartbeat_send(sockfd,
			hTunnel->sIPv4_Local,
			hTunnel->sIPv6_Local,
			hTunnel->sPassword,
			1);
		closesocket(sockfd);
		sockfd = (SOCKET)-1;
	}
}

