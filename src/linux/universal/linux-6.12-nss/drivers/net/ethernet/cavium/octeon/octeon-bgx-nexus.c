/**********************************************************************
 * Author: Cavium, Inc.
 *
 * Contact: support@cavium.com
 * This file is part of the OCTEON SDK
 *
 * Copyright (c) 2014 Cavium, Inc.
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
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/ctype.h>

#include <asm/octeon/cvmx-helper.h>
#include <asm/octeon/cvmx-helper-util.h>
#include <asm/octeon/cvmx-helper-cfg.h>
#include <asm/octeon/cvmx-helper-bgx.h>
#include <asm/octeon/cvmx-bgxx-defs.h>

#include "octeon-bgx.h"
#include "octeon-common-nexus.h"

static atomic_t request_mgmt_once;

static char *mix_port;
module_param(mix_port, charp, S_IRUGO);
MODULE_PARM_DESC(mix_port, "Specifies which ports connect to MIX interfaces.");

static char *pki_port;
module_param(pki_port, charp, S_IRUGO);
MODULE_PARM_DESC(pki_port, "Specifies which ports connect to the PKI.");

#define MAX_MIX_PER_NODE	2
#define MAX_BGX_PER_NODE	6
#define MAX_LMAC_PER_NODE	4

#define MAX_MIX			(MAX_NODES * MAX_MIX_PER_NODE)

/* mix_port_lmac:		Describes a lmac that connects to a mix port.
 *				The lmac must be on the same node as the mix.
 *
 *  node:			Node of the lmac.
 *  bgx:			Bgx of the lmac.
 *  lmac:			Lmac index.
 */
struct mix_port_lmac {
	int	node;
	int	bgx;
	int	lmac;
};

/* mix_ports_lmacs contains all the lmacs connected to mix ports */
static struct mix_port_lmac mix_port_lmacs[MAX_MIX];

/* pki_ports keeps track of the lmacs connected to the pki */
static bool pki_ports[MAX_NODES][MAX_BGX_PER_NODE][MAX_LMAC_PER_NODE];

/* Created platform devices get added to this list */
static struct list_head pdev_list;
static struct mutex pdev_list_lock;

/* Created platform device use this structure to add themselves to the list */
struct pdev_list_item {
	struct list_head	list;
	struct platform_device	*pdev;
};

/* is_lmac_to_mix:		Search the list of lmacs connected to mix'es
 *				for a match.
 *
 *  node:			Numa node of lmac to search for.
 *  bgx:			Bgx of lmac to search for.
 *  lmac:			Lmac index to search for.
 *
 *  returns:			true if the lmac is connected to a mix, false
 *				otherwise.
 */
static bool is_lmac_to_mix(int node, int bgx, int lmac)
{
	int	i;

	for (i = 0; i < MAX_MIX; i++) {
		if (mix_port_lmacs[i].node == node &&
		    mix_port_lmacs[i].bgx == bgx &&
		    mix_port_lmacs[i].lmac == lmac)
			return true;
	}

	return false;
}

/* is_lmac_to_pki:		Search the list of lmacs connected to the pki
 *				for a match.
 *
 *  node:			Numa node of lmac to search for.
 *  bgx:			Bgx of lmac to search for.
 *  lmac:			Lmac index to search for.
 *
 *  returns:			true if the lmac is connected to the pki, false
 *				otherwise.
 */
static bool is_lmac_to_pki(int node, int bgx, int lmac)
{
	return pki_ports[node][bgx][lmac];
}

/* is_lmac_to_xcv:		Check if this lmac is connected to the xcv
 *				block (rgmii).
 *
 *  of_node:			Device node to check.
 *
 *  returns:			true if the lmac is connected to the xcv, false
 *				otherwise.
 */
static bool is_lmac_to_xcv(struct device_node *of_node)
{
	return of_device_is_compatible(of_node, "cavium,octeon-7360-xcv");
}

