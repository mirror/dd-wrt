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
 * @file icp_qat_fw_mmp_ids.h
 * @ingroup icp_qat_fw_mmp
 * $Revision: 0.1 $
 * @brief
 *      This file documents the external interfaces that the QAT FW running
 *      on the QAT Acceleration Engine provides to clients wanting to
 *      accelerate crypto assymetric applications
 */


#ifndef __ICP_QAT_FW_MMP_IDS__
#define __ICP_QAT_FW_MMP_IDS__


#define PKE_MODEXP_G2_160                        0x100b0ff0
   /**< Functionality ID for Modular exponentiation base 2 for 160-bit numbers  
    * @li 2 input parameters : @link icp_qat_fw_mmp_modexp_g2_160_input_s::e e @endlink @link icp_qat_fw_mmp_modexp_g2_160_input_s::m m @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_modexp_g2_160_output_s::r r @endlink 
    */
#define PKE_MODEXP_160                           0x110c0ffb
   /**< Functionality ID for Modular exponentiation for 160-bit numbers  
    * @li 3 input parameters : @link icp_qat_fw_mmp_modexp_160_input_s::g g @endlink @link icp_qat_fw_mmp_modexp_160_input_s::e e @endlink @link icp_qat_fw_mmp_modexp_160_input_s::m m @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_modexp_160_output_s::r r @endlink 
    */
#define PKE_MODEXP_G2_512                        0x150b1007
   /**< Functionality ID for Modular exponentiation base 2 for 512-bit numbers  
    * @li 2 input parameters : @link icp_qat_fw_mmp_modexp_g2_512_input_s::e e @endlink @link icp_qat_fw_mmp_modexp_g2_512_input_s::m m @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_modexp_g2_512_output_s::r r @endlink 
    */
#define PKE_MODEXP_512                           0x190c1012
   /**< Functionality ID for Modular exponentiation for 512-bit numbers  
    * @li 3 input parameters : @link icp_qat_fw_mmp_modexp_512_input_s::g g @endlink @link icp_qat_fw_mmp_modexp_512_input_s::e e @endlink @link icp_qat_fw_mmp_modexp_512_input_s::m m @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_modexp_512_output_s::r r @endlink 
    */
#define PKE_MODEXP_G2_768                        0x1f0b101e
   /**< Functionality ID for Modular exponentiation base 2 for 768-bit numbers  
    * @li 2 input parameters : @link icp_qat_fw_mmp_modexp_g2_768_input_s::e e @endlink @link icp_qat_fw_mmp_modexp_g2_768_input_s::m m @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_modexp_g2_768_output_s::r r @endlink 
    */
#define PKE_MODEXP_768                           0x250c1029
   /**< Functionality ID for Modular exponentiation for 768-bit numbers  
    * @li 3 input parameters : @link icp_qat_fw_mmp_modexp_768_input_s::g g @endlink @link icp_qat_fw_mmp_modexp_768_input_s::e e @endlink @link icp_qat_fw_mmp_modexp_768_input_s::m m @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_modexp_768_output_s::r r @endlink 
    */
#define PKE_MODEXP_G2_1024                       0x250b1035
   /**< Functionality ID for Modular exponentiation base 2 for 1024-bit numbers  
    * @li 2 input parameters : @link icp_qat_fw_mmp_modexp_g2_1024_input_s::e e @endlink @link icp_qat_fw_mmp_modexp_g2_1024_input_s::m m @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_modexp_g2_1024_output_s::r r @endlink 
    */
#define PKE_MODEXP_1024                          0x2d0c1040
   /**< Functionality ID for Modular exponentiation for 1024-bit numbers  
    * @li 3 input parameters : @link icp_qat_fw_mmp_modexp_1024_input_s::g g @endlink @link icp_qat_fw_mmp_modexp_1024_input_s::e e @endlink @link icp_qat_fw_mmp_modexp_1024_input_s::m m @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_modexp_1024_output_s::r r @endlink 
    */
#define PKE_MODEXP_G2_1536                       0x310b104c
   /**< Functionality ID for Modular exponentiation base 2 for 1536-bit numbers  
    * @li 2 input parameters : @link icp_qat_fw_mmp_modexp_g2_1536_input_s::e e @endlink @link icp_qat_fw_mmp_modexp_g2_1536_input_s::m m @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_modexp_g2_1536_output_s::r r @endlink 
    */
#define PKE_MODEXP_1536                          0x3d0c1057
   /**< Functionality ID for Modular exponentiation for 1536-bit numbers  
    * @li 3 input parameters : @link icp_qat_fw_mmp_modexp_1536_input_s::g g @endlink @link icp_qat_fw_mmp_modexp_1536_input_s::e e @endlink @link icp_qat_fw_mmp_modexp_1536_input_s::m m @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_modexp_1536_output_s::r r @endlink 
    */
#define PKE_MODEXP_G2_2048                       0x410b1063
   /**< Functionality ID for Modular exponentiation base 2 for 2048-bit numbers  
    * @li 2 input parameters : @link icp_qat_fw_mmp_modexp_g2_2048_input_s::e e @endlink @link icp_qat_fw_mmp_modexp_g2_2048_input_s::m m @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_modexp_g2_2048_output_s::r r @endlink 
    */
#define PKE_MODEXP_2048                          0x510c106e
   /**< Functionality ID for Modular exponentiation for 2048-bit numbers  
    * @li 3 input parameters : @link icp_qat_fw_mmp_modexp_2048_input_s::g g @endlink @link icp_qat_fw_mmp_modexp_2048_input_s::e e @endlink @link icp_qat_fw_mmp_modexp_2048_input_s::m m @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_modexp_2048_output_s::r r @endlink 
    */
#define PKE_MODEXP_G2_3072                       0x3d0b107a
   /**< Functionality ID for Modular exponentiation base 2 for 3072-bit numbers  
    * @li 2 input parameters : @link icp_qat_fw_mmp_modexp_g2_3072_input_s::e e @endlink @link icp_qat_fw_mmp_modexp_g2_3072_input_s::m m @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_modexp_g2_3072_output_s::r r @endlink 
    */
#define PKE_MODEXP_3072                          0x550c1085
   /**< Functionality ID for Modular exponentiation for 3072-bit numbers  
    * @li 3 input parameters : @link icp_qat_fw_mmp_modexp_3072_input_s::g g @endlink @link icp_qat_fw_mmp_modexp_3072_input_s::e e @endlink @link icp_qat_fw_mmp_modexp_3072_input_s::m m @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_modexp_3072_output_s::r r @endlink 
    */
#define PKE_MODEXP_G2_4096                       0x4d0b1091
   /**< Functionality ID for Modular exponentiation base 2 for 4096-bit numbers  
    * @li 2 input parameters : @link icp_qat_fw_mmp_modexp_g2_4096_input_s::e e @endlink @link icp_qat_fw_mmp_modexp_g2_4096_input_s::m m @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_modexp_g2_4096_output_s::r r @endlink 
    */
