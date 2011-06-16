/*
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
 */

/* --- (Automatically generated (build v. 2.33), do not modify manually) --- */


/**
 * @file icp_qat_fw_mmp.h
 * @defgroup icp_qat_fw_mmp ICP QAT FW MMP Processing Definitions
 * @ingroup icp_qat_fw
 * $Revision: 0.1 $
 * @brief
 *      This file documents the external interfaces that the QAT FW running
 *      on the QAT Acceleration Engine provides to clients wanting to
 *      accelerate crypto assymetric applications
 */


#ifndef __ICP_QAT_FW_MMP__
#define __ICP_QAT_FW_MMP__


/**************************************************************************
 * Include local header files
 **************************************************************************
 */


#include "icp_qat_fw.h"


/**************************************************************************
 * Local constants
 **************************************************************************
 */
#define ICP_QAT_FW_PKE_INPUT_COUNT_MAX      7
/**< @ingroup icp_qat_fw_pke
 * Maximum number of input paramaters in all PKE request */
#define ICP_QAT_FW_PKE_OUTPUT_COUNT_MAX     5
/**< @ingroup icp_qat_fw_pke
 * Maximum number of output paramaters in all PKE request */




/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular exponentiation base 2 for 160-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_MODEXP_G2_160.
 */
typedef struct icp_qat_fw_mmp_modexp_g2_160_input_s
{
    uint64_t e; /**< exponent &gt; 0 and &lt; 2^160 (3 qwords)*/
    uint64_t m; /**< modulus &ge; 2^159 and &lt; 2^160 (3 qwords)*/
} icp_qat_fw_mmp_modexp_g2_160_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular exponentiation for 160-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_MODEXP_160.
 */
typedef struct icp_qat_fw_mmp_modexp_160_input_s
{
    uint64_t g; /**< base &ge; 0 and &lt; 2^160 (3 qwords)*/
    uint64_t e; /**< exponent &ge; 0 and &lt; 2^160 (3 qwords)*/
    uint64_t m; /**< modulus &ge; 2^159 and &lt; 2^160 (3 qwords)*/
} icp_qat_fw_mmp_modexp_160_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular exponentiation base 2 for 512-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_MODEXP_G2_512.
 */
typedef struct icp_qat_fw_mmp_modexp_g2_512_input_s
{
    uint64_t e; /**< exponent &gt; 0 and &lt; 2^512 (8 qwords)*/
    uint64_t m; /**< modulus  &ge; 2^511 and &lt; 2^512 (8 qwords)*/
} icp_qat_fw_mmp_modexp_g2_512_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular exponentiation for 512-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_MODEXP_512.
 */
typedef struct icp_qat_fw_mmp_modexp_512_input_s
{
    uint64_t g; /**< base &ge; 0 and &lt; 2^512 (8 qwords)*/
    uint64_t e; /**< exponent &gt; 0 and &lt; 2^512 (8 qwords)*/
    uint64_t m; /**< modulus   &ge; 2^511 and &lt; 2^512 (8 qwords)*/
} icp_qat_fw_mmp_modexp_512_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular exponentiation base 2 for 768-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_MODEXP_G2_768.
 */
typedef struct icp_qat_fw_mmp_modexp_g2_768_input_s
{
    uint64_t e; /**< exponent &gt; 0 and &lt; 2^768 (12 qwords)*/
    uint64_t m; /**< modulus   &ge; 2^767 and &lt; 2^768 (12 qwords)*/
} icp_qat_fw_mmp_modexp_g2_768_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular exponentiation for 768-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_MODEXP_768.
 */
typedef struct icp_qat_fw_mmp_modexp_768_input_s
{
    uint64_t g; /**< base &ge; 0 and &lt; 2^768 (12 qwords)*/
    uint64_t e; /**< exponent &gt; 0 and &lt; 2^768 (12 qwords)*/
    uint64_t m; /**< modulus   &ge; 2^767 and &lt; 2^768 (12 qwords)*/
} icp_qat_fw_mmp_modexp_768_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular exponentiation base 2 for 1024-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_MODEXP_G2_1024.
 */
typedef struct icp_qat_fw_mmp_modexp_g2_1024_input_s
{
    uint64_t e; /**< exponent &gt; 0 and &lt; 2^1024 (16 qwords)*/
    uint64_t m; /**< modulus   &ge; 2^1023 and &lt; 2^1024 (16 qwords)*/
} icp_qat_fw_mmp_modexp_g2_1024_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular exponentiation for 1024-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_MODEXP_1024.
 */
typedef struct icp_qat_fw_mmp_modexp_1024_input_s
{
    uint64_t g; /**< base &ge; 0 and &lt; 2^1024 (16 qwords)*/
    uint64_t e; /**< exponent &gt; 0 and &lt; 2^1024 (16 qwords)*/
    uint64_t m; /**< modulus   &ge; 2^1023 and &lt; 2^1024 (16 qwords)*/
} icp_qat_fw_mmp_modexp_1024_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular exponentiation base 2 for 1536-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_MODEXP_G2_1536.
 */
typedef struct icp_qat_fw_mmp_modexp_g2_1536_input_s
{
    uint64_t e; /**< exponent &gt; 0 and &lt; 2^1536 (24 qwords)*/
    uint64_t m; /**< modulus   &ge; 2^1535 and &lt; 2^1536 (24 qwords)*/
} icp_qat_fw_mmp_modexp_g2_1536_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular exponentiation for 1536-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_MODEXP_1536.
 */
typedef struct icp_qat_fw_mmp_modexp_1536_input_s
{
    uint64_t g; /**< base &ge; 0 and &lt; 2^1536 (24 qwords)*/
    uint64_t e; /**< exponent &gt; 0 and &lt; 2^1536 (24 qwords)*/
    uint64_t m; /**< modulus   &ge; 2^1535 and &lt; 2^1536 (24 qwords)*/
} icp_qat_fw_mmp_modexp_1536_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular exponentiation base 2 for 2048-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_MODEXP_G2_2048.
 */
typedef struct icp_qat_fw_mmp_modexp_g2_2048_input_s
{
    uint64_t e; /**< exponent &gt; 0 and &lt; 2^2048 (32 qwords)*/
    uint64_t m; /**< modulus  &ge; 2^2047 and &lt; 2^2048 (32 qwords)*/
} icp_qat_fw_mmp_modexp_g2_2048_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular exponentiation for 2048-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_MODEXP_2048.
 */
typedef struct icp_qat_fw_mmp_modexp_2048_input_s
{
    uint64_t g; /**< base &ge; 0 and &lt; 2^2048 (32 qwords)*/
    uint64_t e; /**< exponent &gt; 0 and &lt; 2^2048 (32 qwords)*/
    uint64_t m; /**< modulus   &ge; 2^2047 and &lt; 2^2048 (32 qwords)*/
} icp_qat_fw_mmp_modexp_2048_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular exponentiation base 2 for 3072-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_MODEXP_G2_3072.
 */
typedef struct icp_qat_fw_mmp_modexp_g2_3072_input_s
{
    uint64_t e; /**< exponent &gt; 0 and &lt; 2^3072 (48 qwords)*/
    uint64_t m; /**< modulus  &ge; 2^3071 and &lt; 2^3072 (48 qwords)*/
} icp_qat_fw_mmp_modexp_g2_3072_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular exponentiation for 3072-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_MODEXP_3072.
 */
typedef struct icp_qat_fw_mmp_modexp_3072_input_s
{
    uint64_t g; /**< base &ge; 0 and &lt; 2^3072 (48 qwords)*/
    uint64_t e; /**< exponent &gt; 0 and &lt; 2^3072 (48 qwords)*/
    uint64_t m; /**< modulus  &ge; 2^3071 and &lt; 2^3072 (48 qwords)*/
} icp_qat_fw_mmp_modexp_3072_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular exponentiation base 2 for 4096-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_MODEXP_G2_4096.
 */
typedef struct icp_qat_fw_mmp_modexp_g2_4096_input_s
{
    uint64_t e; /**< exponent &gt; 0 and &lt; 2^4096 (64 qwords)*/
    uint64_t m; /**< modulus   &ge; 2^4095 and &lt; 2^4096 (64 qwords)*/
} icp_qat_fw_mmp_modexp_g2_4096_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular exponentiation for 4096-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_MODEXP_4096.
 */
typedef struct icp_qat_fw_mmp_modexp_4096_input_s
{
    uint64_t g; /**< base &ge; 0 and &lt; 2^4096 (64 qwords)*/
    uint64_t e; /**< exponent &gt; 0 and &lt; 2^4096 (64 qwords)*/
    uint64_t m; /**< modulus   &ge; 2^4095 and &lt; 2^4096 (64 qwords)*/
} icp_qat_fw_mmp_modexp_4096_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for DSA parameter generation P ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_DSA_GEN_P_1024_160.
 */
typedef struct icp_qat_fw_mmp_dsa_gen_p_1024_160_input_s
{
    uint64_t x; /**< DSA 1024-bit randomness  (16 qwords)*/
    uint64_t q; /**< DSA 160-bit parameter  (3 qwords)*/
    uint64_t psp1; /**< product of tiny primes  (16 qwords)*/
    uint64_t psp2; /**< product of small primes  (16 qwords)*/
} icp_qat_fw_mmp_dsa_gen_p_1024_160_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for DSA key generation G ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_DSA_GEN_G_1024_160.
 */
typedef struct icp_qat_fw_mmp_dsa_gen_g_1024_160_input_s
{
    uint64_t p; /**< DSA 1024-bit parameter  (16 qwords)*/
    uint64_t q; /**< DSA 160-bit parameter  (3 qwords)*/
    uint64_t h; /**< DSA 1024-bit parameter  (16 qwords)*/
} icp_qat_fw_mmp_dsa_gen_g_1024_160_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for DSA key generation Y ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_DSA_GEN_Y_1024_160.
 */
typedef struct icp_qat_fw_mmp_dsa_gen_y_1024_160_input_s
{
    uint64_t p; /**< DSA 1024-bit parameter  (16 qwords)*/
    uint64_t g; /**< DSA parameter (16 qwords)*/
    uint64_t x; /**< randomly generated DSA parameter (1024 bits),  (3 qwords)*/
} icp_qat_fw_mmp_dsa_gen_y_1024_160_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for DSA Sign R ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_DSA_SIGN_R_1024_160.
 */
typedef struct icp_qat_fw_mmp_dsa_sign_r_1024_160_input_s
{
    uint64_t k; /**< randomly generated DSA parameter  (3 qwords)*/
    uint64_t p; /**< DSA parameter,  (16 qwords)*/
    uint64_t q; /**< DSA parameter  (3 qwords)*/
    uint64_t g; /**< DSA parameter (16 qwords)*/
} icp_qat_fw_mmp_dsa_sign_r_1024_160_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for DSA Sign S ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_DSA_SIGN_S_1024_160.
 */
typedef struct icp_qat_fw_mmp_dsa_sign_s_1024_160_input_s
{
    uint64_t m; /**< digest message to be signed  (3 qwords)*/
    uint64_t k; /**< randomly generated DSA parameter  (3 qwords)*/
    uint64_t q; /**< DSA parameter  (3 qwords)*/
    uint64_t r; /**< DSA parameter  (3 qwords)*/
    uint64_t x; /**< randomly generated DSA parameter  (3 qwords)*/
} icp_qat_fw_mmp_dsa_sign_s_1024_160_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for DSA Sign R S ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_DSA_SIGN_R_S_1024_160.
 */
typedef struct icp_qat_fw_mmp_dsa_sign_r_s_1024_160_input_s
{
    uint64_t m; /**< digest of the message to be signed  (3 qwords)*/
    uint64_t k; /**< randomly generated DSA parameter  (3 qwords)*/
    uint64_t p; /**< DSA parameter  (16 qwords)*/
    uint64_t q; /**< DSA parameter  (3 qwords)*/
    uint64_t g; /**< DSA parameter (16 qwords)*/
    uint64_t x; /**< randomly generated DSA parameter  (3 qwords)*/
} icp_qat_fw_mmp_dsa_sign_r_s_1024_160_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for DSA Verify V ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_DSA_VERIFY_1024_160.
 */
typedef struct icp_qat_fw_mmp_dsa_verify_1024_160_input_s
{
    uint64_t r; /**< DSA 160-bits signature  (3 qwords)*/
    uint64_t s; /**< DSA 160-bits signature  (3 qwords)*/
    uint64_t m; /**< digest of the message  (3 qwords)*/
    uint64_t p; /**< DSA parameter  (16 qwords)*/
    uint64_t q; /**< DSA parameter  (3 qwords)*/
    uint64_t g; /**< DSA parameter (16 qwords)*/
    uint64_t y; /**< DSA parameter (16 qwords)*/
} icp_qat_fw_mmp_dsa_verify_1024_160_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for RSA 1024 key generation first form ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_RSA_KP1_1024.
 */
typedef struct icp_qat_fw_mmp_rsa_kp1_1024_input_s
{
    uint64_t p; /**< RSA parameter, prime, &nbsp;2 &lt; p &lt; 2^512 (8 qwords)*/
    uint64_t q; /**< RSA parameter, prime, &nbsp;2 &lt; q &lt; 2^512 (8 qwords)*/
    uint64_t e; /**< RSA public key, must be odd, &ge; 3 and &le; (p*q)-1, &nbsp;with GCD(e, p-1, q-1) = 1 (16 qwords)*/
} icp_qat_fw_mmp_rsa_kp1_1024_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for RSA 1024 key generation second form ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_RSA_KP2_1024.
 */
typedef struct icp_qat_fw_mmp_rsa_kp2_1024_input_s
{
    uint64_t p; /**< RSA parameter, prime, &nbsp;2^511 &lt; p &lt; 2^512 (8 qwords)*/
    uint64_t q; /**< RSA parameter, prime, &nbsp;2^511 &lt; q &lt; 2^512 (8 qwords)*/
    uint64_t e; /**< RSA public key, must be odd, &ge; 3 and &le; (p*q)-1, &nbsp;with GCD(e, p-1, q-1) = 1 (16 qwords)*/
} icp_qat_fw_mmp_rsa_kp2_1024_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for RSA 1024 Encryption ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_RSA_EP_1024.
 */
typedef struct icp_qat_fw_mmp_rsa_ep_1024_input_s
{
    uint64_t m; /**< message representative, &lt; n (16 qwords)*/
    uint64_t e; /**< RSA public key, &ge; 3 and &le; n-1 (16 qwords)*/
    uint64_t n; /**< RSA key, &gt; 0 and &lt; 2^1024 (16 qwords)*/
} icp_qat_fw_mmp_rsa_ep_1024_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for RSA 1024 Decryption ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_RSA_DP1_1024.
 */
typedef struct icp_qat_fw_mmp_rsa_dp1_1024_input_s
{
    uint64_t c; /**< cipher text representative, &lt; n (16 qwords)*/
    uint64_t d; /**< RSA private key (RSADP first form) (16 qwords)*/
    uint64_t n; /**< RSA key &gt; 0 and &lt; 2^1024 (16 qwords)*/
} icp_qat_fw_mmp_rsa_dp1_1024_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for RSA 1024 Decryption with CRT ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_RSA_DP2_1024.
 */
typedef struct icp_qat_fw_mmp_rsa_dp2_1024_input_s
{
    uint64_t c; /**< cipher text representative, &lt; (p*q) (16 qwords)*/
    uint64_t p; /**< RSA parameter, prime, &nbsp;2^511 &lt; p &lt; 2^512 (8 qwords)*/
    uint64_t q; /**< RSA parameter, prime, &nbsp;2^511 &lt; q &lt; 2^512 (8 qwords)*/
    uint64_t dp; /**< RSA private key, 0 &lt; dp &lt; p-1 (8 qwords)*/
    uint64_t dq; /**< RSA private key 0 &lt; dq &lt; q-1 (8 qwords)*/
    uint64_t qinv; /**< RSA private key 0 &lt; qInv &lt; p (8 qwords)*/
} icp_qat_fw_mmp_rsa_dp2_1024_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for RSA 1536 key generation first form ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_RSA_KP1_1536.
 */
