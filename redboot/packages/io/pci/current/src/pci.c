//=============================================================================
//
//      pci.c
//
//      PCI library
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2004 Red Hat, Inc.
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    jskov, from design by nickg
// Contributors: jskov
// Date:         1999-08-09
// Purpose:      PCI configuration
// Description: 
//               PCI bus support library.
//               Handles simple resource allocation for devices.
//               Can configure 64bit devices, but drivers may need special
//               magic to access all of this memory space - this is platform
//               specific and the driver must know how to handle it on its own.
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <pkgconf/io_pci.h>
#include <cyg/io/pci_hw.h>

// CYG_PCI_PRESENT only gets defined for targets that provide PCI HAL support.
// See pci_hw.h for details.
#ifdef CYG_PCI_PRESENT

#include <cyg/io/pci.h>
#include <cyg/infra/cyg_ass.h>

static cyg_bool cyg_pci_lib_initialized = false;
static CYG_PCI_ADDRESS64 cyg_pci_memory_base;
static CYG_PCI_ADDRESS32 cyg_pci_io_base;

void
cyg_pci_init( void )
{
    if (!cyg_pci_lib_initialized) {

	cyg_pci_set_memory_base(HAL_PCI_ALLOC_BASE_MEMORY);
	cyg_pci_set_io_base(HAL_PCI_ALLOC_BASE_IO);

        // Initialize the PCI bus, preparing it for general access.
        cyg_pcihw_init();

        cyg_pci_lib_initialized = true;
    }
}

//---------------------------------------------------------------------------
// Common device configuration access functions

