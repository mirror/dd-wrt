/***********************license start***************
 * Copyright (c) 2003-2014  Cavium Inc. (support@cavium.com). All rights
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
 * Interface to the On Chip Logic Analyzer (OCLA) hardware.
 *
 */

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-ocla.h>
#include <asm/octeon/cvmx-dtx-defs.h>
#include <asm/octeon/cvmx-l2c-defs.h>
#include <asm/octeon/cvmx-oclax-defs.h>
#else
#include "cvmx.h"
#include "cvmx-ocla.h"
#include <cvmx-dtx-defs.h>
#include <cvmx-l2c-defs.h>
#include <cvmx-oclax-defs.h>
#endif


/* Types of dtx registers */
enum {
	DTX_SEL,
	DTX_ENA,
	DTX_DAT,
	DTX_CTL
};

/* Information needed to derive a DTX block register address */
struct dtx_reg_addr_rule {
	cvmx_dtx_id_t	id;
	uint64_t	base;
	int		inst_factor;
	int		half_factor;
};


/* DTX registers on the 70xx */
static struct dtx_reg_addr_rule dtx_regs_70xx[] = {
	{AGL,		0x00011800FE700000ull,	0,		8},
	{DFA,		0x00011800FE1B8000ull,	0,		8},
	{DPI,		0x00011800FEEF8000ull,	0,		8},
	{FPA,		0x00011800FE140000ull,	0,		8},
	{GMX,		0x00011800FE040000ull,	0x40000,	8},
	{IOB,		0x00011800FE780000ull,	0,		8},
	{IPD,		0x00011800FE278000ull,	0,		8},
	{L2C_CBC,	0x00011800FE420000ull,	0,		8},
	{L2C_MCI,	0x00011800FE2E0000ull,	0,		8},
	{L2C_TAD,	0x00011800FE240000ull,	0,		8},
	{LMC,		0x00011800FE440000ull,	0,		8},
	{MIO,		0x00011800FE000000ull,	0,		8},
	{PCS,		0x00011800FE580000ull,	0x40000,	8},
	{PEM,		0x00011800FE600000ull,	0x8000,		8},
	{PIP,		0x00011800FE500000ull,	0,		8},
	{PKO,		0x00011800FE280000ull,	0,		8},
	{POW,		0x00011800FE338000ull,	0,		8},
	{RST,		0x00011800FE030000ull,	0,		8},
	{SATA,		0x00011800FE360000ull,	0,		8},
	{SLI,		0x00011800FE8F8000ull,	0,		8},
	{TIM,		0x00011800FE2C0000ull,	0,		8},
	{USBDRD,	0x00011800FE340000ull,	0x8000,		8},
	{INVALID_BLOCK_ID, 0,			0,		0}
};

/* DTX registers on the 78xx */
static struct dtx_reg_addr_rule dtx_regs_78xx[] = {
	{ASE,		0x00011800FE6E8000ull,	0, 		8},
	{BGX,		0x00011800FE700000ull,	0x8000,		8},
	{CIU,		0x00011800FE808000ull,	0,		8},
	{DFA,		0x00011800FE1B8000ull,	0,		8},
	{DPI,		0x00011800FEEF8000ull,	0,		8},
	{FPA,		0x00011800FE940000ull,	0,		8},
	{GSER,		0x00011800FE480000ull,	0x8000,		8},
	{HNA,		0x00011800FE238000ull,	0,		8},
	{ILA,		0x00011800FE0B8000ull,	0,		8},
	{ILK,		0x00011800FE0A0000ull,	0,		8},
	{IOBN,		0x00011800FE780000ull,	0,		8},
	{IOBP,		0x00011800FE7A0000ull,	0,		8},
	{L2C_CBC,	0x00011800FE420000ull,	0x8000,		8},
	{L2C_MCI,	0x00011800FE2E0000ull,	0x8000,		8},
	{L2C_TAD,	0x00011800FE240000ull,	0x8000,		8},
	{LAP,		0x00011800FE060000ull,	0x8000,		8},
	{LBK,		0x00011800FE090000ull,	0,		8},
	{LMC,		0x00011800FE440000ull,	0x8000,		8},
	{MIO,		0x00011800FE000000ull,	0,		8},
	{OCX_LNK,	0x00011800FE180000ull,	0x8000,		8},
	{OCX_OLE,	0x00011800FE1A0000ull,	0x8000,		8},
	{OCX_TOP,	0x00011800FE088000ull,	0,		8},
	{OSM,		0x00011800FE6E0000ull,	0,		8},
	{PEM,		0x00011800FE600000ull,	0x8000,		8},
	{PKI_PBE,	0x00011800FE228000ull,	0,		8},
	{PKI_PFE,	0x00011800FE220000ull,	0,		8},
	{PKI_PIX,	0x00011800FE230000ull,	0,		8},
	{PKO,		0x00011800FEAA0000ull,	0,		8},
	{RAD,		0x00011800FE380000ull,	0,		8},
	{RNM,		0x00011800FE200000ull,	0,		8},
	{RST,		0x00011800FE030000ull,	0,		8},
	{SLI,		0x00011800FE8F8000ull,	0,		8},
	{SSO,		0x00011800FEB38000ull,	0,		8},
	{TIM,		0x00011800FE2C0000ull,	0,		8},
	{USBH,		0x00011800FE340000ull,	0,		8},
	{ZIP,		0x00011800FE1C0000ull,	0,		8},
	{INVALID_BLOCK_ID, 0,			0,		0}
};

