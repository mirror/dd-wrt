/**********************************************************
 SixXS - Automatic IPv6 Connectivity Configuration Utility
***********************************************************
 Copyright 2003-2005 SixXS - http://www.sixxs.net
***********************************************************
 common/tun.c - Tunnel Device Handling
***********************************************************
 $Author: jeroen $
 $Id: tun.c,v 1.14 2007-01-11 00:29:18 jeroen Exp $
 $Date: 2007-01-11 00:29:18 $
**********************************************************/

#include <sys/uio.h>
#include "tun.h"
#include "aiccu.h"

/* The tun/tap device HANDLE */
#ifndef _WIN32
#ifndef ETH_P_IPV6
#define ETH_P_IPV6	0x86DD		/* IPv6 over bluebook		*/
#endif
int			tun_fd;

/*
 * HAS_IFHEAD -> Tunnel Device produces packets with a tun_pi in the front
 * NEED_IFHEAD -> Tunnel Device produces packets with a tun_pi in the front, but it is not active per default
 */ 

#else
HANDLE			device_handle = INVALID_HANDLE_VALUE;
#define ETH_P_IPV6 0x86dd
#define ETH_ALEN 6
struct ether_header
{
	uint8_t		ether_dhost[ETH_ALEN];	/* destination eth addr */
	uint8_t		ether_shost[ETH_ALEN];	/* source ether addr    */
	uint16_t	ether_type;		/* packet type ID field */
};

/* Tap device constants which we use */
#define TAP_CONTROL_CODE(request,method) CTL_CODE(FILE_DEVICE_UNKNOWN, request, method, FILE_ANY_ACCESS)
#define TAP_IOCTL_GET_VERSION           TAP_CONTROL_CODE(2, METHOD_BUFFERED)
#define TAP_IOCTL_CONFIG_POINT_TO_POINT TAP_CONTROL_CODE(5, METHOD_BUFFERED)
#define TAP_IOCTL_SET_MEDIA_STATUS      TAP_CONTROL_CODE(6, METHOD_BUFFERED)
#define TAP_REGISTRY_KEY		"SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}"
#define TAP_ADAPTER_KEY			"SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}"
#define TAP_DEVICE_DIR			"\\\\.\\Global\\"
#define TAP_WIN32_MIN_MAJOR		8
#define TAP_WIN32_MIN_MINOR		1
#define TAP_COMPONENT_ID1		"tap0801"	/* Original Tun/Tap driver ID */
#define TAP_COMPONENT_ID2		"tap0802"	/* Windows Vista marked 801 as broken, thus use another ID */

#endif

void tun_log(int level, const char *what, const char *fmt, ...);
void tun_log(int level, const char *what, const char *fmt, ...)
{
	char	buf[1024];
	va_list ap;

        /* Clear them just in case */
        memset(buf, 0, sizeof(buf));

	snprintf(buf, sizeof(buf), "[tun-%s] ", what);

	/* Print the log message behind it */
	va_start(ap, fmt);
	vsnprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), fmt, ap);
	va_end(ap);

	/* Actually Log it */
	dolog(level, buf);
}

static const char reader_name[] = "tundev->tun";
static const char writer_name[] = "tun->tundev";

#ifdef _WIN32
/* Windows doesn't have writev() but does have WSASend */
int writev(SOCKET sock, const struct iovec *vector, DWORD count)
{
	DWORD sent;
	WSASend(sock, (LPWSABUF)vector, count, &sent, 0, NULL, NULL);
	return sent;
}

uint16_t inchksum(const void *data, uint32_t length);
uint16_t inchksum(const void *data, uint32_t length)
{
        register long           sum = 0;
        register const uint16_t *wrd = (const uint16_t *)data;
        register long           slen = (long)length;

        while (slen >= 2)
        {
                sum += *wrd++;
                slen-=2;
        }

        if (slen > 0) sum+=*(const uint8_t *)wrd;

        while (sum >> 16) sum = (sum & 0xffff) + (sum >> 16);

        return (uint16_t)sum;
}

