/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/sysdev.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/setup.h>
#include <asm/mach-types.h>

#include <asm/mach/arch.h>
#include <asm/mach/irq.h>
#include <asm/mach/map.h>

#include <asm/arch/irqs.h>
#include "mvOs.h"
#include "ctrlEnv/mvCtrlEnvLib.h"
#include "gpp/mvGpp.h"
#include "ctrlEnv/sys/mvCpuIf.h"
#include "cpu/mvCpu.h"

static int coreId = 0;

static void mv_mask_irq(unsigned int irq)
{	
	if(irq < 32)
		MV_REG_BIT_RESET(CPU_INT_MASK_LOW_REG(coreId), (1 << irq) );
	else if(irq < 64)/* irq > 32 && irq < 64 */
		MV_REG_BIT_RESET(CPU_INT_MASK_HIGH_REG(coreId), (1 << (irq - 32)));
	else /* gpp */
		MV_REG_BIT_RESET(GPP_INT_LVL_REG(0), (1 << (irq - 64)));
}

static void mv_unmask_irq(unsigned int irq)
{
	
	if (irq < 32)
		MV_REG_BIT_SET(CPU_INT_MASK_LOW_REG(coreId), (1 << irq));
	else if (irq < 64) /* irq > 32 && irq < 64 */
        	MV_REG_BIT_SET(CPU_INT_MASK_HIGH_REG(coreId), (1 << (irq - 32)));
	else
		MV_REG_BIT_SET(GPP_INT_LVL_REG(0), (1 << (irq - 64)) );
}

struct irq_chip mv_chip = {
	.ack	= mv_mask_irq,
	.mask	= mv_mask_irq,
	.unmask = mv_unmask_irq,
};

void __init mv_init_irq(void)
{
	u32 gppMask,i;
#ifdef CONFIG_MV78200
	coreId = whoAmI();
	printk("IRQ initialize for core %d\n", coreId);
#endif
	/* Disable all interrupts initially. */	
	if (0 == coreId)
	{
		MV_REG_WRITE(GPP_INT_MASK_REG(0), 0);
		MV_REG_WRITE(GPP_INT_LVL_REG(0), 0);
	}	
	MV_REG_WRITE(CPU_INT_MASK_LOW_REG(coreId), 0);
	MV_REG_WRITE(CPU_INT_MASK_HIGH_REG(coreId), 0);

	/* Set Gpp interrupts as needed */
       if (0 == coreId) /*GPP for core 0 only*/
       {       
	       gppMask = mvBoardGpioIntMaskGet();
	       mvGppTypeSet(0, gppMask , (MV_GPP_IN & gppMask));
	       mvGppPolaritySet(0, gppMask , (MV_GPP_IN_INVERT & gppMask));
		
		/* clear all int */
		MV_REG_WRITE(GPP_INT_MASK_REG(0), 0);
		MV_REG_WRITE(CPU_INT_MASK_HIGH_REG(coreId), IRQ_GPP_MASK);	
       }
       else
	       MV_REG_WRITE(CPU_INT_MASK_HIGH_REG(coreId), 0);	

	/* Do the core module ones */
	for (i = 0; i < NR_IRQS; i++) {
		set_irq_chip(i, &mv_chip);
		set_irq_handler(i, handle_level_irq);
		set_irq_flags(i, IRQF_VALID | IRQF_PROBE);
	}
	/* TBD. Add support for error interrupts */
	return;
}

