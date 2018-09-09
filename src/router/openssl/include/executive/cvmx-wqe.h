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
 * This header file defines the work queue entry (wqe) data structure.
 * Since this is a commonly used structure that depends on structures
 * from several hardware blocks, those definitions have been placed
 * in this file to create a single point of definition of the wqe
 * format.
 * Data structures are still named according to the block that they
 * relate to.
 *
 * This file must not depend on any other header files, except for cvmx.h!!!
 *
 *
 * <hr>$Revision: 89326 $<hr>
 *
 *
 */

#ifndef __CVMX_WQE_H__
#define __CVMX_WQE_H__

#ifdef	__cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

#define OCT_TAG_TYPE_STRING(x) (((x) == CVMX_POW_TAG_TYPE_ORDERED) ?  "ORDERED" : \
                                (((x) == CVMX_POW_TAG_TYPE_ATOMIC) ?  "ATOMIC" : \
                                (((x) == CVMX_POW_TAG_TYPE_NULL) ?  "NULL" : \
                                "NULL_NULL")))

typedef union {
	uint64_t u64;
#ifdef __BIG_ENDIAN_BITFIELD
	struct {
		uint64_t ptr_vlan:8;              /**< Contains a byte pointer to the first byte of the
					               VLAN ID field for the first or second VLAN. */
		uint64_t ptr_layer_g:8;           /**< Contains a byte pointer to the start of layer G */
		uint64_t ptr_layer_f:8;           /**< Contains a byte pointer to the start of layer F */
		uint64_t ptr_layer_e:8;           /**< Contains a byte pointer to the start of layer E */
		uint64_t ptr_layer_d:8;           /**< Contains a byte pointer to the start of layer D */
		uint64_t ptr_layer_c:8;           /**< Contains a byte pointer to the start of layer C */
		uint64_t ptr_layer_b:8;           /**< Contains a byte pointer to the start of layer B */
		uint64_t ptr_layer_a:8;           /**< Contains a byte pointer to the start of layer A */
	};
#else
	struct {
		uint64_t ptr_layer_a:8;
		uint64_t ptr_layer_b:8;
		uint64_t ptr_layer_c:8;
		uint64_t ptr_layer_d:8;
		uint64_t ptr_layer_e:8;
		uint64_t ptr_layer_f:8;
		uint64_t ptr_layer_g:8;
		uint64_t ptr_vlan:8;

	};
#endif
} cvmx_wqe_word4_t;

/**
 * HW decode / err_code in work queue entry
 */