uint16_t ipv6_checksum(const struct ip6_hdr *ip6, uint8_t protocol, const void *data, const uint16_t length);
uint16_t ipv6_checksum(const struct ip6_hdr *ip6, uint8_t protocol, const void *data, const uint16_t length)
{
        struct
        {
                uint16_t        length;
                uint16_t        zero1;
                uint8_t         zero2;
                uint8_t         next;
        } pseudo;
        register uint32_t       chksum = 0;

        pseudo.length   = htons(length);
        pseudo.zero1    = 0;
        pseudo.zero2    = 0;
        pseudo.next     = protocol;

        /* IPv6 Source + Dest */
        chksum  = inchksum(&ip6->ip6_src, sizeof(ip6->ip6_src) + sizeof(ip6->ip6_dst));
        chksum += inchksum(&pseudo, sizeof(pseudo));
        chksum += inchksum(data, length);

        /* Wrap in the carries to reduce chksum to 16 bits. */
        chksum  = (chksum >> 16) + (chksum & 0xffff);
        chksum += (chksum >> 16);

        /* Take ones-complement and replace 0 with 0xFFFF. */
        chksum = (uint16_t) ~chksum;
        if (chksum == 0UL) chksum = 0xffffUL;
        return (uint16_t)chksum;
}
#endif

/*
 * Tun -> Socket
 *
 * Needs to be started in a separate thread
 * This gets done by tun_start()
 *
 */
