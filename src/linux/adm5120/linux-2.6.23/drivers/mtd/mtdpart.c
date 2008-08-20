/*
 * Simple MTD partitioning layer
 *
 * (C) 2000 Nicolas Pitre <nico@cam.org>
 *
 * This code is GPL
 *
 * $Id: mtdpart.c,v 1.55 2005/11/07 11:14:20 gleixner Exp $
 *
 * 	02-21-2002	Thomas Gleixner <gleixner@autronix.de>
 *			added support for read_oob, write_oob
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/kmod.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/compatmac.h>
#include <linux/squashfs_fs.h>
#include <linux/root_dev.h>

/* Our partition linked list */
static LIST_HEAD(mtd_partitions);

/* Our partition node structure */
struct mtd_part {
	struct mtd_info mtd;
	struct mtd_info *master;
	u_int32_t offset;
	int index;
	struct list_head list;
	int registered;
};

/*
 * Given a pointer to the MTD object in the mtd_part structure, we can retrieve
 * the pointer to that structure with this macro.
 */
#define PART(x)  ((struct mtd_part *)(x))
#define IS_PART(mtd) (mtd->read == part_read)

/*
 * MTD methods which simply translate the effective address and pass through
 * to the _real_ device.
 */

static int part_read (struct mtd_info *mtd, loff_t from, size_t len,
			size_t *retlen, u_char *buf)
{
	struct mtd_part *part = PART(mtd);
	int res;

	if (from >= mtd->size)
		len = 0;
	else if (from + len > mtd->size)
		len = mtd->size - from;
	res = part->master->read (part->master, from + part->offset,
				   len, retlen, buf);
	if (unlikely(res)) {
		if (res == -EUCLEAN)
			mtd->ecc_stats.corrected++;
		if (res == -EBADMSG)
			mtd->ecc_stats.failed++;
	}
	return res;
}

static int part_point (struct mtd_info *mtd, loff_t from, size_t len,
			size_t *retlen, u_char **buf)
{
	struct mtd_part *part = PART(mtd);
	if (from >= mtd->size)
		len = 0;
	else if (from + len > mtd->size)
		len = mtd->size - from;
	return part->master->point (part->master, from + part->offset,
				    len, retlen, buf);
}

static void part_unpoint (struct mtd_info *mtd, u_char *addr, loff_t from, size_t len)
{
	struct mtd_part *part = PART(mtd);

	part->master->unpoint (part->master, addr, from + part->offset, len);
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
	res = part->master->read_oob(part->master, from + part->offset, ops);

	if (unlikely(res)) {
		if (res == -EUCLEAN)
			mtd->ecc_stats.corrected++;
		if (res == -EBADMSG)
			mtd->ecc_stats.failed++;
	}
	return res;
}

static int part_read_user_prot_reg (struct mtd_info *mtd, loff_t from, size_t len,
			size_t *retlen, u_char *buf)
{
	struct mtd_part *part = PART(mtd);
	return part->master->read_user_prot_reg (part->master, from,
					len, retlen, buf);
}

static int part_get_user_prot_info (struct mtd_info *mtd,
				    struct otp_info *buf, size_t len)
{
	struct mtd_part *part = PART(mtd);
	return part->master->get_user_prot_info (part->master, buf, len);
}

static int part_read_fact_prot_reg (struct mtd_info *mtd, loff_t from, size_t len,
			size_t *retlen, u_char *buf)
{
	struct mtd_part *part = PART(mtd);
	return part->master->read_fact_prot_reg (part->master, from,
					len, retlen, buf);
}

static int part_get_fact_prot_info (struct mtd_info *mtd,
				    struct otp_info *buf, size_t len)
{
	struct mtd_part *part = PART(mtd);
	return part->master->get_fact_prot_info (part->master, buf, len);
}

static int part_write (struct mtd_info *mtd, loff_t to, size_t len,
			size_t *retlen, const u_char *buf)
{
	struct mtd_part *part = PART(mtd);
	if (!(mtd->flags & MTD_WRITEABLE))
		return -EROFS;
	if (to >= mtd->size)
		len = 0;
	else if (to + len > mtd->size)
		len = mtd->size - to;
	return part->master->write (part->master, to + part->offset,
				    len, retlen, buf);
}

static int part_write_oob(struct mtd_info *mtd, loff_t to,
			 struct mtd_oob_ops *ops)
{
	struct mtd_part *part = PART(mtd);

	if (!(mtd->flags & MTD_WRITEABLE))
		return -EROFS;

