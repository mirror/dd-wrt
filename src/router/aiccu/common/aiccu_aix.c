/**********************************************************
 SixXS - Automatic IPv6 Connectivity Configuration Utility
***********************************************************
 Copyright 2003-2005 SixXS - http://www.sixxs.net
***********************************************************
 common/aiccu_aix.c - AIX

 ipv6_interface has to be eg cti0
***********************************************************
 $Author: jeroen $
 $Id: aiccu_aix.c,v 1.3 2006-07-23 14:13:57 jeroen Exp $
 $Date: 2006-07-23 14:13:57 $
**********************************************************/

#include "aiccu.h"

bool aiccu_os_install()
{
	/* Define the CTI (Configured Tunnel Interface) by executing the deftunnel configuration method */
	return aiccu_exec("/usr/lib/methods/deftunnel -c if -s CTI -t cti");
}

bool aiccu_os_setup(struct TIC_Tunnel *hTunnel)
{
	if (hTunnel->uses_tundev == 0)
	{
		/* Build a normal SIT tunnel */
		aiccu_exec(
			"/usr/sbin/ifconfig %s inet6 ::%s/128 ::%s",
			g_aiccu->ipv6_interface,
			strcmp(hTunnel->sIPv4_Local, "heartbeat") == 0 ? "0.0.0.0" : hTunnel->sIPv4_Local,
			hTunnel->sIPv4_POP);

		/* Remove the local endpoint, the remote stays though :) */
		aiccu_exec(
			"/usr/sbin/ifconfig %s inet6 ::%s delete",
			g_aiccu->ipv6_interface,
			strcmp(hTunnel->sIPv4_Local, "heartbeat") == 0 ? "0.0.0.0" : hTunnel->sIPv4_Local);

		/* Add the addresses */
		aiccu_exec(
			"ifconfig %s inet6 %s %s",
			g_aiccu->ipv6_interface,
			hTunnel->sIPv6_Local,
			hTunnel->sIPv6_POP);
	}
	else
	{
		dolog(LOG_DEBUG, "There is no AIX support for tun-devices yet");
		exit(-1);
	}

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
		/* Build a normal SIT tunnel */
		aiccu_exec(
			"/usr/sbin/ifconfig %s inet6 ::%s/128 ::%s",
			g_aiccu->ipv6_interface,
			strcmp(hTunnel->sIPv4_Local, "heartbeat") == 0 ? "0.0.0.0" : hTunnel->sIPv4_Local,
			hTunnel->sIPv4_POP);

		/* Remove the local endpoint, the remote stays */
		aiccu_exec(
			"/usr/sbin/ifconfig %s inet6 ::%s delete",
			g_aiccu->ipv6_interface,
			strcmp(hTunnel->sIPv4_Local, "heartbeat") == 0 ? "0.0.0.0" : hTunnel->sIPv4_Local);
	}
}

void aiccu_os_delete(struct TIC_Tunnel *hTunnel)
{
	hTunnel = hTunnel;
	aiccu_exec(
		"ifconfig %s down",
		g_aiccu->ipv6_interface);
}

