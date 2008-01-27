/**
 * @file IxOsalOsDdkIrq.c (linux)
 *
 * @brief System Interrupt functions.
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

#include <linux/version.h>

#ifdef IX_OSAL_OS_LINUX_VERSION_2_6 
/* #if KERNEL_VERSION(2,6,0) <= LINUX_VERSION_CODE */
#include <linux/hardirq.h>
#include <linux/interrupt.h>
#endif

#include <asm/hardirq.h>
#include <asm/system.h>

#include "IxOsalOsOem.h"

#include <asm/irq.h>



#include "IxOsalOsOemIrq.h"



/* 
 * Note: being referenced by some release 1.4 linux 
 * components as global .
 */
/**
 * This needs to be moved to IxOsalOem.h file
 */
typedef struct IxOsalInfoType
{
    void (*routine) (void *);
    void *parameter;
} IxOsalInfoType;

static IxOsalInfoType IxOsalInfo[NR_IRQS];

/*
 * General interrupt handler
 */

/*
 * Private utility function for ixOsalIrqBind to translate IRQ vector number to
 * IRQ name.
 */
PRIVATE char*
ixOsalGetIrqNameByVector(UINT32 vector)
{
    if (unlikely (ARRAY_SIZE(irq_name) < vector))
        return ((char*)invalid_irq_name);

    return ((char*)irq_name[vector]);
}

#ifdef IX_OSAL_OS_LINUX_VERSION_2_6 
static irqreturn_t
#else
static void
#endif
ixOsalOsIsrProxy (int irq, void *dev_id)
{
    IxOsalInfoType *isr_proxy_info = (IxOsalInfoType *) dev_id;

    IX_OSAL_ENSURE(isr_proxy_info && isr_proxy_info->routine,
		   "ixOsalOsIsrProxy: Interrupt used before ixOsalIrqBind was invoked");

    isr_proxy_info->routine (isr_proxy_info->parameter);

#ifdef IX_OSAL_OS_LINUX_VERSION_2_6 
    return IRQ_HANDLED;
#endif

}

/*
 * Interrupt handler for XScale PMU interrupts
 * This handler saves the interrupted Program Counter (PC)
 * into a global variable
 */
#ifdef IX_OSAL_OS_LINUX_VERSION_2_6 
static irqreturn_t
#else
static void
#endif
ixOsalOsIsrProxyWithPC (int irq, void *dev_id)
{
/**
 * The variable to me moved to IxOSalOem.h file
 */
//	IX_OSAL_OEM_SET_INTERRUPTED_PC(regs);
	
	ixOsalOsIsrProxy(irq, dev_id);

#ifdef IX_OSAL_OS_LINUX_VERSION_2_6 
/* #if KERNEL_VERSION(2,6,0) <= LINUX_VERSION_CODE */
    return IRQ_HANDLED;

#endif

}

/**************************************
 * Irq services 
 *************************************/

PUBLIC IX_STATUS
ixOsalIrqBind (UINT32 vector, IxOsalVoidFnVoidPtr routine, void *parameter)
{
    if (vector >= NR_IRQS)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalIrqBind: Invalid Interrupt Number %d \n", vector, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    if (IxOsalInfo[vector].routine)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalIrqBind: NULL function routine. \n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    IxOsalInfo[vector].routine = routine;
    IxOsalInfo[vector].parameter = parameter;

    /*
     * The PMU interrupt handler is a special case in the sense
     * that it needs to save the address of the interrupted PC
     * In the case of the IA/XScale PMU interrupt, the ixOsalOsIsrProxyWithPC
     * function is registered
     */

	if (vector == IX_OSAL_OEM_IRQ_PMU)
	{
	/*
	 * request_irq will enable interrupt automatically 
	 * A non-zero return value suggest a failure.
	 */
	if (request_irq (vector, ixOsalOsIsrProxyWithPC, IRQF_SHARED, 
			(UINT8 *)ixOsalGetIrqNameByVector(vector),
			 &IxOsalInfo[vector]))
	{
	    ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
		       IX_OSAL_LOG_DEV_STDOUT,
		       "ixOsalIrqBind: Fail to request irq. \n",
		       0, 0, 0, 0, 0, 0);
	    return IX_FAIL;
	}
    }
    else
    {
	/*
	 * request_irq will enable interrupt automatically 
	 * A non-zero return value suggest a failure.
	 */
	if (request_irq (vector, ixOsalOsIsrProxy, IRQF_SHARED,
			(UINT8 *)ixOsalGetIrqNameByVector(vector),
			 &IxOsalInfo[vector]))
	{
	    ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
		       IX_OSAL_LOG_DEV_STDOUT,
		       "ixOsalIrqBind: Fail to request irq. \n",
		       0, 0, 0, 0, 0, 0);
	    return IX_FAIL;
	}
    }
    return IX_SUCCESS;
}

PUBLIC IX_STATUS
ixOsalIrqUnbind (UINT32 vector)
{
    if (!IxOsalInfo[vector].routine)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalIrqUnbind: NULL function routine. \n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    free_irq (vector, &IxOsalInfo[vector]);
    IxOsalInfo[vector].routine = NULL;

    return IX_SUCCESS;
}

PUBLIC UINT32
ixOsalIrqLock ()
{
    UINT32 flags;
#ifdef IX_OSAL_OS_LINUX_VERSION_2_6 
    /* local_irq_save() gives a compilation warning against s32
     * data type. */

	/* Alternate implementation */
	  local_save_flags(flags);
      local_irq_disable();
	 /**/
    //local_irq_save(flags);

#else

    save_flags (flags);
    cli ();

#endif /* LINUX_VERSION */

    return flags;
}

/* Enable interrupts and task scheduling,
 * input parameter: irqEnable status returned
 * by ixOsalIrqLock().
 */
PUBLIC void
ixOsalIrqUnlock (UINT32 lockKey)
{

#ifdef IX_OSAL_OS_LINUX_VERSION_2_6 
/* #if KERNEL_VERSION(2,6,0) <= LINUX_VERSION_CODE */
	local_irq_restore((unsigned long)lockKey);

#else
	
	restore_flags (lockKey);

#endif
}

PUBLIC UINT32
ixOsalIrqLevelSet (UINT32 level)
{
    /*
     * Not supported 
     */
    ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,
        IX_OSAL_LOG_DEV_STDOUT,
        "ixOsalIrqLevelSet: not supported \n", 0, 0, 0, 0, 0, 0);
    return 0;
}

PUBLIC void
ixOsalIrqEnable (UINT32 irqLevel)
{
    if (irqLevel < NR_IRQS)
    {
        enable_irq(irqLevel);
    }
    else
    {
        /*
         * Not supported 
         */
        ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalIrqEnable: IRQ %d not supported \n", irqLevel, 0, 0, 0, 0, 0);
    }
}

PUBLIC void
ixOsalIrqDisable (UINT32 irqLevel)
{
    if (irqLevel < NR_IRQS)
    {
        disable_irq(irqLevel);
    }
    else
    {
        /*
         * Not supported 
         */
        ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalIrqDisable: IRQ %d not supported \n", irqLevel, 0, 0, 0, 0, 0);
    }
}




