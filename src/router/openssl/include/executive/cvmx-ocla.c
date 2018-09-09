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
#else
#include "cvmx.h"
#include "cvmx-ocla.h"
#include <cvmx-dtx-defs.h>
#include <cvmx-l2c-defs.h>
#endif


/* Must keep track of which fsm AND terms are in use */
static uint16_t		and_terms;

/* Must keep track of which fsm OR terms are in use */
static uint16_t		or_terms;


/**
 * Setup the Debug Transmitters (DTX) hardware.
 *
 * @param lo_dtxs	Pointer to array of DTXs sharing the lower 36-bit debug
 *			bus half.
 * @param lo_cnt	Number of DTXs sharing the lower half.
 * @param hi_dtxs	Pointer to array of DTXs sharing the upper 36-bit debug
 *			bus half.
 * @param hi_cnt	Number of DTXs sharing the upper half.
 * 
 * @return Returned value. Zero on success, -1 otherwise.
 */
int cvmx_dtx_init(cvmx_dtx_cfg_info_t	*lo_dtxs,
		  int			lo_cnt,
		  cvmx_dtx_cfg_info_t	*hi_dtxs,
		  int			hi_cnt)
{
	cvmx_dtx_l2c_cbcx_bcst_rsp_t	bcst_rsp;
	int				i;

	if (!octeon_has_feature(OCTEON_FEATURE_OCLA)) {
		printf("This chip does not have OCLA support.\n");
		return -1;
	}

	/* Initialize broadcast */
	bcst_rsp.u64 = 0;
	bcst_rsp.s.ena = 1;
	cvmx_write_csr(CVMX_DTX_L2C_CBCX_BCST_RSP(0),bcst_rsp.u64);

	/* Flush rml */
	cvmx_read_csr(CVMX_L2C_TBFX_BIST_STATUS(0));

	/* Clear all dtx's from driving data into the debug bus */
	cvmx_write_csr(CVMX_DTX_BROADCAST_CTL, 0);
	cvmx_write_csr(CVMX_DTX_BROADCAST_ENAX(0), 0);
	cvmx_write_csr(CVMX_DTX_BROADCAST_ENAX(1), 0);
	cvmx_write_csr(CVMX_DTX_BROADCAST_SELX(0), 0);
	cvmx_write_csr(CVMX_DTX_BROADCAST_SELX(1), 0);

	/* Flush rml */
	cvmx_read_csr(CVMX_L2C_TBFX_BIST_STATUS(0));

	/* Do the lower 36-bit debug bus half */
	for (i = 0; i < lo_cnt; i++) {
		int	block_ix = lo_dtxs[i].block_ix;

		switch (lo_dtxs[i].block) {
		case DTX_L2C:
			cvmx_write_csr(CVMX_DTX_L2C_CBCX_ENAX(0, block_ix),
				       lo_dtxs[i].ena);
			cvmx_write_csr(CVMX_DTX_L2C_CBCX_SELX(0, block_ix),
				       lo_dtxs[i].sel);
			break;

		case DTX_LMC:
			cvmx_write_csr(CVMX_DTX_LMCX_ENAX(0, block_ix),
				       lo_dtxs[i].ena);
			cvmx_write_csr(CVMX_DTX_LMCX_SELX(0, block_ix),
				       lo_dtxs[i].sel);
			break;
		}
	}

	/* Do the upper 36-bit debug bus half */
	for (i = 0; i < hi_cnt; i++) {
		int	block_ix = hi_dtxs[i].block_ix;

		switch (hi_dtxs[i].block) {
		case DTX_L2C:
			cvmx_write_csr(CVMX_DTX_L2C_CBCX_ENAX(1, block_ix),
				       hi_dtxs[i].ena);
			cvmx_write_csr(CVMX_DTX_L2C_CBCX_SELX(1, block_ix),
				       hi_dtxs[i].sel);
			break;

		case DTX_LMC:
			cvmx_write_csr(CVMX_DTX_LMCX_ENAX(1, block_ix),
				       hi_dtxs[i].ena);
			cvmx_write_csr(CVMX_DTX_LMCX_SELX(1, block_ix),
				       hi_dtxs[i].sel);
			break;
		}
	}

	/* Flush rml */
	cvmx_read_csr(CVMX_L2C_TBFX_BIST_STATUS(0));

	return 0;
}

