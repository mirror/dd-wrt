/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Andreas Langer <an.langer@gmx.de>, Marek Lindner <mareklindner@neomailbox.ch>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */

#ifndef _BATCTL_FUNCTIONS_H
#define _BATCTL_FUNCTIONS_H

#include <net/ethernet.h>
#include <netlink/netlink.h>
#include <netlink/handlers.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "main.h"

/**
 * enum batadv_bandwidth_units - bandwidth unit types
 */
enum batadv_bandwidth_units {
	/** @BATADV_BW_UNIT_KBIT: unit type kbit */
	BATADV_BW_UNIT_KBIT,

	/** @BATADV_BW_UNIT_MBIT: unit type mbit */
	BATADV_BW_UNIT_MBIT,
};

#define ETH_STR_LEN 17
#define BATMAN_ADV_TAG "batman-adv:"

#define PATH_BUFF_LEN 400

struct state;

/* return time delta from start to end in milliseconds */
void start_timer(void);
double end_timer(void);
char *ether_ntoa_long(const struct ether_addr *addr);
char *get_name_by_macaddr(struct ether_addr *mac_addr, int read_opt);
char *get_name_by_macstr(char *mac_str, int read_opt);
int file_exists(const char *fpath);
int read_file(const char *full_path, int read_opt);
struct ether_addr *translate_mac(struct state *state,
				 const struct ether_addr *mac);
struct ether_addr *resolve_mac(const char *asc);
int query_rtnl_link(int ifindex, nl_recvmsg_msg_cb_t func, void *arg);
int netlink_simple_request(struct nl_msg *msg);
int translate_mesh_iface_vlan(struct state *state, const char *vlandev);
int translate_vlan_iface(struct state *state, const char *vlandev);
int translate_vid(struct state *state, const char *vidstr);
int translate_hard_iface(struct state *state, const char *hardif);
int guess_netdev_type(const char *netdev, enum selector_prefix *type);
int get_algoname(struct state *state, unsigned int mesh_ifindex,
		 char *algoname, size_t algoname_len);
int check_mesh_iface(struct state *state);
int check_mesh_iface_ownership(struct state *state, char *hard_iface);

void get_random_bytes(void *buf, size_t buflen);
void check_root_or_die(const char *cmd);

int parse_bool(const char *val, bool *res);
bool parse_throughput(char *buff, const char *description,
		      uint32_t *throughput);

extern char *line_ptr;

enum {
	NO_FLAGS = 0x00,
	CONT_READ = 0x01,
	CLR_CONT_READ = 0x02,
	USE_BAT_HOSTS = 0x04,
	USE_READ_BUFF = 0x10,
	SILENCE_ERRORS = 0x20,
	NO_OLD_ORIGS = 0x40,
	COMPAT_FILTER = 0x80,
	SKIP_HEADER = 0x100,
	UNICAST_ONLY = 0x200,
	MULTICAST_ONLY = 0x400,
};

#endif
