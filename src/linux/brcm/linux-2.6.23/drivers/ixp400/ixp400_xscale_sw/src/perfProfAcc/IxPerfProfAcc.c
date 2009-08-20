/**
 * @file IxPerfProfAcc.c
 *
 * @date 22 May 2003
 *
 * @brief Contains the lock and unlock functions to be used across all modules
 * of the IxPerfProfAcc component
 *
 * Design Notes:
 *    <describe any non-obvious design decisions which impact this file>
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

/*
 * Put the user defined include files required.
 */
#include "IxPerfProfAcc.h"
#include "IxPerfProfAcc_p.h"

/*
 * Variable declarations global to this file only.  Externs are followed by
 * static variables.
 */
static BOOL utilLock = FALSE;	/*is false when there is no task running; true 
								 *if any task running
								 */

/*
 * Function definition.
 */
IxPerfProfAccStatus ixPerfProfAccLock(void)
{
	if(utilLock)	/*there is another task already running*/
	{
		return IX_PERFPROF_ACC_STATUS_ANOTHER_UTIL_IN_PROGRESS;
	}
	else	/*there are no tasks currently running*/
	{
	    utilLock = TRUE; /*set to true because a task is now running*/
		return IX_PERFPROF_ACC_STATUS_SUCCESS;
	}
}

void ixPerfProfAccUnlock(void)
{
	utilLock = FALSE; /*task that was running has ended, therefore, the task is 
 					   *free again
					   */
}


