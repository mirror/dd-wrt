/***********************license start***************
 * Copyright (c) 2003-2012  Cavium Inc. (support@cavium.com). All rights
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
 * Small helper utilities.
 *
 */
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <linux/slab.h>
#include <linux/export.h>

#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-pip.h>
#include <asm/octeon/cvmx-ipd.h>
#include <asm/octeon/cvmx-helper.h>
#include <asm/octeon/cvmx-gmxx-defs.h>
#include <asm/octeon/cvmx-pko-defs.h>
#include <asm/octeon/cvmx-pko.h>
#include <asm/octeon/cvmx-sli-defs.h>
#include <asm/octeon/cvmx-pexp-defs.h>
#include <asm/octeon/cvmx-helper-cfg.h>
#include <asm/octeon/cvmx-ilk.h>
#include <asm/octeon/cvmx-pki.h>
#else
#include "cvmx.h"
#include "cvmx-bootmem.h"
#include "cvmx-fpa.h"
#include "cvmx-pip.h"
#include "cvmx-pko.h"
#include "cvmx-ilk.h"
#include "cvmx-ipd.h"
#include "cvmx-spi.h"
#include "cvmx-sysinfo.h"
#include "cvmx-helper.h"
#include "cvmx-helper-util.h"
#include "cvmx-version.h"
#include "cvmx-helper-ilk.h"
#include "cvmx-helper-cfg.h"
#include "cvmx-pki.h"
#endif


/**
 * @INTERNAL
 * These are the interface types needed to convert interface numbers to ipd
 * ports.
 *
 * @param GMII
 *	This type is used for sgmii, rgmii, xaui and rxaui interfaces.
 * @param ILK
 *	This type is used for ilk interfaces.
 * @param NPI
 *	This type is used for npi interfaces.
 * @param LB
 *	This type is used for loopback interfaces.
 * @param INVALID_IF_TYPE
 *	This type indicates the interface hasn't been configured.
 */
typedef enum {
	GMII,
	ILK,
	NPI,
	LB,
	INVALID_IF_TYPE
} port_map_if_type_t;

/**
 * @INTERNAL
 * This structure is used to map interface numbers to ipd ports.
 *
 * @param type
 *	Interface type
 * @param first_ipd_port
 *	First IPD port number assigned to this interface.
 * @param last_ipd_port
 *	Last IPD port number assigned to this interface.
 * @param ipd_port_adj
 *	Different octeon chips require different ipd ports for the
 *	same interface port/mode configuration. This value is used
 *	to account for that difference.
 */
struct ipd_port_map {
	port_map_if_type_t	type;
	int			first_ipd_port;
	int			last_ipd_port;
	int			ipd_port_adj;
};

/**
 * @INTERNAL
 * Interface number to ipd port map for the octeon 68xx.
 */
static const struct ipd_port_map ipd_port_map_68xx[CVMX_HELPER_MAX_IFACE] = {
	{GMII,	0x800,	0x8ff,	0x40},		/* Interface 0 */
	{GMII,	0x900,	0x9ff,	0x40},		/* Interface 1 */
	{GMII,	0xa00,	0xaff,	0x40},		/* Interface 2 */
	{GMII,	0xb00,	0xbff,	0x40},		/* Interface 3 */
	{GMII,	0xc00,	0xcff,	0x40},		/* Interface 4 */
	{ILK,	0x400,	0x4ff,	0x00},		/* Interface 5 */
	{ILK,	0x500,	0x5ff,	0x00},		/* Interface 6 */
	{NPI,	0x100,	0x120,	0x00},		/* Interface 7 */
	{LB,	0x000,	0x008,	0x00},		/* Interface 8 */
};

/**
 * @INTERNAL
 * Interface number to ipd port map for the octeon 78xx.
 */
static const struct ipd_port_map ipd_port_map_78xx[CVMX_HELPER_MAX_IFACE] = {
	{GMII,	0x800,	0x8ff,	0x00},		/* Interface 0 */
	{GMII,	0x900,	0x9ff,	0x00},		/* Interface 1 */
	{GMII,	0xa00,	0xaff,	0x00},		/* Interface 2 */
	{GMII,	0xb00,	0xbff,	0x00},		/* Interface 3 */
	{GMII,	0xc00,	0xcff,	0x00},		/* Interface 4 */
	{GMII,	0xd00,	0xdff,	0x00},		/* Interface 5 */
	{ILK,	0x400,	0x4ff,	0x00},		/* Interface 6 */
	{ILK,	0x500,	0x5ff,	0x00},		/* Interface 7 */
	{NPI,	0x100,	0x120,	0x00},		/* Interface 8 */
	{LB,	0x000,	0x008,	0x00},		/* Interface 9 */
};