static int bgx_probe(struct platform_device *pdev)
{
	struct mac_platform_data platform_data;
	const __be32 *reg;
	u32 port;
	u64 addr;
	struct device_node *child;
	struct platform_device *new_dev;
	struct platform_device *pki_dev;
	int pki_id;
	int numa_node, interface;
	int i;
	int r = 0;
	char id[64];

	reg = of_get_property(pdev->dev.of_node, "reg", NULL);
	addr = of_translate_address(pdev->dev.of_node, reg);
	interface = (addr >> 24) & 0xf;
	numa_node = (addr >> 36) & 0x7;

	/* Initialize the node's ports' configuration data */
	octeon3_init_port_cfg_data(numa_node);

	__cvmx_helper_bgx_probe(cvmx_helper_node_interface_to_xiface(numa_node, interface));

	/* Assign 8 CAM entries per LMAC */
	for (i = 0; i < 32; i++) {
		union cvmx_bgxx_cmr_rx_adrx_cam adr_cam;
		adr_cam.u64 = 0;
		adr_cam.s.id = i >> 3;
		cvmx_write_csr_node(numa_node, CVMX_BGXX_CMR_RX_ADRX_CAM(i, interface), adr_cam.u64);
	}

	for_each_available_child_of_node(pdev->dev.of_node, child) {
		bool is_mix = false;
		bool is_pki = false;
		bool is_xcv = false;
		union cvmx_bgxx_cmrx_config cmr_config;
		cvmx_bgxx_cmr_global_config_t global_config;
		struct pdev_list_item *pdev_item;

		if (!of_device_is_compatible(child, "cavium,octeon-7890-bgx-port") &&
		    !of_device_is_compatible(child, "cavium,octeon-7360-xcv"))
			continue;
		r = of_property_read_u32(child, "reg", &port);
		if (r)
			return -ENODEV;

		is_mix = is_lmac_to_mix(numa_node, interface, port);
		is_pki = is_lmac_to_pki(numa_node, interface, port);
		is_xcv = is_lmac_to_xcv(child);

		/* Check if this port should be configured */
		if (is_mix == false && is_pki == false)
			continue;

		/* Connect to PKI/PKO */
		cmr_config.u64 = cvmx_read_csr_node(numa_node, CVMX_BGXX_CMRX_CONFIG(port, interface));
		cmr_config.s.mix_en = is_mix ? 1 : 0;
		cvmx_write_csr_node(numa_node, CVMX_BGXX_CMRX_CONFIG(port, interface), cmr_config.u64);

		/* Unreset the mix bgx interface or it will interfare with the
		 * other ports.
		 */
		if (is_mix) {
			global_config.u64 = cvmx_read_csr_node(numa_node,
					CVMX_BGXX_CMR_GLOBAL_CONFIG(interface));
			if (!port)
				global_config.s.cmr_mix0_reset = 0;
			else if (port == 1)
				global_config.s.cmr_mix1_reset = 0;
			cvmx_write_csr_node(numa_node,
				    CVMX_BGXX_CMR_GLOBAL_CONFIG(interface),
					    global_config.u64);
		}

		snprintf(id, sizeof(id), "%llx.%u.ethernet-mac", (unsigned long long)addr, port);
		new_dev = of_platform_device_create(child, id, &pdev->dev);
		if (!new_dev) {
			dev_err(&pdev->dev, "Error creating %s\n", id);
			continue;
		}
		platform_data.mac_type = BGX_MAC;
		platform_data.numa_node = numa_node;
		platform_data.interface = interface;
		platform_data.port = port;
		if (is_xcv)
			platform_data.src_type = XCV;
		else
			platform_data.src_type = QLM;

		/* Add device to the list of created devices so we can remove it
		 * on exit.
		 */
		pdev_item = kmalloc(sizeof(*pdev_item), GFP_KERNEL);
		pdev_item->pdev = new_dev;
		mutex_lock(&pdev_list_lock);
		list_add(&pdev_item->list, &pdev_list);
		mutex_unlock(&pdev_list_lock);

		pki_id = octeon3_get_eth_id();
		pki_dev = platform_device_register_data(&new_dev->dev,
							is_mix ? "octeon_mgmt" : "ethernet-mac-pki",
							pki_id,
							&platform_data, sizeof(platform_data));
		dev_info(&pdev->dev, "Created %s %u: %p\n", is_mix ? "MIX" : "PKI", pki_dev->id, pki_dev);

		/* Add device to the list of created devices so we can remove it
		 * on exit.
		 */
		pdev_item = kmalloc(sizeof(*pdev_item), GFP_KERNEL);
		pdev_item->pdev = pki_dev;
		mutex_lock(&pdev_list_lock);
		list_add(&pdev_item->list, &pdev_list);
		mutex_unlock(&pdev_list_lock);

#ifdef CONFIG_NUMA
		new_dev->dev.numa_node = pdev->dev.numa_node;
		pki_dev->dev.numa_node = pdev->dev.numa_node;
#endif
		/* One time request driver module */
		if (is_mix) {
			if (atomic_cmpxchg(&request_mgmt_once, 0, 1) == 0)
				request_module_nowait("octeon_mgmt");
		}
		if (is_pki)
			octeon3_load_eth_driver();
	}

	dev_info(&pdev->dev, "Probed\n");
	return 0;
}