void 
cyg_pci_get_device_info ( cyg_pci_device_id devid, cyg_pci_device *dev_info )
{
    int i;
    unsigned char bar_count;
    cyg_uint8 bus = CYG_PCI_DEV_GET_BUS(devid);
    cyg_uint8 devfn = CYG_PCI_DEV_GET_DEVFN(devid);
    cyg_uint8 header_type;

    dev_info->devid = devid;

    cyg_pcihw_read_config_uint16(bus, devfn, CYG_PCI_CFG_VENDOR,
                                 &dev_info->vendor);
    cyg_pcihw_read_config_uint16(bus, devfn, CYG_PCI_CFG_DEVICE,
                                 &dev_info->device);
    cyg_pcihw_read_config_uint16(bus, devfn, CYG_PCI_CFG_COMMAND,
                                 &dev_info->command);
    cyg_pcihw_read_config_uint16(bus, devfn, CYG_PCI_CFG_STATUS,
                                 &dev_info->status);
    cyg_pcihw_read_config_uint32(bus, devfn, CYG_PCI_CFG_CLASS_REV,
                                 &dev_info->class_rev);
    cyg_pcihw_read_config_uint8(bus, devfn, CYG_PCI_CFG_CACHE_LINE_SIZE,
                                &dev_info->cache_line_size);
    cyg_pcihw_read_config_uint8(bus, devfn, CYG_PCI_CFG_LATENCY_TIMER,
                                &dev_info->latency_timer);
    cyg_pcihw_read_config_uint8(bus, devfn, CYG_PCI_CFG_HEADER_TYPE,
                                &header_type);
    dev_info->header_type = (cyg_pci_header_type) header_type;
    cyg_pcihw_read_config_uint8(bus, devfn, CYG_PCI_CFG_BIST,
                                &dev_info->bist);

    if ((dev_info->header_type & CYG_PCI_CFG_HEADER_TYPE_MASK) == CYG_PCI_HEADER_BRIDGE)
	dev_info->num_bars = 2;
    else
	dev_info->num_bars = 6;

    // Clear out all address info (the code below stops short)
    for (i = 0; i < CYG_PCI_MAX_BAR; i++) {
        dev_info->base_map[i] = 0xffffffff;
        dev_info->base_address[i] = 0;
        dev_info->base_size[i] = 0;
    }

    for (i = 0; i < dev_info->num_bars; i++) {
        if (CYG_PCI_IGNORE_BAR(dev_info, i))
            continue;

        cyg_pcihw_read_config_uint32(bus, devfn,
                                     CYG_PCI_CFG_BAR_BASE + 4*i,
                                     &dev_info->base_address[i]);
    }

    // If device is disabled, probe BARs for sizes.
    if ((dev_info->command & CYG_PCI_CFG_COMMAND_ACTIVE) == 0) {

        bar_count = 0;
        for (i = 0; i < dev_info->num_bars; i++){
            cyg_uint32 size;

            if (CYG_PCI_IGNORE_BAR(dev_info, i))
                continue;

            cyg_pcihw_write_config_uint32(bus, devfn, 
                                          CYG_PCI_CFG_BAR_BASE + 4*i,
                                          0xffffffff);
            cyg_pcihw_read_config_uint32(bus, devfn, 
                                         CYG_PCI_CFG_BAR_BASE + 4*i,
                                         &size);
            dev_info->base_size[i] = size;
            dev_info->base_map[i] = 0xffffffff;

	    // Increment BAR count only if it has valid entry. BARs may not
            // be in contiguous locations. 
	    if (size != 0) {
                ++bar_count;
	    }

            // Check for a 64bit memory region.
            if (CYG_PCI_CFG_BAR_SPACE_MEM == 
                (size & CYG_PCI_CFG_BAR_SPACE_MASK)) {
                if (size & CYG_PRI_CFG_BAR_MEM_TYPE_64) {
                    // Clear fields for next BAR - it's the upper 32 bits.
                    i++;
                    dev_info->base_size[i] = 0;
                    dev_info->base_map[i] = 0xffffffff;
                }
            }

            // Fix any IO devices that only implement the bottom 16
            // bits of the the BAR. Just fill in these bits so it
            // looks like a full 32 bit BAR.
            if ((CYG_PCI_CFG_BAR_SPACE_IO == 
                 (size & CYG_PCI_CFG_BAR_SPACE_MASK)) &&
                ((size & 0xFFFF0000) == 0)
                )
            {
                dev_info->base_size[i] |= 0xFFFF0000;
            }
        }
        dev_info->num_bars = bar_count;
    } else {
	// If the device is already configured. Fill in the base_map.
	CYG_PCI_ADDRESS64 tmp_addr;
	cyg_uint32 bar;

        bar_count = 0;
        for (i = 0; i < dev_info->num_bars; i++){

	    dev_info->base_size[i] = 0;

            bar = dev_info->base_address[i];

	    // Increment BAR count only if it has valid entry. BARs may not
            // be in contiguous locations. 
	    if (bar != 0) {
                ++bar_count;
	    }

	    if ((bar & CYG_PCI_CFG_BAR_SPACE_MASK) == CYG_PCI_CFG_BAR_SPACE_IO) {
		dev_info->base_map[i] = (bar & CYG_PRI_CFG_BAR_IO_MASK) + HAL_PCI_PHYSICAL_IO_BASE;
		bar &= CYG_PRI_CFG_BAR_IO_MASK;
                if ((bar < 0xFF000000) && (bar >= cyg_pci_io_base)) {
                    // Assume that this device owns 512 'registers'
                    cyg_pci_io_base = bar + 0x200;
#ifdef CYGPKG_IO_PCI_DEBUG
                    diag_printf("Update I/O base to %x from bar: %d\n", cyg_pci_io_base, i);
#endif
                }
	    } else {
		tmp_addr = bar & CYG_PRI_CFG_BAR_MEM_MASK;

		if ((bar & CYG_PRI_CFG_BAR_MEM_TYPE_MASK) == CYG_PRI_CFG_BAR_MEM_TYPE_64)
		    tmp_addr |= ((CYG_PCI_ADDRESS64)(dev_info->base_address[i+1] & CYG_PRI_CFG_BAR_MEM_MASK)) << 32;

                if ((tmp_addr < 0xFF000000) && (tmp_addr >= cyg_pci_memory_base)) {
                    // Assume that this device owns 1M
                    cyg_pci_memory_base = tmp_addr + 0x100000;
#ifdef CYGPKG_IO_PCI_DEBUG
                    diag_printf("Update memory base to %llx from bar: %d\n", cyg_pci_memory_base, i);
#endif
                }

		tmp_addr += HAL_PCI_PHYSICAL_MEMORY_BASE;

		dev_info->base_map[i] = tmp_addr;
		if ((bar & CYG_PRI_CFG_BAR_MEM_TYPE_MASK) == CYG_PRI_CFG_BAR_MEM_TYPE_64)
		    dev_info->base_map[++i] = tmp_addr >> 32;
	    }
        }
        dev_info->num_bars = bar_count;
    }
            

    switch (dev_info->header_type & CYG_PCI_CFG_HEADER_TYPE_MASK) {
    case CYG_PCI_HEADER_NORMAL:
        cyg_pcihw_read_config_uint32(bus, devfn, CYG_PCI_CFG_CARDBUS_CIS,
                                     &dev_info->header.normal.cardbus_cis);
        cyg_pcihw_read_config_uint16(bus, devfn, CYG_PCI_CFG_SUB_VENDOR,
                                     &dev_info->header.normal.sub_vendor);
        cyg_pcihw_read_config_uint16(bus, devfn, CYG_PCI_CFG_SUB_ID,
                                     &dev_info->header.normal.sub_id);
        cyg_pcihw_read_config_uint32(bus, devfn, CYG_PCI_CFG_ROM_ADDRESS,
                                     &dev_info->header.normal.rom_address);
        cyg_pcihw_read_config_uint8(bus, devfn, CYG_PCI_CFG_CAP_LIST,
                                    &dev_info->header.normal.cap_list);

        cyg_pcihw_read_config_uint8(bus, devfn, CYG_PCI_CFG_INT_LINE,
                                    &dev_info->header.normal.int_line);
        cyg_pcihw_read_config_uint8(bus, devfn, CYG_PCI_CFG_INT_PIN,
                                    &dev_info->header.normal.int_pin);
        cyg_pcihw_read_config_uint8(bus, devfn, CYG_PCI_CFG_MIN_GNT,
                                    &dev_info->header.normal.min_gnt);
        cyg_pcihw_read_config_uint8(bus, devfn, CYG_PCI_CFG_MAX_LAT,
                                    &dev_info->header.normal.max_lat);
        break;
    case CYG_PCI_HEADER_BRIDGE:
        cyg_pcihw_read_config_uint8(bus, devfn, CYG_PCI_CFG_PRI_BUS,
                                    &dev_info->header.bridge.pri_bus);
        cyg_pcihw_read_config_uint8(bus, devfn, CYG_PCI_CFG_SEC_BUS,
                                    &dev_info->header.bridge.sec_bus);
        cyg_pcihw_read_config_uint8(bus, devfn, CYG_PCI_CFG_SUB_BUS,
                                    &dev_info->header.bridge.sub_bus);
        cyg_pcihw_read_config_uint8(bus, devfn, CYG_PCI_CFG_SEC_LATENCY_TIMER,
                                    &dev_info->header.bridge.sec_latency_timer);
        cyg_pcihw_read_config_uint8(bus, devfn, CYG_PCI_CFG_IO_BASE,
                                    &dev_info->header.bridge.io_base);
        cyg_pcihw_read_config_uint8(bus, devfn, CYG_PCI_CFG_IO_LIMIT,
                                    &dev_info->header.bridge.io_limit);
        cyg_pcihw_read_config_uint16(bus, devfn, CYG_PCI_CFG_SEC_STATUS,
                                    &dev_info->header.bridge.sec_status);
        cyg_pcihw_read_config_uint16(bus, devfn, CYG_PCI_CFG_MEM_BASE,
                                    &dev_info->header.bridge.mem_base);
        cyg_pcihw_read_config_uint16(bus, devfn, CYG_PCI_CFG_MEM_LIMIT,
                                    &dev_info->header.bridge.mem_limit);
        cyg_pcihw_read_config_uint16(bus, devfn, CYG_PCI_CFG_PREFETCH_BASE,
                                    &dev_info->header.bridge.prefetch_base);
        cyg_pcihw_read_config_uint16(bus, devfn, CYG_PCI_CFG_PREFETCH_LIMIT,
                                    &dev_info->header.bridge.prefetch_limit);
        cyg_pcihw_read_config_uint32(bus, devfn, CYG_PCI_CFG_PREFETCH_BASE_UPPER32,
                                    &dev_info->header.bridge.prefetch_base_upper32);
        cyg_pcihw_read_config_uint32(bus, devfn, CYG_PCI_CFG_PREFETCH_LIMIT_UPPER32,
                                    &dev_info->header.bridge.prefetch_limit_upper32);
        cyg_pcihw_read_config_uint16(bus, devfn, CYG_PCI_CFG_IO_BASE_UPPER16,
                                    &dev_info->header.bridge.io_base_upper16);
        cyg_pcihw_read_config_uint16(bus, devfn, CYG_PCI_CFG_IO_LIMIT_UPPER16,
                                    &dev_info->header.bridge.io_limit_upper16);
        cyg_pcihw_read_config_uint32(bus, devfn, CYG_PCI_CFG_BRIDGE_ROM_ADDRESS,
                                    &dev_info->header.bridge.rom_address);
        cyg_pcihw_read_config_uint8(bus, devfn, CYG_PCI_CFG_INT_LINE,
                                    &dev_info->header.bridge.int_line);
        cyg_pcihw_read_config_uint8(bus, devfn, CYG_PCI_CFG_INT_PIN,
                                    &dev_info->header.bridge.int_pin);
        cyg_pcihw_read_config_uint16(bus, devfn, CYG_PCI_CFG_BRIDGE_CONTROL,
                                    &dev_info->header.bridge.control);
        break;
    case CYG_PCI_HEADER_CARDBUS_BRIDGE:
        CYG_FAIL("PCI device header 'cardbus bridge' support not implemented");
        break;
    default:
        CYG_FAIL("Unknown PCI device header type");
        break;
    }
}

