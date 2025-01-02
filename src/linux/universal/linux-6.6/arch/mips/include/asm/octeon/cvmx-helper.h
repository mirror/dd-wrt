/***********************license start***************
 * Copyright (c) 2003-2010  Cavium Inc. (support@cavium.com). All rights
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
 * Helper functions for common, but complicated tasks.
 *
 * <hr>$Revision: 153454 $<hr>
 */

#ifndef __CVMX_HELPER_H__
#define __CVMX_HELPER_H__

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/cvmx.h>
#endif

#include "cvmx-wqe.h"

#ifdef  __cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

/* Max number of GMXX */
#define CVMX_HELPER_MAX_GMX	(OCTEON_IS_MODEL(OCTEON_CN78XX) ? 6 \
				 : (OCTEON_IS_MODEL(OCTEON_CN68XX) ? 5 \
				    : (OCTEON_IS_MODEL(OCTEON_CN73XX) ? 3 \
				       : (OCTEON_IS_MODEL(OCTEON_CNF75XX) ? 1 \
					  : 2))))

#define CVMX_HELPER_CSR_INIT0           0	/* Do not change as
						   CVMX_HELPER_WRITE_CSR()
						   assumes it */
#define CVMX_HELPER_CSR_INIT_READ       -1

/*
 * CVMX_HELPER_WRITE_CSR--set a field in a CSR with a value.
 *
 * @param chcsr_init    intial value of the csr (CVMX_HELPER_CSR_INIT_READ
 *                      means to use the existing csr value as the
 *                      initial value.)
 * @param chcsr_csr     the name of the csr
 * @param chcsr_type    the type of the csr (see the -defs.h)
 * @param chcsr_chip    the chip for the csr/field
 * @param chcsr_fld     the field in the csr
 * @param chcsr_val     the value for field
 */
#define CVMX_HELPER_WRITE_CSR(chcsr_init, chcsr_csr, chcsr_type,        \
    chcsr_chip, chcsr_fld, chcsr_val)                                   \
        do {                                                            \
                chcsr_type csr;                                         \
                if ((chcsr_init) == CVMX_HELPER_CSR_INIT_READ)          \
                        csr.u64 = cvmx_read_csr(chcsr_csr);             \
                else                                                    \
                        csr.u64 = (chcsr_init);                         \
                csr.chcsr_chip.chcsr_fld = (chcsr_val);                 \
                cvmx_write_csr((chcsr_csr), csr.u64);                   \
        } while(0)

/*
 * CVMX_HELPER_WRITE_CSR0--set a field in a CSR with the initial value of 0
 */
#define CVMX_HELPER_WRITE_CSR0(chcsr_csr, chcsr_type, chcsr_chip,       \
    chcsr_fld, chcsr_val)                                               \
        CVMX_HELPER_WRITE_CSR(CVMX_HELPER_CSR_INIT0, chcsr_csr,         \
            chcsr_type, chcsr_chip, chcsr_fld, chcsr_val)

/*
 * CVMX_HELPER_WRITE_CSR1--set a field in a CSR with the initial value of
 *                      the CSR's current value.
 */
#define CVMX_HELPER_WRITE_CSR1(chcsr_csr, chcsr_type, chcsr_chip,       \
    chcsr_fld, chcsr_val)                                               \
        CVMX_HELPER_WRITE_CSR(CVMX_HELPER_CSR_INIT_READ, chcsr_csr,     \
            chcsr_type, chcsr_chip, chcsr_fld, chcsr_val)

/* These flags are passed to __cvmx_helper_packet_hardware_enable */

typedef enum {
	CVMX_HELPER_INTERFACE_MODE_DISABLED,
	CVMX_HELPER_INTERFACE_MODE_RGMII,
	CVMX_HELPER_INTERFACE_MODE_GMII,
	CVMX_HELPER_INTERFACE_MODE_SPI,
	CVMX_HELPER_INTERFACE_MODE_PCIE,
	CVMX_HELPER_INTERFACE_MODE_XAUI,
	CVMX_HELPER_INTERFACE_MODE_SGMII,
	CVMX_HELPER_INTERFACE_MODE_PICMG,
	CVMX_HELPER_INTERFACE_MODE_NPI,
	CVMX_HELPER_INTERFACE_MODE_LOOP,
	CVMX_HELPER_INTERFACE_MODE_SRIO,
	CVMX_HELPER_INTERFACE_MODE_ILK,
	CVMX_HELPER_INTERFACE_MODE_RXAUI,
	CVMX_HELPER_INTERFACE_MODE_QSGMII,
	CVMX_HELPER_INTERFACE_MODE_AGL,
	CVMX_HELPER_INTERFACE_MODE_XLAUI,
	CVMX_HELPER_INTERFACE_MODE_XFI,
	CVMX_HELPER_INTERFACE_MODE_10G_KR,
	CVMX_HELPER_INTERFACE_MODE_40G_KR4,
	CVMX_HELPER_INTERFACE_MODE_MIXED,
} cvmx_helper_interface_mode_t;

