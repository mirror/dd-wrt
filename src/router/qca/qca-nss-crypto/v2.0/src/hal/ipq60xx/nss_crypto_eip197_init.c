/*
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */
#include <linux/firmware.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/memory.h>
#include <linux/delay.h>
#include <linux/io.h>

#include <nss_api_if.h>
#include <nss_crypto_defines.h>
#include <nss_crypto_api.h>
#include "nss_crypto_hlos.h"
#include "nss_crypto_ctrl.h"
#include "nss_crypto_eip197.h"

#define NSS_CRYPTO_EIP197_MAX_STR_LENGTH 64
#define NSS_CRYPTO_EIP197_NEW_LINE 16

#define NSS_CRYPTO_EIP197_REG_DUMP_SZ \
	((2 + NSS_CRYPTO_EIP197_NEW_LINE) * NSS_CRYPTO_EIP197_MAX_STR_LENGTH)
		/* name of the register and value stored in the register */

/*
 * nss_crypto_eip197_regmap
 *	Structure for eip197 register dump
 */
struct nss_crypto_eip197_regmap {
	const uint8_t *name;
	uint32_t offset;	/* Register offset with respect to EIP197 Base */
};

/*
 * g_dma_reg_map
 *	Entries for DMA register dump
 */
struct nss_crypto_eip197_regmap g_dma_reg_map[] = {
	{.name = "cmd_prep_count", .offset = NSS_CRYPTO_EIP197_HIA_CDR_COUNT(0)},
	{.name = "cmd_proc_count", .offset = NSS_CRYPTO_EIP197_HIA_CDR_PROC_COUNT(0)},
	{.name = "cmd_prep_ptr", .offset = NSS_CRYPTO_EIP197_HIA_CDR_PREP_PNTR(0)},
	{.name = "cmd_proc_ptr", .offset = NSS_CRYPTO_EIP197_HIA_CDR_PROC_PNTR(0)},
	{.name = "cmd_dma_error", .offset = NSS_CRYPTO_EIP197_HIA_CDR_STAT(0)},
	{.name = "res_prep_count", .offset = NSS_CRYPTO_EIP197_HIA_RDR_COUNT(0)},
	{.name = "res_proc_count", .offset = NSS_CRYPTO_EIP197_HIA_RDR_PROC_COUNT(0)},
	{.name = "res_prep_ptr", .offset = NSS_CRYPTO_EIP197_HIA_RDR_PREP_PNTR(0)},
	{.name = "res_proc_ptr", .offset = NSS_CRYPTO_EIP197_HIA_RDR_PROC_PNTR(0)},
	{.name = "res_dma_error", .offset = NSS_CRYPTO_EIP197_HIA_RDR_STAT(0)},
};

/*
 * g_hia_reg_map
 *	Entries for HIA register dump
 */
struct nss_crypto_eip197_regmap g_hia_reg_map[] = {
	{.name = "data_fetch_dma_err", .offset = NSS_CRYPTO_EIP197_HIA_DFE_THR_STAT},
	{.name = "data_store_dma_err", .offset = NSS_CRYPTO_EIP197_HIA_DSE_THR_STAT},
	{.name = "la_cmd_dma_err", .offset = NSS_CRYPTO_EIP197_HIA_LASIDE_BASE_ADDR_LO},
	{.name = "la_cmd_res_err", .offset = NSS_CRYPTO_EIP197_HIA_LASIDE_BASE_ADDR_HI},
	{.name = "hia_timeout_err", .offset = NSS_CRYPTO_EIP197_HIA_MST_TIMEOUT_ERR},
	{.name = "axi_err", .offset = NSS_CRYPTO_EIP197_HIA_MST_CTRL},
};

/*
 * g_pe_reg_map
 *	Entries for PE register dump
 */
