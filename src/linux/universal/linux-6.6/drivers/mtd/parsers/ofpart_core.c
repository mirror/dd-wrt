// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Flash partitions described by the OF (or flattened) device tree
 *
 * Copyright © 2006 MontaVista Software Inc.
 * Author: Vitaly Wool <vwool@ru.mvista.com>
 *
 * Revised to handle newer style flash binding by:
 *   Copyright © 2007 David Gibson, IBM Corporation.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/mtd/mtd.h>
#include <linux/slab.h>
#include <linux/mtd/partitions.h>
#include <linux/magic.h>
#include <asm/setup.h>

#include "ofpart_bcm4908.h"
#include "ofpart_linksys_ns.h"


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



struct fixed_partitions_quirks {
	int (*post_parse)(struct mtd_info *mtd, struct mtd_partition *parts, int nr_parts);
};

static struct fixed_partitions_quirks bcm4908_partitions_quirks = {
	.post_parse = bcm4908_partitions_post_parse,
};

static struct fixed_partitions_quirks linksys_ns_partitions_quirks = {
	.post_parse = linksys_ns_partitions_post_parse,
};

static const struct of_device_id parse_ofpart_match_table[];

static bool node_has_compatible(struct device_node *pp)
{
	return of_get_property(pp, "compatible", NULL);
}

static int mangled_rootblock;

static int mangled_rootblock;

static int parse_fixed_partitions(struct mtd_info *master,
				  const struct mtd_partition **pparts,
				  struct mtd_part_parser_data *data)
{
	const struct fixed_partitions_quirks *quirks;
	const struct of_device_id *of_id;
	struct mtd_partition *parts;
	struct device_node *mtd_node;
	struct device_node *ofpart_node;
	const char *partname;
	const char *owrtpart = "ubi";
	struct device_node *pp;
	int nr_parts, i, ret = 0;
	bool dedicated = true;
	struct squashfs_super_block sb;

	/* Pull of_node from the master device node */
	mtd_node = mtd_get_of_node(master);
	if (!mtd_node)
		return 0;

#ifdef CONFIG_ARCH_MVEBU
	for (i = 0;i < COMMAND_LINE_SIZE - sizeof("/dev/mtdblockxx"); i++) {
	    if (!memcmp(&boot_command_line[i],"/dev/mtdblock",13)) {
		    mangled_rootblock = boot_command_line[i + 13] - '0';
		    break;
	    }
	}
#endif

	if (!master->parent) { /* Master */
		ofpart_node = of_get_child_by_name(mtd_node, "partitions");
		if (!ofpart_node) {
			/*
			 * We might get here even when ofpart isn't used at all (e.g.,
			 * when using another parser), so don't be louder than
			 * KERN_DEBUG
			 */
			pr_debug("%s: 'partitions' subnode not found on %pOF. Trying to parse direct subnodes as partitions.\n",
				master->name, mtd_node);
			ofpart_node = mtd_node;
			dedicated = false;
		}
	} else { /* Partition */
		ofpart_node = mtd_node;
	}

	of_id = of_match_node(parse_ofpart_match_table, ofpart_node);
	if (dedicated && !of_id) {
		/* The 'partitions' subnode might be used by another parser */
		return 0;
	}

	quirks = of_id ? of_id->data : NULL;

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

	parts = kcalloc(nr_parts, sizeof(*parts), GFP_KERNEL);
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
				pr_debug("%s: ofpart partition %pOF (%pOF) missing reg property.\n",
					 master->name, pp,
					 mtd_node);
				goto ofpart_fail;
			} else {
				nr_parts--;
				continue;
			}
		}

		a_cells = of_n_addr_cells(pp);
		s_cells = of_n_size_cells(pp);
		if (!dedicated && s_cells == 0) {
			/*
			 * This is a ugly workaround to not create
			 * regression on devices that are still creating
			 * partitions as direct children of the nand controller.
			 * This can happen in case the nand controller node has
			 * #size-cells equal to 0 and the firmware (e.g.
			 * U-Boot) just add the partitions there assuming
			 * 32-bit addressing.
			 *
			 * If you get this warning your firmware and/or DTS
			 * should be really fixed.
			 *
			 * This is working only for devices smaller than 4GiB.
			 */
			pr_warn("%s: ofpart partition %pOF (%pOF) #size-cells is wrongly set to <0>, assuming <1> for parsing partitions.\n",
				master->name, pp, mtd_node);
			s_cells = 1;
		}
		if (len / 4 != a_cells + s_cells) {
			pr_debug("%s: ofpart partition %pOF (%pOF) error parsing reg property.\n",
				 master->name, pp,
				 mtd_node);
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

		if (of_property_read_bool(pp, "slc-mode"))
			parts[i].add_flags |= MTD_SLC_ON_MLC_EMULATION;

#ifdef CONFIG_ARCH_QCOM
		if (!strcmp(partname, "linux") || !strcmp(partname, "linux2")) {
			int offset = parts[i].offset;
			while ((offset + master->erasesize) < master->size) {
				size_t len;
				mtd_read(master, offset, sizeof(sb), &len, (void *)&sb);
				if (le32_to_cpu(sb.s_magic) == 0x23494255) {
					printk(KERN_EMERG "found ubi at %X, skipping scan\n", offset);
					break; // skip if ubifs has been found
				}
				if (le32_to_cpu(sb.s_magic) == SQUASHFS_MAGIC) {
					len = le64_to_cpu(sb.bytes_used);
					len += (offset & 0x000fffff);
					len += (master->erasesize - 1);
					len &= ~(master->erasesize - 1);
					len -= (offset & 0x000fffff);
					printk(KERN_EMERG "found squashfs at %X with len of %d bytes\n", offset, len);
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

	if (quirks && quirks->post_parse)
		quirks->post_parse(master, parts, nr_parts);

	*pparts = parts;
	return nr_parts;

ofpart_fail:
	pr_err("%s: error parsing ofpart partition %pOF (%pOF)\n",
	       master->name, pp, mtd_node);
	ret = -EINVAL;
ofpart_none:
	of_node_put(pp);
	kfree(parts);
	return ret;
}

static const struct of_device_id parse_ofpart_match_table[] = {
	/* Generic */
	{ .compatible = "fixed-partitions" },
	/* Customized */
	{ .compatible = "brcm,bcm4908-partitions", .data = &bcm4908_partitions_quirks, },
	{ .compatible = "linksys,ns-partitions", .data = &linksys_ns_partitions_quirks, },
	{},
};
MODULE_DEVICE_TABLE(of, parse_ofpart_match_table);

static struct mtd_part_parser ofpart_parser = {
	.parse_fn = parse_fixed_partitions,
	.name = "fixed-partitions",
	.of_match_table = parse_ofpart_match_table,
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

	pr_warn("Device tree uses obsolete partition map binding: %pOF\n", dp);

	nr_parts = plen / sizeof(part[0]);

	parts = kcalloc(nr_parts, sizeof(*parts), GFP_KERNEL);
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
MODULE_ALIAS("fixed-partitions");
MODULE_ALIAS("ofoldpart");
