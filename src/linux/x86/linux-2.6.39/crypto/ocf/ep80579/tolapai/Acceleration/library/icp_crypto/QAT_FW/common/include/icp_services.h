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
 * @file icp_services.h
 * @defgroup icp_services ICP Services Definitions
 * @ingroup icp_services
 * $Revision: 0.1 $
 * @description
 *      This file documents the service Id and Request types used by Services
 *      block.
 *
 *****************************************************************************/

#ifndef __ICP_SERVICES_H__
#define __ICP_SERVICES_H__

/*
 ******************************************************************************
 * Include public/global header files
 ******************************************************************************
 */


/**
 *****************************************************************************
 * @ingroup icp_services
 *       Definition of the Service id's
 * @description
 *       Enumeration used to indicate the services Id by services blocks
 *
 *****************************************************************************/

typedef enum
{
        ICP_ARCH_IF_SERV_NULL=0,              /**< NULL Service ID type */
        ICP_ARCH_IF_SERV_QAT_FW=1,            /**< QAT FW Service ID */
        ICP_ARCH_IF_SERV_RANGE_MATCHING=2,    /**< Range-Matching Service ID */
        ICP_ARCH_IF_SERV_DELIMITER            /**< Delimiter Service ID type */
} icp_arch_if_serv_id_t;


/**
 *****************************************************************************
 * @ingroup icp_services
 *      Definition of the request types for the S icp arch interfaces
 * @description
 *      Enumeration used to indicate the ids of the request types
 *      used on the S interfaces that make up the icp sw architecture.
 *      A block of service request type ID's is reserved for each
 *      service component.
 *
 *      01->20:reserved for QAT-AE service
 *      21->30:reserved for Range-Matching service
 *
 *****************************************************************************/

typedef enum
{
        ICP_ARCH_IF_REQ_NULL=0,                   /**< NULL request type */

        /* QAT-AE Service Request Type IDs - 01 to 20 */
        ICP_ARCH_IF_REQ_QAT_FW_INIT=1,            /**< QAT-FW Initialization Request */
        ICP_ARCH_IF_REQ_QAT_FW_ADMIN=2,           /**< QAT-FW Administration Request */
        ICP_ARCH_IF_REQ_QAT_FW_LA=3,              /**< QAT-FW Lookaside Request */
        ICP_ARCH_IF_REQ_QAT_FW_PKE=4,             /**< QAT-FW PKE Request */
        ICP_ARCH_IF_REQ_QAT_FW_IPSEC=5,           /**< QAT-FW IPSec Request */
        ICP_ARCH_IF_REQ_QAT_FW_SSL=6,             /**< QAT-FW SSL Request */
        ICP_ARCH_IF_REQ_QAT_FW_DMA=7,             /**< QAT-FW DMA Request */

        /* IP Service (Range Match and Exception) Blocks Request Type IDs 21 - 30 */
        ICP_ARCH_IF_REQ_RM_FLOW_MISS=21,       /**< RM flow miss request */
        ICP_ARCH_IF_REQ_RM_FLOW_TIMER_EXP,     /**< RM flow timer exp Request */
        ICP_ARCH_IF_REQ_IP_SERVICES_RFC_LOOKUP_UPDATE, /**< RFC Lookup request */
        ICP_ARCH_IF_REQ_IP_SERVICES_CONFIG_UPDATE,     /**< Config Update request */
        ICP_ARCH_IF_REQ_IP_SERVICES_FCT_CONFIG,        /**< FCT Config request */
        ICP_ARCH_IF_REQ_IP_SERVICES_NEXT_HOP_TIMER_EXPIRY, /**< NH Timer expiry request */
        ICP_ARCH_IF_REQ_IP_SERVICES_EXCEPTION,        /**< Exception processign request */
        ICP_ARCH_IF_REQ_IP_SERVICES_STACK_DRIVER,     /**< Send to SD request */
        ICP_ARCH_IF_REQ_IP_SERVICES_ACTION_HANDLER,   /**< Send to AH request */
        ICP_ARCH_IF_REQ_IP_SERVICES_EVENT_HANDLER,    /**< Send to EH request */
        ICP_ARCH_IF_REQ_DELIMITER                     /**< End delimiter */
} icp_arch_if_request_t;

#ifdef ENABLE_DEBUG_SUPPORT

/**
 *****************************************************************************
 * @ingroup icp_services
 *      Definition of the different PPS
 * @description
 *      Enumeration used to indicate the ids of the FP PPS
 *****************************************************************************/

typedef enum icp_pps_id_s
{
	  ICP_PPS_ID_NONE=0,                   /**< NULL PPS ID */
      ICP_PPS_ID_L2L3,					   /**< PPS ID of L2L3 PPS */
      ICP_PPS_ID_IP_SERVICES,			   /**< PPS ID of IP Services */
      ICP_PPS_ID_RX_EVENT_DISP,			   /**< PPS ID of Rx Event Dispatch */
      ICP_PPS_ID_RX_HELPER,				   /**< PPS ID of Rx Helper */
      ICP_PPS_ID_TX,				       /**< PPS ID of Tx */
      ICP_PPS_ID_LAST					   /**< End delimiter */
} icp_pps_id_t;


/*************************************************************************
 * @ingroup icp_services
 * 		ICP PPS Component Version Structure
 *
 * This structure declares the layout of various fields in the version 
 * field  that will be used by various components. When sharing the data 
 * between different endian processors, care should be taken to do 
 * appropriate translation.
 **************************************************************************/
typedef struct icp_debug_pps_comp_ver_s
{
      uint32_t component_id;			 /**< ID of Component 			 */
      uint32_t major_ver_num:8,			 /**< Major ver num of component */
      		   minor_ver_num:8,			 /**< Minor ver num of component */
       		   patch_num:16;			 /**< Patch num of component 	 */
}icp_debug_pps_comp_ver_t;

#endif /* ENABLE_DEBUG_SUPPORT */

#endif /* __ICP_SERVICES_H__ */