struct nss_crypto_eip197_regmap g_pe_reg_map[] = {
	{.name = "ipue_ctrl", .offset = NSS_CRYPTO_EIP197_PE_ICE_PUE_CTRL},
	{.name = "ipue_debug", .offset = NSS_CRYPTO_EIP197_PE_ICE_PUE_DBG},
	{.name = "ifpp_ctrl", .offset = NSS_CRYPTO_EIP197_PE_ICE_FPP_CTRL},
	{.name = "ifpp_debug", .offset = NSS_CRYPTO_EIP197_PE_ICE_FPP_DBG},
	{.name = "contex_error", .offset = NSS_CRYPTO_EIP197_PE_EIP96_CONTEXT_CTRL},
	{.name = "opue_ctrl", .offset = NSS_CRYPTO_EIP197_PE_OCE_PUE_CTRL},
	{.name = "opue_debug", .offset = NSS_CRYPTO_EIP197_PE_OCE_PUE_DBG},
	{.name = "ofpp_ctrl", .offset = NSS_CRYPTO_EIP197_PE_OCE_FPP_CTRL},
	{.name = "ofpp_debug", .offset = NSS_CRYPTO_EIP197_PE_OCE_FPP_DBG},
	{.name = "current_iv0", .offset = NSS_CRYPTO_EIP197_PE_IV0},
	{.name = "current_iv1", .offset = NSS_CRYPTO_EIP197_PE_IV1},
	{.name = "current_iv2", .offset = NSS_CRYPTO_EIP197_PE_IV2},
	{.name = "current_iv3", .offset = NSS_CRYPTO_EIP197_PE_IV3},
#if defined(NSS_CRYPTO_DEBUG_KEYS)
	{.name = "cipher_key0", .offset = NSS_CRYPTO_EIP197_PE_KEY0},
	{.name = "cipher_key1", .offset = NSS_CRYPTO_EIP197_PE_KEY1},
	{.name = "cipher_key2", .offset = NSS_CRYPTO_EIP197_PE_KEY2},
	{.name = "cipher_key3", .offset = NSS_CRYPTO_EIP197_PE_KEY3},
	{.name = "cipher_key4", .offset = NSS_CRYPTO_EIP197_PE_KEY4},
	{.name = "cipher_key5", .offset = NSS_CRYPTO_EIP197_PE_KEY5},
	{.name = "cipher_key6", .offset = NSS_CRYPTO_EIP197_PE_KEY6},
	{.name = "cipher_key7", .offset = NSS_CRYPTO_EIP197_PE_KEY7},
	{.name = "auth_ipad0", .offset = NSS_CRYPTO_EIP197_PE_IDIGEST0},
	{.name = "auth_ipad1", .offset = NSS_CRYPTO_EIP197_PE_IDIGEST1},
	{.name = "auth_ipad2", .offset = NSS_CRYPTO_EIP197_PE_IDIGEST2},
	{.name = "auth_ipad3", .offset = NSS_CRYPTO_EIP197_PE_IDIGEST3},
	{.name = "auth_ipad4", .offset = NSS_CRYPTO_EIP197_PE_IDIGEST4},
	{.name = "auth_ipad5", .offset = NSS_CRYPTO_EIP197_PE_IDIGEST5},
	{.name = "auth_ipad6", .offset = NSS_CRYPTO_EIP197_PE_IDIGEST6},
	{.name = "auth_ipad7", .offset = NSS_CRYPTO_EIP197_PE_IDIGEST7},
	{.name = "auth_opad0", .offset = NSS_CRYPTO_EIP197_PE_ODIGEST0},
	{.name = "auth_opad1", .offset = NSS_CRYPTO_EIP197_PE_ODIGEST1},
	{.name = "auth_opad2", .offset = NSS_CRYPTO_EIP197_PE_ODIGEST2},
	{.name = "auth_opad3", .offset = NSS_CRYPTO_EIP197_PE_ODIGEST3},
	{.name = "auth_opad4", .offset = NSS_CRYPTO_EIP197_PE_ODIGEST4},
	{.name = "auth_opad5", .offset = NSS_CRYPTO_EIP197_PE_ODIGEST5},
	{.name = "auth_opad6", .offset = NSS_CRYPTO_EIP197_PE_ODIGEST6},
	{.name = "auth_opad7", .offset = NSS_CRYPTO_EIP197_PE_ODIGEST7},
#endif
	{.name = "trc_dma_read_err", .offset = NSS_CRYPTO_EIP197_TRC_PARAMS},
	{.name = "trc_dma_wr_err", .offset = NSS_CRYPTO_EIP197_TRC_PARAMS},
};

/*
 * nss_crypto_eip197_read_dma_reg()
 *	Regdump for Command and Result Descriptor Ring
 */
ssize_t nss_crypto_eip197_read_dma_reg(struct file *filep, char __user *buffer, size_t count, loff_t *ppos)
{
	struct nss_crypto_engine *eng = filep->private_data;
	struct nss_crypto_node *node = eng->node;
	int max_dma_ring = hweight32(node->dma_mask);
	void *base_addr = eng->crypto_vaddr;
	ssize_t max_buf_len, ret;
	int i, ring_id = 0;
	size_t len = 0;
	uint32_t val;
	char *buf;

	/*
	 * (Lines of output) * (Max string length)
	 */
	max_buf_len = ARRAY_SIZE(g_dma_reg_map) * NSS_CRYPTO_EIP197_REG_DUMP_SZ * max_dma_ring;

	buf = vzalloc(max_buf_len);
	if (!buf) {
		nss_crypto_warn("Failed to allocate memory for statistic buffer");
		return 0;
	}

	/*
	 * We will dump debugfs entry for all the dma rings available
	 */
	for ( ; ring_id < max_dma_ring; ring_id++) {
		/*
		 * if descriptor ring size is 0, we do not display the registers
		 */
		val = ioread32(base_addr + NSS_CRYPTO_EIP197_HIA_CDR_RING_SIZE(ring_id));
		if (!val)
			continue;

		len += snprintf(buf + len, max_buf_len - len, "Ring_id:%d\n", ring_id);
		for (i = 0; i < ARRAY_SIZE(g_dma_reg_map); i++) {
			val = ioread32(base_addr + (g_dma_reg_map[i].offset * (ring_id + 1)));
			len += snprintf(buf + len, max_buf_len - len, "%s - 0x%x\n", g_dma_reg_map[i].name, val);
		}
	}

	ret = simple_read_from_buffer(buffer, count, ppos, buf, len);
	vfree(buf);

	return ret;
}