struct cvmx_iface {
	int cvif_ipd_nports;
	int cvif_has_fcs;	/* PKO fcs for this interface. */
	enum cvmx_pko_padding cvif_padding;
	cvmx_helper_link_info_t *cvif_ipd_port_link_info;
};

/*
 * This has to be static as u-boot expects to probe an interface and
 * gets the number of its ports.
 */
static CVMX_SHARED struct cvmx_iface cvmx_interfaces[CVMX_HELPER_MAX_IFACE];

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
/**
 * Get the version of the CVMX libraries.
 *
 * @return Version string. Note this buffer is allocated statically
 *         and will be shared by all callers.
 */
const char *cvmx_helper_get_version(void)
{
	return OCTEON_SDK_VERSION_STRING;
}
#endif

/**
 * Convert a interface mode into a human readable string
 *
 * @param mode   Mode to convert
 *
 * @return String
 */
const char *cvmx_helper_interface_mode_to_string(cvmx_helper_interface_mode_t mode)
{
	switch (mode) {
	case CVMX_HELPER_INTERFACE_MODE_DISABLED:
		return "DISABLED";
	case CVMX_HELPER_INTERFACE_MODE_RGMII:
		return "RGMII";
	case CVMX_HELPER_INTERFACE_MODE_GMII:
		return "GMII";
	case CVMX_HELPER_INTERFACE_MODE_SPI:
		return "SPI";
	case CVMX_HELPER_INTERFACE_MODE_PCIE:
		return "PCIE";
	case CVMX_HELPER_INTERFACE_MODE_XAUI:
		return "XAUI";
	case CVMX_HELPER_INTERFACE_MODE_RXAUI:
		return "RXAUI";
	case CVMX_HELPER_INTERFACE_MODE_SGMII:
		return "SGMII";
	case CVMX_HELPER_INTERFACE_MODE_QSGMII:
		return "QSGMII";
	case CVMX_HELPER_INTERFACE_MODE_PICMG:
		return "PICMG";
	case CVMX_HELPER_INTERFACE_MODE_NPI:
		return "NPI";
	case CVMX_HELPER_INTERFACE_MODE_LOOP:
		return "LOOP";
	case CVMX_HELPER_INTERFACE_MODE_SRIO:
		return "SRIO";
	case CVMX_HELPER_INTERFACE_MODE_ILK:
		return "ILK";
	case CVMX_HELPER_INTERFACE_MODE_AGL:
		return "AGL";
	}
	return "UNKNOWN";
}

/**
 * Debug routine to dump the packet structure to the console
 *
 * @param work   Work queue entry containing the packet to dump
 * @return
 */