typedef union {
	uint64_t u64;

#ifdef __BIG_ENDIAN_BITFIELD
    /** Use this struct if the hardware determines that the packet is IP */
	struct {
		uint64_t bufs:8;		  /**< HW sets this to the number of buffers used by this packet */
		uint64_t ip_offset:8;		  /**< HW sets to the number of L2 bytes prior to the IP */
		uint64_t vlan_valid:1;		  /**< set to 1 if we found DSA/VLAN in the L2 */
		uint64_t vlan_stacked:1;	  /**< Set to 1 if the DSA/VLAN tag is stacked */
		uint64_t unassigned:1;
		uint64_t vlan_cfi:1;		  /**< HW sets to the DSA/VLAN CFI flag (valid when vlan_valid) */
		uint64_t vlan_id:12;		  /**< HW sets to the DSA/VLAN_ID field (valid when vlan_valid) */
		uint64_t varies:12;
				/**< 38xx and 68xx have different definitions.  */
		uint64_t dec_ipcomp:1;		  /**< the packet needs to be decompressed */
		uint64_t tcp_or_udp:1;		  /**< the packet is either TCP or UDP */
		uint64_t dec_ipsec:1;		  /**< the packet needs to be decrypted (ESP or AH) */
		uint64_t is_v6:1;		  /**< the packet is IPv6 */

		/* (rcv_error, not_IP, IP_exc, is_frag, L4_error, software, etc.) */

		uint64_t software:1;		  /**< reserved for software use, hardware will clear on packet creation */
		/* exceptional conditions below */
		uint64_t L4_error:1;		  /**< the receive interface hardware detected an L4 error (only applies if !is_frag)
                                                    (only applies if !rcv_error && !not_IP && !IP_exc && !is_frag)
                                                    failure indicated in err_code below, decode:
                                                    - 1 = Malformed L4
                                                    - 2 = L4 Checksum Error: the L4 checksum value is
                                                    - 3 = UDP Length Error: The UDP length field would make the UDP data longer than what
                                                        remains in the IP packet (as defined by the IP header length field).
                                                    - 4 = Bad L4 Port: either the source or destination TCP/UDP port is 0.
                                                    - 8 = TCP FIN Only: the packet is TCP and only the FIN flag set.
                                                    - 9 = TCP No Flags: the packet is TCP and no flags are set.
                                                    - 10 = TCP FIN RST: the packet is TCP and both FIN and RST are set.
                                                    - 11 = TCP SYN URG: the packet is TCP and both SYN and URG are set.
                                                    - 12 = TCP SYN RST: the packet is TCP and both SYN and RST are set.
                                                    - 13 = TCP SYN FIN: the packet is TCP and both SYN and FIN are set. */

		uint64_t is_frag:1;		  /**< set if the packet is a fragment */
		uint64_t IP_exc:1;		  /**< the receive interface hardware detected an IP error / exception
                                                    (only applies if !rcv_error && !not_IP) failure indicated in err_code below, decode:
                                                    - 1 = Not IP: the IP version field is neither 4 nor 6.
                                                    - 2 = IPv4 Header Checksum Error: the IPv4 header has a checksum violation.
                                                    - 3 = IP Malformed Header: the packet is not long enough to contain the IP header.
                                                    - 4 = IP Malformed: the packet is not long enough to contain the bytes indicated by the IP
                                                        header. Pad is allowed.
                                                    - 5 = IP TTL Hop: the IPv4 TTL field or the IPv6 Hop Count field are zero.
                                                    - 6 = IP Options */

		uint64_t is_bcast:1;		  /**< set if the hardware determined that the packet is a broadcast */
		uint64_t is_mcast:1;		  /**< set if the hardware determined that the packet is a multi-cast */
		uint64_t not_IP:1;		  /**< set if the packet may not be IP (must be zero in this case) */
		uint64_t rcv_error:1;		  /**< the receive interface hardware detected a receive error (must be zero in this case) */
		/* lower err_code = first-level descriptor of the work */
		/* zero for packet submitted by hardware that isn't on the slow path */

		uint64_t err_code:8;		  /**< type is cvmx_pip_err_t */
	} s;
	struct {
		uint64_t bufs:8;		  /**< HW sets this to the number of buffers used by this packet */
		uint64_t ip_offset:8;		  /**< HW sets to the number of L2 bytes prior to the IP */
		uint64_t vlan_valid:1;		  /**< set to 1 if we found DSA/VLAN in the L2 */
		uint64_t vlan_stacked:1;	  /**< Set to 1 if the DSA/VLAN tag is stacked */
		uint64_t unassigned:1;
		uint64_t vlan_cfi:1;		  /**< HW sets to the DSA/VLAN CFI flag (valid when vlan_valid) */
		uint64_t vlan_id:12;		  /**< HW sets to the DSA/VLAN_ID field (valid when vlan_valid) */

		uint64_t port:12;
			       /**< MAC/PIP port number.  */

		uint64_t dec_ipcomp:1;		  /**< the packet needs to be decompressed */
		uint64_t tcp_or_udp:1;		  /**< the packet is either TCP or UDP */
		uint64_t dec_ipsec:1;		  /**< the packet needs to be decrypted (ESP or AH) */
		uint64_t is_v6:1;		  /**< the packet is IPv6 */

		/* (rcv_error, not_IP, IP_exc, is_frag, L4_error, software, etc.) */

		uint64_t software:1;		  /**< reserved for software use, hardware will clear on packet creation */
		/* exceptional conditions below */
		uint64_t L4_error:1;		  /**< the receive interface hardware detected an L4 error (only applies if !is_frag)
                                                    (only applies if !rcv_error && !not_IP && !IP_exc && !is_frag)
                                                    failure indicated in err_code below, decode:
                                                    - 1 = Malformed L4
                                                    - 2 = L4 Checksum Error: the L4 checksum value is
                                                    - 3 = UDP Length Error: The UDP length field would make the UDP data longer than what
                                                        remains in the IP packet (as defined by the IP header length field).
                                                    - 4 = Bad L4 Port: either the source or destination TCP/UDP port is 0.
                                                    - 8 = TCP FIN Only: the packet is TCP and only the FIN flag set.
                                                    - 9 = TCP No Flags: the packet is TCP and no flags are set.
                                                    - 10 = TCP FIN RST: the packet is TCP and both FIN and RST are set.
                                                    - 11 = TCP SYN URG: the packet is TCP and both SYN and URG are set.
                                                    - 12 = TCP SYN RST: the packet is TCP and both SYN and RST are set.
                                                    - 13 = TCP SYN FIN: the packet is TCP and both SYN and FIN are set. */

		uint64_t is_frag:1;		  /**< set if the packet is a fragment */
		uint64_t IP_exc:1;		  /**< the receive interface hardware detected an IP error / exception
                                                    (only applies if !rcv_error && !not_IP) failure indicated in err_code below, decode:
                                                    - 1 = Not IP: the IP version field is neither 4 nor 6.
                                                    - 2 = IPv4 Header Checksum Error: the IPv4 header has a checksum violation.
                                                    - 3 = IP Malformed Header: the packet is not long enough to contain the IP header.
                                                    - 4 = IP Malformed: the packet is not long enough to contain the bytes indicated by the IP
                                                        header. Pad is allowed.
                                                    - 5 = IP TTL Hop: the IPv4 TTL field or the IPv6 Hop Count field are zero.
                                                    - 6 = IP Options */

		uint64_t is_bcast:1;		  /**< set if the hardware determined that the packet is a broadcast */
		uint64_t is_mcast:1;		  /**< set if the hardware determined that the packet is a multi-cast */
		uint64_t not_IP:1;		  /**< set if the packet may not be IP (must be zero in this case) */
		uint64_t rcv_error:1;		  /**< the receive interface hardware detected a receive error (must be zero in this case) */
		/* lower err_code = first-level descriptor of the work */
		/* zero for packet submitted by hardware that isn't on the slow path */

		uint64_t err_code:8;		  /**< type is cvmx_pip_err_t */
	} s_cn68xx;
	struct {
		uint64_t bufs:8;		  /**< HW sets this to the number of buffers used by this packet */
		uint64_t ip_offset:8;		  /**< HW sets to the number of L2 bytes prior to the IP */
		uint64_t vlan_valid:1;		  /**< set to 1 if we found DSA/VLAN in the L2 */
		uint64_t vlan_stacked:1;	  /**< Set to 1 if the DSA/VLAN tag is stacked */
		uint64_t unassigned:1;
		uint64_t vlan_cfi:1;		  /**< HW sets to the DSA/VLAN CFI flag (valid when vlan_valid) */
		uint64_t vlan_id:12;		  /**< HW sets to the DSA/VLAN_ID field (valid when vlan_valid) */
		uint64_t pr:4;			  /**< Ring Identifier (if PCIe). Requires PIP_GBL_CTL[RING_EN]=1 */
		uint64_t unassigned2a:4;
		uint64_t unassigned2:4;

		uint64_t dec_ipcomp:1;		  /**< the packet needs to be decompressed */
		uint64_t tcp_or_udp:1;		  /**< the packet is either TCP or UDP */
		uint64_t dec_ipsec:1;		  /**< the packet needs to be decrypted (ESP or AH) */
		uint64_t is_v6:1;		  /**< the packet is IPv6 */

		/* (rcv_error, not_IP, IP_exc, is_frag, L4_error, software, etc.) */

		uint64_t software:1;		  /**< reserved for software use, hardware will clear on packet creation */
		/* exceptional conditions below */
		uint64_t L4_error:1;		  /**< the receive interface hardware detected an L4 error (only applies if !is_frag)
                                                    (only applies if !rcv_error && !not_IP && !IP_exc && !is_frag)
                                                    failure indicated in err_code below, decode:
                                                    - 1 = Malformed L4
                                                    - 2 = L4 Checksum Error: the L4 checksum value is
                                                    - 3 = UDP Length Error: The UDP length field would make the UDP data longer than what
                                                        remains in the IP packet (as defined by the IP header length field).
                                                    - 4 = Bad L4 Port: either the source or destination TCP/UDP port is 0.
                                                    - 8 = TCP FIN Only: the packet is TCP and only the FIN flag set.
                                                    - 9 = TCP No Flags: the packet is TCP and no flags are set.
                                                    - 10 = TCP FIN RST: the packet is TCP and both FIN and RST are set.
                                                    - 11 = TCP SYN URG: the packet is TCP and both SYN and URG are set.
                                                    - 12 = TCP SYN RST: the packet is TCP and both SYN and RST are set.
                                                    - 13 = TCP SYN FIN: the packet is TCP and both SYN and FIN are set. */

		uint64_t is_frag:1;		  /**< set if the packet is a fragment */
		uint64_t IP_exc:1;		  /**< the receive interface hardware detected an IP error / exception
                                                    (only applies if !rcv_error && !not_IP) failure indicated in err_code below, decode:
                                                    - 1 = Not IP: the IP version field is neither 4 nor 6.
                                                    - 2 = IPv4 Header Checksum Error: the IPv4 header has a checksum violation.
                                                    - 3 = IP Malformed Header: the packet is not long enough to contain the IP header.
                                                    - 4 = IP Malformed: the packet is not long enough to contain the bytes indicated by the IP
                                                        header. Pad is allowed.
                                                    - 5 = IP TTL Hop: the IPv4 TTL field or the IPv6 Hop Count field are zero.
                                                    - 6 = IP Options */

		uint64_t is_bcast:1;		  /**< set if the hardware determined that the packet is a broadcast */
		uint64_t is_mcast:1;		  /**< set if the hardware determined that the packet is a multi-cast */
		uint64_t not_IP:1;		  /**< set if the packet may not be IP (must be zero in this case) */
		uint64_t rcv_error:1;		  /**< the receive interface hardware detected a receive error (must be zero in this case) */
		/* lower err_code = first-level descriptor of the work */
		/* zero for packet submitted by hardware that isn't on the slow path */

		uint64_t err_code:8;		  /**< type is cvmx_pip_err_t */
	} s_cn38xx;

    /**< use this to get at the 16 vlan bits */
	struct {
		uint64_t unused1:16;
		uint64_t vlan:16;
		uint64_t unused2:32;
	} svlan;

    /**< use this struct if the hardware could not determine that the packet is ip */
	struct {
		uint64_t bufs:8;		  /**< HW sets this to the number of buffers used by this packet */
		uint64_t unused:8;
		uint64_t vlan_valid:1;		  /**< set to 1 if we found DSA/VLAN in the L2 */
		uint64_t vlan_stacked:1;	  /**< Set to 1 if the DSA/VLAN tag is stacked */
		uint64_t unassigned:1;
		uint64_t vlan_cfi:1;		  /**< HW sets to the DSA/VLAN CFI flag (valid when vlan_valid) */
		uint64_t vlan_id:12;		  /**< HW sets to the DSA/VLAN_ID field (valid when vlan_valid) */

		uint64_t varies:12;
				/**< 38xx and 68xx have different definitions.  */
		uint64_t unassigned2:4;

		uint64_t software:1;		  /**< reserved for software use, hardware will clear on packet creation */
		uint64_t unassigned3:1;
		uint64_t is_rarp:1;		  /**< set if the hardware determined that the packet is rarp */
		uint64_t is_arp:1;		  /**< set if the hardware determined that the packet is arp */
		uint64_t is_bcast:1;		  /**< set if the hardware determined that the packet is a broadcast */
		uint64_t is_mcast:1;		  /**< set if the hardware determined that the packet is a multi-cast */
		uint64_t not_IP:1;		  /**< set if the packet may not be IP (must be one in this case) */
		uint64_t rcv_error:1;		  /**< the receive interface hardware detected a receive error.
                                                    Failure indicated in err_code below, decode:
                                                    - 1 = partial error: a packet was partially received, but internal
                                                        buffering / bandwidth was not adequate to receive the entire packet.
                                                    - 2 = jabber error: the RGMII packet was too large and is truncated.
                                                    - 3 = overrun error: the RGMII packet is longer than allowed and had
                                                        an FCS error.
                                                    - 4 = oversize error: the RGMII packet is longer than allowed.
                                                    - 5 = alignment error: the RGMII packet is not an integer number of bytes
                                                        and had an FCS error (100M and 10M only).
                                                    - 6 = fragment error: the RGMII packet is shorter than allowed and had an
                                                        FCS error.
                                                    - 7 = GMX FCS error: the RGMII packet had an FCS error.
                                                    - 8 = undersize error: the RGMII packet is shorter than allowed.
                                                    - 9 = extend error: the RGMII packet had an extend error.
                                                    - 10 = length mismatch error: the RGMII packet had a length that did not
                                                        match the length field in the L2 HDR.
                                                    - 11 = RGMII RX error/SPI4 DIP4 Error: the RGMII packet had one or more
                                                        data reception errors (RXERR) or the SPI4 packet had one or more DIP4
                                                        errors.
                                                    - 12 = RGMII skip error/SPI4 Abort Error: the RGMII packet was not large
                                                        enough to cover the skipped bytes or the SPI4 packet was terminated
                                                        with an About EOPS.
                                                    - 13 = RGMII nibble error/SPI4 Port NXA Error: the RGMII packet had a
                                                        studder error (data not repeated - 10/100M only) or the SPI4 packet
                                                        was sent to an NXA.
                                                    - 16 = FCS error: a SPI4.2 packet had an FCS error.
                                                    - 17 = Skip error: a packet was not large enough to cover the skipped bytes.
                                                    - 18 = L2 header malformed: the packet is not long enough to contain the L2 */

		/* lower err_code = first-level descriptor of the work */
		/* zero for packet submitted by hardware that isn't on the slow path */
		uint64_t err_code:8;	/* type is cvmx_pip_err_t (union, so can't use directly */
	} snoip;
	struct {
		uint64_t bufs:8;		  /**< HW sets this to the number of buffers used by this packet */
		uint64_t unused:8;
		uint64_t vlan_valid:1;		  /**< set to 1 if we found DSA/VLAN in the L2 */
		uint64_t vlan_stacked:1;	  /**< Set to 1 if the DSA/VLAN tag is stacked */
		uint64_t unassigned:1;
		uint64_t vlan_cfi:1;		  /**< HW sets to the DSA/VLAN CFI flag (valid when vlan_valid) */
		uint64_t vlan_id:12;		  /**< HW sets to the DSA/VLAN_ID field (valid when vlan_valid) */

		uint64_t port:12;
			      /**< MAC/PIP port number.  */
		uint64_t unassigned2:4;

		uint64_t software:1;		  /**< reserved for software use, hardware will clear on packet creation */
		uint64_t unassigned3:1;
		uint64_t is_rarp:1;		  /**< set if the hardware determined that the packet is rarp */
		uint64_t is_arp:1;		  /**< set if the hardware determined that the packet is arp */
		uint64_t is_bcast:1;		  /**< set if the hardware determined that the packet is a broadcast */
		uint64_t is_mcast:1;		  /**< set if the hardware determined that the packet is a multi-cast */
		uint64_t not_IP:1;		  /**< set if the packet may not be IP (must be one in this case) */
		uint64_t rcv_error:1;		  /**< the receive interface hardware detected a receive error.
                                                    Failure indicated in err_code below, decode:
                                                    - 1 = partial error: a packet was partially received, but internal
                                                        buffering / bandwidth was not adequate to receive the entire packet.
                                                    - 2 = jabber error: the RGMII packet was too large and is truncated.
                                                    - 3 = overrun error: the RGMII packet is longer than allowed and had
                                                        an FCS error.
                                                    - 4 = oversize error: the RGMII packet is longer than allowed.
                                                    - 5 = alignment error: the RGMII packet is not an integer number of bytes
                                                        and had an FCS error (100M and 10M only).
                                                    - 6 = fragment error: the RGMII packet is shorter than allowed and had an
                                                        FCS error.
                                                    - 7 = GMX FCS error: the RGMII packet had an FCS error.
                                                    - 8 = undersize error: the RGMII packet is shorter than allowed.
                                                    - 9 = extend error: the RGMII packet had an extend error.
                                                    - 10 = length mismatch error: the RGMII packet had a length that did not
                                                        match the length field in the L2 HDR.
                                                    - 11 = RGMII RX error/SPI4 DIP4 Error: the RGMII packet had one or more
                                                        data reception errors (RXERR) or the SPI4 packet had one or more DIP4
                                                        errors.
                                                    - 12 = RGMII skip error/SPI4 Abort Error: the RGMII packet was not large
                                                        enough to cover the skipped bytes or the SPI4 packet was terminated
                                                        with an About EOPS.
                                                    - 13 = RGMII nibble error/SPI4 Port NXA Error: the RGMII packet had a
                                                        studder error (data not repeated - 10/100M only) or the SPI4 packet
                                                        was sent to an NXA.
                                                    - 16 = FCS error: a SPI4.2 packet had an FCS error.
                                                    - 17 = Skip error: a packet was not large enough to cover the skipped bytes.
                                                    - 18 = L2 header malformed: the packet is not long enough to contain the L2 */

		/* lower err_code = first-level descriptor of the work */
		/* zero for packet submitted by hardware that isn't on the slow path */
		uint64_t err_code:8;	/* type is cvmx_pip_err_t (union, so can't use directly */
	} snoip_cn68xx;
	struct {
		uint64_t bufs:8;		  /**< HW sets this to the number of buffers used by this packet */
		uint64_t unused:8;
		uint64_t vlan_valid:1;		  /**< set to 1 if we found DSA/VLAN in the L2 */
		uint64_t vlan_stacked:1;	  /**< Set to 1 if the DSA/VLAN tag is stacked */
		uint64_t unassigned:1;
		uint64_t vlan_cfi:1;		  /**< HW sets to the DSA/VLAN CFI flag (valid when vlan_valid) */
		uint64_t vlan_id:12;		  /**< HW sets to the DSA/VLAN_ID field (valid when vlan_valid) */
		uint64_t pr:4;			  /**< Ring Identifier (if PCIe). Requires PIP_GBL_CTL[RING_EN]=1 */
		uint64_t unassigned2a:8;
		uint64_t unassigned2:4;

		uint64_t software:1;		  /**< reserved for software use, hardware will clear on packet creation */
		uint64_t unassigned3:1;
		uint64_t is_rarp:1;		  /**< set if the hardware determined that the packet is rarp */
		uint64_t is_arp:1;		  /**< set if the hardware determined that the packet is arp */
		uint64_t is_bcast:1;		  /**< set if the hardware determined that the packet is a broadcast */
		uint64_t is_mcast:1;		  /**< set if the hardware determined that the packet is a multi-cast */
		uint64_t not_IP:1;		  /**< set if the packet may not be IP (must be one in this case) */
		uint64_t rcv_error:1;		  /**< the receive interface hardware detected a receive error.
                                                    Failure indicated in err_code below, decode:
                                                    - 1 = partial error: a packet was partially received, but internal
                                                        buffering / bandwidth was not adequate to receive the entire packet.
                                                    - 2 = jabber error: the RGMII packet was too large and is truncated.
                                                    - 3 = overrun error: the RGMII packet is longer than allowed and had
                                                        an FCS error.
                                                    - 4 = oversize error: the RGMII packet is longer than allowed.
                                                    - 5 = alignment error: the RGMII packet is not an integer number of bytes
                                                        and had an FCS error (100M and 10M only).
                                                    - 6 = fragment error: the RGMII packet is shorter than allowed and had an
                                                        FCS error.
                                                    - 7 = GMX FCS error: the RGMII packet had an FCS error.
                                                    - 8 = undersize error: the RGMII packet is shorter than allowed.
                                                    - 9 = extend error: the RGMII packet had an extend error.
                                                    - 10 = length mismatch error: the RGMII packet had a length that did not
                                                        match the length field in the L2 HDR.
                                                    - 11 = RGMII RX error/SPI4 DIP4 Error: the RGMII packet had one or more
                                                        data reception errors (RXERR) or the SPI4 packet had one or more DIP4
                                                        errors.
                                                    - 12 = RGMII skip error/SPI4 Abort Error: the RGMII packet was not large
                                                        enough to cover the skipped bytes or the SPI4 packet was terminated
                                                        with an About EOPS.
                                                    - 13 = RGMII nibble error/SPI4 Port NXA Error: the RGMII packet had a
                                                        studder error (data not repeated - 10/100M only) or the SPI4 packet
                                                        was sent to an NXA.
                                                    - 16 = FCS error: a SPI4.2 packet had an FCS error.
                                                    - 17 = Skip error: a packet was not large enough to cover the skipped bytes.
                                                    - 18 = L2 header malformed: the packet is not long enough to contain the L2 */

		/* lower err_code = first-level descriptor of the work */
		/* zero for packet submitted by hardware that isn't on the slow path */
		uint64_t err_code:8;	/* type is cvmx_pip_err_t (union, so can't use directly */
	} snoip_cn38xx;
#else				/* __LITTLE_ENDIAN_BITFIELD */
	struct {
		uint64_t err_code:8;
		uint64_t rcv_error:1;
		uint64_t not_IP:1;
		uint64_t is_mcast:1;
		uint64_t is_bcast:1;
		uint64_t IP_exc:1;
		uint64_t is_frag:1;
		uint64_t L4_error:1;
		uint64_t software:1;
		uint64_t is_v6:1;
		uint64_t dec_ipsec:1;
		uint64_t tcp_or_udp:1;
		uint64_t dec_ipcomp:1;
		uint64_t varies:12;
		uint64_t vlan_id:12;
		uint64_t vlan_cfi:1;
		uint64_t unassigned:1;
		uint64_t vlan_stacked:1;
		uint64_t vlan_valid:1;
		uint64_t ip_offset:8;
		uint64_t bufs:8;
	} s;
	struct {
		uint64_t err_code:8;
		uint64_t rcv_error:1;
		uint64_t not_IP:1;
		uint64_t is_mcast:1;
		uint64_t is_bcast:1;
		uint64_t IP_exc:1;
		uint64_t is_frag:1;
		uint64_t L4_error:1;
		uint64_t software:1;
		uint64_t is_v6:1;
		uint64_t dec_ipsec:1;
		uint64_t tcp_or_udp:1;
		uint64_t dec_ipcomp:1;
		uint64_t port:12;
		uint64_t vlan_id:12;
		uint64_t vlan_cfi:1;
		uint64_t unassigned:1;
		uint64_t vlan_stacked:1;
		uint64_t vlan_valid:1;
		uint64_t ip_offset:8;
		uint64_t bufs:8;
	} s_cn68xx;
	struct {
		uint64_t err_code:8;
		uint64_t rcv_error:1;
		uint64_t not_IP:1;
		uint64_t is_mcast:1;
		uint64_t is_bcast:1;
		uint64_t IP_exc:1;
		uint64_t is_frag:1;
		uint64_t L4_error:1;
		uint64_t software:1;
		uint64_t is_v6:1;
		uint64_t dec_ipsec:1;
		uint64_t tcp_or_udp:1;
		uint64_t dec_ipcomp:1;
		uint64_t unassigned2:4;
		uint64_t unassigned2a:4;
		uint64_t pr:4;
		uint64_t vlan_id:12;
		uint64_t vlan_cfi:1;
		uint64_t unassigned:1;
		uint64_t vlan_stacked:1;
		uint64_t vlan_valid:1;
		uint64_t ip_offset:8;
		uint64_t bufs:8;
	} s_cn38xx;

	struct {
		uint64_t unused2:32;
		uint64_t vlan:16;
		uint64_t unused1:16;
	} svlan;

	struct {
		uint64_t err_code:8;
		uint64_t rcv_error:1;
		uint64_t not_IP:1;
		uint64_t is_mcast:1;
		uint64_t is_bcast:1;
		uint64_t is_arp:1;
		uint64_t is_rarp:1;
		uint64_t unassigned3:1;
		uint64_t software:1;
		uint64_t unassigned2:4;
		uint64_t varies:12;
		uint64_t vlan_id:12;
		uint64_t vlan_cfi:1;
		uint64_t unassigned:1;
		uint64_t vlan_stacked:1;
		uint64_t vlan_valid:1;
		uint64_t unused:8;
		uint64_t bufs:8;
	} snoip;
	struct {
		uint64_t err_code:8;
		uint64_t rcv_error:1;
		uint64_t not_IP:1;
		uint64_t is_mcast:1;
		uint64_t is_bcast:1;
		uint64_t is_arp:1;
		uint64_t is_rarp:1;
		uint64_t unassigned3:1;
		uint64_t software:1;
		uint64_t unassigned2:4;
		uint64_t port:12;
		uint64_t vlan_id:12;
		uint64_t vlan_cfi:1;
		uint64_t unassigned:1;
		uint64_t vlan_stacked:1;
		uint64_t vlan_valid:1;
		uint64_t unused:8;
		uint64_t bufs:8;
	} snoip_cn68xx;
	struct {
		uint64_t err_code:8;
		uint64_t rcv_error:1;
		uint64_t not_IP:1;
		uint64_t is_mcast:1;
		uint64_t is_bcast:1;
		uint64_t is_arp:1;
		uint64_t is_rarp:1;
		uint64_t unassigned3:1;
		uint64_t software:1;
		uint64_t unassigned2:4;
		uint64_t unassigned2a:8;
		uint64_t pr:4;
		uint64_t vlan_id:12;
		uint64_t vlan_cfi:1;
		uint64_t unassigned:1;
		uint64_t vlan_stacked:1;
		uint64_t vlan_valid:1;
		uint64_t unused:8;
		uint64_t bufs:8;
	} snoip_cn38xx;
#endif				/* __LITTLE_ENDIAN_BITFIELD */
} cvmx_pip_wqe_word2_t;