typedef union cvmx_helper_link_info {
	uint64_t u64;
	struct {
		uint64_t reserved_20_63:44;
		uint64_t link_up:1;	    /**< Is the physical link up? */
		uint64_t full_duplex:1;
					    /**< 1 if the link is full duplex */
		uint64_t speed:18;	    /**< Speed of the link in Mbps */
	} s;
} cvmx_helper_link_info_t;

/**
 * Sets the back pressure configuration in internal data structure.
 * @param backpressure_dis disable/enable backpressure
 */
void cvmx_rgmii_set_back_pressure(uint64_t backpressure_dis);

#include "cvmx-helper-fpa.h"


#include "cvmx-helper-agl.h"
#include "cvmx-helper-errata.h"
#include "cvmx-helper-ilk.h"
#include "cvmx-helper-loop.h"
#include "cvmx-helper-npi.h"
#include "cvmx-helper-rgmii.h"
#include "cvmx-helper-sgmii.h"
#include "cvmx-helper-spi.h"
#include "cvmx-helper-srio.h"
#include "cvmx-helper-util.h"
#include "cvmx-helper-xaui.h"

enum cvmx_pko_padding {
	CVMX_PKO_PADDING_NONE = 0,
	CVMX_PKO_PADDING_60 = 1,
};

/**
 * cvmx_override_iface_phy_mode(int interface, int index) is a function pointer.
 * It is meant to allow customization of interfaces which do not have a PHY.
 *
 * @returns 0 if MAC decides TX_CONFIG_REG or 1 if PHY decides  TX_CONFIG_REG.
 *
 * If this function pointer is NULL then it defaults to the MAC.
 */
extern CVMX_SHARED int (*cvmx_override_iface_phy_mode) (int interface,
							int index);

/**
 * cvmx_override_ipd_port_setup(int ipd_port) is a function
 * pointer. It is meant to allow customization of the IPD port/port kind
 * setup before packet input/output comes online. It is called
 * after cvmx-helper does the default IPD configuration, but
 * before IPD is enabled. Users should set this pointer to a
 * function before calling any cvmx-helper operations.
 */
extern CVMX_SHARED void (*cvmx_override_ipd_port_setup) (int ipd_port);

/**
 * This function enables the IPD and also enables the packet interfaces.
 * The packet interfaces (RGMII and SPI) must be enabled after the
 * IPD.  This should be called by the user program after any additional
 * IPD configuration changes are made if CVMX_HELPER_ENABLE_IPD
 * is not set in the executive-config.h file.
 *
 * @return 0 on success
 *         -1 on failure
 */
int cvmx_helper_ipd_and_packet_input_enable_node(int node);
int cvmx_helper_ipd_and_packet_input_enable(void);

/**
 * Initialize and allocate memory for the SSO.
 *
 * @param wqe_entries The maximum number of work queue entries to be
 * supported.
 *
 * @return Zero on success, non-zero on failure.
 */
extern int cvmx_helper_initialize_sso(int wqe_entries);

/**
 * Initialize and allocate memory for the SSO on a specific node.
 *
 * @param node Node SSO to initialize
 * @param wqe_entries The maximum number of work queue entries to be
 * supported.
 *
 * @return Zero on success, non-zero on failure.
 */
extern int cvmx_helper_initialize_sso_node(unsigned node, int wqe_entries);

/**
 * Undo the effect of cvmx_helper_initialize_sso().
 *
 * @return Zero on success, non-zero on failure.
 */
extern int cvmx_helper_uninitialize_sso(void);

/**
 * Undo the effect of cvmx_helper_initialize_sso_node().
 *
 * @param node Node SSO to initialize
 *
 * @return Zero on success, non-zero on failure.
 */
extern int cvmx_helper_uninitialize_sso_node(unsigned node);

/**
 * Initialize the PIP, IPD, and PKO hardware to support
 * simple priority based queues for the ethernet ports. Each
 * port is configured with a number of priority queues based
 * on CVMX_PKO_QUEUES_PER_PORT_* where each queue is lower
 * priority than the previous.
 *
 * @return Zero on success, non-zero on failure
 */
int cvmx_helper_initialize_packet_io_global(void);
/**
 * Initialize the PIP, IPD, and PKO hardware to support
 * simple priority based queues for the ethernet ports. Each
 * port is configured with a number of priority queues based
 * on CVMX_PKO_QUEUES_PER_PORT_* where each queue is lower
 * priority than the previous.
 *
 * @param node Node on which to initialize packet io hardware
 *
 * @return Zero on success, non-zero on failure
 */
