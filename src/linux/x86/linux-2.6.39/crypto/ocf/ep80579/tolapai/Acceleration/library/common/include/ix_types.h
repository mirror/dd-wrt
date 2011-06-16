/*******************************************************************************
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


#if !defined(__IX_TYPES_H__)
#define __IX_TYPES_H__

#include "ix_os_type.h"

#if defined(__cplusplus)
extern "C"
{
#endif /* end defined(__cplusplus) */

#include "IxOsalTypes.h"

/**
 * Define generic integral data types that will guarantee the size.
 */

/**
 * TYPENAME: ix_int8
 * 
 * DESCRIPTION: This type defines an 8 bit signed integer value.
 *
 */
typedef CHAR ix_int8;


/**
 * TYPENAME: ix_uint8
 * 
 * DESCRIPTION: This type defines an 8 bit unsigned integer value.
 *
 */
typedef UCHAR ix_uint8;


/**
 * TYPENAME: ix_int16
 * 
 * DESCRIPTION: This type defines an 16 bit signed integer value.
 *
 */
typedef INT16 ix_int16;


/**
 * TYPENAME: ix_uint16
 * 
 * DESCRIPTION: This type defines an 16 bit unsigned integer value.
 *
 */
typedef UINT16 ix_uint16;


/**
 * TYPENAME: ix_int32
 * 
 * DESCRIPTION: This type defines an 32 bit signed integer value.
 *
 */
typedef INT32 ix_int32;


/**
 * TYPENAME: ix_uint32
 * 
 * DESCRIPTION: This type defines an 32 bit unsigned integer value.
 *
 */
typedef UINT32 ix_uint32;


/**
 * TYPENAME: ix_int64
 * 
 * DESCRIPTION: This type defines an 64 bit signed integer value.
 *
 */
typedef INT64 ix_int64;

/**
 * TYPENAME: ix_uint64
 * 
 * DESCRIPTION: This type defines an 64 bit unsigned integer value.
 *
 */
typedef UINT64 ix_uint64;




/**
 * TYPENAME: ix_uint128
 * 
 * DESCRIPTION: 
 *
 */
typedef union ix_u_uint128
{
    ix_uint8    aUint8[16];
    ix_uint16   aUint16[8];
    ix_uint32   aUint32[4];
    ix_uint64   aUint64[2];
} ix_uint128;



/**
 * Supported mnemonics for the next macro are:
 *
 *  ASSIGN              - assigns second ix_uint128 argument value to the first ix_uint128 argument
 *  32_BIT_ASSIGN_TO    - assigns second ix_uint32 array argument value to the first ix_uint128 argument
 *  ASSIGN_TO_32_BIT    - assigns second ix_uint128 argument value to the first ix_uint32 array argument
 *  16_BIT_ASSIGN_TO    - assigns second ix_uint16 array argument value to the first ix_uint128 argument
 *  ASSIGN_TO_16_BIT    - assigns second ix_uint128 argument value to the first ix_uint16 array argument
 *  8_BIT_ASSIGN_TO     - assigns second ix_uint8 array argument value to the first ix_uint128 argument
 *  ASSIGN_TO_8_BIT     - assigns second ix_uint128 argument value to the first ix_uint8 array argument
 *  EQ                  - compares the two ix_uint128 argument values and returns TRUE (non zero) 
 *                      if they are equal
 *  NEQ                 - compares the two ix_uint128 argument values and returns TRUE (non zero) 
 *                      if they are not equal
 *  AND                 - performs a a &= b equivalent operation withe the two ix_uint128 argument values
 *  OR                  - performs a a |= b equivalent operation withe the two ix_uint128 argument values 
 *  XOR                 - performs a a ^= b equivalent operation withe the two ix_uint128 argument values
 *  SHIFT_LEFT          - performs a left shift of the first ix_uint128 argument value by the second ix_uint32
 *                      argument value
 *  RIGHT_LEFT          - performs a right shift of the first ix_uint128 argument value by the second ix_uint32
 *                      argument value
 *
 *
 *  Some other mnemonics could be added based on the newly created operations
 */

