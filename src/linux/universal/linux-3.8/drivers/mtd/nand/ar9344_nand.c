#include <linux/init.h>
#include <linux/mtd/nand.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
//#include <asm/rb/rb400.h>

extern int rb_nand_probe(struct nand_chip *nand, int booter);
static void send_cmd(unsigned cmd);

static struct nand_chip rnand;

#define GPIO_BASE		0x18040000
#define   GPIO_S1_REG		    0x000c
#define   GPIO_S0_REG		    0x0010
#define     GPO_NAND_nCE		(1 << 14)
#define   RST_RESET_REG		   0x2001C
#define     NANDF_RESET			((1 << 14) | (1 << 12))
#define   RST_BOOTSTRAP_REG	   0x200B0
#define     REFCLK_40MHZ		(1 << 4)

#define NAND_FLASH_BASE		0x1b000000
#define   NF_COMMAND		     0x200
#define     NF_CMD_DMA			(1 << 6)
#define     NF_CMD_READ			(1 << 5)
#define   NF_CONTROL		     0x204
#define     NF_CTRL_CUSTOM_SIZE		(1 << 11)
#define     NF_CTRL_SMALL_BLOCK		(1 << 21)
#define   NF_STATUS		     0x208
#define   NF_ADDR0_0		     0x21c
#define   NF_ADDR0_1		     0x224
#define   NF_DMA_ADDR		     0x264
#define   NF_DMA_COUNT		     0x268
#define   NF_DMA_CTRL		     0x26c
#define     NF_DMA_READY		(1 << 0)
#define     NF_DMA_ERROR		(1 << 1)
#define   NF_MEM_CTRL		     0x280
#define   NF_DATA_SIZE		     0x284
#define   NF_RD_STATUS		     0x288
#define   NF_TIME_SEQ		     0x28c
#define   NF_TIMING_ASYNC	     0x290
#define   NF_TIMING_SYNC	     0x294
#define   NF_DMA_ADDR_OFS	     0x2a0
#define   NF_GEN_SEQ_CTRL	     0x2b4
#define     NSEQ_SMALL_PAGE_READ	0x30043

#define NAND(x)		(*(volatile unsigned *)((unsigned) nand_base + (x)))
#define CFG(x)		(*(volatile unsigned *)((unsigned) cfg_base + (x)))

static void __iomem *nand_base = 0;
static void __iomem *cfg_base = 0;

/* ------------------ AR9344 nand driver -------------------- */

static volatile uint8_t	*dma_buf = 0;
static dma_addr_t dma_addr = 0;
static unsigned dma_buf_ofs = 0;
static int small_block = 0;

static uint8_t ar9344_read_byte(struct mtd_info *mtd)
{
	unsigned idx = dma_buf_ofs ^ 3;
	++dma_buf_ofs;
	return dma_buf[idx];
}

static void ar9344_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	for (; len > 0; --len, ++buf) {
		*buf = ar9344_read_byte(mtd);
	}
}

static void ar9344_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	for (; len > 0; --len, ++buf) {
		dma_buf[dma_buf_ofs ^ 3] = *buf;
		++dma_buf_ofs;
	}
}

static void nand_prepare(void) {
	unsigned clk40mhz = CFG(RST_BOOTSTRAP_REG) & REFCLK_40MHZ;
	NAND(NF_TIMING_ASYNC) = clk40mhz ? 0x22 : 0x11;
	NAND(NF_TIMING_SYNC) = 0xf;
	NAND(NF_TIME_SEQ) = 0x7fff;

	NAND(NF_MEM_CTRL) = 0xff00;
	NAND(NF_CONTROL) = NF_CTRL_CUSTOM_SIZE;
	NAND(NF_DMA_ADDR_OFS) = 0;
	NAND(NF_GEN_SEQ_CTRL) = NSEQ_SMALL_PAGE_READ;
}

static void ar9344_select_chip(struct mtd_info *mtd, int chipnr) {
	CFG(chipnr >= 0 ? GPIO_S0_REG : GPIO_S1_REG) = GPO_NAND_nCE;
	ndelay(500);
}

static void ar9344_ctrl(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	if (cmd != NAND_CMD_NONE) {
		send_cmd((cmd & 0xff) << 8);
	}
}

static int wait_rdy(void) {
	unsigned long to = jiffies + HZ / 5;
	while(NAND(NF_STATUS) != 0xff) {
		if (time_after(jiffies, to)) {
			return 1;
		}
	}
	return 0;
}

static int wait_dma_rdy(void) {
	unsigned long to = jiffies + HZ / 5;
	while(!(NAND(NF_DMA_CTRL) & NF_DMA_READY)) {
		if (time_after(jiffies, to)) {
			return 1;
		}
	}
	return 0;
}

