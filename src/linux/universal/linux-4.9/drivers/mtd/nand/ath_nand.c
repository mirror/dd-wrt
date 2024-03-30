/*
 * linux/drivers/mtd/nand/ath_nand.c
 * vim: tabstop=8 : noexpandtab
 * Derived from alauda.c
 */
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/bitops.h>
#include <linux/jiffies.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>
#include <linux/magic.h>
#include <asm/mach-ar71xx/ar71xx.h>

#include <asm/cacheflush.h>
#include "../mtdcore.h"
#include "934x.h"

typedef unsigned int ath_reg_t;

#define ath_reg_rd(_phys)	(*(volatile ath_reg_t *)KSEG1ADDR(_phys))

#define ath_reg_wr_nf(_phys, _val) \
	((*(volatile ath_reg_t *)KSEG1ADDR(_phys)) = (_val))

#define ath_reg_wr(_phys, _val) do {	\
	ath_reg_wr_nf(_phys, _val);	\
	ath_reg_rd(_phys);		\
} while(0)

#define ath_reg_rmw_set(_reg, _mask)	do {			\
	ath_reg_wr((_reg), (ath_reg_rd((_reg)) | (_mask)));	\
	ath_reg_rd((_reg));					\
} while(0)

#define ath_reg_rmw_clear(_reg, _mask) do {			\
	ath_reg_wr((_reg), (ath_reg_rd((_reg)) & ~(_mask)));	\
	ath_reg_rd((_reg));					\
} while(0)


#define DRV_NAME	"ath-nand"
#define DRV_VERSION	"0.1"
#define DRV_AUTHOR	"Atheros"
#define DRV_DESC	"Atheros on-chip NAND FLash Controller Driver"

static int ATH_NAND_FLASH_BASE=0x1b000000;

#define ATH_NF_COMMAND		(ATH_NAND_FLASH_BASE + 0x200u)
#define ATH_NF_CTRL		(ATH_NAND_FLASH_BASE + 0x204u)
#define ATH_NF_STATUS		(ATH_NAND_FLASH_BASE + 0x208u)
#define ATH_NF_INT_MASK		(ATH_NAND_FLASH_BASE + 0x20cu)
#define ATH_NF_INT_STATUS	(ATH_NAND_FLASH_BASE + 0x210u)
#define ATH_NF_ECC_CTRL		(ATH_NAND_FLASH_BASE + 0x214u)
#define ATH_NF_ECC_OFFSET	(ATH_NAND_FLASH_BASE + 0x218u)
#define ATH_NF_ADDR0_0		(ATH_NAND_FLASH_BASE + 0x21cu)
#define ATH_NF_ADDR1_0		(ATH_NAND_FLASH_BASE + 0x220u)
#define ATH_NF_ADDR0_1		(ATH_NAND_FLASH_BASE + 0x224u)
#define ATH_NF_ADDR1_1		(ATH_NAND_FLASH_BASE + 0x228u)
#define ATH_NF_SPARE_SIZE	(ATH_NAND_FLASH_BASE + 0x230u)
#define ATH_NF_PROTECT		(ATH_NAND_FLASH_BASE + 0x238u)
#define ATH_NF_LOOKUP_EN	(ATH_NAND_FLASH_BASE + 0x240u)
#define ATH_NF_LOOKUP0		(ATH_NAND_FLASH_BASE + 0x244u)
#define ATH_NF_LOOKUP1		(ATH_NAND_FLASH_BASE + 0x248u)
#define ATH_NF_LOOKUP2		(ATH_NAND_FLASH_BASE + 0x24cu)
#define ATH_NF_LOOKUP3		(ATH_NAND_FLASH_BASE + 0x250u)
#define ATH_NF_LOOKUP4		(ATH_NAND_FLASH_BASE + 0x254u)
#define ATH_NF_LOOKUP5		(ATH_NAND_FLASH_BASE + 0x258u)
#define ATH_NF_LOOKUP6		(ATH_NAND_FLASH_BASE + 0x25cu)
#define ATH_NF_LOOKUP7		(ATH_NAND_FLASH_BASE + 0x260u)
#define ATH_NF_DMA_ADDR		(ATH_NAND_FLASH_BASE + 0x264u)
#define ATH_NF_DMA_COUNT	(ATH_NAND_FLASH_BASE + 0x268u)
#define ATH_NF_DMA_CTRL		(ATH_NAND_FLASH_BASE + 0x26cu)
#define ATH_NF_MEM_CTRL		(ATH_NAND_FLASH_BASE + 0x280u)
#define ATH_NF_PG_SIZE		(ATH_NAND_FLASH_BASE + 0x284u)
#define ATH_NF_RD_STATUS	(ATH_NAND_FLASH_BASE + 0x288u)
#define ATH_NF_TIME_SEQ		(ATH_NAND_FLASH_BASE + 0x28cu)
#define ATH_NF_TIMINGS_ASYN	(ATH_NAND_FLASH_BASE + 0x290u)
#define ATH_NF_TIMINGS_SYN	(ATH_NAND_FLASH_BASE + 0x294u)
#define ATH_NF_FIFO_DATA	(ATH_NAND_FLASH_BASE + 0x298u)
#define ATH_NF_TIME_MODE	(ATH_NAND_FLASH_BASE + 0x29cu)
#define ATH_NF_DMA_ADDR_OFFSET	(ATH_NAND_FLASH_BASE + 0x2a0u)
#define ATH_NF_FIFO_INIT	(ATH_NAND_FLASH_BASE + 0x2b0u)
#define ATH_NF_GENERIC_SEQ_CTRL	(ATH_NAND_FLASH_BASE + 0x2b4u)

#define ATH_NF_TIMING_ASYN	0x11
#define ATH_NF_STATUS_OK	0x40	//0xc0
#define ATH_NF_RD_STATUS_MASK	0x47	//0xc7

#define ATH_NF_CTRL_SMALL_BLOCK_EN	(1 << 21)

#define ATH_NF_CTRL_ADDR_CYCLE1_0	(0 << 18)
#define ATH_NF_CTRL_ADDR_CYCLE1_1	(1 << 18)
#define ATH_NF_CTRL_ADDR_CYCLE1_2	(2 << 18)
#define ATH_NF_CTRL_ADDR_CYCLE1_3	(3 << 18)
#define ATH_NF_CTRL_ADDR_CYCLE1_4	(4 << 18)
#define ATH_NF_CTRL_ADDR_CYCLE1_5	(5 << 18)

#define ATH_NF_CTRL_ADDR1_AUTO_INC_EN	(1 << 17)
#define ATH_NF_CTRL_ADDR0_AUTO_INC_EN	(1 << 16)
#define ATH_NF_CTRL_WORK_MODE_SYNC	(1 << 15)
#define ATH_NF_CTRL_PROT_EN		(1 << 14)
#define ATH_NF_CTRL_LOOKUP_EN		(1 << 13)
#define ATH_NF_CTRL_IO_WIDTH_16BIT	(1 << 12)
#define ATH_NF_CTRL_CUSTOM_SIZE_EN	(1 << 11)

#define ATH_NF_CTRL_PAGE_SIZE_256	(0 <<  8)	/* bytes */
#define ATH_NF_CTRL_PAGE_SIZE_512	(1 <<  8)
#define ATH_NF_CTRL_PAGE_SIZE_1024	(2 <<  8)
#define ATH_NF_CTRL_PAGE_SIZE_2048	(3 <<  8)
#define ATH_NF_CTRL_PAGE_SIZE_4096	(4 <<  8)
#define ATH_NF_CTRL_PAGE_SIZE_8192	(5 <<  8)
#define ATH_NF_CTRL_PAGE_SIZE_16384	(6 <<  8)
#define ATH_NF_CTRL_PAGE_SIZE_0		(7 <<  8)

#define ATH_NF_CTRL_BLOCK_SIZE_32	(0 <<  6)	/* pages */
#define ATH_NF_CTRL_BLOCK_SIZE_64	(1 <<  6)
#define ATH_NF_CTRL_BLOCK_SIZE_128	(2 <<  6)
#define ATH_NF_CTRL_BLOCK_SIZE_256	(3 <<  6)

#define ATH_NF_CTRL_ECC_EN		(1 <<  5)
#define ATH_NF_CTRL_INT_EN		(1 <<  4)
#define ATH_NF_CTRL_SPARE_EN		(1 <<  3)

#define ATH_NF_CTRL_ADDR_CYCLE0_0	(0 <<  0)
#define ATH_NF_CTRL_ADDR_CYCLE0_1	(1 <<  0)
#define ATH_NF_CTRL_ADDR_CYCLE0_2	(2 <<  0)
#define ATH_NF_CTRL_ADDR_CYCLE0_3	(3 <<  0)
#define ATH_NF_CTRL_ADDR_CYCLE0_4	(4 <<  0)
#define ATH_NF_CTRL_ADDR_CYCLE0_5	(5 <<  0)
#define ATH_NF_CTRL_ADDR_CYCLE0(c)	((c) << 0)