#define PKE_MODEXP_4096                          0x6d0c109c
   /**< Functionality ID for Modular exponentiation for 4096-bit numbers  
    * @li 3 input parameters : @link icp_qat_fw_mmp_modexp_4096_input_s::g g @endlink @link icp_qat_fw_mmp_modexp_4096_input_s::e e @endlink @link icp_qat_fw_mmp_modexp_4096_input_s::m m @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_modexp_4096_output_s::r r @endlink 
    */
#define PKE_DSA_GEN_P_1024_160                   0x3b2010a8
   /**< Functionality ID for DSA parameter generation P  
    * @li 4 input parameters : @link icp_qat_fw_mmp_dsa_gen_p_1024_160_input_s::x x @endlink @link icp_qat_fw_mmp_dsa_gen_p_1024_160_input_s::q q @endlink @link icp_qat_fw_mmp_dsa_gen_p_1024_160_input_s::psp1 psp1 @endlink @link icp_qat_fw_mmp_dsa_gen_p_1024_160_input_s::psp2 psp2 @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_dsa_gen_p_1024_160_output_s::p p @endlink 
    */
#define PKE_DSA_GEN_G_1024_160                   0x2d1110c8
   /**< Functionality ID for DSA key generation G  
    * @li 3 input parameters : @link icp_qat_fw_mmp_dsa_gen_g_1024_160_input_s::p p @endlink @link icp_qat_fw_mmp_dsa_gen_g_1024_160_input_s::q q @endlink @link icp_qat_fw_mmp_dsa_gen_g_1024_160_input_s::h h @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_dsa_gen_g_1024_160_output_s::g g @endlink 
    */
#define PKE_DSA_GEN_Y_1024_160                   0x2d1010d9
   /**< Functionality ID for DSA key generation Y  
    * @li 3 input parameters : @link icp_qat_fw_mmp_dsa_gen_y_1024_160_input_s::p p @endlink @link icp_qat_fw_mmp_dsa_gen_y_1024_160_input_s::g g @endlink @link icp_qat_fw_mmp_dsa_gen_y_1024_160_input_s::x x @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_dsa_gen_y_1024_160_output_s::y y @endlink 
    */
#define PKE_DSA_SIGN_R_1024_160                  0x341a10e9
   /**< Functionality ID for DSA Sign R  
    * @li 4 input parameters : @link icp_qat_fw_mmp_dsa_sign_r_1024_160_input_s::k k @endlink @link icp_qat_fw_mmp_dsa_sign_r_1024_160_input_s::p p @endlink @link icp_qat_fw_mmp_dsa_sign_r_1024_160_input_s::q q @endlink @link icp_qat_fw_mmp_dsa_sign_r_1024_160_input_s::g g @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_dsa_sign_r_1024_160_output_s::r r @endlink 
    */
#define PKE_DSA_SIGN_S_1024_160                  0x16121103
   /**< Functionality ID for DSA Sign S  
    * @li 5 input parameters : @link icp_qat_fw_mmp_dsa_sign_s_1024_160_input_s::m m @endlink @link icp_qat_fw_mmp_dsa_sign_s_1024_160_input_s::k k @endlink @link icp_qat_fw_mmp_dsa_sign_s_1024_160_input_s::q q @endlink @link icp_qat_fw_mmp_dsa_sign_s_1024_160_input_s::r r @endlink @link icp_qat_fw_mmp_dsa_sign_s_1024_160_input_s::x x @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_dsa_sign_s_1024_160_output_s::s s @endlink 
    */
#define PKE_DSA_SIGN_R_S_1024_160                0x3a251115
   /**< Functionality ID for DSA Sign R S  
    * @li 6 input parameters : @link icp_qat_fw_mmp_dsa_sign_r_s_1024_160_input_s::m m @endlink @link icp_qat_fw_mmp_dsa_sign_r_s_1024_160_input_s::k k @endlink @link icp_qat_fw_mmp_dsa_sign_r_s_1024_160_input_s::p p @endlink @link icp_qat_fw_mmp_dsa_sign_r_s_1024_160_input_s::q q @endlink @link icp_qat_fw_mmp_dsa_sign_r_s_1024_160_input_s::g g @endlink @link icp_qat_fw_mmp_dsa_sign_r_s_1024_160_input_s::x x @endlink 
    * @li 2 output parameters : @link icp_qat_fw_mmp_dsa_sign_r_s_1024_160_output_s::r r @endlink @link icp_qat_fw_mmp_dsa_sign_r_s_1024_160_output_s::s s @endlink 
    */
#define PKE_DSA_VERIFY_1024_160                  0x4449113a
   /**< Functionality ID for DSA Verify V  
    * @li 7 input parameters : @link icp_qat_fw_mmp_dsa_verify_1024_160_input_s::r r @endlink @link icp_qat_fw_mmp_dsa_verify_1024_160_input_s::s s @endlink @link icp_qat_fw_mmp_dsa_verify_1024_160_input_s::m m @endlink @link icp_qat_fw_mmp_dsa_verify_1024_160_input_s::p p @endlink @link icp_qat_fw_mmp_dsa_verify_1024_160_input_s::q q @endlink @link icp_qat_fw_mmp_dsa_verify_1024_160_input_s::g g @endlink @link icp_qat_fw_mmp_dsa_verify_1024_160_input_s::y y @endlink 
    * @li no output parameters
    */
#define PKE_RSA_KP1_1024                         0x3c2c1183
   /**< Functionality ID for RSA 1024 key generation first form  
    * @li 3 input parameters : @link icp_qat_fw_mmp_rsa_kp1_1024_input_s::p p @endlink @link icp_qat_fw_mmp_rsa_kp1_1024_input_s::q q @endlink @link icp_qat_fw_mmp_rsa_kp1_1024_input_s::e e @endlink 
    * @li 2 output parameters : @link icp_qat_fw_mmp_rsa_kp1_1024_output_s::n n @endlink @link icp_qat_fw_mmp_rsa_kp1_1024_output_s::d d @endlink 
    */
#define PKE_RSA_KP2_1024                         0x555811af
   /**< Functionality ID for RSA 1024 key generation second form  
    * @li 3 input parameters : @link icp_qat_fw_mmp_rsa_kp2_1024_input_s::p p @endlink @link icp_qat_fw_mmp_rsa_kp2_1024_input_s::q q @endlink @link icp_qat_fw_mmp_rsa_kp2_1024_input_s::e e @endlink 
    * @li 5 output parameters : @link icp_qat_fw_mmp_rsa_kp2_1024_output_s::n n @endlink @link icp_qat_fw_mmp_rsa_kp2_1024_output_s::d d @endlink @link icp_qat_fw_mmp_rsa_kp2_1024_output_s::dp dp @endlink @link icp_qat_fw_mmp_rsa_kp2_1024_output_s::dq dq @endlink @link icp_qat_fw_mmp_rsa_kp2_1024_output_s::qinv qinv @endlink 
    */
