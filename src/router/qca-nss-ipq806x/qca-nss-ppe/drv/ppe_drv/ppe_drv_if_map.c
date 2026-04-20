/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/debugfs.h>
#include <linux/version.h>
#include "ppe_drv.h"

/*
 * Debugfs dentry object.
 */
static struct dentry *ppe_drv_if_map_dentry;

/*
 * Character device stuff - used to communicate status back to user space
 */
static int ppe_drv_if_map_dev_major_id = 0;			/* Major ID of registered char dev from which we can dump out state to userspace */

/*
 * ppe_drv_if_map_type_str
 *	PPE DRV Interface type
 */
static const char *ppe_drv_if_map_type_str[] = {
	"INVALID",	/**< Interface type invalid. */
	"BRIDGE",	/**< Interface type bridge. */
	"LAG",		/**< Interface type LAG. */
	"PPPOE",	/**< Interface type PPPoE. */
	"VLAN",		/**< Interface type VLAN. */
	"PHYSICAL",	/**< Interface type physical port. */
	"VIRTUAL",	/**< Interface type virtual port. */
	"VIRTUAL_PO",	/**< Interface type virtual port for point offload. */
	"VP_L2_TUN",	/**< Interface type VP for hardware L2 tunnel. */
	"VP_L3_TUN",	/**< Interface type VP for hardware L3 tunnel. */
	"EIP",		/**< Interface type EIP. */
	"MAX",		/**< Interface type max. */
};

/*
 * ppe_drv_iface_valid_flags
 * 	PPE DRV valid flags
 */
static const char *ppe_drv_if_map_valid_flags_str[] = {
	"VLAN_OVER_BRIDGE",
	"WAN_IF_VALID",
	"MAX",
};

/*
 * ppe_drv_if_map_write_reset()
 *	Reset the msg buffer, specifying a new initial prefix
 *
 * Returns 0 on success
 */
int ppe_drv_if_map_write_reset(struct ppe_drv_if_map_instance *mfi, char *prefix)
{
	int result;

	mfi->msgp = mfi->msg;
	mfi->msg_len = 0;

	result = snprintf(mfi->prefix, PPE_IF_MAP_FILE_PREFIX_SIZE, "%s", prefix);
	if ((result < 0) || (result >= PPE_IF_MAP_FILE_PREFIX_SIZE)) {
		return -1;
	}

	mfi->prefix_level = 0;
	mfi->prefix_levels[mfi->prefix_level] = result;

	return 0;
}

/*
 * ppe_drv_if_map_prefix_add()
 *	Add another level to the prefix
 *
 * Returns 0 on success
 */
int ppe_drv_if_map_prefix_add(struct ppe_drv_if_map_instance *mfi, char *prefix)
{
	int pxsz;
	int pxremain;
	int result;

	pxsz = mfi->prefix_levels[mfi->prefix_level];
	pxremain = PPE_IF_MAP_FILE_PREFIX_SIZE - pxsz;

	result = snprintf(mfi->prefix + pxsz, pxremain, ".%s", prefix);
	if ((result < 0) || (result >= pxremain)) {
		return -1;
	}

	mfi->prefix_level++;
	ppe_drv_assert(mfi->prefix_level < PPE_IF_MAP_FILE_PREFIX_LEVELS_MAX, "Bad prefix handling\n");
	mfi->prefix_levels[mfi->prefix_level] = pxsz + result;

	return 0;
}

/*
 * ppe_drv_if_map_prefix_index_add()
 *	Add another level (numeric) to the prefix
 *
 * Returns 0 on success
 */
int ppe_drv_if_map_prefix_index_add(struct ppe_drv_if_map_instance *mfi, uint32_t index)
{
	int pxsz;
	int pxremain;
	int result;

	pxsz = mfi->prefix_levels[mfi->prefix_level];
	pxremain = PPE_IF_MAP_FILE_PREFIX_SIZE - pxsz;
	result = snprintf(mfi->prefix + pxsz, pxremain, ".%u", index);
	if ((result < 0) || (result >= pxremain)) {
		return -1;
	}

	mfi->prefix_level++;
	ppe_drv_assert(mfi->prefix_level < PPE_IF_MAP_FILE_PREFIX_LEVELS_MAX, "Bad prefix handling\n");
	mfi->prefix_levels[mfi->prefix_level] = pxsz + result;

	return 0;
}

