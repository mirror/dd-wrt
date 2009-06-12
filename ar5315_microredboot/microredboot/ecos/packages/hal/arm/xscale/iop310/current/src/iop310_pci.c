//==========================================================================
//
//      iop310_pci.c
//
//      HAL board support code for XScale IOP310 PCI
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
// Author(s):    msalter
// Contributors: msalter
// Date:         2000-10-10
// Purpose:      PCI support
// Description:  Implementations of HAL PCI interfaces
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H
#include CYGHWR_MEMORY_LAYOUT_H

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_if.h>             // calling interface API
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_iop310.h>         // Hardware definitions
#include <cyg/io/pci_hw.h>
#include <cyg/io/pci.h>

static cyg_uint8 pbus_nr;
static cyg_uint8 sbus_nr;
static cyg_uint8 subbus_nr;

void cyg_hal_plf_pci_init(void)
{
    cyg_uint32 limit_reg;
    cyg_uint8  next_bus;

    // ************ bridge registers *******************
    if (iop310_is_host()) {

	// set the primary inbound ATU base address to the start of DRAM
	*PIABAR_REG = MEMBASE_DRAM & 0xFFFFF000;

	// ********* Set Primary Outbound Windows *********

	// Note: The primary outbound ATU memory window value register
	//       and i/o window value registers are defaulted to 0

	// set the primary outbound windows to directly map Local - PCI
        // requests
	// outbound memory window
	*POMWVR_REG = PRIMARY_MEM_BASE;

	// outbound DAC Window
	*PODWVR_REG = PRIMARY_DAC_BASE;

	// outbound I/O window
        *POIOWVR_REG = PRIMARY_IO_BASE;	

	// set the bridge command register
	*PCR_REG = (CYG_PCI_CFG_COMMAND_SERR   | \
		    CYG_PCI_CFG_COMMAND_PARITY | \
		    CYG_PCI_CFG_COMMAND_MASTER | \
		    CYG_PCI_CFG_COMMAND_MEMORY);

	// set the subordinate bus number to 0xFF
	*SUBBNR_REG = 0xFF;
	// set the secondary bus number to 1
	*SBNR_REG = SECONDARY_BUS_NUM;
	*BCR_REG = 0x0823;
	// set the primary bus number to 0
	*PBNR_REG = PRIMARY_BUS_NUM;

	// allow primary ATU to act as a bus master, respond to PCI 
	// memory accesses, assert P_SERR#, and enable parity checking
	*PATUCMD_REG = (CYG_PCI_CFG_COMMAND_SERR   | \
			CYG_PCI_CFG_COMMAND_PARITY | \
			CYG_PCI_CFG_COMMAND_MASTER | \
			CYG_PCI_CFG_COMMAND_MEMORY);
    } else {
#ifdef CYGSEM_HAL_ARM_IOP310_CLEAR_PCI_RETRY
	// Wait for PC BIOS to initialize bus number
	int i;

	for (i = 0; i < 15000; i++) {
	    if (*PCR_REG & CYG_PCI_CFG_COMMAND_MEMORY)
		break;
	    hal_delay_us(1000);  // 1msec
	}
	for (i = 0; i < 15000; i++) {
 	    if (*SBNR_REG != 0)
		break;
	    hal_delay_us(1000);  // 1msec
	}
#endif
        if (*SBNR_REG == 0)
            *SBNR_REG = SECONDARY_BUS_NUM;
        if (*SUBBNR_REG == 0)
            *SUBBNR_REG = 0xFF;
        if (*BCR_REG == 0)
            *BCR_REG = 0x0823;
        if (*PCR_REG == 0)
            *PCR_REG = (CYG_PCI_CFG_COMMAND_SERR   | \
			CYG_PCI_CFG_COMMAND_PARITY | \
			CYG_PCI_CFG_COMMAND_MASTER | \
			CYG_PCI_CFG_COMMAND_MEMORY);
        if (*PATUCMD_REG == 0)
            *PATUCMD_REG = (CYG_PCI_CFG_COMMAND_SERR   | \
			    CYG_PCI_CFG_COMMAND_PARITY | \
			    CYG_PCI_CFG_COMMAND_MASTER | \
			    CYG_PCI_CFG_COMMAND_MEMORY);
    }

    // Initialize Secondary PCI bus (bus 1)
    *BCR_REG |= 0x40;           // reset secondary bus
    hal_delay_us(10 * 1000); 	// 10ms enough??
    *BCR_REG &= ~0x40;          // release reset

    // ******** Secondary Inbound ATU ***********

    // set secondary inbound ATU translate value register to point to base
    // of local DRAM
    *SIATVR_REG = MEMBASE_DRAM & 0xFFFFFFFC;

    // set secondary inbound ATU base address to start of DRAM
    *SIABAR_REG = MEMBASE_DRAM & 0xFFFFF000;

    //  always allow secondary pci access to all memory (even with A0 step)
    limit_reg = (0xFFFFFFFF - (hal_dram_size - 1)) & 0xFFFFFFF0;
    *SIALR_REG = limit_reg;

    // ********** Set Secondary Outbound Windows ***********

    // Note: The secondary outbound ATU memory window value register
    // and i/o window value registers are defaulted to 0

    // set the secondary outbound window to directly map Local - PCI requests
    // outbound memory window
    *SOMWVR_REG = SECONDARY_MEM_BASE;

    // outbound DAC Window
    *SODWVR_REG = SECONDARY_DAC_BASE;

    // outbound I/O window
    *SOIOWVR_REG = SECONDARY_IO_BASE;

    // allow secondary ATU to act as a bus master, respond to PCI memory
    // accesses, and assert S_SERR#
    *SATUCMD_REG = (CYG_PCI_CFG_COMMAND_SERR   | \
		    CYG_PCI_CFG_COMMAND_PARITY | \
		    CYG_PCI_CFG_COMMAND_MASTER | \
		    CYG_PCI_CFG_COMMAND_MEMORY);

    // enable primary and secondary outbound ATUs, BIST, and primary bus
    // direct addressing
    *ATUCR_REG = 0x00000006;

    pbus_nr = *PBNR_REG;
    sbus_nr = *SBNR_REG;

    // Now initialize the PCI busses.

    // Next assignable bus number. Yavapai primary bus is fixed as
    // bus zero and yavapai secondary is fixed as bus 1.
    next_bus = sbus_nr + 1;

    // If we are the host on the Primary bus, then configure it.
    if (iop310_is_host()) {

	// Initialize these so all config cycles first go out over
	// the Primary side
	pbus_nr = 0;
	sbus_nr = 0xff;

	// set the primary bus number to 0
	*PBNR_REG = 0;
	next_bus = 1;

	// Initialize Primary PCI bus (bus 0)
	cyg_pci_set_memory_base(PRIMARY_MEM_BASE);
	cyg_pci_set_io_base(PRIMARY_IO_BASE);
	cyg_pci_configure_bus(0, &next_bus);

	// set the secondary bus number to next available number
	*SBNR_REG = sbus_nr = next_bus;

	pbus_nr = *PBNR_REG;
	next_bus = sbus_nr + 1;
    }

    // Initialize Secondary PCI bus (bus 1)
    cyg_pci_set_memory_base(SECONDARY_MEM_BASE);
    cyg_pci_set_io_base(SECONDARY_IO_BASE);
    subbus_nr = 0xFF;
    cyg_pci_configure_bus(sbus_nr, &next_bus);
    *SUBBNR_REG = subbus_nr = next_bus - 1;


    if (0){
	cyg_uint8 devfn;
	cyg_pci_device_id devid;
	cyg_pci_device dev_info;

        diag_printf("pbus[%d] sbus[%d] subbus[%d]\n", pbus_nr, sbus_nr, subbus_nr);

	devid = CYG_PCI_DEV_MAKE_ID(sbus_nr, 0) | CYG_PCI_NULL_DEVFN;
	while (cyg_pci_find_next(devid, &devid)) {
	    devfn = CYG_PCI_DEV_GET_DEVFN(devid);
	    cyg_pci_get_device_info(devid, &dev_info);

	    diag_printf("\n");
	    diag_printf("            Bus:        %d\n", CYG_PCI_DEV_GET_BUS(devid));
	    diag_printf("            PCI Device: %d\n", CYG_PCI_DEV_GET_DEV(devfn));
	    diag_printf("            PCI Func  : %d\n", CYG_PCI_DEV_GET_FN(devfn));
	    diag_printf("            Vendor Id : 0x%08X\n", dev_info.vendor);
	    diag_printf("            Device Id : 0x%08X\n", dev_info.device);
	}
    }
}

