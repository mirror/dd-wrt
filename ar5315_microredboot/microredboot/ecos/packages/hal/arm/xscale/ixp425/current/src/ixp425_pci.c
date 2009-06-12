//==========================================================================
//
//      ixp425_pci.c
//
//      HAL support code for IXP425 PCI
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.
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
// Date:         2002-12-08
// Purpose:      PCI support
// Description:  Implementations of HAL PCI interfaces
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H

#ifdef CYGPKG_IO_PCI

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_if.h>             // calling interface API
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/io/pci_hw.h>
#include <cyg/io/pci.h>


static cyg_uint8
ixp425_read_config_byte(int offset)
{ 
    *IXP425_PCI_CRP_AD_CPE = (offset & ~3) | PCI_CRP_AD_CBE_READ;
    return *IXP425_PCI_CRP_RDATA >> (8*(offset & 3));
}

static cyg_uint16
ixp425_read_config_word(int offset)
{ 
    *IXP425_PCI_CRP_AD_CPE = (offset & ~3) | PCI_CRP_AD_CBE_READ;
    return *IXP425_PCI_CRP_RDATA >> (8*(offset & 3));
}

static cyg_uint32
ixp425_read_config_dword(int offset)
{ 
    *IXP425_PCI_CRP_AD_CPE = (offset & ~3) | PCI_CRP_AD_CBE_READ;
    return *IXP425_PCI_CRP_RDATA;
}

void
ixp425_write_config_byte(int offset, cyg_uint8 value)
{
    static const cyg_uint32 _cbe[4] = {
	(0xe << PCI_CRP_AD_CBE_BE_SHIFT) | PCI_CRP_AD_CBE_WRITE,
	(0xd << PCI_CRP_AD_CBE_BE_SHIFT) | PCI_CRP_AD_CBE_WRITE,
	(0xb << PCI_CRP_AD_CBE_BE_SHIFT) | PCI_CRP_AD_CBE_WRITE,
	(0x7 << PCI_CRP_AD_CBE_BE_SHIFT) | PCI_CRP_AD_CBE_WRITE
    };
    int pos = offset & 3;

    *IXP425_PCI_CRP_AD_CPE = (offset & ~3) | _cbe[pos];
    *IXP425_PCI_CRP_WDATA = value << (8 * pos);
}

void
ixp425_write_config_word(int offset, cyg_uint16 value)
{
    static const cyg_uint32 _cbe[4] = {
	(0xc << PCI_CRP_AD_CBE_BE_SHIFT) | PCI_CRP_AD_CBE_WRITE,
	(0x9 << PCI_CRP_AD_CBE_BE_SHIFT) | PCI_CRP_AD_CBE_WRITE,
	(0x3 << PCI_CRP_AD_CBE_BE_SHIFT) | PCI_CRP_AD_CBE_WRITE,
	(0x7 << PCI_CRP_AD_CBE_BE_SHIFT) | PCI_CRP_AD_CBE_WRITE
    };
    int pos = offset & 3;

    *IXP425_PCI_CRP_AD_CPE = (offset & ~3) | _cbe[pos];
    *IXP425_PCI_CRP_WDATA = value << (8 * pos);
}

void
ixp425_write_config_dword(int offset, cyg_uint32 value)
{
    *IXP425_PCI_CRP_AD_CPE = (offset & ~3) | PCI_CRP_AD_CBE_WRITE;
    *IXP425_PCI_CRP_WDATA = value;
}

