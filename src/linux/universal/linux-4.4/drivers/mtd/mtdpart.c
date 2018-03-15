/*
 * Simple MTD partitioning layer
 *
 * Copyright © 2000 Nicolas Pitre <nico@fluxnic.net>
 * Copyright © 2002 Thomas Gleixner <gleixner@linutronix.de>
 * Copyright © 2000-2010 David Woodhouse <dwmw2@infradead.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/kmod.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/root_dev.h>
#include <linux/magic.h>
#include <linux/err.h>
#include <linux/kconfig.h>
#include <linux/squashfs_fs.h>

#define MTD_ERASE_PARTIAL	0x8000 /* partition only covers parts of an erase block */

#include "mtdcore.h"

/* Our partition linked list */
static LIST_HEAD(mtd_partitions);
static DEFINE_MUTEX(mtd_partitions_mutex);

/* Our partition node structure */
struct mtd_part {
	struct mtd_info mtd;
	struct mtd_info *master;
	uint64_t offset;
	struct list_head list;
};

/*
 * Given a pointer to the MTD object in the mtd_part structure, we can retrieve
 * the pointer to that structure with this macro.
 */
#define PART(x)  ((struct mtd_part *)(x))
#define IS_PART(mtd) (mtd->_read == part_read)

/*
 * MTD methods which simply translate the effective address and pass through
 * to the _real_ device.
 */

static int part_read(struct mtd_info *mtd, loff_t from, size_t len,
		size_t *retlen, u_char *buf)
{
	struct mtd_part *part = PART(mtd);
	struct mtd_ecc_stats stats;
	int res;

	stats = part->master->ecc_stats;
	res = part->master->_read(part->master, from + part->offset, len,
				  retlen, buf);
	if (unlikely(mtd_is_eccerr(res)))
		mtd->ecc_stats.failed +=
			part->master->ecc_stats.failed - stats.failed;
	else
		mtd->ecc_stats.corrected +=
			part->master->ecc_stats.corrected - stats.corrected;
	return res;
}

static int part_point(struct mtd_info *mtd, loff_t from, size_t len,
		size_t *retlen, void **virt, resource_size_t *phys)
{
	struct mtd_part *part = PART(mtd);

	return part->master->_point(part->master, from + part->offset, len,
				    retlen, virt, phys);
}

static int part_unpoint(struct mtd_info *mtd, loff_t from, size_t len)
{
	struct mtd_part *part = PART(mtd);

	return part->master->_unpoint(part->master, from + part->offset, len);
}

static unsigned long part_get_unmapped_area(struct mtd_info *mtd,
					    unsigned long len,
					    unsigned long offset,
					    unsigned long flags)
{
	struct mtd_part *part = PART(mtd);

	offset += part->offset;
	return part->master->_get_unmapped_area(part->master, len, offset,
						flags);
}

static int part_read_oob(struct mtd_info *mtd, loff_t from,
		struct mtd_oob_ops *ops)
{
	struct mtd_part *part = PART(mtd);
	int res;

	if (from >= mtd->size)
		return -EINVAL;
	if (ops->datbuf && from + ops->len > mtd->size)
		return -EINVAL;

	/*
	 * If OOB is also requested, make sure that we do not read past the end
	 * of this partition.
	 */
	if (ops->oobbuf) {
		size_t len, pages;

		if (ops->mode == MTD_OPS_AUTO_OOB)
			len = mtd->oobavail;
		else
			len = mtd->oobsize;
		pages = mtd_div_by_ws(mtd->size, mtd);
		pages -= mtd_div_by_ws(from, mtd);
		if (ops->ooboffs + ops->ooblen > pages * len)
			return -EINVAL;
	}

	res = part->master->_read_oob(part->master, from + part->offset, ops);
	if (unlikely(res)) {
		if (mtd_is_bitflip(res))
			mtd->ecc_stats.corrected++;
		if (mtd_is_eccerr(res))
			mtd->ecc_stats.failed++;
	}
	return res;
}

static int part_read_user_prot_reg(struct mtd_info *mtd, loff_t from,
		size_t len, size_t *retlen, u_char *buf)
{
	struct mtd_part *part = PART(mtd);
	return part->master->_read_user_prot_reg(part->master, from, len,
						 retlen, buf);
}

static int part_get_user_prot_info(struct mtd_info *mtd, size_t len,
				   size_t *retlen, struct otp_info *buf)
{
	struct mtd_part *part = PART(mtd);
	return part->master->_get_user_prot_info(part->master, len, retlen,
						 buf);
}

static int part_read_fact_prot_reg(struct mtd_info *mtd, loff_t from,
		size_t len, size_t *retlen, u_char *buf)
{
	struct mtd_part *part = PART(mtd);
	return part->master->_read_fact_prot_reg(part->master, from, len,
						 retlen, buf);
}

static int part_get_fact_prot_info(struct mtd_info *mtd, size_t len,
				   size_t *retlen, struct otp_info *buf)
{
	struct mtd_part *part = PART(mtd);
	return part->master->_get_fact_prot_info(part->master, len, retlen,
						 buf);
}

