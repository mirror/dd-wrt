// SPDX-License-Identifier: GPL-2.0-only
/*
 * OF helpers for network devices.
 *
 * Initially copied out of arch/powerpc/kernel/prom_parse.c
 */
#include <linux/etherdevice.h>
#include <linux/kernel.h>
#include <linux/of_net.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/phy.h>
#include <linux/export.h>
#include <linux/device.h>
#include <linux/nvmem-consumer.h>
#include <linux/mtd/mtd.h>

/**
 * of_get_phy_mode - Get phy mode for given device_node
 * @np:	Pointer to the given device_node
 * @interface: Pointer to the result
 *
 * The function gets phy interface string from property 'phy-mode' or
 * 'phy-connection-type'. The index in phy_modes table is set in
 * interface and 0 returned. In case of error interface is set to
 * PHY_INTERFACE_MODE_NA and an errno is returned, e.g. -ENODEV.
 */
int of_get_phy_mode(struct device_node *np, phy_interface_t *interface)
{
	const char *pm;
	int err, i;

	*interface = PHY_INTERFACE_MODE_NA;

	err = of_property_read_string(np, "phy-mode", &pm);
	if (err < 0)
		err = of_property_read_string(np, "phy-connection-type", &pm);
	if (err < 0)
		return err;

	for (i = 0; i < PHY_INTERFACE_MODE_MAX; i++)
		if (!strcasecmp(pm, phy_modes(i))) {
			*interface = i;
			return 0;
		}

	return -ENODEV;
}
EXPORT_SYMBOL_GPL(of_get_phy_mode);

static int of_get_mac_addr(struct device_node *np, const char *name, u8 *addr)
{
	struct property *pp = of_find_property(np, name, NULL);

	if (pp && pp->length == ETH_ALEN && is_valid_ether_addr(pp->value)) {
		memcpy(addr, pp->value, ETH_ALEN);
		return 0;
	}
	return -ENODEV;
}

int of_get_mac_address_nvmem(struct device_node *np, u8 *addr)
{
	struct platform_device *pdev = of_find_device_by_node(np);
	struct nvmem_cell *cell;
	const void *mac;
	size_t len;
	int ret;

	/* Try lookup by device first, there might be a nvmem_cell_lookup
	 * associated with a given device.
	 */
	if (pdev) {
		ret = nvmem_get_mac_address(&pdev->dev, addr);
		put_device(&pdev->dev);
		return ret;
	}

	cell = of_nvmem_cell_get(np, "mac-address");
	if (IS_ERR(cell))
		return PTR_ERR(cell);

	mac = nvmem_cell_read(cell, &len);
	nvmem_cell_put(cell);

	if (IS_ERR(mac))
		return PTR_ERR(mac);

	if (len != ETH_ALEN || !is_valid_ether_addr(mac)) {
		kfree(mac);
		return -EINVAL;
	}

	memcpy(addr, mac, ETH_ALEN);
	kfree(mac);

	return 0;
}
EXPORT_SYMBOL(of_get_mac_address_nvmem);

static int of_add_mac_address(struct device_node *np, u8* addr)
{
	struct property *prop;

	prop = kzalloc(sizeof(*prop), GFP_KERNEL);
	if (!prop)
		return -ENOMEM;

	prop->name = "mac-address";
	prop->length = ETH_ALEN;
	prop->value = kmemdup(addr, ETH_ALEN, GFP_KERNEL);
	if (!prop->value || of_update_property(np, prop))
		goto free;

	return 0;
free:
	kfree(prop->value);
	kfree(prop);
	return -ENOMEM;
}

