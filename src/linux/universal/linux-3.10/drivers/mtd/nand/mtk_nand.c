/******************************************************************************
* mtk_nand.c - MTK NAND Flash Device Driver
 *
* Copyright 2009-2012 MediaTek Co.,Ltd.
 *
* DESCRIPTION:
* 	This file provid the other drivers nand relative functions
 *
* modification history
* ----------------------------------------
* v3.0, 11 Feb 2010, mtk
* ----------------------------------------
******************************************************************************/
#include "nand_def.h"
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/dma-mapping.h>
#include <linux/jiffies.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/time.h>
#include <linux/mm.h>
#include <asm/io.h>
#include <asm/cacheflush.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include "mtk_nand.h"
#include "nand_device_list.h"

#include "bmt.h"
#include "partition.h"

unsigned int CFG_BLOCKSIZE;

static int shift_on_bbt = 0;
extern void nand_bbt_set(struct mtd_info *mtd, int page, int flag);
extern int nand_bbt_get(struct mtd_info *mtd, int page);
int mtk_nand_read_oob_hw(struct mtd_info *mtd, struct nand_chip *chip, int page);

static const char * const probe_types[] = { "cmdlinepart", "ofpart", NULL };

#define NAND_CMD_STATUS_MULTI  0x71

void show_stack(struct task_struct *tsk, unsigned long *sp);
extern void mt_irq_set_sens(unsigned int irq, unsigned int sens);
extern void mt_irq_set_polarity(unsigned int irq,unsigned int polarity);

struct mtk_nand_host	mtk_nand_host;	/* include mtd_info and nand_chip structs */
struct mtk_nand_host_hw mt7621_nand_hw = {
    .nfi_bus_width          = 8,
    .nfi_access_timing      = NFI_DEFAULT_ACCESS_TIMING,
    .nfi_cs_num             = NFI_CS_NUM,
    .nand_sec_size          = 512,
    .nand_sec_shift         = 9,
    .nand_ecc_size          = 2048,
    .nand_ecc_bytes         = 32,
    .nand_ecc_mode          = NAND_ECC_HW,
};


/*******************************************************************************
 * Gloable Varible Definition
 *******************************************************************************/

#define NFI_ISSUE_COMMAND(cmd, col_addr, row_addr, col_num, row_num) \
   do { \
      DRV_WriteReg(NFI_CMD_REG16,cmd);\
      while (DRV_Reg32(NFI_STA_REG32) & STA_CMD_STATE);\
      DRV_WriteReg32(NFI_COLADDR_REG32, col_addr);\
      DRV_WriteReg32(NFI_ROWADDR_REG32, row_addr);\
      DRV_WriteReg(NFI_ADDRNOB_REG16, col_num | (row_num<<ADDR_ROW_NOB_SHIFT));\
      while (DRV_Reg32(NFI_STA_REG32) & STA_ADDR_STATE);\
   }while(0);

//-------------------------------------------------------------------------------
static struct NAND_CMD g_kCMD;
static u32 g_u4ChipVer;
bool g_bInitDone;
static bool g_bcmdstatus;
static u32 g_value = 0;
static int g_page_size;

BOOL g_bHwEcc = true;


static u8 *local_buffer_16_align;   // 16 byte aligned buffer, for HW issue
static u8 local_buffer[4096 + 512];

extern void nand_release_device(struct mtd_info *mtd);
extern int nand_get_device(struct nand_chip *chip, struct mtd_info *mtd, int new_state);

#if defined(MTK_NAND_BMT)
static bmt_struct *g_bmt;
#endif
struct mtk_nand_host *host;
extern struct mtd_partition g_pasStatic_Partition[];
int part_num = NUM_PARTITIONS;
int manu_id;
int dev_id;

static u8 local_oob_buf[NAND_MAX_OOBSIZE];

static u8 nand_badblock_offset = 0;

void nand_enable_clock(void)
{
    //enable_clock(MT65XX_PDN_PERI_NFI, "NAND");
}

void nand_disable_clock(void)
{
    //disable_clock(MT65XX_PDN_PERI_NFI, "NAND");
}

static struct nand_ecclayout nand_oob_16 = {
	.eccbytes = 8,
	.eccpos = {8, 9, 10, 11, 12, 13, 14, 15},
	.oobfree = {{1, 6}, {0, 0}}
};

struct nand_ecclayout nand_oob_64 = {
	.eccbytes = 32,
	.eccpos = {32, 33, 34, 35, 36, 37, 38, 39,
		40, 41, 42, 43, 44, 45, 46, 47,
		48, 49, 50, 51, 52, 53, 54, 55,
		56, 57, 58, 59, 60, 61, 62, 63},
	.oobfree = {{1, 7}, {9, 7}, {17, 7}, {25, 6}, {0, 0}}
};

struct nand_ecclayout nand_oob_128 = {
	.eccbytes = 64,
	.eccpos = {
		64, 65, 66, 67, 68, 69, 70, 71,
		72, 73, 74, 75, 76, 77, 78, 79,
		80, 81, 82, 83, 84, 85, 86, 86,
		88, 89, 90, 91, 92, 93, 94, 95,
		96, 97, 98, 99, 100, 101, 102, 103,
		104, 105, 106, 107, 108, 109, 110, 111,
		112, 113, 114, 115, 116, 117, 118, 119,
		120, 121, 122, 123, 124, 125, 126, 127},
	.oobfree = {{1, 7}, {9, 7}, {17, 7}, {25, 7}, {33, 7}, {41, 7}, {49, 7}, {57, 6}}
};

flashdev_info devinfo;

void dump_nfi(void)
{
}

void dump_ecc(void)
{
}

u32
nand_virt_to_phys_add(u32 va)
{
	u32 pageOffset = (va & (PAGE_SIZE - 1));
	pgd_t *pgd;
	pmd_t *pmd;
	pte_t *pte;
	u32 pa;

	if (virt_addr_valid(va))
		return __virt_to_phys(va);

	if (NULL == current) {
		printk(KERN_ERR "[nand_virt_to_phys_add] ERROR ,current is NULL! \n");
		return 0;
	}

	if (NULL == current->mm) {
		printk(KERN_ERR "[nand_virt_to_phys_add] ERROR current->mm is NULL! tgid=0x%x, name=%s \n", current->tgid, current->comm);
		return 0;
	}

	pgd = pgd_offset(current->mm, va);  /* what is tsk->mm */
	if (pgd_none(*pgd) || pgd_bad(*pgd)) {
		printk(KERN_ERR "[nand_virt_to_phys_add] ERROR, va=0x%x, pgd invalid! \n", va);
		return 0;
	}

	pmd = pmd_offset((pud_t *)pgd, va);
	if (pmd_none(*pmd) || pmd_bad(*pmd)) {
		printk(KERN_ERR "[nand_virt_to_phys_add] ERROR, va=0x%x, pmd invalid! \n", va);
		return 0;
	}

	pte = pte_offset_map(pmd, va);
	if (pte_present(*pte)) {
		pa = (pte_val(*pte) & (PAGE_MASK)) | pageOffset;
		return pa;
	}

	printk(KERN_ERR "[nand_virt_to_phys_add] ERROR va=0x%x, pte invalid! \n", va);
	return 0;
}
EXPORT_SYMBOL(nand_virt_to_phys_add);

bool
get_device_info(u16 id, u32 ext_id, flashdev_info * pdevinfo)
{
	u32 index;
	for (index = 0; gen_FlashTable[index].id != 0; index++) {
		if (id == gen_FlashTable[index].id && ext_id == gen_FlashTable[index].ext_id) {
			pdevinfo->id = gen_FlashTable[index].id;
			pdevinfo->ext_id = gen_FlashTable[index].ext_id;
			pdevinfo->blocksize = gen_FlashTable[index].blocksize;
			pdevinfo->addr_cycle = gen_FlashTable[index].addr_cycle;
			pdevinfo->iowidth = gen_FlashTable[index].iowidth;
			pdevinfo->timmingsetting = gen_FlashTable[index].timmingsetting;
			pdevinfo->advancedmode = gen_FlashTable[index].advancedmode;
			pdevinfo->pagesize = gen_FlashTable[index].pagesize;
			pdevinfo->sparesize = gen_FlashTable[index].sparesize;
			pdevinfo->totalsize = gen_FlashTable[index].totalsize;
			memcpy(pdevinfo->devciename, gen_FlashTable[index].devciename, sizeof(pdevinfo->devciename));
			printk(KERN_INFO "Device found in MTK table, ID: %x, EXT_ID: %x\n", id, ext_id);

			goto find;
		}
	}

find:
	if (0 == pdevinfo->id) {
		printk(KERN_INFO "Device not found, ID: %x\n", id);
		return false;
	} else {
		return true;
	}
}

static void
ECC_Config(struct mtk_nand_host_hw *hw,u32 ecc_bit)
{
	u32 u4ENCODESize;
	u32 u4DECODESize;
	u32 ecc_bit_cfg = ECC_CNFG_ECC4;

	switch(ecc_bit){
	case 4:
		ecc_bit_cfg = ECC_CNFG_ECC4;
		break;
	case 8:
		ecc_bit_cfg = ECC_CNFG_ECC8;
		break;
	case 10:
		ecc_bit_cfg = ECC_CNFG_ECC10;
		break;
	case 12:
		ecc_bit_cfg = ECC_CNFG_ECC12;
		break;
	default:
		break;
	}
	DRV_WriteReg16(ECC_DECCON_REG16, DEC_DE);
	do {
	} while (!DRV_Reg16(ECC_DECIDLE_REG16));

	DRV_WriteReg16(ECC_ENCCON_REG16, ENC_DE);
	do {
	} while (!DRV_Reg32(ECC_ENCIDLE_REG32));

	/* setup FDM register base */
	DRV_WriteReg32(ECC_FDMADDR_REG32, NFI_FDM0L_REG32);

	/* Sector + FDM */
	u4ENCODESize = (hw->nand_sec_size + 8) << 3;
	/* Sector + FDM + YAFFS2 meta data bits */
	u4DECODESize = ((hw->nand_sec_size + 8) << 3) + ecc_bit * 13;

	/* configure ECC decoder && encoder */
	DRV_WriteReg32(ECC_DECCNFG_REG32, ecc_bit_cfg | DEC_CNFG_NFI | DEC_CNFG_EMPTY_EN | (u4DECODESize << DEC_CNFG_CODE_SHIFT));

	DRV_WriteReg32(ECC_ENCCNFG_REG32, ecc_bit_cfg | ENC_CNFG_NFI | (u4ENCODESize << ENC_CNFG_MSG_SHIFT));
	NFI_SET_REG32(ECC_DECCNFG_REG32, DEC_CNFG_EL);
}

static void
ECC_Decode_Start(void)
{
	while (!(DRV_Reg16(ECC_DECIDLE_REG16) & DEC_IDLE))
		;
	DRV_WriteReg16(ECC_DECCON_REG16, DEC_EN);
}