#define PKE_RSA_EP_1024                          0x351a1207
   /**< Functionality ID for RSA 1024 Encryption  
    * @li 3 input parameters : @link icp_qat_fw_mmp_rsa_ep_1024_input_s::m m @endlink @link icp_qat_fw_mmp_rsa_ep_1024_input_s::e e @endlink @link icp_qat_fw_mmp_rsa_ep_1024_input_s::n n @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_rsa_ep_1024_output_s::c c @endlink 
    */
#define PKE_RSA_DP1_1024                         0x351a1221
   /**< Functionality ID for RSA 1024 Decryption  
    * @li 3 input parameters : @link icp_qat_fw_mmp_rsa_dp1_1024_input_s::c c @endlink @link icp_qat_fw_mmp_rsa_dp1_1024_input_s::d d @endlink @link icp_qat_fw_mmp_rsa_dp1_1024_input_s::n n @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_rsa_dp1_1024_output_s::m m @endlink 
    */
#define PKE_RSA_DP2_1024                         0x2918123b
   /**< Functionality ID for RSA 1024 Decryption with CRT  
    * @li 6 input parameters : @link icp_qat_fw_mmp_rsa_dp2_1024_input_s::c c @endlink @link icp_qat_fw_mmp_rsa_dp2_1024_input_s::p p @endlink @link icp_qat_fw_mmp_rsa_dp2_1024_input_s::q q @endlink @link icp_qat_fw_mmp_rsa_dp2_1024_input_s::dp dp @endlink @link icp_qat_fw_mmp_rsa_dp2_1024_input_s::dq dq @endlink @link icp_qat_fw_mmp_rsa_dp2_1024_input_s::qinv qinv @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_rsa_dp2_1024_output_s::m m @endlink 
    */
#define PKE_RSA_KP1_1536                         0x5c311253
   /**< Functionality ID for RSA 1536 key generation first form  
    * @li 3 input parameters : @link icp_qat_fw_mmp_rsa_kp1_1536_input_s::p p @endlink @link icp_qat_fw_mmp_rsa_kp1_1536_input_s::q q @endlink @link icp_qat_fw_mmp_rsa_kp1_1536_input_s::e e @endlink 
    * @li 2 output parameters : @link icp_qat_fw_mmp_rsa_kp1_1536_output_s::n n @endlink @link icp_qat_fw_mmp_rsa_kp1_1536_output_s::d d @endlink 
    */
#define PKE_RSA_KP2_1536                         0x3c631284
   /**< Functionality ID for RSA 1536 key generation second form  
    * @li 3 input parameters : @link icp_qat_fw_mmp_rsa_kp2_1536_input_s::p p @endlink @link icp_qat_fw_mmp_rsa_kp2_1536_input_s::q q @endlink @link icp_qat_fw_mmp_rsa_kp2_1536_input_s::e e @endlink 
    * @li 5 output parameters : @link icp_qat_fw_mmp_rsa_kp2_1536_output_s::n n @endlink @link icp_qat_fw_mmp_rsa_kp2_1536_output_s::d d @endlink @link icp_qat_fw_mmp_rsa_kp2_1536_output_s::dp dp @endlink @link icp_qat_fw_mmp_rsa_kp2_1536_output_s::dq dq @endlink @link icp_qat_fw_mmp_rsa_kp2_1536_output_s::qinv qinv @endlink 
    */
#define PKE_RSA_EP_1536                          0x4d1a12e7
   /**< Functionality ID for RSA 1536 Encryption  
    * @li 3 input parameters : @link icp_qat_fw_mmp_rsa_ep_1536_input_s::m m @endlink @link icp_qat_fw_mmp_rsa_ep_1536_input_s::e e @endlink @link icp_qat_fw_mmp_rsa_ep_1536_input_s::n n @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_rsa_ep_1536_output_s::c c @endlink 
    */
#define PKE_RSA_DP1_1536                         0x4d1a1301
   /**< Functionality ID for RSA 1536 Decryption  
    * @li 3 input parameters : @link icp_qat_fw_mmp_rsa_dp1_1536_input_s::c c @endlink @link icp_qat_fw_mmp_rsa_dp1_1536_input_s::d d @endlink @link icp_qat_fw_mmp_rsa_dp1_1536_input_s::n n @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_rsa_dp1_1536_output_s::m m @endlink 
    */
#define PKE_RSA_DP2_1536                         0x451b131b
   /**< Functionality ID for RSA 1536 Decryption with CRT  
    * @li 6 input parameters : @link icp_qat_fw_mmp_rsa_dp2_1536_input_s::c c @endlink @link icp_qat_fw_mmp_rsa_dp2_1536_input_s::p p @endlink @link icp_qat_fw_mmp_rsa_dp2_1536_input_s::q q @endlink @link icp_qat_fw_mmp_rsa_dp2_1536_input_s::dp dp @endlink @link icp_qat_fw_mmp_rsa_dp2_1536_input_s::dq dq @endlink @link icp_qat_fw_mmp_rsa_dp2_1536_input_s::qinv qinv @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_rsa_dp2_1536_output_s::m m @endlink 
    */
#define PKE_RSA_KP1_2048                         0x782c1336
   /**< Functionality ID for RSA 2048 key generation first form  
    * @li 3 input parameters : @link icp_qat_fw_mmp_rsa_kp1_2048_input_s::p p @endlink @link icp_qat_fw_mmp_rsa_kp1_2048_input_s::q q @endlink @link icp_qat_fw_mmp_rsa_kp1_2048_input_s::e e @endlink 
    * @li 2 output parameters : @link icp_qat_fw_mmp_rsa_kp1_2048_output_s::n n @endlink @link icp_qat_fw_mmp_rsa_kp1_2048_output_s::d d @endlink 
    */
#define PKE_RSA_KP2_2048                         0x505e1362
   /**< Functionality ID for RSA 2048 key generation second form  
    * @li 3 input parameters : @link icp_qat_fw_mmp_rsa_kp2_2048_input_s::p p @endlink @link icp_qat_fw_mmp_rsa_kp2_2048_input_s::q q @endlink @link icp_qat_fw_mmp_rsa_kp2_2048_input_s::e e @endlink 
    * @li 5 output parameters : @link icp_qat_fw_mmp_rsa_kp2_2048_output_s::n n @endlink @link icp_qat_fw_mmp_rsa_kp2_2048_output_s::d d @endlink @link icp_qat_fw_mmp_rsa_kp2_2048_output_s::dp dp @endlink @link icp_qat_fw_mmp_rsa_kp2_2048_output_s::dq dq @endlink @link icp_qat_fw_mmp_rsa_kp2_2048_output_s::qinv qinv @endlink 
    */
#define PKE_RSA_EP_2048                          0x6e1f13c0
   /**< Functionality ID for RSA 2048 Encryption  
    * @li 3 input parameters : @link icp_qat_fw_mmp_rsa_ep_2048_input_s::m m @endlink @link icp_qat_fw_mmp_rsa_ep_2048_input_s::e e @endlink @link icp_qat_fw_mmp_rsa_ep_2048_input_s::n n @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_rsa_ep_2048_output_s::c c @endlink 
    */