typedef union {
	uint64_t u64;
#ifdef __BIG_ENDIAN_BITFIELD
	struct {
		uint64_t software:1;		  /**< reserved for software use, hardware always writes 0 to this bit */
		uint64_t lg_hdr_type:5;		  /**< Indicates the Layer G header typed parsed,PKI_LTYPE_E */
		uint64_t lf_hdr_type:5;		  /**< Indicates the Layer F header typed parsed, PKI_LTYPE_E */
		uint64_t le_hdr_type:5;        	  /**< Indicates the Layer E header typed parsed, PKI_LTYPE_E */
		uint64_t ld_hdr_type:5;		  /**< Indicates the Layer D header typed parsed, PKI_LTYPE_E */
		uint64_t lc_hdr_type:5;		  /**< Indicates the Layer C header typed parsed, PKI_LTYPE_E */
		uint64_t lb_hdr_type:5;		  /**< Indicates the Layer B header typed parsed, PKI_LTYPE_E */
		uint64_t is_la_ether:1;	          /**< Indicates that Layer A Ethernet was parsed */
		uint64_t rsvd_0:8;
		uint64_t vlan_valid:1;		  /**< set to 1 if we found VLAN in the L2 */
		uint64_t vlan_stacked:1;	  /**< set to 1 if the VLAN tag is stacked */
		uint64_t stat_inc:1;              /**< stat increment Reserved for Statistics hardware */
		uint64_t pcam_flag4:1;            /**< indicates if PCAM entry has set the flag */
		uint64_t pcam_flag3:1;            /**< indicates if PCAM entry has set the flag */
		uint64_t pcam_flag2:1;            /**< indicates if PCAM entry has set the flag */
		uint64_t pcam_flag1:1;            /**< indicates if PCAM entry has set the flag */
		uint64_t is_frag:1;		  /**< set when the outer IP indicates a fragment */
		uint64_t is_l3_bcast:1;           /**< set when outer ip indicates broadcast */
		uint64_t is_l3_mcast:1;           /**< set when outer ipv4 indicates multicast */
		uint64_t is_l2_bcast:1;           /**< set when the packet’s destination MAC address
							field in the L2 HDR is the broadcast address */
		uint64_t is_l2_mcast:1; 	  /**< set when the packet’s destination MAC address
							field in the L2 HDR is a multicast address */
		uint64_t is_raw:1;                /**< set when PKI_INST_HDR[RAW] was set */
		uint64_t err_level:3;             /**< contains the lowest protocol layer containing error,
							when errors are detected, normally 0; PKI_ERROR_LEVEL_E */
		uint64_t err_code:8;              /**< normally 0, but contains a (non-zero) exception opcode
							enumerated by PKI_OPCODE_E when WQE[ERRLEV] is non-zero */
	};
#else
	struct {
		uint64_t err_code:8;
		uint64_t err_level:3;
		uint64_t is_raw:1;
		uint64_t is_l2_mcast:1;
		uint64_t is_l2_bcast:1;
		uint64_t is_l3_mcast:1;
		uint64_t is_l3_bcast:1;
		uint64_t is_frag:1;
		uint64_t pcam_flag1:1;
		uint64_t pcam_flag2:1;
		uint64_t pcam_flag3:1;
		uint64_t pcam_flag4:1;
		uint64_t stat_inc:1;
		uint64_t vlan_stacked:1;
		uint64_t vlan_valid:1;
		uint64_t rsvd_0:8;
		uint64_t is_la_ether:1;
		uint64_t lb_hdr_type:5;
		uint64_t lc_hdr_type:5;
		uint64_t ld_hdr_type:5;
		uint64_t le_hdr_type:5;
		uint64_t lf_hdr_type:5;
		uint64_t lg_hdr_type:5;
		uint64_t software:1;
	};
#endif
}cvmx_pki_wqe_word2_t;

