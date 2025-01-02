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
 * The ocla provides capabilities to trace internal transactions for software
 * debugging, performance tuning, and hardware diagnostics.
 *
 * The ocla receives 72 bits of data from coprocessors or capture points via
 * debug bus transmitters (DTXs). These 72 bits of data are broken up into two
 * halves of 36 bits each.
 *
 *                          ----------------------------
 *                          |                          |
 *         data from DTXs   |                          |
 *        ----------------->|         ocla             |
 *         2 36-bit halves  |                          |
 *             (72 bits)    |                          |
 *                          ----------------------------
 *
 * The ocla alignes each bit of data (Aligner).
 * The aligner passes the data to the 4 matchers.
 * The matchers pass the data to the FSMs.
 * The FSMs determine when to capture data. When the FSM requests a capture, the
 * data is stored in the data fifo and optionally overflows to ddr/L2 cache.
 *
 *                  X 1               X 4             X 2              X 1
 *              -----------       -----------       -------       -------------
 *         DTX  |         |       |         |       |     |       |           |
 *        ----->| Aligner |------>| Matcher |------>| FSM |------>| data fifo |
 *              |         |   ^   |         |       |     |   |   |           |
 *              -----------   |   -----------       -------   |   -------------
 *                            |                               |
 *                            ---------------------------------
 *
 * Debug Bus Transmitters (DTX):
 *	. The DTXs feed the OCLA.
 *	. The DTXs get data from points outside the ocla.
 *	. Each dtx can drive one or both 36-bit halves.
 *	. Each 36-bit halve can be shared by multiple DTXs, but each DTX must 
 *	  drive a different set of bits.
 *	. Each DTX must be configured to select which data format will be put on
 *	  the 36-bit halve. The data formats are hardcoded in the hardware.
 *	  Software simple selects the format to use.
 *
 * Aligner:
 *	. The aligner delays each bit to correct for the variable amount of
 *	  propagation delay between signals from different blocks.
 *
 * Matchers:
 *      . Each matcher contains a mask and data to compere the aligned data
 *	  with.
 *	. Each matcher also contains a 32-bit counter. This counter can be used
 *	  to count the number of matches or to collect samples of capture data
 *	  (sample every Nth match, or match after every Nth cycle).
 *
 * FSMs:
 *	. Each FSM is implemented as a programmable logic array (PLA) style
 *	  organization, with 15 inputs and 4 outputs.
 *
 *	. The 15 inputs are:
 *	    . Matchers output (x4).
 *	    . Previous state of FSM0 (x4).
 *	    . Previous state of FSM1 (x4).
 *	    . Multicore debug wires (x3).
 *
 *	. The 15 inputs are manipulated and produce the 4 outputs.
 *	. The 4 outputs select one of 16 states the fsm can transtion to.
 *	. Each of the 16 states can be configured to capture data, generate
 *	  an interrupt, etc.
 *
 *	. The FSM implementation is that of an PLA:
 *
 *                          AND0  AND1        AND15
 *                            |    |   .....    |
 *            mcd      -------+----+------------+--
 *                            |    |            |
 *            matchers -------+----+------------+--
 *                            |    |            |
 *            fsm0     -------+----+------------+--
 *                            |    |            |
 *            fsm1     -------+----+------------+--
 *                            |    |            |
 *                            |    |            |
 *                          --+----+------------+-------- OR0
 *                            |    |            |
 *                          --+----+------------+-------- OR1
 *                            |    |            |    .
 *                            |    |            |    .
 *                            |    |            |    .
 *                            |    |            |    .
 *                          --+----+------------+-------- OR15
 *                            |    |            |
 *                            |    |            |
 *
 *	. AND Columns:
 *		. Each AND column ANDs all 15 input signals and all 15
 *		  complemented input signals.
 *		. Any signal can be ignored. To ignore a signal, both the
 *		  signal and it's complement must be set.
 *	        . An AND column will be true (1) when all it's selected input
 *		  signals are 1.
 *
 *     . OR rows:
 *	        . Each OR row must be configured to select one of the AND
 *		  columns.
 *		. The OR row that is true (it's configured AND column is true)
 *		  determines the new fsm state. If OR row 4 is true, the fsm
 *		  transitions to state 4.
 *		. The OR rows are prioritized with row 0 having the highest
 *		  priority and row 15 the lowest. This means if both row 3 and
 *		  row 4 are true, the fsm will transition to state 3.
 *
 * Data fifos:
 *      . The 72-bits of potential capture data is split into two 36-bit halves.
 *	. Capture of each half can be controlled by either a single FSM or by a
 *	  combination of both FSM outputs.
 *	. Captured data is in the form of 38-bit-wide capture entries.
 */


#ifndef __CVMX_OCLA_H__
#define __CVMX_OCLA_H__


#define MAX_70XX_OCLA_COMPLEXES	1
#define MAX_73XX_OCLA_COMPLEXES	3
#define MAX_75XX_OCLA_COMPLEXES	3
#define MAX_78XX_OCLA_COMPLEXES	5
#define MAX_COMPLEXES		MAX_78XX_OCLA_COMPLEXES

/* These are the ocla hardware revisions and the chips that use them */
#define IS_OCLA_REV1		(OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) ||     \
				 OCTEON_IS_MODEL(OCTEON_CN76XX) ||	       \
				 OCTEON_IS_MODEL(OCTEON_CN70XX))