/* DTX registers on the 73xx */
static struct dtx_reg_addr_rule dtx_regs_73xx[] = {
	{BCH,		0x00011800FE388000ull,	0,		8},
	{BGX,		0x00011800FE700000ull,	0x8000,		8},
	{CIU,		0x00011800FE808000ull,	0,		8},
	{DFA,		0x00011800FE1B8000ull,	0,		8},
	{DPI,		0x00011800FEEF8000ull,	0,		8},
	{FPA,		0x00011800FE940000ull,	0,		8},
	{GSER,		0x00011800FE480000ull,	0x8000,		8},
	{HNA,		0x00011800FE238000ull,	0,		8},
	{IOBN,		0x00011800FE780000ull,	0,		8},
	{IOBP,		0x00011800FE7A0000ull,	0,		8},
	{KEY,		0x00011800FE100000ull,	0,		8},
	{L2C_CBC,	0x00011800FE420000ull,	0x8000,		8},
	{L2C_MCI,	0x00011800FE2E0000ull,	0x8000,		8},
	{L2C_TAD,	0x00011800FE240000ull,	0x8000,		8},
	{LBK,		0x00011800FE090000ull,	0,		8},
	{LMC,		0x00011800FE440000ull,	0x8000,		8},
	{MIO,		0x00011800FE000000ull,	0,		8},
	{OSM,		0x00011800FEEE0000ull,	0,		8},
	{PEM,		0x00011800FE600000ull,	0x8000,		8},
	{PKI_PBE,	0x00011800FE228000ull,	0,		8},
	{PKI_PFE,	0x00011800FE220000ull,	0,		8},
	{PKI_PIX,	0x00011800FE230000ull,	0,		8},
	{PKO,		0x00011800FEAA0000ull,	0,		8},
	{RAD,		0x00011800FE380000ull,	0,		8},
	{RNM,		0x00011800FE200000ull,	0,		8},
	{RST,		0x00011800FE030000ull,	0,		8},
	{SATA,		0x00011800FE360000ull,	0,		8},
	{SLI,		0x00011800FE8F8000ull,	0,		8},
	{SPEM,		0x00011800FE600000ull,	0,		8},
	{SSO,		0x00011800FEB38000ull,	0,		8},
	{TIM,		0x00011800FE2C0000ull,	0,		8},
	{USBDRD,	0x00011800FE340000ull,	0x8000,		8},
	{XCV,		0x00011800FE6D8000ull,	0,		8},
	{ZIP,		0x00011800FE1C0000ull,	0,		8},
	{INVALID_BLOCK_ID, 0,			0,		0}
};

