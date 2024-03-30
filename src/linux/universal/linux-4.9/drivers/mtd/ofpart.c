/*
 * Flash partitions described by the OF (or flattened) device tree
 *
 * Copyright © 2006 MontaVista Software Inc.
 * Author: Vitaly Wool <vwool@ru.mvista.com>
 *
 * Revised to handle newer style flash binding by:
 *   Copyright © 2007 David Gibson, IBM Corporation.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/mtd/mtd.h>
#include <linux/slab.h>
#include <linux/mtd/partitions.h>
#include <asm/setup.h>

struct squashfs_super_block {
	__le32			s_magic;
	__le32			inodes;
	__le32			mkfs_time;
	__le32			block_size;
	__le32			fragments;
	__le16			compression;
	__le16			block_log;
	__le16			flags;
	__le16			no_ids;
	__le16			s_major;
	__le16			s_minor;
	__le64			root_inode;
	__le64			bytes_used;
	__le64			id_table_start;
	__le64			xattr_id_table_start;
	__le64			inode_table_start;
	__le64			directory_table_start;
	__le64			fragment_table_start;
	__le64			lookup_table_start;
};

static bool node_has_compatible(struct device_node *pp)
{
	return of_get_property(pp, "compatible", NULL);
}

static int mangled_rootblock;
static int parse_ofpart_partitions(struct mtd_info *master,
				   const struct mtd_partition **pparts,
				   struct mtd_part_parser_data *data)
{
	struct mtd_partition *parts;
	struct device_node *mtd_node;
	struct device_node *ofpart_node;
	const char *partname;
	const char *owrtpart = "ubi";
	struct device_node *pp;
	int nr_parts, i, ret = 0, len;
	bool dedicated = true;
	struct squashfs_super_block sb;


	/* Pull of_node from the master device node */
	mtd_node = mtd_get_of_node(master);
	if (!mtd_node)
		return 0;

#ifdef CONFIG_ARCH_MVEBU
	for (i = 0;i < COMMAND_LINE_SIZE - sizeof("/dev/mtdblockxx"); i++) {
	    if (!memcmp(&boot_command_line[i],"/dev/mtdblock",13)) {
		    printk(KERN_INFO "found commandline\n");
		    mangled_rootblock = boot_command_line[i + 13] - '0';
		    break;
	    }
	}
	printk(KERN_INFO "rename part %d to ubi",mangled_rootblock);
#endif

	ofpart_node = of_get_child_by_name(mtd_node, "partitions");
	if (!ofpart_node) {
		/*
		 * We might get here even when ofpart isn't used at all (e.g.,
		 * when using another parser), so don't be louder than
		 * KERN_DEBUG
		 */
		pr_debug("%s: 'partitions' subnode not found on %s. Trying to parse direct subnodes as partitions.\n",
			 master->name, mtd_node->full_name);
		ofpart_node = mtd_node;
		dedicated = false;
	} else if (!of_device_is_compatible(ofpart_node, "fixed-partitions")) {
		/* The 'partitions' subnode might be used by another parser */
		return 0;
	}

	/* First count the subnodes */
	nr_parts = 0;
	for_each_child_of_node(ofpart_node,  pp) {
		if (!dedicated && node_has_compatible(pp))
			continue;

		nr_parts++;
	}
	#ifdef CONFIG_SOC_IMX6
		nr_parts+=2; // for nvram
	#endif

	if (nr_parts == 0)
		return 0;

	parts = kzalloc(nr_parts * sizeof(*parts), GFP_KERNEL);
	if (!parts)
		return -ENOMEM;

	i = 0;
	for_each_child_of_node(ofpart_node,  pp) {
		const __be32 *reg;
		int len;
		int a_cells, s_cells;

		if (!dedicated && node_has_compatible(pp))
			continue;

		reg = of_get_property(pp, "reg", &len);
		if (!reg) {
			if (dedicated) {
				pr_debug("%s: ofpart partition %s (%s) missing reg property.\n",
					 master->name, pp->full_name,
					 mtd_node->full_name);
				goto ofpart_fail;
			} else {
				nr_parts--;
				continue;
			}
		}

		a_cells = of_n_addr_cells(pp);
		s_cells = of_n_size_cells(pp);
		if (len / 4 != a_cells + s_cells) {
			pr_debug("%s: ofpart partition %s (%s) error parsing reg property.\n",
				 master->name, pp->full_name,
				 mtd_node->full_name);
			goto ofpart_fail;
		}

		parts[i].offset = of_read_number(reg, a_cells);
		parts[i].size = of_read_number(reg + a_cells, s_cells);
		parts[i].of_node = pp;

		if (mangled_rootblock && (i == mangled_rootblock)) {
				partname = owrtpart;
		} else {
			partname = of_get_property(pp, "label", &len);

			if (!partname)
				partname = of_get_property(pp, "name", &len);
		}
 
		parts[i].name = partname;

		if (of_get_property(pp, "read-only", &len))
			parts[i].mask_flags |= MTD_WRITEABLE;

		if (of_get_property(pp, "lock", &len))
			parts[i].mask_flags |= MTD_POWERUP_LOCK;
#ifdef CONFIG_ARCH_QCOM
		if (!strcmp(partname, "linux") || !strcmp(partname, "linux2")) {
			int offset = parts[i].offset;
			while ((offset + master->erasesize) < master->size) {
				mtd_read(master, offset, sizeof(sb), &len, (void *)&sb);
				if (le32_to_cpu(sb.s_magic) == 0x23494255) {
					printk(KERN_INFO "found ubi at %X, skipping scan\n", offset);
					break; // skip if ubifs has been found
				}
				if (le32_to_cpu(sb.s_magic) == SQUASHFS_MAGIC) {
					len = le64_to_cpu(sb.bytes_used);
					len += (offset & 0x000fffff);
					len += (master->erasesize - 1);
					len &= ~(master->erasesize - 1);
					len -= (offset & 0x000fffff);
					printk(KERN_INFO "found squashfs at %X with len of %d bytes\n", offset, len);
					i++;
					parts[i].offset = offset;
					if (!strcmp(partname, "linux2"))
						parts[i].name = "rootfs2";
					else
						parts[i].name = "rootfs";
					parts[i].size = len;
					parts[i].mask_flags = 0;
					nr_parts++;
					break;
				}
				offset += 4096;
			}
		}
#endif

		#ifdef CONFIG_SOC_IMX6
		// for ventana, we hack a nvram partition into the layout
		if (!strcmp(partname,"env")) {
			parts[i].size -= 0x80000;
			i++;
			parts[i].offset = parts[i-1].offset + parts[i-1].size;
			parts[i].size = 0x40000;	
			parts[i].mask_flags = 0;	
			parts[i].name = "nvram";
			i++;
			parts[i].offset = parts[i-1].offset + parts[i-1].size;
			parts[i].size = 0x40000;	
			parts[i].mask_flags = 0;	
			parts[i].name = "mampf";
		}    
		#endif
		i++;
	}

	if (!nr_parts)
		goto ofpart_none;

	*pparts = parts;
	return nr_parts;