// Use "naked" attribute to suppress C prologue/epilogue
static void __attribute__ ((naked)) __pci_abort_handler(void) 
{
    asm ( "subs	pc, lr, #4\n" );
}

static cyg_uint32 orig_abort_vec;

static inline cyg_uint32 *pci_config_setup(cyg_uint32 bus,
					   cyg_uint32 devfn,
					   cyg_uint32 offset)
{
    volatile cyg_uint32 *pdata, *paddr;
    cyg_uint32 dev = CYG_PCI_DEV_GET_DEV(devfn);
    cyg_uint32 fn  = CYG_PCI_DEV_GET_FN(devfn);

    if (bus < sbus_nr || bus > subbus_nr)  {
        paddr = (volatile cyg_uint32 *)POCCAR_ADDR;
        pdata = (volatile cyg_uint32 *)POCCDR_ADDR;
    } else {
        paddr = (volatile cyg_uint32 *)SOCCAR_ADDR;
        pdata = (volatile cyg_uint32 *)SOCCDR_ADDR;
    }

    /* Offsets must be dword-aligned */
    offset &= ~3;
	
    /* Primary or secondary bus use type 0 config */
    /* all others use type 1 config */
    if (bus == pbus_nr || bus == sbus_nr)
	*paddr = ( (1 << (dev + 16)) | (fn << 8) | offset | 0 );
    else
        *paddr = ( (bus << 16) | (dev << 11) | (fn << 8) | offset | 1 );

    orig_abort_vec = ((volatile cyg_uint32 *)0x20)[4];
    ((volatile unsigned *)0x20)[4] = (unsigned)__pci_abort_handler;
    HAL_ICACHE_SYNC();

    return pdata;
}