int cvmx_helper_dump_packet(cvmx_wqe_t *work)
{
	uint64_t count;
	uint64_t remaining_bytes;
	union cvmx_buf_ptr buffer_ptr;
	uint64_t start_of_buffer;
	uint8_t *data_address;
	uint8_t *end_of_data;

	cvmx_dprintf("WORD0 = %lx\n", (unsigned long)work->word0.u64);
	cvmx_dprintf("WORD1 = %lx\n", (unsigned long)work->word1.u64);
	cvmx_dprintf("WORD2 = %lx\n", (unsigned long)work->word2.u64);
	cvmx_dprintf("Packet Length:   %u\n", cvmx_wqe_get_len(work));
	cvmx_dprintf("    Input Port:  %u\n", cvmx_wqe_get_port(work));
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_dprintf("    pkind:       %u\n", work->word0.pki.pknd);
		cvmx_dprintf("    Style:       %u\n", cvmx_wqe_get_style(work));
		cvmx_dprintf("    Aura:        %u\n", cvmx_wqe_get_aura(work));
		cvmx_dprintf("    Group:       %u\n", cvmx_wqe_get_grp(work));
	} else
		cvmx_dprintf("    QoS:         %u\n", cvmx_wqe_get_qos(work));
	cvmx_dprintf("    Buffers:     %u\n", cvmx_wqe_get_bufs(work));
	if (cvmx_wqe_get_bufs(work) == 0) {
		cvmx_ipd_wqe_fpa_queue_t wqe_pool;
		wqe_pool.u64 = cvmx_read_csr(CVMX_IPD_WQE_FPA_QUEUE);
		buffer_ptr.u64 = 0;
		buffer_ptr.s.pool = wqe_pool.s.wqe_pool;
		buffer_ptr.s.size = 128;
		buffer_ptr.s.addr = cvmx_ptr_to_phys(work->packet_data);
		if (cvmx_likely(!work->word2.s.not_IP)) {
			union cvmx_pip_ip_offset pip_ip_offset;
			pip_ip_offset.u64 = cvmx_read_csr(CVMX_PIP_IP_OFFSET);
			buffer_ptr.s.addr += (pip_ip_offset.s.offset << 3) - work->word2.s.ip_offset;
			buffer_ptr.s.addr += (work->word2.s.is_v6 ^ 1) << 2;
		} else {
			/*
			 * WARNING: This code assume that the packet
			 * is not RAW. If it was, we would use
			 * PIP_GBL_CFG[RAW_SHF] instead of
			 * PIP_GBL_CFG[NIP_SHF].
			 */
			union cvmx_pip_gbl_cfg pip_gbl_cfg;
			pip_gbl_cfg.u64 = cvmx_read_csr(CVMX_PIP_GBL_CFG);
			buffer_ptr.s.addr += pip_gbl_cfg.s.nip_shf;
		}
	} else {
		buffer_ptr = work->packet_ptr;
	}
	remaining_bytes = cvmx_wqe_get_len(work);

	while (remaining_bytes) {
		if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
			start_of_buffer = (buffer_ptr.s_cn78xx.addr >> 7) << 7;
			cvmx_dprintf("    Buffer Start:%llx\n", (unsigned long long)start_of_buffer);
			cvmx_dprintf("    Buffer Data: %llx\n", (unsigned long long)buffer_ptr.s_cn78xx.addr);
			cvmx_dprintf("    Buffer Size: %u\n", buffer_ptr.s_cn78xx.size);
			data_address = (uint8_t *) cvmx_phys_to_ptr(buffer_ptr.s_cn78xx.addr);
			end_of_data = data_address + buffer_ptr.s_cn78xx.size;
		} else {
			start_of_buffer = ((buffer_ptr.s.addr >> 7) - buffer_ptr.s.back) << 7;
			cvmx_dprintf("    Buffer Start:%llx\n", (unsigned long long)start_of_buffer);
			cvmx_dprintf("    Buffer I   : %u\n", buffer_ptr.s.i);
			cvmx_dprintf("    Buffer Back: %u\n", buffer_ptr.s.back);
			cvmx_dprintf("    Buffer Pool: %u\n", buffer_ptr.s.pool);
			cvmx_dprintf("    Buffer Data: %llx\n", (unsigned long long)buffer_ptr.s.addr);
			cvmx_dprintf("    Buffer Size: %u\n", buffer_ptr.s.size);
			data_address = (uint8_t *) cvmx_phys_to_ptr(buffer_ptr.s.addr);
			end_of_data = data_address + buffer_ptr.s.size;
		}

		cvmx_dprintf("\t\t");
		count = 0;
		while (data_address < end_of_data) {
			if (remaining_bytes == 0)
				break;
			else
				remaining_bytes--;
			cvmx_dprintf("%02x", (unsigned int)*data_address);
			data_address++;
			if (remaining_bytes && (count == 7)) {
				cvmx_dprintf("\n\t\t");
				count = 0;
			} else {
				count++;
			}
		}
		cvmx_dprintf("\n");

		if (remaining_bytes) {
			if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
				buffer_ptr = *(cvmx_buf_ptr_t *) cvmx_phys_to_ptr(buffer_ptr.s_cn78xx.addr - 8);
			else
				buffer_ptr = *(cvmx_buf_ptr_t *) cvmx_phys_to_ptr(buffer_ptr.s.addr - 8);
		}
	}
	return 0;
}

/**
 * Setup Random Early Drop on a specific input queue
 *
 * @param queue  Input queue to setup RED on (0-7)
 * @param pass_thresh
 *               Packets will begin slowly dropping when there are less than
 *               this many packet buffers free in FPA 0.
 * @param drop_thresh
 *               All incomming packets will be dropped when there are less
 *               than this many free packet buffers in FPA 0.
 * @return Zero on success. Negative on failure
 */
int cvmx_helper_setup_red_queue(int queue, int pass_thresh, int drop_thresh)
{
	union cvmx_ipd_qosx_red_marks red_marks;
	union cvmx_ipd_red_quex_param red_param;

	/*
	 * Set RED to begin dropping packets when there are
	 * pass_thresh buffers left. It will linearly drop more
	 * packets until reaching drop_thresh buffers.
	 */
	red_marks.u64 = 0;
	red_marks.s.drop = drop_thresh;
	red_marks.s.pass = pass_thresh;
	cvmx_write_csr(CVMX_IPD_QOSX_RED_MARKS(queue), red_marks.u64);

	/* Use the actual queue 0 counter, not the average */
	red_param.u64 = 0;
	red_param.s.prb_con = (255ul << 24) / (red_marks.s.pass - red_marks.s.drop);
	red_param.s.avg_con = 1;
	red_param.s.new_con = 255;
	red_param.s.use_pcnt = 1;
	cvmx_write_csr(CVMX_IPD_RED_QUEX_PARAM(queue), red_param.u64);
	return 0;
}

