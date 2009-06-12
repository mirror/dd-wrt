//==========================================================================
//
//      integrator_misc.c
//
//      HAL misc board support code for ARM INTEGRATOR7
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    David A Rusling
// Contributors: Philippe Robin
// Date:         November 7, 2000
// Purpose:      HAL board support
// Description:  Implementations of HAL board interfaces
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // necessary?
#include <cyg/hal/hal_integrator.h>

/*------------------------------------------------------------------------*/
// On-board timer
/*------------------------------------------------------------------------*/

// forward declarations
void hal_if_init(void);

// declarations
static cyg_uint32 _period;

void hal_clock_initialize(cyg_uint32 period)
{
    //diag_init();  diag_printf("%s(%d)\n", __PRETTY_FUNCTION__, period);
    //diag_printf("psr = %x\n", psr());
    HAL_WRITE_UINT32(CYG_DEVICE_TIMER_CONTROL, CTL_DISABLE);    // Turn off
    HAL_WRITE_UINT32(CYG_DEVICE_TIMER_LOAD, period);
    HAL_WRITE_UINT32(CYG_DEVICE_TIMER_CONTROL,
		     CTL_ENABLE | CTL_PERIODIC | CTL_SCALE_16);
    _period = period;
}

void hal_clock_reset(cyg_uint32 vector, cyg_uint32 period)
{
    //diag_init();  diag_printf("%s\n", __PRETTY_FUNCTION__);
    HAL_WRITE_UINT32(CYG_DEVICE_TIMER_CLEAR, 0);
    _period = period;
}

void hal_clock_read(cyg_uint32 *pvalue)
{
    cyg_uint32 value;
//    diag_init();  diag_printf("%s\n", __PRETTY_FUNCTION__);
    HAL_READ_UINT32(CYG_DEVICE_TIMER_CURRENT, value);
    value &= 0xFFFF;
    *pvalue = _period - (value & 0xFFFF);   // Note: counter is only 16 bits
                                            //       and decreases
}

// Delay for some usecs.
void hal_delay_us(cyg_uint32 delay)
{
#if 0
    int i;
    for( i = 0; i < delay; i++ );

#else
    cyg_uint32 now, last, diff, ticks;

    // The timer actually runs at 1.25 ticks per micrsecond.
    // Adjust the supplied delay to compensate.
    
    delay *= 4;
    delay /= 5;
    
    hal_clock_read(&last);
    diff = ticks = 0;

    while (delay > ticks) {
	hal_clock_read(&now);

	// Cope with wrap-around of timer
	if (now < last)
	    diff = ((_period - last) + now);
	else
	    diff = (now - last);

	last = now;

	ticks += diff;
    }
#endif
}


#if defined(CYGPKG_HAL_ARM_INTEGRATOR_ARM7)
void hal_hardware_init(void)
#elif defined(CYGPKG_HAL_ARM_INTEGRATOR_ARM9)
void plf_hardware_init(void)
#endif
{
    // Any hardware/platform initialization that needs to be done.

    // Clear all interrupt sources
    HAL_WRITE_UINT32(CYG_DEVICE_IRQ_EnableClear, 0xFFFF); 

#ifndef CYGFUN_HAL_COMMON_KERNEL_SUPPORT
    HAL_CLOCK_INITIALIZE( CYGNUM_HAL_RTC_PERIOD );
#endif
    
    // FIXME: The line with the thumb check is a hack, allowing
    // the farm to run test. Problem is that virtual vector table
    // API needs to be ARM/Thumb consistent. Will fix later.
#if !defined(__thumb__) && !defined(CYGPKG_HAL_ARM_INTEGRATOR_ARM9)
    // Set up eCos/ROM interfaces
    hal_if_init();
#endif
}

//
// This routine is called to respond to a hardware interrupt (IRQ).  It
// should interrogate the hardware and return the IRQ vector number.

int hal_IRQ_handler(void)
{
    // Do hardware-level IRQ handling
    int irq_status, vector;
    HAL_READ_UINT32(CYG_DEVICE_IRQ_Status, irq_status);
    //diag_init();  diag_printf("IRQ status: 0x%x\n", irq_status); 
    for (vector = 1;  vector <= 16;  vector++) {
        if (irq_status & (1<<vector)) return vector;
    }
    return -1 ; // This shouldn't happen!
}

//
// Interrupt control
//

void hal_interrupt_mask(int vector)
{
    //diag_init();  diag_printf("hal_interrupt_mask(%d)\n", vector);
    HAL_WRITE_UINT32(CYG_DEVICE_IRQ_EnableClear, 1<<vector);
}