typedef union {
	uint64_t u64;
	cvmx_pki_wqe_word2_t pki;
	cvmx_pip_wqe_word2_t pip;
}cvmx_wqe_word2_t;

typedef union {
	uint64_t u64;
#ifdef __BIG_ENDIAN_BITFIELD
	struct {
		/**
		 * raw chksum result generated by the HW
		 */
		uint16_t hw_chksum;
		/**
		 * Field unused by hardware - available for software
		 */
		uint8_t unused;
		/**
		 * Next pointer used by hardware for list maintenance.
		 * May be written/read by HW before the work queue
		 * entry is scheduled to a PP (Only 36 bits used in
		 * Octeon 1)
		 */
		uint64_t next_ptr:40;

	} cn38xx;
	struct {
		uint64_t l4ptr:8;	/* 56..63 */
		uint64_t unused0:8;	/* 48..55 */
		uint64_t l3ptr:8;	/* 40..47 */
		uint64_t l2ptr:8;	/* 32..39 */
		uint64_t unused1:18;	/* 14..31 */
		uint64_t bpid:6;	/* 8..13 */
		uint64_t unused2:2;	/* 6..7 */
		uint64_t pknd:6;	/* 0..5 */
	} cn68xx;
#else
	struct {
		uint64_t next_ptr:40;
		uint8_t unused;
		uint16_t hw_chksum;
	} cn38xx;
	struct {
		uint64_t pknd:6;
		uint64_t unused2:2;
		uint64_t bpid:6;
		uint64_t unused1:18;
		uint64_t l2ptr:8;
		uint64_t l3ptr:8;
		uint64_t unused0:8;
		uint64_t l4ptr:8;
	} cn68xx;
#endif
} cvmx_pip_wqe_word0_t;