typedef struct icp_qat_fw_mmp_rsa_kp1_1536_input_s
{
    uint64_t p; /**< RSA parameter, prime, &nbsp;2 &lt; p &lt; 2^768 (12 qwords)*/
    uint64_t q; /**< RSA parameter, prime, &nbsp;2 &lt; q &lt; 2^768 (12 qwords)*/
    uint64_t e; /**< RSA public key, must be odd, &ge; 3 and &le; (p*q)-1, &nbsp;with GCD(e, p-1, q-1) = 1 (24 qwords)*/
} icp_qat_fw_mmp_rsa_kp1_1536_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for RSA 1536 key generation second form ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_RSA_KP2_1536.
 */
typedef struct icp_qat_fw_mmp_rsa_kp2_1536_input_s
{
    uint64_t p; /**< RSA parameter, prime, &nbsp;2^767 &lt; p &lt; 2^768 (12 qwords)*/
    uint64_t q; /**< RSA parameter, prime, &nbsp;2^767 &lt; q &lt; 2^768 (12 qwords)*/
    uint64_t e; /**< RSA public key, must be odd, &ge; 3 and &le; (p*q)-1, &nbsp;with GCD(e, p-1, q-1) = 1 (24 qwords)*/
} icp_qat_fw_mmp_rsa_kp2_1536_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for RSA 1536 Encryption ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_RSA_EP_1536.
 */
typedef struct icp_qat_fw_mmp_rsa_ep_1536_input_s
{
    uint64_t m; /**< message representative, &lt; n (24 qwords)*/
    uint64_t e; /**< RSA public key, &ge; 3 and &le; (p*q)-1 (24 qwords)*/
    uint64_t n; /**< RSA key &gt; 0 and &lt; 2^1536 (24 qwords)*/
} icp_qat_fw_mmp_rsa_ep_1536_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for RSA 1536 Decryption ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_RSA_DP1_1536.
 */
typedef struct icp_qat_fw_mmp_rsa_dp1_1536_input_s
{
    uint64_t c; /**< cipher text representative, &lt; n (24 qwords)*/
    uint64_t d; /**< RSA private key (24 qwords)*/
    uint64_t n; /**< RSA key, &gt; 0 and &lt; 2^1536 (24 qwords)*/
} icp_qat_fw_mmp_rsa_dp1_1536_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for RSA 1536 Decryption with CRT ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_RSA_DP2_1536.
 */
typedef struct icp_qat_fw_mmp_rsa_dp2_1536_input_s
{
    uint64_t c; /**< cipher text representative, &lt; (p*q) (24 qwords)*/
    uint64_t p; /**< RSA parameter, prime, &nbsp;2^767 &lt; p &lt; 2^768 (12 qwords)*/
    uint64_t q; /**< RSA parameter, prime, &nbsp;2^767 &lt; p &lt; 2^768 (12 qwords)*/
    uint64_t dp; /**< RSA private key, 0 &lt; dp &lt; p-1 (12 qwords)*/
    uint64_t dq; /**< RSA private key, 0 &lt; dq &lt; q-1 (12 qwords)*/
    uint64_t qinv; /**< RSA private key, 0 &lt; qInv &lt; p (12 qwords)*/
} icp_qat_fw_mmp_rsa_dp2_1536_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for RSA 2048 key generation first form ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_RSA_KP1_2048.
 */
typedef struct icp_qat_fw_mmp_rsa_kp1_2048_input_s
{
    uint64_t p; /**< RSA parameter, prime, &nbsp;2 &lt; p &lt; 2^1024 (16 qwords)*/
    uint64_t q; /**< RSA parameter, prime, &nbsp;2 &lt; q &lt; 2^1024 (16 qwords)*/
    uint64_t e; /**< RSA public key, must be odd, &ge; 3 and &le; (p*q)-1, &nbsp;with GCD(e, p-1, q-1) = 1 (32 qwords)*/
} icp_qat_fw_mmp_rsa_kp1_2048_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for RSA 2048 key generation second form ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_RSA_KP2_2048.
 */
typedef struct icp_qat_fw_mmp_rsa_kp2_2048_input_s
{
    uint64_t p; /**< RSA parameter, prime, &nbsp;2^1023 &lt; p &lt; 2^1024 (16 qwords)*/
    uint64_t q; /**< RSA parameter, prime, &nbsp;2^1023 &lt; q &lt; 2^1024 (16 qwords)*/
    uint64_t e; /**< RSA public key, must be odd, &ge; 3 and &le; (p*q)-1, &nbsp;with GCD(e, p-1, q-1) = 1 (32 qwords)*/
} icp_qat_fw_mmp_rsa_kp2_2048_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for RSA 2048 Encryption ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_RSA_EP_2048.
 */
typedef struct icp_qat_fw_mmp_rsa_ep_2048_input_s
{
    uint64_t m; /**< message representative, &lt; n (32 qwords)*/
    uint64_t e; /**< RSA public key, &ge; 3 and &le; n-1 (32 qwords)*/
    uint64_t n; /**< RSA key &gt; 0 and &lt; 2^2048 (32 qwords)*/
} icp_qat_fw_mmp_rsa_ep_2048_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for RSA 2048 Decryption ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_RSA_DP1_2048.
 */
typedef struct icp_qat_fw_mmp_rsa_dp1_2048_input_s
{
    uint64_t c; /**< cipher text representative, &lt; n (32 qwords)*/
    uint64_t d; /**< RSA private key (32 qwords)*/
    uint64_t n; /**< RSA key &gt; 0 and &lt; 2^2048 (32 qwords)*/
} icp_qat_fw_mmp_rsa_dp1_2048_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for RSA 2048 Decryption with CRT ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_RSA_DP2_2048.
 */
typedef struct icp_qat_fw_mmp_rsa_dp2_2048_input_s
{
    uint64_t c; /**< cipher text representative, &lt; (p*q) (32 qwords)*/
    uint64_t p; /**< RSA parameter, prime, &nbsp;2^1023 &lt; p &lt; 2^1024 (16 qwords)*/
    uint64_t q; /**< RSA parameter, prime, &nbsp;2^1023 &lt; q &lt; 2^1024 (16 qwords)*/
    uint64_t dp; /**< RSA private key, 0 &lt; dp &lt; p-1 (16 qwords)*/
    uint64_t dq; /**< RSA private key, 0 &lt; dq &lt; q-1 (16 qwords)*/
    uint64_t qinv; /**< RSA private key, 0 &lt; qInv &lt; p (16 qwords)*/
} icp_qat_fw_mmp_rsa_dp2_2048_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for RSA 3072 key generation first form ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_RSA_KP1_3072.
 */
typedef struct icp_qat_fw_mmp_rsa_kp1_3072_input_s
{
    uint64_t p; /**< RSA parameter, prime, &nbsp;2 &lt; p &lt; 2^1536 (24 qwords)*/
    uint64_t q; /**< RSA parameter, prime, &nbsp;2 &lt; q &lt; 2^1536 (24 qwords)*/
    uint64_t e; /**< RSA public key, must be odd, &ge; 3 and &le; (p*q)-1, &nbsp;with GCD(e, p-1, q-1) = 1 (48 qwords)*/
} icp_qat_fw_mmp_rsa_kp1_3072_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for RSA 3072 key generation second form ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_RSA_KP2_3072.
 */
typedef struct icp_qat_fw_mmp_rsa_kp2_3072_input_s
{
    uint64_t p; /**< RSA parameter, prime, &nbsp;2^1535 &lt; p &lt; 2^1536 (24 qwords)*/
    uint64_t q; /**< RSA parameter, prime, &nbsp;2^1535 &lt; q &lt; 2^1536 (24 qwords)*/
    uint64_t e; /**< RSA public key, must be odd, &ge; 3 and &le; (p*q)-1, &nbsp;with GCD(e, p-1, q-1) = 1 (48 qwords)*/
} icp_qat_fw_mmp_rsa_kp2_3072_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for RSA 3072 Encryption ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_RSA_EP_3072.
 */
typedef struct icp_qat_fw_mmp_rsa_ep_3072_input_s
{
    uint64_t m; /**< message representative, &lt; n (48 qwords)*/
    uint64_t e; /**< RSA public key, &ge; 3 and &le; n-1 (48 qwords)*/
    uint64_t n; /**< RSA key &gt; 0 and &lt; 2^3072 (48 qwords)*/
} icp_qat_fw_mmp_rsa_ep_3072_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for RSA 3072 Decryption ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_RSA_DP1_3072.
 */
typedef struct icp_qat_fw_mmp_rsa_dp1_3072_input_s
{
    uint64_t c; /**< cipher text representative, &lt; n (48 qwords)*/
    uint64_t d; /**< RSA private key (48 qwords)*/
    uint64_t n; /**< RSA key &gt; 0 and &lt; 2^3072 (48 qwords)*/
} icp_qat_fw_mmp_rsa_dp1_3072_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for RSA 3072 Decryption with CRT ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_RSA_DP2_3072.
 */
typedef struct icp_qat_fw_mmp_rsa_dp2_3072_input_s
{
    uint64_t c; /**< cipher text representative, &lt; (p*q) (48 qwords)*/
    uint64_t p; /**< RSA parameter, prime, &nbsp;2^1535 &lt; p &lt; 2^1536 (24 qwords)*/
    uint64_t q; /**< RSA parameter, prime, &nbsp;2^1535 &lt; q &lt; 2^1536 (24 qwords)*/
    uint64_t dp; /**< RSA private key, 0 &lt; dp &lt; p-1 (24 qwords)*/
    uint64_t dq; /**< RSA private key, 0 &lt; dq &lt; q-1 (24 qwords)*/
    uint64_t qinv; /**< RSA private key, 0 &lt; qInv &lt; p (24 qwords)*/
} icp_qat_fw_mmp_rsa_dp2_3072_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for RSA 4096 key generation first form ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_RSA_KP1_4096.
 */
typedef struct icp_qat_fw_mmp_rsa_kp1_4096_input_s
{
    uint64_t p; /**< RSA parameter, prime, &nbsp;2 &lt; p &lt; 2^2048 (32 qwords)*/
    uint64_t q; /**< RSA parameter, prime, &nbsp;2 &lt; q &lt; 2^2048 (32 qwords)*/
    uint64_t e; /**< RSA public key, must be odd, &ge; 3 and &le; (p*q)-1, &nbsp;with GCD(e, p-1, q-1) = 1 (64 qwords)*/
} icp_qat_fw_mmp_rsa_kp1_4096_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for RSA 4096 key generation second form ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_RSA_KP2_4096.
 */
typedef struct icp_qat_fw_mmp_rsa_kp2_4096_input_s
{
    uint64_t p; /**< RSA parameter, prime, &nbsp;2^2047 &lt; p &lt; 2^2048 (32 qwords)*/
    uint64_t q; /**< RSA parameter, prime, &nbsp;2^2047 &lt; q &lt; 2^2048 (32 qwords)*/
    uint64_t e; /**< RSA public key, must be odd, &ge; 3 and &le; (p*q)-1, &nbsp;with GCD(e, p-1, q-1) = 1 (64 qwords)*/
} icp_qat_fw_mmp_rsa_kp2_4096_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for RSA 4096 Encryption ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_RSA_EP_4096.
 */
typedef struct icp_qat_fw_mmp_rsa_ep_4096_input_s
{
    uint64_t m; /**< message representative, &lt; n (64 qwords)*/
    uint64_t e; /**< RSA public key, &ge; 3 and &le; n-1 (64 qwords)*/
    uint64_t n; /**< RSA key, &gt; 0 and &lt; 2^4096 (64 qwords)*/
} icp_qat_fw_mmp_rsa_ep_4096_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for RSA 4096 Decryption ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_RSA_DP1_4096.
 */
typedef struct icp_qat_fw_mmp_rsa_dp1_4096_input_s
{
    uint64_t c; /**< cipher text representative, &lt; n (64 qwords)*/
    uint64_t d; /**< RSA private key (64 qwords)*/
    uint64_t n; /**< RSA key, &gt; 0 and &lt; 2^4096 (64 qwords)*/
} icp_qat_fw_mmp_rsa_dp1_4096_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for RSA 4096 Decryption with CRT ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_RSA_DP2_4096.
 */
typedef struct icp_qat_fw_mmp_rsa_dp2_4096_input_s
{
    uint64_t c; /**< cipher text representative, &lt; (p*q) (64 qwords)*/
    uint64_t p; /**< RSA parameter, prime, &nbsp;2^2047 &lt; p &lt; 2^2048 (32 qwords)*/
    uint64_t q; /**< RSA parameter, prime, &nbsp;2^2047 &lt; q &lt; 2^2048 (32 qwords)*/
    uint64_t dp; /**< RSA private key, 0 &lt; dp &lt; p-1 (32 qwords)*/
    uint64_t dq; /**< RSA private key, 0 &lt; dq &lt; q-1 (32 qwords)*/
    uint64_t qinv; /**< RSA private key, 0 &lt; qInv &lt; p (32 qwords)*/
} icp_qat_fw_mmp_rsa_dp2_4096_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for GCD primality test for 192-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_GCD_PT_192.
 */
typedef struct icp_qat_fw_mmp_gcd_pt_192_input_s
{
    uint64_t m; /**< prime candidate &gt; 1 and &lt; 2^192 (3 qwords)*/
    uint64_t psp; /**< product of small primes, odd, &gt; 1 and &lt; 2^1024 (16 qwords)*/
} icp_qat_fw_mmp_gcd_pt_192_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for GCD primality test for 256-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_GCD_PT_256.
 */
typedef struct icp_qat_fw_mmp_gcd_pt_256_input_s
{
    uint64_t m; /**< prime candidate &gt; 1 and &lt; 2^256 (4 qwords)*/
    uint64_t psp; /**< product of small primes, odd, &gt; 1 and &lt; 2^1024 (16 qwords)*/
} icp_qat_fw_mmp_gcd_pt_256_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for GCD primality test for 384-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_GCD_PT_384.
 */
typedef struct icp_qat_fw_mmp_gcd_pt_384_input_s
{
    uint64_t m; /**< prime candidate &gt; 1 and &lt; 2^384 (6 qwords)*/
    uint64_t psp; /**< product of small primes, odd, &gt; 1 and &lt; 2^1024 (16 qwords)*/
} icp_qat_fw_mmp_gcd_pt_384_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for GCD primality test for 512-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_GCD_PT_512.
 */
typedef struct icp_qat_fw_mmp_gcd_pt_512_input_s
{
    uint64_t m; /**< prime candidate &gt; 1 and &lt; 2^512 (8 qwords)*/
    uint64_t psp; /**< product of small primes, odd, &gt; 1 and &lt; 2^1024 (16 qwords)*/
} icp_qat_fw_mmp_gcd_pt_512_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for GCD primality test for 768-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_GCD_PT_768.
 */
typedef struct icp_qat_fw_mmp_gcd_pt_768_input_s
{
    uint64_t m; /**< prime candidate &gt; 1 and &lt; 2^768 (12 qwords)*/
    uint64_t psp; /**< product of small primes, odd, &gt; 1 and &lt; 2^1024 (16 qwords)*/
} icp_qat_fw_mmp_gcd_pt_768_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for GCD primality test for 1024-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_GCD_PT_1024.
 */