static int part_write(struct mtd_info *mtd, loff_t to, size_t len,
		size_t *retlen, const u_char *buf)
{
	struct mtd_part *part = PART(mtd);
	return part->master->_write(part->master, to + part->offset, len,
				    retlen, buf);
}

static int part_panic_write(struct mtd_info *mtd, loff_t to, size_t len,
		size_t *retlen, const u_char *buf)
{
	struct mtd_part *part = PART(mtd);
	return part->master->_panic_write(part->master, to + part->offset, len,
					  retlen, buf);
}

static int part_write_oob(struct mtd_info *mtd, loff_t to,
		struct mtd_oob_ops *ops)
{
	struct mtd_part *part = PART(mtd);


	if (to >= mtd->size)
		return -EINVAL;
	if (ops->datbuf && to + ops->len > mtd->size)
		return -EINVAL;
	return part->master->_write_oob(part->master, to + part->offset, ops);
}

static int part_write_user_prot_reg(struct mtd_info *mtd, loff_t from,
		size_t len, size_t *retlen, u_char *buf)
{
	struct mtd_part *part = PART(mtd);
	return part->master->_write_user_prot_reg(part->master, from, len,
						  retlen, buf);
}

static int part_lock_user_prot_reg(struct mtd_info *mtd, loff_t from,
		size_t len)
{
	struct mtd_part *part = PART(mtd);
	return part->master->_lock_user_prot_reg(part->master, from, len);
}

static int part_writev(struct mtd_info *mtd, const struct kvec *vecs,
		unsigned long count, loff_t to, size_t *retlen)
{
	struct mtd_part *part = PART(mtd);
	return part->master->_writev(part->master, vecs, count,
				     to + part->offset, retlen);
}

static int part_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct mtd_part *part = PART(mtd);
	int ret;

	instr->partial_start = false;
	if (mtd->flags & MTD_ERASE_PARTIAL) {
		size_t readlen = 0;
		u64 mtd_ofs;

		instr->erase_buf = kmalloc(part->master->erasesize, GFP_ATOMIC);
		if (!instr->erase_buf)
			return -ENOMEM;

		mtd_ofs = part->offset + instr->addr;
		instr->erase_buf_ofs = do_div(mtd_ofs, part->master->erasesize);

		if (instr->erase_buf_ofs > 0) {
			instr->addr -= instr->erase_buf_ofs;
			ret = part->master->_read(part->master,
				instr->addr + part->offset,
				part->master->erasesize,
				&readlen, instr->erase_buf);

			instr->partial_start = true;
		} else {
			mtd_ofs = part->offset + part->mtd.size;
			instr->erase_buf_ofs = part->master->erasesize -
				do_div(mtd_ofs, part->master->erasesize);

			if (instr->erase_buf_ofs > 0) {
				instr->len += instr->erase_buf_ofs;
				ret = part->master->_read(part->master,
					part->offset + instr->addr +
					instr->len - part->master->erasesize,
					part->master->erasesize, &readlen,
					instr->erase_buf);
			} else {
				ret = 0;
			}
		}
		if (ret < 0) {
			kfree(instr->erase_buf);
			return ret;
		}

	}

	instr->addr += part->offset;
	ret = part->master->_erase(part->master, instr);
	if (ret) {
		if (instr->fail_addr != MTD_FAIL_ADDR_UNKNOWN)
			instr->fail_addr -= part->offset;
		instr->addr -= part->offset;
		if (mtd->flags & MTD_ERASE_PARTIAL)
			kfree(instr->erase_buf);
	}

	return ret;
}

void mtd_erase_callback(struct erase_info *instr)
{
	if (instr->mtd->_erase == part_erase) {
		struct mtd_part *part = PART(instr->mtd);
		size_t wrlen = 0;

		if (instr->mtd->flags & MTD_ERASE_PARTIAL) {
			if (instr->partial_start) {
				part->master->_write(part->master,
					instr->addr, instr->erase_buf_ofs,
					&wrlen, instr->erase_buf);
				instr->addr += instr->erase_buf_ofs;
			} else {
				instr->len -= instr->erase_buf_ofs;
				part->master->_write(part->master,
					instr->addr + instr->len,
					instr->erase_buf_ofs, &wrlen,
					instr->erase_buf +
					part->master->erasesize -
					instr->erase_buf_ofs);
			}
			kfree(instr->erase_buf);
		}
		if (instr->fail_addr != MTD_FAIL_ADDR_UNKNOWN)
			instr->fail_addr -= part->offset;
		instr->addr -= part->offset;
	}
	if (instr->callback)
		instr->callback(instr);
}
EXPORT_SYMBOL_GPL(mtd_erase_callback);

static int part_lock(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	struct mtd_part *part = PART(mtd);
	return part->master->_lock(part->master, ofs + part->offset, len);
}

static int part_unlock(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	struct mtd_part *part = PART(mtd);
	return part->master->_unlock(part->master, ofs + part->offset, len);
}