/**
 * Enable the OCLA hardware.
 *
 * @param ix		OCLA complex to enable.
 * 
 * @return Returned value. Zero on success, -1 otherwise.
 */
int cvmx_ocla_enable(int	ix)
{
	cvmx_oclax_gen_ctl_t		gen_ctl;
	cvmx_oclax_sft_rst_t		sft_rst;
	cvmx_ciu_cib_oclax_rawx_t	raw;
	cvmx_oclax_state_int_t		state_int;

	if (!octeon_has_feature(OCTEON_FEATURE_OCLA)) {
		printf("This chip does not have OCLA support.\n");
		return -1;
	}

	/* Reset the OCLA complex */
	sft_rst.u64 = 0;
	sft_rst.s.reset = 1;
	cvmx_write_csr(CVMX_OCLAX_SFT_RST(ix), sft_rst.u64);

	/* Clear registers */
	cvmx_write_csr(CVMX_OCLAX_GEN_CTL(ix), 0);
	cvmx_write_csr(CVMX_OCLAX_FIFO_TRIG(ix), 0);
	cvmx_write_csr(CVMX_OCLAX_STATE_SET(ix), 0);

	/* Flush rml */
	cvmx_read_csr(CVMX_L2C_TBFX_BIST_STATUS(0));

	/* Clear interrupts */
	state_int.u64 = 0;
	state_int.s.fsm0_int = 1;
	state_int.s.fsm1_int = 1;
	cvmx_write_csr(CVMX_OCLAX_STATE_INT(ix), state_int.u64);

	raw.u64 = 0;
	raw.s.state_fsm0_int = 1;
	raw.s.state_fsm1_int = 1;
	cvmx_write_csr(CVMX_CIU_CIB_OCLAX_RAWX(0, ix), raw.u64);

	/* Enable ocla */
	gen_ctl.u64 = cvmx_read_csr(CVMX_OCLAX_GEN_CTL(ix));
	gen_ctl.s.den = 1;
	cvmx_write_csr(CVMX_OCLAX_GEN_CTL(ix), gen_ctl.u64);

	return 0;
}

/**
 * Disable the OCLA hardware.
 *
 * @param ix		OCLA complex to disable.
 * 
 * @return Returned value. Zero on success, -1 otherwise.
 */
int cvmx_ocla_disable(int	ix)
{
	cvmx_oclax_gen_ctl_t		gen_ctl;

	if (!octeon_has_feature(OCTEON_FEATURE_OCLA)) {
		printf("This chip does not have OCLA support.\n");
		return -1;
	}

	/* Disable ocla */
	gen_ctl.u64 = cvmx_read_csr(CVMX_OCLAX_GEN_CTL(ix));
	gen_ctl.s.den = 0;
	cvmx_write_csr(CVMX_OCLAX_GEN_CTL(ix), gen_ctl.u64);

	return 0;
}

/**
 * Initialize the OCLA hardware.
 *
 * @param ix		OCLA complex to initialize.
 * @param num_entries	Number of entries to capture. Zero for wrapping mode.
 * @param lo_cap	Selects when to capture the lower 36-bit debug bus half.
 * @param hi_cap	Selects when to capture the upper 36-bit debug bus half.
 * 
 * @return Returned value. Zero on success, -1 otherwise.
 */
