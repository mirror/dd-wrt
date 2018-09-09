#ifndef __CVMX_ADMA_H__
#define __CVMX_ADMA_H__

#ifdef	__cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

/*
 * PHY memory ranges
 */
#define CVMX_BBP_PHY_SMEM_SIZE	(438 * 1024)
#define CVMX_BBP_PHY_DMEM_SIZE	(384 * 1024)
#define CVMX_BBP_PHY_IMEM_SIZE	(256 * 1024)

#define CVMX_BBP_PHY_SMEM0_BASE	0x900000
#define CVMX_BBP_PHY_SMEM0_SIZE	CVMX_BBP_PHY_SMEM_SIZE
#define CVMX_BBP_PHY_DMEM0_BASE	0x9A0000
#define CVMX_BBP_PHY_DMEM0_SIZE	CVMX_BBP_PHY_DMEM_SIZE
#define CVMX_BBP_PHY_IMEM0_BASE	0x100000
#define CVMX_BBP_PHY_IMEM0_SIZE	CVMX_BBP_PHY_IMEM_SIZE

#define CVMX_BBP_PHY_SMEM1_BASE	0xA00000
#define CVMX_BBP_PHY_SMEM1_SIZE	CVMX_BBP_PHY_SMEM_SIZE
#define CVMX_BBP_PHY_DMEM1_BASE	0xAA0000
#define CVMX_BBP_PHY_DMEM1_SIZE	CVMX_BBP_PHY_DMEM_SIZE
#define CVMX_BBP_PHY_IMEM1_BASE	0x200000
#define CVMX_BBP_PHY_IMEM1_SIZE	CVMX_BBP_PHY_IMEM_SIZE

#define CVMX_BBP_PHY_SMEM2_BASE	0xB00000
#define CVMX_BBP_PHY_SMEM2_SIZE	CVMX_BBP_PHY_SMEM_SIZE
#define CVMX_BBP_PHY_DMEM2_BASE	0xBA0000
#define CVMX_BBP_PHY_DMEM2_SIZE	CVMX_BBP_PHY_DMEM_SIZE
#define CVMX_BBP_PHY_IMEM2_BASE	0x300000
#define CVMX_BBP_PHY_IMEM2_SIZE	CVMX_BBP_PHY_IMEM_SIZE

/*
 * flags for cvmx_bbp_adma_init()
 */
#define CVMX_ADMA_INIT_DEFAULT		0x0
#define CVMX_ADMA_INIT_IMEM_PARITY_ENB	0x1

/*
 * AXI DMA channels
 */
#define CVMX_ADMA_CHAN_WR1	0
#define CVMX_ADMA_CHAN_RD1	1
#define CVMX_ADMA_CHAN_WR2	2
#define CVMX_ADMA_CHAN_RD2	3
#define CVMX_ADMA_CHAN_WR3	4
#define CVMX_ADMA_CHAN_RD3	5
#define CVMX_ADMA_CHAN_WR4	6
#define CVMX_ADMA_CHAN_RD4	7

/*
 * AXI bus max_burst
 * 0 for 8 doublewords and 1 for 16.
 */
#define CVMX_ADMA_MAX_BURST8	0
#define CVMX_ADMA_MAX_BURST16	1

/*
 * Return values for cvmx_adma_io()
 */
#define CVMX_ADMA_IO_SUCCESS	 0
#define CVMX_ADMA_IO_TIMEOUT	-1	/* timeout for polling */
#define CVMX_ADMA_IO_BUSY   	-2	/* ADMA channel busy. */
#define CVMX_ADMA_IO_UNAVAIL	-3	/* This could be because of
					 * - invalid parameter(s),
					 * - invalid parameter(s) that lead
					 *   to operation out of range, or
					 * - DMA from I/DMEM to host
					 *   not allowed.
					 * See __cvmx_validate_adma_params()
					 * for details.
					 */
#define CVMX_ADMA_IO_AXI_ERR	-4	/* AXI error. */