	if (to >= mtd->size)
		return -EINVAL;
	if (ops->datbuf && to + ops->len > mtd->size)
		return -EINVAL;
	return part->master->write_oob(part->master, to + part->offset, ops);
}

static int part_write_user_prot_reg (struct mtd_info *mtd, loff_t from, size_t len,
			size_t *retlen, u_char *buf)
{
	struct mtd_part *part = PART(mtd);
	return part->master->write_user_prot_reg (part->master, from,
					len, retlen, buf);
}

static int part_lock_user_prot_reg (struct mtd_info *mtd, loff_t from, size_t len)
{
	struct mtd_part *part = PART(mtd);
	return part->master->lock_user_prot_reg (part->master, from, len);
}

static int part_writev (struct mtd_info *mtd,  const struct kvec *vecs,
			 unsigned long count, loff_t to, size_t *retlen)
{
	struct mtd_part *part = PART(mtd);
	if (!(mtd->flags & MTD_WRITEABLE))
		return -EROFS;
	return part->master->writev (part->master, vecs, count,
					to + part->offset, retlen);
}

static int part_erase (struct mtd_info *mtd, struct erase_info *instr)
{
	struct mtd_part *part = PART(mtd);
	int ret;
	if (!(mtd->flags & MTD_WRITEABLE))
		return -EROFS;
	if (instr->addr >= mtd->size)
		return -EINVAL;
	instr->addr += part->offset;
	ret = part->master->erase(part->master, instr);
	if (ret) {
		if (instr->fail_addr != 0xffffffff)
			instr->fail_addr -= part->offset;
		instr->addr -= part->offset;
	}
	return ret;
}

void mtd_erase_callback(struct erase_info *instr)
{
	if (instr->mtd->erase == part_erase) {
		struct mtd_part *part = PART(instr->mtd);

		if (instr->fail_addr != 0xffffffff)
			instr->fail_addr -= part->offset;
		instr->addr -= part->offset;
	}
	if (instr->callback)
		instr->callback(instr);
}
EXPORT_SYMBOL_GPL(mtd_erase_callback);

static int part_lock (struct mtd_info *mtd, loff_t ofs, size_t len)
{
	struct mtd_part *part = PART(mtd);
	if ((len + ofs) > mtd->size)
		return -EINVAL;
	return part->master->lock(part->master, ofs + part->offset, len);
}

static int part_unlock (struct mtd_info *mtd, loff_t ofs, size_t len)
{
	struct mtd_part *part = PART(mtd);
	if ((len + ofs) > mtd->size)
		return -EINVAL;
	return part->master->unlock(part->master, ofs + part->offset, len);
}

static void part_sync(struct mtd_info *mtd)
{
	struct mtd_part *part = PART(mtd);
	part->master->sync(part->master);
}

static int part_suspend(struct mtd_info *mtd)
{
	struct mtd_part *part = PART(mtd);
	return part->master->suspend(part->master);
}

static void part_resume(struct mtd_info *mtd)
{
	struct mtd_part *part = PART(mtd);
	part->master->resume(part->master);
}

static int part_block_isbad (struct mtd_info *mtd, loff_t ofs)
{
	struct mtd_part *part = PART(mtd);
	if (ofs >= mtd->size)
		return -EINVAL;
	ofs += part->offset;
	return part->master->block_isbad(part->master, ofs);
}

static int part_block_markbad (struct mtd_info *mtd, loff_t ofs)
{
	struct mtd_part *part = PART(mtd);
	int res;

	if (!(mtd->flags & MTD_WRITEABLE))
		return -EROFS;
	if (ofs >= mtd->size)
		return -EINVAL;
	ofs += part->offset;
	res = part->master->block_markbad(part->master, ofs);
	if (!res)
		mtd->ecc_stats.badblocks++;
	return res;
}

/*
 * This function unregisters and destroy all slave MTD objects which are
 * attached to the given master MTD object.
 */

int del_mtd_partitions(struct mtd_info *master)
{
	struct list_head *node;
	struct mtd_part *slave;

	for (node = mtd_partitions.next;
	     node != &mtd_partitions;
	     node = node->next) {
		slave = list_entry(node, struct mtd_part, list);
		if (slave->master == master) {
			struct list_head *prev = node->prev;
			__list_del(prev, node->next);
			if(slave->registered)
				del_mtd_device(&slave->mtd);
			kfree(slave);
			node = prev;
		}
	}

	return 0;
}

