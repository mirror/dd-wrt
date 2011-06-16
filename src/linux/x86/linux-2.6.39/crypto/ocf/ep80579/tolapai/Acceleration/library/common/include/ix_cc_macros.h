/******************************************************************************
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

#if !defined(__IX_CC_MACROS_H__)
#define __IX_CC_MACROS_H__


#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */


/*
 * If the OPTIONAL flag isn't already turned on, turn it on for DEBUG builds.
 */
#if (!defined(OPTIONAL) && defined(IX_DEBUG))
#define OPTIONAL
#endif /* !OPTIONAL && IX_DEBUG */

/*
 * IX_CC_OPTIONAL provides a simple way to turn on or off a block of code
 * based on the OPTIONAL build flag.  Use it like:
 * IX_CC_OPTIONAL(t = 0;)
 * or
 * IX_CC_OPTIONAL(
 *      i = 0;
 *      s = "";
 * )
 *
 * The lines inside the macro will either be compiled in or not, based on
 * the OPTIONAL build flag.
 */
#ifdef OPTIONAL
#define IX_CC_OPTIONAL(__s) __s
#else
#define IX_CC_OPTIONAL(__s)
#endif

/**
 * In some cases, wrap the statement in a do while zero.  This ensures a
 * statement is not "misused" -- i.e. is only used as a statement, not as an
 * expression.  Remove for TestRT because having the do while affects the line
 * coverage and code complexity scores.  Remove for Release because the macro
 * is only useful at compile-time, to ensure some macros are properly used.
 *
 * This macro should be used to create other macros.
 */
#if !defined(IX_DEBUG) || defined(TESTRT) 
#define IX_CC_WRAP(__s) __s
#else
#define IX_CC_WRAP(__s) do { __s; } while (0)
#endif

/*
 * IX_MIN returns the minimum value of 'a' or 'b'.
 */
#define IX_MIN(a, b) ((a) < (b)) ? (a) : (b)

/*
 * IX_MAX returns the maximum value of 'a' or 'b'.
 */
#define IX_MAX(a, b) ((a) > (b)) ? (a) : (b)

/*IX_INCLUDE_REGISTRY and IX_EXCLUDE_CCI have been removed from here as they need
to defined at the GLOBAL level as compile-time defines*/

/* Comment out this line if you do not want IPv6 support. */
/*#define IX_IPV6_SUPPORT 1*/

/* Comment out this line if you do not want Ethernet support. */
/*#define IX_ETHERNET_INCLUDE 1*/

/* Number of bytes to reserve at head of SOP HW buffers allocated on the core. */
#define IX_CC_SOP_HW_BUFFER_PREPEND     128

/**
*
* defines max size for ublock image file
*/
#define IX_UBLOCK_IMAGE_NAME_MAX_SIZE   128  

/**
*   hz - following macros are for combining SRAM channel number and offset into one
*   long word for patching it to the microblocks. It is prefered way for microblocks 
*   to receive base address of shared SRAM 
*/

/**
 * DESCRIPTION: Least significant bit of SRAM channel number field into a physical offset
 *  
 */
#define IX_CHANNEL_NUMBER_LSB     30U

 /**
 * DESCRIPTION: Max value for the channel number
 *  4 channels - 0,1,2, and 3
 */
#define IX_CHANNEL_NUM_MAX_VALUE   3
 /**
 * DESCRIPTION: 32-bit number with channel bits set to 1's 
 */
#define IX_CHANNEL_NUMBER_BITS    (IX_CHANNEL_NUM_MAX_VALUE << IX_CHANNEL_NUMBER_LSB)
 
 /**
 * MACRO NAME: IX_CC_SET_SRAM_CHANNEL_INTO_OFFSET
 *
 * DESCRIPTION: This macro sets channel number into physical offset adress into
 * two upper bits of the long word with the following mapping
 *   00 - channel 0
 *   01 - channel 1
 *   10 - channel 2
 *   11 - channel 3
 *  @Param:  - IN arg_ChannelNumber -  channel number
 *  @Param:  - INOUT arg_Offset   - physical offset
  * @Return: packed long word inside the offset. 
 */
#define IX_CC_SET_SRAM_CHANNEL_INTO_OFFSET(arg_ChannelNumber,arg_Offset) \
    IX_CC_WRAP(arg_Offset &= ~IX_CHANNEL_NUMBER_BITS; \
               arg_Offset |= (arg_ChannelNumber << IX_CHANNEL_NUMBER_LSB))

/**
 * MACRO NAME: IX_CC_GET_SRAM_CHANNEL_AND_OFFSET
 *
 * DESCRIPTION: This macro reads channel number and physical offset from the
 *              value prepared for patching to microcode by the
 *              IX_CC_SET_SRAM_CHANNEL_INTO_OFFSET macro.
 * NOTE: It is allowed to use the same variable for the input arg_Value and
 *       one of output arguments.
 *  @Param:  - IN  arg_Value   - value containing channel and offset
 *  @Param:  - OUT arg_Channel - store the channel number here
 *  @Param:  - OUT arg_Offset  - store the physical offset here
 * @Return: None
 */
#define IX_CC_GET_SRAM_CHANNEL_AND_OFFSET(arg_Value, arg_Channel, arg_Offset) \
        {                                                                     \
            ix_uint32 val = (ix_uint32)(arg_Value);                           \
            (arg_Channel) = val >> IX_CHANNEL_NUMBER_LSB;                     \
            (arg_Offset) = val & ~IX_CHANNEL_NUMBER_BITS;                     \
        }

/* end of channel macros */

/* Macros to output characters to screen */
#define IX_CC_PRINT         ixOsalStdLog 

/**
 * Macro for definition of __declspec qualifier used by common header files 
 * created for microblocks originally.  The __declspec is used by MicroC 
 * compiler but can not be interpreted by C compiler for XScale.
 * This macro is created for ATM RAN application on IXDP2351.
 */
#if defined(IX_PLATFORM_2351)
#define __declspec(packed)   __attribute__ ((packed))
#endif


#if defined(__cplusplus)
} /* extern "C" */
#endif /* __cplusplus */

#endif /*__IX_CC_MACROS_H__*/
