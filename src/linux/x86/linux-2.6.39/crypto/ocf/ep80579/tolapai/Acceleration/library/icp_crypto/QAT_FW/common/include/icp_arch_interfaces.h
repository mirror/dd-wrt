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
 * @file icp_arch_interfaces.h
 * @defgroup icp_arch_ifs ICP Architectural Interface Definitions
 * @ingroup icp_arch
 * $Revision: 0.1 $
 * @description
 *      This file documents the external ring interfaces provides by the icp
 *      sw layers.
 *
 *****************************************************************************/

#ifndef __ICP_ARCH_INTERFACES_H__
#define __ICP_ARCH_INTERFACES_H__

/*****************************************************************************
 *
 * Contents:
 *
 *      Definitions of the following exported macros:
 *
 *      + ICP_ARCH_IF_FLAGS_BUILD(valid,ring_type,resp_type)
 *      + ICP_ARCH_IF_VALID_FLAG_GET(flags)
 *      + ICP_ARCH_IF_RESP_RING_TYPE_GET(flags)
 *      + ICP_ARCH_IF_RESP_TYPE_GET(flags)
 *      + ICP_ARCH_IF_RESP_PKT_OFFSET_GET(flags)
 *      + ICP_ARCH_IF_VALID_FLAG_SET(flags,val)
 *      + ICP_ARCH_IF_RESP_RING_TYPE_SET(flags,val)
 *      + ICP_ARCH_IF_RESP_TYPE_SET(flags,val)
 *      + ICP_ARCH_IF_RESP_PKT_OFFSET_SET(flags,val)
 *
 *****************************************************************************/

/*
 ******************************************************************************
 * Include public/global header files
 ******************************************************************************
 */


/**
 *****************************************************************************
 * @ingroup icp_arch_ifs
 *      Response Pipe Identifier type
 *
 * @description
 *      ID used to indicate the response pipe to deliver a response on. The
 *      pipe-id is passed to the processing element in the request in order to
 *      route the response to the correct pipe. See the ICP SAS for further
 *      info
 *
 *****************************************************************************/

typedef uint8_t icp_arch_if_resp_pipe_id_t;

/**
 *****************************************************************************
 * @ingroup icp_arch_ifs
 *      Response Destination Identifier type
 *
 * @description
 *      ID passed untouched from the request to the response by a processing
 *      element. Allows clients processing the responses make a decision on
 *      what to do next.
 *
 *****************************************************************************/

typedef uint8_t icp_arch_if_resp_dest_id_t;

/**
 *****************************************************************************
 * @ingroup icp_arch_ifs
 *      Common S-interface flags type
 *
 * @description
 *      Flags used to describe the arch layer request and response processing
 *      required. Bit field definitions for the flags are given below.
 *
 *****************************************************************************/
typedef uint8_t icp_arch_if_flags_t;

#define ICP_ARCH_IF_REQ_VALID_SET                               1
/**< @ingroup icp_arch_ifs
 *  Defintion of the valid bit set for a request */
#define ICP_ARCH_IF_REQ_VALID_CLR                               0
/**< @ingroup icp_arch_ifs
 *  Defintion of the valid bit clear for a request */

#define ICP_ARCH_IF_ET_RING_RESP                                2
/**< @ingroup icp_arch_ifs
 *  Defintion of the EagleTail Ring response type */
#define ICP_ARCH_IF_NN_RING_RESP                                1
/**< @ingroup icp_arch_ifs
 *  Defintion of the Next Neighbour Ring response type */
#define ICP_ARCH_IF_SCRATCH_RING_RESP                           0
/**< @ingroup icp_arch_ifs
 *  Defintion of the Scratch Ring response type */

#define ICP_ARCH_IF_A_RESP                                      0
/**< @ingroup icp_arch_ifs
 *  Definition of the A interface */
#define ICP_ARCH_IF_B_RESP                                      1
/**< @ingroup icp_arch_ifs
 *  Definition of the B interface */
#define ICP_ARCH_IF_C_RESP                                      2
/**< @ingroup icp_arch_ifs
 *  Definition of the C interface */
#define ICP_ARCH_IF_S_RESP                                      3
/**< @ingroup icp_arch_ifs
 * Definition of the S interface. The only one currently used by the
 * CPM FW */

/*  The following bits indicate Service Request flag definitions.
 *  The bit offsets below are within the flags field. These are NOT
 *  relative to the memory word. Unused field must be zeroed.
 *
 *  + ===== + ----- + --------- + --------- + --------- +
 *  |  Bit  |   7   |   6 - 5   |   4 - 3   |   2 - 0   |
 *  + ===== + ----- + --------- + --------- + --------- +
 *  | Flags | Valid |   Resvd   | Resp Ring | Resp Type |
 *  |       |       |           |    Type   |           |
 *  + ===== + ----- + --------- + --------- + --------- +
 */