/*
 * nss_crypto_eip197_read_hia_reg()
 *	Regdump for Command and Result Descriptor Ring
 */
ssize_t nss_crypto_eip197_read_hia_reg(struct file *filep, char __user *buffer, size_t count, loff_t *ppos)
{
	struct nss_crypto_engine *eng = filep->private_data;
	void *base_addr = eng->crypto_vaddr;
	ssize_t max_buf_len, ret;
	size_t len = 0;
	uint32_t val;
	char *buf;
	int i;

	/*
	 * (Lines of output) * (Max string length)
	 */
	max_buf_len = ARRAY_SIZE(g_hia_reg_map) * NSS_CRYPTO_EIP197_REG_DUMP_SZ;

	buf = vzalloc(max_buf_len);
	if (!buf) {
		nss_crypto_warn("Failed to allocate memory for statistic buffer");
		return 0;
	}

	for (i = 0; i < ARRAY_SIZE(g_hia_reg_map); i++) {
		val = ioread32(base_addr + g_hia_reg_map[i].offset);
		len += snprintf(buf + len, max_buf_len - len, "%s - 0x%x\n", g_hia_reg_map[i].name, val);
	}

	ret = simple_read_from_buffer(buffer, count, ppos, buf, len);
	vfree(buf);

	return ret;
}

/*
 * nss_crypto_eip197_read_pe_reg()
 *	Regdump for Command and Result Descriptor Ring
 */
ssize_t nss_crypto_eip197_read_pe_reg(struct file *filep, char __user *buffer, size_t count, loff_t *ppos)
{
	struct nss_crypto_engine *eng = filep->private_data;
	void *base_addr = eng->crypto_vaddr;
	ssize_t max_buf_len, ret;
	ssize_t len = 0;
	uint32_t val;
	char *buf;
	int i;

	/*
	 * (Lines of output) * (Max string length)
	 */
	max_buf_len = ARRAY_SIZE(g_pe_reg_map) * NSS_CRYPTO_EIP197_REG_DUMP_SZ;

	buf = vzalloc(max_buf_len);
	if (!buf) {
		nss_crypto_warn("Failed to allocate memory for statistic buffer");
		return 0;
	}

	for (i = 0; i < ARRAY_SIZE(g_pe_reg_map); i++) {
		val = ioread32(base_addr + g_pe_reg_map[i].offset);
		len += snprintf(buf + len, max_buf_len - len, "%s - 0x%x\n", g_pe_reg_map[i].name, val);
	}

	ret = simple_read_from_buffer(buffer, count, ppos, buf, len);
	vfree(buf);

	return ret;
}

/*
 * nss_crypto_eip197_global_reset_status()
 *	returns true if reset is successful
 */
static bool nss_crypto_eip197_hw_reset_status(void __iomem *base_addr)
{
	uint32_t status;

	/*
	 * Data Fetch Engine and Data Store Engine status are checked
	 * in order to determine whether reset is completed.
	 */
	status = ioread32(base_addr + NSS_CRYPTO_EIP197_HIA_DFE_THR_STAT);
	if (status != NSS_CRYPTO_EIP197_DFE_RST_DONE)
		return false;

	status = ioread32(base_addr + NSS_CRYPTO_EIP197_HIA_DSE_THR_STAT);
	if (status != NSS_CRYPTO_EIP197_DSE_RST_DONE)
		return false;

	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_HIA_DFE_THR_CTRL);
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_HIA_DSE_THR_CTRL);

	return true;
}

/*
 * nss_crypto_eip197_hw_reset()
 *	HW reset routine
 */
static int nss_crypto_eip197_hw_reset(void __iomem *base_addr)
{
	/*
	 * Reset Host Interface Adapter(HIA) module
	 */
	iowrite32(NSS_CRYPTO_EIP197_HIA_RST, base_addr + NSS_CRYPTO_EIP197_HIA_DFE_THR_CTRL);
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_HIA_DFE_CFG);
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_HIA_RA_PRIO0);
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_HIA_RA_PRIO1);
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_HIA_RA_PRIO2);
	iowrite32(NSS_CRYPTO_EIP197_HIA_RST, base_addr + NSS_CRYPTO_EIP197_HIA_RA_PE_CTRL);
	iowrite32(NSS_CRYPTO_EIP197_HIA_RST, base_addr + NSS_CRYPTO_EIP197_HIA_DSE_THR_CTRL);
	iowrite32(NSS_CRYPTO_EIP197_DSE_CFG_RESET, base_addr + NSS_CRYPTO_EIP197_HIA_DSE_CFG);
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_HIA_LASIDE_BASE_ADDR_LO);
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_HIA_LASIDE_BASE_ADDR_HI);
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_HIA_LASIDE_SLAVE_CTRL0);
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_HIA_LASIDE_MASTER_CTRL1);
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_HIA_INLINE_CTRL0);
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_PE_IN_DBUF_THR);
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_PE_IN_TBUF_THR);
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_PE_OUT_DBUF_THRES);
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_PE_OUT_DBUF_THRES);
	iowrite32(NSS_CRYPTO_EIP197_TOKEN_CTRL_RESET,
		base_addr + NSS_CRYPTO_EIP197_PE_EIP96_TOKEN_CTRL);

	/*
	 * wait for 10ms before polling the status
	 */
	mdelay(10);

	return nss_crypto_eip197_hw_reset_status(base_addr);
}

