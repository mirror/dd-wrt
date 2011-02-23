/**********************************************************
 SixXS - Automatic IPv6 Connectivity Configuration Utility
***********************************************************
 Copyright 2003-2005 SixXS - http://www.sixxs.net
***********************************************************
 common/aiccu_openbsd2.c - OpenBSD 2.7-2.9
***********************************************************
 $Author: jeroen $
 $Id: aiccu_openbsd2.c,v 1.7 2007-01-07 16:37:50 jeroen Exp $
 $Date: 2007-01-07 16:37:50 $

 Original version provided by Wouter Van Hemel
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
			"/sbin/ifconfig %s giftunnel %s %s",
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
		aiccu_exec(
			"ifconfig %s inet6 %s prefixlen 64 alias",
			g_aiccu->ipv6_interface,
			hTunnel->sIPv6_LinkLocal);
	}

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
	aiccu_exec(
		"/sbin/ifconfig %s giftunnel %s %s",
		g_aiccu->ipv6_interface,
		hTunnel->sIPv4_Local,
		hTunnel->sIPv4_POP);
}

void aiccu_os_delete(struct TIC_Tunnel *hTunnel)
{
	hTunnel = hTunnel;
	aiccu_exec(
		"ifconfig %s down",
		g_aiccu->ipv6_interface);

	aiccu_exec(
		"ifconfig %s inet6 %s %s prefixlen 128 -alias",
		g_aiccu->ipv6_interface,
		hTunnel->sIPv6_Local,
		hTunnel->sIPv6_POP);

}

