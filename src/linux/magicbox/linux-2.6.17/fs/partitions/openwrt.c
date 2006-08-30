/*
 *  fs/partitions/openwrt.c
 *
 *  Code extracted from drivers/block/genhd.c
 *  and fs/partitions/msdos.c
 *
 *  Copyright (C) 2006       Felix Fietkau <nbd@openwrt.org>
 *  Copyright (C) 1991-1998  Linus Torvalds
 *
 *  Thanks to Branko Lankester, lankeste@fwi.uva.nl, who found a bug
 *  in the early extended-partition checks and added DM partitions
 *
 *  Support for DiskManager v6.0x added by Mark Lord,
 *  with information provided by OnTrack.  This now works for linux fdisk
 *  and LILO, as well as loadlin and bootln.  Note that disks other than
 *  /dev/hda *must* have a "DOS" type 0x51 partition in the first slot (hda1).
 *
 *  More flexible handling of extended partitions - aeb, 950831
 *
 *  Check partition table on IDE disks for common CHS translations
 *
 *  Re-organised Feb 1998 Russell King
 */

#include <linux/config.h>

#include "check.h"
#include "openwrt.h"

/*
 * Many architectures don't like unaligned accesses, while
 * the nr_sects and start_sect partition table entries are
 * at a 2 (mod 4) address.
 */
#include <asm/unaligned.h>

#define SYS_IND(p)	(get_unaligned(&p->sys_ind))
#define NR_SECTS(p)	({ __typeof__(p->nr_sects) __a =	\
				get_unaligned(&p->nr_sects);	\
				le32_to_cpu(__a); \
			})

#define START_SECT(p)	({ __typeof__(p->start_sect) __a =	\
				get_unaligned(&p->start_sect);	\
				le32_to_cpu(__a); \
			})

static inline int is_extended_partition(struct partition *p)
{
	return (SYS_IND(p) == DOS_EXTENDED_PARTITION ||
		SYS_IND(p) == WIN98_EXTENDED_PARTITION ||
		SYS_IND(p) == LINUX_EXTENDED_PARTITION);
}

#define MSDOS_LABEL_MAGIC1	0x55
#define MSDOS_LABEL_MAGIC2	0xAA

static inline int
msdos_magic_present(unsigned char *p)
{
	return (p[0] == MSDOS_LABEL_MAGIC1 && p[1] == MSDOS_LABEL_MAGIC2);
}

static inline int
openwrt_magic_present(unsigned char *p)
{
	return (p[0] == 'O' && 
			p[1] == 'W' &&
			p[2] == 'R' &&
			p[3] == 'T');
}

/*
 * Create devices for each logical partition in an extended partition.
 * The logical partitions form a linked list, with each entry being
 * a partition table with two entries.  The first entry
 * is the real data partition (with a start relative to the partition
 * table start).  The second is a pointer to the next logical partition
 * (with a start relative to the entire extended partition).
 * We do not create a Linux partition for the partition tables, but
 * only for the actual data partitions.
 */

