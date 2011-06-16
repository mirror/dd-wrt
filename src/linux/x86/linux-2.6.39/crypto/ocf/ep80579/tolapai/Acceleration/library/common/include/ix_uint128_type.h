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


#if !defined(__IX_UINT128_TYPE_C__)
#define __IX_UINT128_TYPE_C__


#if defined(__cplusplus)
extern "C"
{
#endif /* end defined(__cplusplus) */


                                  
/**
 * MACRO NAME: IX_128_BIT_BINARY_OPERATION_ASSIGN
 *
 * DESCRIPTION: This macro assigns the arg_Second128BitOperand value to arg_First128BitOperand.
 *
 * @Param:  - INOUT arg_First128BitOperand - first ix_uint128 operand
 * @Param:  - IN arg_Second128BitOperand - second ix_uint128 operand
 *
 * @Return: Returns the new value of arg_First128BitOperand
 */
#define IX_128_BIT_BINARY_OPERATION_ASSIGN( \
                                        arg_First128BitOperand, \
                                        arg_Second128BitOperand \
                                      ) \
                                      (((arg_First128BitOperand).mUINT32[0] = (arg_Second128BitOperand).mUINT32[0]) , \
                                       ((arg_First128BitOperand).mUINT32[1] = (arg_Second128BitOperand).mUINT32[1]) , \
                                       ((arg_First128BitOperand).mUINT32[2] = (arg_Second128BitOperand).mUINT32[2]) , \
                                       ((arg_First128BitOperand).mUINT32[3] = (arg_Second128BitOperand).mUINT32[3]) , \
                                       (arg_First128BitOperand))


/**
 * MACRO NAME: IX_128_BIT_BINARY_OPERATION_32_BIT_ASSIGN_TO
 *
 * DESCRIPTION: This macro assigns the ix_uint32 array arg_32BitUintArray value to arg_128BitOperand.
 *  The size of the array must be at least 4.
 *
 * @Param:  - INOUT arg_128BitOperand - first ix_uint128 operand
 * @Param:  - IN arg_32BitUintArray - second ix_uint32 array type operand
 *
 * @Return: Returns the new value of arg_First128BitOperand
 */
#define IX_128_BIT_BINARY_OPERATION_32_BIT_ASSIGN_TO( \
                                        arg_128BitOperand, \
                                        arg_32BitUintArray \
                                      ) \
                                      (((arg_128BitOperand).mUINT32[0] = (arg_32BitUintArray)[0]) , \
                                       ((arg_128BitOperand).mUINT32[1] = (arg_32BitUintArray)[1]) , \
                                       ((arg_128BitOperand).mUINT32[2] = (arg_32BitUintArray)[2]) , \
                                       ((arg_128BitOperand).mUINT32[3] = (arg_32BitUintArray)[3]) , \
                                       (arg_128BitOperand))


/**
 * MACRO NAME: IX_128_BIT_BINARY_OPERATION_ASSIGN_TO_32_BIT
 *
 * DESCRIPTION: This macro assigns the arg_128BitOperand ix_uint128 value to ix_uint32 array arg_32BitUintArray.
 *  The size of the array must be at least 4.
 *
 * @Param:  - IN arg_32BitUintArray - first ix_uint32 array type operand
 * @Param:  - IN arg_128BitOperand - second ix_uint128 operand
 *
 * @Return: Returns the address of the first element to the modified arg_32BitUintArray
 */
#define IX_128_BIT_BINARY_OPERATION_ASSIGN_TO_32_BIT( \
                                        arg_32BitUintArray, \
                                        arg_128BitOperand \
                                      ) \
                                      (((arg_32BitUintArray)[0] = (arg_128BitOperand).mUINT32[0]) , \
                                       ((arg_32BitUintArray)[1] = (arg_128BitOperand).mUINT32[1]) , \
                                       ((arg_32BitUintArray)[2] = (arg_128BitOperand).mUINT32[2]) , \
                                       ((arg_32BitUintArray)[3] = (arg_128BitOperand).mUINT32[3]) , \
                                       (arg_32BitUintArray))


/**
 * MACRO NAME: IX_128_BIT_BINARY_OPERATION_16_BIT_ASSIGN_TO
 *
 * DESCRIPTION: This macro assigns the ix_uint16 array arg_16BitUintArray value to arg_128BitOperand.
 *  The size of the array must be at least 8.
 *
 * @Param:  - INOUT arg_128BitOperand - first ix_uint128 operand
 * @Param:  - IN arg_16BitUintArray - second ix_uint16 array type operand
 *
 * @Return: Returns the new value of arg_First128BitOperand

 */