typedef struct icp_qat_fw_mmp_gcd_pt_1024_input_s
{
    uint64_t m; /**< prime candidate &gt; 1 and &lt; 2^1024 (16 qwords)*/
    uint64_t psp; /**< product of small primes, odd, &gt; 1 and &lt; 2^1024 (16 qwords)*/
} icp_qat_fw_mmp_gcd_pt_1024_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for GCD primality test for 1536-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_GCD_PT_1536.
 */
typedef struct icp_qat_fw_mmp_gcd_pt_1536_input_s
{
    uint64_t m; /**< prime candidate &gt; 1 and &lt; 2^1536 (24 qwords)*/
    uint64_t psp; /**< product of small primes, odd, &gt; 1 and &lt; 2^1024 (16 qwords)*/
} icp_qat_fw_mmp_gcd_pt_1536_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for GCD primality test for 2048-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_GCD_PT_2048.
 */
typedef struct icp_qat_fw_mmp_gcd_pt_2048_input_s
{
    uint64_t m; /**< prime candidate &gt; 1 and &lt; 2^2048 (32 qwords)*/
    uint64_t psp; /**< product of small primes, odd, &gt; 1 and &lt; 2^1024 (16 qwords)*/
} icp_qat_fw_mmp_gcd_pt_2048_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for GCD primality test for 3072-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_GCD_PT_3072.
 */
typedef struct icp_qat_fw_mmp_gcd_pt_3072_input_s
{
    uint64_t m; /**< prime candidate &gt; 1 and &lt; 2^3072 (48 qwords)*/
    uint64_t psp; /**< product of small primes, odd, &gt; 1 and &lt; 2^1024 (16 qwords)*/
} icp_qat_fw_mmp_gcd_pt_3072_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for GCD primality test for 4096-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_GCD_PT_4096.
 */
typedef struct icp_qat_fw_mmp_gcd_pt_4096_input_s
{
    uint64_t m; /**< prime candidate &gt; 1 and &lt; 2^4096 (64 qwords)*/
    uint64_t psp; /**< product of small primes, odd, &gt; 1 and &lt; 2^1024 (16 qwords)*/
} icp_qat_fw_mmp_gcd_pt_4096_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Fermat primality test for 160-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_FERMAT_PT_160.
 */
typedef struct icp_qat_fw_mmp_fermat_pt_160_input_s
{
    uint64_t m; /**< prime candidate, 2^159 &lt; m &lt; 2^160 (3 qwords)*/
} icp_qat_fw_mmp_fermat_pt_160_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Fermat primality test for 512-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_FERMAT_PT_512.
 */
typedef struct icp_qat_fw_mmp_fermat_pt_512_input_s
{
    uint64_t m; /**< prime candidate, 2^511 &lt; m &lt; 2^512 (8 qwords)*/
} icp_qat_fw_mmp_fermat_pt_512_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Fermat primality test for 768-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_FERMAT_PT_768.
 */
typedef struct icp_qat_fw_mmp_fermat_pt_768_input_s
{
    uint64_t m; /**< prime candidate, 2^767 &lt; m &lt; 2^768 (12 qwords)*/
} icp_qat_fw_mmp_fermat_pt_768_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Fermat primality test for 1024-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_FERMAT_PT_1024.
 */
typedef struct icp_qat_fw_mmp_fermat_pt_1024_input_s
{
    uint64_t m; /**< prime candidate, 2^1023 &lt; m &lt; 2^1024 (16 qwords)*/
} icp_qat_fw_mmp_fermat_pt_1024_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Fermat primality test for 1536-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_FERMAT_PT_1536.
 */
typedef struct icp_qat_fw_mmp_fermat_pt_1536_input_s
{
    uint64_t m; /**< prime candidate, 2^1535 &lt; m &lt; 2^1536 (24 qwords)*/
} icp_qat_fw_mmp_fermat_pt_1536_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Fermat primality test for 2048-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_FERMAT_PT_2048.
 */
typedef struct icp_qat_fw_mmp_fermat_pt_2048_input_s
{
    uint64_t m; /**< prime candidate, 2^2047 &lt; m &lt; 2^2048 (32 qwords)*/
} icp_qat_fw_mmp_fermat_pt_2048_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Fermat primality test for 3072-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_FERMAT_PT_3072.
 */
typedef struct icp_qat_fw_mmp_fermat_pt_3072_input_s
{
    uint64_t m; /**< prime candidate, 2^3071 &lt; m &lt; 2^3072 (48 qwords)*/
} icp_qat_fw_mmp_fermat_pt_3072_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Fermat primality test for 4096-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_FERMAT_PT_4096.
 */
typedef struct icp_qat_fw_mmp_fermat_pt_4096_input_s
{
    uint64_t m; /**< prime candidate, 2^4095 &lt; m &lt; 2^4096 (64 qwords)*/
} icp_qat_fw_mmp_fermat_pt_4096_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Miller-Rabin primality test for 160-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_MR_PT_160.
 */
typedef struct icp_qat_fw_mmp_mr_pt_160_input_s
{
    uint64_t x; /**< randomness  &gt; 1 and &lt; m-1 (3 qwords)*/
    uint64_t m; /**< prime candidate &gt; 2^159 and &lt; 2^160 (3 qwords)*/
} icp_qat_fw_mmp_mr_pt_160_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Miller-Rabin primality test for 512-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_MR_PT_512.
 */
typedef struct icp_qat_fw_mmp_mr_pt_512_input_s
{
    uint64_t x; /**< randomness   &gt; 1 and &lt; m-1 (8 qwords)*/
    uint64_t m; /**< prime candidate  &gt; 2^511 and &lt; 2^512 (8 qwords)*/
} icp_qat_fw_mmp_mr_pt_512_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Miller-Rabin primality test for 768-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_MR_PT_768.
 */
typedef struct icp_qat_fw_mmp_mr_pt_768_input_s
{
    uint64_t x; /**< randomness  &gt; 1 and &lt; m-1 (12 qwords)*/
    uint64_t m; /**< prime candidate &gt; 2^767 and &lt; 2^768 (12 qwords)*/
} icp_qat_fw_mmp_mr_pt_768_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Miller-Rabin primality test for 1024-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_MR_PT_1024.
 */
typedef struct icp_qat_fw_mmp_mr_pt_1024_input_s
{
    uint64_t x; /**< randomness &gt; 1 and &lt; m-1 (16 qwords)*/
    uint64_t m; /**< prime candidate &gt; 2^1023 and &lt; 2^1024 (16 qwords)*/
} icp_qat_fw_mmp_mr_pt_1024_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Miller-Rabin primality test for 1536-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_MR_PT_1536.
 */
typedef struct icp_qat_fw_mmp_mr_pt_1536_input_s
{
    uint64_t x; /**< randomness &gt; 1 and &lt; m-1 (24 qwords)*/
    uint64_t m; /**< prime candidate &gt; 2^1535 and &lt; 2^1536 (24 qwords)*/
} icp_qat_fw_mmp_mr_pt_1536_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Miller-Rabin primality test for 2048-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_MR_PT_2048.
 */
typedef struct icp_qat_fw_mmp_mr_pt_2048_input_s
{
    uint64_t x; /**< randomness  &gt; 1 and &lt;m-1 (32 qwords)*/
    uint64_t m; /**< prime candidate  &gt; 2^2047 and &lt; 2^2048 (32 qwords)*/
} icp_qat_fw_mmp_mr_pt_2048_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Miller-Rabin primality test for 3072-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_MR_PT_3072.
 */
typedef struct icp_qat_fw_mmp_mr_pt_3072_input_s
{
    uint64_t x; /**< randomness  &gt; 1 and &lt; m-1 (48 qwords)*/
    uint64_t m; /**< prime candidate &gt; 2^3071 and &lt; 2^3072 (48 qwords)*/
} icp_qat_fw_mmp_mr_pt_3072_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Miller-Rabin primality test for 4096-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_MR_PT_4096.
 */
typedef struct icp_qat_fw_mmp_mr_pt_4096_input_s
{
    uint64_t x; /**< randomness  &gt; 1 and &lt; m-1 (64 qwords)*/
    uint64_t m; /**< prime candidate &gt; 2^4095 and &lt; 2^4096 (64 qwords)*/
} icp_qat_fw_mmp_mr_pt_4096_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Lucas primality test for 160-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_LUCAS_PT_160.
 */
typedef struct icp_qat_fw_mmp_lucas_pt_160_input_s
{
    uint64_t m; /**< odd prime candidate &gt; 2^159 and &lt; 2^160 (3 qwords)*/
} icp_qat_fw_mmp_lucas_pt_160_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Lucas primality test for 512-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_LUCAS_PT_512.
 */
typedef struct icp_qat_fw_mmp_lucas_pt_512_input_s
{
    uint64_t m; /**< odd prime candidate &gt; 2^511 and &lt; 2^512 (8 qwords)*/
} icp_qat_fw_mmp_lucas_pt_512_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Lucas primality test for 768-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_LUCAS_PT_768.
 */
typedef struct icp_qat_fw_mmp_lucas_pt_768_input_s
{
    uint64_t m; /**< odd prime candidate &gt; 2^767 and &lt; 2^768 (12 qwords)*/
} icp_qat_fw_mmp_lucas_pt_768_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Lucas primality test for 1024-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_LUCAS_PT_1024.
 */
typedef struct icp_qat_fw_mmp_lucas_pt_1024_input_s
{
    uint64_t m; /**< odd prime candidate &gt; 2^1023 and &lt; 2^1024 (16 qwords)*/
} icp_qat_fw_mmp_lucas_pt_1024_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Lucas primality test for 1536-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_LUCAS_PT_1536.
 */
typedef struct icp_qat_fw_mmp_lucas_pt_1536_input_s
{
    uint64_t m; /**< odd prime candidate &gt; 2^1535 and &lt; 2^1536 (24 qwords)*/
} icp_qat_fw_mmp_lucas_pt_1536_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Lucas primality test for 2048-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_LUCAS_PT_2048.
 */
typedef struct icp_qat_fw_mmp_lucas_pt_2048_input_s
{
    uint64_t m; /**< odd prime candidate &gt; 2^2047 and &lt; 2^2048 (32 qwords)*/
} icp_qat_fw_mmp_lucas_pt_2048_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Lucas primality test for 3072-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_LUCAS_PT_3072.
 */
typedef struct icp_qat_fw_mmp_lucas_pt_3072_input_s
{
    uint64_t m; /**< odd prime candidate &gt; 2^3071 and &lt; 2^3072 (48 qwords)*/
} icp_qat_fw_mmp_lucas_pt_3072_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Lucas primality test for 4096-bit numbers ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #PKE_LUCAS_PT_4096.
 */
typedef struct icp_qat_fw_mmp_lucas_pt_4096_input_s
{
    uint64_t m; /**< odd prime candidate &gt; 2^4096 and &lt; 2^4096 (64 qwords)*/
} icp_qat_fw_mmp_lucas_pt_4096_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular exponentiation for numbers less than 512-bits ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #MATHS_MODEXP_L512.
 */
typedef struct icp_qat_fw_maths_modexp_l512_input_s
{
    uint64_t g; /**< base &ge; 0 and &lt; 2^512 (8 qwords)*/
    uint64_t e; /**< exponent &ge; 0 and &lt; 2^512 (8 qwords)*/
    uint64_t m; /**< modulus   &gt; 0 and &lt; 2^512 (8 qwords)*/
} icp_qat_fw_maths_modexp_l512_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular exponentiation for numbers less than 1024-bit ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #MATHS_MODEXP_L1024.
 */
typedef struct icp_qat_fw_maths_modexp_l1024_input_s
{
    uint64_t g; /**< base &ge; 0 and &lt; 2^1024 (16 qwords)*/
    uint64_t e; /**< exponent &ge; 0 and &lt; 2^1024 (16 qwords)*/
    uint64_t m; /**< modulus &gt; 0 and &lt; 2^1024 (16 qwords)*/
} icp_qat_fw_maths_modexp_l1024_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular exponentiation for numbers less than 1536-bits ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #MATHS_MODEXP_L1536.
 */
typedef struct icp_qat_fw_maths_modexp_l1536_input_s
{
    uint64_t g; /**< base &ge; 0 and &lt; 2^1536 (24 qwords)*/
    uint64_t e; /**< exponent &ge; 0 and &lt; 2^1536 (24 qwords)*/
    uint64_t m; /**< modulus   &gt; 0 and &lt; 2^1536 (24 qwords)*/
} icp_qat_fw_maths_modexp_l1536_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular exponentiation for numbers less than 2048-bit ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #MATHS_MODEXP_L2048.
 */
typedef struct icp_qat_fw_maths_modexp_l2048_input_s
{
    uint64_t g; /**< base &ge; 0 and &lt; 2^2048 (32 qwords)*/
    uint64_t e; /**< exponent &ge; 0 and &lt; 2^2048 (32 qwords)*/
    uint64_t m; /**< modulus &gt; 0 and &lt; 2^2048 (32 qwords)*/
} icp_qat_fw_maths_modexp_l2048_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular exponentiation for numbers less than 2560-bits ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #MATHS_MODEXP_L2560.
 */
typedef struct icp_qat_fw_maths_modexp_l2560_input_s
{
    uint64_t g; /**< base &ge; 0 and &lt; 2^2560 (40 qwords)*/
    uint64_t e; /**< exponent &ge; 0 and &lt; 2^2560 (40 qwords)*/
    uint64_t m; /**< modulus   &gt; 0 and &lt; 2^2560 (40 qwords)*/
} icp_qat_fw_maths_modexp_l2560_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular exponentiation for numbers less than 3072-bits ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #MATHS_MODEXP_L3072.
 */
typedef struct icp_qat_fw_maths_modexp_l3072_input_s
{
    uint64_t g; /**< base &ge; 0 and &lt; 2^3072 (48 qwords)*/
    uint64_t e; /**< exponent &ge; 0 and &lt; 2^3072 (48 qwords)*/
    uint64_t m; /**< modulus   &gt; 0 and &lt; 2^3072 (48 qwords)*/
} icp_qat_fw_maths_modexp_l3072_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular exponentiation for numbers less than 3584-bits ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #MATHS_MODEXP_L3584.
 */
typedef struct icp_qat_fw_maths_modexp_l3584_input_s
{
    uint64_t g; /**< base &ge; 0 and &lt; 2^3584 (56 qwords)*/
    uint64_t e; /**< exponent &ge; 0 and &lt; 2^3584 (56 qwords)*/
    uint64_t m; /**< modulus   &gt; 0 and &lt; 2^3584 (56 qwords)*/
} icp_qat_fw_maths_modexp_l3584_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular exponentiation for numbers less than 4096-bit ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #MATHS_MODEXP_L4096.
 */
typedef struct icp_qat_fw_maths_modexp_l4096_input_s
{
    uint64_t g; /**< base &ge; 0 and &lt; 2^4096 (64 qwords)*/
    uint64_t e; /**< exponent &ge; 0 and &lt; 2^4096 (64 qwords)*/
    uint64_t m; /**< modulus   &gt; 0 and &lt; 2^4096 (64 qwords)*/
} icp_qat_fw_maths_modexp_l4096_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular multiplicative inverse for numbers less than 128 bits ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #MATHS_MODINV_ODD_L128.
 */
typedef struct icp_qat_fw_maths_modinv_odd_l128_input_s
{
    uint64_t a; /**< number &gt; 0 and &lt; 2^128 (2 qwords)*/
    uint64_t b; /**< odd modulus &gt; 0 and &lt; 2^128, coprime to a (2 qwords)*/
} icp_qat_fw_maths_modinv_odd_l128_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular multiplicative inverse for numbers less than 192 bits ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #MATHS_MODINV_ODD_L192.
 */
typedef struct icp_qat_fw_maths_modinv_odd_l192_input_s
{
    uint64_t a; /**< number &gt; 0 and &lt; 2^192 (3 qwords)*/
    uint64_t b; /**< odd modulus &gt; 0 and &lt; 2^192, coprime to a (3 qwords)*/
} icp_qat_fw_maths_modinv_odd_l192_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular multiplicative inverse for numbers less than 256 bits ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #MATHS_MODINV_ODD_L256.
 */
typedef struct icp_qat_fw_maths_modinv_odd_l256_input_s
{
    uint64_t a; /**< number &gt; 0 and &lt; 2^256 (4 qwords)*/
    uint64_t b; /**< odd modulus &gt; 0 and &lt; 2^256, coprime to a (4 qwords)*/
} icp_qat_fw_maths_modinv_odd_l256_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular multiplicative inverse for numbers less than 384 bits ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #MATHS_MODINV_ODD_L384.
 */
typedef struct icp_qat_fw_maths_modinv_odd_l384_input_s
{
    uint64_t a; /**< number &gt; 0 and &lt; 2^384 (6 qwords)*/
    uint64_t b; /**< odd modulus &gt; 0 and &lt; 2^384, coprime to a (6 qwords)*/
} icp_qat_fw_maths_modinv_odd_l384_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular multiplicative inverse for numbers less than 512 bits ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #MATHS_MODINV_ODD_L512.
 */
typedef struct icp_qat_fw_maths_modinv_odd_l512_input_s
{
    uint64_t a; /**< number &gt; 0 and &lt; 2^512 (8 qwords)*/
    uint64_t b; /**< odd modulus &gt; 0 and &lt; 2^512, coprime to a (8 qwords)*/
} icp_qat_fw_maths_modinv_odd_l512_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular multiplicative inverse for numbers less than 768 bits ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #MATHS_MODINV_ODD_L768.
 */
typedef struct icp_qat_fw_maths_modinv_odd_l768_input_s
{
    uint64_t a; /**< number &gt; 0 and &lt; 2^768 (12 qwords)*/
    uint64_t b; /**< odd modulus &gt; 0 and &lt; 2^768 ,coprime to a (12 qwords)*/
} icp_qat_fw_maths_modinv_odd_l768_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular multiplicative inverse for numbers less than 1024 bits ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #MATHS_MODINV_ODD_L1024.
 */
typedef struct icp_qat_fw_maths_modinv_odd_l1024_input_s
{
    uint64_t a; /**< number &gt; 0 and &lt; 2^1024 (16 qwords)*/
    uint64_t b; /**< odd modulus &gt; 0 and &lt; 2^1024, coprime to a (16 qwords)*/
} icp_qat_fw_maths_modinv_odd_l1024_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular multiplicative inverse for numbers less than 1536 bits ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #MATHS_MODINV_ODD_L1536.
 */
typedef struct icp_qat_fw_maths_modinv_odd_l1536_input_s
{
    uint64_t a; /**< number &gt; 0 and &lt; 2^1536 (24 qwords)*/
    uint64_t b; /**< odd modulus &gt; 0 and &lt; 2^1536, coprime to a (24 qwords)*/
} icp_qat_fw_maths_modinv_odd_l1536_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular multiplicative inverse for numbers less than 2048 bits ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #MATHS_MODINV_ODD_L2048.
 */
typedef struct icp_qat_fw_maths_modinv_odd_l2048_input_s
{
    uint64_t a; /**< number &gt; 0 and &lt; 2^2048 (32 qwords)*/
    uint64_t b; /**< odd modulus &gt; 0 and &lt; 2^2048, coprime to a (32 qwords)*/
} icp_qat_fw_maths_modinv_odd_l2048_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular multiplicative inverse for numbers less than 3072 bits ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #MATHS_MODINV_ODD_L3072.
 */
typedef struct icp_qat_fw_maths_modinv_odd_l3072_input_s
{
    uint64_t a; /**< number &gt; 0 and &lt; 2^3072 (48 qwords)*/
    uint64_t b; /**< odd modulus &gt; 0 and &lt; 2^3072, coprime to a (48 qwords)*/
} icp_qat_fw_maths_modinv_odd_l3072_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular multiplicative inverse for numbers less than 4096 bits ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #MATHS_MODINV_ODD_L4096.
 */
typedef struct icp_qat_fw_maths_modinv_odd_l4096_input_s
{
    uint64_t a; /**< number &gt; 0 and &lt; 2^4096 (64 qwords)*/
    uint64_t b; /**< odd modulus &gt; 0 and &lt; 2^4096, coprime to a (64 qwords)*/
} icp_qat_fw_maths_modinv_odd_l4096_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular multiplicative inverse for numbers less than 128 bits ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #MATHS_MODINV_EVEN_L128.
 */
typedef struct icp_qat_fw_maths_modinv_even_l128_input_s
{
    uint64_t a; /**< odd number &gt; 0 and &lt; 2^128 (2 qwords)*/
    uint64_t b; /**< even modulus   &gt; 0 and &lt; 2^128, coprime with a (2 qwords)*/
} icp_qat_fw_maths_modinv_even_l128_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular multiplicative inverse for numbers less than 192 bits ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #MATHS_MODINV_EVEN_L192.
 */
typedef struct icp_qat_fw_maths_modinv_even_l192_input_s
{
    uint64_t a; /**< odd number &gt; 0 and &lt; 2^192 (3 qwords)*/
    uint64_t b; /**< even modulus   &gt; 0 and &lt; 2^192, coprime with a (3 qwords)*/
} icp_qat_fw_maths_modinv_even_l192_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular multiplicative inverse for numbers less than 256 bits ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #MATHS_MODINV_EVEN_L256.
 */
typedef struct icp_qat_fw_maths_modinv_even_l256_input_s
{
    uint64_t a; /**< odd number &gt; 0 and &lt; 2^256 (4 qwords)*/
    uint64_t b; /**< even modulus   &gt; 0 and &lt; 2^256, coprime with a (4 qwords)*/
} icp_qat_fw_maths_modinv_even_l256_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular multiplicative inverse for numbers less than 384 bits ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #MATHS_MODINV_EVEN_L384.
 */
typedef struct icp_qat_fw_maths_modinv_even_l384_input_s
{
    uint64_t a; /**< odd number &gt; 0 and &lt; 2^384 (6 qwords)*/
    uint64_t b; /**< even modulus   &gt; 0 and &lt; 2^384, coprime with a (6 qwords)*/
} icp_qat_fw_maths_modinv_even_l384_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular multiplicative inverse for numbers less than 512 bits ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #MATHS_MODINV_EVEN_L512.
 */
typedef struct icp_qat_fw_maths_modinv_even_l512_input_s
{
    uint64_t a; /**< odd number &gt; 0 and &lt; 2^512 (8 qwords)*/
    uint64_t b; /**< even modulus   &gt; 0 and &lt; 2^512, coprime with a (8 qwords)*/
} icp_qat_fw_maths_modinv_even_l512_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular multiplicative inverse for numbers less than 768 bits ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #MATHS_MODINV_EVEN_L768.
 */
typedef struct icp_qat_fw_maths_modinv_even_l768_input_s
{
    uint64_t a; /**< odd number &gt; 0 and &lt; 2^768 (12 qwords)*/
    uint64_t b; /**< even modulus   &gt; 0 and &lt; 2^768, coprime with a (12 qwords)*/
} icp_qat_fw_maths_modinv_even_l768_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular multiplicative inverse for numbers less than 1024 bits ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #MATHS_MODINV_EVEN_L1024.
 */
typedef struct icp_qat_fw_maths_modinv_even_l1024_input_s
{
    uint64_t a; /**< odd number &gt; 0 and &lt; 2^1024 (16 qwords)*/
    uint64_t b; /**< even modulus   &gt; 0 and &lt; 2^1024, coprime with a (16 qwords)*/
} icp_qat_fw_maths_modinv_even_l1024_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular multiplicative inverse for numbers less than 1536 bits ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #MATHS_MODINV_EVEN_L1536.
 */
typedef struct icp_qat_fw_maths_modinv_even_l1536_input_s
{
    uint64_t a; /**< odd number &gt; 0 and &lt; 2^1536 (24 qwords)*/
    uint64_t b; /**< even modulus   &gt; 0 and &lt; 2^1536, coprime with a (24 qwords)*/
} icp_qat_fw_maths_modinv_even_l1536_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular multiplicative inverse for numbers less than 2048 bits ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #MATHS_MODINV_EVEN_L2048.
 */
typedef struct icp_qat_fw_maths_modinv_even_l2048_input_s
{
    uint64_t a; /**< odd number &gt; 0 and &lt; 2^2048 (32 qwords)*/
    uint64_t b; /**< even modulus   &gt; 0 and &lt; 2^2048, coprime with a (32 qwords)*/
} icp_qat_fw_maths_modinv_even_l2048_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular multiplicative inverse for numbers less than 3072 bits ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #MATHS_MODINV_EVEN_L3072.
 */
typedef struct icp_qat_fw_maths_modinv_even_l3072_input_s
{
    uint64_t a; /**< odd number &gt; 0 and &lt; 2^3072 (48 qwords)*/
    uint64_t b; /**< even modulus   &gt; 0 and &lt; 2^3072, coprime with a (48 qwords)*/
} icp_qat_fw_maths_modinv_even_l3072_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Input parameter list for Modular multiplicative inverse for numbers less than 4096 bits ,
 *      to be used when icp_qat_fw_pke_request_s::functionalityId is #MATHS_MODINV_EVEN_L4096.
 */
typedef struct icp_qat_fw_maths_modinv_even_l4096_input_s
{
    uint64_t a; /**< odd number &gt; 0 and &lt; 2^4096 (64 qwords)*/
    uint64_t b; /**< even modulus   &gt; 0 and &lt; 2^4096, coprime with a (64 qwords)*/
} icp_qat_fw_maths_modinv_even_l4096_input_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    MMP input parameters
 */