#ifndef _WIN32
void *tun_reader(void *arg);
void *tun_reader(void *arg)
#else
DWORD WINAPI tun_reader(LPVOID arg);
DWORD WINAPI tun_reader(LPVOID arg)
#endif
{
	unsigned char		buf[2048];

	/* The function that actually does something with the buffer */
	struct tun_reader	*tun = (struct tun_reader *)arg;

#ifdef _WIN32
	DWORD			n, lenin;
	OVERLAPPED		overlapped;
	unsigned int		errcount = 0;

	struct nd_sol
	{
		struct ip6_hdr			ip;
		struct icmp6_hdr		icmp;
		struct nd_neighbor_solicit	sol;
	} *solic = (struct nd_sol *)&buf[sizeof(struct ether)];

	struct nd_adv
	{
		struct ip6_hdr			ip;
		struct icmp6_hdr		icmp;
		struct nd_neighbor_advert	adv;
	} advert;

	/* Create an event for overlapped results */
	overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
#else
	ssize_t			n;
#endif

	/* Forever */
	while (true)
	{
#ifndef _WIN32	
		n = read(tun_fd, buf, sizeof(buf));
		if (n <= 0)
		{
			/* Only report issues when the tunnel is actually up and running */
			if (g_aiccu->tunrunning) tun_log(LOG_ERR, reader_name, "Read error on Tun Device: %s (%d)\n", strerror(errno), errno);
			continue;
		}

#if defined(NEED_IFHEAD) || defined(HAS_IFHEAD)
		/* get the tun_pi struct out of there */
		memmove(&buf, &buf[4], n-4);
		n-=4;
#endif

		tun->function((char *)buf, (unsigned int)n);
#else /* Windows */
		overlapped.Offset = 0;
		overlapped.OffsetHigh = 0;

		memset(buf,0,sizeof(buf));
		n = ReadFile(device_handle, buf, sizeof(buf), &lenin, &overlapped);
		if (!n)
		{
			while (!n && GetLastError() == ERROR_IO_PENDING)
			{
				if (WaitForSingleObject(overlapped.hEvent, 20000) == WAIT_OBJECT_0)
				{
					n = GetOverlappedResult(device_handle, &overlapped, &lenin, FALSE);
				}
			}

			if (!n)
			{
				tun_log(LOG_ERR, reader_name, "Error reading from device: %u, %s (%d)\n", GetLastError(), strerror(errno), errno);
				errcount++;
				if (errcount > 10) break;
				continue;
			}
		}

		/* Check for neighbour discovery packets (ICMPv6, ND_SOL, hop=255)
		 * (XXX: doesn't check for a chain, but ND is usually without)
		 */
		if (	solic->ip.ip6_ctlun.ip6_un1.ip6_un1_nxt == IPPROTO_ICMPV6 &&
			solic->icmp.icmp6_type == ND_NEIGHBOR_SOLICIT &&
			solic->ip.ip6_ctlun.ip6_un1.ip6_un1_hlim == 255)
		{
			/* Ignore unspecified ND's as they are used for DAD */
			if (IN6_IS_ADDR_UNSPECIFIED(&solic->ip.ip6_src)) continue;

			/* Create our reply */
			memset(&advert, 0, sizeof(advert));
			advert.ip.ip6_ctlun.ip6_un2_vfc = 6 << 4;
			advert.ip.ip6_ctlun.ip6_un1.ip6_un1_flow = solic->ip.ip6_ctlun.ip6_un1.ip6_un1_flow;
			advert.ip.ip6_ctlun.ip6_un1.ip6_un1_plen = htons(sizeof(advert.icmp) + sizeof(advert.adv));
			advert.ip.ip6_ctlun.ip6_un1.ip6_un1_nxt = IPPROTO_ICMPV6;
			advert.ip.ip6_ctlun.ip6_un1.ip6_un1_hlim = 255;

			/* Swap src/dst */
			memcpy(&advert.ip.ip6_src, &solic->sol.nd_ns_target, sizeof(advert.ip.ip6_src));
			memcpy(&advert.ip.ip6_dst, &solic->ip.ip6_src, sizeof(advert.ip.ip6_dst));

			/* ICMP Neighbour Advertisement */
			advert.icmp.icmp6_type = ND_NEIGHBOR_ADVERT;
			advert.icmp.icmp6_code = 0;
			advert.icmp.icmp6_dataun.icmp6_un_data8[0] = 0xe0;
			memcpy(&advert.adv.nd_na_target, &solic->sol.nd_ns_target, sizeof(advert.adv.nd_na_target));
			/* Fake MAC address */
			advert.adv.nd_no_type = 2;
			advert.adv.nd_no_len = 1;
			advert.adv.nd_no_mac[0] = 0x00;
			advert.adv.nd_no_mac[1] = 0xff;
			advert.adv.nd_no_mac[2] = 0x25;
			advert.adv.nd_no_mac[3] = 0x02;
			advert.adv.nd_no_mac[4] = 0x19;
			advert.adv.nd_no_mac[5] = 0x78;

			/* ICMP has a checksum */
			advert.icmp.icmp6_cksum = ipv6_checksum(&advert.ip, IPPROTO_ICMPV6, (uint8_t *)&advert.icmp, sizeof(advert.icmp) + sizeof(advert.adv));

			/* We'll need to answer this back to the TAP device */
			tun_write((char *)&advert, (unsigned int)sizeof(advert));
			continue;
		}
		tun->function((char *)&buf[sizeof(struct ether)], (unsigned int)lenin - sizeof(struct ether));
#endif
	}

	D(dolog(LOG_DEBUG, "TUN Reader stopping\n"));
#ifndef _WIN32
	return NULL;
#else
	return 0;
#endif
}