#define ATH_NF_DMA_CTRL_DMA_START	(1 << 7)
#define ATH_NF_DMA_CTRL_DMA_DIR_WRITE	(0 << 6)
#define ATH_NF_DMA_CTRL_DMA_DIR_READ	(1 << 6)
#define ATH_NF_DMA_CTRL_DMA_MODE_SG	(1 << 5)
/*
 * 000 - incrementing precise burst of precisely four transfers
 * 001 - stream burst (address const)
 * 010 - single transfer (address increment)
 * 011 - burst of unspecified length (address increment)
 * 100 - incrementing precise burst of precisely eight transfers
 * 101 - incrementing precise burst of precisely sixteen transfers
 */
#define ATH_NF_DMA_CTRL_DMA_BURST_0	(0 << 2)
#define ATH_NF_DMA_CTRL_DMA_BURST_1	(1 << 2)
#define ATH_NF_DMA_CTRL_DMA_BURST_2	(2 << 2)
#define ATH_NF_DMA_CTRL_DMA_BURST_3	(3 << 2)
#define ATH_NF_DMA_CTRL_DMA_BURST_4	(4 << 2)
#define ATH_NF_DMA_CTRL_DMA_BURST_5	(5 << 2)
#define ATH_NF_DMA_CTRL_ERR_FLAG	(1 << 1)
#define ATH_NF_DMA_CTRL_DMA_READY	(1 << 0)

#define ATH_NF_ECC_CTRL_ERR_THRESH(x)	((x << 8) & (0x1fu << 8))
#define ATH_NF_ECC_CTRL_ECC_CAP(x)	((x << 5) & (0x07u << 5))
#define ATH_NF_ECC_CTRL_ECC_2_BITS	ATH_NF_ECC_CTRL_ECC_CAP(0)
#define ATH_NF_ECC_CTRL_ECC_4_BITS	ATH_NF_ECC_CTRL_ECC_CAP(1)
#define ATH_NF_ECC_CTRL_ECC_6_BITS	ATH_NF_ECC_CTRL_ECC_CAP(2)
#define ATH_NF_ECC_CTRL_ECC_8_BITS	ATH_NF_ECC_CTRL_ECC_CAP(3)
#define ATH_NF_ECC_CTRL_ECC_10_BITS	ATH_NF_ECC_CTRL_ECC_CAP(4)
#define ATH_NF_ECC_CTRL_ECC_12_BITS	ATH_NF_ECC_CTRL_ECC_CAP(5)
#define ATH_NF_ECC_CTRL_ECC_14_BITS	ATH_NF_ECC_CTRL_ECC_CAP(6)
#define ATH_NF_ECC_CTRL_ECC_16_BITS	ATH_NF_ECC_CTRL_ECC_CAP(7)

#define ATH_NF_ECC_CTRL_ERR_OVER	(1 << 2)
#define ATH_NF_ECC_CTRL_ERR_UNCORR	(1 << 1)
#define ATH_NF_ECC_CTRL_ERR_CORR	(1 << 0)
#	define ATH_NF_ECC_ERROR		(ATH_NF_ECC_CTRL_ERR_UNCORR | \
					 ATH_NF_ECC_CTRL_ERR_OVER)

#define ATH_NF_CMD_END_INT		(1 << 1)

#define ATH_NF_HW_ECC		1
#define ATH_NF_STATUS_RETRY	1000

#define ath_nand_get_cmd_end_status(void)	\
	(ath_reg_rd(ATH_NF_INT_STATUS) & ATH_NF_CMD_END_INT)

#define ath_nand_clear_int_status()	ath_reg_wr(ATH_NF_INT_STATUS, 0)

#define ATH_NAND_BLK_DONT_KNOW	0x0
#define ATH_NAND_BLK_GOOD	0x1
#define ATH_NAND_BLK_BAD	0x2
#define ATH_NAND_BLK_ERASED	0x3

/*
 * Note: The byte positions might not match the spec.
 * It is to handle the endianness issues.
 */
#define ONFI_NUM_ADDR_CYCLES	102	/* see note */
#define ONFI_DEV_DESC		32
#define ONFI_DEV_DESC_SZ	32
#define ONFI_PAGE_SIZE		80
#define ONFI_SPARE_SIZE		86	/* see note */
#define ONFI_PAGES_PER_BLOCK	92
#define ONFI_BLOCKS_PER_LUN	96
#define ONFI_NUM_LUNS		103	/* see note */
#define ONFI_RD_PARAM_PAGE_SZ	128
#define READ_PARAM_STATUS_OK	0x40
#define READ_PARAM_STATUS_MASK	0x41

#define ATH_NAND_IO_DBG		0
#define ATH_NAND_OOB_DBG	0
#define ATH_NAND_IN_DBG		0

#if ATH_NAND_IO_DBG
#	define iodbg	printk
#else
#	define iodbg(...)
#endif

#if ATH_NAND_OOB_DBG
#	define oobdbg	printk
#else
#	define oobdbg(...)
#endif