static void send_cmd(unsigned cmd) {
	NAND(NF_COMMAND) = cmd;
	NAND(NF_COMMAND);
	wait_rdy();
}

static unsigned read_status(void) {
	NAND(NF_COMMAND) = 0x07024;
	NAND(NF_COMMAND);			// flush
	wait_rdy();
	return NAND(NF_RD_STATUS);
}

static void nand_reset(void) {
	unsigned nf_ctrl = NAND(NF_CONTROL);
	unsigned rst = CFG(RST_RESET_REG);
	CFG(RST_RESET_REG) = rst | NANDF_RESET;
	CFG(RST_RESET_REG);
	udelay(100);
	CFG(RST_RESET_REG) = rst;
	CFG(RST_RESET_REG);
	udelay(100);
	nand_prepare();
	NAND(NF_CONTROL) = nf_ctrl;
}

static int do_rw_cmd(unsigned page, unsigned offset,
		     unsigned len, unsigned cmd) {
	NAND(NF_ADDR0_0) = offset | (page << 16);
	NAND(NF_ADDR0_1) = (page >> 16) & 0xf;
	if (small_block) offset &= 0xff;
	NAND(NF_DMA_ADDR) = offset + (unsigned)dma_addr;
	NAND(NF_DMA_COUNT) = len;
	NAND(NF_DATA_SIZE) = len;
	NAND(NF_DMA_CTRL) = (cmd & NF_CMD_READ) ? 0xcc : 0x8c;
	NAND(NF_COMMAND) = cmd;
	NAND(NF_COMMAND);
	return wait_rdy() || wait_dma_rdy();
}

static void nand_rw_buf(unsigned page, unsigned offset,
			unsigned len, unsigned cmd, const char *action) {
	BUG_ON(len & 3);
	while (do_rw_cmd(page, offset, len, cmd)) {
		printk(KERN_WARNING "NAND %s: DMA reset at page %x\n",
		       action, page);
		ar9344_select_chip(NULL, -1);
		nand_reset();
		ar9344_select_chip(NULL, 0);
	}
}

static void read_buf_from_nand(unsigned page, unsigned offset,
			       unsigned len, unsigned cmd) {
	nand_rw_buf(page, offset, len, cmd, "read");
}

static int write_buf_to_nand(unsigned page, unsigned offset, unsigned len) {
	unsigned status;
	dma_buf[0];				// flush writes

	nand_rw_buf(page, offset, len, 0x10804c, "write");

	status = read_status();
	if ((status & 0xc7) != 0xc0) {
		printk(KERN_WARNING "write failure: "
		       "page %x offset %x len %x status %x\n",
		       page, offset, len, status);
		return -EIO;
	}
	return 0;
}

static int ar9344_write_page(struct mtd_info *mtd, struct nand_chip *chip,
			    const uint8_t *buf, int oob_required, int page, int cached, int raw)
{
	dma_buf_ofs = 0;

	if (unlikely(raw))
		chip->ecc.write_page_raw(mtd, chip, buf, oob_required);
	else
		chip->ecc.write_page(mtd, chip, buf, oob_required);

//	printk("ar9344_write_page %x len %x\n", page, dma_buf_ofs);
	if (small_block) {
		send_cmd(0);
		return write_buf_to_nand(page >> 8, (page & 0xff) << 8,
					 dma_buf_ofs);
	}
	return write_buf_to_nand(page, 0, dma_buf_ofs);
}

static int ar9344_write_oob(struct mtd_info *mtd, struct nand_chip *chip,
			   int page)
{
	if (small_block) {
		send_cmd(0x5000);
		dma_buf_ofs = 0;
		ar9344_write_buf(mtd, chip->oob_poi, mtd->oobsize);
		return write_buf_to_nand(page >> 8, (page & 0xff) << 8,
					 mtd->oobsize);
	}
	dma_buf_ofs = mtd->writesize;
	ar9344_write_buf(mtd, chip->oob_poi, mtd->oobsize);
//	printk("ar9344_write_oob page %x len %x\n", page, dma_buf_ofs);
	return write_buf_to_nand(page, mtd->writesize, mtd->oobsize);
}

