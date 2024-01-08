/*
 * bridgetools.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
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
 */
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <libbridge.h>

#ifdef HAVE_MICRO

int brctl_main(int argc, char **argv)
{
	if (argc == 1) {
		fprintf(stderr, "try to be professional!\n");
		return -1;
	}
	br_init();
	if (!strcmp(argv[1], "addif")) {
		if (ifexists(argv[3]))
			br_add_interface(argv[2], argv[3]);
	}
	if (!strcmp(argv[1], "delif")) {
		if (ifexists(argv[3]))
			br_del_interface(argv[2], argv[3]);
	}
	if (!strcmp(argv[1], "addbr")) {
		br_add_bridge(argv[2]);
	}
	if (!strcmp(argv[1], "stp")) {
		br_set_stp_state(argv[2], atoi(argv[3]));
	}
	if (!strcmp(argv[1], "delbr")) {
		if (!ifexists(argv[2]))
			return -1;
		br_del_bridge(argv[2]);
	}
	if (!strcmp(argv[1], "show")) {
		br_cmd_show();
	}
	br_shutdown();
}

int br_set_port_stp(const char *br, char *port, int on) // unsupported
{
	return br_set_filterbpdu(br, port, on);
}

#else

int br_set_port_hairpin(const char *br, char *port, int on)
{
	if (!ifexists(br))
		return -1;
	if (!ifexists(port))
		return -1;
	return eval("brctl", "hairpin", br, port, on ? "on" : "off");
}

#ifdef HAVE_MSTP

int br_set_port_stp(const char *br, char *port, int on) // unsupported
{
	if (!ifexists(br))
		return -1;

	if (on) {
		return eval("mstpctl", "setportbpdufilter", br, port, "no");
	} else {
		return eval("mstpctl", "setportbpdufilter", br, port, "yes");
	}

	return 0;
}

int br_set_stp_state(const char *br, int stp_state)
{
	if (!ifexists(br))
		return -1;
	if (stp_state) {
		// syslog (LOG_INFO, "stp is set to on\n");
		return eval("brctl", "stp", br, "on");
	} else {
		// syslog (LOG_INFO, "stp is set to off\n");
		return eval("brctl", "stp", br, "off");
	}
}

int br_set_path_cost(const char *br, const char *port, int cost)
{
	if (!ifexists(br))
		return -1;
	if (!ifexists(port))
		return -1;
	char scost[32];
	snprintf(scost, sizeof(scost), "%d", cost);
	return eval("mstpctl", "setportpathcost", br, port, scost);
}

int br_set_port_prio(const char *br, char *port, int prio)
{
	if (!ifexists(br))
		return -1;
	if (!ifexists(port))
		return -1;
	char sprio[32];
	snprintf(sprio, sizeof(sprio), "%d", (prio / 16));
	return eval("mstpctl", "settreeportprio", br, port, "0", sprio);
}

int br_set_bridge_prio(const char *br, int prio)
{
	if (!ifexists(br))
		return -1;
	char sprio[32];
	snprintf(sprio, sizeof(sprio), "%d", (prio / 4096));
	return eval("mstpctl", "settreeprio", br, "0", sprio);
}

int br_set_bridge_forward_delay(const char *br, int sec)
{
	if (!ifexists(br))
		return -1;
	char delay[32];
	sprintf(delay, "%d", sec);
	return eval("mstpctl", "setfdelay", br, delay);
}

int br_set_bridge_max_age(const char *br, int sec)
{
	if (!ifexists(br))
		return -1;
	char age[32];
	sprintf(age, "%d", sec);
	return eval("mstpctl", "setmaxage", br, age);
}
#else

int br_set_port_stp(const char *br, char *port, int on) // unsupported
{
	char set[32];
	sprintf(set, "%d", !on);
	return eval("brctl", "filterbpdu", br, port, set);
}

int br_set_bridge_forward_delay(const char *br, int sec)
{
	if (!ifexists(br))
		return -1;
	char delay[32];
	sprintf(delay, "%d", sec);
	return eval("brctl", "setfd", br, delay);
}

int br_set_bridge_max_age(const char *br, int sec)
{
	if (!ifexists(br))
		return -1;
	char age[32];
	sprintf(age, "%d", sec);
	return eval("brctl", "setmaxage", br, age);
}

int br_set_stp_state(const char *br, int stp_state)
{
	if (!ifexists(br))
		return -1;
	char set[32];
	sprintf(set, "%d", stp_state);
	return eval("brctl", "stp", br, set);
}

int br_set_path_cost(const char *br, const char *port, int cost)
{
	if (!ifexists(br))
		return -1;
	if (!ifexists(port))
		return -1;
	char scost[32];
	snprintf(scost, sizeof(scost), "%d", cost);
	return eval("brctl", "setpathcost", br, port, scost);
}

int br_set_port_prio(const char *br, char *port, int prio)
{
	if (!ifexists(br))
		return -1;
	if (!ifexists(port))
		return -1;
	char sprio[32];
	snprintf(sprio, sizeof(sprio), "%d", prio);
	return eval("brctl", "setportprio", br, port, sprio);
}

int br_set_bridge_prio(const char *br, int prio)
{
	if (!ifexists(br))
		return -1;
	char sprio[32];
	snprintf(sprio, sizeof(sprio), "%d", prio);
	return eval("brctl", "setbridgeprio", br, sprio);
}