#if ATH_NAND_IN_DBG
#	define indbg(a, ...)					\
	do {							\
		printk("--- %s(%d):" a "\n",			\
			__func__, __LINE__, ## __VA_ARGS__);	\
	} while (0)
#else
#	define indbg(...)
#	define indbg1(a, ...)					\
	do {							\
		printk("--- %s(%d):" a "\n",			\
			__func__, __LINE__, ## __VA_ARGS__);	\
	} while (0)
#endif

/*
 * Data structures for ath nand flash controller driver
 */

typedef union {
	uint8_t			byte_id[8];

	struct {
		uint8_t		sa1	: 1,	// Serial access time (bit 1)
				org	: 1,	// Organisation
				bs	: 2,	// Block size
				sa0	: 1,	// Serial access time (bit 0)
				ss	: 1,	// Spare size per 512 bytes
				ps	: 2,	// Page Size

				wc	: 1,	// Write Cache
				ilp	: 1, 	// Interleaved Programming
				nsp	: 2, 	// No. of simult prog pages
				ct	: 2,	// Cell type
				dp	: 2,	// Die/Package

				did,		// Device id
				vid,		// Vendor id

				res1	: 2,	// Reserved
				pls	: 2,	// Plane size
				pn	: 2,	// Plane number
				res2	: 2;	// Reserved
	} __details;
} ath_nand_id_t;

uint64_t ath_plane_size[] = {
	64 << 20,
	 1 << 30,
	 2 << 30,
	 4 << 30,
	 8 << 30
};

typedef struct {
	uint8_t vid,
		did,
		b3,
		addrcyc,
		pgsz,
		blk,
		spare;
} ath_nand_vend_data_t;

ath_nand_vend_data_t ath_nand_arr[] = {
	{ 0x20, 0xda, 0x10, 5, 3, 1, 1 },	// NU2g3B2D
	{ 0x20, 0xf1, 0x00, 4, 3, 1, 1 },	// NU1g3B2C
	{ 0x20, 0xdc, 0x10, 5, 3, 1, 1 },	// NU4g3B2D
	{ 0x20, 0xd3, 0x10, 5, 4, 1, 1 },	// NU8g3F2A
	{ 0x20, 0xd3, 0x14, 5, 3, 2, 1 },	// NU8g3C2B
	{ 0xad, 0xf1, 0x00, 4, 3, 1, 1 },	// HY1g2b
	{ 0xad, 0xda, 0x10, 5, 3, 1, 1 },	// HY2g2b
	{ 0xec, 0xf1, 0x00, 4, 3, 1, 1 },	// Samsung 3,3V 8-bit [128MB]
	{ 0x98, 0xd1, 0x90, 4, 3, 1, 1 },	// Toshiba
	//{ 0x2c, 0x48, 0x04, 5, 4, 3, 1 },	// Micron 16GBit MLC
};

#define NUM_ARRAY_ENTRIES(a)	(sizeof((a)) / sizeof((a)[0]))
#define NUM_ATH_NAND		NUM_ARRAY_ENTRIES(ath_nand_arr)

/* ath nand info */
typedef struct {
	/* mtd info */
	struct mtd_info		mtd;

	/* NAND MTD partition information */
	int			nr_partitions;
	struct mtd_partition	*partitions;

	unsigned		*bbt;

	unsigned		ba0,
				ba1,
				cmd;	// Current command
	ath_nand_id_t		__id;	// for readid
	uint8_t			onfi[ONFI_RD_PARAM_PAGE_SZ];
#if ATH_NF_HW_ECC
	uint32_t		ecc_offset;
#endif
	uint32_t		nf_ctrl;
} ath_nand_sc_t;

ath_nand_sc_t *ath_nand_sc;
static int ath_nand_hw_init(ath_nand_sc_t *, void *);

#define	nid	__id.__details
#define	bid	__id.byte_id

static int ath_nand_block_isbad(struct mtd_info *mtd, loff_t ofs);

static const char *part_probes[] __initdata = { "cmdlinepart", "RedBoot", NULL };

/* max page size (16k) + oob buf size */
uint8_t	ath_nand_io_buf[24 << 10] __attribute__((aligned(4096)));
#define get_ath_nand_io_buf()	ath_nand_io_buf

void ath_nand_dump_buf(loff_t addr, void *v, unsigned count);

#define micron_debug		0

#if micron_debug
#define ATH_DEBUG_FS_SIZE	(7 << 20)
#define ATH_DEBUG_PG_SIZE	(2 << 10)
#define ATH_DEBUG_SP_SIZE	(64)
#define ATH_DEBUG_NUM_PAGES	(ATH_DEBUG_FS_SIZE / ATH_DEBUG_PG_SIZE)

#define fs_offset(o)		((o) - 0xa80000u)
#define off_to_page(o)		((fs_offset(o) / ATH_DEBUG_PG_SIZE) * ATH_DEBUG_SP_SIZE)
uint8_t	ath_dbg_fs[ATH_DEBUG_FS_SIZE],
	ath_dbg_fs_spare[ATH_DEBUG_NUM_PAGES * ATH_DEBUG_SP_SIZE];

loff_t	ath_dbg_last, ath_dbg_last_spare;

static void inline
ath_dbg_dump_to_fs(loff_t ofs, uint8_t *buf)
{
	ath_dbg_last = ofs;
	memcpy(ath_dbg_fs + fs_offset(ofs), buf, ATH_DEBUG_PG_SIZE);
	printk(KERN_DEBUG "===== 0x%llx %p %p\n", ofs, ath_dbg_fs + fs_offset(ofs), buf);
}

static void inline
ath_dbg_dump_to_fs_spare(loff_t ofs, uint8_t *buf)
{
	ath_dbg_last_spare = ofs;
	memcpy(ath_dbg_fs_spare + off_to_page(ofs), buf, ATH_DEBUG_SP_SIZE);
	//printk("----- 0x%llx %p %p\n", ofs, ath_dbg_fs_spare + off_to_page(ofs), buf);
}
#else
#define ath_dbg_dump_to_fs(...)		/* nothing */
#define ath_dbg_dump_to_fs_spare(...)	/* nothing */
#endif

#define	bbt_index	(sizeof(*sc->bbt) * 8 / 2)

inline unsigned
ath_nand_get_blk_state(struct mtd_info *mtd, loff_t b)
{
	unsigned		x, y;
	ath_nand_sc_t		*sc = mtd->priv;

	if (!sc->bbt)	return ATH_NAND_BLK_DONT_KNOW;

	b = b >> mtd->erasesize_shift;

	x = b / bbt_index;
	y = b % bbt_index;

	return (sc->bbt[x] >> (y * 2)) & 0x3;
}

inline void
ath_nand_set_blk_state(struct mtd_info *mtd, loff_t b, unsigned state)
{
	unsigned		x, y;
	ath_nand_sc_t		*sc = mtd->priv;

	if (!sc->bbt)	return;

	b = b >> mtd->erasesize_shift;

	x = b / bbt_index;
	y = b % bbt_index;

	sc->bbt[x] = (sc->bbt[x] & ~(3 << (y * 2))) | (state << (y * 2));
}

static unsigned
ath_nand_status(ath_nand_sc_t *sc, unsigned *ecc)
{
	unsigned	rddata, i, j, dmastatus;

	rddata = ath_reg_rd(ATH_NF_STATUS);
	for (i = 0; i < ATH_NF_STATUS_RETRY && rddata != 0xff; i++) {
		udelay(25);
		rddata = ath_reg_rd(ATH_NF_STATUS);
	}

	dmastatus = ath_reg_rd(ATH_NF_DMA_CTRL);
	for (j = 0; j < ATH_NF_STATUS_RETRY && !(dmastatus & 1); j++) {
		udelay(25);
		dmastatus = ath_reg_rd(ATH_NF_DMA_CTRL);
	}

	if ((i == ATH_NF_STATUS_RETRY) || (j == ATH_NF_STATUS_RETRY)) {
		//printk("ath_nand_status: i = %u j = %u\n", i, j);
		ath_nand_hw_init(sc, NULL);
		return -1;
	}
	if (ecc) {
		*ecc = ath_reg_rd(ATH_NF_ECC_CTRL);
	}
	ath_nand_clear_int_status();
	ath_reg_wr(ATH_NF_COMMAND, 0x07024);	// READ STATUS
	while (ath_nand_get_cmd_end_status() == 0);
	rddata = ath_reg_rd(ATH_NF_RD_STATUS);

	return rddata;
}

static unsigned
ath_check_all_0xff(ath_nand_sc_t *sc, unsigned addr0, unsigned addr1)
{
	uint8_t		*pa, *buf = ath_nand_io_buf, *end;
	struct mtd_info	*mtd = &sc->mtd;
	unsigned	i, count = mtd->writesize + mtd->oobsize;

	/*
	 * The same buf is being dma_(un)map_singled.
	 * Hope that is ok!
	 */
	ath_nand_clear_int_status();
	ath_reg_wr(ATH_NF_ADDR0_0, addr0);
	ath_reg_wr(ATH_NF_ADDR0_1, addr1);
	ath_reg_wr(ATH_NF_DMA_COUNT, count);
	ath_reg_wr(ATH_NF_DMA_CTRL, ATH_NF_DMA_CTRL_DMA_START |
				ATH_NF_DMA_CTRL_DMA_DIR_READ |
				ATH_NF_DMA_CTRL_DMA_BURST_3);
	ath_reg_wr(ATH_NF_ECC_OFFSET, 0);
	ath_reg_wr(ATH_NF_ECC_CTRL, 0);
	ath_reg_wr(ATH_NF_CTRL, sc->nf_ctrl | ATH_NF_CTRL_CUSTOM_SIZE_EN);
	ath_reg_wr(ATH_NF_PG_SIZE, count);
	pa = (uint8_t *)dma_map_single(NULL, buf, count, DMA_FROM_DEVICE);
	ath_reg_wr(ATH_NF_DMA_ADDR, (unsigned)pa);
	ath_reg_wr(ATH_NF_COMMAND, 0x30006a);	// Read page
	while (ath_nand_get_cmd_end_status() == 0);
	udelay(1000);
	i = ath_nand_status(sc, NULL) & ATH_NF_RD_STATUS_MASK;
	dma_unmap_single(NULL, (dma_addr_t)pa, count, DMA_FROM_DEVICE);
	if (i != ATH_NF_STATUS_OK) {
		return 0;
	}
	end = buf + count;
	for (buf += sc->ecc_offset; (*buf == 0xff) && buf != end; buf ++);

	if (buf == end) {
		/* This page was read without ECC. From the spare area
		 * content we see that it a blank page (i.e. full 0xff).
		 * To take care of bit flips if any, force 0xff on it.
		 */
		memset(ath_nand_io_buf, 0xff, mtd->writesize);
	} else {
		printk("%p %p %p 0x%x\n", ath_nand_io_buf, buf, end, count);
		ath_nand_dump_buf(addr0, ath_nand_io_buf, mtd->writesize + mtd->oobsize);
	}
	return (buf == end);
}

static unsigned
ath_nand_rw_page(ath_nand_sc_t *sc, int rd, unsigned addr0, unsigned addr1, unsigned count, unsigned char *buf, unsigned ecc_needed)
{
	unsigned	ecc, i = 0, tmp, rddata, all_0xff = 0;
#if ATH_NF_HW_ECC
	unsigned	mlc_retry = 0;
#endif
	char		*err[] = { "Write", "Read" };
#define ATH_MAX_RETRY	25
#define ATH_MLC_RETRY	3
retry:
	ecc = 0;
	ath_nand_clear_int_status();
	ath_reg_wr(ATH_NF_ADDR0_0, addr0);
	ath_reg_wr(ATH_NF_ADDR0_1, addr1);
	ath_reg_wr(ATH_NF_DMA_ADDR, (unsigned)buf);
	ath_reg_wr(ATH_NF_DMA_COUNT, count);

	{
		unsigned x0, x1;
		x0 = ath_reg_rd(ATH_NF_ADDR0_0);
		x1 = ath_reg_rd(ATH_NF_ADDR0_1);

		if (x0 != addr0 || x1 != addr1) {
			printk("=== reg write failed buf = %p ===\n", buf);
			printk("%s: 0x%x 0x%x, 0x%x 0x%x\n", __func__,
					x0, x1, addr0, addr1);
			printk("========================\n");
			panic("Register writes to NAND failed\n");
		}
	}
#if ATH_NF_HW_ECC
	if (ecc_needed && sc->ecc_offset && (count & sc->mtd.writesize_mask) == 0) {
		/*
		 * ECC can operate only on the device's pages.
		 * Cannot be used for non-page-sized read/write
		 */
		ath_reg_wr(ATH_NF_ECC_OFFSET, sc->ecc_offset);
		ath_reg_wr(ATH_NF_ECC_CTRL, ATH_NF_ECC_CTRL_ERR_THRESH(4) |
						ATH_NF_ECC_CTRL_ECC_4_BITS);
		ath_reg_wr(ATH_NF_CTRL, sc->nf_ctrl | ATH_NF_CTRL_ECC_EN);
		ath_reg_wr(ATH_NF_SPARE_SIZE, sc->mtd.oobsize);
	} else
#endif
	{
		ath_reg_wr(ATH_NF_ECC_OFFSET, 0);
		ath_reg_wr(ATH_NF_ECC_CTRL, 0);
		ath_reg_wr(ATH_NF_CTRL, sc->nf_ctrl | ATH_NF_CTRL_CUSTOM_SIZE_EN);
		ath_reg_wr(ATH_NF_PG_SIZE, count);
	}

	if (rd) {	// Read Page
		ath_reg_wr(ATH_NF_DMA_CTRL,
					ATH_NF_DMA_CTRL_DMA_START |
					ATH_NF_DMA_CTRL_DMA_DIR_READ |
					ATH_NF_DMA_CTRL_DMA_BURST_3);
		ath_reg_wr(ATH_NF_COMMAND, 0x30006a);
	} else {	// Write Page
		ath_reg_wr(ATH_NF_MEM_CTRL, 0xff00);	// Remove write protect
		ath_reg_wr(ATH_NF_DMA_CTRL,
					ATH_NF_DMA_CTRL_DMA_START |
					ATH_NF_DMA_CTRL_DMA_DIR_WRITE |
					ATH_NF_DMA_CTRL_DMA_BURST_3);
		ath_reg_wr(ATH_NF_COMMAND, 0x10804c);
	}

	while (ath_nand_get_cmd_end_status() == 0);

	//printk(KERN_DEBUG "%s(%c): 0x%x 0x%x 0x%x 0x%p\n", __func__,
	//	rd ? 'r' : 'w', addr0, addr1, count, buf);
	udelay(1000);
	rddata = (tmp = ath_nand_status(sc, &ecc)) & ATH_NF_RD_STATUS_MASK;
	if ((rddata != ATH_NF_STATUS_OK)) { // && (i < ATH_MAX_RETRY))
		i++;
		if ((i % 100) == 0) {
			printk("%s: retry %u\n", __func__, i);
		}
		goto retry;
	}

	ath_reg_wr(ATH_NF_MEM_CTRL, 0x0000);	// Enable write protect
	ath_reg_wr(ATH_NF_FIFO_INIT, 1);
	ath_reg_wr(ATH_NF_FIFO_INIT, 0);

	if (rddata != ATH_NF_STATUS_OK) {
		printk("%s: %s Failed. tmp = 0x%x, status = 0x%x 0x%x retries = %d\n", __func__,
			err[rd], tmp, rddata, ath_reg_rd(ATH_NF_DMA_CTRL), i);
	}
#if ATH_NF_HW_ECC
	else {
#define DDR_WB_FLUSH_USB_ADDRESS		0x180000a4

		ath_reg_wr(DDR_WB_FLUSH_USB_ADDRESS, 1);
		while (ath_reg_rd(DDR_WB_FLUSH_USB_ADDRESS) & 1);
		udelay(50);

		if (ecc_needed && (ecc & ATH_NF_ECC_ERROR)) {
			if (rd && all_0xff == 0) {
				all_0xff = 1;
				//printk("------- ath_check_all_0xff called for 0x%x 0x%x\n", addr1, addr0);
				if (ath_check_all_0xff(sc, addr0, addr1)) {
					return ATH_NF_STATUS_OK;
				}
			}

			if (mlc_retry < ATH_MLC_RETRY) {
				mlc_retry ++;
				i = 0;
				goto retry;
			} else {
				printk("%s: %s uncorrectable errors. 0x%x %x ecc = 0x%x\n",
					__func__, err[rd], addr0, addr1, ecc);
				return -1;
			}
		}
	}
#endif
	return rddata;
}

void
ath_nand_dump_buf(loff_t addr, void *v, unsigned count)
{
	unsigned	*buf = v,
			*end = buf + (count / sizeof(*buf));

	iodbg("____ Dumping %d bytes at 0x%p 0x%llx_____\n", count, buf, addr);

	for (; buf && buf < end; buf += 4, addr += 16) {
		iodbg("%08llx: %08x %08x %08x %08x\n",
			addr, buf[0], buf[1], buf[2], buf[3]);
	}
	iodbg("___________________________________\n");
	//while(1);
}

static int
ath_nand_rw_buff(struct mtd_info *mtd, int rd, uint8_t *buff,
		loff_t addr, size_t len, size_t *iodone)
{
	unsigned	iolen, ret = ATH_NF_STATUS_OK, dir;
	unsigned char	*pa;
	ath_nand_sc_t	*sc = mtd->priv;
	uint8_t		*buf = get_ath_nand_io_buf();

	*iodone = 0;

	dir = rd ? DMA_FROM_DEVICE : DMA_TO_DEVICE;

	while (len) {
		unsigned c, ba0, ba1;

		if (ath_nand_block_isbad(mtd, addr)) {
			//printk("Skipping bad block[0x%x]\n", (unsigned)addr);
			addr += mtd->erasesize;
			continue;
		}

		c = (addr & mtd->writesize_mask);

		/*
		 * addr format:
		 * a0 - a11 - xxxx - a19 - a27 == 32 bits, will be in ba0
		 * a28 - a31 - xxxxxxxxxxxxxxxx == 4 bits, will be in ba1 in lsb
		 */

		ba0 = ((addr >> mtd->writesize_shift) << 16);
		ba1 = ((addr >> (mtd->writesize_shift + 16)) & 0xf);

		if (c) {
			iolen = mtd->writesize - c;
		} else {
			iolen = mtd->writesize;
		}

		if (len < iolen) {
			iolen = len;
		}

		if (!rd) {
			int i;

			for (i = 0; (i < mtd->writesize) && (buff[i] == 0xff); i++);
			if (i == mtd->writesize) {
				ret = ATH_NF_STATUS_OK;
			//	printk("Skipping write for 0x%llx\n", addr);
				goto skip_write_for_all_0xff;
			}

			/* FIXME for writes FIXME */
			memcpy(buf, buff, iolen);
		}

		if (!rd) {
			//ath_nand_dump_buf(ATH_NF_COMMAND, (unsigned *)KSEG1ADDR(ATH_NF_COMMAND), 0xb8);
			ath_dbg_dump_to_fs(addr, buf);
		}
		pa = (unsigned char *)dma_map_single(NULL, buf, mtd->writesize, dir);

		//printk("%s(%c): 0x%x 0x%x 0x%x 0x%p\n", __func__,
		//	rd ? 'r' : 'w', ba0, ba1, iolen, pa);

		ret = ath_nand_rw_page(sc, rd, ba0, ba1, mtd->writesize, pa, 1);

		dma_unmap_single(NULL, (dma_addr_t)pa, mtd->writesize, dir);

		if (rd) {
			memcpy(buff, buf + c, iolen);
		}

skip_write_for_all_0xff:

		if (ret != ATH_NF_STATUS_OK) {
			return 1;
		}

		len -= iolen;
		buff += iolen;
		addr += iolen;
		*iodone += iolen;
	}

	return 0;
}

#define ath_nand_write_verify	0

#if ath_nand_write_verify
uint8_t	ath_nand_rd_buf[4096 + 256] __attribute__((aligned(4096)));
#endif

#define inject_failure	0
#if inject_failure
unsigned ath_nand_inject;
#endif

static int
ath_nand_write(struct mtd_info *mtd, loff_t to, size_t len,
		size_t *retlen, const u_char *buf)
{
	int	ret;
#if ath_nand_write_verify
	int	r, rl;
#endif

	if (!len || !retlen) return (0);

	indbg("0x%llx	%u", to, len);

	ret = ath_nand_rw_buff(mtd, 0 /* write */, (u_char *)buf, to, len, retlen);
#if ath_nand_write_verify
	//printk("Verifying 0x%llx 0x%x\n", to, len);
	r = ath_nand_rw_buff(mtd, 1 /* read */, ath_nand_rd_buf, to, len, &rl);
	if (r || memcmp(ath_nand_rd_buf, buf, len)) {
		printk("write failed at 0x%llx 0x%x\n", to, len);
		while (1);
	}
#endif
#if inject_failure
	if (ath_nand_inject & 2 && ((ath_nand_inject & ~2u) == to)) {
		retlen = 0;
		ret = -EIO;
		printk("Forcing write failure at 0x%llx\n", to);
	}
#endif

	return ret;
}

static int
ath_nand_read(struct mtd_info *mtd, loff_t from, size_t len,
		size_t *retlen, u_char *buf)
{
	int	ret;

	if (!len || !retlen) return (0);

	indbg("0x%llx	%u", from, len);

	ret = ath_nand_rw_buff(mtd, 1 /* read */, buf, from, len, retlen);

#if inject_failure
	if (ath_nand_inject & 1 && ((ath_nand_inject & ~1u) == from)) {
		retlen = 0;
		ret = -EIO;
		printk("Forcing read failure at 0x%llx\n", from);
	}
#endif

	return ret;
}

static inline int
ath_nand_block_erase(ath_nand_sc_t *sc, unsigned addr0, unsigned addr1)
{
	unsigned	rddata;

	indbg("0x%x 0x%x", addr1, addr0);

	ath_nand_clear_int_status();
	ath_reg_wr(ATH_NF_MEM_CTRL, 0xff00);	// Remove write protect
	ath_reg_wr(ATH_NF_ADDR0_0, addr0);
	ath_reg_wr(ATH_NF_ADDR0_1, addr1);
	ath_reg_wr(ATH_NF_COMMAND, 0xd0600e);	// BLOCK ERASE

	while (ath_nand_get_cmd_end_status() == 0);

	rddata = ath_nand_status(sc, NULL) & ATH_NF_RD_STATUS_MASK;

	ath_reg_wr(ATH_NF_MEM_CTRL, 0x0000);	// Enable write protect

	if (rddata != ATH_NF_STATUS_OK) {
		printk("Erase Failed. status = 0x%x", rddata);
		return 1;
	}
	return 0;
}


static int
ath_nand_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	uint64_t	s_first, i;
	unsigned	n, j;
	int		ret = 0, bad = 0;
	ath_nand_sc_t	*sc = mtd->priv;

	if (instr->addr + instr->len > mtd->size) {
		return (-EINVAL);
	}

	s_first = instr->addr;
	n = instr->len >> mtd->erasesize_shift;

	indbg("0x%llx 0x%llx 0x%x", instr->addr, s_last, mtd->erasesize);

	for (j = 0, i = s_first; j < n; j++, i += mtd->erasesize) {
		ulong ba0, ba1;

		if (ath_nand_block_isbad(mtd, i)) {
			bad ++;
			continue;
		}

		ba0 = ((i >> mtd->writesize_shift) << 16);
		ba1 = ((i >> (mtd->writesize_shift + 16)) & 0xf);

		if ((ret = ath_nand_block_erase(sc, ba0, ba1)) != 0) {
			iodbg("%s: erase failed 0x%llx 0x%llx 0x%x %llu "
				"%lx %lx\n", __func__, instr->addr, s_last,
				mtd->erasesize, i, ba1, ba0);
			break;
		}
		ath_nand_set_blk_state(mtd, i, ATH_NAND_BLK_ERASED);
#if inject_failure
		if (ath_nand_inject & 4 && ((ath_nand_inject & ~4u) == i)) {
			printk("Forcing erase failure at 0x%llx\n", i);
			break;
		}
#endif

	}

	if (instr->callback) {
		if ((j < n) || bad) {
			instr->state = MTD_ERASE_FAILED;
		} else {
			instr->state = MTD_ERASE_DONE;
		}
		mtd_erase_callback(instr);
	}

	return ret;
}

static int
ath_nand_rw_oob(struct mtd_info *mtd, int rd, loff_t addr,
		struct mtd_oob_ops *ops)
{
	unsigned	dir, ret = ATH_NF_STATUS_OK;
	unsigned char	*pa;
	unsigned	ba0, ba1;
	uint8_t		*buf = get_ath_nand_io_buf();
	uint8_t		*oob = buf + mtd->writesize;
	ath_nand_sc_t	*sc = mtd->priv;

	ba0 = ((addr >> mtd->writesize_shift) << 16);
	ba1 = ((addr >> (mtd->writesize_shift + 16)) & 0xf);

	dir = rd ? DMA_FROM_DEVICE : DMA_TO_DEVICE;

	if (!rd) {
		if (ops->datbuf) {
			/*
			 * XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
			 * We assume that the caller gives us a full
			 * page to write. We don't read the page and
			 * update the changed portions alone.
			 *
			 * Hence, not checking for len < or > pgsz etc...
			 * XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
			 */
			memcpy(buf, ops->datbuf, ops->len);
		}
		if (ops->mode == MTD_OPS_PLACE_OOB) {
			oob += ops->ooboffs;
		} else if (ops->mode == MTD_OPS_AUTO_OOB) {
			// clean markers
			oob[0] = oob[1] = 0xff;
			oob += 2;
		}
		memcpy(oob, ops->oobbuf, ops->ooblen);
	}

	if (!rd) {
		ath_dbg_dump_to_fs(addr, buf);
		ath_dbg_dump_to_fs_spare(addr, buf + mtd->writesize);
	}

	pa = (unsigned char *)dma_map_single(NULL, buf,
				mtd->writesize + mtd->oobsize, dir);

	//printk("%s(%c): 0x%x 0x%x 0x%x 0x%p\n", __func__,
	//	rd ? 'r' : 'w', ba0, ba1, mtd->writesize + mtd->oobsize, pa);

	ret = ath_nand_rw_page(sc, rd, ba0, ba1, mtd->writesize + mtd->oobsize, pa, 0);

	dma_unmap_single(NULL, (dma_addr_t)pa, mtd->writesize + mtd->oobsize, dir);

	//ath_nand_dump_buf(addr, buf, iolen);

	if (ret != ATH_NF_STATUS_OK) {
		return 1;
	}

	if (rd) {
		if (ops->datbuf) {
			memcpy(ops->datbuf, buf, ops->len);
		}
		if (ops->mode == MTD_OPS_PLACE_OOB) {
			oob += ops->ooboffs;
		} else if (ops->mode == MTD_OPS_AUTO_OOB) {
			// copy after clean marker
			oob += 2;
		}
		memcpy(ops->oobbuf, oob, ops->ooblen);
	}

	//if (rd) {
	//	ath_nand_dump_buf(addr, ops->datbuf, ops->len);
	//	ath_nand_dump_buf(addr, ops->oobbuf, ops->ooblen);
	//}

	if (ops->datbuf) {
		ops->retlen = ops->len;
	}
	ops->oobretlen = ops->ooblen;

	return 0;
}

static int
ath_nand_read_oob(struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops)
{
	oobdbg(	"%s: from: 0x%llx mode: 0x%x len: 0x%x retlen: 0x%x\n"
		"ooblen: 0x%x oobretlen: 0x%x ooboffs: 0x%x datbuf: %p "
		"oobbuf: %p\n", __func__, from,
		ops->mode, ops->len, ops->retlen, ops->ooblen,
		ops->oobretlen, ops->ooboffs, ops->datbuf,
		ops->oobbuf);

	indbg("0x%llx %p %p %u", from, ops->oobbuf, ops->datbuf, ops->len);

	return ath_nand_rw_oob(mtd, 1 /* read */, from, ops);
}

static int
ath_nand_write_oob(struct mtd_info *mtd, loff_t to, struct mtd_oob_ops *ops)
{
	int ret;
	unsigned char oob[128];
	struct mtd_oob_ops	rops = {
		.mode	= MTD_OPS_RAW,
		.ooblen	= mtd->oobsize,
		.oobbuf	= oob,
	};

	if (ops->mode == MTD_OPS_AUTO_OOB) {
		/* read existing oob */
		if (ath_nand_read_oob(mtd, to, &rops) ||
			rops.oobretlen != rops.ooblen) {
			printk("%s: oob read failed at 0x%llx\n", __func__, to);
			return 1;
		}
		memcpy(oob + 2, ops->oobbuf, ops->ooblen);
		rops = *ops;
		ops->oobbuf = oob;
		ops->ooblen = mtd->oobsize;
		ops->mode = MTD_OPS_RAW;
	}

	oobdbg(	"%s: from: 0x%llx mode: 0x%x len: 0x%x retlen: 0x%x\n"
		"ooblen: 0x%x oobretlen: 0x%x ooboffs: 0x%x datbuf: %p "
		"oobbuf: %p\n", __func__, to,
		ops->mode, ops->len, ops->retlen, ops->ooblen,
		ops->oobretlen, ops->ooboffs, ops->datbuf,
		ops->oobbuf);

	indbg("0x%llx", to);

	ret = ath_nand_rw_oob(mtd, 0 /* write */, to, ops);

	if (rops.mode == MTD_OPS_AUTO_OOB) {
		if (ret == 0) { // rw oob success
			rops.oobretlen = rops.ooblen;
			rops.retlen = rops.len;
		}
		*ops = rops;
	}

	return ret;
}

static int
ath_nand_block_isbad(struct mtd_info *mtd, loff_t ofs)
{
	unsigned char		oob[256];
	unsigned		bs, i;
	struct mtd_oob_ops	ops = {
		.mode	= MTD_OPS_RAW,
		.ooblen	= mtd->oobsize,
		.oobbuf	= oob,
	};

	bs = ath_nand_get_blk_state(mtd, ofs);

	if ((bs == ATH_NAND_BLK_ERASED) || (bs == ATH_NAND_BLK_GOOD)) {
		return 0;
	}

	if (bs == ATH_NAND_BLK_BAD) {
		return 1;
	}

	/*
	 * H27U1G8F2B Series [1 Gbit (128 M x 8 bit) NAND Flash]
	 *
	 * The Bad Block Information is written prior to shipping. Any
	 * block where the 1st Byte in the spare area of the 1st or
	 * 2nd th page (if the 1st page is Bad) does not contain FFh
	 * is a Bad Block. The Bad Block Information must be read
	 * before any erase is attempted as the Bad Block Information
	 * may be erased. For the system to be able to recognize the
	 * Bad Blocks based on the original information it is
	 * recommended to create a Bad Block table following the
	 * flowchart shown in Figure 24. The 1st block, which is
	 *                               ^^^^^^^^^^^^^
	 * placed on 00h block address is guaranteed to be a valid
	 * block.                      ^^^^^^^^^^^^^^^^^^^^^^^^^^^
	 */

	for (i = 0; i < 2; i++, ofs += mtd->writesize) {
		if (ath_nand_read_oob(mtd, ofs, &ops) ||
			ops.oobretlen != ops.ooblen) {
			printk("%s: oob read failed at 0x%llx\n", __func__, ofs);
			return 1;
		}

		/* First two bytes of oob data are clean markers */
		if (oob[0] != 0xff || oob[1] != 0xff) {
			oobdbg("%s: block is bad at 0x%llx\n", __func__, ofs);
			oobdbg(	"%02x %02x %02x %02x %02x %02x %02x %02x "
				"%02x %02x %02x %02x %02x %02x %02x %02x "
				"%02x %02x %02x %02x %02x %02x %02x %02x "
				"%02x %02x %02x %02x %02x %02x %02x %02x "
				"%02x %02x %02x %02x %02x %02x %02x %02x "
				"%02x %02x %02x %02x %02x %02x %02x %02x "
				"%02x %02x %02x %02x %02x %02x %02x %02x "
				"%02x %02x %02x %02x %02x %02x %02x %02x\n",
				0xff & oob[ 0], 0xff & oob[ 1], 0xff & oob[ 2],
				0xff & oob[ 3], 0xff & oob[ 4], 0xff & oob[ 5],
				0xff & oob[ 6], 0xff & oob[ 7], 0xff & oob[ 8],
				0xff & oob[ 9], 0xff & oob[10], 0xff & oob[11],
				0xff & oob[12], 0xff & oob[13], 0xff & oob[14],
				0xff & oob[15], 0xff & oob[16], 0xff & oob[17],
				0xff & oob[18], 0xff & oob[19], 0xff & oob[20],
				0xff & oob[21], 0xff & oob[22], 0xff & oob[23],
				0xff & oob[24], 0xff & oob[25], 0xff & oob[26],
				0xff & oob[27], 0xff & oob[28], 0xff & oob[29],
				0xff & oob[30], 0xff & oob[31], 0xff & oob[32],
				0xff & oob[33], 0xff & oob[34], 0xff & oob[35],
				0xff & oob[36], 0xff & oob[37], 0xff & oob[38],
				0xff & oob[39], 0xff & oob[40], 0xff & oob[41],
				0xff & oob[42], 0xff & oob[43], 0xff & oob[44],
				0xff & oob[45], 0xff & oob[46], 0xff & oob[47],
				0xff & oob[48], 0xff & oob[49], 0xff & oob[50],
				0xff & oob[51], 0xff & oob[52], 0xff & oob[53],
				0xff & oob[54], 0xff & oob[55], 0xff & oob[56],
				0xff & oob[57], 0xff & oob[58], 0xff & oob[59],
				0xff & oob[60], 0xff & oob[61], 0xff & oob[62],
				0xff & oob[63]);
			ath_nand_set_blk_state(mtd, ofs, ATH_NAND_BLK_BAD);
			return 1;
		}
	}

	for (i = 0; (i < mtd->oobsize) && (oob[i] == 0xff); i++);

	if (i == mtd->oobsize) {
		ath_nand_set_blk_state(mtd, ofs, ATH_NAND_BLK_ERASED);
	} else {
		ath_nand_set_blk_state(mtd, ofs, ATH_NAND_BLK_GOOD);
	}

	return 0;
}

static int
ath_nand_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	unsigned char oob[128] = { "bad block" };
	struct mtd_oob_ops	ops = {
		.mode	= MTD_OPS_RAW,
		.ooblen	= mtd->oobsize,
		.oobbuf	= oob,
	};

	indbg("called 0x%llx", ofs);

	if (ath_nand_write_oob(mtd, ofs, &ops) ||
		ops.oobretlen != ops.ooblen) {
		printk("%s: oob write failed at 0x%llx\n", __func__, ofs);
		return 1;
	}

	return 0;
}

