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
 * @file lac_sym_key.h
 *
 * @defgroup    LacSymKey  Key Generation
 *
 * @ingroup     LacSym
 *
 * @lld_start
 *
 * @lld_overview
 * 
 * Key generation component is reponsible for SSL, TLS & MGF operations. All 
 * memory required for the keygen operations is got from the keygen cookie 
 * structure which is carved up as required. 
 * 
 * For SSL the QAT accelerates the nested hash function with MD5 as the 
 * outer hash and SHA1 as the inner hash.
 * 
 * Refer to sections in draft-freier-ssl-version3-02.txt: 
 *      6.1 Asymmetric cryptographic computations - This refers to coverting
 *          the pre master secret to the master secret. 
 *      6.2.2 Converting the master secret into keys and MAC secrets - Using
 *          the master secret to generate the key material.
 * 
 * For TLS the QAT accelerates the PRF function as described in 
 * rfc4346 - TLS version 1.1 (this obsoletes rfc2246 - TLS version 1.0)
 *      5. HMAC and the pseudorandom function - For the TLS PRF and getting 
 *         S1 and S2 from the secret.
 *      6.3. Key calculation - For how the key material is generated
 *      7.4.9. Finished - How the finished message uses the TLS PRF
 *      8.1. Computing the master secret 
 * 
 * 
 * @lld_dependencies
 * \ref LacSymQatHash: for building up hash content descriptor
 * \ref LacMem: for virt to phys coversions
 * 
 * @lld_initialisation
 * The reponse handler is registered with Symmetric. The Maximum SSL is
 * allocated. A structure is allocated containing all the TLS lables that
 * are supported. On shutdown the memory for these structures are freed. 
 * 
 * @lld_module_algorithms
 * @lld_process_context
 *
 * @lld_end
 *
 *
 *****************************************************************************/
#ifndef LAC_SYM_KEY_H_
#define LAC_SYM_KEY_H_

/**
 ******************************************************************************
 * @ingroup LacSymKey
 *      This function prints the stats to standard out. 
 *
 * @retval CPA_STATUS_SUCCESS   Status Success
 * @retval CPA_STATUS_FAIL      General failure
 * 
 *****************************************************************************/
void 
LacKeygen_StatsShow(void);

#endif