/* DTX registers on the 75xx */
static struct dtx_reg_addr_rule dtx_regs_75xx[] = {
	{BBX1I,		0x00011800FED78000ull,	0,		8},
	{BBX2I,		0x00011800FED80000ull,	0,		8},
	{BBX3I,		0X00011800FED88000ull,	0,		8},
	{BCH,		0x00011800FE388000ull,	0,		8},
	{BGX,		0x00011800FE700000ull,	0x8000,		8},
	{BTS,		0x00011800FE5B0000ULL,	0,		8},
	{CIU,		0x00011800FE808000ull,	0,		8},
	{DENC,		0x00011800FED48000ull,	0,		8},
	{DLFE,		0x00011800FED18000ull,	0,		8},
	{DPI,		0x00011800FEEF8000ull,	0,		8},
	{FDEQ,		0x00011800FED30000ull,	0x20000,	8},
	{FPA,		0x00011800FE940000ull,	0,		8},
	{GSER,		0x00011800FE480000ull,	0x8000,		8},
	{IOBN,		0x00011800FE780000ull,	0,		8},
	{IOBP,		0x00011800FE7A0000ull,	0,		8},
	{KEY,		0x00011800FE100000ull,	0,		8},
	{L2C_CBC,	0x00011800FE420000ull,	0x8000,		8},
	{L2C_MCI,	0x00011800FE2E0000ull,	0x8000,		8},
	{L2C_TAD,	0x00011800FE240000ull,	0x8000,		8},
	{LBK,		0x00011800FE090000ull,	0,		8},
	{LMC,		0x00011800FE440000ull,	0x8000,		8},
	{MDB,		0x00011800FEC00000ull,	0x8000,		8},
	{MHBW,		0x00011800FE598000ull,	0,		8},
	{MIO,		0x00011800FE000000ull,	0,		8},
	{PEM,		0x00011800FE600000ull,	0x8000,		8},
	{PKI_PBE,	0x00011800FE228000ull,	0,		8},
	{PKI_PFE,	0x00011800FE220000ull,	0,		8},
	{PKI_PIX,	0x00011800FE230000ull,	0,		8},
	{PKO,		0x00011800FEAA0000ull,	0,		8},
	{PNB,		0x00011800FE580000ull,	0x8000,		8},
	{PNBD,		0x00011800FED90000ull,	0x8000,		8},
	{PRCH,		0x00011800FED00000ull,	0,		8},
	{PSM,		0x00011800FEEA0000ull,	0,		8},
	{RDEC,		0x00011800FED68000ull,	0,		8},
	{RFIF,		0x00011800FE6A8000ull,	0,		8},
	{RMAP,		0x00011800FED40000ull,	0,		8},
	{RNM,		0x00011800FE200000ull,	0,		8},
	{RST,		0x00011800FE030000ull,	0,		8},
	{SLI,		0x00011800FE8F8000ull,	0,		8},
	{SRIO,		0x00011800FE640000ull,	0x8000,		8},
	{SSO,		0x00011800FEB38000ull,	0,		8},
	{TDEC,		0x00011800FED60000ull,	0,		8},
	{TIM,		0x00011800FE2C0000ull,	0,		8},
	{ULFE,		0x00011800FED08000ull,	0,		8},
	{USBDRD,	0x00011800FE340000ull,	0,		8},
	{VDEC,		0x00011800FED70000ull,	0,		8},
	{WPSE,		0x00011800FED10000ull,	0,		8},
	{WRCE,		0x00011800FED38000ull,	0,		8},
	{WRDE,		0x00011800FED58000ull,	0,		8},
	{WRSE,		0x00011800FED28000ull,	0,		8},
	{WTXE,		0x00011800FED20000ull,	0,		8},
	{XCV,		0x00011800FE6D8000ull,	0,		8},
	{XSX,		0x00011800FE5A8000ull,	0,		8},
	{INVALID_BLOCK_ID, 0,			0,		0}
};

/* Must keep track of which fsm AND terms are in use */
static uint16_t		and_terms[CVMX_MAX_NODES][MAX_COMPLEXES];

/* Must keep track of which fsm OR terms are in use */
static uint16_t		or_terms[CVMX_MAX_NODES][MAX_COMPLEXES];


/*
 * Get the register address for the given dtx block and register type.
 *
 *  block_id:		Specifies one of the DTX block types.
 *  block_inst:		Specifies the block instance.
 *  half:		36-bit debug bus half.
 *  reg_type:		Specifies the register to get the address for.
 * 
 *  Returns:		Register address on success, 0 otherwise.
 */
static uint64_t cvmx_get_dtx_reg_addr(cvmx_dtx_id_t	block_id,
				      int		block_inst,
				      int		half,
				      int		reg_type)
{
	struct dtx_reg_addr_rule	*regs = dtx_regs_78xx;
	uint64_t			addr;
	int				offset;

	if (OCTEON_IS_MODEL(OCTEON_CN70XX))
		regs = dtx_regs_70xx;
	else if (OCTEON_IS_MODEL(OCTEON_CN73XX))
		regs = dtx_regs_73xx;
	else if (OCTEON_IS_MODEL(OCTEON_CNF75XX))
		regs = dtx_regs_75xx;

	while ((block_id != regs->id) && (regs->id != INVALID_BLOCK_ID))
		regs++;

	if (regs->id == INVALID_BLOCK_ID) {
		cvmx_dprintf("Invalid dtx block ID [%d]\n", block_id);
		return 0;
	}

	switch (reg_type) {
	case DTX_SEL:
		offset = 0;
		break;
	case DTX_ENA:
		offset = 0x20;
		break;
	case DTX_DAT:
		offset = 0x40;
		break;
	case DTX_CTL:
		offset = 0x60;
		break;
	default:
		cvmx_dprintf("Invalid dtx register type [%d]\n",reg_type);
		return 0;
	}

	addr = CVMX_ADD_IO_SEG(regs->base) + offset +
		regs->inst_factor * block_inst + regs->half_factor * half;

	return addr;
}

