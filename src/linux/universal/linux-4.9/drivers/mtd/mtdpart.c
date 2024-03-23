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
#include "mtdcore.h"
#include <linux/magic.h>

#define MTD_ERASE_PARTIAL	0x8000 /* partition only covers parts of an erase block */
 
/* Our partition linked list */
static LIST_HEAD(mtd_partitions);
static DEFINE_MUTEX(mtd_partitions_mutex);

/**
 * struct mtd_part - our partition node structure
 *
 * @mtd: struct holding partition details
 * @parent: parent mtd - flash device or another partition
 * @offset: partition offset relative to the *flash device*
 */
struct mtd_part {
	struct mtd_info mtd;
	struct mtd_info *master;
	uint64_t offset;
	struct list_head list;
};

/*
 * Given a pointer to the MTD object in the mtd_part structure, we can retrieve
 * the pointer to that structure.
 */
static inline struct mtd_part *mtd_to_part(const struct mtd_info *mtd)
{
	return container_of(mtd, struct mtd_part, mtd);
}

#define PART(x)  ((struct mtd_part *)(x))
#define IS_PART(mtd) (mtd->_read == part_read)

/*
 * MTD methods which simply translate the effective address and pass through
 * to the _real_ device.
 */

static int part_read(struct mtd_info *mtd, loff_t from, size_t len,
		size_t *retlen, u_char *buf)
{
	struct mtd_part *part = mtd_to_part(mtd);
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
	struct mtd_part *part = mtd_to_part(mtd);

	return part->master->_point(part->master, from + part->offset, len,
				    retlen, virt, phys);
}

static int part_unpoint(struct mtd_info *mtd, loff_t from, size_t len)
{
	struct mtd_part *part = mtd_to_part(mtd);

	return part->master->_unpoint(part->master, from + part->offset, len);
}

static unsigned long part_get_unmapped_area(struct mtd_info *mtd,
					    unsigned long len,
					    unsigned long offset,
					    unsigned long flags)
{
	struct mtd_part *part = mtd_to_part(mtd);

	offset += part->offset;
	return part->master->_get_unmapped_area(part->master, len, offset,
						flags);
}

static int part_read_oob(struct mtd_info *mtd, loff_t from,
		struct mtd_oob_ops *ops)
{
	struct mtd_part *part = mtd_to_part(mtd);
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

		len = mtd_oobavail(mtd, ops);
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
	struct mtd_part *part = mtd_to_part(mtd);
	return part->master->_read_user_prot_reg(part->master, from, len,
						 retlen, buf);
}

static int part_get_user_prot_info(struct mtd_info *mtd, size_t len,
				   size_t *retlen, struct otp_info *buf)
{
	struct mtd_part *part = mtd_to_part(mtd);
	return part->master->_get_user_prot_info(part->master, len, retlen,
						 buf);
}

static int part_read_fact_prot_reg(struct mtd_info *mtd, loff_t from,
		size_t len, size_t *retlen, u_char *buf)
{
	struct mtd_part *part = mtd_to_part(mtd);
	return part->master->_read_fact_prot_reg(part->master, from, len,
						 retlen, buf);
}

static int part_get_fact_prot_info(struct mtd_info *mtd, size_t len,
				   size_t *retlen, struct otp_info *buf)
{
	struct mtd_part *part = mtd_to_part(mtd);
	return part->master->_get_fact_prot_info(part->master, len, retlen,
						 buf);
}

static int part_write(struct mtd_info *mtd, loff_t to, size_t len,
		size_t *retlen, const u_char *buf)
{
	struct mtd_part *part = mtd_to_part(mtd);
	return part->master->_write(part->master, to + part->offset, len,
				    retlen, buf);
}

static int part_panic_write(struct mtd_info *mtd, loff_t to, size_t len,
		size_t *retlen, const u_char *buf)
{
	struct mtd_part *part = mtd_to_part(mtd);
	return part->master->_panic_write(part->master, to + part->offset, len,
					  retlen, buf);
}

static int part_write_oob(struct mtd_info *mtd, loff_t to,
		struct mtd_oob_ops *ops)
{
	struct mtd_part *part = mtd_to_part(mtd);


	if (to >= mtd->size)
		return -EINVAL;
	if (ops->datbuf && to + ops->len > mtd->size)
		return -EINVAL;
	return part->master->_write_oob(part->master, to + part->offset, ops);
}

