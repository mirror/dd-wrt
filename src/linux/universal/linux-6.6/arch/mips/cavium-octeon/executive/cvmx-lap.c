/***********************license start***************
 * Copyright (c) 2014  Cavium Inc. (support@cavium.com). All rights
 * reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.

 *   * Neither the name of Cavium Inc. nor the names of
 *     its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written
 *     permission.

 * This Software, including technical data, may be subject to U.S. export  control
 * laws, including the U.S. Export Administration Act and its  associated
 * regulations, and may be subject to export or import  regulations in other
 * countries.

 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND CAVIUM INC. MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY REPRESENTATION OR
 * DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT DEFECTS, AND CAVIUM
 * SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES OF TITLE,
 * MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF
 * VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. THE ENTIRE  RISK ARISING OUT OF USE OR
 * PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
 ***********************license end**************************************/

/**
 * @file
 *
 * Support library for the LAP
 *
 */

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/irqflags.h>
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-pko3.h>
#include <asm/octeon/cvmx-lapx-defs.h>
#include <asm/octeon/cvmx-osm-defs.h>
#include <asm/octeon/cvmx-lap.h>
#else
#include <stdio.h>
#include <string.h>
#include "cvmx.h"
#include "cvmx-spinlock.h"
#include "cvmx-pko3.h"
#include "cvmx-lapx-defs.h"
#include "cvmx-lap.h"
#endif /*CVMX_BUILD_FOR_LINUX_KERNEL*/


#define LAP_BIST_RESULT_MASK 0x1cfull
#define NUM_EQ_BUFS 2
static CVMX_SHARED int max_labs[CVMX_LAP_MAX_LAPS];

static CVMX_SHARED int32_t gbl_labs_in_use_cnt[CVMX_LAP_MAX_LAPS];

/**
 * soft reset
 *
 * @param lap_num - lap number to be reset (0 or 1)
 * @return 0 on success non zero on failure
 */
int cvmx_lap_soft_reset(int lap_num)
{
	cvmx_lapx_sft_rst_t soft_rst;

	if ((lap_num != 0) && (lap_num != 1)) {
		cvmx_dprintf("Invalid lap num %d\n", lap_num);
		return -1;
	}
	soft_rst.u64 = cvmx_read_csr(CVMX_LAPX_SFT_RST(lap_num));
	soft_rst.s.reset = 1;
	cvmx_write_csr(CVMX_LAPX_SFT_RST(lap_num), soft_rst.u64);
	do {
		soft_rst.u64 = cvmx_read_csr(CVMX_LAPX_SFT_RST(lap_num));
		//TODO timeout
	} while(soft_rst.s.busy);
	return 0;
}
EXPORT_SYMBOL(cvmx_lap_soft_reset);

/**
 * Software lab management
 *
 * @param lap_num - lap number (0 or 1)
 * @return 1 if lab is allocated 0 otherwise
 */
int cvmx_lap_mgr_get_lab(int lap_num)
{
	int32_t val;

	lap_num &= 1;
	do {
		val = cvmx_atomic_get32(&gbl_labs_in_use_cnt[lap_num]);
		if (val >= max_labs[lap_num]) {
			return 0;
		}
	} while (!cvmx_atomic_compare_and_store32((uint32_t *)&gbl_labs_in_use_cnt[lap_num],(uint32_t)val,(uint32_t)(val+1)));
	return 1;
}
EXPORT_SYMBOL(cvmx_lap_mgr_get_lab);

/**
 * Software lab management
 *
 * @param lap_num - lap number (0 or 1) 
 */
void cvmx_lap_mgr_put_lab(int lap_num)
{
	lap_num &= 1;
	cvmx_atomic_add32(&gbl_labs_in_use_cnt[lap_num], -1); 
}
EXPORT_SYMBOL(cvmx_lap_mgr_put_lab);

/**
 * Fill exeception queue
 *
 * @param lap_num - lap number (0 or 1)
 * @param num_bufs - number of buffers to be queued to exeception queue
 * @return number of labs queued
 */