/* Socket -> Tun */
void tun_write(char *buf, unsigned int length)
{
	unsigned int	c = 0;
#ifndef _WIN32
#ifdef linux
	struct iovec	dat[2];
	struct tun_pi   pi;
	memset(&pi, 0, sizeof(pi));

	pi.proto = htons(ETH_P_IPV6);

	dat[0].iov_base = &pi;
	dat[0].iov_len	= sizeof(pi);
	dat[1].iov_base = buf;
	dat[1].iov_len	= length;

	length += sizeof(pi);

	/* Forward the packet to the kernel */
	c = writev(tun_fd, dat, 2);

#else /* *BSD/Darwin */

	uint32_t	type = htonl(AF_INET6);
	struct iovec	dat[2];

	dat[0].iov_base = (void *)&type;
	dat[0].iov_len	= sizeof(type);
	dat[1].iov_base = buf;
	dat[1].iov_len	= length;

	length += sizeof(type);

	/* Forward the packet to the kernel */
	c = writev(tun_fd, dat, 2);

#endif

	if (c != length)
	{
		tun_log(LOG_ERR, writer_name, "Error while writing to TUN: %u != %u\n", c, length);
	}

#else /* Windows */
	DWORD		n, lenout;
	OVERLAPPED	overlapped;
	unsigned char	mbuf[4096];

	struct ether *eth = (struct ether *)mbuf;

	/* Sent the packet outbound */
	overlapped.Offset = 0;
	overlapped.OffsetHigh = 0;
	overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	memset(mbuf,0,sizeof(mbuf));
	eth->ether_dhost[0] = htons(0x3333);
	eth->ether_dhost[1] = htons(0xff00);
	eth->ether_dhost[2] = htons(0x0002);
	eth->ether_shost[0] = htons(0x00ff);
	eth->ether_shost[1] = htons(0x5342);
	eth->ether_shost[2] = htons(0x2768);
	eth->ether_type = htons(ETH_P_IPV6);
	memcpy(&mbuf[sizeof(*eth)],buf,length);

	n = WriteFile(device_handle, mbuf, sizeof(*eth)+length, &lenout, &overlapped);
	if (!n && GetLastError() == ERROR_IO_PENDING)
	{
		WaitForSingleObject(overlapped.hEvent, INFINITE);
		n = GetOverlappedResult(device_handle, &overlapped, &lenout, FALSE);
	}

	if (!n)
	{
		tun_log(LOG_ERR, writer_name, "Error writing to device: %u, %s (%d)\n", GetLastError(), strerror(errno), errno);
	}
#endif
}

#ifdef _WIN32

struct tap_reg
{
	char			*guid;
	struct tap_reg		*next;
};

struct panel_reg
{
	char			*name;
	char			*guid;
	struct panel_reg	*next;
};

/* Get a working tunnel adapter */
struct tap_reg *get_tap_reg(void)
{
	HKEY		adapter_key;
	LONG		status;
	DWORD		len;
	struct tap_reg	*first = NULL;
	struct tap_reg	*last = NULL;
	int		i = 0;

	status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TAP_ADAPTER_KEY, 0, KEY_READ, &adapter_key);
	if (status != ERROR_SUCCESS)
	{
		dolog(LOG_ERR, "Error opening registry key: %s\n", TAP_ADAPTER_KEY);
		return NULL;
	}

	while (true)
	{
		char	enum_name[256];
		char	unit_string[256];
		HKEY	unit_key;
		char	component_id_string[] = "ComponentId";
		char	component_id[256];
		char	net_cfg_instance_id_string[] = "NetCfgInstanceId";
		char	net_cfg_instance_id[256];
		DWORD	data_type;

		len = sizeof(enum_name);
		status = RegEnumKeyEx(adapter_key, i, enum_name, &len, NULL, NULL, NULL, NULL);
		if (status == ERROR_NO_MORE_ITEMS) break;
		else if (status != ERROR_SUCCESS)
		{
			dolog(LOG_ERR, "Error enumerating registry subkeys of key: %s (t0)\n", TAP_ADAPTER_KEY);
			break;
		}

		snprintf(unit_string, sizeof(unit_string), "%s\\%s", TAP_ADAPTER_KEY, enum_name);
		status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, unit_string, 0, KEY_READ, &unit_key);
		if (status != ERROR_SUCCESS)
		{
			dolog(LOG_WARNING, "Error opening registry key: %s (t1)\n", unit_string);
		}
		else
		{
			len = sizeof(component_id);
			status = RegQueryValueEx(unit_key, component_id_string,	NULL, &data_type, (LPBYTE)component_id, &len);
			if (status != ERROR_SUCCESS || data_type != REG_SZ)
			{
				dolog(LOG_WARNING, "Error opening registry key: %s\\%s (t2)\n", unit_string, component_id_string);
			}
			else
			{
				len = sizeof(net_cfg_instance_id);
				status = RegQueryValueEx(unit_key, net_cfg_instance_id_string, NULL, &data_type, (LPBYTE)net_cfg_instance_id, &len);
				if (status == ERROR_SUCCESS && data_type == REG_SZ)
				{
					if (	strcmp(component_id, TAP_COMPONENT_ID1) == 0 ||
						strcmp(component_id, TAP_COMPONENT_ID2) == 0)
					{
						struct tap_reg *reg = (struct tap_reg *)malloc(sizeof(*reg));
						memset(reg, 0, sizeof(*reg));
						reg->guid = strdup(net_cfg_instance_id);

						if (!first) first = reg;
						if (last) last->next = reg;
						last = reg;
					}
				}
			}

			RegCloseKey(unit_key);
		}
		i++;
	}

	RegCloseKey(adapter_key);
	return first;
}