static u_int32_t cur_offset = 0;
static int add_one_partition(struct mtd_info *master, const struct mtd_partition *part,
		int i, struct mtd_part **slp)
{
	struct mtd_part *slave;

	/* allocate the partition structure */
	slave = kzalloc (sizeof(*slave), GFP_KERNEL);
	if (!slave) {
		printk ("memory allocation error while creating partitions for \"%s\"\n",
			master->name);
		del_mtd_partitions(master);
		return -ENOMEM;
	}
	list_add(&slave->list, &mtd_partitions);

	/* set up the MTD object for this partition */
	slave->mtd.type = master->type;
	slave->mtd.flags = master->flags & ~part->mask_flags;
	slave->mtd.size = part->size;
	slave->mtd.writesize = master->writesize;
	slave->mtd.oobsize = master->oobsize;
	slave->mtd.oobavail = master->oobavail;
	slave->mtd.subpage_sft = master->subpage_sft;

	slave->mtd.name = part->name;
	slave->mtd.owner = master->owner;

	slave->mtd.read = part_read;
	slave->mtd.write = part_write;
	slave->mtd.refresh_device = part->refresh_partition;

	if(master->point && master->unpoint){
		slave->mtd.point = part_point;
		slave->mtd.unpoint = part_unpoint;
	}

	if (master->read_oob)
		slave->mtd.read_oob = part_read_oob;
	if (master->write_oob)
		slave->mtd.write_oob = part_write_oob;
	if(master->read_user_prot_reg)
		slave->mtd.read_user_prot_reg = part_read_user_prot_reg;
	if(master->read_fact_prot_reg)
		slave->mtd.read_fact_prot_reg = part_read_fact_prot_reg;
	if(master->write_user_prot_reg)
		slave->mtd.write_user_prot_reg = part_write_user_prot_reg;
	if(master->lock_user_prot_reg)
		slave->mtd.lock_user_prot_reg = part_lock_user_prot_reg;
	if(master->get_user_prot_info)
		slave->mtd.get_user_prot_info = part_get_user_prot_info;
	if(master->get_fact_prot_info)
		slave->mtd.get_fact_prot_info = part_get_fact_prot_info;
	if (master->sync)
		slave->mtd.sync = part_sync;
	if (!i && master->suspend && master->resume) {
			slave->mtd.suspend = part_suspend;
			slave->mtd.resume = part_resume;
	}
	if (master->writev)
		slave->mtd.writev = part_writev;
	if (master->lock)
		slave->mtd.lock = part_lock;
	if (master->unlock)
		slave->mtd.unlock = part_unlock;
	if (master->block_isbad)
		slave->mtd.block_isbad = part_block_isbad;
	if (master->block_markbad)
		slave->mtd.block_markbad = part_block_markbad;
	slave->mtd.erase = part_erase;
	slave->master = master;
	slave->offset = part->offset;
	slave->index = i;

	if (slave->offset == MTDPART_OFS_APPEND)
		slave->offset = cur_offset;
	if (slave->offset == MTDPART_OFS_NXTBLK) {
		slave->offset = cur_offset;
		if ((cur_offset % master->erasesize) != 0) {
			/* Round up to next erasesize */
			slave->offset = ((cur_offset / master->erasesize) + 1) * master->erasesize;
			printk(KERN_NOTICE "Moving partition %d: "
			       "0x%08x -> 0x%08x\n", i,
			       cur_offset, slave->offset);
		}
	}
	if (slave->mtd.size == MTDPART_SIZ_FULL)
		slave->mtd.size = master->size - slave->offset;
	cur_offset = slave->offset + slave->mtd.size;

	printk (KERN_NOTICE "0x%08x-0x%08x : \"%s\"\n", slave->offset,
		slave->offset + slave->mtd.size, slave->mtd.name);

	/* let's do some sanity checks */
	if (slave->offset >= master->size) {
			/* let's register it anyway to preserve ordering */
		slave->offset = 0;
		slave->mtd.size = 0;
		printk ("mtd: partition \"%s\" is out of reach -- disabled\n",
			part->name);
	}
	if (slave->offset + slave->mtd.size > master->size) {
		slave->mtd.size = master->size - slave->offset;
		printk ("mtd: partition \"%s\" extends beyond the end of device \"%s\" -- size truncated to %#x\n",
			part->name, master->name, slave->mtd.size);
	}
	if (master->numeraseregions>1) {
		/* Deal with variable erase size stuff */
		int i;
		struct mtd_erase_region_info *regions = master->eraseregions;

		/* Find the first erase regions which is part of this partition. */
		for (i=0; i < master->numeraseregions && slave->offset >= regions[i].offset; i++)
			;

		for (i--; i < master->numeraseregions && slave->offset + slave->mtd.size > regions[i].offset; i++) {
			if (slave->mtd.erasesize < regions[i].erasesize) {
				slave->mtd.erasesize = regions[i].erasesize;
			}
		}
	} else {
		/* Single erase size */
		slave->mtd.erasesize = master->erasesize;
	}

	if ((slave->mtd.flags & MTD_WRITEABLE) &&
	    (slave->offset % slave->mtd.erasesize)) {
		/* Doesn't start on a boundary of major erase size */
		/* FIXME: Let it be writable if it is on a boundary of _minor_ erase size though */
		slave->mtd.flags &= ~MTD_WRITEABLE;
		printk ("mtd: partition \"%s\" doesn't start on an erase block boundary -- force read-only\n",
			part->name);
	}
	if ((slave->mtd.flags & MTD_WRITEABLE) &&
	    (slave->mtd.size % slave->mtd.erasesize)) {
		slave->mtd.flags &= ~MTD_WRITEABLE;
		printk ("mtd: partition \"%s\" doesn't end on an erase block -- force read-only\n",
			part->name);
	}

	slave->mtd.ecclayout = master->ecclayout;
	if (master->block_isbad) {
		uint32_t offs = 0;

		while(offs < slave->mtd.size) {
			if (master->block_isbad(master,
						offs + slave->offset))
				slave->mtd.ecc_stats.badblocks++;
			offs += slave->mtd.erasesize;
		}
	}

	if(part->mtdp)
	{	/* store the object pointer (caller may or may not register it */
		*part->mtdp = &slave->mtd;
		slave->registered = 0;
	}
	else
	{
		/* register our partition */
		add_mtd_device(&slave->mtd);
		slave->registered = 1;
	}

	if (slp)
		*slp = slave;

	return 0;
}