typedef union {
	uint64_t u64;
#ifdef __BIG_ENDIAN_BITFIELD
	struct {
		uint64_t rsvd_0:4;
		uint64_t aura:12;
		uint64_t rsvd_1:1;
		uint64_t apad:3;
		uint64_t channel:12;
		uint64_t bufs:8;
		uint64_t style:8;
		uint64_t rsvd_2:10;
		uint64_t pknd:6;
	};
#else
	struct {
		uint64_t pknd:6;
		uint64_t rsvd_2:10;
		uint64_t style:8;
		uint64_t bufs:8;
		uint64_t channel:12;
		uint64_t apad:3;
		uint64_t rsvd_1:1;
		uint64_t aura:12;
		uint64_t rsvd_0:4;
	};
#endif
} cvmx_pki_wqe_word0_t;

typedef union {
	uint64_t u64;
	cvmx_pip_wqe_word0_t pip;
	cvmx_pki_wqe_word0_t pki;
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t unused:24;
		uint64_t next_ptr:40;	/* on cn68xx this is unused as well */
#else
		uint64_t next_ptr:40;
		uint64_t unused:24;
#endif
	} raw;
} cvmx_wqe_word0_t;

typedef union {
	uint64_t u64;
#ifdef __BIG_ENDIAN_BITFIELD
	struct {
		uint64_t len:16;
		uint64_t varies:14;
		/**
		 * the type of the tag (ORDERED, ATOMIC, NULL)
		 */
		cvmx_pow_tag_type_t tag_type:2;
		uint64_t tag:32;
	};
	struct {
		uint64_t len:16;
		uint64_t rsvd_0:2;
		uint64_t rsvd_1:2;
		/**
		 * the group that the work queue entry will be scheduled to
		 */
		uint64_t grp:10;
		cvmx_pow_tag_type_t tag_type:2;
		uint64_t tag:32;
	} cn78xx;
	struct {
		uint64_t len:16;
		uint64_t zero_0:1;
		/**
		 * HW sets this to what it thought the priority of the input packet was
		 */
		uint64_t qos:3;

		uint64_t zero_1:1;
		/**
		 * the group that the work queue entry will be scheduled to
		 */
		uint64_t grp:6;
		uint64_t zero_2:3;
		cvmx_pow_tag_type_t tag_type:2;
		uint64_t tag:32;
	} cn68xx;
	struct {
		uint64_t len:16;
		/**
		 * HW sets this to input physical port
		 */
		uint64_t ipprt:6;

		/**
		 * HW sets this to what it thought the priority of the input packet was
		 */
		uint64_t qos:3;

		/**
		 * the group that the work queue entry will be scheduled to
		 */
		uint64_t grp:4;
		uint64_t zero_2:1;
		cvmx_pow_tag_type_t tag_type:2;
		uint64_t tag:32;
	} cn38xx;
#else
	struct {
		uint64_t tag:32;
		cvmx_pow_tag_type_t tag_type:2;
		uint64_t varies:14;
		uint64_t len:16;
	};
	struct {
		uint64_t tag:32;
		cvmx_pow_tag_type_t tag_type:2;
		/**
		 * the group that the work queue entry will be scheduled to
		 */
		uint64_t grp:10;
		uint64_t rsvd_1:2;
		uint64_t rsvd_0:2;
		uint64_t len:16;
	} cn78xx;
	struct {
		uint64_t tag:32;
		cvmx_pow_tag_type_t tag_type:2;
		uint64_t zero_2:3;
		uint64_t grp:6;
		uint64_t zero_1:1;
		uint64_t qos:3;
		uint64_t zero_0:1;
		uint64_t len:16;
	} cn68xx;
	struct {
		uint64_t tag:32;
		cvmx_pow_tag_type_t tag_type:2;
		uint64_t zero_2:1;
		uint64_t grp:4;
		uint64_t qos:3;
		uint64_t ipprt:6;
		uint64_t len:16;
	} cn38xx;
#endif
} cvmx_wqe_word1_t;

