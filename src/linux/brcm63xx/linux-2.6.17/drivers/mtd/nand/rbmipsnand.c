#include <linux/init.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/bootinfo.h>
//#include <asm/rb/rb100.h>

#define IDT434_REG_BASE ((volatile void *) KSEG1ADDR(0x18000000))

#define SMEM1(x) (*((volatile unsigned char *) (KSEG1ADDR(SMEM1_BASE) + x)))

#define GPIOF 0x050000
#define GPIOC 0x050004
#define GPIOD 0x050008

#define GPIO_RDY (1 << 0x08)
#define GPIO_WPX (1 << 0x09)
#define GPIO_ALE (1 << 0x0a)
#define GPIO_CLE (1 << 0x0b)

#define NAND_RW_REG	0x0	//data register
#define NAND_SET_CEn	0x1	//CE# low
#define NAND_CLR_CEn	0x2	//CE# high
#define NAND_CLR_CLE	0x3	//CLE low
#define NAND_SET_CLE	0x4	//CLE high
#define NAND_CLR_ALE	0x5	//ALE low
#define NAND_SET_ALE	0x6	//ALE high
#define NAND_SET_SPn	0x7	//SP# low (use spare area)
#define NAND_CLR_SPn	0x8	//SP# high (do not use spare area)
#define NAND_SET_WPn	0x9	//WP# low
#define NAND_CLR_WPn	0xA	//WP# high
#define NAND_STS_REG	0xB	//Status register

#define DEV2BASE 0x010020

#define LO_WPX   (1 << 0)
#define LO_ALE   (1 << 1)
#define LO_CLE   (1 << 2)
#define LO_CEX   (1 << 3)
#define LO_FOFF  (1 << 5)
#define LO_SPICS (1 << 6)
#define LO_ULED  (1 << 7)

#define MEM32(x) *((volatile unsigned *) (x))


extern void changeLatchU5(unsigned char orMask, unsigned char nandMask);

static int rb500_dev_ready(struct mtd_info *mtd) {
    return MEM32(IDT434_REG_BASE + GPIOD) & GPIO_RDY;
}
/*
static int rb100_dev_ready(struct mtd_info *mtd) {
    return SMEM1(NAND_STS_REG) & 0x80;
}
*/
static unsigned long iflags = 0;
static int ioff = 0;
/*
static void rbmips_hwcontrol400(struct mtd_info *mtd, int cmd) {
    switch (cmd) {
    case NAND_CTL_SETCLE:
	MEM32(IDT434_REG_BASE + GPIOD) |= GPIO_CLE;
        break;
    case NAND_CTL_CLRCLE:
	MEM32(IDT434_REG_BASE + GPIOD) &= ~GPIO_CLE;
        break;
    case NAND_CTL_SETALE:
	MEM32(IDT434_REG_BASE + GPIOD) |= GPIO_ALE;
        break;
    case NAND_CTL_CLRALE:
	MEM32(IDT434_REG_BASE + GPIOD) &= ~GPIO_ALE;
        break;
    default:
	break;
    }
}
*/
static void rbmips_hwcontrol500(struct mtd_info *mtd, int cmd) {
    switch (cmd) {
    case NAND_CTL_SETCLE:
	changeLatchU5(LO_CLE, 0);
        break;
    case NAND_CTL_CLRCLE:
	changeLatchU5(0, LO_CLE);
        break;
    case NAND_CTL_SETALE:
	changeLatchU5(LO_ALE, 0);
        break;
    case NAND_CTL_CLRALE:
	changeLatchU5(0, LO_ALE);
        break;
    default:
        break;
    }
}
/*
static void rbmips_hwcontrol100(struct mtd_info *mtd, int cmd){
    switch(cmd){
    case NAND_CTL_SETCLE: 
	SMEM1(NAND_SET_CLE) = 0x01; 
	break;
    case NAND_CTL_CLRCLE: 
	SMEM1(NAND_CLR_CLE) = 0x01; 
	break;
    case NAND_CTL_SETALE: 
	SMEM1(NAND_SET_ALE) = 0x01; 
	break;
    case NAND_CTL_CLRALE: 
	SMEM1(NAND_CLR_ALE) = 0x01; 
	break;
    case NAND_CTL_SETNCE: 
	SMEM1(NAND_SET_CEn) = 0x01; 
	break;
    case NAND_CTL_CLRNCE: 
	SMEM1(NAND_CLR_CEn) = 0x01; 
	break;
    }
}
*/
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

static struct mtd_info rmtd;
static struct nand_chip rnand;

static unsigned init_ok = 0;

unsigned get_rbnand_block_size(void) {
	if (init_ok) return rmtd.oobblock; else return 0;
}

EXPORT_SYMBOL(get_rbnand_block_size);

int __init rbmips_init(void) {
	memset(&rmtd, 0, sizeof(rmtd));
	memset(&rnand, 0, sizeof(rnand));
/*
	if (is_rb500()) {
		if (is_rb400()) {
			printk("RB400 nand\n");
			MEM32(IDT434_REG_BASE + GPIOD) |= GPIO_WPX;
			MEM32(IDT434_REG_BASE + GPIOD) &= ~GPIO_CLE;
			MEM32(IDT434_REG_BASE + GPIOD) &= ~GPIO_ALE;
			rnand.hwcontrol = rbmips_hwcontrol400;
		} else {
*/		
			printk("RB500 nand\n");
			changeLatchU5(LO_WPX | LO_FOFF | LO_CEX,
				      LO_ULED | LO_ALE | LO_CLE);
			rnand.hwcontrol = rbmips_hwcontrol500;
//		}
    
		rnand.dev_ready = rb500_dev_ready;
		rnand.IO_ADDR_W = (unsigned char *)
			KSEG1ADDR(MEM32(IDT434_REG_BASE + DEV2BASE));
		rnand.IO_ADDR_R = rnand.IO_ADDR_W;
/*	} else if (is_rb100()) {
		printk("RB100 nand\n");
		MEM32(0xB2000064) = 0x100;
		MEM32(0xB2000008) = 0x1;
		SMEM1(NAND_SET_SPn) = 0x01; 
		SMEM1(NAND_CLR_WPn) = 0x01; 
		rnand.IO_ADDR_R = (unsigned char *)KSEG1ADDR(SMEM1_BASE);
		rnand.IO_ADDR_W = rnand.IO_ADDR_R;
		rnand.hwcontrol = rbmips_hwcontrol100;
		rnand.dev_ready = rb100_dev_ready;
	}
*/
	rnand.eccmode = NAND_ECC_SOFT;
	rnand.chip_delay = 25;
	rnand.options |= NAND_NO_AUTOINCR;
	rmtd.priv = &rnand;

	if (nand_scan(&rmtd, 1) && nand_scan(&rmtd, 1)
	    && nand_scan(&rmtd, 1)  && nand_scan(&rmtd, 1)) {
		printk("RBxxx nand device not found");
		return -ENXIO;
	}

	add_mtd_partitions(&rmtd, partition_info, 2);
	init_ok = 1;
	return 0;
}

module_init(rbmips_init);