typedef union icp_qat_fw_mmp_input_param_u
{
    /** Generic parameter structure : All members of this wrapper structure
     * are pointers to large integers.
     */
    uint64_t flat_array[ICP_QAT_FW_PKE_INPUT_COUNT_MAX];

    /** Modular exponentiation base 2 for 160-bit numbers  */
    icp_qat_fw_mmp_modexp_g2_160_input_t mmp_modexp_g2_160;

    /** Modular exponentiation for 160-bit numbers  */
    icp_qat_fw_mmp_modexp_160_input_t mmp_modexp_160;

    /** Modular exponentiation base 2 for 512-bit numbers  */
    icp_qat_fw_mmp_modexp_g2_512_input_t mmp_modexp_g2_512;

    /** Modular exponentiation for 512-bit numbers  */
    icp_qat_fw_mmp_modexp_512_input_t mmp_modexp_512;

    /** Modular exponentiation base 2 for 768-bit numbers  */
    icp_qat_fw_mmp_modexp_g2_768_input_t mmp_modexp_g2_768;

    /** Modular exponentiation for 768-bit numbers  */
    icp_qat_fw_mmp_modexp_768_input_t mmp_modexp_768;

    /** Modular exponentiation base 2 for 1024-bit numbers  */
    icp_qat_fw_mmp_modexp_g2_1024_input_t mmp_modexp_g2_1024;

    /** Modular exponentiation for 1024-bit numbers  */
    icp_qat_fw_mmp_modexp_1024_input_t mmp_modexp_1024;

    /** Modular exponentiation base 2 for 1536-bit numbers  */
    icp_qat_fw_mmp_modexp_g2_1536_input_t mmp_modexp_g2_1536;

    /** Modular exponentiation for 1536-bit numbers  */
    icp_qat_fw_mmp_modexp_1536_input_t mmp_modexp_1536;

    /** Modular exponentiation base 2 for 2048-bit numbers  */
    icp_qat_fw_mmp_modexp_g2_2048_input_t mmp_modexp_g2_2048;

    /** Modular exponentiation for 2048-bit numbers  */
    icp_qat_fw_mmp_modexp_2048_input_t mmp_modexp_2048;

    /** Modular exponentiation base 2 for 3072-bit numbers  */
    icp_qat_fw_mmp_modexp_g2_3072_input_t mmp_modexp_g2_3072;

    /** Modular exponentiation for 3072-bit numbers  */
    icp_qat_fw_mmp_modexp_3072_input_t mmp_modexp_3072;

    /** Modular exponentiation base 2 for 4096-bit numbers  */
    icp_qat_fw_mmp_modexp_g2_4096_input_t mmp_modexp_g2_4096;

    /** Modular exponentiation for 4096-bit numbers  */
    icp_qat_fw_mmp_modexp_4096_input_t mmp_modexp_4096;

    /** DSA parameter generation P  */
    icp_qat_fw_mmp_dsa_gen_p_1024_160_input_t mmp_dsa_gen_p_1024_160;

    /** DSA key generation G  */
    icp_qat_fw_mmp_dsa_gen_g_1024_160_input_t mmp_dsa_gen_g_1024_160;

    /** DSA key generation Y  */
    icp_qat_fw_mmp_dsa_gen_y_1024_160_input_t mmp_dsa_gen_y_1024_160;

    /** DSA Sign R  */
    icp_qat_fw_mmp_dsa_sign_r_1024_160_input_t mmp_dsa_sign_r_1024_160;

    /** DSA Sign S  */
    icp_qat_fw_mmp_dsa_sign_s_1024_160_input_t mmp_dsa_sign_s_1024_160;

    /** DSA Sign R S  */
    icp_qat_fw_mmp_dsa_sign_r_s_1024_160_input_t mmp_dsa_sign_r_s_1024_160;

    /** DSA Verify V  */
    icp_qat_fw_mmp_dsa_verify_1024_160_input_t mmp_dsa_verify_1024_160;

    /** RSA 1024 key generation first form  */
    icp_qat_fw_mmp_rsa_kp1_1024_input_t mmp_rsa_kp1_1024;

    /** RSA 1024 key generation second form  */
    icp_qat_fw_mmp_rsa_kp2_1024_input_t mmp_rsa_kp2_1024;

    /** RSA 1024 Encryption  */
    icp_qat_fw_mmp_rsa_ep_1024_input_t mmp_rsa_ep_1024;

    /** RSA 1024 Decryption  */
    icp_qat_fw_mmp_rsa_dp1_1024_input_t mmp_rsa_dp1_1024;

    /** RSA 1024 Decryption with CRT  */
    icp_qat_fw_mmp_rsa_dp2_1024_input_t mmp_rsa_dp2_1024;

    /** RSA 1536 key generation first form  */
    icp_qat_fw_mmp_rsa_kp1_1536_input_t mmp_rsa_kp1_1536;

    /** RSA 1536 key generation second form  */
    icp_qat_fw_mmp_rsa_kp2_1536_input_t mmp_rsa_kp2_1536;

    /** RSA 1536 Encryption  */
    icp_qat_fw_mmp_rsa_ep_1536_input_t mmp_rsa_ep_1536;

    /** RSA 1536 Decryption  */
    icp_qat_fw_mmp_rsa_dp1_1536_input_t mmp_rsa_dp1_1536;

    /** RSA 1536 Decryption with CRT  */
    icp_qat_fw_mmp_rsa_dp2_1536_input_t mmp_rsa_dp2_1536;

    /** RSA 2048 key generation first form  */
    icp_qat_fw_mmp_rsa_kp1_2048_input_t mmp_rsa_kp1_2048;

    /** RSA 2048 key generation second form  */
    icp_qat_fw_mmp_rsa_kp2_2048_input_t mmp_rsa_kp2_2048;

    /** RSA 2048 Encryption  */
    icp_qat_fw_mmp_rsa_ep_2048_input_t mmp_rsa_ep_2048;

    /** RSA 2048 Decryption  */
    icp_qat_fw_mmp_rsa_dp1_2048_input_t mmp_rsa_dp1_2048;

    /** RSA 2048 Decryption with CRT  */
    icp_qat_fw_mmp_rsa_dp2_2048_input_t mmp_rsa_dp2_2048;

    /** RSA 3072 key generation first form  */
    icp_qat_fw_mmp_rsa_kp1_3072_input_t mmp_rsa_kp1_3072;

    /** RSA 3072 key generation second form  */
    icp_qat_fw_mmp_rsa_kp2_3072_input_t mmp_rsa_kp2_3072;

    /** RSA 3072 Encryption  */
    icp_qat_fw_mmp_rsa_ep_3072_input_t mmp_rsa_ep_3072;

    /** RSA 3072 Decryption  */
    icp_qat_fw_mmp_rsa_dp1_3072_input_t mmp_rsa_dp1_3072;

    /** RSA 3072 Decryption with CRT  */
    icp_qat_fw_mmp_rsa_dp2_3072_input_t mmp_rsa_dp2_3072;

    /** RSA 4096 key generation first form  */
    icp_qat_fw_mmp_rsa_kp1_4096_input_t mmp_rsa_kp1_4096;

    /** RSA 4096 key generation second form  */
    icp_qat_fw_mmp_rsa_kp2_4096_input_t mmp_rsa_kp2_4096;

    /** RSA 4096 Encryption  */
    icp_qat_fw_mmp_rsa_ep_4096_input_t mmp_rsa_ep_4096;

    /** RSA 4096 Decryption  */
    icp_qat_fw_mmp_rsa_dp1_4096_input_t mmp_rsa_dp1_4096;

    /** RSA 4096 Decryption with CRT  */
    icp_qat_fw_mmp_rsa_dp2_4096_input_t mmp_rsa_dp2_4096;

    /** GCD primality test for 192-bit numbers  */
    icp_qat_fw_mmp_gcd_pt_192_input_t mmp_gcd_pt_192;

    /** GCD primality test for 256-bit numbers  */
    icp_qat_fw_mmp_gcd_pt_256_input_t mmp_gcd_pt_256;

    /** GCD primality test for 384-bit numbers  */
    icp_qat_fw_mmp_gcd_pt_384_input_t mmp_gcd_pt_384;

    /** GCD primality test for 512-bit numbers  */
    icp_qat_fw_mmp_gcd_pt_512_input_t mmp_gcd_pt_512;

    /** GCD primality test for 768-bit numbers  */
    icp_qat_fw_mmp_gcd_pt_768_input_t mmp_gcd_pt_768;

    /** GCD primality test for 1024-bit numbers  */
    icp_qat_fw_mmp_gcd_pt_1024_input_t mmp_gcd_pt_1024;

    /** GCD primality test for 1536-bit numbers  */
    icp_qat_fw_mmp_gcd_pt_1536_input_t mmp_gcd_pt_1536;

    /** GCD primality test for 2048-bit numbers  */
    icp_qat_fw_mmp_gcd_pt_2048_input_t mmp_gcd_pt_2048;

    /** GCD primality test for 3072-bit numbers  */
    icp_qat_fw_mmp_gcd_pt_3072_input_t mmp_gcd_pt_3072;

    /** GCD primality test for 4096-bit numbers  */
    icp_qat_fw_mmp_gcd_pt_4096_input_t mmp_gcd_pt_4096;

    /** Fermat primality test for 160-bit numbers  */
    icp_qat_fw_mmp_fermat_pt_160_input_t mmp_fermat_pt_160;

    /** Fermat primality test for 512-bit numbers  */
    icp_qat_fw_mmp_fermat_pt_512_input_t mmp_fermat_pt_512;

    /** Fermat primality test for 768-bit numbers  */
    icp_qat_fw_mmp_fermat_pt_768_input_t mmp_fermat_pt_768;

    /** Fermat primality test for 1024-bit numbers  */
    icp_qat_fw_mmp_fermat_pt_1024_input_t mmp_fermat_pt_1024;

    /** Fermat primality test for 1536-bit numbers  */
    icp_qat_fw_mmp_fermat_pt_1536_input_t mmp_fermat_pt_1536;

    /** Fermat primality test for 2048-bit numbers  */
    icp_qat_fw_mmp_fermat_pt_2048_input_t mmp_fermat_pt_2048;

    /** Fermat primality test for 3072-bit numbers  */
    icp_qat_fw_mmp_fermat_pt_3072_input_t mmp_fermat_pt_3072;

    /** Fermat primality test for 4096-bit numbers  */
    icp_qat_fw_mmp_fermat_pt_4096_input_t mmp_fermat_pt_4096;

    /** Miller-Rabin primality test for 160-bit numbers  */
    icp_qat_fw_mmp_mr_pt_160_input_t mmp_mr_pt_160;

    /** Miller-Rabin primality test for 512-bit numbers  */
    icp_qat_fw_mmp_mr_pt_512_input_t mmp_mr_pt_512;

    /** Miller-Rabin primality test for 768-bit numbers  */
    icp_qat_fw_mmp_mr_pt_768_input_t mmp_mr_pt_768;

    /** Miller-Rabin primality test for 1024-bit numbers  */
    icp_qat_fw_mmp_mr_pt_1024_input_t mmp_mr_pt_1024;

    /** Miller-Rabin primality test for 1536-bit numbers  */
    icp_qat_fw_mmp_mr_pt_1536_input_t mmp_mr_pt_1536;

    /** Miller-Rabin primality test for 2048-bit numbers  */
    icp_qat_fw_mmp_mr_pt_2048_input_t mmp_mr_pt_2048;

    /** Miller-Rabin primality test for 3072-bit numbers  */
    icp_qat_fw_mmp_mr_pt_3072_input_t mmp_mr_pt_3072;

    /** Miller-Rabin primality test for 4096-bit numbers  */
    icp_qat_fw_mmp_mr_pt_4096_input_t mmp_mr_pt_4096;

    /** Lucas primality test for 160-bit numbers  */
    icp_qat_fw_mmp_lucas_pt_160_input_t mmp_lucas_pt_160;

    /** Lucas primality test for 512-bit numbers  */
    icp_qat_fw_mmp_lucas_pt_512_input_t mmp_lucas_pt_512;

    /** Lucas primality test for 768-bit numbers  */
    icp_qat_fw_mmp_lucas_pt_768_input_t mmp_lucas_pt_768;

    /** Lucas primality test for 1024-bit numbers  */
    icp_qat_fw_mmp_lucas_pt_1024_input_t mmp_lucas_pt_1024;

    /** Lucas primality test for 1536-bit numbers  */
    icp_qat_fw_mmp_lucas_pt_1536_input_t mmp_lucas_pt_1536;

    /** Lucas primality test for 2048-bit numbers  */
    icp_qat_fw_mmp_lucas_pt_2048_input_t mmp_lucas_pt_2048;

    /** Lucas primality test for 3072-bit numbers  */
    icp_qat_fw_mmp_lucas_pt_3072_input_t mmp_lucas_pt_3072;

    /** Lucas primality test for 4096-bit numbers  */
    icp_qat_fw_mmp_lucas_pt_4096_input_t mmp_lucas_pt_4096;

    /** Modular exponentiation for numbers less than 512-bits  */
    icp_qat_fw_maths_modexp_l512_input_t maths_modexp_l512;

    /** Modular exponentiation for numbers less than 1024-bit  */
    icp_qat_fw_maths_modexp_l1024_input_t maths_modexp_l1024;

    /** Modular exponentiation for numbers less than 1536-bits  */
    icp_qat_fw_maths_modexp_l1536_input_t maths_modexp_l1536;

    /** Modular exponentiation for numbers less than 2048-bit  */
    icp_qat_fw_maths_modexp_l2048_input_t maths_modexp_l2048;

    /** Modular exponentiation for numbers less than 2560-bits  */
    icp_qat_fw_maths_modexp_l2560_input_t maths_modexp_l2560;

    /** Modular exponentiation for numbers less than 3072-bits  */
    icp_qat_fw_maths_modexp_l3072_input_t maths_modexp_l3072;

    /** Modular exponentiation for numbers less than 3584-bits  */
    icp_qat_fw_maths_modexp_l3584_input_t maths_modexp_l3584;

    /** Modular exponentiation for numbers less than 4096-bit  */
    icp_qat_fw_maths_modexp_l4096_input_t maths_modexp_l4096;

    /** Modular multiplicative inverse for numbers less than 128 bits  */
    icp_qat_fw_maths_modinv_odd_l128_input_t maths_modinv_odd_l128;

    /** Modular multiplicative inverse for numbers less than 192 bits  */
    icp_qat_fw_maths_modinv_odd_l192_input_t maths_modinv_odd_l192;

    /** Modular multiplicative inverse for numbers less than 256 bits  */
    icp_qat_fw_maths_modinv_odd_l256_input_t maths_modinv_odd_l256;

    /** Modular multiplicative inverse for numbers less than 384 bits  */
    icp_qat_fw_maths_modinv_odd_l384_input_t maths_modinv_odd_l384;

    /** Modular multiplicative inverse for numbers less than 512 bits  */
    icp_qat_fw_maths_modinv_odd_l512_input_t maths_modinv_odd_l512;

    /** Modular multiplicative inverse for numbers less than 768 bits  */
    icp_qat_fw_maths_modinv_odd_l768_input_t maths_modinv_odd_l768;

    /** Modular multiplicative inverse for numbers less than 1024 bits  */
    icp_qat_fw_maths_modinv_odd_l1024_input_t maths_modinv_odd_l1024;

    /** Modular multiplicative inverse for numbers less than 1536 bits  */
    icp_qat_fw_maths_modinv_odd_l1536_input_t maths_modinv_odd_l1536;

    /** Modular multiplicative inverse for numbers less than 2048 bits  */
    icp_qat_fw_maths_modinv_odd_l2048_input_t maths_modinv_odd_l2048;

    /** Modular multiplicative inverse for numbers less than 3072 bits  */
    icp_qat_fw_maths_modinv_odd_l3072_input_t maths_modinv_odd_l3072;

    /** Modular multiplicative inverse for numbers less than 4096 bits  */
    icp_qat_fw_maths_modinv_odd_l4096_input_t maths_modinv_odd_l4096;

    /** Modular multiplicative inverse for numbers less than 128 bits  */
    icp_qat_fw_maths_modinv_even_l128_input_t maths_modinv_even_l128;

    /** Modular multiplicative inverse for numbers less than 192 bits  */
    icp_qat_fw_maths_modinv_even_l192_input_t maths_modinv_even_l192;

    /** Modular multiplicative inverse for numbers less than 256 bits  */
    icp_qat_fw_maths_modinv_even_l256_input_t maths_modinv_even_l256;

    /** Modular multiplicative inverse for numbers less than 384 bits  */
    icp_qat_fw_maths_modinv_even_l384_input_t maths_modinv_even_l384;

    /** Modular multiplicative inverse for numbers less than 512 bits  */
    icp_qat_fw_maths_modinv_even_l512_input_t maths_modinv_even_l512;

    /** Modular multiplicative inverse for numbers less than 768 bits  */
    icp_qat_fw_maths_modinv_even_l768_input_t maths_modinv_even_l768;

    /** Modular multiplicative inverse for numbers less than 1024 bits  */
    icp_qat_fw_maths_modinv_even_l1024_input_t maths_modinv_even_l1024;

    /** Modular multiplicative inverse for numbers less than 1536 bits  */
    icp_qat_fw_maths_modinv_even_l1536_input_t maths_modinv_even_l1536;

    /** Modular multiplicative inverse for numbers less than 2048 bits  */
    icp_qat_fw_maths_modinv_even_l2048_input_t maths_modinv_even_l2048;

    /** Modular multiplicative inverse for numbers less than 3072 bits  */
    icp_qat_fw_maths_modinv_even_l3072_input_t maths_modinv_even_l3072;

    /** Modular multiplicative inverse for numbers less than 4096 bits  */
    icp_qat_fw_maths_modinv_even_l4096_input_t maths_modinv_even_l4096;
} icp_qat_fw_mmp_input_param_t;





/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular exponentiation base 2 for 160-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_MODEXP_G2_160.
 */
typedef struct icp_qat_fw_mmp_modexp_g2_160_output_s
{
    uint64_t r; /**< modular exponentiation result  &ge; 0 and &lt; m (3 qwords)*/
} icp_qat_fw_mmp_modexp_g2_160_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular exponentiation for 160-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_MODEXP_160.
 */
typedef struct icp_qat_fw_mmp_modexp_160_output_s
{
    uint64_t r; /**< modular exponentiation result  &ge; 0 and &lt; m (3 qwords)*/
} icp_qat_fw_mmp_modexp_160_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular exponentiation base 2 for 512-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_MODEXP_G2_512.
 */
typedef struct icp_qat_fw_mmp_modexp_g2_512_output_s
{
    uint64_t r; /**< modular exponentiation result  &ge; 0 and &lt; m (8 qwords)*/
} icp_qat_fw_mmp_modexp_g2_512_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular exponentiation for 512-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_MODEXP_512.
 */
typedef struct icp_qat_fw_mmp_modexp_512_output_s
{
    uint64_t r; /**< modular exponentiation result  &ge; 0 and &lt; m (8 qwords)*/
} icp_qat_fw_mmp_modexp_512_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular exponentiation base 2 for 768-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_MODEXP_G2_768.
 */
typedef struct icp_qat_fw_mmp_modexp_g2_768_output_s
{
    uint64_t r; /**< modular exponentiation result  &ge; 0 and &lt; m (12 qwords)*/
} icp_qat_fw_mmp_modexp_g2_768_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular exponentiation for 768-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_MODEXP_768.
 */
typedef struct icp_qat_fw_mmp_modexp_768_output_s
{
    uint64_t r; /**< modular exponentiation result  &ge; 0 and &lt; m (12 qwords)*/
} icp_qat_fw_mmp_modexp_768_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular exponentiation base 2 for 1024-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_MODEXP_G2_1024.
 */
typedef struct icp_qat_fw_mmp_modexp_g2_1024_output_s
{
    uint64_t r; /**< modular exponentiation result  &ge; 0 and &lt; m (16 qwords)*/
} icp_qat_fw_mmp_modexp_g2_1024_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular exponentiation for 1024-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_MODEXP_1024.
 */
typedef struct icp_qat_fw_mmp_modexp_1024_output_s
{
    uint64_t r; /**< modular exponentiation result  &ge; 0 and &lt; m (16 qwords)*/
} icp_qat_fw_mmp_modexp_1024_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular exponentiation base 2 for 1536-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_MODEXP_G2_1536.
 */
typedef struct icp_qat_fw_mmp_modexp_g2_1536_output_s
{
    uint64_t r; /**< modular exponentiation result  &ge; 0 and &lt; m (24 qwords)*/
} icp_qat_fw_mmp_modexp_g2_1536_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular exponentiation for 1536-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_MODEXP_1536.
 */
typedef struct icp_qat_fw_mmp_modexp_1536_output_s
{
    uint64_t r; /**< modular exponentiation result  &ge; 0 and &lt; m (24 qwords)*/
} icp_qat_fw_mmp_modexp_1536_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular exponentiation base 2 for 2048-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_MODEXP_G2_2048.
 */
typedef struct icp_qat_fw_mmp_modexp_g2_2048_output_s
{
    uint64_t r; /**< modular exponentiation result   &ge; 0 and &lt; m (32 qwords)*/
} icp_qat_fw_mmp_modexp_g2_2048_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular exponentiation for 2048-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_MODEXP_2048.
 */
typedef struct icp_qat_fw_mmp_modexp_2048_output_s
{
    uint64_t r; /**< modular exponentiation result   &ge; 0 and &lt; m (32 qwords)*/
} icp_qat_fw_mmp_modexp_2048_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular exponentiation base 2 for 3072-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_MODEXP_G2_3072.
 */