void free_tap_reg(struct tap_reg *tap_reg)
{
	struct tap_reg *tr, *tr1;

	for (tr = tap_reg; tr != NULL; tr = tr1)
	{
		tr1 = tr->next;
		free(tr->guid);
		free(tr);
	}
}

void free_panel_reg(struct panel_reg *panel_reg)
{
	struct panel_reg *pr, *pr1;

	for (pr = panel_reg; pr != NULL; pr = pr1)
	{
		pr1 = pr->next;
		free(pr->guid);
		free(pr->name);
		free(pr);
	}
}


/* Collect GUID's and names of all the Connections that are available */
struct panel_reg *get_panel_reg(void)
{
	LONG			status;
	HKEY			network_connections_key;
	DWORD			len;
	struct panel_reg	*first = NULL;
	struct panel_reg	*last = NULL;
	int			i = 0;

	status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TAP_REGISTRY_KEY, 0, KEY_READ, &network_connections_key);

	if (status != ERROR_SUCCESS)
	{
		dolog(LOG_ERR, "Error opening registry key: %s (p0)\n", TAP_REGISTRY_KEY);
		return NULL;
	}

	while (true)
	{
		char		enum_name[256];
		char		connection_string[256];
		HKEY		connection_key;
		char		name_data[256];
		DWORD		name_type;
		const char	name_string[] = "Name";

		len = sizeof(enum_name);
		status = RegEnumKeyEx(network_connections_key, i, enum_name, &len, NULL, NULL, NULL, NULL);
		if (status == ERROR_NO_MORE_ITEMS) break;
		else if (status != ERROR_SUCCESS)
		{
			dolog(LOG_ERR, "Error enumerating registry subkeys of key: %s (p1)\n", TAP_REGISTRY_KEY);
			break;
		}

		i++;

		if (enum_name[0] != '{') continue;

		snprintf(connection_string, sizeof(connection_string), "%s\\%s\\Connection", TAP_REGISTRY_KEY, enum_name);

		status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, connection_string, 0,	KEY_READ, &connection_key);
		if (status != ERROR_SUCCESS)
		{
			dolog(LOG_WARNING, "Error opening registry key: %s (p2)\n", connection_string);
		}
		else
		{
			len = sizeof(name_data);
			status = RegQueryValueEx(connection_key, name_string, NULL, &name_type,	(LPBYTE)name_data, &len);

			if (status != ERROR_SUCCESS || name_type != REG_SZ)
			{
				dolog(LOG_WARNING, "Error opening registry key: %s\\%s\\%s (p3)\n", TAP_REGISTRY_KEY, (LPBYTE)connection_string, name_string);
			}
			else
			{
				struct panel_reg *reg = (struct panel_reg *)malloc(sizeof(*reg));
				memset(reg, 0, sizeof(*reg));
				reg->name = strdup(name_data);
				reg->guid = strdup(enum_name);

				/* link into return list */
				if (!first) first = reg;
				if (last) last->next = reg;
				last = reg;
			}

			RegCloseKey(connection_key);
		}
	}

	RegCloseKey(network_connections_key);

	return first;
}

