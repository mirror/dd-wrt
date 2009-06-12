//==========================================================================
//
//        pci1.c
//
//        Test PCI library API
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
// Author(s):     jskov
// Contributors:  jskov
// Date:          1999-03-17
// Description:   Simple test that:
//                  Checks API (compile time)
//                  Prints out information about found PCI devices.
//                  Allocates resources to devices (but does not enable
//                  them).
//####DESCRIPTIONEND####

#include <pkgconf/system.h>

#include <cyg/infra/diag.h>             // diag_printf
#include <cyg/infra/testcase.h>         // test macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

// Package requirements
#if defined(CYGPKG_IO_PCI) && defined(CYGPKG_KERNEL) && defined(CYGPKG_ISOINFRA)

#include <pkgconf/kernel.h>
#include <pkgconf/io_pci.h>
#include <cyg/io/pci.h>
#include <cyg/hal/hal_arch.h>
#include <string.h>

// Package option requirements
#if defined(CYGFUN_KERNEL_API_C) && defined(CYG_PCI_PRESENT)

#include <cyg/kernel/kapi.h>

// If the target has limited memory resources, undef the below to
// avoid inclusion of the big PCI code tables.
//
// The header file is created from http://www.yourvote.com/pci
// maintained by Jim Boemler (jboemler@halcyon.com).
//
// If you have PCI devices not listed in this list, please consider
// registering the codes in the database.
#define USE_PCI_CODE_LIST

#ifdef USE_PCI_CODE_LIST
#include "pcihdr.h"
#endif

// You may want to use this code to do some simple testing of the
// devices on the PCI bus. By enabling the below definition, the
// devices will get PCI IO and MEM access activated after configuration
// so you can play with IO registers and display/set contents of MEM.
#define nENABLE_PCI_DEVICES

unsigned char stack[CYGNUM_HAL_STACK_SIZE_TYPICAL];
cyg_thread thread_data;
cyg_handle_t thread_handle;

void pci_scan( void );

cyg_bool
pci_api_test(int dummy)
{
    cyg_pci_device dev_info;
    cyg_pci_device_id devid = CYG_PCI_NULL_DEVID;
    cyg_bool ret;
    CYG_PCI_ADDRESS64 mem_base = 0;
    CYG_PCI_ADDRESS32 io_base = 0;
    CYG_ADDRWORD vec;
    cyg_uint8 var_uint8;
    cyg_uint16 var_uint16;
    cyg_uint32 var_uint32;


    // Always return...
    if (dummy)
        return true;

    CYG_TEST_FAIL_FINISH("Not reached");

    // PCI library API
    cyg_pci_init();

    cyg_pci_get_device_info (devid, &dev_info);
    cyg_pci_set_device_info (devid, &dev_info);

    ret = cyg_pci_find_device(0, 0, &devid);
    ret |= cyg_pci_find_class(0, &devid);
    ret |= cyg_pci_find_next(devid, &devid);

    ret |= cyg_pci_configure_device(&dev_info);

    cyg_pci_set_memory_base(mem_base);
    cyg_pci_set_io_base(io_base);

    ret |= cyg_pci_allocate_memory(&dev_info, 0, &mem_base);
    ret |= cyg_pci_allocate_io(&dev_info, 0, &io_base);

    ret |= cyg_pci_translate_interrupt(&dev_info, &vec);

    cyg_pci_read_config_uint8(devid, 0, &var_uint8);
    cyg_pci_read_config_uint16(devid, 0, &var_uint16);
    cyg_pci_read_config_uint32(devid, 0, &var_uint32);

    cyg_pci_write_config_uint8(devid, 0, var_uint8);
    cyg_pci_write_config_uint16(devid, 0, var_uint16);
    cyg_pci_write_config_uint32(devid, 0, var_uint32);

    return ret;
}

