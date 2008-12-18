/*****************************************************************************
  Copyright (c) 2006 EMC Corporation.

  This program is free software; you can redistribute it and/or modify it 
  under the terms of the GNU General Public License as published by the Free 
  Software Foundation; either version 2 of the License, or (at your option) 
  any later version.
  
  This program is distributed in the hope that it will be useful, but WITHOUT 
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
  more details.
  
  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 59 
  Temple Place - Suite 330, Boston, MA  02111-1307, USA.
  
  The full GNU General Public License is included in this distribution in the
  file called LICENSE.

  Authors: Srinivas Aji <Aji_Srinivas@emc.com>

******************************************************************************/

#include "bridge_ctl.h"
#include "netif_utils.h"
#include "packet.h"

#include <unistd.h>
#include <net/if.h>
#include <stdlib.h>
#include <linux/if_bridge.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>
#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "log.h"

#include "rstp.h"
/*------------------------------------------------------------*/

struct ifdata {
	int if_index;
	struct ifdata *next;
	int up;
	char name[IFNAMSIZ];
	unsigned char macaddr[ETH_ALEN];

	int is_bridge;
	/* If bridge */
	struct ifdata *bridge_next;
	struct ifdata *port_list;
	int do_stp;
	int stp_up;
	STP_Bridge *stp_bridge;


	/* If port */
	int speed;
	int duplex;
	struct ifdata *master;
	struct ifdata *port_next;
	/* STP port index */
	int port_index;
	STP_Port *stp_port;

	struct epoll_event_handler event;
};

/* Instances */
struct ifdata *current_br = NULL;

struct ifdata *find_port(int port_index)
{
	struct ifdata *ifc = current_br->port_list;
	while (ifc && ifc->port_index != port_index)
		ifc = ifc->port_next;
	return ifc;
}


struct ifdata *if_head = NULL;
struct ifdata *br_head = NULL;

struct ifdata *find_if(int if_index)
{
	struct ifdata *p = if_head;
	while (p && p->if_index != if_index)
		p = p->next;
	return p;
}

#define ADD_TO_LIST(_list, _next, _ifc) \
    do { \
      (_ifc)->_next = (_list); \
      (_list) = (_ifc); \
    } while (0)