/**
 * MACRO NAME: IX_128_BIT_BINARY_OPERATION
 *
 * DESCRIPTION: This macro resolves into the macros obtained through pasting with the second argument
 *  operator mnemonic. See the above comment for the available mnemonics.
 *
 * @Param:  - IN arg_FirstOperand - first operand to be passed to the resulting macro
 * @Param:  - IN arg_OperatorMnemonic - the mnemonic for the binary operation operator
 * @Param:  - IN arg_SecondOperand - second operand to be passed to the resulting macro
 *
 * @Return: Returns the value returned by the resulting macro
 */
#define IX_128_BIT_BINARY_OPERATION( \
                                     arg_FirstOperand, \
                                     arg_OperatorMnemonic, \
                                     arg_SecondOperand \
                                   ) \
                                   IX_128_BIT_BINARY_OPERATION_##arg_OperatorMnemonic( \
                                        arg_FirstOperand, \
                                        arg_SecondOperand \
                                    )



/**
 * Include the file with macro implementation of the ix_uint128 operations
 */
#include "ix_uint128_type.h"


/**
 * TYPENAME: ix_bit_mask8
 * 
 * DESCRIPTION: This is a generic type for a 8 bit mask. 
 */
typedef ix_uint8 ix_bit_mask8;


/**
 * TYPENAME: ix_bit_mask16
 * 
 * DESCRIPTION: This is a generic type for a 16 bit mask. 
 */
typedef ix_uint16 ix_bit_mask16;


/**
 * TYPENAME: ix_bit_mask32
 * 
 * DESCRIPTION: This is a generic type for a 32 bit mask. 
 */
typedef ix_uint32 ix_bit_mask32;


/**
 * TYPENAME: ix_bit_mask64
 * 
 * DESCRIPTION: This is a generic type for a 64 bit mask. 
 */
typedef ix_uint64 ix_bit_mask64;



/**
 * TYPENAME: ix_handle
 * 
 * DESCRIPTION: This type defines a generic handle.
 *
 */
typedef ix_uint32 ix_handle;



/**
 * DESCRIPTION: This symbol defines a NULL handle
 *
 */
#define IX_NULL_HANDLE   ((ix_handle)0) 



/**
 * TYPENAME: ix_bool
 * 
 * DESCRIPTION: This type define an IXA SDK boolean type
 *
 */
typedef ix_uint32 ix_bool;



/**
 * DESCRIPTION: This symbol defines the FALSE boolean type
 *
 */
#define IX_FALSE    ((ix_bool)0)


/**
 * DESCRIPTION: This symbol defines the TRUE boolean type
 *
 */
#define IX_TRUE     ((ix_bool)1)



/**
 * DESCRIPTION: This symbol defines the semaphore available type
 *
 */
#define IX_SEM_AVAILABLE     1


/**
 * DESCRIPTION: This symbol defines the semaphore unavailable type
 *
 */
#define IX_SEM_UNAVAILABLE     0

/**
 * DESCRIPTION: This symbol defines the error timeout type
 *
 */
#define IX_ERROR_TIMEOUT 3
 
/**
 * DESCRIPTION: This symbol defines the thread high priority type
 *
 */
#define IX_THREAD_PRI_HIGH 128 


/**
 * DESCRIPTION: This symbol defines the thread medium high priority type
 *
 */
#define IX_THREAD_PRI_MEDIUM_HIGH  96


/**
 * DESCRIPTION: This symbol defines the thread medium priority type
 *
 */
#define IX_THREAD_PRI_MEDIUM  64


/**
 * DESCRIPTION: This symbol defines the thread medium low priority type
 *
 */
#define IX_THREAD_PRI_MEDIUM_LOW  32


/**
 * DESCRIPTION: This symbol defines the thread low priority type
 *
 */
#define IX_THREAD_PRI_LOW  0








#if defined(__cplusplus)
}
#endif /* end defined(__cplusplus) */

#endif /* end !defined(__IX_TYPES_H__) */
