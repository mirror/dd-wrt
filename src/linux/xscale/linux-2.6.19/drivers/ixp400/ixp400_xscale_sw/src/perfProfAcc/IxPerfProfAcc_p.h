/**
 * @file IxPerfProfAcc_p.h
 *
 * @date 22 May 2003
 *
 * @brief Private header file containing lock and unlock function prototypes
 *
 * Design Notes:
 *    
 * 
 * @par
 * IXP400 SW Release Crypto version 2.3
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2005, Intel Corporation.
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

#ifndef IXPERFPROFACC_P_H
#define IXPERFPROFACC_P_H

#include "IxPerfProfAcc.h"


/**
 * ixPerfProfAccLock (void)
 *
 * This will determine if any other utility is running; if not, this function 
 * will get the lock so that no other utilities can be called.  The function
 * returns :
 *	- IX_PERFPROF_ACC_STATUS_ANOTHER_UTIL_IN_PROGRESS if another utility is
 *	  already running
 *  - IX_PERFPROF_ACC_STATUS_SUCCESS if no other utility is running and the lock
 *    is obtained	
 *
 */
IxPerfProfAccStatus 
ixPerfProfAccLock (void);

/**
 * ixPerfProfAccUnlock (void)
 *
 * This will return the lock so that another utility may begin.
 *
 */
void 
ixPerfProfAccUnlock (void);

#endif /*#ifndef IXPERFPROFACC_P_H*/


