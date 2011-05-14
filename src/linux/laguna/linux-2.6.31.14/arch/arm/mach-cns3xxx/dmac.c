/*******************************************************************************
 *
 *  Copyright (c) 2008 Cavium Networks 
 * 
 *  This file is free software; you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License, Version 2, as 
 *  published by the Free Software Foundation. 
 *
 *  This file is distributed in the hope that it will be useful, 
 *  but AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or 
 *  NONINFRINGEMENT.  See the GNU General Public License for more details. 
 *
 *  You should have received a copy of the GNU General Public License 
 *  along with this file; if not, write to the Free Software 
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA or 
 *  visit http://www.gnu.org/licenses/. 
 *
 *  This file may also be available under a different license from Cavium. 
 *  Contact Cavium Networks for more information
 *
 ******************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/dma-mapping.h>
#include <asm/memory.h>
#include <asm/dma.h>
#include <mach/board.h>
#include <mach/pm.h>


#include <mach/dmac.h>

//#define DEBUG_GDMA

#define DMAC_MEM_MAP_VALUE(reg_offset)	(*((uint32_t volatile *)(CNS3XXX_DMAC_BASE_VIRT + reg_offset)))

#define DMAC_INTEN		DMAC_MEM_MAP_VALUE(0x020)
#define DMAC_INTSTATUS		DMAC_MEM_MAP_VALUE(0x028)
#define DMAC_INTCLR		DMAC_MEM_MAP_VALUE(0x02C)

/* DMAC Debug registers */
#define DMAC_DBGSTATUS		DMAC_MEM_MAP_VALUE(0xD00)	/* Debug Status Register */
#define DMAC_DBGCMD		DMAC_MEM_MAP_VALUE(0xD04)	/* Debug Command Register */
#define DMAC_DBGINST0		DMAC_MEM_MAP_VALUE(0xD08)	/* Debug Instrucion-0 Register */
#define DMAC_DBGINST1		DMAC_MEM_MAP_VALUE(0xD0C)	/* Debug Instrucion-1 Register */

#define CHANNEL_AND_MANAGER	0xffff //0x1ff
#define CHANNEL_ONLY		0xff
#define MANAGER_ONLY		0xff00 //0x100

#define MAX_MICROCODE_SIZE	2048

#if 0
#define ERROR_INTR		45
#define DMAC_IRQNO_BASE		46
#else
#define ERROR_INTR		68
#define DMAC_IRQNO_BASE		69
#endif

#define MAX_INTR_EVENTS		32

#define MIN_EVENT_NUM		8 //2

/* Debug Status Register */
#define DMAC_DBG_BUSY_BIT		(1<<0)
#define DMAC_DBG_INSTR_0_SHIFT		16
#define DMAC_DBG_INSTR_2_SHIFT		0
#define DMAC_DBG_THREAD_BIT		(1<<0)
#define DMAC_DBG_CH_NUM_SHIFT		8
#define DMAC_DBG_CH_NUM_BIT_MASK	0x7
#define DMAC_CHMGR			8

spinlock_t dma_mgr_lock;

typedef enum {
	DMAC_INSTR_DMAADDH = 0, /* Add Halfword */ /*** No implement ***/
	DMAC_INSTR_DMAEND,	/* End */
	DMAC_INSTR_DMAFLUSHP,	/* Flash and notify Peripheral */
	DMAC_INSTR_DMAGO,	/* Go */
	DMAC_INSTR_DMALD,	/* Load */
	DMAC_INSTR_DMALDP,	/* Load aPeripheral */
	DMAC_INSTR_DMALP,	/* Loop */
	DMAC_INSTR_DMALPEND,	/* Loop End */
//      DMAC_INSTR_DMALPFE,     /* Loop Forever */
	DMAC_INSTR_DMAKILL,	/* kill */
	DMAC_INSTR_DMAMOV,	/* Move */
	DMAC_INSTR_DMANOP,	/* No operation */
//      DMAC_INSTR_DMARMB,      /* Read Memory Barrier */
	DMAC_INSTR_DMASEV,	/* Send Event */
	DMAC_INSTR_DMAST,	/* Store */
	DMAC_INSTR_DMASTP,	/* Store and notify Peripheral */
	DMAC_INSTR_DMASTZ,	/* Store Zero */
	DMAC_INSTR_DMAWFE,	/* Wait For Event */
	DMAC_INSTR_DMAWFP,	/* Wait For Peripheral */
	DMAC_INSTR_DMAWMB,	/* Wait For Barrier */
} dmac_instr_t;

typedef struct {
	const char *enc_buf;
	int enc_buf_len;
	int chan_or_mgr;	/* 0xff for DMA manager and DMA channel, 
				   0x7f for DMA channel, 
				   0x80 for DMA manager */
} dmac_instr_encode_t;

typedef struct {
	uint32_t sa:1;		/* source address  increment: 0 - FIXED / 1 - INCR */
	uint32_t ss:3;		/* source burst size in bytes: mapping value TBD with designer */
	uint32_t sb:4;		/* source burst length */
	uint32_t sp:3;		/* source protection */
	uint32_t sc:3;		/* source cache */
	uint32_t da:1;		/* destination address  increment: 0 - FIXED / 1 - INCR */
	uint32_t ds:3;		/* destination burst size in bytes: mapping value TBD with designer */
	uint32_t db:4;		/* destination burst length */
	uint32_t dp:3;		/* destination protection */
	uint32_t dc:3;		/* destination cache */
	uint32_t es:3;		/* endian swap size, in bits */
	uint32_t padding:1;
} dmac_ch_ctrl_t;

typedef struct {
	union {
		dmac_ch_ctrl_t ccr;
		uint32_t val;
	} i;
} dmac_cmd_imm32_t;

typedef struct {
	uint16_t bs:1;		/* burst/single bit */
	uint16_t x:1;		/* x bit */
	uint16_t ns:1;		/* not secure bit */
	uint16_t lc:1;		/* loop counter bit */
	uint16_t p:1;		/* p bit */
	uint16_t nf:1;		/* no-finite bit */
	uint16_t i:1;		/* invalid bit */
	uint16_t padding:9;
} dmac_cmd_bits_t;

typedef struct {
	uint8_t periph;		/* peripheral ID */
	uint8_t cn;		/* Channel Number */
	uint8_t iter;		/* iteration count */
	uint8_t backwards_jump;	/* backwards jump length */
	uint8_t rd;		/* destination register, <SAR=b000, CCR=b001, DAR=b010> */
	uint8_t ra;             /* address register, <SAR=b0, DAR=b1> */
	uint8_t event_num;	/* event number */

	union {
		dmac_cmd_bits_t b;
		uint16_t val;
	} bits;

	dmac_cmd_imm32_t imm32;	/* immediate 32bit value */
	uint16_t imm16;

} dmac_instr_param_t;