#define REMOVE_FROM_LIST(_list, _next, _ifc, _error_fmt, _args...) \
    do { \
      struct ifdata **_prev = &(_list); \
      while (*_prev && *_prev != (_ifc)) \
        _prev = &(*_prev)->_next; \
      if (*_prev != (_ifc)) \
        ERROR(_error_fmt, ##_args); \
      else \
        *_prev = (_ifc)->_next; \
    } while (0)

/* Caller ensures that there isn't any ifdata with this index */
/* If br is NULL, new interface is a bridge, else it is a port of br */
struct ifdata *create_if(int if_index, struct ifdata *br)
{
	struct ifdata *p;
	TST((p = malloc(sizeof(*p))) != NULL, NULL);

	memset(p, 0, sizeof(*p));

	/* Init fields */
	p->if_index = if_index;
	p->is_bridge = (br == NULL);

	/* TODO: purge use of name, due to issue with renameing */
	if_indextoname(if_index, p->name);
	get_hwaddr(p->name, p->macaddr);

	if (p->is_bridge) {
		INFO("Add bridge %s", p->name);
		p->stp_bridge = STP_IN_bridge_create(p);
		if (!p->stp_bridge) {
			ERROR("Couldn't create STP Bridge");
			free(p);
			return NULL;
		}
		STP_IN_set_bridge_address(p->stp_bridge,
					  (STP_MacAddress *)p->macaddr);
		INFO("Set bridge address %s to %02x:%02x:%02x:%02x:%02x:%02x",
                     p->name,
                     p->macaddr[0], p->macaddr[1], p->macaddr[2],
                     p->macaddr[1], p->macaddr[4], p->macaddr[5]
                     );
		/* Init slave list */
		p->port_list = NULL;

		p->do_stp = 0;
		p->up = 0;
		p->stp_up = 0;
		ADD_TO_LIST(br_head, bridge_next, p);  /* Add to bridge list */
	} else {
		INFO("Add iface %s to bridge %s", p->name, br->name);
		p->up = 0;
		p->speed = 0;
		p->duplex = 0;
		p->master = br;

		p->port_index = get_bridge_portno(p->name);
		if (p->port_index < 0) {
			ERROR("Couldn't get port number for %s", p->name);
			free(p);
			return NULL;
		}
		p->stp_port = STP_IN_port_create(p->master->stp_bridge,
						 p->port_index, p);
		if (!p->stp_port) {
			ERROR("Couldn't create STP Port");
			free(p);
			return NULL;
		}

		ADD_TO_LIST(br->port_list, port_next, p);	/* Add to bridge port list */

	}

	/* Add to interface list */
	ADD_TO_LIST(if_head, next, p);

	return p;
}

void delete_if(struct ifdata *ifc)
{
	INFO("Delete iface %s", ifc->name);
	if (ifc->is_bridge) {	/* Bridge: */
		STP_IN_set_bridge_enable(ifc->stp_bridge, 0);
		/* Delete ports */
		while (ifc->port_list)
			delete_if(ifc->port_list);
		/* Remove from bridge list */
		REMOVE_FROM_LIST(br_head, bridge_next, ifc,
				 "Can't find interface ifindex %d bridge list",
				 ifc->if_index);
		STP_IN_bridge_delete(ifc->stp_bridge);
	} else {		/* Port */
		/* Remove from bridge port list */
		REMOVE_FROM_LIST(ifc->master->port_list, port_next, ifc,
				 "Can't find interface ifindex %d on br %d's port list",
				 ifc->if_index, ifc->master->if_index);
		STP_IN_port_delete(ifc->stp_port);
	}

	/* Remove from bridge interface list */
	REMOVE_FROM_LIST(if_head, next, ifc,
			 "Can't find interface ifindex %d on iflist",
			 ifc->if_index);
	free(ifc);
}

/* New MAC address is stored in addr, which also holds the old value on entry.
   Return nonzero if the address changed */
static int check_mac_address(char *name, unsigned char *addr)
{
	unsigned char temp_addr[6];
	if (get_hwaddr(name, temp_addr)) {
		LOG("Error getting hw address: %s", name);
		/* Error. Ignore the new value */
		return 0;
	}
	if (memcmp(addr, temp_addr, sizeof(temp_addr)) == 0)
		return 0;
	else {
		memcpy(addr, temp_addr, sizeof(temp_addr));
		return 1;
	}
}


static int stp_enabled(struct ifdata *br)
{
	char path[40 + IFNAMSIZ];
	sprintf(path, "/sys/class/net/%s/bridge/stp_state", br->name);
	FILE *f = fopen(path, "r");
	if (!f) {
		LOG("Open %s failed", path);
		return 0;
	}
	int enabled = 0;
	fscanf(f, "%d", &enabled);
	fclose(f);
	INFO("STP on %s state %d", br->name, enabled);

	return enabled == 2;	/* ie user mode STP */
}

void set_br_up(struct ifdata *br, int up)
{
	int stp_up = stp_enabled(br);
	INFO("%s was %s stp was %s", br->name, br->up ? "up" : "down", br->stp_up ? "up" : "down");
	INFO("Set bridge %s %s stp %s" , br->name,
	     up ? "up" : "down", stp_up ? "up" : "down");

	int changed = 0;

	if (up != br->up) {
		br->up = up;
		changed = 1;
	}
	
	if (br->stp_up != stp_up) {
		br->stp_up = stp_up;
		changed = 1;
	}

	if (check_mac_address(br->name, br->macaddr)) {
		/* MAC address changed */
		/* Notify bridge address change */
		STP_IN_set_bridge_address(
			br->stp_bridge, (STP_MacAddress *)br->macaddr);
	}

	if (changed)
		STP_IN_set_bridge_enable(br->stp_bridge,
					 (br->up && br->stp_up)?1:0);
}

void set_if_up(struct ifdata *ifc, int up)
{
	INFO("Port %s : %s", ifc->name, (up ? "up" : "down"));
	int speed = -1;
	int duplex = -1;
	int changed = 0;

	if (check_mac_address(ifc->name, ifc->macaddr)) {
		/* MAC address changed */
		if (check_mac_address(ifc->master->name, ifc->master->macaddr)
			) {
			/* Notify bridge address change */
			STP_IN_set_bridge_address(
				ifc->master->stp_bridge,
				(STP_MacAddress *)ifc->master->macaddr);
		}
	}

	if (!up) {		/* Down */
		if (ifc->up) {
			ifc->up = up;
			changed = 1;
		}
	} else {		/* Up */
		int r = ethtool_get_speed_duplex(ifc->name, &speed, &duplex);
		if (r < 0) {	/* Didn't succeed */
		}
		if (speed < 0)
			speed = 10;
		if (duplex < 0)
			duplex = 0;	/* Assume half duplex */

		if (speed != ifc->speed) {
			ifc->speed = speed;
			changed = 1;
		}
		if (duplex != ifc->duplex) {
			ifc->duplex = duplex;
			changed = 1;
		}
		if (!ifc->up) {
			ifc->up = 1;
			changed = 1;
		}
	}
	if (changed)
		STP_IN_set_port_enable(ifc->stp_port,
				       ifc->up, ifc->speed, ifc->duplex);
}

/*------------------------------------------------------------*/

int bridge_notify(int br_index, int if_index, int newlink, int up)
{
	if (up)
		up = 1;
	LOG("br_index %d, if_index %d, newlink %d, up %d",
		br_index, if_index, newlink, up);

	struct ifdata *br = NULL;
	if (br_index >= 0) {
		br = find_if(br_index);
		if (br && !br->is_bridge) {
			ERROR
			    ("Notification shows non bridge interface %d as bridge.",
			     br_index);
			return -1;
		}
		if (!br)
			br = create_if(br_index, NULL);
		if (!br) {
			ERROR("Couldn't create data for bridge interface %d",
			      br_index);
			return -1;
		}
		/* Bridge must be up if we get such notifications */
		// Not true anymore - set_br_up(br, 1);
	}

	struct ifdata *ifc = find_if(if_index);

	if (br) {
		if (ifc) {
			if (ifc->is_bridge) {
				ERROR
				    ("Notification shows bridge interface %d as slave of %d",
				     if_index, br_index);
				return -1;
			}
			if (ifc->master != br) {
				INFO("Device %d has come to bridge %d. "
				     "Missed notify for deletion from bridge %d",
				     if_index, br_index, ifc->master->if_index);
				delete_if(ifc);
				ifc = NULL;
			}
		}
		if (!ifc) {
			if (!newlink) {
				INFO("Got DELLINK for unknown port %d on "
				     "bridge %d", if_index, br_index);
				return -1;
			}
			ifc = create_if(if_index, br);
		}
		if (!ifc) {
			ERROR
			    ("Couldn't create data for interface %d (master %d)",
			     if_index, br_index);
			return -1;
		}
		if (!newlink) {
			delete_if(ifc);
			return 0;
		}
		set_if_up(ifc, up);	/* And speed and duplex */
	} else {		/* No br_index */
		if (!newlink) {
			/* DELLINK not from bridge means interface unregistered. */
			/* Cleanup removed bridge or removed bridge slave */
			if (ifc)
				delete_if(ifc);
			return 0;
		} else {	/* This may be a new link */
			if (!ifc) {
				char ifname[IFNAMSIZ];
				if (if_indextoname(if_index, ifname)
				    && is_bridge(ifname)) {
					ifc = create_if(if_index, NULL);
					if (!ifc) {
						ERROR
						    ("Couldn't create data for bridge interface %d",
						     if_index);
						return -1;
					}
				}
			}
			if (ifc) {
				if (ifc->is_bridge)
					set_br_up(ifc, up);
				else
					set_if_up(ifc, up);
			}
		}
	}
	return 0;
}

struct llc_header
{
	uint8_t dest_addr[ETH_ALEN];
	uint8_t src_addr[ETH_ALEN];
	uint16_t len8023;
	uint8_t d_sap;  /* 0x42 */
	uint8_t s_sap;  /* 0x42 */
	uint8_t llc_ui; /* 0x03 */
} __attribute__((packed));

const unsigned char bridge_group_address[ETH_ALEN] = {
	0x01, 0x80, 0xc2, 0x00, 0x00, 0x00
};

const unsigned char STP_SAP = 0x42;

void bridge_bpdu_rcv(int if_index, const unsigned char *data, int len)
{
	struct ifdata *ifc = find_if(if_index);

	LOG("ifindex %d, len %d", if_index, len);
	if (!ifc)
		return;

	TST(ifc->up,);

	if (!ifc->master) 
		return;

	if (!ifc->master->stp_up)
		return;

	/* Validate Ethernet and LLC header */
	{
		struct llc_header *h;
		unsigned int l;
		TST(len > sizeof(struct llc_header),);
		h = (struct llc_header *)data;
		TST(memcmp(h->dest_addr, bridge_group_address, ETH_ALEN) == 0,
                    INFO("ifindex %d, len %d, %02x:%02x:%02x:%02x:%02x:%02x",
                         if_index, len,
                         h->dest_addr[0],h->dest_addr[1],h->dest_addr[2],
                         h->dest_addr[3],h->dest_addr[4],h->dest_addr[5]));
		l = ntohs(h->len8023);
		TST(l <= ETH_DATA_LEN && l <= len - ETH_HLEN && l >= 3,);
		TST(h->d_sap == STP_SAP && h->s_sap == STP_SAP
		    && (h->llc_ui & 0x3) == 0x3 /* LLC UI */,);

		STP_IN_rx_bpdu(ifc->stp_port,
			       /* Don't include LLC header */
			       data + sizeof(*h), l - 3);
	}
}

void bridge_one_second(void)
{
	//  LOG("");
	struct ifdata *br;
	for (br = br_head; br; br = br->bridge_next) {
		STP_IN_one_second(br->stp_bridge);
	}
}

/* Implementing STP_OUT functions */

int flush_port(char *sys_name)
{
	FILE *f = fopen(sys_name, "w");
	TSTM(f, -1, "Couldn't open flush file %s for write.", sys_name);
	int r = fwrite("1", 1, 1, f);
	fclose(f);
	TST(r == 1, -1);
	return 0;
}

void STP_OUT_port_fdb_flush(void *user_ref)
{
	struct ifdata *port = user_ref;
	char fname[128];
	snprintf(fname, sizeof(fname),
		 "/sys/class/net/%s/brport/flush", port->name);
	fname[sizeof(fname) - 1] = 0;
	TST(flush_port(fname) == 0,);
}

void STP_OUT_port_set_state(void *user_ref, unsigned int flags)
{
	struct ifdata *port = user_ref;
	int br_state;
	
	LOG("port index %d, flags %d", port->if_index, flags);
	
	if (flags & STP_PORT_STATE_FLAG_FORWARDING)
		br_state = BR_STATE_FORWARDING;
	else if (flags & STP_PORT_STATE_FLAG_LEARNING)
		br_state = BR_STATE_LEARNING;
	else
		br_state = BR_STATE_BLOCKING;
	
	if (port->up)
		bridge_set_state(port->if_index, br_state);
}


void STP_OUT_tx_bpdu(void *port_user_ref, void *base, unsigned int len)
{
	struct ifdata *port = port_user_ref;

	LOG("port index %d, len %d", port->if_index, len);

	struct llc_header h;
	memcpy(h.dest_addr, bridge_group_address, ETH_ALEN);
	memcpy(h.src_addr, port->macaddr, ETH_ALEN);
	/* bpdu_len excludes MAC and LLC headers */
	h.len8023 = htons(len + 3);
	h.d_sap = h.s_sap = STP_SAP;
	h.llc_ui = 0x03; /* LLC UI packet */

	struct iovec iov[2] = {
		{ .iov_base = &h, .iov_len = sizeof(h) },
		{ .iov_base = base, .iov_len = len }
	};

	packet_send(port->if_index, iov, 2, sizeof(h) + len);
}


void STP_OUT_logmsg(void *br_user_ref, void *port_user_ref,
		    int level, char *fmt, ...)
{
	struct ifdata *bridge = br_user_ref;
	struct ifdata *port = port_user_ref;
	char buf[256];
	int r;
	int ll = (level < STP_LOG_LEVEL_DEBUG) ?
		LOG_LEVEL_INFO : LOG_LEVEL_DEBUG;
        struct timeval tv;
        gettimeofday(&tv, NULL);
	r = snprintf(buf, sizeof(buf), "LOG Level %d:%s:%s: %02d.%03d: ",
                     level,
		     (bridge?bridge->name:""), (port?port->name:""),
                     (int)tv.tv_sec % 60, (int)tv.tv_usec / 1000);
	if (r >= sizeof(buf)) {
		buf[sizeof(buf) - 1] = 0;
		Dprintf(ll, "%s", buf);
		r = 0;
	}

	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf + r, sizeof(buf) - r, fmt, ap);
	buf[sizeof(buf) - 1] = 0;
	Dprintf(ll, "%s", buf);
	va_end(ap);
	if (level == STP_LOG_LEVEL_ERROR)
		ctl_err_log("%s", buf + r);
}