static void ar9344_command(struct mtd_info *mtd, unsigned int command,
			   int column, int page)
{
	register struct nand_chip *chip = mtd->priv;
	static int ctrl_init_done = 0;

	if (command == NAND_CMD_RESET) {
		send_cmd(0xff00);
		return;
	}
	if (command == NAND_CMD_READID) {
		read_buf_from_nand(0, 0, 16, 0x9061);
		dma_buf_ofs = 0;
		return;
	}
	if (!ctrl_init_done) {
		unsigned ctrl = NF_CTRL_CUSTOM_SIZE;
		if (chip->page_shift <= 9) {
			ctrl |= NF_CTRL_SMALL_BLOCK;
			small_block = 1;
		}
		ctrl |= (chip->page_shift - 8) << 8;
		ctrl |= (chip->phys_erase_shift - chip->page_shift - 5) << 6;
		ctrl |= ((chip->page_shift + 6) / 8 + 
			 (chip->chip_shift - chip->page_shift + 7) / 8);
		NAND(NF_CONTROL) = ctrl;
		ctrl_init_done = 1;
		printk("AR9344 NAND ctrl %x\n", ctrl);
	}
	if (command == NAND_CMD_STATUS) {
		dma_buf_ofs = 0;
		dma_buf[3] = read_status();
		return;
	}
	if (command == NAND_CMD_ERASE1) {
		unsigned status;
//		printk("erase page 0x%08x\n", page);
		if (small_block) ++NAND(NF_CONTROL);
		NAND(NF_ADDR0_0) = page << 16;
		NAND(NF_ADDR0_1) = page >> 16;
		NAND(NF_ADDR0_1);
		send_cmd(0xd0600e);
		if (small_block) --NAND(NF_CONTROL);
		status = read_status();
		if ((status & 0xc7) != 0xc0) {
			printk("erase failure: page %x status %x\n",
			       page, status);
		}
		return;
	}
	if (command == NAND_CMD_ERASE2) {
		/* erase is already done */
		return;
	}
	if (command == NAND_CMD_READOOB) {
//		printk("read_oob page %x ofs %x\n", page, column);
		if (small_block) {
			read_buf_from_nand(page >> 8, (page & 0xff) << 8,
					   mtd->oobsize, 0x5072);
			dma_buf_ofs = column;
			return;
		}
		read_buf_from_nand(page, mtd->writesize, mtd->oobsize,
				   0x30006a);
		dma_buf_ofs = mtd->writesize + column;
		return;
	}
	if (command == NAND_CMD_READ0) {
//		printk("read_page page %x ofs %x\n", page, i);
		if (small_block) {
			read_buf_from_nand(page >> 8, (page & 0xff) << 8,
					   mtd->writesize + mtd->oobsize,
					   0x72);
			dma_buf_ofs = column;
			return;
		}
		read_buf_from_nand(page, 0, mtd->writesize + mtd->oobsize,
				   0x30006a);
		dma_buf_ofs = column;
		return;
	}

	printk(KERN_ERR "ar9344: unknown command %x\n", command);
}

static int ar9344_wait_rdy(struct mtd_info *mtd, struct nand_chip *chip)
{
	/* command is already finished, nothing to wait for */
	return 0;
}

static int ar9344_nand_probe(struct platform_device *pdev)
{
	printk("AR9344 nand\n");
	dma_buf = (unsigned char *)dma_alloc_coherent(&pdev->dev, 4096,
						      &dma_addr, GFP_KERNEL);
	if (dma_buf == NULL) {
		printk("AR9344 nand - dma alloc failed\n");
		return -ENOMEM;
	}
	memset(&rnand, 0, sizeof(rnand));

	rnand.read_byte = ar9344_read_byte;
	rnand.read_buf = ar9344_read_buf;
	rnand.write_buf = ar9344_write_buf;

	rnand.select_chip = ar9344_select_chip;
	rnand.cmd_ctrl = ar9344_ctrl;
	rnand.cmdfunc = ar9344_command;
	rnand.waitfunc = ar9344_wait_rdy;
	rnand.write_page = ar9344_write_page;
	rnand.ecc.write_oob = ar9344_write_oob;

	nand_prepare();
	return rb_nand_probe(&rnand, 1);
}

static struct platform_driver ar9344_nand_driver = {
	.probe	= ar9344_nand_probe,
	.driver	= {
		.name = "ar9344-nand",
	},
};

/* ------------------ common init/exit code -------------------- */

static int __init ar9344_nand_init(void)
{
	nand_base = ioremap_nocache(NAND_FLASH_BASE, PAGE_SIZE);
	cfg_base = ioremap_nocache(GPIO_BASE, 0x30000);
	if (!nand_base || !cfg_base)
		return -ENOMEM;

	return platform_driver_register(&ar9344_nand_driver);
}

static void __exit ar9344_nand_exit(void)
{
	if (nand_base) iounmap(nand_base);
	if (cfg_base) iounmap(cfg_base);

	platform_driver_unregister(&ar9344_nand_driver);
}
							     
module_init(ar9344_nand_init);
module_exit(ar9344_nand_exit);