/*
 * ppe_drv_if_map_prefix_remove()
 *	Remove level from the prefix
 *
 * Returns 0 on success
 */
int ppe_drv_if_map_prefix_remove(struct ppe_drv_if_map_instance *mfi)
{
	int pxsz;

	mfi->prefix_level--;
	ppe_drv_assert(mfi->prefix_level >= 0, "Bad prefix handling\n");
	pxsz = mfi->prefix_levels[mfi->prefix_level];
	mfi->prefix[pxsz] = 0;

	return 0;
}

/*
 * ppe_drv_if_map_write()
 *	Write out to the message buffer, prefix is added automatically.
 *
 * Returns 0 on success
 */
int ppe_drv_if_map_write(struct ppe_drv_if_map_instance *mfi, char *name, char *fmt, ...)
{
	int remain;
	char *ptr;
	int result;
	va_list args;

	remain = PPE_IF_MAP_FILE_BUFFER_SIZE - mfi->msg_len;
	ptr = mfi->msg + mfi->msg_len;
	result = snprintf(ptr, remain, "%s.%s=", mfi->prefix, name);
	if ((result < 0) || (result >= remain)) {
		return -1;
	}

	mfi->msg_len += result;
	remain -= result;
	ptr += result;

	va_start(args, fmt);
	result = vsnprintf(ptr, remain, fmt, args);
	va_end(args);
	if ((result < 0) || (result >= remain)) {
		return -2;
	}

	mfi->msg_len += result;
	remain -= result;
	ptr += result;

	result = snprintf(ptr, remain, "\n");
	if ((result < 0) || (result >= remain)) {
		return -3;
	}

	mfi->msg_len += result;
	return 0;
}

/*
 * ppe_drv_if_map_dump_get()
 *	Prepare a connection message for ipv4
 */