void 
cyg_pci_set_device_info ( cyg_pci_device_id devid, cyg_pci_device *dev_info )
{
    cyg_uint8 bus = CYG_PCI_DEV_GET_BUS(devid);
    cyg_uint8 devfn = CYG_PCI_DEV_GET_DEVFN(devid);
    int i;

    // Only writable entries are updated.
    cyg_pcihw_write_config_uint16(bus, devfn, CYG_PCI_CFG_COMMAND,
                                  dev_info->command);
    cyg_pcihw_write_config_uint16(bus, devfn, CYG_PCI_CFG_STATUS,
                                  dev_info->status);
    cyg_pcihw_write_config_uint8(bus, devfn, CYG_PCI_CFG_CACHE_LINE_SIZE,
                                 dev_info->cache_line_size);
    cyg_pcihw_write_config_uint8(bus, devfn, CYG_PCI_CFG_LATENCY_TIMER,
                                 dev_info->latency_timer);
    cyg_pcihw_write_config_uint8(bus, devfn, CYG_PCI_CFG_BIST,
                                 dev_info->bist);

    // Check for all possible BARs because they may be non-contiguous
    for (i = 0; i < CYG_PCI_MAX_BAR; i++) {
        if (dev_info->base_address[i]) {
	    cyg_pcihw_write_config_uint32 (bus, devfn, 
                                           CYG_PCI_CFG_BAR_BASE+4*i,
				           dev_info->base_address[i]);
        }
    }

    switch (dev_info->header_type & CYG_PCI_CFG_HEADER_TYPE_MASK) {
    case CYG_PCI_HEADER_NORMAL:
        cyg_pcihw_write_config_uint32(bus, devfn, CYG_PCI_CFG_CARDBUS_CIS,
                                      dev_info->header.normal.cardbus_cis);
        cyg_pcihw_write_config_uint16(bus, devfn, CYG_PCI_CFG_SUB_VENDOR,
                                      dev_info->header.normal.sub_vendor);
        cyg_pcihw_write_config_uint16(bus, devfn, CYG_PCI_CFG_SUB_ID,
                                      dev_info->header.normal.sub_id);
        cyg_pcihw_write_config_uint32(bus, devfn, CYG_PCI_CFG_ROM_ADDRESS,
                                      dev_info->header.normal.rom_address);

        cyg_pcihw_write_config_uint8(bus, devfn, CYG_PCI_CFG_INT_LINE,
                                     dev_info->header.normal.int_line);
        cyg_pcihw_write_config_uint8(bus, devfn, CYG_PCI_CFG_INT_PIN,
                                     dev_info->header.normal.int_pin);
        cyg_pcihw_write_config_uint8(bus, devfn, CYG_PCI_CFG_MIN_GNT,
                                     dev_info->header.normal.min_gnt);
        cyg_pcihw_write_config_uint8(bus, devfn, CYG_PCI_CFG_MAX_LAT,
                                     dev_info->header.normal.max_lat);
        break;
    case CYG_PCI_HEADER_BRIDGE:
        cyg_pcihw_write_config_uint8(bus, devfn, CYG_PCI_CFG_PRI_BUS,
				     dev_info->header.bridge.pri_bus);
        cyg_pcihw_write_config_uint8(bus, devfn, CYG_PCI_CFG_SEC_BUS,
				     dev_info->header.bridge.sec_bus);
        cyg_pcihw_write_config_uint8(bus, devfn, CYG_PCI_CFG_SUB_BUS,
				     dev_info->header.bridge.sub_bus);
        cyg_pcihw_write_config_uint8(bus, devfn, CYG_PCI_CFG_SEC_LATENCY_TIMER,
				     dev_info->header.bridge.sec_latency_timer);
        cyg_pcihw_write_config_uint8(bus, devfn, CYG_PCI_CFG_IO_BASE,
				     dev_info->header.bridge.io_base);
        cyg_pcihw_write_config_uint8(bus, devfn, CYG_PCI_CFG_IO_LIMIT,
				     dev_info->header.bridge.io_limit);
        cyg_pcihw_write_config_uint16(bus, devfn, CYG_PCI_CFG_SEC_STATUS,
				      dev_info->header.bridge.sec_status);
        cyg_pcihw_write_config_uint16(bus, devfn, CYG_PCI_CFG_MEM_BASE,
				      dev_info->header.bridge.mem_base);
        cyg_pcihw_write_config_uint16(bus, devfn, CYG_PCI_CFG_MEM_LIMIT,
				      dev_info->header.bridge.mem_limit);
        cyg_pcihw_write_config_uint16(bus, devfn, CYG_PCI_CFG_PREFETCH_BASE,
				      dev_info->header.bridge.prefetch_base);
        cyg_pcihw_write_config_uint16(bus, devfn, CYG_PCI_CFG_PREFETCH_LIMIT,
				      dev_info->header.bridge.prefetch_limit);
        cyg_pcihw_write_config_uint32(bus, devfn, CYG_PCI_CFG_PREFETCH_BASE_UPPER32,
				      dev_info->header.bridge.prefetch_base_upper32);
        cyg_pcihw_write_config_uint32(bus, devfn, CYG_PCI_CFG_PREFETCH_LIMIT_UPPER32,
				      dev_info->header.bridge.prefetch_limit_upper32);
        cyg_pcihw_write_config_uint16(bus, devfn, CYG_PCI_CFG_IO_BASE_UPPER16,
				      dev_info->header.bridge.io_base_upper16);
        cyg_pcihw_write_config_uint16(bus, devfn, CYG_PCI_CFG_IO_LIMIT_UPPER16,
				      dev_info->header.bridge.io_limit_upper16);
        cyg_pcihw_write_config_uint32(bus, devfn, CYG_PCI_CFG_BRIDGE_ROM_ADDRESS,
				      dev_info->header.bridge.rom_address);
        cyg_pcihw_write_config_uint8(bus, devfn, CYG_PCI_CFG_INT_LINE,
				     dev_info->header.bridge.int_line);
        cyg_pcihw_write_config_uint16(bus, devfn, CYG_PCI_CFG_BRIDGE_CONTROL,
				      dev_info->header.bridge.control);
        break;
    case CYG_PCI_HEADER_CARDBUS_BRIDGE:
        CYG_FAIL("PCI device header 'cardbus bridge' support not implemented");
        break;
    default:
        CYG_FAIL("Unknown PCI device header type");
        break;
    }

    // Update values in dev_info.
    cyg_pci_get_device_info(devid, dev_info);
}