/*
 * nss_crypto_eip197_hw_setup_core()
 *	setup eip197 core
 */
static void nss_crypto_eip197_hw_setup_core(void __iomem *base_addr)
{
	uint32_t data;
	uint32_t val;

	/*
	 * HIA MASTER BURST SIZE update
	 */
	iowrite32(NSS_CRYPTO_EIP197_HIA_CFG, base_addr + NSS_CRYPTO_EIP197_MST_CTRL);

	/*
	 * Data Fetch Engine configuration
	 */
	iowrite32(NSS_CRYPTO_EIP197_DFE_CFG, base_addr + NSS_CRYPTO_EIP197_HIA_DFE_CFG);

	/*
	 * Data Store engine configuration
	 */
	iowrite32(NSS_CRYPTO_EIP197_DSE_CFG, base_addr + NSS_CRYPTO_EIP197_HIA_DSE_CFG);
	iowrite32(NSS_CRYPTO_EIP197_INDATA_THR, base_addr + NSS_CRYPTO_EIP197_PE_IN_DBUF_THR);
	iowrite32(NSS_CRYPTO_EIP197_INTOKEN_THR, base_addr + NSS_CRYPTO_EIP197_PE_IN_TBUF_THR);
	iowrite32(NSS_CRYPTO_EIP197_OUTDATA_THR, base_addr + NSS_CRYPTO_EIP197_PE_OUT_DBUF_THRES);

	/*
	 * This enable timeout counter, if packet engine hangs, this timeout can
	 * help to recover from hang
	 */
	iowrite32(NSS_CRYPTO_EIP197_TOKEN_CFG, base_addr + NSS_CRYPTO_EIP197_PE_EIP96_TOKEN_CTRL);
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_PE_EIP96_OUT_BUF_CTRL);

	/*
	 * Write LA FIFO BASE ADDR LOW WRITE
	 */
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_HIA_LASIDE_BASE_ADDR_LO);

	/*
	 * This bit is set for a command descriptor error, write 1 to clear the error
	 */
	iowrite32(NSS_CRYPTO_EIP197_HIA_RST, base_addr + NSS_CRYPTO_EIP197_HIA_LASIDE_SLAVE_CTRL0);

	/*
	 * LA master control write
	 */
	iowrite32(NSS_CRYPTO_EIP197_HIA_RST, base_addr + NSS_CRYPTO_EIP197_HIA_LASIDE_MASTER_CTRL1);
	iowrite32(NSS_CRYPTO_EIP197_INLINE_CFG, base_addr + NSS_CRYPTO_EIP197_HIA_INLINE_CTRL0);
	iowrite32(NSS_CRYPTO_EIP197_ENB_ALL_RINGS, base_addr + NSS_CRYPTO_EIP197_HIA_RA_PE_CTRL);

	/*
	 * Enable Data store engine processing thread
	 */
	data = ioread32(base_addr + NSS_CRYPTO_EIP197_HIA_DSE_THR_CTRL);
	data |= NSS_CRYPTO_EIP197_ENB_DSE_THREAD;
	iowrite32(data, base_addr + NSS_CRYPTO_EIP197_HIA_DSE_THR_CTRL);


	/* DRBG INIT */

	/*
	 * Set the engine in idle mode
	 */
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_DRBG_CONTROL);
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_DRBG_CONTROL);


	/*
	 * poll the ps_ai_write_ok bit
	 */
	val = ioread32(base_addr + NSS_CRYPTO_EIP197_DRBG_STATUS);
	while (!(val & NSS_CRYPTO_EIP197_DRBG_AI_STATUS_MASK)) {
		nss_crypto_info("DRBG module not ready for reseed");
		val = ioread32(base_addr + NSS_CRYPTO_EIP197_DRBG_STATUS);
	}

	/*
	 * TODO; check NSS_CRYPTO_EIP197_DRBG_GEN_BLOCK_SIZE default value
	 */

	/*
	 * Program the seed values
	 */
	iowrite32(NSS_CRYPTO_EIP197_DRBG_INITIAL_SEED0, base_addr + NSS_CRYPTO_EIP197_DRBG_PS_AI(0));
	iowrite32(NSS_CRYPTO_EIP197_DRBG_INITIAL_SEED1, base_addr + NSS_CRYPTO_EIP197_DRBG_PS_AI(1));
	iowrite32(NSS_CRYPTO_EIP197_DRBG_INITIAL_SEED2, base_addr + NSS_CRYPTO_EIP197_DRBG_PS_AI(2));
	iowrite32(NSS_CRYPTO_EIP197_DRBG_INITIAL_SEED3, base_addr + NSS_CRYPTO_EIP197_DRBG_PS_AI(3));
	iowrite32(NSS_CRYPTO_EIP197_DRBG_INITIAL_SEED4, base_addr + NSS_CRYPTO_EIP197_DRBG_PS_AI(4));
	iowrite32(NSS_CRYPTO_EIP197_DRBG_INITIAL_SEED5, base_addr + NSS_CRYPTO_EIP197_DRBG_PS_AI(5));
	iowrite32(NSS_CRYPTO_EIP197_DRBG_INITIAL_SEED6, base_addr + NSS_CRYPTO_EIP197_DRBG_PS_AI(6));
	iowrite32(NSS_CRYPTO_EIP197_DRBG_INITIAL_SEED7, base_addr + NSS_CRYPTO_EIP197_DRBG_PS_AI(7));
	iowrite32(NSS_CRYPTO_EIP197_DRBG_INITIAL_SEED8, base_addr + NSS_CRYPTO_EIP197_DRBG_PS_AI(8));
	iowrite32(NSS_CRYPTO_EIP197_DRBG_INITIAL_SEED9, base_addr + NSS_CRYPTO_EIP197_DRBG_PS_AI(9));
	iowrite32(NSS_CRYPTO_EIP197_DRBG_INITIAL_SEED10, base_addr + NSS_CRYPTO_EIP197_DRBG_PS_AI(10));
	iowrite32(NSS_CRYPTO_EIP197_DRBG_INITIAL_SEED11, base_addr + NSS_CRYPTO_EIP197_DRBG_PS_AI(11));

	/*
	 * Enable DRBG
	 */
	iowrite32(NSS_CRYPTO_EIP197_DRBG_CTRL_STUCKOUT | NSS_CRYPTO_EIP197_DRBG_CTRL_EN,
		base_addr + NSS_CRYPTO_EIP197_DRBG_CONTROL);
}

