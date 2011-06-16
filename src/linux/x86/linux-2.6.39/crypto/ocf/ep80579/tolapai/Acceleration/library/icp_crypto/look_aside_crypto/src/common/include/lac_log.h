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
 * @file lac_log.h
 *
 * @defgroup LacLog     Log
 *
 * @ingroup LacCommon
 * 
 * Logging Macros. These macros also log the function name they are called in.
 *
 ***************************************************************************/


/***************************************************************************/

#ifndef LAC_LOG_H
#define LAC_LOG_H

/***************************************************************************
 * Include public/global header files
 ***************************************************************************/
#include "cpa.h"
#include "IxOsal.h"


#define _LAC_LOG_PARAM0_(level, log)                                        \
    (void)ixOsalLog (level, IX_OSAL_LOG_DEV_STDERR,                         \
            "%s() - : " log "\n", (Cpa32U)__func__, 0, 0, 0, 0, 0)
/**< @ingroup LacLog 
 * Internal macro that accepts no parameters in the string to be logged */

#define _LAC_LOG_PARAM1_(level, log, param1)                                \
    (void)ixOsalLog (level, IX_OSAL_LOG_DEV_STDERR,                         \
            "%s() - : " log "\n", (Cpa32U)__func__, (int)param1, 0, 0, 0, 0)
/**< @ingroup LacLog 
 * Internal macro that accepts 1 parameter in the string to be logged */

#define _LAC_LOG_PARAM2_(level, log, param1, param2)                        \
    (void)ixOsalLog (level, IX_OSAL_LOG_DEV_STDERR,                         \
            "%s() - : " log "\n", (Cpa32U)__func__, (int)param1,          \
            (int)param2, 0, 0, 0)
/**< @ingroup LacLog 
 * Internal macro that accepts 2 parameters in the string to be logged */



/************************** Lac Invalid Param Macros **************************/

#define LAC_INVALID_PARAM_LOG(log)                                      \
    _LAC_LOG_PARAM0_(IX_OSAL_LOG_LVL_ERROR, "Invalid API Param - " log)
/**< @ingroup LacLog 
 * Invalid parameter log macro. Has the prefix "[error]" */

#define LAC_INVALID_PARAM_LOG1(log, param1)                                   \
    _LAC_LOG_PARAM1_(IX_OSAL_LOG_LVL_ERROR, "Invalid API Param - " log, param1)
/**< @ingroup LacLog 
 * Invalid parameter log macro. Has the prefix "[error]" and also  
 * (1 parameter in the string to be logged). */



/************************** Lac Logging Macros **************************/

#define LAC_LOG(log)  _LAC_LOG_PARAM0_(IX_OSAL_LOG_LVL_USER, log)
/**< @ingroup LacLog 
 * Log a string with no prefix */

#define LAC_LOG1(log, param1)                                   \
    _LAC_LOG_PARAM1_(IX_OSAL_LOG_LVL_USER, log, param1)
/**< @ingroup LacLog 
 * Log a string with no prefix 
 * (1 parameter in the string to be logged). */

#define LAC_LOG2(log, param1, param2)                           \
    _LAC_LOG_PARAM2_(IX_OSAL_LOG_LVL_USER, log, param1, param2)
/**< @ingroup LacLog 
 * Log a string with no prefix 
 * (2 parameter in the string to be logged). */



/************************** Lac Error Log Macros **************************/

#define LAC_LOG_ERROR(log)  _LAC_LOG_PARAM0_(IX_OSAL_LOG_LVL_ERROR, log)
/**< @ingroup LacLog 
 * Log an error with the prefix "[error]" */

#define LAC_LOG_ERROR1(log, param1)                             \
    _LAC_LOG_PARAM1_(IX_OSAL_LOG_LVL_ERROR, log, param1)
/**< @ingroup LacLog 
 * Log an error with the prefix "[error]"
 * (1 parameter in the string to be logged). */

#define LAC_LOG_ERROR2(log, param1, param2)                     \
    _LAC_LOG_PARAM2_(IX_OSAL_LOG_LVL_ERROR, log, param1, param2)
/**< @ingroup LacLog 
 * Log an error with the prefix "[error]"
 * (2 parameters in the string to be logged). */



/************************** Lac Debug Macros **************************/

#define LAC_LOG_DEBUG(log)  _LAC_LOG_PARAM0_(IX_OSAL_LOG_LVL_DEBUG1, log)
/**< @ingroup LacLog 
 * Log a message with the prefix "[debug]" */

#define LAC_LOG_DEBUG1(log, param1)                             \
    _LAC_LOG_PARAM1_(IX_OSAL_LOG_LVL_DEBUG1, log, param1)
/**< @ingroup LacLog 
 * Log a message with the prefix "[debug]" 
 * (1 parameter in the string to be logged). */

#define LAC_LOG_DEBUG2(log, param1, param2)                     \
    _LAC_LOG_PARAM2_(IX_OSAL_LOG_LVL_DEBUG1, log, param1, param2)
/**< @ingroup LacLog 
 * Log a message with the prefix "[debug]" 
   (2 parameters in the string to be logged). */

#endif /* LAC_LOG_H */