typedef struct {
	int in_use;		/* Channel in use or not */
	int channel;		/* Channel number */
	int microcode_size;	/* Microcode size */
	uint8_t *microcode;	/* TODO */
	dma_addr_t microcode_dma;
	int (*intr_handler) (void *);
	void *handler_args;
	int notifications_used;	/* 32 bits for every interrupt/event */
} dmac_channel_t;

/* TODO: Not protected as of now */
dmac_channel_t *dmac_channels[MAX_DMA_CHANNELS + (MIN_EVENT_NUM-1)];

int dmac_events[MAX_INTR_EVENTS];

static int dmac_create_instr(int chan, dmac_instr_t instr,
			     dmac_instr_param_t * param);
static int dmac_exec_ucode(int ucode_channel, int ch);
void pl330_dump_regs(void);

/******************************************************************************
 *
 * Instruction:  DMAEND
 * Description:
 *   | 7 6 5 4 | 3 2 1 0 |
 *     0 0 0 0   0 0 0 0
 * Example:
 *    DMAEND
 *   00
 ******************************************************************************/
const char dmac_code_DMAEND[] = { 0x00 };

int DMAC_DMAEND(int ch_num)
{
	dmac_instr_param_t param;
	int instr_len;
	memset(&param, 0, sizeof(dmac_instr_param_t));
	instr_len = dmac_create_instr(ch_num, DMAC_INSTR_DMAEND, &param);
	if (instr_len < 0) {
		printk("dmac_create_instr failed \n");
		return -1;
	}

	return 0;
}

EXPORT_SYMBOL(DMAC_DMAEND);

/******************************************************************************
 *
 * Instruction:  DMAFLUSHP
 * Description:
 *   | 15 14 13 12 | 11 10  9  8 |  7  6  5  4 |  3  2  1  0 |
 *     <periph[4:0]   >  0  0  0    0  0  1  1    0  1  0  1
 * Example:
 *    DMAFLUSHP P0
 *   35 00
 ******************************************************************************/
const char dmac_code_DMAFLUSHP[] = { 0x35, 0x00 };

int DMAC_DMAFLUSHP(int ch_num, int periph)
{
	dmac_instr_param_t param;
	int instr_len;
	memset(&param, 0, sizeof(dmac_instr_param_t));
	param.periph = periph;
	instr_len = dmac_create_instr(ch_num, DMAC_INSTR_DMAFLUSHP, &param);
	if (instr_len < 0) {
		printk("dmac_create_instr failed \n");
		return -1;
	}

	return 0;
}

EXPORT_SYMBOL(DMAC_DMAFLUSHP);

/******************************************************************************
 *
 * Instruction:  DMAGO
 * Description:
 *   | 15 14 13 12 | 11 10  9  8 |  7  6  5  4 |  3  2  1  0 |
 *      0  0  0  0    0 <cn[2:0]>   1  0  1  0    0  0 ns  0
 *
 *   | 47                                                 16 |
 *        <                     imm[31:0]                     >
 * Example:
 *    DMAGO  C0, 0x40000000
 *   A0 00 00 00 00 40
 ******************************************************************************/
const char dmac_code_DMAGO[] = { 0xA0, 0x00, 0x00, 0x00, 0x00, 0x40 };

int DMAC_DMAGO(int ch_num)
{
	dmac_instr_param_t param;
	int instr_len;
	dmac_channel_t *dma_ch = dmac_channels[ch_num];
	
	if(!dma_ch->in_use) {
		printk("DMAC_DMAGO an unused channel\n");
		return -1;
	}
	
	memset(&param, 0, sizeof(dmac_instr_param_t));
	param.bits.b.ns = 1;
	param.cn = ch_num;
	param.imm32.i.val = dma_ch->microcode_dma;
#ifdef DEBUG_GDMA
	printk("%s:%d: microcode Physical Address *(%x)==[%x]\n", __FUNCTION__,
	       __LINE__, param.imm32.i.val,
	       *((uint32_t *) phys_to_virt(dma_ch->microcode_dma)));
#endif
	instr_len = dmac_create_instr((DMAC_CHMGR + ch_num), DMAC_INSTR_DMAGO, &param);
	if (instr_len < 0) {
		printk("dmac_create_instr failed \n");
		return -1;
	}

	dmac_exec_ucode((DMAC_CHMGR + ch_num), (DMAC_CHMGR + ch_num));	// DMAC_CHMGR);
	if (dmac_channels[(DMAC_CHMGR + ch_num)])
		dmac_channels[(DMAC_CHMGR + ch_num)]->microcode_size = 0;
	else
		printk("BUG HERE !! DEBUG .. \n");

	return 0;
}

EXPORT_SYMBOL(DMAC_DMAGO);

/******************************************************************************
 *
 * Instruction:  DMALD
 * Description:
 *   | 7 6 5 4 |  3  2  1 0 |
 *     0 0 0 0    0  1 bs x
 * Example:
 *    DMALD
 *   04
 ******************************************************************************/
const char dmac_code_DMALD[] = { 0x04 };

int DMAC_DMALD(int ch_num)
{
	dmac_instr_param_t param;
	int instr_len;
	memset(&param, 0, sizeof(dmac_instr_param_t));
	/* param.bits.b.x = param.bits.b.bs = 0; */
	instr_len = dmac_create_instr(ch_num, DMAC_INSTR_DMALD, &param);
	if (instr_len < 0) {
		printk("dmac_create_instr failed \n");
		return -1;
	}

	return 0;
}

EXPORT_SYMBOL(DMAC_DMALD);

int DMAC_DMALDB(int ch_num)
{
	dmac_instr_param_t param;
	int instr_len;
	memset(&param, 0, sizeof(dmac_instr_param_t));
	/* param.bits.b.x = param.bits.b.bs = 0; */
	param.bits.b.x = 1;
	param.bits.b.bs = 1;
	instr_len = dmac_create_instr(ch_num, DMAC_INSTR_DMALD, &param);
	if (instr_len < 0) {
		printk("dmac_create_instr failed \n");
		return -1;
	}

	return 0;
}

EXPORT_SYMBOL(DMAC_DMALDB);

int DMAC_DMALDS(int ch_num)
{
	dmac_instr_param_t param;
	int instr_len;
	memset(&param, 0, sizeof(dmac_instr_param_t));
	/* param.bits.b.x = param.bits.b.bs = 0; */
	param.bits.b.x = 1;
	param.bits.b.bs = 0;
	instr_len = dmac_create_instr(ch_num, DMAC_INSTR_DMALD, &param);
	if (instr_len < 0) {
		printk("dmac_create_instr failed \n");
		return -1;
	}

	return 0;
}

