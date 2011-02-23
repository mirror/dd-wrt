/**********************************************************
 SixXS - Automatic IPv6 Connectivity Configuration Utility
***********************************************************
 Copyright 2003-2005 SixXS - http://www.sixxs.net
***********************************************************
 common/aiccu_kame.c - NetBSD + FreeBSD 5.x+
***********************************************************
 $Author: jeroen $
 $Id: aiccu_kame.c,v 1.15 2007-01-11 15:11:27 jeroen Exp $
 $Date: 2007-01-11 15:11:27 $
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
		aiccu_exec(
			"/sbin/ifconfig %s create",
			g_aiccu->ipv6_interface);

		aiccu_exec(
			"/sbin/ifconfig %s tunnel %s %s",
			g_aiccu->ipv6_interface,
			strcmp(hTunnel->sIPv4_Local, "heartbeat") == 0 ? "0.0.0.0" : hTunnel->sIPv4_Local,
			hTunnel->sIPv4_POP);
	}

	aiccu_exec(
		"ifconfig %s up",
		g_aiccu->ipv6_interface);

        aiccu_exec(
		"ifconfig %s mtu %u",
		g_aiccu->ipv6_interface,
		hTunnel->nMTU);

	if (hTunnel->uses_tundev == 1)
	{
		/* Configure a generated linklocal address */
		aiccu_exec(
			"ifconfig %s inet6 %s prefixlen 64 alias",
			g_aiccu->ipv6_interface,
			hTunnel->sIPv6_LinkLocal);
	}

	/* PtP link, so we can use the PtP syntax */
	aiccu_exec(
		"ifconfig %s inet6 %s %s prefixlen 128 alias",
		g_aiccu->ipv6_interface,
		hTunnel->sIPv6_Local,
		hTunnel->sIPv6_POP);

	if (g_aiccu->defaultroute)
	{
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
	aiccu_exec(
		"ifconfig %s down",
		g_aiccu->ipv6_interface);

	if (hTunnel->uses_tundev == 0)
	{
		aiccu_exec(
			"ifconfig %s deletetunnel",
			g_aiccu->ipv6_interface);
	}
}

