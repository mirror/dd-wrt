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
 * @file icp_qat_fw_la.h
 * @defgroup icp_qat_fw_la ICP QAT FW Lookaside Service Interface Definitions
 * @ingroup icp_qat_fw
 * @description
 *      This file documents structs used to provided the interface to the
 *        LookAside (LA) QAT FW service
 *
 *****************************************************************************/

#ifndef __ICP_QAT_FW_LA_H__
#define __ICP_QAT_FW_LA_H__

/*
******************************************************************************
* Include local header files
******************************************************************************
*/
#include "icp_qat_fw.h"


/**
 *****************************************************************************
 * @ingroup icp_qat_fw_la
 *        Definition of the LookAside (LA) command types
 * @description
 *        Enumeration which is used to indicate the ids of functions
 *              that are exposed by the LA QAT FW service
 *
 *****************************************************************************/

typedef enum
{
    /*!< Cipher Request */
    ICP_QAT_FW_LA_CMD_CIPHER=0,

    /*!< Auth Request */
    ICP_QAT_FW_LA_CMD_AUTH=1,

    /*!< Cipher-Hash Request */
    ICP_QAT_FW_LA_CMD_CIPHER_HASH=2,

    /*!< DRBG Enable Request */
    ICP_QAT_FW_LA_CMD_DRBG_ENABLE=3,

    /*!< DRBG Disable Request */
    ICP_QAT_FW_LA_CMD_DRBG_DISABLE=4,

    /*!< DRBG Get Random Request */
    ICP_QAT_FW_LA_CMD_DRBG_GET_RANDOM=5,

    /*!< NRBG Test Request */
    ICP_QAT_FW_LA_CMD_NRBG_TEST=6,

    /*!< NRBG Get Test Status Request */
    ICP_QAT_FW_LA_CMD_NRBG_GET_TEST_STATUS=7,

    /*!< NRBG Get Counters Request */
    ICP_QAT_FW_LA_CMD_NRBG_GET_COUNTERS=8,

    /*!< SSL3 Key Derivation Request */
    ICP_QAT_FW_LA_CMD_SSL3_KEY_DERIVE=9,

    /*!< TLS Key Derivation Request */
    ICP_QAT_FW_LA_CMD_TLS_KEY_DERIVE=10,

    /*!< MGF1 Request */
    ICP_QAT_FW_LA_CMD_MGF1=11,
    
    /*!< Auth Pre-Compute Request */
    ICP_QAT_FW_LA_CMD_AUTH_PRE_COMP=12, 

    /**< Delimiter type */
    ICP_QAT_FW_LA_CMD_DELIMITER=13

} icp_qat_fw_la_cmd_id_t;

/**
 *****************************************************************************
 * @ingroup icp_qat_fw_la
 *        Definition of the DRBG states
 * @description
 *        Enumeration of the GATHER states of the DRBG
 *
 *****************************************************************************/
typedef enum
{
    ICP_QAT_FW_LA_DRBG_GATHER_STATE_IGNORE = 0,
    ICP_QAT_FW_LA_DRBG_GATHER_STATE_RESTART = 1,
    ICP_QAT_FW_LA_DRBG_GATHER_STATE_DELIMITER = 2   /**< Delimiter type */
} icp_qat_fw_la_drbg_gather_state_t;

/*  Definitions of the bits in the stats flags field of the common response.
 *  The values returned by the Lookaside service are given below
 *
 *  RESPONSE COMMON FLAGS
 *  + ===== + -------- + ------------------------------------------------ +
 *  |  Bit  |  15 - 13 |                    12 - 0                        |
 *  + ===== + -------- + ------------------------------------------------ +
 *  | Flags |  Status  |                 RESERVED = 0                     |
 *  + ===== + -------- + ------------------------------------------------ +
 */


#define QAT_LA_ICV_VER_STATUS_BITPOS                            15
/**< @ingroup icp_qat_fw_la
 * Starting bit position of the ICV Verification */

#define QAT_LA_ICV_VER_STATUS_MASK                              1
/**< @ingroup icp_qat_fw_la
 * Mask of one bit used to determine the ICV Verification Status */

#define QAT_LA_NRBG_STATE_STATUS_BITPOS                         15
/**< @ingroup icp_qat_fw_la
 * Starting bit position of the NRBG state status */

#define QAT_LA_NRBG_STATE_STATUS_MASK                           1
/**< @ingroup icp_qat_fw_la
 * Mask of one bit used to determine the NRBG state status */

#define QAT_LA_DRBG_STATE_STATUS_BITPOS                         14
/**< @ingroup icp_qat_fw_la
 * Starting bit position of the DRBG state status */

#define QAT_LA_DRBG_STATE_STATUS_MASK                           1
/**< @ingroup icp_qat_fw_la
 * Mask of one bit used to determine the DRBG state status */

