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

#include <linux/module.h>
#include <linux/types.h>
#include <linux/atomic.h>

#include <asm/octeon/cvmx-helper-cfg.h>


static atomic_t init_port_cfg_data_once[4];
static atomic_t octeon3_eth_driver_once;


int octeon3_init_port_cfg_data(int node)
{
	int	rc = 0;

	if (atomic_cmpxchg(init_port_cfg_data_once + node, 0, 1) == 0)
		rc = __cvmx_helper_init_port_config_data(node);

	return rc;
}
EXPORT_SYMBOL(octeon3_init_port_cfg_data);

int octeon3_load_eth_driver(void)
{
	if (atomic_cmpxchg(&octeon3_eth_driver_once, 0, 1) == 0)
		request_module_nowait("octeon3-ethernet");

	return 0;
}
EXPORT_SYMBOL(octeon3_load_eth_driver);

int octeon3_get_eth_id(void)
{
	static int pki_id;

	return pki_id++;
}
EXPORT_SYMBOL(octeon3_get_eth_id);

static int __init common_nexus_init(void)
{
	return 0;
}

static void __exit common_nexus_exit(void)
{
}

module_init(common_nexus_init);
module_exit(common_nexus_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Carlos Munoz <cmunoz@caviumnetworks.com>");
MODULE_DESCRIPTION("Cavium Networks common ethernet nexus driver.");
