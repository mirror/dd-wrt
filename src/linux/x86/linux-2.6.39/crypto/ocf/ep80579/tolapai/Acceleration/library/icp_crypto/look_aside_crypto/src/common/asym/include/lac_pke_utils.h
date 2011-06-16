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
 ***************************************************************************
 * @file lac_pke_utils.h
 *
 * @defgroup LacAsymComonUtils Lac Pke Utils
 *
 * @ingroup LacAsymCommon
 *
 * utils that are PKE specific
 *
 ******************************************************************************/

/******************************************************************************/

#ifndef _LAC_PKE_UTILS_H_
#define _LAC_PKE_UTILS_H_

/*
********************************************************************************
* Include public/global header files
********************************************************************************
*/

#include "cpa.h"
#include "cpa_cy_common.h"
#include "IxOsal.h"
#include "lac_mem.h"

/*
********************************************************************************
* Include private header files
********************************************************************************
*/

/******************************************************************************/

/*
*******************************************************************************
* LAC PKE Operation sizes
*******************************************************************************
*/

#define LAC_128_BITS (128)
#define LAC_160_BITS (160)
#define LAC_192_BITS (192)
#define LAC_256_BITS (256)
#define LAC_384_BITS (384)
#define LAC_512_BITS (512)
#define LAC_576_BITS (576)
#define LAC_640_BITS (640)
#define LAC_706_BITS (706)
#define LAC_768_BITS (768)
#define LAC_832_BITS (832)
#define LAC_896_BITS (896)
#define LAC_960_BITS (960)
#define LAC_1024_BITS (1024)
#define LAC_1536_BITS (1536)
#define LAC_2048_BITS (2048)
#define LAC_2560_BITS (2560)
#define LAC_3584_BITS (3584)
#define LAC_3072_BITS (3072)
#define LAC_4096_BITS (4096)

#define LAC_MAX_OP_SIZE_IN_BITS LAC_4096_BITS
/**< @ingroup LacAsymCommon
 * Maximum PKE operation in bits */

#define LAC_PKE_BUFFERS_PER_OP_MAX 8
/**< @ingroup LacAsymCommon
 * LAC PKE Max number of Alignment buffers and directions */

#define LAC_PKE_MAX_OUTSTANDING_REQUESTS 2
/**< @ingroup LacAsymCommon
 * A buffer is needed for each alignment required and for each direction */

/**
 *******************************************************************************
 * @ingroup LacAsymComonUtils
 *      This macro calculates the length of the given array
 *
 * @param[in] array         the array whose length we are calculating
 *
 ******************************************************************************/
#define LAC_ARRAY_LEN(array) (sizeof(array)/sizeof(array[0]))

/**
 *******************************************************************************
 * @ingroup LacAsymComonUtils
 *      This macro calculates the position of the given member in a struct
 *      It returns the number of bytes that the member field is offset from the
 *      begining of the struct structType.
 *
 * @param[in] structType        the struct
 * @param[in] member            the member of the given struct
 *
 ******************************************************************************/
#define LAC_IDX_OF(structType, member) \
    (offsetof(structType, member) / sizeof(((structType *) 0)->member))

/**
 *****************************************************************************
 * @ingroup LacAsymComonUtils
 *
 * @description
 *      This enum lists the types of param check that can be performed on the #
 * size of a buffer.
 *
 *****************************************************************************/
typedef enum
{
    CHECK_EQUALS = 0,
    CHECK_LESS_EQUALS,
    CHECK_GREATER_EQUALS
} lac_pke_size_check_type_t;

 /**
 *******************************************************************************
 * @ingroup LacAsymComonUtils
 *      This macro checks if the flat buffer data is ODD
 *
 * @param[in] pBuffer           pointer to the flat buffer to check
 *
 ******************************************************************************/
#define LAC_CHECK_ODD_PARAM(pBuffer)                                           \
    ((pBuffer)->pData[(pBuffer)->dataLenInBytes - 1] & 0x01)
 
 /**
 *******************************************************************************
 * @ingroup LacAsymComonUtils
 *      This macro checks if the flat buffer data is EVEN
 *
 * @param[in] pBuff             pointer to the flat buffer to check
 *
 ******************************************************************************/
#define LAC_CHECK_EVEN_PARAM(pBuff)                                            \
    (!LAC_CHECK_ODD_PARAM(pBuff))   