static void
ECC_Decode_End(void)
{
	while (!(DRV_Reg16(ECC_DECIDLE_REG16) & DEC_IDLE))
		;
	DRV_WriteReg16(ECC_DECCON_REG16, DEC_DE);
}

static void
ECC_Encode_Start(void)
{
	while (!(DRV_Reg32(ECC_ENCIDLE_REG32) & ENC_IDLE))
		;
	mb();
	DRV_WriteReg16(ECC_ENCCON_REG16, ENC_EN);
}

static void
ECC_Encode_End(void)
{
	/* wait for device returning idle */
	while (!(DRV_Reg32(ECC_ENCIDLE_REG32) & ENC_IDLE)) ;
	mb();
	DRV_WriteReg16(ECC_ENCCON_REG16, ENC_DE);
}

static bool
mtk_nand_check_bch_error(struct mtd_info *mtd, u8 * pDataBuf, u32 u4SecIndex, u32 u4PageAddr)
{
	bool bRet = true;
	u16 u2SectorDoneMask = 1 << u4SecIndex;
	u32 u4ErrorNumDebug, i, u4ErrNum;
	u32 timeout = 0xFFFF;
	// int el;
	u32 au4ErrBitLoc[6];
	u32 u4ErrByteLoc, u4BitOffset;
	u32 u4ErrBitLoc1th, u4ErrBitLoc2nd;

	//4 // Wait for Decode Done
	while (0 == (u2SectorDoneMask & DRV_Reg16(ECC_DECDONE_REG16))) {
		timeout--;
		if (0 == timeout)
			return false;
	}
	/* We will manually correct the error bits in the last sector, not all the sectors of the page! */
	memset(au4ErrBitLoc, 0x0, sizeof(au4ErrBitLoc));
	u4ErrorNumDebug = DRV_Reg32(ECC_DECENUM_REG32);
	u4ErrNum = DRV_Reg32(ECC_DECENUM_REG32) >> (u4SecIndex << 2);
	u4ErrNum &= 0xF;

	if (u4ErrNum) {
		if (0xF == u4ErrNum) {
			mtd->ecc_stats.failed++;
			bRet = false;
			//printk(KERN_ERR"UnCorrectable at PageAddr=%d\n", u4PageAddr);
		} else {
			for (i = 0; i < ((u4ErrNum + 1) >> 1); ++i) {
				au4ErrBitLoc[i] = DRV_Reg32(ECC_DECEL0_REG32 + i);
				u4ErrBitLoc1th = au4ErrBitLoc[i] & 0x1FFF;
				if (u4ErrBitLoc1th < 0x1000) {
					u4ErrByteLoc = u4ErrBitLoc1th / 8;
					u4BitOffset = u4ErrBitLoc1th % 8;
					pDataBuf[u4ErrByteLoc] = pDataBuf[u4ErrByteLoc] ^ (1 << u4BitOffset);
					mtd->ecc_stats.corrected++;
				} else {
					mtd->ecc_stats.failed++;
				}
				u4ErrBitLoc2nd = (au4ErrBitLoc[i] >> 16) & 0x1FFF;
				if (0 != u4ErrBitLoc2nd) {
					if (u4ErrBitLoc2nd < 0x1000) {
						u4ErrByteLoc = u4ErrBitLoc2nd / 8;
						u4BitOffset = u4ErrBitLoc2nd % 8;
						pDataBuf[u4ErrByteLoc] = pDataBuf[u4ErrByteLoc] ^ (1 << u4BitOffset);
						mtd->ecc_stats.corrected++;
					} else {
						mtd->ecc_stats.failed++;
						//printk(KERN_ERR"UnCorrectable High ErrLoc=%d\n", au4ErrBitLoc[i]);
					}
				}
			}
		}
		if (0 == (DRV_Reg16(ECC_DECFER_REG16) & (1 << u4SecIndex)))
			bRet = false;
	}
	return bRet;
}

static bool
mtk_nand_RFIFOValidSize(u16 u2Size)
{
	u32 timeout = 0xFFFF;
	while (FIFO_RD_REMAIN(DRV_Reg16(NFI_FIFOSTA_REG16)) < u2Size) {
		timeout--;
		if (0 == timeout)
			return false;
	}
	return true;
}

static bool
mtk_nand_WFIFOValidSize(u16 u2Size)
{
	u32 timeout = 0xFFFF;

	while (FIFO_WR_REMAIN(DRV_Reg16(NFI_FIFOSTA_REG16)) > u2Size) {
		timeout--;
		if (0 == timeout)
			return false;
	}
	return true;
}

static bool
mtk_nand_status_ready(u32 u4Status)
{
	u32 timeout = 0xFFFF;

	while ((DRV_Reg32(NFI_STA_REG32) & u4Status) != 0) {
		timeout--;
		if (0 == timeout)
			return false;
	}
	return true;
}

static bool
mtk_nand_reset(void)
{
	int timeout = 0xFFFF;
	if (DRV_Reg16(NFI_MASTERSTA_REG16)) {
		mb();
		DRV_WriteReg16(NFI_CON_REG16, CON_FIFO_FLUSH | CON_NFI_RST);
		while (DRV_Reg16(NFI_MASTERSTA_REG16)) {
			timeout--;
			if (!timeout)
				MSG(INIT, "Wait for NFI_MASTERSTA timeout\n");
		}
	}
	/* issue reset operation */
	mb();
	DRV_WriteReg16(NFI_CON_REG16, CON_FIFO_FLUSH | CON_NFI_RST);

	return mtk_nand_status_ready(STA_NFI_FSM_MASK | STA_NAND_BUSY) && mtk_nand_RFIFOValidSize(0) && mtk_nand_WFIFOValidSize(0);
}

static void
mtk_nand_set_mode(u16 u2OpMode)
{
	u16 u2Mode = DRV_Reg16(NFI_CNFG_REG16);
	u2Mode &= ~CNFG_OP_MODE_MASK;
	u2Mode |= u2OpMode;
	DRV_WriteReg16(NFI_CNFG_REG16, u2Mode);
}

static void
mtk_nand_set_autoformat(bool bEnable)
{
	if (bEnable)
		NFI_SET_REG16(NFI_CNFG_REG16, CNFG_AUTO_FMT_EN);
	else
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AUTO_FMT_EN);
}

static void
mtk_nand_configure_fdm(u16 u2FDMSize)
{
	NFI_CLN_REG16(NFI_PAGEFMT_REG16, PAGEFMT_FDM_MASK | PAGEFMT_FDM_ECC_MASK);
	NFI_SET_REG16(NFI_PAGEFMT_REG16, u2FDMSize << PAGEFMT_FDM_SHIFT);
	NFI_SET_REG16(NFI_PAGEFMT_REG16, u2FDMSize << PAGEFMT_FDM_ECC_SHIFT);
}

static void
mtk_nand_configure_lock(void)
{
	u32 u4WriteColNOB = 2;
	u32 u4WriteRowNOB = 3;
	u32 u4EraseColNOB = 0;
	u32 u4EraseRowNOB = 3;
	DRV_WriteReg16(NFI_LOCKANOB_REG16,
		(u4WriteColNOB << PROG_CADD_NOB_SHIFT) | (u4WriteRowNOB << PROG_RADD_NOB_SHIFT) | (u4EraseColNOB << ERASE_CADD_NOB_SHIFT) | (u4EraseRowNOB << ERASE_RADD_NOB_SHIFT));

	if (CHIPVER_ECO_1 == g_u4ChipVer) {
		int i;
		for (i = 0; i < 16; ++i) {
			DRV_WriteReg32(NFI_LOCK00ADD_REG32 + (i << 1), 0xFFFFFFFF);
			DRV_WriteReg32(NFI_LOCK00FMT_REG32 + (i << 1), 0xFFFFFFFF);
		}
		//DRV_WriteReg16(NFI_LOCKANOB_REG16, 0x0);
		DRV_WriteReg32(NFI_LOCKCON_REG32, 0xFFFFFFFF);
		DRV_WriteReg16(NFI_LOCK_REG16, NFI_LOCK_ON);
	}
}

static bool
mtk_nand_pio_ready(void)
{
	int count = 0;
	while (!(DRV_Reg16(NFI_PIO_DIRDY_REG16) & 1)) {
		count++;
		if (count > 0xffff) {
			printk("PIO_DIRDY timeout\n");
			return false;
		}
	}

	return true;
}

static bool
mtk_nand_set_command(u16 command)
{
	mb();
	DRV_WriteReg16(NFI_CMD_REG16, command);
	return mtk_nand_status_ready(STA_CMD_STATE);
}

static bool
mtk_nand_set_address(u32 u4ColAddr, u32 u4RowAddr, u16 u2ColNOB, u16 u2RowNOB)
{
	mb();
	DRV_WriteReg32(NFI_COLADDR_REG32, u4ColAddr);
	DRV_WriteReg32(NFI_ROWADDR_REG32, u4RowAddr);
	DRV_WriteReg16(NFI_ADDRNOB_REG16, u2ColNOB | (u2RowNOB << ADDR_ROW_NOB_SHIFT));
	return mtk_nand_status_ready(STA_ADDR_STATE);
}

static bool
mtk_nand_check_RW_count(u16 u2WriteSize)
{
	u32 timeout = 0xFFFF;
	u16 u2SecNum = u2WriteSize >> 9;

	while (ADDRCNTR_CNTR(DRV_Reg16(NFI_ADDRCNTR_REG16)) < u2SecNum) {
		timeout--;
		if (0 == timeout) {
			printk(KERN_INFO "[%s] timeout\n", __FUNCTION__);
			return false;
		}
	}
	return true;
}

static bool
mtk_nand_ready_for_read(struct nand_chip *nand, u32 u4RowAddr, u32 u4ColAddr, bool full, u8 * buf)
{
	/* Reset NFI HW internal state machine and flush NFI in/out FIFO */
	bool bRet = false;
	u16 sec_num = 1 << (nand->page_shift - 9);
	u32 col_addr = u4ColAddr;
	u32 colnob = 2, rownob = devinfo.addr_cycle - 2;
	if (nand->options & NAND_BUSWIDTH_16)
		col_addr /= 2;

	if (!mtk_nand_reset())
		goto cleanup;
	if (g_bHwEcc)	{
		NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
	} else	{
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
	}

	mtk_nand_set_mode(CNFG_OP_READ);
	NFI_SET_REG16(NFI_CNFG_REG16, CNFG_READ_EN);
	DRV_WriteReg16(NFI_CON_REG16, sec_num << CON_NFI_SEC_SHIFT);

	if (full) {
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);

		if (g_bHwEcc)
			NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
		else
			NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
	} else {
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);
	}

	mtk_nand_set_autoformat(full);
	if (full)
		if (g_bHwEcc)
			ECC_Decode_Start();
	if (!mtk_nand_set_command(NAND_CMD_READ0))
		goto cleanup;
	if (!mtk_nand_set_address(col_addr, u4RowAddr, colnob, rownob))
		goto cleanup;
	if (!mtk_nand_set_command(NAND_CMD_READSTART))
		goto cleanup;
	if (!mtk_nand_status_ready(STA_NAND_BUSY))
		goto cleanup;

	bRet = true;

