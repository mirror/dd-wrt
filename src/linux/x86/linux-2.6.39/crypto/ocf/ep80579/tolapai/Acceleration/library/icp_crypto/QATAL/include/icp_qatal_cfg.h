
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
 * @file icp_qatal_cfg.h
 * 
 * @defgroup icp_QatalCfg QAT-AL ASD Interface.
 * 
 * @ingroup icp_Qatal
 *
 * @description
 *      This header file exports the Basic Init API for the QATAL subcomponent.
 *      It is used by ASD to initialise QATAL.
 *
 *****************************************************************************/

#ifndef ICP_QATAL_CFG_H
#define ICP_QATAL_CFG_H

#include "cpa.h"
#include "icp_asd_cfg.h"


/*********************************************************
 ******************* Internal Defines ********************
 *********************************************************/ 

/* required for poker test ... bit combinations */

#define MAX_BIT_COMBINATION (16)

#define BIT_COMBITATION_0 (0)
#define BIT_COMBITATION_1 (1)
#define BIT_COMBITATION_2 (2)
#define BIT_COMBITATION_3 (3)
#define BIT_COMBITATION_4 (4)
#define BIT_COMBITATION_5 (5)
#define BIT_COMBITATION_6 (6)
#define BIT_COMBITATION_7 (7)
#define BIT_COMBITATION_8 (8)
#define BIT_COMBITATION_9 (9)
#define BIT_COMBITATION_A (10)
#define BIT_COMBITATION_B (11)
#define BIT_COMBITATION_C (12)
#define BIT_COMBITATION_D (13)
#define BIT_COMBITATION_E (14)
#define BIT_COMBITATION_F (15)

#define NEXT_NIBBLE (4)

#define MAX_NO_OF_RUNS (34)

#define MAX_RUNS_PLUS_OVERFLOW (38)

typedef void * Qat_CallbackTag;

/**
 *****************************************************************************
 * @ingroup icp_QatalCfg
 *      QAT Access Layer Initialization function.
 * 
 * @description
 *      This function will initialize the QAT Access Layer component.
 *      It also performs the following operations:
 *       initialize system resources associated with the QATAL,
 *       bring QAT unit out of reset,
 *       initialize QAT init/admin rings,
 *       patch symbols into QAT-AE executable image      
 *      It MUST be called before any other QATAL function is called.
 *      This function SHOULD be called only once.
 *      Subsequent calls will only work after icp_QatalShutdown. 
 * 
 * @context
 *      This function may sleep, and so MUST NOT be called in interrupt
 *      context.
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      This function is synchronous and blocking.
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * @param instanceHandle      IN   Accelerator handle for which QATAL is initialized.
 * @param getCfgParamFunc  IN   ASD callback function to get system configuration parameters.
 *
 * @retval ICP_E_NO_ERROR       Function executed successfully.
 * @retval ICP_E_FAIL           Function failed.
 * @retval ICP_E_RESOURCE       Error related to system resources.
 */
 


/*
 * 
 * @pre
 *      None.
 * @post
 *      None
 * @note
 *      Note that this is a synchronous function and has no completion callback
 *      associated with it.
 * 
 * @see
 *      icp_AsdCfgQatalShutdown()
 *
 *****************************************************************************/
CpaStatus icp_AsdCfgQatalInit(CpaInstanceHandle instanceHandle, 
                                 icp_asd_cfg_param_get_cb_func_t getCfgParamFunc);

/**
 *****************************************************************************
 * @ingroup icp_QatalCfg
 *      QAT Access Layer Start function.
 * 
 * @description
 */


/*
 *      This function is called to put the QATAL into run-time state and
 *      start the QAT-AE.
 *      It performs the following operations:
 *       start the QAT-AE,
 */

 
/*
 *       send init msgs to QAT-AE for DBRG,Constants,Ring-info and MMP lib addr
 *       send INIT_FINAL msg to QAT-AE to put it into run-time state
 *       register liveness and version-info functions with DCC       
 *      It MUST be called after the icp_AsdCfgQatalInit function is called and
 *      before any other QATAL function is called.
 *      This function SHOULD be called only once.
 *      Subsequent calls will only work after icp_QatalShutdown. 
 * 
 * @context
 *      This function may sleep, and so MUST NOT be called in interrupt
 *      context.
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      This function is synchronous and blocking.
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * @param instanceHandle      IN   Accelerator handle for which QATAL is started.
 *
 * @retval ICP_E_NO_ERROR       Function executed successfully.
 * @retval ICP_E_FAIL           Function failed.
 * @retval ICP_E_RESOURCE       Error related to system resources.
 * 
 * @pre
 *      None.
 * @post
 *      None
 * @note
 *      Note that this is a synchronous function and has no completion callback
 *      associated with it.
 * 
 * @see
 *      icp_AsdCfgQatalInit()
 *      icp_AsdCfgQatalStop()
 *
 *****************************************************************************/