int cvmx_ocla_init(int			ix,
		   int			num_entries,
		   cvmx_cap_sel_t	lo_cap,
		   cvmx_cap_sel_t	hi_cap)
{
	cvmx_oclax_time_t		time;
	cvmx_oclax_const_t		constant;
	cvmx_oclax_fifo_trig_t		fifo_trig;
	cvmx_oclax_cdhx_ctl_t		cdhx_ctl;

	if (!octeon_has_feature(OCTEON_FEATURE_OCLA)) {
		printf("This chip does not have OCLA support.\n");
		return -1;
	}

	/* Initialize variables */
	and_terms = 0;
	or_terms = 0;

	/* Clear the time */
	time.u64 = cvmx_read_csr(CVMX_OCLAX_TIME(ix));
	time.s.cycle = 0;
	cvmx_write_csr(CVMX_OCLAX_TIME(ix), time.u64);

	/*
	 * Set the number of entries to collect after trigger event to the size
	 * of ram.
	 */
	fifo_trig.u64 = 0;
	if (num_entries) {
		int	max_entries;

		constant.u64 = cvmx_read_csr(CVMX_OCLAX_CONST(ix));
		max_entries = constant.s.dat_size - 5;
		fifo_trig.s.limit = num_entries <= max_entries ? num_entries :
			max_entries;
	}
	cvmx_write_csr(CVMX_OCLAX_FIFO_TRIG(ix), fifo_trig.u64);

	/* Configure when to capture for both the lower and upper 36 bits */
	cdhx_ctl.u64 = 0;
	cdhx_ctl.s.dup = 1;
	cdhx_ctl.s.cap_ctl = lo_cap;
	cvmx_write_csr(CVMX_OCLAX_CDHX_CTL(0, ix), cdhx_ctl.u64);

	cdhx_ctl.s.cap_ctl = hi_cap;
	cvmx_write_csr(CVMX_OCLAX_CDHX_CTL(1, ix), cdhx_ctl.u64);

	return 0;
}

/**
 * Initialize the OCLA matchers hardware.
 *
 * @param ocla_ix	OCLA complex to initialize.
 * @param ix		Matcher to initialize.
 * @param lo_mask	Mask for the lower 36-bit debug bus half.
 * @param lo_val	Value to match in the lower 36-bit debug bus half.
 * @param hi_mask	Mask for the upper 36-bit debug bus half.
 * @param hi_val	Value to match in the upper 36-bit debug bus half.
 * 
 * @return Returned value. Zero on success, -1 otherwise.
 */
int cvmx_matcher_init(int	ocla_ix,
		      int	ix,
		      uint64_t	lo_mask,
		      uint64_t	lo_val,
		      uint64_t	hi_mask,
		      uint64_t	hi_val)
{
	if (!octeon_has_feature(OCTEON_FEATURE_OCLA)) {
		printf("This chip does not have OCLA support.\n");
		return -1;
	}

	/* Lower 36-bit debug bus half */
	cvmx_write_csr(CVMX_OCLAX_MATX_MASKX(ocla_ix, ix, 0), lo_mask);
	cvmx_write_csr(CVMX_OCLAX_MATX_VALUEX(ocla_ix, ix, 0), lo_val);

	/* Upper 36-bit debug bus half */
	cvmx_write_csr(CVMX_OCLAX_MATX_MASKX(ocla_ix, ix, 1), hi_mask);
	cvmx_write_csr(CVMX_OCLAX_MATX_VALUEX(ocla_ix, ix, 1), hi_val);

	return 0;
}

/*
 * Allocate a free AND term.
 *
 * @return Returned value. AND term or -1 on error.
 */
static int and_term_alloc(void)
{
	int	i;

	/*
	 * Start allocating from term 15 to avoid using term 0 since term 0 is
	 * the default term.
	 */
	for (i = 15; i >= 0; i--) {
		if (!(and_terms & (1 << i))) {
			and_terms |= 1 << i;
			break;
		}
	}

	return i;
}

/**
 * Initialize the OCLA finite state machine hardware.
 *
 * @param ocla_ix	OCLA complex to initialize.
 * @param ix		Fsm to initialize.
 * @param mcd		MCD inputs to use.
 * @param matcher	Matcher inputs to use.
 * @param fsm0_st	Fsm0 state to use as input.
 * @param fsm1_st	Fsm1 state to use as input.
 * @param new_st	State to transtion to when all inputs are true.
 * 
 * @return Returned value. Zero on success, -1 otherwise.
 */