cleanup:
	return bRet;
}

static bool
mtk_nand_ready_for_write(struct nand_chip *nand, u32 u4RowAddr, u32 col_addr, bool full, u8 * buf)
{
	bool bRet = false;
	u32 sec_num = 1 << (nand->page_shift - 9);
	u32 colnob = 2, rownob = devinfo.addr_cycle - 2;
	if (nand->options & NAND_BUSWIDTH_16)
		col_addr /= 2;

	/* Reset NFI HW internal state machine and flush NFI in/out FIFO */
	if (!mtk_nand_reset())
		return false;

	mtk_nand_set_mode(CNFG_OP_PRGM);

	NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_READ_EN);

	DRV_WriteReg16(NFI_CON_REG16, sec_num << CON_NFI_SEC_SHIFT);

	if (full) {
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);
		if (g_bHwEcc)
			NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
		else
			NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
	} else {
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);
	}

	mtk_nand_set_autoformat(full);

	if (full)
		if (g_bHwEcc)
			ECC_Encode_Start();

	if (!mtk_nand_set_command(NAND_CMD_SEQIN))
		goto cleanup;
	//1 FIXED ME: For Any Kind of AddrCycle
	if (!mtk_nand_set_address(col_addr, u4RowAddr, colnob, rownob))
		goto cleanup;

	if (!mtk_nand_status_ready(STA_NAND_BUSY))
		goto cleanup;

	bRet = true;

cleanup:
	return bRet;
}

static bool
mtk_nand_check_dececc_done(u32 u4SecNum)
{
	u32 timeout, dec_mask;

	timeout = 0xffff;
	dec_mask = (1 << u4SecNum) - 1;
	while ((dec_mask != DRV_Reg(ECC_DECDONE_REG16)) && timeout > 0)
		timeout--;
	if (timeout == 0) {
		MSG(VERIFY, "ECC_DECDONE: timeout\n");
		return false;
	}
	return true;
}

static bool
mtk_nand_mcu_read_data(u8 * buf, u32 length)
{
	int timeout = 0xffff;
	u32 i;
	u32 *buf32 = (u32 *) buf;
	if ((u32) buf % 4 || length % 4)
		NFI_SET_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);
	else
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);

	//DRV_WriteReg32(NFI_STRADDR_REG32, 0);
	mb();
	NFI_SET_REG16(NFI_CON_REG16, CON_NFI_BRD);

	if ((u32) buf % 4 || length % 4) {
		for (i = 0; (i < (length)) && (timeout > 0);) {
			if (DRV_Reg16(NFI_PIO_DIRDY_REG16) & 1) {
				*buf++ = (u8) DRV_Reg32(NFI_DATAR_REG32);
				i++;
			} else {
				timeout--;
			}
			if (0 == timeout) {
				printk(KERN_ERR "[%s] timeout\n", __FUNCTION__);
				dump_nfi();
				return false;
			}
		}
	} else {
		for (i = 0; (i < (length >> 2)) && (timeout > 0);) {
			if (DRV_Reg16(NFI_PIO_DIRDY_REG16) & 1) {
				*buf32++ = DRV_Reg32(NFI_DATAR_REG32);
				i++;
			} else {
				timeout--;
			}
			if (0 == timeout) {
				printk(KERN_ERR "[%s] timeout\n", __FUNCTION__);
				dump_nfi();
				return false;
			}
		}
	}
	return true;
}

static bool
mtk_nand_read_page_data(struct mtd_info *mtd, u8 * pDataBuf, u32 u4Size)
{
	return mtk_nand_mcu_read_data(pDataBuf, u4Size);
}

static bool
mtk_nand_mcu_write_data(struct mtd_info *mtd, const u8 * buf, u32 length)
{
	u32 timeout = 0xFFFF;
	u32 i;
	u32 *pBuf32;
	NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);
	mb();
	NFI_SET_REG16(NFI_CON_REG16, CON_NFI_BWR);
	pBuf32 = (u32 *) buf;

	if ((u32) buf % 4 || length % 4)
		NFI_SET_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);
	else
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);

	if ((u32) buf % 4 || length % 4) {
		for (i = 0; (i < (length)) && (timeout > 0);) {
			if (DRV_Reg16(NFI_PIO_DIRDY_REG16) & 1) {
				DRV_WriteReg32(NFI_DATAW_REG32, *buf++);
				i++;
			} else {
				timeout--;
			}
			if (0 == timeout) {
				printk(KERN_ERR "[%s] timeout\n", __FUNCTION__);
				dump_nfi();
				return false;
			}
		}
	} else {
		for (i = 0; (i < (length >> 2)) && (timeout > 0);) {
			if (DRV_Reg16(NFI_PIO_DIRDY_REG16) & 1) {
				DRV_WriteReg32(NFI_DATAW_REG32, *pBuf32++);
				i++;
			} else {
				timeout--;
			}
			if (0 == timeout) {
				printk(KERN_ERR "[%s] timeout\n", __FUNCTION__);
				dump_nfi();
				return false;
			}
		}
	}

	return true;
}

static bool
mtk_nand_write_page_data(struct mtd_info *mtd, u8 * buf, u32 size)
{
	return mtk_nand_mcu_write_data(mtd, buf, size);
}

static void
mtk_nand_read_fdm_data(u8 * pDataBuf, u32 u4SecNum)
{
	u32 i;
	u32 *pBuf32 = (u32 *) pDataBuf;

	if (pBuf32) {
		for (i = 0; i < u4SecNum; ++i) {
			*pBuf32++ = DRV_Reg32(NFI_FDM0L_REG32 + (i << 1));
			*pBuf32++ = DRV_Reg32(NFI_FDM0M_REG32 + (i << 1));
		}
	}
}

static u8 fdm_buf[64];
static void
mtk_nand_write_fdm_data(struct nand_chip *chip, u8 * pDataBuf, u32 u4SecNum)
{
	u32 i, j;
	u8 checksum = 0;
	bool empty = true;
	struct nand_oobfree *free_entry;
	u32 *pBuf32;

	memcpy(fdm_buf, pDataBuf, u4SecNum * 8);

	free_entry = chip->ecc.layout->oobfree;
	for (i = 0; i < MTD_MAX_OOBFREE_ENTRIES && free_entry[i].length; i++) {
		for (j = 0; j < free_entry[i].length; j++) {
			if (pDataBuf[free_entry[i].offset + j] != 0xFF)
				empty = false;
			checksum ^= pDataBuf[free_entry[i].offset + j];
		}
	}

	if (!empty) {
		fdm_buf[free_entry[i - 1].offset + free_entry[i - 1].length] = checksum;
	}

	pBuf32 = (u32 *) fdm_buf;
	for (i = 0; i < u4SecNum; ++i) {
		DRV_WriteReg32(NFI_FDM0L_REG32 + (i << 1), *pBuf32++);
		DRV_WriteReg32(NFI_FDM0M_REG32 + (i << 1), *pBuf32++);
	}
}

static void
mtk_nand_stop_read(void)
{
	NFI_CLN_REG16(NFI_CON_REG16, CON_NFI_BRD);
	mtk_nand_reset();
	if (g_bHwEcc)
		ECC_Decode_End();
	DRV_WriteReg16(NFI_INTR_EN_REG16, 0);
}

static void
mtk_nand_stop_write(void)
{
	NFI_CLN_REG16(NFI_CON_REG16, CON_NFI_BWR);
	if (g_bHwEcc)
		ECC_Encode_End();
	DRV_WriteReg16(NFI_INTR_EN_REG16, 0);
}

bool
mtk_nand_exec_read_page(struct mtd_info *mtd, u32 u4RowAddr, u32 u4PageSize, u8 * pPageBuf, u8 * pFDMBuf)
{
	u8 *buf;
	bool bRet = true;
	struct nand_chip *nand = mtd->priv;
	u32 u4SecNum = u4PageSize >> 9;

	if (((u32) pPageBuf % 16) && local_buffer_16_align)
		buf = local_buffer_16_align;
	else
		buf = pPageBuf;
	if (mtk_nand_ready_for_read(nand, u4RowAddr, 0, true, buf)) {
		int j;
		for (j = 0 ; j < u4SecNum; j++) {
			if (!mtk_nand_read_page_data(mtd, buf+j*512, 512))
				bRet = false;
			if(g_bHwEcc && !mtk_nand_check_dececc_done(j+1))
				bRet = false;
			if(g_bHwEcc && !mtk_nand_check_bch_error(mtd, buf+j*512, j, u4RowAddr))
				bRet = false;
		}
		if (!mtk_nand_status_ready(STA_NAND_BUSY))
			bRet = false;

		mtk_nand_read_fdm_data(pFDMBuf, u4SecNum);
		mtk_nand_stop_read();
	}

	if (buf == local_buffer_16_align)
		memcpy(pPageBuf, buf, u4PageSize);

	return bRet;
}

int
mtk_nand_exec_write_page(struct mtd_info *mtd, u32 u4RowAddr, u32 u4PageSize, u8 * pPageBuf, u8 * pFDMBuf)
{
	struct nand_chip *chip = mtd->priv;
	u32 u4SecNum = u4PageSize >> 9;
	u8 *buf;
	u8 status;

	MSG(WRITE, "mtk_nand_exec_write_page, page: 0x%x\n", u4RowAddr);

	if (((u32) pPageBuf % 16) && local_buffer_16_align) {
		printk(KERN_INFO "Data buffer not 16 bytes aligned: %p\n", pPageBuf);
		memcpy(local_buffer_16_align, pPageBuf, mtd->writesize);
		buf = local_buffer_16_align;
	} else
		buf = pPageBuf;

	if (mtk_nand_ready_for_write(chip, u4RowAddr, 0, true, buf)) {
		mtk_nand_write_fdm_data(chip, pFDMBuf, u4SecNum);
		(void)mtk_nand_write_page_data(mtd, buf, u4PageSize);
		(void)mtk_nand_check_RW_count(u4PageSize);
		mtk_nand_stop_write();
		(void)mtk_nand_set_command(NAND_CMD_PAGEPROG);
		while (DRV_Reg32(NFI_STA_REG32) & STA_NAND_BUSY) ;
	}

	status = chip->waitfunc(mtd, chip);
	if (status & NAND_STATUS_FAIL)
		return -EIO;
	return 0;
}