#define PKE_RSA_DP1_2048                         0x6e1f13df
   /**< Functionality ID for RSA 2048 Decryption  
    * @li 3 input parameters : @link icp_qat_fw_mmp_rsa_dp1_2048_input_s::c c @endlink @link icp_qat_fw_mmp_rsa_dp1_2048_input_s::d d @endlink @link icp_qat_fw_mmp_rsa_dp1_2048_input_s::n n @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_rsa_dp1_2048_output_s::m m @endlink 
    */
#define PKE_RSA_DP2_2048                         0x591b13fe
   /**< Functionality ID for RSA 2048 Decryption with CRT  
    * @li 6 input parameters : @link icp_qat_fw_mmp_rsa_dp2_2048_input_s::c c @endlink @link icp_qat_fw_mmp_rsa_dp2_2048_input_s::p p @endlink @link icp_qat_fw_mmp_rsa_dp2_2048_input_s::q q @endlink @link icp_qat_fw_mmp_rsa_dp2_2048_input_s::dp dp @endlink @link icp_qat_fw_mmp_rsa_dp2_2048_input_s::dq dq @endlink @link icp_qat_fw_mmp_rsa_dp2_2048_input_s::qinv qinv @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_rsa_dp2_2048_output_s::m m @endlink 
    */
#define PKE_RSA_KP1_3072                         0x682d1419
   /**< Functionality ID for RSA 3072 key generation first form  
    * @li 3 input parameters : @link icp_qat_fw_mmp_rsa_kp1_3072_input_s::p p @endlink @link icp_qat_fw_mmp_rsa_kp1_3072_input_s::q q @endlink @link icp_qat_fw_mmp_rsa_kp1_3072_input_s::e e @endlink 
    * @li 2 output parameters : @link icp_qat_fw_mmp_rsa_kp1_3072_output_s::n n @endlink @link icp_qat_fw_mmp_rsa_kp1_3072_output_s::d d @endlink 
    */
#define PKE_RSA_KP2_3072                         0x78571446
   /**< Functionality ID for RSA 3072 key generation second form  
    * @li 3 input parameters : @link icp_qat_fw_mmp_rsa_kp2_3072_input_s::p p @endlink @link icp_qat_fw_mmp_rsa_kp2_3072_input_s::q q @endlink @link icp_qat_fw_mmp_rsa_kp2_3072_input_s::e e @endlink 
    * @li 5 output parameters : @link icp_qat_fw_mmp_rsa_kp2_3072_output_s::n n @endlink @link icp_qat_fw_mmp_rsa_kp2_3072_output_s::d d @endlink @link icp_qat_fw_mmp_rsa_kp2_3072_output_s::dp dp @endlink @link icp_qat_fw_mmp_rsa_kp2_3072_output_s::dq dq @endlink @link icp_qat_fw_mmp_rsa_kp2_3072_output_s::qinv qinv @endlink 
    */
#define PKE_RSA_EP_3072                          0x7d19149d
   /**< Functionality ID for RSA 3072 Encryption  
    * @li 3 input parameters : @link icp_qat_fw_mmp_rsa_ep_3072_input_s::m m @endlink @link icp_qat_fw_mmp_rsa_ep_3072_input_s::e e @endlink @link icp_qat_fw_mmp_rsa_ep_3072_input_s::n n @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_rsa_ep_3072_output_s::c c @endlink 
    */
#define PKE_RSA_DP1_3072                         0x7d1914b6
   /**< Functionality ID for RSA 3072 Decryption  
    * @li 3 input parameters : @link icp_qat_fw_mmp_rsa_dp1_3072_input_s::c c @endlink @link icp_qat_fw_mmp_rsa_dp1_3072_input_s::d d @endlink @link icp_qat_fw_mmp_rsa_dp1_3072_input_s::n n @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_rsa_dp1_3072_output_s::m m @endlink 
    */
#define PKE_RSA_DP2_3072                         0x811c14cf
   /**< Functionality ID for RSA 3072 Decryption with CRT  
    * @li 6 input parameters : @link icp_qat_fw_mmp_rsa_dp2_3072_input_s::c c @endlink @link icp_qat_fw_mmp_rsa_dp2_3072_input_s::p p @endlink @link icp_qat_fw_mmp_rsa_dp2_3072_input_s::q q @endlink @link icp_qat_fw_mmp_rsa_dp2_3072_input_s::dp dp @endlink @link icp_qat_fw_mmp_rsa_dp2_3072_input_s::dq dq @endlink @link icp_qat_fw_mmp_rsa_dp2_3072_input_s::qinv qinv @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_rsa_dp2_3072_output_s::m m @endlink 
    */
#define PKE_RSA_KP1_4096                         0x803014eb
   /**< Functionality ID for RSA 4096 key generation first form  
    * @li 3 input parameters : @link icp_qat_fw_mmp_rsa_kp1_4096_input_s::p p @endlink @link icp_qat_fw_mmp_rsa_kp1_4096_input_s::q q @endlink @link icp_qat_fw_mmp_rsa_kp1_4096_input_s::e e @endlink 
    * @li 2 output parameters : @link icp_qat_fw_mmp_rsa_kp1_4096_output_s::n n @endlink @link icp_qat_fw_mmp_rsa_kp1_4096_output_s::d d @endlink 
    */
#define PKE_RSA_KP2_4096                         0x805c151b
   /**< Functionality ID for RSA 4096 key generation second form  
    * @li 3 input parameters : @link icp_qat_fw_mmp_rsa_kp2_4096_input_s::p p @endlink @link icp_qat_fw_mmp_rsa_kp2_4096_input_s::q q @endlink @link icp_qat_fw_mmp_rsa_kp2_4096_input_s::e e @endlink 
    * @li 5 output parameters : @link icp_qat_fw_mmp_rsa_kp2_4096_output_s::n n @endlink @link icp_qat_fw_mmp_rsa_kp2_4096_output_s::d d @endlink @link icp_qat_fw_mmp_rsa_kp2_4096_output_s::dp dp @endlink @link icp_qat_fw_mmp_rsa_kp2_4096_output_s::dq dq @endlink @link icp_qat_fw_mmp_rsa_kp2_4096_output_s::qinv qinv @endlink 
    */
#define PKE_RSA_EP_4096                          0xa6191577
   /**< Functionality ID for RSA 4096 Encryption  
    * @li 3 input parameters : @link icp_qat_fw_mmp_rsa_ep_4096_input_s::m m @endlink @link icp_qat_fw_mmp_rsa_ep_4096_input_s::e e @endlink @link icp_qat_fw_mmp_rsa_ep_4096_input_s::n n @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_rsa_ep_4096_output_s::c c @endlink 
    */
#define PKE_RSA_DP1_4096                         0xa6191590
   /**< Functionality ID for RSA 4096 Decryption  
    * @li 3 input parameters : @link icp_qat_fw_mmp_rsa_dp1_4096_input_s::c c @endlink @link icp_qat_fw_mmp_rsa_dp1_4096_input_s::d d @endlink @link icp_qat_fw_mmp_rsa_dp1_4096_input_s::n n @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_rsa_dp1_4096_output_s::m m @endlink 
    */
