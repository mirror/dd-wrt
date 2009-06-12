/*
 * linux/include/asm-arm/arch-sa1100/ide.h
 *
 * Copyright (c) 1998 Hugo Fiennes & Nicolas Pitre
 *
 * 18-aug-2000: Cleanup by Erik Mouw (J.A.K.Mouw@its.tudelft.nl)
 *              Get rid of the special ide_init_hwif_ports() functions
 *              and make a generalised function that can be used by all
 *              architectures.
 */

#include <linux/config.h>
#include <asm/irq.h>
#include <asm/hardware.h>
#include <asm/mach-types.h>


/*
 * Set up a hw structure for a specified data port, control port and IRQ.
 * This should follow whatever the default interface uses.
 */
static __inline__ void
ide_init_hwif_ports(hw_regs_t *hw, int data_port, int ctrl_port, int *irq)
{
#ifdef CONFIG_REVISIT
#error Not sure what to do for ide_init_hwif_ports
#endif
}




/*
 * This registers the standard ports for this architecture with the IDE
 * driver.
 */
static __inline__ void
ide_init_default_hwifs(void)
{
#ifdef CONFIG_REVISIT
#error Not sure what to do for ide_init_default_hwifs
#endif
}