//---------------------------------------------------------------------------
// Specific device configuration access functions
void 
cyg_pci_read_config_uint8( cyg_pci_device_id devid,
                           cyg_uint8 offset, cyg_uint8 *val)
{
    cyg_pcihw_read_config_uint8(CYG_PCI_DEV_GET_BUS(devid),
                                CYG_PCI_DEV_GET_DEVFN(devid),
                                offset, val);
}

void 
cyg_pci_read_config_uint16( cyg_pci_device_id devid,
                            cyg_uint8 offset, cyg_uint16 *val)
{
    cyg_pcihw_read_config_uint16(CYG_PCI_DEV_GET_BUS(devid),
                                 CYG_PCI_DEV_GET_DEVFN(devid),
                                 offset, val);
}

void
cyg_pci_read_config_uint32( cyg_pci_device_id devid,
                            cyg_uint8 offset, cyg_uint32 *val)
{
    cyg_pcihw_read_config_uint32(CYG_PCI_DEV_GET_BUS(devid),
                                 CYG_PCI_DEV_GET_DEVFN(devid),
                                 offset, val);
}


// Write functions
void
cyg_pci_write_config_uint8( cyg_pci_device_id devid,
                            cyg_uint8 offset, cyg_uint8 val)
{
    cyg_pcihw_write_config_uint8(CYG_PCI_DEV_GET_BUS(devid),
                                 CYG_PCI_DEV_GET_DEVFN(devid),
                                 offset, val);
}

void
cyg_pci_write_config_uint16( cyg_pci_device_id devid,
                             cyg_uint8 offset, cyg_uint16 val)
{
    cyg_pcihw_write_config_uint16(CYG_PCI_DEV_GET_BUS(devid),
                                  CYG_PCI_DEV_GET_DEVFN(devid),
                                  offset, val);
}

void
cyg_pci_write_config_uint32( cyg_pci_device_id devid,
                             cyg_uint8 offset, cyg_uint32 val)
{
    cyg_pcihw_write_config_uint32(CYG_PCI_DEV_GET_BUS(devid),
                                  CYG_PCI_DEV_GET_DEVFN(devid),
                                  offset, val);
}

//------------------------------------------------------------------------
// Device find functions

cyg_bool
cyg_pci_find_next( cyg_pci_device_id cur_devid, 
                   cyg_pci_device_id *next_devid )
{
    cyg_uint8 bus = CYG_PCI_DEV_GET_BUS(cur_devid);
    cyg_uint8 devfn = CYG_PCI_DEV_GET_DEVFN(cur_devid);
    cyg_uint8 dev = CYG_PCI_DEV_GET_DEV(devfn);
    cyg_uint8 fn = CYG_PCI_DEV_GET_FN(devfn);

#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("cyg_pci_find_next: start[%x] ...\n",(unsigned)cur_devid);
#endif

    // If this is the initializer, start with 0/0/0
    if (CYG_PCI_NULL_DEVID == cur_devid) {
        bus = dev = fn = 0;
        dev = CYG_PCI_MIN_DEV;
    } else if (CYG_PCI_NULL_DEVFN == (cur_devid & CYG_PCI_NULL_DEVFN)) {
        dev = fn = 0;
        dev = CYG_PCI_MIN_DEV;
    } else {
        // Otherwise, check multi-function bit of device's first function
        cyg_uint8 header;

	devfn = CYG_PCI_DEV_MAKE_DEVFN(dev, 0);
	cyg_pcihw_read_config_uint8(bus, devfn,
				    CYG_PCI_CFG_HEADER_TYPE, &header);
	if (header & CYG_PCI_CFG_HEADER_TYPE_MF) {
	    // Multi-function device. Increase fn.
	    fn++;
	    if (fn >= CYG_PCI_MAX_FN) {
		fn = 0;
		dev++;
	    }
	} else {
	    // Single-function device. Skip to next.
	    dev++;
	}
    }

    // Note: Reset iterators in enclosing statement's "next" part.
    //       Allows resuming scan with given input values. 
    for (;bus < CYG_PCI_MAX_BUS; bus++, dev=CYG_PCI_MIN_DEV) {
        for (;dev < CYG_PCI_MAX_DEV; dev++, fn=0) {
            for (;fn < CYG_PCI_MAX_FN; fn++) {
                cyg_uint16 vendor;

		if (CYG_PCI_IGNORE_DEVICE(bus, dev, fn))
		    continue;

                devfn = CYG_PCI_DEV_MAKE_DEVFN(dev, fn);
                cyg_pcihw_read_config_uint16(bus, devfn,
                                             CYG_PCI_CFG_VENDOR, &vendor);
                if (CYG_PCI_VENDOR_UNDEFINED != vendor) {
#ifdef CYGPKG_IO_PCI_DEBUG
                diag_printf("   Bus: %d, Dev: %d, Fn: %d, Vendor: %x\n", bus, dev, fn, vendor);
#endif
                    *next_devid = CYG_PCI_DEV_MAKE_ID(bus, devfn);
                    return true;
                }
            }
        }
    }

#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("nothing.\n");
#endif

    return false;
}

