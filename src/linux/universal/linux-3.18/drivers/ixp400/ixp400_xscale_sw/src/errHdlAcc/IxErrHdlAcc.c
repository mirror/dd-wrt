/**
 * @file    IxErrHdlAcc.c
 *
 * @author Intel Corporation
 * @date    1-Jan-2006
 *    
 * @brief   This file contains the implementation of the ixErrHdlAcc main
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
#include "IxOsal.h"
#include "IxErrHdlAcc.h"
#include "IxErrHdlAcc_p.h"
#include "IxErrHdlAccControl_p.h"
#include "IxErrHdlAccEthNPE_p.h"

BOOL ixErrHdlAccInitDone = FALSE;

/*
 * ixErrHdlAccInit: Initialize the component
 */
PUBLIC IX_STATUS ixErrHdlAccInit(void)
{
#ifdef IX_ERRHDLACC_DEBUG
	ixOsalLogLevelSet(IX_OSAL_LOG_LVL_MESSAGE);
#endif
	if(ixErrHdlAccInitDone == FALSE)
	{		
	  /* Initialize Recovery Task */
	  if(ixErrHdlAccRecoveryTaskInit() == IX_FAIL)
   {
     IX_ERRHDLACC_FATAL_LOG("ixErrHdlAccInit:" 
                            "Init Failure.",
                            0, 0, 0, 0, 0, 0);
      return IX_FAIL;
   }	 
   ixErrHdlAccInitDone = TRUE;
	} 
	return IX_SUCCESS;
}

/*
 * ixErrHdlAccUnload: Unload the component
 */
PUBLIC IX_STATUS ixErrHdlAccUnload(void)
{
 if(FALSE == ixErrHdlAccInitDone)
	{
	 IX_ERRHDLACC_WARNING_LOG("\n ixErrHdlAccUnload: Warning:" 
                           "Unload of an uninitialize component.\n", 
                            0, 0, 0, 0, 0, 0);
  return IX_SUCCESS;
 }
 ixErrHdlAccInitDone = FALSE;
	if(ixErrHdlAccRecoveryTaskUnInit() == IX_FAIL)
 {
   IX_ERRHDLACC_FATAL_LOG("ixErrHdlAccUnload:" 
                          "Unload Failure.",
                           0, 0, 0, 0, 0, 0);
   return IX_FAIL;
 }	
 return IX_SUCCESS;
}







