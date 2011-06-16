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
 * @file icp_qat_fw_init.h
 * @defgroup icp_qat_fw_init ICP QAT FW Initialisation Interface Definitions
 * @ingroup icp_qat_fw
 *
 * @description
 *      This file documents structs used at init time in the configuration of
 *      the QAT FW.
 *
 *****************************************************************************/

#ifndef __ICP_QAT_FW_INIT_H__
#define __ICP_QAT_FW_INIT_H__

/*
******************************************************************************
* Include local header files
******************************************************************************
*/

#include "icp_qat_fw.h"

/**
 *****************************************************************************
 * @ingroup icp_qat_fw_init
 *      Definition of the init time command types
 * @description
 *      Enumeration which is used to indicate the ids of commands that are
 *      processed at init time
 *
 *****************************************************************************/

typedef enum
{
   ICP_QAT_FW_INIT_CMD_CONFIG_NRBG=0,          /**< NRBG Config command type */
   ICP_QAT_FW_INIT_CMD_ENABLE_DRBG=1,          /**< DRBG Enable command type */
   ICP_QAT_FW_INIT_CMD_SET_CONSTS=2,          /**< Setup Constants command type */
   ICP_QAT_FW_INIT_CMD_SET_RING_INFO=3,       /**< Setup Ring Info command type */
   ICP_QAT_FW_INIT_CMD_INIT_MMP=4,             /**< Init MMP command type */
   ICP_QAT_FW_INIT_CMD_INIT_FINAL=5,          /**< Init final command type */
   ICP_QAT_FW_INIT_CMD_DELIMITER             /**< Delimiter type */
} icp_qat_fw_init_cmd_id_t;

/**
 *****************************************************************************
 * @ingroup icp_qat_fw_init
 *      QAT FW Init time request
 * @description
 *      This struct contains data needed to generate a init time request
 *
 *****************************************************************************/
typedef struct icp_qat_fw_init_req_s
{
   icp_qat_fw_comn_req_hdr_t comn_req;
    /**< Common interface request */

   uint8_t init_cmd_id;
    /**< Init time command that is described in the request */

   uint8_t resrvd1;
    /**< Reserved field and must be set to 0 by the client */

   uint16_t data_sz;
    /**< Optional size of the data used by the init command located at the
    * data_addr. If the init command has no associated data in DRAM then
    * field must be set to 0 */

   uint32_t resrvd2;
    /**< Reserved field and must be set to 0 by the client */

   uint64_t data_addr;
    /**< Physical address in DRAM where the data needed by the init command is
    * located. Must be set to zero if there is no DRAM data associated with
    * the command */

   union {
      uint32_t resrvd3;
       /**< Reserved field and must be set to 0 by the client */

      uint32_t bulk_rings_mask;
       /**< Bitmask for bulk service rings to be polled, 1 on a given bit
           position represents the ring ID to be checked for demand. */
   } u1;

   union {
      uint32_t resrvd4;
       /**< Reserved field and must be set to 0 by the client */

      uint32_t pke_rings_mask;
       /**< PKE only bitmask */
   } u2;

} icp_qat_fw_init_req_t;

/* ========================================================================= */
/*                                   Ring Polling setup definitions */
/* ========================================================================= */

/* Number of entries in the polling ring table */
#define INIT_RING_TABLE_SZ                      (32)

#define ICP_QAT_FW_INIT_NULL_RING_ENTRY         (0xFF)
/**< @ingroup icp_qat_fw_init
 * Definition of a NULL ring entry in the ring polling table. Note ensure that
 * this field is the correct endianness in the ring table */

/**
 *****************************************************************************
 * @ingroup icp_qat_fw_init
 *      QAT FW Init ring parameters entry
 * @description
 *      This struct contains fields neccesary to initialize the ring table
 *
 *****************************************************************************/