//
// Scan for a particular device, starting with 'devid'
// 'devid' is updated with the next device if found.
//         is not changed if no suitable device is found.
cyg_bool
cyg_pci_find_device( cyg_uint16 vendor, cyg_uint16 device,
                     cyg_pci_device_id *devid )
{
    cyg_pci_device_id new_devid = *devid;

#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("cyg_pci_find_device - vendor: %x, device: %x\n", vendor, device);
#endif
    // Scan entire bus, check for matches on valid devices.
    while (cyg_pci_find_next(new_devid, &new_devid)) {
        cyg_uint8 bus = CYG_PCI_DEV_GET_BUS(new_devid);
        cyg_uint8 devfn = CYG_PCI_DEV_GET_DEVFN(new_devid);
        cyg_uint16 v, d;

        // Check that vendor matches.
        cyg_pcihw_read_config_uint16(bus, devfn,
                                     CYG_PCI_CFG_VENDOR, &v);
        cyg_pcihw_read_config_uint16(bus, devfn,
                                     CYG_PCI_CFG_DEVICE, &d);
#ifdef CYGPKG_IO_PCI_DEBUG
        diag_printf("... PCI vendor = %x, device = %x\n", v, d);
#endif

        if (v != vendor) continue;

        // Check that device matches.
        if (d == device) {
#ifdef CYGPKG_IO_PCI_DEBUG
            diag_printf("Found it!\n");
#endif
            *devid = new_devid;
            return true;
        }
    }

    return false;
}

cyg_bool
cyg_pci_find_class( cyg_uint32 dev_class, cyg_pci_device_id *devid )
{
    // Scan entire bus, check for matches on valid devices.
    while (cyg_pci_find_next(*devid, devid)) {
        cyg_uint8 bus = CYG_PCI_DEV_GET_BUS(*devid);
        cyg_uint8 devfn = CYG_PCI_DEV_GET_DEVFN(*devid);
        cyg_uint32 c;

        // Check that class code matches.
        cyg_pcihw_read_config_uint32(bus, devfn,
                                     CYG_PCI_CFG_CLASS_REV, &c);
        c >>= 8;
        if (c == dev_class)
            return true;
    }

    return false;
}

cyg_bool
cyg_pci_find_matching( cyg_pci_match_func *matchp, 
                       void * match_callback_data,
                       cyg_pci_device_id *devid )
{
    cyg_pci_device_id new_devid = *devid;

#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("cyg_pci_find_matching - func is at %x\n", (unsigned int)matchp);
#endif
    // Scan entire bus, check for matches on valid devices.
    while (cyg_pci_find_next(new_devid, &new_devid)) {
        cyg_uint8 bus = CYG_PCI_DEV_GET_BUS(new_devid);
        cyg_uint8 devfn = CYG_PCI_DEV_GET_DEVFN(new_devid);
        cyg_uint16 v, d;
        cyg_uint32 c;

        // Check that vendor, device and class match.
        cyg_pcihw_read_config_uint16(bus, devfn,
                                     CYG_PCI_CFG_VENDOR, &v);
        cyg_pcihw_read_config_uint16(bus, devfn,
                                     CYG_PCI_CFG_DEVICE, &d);
        cyg_pcihw_read_config_uint32(bus, devfn,
                                     CYG_PCI_CFG_CLASS_REV, &c);
        c >>= 8;
#ifdef CYGPKG_IO_PCI_DEBUG
        diag_printf("... PCI vendor = %x, device = %x, class %x\n", v, d, c);
#endif
        // Check that device matches as the caller desires:
        if ( (*matchp)(v, d, c, match_callback_data) ) {
            *devid = new_devid;
            return true;
        }
    }

    return false;
}

//------------------------------------------------------------------------
// Resource Allocation

void
cyg_pci_set_memory_base(CYG_PCI_ADDRESS64 base)
{
    cyg_pci_memory_base = base;
}

void
cyg_pci_set_io_base(CYG_PCI_ADDRESS32 base)
{
    cyg_pci_io_base = base;
}

cyg_bool
cyg_pci_configure_device( cyg_pci_device *dev_info )
{
    int bar;
    cyg_uint32 flags;
    cyg_bool ret = true;

    // If device is already active, just return true as
    // cyg_pci_get_device_info has presumably filled in
    // the base_map already.
    if ((dev_info->command & CYG_PCI_CFG_COMMAND_ACTIVE) != 0)
	return true;

    if (dev_info->num_bars > 0) {
        for (bar = 0; bar < CYG_PCI_MAX_BAR; bar++) {
            if (!dev_info->base_address[bar]) {
                continue;
            }
	    flags = dev_info->base_size[bar];

	    ret = false;

	    if ((flags & CYG_PCI_CFG_BAR_SPACE_MASK) == CYG_PCI_CFG_BAR_SPACE_MEM){
		ret |= cyg_pci_allocate_memory(dev_info, bar, 
					       &cyg_pci_memory_base);

		// If this is a 64bit memory region, skip the next bar
		// since it will contain the top 32 bits.
		if (flags & CYG_PRI_CFG_BAR_MEM_TYPE_64)
		    bar++;
	    } else
		ret |= cyg_pci_allocate_io(dev_info, bar, &cyg_pci_io_base);

	    if (!ret)
		return ret;
	}
    }

    cyg_pci_translate_interrupt(dev_info, &dev_info->hal_vector);

    return ret;
}

