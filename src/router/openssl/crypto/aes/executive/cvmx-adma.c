#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-bootmem.h>
#include <asm/octeon/cvmx-bbp-defs.h>
#include <asm/octeon/cvmx-eoi-defs.h>
#include <asm/octeon/cvmx-adma.h>
#else
#include "cvmx.h"
#include "cvmx-bootmem.h"
#include "cvmx-bbp-defs.h"
#include "cvmx-eoi-defs.h"
#include "cvmx-adma.h"
#endif
/*
 * Endor Tiles
 */
#define RX0_TILE	0
#define RX1_TILE	1
#define TX_TILE		2

/*
 * PHY memory types
 */
#define BBP_PHY_MTYPE_SMEM	1
#define BBP_PHY_MTYPE_DMEM	2
#define BBP_PHY_MTYPE_IMEM	3

/* #define DEBUG_ADMA */
#ifdef min
#undef min
#endif
#define min(a, b) ((a) < (b)) ? (a) : (b)

#define ENDOR_CSR_MODIFY(name, csr, lower_case_csr, code_block) do {	\
        lower_case_csr##_t name = {.u32 = cvmx_read64_uint32(csr)};	\
        code_block;							\
        cvmx_write64_uint32(csr, name.u32);				\
    } while (0)

/*
 * misc
 */
#define ADMA_BOOTMEM_NAME "endor_adma_buffer"
#define ADMA_BOOTMEM_SIZE (512 * 1024)	/* Make sure this is larger than
					   the biggest PHY memory. */
#define ADMA_IS_CHAN_RD(channel)	(channel % 2 == 1)
#define IS_BUSY(channel)						\
    ((1ULL << channel) & cvmx_read64_uint32(CVMX_BBP_ADMA_MODULE_STATUS))

CVMX_SHARED void *cvmx_adma_buf;

/*
 * Validate the combination of PHY parameters for ADMA transfer.
 *
 * @param phy_addr is the 32-bit address of the HAB Memory
 *     Manager(HMM).
 * @param num_bytes is the number of bytes to transfer.
 * @param channel identifies the DMA channel (CVMX_ADMA_CHAN_XXX) for
 *     the transfer.
 *
 * @return > 0 for valid memory types (CVMX_BBP_PHY_MTYPE_XMEM) and
 *     -1 for invalid channel,
 *     -2 for invalid address range, and
 *     -3 for invalid num_bytes.
 *
 * Note: See 3.2 DMA and non-DMA Channels in [endor_adma_ug_ap3].
 */
