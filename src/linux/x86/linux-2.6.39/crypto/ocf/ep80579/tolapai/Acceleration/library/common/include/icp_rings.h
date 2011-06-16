/***************************************************************************
 *
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
 *
 ***************************************************************************/

/*
 *****************************************************************************
 * Doxygen group definitions
 ****************************************************************************/

/**
 *****************************************************************************
 * @file icp_rings.h
 * 
 * @defgroup icp_Rings ICP Ring Id Allocation details
 * 
 * @ingroup icp_Lac
 * 
 * @description
 *      This file contains details of the allocation of ICP rings
 *      throughout the system.  Any software component which uses rings
 *      needs to have update this header file with the necessary details 
 * 
 *****************************************************************************/
#ifndef ICP_RINGS_H
#define ICP_RINGS_H

/**
 *****************************************************************************
 * @ingroup icp_Rings
 *      ICP Ring default size
 * 
 * @description
 *      Defines the recommended size for all ICP rings in the system.
 *      The size is expressed as a number of double-words.
 *
 *****************************************************************************/
#define ICP_RING_DEFAULT_SIZE  (IX_ICP_RING_SIZE_16K)


/**
 *****************************************************************************
 * @ingroup icp_Rings
 *      ICP Ring Id allocation list
 * @description
 *      Enumeration which is used to associate specific ring Ids with specific
 *      functions and software components in the system.  Unused rings below
 *      may be renamed to indicate that the ring has been allocated. Rings must
 *      be named to clearly indicate their purpose, as illustrated below.   
 * @note
 *      Each ring must be created by the software component that owns the ring.
 *      Resource Manager APIs are provided to assist with ring creation.
 *
 *****************************************************************************/
typedef enum
{
    ICP_RING_UNUSED_0,                                      /**< ICP Ring 0 */
    ICP_RING_UNUSED_1,                                      /**< ICP Ring 1 */
    ICP_RING_UNUSED_2,                                      /**< ICP Ring 2 */
    ICP_RING_UNUSED_3,                                      /**< ICP Ring 3 */
    ICP_RING_UNUSED_4,                                      /**< ICP Ring 4 */
    ICP_RING_UNUSED_5,   	                            /**< ICP Ring 5 */
    ICP_RING_UNUSED_6,                                      /**< ICP Ring 6 */
    ICP_RING_UNUSED_7,                                      /**< ICP Ring 7 */
    L2L3_TO_QAT_PIPE,                                       /**< ICP Ring 8 */
    ICP_RING_UNUSED_9,                                      /**< ICP Ring 9 */
    ICP_RING_UNUSED_10,                                     /**< ICP Ring 10 */
    ICP_RING_UNUSED_11,                                     /**< ICP Ring 11 */
    IPFFC_TO_L2L3_PIPE,                                     /**< ICP Ring 12 */
    ICP_RING_LAC_PKE_REQUEST,                               /**< ICP Ring 13 */
    ICP_RING_LAC_PKE_RESPONSE,                              /**< ICP Ring 14 */
    ICP_RING_LAC_LA_HI_REQUEST,                             /**< ICP Ring 15 */
    ICP_RING_LAC_LA_LO_REQUEST,                             /**< ICP Ring 16 */
    ICP_RING_LAC_LA_RESPONSE,                               /**< ICP Ring 17 */
    ICP_RING_QATAL_ADMIN_REQUEST,                           /**< ICP Ring 18 */
    ICP_RING_QATAL_ADMIN_RESPONSE,                          /**< ICP Ring 19 */
    ICP_RING_IPSEC_REQUEST,                                 /**< ICP Ring 20 */
    ICP_RING_IPSEC_RESPONSE,                                /**< ICP Ring 21 */
    ICP_RING_UNUSED_22,                                     /**< ICP Ring 22 */
    ICP_RING_UNUSED_23,                                     /**< ICP Ring 23 */
    ICP_RING_UNUSED_24,                                     /**< ICP Ring 24 */
    ICP_RING_UNUSED_25,                                     /**< ICP Ring 25 */
    ICP_RING_UNUSED_26,                                     /**< ICP Ring 26 */
    ICP_RING_UNUSED_27,                                     /**< ICP Ring 27 */
    ICP_RING_UNUSED_28,                                     /**< ICP Ring 28 */
    ICP_RING_UNUSED_29,                                     /**< ICP Ring 29 */
    ICP_RING_UNUSED_30,                                     /**< ICP Ring 30 */
    ICP_RING_UNUSED_31,                                     /**< ICP Ring 31 */
    RX_TO_L2L3_HI_PRIO_PIPE,                                /**< ICP Ring 32 */
    RX_TO_L2L3_LO_PRIO_PIPE,                                /**< ICP Ring 33 */
    SERVICES_TO_L2L3_PIPE,                                  /**< ICP Ring 34 */
    IA_TO_L2L3_PIPE,                                        /**< ICP Ring 35 */
    L2L3_TO_TX_PIPE1,                                       /**< ICP Ring 36 */
    L2L3_TO_TX_PIPE2,                                       /**< ICP Ring 37 */
    L2L3_TO_TX_PIPE3,                                       /**< ICP Ring 38 */
    L2L3_TO_TCP_PIPE,                                       /**< ICP Ring 39 */
    L2L3_TO_IP_SERVICES_PIPE,                               /**< ICP Ring 40 */
    CC_TO_SD_PKT_HANDLER2,                                  /**< ICP Ring 41 */
    CC_TO_SD_PKT_HANDLER1,	                            /**< ICP Ring 42 */
    ICP_RING_CSF_DCC_SEN,                                   /**< ICP Ring 43 */
    IA_BUFFER_FREE_PIPE,                                    /**< ICP Ring 44 */
    IP_SERVICES_PIPE_TO_RM_PIPE1,                           /**< ICP Ring 45 */
    IP_SERVICES_PIPE_TO_RM_PIPE2,                           /**< ICP Ring 46 */
    IP_SERVICES_PIPE_TO_SD_PIPE0,                           /**< ICP Ring 47 */
    ICP_RING_UNUSED_48,                                     /**< ICP Ring 48 */
    ICP_RING_UNUSED_49,                                     /**< ICP Ring 49 */
    AEH_HI_PRIO_RING_ID,                                    /**< ICP Ring 50 */
    AEH_LO_PRIO_RING_ID,                                    /**< ICP Ring 51 */
    AEH_EH_RING_ID,                                         /**< ICP Ring 52 */
    AEH_CC_TO_EH_RING_ID,                                   /**< ICP Ring 53 */
    CC_TO_SD_PKT_HANDLER0,                                  /**< ICP Ring 54 */
    ICP_RING_CSF_MBIL_POOL_0,                               /**< ICP Ring 55 */
    ICP_RING_CSF_MBIL_POOL_1,                               /**< ICP Ring 56 */
    ICP_RING_CSF_MBIL_POOL_2,                               /**< ICP Ring 57 */
    ICP_RING_CSF_MBIL_POOL_3,                               /**< ICP Ring 58 */
    ICP_RING_CSF_MBIL_POOL_4,                               /**< ICP Ring 59 */
    ICP_RING_CSF_RESERVED_60,                               /**< ICP Ring 60 */
    IP_SERVICES_PIPE_TO_SD_PIPE1,                           /**< ICP Ring 61 */
    IP_SERVICES_PIPE_TO_SD_PIPE2,                           /**< ICP Ring 62 */
    ICP_RING_UNUSED_63                                      /**< ICP Ring 63 */

} icp_ring_id_t;