// This is the function that handles resource allocation. It doesn't
// affect the device state.
// Should not be called with top32bit-bar of a 64bit pair.
inline cyg_bool
cyg_pci_allocate_memory_priv( cyg_pci_device *dev_info, cyg_uint32 bar,
                              CYG_PCI_ADDRESS64 *base, 
                              CYG_PCI_ADDRESS64 *assigned_addr)
{
    cyg_uint32 mem_type, flags;
    CYG_PCI_ADDRESS64 size, aligned_addr;

    // Get the probed size and flags
    flags = dev_info->base_size[bar];

    // Decode size
    size = (CYG_PCI_ADDRESS64) ((~(flags & CYG_PRI_CFG_BAR_MEM_MASK))+1);

    // Calculate address we will assign the device.
    // This can be made more clever, specifically:
    //  1) The lowest 1MB should be reserved for devices with 1M memory type.
    //     : Needs to be handled.
    //  2) The low 32bit space should be reserved for devices with 32bit type.
    //     : With the usual handful of devices it is unlikely that the
    //       low 4GB space will become full.
    //  3) A bitmap can be used to avoid fragmentation.
    //     : Again, unlikely to be necessary.
    //
    // For now, simply align to required size.
    aligned_addr = (*base+size-1) & ~(size-1);

    // Is the request for memory space?
    if (CYG_PCI_CFG_BAR_SPACE_MEM != (flags & CYG_PCI_CFG_BAR_SPACE_MASK))
        return false;

    // Check type of memory requested...
    mem_type = CYG_PRI_CFG_BAR_MEM_TYPE_MASK & flags;

    // We don't handle <1MB devices optimally yet.
    if (CYG_PRI_CFG_BAR_MEM_TYPE_1M == mem_type
        && (aligned_addr + size) > 1024*1024)
        return false;

    // Update the resource pointer and return values.
    *base = aligned_addr+size;
    *assigned_addr = aligned_addr;

    dev_info->base_map[bar] = (cyg_uint32) 
        (aligned_addr+HAL_PCI_PHYSICAL_MEMORY_BASE) & 0xffffffff;

    // If a 64bit region, store upper 32 bits in the next bar.
    // Note: The CPU is not necessarily able to access the region
    // linearly - it may have to do it in segments. Driver must handle that.
    if (CYG_PRI_CFG_BAR_MEM_TYPE_64 == mem_type) {
        dev_info->base_map[bar+1] = (cyg_uint32) 
            ((aligned_addr+HAL_PCI_PHYSICAL_MEMORY_BASE) >> 32) & 0xffffffff;
    }
    
    return true;
}

cyg_bool
cyg_pci_allocate_memory( cyg_pci_device *dev_info, cyg_uint32 bar,
                         CYG_PCI_ADDRESS64 *base)
{
    cyg_uint8 bus = CYG_PCI_DEV_GET_BUS(dev_info->devid);
    cyg_uint8 devfn = CYG_PCI_DEV_GET_DEVFN(dev_info->devid);
    CYG_PCI_ADDRESS64 assigned_addr;
    cyg_bool ret;

    // Check that device is inactive.
    if ((dev_info->command & CYG_PCI_CFG_COMMAND_ACTIVE) != 0)
        return false;

    // Allocate memory space for the device.
    ret = cyg_pci_allocate_memory_priv(dev_info, bar, base, &assigned_addr);

    if (ret) {
        // Map the device and update the BAR in the dev_info structure.
        cyg_pcihw_write_config_uint32(bus, devfn,
                                      CYG_PCI_CFG_BAR_BASE+4*bar, 
                                      (cyg_uint32) 
                                      (assigned_addr & 0xffffffff));
        cyg_pcihw_read_config_uint32(bus, devfn,
                                     CYG_PCI_CFG_BAR_BASE+4*bar, 
                                     &dev_info->base_address[bar]);

        // Handle upper 32 bits if necessary.
        if (dev_info->base_size[bar] & CYG_PRI_CFG_BAR_MEM_TYPE_64) {
            cyg_pcihw_write_config_uint32(bus, devfn,
                                          CYG_PCI_CFG_BAR_BASE+4*(bar+1), 
                                          (cyg_uint32) 
                                          ((assigned_addr >> 32)& 0xffffffff));
            cyg_pcihw_read_config_uint32(bus, devfn,
                                         CYG_PCI_CFG_BAR_BASE+4*(bar+1), 
                                         &dev_info->base_address[bar+1]);
        }
    }

    return ret;
}    

cyg_bool
cyg_pci_allocate_io_priv( cyg_pci_device *dev_info, cyg_uint32 bar, 
                          CYG_PCI_ADDRESS32 *base, 
                          CYG_PCI_ADDRESS32 *assigned_addr)
{
    cyg_uint32 flags, size;
    CYG_PCI_ADDRESS32 aligned_addr;

    // Get the probed size and flags
    flags = dev_info->base_size[bar];

    // Decode size
    size = (~(flags & CYG_PRI_CFG_BAR_IO_MASK))+1;

    // Calculate address we will assign the device.
    // This can be made more clever.
    // For now, simply align to required size.
    aligned_addr = (*base+size-1) & ~(size-1);

    // Is the request for IO space?
    if (CYG_PCI_CFG_BAR_SPACE_IO != (flags & CYG_PCI_CFG_BAR_SPACE_MASK))
        return false;

    // Update the resource pointer and return values.
    *base = aligned_addr+size;
    dev_info->base_map[bar] = aligned_addr+HAL_PCI_PHYSICAL_IO_BASE;
    *assigned_addr = aligned_addr;

    return true;
}