void *STP_OUT_mem_zalloc(unsigned int size)
{
	return calloc(1, size);
}


void STP_OUT_mem_free(void *p)
{
	free(p);
}


/* Commands and status */
#include "ctl_functions.h"

#define CTL_CHECK_BRIDGE \
	struct ifdata *br = find_if(br_index);				\
	if (br == NULL || !br->is_bridge) {				\
		ERROR("Couldn't find bridge with index %d", br_index);	\
		return -1;						\
	}								\
	do { } while (0)

#define CTL_CHECK_BRIDGE_PORT \
	CTL_CHECK_BRIDGE;		     \
	struct ifdata *port = find_if(port_index);			\
	if (port == NULL || port->is_bridge || port->master != br) {	\
		ERROR("Interface with index %d not a port of bridge "	\
		      "with index %d", port_index, br_index);		\
		return -1;						\
	}								\
	do { } while (0)

int CTL_enable_bridge_rstp(int br_index, int enable)
{
	INFO("bridge %d, enable %d", br_index, enable);
	int r = 0;
	if (enable)
		enable = 1;
	struct ifdata *br = find_if(br_index);
	if (br == NULL) {
		char ifname[IFNAMSIZ];
		if (if_indextoname(br_index, ifname) && is_bridge(ifname))
			br = create_if(br_index, NULL);
	}
	if (br == NULL || !br->is_bridge) {
		ERROR("Couldn't find bridge with index %d", br_index);
		return -1;
	}
	if (br->up)
		set_br_up(br, 1);
	return r;
}

