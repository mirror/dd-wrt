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
 * @file icp_qat_fw.h
 * @defgroup icp_qat_fw_comn ICP QAT FW Common Processing Definitions
 * @ingroup icp_qat_fw
 *
 * @description
 *      This file documents the common interfaces that the QAT FW running on
 *      the QAT AE exports. This common layer is used by a number of services
 *      to export content processing services.
 *
 *****************************************************************************/

#ifndef __ICP_QAT_FW_H__
#define __ICP_QAT_FW_H__

/*
* ==============================
* General Notes on the Interface
*/

/*
*
* ==============================
*
* Introduction
*
* Data movement and slice chaining
*
* Endianness
*      - Unless otherwise stated all structures are defined in BIG ENDIAN MODE
*
* Alignment
*      - In general all data structures provided to a request should be aligned
*      on the 64 byte boundary so as to allow optimal memory transfers. At the
*      minimum they must be aligned to the 8 byte boundary
*
* Ordering
*      -
*
* Extensibility
*
* Debugging
*
* Further information
*
* Sizes
*   Quad words = 8 bytes
*
* Terminology
*
* ==============================
*/

/*
******************************************************************************
* Include public/global header files
******************************************************************************
*/

#include "icp_arch_interfaces.h"
#include "icp_qat_hw.h"

/* Big assumptions that both bitpos and mask are constants */
#define QAT_FIELD_SET(flags,val,bitpos,mask)      \
   (flags) = (((flags) & (~((mask) << (bitpos)))) | (((val) & (mask)) << \
   (bitpos)))
#define QAT_FIELD_GET(flags,bitpos,mask)         \
   (((flags) >> (bitpos)) & (mask))


/**< @ingroup icp_qat_fw_comn
 * Default request and response ring size in bytes */
#define ICP_QAT_FW_RESP_DEFAULT_SZ               64

/**
 *****************************************************************************
 * @ingroup icp_qat_fw_comn
 *      Common flags type
 *
 * @description
 *      Definition of the common request and response flags. These describe
 *      the contents of the common request and processing attributes for the
 *      request.
 *
 *****************************************************************************/
typedef uint16_t icp_qat_fw_comn_flags;

/**
 *****************************************************************************
 * @ingroup icp_qat_fw_comn
 *      Definition of the common QAT FW request header.
 * @description
 *      Common section of the request used across all of the services exposes
 *      by the QAT FW. Each of the services inherit these common fields
 *
 *****************************************************************************/
typedef struct icp_qat_fw_comn_req_hdr_s
{
   icp_arch_if_req_hdr_t arch_if;
    /**< Common arch fields used by all ICP interface requests. Remaining
    * fields are specific to the common QAT FW service. */

   uint8_t   content_desc_hdr_sz;
   /**< Size of the content descriptor header in quad words. This information
    * is read into the QAT AE xfr registers */

   uint8_t content_desc_params_sz;
   /**< Size of the content descriptor parameters in quad words. These
    * parameters describe the session setup configuration info for the
    * slices that this request relies upon i.e. the configuration word and
    * cipher key needed by the cipher slice if there is a request for cipher
    * processing. The format of the parameters are contained in icp_qat_hw.h
    * and vary depending on the algorithm and mode being used. It is the
    * clients responsibility to ensure this structure is correctly packed */

   icp_qat_fw_comn_flags comn_req_flags;
   /**< Flags used to describe common processing required by the request and
    * the meaning of parameters in it i.e. differentiating between a buffer
    * descriptor and a flat buffer pointer in the source (src) and destination
    * (dest) data address fields. Full definition of the fields is given
    * below */

   uint64_t opaque_data;
   /**< Opaque data passed unmodified from the request to response messages by
    * firmware (fw) */

   uint64_t src_data_addr;
   /**< Generic definition of the source data supplied to the QAT AE.i The
    * common flags are used to further describe the attributes of this
    * field */

   uint64_t dest_data_addr;
   /**< Generic definition of the destination data supplied to the QAT AE.i The
    * common flags are used to further describe the attributes of this
    * field */

   uint64_t content_desc_addr;
   /**< Address of the content descriptor containing both the content header
    * the size of which is defined by content_desc_hdr_sz followed by the
    * content parameters whose size is described by content_desc_params_sz */

} icp_qat_fw_comn_req_hdr_t;