#define PKE_RSA_DP2_4096                         0x792115a9
   /**< Functionality ID for RSA 4096 Decryption with CRT  
    * @li 6 input parameters : @link icp_qat_fw_mmp_rsa_dp2_4096_input_s::c c @endlink @link icp_qat_fw_mmp_rsa_dp2_4096_input_s::p p @endlink @link icp_qat_fw_mmp_rsa_dp2_4096_input_s::q q @endlink @link icp_qat_fw_mmp_rsa_dp2_4096_input_s::dp dp @endlink @link icp_qat_fw_mmp_rsa_dp2_4096_input_s::dq dq @endlink @link icp_qat_fw_mmp_rsa_dp2_4096_input_s::qinv qinv @endlink 
    * @li 1 output parameters : @link icp_qat_fw_mmp_rsa_dp2_4096_output_s::m m @endlink 
    */
#define PKE_GCD_PT_192                           0x252515ca
   /**< Functionality ID for GCD primality test for 192-bit numbers  
    * @li 2 input parameters : @link icp_qat_fw_mmp_gcd_pt_192_input_s::m m @endlink @link icp_qat_fw_mmp_gcd_pt_192_input_s::psp psp @endlink 
    * @li no output parameters
    */
#define PKE_GCD_PT_256                           0x252515ef
   /**< Functionality ID for GCD primality test for 256-bit numbers  
    * @li 2 input parameters : @link icp_qat_fw_mmp_gcd_pt_256_input_s::m m @endlink @link icp_qat_fw_mmp_gcd_pt_256_input_s::psp psp @endlink 
    * @li no output parameters
    */
#define PKE_GCD_PT_384                           0x25251614
   /**< Functionality ID for GCD primality test for 384-bit numbers  
    * @li 2 input parameters : @link icp_qat_fw_mmp_gcd_pt_384_input_s::m m @endlink @link icp_qat_fw_mmp_gcd_pt_384_input_s::psp psp @endlink 
    * @li no output parameters
    */
#define PKE_GCD_PT_512                           0x211a1639
   /**< Functionality ID for GCD primality test for 512-bit numbers  
    * @li 2 input parameters : @link icp_qat_fw_mmp_gcd_pt_512_input_s::m m @endlink @link icp_qat_fw_mmp_gcd_pt_512_input_s::psp psp @endlink 
    * @li no output parameters
    */
#define PKE_GCD_PT_768                           0x18101653
   /**< Functionality ID for GCD primality test for 768-bit numbers  
    * @li 2 input parameters : @link icp_qat_fw_mmp_gcd_pt_768_input_s::m m @endlink @link icp_qat_fw_mmp_gcd_pt_768_input_s::psp psp @endlink 
    * @li no output parameters
    */
#define PKE_GCD_PT_1024                          0x180c1663
   /**< Functionality ID for GCD primality test for 1024-bit numbers  
    * @li 2 input parameters : @link icp_qat_fw_mmp_gcd_pt_1024_input_s::m m @endlink @link icp_qat_fw_mmp_gcd_pt_1024_input_s::psp psp @endlink 
    * @li no output parameters
    */
#define PKE_GCD_PT_1536                          0x421f166f
   /**< Functionality ID for GCD primality test for 1536-bit numbers  
    * @li 2 input parameters : @link icp_qat_fw_mmp_gcd_pt_1536_input_s::m m @endlink @link icp_qat_fw_mmp_gcd_pt_1536_input_s::psp psp @endlink 
    * @li no output parameters
    */
#define PKE_GCD_PT_2048                          0x361b168e
   /**< Functionality ID for GCD primality test for 2048-bit numbers  
    * @li 2 input parameters : @link icp_qat_fw_mmp_gcd_pt_2048_input_s::m m @endlink @link icp_qat_fw_mmp_gcd_pt_2048_input_s::psp psp @endlink 
    * @li no output parameters
    */
#define PKE_GCD_PT_3072                          0x962a16a9
   /**< Functionality ID for GCD primality test for 3072-bit numbers  
    * @li 2 input parameters : @link icp_qat_fw_mmp_gcd_pt_3072_input_s::m m @endlink @link icp_qat_fw_mmp_gcd_pt_3072_input_s::psp psp @endlink 
    * @li no output parameters
    */
#define PKE_GCD_PT_4096                          0x7e2616d3
   /**< Functionality ID for GCD primality test for 4096-bit numbers  
    * @li 2 input parameters : @link icp_qat_fw_mmp_gcd_pt_4096_input_s::m m @endlink @link icp_qat_fw_mmp_gcd_pt_4096_input_s::psp psp @endlink 
    * @li no output parameters
    */
#define PKE_FERMAT_PT_160                        0x101016f9
   /**< Functionality ID for Fermat primality test for 160-bit numbers  
    * @li 1 input parameters : @link icp_qat_fw_mmp_fermat_pt_160_input_s::m m @endlink 
    * @li no output parameters
    */
#define PKE_FERMAT_PT_512                        0x15101709
   /**< Functionality ID for Fermat primality test for 512-bit numbers  
    * @li 1 input parameters : @link icp_qat_fw_mmp_fermat_pt_512_input_s::m m @endlink 
    * @li no output parameters
    */
#define PKE_FERMAT_PT_768                        0x1f101719
   /**< Functionality ID for Fermat primality test for 768-bit numbers  
    * @li 1 input parameters : @link icp_qat_fw_mmp_fermat_pt_768_input_s::m m @endlink 
    * @li no output parameters
    */
#define PKE_FERMAT_PT_1024                       0x25101729
   /**< Functionality ID for Fermat primality test for 1024-bit numbers  
    * @li 1 input parameters : @link icp_qat_fw_mmp_fermat_pt_1024_input_s::m m @endlink 
    * @li no output parameters
    */
#define PKE_FERMAT_PT_1536                       0x31101739
   /**< Functionality ID for Fermat primality test for 1536-bit numbers  
    * @li 1 input parameters : @link icp_qat_fw_mmp_fermat_pt_1536_input_s::m m @endlink 
    * @li no output parameters
    */
#define PKE_FERMAT_PT_2048                       0x41101749
   /**< Functionality ID for Fermat primality test for 2048-bit numbers  
    * @li 1 input parameters : @link icp_qat_fw_mmp_fermat_pt_2048_input_s::m m @endlink 
    * @li no output parameters
    */
#define PKE_FERMAT_PT_3072                       0x3d101759
   /**< Functionality ID for Fermat primality test for 3072-bit numbers  
    * @li 1 input parameters : @link icp_qat_fw_mmp_fermat_pt_3072_input_s::m m @endlink 
    * @li no output parameters
    */
#define PKE_FERMAT_PT_4096                       0x4d101769
   /**< Functionality ID for Fermat primality test for 4096-bit numbers  
    * @li 1 input parameters : @link icp_qat_fw_mmp_fermat_pt_4096_input_s::m m @endlink 
    * @li no output parameters
    */
