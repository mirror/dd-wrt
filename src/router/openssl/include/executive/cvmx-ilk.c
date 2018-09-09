/***********************license start***************
 * Copyright (c) 2003-2013  Cavium Inc. (support@cavium.com). All rights
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
 * Support library for the ILK
 *
 */

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <linux/export.h>
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-sysinfo.h>
#include <asm/octeon/cvmx-pko.h>
#include <asm/octeon/cvmx-ilk.h>
#include <asm/octeon/cvmx-qlm.h>
#include <asm/octeon/cvmx-ilk-defs.h>
#include <asm/octeon/cvmx-helper-util.h>
#include <asm/octeon/cvmx-helper-ilk.h>
#include <asm/octeon/cvmx-gserx-defs.h>
#include <asm/octeon/cvmx-pip-defs.h>
#else
#include "cvmx.h"
#include "cvmx-sysinfo.h"
#include "cvmx-pko.h"
#include "cvmx-ilk.h"
#include "cvmx-qlm.h"
#include "cvmx-helper-util.h"
#include "cvmx-helper-ilk.h"
#include "cvmx-pip-defs.h"
#endif


/*
 * global configurations.
 *
 * for cn68, the default is {0xf, 0xf0}. to disable the 2nd ILK, set
 * cvmx_ilk_lane_mask[CVMX_NUM_ILK_INTF] = {0xff, 0x0} and
 * cvmx_ilk_chans[CVMX_NUM_ILK_INTF] = {8, 0}
 *
 * for cn78, the default is {0xff, 0xf000}.
 */
CVMX_SHARED unsigned short cvmx_ilk_lane_mask[CVMX_NUM_ILK_INTF] = {0xf, 0xf0};

CVMX_SHARED unsigned char cvmx_ilk_chans[CVMX_NUM_ILK_INTF] = {8,8};

unsigned char cvmx_ilk_chan_map[CVMX_NUM_ILK_INTF][CVMX_ILK_MAX_CHANS] = { {0, 1, 2, 3, 4, 5, 6, 7},
{0, 1, 2, 3, 4, 5, 6, 7}
};

static cvmx_ilk_intf_t cvmx_ilk_intf_cfg[CVMX_NUM_ILK_INTF];

CVMX_SHARED cvmx_ilk_LA_mode_t cvmx_ilk_LA_mode[CVMX_NUM_ILK_INTF] = {{0, 0},
									{0, 0}};

void cvmx_ilk_config_set_LA_mode(int interface, int mode)
{
	if(interface >= CVMX_NUM_ILK_INTF || interface < 0)
		cvmx_dprintf("ERROR: Invalid interface=%d in cvmx_ilk_config_set_LA_mode\n",
			     interface);
	else
		cvmx_ilk_LA_mode[interface].ilk_LA_mode = mode;
}

void cvmx_ilk_config_set_LA_mode_cal(int interface, int mode)
{
	if(interface >= CVMX_NUM_ILK_INTF || interface < 0)
		cvmx_dprintf("ERROR: Invalid interface=%d in cvmx_ilk_config_set_LA_mode_cal\n",
			     interface);
	else
		cvmx_ilk_LA_mode[interface].ilk_LA_mode_cal_ena = mode;
}

void cvmx_ilk_config_set_lane_mask(int interface, unsigned char mask)
{
	if(interface >= CVMX_NUM_ILK_INTF || interface < 0)
		cvmx_dprintf("ERROR: Invalid interface=%d in cvmx_ilk_set_lane_mask\n",
			     interface);
	else
		cvmx_ilk_lane_mask[interface] = mask << (4*interface);
}

void cvmx_ilk_config_set_max_channels(int interface, unsigned char channels)
{
	if(interface >= CVMX_NUM_ILK_INTF || interface < 0) {
		cvmx_dprintf("ERROR: Invalid interface=%d in cvmx_ilk_config_set_max_channels\n",
			     interface);
		return;
	}
	if(channels > CVMX_ILK_MAX_CHANS){
		cvmx_dprintf("ERROR: Invalid channel=%d in cvmx_ilk_config_set_max_channels",channels);
		return;
	}
	cvmx_ilk_chans[interface] = channels;
}

/**
 * User-overrideable callback function that returns whether or not an interface
 * should use look-aside mode.
 *
 * @param interface - interface being checked
 * @param channel - channel number, can be 0 or 1 or -1 to see if LA mode
 *                  should be enabled for the interface.
 * @return 0 to not use LA-mode, 1 to use LA-mode.
 */
int cvmx_ilk_use_la_mode(int interface, int channel)
{
	if (OCTEON_IS_MODEL(OCTEON_CN68XX_PASS1_0))
		return 0;

	if(interface >= CVMX_NUM_ILK_INTF) {
		cvmx_dprintf("ERROR: invalid interface=%d in cvmx_ilk_use_la_mode\n",
			     interface);
		return -1;
	}
	return cvmx_ilk_LA_mode[interface].ilk_LA_mode;
}

/**
 * User-overrideable callback function that returns whether or not an interface
 * in look-aside mode should enable the RX calendar.
 *
 * @param interface - interface to check
 * @return 1 to enable RX calendar, 0 to disable RX calendar.
 *
 * NOTE: For the CN68XX pass 2.0 this will enable the RX calendar for interface
 * 0 and not interface 1.  It is up to the customer to override this behavior.
 */
int cvmx_ilk_la_mode_enable_rx_calendar(int interface)
{
	/* There is an errata in the CN68XX pass 2.0 where if connected
	 * in a loopback configuration or back to back then only one interface
	 * can have the RX calendar enabled.
	 */
	if(interface >= CVMX_NUM_ILK_INTF) {
		cvmx_dprintf("ERROR: invalid interface=%d in \
				cvmx_ilk_la_mode_enable_rx_calendar\n", interface);
		return -1;
	}
	return cvmx_ilk_LA_mode[interface].ilk_LA_mode_cal_ena;
}