int ppe_drv_if_map_dump_get(struct ppe_drv_if_map_instance *mfi)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_iface *iface;
	struct ppe_drv_stats_if_map *stats;
	int i, active_if = 0, j = 0;
	int port_number, vsi_number, l3_if_number;
	int result;

	stats = kzalloc(p->iface_num * sizeof(struct ppe_drv_stats_if_map), GFP_KERNEL);
	if (!stats) {
		ppe_drv_warn("Error in allocating if_map stats\n");
		return -ENOMEM;
	}

	spin_lock_bh(&p->lock);

	for (i = 0; i < p->iface_num ; i++) {
		iface = &p->iface[i];
		if ((iface->flags & PPE_DRV_IFACE_FLAG_VALID) != PPE_DRV_IFACE_FLAG_VALID) {
			continue;
		}

		stats[active_if].iface_number = iface->index;
		stats[active_if].iface_type = iface->type;
		stats[active_if].base_iface_number = -EINVAL;
		stats[active_if].parent_iface_number = -EINVAL;
		stats[active_if].port_number = -EINVAL;
		stats[active_if].vsi_number = -EINVAL;
		stats[active_if].l3_if_number = -EINVAL;
		port_number = vsi_number = l3_if_number = -EINVAL;
		stats[active_if].iface_flag_cnt = 0;

		if (iface->base_if) {
			stats[active_if].base_iface_number = iface->base_if->index;
		}

		if (iface->flags & PPE_DRV_IFACE_VLAN_OVER_BRIDGE) {
			stats[active_if].iface_valid_flags[stats[active_if].iface_flag_cnt] = 0;
			stats[active_if].iface_flag_cnt++;
		}

		if (iface->flags & PPE_DRV_IFACE_FLAG_WAN_IF_VALID) {
			stats[active_if].iface_valid_flags[stats[active_if].iface_flag_cnt] = 1;
			stats[active_if].iface_flag_cnt++;
		}

		if (iface->parent) {
			stats[active_if].parent_iface_number = iface->parent->index;
		}

		if (iface->dev) {
			memcpy(stats[active_if].netdev_name, iface->dev->name, sizeof(iface->dev->name));
		}

		port_number = ppe_drv_iface_port_idx_get(iface);
		if (port_number != -1) {
			stats[active_if].port_number = port_number;
		}

		vsi_number = ppe_drv_iface_vsi_idx_get(iface);
		if (vsi_number != -1) {
			stats[active_if].vsi_number = vsi_number;
		}

		l3_if_number = ppe_drv_iface_l3_if_idx_get(iface);
		if (l3_if_number != -1) {
			stats[active_if].l3_if_number = l3_if_number;
		}

		active_if++;
	}

	spin_unlock_bh(&p->lock);

	for (i = 0; i < active_if; i++) {

		if ((result = ppe_drv_if_map_prefix_index_add(mfi, mfi->iface_cnt))) {
			goto ppe_drv_if_map_write_error;
		}

		if ((result = ppe_drv_if_map_write(mfi, "iface_number", "%u",stats[i].iface_number))) {
			goto ppe_drv_if_map_write_error;
		}

		if (stats[i].base_iface_number != -EINVAL) {
			if ((result = ppe_drv_if_map_write(mfi, "base_iface_number", "%u",stats[i].base_iface_number))) {
				goto ppe_drv_if_map_write_error;
			}
		}

		if (stats[i].parent_iface_number != -EINVAL) {
			if ((result = ppe_drv_if_map_write(mfi, "parent_iface_number", "%u",stats[i].parent_iface_number))) {
				goto ppe_drv_if_map_write_error;
			}
		}

		for (j = 0;j < stats[i].iface_flag_cnt;j++) {
			if ((result = ppe_drv_if_map_write(mfi, "iface_valid_flags", "%s", ppe_drv_if_map_valid_flags_str[stats[i].iface_valid_flags[j]]))) {
				goto ppe_drv_if_map_write_error;
			}
		}

		if ((result = ppe_drv_if_map_write(mfi, "iface_type", "%s", ppe_drv_if_map_type_str[stats[i].iface_type]))) {
			goto ppe_drv_if_map_write_error;
		}

		if ((result = ppe_drv_if_map_write(mfi, "netdev_name", "%s", stats[i].netdev_name))) {
			goto ppe_drv_if_map_write_error;
		}

		if (stats[i].port_number != -EINVAL) {
			if ((result = ppe_drv_if_map_write(mfi, "port_number", "%u", stats[i].port_number))) {
				goto ppe_drv_if_map_write_error;
			}
		}

		if (stats[i].vsi_number != -EINVAL) {
			if ((result = ppe_drv_if_map_write(mfi, "vsi_number", "%u", stats[i].vsi_number))) {
				goto ppe_drv_if_map_write_error;
			}
		}

		if (stats[i].l3_if_number != -EINVAL) {
			if ((result = ppe_drv_if_map_write(mfi, "l3_if_number", "%u\n", stats[i].l3_if_number))) {
				goto ppe_drv_if_map_write_error;
			}
		}

		if ((result = ppe_drv_if_map_prefix_remove(mfi))) {
			goto ppe_drv_if_map_write_error;
		}

		mfi->iface_cnt++;
	}

	kfree(stats);

	return 0;

ppe_drv_if_map_write_error :
	return result;
}

/*
 * ppe_drv_if_map_char_dev_conn_msg_prep()
 *	Prepare a if_map message
 */
static bool ppe_drv_if_map_char_dev_msg_prep(struct ppe_drv_if_map_instance *mfi)
{
	int result;

	if ((result = ppe_drv_if_map_write_reset(mfi, "Interface"))) {
		return result;
	}

	ppe_drv_trace("Prepare ppe_drv_if_map msg for %px\n", mfi);

	return ppe_drv_if_map_dump_get(mfi);
}

/*
 * ppe_drv_if_map_dev_open()
 *	Opens the special char device file which we use to dump our state.
 */
static int ppe_drv_if_map_dev_open(struct inode *inode, struct file *file)
{
	struct ppe_drv_if_map_instance *mfi;

	ppe_drv_info("if_map open\n");

	/*
	 * Allocate state information for the reading
	 */
	ppe_drv_assert(file->private_data == NULL, "unexpected double open: %px?\n", file->private_data);

	mfi = (struct ppe_drv_if_map_instance *)kzalloc(sizeof(struct ppe_drv_if_map_instance), GFP_ATOMIC | __GFP_NOWARN);
	if (!mfi) {
		return -ENOMEM;
	}

	file->private_data = mfi;
	mfi->iface_cnt = 0;
	mfi->dump_once = 1;

	return 0;
}

/*
 * ppe_drv_if_map_dev_release()
 *	Called when a process closes the device file.
 */
static int ppe_drv_if_map_dev_release(struct inode *inode, struct file *file)
{
	struct ppe_drv_if_map_instance *mfi;

	mfi = (struct ppe_drv_if_map_instance *)file->private_data;

	if (mfi) {
		kfree(mfi);
	}

	return 0;
}

