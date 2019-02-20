/*
 * Copyright (C) 2004 Luca Berra
 * Copyright (C) 2004-2008 Red Hat, Inc. All rights reserved.
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

#include "lib/misc/lib.h"
#include "lib/device/dev-type.h"
#include "lib/mm/xlate.h"
#ifdef UDEV_SYNC_SUPPORT
#include <libudev.h> /* for MD detection using udev db records */
#include "lib/device/dev-ext-udev-constants.h"
#endif

#ifdef __linux__

/* Lifted from <linux/raid/md_p.h> because of difficulty including it */

#define MD_SB_MAGIC 0xa92b4efc
#define MD_RESERVED_BYTES (64 * 1024ULL)
#define MD_RESERVED_SECTORS (MD_RESERVED_BYTES / 512)
#define MD_NEW_SIZE_SECTORS(x) (((x) & ~(MD_RESERVED_SECTORS - 1)) \
				- MD_RESERVED_SECTORS)
#define MD_MAX_SYSFS_SIZE 64

static int _dev_has_md_magic(struct device *dev, uint64_t sb_offset)
{
	uint32_t md_magic;

	/* Version 1 is little endian; version 0.90.0 is machine endian */

	if (!dev_read_bytes(dev, sb_offset, sizeof(uint32_t), &md_magic))
		return_0;

	if ((md_magic == MD_SB_MAGIC) ||
	     ((MD_SB_MAGIC != xlate32(MD_SB_MAGIC)) && (md_magic == xlate32(MD_SB_MAGIC))))
		return 1;

	return 0;
}

/*
 * Calculate the position of the superblock.
 * It is always aligned to a 4K boundary and
 * depending on minor_version, it can be:
 * 0: At least 8K, but less than 12K, from end of device
 * 1: At start of device
 * 2: 4K from start of device.
 */
typedef enum {
	MD_MINOR_VERSION_MIN,
	MD_MINOR_V0 = MD_MINOR_VERSION_MIN,
	MD_MINOR_V1,
	MD_MINOR_V2,
	MD_MINOR_VERSION_MAX = MD_MINOR_V2
} md_minor_version_t;

static uint64_t _v1_sb_offset(uint64_t size, md_minor_version_t minor_version)
{
	uint64_t sb_offset;

	switch(minor_version) {
	case MD_MINOR_V0:
		sb_offset = (size - 8 * 2) & ~(4 * 2 - 1ULL);
		break;
	case MD_MINOR_V1:
		sb_offset = 0;
		break;
	case MD_MINOR_V2:
		sb_offset = 4 * 2;
		break;
	default:
		log_warn(INTERNAL_ERROR "WARNING: Unknown minor version %d.",
			 minor_version);
		return 0;
	}
	sb_offset <<= SECTOR_SHIFT;

	return sb_offset;
}

#ifdef UDEV_SYNC_SUPPORT
static int _udev_dev_is_md(struct device *dev)
{
	const char *value;
	struct dev_ext *ext;

	if (!(ext = dev_ext_get(dev)))
		return_0;

	if (!(value = udev_device_get_property_value((struct udev_device *)ext->handle, DEV_EXT_UDEV_BLKID_TYPE)))
		return 0;

	return !strcmp(value, DEV_EXT_UDEV_BLKID_TYPE_SW_RAID);
}
#else
static int _udev_dev_is_md(struct device *dev)
{
	return 0;
}
#endif

/*
 * Returns -1 on error
 */