int cvmx_lap_fill_eq(int lap_num, int num_bufs)
{
	cvmx_lap_send_lmtdma_t send_dma;
	uint64_t dma_addr;
	cvmx_lap_ctl_rtn_t rtn_ctl;
	int done, itr;
	int num_labs_qued = 0;
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
	unsigned long flags;
#endif
#define DUMMY_CMD_LEN (5)
	uint64_t dummy_cmd[DUMMY_CMD_LEN] = {0x100008000000000llu,
					     0x680000000llu,
					     0x0llu,
					     0x0llu,
					     0x0llu};
	
	while (num_labs_qued < num_bufs) {
		unsigned scr_off = CVMX_PKO_LMTLINE * CVMX_CACHE_LINE_SIZE;
		/* LMTDMA globally ordered */
		dma_addr = CVMX_LMTDMA_ORDERED_IO_ADDR;
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
		/* dont want to be preempted in the middle of LMTDMA */
		local_irq_save(flags);
#endif
		for (itr = 0; itr < DUMMY_CMD_LEN; itr++) {
			cvmx_scratch_write64(scr_off, dummy_cmd[itr]);
			scr_off += 8;
		}
		dma_addr += ((DUMMY_CMD_LEN - 1) << 3);
		cvmx_scratch_write64(scr_off, 0); 

		/* Check for LABs in software as a workaround for the
		   Errata (LAP-20136). */
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) &&
		   (!cvmx_lap_mgr_get_lab(lap_num))) {
			cvmx_dprintf("software lab manager: no labs\n");
			break;
		}
		send_dma.u64 = 0;
		send_dma.s.scraddr = scr_off >> 3;
		send_dma.s.rtnlen = 5; //we only want the first word 
		send_dma.s.did = CVMX_LAPX_DID(lap_num);
		send_dma.s.node = cvmx_get_node_num();
		send_dma.s.cmd = CVMX_LAP_SEND;
		send_dma.s.df = 1;
		send_dma.s.ds = 1;
		send_dma.s.eq = 1;
		CVMX_SYNCW;
		cvmx_write64_uint64(dma_addr, send_dma.u64);
		CVMX_SYNCIOBDMA;
		done = 0;
		do {
			rtn_ctl.u64 = cvmx_scratch_read64(scr_off);
			/* Apply the workaround for Errata (LAP-20342) */
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) &&
				rtn_ctl.s.err == CVMX_LAP_LAB_OUT) { 
				//cvmx_dprintf("no labs to queue to eq\n");
				done = 1;
			} else if  (rtn_ctl.s.timeout) {
				//cvmx_dprintf("lab queuing to eq timeout\n");
				num_labs_qued++;
				done = 1;
			} else if (rtn_ctl.u64) {
				num_labs_qued++;
				done = 1;
			}
		} while(!done);
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL	
		local_irq_restore(flags);
#endif
		/* Manage LABs in software as a workaround for the
		   Errata (LAP-20136). */
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			cvmx_lap_mgr_put_lab(lap_num); 
	}
	return num_labs_qued;
}
EXPORT_SYMBOL(cvmx_lap_fill_eq);


/**
 * initialize LAP with given config
 *
 * @param lap_num - lap number (0 or 1)
 * @param lap_config - lap configuration 
 * @return 0 on success nonzero on failure
 */