EXPORT_SYMBOL(DMAC_DMALDS);

/******************************************************************************
 *
 * Instruction:  DMALP
 * Description:
 *   | 15 14 13 12 | 11 10  9  8 |  7  6  5  4 |  3  2  1  0 |
 *     <       iter[7:0]       >    0  0  1  0    0  0 lc  0
 * Example:
 *    DMALP 8
 *   20 07
 ******************************************************************************/
const char dmac_code_DMALP[] = { 0x20, 0x07 };

int DMAC_DMALP(int ch_num, int loop_reg_idx, int iter)
{
	dmac_instr_param_t param;
	int instr_len;
	memset(&param, 0, sizeof(dmac_instr_param_t));
	param.bits.b.lc = loop_reg_idx;
	param.iter = (uint8_t) (iter - 1);
	instr_len = dmac_create_instr(ch_num, DMAC_INSTR_DMALP, &param);
	if (instr_len < 0) {
		printk("dmac_create_instr failed \n");
		return -1;
	}
	return 0;
}

EXPORT_SYMBOL(DMAC_DMALP);

/******************************************************************************
 *
 * Instruction:  DMALPEND
 * Description:
 *   | 15 14 13 12 | 11 10  9  8 |  7  6  5  4 |  3  2  1  0 |
 *     <  backwards_jump[7:0]  >    0  0  1 nf    1 lc bs  x
 * Example:
 *    DMALPEND
 *   38 04
 ******************************************************************************/
const char dmac_code_DMALPEND[] = { 0x38, 0x04 };

int DMAC_DMALPEND(int ch_num, int loop_reg_idx, int jump, int lpfe)
{
	dmac_instr_param_t param;
	int instr_len;
	memset(&param, 0, sizeof(dmac_instr_param_t));
	/* param.bits.b.x = param.bits.b.bs = 0; */
	param.bits.b.lc = loop_reg_idx;
	param.bits.b.nf = lpfe;
	param.backwards_jump = jump;
	instr_len = dmac_create_instr(ch_num, DMAC_INSTR_DMALPEND, &param);
	if (instr_len < 0) {
		printk("dmac_create_instr failed \n");
		return -1;
	}
	return 0;
}

EXPORT_SYMBOL(DMAC_DMALPEND);

/******************************************************************************
 *
 * Instruction:  DMAMOV
 * Description:
 *   | 15 14 13 12 | 11 10  9  8 |  7  6  5  4 |  3  2  1  0 |
 *      0  0  0  0    0 <rd[2:0]>   1  0  1  1    1  1  0  0
 *
 *   | 47                                                 16 |
 *        <                     imm[31:0]                     >
 *
 *      # CCR Description
 *      # [30:28]  Endian swap size
 *      # [27:25]  AWCACHE[3,1:0] value
 *      # [24:22]  AWPROT value
 *      # [21:18]  AWLEN value
 *      # [17:15]  AWSIZE value
 *      # [14]     AWBURST[0] value
 *                 0 - FIXED / 1 - INCR
 *      # [13:11]  ARCACHE[2:0] value
 *      # [10:8]   ARPROT value
 *      # [7:4]    ARLEN value
 *      # [3:1]    ARSIZE value
 *      # [0]      ARBURST[0] value
 *                 0 - FIXED / 1 - INCR
 * Example:
 *    DMAMOV   CCR, SB1 SS32 DB1 DS32
 *   BC 01 05 40 01 00
 ******************************************************************************/
const char dmac_code_DMAMOV[] = { 0xBC, 0x01, 0x05, 0x40, 0x01, 0x00 };

/* ccr_sar_dar: 0 for SAR, 1, for CCR, 2 for DAR */
//typedef enum { SAR = 0, CCR = 1, DAR = 2} dmamov_arg_t;
int DMAC_DMAMOV(int ch_num, dmamov_arg_t ccr_sar_dar, uint32_t value)
{
	dmac_instr_param_t param;
	int instr_len;
	memset(&param, 0, sizeof(dmac_instr_param_t));
	param.rd = ccr_sar_dar;
	param.imm32.i.val = value;
	instr_len = dmac_create_instr(ch_num, DMAC_INSTR_DMAMOV, &param);
	if (instr_len < 0) {
		printk("dmac_create_instr failed \n");
		return -1;
	}
	return 0;
}

EXPORT_SYMBOL(DMAC_DMAMOV);

/******************************************************************************
 *
 * Instruction:  DMAST
 * Description:
 *   | 7 6 5 4 |  3  2  1 0 |
 *     0 0 0 0    1  0 bs x
 * Example:
 *    DMAST
 *   08
 ******************************************************************************/
const char dmac_code_DMAST[] = { 0x08 };

int DMAC_DMAST(int ch_num)
{
	dmac_instr_param_t param;
	int instr_len;
	memset(&param, 0, sizeof(dmac_instr_param_t));
	/* param.bits.b.x = param.bits.b.bs = 0; */
	instr_len = dmac_create_instr(ch_num, DMAC_INSTR_DMAST, &param);
	if (instr_len < 0) {
		printk("dmac_create_instr failed \n");
		return -1;
	}

	return 0;
}

EXPORT_SYMBOL(DMAC_DMAST);

const char dmac_code_DMAWMB[] = { 0x13 };

int DMAC_DMAWMB(int ch_num)
{
	dmac_instr_param_t param;
	int instr_len;
	memset(&param, 0, sizeof(dmac_instr_param_t));
	instr_len = dmac_create_instr(ch_num, DMAC_INSTR_DMAWMB, &param);
	if (instr_len < 0) {
		printk("dmac_create_instr failed\n");
		return -1;
	}
	return 0;
}

EXPORT_SYMBOL(DMAC_DMAWMB);

const char dmac_code_DMANOP[] = { 0x18 };

int DMAC_DMANOP(int ch_num)
{
	dmac_instr_param_t param;
	int instr_len;
	memset(&param, 0, sizeof(dmac_instr_param_t));
	instr_len = dmac_create_instr(ch_num, DMAC_INSTR_DMANOP, &param);
	if (instr_len < 0) {
		printk("dmac_create_instr failed\n");
		return -1;
	}
	return 0;
}

EXPORT_SYMBOL(DMAC_DMANOP);

int DMAC_DMASTB(int ch_num)
{
	dmac_instr_param_t param;
	int instr_len;
	memset(&param, 0, sizeof(dmac_instr_param_t));
	param.bits.b.x = 1;
	param.bits.b.bs = 1;
	instr_len = dmac_create_instr(ch_num, DMAC_INSTR_DMAST, &param);
	if (instr_len < 0) {
		printk("dmac_create_instr failed \n");
		return -1;
	}

	return 0;
}

EXPORT_SYMBOL(DMAC_DMASTB);