// Setup address/cbe/cmd for read/write.
// Return CBE register value to use.
//
static cyg_uint32
pci_config(cyg_uint32 bus, cyg_uint32 devfn, cyg_uint32 offset, cyg_uint32 cmd, int size)
{
    static const cyg_uint8 byte_cbe[4] = { 0xe0, 0xd0, 0xb0, 0x70 };
    static const cyg_uint8 word_cbe[4] = { 0xc0, 0x90, 0x30, 0x70 };

    if (bus == 0) {
	// bus == 0 && devfn == 0 is a special case. Higher level config cycle
	// code will not call this routine for bus 0, devfn 0 as that is the
	// device address used by the IXP425 PCI device. When commands other
	// than config read/write are desired, bus 0, devfn 0 is used to
	// indicate that the offset should be used as the address.
	if (devfn == 0) {
	    *IXP425_PCI_NP_AD = offset;
	} else {
	    // type 0
	    //
	    // Take care with device number as we use device 0 to indicate
	    // the IXP425 itself. This code makes device 1-21 externally
	    // addressable.
	    //
	    *IXP425_PCI_NP_AD = (0x400 << CYG_PCI_DEV_GET_DEV(devfn)) |
	    (CYG_PCI_DEV_GET_FN(devfn)<< 8) | (offset & ~3);
	}
    } else {
	// type 1
	*IXP425_PCI_NP_AD = (bus << 16) | (CYG_PCI_DEV_GET_DEV(devfn) << 11) | 
	    (CYG_PCI_DEV_GET_FN(devfn) << 8) | (offset & ~3) | 1;
    }

    if (size == 1)
	cmd |= byte_cbe[offset & 3];
    else if (size == 2)
	cmd |= word_cbe[offset & 3];

    return cmd;
}

// check for abort after config access
static int
pci_check_abort(void)
{
    if (*IXP425_PCI_ISR & PCI_ISR_PFE) {
	*IXP425_PCI_ISR = PCI_ISR_PFE;
	return 1;
    }
    return 0;
}

static cyg_uint32
pci_np_read(cyg_uint32 cmd)
{
    cyg_uint32 data;
#ifdef CYGHWR_HAL_IXP425_PCI_NP_WORKAROUND
    // 
    // PCI NP Bug workaround  - only works if NP PCI space reads have
    // no side effects! Read 8 times. Last one will be good.
    //
    int i;

    for (i = 0; i < 8; i++) {
	// set up and execute the read
	*IXP425_PCI_NP_CBE = cmd;
	// the result of the read is now in NP_RDATA (maybe)
	data = *IXP425_PCI_NP_RDATA;
	// the result of the read is now in NP_RDATA (for sure)
	data = *IXP425_PCI_NP_RDATA;
    }
#else
    // set up and execute the read
    *IXP425_PCI_NP_CBE = cmd;
    // the result of the read is now in NP_RDATA
    data = *IXP425_PCI_NP_RDATA;
#endif
    if (pci_check_abort())
	return 0xffffffff;
    return data;
}

static void
pci_np_write(cyg_uint32 cmd, cyg_uint32 data)
{
    // set up the write
    *IXP425_PCI_NP_CBE = cmd;
    // write the date
    *IXP425_PCI_NP_WDATA = data;

    (void)pci_check_abort();
}

static cyg_uint8
bus_read_config_byte(cyg_uint32 bus, cyg_uint32 devfn, int offset)
{
    cyg_uint32 cmd = pci_config(bus, devfn, offset, PCI_NP_CMD_CONFIGR, 1);

    return pci_np_read(cmd) >> ((offset & 3) * 8);
}

static cyg_uint16
bus_read_config_word(cyg_uint32 bus, cyg_uint32 devfn, int offset)
{
    cyg_uint32 cmd = pci_config(bus, devfn, offset, PCI_NP_CMD_CONFIGR, 2);

    return pci_np_read(cmd) >> ((offset & 3) * 8);
}

static cyg_uint32
bus_read_config_dword(cyg_uint32 bus, cyg_uint32 devfn, int offset)
{
    cyg_uint32 cmd = pci_config(bus, devfn, offset, PCI_NP_CMD_CONFIGR, 4);

    return pci_np_read(cmd);
}

static void
bus_write_config_byte(cyg_uint32 bus, cyg_uint32 devfn, int offset, cyg_uint8 value)
{
    cyg_uint32 cmd = pci_config(bus, devfn, offset, PCI_NP_CMD_CONFIGW, 1);

    pci_np_write(cmd, value << ((offset & 3) * 8));
}

