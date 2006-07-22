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
 * IXP400 SW Release version  2.1
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright (c) 2002-2005 Intel Corporation All Rights Reserved.
 * 
 * @par
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel Corporation
 * or its suppliers or licensors.  Title to the Material remains with
 * Intel Corporation or its suppliers and licensors.
 * 
 * @par
 * The Material is protected by worldwide copyright and trade secret laws
 * and treaty provisions. No part of the Material may be used, copied,
 * reproduced, modified, published, uploaded, posted, transmitted,
 * distributed, or disclosed in any way except in accordance with the
 * applicable license agreement .
 * 
 * @par
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you by
 * disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel, except in accordance with the
 * applicable license agreement.
 * 
 * @par
 * Unless otherwise agreed by Intel in writing, you may not remove or
 * alter this notice or any other notice embedded in Materials by Intel
 * or Intel's suppliers or licensors in any way.
 * 
 * @par
 * For further details, please see the file README.TXT distributed with
 * this software.
 * 
 * @par
 * -- End Intel Copyright Notice --
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
#if (CPU==SIMSPARCSOLARIS)
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