typedef struct icp_qat_fw_mmp_modexp_g2_3072_output_s
{
    uint64_t r; /**< modular exponentiation result   &ge; 0 and &lt; m (48 qwords)*/
} icp_qat_fw_mmp_modexp_g2_3072_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular exponentiation for 3072-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_MODEXP_3072.
 */
typedef struct icp_qat_fw_mmp_modexp_3072_output_s
{
    uint64_t r; /**< modular exponentiation result   &ge; 0 and &lt; m (48 qwords)*/
} icp_qat_fw_mmp_modexp_3072_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular exponentiation base 2 for 4096-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_MODEXP_G2_4096.
 */
typedef struct icp_qat_fw_mmp_modexp_g2_4096_output_s
{
    uint64_t r; /**< modular exponentiation result  &ge; 0 and &lt; m (64 qwords)*/
} icp_qat_fw_mmp_modexp_g2_4096_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular exponentiation for 4096-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_MODEXP_4096.
 */
typedef struct icp_qat_fw_mmp_modexp_4096_output_s
{
    uint64_t r; /**< modular exponentiation result   &ge; 0 and &lt; m (64 qwords)*/
} icp_qat_fw_mmp_modexp_4096_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for DSA parameter generation P ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_DSA_GEN_P_1024_160.
 */
typedef struct icp_qat_fw_mmp_dsa_gen_p_1024_160_output_s
{
    uint64_t p; /**< candidate for DSA parameter p  (16 qwords)*/
} icp_qat_fw_mmp_dsa_gen_p_1024_160_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for DSA key generation G ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_DSA_GEN_G_1024_160.
 */
typedef struct icp_qat_fw_mmp_dsa_gen_g_1024_160_output_s
{
    uint64_t g; /**< DSA parameter  (16 qwords)*/
} icp_qat_fw_mmp_dsa_gen_g_1024_160_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for DSA key generation Y ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_DSA_GEN_Y_1024_160.
 */
typedef struct icp_qat_fw_mmp_dsa_gen_y_1024_160_output_s
{
    uint64_t y; /**< DSA parameter (16 qwords)*/
} icp_qat_fw_mmp_dsa_gen_y_1024_160_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for DSA Sign R ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_DSA_SIGN_R_1024_160.
 */
typedef struct icp_qat_fw_mmp_dsa_sign_r_1024_160_output_s
{
    uint64_t r; /**< DSA 160-bits signature  (3 qwords)*/
} icp_qat_fw_mmp_dsa_sign_r_1024_160_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for DSA Sign S ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_DSA_SIGN_S_1024_160.
 */
typedef struct icp_qat_fw_mmp_dsa_sign_s_1024_160_output_s
{
    uint64_t s; /**< s DSA 160-bits signature  (3 qwords)*/
} icp_qat_fw_mmp_dsa_sign_s_1024_160_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for DSA Sign R S ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_DSA_SIGN_R_S_1024_160.
 */
typedef struct icp_qat_fw_mmp_dsa_sign_r_s_1024_160_output_s
{
    uint64_t r; /**< DSA 160-bits signature  (3 qwords)*/
    uint64_t s; /**< DSA 160-bits signature  (3 qwords)*/
} icp_qat_fw_mmp_dsa_sign_r_s_1024_160_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for DSA Verify V ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_DSA_VERIFY_1024_160.
 */
typedef struct icp_qat_fw_mmp_dsa_verify_1024_160_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_dsa_verify_1024_160_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for RSA 1024 key generation first form ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_RSA_KP1_1024.
 */
typedef struct icp_qat_fw_mmp_rsa_kp1_1024_output_s
{
    uint64_t n; /**< RSA key (16 qwords)*/
    uint64_t d; /**< RSA private key (first form) (16 qwords)*/
} icp_qat_fw_mmp_rsa_kp1_1024_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for RSA 1024 key generation second form ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_RSA_KP2_1024.
 */
typedef struct icp_qat_fw_mmp_rsa_kp2_1024_output_s
{
    uint64_t n; /**< RSA key (16 qwords)*/
    uint64_t d; /**< RSA private key (second form) (16 qwords)*/
    uint64_t dp; /**< RSA private key (second form) (8 qwords)*/
    uint64_t dq; /**< RSA private key (second form) (8 qwords)*/
    uint64_t qinv; /**< RSA private key (second form) (8 qwords)*/
} icp_qat_fw_mmp_rsa_kp2_1024_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for RSA 1024 Encryption ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_RSA_EP_1024.
 */
typedef struct icp_qat_fw_mmp_rsa_ep_1024_output_s
{
    uint64_t c; /**< cipher text representative, &lt; n (16 qwords)*/
} icp_qat_fw_mmp_rsa_ep_1024_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for RSA 1024 Decryption ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_RSA_DP1_1024.
 */
typedef struct icp_qat_fw_mmp_rsa_dp1_1024_output_s
{
    uint64_t m; /**< message representative, &lt; n (16 qwords)*/
} icp_qat_fw_mmp_rsa_dp1_1024_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for RSA 1024 Decryption with CRT ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_RSA_DP2_1024.
 */
typedef struct icp_qat_fw_mmp_rsa_dp2_1024_output_s
{
    uint64_t m; /**< message representative, &lt; (p*q) (16 qwords)*/
} icp_qat_fw_mmp_rsa_dp2_1024_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for RSA 1536 key generation first form ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_RSA_KP1_1536.
 */
typedef struct icp_qat_fw_mmp_rsa_kp1_1536_output_s
{
    uint64_t n; /**< RSA key (24 qwords)*/
    uint64_t d; /**< RSA private key (24 qwords)*/
} icp_qat_fw_mmp_rsa_kp1_1536_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for RSA 1536 key generation second form ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_RSA_KP2_1536.
 */
typedef struct icp_qat_fw_mmp_rsa_kp2_1536_output_s
{
    uint64_t n; /**< RSA key (24 qwords)*/
    uint64_t d; /**< RSA private key (24 qwords)*/
    uint64_t dp; /**< RSA private key (12 qwords)*/
    uint64_t dq; /**< RSA private key (12 qwords)*/
    uint64_t qinv; /**< RSA private key (12 qwords)*/
} icp_qat_fw_mmp_rsa_kp2_1536_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for RSA 1536 Encryption ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_RSA_EP_1536.
 */
typedef struct icp_qat_fw_mmp_rsa_ep_1536_output_s
{
    uint64_t c; /**< cipher text representative, &lt; n (24 qwords)*/
} icp_qat_fw_mmp_rsa_ep_1536_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for RSA 1536 Decryption ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_RSA_DP1_1536.
 */
typedef struct icp_qat_fw_mmp_rsa_dp1_1536_output_s
{
    uint64_t m; /**< message representative, &lt; n (24 qwords)*/
} icp_qat_fw_mmp_rsa_dp1_1536_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for RSA 1536 Decryption with CRT ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_RSA_DP2_1536.
 */
typedef struct icp_qat_fw_mmp_rsa_dp2_1536_output_s
{
    uint64_t m; /**< message representative, &lt; (p*q) (24 qwords)*/
} icp_qat_fw_mmp_rsa_dp2_1536_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for RSA 2048 key generation first form ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_RSA_KP1_2048.
 */
typedef struct icp_qat_fw_mmp_rsa_kp1_2048_output_s
{
    uint64_t n; /**< RSA key (32 qwords)*/
    uint64_t d; /**< RSA private key (32 qwords)*/
} icp_qat_fw_mmp_rsa_kp1_2048_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for RSA 2048 key generation second form ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_RSA_KP2_2048.
 */
typedef struct icp_qat_fw_mmp_rsa_kp2_2048_output_s
{
    uint64_t n; /**< RSA key (32 qwords)*/
    uint64_t d; /**< RSA private key (32 qwords)*/
    uint64_t dp; /**< RSA private key (16 qwords)*/
    uint64_t dq; /**< RSA private key (16 qwords)*/
    uint64_t qinv; /**< RSA private key (16 qwords)*/
} icp_qat_fw_mmp_rsa_kp2_2048_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for RSA 2048 Encryption ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_RSA_EP_2048.
 */
typedef struct icp_qat_fw_mmp_rsa_ep_2048_output_s
{
    uint64_t c; /**< cipher text representative, &lt; n (32 qwords)*/
} icp_qat_fw_mmp_rsa_ep_2048_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for RSA 2048 Decryption ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_RSA_DP1_2048.
 */
typedef struct icp_qat_fw_mmp_rsa_dp1_2048_output_s
{
    uint64_t m; /**< message representative, &lt; n (32 qwords)*/
} icp_qat_fw_mmp_rsa_dp1_2048_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for RSA 2048 Decryption with CRT ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_RSA_DP2_2048.
 */
typedef struct icp_qat_fw_mmp_rsa_dp2_2048_output_s
{
    uint64_t m; /**< message representative, &lt; (p*q) (32 qwords)*/
} icp_qat_fw_mmp_rsa_dp2_2048_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for RSA 3072 key generation first form ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_RSA_KP1_3072.
 */
typedef struct icp_qat_fw_mmp_rsa_kp1_3072_output_s
{
    uint64_t n; /**< RSA key (48 qwords)*/
    uint64_t d; /**< RSA private key (48 qwords)*/
} icp_qat_fw_mmp_rsa_kp1_3072_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for RSA 3072 key generation second form ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_RSA_KP2_3072.
 */
typedef struct icp_qat_fw_mmp_rsa_kp2_3072_output_s
{
    uint64_t n; /**< RSA key (48 qwords)*/
    uint64_t d; /**< RSA private key (48 qwords)*/
    uint64_t dp; /**< RSA private key (24 qwords)*/
    uint64_t dq; /**< RSA private key (24 qwords)*/
    uint64_t qinv; /**< RSA private key (24 qwords)*/
} icp_qat_fw_mmp_rsa_kp2_3072_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for RSA 3072 Encryption ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_RSA_EP_3072.
 */
typedef struct icp_qat_fw_mmp_rsa_ep_3072_output_s
{
    uint64_t c; /**< cipher text representative, &lt; n (48 qwords)*/
} icp_qat_fw_mmp_rsa_ep_3072_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for RSA 3072 Decryption ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_RSA_DP1_3072.
 */
typedef struct icp_qat_fw_mmp_rsa_dp1_3072_output_s
{
    uint64_t m; /**< message representative, &lt; n (48 qwords)*/
} icp_qat_fw_mmp_rsa_dp1_3072_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for RSA 3072 Decryption with CRT ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_RSA_DP2_3072.
 */
typedef struct icp_qat_fw_mmp_rsa_dp2_3072_output_s
{
    uint64_t m; /**< message representative, &lt; (p*q) (48 qwords)*/
} icp_qat_fw_mmp_rsa_dp2_3072_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for RSA 4096 key generation first form ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_RSA_KP1_4096.
 */
typedef struct icp_qat_fw_mmp_rsa_kp1_4096_output_s
{
    uint64_t n; /**< RSA key (64 qwords)*/
    uint64_t d; /**< RSA private key (64 qwords)*/
} icp_qat_fw_mmp_rsa_kp1_4096_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for RSA 4096 key generation second form ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_RSA_KP2_4096.
 */
typedef struct icp_qat_fw_mmp_rsa_kp2_4096_output_s
{
    uint64_t n; /**< RSA key (64 qwords)*/
    uint64_t d; /**< RSA private key (64 qwords)*/
    uint64_t dp; /**< RSA private key (32 qwords)*/
    uint64_t dq; /**< RSA private key (32 qwords)*/
    uint64_t qinv; /**< RSA private key (32 qwords)*/
} icp_qat_fw_mmp_rsa_kp2_4096_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for RSA 4096 Encryption ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_RSA_EP_4096.
 */
typedef struct icp_qat_fw_mmp_rsa_ep_4096_output_s
{
    uint64_t c; /**< cipher text representative, &lt; n (64 qwords)*/
} icp_qat_fw_mmp_rsa_ep_4096_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for RSA 4096 Decryption ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_RSA_DP1_4096.
 */
typedef struct icp_qat_fw_mmp_rsa_dp1_4096_output_s
{
    uint64_t m; /**< message representative, &lt; n (64 qwords)*/
} icp_qat_fw_mmp_rsa_dp1_4096_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for RSA 4096 Decryption with CRT ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_RSA_DP2_4096.
 */
typedef struct icp_qat_fw_mmp_rsa_dp2_4096_output_s
{
    uint64_t m; /**< message representative, &lt; (p*q) (64 qwords)*/
} icp_qat_fw_mmp_rsa_dp2_4096_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for GCD primality test for 192-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_GCD_PT_192.
 */
typedef struct icp_qat_fw_mmp_gcd_pt_192_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_gcd_pt_192_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for GCD primality test for 256-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_GCD_PT_256.
 */
typedef struct icp_qat_fw_mmp_gcd_pt_256_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_gcd_pt_256_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for GCD primality test for 384-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_GCD_PT_384.
 */
typedef struct icp_qat_fw_mmp_gcd_pt_384_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_gcd_pt_384_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for GCD primality test for 512-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_GCD_PT_512.
 */
typedef struct icp_qat_fw_mmp_gcd_pt_512_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_gcd_pt_512_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for GCD primality test for 768-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_GCD_PT_768.
 */
typedef struct icp_qat_fw_mmp_gcd_pt_768_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_gcd_pt_768_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for GCD primality test for 1024-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_GCD_PT_1024.
 */
typedef struct icp_qat_fw_mmp_gcd_pt_1024_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_gcd_pt_1024_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for GCD primality test for 1536-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_GCD_PT_1536.
 */
typedef struct icp_qat_fw_mmp_gcd_pt_1536_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_gcd_pt_1536_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for GCD primality test for 2048-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_GCD_PT_2048.
 */
typedef struct icp_qat_fw_mmp_gcd_pt_2048_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_gcd_pt_2048_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for GCD primality test for 3072-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_GCD_PT_3072.
 */
typedef struct icp_qat_fw_mmp_gcd_pt_3072_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_gcd_pt_3072_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for GCD primality test for 4096-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_GCD_PT_4096.
 */
typedef struct icp_qat_fw_mmp_gcd_pt_4096_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_gcd_pt_4096_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Fermat primality test for 160-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_FERMAT_PT_160.
 */
typedef struct icp_qat_fw_mmp_fermat_pt_160_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_fermat_pt_160_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Fermat primality test for 512-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_FERMAT_PT_512.
 */
typedef struct icp_qat_fw_mmp_fermat_pt_512_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_fermat_pt_512_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Fermat primality test for 768-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_FERMAT_PT_768.
 */
typedef struct icp_qat_fw_mmp_fermat_pt_768_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_fermat_pt_768_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Fermat primality test for 1024-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_FERMAT_PT_1024.
 */
typedef struct icp_qat_fw_mmp_fermat_pt_1024_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_fermat_pt_1024_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Fermat primality test for 1536-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_FERMAT_PT_1536.
 */
typedef struct icp_qat_fw_mmp_fermat_pt_1536_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_fermat_pt_1536_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Fermat primality test for 2048-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_FERMAT_PT_2048.
 */
typedef struct icp_qat_fw_mmp_fermat_pt_2048_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_fermat_pt_2048_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Fermat primality test for 3072-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_FERMAT_PT_3072.
 */
typedef struct icp_qat_fw_mmp_fermat_pt_3072_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_fermat_pt_3072_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Fermat primality test for 4096-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_FERMAT_PT_4096.
 */
typedef struct icp_qat_fw_mmp_fermat_pt_4096_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_fermat_pt_4096_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Miller-Rabin primality test for 160-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_MR_PT_160.
 */
typedef struct icp_qat_fw_mmp_mr_pt_160_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_mr_pt_160_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Miller-Rabin primality test for 512-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_MR_PT_512.
 */
typedef struct icp_qat_fw_mmp_mr_pt_512_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_mr_pt_512_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Miller-Rabin primality test for 768-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_MR_PT_768.
 */