static int
get_start_end_block(struct mtd_info *mtd, int block, int *start_blk, int *end_blk)
{
	struct nand_chip *chip = mtd->priv;
	int i;

	*start_blk = 0;
        for (i = 0; i <= part_num; i++)
        {
		if (i == part_num)
		{
			// try the last reset partition
			*end_blk = (chip->chipsize >> chip->phys_erase_shift) - 1;
			if (*start_blk <= *end_blk)
			{
				if ((block >= *start_blk) && (block <= *end_blk))
					break;
			}
		}
		// skip All partition entry
		else if (g_pasStatic_Partition[i].size == MTDPART_SIZ_FULL)
		{
			continue;
		}
                *end_blk = *start_blk + (g_pasStatic_Partition[i].size >> chip->phys_erase_shift) - 1;
                if ((block >= *start_blk) && (block <= *end_blk))
                        break;
                *start_blk = *end_blk + 1;
        }
        if (*start_blk > *end_blk)
	{
                return -1;
	}
	return 0;
}

static int
block_remap(struct mtd_info *mtd, int block)
{
	struct nand_chip *chip = mtd->priv;
	int start_blk, end_blk;
	int j, block_offset;
	int bad_block = 0;

	if (chip->bbt == NULL) {
		printk("ERROR!! no bbt table for block_remap\n");
		return -1;
	}

	if (get_start_end_block(mtd, block, &start_blk, &end_blk) < 0) {
		printk("ERROR!! can not find start_blk and end_blk\n");
		return -1;
	}

	block_offset = block - start_blk;
	for (j = start_blk; j <= end_blk;j++) {
		if (((chip->bbt[j >> 2] >> ((j<<1) & 0x6)) & 0x3) == 0x0) {
			if (!block_offset)
				break;
			block_offset--;
		} else {
			bad_block++;
		}
	}
	if (j <= end_blk) {
		return j;
	} else {
		// remap to the bad block
		for (j = end_blk; bad_block > 0; j--)
		{
			if (((chip->bbt[j >> 2] >> ((j<<1) & 0x6)) & 0x3) != 0x0)
			{
				bad_block--;
				if (bad_block <= block_offset)
					return j;
			}
		}
	}

	printk("Error!! block_remap error\n");
	return -1;
}

int
check_block_remap(struct mtd_info *mtd, int block)
{
	if (shift_on_bbt)
		return  block_remap(mtd, block);
	else
		return block;
}
EXPORT_SYMBOL(check_block_remap);


static int
write_next_on_fail(struct mtd_info *mtd, char *write_buf, int page, int * to_blk)
{
	struct nand_chip *chip = mtd->priv;
	int i, j, to_page = 0, first_page;
	char *buf, *oob;
	int start_blk = 0, end_blk;
	int mapped_block;
	int page_per_block_bit = chip->phys_erase_shift - chip->page_shift;
	int block = page >> page_per_block_bit;

	// find next available block in the same MTD partition 
	mapped_block = block_remap(mtd, block);
	if (mapped_block == -1)
		return NAND_STATUS_FAIL;

	get_start_end_block(mtd, block, &start_blk, &end_blk);

	buf = kzalloc(mtd->writesize + mtd->oobsize, GFP_KERNEL | GFP_DMA);
	if (buf == NULL)
		return -1;

	oob = buf + mtd->writesize;
	for ((*to_blk) = block + 1; (*to_blk) <= end_blk ; (*to_blk)++)	{
		if (nand_bbt_get(mtd, (*to_blk) << page_per_block_bit) == 0) {
			int status;
			status = mtk_nand_erase_hw(mtd, (*to_blk) << page_per_block_bit);
			if (status & NAND_STATUS_FAIL)	{
				mtk_nand_block_markbad_hw(mtd, (*to_blk) << chip->phys_erase_shift);
				nand_bbt_set(mtd, (*to_blk) << page_per_block_bit, 0x3);
			} else {
				/* good block */
				to_page = (*to_blk) << page_per_block_bit;
				break;
			}
		}
	}

	if (!to_page) {
		kfree(buf);
		return -1;
	}

	first_page = (page >> page_per_block_bit) << page_per_block_bit;
	for (i = 0; i < (1 << page_per_block_bit); i++) {
		if ((first_page + i) != page) {
			mtk_nand_read_oob_hw(mtd, chip, (first_page+i));
			for (j = 0; j < mtd->oobsize; j++)
				if (chip->oob_poi[j] != (unsigned char)0xff)
					break;
			if (j < mtd->oobsize)	{
				mtk_nand_exec_read_page(mtd, (first_page+i), mtd->writesize, buf, oob);
				memset(oob, 0xff, mtd->oobsize);
				if (mtk_nand_exec_write_page(mtd, to_page + i, mtd->writesize, (u8 *)buf, oob) != 0) {
					int ret, new_blk = 0;
					nand_bbt_set(mtd, to_page, 0x3);
					ret =  write_next_on_fail(mtd, buf, to_page + i, &new_blk);
					if (ret) {
						kfree(buf);
						mtk_nand_block_markbad_hw(mtd, to_page << chip->page_shift);
						return ret;
					}
					mtk_nand_block_markbad_hw(mtd, to_page << chip->page_shift);
					*to_blk = new_blk;
					to_page = ((*to_blk) <<  page_per_block_bit);
				}
			}
		} else {
			memset(chip->oob_poi, 0xff, mtd->oobsize);
			if (mtk_nand_exec_write_page(mtd, to_page + i, mtd->writesize, (u8 *)write_buf, chip->oob_poi) != 0) {
				int ret, new_blk = 0;
				nand_bbt_set(mtd, to_page, 0x3);
				ret =  write_next_on_fail(mtd, write_buf, to_page + i, &new_blk);
				if (ret) {
					kfree(buf);
					mtk_nand_block_markbad_hw(mtd, to_page << chip->page_shift);
					return ret;
				}
				mtk_nand_block_markbad_hw(mtd, to_page << chip->page_shift);
				*to_blk = new_blk;
				to_page = ((*to_blk) <<  page_per_block_bit);
			}
		}
	}

	kfree(buf);

	return 0;
}

static int
mtk_nand_write_page(struct mtd_info *mtd, struct nand_chip *chip, uint32_t offset,
		int data_len, const u8 * buf, int oob_required, int page, int cached, int raw)
{
	int page_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);
	int block = page / page_per_block;
	u16 page_in_block = page % page_per_block;
	int mapped_block = block;

#if defined(MTK_NAND_BMT)
	mapped_block = get_mapping_block_index(block);
	// write bad index into oob
	if (mapped_block != block)
		set_bad_index_to_oob(chip->oob_poi, block);
	else
		set_bad_index_to_oob(chip->oob_poi, FAKE_INDEX);
#else
	if (shift_on_bbt) {
		mapped_block = block_remap(mtd, block);
		if (mapped_block == -1)
			return NAND_STATUS_FAIL;
		if (nand_bbt_get(mtd, mapped_block << (chip->phys_erase_shift - chip->page_shift)) != 0x0)
			return NAND_STATUS_FAIL;
	}
#endif
	do {
		if (mtk_nand_exec_write_page(mtd, page_in_block + mapped_block * page_per_block, mtd->writesize, (u8 *)buf, chip->oob_poi)) {
			MSG(INIT, "write fail at block: 0x%x, page: 0x%x\n", mapped_block, page_in_block);
#if defined(MTK_NAND_BMT)
			if (update_bmt((page_in_block + mapped_block * page_per_block) << chip->page_shift, UPDATE_WRITE_FAIL, (u8 *) buf, chip->oob_poi)) {
				MSG(INIT, "Update BMT success\n");
				return 0;
			} else {
				MSG(INIT, "Update BMT fail\n");
				return -EIO;
			}
#else
			{
				int new_blk;
				nand_bbt_set(mtd, page_in_block + mapped_block * page_per_block, 0x3);
				if (write_next_on_fail(mtd, (char *)buf, page_in_block + mapped_block * page_per_block, &new_blk) != 0)
				{
				mtk_nand_block_markbad_hw(mtd, (page_in_block + mapped_block * page_per_block) << chip->page_shift);
				return NAND_STATUS_FAIL;
				}
				mtk_nand_block_markbad_hw(mtd, (page_in_block + mapped_block * page_per_block) << chip->page_shift);
				break;
			}
#endif
		} else
			break;
	} while(1);

	return 0;
}

static void
mtk_nand_command_bp(struct mtd_info *mtd, unsigned int command, int column, int page_addr)
{
	struct nand_chip *nand = mtd->priv;

	switch (command) {
	case NAND_CMD_SEQIN:
		memset(g_kCMD.au1OOB, 0xFF, sizeof(g_kCMD.au1OOB));
		g_kCMD.pDataBuf = NULL;
		g_kCMD.u4RowAddr = page_addr;
		g_kCMD.u4ColAddr = column;
		break;

	case NAND_CMD_PAGEPROG:
		if (g_kCMD.pDataBuf || (0xFF != g_kCMD.au1OOB[nand_badblock_offset])) {
			u8 *pDataBuf = g_kCMD.pDataBuf ? g_kCMD.pDataBuf : nand->buffers->databuf;
			mtk_nand_exec_write_page(mtd, g_kCMD.u4RowAddr, mtd->writesize, pDataBuf, g_kCMD.au1OOB);
			g_kCMD.u4RowAddr = (u32) - 1;
			g_kCMD.u4OOBRowAddr = (u32) - 1;
		}
		break;

	case NAND_CMD_READOOB:
		g_kCMD.u4RowAddr = page_addr;
		g_kCMD.u4ColAddr = column + mtd->writesize;
		break;

	case NAND_CMD_READ0:
		g_kCMD.u4RowAddr = page_addr;
		g_kCMD.u4ColAddr = column;
		break;

	case NAND_CMD_ERASE1:
		nand->state=FL_ERASING;
		(void)mtk_nand_reset();
		mtk_nand_set_mode(CNFG_OP_ERASE);
		(void)mtk_nand_set_command(NAND_CMD_ERASE1);
		(void)mtk_nand_set_address(0, page_addr, 0, devinfo.addr_cycle - 2);
		break;

	case NAND_CMD_ERASE2:
		(void)mtk_nand_set_command(NAND_CMD_ERASE2);
		while (DRV_Reg32(NFI_STA_REG32) & STA_NAND_BUSY)
			;
		break;

	case NAND_CMD_STATUS:
		(void)mtk_nand_reset();
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);
		mtk_nand_set_mode(CNFG_OP_SRD);
		mtk_nand_set_mode(CNFG_READ_EN);
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
		(void)mtk_nand_set_command(NAND_CMD_STATUS);
		NFI_CLN_REG16(NFI_CON_REG16, CON_NFI_NOB_MASK);
		mb();
		DRV_WriteReg16(NFI_CON_REG16, CON_NFI_SRD | (1 << CON_NFI_NOB_SHIFT));
		g_bcmdstatus = true;
		break;

	case NAND_CMD_RESET:
		(void)mtk_nand_reset();
		DRV_WriteReg16(NFI_INTR_EN_REG16, INTR_RST_DONE_EN);
		(void)mtk_nand_set_command(NAND_CMD_RESET);
		DRV_WriteReg16(NFI_BASE+0x44, 0xF1);
		while(!(DRV_Reg16(NFI_INTR_REG16)&INTR_RST_DONE_EN))
			;
		break;

	case NAND_CMD_READID:
		mtk_nand_reset();
		/* Disable HW ECC */
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);
		NFI_SET_REG16(NFI_CNFG_REG16, CNFG_READ_EN | CNFG_BYTE_RW);
		(void)mtk_nand_reset();
		mb();
		mtk_nand_set_mode(CNFG_OP_SRD);
		(void)mtk_nand_set_command(NAND_CMD_READID);
		(void)mtk_nand_set_address(0, 0, 1, 0);
		DRV_WriteReg16(NFI_CON_REG16, CON_NFI_SRD);
		while (DRV_Reg32(NFI_STA_REG32) & STA_DATAR_STATE)
			;
		break;

	default:
		BUG();
		break;
	}
}

