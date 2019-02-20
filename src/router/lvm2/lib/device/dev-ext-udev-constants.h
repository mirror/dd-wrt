/*
 * Copyright (C) 2015 Red Hat, Inc. All rights reserved.
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

/*************************************************************************
 * Properties saved in udev db and accesible via libudev and used by LVM *
 *************************************************************************/

/*
 * DEV_EXT_UDEV_BLKID_TYPE property with various DEV_EXT_UDEV_BLKID_TYPE_*
 * values that is saved in udev db via blkid call in udev rules
 */
#define DEV_EXT_UDEV_BLKID_TYPE                 "ID_FS_TYPE"
/*
 * mpath_member is forced by multipath - it's set in udev db via
 * multipath call overwriting any existing ID_FS_TYPE value for
 * a device which is a multipath component which prevents incorrect
 * claim of the device by any other block device subsystem
 */
#define DEV_EXT_UDEV_BLKID_TYPE_MPATH           "mpath_member"
/* FW RAIDs are all *_raid_member types except linux_raid_member which denotes SW RAID */
#define DEV_EXT_UDEV_BLKID_TYPE_RAID_SUFFIX     "_raid_member"
#define DEV_EXT_UDEV_BLKID_TYPE_SW_RAID         "linux_raid_member"
#define DEV_EXT_UDEV_BLKID_PART_TABLE_TYPE      "ID_PART_TABLE_TYPE"

#define DEV_EXT_UDEV_DEVTYPE			"DEVTYPE"
#define DEV_EXT_UDEV_DEVTYPE_DISK		"disk"

/* the list of symlinks associated with device node */
#define DEV_EXT_UDEV_DEVLINKS			"DEVLINKS"

/*
 * DEV_EXT_UDEV_MPATH_DEVICE_PATH is set by multipath in udev db
 * with value either 0 or 1. The same functionality as
 * DEV_EXT_UDEV_BLKID_TYPE_MPATH actually, but introduced later
 * for some reason.
 */
#define DEV_EXT_UDEV_MPATH_DEVICE_PATH          "DM_MULTIPATH_DEVICE_PATH"


/***********************************************************
 * Sysfs attributes accessible via libudev and used by LVM *
 ***********************************************************/

/* the value of size sysfs attribute is size in bytes */
#define DEV_EXT_UDEV_SYSFS_ATTR_SIZE            "size"

