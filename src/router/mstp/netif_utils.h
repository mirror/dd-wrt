/*****************************************************************************
  Copyright (c) 2006 EMC Corporation.
  Copyright (c) 2011 Factor-SPE

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
  Authors: Vitalii Demianets <vitas@nppfactor.kiev.ua>

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

#endif /* NETIF_UTILS_H */