/**
 * Setup Random Early Drop to automatically begin dropping packets.
 *
 * @param pass_thresh
 *               Packets will begin slowly dropping when there are less than
 *               this many packet buffers free in FPA 0.
 * @param drop_thresh
 *               All incomming packets will be dropped when there are less
 *               than this many free packet buffers in FPA 0.
 * @return Zero on success. Negative on failure
 */
int cvmx_helper_setup_red(int pass_thresh, int drop_thresh)
{
	int queue;
	int interface;
	int port;

	/*vinita_to_do modify for 78xx*/
	if (!OCTEON_IS_MODEL(OCTEON_CN78XX)) {
	/*
	 * Disable backpressure based on queued buffers. It needs SW support
	 */
	if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
		int bpid;
		for (interface = 0; interface < CVMX_HELPER_MAX_GMX; interface++) {
			int num_ports;

			num_ports = cvmx_helper_ports_on_interface(interface);
			for (port = 0; port < num_ports; port++) {
				bpid = cvmx_helper_get_bpid(interface, port);
				if (bpid == CVMX_INVALID_BPID)
					cvmx_dprintf("setup_red: cvmx_helper_get_bpid(%d, %d) = %d\n",
						     interface, port,
						     cvmx_helper_get_bpid(interface, port));
				else
					cvmx_write_csr(CVMX_IPD_BPIDX_MBUF_TH(bpid), 0);
			}
		}
	} else {
		union cvmx_ipd_portx_bp_page_cnt page_cnt;

		page_cnt.u64 = 0;
		page_cnt.s.bp_enb = 0;
		page_cnt.s.page_cnt = 100;
		for (interface = 0; interface < CVMX_HELPER_MAX_GMX; interface++) {
			for (port = cvmx_helper_get_first_ipd_port(interface); port < cvmx_helper_get_last_ipd_port(interface); port++)
				cvmx_write_csr(CVMX_IPD_PORTX_BP_PAGE_CNT(port), page_cnt.u64);
		}
	}

	for (queue = 0; queue < 8; queue++)
		cvmx_helper_setup_red_queue(queue, pass_thresh, drop_thresh);

	/*
	 * Shutoff the dropping based on the per port page count. SW isn't
	 * decrementing it right now
	 */
	if (octeon_has_feature(OCTEON_FEATURE_PKND))
		cvmx_write_csr(CVMX_IPD_ON_BP_DROP_PKTX(0), 0);
	else
		cvmx_write_csr(CVMX_IPD_BP_PRT_RED_END, 0);

#define IPD_RED_AVG_DLY	1000
#define IPD_RED_PRB_DLY	1000
	/*
	 * Setting up avg_dly and prb_dly, enable bits
	 */
	if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
		union cvmx_ipd_red_delay red_delay;
		union cvmx_ipd_red_bpid_enablex red_bpid_enable;

		red_delay.u64 = 0;
		red_delay.s.avg_dly = IPD_RED_AVG_DLY;
		red_delay.s.prb_dly = IPD_RED_PRB_DLY;
		cvmx_write_csr(CVMX_IPD_RED_DELAY, red_delay.u64);

		/*
		 * Only enable the gmx ports
		 */
		red_bpid_enable.u64 = 0;
		for (interface = 0; interface < CVMX_HELPER_MAX_GMX; interface++) {
			int num_ports = cvmx_helper_ports_on_interface(interface);
			for (port = 0; port < num_ports; port++)
				red_bpid_enable.u64 |= (((uint64_t) 1) << cvmx_helper_get_bpid(interface, port));
		}
		cvmx_write_csr(CVMX_IPD_RED_BPID_ENABLEX(0), red_bpid_enable.u64);
	} else {
		union cvmx_ipd_red_port_enable red_port_enable;

		red_port_enable.u64 = 0;
		red_port_enable.s.prt_enb = 0xfffffffffull;
		red_port_enable.s.avg_dly = IPD_RED_AVG_DLY;
		red_port_enable.s.prb_dly = IPD_RED_PRB_DLY;
		cvmx_write_csr(CVMX_IPD_RED_PORT_ENABLE, red_port_enable.u64);

		/*
		 * Shutoff the dropping of packets based on RED for SRIO ports
		 */
		if (octeon_has_feature(OCTEON_FEATURE_SRIO)) {
			union cvmx_ipd_red_port_enable2 red_port_enable2;
			red_port_enable2.u64 = 0;
			red_port_enable2.s.prt_enb = 0xf0;
			cvmx_write_csr(CVMX_IPD_RED_PORT_ENABLE2, red_port_enable2.u64);
		}
	}
	}

	return 0;
}
EXPORT_SYMBOL(cvmx_helper_setup_red);