static int of_get_mac_address_mtd(struct device_node *np, u8 *addr)
{
#ifdef CONFIG_MTD
	struct device_node *mtd_np = NULL;
	struct property *prop;
	size_t retlen;
	int size, ret;
	struct mtd_info *mtd;
	const char *part;
	const __be32 *list;
	phandle phandle;
	u32 mac_inc = 0;
	u8 mac[ETH_ALEN];

	list = of_get_property(np, "mtd-mac-address", &size);
	if (!list || (size != (2 * sizeof(*list))))
		return -ENOMEM;

	phandle = be32_to_cpup(list++);
	if (phandle)
		mtd_np = of_find_node_by_phandle(phandle);

	if (!mtd_np)
		return -ENOMEM;

	part = of_get_property(mtd_np, "label", NULL);
	if (!part)
		part = mtd_np->name;

	mtd = get_mtd_device_nm(part);
	if (IS_ERR(mtd))
		return -ENOMEM;

	ret = mtd_read(mtd, be32_to_cpup(list), 6, &retlen, mac);
	put_mtd_device(mtd);

	if (!of_property_read_u32(np, "mtd-mac-address-increment", &mac_inc))
		mac[5] += mac_inc;

	if (!is_valid_ether_addr(mac))
		return -ENOMEM;

	ret = of_get_mac_addr(np, "mac-address", addr);
	if (!ret) {
		memcpy(addr, mac, ETH_ALEN);
		return 0;
	}

	prop = kzalloc(sizeof(*prop), GFP_KERNEL);
	if (!prop)
		return -ENOMEM;

	prop->name = "mac-address";
	prop->length = ETH_ALEN;
	prop->value = kmemdup(mac, ETH_ALEN, GFP_KERNEL);
	if (!prop->value || of_add_property(np, prop))
		goto free;

	memcpy(addr, prop->value, ETH_ALEN);
	return 0;
free:
	kfree(prop->value);
	kfree(prop);
#endif
	return -ENOMEM;
}

/**
 * of_get_mac_address()
 * @np:		Caller's Device Node
 * @addr:	Pointer to a six-byte array for the result
 *
 * Search the device tree for the best MAC address to use.  'mac-address' is
 * checked first, because that is supposed to contain to "most recent" MAC
 * address. If that isn't set, then 'local-mac-address' is checked next,
 * because that is the default address. If that isn't set, then the obsolete
 * 'address' is checked, just in case we're using an old device tree. If any
 * of the above isn't set, then try to get MAC address from nvmem cell named
 * 'mac-address'.
 *
 * Note that the 'address' property is supposed to contain a virtual address of
 * the register set, but some DTS files have redefined that property to be the
 * MAC address.
 *
 * All-zero MAC addresses are rejected, because those could be properties that
 * exist in the device tree, but were not set by U-Boot.  For example, the
 * DTS could define 'mac-address' and 'local-mac-address', with zero MAC
 * addresses.  Some older U-Boots only initialized 'local-mac-address'.  In
 * this case, the real MAC is in 'local-mac-address', and 'mac-address' exists
 * but is all zeros.
 *
 * Return: 0 on success and errno in case of error.
*/
int of_get_mac_address(struct device_node *np, u8 *addr)
{
	int ret;


	if (!np)
		return -ENODEV;

	ret = of_get_mac_address_mtd(np, addr);
	if (!ret)
		goto found;

	ret = of_get_mac_addr(np, "mac-address", addr);
	if (!ret)
		goto found;

	ret = of_get_mac_addr(np, "local-mac-address", addr);
	if (!ret)
		goto found;

	ret = of_get_mac_addr(np, "address", addr);
	if (!ret)
		goto found;

	ret = of_get_mac_address_nvmem(np, addr);
	if (ret)
		return ret;

found:
	ret = of_add_mac_address(np, addr);
	return ret;
}
EXPORT_SYMBOL(of_get_mac_address);

/**
 * of_get_ethdev_address()
 * @np:		Caller's Device Node
 * @dev:	Pointer to netdevice which address will be updated
 *
 * Search the device tree for the best MAC address to use.
 * If found set @dev->dev_addr to that address.
 *
 * See documentation of of_get_mac_address() for more information on how
 * the best address is determined.
 *
 * Return: 0 on success and errno in case of error.
 */
int of_get_ethdev_address(struct device_node *np, struct net_device *dev)
{
	u8 addr[ETH_ALEN];
	int ret;

	ret = of_get_mac_address(np, addr);
	if (!ret)
		eth_hw_addr_set(dev, addr);
	return ret;
}
EXPORT_SYMBOL(of_get_ethdev_address);