#define IS_OCLA_REV2		(OCTEON_IS_MODEL(OCTEON_CN78XX_PASS2_X) ||     \
				 OCTEON_IS_MODEL(OCTEON_CN73XX) ||	       \
				 OCTEON_IS_MODEL(OCTEON_CNF75XX))


/**
 * Supported DTX blocks.
 */
typedef enum {
	AGL,
	ASE,
	BBX1I,
	BBX2I,
	BBX3I,
	BCH,
	BGX,
	BTS,
	CIU,
	DENC,
	DFA,
	DLFE,
	DPI,
	FDEQ,
	FPA,
	GMX,
	GSER,
	HNA,
	ILA,
	ILK,
	IOBN,
	IOBP,
	IOB,
	IPD,
	KEY,
	L2C_CBC,
	L2C_MCI,
	L2C_TAD,
	LAP,
	LBK,
	LMC,
	MDB,
	MHBW,
	MIO,
	OCX_LNK,
	OCX_OLE,
	OCX_TOP,
	OSM,
	PCS,
	PEM,
	PIP,
	PKI_PBE,
	PKI_PFE,
	PKI_PIX,
	PKO,
	PNB,
	PNBD,
	POW,
	PRCH,
	PSM,
	RAD,
	RDEC,
	RFIF,
	RMAP,
	RNM,
	RST,
	SATA,
	SLI,
	SPEM,
	SRIO,
	SSO,
	TDEC,
	TIM,
	ULFE,
	USBH,
	USBDRD,
	VDEC,
	WPSE,
	WRCE,
	WRDE,
	WRSE,
	WTXE,
	XCV,
	XSX,
	ZIP,
	INVALID_BLOCK_ID = -1
} cvmx_dtx_id_t;

/**
 *  DTX definition.
 *
 *  half:	36-bit debug bus half used by this DTX.
 *  id:		ID of DTX block to configure.
 *  inst:	DTX block instance to configure.
 *  sel:	One of the hardware hardcoded values to select which signals
 *		will be put on the debug bus.
 *  ena:	36-bit mask to enable the bits to drive the debug bus.
 */
typedef struct {
	uint			half;
	cvmx_dtx_id_t		id;
	uint			inst;
	uint64_t		sel;
	uint64_t		ena;
} cvmx_dtx_def_t;

/**
 * Capture request.
 *
 *  node:			Node to capture data on.
 *  ix:				Ocla complex index.
 */
struct cap_req {
	uint			node;
	uint			ix;
};

/**
 * Capture ddr buffer request. The kernel must fulfill the request as a
 * physical address is needed by ocla.
 *
 *  node:			Node to capture data on.
 *  ix:				Ocla complex index.
 *  size:			Size of ddr buffer.
 *  pbuf:			Pointer to physicall address of ddr buffer.
 */
struct ddr_buf_req {
	uint			node;
	uint			ix;
	uint			size;
	uint64_t		pbuf;
};

/* Ioctl commands */
#define OCLA_CAP_REQ		_IOW('o', 0x01, struct cap_req)
#define OCLA_DDR_BUF_REQ	_IOWR('o', 0x02, struct ddr_buf_req)

/**
 * Selects what causes data to be captured.
 *
 *  NO_CAP		No data is captured.
 *  FSM0_CAP		Capture when fsm0 requests it.
 *  FSM1_CAP		Capture when fsm1 requests it.
 *  FSM0_OR_FSM1	Capture when fsm0 or fsm1 request it.
 *  FSM0_AND_FSM1	Capture when fsm0 and fsm1 request it.
 *  CAP			Always capture.
 */
typedef enum {
	NO_CAP = 0,
	FSM0_CAP = 2,
	FSM1_CAP = 4,
	FSM0_OR_FSM1_REV1 = 6,
	FSM0_AND_FSM1 = 8,
	FSM0_OR_FSM1_REV2 = 0xe,
	CAP = 0xf
} cvmx_cap_sel_t;