/*
 * nss_crypto_eip197_hw_setup_cache()
 *	setup EIP197 flow and transform cache
 */
void nss_crypto_eip197_hw_setup_cache(void __iomem *base_addr)
{
	void __iomem *addr;
	uint32_t val;
	int i;

	/*
	 * Enable Flow record cache 0
	 */
	iowrite32(NSS_CRYPTO_EIP197_ENB_FLOW_REC, base_addr + NSS_CRYPTO_EIP197_CS_RAM_CTRL);
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_FRC_ECCCTRL);

	/*
	 * Put Flow record cache into SW reset
	 */
	iowrite32(NSS_CRYPTO_EIP197_RST_FLOW_REC, base_addr + NSS_CRYPTO_EIP197_FRC_PARAMS);

	/*
	 * Initialise FLow record Cache
	 * Set Record administrative words
	 *	- Set Hash collision Head and Next pointers
	 *	- Set Previous and Next free list pointers
	 * Hash table words
	 *	- Set Hash collision head
	 */
	for (i = 0 ; i < NSS_CRYPTO_EIP197_MAX_FLOW_REC; i++) {
		addr = base_addr + NSS_CRYPTO_EIP197_CS_RAM_START + (i * 16);

		/*
		 * if its the first record then, then free_list_prev is loaded
		 * with 0x3ff, while all other records are loaded with record_index - 1
		 *
		 * if its the last record the, then free_list_next is loaded
		 * with 0x3ff, while all other records are loaded with record_index + 1
		 */
		if (i == 0)
			val = (NSS_CRYPTO_EIP197_EMPTY_REC << 10) | 0x1;
		else if (i == (NSS_CRYPTO_EIP197_MAX_FLOW_REC - 1))
			val = ((i - 1) << 10) | NSS_CRYPTO_EIP197_EMPTY_REC;
		else
			val =  ((i - 1) << 10) | (i + 1);

		iowrite32(NSS_CRYPTO_EIP197_RST_HASH_COL, addr);
		iowrite32(val, addr + 0x4);
		iowrite32(0, addr + 0x8);
		iowrite32(0, addr + 0xC);
	}

	/*
	 * reset hash collision bucket
	 */
	addr = base_addr + NSS_CRYPTO_EIP197_CS_RAM_START + NSS_CRYPTO_EIP197_FLOW_REC_END;
	for (i = 0; i < NSS_CRYPTO_EIP197_MAX_FRC_HASH_BUCKETS; i++)
		iowrite32(U32_MAX, addr + (0x4 * i));

	/*
	 * configure FRC control registers
	 */
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_CS_RAM_CTRL);
	iowrite32(NSS_CRYPTO_EIP197_FRC_FREECHAIN_CFG, base_addr + NSS_CRYPTO_EIP197_FRC_FREECHAIN);
	iowrite32(NSS_CRYPTO_EIP197_FRC_PARAMS2_CFG, base_addr + NSS_CRYPTO_EIP197_FRC_PARAMS2);
	iowrite32(NSS_CRYPTO_EIP197_FRC_PARAMS_CFG, base_addr + NSS_CRYPTO_EIP197_FRC_PARAMS);

	/*
	 * Enable Transform record cache 0
	 */
	iowrite32(NSS_CRYPTO_EIP197_ENB_TRANS_REC, base_addr + NSS_CRYPTO_EIP197_CS_RAM_CTRL);

	/*
	 * Put Transform record cache into SW reset
	 */
	iowrite32(0x1, base_addr + NSS_CRYPTO_EIP197_TRC_PARAMS);

	/*
	 * Initialise Transform record Cache
	 * Set Record administrative words
	 *	- Set Hash collision Head and Next pointers
	 *	- Set Previous and Next free list pointers
	 * Hash table words
	 *	- Set Hash collision head
	 */
	for (i = 0 ; i < NSS_CRYPTO_EIP197_MAX_TRANS_REC; i++) {
		addr = base_addr + NSS_CRYPTO_EIP197_CS_RAM_START + (i * 16);

		/*
		 * if its the first record them, then free_list_prev is loaded
		 * with 0x3ff, while all other records are loaded with record_index - 1
		 *
		 * if its the last record the, then free_list_next is loaded
		 * with 0x3ff, while all other records are loaded with record_index + 1
		 */
		if (i == 0)
			val = ((NSS_CRYPTO_EIP197_EMPTY_REC << 10) | 0x1);
		else if (i == (NSS_CRYPTO_EIP197_MAX_TRANS_REC - 1))
			val = (((i - 1) << 10) | NSS_CRYPTO_EIP197_EMPTY_REC);
		else
			val =  (((i - 1) << 10) | (i + 1));

		iowrite32(NSS_CRYPTO_EIP197_RST_HASH_COL, addr);
		iowrite32(val, addr + 0x4);
		iowrite32(0, addr + 0x8);
		iowrite32(0, addr + 0xC);
	}

	/*
	 * reset hash collision bucket
	 */
	addr = base_addr + NSS_CRYPTO_EIP197_CS_RAM_START + NSS_CRYPTO_EIP197_TRANS_REC_END;
	for (i = 0; i < NSS_CRYPTO_EIP197_MAX_TRC_HASH_BUCKETS; i++)
		iowrite32(U32_MAX, addr + (0x4 * i));

	/*
	 * configure TRC configuration parameters
	 */
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_CS_RAM_CTRL);
	iowrite32(NSS_CRYPTO_EIP197_TRC_FREECHAIN_CFG, base_addr + NSS_CRYPTO_EIP197_TRC_FREECHAIN);
	iowrite32(NSS_CRYPTO_EIP197_TRC_PARAMS2_CFG, base_addr + NSS_CRYPTO_EIP197_TRC_PARAMS2);
	iowrite32(NSS_CRYPTO_EIP197_TRC_PARAMS_CFG, base_addr + NSS_CRYPTO_EIP197_TRC_PARAMS);

	/*
	 * Configure FLUE engine initial Hash value, This initial hash
	 * will be used by the engine to  compute flow hash id
	 */
	iowrite32(NSS_CRYPTO_EIP197_FHASH_IV0_CFG, base_addr + NSS_CRYPTO_EIP197_FHASH_IV0);
	iowrite32(NSS_CRYPTO_EIP197_FHASH_IV1_CFG, base_addr + NSS_CRYPTO_EIP197_FHASH_IV1);
	iowrite32(NSS_CRYPTO_EIP197_FHASH_IV2_CFG, base_addr + NSS_CRYPTO_EIP197_FHASH_IV2);
	iowrite32(NSS_CRYPTO_EIP197_FHASH_IV3_CFG, base_addr + NSS_CRYPTO_EIP197_FHASH_IV3);

	/*
	 * Configure FLUE_CONFIG register
	 */
	iowrite32(NSS_CRYPTO_EIP197_ENB_FLUE, base_addr + NSS_CRYPTO_EIP197_FLUE_CONFIG(0));
	iowrite32(NSS_CRYPTO_EIP197_FLUE_OFFSET_CFG, base_addr + NSS_CRYPTO_EIP197_FLUE_OFFSET);

	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_FLUE_IFC_LUT0);
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_FLUE_IFC_LUT1);
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_FLUE_IFC_LUT2);
 }