#ifdef CONFIG_MTD_ROOTFS_SPLIT
#define ROOTFS_SPLIT_NAME "rootfs_data"
#define ROOTFS_REMOVED_NAME "<removed>"
static int split_squashfs(struct mtd_info *master, int offset, int *split_offset)
{
	char buf[512];
	struct squashfs_super_block *sb = (struct squashfs_super_block *) buf;
	int len, ret;

	ret = master->read(master, offset, sizeof(*sb), &len, buf);
	if (ret || (len != sizeof(*sb))) {
		printk(KERN_ALERT "split_squashfs: error occured while reading "
			"from \"%s\"\n", master->name);
		return -EINVAL;
	}

	if (*((u32 *) buf) != SQUASHFS_MAGIC) {
		printk(KERN_ALERT "split_squashfs: no squashfs found in \"%s\"\n",
			master->name);
		*split_offset = 0;
		return 0;
	}

	if (sb->bytes_used <= 0) {
		printk(KERN_ALERT "split_squashfs: squashfs is empty in \"%s\"\n",
			master->name);
		*split_offset = 0;
		return 0;
	}

	len = (u32) sb->bytes_used;
	len += (offset & 0x000fffff);
	len +=  (master->erasesize - 1);
	len &= ~(master->erasesize - 1);
	len -= (offset & 0x000fffff);
	*split_offset = offset + len;

	return 0;
}

static int split_rootfs_data(struct mtd_info *master, struct mtd_info *rpart, struct mtd_partition *part,
		int index)
{
	struct mtd_partition *dpart;
	struct mtd_part *slave = NULL;
	int split_offset = 0;
	int ret;

	ret = split_squashfs(master, part->offset, &split_offset);
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

	dpart->size -= split_offset - dpart->offset;
	dpart->offset = split_offset;

	if (dpart == NULL)
		return 1;

	printk(KERN_INFO "mtd: partition \"%s\" created automatically, ofs=%X, len=%X \n",
		ROOTFS_SPLIT_NAME, dpart->offset, dpart->size);

	ret = add_one_partition(master, dpart, index, &slave);
	if (ret)
		kfree(dpart);
	else if (slave)
		rpart->split = &slave->mtd;

	return ret;
}

