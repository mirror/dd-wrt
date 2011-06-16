/*
 * This file is provided under a dual BSD/GPLv2 license.  When using or 
 *   redistributing this file, you may do so under either license.
 * 
 *   GPL LICENSE SUMMARY
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify 
 *   it under the terms of version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
 * 
 *   This program is distributed in the hope that it will be useful, but 
 *   WITHOUT ANY WARRANTY; without even the implied warranty of 
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 *   General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License 
 *   along with this program; if not, write to the Free Software 
 *   Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *   The full GNU General Public License is included in this distribution 
 *   in the file called LICENSE.GPL.
 * 
 *   Contact Information:
 *   Intel Corporation
 * 
 *   BSD LICENSE 
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 *   All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without 
 *   modification, are permitted provided that the following conditions 
 *   are met:
 * 
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided with the 
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its 
 *       contributors may be used to endorse or promote products derived 
 *       from this software without specific prior written permission.
 * 
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * 
 *  version: Security.L.1.0.3-98
 */

/**
 *****************************************************************************
 * @file icp_qat_fw_ipsec.h
 * @defgroup icp_qat_fw_ipsec ICP QAT FW IPsec Service Interface Definitions
 * @ingroup icp_qat_fw
 * $Revision: 0.1 $
 * @description
 *      This file documents structs used to provide the interface to the
 *        IPsec QAT FW service
 *
 *****************************************************************************/

#ifndef __ICP_QAT_FW_IPSEC_H__
#define __ICP_QAT_FW_IPSEC_H__

/*
******************************************************************************
* Include local header files
******************************************************************************
*/

#include "icp_qat_fw.h"

/*
 * ==============================
 * General Notes on the Interface
 * ==============================
 *
 * To be completed for the next release
 *
 * ==============================
 */

/**
 *****************************************************************************
 * @ingroup icp_qat_fw_ipsec
 *        Definition of the IPsec request
 * @description
 *        This is the request added to the ET rings for IPsec servicing
 *
 *****************************************************************************/
typedef struct icp_qat_fw_ipsec_req_s
{
    icp_qat_fw_comn_req_hdr_t comn_req;
    /**< Common interface request */

    uint8_t resrvd;
    /**< Field reserved for future use and must be set to 0 */

    uint8_t tfc_len;
    /**< Note that support for TFC is not needed in the current release. This
     * field may be removed in a future release */

    uint8_t pad_len;
    /**< For outbound processing this field contains the number of IPsec pad
     * bytes to append to the data. If the padding bit is not set it is assumed
     * the client will add the padding */

    uint8_t next_hdr;
    /**< For the outbound case this field contains the next_hdr to add to the
     * ESP trailer */

    uint16_t packet_len;
    /**< For outbound processing this field is set to the input packet length
     * since the packet length in the ICP Descriptor is set to the output
     * packet length */

    uint16_t ipsec_hdr_off;
    /**< Indicates the offset to insert the ESP header for outbound processing
     * For the inbound case it is the offset to the ESP header in the packet.
     * This is an absolute offset from the start of buffer data area  i.e. it
     * includes bufferDataStartOffset value plus outer protocol header sizes.
     */

    uint32_t high_seq_num;
    /**< Upper long word of the IPsec sequence number */

    uint32_t low_seq_num;
    /**< Lower long word of the IPsec sequence number */

    uint64_t reserved;
    /**< Fields reserved for future use and must be set to 0. */

} icp_qat_fw_ipsec_req_t;

/* Private defines */
#define QAT_FW_IPSEC_ET_RESP_RSVD2_LW_SZ     7
/**< @ingroup icp_qat_fw_ipsec
 * define for IPSEC eagle tail reserved2 response in longwords.  */

/**
 *****************************************************************************
 * @ingroup icp_qat_fw_ipsec
 *        Definition of the IPsec Eagle Tail Response
 * @description
 *        This is the response delivered to the ET rings by the IPsec QAT FW
 *      service. For the cases where IPsec returns a response to the abc
 *        interface the common arch response message defined in
 *        icp_arch_interfaces.h will be used
 *
 *****************************************************************************/