static int part_is_locked(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	struct mtd_part *part = PART(mtd);
	return part->master->_is_locked(part->master, ofs + part->offset, len);
}

static void part_sync(struct mtd_info *mtd)
{
	struct mtd_part *part = PART(mtd);
	part->master->_sync(part->master);
}

static int part_suspend(struct mtd_info *mtd)
{
	struct mtd_part *part = PART(mtd);
	return part->master->_suspend(part->master);
}

static void part_resume(struct mtd_info *mtd)
{
	struct mtd_part *part = PART(mtd);
	part->master->_resume(part->master);
}

static int part_block_isreserved(struct mtd_info *mtd, loff_t ofs)
{
	struct mtd_part *part = PART(mtd);
	ofs += part->offset;
	return part->master->_block_isreserved(part->master, ofs);
}

static int part_block_isbad(struct mtd_info *mtd, loff_t ofs)
{
	struct mtd_part *part = PART(mtd);
	ofs += part->offset;
	return part->master->_block_isbad(part->master, ofs);
}

static int part_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	struct mtd_part *part = PART(mtd);
	int res;

	ofs += part->offset;
	res = part->master->_block_markbad(part->master, ofs);
	if (!res)
		mtd->ecc_stats.badblocks++;
	return res;
}

static inline void free_partition(struct mtd_part *p)
{
	kfree(p->mtd.name);
	kfree(p);
}

/*
 * This function unregisters and destroy all slave MTD objects which are
 * attached to the given master MTD object.
 */

int del_mtd_partitions(struct mtd_info *master)
{
	struct mtd_part *slave, *next;
	int ret, err = 0;

	mutex_lock(&mtd_partitions_mutex);
	list_for_each_entry_safe(slave, next, &mtd_partitions, list)
		if (slave->master == master) {
			ret = del_mtd_device(&slave->mtd);
			if (ret < 0) {
				err = ret;
				continue;
			}
			list_del(&slave->list);
			free_partition(slave);
		}
	mutex_unlock(&mtd_partitions_mutex);

	return err;
}