#if 0
void hal_interrupt_status(void)
{
    int irq_status, irq_enable, timer_status, timer_value, timer_load;
    HAL_READ_UINT32(CYG_DEVICE_IRQ_Status, irq_status);
    HAL_READ_UINT32(CYG_DEVICE_IRQ_Enable, irq_enable);
    HAL_READ_UINT32(CYG_DEVICE_TIMER_LOAD, timer_load);
    HAL_READ_UINT32(CYG_DEVICE_TIMER_CURRENT, timer_value);
    HAL_READ_UINT32(CYG_DEVICE_TIMER_CONTROL, timer_status);    
    diag_printf("Interrupt: IRQ: %x.%x, TIMER: %x.%x.%x, psr: %x\n",
                irq_status, irq_enable, timer_status, timer_value,
                timer_load, psr());
}
#endif

void hal_interrupt_unmask(int vector)
{
    //diag_init();  diag_printf("hal_interrupt_unmask(%d)\n", vector);
    HAL_WRITE_UINT32(CYG_DEVICE_IRQ_EnableSet, 1<<vector);
}

void hal_interrupt_acknowledge(int vector)
{
    //diag_init();  diag_printf("%s(%d)\n", __PRETTY_FUNCTION__, vector);
}

void hal_interrupt_configure(int vector, int level, int up)
{
    //diag_init();  diag_printf("%s(%d,%d,%d)\n", __PRETTY_FUNCTION__, vector, level, up);
}

void hal_interrupt_set_level(int vector, int level)
{
    //diag_init();  diag_printf("%s(%d,%d)\n", __PRETTY_FUNCTION__, vector, level);
}

void hal_show_IRQ(int vector, int data, int handler)
{
    //    diag_printf("IRQ - vector: %x, data: %x, handler: %x\n", vector, data, handler);
}

/*---------------------------------------------------------------------------*/

