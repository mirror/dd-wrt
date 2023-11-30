/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner <mareklindner@neomailbox.ch>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */



#ifndef _BATCTL_BAT_HOSTS_H
#define _BATCTL_BAT_HOSTS_H

#include <net/ethernet.h>

#define HOST_NAME_MAX_LEN 50
#define CONF_DIR_LEN 256


struct bat_host {
	struct ether_addr mac_addr;
	char name[HOST_NAME_MAX_LEN];
} __attribute__((packed));

void bat_hosts_init(int read_opt);
struct bat_host *bat_hosts_find_by_name(char *name);
struct bat_host *bat_hosts_find_by_mac(char *mac);
void bat_hosts_free(void);

#endif