static int _native_dev_is_md(struct device *dev, uint64_t *offset_found, int full)
{
	md_minor_version_t minor;
	uint64_t size, sb_offset;
	int ret;

	if (!scan_bcache)
		return -EAGAIN;

	if (!dev_get_size(dev, &size)) {
		stack;
		return -1;
	}

	if (size < MD_RESERVED_SECTORS * 2)
		return 0;

	/*
	 * Old md versions locate the magic number at the end of the device.
	 * Those checks can't be satisfied with the initial bcache data, and
	 * would require an extra read i/o at the end of every device.  Issuing
	 * an extra read to every device in every command, just to check for
	 * the old md format is a bad tradeoff.
	 *
	 * When "full" is set, we check a the start and end of the device for
	 * md magic numbers.  When "full" is not set, we only check at the
	 * start of the device for the magic numbers.  We decide for each
	 * command if it should do a full check (cmd->use_full_md_check),
	 * and set it for commands that could possibly write to an md dev
	 * (pvcreate/vgcreate/vgextend).
	 */
	if (!full) {
		sb_offset = 0;
		if (_dev_has_md_magic(dev, sb_offset)) {
			log_debug_devs("Found md magic number at offset 0 of %s.", dev_name(dev));
			ret = 1;
			goto out;
		}

		sb_offset = 8 << SECTOR_SHIFT;
		if (_dev_has_md_magic(dev, sb_offset)) {
			log_debug_devs("Found md magic number at offset %d of %s.", (int)sb_offset, dev_name(dev));
			ret = 1;
			goto out;
		}

		ret = 0;
		goto out;
	}

	/* Check if it is an md component device. */
	/* Version 0.90.0 */
	sb_offset = MD_NEW_SIZE_SECTORS(size) << SECTOR_SHIFT;
	if (_dev_has_md_magic(dev, sb_offset)) {
		ret = 1;
		goto out;
	}

	minor = MD_MINOR_VERSION_MIN;
	/* Version 1, try v1.0 -> v1.2 */
	do {
		sb_offset = _v1_sb_offset(size, minor);
		if (_dev_has_md_magic(dev, sb_offset)) {
			ret = 1;
			goto out;
		}
	} while (++minor <= MD_MINOR_VERSION_MAX);

	ret = 0;
out:
	if (ret && offset_found)
		*offset_found = sb_offset;

	return ret;
}

int dev_is_md(struct device *dev, uint64_t *offset_found, int full)
{
	int ret;

	/*
	 * If non-native device status source is selected, use it
	 * only if offset_found is not requested as this
	 * information is not in udev db.
	 */
	if ((dev->ext.src == DEV_EXT_NONE) || offset_found) {
		ret = _native_dev_is_md(dev, offset_found, full);

		if (!full) {
			if (!ret || (ret == -EAGAIN)) {
				if (udev_dev_is_md_component(dev))
					return 1;
			}
		}
		return ret;
	}

	if (dev->ext.src == DEV_EXT_UDEV)
		return _udev_dev_is_md(dev);

	log_error(INTERNAL_ERROR "Missing hook for MD device recognition "
		  "using external device info source %s", dev_ext_name(dev));

	return -1;

}

static int _md_sysfs_attribute_snprintf(char *path, size_t size,
					struct dev_types *dt,
					struct device *blkdev,
					const char *attribute)
{
	const char *sysfs_dir = dm_sysfs_dir();
	struct stat info;
	dev_t dev = blkdev->dev;
	int ret = -1;

	if (!sysfs_dir || !*sysfs_dir)
		return ret;

	if (MAJOR(dev) == dt->blkext_major) {
		/* lookup parent MD device from blkext partition */
		if (!dev_get_primary_dev(dt, blkdev, &dev))
			return ret;
	}

	if (MAJOR(dev) != dt->md_major)
		return ret;

	ret = dm_snprintf(path, size, "%s/dev/block/%d:%d/md/%s", sysfs_dir,
			  (int)MAJOR(dev), (int)MINOR(dev), attribute);
	if (ret < 0) {
		log_error("dm_snprintf md %s failed", attribute);
		return ret;
	}

	if (stat(path, &info) == -1) {
		if (errno != ENOENT) {
			log_sys_error("stat", path);
			return ret;
		}
		/* old sysfs structure */
		ret = dm_snprintf(path, size, "%s/block/md%d/md/%s",
				  sysfs_dir, (int)MINOR(dev), attribute);
		if (ret < 0) {
			log_error("dm_snprintf old md %s failed", attribute);
			return ret;
		}
	}

	return ret;
}

static int _md_sysfs_attribute_scanf(struct dev_types *dt,
				     struct device *dev,
				     const char *attribute_name,
				     const char *attribute_fmt,
				     void *attribute_value)
{
	char path[PATH_MAX+1], buffer[MD_MAX_SYSFS_SIZE];
	FILE *fp;
	int ret = 0;

	if (_md_sysfs_attribute_snprintf(path, PATH_MAX, dt,
					 dev, attribute_name) < 0)
		return ret;

	if (!(fp = fopen(path, "r"))) {
		log_sys_error("fopen", path);
		return ret;
	}

	if (!fgets(buffer, sizeof(buffer), fp)) {
		log_sys_error("fgets", path);
		goto out;
	}

	if ((ret = sscanf(buffer, attribute_fmt, attribute_value)) != 1) {
		log_error("%s sysfs attr %s not in expected format: %s",
			  dev_name(dev), attribute_name, buffer);
		goto out;
	}

out:
	if (fclose(fp))
		log_sys_error("fclose", path);

	return ret;
}