static struct mtd_part *allocate_partition(struct mtd_info *master,
			const struct mtd_partition *part, int partno,
			uint64_t cur_offset)
{
	struct mtd_part *slave;
	char *name;

	/* allocate the partition structure */
	slave = kzalloc(sizeof(*slave), GFP_KERNEL);
	name = kstrdup(part->name, GFP_KERNEL);
	if (!name || !slave) {
		printk(KERN_ERR"memory allocation error while creating partitions for \"%s\"\n",
		       master->name);
		kfree(name);
		kfree(slave);
		return ERR_PTR(-ENOMEM);
	}

	/* set up the MTD object for this partition */
	slave->mtd.type = master->type;
	slave->mtd.flags = master->flags & ~part->mask_flags;
	slave->mtd.size = part->size;
	slave->mtd.writesize = master->writesize;
	slave->mtd.writebufsize = master->writebufsize;
	slave->mtd.oobsize = master->oobsize;
	slave->mtd.oobavail = master->oobavail;
	slave->mtd.subpage_sft = master->subpage_sft;

	slave->mtd.name = name;
	slave->mtd.owner = master->owner;

	/* NOTE: Historically, we didn't arrange MTDs as a tree out of
	 * concern for showing the same data in multiple partitions.
	 * However, it is very useful to have the master node present,
	 * so the MTD_PARTITIONED_MASTER option allows that. The master
	 * will have device nodes etc only if this is set, so make the
	 * parent conditional on that option. Note, this is a way to
	 * distinguish between the master and the partition in sysfs.
	 */
	slave->mtd.dev.parent = IS_ENABLED(CONFIG_MTD_PARTITIONED_MASTER) ?
				&master->dev :
				master->dev.parent;

	slave->mtd._read = part_read;
	slave->mtd._write = part_write;

	if (master->_panic_write)
		slave->mtd._panic_write = part_panic_write;

	if (master->_point && master->_unpoint) {
		slave->mtd._point = part_point;
		slave->mtd._unpoint = part_unpoint;
	}

	if (master->_get_unmapped_area)
		slave->mtd._get_unmapped_area = part_get_unmapped_area;
	if (master->_read_oob)
		slave->mtd._read_oob = part_read_oob;
	if (master->_write_oob)
		slave->mtd._write_oob = part_write_oob;
	if (master->_read_user_prot_reg)
		slave->mtd._read_user_prot_reg = part_read_user_prot_reg;
	if (master->_read_fact_prot_reg)
		slave->mtd._read_fact_prot_reg = part_read_fact_prot_reg;
	if (master->_write_user_prot_reg)
		slave->mtd._write_user_prot_reg = part_write_user_prot_reg;
	if (master->_lock_user_prot_reg)
		slave->mtd._lock_user_prot_reg = part_lock_user_prot_reg;
	if (master->_get_user_prot_info)
		slave->mtd._get_user_prot_info = part_get_user_prot_info;
	if (master->_get_fact_prot_info)
		slave->mtd._get_fact_prot_info = part_get_fact_prot_info;
	if (master->_sync)
		slave->mtd._sync = part_sync;
	if (!partno && !master->dev.class && master->_suspend &&
	    master->_resume) {
			slave->mtd._suspend = part_suspend;
			slave->mtd._resume = part_resume;
	}
	if (master->_writev)
		slave->mtd._writev = part_writev;
	if (master->_lock)
		slave->mtd._lock = part_lock;
	if (master->_unlock)
		slave->mtd._unlock = part_unlock;
	if (master->_is_locked)
		slave->mtd._is_locked = part_is_locked;
	if (master->_block_isreserved)
		slave->mtd._block_isreserved = part_block_isreserved;
	if (master->_block_isbad)
		slave->mtd._block_isbad = part_block_isbad;
	if (master->_block_markbad)
		slave->mtd._block_markbad = part_block_markbad;
	slave->mtd._erase = part_erase;
	slave->master = master;
	slave->offset = part->offset;

	if (slave->offset == MTDPART_OFS_APPEND)
		slave->offset = cur_offset;
	if (slave->offset == MTDPART_OFS_NXTBLK) {
		slave->offset = cur_offset;
		if (mtd_mod_by_eb(cur_offset, master) != 0) {
			/* Round up to next erasesize */
			slave->offset = (mtd_div_by_eb(cur_offset, master) + 1) * master->erasesize;
			printk(KERN_NOTICE "Moving partition %d: "
			       "0x%012llx -> 0x%012llx\n", partno,
			       (unsigned long long)cur_offset, (unsigned long long)slave->offset);
		}
	}
	if (slave->offset == MTDPART_OFS_RETAIN) {
		slave->offset = cur_offset;
		if (master->size - slave->offset >= slave->mtd.size) {
			slave->mtd.size = master->size - slave->offset
							- slave->mtd.size;
		} else {
			printk(KERN_ERR "mtd partition \"%s\" doesn't have enough space: %#llx < %#llx, disabled\n",
				part->name, master->size - slave->offset,
				slave->mtd.size);
			/* register to preserve ordering */
			goto out_register;
		}
	}
	if (slave->mtd.size == MTDPART_SIZ_FULL)
		slave->mtd.size = master->size - slave->offset;

	printk(KERN_NOTICE "0x%012llx-0x%012llx : \"%s\"\n", (unsigned long long)slave->offset,
		(unsigned long long)(slave->offset + slave->mtd.size), slave->mtd.name);

	/* let's do some sanity checks */
	if (slave->offset >= master->size) {
		/* let's register it anyway to preserve ordering */
		slave->offset = 0;
		slave->mtd.size = 0;
		printk(KERN_ERR"mtd: partition \"%s\" is out of reach -- disabled\n",
			part->name);
		goto out_register;
	}
	if (slave->offset + slave->mtd.size > master->size) {
		slave->mtd.size = master->size - slave->offset;
		printk(KERN_WARNING"mtd: partition \"%s\" extends beyond the end of device \"%s\" -- size truncated to %#llx\n",
			part->name, master->name, (unsigned long long)slave->mtd.size);
	}
	if (master->numeraseregions > 1) {
		/* Deal with variable erase size stuff */
		int i, max = master->numeraseregions;
		u64 end = slave->offset + slave->mtd.size;
		struct mtd_erase_region_info *regions = master->eraseregions;

		/* Find the first erase regions which is part of this
		 * partition. */
		for (i = 0; i < max && regions[i].offset <= slave->offset; i++)
			;
		/* The loop searched for the region _behind_ the first one */
		if (i > 0)
			i--;

		/* Pick biggest erasesize */
		for (; i < max && regions[i].offset < end; i++) {
			if (slave->mtd.erasesize < regions[i].erasesize) {
				slave->mtd.erasesize = regions[i].erasesize;
			}
		}
		BUG_ON(slave->mtd.erasesize == 0);
	} else {
		/* Single erase size */
		slave->mtd.erasesize = master->erasesize;
	}

	if ((slave->mtd.flags & MTD_WRITEABLE) &&
	    mtd_mod_by_eb(slave->offset, &slave->mtd)) {
		/* Doesn't start on a boundary of major erase size */
		slave->mtd.flags |= MTD_ERASE_PARTIAL;
		if (((u32) slave->mtd.size) > master->erasesize)
			slave->mtd.flags &= ~MTD_WRITEABLE;
		else
			slave->mtd.erasesize = slave->mtd.size;
	}
	if ((slave->mtd.flags & MTD_WRITEABLE) &&
	    mtd_mod_by_eb(slave->offset + slave->mtd.size, &slave->mtd)) {
		slave->mtd.flags |= MTD_ERASE_PARTIAL;

		if ((u32) slave->mtd.size > master->erasesize)
			slave->mtd.flags &= ~MTD_WRITEABLE;
		else
			slave->mtd.erasesize = slave->mtd.size;
	}
	if ((slave->mtd.flags & (MTD_ERASE_PARTIAL|MTD_WRITEABLE)) == MTD_ERASE_PARTIAL)
		printk(KERN_WARNING"mtd: partition \"%s\" must either start or end on erase block boundary or be smaller than an erase block -- forcing read-only\n",
				part->name);

	slave->mtd.ecclayout = master->ecclayout;
	slave->mtd.ecc_step_size = master->ecc_step_size;
	slave->mtd.ecc_strength = master->ecc_strength;
	slave->mtd.bitflip_threshold = master->bitflip_threshold;

	if (master->_block_isbad) {
		uint64_t offs = 0;

		while (offs < slave->mtd.size) {
			if (mtd_block_isreserved(master, offs + slave->offset))
				slave->mtd.ecc_stats.bbtblocks++;
			else if (mtd_block_isbad(master, offs + slave->offset))
				slave->mtd.ecc_stats.badblocks++;
			offs += slave->mtd.erasesize;
		}
	}

out_register:
	return slave;
}

