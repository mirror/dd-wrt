#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <net/if.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>
#include <syslog.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <resolv.h>
#include <signal.h>

#include <utils.h>
#include <wlutils.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <cy_conf.h>
#include <code_pattern.h>
#include <bcmdevs.h>
#include <net/route.h>
#include <cy_conf.h>
#include <linux/if_ether.h>
// #include <linux/mii.h>
#include <linux/sockios.h>
#include <cymac.h>
#include <broadcom.h>
#include <md5.h>

#define SIOCGMIIREG	0x8948	/* Read MII PHY register.  */
#define SIOCSMIIREG	0x8949	/* Write MII PHY register.  */

void getLANMac(char *newmac)
{
	if (getRouterBrand() == ROUTER_ASUS_AC87U) {
		strcpy(newmac, nvram_safe_get("et1macaddr"));
	} else {
		strcpy(newmac, nvram_safe_get("et0macaddr"));
	}
#ifndef HAVE_BUFFALO

	if (nvram_match("port_swap", "1")) {
		if (strlen(nvram_safe_get("et1macaddr")) != 0)	// safe:
			// maybe
			// et1macaddr 
			// not there?
		{
			strcpy(newmac, nvram_safe_get("et1macaddr"));
		} else {
			MAC_ADD(newmac);	// et0macaddr +1
		}
	}
#endif
	return;
}

void getWirelessMac(char *newmac, int instance)
{
#ifdef HAVE_BUFFALO
#ifdef HAVE_BCMMODERN
	strcpy(newmac, nvram_safe_get("et0macaddr"));
#else
	strcpy(newmac, nvram_safe_get("il0macaddr"));
#endif
#else
	if (instance < 0)
		instance = 0;
	// if (strlen(nvram_safe_get ("il0macaddr")) != 0)
	// {
	// strcpy (newmac, nvram_safe_get ("il0macaddr"));
	// }
	// else
	{
		if (nvram_match("port_swap", "1")) {
			if (strlen(nvram_safe_get("et1macaddr")) != 0)	// safe:
				// maybe
				// et1macaddr 
				// not there?
			{
				strcpy(newmac, nvram_safe_get("et1macaddr"));
				MAC_ADD(newmac);	// et1macaddr +2
				MAC_ADD(newmac);
			} else {
				strcpy(newmac, nvram_safe_get("et0macaddr"));
				MAC_ADD(newmac);	// et0macaddr +3
				MAC_ADD(newmac);
				MAC_ADD(newmac);
			}
		} else {

			if (getRouterBrand() == ROUTER_ASUS_AC87U) {
				strcpy(newmac, nvram_safe_get("et1macaddr"));
				MAC_ADD(newmac);	// et0macaddr +2
				MAC_ADD(newmac);
				if (instance)
					MAC_ADD(newmac);
			} else if (getRouterBrand() != ROUTER_ASUS_AC66U) {
				char *et0 = nvram_get("et0macaddr");
				strcpy(newmac, nvram_safe_get("et0macaddr"));
				MAC_ADD(newmac);	// et0macaddr +2
				MAC_ADD(newmac);
				if (instance)
					MAC_ADD(newmac);
			} else {
				switch (instance) {
				case 0:
					strcpy(newmac, nvram_safe_get("pci/1/1/macaddr"));
					break;
				case 1:
					strcpy(newmac, nvram_safe_get("pci/2/1/macaddr"));
					break;

				}

			}

		}
	}
#endif
	return;
}

void getWANMac(char *newmac)
{
// This should be done for more routers that have a true wan-ethernet port with an own mac-adddress
#if defined(HAVE_HORNET)
	struct ifreq ifr;
	int s;

	if (nvram_get("wan_ifname")) {
		if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW))) {
			char eabuf[32];

			strncpy(ifr.ifr_name, nvram_safe_get("wan_ifname"), IFNAMSIZ);
			ioctl(s, SIOCGIFHWADDR, &ifr);
			strcpy(newmac, ether_etoa((unsigned char *)ifr.ifr_hwaddr.sa_data, eabuf));
			close(s);
		}
		cprintf("getWANMAC returns %s from %s\n", newmac, nvram_safe_get("wan_ifname"));
		return;
	} else {
		strcpy(newmac, nvram_safe_get("et0macaddr"));
		MAC_ADD(newmac);	// et0macaddr +1
		return;
	}
#endif
	if (getRouterBrand() == ROUTER_ASUS_AC87U) {
		strcpy(newmac, nvram_safe_get("et1macaddr"));
	} else {
		strcpy(newmac, nvram_safe_get("et0macaddr"));
	}
#if !defined(HAVE_BUFFALO) && !defined(HAVE_WZRG300NH) && !defined(HAVE_WHRHPGN)
	if (getRouterBrand() != ROUTER_ASUS_AC66U) {
		if (nvram_invmatch("wan_proto", "disabled")) {
			MAC_ADD(newmac);	// et0macaddr +1

			if (nvram_match("port_swap", "1")) {
				if (strlen(nvram_safe_get("et1macaddr")) != 0)	// safe:
					// maybe
					// et1macaddr 
					// not there?
				{
					strcpy(newmac, nvram_safe_get("et1macaddr"));
					MAC_ADD(newmac);	// et1macaddr +1 
				} else {
					MAC_ADD(newmac);	// et0macaddr +2
				}
			}
		}
	}
#endif
	return;
}
