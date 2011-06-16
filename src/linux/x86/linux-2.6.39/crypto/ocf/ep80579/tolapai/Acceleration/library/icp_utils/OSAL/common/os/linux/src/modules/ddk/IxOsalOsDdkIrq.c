/**
 * @file IxOsalOsDdkIrq.c (linux)
 *
 * @brief System Interrupt functions.
 * 
 * 
 * @par
 * This file is provided under a dual BSD/GPLv2 license.  When using or 
 *   redistributing this file, you may do so under either license.
 * 
 *   GPL LICENSE SUMMARY
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify 
 *   it under the terms of version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
 * 
 *   This program is distributed in the hope that it will be useful, but 
 *   WITHOUT ANY WARRANTY; without even the implied warranty of 
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 *   General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License 
 *   along with this program; if not, write to the Free Software 
 *   Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *   The full GNU General Public License is included in this distribution 
 *   in the file called LICENSE.GPL.
 * 
 *   Contact Information:
 *   Intel Corporation
 * 
 *   BSD LICENSE 
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 *   All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without 
 *   modification, are permitted provided that the following conditions 
 *   are met:
 * 
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided with the 
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its 
 *       contributors may be used to endorse or promote products derived 
 *       from this software without specific prior written permission.
 * 
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * 
 *  version: Security.L.1.0.3-98
 */

#include "IxOsal.h"

#include <linux/version.h>
#ifdef IX_OSAL_OS_LINUX_VER_GT_2_6_20
#include <linux/irq.h>
#endif
#ifdef IX_OSAL_OS_LINUX_VERSION_2_6 
#include <linux/hardirq.h>
#include <linux/interrupt.h>
#endif

#include <asm/hardirq.h>
#include <asm/system.h>
#include <asm/irq.h>

#include "IxOsalOsOem.h"
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
    voidFnVoidPtr routine;
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
PRIVATE const char*
ixOsalGetIrqNameByVector(UINT32 vector)
{
    if (unlikely (ARRAY_SIZE(irq_name) <= vector))
    {
        return (invalid_irq_name);
    }

    return (irq_name[vector]);
}


#ifdef IX_OSAL_OS_LINUX_VERSION_2_6 
static irqreturn_t
#else
static void
#endif
/*  Definition  of request_irq changed for 2.6.20 onwards */
#ifdef IX_OSAL_OS_LINUX_VER_GT_2_6_20
ixOsalOsIsrProxy (int irq, void *dev_id)
#else 
ixOsalOsIsrProxy (int irq, void *dev_id, struct pt_regs *regs)
#endif 
{
    IxOsalInfoType *isr_proxy_info = (IxOsalInfoType *) dev_id;
    
#ifdef IX_OSAL_OS_LINUX_VERSION_2_6 
    IX_OSAL_LOCAL_ENSURE(isr_proxy_info,
      "ixOsalOsIsrProxy: Interrupt used before "
      "ixOsalIrqBind was invoked (isr_proxy_info == NULL)\n",
      IRQ_NONE);

    IX_OSAL_LOCAL_ENSURE(isr_proxy_info->routine,
      "ixOsalOsIsrProxy: Interrupt used before "
      "ixOsalIrqBind was invoked (isr_proxy_info->routine == NULL)\n",
      IRQ_NONE);
#else
    IX_OSAL_ENSURE_JUST_RETURN(isr_proxy_info,
      "ixOsalOsIsrProxy: Interrupt used before "
      "ixOsalIrqBind was invoked (isr_proxy_info == NULL)\n");

    IX_OSAL_ENSURE_JUST_RETURN(isr_proxy_info->routine,
      "ixOsalOsIsrProxy: Interrupt used before "
      "ixOsalIrqBind was invoked (isr_proxy_info->routine == NULL)\n");
#endif

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

/*  Definition  of request_irq changed for 2.6.20 onwards */
#ifdef IX_OSAL_OS_LINUX_VER_GT_2_6_20

static irqreturn_t

ixOsalOsIsrProxyWithPC (int irq, void *dev_id)
{

    IX_OSAL_OEM_SET_INTERRUPTED_PC(get_irq_regs());

    return ixOsalOsIsrProxy(irq, dev_id);

}

/* prior to 2.6.20 */
#else

#ifdef IX_OSAL_OS_LINUX_VERSION_2_6 
static irqreturn_t
#else
static void
#endif
ixOsalOsIsrProxyWithPC (int irq, void *dev_id, struct pt_regs *regs)
{
    /**
     * The variable to me moved to IxOSalOem.h file
     */
    IX_OSAL_OEM_SET_INTERRUPTED_PC(regs); 
    
#ifdef IX_OSAL_OS_LINUX_VERSION_2_6 
    return ixOsalOsIsrProxy(irq, dev_id, regs);
#else
    ixOsalOsIsrProxy(irq, dev_id, regs);
#endif

}

#endif /*endif prior to 2.6.20 */





/**************************************
 * Irq services 
 *************************************/

PUBLIC IX_STATUS
ixOsalIrqBind (UINT32 vector, IxOsalVoidFnVoidPtr routine, void *parameter)
{

    if (vector >= NR_IRQS)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalIrqBind: Invalid Interrupt Number %d \n", vector, \
            0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    if (IxOsalInfo[vector].routine)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
        /*"ixOsalIrqBind: NULL function routine. \n", 0, 0, 0, 0, 0, 0); */
            "ixOsalIrqBind: interrupt vector %d already binded. \n",
            vector, 0, 0, 0, 0, 0);
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
                         ixOsalGetIrqNameByVector(vector),
                         &IxOsalInfo[vector]))
        {
            ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
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
                         ixOsalGetIrqNameByVector(vector), &IxOsalInfo[vector]))
        {
            ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
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
ixOsalIrqLock (void)
{
    unsigned long flags;
#ifdef IX_OSAL_OS_LINUX_VERSION_2_6 
    /* local_irq_save() gives a compilation warning against s32
     * data type. */

    /* Alternate implementation */
#ifdef IX_OSAL_OS_LINUX_VERSION_2_6_18
    local_save_flags(flags);
#else
    local_save_flags(flags);
#endif
    local_irq_disable();
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
    ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,
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
        ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalIrqEnable: IRQ %d not supported \n", irqLevel,
            0, 0, 0, 0, 0);
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
        ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalIrqDisable: IRQ %d not supported \n", irqLevel,
             0, 0, 0, 0, 0);
    }
}

void *
ixOsalMemAllocAtomic (UINT32 memsize)
{
    return (kmalloc (memsize, GFP_ATOMIC));
}

