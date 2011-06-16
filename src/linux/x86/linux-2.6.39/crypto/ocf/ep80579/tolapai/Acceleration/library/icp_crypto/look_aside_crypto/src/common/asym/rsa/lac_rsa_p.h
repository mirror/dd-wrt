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
 *******************************************************************************
 * @file lac_rsa_p.h
 *
 * @ingroup LacRsa
 *
 * This file defines private data for RSA operations.
 * 
 * @note
 * RSA operations may be called in Asynchronous or Synchronous modes.
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

 ******************************************************************************/

/******************************************************************************/

#ifndef _LAC_RSA_P_H_
#define _LAC_RSA_P_H_

/*
********************************************************************************
* Include public/global header files
********************************************************************************
*/

#include "cpa.h"
#include "cpa_cy_rsa.h"

/*
********************************************************************************
* Include private header files
********************************************************************************
*/

/* Calculate the buffer size for type 2 key buffers. This is half the operation
 * size */
#define LAC_RSA_TYPE_2_BUF_SIZE_GET(size) ((size) >> 1)

/**
 *******************************************************************************
 * @ingroup LacRsa
 *      Checks that a flat buffer is valid for RSA operations .
 *
 * @description
 *      This macro does the standard buffer checks but also can check that
 * one of the two most significant bits are set in the buffer.
 *
 * @param[in] pBuffer           pointer to the flat buffer to check
 * @param[in] checkType         type of check performed on the buffer's length
 * @param[in] lenInBytes        the required length
 * @param[in] checkMsb          flag to indicate whether (true) or not to
 *                              check that one of the two most significant
 *                              bits are set.
 * @param[in] checkLsb          flag to indicate whether (true) or not (false)
 *                              to check that the LSB is set
 *
 * @return CPA_STATUS_INVALID_PARAM  Null, Length, LSB and/or MSB checks failed
 * @return void                 All checks passed
 ******************************************************************************/
#define LAC_CHECK_RSA_BUFFER_PARAM(                                     \
    pBuffer, checkType, lenInBytes, checkMsb, checkLsb)                 \
do {                                                                    \
    LAC_CHECK_FLAT_BUFFER_PARAM(                                        \
        (pBuffer), checkType, lenInBytes, CPA_FALSE, checkLsb);         \
                                                                        \
    if (checkMsb && !((pBuffer)->pData[0] & 0xC0))                      \
    {                                                                   \
        LAC_INVALID_PARAM_LOG(#pBuffer                                  \
        " must have one of the 2 most significant bits set");           \
        return CPA_STATUS_INVALID_PARAM;                                \
    }                                                                   \
                                                                        \
} while (0)

/**
 *******************************************************************************
 * @ingroup LacRsa
 *      This function checks that the given RSA size is valid
 *
 * @description
 *      This function checks that the given RSA size is valid. Valid sizes
 * are 1024, 2048 and 4096 bits.
 *
 * @param[in] opSizeInBytes     the length to be checked
  *
 * @retval CPA_TRUE
 * @retval CPA_FALSE
 ******************************************************************************/
CpaBoolean
LacRsa_IsValidRsaSize(Cpa32U opSizeInBytes);

/**
 *******************************************************************************
 * @ingroup LacRsa
 *      This function checks that the given RSA private key is valid
 *
 * @description
 *      This function checks that the given RSA private key is valid
 *
 * @param[in] pPrivateKey   the private key to be checked
 * @param[in] opLenInBytes  the required length - if set to 0 then the
 *                          required length is calculated from the private key.
 * @param[in] checkMsb      flag to indicate whether (true) or not (false)
 *                          to check that the appropriate MSB's are set
 * @param[in] checkLsb      flag to indicate whether (true) or not (false)
 *                          to check that the appropriate LSB's are set
 *
 * @retval CPA_STATUS_SUCCESS
 * @retval CPA_STATUS_INVALID_PARAM
 ******************************************************************************/
CpaStatus
LacRsa_CheckPrivateKeyParam(
    CpaCyRsaPrivateKey *pPrivateKey,
    Cpa32U opLenInBytes,
    CpaBoolean checkMsb,
    CpaBoolean checkLsb);

/**
 *******************************************************************************
 * @ingroup LacRsa
 *      This function returns the size of the operation based on this private
 * key in bytes.
 *
 * @description
 *      This function returns the size of the operation based on this private
 * key in bytes. This is the length of the modulus for type 1 keys and the
 * length of p + the length of q for the type 2 keys.
 *
 * @param[in] pPrivateKey   the RSA private key - either type 1 or type 2
 *
 * @retval the size of the operation based on this private key in bytes.
 ******************************************************************************/
Cpa32U
LacRsa_GetPrivateKeyOpSize(
    const CpaCyRsaPrivateKey *pPrivateKey);

/*
 * Performs standards based checks for type2 rsa keys
 */
CpaStatus
LacRsa_Type2StdsCheck(CpaCyRsaPrivateKeyRep2 *pPrivateKeyRep2);

#endif /* _LAC_RSA_P_H_ */
