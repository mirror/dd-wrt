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

#include "ctrlEnv/mvCtrlEnvLib.h"
#include "gpp/mvGpp.h"
#include "mvOs.h"

#if (CONFIG_MV_GPP_MAX_PINS == 32)
unsigned int  irq_int_type[32];
#else
unsigned int irq_int_type[16];
#endif

static void mv_mask_irq(unsigned int irq)
{

	//printk(" main cause is %x , mask %x \n",MV_REG_READ(MV_IRQ_CAUSE_REG), MV_REG_READ(MV_IRQ_MASK_REG) );
	//printk(" gpp cause is %x , mask %x \n",MV_REG_READ(MV_GPP_IRQ_CAUSE_REG), MV_REG_READ(MV_GPP_IRQ_MASK_REG) );
	//printk(" data in %x  gpp outen %x \n\n", MV_REG_READ(0x10110), MV_REG_READ(0x10104));
	if(irq < 32)
		MV_REG_BIT_RESET(MV_IRQ_MASK_REG, (1 << irq) );
	else /* irq > 32 */
	{
		MV_REG_BIT_RESET(MV_GPP_IRQ_MASK_REG, (1 << (irq - 32)) );
	}
	return;
}

static void mv_unmask_irq(unsigned int irq)
{
	if(irq < 32)
		MV_REG_BIT_SET(MV_IRQ_MASK_REG, (1 << irq) );
	else /* irq > 32 */
	{
		MV_REG_BIT_SET(MV_GPP_IRQ_MASK_REG, (1 << (irq - 32)) );
		/* change polarity if needed*/
		if (irq_int_type[irq - 32] == GPP_IRQ_TYPE_CHANGE_LEVEL)
		{
			(MV_REG_READ(MV_GPP_IRQ_POLARITY) & (1 << (irq - 32)))?
				MV_REG_BIT_RESET(MV_GPP_IRQ_POLARITY, (1 << (irq - 32)) ):
				MV_REG_BIT_SET(MV_GPP_IRQ_POLARITY, (1 << (irq - 32)) );
        }

	}
	return;
}

struct irq_chip mv_chip = {
	.ack	= mv_mask_irq,
	.mask	= mv_mask_irq,
	.unmask = mv_unmask_irq,
};

void __init mv_init_irq(void)
{
	u32 gppMask,i;

	/* Set Gpp interrupts as needed */
     gppMask = mvBoardGpioIntMaskGet();
     mvGppTypeSet(0, gppMask , (MV_GPP_IN & gppMask));
     mvGppPolaritySet(0, gppMask , (MV_GPP_IN_INVERT & gppMask));
                                                                                                                                               
	/* Disable all interrupts initially. */
	MV_REG_WRITE(MV_IRQ_MASK_REG, 0x0);
	MV_REG_WRITE(MV_GPP_IRQ_MASK_REG, 0x0);
	MV_REG_WRITE(MV_GPP_IRQ_EDGE_REG, 0x0);

	/* enable GPP in the main cause */
	MV_REG_BIT_SET(MV_IRQ_MASK_REG, (1 << IRQ_GPP_0_7) | (1 << IRQ_GPP_8_15));
#if (CONFIG_MV_GPP_MAX_PINS == 32)
	MV_REG_BIT_SET(MV_IRQ_MASK_REG, (1 << IRQ_GPP_16_23) | (1 << IRQ_GPP_24_31));	
#endif

	/* clear all int */
	MV_REG_WRITE(MV_IRQ_CAUSE_REG, 0x0);
	MV_REG_WRITE(MV_GPP_IRQ_CAUSE_REG, 0x0);


	/* Do the core module ones */
	for (i = 0; i < NR_IRQS; i++) {
		set_irq_chip(i, &mv_chip);
		set_irq_handler(i, handle_level_irq);
		set_irq_flags(i, IRQF_VALID | IRQF_PROBE);
	}
	
	/* init GPP IRQs in default level mode*/

#if (CONFIG_MV_GPP_MAX_PINS == 32)
	for (i = 0; i < 32; i++)
#else
	for (i = 0; i < 16; i++)
#endif
	{
		irq_int_type[i] = GPP_IRQ_TYPE_LEVEL;
	}
	
	/* TBD. Add support for error interrupts */

	return;
}