void tun_list_tap_adapters(void)
{
	int			links;
	struct tap_reg		*tap_reg = get_tap_reg(), *tr, *tr1;
	struct panel_reg	*panel_reg = get_panel_reg(), *pr;

	dolog(LOG_INFO, "Available TAP-WIN32 adapters [name, GUID]:\n");

	/* loop through each TAP-Win32 adapter registry entry */
	for (tr = tap_reg; tr != NULL; tr = tr->next)
	{
		links = 0;

		/* loop through each network connections entry in the control panel */
		for (pr = panel_reg; pr != NULL; pr = pr->next)
		{
			if (strcmp(tr->guid, pr->guid) == 0)
			{
				dolog(LOG_INFO, "'%s' %s\n", pr->name, tr->guid);
				links++;
			}
		}

		if (links > 1)
		{
			dolog(LOG_WARNING, "*** Adapter with GUID %s has %u links from the Network Connections control panel, it should only be 1\n", tr->guid, links);
		}
		else if (links == 0)
		{
			dolog(LOG_WARNING, "[NULL] %s\n", tr->guid);
			dolog(LOG_WARNING, "*** Adapter with GUID %s doesn't have a link from the control panel\n", tr->guid);
		}

		/* check for TAP-Win32 adapter duplicated GUIDs */
		for (tr1 = tap_reg; tr1 != NULL; tr1 = tr1->next)
		{
			if (tr != tr1 && strcmp(tr->guid, tr1->guid) == 0)
			{
				dolog(LOG_WARNING, "*** Duplicate Adapter GUID %s\n", tr->guid);
			}
		}
	}

	free_tap_reg(tap_reg);
	free_panel_reg(panel_reg);
}

bool tun_fixup_adapters(void)
{
	int			links, count = 0, found = 0;
	struct tap_reg		*tap_reg = get_tap_reg(), *tr = NULL, *tr1 = NULL;
	struct panel_reg	*panel_reg = get_panel_reg(), *pr = NULL, *first = NULL, *prf = NULL;
	bool			ok;

	/* loop through each TAP-Win32 adapter registry entry */
	for (tr = tap_reg; tr != NULL; tr = tr->next)
	{
		links = 0;
		ok = true;

		/* loop through each network connections entry in the control panel */
		for (pr = panel_reg; pr != NULL; pr = pr->next)
		{
			if (strcmp(tr->guid, pr->guid) == 0)
			{
				links++;
				prf = pr;

				/* Is this the one wanted by the user? */
				if (strcasecmp(g_aiccu->ipv6_interface, pr->name) == 0) found++;
			}
		}

		if (links > 1)
		{
			dolog(LOG_WARNING, "*** Adapter with GUID %s has %u links from the Network Connections control panel, it should only be 1\n", tr->guid, links);
			ok = false;
		}
		else if (links == 0)
		{
			dolog(LOG_WARNING, "[NULL] %s\n", tr->guid);
			dolog(LOG_WARNING, "*** Adapter with GUID %s doesn't have a link from the control panel\n", tr->guid);
			ok = false;
		}

		/* check for TAP-Win32 adapter duplicated GUIDs */
		for (tr1 = tap_reg; tr1 != NULL; tr1 = tr1->next)
		{
			if (tr != tr1 && strcmp(tr->guid, tr1->guid) == 0)
			{
				dolog(LOG_WARNING, "*** Duplicate Adapter GUID %s\n", tr->guid);
				ok = false;
			}
		}

		if (ok)
		{
			count++;
			first = prf;
		}
	}

	ok = false;

	/* When the user didn't configure us correctly and we find a single TAP interface, just rename it */
	if (found == 0 && count == 1 && first)
	{
		dolog(LOG_INFO, "Renaming adapter '%s' to '%s' and using it\n", first->name, g_aiccu->ipv6_interface);
		aiccu_win32_rename_adapter(first->name);
		ok = true;
	}
	else if (found == 1 && count == 1)
	{
		D(dolog(LOG_DEBUG, "Using configured interface %s\n", g_aiccu->ipv6_interface));
		ok = true;
	}
	else
	{
		ok = false;
		dolog(LOG_WARNING, "Found = %u, Count = %u\n", found, count);
	}

	free_tap_reg(tap_reg);
	free_panel_reg(panel_reg);

	return ok;
}