typedef struct icp_qat_fw_ipsec_et_resp_s
{
    icp_qat_fw_comn_resp_hdr_t comn_resp;
    /**< Common interface response format see icp_qat_fw.h */

    uint64_t src_address;
    /**< Pointer to buffer containing the result of the ipsec processing. Same
     * as the src address passed in on the request */

    uint32_t spi;
    /**< SA SPI extracted by the QAT from the inbound header */

    uint32_t low_seq_num;
    /**< Lower long word of the sequece number extracted from the inbound
     * packet */

    uint8_t resrvd1;
    /**< Field reserved for future use and must be set to 0 */

    uint8_t resrvd0;
    /**< Field reserved for future use and must be set to 0 */

    uint8_t pad_len;
    /**< Number of pad bytes in the packet. Extracted from the inbound packets
     * pad_len field */

    uint8_t next_hdr;
    /**< Next header field extracted from the inbound packet */

    uint32_t resrvd2[QAT_FW_IPSEC_ET_RESP_RSVD2_LW_SZ];
    /**< Fields reserved for future use and must be set to 0 */

} icp_qat_fw_ipsec_et_resp_t;

/*  Definitions of the bits in the stats flags field of the common response.
 *  The values returned by the IPsec service are given below
 *
 *  COMMON FLAGS
 *  + ===== + -------- + ------------------------------------------------ +
 *  |  Bit  |  15 - 13 |                    12 - 0                        |
 *  + ===== + -------- + ------------------------------------------------ +
 *  | Flags |  Status  |                 RESERVED = 0                     |
 *  + ===== + -------- + ------------------------------------------------ +
 */

/* Bit position of the pad verification bit in the flags */
#define QAT_IPSEC_PAD_VER_STATUS_BITPOS             15
/* Mask to extract the pad verification flag */
#define QAT_IPSEC_PAD_VER_STATUS_MASK               1
/* Bit position of the ICV verification bit in the flags */
#define QAT_IPSEC_ICV_VER_STATUS_BITPOS             14
/*  Mask to extract the ICV verification flag */
#define QAT_IPSEC_ICV_VER_STATUS_MASK               1

#define ICP_QAT_FW_IPSEC_PAD_VER_STATUS_PASS        0
/**< @ingroup icp_qat_fw_ipsec
 * Status flag indicating that the pad verification on the inbound request
 * passed */
#define ICP_QAT_FW_IPSEC_PAD_VER_STATUS_FAIL        1
/**< @ingroup icp_qat_fw_ipsec
 * Status flag indicating that the pad verification on the inbound request
 * failed */

#define ICP_QAT_FW_IPSEC_ICV_VER_STATUS_PASS        0
/**< @ingroup icp_qat_fw_ipsec
 * Status flag indicating that the ICV verification on the inbound request
 * passed */
#define ICP_QAT_FW_IPSEC_ICV_VER_STATUS_FAIL        1
/**< @ingroup icp_qat_fw_ipsec
 * Status flag indicating that the ICV verification on the inbound request
 * failed */

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_ipsec
 *
 * @description
 *        Macro for extraction of the pad verify status returned in the response.
 *
 * @param flags        Flags to extract the status bit from
 *
 *****************************************************************************/