/* bgx_mix_init_from_fdt:	Initialize the list of lmacs that connect to mix
 *				ports from information in the device tree.
 *
 *  returns:			Zero on success, error otherwise.
 */
static int bgx_mix_init_from_fdt(void)
{
	struct device_node	*node;
	struct device_node	*parent = NULL;
	int			mix = 0;

	for_each_compatible_node(node, NULL, "cavium,octeon-7890-mix") {
		struct device_node	*lmac_fdt_node;
		const __be32		*reg;
		u64			addr;

		/* Get the fdt node of the lmac connected to this mix */
		lmac_fdt_node = of_parse_phandle(node, "cavium,mac-handle", 0);
		if (!lmac_fdt_node)
			goto err;

		/* Get the numa node and bgx of the lmac */
		parent = of_get_parent(lmac_fdt_node);
		if (!parent)
			goto err;
		reg = of_get_property(parent, "reg", NULL);
		if (!reg)
			goto err;
		addr = of_translate_address(parent, reg);
		of_node_put(parent);
		parent = NULL;

		mix_port_lmacs[mix].node = (addr >> 36) & 0x7;
		mix_port_lmacs[mix].bgx = (addr >> 24) & 0xf;

		/* Get the lmac index */
		reg = of_get_property(lmac_fdt_node, "reg", NULL);
		if (!reg)
			goto err;

		mix_port_lmacs[mix].lmac = *reg;

		mix++;
		if (mix >= MAX_MIX)
			break;
	}

	return 0;
 err:
	pr_warn("Invalid device tree mix port information\n");
	for (mix = 0; mix < MAX_MIX; mix++) {
		mix_port_lmacs[mix].node = -1;
		mix_port_lmacs[mix].bgx = -1;
		mix_port_lmacs[mix].lmac = -1;
	}
	if (parent)
		of_node_put(parent);

	return -EINVAL;
}

/* bgx_mix_init_from_param:	Initialize the list of lmacs that connect to mix
 *				ports from information in the "mix_port"
 *				parameter. The mix_port parameter format is as
 *				follows:
 *				mix_port=nbl
 *				where:
 *					n = node
 *					b = bgx
 *					l = lmac
 *				There can be up to 4 lmacs defined separated by
 *				commas. For example to select node0, bgx0, lmac0
 *				and node0, bgx4, lamc0, the mix_port parameter
 *				would be: mix_port=000,040
 *
 *  returns:			Zero on success, error otherwise.
 */
static int bgx_mix_init_from_param(void)
{
	char	*p = mix_port;
	int	mix = 0;
	int	i;

	while (*p) {
		int	node = -1;
		int	bgx = -1;
		int	lmac = -1;

		if (strlen(p) < 3)
			goto err;

		/* Get the numa node */
		if (!isdigit(*p))
			goto err;
		node = *p - '0';
		if (node >= MAX_NODES)
			goto err;

		/* Get the bgx */
		p++;
		if (!isdigit(*p))
			goto err;
		bgx = *p - '0';
		if (bgx >= MAX_BGX_PER_NODE)
			goto err;

		/* Get the lmac index */
		p++;
		if (!isdigit(*p))
			goto err;
		lmac = *p - '0';
		if (lmac >= 2)
			goto err;

		/* Only one lmac0 and one lmac1 per node is supported */
		for (i = 0; i < MAX_MIX; i++) {
			if (mix_port_lmacs[i].node == node &&
			    mix_port_lmacs[i].lmac == lmac)
				goto err;
		}

		mix_port_lmacs[mix].node = node;
		mix_port_lmacs[mix].bgx = bgx;
		mix_port_lmacs[mix].lmac = lmac;

		p++;
		if (*p == ',')
			p++;

		mix++;
		if (mix >= MAX_MIX)
			break;
	}

	return 0;
 err:
	pr_warn("Invalid parameter mix_port=%s\n", mix_port);
	for (mix = 0; mix < MAX_MIX; mix++) {
		mix_port_lmacs[mix].node = -1;
		mix_port_lmacs[mix].bgx = -1;
		mix_port_lmacs[mix].lmac = -1;
	}
	return -EINVAL;
}