/*  The bit offsets below are within the flags field. These are NOT relative to
 *  the memory word. Unused field must be zeroed.
 *
 *  + ===== + ------ + ---------- + --- + --- + --- + --------- + -------- +
 *  |  Bit  |  15-13 |   12 - 7   |  6  |  5  |  4  |   3 - 2   |   1 - 0  |
 *  + ===== + ------ + ---------- + --- + --- + --- + --------- + -------- +
 *  | Flags | Status | Resvd Bits | Own | Dir | Ord | Dest Type | Src Type |
 *  + ===== + -------+ ---------- + --- + --- + --- + --------- + -------- +
 */

/* ========================================================================= */
/*                                        Status Flag definitions */
/* ========================================================================= */

#define ICP_QAT_FW_COMN_STATUS_FLAG_OK      0
/**< @ingroup icp_qat_fw_comn
 * Definition of successful processing of a request */

/* ========================================================================= */
/*                                  Buffer Ownership Flag definitions */
/* ========================================================================= */

#define ICP_QAT_FW_COMN_OWN_NONE   0
/**< @ingroup icp_qat_fw_comn
 * Definition that the leading and trailing quad word data in the supplied
 * buffer ownership cannot be assumed for processing the request */
#define ICP_QAT_FW_COMN_OWN_ALLOWED   1
/**< @ingroup icp_qat_fw_comn
 * Definition that the leading and trailing quad word data in the supplied
 * buffer is owned by the FW for the duration that it holds the request */

/* ========================================================================= */
/*                                      Direction Flag definitions */
/* ========================================================================= */

#define ICP_QAT_FW_COMN_DIR_FLAG_INPLACE   0
/**< @ingroup icp_qat_fw_comn
 * Definition of an inplace request */
#define ICP_QAT_FW_COMN_DIR_FLAG_OUT_PLACE   1
/**< @ingroup icp_qat_fw_comn
 * Definition of an out of place (OOP) request */

/* ========================================================================= */
/*                                       Ordering Flag definitions */
/* ========================================================================= */

#define ICP_QAT_FW_COMN_ORD_FLAG_NONE          0
/**< @ingroup icp_qat_fw_comn
 * Definition of a request that will not guarantee ordered processing in the
 * QAT FW*/
#define ICP_QAT_FW_COMN_ORD_FLAG_STRICT      1
/**< @ingroup icp_qat_fw_comn
 * Definition of a request that will guarantee ordered processing in the
 * QAT FW. NOTE: the ordering is based on ring ordering not session ordering.
 * Should a session used a single ring to submit demand to the QAT FW then
 * ordering of requests will be maintained assuming the bit is set. However
 * if requests for a session are submitted across a number of rings then the
 * QAT FW will not guarantee the ordering */

/* ========================================================================= */
/*                           Destination Data Address Flag definitions */
/* ========================================================================= */

#define ICP_QAT_FW_COMN_DEST_FLAG_FLAT      0
/**< @ingroup icp_qat_fw_comn
 * Definition of a flat buffer used for destination data. Also set to this
 * value if the destination is unused by the request */
#define ICP_QAT_FW_COMN_DEST_FLAG_ICP_BUF    1
/**< @ingroup icp_qat_fw_comn
 * Definition of ICP buffer descriptor used for destination data */
#define ICP_QAT_FW_COMN_DEST_FLAG_SGL_BUF    2
/**< @ingroup icp_qat_fw_comn
 * Definition of SGL buffer descriptor used for destination data */

/* ========================================================================= */
/*                              Source Data Address Flag definitions */
/* ========================================================================= */

#define ICP_QAT_FW_COMN_SRC_FLAG_FLAT          ICP_QAT_FW_COMN_DEST_FLAG_FLAT
/**< @ingroup icp_qat_fw_comn
 * Definition of a flat buffer used for source data. Also set to this
 * value if the source is unused by the request */
#define ICP_QAT_FW_COMN_SRC_FLAG_ICP_BUF    ICP_QAT_FW_COMN_DEST_FLAG_ICP_BUF
/**< @ingroup icp_qat_fw_comn
 * Definition of ICP buffer descriptor used for source data */