/**
 * Initialize and start the ILK interface.
 *
 * @param interface The identifier of the packet interface to configure and
 *                  use as a ILK interface. cn68xx has 2 interfaces: ilk0 and
 *                  ilk1.
 *
 * @param lane_mask the lane group for this interface
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_ilk_start_interface(int interface, unsigned short lane_mask)
{
	int res = -1;
	int other_intf, this_qlm, other_qlm;
	unsigned short uni_mask;
	cvmx_mio_qlmx_cfg_t mio_qlmx_cfg, other_mio_qlmx_cfg;
	cvmx_ilk_txx_cfg0_t ilk_txx_cfg0;
	cvmx_ilk_rxx_cfg0_t ilk_rxx_cfg0;
	cvmx_ilk_ser_cfg_t ilk_ser_cfg;

	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)) &&
	    !(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		return res;

	if (interface >= CVMX_NUM_ILK_INTF)
		return res;

	if (lane_mask == 0)
		return res;

	/* check conflicts between 2 ilk interfaces. 1 lane can be assigned to 1
	 * interface only */
	other_intf = !interface;
	if (cvmx_ilk_intf_cfg[other_intf].lane_en_mask & lane_mask) {
		cvmx_dprintf("ILK%d: %s: lane assignment conflict\n", interface, __func__);
		return res;
	}

	/* check the legality of the lane mask. interface 0 can have 8 lanes,
	 * while interface 1 can have 4 lanes at most */
	uni_mask = lane_mask >> (interface * 4);
	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		if ((uni_mask != 0x1 && uni_mask != 0x3 && uni_mask != 0xf && uni_mask != 0xff) || (interface == 1 && lane_mask > 0xf0)) {
			cvmx_dprintf("ILK%d: %s: incorrect lane mask: 0x%x \n", interface, __func__, uni_mask);
			return res;
		}
	}
	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		if ((uni_mask != 0x1 && uni_mask != 0x3 && uni_mask != 0xf &&
		     uni_mask != 0xff && uni_mask != 0x100 &&
		     uni_mask != 0x300 && uni_mask != 0xf00 &&
		     uni_mask != 0xff0 && uni_mask != 0xfff) ||
		     (interface == 0 && lane_mask > 0xff) ||
		     (interface == 1 && lane_mask > 0xfff0)) {
			cvmx_dprintf("ILK%d: %s: incorrect lane mask: 0x%x \n", interface, __func__, uni_mask);
			return res;
		}
	}

	/* check the availability of qlms. qlm_cfg = 001 means the chip is fused
	 * to give this qlm to ilk */
	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		this_qlm = interface + CVMX_ILK_QLM_BASE();
		other_qlm = other_intf + CVMX_ILK_QLM_BASE();
		mio_qlmx_cfg.u64 = cvmx_read_csr(CVMX_MIO_QLMX_CFG(this_qlm));
		other_mio_qlmx_cfg.u64 = cvmx_read_csr(CVMX_MIO_QLMX_CFG(other_qlm));
		if (mio_qlmx_cfg.s.qlm_cfg != 1 || (uni_mask == 0xff && other_mio_qlmx_cfg.s.qlm_cfg != 1)) {
			cvmx_dprintf("ILK%d: %s: qlm unavailable\n", interface, __func__);
			return res;
		}
	}
	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		uint8_t 		qlm, i;
		cvmx_mio_qlmx_cfg_t 	mio_qlmx_cfg[4];
		cvmx_gserx_cfg_t	cvmx_gserx_cfg;
		cvmx_gserx_phy_ctl_t	cvmx_gserx_phy_ctl;

		for (i = 0, qlm = CVMX_ILK_QLM_BASE(); i < 4; i++, qlm++)
			mio_qlmx_cfg[i].u64 = cvmx_read_csr(CVMX_MIO_QLMX_CFG(qlm));

		/*
		 * QLM registers are not modelled yet. So hardcore the expected
		 * value here. This must be removed once the hardware is ready
		 * or the simulator has qlm support. TODO
		 */
		mio_qlmx_cfg[0].s.qlm_cfg = 1;
		if (interface == 0) {
			if ((uni_mask <= 0xf && mio_qlmx_cfg[0].s.qlm_cfg != 1) ||
			    (uni_mask == 0xff &&
			     (mio_qlmx_cfg[0].s.qlm_cfg != 1 || mio_qlmx_cfg[1].s.qlm_cfg != 1))) {
				cvmx_dprintf("ILK%d: %s: qlm unavailable\n", interface, __func__);
				return res;
			} 
		}
			      
		if (interface == 1) {
			if ((uni_mask >= 0x100 && uni_mask <= 0xf00 && mio_qlmx_cfg[3].s.qlm_cfg != 1) ||
			    (uni_mask == 0xff0 && 
			     (mio_qlmx_cfg[3].s.qlm_cfg != 1 || mio_qlmx_cfg[2].s.qlm_cfg != 1)) ||
			    (uni_mask == 0xfff &&
			     (mio_qlmx_cfg[3].s.qlm_cfg != 1 || mio_qlmx_cfg[2].s.qlm_cfg != 1 || mio_qlmx_cfg[1].s.qlm_cfg != 1))) {
				cvmx_dprintf("ILK%d: %s: qlm unavailable\n", interface, __func__);
				return res;
			} 
		}

		/*
		 * Configure the GSER.
		 * For now, we configured the minimum needed to work with the
		 * simulator. TODO
		 */
		qlm = CVMX_ILK_QLM_BASE() + interface;
		cvmx_gserx_phy_ctl.u64 = cvmx_read_csr(CVMX_GSERX_PHY_CTL(qlm));
		cvmx_gserx_phy_ctl.s.phy_pd = 0;
		cvmx_gserx_phy_ctl.s.phy_reset = 1;
		cvmx_write_csr(CVMX_GSERX_PHY_CTL(qlm), cvmx_gserx_phy_ctl.u64);

		cvmx_gserx_cfg.u64 = cvmx_read_csr(CVMX_GSERX_CFG(qlm));
		cvmx_gserx_cfg.s.ila = 1;
		cvmx_write_csr(CVMX_GSERX_CFG(qlm), cvmx_gserx_cfg.u64);

		cvmx_gserx_phy_ctl.u64 = cvmx_read_csr(CVMX_GSERX_PHY_CTL(qlm));
		cvmx_gserx_phy_ctl.s.phy_reset = 0;
		cvmx_write_csr(CVMX_GSERX_PHY_CTL(qlm), cvmx_gserx_phy_ctl.u64);
	}

	/* power up the serdes */
	ilk_ser_cfg.u64 = cvmx_read_csr(CVMX_ILK_SER_CFG);
	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		if (ilk_ser_cfg.cn68xx.ser_pwrup == 0) {
			ilk_ser_cfg.cn68xx.ser_rxpol_auto = 1;
			ilk_ser_cfg.cn68xx.ser_rxpol = 0;
			ilk_ser_cfg.cn68xx.ser_txpol = 0;
			ilk_ser_cfg.cn68xx.ser_reset_n = 0xff;
			ilk_ser_cfg.cn68xx.ser_haul = 0;
		}
		ilk_ser_cfg.cn68xx.ser_pwrup |= ((interface == 0) && (lane_mask > 0xf)) ? 0x3 : (1 << interface);
	}
	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		ilk_ser_cfg.cn78xx.ser_rxpol_auto = 1;
		ilk_ser_cfg.cn78xx.ser_rxpol = 0;
		ilk_ser_cfg.cn78xx.ser_txpol = 0;
	}
	cvmx_write_csr(CVMX_ILK_SER_CFG, ilk_ser_cfg.u64);

	if (OCTEON_IS_MODEL(OCTEON_CN68XX_PASS2_X)
	    && (cvmx_sysinfo_get()->board_type != CVMX_BOARD_TYPE_SIM)) {
		/* Workaround for Errata (G-16467) */
		int qlm = (interface) ? 2 : 1;
		int start_qlm, end_qlm;

		/* Apply the workaround to both the QLMs if configured for x8 lanes */
		if (cvmx_pop(lane_mask) > 4) {
			start_qlm = 1;
			end_qlm = 2;
		} else {
			start_qlm = qlm;
			end_qlm = qlm;
		}

		for (qlm = start_qlm; qlm <= end_qlm; qlm++) {
#ifdef CVMX_QLM_DUMP_STATE
			cvmx_dprintf("%s:%d: ILK%d: Applying workaround for Errata G-16467\n", __func__, __LINE__, qlm);
			cvmx_qlm_display_registers(qlm);
			cvmx_dprintf("\n");
#endif
			/* This workaround only applies to QLMs running ILK at 6.25Ghz */
			if ((cvmx_qlm_get_gbaud_mhz(qlm) == 6250) && (cvmx_qlm_jtag_get(qlm, 0, "clkf_byp") != 20)) {
				cvmx_wait_usec(100);	/* Wait 100us for links to stabalize */
				cvmx_qlm_jtag_set(qlm, -1, "clkf_byp", 20);
				/* Allow the QLM to exit reset */
				cvmx_qlm_jtag_set(qlm, -1, "cfg_rst_n_clr", 0);
				cvmx_wait_usec(100);	/* Wait 100us for links to stabalize */
				/* Allow TX on QLM */
				cvmx_qlm_jtag_set(qlm, -1, "cfg_tx_idle_set", 0);
			}
#ifdef CVMX_QLM_DUMP_STATE
			cvmx_dprintf("%s:%d: ILK%d: Done applying workaround for Errata G-16467\n", __func__, __LINE__, qlm);
			cvmx_qlm_display_registers(qlm);
			cvmx_dprintf("\n\n");
#endif
		}
	}

	/* Initialize all calendar entries to xoff state */
	__cvmx_ilk_init_cal(interface);

	/* Enable ILK LA mode if configured. */
	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		if (cvmx_ilk_use_la_mode(interface, 0)) {
			cvmx_ilk_txx_cfg1_t ilk_txx_cfg1;
			cvmx_ilk_rxx_cfg1_t ilk_rxx_cfg1;
	
			ilk_txx_cfg1.u64 = cvmx_read_csr(CVMX_ILK_TXX_CFG1(interface));
			ilk_rxx_cfg1.u64 = cvmx_read_csr(CVMX_ILK_RXX_CFG1(interface));
			ilk_txx_cfg1.s.la_mode = 1;
			ilk_txx_cfg1.s.tx_link_fc_jam = 1;
			ilk_txx_cfg1.s.rx_link_fc_ign = 1;
			ilk_rxx_cfg1.s.la_mode = 1;
			cvmx_write_csr(CVMX_ILK_TXX_CFG1(interface), ilk_txx_cfg1.u64);
			cvmx_write_csr(CVMX_ILK_RXX_CFG1(interface), ilk_rxx_cfg1.u64);
			cvmx_ilk_intf_cfg[interface].la_mode = 1;	/* Enable look-aside mode */
		} else
			cvmx_ilk_intf_cfg[interface].la_mode = 0;	/* Disable look-aside mode */
	}
	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		cvmx_ilk_intf_cfg[interface].la_mode = 0;
	}

	/* configure the lane enable of the interface */
	ilk_txx_cfg0.u64 = cvmx_read_csr(CVMX_ILK_TXX_CFG0(interface));
	ilk_rxx_cfg0.u64 = cvmx_read_csr(CVMX_ILK_RXX_CFG0(interface));
	ilk_txx_cfg0.s.lane_ena = ilk_rxx_cfg0.s.lane_ena = lane_mask;
	cvmx_write_csr(CVMX_ILK_TXX_CFG0(interface), ilk_txx_cfg0.u64);
	cvmx_write_csr(CVMX_ILK_RXX_CFG0(interface), ilk_rxx_cfg0.u64);

	/* write to local cache. for lane speed, if interface 0 has 8 lanes,
	 * assume both qlms have the same speed */
	cvmx_ilk_intf_cfg[interface].intf_en = 1;
	cvmx_ilk_intf_cfg[interface].lane_en_mask = lane_mask;
	res = 0;

	return res;
}