/**
 * Disable all DTXs on all nodes.
 */
int cvmx_dtx_reset(void)
{
	cvmx_dtx_l2c_cbcx_bcst_rsp_t	bcst_rsp;
	int				node;

	if (!octeon_has_feature(OCTEON_FEATURE_OCLA)) {
		cvmx_dprintf("This chip does not have OCLA support.\n");
		return -1;
	}

	for (node = 0; node < CVMX_MAX_NODES; node++) {
		if (!OCTEON_IS_MODEL(OCTEON_CN78XX) && node)
			continue;

		/* Initialize broadcast */
		bcst_rsp.u64 = 0;
		bcst_rsp.s.ena = 1;
		cvmx_write_csr_node(node, CVMX_DTX_L2C_CBCX_BCST_RSP(0),
				    bcst_rsp.u64);

		/* Flush rml */
		cvmx_read_csr_node(node, CVMX_L2C_TBFX_BIST_STATUS(0));

		/* Clear all dtx's from driving data into the debug bus */
		cvmx_write_csr_node(node, CVMX_DTX_BROADCAST_CTL, 0);
		cvmx_write_csr_node(node, CVMX_DTX_BROADCAST_ENAX(0), 0);
		cvmx_write_csr_node(node, CVMX_DTX_BROADCAST_ENAX(1), 0);
		cvmx_write_csr_node(node, CVMX_DTX_BROADCAST_SELX(0), 0);
		cvmx_write_csr_node(node, CVMX_DTX_BROADCAST_SELX(1), 0);

		/* Flush rml */
		cvmx_read_csr_node(node, CVMX_L2C_TBFX_BIST_STATUS(0));
	}

	return 0;
}
EXPORT_SYMBOL(cvmx_dtx_reset);

/**
 * Setup a Debug Transmitter (DTX) hardware.
 *
 * @param node		Node to enable dtx on.
 * @param dtx		Pointer to DTX configuration to enable.
 * 
 * @return Returned value. Zero on success, -1 otherwise.
 */
int cvmx_dtx_enable(int			node,
		    cvmx_dtx_def_t	*dtx)
{
	uint64_t			addr;

	if (!octeon_has_feature(OCTEON_FEATURE_OCLA)) {
		cvmx_dprintf("This chip does not have OCLA support.\n");
		return -1;
	}

	addr = cvmx_get_dtx_reg_addr(dtx->id, dtx->inst, dtx->half, DTX_ENA);
	cvmx_write_csr_node(node, addr, dtx->ena);

	addr = cvmx_get_dtx_reg_addr(dtx->id, dtx->inst, dtx->half, DTX_SEL);
	cvmx_write_csr_node(node, addr, dtx->sel);	

	/* Flush rml */
	cvmx_read_csr_node(node, CVMX_L2C_TBFX_BIST_STATUS(0));

	return 0;
}
EXPORT_SYMBOL(cvmx_dtx_enable);

/**
 * Disable a DTX.
 *
 * @param node		Node to disable dtx on.
 * @param dtx		Pointer to DTX to disable.
 * 
 * @return Returned value. Zero on success, -1 otherwise.
 */
int cvmx_dtx_disable(int		node,
		     cvmx_dtx_def_t	*dtx)
{
	uint64_t			addr;

	if (!octeon_has_feature(OCTEON_FEATURE_OCLA)) {
		cvmx_dprintf("This chip does not have OCLA support.\n");
		return -1;
	}

	addr = cvmx_get_dtx_reg_addr(dtx->id, dtx->inst, dtx->half, DTX_ENA);
	cvmx_write_csr_node(node, addr, 0);

	addr = cvmx_get_dtx_reg_addr(dtx->id, dtx->inst, dtx->half, DTX_SEL);
	cvmx_write_csr_node(node, addr, 0);

	/* Flush rml */
	cvmx_read_csr_node(node, CVMX_L2C_TBFX_BIST_STATUS(0));

	return 0;
}
EXPORT_SYMBOL(cvmx_dtx_disable);

