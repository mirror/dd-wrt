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