static void
mtk_nand_select_chip(struct mtd_info *mtd, int chip)
{
	if ((chip == -1) && (false == g_bInitDone)) {
		struct nand_chip *nand = mtd->priv;
		struct mtk_nand_host *host = nand->priv;
		struct mtk_nand_host_hw *hw = host->hw;
		u32 spare_per_sector = mtd->oobsize / (mtd->writesize / 512);
		u32 ecc_bit = 4;
		u32 spare_bit = PAGEFMT_SPARE_16;

		if (spare_per_sector >= 28) {
			spare_bit = PAGEFMT_SPARE_28;
			ecc_bit = 12;
			spare_per_sector = 28;
		} else if (spare_per_sector >= 27) {
			spare_bit = PAGEFMT_SPARE_27;
			ecc_bit = 8;
			spare_per_sector = 27;
		} else if (spare_per_sector >= 26) {
			spare_bit = PAGEFMT_SPARE_26;
			ecc_bit = 8;
			spare_per_sector = 26;
		} else if (spare_per_sector >= 16) {
			spare_bit = PAGEFMT_SPARE_16;
			ecc_bit = 4;
			spare_per_sector = 16;
		} else {
			MSG(INIT, "[NAND]: NFI not support oobsize: %x\n", spare_per_sector);
			ASSERT(0);
		}
		mtd->oobsize = spare_per_sector*(mtd->writesize/512);
		MSG(INIT, "[NAND]select ecc bit:%d, sparesize :%d spare_per_sector=%d\n",ecc_bit,mtd->oobsize,spare_per_sector);
		/* Setup PageFormat */
		if (4096 == mtd->writesize) {
			NFI_SET_REG16(NFI_PAGEFMT_REG16, (spare_bit << PAGEFMT_SPARE_SHIFT) | PAGEFMT_4K);
			nand->cmdfunc = mtk_nand_command_bp;
		} else if (2048 == mtd->writesize) {
			NFI_SET_REG16(NFI_PAGEFMT_REG16, (spare_bit << PAGEFMT_SPARE_SHIFT) | PAGEFMT_2K);
			nand->cmdfunc = mtk_nand_command_bp;
		}
		ECC_Config(hw,ecc_bit);
		g_bInitDone = true;
	}
	switch (chip) {
	case -1:
		break;
	case 0:
	case 1:
		/*  Jun Shen, 2011.04.13  */
		/* Note: MT6577 EVB NAND  is mounted on CS0, but FPGA is CS1  */
		DRV_WriteReg16(NFI_CSEL_REG16, chip);
		/*  Jun Shen, 2011.04.13 */
		break;
	}
}

static uint8_t
mtk_nand_read_byte(struct mtd_info *mtd)
{
	uint8_t retval = 0;

	if (!mtk_nand_pio_ready()) {
		printk("pio ready timeout\n");
		retval = false;
	}

	if (g_bcmdstatus) {
		retval = DRV_Reg8(NFI_DATAR_REG32);
		NFI_CLN_REG16(NFI_CON_REG16, CON_NFI_NOB_MASK);
		mtk_nand_reset();
		if (g_bHwEcc) {
			NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
		} else {
			NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
		}
		g_bcmdstatus = false;
	} else
		retval = DRV_Reg8(NFI_DATAR_REG32);

	return retval;
}

static void
mtk_nand_read_buf(struct mtd_info *mtd, uint8_t * buf, int len)
{
	struct nand_chip *nand = (struct nand_chip *)mtd->priv;
	struct NAND_CMD *pkCMD = &g_kCMD;
	u32 u4ColAddr = pkCMD->u4ColAddr;
	u32 u4PageSize = mtd->writesize;

	if (u4ColAddr < u4PageSize) {
		if ((u4ColAddr == 0) && (len >= u4PageSize)) {
			mtk_nand_exec_read_page(mtd, pkCMD->u4RowAddr, u4PageSize, buf, pkCMD->au1OOB);
			if (len > u4PageSize) {
				u32 u4Size = min(len - u4PageSize, sizeof(pkCMD->au1OOB));
				memcpy(buf + u4PageSize, pkCMD->au1OOB, u4Size);
			}
		} else {
			mtk_nand_exec_read_page(mtd, pkCMD->u4RowAddr, u4PageSize, nand->buffers->databuf, pkCMD->au1OOB);
			memcpy(buf, nand->buffers->databuf + u4ColAddr, len);
		}
		pkCMD->u4OOBRowAddr = pkCMD->u4RowAddr;
	} else {
		u32 u4Offset = u4ColAddr - u4PageSize;
		u32 u4Size = min(len - u4Offset, sizeof(pkCMD->au1OOB));
		if (pkCMD->u4OOBRowAddr != pkCMD->u4RowAddr) {
			mtk_nand_exec_read_page(mtd, pkCMD->u4RowAddr, u4PageSize, nand->buffers->databuf, pkCMD->au1OOB);
			pkCMD->u4OOBRowAddr = pkCMD->u4RowAddr;
		}
		memcpy(buf, pkCMD->au1OOB + u4Offset, u4Size);
	}
	pkCMD->u4ColAddr += len;
}

static void
mtk_nand_write_buf(struct mtd_info *mtd, const uint8_t * buf, int len)
{
	struct NAND_CMD *pkCMD = &g_kCMD;
	u32 u4ColAddr = pkCMD->u4ColAddr;
	u32 u4PageSize = mtd->writesize;
	int i4Size, i;

	if (u4ColAddr >= u4PageSize) {
		u32 u4Offset = u4ColAddr - u4PageSize;
		u8 *pOOB = pkCMD->au1OOB + u4Offset;
		i4Size = min(len, (int)(sizeof(pkCMD->au1OOB) - u4Offset));
		for (i = 0; i < i4Size; i++) {
			pOOB[i] &= buf[i];
		}
	} else {
		pkCMD->pDataBuf = (u8 *) buf;
	}

	pkCMD->u4ColAddr += len;
}

static int
mtk_nand_write_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip, const uint8_t * buf, int oob_required)
{
	mtk_nand_write_buf(mtd, buf, mtd->writesize);
	mtk_nand_write_buf(mtd, chip->oob_poi, mtd->oobsize);
	return 0;
}

static int
mtk_nand_read_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip, uint8_t * buf, int oob_required, int page)
{
	struct NAND_CMD *pkCMD = &g_kCMD;
	u32 u4ColAddr = pkCMD->u4ColAddr;
	u32 u4PageSize = mtd->writesize;

	if (u4ColAddr == 0) {
		mtk_nand_exec_read_page(mtd, pkCMD->u4RowAddr, u4PageSize, buf, chip->oob_poi);
		pkCMD->u4ColAddr += u4PageSize + mtd->oobsize;
	}

	return 0;
}

static int
mtk_nand_read_page(struct mtd_info *mtd, struct nand_chip *chip, u8 * buf, int page)
{
	int page_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);
	int block = page / page_per_block;
	u16 page_in_block = page % page_per_block;
	int mapped_block = block;

#if defined (MTK_NAND_BMT)
	mapped_block = get_mapping_block_index(block);
	if (mtk_nand_exec_read_page(mtd, page_in_block + mapped_block * page_per_block,
			mtd->writesize, buf, chip->oob_poi))
		return 0;
#else
	if (shift_on_bbt) {
		mapped_block = block_remap(mtd, block);
		if (mapped_block == -1)
			return NAND_STATUS_FAIL;
		if (nand_bbt_get(mtd, mapped_block << (chip->phys_erase_shift - chip->page_shift)) != 0x0)
			return NAND_STATUS_FAIL;
	}

	if (mtk_nand_exec_read_page(mtd, page_in_block + mapped_block * page_per_block, mtd->writesize, buf, chip->oob_poi))
		return 0;
	else
		return -EIO;
#endif
}

int
mtk_nand_erase_hw(struct mtd_info *mtd, int page)
{
	struct nand_chip *chip = (struct nand_chip *)mtd->priv;

	chip->erase_cmd(mtd, page);

	return chip->waitfunc(mtd, chip);
}

static int
mtk_nand_erase(struct mtd_info *mtd, int page)
{
	// get mapping 
	struct nand_chip *chip = mtd->priv;
	int page_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);
	int page_in_block = page % page_per_block;
	int block = page / page_per_block;
	int mapped_block = block;

#if defined(MTK_NAND_BMT)    
	mapped_block = get_mapping_block_index(block);
#else
	if (shift_on_bbt) {
		mapped_block = block_remap(mtd, block);
		if (mapped_block == -1)
			return NAND_STATUS_FAIL;
		if (nand_bbt_get(mtd, mapped_block << (chip->phys_erase_shift - chip->page_shift)) != 0x0)
			return NAND_STATUS_FAIL;
	}
#endif

	do {
		int status = mtk_nand_erase_hw(mtd, page_in_block + page_per_block * mapped_block);

		if (status & NAND_STATUS_FAIL) {
#if defined (MTK_NAND_BMT)    	
			if (update_bmt( (page_in_block + mapped_block * page_per_block) << chip->page_shift,
					UPDATE_ERASE_FAIL, NULL, NULL))
			{
				MSG(INIT, "Erase fail at block: 0x%x, update BMT success\n", mapped_block);
				return 0;
			} else {
				MSG(INIT, "Erase fail at block: 0x%x, update BMT fail\n", mapped_block);
				return NAND_STATUS_FAIL;
			}
#else
			mtk_nand_block_markbad_hw(mtd, (page_in_block + mapped_block * page_per_block) << chip->page_shift);
			nand_bbt_set(mtd, page_in_block + mapped_block * page_per_block, 0x3);
			if (shift_on_bbt) {
				mapped_block = block_remap(mtd, block);
				if (mapped_block == -1)
					return NAND_STATUS_FAIL;
				if (nand_bbt_get(mtd, mapped_block << (chip->phys_erase_shift - chip->page_shift)) != 0x0)
					return NAND_STATUS_FAIL;
			} else
				return NAND_STATUS_FAIL;
#endif
		} else
			break;
	} while(1);

	return 0;
}