/**
 * Describe a fsm input.
 *
 *  msk			Bit mask.
 *  val			Bit values
 */
typedef struct {
	uint8_t		msk;
	uint8_t		val;
} cvmx_fsm_input_t;

/**
 * Bit mask of actions to take by fsm states.
 *
 *  NO_ACTION		Do nothing.
 *  ACTION_CAP		Capture.
 *  ACTION_INT		Generate an interrupt.
 */
typedef enum {
	NO_ACTION 	= 0,
	ACTION_CAP	= 1,
	ACTION_INT	= 2
} cvmx_fsm_action_t;

/**
 * Disable all DTXs on all nodes.
 * 
 * @return Returned value. Zero on success, -1 otherwise.
 */
extern int cvmx_dtx_reset(void);

/**
 * Setup the Debug Transmitters (DTX) hardware.
 *
 * @param node		Node to enable dtx on.
 * @param dtx		Pointer to DTX configuration to enable.
 * 
 * @return Returned value. Zero on success, -1 otherwise.
 */
extern int cvmx_dtx_enable(int node, cvmx_dtx_def_t *dtx);

/**
 * Disable a DTX.
 *
 * @param node		Node to disable dtx on.
 * @param dtx		Pointer to DTX to disable.
 * 
 * @return Returned value. Zero on success, -1 otherwise.
 */
extern int cvmx_dtx_disable(int node, cvmx_dtx_def_t *dtx);

/**
 * Reset the OCLA hardware.
 *
 * @param node		Node ocla complex is on.
 * @param ix		OCLA complex to reset.
 * 
 * @return Returned value. Zero on success, -1 otherwise.
 */
extern int cvmx_ocla_reset(int node, int ix);

/**
 * Clear the OCLA interrupts.
 *
 * @param node		Node ocla complex is on.
 * @param ix		OCLA complex whose interrupts are to be cleared.
 * 
 * @return Returned value. Zero on success, -1 otherwise.
 */
extern int cvmx_ocla_clear_interrupts(int node, int ix);

/**
 * Disable the OCLA hardware.
 *
 * @param node		Node ocla complex is on.
 * @param ix		OCLA complex to disable.
 * 
 * @return Returned value. Zero on success, -1 otherwise.
 */
extern int cvmx_ocla_disable(int node, int ix);

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
extern int cvmx_ocla_init(int node, int ix, uint64_t buf, uint buf_size,
			  int num_entries, cvmx_cap_sel_t lo_cap,
			  cvmx_cap_sel_t hi_cap);

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
extern int cvmx_matcher_init(int node, int ix, int mat, uint64_t lo_mask,
			     uint64_t lo_val, uint64_t hi_mask,
			     uint64_t hi_val);

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
extern int cvmx_fsm_init(int node, int ix, int fsm, cvmx_fsm_input_t *mcd,
			 cvmx_fsm_input_t *matcher, cvmx_fsm_input_t *fsm0_st,
			 cvmx_fsm_input_t *fsm1_st, uint new_st);

/**
 * Enable a fsm.
 *
 * @param node		Node fsm is on.
 * @param ix		OCLA complex fsm is on.
 * @param fsm		Fsm to enable.
 * 
 * @return Returned value. Zero on success, -1 otherwise.
 */
extern int cvmx_fsm_enable(int node, int ix, int fsm);

/**
 * Set the action to take for a fsm state.
 *
 * @param node		Node fsm is on.
 * @param ix		OCLA complex fsm is on.
 * @param fsm		Fsm to initialize.
 * @param state		State to assign action to.
 * @param action	Action for the fsm state.
 * 
 * @return Returned value. Zero on success, -1 otherwise.
 */
extern int cvmx_fsm_state_set(int node, int ix, int fsm, uint state,
			      cvmx_fsm_action_t action);

/**
 * Read a packet from the fifo.
 *
 * @param node		Node ocla complex is on.
 * @param ix		OCLA complex to read from.
 * @param data		Updated with packet read.
 * 
 * @return Returned value. Zero if packet is valid, -1 otherwise.
 */
extern int cvmx_ocla_get_packet(int node, int ix, uint64_t *data);

/**
 * Check if the ocla fifo is full.
 *
 * @param node		Node ocla complex is on.
 * @param ix		OCLA complex whose fifo is to be checked.
 * 
 * @return Returned value. Zero if fifo is not full, 1 if it's full.
 */
extern int cvmx_is_fifo_full(int node, int ix);

#endif /* __CVMX_OCLA_H__ */