/* Private flags */
#define ICP_ARCH_IF_VALID_FLAG_BITPOS              7
#define ICP_ARCH_IF_VALID_FLAG_MASK                0x1

#define ICP_ARCH_IF_RESP_RING_TYPE_BITPOS          3
#define ICP_ARCH_IF_RESP_RING_TYPE_MASK            0x3

#define ICP_ARCH_IF_RESP_TYPE_BITPOS               0
#define ICP_ARCH_IF_RESP_TYPE_MASK                 0x7


/*  The following bits indicate Service Response flag definitions.
 *
 *  + ===== + ----- + --------- + --------- +
 *  |  Bit  |   7   |   6 - 3   |   2 - 0   |
 *  + ===== + ----- + --------- + --------- +
 *  | Flags | Valid |   Packet  |   Resv    |
 *  |       |       |   Offset  |           |
 *  + ===== + ----- + --------- + --------- +
 */

/* Private flags */
#define ICP_ARCH_IF_RESP_PKT_OFFSET_BITPOS         3
#define ICP_ARCH_IF_RESP_PKT_OFFSET_MASK           0x0F


/* Big assumptions that both bitpos and mask are constants */

#define ARCH_FIELD_SET(flags,val,bitpos,mask) \
        (flags) = (((flags) & (~((mask) << (bitpos)))) | \
        (((val) & (mask)) << (bitpos)))

#define ARCH_FIELD_GET(flags,bitpos,mask) \
        (((flags) >> (bitpos)) & (mask))


/**
 ******************************************************************************
 * @ingroup icp_arch_ifs
 *
 * @description
 *      Build the flags field for the S interface request
 *
 * @param valid     Ring entry valid flag
 * @param ring_type Ring type to send the response to
 * @param resp_type Response interface to writeback to
 *
 *****************************************************************************/
#define ICP_ARCH_IF_FLAGS_BUILD(valid,ring_type,resp_type)                          \
        ((((valid) & ICP_ARCH_IF_VALID_FLAG_MASK) << ICP_ARCH_IF_VALID_FLAG_BITPOS) | \
         (((ring_type) & ICP_ARCH_IF_RESP_RING_TYPE_MASK) << \
           ICP_ARCH_IF_RESP_RING_TYPE_BITPOS) | \
         (((resp_type) & ICP_ARCH_IF_RESP_TYPE_MASK) << ICP_ARCH_IF_RESP_TYPE_BITPOS))

/**
 ******************************************************************************
 * @ingroup icp_arch_ifs
 *
 * @description
 *      Extract the valid flag for the request
 *
 * @param flags  Flags to extract the valid bit from
 *
 *****************************************************************************/
#define ICP_ARCH_IF_VALID_FLAG_GET(flags) \
        ARCH_FIELD_GET(flags,ICP_ARCH_IF_VALID_FLAG_BITPOS,ICP_ARCH_IF_VALID_FLAG_MASK)

/**
 ******************************************************************************
 * @ingroup icp_arch_ifs
 *
 * @description
 *      Extract the rint type from the request
 *
 * @param flags  Flags to extract the ring type from
 *
 *****************************************************************************/
#define ICP_ARCH_IF_RESP_RING_TYPE_GET(flags) \
        ARCH_FIELD_GET(flags,ICP_ARCH_IF_RESP_RING_TYPE_BITPOS, \
                       ICP_ARCH_IF_RESP_RING_TYPE_MASK)

/**
 ******************************************************************************
 * @ingroup icp_arch_ifs
 *
 * @description
 *      Extract the response interface type for the request
 *
 * @param flags  Flags to extract the response interface type from
 *
 *****************************************************************************/
#define ICP_ARCH_IF_RESP_TYPE_GET(flags) \
        ARCH_FIELD_GET(flags,ICP_ARCH_IF_RESP_TYPE_BITPOS, \
                       ICP_ARCH_IF_RESP_TYPE_MASK)

/**
 ******************************************************************************
 * @ingroup icp_arch_ifs
 *
 * @description
 *      Extract the packet offset from the request message flag
 *
 * @param flags  Flags to extract the packet offset bit from
 *
 *****************************************************************************/
#define ICP_ARCH_IF_RESP_PKT_OFFSET_GET(flags) \
        ARCH_FIELD_GET(flags,ICP_ARCH_IF_RESP_PKT_OFFSET_BITPOS, \
                       ICP_ARCH_IF_RESP_PKT_OFFSET_MASK)