int DMAC_DMASTS(int ch_num)
{
	dmac_instr_param_t param;
	int instr_len;
	memset(&param, 0, sizeof(dmac_instr_param_t));
	param.bits.b.x = 1;
	param.bits.b.bs = 0;
	instr_len = dmac_create_instr(ch_num, DMAC_INSTR_DMAST, &param);
	if (instr_len < 0) {
		printk("dmac_create_instr failed \n");
		return -1;
	}

	return 0;
}

EXPORT_SYMBOL(DMAC_DMASTS);

/******************************************************************************
 *
 * Instruction:  DMASTZ
 * Description:
 *   | 7 6 5 4 |  3  2  1 0 |
 *     0 0 0 0    1  1  0 0
 * Example:
 *    DMASTZ
 *   08
 ******************************************************************************/
const char dmac_code_DMASTZ[] = { 0x0C };

/******************************************************************************
 *
 * Instruction:  DMAWFE
 * Description:
 *   | 15 14 13 12 | 11 10  9  8 |  7  6  5  4 |  3  2  1  0 |
 *     <event_num[4:0]>  0  i  0    0  0  1  1    0  1  1  0
 * Example:
 *    DMAWFE E0
 *   36 00
 ******************************************************************************/
const char dmac_code_DMAWFE[] = { 0x36, 0x00 };

int DMAC_WFE(int chan, int event_num)
{
	dmac_instr_param_t param;
	int instr_len;
	memset(&param, 0, sizeof(dmac_instr_param_t));
	/* param.bits.b.x = param.bits.b.bs = 0; */
//#warning "to set bits"
	param.event_num = event_num;
	instr_len = dmac_create_instr(chan, DMAC_INSTR_DMAWFE, &param);
	if (instr_len < 0) {
		printk("dmac_create_instr failed \n");
		return -1;
	}

	return 0;
}

EXPORT_SYMBOL(DMAC_WFE);

/******************************************************************************
 *
 * Instruction:  DMAWFP
 * Description:
 *   | 15 14 13 12 | 11 10  9  8 |  7  6  5  4 |  3  2  1  0 |
 *     <  periph[4:0] >  0  0  0    0  0  1  1    0  0 bs  p
 * Example:
 *    DMAWFP P0, periph
 *   31 00
 ******************************************************************************/
const char dmac_code_DMAWFP[] = { 0x31, 0x00 };

int DMAC_DMAWFP(int ch_num, int periph_id, dmawfp_burst_type s)
{
	dmac_instr_param_t param;
	int instr_len;
	memset(&param, 0, sizeof(dmac_instr_param_t));
	if (s == SINGLE) {
		param.bits.b.bs = 0;
		param.bits.b.p = 0;
	}
	if (s == BURST) {
		param.bits.b.bs = 1;
		param.bits.b.p = 0;
	}
	if (s == PERIPHERAL) {
		param.bits.b.bs = 0;
		param.bits.b.p = 1;
	}
	param.periph = periph_id;
	instr_len = dmac_create_instr(ch_num, DMAC_INSTR_DMAWFP, &param);
	if (instr_len < 0) {
		printk("dmac_create_instr failed \n");
		return -1;
	}

	return 0;
}

EXPORT_SYMBOL(DMAC_DMAWFP);

/******************************************************************************
 *
 * Instruction:  DMAKILL
 * Description:
 *   | 7 6 5 4 | 3 2 1 0 |
 *     0 0 0 0   0 0 0 1
 * Example:
 *    DMAKILL
 *   01
 ******************************************************************************/
const char dmac_code_DMAKILL[] = { 0x01 };

/******************************************************************************
 *
 * Instruction:  DMASEV
 * Description:
 *   | 15 14 13 12 | 11 10  9  8 |  7  6  5  4 |  3  2  1  0 |
 *     <event_num[4:0]>  0  i  0    0  0  1  1    0  1  0  0
 * Example:
 *    DMASEV E0
 *   34 00
 ******************************************************************************/
const char dmac_code_DMASEV[] = { 0x34, 0x00 };

int DMAC_DMASEV(int ch_num, int event_num)
{
	dmac_instr_param_t param;
	int instr_len;
	dmac_channel_t *dma_ch = dmac_channels[ch_num];
	if ((event_num >= MIN_EVENT_NUM)
	    && !(dma_ch->notifications_used & (1 << event_num))) {
		printk("DMAC_DMASEV failed event number request not done\n");
		return -1;
	} else if ((event_num < MIN_EVENT_NUM) && (event_num != ch_num)) {
		printk
		    ("%s:%d - Presently, we have this hard restriction that each channel can signal irq event == channel_no\n",
		     __FUNCTION__, __LINE__);
		return -1;
	}
	memset(&param, 0, sizeof(dmac_instr_param_t));
	param.event_num = event_num;
	instr_len = dmac_create_instr(ch_num, DMAC_INSTR_DMASEV, &param);
	if (instr_len < 0) {
		printk("dmac_create_instr failed \n");
		return -1;
	}

	return 0;
}

EXPORT_SYMBOL(DMAC_DMASEV);

/******************************************************************************
 *
 * Instruction:  DMALDP<S|B>
 * Description:
 *   | 15 14 13 12 | 11 10  9  8 |  7  6  5  4 |  3  2  1  0 |
 *     <  periph[4:0] >  0  0  0    0  0  1  0    0  1 bs  1
 * Example:
 *    DMALDPS P0
 *   25 00
 ******************************************************************************/
const char dmac_code_DMALDP[] = { 0x25, 0x00 };

int DMAC_DMALDP(int ch_num, int periph_id, int burst)
{
	dmac_instr_param_t param;
	int instr_len;
	memset(&param, 0, sizeof(dmac_instr_param_t));
	/* param.bits.b.x = param.bits.b.bs = 0; */
	param.periph = periph_id;
	param.bits.b.bs = burst;
	instr_len = dmac_create_instr(ch_num, DMAC_INSTR_DMALDP, &param);
	if (instr_len < 0) {
		printk("dmac_create_instr failed \n");
		return -1;
	}

	return 0;
}

EXPORT_SYMBOL(DMAC_DMALDP);

/******************************************************************************
 *
 * Instruction:  DMASTP<S|B>
 * Description:
 *   | 15 14 13 12 | 11 10  9  8 |  7  6  5  4 |  3  2  1  0 |
 *     <  periph[4:0] >  0  0  0    0  0  1  0    1  0 bs  1
 * Example:
 *    DMASTPS P0
 *   29 00
 ******************************************************************************/
const char dmac_code_DMASTP[] = { 0x29, 0x00 };

int DMAC_DMASTP(int ch_num, int periph_id, int burst)
{
	dmac_instr_param_t param;
	int instr_len;
	memset(&param, 0, sizeof(dmac_instr_param_t));
	/* param.bits.b.x = param.bits.b.bs = 0; */
	param.periph = periph_id;
	param.bits.b.bs = burst;
	instr_len = dmac_create_instr(ch_num, DMAC_INSTR_DMASTP, &param);
	if (instr_len < 0) {
		printk("dmac_create_instr failed \n");
		return -1;
	}

	return 0;
}

EXPORT_SYMBOL(DMAC_DMASTP);

/******************************************************************************
 *
 * Instruction:  DMAADDH
 * Description:
 *      | 23 22 21 20 | 19 18 17 16 | 15 14 13 12 | 11 10  9  8 |  7  6  5  4 |  3  2  1  0 |
 *        <       imm[15:8]       >   <       imm[7:0]        >    0  1  0  1    0  1 ra  0
 * Example:
 *      DMAADDH <address register> <16-bit immediate>
 *      56 00 00
 ******************************************************************************/
const char dmac_code_DMAADDH[] = {0x56, 0x00, 0x00};

int DMAC_DMAADDH(int ch_num, int r, int val)
{
        dmac_instr_param_t param;
        int instr_len;

        memset(&param, 0, sizeof(dmac_instr_param_t));
        param.ra = r;
        param.imm16 = val;
        instr_len = dmac_create_instr(ch_num, DMAC_INSTR_DMAADDH, &param);
        if (instr_len < 0) {
                printk("dmac_create_instr failed \n");
                return -1;
        }

        return 0;
}

EXPORT_SYMBOL(DMAC_DMAADDH);



dmac_instr_encode_t dmac_codes[] = {
	{dmac_code_DMAADDH, sizeof(dmac_code_DMAADDH), CHANNEL_ONLY}
	,
	{dmac_code_DMAEND, sizeof(dmac_code_DMAEND), CHANNEL_AND_MANAGER}
	,
	{dmac_code_DMAFLUSHP, sizeof(dmac_code_DMAFLUSHP), CHANNEL_ONLY}
	,
	{dmac_code_DMAGO, sizeof(dmac_code_DMAGO), MANAGER_ONLY}
	,
	{dmac_code_DMALD, sizeof(dmac_code_DMALD), CHANNEL_ONLY}
	,
	{dmac_code_DMALDP, sizeof(dmac_code_DMALDP), CHANNEL_ONLY}
	,
	{dmac_code_DMALP, sizeof(dmac_code_DMALP), CHANNEL_ONLY}
	,
	{dmac_code_DMALPEND, sizeof(dmac_code_DMALPEND), CHANNEL_ONLY}
	,
	{dmac_code_DMAKILL, sizeof(dmac_code_DMAKILL), CHANNEL_AND_MANAGER}
	,
	{dmac_code_DMAMOV, sizeof(dmac_code_DMAMOV), CHANNEL_ONLY}
	,
	{dmac_code_DMANOP, sizeof(dmac_code_DMANOP), CHANNEL_AND_MANAGER}
	,
	{dmac_code_DMASEV, sizeof(dmac_code_DMASEV), CHANNEL_AND_MANAGER}
	,
	{dmac_code_DMAST, sizeof(dmac_code_DMAST), CHANNEL_ONLY}
	,
	{dmac_code_DMASTP, sizeof(dmac_code_DMASTP), CHANNEL_ONLY}
	,
	{dmac_code_DMASTZ, sizeof(dmac_code_DMASTZ), CHANNEL_ONLY}
	,
	{dmac_code_DMAWFE, sizeof(dmac_code_DMAWFE), CHANNEL_AND_MANAGER}
	,
	{dmac_code_DMAWFP, sizeof(dmac_code_DMAWFP), CHANNEL_ONLY}
	,
	{dmac_code_DMAWMB, sizeof(dmac_code_DMAWMB), CHANNEL_ONLY}
	,
};

static void Dmac_Cmd_Write32(uint8_t * buf, uint32_t val)
{
	buf[0] = (uint8_t) (val);
	buf[1] = (uint8_t) (val >> 8);
	buf[2] = (uint8_t) (val >> 16);
	buf[3] = (uint8_t) (val >> 24);

	return;
}