#define IX_128_BIT_BINARY_OPERATION_16_BIT_ASSIGN_TO( \
                                        arg_128BitOperand, \
                                        arg_16BitUintArray \
                                      ) \
                                      (((arg_128BitOperand).mUINT16[0] = (arg_16BitUintArray)[0]) , \
                                       ((arg_128BitOperand).mUINT16[1] = (arg_16BitUintArray)[1]) , \
                                       ((arg_128BitOperand).mUINT16[2] = (arg_16BitUintArray)[2]) , \
                                       ((arg_128BitOperand).mUINT16[3] = (arg_16BitUintArray)[3]) , \
                                       ((arg_128BitOperand).mUINT16[4] = (arg_16BitUintArray)[4]) , \
                                       ((arg_128BitOperand).mUINT16[5] = (arg_16BitUintArray)[5]) , \
                                       ((arg_128BitOperand).mUINT16[6] = (arg_16BitUintArray)[6]) , \
                                       ((arg_128BitOperand).mUINT16[7] = (arg_16BitUintArray)[7]) , \
                                       (arg_128BitOperand))


/**
 * MACRO NAME: IX_128_BIT_BINARY_OPERATION_ASSIGN_TO_16_BIT
 *
 * DESCRIPTION: This macro assigns the arg_128BitOperand ix_uint128 value to ix_uint16 array arg_16BitUintArray.
 *  The size of the array must be at least 8.
 *
 * @Param:  - IN arg_16BitUintArray - first ix_uint16 array type operand
 * @Param:  - IN arg_128BitOperand - second ix_uint128 operand
 *
 * @Return: Returns the address of the first element to the modified arg_16BitUintArray

 */
#define IX_128_BIT_BINARY_OPERATION_ASSIGN_TO_16_BIT( \
                                        arg_16BitUintArray, \
                                        arg_128BitOperand \
                                      ) \
                                      (((arg_16BitUintArray)[0] = (arg_128BitOperand).mUINT16[0]) , \
                                       ((arg_16BitUintArray)[1] = (arg_128BitOperand).mUINT16[1]) , \
                                       ((arg_16BitUintArray)[2] = (arg_128BitOperand).mUINT16[2]) , \
                                       ((arg_16BitUintArray)[3] = (arg_128BitOperand).mUINT16[3]) , \
                                       ((arg_16BitUintArray)[4] = (arg_128BitOperand).mUINT16[4]) , \
                                       ((arg_16BitUintArray)[5] = (arg_128BitOperand).mUINT16[5]) , \
                                       ((arg_16BitUintArray)[6] = (arg_128BitOperand).mUINT16[6]) , \
                                       ((arg_16BitUintArray)[7] = (arg_128BitOperand).mUINT16[7]) , \
                                       (arg_16BitUintArray))



/**
 * MACRO NAME: IX_128_BIT_BINARY_OPERATION_8_BIT_ASSIGN_TO
 *
 * DESCRIPTION: This macro assigns the ix_uint8 array arg_8BitUintArray value to arg_128BitOperand.
 *  The size of the array must be at least 16.
 *
 * @Param:  - INOUT arg_128BitOperand - first ix_uint128 operand
 * @Param:  - IN arg_8BitUintArray - second ix_uint8 array type operand
 *
 * @Return: Returns the new value of arg_First128BitOperand

 */