typedef struct icp_qat_fw_mmp_mr_pt_768_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_mr_pt_768_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Miller-Rabin primality test for 1024-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_MR_PT_1024.
 */
typedef struct icp_qat_fw_mmp_mr_pt_1024_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_mr_pt_1024_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Miller-Rabin primality test for 1536-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_MR_PT_1536.
 */
typedef struct icp_qat_fw_mmp_mr_pt_1536_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_mr_pt_1536_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Miller-Rabin primality test for 2048-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_MR_PT_2048.
 */
typedef struct icp_qat_fw_mmp_mr_pt_2048_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_mr_pt_2048_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Miller-Rabin primality test for 3072-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_MR_PT_3072.
 */
typedef struct icp_qat_fw_mmp_mr_pt_3072_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_mr_pt_3072_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Miller-Rabin primality test for 4096-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_MR_PT_4096.
 */
typedef struct icp_qat_fw_mmp_mr_pt_4096_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_mr_pt_4096_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Lucas primality test for 160-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_LUCAS_PT_160.
 */
typedef struct icp_qat_fw_mmp_lucas_pt_160_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_lucas_pt_160_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Lucas primality test for 512-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_LUCAS_PT_512.
 */
typedef struct icp_qat_fw_mmp_lucas_pt_512_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_lucas_pt_512_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Lucas primality test for 768-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_LUCAS_PT_768.
 */
typedef struct icp_qat_fw_mmp_lucas_pt_768_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_lucas_pt_768_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Lucas primality test for 1024-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_LUCAS_PT_1024.
 */
typedef struct icp_qat_fw_mmp_lucas_pt_1024_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_lucas_pt_1024_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Lucas primality test for 1536-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_LUCAS_PT_1536.
 */
typedef struct icp_qat_fw_mmp_lucas_pt_1536_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_lucas_pt_1536_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Lucas primality test for 2048-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_LUCAS_PT_2048.
 */
typedef struct icp_qat_fw_mmp_lucas_pt_2048_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_lucas_pt_2048_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Lucas primality test for 3072-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_LUCAS_PT_3072.
 */
typedef struct icp_qat_fw_mmp_lucas_pt_3072_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_lucas_pt_3072_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Lucas primality test for 4096-bit numbers ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #PKE_LUCAS_PT_4096.
 */
typedef struct icp_qat_fw_mmp_lucas_pt_4096_output_s
{
    /* no output parameters */
} icp_qat_fw_mmp_lucas_pt_4096_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular exponentiation for numbers less than 512-bits ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #MATHS_MODEXP_L512.
 */
typedef struct icp_qat_fw_maths_modexp_l512_output_s
{
    uint64_t r; /**< modular exponentiation result  &ge; 0 and &lt; m (8 qwords)*/
} icp_qat_fw_maths_modexp_l512_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular exponentiation for numbers less than 1024-bit ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #MATHS_MODEXP_L1024.
 */
typedef struct icp_qat_fw_maths_modexp_l1024_output_s
{
    uint64_t r; /**< modular exponentiation result  &ge; 0 and &lt; m (16 qwords)*/
} icp_qat_fw_maths_modexp_l1024_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular exponentiation for numbers less than 1536-bits ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #MATHS_MODEXP_L1536.
 */
typedef struct icp_qat_fw_maths_modexp_l1536_output_s
{
    uint64_t r; /**< modular exponentiation result  &ge; 0 and &lt; m (24 qwords)*/
} icp_qat_fw_maths_modexp_l1536_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular exponentiation for numbers less than 2048-bit ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #MATHS_MODEXP_L2048.
 */
typedef struct icp_qat_fw_maths_modexp_l2048_output_s
{
    uint64_t r; /**< modular exponentiation result  &ge; 0 and &lt; m (32 qwords)*/
} icp_qat_fw_maths_modexp_l2048_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular exponentiation for numbers less than 2560-bits ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #MATHS_MODEXP_L2560.
 */
typedef struct icp_qat_fw_maths_modexp_l2560_output_s
{
    uint64_t r; /**< modular exponentiation result  &ge; 0 and &lt; m (40 qwords)*/
} icp_qat_fw_maths_modexp_l2560_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular exponentiation for numbers less than 3072-bits ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #MATHS_MODEXP_L3072.
 */
typedef struct icp_qat_fw_maths_modexp_l3072_output_s
{
    uint64_t r; /**< modular exponentiation result  &ge; 0 and &lt; m (48 qwords)*/
} icp_qat_fw_maths_modexp_l3072_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular exponentiation for numbers less than 3584-bits ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #MATHS_MODEXP_L3584.
 */
typedef struct icp_qat_fw_maths_modexp_l3584_output_s
{
    uint64_t r; /**< modular exponentiation result  &ge; 0 and &lt; m (56 qwords)*/
} icp_qat_fw_maths_modexp_l3584_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular exponentiation for numbers less than 4096-bit ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #MATHS_MODEXP_L4096.
 */
typedef struct icp_qat_fw_maths_modexp_l4096_output_s
{
    uint64_t r; /**< modular exponentiation result   &ge; 0 and &lt; m (64 qwords)*/
} icp_qat_fw_maths_modexp_l4096_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular multiplicative inverse for numbers less than 128 bits ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #MATHS_MODINV_ODD_L128.
 */
typedef struct icp_qat_fw_maths_modinv_odd_l128_output_s
{
    uint64_t c; /**< modular multiplicative inverse of a, &gt; 0 and &lt; b (2 qwords)*/
} icp_qat_fw_maths_modinv_odd_l128_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular multiplicative inverse for numbers less than 192 bits ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #MATHS_MODINV_ODD_L192.
 */
typedef struct icp_qat_fw_maths_modinv_odd_l192_output_s
{
    uint64_t c; /**< modular multiplicative inverse of a, &gt; 0 and &lt; b (3 qwords)*/
} icp_qat_fw_maths_modinv_odd_l192_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular multiplicative inverse for numbers less than 256 bits ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #MATHS_MODINV_ODD_L256.
 */
typedef struct icp_qat_fw_maths_modinv_odd_l256_output_s
{
    uint64_t c; /**< modular multiplicative inverse of a, &gt; 0 and &lt; b (4 qwords)*/
} icp_qat_fw_maths_modinv_odd_l256_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular multiplicative inverse for numbers less than 384 bits ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #MATHS_MODINV_ODD_L384.
 */
typedef struct icp_qat_fw_maths_modinv_odd_l384_output_s
{
    uint64_t c; /**< modular multiplicative inverse of a, &gt; 0 and &lt; b (6 qwords)*/
} icp_qat_fw_maths_modinv_odd_l384_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular multiplicative inverse for numbers less than 512 bits ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #MATHS_MODINV_ODD_L512.
 */
typedef struct icp_qat_fw_maths_modinv_odd_l512_output_s
{
    uint64_t c; /**< modular multiplicative inverse of a, &gt; 0 and &lt; b (8 qwords)*/
} icp_qat_fw_maths_modinv_odd_l512_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular multiplicative inverse for numbers less than 768 bits ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #MATHS_MODINV_ODD_L768.
 */
typedef struct icp_qat_fw_maths_modinv_odd_l768_output_s
{
    uint64_t c; /**< modular multiplicative inverse of a, &gt; 0 and &lt; b (12 qwords)*/
} icp_qat_fw_maths_modinv_odd_l768_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular multiplicative inverse for numbers less than 1024 bits ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #MATHS_MODINV_ODD_L1024.
 */
typedef struct icp_qat_fw_maths_modinv_odd_l1024_output_s
{
    uint64_t c; /**< modular multiplicative inverse of a, &gt; 0 and &lt; b (16 qwords)*/
} icp_qat_fw_maths_modinv_odd_l1024_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular multiplicative inverse for numbers less than 1536 bits ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #MATHS_MODINV_ODD_L1536.
 */
typedef struct icp_qat_fw_maths_modinv_odd_l1536_output_s
{
    uint64_t c; /**< modular multiplicative inverse of a, &gt; 0 and &lt; b (24 qwords)*/
} icp_qat_fw_maths_modinv_odd_l1536_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular multiplicative inverse for numbers less than 2048 bits ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #MATHS_MODINV_ODD_L2048.
 */
typedef struct icp_qat_fw_maths_modinv_odd_l2048_output_s
{
    uint64_t c; /**< modular multiplicative inverse of a, &gt; 0 and &lt; b (32 qwords)*/
} icp_qat_fw_maths_modinv_odd_l2048_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular multiplicative inverse for numbers less than 3072 bits ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #MATHS_MODINV_ODD_L3072.
 */
typedef struct icp_qat_fw_maths_modinv_odd_l3072_output_s
{
    uint64_t c; /**< modular multiplicative inverse of a, &gt; 0 and &lt; b (48 qwords)*/
} icp_qat_fw_maths_modinv_odd_l3072_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular multiplicative inverse for numbers less than 4096 bits ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #MATHS_MODINV_ODD_L4096.
 */
typedef struct icp_qat_fw_maths_modinv_odd_l4096_output_s
{
    uint64_t c; /**< modular multiplicative inverse of a, &gt; 0 and &lt; b (64 qwords)*/
} icp_qat_fw_maths_modinv_odd_l4096_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular multiplicative inverse for numbers less than 128 bits ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #MATHS_MODINV_EVEN_L128.
 */
typedef struct icp_qat_fw_maths_modinv_even_l128_output_s
{
    uint64_t c; /**< modular multiplicative inverse of a, &gt; 0 and &lt; b (2 qwords)*/
} icp_qat_fw_maths_modinv_even_l128_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular multiplicative inverse for numbers less than 192 bits ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #MATHS_MODINV_EVEN_L192.
 */
typedef struct icp_qat_fw_maths_modinv_even_l192_output_s
{
    uint64_t c; /**< modular multiplicative inverse of a, &gt; 0 and &lt; b (3 qwords)*/
} icp_qat_fw_maths_modinv_even_l192_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular multiplicative inverse for numbers less than 256 bits ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #MATHS_MODINV_EVEN_L256.
 */
typedef struct icp_qat_fw_maths_modinv_even_l256_output_s
{
    uint64_t c; /**< modular multiplicative inverse of a, &gt; 0 and &lt; b (4 qwords)*/
} icp_qat_fw_maths_modinv_even_l256_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular multiplicative inverse for numbers less than 384 bits ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #MATHS_MODINV_EVEN_L384.
 */
typedef struct icp_qat_fw_maths_modinv_even_l384_output_s
{
    uint64_t c; /**< modular multiplicative inverse of a, &gt; 0 and &lt; b (6 qwords)*/
} icp_qat_fw_maths_modinv_even_l384_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular multiplicative inverse for numbers less than 512 bits ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #MATHS_MODINV_EVEN_L512.
 */
typedef struct icp_qat_fw_maths_modinv_even_l512_output_s
{
    uint64_t c; /**< modular multiplicative inverse of a, &gt; 0 and &lt; b (8 qwords)*/
} icp_qat_fw_maths_modinv_even_l512_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular multiplicative inverse for numbers less than 768 bits ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #MATHS_MODINV_EVEN_L768.
 */
typedef struct icp_qat_fw_maths_modinv_even_l768_output_s
{
    uint64_t c; /**< modular multiplicative inverse of a, &gt; 0 and &lt; b (12 qwords)*/
} icp_qat_fw_maths_modinv_even_l768_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular multiplicative inverse for numbers less than 1024 bits ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #MATHS_MODINV_EVEN_L1024.
 */
typedef struct icp_qat_fw_maths_modinv_even_l1024_output_s
{
    uint64_t c; /**< modular multiplicative inverse of a, &gt; 0 and &lt; b (16 qwords)*/
} icp_qat_fw_maths_modinv_even_l1024_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular multiplicative inverse for numbers less than 1536 bits ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #MATHS_MODINV_EVEN_L1536.
 */
typedef struct icp_qat_fw_maths_modinv_even_l1536_output_s
{
    uint64_t c; /**< modular multiplicative inverse of a, &gt; 0 and &lt; b (24 qwords)*/
} icp_qat_fw_maths_modinv_even_l1536_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular multiplicative inverse for numbers less than 2048 bits ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #MATHS_MODINV_EVEN_L2048.
 */
typedef struct icp_qat_fw_maths_modinv_even_l2048_output_s
{
    uint64_t c; /**< modular multiplicative inverse of a, &gt; 0 and &lt; b (32 qwords)*/
} icp_qat_fw_maths_modinv_even_l2048_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular multiplicative inverse for numbers less than 3072 bits ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #MATHS_MODINV_EVEN_L3072.
 */
typedef struct icp_qat_fw_maths_modinv_even_l3072_output_s
{
    uint64_t c; /**< modular multiplicative inverse of a, &gt; 0 and &lt; b (48 qwords)*/
} icp_qat_fw_maths_modinv_even_l3072_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    Output parameter list for Modular multiplicative inverse for numbers less than 4096 bits ,
 *      to be used when icp_qat_fw_pke_response_s::functionalityId is #MATHS_MODINV_EVEN_L4096.
 */
typedef struct icp_qat_fw_maths_modinv_even_l4096_output_s
{
    uint64_t c; /**< modular multiplicative inverse of a, &gt; 0 and &lt; b (64 qwords)*/
} icp_qat_fw_maths_modinv_even_l4096_output_t;



/**
 * @ingroup icp_qat_fw_mmp
 * @brief
 *    MMP output parameters
 */