int cvmx_lap_init(int lap_num, cvmx_lap_config_t *lap_config)
{
	cvmx_lapx_bist_result_t bist_result;
	cvmx_lapx_cfg_t lap_cfg;
	cvmx_lapx_xid_pos_t xid_pos;
	cvmx_lapx_quex_cfg_t que_cfg;
	cvmx_lapx_timeout_t timeout;

	int itr;
	lap_num &= 1;

	if ((lap_num != 0) && (lap_num != 1)) {
		cvmx_dprintf("Invalid lap num %d\n", lap_num);
		return -1;
	}

	/* bist */
	bist_result.u64 = cvmx_read_csr(CVMX_LAPX_BIST_RESULT(lap_num));
	if (bist_result.u64 &  LAP_BIST_RESULT_MASK) {
		cvmx_dprintf("Bist failed\n");
		if (bist_result.s.nbr)
			cvmx_dprintf("NBR bist failed\n");
		if (bist_result.s.edat)
			cvmx_dprintf("EDAT bist failed\n");
		if (bist_result.s.emsk)
			cvmx_dprintf("EMSK bist failed\n");
		if (bist_result.s.lab_dat)
			cvmx_dprintf("LAB_DAT bist failed\n");
		if (bist_result.s.ctl_nxt)
			cvmx_dprintf("CTL_NXT bist failed\n");
		if (bist_result.s.ctl_sta)
			cvmx_dprintf("CTL_STA bist failed\n");
	} 
	
	/* configure lap */
	xid_pos.u64 = cvmx_read_csr(CVMX_LAPX_XID_POS(lap_num));
	xid_pos.s.as_only = lap_config->xid_pos.s.as_only;
	xid_pos.s.rtn_wd  = lap_config->xid_pos.s.rtn_wd;
	xid_pos.s.rtn_lsb = lap_config->xid_pos.s.rtn_lsb; 
	xid_pos.s.req_wd  = lap_config->xid_pos.s.req_wd;
	xid_pos.s.req_lsb = lap_config->xid_pos.s.req_lsb; 
	cvmx_write_csr(CVMX_LAPX_XID_POS(lap_num), xid_pos.u64);
	
	for (itr =0; itr < CVMX_LAP_QUEUES_PER_LAP; itr++) {
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)) {
			/* Apply workaround for Errata (LAP-20300) */
			que_cfg.u64 = cvmx_read_csr(CVMX_LAPX_QUEX_CFG(lap_num, itr));
			que_cfg.s.max_labs = lap_config->max_labs[CVMX_LAP_QUEUES_PER_LAP-1-itr];
			cvmx_write_csr(CVMX_LAPX_QUEX_CFG(lap_num, itr),que_cfg.u64); 
		} else {
			que_cfg.u64 = cvmx_read_csr(CVMX_LAPX_QUEX_CFG(lap_num, itr));
			que_cfg.s.max_labs = lap_config->max_labs[itr];
			cvmx_write_csr(CVMX_LAPX_QUEX_CFG(lap_num, itr),que_cfg.u64); 
		}
	}
	
	/* Apply the workaround for Errata (LAP-20620). */
	if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)) {
		timeout.u64 = cvmx_read_csr(CVMX_LAPX_TIMEOUT(lap_num));
		timeout.s.iobdma = 0xff;
		timeout.s.resp = 0x201;
		cvmx_write_csr(CVMX_LAPX_TIMEOUT(lap_num), timeout.u64);
	}
	switch (lap_config->lab_size) {
	case 0x0:
		max_labs[lap_num] = 128;
		break;
	case 0x1:
		max_labs[lap_num] = 170;
		break;
	case 0x2:
		max_labs[lap_num] = 256;
		break;
	default:
		max_labs[lap_num] = 256;
		lap_config->lab_size = 0x2;
		break;
	}
 
	/* Return if LAP already initialized */
	lap_cfg.u64 = cvmx_read_csr(CVMX_LAPX_CFG(lap_num));
	if (lap_cfg.s.ena) {
		cvmx_dprintf("LAP%d is already enabled\n", lap_num);
		return 0;
	}
	/* enable */
	lap_cfg.s.ooo = lap_config->ooo;
	lap_cfg.s.lab_size =  lap_config->lab_size;
	lap_cfg.s.ena = 1;
	cvmx_write_csr(CVMX_LAPX_CFG(lap_num), lap_cfg.u64);

	for (itr =0; itr < CVMX_LAP_MAX_EXCEPTION_REGS; itr++) {
		if (lap_config->exp_valid[itr]) {
			cvmx_write_csr(CVMX_LAPX_EXPX_DATA(itr, lap_num), lap_config->exp_data[itr]);
			cvmx_write_csr(CVMX_LAPX_EXPX_VALID(itr, lap_num), lap_config->exp_valid[itr]);
		}
	}
   
	/* setup exception queue */
	if (cvmx_lap_fill_eq(lap_num, NUM_EQ_BUFS) != (NUM_EQ_BUFS)) {
		cvmx_dprintf("unable to fill Exception queue\n");
		return -1;
	}
	return 0;
}
EXPORT_SYMBOL(cvmx_lap_init);

