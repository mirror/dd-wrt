/*
 * Copyright (C) 2004 Luca Berra
 * Copyright (C) 2004-2006 Red Hat, Inc. All rights reserved.
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
#include "lib/misc/lib.h"
#include "lib/filters/filter.h"
#include "lib/commands/toolcontext.h"

#ifdef __linux__

#define MSG_SKIPPING "%s: Skipping md component device"

/*
 * The purpose of these functions is to ignore md component devices,
 * e.g. if /dev/md0 is a raid1 composed of /dev/loop0 and /dev/loop1,
 * lvm wants to deal with md0 and ignore loop0 and loop1.  md0 should
 * pass the filter, and loop0,loop1 should not pass the filter so lvm
 * will ignore them.
 *
 * (This is assuming lvm.conf md_component_detection=1.)
 *
 * If lvm does *not* ignore the components, then lvm may read lvm
 * labels from the component devs and potentially the md dev,
 * which can trigger duplicate detection, and/or cause lvm to display
 * md components as PVs rather than ignoring them.
 *
 * If scanning md componenents causes duplicates to be seen, then
 * the lvm duplicate resolution will exclude the components.
 *
 * The lvm md filter has three modes:
 *
 * 1. look for md superblock at the start of the device
 * 2. look for md superblock at the start and end of the device
 * 3. use udev to detect components
 *
 * mode 1 will not detect and exclude components of md devices
 * that use superblock version 0.9 or 1.0 which is at the end of the device.
 *
 * mode 2 will detect these, but mode 2 doubles the i/o done by label
 * scan, since there's a read at both the start and end of every device.
 *
 * mode 3 is used when external_device_info_source="udev".  It does
 * not require any io from lvm, but this mode is not used by default
 * because there have been problems getting reliable info from udev.
 *
 * lvm uses mode 2 when:
 *
 * - the command is pvcreate/vgcreate/vgextend, which format new
 *   devices, and if the user ran these commands on a component
 *   device of an md device 0.9 or 1.0, then it would cause problems.
 *   FIXME: this would only really need to scan the end of the
 *   devices being formatted, not all devices.
 *
 * - it sees an md device on the system using version 0.9 or 1.0.
 *   The point of this is just to avoid displaying md components
 *   from the 'pvs' command.
 *   FIXME: the cost (double i/o) may not be worth the benefit
 *   (not showing md components).
 */

/*
 * Returns 0 if:
 * the device is an md component and it should be ignored.
 *
 * Returns 1 if:
 * the device is not md component and should not be ignored.
 *
 * The actual md device will pass this filter and should be used,
 * it is the md component devices that we are trying to exclude
 * that will not pass.
 */

static int _passes_md_filter(struct cmd_context *cmd, struct dev_filter *f __attribute__((unused)), struct device *dev)
{
	int ret;

	/*
	 * When md_component_dectection=0, don't even try to skip md
	 * components.
	 */
	if (!md_filtering())
		return 1;

	ret = dev_is_md(dev, NULL, cmd->use_full_md_check);

	if (ret == -EAGAIN) {
		/* let pass, call again after scan */
		dev->flags |= DEV_FILTER_AFTER_SCAN;
		log_debug_devs("filter md deferred %s", dev_name(dev));
		return 1;
	}

	if (ret == 0)
		return 1;

	if (ret == 1) {
		log_debug_devs("md filter full %d excluding md component %s", cmd->use_full_md_check, dev_name(dev));
		if (dev->ext.src == DEV_EXT_NONE)
			log_debug_devs(MSG_SKIPPING, dev_name(dev));
		else
			log_debug_devs(MSG_SKIPPING " [%s:%p]", dev_name(dev),
					dev_ext_name(dev), dev->ext.handle);
		return 0;
	}

	if (ret < 0) {
		log_debug_devs("%s: Skipping: error in md component detection",
			       dev_name(dev));
		return 0;
	}

	return 1;
}

static void _destroy(struct dev_filter *f)
{
	if (f->use_count)
		log_error(INTERNAL_ERROR "Destroying md filter while in use %u times.", f->use_count);

	free(f);
}

struct dev_filter *md_filter_create(struct cmd_context *cmd, struct dev_types *dt)
{
	struct dev_filter *f;

	if (!(f = zalloc(sizeof(*f)))) {
		log_error("md filter allocation failed");
		return NULL;
	}

	f->passes_filter = _passes_md_filter;
	f->destroy = _destroy;
	f->use_count = 0;
	f->private = dt;

	log_debug_devs("MD filter initialised.");

	return f;
}

#else

struct dev_filter *md_filter_create(struct dev_types *dt)
{
	return NULL;
}

#endif