void
pci_test( void )
{
    cyg_pci_device dev_info;
    cyg_pci_device_id devid;
    CYG_ADDRWORD irq;
    int i;
#ifdef USE_PCI_CODE_LIST
    cyg_bool no_match = false;
    cyg_uint16 vendor, device;
    cyg_uint8  bc, sc, pi;
    PCI_VENTABLE* vtbl;
    PCI_DEVTABLE* dtbl;
    PCI_CLASSCODETABLE* ctbl;
#endif

    pci_api_test(1);

    cyg_pci_init();

    diag_printf( "========== Finding and configuring devices\n" );

    if (cyg_pci_find_next(CYG_PCI_NULL_DEVID, &devid)) {
        do {
            // Get device info
            cyg_pci_get_device_info(devid, &dev_info);


            // Print stuff
            diag_printf("Found device on bus %d, devfn 0x%02x:\n",
                        CYG_PCI_DEV_GET_BUS(devid),
                        CYG_PCI_DEV_GET_DEVFN(devid));

            if (dev_info.command & CYG_PCI_CFG_COMMAND_ACTIVE) {
                diag_printf(" Note that board is active. Probed"
                            " sizes and CPU addresses invalid!\n");
            }

            // Configure the device
            if (cyg_pci_configure_device(&dev_info)) {
                diag_printf(" Device configuration succeeded\n");
#ifdef ENABLE_PCI_DEVICES
                {
                    cyg_uint16 cmd;

                    // Don't use cyg_pci_set_device_info since it clears
                    // some of the fields we want to print out below.
                    cyg_pci_read_config_uint16(dev_info.devid,
                                               CYG_PCI_CFG_COMMAND, &cmd);
                    cmd |= CYG_PCI_CFG_COMMAND_IO|CYG_PCI_CFG_COMMAND_MEMORY;
                    cyg_pci_write_config_uint16(dev_info.devid,
                                                CYG_PCI_CFG_COMMAND, cmd);
                }
                diag_printf(" **** Device IO and MEM access enabled\n");
#endif
            } else {
                diag_printf(" Device configuration failed");
                if (dev_info.command & CYG_PCI_CFG_COMMAND_ACTIVE)
                    diag_printf(" - device already enabled\n");
                else
                    diag_printf(" - resource problem\n");
            }

            diag_printf(" Vendor    0x%04x", dev_info.vendor);
#ifdef USE_PCI_CODE_LIST
            vendor = dev_info.vendor;
            vtbl = PciVenTable;
            for (i = 0; i < PCI_VENTABLE_LEN; i++, vtbl++)
                if (vendor == vtbl->VenId) 
                    break;

            if (i < PCI_VENTABLE_LEN) {
                diag_printf(" [%s][%s]", vtbl->VenShort, vtbl->VenFull);
            } else {
                diag_printf(" [UNKNOWN]");
                no_match = true;
            }
#endif
            diag_printf("\n Device    0x%04x", dev_info.device);
#ifdef USE_PCI_CODE_LIST
            device = dev_info.device;
            dtbl = PciDevTable;
            for (i = 0; i < PCI_DEVTABLE_LEN; i++, dtbl++)
                if (vendor == dtbl->VenId && device == dtbl->DevId) 
                    break;

            if (i < PCI_DEVTABLE_LEN) {
                diag_printf(" [%s][%s]", dtbl->Chip, dtbl->ChipDesc);
            } else {
                diag_printf(" [UNKNOWN]");
                no_match = true;
            }
#endif

            diag_printf("\n Command   0x%04x, Status 0x%04x\n",
                        dev_info.command, dev_info.status);

            diag_printf(" Class/Rev 0x%08x", dev_info.class_rev);
#ifdef USE_PCI_CODE_LIST
            bc = (dev_info.class_rev >> 24) & 0xff;
            sc = (dev_info.class_rev >> 16) & 0xff;
            pi = (dev_info.class_rev >>  8) & 0xff;
            ctbl = PciClassCodeTable;
            for (i = 0; i < PCI_CLASSCODETABLE_LEN; i++, ctbl++)
                if (bc == ctbl->BaseClass 
                    && sc == ctbl->SubClass
                    && pi == ctbl->ProgIf)
                    break;

            if (i < PCI_CLASSCODETABLE_LEN) {
                diag_printf(" [%s][%s][%s]", ctbl->BaseDesc,
                            ctbl->SubDesc, ctbl->ProgDesc);
            } else {
                diag_printf(" [UNKNOWN]");
                no_match = true;
            }
#endif
            diag_printf("\n Header 0x%02x\n", dev_info.header_type);

            diag_printf(" SubVendor 0x%04x, Sub ID 0x%04x\n",
                        dev_info.header.normal.sub_vendor, 
                        dev_info.header.normal.sub_id);


            for(i = 0; i < CYG_PCI_MAX_BAR; i++) {
                diag_printf(" BAR[%d]    0x%08x /", i, dev_info.base_address[i]);
                diag_printf(" probed size 0x%08x / CPU addr 0x%08x\n",
                            dev_info.base_size[i], dev_info.base_map[i]);
            }
            
            if (cyg_pci_translate_interrupt(&dev_info, &irq))
                diag_printf(" Wired to HAL vector %d\n", irq);
            else
                diag_printf(" Does not generate interrupts.\n");

        } while (cyg_pci_find_next(devid, &devid));


#ifdef USE_PCI_CODE_LIST
        diag_printf("\nStrings in [] are from the PCI Code List at http://www.yourvote.com/pci\n");
        if (no_match) {
            diag_printf("It seems that some of the device information has not been registered in\n");
            diag_printf("the PCI Code List. Please consider improving the database by registering\n");
            diag_printf("the [UNKNOWN] information for your devices. Thanks.\n");
        }
#endif
    } else {
        diag_printf("No PCI devices found.");
    }

    pci_scan();

    CYG_TEST_PASS_FINISH("pci1 test OK");
}