CpaStatus icp_AsdCfgQatalStart(CpaInstanceHandle instanceHandle);

/**
 *****************************************************************************
 * @ingroup icp_QatalCfg
 *      QAT Access Layer Stop function.
 * 
 * @description
 */

/*
 *      This function is called to put the QATAL into stopped state and
 *      clear inflight requests in the QAT-AE. The client MUST ensure that
 *      all outstanding operations have completed before calling this function.
 *      This function performs the following operations:
 *       unregister liveness and version-info functions in the DCC,
 *       clear in-flight requests in the QAT-AE,
 *       put the QAT-AL into stopped state
 *      It MUST be called after the icp_AsdCfgQatalStart function is called
 *      and before the icp_AsdCfgQatalShutdown is called.
 *      This function SHOULD be called only once.
 *      Subsequent calls will only work after icp_QatalShutdown. 
 * 
 * @context
 *      This function may sleep, and so MUST NOT be called in interrupt
 *      context.
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      This function is synchronous and blocking.
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * @param instanceHandle      IN   Accelerator handle for which QATAL is stopped.
 *
 * @retval ICP_E_NO_ERROR       Function executed successfully.
 * @retval ICP_E_FAIL           Function failed.
 * @retval ICP_E_RESOURCE       Error related to system resources.
 * 
 * @pre
 *      None.
 * @post
 *      None
 * @note
 *      Note that this is a synchronous function and has no completion callback
 *      associated with it.
 * 
 * @see
 *      icp_AsdCfgQatalInit()
 *      icp_AsdCfgQatalStop()
 *
 *****************************************************************************/
CpaStatus icp_AsdCfgQatalStop(CpaInstanceHandle instanceHandle);

/**
 *****************************************************************************
 * @ingroup icp_QatalCfg
 *      QAT Access Layer Shutdown function.
 * 
 * @description
 *      This function will shutdown the QAT Access Layer component, free
 *      all system resources associated with it and stop the QAT-AE
 *      The client MUST ensure that the icp_AsdCfgQatalStop function has been
 *      called before calling this function.
 *      This function performs the following operations:
 *       stop the QAT-AE,
 *       destroy the QAT init/admin rings,
 *       clean up all other system resources associated with the QATAL
 *      It MUST be called after the icp_AsdCfgQatalStop function is called.
 *      This function SHOULD be called only once.
 *      Subsequent calls will only work after icp_QatalInit & start. 
 * 
 * @context
 *      This function may sleep, and so MUST NOT be called in interrupt
 *      context.
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      This function is synchronous and blocking.
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * @param instanceHandle       IN  Accelerator handle for which QATAL is shutdown.
 *
 * @retval ICP_E_NO_ERROR       Function executed successfully.
 * @retval ICP_E_FAIL           Function failed.
 * 
 * @pre
 *      The component has been initialized via icp_AsdCfgQatalInit
 * @post
 *      None
 * @note
 *      Note that this is a synchronous function and has no completion callback
 *      associated with it.
 * 
 * @see
 *      icp_AsdCfgQatalInit()
 *
 *****************************************************************************/
CpaStatus icp_AsdCfgQatalShutdown(CpaInstanceHandle instanceHandle);

  


/**
 *****************************************************************************
 * @ingroup icp_QatalCfg
 *      QAT Access Layer - Return pointer to Entropy block.
 * 
 * @description
 *
 *
 *      This function will return the pointer to the entropy block
 * 
 * @context
 *      This function may sleep, and so MUST NOT be called in interrupt
 *      context.
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      This function is synchronous and blocking.
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * @param instanceHandle       IN  pointer to store address of entrophy block
 *
 * @retval ICP_E_NO_ERROR       Function executed successfully.
 * @retval ICP_E_FAIL           Function failed.
 * 
 * @pre
 *      None
 * @post
 *      None
 * @note
 *      Note that this is a synchronous function and has no completion callback
 *      associated with it.
 * 
 *
 *****************************************************************************/
CpaStatus Qatal_GetRANDDataBlockPtr(Cpa8U **ppRANDDataBlock);


/**
 *****************************************************************************
 * @ingroup icp_QatalCfg
 *      QAT Access Layer - Store pointer given as pointer to Entropy block.
 * 
 * @description
 *      This function will store the pointer given as a pointer to the entrophy block
 * 
 * @context
 *      This function may sleep, and so MUST NOT be called in interrupt
 *      context.
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      This function is synchronous and blocking.
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * @param instanceHandle       IN  pointer to entrophy block
 *
 * @retval ICP_E_NO_ERROR       Function executed successfully.
 * @retval ICP_E_FAIL           Function failed.
 * 
 * @pre
 *      None
 * @post
 *      None
 * @note
 *      Note that this is a synchronous function and has no completion callback
 *      associated with it.
 * 
 *
 *****************************************************************************/
CpaStatus Qatal_PutRANDDataBlockPtr(Cpa8U **ppRANDDataBlock);


#endif /*  ICP_QATAL_CFG_H */