int cvmx_helper_initialize_packet_io_node(unsigned int node);

/**
 * Does core local initialization for packet io
 *
 * @return Zero on success, non-zero on failure
 */
extern int cvmx_helper_initialize_packet_io_local(void);

/**
 * Undo the initialization performed in
 * cvmx_helper_initialize_packet_io_global(). After calling this routine and the
 * local version on each core, packet IO for Octeon will be disabled and placed
 * in the initial reset state. It will then be safe to call the initialize
 * later on. Note that this routine does not empty the FPA pools. It frees all
 * buffers used by the packet IO hardware to the FPA so a function emptying the
 * FPA after shutdown should find all packet buffers in the FPA.
 *
 * @return Zero on success, negative on failure.
 */
extern int cvmx_helper_shutdown_packet_io_global(void);

/**
 * Helper function for 78xx global packet IO shutdown
 */
extern int cvmx_helper_shutdown_packet_io_global_cn78xx(int node);

/**
 * Does core local shutdown of packet io
 *
 * @return Zero on success, non-zero on failure
 */
extern int cvmx_helper_shutdown_packet_io_local(void);

/**
 * Returns the number of ports on the given interface.
 * The interface must be initialized before the port count
 * can be returned.
 *
 * @param interface Which interface to return port count for.
 *
 * @return Port count for interface
 *         -1 for uninitialized interface
 */
extern int cvmx_helper_ports_on_interface(int interface);

/**
 * Return the number of interfaces the chip has. Each interface
 * may have multiple ports. Most chips support two interfaces,
 * but the CNX0XX and CNX1XX are exceptions. These only support
 * one interface.
 *
 * @return Number of interfaces on chip
 */
extern int cvmx_helper_get_number_of_interfaces(void);

/**
 * Get the operating mode of an interface. Depending on the Octeon
 * chip and configuration, this function returns an enumeration
 * of the type of packet I/O supported by an interface.
 *
 * @param xiface Interface to probe
 *
 * @return Mode of the interface. Unknown or unsupported interfaces return
 *         DISABLED.
 */
extern cvmx_helper_interface_mode_t cvmx_helper_interface_get_mode(int xiface);

/**
 * Auto configure an IPD/PKO port link state and speed. This
 * function basically does the equivalent of:
 * cvmx_helper_link_set(ipd_port, cvmx_helper_link_get(ipd_port));
 *
 * @param ipd_port IPD/PKO port to auto configure
 *
 * @return Link state after configure
 */
extern cvmx_helper_link_info_t cvmx_helper_link_autoconf(int ipd_port);

/**
 * Return the link state of an IPD/PKO port as returned by
 * auto negotiation. The result of this function may not match
 * Octeon's link config if auto negotiation has changed since
 * the last call to cvmx_helper_link_set().
 *
 * @param ipd_port IPD/PKO port to query
 *
 * @return Link state
 */
extern cvmx_helper_link_info_t cvmx_helper_link_get(int ipd_port);

/**
 * Configure an IPD/PKO port for the specified link state. This
 * function does not influence auto negotiation at the PHY level.
 * The passed link state must always match the link state returned
 * by cvmx_helper_link_get(). It is normally best to use
 * cvmx_helper_link_autoconf() instead.
 *
 * @param ipd_port  IPD/PKO port to configure
 * @param link_info The new link state
 *
 * @return Zero on success, negative on failure
 */
extern int cvmx_helper_link_set(int ipd_port, cvmx_helper_link_info_t link_info);

/**
 * This function probes an interface to determine the actual number of
 * hardware ports connected to it. It does some setup the ports but
 * doesn't enable them. The main goal here is to set the global
 * interface_port_count[interface] correctly. Final hardware setup of
 * the ports will be performed later.
 *
 * @param xiface Interface to probe
 *
 * @return Zero on success, negative on failure
 */
extern int cvmx_helper_interface_probe(int xiface);

/**
 * Determine the actual number of hardware ports connected to an
 * interface. It doesn't setup the ports or enable them.
 *
 * @param xiface Interface to enumerate
 *
 * @return Zero on success, negative on failure
 */
extern int cvmx_helper_interface_enumerate(int xiface);

/**
 * Configure a port for internal and/or external loopback. Internal loopback
 * causes packets sent by the port to be received by Octeon. External loopback
 * causes packets received from the wire to sent out again.
 *
 * @param ipd_port IPD/PKO port to loopback.
 * @param enable_internal
 *                 Non zero if you want internal loopback
 * @param enable_external
 *                 Non zero if you want external loopback
 *
 * @return Zero on success, negative on failure.
 */
extern int cvmx_helper_configure_loopback(int ipd_port, int enable_internal, int enable_external);

