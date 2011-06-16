/*
 * @file        IxOsalAssert.h 
 * @author 	Intel Corporation
 * @date        25-08-2004
 *
 * @brief       description goes here
 *
 * @par
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

#ifndef IX_OSAL_ASSERT_H
#define IX_OSAL_ASSERT_H
#ifdef __cplusplus
extern "C"{
#endif
/*
 * Put the system defined include files required
 * @par
 * <TAGGED>
 */

#include "IxOsalOsAssert.h"

/**
 * @brief Assert macro, assert the condition is true. This
 *        will not be compiled out.
 *        N.B. will result in a system crash if it is false.
 */

/**
 * @brief Ensure macro, ensure the condition is true.
 *        This will be conditionally compiled out and
 *        may be used for test purposes.
 */
#ifdef IX_OSAL_ENSURE_ON
#define IX_OSAL_ENSURE(c, str) do { \
if (!(c)) ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,"%s in file %s at line %d ", \
(int)str,(int)__FILE__, (int)__LINE__, 0, 0, 0); } while (0)


#define IX_OSAL_ENSURE_RETURN(c, str) \
        if (NULL == c) { ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, \
            IX_OSAL_LOG_DEV_STDOUT, "%s in file %s at line %d ", \
            (int)str, (int)__FILE__, (int)__LINE__, 0, 0, 0); \
            return IX_FAIL; } 
            
            
#define IX_OSAL_LOCAL_ENSURE(c, str, ret) \
        if (!(c)) { \
			ixOsalLog (IX_OSAL_LOG_LVL_ERROR, \
            IX_OSAL_LOG_DEV_STDOUT, "%s in file %s at line %d ", \
            (int)str, (int)__FILE__, (int)__LINE__, 0, 0, 0); \
            return ret; } 

#define IX_OSAL_ENSURE_JUST_RETURN(c, str) \
        if (!(c)) { \
        	ixOsalLog (IX_OSAL_LOG_LVL_ERROR, \
            IX_OSAL_LOG_DEV_STDOUT, "%s in file %s at line %d ", \
            (int)str, (int)__FILE__, (int)__LINE__, 0, 0, 0); \
            return; } 
            
#define OSAL_ENSURE_CHECK_SUCCESS(c, str) \
        if (IX_SUCCESS != c) { ixOsalLog (IX_OSAL_LOG_LVL_ERROR, \
            IX_OSAL_LOG_DEV_STDOUT, "%s in file %s at line %d ", \
            (int)str, (int)__FILE__, (int)__LINE__, 0, 0, 0); \
            return IX_FAIL; } 
#else
#define IX_OSAL_ENSURE(c, str) 				do {} while (0);
#define IX_OSAL_ENSURE_RETURN(c, str) 		do {} while (0);          
#define IX_OSAL_LOCAL_ENSURE(c, str, ret) 	do {} while (0);
#define IX_OSAL_ENSURE_JUST_RETURN(c, str) 	do {} while (0);
#define OSAL_ENSURE_CHECK_SUCCESS(c, str) 	do {} while (0); 
#endif

#ifdef IX_OSAL_MEM_ASSERT_ON
#define IX_OSAL_MEM_ASSERT(c) IX_OSAL_OS_MEM_ASSERT(c) 
#else
#define IX_OSAL_MEM_ASSERT(c) {\
} 
#endif

#define IX_OSAL_ASSERT(c) IX_OSAL_OS_ASSERT(c)
#ifdef __cplusplus
}
#endif
#endif /* IX_OSAL_ASSERT_H */