/* bgx_mix_port_lmacs_init:	Initialize the mix_port_lmacs variable with the
 *				lmacs that connect to mic ports.
 *
 *  returns:			Zero on success, error otherwise.
 */
static int bgx_mix_port_lmacs_init(void)
{
	int	mix;

	/* Start with no mix ports configured */
	for (mix = 0; mix < MAX_MIX; mix++) {
		mix_port_lmacs[mix].node = -1;
		mix_port_lmacs[mix].bgx = -1;
		mix_port_lmacs[mix].lmac = -1;
	}

	/* Check if no mix port should be configured */
	if (mix_port && !strcmp(mix_port, "none"))
		return 0;

	/* Configure the mix ports using information from the device tree if no
	 * parameter was passed. Otherwise, use the information in the module
	 * parameter.
	 */
	if (!mix_port)
		bgx_mix_init_from_fdt();
	else
		bgx_mix_init_from_param();

	return 0;
}

/* bgx_parse_pki_elem:		Parse a single element (node, bgx, or lmac) out
 *				a pki lmac string and set its bitmap
 *				accordingly.
 *
 *  str:			Pki lmac string to parse.
 *  bitmap:			Updated with the bits selected by str.
 *  size:			Maximum size of the bitmap.
 *
 *  returns:			Number of characters processed from str,
 *				error otherwise.
 */
static int bgx_parse_pki_elem(const char *str, unsigned long *bitmap, int size)
{
	const char	*p = str;
	int		len = -1;
	int		bit;

	if (*p == 0) {
		/* If identifier is missing, the whole subset is allowed */
		bitmap_set(bitmap, 0, size);
		len = 0;
	} else if (*p == '*') {
		/* If identifier is an asterisk, the whole subset is allowed */
		bitmap_set(bitmap, 0, size);
		len = 1;
	} else if (isdigit(*p)) {
		/* If identifier is a digit, only the bit corresponding to the
		 * digit is set.
		 */
		bit = *p - '0';
		if (bit < size) {
			bitmap_set(bitmap, bit, 1);
			len = 1;
		}
	} else if (*p == '[') {
		/* If identifier is a bracket, all the bits corresponding to
		 * the digits inside the bracket are set.
		 */
		p++;
		len = 1;
		do {
			if (isdigit(*p)) {
				bit = *p - '0';
				if (bit < size)
					bitmap_set(bitmap, bit, 1);
				else
					return -1;
			} else
				return -1;
			p++;
			len++;
		} while (*p != ']');
		len++;
	} else
		len = -1;

	return len;
}

/* bgx_pki_bitmap_set:		Set the bitmap bits for all elements (node, bgx,
 *				and lmac) selected by a pki lmac string.
 *
 *  str:			Pki lmac string to process.
 *  node:			Updated with the nodes specified in the pki lmac
 *				string.
 *  bgx:			Updated with the bgx's specified in the pki lmac
 *				string.
 *  lmac:			Updated with the lmacs specified in the pki lmac
 *				string.
 *
 *  returns:			Zero on success, error otherwise.
 */
static unsigned long bgx_pki_bitmap_set(const char *str, unsigned long *node,
					unsigned long *bgx, unsigned long *lmac)
{
	const char	*p = str;
	int		len;

	/* Parse the node */
	len = bgx_parse_pki_elem(p, node, MAX_NODES);
	if (len < 0)
		goto err;

	/* Parse the bgx */
	p += len;
	len = bgx_parse_pki_elem(p, bgx, MAX_BGX_PER_NODE);
	if (len < 0)
		goto err;

	/* Parse the lmac */
	p += len;
	len = bgx_parse_pki_elem(p, lmac, MAX_LMAC_PER_NODE);
	if (len < 0)
		goto err;

	return 0;
 err:
	bitmap_zero(node, MAX_NODES);
	bitmap_zero(bgx, MAX_BGX_PER_NODE);
	bitmap_zero(lmac, MAX_LMAC_PER_NODE);
	return len;

}

/* bgx_pki_init_from_param:	Initialize the list of lmacs that connect to the
 *				pki from information in the "pki_port"
 *				parameter.
 *
 *				The pki_port parameter format is as follows:
 *				pki_port=nbl
 *				where:
 *					n = node
 *					b = bgx
 *					l = lmac
 *
 *				Commas must be used to separate multiple lmacs:
 *				pki_port=000,100,110
 *
 *				Asterisks (*) specify all possible characters in
 *				the subset:
 *				pki_port=00* (all lmacs of node0 bgx0).
 *
 *				Missing lmacs identifiers default to all
 *				possible characters in the subset:
 *				pki_port=00 (all lmacs on node0 bgx0)
 *
 *				Brackets ('[' and ']') specify the valid
 *				characters in the subset:
 *				pki_port=00[01] (lmac0 and lmac1 of node0 bgx0).
 *
 *  returns:			Zero on success, error otherwise.
 */