static int
dmac_create_instr(int chan, dmac_instr_t instr, dmac_instr_param_t * param)
{
	int len = 0;
	dmac_channel_t *dma_ch = dmac_channels[chan];
	uint8_t *buf = NULL;
#ifdef DEBUG_GDMA
	printk("%s:%d: In with channel no %d\n", __FUNCTION__, __LINE__, chan);
#endif

	if (!((0x1 << chan) & dmac_codes[instr].chan_or_mgr)) {
		printk("Channel %d does not support this instruction %d\n",
		       chan, instr);
		return -1;
	}
#ifdef DEBUG_GDMA
	if (!dma_ch)
		printk("%s:%d: Bug here !!\n", __FUNCTION__, __LINE__);
#endif

	if (dma_ch->microcode == NULL) {
		buf = dma_ch->microcode =
		    dma_alloc_coherent(NULL, MAX_MICROCODE_SIZE,
				       &dma_ch->microcode_dma, GFP_KERNEL);
		printk
		    ("First time microcode alloc for channel %d done @phy:%x\n",
		     chan, dma_ch->microcode_dma);
		dma_ch->microcode_size = 0;
	} else {
		if ((dmac_codes[instr].enc_buf_len + dma_ch->microcode_size) >
		    MAX_MICROCODE_SIZE) {
			printk
			    ("We have a buffer overflow [%d]issue here ... BUG !!\n",
			     dma_ch->microcode_size);
			return -1;
		}
		buf = dma_ch->microcode + dma_ch->microcode_size;
	}
#ifdef DEBUG_GDMA
	printk("%s:%d: Microcode alloc for channel %d\n", __FUNCTION__,
	       __LINE__, chan);
#endif

	if (buf == NULL) {
		printk("%s: Unable to allocate memory for microocode space\n",
		       __FUNCTION__);
		return -1;
	}
#ifdef DEBUG_GDMA
	printk("%s:%d: allocated microcode buffer%p [@phy: %x]\n", __FUNCTION__,
	       __LINE__, buf, dma_ch->microcode_dma + dma_ch->microcode_size);
#endif
	/* TODO: buf_space checking */
	memcpy(buf, dmac_codes[instr].enc_buf, dmac_codes[instr].enc_buf_len);
	len += dmac_codes[instr].enc_buf_len;

	/* TODO: Parameter checking */
	switch (instr) {
	case DMAC_INSTR_DMAEND:
	case DMAC_INSTR_DMASTZ:
	case DMAC_INSTR_DMAKILL:
	case DMAC_INSTR_DMAWMB:
	case DMAC_INSTR_DMANOP:
		/* no parameter needed */
		break;

	case DMAC_INSTR_DMAFLUSHP:
		/* Fill additional parameters */
		buf[1] |= (param->periph) << 3;	// shift to bit 11
		break;

	case DMAC_INSTR_DMAGO:
		// Fill additional parameters
		if (param->bits.b.ns)
			buf[0] |= 0x2;
		else
			buf[0] &= ~0x2;
		buf[1] = param->cn & 0x7;
//#warning "rewrite this"
		Dmac_Cmd_Write32(&buf[2], param->imm32.i.val);
		//memcpy (&buf[2],&(param->imm32.i.val),4);
		break;

	case DMAC_INSTR_DMALD:
	case DMAC_INSTR_DMAST:
		// Fill additional parameters
		buf[0] &= 0xFC;
		if (param->bits.b.x)
			buf[0] |= 0x1;
		else
			buf[0] &= ~0x1;
		if (param->bits.b.bs)
			buf[0] |= 0x2;
		else
			buf[0] &= ~0x2;
		break;

	case DMAC_INSTR_DMALP:
		buf[0] &= (~0x2);
		if (param->bits.b.lc)
			buf[0] |= 0x2;
		buf[1] = param->iter;
		break;

	case DMAC_INSTR_DMALPEND:
		// Fill additional parameters
		buf[0] = 0x28;
		if (param->bits.b.x)
			buf[0] |= 0x1;
		if (param->bits.b.bs)
			buf[0] |= 0x2;
		if (param->bits.b.lc)
			buf[0] |= 0x4;
		if (param->bits.b.nf)
			buf[0] |= 0x10;
		buf[1] = param->backwards_jump;
		break;

	case DMAC_INSTR_DMAMOV:
		// Fill additional parameters
		buf[1] = (param->rd) & 0x7;
//#warning "rewrite this"
		Dmac_Cmd_Write32(&buf[2], param->imm32.i.val);
		//memcpy (&buf[2],&(param->imm32.i.val),4);
		break;

	case DMAC_INSTR_DMAWFE:
		buf[1] = 0x0;
		if (param->bits.b.i)
			buf[1] |= 0x2;
		buf[1] |= (param->event_num) << 3;	// shift to bit 11
		break;

	case DMAC_INSTR_DMASEV:
		buf[1] |= (param->event_num) << 3;	// shift to bit 11
		break;

	case DMAC_INSTR_DMAWFP:
		if (param->bits.b.p)
			buf[0] |= 0x1;
		else
			buf[0] &= ~0x1;
		if (param->bits.b.bs)
			buf[0] |= 0x2;
		else
			buf[0] &= ~0x2;
		buf[1] |= (param->periph) << 3;	// shift to bit 11
		break;

	case DMAC_INSTR_DMALDP:
	case DMAC_INSTR_DMASTP:
		// Fill additional parameters
		if (param->bits.b.bs)
			buf[0] |= 0x2;
		else
			buf[0] &= ~0x2;
		buf[1] |= (param->periph) << 3;	// shift to bit 11
		break;
	case DMAC_INSTR_DMAADDH:
		if(param->ra)
			buf[0] |=0x2;
		else
			buf[0] &= ~0x2;

		buf[1] = param->imm16 & 0xFF;
		buf[2] = (param->imm16 >> 8) & 0xFF;
			
		break;

	default:
		printk("%s: unknown instr (%d)\r\n", __FUNCTION__, instr);
		break;
	}
	dma_ch->microcode_size += len;
#ifdef DEBUG_GDMA
	printk("%s:%d: out with length %d\n", __FUNCTION__, __LINE__,
	       dma_ch->microcode_size);
	{
		int foo = 0;
		uint8_t *foop = dma_ch->microcode;
		printk("Dumping the buffer -- ");
		for (foo = 0; foo < dma_ch->microcode_size; foo++)
			printk("%x ", *(foop + foo));
		printk(" -- done.\n");
	}
#endif
	return len;
}

static int dmac_exec_ucode(int ucode_channel, int ch)
{
	uint8_t i, dbg_instr_0_shift_base, dbg_instr_2_shift_base, dbg_cmd_len,
	    *dbg_cmd_buf;
	uint32_t dbg1_val, dbg2_val;
	dmac_channel_t *dma_ch = dmac_channels[ucode_channel];

	if (!dma_ch->microcode_size) {
		printk("%s: No instructions have been created\n", __FUNCTION__);
		return -1;
	}

	dbg_cmd_buf = dma_ch->microcode;
	dbg_cmd_len = dma_ch->microcode_size;	
#ifdef DEBUG_GDMA
	{
		int tmp;
		uint8_t *tmpp = dbg_cmd_buf;
		printk
		    ("Executing the code for channel %d, with instrn len %d\n",
		     ch, dma_ch->microcode_size);
		printk("Dumping microcode : ");
		for (tmp = 0; tmp < dbg_cmd_len; tmp++)
			printk("%x ", *tmpp++);
		printk("\n");
	}
#endif

	spin_lock(&dma_mgr_lock);

	/* 3. Poll the Debug Status Register */
	while (DMAC_DBGSTATUS & DMAC_DBG_BUSY_BIT) ;

	/* 4. Write to the Debug Instrution-X Register */
	dbg1_val = 0;
	dbg2_val = 0;

	dbg_instr_0_shift_base = DMAC_DBG_INSTR_0_SHIFT;
	dbg_instr_2_shift_base = DMAC_DBG_INSTR_2_SHIFT;
	for (i = 0; i < dbg_cmd_len; i++) {
		uint8_t tmp_val = dbg_cmd_buf[i];
		switch (i) {
		case 0:
		case 1:
			dbg1_val |= (tmp_val << dbg_instr_0_shift_base);
			dbg_instr_0_shift_base += 8;
			break;
		case 2:
		case 3:
		case 4:
		case 5:
			tmp_val = dbg_cmd_buf[i];
			dbg2_val |= (tmp_val << dbg_instr_2_shift_base);
			dbg_instr_2_shift_base += 8;
			break;
		default:
			printk("BUG here ... DEBUG\n");
			break;
		}
	}

	// Fill channel field
	if (ch >= DMAC_CHMGR) {
		dbg1_val &= (~DMAC_DBG_THREAD_BIT);
	} else {
		dbg1_val |= DMAC_DBG_THREAD_BIT;
		dbg1_val |=
		    ((ch & DMAC_DBG_CH_NUM_BIT_MASK) << DMAC_DBG_CH_NUM_SHIFT);
	}

#ifdef DEBUG_GDMA
	{
		printk("dbg1_val: %x, dbg2_val: %x\n", dbg1_val, dbg2_val);
	}
#endif

	DMAC_DBGINST0 = dbg1_val;
	DMAC_DBGINST1 = dbg2_val;

	/* 5. Writing zero to the Debug Command Register */
	DMAC_DBGCMD = 0x0;

	spin_unlock(&dma_mgr_lock);
	return 0;
}

