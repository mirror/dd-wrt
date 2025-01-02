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
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>

#include "octeon-bgx.h"
#include "octeon-common-nexus.h"


/* Created platform devices get added to this list */
static struct list_head pdev_list;
static struct mutex pdev_list_lock;

/* Created platform device use this structure to add themselves to the list */
struct pdev_list_item {
	struct list_head	list;
	struct platform_device	*pdev;
};


static int srio_probe(struct platform_device *pdev)
{
	struct mac_platform_data	platform_data;
	struct device_node		*child;
	const __be32			*reg;
	u64				addr;
	int				numa_node;
	int				interface;
	int				r = 0;

	reg = of_get_property(pdev->dev.of_node, "reg", NULL);
	addr = of_translate_address(pdev->dev.of_node, reg);
	interface = ((addr >> 24) & 0x1) + SRIO_INTERFACE_OFFSET;
	numa_node = (addr >> 36) & 0x7;

	/* Initialize the node's ports' configuration data */
	octeon3_init_port_cfg_data(numa_node);

	for_each_available_child_of_node(pdev->dev.of_node, child) {
		u32			port;
		struct pdev_list_item	*pdev_item;
		struct platform_device	*new_dev;
		struct platform_device	*pki_dev;
		int			pki_id;
		char			id[64];

		if (!of_device_is_compatible(child,
					     "cavium,octeon-7890-srio-port"))
			continue;

		r = of_property_read_u32(child, "reg", &port);
		if (r)
			return -ENODEV;

		/* Create a platform device for this child node */
		snprintf(id, sizeof(id), "%llx.%u.ethernet-mac",
			 (unsigned long long)addr, port);
		new_dev = of_platform_device_create(child, id, &pdev->dev);
		if (!new_dev) {
			dev_err(&pdev->dev, "Error creating %s\n", id);
			continue;
		}
		platform_data.mac_type = SRIO_MAC;
		platform_data.numa_node = numa_node;
		platform_data.interface = interface;
		platform_data.port = port;

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
							"ethernet-mac-pki",
							pki_id,
							&platform_data,
							sizeof(platform_data));
		dev_info(&pdev->dev, "Created %s %u: %p\n", "SRIO-PKI",
			 pki_dev->id, pki_dev);

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

		/* Load the ethernet driver */
		octeon3_load_eth_driver();
	}

	dev_info(&pdev->dev, "Probed\n");
	return 0;
}

static int srio_remove(struct platform_device *pdev)
{
	return 0;
}

static void srio_shutdown(struct platform_device *pdev)
{
	return;
}

static struct of_device_id srio_match[] = {
	{
		.compatible = "cavium,octeon-7890-srio",
	},
	{},
};
MODULE_DEVICE_TABLE(of, srio_match);

static struct platform_driver srio_driver = {
	.probe		= srio_probe,
	.remove		= srio_remove,
	.shutdown       = srio_shutdown,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= KBUILD_MODNAME,
		.of_match_table = srio_match,
	},
};

void srio_nexus_load(void)
{
}
EXPORT_SYMBOL(srio_nexus_load);

static int __init srio_driver_init(void)
{
	int rc;

	INIT_LIST_HEAD(&pdev_list);
	mutex_init(&pdev_list_lock);

	rc = platform_driver_register(&srio_driver);

	return rc;
}

static void __exit srio_driver_exit(void)
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

	platform_driver_unregister(&srio_driver);
}

module_init(srio_driver_init);
module_exit(srio_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Carlos Munoz <cmunoz@caviumnetworks.com>");
MODULE_DESCRIPTION("Cavium Networks SRIO MAC Nexus driver.");
