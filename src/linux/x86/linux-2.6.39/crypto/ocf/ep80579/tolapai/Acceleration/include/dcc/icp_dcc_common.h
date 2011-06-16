/******************************************************************************
 * 
 * @par
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
 *****************************************************************************/
#if !defined(__ICP_DCC_COMMON_H__)
#define __ICP_DCC_COMMON_H__

#if defined(__cplusplus)
extern "C"
{
#endif /* end defined(__cplusplus) */

/**
 *****************************************************************************
 * 
 * @defgroup DCCBaseTypes Debug Base Data Structures
 *
 * @ingroup DCCLibrary
 *
 * @description
 *         This section describes the data structures, defines, typedefs and 
 *         enumerations used by acceleration software components and Debug 
 *         Clients. 
 * 
 *****************************************************************************/


/* Include Header Files */
#include "icp.h"

/**
 *****************************************************************************
 * @ingroup DCCBaseTypes
 *    Maximum number of Modules supported by the DCC.
 *
 * @description
 *    The number of Modules that shall register their version information 
 *    with DCC, apart from the Package version information. 
 *
 * @purpose
 *    The Maximum number of Modules that shall be supported by DCC.
 *
 *****************************************************************************/
#define ICP_DCC_MAX_MODULES 255U

/**
 *****************************************************************************
 * @ingroup DCCBaseTypes
 *    Maximum number of threads supported by the DCC.
 *
 * @description
 *    The number of threads registered with the DCC shall not exceed 
 *    this value.
 *
 * @purpose
 *    Threads register with the DCC to provide their responsiveness. The 
 *    number of threads registered with DCC at any point in time should not 
 *    exceed this value. 
 *
 *****************************************************************************/
#define ICP_DCC_MAX_THREADS 512U

/**
 *****************************************************************************
 * @ingroup DCCBaseTypes
 *    Maximum Length of Thread ID.
 *
 * @description
 *    This value specifies the length of the Thread ID.
 * 
 * @purpose
 *    The Thread ID is defined as a character string. The length of the string 
 *    is defined by this value.
 *
 *****************************************************************************/
#define ICP_DCC_THREAD_ID_LENGTH 16

/**
 *****************************************************************************
 * @ingroup DCCBaseTypes
 *    Maximum Length of Package/Component name.
 *
 * @description
 *    This value specifies the length of the component name.
 * 
 * @purpose
 *    The component name is defined as a character string. The length of the 
 *    string is defined by this value.
 *
 *****************************************************************************/
#define ICP_DCC_COMPONENT_NAME_LENGTH 16

/**
 *****************************************************************************
 * @ingroup DCCBaseTypes
 *    Length of the SEN Message.
 *
 * @description
 *    The Message length should not exceed this value.
 *
 * @purpose
 *    SEN messages are defined as character strings. The maximum size 
 *    of this string is defined by this value.
 *
 *****************************************************************************/
#define ICP_DCC_MAX_MSG_SIZE 128

/**
 *****************************************************************************
 * @ingroup DCCBaseTypes
 *    Length of Module ID String.
 *
 * @description
 *    The Module ID string length should not exceed this value.
 *
 * @purpose
 *    The Module ID, when converted to string format is defined as a 
 *    character string. The maximum length of this string is defined by 
 *    this value.  
 *
 *****************************************************************************/
#define ICP_DCC_MODULE_ID_STR_LENGTH 5

/**
 *****************************************************************************
 * @ingroup DCCBaseTypes
 *    Length of Version String.
 *
 * @description
 *    The Version string length shall not exceed this value.
 *
 * @purpose
 *    The Version String is obtained from major version, minor version and 
 *    patch version. The maximum size of this string is defined by this value.
 *
 *****************************************************************************/
#define ICP_DCC_VERSION_STR_LENGTH 13


/**
 *****************************************************************************
 * @ingroup DCCBaseTypes
 *    Response state of the Thread.
 *
 * @description
 *    This structure provides the thread state. Possible states are "live" 
 *    or "dead". 
 *
 * @purpose
 *    This enumeration type defines the possible states of the executing thread.
 *    It indicates whether the thread has responded back within a certain time 
 *    period or not.
 *
 *****************************************************************************/
typedef enum icp_dcc_thread_status_s
{
    ICP_DCC_THREAD_ID_LIVE, /**< Thread is responding */
    ICP_DCC_THREAD_ID_DEAD  /**< Thread is not responding */
} icp_dcc_thread_status_t;

/**
 *****************************************************************************
 * @ingroup DCCBaseTypes
 *    Thread ID.
 *
 * @description
 *    This structure provides thread Identification details.
 *
 * @purpose
 *    This structure provides Thread ID as a string.
 *
 *****************************************************************************/
typedef struct icp_dcc_thread_id_s
{
    uint8_t threadIdString[ICP_DCC_THREAD_ID_LENGTH]; /**< Thread ID String */
} icp_dcc_thread_id_t;

/**
 *****************************************************************************
 * @ingroup DCCBaseTypes
 *    Software Version information.
 *
 * @description
 *    This structure provides Software Version information. 
 *
 * @purpose
 *    The Software Version information of Package and Components are provided 
 *    through this structure.
 *
 *****************************************************************************/
typedef struct icp_dcc_ver_info_s
{
    uint8_t   name[ICP_DCC_COMPONENT_NAME_LENGTH];     
                              /**< Software Package/Component Name. */
    uint8_t  majorVersion;       /**< Software Major Version Number */
    uint8_t  minorVersion;       /**< Software Minor Version Number */
    uint16_t  patchVersion;       /**< Software Patch Version Number */
} icp_dcc_ver_info_t;


/**
 *****************************************************************************
 * @ingroup DCCBaseTypes
 *    Priority of the Software Error Notification (SEN) event message. 
 *
 * @description
 *    This structure provides supported SEN message priorities. 
 *
 * @purpose
 *    This enumeration type defines the priorities of the SEN events that occur
 *    in the system. Supported priority types are: Severe and Warning. 
 *
 *****************************************************************************/
typedef enum icp_dcc_sen_priority_s
{
    ICP_DCC_SEN_MSG_PRIORITY_SEVERE,     /**< Critical errors impacting 
                                               functionality or requiring 
                                               immediate attention */
    ICP_DCC_SEN_MSG_PRIORITY_WARNING     /**< Important system notifications */
} icp_dcc_sen_priority_t;

/**
 *****************************************************************************
 * @ingroup DCCBaseTypes
 *    Software Error Notification (SEN) event message details.
 *
 * @description
 *    This structure contains SEN event details. 
 *
 * @purpose
 *    This data structure defines the SEN message structure. This message 
 *    structure has to be formed by the Acceleration Software Modules 
 *    and sent to DCC. 
 *    DCC passes this information to the Debug Client's registered 
 *    event handler.
 *
 *****************************************************************************/
typedef struct icp_dcc_sen_msg_s
{
    icp_dcc_sen_priority_t senPriority;  
    /**< Priority of the SEN event */
    uint64_t               timestamp;     
    /**< Timestamp when the event is received by DCC */
    uint32_t               moduleId;      
    /**< Module ID */
    uint16_t               eventId;   
    /**< Event ID */
    uint16_t               eventInfoSize; 
    /**< Size of the SEN eventInfo string in bytes */
    uint8_t                eventInfo[ICP_DCC_MAX_MSG_SIZE];
    /**< Descriptive null-terminated SEN Event string */	
} icp_dcc_sen_msg_t;



#if defined(__cplusplus)
}
#endif /* end defined(__cplusplus) */

#endif /* end !defined(__ICP_DCC_COMMON_H__) */