static uint64_t 
ath_parse_read_id(ath_nand_sc_t *sc)
{
	int	i;

	extern struct nand_manufacturers nand_manuf_ids[];
	extern struct nand_flash_dev nand_flash_ids[];

	iodbg(	"____ %s _____\n"
		"  vid did wc  ilp nsp ct  dp  sa1 org bs  sa0 ss  "
		"ps  res1 pls pn  res2\n"
		"0x%3x %3x %3x %3x %3x %3x %3x %3x %3x %3x %3x %3x "
		"%3x %3x  %3x %3x %3x\n-------------\n", __func__,
			sc->nid.vid, sc->nid.did, sc->nid.wc, sc->nid.ilp,
			sc->nid.nsp, sc->nid.ct, sc->nid.dp, sc->nid.sa1,
			sc->nid.org, sc->nid.bs, sc->nid.sa0, sc->nid.ss,
			sc->nid.ps, sc->nid.res1, sc->nid.pls, sc->nid.pn,
			sc->nid.res2);

	for (i = 0; i < nand_manuf_ids[i].id; i++) {
		if (nand_manuf_ids[i].id == sc->nid.vid) {
			printk(nand_manuf_ids[i].name);
			break;
		}
	}

	for (i = 0; i < nand_flash_ids[i].id; i++) {
		if (nand_flash_ids[i].id == sc->nid.did) {
			printk(" %s [%luMB]\n", nand_flash_ids[i].name,
				nand_flash_ids[i].chipsize);
			return nand_flash_ids[i].chipsize;
		}
	}

	return 0;
}