static int
mtk_nand_read_oob_raw(struct mtd_info *mtd, uint8_t * buf, int page_addr, int len)
{
	struct nand_chip *chip = (struct nand_chip *)mtd->priv;
	u32 col_addr = 0;
	u32 sector = 0;
	int res = 0;
	u32 colnob = 2, rawnob = devinfo.addr_cycle - 2;
	int randomread = 0;
	int read_len = 0;
	int sec_num = 1<<(chip->page_shift-9);
	int spare_per_sector = mtd->oobsize/sec_num;

	if (len >  NAND_MAX_OOBSIZE || len % OOB_AVAI_PER_SECTOR || !buf) {
		printk(KERN_WARNING "[%s] invalid parameter, len: %d, buf: %p\n", __FUNCTION__, len, buf);
		return -EINVAL;
	}
	if (len > spare_per_sector)
		randomread = 1;
	if (!randomread || !(devinfo.advancedmode & RAMDOM_READ)) {
		while (len > 0) {
			read_len = min(len, spare_per_sector);
			col_addr = NAND_SECTOR_SIZE + sector * (NAND_SECTOR_SIZE + spare_per_sector); // TODO: Fix this hard-code 16
			if (!mtk_nand_ready_for_read(chip, page_addr, col_addr, false, NULL)) {
				printk(KERN_WARNING "mtk_nand_ready_for_read return failed\n");
				res = -EIO;
				goto error;
			}
			if (!mtk_nand_mcu_read_data(buf + spare_per_sector * sector, read_len)) {
				printk(KERN_WARNING "mtk_nand_mcu_read_data return failed\n");
				res = -EIO;
				goto error;
			}
			mtk_nand_check_RW_count(read_len);
			mtk_nand_stop_read();
			sector++;
			len -= read_len;
		}
	} else {
		col_addr = NAND_SECTOR_SIZE;
		if (chip->options & NAND_BUSWIDTH_16)
			col_addr /= 2;
		if (!mtk_nand_reset())
			goto error;
		mtk_nand_set_mode(0x6000);
		NFI_SET_REG16(NFI_CNFG_REG16, CNFG_READ_EN);
		DRV_WriteReg16(NFI_CON_REG16, 4 << CON_NFI_SEC_SHIFT);

		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);

		mtk_nand_set_autoformat(false);

		if (!mtk_nand_set_command(NAND_CMD_READ0))
			goto error;
		//1 FIXED ME: For Any Kind of AddrCycle
		if (!mtk_nand_set_address(col_addr, page_addr, colnob, rawnob))
			goto error;
		if (!mtk_nand_set_command(NAND_CMD_READSTART))
			goto error;
		if (!mtk_nand_status_ready(STA_NAND_BUSY))
			goto error;
		read_len = min(len, spare_per_sector);
		if (!mtk_nand_mcu_read_data(buf + spare_per_sector * sector, read_len)) {
			printk(KERN_WARNING "mtk_nand_mcu_read_data return failed first 16\n");
			res = -EIO;
			goto error;
		}
		sector++;
		len -= read_len;
		mtk_nand_stop_read();
		while (len > 0) {
			read_len = min(len,  spare_per_sector);
			if (!mtk_nand_set_command(0x05))
				goto error;
			col_addr = NAND_SECTOR_SIZE + sector * (NAND_SECTOR_SIZE + spare_per_sector);
			if (chip->options & NAND_BUSWIDTH_16)
				col_addr /= 2;
			DRV_WriteReg32(NFI_COLADDR_REG32, col_addr);
			DRV_WriteReg16(NFI_ADDRNOB_REG16, 2);
			DRV_WriteReg16(NFI_CON_REG16, 4 << CON_NFI_SEC_SHIFT);
			if (!mtk_nand_status_ready(STA_ADDR_STATE))
				goto error;
			if (!mtk_nand_set_command(0xE0))
				goto error;
			if (!mtk_nand_status_ready(STA_NAND_BUSY))
				goto error;
			if (!mtk_nand_mcu_read_data(buf + spare_per_sector * sector, read_len)) {
				printk(KERN_WARNING "mtk_nand_mcu_read_data return failed first 16\n");
				res = -EIO;
				goto error;
			}
			mtk_nand_stop_read();
			sector++;
			len -= read_len;
		}
	}
error:
	NFI_CLN_REG16(NFI_CON_REG16, CON_NFI_BRD);
	return res;
}

static int
mtk_nand_write_oob_raw(struct mtd_info *mtd, const uint8_t * buf, int page_addr, int len)
{
	struct nand_chip *chip = mtd->priv;
	u32 col_addr = 0;
	u32 sector = 0;
	int write_len = 0;
	int status;
	int sec_num = 1<<(chip->page_shift-9);
	int spare_per_sector = mtd->oobsize/sec_num;

	if (len >  NAND_MAX_OOBSIZE || len % OOB_AVAI_PER_SECTOR || !buf) {
		printk(KERN_WARNING "[%s] invalid parameter, len: %d, buf: %p\n", __FUNCTION__, len, buf);
		return -EINVAL;
	}

	while (len > 0) {
		write_len = min(len,  spare_per_sector);
		col_addr = sector * (NAND_SECTOR_SIZE +  spare_per_sector) + NAND_SECTOR_SIZE;
		if (!mtk_nand_ready_for_write(chip, page_addr, col_addr, false, NULL))
			return -EIO;
		if (!mtk_nand_mcu_write_data(mtd, buf + sector * spare_per_sector, write_len))
			return -EIO;
		(void)mtk_nand_check_RW_count(write_len);
		NFI_CLN_REG16(NFI_CON_REG16, CON_NFI_BWR);
		(void)mtk_nand_set_command(NAND_CMD_PAGEPROG);
		while (DRV_Reg32(NFI_STA_REG32) & STA_NAND_BUSY)
			;
		status = chip->waitfunc(mtd, chip);
		if (status & NAND_STATUS_FAIL) {
			printk(KERN_INFO "status: %d\n", status);
			return -EIO;
		}
		len -= write_len;
		sector++;
	}

	return 0;
}

static int
mtk_nand_write_oob_hw(struct mtd_info *mtd, struct nand_chip *chip, int page)
{
	int i, iter;
	int sec_num = 1<<(chip->page_shift-9);
	int spare_per_sector = mtd->oobsize/sec_num;

	memcpy(local_oob_buf, chip->oob_poi, mtd->oobsize);

	// copy ecc data
	for (i = 0; i < chip->ecc.layout->eccbytes; i++) {
		iter = (i / (spare_per_sector-OOB_AVAI_PER_SECTOR)) *  spare_per_sector + OOB_AVAI_PER_SECTOR + i % (spare_per_sector-OOB_AVAI_PER_SECTOR);
		local_oob_buf[iter] = chip->oob_poi[chip->ecc.layout->eccpos[i]];
	}

	// copy FDM data
	for (i = 0; i < sec_num; i++)
		memcpy(&local_oob_buf[i * spare_per_sector], &chip->oob_poi[i * OOB_AVAI_PER_SECTOR], OOB_AVAI_PER_SECTOR);

	return mtk_nand_write_oob_raw(mtd, local_oob_buf, page, mtd->oobsize);
}

static int mtk_nand_write_oob(struct mtd_info *mtd, struct nand_chip *chip, int page)
{
	int page_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);
	int block = page / page_per_block;
	u16 page_in_block = page % page_per_block;
	int mapped_block = block;

#if defined(MTK_NAND_BMT)
	mapped_block = get_mapping_block_index(block);
	// write bad index into oob
	if (mapped_block != block)
		set_bad_index_to_oob(chip->oob_poi, block);
	else
		set_bad_index_to_oob(chip->oob_poi, FAKE_INDEX);
#else
	if (shift_on_bbt)
	{
		mapped_block = block_remap(mtd, block);
		if (mapped_block == -1)
			return NAND_STATUS_FAIL;
		if (nand_bbt_get(mtd, mapped_block << (chip->phys_erase_shift - chip->page_shift)) != 0x0)
			return NAND_STATUS_FAIL;
	}
#endif
	do {
		if (mtk_nand_write_oob_hw(mtd, chip, page_in_block + mapped_block * page_per_block /* page */)) {
			MSG(INIT, "write oob fail at block: 0x%x, page: 0x%x\n", mapped_block, page_in_block);
#if defined(MTK_NAND_BMT)      
			if (update_bmt((page_in_block + mapped_block * page_per_block) << chip->page_shift,
					UPDATE_WRITE_FAIL, NULL, chip->oob_poi))
			{
				MSG(INIT, "Update BMT success\n");
				return 0;
			} else {
				MSG(INIT, "Update BMT fail\n");
				return -EIO;
			}
#else
			mtk_nand_block_markbad_hw(mtd, (page_in_block + mapped_block * page_per_block) << chip->page_shift);
			nand_bbt_set(mtd, page_in_block + mapped_block * page_per_block, 0x3);
			if (shift_on_bbt) {
				mapped_block = block_remap(mtd, mapped_block);
				if (mapped_block == -1)
					return NAND_STATUS_FAIL;
				if (nand_bbt_get(mtd, mapped_block << (chip->phys_erase_shift - chip->page_shift)) != 0x0)
					return NAND_STATUS_FAIL;
			} else {
				return NAND_STATUS_FAIL;
			}
#endif
		} else
			break;
	} while (1);

	return 0;
}

int
mtk_nand_block_markbad_hw(struct mtd_info *mtd, loff_t offset)
{
	struct nand_chip *chip = mtd->priv;
	int block = (int)offset >> chip->phys_erase_shift;
	int page = block * (1 << (chip->phys_erase_shift - chip->page_shift));
	u8 buf[8];

	memset(buf, 0xFF, 8);
	buf[0] = 0;
	return  mtk_nand_write_oob_raw(mtd, buf, page, 8);
}

static int
mtk_nand_block_markbad(struct mtd_info *mtd, loff_t offset)
{
	struct nand_chip *chip = mtd->priv;
	int block = (int)offset >> chip->phys_erase_shift;
	int ret;
	int mapped_block = block;

	nand_get_device(chip, mtd, FL_WRITING);

#if defined(MTK_NAND_BMT)    
	mapped_block = get_mapping_block_index(block);
	ret = mtk_nand_block_markbad_hw(mtd, mapped_block << chip->phys_erase_shift);
#else
	if (shift_on_bbt) {
		mapped_block = block_remap(mtd, block);
		if (mapped_block == -1) {
			printk("NAND mark bad failed\n");
			nand_release_device(mtd);
			return NAND_STATUS_FAIL;
		}
	}
	ret = mtk_nand_block_markbad_hw(mtd, mapped_block << chip->phys_erase_shift);
#endif
	nand_release_device(mtd);

	return ret;
}

