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
 * @file icp_qat_fw_admin.h
 * @defgroup icp_qat_fw_admin ICP QAT FW Admin Interface Definitions
 * @ingroup icp_qat_fw
 *
 * @description
 *      This file documents QAT FW admin requests and responses
 *
 *****************************************************************************/

#ifndef __ICP_QAT_FW_ADMIN_H__
#define __ICP_QAT_FW_ADMIN_H__

/*
******************************************************************************
* Include local header files
******************************************************************************
*/

#include "icp_qat_fw.h"

/**
 *****************************************************************************
 * @ingroup icp_qat_fw_admin
 *      Definition of the admin command types
 * @description
 *      Enumeration which is used to indicate the ids of the admin commands
 *
 *****************************************************************************/

typedef enum
{
   ICP_QAT_FW_ADMIN_CMD_GET=0,         /**< Admin get command */
   ICP_QAT_FW_ADMIN_CMD_SET=1,         /**< Admin set command */
   ICP_QAT_FW_ADMIN_CMD_DELIMITER      /**< Delimiter type */
} icp_qat_fw_admin_cmd_id_t;


/**
 *****************************************************************************
 * @ingroup icp_qat_fw_admin
 *      QAT FW Admin request
 * @description
 *      This struct contains data needed to generate an admin request. For the
 *      GET case all fields are used but for SET the status_tbl_sz and
 *      status_tbl_addr are unused and must be set to 0.
 *
 *****************************************************************************/
typedef struct icp_qat_fw_admin_req_s
{
   icp_qat_fw_comn_req_hdr_t comn_req;
    /**< Common interface request */

   uint8_t admin_cmd_id;
    /**< Admin command that is described in the request */

   uint8_t resrvd1;
    /**< Reserved field and must be set to 0 by the client */

   uint16_t status_tbl_sz;
    /**< Size of the status table located at the status_tbl_addr */

   uint32_t resrvd2;
    /**< Reserved field and must be set to 0 by the client */

   uint64_t status_tbl_addr;
    /**< Physical address in DRAM where the status information will be written
    * This must be quad word aligned */

   uint32_t resrvd3;
    /**< Reserved field and must be set to 0 by the client */

   uint32_t resrvd4;
    /**< Reserved field and must be set to 0 by the client */
} icp_qat_fw_admin_req_t;

/**
 *****************************************************************************
 * @ingroup icp_qat_fw_admin
 *      QAT FW status table
 * @description
 *      This struct contains the status information returned by the QAT FW on
 *      reception of an admin_get request.
 *
 *****************************************************************************/
typedef struct icp_qat_fw_admin_status_tbl_s
{
   uint64_t resp_ring_full_count;
    /**< Number of times a response was attempted to get put on the ring when
         the ring was already full  */

   uint64_t qwrd_aligned_pkt_count;
    /**< Count of packets with an aligned src data ptr */

   uint64_t unaligned_pkt_count;
    /**< Count of packets with an unaligned source data ptr */

   uint64_t no_page_free_count;
    /**< Count of times that no data page is free */

   uint64_t resrvd1;
    /**< Counter1 reserved for future usage */

   uint64_t resrvd2;
    /**< Counter2 reserved for future usage */

   uint64_t resrvd3;
    /**< Counter3 reserved for future usage */

   uint64_t resrvd4;
    /**< Counter4 reserved for future usage */
} icp_qat_fw_admin_status_tbl_t;


/* Private defines */
#define QAT_FW_ADMIN_RESP_LIVENESS_RSVD_SZ   3
/**< @ingroup icp_qat_fw_admin
 * Admin Reserved Response Liveness fields size in bytes  */

#define QAT_FW_ADMIN_RESP_CONTEXT_RSVD_SZ    3
/**< @ingroup icp_qat_fw_admin
 * Admin Reserved Response Context fields size in bytes */

#define QAT_FW_ADMIN_RESP_RSVD_SZ            24
/**< @ingroup icp_qat_fw_admin
 * Admin Reserved Response Reserved fields size in bytes */


/**
 *****************************************************************************
 * @ingroup icp_qat_fw_admin
 *      QAT FW admin message response
 * @description
 *      This struct contains data needed to generate an admin response. These
 *      always go to the response ring
 *
 *****************************************************************************/
typedef struct icp_qat_fw_admin_resp_s
{
   icp_qat_fw_comn_resp_hdr_t comn_resp;
    /**< Common interface response */

   uint16_t version_major_num;
    /**< QAT FW major build number */

   uint16_t version_minor_num;
    /**< QAT FW minor build number */

   uint32_t version_patch_num;
    /**< QAT FW build patch number */

   uint8_t resrvd_liveness[QAT_FW_ADMIN_RESP_LIVENESS_RSVD_SZ];
    /**< Reserved fields of liveness word */

   uint8_t liveness_info;
    /**< QAT FW liveness info */

   uint8_t resrvd_context[QAT_FW_ADMIN_RESP_CONTEXT_RSVD_SZ];
    /**< Reserved fields of the context word */

   uint8_t context_id;
    /**< Context id of the context that serviced the status request */

   uint32_t request_recvd_cnt;
   /**< Number of requests received */

   uint32_t response_sent_cnt;
   /**< Number of responses sent */

   uint8_t resrvd[QAT_FW_ADMIN_RESP_RSVD_SZ];
    /**< Reserved padding of the response out to the default size */
} icp_qat_fw_admin_resp_t;


#endif  /* __ICP_QAT_FW_ADMIN_H__ */