/**
 * Work queue entry format
 *
 * must be 8-byte aligned
 */
typedef struct {

    /*****************************************************************
     * WORD 0
     *  HW WRITE: the following 64 bits are filled by HW when a packet arrives
     */

	cvmx_wqe_word0_t word0;

    /*****************************************************************
     * WORD 1
     *  HW WRITE: the following 64 bits are filled by HW when a packet arrives
     */

	cvmx_wqe_word1_t word1;
    /**
     * WORD 2
     *   HW WRITE: the following 64-bits are filled in by hardware when a packet arrives
     *   This indicates a variety of status and error conditions.
     */
	cvmx_pip_wqe_word2_t word2;

    /**
     * Pointer to the first segment of the packet.
     */
	cvmx_buf_ptr_t packet_ptr;

    /**
     *   HW WRITE: octeon will fill in a programmable amount from the
     *             packet, up to (at most, but perhaps less) the amount
     *             needed to fill the work queue entry to 128 bytes
     *   If the packet is recognized to be IP, the hardware starts (except that
     *   the IPv4 header is padded for appropriate alignment) writing here where
     *   the IP header starts.
     *   If the packet is not recognized to be IP, the hardware starts writing
     *   the beginning of the packet here.
     */
	uint8_t packet_data[96];

    /**
     * If desired, SW can make the work Q entry any length. For the
     * purposes of discussion here, Assume 128B always, as this is all that
     * the hardware deals with.
     *
     */

} CVMX_CACHE_LINE_ALIGNED cvmx_wqe_t;

