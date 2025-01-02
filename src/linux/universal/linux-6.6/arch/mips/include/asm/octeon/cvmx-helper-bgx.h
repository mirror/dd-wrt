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
 * Functions to configure the BGX MAC.
 *
 * <hr>$Revision$<hr>
 */

#ifndef __CVMX_HELPER_BGX_H__
#define __CVMX_HELPER_BGX_H__

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/cvmx-bgxx-defs.h>
#endif

#ifdef __cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

#define CVMX_BGX_RX_FIFO_SIZE	(64 * 1024)
#define CVMX_BGX_TX_FIFO_SIZE	(32 * 1024)

extern int __cvmx_helper_bgx_enumerate(int xiface);

/**
 * @INTERNAL
 * Disable the BGX port
 *
 * @param xipd_port IPD port of the BGX interface to disable
 */
extern void cvmx_helper_bgx_disable(int xipd_port);

/**
 * @INTERNAL
 * Probe a SGMII interface and determine the number of ports
 * connected to it. The SGMII/XAUI interface should still be down after
 * this call. This is used by interfaces using the bgx mac.
 *
 * @param xiface Interface to probe
 *
 * @return Number of ports on the interface. Zero to disable.
 */
extern int __cvmx_helper_bgx_probe(int xiface);

/**
 * @INTERNAL
 * Bringup and enable a SGMII interface. After this call packet
 * I/O should be fully functional. This is called with IPD
 * enabled but PKO disabled. This is used by interfaces using the
 * bgx mac.
 *
 * @param xiface Interface to bring up
 *
 * @return Zero on success, negative on failure
 */
extern int __cvmx_helper_bgx_sgmii_enable(int xiface);

/**
 * @INTERNAL
 * Return the link state of an IPD/PKO port as returned by
 * auto negotiation. The result of this function may not match
 * Octeon's link config if auto negotiation has changed since
 * the last call to cvmx_helper_link_set(). This is used by
 * interfaces using the bgx mac.
 *
 * @param xipd_port IPD/PKO port to query
 *
 * @return Link state
 */
extern cvmx_helper_link_info_t __cvmx_helper_bgx_sgmii_link_get(int xipd_port);

/**
 * @INTERNAL
 * Configure an IPD/PKO port for the specified link state. This
 * function does not influence auto negotiation at the PHY level.
 * The passed link state must always match the link state returned
 * by cvmx_helper_link_get(). It is normally best to use
 * cvmx_helper_link_autoconf() instead. This is used by interfaces
 * using the bgx mac.
 *
 * @param xipd_port  IPD/PKO port to configure
 * @param link_info The new link state
 *
 * @return Zero on success, negative on failure
 */
extern int __cvmx_helper_bgx_sgmii_link_set(int xipd_port,
					    cvmx_helper_link_info_t link_info);

/**
 * @INTERNAL
 * Configure a port for internal and/or external loopback. Internal loopback
 * causes packets sent by the port to be received by Octeon. External loopback
 * causes packets received from the wire to sent out again. This is used by
 * interfaces using the bgx mac.
 *
 * @param xipd_port IPD/PKO port to loopback.
 * @param enable_internal
 *                 Non zero if you want internal loopback
 * @param enable_external
 *                 Non zero if you want external loopback
 *
 * @return Zero on success, negative on failure.
 */
extern int __cvmx_helper_bgx_sgmii_configure_loopback(int xipd_port,
						      int enable_internal,
						      int enable_external);

/**
 * @INTERNAL
 * Bringup and enable a XAUI interface. After this call packet
 * I/O should be fully functional. This is called with IPD
 * enabled but PKO disabled. This is used by interfaces using the
 * bgx mac.
 *
 * @param xiface Interface to bring up
 *
 * @return Zero on success, negative on failure
 */
extern int __cvmx_helper_bgx_xaui_enable(int xiface);

extern CVMX_SHARED int(*cvmx_helper_bgx_override_autoneg)(int xiface, int index);
extern CVMX_SHARED int(*cvmx_helper_bgx_override_fec)(int xiface, int index);

/**
 * @INTERNAL
 * Return the link state of an IPD/PKO port as returned by
 * auto negotiation. The result of this function may not match
 * Octeon's link config if auto negotiation has changed since
 * the last call to cvmx_helper_link_set(). This is used by
 * interfaces using the bgx mac.
 *
 * @param xipd_port IPD/PKO port to query
 *
 * @return Link state
 */
extern cvmx_helper_link_info_t __cvmx_helper_bgx_xaui_link_get(int xipd_port);

/**
 * @INTERNAL
 * Configure an IPD/PKO port for the specified link state. This
 * function does not influence auto negotiation at the PHY level.
 * The passed link state must always match the link state returned
 * by cvmx_helper_link_get(). It is normally best to use
 * cvmx_helper_link_autoconf() instead. This is used by interfaces
 * using the bgx mac.
 *
 * @param xipd_port  IPD/PKO port to configure
 * @param link_info The new link state
 *
 * @return Zero on success, negative on failure
 */
extern int __cvmx_helper_bgx_xaui_link_set(int xipd_port,
					   cvmx_helper_link_info_t link_info);

/**
 * @INTERNAL
 * Configure a port for internal and/or external loopback. Internal loopback
 * causes packets sent by the port to be received by Octeon. External loopback
 * causes packets received from the wire to sent out again. This is used by
 * interfaces using the bgx mac.
 *
 * @param xipd_port IPD/PKO port to loopback.
 * @param enable_internal
 *                 Non zero if you want internal loopback
 * @param enable_external
 *                 Non zero if you want external loopback
 *
 * @return Zero on success, negative on failure.
 */
