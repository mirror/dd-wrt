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

/**
 *****************************************************************************
 *
 * @file platform.c
 *
 * @description
 *     This file contains implementation for all the exported APIs and 
 *     definitions specific to EP805XX Platform.
 *
 * @revision
 *
 ****************************************************************************/

#include "IxOsal.h"

#include "ix_os_type.h"
#include "ix_symbols.h"
#include "ix_types.h"
#include "ix_macros.h"
#include "ix_error.h"

#include "platform.h"

#include "rm/symbols.h"
#include "rm/debug_component.h"
#include "rm/internal/internal_support.h"
#include "rm/sync_mechanism.h"
#include "rm/internal/internal_sync.h"
#include "rm/system.h"
#include "rm/internal/internal_system.h"
#include "rm/internal/internal_trace.h"



#if defined(_IX_RM_IMPL_SIMULATION_)

#include "rm/system.h"

#include <simIo.h>

#endif /* defined(_IX_RM_IMPL_SIMULATION_) */



/**
 *****************************************************************************
 * @ingroup rm_platform
 *     Accelengine Numbers
 *
 * @description
 *     This array defines the accelengine number mapping
 *
 * @purpose
 *
 *
 *****************************************************************************/
unsigned char ix_accelengine_number_mapping[IX_MAX_ACCELENGINE_NUMBER + 1] =
{
    /* 0 */ 0x00,
    /* 1 */ 0x01,
    /* 2 */ 0x02,
    /* 3 */ 0x03,

    /* 4 */ 0x04, /* Extra to handle code organization */
};


/**
 * NAME: _ix_rm_platform_check_for_rmsupp_memtype
 *
 * DESCRIPTION: This function will verify if the memory is
 *  accessible by RM. Will return err in the case of failure 
 *  (i.e. this is not a RM DRAM managed address)
 * 
 * @Param:  - IN arg_MemType - Type of Memory being accessed
 *
 * @Return: IX_SUCCESS if successful.
 *          IX_RM_ERROR_BAD_MEMORY_TYPE if memory type is not valid.
 *          
 */
ix_error _ix_rm_platform_check_for_rmsupp_memtype (ix_uint32 arg_MemType)
{
    ix_error err = IX_SUCCESS;

    
    static const char s_aFunctionIdentifier[] = 
                                 "_ix_rm_platform_check_for_rmsupp_memtype";
    
    IX_RM_ENTER_TRACE("_ix_rm_platform_check_for_rmsupp_memtype",
                        TRACE_DEFAULT_STRING, 
                        TRACE_DEFAULT_LEVEL,
                        ("arg_MemType = %d \n"
                        ,(ix_uint32)arg_MemType));

    IX_RM_UNUSED_VARIABLE(s_aFunctionIdentifier);


    /* memory type is is not valid then return error */
    if (arg_MemType >= IX_MEMORY_TYPE_LAST)
    {
#ifdef _IX_RM_DEBUG_

        ixOsalStdLog("%s: Invalid memory type %u\n",
                                s_aFunctionIdentifier, arg_MemType);
#endif

        IX_RM_EXIT_TRACE("_ix_rm_platform_check_for_rmsupp_memtype"
                  , TRACE_DEFAULT_STRING, TRACE_DEFAULT_LEVEL ); 
        err = IX_RM_ERROR_NEW(IX_RM_ERROR_BAD_MEMORY_TYPE, 
                              s_aFunctionIdentifier, 
                "this memory is inavlid and cannot be accessed!", err);
        return err;
     
    }
    
    
    IX_RM_EXIT_TRACE("_ix_rm_platform_check_for_rmsupp_memtype"
                             , TRACE_DEFAULT_STRING, TRACE_DEFAULT_LEVEL ); 
    return IX_SUCCESS; 

}


/**
 * NAME: _ix_rm_platform_check_for_rmsupp_memory
 *
 * DESCRIPTION: This function will verify if the memory is
 *  accessible by RM. Will return err in the case of failure 
 *  (i.e. this is not a RM DRAM managed address)
 * 
 * @Param:  - IN arg_MemType - Type of Memory being accessed
 * @Param:  - IN arg_MemChannel - Channel Number for the specified memory type
 *
 * @Return: IX_SUCCESS if successful.
 *          IX_RM_ERROR_BAD_MEMORY_TYPE if memory type is not valid.
 *          IX_RM_ERROR_BAD_MEMORY_CHANNEL if memory channel is not valid.
 *          
 */