/**
 * set pipe group base and length for the interface
 *
 * @param interface The identifier of the packet interface to configure and
 *                  use as a ILK interface. cn68xx has 2 interfaces: ilk0 and
 *                  ilk1.
 *
 * @param pipe_base the base of the pipe group
 * @param pipe_len  the length of the pipe group
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_ilk_set_pipe(int interface, int pipe_base, unsigned int pipe_len)
{
	int res = -1;
	cvmx_ilk_txx_pipe_t ilk_txx_pipe;

	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		return res;

	if (interface >= CVMX_NUM_ILK_INTF)
		return res;

	/* set them in ilk tx section */
	ilk_txx_pipe.u64 = cvmx_read_csr(CVMX_ILK_TXX_PIPE(interface));
	ilk_txx_pipe.s.base = pipe_base;
	ilk_txx_pipe.s.nump = pipe_len;
	cvmx_write_csr(CVMX_ILK_TXX_PIPE(interface), ilk_txx_pipe.u64);
	res = 0;

	return res;
}

/**
 * set logical channels for tx
 *
 * @param interface The identifier of the packet interface to configure and
 *                  use as a ILK interface. cn68xx has 2 interfaces: ilk0 and
 *                  ilk1.
 *
 * @param pch     pointer to an array of pipe-channel pair
 * @param num_chs the number of entries in the pipe-channel array
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_ilk_tx_set_channel(int interface, cvmx_ilk_pipe_chan_t * pch, unsigned int num_chs)
{
	int res = -1;
	cvmx_ilk_txx_idx_pmap_t ilk_txx_idx_pmap;
	cvmx_ilk_txx_mem_pmap_t ilk_txx_mem_pmap;
	unsigned int i;

	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		return res;

	if (interface >= CVMX_NUM_ILK_INTF)
		return res;

	if (pch == NULL || num_chs > CVMX_ILK_MAX_PIPES)
		return res;

	if (cvmx_ilk_use_la_mode(interface, 0)) {
		ilk_txx_idx_pmap.u64 = 0;
		ilk_txx_mem_pmap.u64 = 0;
		for (i = 0; i < num_chs; i++) {
			ilk_txx_idx_pmap.s.index = pch->pipe;
			ilk_txx_mem_pmap.s.channel = pch->chan;
			ilk_txx_mem_pmap.s.remap = 1;
			cvmx_write_csr(CVMX_ILK_TXX_IDX_PMAP(interface), ilk_txx_idx_pmap.u64);
			cvmx_write_csr(CVMX_ILK_TXX_MEM_PMAP(interface), ilk_txx_mem_pmap.u64);
			pch++;
		}
	} else {
		/* write the pair to ilk tx */
		ilk_txx_mem_pmap.u64 = 0;
		ilk_txx_idx_pmap.u64 = 0;
		for (i = 0; i < num_chs; i++) {
			ilk_txx_idx_pmap.s.index = pch->pipe;
			ilk_txx_mem_pmap.s.channel = pch->chan;
			cvmx_write_csr(CVMX_ILK_TXX_IDX_PMAP(interface), ilk_txx_idx_pmap.u64);
			cvmx_write_csr(CVMX_ILK_TXX_MEM_PMAP(interface), ilk_txx_mem_pmap.u64);
			pch++;
		}
	}
	res = 0;

	return res;
}

/**
 * set pkind for rx
 *
 * @param interface The identifier of the packet interface to configure and
 *                  use as a ILK interface. cn68xx has 2 interfaces: ilk0 and
 *                  ilk1.
 *
 * @param chpknd    pointer to an array of channel-pkind pair
 * @param num_pknd the number of entries in the channel-pkind array
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_ilk_rx_set_pknd(int interface, cvmx_ilk_chan_pknd_t * chpknd, unsigned int num_pknd)
{
	int res = -1;
	cvmx_ilk_rxf_idx_pmap_t ilk_rxf_idx_pmap;
	unsigned int i;

	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)) &&
	    !(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		return res;

	if (interface >= CVMX_NUM_ILK_INTF)
		return res;

	if (chpknd == NULL || num_pknd > CVMX_ILK_MAX_PKNDS)
		return res;

	res = 0;

	for (i = 0; i < num_pknd; i++) {
		if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
			ilk_rxf_idx_pmap.u64 = 0;
			/* write the pair to ilk rx. note the channels for different interfaces
			 * are given in *chpknd and interface is not used as a param */
			if ((chpknd->chan < 2) && cvmx_ilk_use_la_mode(interface, chpknd->chan)) {
				ilk_rxf_idx_pmap.s.index = interface * 256 + 128 + chpknd->chan;
				cvmx_write_csr(CVMX_ILK_RXF_IDX_PMAP, ilk_rxf_idx_pmap.u64);
				cvmx_write_csr(CVMX_ILK_RXF_MEM_PMAP, chpknd->pknd);
			}
			ilk_rxf_idx_pmap.s.index = interface * 256 + chpknd->chan;
			cvmx_write_csr(CVMX_ILK_RXF_IDX_PMAP, ilk_rxf_idx_pmap.u64);
			cvmx_write_csr(CVMX_ILK_RXF_MEM_PMAP, chpknd->pknd);
		}
		if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
			cvmx_write_csr(CVMX_ILK_RXX_CHAX(chpknd->chan, interface), chpknd->pknd);
		}
		chpknd++;
	}
	return res;
}

/**
 * configure calendar for rx
 *
 * @param interface The identifier of the packet interface to configure and
 *                  use as a ILK interface. cn68xx has 2 interfaces: ilk0 and
 *                  ilk1.
 *
 * @param cal_depth the number of calendar entries
 * @param pent      pointer to calendar entries
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_ilk_rx_cal_conf(int interface, int cal_depth, cvmx_ilk_cal_entry_t * pent)
{
	int res = -1, i;
	cvmx_ilk_rxx_cfg0_t ilk_rxx_cfg0;
	int num_entries;

	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)) &&
	    !(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		return res;

	if (interface >= CVMX_NUM_ILK_INTF)
		return res;

	if (cal_depth < CVMX_ILK_RX_MIN_CAL || cal_depth > CVMX_ILK_MAX_CAL || pent == NULL)
		return res;

	/* mandatory link-level fc as workarounds for ILK-15397 and ILK-15479 */
	/* TODO: test effectiveness */
#if 0
	if (OCTEON_IS_MODEL(OCTEON_CN68XX_PASS1_0) && pent->ent_ctrl == PIPE_BPID)
		for (i = 0; i < cal_depth; i++)
			pent->ent_ctrl = LINK;
#endif

	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		/* Update the calendar for each channel */
		if ((cvmx_ilk_use_la_mode(interface, 0) == 0) ||
		    (cvmx_ilk_use_la_mode(interface, 0) && 
		     cvmx_ilk_la_mode_enable_rx_calendar(interface))) {
			for (i = 0; i < cal_depth; i++) {
				__cvmx_ilk_write_rx_cal_entry(interface, i,
							     pent[i].pipe_bpid);
			}
		}

		/* Update the depth */
		ilk_rxx_cfg0.u64 = cvmx_read_csr(CVMX_ILK_RXX_CFG0(interface));
		num_entries = 1 + cal_depth + (cal_depth - 1) / 15;
		ilk_rxx_cfg0.s.cal_depth = num_entries;
		if (cvmx_ilk_use_la_mode(interface, 0)) {
			ilk_rxx_cfg0.s.mproto_ign = 1;
			ilk_rxx_cfg0.s.lnk_stats_ena = 1;
			ilk_rxx_cfg0.s.lnk_stats_wrap = 1;
		}
		cvmx_write_csr(CVMX_ILK_RXX_CFG0(interface), ilk_rxx_cfg0.u64);
	}

	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		cvmx_ilk_rxx_cal_entryx_t rxx_cal_entryx;

		ilk_rxx_cfg0.u64 = cvmx_read_csr(CVMX_ILK_RXX_CFG0(interface));
		ilk_rxx_cfg0.s.cal_depth = cal_depth;
		cvmx_write_csr(CVMX_ILK_RXX_CFG0(interface), ilk_rxx_cfg0.u64);

		for (i = 0; i < cal_depth; i++) {
			rxx_cal_entryx.u64 = 0;
			rxx_cal_entryx.s.ctl = pent->ent_ctrl;
			rxx_cal_entryx.s.channel = pent->pipe_bpid;

			cvmx_write_csr(CVMX_ILK_RXX_CAL_ENTRYX(i, interface), rxx_cal_entryx.u64);
			pent++;
		}
	}

	return 0;
}

/**
 * set high water mark for rx
 *
 * @param interface The identifier of the packet interface to configure and
 *                  use as a ILK interface. cn68xx has 2 interfaces: ilk0 and
 *                  ilk1.
 *
 * @param hi_wm     high water mark for this interface
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_ilk_rx_set_hwm(int interface, int hi_wm)
{
	int res = -1;
	cvmx_ilk_rxx_cfg1_t ilk_rxx_cfg1;

	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)) &&
	    !(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		return res;

	if (interface >= CVMX_NUM_ILK_INTF)
		return res;

	if (hi_wm <= 0)
		return res;

	/* set the hwm */
	ilk_rxx_cfg1.u64 = cvmx_read_csr(CVMX_ILK_RXX_CFG1(interface));
	ilk_rxx_cfg1.s.rx_fifo_hwm = hi_wm;
	cvmx_write_csr(CVMX_ILK_RXX_CFG1(interface), ilk_rxx_cfg1.u64);
	res = 0;

	return res;
}