static int __validate_adma_params(uint32_t phy_addr, int num_bytes,
    int channel)
{
    int ret;

    if (!(num_bytes > 0))
        return -3;

    ret = 0;

    switch (channel)
    {
#define MEM_OORANGE(mtype, idx)					\
    (phy_addr < CVMX_BBP_PHY_##mtype##MEM##idx##_BASE ||	\
	(phy_addr + num_bytes) >				\
	    (CVMX_BBP_PHY_##mtype##MEM##idx##_BASE  +		\
		CVMX_BBP_PHY_##mtype##MEM##idx##_SIZE))
#define SMEM_OORANGE(idx)	MEM_OORANGE(S, idx)
#define DMEM_OORANGE(idx)	MEM_OORANGE(D, idx)
#define IMEM_OORANGE(idx)	MEM_OORANGE(I, idx)

#define MEM_VALID(mtype, idx)					\
    (phy_addr >= CVMX_BBP_PHY_##mtype##MEM##idx##_BASE &&	\
	(phy_addr + num_bytes) <=				\
	    (CVMX_BBP_PHY_##mtype##MEM##idx##_BASE  +		\
		CVMX_BBP_PHY_##mtype##MEM##idx##_SIZE))
#define SMEM_VALID(idx)		MEM_VALID(S, idx)
#define DMEM_VALID(idx)		MEM_VALID(D, idx)
#define IMEM_VALID(idx)		MEM_VALID(I, idx)

    case CVMX_ADMA_CHAN_WR1:
        ret = (SMEM_OORANGE(0)) ? -2 : BBP_PHY_MTYPE_SMEM;
	break;
    case CVMX_ADMA_CHAN_WR2:
    case CVMX_ADMA_CHAN_WR4:
        ret = (SMEM_OORANGE(1)) ? -2 : BBP_PHY_MTYPE_SMEM;
        break;
    case CVMX_ADMA_CHAN_WR3:
        ret = (SMEM_OORANGE(2)) ? -2 : BBP_PHY_MTYPE_SMEM;
        break;
    case CVMX_ADMA_CHAN_RD1:
        ret = (SMEM_VALID(0)) ? BBP_PHY_MTYPE_SMEM : 
	      (DMEM_VALID(0)) ? BBP_PHY_MTYPE_DMEM : 
	      (IMEM_VALID(0)) ? BBP_PHY_MTYPE_IMEM : -2;
        break;
    case CVMX_ADMA_CHAN_RD2:
        ret = (SMEM_VALID(1)) ? BBP_PHY_MTYPE_SMEM : 
	      (DMEM_VALID(1)) ? BBP_PHY_MTYPE_DMEM : 
	      (IMEM_VALID(1)) ? BBP_PHY_MTYPE_IMEM : -2;
        break;
    case CVMX_ADMA_CHAN_RD3:
        ret = (SMEM_VALID(2)) ? BBP_PHY_MTYPE_SMEM : 
	      (DMEM_VALID(2)) ? BBP_PHY_MTYPE_DMEM : 
	      (IMEM_VALID(2)) ? BBP_PHY_MTYPE_IMEM : -2;
        break;
    case CVMX_ADMA_CHAN_RD4:
        ret = (SMEM_VALID(0)) ? BBP_PHY_MTYPE_SMEM : 
	      (DMEM_VALID(0)) ? BBP_PHY_MTYPE_DMEM : -2;
        break;
    default:
        ret = -1;
        break;
    }

    return ret;
}

/*
 * Config the ADMA side.
 *
 * @param channel
 * @param num_bytes
 * @param phy_addr is the address in the tile memory
 * @param hst_addr is the address in the bounce buffer
 * @return 0
 */
static int __start_adma(int channel, int num_bytes, uint32_t phy_addr,
    uint64_t hst_addr)
{
    cvmx_bbp_adma_dmax_cfg_t dma_cfg;
    cvmx_bbp_adma_dmax_size_t dma_size;
    cvmx_bbp_adma_dmax_addr_lo_t dma_addr_lo;
    cvmx_bbp_adma_dmax_addr_hi_t dma_addr_hi;
    cvmx_bbp_adma_intr_enb_t intr_enb;
    cvmx_bbp_adma_dmadone_intr_t dmadone_intr;
    /* cvmx_bbp_adma_axierr_intr_t axierr_intr; */

    /* Clear the interrupt for the channel. */
    dmadone_intr.s.dma_ch_done = 1 << channel;
    cvmx_write64_uint32(CVMX_BBP_ADMA_DMADONE_INTR, dmadone_intr.u32);

    /* Enable the interrupt for the channel. */
    /* intr_enb.u32 = cvmx_read64_uint32(CVMX_BBP_ADMA_INTR_ENB); */
    /* intr_enb.s.dmadone_intr_enb = intr_enb.s.dmadone_intr_enb | 1 << channel; */
    intr_enb.s.dmadone_intr_enb = 1 << channel;
    intr_enb.s.axierr_intr_enb = 1; /* Error interrupt as well */
    cvmx_write64_uint32(CVMX_BBP_ADMA_INTR_ENB, intr_enb.u32);

    /* Set up the adma size */
    dma_size.s.dma_size = num_bytes;
    cvmx_write64_uint32(CVMX_BBP_ADMA_DMAX_SIZE(channel), dma_size.u32);

    /* Set up the high and low adma addresses. */
    dma_addr_lo.s.lo_addr = hst_addr & 0xffffffff;
    cvmx_write64_uint32(CVMX_BBP_ADMA_DMAX_ADDR_LO(channel), dma_addr_lo.u32);
    dma_addr_hi.s.hi_addr = (hst_addr >> 32) & 0xffffffff;
    cvmx_write64_uint32(CVMX_BBP_ADMA_DMAX_ADDR_HI(channel), dma_addr_hi.u32);

    /* Enable the transfer. */
    dma_cfg.u32 = 0;
    dma_cfg.s.enable = 1;
    dma_cfg.s.hmm_ofs = phy_addr & 0x3;
#ifdef DEBUG_ADMA
    cvmx_dprintf("dma_size.s.dma_size = %d hmm_ofs = %d\n",
        dma_size.s.dma_size, dma_cfg.s.hmm_ofs);
#endif
    cvmx_write64_uint32(CVMX_BBP_ADMA_DMAX_CFG(channel), dma_cfg.u32);

    return 0;
}

static int __ceil4(int n)
{
    if ((n & 0x3) != 0)
        return n / 4 + 1;
    else 
        return  n / 4;
}

static int __check_axi_error(int channel)
{
    cvmx_bbp_adma_axierr_intr_t axierr_intr;

    axierr_intr.u32 = cvmx_read64_uint32(
        CVMX_BBP_ADMA_AXIERR_INTR);
    if (axierr_intr.u32 & (1 << (channel + 16)))
    {
        cvmx_dprintf("timeout axierr_intr = 0x%x (channel = %d)\n",
            axierr_intr.u32, channel);
        return 1;
    }
    
    return 0;
}

static int __reset_adma(int channel)
{
    cvmx_bbp_adma_dma_reset_t dma_reset;

    dma_reset.u32 = (1 << channel);
    cvmx_write64_uint32(CVMX_BBP_ADMA_DMA_RESET, dma_reset.u32);
    
    /* Wait for reset to self clear */
    while (1)
    {
        dma_reset.u32 = cvmx_read64_uint32(
            CVMX_BBP_ADMA_DMA_RESET);
        if ((dma_reset.u32 & (1 << channel)) == 0)
            break;
    }

    return 0;
}

/*
 * Is the transfer done?
 *
 * @param channel
 * @param mtype
 * @return 1 for yes and 0 for no.
 *
 * This is intended to be called right after the xfer is kicked off to
 * check if it is done.
 */
static int __xfer_done(int channel, int mtype)
{
#if 0
    return IS_BUSY(channel) ? 0 : 1;
#endif

#if 1
    if (ADMA_IS_CHAN_RD(channel))
    {
        /* check the hmm side */
#define CHECK_HMM(HMM_PREFIX, hmm_prefix)				\
    do									\
    {									\
	cvmx_bbp_##hmm_prefix##_dma_wr_intr_rstatus_t r;		\
	r.u32 = cvmx_read64_uint32(					\
	    CVMX_BBP_##HMM_PREFIX##_DMA_WR_INTR_RSTATUS);		\
	return (r.s.xfer_raw_done_int) ? 1 : 0;				\
    } while (0)

#define CHECK_TILE(TILE_PREFIX, tile_prefix, mtype)			\
    do									\
    {									\
	if (mtype == BBP_PHY_MTYPE_IMEM)				\
	    CHECK_HMM(TILE_PREFIX##_INSTR, tile_prefix##_instr);	\
        else								\
	    CHECK_HMM(TILE_PREFIX##_EXT, tile_prefix##_ext);		\
    } while (0)

#ifdef DEBUG_ADMA
	cvmx_dprintf("%s: channel%d, mtype%d\n", __func__, channel, mtype);
#endif
	switch (channel)
	{
	case CVMX_ADMA_CHAN_RD1:
	    CHECK_TILE(RX0, rx0, mtype);
	    break;
	case CVMX_ADMA_CHAN_RD2:
	    CHECK_TILE(RX1, rx1, mtype);
	    break;
	case CVMX_ADMA_CHAN_RD3:
	    CHECK_TILE(TX, tx, mtype);
	    break;
	case CVMX_ADMA_CHAN_RD4:
	    CHECK_HMM(RX1_HARQ_DMA, rx1_harq_dma);
	    break;
	default:
	    return 0;
	}
    }
    else
    {
        /* check the adma interrupt */
        cvmx_bbp_adma_dmadone_intr_t r;
	r.u32 = cvmx_read64_uint32(CVMX_BBP_ADMA_DMADONE_INTR);
#ifdef DEBUG_ADMA
	cvmx_dprintf("%s: dmadone_intr 0x%x channel %d\n",
	    __func__, r.u32, channel);
#endif
	return (r.u32 & (1ULL << channel)) ? 1 : 0;
    }

    return 0;
#endif
}

int cvmx_adma_io(void *buffer, uint32_t phy_addr, int num_bytes, int channel,
    int max_burst, int use_word_xfer_mode, int timeout)
{
    int mtype, nbytes, leftover, offset, paddr;

    if (max_burst != CVMX_ADMA_MAX_BURST8 &&
        max_burst != CVMX_ADMA_MAX_BURST16)
	return CVMX_ADMA_IO_UNAVAIL;

    mtype = __validate_adma_params(phy_addr, num_bytes, channel);
    if (mtype < 0)
        return CVMX_ADMA_IO_UNAVAIL;

    if (IS_BUSY(channel))
        return CVMX_ADMA_IO_BUSY;
    
#define START_HMM(HMM_PREFIX, hmm_prefix, num_bytes, phy_addr)		\
    do {								\
        cvmx_bbp_##hmm_prefix##_intr_clear_t	intr_clear;		\
        cvmx_bbp_##hmm_prefix##_mode_t		mode;			\
        cvmx_bbp_##hmm_prefix##_start_addr0_t	start_addr0;		\
        cvmx_bbp_##hmm_prefix##_xfer_start_t 	xfer_start;		\
        cvmx_bbp_##hmm_prefix##_xfer_mode_count_t xfer_mode_count;	\
									\
	/* Clear pending HMM interrupts. */				\
	intr_clear.u32 = 0;						\
        intr_clear.s.xfer_done_int_clr = 1;				\
        intr_clear.s.xfer_q_empty_int_clr = 1;				\
	cvmx_write64_uint32(CVMX_BBP_##HMM_PREFIX##_INTR_CLEAR,		\
	    intr_clear.u32);						\
									\
	/* Clear the mode. */						\
	mode.u32 = 0;							\
	cvmx_write64_uint32(CVMX_BBP_##HMM_PREFIX##_MODE, mode.u32);	\
									\
        /* Determine the address and size. */				\
	start_addr0.u32 = 0;						\
	xfer_mode_count.u32 = 0;					\
									\
	start_addr0.s.addr = phy_addr;					\
	xfer_mode_count.s.xfer_count = __ceil4(num_bytes + 		\
	    (phy_addr & 0x3));						\
	xfer_mode_count.s.xfer_done_intr = 1;				\
									\
	cvmx_write64_uint32(CVMX_BBP_##HMM_PREFIX##_START_ADDR0,	\
	    start_addr0.u32);						\
	cvmx_write64_uint32(CVMX_BBP_##HMM_PREFIX##_XFER_MODE_COUNT,	\
	    xfer_mode_count.u32);					\
									\
	/* Start the transfer. */					\
	xfer_start.u32 = 0;						\
	xfer_start.s.xfer_start = 1;					\
	cvmx_write64_uint32(CVMX_BBP_##HMM_PREFIX##_XFER_START,		\
	    xfer_start.u32);						\
    } while (0)

    if (ADMA_IS_CHAN_RD(channel))
    {
        /* copy from user's buffer to the bounce buffer */
        memcpy(cvmx_adma_buf, buffer, num_bytes);
        CVMX_SYNCW;
    }

    leftover = num_bytes;
    offset = 0;

#define MAX_XFER_SIZE	0x20000
    do {
        nbytes = min(leftover, MAX_XFER_SIZE);
        paddr = phy_addr + offset;

        /*
         * Two steps to kick off ADMA are
         * 1. configure the AXI DMA module, and
         * 2. configure the HMM. 
         */
        __start_adma(channel, nbytes, paddr,
	    CAST64(cvmx_adma_buf + offset));

#define START_TILE(TILE_PREFIX, tile_prefix, mtype)			\
    do									\
    {									\
        if (mtype == BBP_PHY_MTYPE_IMEM)				\
            START_HMM(TILE_PREFIX##_INSTR_DMA_WR,			\
	        tile_prefix##_instr_dma_wr, nbytes, paddr);		\
	else								\
            START_HMM(TILE_PREFIX##_EXT_DMA_WR,				\
	        tile_prefix##_ext_dma_wr, nbytes, paddr);		\
    } while (0)

        switch (channel)
        {
        case CVMX_ADMA_CHAN_WR1:
            START_HMM(RX0_EXT_DMA_RD, rx0_ext_dma_rd, nbytes, paddr);
            break;
        case CVMX_ADMA_CHAN_RD1:
	    START_TILE(RX0, rx0, mtype);
            break;
        case CVMX_ADMA_CHAN_WR2:
            START_HMM(RX1_EXT_DMA_RD, rx1_ext_dma_rd, nbytes, paddr);
            break;
        case CVMX_ADMA_CHAN_RD2:
	    START_TILE(RX1, rx1, mtype);
            break;
        case CVMX_ADMA_CHAN_WR3:
            START_HMM(TX_EXT_DMA_RD, tx_ext_dma_rd, nbytes, paddr);
            break;
        case CVMX_ADMA_CHAN_RD3:
	    START_TILE(TX, tx, mtype);
            break;
        case CVMX_ADMA_CHAN_WR4:
            START_HMM(RX1_HARQ_DMA_DMA_RD, rx1_harq_dma_dma_rd, nbytes,
	        paddr);
	    break;
        case CVMX_ADMA_CHAN_RD4:
            START_HMM(RX1_HARQ_DMA_DMA_WR, rx1_harq_dma_dma_wr, nbytes,
	        paddr);
            return 0;
        }

        /*
         * Poll
         */
        if (timeout < 0)
        {
            while (!__xfer_done(channel, mtype))
                cvmx_wait_usec(1000);
        }
        else if (timeout > 0)
        {
            while ((!__xfer_done(channel, mtype)) && timeout--)
                cvmx_wait_usec(1000);

            if (timeout == -1)
            {
                __reset_adma(channel);
                return CVMX_ADMA_IO_TIMEOUT;
            }
        }

        leftover -= MAX_XFER_SIZE;
        offset += MAX_XFER_SIZE;
    } while (leftover > 0);

    if (__check_axi_error(channel))
    {
        __reset_adma(channel);
	return CVMX_ADMA_IO_AXI_ERR;
    }
    
    /* copy from the bounce buffer to user's buffer */
    if (!ADMA_IS_CHAN_RD(channel))
        memcpy(buffer, cvmx_adma_buf, num_bytes);

#ifdef DEBUG_ADMA
    cvmx_dprintf("%s: returning success.\n", __func__);

    /* Fill the bounce buffer w/ 0xFF for error detection. */
    {
        memset(cvmx_adma_buf, 0xFF, ADMA_BOOTMEM_SIZE);
        CVMX_SYNCW;
    }
#endif

    return CVMX_ADMA_IO_SUCCESS;
}

static uint64_t cvmx_endor_bist(void)
{
#define phy_addr_prefix		0x80010F0000000000ULL
    cvmx_eoi_bist_ctl_sta_t bist_ctl_sta;
    cvmx_eoi_endor_bistr_ctl_sta_t endor_bistr_ctl_sta;
    cvmx_eoi_def_sta0_t def_sta0;
    cvmx_eoi_def_sta1_t def_sta1;
    cvmx_eoi_def_sta2_t def_sta2;

    /* EOI BIST */
    bist_ctl_sta.u64 = 0;
    bist_ctl_sta.s.clear_bist = 1;
    cvmx_write_csr(CVMX_EOI_BIST_CTL_STA, bist_ctl_sta.u64);
    CVMX_SYNCW;
    bist_ctl_sta.s.start_bist = 1;
    cvmx_write_csr(CVMX_EOI_BIST_CTL_STA, bist_ctl_sta.u64);

    do {
        bist_ctl_sta.u64 = cvmx_read_csr(CVMX_EOI_BIST_CTL_STA);
        if (OCTEON_IS_MODEL(OCTEON_CNF71XX_PASS1_X)) {
            break; /* CNF71XX_PASS1 start_bist always fails to clear */
        }
        if (bist_ctl_sta.s.start_bist == 0)
            break;
        cvmx_wait_usec(1000);
    } while (1);

    if (bist_ctl_sta.s.stdf)
        cvmx_dprintf( " BIST failure -- STDF\n" );
    if (bist_ctl_sta.s.ppaf)
        cvmx_dprintf( " BIST failure -- PPAF\n" );
    if (bist_ctl_sta.s.lddf)
        cvmx_dprintf( " BIST failure -- LDDF\n" );

    bist_ctl_sta.u64 = 0;
    cvmx_write_csr(CVMX_EOI_BIST_CTL_STA, bist_ctl_sta.u64);

    /* ENDOR BIST */
    endor_bistr_ctl_sta.u64 = 0;
    endor_bistr_ctl_sta.s.start_bist = 1;
    endor_bistr_ctl_sta.s.bisr_dir = 1;
    cvmx_write_csr(CVMX_EOI_ENDOR_BISTR_CTL_STA, endor_bistr_ctl_sta.u64);
    CVMX_SYNCW;

    do {
        endor_bistr_ctl_sta.u64= cvmx_read_csr(CVMX_EOI_ENDOR_BISTR_CTL_STA);
        if (endor_bistr_ctl_sta.s.bisr_done == 1)
            break;
        cvmx_wait_usec(1000);
    } while (1);

    /* 71XX PASS1 always fails BIST */
    if (!OCTEON_IS_MODEL(OCTEON_CNF71XX_PASS1_X)) {
        if (endor_bistr_ctl_sta.s.failed) {
            cvmx_dprintf( " ENDOR BIST failed!\n" );
        }
    }
    def_sta0.u64 = cvmx_read_csr(CVMX_EOI_DEF_STA0);
    def_sta1.u64 = cvmx_read_csr(CVMX_EOI_DEF_STA1);
    def_sta1.u64 = cvmx_read_csr(CVMX_EOI_DEF_STA2);
    if (def_sta0.u64 | def_sta1.u64 | def_sta2.u64) {
        cvmx_dprintf(" Endor Bist repairs:\n");
        if (def_sta0.s.rout0) cvmx_dprintf( "  ROUT0 = 0x%08x", (uint32_t)def_sta0.s.rout0);
        if (def_sta0.s.rout1) cvmx_dprintf( "  ROUT1 = 0x%08x", (uint32_t)def_sta0.s.rout1);
        if (def_sta0.s.rout2) cvmx_dprintf( "  ROUT2 = 0x%08x", (uint32_t)def_sta0.s.rout2);
        if (def_sta1.s.rout3) cvmx_dprintf( "  ROUT3 = 0x%08x", (uint32_t)def_sta1.s.rout3);
        if (def_sta1.s.rout4) cvmx_dprintf( "  ROUT4 = 0x%08x", (uint32_t)def_sta1.s.rout4);
        if (def_sta1.s.rout5) cvmx_dprintf( "  ROUT5 = 0x%08x", (uint32_t)def_sta1.s.rout5);
        if (def_sta2.s.rout6) cvmx_dprintf( "  ROUT6 = 0x%08x", (uint32_t)def_sta2.s.rout6);
        cvmx_dprintf( "  TOOMANYDEFECTS is %s\n", def_sta2.s.toomany? "true" : "false");
    }

    /* Don't print this if it is the MFG TEST app. */
    uint32_t rx0_bisr[8] ={0x0,0x4,0x10000000,0x0,0x0,0x0,0x0,0x0};
    uint32_t rx1_bisr[8] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    uint32_t tx_bisr[8] = {0x0, 0x10000, 0x18000000, 0x0, 0x0, 0x0, 0x0, 0x0};
    uint64_t bisr_error =0;
    uint32_t * addr;
    int i;

    if (OCTEON_IS_MODEL(OCTEON_CNF71XX_PASS1_X)) {
        addr = (uint32_t *)(phy_addr_prefix + 0x833860);
        for ( i = 0; i < 8; i++) {
            if (rx0_bisr[i] != *addr) {
                bisr_error++;
            }
            addr++;
        }
        addr = (uint32_t *)(phy_addr_prefix + 0x853860);
        for ( i = 0; i < 8; i++) {
            if (rx1_bisr[i] != *addr) {
                bisr_error++;
            }
            addr++;
        }
        addr = (uint32_t *)(phy_addr_prefix + 0x874460);
        for ( i = 0; i < 8; i++) {
            if (tx_bisr[i] != *addr) {
                bisr_error++;
            }
            addr++;
        }
    }

    if ( bisr_error ) {
        cvmx_dprintf("\n BISR error information is different than expected for CNF71XX_PASS1.\n\n");

        addr = (uint32_t *)(phy_addr_prefix + 0x833860);
        for ( i = 0; i < 8; i++) {
            cvmx_dprintf(" RX0 - bisr info. reg  at %p expected = 0x%08x actual = 0x%08x\n", addr, rx0_bisr[i], *addr);
            addr++;
        }
        addr = (uint32_t *)(phy_addr_prefix + 0x853860);
        for ( i = 0; i < 8; i++) {
            cvmx_dprintf(" RX1 - bisr info. reg  at %p expected = 0x%08x actual = 0x%08x\n", addr, rx1_bisr[i], *addr);
            addr++;
        }
        addr = (uint32_t *)(phy_addr_prefix + 0x874460);
        for ( i = 0; i < 8; i++) {
            cvmx_dprintf("  TX - bisr info. reg  at %p expected = 0x%08x actual = 0x%08x\n", addr, tx_bisr[i], *addr);
            addr++;
        }
    }
    return(bisr_error);
}

/* Global to indicate that the PPL/CLOCKS hae been initialized */
CVMX_SHARED uint32_t init_pll = 0; 

static void cvmx_endor_init_pll(uint64_t clkf, uint64_t dsp_ps,
    uint64_t hab_ps)
{
    uint64_t phy_pll_divisor[8] = {1,2,3,4,6,8,12,12}; 
    cvmx_eoi_endor_ctl_t eoi_endor_ctl;
    cvmx_eoi_ctl_sta_t eoi_ctl_sta;
    cvmx_eoi_endor_clk_ctl_t clk_ctl;

#if 0
    cvmx_dprintf("Init PHY Clocks to: PLL Freq. = %ldMhz (%ld), "
        "DSP Freq. = %ldMhz (%ld), "
	"HAB Freq. = %ldMhz (%ld)\n",
#endif
    cvmx_dprintf("Init PHY Clocks to: PLL Freq. = %uMhz (%u), "
        "DSP Freq. = %uMhz (%u), "
	"HAB Freq. = %uMhz (%u)\n",
        (unsigned int)(50*clkf), (unsigned int)clkf,
	(unsigned int)((50*clkf)/phy_pll_divisor[dsp_ps]),
	(unsigned int)dsp_ps,
	(unsigned int)((50*clkf)/phy_pll_divisor[hab_ps]),
	(unsigned int)hab_ps);

    if (init_pll) {
        /* If it's not in reset, then reset endor */
        eoi_endor_ctl.u64 = cvmx_read_csr(CVMX_EOI_ENDOR_CTL);
        if (eoi_endor_ctl.s.reset == 0) {
            eoi_endor_ctl.s.reset = 0x1;
            cvmx_write_csr(CVMX_EOI_ENDOR_CTL, eoi_endor_ctl.u64);
            cvmx_wait_usec(2000);
        }

        /* If EOI is not in reset, reset it */
        eoi_ctl_sta.u64 = cvmx_read_csr(CVMX_EOI_CTL_STA);
        if (eoi_ctl_sta.s.reset == 0) {
             eoi_ctl_sta.s.reset = 0x1;
             cvmx_write_csr(CVMX_EOI_CTL_STA, eoi_ctl_sta.u64);
             cvmx_wait_usec(2000);
        }

        /* Restore power-up defaults to the PLL and dividers */
        clk_ctl.u64 = 0x0;
        clk_ctl.s.clkf = 0x20;
        clk_ctl.s.reset_n = 0x0;
        clk_ctl.s.cpb = 0x0;
        clk_ctl.s.cps = 0x0;
        clk_ctl.s.diffamp = 0x0;
        clk_ctl.s.hab_ps_en = 0x5;
        clk_ctl.s.hab_div_reset = 0x1;
        clk_ctl.s.dsp_ps_en = 0x4;
        clk_ctl.s.dsp_div_reset = 0x1;
        clk_ctl.s.habclk_sel = 0x0;
        cvmx_write_csr(CVMX_EOI_ENDOR_CLK_CTL, clk_ctl.u64);
        cvmx_wait_usec(2000);
    }

    init_pll = 1;  /* Set flag to indicate the PLL and clocks are running */

    clk_ctl.u64 = cvmx_read_csr(CVMX_EOI_ENDOR_CLK_CTL);
    clk_ctl.s.clkf = clkf;
    cvmx_write_csr(CVMX_EOI_ENDOR_CLK_CTL, clk_ctl.u64);

    cvmx_wait_usec(100);

    clk_ctl.u64 = cvmx_read_csr(CVMX_EOI_ENDOR_CLK_CTL);
    clk_ctl.s.reset_n = 1;
    cvmx_write_csr(CVMX_EOI_ENDOR_CLK_CTL, clk_ctl.u64);

    clk_ctl.u64 = cvmx_read_csr(CVMX_EOI_ENDOR_CLK_CTL);
    cvmx_wait_usec(100);

    clk_ctl.s.dsp_ps_en = dsp_ps;
    clk_ctl.s.dsp_div_reset = 0x1;
    clk_ctl.s.hab_ps_en = hab_ps;
    clk_ctl.s.hab_div_reset = 0x1;
    cvmx_write_csr(CVMX_EOI_ENDOR_CLK_CTL, clk_ctl.u64);

    clk_ctl.u64 = cvmx_read_csr(CVMX_EOI_ENDOR_CLK_CTL);
    cvmx_wait_usec(100);

    clk_ctl.s.dsp_div_reset = 0x0;
    clk_ctl.s.hab_div_reset = 0x0;
    cvmx_write_csr(CVMX_EOI_ENDOR_CLK_CTL, clk_ctl.u64);
    cvmx_wait_usec(100);

    init_pll = 1;  /* Set flag to indicate the PLL and clocks are running */
}

static void cvmx_endor_init_endor_block(uint64_t init_flags)
{

    cvmx_eoi_ctl_sta_t eoi_ctl_sta;
    cvmx_eoi_int_ena_t eoi_int_ena;
    cvmx_eoi_int_sta_t eoi_int_sta;
    cvmx_eoi_endor_ctl_t eoi_endor_ctl;
    cvmx_eoi_throttle_ctl_t eoi_throttle_ctl;
    cvmx_eoi_ecc_ctl_t eoi_ecc_ctl;
    cvmx_bbp_rstclk_phy_config_t phy_config;

    /* If the PLL isn't configured, do it now */
    if (!init_pll) {
        /* 
         * PLL = 1.6 Ghz DSP = 533 Mhz, HAB = 200 Mhz
         * endor_init_pll takes the actual eoi_endor_clk_ctl reg. settings
         */
         cvmx_endor_init_pll(32, 2, 5);
    }

#if 0
    /* If it's not in reset, then reset endor */
    eoi_endor_ctl.u64 =  cvmx_read_csr(CVMX_EOI_ENDOR_CTL);
    if (eoi_endor_ctl.s.reset != 0x1) {
        eoi_endor_ctl.s.reset = 0x1;
        cvmx_write_csr(CVMX_EOI_ENDOR_CTL, eoi_endor_ctl.u64);
        cvmx_wait_usec(2000);
    }

    /* If EOI is not in reset, reset it */
    eoi_ctl_sta.u64 = cvmx_read_csr(CVMX_EOI_CTL_STA);
    if (eoi_ctl_sta.s.reset == 0) {
        eoi_ctl_sta.s.reset = 1;
        cvmx_write_csr(CVMX_EOI_CTL_STA, eoi_ctl_sta.u64);
        cvmx_wait_usec(2000);
    }
#endif

    /* Set enable and clear reset for EOI */
    eoi_ctl_sta.u64 = 0;
    eoi_ctl_sta.s.ppaf_wm = 8;
    eoi_ctl_sta.s.rwam = 0;
    eoi_ctl_sta.s.ena = 1;
    eoi_ctl_sta.s.reset = 0;
    cvmx_write_csr(CVMX_EOI_CTL_STA, eoi_ctl_sta.u64);

    /* Clear Endor block software reset  */
    eoi_endor_ctl.u64 =  cvmx_read_csr(CVMX_EOI_ENDOR_CTL);
    eoi_endor_ctl.s.reset = 0;
    cvmx_write_csr(CVMX_EOI_ENDOR_CTL, eoi_endor_ctl.u64);

    cvmx_wait_usec(2000);

    /* Run BIST/BISR */
    cvmx_endor_bist();

    eoi_endor_ctl.u64 =  cvmx_read_csr(CVMX_EOI_ENDOR_CTL);

    /* Disable all EOI interrupts */
    eoi_int_ena.u64 = 0;
    eoi_int_ena.s.rb_dbe = 0;              /* Double-bit error enable */
    eoi_int_ena.s.rb_sbe = 0;              /* Single-bit error eanble */
    cvmx_write_csr(CVMX_EOI_INT_ENA, eoi_int_ena.u64);

    /* Clear all interrupts */
    eoi_int_sta.u64 = 0;
    eoi_int_sta.s.rb_dbe = 0;              /* Double-bit error clear */
    eoi_int_sta.s.rb_sbe = 0;              /* Single-bit error clear */
    cvmx_write_csr(CVMX_EOI_INT_STA, eoi_int_sta.u64);

    /* Set to power-up defaults */
    eoi_throttle_ctl.u64 = 0;
    eoi_throttle_ctl.s.std = 31;
    eoi_throttle_ctl.s.stc = 0x3;
    eoi_throttle_ctl.s.ldc = 0x8;
    cvmx_write_csr(CVMX_EOI_THROTTLE_CTL, eoi_throttle_ctl.u64);

    /* Enable ECC for read buffer (default) */
    eoi_ecc_ctl.u64 = 0;
    eoi_ecc_ctl.s.rben  = 1;
    cvmx_write_csr(CVMX_EOI_ECC_CTL,  eoi_ecc_ctl.u64);

#if 0
    /*
     * Enable the DSP clocks, memory is clocked from the DSP clock
     * Probably not needed as all clocks are enabled when ENDOR is
     * taken out of reset.
     */
    bdk_endor_enable_dsp_clocks(RX0_TILE);
    bdk_endor_enable_dsp_clocks(RX1_TILE);
    bdk_endor_enable_dsp_clocks(TX_TILE);
#endif

#if 0
    /* Enable Init mode for DSP memories */
    phy_config.u32 = 0;
    phy_config.s.rx0imem_initenb = 1;
    phy_config.s.rx0smem_initenb = 1;
    phy_config.s.rx1imem_initenb = 1;
    phy_config.s.rx1smem_initenb = 1;
    phy_config.s.tximem_initenb = 1;
    phy_config.s.txsmem_initenb = 1;
    cvmx_write64_uint32(CVMX_BBP_RSTCLK_PHY_CONFIG, phy_config.u32);

    {
        cvmx_bbp_rx0_instr_dma_wr_mode_t wr_mode;
        cvmx_bbp_rx0_instr_dma_wr_intr_rstatus_t wr_intr_rstatus;
        cvmx_bbp_rx0_instr_dma_wr_intr_clear_t wr_intr_clear;
        cvmx_bbp_rx0_instr_dma_wr_xfer_start_t wr_xfer_start;
        cvmx_bbp_rx0_instr_dma_wr_xfer_mode_count_t wr_xfer_mode_count;
        cvmx_bbp_rx0_instr_dma_wr_start_addr0_t wr_start_addr0;

	/* Clear any pending interrupts */
	wr_intr_clear.u32 = 0;
        wr_intr_clear.s.xfer_done_int_clr = 1;
        wr_intr_clear.s.xfer_q_empty_int_clr = 1;
	cvmx_write64_uint32(CVMX_BBP_RX0_EXT_DMA_WR_INTR_CLEAR,
	    wr_intr_clear.u32 );

        /* Set mode to enable memory clear */
        wr_mode.u32 = 0;
        wr_mode.s.mem_clr_enb = 1;
        wr_mode.s.mem_clr_data_sel = 1;
	cvmx_write64_uint32(CVMX_BBP_RX0_EXT_DMA_WR_MODE, wr_mode.u32);

        /* Determine address and size */
        wr_start_addr0.u32 = 0;
        wr_start_addr0.s.addr = CVMX_ENDOR_PHY_SMEM0_BASE;

        wr_xfer_mode_count.u32 = 0;
        wr_xfer_mode_count.s.xfer_count =  (4 * 1024) - 1;  // 16kb
        wr_xfer_mode_count.s.xfer_done_intr = 1;
	cvmx_write64_uint32(CVMX_BBP_RX0_EXT_DMA_WR_START_ADDR0, wr_start_addr0.u32);
	cvmx_write64_uint32(CVMX_BBP_RX0_EXT_DMA_WR_XFER_MODE_COUNT, wr_xfer_mode_count.u32);
	cvmx_write64_uint32(CVMX_BBP_RX0_EXT_DMA_WR_MEMCLR_DATA, 0xdeadbeef);

        /* Start the transfer */
        wr_xfer_start.u32 = 0;
        wr_xfer_start.s.xfer_start = 1;
	cvmx_write64_uint32(CVMX_BBP_RX0_EXT_DMA_WR_XFER_START, wr_xfer_start.u32);

	cvmx_wait_usec(1000000);
    }

    cvmx_bbp_rx0_instr_dma_wr_mode_t wr_mode;
    cvmx_bbp_rx0_instr_dma_wr_intr_rstatus_t wr_intr_rstatus;
    cvmx_bbp_rx0_instr_dma_wr_intr_clear_t wr_intr_clear;
    cvmx_bbp_rx0_instr_dma_wr_xfer_start_t wr_xfer_start;
    cvmx_bbp_rx0_instr_dma_wr_xfer_mode_count_t wr_xfer_mode_count;
    cvmx_bbp_rx0_instr_dma_wr_start_addr0_t wr_start_addr0;

    for (tile_index = RX0_TILE; tile_index <= TX_TILE; tile_index++) {

        // Init imem first

        // Clear any pending interrupts
        wr_intr_clear.u = 0;
        wr_intr_clear.s.xfer_done_int_clr = 1;
        wr_intr_clear.s.xfer_q_empty_int_clr = 1;
        write_hmm_instr_dma_wr_intr_clear(tile_index, wr_intr_clear.u);
        cvmx_wait_usec(200);
        wr_intr_rstatus.u = read_hmm_instr_dma_wr_intr_rstatus(tile_index);

        // Set mode to enable memory clear
        wr_mode.u = 0;
        wr_mode.s.mem_clr_enb = 1;
        wr_mode.s.mem_clr_data_sel = 1;
        write_hmm_instr_dma_wr_mode(tile_index, wr_mode.u);

        // Determine address and size
        wr_start_addr0.u = 0;
	switch (tile_index) {
            case RX0_TILE:
                 wr_start_addr0.s.addr = ENDOR_RX0_UADDR_IMEM;
                 break;
            case RX1_TILE:
                 wr_start_addr0.s.addr = ENDOR_RX1_UADDR_IMEM;
                 break;
            case TX_TILE:
                 wr_start_addr0.s.addr = ENDOR_TX_UADDR_IMEM;
                 break;
        }        

        wr_xfer_mode_count.u = 0;
        wr_xfer_mode_count.s.xfer_count =  (4 * 1024) - 1;  // 16kb
        wr_xfer_mode_count.s.xfer_done_intr = 1;

        write_hmm_instr_dma_wr_start_addr0(tile_index, wr_start_addr0.u);
        write_hmm_instr_dma_wr_xfer_mode_count(tile_index, wr_xfer_mode_count.u);
        write_hmm_instr_dma_wr_memclr_data(tile_index, pattern);

        // Start the transfer
        wr_xfer_start.u = 0;
        wr_xfer_start.s.xfer_start = 1;
        write_hmm_instr_dma_wr_xfer_start(tile_index, wr_xfer_start.u);

        // Set-up for a 1 sec. timer
        uint64_t done = cvmx_clock_get_count(CVMX_CLOCK_CORE) + (uint64_t)1000000 * 
                        cvmx_clock_get_rate(CVMX_CLOCK_CORE) / 1000000;

         int result;
        // Wait for the raw interrupt xfer_raw_done_int to set
        while (1)
        {            
            wr_intr_rstatus.u = read_hmm_instr_dma_wr_intr_rstatus(tile_index);
            if (wr_intr_rstatus.s.xfer_raw_done_int) {
                result = 0;
                break;
            } else if (cvmx_clock_get_count(CVMX_CLOCK_CORE) > done) {
                result = -1;
                break;
            } else
                cvmx_thread_yield(); 
        }

        if (result == -1) {
            // Timeout
            cvmx_error("Timeout waiting for raw interupt on IMEM HMM\n");
         }

        // Init smem next

        // Clear any pending interrupts
        wr_intr_clear.u = 0;
        wr_intr_clear.s.xfer_done_int_clr = 1;
        wr_intr_clear.s.xfer_q_empty_int_clr = 1;
        write_hmm_ext_dma_wr_intr_clear(tile_index, wr_intr_clear.u);
        cvmx_wait_usec(200);
        wr_intr_rstatus.u = read_hmm_ext_dma_wr_intr_rstatus(tile_index);

        // Set mode to enable memory clear
        wr_mode.u = 0;
        wr_mode.s.mem_clr_enb = 1;
        wr_mode.s.mem_clr_data_sel = 1;
        write_hmm_ext_dma_wr_mode(tile_index, wr_mode.u);

        // Determine address and size
        wr_start_addr0.u = 0;
	switch (tile_index) {
            case RX0_TILE:
                 wr_start_addr0.s.addr = ENDOR_RX0_SDMEM;
                 break;
            case RX1_TILE:
                 wr_start_addr0.s.addr = ENDOR_RX1_SDMEM;
                 break;
            case TX_TILE:
                 wr_start_addr0.s.addr = ENDOR_TX_SDMEM;
                 break;
        }        

        wr_xfer_mode_count.u = 0;
        wr_xfer_mode_count.s.xfer_count =  (4 * 1024) - 1;  // 16kb
        wr_xfer_mode_count.s.xfer_done_intr = 1;

        write_hmm_ext_dma_wr_start_addr0(tile_index, wr_start_addr0.u);
        write_hmm_ext_dma_wr_xfer_mode_count(tile_index, wr_xfer_mode_count.u);
        write_hmm_ext_dma_wr_memclr_data(tile_index, pattern);

        // Start the transfer
        wr_xfer_start.u = 0;
        wr_xfer_start.s.xfer_start = 1;
        write_hmm_ext_dma_wr_xfer_start(tile_index, wr_xfer_start.u);

        // Set-up for a 1 sec. timer
        done = cvmx_clock_get_count(CVMX_CLOCK_CORE) + (uint64_t)1000000 * 
                        cvmx_clock_get_rate(CVMX_CLOCK_CORE) / 1000000;


        // Wait for the raw interrupt xfer_raw_done_int to set
        while (1)
        {            
            wr_intr_rstatus.u = read_hmm_ext_dma_wr_intr_rstatus(tile_index);
            if (wr_intr_rstatus.s.xfer_raw_done_int) {
                result = 0;
                break;
            } else if (cvmx_clock_get_count(CVMX_CLOCK_CORE) > done) {
                result = -1;
                break;
            } else
                cvmx_thread_yield(); 
        }

        if (result == -1) {
            // Timeout
            cvmx_error("Timeout waiting for raw interupt on SMEM HMM\n");
         }

    }
#endif
    /* Disable Init mode for DSP memories */
    phy_config.s.rx0imem_initenb = 0;
    phy_config.s.rx0smem_initenb = 0;
    phy_config.s.rx1imem_initenb = 0;
    phy_config.s.rx1smem_initenb = 0;
    phy_config.s.tximem_initenb = 0;
    phy_config.s.txsmem_initenb = 0;
    phy_config.s.rx0_gen_perr = init_flags & CVMX_ADMA_INIT_IMEM_PARITY_ENB;
    phy_config.s.rx1_gen_perr =  init_flags & CVMX_ADMA_INIT_IMEM_PARITY_ENB;
    phy_config.s.tx_gen_perr = init_flags & CVMX_ADMA_INIT_IMEM_PARITY_ENB;
    cvmx_write64_uint32(CVMX_BBP_RSTCLK_PHY_CONFIG, phy_config.u32);

    /* 
     * Enable all the clocks, probably not needed as clocks are enabled 
     * when ENDOR is released from reset
     */
    cvmx_bbp_rstclk_clkenb0_set_t clkenb0_set;
    cvmx_bbp_rstclk_clkenb1_set_t clkenb1_set;

    clkenb0_set.u32 = 0x1fff;
    clkenb1_set.u32 = 0xff;  /* bit 7 seems to be there but is not documented, why? */
    cvmx_write64_uint32(CVMX_BBP_RSTCLK_CLKENB0_SET, clkenb0_set.u32);
    cvmx_write64_uint32(CVMX_BBP_RSTCLK_CLKENB1_SET, clkenb1_set.u32);
}

static void cvmx_endor_init_adma_block(void)
{
    cvmx_bbp_adma_auto_clk_gate_t adma_auto_clk_gate;
    cvmx_bbp_adma_dma_reset_t adma_dma_reset;
    cvmx_bbp_adma_axierr_intr_t adma_axierr_intr;
    cvmx_bbp_adma_intr_enb_t adma_intr_enb;
    cvmx_bbp_adma_intr_dis_t adma_intr_dis;
    cvmx_bbp_adma_dma_priority_t adma_dma_priority;
    cvmx_bbp_adma_axi_signal_t adma_axi_signal;

    /* Enable the clock for the AXI block */
    {
	cvmx_bbp_rstclk_clkenb0_set_t rstclk_clkenb0_set;
	rstclk_clkenb0_set.u32 = cvmx_read64_uint32(
	    CVMX_BBP_RSTCLK_CLKENB0_SET);
	rstclk_clkenb0_set.s.axidma = 1;
	cvmx_write64_uint32(CVMX_BBP_RSTCLK_CLKENB0_SET,
	    rstclk_clkenb0_set.u32);
    }

    /* Put the AXI block into reset */
    {
	cvmx_bbp_rstclk_reset0_set_t rstclk_reset0_set;
	rstclk_reset0_set.u32 = cvmx_read64_uint32(
	    CVMX_BBP_RSTCLK_RESET0_SET);
    	rstclk_reset0_set.s.axidma = 1;
	cvmx_write64_uint32(CVMX_BBP_RSTCLK_RESET0_SET,
	    rstclk_reset0_set.u32);
    }

    cvmx_wait_usec(2000);

    /* Take the AXI block out of reset */
    {
	cvmx_bbp_rstclk_reset0_clr_t rstclk_reset0_clr;
	rstclk_reset0_clr.u32 = cvmx_read64_uint32(
	    CVMX_BBP_RSTCLK_RESET0_CLR);
    	rstclk_reset0_clr.s.axidma = 1;
	cvmx_write64_uint32(CVMX_BBP_RSTCLK_RESET0_CLR,
	    rstclk_reset0_clr.u32);
    }

    /* Clear auto  clock gating */
    adma_auto_clk_gate.u32 = 0;
    cvmx_write64_uint32(CVMX_BBP_ADMA_AUTO_CLK_GATE, adma_auto_clk_gate.u32);

    /* Set/Clear DMA channel reset */
    adma_dma_reset.u32 = 0;
    adma_dma_reset.s.dma_ch_reset = 0xff;
    cvmx_write64_uint32(CVMX_BBP_ADMA_DMA_RESET, adma_dma_reset.u32);

    cvmx_wait_usec( 2000 );

    adma_dma_reset.s.dma_ch_reset = 0x0;
    cvmx_write64_uint32(CVMX_BBP_ADMA_DMA_RESET, adma_dma_reset.u32);

    /* Clear AXI error interrupt and channel error status */
    adma_axierr_intr.u32 = 0;
    adma_axierr_intr.s.axi_err_int = 0;
    adma_axierr_intr.s.dma_ch_err = 0xff;
    cvmx_write64_uint32(CVMX_BBP_ADMA_AXIERR_INTR, adma_axierr_intr.u32);

    /* Clear DMADONE interrupt enables */
    adma_intr_enb.u32 = 0;
    cvmx_write64_uint32(CVMX_BBP_ADMA_INTR_ENB, adma_intr_enb.u32);

    /* Clear DMADONE interrupt disables */
    adma_intr_dis.u32 = 0;
    adma_intr_dis.s.dmadone_intr_dis = 0xff;
    adma_intr_dis.s.axierr_intr_dis = 0x1;
    cvmx_write64_uint32(CVMX_BBP_ADMA_INTR_DIS, adma_intr_dis.u32);

    /* Set DMA priority(s) */
    adma_dma_priority.u32 = 0;
    adma_dma_priority.s.wdma_fav = 0xf;  //no favorite
    adma_dma_priority.s.wdma_arb_mode = 1;  //round robin
    adma_dma_priority.s.rdma_fav = 0x0;     //no favorite
    adma_dma_priority.s.rdma_arb_mode = 1;  //round robin

    cvmx_write64_uint32(CVMX_BBP_ADMA_DMA_PRIORITY, adma_dma_priority.u32);

    // Set ADMA signals
    adma_axi_signal.u32 =0;
    adma_axi_signal.s.arlock =0;
    adma_axi_signal.s.awlock =0;
    adma_axi_signal.s.awcobuf =0;
    cvmx_write64_uint32(CVMX_BBP_ADMA_AXI_SIGNAL, adma_axi_signal.u32);
}

/*
 * Returns 1 if either the EOI or the EOI_ENDOR reset is on.
 */
static int __cvmx_endor_in_reset(void)
{
    cvmx_eoi_endor_ctl_t eoi_endor_ctl;
    cvmx_eoi_ctl_sta_t eoi_ctl_sta;

    eoi_endor_ctl.u64 = cvmx_read_csr(CVMX_EOI_ENDOR_CTL);
    if (eoi_endor_ctl.s.reset == 1)
        return 1;

    eoi_ctl_sta.u64 = cvmx_read_csr(CVMX_EOI_CTL_STA);
    if (eoi_ctl_sta.s.reset == 1)
        return 1;

    return 0;
}

void cvmx_bbp_reset_dsps(int tile)
{
cvmx_bbp_rstclk_clkenb1_set_t  clkenb1_set;
cvmx_bbp_rstclk_reset1_set_t   reset1_set;

clkenb1_set.u32 = 0;
reset1_set.u32 = 0;

    switch (tile)
    {
    case RX0_TILE:
        clkenb1_set.u32 = 1<<3;
        ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_CLKENB1_SET, cvmx_bbp_rstclk_clkenb1_set, c.u32=clkenb1_set.u32);
        cvmx_wait_usec(2000);

        ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_RESET1_SET, cvmx_bbp_rstclk_reset1_set,  c.s.rx0_dsps=1);
        cvmx_wait_usec(2000);

        ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_DSPSTALL_SET, cvmx_bbp_rstclk_dspstall_set, {c.s.rx0dsp0=1;c.s.rx0dsp1=1;});
        cvmx_wait_usec(2000);
        ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_RESET1_SET, cvmx_bbp_rstclk_reset1_clr,  c.s.rx0_dsps=1);
        cvmx_wait_usec(2000);
        break;
    case RX1_TILE:
	clkenb1_set.u32 = 1<<4;
        ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_CLKENB1_SET, cvmx_bbp_rstclk_clkenb1_set, c.u32 = clkenb1_set.u32);
        cvmx_wait_usec(2000);

	reset1_set.u32 = 1<<4;
        ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_RESET1_SET, cvmx_bbp_rstclk_reset1_set,  c.u32 = reset1_set.u32);
        cvmx_wait_usec(2000);

        ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_DSPSTALL_SET, cvmx_bbp_rstclk_dspstall_set, {c.s.rx1dsp0=1;c.s.rx1dsp1=1;});
        cvmx_wait_usec(2000);
	reset1_set.u32 = 1<<4;
        ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_RESET1_SET, cvmx_bbp_rstclk_reset1_clr,  c.u32 = reset1_set.u32);
        cvmx_wait_usec(2000);
        break;
    case TX_TILE:
	clkenb1_set.u32 = 1<<5;
        ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_CLKENB1_SET, cvmx_bbp_rstclk_clkenb1_set, c.u32 = clkenb1_set.u32);
        cvmx_wait_usec(2000);

        ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_RESET1_SET, cvmx_bbp_rstclk_reset1_set,  c.s.tx_dsps=1);
        cvmx_wait_usec(2000);
        ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_DSPSTALL_SET, cvmx_bbp_rstclk_dspstall_set, {c.s.txdsp0=1;c.s.txdsp1=1;});
        cvmx_wait_usec(2000);
        ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_RESET1_SET, cvmx_bbp_rstclk_reset1_clr,  c.s.tx_dsps=1);
        cvmx_wait_usec(2000);
        break;
    }
}