/**
 * Reset the OCLA hardware.
 *
 * @param node		Node ocla complex is on.
 * @param ix		OCLA complex to reset.
 * 
 * @return Returned value. Zero on success, -1 otherwise.
 */
int cvmx_ocla_reset(int	node,
		    int	ix)
{
	cvmx_oclax_gen_ctl_t		gen_ctl;
	cvmx_oclax_sft_rst_t		sft_rst;
	cvmx_oclax_state_int_t		state_int;

	if (!octeon_has_feature(OCTEON_FEATURE_OCLA)) {
		cvmx_dprintf("This chip does not have OCLA support.\n");
		return -1;
	}

	/* Reset the OCLA complex */
	sft_rst.u64 = 0;
	sft_rst.s.reset = 1;
	cvmx_write_csr_node(node, CVMX_OCLAX_SFT_RST(ix), sft_rst.u64);
	do {
		sft_rst.u64 = cvmx_read_csr_node(node, CVMX_OCLAX_SFT_RST(ix));
	} while (sft_rst.s.reset == 1);

	/* Clear registers */
	cvmx_write_csr_node(node, CVMX_OCLAX_GEN_CTL(ix), 0);
	cvmx_write_csr_node(node, CVMX_OCLAX_FIFO_TRIG(ix), 0);
	cvmx_write_csr_node(node, CVMX_OCLAX_STATE_SET(ix), 0);

	/* Flush rml */
	cvmx_read_csr_node(node, CVMX_L2C_TBFX_BIST_STATUS(0));

	/* Clear interrupts */
	state_int.u64 = 0;
	state_int.s.fsm0_int = 1;
	state_int.s.fsm1_int = 1;
	state_int.s.trigfull = 1;
	state_int.s.captured = 1;
	cvmx_write_csr_node(node, CVMX_OCLAX_STATE_INT(ix), state_int.u64);

	/* Enable ocla */
	gen_ctl.u64 = cvmx_read_csr_node(node, CVMX_OCLAX_GEN_CTL(ix));
	gen_ctl.s.den = 1;
	cvmx_write_csr_node(node, CVMX_OCLAX_GEN_CTL(ix), gen_ctl.u64);

	return 0;
}
EXPORT_SYMBOL(cvmx_ocla_reset);

/**
 * Clear the OCLA interrupts.
 *
 * @param node		Node ocla complex is on.
 * @param ix		OCLA complex whose interrupts are to be cleared.
 * 
 * @return Returned value. Zero on success, -1 otherwise.
 */
int cvmx_ocla_clear_interrupts(int	node,
			       int	ix)
{
	cvmx_oclax_state_int_t		state_int;

	if (!octeon_has_feature(OCTEON_FEATURE_OCLA)) {
		cvmx_dprintf("This chip does not have OCLA support.\n");
		return -1;
	}

	/* Clear interrupts */
	state_int.u64 = 0;
	state_int.s.fsm0_int = 1;
	state_int.s.fsm1_int = 1;
	state_int.s.trigfull = 1;
	state_int.s.captured = 1;
	cvmx_write_csr_node(node, CVMX_OCLAX_STATE_INT(ix), state_int.u64);

	return 0;
}
EXPORT_SYMBOL(cvmx_ocla_clear_interrupts);

/**
 * Disable the OCLA hardware.
 *
 * @param node		Node ocla complex is on.
 * @param ix		OCLA complex to disable.
 * 
 * @return Returned value. Zero on success, -1 otherwise.
 */
int cvmx_ocla_disable(int	node,
		      int	ix)
{
	cvmx_oclax_gen_ctl_t		gen_ctl;

	if (!octeon_has_feature(OCTEON_FEATURE_OCLA)) {
		cvmx_dprintf("This chip does not have OCLA support.\n");
		return -1;
	}

	/* Disable ocla */
	gen_ctl.u64 = cvmx_read_csr_node(node, CVMX_OCLAX_GEN_CTL(ix));
	gen_ctl.s.den = 0;
	cvmx_write_csr_node(node, CVMX_OCLAX_GEN_CTL(ix), gen_ctl.u64);

	return 0;
}
EXPORT_SYMBOL(cvmx_ocla_disable);

/**
 * Initialize the OCLA hardware.
 *
 * @param node		Node ocla complex is on.
 * @param ix		OCLA complex to initialize.
 * @param buf		Pointer to ddr buffer to use as capture buffer.
 * @param buf_size	Size of ddr capture buffer.
 * @param num_entries	Number of entries to capture. Zero for wrapping mode.
 * @param lo_cap	Selects when to capture the lower 36-bit debug bus half.
 * @param hi_cap	Selects when to capture the upper 36-bit debug bus half.
 * 
 * @return Returned value. Zero on success, -1 otherwise.
 */