static int bgx_pki_init_from_param(void)
{
	char	*cur;
	char	*next;
	DECLARE_BITMAP(node_bitmap, MAX_NODES);
	DECLARE_BITMAP(bgx_bitmap, MAX_BGX_PER_NODE);
	DECLARE_BITMAP(lmac_bitmap, MAX_LMAC_PER_NODE);

	/* Parse each comma separated lmac specifier */
	cur = pki_port;
	while (cur) {
		unsigned long	node;
		unsigned long	bgx;
		unsigned long	lmac;

		bitmap_zero(node_bitmap, BITS_PER_LONG);
		bitmap_zero(bgx_bitmap, BITS_PER_LONG);
		bitmap_zero(lmac_bitmap, BITS_PER_LONG);

		next = strchr(cur, ',');
		if (next)
			*next++ = '\0';

		/* Convert the specifier into a bitmap */
		bgx_pki_bitmap_set(cur, node_bitmap, bgx_bitmap, lmac_bitmap);

		/* Mark the lmacs to be connected to the pki */
		for_each_set_bit(node, node_bitmap, MAX_NODES) {
			for_each_set_bit(bgx, bgx_bitmap, MAX_BGX_PER_NODE) {
				for_each_set_bit(lmac, lmac_bitmap,
						 MAX_LMAC_PER_NODE)
					pki_ports[node][bgx][lmac] = true;
			}
		}

		cur = next;
	}

	return 0;
}

/* bgx_pki_ports_init:		Initialize the pki_ports variable with the
 *				lmacs that connect to the pki.
 *
 *  returns:			Zero on success, error otherwise.
 */
static int bgx_pki_ports_init(void)
{
	int	i, j, k;
	bool	def_val;

	/* Whether all ports default to connect to the pki or not depend on the
	 * passed module parameter (if any).
	 */
	if (pki_port)
		def_val = false;
	else
		def_val = true;

	for (i = 0; i < MAX_NODES; i++) {
		for (j = 0; j < MAX_BGX_PER_NODE; j++) {
			for (k = 0; k < MAX_LMAC_PER_NODE; k++)
				pki_ports[i][j][k] = def_val;
		}
	}

	/* Check if ports have to be individually configured */
	if (pki_port && strcmp(pki_port, "none"))
		bgx_pki_init_from_param();

	return 0;
}

static int bgx_remove(struct platform_device *pdev)
{
	return 0;
}

static void bgx_shutdown(struct platform_device *pdev)
{
	return;
}

static struct of_device_id bgx_match[] = {
	{
		.compatible = "cavium,octeon-7890-bgx",
	},
	{},
};
MODULE_DEVICE_TABLE(of, bgx_match);

static struct platform_driver bgx_driver = {
	.probe		= bgx_probe,
	.remove		= bgx_remove,
	.shutdown       = bgx_shutdown,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= KBUILD_MODNAME,
		.of_match_table = bgx_match,
	},
};

/* Allow bgx_port driver to force this driver to load */
void bgx_nexus_load(void)
{
}
EXPORT_SYMBOL(bgx_nexus_load);

static int __init bgx_driver_init(void)
{
	int r;

	INIT_LIST_HEAD(&pdev_list);
	mutex_init(&pdev_list_lock);

	bgx_mix_port_lmacs_init();
	bgx_pki_ports_init();

	r = platform_driver_register(&bgx_driver);

	return r;
}

static void __exit bgx_driver_exit(void)
{
	struct pdev_list_item *pdev_item;

	mutex_lock(&pdev_list_lock);
	while (!list_empty(&pdev_list)) {
		pdev_item = list_first_entry(&pdev_list, struct pdev_list_item,
					     list);
		list_del(&pdev_item->list);
		platform_device_unregister(pdev_item->pdev);
		kfree(pdev_item);
	}
	mutex_unlock(&pdev_list_lock);

	platform_driver_unregister(&bgx_driver);
}

module_init(bgx_driver_init);
module_exit(bgx_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Cavium Networks <support@caviumnetworks.com>");
MODULE_DESCRIPTION("Cavium Networks BGX MAC Nexus driver.");
