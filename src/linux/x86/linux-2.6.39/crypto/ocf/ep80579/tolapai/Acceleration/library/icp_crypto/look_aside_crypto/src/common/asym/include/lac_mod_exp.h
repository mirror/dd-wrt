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
 * @file lac_mod_exp.h
 *
 * @defgroup LacModExp Mod Exp
 *
 * @ingroup LacAsym
 *
 * internal interface for the modular exponentiation operation.
 *
 ******************************************************************************/

/***************************************************************************/

#ifndef _LAC_MOD_EXP_H_
#define _LAC_MOD_EXP_H_

/*
******************************************************************************
* Include public/global header files
******************************************************************************
*/

#include "cpa.h"
#include "lac_pke_qat_comms.h"
/*
*******************************************************************************
* Include private header files
*******************************************************************************
*/

/**
 ******************************************************************************
 * @ingroup LacModExp
 * @description
 *      This enum lists the types of mod exp operation.
 *****************************************************************************/
typedef enum
{
    LAC_MOD_EXP_OP = 0,
    /**< Mormal mod exp operation - optimised for Diffie Hellman */
    LAC_MOD_EXP_G2_OP
    /**< mod exp operation when exp = 2 - optimised for Diffie Hellman */
} lac_mod_exp_op_type_t;


/**
 ******************************************************************************
 * @ingroup LacModExp
 * @description
 *      This struct contains the parameters required to perform a  modular
 *      exponentiation operation.
 *****************************************************************************/
typedef struct lac_mod_exp_op_data_s
{
    lac_pke_op_cb_func_t pPkeOpCbFunc; 
    /**< function invoked when mod exp op is complete */
    lac_pke_op_cb_data_t *pPkeOpData; 
    /**< data received from the lac client */
    Cpa32U sizeInBits;
    /**< the size of the buffers in bits - must all be the same size*/
    const CpaFlatBuffer *pBase; 
    /**< the base number*/
    const CpaFlatBuffer *pExponent; 
    /**< the exponent to which the base is raised*/
    const CpaFlatBuffer *pModulus; 
    /**< the modulus. Note that most significant bit of modulus must be set. */
    CpaFlatBuffer *pResult; 
    /**< (base^exponent)%modulus is outputted here*/
    CpaInstanceHandle instanceHandle; 
    /**< the accel handle */
    lac_mod_exp_op_type_t opType; 
    /**< the type of mod exp to perform */
} lac_mod_exp_op_data_t;

/**
 *****************************************************************************
 * @ingroup LacModExp
 *      Function to accelerate Mod Exp operations. This does not block.
 *
 * @description
 *      This function may be used to accelerate Mod Exp operations. It formats
 *      and sends a message to the QAT given the input parameters. The caller
 *      specifies the base, exponent and modulus. The result is written to
 *      the result flat buffer. The result will be (base^exponent) % modulus.
 *
 *      This function does NOT block until the response has been received from
 *      the QAT. It returns after sending the message. The callback will be
 *      invoked when the operation has been completed.
 *
 * @param[in] pData contains the data required to perform the operation
 *
 * @retval CPA_STATUS_SUCCESS       Function executed successfully.
 * @retval CPA_STATUS_FAIL          Function failed.
 * @retval CPA_STATUS_RESOURCE      Error related to system resources.
 * @retval CPA_STATUS_RETRY         Retry the operation
 *****************************************************************************/
CpaStatus
LacModExp_Perform(lac_mod_exp_op_data_t *pData);

#endif /* _LAC_MOD_EXP_H_ */