ath_nand_vend_data_t *
nand_get_entry(ath_nand_id_t *nand_id, ath_nand_vend_data_t *tbl, int count)
{
	int     i;

	for (i = 0; i < count; i++, tbl ++) {
		if ((nand_id->__details.vid == tbl->vid) &&
		    (nand_id->__details.did == tbl->did) &&
		    (nand_id->byte_id[1] == tbl->b3)) {
			return tbl;
		}
	}

	return NULL;
}

static inline void
ath_nand_onfi_endian_convert(uint8_t *buf)
{
	uint32_t        i, *u = (uint32_t *)(buf + ONFI_DEV_DESC);

	for (i = 0; i < (ONFI_DEV_DESC_SZ / sizeof(*u)); i++) {
		u[i] = __le32_to_cpu(u[i]);
	}

	// Hope nobody has a 20 character device description
	buf[ONFI_DEV_DESC + ONFI_DEV_DESC_SZ - 1] = 0;
}

int
nand_param_page(ath_nand_sc_t *sc, uint8_t *buf, unsigned count)
{
	unsigned int	tries, rddata;
	uint8_t		*pa;

	pa = (uint8_t *)dma_map_single(NULL, buf, count, DMA_FROM_DEVICE);

	for (tries = 3; tries; tries --) {
		// ADDR0_0 Reg Settings
		ath_reg_wr(ATH_NF_ADDR0_0, 0x0);

		// ADDR0_1 Reg Settings
		ath_reg_wr(ATH_NF_ADDR0_1, 0x0);

		// DMA Start Addr
		ath_reg_wr(ATH_NF_DMA_ADDR, (unsigned)pa);

		// DMA count
		ath_reg_wr(ATH_NF_DMA_COUNT, count);

		// Custom Page Size
		ath_reg_wr(ATH_NF_PG_SIZE, count);

		// DMA Control Reg
		ath_reg_wr(ATH_NF_DMA_CTRL, 0xcc);

		ath_nand_clear_int_status();
		// READ PARAMETER PAGE
		ath_reg_wr(ATH_NF_COMMAND, 0xec62);
		while (ath_nand_get_cmd_end_status() == 0);

		rddata = ath_nand_status(sc, NULL) & READ_PARAM_STATUS_MASK;
		if (rddata == READ_PARAM_STATUS_OK) {
			break;
		} else {
			printk("\nParam Page Failure: 0x%x", rddata);
			ath_nand_hw_init(sc, NULL);
		}
	}

	dma_unmap_single(NULL, (dma_addr_t)pa, count, DMA_FROM_DEVICE);

	if ((rddata == READ_PARAM_STATUS_OK) &&
	    (buf[3] == 'O' && buf[2] == 'N' && buf[1] == 'F' && buf[0] == 'I')) {
		ath_nand_onfi_endian_convert(buf);
		printk("ONFI %s\n", buf + ONFI_DEV_DESC);
		return 0;
	}

	return 1;
}