/**
 * enable calendar for rx
 *
 * @param interface The identifier of the packet interface to configure and
 *                  use as a ILK interface. cn68xx has 2 interfaces: ilk0 and
 *                  ilk1.
 *
 * @param cal_ena   enable or disable calendar
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_ilk_rx_cal_ena(int interface, unsigned char cal_ena)
{
	int res = -1;
	cvmx_ilk_rxx_cfg0_t ilk_rxx_cfg0;

	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)) &&
	    !(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		return res;

	if (interface >= CVMX_NUM_ILK_INTF)
		return res;

	if (cvmx_ilk_use_la_mode(interface, 0) && !cvmx_ilk_la_mode_enable_rx_calendar(interface))
		return 0;

	/* set the enable */
	ilk_rxx_cfg0.u64 = cvmx_read_csr(CVMX_ILK_RXX_CFG0(interface));
	ilk_rxx_cfg0.s.cal_ena = cal_ena;
	cvmx_write_csr(CVMX_ILK_RXX_CFG0(interface), ilk_rxx_cfg0.u64);
	cvmx_read_csr(CVMX_ILK_RXX_CFG0(interface));
	res = 0;

	return res;
}

/**
 * set up calendar for rx
 *
 * @param interface The identifier of the packet interface to configure and
 *                  use as a ILK interface. cn68xx has 2 interfaces: ilk0 and
 *                  ilk1.
 *
 * @param cal_depth the number of calendar entries
 * @param pent      pointer to calendar entries
 * @param hi_wm     high water mark for this interface
 * @param cal_ena   enable or disable calendar
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_ilk_cal_setup_rx(int interface, int cal_depth, cvmx_ilk_cal_entry_t * pent, int hi_wm, unsigned char cal_ena)
{
	int res = -1;

	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)) &&
	    !(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		return res;

	if (interface >= CVMX_NUM_ILK_INTF)
		return res;

	res = cvmx_ilk_rx_cal_conf(interface, cal_depth, pent);
	if (res < 0)
		return res;

	res = cvmx_ilk_rx_set_hwm(interface, hi_wm);
	if (res < 0)
		return res;

	res = cvmx_ilk_rx_cal_ena(interface, cal_ena);
	return res;
}

EXPORT_SYMBOL(cvmx_ilk_cal_setup_rx);

/**
 * configure calendar for tx
 *
 * @param interface The identifier of the packet interface to configure and
 *                  use as a ILK interface. cn68xx has 2 interfaces: ilk0 and
 *                  ilk1.
 *
 * @param cal_depth the number of calendar entries
 * @param pent      pointer to calendar entries
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_ilk_tx_cal_conf(int interface, int cal_depth, cvmx_ilk_cal_entry_t * pent)
{
	int res = -1, i;
	cvmx_ilk_txx_cfg0_t ilk_txx_cfg0;
	int num_entries;

	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)) &&
	    !(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		return res;

	if (interface >= CVMX_NUM_ILK_INTF)
		return res;

	if (cal_depth < CVMX_ILK_TX_MIN_CAL || cal_depth > CVMX_ILK_MAX_CAL || pent == NULL)
		return res;

	/* mandatory link-level fc as workarounds for ILK-15397 and ILK-15479 */
	/* TODO: test effectiveness */
#if 0
	if (OCTEON_IS_MODEL(OCTEON_CN68XX_PASS1_0) && pent->ent_ctrl == PIPE_BPID)
		for (i = 0; i < cal_depth; i++)
			pent->ent_ctrl = LINK;
#endif

	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		/* Update the calendar for each channel */
		for (i = 0; i < cal_depth; i++) {
			__cvmx_ilk_write_tx_cal_entry(interface, i,
						      pent[i].pipe_bpid);
		}

		/* Set the depth (must be multiple of 8)*/
		ilk_txx_cfg0.u64 = cvmx_read_csr(CVMX_ILK_TXX_CFG0(interface));
		num_entries = 1 + cal_depth + (cal_depth - 1) / 15;
		ilk_txx_cfg0.s.cal_depth = (num_entries + 7) & ~7;
		cvmx_write_csr(CVMX_ILK_TXX_CFG0(interface), ilk_txx_cfg0.u64);
	}

	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		cvmx_ilk_txx_cal_entryx_t txx_cal_entryx;

		ilk_txx_cfg0.u64 = cvmx_read_csr(CVMX_ILK_TXX_CFG0(interface));
		ilk_txx_cfg0.s.cal_depth = cal_depth;
		cvmx_write_csr(CVMX_ILK_TXX_CFG0(interface), ilk_txx_cfg0.u64);

		for (i = 0; i < cal_depth; i++) {
			txx_cal_entryx.u64 = 0;
			txx_cal_entryx.s.ctl = pent->ent_ctrl;
			txx_cal_entryx.s.channel = pent->pipe_bpid;

			cvmx_write_csr(CVMX_ILK_TXX_CAL_ENTRYX(i, interface), txx_cal_entryx.u64);
			pent++;
		}
	}

	return 0;
}

/**
 * enable calendar for tx
 *
 * @param interface The identifier of the packet interface to configure and
 *                  use as a ILK interface. cn68xx has 2 interfaces: ilk0 and
 *                  ilk1.
 *
 * @param cal_ena   enable or disable calendar
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_ilk_tx_cal_ena(int interface, unsigned char cal_ena)
{
	int res = -1;
	cvmx_ilk_txx_cfg0_t ilk_txx_cfg0;

	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)) &&
	    !(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		return res;

	if (interface >= CVMX_NUM_ILK_INTF)
		return res;

	/* set the enable */
	ilk_txx_cfg0.u64 = cvmx_read_csr(CVMX_ILK_TXX_CFG0(interface));
	ilk_txx_cfg0.s.cal_ena = cal_ena;
	cvmx_write_csr(CVMX_ILK_TXX_CFG0(interface), ilk_txx_cfg0.u64);
	cvmx_read_csr(CVMX_ILK_TXX_CFG0(interface));
	res = 0;

	return res;
}