void
pci_scan( void )
{
    cyg_pci_device dev_info;
    cyg_pci_device_id devid;
    CYG_ADDRWORD irq;
    int i;
#ifdef USE_PCI_CODE_LIST
    cyg_bool no_match = false;
    cyg_uint16 vendor, device;
    cyg_uint8  bc, sc, pi;
    PCI_VENTABLE* vtbl;
    PCI_DEVTABLE* dtbl;
    PCI_CLASSCODETABLE* ctbl;
#endif

    diag_printf( "========== Scanning initialized devices\n" );

    if (cyg_pci_find_next(CYG_PCI_NULL_DEVID, &devid)) {
        do {
            // Since we are NOT about to configure the device, set the
            // devinfo record so we don't mistake garbage for data.
            memset( &dev_info, 0xAAAAAAAAu, sizeof( dev_info ) );

            // Get device info
            cyg_pci_get_device_info(devid, &dev_info);

            // Print stuff
            diag_printf("Found device on bus %d, devfn 0x%02x:\n",
                        CYG_PCI_DEV_GET_BUS(devid),
                        CYG_PCI_DEV_GET_DEVFN(devid));

            if (dev_info.command & CYG_PCI_CFG_COMMAND_ACTIVE) {
                diag_printf(" Note that board is active. Probed"
                            " sizes and CPU addresses invalid!\n");
            }

            diag_printf(" Vendor    0x%04x", dev_info.vendor);
#ifdef USE_PCI_CODE_LIST
            vendor = dev_info.vendor;
            vtbl = PciVenTable;
            for (i = 0; i < PCI_VENTABLE_LEN; i++, vtbl++)
                if (vendor == vtbl->VenId) 
                    break;

            if (i < PCI_VENTABLE_LEN) {
                diag_printf(" [%s][%s]", vtbl->VenShort, vtbl->VenFull);
            } else {
                diag_printf(" [UNKNOWN]");
                no_match = true;
            }
#endif
            diag_printf("\n Device    0x%04x", dev_info.device);
#ifdef USE_PCI_CODE_LIST
            device = dev_info.device;
            dtbl = PciDevTable;
            for (i = 0; i < PCI_DEVTABLE_LEN; i++, dtbl++)
                if (vendor == dtbl->VenId && device == dtbl->DevId) 
                    break;

            if (i < PCI_DEVTABLE_LEN) {
                diag_printf(" [%s][%s]", dtbl->Chip, dtbl->ChipDesc);
            } else {
                diag_printf(" [UNKNOWN]");
                no_match = true;
            }
#endif

            diag_printf("\n Command   0x%04x, Status 0x%04x\n",
                        dev_info.command, dev_info.status);

            diag_printf(" Class/Rev 0x%08x", dev_info.class_rev);
#ifdef USE_PCI_CODE_LIST
            bc = (dev_info.class_rev >> 24) & 0xff;
            sc = (dev_info.class_rev >> 16) & 0xff;
            pi = (dev_info.class_rev >>  8) & 0xff;
            ctbl = PciClassCodeTable;
            for (i = 0; i < PCI_CLASSCODETABLE_LEN; i++, ctbl++)
                if (bc == ctbl->BaseClass 
                    && sc == ctbl->SubClass
                    && pi == ctbl->ProgIf)
                    break;

            if (i < PCI_CLASSCODETABLE_LEN) {
                diag_printf(" [%s][%s][%s]", ctbl->BaseDesc,
                            ctbl->SubDesc, ctbl->ProgDesc);
            } else {
                diag_printf(" [UNKNOWN]");
                no_match = true;
            }
#endif
            diag_printf("\n Header 0x%02x\n", dev_info.header_type);

            diag_printf(" SubVendor 0x%04x, Sub ID 0x%04x\n",
                        dev_info.header.normal.sub_vendor, 
                        dev_info.header.normal.sub_id);


            for(i = 0; i < CYG_PCI_MAX_BAR; i++) {
                diag_printf(" BAR[%d]    0x%08x /", i, dev_info.base_address[i]);
                diag_printf(" probed size 0x%08x / CPU addr 0x%08x\n",
                            dev_info.base_size[i], dev_info.base_map[i]);
            }
            
            if (cyg_pci_translate_interrupt(&dev_info, &irq))
                diag_printf(" Wired to HAL vector %d\n", irq);
            else
                diag_printf(" Does not generate interrupts.\n");

        } while (cyg_pci_find_next(devid, &devid));
    } else {
        diag_printf("No PCI devices found.");
    }
}

void
cyg_start(void)
{
    CYG_TEST_INIT();
    cyg_thread_create(10,                   // Priority - just a number
                      (cyg_thread_entry_t*)pci_test,         // entry
                      0,                    // 
                      "pci_thread",     // Name
                      &stack[0],            // Stack
                      CYGNUM_HAL_STACK_SIZE_TYPICAL,           // Size
                      &thread_handle,       // Handle
                      &thread_data          // Thread data structure
        );
    cyg_thread_resume(thread_handle);
    cyg_scheduler_start();
}

#else // CYGFUN_KERNEL_API_C && CYG_PCI_PRESENT
#define N_A_MSG "Needs kernel C API & PCI platform support"
#endif

#else // CYGPKG_IO_PCI && CYGPKG_KERNEL
#define N_A_MSG "Needs IO/PCI, ISOINFRA, and Kernel"
#endif

#ifdef N_A_MSG
void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA( N_A_MSG);
}
#endif // N_A_MSG

// EOF pci1.c