/*
 * System initialization functions
 */
static int
ath_nand_hw_init(ath_nand_sc_t *sc, void *p)
{
	uint8_t		id[8];
	unsigned char	*pa;
	unsigned	rddata, i;

	// Put into reset
	ath_reg_rmw_set(ATH_RESET, RST_RESET_NANDF_RESET_MASK);
	udelay(250);

	ath_reg_rmw_clear(ATH_RESET, RST_RESET_NANDF_RESET_MASK);
	udelay(100);

	ath_reg_wr(ATH_NF_INT_MASK, ATH_NF_CMD_END_INT);
	ath_nand_clear_int_status();

	// TIMINGS_ASYN Reg Settings
	ath_reg_wr(ATH_NF_TIMINGS_ASYN, ATH_NF_TIMING_ASYN);

	// NAND Mem Control Reg
	ath_reg_wr(ATH_NF_MEM_CTRL, 0xff00);
	//ath_reg_wr(ATH_NF_MEM_CTRL, 0x0000);

	// Reset Command
	ath_reg_wr(ATH_NF_COMMAND, 0xff00);

	while (ath_nand_get_cmd_end_status() == 0);

	udelay(1000);

	rddata = ath_reg_rd(ATH_NF_STATUS);
	for (i = 0; i < ATH_NF_STATUS_RETRY && rddata != 0xff; i++) {
		udelay(25);
		rddata = ath_reg_rd(ATH_NF_STATUS);
	}

	if (i == ATH_NF_STATUS_RETRY) {
		printk("device reset failed\n");
		while(1);
	}

	if (p) {
		ath_nand_vend_data_t *entry;

		ath_nand_clear_int_status();
		pa = (unsigned char *)dma_map_single(NULL, p ? p : id,
						8, DMA_FROM_DEVICE);
		ath_reg_wr(ATH_NF_DMA_ADDR, (unsigned)p);
		ath_reg_wr(ATH_NF_ADDR0_0, 0x0);
		ath_reg_wr(ATH_NF_ADDR0_1, 0x0);
		ath_reg_wr(ATH_NF_DMA_COUNT, 0x8);
		ath_reg_wr(ATH_NF_PG_SIZE, 0x8);
		ath_reg_wr(ATH_NF_DMA_CTRL, 0xcc);
		ath_reg_wr(ATH_NF_COMMAND, 0x9061);	// READ ID
		while (ath_nand_get_cmd_end_status() == 0);

		rddata = ath_nand_status(sc, NULL);
		if ((rddata & ATH_NF_RD_STATUS_MASK) != ATH_NF_STATUS_OK) {
			printk("%s: failed\nath nand status = 0x%x\n",
				__func__, rddata);
		}
		dma_unmap_single(NULL, (dma_addr_t)pa, 8, DMA_FROM_DEVICE);

		pa = p;
		printk("Ath Nand ID[%p]: %02x:%02x:%02x:%02x:%02x\n",
				pa, pa[3], pa[2], pa[1], pa[0], pa[7]);

		sc->onfi[0] = 0;

		entry = nand_get_entry((ath_nand_id_t *)p, ath_nand_arr, NUM_ATH_NAND);
		if (entry) {
			sc->nf_ctrl = ATH_NF_CTRL_ADDR_CYCLE0(entry->addrcyc);
		} else if (nand_param_page(sc, sc->onfi, sizeof(sc->onfi)) == 0) {
			rddata = sc->onfi[ONFI_NUM_ADDR_CYCLES];
			rddata = ((rddata >> 4) & 0xf) + (rddata & 0xf);
			sc->nf_ctrl = ATH_NF_CTRL_ADDR_CYCLE0(rddata);
		} else {
			printk("Attempting to use unknown device\n");
			sc->nf_ctrl = ATH_NF_CTRL_ADDR_CYCLE0(5);
		}

		iodbg("******* %s done ******\n", __func__);
	}

	return 0;
}

