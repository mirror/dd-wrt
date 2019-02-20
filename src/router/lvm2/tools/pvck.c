/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2007 Red Hat, Inc. All rights reserved.
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

#include "base/memory/zalloc.h"
#include "tools.h"

int pvck(struct cmd_context *cmd, int argc, char **argv)
{
	struct dm_list devs;
	struct device_list *devl;
	struct device *dev;
	const char *pv_name;
	uint64_t labelsector;
	int i;
	int ret_max = ECMD_PROCESSED;

	labelsector = arg_uint64_value(cmd, labelsector_ARG, UINT64_C(0));

	dm_list_init(&devs);

	for (i = 0; i < argc; i++) {
		dm_unescape_colons_and_at_signs(argv[i], NULL, NULL);

		pv_name = argv[i];

		dev = dev_cache_get(cmd, pv_name, cmd->filter);

		if (!dev) {
			log_error("Device %s %s.", pv_name, dev_cache_filtered_reason(pv_name));
			continue;
		}

		if (!(devl = zalloc(sizeof(*devl))))
			continue;

		devl->dev = dev;
		dm_list_add(&devs, &devl->list);
	}

	label_scan_setup_bcache();
	label_scan_devs(cmd, cmd->filter, &devs);

	dm_list_iterate_items(devl, &devs) {

		/*
		 * The scan above will populate lvmcache with any info from the
		 * standard locations at the start of the device.  Now populate
		 * lvmcache with any info from non-standard offsets.
		 *
		 * FIXME: is it possible for a real lvm label sector to be
		 * anywhere other than the first four sectors of the disk?
		 * If not, drop the code in label_read_sector/find_lvm_header
		 * that supports searching at any sector.
		 */
		if (labelsector) {
			if (!label_read_sector(devl->dev, labelsector)) {
				stack;
				ret_max = ECMD_FAILED;
				continue;
			}
		}

		if (!pv_analyze(cmd, devl->dev, labelsector)) {
			stack;
			ret_max = ECMD_FAILED;
		}
	}

	return ret_max;
}