/**
 * Returns the number of ports on the given interface.
 *
 * @param interface Which interface to return port count for.
 *
 * @return Port count for interface
 *         -1 for uninitialized interface
 */
int __cvmx_helper_early_ports_on_interface(int interface);

void cvmx_helper_setup_simulator_io_buffer_counts(int node, int num_packet_buffers,
		int pko_buffers);

void cvmx_helper_set_wqe_no_ptr_mode(bool mode);
void cvmx_helper_set_pkt_wqe_le_mode(bool mode);
int cvmx_helper_shutdown_fpa_pools(int node);


/**
 * Convert Ethernet QoS/PCP value to system-level priority
 *
 * In OCTEON, highest priority is 0, in Ethernet 802.1p PCP field
 * the highest priority is 7, lowest is 1. Here is the full conversion
 * table between QoS (PCP) and OCTEON priority values, per IEEE 802.1Q-2005:
 *
 * PCP 	Priority 	Acronym 	Traffic Types
 * 1 	7 (lowest) 	BK 	Background
 * 0 	6 	BE 	Best Effort
 * 2 	5 	EE 	Excellent Effort
 * 3 	4 	CA 	Critical Applications
 * 4 	3 	VI 	Video, < 100 ms latency and jitter
 * 5 	2 	VO 	Voice, < 10 ms latency and jitter
 * 6 	1 	IC 	Internetwork Control
 * 7 	0 (highest) 	NC 	Network Control
 */
static inline uint8_t cvmx_helper_qos2prio(uint8_t qos)
{
	static const unsigned pcp_map =
		6 << (4 * 0) |
		7 << (4 * 1) |
		5 << (4 * 2) |
		4 << (4 * 3) |
		3 << (4 * 4) |
		2 << (4 * 5) |
		1 << (4 * 6) |
		0 << (4 * 7);

	return (pcp_map >> ((qos & 0x7) << 2)) & 0x7;
}

/**
 * Convert system-level priority to Ethernet QoS/PCP value
 *
 * Calculate the reverse of cvmx_helper_qos2prio() per IEEE 802.1Q-2005.
 */
static inline uint8_t cvmx_helper_prio2qos(uint8_t prio)
{
	static const unsigned prio_map =
		7 << (4 * 0) |
		6 << (4 * 1) |
		5 << (4 * 2) |
		4 << (4 * 3) |
		3 << (4 * 4) |
		2 << (4 * 5) |
		0 << (4 * 6) |
		1 << (4 * 7);

	return (prio_map >> ((prio & 0x7) << 2)) & 0x7;
}

/**
 * @INTERNAL
 * Get the number of ipd_ports on an interface.
 *
 * @param xiface
 *
 * @return the number of ipd_ports on the interface and -1 for error.
 */
int __cvmx_helper_get_num_ipd_ports(int xiface);

enum cvmx_pko_padding __cvmx_helper_get_pko_padding(int xiface);

/**
 * @INTERNAL
 *
 * @param xiface
 * @param num_ipd_ports is the number of ipd_ports on the interface
 * @param has_fcs indicates if PKO does FCS for the ports on this
 * @param pad The padding that PKO should apply.
 * interface.
 *
 * @return 0 for success and -1 for failure
 */
int __cvmx_helper_init_interface(int xiface, int num_ipd_ports, int has_fcs, enum cvmx_pko_padding pad);

void __cvmx_helper_shutdown_interfaces(void);

/*
 * @INTERNAL
 * Enable packet input/output from the hardware. This function is
 * called after all internal setup is complete and IPD is enabled.
 * After this function completes, packets will be accepted from the
 * hardware ports. PKO should still be disabled to make sure packets
 * aren't sent out partially setup hardware.
 *
 * @return Zero on success, negative on failure
 */
int __cvmx_helper_packet_hardware_enable(int xiface);

/*
 * @INTERNAL
 *
 * @return 0 for success and -1 for failure
 */
int __cvmx_helper_set_link_info(int xiface, int index, cvmx_helper_link_info_t link_info);

/**
 * @INTERNAL
 *
 * @param xiface
 * @param port
 *
 * @return valid link_info on success or -1 on failure
 */
cvmx_helper_link_info_t __cvmx_helper_get_link_info(int xiface, int port);

/**
 * @INTERNAL
 *
 * @param xiface
 *
 * @return 0 if PKO does not do FCS and 1 otherwise.
 */
int __cvmx_helper_get_has_fcs(int xiface);

void *cvmx_helper_mem_alloc(int node, uint64_t alloc_size, uint64_t align);
void cvmx_helper_mem_free(void *buffer, uint64_t size);

#ifdef  __cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif
#endif /* __CVMX_HELPER_H__ */