typedef struct icp_qat_fw_init_ring_params_s
{
   uint16_t reserved;
   /**< Reserved field which must be set to 0 by the client */

   uint8_t init_weight;
   /**< Initial ring weight: -1 ... 0 */
    /**< -1 is equal to FF, -2 is equal to FE, the weighting uses negative logic
     * where FF means poll the ring once, -2 is poll the ring twice, 0 is poll
     * the ring 255 times                                                   */

   uint8_t curr_weight;
   /**< Current ring weight (working copy), has to be equal to init_weight */

} icp_qat_fw_init_ring_params_t;

/**
 *****************************************************************************
 * @ingroup icp_qat_fw_init
 *      QAT FW Init ring polling table
 * @description
 *      This struct contains list of rings that the QAT FW will continuously
 *      poll checking for request demand
 *
 *****************************************************************************/
typedef struct icp_qat_fw_init_ring_table_s
{
   icp_qat_fw_init_ring_params_t bulk_rings[INIT_RING_TABLE_SZ];
   /**< An array of ring parameters */
} icp_qat_fw_init_ring_table_t;


/* ========================================================================= */
/*                                     Constants setup definitions */
/* ========================================================================= */

/* Private defines, obtained by a client using sizeof(struct_field_name)*/

/* Size in bytes of the incrementing pad pattern */
#define INIT_INCR_PAD_CONSTS_SZ                   (256)

/* Size in bytes of the zero pad pattern */
#define INIT_ZERO_PAD_CONSTS_SZ                   (64)

/* Size in bytes of the ones pad pattern */
#define INIT_ONES_PAD_CONSTS_SZ                   (64)

/* Size in bytes of the msb set and zero pad pattern */
#define INIT_MSB_ZERO_PAD_CONSTS_SZ                (64)

/**
 *****************************************************************************
 * @ingroup icp_qat_fw_init
 *      QAT FW Init ring polling table
 * @description
 *      This struct contains list of constants used by the QAT FW
 *
 *****************************************************************************/
typedef struct icp_qat_fw_init_consts_table_s
{

   uint8_t incr_pad_pattern[INIT_INCR_PAD_CONSTS_SZ];
    /**< Incrementing pad pattern */

   uint8_t zero_pad_pattern[INIT_ZERO_PAD_CONSTS_SZ];
    /**< Zero pad pattern */

   uint8_t ones_pad_pattern[INIT_ONES_PAD_CONSTS_SZ];
    /**< Zero pad pattern */

   uint8_t msb_zero_pad_pattern[INIT_MSB_ZERO_PAD_CONSTS_SZ];
    /**< MSb set and zeros pad pattern */
    /**< most significant bit             */

   uint64_t align_config;
    /**< Definition of the default data alignment */

   uint64_t drbg_cipher_config;
    /**< Definition of the default configuration for the drbg */


} icp_qat_fw_init_consts_table_t;

/**
 *****************************************************************************
 * @ingroup icp_qat_fw_init
 *      Constants setup request message definition
 *
 * @description
 *      This request is based on the common init request format. For this
 *      message the data_addr corresponds to the address of the constants
 *      in DRAM and the data_sz is the size of the constants there
 *
 *****************************************************************************/
typedef icp_qat_fw_init_req_t icp_qat_fw_init_const_setup_req_t;

/**
 *****************************************************************************
 * @ingroup icp_qat_fw_init
 *      QAT FW Init time response
 * @description
 *      This struct contains data needed to generate a init time response.
 *      These always go to the ET rings with size ICP_QAT_FW_RESP_DEFAULT_SZ
 *      bytes
 *
 *****************************************************************************/
typedef struct icp_qat_fw_init_resp_s
{
   icp_qat_fw_comn_resp_hdr_t comn_resp;
    /**< Common interface response */


   uint8_t resrvd[ICP_QAT_FW_RESP_DEFAULT_SZ - \
                                        sizeof(icp_qat_fw_comn_resp_hdr_t)];
    /**< Reserved padding out to the default response size */
} icp_qat_fw_init_resp_t;

#endif  /* __ICP_QAT_FW_INIT_H__ */