static ssize_t mtd_partition_offset_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mtd_info *mtd = dev_get_drvdata(dev);
	struct mtd_part *part = PART(mtd);
	return snprintf(buf, PAGE_SIZE, "%lld\n", part->offset);
}

static DEVICE_ATTR(offset, S_IRUGO, mtd_partition_offset_show, NULL);

static const struct attribute *mtd_partition_attrs[] = {
	&dev_attr_offset.attr,
	NULL
};

static int mtd_add_partition_attrs(struct mtd_part *new)
{
	int ret = sysfs_create_files(&new->mtd.dev.kobj, mtd_partition_attrs);
	if (ret)
		printk(KERN_WARNING
		       "mtd: failed to create partition attrs, err=%d\n", ret);
	return ret;
}

int mtd_add_partition(struct mtd_info *master, const char *name,
		      long long offset, long long length)
{
	struct mtd_partition part;
	struct mtd_part *new;
	int ret = 0;

	/* the direct offset is expected */
	if (offset == MTDPART_OFS_APPEND ||
	    offset == MTDPART_OFS_NXTBLK)
		return -EINVAL;

	if (length == MTDPART_SIZ_FULL)
		length = master->size - offset;

	if (length <= 0)
		return -EINVAL;

	part.name = name;
	part.size = length;
	part.offset = offset;
	part.mask_flags = 0;
	part.ecclayout = NULL;

	new = allocate_partition(master, &part, -1, offset);
	if (IS_ERR(new))
		return PTR_ERR(new);

	mutex_lock(&mtd_partitions_mutex);
	list_add(&new->list, &mtd_partitions);
	mutex_unlock(&mtd_partitions_mutex);

	add_mtd_device(&new->mtd);

	mtd_add_partition_attrs(new);

	return ret;
}
EXPORT_SYMBOL_GPL(mtd_add_partition);

int mtd_del_partition(struct mtd_info *master, int partno)
{
	struct mtd_part *slave, *next;
	int ret = -EINVAL;

	mutex_lock(&mtd_partitions_mutex);
	list_for_each_entry_safe(slave, next, &mtd_partitions, list)
		if ((slave->master == master) &&
		    (slave->mtd.index == partno)) {
			sysfs_remove_files(&slave->mtd.dev.kobj,
					   mtd_partition_attrs);
			ret = del_mtd_device(&slave->mtd);
			if (ret < 0)
				break;

			list_del(&slave->list);
			free_partition(slave);
			break;
		}
	mutex_unlock(&mtd_partitions_mutex);

	return ret;
}
EXPORT_SYMBOL_GPL(mtd_del_partition);

#ifdef CONFIG_MTD_ROOTFS_SPLIT
#define ROOTFS_SPLIT_NAME "ddwrt"
#define ROOTFS_REMOVED_NAME "<removed>"


static int split_squashfs(struct mtd_info *master, int offset, int *split_offset)
{
	struct squashfs_super_block sb;
	int len, ret;

	ret = mtd_read(master, offset, sizeof(sb), &len, (void *) &sb);
	if (ret || (len != sizeof(sb))) {
		printk(KERN_ALERT "split_squashfs: error occured while reading "
			"from \"%s\"\n", master->name);
		return -EINVAL;
	}
	printk(KERN_EMERG " magic %X vs %X\n",le32_to_cpu(sb.s_magic), SQUASHFS_MAGIC);
	if (SQUASHFS_MAGIC != le32_to_cpu(sb.s_magic)) {
		printk(KERN_ALERT "split_squashfs: no squashfs found in \"%s\"\n",
			master->name);
		*split_offset = 0;
		return 0;
	}

	if (le64_to_cpu(sb.bytes_used) <= 0) {
		printk(KERN_ALERT "split_squashfs: squashfs is empty in \"%s\"\n",
			master->name);
		*split_offset = 0;
		return 0;
	}

	len = le64_to_cpu(sb.bytes_used);
	len += (offset & 0x000fffff);
	len +=  (master->erasesize - 1);
	len &= ~(master->erasesize - 1);
	len -= (offset & 0x000fffff);
	*split_offset = offset + len;

	return 0;
}

