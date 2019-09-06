/*
	Copyright (C) 2004 - 2009 Ivo van Doorn <IvDoorn@gmail.com>
	Copyright (C) 2004 - 2009 Gertjan van Wingerde <gwingerde@gmail.com>
	<http://rt2x00.serialmonkey.com>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the
	Free Software Foundation, Inc.,
	59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
	Module: rt2x00lib
	Abstract: rt2x00 eeprom file loading routines.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/of.h>

#include "rt2x00.h"
#include "rt2x00lib.h"

static int rt2800lib_read_eeprom_mtd(struct rt2x00_dev *rt2x00dev)
{
	int ret = -EINVAL;
#ifdef CONFIG_OF
	static struct firmware mtd_fw;
	struct device_node *np = rt2x00dev->dev->of_node, *mtd_np = NULL;
	size_t retlen, len = rt2x00dev->ops->eeprom_size;
	int i, size, offset = 0;
	struct mtd_info *mtd;
	const char *part;
	const __be32 *list;
	phandle phandle;

	list = of_get_property(np, "ralink,mtd-eeprom", &size);
	if (!list)
		return -ENOENT;

	phandle = be32_to_cpup(list++);
	if (phandle)
		mtd_np = of_find_node_by_phandle(phandle);
	if (!mtd_np) {
		dev_err(rt2x00dev->dev, "failed to load mtd phandle\n");
		return -EINVAL;
	}

	part = of_get_property(mtd_np, "label", NULL);
	if (!part)
		part = mtd_np->name;

	mtd = get_mtd_device_nm(part);
	if (IS_ERR(mtd)) {
		dev_err(rt2x00dev->dev, "failed to get mtd device \"%s\"\n", part);
		return PTR_ERR(mtd);
	}

	if (size > sizeof(*list))
		offset = be32_to_cpup(list);

	ret = mtd_read(mtd, offset, len, &retlen, (u_char *) rt2x00dev->eeprom);
	put_mtd_device(mtd);

	if ((retlen != rt2x00dev->ops->eeprom_size) || ret) {
		dev_err(rt2x00dev->dev, "failed to load eeprom from device \"%s\"\n", part);
		return ret;
	}

	if (of_find_property(np, "ralink,mtd-eeprom-swap", NULL))
		for (i = 0; i < len/sizeof(u16); i++)
			rt2x00dev->eeprom[i] = swab16(rt2x00dev->eeprom[i]);

	rt2x00dev->eeprom_file = &mtd_fw;
	mtd_fw.size = len;
	mtd_fw.data = (const u8 *) rt2x00dev->eeprom;

	dev_info(rt2x00dev->dev, "loaded eeprom from mtd device \"%s\"\n", part);
#endif

	return ret;
}

static const char *
rt2x00lib_get_eeprom_file_name(struct rt2x00_dev *rt2x00dev)
{
	struct rt2x00_platform_data *pdata = rt2x00dev->dev->platform_data;
#ifdef CONFIG_OF
	struct device_node *np;
	const char *eep;
#endif

	if (pdata && pdata->eeprom_file_name)
		return pdata->eeprom_file_name;

#ifdef CONFIG_OF
	np = rt2x00dev->dev->of_node;
	if (np && of_property_read_string(np, "ralink,eeprom", &eep) == 0)
	    return eep;
#endif

	return NULL;
}

static int rt2x00lib_request_eeprom_file(struct rt2x00_dev *rt2x00dev)
{
	const struct firmware *ee;
	const char *ee_name;
	int retval;

	if (!rt2800lib_read_eeprom_mtd(rt2x00dev))
		return 0;

	ee_name = rt2x00lib_get_eeprom_file_name(rt2x00dev);
	if (!ee_name && test_bit(REQUIRE_EEPROM_FILE, &rt2x00dev->cap_flags)) {
		rt2x00_err(rt2x00dev, "Required EEPROM name is missing.");
		return -EINVAL;
	}

	if (!ee_name)
		return 0;

	rt2x00_info(rt2x00dev, "Loading EEPROM data from '%s'.\n", ee_name);

	retval = request_firmware(&ee, ee_name, rt2x00dev->dev);
	if (retval) {
		rt2x00_err(rt2x00dev, "Failed to request EEPROM.\n");
		return retval;
	}

	if (!ee || !ee->size || !ee->data) {
		rt2x00_err(rt2x00dev, "Failed to read EEPROM file.\n");
		retval = -ENOENT;
		goto err_exit;
	}

	if (ee->size != rt2x00dev->ops->eeprom_size) {
		rt2x00_err(rt2x00dev,
			   "EEPROM file size is invalid, it should be %d bytes\n",
			   rt2x00dev->ops->eeprom_size);
		retval = -EINVAL;
		goto err_release_ee;
	}

	rt2x00dev->eeprom_file = ee;
	return 0;

err_release_ee:
	release_firmware(ee);
err_exit:
	return retval;
}

int rt2x00lib_load_eeprom_file(struct rt2x00_dev *rt2x00dev)
{
	int retval;

	retval = rt2x00lib_request_eeprom_file(rt2x00dev);
	if (retval)
		return retval;

	return 0;
}

void rt2x00lib_free_eeprom_file(struct rt2x00_dev *rt2x00dev)
{
	release_firmware(rt2x00dev->eeprom_file);
	rt2x00dev->eeprom_file = NULL;
}