/*
 * Copied from drivers/mtd/nand/nand_base.c
 * http://ptgmedia.pearsoncmg.com/images/chap17_9780132396554/elementLinks/17fig04.gif
 *
 * +---...---+--+----------+---------+
 * |  2048   |  |          |         |
 * | File    |cm| FS spare | ecc data|
 * | data    |  |          |         |
 * +---...---+--+----------+---------+
 * cm -> clean marker (2 bytes)
 * FS Spare -> bytes available for jffs2
 */
static struct nand_ecclayout ath_nand_oob = {
	.oobfree = { {.offset = 2, .length = 18} }
};

static void
ath_nand_ecc_init(struct mtd_info *mtd)
{
#if ATH_NF_HW_ECC
	ath_nand_sc_t		*sc = mtd->priv;
	int			i, off;

	for (off = i = 0; ath_nand_oob.oobfree[i].length
                	&& i < ARRAY_SIZE(ath_nand_oob.oobfree); i++) {
		ath_nand_oob.oobavail += ath_nand_oob.oobfree[i].length;
		off += ath_nand_oob.oobfree[i].offset;
	}
	mtd->ecclayout = &ath_nand_oob;
	sc->ecc_offset = mtd->writesize + ath_nand_oob.oobavail + off;
#else
	mtd->ecclayout	= NULL;
	sc->ecc_offset = 0;
#endif
}

static struct mtd_partition dir_parts[] = {
      {name: "RedBoot", offset: 0x0, size:0x80000,},
      {name: "linux", offset: 0x6c0000, size:0x40000,},
      {name: "rootfs", offset: 0x0, size:0x2b0000,},
      {name: "ddwrt", offset: 0x0, size:0x2b0000,},
      {name: "nvram", offset: 0x340000, size:0x80000,},
      {name: "board_config", offset: 0x80000, size:0x40000,},
      {name: "fullflash", offset: 0x0, size:0x10000,},
      {name:NULL,},
};

static struct mtd_partition ubi_parts[] = {
      {name: "RedBoot", offset: 0x0, size:0x80000,},
      {name: "linux", offset: 0x6c0000, size:0x40000,},
      {name: "nvram", offset: 0x340000, size:0x80000,},
      {name: "fullflash", offset: 0x0, size:0x10000,},
      {name:NULL,},
};


/*
 * Device management interface
 */