__externC void cyg_plf_pci_init(void)
{
    // Only do this for non-RAM startups. If we do it during RAM
    // startup and we are using the ethernet for debugging, this kills
    // the ethernet controller.
#ifndef CYG_HAL_STARTUP_RAM
    
    volatile int i, j;

    /* setting this register will take the V3 out of reset */

    *(volatile cyg_uint32 *)(INTEGRATOR_SC_PCIENABLE) = 1;

    /* wait a few usecs to settle the device and the PCI bus */

    for (i = 0; i < 100 ; i++)
	   j = i + 1;

    /* Now write the Base I/O Address Word to V3_BASE + 0x6C */

    *(volatile cyg_uint16 *)(V3_BASE + V3_LB_IO_BASE) = (cyg_uint16)(V3_BASE >> 16);

    do {
        *(volatile cyg_uint8 *)(V3_BASE + V3_MAIL_DATA) = 0xAA;
	*(volatile cyg_uint8 *)(V3_BASE + V3_MAIL_DATA + 4) = 0x55;
    } while (*(volatile cyg_uint8 *)(V3_BASE + V3_MAIL_DATA) != 0xAA ||
	     *(volatile cyg_uint8 *)(V3_BASE + V3_MAIL_DATA + 4) != 0x55);

    /* Make sure that V3 register access is not locked, if it is, unlock it */

    if ((*(volatile cyg_uint16 *)(V3_BASE + V3_SYSTEM) & V3_SYSTEM_M_LOCK)
				== V3_SYSTEM_M_LOCK)
	*(volatile cyg_uint16 *)(V3_BASE + V3_SYSTEM) = 0xA05F;

    /* Ensure that the slave accesses from PCI are disabled while we */
    /* setup windows */

    *(volatile cyg_uint16 *)(V3_BASE + V3_PCI_CMD) &=
				~(V3_COMMAND_M_MEM_EN | V3_COMMAND_M_IO_EN);

    /* Clear RST_OUT to 0; keep the PCI bus in reset until we've finished */

    *(volatile cyg_uint16 *)(V3_BASE + V3_SYSTEM) &= ~V3_SYSTEM_M_RST_OUT;

    /* Make all accesses from PCI space retry until we're ready for them */

    *(volatile cyg_uint16 *)(V3_BASE + V3_PCI_CFG) |= V3_PCI_CFG_M_RETRY_EN;

    /* Set up any V3 PCI Configuration Registers that we absolutely have to */
    /* LB_CFG controls Local Bus protocol. */
    /* Enable LocalBus byte strobes for READ accesses too. */
    /* set bit 7 BE_IMODE and bit 6 BE_OMODE */

    *(volatile cyg_uint16 *)(V3_BASE + V3_LB_CFG) |= 0x0C0;

    /* PCI_CMD controls overall PCI operation. */
    /* Enable PCI bus master. */

    *(volatile cyg_uint16 *)(V3_BASE + V3_PCI_CMD) |= 0x04;

    /* PCI_MAP0 controls where the PCI to CPU memory window is on Local Bus*/

    *(volatile cyg_uint32 *)(V3_BASE + V3_PCI_MAP0) = (INTEGRATOR_BOOT_ROM_BASE) |
					(V3_PCI_MAP_M_ADR_SIZE_512M |
					V3_PCI_MAP_M_REG_EN |
					V3_PCI_MAP_M_ENABLE);

    /* PCI_BASE0 is the PCI address of the start of the window */

    *(volatile cyg_uint32 *)(V3_BASE + V3_PCI_BASE0) = INTEGRATOR_BOOT_ROM_BASE;

    /* PCI_MAP1 is LOCAL address of the start of the window */

    *(volatile cyg_uint32 *)(V3_BASE + V3_PCI_MAP1) = (INTEGRATOR_HDR0_SDRAM_BASE) |
			(V3_PCI_MAP_M_ADR_SIZE_1024M | V3_PCI_MAP_M_REG_EN |
			 V3_PCI_MAP_M_ENABLE);

    /* PCI_BASE1 is the PCI address of the start of the window */

    *(volatile cyg_uint32 *)(V3_BASE + V3_PCI_BASE1) = INTEGRATOR_HDR0_SDRAM_BASE;

    /* Set up the windows from local bus memory into PCI configuration, */
    /* I/O and Memory. */
    /* PCI I/O, LB_BASE2 and LB_MAP2 are used exclusively for this. */

    *(volatile cyg_uint16 *)(V3_BASE +V3_LB_BASE2) =
			((CPU_PCI_IO_ADRS >> 24) << 8) | V3_LB_BASE_M_ENABLE;
    *(volatile cyg_uint16 *)(V3_BASE + V3_LB_MAP2) = 0;

    /* PCI Configuration, use LB_BASE1/LB_MAP1. */

    /* PCI Memory use LB_BASE0/LB_MAP0 and LB_BASE1/LB_MAP1 */
    /* Map first 256Mbytes as non-prefetchable via BASE0/MAP0 */
    /* (INTEGRATOR_PCI_BASE == PCI_MEM_BASE) */

    *(volatile cyg_uint32 *)(V3_BASE + V3_LB_BASE0) =
			INTEGRATOR_PCI_BASE | (0x80 | V3_LB_BASE_M_ENABLE);

    *(volatile cyg_uint16 *)(V3_BASE + V3_LB_MAP0) =
			((INTEGRATOR_PCI_BASE >> 20) << 0x4) | 0x0006;

    /* Map second 256 Mbytes as prefetchable via BASE1/MAP1 */

    *(volatile cyg_uint32 *)(V3_BASE + V3_LB_BASE1) =
			INTEGRATOR_PCI_BASE | (0x84 | V3_LB_BASE_M_ENABLE);

    *(volatile cyg_uint16 *)(V3_BASE + V3_LB_MAP1) =
			(((INTEGRATOR_PCI_BASE + SZ_256M) >> 20) << 4) | 0x0006;

    /* Allow accesses to PCI Configuration space */
    /* and set up A1, A0 for type 1 config cycles */

    *(volatile cyg_uint16 *)(V3_BASE + V3_PCI_CFG) =
			((*(volatile cyg_uint16 *)(V3_BASE + V3_PCI_CFG)) &
			   ~(V3_PCI_CFG_M_RETRY_EN | V3_PCI_CFG_M_AD_LOW1) ) |
			   V3_PCI_CFG_M_AD_LOW0;

    /* now we can allow in PCI MEMORY accesses */

    *(volatile cyg_uint16 *)(V3_BASE + V3_PCI_CMD) =
		(*(volatile cyg_uint16 *)(V3_BASE + V3_PCI_CMD)) | V3_COMMAND_M_MEM_EN;

    /* Set RST_OUT to take the PCI bus is out of reset, PCI devices can */
    /* initialise and lock the V3 system register so that no one else */
    /* can play with it */

   *(volatile cyg_uint16 *)(V3_BASE + V3_SYSTEM) =
		(*(volatile cyg_uint16 *)(V3_BASE + V3_SYSTEM)) | V3_SYSTEM_M_RST_OUT;

   *(volatile cyg_uint16 *)(V3_BASE + V3_SYSTEM) =
		(*(volatile cyg_uint16 *)(V3_BASE + V3_SYSTEM)) | V3_SYSTEM_M_LOCK;

#endif
}

/*---------------------------------------------------------------------------*/
/* End of hal_misc.c */