/**
 *****************************************************************************
 * @ingroup icp_Rings
 *      ICP Software Ring Id list
 * @description
 *      Enumeration which is used to define the software ring Ids whcih is 
 *      used across the application.
 * @note
 *      Each ring must be created by the software component that owns the ring.
 *      Resource Manager APIs are provided to assist with ring creation.
 *
 *****************************************************************************/
typedef enum
{
    ICP_SW_RING_CSF_RM_CONF_PROP_POOL,                     /**< ICP SW Ring 0 */
    ICP_SW_RING_CSF_BTR_INDIRECT_POOL,                     /**< ICP SW Ring 1 */
    ICP_SW_RING_CSF_RESERVED_2,                            /**< ICP SW Ring 2 */
    ICP_SW_RING_CSF_RESERVED_3,                            /**< ICP SW Ring 3 */
    ICP_SW_RING_CSF_RESERVED_4,                            /**< ICP SW Ring 4 */
    ICP_SW_RING_CSF_RESERVED_5,    	                       /**< ICP SW Ring 5 */
    ICP_SW_RING_CSF_RESERVED_6,                            /**< ICP SW Ring 6 */
    ICP_SW_RING_CSF_RESERVED_7,                            /**< ICP SW Ring 7 */
    ICP_SW_RING_CSF_RESERVED_8,                            /**< ICP SW Ring 8 */
    ICP_SW_RING_CSF_RESERVED_9,                            /**< ICP SW Ring 9 */
    ICP_SW_RING_MM_POOL_ID, 	                           /**< ICP SW Ring 10 */
    ICP_SW_RING_UNUSED_11,		                           /**< ICP SW Ring 11 */
    ICP_SW_RING_UNUSED_12,                                 /**< ICP SW Ring 12 */
    ICP_SW_RING_UNUSED_13,                                 /**< ICP SW Ring 13 */
    ICP_SW_RING_UNUSED_14,                                 /**< ICP SW Ring 14 */
    ICP_SW_RING_UNUSED_15,                                 /**< ICP SW Ring 15 */
    ICP_SW_RING_UNUSED_16,                                 /**< ICP SW Ring 16 */
    ICP_SW_RING_UNUSED_17,                                 /**< ICP SW Ring 17 */
    ICP_SW_RING_UNUSED_18,                                 /**< ICP SW Ring 18 */
    ICP_SW_RING_UNUSED_19,                                 /**< ICP SW Ring 19 */
    ICP_SW_RING_UNUSED_20,                                 /**< ICP SW Ring 20 */
    ICP_SW_RING_UNUSED_21,                                 /**< ICP SW Ring 21 */
    ICP_SW_RING_UNUSED_22,                                 /**< ICP SW Ring 22 */
    ICP_SW_RING_UNUSED_23,                                 /**< ICP SW Ring 23 */
    ICP_SW_RING_UNUSED_24,                                 /**< ICP SW Ring 24 */
    ICP_SW_RING_UNUSED_25,                                 /**< ICP SW Ring 25 */
    ICP_SW_RING_UNUSED_26,                                 /**< ICP SW Ring 26 */
    ICP_SW_RING_UNUSED_27,                                 /**< ICP SW Ring 27 */
    ICP_SW_RING_UNUSED_28,                                 /**< ICP SW Ring 28 */
    ICP_SW_RING_UNUSED_29,                                 /**< ICP SW Ring 29 */
    ICP_SW_RING_UNUSED_30,                                 /**< ICP SW Ring 30 */
    ICP_SW_RING_UNUSED_31,                                 /**< ICP SW Ring 31 */
    ICP_SW_RING_UNUSED_32,                                 /**< ICP SW Ring 32 */
    ICP_SW_RING_UNUSED_33,                                 /**< ICP SW Ring 33 */
    ICP_SW_RING_UNUSED_34,                                 /**< ICP SW Ring 34 */
    ICP_SW_RING_UNUSED_35,                                 /**< ICP SW Ring 35 */
    ICP_SW_RING_UNUSED_36,                                 /**< ICP SW Ring 36 */
    ICP_SW_RING_UNUSED_37,                                 /**< ICP SW Ring 37 */
    ICP_SW_RING_UNUSED_38,                                 /**< ICP SW Ring 38 */
    ICP_SW_RING_UNUSED_39,                                 /**< ICP SW Ring 39 */
    ICP_SW_RING_UNUSED_40,                                 /**< ICP SW Ring 40 */
    ICP_SW_RING_UNUSED_41,                                 /**< ICP SW Ring 41 */
    ICP_SW_RING_UNUSED_42,                                 /**< ICP SW Ring 42 */
    ICP_SW_RING_UNUSED_43,                                 /**< ICP SW Ring 43 */
    ICP_SW_RING_UNUSED_44,                                 /**< ICP SW Ring 44 */
    ICP_SW_RING_UNUSED_45,                                 /**< ICP SW Ring 45 */
    ICP_SW_RING_UNUSED_46,                                 /**< ICP SW Ring 46 */
    ICP_SW_RING_UNUSED_47,                                 /**< ICP SW Ring 47 */
    ICP_SW_RING_UNUSED_48,                                 /**< ICP SW Ring 48 */
    ICP_SW_RING_UNUSED_49,                                 /**< ICP SW Ring 49 */
    ICP_SW_RING_UNUSED_50,                                 /**< ICP SW Ring 50 */
    ICP_SW_RING_UNUSED_51,                                 /**< ICP SW Ring 51 */
    ICP_SW_RING_UNUSED_52,                                 /**< ICP SW Ring 52 */
    ICP_SW_RING_UNUSED_53,                                 /**< ICP SW Ring 53 */
    ICP_SW_RING_UNUSED_54,                                 /**< ICP SW Ring 54 */
    ICP_SW_RING_UNUSED_55,                                 /**< ICP SW Ring 55 */
    ICP_SW_RING_UNUSED_56,                                 /**< ICP SW Ring 56 */
    ICP_SW_RING_UNUSED_57,                                 /**< ICP SW Ring 57 */
    ICP_SW_RING_UNUSED_58,                                 /**< ICP SW Ring 58 */
    ICP_SW_RING_UNUSED_59,                                 /**< ICP SW Ring 59 */
    ICP_SW_RING_UNUSED_60,                                 /**< ICP SW Ring 60 */
    ICP_SW_RING_UNUSED_61,                                 /**< ICP SW Ring 61 */
    ICP_SW_RING_UNUSED_62,                                 /**< ICP SW Ring 62 */
    ICP_SW_RING_UNUSED_63                                  /**< ICP SW Ring 63 */

} icp_sw_ring_id_t;



#endif /* ICP_RINGS_H */
