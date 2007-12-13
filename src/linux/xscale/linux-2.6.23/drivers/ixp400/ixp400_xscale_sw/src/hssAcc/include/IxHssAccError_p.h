/**
 * @file IxHssAccError_p.h
 * 
 * @author Intel Corporation
 * @date 30-Jan-02
 *
 * @brief Private API for HssAccess error handling
 *
 * 
 * @par
 * IXP400 SW Release version 2.4
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2007, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
*/

/**
 * @defgroup IxHssAccess IxHssAccError_p
 *
 * @brief Private API for HssAccess error/debug handling
 * 
 * @{
 */

#ifndef IXHSSACCERROR_P_H
#define IXHSSACCERROR_P_H

#include "IxOsal.h"

/**
 * Prototypes for interface functions.
 */

/**
 * @typedef IxHssAccTraceTypes
 * @brief Enumeration defining HssAccess trace levels
 */

typedef enum
{
    IX_HSSACC_TRACE_OFF,    /**< NO TRACE */
    IX_HSSACC_DEBUG,        /**< Select traces of interest */
    IX_HSSACC_FN_ENTRY_EXIT /**< ALL function entry/exit traces */
} IxHssAccTraceTypes;

/**
 * #defines for function return types, etc.
 */

/**
 * @def IX_HSSACC_TRACE_LEVEL
 *
 * @brief HssAccess debug trace level
 */
#if ((CPU==SIMSPARCSOLARIS) || (CPU==SIMLINUX))
#define IX_HSSACC_TRACE_LEVEL IX_HSSACC_FN_ENTRY_EXIT
#else
#define IX_HSSACC_TRACE_LEVEL IX_HSSACC_TRACE_OFF
#endif

/**
 * @def IX_HSSACC_DEBUG_ON
 *
 * @brief Mechanism for switching on debug information
 *
 * @param STATEMENT [in] STATEMENT - debug statement
 *
 * This macro executes the statement passed
 *
 * @return none
 */
#define IX_HSSACC_DEBUG_ON(STATEMENT) (STATEMENT)

/**
 * @def IX_HSSACC_DEBUG_OFF
 *
 * @brief Mechanism for switching off debug information
 *
 * @param STATEMENT [in] STATEMENT - debug statement
 *
 * This macro prevents the execution of STATEMENT
 *
 * @return none
 */
#define IX_HSSACC_DEBUG_OFF(STATEMENT)

/**
 * @def IX_HSSACC_REPORT_ERROR
 *
 * @brief Mechanism for reporting HssAccess software errors
 *
 * @param char* [in] STR - Error string to report
 *
 * This macro sends the error string passed to ixOsalLog
 *
 * @return none
 */
#define IX_HSSACC_REPORT_ERROR(STR) ixOsalLog (IX_OSAL_LOG_LVL_ERROR,  \
                                               IX_OSAL_LOG_DEV_STDERR, \
                                               STR, 0, 0, 0, 0, 0, 0);

/**
 * @def IX_HSSACC_REPORT_ERROR_WITH_ARG
 *
 * @brief Mechanism for reporting HssAccess software errors
 *
 * @param char* [in] STR - Error string to report
 * @param argType [in] ARG1 - Argument to output
 * @param argType [in] ARG2 - Argument to output
 * @param argType [in] ARG3 - Argument to output
 * @param argType [in] ARG4 - Argument to output
 * @param argType [in] ARG5 - Argument to output
 * @param argType [in] ARG6 - Argument to output
 *
 * This macro sends the error string passed to ixOsalLog
 *
 * @return none
 */
#define IX_HSSACC_REPORT_ERROR_WITH_ARG(STR, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6) \
{ \
    ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, STR, \
    (int) ARG1, (int) ARG2, (int) ARG3, (int) ARG4, (int) ARG5, (int) ARG6); \
}

/**
 * @def IX_HSSACC_TRACE0
 *
 * @brief Mechanism for tracing debug for the HssAccess component, for no
 * arguments
 *
 * @param unsigned [in] LEVEL - one of IxHssAccTraceTypes enumerated values
 * @param char* [in] STR - Trace string
 *
 * This macro sends the trace string passed to ixOsalLog 
 *
 * @return none
 */
#define IX_HSSACC_TRACE0(LEVEL, STR) \
{ \
    if (LEVEL <= IX_HSSACC_TRACE_LEVEL) \
    { \
        ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT, \
        STR, 0, 0, 0, 0, 0, 0); \
    } \
}

/**
 * @def IX_HSSACC_TRACE1
 *
 * @brief Mechanism for tracing debug for the HssAccess component, with one
 * argument
 *
 * @param unsigned [in] LEVEL - one of IxHssAccTraceTypes enumerated values
 * @param char* [in] STR - Trace string
 * @param argType [in] ARG1 - Argument to trace
 *
 * This macro sends the trace string passed to ixOsalLog 
 *
 * @return none
 */
#define IX_HSSACC_TRACE1(LEVEL, STR, ARG1) \
{ \
    if (LEVEL <= IX_HSSACC_TRACE_LEVEL) \
    { \
        ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT, \
        STR, (int) ARG1, 0, 0, 0, 0, 0); \
    } \
}

/**
 * @def IX_HSSACC_TRACE2
 *
 * @brief Mechanism for tracing debug for the HssAccess component, with two
 * arguments
 *
 * @param unsigned [in] LEVEL - one of IxHssAccTraceTypes enumerated values
 * @param char* [in] STR - Trace string
 * @param argType [in] ARG1 - Argument to trace
 * @param argType [in] ARG2 - Argument to trace
 *
 * This macro sends the trace string passed to ixOsalLog 
 *
 * @return none
 */
#define IX_HSSACC_TRACE2(LEVEL, STR, ARG1, ARG2) \
{ \
    if (LEVEL <= IX_HSSACC_TRACE_LEVEL) \
    { \
        ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT, \
        STR, (int) ARG1, (int) ARG2, 0, 0, 0, 0); \
    } \
}


#endif /* IXHSSACCERROR_H */

/**
 * @} defgroup IxHssAccess
 */