#define QAT_LA_DRBG_GATHER_STATE_BITPOS                         13
/**< @ingroup icp_qat_fw_la
 * Starting bit position of the DRBG gather state */

#define QAT_LA_DRBG_GATHER_STATE_MASK                           1
/**< @ingroup icp_qat_fw_la
 * Mask of one bit used to determine the DRBG gather state */

#define ICP_QAT_FW_LA_ICV_VER_STATUS_PASS                       0
/**< @ingroup icp_qat_fw_la
 * Status flag indicating that the ICV verification passed */

#define ICP_QAT_FW_LA_ICV_VER_STATUS_FAIL                       1
/**< @ingroup icp_qat_fw_la
 * Status flag indicating that the ICV verification failed */

#define ICP_QAT_FW_LA_NRBG_STATUS_PASS                          0
/**< @ingroup icp_qat_fw_la
 * Status flag indicating that the NRBG returned valid entropy data */

#define ICP_QAT_FW_LA_NRBG_STATUS_FAIL                          1
/**< @ingroup icp_qat_fw_la
 * Status flag indicating that the NRBG Failed to get entropy */

#define ICP_QAT_FW_LA_DRBG_ENABLED                              0
/**< @ingroup icp_qat_fw_la
 * Status flag indicating that the DRBG was enabled when RAND GET was called */

#define ICP_QAT_FW_LA_DRBG_DISABLED                             1
/**< @ingroup icp_qat_fw_la
 * Status flag indicating that the DRBG was disabled when RAND GET was called*/

#define ICP_QAT_FW_LA_DRBG_GATHER_STATE_NOT_COMPLETED           0
/**< @ingroup icp_qat_fw_la
 * Status flag indicating that the DRBG is in GET state collecting
 * entropy sample */

#define ICP_QAT_FW_LA_DRBG_GATHER_STATE_COMPLETED               1
/**< @ingroup icp_qat_fw_la
 * Status flag indicating that the DRBG is in COMPLETED state and entropy
 * sample was collected */

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_la
 *
 * @description
 *        Macro for extraction of the ICV verify status returned in the response.
 *
 * @param flags        Flags to extract the status bit from
 *
 *****************************************************************************/
