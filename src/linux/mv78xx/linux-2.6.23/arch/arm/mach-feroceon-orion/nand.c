
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand.h>
#include <asm/io.h>
#include "ctrlEnv/bars/mvCpuIf.h"

extern MV_U32    boardGetDevCSNum(MV_32 devNum, MV_BOARD_DEV_CLASS devType);

static struct mtd_info *mv_mtd;
static unsigned long baseaddr;

#ifdef CONFIG_MTD_PARTITIONS
#define MV_NUM_OF_NAND_PARTS 3
static struct mtd_partition parts_info[] = {
	{ .name = "u-boot",
	  .offset = 0,
	  .size = 1 * 1024 * 1024 },
	{ .name = "uImage",
	  .offset = MTDPART_OFS_NXTBLK,
	  .size = 2 * 1024 * 1024 },
	{ .name = "root",
	  .offset = MTDPART_OFS_NXTBLK,
	  .size = MTDPART_SIZ_FULL },
};
static const char *part_probes[] __initdata = { "cmdlinepart", NULL };
#endif

static void board_hwcontrol(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	if (ctrl & NAND_CTRL_CHANGE) {        
		this->IO_ADDR_W = (void __iomem *)((unsigned long)this->IO_ADDR_W & ~3);
		ctrl &= ~NAND_CTRL_CHANGE;
		switch(ctrl) {
			case NAND_CTRL_CLE: 
				this->IO_ADDR_W = (void __iomem *)((unsigned long)this->IO_ADDR_W | 1); //x8=>1, x16=>2
				break;
			case NAND_CTRL_ALE:
				this->IO_ADDR_W = (void __iomem *)((unsigned long)this->IO_ADDR_W | 2); //x8=>2, x16=>4
				break;
		}				
	}
	if (cmd != NAND_CMD_NONE) 
	{
		writeb(cmd, this->IO_ADDR_W);		
	}	
}


int __init mv_nand_init(void)
{
	struct nand_chip *this;
	int err = 0;
	int num_of_parts = 0;
	const char *part_type = 0;
	struct mtd_partition *mtd_parts = 0;
	u32 physaddr;
	int nand_dev_num;
	MV_CPU_DEC_WIN addr_win;

	nand_dev_num =boardGetDevCSNum(0, BOARD_DEV_NAND_FLASH);
	if(-1 == nand_dev_num) {
		printk("NAND init: NAND device not found on board\n");
		err = -ENODEV;
		goto out;
	}

	if( MV_OK != mvCpuIfTargetWinGet((DEVICE_CS0 + nand_dev_num), &addr_win) ) {
                printk("Failed to init NAND MTD (boot-CS window %d err).\n", (DEVICE_CS0 + nand_dev_num));
		err = -ENODEV;
		goto out;
	}

	if(!addr_win.enable) {
                printk("Failed to init NAND MTD (boot-CS window disabled).\n" );
		err = -ENODEV;
		goto out;
	}
	physaddr = addr_win.addrWin.baseLow;

	mv_mtd = (struct mtd_info *)kmalloc(sizeof(struct mtd_info)+sizeof(struct nand_chip), GFP_KERNEL);
	if(!mv_mtd){
		printk("Failed to allocate NAND MTD structure\n");
		err = -ENOMEM;
		goto out;
	}

	memset((char*)mv_mtd,0,sizeof(struct mtd_info)+sizeof(struct nand_chip));

	baseaddr = (unsigned long)ioremap(physaddr, 1024);
	if(!baseaddr) {
		printk("Failed to remap NAND MTD\n");
		err = -EIO;
		goto out_mtd;
	}

	this = (struct nand_chip *)((char *)mv_mtd+sizeof(struct mtd_info));
	mv_mtd->priv = this;
	this->IO_ADDR_R = this->IO_ADDR_W = (void __iomem *)baseaddr;
	this->cmd_ctrl = board_hwcontrol;
	this->ecc.mode = NAND_ECC_SOFT;
	if(nand_scan(mv_mtd,1)) {
		err = -ENXIO;
		goto out_ior;
	}

#ifdef CONFIG_MTD_PARTITIONS
        mv_mtd->name = "nand_mtd";
        num_of_parts = parse_mtd_partitions(mv_mtd,part_probes,&mtd_parts,0);
        if(num_of_parts > 0)
                part_type = "command line";
        else
                num_of_parts = 0;
        if(num_of_parts == 0) {
                mtd_parts = parts_info;
                num_of_parts = MV_NUM_OF_NAND_PARTS;
                part_type = "static";
        }

	printk("Using %s partition definition\n", part_type);
	add_mtd_partitions(mv_mtd, mtd_parts, num_of_parts);
#endif
	goto out;

out_ior:
	iounmap((void *)baseaddr);
out_mtd:
	kfree(mv_mtd);
out:
	return err;
}

module_init(mv_nand_init);

#ifdef MODULE
static void __exit board_cleanup(void)
{
	nand_release(mv_mtd);
	iounmap((void*)baseaddr);
	kfree(mv_mtd);
}
module_exit(board_cleanup);
#endif

