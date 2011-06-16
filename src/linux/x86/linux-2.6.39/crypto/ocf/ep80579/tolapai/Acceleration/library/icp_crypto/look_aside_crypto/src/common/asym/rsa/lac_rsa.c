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
 * @file lac_rsa.c
 *
 * @defgroup LacRsa RSA
 *
 * @ingroup LacAsym
 *
 * This file implements functions for RSA.
 *
 * @lld_start
 *
 * @lld_overview
 * This is the RSA feature implementation.  It implements 3 RSA API
 * services: keygen, encrypt and decrypt.  Statistics are maintained for each
 * service.
 * For each service the parameters supplied by the client are checked, and then
 * input/output argument lists are constructed before calling the PKE QAT
 * Comms layer to create and send a request to the QAT.
 *
 * For encrypt operations there is only one type of public key so only one
 * QAT message can be constructed. however for keygena dn decrypt there are two
 * types of private key to be dealt with. This means that one of two messages
 * can be sent to the QAT. In these cases all common processing shall be done
 * first. Then we branch depending on the key type and perform key specific
 * processing.
 *
 * In all cases the service implementations are a straightforward
 * marshalling of client-supplied parameters for the QAT. i.e. there is
 * minimal logic handled by this component.  Buffer alignment is handled by
 * the PKE QAT Comms layer.
 *
 * The user's input buffers are checked for null params, correct length, msb
 * and lsb set where necessary. The following parameter checks based on the
 * standard are also performed for RSA
 *
 * RSA Keygen:
 *             Test: p and q must have the msb /lsb  set.
 *             Test: I will check that e > = 3 before sending to the QAT.
 *                      e is odd
 *             Test: Once the QAT has returned I can check that e < n
 *             Test: Once the QAT has returned I can check that the top bit of
 *                      n is set
 *
 * Encrypt:
 *             Test: The message parameter must satisfy 0 < m < =  n-1
 *             Test: The Modulus n must have the msb/lsb  set.
 *
 * Decrypt:
 *             Form 1:
 *                         Test: The ciphertext must satisfy 0 < c < =  n-1.
 *                         Test: Modulus n has msb /lsb  set.
 *             Form 2:
 *                         Test: p and q must have the msb /lsb  set.
 *                         Test: For prime tests I will just check that p and q
 *                                  are odd.
 *                         Test: 1 < Dp < p-1
 *                         Test: 1 < Dq < q-1
 *                         Test: 1 < =  qInv < p   (this could be 1)
 *
 * @lld_dependencies
 * - \ref LacAsymCommonQatComms "PKE QAT Comms" : For creating and sending
 * messages to the QAT
 * - \ref LacMem "Mem" : For memory allocation and freeing, and translating
 * between scalar and pointer types
 * - \ref LacAsymCommon "PKE Common" : For MMP generic sturcuts and param
 * checking macros
 * - OSAL : For atomics and logging
 *
 * @lld_initialisation
 * On initialization this component clears the stats.
 *
 * @lld_module_algorithms
 *
 * @lld_process_context
 *
 * @lld_end
 *****************************************************************************/

/*
********************************************************************************
* Include public/global header files
********************************************************************************
*/

#include "lac_rsa_p.h"
/* Include API files */
#include "cpa.h"
#include "cpa_cy_rsa.h"

/* Include Osal files */
#include "IxOsal.h"

/*Include LAC files */
#include "lac_mem.h"
#include "lac_common.h"
#include "lac_pke_utils.h"
#include "lac_pke_qat_comms.h"

/*
********************************************************************************
* Include private header files
********************************************************************************
*/


/*
********************************************************************************
* Static Variables
********************************************************************************
*/

/*
********************************************************************************
* Define static function definitions
********************************************************************************
*/

/*
********************************************************************************
* Global Variables
********************************************************************************
*/

/*
********************************************************************************
* Define public/global function definitions
********************************************************************************
*/