#define ICP_QAT_FW_IPSEC_PAD_VER_STATUS_FLAGS_GET(flags)        \
    QAT_FIELD_GET(flags,QAT_IPSEC_PAD_VER_STATUS_BITPOS,\
                                               QAT_IPSEC_PAD_VER_STATUS_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_ipsec
 *
 * @description
 *        Macro for extraction of the verify status returned in the response.
 *
 * @param flags        Flags to extract the status bit from
 *
 *****************************************************************************/
#define ICP_QAT_FW_IPSEC_ICV_VER_STATUS_FLAGS_GET(flags)        \
    QAT_FIELD_GET(flags,QAT_IPSEC_ICV_VER_STATUS_BITPOS,\
                                               QAT_IPSEC_ICV_VER_STATUS_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_ipsec
 *
 * @description
 *        Macro for setting the Pad verify status of the flags.
 *
 * @param flags        Flags to set with the status
 * @param val        Verify status
 *
 *****************************************************************************/
#define ICP_QAT_FW_IPSEC_PAD_VER_STATUS_SET(flags,val)        \
    QAT_FIELD_SET(flags,val,QAT_IPSEC_PAD_VER_STATUS_BITPOS,\
                                                QAT_IPSEC_PAD_VER_STATUS_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_ipsec
 *
 * @description
 *        Macro for setting the ICV verify status of the flags.
 *
 * @param flags        Flags to set with the status
 * @param val        Verify status
 *
 *****************************************************************************/
#define ICP_QAT_FW_IPSEC_ICV_VER_STATUS_SET(flags,val)        \
    QAT_FIELD_SET(flags,val,QAT_IPSEC_ICV_VER_STATUS_BITPOS,\
                                                QAT_IPSEC_ICV_VER_STATUS_MASK)

/**
 *****************************************************************************
 * @ingroup icp_qat_fw_ipsec
 *        IPsec content descriptor header block
 * @description
 *        Definition of the content descriptor header used in the processing of
 *        the IPsec request
 *
 *****************************************************************************/
typedef struct icp_qat_fw_ipsec_cd_hdr_s
{
    uint8_t next;
    /**< Set to the next slice to pass data through. For IPsec this will either
     * be HASH or CIPHER depending on the mode i.e. inbound or outbound
     * processing */

    uint8_t curr;
    /**< Initialised with the IPsec logical slice type. The logical slice IDs
     * are defined in icp_qat_fw.h. See the icp_qat_fw_slice_t type */

    uint8_t offset;
    /**< Quad word offset from the content descriptor parameters address to the
     * parameters for the IPsec processing. None for IPsec so set to 0 */

    uint8_t resrvd1;
    /**< Reserved byte that must be set to 0 */

    uint8_t inner_hash_res_sz;
    /**< Indicates the number of state bytes to read from the slice
     * of the auth slice and must be a quad word multiple.
     * This is a copy of the inner_state1_sz in icp_qat_fw_auth_hdr_t */

    uint8_t final_hash_res_sz;
    /**< Size of the digest in bytes to be returned to the client. In some cases
     * this is the truncated state returned from the slice starting from byte0.
     * This is a copy of the final_sz in icp_qat_fw_auth_hdr_t */

    uint16_t ipsec_flags;
    /**< IPsec service specific flags used to control the request handling. The
     * bit positions and definitions of the flags are given below */

    uint32_t spi;
    /**< Outbound spi to use for requests */

    uint32_t nonce1;
    /**< Value used in the construction of the CTR for CCM processing see
     *   RFC3610 */

    uint32_t nonce2;
    /**< Value used in the construction of B0 auth block for CCM processing see
     * RFC3610 for usage of the nonce */

    uint32_t ctr_val;
    /**< Initial counter value */

} icp_qat_fw_ipsec_cd_hdr_t;

/*  Definitions of the bits in the flags field. This is NOT relative to the
 *  memory word.
 *
 *  + ===== + ----- + ----- + ----- + ----- + ----- + ----- + ----- +
 *  |  Bit  | 15:12 | 11:10 |   9   |    8  |   7   |   6   |   5   |
 *  + ===== + ----- + ----- + ----- + ----- + ----- + ----- + ----- +
 *  | Flags |  Res  |  Gen  | Post  | Pre-  |  IP   | IPsec | IPsec |
 *  |       |       |  IV   |  IV   |  IV   | Proto | Proto | Mode  |
 *  + ===== + ----- + ----- + ----- + ----- + ----- + ----- + ----- +
 *
 *  + --- + --- + --- + ---- +
 *  |  4  |  3  |  2  | 1-0  |
 *  + --- + --------- + ---- +
 *  | Pad | Dir | ESN | Ciph |
 *  | Enb |     | Enb | Mode |
 *  + --- + --- + --- + ---- +




 */

/* ========================================================================= */
/*                                                    IPsec Flag definitions */
/* ========================================================================= */

/* IPsec Generate IV */

#define ICP_QAT_FW_IPSEC_GEN_NULL_IV            0
/**< @ingroup icp_qat_fw_ipsec
 * Flag indicating that a 0 byte or NULL IV is to be generated */
#define ICP_QAT_FW_IPSEC_GEN_8B_IV                1
/**< @ingroup icp_qat_fw_ipsec
 * Flag indicating that a 8 byte IV is to be generated i.e. for DES */
#define ICP_QAT_FW_IPSEC_GEN_16B_IV                2
/**< @ingroup icp_qat_fw_ipsec
 * Flag indicating that a 16 byte IV is to be generated i.e. for AES CBC */

/* IPsec Post IV */

#define ICP_QAT_FW_IPSEC_POST_IV                1
/**< @ingroup icp_qat_fw_ipsec
 * Flag indicating that there is post IV data for this algorithm */

#define ICP_QAT_FW_IPSEC_NO_POST_IV                0
/**< @ingroup icp_qat_fw_ipsec
 * Flag indicating that there is no post IV data for this algorithm */

/* IPsec Pre IV */

#define ICP_QAT_FW_IPSEC_PRE_IV                    1
/**< @ingroup icp_qat_fw_ipsec
 * Flag indicating that there is pre IV data for this algorithm */

#define ICP_QAT_FW_IPSEC_NO_PRE_IV                0
/**< @ingroup icp_qat_fw_ipsec
 * Flag indicating that there is no pre IV data for this algorithm */

/* IPsec IP Protocol Type Flag */

#define ICP_QAT_FW_IPSEC_IP_PROTO_V4            0
/**< @ingroup icp_qat_fw_ipsec
 * Flag indicating that IPv4 packet is in the request */
#define ICP_QAT_FW_IPSEC_IP_PROTO_V6            1
/**< @ingroup icp_qat_fw_ipsec
 * Flag indicating that IPv6 packet is in the request */

/* IPsec Protocol Type Flag */

#define ICP_QAT_FW_IPSEC_PROTO_TYPE_ESP            0
/**< @ingroup icp_qat_fw_ipsec
 * Flag indicating that type of the IPsec request is ESP. All request must be in
 * ESP mode */
#define ICP_QAT_FW_IPSEC_PROTO_TYPE_AH            1
/**< @ingroup icp_qat_fw_ipsec
 * Flag indicating that type of the IPsec request is AH. Note that AH mode is NOT
 * supported in the current release. The flag is left as is for future proofing */

/* IPsec Mode Flag */

#define ICP_QAT_FW_IPSEC_MODE_TRANSPORT            0
/**< @ingroup icp_qat_fw_ipsec
 * Flag indicating that this is a Transport mode IPsec request */
#define ICP_QAT_FW_IPSEC_MODE_TUNNEL            1
/**< @ingroup icp_qat_fw_ipsec
 * Flag indicating that this is a Tunnel mode IPsec request */

/* IPsec Pad Enable Flag */
/* Also used to signify if the pad verification is completed */

#define ICP_QAT_FW_IPSEC_PAD_DISABLE            0
/**< @ingroup icp_qat_fw_ipsec
 * Flag indicating that the usage of padding is disabled for the request */
#define ICP_QAT_FW_IPSEC_PAD_ENABLE                1
/**< @ingroup icp_qat_fw_ipsec
 * Flag indicating that the usage of padding is enabled for the request */

/* IPsec Direction Flag */

#define ICP_QAT_FW_IPSEC_DIR_IN                    0
/**< @ingroup icp_qat_fw_ipsec
 * Flag indicating that the request is an inbound one */
#define ICP_QAT_FW_IPSEC_DIR_OUT                1
/**< @ingroup icp_qat_fw_ipsec
 * Flag indicating that the request is an outbound one */

/* IPsec ESN Enable Flag */

#define ICP_QAT_FW_IPSEC_ESN_DISABLE            0
/**< @ingroup icp_qat_fw_ipsec
 * Flag indicating that the usage of extended sequence numbers are disabled for the
 * request */
#define ICP_QAT_FW_IPSEC_ESN_ENABLE                1
/**< @ingroup icp_qat_fw_ipsec
 * Flag indicating that the usage of extended sequence numbers are enabled for the
 * request */

/* IPsec Cipher Mode Flag */

#define ICP_QAT_FW_IPSEC_CIPHER_DEFAULT            0
/**< @ingroup icp_qat_fw_ipsec
 * Flag indicating that this is a default cipher mode used for the IPsec request.
 * These are NULL, DES, AES */
#define ICP_QAT_FW_IPSEC_CIPHER_AES_CCM            1
/**< @ingroup icp_qat_fw_ipsec
 * Flag indicating that this is a AES CCM request */
#define ICP_QAT_FW_IPSEC_CIPHER_AES_GCM            2
/**< @ingroup icp_qat_fw_ipsec
 * Flag indicating that this is a AES GCM request */

/* Private defines */
#define QAT_IPSEC_GEN_IV_BITPOS                    10
/**< @ingroup icp_qat_fw_ipsec
 * Starting bit position of size of IV to generate. */

#define QAT_IPSEC_GEN_IV_MASK                    0x3
/**< @ingroup icp_qat_fw_ipsec
 * Mask of two bits used to determine the IV generation.*/

#define QAT_IPSEC_POST_IV_BITPOS                9
/**< @ingroup icp_qat_fw_ipsec
 * Starting bit position of whether the Post IV 4 byte should be applied
 * to the generated IV */

#define QAT_IPSEC_POST_IV_MASK                    0x1
/**< @ingroup icp_qat_fw_ipsec
 * One bit mask used to determine POST/NO_POST */

#define QAT_IPSEC_PRE_IV_BITPOS                    8
/**< @ingroup icp_qat_fw_ipsec
 * Starting bit position of whether the Pre IV 4 byte should be applied
 * to the generated IV */

#define QAT_IPSEC_PRE_IV_MASK                    0x1
/**< @ingroup icp_qat_fw_ipsec
 * One bit mask used to determine PRE/NO_PRE*/

#define QAT_IPSEC_IP_PROTO_BITPOS                7
/**< @ingroup icp_qat_fw_ipsec
 * Starting bit position determinig the IP protocol type (v4/v6) */

#define QAT_IPSEC_IP_PROTO_MASK                    0x1
/**< @ingroup icp_qat_fw_ipsec
 * One bit mask used to determine the IP protocol */

#define QAT_IPSEC_PROTO_TYPE_BITPOS                6
/**< @ingroup icp_qat_fw_ipsec
 * Starting bit position determinig the IPsec proto (ESP/AH) */

#define QAT_IPSEC_PROTO_TYPE_MASK                0x1
/**< @ingroup icp_qat_fw_ipsec
 * One bit mask used to determine the IPsec proto */

#define QAT_IPSEC_MODE_BITPOS                    5
/**< @ingroup icp_qat_fw_ipsec
 * Starting bit position determinig the IPsec transport mode */

#define QAT_IPSEC_MODE_MASK                        0x1
/**< @ingroup icp_qat_fw_ipsec
 * One bit mask used to determine the IPsec transport mode */

#define QAT_IPSEC_PAD_BITPOS                    4
/**< @ingroup icp_qat_fw_ipsec
 * Starting bit position determinig the IPsec padding enabled/disabled */

#define QAT_IPSEC_PAD_MASK                        0x1
/**< @ingroup icp_qat_fw_ipsec
 * One bit mask used to determine the IPsec padding/no_padding */

#define QAT_IPSEC_DIR_BITPOS                    3
/**< @ingroup icp_qat_fw_ipsec
 * Starting bit position determinig the IPsec direction inbound/outbound */

#define QAT_IPSEC_DIR_MASK                        0x1
/**< @ingroup icp_qat_fw_ipsec
 * One bit mask used to determine the IPsec inbound/outbound */

#define QAT_IPSEC_ESN_BITPOS                    2
/**< @ingroup icp_qat_fw_ipsec
 * Starting bit position determinig the IPsec Extended Sequence Number
 * enabled/disabled */

#define QAT_IPSEC_ESN_MASK                        0x1
/**< @ingroup icp_qat_fw_ipsec
 * One bit mask used to determine the ESN enabled/disabled */

#define QAT_IPSEC_CIPHER_MODE_BITPOS            0
/**< @ingroup icp_qat_fw_ipsec
 * Starting bit positin determining IPsec cipher mode encoding*/

#define QAT_IPSEC_CIPHER_MODE_MASK                0x3
/**< @ingroup icp_qat_fw_ipsec
 * Two bit mask used to determine the Cipher mode. */

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_ipsec
 *
 * @description
 * Generic macro used to build the IPsec flags field. This should always be used
 * for creating this field. No direct set, gets or masks should be applied to the
 * flags field. The macros should be used instead
 *
 * @param gen        IV Generate size
 * @param post        Post IV generation required
 * @param pre        Pre IV generation required
 * @param ip        IP header type used, either ipv4 or ipv6
 * @param type        IPsec processing type, always ESP. AH not supported
 * @param mode        Tunnel or transport mode condition
 * @param pad        Condition indicating if padding is required
 * @param dir        Direction of the processing. Either inbound or outbound
 * @param esn        Condition to indicate if extended sequence numbering is to
 *                    be used
 * @param cipher    Cipher mode to use. Normal, CCM or GCM
 *
 *****************************************************************************/

 
#define ICP_QAT_FW_IPSEC_FLAGS_BUILD(gen, post, pre, ip, type, mode, pad,     \
                     dir, esn, cipher)                           \
    ((((gen) & QAT_IPSEC_GEN_IV_MASK) << QAT_IPSEC_GEN_IV_BITPOS)           | \
     (((post) & QAT_IPSEC_POST_IV_MASK) << QAT_IPSEC_POST_IV_BITPOS)       | \
     (((pre) & QAT_IPSEC_PRE_IV_MASK) << QAT_IPSEC_PRE_IV_BITPOS)          | \
     (((ip) & QAT_IPSEC_IP_PROTO_MASK) << QAT_IPSEC_IP_PROTO_BITPOS)       | \
     (((type) & QAT_IPSEC_PROTO_TYPE_MASK) << QAT_IPSEC_PROTO_TYPE_BITPOS) | \
     (((mode) & QAT_IPSEC_MODE_MASK) << QAT_IPSEC_MODE_BITPOS)           | \
     (((pad) & QAT_IPSEC_PAD_MASK) << QAT_IPSEC_PAD_BITPOS)                | \
     (((dir) & QAT_IPSEC_DIR_MASK) << QAT_IPSEC_DIR_BITPOS)                | \
     (((esn) & QAT_IPSEC_ESN_MASK) << QAT_IPSEC_ESN_BITPOS)                | \
     (((cipher) & QAT_IPSEC_CIPHER_MODE_MASK)  << QAT_IPSEC_CIPHER_MODE_BITPOS))

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_ipsec
 *
 * @description
 *        Macro for extraction of the IPsec Generate IV state value
 *
 * @param flags        Flags to extract the Gen IV state from
 *
 *****************************************************************************/
#define ICP_QAT_FW_IPSEC_GEN_IV_GET(flags)        \
    QAT_FIELD_GET(flags,QAT_IPSEC_GEN_IV_BITPOS,QAT_IPSEC_GEN_IV_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_ipsec
 *
 * @description
 *        Macro for extraction of the IPsec Post IV state value
 *
 * @param flags        Flags to extract the Post IV state from
 *
 *****************************************************************************/
#define ICP_QAT_FW_IPSEC_POST_IV_GET(flags)        \
    QAT_FIELD_GET(flags,QAT_IPSEC_POST_IV_BITPOS,QAT_IPSEC_POST_IV_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_ipsec
 *
 * @description
 *        Macro for extraction of the IPsec Pre IV state value
 *
 * @param flags        Flags to extract the Pre IV state from
 *
 *****************************************************************************/
#define ICP_QAT_FW_IPSEC_PRE_IV_GET(flags)        \
    QAT_FIELD_GET(flags,QAT_IPSEC_PRE_IV_BITPOS,QAT_IPSEC_PRE_IV_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_ipsec
 *
 * @description
 *        Macro for extraction of the IP header type returned in the response. 
 *
 * @param flags        Flags to extract the IP header type from
 *
 *****************************************************************************/
#define ICP_QAT_FW_IPSEC_IP_HDR_TYPE_GET(flags)        \
    QAT_FIELD_GET(flags,QAT_IPSEC_IP_PROTO_BITPOS,QAT_IPSEC_IP_PROTO_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_ipsec
 *
 * @description
 *        Macro for extraction of the IPsec processing type returned in the 
 *        response. 
 *
 * @param flags        Flags to extract the IPsec processing type from
 *
 *****************************************************************************/
#define ICP_QAT_FW_IPSEC_PROC_TYPE_GET(flags)        \
    QAT_FIELD_GET(flags,QAT_IPSEC_PROTO_TYPE_BITPOS,QAT_IPSEC_PROTO_TYPE_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_ipsec
 *
 * @description
 *        Macro for extraction of the IPsec header mode returned in the 
 *        response. 
 *
 * @param flags        Flags to extract the IPsec header mode from
 *
 *****************************************************************************/
#define ICP_QAT_FW_IPSEC_HDR_MODE_GET(flags)        \
    QAT_FIELD_GET(flags,QAT_IPSEC_MODE_BITPOS,QAT_IPSEC_MODE_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_ipsec
 *
 * @description
 *        Macro for extraction of the IPsec padding mode returned in the 
 *        response. 
 *
 * @param flags        Flags to extract the IPsec padding mode from
 *
 *****************************************************************************/
#define ICP_QAT_FW_IPSEC_PAD_MODE_GET(flags)        \
    QAT_FIELD_GET(flags,QAT_IPSEC_PAD_BITPOS,QAT_IPSEC_PAD_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_ipsec
 *
 * @description
 *        Macro for extraction of the IPsec direction returned in the 
 *        response. 
 *
 * @param flags        Flags to extract the IPsec processing direction mode from
 *
 *****************************************************************************/
#define ICP_QAT_FW_IPSEC_PROC_DIR_GET(flags)        \
    QAT_FIELD_GET(flags,QAT_IPSEC_DIR_BITPOS,QAT_IPSEC_DIR_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_ipsec
 *
 * @description
 *        Macro for extraction of the extended sequencing number mode returned
 *        in the response. 
 *
 * @param flags        Flags to extract the IPsec ESN mode from
 *
 *****************************************************************************/
#define ICP_QAT_FW_IPSEC_ESN_MODE_GET(flags)        \
    QAT_FIELD_GET(flags,QAT_IPSEC_ESN_BITPOS,QAT_IPSEC_ESN_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_ipsec
 *
 * @description
 *        Macro for extraction of the IPsec cipher processing mode
 *
 * @param flags        Flags to extract the IPsec cipher processing mode from
 *
 *****************************************************************************/
#define ICP_QAT_FW_IPSEC_CIPHER_MODE_GET(flags)        \
    QAT_FIELD_GET(flags,QAT_IPSEC_CIPHER_MODE_BITPOS,QAT_IPSEC_CIPHER_MODE_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_ipsec
 *
 * @description
 *        Macro for setting the Generate IV type of the flags. 
 *
 * @param flags        Flags to set with the Gen IV size
 * @param val        Generate IV size
 *
 *****************************************************************************/
#define ICP_QAT_FW_IPSEC_GEN_IV_SET(flags,val)        \
    QAT_FIELD_SET(flags,val,QAT_IPSEC_GEN_IV_BITPOS,QAT_IPSEC_GEN_IV_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_ipsec
 *
 * @description
 *        Macro for setting the Post IV type of the flags. 
 *
 * @param flags        Flags to set with the Post IV
 * @param val        Post IV value
 *
 *****************************************************************************/
#define ICP_QAT_FW_IPSEC_POST_IV_SET(flags,val)        \
    QAT_FIELD_SET(flags,val,QAT_IPSEC_POST_IV_BITPOS,QAT_IPSEC_POST_IV_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_ipsec
 *
 * @description
 *        Macro for setting the Pre IV type of the flags. 
 *
 * @param flags        Flags to set with the Pre IV
 * @param val        Pre IV value
 *
 *****************************************************************************/
#define ICP_QAT_FW_IPSEC_PRE_IV_SET(flags,val)        \
    QAT_FIELD_SET(flags,val,QAT_IPSEC_PRE_IV_BITPOS,QAT_IPSEC_PRE_IV_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_ipsec
 *
 * @description
 *        Macro for setting the IP header type of the flags. 
 *
 * @param flags        Flags to set with the IP header type
 * @param val        IP header value
 *
 *****************************************************************************/
#define ICP_QAT_FW_IPSEC_IP_HDR_TYPE_SET(flags,val)        \
    QAT_FIELD_SET(flags,val,QAT_IPSEC_IP_PROTO_BITPOS,QAT_IPSEC_IP_PROTO_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_ipsec
 *
 * @description
 *        Macro for setting the IPsec processing type of the flags
 *
 * @param flags        Flags to set with the IPsec processing type
 * @param val        Processing type value
 *
 *****************************************************************************/
#define ICP_QAT_FW_IPSEC_PROC_TYPE_SET(flags,val)        \
    QAT_FIELD_SET(flags,val,QAT_IPSEC_PROTO_TYPE_BITPOS,    \
                        QAT_IPSEC_PROTO_TYPE_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_ipsec
 *
 * @description
 *        Macro for setting the IPsec header mode of the flags
 *
 * @param flags        Flags to set with the IPsec header mode
 * @param val        Header mode value
 *
 *****************************************************************************/
#define ICP_QAT_FW_IPSEC_HDR_MODE_SET(flags,val)        \
    QAT_FIELD_SET(flags,val,QAT_IPSEC_MODE_BITPOS,QAT_IPSEC_MODE_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_ipsec
 *
 * @description
 *        Macro for setting the IPsec padding mode of the flags
 *
 * @param flags        Flags to set with the IPsec padding mode
 * @param val        Padding Mode value
 *
 *****************************************************************************/
#define ICP_QAT_FW_IPSEC_PAD_MODE_SET(flags,val)        \
    QAT_FIELD_SET(flags,val,QAT_IPSEC_PAD_BITPOS,QAT_IPSEC_PAD_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_ipsec
 *
 * @description
 *        Macro for setting the IPsec direction of the flags
 *
 * @param flags        Flags to set with the IPsec processing direction mode
 * @param val        Processing Direction value
 *
 *****************************************************************************/
#define ICP_QAT_FW_IPSEC_PROC_DIR_SET(flags,val)        \
    QAT_FIELD_SET(flags,val,QAT_IPSEC_DIR_BITPOS,QAT_IPSEC_DIR_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_ipsec
 *
 * @description
 *        Macro for setting the extended sequencing number mode of the flags
 *
 * @param flags        Flags to set with the IPsec ESN mode
 * @param val        ESN value
 *
 *****************************************************************************/
#define ICP_QAT_FW_IPSEC_ESN_MODE_SET(flags,val)        \
    QAT_FIELD_SET(flags,val,QAT_IPSEC_ESN_BITPOS,QAT_IPSEC_ESN_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_ipsec
 *
 * @description
 *        Macro for setting the IPsec cipher processing mode of the flags
 *
 * @param flags        Flags to set with the IPsec cipher processing mode
 * @param val        Cipher mode value
 *
 *****************************************************************************/
#define ICP_QAT_FW_IPSEC_CIPHER_MODE_SET(flags,val)        \
    QAT_FIELD_SET(flags,val,QAT_IPSEC_CIPHER_MODE_BITPOS,    \
                        QAT_IPSEC_CIPHER_MODE_MASK)

#endif /* __ICP_QAT_FW_IPSEC_H__ */