int
mtk_nand_read_oob_hw(struct mtd_info *mtd, struct nand_chip *chip, int page)
{
	int i;
	u8 iter = 0;

	int sec_num = 1<<(chip->page_shift-9);
	int spare_per_sector = mtd->oobsize/sec_num;

	if (mtk_nand_read_oob_raw(mtd, chip->oob_poi, page, mtd->oobsize)) {
		printk(KERN_ERR "[%s]mtk_nand_read_oob_raw return failed\n", __FUNCTION__);
		return -EIO;
	}

	// adjust to ecc physical layout to memory layout
	/*********************************************************/
	/* FDM0 | ECC0 | FDM1 | ECC1 | FDM2 | ECC2 | FDM3 | ECC3 */
	/*  8B  |  8B  |  8B  |  8B  |  8B  |  8B  |  8B  |  8B  */
	/*********************************************************/

	memcpy(local_oob_buf, chip->oob_poi, mtd->oobsize);
	// copy ecc data
	for (i = 0; i < chip->ecc.layout->eccbytes; i++) {
		iter = (i / (spare_per_sector-OOB_AVAI_PER_SECTOR)) *  spare_per_sector + OOB_AVAI_PER_SECTOR + i % (spare_per_sector-OOB_AVAI_PER_SECTOR);
		chip->oob_poi[chip->ecc.layout->eccpos[i]] = local_oob_buf[iter];
	}

	// copy FDM data
	for (i = 0; i < sec_num; i++) {
		memcpy(&chip->oob_poi[i * OOB_AVAI_PER_SECTOR], &local_oob_buf[i *  spare_per_sector], OOB_AVAI_PER_SECTOR);
	}

	return 0;
}

static int
mtk_nand_read_oob(struct mtd_info *mtd, struct nand_chip *chip, int page)
{
	int page_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);
	int block = page / page_per_block;
	u16 page_in_block = page % page_per_block;
	int mapped_block = block;

#if defined (MTK_NAND_BMT)
	mapped_block = get_mapping_block_index(block);
	mtk_nand_read_oob_hw(mtd, chip, page_in_block + mapped_block * page_per_block);
#else
	if (shift_on_bbt) {
		mapped_block = block_remap(mtd, block);
		if (mapped_block == -1)
			return NAND_STATUS_FAIL;
		// allow to read oob even if the block is bad
	}
	if (mtk_nand_read_oob_hw(mtd, chip, page_in_block + mapped_block * page_per_block)!=0)
		return -1;
#endif
	return 0;
}

int
mtk_nand_block_bad_hw(struct mtd_info *mtd, loff_t ofs)
{
	struct nand_chip *chip = (struct nand_chip *)mtd->priv;
	int page_addr = (int)(ofs >> chip->page_shift);
	unsigned int page_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);
	unsigned char oob_buf[8];

	page_addr &= ~(page_per_block - 1);
	if (mtk_nand_read_oob_raw(mtd, oob_buf, page_addr, sizeof(oob_buf))) {
		printk(KERN_WARNING "mtk_nand_read_oob_raw return error\n");
		return 1;
	}

	if (oob_buf[0] != 0xff) {
		printk(KERN_WARNING "Bad block detected at 0x%x, oob_buf[0] is 0x%x\n", page_addr, oob_buf[0]);
		// dump_nfi();
		return 1;
	}

	return 0;
}

static int
mtk_nand_block_bad(struct mtd_info *mtd, loff_t ofs, int getchip)
{
	int chipnr = 0;
	struct nand_chip *chip = (struct nand_chip *)mtd->priv;
	int block = (int)ofs >> chip->phys_erase_shift;
	int mapped_block = block;
	int ret;

	if (getchip) {
		chipnr = (int)(ofs >> chip->chip_shift);
		nand_get_device(chip, mtd, FL_READING);
		/* Select the NAND device */
		chip->select_chip(mtd, chipnr);
	}

#if defined(MTK_NAND_BMT)    
	mapped_block = get_mapping_block_index(block);
#else
	if (shift_on_bbt) {
		mapped_block = block_remap(mtd, block);
		if (mapped_block == -1) {
		if (getchip)
			nand_release_device(mtd);
			return NAND_STATUS_FAIL;
		}
	}
#endif

	ret = mtk_nand_block_bad_hw(mtd, mapped_block << chip->phys_erase_shift);
#if defined (MTK_NAND_BMT)	
	if (ret) {
		MSG(INIT, "Unmapped bad block: 0x%x\n", mapped_block);
		if (update_bmt(mapped_block << chip->phys_erase_shift, UPDATE_UNMAPPED_BLOCK, NULL, NULL)) {
			MSG(INIT, "Update BMT success\n");
			ret = 0;
		} else {
			MSG(INIT, "Update BMT fail\n");
			ret = 1;
		}
	}
#endif

	if (getchip)
		nand_release_device(mtd);

	return ret;
}

#ifdef CONFIG_MTD_NAND_VERIFY_WRITE
char gacBuf[4096 + 288];

static int
mtk_nand_verify_buf(struct mtd_info *mtd, const uint8_t * buf, int len)
{
	struct nand_chip *chip = (struct nand_chip *)mtd->priv;
	struct NAND_CMD *pkCMD = &g_kCMD;
	u32 u4PageSize = mtd->writesize;
	u32 *pSrc, *pDst;
	int i;

	mtk_nand_exec_read_page(mtd, pkCMD->u4RowAddr, u4PageSize, gacBuf, gacBuf + u4PageSize);

	pSrc = (u32 *) buf;
	pDst = (u32 *) gacBuf;
	len = len / sizeof(u32);
	for (i = 0; i < len; ++i) {
		if (*pSrc != *pDst) {
			MSG(VERIFY, "mtk_nand_verify_buf page fail at page %d\n", pkCMD->u4RowAddr);
			return -1;
		}
		pSrc++;
		pDst++;
	}

	pSrc = (u32 *) chip->oob_poi;
	pDst = (u32 *) (gacBuf + u4PageSize);

	if ((pSrc[0] != pDst[0]) || (pSrc[1] != pDst[1]) || (pSrc[2] != pDst[2]) || (pSrc[3] != pDst[3]) || (pSrc[4] != pDst[4]) || (pSrc[5] != pDst[5])) {
	// TODO: Ask Designer Why?
	//(pSrc[6] != pDst[6]) || (pSrc[7] != pDst[7])) 
		MSG(VERIFY, "mtk_nand_verify_buf oob fail at page %d\n", pkCMD->u4RowAddr);
		MSG(VERIFY, "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", pSrc[0], pSrc[1], pSrc[2], pSrc[3], pSrc[4], pSrc[5], pSrc[6], pSrc[7]);
		MSG(VERIFY, "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", pDst[0], pDst[1], pDst[2], pDst[3], pDst[4], pDst[5], pDst[6], pDst[7]);
		return -1;
	}
	return 0;
}
#endif