/*
 * nss_crypto_eip197_hw_setup_fw()
 *	setup EIP197 firmware resets and initializes
 */
static void nss_crypto_eip197_hw_setup_fw(void __iomem *base_addr)
{
	int i;

	/*
	 * Disable ICE SCRATCHRAM PAD statistics update
	 */
	iowrite32(0x1, base_addr + NSS_CRYPTO_EIP197_ICE_DISABLE_STATS);

	/*
	 * Init ICE scratch control register
	 * Reset ICE scratch RAM area
	 */
	iowrite32(NSS_CRYPTO_EIP197_ENB_SCRATCH_RAM, base_addr + NSS_CRYPTO_EIP197_PE_ICE_SCRATCH_CTRL);
	for (i = 0; i < NSS_CRYPTO_EIP197_MAX_SCRATCH_RAM; i++)
		iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_PE_ICE_SCRATCH_RAM + (i * 0x4));


	/*
	 * Init OCE scratch control register
	 * Reset OCE scratch RAM area
	 */
	iowrite32(NSS_CRYPTO_EIP197_ENB_SCRATCH_RAM, base_addr + NSS_CRYPTO_EIP197_PE_OCE_SCRATCH_CTRL);
	for (i = 0; i < NSS_CRYPTO_EIP197_MAX_SCRATCH_RAM; i++)
		iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_PE_OCE_SCRATCH_RAM + (i * 0x4));

}

/*
 * nss_crypto_eip197_hw_boot_ofpp()
 *	load output flow post processor (OFPP) firmware
 */