#define IX_128_BIT_BINARY_OPERATION_8_BIT_ASSIGN_TO( \
                                        arg_128BitOperand, \
                                        arg_8BitUintArray \
                                      ) \
                                      (((arg_128BitOperand).mUINT8[0] = (arg_8BitUintArray)[0]) , \
                                       ((arg_128BitOperand).mUINT8[1] = (arg_8BitUintArray)[1]) , \
                                       ((arg_128BitOperand).mUINT8[2] = (arg_8BitUintArray)[2]) , \
                                       ((arg_128BitOperand).mUINT8[3] = (arg_8BitUintArray)[3]) , \
                                       ((arg_128BitOperand).mUINT8[4] = (arg_8BitUintArray)[4]) , \
                                       ((arg_128BitOperand).mUINT8[5] = (arg_8BitUintArray)[5]) , \
                                       ((arg_128BitOperand).mUINT8[6] = (arg_8BitUintArray)[6]) , \
                                       ((arg_128BitOperand).mUINT8[7] = (arg_8BitUintArray)[7]) , \
                                       ((arg_128BitOperand).mUINT8[8] = (arg_8BitUintArray)[8]) , \
                                       ((arg_128BitOperand).mUINT8[9] = (arg_8BitUintArray)[9]) , \
                                       ((arg_128BitOperand).mUINT8[10] = (arg_8BitUintArray)[10]) , \
                                       ((arg_128BitOperand).mUINT8[11] = (arg_8BitUintArray)[11]) , \
                                       ((arg_128BitOperand).mUINT8[12] = (arg_8BitUintArray)[12]) , \
                                       ((arg_128BitOperand).mUINT8[13] = (arg_8BitUintArray)[13]) , \
                                       ((arg_128BitOperand).mUINT8[14] = (arg_8BitUintArray)[14]) , \
                                       ((arg_128BitOperand).mUINT8[15] = (arg_8BitUintArray)[15]) , \
                                       (arg_128BitOperand))


/**
 * MACRO NAME: IX_128_BIT_BINARY_OPERATION_ASSIGN_TO_8_BIT
 *
 * DESCRIPTION: This macro assigns the arg_128BitOperand ix_uint128 value to ix_uint8 array arg_8BitUintArray.
 *  The size of the array must be at least 16.
 *
 * @Param:  - IN arg_8BitUintArray - first ix_uint8 array type operand
 * @Param:  - IN arg_128BitOperand - second ix_uint128 operand
 *
 * @Return: Returns the address of the first element to the modified arg_32BitUintArray

 */
#define IX_128_BIT_BINARY_OPERATION_ASSIGN_TO_8_BIT( \
                                        arg_8BitUintArray, \
                                        arg_128BitOperand \
                                      ) \
                                      (((arg_8BitUintArray)[0] = (arg_128BitOperand).mUINT8[0]) , \
                                       ((arg_8BitUintArray)[1] = (arg_128BitOperand).mUINT8[1]) , \
                                       ((arg_8BitUintArray)[2] = (arg_128BitOperand).mUINT8[2]) , \
                                       ((arg_8BitUintArray)[3] = (arg_128BitOperand).mUINT8[3]) , \
                                       ((arg_8BitUintArray)[4] = (arg_128BitOperand).mUINT8[4]) , \
                                       ((arg_8BitUintArray)[5] = (arg_128BitOperand).mUINT8[5]) , \
                                       ((arg_8BitUintArray)[6] = (arg_128BitOperand).mUINT8[6]) , \
                                       ((arg_8BitUintArray)[7] = (arg_128BitOperand).mUINT8[7]) , \
                                       ((arg_8BitUintArray)[8] = (arg_128BitOperand).mUINT8[8]) , \
                                       ((arg_8BitUintArray)[9] = (arg_128BitOperand).mUINT8[9]) , \
                                       ((arg_8BitUintArray)[10] = (arg_128BitOperand).mUINT8[10]) , \
                                       ((arg_8BitUintArray)[11] = (arg_128BitOperand).mUINT8[11]) , \
                                       ((arg_8BitUintArray)[12] = (arg_128BitOperand).mUINT8[12]) , \
                                       ((arg_8BitUintArray)[13] = (arg_128BitOperand).mUINT8[13]) , \
                                       ((arg_8BitUintArray)[14] = (arg_128BitOperand).mUINT8[14]) , \
                                       ((arg_8BitUintArray)[15] = (arg_128BitOperand).mUINT8[15]) , \
                                       (arg_8BitUintArray))

/**
 * MACRO NAME: IX_128_BIT_BINARY_OPERATION_EQ
 *
 * DESCRIPTION: This macro performs a comparison between the two argument ix_uint128 values.
 *
 * @Param:  - INOUT arg_First128BitOperand - first ix_uint128 operand
 * @Param:  - IN arg_Second128BitOperand - second ix_uint128 operand
 *
 * @Return: Returns TRUE (non zero) if the values are equal.
 */
#define IX_128_BIT_BINARY_OPERATION_EQ( \
                                        arg_First128BitOperand, \
                                        arg_Second128BitOperand \
                                      ) \
                                      (((arg_First128BitOperand).mUINT32[0] == (arg_Second128BitOperand).mUINT32[0]) && \
                                       ((arg_First128BitOperand).mUINT32[1] == (arg_Second128BitOperand).mUINT32[1]) && \
                                       ((arg_First128BitOperand).mUINT32[2] == (arg_Second128BitOperand).mUINT32[2]) && \
                                       ((arg_First128BitOperand).mUINT32[3] == (arg_Second128BitOperand).mUINT32[3]))