/**
 * LMTDMA send operation. This will send request and receive the response data
 *
 * @param lap_num  - lap number (0 or 1)
 * @param channel  - Channel number
 * @param req_ptr  - Request buffer pointer
 * @param req_len  - Request length (in bytes)
 * @param rsp_ptr  - Response buffer pointer
 * @param rsp_size - Response buffer size 
 *                   (should be more than expected response size)
 *
 * @return size of Response (>=0) on success else negative (-1) for error
 */
int cvmx_lap_send_lmtdma(int lap_num, int channel, void * req_ptr, int req_len, 
			void * rsp_ptr, int rsp_size)
{
	const unsigned scr_base = CVMX_PKO_LMTLINE * CVMX_CACHE_LINE_SIZE;
	cvmx_lap_send_lmtdma_t send_dma;
	cvmx_lap_rd_iobdma_t iobdma;
	cvmx_lap_ctl_rtn_t rtn_ctl;
	uint64_t dma_addr;
	int done, lab, itr, num_words;
	unsigned scr_off;
	uint64_t rtn_addr;
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
	unsigned long kflags = 0;
#endif
	if ((!req_ptr) || (!rsp_ptr) || 
	    (!req_len) || (rsp_size < 8)) {
		cvmx_dprintf("Invalid request or response\n");
		return -1;
	}
		
	num_words = (req_len >> 3) + ((req_len & 0x7)?1:0);
	/* LMTDMA globally ordered */
	dma_addr = CVMX_LMTDMA_ORDERED_IO_ADDR;
	dma_addr += (num_words - 1) << 3;

	/* Manage LAB entries in software based on the Errata (LAP-20136) */
	if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) &&
		(!cvmx_lap_mgr_get_lab(lap_num))) {
		cvmx_dprintf("software lab manager: no labs\n");
		return -1;
	}
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
	local_irq_save(kflags);
#endif
	for (itr = 0, scr_off = scr_base; itr < num_words; itr++) { 
		cvmx_scratch_write64(scr_off, ((uint64_t *) req_ptr)[itr]);
		scr_off += 8;
	}
	rtn_addr = scr_off;
	cvmx_scratch_write64(rtn_addr, 0); 
	send_dma.u64 = 0;
	send_dma.s.scraddr = rtn_addr >> 3;
	send_dma.s.rtnlen = rsp_size>>3; 
	send_dma.s.did = CVMX_LAPX_DID(lap_num);
	send_dma.s.node = cvmx_get_node_num();
	send_dma.s.chan = channel;
	send_dma.s.cmd = CVMX_LAP_SEND;
	done = 0;

	CVMX_SYNCW;
	cvmx_write64_uint64(dma_addr, send_dma.u64);
	CVMX_SYNCIOBDMA;

	do {
		rtn_ctl.u64 = cvmx_scratch_read64(rtn_addr);
		/* Apply the workaround for Errata (LAP-20342) */
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) &&
			rtn_ctl.s.err == CVMX_LAP_LAB_OUT) { 
			done = 1;
		} else if (rtn_ctl.s.timeout) {
				lab = rtn_ctl.s.lab;
				/* cvmx_dprintf("lab = 0x%x\n", lab); */
				/* IOBDMA globally ordered */
				dma_addr = CVMX_IOBDMA_ORDERED_IO_ADDR;
				cvmx_scratch_write64(rtn_addr, 0); 
				iobdma.u64 = 0;
				iobdma.s.scraddr = rtn_addr >> 3;
				iobdma.s.rtnlen = 8;
				iobdma.s.did = CVMX_LAPX_DID(lap_num);
				iobdma.s.node = cvmx_get_node_num();
				iobdma.s.cmd = CVMX_LAP_RTN;
				iobdma.s.lab = lab;
				CVMX_SYNCW;
				cvmx_write64_uint64(dma_addr, iobdma.u64); 
				CVMX_SYNCIOBDMA;
		} else if (rtn_ctl.u64) {
			done = 1;
		}
	} while(!done);

	if (rtn_ctl.s.err) {
		/* Manage LAB entries in software based on the Errata (LAP-20136) */
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			cvmx_lap_mgr_put_lab(lap_num);
		cvmx_dprintf("lmtdma error %d\n", rtn_ctl.s.err);
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
		local_irq_restore(kflags);
#endif
		return -1;
	}
	
	if (rsp_size < (rtn_ctl.s.length*8)) {
		/* Manage LAB entries in software based on the Errata (LAP-20136) */
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			cvmx_lap_mgr_put_lab(lap_num);
		cvmx_dprintf("insufficient rsp buffer size %d return size %d\n",
			      rsp_size, rtn_ctl.s.length * 8);
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
		local_irq_restore(kflags);
#endif
		return -1;
	}

	for (itr = 0; itr < rtn_ctl.s.length; itr++) {
		((uint64_t *) rsp_ptr)[itr] = cvmx_scratch_read64(rtn_addr + itr*8);
	}

	/* Manage LAB entries in software based on the Errata (LAP-20136) */
	if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
		cvmx_lap_mgr_put_lab(lap_num);

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
	local_irq_restore(kflags);