#endif
int br_add_bridge(const char *brname)
{
	dd_loginfo("bridge", "bridge %s successfully added\n", brname);
	char ipaddr[32];
	char brmcast[32];
	char hwaddr[32];
	char tmp[256];

	sprintf(brmcast, "%s_mcast", brname);
	sprintf(ipaddr, "%s_ipaddr", brname);
	sprintf(hwaddr, "%s_hwaddr", brname);
	char netmask[32];

	sprintf(netmask, "%s_netmask", brname);
	int ret = eval("brctl", "addbr", brname);
	char *mcast = nvram_default_get(brmcast, "0");

#if 0 //def HAVE_80211AC
	sysprintf("echo 0 > /sys/devices/virtual/net/%s/bridge/multicast_snooping", brname);
	eval("emf", "add", "bridge", brname);
	if (!strcmp(mcast, "1"))
		eval("igs", "add", "bridge", brname);
#else
	sysprintf(
		"echo %s > /sys/devices/virtual/net/%s/bridge/multicast_snooping",
		mcast, brname);
#endif

	if (nvram_exists(ipaddr) && nvram_exists(netmask) &&
	    !nvram_match(ipaddr, "0.0.0.0") &&
	    !nvram_match(netmask, "0.0.0.0")) {
		eval("ifconfig", brname, nvram_safe_get(ipaddr), "netmask",
		     nvram_safe_get(netmask), "mtu", getBridgeMTU(brname, tmp),
		     "up");
	} else
		eval("ifconfig", brname, "mtu", getBridgeMTU(brname, tmp));

	if (strcmp(brname, "br0") && *(nvram_safe_get(hwaddr))) {
		set_hwaddr(brname, nvram_safe_get(hwaddr));
	} else {
		set_hwaddr(brname, nvram_safe_get("lan_hwaddr"));
	}

	return ret;
}

int br_del_bridge(const char *brname)
{
	if (!ifexists(brname))
		return -1;
	dd_loginfo("bridge", "bridge %s successfully deleted\n", brname);
	/* Stop the EMF for this LAN */
#if 0 //def HAVE_80211AC
	eval("emf", "stop", brname);
	/* Remove Bridge from igs */
	eval("igs", "del", "bridge", brname);
	eval("emf", "del", "bridge", brname);
#endif
	return eval("brctl", "delbr", brname);
}

int br_add_interface(const char *br, const char *dev)
{
	char eabuf[32];
	char tmp[256];

	if (!ifexists(dev))
		return -1;

	if (!ifexists(br))
		return -1;
	if (nvram_nmatch("apsta", "%s_mode", dev)) {
		fprintf(stderr, "skip %s is apsta\n", dev);
		return 0;
	}

	char ipaddr[32];
	char hwaddr[32];
	char netmask[32];
	char unicast[32];

	sprintf(ipaddr, "%s_ipaddr", dev);
	sprintf(netmask, "%s_netmask", dev);
	sprintf(hwaddr, "%s_hwaddr", dev);

	eval("ifconfig", dev, "0.0.0.0");
	if (strncmp(dev, "wl", 2) != 0) { // this is not an ethernet driver
		eval("ifconfig", dev, "down"); //fixup for some ethernet drivers
	}
	if (!nvram_match(hwaddr, ""))
		set_hwaddr(dev, nvram_safe_get(hwaddr));
	eval("ifconfig", dev, "mtu", getBridgeMTU(br, tmp));
	if (strncmp(dev, "wl", 2) != 0) { // this is not an ethernet driver
		eval("ifconfig", dev, "up");
	}

	dd_loginfo("bridge", "interface %s successfully added to bridge %s\n",
		   dev, br);
	int ret = eval("brctl", "addif", br, dev);
#ifdef HAVE_80211AC
//      eval("emf", "add", "iface", br, dev);
#endif
	if (strcmp(br, "br0") && *(nvram_nget("%s_hwaddr", br))) {
		get_hwaddr(br, eabuf);
		nvram_nset(eabuf, "%s_hwaddr", br); // safe for gui
		set_hwaddr(br, eabuf);
	}
	char *sep = NULL;
	char mainif[32];
	strncpy(mainif, dev, 31);
	if (!strncmp(dev, "wlan", 4) && (sep = strstr(mainif, ".sta"))) {
		*sep = 0;
		sysprintf(
			"echo %d > /sys/class/net/%s/brport/multicast_to_unicast",
			nvram_ngeti("%s_multicast_to_unicast", mainif), dev);
		if (nvram_nmatch("1", "%s_usteer", mainif))
			sysprintf(
				"echo 1 > /sys/class/net/%s/brport/multicast_to_unicast",
				dev);
	} else {
		sysprintf(
			"echo %d > /sys/class/net/%s/brport/multicast_to_unicast",
			nvram_ngeti("%s_multicast_to_unicast", dev), dev);
		if (nvram_nmatch("1", "%s_usteer", dev))
			sysprintf(
				"echo 1 > /sys/class/net/%s/brport/multicast_to_unicast",
				dev);
	}
	return ret;
}

int br_del_interface(const char *br, const char *dev)
{
	if (!ifexists(dev))
		return -1;
	dd_loginfo("bridge",
		   "interface %s successfully deleted from bridge %s\n", dev,
		   br);
	int ret = eval("brctl", "delif", br, dev);
#ifdef HAVE_80211AC
//      eval("emf", "del", "iface", br, dev);
#endif
	return ret;
}

#endif