#define ICP_QAT_FW_COMN_SRC_FLAG_SGL_BUF    ICP_QAT_FW_COMN_DEST_FLAG_SGL_BUF
/**< @ingroup icp_qat_fw_comn
 * Definition of SGL buffer descriptor used for source data */

/* ========================================================================= */
/*                      Private Common Flags Accessor Macro definitions */
/* ========================================================================= */

#define QAT_COMN_STATUS_BITPOS              13
/**< @ingroup icp_qat_fw_comn
 * Starting bit position indicating status of a response */

#define QAT_COMN_STATUS_MASK                   0x7
/**< @ingroup icp_qat_fw_comn
 * Two bit mask used to determine the status.*/

#define QAT_COMN_OWN_BITPOS                 6
/**< @ingroup icp_qat_fw_comn
 * Starting bit position indicating buffer ownership */

#define QAT_COMN_OWN_MASK                      0x1
/**< @ingroup icp_qat_fw_comn
 * One bit mask used to determine the ownership. */

#define QAT_COMN_DIR_BITPOS                 5
/**< @ingroup icp_qat_fw_comn
 * Starting bit position indicating processing direction */

#define QAT_COMN_DIR_MASK                      0x1
/**< @ingroup icp_qat_fw_comn
 * One bit mask used to determine the direction. */

#define QAT_COMN_ORD_BITPOS                 4
/**< @ingroup icp_qat_fw_comn
 * Starting bit position indicating ordered requests/responses  */

#define QAT_COMN_ORD_MASK                      0x1
/**< @ingroup icp_qat_fw_comn
 * One bit mask used to determine the ordering. */

#define QAT_COMN_DEST_TYPE_BITPOS           2
/**< @ingroup icp_qat_fw_comn
 * Starting bit position indicating Destination Pointer type */

#define QAT_COMN_DEST_TYPE_MASK               0x3
/**< @ingroup icp_qat_fw_comn
 * Two bit mask used to determine Destination Pointer type */

#define QAT_COMN_SRC_TYPE_BITPOS            0
/**< @ingroup icp_qat_fw_comn
 * Starting bit position indicating Source Pointer type  */

#define QAT_COMN_SRC_TYPE_MASK               0x3
/**< @ingroup icp_qat_fw_comn
 * Two bit mask used to determine Source Pointer type */

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_comn
 *
 * @description
 *      Macro that must be used when build the flags for the common request
 *
 * @param status   Value of the status flag
 * @param dir      Request direction inplace or out of place flag
 * @param ord      Request ordering flag
 * @param dest_buf   Type of destination used
 * @param src_buf   Type of source buffer used
 *
 *****************************************************************************/
#define ICP_QAT_FW_COMN_FLAGS_BUILD(status,dir,ord,dest_buf,src_buf)         \
   ((((status) & QAT_COMN_STATUS_MASK) << QAT_COMN_STATUS_BITPOS)             |\
    (((dir) & QAT_COMN_DIR_MASK) << QAT_COMN_DIR_BITPOS)                       |\
    (((ord) & QAT_COMN_ORD_MASK) << QAT_COMN_ORD_BITPOS)                       |\
    (((dest_buf) & QAT_COMN_DEST_TYPE_MASK) << QAT_COMN_DEST_TYPE_BITPOS)   |\
    (((src_buf) & QAT_COMN_SRC_TYPE_MASK) << QAT_COMN_SRC_TYPE_BITPOS))

/* ========================================================================= */
/*                                                    GETTERS */
/* ========================================================================= */

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_comn
 *
 * @description
 *      Macro for extraction of the status returned in the response. The
 *      meaning of the status bits is defined in each of the services
 *
 * @param flags      Flags to extract the status bits from
 *
 *****************************************************************************/
#define ICP_QAT_FW_COMN_STATUS_GET(flags)   \
   QAT_FIELD_GET(flags,QAT_COMN_STATUS_BITPOS,QAT_COMN_STATUS_MASK)


/**
 ******************************************************************************
 * @ingroup icp_qat_fw_comn
 *
 * @description
 *      Macro for extraction of the ownership bit from the common flags
 *
 * @param flags      Flags to extract the ownership bit from
 *
 *****************************************************************************/
