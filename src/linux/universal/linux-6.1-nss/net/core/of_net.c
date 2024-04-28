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
			return i;
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

static void *nvmem_cell_get_mac_address(struct nvmem_cell *cell)
{
	size_t len;
	void *mac;

	mac = nvmem_cell_read(cell, &len);
	if (IS_ERR(mac))
		return mac;
	if (len != ETH_ALEN) {
		kfree(mac);
		return ERR_PTR(-EINVAL);
	}
	return mac;
}

static void *nvmem_cell_get_mac_address_ascii(struct nvmem_cell *cell)
{
	size_t len;
	int ret;
	void *mac_ascii;
	u8 *mac;

	mac_ascii = nvmem_cell_read(cell, &len);
	if (IS_ERR(mac_ascii))
		return mac_ascii;
	if (len != ETH_ALEN*2+5) {
		kfree(mac_ascii);
		return ERR_PTR(-EINVAL);
	}
	mac = kmalloc(ETH_ALEN, GFP_KERNEL);
	if (!mac) {
		kfree(mac_ascii);
		return ERR_PTR(-ENOMEM);
	}
	ret = sscanf(mac_ascii, "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx",
				&mac[0], &mac[1], &mac[2],
				&mac[3], &mac[4], &mac[5]);
	kfree(mac_ascii);
	if (ret == ETH_ALEN)
		return mac;
	kfree(mac);
	return ERR_PTR(-EINVAL);
}

static struct nvmem_cell_mac_address_property {
	char *name;
	void *(*read)(struct nvmem_cell *);
} nvmem_cell_mac_address_properties[] = {
	{
		.name = "mac-address",
		.read = nvmem_cell_get_mac_address,
	}, {
		.name = "mac-address-ascii",
		.read = nvmem_cell_get_mac_address_ascii,
	},
};

static int of_get_mac_addr_nvmem(struct device_node *np, u8 *addr)
{
	struct platform_device *pdev = of_find_device_by_node(np);
	struct nvmem_cell_mac_address_property *property;
	struct nvmem_cell *cell;
	const void *mac;
	int ret, i;

	/* Try lookup by device first, there might be a nvmem_cell_lookup
	 * associated with a given device.
	 */
	if (pdev) {
		ret = nvmem_get_mac_address(&pdev->dev, addr);
		put_device(&pdev->dev);
		return ret;
	}

	for (i = 0; i < ARRAY_SIZE(nvmem_cell_mac_address_properties); i++) {
		property = &nvmem_cell_mac_address_properties[i];
		cell = of_nvmem_cell_get(np, property->name);
		/* For -EPROBE_DEFER don't try other properties.
		 * We'll get back to this one.
		 */
		if (!IS_ERR(cell) || PTR_ERR(cell) == -EPROBE_DEFER)
			break;
	}

	if (IS_ERR(cell))
		return PTR_ERR(cell);

	mac = property->read(cell);
	nvmem_cell_put(cell);

	if (IS_ERR(mac))
		return PTR_ERR(mac);

	if (!is_valid_ether_addr(mac)) {
		kfree(mac);
		return -EINVAL;
	}

	memcpy(addr, mac, ETH_ALEN);
	kfree(mac);

	return 0;
}

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
		return addr;
	}

	prop = kzalloc(sizeof(*prop), GFP_KERNEL);
	if (!prop)
		return -ENOMEM;

	prop->name = "mac-address";
	prop->length = ETH_ALEN;
	prop->value = kmemdup(mac, ETH_ALEN, GFP_KERNEL);
	if (!prop->value || of_add_property(np, prop))
		goto free;

	return prop->value;
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
 * DT can tell the system to increment the mac-address after is extracted by
 * using:
 * - mac-address-increment-byte to decide what byte to increase
 *   (if not defined is increased the last byte)
 * - mac-address-increment to decide how much to increase. The value WILL
 *   overflow to other bytes if the increment is over 255 or the total
 *   increment will exceed 255 of the current byte.
 *   (example 00:01:02:03:04:ff + 1 == 00:01:02:03:05:00)
 *   (example 00:01:02:03:04:fe + 5 == 00:01:02:03:05:03)
 *
 * Return: 0 on success and errno in case of error.
*/
int of_get_mac_address(struct device_node *np, u8 *addr)
{
	u32 inc_idx, mac_inc, mac_val;
	int ret;

	ret = of_get_mac_address_mtd(np, addr);
	if (!ret)
		return addr;

	/* Check first if the increment byte is present and valid.
	 * If not set assume to increment the last byte if found.
	 */
	if (of_property_read_u32(np, "mac-address-increment-byte", &inc_idx))
		inc_idx = 5;
	if (inc_idx < 3 || inc_idx > 5)
		return -EINVAL;

	if (!np)
		return -ENODEV;

	ret = of_get_mac_addr(np, "mac-address", addr);
	if (!ret)
		goto found;

	ret = of_get_mac_addr(np, "local-mac-address", addr);
	if (!ret)
		goto found;

	ret = of_get_mac_addr(np, "address", addr);
	if (!ret)
		goto found;

	ret = of_get_mac_addr_nvmem(np, addr);
	if (ret)
		return ret;

found:
	if (!of_property_read_u32(np, "mac-address-increment", &mac_inc)) {
		/* Convert to a contiguous value */
		mac_val = (addr[3] << 16) + (addr[4] << 8) + addr[5];
		mac_val += mac_inc << 8 * (5-inc_idx);

		/* Apply the incremented value handling overflow case */
		addr[3] = (mac_val >> 16) & 0xff;
		addr[4] = (mac_val >> 8) & 0xff;
		addr[5] = (mac_val >> 0) & 0xff;

		/* Remove mac-address-increment and mac-address-increment-byte
		 * DT property to make sure MAC address would not get incremented
		 * more if this function is stared again. */
		of_remove_property(np, of_find_property(np, "mac-address-increment", NULL));
		of_remove_property(np, of_find_property(np, "mac-address-increment-byte", NULL));
	}

	of_add_mac_address(np, addr);
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
