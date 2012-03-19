/*
 * FileName:    IxAtmSchUtils_p.h
 * Author: Intel Corporation
 * Created:     1-Mar-2002
 * Description:
 *    Util header macros.
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