#define ICP_QAT_FW_COMN_OWN_GET(flags)   \
   QAT_FIELD_GET(flags,QAT_COMN_OWN_BITPOS,QAT_COMN_OWN_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_comn
 *
 * @description
 *      Macro for extraction of the direction bit from the common flags
 *
 * @param flags      Flags to extract the direction bit from
 *
 *****************************************************************************/
#define ICP_QAT_FW_COMN_DIR_GET(flags)   \
   QAT_FIELD_GET(flags,QAT_COMN_DIR_BITPOS,QAT_COMN_DIR_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_comn
 *
 * @description
 *      Macro for extraction of the ordering bit from the common flags
 *
 * @param flags      Flags to extract the ordering bit from
 *
 *****************************************************************************/
#define ICP_QAT_FW_COMN_ORD_GET(flags)   \
   QAT_FIELD_GET(flags,QAT_COMN_ORD_BITPOS,QAT_COMN_ORD_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_comn
 *
 * @description
 *      Macro for extraction of the destination buffer type bit from the
 *      common flags
 *
 * @param flags      Flags to extract the destination buffer type bit from
 *
 *****************************************************************************/
#define ICP_QAT_FW_COMN_DEST_TYPE_GET(flags)   \
   QAT_FIELD_GET(flags,QAT_COMN_DEST_TYPE_BITPOS,QAT_COMN_DEST_TYPE_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_comn
 *
 * @description
 *      Macro for extraction of the source buffer type bit from the common
 *      flags
 *
 * @param flags      Flags to extract the source buffer type bit from
 *
 *****************************************************************************/
#define ICP_QAT_FW_COMN_SRC_TYPE_GET(flags)   \
   QAT_FIELD_GET(flags,QAT_COMN_SRC_TYPE_BITPOS,QAT_COMN_SRC_TYPE_MASK)

/* ========================================================================= */
/*                                                    SETTERS */
/* ========================================================================= */

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_comn
 *
 * @description
 *      Macro for setting of the status field of the flags. The meaning of the
 *      status bits is defined in each of the services
 *
 * @param flags      Flags to set with the status
 * @param val      Status value
 *
 *****************************************************************************/
#define ICP_QAT_FW_COMN_STATUS_SET(flags,val)   \
   QAT_FIELD_SET(flags,val,QAT_COMN_STATUS_BITPOS,QAT_COMN_STATUS_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_comn
 *
 * @description
 *      Macro for setting the ownership bit of the common flags
 *
 * @param flags      Flags to update with the ownership bit
 * @param val      Ownership value
 *
 *****************************************************************************/
#define ICP_QAT_FW_COMN_OWN_SET(flags,val)   \
   QAT_FIELD_SET(flags,val,QAT_COMN_OWN_BITPOS,QAT_COMN_OWN_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_comn
 *
 * @description
 *      Macro for setting the direction bit of the common flags
 *
 * @param flags      Flags to update with the direction bit
 * @param val      Direction value
 *
 *****************************************************************************/
#define ICP_QAT_FW_COMN_DIR_SET(flags,val)   \
   QAT_FIELD_SET(flags,val,QAT_COMN_DIR_BITPOS,QAT_COMN_DIR_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_comn
 *
 * @description
 *      Macro for setting the ordering bit of the common flags
 *
 * @param flags      Flags to set with the ordering bit
 * @param val      Ordering value
 *
 *****************************************************************************/
#define ICP_QAT_FW_COMN_ORD_SET(flags,val)   \
   QAT_FIELD_SET(flags,val,QAT_COMN_ORD_BITPOS,QAT_COMN_ORD_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_comn
 *
 * @description
 *      Macro for setting the destination buffer type of the common flags
 *
 * @param flags      Flags to set with the destination buffer type
 * @param val      Destination buffer type
 *
 *****************************************************************************/
#define ICP_QAT_FW_COMN_DEST_TYPE_SET(flags,val)   \
   QAT_FIELD_SET(flags,val,QAT_COMN_DEST_TYPE_BITPOS,QAT_COMN_DEST_TYPE_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_comn
 *
 * @description
 *      Macro for setting the source buffer type of the common flags
 *
 * @param flags      Flags to set with the source buffer type
 * @param val      Source buffer type
 *
 *****************************************************************************/
#define ICP_QAT_FW_COMN_SRC_TYPE_SET(flags,val)   \
   QAT_FIELD_SET(flags,val,QAT_COMN_SRC_TYPE_BITPOS,QAT_COMN_SRC_TYPE_MASK)

/*
 * ============================================================================
 * Definition of types and data used to build a content descriptor to associate
 * with a request
 * ============================================================================
 */

/**
 *****************************************************************************
 * @ingroup icp_qat_fw_comn
 *      Slice types for building of the processing chain within the content
 *      descriptor
 *
 * @description
 *      Enumeration used to indicate the ids of the slice types through which
 *      data will pass.
 *
 *      A logical slice is not a hardware slice but is a software FSM
 *      performing the actions of a slice
 *
 *****************************************************************************/

typedef enum
{
   ICP_QAT_FW_SLICE_NULL=0,          /**< NULL slice type */
   ICP_QAT_FW_SLICE_CIPHER=1,          /**< CIPHER slice type */
   ICP_QAT_FW_SLICE_AUTH=2,          /**< AUTH slice type */
   ICP_QAT_FW_SLICE_MEM_IN=3,          /**< MEM_IN Logical slice type */
   ICP_QAT_FW_SLICE_MEM_OUT=4,      /**< MEM_OUT Logical slice type */
   ICP_QAT_FW_SLICE_IPSEC=5,          /**< IPsec Logical slice type */
   ICP_QAT_FW_SLICE_DELIMITER          /**< End delimiter */
} icp_qat_fw_slice_t;

/**
 *****************************************************************************
 * @ingroup icp_qat_fw_comn
 *      Cipher header of the content descriptor header block
 * @description
 *      Definition of the structure used to describe the cipher processing to
 *      perform on data. The cipher parameters are defined per algorithm
 *      and are located in the icp_qat_hw.h file.
 *
 *****************************************************************************/
typedef struct icp_qat_fw_cipher_hdr_s
{
   uint8_t next_id;
   /**< Set to the next slice to pass the ciphered data through.
    * Set to ICP_QAT_FW_SLICE_MEM_OUT if the data is not to go through
    * anymore slices after cipher */
   uint8_t curr_id;
   /**< Initialised with the cipher slice type */
   uint8_t offset;
   /**< Quad word offset from the content descriptor parameters address i.e.
    * (content_address + (cd_hdr_sz << 3)) to the parameters for the cipher
    * processing */
   uint8_t state_sz;
   /**< State size in quad words of the cipher algorithm used in this session.
    * Set to zero if the algorithm doesnt provide any state */
   uint8_t key_sz;
   /**< Key size in quad words of the cipher algorithm used in this session */
   uint8_t block_sz;
   /**< Block byte size of the cipher algorithm used in this session. Note that
    * the QAT HW internally always deals with data in quad word blocks
    * (8 bytes). Therefore for the stream ciphers such as RC4 the block size
    * is 8 bytes. See the QAT EAS for further information on the state, key
    * and blocks sizes that must be used. Block sizes to use are defined in
    * icp_qat_hw.h too */
   uint8_t reserved1;
   /**< Reserved padding byte to bring the struct to the word boundary. MUST be
    * set to 0 */
   uint8_t reserved2;
   /**< Reserved padding byte to bring the struct to the word boundary. MUST be
    * set to 0 */
} icp_qat_fw_cipher_hdr_t;


/**
 *****************************************************************************
 * @ingroup icp_qat_fw_comn
 *      Authentication header of the content descriptor header block
 * @description
 *      Definition of the structure used to describe the auth processing to
 *      perform on data. The auth parameters are defined per algorithm
 *      and are located in the icp_qat_hw.h file.
 *
 *      Unused fields must be set to 0.
 *
 *****************************************************************************/
typedef struct icp_qat_fw_auth_hdr_s
{
   uint8_t next_id;
   /**< Set to the next slice to pass data through.
    * Set to ICP_QAT_FW_SLICE_MEM_OUT if the data is not to go through
    * anymore slices after auth */
   uint8_t curr_id;
   /**< Initialised with the auth slice type */
   uint8_t offset;
   /**< Quad word offset from the content descriptor parameters address to the
    * parameters for the auth processing */
   uint8_t hash_flags;
   /**< General flags defining the processing to perform. 0 is normal processing
    * and 1 means there is a nested hash processing loop to go through */
   uint8_t inner_res_sz;
   /**< Size in bytes of the digest from the inner hash algorithm */
   uint8_t final_sz;
   /**< Size in bytes of digest to be returned to the client if requested */
   uint8_t outer_prefix_sz;
   /**< Size in bytes of outer prefix data */
   union {
      uint8_t inner_prefix_sz;
      /**< Size in bytes of the inner prefix data */
      uint8_t aad_sz;
      /**< Size in bytes of padded AAD data to prefix to the packet for CCM
        *  or GCM processing */
   } u;
   uint8_t inner_state2_sz;
   /**< Size in bytes of inner hash state2 data. Must be a qword multiple */
   uint8_t inner_state2_off;
   /**< Quad word offset from the content descriptor parameters pointer to the
    * inner state2 value */
   uint8_t inner_state1_sz;
   /**< Size in bytes of inner hash state1 data. Must be a qword multiple */
   uint8_t reserved;
   /**< This field is unused, assumed value is zero. */
   uint8_t outer_prefix_off;
   /**< Quad word offset from the start of the inner prefix data to the outer 
    * prefix information. Should equal the rounded inner prefix size, converted 
    * to qwords  */
   uint8_t outer_res_sz;
   /**< Size in bytes of digest from the outer auth algorithm */
   uint8_t outer_state1_sz;
   /**< Size in bytes of the outer state1 value */
   uint8_t outer_config_off;
   /**< Quad word offset from the content descriptor parameters pointer to the
    * outer configuration information */
} icp_qat_fw_auth_hdr_t;


#define ICP_QAT_FW_AUTH_HDR_FLAG_DO_NESTED   1
/**< @ingroup icp_qat_fw_comn
 * Definition of the hash_flags bit of the auth_hdr to inidacte the request
 * requires nested hashing */

#define ICP_QAT_FW_AUTH_HDR_FLAG_NO_NESTED   0
/**< @ingroup icp_qat_fw_comn
 * Definition of the hash_flags bit of the auth_hdr for no nested hashing
 * required */

#define ICP_QAT_FW_CCM_GCM_AAD_SZ_MAX        240
/**< @ingroup icp_qat_fw_comn
 * Maximum size of AAD data allowed for CCM or GCM processing. AAD data size
 * is stored in 8-bit field and must be multiple of hash block size. 240 is
 * largest value which satisfy both requirements.AAD_SZ_MAX is in byte units*/

/**
 *****************************************************************************
 * @ingroup icp_qat_fw_comn
 *      Definition of the common request structure with service specific
 *      fields
 * @description
 *      This is a definition of the full qat request structure used by all
 *      services. Each service is free to use the service fields in its own
 *      way. This struct is useful as a message passing argument before the
 *      service contained within the request is determined.
 *
 *****************************************************************************/
typedef struct icp_qat_fw_comn_req_s
{
   icp_qat_fw_comn_req_hdr_t comn_hdr;
   /**< Common header fields */

   uint32_t serv_fields[(ICP_QAT_FW_RESP_DEFAULT_SZ -                   \
                                   sizeof(icp_qat_fw_comn_req_hdr_t)) / \
                   sizeof(uint32_t)];
   /**< Service Specific fields of the standard request */

} icp_qat_fw_comn_req_t;

/**
 *****************************************************************************
 * @ingroup icp_qat_fw_comn
 *      Definition of the common QAT FW response header.
 * @description
 *      This section of the response is common across all of the services
 *      that generate a S interface response
 *
 *****************************************************************************/
typedef struct icp_qat_fw_comn_resp_hdr_s
{
   icp_arch_if_resp_hdr_t arch_if;
   /**< Common arch fields used by all ICP interface response messages. The
    * remaining fields are specific to the QAT FW */

   uint8_t   serv_cmd_id;
    /**< For services that define multiple commands this field represents the
    * command. If only 1 command is supported then this field will be 0 */

   uint8_t resrvd;
   /**< Reserved field whose value is undefined coming back from the QAT AE */

   icp_qat_fw_comn_flags comn_resp_flags;
   /**< Flags used to describe the response processing. The format is similar
    * as for the common request except that only the status bits are valid for
    * the response. All other bits are reserved for future use and are cleared
    * by the firmware */

   uint64_t opaque_data;
   /**< Opaque data passed from the request to the response message */

} icp_qat_fw_comn_resp_hdr_t;



#endif /* __ICP_QAT_FW_H__ */
