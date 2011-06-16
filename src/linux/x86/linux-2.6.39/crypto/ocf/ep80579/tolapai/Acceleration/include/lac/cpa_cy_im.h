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
 * @file cpa_cy_im.h
 *
 * @defgroup cpaCyInstMaint Crypto Instance Maintainence API.
 *
 * @ingroup cpaCy
 *
 * @description
 *      These functions specify the Instance Maintainence API for available
 *      Crypto Instances. It's expected that these functions will only be
 *      called via a single system maintenance entity, rather than individual
 *      clients
 *
 *****************************************************************************/

#ifndef CPA_CY_IM_H_
#define CPA_CY_IM_H_

#include "cpa_cy_common.h"

/**
 *****************************************************************************
 * @file cpa_cy_im.h
 * @ingroup cpaCyInstMaint
 *      Cryptographic Component Initialization and Start function.
 * 
 * @description
 *      This function will initialize and start the Cryptographic component.
 *      It MUST be called before any other crypto function is called. This
 *      function SHOULD be called only once (either for the very first time,
 *      or after an cpaCyStopInstance call which succeeded) per instance.
 *      Subsequent calls will have no effect. 
 * 
 * @context
 *      This function may sleep, and  MUST NOT be called in interrupt context.
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
 * @param[out] instanceHandle       Handle to an instance of this API to be 
 *                                  initialized.
 *
 * @retval CPA_STATUS_SUCCESS       Function executed successfully.
 * @retval CPA_STATUS_FAIL          Function failed. Suggested course of action
 *                                  is to shutdown and restart. 
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
 *      cpaCyStopInstance()
 *
 *****************************************************************************/
CpaStatus
cpaCyStartInstance(CpaInstanceHandle instanceHandle);

/**
 *****************************************************************************
 * @file cpa_cy_im.h
 * @ingroup cpaCyInstMaint
 *      Cryptographic Component Stop function.
 * 
 * @description
 *      This function will stop the Cryptographic component and free
 *      all system resources associated with it. The client MUST ensure that
 *      all outstanding operations have completed before calling this function.
 *      The recommended approach to ensure this is to deregister all session or
 *      callback handles before calling this function. If outstanding 
 *      operations still exist when this function is invoked, the callback
 *      function for each of those operations will NOT be invoked and the
 *      shutdown will continue.  If the component is to be restarted, then a 
 *      call to cpaCyStartInstance is required. 
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
 * @param[in] instanceHandle        Handle to an instance of this API to be
 *                                  shutdown.
 *
 * @retval CPA_STATUS_SUCCESS       Function executed successfully.
 * @retval CPA_STATUS_FAIL          Function failed. Suggested course of action
 *                                  is to ensure requests are not still being 
 *                                  submitted and that all sessions are 
 *                                  deregistered. If this does not help, then 
 *                                  forcefully remove the component from the 
 *                                  system.
 * 
 * @pre
 *      The component has been initialized via cpaCyStartInstance
 * @post
 *      None
 * @note
 *      Note that this is a synchronous function and has no completion callback
 *      associated with it.
 * 
 * @see
 *      cpaCyStartInstance()
 *
 *****************************************************************************/
CpaStatus
cpaCyStopInstance(CpaInstanceHandle instanceHandle);

#endif /*CPA_CY_IM_H_*/