static inline int pci_config_cleanup(cyg_uint32 bus)
{
    cyg_uint32 status = 0, err = 0;

    if (bus < sbus_nr || bus > subbus_nr)  {
	status = *PATUSR_REG;
	if ((status & 0xF900) != 0) {
	    err = 1;
	    *PATUSR_REG = status & 0xF980;
	}
	status = *PSR_REG;
	if ((status & 0xF900) != 0) {
	    err = 1;
	    *PSR_REG = status & 0xF980;
	}
	status = *PATUISR_REG;
	if ((status & 0x79F) != 0) {
	    err = 1;
	    *PATUISR_REG = status & 0x79f;
	}
	status = *PBISR_REG;
	if ((status & 0x3F) != 0) {
	    err = 1;
	    *PBISR_REG = status & 0x3F;
	}
    } else {
	status = *SATUSR_REG;
	if ((status & 0xF900) != 0) {
	    err = 1;
	    *SATUSR_REG = status & 0xF900;
	}
	status = *SSR_REG;
	if ((status & 0xF900) != 0) {
	    err = 1;
	    *SSR_REG = status & 0xF980;
	}
	status = *SATUISR_REG;
	if ((status & 0x69F) != 0) {
	    err = 1;
	    *SATUISR_REG = status & 0x69F;
	}
    }

    ((volatile unsigned *)0x20)[4] = orig_abort_vec;
    HAL_ICACHE_SYNC();

    return err;
}



cyg_uint32 cyg_hal_plf_pci_cfg_read_dword (cyg_uint32 bus,
					   cyg_uint32 devfn,
					   cyg_uint32 offset)
{
    cyg_uint32 *pdata, config_data;

    pdata = pci_config_setup(bus, devfn, offset);

    config_data = *pdata;

    if (pci_config_cleanup(bus))
      return 0xffffffff;
    else
      return config_data;
}


void cyg_hal_plf_pci_cfg_write_dword (cyg_uint32 bus,
				      cyg_uint32 devfn,
				      cyg_uint32 offset,
				      cyg_uint32 data)
{
    cyg_uint32 *pdata;

    pdata = pci_config_setup(bus, devfn, offset);

    *pdata = data;

    pci_config_cleanup(bus);
}


cyg_uint16 cyg_hal_plf_pci_cfg_read_word (cyg_uint32 bus,
					  cyg_uint32 devfn,
					  cyg_uint32 offset)
{
    cyg_uint32 *pdata;
    cyg_uint16 config_data;

    pdata = pci_config_setup(bus, devfn, offset);

    config_data = (cyg_uint16)(((*pdata) >> ((offset % 0x4) * 8)) & 0xffff);

    if (pci_config_cleanup(bus))
      return 0xffff;
    else
      return config_data;
}

void cyg_hal_plf_pci_cfg_write_word (cyg_uint32 bus,
				     cyg_uint32 devfn,
				     cyg_uint32 offset,
				     cyg_uint16 data)
{
    cyg_uint32 *pdata, mask, temp;

    pdata = pci_config_setup(bus, devfn, offset);

    mask = ~(0x0000ffff << ((offset % 0x4) * 8));

    temp = (cyg_uint32)(((cyg_uint32)data) << ((offset % 0x4) * 8));
    *pdata = (*pdata & mask) | temp; 

    pci_config_cleanup(bus);
}

cyg_uint8 cyg_hal_plf_pci_cfg_read_byte (cyg_uint32 bus,
					 cyg_uint32 devfn,
					 cyg_uint32 offset)
{
    cyg_uint32 *pdata;
    cyg_uint8 config_data;

    pdata = pci_config_setup(bus, devfn, offset);

    config_data = (cyg_uint8)(((*pdata) >> ((offset % 0x4) * 8)) & 0xff);

    if (pci_config_cleanup(bus))
	return 0xff;
    else
	return config_data;
}


void cyg_hal_plf_pci_cfg_write_byte (cyg_uint32 bus,
				     cyg_uint32 devfn,
				     cyg_uint32 offset,
				     cyg_uint8 data)
{
    cyg_uint32 *pdata, mask, temp;

    pdata = pci_config_setup(bus, devfn, offset);

    mask = ~(0x000000ff << ((offset % 0x4) * 8));
    temp = (cyg_uint32)(((cyg_uint32)data) << ((offset % 0x4) * 8));
    *pdata = (*pdata & mask) | temp; 

    pci_config_cleanup(bus);
}



