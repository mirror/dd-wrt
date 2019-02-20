/*
 * Copyright (C) 2002-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2007 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Changelog
 *
 *   05/02/2002 - First drop [HM]
 */

#include "tools.h"

int disks_found;
int parts_found;
int pv_disks_found;
int pv_parts_found;
int max_len;

static int _get_max_dev_name_len(struct cmd_context *cmd, struct dev_filter *filter)
{
	int len = 0;
	int maxlen = 0;
	struct dev_iter *iter;
	struct device *dev;

	if (!(iter = dev_iter_create(filter, 1))) {
		log_error("dev_iter_create failed");
		return 0;
	}

	/* Do scan */
	for (dev = dev_iter_get(cmd, iter); dev; dev = dev_iter_get(cmd, iter)) {
		len = strlen(dev_name(dev));
		if (len > maxlen)
			maxlen = len;
	}
	dev_iter_destroy(iter);

	return maxlen;
}

static void _count(struct device *dev, int *disks, int *parts)
{
	int c = dev_name(dev)[strlen(dev_name(dev)) - 1];

	if (!isdigit(c))
		(*disks)++;
	else
		(*parts)++;
}

static void _print(struct cmd_context *cmd, const struct device *dev,
		   uint64_t size, const char *what)
{
	log_print("%-*s [%15s] %s", max_len, dev_name(dev),
		  display_size(cmd, size), what ? : "");
}

static int _check_device(struct cmd_context *cmd, struct device *dev)
{
	uint64_t size;

	if (!dev_get_size(dev, &size)) {
		log_error("Couldn't get size of \"%s\"", dev_name(dev));
		size = 0;
	}
	_print(cmd, dev, size, NULL);
	_count(dev, &disks_found, &parts_found);

	return 1;
}

int lvmdiskscan(struct cmd_context *cmd, int argc __attribute__((unused)),
		char **argv __attribute__((unused)))
{
	uint64_t size;
	struct dev_iter *iter;
	struct device *dev;

	/* initialise these here to avoid problems with the lvm shell */
	disks_found = 0;
	parts_found = 0;
	pv_disks_found = 0;
	pv_parts_found = 0;

	if (arg_is_set(cmd, lvmpartition_ARG))
		log_warn("WARNING: only considering LVM devices");

	/* Call before using dev_iter which uses filters which want bcache data. */
	label_scan(cmd);

	max_len = _get_max_dev_name_len(cmd, cmd->filter);

	if (!(iter = dev_iter_create(cmd->filter, 0))) {
		log_error("dev_iter_create failed");
		return ECMD_FAILED;
	}

	for (dev = dev_iter_get(cmd, iter); dev; dev = dev_iter_get(cmd, iter)) {
		if (lvmcache_has_dev_info(dev)) {
			if (!dev_get_size(dev, &size)) {
				log_error("Couldn't get size of \"%s\"",
					  dev_name(dev));
				continue;
			}
			_print(cmd, dev, size, "LVM physical volume");
			_count(dev, &pv_disks_found, &pv_parts_found);
			continue;
		}
		/* If user just wants PVs we are done */
		if (arg_is_set(cmd, lvmpartition_ARG))
			continue;

		/* What other device is it? */
		if (!_check_device(cmd, dev))
			continue;
	}
	dev_iter_destroy(iter);

	/* Display totals */
	if (!arg_is_set(cmd, lvmpartition_ARG)) {
		log_print("%d disk%s",
			  disks_found, disks_found == 1 ? "" : "s");
		log_print("%d partition%s",
			  parts_found, parts_found == 1 ? "" : "s");
	}
	log_print("%d LVM physical volume whole disk%s",
		  pv_disks_found, pv_disks_found == 1 ? "" : "s");
	log_print("%d LVM physical volume%s",
		  pv_parts_found, pv_parts_found == 1 ? "" : "s");

	return ECMD_PROCESSED;
}