int cvmx_ocla_init(int			node,
		   int			ix,
		   uint64_t		buf,
		   uint			buf_size,
		   int			num_entries,
		   cvmx_cap_sel_t	lo_cap,
		   cvmx_cap_sel_t	hi_cap)
{
	cvmx_oclax_time_t		time;
	cvmx_oclax_fifo_trig_t		fifo_trig;
	cvmx_oclax_cdhx_ctl_t		cdhx_ctl;

	if (!octeon_has_feature(OCTEON_FEATURE_OCLA)) {
		cvmx_dprintf("This chip does not have OCLA support.\n");
		return -1;
	}

	/* Initialize variables */
	and_terms[node][ix] = 0;
	or_terms[node][ix] = 0;

	/* Clear the time */
	time.u64 = cvmx_read_csr_node(node, CVMX_OCLAX_TIME(ix));
	time.s.cycle = 0;
	cvmx_write_csr_node(node, CVMX_OCLAX_TIME(ix), time.u64);

	/* Use the ddr buffer if present */
	if (buf && buf_size) {
		cvmx_oclax_stack_base_t	base;
		cvmx_oclax_stack_top_t	top;
		cvmx_oclax_stack_cur_t	cur;
		cvmx_oclax_fifo_limit_t	limit;

		base.u64 = 0;
		base.s.ptr = buf >> 7;
		cvmx_write_csr_node(node, CVMX_OCLAX_STACK_BASE(ix), base.u64);

		top.u64 = 0;
		top.s.ptr = (buf + buf_size + CVMX_CACHE_LINE_SIZE) >> 7;
		cvmx_write_csr_node(node, CVMX_OCLAX_STACK_TOP(ix), top.u64);

		cur.u64 = 0;
		cur.s.ptr = buf >> 7;
		cvmx_write_csr_node(node, CVMX_OCLAX_STACK_CUR(ix), cur.u64);

		cvmx_write_csr_node(node, CVMX_OCLAX_STACK_WRAP(ix), 0);
		cvmx_write_csr_node(node, CVMX_OCLAX_STACK_STORE_CNT(ix), 0);

		limit.u64 = cvmx_read_csr_node(node, CVMX_OCLAX_FIFO_LIMIT(ix));
		/*
		 * To accomodate 78xx'x pass1 bug, ddr must be:
		 * ddr > 28
		 * ddr % 26 == 0
		 */
		limit.s.ddr = 52;
		cvmx_write_csr_node(node, CVMX_OCLAX_FIFO_LIMIT(ix), limit.u64);
	}

	/* Stop capture after num_entries entries have been captured */
	fifo_trig.u64 = 0;
	fifo_trig.s.limit = num_entries;
	cvmx_write_csr_node(node, CVMX_OCLAX_FIFO_TRIG(ix), fifo_trig.u64);

	/* Configure when to capture for both the lower and upper 36 bits */
	cdhx_ctl.u64 = 0;
	cdhx_ctl.s.dup = 1;
	cdhx_ctl.s.cap_ctl = lo_cap;
	cvmx_write_csr_node(node, CVMX_OCLAX_CDHX_CTL(0, ix), cdhx_ctl.u64);

	cdhx_ctl.s.cap_ctl = hi_cap;
	cvmx_write_csr_node(node, CVMX_OCLAX_CDHX_CTL(1, ix), cdhx_ctl.u64);

	return 0;
}
EXPORT_SYMBOL(cvmx_ocla_init);

/**
 * Initialize the OCLA matchers hardware.
 *
 * @param node		Node matcher is on.
 * @param ix		OCLA complex to initialize.
 * @param mat		Matcher to initialize.
 * @param lo_mask	Mask for the lower 36-bit debug bus half.
 * @param lo_val	Value to match in the lower 36-bit debug bus half.
 * @param hi_mask	Mask for the upper 36-bit debug bus half.
 * @param hi_val	Value to match in the upper 36-bit debug bus half.
 * 
 * @return Returned value. Zero on success, -1 otherwise.
 */