static int split_rootfs_data(struct mtd_info *master, struct mtd_info *rpart, const struct mtd_partition *part)
{
	struct mtd_partition *dpart;
	struct mtd_part *slave = NULL;
	struct mtd_part *spart;
	int ret, split_offset = 0;

	spart = PART(rpart);
	ret = split_squashfs(master, spart->offset, &split_offset);
	if (ret)
		return ret;

	if (split_offset <= 0)
		return 0;

	dpart = kmalloc(sizeof(*part)+sizeof(ROOTFS_SPLIT_NAME)+1, GFP_KERNEL);
	if (dpart == NULL) {
		printk(KERN_INFO "split_squashfs: no memory for partition \"%s\"\n",
			ROOTFS_SPLIT_NAME);
		return -ENOMEM;
	}

	memcpy(dpart, part, sizeof(*part));
	dpart->name = (unsigned char *)&dpart[1];
	strcpy(dpart->name, ROOTFS_SPLIT_NAME);

	dpart->size = rpart->size - (split_offset - spart->offset);
 	dpart->size /= 65536;
 	dpart->size *= 65536;
#ifdef CONFIG_SOC_MT7620_OPENWRT
	// todo: add proper board detection
	dpart->size -= 0x110000;
#endif
	dpart->offset = split_offset;
	dpart->mask_flags = 0;

	if (dpart == NULL)
		return 1;

	printk(KERN_INFO "mtd: partition \"%s\" created automatically, ofs=%llX, len=%llX \n",
		ROOTFS_SPLIT_NAME, dpart->offset, dpart->size);

	slave = allocate_partition(master, dpart, 0, split_offset);
	if (IS_ERR(slave))
		return PTR_ERR(slave);
	mutex_lock(&mtd_partitions_mutex);
	list_add(&slave->list, &mtd_partitions);
	mutex_unlock(&mtd_partitions_mutex);

	add_mtd_device(&slave->mtd);

	rpart->split = &slave->mtd;

	return 0;
}

static int refresh_rootfs_split(struct mtd_info *mtd)
{
	struct mtd_partition tpart;
	struct mtd_part *part;
	char *name;
	//int index = 0;
	int offset, size;
	int ret;

	part = PART(mtd);

	/* check for the new squashfs offset first */
	ret = split_squashfs(part->master, part->offset, &offset);
	if (ret)
		return ret;

	if ((offset > 0) && !mtd->split) {
		printk(KERN_INFO "%s: creating new split partition for \"%s\"\n", __func__, mtd->name);
		/* if we don't have a rootfs split partition, create a new one */
		tpart.name = (char *) mtd->name;
		tpart.size = mtd->size;
		tpart.offset = part->offset;

		return split_rootfs_data(part->master, &part->mtd, &tpart);
	} else if ((offset > 0) && mtd->split) {
		/* update the offsets of the existing partition */
		size = mtd->size + part->offset - offset;

		part = PART(mtd->split);
		part->offset = offset;
		part->mtd.size = size;
		printk(KERN_INFO "%s: %s partition \"" ROOTFS_SPLIT_NAME "\", offset: 0x%06x (0x%06x)\n",
			__func__, (!strcmp(part->mtd.name, ROOTFS_SPLIT_NAME) ? "updating" : "creating"),
			(u32) part->offset, (u32) part->mtd.size);
		name = kmalloc(sizeof(ROOTFS_SPLIT_NAME) + 1, GFP_KERNEL);
		strcpy(name, ROOTFS_SPLIT_NAME);
		part->mtd.name = name;
	} else if ((offset <= 0) && mtd->split) {
		printk(KERN_INFO "%s: removing partition \"%s\"\n", __func__, mtd->split->name);

		/* mark existing partition as removed */
		part = PART(mtd->split);
		name = kmalloc(sizeof(ROOTFS_SPLIT_NAME) + 1, GFP_KERNEL);
		strcpy(name, ROOTFS_REMOVED_NAME);
		part->mtd.name = name;
		part->offset = 0;
		part->mtd.size = 0;
	}

	return 0;
}
#endif /* CONFIG_MTD_ROOTFS_SPLIT */

#ifdef CONFIG_MTD_ROOTFS_GEN
#include <linux/vmalloc.h>
#endif

/*
 * This function, given a master MTD object and a partition table, creates
 * and registers slave MTD objects which are bound to the master according to
 * the partition definitions.
 *
 * For historical reasons, this function's caller only registers the master
 * if the MTD_PARTITIONED_MASTER config option is set.
 */