/**
 * set up calendar for tx
 *
 * @param interface The identifier of the packet interface to configure and
 *                  use as a ILK interface. cn68xx has 2 interfaces: ilk0 and
 *                  ilk1.
 *
 * @param cal_depth the number of calendar entries
 * @param pent      pointer to calendar entries
 * @param cal_ena   enable or disable calendar
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_ilk_cal_setup_tx(int interface, int cal_depth, cvmx_ilk_cal_entry_t * pent, unsigned char cal_ena)
{
	int res = -1;

	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)) &&
	    !(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		return res;

	if (interface >= CVMX_NUM_ILK_INTF)
		return res;

	res = cvmx_ilk_tx_cal_conf(interface, cal_depth, pent);
	if (res < 0)
		return res;

	res = cvmx_ilk_tx_cal_ena(interface, cal_ena);
	return res;
}

EXPORT_SYMBOL(cvmx_ilk_cal_setup_tx);

//#define CVMX_ILK_STATS_ENA 1
#ifdef CVMX_ILK_STATS_ENA
static void cvmx_ilk_reg_dump_rx(int interface)
{
	int i;
	cvmx_ilk_rxx_cfg0_t ilk_rxx_cfg0;
	cvmx_ilk_rxx_cfg1_t ilk_rxx_cfg1;
	cvmx_ilk_rxx_int_t ilk_rxx_int;
	cvmx_ilk_rxx_jabber_t ilk_rxx_jabber;
	cvmx_ilk_rx_lnex_cfg_t ilk_rx_lnex_cfg;
	cvmx_ilk_rx_lnex_int_t ilk_rx_lnex_int;
	cvmx_ilk_gbl_cfg_t ilk_gbl_cfg;
	cvmx_ilk_ser_cfg_t ilk_ser_cfg;
	cvmx_ilk_rxf_idx_pmap_t ilk_rxf_idx_pmap;
	cvmx_ilk_rxf_mem_pmap_t ilk_rxf_mem_pmap;
	cvmx_ilk_rxx_idx_cal_t ilk_rxx_idx_cal;
	cvmx_ilk_rxx_mem_cal0_t ilk_rxx_mem_cal0;
	cvmx_ilk_rxx_mem_cal1_t ilk_rxx_mem_cal1;

	ilk_rxx_cfg0.u64 = cvmx_read_csr(CVMX_ILK_RXX_CFG0(interface));
	cvmx_dprintf("ilk rxx cfg0: 0x%16lx\n", ilk_rxx_cfg0.u64);

	ilk_rxx_cfg1.u64 = cvmx_read_csr(CVMX_ILK_RXX_CFG1(interface));
	cvmx_dprintf("ilk rxx cfg1: 0x%16lx\n", ilk_rxx_cfg1.u64);

	ilk_rxx_int.u64 = cvmx_read_csr(CVMX_ILK_RXX_INT(interface));
	cvmx_dprintf("ilk rxx int: 0x%16lx\n", ilk_rxx_int.u64);
	cvmx_write_csr(CVMX_ILK_RXX_INT(interface), ilk_rxx_int.u64);

	ilk_rxx_jabber.u64 = cvmx_read_csr(CVMX_ILK_RXX_JABBER(interface));
	cvmx_dprintf("ilk rxx jabber: 0x%16lx\n", ilk_rxx_jabber.u64);

#define LNE_NUM_DBG 4
	for (i = 0; i < LNE_NUM_DBG; i++) {
		ilk_rx_lnex_cfg.u64 = cvmx_read_csr(CVMX_ILK_RX_LNEX_CFG(i));
		cvmx_dprintf("ilk rx lnex cfg lane: %d  0x%16lx\n", i, ilk_rx_lnex_cfg.u64);
	}

	for (i = 0; i < LNE_NUM_DBG; i++) {
		ilk_rx_lnex_int.u64 = cvmx_read_csr(CVMX_ILK_RX_LNEX_INT(i));
		cvmx_dprintf("ilk rx lnex int lane: %d  0x%16lx\n", i, ilk_rx_lnex_int.u64);
		cvmx_write_csr(CVMX_ILK_RX_LNEX_INT(i), ilk_rx_lnex_int.u64);
	}

	ilk_gbl_cfg.u64 = cvmx_read_csr(CVMX_ILK_GBL_CFG);
	cvmx_dprintf("ilk gbl cfg: 0x%16lx\n", ilk_gbl_cfg.u64);

	ilk_ser_cfg.u64 = cvmx_read_csr(CVMX_ILK_SER_CFG);
	cvmx_dprintf("ilk ser cfg: 0x%16lx\n", ilk_ser_cfg.u64);

#define CHAN_NUM_DBG 8
	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		ilk_rxf_idx_pmap.u64 = 0;
		ilk_rxf_idx_pmap.s.index = interface * 256;
		ilk_rxf_idx_pmap.s.inc = 1;
		cvmx_write_csr(CVMX_ILK_RXF_IDX_PMAP, ilk_rxf_idx_pmap.u64);
		for (i = 0; i < CHAN_NUM_DBG; i++) {
			ilk_rxf_mem_pmap.u64 = cvmx_read_csr(CVMX_ILK_RXF_MEM_PMAP);
			cvmx_dprintf("ilk rxf mem pmap chan: %3d  0x%16lx\n", i, ilk_rxf_mem_pmap.u64);
		}
	}
	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		cvmx_ilk_rxx_chax_t rxx_chax;

		for (i = 0; i < CHAN_NUM_DBG; i++) {
			rxx_chax.u64 = cvmx_read_csr(CVMX_ILK_RXX_CHAX(i, interface));
			cvmx_dprintf("ilk chan: %d  pki chan: 0x%x\n", i, rxx_chax.s.port_kind);
		}
	}

#define CAL_NUM_DBG 2
	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		ilk_rxx_idx_cal.u64 = 0;
		ilk_rxx_idx_cal.s.inc = 1;
		cvmx_write_csr(CVMX_ILK_RXX_IDX_CAL(interface), ilk_rxx_idx_cal.u64);
		for (i = 0; i < CAL_NUM_DBG; i++) {
			ilk_rxx_idx_cal.u64 = cvmx_read_csr(CVMX_ILK_RXX_IDX_CAL(interface));
			cvmx_dprintf("ilk rxx idx cal: 0x%16lx\n", ilk_rxx_idx_cal.u64);
	
			ilk_rxx_mem_cal0.u64 = cvmx_read_csr(CVMX_ILK_RXX_MEM_CAL0(interface));
			cvmx_dprintf("ilk rxx mem cal0: 0x%16lx\n", ilk_rxx_mem_cal0.u64);
			ilk_rxx_mem_cal1.u64 = cvmx_read_csr(CVMX_ILK_RXX_MEM_CAL1(interface));
			cvmx_dprintf("ilk rxx mem cal1: 0x%16lx\n", ilk_rxx_mem_cal1.u64);
		}
	}
	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		cvmx_ilk_rxx_cal_entryx_t rxx_cal_entryx;

		for (i = 0; i < CAL_NUM_DBG; i++) {
			rxx_cal_entryx.u64 = cvmx_read_csr(CVMX_ILK_RXX_CAL_ENTRYX(i, interface));
			cvmx_dprintf("ilk rxx cal idx: %d\n", i);
			cvmx_dprintf("ilk rxx cal ctl: 0x%x\n", rxx_cal_entryx.s.ctl);
			cvmx_dprintf("ilk rxx cal pko chan: 0x%x\n", rxx_cal_entryx.s.channel);
		}
	}
}

static void cvmx_ilk_reg_dump_tx(int interface)
{
	int i;
	cvmx_ilk_txx_cfg0_t ilk_txx_cfg0;
	cvmx_ilk_txx_cfg1_t ilk_txx_cfg1;
	cvmx_ilk_txx_idx_pmap_t ilk_txx_idx_pmap;
	cvmx_ilk_txx_mem_pmap_t ilk_txx_mem_pmap;
	cvmx_ilk_txx_int_t ilk_txx_int;
	cvmx_ilk_txx_pipe_t ilk_txx_pipe;
	cvmx_ilk_txx_idx_cal_t ilk_txx_idx_cal;
	cvmx_ilk_txx_mem_cal0_t ilk_txx_mem_cal0;
	cvmx_ilk_txx_mem_cal1_t ilk_txx_mem_cal1;

	ilk_txx_cfg0.u64 = cvmx_read_csr(CVMX_ILK_TXX_CFG0(interface));
	cvmx_dprintf("ilk txx cfg0: 0x%16lx\n", ilk_txx_cfg0.u64);

	ilk_txx_cfg1.u64 = cvmx_read_csr(CVMX_ILK_TXX_CFG1(interface));
	cvmx_dprintf("ilk txx cfg1: 0x%16lx\n", ilk_txx_cfg1.u64);

	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		ilk_txx_pipe.u64 = cvmx_read_csr(CVMX_ILK_TXX_PIPE(interface));
		cvmx_dprintf("ilk txx pipe: 0x%16lx\n", ilk_txx_pipe.u64);

		ilk_txx_idx_pmap.u64 = 0;
		ilk_txx_idx_pmap.s.index = ilk_txx_pipe.s.base;
		ilk_txx_idx_pmap.s.inc = 1;
		cvmx_write_csr(CVMX_ILK_TXX_IDX_PMAP(interface), ilk_txx_idx_pmap.u64);
		for (i = 0; i < CHAN_NUM_DBG; i++) {
			ilk_txx_mem_pmap.u64 = cvmx_read_csr(CVMX_ILK_TXX_MEM_PMAP(interface));
			cvmx_dprintf("ilk txx mem pmap pipe: %3d  0x%16lx\n", ilk_txx_pipe.s.base + i, ilk_txx_mem_pmap.u64);
		}
	}

	ilk_txx_int.u64 = cvmx_read_csr(CVMX_ILK_TXX_INT(interface));
	cvmx_dprintf("ilk txx int: 0x%16lx\n", ilk_txx_int.u64);

	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		ilk_txx_idx_cal.u64 = 0;
		ilk_txx_idx_cal.s.inc = 1;
		cvmx_write_csr(CVMX_ILK_TXX_IDX_CAL(interface), ilk_txx_idx_cal.u64);
		for (i = 0; i < CAL_NUM_DBG; i++) {
			ilk_txx_idx_cal.u64 = cvmx_read_csr(CVMX_ILK_TXX_IDX_CAL(interface));
			cvmx_dprintf("ilk txx idx cal: 0x%16lx\n", ilk_txx_idx_cal.u64);
	
			ilk_txx_mem_cal0.u64 = cvmx_read_csr(CVMX_ILK_TXX_MEM_CAL0(interface));
			cvmx_dprintf("ilk txx mem cal0: 0x%16lx\n", ilk_txx_mem_cal0.u64);
			ilk_txx_mem_cal1.u64 = cvmx_read_csr(CVMX_ILK_TXX_MEM_CAL1(interface));
			cvmx_dprintf("ilk txx mem cal1: 0x%16lx\n", ilk_txx_mem_cal1.u64);
		}
	}
	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		cvmx_ilk_txx_cal_entryx_t txx_cal_entryx;

		for (i = 0; i < CAL_NUM_DBG; i++) {
			txx_cal_entryx.u64 = cvmx_read_csr(CVMX_ILK_TXX_CAL_ENTRYX(i, interface));
			cvmx_dprintf("ilk txx cal idx: %d\n", i);
			cvmx_dprintf("ilk txx cal ctl: 0x%x\n", txx_cal_entryx.s.ctl);
			cvmx_dprintf("ilk txx cal pki chan: 0x%x\n", txx_cal_entryx.s.channel);
		}
	}
}
#endif

/**
 * show run time status
 *
 * @param interface The identifier of the packet interface to enable. cn68xx
 *                  has 2 interfaces: ilk0 and ilk1.
 *
 * @return nothing
 */