/**
 * This function sets up aura QOS for RED, backpressure and tail-drop.
 *
 * @param node       node number.
 * @param aura       aura to configure.
 * @param ena_red       enable RED based on [DROP] and [PASS] levels
 *			1: enable 0:disable
 * @param pass_thresh   pass threshold for RED.
 * @param drop_thresh   drop threshold for RED
 * @param ena_bp        enable backpressure based on [BP] level.
 *			1:enable 0:disable
 * @param bp_thresh     backpressure threshold.
 * @param ena_drop      enable tail drop.
 *			1:enable 0:disable
 * @return Zero on success. Negative on failure
 */
int cvmx_helper_setup_aura_qos(int node, int aura, bool ena_red, bool ena_drop,
			       uint64_t pass_thresh, uint64_t drop_thresh,
			       bool ena_bp, uint64_t bp_thresh)
{
	ena_red = ena_red | ena_drop;
	cvmx_fpa_setup_aura_qos(node, aura, ena_red, pass_thresh, drop_thresh,
				ena_bp, bp_thresh);
	cvmx_pki_enable_aura_qos(node, aura, ena_red, ena_drop, ena_bp);
	return 0;
}

/**
 * This function maps specified bpid to all the auras from which it can receive bp and
 * then maps that bpid to all the channels, that bpid can asserrt bp on.
 *
 * @param node          node number.
 * @param aura_map      array of auras to map to that bpid.
 * @param aura_cnt      number of auras to map to the bpid
 * @param chl_map       array of channels to map to that bpid.
 * @param chl_cnt       number of channels to map to the bpid
 * @param bpid          bpid to map
 * @return Zero on success. Negative on failure
 */
int cvmx_helper_map_aura_channel_bpid(int node, int aura_map[], int aura_cnt,
				      int chl_map[], int chl_cnt, int bpid)
{
	while (aura_cnt--) {
		cvmx_pki_write_aura_bpid(node, aura_map[aura_cnt], bpid);
	}
	while (chl_cnt--) {
		cvmx_pki_write_channel_bpid(node, chl_map[chl_cnt], bpid);
	}
	return 0;
}

/**
 * @INTERNAL
 * Setup the common GMX settings that determine the number of
 * ports. These setting apply to almost all configurations of all
 * chips.
 *
 * @param interface Interface to configure
 * @param num_ports Number of ports on the interface
 *
 * @return Zero on success, negative on failure
 */