void cvmx_bbp_release_dsps(int tile)
{
    switch (tile)
    {
    case RX0_TILE:
        ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_RESET1_CLR, cvmx_bbp_rstclk_reset1_clr, c.s.rx0_dsps=1);
        break;
    case RX1_TILE:
/*        ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_RESET1_CLR, cvmx_bbp_rstclk_reset1_clr, c.s.rx1_dsps=1); */
        break;
    case TX_TILE:
        ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_RESET1_CLR, cvmx_bbp_rstclk_reset1_clr, c.s.tx_dsps=1);
        break;
    }
}

void cvmx_bbp_stall_dsps(int tile, int dsp)
{
    switch (tile)
    {
    case RX0_TILE:
        switch (dsp)
	{
        case 0:
            ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_DSPSTALL_SET, cvmx_bbp_rstclk_dspstall_set, c.s.rx0dsp0=1);
            break;
        case 1:
            ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_DSPSTALL_SET, cvmx_bbp_rstclk_dspstall_set, c.s.rx0dsp1=1);
            break;
        case 2:
            ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_DSPSTALL_SET, cvmx_bbp_rstclk_dspstall_set, {c.s.rx0dsp0=1; c.s.rx0dsp1=1;});
            break;
        }
        break;
    case RX1_TILE:
        switch (dsp)
	{
        case 0:
            ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_DSPSTALL_SET, cvmx_bbp_rstclk_dspstall_set, c.s.rx1dsp0=1);
            break;
        case 1:
            ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_DSPSTALL_SET, cvmx_bbp_rstclk_dspstall_set, c.s.rx1dsp1=1);
            break;
        case 2:
            ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_DSPSTALL_SET, cvmx_bbp_rstclk_dspstall_set, {c.s.rx1dsp0=1; c.s.rx1dsp1=1;});
            break;
        }
        break;
    case TX_TILE:
        switch (dsp)
	{
        case 0:
            ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_DSPSTALL_SET, cvmx_bbp_rstclk_dspstall_set, c.s.txdsp0=1);
            break;
        case 1:
            ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_DSPSTALL_SET, cvmx_bbp_rstclk_dspstall_set, c.s.txdsp1=1);
            break;
        case 2:
            ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_DSPSTALL_SET, cvmx_bbp_rstclk_dspstall_set, {c.s.txdsp0=1; c.s.txdsp1=1;});
            break;
        }
        break;
    }
}