static void
bus_write_config_word(cyg_uint32 bus, cyg_uint32 devfn, int offset, cyg_uint16 value)
{
    cyg_uint32 cmd = pci_config(bus, devfn, offset, PCI_NP_CMD_CONFIGW, 2);

    pci_np_write(cmd, value << ((offset & 3) * 8));
}

static void
bus_write_config_dword(cyg_uint32 bus, cyg_uint32 devfn, int offset, cyg_uint32 value)
{
    cyg_uint32 cmd = pci_config(bus, devfn, offset, PCI_NP_CMD_CONFIGW, 4);

    pci_np_write(cmd, value);
}


cyg_uint32
cyg_hal_plf_pci_cfg_read_dword (cyg_uint32 bus, cyg_uint32 devfn, cyg_uint32 offset)
{
    if (bus || CYG_PCI_DEV_GET_DEV(devfn))
	return bus_read_config_dword(bus, devfn, offset);

    return ixp425_read_config_dword(offset);
}


void
cyg_hal_plf_pci_cfg_write_dword (cyg_uint32 bus,
				 cyg_uint32 devfn,
				 cyg_uint32 offset,
				 cyg_uint32 data)
{
    if (bus || CYG_PCI_DEV_GET_DEV(devfn))
	bus_write_config_dword(bus, devfn, offset, data);
    else
	ixp425_write_config_dword(offset, data);
}


cyg_uint16
cyg_hal_plf_pci_cfg_read_word (cyg_uint32 bus,
			       cyg_uint32 devfn,
			       cyg_uint32 offset)
{
    if (bus || CYG_PCI_DEV_GET_DEV(devfn))
	return bus_read_config_word(bus, devfn, offset);

    return ixp425_read_config_word(offset);
}

void
cyg_hal_plf_pci_cfg_write_word (cyg_uint32 bus,
				cyg_uint32 devfn,
				cyg_uint32 offset,
				cyg_uint16 data)
{
    if (bus || CYG_PCI_DEV_GET_DEV(devfn))
	bus_write_config_word(bus, devfn, offset, data);
    else
	ixp425_write_config_word(offset, data);
}

cyg_uint8
cyg_hal_plf_pci_cfg_read_byte (cyg_uint32 bus,
			       cyg_uint32 devfn,
			       cyg_uint32 offset)
{
    if (bus || CYG_PCI_DEV_GET_DEV(devfn))
	return bus_read_config_byte(bus, devfn, offset);

    return ixp425_read_config_byte(offset);
}


void
cyg_hal_plf_pci_cfg_write_byte (cyg_uint32 bus,
				cyg_uint32 devfn,
				cyg_uint32 offset,
				cyg_uint8 data)
{
    if (bus || CYG_PCI_DEV_GET_DEV(devfn))
	bus_write_config_byte(bus, devfn, offset, data);
    else
	ixp425_write_config_byte(offset, data);
}


void
cyg_hal_plf_pci_io_outb(cyg_uint32 offset, cyg_uint8 value)
{
    cyg_uint32 cmd = pci_config(0, 0, offset, PCI_NP_CMD_IOW, 1);

    pci_np_write(cmd, value << ((offset & 3) * 8));
}

void
cyg_hal_plf_pci_io_outw(cyg_uint32 offset, cyg_uint16 value)
{
    cyg_uint32 cmd = pci_config(0, 0, offset, PCI_NP_CMD_IOW, 2);

    pci_np_write(cmd, value << ((offset & 3) * 8));
}

void
cyg_hal_plf_pci_io_outl(cyg_uint32 offset, cyg_uint32 value)
{
    cyg_uint32 cmd = pci_config(0, 0, offset, PCI_NP_CMD_IOW, 4);

    pci_np_write(cmd, value);
}

cyg_uint8
cyg_hal_plf_pci_io_inb(cyg_uint32 offset)
{
    cyg_uint32 cmd = pci_config(0, 0, offset, PCI_NP_CMD_IOR, 1);

    return pci_np_read(cmd) >> ((offset & 3) * 8);
}

