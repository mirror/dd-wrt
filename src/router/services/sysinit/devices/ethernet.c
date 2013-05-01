/*
 * ethernet.c
 *
 * Copyright (C) 2009 Sebastian Gottschall <gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 * 
 * detects ethernet adapters and loads the drivers
 */

static int try_module(char *module)
{
	sysprintf("insmod %s", module);
	return 1;
}

static int detect(char *devicename)
{
	FILE *tmp = fopen("/tmp/devices", "rb");

	if (tmp == NULL) {
		system2("/sbin/lspci>/tmp/devices");
	} else
		fclose(tmp);
	char devcall[128];
	int res;

	sprintf(devcall, "cat /tmp/devices|/bin/grep \"%s\"|/usr/bin/wc -l",
		devicename);
	FILE *in = popen(devcall, "rb");

	fscanf(in, "%d", &res);
	pclose(in);
	return res > 0 ? 1 : 0;
}

static int detect_ethernet_devices(void)
{
	int returncode = 0;
#ifndef HAVE_XSCALE
	if (detect("Rhine-"))	// VIA Rhine-I, Rhine-II, Rhine-III
		returncode = try_module("via-rhine");
	if (detect("VT6120"))	// VIA Rhine-I, Rhine-II, Rhine-III
		returncode = try_module("via-velocity");
	else if (detect("VT6121"))	// VIA Rhine-I, Rhine-II, Rhine-III
		returncode = try_module("via-velocity");
	else if (detect("VT6122"))	// VIA Rhine-I, Rhine-II, Rhine-III
		returncode = try_module("via-velocity");

	if (detect("DP8381"))
		returncode = try_module("natsemi");
	if (detect("DP83065"))
		returncode = try_module("cassini");
	if (detect("EG20T"))
		returncode = try_module("pch_gbe");
	if (detect("Rohm"))
		returncode = try_module("pch_gbe");
	if (detect("Cassini"))
		returncode = try_module("cassini");
	if (detect("PCnet32"))	// vmware?
		returncode = try_module("pcnet32");
	if (detect("Tigon3"))	// Broadcom 
		returncode = try_module("tg3");
	else if (detect("NetXtreme"))	// Broadcom 
		returncode = try_module("tg3");
	if (detect("NetXtreme II"))	// Broadcom 
		returncode = try_module("bnx2");
	if (detect("BCM44"))	// Broadcom 
		returncode = try_module("b44");

	if (detect("EtherExpress PRO/100"))	// intel 100 mbit 
		returncode = try_module("e100");
	else if (detect("PRO/100"))	// intel 100 mbit
		returncode = try_module("e100");
	else if (detect("8280"))	// intel 100 mbit 
		returncode = try_module("e100");
	else if (detect("Ethernet Pro 100"))	// intel 100 mbit 
		returncode = try_module("e100");
	else if (detect("8255"))	// intel 100 mbit 
		returncode = try_module("e100");
#endif
	if (detect("PRO/1000"))	// Intel Gigabit 
	{
		returncode = try_module("e1000");
		returncode = try_module("e1000e");
	} else if (detect("82541"))	// Intel Gigabit
	{
		returncode = try_module("e1000");
		returncode = try_module("e1000e");
	} else if (detect("82542"))	// Intel Gigabit
	{
		returncode = try_module("e1000");
		returncode = try_module("e1000e");
	} else if (detect("82543"))	// Intel Gigabit
	{
		returncode = try_module("e1000");
		returncode = try_module("e1000e");
	} else if (detect("82544"))	// Intel Gigabit
	{
		returncode = try_module("e1000");
		returncode = try_module("e1000e");
	} else if (detect("82545"))	// Intel Gigabit
	{
		returncode = try_module("e1000");
		returncode = try_module("e1000e");
	} else if (detect("82546"))	// Intel Gigabit
	{
		returncode = try_module("e1000");
		returncode = try_module("e1000e");
	} else if (detect("82547"))	// Intel Gigabit
	{
		returncode = try_module("e1000");
		returncode = try_module("e1000e");
	} else if (detect("82571"))	// Intel Gigabit 
	{
		returncode = try_module("e1000");
		returncode = try_module("e1000e");
	} else if (detect("82572"))	// Intel Gigabit 
	{
		returncode = try_module("e1000");
		returncode = try_module("e1000e");
	} else if (detect("82573"))	// Intel Gigabit 
	{
		returncode = try_module("e1000");
		returncode = try_module("e1000e");
	} else if (detect("82574"))	// Intel Gigabit 
	{
		returncode = try_module("e1000");
		returncode = try_module("e1000e");
	} else if (detect("82583"))	// Intel Gigabit 
	{
		returncode = try_module("e1000");
		returncode = try_module("e1000e");
	}
#ifndef HAVE_XSCALE
	if (detect("Tolapai"))	// Realtek 8169 Adapter (various notebooks) 
	{
		returncode = try_module("e1000");
		returncode = try_module("e1000e");
		returncode = try_module("e1000gcu");
		returncode = try_module("e1000gbe");
	} else if (detect("EP80579"))	// Realtek 8169 Adapter (various notebooks) 
	{
		returncode = try_module("e1000");
		returncode = try_module("e1000e");
		returncode = try_module("e1000gcu");
		returncode = try_module("e1000gbe");
	}
#endif
	if (detect("RTL-8110"))	// Realtek 8169 Adapter (various notebooks) 
		returncode = try_module("r8169");
	else if (detect("RTL-8111"))	// Realtek 8169 Adapter (various notebooks) 
		returncode = try_module("r8169");
	else if (detect("RTL8111"))	// Realtek 8169 Adapter (various notebooks) 
		returncode = try_module("r8169");
	else if (detect("RTL-8169"))	// Realtek 8169 Adapter (various
		// notebooks) 
		returncode = try_module("r8169");
	else if (detect("Linksys Gigabit"))
		returncode = try_module("r8169");
	else if (detect("RTL8101"))	// Realtek 8169 Adapter (various
		// notebooks) 
		returncode = try_module("r8169");

#ifndef HAVE_XSCALE
	if (detect("Happy Meal"))
		returncode = try_module("sunhme");

#endif
	if (detect("8139"))	// Realtek 8139 Adapter (various notebooks) 
		returncode = try_module("8139too");
	if (detect("DFE-690TXD"))	// Realtek 8139 Adapter (various
		// notebooks) 
		returncode = try_module("8139too");
	else if (detect("SMC2-1211TX"))	// Realtek 8139 Adapter (various
		// notebooks) 
		returncode = try_module("8139too");
	else if (detect("Robotics"))	// Realtek 8139 Adapter (various
		// notebooks) 
		returncode = try_module("8139too");
#ifndef HAVE_XSCALE

	if (detect("nForce2 Ethernet"))	// nForce2 
		returncode = try_module("forcedeth");
	else if (detect("nForce3 Ethernet"))	// nForce3 
		returncode = try_module("forcedeth");
	else if (detect("nForce Ethernet"))	// nForce 
		returncode = try_module("forcedeth");
	else if (detect("CK804 Ethernet"))	// nForce
		returncode = try_module("forcedeth");
	else if (detect("CK8S Ethernet"))	// nForce
		returncode = try_module("forcedeth");
	else if (detect("MCP04 Ethernet"))	// nForce
		returncode = try_module("forcedeth");
	else if (detect("MCP2A Ethernet"))	// nForce
		returncode = try_module("forcedeth");
	else if (detect("MCP51 Ethernet"))	// nForce
		returncode = try_module("forcedeth");
	else if (detect("MCP55 Ethernet"))	// nForce
		returncode = try_module("forcedeth");
	else if (detect("MCP61 Ethernet"))	// nForce
		returncode = try_module("forcedeth");
	else if (detect("MCP65 Ethernet"))	// nForce
		returncode = try_module("forcedeth");
	else if (detect("MCP67 Ethernet"))	// nForce
		returncode = try_module("forcedeth");
	else if (detect("MCP67 Gigabit"))	// nForce
		returncode = try_module("forcedeth");
	else if (detect("MCP73 Ethernet"))	// nForce
		returncode = try_module("forcedeth");
	else if (detect("MCP77 Ethernet"))	// nForce
		returncode = try_module("forcedeth");
	else if (detect("MCP79 Ethernet"))	// nForce
		returncode = try_module("forcedeth");

	if (detect("Sundance"))	// Dlink fibre
		returncode = try_module("sundance");
	else if (detect("DL10050"))
		returncode = try_module("sundance");

	if (detect("88E8001"))	// Marvell Yukon
		returncode = try_module("sk98lin");
	else if (detect("RDK-"))
		returncode = try_module("sk98lin");
	else if (detect("SK-98"))
		returncode = try_module("sk98lin");
	else if (detect("3c940"))
		returncode = try_module("sk98lin");
	else if (detect("Marvell"))
		returncode = try_module("sk98lin");

	if (detect("RTL-8029"))	// Old Realtek PCI NE2000 clone (10M only)
	{
		returncode = try_module("8390");
		returncode = try_module("ne2k-pci");
	}

	if (detect("3c905"))	// 3Com
		returncode = try_module("3c59x");
	else if (detect("3c555"))	// 3Com
		returncode = try_module("3c59x");
	else if (detect("3c556"))	// 3Com
		returncode = try_module("3c59x");
	else if (detect("ScSOHO100"))	// 3Com
		returncode = try_module("3c59x");
	else if (detect("Hurricane"))	// 3Com
		returncode = try_module("3c59x");

	if (detect("LNE100TX"))	// liteon / linksys
		returncode = try_module("tulip");
	else if (detect("FasterNet"))
		returncode = try_module("tulip");
	else if (detect("ADMtek NC100"))
		returncode = try_module("tulip");
	else if (detect("910-A1"))
		returncode = try_module("tulip");
	else if (detect("tulip"))
		returncode = try_module("tulip");
	else if (detect("DECchip 21142"))
		returncode = try_module("tulip");
	else if (detect("MX987x5"))
		returncode = try_module("tulip");

	if (detect("DGE-530T"))
		returncode = try_module("skge");
	else if (detect("D-Link Gigabit"))
		returncode = try_module("skge");

	if (detect("SiS900"))	// Sis 900
		returncode = try_module("sis900");

	if (detect("SafeXcel-1141")) {
		try_module("ocf");
		try_module("cryptodev");
		try_module("safe");
		nvram_set("use_crypto", "1");
	} else
		nvram_set("use_crypto", "0");
#endif
	return returncode;
}