int cvmx_matcher_init(int	node,
		      int	ix,
		      int	mat,
		      uint64_t	lo_mask,
		      uint64_t	lo_val,
		      uint64_t	hi_mask,
		      uint64_t	hi_val)
{
	if (!octeon_has_feature(OCTEON_FEATURE_OCLA)) {
		cvmx_dprintf("This chip does not have OCLA support.\n");
		return -1;
	}

	/* Lower 36-bit debug bus half */
	cvmx_write_csr_node(node, CVMX_OCLAX_MATX_MASKX(ix, mat, 0), lo_mask);
	cvmx_write_csr_node(node, CVMX_OCLAX_MATX_VALUEX(ix, mat, 0), lo_val);

	/* Upper 36-bit debug bus half */
	cvmx_write_csr_node(node, CVMX_OCLAX_MATX_MASKX(ix, mat, 1), hi_mask);
	cvmx_write_csr_node(node, CVMX_OCLAX_MATX_VALUEX(ix, mat, 1), hi_val);

	return 0;
}
EXPORT_SYMBOL(cvmx_matcher_init);

/*
 * Allocate a free AND term.
 *
 *  node:		Node to allocate AND term from.
 *  ix:			OCLA complex to allocate AND term from.
 *
 *  Returns:		AND term or -1 on error.
 */
static int and_term_alloc(int	node,
			  int	ix)
{
	int	i;

	/*
	 * Start allocating from term 15 to avoid using term 0 since term 0 is
	 * the default term.
	 */
	for (i = 15; i >= 0; i--) {
		if (!(and_terms[node][ix] & (1 << i))) {
			and_terms[node][ix] |= 1 << i;
			break;
		}
	}

	return i;
}

/**
 * Initialize the OCLA finite state machine hardware.
 *
 * @param node		Node fsm is on.
 * @param ix		OCLA complex to initialize.
 * @param fsm		Fsm to initialize.
 * @param mcd		MCD inputs to use.
 * @param matcher	Matcher inputs to use.
 * @param fsm0_st	Fsm0 state to use as input.
 * @param fsm1_st	Fsm1 state to use as input.
 * @param new_st	State to transtion to when all inputs are true.
 * 
 * @return Returned value. Zero on success, -1 otherwise.
 */
int cvmx_fsm_init(int			node,
		  int			ix,
		  int			fsm,
		  cvmx_fsm_input_t	*mcd,
		  cvmx_fsm_input_t	*matcher,
		  cvmx_fsm_input_t	*fsm0_st,
		  cvmx_fsm_input_t	*fsm1_st,
		  uint			new_st)
{
	cvmx_oclax_fsmx_andx_ix_t	fsm_and;
	cvmx_oclax_fsmx_andx_ix_t	fsm_and_i;
	cvmx_oclax_fsmx_orx_t		fsm_or;
	int				and_term;

	if (!octeon_has_feature(OCTEON_FEATURE_OCLA)) {
		cvmx_dprintf("This chip does not have OCLA support.\n");
		return -1;
	}

	/* Select a free AND term to use and check the new state is available */
	if ((and_term = and_term_alloc(node, ix)) < 0) {
		cvmx_dprintf("Failed to allocate a free AND term.\n");
		return -1;
	}
	if (new_st > 15 || or_terms[node][ix] & (1 << new_st)) {
		cvmx_dprintf("Invalid new state.\n");
		return -1;
	}
	or_terms[node][ix] = 1 << new_st;

	/* Start with zero values */
	fsm_and.u64 = 0;
	fsm_and_i.u64 = 0;
	fsm_or.u64 = 0;

	/* To disable an input the input and its complement must be set to 1 */
	fsm_and.s.trig 		= 1;
	fsm_and_i.s.trig 	= 1;
	fsm_and.s.mcd 		= ~mcd->msk;
	fsm_and_i.s.mcd 	= ~mcd->msk;
	fsm_and.s.match		= ~matcher->msk;
	fsm_and_i.s.match	= ~matcher->msk;
	fsm_and.s.fsm0_state	= ~fsm0_st->msk;
	fsm_and_i.s.fsm0_state	= ~fsm0_st->msk;
	fsm_and.s.fsm1_state	= ~fsm1_st->msk;
	fsm_and_i.s.fsm1_state	= ~fsm1_st->msk;

	/* Set the inputs that must be 1 */
	fsm_and.s.mcd 		|= mcd->msk & mcd->val;
	fsm_and.s.match		|= matcher->msk & matcher->val;
	fsm_and.s.fsm0_state	|= fsm0_st->msk & fsm0_st->val;
	fsm_and.s.fsm1_state	|= fsm1_st->msk & fsm1_st->val;

	/* Set the inputs that must be 0 */
	fsm_and_i.s.mcd		|= mcd->msk & ~mcd->val;
	fsm_and_i.s.match	|= matcher->msk & ~matcher->val;
	fsm_and_i.s.fsm0_state	|= fsm0_st->msk & ~fsm0_st->val;
	fsm_and_i.s.fsm1_state	|= fsm1_st->msk & ~fsm1_st->val;

	/* Set the AND term */
	cvmx_write_csr_node(node, CVMX_OCLAX_FSMX_ANDX_IX(ix, fsm, and_term, 0),
		       fsm_and.u64);
	cvmx_write_csr_node(node, CVMX_OCLAX_FSMX_ANDX_IX(ix, fsm, and_term, 1),
		       fsm_and_i.u64);

	/* The OR term determines the new state */
	if (IS_OCLA_REV1)
		fsm_or.s.or_state = and_term;
	else
		fsm_or.s.or_state = 1 << and_term;

	cvmx_write_csr_node(node, CVMX_OCLAX_FSMX_ORX(ix, fsm, new_st),
			    fsm_or.u64);

	return 0;
}
EXPORT_SYMBOL(cvmx_fsm_init);