static int refresh_rootfs_split(struct mtd_info *mtd)
{
	struct mtd_partition tpart;
	struct mtd_part *part;
	int index = 0;
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
		tpart.name = mtd->name;
		tpart.size = mtd->size;
		tpart.offset = part->offset;

		/* find the index of the last partition */
		if (!list_empty(&mtd_partitions))
			index = list_first_entry(&mtd_partitions, struct mtd_part, list)->index + 1;

		return split_rootfs_data(part->master, &part->mtd, &tpart, index);
	} else if ((offset > 0) && mtd->split) {
		/* update the offsets of the existing partition */
		size = mtd->size + part->offset - offset;

		part = PART(mtd->split);
		part->offset = offset;
		part->mtd.size = size;
		printk(KERN_INFO "%s: %s partition \"" ROOTFS_SPLIT_NAME "\", offset: 0x%06x (0x%06x)\n",
			__func__, (!strcmp(part->mtd.name, ROOTFS_SPLIT_NAME) ? "updating" : "creating"),
			part->offset, part->mtd.size);
		strcpy(part->mtd.name, ROOTFS_SPLIT_NAME);
	} else if ((offset <= 0) && mtd->split) {
		printk(KERN_INFO "%s: removing partition \"%s\"\n", __func__, mtd->split->name);

		/* mark existing partition as removed */
		part = PART(mtd->split);
		strcpy(part->mtd.name, ROOTFS_REMOVED_NAME);
		part->offset = 0;
		part->mtd.size = 0;
	}

	return 0;
}
#endif /* CONFIG_MTD_ROOTFS_SPLIT */

/*
 * This function, given a master MTD object and a partition table, creates
 * and registers slave MTD objects which are bound to the master according to
 * the partition definitions.
 * (Q: should we register the master MTD object as well?)
 */

int add_mtd_partitions(struct mtd_info *master,
		       const struct mtd_partition *parts,
		       int nbparts)
{
	struct mtd_part *slave;
	struct mtd_partition *part;
	int i, j, ret = 0;

	printk (KERN_NOTICE "Creating %d MTD partitions on \"%s\":\n", nbparts, master->name);

	for (i = 0, j = 0; i < nbparts; i++) {
		part = (struct mtd_partition *) &parts[i];
		ret = add_one_partition(master, part, j, &slave);
		if (ret)
			return ret;
		j++;

		if (strcmp(part->name, "rootfs") == 0 && slave->registered) {
#ifdef CONFIG_MTD_ROOTFS_ROOT_DEV
			if (ROOT_DEV == 0) {
				printk(KERN_NOTICE "mtd: partition \"rootfs\" "
					"set to be root filesystem\n");
				ROOT_DEV = MKDEV(MTD_BLOCK_MAJOR, slave->mtd.index);
			}
#endif
#ifdef CONFIG_MTD_ROOTFS_SPLIT
			ret = split_rootfs_data(master, &slave->mtd, part, j);
			if (ret == 0)
				j++;
#endif
		}
	}

	return 0;
}

EXPORT_SYMBOL(add_mtd_partitions);
EXPORT_SYMBOL(del_mtd_partitions);

static DEFINE_SPINLOCK(part_parser_lock);
static LIST_HEAD(part_parsers);

static struct mtd_part_parser *get_partition_parser(const char *name)
{
	struct list_head *this;
	void *ret = NULL;
	spin_lock(&part_parser_lock);

	list_for_each(this, &part_parsers) {
		struct mtd_part_parser *p = list_entry(this, struct mtd_part_parser, list);

		if (!strcmp(p->name, name) && try_module_get(p->owner)) {
			ret = p;
			break;
		}
	}
	spin_unlock(&part_parser_lock);

	return ret;
}

int register_mtd_parser(struct mtd_part_parser *p)
{
	spin_lock(&part_parser_lock);
	list_add(&p->list, &part_parsers);
	spin_unlock(&part_parser_lock);

	return 0;
}

int deregister_mtd_parser(struct mtd_part_parser *p)
{
	spin_lock(&part_parser_lock);
	list_del(&p->list);
	spin_unlock(&part_parser_lock);
	return 0;
}

int parse_mtd_partitions(struct mtd_info *master, const char **types,
			 struct mtd_partition **pparts, unsigned long origin)
{
	struct mtd_part_parser *parser;
	int ret = 0;

	for ( ; ret <= 0 && *types; types++) {
		parser = get_partition_parser(*types);
#ifdef CONFIG_KMOD
		if (!parser && !request_module("%s", *types))
				parser = get_partition_parser(*types);
#endif
		if (!parser) {
			printk(KERN_NOTICE "%s partition parsing not available\n",
			       *types);
			continue;
		}
		ret = (*parser->parse_fn)(master, pparts, origin);
		if (ret > 0) {
			printk(KERN_NOTICE "%d %s partitions found on MTD device %s\n",
			       ret, parser->name, master->name);
		}
		put_partition_parser(parser);
	}
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

EXPORT_SYMBOL_GPL(parse_mtd_partitions);
EXPORT_SYMBOL_GPL(refresh_mtd_partitions);
EXPORT_SYMBOL_GPL(register_mtd_parser);
EXPORT_SYMBOL_GPL(deregister_mtd_parser);