#define PKE_MR_PT_160                            0x10161779
   /**< Functionality ID for Miller-Rabin primality test for 160-bit numbers  
    * @li 2 input parameters : @link icp_qat_fw_mmp_mr_pt_160_input_s::x x @endlink @link icp_qat_fw_mmp_mr_pt_160_input_s::m m @endlink 
    * @li no output parameters
    */
#define PKE_MR_PT_512                            0x1516178f
   /**< Functionality ID for Miller-Rabin primality test for 512-bit numbers  
    * @li 2 input parameters : @link icp_qat_fw_mmp_mr_pt_512_input_s::x x @endlink @link icp_qat_fw_mmp_mr_pt_512_input_s::m m @endlink 
    * @li no output parameters
    */
#define PKE_MR_PT_768                            0x1f1617a5
   /**< Functionality ID for Miller-Rabin primality test for 768-bit numbers  
    * @li 2 input parameters : @link icp_qat_fw_mmp_mr_pt_768_input_s::x x @endlink @link icp_qat_fw_mmp_mr_pt_768_input_s::m m @endlink 
    * @li no output parameters
    */
#define PKE_MR_PT_1024                           0x251617bb
   /**< Functionality ID for Miller-Rabin primality test for 1024-bit numbers  
    * @li 2 input parameters : @link icp_qat_fw_mmp_mr_pt_1024_input_s::x x @endlink @link icp_qat_fw_mmp_mr_pt_1024_input_s::m m @endlink 
    * @li no output parameters
    */
#define PKE_MR_PT_1536                           0x351617d1
   /**< Functionality ID for Miller-Rabin primality test for 1536-bit numbers  
    * @li 2 input parameters : @link icp_qat_fw_mmp_mr_pt_1536_input_s::x x @endlink @link icp_qat_fw_mmp_mr_pt_1536_input_s::m m @endlink 
    * @li no output parameters
    */
#define PKE_MR_PT_2048                           0x491617e7
   /**< Functionality ID for Miller-Rabin primality test for 2048-bit numbers  
    * @li 2 input parameters : @link icp_qat_fw_mmp_mr_pt_2048_input_s::x x @endlink @link icp_qat_fw_mmp_mr_pt_2048_input_s::m m @endlink 
    * @li no output parameters
    */
#define PKE_MR_PT_3072                           0x4d1617fd
   /**< Functionality ID for Miller-Rabin primality test for 3072-bit numbers  
    * @li 2 input parameters : @link icp_qat_fw_mmp_mr_pt_3072_input_s::x x @endlink @link icp_qat_fw_mmp_mr_pt_3072_input_s::m m @endlink 
    * @li no output parameters
    */
#define PKE_MR_PT_4096                           0x65161813
   /**< Functionality ID for Miller-Rabin primality test for 4096-bit numbers  
    * @li 2 input parameters : @link icp_qat_fw_mmp_mr_pt_4096_input_s::x x @endlink @link icp_qat_fw_mmp_mr_pt_4096_input_s::m m @endlink 
    * @li no output parameters
    */
#define PKE_LUCAS_PT_160                         0x0f101829
   /**< Functionality ID for Lucas primality test for 160-bit numbers  
    * @li 1 input parameters : @link icp_qat_fw_mmp_lucas_pt_160_input_s::m m @endlink 
    * @li no output parameters
    */
#define PKE_LUCAS_PT_512                         0x12101839
   /**< Functionality ID for Lucas primality test for 512-bit numbers  
    * @li 1 input parameters : @link icp_qat_fw_mmp_lucas_pt_512_input_s::m m @endlink 
    * @li no output parameters
    */
#define PKE_LUCAS_PT_768                         0x14101849
   /**< Functionality ID for Lucas primality test for 768-bit numbers  
    * @li 1 input parameters : @link icp_qat_fw_mmp_lucas_pt_768_input_s::m m @endlink 
    * @li no output parameters
    */
#define PKE_LUCAS_PT_1024                        0x16101859
   /**< Functionality ID for Lucas primality test for 1024-bit numbers  
    * @li 1 input parameters : @link icp_qat_fw_mmp_lucas_pt_1024_input_s::m m @endlink 
    * @li no output parameters
    */
#define PKE_LUCAS_PT_1536                        0x1a101869
   /**< Functionality ID for Lucas primality test for 1536-bit numbers  
    * @li 1 input parameters : @link icp_qat_fw_mmp_lucas_pt_1536_input_s::m m @endlink 
    * @li no output parameters
    */
#define PKE_LUCAS_PT_2048                        0x1e101879
   /**< Functionality ID for Lucas primality test for 2048-bit numbers  
    * @li 1 input parameters : @link icp_qat_fw_mmp_lucas_pt_2048_input_s::m m @endlink 
    * @li no output parameters
    */
#define PKE_LUCAS_PT_3072                        0x26101889
   /**< Functionality ID for Lucas primality test for 3072-bit numbers  
    * @li 1 input parameters : @link icp_qat_fw_mmp_lucas_pt_3072_input_s::m m @endlink 
    * @li no output parameters
    */
#define PKE_LUCAS_PT_4096                        0x6e151899
   /**< Functionality ID for Lucas primality test for 4096-bit numbers  
    * @li 1 input parameters : @link icp_qat_fw_mmp_lucas_pt_4096_input_s::m m @endlink 
    * @li no output parameters
    */
#define MATHS_MODEXP_L512                        0x1d1518ae
   /**< Functionality ID for Modular exponentiation for numbers less than 512-bits  
    * @li 3 input parameters : @link icp_qat_fw_maths_modexp_l512_input_s::g g @endlink @link icp_qat_fw_maths_modexp_l512_input_s::e e @endlink @link icp_qat_fw_maths_modexp_l512_input_s::m m @endlink 
    * @li 1 output parameters : @link icp_qat_fw_maths_modexp_l512_output_s::r r @endlink 
    */
#define MATHS_MODEXP_L1024                       0x2d1518c3
   /**< Functionality ID for Modular exponentiation for numbers less than 1024-bit  
    * @li 3 input parameters : @link icp_qat_fw_maths_modexp_l1024_input_s::g g @endlink @link icp_qat_fw_maths_modexp_l1024_input_s::e e @endlink @link icp_qat_fw_maths_modexp_l1024_input_s::m m @endlink 
    * @li 1 output parameters : @link icp_qat_fw_maths_modexp_l1024_output_s::r r @endlink 
    */
#define MATHS_MODEXP_L1536                       0x411518d8
   /**< Functionality ID for Modular exponentiation for numbers less than 1536-bits  
    * @li 3 input parameters : @link icp_qat_fw_maths_modexp_l1536_input_s::g g @endlink @link icp_qat_fw_maths_modexp_l1536_input_s::e e @endlink @link icp_qat_fw_maths_modexp_l1536_input_s::m m @endlink 
    * @li 1 output parameters : @link icp_qat_fw_maths_modexp_l1536_output_s::r r @endlink 
    */