static void nss_crypto_eip197_hw_boot_ofpp(struct device *dev, void __iomem *base_addr, const char *fw_name)
{
	const struct firmware *fw = NULL;
	uint32_t *data;
	int i;

	/*
	 * pulling engine out of reset
	 */
	iowrite32(NSS_CRYPTO_EIP197_RST_PE, base_addr + NSS_CRYPTO_EIP197_PE_OCE_FPP_CTRL);
	iowrite32(NSS_CRYPTO_EIP197_ENB_OFPP, base_addr + NSS_CRYPTO_EIP197_PE_OCE_RAM_CTRL);

	/*
	 * load the firmware to the engine
	 */
	if (request_firmware(&fw, fw_name, dev)) {
		nss_crypto_warn("%p: FW(%s) load failed\n", dev, fw_name);
		return;
	}

	for (i = 0, data = (uint32_t *)fw->data; i < fw->size; i += 4, data++)
		iowrite32(*data, base_addr + NSS_CRYPTO_EIP197_CS_RAM_START + i);

	/*
	 * put OCE RAM into reset so that the next engine can load
	 */
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_PE_OCE_RAM_CTRL);
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_PE_OCE_FPP_CTRL);

	release_firmware(fw);

#if defined(NSS_CRYPTO_DEBUG)
	/*
	 * put a delay of 100ms before accessing the FW specific registers
	 */
	mdelay(100);

	/*
	 * put engine into debug mode for reading the FW version
	 */
	iowrite32(NSS_CRYPTO_EIP197_ENB_OFPP_DBG, base_addr + NSS_CRYPTO_EIP197_PE_OCE_FPP_CTRL);

	nss_crypto_info("%p: %s version(0x%x)\n", dev, fw_name,
			ioread32(base_addr + NSS_CRYPTO_EIP197_OCE_OFPP_VERSION_REG));
#endif
}

/*
 * nss_crypto_eip197_hw_boot_opue()
 *	load output pull-up engine (OPUE) firmware
 */
static void nss_crypto_eip197_hw_boot_opue(struct device *dev, void __iomem *base_addr, const char *fw_name)
{
	const struct firmware *fw = NULL;
	uint32_t *data;
	int i;

	/*
	 * pulling engine out of reset
	 */
	iowrite32(NSS_CRYPTO_EIP197_RST_PE, base_addr + NSS_CRYPTO_EIP197_PE_OCE_PUE_CTRL);
	iowrite32(NSS_CRYPTO_EIP197_ENB_OPUE, base_addr + NSS_CRYPTO_EIP197_PE_OCE_RAM_CTRL);

	/*
	 * load the firmware to the engine
	 */
	if (request_firmware(&fw, fw_name, dev)) {
		nss_crypto_warn("%p: FW(%s) load failed\n", dev, fw_name);
		return;
	}

	for (i = 0, data = (uint32_t *)fw->data; i < fw->size; i += 4, data++)
		iowrite32(*data, base_addr + NSS_CRYPTO_EIP197_CS_RAM_START + i);

	/*
	 * put OCE RAM into reset
	 */
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_PE_OCE_RAM_CTRL);
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_PE_OCE_PUE_CTRL);

	release_firmware(fw);

#if defined(NSS_CRYPTO_DEBUG)
	/*
	 * put a delay of 100ms before accessing the FW specific registers
	 */
	mdelay(100);

	/*
	 * put engine into debug mode for reading the FW version
	 */
	iowrite32(NSS_CRYPTO_EIP197_ENB_OPUE_DBG, base_addr + NSS_CRYPTO_EIP197_PE_OCE_PUE_CTRL);

	nss_crypto_info("%p: %s version(0x%x)\n", dev, fw_name,
			ioread32(base_addr + NSS_CRYPTO_EIP197_OCE_OPUE_VERSION_REG));
#endif
}

/*
 * nss_crypto_eip197_hw_boot_ifpp()
 *	load input flow post processor (IFPP) firmware
 */
static void nss_crypto_eip197_hw_boot_ifpp(struct device *dev, void __iomem *base_addr, const char *fw_name)
{
	const struct firmware *fw = NULL;
	uint32_t *data;
	int i;

	/*
	 * pulling engine out of reset
	 */
	iowrite32(NSS_CRYPTO_EIP197_RST_PE, base_addr + NSS_CRYPTO_EIP197_PE_ICE_FPP_CTRL);
	iowrite32(NSS_CRYPTO_EIP197_ENB_IFPP, base_addr + NSS_CRYPTO_EIP197_PE_ICE_RAM_CTRL);

	/*
	 * load the firmware to the engine
	 */
	if (request_firmware(&fw, fw_name, dev)) {
		nss_crypto_warn("%p: FW(%s) load failed\n", dev, fw_name);
		return;
	}

	for (i = 0, data = (uint32_t *)fw->data; i < fw->size; i += 4, data++)
		iowrite32(*data, base_addr + NSS_CRYPTO_EIP197_CS_RAM_START + i);

	/*
	 * put ICE (input classification engine) RAM into reset
	 */
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_PE_ICE_RAM_CTRL);
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_PE_ICE_FPP_CTRL);

	release_firmware(fw);