static void
parse_extended(struct parsed_partitions *state, struct block_device *bdev,
			u32 first_sector, u32 first_size)
{
	struct partition *p;
	Sector sect;
	unsigned char *data;
	u32 this_sector, this_size;
	int sector_size = bdev_hardsect_size(bdev) / 512;
	int loopct = 0;		/* number of links followed
				   without finding a data partition */
	int i;

	this_sector = first_sector;
	this_size = first_size;

	while (1) {
		if (++loopct > 100)
			return;
		if (state->next == state->limit)
			return;
		data = read_dev_sector(bdev, this_sector, &sect);
		if (!data)
			return;

		if (!msdos_magic_present(data + 510))
			goto done; 

		p = (struct partition *) (data + 0x1be);

		/*
		 * Usually, the first entry is the real data partition,
		 * the 2nd entry is the next extended partition, or empty,
		 * and the 3rd and 4th entries are unused.
		 * However, DRDOS sometimes has the extended partition as
		 * the first entry (when the data partition is empty),
		 * and OS/2 seems to use all four entries.
		 */

		/* 
		 * First process the data partition(s)
		 */
		for (i=0; i<4; i++, p++) {
			u32 offs, size, next;
			if (!NR_SECTS(p) || is_extended_partition(p))
				continue;

			/* Check the 3rd and 4th entries -
			   these sometimes contain random garbage */
			offs = START_SECT(p)*sector_size;
			size = NR_SECTS(p)*sector_size;
			next = this_sector + offs;
			if (i >= 2) {
				if (offs + size > this_size)
					continue;
				if (next < first_sector)
					continue;
				if (next + size > first_sector + first_size)
					continue;
			}

			put_partition(state, state->next, next, size);
			if (SYS_IND(p) == LINUX_RAID_PARTITION)
				state->parts[state->next].flags = 1;
			loopct = 0;
			if (++state->next == state->limit)
				goto done;
		}
		/*
		 * Next, process the (first) extended partition, if present.
		 * (So far, there seems to be no reason to make
		 *  parse_extended()  recursive and allow a tree
		 *  of extended partitions.)
		 * It should be a link to the next logical partition.
		 */
		p -= 4;
		for (i=0; i<4; i++, p++)
			if (NR_SECTS(p) && is_extended_partition(p))
				break;
		if (i == 4)
			goto done;	 /* nothing left to do */

		this_sector = first_sector + START_SECT(p) * sector_size;
		this_size = NR_SECTS(p) * sector_size;
		put_dev_sector(sect);
	}
done:
	put_dev_sector(sect);
}


int openwrt_partition(struct parsed_partitions *state, struct block_device *bdev)
{
	int sector_size = bdev_hardsect_size(bdev) / 512;
	Sector sect;
	unsigned char *data;
	struct partition *p;
	int slot;
	u32 last_block = 0;
	u32 size = 0;
	int firstfree = 5;

	data = read_dev_sector(bdev, 0, &sect);
	if (!data)
		return -1;
	if (!openwrt_magic_present(data)) {
		printk("No OpenWrt partition table detected\n");	
		put_dev_sector(sect);
		return 0;
	}

	/*
	 * Now that the 55aa signature is present, this is probably
	 * either the boot sector of a FAT filesystem or a DOS-type
	 * partition table. Reject this in case the boot indicator
	 * is not 0 or 0x80.
	 */
	p = (struct partition *) (data + 0x1be);
	for (slot = 1; slot <= 4; slot++, p++) {
		if (p->boot_ind != 0 && p->boot_ind != 0x80) {
			put_dev_sector(sect);
			return 0;
		}
	}

	p = (struct partition *) (data + 0x1be);

	/*
	 * Look for partitions in two passes:
	 * First find the primary and DOS-type extended partitions.
	 */

	state->next = 6;
	for (slot = 1 ; slot <= 4 ; slot++, p++) {
		u32 start = START_SECT(p)*sector_size;
		size = NR_SECTS(p)*sector_size;
		if (!size) {
			if (firstfree > slot)
				firstfree = slot;

			continue;
		}
		if (is_extended_partition(p)) {
			/* prevent someone doing mkfs or mkswap on an
			   extended partition, but leave room for LILO */
			last_block = start + size;
			put_partition(state, slot, start, size == 1 ? 1 : 2);
			printk(" <");
			parse_extended(state, bdev, start, size);
			printk(" >");
			continue;
		}
		if ((start + size) > get_capacity(bdev->bd_disk))
			size = get_capacity(bdev->bd_disk) - start;
		last_block = start + size;
		put_partition(state, slot, start, size);
	}
	if (last_block + 1024 < (size = get_capacity(bdev->bd_disk)))
		put_partition(state, firstfree, last_block, size - last_block);

	printk("\n");

	put_dev_sector(sect);
	return 1;
}

