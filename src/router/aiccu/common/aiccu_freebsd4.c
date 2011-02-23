/**********************************************************
 SixXS - Automatic IPv6 Connectivity Configuration Utility
***********************************************************
 Copyright 2003-2005 SixXS - http://www.sixxs.net
***********************************************************
 common/aiccu_freebsd4.c - FreeBSD 3.x/4.x
***********************************************************
 $Author: jeroen $
 $Id: aiccu_freebsd4.c,v 1.11 2007-01-07 17:05:23 jeroen Exp $
 $Date: 2007-01-07 17:05:23 $
**********************************************************/

#include "aiccu.h"

bool aiccu_os_install(void)
{
	return true;
}

bool aiccu_os_setup(struct TIC_Tunnel *hTunnel)
{
	if (hTunnel->uses_tundev == 0)
	{
		/* Create the tunnel device */
		aiccu_exec(
			"/sbin/ifconfig %s create",
			g_aiccu->ipv6_interface);

		/* Configure the endpoint */
		aiccu_exec(
			"/sbin/ifconfig %s tunnel %s %s",
			g_aiccu->ipv6_interface,
			strcmp(hTunnel->sIPv4_Local, "heartbeat") == 0 ? "0.0.0.0" : hTunnel->sIPv4_Local,
			hTunnel->sIPv4_POP);
	}

	/* Mark the interface up */
	aiccu_exec(
		"ifconfig %s up",
		g_aiccu->ipv6_interface);

	/* Configure the MTU */
	aiccu_exec(
		"ifconfig %s mtu %u",
		g_aiccu->ipv6_interface,
		hTunnel->nMTU);

	if (hTunnel->uses_tundev == 1)
	{
		/* Give it a link local address */
		aiccu_exec(
			"ifconfig %s inet6 %s prefixlen 64 alias",
			g_aiccu->ipv6_interface,
			hTunnel->sIPv6_LinkLocal);
	}

	/* Local side of the tunnel */
	aiccu_exec(
		"ifconfig %s inet6 %s prefixlen 128 alias",
		g_aiccu->ipv6_interface,
		hTunnel->sIPv6_Local);

	/* Route to the remote side of the tunnel */
	aiccu_exec(
		"route add -inet6 %s -prefixlen 128 %s",
		hTunnel->sIPv6_POP,
		hTunnel->sIPv6_Local);

	if (g_aiccu->defaultroute)
	{
		/* Add a default route */
		aiccu_exec(
			"route add -inet6 %s %s",
			"default",
			hTunnel->sIPv6_POP);
	}

	return true;
}

void aiccu_os_reconfig(struct TIC_Tunnel *hTunnel)
{
	if (hTunnel->uses_tundev == 0)
	{
		/* Change the endpoints of the tunnel */
		aiccu_exec(
			"/sbin/ifconfig %s tunnel %s %s",
			g_aiccu->ipv6_interface,
			hTunnel->sIPv4_Local,
			hTunnel->sIPv4_POP);
	}
}

void aiccu_os_delete(struct TIC_Tunnel *hTunnel)
{
	hTunnel = hTunnel;

	/* Mark the interface down */
	aiccu_exec(
		"ifconfig %s down",
		g_aiccu->ipv6_interface);

	if (hTunnel->uses_tundev == 0)
	{
		/* Destroy the tunnel */
		aiccu_exec(
			"ifconfig %s destroy",
			g_aiccu->ipv6_interface);
	}
}