#if defined(NSS_CRYPTO_DEBUG)
	/*
	 * put a delay of 100ms before accessing the FW specific registers
	 */
	mdelay(100);

	/*
	 * put engine into debug mode for reading the FW version
	 */
	iowrite32(NSS_CRYPTO_EIP197_ENB_IFPP_DBG, base_addr + NSS_CRYPTO_EIP197_PE_ICE_FPP_CTRL);

	nss_crypto_info("%p: %s version(0x%x)\n", dev, fw_name,
			ioread32(base_addr + NSS_CRYPTO_EIP197_ICE_IFPP_VERSION_REG));
#endif
}

/*
 * nss_crypto_eip197_hw_boot_ipue()
 *	load input pull-up engine (IPUE) firmware
 */
static void nss_crypto_eip197_hw_boot_ipue(struct device *dev, void __iomem *base_addr, const char *fw_name)
{
	const struct firmware *fw = NULL;
	uint32_t *data;
	int i;

	/*
	 * pulling engine out of reset
	 */
	iowrite32(NSS_CRYPTO_EIP197_RST_PE, base_addr + NSS_CRYPTO_EIP197_PE_ICE_PUE_CTRL);
	iowrite32(NSS_CRYPTO_EIP197_ENB_IPUE, base_addr + NSS_CRYPTO_EIP197_PE_ICE_RAM_CTRL);

	/*
	 * load the firmware to the engine
	 */
	if (request_firmware(&fw, fw_name, dev)) {
		nss_crypto_warn("%p: FW(%s) load failed\n", dev, fw_name);
		return;
	}

	for (i = 0, data = (uint32_t *)fw->data; i < fw->size; i += 4, data++)
		iowrite32(*data, base_addr + NSS_CRYPTO_EIP197_CS_RAM_START + i);

	/*
	 * put ICE (input classification engine) RAM into reset
	 */
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_PE_ICE_RAM_CTRL);
	iowrite32(0x0, base_addr + NSS_CRYPTO_EIP197_PE_ICE_PUE_CTRL);

	release_firmware(fw);

#if defined(NSS_CRYPTO_DEBUG)
	/*
	 * put a delay of 100ms before accessing the FW specific registers
	 */
	mdelay(100);

	/*
	 * put engine into debug mode for reading the FW version
	 */
	iowrite32(NSS_CRYPTO_EIP197_ENB_IPUE_DBG, base_addr + NSS_CRYPTO_EIP197_PE_ICE_PUE_CTRL);

	nss_crypto_info("%p: %s version(0x%x)\n", dev, fw_name,
		ioread32(base_addr + NSS_CRYPTO_EIP197_ICE_IPUE_VERSION_REG));
#endif
}

/*
 * nss_crypto_eip197_hw_setup()
 *	Pre initialization function for eip197
 */
void nss_crypto_eip197_hw_setup(void __iomem *base_addr)
{
	/*
	 * Reset EIP blocks and check if reset is complete
	 */
	if (!nss_crypto_eip197_hw_reset(base_addr)) {
		/*
		 * If reset is not complete yet, wait for reset to
		 * complete before doing any other initialization
		 */
		while (!nss_crypto_eip197_hw_reset_status(base_addr))
			mdelay(100);
	}

	/*
	 * Pre-initialization of EIP block, the following is performed
	 * - Data Fetch Engine (DFE) init
	 * - Data Store Engine (DSE) init
	 * - Ring Arbiter (HIA_RA) init
	 * - DRBG init
	 * - Flow record cache init
	 * - Transform record cache init
	 * - ICE/OCE firmware init
	 */
	nss_crypto_eip197_hw_setup_core(base_addr);
	nss_crypto_eip197_hw_setup_cache(base_addr);
	nss_crypto_eip197_hw_setup_fw(base_addr);
}

/*
 * nss_crypto_eip197_hw_init()
 *	initialize the EIP197 HW
 */
void nss_crypto_eip197_hw_init(struct platform_device *pdev, struct device_node *np, void __iomem *base_addr)
{
	nss_crypto_eip197_hw_setup(base_addr);

	/*
	 * check and load engine firmware; if this engine is enabled
	 * with these additonal classifiers
	 */
	if (of_property_read_bool(np, "qcom,ifpp-enabled"))
		nss_crypto_eip197_hw_boot_ifpp(&pdev->dev, base_addr, "ifpp.bin");

	if (of_property_read_bool(np, "qcom,ipue-enabled"))
		nss_crypto_eip197_hw_boot_ipue(&pdev->dev, base_addr, "ipue.bin");

	if (of_property_read_bool(np, "qcom,ofpp-enabled"))
		nss_crypto_eip197_hw_boot_ofpp(&pdev->dev, base_addr, "ofpp.bin");

	if (of_property_read_bool(np, "qcom,opue-enabled"))
		nss_crypto_eip197_hw_boot_opue(&pdev->dev, base_addr, "opue.bin");

	/*
	 * Allow passing 16 bytes of metadata information from
	 * input to output.
	 */
	iowrite32(NSS_CRYPTO_EIP197_ENB_MDATA, base_addr + NSS_CRYPTO_EIP197_METADATA_EN_OFFSET);
}
