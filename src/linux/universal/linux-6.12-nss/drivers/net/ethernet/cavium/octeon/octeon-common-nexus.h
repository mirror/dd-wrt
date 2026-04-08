/**********************************************************************
 * Copyright (c) 2015 Cavium, Inc.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, Version 2, as
 * published by the Free Software Foundation.
 *
 * This file is distributed in the hope that it will be useful, but
 * AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or
 * NONINFRINGEMENT.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this file; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 * or visit http://www.gnu.org/licenses/.
 *
 * This file may also be available under a different license from Cavium.
 * Contact Cavium, Inc. for more information
 **********************************************************************/

#ifndef __OCTEON3_COMMON_NEXUS_H__
#define __OCTEON3_COMMON_NEXUS_H__

#define MAX_NODES		2
#define SRIO_INTERFACE_OFFSET	1


enum octeon3_mac_type {
	BGX_MAC,
	SRIO_MAC
};

enum octeon3_src_type {
	QLM,
	XCV
};

struct mac_platform_data {
	enum octeon3_mac_type	mac_type;
	int			numa_node;
	int			interface;
	int			port;
	enum octeon3_src_type	src_type;
};

extern int octeon3_init_port_cfg_data(int node);
extern int octeon3_load_eth_driver(void);
extern int octeon3_get_eth_id(void);
extern void srio_nexus_load(void);

#endif /* __OCTEON3_COMMON_NEXUS_H__ */