ix_error _ix_rm_platform_check_for_rmsupp_memory (
                                            ix_uint32 arg_MemType,
                                            ix_uint32 arg_MemChannel
                                          )
{
    ix_error err = IX_SUCCESS;

    
    static const char s_aFunctionIdentifier[] = 
                                   "_ix_rm_platform_check_for_rmsupp_memory";
    
    IX_RM_ENTER_TRACE("_ix_rm_platform_check_for_rmsupp_memory",
                        TRACE_DEFAULT_STRING, 
                        TRACE_DEFAULT_LEVEL,
                        ("arg_MemType = %d \n arg_MemChannel = %d \n",
                        (ix_uint32)arg_MemType, 
                        (ix_uint32)arg_MemChannel));

    IX_RM_UNUSED_VARIABLE(s_aFunctionIdentifier);


    /* memory type is is not valid then return error */
    if (arg_MemType >= IX_MEMORY_TYPE_LAST)
    {
#ifdef _IX_RM_DEBUG_

        ixOsalStdLog("%s: Invalid memory type %u\n",
                                s_aFunctionIdentifier, arg_MemType);
#endif

        IX_RM_EXIT_TRACE("_ix_rm_platform_check_for_rmsupp_memory"
                  , TRACE_DEFAULT_STRING, TRACE_DEFAULT_LEVEL ); 
        err = IX_RM_ERROR_NEW(IX_RM_ERROR_BAD_MEMORY_TYPE, 
                                s_aFunctionIdentifier, 
                "this memory is invalid and cannot be accessed!", err);
        return err;
     
    }
    

    /* Check if the operation in DRAM is in CDRAM or NCDRAM */
    if ((arg_MemType == IX_MEMORY_TYPE_DRAM) &&
                    (arg_MemChannel >= IX_DRAM_CHANNELS_NUMBER))
    {
#ifdef _IX_RM_DEBUG_

        ixOsalStdLog("%s: Invalid DRAM memory channel %u\n",
                                s_aFunctionIdentifier, arg_MemChannel);
#endif

        IX_RM_EXIT_TRACE("_ix_rm_platform_check_for_rmsupp_memory"
                  , TRACE_DEFAULT_STRING, TRACE_DEFAULT_LEVEL ); 
        err = IX_RM_ERROR_NEW(IX_RM_ERROR_BAD_MEMORY_CHANNEL, 
                              s_aFunctionIdentifier, 
                "this memory is invalid and cannot be accessed!", err);
        return err;
     
    }
    
    
    IX_RM_EXIT_TRACE("_ix_rm_platform_check_for_rmsupp_memory"
                             , TRACE_DEFAULT_STRING, TRACE_DEFAULT_LEVEL ); 
    return IX_SUCCESS; 

}




/**
 * NAME: _ix_rm_platform_check_for_rmsupp_ring_memory
 *
 * DESCRIPTION: This function will verify if the memory passed can support a 
 * ring that can been accessed by RM. 
 * Will return err in the case of failure
 * 
 * @Param:  - IN arg_MemType - Type of Memory being accessed
 * #Param:  - IN arg_MemChannel - memory channel number where the ring resides
 *
 * @Return: IX_SUCCESS if successful.
 *          IX_RM_ERROR_HW_RING_MEM_NOT_SUPPORTED if hardware ring is not
 *              supported in that particular memory.
 */
ix_error _ix_rm_platform_check_for_rmsupp_ring_memory (
                                            ix_uint32 arg_MemType,
                                            ix_uint32 arg_MemChannel    
                                        )
{
    ix_error err = IX_SUCCESS;

    
    static const char s_aFunctionIdentifier[] = 
                             "_ix_rm_platform_check_for_rmsupp_ring_memory";
    
    IX_RM_ENTER_TRACE("_ix_rm_platform_check_for_rmsupp_ring_memory",
                        TRACE_DEFAULT_STRING, TRACE_DEFAULT_LEVEL ,
                        ("arg_MemType = %d \n arg_MemChannel = %d \n"
                        ,(ix_uint32)arg_MemType
                        ,(ix_uint32)arg_MemChannel));

    IX_RM_UNUSED_VARIABLE(s_aFunctionIdentifier);


    /* memory type is is not valid then return error */
    if (arg_MemType >= IX_MEMORY_TYPE_LAST)
    {
#ifdef _IX_RM_DEBUG_

        ixOsalStdLog("%s: Invalid ring memory type %u\n",
                                s_aFunctionIdentifier, arg_MemType);
#endif

        IX_RM_EXIT_TRACE("_ix_rm_platform_check_for_rmsupp_ring_memory"
                  , TRACE_DEFAULT_STRING, TRACE_DEFAULT_LEVEL ); 
        err = IX_RM_ERROR_NEW(IX_RM_ERROR_HW_RING_MEM_NOT_SUPPORTED, 
                              s_aFunctionIdentifier, 
                "the specified ring memory is invalid!", err);
        return err;
     
    }

    if (arg_MemChannel >= IX_DRAM_CHANNELS_NUMBER)
    {
#ifdef _IX_RM_DEBUG_

        ixOsalStdLog("%s: Invalid ring memory channel number %u\n",
                                s_aFunctionIdentifier, arg_MemChannel);
#endif

    {IX_RM_EXIT_TRACE("_ix_rm_platform_check_for_rmsupp_ring_memory"
                  , TRACE_DEFAULT_STRING, TRACE_DEFAULT_LEVEL ); 
     err = IX_RM_ERROR_NEW(IX_RM_ERROR_HW_RING_MEM_NOT_SUPPORTED, 
                           s_aFunctionIdentifier, 
                "the specified ring memory is invalid!", err);
     return err;}
     
    }
        
    
    IX_RM_EXIT_TRACE("_ix_rm_platform_check_for_rmsupp_ring_memory"
                             , TRACE_DEFAULT_STRING, TRACE_DEFAULT_LEVEL ); 
    return IX_SUCCESS; 

}


/**
 * NAME: _ix_rm_platform_check_for_rmsupp_icp_ring_memory
 *
 * DESCRIPTION: This function will verify if the memory passed can support ICP 
 * ring that can been accessed by RM. 
 * Will return err in the case of failure
 * 
 * @Param:  - IN arg_MemType - Type of Memory being accessed
 *
 * @Return: IX_SUCCESS if successful.
 *          IX_RM_ERROR_HW_RING_MEM_NOT_SUPPORTED if ICP ring is not supported
 *              in that particular memory.
 */
ix_error _ix_rm_platform_check_for_rmsupp_icp_ring_memory (
                                                    ix_uint32 arg_MemType,
                                                    ix_uint32 arg_MemChannel
                                                  )
{
    ix_error err = IX_SUCCESS;

    static const char s_aFunctionIdentifier[] = 
                          "_ix_rm_platform_check_for_rmsupp_icp_ring_memory";
    
    IX_RM_ENTER_TRACE("_ix_rm_platform_check_for_rmsupp_icp_ring_memory",
                        TRACE_DEFAULT_STRING, TRACE_DEFAULT_LEVEL ,
                        ("arg_MemType = %d \n arg_MemChannel = %d \n"
                        ,(ix_uint32)arg_MemType
                        ,(ix_uint32)arg_MemChannel));

    IX_RM_UNUSED_VARIABLE(s_aFunctionIdentifier);


    /* memory type is is not valid then return error */
    if (arg_MemType >= IX_MEMORY_TYPE_LAST)
    {
#ifdef _IX_RM_DEBUG_

        ixOsalStdLog("%s: Invalid ring memory type %u\n",
                                s_aFunctionIdentifier, arg_MemType);
#endif

        IX_RM_EXIT_TRACE("_ix_rm_platform_check_for_rmsupp_icp_ring_memory"
                  , TRACE_DEFAULT_STRING, TRACE_DEFAULT_LEVEL ); 
        err = IX_RM_ERROR_NEW(IX_RM_ERROR_HW_RING_MEM_NOT_SUPPORTED, 
                                s_aFunctionIdentifier, 
                "the specified ICP ring memory is invalid!", err);
        return err;
     
    }

    if (arg_MemChannel >= IX_DRAM_CHANNELS_NUMBER)
    {
#ifdef _IX_RM_DEBUG_

        ixOsalStdLog("%s: Invalid ring memory channel number %u\n",
                                s_aFunctionIdentifier, arg_MemChannel);
#endif

        IX_RM_EXIT_TRACE("_ix_rm_platform_check_for_rmsupp_icp_ring_memory"
                  , TRACE_DEFAULT_STRING, TRACE_DEFAULT_LEVEL ); 
        err = IX_RM_ERROR_NEW(IX_RM_ERROR_HW_RING_MEM_NOT_SUPPORTED, 
                              s_aFunctionIdentifier, 
                "the specified ICP ring memory is invalid!", err);
        return err;
     
    }
        
    
    IX_RM_EXIT_TRACE("_ix_rm_platform_check_for_rmsupp_icp_ring_memory"
                             , TRACE_DEFAULT_STRING, TRACE_DEFAULT_LEVEL ); 
    return IX_SUCCESS; 

}



/**
 * NAME: _ix_rm_platform_check_for_rmsupp_software_ring_memory
 *
 * DESCRIPTION: This function will verify if the memory passed can support ICP 
 * ring that can been accessed by RM. 
 * Will return err in the case of failure
 * 
 * @Param:  - IN arg_MemType - Type of Memory being accessed
 *
 * @Return: IX_SUCCESS if successful.
 *          IX_RM_ERROR_HW_RING_MEM_NOT_SUPPORTED if software ring is not
 *              supported in that particular memory.
 */
ix_error _ix_rm_platform_check_for_rmsupp_software_ring_memory (
                                                    ix_uint32 arg_MemType,
                                                    ix_uint32 arg_MemChannel
                                                  )
{
    ix_error err = IX_SUCCESS;

    static const char s_aFunctionIdentifier[] = 
                       "_ix_rm_platform_check_for_rmsupp_software_ring_memory";
    
    IX_RM_ENTER_TRACE("_ix_rm_platform_check_for_rmsupp_software_ring_memory",
                        TRACE_DEFAULT_STRING, TRACE_DEFAULT_LEVEL ,
                        ("arg_MemType = %d \n arg_MemChannel = %d \n"
                        ,(ix_uint32)arg_MemType
                        ,(ix_uint32)arg_MemChannel));

    IX_RM_UNUSED_VARIABLE(s_aFunctionIdentifier);


    /* memory type is is not valid then return error */
    if (arg_MemType >= IX_MEMORY_TYPE_LAST)
    {
#ifdef _IX_RM_DEBUG_

        ixOsalStdLog("%s: Invalid ring memory type %u\n",
                                s_aFunctionIdentifier, arg_MemType);
#endif

        IX_RM_EXIT_TRACE(
                "_ix_rm_platform_check_for_rmsupp_software_ring_memory"
                  , TRACE_DEFAULT_STRING, TRACE_DEFAULT_LEVEL ); 

        err = IX_RM_ERROR_NEW(IX_RM_ERROR_HW_RING_MEM_NOT_SUPPORTED, 
                               s_aFunctionIdentifier, 
                "the specified ICP ring memory is invalid!", err);
        return err;
     
    }

    if (arg_MemChannel >= IX_DRAM_CHANNELS_NUMBER)
    {
#ifdef _IX_RM_DEBUG_

        ixOsalStdLog("%s: Invalid ring memory channel number %u\n",
                                s_aFunctionIdentifier, arg_MemChannel);
#endif

        IX_RM_EXIT_TRACE(
                  "_ix_rm_platform_check_for_rmsupp_software_ring_memory"
                  , TRACE_DEFAULT_STRING, TRACE_DEFAULT_LEVEL ); 

        err = IX_RM_ERROR_NEW(IX_RM_ERROR_HW_RING_MEM_NOT_SUPPORTED, 
                              s_aFunctionIdentifier, 
                "the specified ICP ring memory is invalid!", err);
        return err;
     
    }
        
    
    IX_RM_EXIT_TRACE("_ix_rm_platform_check_for_rmsupp_software_ring_memory"
                             , TRACE_DEFAULT_STRING, TRACE_DEFAULT_LEVEL ); 
    return IX_SUCCESS; 

}




/**
 * NAME: ix_rm_platform_check_for_rmsupp_ring_type
 *
 * DESCRIPTION: This function will verify if the ring type passed is supported
 *  by RM. 
 * Will return err in the case of failure
 * 
 * @Param:  - IN arg_RingType - Type of Ring
 *
 * @Return: IX_SUCCESS if successful.
 *          IX_RM_ERROR_HW_RING_NOT_SUPPORTED if ring type is is not valid.
 */

ix_error _ix_rm_platform_check_for_rmsupp_ring_type (ix_uint32 arg_RingType)
{
    ix_error err = IX_SUCCESS;

    
    static const char s_aFunctionIdentifier[] = 
                             "_ix_rm_platform_check_for_rmsupp_ring_type";
    
    IX_RM_ENTER_TRACE("_ix_rm_platform_check_for_rmsupp_ring_type",
                        TRACE_DEFAULT_STRING, TRACE_DEFAULT_LEVEL,
                        ("arg_RingType = %d \n"
                        ,(ix_uint32)arg_RingType));

    IX_RM_UNUSED_VARIABLE(s_aFunctionIdentifier);


    /* ring type is is not valid then return error */
    if (arg_RingType >= IX_RING_LAST)
    {
#ifdef _IX_RM_DEBUG_

        ixOsalStdLog("%s: Invalid ring type %u\n",
                                s_aFunctionIdentifier, arg_RingType);
#endif

     IX_RM_EXIT_TRACE("_ix_rm_platform_check_for_rmsupp_ring_type"
                  , TRACE_DEFAULT_STRING, TRACE_DEFAULT_LEVEL ); 
     err = IX_RM_ERROR_NEW(IX_RM_ERROR_HW_RING_NOT_SUPPORTED, 
                           s_aFunctionIdentifier, 
                "this ring type is not supported!", err);
     return err;
     
    }
    
    
    IX_RM_EXIT_TRACE("_ix_rm_platform_check_for_rmsupp_ring_type"
                             , TRACE_DEFAULT_STRING, TRACE_DEFAULT_LEVEL ); 
    return IX_SUCCESS; 

}
