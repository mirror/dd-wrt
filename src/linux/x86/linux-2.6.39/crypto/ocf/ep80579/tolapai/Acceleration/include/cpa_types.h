
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

/*
 *****************************************************************************
 * Doxygen group definitions
 ****************************************************************************/

/**
 *****************************************************************************
 * @file cpa_types.h
 * 
 * @defgroup cpa_Types CPA Type Definition
 * 
 * @ingroup cpa
 *
 * @description
 *      This is the CPA Type Definitions. 
 * 
 *****************************************************************************/

#ifndef CPA_TYPES_H
#define CPA_TYPES_H

#if defined (__linux__) && defined (__KERNEL__)

/* Linux kernel mode */
#include <linux/kernel.h>
#include <linux/types.h>

#elif defined (__freebsd) && defined (_KERNEL)

/* FreeBSD kernel mode */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/kernel.h>

#elif defined (__linux__) || defined (__freebsd)

/* Linux or FreeBSD, user mode */
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#else
#error Unsupported operating system
#endif /* OS and mode */


typedef uint8_t Cpa8U;
typedef int8_t Cpa8S;
typedef uint16_t Cpa16U;
typedef int16_t Cpa16S;
typedef uint32_t Cpa32U;
typedef int32_t Cpa32S;
typedef uint64_t Cpa64U;
typedef int64_t Cpa64S;

/*****************************************************************************
 *      Generic Base Data Type definitions
 *****************************************************************************/
#ifndef NULL
#define NULL (0)
/**< @ingroup cpa_Types
 *   NULL definition */
#endif

#ifndef TRUE
#define TRUE (1==1)
/**< @ingroup cpa_Types
 *   True value definition */
#endif
#ifndef FALSE
#define FALSE (0==1)
/**< @ingroup cpa_Types
 *   False value definition */
#endif

/**
 *****************************************************************************
 * @ingroup cpa_Types
 *      Boolean type 
 *
 * @description
 *      Functions in this API use this type for Boolean variables that take 
 *      true or false values. 
 * 
 *****************************************************************************/
typedef enum _CpaBoolean
{
    CPA_FALSE = FALSE, /**< False value */
    CPA_TRUE = TRUE, /**< True value */
} CpaBoolean;

#endif /* CPA_TYPES_H */

