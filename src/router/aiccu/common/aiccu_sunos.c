/**********************************************************
 SixXS - Automatic IPv6 Connectivity Configuration Utility
***********************************************************
 Copyright 2003-2005 SixXS - http://www.sixxs.net
***********************************************************
 common/aiccu_sunos.c - Sun Solaris / SunOS

 ipv6_interface has to be eg ip.tun0
***********************************************************
 $Author: jeroen $
 $Id: aiccu_sunos.c,v 1.3 2006-07-23 14:13:57 jeroen Exp $
 $Date: 2006-07-23 14:13:57 $
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
			"/sbin/ifconfig %s inet6 plumb tsrc %s tdst %s up",
			g_aiccu->ipv6_interface,
			strcmp(hTunnel->sIPv4_Local, "heartbeat") == 0 ? "0.0.0.0" : hTunnel->sIPv4_Local,
			hTunnel->sIPv4_POP);

		aiccu_exec(
			"ifconfig %s inet6 addif %s %s up",
			g_aiccu->ipv6_interface,
			hTunnel->sIPv6_Local,
			hTunnel->sIPv6_POP);
	}
	else
	{
		dolog(LOG_DEBUG, "There is no Solaris support for tun-devices yet");
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
		aiccu_exec(
			"/sbin/ifconfig %s inet6 plumb tsrc %s tdst %s up",
			g_aiccu->ipv6_interface,
			strcmp(hTunnel->sIPv4_Local, "heartbeat") == 0 ? "0.0.0.0" : hTunnel->sIPv4_Local,
			hTunnel->sIPv4_POP);
	}
}

void aiccu_os_delete(struct TIC_Tunnel *hTunnel)
{
	hTunnel = hTunnel;
	aiccu_exec(
		"ifconfig %s down",
		g_aiccu->ipv6_interface);
}