int __cvmx_helper_setup_gmx(int interface, int num_ports)
{
	union cvmx_gmxx_tx_prts gmx_tx_prts;
	union cvmx_gmxx_rx_prts gmx_rx_prts;
	union cvmx_pko_reg_gmx_port_mode pko_mode;
	union cvmx_gmxx_txx_thresh gmx_tx_thresh;
	int index;

	/* Tell GMX the number of TX ports on this interface */
	gmx_tx_prts.u64 = cvmx_read_csr(CVMX_GMXX_TX_PRTS(interface));
	gmx_tx_prts.s.prts = num_ports;
	cvmx_write_csr(CVMX_GMXX_TX_PRTS(interface), gmx_tx_prts.u64);

	/*
	 * Tell GMX the number of RX ports on this interface.  This only applies
	 * to *GMII and XAUI ports.
	 */
	switch (cvmx_helper_interface_get_mode(interface)) {
	case CVMX_HELPER_INTERFACE_MODE_RGMII:
	case CVMX_HELPER_INTERFACE_MODE_SGMII:
	case CVMX_HELPER_INTERFACE_MODE_QSGMII:
	case CVMX_HELPER_INTERFACE_MODE_GMII:
	case CVMX_HELPER_INTERFACE_MODE_XAUI:
	case CVMX_HELPER_INTERFACE_MODE_RXAUI:
		if (num_ports > 4) {
			cvmx_dprintf("__cvmx_helper_setup_gmx: Illegal num_ports\n");
			return -1;
		}

		gmx_rx_prts.u64 = cvmx_read_csr(CVMX_GMXX_RX_PRTS(interface));
		gmx_rx_prts.s.prts = num_ports;
		cvmx_write_csr(CVMX_GMXX_RX_PRTS(interface), gmx_rx_prts.u64);
		break;

	default:
		break;
	}

	/*
	 * Skip setting CVMX_PKO_REG_GMX_PORT_MODE on 30XX, 31XX, 50XX,
	 * and 68XX.
	 */
	if (!OCTEON_IS_MODEL(OCTEON_CN30XX) &&
	    !OCTEON_IS_MODEL(OCTEON_CN31XX) &&
	    !OCTEON_IS_MODEL(OCTEON_CN50XX) &&
	    !OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		/* Tell PKO the number of ports on this interface */
		pko_mode.u64 = cvmx_read_csr(CVMX_PKO_REG_GMX_PORT_MODE);
		if (interface == 0) {
			if (num_ports == 1)
				pko_mode.s.mode0 = 4;
			else if (num_ports == 2)
				pko_mode.s.mode0 = 3;
			else if (num_ports <= 4)
				pko_mode.s.mode0 = 2;
			else if (num_ports <= 8)
				pko_mode.s.mode0 = 1;
			else
				pko_mode.s.mode0 = 0;
		} else {
			if (num_ports == 1)
				pko_mode.s.mode1 = 4;
			else if (num_ports == 2)
				pko_mode.s.mode1 = 3;
			else if (num_ports <= 4)
				pko_mode.s.mode1 = 2;
			else if (num_ports <= 8)
				pko_mode.s.mode1 = 1;
			else
				pko_mode.s.mode1 = 0;
		}
		cvmx_write_csr(CVMX_PKO_REG_GMX_PORT_MODE, pko_mode.u64);
	}

	/*
	 * Set GMX to buffer as much data as possible before starting
	 * transmit. This reduces the chances that we have a TX under run
	 * due to memory contention. Any packet that fits entirely in the
	 * GMX FIFO can never have an under run regardless of memory load.
	 */
	gmx_tx_thresh.u64 = cvmx_read_csr(CVMX_GMXX_TXX_THRESH(0, interface));
	if (OCTEON_IS_MODEL(OCTEON_CN30XX) ||
	    OCTEON_IS_MODEL(OCTEON_CN31XX) ||
	    OCTEON_IS_MODEL(OCTEON_CN50XX)) {
		/* These chips have a fixed max threshold of 0x40 */
		gmx_tx_thresh.s.cnt = 0x40;
	} else {
		/* ccn - common cnt numberator */
		int ccn = 0x100;

		/* Choose the max value for the number of ports */
		if (num_ports <= 1)
			gmx_tx_thresh.s.cnt = ccn / 1;
		else if (num_ports == 2)
			gmx_tx_thresh.s.cnt = ccn / 2;
		else
			gmx_tx_thresh.s.cnt = ccn / 4;
	}

	/*
	 * SPI and XAUI can have lots of ports but the GMX hardware
	 * only ever has a max of 4
	 */
	if (num_ports > 4)
		num_ports = 4;
	for (index = 0; index < num_ports; index++)
		cvmx_write_csr(CVMX_GMXX_TXX_THRESH(index, interface), gmx_tx_thresh.u64);

	/*
	 * For o68, we need to setup the pipes
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN68XX) && interface < CVMX_HELPER_MAX_GMX) {
		union cvmx_gmxx_txx_pipe config;

		for (index = 0; index < num_ports; index++) {
			config.u64 = 0;

			if (__cvmx_helper_cfg_pko_port_base(interface, index) >= 0) {
				config.u64 = cvmx_read_csr(CVMX_GMXX_TXX_PIPE(index, interface));
				config.s.nump = __cvmx_helper_cfg_pko_port_num(interface, index);
				config.s.base = __cvmx_helper_cfg_pko_port_base(interface, index);
				cvmx_write_csr(CVMX_GMXX_TXX_PIPE(index, interface), config.u64);
			}
		}
	}

	return 0;
}

int cvmx_helper_get_pko_port(int interface, int port)
{
	return cvmx_pko_get_base_pko_port(interface, port);
}
EXPORT_SYMBOL(cvmx_helper_get_pko_port);

int cvmx_helper_get_ipd_port(int interface, int port)
{
	if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
		const struct ipd_port_map	*port_map;
		int				ipd_port;

		if (OCTEON_IS_MODEL(OCTEON_CN68XX))
			port_map = ipd_port_map_68xx;
		else if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			port_map = ipd_port_map_78xx;
		else
			return -1;

		ipd_port = port_map[interface].first_ipd_port;
		if (port_map[interface].type == GMII) {
			cvmx_helper_interface_mode_t mode =
				cvmx_helper_interface_get_mode(interface);
			if (mode == CVMX_HELPER_INTERFACE_MODE_XAUI ||
			    mode == CVMX_HELPER_INTERFACE_MODE_RXAUI) {
				ipd_port += port_map[interface].ipd_port_adj;
				return ipd_port;
			}
			else
				return ipd_port + (port * 16);
		} else if (port_map[interface].type == ILK)
			return ipd_port + port;
		else if (port_map[interface].type == NPI)
			return ipd_port + port;
		else if (port_map[interface].type == LB)
			return ipd_port + port;
		else
			return -1;

	} else if (cvmx_helper_interface_get_mode(interface) ==
			CVMX_HELPER_INTERFACE_MODE_AGL) {
		return 24;
	}

	switch (interface) {
	case 0:
		return port;
	case 1:
		return port + 16;
	case 2:
		return port + 32;
	case 3:
		return port + 36;
	case 4:
		return port + 40;
	case 5:
		return port + 42;
	case 6:
		return port + 44;
	case 7:
		return port + 46;
	}
	return -1;
}
EXPORT_SYMBOL(cvmx_helper_get_ipd_port);

int __cvmx_helper_get_num_ipd_ports(int interface)
{
	struct cvmx_iface *piface;

	if (interface >= cvmx_helper_get_number_of_interfaces())
		return -1;

	piface = &cvmx_interfaces[interface];
	return piface->cvif_ipd_nports;
}

enum cvmx_pko_padding __cvmx_helper_get_pko_padding(int interface)
{
	struct cvmx_iface *piface;

	if (interface >= cvmx_helper_get_number_of_interfaces())
		return CVMX_PKO_PADDING_NONE;

	piface = &cvmx_interfaces[interface];
	return piface->cvif_padding;
}

int __cvmx_helper_init_interface(int interface, int num_ipd_ports,
				 int has_fcs, enum cvmx_pko_padding pad)
{
	struct cvmx_iface *piface;
	cvmx_helper_link_info_t *p;
	int i;
	int sz;
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	uint64_t addr;
#endif

	if (interface >= cvmx_helper_get_number_of_interfaces())
		return -1;

	piface = &cvmx_interfaces[interface];
	piface->cvif_ipd_nports = num_ipd_ports;
	piface->cvif_padding = pad;

	piface->cvif_has_fcs = has_fcs;

	/*
	 * allocate the per-ipd_port link_info structure
	 */
	sz = piface->cvif_ipd_nports * sizeof(cvmx_helper_link_info_t);
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
	if (sz == 0)
		sz = sizeof(cvmx_helper_link_info_t);
	piface->cvif_ipd_port_link_info = (cvmx_helper_link_info_t *) kmalloc(sz, GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(piface->cvif_ipd_port_link_info))
		panic("Cannot allocate memory in __cvmx_helper_init_interface.");
#else
	char name[32];
	snprintf(name, sizeof(name), "__int_%d_link_info", interface);
	addr = CAST64(cvmx_bootmem_alloc_named_range_once(sz, 0, 0, 128, name, NULL));
	piface->cvif_ipd_port_link_info = (cvmx_helper_link_info_t *) __cvmx_phys_addr_to_ptr(addr, sz);
#endif
	if (!piface->cvif_ipd_port_link_info)
	{
		if (sz != 0)
			cvmx_dprintf("iface %d failed to alloc link info\n",
			    interface);
		return -1;
	}

	/* Initialize them */
	p = piface->cvif_ipd_port_link_info;

	for (i = 0; i < piface->cvif_ipd_nports; i++) {
		(*p).u64 = 0;
		p++;
	}
	return 0;
}