#define MATHS_MODEXP_L2048                       0x5e1a18ed
   /**< Functionality ID for Modular exponentiation for numbers less than 2048-bit  
    * @li 3 input parameters : @link icp_qat_fw_maths_modexp_l2048_input_s::g g @endlink @link icp_qat_fw_maths_modexp_l2048_input_s::e e @endlink @link icp_qat_fw_maths_modexp_l2048_input_s::m m @endlink 
    * @li 1 output parameters : @link icp_qat_fw_maths_modexp_l2048_output_s::r r @endlink 
    */
#define MATHS_MODEXP_L2560                       0x65201907
   /**< Functionality ID for Modular exponentiation for numbers less than 2560-bits  
    * @li 3 input parameters : @link icp_qat_fw_maths_modexp_l2560_input_s::g g @endlink @link icp_qat_fw_maths_modexp_l2560_input_s::e e @endlink @link icp_qat_fw_maths_modexp_l2560_input_s::m m @endlink 
    * @li 1 output parameters : @link icp_qat_fw_maths_modexp_l2560_output_s::r r @endlink 
    */
#define MATHS_MODEXP_L3072                       0x65151927
   /**< Functionality ID for Modular exponentiation for numbers less than 3072-bits  
    * @li 3 input parameters : @link icp_qat_fw_maths_modexp_l3072_input_s::g g @endlink @link icp_qat_fw_maths_modexp_l3072_input_s::e e @endlink @link icp_qat_fw_maths_modexp_l3072_input_s::m m @endlink 
    * @li 1 output parameters : @link icp_qat_fw_maths_modexp_l3072_output_s::r r @endlink 
    */
#define MATHS_MODEXP_L3584                       0x851c193c
   /**< Functionality ID for Modular exponentiation for numbers less than 3584-bits  
    * @li 3 input parameters : @link icp_qat_fw_maths_modexp_l3584_input_s::g g @endlink @link icp_qat_fw_maths_modexp_l3584_input_s::e e @endlink @link icp_qat_fw_maths_modexp_l3584_input_s::m m @endlink 
    * @li 1 output parameters : @link icp_qat_fw_maths_modexp_l3584_output_s::r r @endlink 
    */
#define MATHS_MODEXP_L4096                       0x85161958
   /**< Functionality ID for Modular exponentiation for numbers less than 4096-bit  
    * @li 3 input parameters : @link icp_qat_fw_maths_modexp_l4096_input_s::g g @endlink @link icp_qat_fw_maths_modexp_l4096_input_s::e e @endlink @link icp_qat_fw_maths_modexp_l4096_input_s::m m @endlink 
    * @li 1 output parameters : @link icp_qat_fw_maths_modexp_l4096_output_s::r r @endlink 
    */
#define MATHS_MODINV_ODD_L128                    0x0906196e
   /**< Functionality ID for Modular multiplicative inverse for numbers less than 128 bits  
    * @li 2 input parameters : @link icp_qat_fw_maths_modinv_odd_l128_input_s::a a @endlink @link icp_qat_fw_maths_modinv_odd_l128_input_s::b b @endlink 
    * @li 1 output parameters : @link icp_qat_fw_maths_modinv_odd_l128_output_s::c c @endlink 
    */
#define MATHS_MODINV_ODD_L192                    0x0a061974
   /**< Functionality ID for Modular multiplicative inverse for numbers less than 192 bits  
    * @li 2 input parameters : @link icp_qat_fw_maths_modinv_odd_l192_input_s::a a @endlink @link icp_qat_fw_maths_modinv_odd_l192_input_s::b b @endlink 
    * @li 1 output parameters : @link icp_qat_fw_maths_modinv_odd_l192_output_s::c c @endlink 
    */
#define MATHS_MODINV_ODD_L256                    0x0a06197a
   /**< Functionality ID for Modular multiplicative inverse for numbers less than 256 bits  
    * @li 2 input parameters : @link icp_qat_fw_maths_modinv_odd_l256_input_s::a a @endlink @link icp_qat_fw_maths_modinv_odd_l256_input_s::b b @endlink 
    * @li 1 output parameters : @link icp_qat_fw_maths_modinv_odd_l256_output_s::c c @endlink 
    */
#define MATHS_MODINV_ODD_L384                    0x0b061980
   /**< Functionality ID for Modular multiplicative inverse for numbers less than 384 bits  
    * @li 2 input parameters : @link icp_qat_fw_maths_modinv_odd_l384_input_s::a a @endlink @link icp_qat_fw_maths_modinv_odd_l384_input_s::b b @endlink 
    * @li 1 output parameters : @link icp_qat_fw_maths_modinv_odd_l384_output_s::c c @endlink 
    */
#define MATHS_MODINV_ODD_L512                    0x0c061986
   /**< Functionality ID for Modular multiplicative inverse for numbers less than 512 bits  
    * @li 2 input parameters : @link icp_qat_fw_maths_modinv_odd_l512_input_s::a a @endlink @link icp_qat_fw_maths_modinv_odd_l512_input_s::b b @endlink 
    * @li 1 output parameters : @link icp_qat_fw_maths_modinv_odd_l512_output_s::c c @endlink 
    */
#define MATHS_MODINV_ODD_L768                    0x0e06198c
   /**< Functionality ID for Modular multiplicative inverse for numbers less than 768 bits  
    * @li 2 input parameters : @link icp_qat_fw_maths_modinv_odd_l768_input_s::a a @endlink @link icp_qat_fw_maths_modinv_odd_l768_input_s::b b @endlink 
    * @li 1 output parameters : @link icp_qat_fw_maths_modinv_odd_l768_output_s::c c @endlink 
    */
#define MATHS_MODINV_ODD_L1024                   0x10061992
   /**< Functionality ID for Modular multiplicative inverse for numbers less than 1024 bits  
    * @li 2 input parameters : @link icp_qat_fw_maths_modinv_odd_l1024_input_s::a a @endlink @link icp_qat_fw_maths_modinv_odd_l1024_input_s::b b @endlink 
    * @li 1 output parameters : @link icp_qat_fw_maths_modinv_odd_l1024_output_s::c c @endlink 
    */
#define MATHS_MODINV_ODD_L1536                   0x18061998
   /**< Functionality ID for Modular multiplicative inverse for numbers less than 1536 bits  
    * @li 2 input parameters : @link icp_qat_fw_maths_modinv_odd_l1536_input_s::a a @endlink @link icp_qat_fw_maths_modinv_odd_l1536_input_s::b b @endlink 
    * @li 1 output parameters : @link icp_qat_fw_maths_modinv_odd_l1536_output_s::c c @endlink 
    */
#define MATHS_MODINV_ODD_L2048                   0x2006199e
   /**< Functionality ID for Modular multiplicative inverse for numbers less than 2048 bits  
    * @li 2 input parameters : @link icp_qat_fw_maths_modinv_odd_l2048_input_s::a a @endlink @link icp_qat_fw_maths_modinv_odd_l2048_input_s::b b @endlink 
    * @li 1 output parameters : @link icp_qat_fw_maths_modinv_odd_l2048_output_s::c c @endlink 
    */