/**
 * Enable a fsm.
 *
 * @param node		Node fsm is on.
 * @param ix		OCLA complex fsm is on.
 * @param fsm		Fsm to enable.
 * 
 * @return Returned value. Zero on success, -1 otherwise.
 */
int cvmx_fsm_enable(int	node,
		    int	ix,
		    int	fsm)
{
	cvmx_oclax_state_set_t		state_set;

	if (!octeon_has_feature(OCTEON_FEATURE_OCLA)) {
		cvmx_dprintf("This chip does not have OCLA support.\n");
		return -1;
	}

	state_set.u64 = cvmx_read_csr_node(node, CVMX_OCLAX_STATE_SET(ix));
	switch (fsm) {
	case 0:
		state_set.s.fsm0_ena = 1;
		break;
	case 1:
		state_set.s.fsm1_ena = 1;
		break;
	default:
		cvmx_dprintf("Invalid fsm.\n");
		return -1;
	}
	cvmx_write_csr_node(node, CVMX_OCLAX_STATE_SET(ix), state_set.u64);

	/* Flush rml */
	cvmx_read_csr_node(node, CVMX_L2C_TBFX_BIST_STATUS(0));

	return 0;
}
EXPORT_SYMBOL(cvmx_fsm_enable);

/**
 * Set the action to take for a fsm state.
 *
 * @param node		Node fsm is on.
 * @param ix		OCLA complex fsm is on.
 * @param fsm		Fsm to initialize.
 * @param state		State to assign action to.
 * @param action	Bit mask of actions for the fsm state.
 * 
 * @return Returned value. Zero on success, -1 otherwise.
 */
int cvmx_fsm_state_set(int			node,
		       int			ix,
		       int			fsm,
		       uint			state,
		       cvmx_fsm_action_t	action)
{
	cvmx_oclax_fsmx_statex_t	fsm_state;

	if (!octeon_has_feature(OCTEON_FEATURE_OCLA)) {
		cvmx_dprintf("This chip does not have OCLA support.\n");
		return -1;
	}

	/* Set the action for the new state */
	fsm_state.u64 = cvmx_read_csr_node(node, CVMX_OCLAX_FSMX_STATEX(ix, fsm,
									state));
	if (action & ACTION_CAP) {
		fsm_state.s.cap = 1;
		fsm_state.s.set_trig = 1;
	}
	if (action & ACTION_INT)
		fsm_state.s.set_int = 1;

	cvmx_write_csr_node(node, CVMX_OCLAX_FSMX_STATEX(ix, fsm, state),
			    fsm_state.u64);

	return 0;
}
EXPORT_SYMBOL(cvmx_fsm_state_set);

/**
 * Read a packet from the fifo.
 *
 * @param node		Node ocla complex is on.
 * @param ix		OCLA complex to read from.
 * @param data		Updated with packet read.
 * 
 * @return Returned value. Zero if packet is valid, -1 otherwise.
 */
int cvmx_ocla_get_packet(int		node,
			 int		ix,
			 uint64_t	*data)
{
	cvmx_oclax_dat_pop_t	dat_pop;
	int			rc = -1;

	if (!octeon_has_feature(OCTEON_FEATURE_OCLA)) {
		cvmx_dprintf("This chip does not have OCLA support.\n");
		return -1;
	}

	dat_pop.u64 = cvmx_read_csr_node(node, CVMX_OCLAX_DAT_POP(ix));
	if (dat_pop.s.valid) {
		*data = dat_pop.u64;
		rc = 0;
	}

	return rc;
}
EXPORT_SYMBOL(cvmx_ocla_get_packet);
