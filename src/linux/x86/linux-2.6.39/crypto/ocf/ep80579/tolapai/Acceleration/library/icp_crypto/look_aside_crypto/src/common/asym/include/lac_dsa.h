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
 *****************************************************************************
 * @file lac_dsa.h
 *
 * @defgroup Lac_Dsa     DSA
 *
 * @ingroup LacAsym
 *
 * Interfaces exposed by the DSA component
 * 
 * @lld_start
 *
 * @lld_overview
 * This is the DSA feature implementation.  It implements 7 DSA API services:
 * parameter generation (p, g, y), signature generation (r, s, rs), and
 * signature verification.  Statistics are maintained for each service.  For
 * each service the parameters supplied by the client are checked, and then
 * input/output argument lists are constructed before calling the PKE QAT
 * Comms layer to create and send a request to the QAT.
 *
 * For DSA P Parameter Generation two inputs are required by the QAT which are
 * handled internally so the client is not aware of them.  They are products
 * of small primes, used by the QAT to perform a GCD (or alternatively
 * Pollard-Rho) test on the generated P parameter as an initial (non-robust,
 * but fast) primality test.  These two inputs are allocated and managed
 * internally by this component.
 * 
 * For Verification service the output is the result of the verification
 * returned by the QAT in the form of pass/fail status. The status is
 * returned to the caller.
 *
 * In all other cases the service implementations are a straightforward
 * marshalling of client-supplied parameters for the QAT.  I.e. there is
 * minimal logic handled by this component.  Buffer alignment, and padding up
 * to a whole number of quadwords, is handled by the PKE QAT Comms layer.
 *
 * @lld_initialisation
 * On initialization this component allocates the two product-of-small-primes
 * parameters (psp1 and psp2) for the DSA P Parameter Generation operation.
 * It also clears the stats.
 *
 * @note
 * The DSA feature may be called in Asynchronous or Synchronous modes.
 * In Asynchronous mode the user supplies a Callback function to the API.
 * Control returns to the client after the message has been sent to the QAT and
 * the Callback gets invoked when the QAT completes the operation. There is NO
 * BLOCKING. This mode is preferred for maximum performance.
 * In Synchronous mode the client supplies no Callback function pointer (NULL)
 * and the point of execution is placed on a wait-queue internally, and this is
 * de-queued once the QAT completes the operation. Hence, Synchronous mode is
 * BLOCKING. So avoid using in an interrupt context. To achieve maximum
 * performance from the API Asynchronous mode is preferred. 
 * 
 * @lld_module_algorithms
 *
 * @lld_process_context
 *
 * @lld_end
 * 
 *****************************************************************************/

/*****************************************************************************/

#ifndef LAC_DSA_H
#define LAC_DSA_H

/* 
******************************************************************************
* Include public/global header files 
******************************************************************************
*/ 

#include "cpa.h"

/* 
*******************************************************************************
* Include private header files 
*******************************************************************************
*/ 

/**
 *******************************************************************************
 * @ingroup Lac_Dsa 
 *      print the DSA stats to standard output
 *
 * @description
 *      For each engine this function copies the stats using the function
 *      cpaCyDsaQueryStats. It then prints contents of this structure to
 *      standard output
 *
 * @see cpaCyDsaQueryStats()
 *
 *****************************************************************************/
void
LacDsa_StatsShow(void);

#endif /* LAC_DSA_H */