#ifdef CVMX_ILK_RUNTIME_DBG
void cvmx_ilk_runtime_status(int interface)
{
	cvmx_ilk_txx_cfg1_t ilk_txx_cfg1;
	cvmx_ilk_txx_flow_ctl0_t ilk_txx_flow_ctl0;
	cvmx_ilk_rxx_cfg1_t ilk_rxx_cfg1;
	cvmx_ilk_rxx_int_t ilk_rxx_int;
	cvmx_ilk_rxx_flow_ctl0_t ilk_rxx_flow_ctl0;
	cvmx_ilk_rxx_flow_ctl1_t ilk_rxx_flow_ctl1;
	cvmx_ilk_gbl_int_t ilk_gbl_int;

	cvmx_dprintf("\nilk run-time status: interface: %d\n", interface);

	ilk_txx_cfg1.u64 = cvmx_read_csr(CVMX_ILK_TXX_CFG1(interface));
	cvmx_dprintf("\nilk txx cfg1: 0x%16lx\n", ilk_txx_cfg1.u64);
	if (ilk_txx_cfg1.s.rx_link_fc)
		cvmx_dprintf("link flow control received\n");
	if (ilk_txx_cfg1.s.tx_link_fc)
		cvmx_dprintf("link flow control sent\n");

	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		ilk_txx_flow_ctl0.u64 = cvmx_read_csr(CVMX_ILK_TXX_FLOW_CTL0(interface));
		cvmx_dprintf("\nilk txx flow ctl0: 0x%16lx\n", ilk_txx_flow_ctl0.u64);
	}
	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		int i;
		cvmx_ilk_txx_cha_xonx_t txx_cha_xonx;

		for (i = 0; i < 4; i++) {
			txx_cha_xonx.u64 = cvmx_read_csr(CVMX_ILK_TXX_CHA_XONX(i, interface));
			cvmx_dprintf("\nilk txx cha xon: 0x%16lx\n", txx_cha_xonx.u64);
		}
	}

	ilk_rxx_cfg1.u64 = cvmx_read_csr(CVMX_ILK_RXX_CFG1(interface));
	cvmx_dprintf("\nilk rxx cfg1: 0x%16lx\n", ilk_rxx_cfg1.u64);
	cvmx_dprintf("rx fifo count: %d\n", ilk_rxx_cfg1.s.rx_fifo_cnt);

	ilk_rxx_int.u64 = cvmx_read_csr(CVMX_ILK_RXX_INT(interface));
	cvmx_dprintf("\nilk rxx int: 0x%16lx\n", ilk_rxx_int.u64);
	if (ilk_rxx_int.s.pkt_drop_rxf)
		cvmx_dprintf("rx fifo packet drop\n");
	if (ilk_rxx_int.u64)
		cvmx_write_csr(CVMX_ILK_RXX_INT(interface), ilk_rxx_int.u64);

	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		ilk_rxx_flow_ctl0.u64 = cvmx_read_csr(CVMX_ILK_RXX_FLOW_CTL0(interface));
		cvmx_dprintf("\nilk rxx flow ctl0: 0x%16lx\n", ilk_rxx_flow_ctl0.u64);

		ilk_rxx_flow_ctl1.u64 = cvmx_read_csr(CVMX_ILK_RXX_FLOW_CTL1(interface));
		cvmx_dprintf("\nilk rxx flow ctl1: 0x%16lx\n", ilk_rxx_flow_ctl1.u64);
	}
	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		int i;
		cvmx_ilk_rxx_cha_xonx_t rxx_cha_xonx;

		for (i = 0; i < 4; i++) {
			rxx_cha_xonx.u64 = cvmx_read_csr(CVMX_ILK_RXX_CHA_XONX(i, interface));
			cvmx_dprintf("\nilk rxx cha xon: 0x%16lx\n", rxx_cha_xonx.u64);
		}
	}

	ilk_gbl_int.u64 = cvmx_read_csr(CVMX_ILK_GBL_INT);
	cvmx_dprintf("\nilk gbl int: 0x%16lx\n", ilk_gbl_int.u64);
	if (ilk_gbl_int.s.rxf_push_full)
		cvmx_dprintf("rx fifo overflow\n");
	if (ilk_gbl_int.u64)
		cvmx_write_csr(CVMX_ILK_GBL_INT, ilk_gbl_int.u64);
}
#endif

/**
 * enable interface
 *
 * @param interface The identifier of the packet interface to enable. cn68xx
 *                  has 2 interfaces: ilk0 and ilk1.
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_ilk_enable(int interface)
{
	int res = -1;
	int retry_count = 0;
	cvmx_helper_link_info_t result;
	cvmx_ilk_txx_cfg1_t ilk_txx_cfg1;
	cvmx_ilk_rxx_cfg1_t ilk_rxx_cfg1;
#ifdef CVMX_ILK_STATS_ENA
	cvmx_ilk_rxx_cfg0_t ilk_rxx_cfg0;
	cvmx_ilk_txx_cfg0_t ilk_txx_cfg0;
#endif

	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)) &&
	    !(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		return res;

	if (interface >= CVMX_NUM_ILK_INTF)
		return res;

	result.u64 = 0;

#ifdef CVMX_ILK_STATS_ENA
	cvmx_dprintf("\n");
	cvmx_dprintf("<<<< ILK%d: Before enabling ilk\n", interface);
	cvmx_ilk_reg_dump_rx(interface);
	cvmx_ilk_reg_dump_tx(interface);
#endif

	/* RX packet will be enabled only if link is up */

	/* TX side */
	ilk_txx_cfg1.u64 = cvmx_read_csr(CVMX_ILK_TXX_CFG1(interface));
	ilk_txx_cfg1.s.pkt_ena = 1;
	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		if (cvmx_ilk_use_la_mode(interface, 0)) {
			ilk_txx_cfg1.s.la_mode = 1;
			ilk_txx_cfg1.s.tx_link_fc_jam = 1;
		}
	}
	cvmx_write_csr(CVMX_ILK_TXX_CFG1(interface), ilk_txx_cfg1.u64);
	cvmx_read_csr(CVMX_ILK_TXX_CFG1(interface));

#ifdef CVMX_ILK_STATS_ENA
	/* RX side stats */
	ilk_rxx_cfg0.u64 = cvmx_read_csr(CVMX_ILK_RXX_CFG0(interface));
	ilk_rxx_cfg0.s.lnk_stats_ena = 1;
	cvmx_write_csr(CVMX_ILK_RXX_CFG0(interface), ilk_rxx_cfg0.u64);

	/* TX side stats */
	ilk_txx_cfg0.u64 = cvmx_read_csr(CVMX_ILK_TXX_CFG0(interface));
	ilk_txx_cfg0.s.lnk_stats_ena = 1;
	cvmx_write_csr(CVMX_ILK_TXX_CFG0(interface), ilk_txx_cfg0.u64);
#endif

retry:
	retry_count++;
	if (retry_count > 10)
		goto out;

	/* Make sure the link is up, so that packets can be sent. */
	result = __cvmx_helper_ilk_link_get(cvmx_helper_get_ipd_port(interface + CVMX_ILK_GBL_BASE(), 0));

	/* Small delay before another retry. */
	cvmx_wait_usec(100);

	ilk_rxx_cfg1.u64 = cvmx_read_csr(CVMX_ILK_RXX_CFG1(interface));
	if (ilk_rxx_cfg1.s.pkt_ena == 0)
		goto retry;

out:

#ifdef CVMX_ILK_STATS_ENA
	cvmx_dprintf(">>>> ILK%d: After ILK is enabled\n", interface);
	cvmx_ilk_reg_dump_rx(interface);
	cvmx_ilk_reg_dump_tx(interface);
#endif

	if (result.s.link_up)
		return 0;

	return -1;
}

/**
 * Disable interface
 *
 * @param interface The identifier of the packet interface to disable. cn68xx
 *                  has 2 interfaces: ilk0 and ilk1.
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_ilk_disable(int interface)
{
	int res = -1;
	cvmx_ilk_txx_cfg1_t ilk_txx_cfg1;
	cvmx_ilk_rxx_cfg1_t ilk_rxx_cfg1;
#ifdef CVMX_ILK_STATS_ENA
	cvmx_ilk_rxx_cfg0_t ilk_rxx_cfg0;
	cvmx_ilk_txx_cfg0_t ilk_txx_cfg0;
#endif

	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)) &&
	    !(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		return res;

	if (interface >= CVMX_NUM_ILK_INTF)
		return res;

	/* TX side */
	ilk_txx_cfg1.u64 = cvmx_read_csr(CVMX_ILK_TXX_CFG1(interface));
	ilk_txx_cfg1.s.pkt_ena = 0;
	cvmx_write_csr(CVMX_ILK_TXX_CFG1(interface), ilk_txx_cfg1.u64);

	/* RX side */
	ilk_rxx_cfg1.u64 = cvmx_read_csr(CVMX_ILK_RXX_CFG1(interface));
	ilk_rxx_cfg1.s.pkt_ena = 0;
	cvmx_write_csr(CVMX_ILK_RXX_CFG1(interface), ilk_rxx_cfg1.u64);

#ifdef CVMX_ILK_STATS_ENA
	/* RX side stats */
	ilk_rxx_cfg0.u64 = cvmx_read_csr(CVMX_ILK_RXX_CFG0(interface));
	ilk_rxx_cfg0.s.lnk_stats_ena = 0;
	cvmx_write_csr(CVMX_ILK_RXX_CFG0(interface), ilk_rxx_cfg0.u64);

	/* RX side stats */
	ilk_txx_cfg0.u64 = cvmx_read_csr(CVMX_ILK_TXX_CFG0(interface));
	ilk_txx_cfg0.s.lnk_stats_ena = 0;
	cvmx_write_csr(CVMX_ILK_TXX_CFG0(interface), ilk_txx_cfg0.u64);
#endif

	return 0;
}