typedef union icp_qat_fw_mmp_output_param_u
{
    /** Generic parameter structure : All members of this wrapper structure
     * are pointers to large integers.
     */
    uint64_t flat_array[ICP_QAT_FW_PKE_OUTPUT_COUNT_MAX];

    /** Modular exponentiation base 2 for 160-bit numbers  */
    icp_qat_fw_mmp_modexp_g2_160_output_t mmp_modexp_g2_160;

    /** Modular exponentiation for 160-bit numbers  */
    icp_qat_fw_mmp_modexp_160_output_t mmp_modexp_160;

    /** Modular exponentiation base 2 for 512-bit numbers  */
    icp_qat_fw_mmp_modexp_g2_512_output_t mmp_modexp_g2_512;

    /** Modular exponentiation for 512-bit numbers  */
    icp_qat_fw_mmp_modexp_512_output_t mmp_modexp_512;

    /** Modular exponentiation base 2 for 768-bit numbers  */
    icp_qat_fw_mmp_modexp_g2_768_output_t mmp_modexp_g2_768;

    /** Modular exponentiation for 768-bit numbers  */
    icp_qat_fw_mmp_modexp_768_output_t mmp_modexp_768;

    /** Modular exponentiation base 2 for 1024-bit numbers  */
    icp_qat_fw_mmp_modexp_g2_1024_output_t mmp_modexp_g2_1024;

    /** Modular exponentiation for 1024-bit numbers  */
    icp_qat_fw_mmp_modexp_1024_output_t mmp_modexp_1024;

    /** Modular exponentiation base 2 for 1536-bit numbers  */
    icp_qat_fw_mmp_modexp_g2_1536_output_t mmp_modexp_g2_1536;

    /** Modular exponentiation for 1536-bit numbers  */
    icp_qat_fw_mmp_modexp_1536_output_t mmp_modexp_1536;

    /** Modular exponentiation base 2 for 2048-bit numbers  */
    icp_qat_fw_mmp_modexp_g2_2048_output_t mmp_modexp_g2_2048;

    /** Modular exponentiation for 2048-bit numbers  */
    icp_qat_fw_mmp_modexp_2048_output_t mmp_modexp_2048;

    /** Modular exponentiation base 2 for 3072-bit numbers  */
    icp_qat_fw_mmp_modexp_g2_3072_output_t mmp_modexp_g2_3072;

    /** Modular exponentiation for 3072-bit numbers  */
    icp_qat_fw_mmp_modexp_3072_output_t mmp_modexp_3072;

    /** Modular exponentiation base 2 for 4096-bit numbers  */
    icp_qat_fw_mmp_modexp_g2_4096_output_t mmp_modexp_g2_4096;

    /** Modular exponentiation for 4096-bit numbers  */
    icp_qat_fw_mmp_modexp_4096_output_t mmp_modexp_4096;

    /** DSA parameter generation P  */
    icp_qat_fw_mmp_dsa_gen_p_1024_160_output_t mmp_dsa_gen_p_1024_160;

    /** DSA key generation G  */
    icp_qat_fw_mmp_dsa_gen_g_1024_160_output_t mmp_dsa_gen_g_1024_160;

    /** DSA key generation Y  */
    icp_qat_fw_mmp_dsa_gen_y_1024_160_output_t mmp_dsa_gen_y_1024_160;

    /** DSA Sign R  */
    icp_qat_fw_mmp_dsa_sign_r_1024_160_output_t mmp_dsa_sign_r_1024_160;

    /** DSA Sign S  */
    icp_qat_fw_mmp_dsa_sign_s_1024_160_output_t mmp_dsa_sign_s_1024_160;

    /** DSA Sign R S  */
    icp_qat_fw_mmp_dsa_sign_r_s_1024_160_output_t mmp_dsa_sign_r_s_1024_160;

    /** DSA Verify V  */
    icp_qat_fw_mmp_dsa_verify_1024_160_output_t mmp_dsa_verify_1024_160;

    /** RSA 1024 key generation first form  */
    icp_qat_fw_mmp_rsa_kp1_1024_output_t mmp_rsa_kp1_1024;

    /** RSA 1024 key generation second form  */
    icp_qat_fw_mmp_rsa_kp2_1024_output_t mmp_rsa_kp2_1024;

    /** RSA 1024 Encryption  */
    icp_qat_fw_mmp_rsa_ep_1024_output_t mmp_rsa_ep_1024;

    /** RSA 1024 Decryption  */
    icp_qat_fw_mmp_rsa_dp1_1024_output_t mmp_rsa_dp1_1024;

    /** RSA 1024 Decryption with CRT  */
    icp_qat_fw_mmp_rsa_dp2_1024_output_t mmp_rsa_dp2_1024;

    /** RSA 1536 key generation first form  */
    icp_qat_fw_mmp_rsa_kp1_1536_output_t mmp_rsa_kp1_1536;

    /** RSA 1536 key generation second form  */
    icp_qat_fw_mmp_rsa_kp2_1536_output_t mmp_rsa_kp2_1536;

    /** RSA 1536 Encryption  */
    icp_qat_fw_mmp_rsa_ep_1536_output_t mmp_rsa_ep_1536;

    /** RSA 1536 Decryption  */
    icp_qat_fw_mmp_rsa_dp1_1536_output_t mmp_rsa_dp1_1536;

    /** RSA 1536 Decryption with CRT  */
    icp_qat_fw_mmp_rsa_dp2_1536_output_t mmp_rsa_dp2_1536;

    /** RSA 2048 key generation first form  */
    icp_qat_fw_mmp_rsa_kp1_2048_output_t mmp_rsa_kp1_2048;

    /** RSA 2048 key generation second form  */
    icp_qat_fw_mmp_rsa_kp2_2048_output_t mmp_rsa_kp2_2048;

    /** RSA 2048 Encryption  */
    icp_qat_fw_mmp_rsa_ep_2048_output_t mmp_rsa_ep_2048;

    /** RSA 2048 Decryption  */
    icp_qat_fw_mmp_rsa_dp1_2048_output_t mmp_rsa_dp1_2048;

    /** RSA 2048 Decryption with CRT  */
    icp_qat_fw_mmp_rsa_dp2_2048_output_t mmp_rsa_dp2_2048;

    /** RSA 3072 key generation first form  */
    icp_qat_fw_mmp_rsa_kp1_3072_output_t mmp_rsa_kp1_3072;

    /** RSA 3072 key generation second form  */
    icp_qat_fw_mmp_rsa_kp2_3072_output_t mmp_rsa_kp2_3072;

    /** RSA 3072 Encryption  */
    icp_qat_fw_mmp_rsa_ep_3072_output_t mmp_rsa_ep_3072;

    /** RSA 3072 Decryption  */
    icp_qat_fw_mmp_rsa_dp1_3072_output_t mmp_rsa_dp1_3072;

    /** RSA 3072 Decryption with CRT  */
    icp_qat_fw_mmp_rsa_dp2_3072_output_t mmp_rsa_dp2_3072;

    /** RSA 4096 key generation first form  */
    icp_qat_fw_mmp_rsa_kp1_4096_output_t mmp_rsa_kp1_4096;

    /** RSA 4096 key generation second form  */
    icp_qat_fw_mmp_rsa_kp2_4096_output_t mmp_rsa_kp2_4096;

    /** RSA 4096 Encryption  */
    icp_qat_fw_mmp_rsa_ep_4096_output_t mmp_rsa_ep_4096;

    /** RSA 4096 Decryption  */
    icp_qat_fw_mmp_rsa_dp1_4096_output_t mmp_rsa_dp1_4096;

    /** RSA 4096 Decryption with CRT  */
    icp_qat_fw_mmp_rsa_dp2_4096_output_t mmp_rsa_dp2_4096;

    /** GCD primality test for 192-bit numbers  */
    icp_qat_fw_mmp_gcd_pt_192_output_t mmp_gcd_pt_192;

    /** GCD primality test for 256-bit numbers  */
    icp_qat_fw_mmp_gcd_pt_256_output_t mmp_gcd_pt_256;

    /** GCD primality test for 384-bit numbers  */
    icp_qat_fw_mmp_gcd_pt_384_output_t mmp_gcd_pt_384;

    /** GCD primality test for 512-bit numbers  */
    icp_qat_fw_mmp_gcd_pt_512_output_t mmp_gcd_pt_512;

    /** GCD primality test for 768-bit numbers  */
    icp_qat_fw_mmp_gcd_pt_768_output_t mmp_gcd_pt_768;

    /** GCD primality test for 1024-bit numbers  */
    icp_qat_fw_mmp_gcd_pt_1024_output_t mmp_gcd_pt_1024;

    /** GCD primality test for 1536-bit numbers  */
    icp_qat_fw_mmp_gcd_pt_1536_output_t mmp_gcd_pt_1536;

    /** GCD primality test for 2048-bit numbers  */
    icp_qat_fw_mmp_gcd_pt_2048_output_t mmp_gcd_pt_2048;

    /** GCD primality test for 3072-bit numbers  */
    icp_qat_fw_mmp_gcd_pt_3072_output_t mmp_gcd_pt_3072;

    /** GCD primality test for 4096-bit numbers  */
    icp_qat_fw_mmp_gcd_pt_4096_output_t mmp_gcd_pt_4096;

    /** Fermat primality test for 160-bit numbers  */
    icp_qat_fw_mmp_fermat_pt_160_output_t mmp_fermat_pt_160;

    /** Fermat primality test for 512-bit numbers  */
    icp_qat_fw_mmp_fermat_pt_512_output_t mmp_fermat_pt_512;

    /** Fermat primality test for 768-bit numbers  */
    icp_qat_fw_mmp_fermat_pt_768_output_t mmp_fermat_pt_768;

    /** Fermat primality test for 1024-bit numbers  */
    icp_qat_fw_mmp_fermat_pt_1024_output_t mmp_fermat_pt_1024;

    /** Fermat primality test for 1536-bit numbers  */
    icp_qat_fw_mmp_fermat_pt_1536_output_t mmp_fermat_pt_1536;

    /** Fermat primality test for 2048-bit numbers  */
    icp_qat_fw_mmp_fermat_pt_2048_output_t mmp_fermat_pt_2048;

    /** Fermat primality test for 3072-bit numbers  */
    icp_qat_fw_mmp_fermat_pt_3072_output_t mmp_fermat_pt_3072;

    /** Fermat primality test for 4096-bit numbers  */
    icp_qat_fw_mmp_fermat_pt_4096_output_t mmp_fermat_pt_4096;

    /** Miller-Rabin primality test for 160-bit numbers  */
    icp_qat_fw_mmp_mr_pt_160_output_t mmp_mr_pt_160;

    /** Miller-Rabin primality test for 512-bit numbers  */
    icp_qat_fw_mmp_mr_pt_512_output_t mmp_mr_pt_512;

    /** Miller-Rabin primality test for 768-bit numbers  */
    icp_qat_fw_mmp_mr_pt_768_output_t mmp_mr_pt_768;

    /** Miller-Rabin primality test for 1024-bit numbers  */
    icp_qat_fw_mmp_mr_pt_1024_output_t mmp_mr_pt_1024;

    /** Miller-Rabin primality test for 1536-bit numbers  */
    icp_qat_fw_mmp_mr_pt_1536_output_t mmp_mr_pt_1536;

    /** Miller-Rabin primality test for 2048-bit numbers  */
    icp_qat_fw_mmp_mr_pt_2048_output_t mmp_mr_pt_2048;

    /** Miller-Rabin primality test for 3072-bit numbers  */
    icp_qat_fw_mmp_mr_pt_3072_output_t mmp_mr_pt_3072;

    /** Miller-Rabin primality test for 4096-bit numbers  */
    icp_qat_fw_mmp_mr_pt_4096_output_t mmp_mr_pt_4096;

    /** Lucas primality test for 160-bit numbers  */
    icp_qat_fw_mmp_lucas_pt_160_output_t mmp_lucas_pt_160;

    /** Lucas primality test for 512-bit numbers  */
    icp_qat_fw_mmp_lucas_pt_512_output_t mmp_lucas_pt_512;

    /** Lucas primality test for 768-bit numbers  */
    icp_qat_fw_mmp_lucas_pt_768_output_t mmp_lucas_pt_768;

    /** Lucas primality test for 1024-bit numbers  */
    icp_qat_fw_mmp_lucas_pt_1024_output_t mmp_lucas_pt_1024;

    /** Lucas primality test for 1536-bit numbers  */
    icp_qat_fw_mmp_lucas_pt_1536_output_t mmp_lucas_pt_1536;

    /** Lucas primality test for 2048-bit numbers  */
    icp_qat_fw_mmp_lucas_pt_2048_output_t mmp_lucas_pt_2048;

    /** Lucas primality test for 3072-bit numbers  */
    icp_qat_fw_mmp_lucas_pt_3072_output_t mmp_lucas_pt_3072;

    /** Lucas primality test for 4096-bit numbers  */
    icp_qat_fw_mmp_lucas_pt_4096_output_t mmp_lucas_pt_4096;

    /** Modular exponentiation for numbers less than 512-bits  */
    icp_qat_fw_maths_modexp_l512_output_t maths_modexp_l512;

    /** Modular exponentiation for numbers less than 1024-bit  */
    icp_qat_fw_maths_modexp_l1024_output_t maths_modexp_l1024;

    /** Modular exponentiation for numbers less than 1536-bits  */
    icp_qat_fw_maths_modexp_l1536_output_t maths_modexp_l1536;

    /** Modular exponentiation for numbers less than 2048-bit  */
    icp_qat_fw_maths_modexp_l2048_output_t maths_modexp_l2048;

    /** Modular exponentiation for numbers less than 2560-bits  */
    icp_qat_fw_maths_modexp_l2560_output_t maths_modexp_l2560;

    /** Modular exponentiation for numbers less than 3072-bits  */
    icp_qat_fw_maths_modexp_l3072_output_t maths_modexp_l3072;

    /** Modular exponentiation for numbers less than 3584-bits  */
    icp_qat_fw_maths_modexp_l3584_output_t maths_modexp_l3584;

    /** Modular exponentiation for numbers less than 4096-bit  */
    icp_qat_fw_maths_modexp_l4096_output_t maths_modexp_l4096;

    /** Modular multiplicative inverse for numbers less than 128 bits  */
    icp_qat_fw_maths_modinv_odd_l128_output_t maths_modinv_odd_l128;

    /** Modular multiplicative inverse for numbers less than 192 bits  */
    icp_qat_fw_maths_modinv_odd_l192_output_t maths_modinv_odd_l192;

    /** Modular multiplicative inverse for numbers less than 256 bits  */
    icp_qat_fw_maths_modinv_odd_l256_output_t maths_modinv_odd_l256;

    /** Modular multiplicative inverse for numbers less than 384 bits  */
    icp_qat_fw_maths_modinv_odd_l384_output_t maths_modinv_odd_l384;

    /** Modular multiplicative inverse for numbers less than 512 bits  */
    icp_qat_fw_maths_modinv_odd_l512_output_t maths_modinv_odd_l512;

    /** Modular multiplicative inverse for numbers less than 768 bits  */
    icp_qat_fw_maths_modinv_odd_l768_output_t maths_modinv_odd_l768;

    /** Modular multiplicative inverse for numbers less than 1024 bits  */
    icp_qat_fw_maths_modinv_odd_l1024_output_t maths_modinv_odd_l1024;

    /** Modular multiplicative inverse for numbers less than 1536 bits  */
    icp_qat_fw_maths_modinv_odd_l1536_output_t maths_modinv_odd_l1536;

    /** Modular multiplicative inverse for numbers less than 2048 bits  */
    icp_qat_fw_maths_modinv_odd_l2048_output_t maths_modinv_odd_l2048;

    /** Modular multiplicative inverse for numbers less than 3072 bits  */
    icp_qat_fw_maths_modinv_odd_l3072_output_t maths_modinv_odd_l3072;

    /** Modular multiplicative inverse for numbers less than 4096 bits  */
    icp_qat_fw_maths_modinv_odd_l4096_output_t maths_modinv_odd_l4096;

    /** Modular multiplicative inverse for numbers less than 128 bits  */
    icp_qat_fw_maths_modinv_even_l128_output_t maths_modinv_even_l128;

    /** Modular multiplicative inverse for numbers less than 192 bits  */
    icp_qat_fw_maths_modinv_even_l192_output_t maths_modinv_even_l192;

    /** Modular multiplicative inverse for numbers less than 256 bits  */
    icp_qat_fw_maths_modinv_even_l256_output_t maths_modinv_even_l256;

    /** Modular multiplicative inverse for numbers less than 384 bits  */
    icp_qat_fw_maths_modinv_even_l384_output_t maths_modinv_even_l384;

    /** Modular multiplicative inverse for numbers less than 512 bits  */
    icp_qat_fw_maths_modinv_even_l512_output_t maths_modinv_even_l512;

    /** Modular multiplicative inverse for numbers less than 768 bits  */
    icp_qat_fw_maths_modinv_even_l768_output_t maths_modinv_even_l768;

    /** Modular multiplicative inverse for numbers less than 1024 bits  */
    icp_qat_fw_maths_modinv_even_l1024_output_t maths_modinv_even_l1024;

    /** Modular multiplicative inverse for numbers less than 1536 bits  */
    icp_qat_fw_maths_modinv_even_l1536_output_t maths_modinv_even_l1536;

    /** Modular multiplicative inverse for numbers less than 2048 bits  */
    icp_qat_fw_maths_modinv_even_l2048_output_t maths_modinv_even_l2048;

    /** Modular multiplicative inverse for numbers less than 3072 bits  */
    icp_qat_fw_maths_modinv_even_l3072_output_t maths_modinv_even_l3072;

    /** Modular multiplicative inverse for numbers less than 4096 bits  */
    icp_qat_fw_maths_modinv_even_l4096_output_t maths_modinv_even_l4096;
} icp_qat_fw_mmp_output_param_t;



#endif /* __ICP_QAT_FW_MMP__ */


/* --- (Automatically generated (build v. 2.33), do not modify manually) --- */

/* --- end of file --- */



