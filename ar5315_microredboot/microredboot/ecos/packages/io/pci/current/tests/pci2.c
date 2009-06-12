//==========================================================================
//
//        pci2.c
//
//        Test PCI library's resource allocation.
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
// Description: Simple test that prints out information about found
//              PCI devices. Entirely passive - no configuration happens.
// 
//####DESCRIPTIONEND####

#include <pkgconf/system.h>

#include <cyg/infra/diag.h>             // diag_printf
#include <cyg/infra/testcase.h>         // test macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

// Package requirements
#if defined(CYGPKG_IO_PCI) && defined(CYGPKG_KERNEL)

#include <pkgconf/kernel.h>
#include <pkgconf/io_pci.h>
#include <cyg/io/pci.h>
#include <cyg/hal/hal_arch.h>

// Package option requirements
#if defined(CYGFUN_KERNEL_API_C) && defined(CYG_PCI_PRESENT)

#include <cyg/kernel/kapi.h>

unsigned char stack[CYGNUM_HAL_STACK_SIZE_TYPICAL];
cyg_thread thread_data;
cyg_handle_t thread_handle;

void
pci_test( void )
{
    cyg_pci_device dev_info;
    CYG_PCI_ADDRESS64 mem_base;
    CYG_PCI_ADDRESS64 mem_assigned_addr;
    CYG_PCI_ADDRESS32 io_base;
    CYG_PCI_ADDRESS32 io_assigned_addr;

    // Verify that space type requests are checked.
    dev_info.base_size[0] = CYG_PCI_CFG_BAR_SPACE_IO;
    CYG_TEST_CHECK(!cyg_pci_allocate_memory_priv(&dev_info, 0, &mem_base, 
                                                 &mem_assigned_addr),
                   "cyg_pci_allocate_memory_priv accepted IO request");
    dev_info.base_size[0] = CYG_PCI_CFG_BAR_SPACE_MEM;
    CYG_TEST_CHECK(!cyg_pci_allocate_io_priv(&dev_info, 0, &io_base, 
                                             &io_assigned_addr),
                   "cyg_pci_allocate_io_priv accepted MEM request");

    // Check allocation and alignment of IO space
    io_base = 0x0000;
    // Size 4
    dev_info.base_size[0] = CYG_PCI_CFG_BAR_SPACE_IO | 0xfffffffc;
    CYG_TEST_CHECK(cyg_pci_allocate_io_priv(&dev_info, 0, &io_base, 
                                            &io_assigned_addr),
                   "cyg_pci_allocate_io_priv(4) failed");
    CYG_TEST_CHECK(0x0004 == io_base, 
                   "cyg_pci_allocate_io_priv(4) size failed");
    CYG_TEST_CHECK(0x0000 == io_assigned_addr,
                   "cyg_pci_allocate_io_priv(4) align failed");
    // Size 8 (alignment required)
    dev_info.base_size[0] = CYG_PCI_CFG_BAR_SPACE_IO | 0xfffffff8;
    CYG_TEST_CHECK(cyg_pci_allocate_io_priv(&dev_info, 0, &io_base, 
                                            &io_assigned_addr),
                   "cyg_pci_allocate_io_priv(8) failed");
    CYG_TEST_CHECK(0x0010 == io_base, 
                   "cyg_pci_allocate_io_priv(8) size failed");
    CYG_TEST_CHECK(0x0008 == io_assigned_addr,
                   "cyg_pci_allocate_io_priv(8) align failed");
    // Size 16 (no alignment required)
    dev_info.base_size[0] = CYG_PCI_CFG_BAR_SPACE_IO | 0xfffffff0;
    CYG_TEST_CHECK(cyg_pci_allocate_io_priv(&dev_info, 0, &io_base, 
                                            &io_assigned_addr),
                   "cyg_pci_allocate_io_priv(16) failed");
    CYG_TEST_CHECK(0x0020 == io_base, 
                   "cyg_pci_allocate_io_priv(16) size failed");
    CYG_TEST_CHECK(0x0010 == io_assigned_addr,
                   "cyg_pci_allocate_io_priv(16) align failed");
    // Size 64 (alignment required)
    dev_info.base_size[0] = CYG_PCI_CFG_BAR_SPACE_IO | 0xffffffc0;
    CYG_TEST_CHECK(cyg_pci_allocate_io_priv(&dev_info, 0, &io_base,
                                            &io_assigned_addr),
                   "cyg_pci_allocate_io_priv(64) failed");
    CYG_TEST_CHECK(0x0080 == io_base,
                   "cyg_pci_allocate_io_priv(64) size failed");
    CYG_TEST_CHECK(0x0040 == io_assigned_addr,
                   "cyg_pci_allocate_io_priv(64) align failed");
    // Size 256 (alignment required)
    dev_info.base_size[0] = CYG_PCI_CFG_BAR_SPACE_IO | 0xffffff00;
    CYG_TEST_CHECK(cyg_pci_allocate_io_priv(&dev_info, 0, &io_base,
                                            &io_assigned_addr),
                   "cyg_pci_allocate_io_priv(256) failed");
    CYG_TEST_CHECK(0x0200 == io_base, 
                   "cyg_pci_allocate_io_priv(256) size failed");
    CYG_TEST_CHECK(0x0100 == io_assigned_addr,
                   "cyg_pci_allocate_io_priv(256) align failed");

    // Check allocation and alignment of MEM space
    mem_base = 0x00000000;
    // Size 16 (no alignment required)
    dev_info.base_size[0] = CYG_PCI_CFG_BAR_SPACE_MEM | 0xfffffff0;
    CYG_TEST_CHECK(cyg_pci_allocate_memory_priv(&dev_info, 0, &mem_base, 
                                                &mem_assigned_addr),
                   "cyg_pci_allocate_memory_priv(16) failed");
    CYG_TEST_CHECK(0x00000010 == mem_base, 
                   "cyg_pci_allocate_memory_priv(16) size failed");
    CYG_TEST_CHECK(0x00000000 == mem_assigned_addr,
                   "cyg_pci_allocate_memory_priv(16) align failed");
    // Size 64 (alignment required - <1MB)
    dev_info.base_size[0] = (CYG_PCI_CFG_BAR_SPACE_MEM 
                             | CYG_PRI_CFG_BAR_MEM_TYPE_1M
                             | 0xffffffc0);
    CYG_TEST_CHECK(cyg_pci_allocate_memory_priv(&dev_info, 0, &mem_base, 
                                                &mem_assigned_addr),
                   "cyg_pci_allocate_memory_priv(64/<1MB) failed");
    CYG_TEST_CHECK(0x00000080 == mem_base, 
                   "cyg_pci_allocate_memory_priv(64/<1MB) size failed");
    CYG_TEST_CHECK(0x00000040 == mem_assigned_addr,
                   "cyg_pci_allocate_memory_priv(64/<1MB) align failed");
    // Size 1MB (alignment required)
    dev_info.base_size[0] = CYG_PCI_CFG_BAR_SPACE_MEM | 0xfff00000;
    CYG_TEST_CHECK(cyg_pci_allocate_memory_priv(&dev_info, 0, &mem_base, 
                                                &mem_assigned_addr),
                   "cyg_pci_allocate_memory_priv(1MB) failed");
    CYG_TEST_CHECK(0x00200000 == mem_base, 
                   "cyg_pci_allocate_memory_priv(1MB) size failed");
    CYG_TEST_CHECK(0x00100000 == mem_assigned_addr,
                   "cyg_pci_allocate_memory_priv(1MB) align failed");
    // Size 16MB (alignment required)
    dev_info.base_size[0] = CYG_PCI_CFG_BAR_SPACE_MEM | 0xff000000;
    CYG_TEST_CHECK(cyg_pci_allocate_memory_priv(&dev_info, 0, &mem_base, 
                                                &mem_assigned_addr),
                   "cyg_pci_allocate_memory_priv(16MB) failed");
    CYG_TEST_CHECK(0x02000000 == mem_base, 
                   "cyg_pci_allocate_memory_priv(16MB) size failed");
    CYG_TEST_CHECK(0x01000000 == mem_assigned_addr,
                   "cyg_pci_allocate_memory_priv(16MB) align failed");
    // Size 32 (no alignment required - <1MB - FAILS!)
    // Note: When more clever allocation has been made for the bottom
    // 1MB of PCI memory space, this test needs to be made more elaborate.
    dev_info.base_size[0] = (CYG_PCI_CFG_BAR_SPACE_MEM 
                             | CYG_PRI_CFG_BAR_MEM_TYPE_1M
                             | 0xffffffe0);
    CYG_TEST_CHECK(!cyg_pci_allocate_memory_priv(&dev_info, 0, &mem_base, 
                                                 &mem_assigned_addr),
                 "cyg_pci_allocate_memory_priv(32/<1MB) unexpectedly worked!");
    
    // TBD: Check 64bit MEM allocation.

    CYG_TEST_PASS_FINISH("pci2 test OK");
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
#define N_A_MSG "Needs IO/PCI and Kernel"
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