/*
 * Retrieve chunk size from md device using sysfs.
 */
static unsigned long _dev_md_chunk_size(struct dev_types *dt, struct device *dev)
{
	const char *attribute = "chunk_size";
	unsigned long chunk_size_bytes = 0UL;

	if (_md_sysfs_attribute_scanf(dt, dev, attribute,
				      "%lu", &chunk_size_bytes) != 1)
		return 0;

	log_very_verbose("Device %s %s is %lu bytes.",
			 dev_name(dev), attribute, chunk_size_bytes);

	return chunk_size_bytes >> SECTOR_SHIFT;
}

/*
 * Retrieve level from md device using sysfs.
 */
static int _dev_md_level(struct dev_types *dt, struct device *dev)
{
	char level_string[MD_MAX_SYSFS_SIZE];
	const char *attribute = "level";
	int level = -1;

	if (_md_sysfs_attribute_scanf(dt, dev, attribute,
				      "%s", &level_string) != 1)
		return -1;

	log_very_verbose("Device %s %s is %s.",
			 dev_name(dev), attribute, level_string);

	/*  We only care about raid - ignore linear/faulty/multipath etc. */
	if (sscanf(level_string, "raid%d", &level) != 1)
		return -1;

	return level;
}

/*
 * Retrieve raid_disks from md device using sysfs.
 */
static int _dev_md_raid_disks(struct dev_types *dt, struct device *dev)
{
	const char *attribute = "raid_disks";
	int raid_disks = 0;

	if (_md_sysfs_attribute_scanf(dt, dev, attribute,
				      "%d", &raid_disks) != 1)
		return 0;

	log_very_verbose("Device %s %s is %d.",
			 dev_name(dev), attribute, raid_disks);

	return raid_disks;
}

/*
 * Calculate stripe width of md device using its sysfs files.
 */
unsigned long dev_md_stripe_width(struct dev_types *dt, struct device *dev)
{
	unsigned long chunk_size_sectors = 0UL;
	unsigned long stripe_width_sectors = 0UL;
	int level, raid_disks, data_disks;

	chunk_size_sectors = _dev_md_chunk_size(dt, dev);
	if (!chunk_size_sectors)
		return 0;

	level = _dev_md_level(dt, dev);
	if (level < 0)
		return 0;

	raid_disks = _dev_md_raid_disks(dt, dev);
	if (!raid_disks)
		return 0;

	/* The raid level governs the number of data disks. */
	switch (level) {
	case 0:
		/* striped md does not have any parity disks */
		data_disks = raid_disks;
		break;
	case 1:
	case 10:
		/* mirrored md effectively has 1 data disk */
		data_disks = 1;
		break;
	case 4:
	case 5:
		/* both raid 4 and 5 have a single parity disk */
		data_disks = raid_disks - 1;
		break;
	case 6:
		/* raid 6 has 2 parity disks */
		data_disks = raid_disks - 2;
		break;
	default:
		log_error("Device %s has an unknown md raid level: %d",
			  dev_name(dev), level);
		return 0;
	}

	stripe_width_sectors = chunk_size_sectors * data_disks;

	log_very_verbose("Device %s stripe-width is %lu bytes.",
			 dev_name(dev),
			 stripe_width_sectors << SECTOR_SHIFT);

	return stripe_width_sectors;
}

int dev_is_md_with_end_superblock(struct dev_types *dt, struct device *dev)
{
	char version_string[MD_MAX_SYSFS_SIZE];
	const char *attribute = "metadata_version";

	if (MAJOR(dev->dev) != dt->md_major)
		return 0;

	if (_md_sysfs_attribute_scanf(dt, dev, attribute,
				      "%s", &version_string) != 1)
		return -1;

	log_very_verbose("Device %s %s is %s.",
			 dev_name(dev), attribute, version_string);

	if (!strcmp(version_string, "1.0") || !strcmp(version_string, "0.90"))
		return 1;
	return 0;
}

#else

int dev_is_md(struct device *dev __attribute__((unused)),
	      uint64_t *sb __attribute__((unused)))
{
	return 0;
}

unsigned long dev_md_stripe_width(struct dev_types *dt __attribute__((unused)),
				  struct device *dev __attribute__((unused)))
{
	return 0UL;
}

#endif