CpaBoolean
LacRsa_IsValidRsaSize(Cpa32U opSizeInBytes)
{
    Cpa32U opSizeInBits = LAC_NUM_BITS_IN_BYTE * opSizeInBytes;

    if ((LAC_1024_BITS != opSizeInBits)
        && (LAC_1536_BITS != opSizeInBits)
        && (LAC_2048_BITS != opSizeInBits)
        && (LAC_3072_BITS != opSizeInBits)
        && (LAC_4096_BITS != opSizeInBits))
    {
        LAC_INVALID_PARAM_LOG("Invalid operation size. Valid op sizes for "
            "RSA are 1024, 1536, 2048, 3072 and 4096 bits.");
        return CPA_FALSE;
    }

    return CPA_TRUE;
}

CpaStatus
LacRsa_Type2StdsCheck(CpaCyRsaPrivateKeyRep2 *pPrivateKeyRep2)
{
    /*
     * @note ideally we we check for type two keys that c < n. However
     * we cannot guarantee that the a type 2 key struct has the correct
     * values set for the type 1 fields (n is a type 1 field).
     */

    /* Standards based check: 1 < Dp < p-1 */
    if(LacPke_CompareZero(&(pPrivateKeyRep2->exponent1Dp), -1) <= 0)
    {
        LAC_INVALID_PARAM_LOG("exponent1Dp must be > 1");
        return CPA_STATUS_INVALID_PARAM;
    }
    if(LacPke_Compare(&(pPrivateKeyRep2->exponent1Dp), 0,
        &(pPrivateKeyRep2->prime1P), -1) >= 0)
    {
        LAC_INVALID_PARAM_LOG("exponent1Dp must be < prime1P - 1");
        return CPA_STATUS_INVALID_PARAM;
    }

    /* Standards based check: 1 < Dq < q-1 */
    if(LacPke_CompareZero(
        &(pPrivateKeyRep2->exponent2Dq), -1) <= 0)
    {
        LAC_INVALID_PARAM_LOG("exponent2Dq must be > 1");
        return CPA_STATUS_INVALID_PARAM;
    }
    if(LacPke_Compare(&(pPrivateKeyRep2->exponent2Dq), 0,
        &(pPrivateKeyRep2->prime2Q), -1) >= 0)
    {
        LAC_INVALID_PARAM_LOG("exponent2Dq must be < pPrime2Q - 1");
        return CPA_STATUS_INVALID_PARAM;
    }

    /* Standards based check: 1 <=  qInv < p */
    LAC_CHECK_NON_ZERO_PARAM(&(pPrivateKeyRep2->coefficientQInv));
    if(LacPke_Compare(&(pPrivateKeyRep2->coefficientQInv), 0,
        &(pPrivateKeyRep2->prime1P), 0) >= 0)
    {
        LAC_INVALID_PARAM_LOG("coefficientQInv must be < prime1P");
        return CPA_STATUS_INVALID_PARAM;
    }

    return CPA_STATUS_SUCCESS;
}