static void
mtk_nand_init_hw(struct mtk_nand_host *host) {
	struct mtk_nand_host_hw *hw = host->hw;
	u32 data;

	data = DRV_Reg32(RALINK_SYSCTL_BASE+0x60);
	data &= ~((0x3<<18)|(0x3<<16));
	data |= ((0x2<<18) |(0x2<<16));
	DRV_WriteReg32(RALINK_SYSCTL_BASE+0x60, data);

	MSG(INIT, "Enable NFI Clock\n");
	nand_enable_clock();

	g_bInitDone = false;
	g_kCMD.u4OOBRowAddr = (u32) - 1;

	/* Set default NFI access timing control */
	DRV_WriteReg32(NFI_ACCCON_REG32, hw->nfi_access_timing);
	DRV_WriteReg16(NFI_CNFG_REG16, 0);
	DRV_WriteReg16(NFI_PAGEFMT_REG16, 0);

	/* Reset the state machine and data FIFO, because flushing FIFO */
	(void)mtk_nand_reset();

	/* Set the ECC engine */
	if (hw->nand_ecc_mode == NAND_ECC_HW) {
		MSG(INIT, "%s : Use HW ECC\n", MODULE_NAME);
		if (g_bHwEcc)
			NFI_SET_REG32(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
		ECC_Config(host->hw,4);
		mtk_nand_configure_fdm(8);
		mtk_nand_configure_lock();
	}

	NFI_SET_REG16(NFI_IOCON_REG16, 0x47);
}

static int mtk_nand_dev_ready(struct mtd_info *mtd)
{
	return !(DRV_Reg32(NFI_STA_REG32) & STA_NAND_BUSY);
}

#define FACT_BBT_BLOCK_NUM  32 // use the latest 32 BLOCK for factory bbt table
#define FACT_BBT_OOB_SIGNATURE  1
#define FACT_BBT_SIGNATURE_LEN  7
const u8 oob_signature[] = "mtknand";
static u8 *fact_bbt = 0;
static u32 bbt_size = 0;

static int
read_fact_bbt(struct mtd_info *mtd, unsigned int page)
{
	struct nand_chip *chip = mtd->priv;

	// read oob
	if (mtk_nand_read_oob_hw(mtd, chip, page)==0)
	{
		if (chip->oob_poi[nand_badblock_offset] != 0xFF)
		{
			printk("Bad Block on Page %x\n", page);
			return -1;
		}
		if (memcmp(&chip->oob_poi[FACT_BBT_OOB_SIGNATURE], oob_signature, FACT_BBT_SIGNATURE_LEN) != 0)
		{
			printk("compare signature failed %x\n", page);
			return -1;
		}
		if (mtk_nand_exec_read_page(mtd, page, mtd->writesize, chip->buffers->databuf, chip->oob_poi))
		{
			printk("Signature matched and data read!\n");
			memcpy(fact_bbt, chip->buffers->databuf, (bbt_size <= mtd->writesize)? bbt_size:mtd->writesize);
			return 0;
		}

	}
	printk("failed at page %x\n", page);
	return -1;
}

static int
load_fact_bbt(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd->priv;
	int i;
	u32 total_block;

	total_block = 1 << (chip->chip_shift - chip->phys_erase_shift);
	bbt_size = total_block >> 2;

	if ((!fact_bbt) && (bbt_size))
		fact_bbt = (u8 *)kmalloc(bbt_size, GFP_KERNEL);
	if (!fact_bbt)
		return -1;

	for (i = total_block - 1; i >= (total_block - FACT_BBT_BLOCK_NUM); i--)
	{
		if (read_fact_bbt(mtd, i << (chip->phys_erase_shift - chip->page_shift)) == 0)
		{
			printk("load_fact_bbt success %d\n", i);
			return 0;
		}

	}
	printk("load_fact_bbt failed\n");
	return -1;
}

static int
mtk_nand_probe(struct platform_device *pdev)
{
	struct mtd_part_parser_data ppdata;
	struct mtk_nand_host_hw *hw;
	struct mtd_info *mtd;
	struct nand_chip *nand_chip;
	u8 ext_id1, ext_id2, ext_id3;
	int err = 0;
	int id;
	u32 ext_id;
	int i;
	u32 data;

	data = DRV_Reg32(RALINK_SYSCTL_BASE+0x60);
	data &= ~((0x3<<18)|(0x3<<16));
	data |= ((0x2<<18) |(0x2<<16));
	DRV_WriteReg32(RALINK_SYSCTL_BASE+0x60, data);

	hw = &mt7621_nand_hw,
	BUG_ON(!hw);
	/* Allocate memory for the device structure (and zero it) */
	host = kzalloc(sizeof(struct mtk_nand_host), GFP_KERNEL);
	if (!host) {
		MSG(INIT, "mtk_nand: failed to allocate device structure.\n");
		return -ENOMEM;
	}

	/* Allocate memory for 16 byte aligned buffer */
	local_buffer_16_align = local_buffer + 16 - ((u32) local_buffer % 16);
	printk(KERN_INFO "Allocate 16 byte aligned buffer: %p\n", local_buffer_16_align);
	host->hw = hw;

	/* init mtd data structure */
	nand_chip = &host->nand_chip;
	nand_chip->priv = host;     /* link the private data structures */

	mtd = &host->mtd;
	mtd->priv = nand_chip;
	mtd->owner = THIS_MODULE;
	mtd->name  = "MT7621-NAND";

	hw->nand_ecc_mode = NAND_ECC_HW;

	/* Set address of NAND IO lines */
	nand_chip->IO_ADDR_R = (void __iomem *)NFI_DATAR_REG32;
	nand_chip->IO_ADDR_W = (void __iomem *)NFI_DATAW_REG32;
	nand_chip->chip_delay = 20; /* 20us command delay time */
	nand_chip->ecc.mode = hw->nand_ecc_mode;    /* enable ECC */
	nand_chip->ecc.strength = 1;
	nand_chip->read_byte = mtk_nand_read_byte;
	nand_chip->read_buf = mtk_nand_read_buf;
	nand_chip->write_buf = mtk_nand_write_buf;
#ifdef CONFIG_MTD_NAND_VERIFY_WRITE
	nand_chip->verify_buf = mtk_nand_verify_buf;
#endif
	nand_chip->select_chip = mtk_nand_select_chip;
	nand_chip->dev_ready = mtk_nand_dev_ready;
	nand_chip->cmdfunc = mtk_nand_command_bp;
	nand_chip->ecc.read_page = mtk_nand_read_page_hwecc;
	nand_chip->ecc.write_page = mtk_nand_write_page_hwecc;

	nand_chip->ecc.layout = &nand_oob_64;
	nand_chip->ecc.size = hw->nand_ecc_size;    //2048
	nand_chip->ecc.bytes = hw->nand_ecc_bytes;  //32

	// For BMT, we need to revise driver architecture
	nand_chip->write_page = mtk_nand_write_page;
	nand_chip->ecc.write_oob = mtk_nand_write_oob;
	nand_chip->block_markbad = mtk_nand_block_markbad;   // need to add nand_get_device()/nand_release_device().
	//	nand_chip->erase = mtk_nand_erase;	
	//    nand_chip->read_page = mtk_nand_read_page;
	nand_chip->ecc.read_oob = mtk_nand_read_oob;
	nand_chip->block_bad = mtk_nand_block_bad;

	//Qwert:Add for Uboot
	mtk_nand_init_hw(host);
	/* Select the device */
	nand_chip->select_chip(mtd, NFI_DEFAULT_CS);

	/*
	* Reset the chip, required by some chips (e.g. Micron MT29FxGxxxxx)
	* after power-up
	*/
	nand_chip->cmdfunc(mtd, NAND_CMD_RESET, -1, -1);

	memset(&devinfo, 0 , sizeof(flashdev_info));

	/* Send the command for reading device ID */

	nand_chip->cmdfunc(mtd, NAND_CMD_READID, 0x00, -1);

	/* Read manufacturer and device IDs */
	manu_id = nand_chip->read_byte(mtd);
	dev_id = nand_chip->read_byte(mtd);
	id = dev_id | (manu_id << 8);
	        ext_id1 = nand_chip->read_byte(mtd);
		    ext_id2 = nand_chip->read_byte(mtd);
		        ext_id3 = nand_chip->read_byte(mtd);
			    ext_id = ext_id1 << 16 | ext_id2 << 8 | ext_id3;
	if (!get_device_info(id, ext_id, &devinfo)) {
		u32 chip_mode = RALINK_REG(RALINK_SYSCTL_BASE+0x010)&0x0F;
		MSG(INIT, "Not Support this Device! \r\n");
		memset(&devinfo, 0 , sizeof(flashdev_info));
		MSG(INIT, "chip_mode=%08X\n",chip_mode);

		/* apply bootstrap first */
		devinfo.addr_cycle = 5;
		devinfo.iowidth = 8;

		switch (chip_mode) {
		case 10:
			devinfo.pagesize = 2048;
			devinfo.sparesize = 128;
			devinfo.totalsize = 128;
			devinfo.blocksize = 128;
			break;
		case 11:
			devinfo.pagesize = 4096;
			devinfo.sparesize = 128;
			devinfo.totalsize = 1024;
			devinfo.blocksize = 256;
			break;
		case 12:
			devinfo.pagesize = 4096;
			devinfo.sparesize = 224;
			devinfo.totalsize = 2048;
			devinfo.blocksize = 512;
			break;
		default:
		case 1:
			devinfo.pagesize = 2048;
			devinfo.sparesize = 64;
			devinfo.totalsize = 128;
			devinfo.blocksize = 128;
			break;
		}

		devinfo.timmingsetting = NFI_DEFAULT_ACCESS_TIMING;
		devinfo.devciename[0] = 'U';
		devinfo.advancedmode = 0;
	}
	mtd->writesize = devinfo.pagesize;
	mtd->erasesize = (devinfo.blocksize<<10);
	mtd->oobsize = devinfo.sparesize;

	nand_chip->chipsize = (devinfo.totalsize<<20);
	nand_chip->page_shift = ffs(mtd->writesize) - 1;
	nand_chip->pagemask = (nand_chip->chipsize >> nand_chip->page_shift) - 1;
	nand_chip->phys_erase_shift = ffs(mtd->erasesize) - 1;
	nand_chip->chip_shift = ffs(nand_chip->chipsize) - 1;//0x1C;//ffs(nand_chip->chipsize) - 1;
	nand_chip->oob_poi = nand_chip->buffers->databuf + mtd->writesize;
	nand_chip->badblockpos = 0;

	if (devinfo.pagesize == 4096)
		nand_chip->ecc.layout = &nand_oob_128;
	else if (devinfo.pagesize == 2048)
		nand_chip->ecc.layout = &nand_oob_64;
	else if (devinfo.pagesize == 512)
		nand_chip->ecc.layout = &nand_oob_16;

	nand_chip->ecc.layout->eccbytes = devinfo.sparesize-OOB_AVAI_PER_SECTOR*(devinfo.pagesize/NAND_SECTOR_SIZE);
	for (i = 0; i < nand_chip->ecc.layout->eccbytes; i++)
		nand_chip->ecc.layout->eccpos[i]=OOB_AVAI_PER_SECTOR*(devinfo.pagesize/NAND_SECTOR_SIZE)+i;

	MSG(INIT, "Support this Device in MTK table! %x \r\n", id);
	hw->nfi_bus_width = devinfo.iowidth;
	DRV_WriteReg32(NFI_ACCCON_REG32, devinfo.timmingsetting);

	/* 16-bit bus width */
	if (hw->nfi_bus_width == 16) {
		MSG(INIT, "%s : Set the 16-bit I/O settings!\n", MODULE_NAME);
		nand_chip->options |= NAND_BUSWIDTH_16;
	}
	mtd->oobsize = devinfo.sparesize;
	hw->nfi_cs_num = 1;

	/* Scan to find existance of the device */
	if (nand_scan(mtd, hw->nfi_cs_num)) {
		MSG(INIT, "%s : nand_scan fail.\n", MODULE_NAME);
		err = -ENXIO;
		goto out;
	}

	g_page_size = mtd->writesize;
	platform_set_drvdata(pdev, host);
	if (hw->nfi_bus_width == 16) {
		NFI_SET_REG16(NFI_PAGEFMT_REG16, PAGEFMT_DBYTE_EN);
	}

	nand_chip->select_chip(mtd, 0);
#if defined(MTK_NAND_BMT)  
	nand_chip->chipsize -= (BMT_POOL_SIZE) << nand_chip->phys_erase_shift;
#endif
	mtd->size = nand_chip->chipsize;

	CFG_BLOCKSIZE = mtd->erasesize;

#if defined(MTK_NAND_BMT)
	if (!g_bmt) {
		if (!(g_bmt = init_bmt(nand_chip, BMT_POOL_SIZE))) {
			MSG(INIT, "Error: init bmt failed\n");
			return 0;
		}
	}
#endif

	ppdata.of_node = pdev->dev.of_node;
	err = mtd_device_parse_register(mtd, probe_types, &ppdata,
					NULL, 0);
	if (!err) {
		MSG(INIT, "[mtk_nand] probe successfully!\n");
		nand_disable_clock();
		shift_on_bbt = 1;
		if (load_fact_bbt(mtd) == 0) {
			int i;
			for (i = 0; i < 0x100; i++)
				nand_chip->bbt[i] |= fact_bbt[i];
		}

		return err;
	}

out:
	MSG(INIT, "[NFI] mtk_nand_probe fail, err = %d!\n", err);
	nand_release(mtd);
	platform_set_drvdata(pdev, NULL);
	kfree(host);
	nand_disable_clock();
	return err;
}

static int
mtk_nand_remove(struct platform_device *pdev)
{
	struct mtk_nand_host *host = platform_get_drvdata(pdev);
	struct mtd_info *mtd = &host->mtd;

	nand_release(mtd);
	kfree(host);
	nand_disable_clock();

	return 0;
}

static const struct of_device_id mt7621_nand_match[] = {
	{ .compatible = "mtk,mt7621-nand" },
	{},
};
MODULE_DEVICE_TABLE(of, mt7621_nand_match);

static struct platform_driver mtk_nand_driver = {
	.probe = mtk_nand_probe,
	.remove = mtk_nand_remove,
	.driver = {
		.name = "MT7621-NAND",
		.owner = THIS_MODULE,
		.of_match_table = mt7621_nand_match,
	},
};

static int __init
mtk_nand_init(void)
{
	printk("MediaTek Nand driver init, version %s\n", VERSION);

	return platform_driver_register(&mtk_nand_driver);
}

static void __exit
mtk_nand_exit(void)
{
	platform_driver_unregister(&mtk_nand_driver);
}

module_init(mtk_nand_init);
module_exit(mtk_nand_exit);
MODULE_LICENSE("GPL");