/*
 * ppe_drv_if_map_dev_read()
 *	Called to read the state
 */
static ssize_t ppe_drv_if_map_dev_read(struct file *file,	/* see include/linux/fs.h   */
		char *buffer,				/* buffer to fill with data */
		size_t length,				/* length of the buffer     */
		loff_t *offset)				/* Doesn't apply - this is a char file */
{
	struct ppe_drv_if_map_instance *mfi;
	int bytes_read = 0;						/* Number of bytes actually written to the buffer */
	mfi = (struct ppe_drv_if_map_instance *)file->private_data;
	if (!mfi) {
		return -ENOMEM;
	}

        ppe_drv_assert(file->private_data == NULL, "unexpected double open: %px?\n", file->private_data);

	/*
	 * If there is still some message remaining to be output then complete that first
	 */
	if (mfi->msg_len) {
		goto char_device_read_output;
	}

	if (mfi->dump_once) {
		if (ppe_drv_if_map_char_dev_msg_prep(mfi)) {
			ppe_drv_warn("Failed to create ppe_drv_if_map msg\n");
			return -EIO;
		}

		mfi->dump_once=0;
                goto char_device_read_output;
	}

	return 0;

char_device_read_output:
	/*
	 * If supplied buffer is small we limit what we output
	 */
	bytes_read = mfi->msg_len;
	if (bytes_read > length) {
		bytes_read = length;
	}

	if (copy_to_user(buffer, mfi->msgp, bytes_read)) {
		return -EIO;
	}

	mfi->msg_len -= bytes_read;
	mfi->msgp += bytes_read;

	ppe_drv_trace("State read done, bytes_read %d bytes\n", bytes_read);

	/*
	 * Most read functions return the number of bytes put into the buffer
	 */
	return bytes_read;
}

/*
 * ppe_drv_if_map_dev_write()
 */
static ssize_t ppe_drv_if_map_dev_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	return -EINVAL;
}

/*
 * File operations used in the char device
 *	NOTE: The char device is a simple file that allows us to dump our connection tracking state
 */
static struct file_operations ppe_drv_if_map_fops = {
	.read = ppe_drv_if_map_dev_read,
	.write = ppe_drv_if_map_dev_write,
	.open = ppe_drv_if_map_dev_open,
	.release = ppe_drv_if_map_dev_release
};

/*
 * ppe_drv_if_map_exit()
 */
void ppe_drv_if_map_exit(void)
{
	unregister_chrdev(ppe_drv_if_map_dev_major_id, "ppe_if_map");

	/*
	 * Remove the debugfs files recursively.
	 */
	if (ppe_drv_if_map_dentry) {
		debugfs_remove_recursive(ppe_drv_if_map_dentry);
	}

}

/*
 * ppe_drv_if_map_init()
 */
int ppe_drv_if_map_init(struct dentry *dentry)
{
	int dev_id = -1;

	ppe_drv_if_map_dentry = debugfs_create_dir("ppe_drv_if_map", dentry);
	if (!ppe_drv_if_map_dentry) {
		ppe_drv_warn("Failed to create ppe state directory in debugfs\n");
		return -1;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	if (!debugfs_create_u32("ppe_if_map", S_IRUGO, ppe_drv_if_map_dentry,
				(u32 *)&ppe_drv_if_map_dev_major_id)) {
		ppe_drv_warn("Failed to create ppe state dev major file in debugfs\n");
		goto init_cleanup;
	}
#else
	debugfs_create_u32("ppe_if_map", S_IRUGO, ppe_drv_if_map_dentry,
				(u32 *)&ppe_drv_if_map_dev_major_id);
#endif

	/*
	 * Register a char device that we will use to provide a dump of our state
	 */
	dev_id = register_chrdev(0, "ppe_drv_if_map", &ppe_drv_if_map_fops);
	if (dev_id < 0) {
		ppe_drv_warn("Failed to register chrdev %d\n", dev_id);
		goto init_cleanup;
	}

	ppe_drv_if_map_dev_major_id = dev_id;
	ppe_drv_trace("registered chr dev major id assigned %d\n", ppe_drv_if_map_dev_major_id);

	return 0;

init_cleanup:
	debugfs_remove_recursive(ppe_drv_if_map_dentry);
	return dev_id;
}