/*
 * Shut down the interfaces; free the resources.
 * @INTERNAL
 */
void __cvmx_helper_shutdown_interfaces(void)
{
	int i;
	int nifaces;		/* number of interfaces */
	struct cvmx_iface *piface;

	nifaces = cvmx_helper_get_number_of_interfaces();
	for (i = 0; i < nifaces; i++) {
		piface = cvmx_interfaces + i;
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
		if (piface->cvif_ipd_port_link_info)
			kfree(piface->cvif_ipd_port_link_info);
#elif defined(__U_BOOT__) && 0
		if (piface->cvif_ipd_port_link_info) {
			char name[32];
			snprintf(name, sizeof(name),
				 "__int_%d_link_info", i);
			cvmx_bootmem_phy_named_block_free(name, 0);
		}
#else
		/*
		 * For SE apps, bootmem was meant to be allocated and never
		 * freed.
		 */
#endif
		piface->cvif_ipd_port_link_info = 0;
	}
}

int __cvmx_helper_set_link_info(int interface, int port, cvmx_helper_link_info_t link_info)
{
	struct cvmx_iface *piface;

	if (interface >= cvmx_helper_get_number_of_interfaces())
		return -1;

	piface = &cvmx_interfaces[interface];

	if (piface->cvif_ipd_port_link_info) {
		piface->cvif_ipd_port_link_info[port] = link_info;
		return 0;
	}

	return -1;
}

cvmx_helper_link_info_t __cvmx_helper_get_link_info(int interface, int port)
{
	struct cvmx_iface *piface;
	cvmx_helper_link_info_t err;

	err.u64 = 0;

	if (interface >= cvmx_helper_get_number_of_interfaces())
		return err;
	piface = &cvmx_interfaces[interface];

	if (piface->cvif_ipd_port_link_info)
		return piface->cvif_ipd_port_link_info[port];

	return err;
}

/**
 * Returns if FCS is enabled for the specified interface and port
 *
 * @param interface - interface to check
 *
 * @return zero if FCS is not used, otherwise FCS is used.
 */
int __cvmx_helper_get_has_fcs(int interface)
{
	return cvmx_interfaces[interface].cvif_has_fcs;
}