#endif
	return rtn_ctl.s.length;
}

EXPORT_SYMBOL(cvmx_lap_send_lmtdma);

/**
 * LMTDMA send operation with immediate time out.
 * This will send request and get LAB (transaction ID) as response
 *
 * @param lap_num  - lap number (0 or 1)
 * @param channel  - Channel number
 * @param req_ptr  - Request buffer pointer
 * @param req_len  - Request length (in bytes)
 *
 * @return LAB (transaction ID >=0) in case of success 
 * 		else negative (-1) for error
 */
int cvmx_lap_send_lmtdma_ito(int lap_num, int channel, 
				void * req_ptr, int req_len)
{
	const unsigned scr_base = CVMX_PKO_LMTLINE * CVMX_CACHE_LINE_SIZE;
	cvmx_lap_send_lmtdma_t send_dma;
	cvmx_lap_ctl_rtn_t rtn_ctl;
	uint64_t dma_addr;
	int done, lab, itr, num_words;
	unsigned scr_off;
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
	unsigned long kflags = 0;
#endif
	if ((!req_ptr) || (!req_len)) {
		cvmx_dprintf("Invalid request\n");
		return -1;
	}
		
	num_words = (req_len >> 3) + ((req_len & 0x7)?1:0);
	/* LMTDMA globally ordered */
	dma_addr = CVMX_LMTDMA_ORDERED_IO_ADDR;
	dma_addr += (num_words - 1) << 3;

	/* Manage LAB entries in software based on the Errata (LAP-20136) */
	if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) &&
		(!cvmx_lap_mgr_get_lab(lap_num))) {
		cvmx_dprintf("software lab manager: no labs\n");
		return -1;
	}
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
	local_irq_save(kflags);