cyg_bool
cyg_pci_allocate_io( cyg_pci_device *dev_info, cyg_uint32 bar, 
                     CYG_PCI_ADDRESS32 *base)
{
    cyg_uint8 bus = CYG_PCI_DEV_GET_BUS(dev_info->devid);
    cyg_uint8 devfn = CYG_PCI_DEV_GET_DEVFN(dev_info->devid);
    CYG_PCI_ADDRESS32 assigned_addr;
    cyg_bool ret;
    
    // Check that device is inactive.
    if ((dev_info->command & CYG_PCI_CFG_COMMAND_ACTIVE) != 0)
        return false;

    // Allocate IO space for the device.
    ret = cyg_pci_allocate_io_priv(dev_info, bar, base, &assigned_addr);

    if (ret) {
        // Map the device and update the BAR in the dev_info structure.
        cyg_pcihw_write_config_uint32(bus, devfn,
                                      CYG_PCI_CFG_BAR_BASE+4*bar, 
                                      assigned_addr);
        cyg_pcihw_read_config_uint32(bus, devfn,
                                     CYG_PCI_CFG_BAR_BASE+4*bar, 
                                     &dev_info->base_address[bar]);
    }

    return ret;
}

cyg_bool
cyg_pci_translate_interrupt( cyg_pci_device *dev_info,
                             CYG_ADDRWORD *vec )
{
    cyg_uint8 bus = CYG_PCI_DEV_GET_BUS(dev_info->devid);
    cyg_uint8 devfn = CYG_PCI_DEV_GET_DEVFN(dev_info->devid);

    if (cyg_pcihw_translate_interrupt(bus, devfn, vec)) {
        // Fill in interrupt line info. This only really works for
        // platforms where assigned PCI irq numbers are less than 255.
        cyg_pcihw_write_config_uint8(bus, devfn,
                                     CYG_PCI_CFG_INT_LINE, 
                                     *vec & 0xff);
        return true;
    }
    return false;
}


