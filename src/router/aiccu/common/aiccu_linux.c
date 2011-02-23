/**********************************************************
 SixXS - Automatic IPv6 Connectivity Configuration Utility
***********************************************************
 Copyright 2003-2005 SixXS - http://www.sixxs.net
***********************************************************
 common/aiccu_linux.c - AICCU Linux Abstracted functions
***********************************************************
 $Author: jeroen $
 $Id: aiccu_linux.c,v 1.15 2007-01-15 12:18:58 jeroen Exp $
 $Date: 2007-01-15 12:18:58 $
**********************************************************/

#include "aiccu.h"

bool aiccu_os_install(void)
{
	/* Check if IPv6 support is available */
	if (access("/proc/net/if_inet6", F_OK))
	{
		/* Doing the modprobe doesn't guarantee success unfortunately */
		(void)system("modprobe -q ipv6 2>/dev/null >/dev/null");

		/* Thus test it again */
		if (access("/proc/net/if_inet6", F_OK))
		{
			dolog(LOG_ERR, "No IPv6 Stack found! Please check your kernel and module configuration\n");
			return false;
		}
	}

	/* Try to load modules (SIT tunnel, TUN/TAP)
	 * They can be kernel builtins and there is no easy
	 * way to check if they are loaded/built except for
	 * trying to use them and fail at that point
	 */
	(void)system("modprobe -q sit 2>/dev/null >/dev/null");
	(void)system("modprobe -q tun 2>/dev/null >/dev/null");

	return true;
}

bool aiccu_os_setup(struct TIC_Tunnel *hTunnel)
{
	if (hTunnel->uses_tundev == 0)
	{
		aiccu_exec(
			"ip tunnel add %s mode sit %s%s remote %s",
			g_aiccu->ipv6_interface,
			strcmp(hTunnel->sIPv4_Local, "heartbeat") == 0 ? "" : "local ",
			strcmp(hTunnel->sIPv4_Local, "heartbeat") == 0 ? "" : hTunnel->sIPv4_Local,
			hTunnel->sIPv4_POP);
	}

	aiccu_exec(
		"ip link set %s up",
		g_aiccu->ipv6_interface);

	aiccu_exec(
		"ip link set mtu %u dev %s",
		hTunnel->nMTU,
		g_aiccu->ipv6_interface);

	if (hTunnel->uses_tundev == 0)
	{
		aiccu_exec(
			"ip tunnel change %s ttl 64",
			g_aiccu->ipv6_interface);
	}
	else
	{
		/* Add a LinkLocal address for AYIYA tunnels */
		aiccu_exec(
			"ip -6 addr add %s/%u dev %s",
			hTunnel->sIPv6_LinkLocal,
			64,
			g_aiccu->ipv6_interface);
	}

	aiccu_exec(
		"ip -6 addr add %s/%u dev %s",
		hTunnel->sIPv6_Local,
		hTunnel->nIPv6_PrefixLength,
		g_aiccu->ipv6_interface);

	if (g_aiccu->defaultroute)
	{
		aiccu_exec(
			"ip -6 ro add %s via %s dev %s",
			"default",
			hTunnel->sIPv6_POP,
			g_aiccu->ipv6_interface);
	}

	return true;
}

void aiccu_os_reconfig(struct TIC_Tunnel *hTunnel)
{
	if (hTunnel->uses_tundev == 0)
	{
		aiccu_exec(
			"ip tunnel change %s local %s",
			g_aiccu->ipv6_interface,
			hTunnel->sIPv4_Local);
	}
}

void aiccu_os_delete(struct TIC_Tunnel *hTunnel)
{
	hTunnel = hTunnel;
	aiccu_exec(
		"ip link set %s down",
		g_aiccu->ipv6_interface);

	if (hTunnel->uses_tundev == 0)
	{
		aiccu_exec(
			"ip tunnel del %s",
			g_aiccu->ipv6_interface);
	}
}