int add_mtd_partitions(struct mtd_info *master,
		       const struct mtd_partition *parts,
		       int nbparts)
{
	struct mtd_part *slave;
	uint64_t cur_offset = 0;
	int i;
#ifdef CONFIG_MTD_ROOTFS_SPLIT
	int ret;
#endif

	printk(KERN_NOTICE "Creating %d MTD partitions on \"%s\":\n", nbparts, master->name);

	for (i = 0; i < nbparts; i++) {
		slave = allocate_partition(master, parts + i, i, cur_offset);
		if (IS_ERR(slave)) {
			del_mtd_partitions(master);
			return PTR_ERR(slave);
		}

		mutex_lock(&mtd_partitions_mutex);
		list_add(&slave->list, &mtd_partitions);
		mutex_unlock(&mtd_partitions_mutex);

		add_mtd_device(&slave->mtd);
		mtd_add_partition_attrs(slave);
#ifdef CONFIG_MTD_ROOTFS_GEN
		if (!strcmp(parts[i].name, "linux") || !strcmp(parts[i].name, "linux2")) {

		unsigned int buf;
		int offset = slave->offset;
		int bootsize = slave->offset;
		printk(KERN_INFO "scan from offset %X\n",bootsize);
		int nvramsize = master->erasesize;
			    while((offset + master->erasesize) < master->size)
			    {
			    int retlen;
			    mtd_read(master,offset,4, &retlen, (u_char *)&buf);
			    
			    if (SQUASHFS_MAGIC == le32_to_cpu(buf))
				    {
				    	printk(KERN_EMERG "\nfound squashfs at %X\n",offset);
				    	struct mtd_partition part;
				    	part.name = "rootfs";
				    	part.offset = offset;
				    	part.size = (master->size - nvramsize) - offset; 
				    	part.mask_flags = 0;
				    	add_mtd_partitions(master,&part,1);
					break;
				    } 
			    offset+=4096;
			    }
		}
#endif
		if (!strcmp(parts[i].name, "rootfs") || !strcmp(parts[i].name, "rootfs2")) {
#ifdef CONFIG_MTD_ROOTFS_ROOT_DEV
			if (ROOT_DEV == 0) {
				printk(KERN_NOTICE "mtd: partition \"rootfs\" "
					"set to be root filesystem\n");
				ROOT_DEV = MKDEV(MTD_BLOCK_MAJOR, slave->mtd.index);
			}
#endif
#ifdef CONFIG_MTD_ROOTFS_SPLIT
			ret = split_rootfs_data(master, &slave->mtd, &parts[i]);
			/* if (ret == 0)
			 * 	j++; */
#endif
		}

		cur_offset = slave->offset + slave->mtd.size;
	}

	return 0;
}

int refresh_mtd_partitions(struct mtd_info *mtd)
{
	int ret = 0;

	if (IS_PART(mtd)) {
		struct mtd_part *part;
		struct mtd_info *master;

		part = PART(mtd);
		master = part->master;
		if (master->refresh_device)
			ret = master->refresh_device(master);
	}

	if (!ret && mtd->refresh_device)
		ret = mtd->refresh_device(mtd);

#ifdef CONFIG_MTD_ROOTFS_SPLIT
	if (!ret && IS_PART(mtd) && !strcmp(mtd->name, "rootfs"))
		refresh_rootfs_split(mtd);
#endif

	return 0;
}
EXPORT_SYMBOL_GPL(refresh_mtd_partitions);

static DEFINE_SPINLOCK(part_parser_lock);
static LIST_HEAD(part_parsers);

static struct mtd_part_parser *get_partition_parser(const char *name)
{
	struct mtd_part_parser *p, *ret = NULL;

	spin_lock(&part_parser_lock);

	list_for_each_entry(p, &part_parsers, list)
		if (!strcmp(p->name, name) && try_module_get(p->owner)) {
			ret = p;
			break;
		}

	spin_unlock(&part_parser_lock);

	return ret;
}

#define put_partition_parser(p) do { module_put((p)->owner); } while (0)

void register_mtd_parser(struct mtd_part_parser *p)
{
	spin_lock(&part_parser_lock);
	list_add(&p->list, &part_parsers);
	spin_unlock(&part_parser_lock);
}
EXPORT_SYMBOL_GPL(register_mtd_parser);

void deregister_mtd_parser(struct mtd_part_parser *p)
{
	spin_lock(&part_parser_lock);
	list_del(&p->list);
	spin_unlock(&part_parser_lock);
}
EXPORT_SYMBOL_GPL(deregister_mtd_parser);

/*
 * Do not forget to update 'parse_mtd_partitions()' kerneldoc comment if you
 * are changing this array!
 */
static const char * const default_mtd_part_types[] = {
	"cmdlinepart",
	"ofpart",
	NULL
};

/**
 * parse_mtd_partitions - parse MTD partitions
 * @master: the master partition (describes whole MTD device)
 * @types: names of partition parsers to try or %NULL
 * @pparts: array of partitions found is returned here
 * @data: MTD partition parser-specific data
 *
 * This function tries to find partition on MTD device @master. It uses MTD
 * partition parsers, specified in @types. However, if @types is %NULL, then
 * the default list of parsers is used. The default list contains only the
 * "cmdlinepart" and "ofpart" parsers ATM.
 * Note: If there are more then one parser in @types, the kernel only takes the
 * partitions parsed out by the first parser.
 *
 * This function may return:
 * o a negative error code in case of failure
 * o zero if no partitions were found
 * o a positive number of found partitions, in which case on exit @pparts will
 *   point to an array containing this number of &struct mtd_info objects.
 */
