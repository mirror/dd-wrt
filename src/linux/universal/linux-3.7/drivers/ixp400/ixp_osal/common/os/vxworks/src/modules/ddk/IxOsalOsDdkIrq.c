/**
 * @file IxOsalOsServices.c (vxWorks)
 *
 * @brief Implementation for Irq, Mem, sleep. 
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

#include <vxWorks.h>
#include <iv.h>
#include <intLib.h>
#include <sysLib.h>
#include <logLib.h>
#include <taskLib.h>
#include <string.h>

#include "IxOsal.h"

/* dummy ISR to register during unBind */
PRIVATE void ixOsalDummyIsr (void *parameter);
PRIVATE void
ixOsalDummyIsr (void *parameter)
{
}

/**************************************
 * Irq services 
 *************************************/

PUBLIC IX_STATUS
ixOsalIrqBind (UINT32 irqLevel, IxOsalVoidFnVoidPtr routine, void *parameter)
{

#if((CPU==SIMSPARCSOLARIS) || (CPU==SIMLINUX))
    /*
     * No Irq support in simulation env 
     */
    return IX_FAIL;
#else
    if (intConnect
        (INUM_TO_IVEC(irqLevel),
	(IxOsalVoidFnVoidPtr) routine, 
	(int) parameter) != OK)
    {
        return IX_FAIL;
    }
    intEnable (irqLevel);
    return IX_SUCCESS;

#endif

}

PUBLIC IX_STATUS
ixOsalIrqUnbind (UINT32 irqLevel)
{
    /*
     * disable interrupts for this vector 
     */
    if (intDisable (IVEC_TO_INUM (irqLevel)) != OK)
    {
        return IX_FAIL;
    }

    /*
     * register a dummy ISR 
     */
    if (intConnect (INUM_TO_IVEC(irqLevel), 
	           (IxOsalVoidFnVoidPtr) ixOsalDummyIsr, 0) != OK)
    {
        return IX_FAIL;
    }
    return IX_SUCCESS;
}

PUBLIC UINT32
ixOsalIrqLock ()
{
#if((CPU==SIMSPARCSOLARIS) || (CPU==SIMLINUX))

    ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        IX_OSAL_LOG_DEV_STDOUT,
        "ixOsalIrqLock: not supported in simulation  \n", 0, 0, 0, 0, 0, 0);

    return 0;

#else
    return intLock ();
#endif

}

/* Enable interrupts and task scheduling,
 * input parameter: irqEnable status returned
 * by ixOsalIrqLock().
 */
PUBLIC void
ixOsalIrqUnlock (UINT32 irqEnable)
{

#if((CPU==SIMSPARCSOLARIS) || (CPU==SIMLINUX))
    ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        IX_OSAL_LOG_DEV_STDOUT,
        "ixOsalIrqUnlock: not supported in simulation  \n", 0, 0, 0, 0, 0, 0);
#else
    intUnlock (irqEnable);
#endif

}

PUBLIC void
ixOsalIrqEnable (UINT32 irqLevel)
{
    intEnable (irqLevel);
}

PUBLIC void
ixOsalIrqDisable (UINT32 irqLevel)
{
    intDisable (irqLevel);
}

/** 
 * Return previous int level, return IX_FAIL if fail.
 * Interrupt are locked out at or below that level.
 */
PUBLIC UINT32
ixOsalIrqLevelSet (UINT32 irqLevel)
{

#if((CPU==SIMSPARCSOLARIS) || (CPU==SIMLINUX))
    /*
     * No irq support in simulation, log error and return any 
     * * number, in this case we return zero.
     */
    ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        IX_OSAL_LOG_DEV_STDOUT,
        "ixOsalIrqLevelSet: not supported in simulation  \n",
        0, 0, 0, 0, 0, 0);
    return 0;
#else
    return intLevelSet (irqLevel);
#endif
}
