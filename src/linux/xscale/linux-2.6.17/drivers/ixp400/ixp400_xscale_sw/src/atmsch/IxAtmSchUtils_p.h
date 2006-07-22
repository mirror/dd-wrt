/*
 * FileName:    IxAtmSchUtils_p.h
 * Author: Intel Corporation
 * Created:     1-Mar-2002
 * Description:
 *    Util header macros.
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

#ifndef IXATMSCHUTILS_P_H
#define IXATMSCHUTILS_P_H

#include "IxOsal.h"

#define IX_ATMSCH_ASSERT(expr) \
    if(!(expr)) \
    { \
        printf("Assertion in file %s, at line %i\n", __FILE__, __LINE__);\
    }

#define IX_ATMSCH_WARNING_REPORT(STRING)                  \
        ixOsalLog(IX_OSAL_LOG_LVL_WARNING, IX_OSAL_LOG_DEV_STDOUT, \
                  "ERROR: %s, %i, %s \n",                          \
                  (int)__FILE__, __LINE__, (int)(STRING), 0, 0, 0);

#define IX_ATMSCH_ERROR_REPORT(STRING)                    \
        ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, \
                  "ERROR: %s, %i, %s \n",                        \
                  (int)__FILE__, __LINE__, (int)(STRING), 0, 0, 0);

#define IX_ATMSCH_PERF_TIMER_START(timer)
#define IX_ATMSCH_PERF_TIMER_STOP(timer)

/****************************************************************************/
#define TEST_START(STRING) \
        printf("TEST - %s\n", STRING);

#define TRACER(compId,level,string)                (printf("%s",string ))
#define TRACE0(compId,level,string)                (printf("%s\n",string ))
#define TRACE1(compId,level,string,a1)             (printf(string "\n",a1))
#define TRACE2(compId,level,string,a1,a2)          (printf(string "\n",a1,a2))
#define TRACE3(compId,level,string,a1,a2,a3)       (printf(string "\n",a1,a2,a3))
#define TRACE4(compId,level,string,a1,a2,a3,a4)    (printf(string "\n",a1,a2,a3,a4))
#define TRACE5(compId,level,string,a1,a2,a3,a4,a5) (printf(string "\n",a1,a2,a3,a4,a5))

#endif /* IXATMSCHUTILS_P_H */