CpaStatus
LacRsa_CheckPrivateKeyParam(
    CpaCyRsaPrivateKey *pPrivateKey,
    Cpa32U opLenInBytes,
    CpaBoolean checkMsb,
    CpaBoolean checkLsb)
{
    Cpa32U lenInBytes = opLenInBytes;
    LAC_CHECK_NULL_PARAM(pPrivateKey);

    if (CPA_CY_RSA_VERSION_TWO_PRIME != pPrivateKey->version)
    {
        LAC_INVALID_PARAM_LOG("Invalid pPrivateKey->version");
        return CPA_STATUS_INVALID_PARAM;
    }

    switch(pPrivateKey->privateKeyRepType)
    {
        case CPA_CY_RSA_PRIVATE_KEY_REP_TYPE_1:
        {
            LAC_CHECK_FLAT_BUFFER(&(pPrivateKey->privateKeyRep1.modulusN));
            if (0 == lenInBytes)
            {
                lenInBytes
                    = pPrivateKey->privateKeyRep1.modulusN.dataLenInBytes;
            }
            LAC_CHECK_RSA_BUFFER_PARAM(&(pPrivateKey->privateKeyRep1.modulusN),
                CHECK_EQUALS, lenInBytes, checkMsb, checkLsb);

            LAC_CHECK_FLAT_BUFFER_PARAM(
                &(pPrivateKey->privateKeyRep1.privateExponentD),
                CHECK_LESS_EQUALS, lenInBytes, CPA_FALSE, CPA_FALSE);
        }
        break;

        case CPA_CY_RSA_PRIVATE_KEY_REP_TYPE_2:
        {
            Cpa32U type2BufSize = 0;

            LAC_CHECK_FLAT_BUFFER(&(pPrivateKey->privateKeyRep2.prime1P));
            LAC_CHECK_FLAT_BUFFER(&(pPrivateKey->privateKeyRep2.prime2Q));

            if (0 == lenInBytes)
            {
                lenInBytes
                    = pPrivateKey->privateKeyRep2.prime1P.dataLenInBytes
                    + pPrivateKey->privateKeyRep2.prime2Q.dataLenInBytes;
           }

           type2BufSize = LAC_RSA_TYPE_2_BUF_SIZE_GET(lenInBytes);

            /* parameters for type two decryption are half the operation size */
            LAC_CHECK_FLAT_BUFFER_PARAM(
                &(pPrivateKey->privateKeyRep2.prime1P),
                CHECK_EQUALS, type2BufSize, checkMsb, checkLsb);
            LAC_CHECK_FLAT_BUFFER_PARAM(
                &(pPrivateKey->privateKeyRep2.prime2Q),
                CHECK_EQUALS, type2BufSize, checkMsb, checkLsb);
            LAC_CHECK_FLAT_BUFFER_PARAM(
                &(pPrivateKey->privateKeyRep2.exponent1Dp),
                CHECK_LESS_EQUALS, type2BufSize, CPA_FALSE, CPA_FALSE);
            LAC_CHECK_FLAT_BUFFER_PARAM(
                &(pPrivateKey->privateKeyRep2.exponent2Dq),
                CHECK_LESS_EQUALS, type2BufSize, CPA_FALSE, CPA_FALSE);
            LAC_CHECK_FLAT_BUFFER_PARAM(
                &(pPrivateKey->privateKeyRep2.coefficientQInv),
                CHECK_LESS_EQUALS, type2BufSize, CPA_FALSE, CPA_FALSE);
        }
        break;

        default:
        {
            /* Invalid Key Type */
            LAC_INVALID_PARAM_LOG("Invalid pPrivateKey->privateKeyRepType");
            return CPA_STATUS_INVALID_PARAM;
        }
    }

    return CPA_STATUS_SUCCESS;
}

Cpa32U
LacRsa_GetPrivateKeyOpSize(const CpaCyRsaPrivateKey *pPrivateKey)
{
    Cpa32U sizeInBytes = 0;
    LAC_ASSERT_NOT_NULL(pPrivateKey);

    switch(pPrivateKey->privateKeyRepType)
    {
        case CPA_CY_RSA_PRIVATE_KEY_REP_TYPE_1:
        {
            sizeInBytes
                = pPrivateKey->privateKeyRep1.modulusN.dataLenInBytes;
        }
        break;

        case CPA_CY_RSA_PRIVATE_KEY_REP_TYPE_2:
        {
            sizeInBytes
                = pPrivateKey->privateKeyRep2.prime1P.dataLenInBytes
                + pPrivateKey->privateKeyRep2.prime2Q.dataLenInBytes;
        }
        break;

        default:
        {
            /* Invalid Key Type */
            LAC_LOG_ERROR("Invalid Private Key Type.");
        }
    }

    return sizeInBytes;
}