// Initialize devices on a given bus and all subordinate busses.
cyg_bool
cyg_pci_configure_bus( cyg_uint8 bus,
		       cyg_uint8 *next_bus )
{
    cyg_uint8 devfn, header_type;
    cyg_pci_device_id devid;
    cyg_pci_device dev_info;

    CYG_PCI_ADDRESS64 mem_start, mem_limit, mem_base;
    CYG_PCI_ADDRESS32 io_start, io_limit, io_base;

    // Scan only this bus for valid devices.
    devid = CYG_PCI_DEV_MAKE_ID(bus, 0) | CYG_PCI_NULL_DEVFN;

#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("Configuring bus %d.\n", bus);
#endif

    while (cyg_pci_find_next(devid, &devid) && bus == CYG_PCI_DEV_GET_BUS(devid)) {

        devfn = CYG_PCI_DEV_GET_DEVFN(devid);
	
	// Get the device info
	cyg_pci_get_device_info(devid, &dev_info);

#ifdef CYGPKG_IO_PCI_DEBUG
	diag_printf("\n");
	diag_printf("Configuring PCI Bus   : %d\n", bus);
	diag_printf("            PCI Device: %d\n", CYG_PCI_DEV_GET_DEV(devfn));
	diag_printf("            PCI Func  : %d\n", CYG_PCI_DEV_GET_FN(devfn));
	diag_printf("            Vendor Id : 0x%08X\n", dev_info.vendor);
	diag_printf("            Device Id : 0x%08X\n", dev_info.device);
#endif

	header_type = dev_info.header_type & CYG_PCI_CFG_HEADER_TYPE_MASK;

	// Check for supported header types.
	if (header_type != CYG_PCI_HEADER_NORMAL &&
	    header_type != CYG_PCI_HEADER_BRIDGE) {
	    CYG_FAIL("Unsupported PCI header type");
	    continue;
	}

	// Only PCI-to-PCI bridges
	if (header_type == CYG_PCI_HEADER_BRIDGE &&
	    (dev_info.class_rev >> 8) != CYG_PCI_CLASS_BRIDGE_PCI_PCI) {
	    CYG_FAIL("Unsupported PCI bridge class");
	    continue;
	}

	// Configure the base registers
	if (!cyg_pci_configure_device(&dev_info)) {
	    // Apparently out of resources.
	    CYG_FAIL("cyg_pci_configure_device failed");
	    break;
	}
	
	// Activate non-bridge devices.
	if (header_type != CYG_PCI_HEADER_BRIDGE) {
	    dev_info.command |= (CYG_PCI_CFG_COMMAND_IO 	// enable I/O space
		              | CYG_PCI_CFG_COMMAND_MEMORY	// enable memory space
		              | CYG_PCI_CFG_COMMAND_MASTER); 	// enable bus master
	    cyg_pci_write_config_uint16(dev_info.devid, CYG_PCI_CFG_COMMAND, dev_info.command);
	} else {
	    //  Bridge Configuration

	    // Set up the bus numbers that define the bridge
	    dev_info.header.bridge.pri_bus = bus;
	    cyg_pcihw_write_config_uint8(bus, devfn, CYG_PCI_CFG_PRI_BUS,
					 dev_info.header.bridge.pri_bus);

	    dev_info.header.bridge.sec_bus = *next_bus;
	    cyg_pcihw_write_config_uint8(bus, devfn, CYG_PCI_CFG_SEC_BUS,
					 dev_info.header.bridge.sec_bus);

	    // Temporarily set to maximum so config cycles get passed along.
	    dev_info.header.bridge.sub_bus = CYG_PCI_MAX_BUS - 1;
	    cyg_pcihw_write_config_uint8(bus, devfn, CYG_PCI_CFG_SUB_BUS,
					 dev_info.header.bridge.sub_bus);

	    // increment bus counter
	    *next_bus += 1;

	    // To figure the sizes of the memory and I/O windows, save the
	    // current base of memory and I/O before configuring the bus
	    // or busses on the secondary side of the bridge. After the
	    // secondary side is configured, the difference between the
	    // current values and saved values will tell the size.

	    // For bridges, the memory window must start and end on a 1M
	    // boundary and the I/O window must start and end on a 4K
	    // boundary. We round up the mem and I/O allocation bases
	    // to appropriate boundaries before configuring the secondary
	    // bus. Save the pre-rounded values in case no mem or I/O
	    // is needed we can recover any space lost due to rounding.

	    // round up start of PCI memory space to a 1M boundary
	    mem_base = cyg_pci_memory_base;
	    cyg_pci_memory_base += 0xfffff;
	    cyg_pci_memory_base &= ~0xfffff;
	    mem_start = cyg_pci_memory_base;

	    // round up start of PCI I/O space to a 4 Kbyte start address
	    io_base = cyg_pci_io_base;
	    cyg_pci_io_base += 0xfff;
	    cyg_pci_io_base &= ~0xfff;
	    io_start = cyg_pci_io_base;
		
	    // configure the subordinate PCI bus
	    cyg_pci_configure_bus (dev_info.header.bridge.sec_bus, next_bus);
			
	    // set subordinate bus number to last assigned bus number
	    dev_info.header.bridge.sub_bus = *next_bus - 1;
	    cyg_pcihw_write_config_uint8(bus, devfn, CYG_PCI_CFG_SUB_BUS,
					 dev_info.header.bridge.sub_bus);

	    // Did sub bus configuration use any I/O?
	    if (cyg_pci_io_base > io_start) {

		// round up end of bridge's I/O space to a 4K boundary
		cyg_pci_io_base += 0xfff;
		cyg_pci_io_base &= ~0xfff;
		io_limit = cyg_pci_io_base - 0x1000;

		// Enable I/O cycles across bridge
		dev_info.command |= CYG_PCI_CFG_COMMAND_IO;

		// 32 Bit I/O?
		if ((dev_info.header.bridge.io_base & 0x0f) == 0x01) {
		    // I/O Base Upper 16 Bits Register
		    dev_info.header.bridge.io_base_upper16 = io_start >> 16;
		    cyg_pcihw_write_config_uint16(bus, devfn, CYG_PCI_CFG_IO_BASE_UPPER16,
						 dev_info.header.bridge.io_base_upper16);
		    // I/O Limit Upper 16 Bits Register
		    dev_info.header.bridge.io_limit_upper16 = io_limit >> 16;
		    cyg_pcihw_write_config_uint16(bus, devfn, CYG_PCI_CFG_IO_LIMIT_UPPER16,
						 dev_info.header.bridge.io_limit_upper16);
		}

		// I/O Base Register
		dev_info.header.bridge.io_base = (io_start & 0xf000) >> 8;
		cyg_pcihw_write_config_uint8(bus, devfn, CYG_PCI_CFG_IO_BASE,
					     dev_info.header.bridge.io_base);
		// I/O Limit Register
		dev_info.header.bridge.io_limit = (io_limit & 0xf000) >> 8;
		cyg_pcihw_write_config_uint8(bus, devfn, CYG_PCI_CFG_IO_LIMIT,
					     dev_info.header.bridge.io_limit);

	    } else {
		// No I/O space used on secondary bus.
		// Recover any space lost on unnecessary rounding
		cyg_pci_io_base = io_base;
	    }

	    // Did sub bus configuration use any memory?
	    if (cyg_pci_memory_base > mem_start) {

		// round up end of bridge's PCI memory space to a 1M boundary
		cyg_pci_memory_base += 0xfffff;
		cyg_pci_memory_base &= ~0xfffff;
		mem_limit = cyg_pci_memory_base - 0x100000;

		// Enable memory cycles across bridge
		dev_info.command |= CYG_PCI_CFG_COMMAND_MEMORY;

		// Memory Base Register
		dev_info.header.bridge.mem_base = (mem_start >> 16) & 0xfff0;
		cyg_pcihw_write_config_uint16(bus, devfn, CYG_PCI_CFG_MEM_BASE,
						 dev_info.header.bridge.mem_base);

		// Memory Limit Register
		dev_info.header.bridge.mem_limit = (mem_limit >> 16) & 0xfff0;
		cyg_pcihw_write_config_uint16(bus, devfn, CYG_PCI_CFG_MEM_LIMIT,
						 dev_info.header.bridge.mem_limit);

		// Prefetchable memory not yet supported across bridges.
		// Disable by making limit < base
		{
		    cyg_uint16 tmp_word;

		    tmp_word = 0;
		    cyg_pcihw_write_config_uint16(bus, devfn, CYG_PCI_CFG_PREFETCH_LIMIT,
						 tmp_word);
		    tmp_word = 0xfff0;
		    cyg_pcihw_write_config_uint16(bus, devfn, CYG_PCI_CFG_PREFETCH_BASE,
						 tmp_word);
		}
	    } else {
		// No memory space used on secondary bus.
		// Recover any space lost on unnecessary rounding
		cyg_pci_memory_base = mem_base;
	    }

	    // Setup the bridge command register
	    dev_info.command |= CYG_PCI_CFG_COMMAND_MASTER;
	    dev_info.command |= CYG_PCI_CFG_COMMAND_SERR;
	    cyg_pcihw_write_config_uint16(bus, devfn, CYG_PCI_CFG_COMMAND,
					  dev_info.command);

	    /* Setup the Bridge Control Register */
	    dev_info.header.bridge.control |= CYG_PCI_CFG_BRIDGE_CTL_PARITY;
	    dev_info.header.bridge.control |= CYG_PCI_CFG_BRIDGE_CTL_SERR;
	    dev_info.header.bridge.control |= CYG_PCI_CFG_BRIDGE_CTL_MASTER;
	    cyg_pcihw_write_config_uint16(bus, devfn, CYG_PCI_CFG_BRIDGE_CONTROL,
					  dev_info.header.bridge.control);
	}
    }
#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("Finished configuring bus %d.\n", bus);
#endif

    return true;
}


#endif // ifdef CYG_PCI_PRESENT

//-----------------------------------------------------------------------------
// end of pci.c