/*
 * Perform an AXI DMA operation.
 *
 * @param buffer is the 64-bit address of the buffer in the DMA host's RAM.
 * @param phy_addr is the 32-bit address of the HAB Memory Manager(HMM).
 * @param num_bytes is the number of bytes to transfer.
 * @param channel identifies the DMA channel (CVMX_ADMA_CHAN_XXX) for the
 *     transfer.
 * @param max_burst is the maximum burst length attempted on the AXI bus,
 *     CVMX_ADMA_MAX_BURST8 or CVMX_ADMA_MAX_BURST16.
 * @param use_word_xfer_mode with a non-zero value indicates that the
 *     ``word transfer mode'' should be used for the transaction.
 * @param timeout indicates the amount of polling to be done to check for
 *     the ADMA results.
 *     - timeout == 0 means immediately return after initiating the ADMA
 *       transaction.
 *     - timeout > 0 is the maximum number of milliseconds to poll.
 *     - timeout < 0 means the function should poll indefinitely for the
 *       completion of the transaction.
 *
 * @return 0 for success and a negative value for failure.
 */
extern int cvmx_adma_io(void *buffer, uint32_t phy_addr, int num_bytes, int channel, int max_burst, int use_word_xfer_mode, int timeout);

/*
 * A wrapper over cvmx_adma_io() to transfer data from host to the
 * Endor PHY
 */
static inline int cvmx_adma_host2bbp(void *buffer, uint32_t phy_addr, int num_bytes, int max_burst, int use_word_xfer_mode, int timeout)
{
	int tile, channel;

	/* derive the read channel from phy_addr */
	tile = phy_addr >> 20;
	tile = (tile > 8) ? tile - 9 : tile - 1;
	channel = tile * 2 + 1;

	return cvmx_adma_io(buffer, channel, phy_addr, num_bytes, max_burst, use_word_xfer_mode, timeout);
}

/*
 * A wrapper over cvmx_adma_io() to transfer data the Endor PHY to
 * host
 */
static inline int cvmx_adma_bbp2host(void *buffer, uint32_t phy_addr, int num_bytes, int max_burst, int use_word_xfer_mode, int timeout)
{
	int tile, channel;

	/* derive the write channel from phy_addr */
	tile = phy_addr >> 20;
	tile = (tile > 8) ? tile - 9 : tile - 1;
	channel = tile * 2;

	return cvmx_adma_io(buffer, channel, phy_addr, num_bytes, max_burst, use_word_xfer_mode, timeout);
}

/*
 * Pull the DSPS into reset, enable the clocks, set stall state.
 *
 * @param tile is 0, 1, or 2 for tile RX0, RX1, and TX, respectively.
 */
extern void cvmx_bbp_reset_dsps(int tile);

/*
 * Release the DSPs from reset,.
 *
 * @param tile is 0, 1, or 2 for tile RX0, RX1, and TX, respectively.
 */
extern void cvmx_bbp_release_dsps(int tile);

/*
 * Set dsp stall state.
 *
 * @param tile is 0, 1, or 2 for tile RX0, RX1, and TX, respectively.
 * @param dsp is 0 for dsp0, 1 for dsp1, and 2 for both.
 */
extern void cvmx_bbp_stall_dsps(int tile, int dsp);

/*
 * Clear dsp stall state
 *
 * @param tile is 0, 1, or 2 for tile RX0, RX1, and TX
 * @param dsp is 0 for dsp0, 1 for dsp1, and 2 for both.
 */
extern void cvmx_bbp_unstall_dsps(int tile, int dsp);

/*
 * Intialize the BBP PHY and the ADMA block.
 *
 * @param init_flags is a bit vector that holds init options, where the
 *     default is CVMX_ADMA_INIT_DEFAULT	
 *
 * @return 0 for success and a negative value for error.
 *
 * Note if the BBP PHY is not in the reset state, this funtion does
 * not force it.
 */
extern int cvmx_bbp_adma_init(uint64_t init_flags);
#endif

#ifdef	__cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif
