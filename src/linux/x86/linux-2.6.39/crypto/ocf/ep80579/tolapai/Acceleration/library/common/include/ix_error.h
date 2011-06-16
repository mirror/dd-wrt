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

#if !defined(__IX_ERROR_H__)
#define __IX_ERROR_H__

#include "ix_types.h"
#include "ix_macros.h"
#include "ix_symbols.h"

#if defined(__cplusplus)
extern "C"
{
#endif /* end defined(__cplusplus) */


/**
 * TYPENAME: ix_error
 * 
 * DESCRIPTION: This type represents an error token. It is mapped as 32 bit unsigned
 *          integer and several pieces of information will be packed in this token as
 *          follows:
 *          Bits    Size    Field       Description
 *      -----------------------------------------------------------------
 *           0:23   24      Error code  This field describes the error code
 *          24:30   7       Error level This field describes the error level
 *          31:31   1       Error mode  Flat vs. chained error codes
 *
 *          IXA SDK errors belong to different error groups. 
 *          This feature permits the reuse of same error code 
 *          numbers for different modules (error groups).  
 *          A module can define the error codes independently 
 *          from other modules, as long as it uses different 
 *          error group.  However, some synchronization should 
 *          be done in choosing the error groups. The error 
 *          level describes the severity of an error. Based 
 *          on this level, the programmer can take different 
 *          actions to handle the error. Macros are provided 
 *          to get and set the different bit fields of an 
 *          ix_error token.
 *
 */
typedef ix_uint32   ix_error;
typedef ix_uint32	ix_error_number;

/**
 * DESCRIPTION: This symbol defines an error token corresponding to
 *          successful completion of an operation.
 */
#ifndef IX_SUCCESS
#define IX_SUCCESS    ((ix_error)0)
#endif


/**
 * TYPENAME: ix_error_group
 * 
 * DESCRIPTION: This enumeration describes the existing error groups in the system. 
 *          New error groups should be added all the times at the end. Developers can
 *          use IX_ERROR_GROUP_LAST value to start assigning new error group numbers.
 *
 */
typedef enum ix_e_error_group
{
    IX_ERROR_GROUP_FIRST = 0,
    IX_ERROR_GROUP_RESOURCE_MANAGER = IX_ERROR_GROUP_FIRST, /* Resource Manager error group */
    IX_ERROR_GROUP_OSAL, /* OSAL API error group */
    IX_ERROR_GROUP_OSSL, /* ADDED for Tools WR IXA00178349 */
    IX_ERROR_GROUP_CC_INFRASTRUCTURE, /* Core component infrastructure API error group */
    IX_ERROR_GROUP_CC,
    IX_ERROR_GROUP_CC_RTMV4,
    IX_ERROR_GROUP_CC_IPV4,
    IX_ERROR_GROUP_CC_FRAGV4,
    IX_ERROR_GROUP_LKUP,
    IX_ERROR_GROUP_SA,
    IX_ERROR_GROUP_CC_MSUP,
    IX_ERROR_GROUP_CC_POS_RX,
    IX_ERROR_GROUP_CC_POS_TX,
    IX_ERROR_GROUP_CC_ETH_RX,
    IX_ERROR_GROUP_CC_ETH_TX,
    IX_ERROR_GROUP_CC_CSIX_RX,
    IX_ERROR_GROUP_CC_CSIX_TX,
    IX_ERROR_GROUP_CC_QM,
    IX_ERROR_GROUP_CC_SCHED,
    IX_ERROR_GROUP_CC_GENEX,
    IX_ERROR_GROUP_CC_STKDRV, /* Stack Driver error group */
    IX_ERROR_GROUP_CC_IPV6,
    IX_ERROR_GROUP_CC_IPV6TM,
    IX_ERROR_GROUP_CC_ARP,
    IX_ERROR_GROUP_CC_L2TM,
    IX_ERROR_GROUP_CC_MPLS,
    IX_ERROR_GROUP_CC_IP_GRE,
    IX_ERROR_GROUP_CC_IPV4_VALIDATOR,
    IX_ERROR_GROUP_CC_IPV6_VALIDATOR,
    IX_ERROR_GROUP_CC_CLASSIFIER_3T,
    IX_ERROR_GROUP_CC_CLASSIFIER_5T,
    IX_ERROR_GROUP_CC_CLASSIFIER_6T,
    IX_ERROR_GROUP_CC_CLASSIFIER_6T_IPV6,
    IX_ERROR_GROUP_CC_CLASSIFIER_6T_IPV4,
    IX_ERROR_GROUP_CC_TC_METER,
    IX_ERROR_GROUP_CC_WRED,
    IX_ERROR_GROUP_CC_RTMV6,
    IX_ERROR_GROUP_CC_FPM,
    IX_ERROR_GROUP_CC_CLASSIFIER_DSCP,
    IX_ERROR_GROUP_CC_NATPT,
    IX_ERROR_GROUP_CC_L2BRIDGE,
    IX_ERROR_GROUP_CC_ATMRX,
    IX_ERROR_GROUP_CC_ATMTX,
    IX_ERROR_GROUP_CC_ATMTM41,
    IX_ERROR_GROUP_CC_ATMSAR,
    IX_ERROR_GROUP_CC_ATMSARTEST,
    IX_ERROR_GROUP_CC_LLC_SNAP_ENCAP,
    IX_ERROR_GROUP_CC_IUB_DUMMY,
    IX_ERROR_GROUP_CC_ICF,    
    IX_ERROR_GROUP_CC_IPENCAP,    
    IX_ERROR_GROUP_CC_PRISCH,    
    IX_ERROR_GROUP_CC_ARPAGENT,        
    IX_ERROR_GROUP_CC_IPV4_5T_CLASSIFIER,
    IX_ERROR_GROUP_CC_IPV6_5T_CLASSIFIER,
    IX_ERROR_GROUP_CC_ETHERNET_ENCAP,
    IX_ERROR_GROUP_CC_ETHERNET_DECAP,
    IX_ERROR_GROUP_CC_IPUDP_ENCAP,
    IX_ERROR_GROUP_CC_IPUDP_DECAP,
    IX_ERROR_GROUP_CC_PIMSM,
    IX_ERROR_GROUP_CC_IPV4_EMCAST,
    IX_ERROR_GROUP_CC_IPV4_EMCAST_MOIT_OIF,
    IX_ERROR_GROUP_CC_IPV4_EMCAST_MOIT_PORT_MASK,
    IX_ERROR_GROUP_CC_ATMOS,
    IX_ERROR_GROUP_CC_CONTROL_CLASSIFIER,
    IX_ERROR_GROUP_CC_BKPLANE_ENCAP,
    IX_ERROR_GROUP_CC_PPP_ENCAP,
    IX_ERROR_GROUP_CC_PPP_DECAP,    
    IX_ERROR_GROUP_CC_IPHC,
    IX_ERROR_GROUP_CC_IP_V4_LOOKUP,
    IX_ERROR_GROUP_CC_IP_V6_LOOKUP,
    IX_ERROR_GROUP_CC_IPV4_FILTER,
    IX_ERROR_GROUP_CC_IPV6_FILTER,
    IX_ERROR_GROUP_CC_PMU,
    IX_ERROR_GROUP_CC_ETHER_DECAP,
    IX_ERROR_GROUP_CC_MLMC_REA,   
    IX_ERROR_GROUP_CC_MLMC_FRAG,
    IX_ERROR_GROUP_CC_PPP_MUX,
    IX_ERROR_GROUP_CC_PPP_DMUX,     
    IX_ERROR_GROUP_CC_HDLC,
    IX_ERROR_GROUP_CC_NPE_PACKET_TX,
    IX_ERROR_GROUP_CC_PACKET_RX,
    IX_ERROR_GROUP_LAST
} ix_error_group;

#define IX_ERROR_MAKE_GROUP(__g) (IX_ERROR_GROUP_##__g << 16)


/**
 * TYPENAME: ix_error_level
 * 
 * DESCRIPTION: This enumeration designates the severity level of an error, 
 *          zero repreesenting no error at all, and higher numbers
 *          representing more and more severe errors. Severity roughly means
 *          how much of the system has become corrupted and is likely to  be no
 *          longer functional after the error occurs.
 */
typedef enum ix_e_error_level
{
    IX_ERROR_LEVEL_FIRST = 0,

    /** 
     * No error reported.
     *
     * This error level corresponds to returns from functions that
     * successfully accomplished their given tasks. This error level
     * will mean that no error occurred. It should not be used in any
     * valid  error token bacause it corresponds to IX_ERROR_SUCCESS.
     */
    IX_ERROR_LEVEL_NONE = IX_ERROR_LEVEL_FIRST,

    /** 
     * Warning level: Recoverable condition.
     *
     * This error level indicates that the requested task could not be
     * performed, but there are no lasting effects within the system
     * from that failure. The caller can recover from this condition
     * by simply continuing to execute as if the call had not been
     * made. Functions returning WARNING level messages should clearly
     * document the procedures for recovery.
     *
     * During debugging, WARNING messages should be logged for later
     * analysis as they may indicate the presence of a bug. This step
     * can probably be skipped in Production mode systems.
     */
    IX_ERROR_LEVEL_WARNING,

    /** 
     * Error level: Unrecoverable Local Condition.
     *
     * This error level indicates that the requested task could not be
     * performed, and that one or more of the specific data items
     * involved in the call has been permanently altered, so that in
     * general recovery may require "leaking" a corrupted data item
     * or shutting down and restarting a service. Functions returning
     * ERROR level messages should clearly document recovery methods
     * for keeping the system running, and what storage or
     * functionality will be lost.
     *
     * During debugging, ERROR messages should be immediately
     * analyzed , along with any prior WARNING messages. Production
     * mode systems should log all ERROR messages, even when we
     * expect to recover from them.
     */
    IX_ERROR_LEVEL_LOCAL,

    /** 
     * Error level: Unrecoverable Remote Condition.
     *
     * This error level indicates that there is an error in the module 
     * we are trying to communicate with. That will have no impact on the
     * local module but the functionality of the system as a whole will be
     * most likely affected.
     * This error level might be used for the case the libraries are used
     * in conjunction with a Foreign Model running on transactor.
     */
    IX_ERROR_LEVEL_REMOTE,

    /** 
     * Error level: Unrecoverable Global Condition.
     * 
     * This error level indicates that a function has detected a
     * condition that indicates that the system as a whole has become
     * compromised, and that multiple processing elements or data
     * modules are likely to experience a cascade failure.
     * 
     * During debugging, this level of error message should cause the
     * system to freeze and drop into a debugger where the problem
     * can be analyzed by an engineer. In Production deployment,
     * huge effort should be made to log these errors and as much
     * supporting information as possible where the logs can be
     * analyzed by a responsible human.
     */
    IX_ERROR_LEVEL_GLOBAL,

    /** 
     * Error level: Totally Impossible Condition.
     * 
     * This error level indicates that a function has detected a
     * condition that indicates that a primary assumption in a module
     * design has been proven impossible, and that in general the
     * system would be doing arbitrarily bad things.
     * 
     * During debugging, PANIC messages should cause the system to
     * freeze and drop into a debugger state where the problem can be
     * analyzed by an engineer.
     * 
     * In a Production deployed system, PANIC messages should cause
     * an immediate attention.
     * 
     * PANIC messages should be used very, very sparingly.
     */
    IX_ERROR_LEVEL_PANIC,

    IX_ERROR_LEVEL_LAST
} ix_error_level;


/**
 * MACRO NAME: IX_ERROR_GET_MODE
 *
 * DESCRIPTION: This macro retrieves the error mode field from an error token.
 *
 * @Param:  - IN arg_Error -  error token of type ix_error.
 *
 * @Return: Returns a ix_uint32 value representing the error mode for
 *          this error token.
 */
#define IX_ERROR_GET_MODE( \
                            arg_Error \
                          ) \
                          (IX_GET_BIT_FIELD32(arg_Error, 31U, 31U))



/**
 * MACRO NAME: IX_ERROR_GET_CODE
 *
 * DESCRIPTION: This macro retrieves the error code field from an error token.
 *
 * @Param:  - IN arg_Error -  error token of type ix_error.
 *
 * @Return: Returns an ix_uint32 value representing the error code for 
 *          the error token.
 */
#if defined(IX_ERROR_EXTENSIONS)
IX_EXPORT_FUNCTION ix_uint32 ix_error_get_code(ix_error err);
#define IX_ERROR_GET_CODE( \
                            arg_Error \
                         ) \
                         (ix_error_number)(IX_ERROR_GET_MODE(arg_Error) ? ix_error_get_code(arg_Error) : IX_GET_BIT_FIELD32(arg_Error, 0U, 23U))
#else /* no extensions */
#define IX_ERROR_GET_CODE( \
                            arg_Error \
                         ) \
                         (ix_error_number)IX_GET_BIT_FIELD32(arg_Error, 0U, 23U)
#endif /* IX_ERROR_EXTENSIONS */

/**
 * MACRO NAME: IX_ERROR_SET_CODE
 *
 * DESCRIPTION: This macro sets the error code field for an error token.
 *
 * @Param:  - IN arg_Error -  error token of type ix_error.
 * @Param:  - IN arg_ErrorCode - this is the new error code for the error token.
 *
 * @Return: Returns the new value of the error token.
 */
#if defined(IX_ERROR_EXTENSIONS)
ix_error ix_error_set_code(ix_error err, ix_error_number code);
#define IX_ERROR_SET_CODE( \
                            arg_Error, \
                            arg_ErrorCode \
                         ) \
                         ((ix_error)IX_ERROR_GET_MODE(arg_Error) ? ix_error_set_code(arg_Error, arg_ErrorCode) : IX_SET_BIT_FIELD32(arg_Error, arg_ErrorCode, 0U, 23U))
#else /* no extensions */
#define IX_ERROR_SET_CODE( \
                            arg_Error, \
                            arg_ErrorCode \
                         ) \
                         ((ix_error)IX_SET_BIT_FIELD32(arg_Error, arg_ErrorCode, 0U, 23U))
#endif /* IX_ERROR_EXTENSIONS */



/**
 * MACRO NAME: IX_ERROR_GET_LEVEL
 *
 * DESCRIPTION: This macro retrieves the error level field from an error token.
 *
 * @Param:  - IN arg_Error -  error token of type ix_error.
 *
 * @Return: Returns a ix_uint32 value representing the error level for 
 *          this error token.
 */
#if defined(IX_ERROR_EXTENSIONS)
ix_error_level ix_error_get_level(ix_error err);
#define IX_ERROR_GET_LEVEL( \
                            arg_Error \
                          ) \
                          (IX_ERROR_GET_MODE(arg_Error) ? ix_error_get_level(arg_Error): IX_GET_BIT_FIELD32(arg_Error, 24U, 30U))
#else /* no extensions */
#define IX_ERROR_GET_LEVEL( \
                            arg_Error \
                          ) \
                          IX_GET_BIT_FIELD32(arg_Error, 24U, 30U)
#endif /* IX_ERROR_EXTENSIONS */

/**
 * MACRO NAME: IX_ERROR_SET_LEVEL
 *
 * DESCRIPTION: This macro sets the error level field for an error token.
 *
 * @Param:  - IN arg_Error -   error token of type ix_error.
 * @Param:  - IN arg_ErrorLevel - this is the new error level for the error token.
 *
 * @Return: Returns the new value of the error token.
 */
#if defined(IX_ERROR_EXTENSIONS)
ix_error ix_error_set_level(ix_error err, ix_error_level level);
#define IX_ERROR_SET_LEVEL( \
                            arg_Error, \
                            arg_ErrorLevel \
                          ) \
                          ((ix_error)IX_ERROR_GET_MODE(arg_Error) ? ix_error_set_level(arg_Error, arg_ErrorLevel) : IX_SET_BIT_FIELD32(arg_Error, arg_ErrorLevel, 24U, 31U))
#else /* no extensions */
#define IX_ERROR_SET_LEVEL( \
                            arg_Error, \
                            arg_ErrorLevel \
                          ) \
                          (ix_error)IX_SET_BIT_FIELD32(arg_Error, arg_ErrorLevel, 24U, 31U)
#endif /* IX_ERROR_EXTENSIONS */


/**
 * MACRO NAME: IX_ERROR_NEW
 *
 * DESCRIPTION: This macro generates a new error token based on the error code,
 *          group and level that are passed as arguments.
 *
 * @Param:  - IN arg_ErrorCode - the error code of the new error token to be generated.
 *          The range is 0..65535
 * @Param:  - IN arg_ErrorGroup - the error group of the new error token to be generated. 
 *          The range is 0..255
 * @Param:  - IN arg_ErrorLevel - the error level of the new error token to be generated.
 *          The range is 0..255.
 *
 * @Return: Returns a new ix_error token.
 */
#define IX_ERROR_NEW( \
                       arg_ErrorCode, \
                       arg_ErrorLevel \
                    ) \
                    ((ix_error) /* typecast */ \
                    IX_MAKE_BIT_FIELD32(arg_ErrorCode, 0U, 23U) | \
                    IX_MAKE_BIT_FIELD32(arg_ErrorLevel, 24U, 30U))



#if defined(__cplusplus)
}
#endif /* end defined(__cplusplus) */

#endif /* end !defined(__IX_ERROR_H__) */