/**
 ******************************************************************************
 * @ingroup icp_arch_ifs
 *
 * @description
 *      Set the valid bit in the request and response flags
 *
 * @param flags  Flags to set with the valid bit
 * @param val    Value of the valid bit
 *
 *****************************************************************************/
#define ICP_ARCH_IF_VALID_FLAG_SET(flags,val) \
        ARCH_FIELD_SET((flags),(val),ICP_ARCH_IF_VALID_FLAG_BITPOS, \
                       ICP_ARCH_IF_VALID_FLAG_MASK)

/**
 ******************************************************************************
 * @ingroup icp_arch_ifs
 *
 * @description
 *      Set the ring type for the request
 *
 * @param flags  Flags to set with the ring type
 * @param val    Ring type value
 *
 *****************************************************************************/
#define ICP_ARCH_IF_RESP_RING_TYPE_SET(flags,val) \
        ARCH_FIELD_SET((flags),(val),ICP_ARCH_IF_RESP_RING_TYPE_BITPOS, \
                       ICP_ARCH_IF_RESP_RING_TYPE_MASK)

/**
 ******************************************************************************
 * @ingroup icp_arch_ifs
 *
 * @description
 *      Set the response interface type for the request
 *
 * @param flags  Flags to set with the response interface type
 * @param val    Response Interface type value
 *
 *****************************************************************************/
#define ICP_ARCH_IF_RESP_TYPE_SET(flags,val) \
        ARCH_FIELD_SET((flags),(val),ICP_ARCH_IF_RESP_TYPE_BITPOS, \
                       ICP_ARCH_IF_RESP_TYPE_MASK)

/**
 ******************************************************************************
 * @ingroup icp_arch_ifs
 *
 * @description
 *      Set the packet offset in the response flags
 *
 * @param flags  Flags to set with the packet offset
 * @param val    Value of the packet offset
 *
 *****************************************************************************/
#define ICP_ARCH_IF_RESP_PKT_OFFSET_SET(flags,val) \
        ARCH_FIELD_SET((flags),(val),ICP_ARCH_IF_RESP_PKT_OFFSET_BITPOS, \
                       ICP_ARCH_IF_RESP_PKT_OFFSET_MASK)


/**
 *****************************************************************************
 * @ingroup icp_arch_ifs
 *      ICP sw architecture S interface request header
 * @description
 *      Standard request header information needed by all S interface
 *      processing elements
 *
 *****************************************************************************/
typedef struct icp_arch_if_req_hdr_s
{
    icp_arch_if_flags_t flags;
    /**< Request and response control flags */

    uint8_t req_type;
    /**< Definition of the service described by the request */

    icp_arch_if_resp_pipe_id_t resp_pipe_id;
    /**< Response pipe to write the response associated with this request to */

    icp_arch_if_resp_dest_id_t resp_dest_id;
    /**< Opaque identifier passed from the request to response to allow
     * response handler perform any further processing */

} icp_arch_if_req_hdr_t;

/**
 *****************************************************************************
 * @ingroup icp_arch_ifs
 *      ICP sw architecture S interface response header
 * @description
 *      Standard response header generated by a S interface processing element
 *
 *****************************************************************************/
typedef struct icp_arch_if_resp_hdr_s
{
    icp_arch_if_flags_t flags;
    /**< Request and response control flags */

    uint8_t resp_type;
    /**< Definition of the service described by the request */

    uint8_t serv_id;
    /**< Definition of the service id generating the response */

    uint8_t dest_id;
    /**< Opaque identifier passed from the request to response to allow
     * response handler perform any further processing */

} icp_arch_if_resp_hdr_t;


/**
 *****************************************************************************
 * @ingroup icp_arch_ifs
 *      ICP sw architecture ABC Interface response structure
 * @description
 *      Definition of the response message to generate for an ABC interface
 *
 *****************************************************************************/
typedef struct icp_arch_if_abc_resp_s
{
    icp_arch_if_flags_t flags;
    /**< Response control flags */

    uint8_t pkt_type;
    /**< Packet type set to the resp_dest_id from the request */

    uint8_t pkt_priority;
    /**< Packet priority that will always be set to 0 by the S interface */

    uint8_t hdr_type;
    /**< Header type. Unused by the S interface when generating a response */

    uint16_t if_id;
    /**< Interface ID extracted from the interface ID of the source buffer
         * descriptor */

    uint16_t pkt_sz;
    /**< Packet size field. Set to the mpkt_len of the source buffer
     * descriptor */

     uint64_t pkt_handle;
    /**< Handle of the SOP source buffer descriptor from the request */

} icp_arch_if_abc_resp_t;

#endif /* ICP_ARCH_INTERFACES_H */