static int ath_nand_add_partition(ath_nand_sc_t *sc)
{
	struct mtd_info *mtd = &sc->mtd;
#ifdef CONFIG_MTD
	struct squashfs_super_block *sb;
	uint64_t offset;
	char buf[512];
	char *bbuf = NULL;
	char *ubi = NULL;
	int retlen;
	unsigned int rootsize,len;
	uint64_t base = offset + mtd->erasesize;
	while (base < mtd->size) {
		mtd_read(mtd,offset,512,&retlen,&buf[0]);		
		if (*((__u32 *)buf) == SQUASHFS_MAGIC_SWAP)
		    bbuf = buf;
		else
		if (*((__u32 *)&buf[128]) == SQUASHFS_MAGIC_SWAP)
		{
		    bbuf = &buf[128];
		    offset+=128;
		}
		else
		if (!memcmp(buf, "UBI#",4))
		    ubi = buf;		
		else {
		    bbuf = NULL;
		    ubi = NULL;
		}
				    
		if (bbuf) {
				printk(KERN_INFO "\nfound squashfs at 0x%llX\n",offset);
				sb = (struct squashfs_super_block *)buf;
				dir_parts[2].offset = offset;
				dir_parts[2].size = le64_to_cpu(sb->bytes_used);
				len = dir_parts[2].offset + dir_parts[2].size;
				len += (mtd->erasesize - 1);
				len &= ~(mtd->erasesize - 1);
				dir_parts[2].size = (len & 0x1ffffff) - dir_parts[2].offset;
				dir_parts[3].offset = dir_parts[2].offset + dir_parts[2].size;
				dir_parts[3].size = mtd->size - dir_parts[3].offset;
				dir_parts[1].size = dir_parts[3].offset + dir_parts[3].size - dir_parts[1].offset; 
				dir_parts[6].size = mtd->size;
				break;
		}

		if (ubi) {
				printk(KERN_INFO "\nfound ubifs at 0x%llX\n",offset);
				ubi_parts[0].size = offset;
				ubi_parts[1].offset = offset;
				ubi_parts[1].size = mtd->size - offset;
				ubi_parts[2].offset = ubi_parts[0].size;
				ubi_parts[2].offset -= mtd->erasesize; // last block safed for special purpose
				ubi_parts[2].offset -= 0x80000; 
				ubi_parts[2].size = 0x80000;

				ubi_parts[3].offset = 0; //must use all for ubi, since ubi is shit and doesnt like data beyond itself
				ubi_parts[3].size = mtd->size;
				break;
		}
	offset += mtd->erasesize;
	base += mtd->erasesize;
	}
	if (bbuf)
		return add_mtd_partitions(mtd, dir_parts, 7);
	if (ubi) 
		return add_mtd_partitions(mtd, ubi_parts, 4);

	return -1;
#else
	return add_mtd_device(mtd);
#endif
}

static int ath_nand_remove(void)
{
#ifdef CONFIG_MTD
	/* Deregister partitions */
	del_mtd_partitions(&ath_nand_sc->mtd);
#endif
	kfree(ath_nand_sc);
	ath_nand_sc = NULL;
	return 0;
}

extern void nand_postinit(struct mtd_info *mtd);
/*
 * ath_nand_probe
 *
 * called by device layer when it finds a device matching
 * one our driver can handled. This code checks to see if
 * it can allocate all necessary resources then calls the
 * nand layer to look for devices
 */
static int ath_nand_probe(void)
{
	ath_nand_sc_t	*sc = NULL;
	struct mtd_info	*mtd = NULL;
	int		i, err = 0, bbt_size;
	unsigned	nf_ctrl_pg[][2] = {
		/* page size in bytes, register val */
		{   256, ATH_NF_CTRL_PAGE_SIZE_256	},
		{   512, ATH_NF_CTRL_PAGE_SIZE_512	},
		{  1024, ATH_NF_CTRL_PAGE_SIZE_1024	},
		{  2048, ATH_NF_CTRL_PAGE_SIZE_2048	},
		{  4096, ATH_NF_CTRL_PAGE_SIZE_4096	},
		{  8192, ATH_NF_CTRL_PAGE_SIZE_8192	},
		{ 16384, ATH_NF_CTRL_PAGE_SIZE_16384	},
		{     0, ATH_NF_CTRL_PAGE_SIZE_0	},
		};
	unsigned	nf_ctrl_blk[][2] = {
		/* no. of pages, register val */
		{  32, ATH_NF_CTRL_BLOCK_SIZE_32	},
		{  64, ATH_NF_CTRL_BLOCK_SIZE_64	},
		{ 128, ATH_NF_CTRL_BLOCK_SIZE_128	},
		{ 256, ATH_NF_CTRL_BLOCK_SIZE_256	},
		{   0, 0				},
		};

	sc = kzalloc(sizeof(*sc), GFP_KERNEL);
	if (sc == NULL) {
		printk("%s: no memory for flash sc\n", __func__);
		err = -ENOMEM;
		goto out_err_kzalloc;
	}

	/* initialise the hardware */
	err = ath_nand_hw_init(sc, &sc->nid);
	if (err) {
		goto out_err_hw_init;
	}

	/* initialise mtd sc data struct */
	mtd = &sc->mtd;
	mtd->size = ath_parse_read_id(sc) << 20;

	mtd->name		= DRV_NAME;
	mtd->owner		= THIS_MODULE;
	if (mtd->size == 0) {
		mtd->size	= ath_plane_size[sc->nid.pls] << sc->nid.pn;
	}

	if (!sc->onfi[0]) {
		mtd->writesize_shift	= 10 + sc->nid.ps;
		mtd->writesize		= (1 << mtd->writesize_shift);
		mtd->writesize_mask	= (mtd->writesize - 1);

		mtd->erasesize_shift	= 16 + sc->nid.bs;
		mtd->erasesize		= (1 << mtd->erasesize_shift);
		mtd->erasesize_mask	= (mtd->erasesize - 1);

		mtd->oobsize		= (mtd->writesize / 512) * (8 << sc->nid.ss);
		mtd->oobavail		= mtd->oobsize;
	} else {
		mtd->writesize		= *(uint32_t *)(&sc->onfi[ONFI_PAGE_SIZE]);
		mtd->writesize_shift	= ffs(mtd->writesize) - 1;
		mtd->writesize_mask	= (mtd->writesize - 1);

		mtd->erasesize		= *(uint32_t *)(&sc->onfi[ONFI_PAGES_PER_BLOCK]) *
					  mtd->writesize;
		mtd->erasesize_shift	= ffs(mtd->erasesize) - 1;
		mtd->erasesize_mask	= (mtd->erasesize - 1);

		mtd->oobsize		= *(uint16_t *)(&sc->onfi[ONFI_SPARE_SIZE]);
		mtd->oobavail		= mtd->oobsize;

		mtd->size		= mtd->erasesize *
					  (*(uint32_t *)(&sc->onfi[ONFI_BLOCKS_PER_LUN])) *
					  sc->onfi[ONFI_NUM_LUNS];
	}
	mtd->writebufsize = mtd->writesize;

	for (i = 0; nf_ctrl_pg[i][0]; i++) {
		if (nf_ctrl_pg[i][0] == mtd->writesize) {
			sc->nf_ctrl |= nf_ctrl_pg[i][1];
			break;
		}
	}

	for (i = 0; nf_ctrl_blk[i][0]; i++) {
		if (nf_ctrl_blk[i][0] == (mtd->erasesize / mtd->writesize)) {
			sc->nf_ctrl |= nf_ctrl_blk[i][1];
			break;
		}
	}

	mtd->type		= MTD_NANDFLASH;
	mtd->flags		= MTD_CAP_NANDFLASH;

	mtd->_read		= ath_nand_read;
	mtd->_write		= ath_nand_write;
	mtd->_erase		= ath_nand_erase;

	mtd->_read_oob		= ath_nand_read_oob;
	mtd->_write_oob		= ath_nand_write_oob;

	mtd->_block_isbad	= ath_nand_block_isbad;
	mtd->_block_markbad	= ath_nand_block_markbad;

	mtd->priv		= sc;

	ath_nand_ecc_init(mtd);

	/* add NAND partition */
	ath_nand_add_partition(sc);

	// bbt has 2 bits per block
	bbt_size = ((mtd->size >> mtd->erasesize_shift) * 2) / 8;
	sc->bbt = kmalloc(bbt_size, GFP_KERNEL);

	if (sc->bbt) {
		memset(sc->bbt, 0, bbt_size);
	}

	ath_nand_sc = sc;
	printk(	"====== NAND Parameters ======\n"
		"sc = 0x%p bbt = 0x%p bbt_size = 0x%x nf_ctrl = 0x%x\n"
		"page = 0x%x block = 0x%x oob = 0x%x\n", sc, sc->bbt, bbt_size,
		sc->nf_ctrl, mtd->writesize, mtd->erasesize, mtd->oobsize);

	nand_postinit(mtd);
	return 0;

out_err_hw_init:
	kfree(sc);
out_err_kzalloc:

	return err;
}

#if 0
static struct platform_driver ath_nand_driver = {
	//.probe		= ath_nand_probe,
	.remove		= ath_nand_remove,
	.driver		= {
		.name	= DRV_NAME,
		.owner	= THIS_MODULE,
	},
};
#endif


static int __init ath_nand_init(void)
{
	printk(DRV_DESC ", Version " DRV_VERSION
		" (c) 2010 Atheros Communications, Ltd.\n");

	if (soc_is_qca955x())
		ATH_NAND_FLASH_BASE=0x1b800200;

	//return platform_driver_register(&ath_nand_driver);
	//return platform_driver_probe(&ath_nand_driver, ath_nand_probe);
	return ath_nand_probe();
}

static void __exit ath_nand_exit(void)
{
	//platform_driver_unregister(&ath_nand_driver);
	ath_nand_remove();
}

module_init(ath_nand_init);
module_exit(ath_nand_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRV_AUTHOR);
MODULE_DESCRIPTION(DRV_DESC);
MODULE_ALIAS("platform:" DRV_NAME);