cyg_uint16
cyg_hal_plf_pci_io_inw(cyg_uint32 offset)
{
    cyg_uint32 cmd = pci_config(0, 0, offset, PCI_NP_CMD_IOR, 2);

    return pci_np_read(cmd) >> ((offset & 3) * 8);
}

cyg_uint32
cyg_hal_plf_pci_io_inl(cyg_uint32 offset)
{
    cyg_uint32 cmd = pci_config(0, 0, offset, PCI_NP_CMD_IOR, 4);

    return pci_np_read(cmd);
}

#ifdef CYGHWR_HAL_ARM_BIGENDIAN
#define CSR_ENDIAN_BITS  (PCI_CSR_PDS | PCI_CSR_ABE | PCI_CSR_ADS)
#else
#define CSR_ENDIAN_BITS  (PCI_CSR_ABE | PCI_CSR_ADS)
#endif

void
cyg_hal_plf_pci_init(void)
{  
    static int inited = 0;
    int  is_host = (*IXP425_PCI_CSR & PCI_CSR_HOST);

    if (inited)
	return;
    else
	inited = 1;
	    
    // If IC is set, assume warm start
    if (*IXP425_PCI_CSR  & PCI_CSR_IC)
	return;

    // We use identity AHB->PCI address translation
    // in the 0x48000000 address space
    *IXP425_PCI_PCIMEMBASE = 0x48494A4B;

    // We also use identity PCI->AHB address translation
    // in 4 16MB BARs that begin at the physical memory start
    *IXP425_PCI_AHBMEMBASE = 0x00010203;

    if (is_host) {

	HAL_PCI_CFG_WRITE_UINT32(0, 0, CYG_PCI_CFG_BAR_0, 0x00000000);
	HAL_PCI_CFG_WRITE_UINT32(0, 0, CYG_PCI_CFG_BAR_1, 0x01000000);
	HAL_PCI_CFG_WRITE_UINT32(0, 0, CYG_PCI_CFG_BAR_2, 0x02000000);
	HAL_PCI_CFG_WRITE_UINT32(0, 0, CYG_PCI_CFG_BAR_3, 0x03000000);

	cyg_pci_set_memory_base(HAL_PCI_ALLOC_BASE_MEMORY);
	cyg_pci_set_io_base(HAL_PCI_ALLOC_BASE_IO);

	// This one should never get used, as we request the memory for
	// work with PCI with GFP_DMA, which will return mem in the first 64 MB.
	// But we still must initialize it so that it wont intersect with first 4
	// BARs
	// XXX: Should we initialize the BAR5 to some very large value, so that
	// it also will not be hit?
	//
	HAL_PCI_CFG_WRITE_UINT32(0, 0, CYG_PCI_CFG_BAR_4, 0x80000000);
	HAL_PCI_CFG_WRITE_UINT32(0, 0, CYG_PCI_CFG_BAR_5, 0x90000000);

	*IXP425_PCI_ISR = PCI_ISR_PSE | PCI_ISR_PFE | PCI_ISR_PPE | PCI_ISR_AHBE;

	//
	// Set Initialize Complete in PCI Control Register: allow IXP425 to
	// respond to PCI configuration cycles. Specify that the AHB bus is
	// operating in big endian mode. Set up byte lane swapping between 
	// little-endian PCI and the big-endian AHB bus 
	*IXP425_PCI_CSR = PCI_CSR_IC | CSR_ENDIAN_BITS;
    
	HAL_PCI_CFG_WRITE_UINT16(0, 0, CYG_PCI_CFG_COMMAND,
		 CYG_PCI_CFG_COMMAND_MASTER | CYG_PCI_CFG_COMMAND_MEMORY);
    } else {
	//
	// Set Initialize Complete in PCI Control Register: allow IXP425 to
	// respond to PCI configuration cycles. Specify that the AHB bus is
	// operating in big endian mode. Set up byte lane swapping between 
	// little-endian PCI and the big-endian AHB bus 
	*IXP425_PCI_CSR = PCI_CSR_IC | PCI_CSR_ABE | PCI_CSR_PDS | PCI_CSR_ADS;
    }
}

#endif // CYGPKG_IO_PCI