int cvmx_fsm_init(int			ocla_ix,
		  int			ix,
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
		printf("This chip does not have OCLA support.\n");
		return -1;
	}

	/* Select a free AND term to use and check the new state is available */
	if ((and_term = and_term_alloc()) < 0) {
		cvmx_dprintf("Failed to allocate a free AND term.\n");
		return -1;
	}
	if (new_st > 15 || or_terms & (1 << new_st)) {
		cvmx_dprintf("Invalid new state.\n");
		return -1;
	}

	/* Start with zero values */
	fsm_and.u64 = 0;
	fsm_and_i.u64 = 0;
	fsm_or.u64 = 0;

	/* To disable an input the input and its complement must be set to 1 */
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
	cvmx_write_csr(CVMX_OCLAX_FSMX_ANDX_IX(ocla_ix, ix, and_term, 0),
		       fsm_and.u64);
	cvmx_write_csr(CVMX_OCLAX_FSMX_ANDX_IX(ocla_ix, ix, and_term, 1),
		       fsm_and_i.u64);

	/* The OR term determines the new state */
	fsm_or.s.or_state = and_term;
	cvmx_write_csr(CVMX_OCLAX_FSMX_ORX(ocla_ix, ix, new_st), fsm_or.u64);

	return 0;
}

/**
 * Enable a fsm.
 *
 * @param ocla_ix	OCLA complex to initialize.
 * @param ix		Fsm to enable.
 * 
 * @return Returned value. Zero on success, -1 otherwise.
 */
int cvmx_fsm_enable(int	ocla_ix,
		    int	ix)
{
	cvmx_oclax_state_set_t		state_set;

	if (!octeon_has_feature(OCTEON_FEATURE_OCLA)) {
		printf("This chip does not have OCLA support.\n");
		return -1;
	}

	state_set.u64 = cvmx_read_csr(CVMX_OCLAX_STATE_SET(ocla_ix));
	switch (ix) {
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
	cvmx_write_csr(CVMX_OCLAX_STATE_SET(ocla_ix), state_set.u64);

	/* Flush rml */
	cvmx_read_csr(CVMX_L2C_TBFX_BIST_STATUS(0));

	return 0;
}

/**
 * Set the action to take for a fsm state.
 *
 * @param ocla_ix	OCLA complex to initialize.
 * @param ix		Fsm to initialize.
 * @param state		State to assign action to.
 * @param action	Bit mask of actions for the fsm state.
 * 
 * @return Returned value. Zero on success, -1 otherwise.
 */
int cvmx_fsm_state_set(int			ocla_ix,
		       int			ix,
		       uint			state,
		       cvmx_fsm_action_t	action)
{
	cvmx_oclax_fsmx_statex_t	fsm_state;

	if (!octeon_has_feature(OCTEON_FEATURE_OCLA)) {
		printf("This chip does not have OCLA support.\n");
		return -1;
	}

	/* Set the action for the new state */
	fsm_state.u64 = cvmx_read_csr(CVMX_OCLAX_FSMX_STATEX(ocla_ix, ix, 
							     state));
	if (action & ACTION_CAP)
		fsm_state.s.cap = 1;
	if (action & ACTION_INT)
		fsm_state.s.set_int = 1;

	cvmx_write_csr(CVMX_OCLAX_FSMX_STATEX(ocla_ix, ix, state),
		       fsm_state.u64);

	return 0;
}

/**
 * Read a packet from the fifo.
 *
 * @param ix		OCLA complex to read from.
 * @param data		Updated with packet read.
 * 
 * @return Returned value. Zero if packet is valid, -1 otherwise.
 */
int cvmx_ocla_get_packet(int		ix,
			 uint64_t	*data)
{
	cvmx_oclax_dat_pop_t	dat_pop;
	int			rc = -1;

	if (!octeon_has_feature(OCTEON_FEATURE_OCLA)) {
		printf("This chip does not have OCLA support.\n");
		return -1;
	}

	dat_pop.u64 = cvmx_read_csr(CVMX_OCLAX_DAT_POP(0));
	if (dat_pop.s.valid) {
		*data = dat_pop.u64;
		rc = 0;
	}

	return rc;
}
