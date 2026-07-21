/* SPDX-License-Identifier: GPL-2.0-or-later */
/*****************************************************************************
  Copyright (c) 2006 EMC Corporation.
  Copyright (c) 2011 Factor-SPE

  Authors: Srinivas Aji <Aji_Srinivas@emc.com>
  Authors: Vitalii Demianets <dvitasgs@gmail.com>

******************************************************************************/

#ifndef NETIF_UTILS_H
#define NETIF_UTILS_H

/* An inet socket for everyone to use for ifreqs. */
int netsock_init(void);

int get_hwaddr(char *ifname, unsigned char *hwaddr);
int get_flags(char *ifname);
int if_shutdown(char *ifname);

int ethtool_get_speed_duplex(char *ifname, int *speed, int *duplex);

bool is_bridge(char *if_name);

int get_bridge_portno(char *if_name);

char *index_to_name(int index, char *name);
char *index_to_port_name(int index, char *name);

#endif /* NETIF_UTILS_H */