#define MAX_SINGLE_INSTR_LEN          8	/* TODO */

static int dmac_channel_state_init(int ch_num)
{
	int instr_len = dmac_create_instr(ch_num, DMAC_INSTR_DMAKILL, NULL);

	if (instr_len < 0) {
		printk("dmac_create_instr failed \n");
		return -1;
	}

	dmac_exec_ucode(ch_num, ch_num);

	if (dmac_channels[ch_num])
		dmac_channels[ch_num]->microcode_size = 0;
	else
		printk("BUG HERE !! DEBUG .. \n");

	return 0;
}

static irqreturn_t dmac_irq_handler(int irq, void *dev_id)
{
	uint32_t irq_status = 0;
	uint8_t event_status = 0, channel_no = 0;
	dmac_channel_t *chan = NULL;

	irq_status = DMAC_INTSTATUS;	/* TODO: Get Interrupt status */
#ifdef DEBUG_GDMA
	printk("Dumping the interrupt status register %x\n", irq_status);
#endif

	if (!irq_status) {
#ifdef DEBUG_GDMA
		printk("%s: Probably a DMAC Fault !!%x\n", __FUNCTION__,
		       irq_status);
		pl330_dump_regs();
#endif
		return IRQ_NONE;
	}

//	if (irq_status >= MIN_EVENT_NUM) {
//		printk(KERN_CRIT
//		       "Event interrupt handler..(%d) Not implemented\n",
//		       irq_status);
//		return IRQ_NONE;
//	}

	event_status = irq_status & 0xff;
	/* Clear Interrupt */
	DMAC_INTCLR |= (irq_status & 0xff);

	while (event_status) {
		if (event_status & 0x1) {
			chan = dmac_channels[channel_no];
			if (chan->intr_handler && chan->in_use)
				chan->intr_handler(chan->handler_args);
		}
		event_status >>= 1;
		channel_no++;
	}
	return IRQ_HANDLED;
}

#ifdef CONFIG_SILICON
static void cns3xxx_dmac_hw_init(void)
{
	/* enable GDMA clock*/
	cns3xxx_pwr_clk_en(CNS3XXX_PWR_CLK_EN(GDMA));
	/* do software reset*/
	cns3xxx_pwr_soft_rst(CNS3XXX_PWR_SOFTWARE_RST(GDMA));
}

#endif

/*
 * dmac_init
 */
int dmac_init(void)
{
	int i, irqno = DMAC_IRQNO_BASE;

	printk(KERN_INFO "Initializing CNS3XXX DMA controller \n");
#ifdef CONFIG_SILICON
	cns3xxx_dmac_hw_init();
#endif
	memset(dmac_channels, 0, sizeof(dmac_channel_t *) * (MAX_DMA_CHANNELS + (MIN_EVENT_NUM-1)));

	spin_lock_init(&dma_mgr_lock);

	for (i = 0; i < (MAX_DMA_CHANNELS + (MIN_EVENT_NUM-1)); i++) {
		dmac_channels[i] = kmalloc(sizeof(dmac_channel_t), GFP_KERNEL);

		if (dmac_channels[i] == NULL) {
			printk("Unable to allocate memory for channel %d \n",
			       i);
			return -ENOMEM;
		}

		memset(dmac_channels[i], 0, sizeof(dmac_channel_t));
	}

	/* Moves all the DMA channels to the Stopped state */
	for (i = 0; i < MAX_DMA_CHANNELS; i++)
		dmac_channel_state_init(i);

	for (i = 0; i < MAX_INTR_EVENTS; i++)
		dmac_events[i] = -1;

	/* Clear spurious interrupts */
	DMAC_INTCLR = 0xffffffff;
	DMAC_INTEN = 0xff; //Enable 8 interrupt 0x03;	/* Enabling interrupts IRQ[0], IRQ[1] */

	/* TODO: error interrupt Right now using the same irq handler,
	 * and reporting error inside the handler
	 */
	if (request_irq(ERROR_INTR, dmac_irq_handler, 0, "DMAC-ERR", NULL)) {
		printk(KERN_CRIT "failed to request DMAC-ERR interrupt.\n");
		return -ENOENT;
	}

	do {
		if (request_irq(irqno, dmac_irq_handler, 0, "DMAC", NULL)) {
			printk(KERN_CRIT "failed to request DMAC interrupt.\n");
			return -ENOENT;
		}
	} while (++irqno < (DMAC_IRQNO_BASE + MIN_EVENT_NUM));

	return 0;
}

EXPORT_SYMBOL(dmac_init);

/*
 * dmac_get_channel
 */
int dmac_get_channel(int (*handler) (void *), void *handler_args)
{
	int i;

	for (i = 0; i < MAX_DMA_CHANNELS; i++)
		if (dmac_channels[i]->in_use == 0) {
			dmac_channel_t *dmac_ch = dmac_channels[i];

			dmac_ch->microcode_size = 0;
			dmac_ch->in_use = 1;
			dmac_ch->intr_handler = handler;
			dmac_ch->handler_args = handler_args;

			/* TODO enable interrupts for that channel */
//			dmac_channel_state_init(i);
			return i;
		}

	return -1;
}

int dmac_get_channel_ex(int channel, int (*handler) (void *), void *handler_args)
{
	if((channel >= 0) && (channel < MAX_DMA_CHANNELS) && (dmac_channels[channel]->in_use == 0)) {
			dmac_channel_t *dmac_ch = dmac_channels[channel];

			dmac_ch->microcode_size = 0;
			dmac_ch->in_use = 1;
			dmac_ch->intr_handler = handler;
			dmac_ch->handler_args = handler_args;

			/* TODO enable interrupts for that channel */
//			dmac_channel_state_init(channel);
			return channel;
	}

	return -1;
}

EXPORT_SYMBOL(dmac_get_channel);
EXPORT_SYMBOL(dmac_get_channel_ex);

/*
 * dmac_release_channel
 */
int dmac_release_channel(int chan)
{
	dmac_channel_t *dma_ch;

	if (chan < 0 || chan > 7)
		return -1;

	dma_ch = dmac_channels[chan];
	if (!dma_ch->in_use)
		return -1;

	dma_ch->in_use = 0;
	dma_ch->microcode_size = 0;
	dma_ch->intr_handler = 0;
	dma_ch->handler_args = 0;

	/* TODO enable interrupts for that channel */
	dmac_channel_state_init(chan);

	return 0;
}

EXPORT_SYMBOL(dmac_release_channel);

/*
 *
 */