/**
 *******************************************************************************
 * @ingroup LacAsymComonUtils
 *      Checks that a flat buffer's size is correct.
 *
 * @description
 *      Checks that a flat buffer's size is correct. The check type specifies
 * the type of check to be performed e.g. less than or equals, equals etc.
 * If any check fails an error is logged and the calling function is
 * returned out of with an error.
 *
 * @param[in] pBuffer           pointer to the flat buffer to check
 * @param[in] checkType         type of check performed on the buffer's length
 * @param[in] lenInBytes        the required length
 *
 * @return CPA_STATUS_INVALID_PARAM  Length check failed
 * @return void                 All checks passed
 ******************************************************************************/
#define LAC_CHECK_SIZE(pBuffer, checkType, lenInBytes)                         \
do {                                                                           \
    if ((pBuffer)->dataLenInBytes == 0)                                        \
    {                                                                          \
        LAC_INVALID_PARAM_LOG(#pBuffer " has incorrect length of zero");       \
        return CPA_STATUS_INVALID_PARAM;                                       \
    }                                                                          \
                                                                               \
    if ((CHECK_EQUALS == checkType)                                            \
        && ((pBuffer)->dataLenInBytes != lenInBytes))                          \
    {                                                                          \
        LAC_INVALID_PARAM_LOG(#pBuffer " has incorrect length");               \
        return CPA_STATUS_INVALID_PARAM;                                       \
    }                                                                          \
    else if ((CHECK_LESS_EQUALS == checkType)                                  \
        && ((pBuffer)->dataLenInBytes > lenInBytes))                           \
    {                                                                          \
        LAC_INVALID_PARAM_LOG(#pBuffer " has incorrect length");               \
        return CPA_STATUS_INVALID_PARAM;                                       \
    }                                                                          \
    else if ((CHECK_GREATER_EQUALS == checkType)                               \
        && ((pBuffer)->dataLenInBytes < lenInBytes))                           \
    {                                                                          \
        LAC_INVALID_PARAM_LOG(#pBuffer " has incorrect length");               \
        return CPA_STATUS_INVALID_PARAM;                                       \
    }                                                                          \
                                                                               \
} while (0)

/**
 *******************************************************************************
 * @ingroup LacAsymComonUtils
 *      Checks that a flat buffer is valid.
 *
 * @description
 *      This macro checks that a flat buffer is not null, that its length is
 * correct, and optionally checks that the MSB and/or LSB is set.
 * If any check fails an error is logged and the calling function is
 * returned out of with an error.
 *
 * @param[in] pBuffer           pointer to the flat buffer to check
 * @param[in] checkType         type of check performed on the buffer's length
 * @param[in] lenInBytes        the required length
 * @param[in] checkMsb          flag to indicate whether (true) or not (false)
 *                              to check that the MSB is set
 * @param[in] checkLsb          flag to indicate whether (true) or not (false)
 *                              to check that the LSB is set
 *
 * @return CPA_STATUS_INVALID_PARAM  Null, Length, LSB and/or MSB checks failed
 * @return void                 All checks passed
 ******************************************************************************/
#define LAC_CHECK_FLAT_BUFFER_PARAM(                                    \
    pBuffer, checkType, lenInBytes, checkMsb, checkLsb)                 \
do {                                                                    \
    LAC_CHECK_FLAT_BUFFER(pBuffer);                                     \
    LAC_CHECK_SIZE((pBuffer), checkType, lenInBytes);                   \
                                                                        \
    if (checkMsb && !((pBuffer)->pData[0] & 0x80))                      \
    {                                                                   \
        LAC_INVALID_PARAM_LOG(#pBuffer " doesn't have MSB set");        \
        return CPA_STATUS_INVALID_PARAM;                                \
    }                                                                   \
                                                                        \
    if (checkLsb && !((pBuffer)->pData[(pBuffer)->dataLenInBytes - 1] & 0x01))\
    {                                                                   \
        LAC_INVALID_PARAM_LOG(#pBuffer " doesn't have LSB set");        \
        return CPA_STATUS_INVALID_PARAM;                                \
    }                                                                   \
} while (0)

/* define MSB/LSB check flags */
#define LAC_CHECK_MSB_YES               1
#define LAC_CHECK_MSB_NO                0
#define LAC_CHECK_LSB_YES               1
#define LAC_CHECK_LSB_NO                0

/**
 *******************************************************************************
 * @ingroup LacAsymComonUtils
 *      Checks that a flat buffer is non zero.
 *
 * @description
 *      This macro checks that a flat buffer's data is non zero.
 *
 * @param[in] pBuffer           pointer to the flat buffer to check
 *
 * @return CPA_STATUS_INVALID_PARAM  Non zero check failed
 * @return void                 All checks passed
 ******************************************************************************/
#define LAC_CHECK_NON_ZERO_PARAM(pBuffer)                               \
do {                                                                    \
    if (0 == LacPke_CompareZero((pBuffer), 0))                          \
    {                                                                   \
        LAC_INVALID_PARAM_LOG(#pBuffer " cannot be zero");              \
        return CPA_STATUS_INVALID_PARAM;                                \
    }                                                                   \
} while (0)

/**
 *******************************************************************************
 * @ingroup LacAsymComonUtils
 *      Checks that a flat buffer data size is non zero.
 *
 * @description
 *      This macro checks that a flat buffer's data size is non zero.
 *
 * @param[in] pBuffer           pointer to the flat buffer to check
 *
 * @return CPA_STATUS_INVALID_PARAM  Non zero check failed
 * @return void                 Check passed
 ******************************************************************************/
#define LAC_CHECK_ZERO_SIZE(pBuffer)                                           \
do {                                                                           \
    if ((pBuffer)->dataLenInBytes == 0)                                        \
    {                                                                          \
        LAC_INVALID_PARAM_LOG(#pBuffer " has incorrect length of zero");       \
        return CPA_STATUS_INVALID_PARAM;                                       \
    }                                                                          \
} while (0)

/**
*******************************************************************************
 * @ingroup LacCommon
 *      Check to see if the LAC PKE component has been initialised
 *
 * @description
 *      This function checks the state of LAC PKE to see if it has being
 *      initialised
 *
 * @retval CPA_TRUE   Initialised
 * @retval CPA_FALSE  Not Initialised.
 *
 *****************************************************************************/
CpaBoolean
LacPke_IsInitialised(void);

/**
 *******************************************************************************
 * @ingroup LacAsymComonUtils
 *      This macro checks if PKE is initilised. An error message is Logged if it
 *      has not been initialised.
 *
 * @return CPA_STATUS_FAIL  Lac PKE has not been initialised
 * @return void             Lac PKE has been successfully initialised
 ******************************************************************************/
#define LAC_PKE_INITIALISED_CHECK()                                     \
do {                                                                    \
    if ( CPA_TRUE != LacPke_IsInitialised() )                           \
    {                                                                   \
        LAC_LOG_ERROR("PKE API called before PKE was initialised");     \
        return CPA_STATUS_FAIL;                                         \
    }                                                                   \
} while(0)

/**
 *******************************************************************************
 * @ingroup LacAsymComonUtils
 *      Compare one large integer (+- delta) to another (+- delta)
 *
 * @description
 *      This function compares two large integers. The deltas are added to
 * the large integers and may be negative. The length of the large integers
 * must be less than LAC_MAX_OP_SIZE_IN_BITS
 *
 * @param[in] pFlatBufferA      first integer
 * @param[in] deltaA            delta
 * @param[in] pFlatBufferB      second integer
 * @param[in] deltaB            delta
 *
 * @retval less than 0 if integerA + deltaA < integerB + deltaB
 * @retval 0 if integerA + deltaA = integerB + deltaB
 * @retval greater than 0 if integerA + deltaA > integerB + deltaB
 *
 ******************************************************************************/
Cpa32S
LacPke_Compare (
    const CpaFlatBuffer* pFlatBufferA,
    const Cpa32S deltaA,
    const CpaFlatBuffer* pFlatBufferB,
    const Cpa32S deltaB);

/**
 *******************************************************************************
 * @ingroup LacAsymComonUtils
 *      Compare one large integer (+- delta) to 0
 *
 * @description
 *      This function compares a large integer to 0. The delta is added to
 * the large integer and may be negative. The length of the large integers
 * must be less than LAC_MAX_OP_SIZE_IN_BITS
 *
 * @param[in] pFlatBuffer       buffer containing large integer
 * @param[in] delta             delta
 *
 * @retval less than 0 if integerA + deltaA < 0
 * @retval 0 if integerA + deltaA = 0
 * @retval greater than 0 if integerA + deltaA > 0
 *
 ******************************************************************************/
Cpa32S
LacPke_CompareZero (
    const CpaFlatBuffer* pFlatBuffer,
    const Cpa32S delta);

/**
 *******************************************************************************
 * @ingroup LacAsymComonUtils
 *      Initialise the PKE component
 *
 * @description
 *      Initialise the PKE component. This will initialise all the PKE features.
 * It will verify that the correct PKE firmware version was loaded.
 *
 * @retval CPA_STATUS_SUCCESS successful initialisation
 * @retval CPA_STATUS_FAIL failed initialisation
 ******************************************************************************/
CpaStatus
LacPke_Init(void);

/**
 *******************************************************************************
 * @ingroup LacAsymComonUtils
 *      Shutdown the PKE component
 *
 * @description
 *      Shutdown the PKE component.
 ******************************************************************************/
void
LacPke_Shutdown(void);

#endif /* _LAC_PKE_UTILS_H_ */