/**
 * Provide interface enable status
 *
 * @param interface The identifier of the packet interface to disable. cn68xx
 *                  has 2 interfaces: ilk0 and ilk1.
 *
 * @return Zero, not enabled; One, enabled.
 */
int cvmx_ilk_get_intf_ena(int interface)
{
	return cvmx_ilk_intf_cfg[interface].intf_en;
}

/**
 * Provide interface lane mask
 *
 * @param interface The identifier of the packet interface to disable. cn68xx
 *                  has 2 interfaces: ilk0 and ilk1.
 *
 * @return lane mask
 */
unsigned char cvmx_ilk_get_intf_ln_msk(int interface)
{
	return cvmx_ilk_intf_cfg[interface].lane_en_mask;
}

/**
 * Provide channel info
 *
 * @param interface The identifier of the packet interface to disable. cn68xx
 *                  has 2 interfaces: ilk0 and ilk1.
 * @param chans    A pointer to a channel array
 * @param num_chan A pointer to the number of channels
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_ilk_get_chan_info(int interface, unsigned char **chans, unsigned char *num_chan)
{
	*chans = cvmx_ilk_chan_map[interface];
	*num_chan = cvmx_ilk_chans[interface];

	return 0;
}

/**
 * Enable or disable LA mode in ILK header.
 * For normal ILK mode, enable CRC and skip = 0.
 * For ILK LA mode, disable CRC and set skip to size of ILK header.
 *
 * @param port   IPD port of the ILK header
 * @param mode   If set, enable LA mode in ILK header, else disable
 *
 * @return ILK header
 */
cvmx_ilk_la_nsp_compact_hdr_t cvmx_ilk_enable_la_header(int port, int mode)
{
	cvmx_ilk_la_nsp_compact_hdr_t ilk_header;
	cvmx_pip_prt_cfgx_t pip_config;
	int interface = cvmx_helper_get_interface_num(port);
	int ilk_interface = interface - CVMX_ILK_GBL_BASE();
	int skip = 0;
	int crc = 1;
	int len_chk = 1;

	ilk_header.u64 = 0;

	if(ilk_interface >= CVMX_NUM_ILK_INTF)
		cvmx_dprintf("ERROR: Invalid interface %d\n",ilk_interface);
	if (!cvmx_ilk_use_la_mode(ilk_interface, 0))
		return ilk_header;

	if (mode) {
		ilk_header.s.la_mode = 1;
		ilk_header.s.ilk_channel = port & 1;
		skip = sizeof(ilk_header);
		crc = 0;
	}
	/* There is a bug in the CN68XX pass 2.x where the CRC erroneously is
	 * computed over the ILK header when it should not be so we ignore it.
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN68XX_PASS2_X)) {
		crc = 0;
		if (!mode)
			len_chk = 0;
		skip = sizeof(ilk_header);
	}

	/* SKIP ILK header only for first 2 ports */
	if ((port & 0x7) < 2) {
		int pknd = cvmx_helper_get_pknd(interface, port & 1);
		int ipko_port;
		cvmx_pko_reg_read_idx_t pko_reg;
		cvmx_pko_mem_iport_ptrs_t pko_mem_iport;
		cvmx_pip_sub_pkind_fcsx_t pknd_fcs;

		/* Enable/Disable CRC in IPD and set skip */
		pip_config.u64 = cvmx_read_csr(CVMX_PIP_PRT_CFGX(pknd));
		pip_config.s.skip = skip;
		pip_config.s.crc_en = crc;
		pip_config.s.lenerr_en = len_chk;
		pip_config.s.minerr_en = (len_chk && !mode);
		cvmx_write_csr(CVMX_PIP_PRT_CFGX(pknd), pip_config.u64);

		pknd_fcs.u64 = cvmx_read_csr(CVMX_PIP_SUB_PKIND_FCSX(0));
		pknd_fcs.s.port_bit &= ~(1ull << pknd);
		cvmx_write_csr(CVMX_PIP_SUB_PKIND_FCSX(0), pknd_fcs.u64);

		/* Enable/Disable CRC in PKO */

		/* Get PKO internal port */
		ipko_port = ilk_interface + 0x1c;

		pko_reg.u64 = cvmx_read_csr(CVMX_PKO_REG_READ_IDX);
		pko_reg.s.index = cvmx_helper_get_pko_port(interface, port & 1);
		cvmx_write_csr(CVMX_PKO_REG_READ_IDX, pko_reg.u64);

		pko_mem_iport.u64 = cvmx_read_csr(CVMX_PKO_MEM_IPORT_PTRS);
		pko_mem_iport.s.crc = crc;
		pko_mem_iport.s.intr = ipko_port;
		cvmx_write_csr(CVMX_PKO_MEM_IPORT_PTRS, pko_mem_iport.u64);
	}

	return ilk_header;
}

/**
 * Show channel statistics
 *
 * @param interface The identifier of the packet interface to disable. cn68xx
 *                  has 2 interfaces: ilk0 and ilk1.
 * @param pstats A pointer to cvmx_ilk_stats_ctrl_t that specifies which
 *               logical channels to access
 *
 * @return nothing
 */