int dmac_get_event(int chan, int event_num)
{
	if ((event_num < MIN_EVENT_NUM) || (event_num > MAX_INTR_EVENTS)) {
		return -1;
	}

	if (dmac_events[event_num] == -1) {
		dmac_channel_t *dmac_ch = dmac_channels[chan];
		dmac_events[event_num] = chan;
		dmac_ch->notifications_used |= (1 << event_num);
		return 0;
	}
	return -1;
}

EXPORT_SYMBOL(dmac_get_event);

/*
 *
 */
int dmac_release_event(int chan, int event_num)
{
	if (dmac_events[event_num] != chan)
		return -1;

	dmac_events[event_num] = -1;
	dmac_channels[chan]->notifications_used ^= (1 << event_num);
	return 0;
}

EXPORT_SYMBOL(dmac_release_event);

static int get_bpb_val(int bpb)
{
	int i = bpb;
	int retval = -1;
	while (i) {
		retval += 0x1;
		i /= 2;
	}
	return retval;
}

/* @src_inc - src address auto increment
 * @s_bpb - src bytes per burst
 * @s_dt  - src num of data transfers
 * @dst_inc - dst address auto increment
 * @d_bpb - dst bytes per burst
 * @d_dt - dst data transfers
 * @swap - swapping bytes
 */
uint32_t dmac_create_ctrlval(int src_inc, int s_bpb, int s_dt, int dst_inc,
			     int d_bpb, int d_dt, int swap)
{
	if (!
	    ((s_bpb == 1) || (s_bpb == 2) || (s_bpb == 4) || (s_bpb == 8)
	     || (s_bpb == 16)
	     || (s_bpb == 32) || (s_bpb == 64) || (s_bpb == 128))) {
		printk
		    ("INVALID s_bpb parameter ... setting default and proceeding\n");
		s_bpb = 4;
	}
	if (!
	    ((d_bpb == 1) || (d_bpb == 2) || (d_bpb == 4) || (d_bpb == 8)
	     || (d_bpb == 16)
	     || (d_bpb == 32) || (d_bpb == 64) || (d_bpb == 128))) {
		printk
		    ("INVALID d_bpb parameter ... setting default and proceeding\n");
		d_bpb = 4;
	}

	if ((s_dt < 1) || (s_dt > 16)) {
		printk
		    ("INVALID s_dt parameter ... setting default and proceeding\n");
		s_dt = 1;
	}
	if ((d_dt < 1) || (d_dt > 16)) {
		printk
		    ("INVALID d_dt parameter ... setting default and proceeding\n");
		d_dt = 1;
	}
	return (((src_inc & 0x1) << 0) |
		((get_bpb_val(s_bpb) & 0x7) << 1) |
		((s_dt - 1) << 4) |
		(0x2 << 8) |
		(0x0 << 11) |
		((dst_inc & 0x1) << 14) |
		((get_bpb_val(d_bpb) & 0x7) << 15) |
		((d_dt - 1) << 18) | (0x2 << 22) | (0x0 << 25) | (swap << 28)
	    );
}

EXPORT_SYMBOL(dmac_create_ctrlval);

void pl330_dump_regs(void)
{
	printk("Read Periph Id 0 for GDMAC is %x\n", DMAC_MEM_MAP_VALUE(0xFE0));
	printk("DS Register: %x\n", DMAC_MEM_MAP_VALUE(0x0));
	printk("Conf Reg 0 : %x\n", DMAC_MEM_MAP_VALUE(0xE00));
	printk("Conf Reg 1 : %x\n", DMAC_MEM_MAP_VALUE(0xE04));
	printk("Conf Reg 2 : %x\n", DMAC_MEM_MAP_VALUE(0xE08));
	printk("Conf Reg 3 : %x\n", DMAC_MEM_MAP_VALUE(0xE0C));
	printk("Conf Reg 4 : %x\n", DMAC_MEM_MAP_VALUE(0xE10));
	printk("Conf Reg d : %x\n", DMAC_MEM_MAP_VALUE(0xE14));

	printk("Dumping the status registers \n");
	printk("INTEN Register: %x\n", DMAC_MEM_MAP_VALUE(0x20));
	printk("ES Register: %x\n", DMAC_MEM_MAP_VALUE(0x24));
	printk("INTSTAT Register: %x\n", DMAC_MEM_MAP_VALUE(0x28));
	printk("FSDM Register: %x\n", DMAC_MEM_MAP_VALUE(0x30));
	printk("FSC Register: %x\n", DMAC_MEM_MAP_VALUE(0x34));
	printk("FTM Register: %x\n", DMAC_MEM_MAP_VALUE(0x38));
	printk("FTC0 Register: %x\n", DMAC_MEM_MAP_VALUE(0x40));
	printk("FTC1 Register: %x\n", DMAC_MEM_MAP_VALUE(0x44));
	printk("CS0 Register: %x\n", DMAC_MEM_MAP_VALUE(0x100));
	printk("CPC0 Register: %x\n", DMAC_MEM_MAP_VALUE(0x104));
	printk("CS1 Register: %x\n", DMAC_MEM_MAP_VALUE(0x108));
	printk("CPC1 Register: %x\n", DMAC_MEM_MAP_VALUE(0x10C));
	printk("SA0 Register: %x\n", DMAC_MEM_MAP_VALUE(0x400));
	printk("SA1 Register: %x\n", DMAC_MEM_MAP_VALUE(0x420));
	printk("DA0 Register: %x\n", DMAC_MEM_MAP_VALUE(0x404));
	printk("DA1 Register: %x\n", DMAC_MEM_MAP_VALUE(0x424));
	return;
}

EXPORT_SYMBOL(pl330_dump_regs);

/*
 *
 */
uint32_t DMAC_READ_CHREGS(int chan, chregs_t reg)
{
	int step = 0, base = 0;

	switch (reg) {
	case PL330_FTC:
		base = 0x40;
		step = chan * 0x4;
		break;
	case PL330_CS:
		base = 0x100;
		step = chan * 0x8;
		break;
	case PL330_CPC:
		base = 0x104;
		step = chan * 0x8;
		break;
	case PL330_SA:
		base = 0x400;
		step = chan * 0x20;
		break;
	case PL330_DA:
		base = 0x404;
		step = chan * 0x20;
		break;
	case PL330_CC:
		base = 0x408;
		step = chan * 0x20;
		break;
	case PL330_LC0:
		base = 0x40C;
		step = chan * 0x20;
		break;
	case PL330_LC1:
		base = 0x410;
		step = chan * 0x20;
		break;
	default:
		printk("Wrong argument to function %s\n", __FUNCTION__);
	}
	return DMAC_MEM_MAP_VALUE(base + step);
}

EXPORT_SYMBOL(DMAC_READ_CHREGS);