/**
 * MACRO NAME: IX_128_BIT_BINARY_OPERATION_NEQ
 *
 * DESCRIPTION: This macro performs a comparison between the two argument ix_uint128 values.
 *
 * @Param:  - INOUT arg_First128BitOperand - first ix_uint128 operand
 * @Param:  - IN arg_Second128BitOperand - second ix_uint128 operand
 *
 * @Return: Returns TRUE (non zero) if the values are non equal.
 */
#define IX_128_BIT_BINARY_OPERATION_NEQ( \
                                         arg_First128BitOperand, \
                                         arg_Second128BitOperand \
                                       ) \
                                       (((arg_First128BitOperand).mUINT32[0] != (arg_Second128BitOperand).mUINT32[0]) || \
                                        ((arg_First128BitOperand).mUINT32[1] != (arg_Second128BitOperand).mUINT32[1]) || \
                                        ((arg_First128BitOperand).mUINT32[2] != (arg_Second128BitOperand).mUINT32[2]) || \
                                        ((arg_First128BitOperand).mUINT32[3] != (arg_Second128BitOperand).mUINT32[3]))


/**
 * MACRO NAME: IX_128_BIT_BINARY_OPERATION_AND
 *
 * DESCRIPTION: This macro performs the equivalent bit AND operation arg_First128BitOperand &= arg_Second128BitOperand.
 *
 * @Param:  - INOUT arg_First128BitOperand - first ix_uint128 operand
 * @Param:  - IN arg_Second128BitOperand - second ix_uint128 operand
 *
 * @Return: Returns the new value of arg_First128BitOperand
 */
#define IX_128_BIT_BINARY_OPERATION_AND( \
                                         arg_First128BitOperand, \
                                         arg_Second128BitOperand \
                                       ) \
                                       (((arg_First128BitOperand).mUINT32[0] &= (arg_Second128BitOperand).mUINT32[0]) , \
                                        ((arg_First128BitOperand).mUINT32[1] &= (arg_Second128BitOperand).mUINT32[1]) , \
                                        ((arg_First128BitOperand).mUINT32[2] &= (arg_Second128BitOperand).mUINT32[2]) , \
                                        ((arg_First128BitOperand).mUINT32[3] &= (arg_Second128BitOperand).mUINT32[3]) , \
                                        (arg_First128BitOperand))

/**
 * MACRO NAME: IX_128_BIT_BINARY_OPERATION_OR
 *
 * DESCRIPTION: This macro performs the equivalent bit OR operation arg_First128BitOperand |= arg_Second128BitOperand.
 *
 * @Param:  - INOUT arg_First128BitOperand - first ix_uint128 operand
 * @Param:  - IN arg_Second128BitOperand - second ix_uint128 operand
 *
 * @Return: Returns the new value of arg_First128BitOperand
 */
#define IX_128_BIT_BINARY_OPERATION_OR( \
                                        arg_First128BitOperand, \
                                        arg_Second128BitOperand \
                                      ) \
                                      (((arg_First128BitOperand).mUINT32[0] |= (arg_Second128BitOperand).mUINT32[0]) , \
                                       ((arg_First128BitOperand).mUINT32[1] |= (arg_Second128BitOperand).mUINT32[1]) , \
                                       ((arg_First128BitOperand).mUINT32[2] |= (arg_Second128BitOperand).mUINT32[2]) , \
                                       ((arg_First128BitOperand).mUINT32[3] |= (arg_Second128BitOperand).mUINT32[3]) , \
                                       (arg_First128BitOperand))


/**
 * MACRO NAME: IX_128_BIT_BINARY_OPERATION_XOR
 *
 * DESCRIPTION: This macro performs the equivalent bit XOR operation arg_First128BitOperand ^= arg_Second128BitOperand.
 *
 * @Param:  - INOUT arg_First128BitOperand - first ix_uint128 operand
 * @Param:  - IN arg_Second128BitOperand - second ix_uint128 operand
 *
 * @Return: Returns the new value of arg_First128BitOperand
 */