void cvmx_bbp_unstall_dsps(int tile, int dsp)
{
    switch (tile)
    {
    case RX0_TILE:
        switch (dsp)
	{
        case 0:
            ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_DSPSTALL_CLR, cvmx_bbp_rstclk_dspstall_clr, c.s.rx0dsp0=1);
            break;
        case 1:
            ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_DSPSTALL_CLR, cvmx_bbp_rstclk_dspstall_clr, c.s.rx0dsp1=1);
            break;
        case 2:
            ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_DSPSTALL_CLR, cvmx_bbp_rstclk_dspstall_clr, {c.s.rx0dsp0=1; c.s.rx0dsp1=1;});
            break;
        }
        break;
    case RX1_TILE:
        switch (dsp)
	{
        case 0:
            ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_DSPSTALL_CLR, cvmx_bbp_rstclk_dspstall_clr, c.s.rx1dsp0=1);
            break;
        case 1:
            ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_DSPSTALL_CLR, cvmx_bbp_rstclk_dspstall_clr, c.s.rx1dsp1=1);
            break;
        case 2:
            ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_DSPSTALL_CLR, cvmx_bbp_rstclk_dspstall_clr, {c.s.rx1dsp0=1; c.s.rx1dsp1=1;});
            break;
        }
        break;
    case TX_TILE:
        switch (dsp)
	{
        case 0:
            ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_DSPSTALL_CLR, cvmx_bbp_rstclk_dspstall_clr, c.s.txdsp0=1);
            break;
        case 1:
            ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_DSPSTALL_CLR, cvmx_bbp_rstclk_dspstall_clr, c.s.txdsp1=1);
            break;
        case 2:
            ENDOR_CSR_MODIFY(c, CVMX_BBP_RSTCLK_DSPSTALL_CLR, cvmx_bbp_rstclk_dspstall_clr, {c.s.txdsp0=1; c.s.txdsp1=1;});
            break;
        }
        break;
    }
}