#define ICP_QAT_FW_LA_ICV_VER_STATUS_FLAGS_GET(flags)        \
    QAT_FIELD_GET(flags,                    \
                  QAT_LA_ICV_VER_STATUS_BITPOS,             \
                  QAT_LA_ICV_VER_STATUS_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_la
 *
 * @description
 *        Macro for extraction of the NRBG status returned in the response.
 *
 * @param flags        Flags to extract the status bit from
 *
 *****************************************************************************/
#define ICP_QAT_FW_LA_NRBG_STATUS_GET(flags)            \
    QAT_FIELD_GET(flags,                    \
                  QAT_LA_NRBG_STATE_STATUS_BITPOS,           \
                  QAT_LA_NRBG_STATE_STATUS_MASK)



/**
 ******************************************************************************
 * @ingroup icp_qat_fw_la
 *
 * @description
 *        Macro for extraction of the DRBG state in the response.
 *
 * @param flags        Flags to extract the state from
 *
 *****************************************************************************/
#define ICP_QAT_FW_LA_DRBG_STATE_GET(flags)            \
    QAT_FIELD_GET(flags,                    \
                  QAT_LA_DRBG_STATE_STATUS_BITPOS,          \
                  QAT_LA_DRBG_STATE_STATUS_MASK)


/**
 ******************************************************************************
 * @ingroup icp_qat_fw_la
 *
 * @description
 *        Macro for extraction of the DRBG Gather Staus in the response.
 *
 *
 * @param flags        Flags to extract the state from
 *
 *****************************************************************************/
#define ICP_QAT_FW_LA_DRBG_GATHER_STATE_GET(flags)        \
    QAT_FIELD_GET(flags,                    \
                  QAT_LA_DRBG_GATHER_STATE_BITPOS,          \
                  QAT_LA_DRBG_GATHER_STATE_MASK)


/**
 ******************************************************************************
 * @ingroup icp_qat_fw_la
 *
 * @description
 *        Macro for setting the ICV verify status of the flags.
 *
 * @param flags        Flags to set with the status
 * @param val        Verify status
 *
 *****************************************************************************/
#define ICP_QAT_FW_LA_ICV_VER_STATUS_SET(flags,val)        \
    QAT_FIELD_SET(flags,                    \
                  val,                          \
                  QAT_LA_ICV_VER_STATUS_BITPOS,         \
                  QAT_LA_ICV_VER_STATUS_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_la
 *
 * @description
 *        Macro for setting the NRBG status in the response flag
 *
 * @param flags        Flags to set with the status
 * @param val        NRBG status
 *
 *****************************************************************************/
#define ICP_QAT_FW_LA_NRBG_STATUS_SET(flags,val)        \
    QAT_FIELD_SET(flags,                    \
                  val,                          \
                  QAT_LA_NRBG_STATE_STATUS_BITPOS,           \
                  QAT_LA_NRBG_STATE_STATUS_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_la
 *
 * @description
 *        Macro for setting the DRBG state in the response flag. Only set for
 *      a random get message
 *
 * @param flags        Status flags to set with the state
 * @param val        DRBG state
 *
 *****************************************************************************/
#define ICP_QAT_FW_LA_DRBG_STATE_SET(flags,val)            \
    QAT_FIELD_SET(flags,                    \
                  val,                      \
                  QAT_LA_DRBG_STATE_STATUS_BITPOS,          \
                  QAT_LA_DRBG_STATE_STATUS_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_la
 *
 * @description
 *        Macro for setting the DRBG Gather Status in the response flag.
 *      Only set for a random get message
 *
 * @param flags        Status flags to set with the state
 * @param val        DRBG state
 *
 *****************************************************************************/
#define ICP_QAT_FW_LA_DRBG_GATHER_STATE_SET(flags,val)        \
    QAT_FIELD_SET(flags,                    \
                  val,                          \
                  QAT_LA_DRBG_GATHER_STATE_BITPOS,          \
                  QAT_LA_DRBG_GATHER_STATE_MASK)

/**
 *****************************************************************************
 * @ingroup icp_qat_fw_la
 *        Definition of the common LA QAT FW request descriptor parameters
 * @description
 *        This part of the request is common across all of the LA commands
 *
 *****************************************************************************/
typedef struct icp_qat_fw_la_comn_req_s
{
    uint8_t la_cmd_id;
    /**< Definition of the LA command defined by this request */

    union {
        uint8_t resrvd;
        /**< If not used by a request this field must be set to 0 */

        uint8_t req_params_blk_sz;
        /**<  For bulk processing this field represents the request parameters
         * block size */
    } u1;

    union {
        uint16_t la_flags;
        /**< Definition of the common LA processing flags used for the bulk
         * processing */

        uint16_t resrvd2;
        /**< If not used by a request this field must be set to 0 i.e.
          *  key gen */
    } u2;

} icp_qat_fw_la_comn_req_t;

/*
 *  REQUEST FLAGS IN COMMON LA FLAGS
 *
 *  + ===== + ---------- + ----- + ----- + ---- + ---- + ----- + ------- +
 *  |  Bit  |   15 - 11  |   10  |  9-7  |  6   |  5   |   4   |   1-0   |
 *  + ===== + ---------- + ----- + ----- + ---- + ---- + ----- + ------- +
 *  | Flags | Resvd Bits |  Apd  | Proto | Cmp  | Ret  |  Upd  | Partial |
 *  |       |     =0     |  Auth |  Type | Auth | Auth | State |         |
 *  + ===== + ---------- + ----- + ----- + ---- + ---- + ----- + ------- +
 */

#define ICP_QAT_FW_LA_GCM_PROTO                                 2
/**< @ingroup icp_qat_fw_la
 * Indicates GCM processing for a auth_encrypt command */
#define ICP_QAT_FW_LA_CCM_PROTO                                 1
/**< @ingroup icp_qat_fw_la
 * Indicates CCM processing for a auth_encrypt command */
#define ICP_QAT_FW_LA_NO_PROTO                                  0
/**< @ingroup icp_qat_fw_la
 * Indicates no specific protocol processing for the command */

#define ICP_QAT_FW_LA_CMP_AUTH_RES                              1
/**< @ingroup icp_qat_fw_la
 * Flag representing the need to compare the auth result data to the expected
 * value in DRAM at the auth_address. */
#define ICP_QAT_FW_LA_NO_CMP_AUTH_RES                           0
/**< @ingroup icp_qat_fw_la
 * Flag representing that there is no need to do a compare of the auth data
 * to the expected value */

#define ICP_QAT_FW_LA_RET_AUTH_RES                              1
/**< @ingroup icp_qat_fw_la
 * Flag representing the need to return the auth result data to dram after the
 * request processing is complete */
#define ICP_QAT_FW_LA_NO_RET_AUTH_RES                           0
/**< @ingroup icp_qat_fw_la
 * Flag representing that there is no need to return the auth result data */

#define ICP_QAT_FW_LA_APD_AUTH_RES                              1
/**< @ingroup icp_qat_fw_la
 * Flag representing the need to append the auth result (digest) after the
 * authenticated region for further processing (e.g. encryption). Once
 * overall processing is complete, authentication result will get written
 * to dram straight after the authenticated region. Auth Result Pointer
 * will be ignored in this case. */
#define ICP_QAT_FW_LA_NO_APD_AUTH_RES                           0
/**< @ingroup icp_qat_fw_la
 * Flag representing that there is no need to append auth result. Auth result
 * will get stored under the Auth Result Pointer. */

#define ICP_QAT_FW_LA_UPDATE_STATE                              1
/**< @ingroup icp_qat_fw_la
 * Flag representing the need to update the state data in dram after the
 * request processing is complete */
#define ICP_QAT_FW_LA_NO_UPDATE_STATE                           0
/**< @ingroup icp_qat_fw_la
 * Flag representing that there is no need to update the state data */

#define ICP_QAT_FW_LA_PARTIAL_NONE                              0
/**< @ingroup icp_qat_fw_la
 * Flag representing no need for partial processing condition */
#define ICP_QAT_FW_LA_PARTIAL_START                             1
/**< @ingroup icp_qat_fw_la
 * Flag representing the first chunk of the partial packet */
#define ICP_QAT_FW_LA_PARTIAL_MID                               3
/**< @ingroup icp_qat_fw_la
 * Flag representing a middle chunk of the partial packet */
#define ICP_QAT_FW_LA_PARTIAL_END                               2
/**< @ingroup icp_qat_fw_la
 * Flag representing the final/end chunk of the partial packet */

/* The table below defines the meaning of the prefix_addr & hash_state_sz in
 * the case of partial processing. See the HLD for further details
 *
 *  + ====== + ------------------------- + ----------------------- +
 *  | Parial |       Prefix Addr         |       Hash State Sz     |
 *  | State  |                           |                         |
 *  + ====== + ------------------------- + ----------------------- +
 *  |  FULL  | Points to the prefix data | Prefix size as below.   |
 *  |        |                           | No update of state      |
 *  + ====== + ------------------------- + ----------------------- +
 *  |  SOP   | Points to the prefix      | = inner prefix rounded  |
 *  |        | data. State is updated    | to qwrds + outer prefix |
 *  |        | at prefix_addr - state_sz | rounded to qwrds. The   |
 *  |        | - 8 (counter size)        | writeback state sz      |
 *  |        |                           | comes from the CD       |
 *  + ====== + ------------------------- + ----------------------- +
 *  |  MOP   | Points to the state data  | State size rounded to   |
 *  |        | Updated state written to  | num qwrds + 8 (for the  |
 *  |        | same location             | counter) + inner prefix |
 *  |        |                           | rounded to qwrds +      |
 *  |        |                           | outer prefix rounded to |
 *  |        |                           | qwrds.                  |
 *  + ====== + ------------------------- + ----------------------- +
 *  |  EOP   | Points to the state data  | State size rounded to   |
 *  |        |                           | num qwrds + 8 (for the  |
 *  |        |                           | counter) + inner prefix |
 *  |        |                           | rounded to qwrds +      |
 *  |        |                           | outer prefix rounded to |
 *  |        |                           | qwrds.                  |
 *  + ====== + ------------------------- + ----------------------- +
 *
 *  Notes:
 *
 *  - If the EOP is set it is assumed that no state update is to be performed.
 *    However it is the clients responsibility to set the update_state flag
 *    correctly i.e. not set for EOP or Full packet cases. Only set for SOP and
 *    MOP with no EOP flag
 *  - The SOP take precedence over the MOP and EOP i.e. in the calculation of
 *    the address to writeback the state.
 *  - The prefix address must be on at least the 8 byte boundary
 */

/* Private defines */
#define QAT_LA_APD_AUTH_RES_BITPOS                              10
/**< @ingroup icp_qat_fw_la
 * Starting bit position for Auth append digest result */

#define QAT_LA_APD_AUTH_RES_MASK                               0x1
/**< @ingroup icp_qat_fw_la
 * One bit mask used to determine the Auth append digest result */

#define QAT_LA_PROTO_BITPOS                                      7
/**< @ingroup icp_qat_fw_la
 * Starting bit position for the Lookaside Protocols */

#define QAT_LA_PROTO_MASK                                      0x7
/**< @ingroup icp_qat_fw_la
 * Three bit mask used to determine the Lookaside Protocol  */

#define QAT_LA_CMP_AUTH_RES_BITPOS                               6
/**< @ingroup icp_qat_fw_la
 * Starting bit position for Auth compare digest result */

#define QAT_LA_CMP_AUTH_RES_MASK                               0x1
/**< @ingroup icp_qat_fw_la
 * One bit mask used to determine the Auth compare digest result */

#define QAT_LA_RET_AUTH_RES_BITPOS                               5
/**< @ingroup icp_qat_fw_la
 * Starting bit position for Auth return digest result */

#define QAT_LA_RET_AUTH_RES_MASK                               0x1
/**< @ingroup icp_qat_fw_la
 * One bit mask used to determine the Auth return digest result */

#define QAT_LA_UPDATE_STATE_BITPOS                               4
/**< @ingroup icp_qat_fw_la
 * Starting bit position for Update State. */

#define QAT_LA_UPDATE_STATE_MASK                               0x1
/**< @ingroup icp_qat_fw_la
 * One bit mask used to determine the Update State */

#define QAT_LA_PARTIAL_BITPOS                                    0
/**< @ingroup icp_qat_fw_la
 * Starting bit position indicating partial state */

#define QAT_LA_PARTIAL_MASK                                    0x3
/**< @ingroup icp_qat_fw_la
 * Two bit mask used to determine the partial state */

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_la
 *
 * @description
 * Macro used for the generation of the Lookaside flags for a request. This
 * should always be used for the generation of the flags field. No direct sets
 * or masks should be performed on the flags data
 *
 * @param proto            Protocol handled by a command
 * @param cmp_auth        Compare auth result with the expected value
 * @param ret_auth        Return auth result to the client via DRAM
 * @param apd_auth      Append auth result (digest) straight after the
 *                      authenticated region in DRAM
 * @param wr_state        Indicate Writeback of the crypto state information is
 *                        required
 * @param partial        Inidicate if the packet is a partial part
 *
 *****************************************************************************/
#define ICP_QAT_FW_LA_FLAGS_BUILD(proto, cmp_auth, ret_auth, apd_auth,      \
                                  wr_state, partial)                        \
        ( ((proto & QAT_LA_PROTO_MASK) <<                                   \
            QAT_LA_PROTO_BITPOS)                |                           \
          ((cmp_auth & QAT_LA_CMP_AUTH_RES_MASK) <<                         \
            QAT_LA_CMP_AUTH_RES_BITPOS)         |                           \
          ((ret_auth & QAT_LA_RET_AUTH_RES_MASK) <<                         \
           QAT_LA_RET_AUTH_RES_BITPOS)          |                           \
          ((apd_auth & QAT_LA_APD_AUTH_RES_MASK) <<                         \
           QAT_LA_APD_AUTH_RES_BITPOS)          |                           \
          ((wr_state & QAT_LA_UPDATE_STATE_MASK) <<                         \
           QAT_LA_UPDATE_STATE_BITPOS)          |                           \
          ((partial & QAT_LA_PARTIAL_MASK) << QAT_LA_PARTIAL_BITPOS) )

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_la
 *
 * @description
 *        Macro for extraction of the LA protocol state
 *
 * @param flags        Flags to extract the protocol state from
 *
 *****************************************************************************/
#define ICP_QAT_FW_LA_PROTO_GET(flags)                        \
    QAT_FIELD_GET(flags,QAT_LA_PROTO_BITPOS,QAT_LA_PROTO_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_la
 *
 * @description
 *        Macro for extraction of the "compare auth" state
 *
 * @param flags        Flags to extract the compare auth result state from
 *
 *****************************************************************************/
#define ICP_QAT_FW_LA_CMP_AUTH_GET(flags)                   \
    QAT_FIELD_GET(flags,QAT_LA_CMP_AUTH_RES_BITPOS,QAT_LA_CMP_AUTH_RES_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_la
 *
 * @description
 *        Macro for extraction of the "return auth" state
 *
 * @param flags        Flags to extract the return auth result state from
 *
 *****************************************************************************/
#define ICP_QAT_FW_LA_RET_AUTH_GET(flags)                   \
    QAT_FIELD_GET(flags,QAT_LA_RET_AUTH_RES_BITPOS,QAT_LA_RET_AUTH_RES_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_la
 *
 * @description
 *      Macro for extraction of the "append auth" state
 *
 * @param flags     Flags to extract the return auth result state from
 *
 *****************************************************************************/
#define ICP_QAT_FW_LA_APD_AUTH_GET(flags)                   \
    QAT_FIELD_GET(flags,QAT_LA_APD_AUTH_RES_BITPOS,QAT_LA_APD_AUTH_RES_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_la
 *
 * @description
 *        Macro for extraction of the "update state" value.
 *
 * @param flags        Flags to extract the update state bit from
 *
 *****************************************************************************/
#define ICP_QAT_FW_LA_WR_STATE_GET(flags)                   \
    QAT_FIELD_GET(flags,QAT_LA_UPDATE_STATE_BITPOS,QAT_LA_UPDATE_STATE_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_la
 *
 * @description
 *        Macro for extraction of the "partial" packet state
 *
 * @param flags        Flags to extract the partial state from
 *
 *****************************************************************************/
#define ICP_QAT_FW_LA_PARTIAL_GET(flags)                    \
    QAT_FIELD_GET(flags,QAT_LA_PARTIAL_BITPOS,QAT_LA_PARTIAL_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_la
 *
 * @description
 *        Macro for setting the LA protocol state of the flags
 *
 * @param flags        Flags to set with the protocol state
 * @param val        Protocol value
 *
 *****************************************************************************/
#define ICP_QAT_FW_LA_PROTO_SET(flags,val)                  \
    QAT_FIELD_SET(flags,val,QAT_LA_PROTO_BITPOS,QAT_LA_PROTO_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_la
 *
 * @description
 *        Macro for setting the "compare auth" state in the flags
 *
 * @param flags        Flags to set with the compare auth result state
 * @param val        Compare Auth value
 *
 *****************************************************************************/
#define ICP_QAT_FW_LA_CMP_AUTH_SET(flags,val)               \
    QAT_FIELD_SET(flags,val,QAT_LA_CMP_AUTH_RES_BITPOS,QAT_LA_CMP_AUTH_RES_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_la
 *
 * @description
 *        Macro for setting the "return auth" state of the flags
 *
 * @param flags        Flags to set with the return auth result state
 * @param val        Return Auth value
 *
 *****************************************************************************/
#define ICP_QAT_FW_LA_RET_AUTH_SET(flags,val)               \
    QAT_FIELD_SET(flags,val,QAT_LA_RET_AUTH_RES_BITPOS,QAT_LA_RET_AUTH_RES_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_la
 *
 * @description
 *      Macro for setting the "append auth" state of the flags
 *
 * @param flags     Flags to set with the return auth result state
 * @param val       Return Auth value
 *
 *****************************************************************************/
#define ICP_QAT_FW_LA_APD_AUTH_SET(flags,val)               \
    QAT_FIELD_SET(flags,val,QAT_LA_APD_AUTH_RES_BITPOS,QAT_LA_APD_AUTH_RES_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_la
 *
 * @description
 *        Macro for setting the "update state" value of the flags
 *
 * @param flags        Flags to set with the update state bit
 * @param val        Writeback State value
 *
 *****************************************************************************/
#define ICP_QAT_FW_LA_WR_STATE_SET(flags,val)               \
    QAT_FIELD_SET(flags,val,QAT_LA_UPDATE_STATE_BITPOS,QAT_LA_UPDATE_STATE_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_fw_la
 *
 * @description
 *        Macro for setting the "partial" packet state of the flags
 *
 * @param flags        Flags to set with the partial state
 * @param val        Partial state value
 *
 *****************************************************************************/
#define ICP_QAT_FW_LA_PARTIAL_SET(flags,val)                \
    QAT_FIELD_SET(flags,val,QAT_LA_PARTIAL_BITPOS,QAT_LA_PARTIAL_MASK)


/* Private defines */
#define QAT_FW_LA_BULK_REQ_RSVD_LW_SZ        3
/**< @ingroup icp_qat_fw_la
 * Define for Bulk request reserved size in longwords */


/**
 *****************************************************************************
 * @ingroup icp_qat_fw_la
 *        Definition of the common LA QAT FW bulk request
 * @description
 *        Definition of the bulk processing request type. Used for hash only,
 *        cipher only, hash-cipher and authentication-encryption requests
 *
 *****************************************************************************/
typedef struct icp_qat_fw_la_bulk_req_s
{
    icp_qat_fw_comn_req_hdr_t comn_req;
    /**< Common interface request */

    uint64_t req_params_addr;
    /**< Memory address of the request parameters */

    icp_qat_fw_la_comn_req_t comn_la_req;
    /**< Common LA request parameters */

    uint32_t resrvd[QAT_FW_LA_BULK_REQ_RSVD_LW_SZ];
    /**< Reserved data that is assumed set to 0 */

} icp_qat_fw_la_bulk_req_t;


/**
 *****************************************************************************
 * @ingroup icp_qat_fw_la
 *        Definition of the cipher request parameters block
 * @description
 *        Definition of the cipher processing request parameters block
 *
 *****************************************************************************/
typedef struct icp_qat_fw_la_cipher_req_params_s
{
    uint8_t next_id;
    /**< Set to the next slice to pass the ciphered data through.
     * Set to ICP_QAT_FW_SLICE_MEM_OUT if the data is not to go through
     * anymore slices after cipher */

    uint8_t curr_id;
    /**< Initialised with the cipher slice type */

    uint8_t cipher_state_sz;
    /**< Number of quad words of state data for the cipher algorithm */

    uint8_t resrvd;
    /**< Reserved field and assumed set to 0 */

    uint16_t cipher_off;
    /**< Byte offset from the start of packet to the cipher data region */

    uint16_t cipher_len;
    /**< Byte length of the cipher data region */

    uint64_t state_address;
    /**< Flat buffer address in memory of the cipher state information. Unused
     * if the state size is 0 */

} icp_qat_fw_la_cipher_req_params_t;

/**
 *****************************************************************************
 * @ingroup icp_qat_fw_la
 *        Definition of the auth request parameters block
 * @description
 *        Definition of the auth processing request parameters block
 *
 *****************************************************************************/
typedef struct icp_qat_fw_la_auth_req_params_s
{
    uint8_t next_id;
    /**< Set to the next slice to pass the auth data through.
     * Set to ICP_QAT_FW_SLICE_NULL for in-place auth-only requests
     * Set to ICP_QAT_FW_SLICE_MEM_OUT for all other request types
     * if the data is not to go through anymore slices after auth */

    uint8_t curr_id;
    /**< Initialised with the auth slice type */

    uint8_t hash_state_sz;
    /**< Number of quad words of inner and outer hash prefix data to process */

    uint8_t auth_res_sz;
    /**< Size in quad words of digest information to validate */

    uint16_t auth_off;
    /**< Byte offset from the start of packet to the auth data region */

    uint16_t auth_len;
    /**< Byte length of the auth data region */

    union {
        uint64_t prefix_addr;
        /**< Address of the prefix information */

        uint64_t aad_addr;
        /**< Address of the AAD info in DRAM. Used for the CCM and GCM
         * protocols */
    } u;

    uint64_t auth_res_address;
    /**< Address of the auth result information to validate or the location to
     * writeback the digest information to */

} icp_qat_fw_la_auth_req_params_t;


/* Private defines */
#define QAT_FW_LA_RNG_REQ_RSVD_LW_SZ         2
/**< @ingroup icp_qat_fw_la
 * Define for Random Number Generatoin request reserved size in longwords */

/**
 *****************************************************************************
 * @ingroup icp_qat_fw_la
 *        Definition of the common LA QAT FW RNG request
 * @description
 *        Definition of the RNG processing request type
 *
 *****************************************************************************/
typedef struct icp_qat_fw_la_rng_req_s
{
    icp_qat_fw_comn_req_hdr_t comn_req;
        /**< Common interface request */

    uint32_t resrvd1[QAT_FW_LA_RNG_REQ_RSVD_LW_SZ];
        /**< Reserved data assumed set to 0 */

    icp_qat_fw_la_comn_req_t comn_la_req;
        /**< Common LA request parameters */

    uint16_t length;
    /**< Size of the data in bytes to process. Used by the get_random command.
     * Set to 0 for commands that dont need a length parameter */

        uint8_t  gather_state;
    /**< State of the entropy_sample buffer to process. Used by the get_random
         *  command.
     *  Set to 0 for commands that dont need a gather_state parameter */

    uint8_t resrvd2;
        /**< Reserved data assumed set to 0 */

    uint32_t resrvd3[QAT_FW_LA_RNG_REQ_RSVD_LW_SZ];
        /**< Reserved data assumed set to 0 */

} icp_qat_fw_la_rng_req_t;


/*
 ******************************************************************************
 * MGF Max supported input parameters
 ******************************************************************************
 */
#define ICP_QAT_FW_LA_MGF_SEED_LEN_MAX            255
/**< @ingroup icp_qat_fw_la
 * Maximum seed length for MGF1 request in bytes
 * Typical values may be 48, 64, 128 bytes (or any).*/
#define ICP_QAT_FW_LA_MGF_MASK_LEN_MAX            65528
/**< @ingroup icp_qat_fw_la
 * Maximum mask length for MGF1 request in bytes
 * Typical values may be 8 (64-bit), 16 (128-bit). MUST be qwad word multiple */

/*
 ******************************************************************************
 * SSL Max supported input parameters
 ******************************************************************************
 */
#define ICP_QAT_FW_LA_SSL_SECRET_LEN_MAX        64
/**< @ingroup icp_qat_fw_la
 * Maximum secret length for SSL3 Key Gen request (bytes) */
#define ICP_QAT_FW_LA_SSL_ITERATES_LEN_MAX        16
/**< @ingroup icp_qat_fw_la
 * Maximum iterations for SSL3 Key Gen request (integer) */
#define ICP_QAT_FW_LA_SSL_LABEL_LEN_MAX            136
/**< @ingroup icp_qat_fw_la
 * Maximum label length for SSL3 Key Gen request (bytes) */
#define ICP_QAT_FW_LA_SSL_SEED_LEN_MAX            64
/**< @ingroup icp_qat_fw_la
 * Maximum seed length for SSL3 Key Gen request (bytes) */
#define ICP_QAT_FW_LA_SSL_OUTPUT_LEN_MAX        248
/**< @ingroup icp_qat_fw_la
 * Maximum output length for SSL3 Key Gen request (bytes) */


/*
 ******************************************************************************
 * TLS Max supported input parameters
 ******************************************************************************
 */
#define ICP_QAT_FW_LA_TLS_SECRET_LEN_MAX        128
/**< @ingroup icp_qat_fw_la
 * Maximum secret length for TLS Key Gen request (bytes) */
#define ICP_QAT_FW_LA_TLS_LABEL_LEN_MAX            16
/**< @ingroup icp_qat_fw_la
 * Maximum label length for TLS Key Gen request (bytes) */
#define ICP_QAT_FW_LA_TLS_SEED_LEN_MAX            64
/**< @ingroup icp_qat_fw_la
 * Maximum seed length for TLS Key Gen request (bytes) */
#define ICP_QAT_FW_LA_TLS_OUTPUT_LEN_MAX        248
/**< @ingroup icp_qat_fw_la
 * Maximum output length for TLS Key Gen request (bytes) */


/* Private defines */
#define QAT_FW_LA_KEY_GEN_REQ_RSVD2_SZ       3
/**< @ingroup icp_qat_fw_la
 * Reserved2 byte field size for Lookaside key genreation requests */

/**
 *****************************************************************************
 * @ingroup icp_qat_fw_la
 *        Definition of the common LA QAT FW Key Generate Functions
 * @description
 *        Definition of the Key Generate Function processing request types.
 *        This struct defines the layout for the SSL3 key generation, TLS Key
 *        generation and MGF1
 *
 *****************************************************************************/
typedef struct icp_qat_fw_la_key_gen_req_s
{
    icp_qat_fw_comn_req_hdr_t comn_req;
    /**< Common interface request */

    uint64_t req_params_addr;
    /**< For the TLS processing this is the pointer to the request parameters
     * For all other key gen requests the address is set to NULL */

    icp_qat_fw_la_comn_req_t comn_la_req;
    /**< Common LA request parameters. Note the la command flags have no
     * meaning for this request type */

    union {
        uint8_t label_len;
        /**< Number of bytes of label for SSL and bytes for TLS key
         * generation  */
        uint8_t seed_len;
        /**< Number of bytes of seed provided in src buffer for MGF1 */
    } u1;

    union {
        uint8_t out_len;
        /**< Number of bytes of key material to output. */
        uint8_t hash_len;
        /**< Number of bytes of hash output by the QAT per iteration */
    } u2;

    union {
        uint16_t secret_len;
        /**< Length of Secret information for SSL. In the case of TLS the
         * secret is supplied in the content descriptor */
        uint16_t mask_len;
        /**< Size in bytes of the desired output mask for MGF1*/
    } u3;

    uint8_t resrvd2[QAT_FW_LA_KEY_GEN_REQ_RSVD2_SZ];
    /**< Reserved data assumed set to 0 */

    union {
        uint8_t iter_count;
        /**< Iteration count used by the SSL key gen request */
        uint8_t resrvd4;
        /**< If not used by a request this field assumed set to 0 */
    } u4;

    uint32_t resrvd3;
    /**< Reserved data assumed set to 0 */

} icp_qat_fw_la_key_gen_req_t;

/**
 *****************************************************************************
 * @ingroup icp_qat_fw_la
 *        Definition of the Lookaside Eagle Tail Response
 * @description
 *        This is the response delivered to the ET rings by the Lookaside
 *              QAT FW service for all commands
 *
 *****************************************************************************/
typedef struct icp_qat_fw_la_resp_s
{
    icp_qat_fw_comn_resp_hdr_t comn_resp;
    /**< Common interface response format see icp_qat_fw.h */

    uint32_t resrvd1[(ICP_QAT_FW_RESP_DEFAULT_SZ -                     \
                                    sizeof(icp_qat_fw_comn_resp_hdr_t)) /  \
                    sizeof(uint32_t)];
    /**< Fields reserved for future use and assumed set to 0 */

} icp_qat_fw_la_resp_t;


/**
 *****************************************************************************
 * @ingroup icp_qat_fw_la
 *      Definition of the Lookaside SSL Key Material Input
 * @description
 *      This struct defines the layout of input parameters for the
 *      SSL3 key generation (source flat buffer format)
 *
 *****************************************************************************/
typedef struct icp_qat_fw_la_ssl_key_material_input_s
{
    uint64_t seed_addr;
    /**< Pointer to seed */

    uint64_t label_addr;
    /**< Pointer to label(s) */

    uint64_t secret_addr;
    /**< Pointer to secret */

} icp_qat_fw_la_ssl_key_material_input_t;


/**
 *****************************************************************************
 * @ingroup icp_qat_fw_la
 *      Definition of the Lookaside TLS Key Material Input
 * @description
 *      This struct defines the layout of input parameters for the
 *      TLS key generation (source flat buffer format)
 * @note
 *      Secret state value (S split into S1 and S2 parts) is supplied via
 *      Content Descriptor. S1 is placed in an outer prefix buffer, and S2
 *      inside the inner prefix buffer.
 *
 *****************************************************************************/
typedef struct icp_qat_fw_la_tls_key_material_input_s
{
    uint64_t seed_addr;
    /**< Pointer to seed */

    uint64_t label_addr;
    /**< Pointer to label(s) */

} icp_qat_fw_la_tls_key_material_input_t;


#endif /* __ICP_QAT_FW_LA_H__ */
