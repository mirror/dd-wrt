/******************************************************************************
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
 *****************************************************************************/

/**
 ***************************************************************************
 * @file lac_sym_cb.h
 *
 * @defgroup LacSymCb Symmetric callback functions
 *
 * @ingroup LacSym
 *
 * Functions to assist with callback processing for the symmetric component
 ***************************************************************************/

#ifndef LAC_SYM_CB_H
#define LAC_SYM_CB_H

/**
 *****************************************************************************
 * @ingroup LacSym
 *      Dequeue pending requests
 * @description
 *      This function is called by a callback function of a blocking 
 *      operation (either a partial packet or a hash precompute operaion) 
 *      in softIRQ context. It dequeues requests for the following reasons:
 *          1. All pre-computes that happened when initialising a session 
 *             have completed. Dequeue any requests that were queued on the
 *             session while waiting for the precompute operations to complete.
 *          2. A partial packet request has completed. Dequeue any partials
 *             that were queued for this session while waiting for a previous 
 *             partial to complete.
 * 
 * @param[in] pSessionDesc  Pointer to the session descriptor
 * 
 * @return None 
 * 
 ****************************************************************************/
void
LacSymCb_PendingReqsDequeue(lac_session_desc_t *pSessionDesc);



/**
 *****************************************************************************
 * @ingroup LacSym
 *      Register symmetric callback funcion handlers
 * 
 * @description
 *      This function registers the symmetric callback handler functions with 
 *      the main symmetric callback handler function
 * 
 * @return None
 * 
 ****************************************************************************/
void
LacSymCb_CallbacksRegister(void);


#endif /* LAC_SYM_CB_H */