/**
 * Work queue entry format for 78XX
 * In 78XX packet data always resides in WQE buffer unless
 * option DIS_WQ_DAT=1 in PKI_STYLE_BUF, which causes packet data to use
 * separate buffer.
 *
 * must be 8-byte aligned
 */
typedef struct {

    /*****************************************************************
	* WORD 0
	*  HW WRITE: the following 64 bits are filled by HW when a packet arrives
    */

	cvmx_wqe_word0_t word0;

    /*****************************************************************
	* WORD 1
	*  HW WRITE: the following 64 bits are filled by HW when a packet arrives
    */

	cvmx_wqe_word1_t word1;
    /**
	 * WORD 2
	 *   HW WRITE: the following 64-bits are filled in by hardware when a packet arrives
	 *   This indicates a variety of status and error conditions.
     */
	cvmx_wqe_word2_t word2;

    /**
	 * Pointer to the first segment of the packet.
         * WORD 3
     */
	cvmx_buf_ptr_t packet_ptr;

    /**
         * WORD 4
         *   HW WRITE: the following 64-bits are filled in by hardware when a packet arrives
	 *   contains a byte pointer to the start of Layer A/B/C/D/E/F/G relative of start of packet
     */

	cvmx_wqe_word4_t word4;

    /**
         * WORD 5/6/7 may be extended here if WQE_HSZ is set to !0
     */

} CVMX_CACHE_LINE_ALIGNED cvmx_wqe_78xx_t;