int cvmx_bbp_adma_init(uint64_t init_flags)
{
    if (__cvmx_endor_in_reset())
    {
        cvmx_endor_init_endor_block(init_flags);
	cvmx_endor_init_adma_block();
    }

    /*
     * We expect a named block (ADMA_BOOTMEM_NAME) reserved for the
     * 512K bounce buffer before the app boots up. But in case it is
     * not, make the last-ditch effort.
     */
    cvmx_adma_buf = cvmx_bootmem_alloc_named_range_once(
	ADMA_BOOTMEM_SIZE, 0, 0, 128, ADMA_BOOTMEM_NAME, NULL);
    if (cvmx_adma_buf == NULL)
        return -1;

#ifdef DEBUG_ADMA
    cvmx_dprintf("__cvmx_endor_adma_init() returns 0 with"
        " cvmx_adma_buf = %p\n", cvmx_adma_buf);
#endif

    return 0;
}

EXPORT_SYMBOL(cvmx_bbp_adma_init);
EXPORT_SYMBOL(cvmx_bbp_unstall_dsps);
EXPORT_SYMBOL(cvmx_bbp_stall_dsps);
EXPORT_SYMBOL(cvmx_bbp_release_dsps);
EXPORT_SYMBOL(cvmx_bbp_reset_dsps);
EXPORT_SYMBOL(cvmx_adma_io);
