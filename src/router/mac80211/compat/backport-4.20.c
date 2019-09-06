/*
 * Copyright(c) 2016 Hauke Mehrtens <hauke@hauke-m.de>
 *
 * Backport functionality introduced in Linux 4.7.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/export.h>
#include <linux/list.h>
#include <linux/rcupdate.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/firmware.h>
#include <linux/module.h>

int firmware_request_nowarn(const struct firmware **firmware, const char *name,
			    struct device *device)
{
	return request_firmware(firmware, name, device);
}
EXPORT_SYMBOL_GPL(firmware_request_nowarn);