#endif
	for (itr = 0, scr_off = scr_base; itr < num_words; itr++) { 
		cvmx_scratch_write64(scr_off, ((uint64_t *) req_ptr)[itr]);
		scr_off += 8;
	}
	
	cvmx_scratch_write64(scr_off, 0); 
	send_dma.u64 = 0;
	send_dma.s.scraddr = scr_off >> 3;
	send_dma.s.rtnlen = 1;
	send_dma.s.did = CVMX_LAPX_DID(lap_num);
	send_dma.s.node = cvmx_get_node_num();
	send_dma.s.chan = channel;
	send_dma.s.cmd = CVMX_LAP_SEND;
	send_dma.s.ito = 1;
	send_dma.s.df = 1;
	done = 0;
	lab = -1;

	CVMX_SYNCW;
	cvmx_write64_uint64(dma_addr, send_dma.u64);
	CVMX_SYNCIOBDMA;

	do {
		rtn_ctl.u64 = cvmx_scratch_read64(scr_off);
		//cvmx_dprintf("rtn_ctl.u64 = 0x%llx\n", (unsigned long long) rtn_ctl.u64);
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) &&
			rtn_ctl.s.err == CVMX_LAP_LAB_OUT) { 
			done = 1;
		} else if (rtn_ctl.s.timeout) {
			lab = rtn_ctl.s.lab;
			done = 1;
		} else if (rtn_ctl.u64) {
			done = 1;
		}
	} while(!done);
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
	local_irq_restore(kflags);
#endif
	return lab;
}

EXPORT_SYMBOL(cvmx_lap_send_lmtdma_ito);

/**
 * IOBDMA read operation by providing LAB identifier
 * This will send IOBDMA read request with LAB (transaction ID)
 * and receive response data for the request that was sent earlier.
 *
 * @param lap_num  - lap number (0 or 1)
 * @param lab_id   - LAB (transaction ID >= 0) 
 * 			(which is received by cvmx_lap_send_lmtdma_ito() call)
 * @param offset   - Beginning word number from IOBDMA response buffer 
 * 			(default value 0)
 * @param rsp_ptr  - Response buffer pointer
 * @param rsp_size - Response buffer size 
 * 			(should be more than expected response size)
 *
 * @return size of Response (>=0) on success 
 * 	else negative (-1) for error
 */
int cvmx_lap_read_iobdma(int lap_num, int lab_id,  int offset, 
			void * rsp_ptr, int rsp_size)
{
	cvmx_lap_rd_iobdma_t iobdma;
	cvmx_lap_ctl_rtn_t rtn_ctl;
	uint64_t dma_addr;
	int itr = 0;
	const unsigned scr_base = CVMX_PKO_LMTLINE * CVMX_CACHE_LINE_SIZE;
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
	unsigned long kflags = 0;
#endif
	 
	if ((lab_id < 0) || (!rsp_ptr) || (rsp_size < 8))
	{
		cvmx_dprintf("Invalid request or response\n");
		return -1;
	}

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
	local_irq_save(kflags);
#endif
	/* IOBDMA globally ordered */
	dma_addr = CVMX_IOBDMA_ORDERED_IO_ADDR;
	cvmx_scratch_write64(scr_base, 0); 
	iobdma.u64 = 0;
	rtn_ctl.u64 = 0;
	iobdma.s.scraddr = (scr_base >> 3);
	iobdma.s.rtnlen = rsp_size >> 3;
	iobdma.s.did = CVMX_LAPX_DID(lap_num);
	iobdma.s.node = cvmx_get_node_num();
	iobdma.s.offset = offset;
	iobdma.s.cmd = CVMX_LAP_RTN;
	iobdma.s.lab = lab_id;
	CVMX_SYNCW;
	cvmx_write64_uint64(dma_addr, iobdma.u64);
	CVMX_SYNCIOBDMA; 
	rtn_ctl.u64 = cvmx_scratch_read64(scr_base);

	if (rsp_size < (rtn_ctl.s.length*8)) {
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			cvmx_lap_mgr_put_lab(lap_num);
 
		cvmx_dprintf("insufficient rsp buffer size %d return size %d\n",
			      rsp_size, rtn_ctl.s.length * 8);
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
		local_irq_restore(kflags);
#endif
		return -1;
	}

	for (itr = 0; itr < rtn_ctl.s.length; itr++) {
		((uint64_t *) rsp_ptr)[itr] = cvmx_scratch_read64(scr_base + itr*8);
	}
	if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
		cvmx_lap_mgr_put_lab(lap_num);
 
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
	local_irq_restore(kflags);
#endif
	return rtn_ctl.s.length;
}
EXPORT_SYMBOL(cvmx_lap_read_iobdma);