#endif

bool tun_start(struct tun_reader *tun)
{
#ifndef _WIN32
	pthread_t		thread;
#ifdef linux
	struct ifreq		ifr;

	/* Create a new tap device */
	tun_fd = open("/dev/net/tun", O_RDWR);
	if (tun_fd == -1)
	{
		tun_log(LOG_ERR, "start", "Couldn't open device %s: %s (%d)\n", "/dev/net/tun", strerror(errno), errno);
		return false;
	}

	memset(&ifr, 0, sizeof(ifr));
	/* Request a TUN device */
	ifr.ifr_flags = IFF_TUN;
	/* Set the interface name */
	strncpy(ifr.ifr_name, g_aiccu->ipv6_interface, sizeof(ifr.ifr_name));

	if (ioctl(tun_fd, TUNSETIFF, &ifr))
	{
		tun_log(LOG_ERR, "start", "Couldn't set interface name to %s: %s (%d)\n",
			g_aiccu->ipv6_interface, strerror(errno), errno);
		return false;
	}

#else /* *BSD/Darwin */

	char		buf[128];
	unsigned int	i;
	int		mode = IFF_MULTICAST | IFF_POINTOPOINT;

	/* Try the configured interface */
	tun_log(LOG_DEBUG, "start", "Trying Configured TUN/TAP interface %s...\n", g_aiccu->ipv6_interface);
	snprintf(buf, sizeof(buf), "/dev/%s", g_aiccu->ipv6_interface);
	tun_fd = open(buf, O_RDWR);
	if (tun_fd < 0)
	{
		/* Fall back to trying all /dev/tun* devices */
		for (i = 0; i < 256; ++i)
		{
			snprintf(buf, sizeof(buf), "/dev/tun%u", i);
			tun_log(LOG_DEBUG, "start", "Trying TUN/TAP interface %s...\n", &buf[8]);
			tun_fd = open(buf, O_RDWR);
			if (tun_fd >= 0)
			{
				/* Copy over the name of the interface so that configging goes okay */
				if (g_aiccu->ipv6_interface) free(g_aiccu->ipv6_interface);
				snprintf(buf, sizeof(buf), "tun%u", i);
				g_aiccu->ipv6_interface = strdup(buf);
			}
			break;
		}
	}

	if (tun_fd < 0)
	{
		tun_log(LOG_ERR, "start", "Couldn't open device %s or /dev/tun*: %s (%d)\n", g_aiccu->ipv6_interface, strerror(errno), errno);
		return false;
	}

	tun_log(LOG_DEBUG, "start", "Using TUN/TAP interface %s\n", g_aiccu->ipv6_interface);

#ifndef _FREEBSD
#ifndef _DARWIN
#ifndef _AIX
	tun_log(LOG_DEBUG, "start", "Setting TUNSIFMODE for %s\n", g_aiccu->ipv6_interface);
	if (ioctl(tun_fd, TUNSIFMODE, &mode, sizeof(mode)) == -1)
	{
		tun_log(LOG_ERR, "start", "Couldn't set interface %s's TUNSIFMODE to MULTICAST|POINTOPOINT: %s (%d)\n",
			g_aiccu->ipv6_interface, strerror(errno), errno);
		close(tun_fd);
		tun_fd = -1;
		return false;
	}
#endif
#endif
#endif

#ifdef NEED_IFHEAD
	tun_log(LOG_DEBUG, "start", "Setting TUNSIFHEAD for %s\n", g_aiccu->ipv6_interface);
	mode = 1;
	if (ioctl(tun_fd, TUNSIFHEAD, &mode, sizeof(mode)) == -1)
	{
		tun_log(LOG_ERR, "start", "Couldn't set interface %s's TUNSIFHEAD to enabled: %s (%d)\n",
			g_aiccu->ipv6_interface, strerror(errno), errno);
		close(tun_fd);
		tun_fd = -1;
		return false;
	}
#endif

#endif /* linux */


#else /* Windows */

	HKEY		key;
	DWORD		pID;
	HANDLE		h;
	int		i;

	char		adapterid[1024];
	char		tapname[1024];
	DWORD		len;

	if (!tun_fixup_adapters())
	{
		tun_log(LOG_ERR, "start", "TAP-Win32 Adapter not configured properly...\n");
		return false;
	}

	/* Open registry and look for network adapters */
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TAP_REGISTRY_KEY, 0, KEY_READ, &key))
	{
		tun_log(LOG_ERR, "start", "Could not open the networking registry key\n");
		return false;
	}

	for (i = 0; device_handle == INVALID_HANDLE_VALUE; i++)
	{
		len = sizeof(adapterid);
		if (RegEnumKeyEx(key, i, adapterid, &len, 0, 0, 0, NULL)) break;

		snprintf(tapname, sizeof(tapname), TAP_DEVICE_DIR "%s.tap", adapterid);
		tun_log(LOG_DEBUG, "start", "Trying %s\n", tapname);
		device_handle = CreateFile(tapname, GENERIC_WRITE | GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM | FILE_FLAG_OVERLAPPED, 0);

		if (device_handle != INVALID_HANDLE_VALUE)
		{
			unsigned long	status, info[3] = {0,0,0};

			/* get driver version info */
			if (DeviceIoControl(device_handle, TAP_IOCTL_GET_VERSION, &info, sizeof(info), &info, sizeof(info), &len, NULL))
			{
				D(tun_log(LOG_DEBUG, "start", "TAP-Win32 Driver Version %d.%d %s", (int)info[0], (int)info[1], info[2] ? "(DEBUG)" : ""));
			}

			if (!(info[0] > TAP_WIN32_MIN_MAJOR || (info[0] == TAP_WIN32_MIN_MAJOR && info[1] >= TAP_WIN32_MIN_MINOR)))
			{
				tun_log(LOG_ERR, "start", "A TAP-Win32 driver is required that is at least version %d.%d -- If you recently upgraded your Tap32 driver, a reboot is probably required at this point to get Windows to see the new driver.", TAP_WIN32_MIN_MAJOR, TAP_WIN32_MIN_MINOR);
				CloseHandle(device_handle);
				device_handle = INVALID_HANDLE_VALUE;
				continue;
			}

			/* Note: we use TAP mode on Windows, not TUN */

			/* Try to mark the device as 'up */
			status = true;
			DeviceIoControl(device_handle, TAP_IOCTL_SET_MEDIA_STATUS, &status, sizeof(status), &status, sizeof(status), &len, NULL);
		}
	}

	RegCloseKey(key);

	if (device_handle == INVALID_HANDLE_VALUE)
	{
		tun_log(LOG_ERR, "start", "No working Tap device found!\n");
		return false;
	}

#endif /* _WIN32 */

	/* Launch a thread for reader */
#ifndef _WIN32
	pthread_create(&thread, NULL, tun_reader, (void *)tun);
#else
	h = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)tun_reader, tun, 0, &pID);
#endif

	/* We now return, the real tunneling tool can call tun_write() when it wants */

	return true;
}