extern int __cvmx_helper_bgx_xaui_configure_loopback(int xipd_port,
						     int enable_internal,
						     int enable_external);

extern int __cvmx_helper_bgx_mixed_enable(int xiface);

extern cvmx_helper_link_info_t __cvmx_helper_bgx_mixed_link_get(int xipd_port);

extern int __cvmx_helper_bgx_mixed_link_set(int xipd_port, cvmx_helper_link_info_t link_info);

extern int __cvmx_helper_bgx_mixed_configure_loopback(int xipd_port,
						     int enable_internal,
						     int enable_external);

extern cvmx_helper_interface_mode_t cvmx_helper_bgx_get_mode(int xiface, int index);

/**
 * @INTERNAL
 * Configure Priority-Based Flow Control (a.k.a. PFC/CBFC)
 * on a specific BGX interface/port.
 */
extern void __cvmx_helper_bgx_xaui_config_pfc(unsigned node,
		unsigned interface, unsigned port, bool pfc_enable);
/**
 * This function control how the hardware handles incoming PAUSE
 * packets. The most common modes of operation:
 * ctl_bck = 1, ctl_drp = 1: hardware handles everything
 * ctl_bck = 0, ctl_drp = 0: software sees all PAUSE frames
 * ctl_bck = 0, ctl_drp = 1: all PAUSE frames are completely ignored
 * @param node		node number.
 * @param interface	interface number
 * @param port		port number
 * @param ctl_bck	1: Forward PAUSE information to TX block
 * @param ctl_drp	1: Drop control PAUSE frames.
 */
extern void cvmx_helper_bgx_rx_pause_ctl(unsigned node, unsigned interface,
				  unsigned port, unsigned ctl_bck, unsigned ctl_drp);

/**
 * This function configures the receive action taken for multicast, broadcast
 * and dmac filter match packets.
 * @param node		node number.
 * @param interface	interface number
 * @param port		port number
 * @param cam_accept	0: reject packets on dmac filter match
 *                      1: accept packet on dmac filter match
 * @param mcast_mode	0x0 = Force reject all multicast packets
 *                      0x1 = Force accept all multicast packets
 *                      0x2 = Use the address filter CAM
 * @param bcast_accept  0 = Reject all broadcast packets
 *                      1 = Accept all broadcast packets
 */
extern void cvmx_helper_bgx_rx_adr_ctl(unsigned node, unsigned interface, unsigned port,
                                unsigned cam_accept, unsigned mcast_mode, unsigned bcast_accept);

/**
 * Function to control the generation of FCS, padding by the BGX
 *
 */
extern void cvmx_helper_bgx_tx_options(unsigned node,
	unsigned interface, unsigned index,
	bool fcs_enable, bool pad_enable);

/**
 * Set mac for the ipd_port
 *
 * @param xipd_port ipd_port to set the mac
 * @param bcst      If set, accept all broadcast packets
 * @param mcst      Multicast mode
 * 		    0 = Force reject all multicast packets
 * 		    1 = Force accept all multicast packets
 * 		    2 = use the address filter CAM.
 * @param mac       mac address for the ipd_port
 */
extern void cvmx_helper_bgx_set_mac(int xipd_port, int bcst, int mcst, uint64_t mac);


extern int __cvmx_helper_bgx_port_init(int xipd_port, int phy_pres);
extern void cvmx_helper_bgx_set_jabber(int xiface, unsigned index, unsigned size);
extern int cvmx_helper_bgx_shutdown_port(int xiface, int index);
extern int cvmx_bgx_set_backpressure_override(int xiface, unsigned port_mask);
extern int __cvmx_helper_bgx_fifo_size(int xiface, unsigned lmac);

/**
 * Returns if an interface is RGMII or not
 *
 * @param xiface	xinterface to check
 * @param index		port index (must be 0 for rgmii)
 *
 * @return	true if RGMII, false otherwise
 */
static inline bool cvmx_helper_bgx_is_rgmii(int xiface, int index)
{
	union cvmx_bgxx_cmrx_config cmr_config;

	if (!OCTEON_IS_MODEL(OCTEON_CN73XX) || index != 0)
		return false;
	cmr_config.u64 = cvmx_read_csr(CVMX_BGXX_CMRX_CONFIG(index, xiface));
	return cmr_config.s.lmac_type == 5;
}

/**
 * Enables or disables autonegotiation for an interface.
 *
 * @param	xiface	interface to set autonegotiation
 * @param	index	port index
 * @param	enable	true to enable autonegotiation, false to disable it
 *
 * @return	0 for success, -1 on error.
 */
int cvmx_helper_set_autonegotiation(int xiface, int index, bool enable);

/**
 * Enables or disables forward error correction
 *
 * @param	xiface	interface
 * @param	index	port index
 * @param	enable	set to true to enable FEC, false to disable
 *
 * @return	0 for success, -1 on error
 *
 * @NOTE:	If autonegotiation is enabled then autonegotiation will be
 *		restarted for negotiating FEC.
 */
int cvmx_helper_set_fec(int xiface, int index, bool enable);

#ifdef CVMX_DUMP_BGX
/**
 * Dump BGX configuration for node 0
 */
int cvmx_dump_bgx_config(unsigned bgx);
/**
 * Dump BGX status for node 0
 */
int cvmx_dump_bgx_status(unsigned bgx);
/**
 * Dump BGX configuration
 */
int cvmx_dump_bgx_config_node(unsigned node, unsigned bgx);
/**
 * Dump BGX status
 */
int cvmx_dump_bgx_status_node(unsigned node, unsigned bgx);

#endif
#ifdef __cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif
#endif
