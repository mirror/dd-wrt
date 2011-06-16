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

/**
 *****************************************************************************
 * @file icp_dcc_cfg.h
 * 
 * @defgroup icp_Dcc DCC ASD Basic Init API
 * 
 * @ingroup icp
 *
 * @description
 *      This is the top level header file that exports the Basic Init API
 *      for the DCC subcomponent. It is used by ASD to initialise DCC.
 *
 *****************************************************************************/
#ifndef ICP_DCC_CFG_H
#define ICP_DCC_CFG_H 1

#include "cpa.h"
#include "icp_asd_cfg.h"

#ifndef ICP_DCC_EXPORT_FUNCTION
#define ICP_DCC_EXPORT_FUNTION    extern
#endif


#define DCC_MODULE_VERSION	1
#define OSAL_REQUIRED_MIN	1
#define OSAL_REQUIRED_MAX	2
#define OSAL_REQUIRED_PREF	1


/**
 *****************************************************************************
 * @ingroup icp_Dcc 
 *      Initialize DCC Library
 * 
 * @description
 *      This function will initialize the DCC Library. It will create the 
 *      required tables and setup the default debug functionality handlers
 *
 * @context
 *      This function may sleep and therefore may not be called from interrupt
 *      context.
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is synchronous and blocking.
 *      
 * @reentrant
 *      No
 *
 * @threadSafe
 *      No
 *
 * @param 
 *     
 *     
 * @retval  CPA_STATUS_SUCCESS If successful
 * @retval  CPA_STATUS_RESOURCE Error related to system resource
 * @retval  CPA_STATUS_FAIL Unspecified error 
 *
 * @pre
 *      None
 *
 * @post
 *      None.
 *
 *****************************************************************************/
ICP_DCC_EXPORT_FUNTION
CpaStatus 
icp_AsdCfgDccInit(
                           CpaInstanceHandle instanceHandle, 
                           icp_asd_cfg_param_get_cb_func_t getCfgParamFunc
                               );

/**
 *****************************************************************************
 * @ingroup icp_Dcc
 *      Starts the DCC library.
 * 
 *      This function is a place holder.
 *
 * @context
 *      Any context
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is nonblocking.
 *      
 * @reentrant
 *      Yes
 *
 * @threadSafe
 *      Yes
 *
 * @retval  ICP_STATUS_SUCCESS        No error 
 *
 * @pre
 *      None.
 *
 * @post
 *      None.
 *
 *****************************************************************************/
ICP_DCC_EXPORT_FUNTION
CpaStatus 
icp_AsdCfgDccStart(
                           CpaInstanceHandle instanceHandle
                               );

/**
 *****************************************************************************
 * @ingroup icp_Dcc
 *      Stops the DCC library.
 * 
 *      This function is a place holder.
 *
 * @context
 *      Any context
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is nonblocking.
 *      
 * @reentrant
 *      Yes
 *
 * @threadSafe
 *      Yes
 *
 * @retval  CPA_STATUS_SUCCESS        No error 
 *
 * @pre
 *      None.
 *
 * @post
 *      None.
 *
 *****************************************************************************/
ICP_DCC_EXPORT_FUNTION
CpaStatus 
icp_AsdCfgDccStop(
                           CpaInstanceHandle instanceHandle
                               );

/**
 *****************************************************************************
 * @ingroup icp_Dcc 
 *      Shutdown DCC Library
 * 
 * @description
 *      This function will shutdown the DCC Library. It will delete all the 
 *      created tables and free the allocated memory
 *
 * @context
 *      This function may sleep and therefore may not be called from interrupt
 *      context.
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is synchronous and blocking.
 *      
 * @reentrant
 *      No
 *
 * @threadSafe
 *      No
 *
 * @param  
 *  
 *     
 * @retval  CPA_STATUS_SUCCESS If successful
 * @retval  CPA_STATUS_RESOURCE Error related to system resource
 * @retval  CPA_STATUS_FAIL Unspecified error 
 *
 * @pre
 *      DCC should be initialized before calling this function
 *
 * @post
 *      None.
 *
 *****************************************************************************/
ICP_DCC_EXPORT_FUNTION
CpaStatus 
icp_AsdCfgDccShutdown(
                        CpaInstanceHandle instanceHandle
                                  );

#endif /*  ICP_DCC_CFG_H */

