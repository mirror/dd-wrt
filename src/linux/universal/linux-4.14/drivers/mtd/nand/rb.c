#include <linux/mtd/nand.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/module.h>
#include "../mtdcore.h"

static struct mtd_partition partition_info[] = {
    {
        name: "RouterBoard NAND Boot",
        offset: 0,
	size: 4 * 1024 * 1024
    },
    {
        name: "RouterBoard NAND Main",
	offset: MTDPART_OFS_NXTBLK,
	size: MTDPART_SIZ_FULL
    }
};

static struct mtd_partition partition_info_booter[] = {
    {
        name: "RouterBoard NAND Boot",
        offset: 256 * 1024,
	size: 4 * 1024 * 1024 - 256 * 1024,
    },
    {
        name: "RouterBoard NAND Main",
	offset: MTDPART_OFS_NXTBLK,
	size: MTDPART_SIZ_FULL,
    },
    {
        name: "RouterBoard NAND Booter",
	offset: 0,
	size: 256 * 1024,
    }
};

static struct mtd_info rmtd;
static int rmtd_valid = 0;

extern int nand_lock_and_callback(struct mtd_info *mtd,
				  int (*callback)(struct mtd_info *, void *),
				  void *priv);

int nand_custom_cmd(int (*cmd)(struct mtd_info *mtd, void *priv),
		    void *priv) {
	if (!rmtd_valid) return -ENODEV;
	return nand_lock_and_callback(&rmtd, cmd, priv);
}
EXPORT_SYMBOL(nand_custom_cmd);

#ifdef MIPSEL
static int nand_is_bad = 0;

static int check_nand(struct mtd_info *mtd, void *priv) {
	struct nand_chip *chip = mtd->priv;
	unsigned char ids[4];

	chip->cmdfunc(mtd, NAND_CMD_READID, 0x00, -1);
	ids[0] = chip->read_byte(mtd);
	ids[1] = chip->read_byte(mtd);
	ids[2] = chip->read_byte(mtd);
	ids[3] = chip->read_byte(mtd);

	nand_is_bad = (ids[0] == 0xad &&
		       ids[1] == 0xf1 &&
		       ids[2] == 0x80 &&
		       ids[3] == 0x1d) ? 1 : 0;
	return 0;
}

int is_nand_bad(void) {
	return nand_is_bad;
}
EXPORT_SYMBOL(is_nand_bad);

static int nand_supports_backup(struct mtd_info *mtd) {
	unsigned ofs;

	for (ofs = mtd->size / 2; ofs < mtd->size; ofs += mtd->erasesize) {
		struct mtd_oob_ops ops;
		uint32_t oobdata;

		ops.ooblen = 4;
		ops.oobbuf = (uint8_t *)&oobdata;
		ops.ooboffs = BACKUP_4xFF_OFFSET;
		ops.datbuf = NULL;
		ops.mode = MTD_OPS_RAW;

		if (mtd->block_isbad(mtd, ofs)) {
			continue;
		}
		if (mtd->read_oob(mtd, ofs, &ops)) {
			printk(KERN_INFO "nand backup disabled - "
			       "read error at block %u\n",
			       ofs / mtd->erasesize);
			return 0;
		}
		if (oobdata != 0xffffffff) {
			printk(KERN_INFO "nand backup disabled - "
			       "backup area not empty at block %u\n",
			       ofs / mtd->erasesize);
			return 0;
		}
	}

	printk(KERN_INFO "nand backup enabled\n");
	return 1;
}

int nand_enable_backup(struct mtd_info *mtd_part) {
	struct nand_chip *chip = rmtd.priv;
	if (!rmtd_valid) return 0;
	if (chip->backup_offset) return 0;
	if (!is_nand_bad()) return 0;
	if (!nand_supports_backup(&rmtd)) return 0;

	chip->backup_offset = rmtd.size / 2;
	partition_info[1].size = (chip->backup_offset -
				  partition_info[0].size);
	if (mtd_part && mtd_part != &rmtd) {
		// reduce mtd partition size
		mtd_part->size -= chip->backup_offset;
	}
	return 1;
}
EXPORT_SYMBOL(nand_enable_backup);
#endif

int rb_nand_probe(struct nand_chip *nand, int booter)
{
	memset(&rmtd, 0, sizeof(rmtd));

	nand->ecc.mode = NAND_ECC_SOFT;
	nand->chip_delay = 25;
	rmtd.priv = nand;

	if (nand_scan(&rmtd, 1) && nand_scan(&rmtd, 1)
	    && nand_scan(&rmtd, 1)  && nand_scan(&rmtd, 1)) {
		printk("RBxxx nand device not found\n");
		return -ENXIO;
	}
	rmtd_valid = 1;

#ifdef MIPSEL
	nand_custom_cmd(&check_nand, NULL);
	nand_enable_backup(NULL);
#endif

	if (booter) {
		add_mtd_partitions(&rmtd, partition_info_booter, 3);
	}
	else {
		add_mtd_partitions(&rmtd, partition_info, 2);
	}
	return 0;
}