int parse_mtd_partitions(struct mtd_info *master, const char *const *types,
			 struct mtd_partition **pparts,
			 struct mtd_part_parser_data *data)
{
	struct mtd_part_parser *parser;
	int ret, err = 0;

	if (!types)
		types = default_mtd_part_types;

	for ( ; *types; types++) {
		pr_debug("%s: parsing partitions %s\n", master->name, *types);
		parser = get_partition_parser(*types);
		if (!parser && !request_module("%s", *types))
			parser = get_partition_parser(*types);
		pr_debug("%s: got parser %s\n", master->name,
			 parser ? parser->name : NULL);
		if (!parser)
			continue;
		ret = (*parser->parse_fn)(master, pparts, data);
		pr_debug("%s: parser %s: %i\n",
			 master->name, parser->name, ret);
		put_partition_parser(parser);
		if (ret > 0) {
			printk(KERN_NOTICE "%d %s partitions found on MTD device %s\n",
			       ret, parser->name, master->name);
			return ret;
		}
		/*
		 * Stash the first error we see; only report it if no parser
		 * succeeds
		 */
		if (ret < 0 && !err)
			err = ret;
	}
	return err;
}

int mtd_is_partition(const struct mtd_info *mtd)
{
	struct mtd_part *part;
	int ispart = 0;

	mutex_lock(&mtd_partitions_mutex);
	list_for_each_entry(part, &mtd_partitions, list)
		if (&part->mtd == mtd) {
			ispart = 1;
			break;
		}
	mutex_unlock(&mtd_partitions_mutex);

	return ispart;
}
EXPORT_SYMBOL_GPL(mtd_is_partition);

/* Returns the size of the entire flash chip */
uint64_t mtd_get_device_size(const struct mtd_info *mtd)
{
	if (!mtd_is_partition(mtd))
		return mtd->size;

	return PART(mtd)->master->size;
}
EXPORT_SYMBOL_GPL(mtd_get_device_size);



#ifdef CONFIG_RALINK_OPENWRT
/*
 * Flash API: ra_mtd_read, ra_mtd_write
 * Arguments:
 *   - num: specific the mtd number
 *   - to/from: the offset to read from or written to
 *   - len: length
 *   - buf: data to be read/written
 * Returns:
 *   - return -errno if failed
 *   - return the number of bytes read/written if successed
 */
int ra_mtd_write(int num, loff_t to, size_t len, const u_char *buf)
{
	int ret = -1;
	size_t rdlen, wrlen;
	struct mtd_info *mtd;
	struct erase_info ei;
	u_char *bak = NULL;
//	printk(KERN_EMERG "writing to partition %d, offset %d, len %d\n",num,to,len);
#ifdef CONFIG_RT2880_FLASH_8M
        /* marklin 20080605 : return read mode for ST */
        Flash_SetModeRead();
#endif

	mtd = get_mtd_device(NULL, num);
	if (IS_ERR(mtd))
		return (int)mtd;
	if (len > mtd->erasesize) {
		put_mtd_device(mtd);
		return -E2BIG;
	}

	bak = kmalloc(mtd->erasesize, GFP_KERNEL);
	if (bak == NULL) {
		put_mtd_device(mtd);
		return -ENOMEM;
	}

	ret = mtd_read(mtd, 0, mtd->erasesize, &rdlen, bak);
	if (ret != 0) {
		put_mtd_device(mtd);
		kfree(bak);
		return ret;
	}
	if (rdlen != mtd->erasesize)
		printk(KERN_EMERG "warning: ra_mtd_write: rdlen is not equal to erasesize\n");

	memcpy(bak + to, buf, len);

	ei.mtd = mtd;
	ei.callback = NULL;
	ei.addr = 0;
	ei.len = mtd->erasesize;
	ei.priv = 0;
	ret = mtd_erase(mtd, &ei);
	if (ret != 0) {
		put_mtd_device(mtd);
		kfree(bak);
		return ret;
	}

	ret = mtd_write(mtd, 0, mtd->erasesize, &wrlen, bak);

	put_mtd_device(mtd);
	kfree(bak);
#ifdef CONFIG_RT2880_FLASH_8M
        /* marklin 20080605 : return read mode for ST */
        Flash_SetModeRead();
#endif
	return ret;
}


int ra_mtd_read(int num,int from, int len, u_char *buf)
{
	int ret;
	size_t rdlen;
	struct mtd_info *mtd;
	printk(KERN_INFO "read ralink eeprom from %X with len %X to %p (device %d)\n",from,len,buf,num);
	mtd = get_mtd_device(NULL, num);

	ret = mtd_read(mtd, from, len, &rdlen, buf);
	if (rdlen != len)
		printk(KERN_EMERG "warning: ra_mtd_read: rdlen is not equal to len\n");

	put_mtd_device(mtd);
	return ret;
}
EXPORT_SYMBOL(ra_mtd_read);
EXPORT_SYMBOL(ra_mtd_write);

#endif