void cvmx_ilk_show_stats(int interface, cvmx_ilk_stats_ctrl_t * pstats)
{
	unsigned int i;
	cvmx_ilk_rxx_idx_stat0_t ilk_rxx_idx_stat0;
	cvmx_ilk_rxx_idx_stat1_t ilk_rxx_idx_stat1;
	cvmx_ilk_rxx_mem_stat0_t ilk_rxx_mem_stat0;
	cvmx_ilk_rxx_mem_stat1_t ilk_rxx_mem_stat1;

	cvmx_ilk_txx_idx_stat0_t ilk_txx_idx_stat0;
	cvmx_ilk_txx_idx_stat1_t ilk_txx_idx_stat1;
	cvmx_ilk_txx_mem_stat0_t ilk_txx_mem_stat0;
	cvmx_ilk_txx_mem_stat1_t ilk_txx_mem_stat1;

	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)) &&
	    !(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		return;

	if (interface >= CVMX_NUM_ILK_INTF)
		return;

	if (pstats == NULL)
		return;

	/* discrete channels */
	if (pstats->chan_list != NULL) {
		for (i = 0; i < pstats->num_chans; i++) {

			if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
				/* get the number of rx packets */
				ilk_rxx_idx_stat0.u64 = 0;
				ilk_rxx_idx_stat0.s.index = *pstats->chan_list;
				ilk_rxx_idx_stat0.s.clr = pstats->clr_on_rd;
				cvmx_write_csr(CVMX_ILK_RXX_IDX_STAT0(interface), ilk_rxx_idx_stat0.u64);
				ilk_rxx_mem_stat0.u64 = cvmx_read_csr(CVMX_ILK_RXX_MEM_STAT0(interface));
	
				/* get the number of rx bytes */
				ilk_rxx_idx_stat1.u64 = 0;
				ilk_rxx_idx_stat1.s.index = *pstats->chan_list;
				ilk_rxx_idx_stat1.s.clr = pstats->clr_on_rd;
				cvmx_write_csr(CVMX_ILK_RXX_IDX_STAT1(interface), ilk_rxx_idx_stat1.u64);
				ilk_rxx_mem_stat1.u64 = cvmx_read_csr(CVMX_ILK_RXX_MEM_STAT1(interface));

				cvmx_dprintf("ILK%d Channel%d Rx: %d packets %d bytes\n", interface,
					     *pstats->chan_list, ilk_rxx_mem_stat0.s.rx_pkt, (unsigned int)ilk_rxx_mem_stat1.s.rx_bytes);
			}
			if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
				cvmx_ilk_rxx_pkt_cntx_t rxx_pkt_cntx;
				cvmx_ilk_rxx_byte_cntx_t rxx_byte_cntx;

				rxx_pkt_cntx.u64 = cvmx_read_csr(CVMX_ILK_RXX_PKT_CNTX(*pstats->chan_list, interface));
				rxx_byte_cntx.u64 = cvmx_read_csr(CVMX_ILK_RXX_BYTE_CNTX(*pstats->chan_list, interface));
				cvmx_dprintf("ILK%d Channel%d Rx: %llu packets %llu bytes\n", interface,
					     *pstats->chan_list, 
					     (unsigned long long)rxx_pkt_cntx.s.rx_pkt,
					     (unsigned long long)rxx_byte_cntx.s.rx_bytes);
			}

			if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
				/* get the number of tx packets */
				ilk_txx_idx_stat0.u64 = 0;
				ilk_txx_idx_stat0.s.index = *pstats->chan_list;
				ilk_txx_idx_stat0.s.clr = pstats->clr_on_rd;
				cvmx_write_csr(CVMX_ILK_TXX_IDX_STAT0(interface), ilk_txx_idx_stat0.u64);
				ilk_txx_mem_stat0.u64 = cvmx_read_csr(CVMX_ILK_TXX_MEM_STAT0(interface));
	
				/* get the number of tx bytes */
				ilk_txx_idx_stat1.u64 = 0;
				ilk_txx_idx_stat1.s.index = *pstats->chan_list;
				ilk_txx_idx_stat1.s.clr = pstats->clr_on_rd;
				cvmx_write_csr(CVMX_ILK_TXX_IDX_STAT1(interface), ilk_txx_idx_stat1.u64);
				ilk_txx_mem_stat1.u64 = cvmx_read_csr(CVMX_ILK_TXX_MEM_STAT1(interface));
	
				cvmx_dprintf("ILK%d Channel%d Tx: %d packets %d bytes\n", interface,
					     *pstats->chan_list, ilk_txx_mem_stat0.s.tx_pkt, (unsigned int)ilk_txx_mem_stat1.s.tx_bytes);
			}
			if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
				cvmx_ilk_txx_pkt_cntx_t txx_pkt_cntx;
				cvmx_ilk_txx_byte_cntx_t txx_byte_cntx;

				txx_pkt_cntx.u64 = cvmx_read_csr(CVMX_ILK_TXX_PKT_CNTX(*pstats->chan_list, interface));
				txx_byte_cntx.u64 = cvmx_read_csr(CVMX_ILK_TXX_BYTE_CNTX(*pstats->chan_list, interface));
				cvmx_dprintf("ILK%d Channel%d Tx: %llu packets %llu bytes\n", interface,
					     *pstats->chan_list,
					     (unsigned long long)txx_pkt_cntx.s.tx_pkt,
					     (unsigned long long)txx_byte_cntx.s.tx_bytes);
			}

			pstats++;
		}
		return;
	}

	/* continuous channels */
	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		ilk_rxx_idx_stat0.u64 = 0;
		ilk_rxx_idx_stat0.s.index = pstats->chan_start;
		ilk_rxx_idx_stat0.s.inc = pstats->chan_step;
		ilk_rxx_idx_stat0.s.clr = pstats->clr_on_rd;
		cvmx_write_csr(CVMX_ILK_RXX_IDX_STAT0(interface), ilk_rxx_idx_stat0.u64);
	
		ilk_rxx_idx_stat1.u64 = 0;
		ilk_rxx_idx_stat1.s.index = pstats->chan_start;
		ilk_rxx_idx_stat1.s.inc = pstats->chan_step;
		ilk_rxx_idx_stat1.s.clr = pstats->clr_on_rd;
		cvmx_write_csr(CVMX_ILK_RXX_IDX_STAT1(interface), ilk_rxx_idx_stat1.u64);
	
		ilk_txx_idx_stat0.u64 = 0;
		ilk_txx_idx_stat0.s.index = pstats->chan_start;
		ilk_txx_idx_stat0.s.inc = pstats->chan_step;
		ilk_txx_idx_stat0.s.clr = pstats->clr_on_rd;
		cvmx_write_csr(CVMX_ILK_TXX_IDX_STAT0(interface), ilk_txx_idx_stat0.u64);
	
		ilk_txx_idx_stat1.u64 = 0;
		ilk_txx_idx_stat1.s.index = pstats->chan_start;
		ilk_txx_idx_stat1.s.inc = pstats->chan_step;
		ilk_txx_idx_stat1.s.clr = pstats->clr_on_rd;
		cvmx_write_csr(CVMX_ILK_TXX_IDX_STAT1(interface), ilk_txx_idx_stat1.u64);
	
		for (i = pstats->chan_start; i <= pstats->chan_end; i += pstats->chan_step) {
			ilk_rxx_mem_stat0.u64 = cvmx_read_csr(CVMX_ILK_RXX_MEM_STAT0(interface));
			ilk_rxx_mem_stat1.u64 = cvmx_read_csr(CVMX_ILK_RXX_MEM_STAT1(interface));
			cvmx_dprintf("ILK%d Channel%d Rx: %d packets %d bytes\n", interface, i, ilk_rxx_mem_stat0.s.rx_pkt, (unsigned int)ilk_rxx_mem_stat1.s.rx_bytes);
	
			ilk_txx_mem_stat0.u64 = cvmx_read_csr(CVMX_ILK_TXX_MEM_STAT0(interface));
			ilk_txx_mem_stat1.u64 = cvmx_read_csr(CVMX_ILK_TXX_MEM_STAT1(interface));
			cvmx_dprintf("ILK%d Channel%d Tx: %d packets %d bytes\n", interface, i, ilk_rxx_mem_stat0.s.rx_pkt, (unsigned int)ilk_rxx_mem_stat1.s.rx_bytes);
		}
	}
	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		for (i = pstats->chan_start; i <= pstats->chan_end; i += pstats->chan_step) {
			cvmx_ilk_rxx_pkt_cntx_t rxx_pkt_cntx;
			cvmx_ilk_rxx_byte_cntx_t rxx_byte_cntx;
			cvmx_ilk_txx_pkt_cntx_t txx_pkt_cntx;
			cvmx_ilk_txx_byte_cntx_t txx_byte_cntx;

			rxx_pkt_cntx.u64 = cvmx_read_csr(CVMX_ILK_RXX_PKT_CNTX(i, interface));
			rxx_byte_cntx.u64 = cvmx_read_csr(CVMX_ILK_RXX_BYTE_CNTX(i, interface));
			cvmx_dprintf("ILK%d Channel%d Rx: %llu packets %llu bytes\n", interface,
				     i, (unsigned long long)rxx_pkt_cntx.s.rx_pkt,
				     (unsigned long long)rxx_byte_cntx.s.rx_bytes);

			txx_pkt_cntx.u64 = cvmx_read_csr(CVMX_ILK_TXX_PKT_CNTX(i, interface));
			txx_byte_cntx.u64 = cvmx_read_csr(CVMX_ILK_TXX_BYTE_CNTX(i, interface));
			cvmx_dprintf("ILK%d Channel%d Tx: %llu packets %llu bytes\n", interface,
				     i, (unsigned long long)txx_pkt_cntx.s.tx_pkt,
				     (unsigned long long)txx_byte_cntx.s.tx_bytes);
		}
	}

	return;
}

/**
 * enable or disable loopbacks
 *
 * @param interface The identifier of the packet interface to disable. cn68xx
 *                  has 2 interfaces: ilk0 and ilk1.
 * @param enable    Enable or disable loopback
 * @param mode      Internal or external loopback
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_ilk_lpbk(int interface, cvmx_ilk_lpbk_ena_t enable, cvmx_ilk_lpbk_mode_t mode)
{
	int res = -1;
	cvmx_ilk_txx_cfg0_t ilk_txx_cfg0;
	cvmx_ilk_rxx_cfg0_t ilk_rxx_cfg0;

	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)) &&
	    !(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		return res;

	if (interface >= CVMX_NUM_ILK_INTF)
		return res;

	/* internal loopback. only 1 type of loopback can be on at 1 time */
	if (mode == CVMX_ILK_LPBK_INT) {
		if (enable == CVMX_ILK_LPBK_ENA) {
			ilk_txx_cfg0.u64 = cvmx_read_csr(CVMX_ILK_TXX_CFG0(interface));
			ilk_txx_cfg0.s.ext_lpbk = CVMX_ILK_LPBK_DISA;
			ilk_txx_cfg0.s.ext_lpbk_fc = CVMX_ILK_LPBK_DISA;
			cvmx_write_csr(CVMX_ILK_TXX_CFG0(interface), ilk_txx_cfg0.u64);

			ilk_rxx_cfg0.u64 = cvmx_read_csr(CVMX_ILK_RXX_CFG0(interface));
			ilk_rxx_cfg0.s.ext_lpbk = CVMX_ILK_LPBK_DISA;
			ilk_rxx_cfg0.s.ext_lpbk_fc = CVMX_ILK_LPBK_DISA;
			cvmx_write_csr(CVMX_ILK_RXX_CFG0(interface), ilk_rxx_cfg0.u64);
		}

		ilk_txx_cfg0.u64 = cvmx_read_csr(CVMX_ILK_TXX_CFG0(interface));
		ilk_txx_cfg0.s.int_lpbk = enable;
		cvmx_write_csr(CVMX_ILK_TXX_CFG0(interface), ilk_txx_cfg0.u64);

		res = 0;
		return res;
	}

	/* external loopback. only 1 type of loopback can be on at 1 time */
	if (enable == CVMX_ILK_LPBK_ENA) {
		ilk_txx_cfg0.u64 = cvmx_read_csr(CVMX_ILK_TXX_CFG0(interface));
		ilk_txx_cfg0.s.int_lpbk = CVMX_ILK_LPBK_DISA;
		cvmx_write_csr(CVMX_ILK_TXX_CFG0(interface), ilk_txx_cfg0.u64);
	}

	ilk_txx_cfg0.u64 = cvmx_read_csr(CVMX_ILK_TXX_CFG0(interface));
	ilk_txx_cfg0.s.ext_lpbk = enable;
	ilk_txx_cfg0.s.ext_lpbk_fc = enable;
	cvmx_write_csr(CVMX_ILK_TXX_CFG0(interface), ilk_txx_cfg0.u64);

	ilk_rxx_cfg0.u64 = cvmx_read_csr(CVMX_ILK_RXX_CFG0(interface));
	ilk_rxx_cfg0.s.ext_lpbk = enable;
	ilk_rxx_cfg0.s.ext_lpbk_fc = enable;
	cvmx_write_csr(CVMX_ILK_RXX_CFG0(interface), ilk_rxx_cfg0.u64);

	res = 0;
	return res;
}
