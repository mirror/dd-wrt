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
 * @file qatal_common.h
 *
 * @defgroup  QatalCommon   QAT Access Layer Common macros and definitions
 *
 * @ingroup icp_Qatal
 *
 * @description
 *      This file contains macros and enumerations that are used across all of
 *      QATAL.
 *
 *****************************************************************************/

#ifndef QATAL_COMMON_H
#define QATAL_COMMON_H

/*
******************************************************************************
* Include public/global header files
******************************************************************************
*/

/*
********************************************************************************
* Debug Macros and settings
********************************************************************************
*/


#define STATIC static




/**
*****************************************************************************
 * @ingroup Qatal
 *      Component state
 *
 * @description
 *      This enum is used to indicate the state that the component is in. Its
 *      purpose is to prevent components from being initialised or shutdown
 *      incorrectly.
 *
 *****************************************************************************/
typedef enum {
    QATAL_COMP_SHUT_DOWN = 0,
    /**< Component is in the Shut-down state */
    QATAL_COMP_INITIALISING,
    /**< Component in the Process of being initialised */
    QATAL_COMP_INITIALISED,
    /**< Component in the Initialised state */
    QATAL_COMP_STARTING,
    /**< Component is in the process of Starting */
    QATAL_COMP_STARTED,
    /**< Component is the Started state */
    QATAL_COMP_STOPPING,
    /**< Component is in the process of Stopping*/
    QATAL_COMP_STOPPED,
    /**< Component is in the Stopped state*/
    QATAL_COMP_SHUTTING_DOWN,
    /**< Component in the process of Shutting down */
}qatal_comp_state_t;


/**
 *******************************************************************************
 * @ingroup QatalCommon
 *      This macro checks if a paramater is NULL
 *
 * @param param  IN  Paramater
 *
 * @return CPA_STATUS_INVALID_PARAM Paramater is NULL
 * @return void                Paramater is not NULL
 ******************************************************************************/
#define QATAL_CHECK_NULL_PARAM(param)               \
do {                                                \
    if (NULL == (param))                            \
    {                                               \
        QATAL_ERROR_LOG(#param " is NULL");         \
        return CPA_STATUS_INVALID_PARAM;                 \
    }                                               \
} while(0)


/**
 *******************************************************************************
 * @ingroup QatalCommon
 *      This macro verifies that the Acceleration Handle is valid.
 *
 * @param instanceHandle IN Acceleration Handle
 *
 * @return void                 Acceleration Handle is ok
 * @return CPA_STATUS_INVALID_PARAM  The Acceleration Handle is invalid
 *
 ******************************************************************************/
#define QATAL_CHECK_ACCEL_HANDLE(instanceHandle)       \
do {                                                \
    if (ICP_LAC_ACCEL_HANDLE_NULL != instanceHandle)   \
    {                                               \
        return CPA_STATUS_INVALID_PARAM;                 \
    }                                               \
} while(0)

/*
********************************************************************************
* Alignment and Bit Operation Macros
********************************************************************************
*/

#define QATAL_NUM_BITS_IN_BYTE    (8)
/**< @ingroup QatalCommon
 * Number of bits in a byte */

#define QATAL_IA_WORD_SIZE_IN_BYTES   (4)
/**< @ingroup QatalCommon
 * Number of bytes in an IA word */

#define QATAL_QUAD_WORD_IN_BYTES  (8)
/**< @ingroup QatalCommon
 * Number of bytes in a QUAD word */

#define QATAL_8BYTE_ALIGNMENT_SHIFT   (3)
/**< @ingroup QatalCommon
 * 8 byte alignment to a power of 2 */

#define QATAL_64BYTE_ALIGNMENT_SHIFT  (6)
/**< @ingroup QatalCommon
 * 64 byte alignment to a power of 2 */

/**
 *******************************************************************************
 * @ingroup QatalCommon
 *      This macro checks if an address is aligned to the specified power of 2
 *      Returns 0 if alignment is ok, or non-zero otherwise
 *
 * @param address   the address we are checking
 *
 * @param alignment the byte alignment to check (specified as power of 2)
 *
 ******************************************************************************/
#define QATAL_ADDRESS_ALIGNED(address, alignment)\
    (!((Cpa32U)(address) & ((1 << (alignment)) - 1)))


/**
 *******************************************************************************
 * @ingroup QatalCommon
 *      This macro rounds up a number to a be a multiple of the alignment when
 *      the alignment is a power of 2.
 *
 * @param num   IN  Number
 * @param align IN  Alignement (must be a power of 2)
 *
 ******************************************************************************/
#define QATAL_ALIGN_POW2_ROUNDUP(num, align) \
    ( ((num) + (align) -1UL) & ~((align) - 1UL) )


/**
 *******************************************************************************
 * @ingroup QatalCommon
 *      This macro generates a bit mask to select a particular bit
 *
 * @param bitPos  IN  Bit position to select
 *
 ******************************************************************************/
#define QATAL_BIT(bitPos)       (0x1 << (bitPos))

/**
 *******************************************************************************
 * @ingroup QatalCommon
 *      This macro converts a size in bits to the equivalent size in bytes,
 *      using a bit shift to divide by 8
 *
 * @param x   IN  size in bits
 *
 ******************************************************************************/
#define QATAL_BITS_TO_BYTES(x) ((x) >> 3) 

/**
 *******************************************************************************
 * @ingroup QatalCommon
 *      This macro converts a size in bytes to the equivalent size in quadwords,
 *      using a bit shift to divide by 8
 *
 * @param x   IN  size in bytes
 *
 ******************************************************************************/
#define QATAL_BYTES_TO_QUADWORDS(x) ((x) >> 3) 


#endif /* QATAL_COMMON_H */