int cvmx_helper_get_pknd(int interface, int port)
{
	if (octeon_has_feature(OCTEON_FEATURE_PKND))
		return __cvmx_helper_cfg_pknd(interface, port);

	return CVMX_INVALID_PKND;
}
EXPORT_SYMBOL(cvmx_helper_get_pknd);

int cvmx_helper_get_bpid(int interface, int port)
{
	if (octeon_has_feature(OCTEON_FEATURE_PKND))
		return __cvmx_helper_cfg_bpid(interface, port);

	return CVMX_INVALID_BPID;
}
EXPORT_SYMBOL(cvmx_helper_get_bpid);

/**
 * Display interface statistics.
 *
 * @param port IPD/PKO port number
 *
 * @return none
 */
void cvmx_helper_show_stats(int port)
{
	cvmx_pip_port_status_t status;
	cvmx_pko_port_status_t pko_status;

	/* ILK stats */
	if (octeon_has_feature(OCTEON_FEATURE_ILK))
		__cvmx_helper_ilk_show_stats();

	/* PIP stats */
	cvmx_pip_get_port_status(port, 0, &status);
	cvmx_dprintf("port %d: the number of packets - ipd: %d\n",
		     port, (int)status.packets);

	/* PKO stats */
	cvmx_pko_get_port_status(port, 0, &pko_status);
	cvmx_dprintf("port %d: the number of packets - pko: %d\n",
		     port, (int)pko_status.packets);

	/* TODO: other stats */
}

/**
 * Returns the interface number for an IPD/PKO port number.
 *
 * @param ipd_port IPD/PKO port number
 *
 * @return Interface number
 */
int cvmx_helper_get_interface_num(int ipd_port)
{
	if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
		const struct ipd_port_map	*port_map;
		int				i;

		if (OCTEON_IS_MODEL(OCTEON_CN68XX))
			port_map = ipd_port_map_68xx;
		else if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			port_map = ipd_port_map_78xx;
		else
			return -1;

		for (i = 0; i < CVMX_HELPER_MAX_IFACE; i++) {
			if (ipd_port >= port_map[i].first_ipd_port &&
			    ipd_port <= port_map[i].last_ipd_port)
				return i;
		}

		return -1;

	} else if (OCTEON_IS_MODEL(OCTEON_CN70XX) && ipd_port == 24) {
		return 4;
	} else {
		if (ipd_port < 16)
			return 0;
		else if (ipd_port < 32)
			return 1;
		else if (ipd_port < 36)
			return 2;
		else if (ipd_port < 40)
			return 3;
		else if (ipd_port < 42)
			return 4;
		else if (ipd_port < 44)
			return 5;
		else if (ipd_port < 46)
			return 6;
		else if (ipd_port < 48)
			return 7;
	}
	cvmx_dprintf("cvmx_helper_get_interface_num: Illegal IPD port number %d\n",
		     ipd_port);
	return -1;
}
EXPORT_SYMBOL(cvmx_helper_get_interface_num);

/**
 * Returns the interface index number for an IPD/PKO port
 * number.
 *
 * @param ipd_port IPD/PKO port number
 *
 * @return Interface index number
 */
int cvmx_helper_get_interface_index_num(int ipd_port)
{
	if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
		const struct ipd_port_map	*port_map;
		int				port;
		port_map_if_type_t		type = INVALID_IF_TYPE;
		int				i;
		int				num_interfaces;

		if (OCTEON_IS_MODEL(OCTEON_CN68XX))
			port_map = ipd_port_map_68xx;
		else if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			port_map = ipd_port_map_78xx;
		else
			return -1;

		num_interfaces = cvmx_helper_get_number_of_interfaces();

		/* Get the interface type of the ipd port */
		for (i = 0; i < num_interfaces; i++) {
			if (ipd_port >= port_map[i].first_ipd_port &&
			    ipd_port <= port_map[i].last_ipd_port) {
				type = port_map[i].type;
				break;
			}
		}

		/* Convert the ipd port to the interface port */
		switch (type) {
		case GMII:
			port = ((ipd_port & 0xff) >> 6);
 			return port ? (port - 1) : ((ipd_port & 0xff) >> 4);
			break;

		case ILK:
		case NPI:
		case LB:
			return ipd_port & 0xff;
			break;

		default:
			cvmx_dprintf("cvmx_helper_get_interface_index_num: "
				     "Illegal IPD port number %d\n", ipd_port);
			return -1;
		}
	}
	if (OCTEON_IS_MODEL(OCTEON_CN70XX))
		return ipd_port & 3;
	if (ipd_port < 32)
		return ipd_port & 15;
	else if (ipd_port < 40)
		return ipd_port & 3;
	else if (ipd_port < 48)
		return ipd_port & 1;
	else
		cvmx_dprintf("cvmx_helper_get_interface_index_num: Illegal IPD port number\n");

	return -1;
}
EXPORT_SYMBOL(cvmx_helper_get_interface_index_num);