#define IX_128_BIT_BINARY_OPERATION_XOR( \
                                         arg_First128BitOperand, \
                                         arg_Second128BitOperand \
                                       ) \
                                       (((arg_First128BitOperand).mUINT32[0] ^= (arg_Second128BitOperand).mUINT32[0]) , \
                                        ((arg_First128BitOperand).mUINT32[1] ^= (arg_Second128BitOperand).mUINT32[1]) , \
                                        ((arg_First128BitOperand).mUINT32[2] ^= (arg_Second128BitOperand).mUINT32[2]) , \
                                        ((arg_First128BitOperand).mUINT32[3] ^= (arg_Second128BitOperand).mUINT32[3]) , \
                                        (arg_First128BitOperand))


/**
 * MACRO NAME: IX_128_BIT_BINARY_OPERATION_SHIFT_LEFT
 *
 * DESCRIPTION: This macro performs the equivalent left shift operation arg_128BitOperand <<= arg_Shift32Bit.
 *
 * @Param:  - INOUT arg_128BitOperand - first ix_uint128 operand
 * @Param:  - IN arg_Shift32Bit - second ix_uint32 operand
 *
 * @Return: Returns nothing. Upon return arg_128BitOperand will be updated to the new value.
 */
#define IX_128_BIT_BINARY_OPERATION_SHIFT_LEFT( \
                                         arg_128BitOperand, \
                                         arg_Shift32Bit \
                                       ) \
                { \
                    ix_uint32 a = (arg_Shift32Bit) , b = a >> 5 , i = 3; \
                    if(b < 4) \
                    { \
                        a &= 0x1f; \
                        if(a == 0) \
                        { \
                            for( ; i > b ; i--) \
                                (arg_128BitOperand).mUINT32[i] = (arg_128BitOperand).mUINT32[i - b]; \
                            (arg_128BitOperand).mUINT32[b] = (arg_128BitOperand).mUINT32[0]; \
                            for(i = 0; i < b ; i++) \
                                (arg_128BitOperand).mUINT32[i] = 0; \
                        } \
                        else \
                        { \
                            ix_uint32 c = ~a & 0x1f; \
                            for( ; i > b ; i--) \
                                (arg_128BitOperand).mUINT32[i] = ((arg_128BitOperand).mUINT32[i - b] << a) | \
                                        ((arg_128BitOperand).mUINT32[i - b - 1] >> c); \
                            (arg_128BitOperand).mUINT32[i] = (arg_128BitOperand).mUINT32[0] << a; \
                            for(i = 0; i < b ; i++) \
                                (arg_128BitOperand).mUINT32[i] = 0; \
                        } \
                    } \
                }

/**
 * MACRO NAME: IX_128_BIT_BINARY_OPERATION_SHIFT_RIGHT
 *
 * DESCRIPTION: This macro performs the equivalent right shift operation arg_128BitOperand >>= arg_Shift32Bit.
 *
 * @Param:  - INOUT arg_128BitOperand - first ix_uint128 operand
 * @Param:  - IN arg_Shift32Bit - second ix_uint32 operand
 *
 * @Return: Returns nothing. Upon return arg_128BitOperand will be updated to the new value.
 */
#define IX_128_BIT_BINARY_OPERATION_SHIFT_RIGHT( \
                                         arg_128BitOperand, \
                                         arg_Shift32Bit \
                                       ) \
                { \
                    ix_uint32 a = (arg_Shift32Bit) , b = a >> 5 , i = 0; \
                    if(b < 4) \
                    { \
                        a &= 0x1f; \
                        if(a == 0) \
                        { \
                            for( ; i < 4 - b ; i++) \
                                (arg_128BitOperand).mUINT32[i] = (arg_128BitOperand).mUINT32[i + b]; \
                            for(; i < 4 ; i++) \
                                (arg_128BitOperand).mUINT32[i] = 0; \
                        } \
                        else \
                        { \
                            ix_uint32 c = ~a & 0x1f; \
                            for( ; i < 3 - b ; i++) \
                                (arg_128BitOperand).mUINT32[i] = ((arg_128BitOperand).mUINT32[i + b] >> a) | \
                                        ((arg_128BitOperand).mUINT32[i + b + 1] << c); \
                            (arg_128BitOperand).mUINT32[i++] = (arg_128BitOperand).mUINT32[3] >> a; \
                            for(; i < 4 ; i++) \
                                (arg_128BitOperand).mUINT32[i] = 0; \
                        } \
                    } \
                }

#if defined(__cplusplus)
}
#endif /* end defined(__cplusplus) */

#endif /* end !defined(__IX_UINT128_TYPE_C__) */