int CTL_get_bridge_status(int br_index, STP_BridgeStatus *status)
{
	LOG("bridge %d", br_index);
	CTL_CHECK_BRIDGE;

	STP_IN_get_bridge_status(br->stp_bridge, status);
	return 0;
}

int CTL_set_bridge_config(int br_index,  STP_BridgeConfig *cfg)
{
	INFO("bridge %d", br_index);
	CTL_CHECK_BRIDGE;

	if (cfg->set_bridge_address) {
		ERROR("Setting bridge address not permitted: %s", br->name);
		return -1;
	}
	
	int r = STP_IN_set_bridge_config(br->stp_bridge, cfg);

	if (r) {
		ERROR("Error setting bridge config for %s", br->name);
		return r;
	}
	return 0;
}

int CTL_get_port_status(int br_index, int port_index, STP_PortStatus *status)
{
	LOG("bridge %d port %d", br_index, port_index);
	CTL_CHECK_BRIDGE_PORT;

	STP_IN_get_port_status(port->stp_port, status);
	return 0;
}

int CTL_set_port_config(int br_index, int port_index, STP_PortConfig *cfg)
{
	INFO("bridge %d, port %d", br_index, port_index);
	CTL_CHECK_BRIDGE_PORT;

	int r = STP_IN_set_port_config(port->stp_port, cfg);
	if (r) {
		ERROR("Error setting port config for %s", port->name);
		return r;
	}
	
	return 0;
}

int CTL_port_mcheck(int br_index, int port_index)
{
	INFO("bridge %d, port %d", br_index, port_index);
	CTL_CHECK_BRIDGE_PORT;

	int r = STP_IN_port_mcheck(port->stp_port);
	if (r) {
		ERROR("Error doing port mcheck for %s", port->name);
		return r;
	}
	
	return 0;
}

int CTL_set_debug_level(int level)
{
	INFO("level %d", level);
	log_level = level;
	return 0;
}

#undef CTL_CHECK_BRIDGE_PORT
#undef CTL_CHECK_BRIDGE