static inline int cvmx_wqe_get_port(cvmx_wqe_t *work)
{
	int port;

	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		/* In 78xx wqe entry has channel number not port*/
		port = work->word0.pki.channel; //vinita_to_do
	else if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE))
		port = work->word2.s_cn68xx.port;
	else
		port = work->word1.cn38xx.ipprt;

	return port;
}


static inline void cvmx_wqe_set_port(cvmx_wqe_t *work, int port)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		work->word0.pki.channel = port;
	else if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE))
		work->word2.s_cn68xx.port = port;
	else
		work->word1.cn38xx.ipprt = port;
}

static inline int cvmx_wqe_get_grp(cvmx_wqe_t *work)
{
	int grp;

	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		grp = work->word1.cn78xx.grp;
	else if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE))
		grp = work->word1.cn68xx.grp;
	else
		grp = work->word1.cn38xx.grp;

	return grp;
}

static inline void cvmx_wqe_set_grp(cvmx_wqe_t *work, int grp)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		work->word1.cn78xx.grp = grp;
	if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE))
		work->word1.cn68xx.grp = grp;
	else
		work->word1.cn38xx.grp = grp;
}

static inline int cvmx_wqe_get_qos(cvmx_wqe_t *work)
{
	int qos;

	if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE))
		qos = work->word1.cn68xx.qos;
	else
		qos = work->word1.cn38xx.qos;

	return qos;
}

static inline void cvmx_wqe_set_qos(cvmx_wqe_t *work, int qos)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE))
		work->word1.cn68xx.qos = qos;
	else
		work->word1.cn38xx.qos = qos;
}

static inline int cvmx_wqe_get_len(cvmx_wqe_t *work)
{
	int len;

	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		len = work->word1.cn78xx.len;
	else if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE))
		len = work->word1.cn68xx.len;
	else
		len = work->word1.cn38xx.len;

	return len;
}

static inline void cvmx_wqe_set_len(cvmx_wqe_t *work, int len)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		work->word1.cn78xx.len = len;
	else if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE))
		work->word1.cn68xx.len = len;
	else
		work->word1.cn38xx.len = len;
}

static inline int cvmx_wqe_get_bufs(cvmx_wqe_t *work)
{
	int bufs;

	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		bufs = work->word0.pki.bufs;
	else
		bufs = work->word2.s.bufs;

	return bufs;
}

static inline int cvmx_wqe_get_error(cvmx_wqe_t *work)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		return (((cvmx_wqe_78xx_t*)(work))->word2.pki.err_level);
	else
		return (work->word2.snoip.rcv_error);
}

static inline uint32_t cvmx_wqe_get_tag(cvmx_wqe_t *work)
{
	return work->word1.tag;
}

static inline void cvmx_wqe_set_tag(cvmx_wqe_t *work, uint32_t tag)
{
	work->word1.tag = tag;
}

static inline int cvmx_wqe_get_tt(cvmx_wqe_t *work)
{
	return work->word1.tag_type;
}

static inline void cvmx_wqe_set_tt(cvmx_wqe_t *work, int tt)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		work->word1.cn78xx.tag_type = (cvmx_pow_tag_type_t) tt;
	}
	else if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE)) {
		work->word1.cn68xx.tag_type = (cvmx_pow_tag_type_t) tt;
		work->word1.cn68xx.zero_2 = 0;
	}
	else {
		work->word1.cn38xx.tag_type = (cvmx_pow_tag_type_t) tt;
		work->word1.cn38xx.zero_2 = 0;
	}
}

static inline int cvmx_wqe_get_unused8(cvmx_wqe_t *work)
{
	int len;

	if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE))
		len = work->word0.pip.cn68xx.unused1;
	else
		len = work->word0.pip.cn38xx.unused;

	return len;
}

static inline void cvmx_wqe_set_unused8(cvmx_wqe_t *work, int v)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE))
		work->word0.pip.cn68xx.unused1 = v;
	else
		work->word0.pip.cn38xx.unused = v;
}

static inline int cvmx_wqe_get_channel(cvmx_wqe_t *work)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		return (work->word0.pki.channel);
	else {
		cvmx_dprintf("ERROR: cvmx_wqe_get_channel\n");
		return -1;
	}
}

static inline void cvmx_wqe_set_channel(cvmx_wqe_t *work, int channel)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		work->word0.pki.channel = channel;
	else
		cvmx_dprintf("ERROR: cvmx_wqe_set_channel\n");
}

static inline int cvmx_wqe_get_aura(cvmx_wqe_t *work)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		return (work->word0.pki.aura);
	else {
		cvmx_dprintf("ERROR: cvmx_wqe_get_aura\n");
		return -1;
	}
}

static inline void cvmx_wqe_set_aura(cvmx_wqe_t *work, int aura)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		work->word0.pki.aura = aura;
	else
		cvmx_dprintf("ERROR: cvmx_wqe_set_aura\n");
}

static inline int cvmx_wqe_get_style(cvmx_wqe_t *work)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		return (work->word0.pki.style);
	else {
		cvmx_dprintf("ERROR: cvmx_wqe_get_style\n");
		return -1;
	}
}

static inline void cvmx_wqe_set_style(cvmx_wqe_t *work, int style)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		work->word0.pki.style = style;
	else
		cvmx_dprintf("ERROR: cvmx_wqe_set_style\n");
}

#ifdef	__cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif

#endif /* __CVMX_WQE_H__ */