static int part_write_user_prot_reg(struct mtd_info *mtd, loff_t from,
		size_t len, size_t *retlen, u_char *buf)
{
	struct mtd_part *part = mtd_to_part(mtd);
	return part->master->_write_user_prot_reg(part->master, from, len,
						  retlen, buf);
}

static int part_lock_user_prot_reg(struct mtd_info *mtd, loff_t from,
		size_t len)
{
	struct mtd_part *part = mtd_to_part(mtd);
	return part->master->_lock_user_prot_reg(part->master, from, len);
}

static int part_writev(struct mtd_info *mtd, const struct kvec *vecs,
		unsigned long count, loff_t to, size_t *retlen)
{
	struct mtd_part *part = mtd_to_part(mtd);
	return part->master->_writev(part->master, vecs, count,
				     to + part->offset, retlen);
}

static int part_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct mtd_part *part = mtd_to_part(mtd);
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
		struct mtd_part *part = mtd_to_part(instr->mtd);
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
	struct mtd_part *part = mtd_to_part(mtd);
	return part->master->_lock(part->master, ofs + part->offset, len);
}

static int part_unlock(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	struct mtd_part *part = mtd_to_part(mtd);
	return part->master->_unlock(part->master, ofs + part->offset, len);
}

static int part_is_locked(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	struct mtd_part *part = mtd_to_part(mtd);
	return part->master->_is_locked(part->master, ofs + part->offset, len);
}

static void part_sync(struct mtd_info *mtd)
{
	struct mtd_part *part = mtd_to_part(mtd);
	part->master->_sync(part->master);
}

static int part_suspend(struct mtd_info *mtd)
{
	struct mtd_part *part = mtd_to_part(mtd);
	return part->master->_suspend(part->master);
}

static void part_resume(struct mtd_info *mtd)
{
	struct mtd_part *part = mtd_to_part(mtd);
	part->master->_resume(part->master);
}

static int part_block_isreserved(struct mtd_info *mtd, loff_t ofs)
{
	struct mtd_part *part = mtd_to_part(mtd);
	ofs += part->offset;
	return part->master->_block_isreserved(part->master, ofs);
}

static int part_block_isbad(struct mtd_info *mtd, loff_t ofs)
{
	struct mtd_part *part = mtd_to_part(mtd);
	ofs += part->offset;
	return part->master->_block_isbad(part->master, ofs);
}

static int part_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	struct mtd_part *part = mtd_to_part(mtd);
	int res;

	ofs += part->offset;
	res = part->master->_block_markbad(part->master, ofs);
	if (!res)
		mtd->ecc_stats.badblocks++;
	return res;
}

static int part_get_device(struct mtd_info *mtd)
{
	struct mtd_part *part = mtd_to_part(mtd);
	return part->master->_get_device(part->master);
}

static void part_put_device(struct mtd_info *mtd)
{
	struct mtd_part *part = mtd_to_part(mtd);
	part->master->_put_device(part->master);
}

static int part_ooblayout_ecc(struct mtd_info *mtd, int section,
			      struct mtd_oob_region *oobregion)
{
	struct mtd_part *part = mtd_to_part(mtd);

	return mtd_ooblayout_ecc(part->master, section, oobregion);
}

static int part_ooblayout_free(struct mtd_info *mtd, int section,
			       struct mtd_oob_region *oobregion)
{
	struct mtd_part *part = mtd_to_part(mtd);

	return mtd_ooblayout_free(part->master, section, oobregion);
}

static const struct mtd_ooblayout_ops part_ooblayout_ops = {
	.ecc = part_ooblayout_ecc,
	.free = part_ooblayout_free,
};

static int part_max_bad_blocks(struct mtd_info *mtd, loff_t ofs, size_t len)
{
	struct mtd_part *part = mtd_to_part(mtd);

	return part->master->_max_bad_blocks(part->master,
					     ofs + part->offset, len);
}

static inline void free_partition(struct mtd_part *p)
{
	kfree(p->mtd.name);
	kfree(p);
}