ofpart_fail:
	pr_err("%s: error parsing ofpart partition %s (%s)\n",
	       master->name, pp->full_name, mtd_node->full_name);
	ret = -EINVAL;
ofpart_none:
	of_node_put(pp);
	kfree(parts);
	return ret;
}

static struct mtd_part_parser ofpart_parser = {
	.parse_fn = parse_ofpart_partitions,
	.name = "ofpart",
};

static int parse_ofoldpart_partitions(struct mtd_info *master,
				      const struct mtd_partition **pparts,
				      struct mtd_part_parser_data *data)
{
	struct mtd_partition *parts;
	struct device_node *dp;
	int i, plen, nr_parts;
	const struct {
		__be32 offset, len;
	} *part;
	const char *names;

	/* Pull of_node from the master device node */
	dp = mtd_get_of_node(master);
	if (!dp)
		return 0;

	part = of_get_property(dp, "partitions", &plen);
	if (!part)
		return 0; /* No partitions found */

	pr_warning("Device tree uses obsolete partition map binding: %s\n",
			dp->full_name);

	nr_parts = plen / sizeof(part[0]);

	parts = kzalloc(nr_parts * sizeof(*parts), GFP_KERNEL);
	if (!parts)
		return -ENOMEM;

	names = of_get_property(dp, "partition-names", &plen);

	for (i = 0; i < nr_parts; i++) {
		parts[i].offset = be32_to_cpu(part->offset);
		parts[i].size   = be32_to_cpu(part->len) & ~1;
		/* bit 0 set signifies read only partition */
		if (be32_to_cpu(part->len) & 1)
			parts[i].mask_flags = MTD_WRITEABLE;

		if (names && (plen > 0)) {
			int len = strlen(names) + 1;

			parts[i].name = names;
			plen -= len;
			names += len;
		} else {
			parts[i].name = "unnamed";
		}

		part++;
	}

	*pparts = parts;
	return nr_parts;
}

static struct mtd_part_parser ofoldpart_parser = {
	.parse_fn = parse_ofoldpart_partitions,
	.name = "ofoldpart",
};

static int __init ofpart_parser_init(void)
{
	register_mtd_parser(&ofpart_parser);
	register_mtd_parser(&ofoldpart_parser);
	return 0;
}

static int __init active_root(char *str)
{
	get_option(&str, &mangled_rootblock);

	if (!mangled_rootblock)
		return 1;

	return 1;
}

__setup("mangled_rootblock=", active_root);

static void __exit ofpart_parser_exit(void)
{
	deregister_mtd_parser(&ofpart_parser);
	deregister_mtd_parser(&ofoldpart_parser);
}

module_init(ofpart_parser_init);
module_exit(ofpart_parser_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Parser for MTD partitioning information in device tree");
MODULE_AUTHOR("Vitaly Wool, David Gibson");
/*
 * When MTD core cannot find the requested parser, it tries to load the module
 * with the same name. Since we provide the ofoldpart parser, we should have
 * the corresponding alias.
 */
MODULE_ALIAS("ofoldpart");