#define MATHS_MODINV_ODD_L3072                   0x300619a4
   /**< Functionality ID for Modular multiplicative inverse for numbers less than 3072 bits  
    * @li 2 input parameters : @link icp_qat_fw_maths_modinv_odd_l3072_input_s::a a @endlink @link icp_qat_fw_maths_modinv_odd_l3072_input_s::b b @endlink 
    * @li 1 output parameters : @link icp_qat_fw_maths_modinv_odd_l3072_output_s::c c @endlink 
    */
#define MATHS_MODINV_ODD_L4096                   0x400619aa
   /**< Functionality ID for Modular multiplicative inverse for numbers less than 4096 bits  
    * @li 2 input parameters : @link icp_qat_fw_maths_modinv_odd_l4096_input_s::a a @endlink @link icp_qat_fw_maths_modinv_odd_l4096_input_s::b b @endlink 
    * @li 1 output parameters : @link icp_qat_fw_maths_modinv_odd_l4096_output_s::c c @endlink 
    */
#define MATHS_MODINV_EVEN_L128                   0x090619b0
   /**< Functionality ID for Modular multiplicative inverse for numbers less than 128 bits  
    * @li 2 input parameters : @link icp_qat_fw_maths_modinv_even_l128_input_s::a a @endlink @link icp_qat_fw_maths_modinv_even_l128_input_s::b b @endlink 
    * @li 1 output parameters : @link icp_qat_fw_maths_modinv_even_l128_output_s::c c @endlink 
    */
#define MATHS_MODINV_EVEN_L192                   0x0a0619b6
   /**< Functionality ID for Modular multiplicative inverse for numbers less than 192 bits  
    * @li 2 input parameters : @link icp_qat_fw_maths_modinv_even_l192_input_s::a a @endlink @link icp_qat_fw_maths_modinv_even_l192_input_s::b b @endlink 
    * @li 1 output parameters : @link icp_qat_fw_maths_modinv_even_l192_output_s::c c @endlink 
    */
#define MATHS_MODINV_EVEN_L256                   0x0a0619bc
   /**< Functionality ID for Modular multiplicative inverse for numbers less than 256 bits  
    * @li 2 input parameters : @link icp_qat_fw_maths_modinv_even_l256_input_s::a a @endlink @link icp_qat_fw_maths_modinv_even_l256_input_s::b b @endlink 
    * @li 1 output parameters : @link icp_qat_fw_maths_modinv_even_l256_output_s::c c @endlink 
    */
#define MATHS_MODINV_EVEN_L384                   0x110b19c2
   /**< Functionality ID for Modular multiplicative inverse for numbers less than 384 bits  
    * @li 2 input parameters : @link icp_qat_fw_maths_modinv_even_l384_input_s::a a @endlink @link icp_qat_fw_maths_modinv_even_l384_input_s::b b @endlink 
    * @li 1 output parameters : @link icp_qat_fw_maths_modinv_even_l384_output_s::c c @endlink 
    */
#define MATHS_MODINV_EVEN_L512                   0x140b19cd
   /**< Functionality ID for Modular multiplicative inverse for numbers less than 512 bits  
    * @li 2 input parameters : @link icp_qat_fw_maths_modinv_even_l512_input_s::a a @endlink @link icp_qat_fw_maths_modinv_even_l512_input_s::b b @endlink 
    * @li 1 output parameters : @link icp_qat_fw_maths_modinv_even_l512_output_s::c c @endlink 
    */
#define MATHS_MODINV_EVEN_L768                   0x1a0b19d8
   /**< Functionality ID for Modular multiplicative inverse for numbers less than 768 bits  
    * @li 2 input parameters : @link icp_qat_fw_maths_modinv_even_l768_input_s::a a @endlink @link icp_qat_fw_maths_modinv_even_l768_input_s::b b @endlink 
    * @li 1 output parameters : @link icp_qat_fw_maths_modinv_even_l768_output_s::c c @endlink 
    */
#define MATHS_MODINV_EVEN_L1024                  0x200b19e3
   /**< Functionality ID for Modular multiplicative inverse for numbers less than 1024 bits  
    * @li 2 input parameters : @link icp_qat_fw_maths_modinv_even_l1024_input_s::a a @endlink @link icp_qat_fw_maths_modinv_even_l1024_input_s::b b @endlink 
    * @li 1 output parameters : @link icp_qat_fw_maths_modinv_even_l1024_output_s::c c @endlink 
    */
#define MATHS_MODINV_EVEN_L1536                  0x2c0b19ee
   /**< Functionality ID for Modular multiplicative inverse for numbers less than 1536 bits  
    * @li 2 input parameters : @link icp_qat_fw_maths_modinv_even_l1536_input_s::a a @endlink @link icp_qat_fw_maths_modinv_even_l1536_input_s::b b @endlink 
    * @li 1 output parameters : @link icp_qat_fw_maths_modinv_even_l1536_output_s::c c @endlink 
    */
#define MATHS_MODINV_EVEN_L2048                  0x380b19f9
   /**< Functionality ID for Modular multiplicative inverse for numbers less than 2048 bits  
    * @li 2 input parameters : @link icp_qat_fw_maths_modinv_even_l2048_input_s::a a @endlink @link icp_qat_fw_maths_modinv_even_l2048_input_s::b b @endlink 
    * @li 1 output parameters : @link icp_qat_fw_maths_modinv_even_l2048_output_s::c c @endlink 
    */
#define MATHS_MODINV_EVEN_L3072                  0x500b1a04
   /**< Functionality ID for Modular multiplicative inverse for numbers less than 3072 bits  
    * @li 2 input parameters : @link icp_qat_fw_maths_modinv_even_l3072_input_s::a a @endlink @link icp_qat_fw_maths_modinv_even_l3072_input_s::b b @endlink 
    * @li 1 output parameters : @link icp_qat_fw_maths_modinv_even_l3072_output_s::c c @endlink 
    */
#define MATHS_MODINV_EVEN_L4096                  0x680b1a0f
   /**< Functionality ID for Modular multiplicative inverse for numbers less than 4096 bits  
    * @li 2 input parameters : @link icp_qat_fw_maths_modinv_even_l4096_input_s::a a @endlink @link icp_qat_fw_maths_modinv_even_l4096_input_s::b b @endlink 
    * @li 1 output parameters : @link icp_qat_fw_maths_modinv_even_l4096_output_s::c c @endlink 
    */


#define PKE_LIVENESS             0x00000001
   /**< Functionality ID for PKE_LIVENESS 
    * @li 0 input parameter(s) 
    * @li 1 output parameter(s) (8 qwords)
    */
#define PKE_INTERFACE_SIGNATURE  0x421cd4c2
   /**<  Encoded signature of the interface specifications
    */


#endif /* __ICP_QAT_FW_MMP_IDS__ */


/* --- (Automatically generated (build v. 2.33), do not modify manually) --- */

/* --- end of file --- */