/**
 * mtd_parse_part - parse MTD partition looking for subpartitions
 *
 * @slave: part that is supposed to be a container and should be parsed
 * @types: NULL-terminated array with names of partition parsers to try
 *
 * Some partitions are kind of containers with extra subpartitions (volumes).
 * There can be various formats of such containers. This function tries to use
 * specified parsers to analyze given partition and registers found
 * subpartitions on success.
 */
static int mtd_parse_part(struct mtd_part *slave, const char *const *types)
{
	struct mtd_partitions parsed;
	int err;

	err = parse_mtd_partitions(&slave->mtd, types, &parsed, NULL);
	if (err)
		return err;
	else if (!parsed.nr_parts)
		return -ENOENT;

	err = add_mtd_partitions(&slave->mtd, parsed.parts, parsed.nr_parts);

	mtd_part_parser_cleanup(&parsed);

	return err;
}

static struct mtd_part *allocate_partition(struct mtd_info *parent,
			const struct mtd_partition *part, int partno,
			uint64_t cur_offset)
{
	int wr_alignment = (parent->flags & MTD_NO_ERASE) ? parent->writesize:
							    parent->erasesize;
	struct mtd_part *slave;
	u32 remainder;
	char *name;
	u64 tmp;

	/* allocate the partition structure */
	slave = kzalloc(sizeof(*slave), GFP_KERNEL);
	name = kstrdup(part->name, GFP_KERNEL);
	if (!name || !slave) {
		printk(KERN_ERR"memory allocation error while creating partitions for \"%s\"\n",
		       parent->name);
		kfree(name);
		kfree(slave);
		return ERR_PTR(-ENOMEM);
	}

	/* set up the MTD object for this partition */
	slave->mtd.type = parent->type;
	slave->mtd.flags = parent->flags & ~part->mask_flags;
	slave->mtd.size = part->size;
	slave->mtd.writesize = parent->writesize;
	slave->mtd.writebufsize = parent->writebufsize;
	slave->mtd.oobsize = parent->oobsize;
	slave->mtd.oobavail = parent->oobavail;
	slave->mtd.subpage_sft = parent->subpage_sft;
	slave->mtd.pairing = parent->pairing;

	slave->mtd.name = name;
	slave->mtd.owner = parent->owner;

	/* NOTE: Historically, we didn't arrange MTDs as a tree out of
	 * concern for showing the same data in multiple partitions.
	 * However, it is very useful to have the master node present,
	 * so the MTD_PARTITIONED_MASTER option allows that. The master
	 * will have device nodes etc only if this is set, so make the
	 * parent conditional on that option. Note, this is a way to
	 * distinguish between the master and the partition in sysfs.
	 */
	slave->mtd.dev.parent = IS_ENABLED(CONFIG_MTD_PARTITIONED_MASTER) || mtd_is_partition(parent) ?
				&parent->dev :
				parent->dev.parent;
	slave->mtd.dev.of_node = part->of_node;

	slave->mtd._read = part_read;
	slave->mtd._write = part_write;

	if (parent->_panic_write)
		slave->mtd._panic_write = part_panic_write;

	if (parent->_point && parent->_unpoint) {
		slave->mtd._point = part_point;
		slave->mtd._unpoint = part_unpoint;
	}

	if (parent->_get_unmapped_area)
		slave->mtd._get_unmapped_area = part_get_unmapped_area;
	if (parent->_read_oob)
		slave->mtd._read_oob = part_read_oob;
	if (parent->_write_oob)
		slave->mtd._write_oob = part_write_oob;
	if (parent->_read_user_prot_reg)
		slave->mtd._read_user_prot_reg = part_read_user_prot_reg;
	if (parent->_read_fact_prot_reg)
		slave->mtd._read_fact_prot_reg = part_read_fact_prot_reg;
	if (parent->_write_user_prot_reg)
		slave->mtd._write_user_prot_reg = part_write_user_prot_reg;
	if (parent->_lock_user_prot_reg)
		slave->mtd._lock_user_prot_reg = part_lock_user_prot_reg;
	if (parent->_get_user_prot_info)
		slave->mtd._get_user_prot_info = part_get_user_prot_info;
	if (parent->_get_fact_prot_info)
		slave->mtd._get_fact_prot_info = part_get_fact_prot_info;
	if (parent->_sync)
		slave->mtd._sync = part_sync;
	if (!partno && !parent->dev.class && parent->_suspend &&
	    parent->_resume) {
			slave->mtd._suspend = part_suspend;
			slave->mtd._resume = part_resume;
	}
	if (parent->_writev)
		slave->mtd._writev = part_writev;
	if (parent->_lock)
		slave->mtd._lock = part_lock;
	if (parent->_unlock)
		slave->mtd._unlock = part_unlock;
	if (parent->_is_locked)
		slave->mtd._is_locked = part_is_locked;
	if (parent->_block_isreserved)
		slave->mtd._block_isreserved = part_block_isreserved;
	if (parent->_block_isbad)
		slave->mtd._block_isbad = part_block_isbad;
	if (parent->_block_markbad)
		slave->mtd._block_markbad = part_block_markbad;
	if (parent->_max_bad_blocks)
		slave->mtd._max_bad_blocks = part_max_bad_blocks;

	if (parent->_get_device)
		slave->mtd._get_device = part_get_device;
	if (parent->_put_device)
		slave->mtd._put_device = part_put_device;

	slave->mtd._erase = part_erase;
	slave->master = parent;
	slave->offset = part->offset;

	if (slave->offset == MTDPART_OFS_APPEND)
		slave->offset = cur_offset;
	if (slave->offset == MTDPART_OFS_NXTBLK) {
		tmp = cur_offset;
		slave->offset = cur_offset;
		remainder = do_div(tmp, wr_alignment);
		if (remainder) {
			slave->offset += wr_alignment - remainder;
			printk(KERN_NOTICE "Moving partition %d: "
			       "0x%012llx -> 0x%012llx\n", partno,
			       (unsigned long long)cur_offset, (unsigned long long)slave->offset);
		}
	}
	if (slave->offset == MTDPART_OFS_RETAIN) {
		slave->offset = cur_offset;
		if (parent->size - slave->offset >= slave->mtd.size) {
			slave->mtd.size = parent->size - slave->offset
							- slave->mtd.size;
		} else {
			printk(KERN_ERR "mtd partition \"%s\" doesn't have enough space: %#llx < %#llx, disabled\n",
				part->name, parent->size - slave->offset,
				slave->mtd.size);
			/* register to preserve ordering */
			goto out_register;
		}
	}
	if (slave->mtd.size == MTDPART_SIZ_FULL)
		slave->mtd.size = parent->size - slave->offset;

	printk(KERN_NOTICE "0x%012llx-0x%012llx : \"%s\"\n", (unsigned long long)slave->offset,
		(unsigned long long)(slave->offset + slave->mtd.size), slave->mtd.name);

	/* let's do some sanity checks */
	if (slave->offset >= parent->size) {
		/* let's register it anyway to preserve ordering */
		slave->offset = 0;
		slave->mtd.size = 0;
		printk(KERN_ERR"mtd: partition \"%s\" is out of reach -- disabled\n",
			part->name);
		goto out_register;
	}
	if (slave->offset + slave->mtd.size > parent->size) {
		slave->mtd.size = parent->size - slave->offset;
		printk(KERN_WARNING"mtd: partition \"%s\" extends beyond the end of device \"%s\" -- size truncated to %#llx\n",
			part->name, parent->name, (unsigned long long)slave->mtd.size);
	}
	if (parent->numeraseregions > 1) {
		/* Deal with variable erase size stuff */
		int i, max = parent->numeraseregions;
		u64 end = slave->offset + slave->mtd.size;
		struct mtd_erase_region_info *regions = parent->eraseregions;

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
		slave->mtd.erasesize = parent->erasesize;
	}

	tmp = slave->offset;
	remainder = do_div(tmp, wr_alignment);
	if ((slave->mtd.flags & MTD_WRITEABLE) && remainder) {
		/* Doesn't start on a boundary of major erase size */
		slave->mtd.flags |= MTD_ERASE_PARTIAL;
		if (((u32) slave->mtd.size) > parent->erasesize)
			slave->mtd.flags &= ~MTD_WRITEABLE;
		else
			slave->mtd.erasesize = slave->mtd.size;
	}
	tmp = slave->mtd.size;
	remainder = do_div(tmp, wr_alignment);
	if ((slave->mtd.flags & MTD_WRITEABLE) && remainder) {
		slave->mtd.flags |= MTD_ERASE_PARTIAL;

		if ((u32) slave->mtd.size > parent->erasesize)
			slave->mtd.flags &= ~MTD_WRITEABLE;
		else
			slave->mtd.erasesize = slave->mtd.size;
	}
	if ((slave->mtd.flags & (MTD_ERASE_PARTIAL|MTD_WRITEABLE)) == MTD_ERASE_PARTIAL)
		printk(KERN_WARNING"mtd: partition \"%s\" doesn't start on an erase/write block boundary -- force read-only\n",
				part->name);

	mtd_set_ooblayout(&slave->mtd, &part_ooblayout_ops);
	slave->mtd.ecc_step_size = parent->ecc_step_size;
	slave->mtd.ecc_strength = parent->ecc_strength;
	slave->mtd.bitflip_threshold = parent->bitflip_threshold;

	if (parent->_block_isbad) {
		uint64_t offs = 0;

		while (offs < slave->mtd.size) {
			if (mtd_block_isreserved(parent, offs + slave->offset))
				slave->mtd.ecc_stats.bbtblocks++;
			else if (mtd_block_isbad(parent, offs + slave->offset))
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
	struct mtd_part *part = mtd_to_part(mtd);
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

int mtd_add_partition(struct mtd_info *parent, const char *name,
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
		length = parent->size - offset;

	if (length <= 0)
		return -EINVAL;

	memset(&part, 0, sizeof(part));
	part.name = name;
	part.size = length;
	part.offset = offset;

	new = allocate_partition(parent, &part, -1, offset);
	if (IS_ERR(new))
		return PTR_ERR(new);

	mutex_lock(&mtd_partitions_mutex);
	list_add(&new->list, &mtd_partitions);
	mutex_unlock(&mtd_partitions_mutex);

	ret = add_mtd_device(&new->mtd);
	if (ret)
		goto err_remove_part;

	mtd_add_partition_attrs(new);

	return 0;

err_remove_part:
	mutex_lock(&mtd_partitions_mutex);
	list_del(&new->list);
	mutex_unlock(&mtd_partitions_mutex);

	free_partition(new);

	return ret;
}
EXPORT_SYMBOL_GPL(mtd_add_partition);

/**
 * __mtd_del_partition - delete MTD partition
 *
 * @priv: internal MTD struct for partition to be deleted
 *
 * This function must be called with the partitions mutex locked.
 */
static int __mtd_del_partition(struct mtd_part *priv)
{
	struct mtd_part *child, *next;
	int err;

	list_for_each_entry_safe(child, next, &mtd_partitions, list) {
		if (child->master == &priv->mtd) {
			err = __mtd_del_partition(child);
			if (err)
				return err;
		}
	}

	sysfs_remove_files(&priv->mtd.dev.kobj, mtd_partition_attrs);

	err = del_mtd_device(&priv->mtd);
	if (err)
		return err;

	list_del(&priv->list);
	free_partition(priv);

	return 0;
}

/*
 * This function unregisters and destroy all slave MTD objects which are
 * attached to the given MTD object.
 */
int del_mtd_partitions(struct mtd_info *mtd)
{
	struct mtd_part *slave, *next;
	int ret, err = 0;

	mutex_lock(&mtd_partitions_mutex);
	list_for_each_entry_safe(slave, next, &mtd_partitions, list)
		if (slave->master == mtd) {
			ret = __mtd_del_partition(slave);
			if (ret < 0)
				err = ret;
		}
	mutex_unlock(&mtd_partitions_mutex);

	return err;
}

int mtd_del_partition(struct mtd_info *mtd, int partno)
{
	struct mtd_part *slave, *next;
	int ret = -EINVAL;

	mutex_lock(&mtd_partitions_mutex);
	list_for_each_entry_safe(slave, next, &mtd_partitions, list)
		if ((slave->master == mtd) &&
		    (slave->mtd.index == partno)) {
			ret = __mtd_del_partition(slave);
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
	printk(KERN_EMERG " magic %X vs %X\n",sb.s_magic, SQUASHFS_MAGIC);
	if (SQUASHFS_MAGIC != sb.s_magic ) {
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
	int ret;

	printk(KERN_NOTICE "Creating %d MTD partitions on \"%s\":\n", nbparts, master->name);

	for (i = 0; i < nbparts; i++) {
		slave = allocate_partition(master, parts + i, i, cur_offset);
		if (IS_ERR(slave)) {
			ret = PTR_ERR(slave);
			goto err_del_partitions;
		}

		mutex_lock(&mtd_partitions_mutex);
		list_add(&slave->list, &mtd_partitions);
		mutex_unlock(&mtd_partitions_mutex);
		ret = add_mtd_device(&slave->mtd);
		if (ret) {
			mutex_lock(&mtd_partitions_mutex);
			list_del(&slave->list);
			mutex_unlock(&mtd_partitions_mutex);

			free_partition(slave);
			goto err_del_partitions;
		}
		mtd_add_partition_attrs(slave);
		if (parts[i].types)
			mtd_parse_part(slave, parts[i].types);
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

err_del_partitions:
	del_mtd_partitions(master);

	return ret;
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

static struct mtd_part_parser *mtd_part_parser_get(const char *name)
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

static inline void mtd_part_parser_put(const struct mtd_part_parser *p)
{
	module_put(p->owner);
}

/*
 * Many partition parsers just expected the core to kfree() all their data in
 * one chunk. Do that by default.
 */
static void mtd_part_parser_cleanup_default(const struct mtd_partition *pparts,
					    int nr_parts)
{
	kfree(pparts);
}

int __register_mtd_parser(struct mtd_part_parser *p, struct module *owner)
{
	p->owner = owner;

	if (!p->cleanup)
		p->cleanup = &mtd_part_parser_cleanup_default;

	spin_lock(&part_parser_lock);
	list_add(&p->list, &part_parsers);
	spin_unlock(&part_parser_lock);

	return 0;
}
EXPORT_SYMBOL_GPL(__register_mtd_parser);

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

static int mtd_part_do_parse(struct mtd_part_parser *parser,
			     struct mtd_info *master,
			     struct mtd_partitions *pparts,
			     struct mtd_part_parser_data *data)
{
	int ret;

	ret = (*parser->parse_fn)(master, &pparts->parts, data);
	pr_debug("%s: parser %s: %i\n", master->name, parser->name, ret);
	if (ret <= 0)
		return ret;

	pr_notice("%d %s partitions found on MTD device %s\n", ret,
		  parser->name, master->name);

	pparts->nr_parts = ret;
	pparts->parser = parser;

	return ret;
}

/**
 * parse_mtd_partitions - parse MTD partitions
 * @master: the master partition (describes whole MTD device)
 * @types: names of partition parsers to try or %NULL
 * @pparts: info about partitions found is returned here
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
 * o zero otherwise, and @pparts will describe the partitions, number of
 *   partitions, and the parser which parsed them. Caller must release
 *   resources with mtd_part_parser_cleanup() when finished with the returned
 *   data.
 */
int parse_mtd_partitions(struct mtd_info *master, const char *const *types,
			 struct mtd_partitions *pparts,
			 struct mtd_part_parser_data *data)
{
	struct mtd_part_parser *parser;
	int ret, err = 0;

	if (!types)
		types = default_mtd_part_types;

	for ( ; *types; types++) {
		pr_debug("%s: parsing partitions %s\n", master->name, *types);
		parser = mtd_part_parser_get(*types);
		if (!parser && !request_module("%s", *types))
			parser = mtd_part_parser_get(*types);
		pr_debug("%s: got parser %s\n", master->name,
			 parser ? parser->name : NULL);
		if (!parser)
			continue;
		ret = mtd_part_do_parse(parser, master, pparts, data);
		/* Found partitions! */
		if (ret > 0)
			return 0;
		mtd_part_parser_put(parser);
		/*
		 * Stash the first error we see; only report it if no parser
		 * succeeds
		 */
		if (ret < 0 && !err)
			err = ret;
	}
	return err;
}

void mtd_part_parser_cleanup(struct mtd_partitions *parts)
{
	const struct mtd_part_parser *parser;

	if (!parts)
		return;

	parser = parts->parser;
	if (parser) {
		if (parser->cleanup)
			parser->cleanup(parts->parts, parts->nr_parts);

		mtd_part_parser_put(parser);
	}
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

	return mtd_get_device_size(mtd_to_part(mtd)->master);
}
EXPORT_SYMBOL_GPL(mtd_get_device_size);



#ifdef CONFIG_RALINK
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