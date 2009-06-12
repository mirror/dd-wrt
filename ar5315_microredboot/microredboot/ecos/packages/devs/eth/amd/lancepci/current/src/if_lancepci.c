//==========================================================================
//
//      dev/if_lancepci.c
//
//      Ethernet device driver for AMD PCI Lance (for instance vmWare VLANCE)
//      compatible controllers
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
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD or other sources,
// and are covered by the appropriate copyright disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    jskov, based on lan91cxx driver by hmt & jskov, iz
// Contributors: gthomas, jskov, hmt, iz
// Date:         2002-07-17, 2003-01-26
// Purpose:
// Description:  hardware driver for AMD Lance PCI (and possibly PCnet)
//               and wmWare VLANCE ethernet
// Notes:        The controller is used in its 16bit mode. That means that
//               all addresses are 24bit only - and that all controller
//               accessed memory must be within the same 16MB region
//               (starting at 0 on older controllers).
//
//               The KEEP_STATISTICS code is not implemented yet. Look
//               for FIXME macro.
//
//####DESCRIPTIONEND####
//
//==========================================================================
//#####VMWAREDESCRIPTIONBEGIN####
//
// Notes:        The vmWare VLACNCE virtual controller does not seem to do
//               anything about SUSPEND  and seems it must be reinitialized after
//               every STOP. In addition it lacks some registers.
//
//		 Sometimes, the driver must wait to let Vmware get a tick, to
//		 process the chip initialization and control functions!!!
//
//		 That's the reason for not patching the PCnet driver
//		 but cloning a special one from it.
//
//
//####VMWAREDESCRIPTIONEND####
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/devs_eth_amd_lancepci.h>
#include <pkgconf/io_eth_drivers.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_if.h>             // delays
#include <string.h>
#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>
#ifdef CYGPKG_NET
#include <pkgconf/net.h>
#include <cyg/kernel/kapi.h>
#include <net/if.h>  			// Needed for struct ifnet
#include <pkgconf/io_eth_drivers.h>
#endif
#include CYGHWR_MEMORY_LAYOUT_H

#ifdef CYGPKG_IO_PCI
#include <cyg/io/pci.h>
#else
#error "Need PCI package here"
#endif

#define FIXME 0

#define _BUF_SIZE 1544

#ifdef CYGPKG_INFRA_DEBUG
// Then we log, OOI, the number of times we get a bad packet number
// from the tx done fifo.
int lancepci_txfifo_good = 0;
int lancepci_txfifo_bad = 0;
#endif

#include "amd_lance.h"
#define __WANT_DEVS
#include CYGDAT_DEVS_ETH_AMD_LANCEPCI_INL
#undef  __WANT_DEVS

//#define DEBUG 0xff

#if defined(CYGPKG_REDBOOT)

static void db_printf( char *fmt, ... )
{
    extern int start_console(void);
    extern void end_console(int);
    va_list a;
    int old_console;
    va_start( a, fmt );
    old_console = start_console();
    diag_vprintf( fmt, a );
    end_console(old_console);
    va_end( a );
}

#else

#define db_printf diag_printf

#endif


static struct eth_drv_sc *oursc;	//a dummy sc pointer

static void lancepci_poll(struct eth_drv_sc *sc);


// This ISR is called when the ethernet interrupt occurs
static cyg_uint32
lancepci_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    struct lancepci_priv_data *cpd = (struct lancepci_priv_data *)data;

    DEBUG_FUNCTION();

    INCR_STAT( interrupts );
    cpd->event = get_reg(oursc, LANCE_CSR_CSCR);
    if (cpd->event & LANCE_CSR_CSCR_TINT)
        cpd->txbusyh=0;				// take care of HW txbusy flag
    cyg_drv_interrupt_mask(cpd->interrupt);
    cyg_drv_interrupt_acknowledge(cpd->interrupt);
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR);  // Run the DSR
}

static void
lancepci_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    // This conditioning out is necessary because of explicit calls to this
    // DSR - which would not ever be called in the case of a polled mode
    // usage ie. in RedBoot.
#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
    struct lancepci_priv_data* cpd = (struct lancepci_priv_data *)data;
    struct cyg_netdevtab_entry *ndp = (struct cyg_netdevtab_entry *)(cpd->ndp);
    struct eth_drv_sc *sc = (struct eth_drv_sc *)(ndp->device_instance);

    // but here, it must be a *sc:
    eth_drv_dsr( vector, count, (cyg_addrword_t)sc );
#else
# ifndef CYGPKG_REDBOOT
#  error Empty lancepci ethernet DSR is compiled.  Is this what you want?
# endif
#endif
}


// The deliver function (ex-DSR)  handles the ethernet [logical] processing
static void
lancepci_deliver(struct eth_drv_sc *sc)
{
    struct lancepci_priv_data *cpd =
        (struct lancepci_priv_data *)sc->driver_private;

    DEBUG_FUNCTION();

    // Service the interrupt:
    lancepci_poll(sc);
    // Allow interrupts to happen again
    cyg_drv_interrupt_unmask(cpd->interrupt);
}

static int
lancepci_int_vector(struct eth_drv_sc *sc)
{
    struct lancepci_priv_data *cpd =
        (struct lancepci_priv_data *)sc->driver_private;

    return (cpd->interrupt);
}

// ------------------------------------------------------------------------
// Memory management
//
// Simply carve off from the front of the PCI mapped window into real memory
static cyg_uint32 lancepci_heap_size;
static cyg_uint8 *lancepci_heap_base;
static cyg_uint8 *lancepci_heap_free;

static void*
pciwindow_mem_alloc(int size)
{
    void *p_memory;
    int _size = size;

    CYG_ASSERT(
        (CYGHWR_AMD_LANCEPCI_PCI_MEM_MAP_BASE <= (int)lancepci_heap_free)
        &&
        ((CYGHWR_AMD_LANCEPCI_PCI_MEM_MAP_BASE +
          CYGHWR_AMD_LANCEPCI_PCI_MEM_MAP_SIZE) > (int)lancepci_heap_free)
        &&
        (0 < lancepci_heap_size)
        &&
        (CYGHWR_AMD_LANCEPCI_PCI_MEM_MAP_SIZE >= lancepci_heap_size)
        &&
        (CYGHWR_AMD_LANCEPCI_PCI_MEM_MAP_BASE == (int)lancepci_heap_base),
        "Heap variables corrupted" );

    p_memory = (void *)0;
    size = (size + 3) & ~3;
    if ( (lancepci_heap_free+size) < (lancepci_heap_base+lancepci_heap_size) ) {
        cyg_uint32 *p;
        p_memory = (void *)lancepci_heap_free;
        lancepci_heap_free += size;
        for ( p = (cyg_uint32 *)p_memory; _size > 0; _size -= 4 )
            *p++ = 0;
    }

#if DEBUG & 9
    db_printf("Allocated %d bytes at 0x%08x\n", size, p_memory);
#endif

    return p_memory;
}

static cyg_pci_match_func find_lancepci_match_func;

static cyg_bool
find_lancepci_match_func( cyg_uint16 v, cyg_uint16 d, cyg_uint32 c, void *p )
{
#if DEBUG & 9
    db_printf("PCI match vendor 0x%04x device 0x%04x\n", v, d);
#endif
    return (0x1022 == v) && (0x2000 == d);
}

static int
pci_init_find_lancepci( void )
{
    cyg_pci_device_id devid;
    cyg_pci_device dev_info;
    cyg_uint16 cmd;
    int device_index;
    int found_devices = 0;

    DEBUG_FUNCTION();

#ifdef CYGARC_UNCACHED_ADDRESS
    CYG_ASSERT( CYGARC_UNCACHED_ADDRESS((CYG_ADDRWORD)CYGMEM_SECTION_pci_window) ==
                CYGHWR_AMD_LANCEPCI_PCI_MEM_MAP_BASE,
      "PCI window configured does not match PCI memory section base" );
#else
    CYG_ASSERT( (CYG_ADDRWORD)CYGMEM_SECTION_pci_window ==
                CYGHWR_AMD_LANCEPCI_PCI_MEM_MAP_BASE,
      "PCI window configured does not match PCI memory section base" );
#endif
    CYG_ASSERT( CYGMEM_SECTION_pci_window_SIZE ==
                CYGHWR_AMD_LANCEPCI_PCI_MEM_MAP_SIZE,
        "PCI window configured does not match PCI memory section size" );

    if (
#ifdef CYGARC_UNCACHED_ADDRESS
         CYGARC_UNCACHED_ADDRESS((CYG_ADDRWORD)CYGMEM_SECTION_pci_window) !=
#else
         (CYG_ADDRWORD)CYGMEM_SECTION_pci_window !=
#endif
         (CYG_ADDRWORD)CYGHWR_AMD_LANCEPCI_PCI_MEM_MAP_BASE
         ||
         CYGMEM_SECTION_pci_window_SIZE !=
         CYGHWR_AMD_LANCEPCI_PCI_MEM_MAP_SIZE ) {
#if DEBUG & 8
        db_printf("pci_init_find_lancepci(): PCI window misconfigured\n");
#endif
        return 0;
    }

    // First initialize the heap in PCI window'd memory
    lancepci_heap_size = CYGHWR_AMD_LANCEPCI_PCI_MEM_MAP_SIZE;
    lancepci_heap_base = (cyg_uint8 *)CYGHWR_AMD_LANCEPCI_PCI_MEM_MAP_BASE;
    lancepci_heap_free = lancepci_heap_base;
#if DEBUG & 9
    db_printf("pcimem : 0x%08x size: 0x%08x\n", lancepci_heap_base, lancepci_heap_size);
#endif

    cyg_pci_init();
#if DEBUG & 8
    db_printf("Finished cyg_pci_init();\n");
#endif

    devid = CYG_PCI_NULL_DEVID;

    for (device_index = 0;
         device_index < CYGNUM_DEVS_ETH_AMD_LANCEPCI_DEV_COUNT;
         device_index++) {
        struct lancepci_priv_data* cpd = lancepci_priv_array[device_index];

        cpd->index = device_index;

        // See above for find_lancepci_match_func - it selects any of several
        // variants.  This is necessary in case we have multiple mixed-type
        // devices on one board in arbitrary orders.
        if (cyg_pci_find_matching( &find_lancepci_match_func, NULL, &devid )) {
#if DEBUG & 8
            db_printf("eth%d = lancepci\n", device_index);
#endif
            cyg_pci_get_device_info(devid, &dev_info);

            cpd->interrupt_handle = 0; // Flag not attached.
            if (cyg_pci_translate_interrupt(&dev_info, &cpd->interrupt)) {
#if DEBUG & 8
                db_printf(" Wired to HAL vector %d\n", cpd->interrupt);
#endif
                cyg_drv_interrupt_create(
                    cpd->interrupt,
                    1,                  // Priority - unused
                    (cyg_addrword_t)cpd,// Data item passed to ISR & DSR
                    lancepci_isr,          // ISR
                    lancepci_dsr,          // DSR
                    &cpd->interrupt_handle, // handle to intr obj
                    &cpd->interrupt_object ); // space for int obj

                cyg_drv_interrupt_attach(cpd->interrupt_handle);

                // Don't unmask the interrupt yet, that could get us into a
                // race.
            }
            else {
                cpd->interrupt = 0;
#if DEBUG & 8
                db_printf(" Does not generate interrupts.\n");
#endif
            }

            if (cyg_pci_configure_device(&dev_info)) {
#if DEBUG & 8
                int i;
                db_printf("Found device on bus %d, devfn 0x%02x:\n",
                          CYG_PCI_DEV_GET_BUS(devid),
                          CYG_PCI_DEV_GET_DEVFN(devid));

                if (dev_info.command & CYG_PCI_CFG_COMMAND_ACTIVE) {
                    db_printf(" Note that board is active. Probed"
                              " sizes and CPU addresses invalid!\n");
                }
                db_printf(" Vendor    0x%04x", dev_info.vendor);
                db_printf("\n Device    0x%04x", dev_info.device);
                db_printf("\n Command   0x%04x, Status 0x%04x\n",
                          dev_info.command, dev_info.status);

                db_printf(" Class/Rev 0x%08x", dev_info.class_rev);
                db_printf("\n Header 0x%02x\n", dev_info.header_type);

                db_printf(" SubVendor 0x%04x, Sub ID 0x%04x\n",
                          dev_info.header.normal.sub_vendor,
                          dev_info.header.normal.sub_id);

                for(i = 0; i < CYG_PCI_MAX_BAR; i++) {
                    db_printf(" BAR[%d]    0x%08x /", i, dev_info.base_address[i]);
                    db_printf(" probed size 0x%08x / CPU addr 0x%08x\n",
                              dev_info.base_size[i], dev_info.base_map[i]);
                }
                db_printf(" eth%d configured\n", device_index);
#endif
                found_devices++;
                cpd->found = 1;
                cpd->active = 0;
                cpd->devid = devid;
                cpd->base = (unsigned char*) dev_info.base_map[0];
#if DEBUG & 8
                db_printf(" I/O address = 0x%08x\n", cpd->base);
#endif

                // Don't use cyg_pci_set_device_info since it clears
                // some of the fields we want to print out below.
                cyg_pci_read_config_uint16(dev_info.devid,
                                           CYG_PCI_CFG_COMMAND, &cmd);
                cmd |= (CYG_PCI_CFG_COMMAND_IO         // enable I/O space
                        | CYG_PCI_CFG_COMMAND_MEMORY   // enable memory space
                        | CYG_PCI_CFG_COMMAND_MASTER); // enable bus master
                cyg_pci_write_config_uint16(dev_info.devid,
                                            CYG_PCI_CFG_COMMAND, cmd);

                // This is the indicator for "uses an interrupt"
                if (cpd->interrupt_handle != 0) {
                    cyg_drv_interrupt_acknowledge(cpd->interrupt);
                    cyg_drv_interrupt_unmask(cpd->interrupt);
#if DEBUG & 8
                    db_printf(" Enabled interrupt %d\n", cpd->interrupt);
#endif
                }
#if DEBUG & 8
                db_printf(" **** Device enabled for I/O and Memory "
                            "and Bus Master\n");
#endif
            }
            else {
                cpd->found = 0;
                cpd->active = 0;
#if DEBUG & 8
                db_printf("Failed to configure device %d\n", device_index);
#endif
            }
        }
        else {
            cpd->found = 0;
            cpd->active = 0;
#if DEBUG & 8
            db_printf("eth%d not found\n", device_index);
#endif
        }
    }

    if (0 == found_devices)
        return 0;

    return 1;
}


static bool
amd_lancepci_init(struct cyg_netdevtab_entry *tab)
{
    static int initialized = 0; // only probe PCI et al *once*
    struct eth_drv_sc *sc = (struct eth_drv_sc *)tab->device_instance;
    struct lancepci_priv_data *cpd =
        (struct lancepci_priv_data *)sc->driver_private;
    cyg_uint16 val;
    cyg_uint32 b;
    cyg_uint8* p;
    cyg_uint8* d;
    int i;

    DEBUG_FUNCTION();

    if ( 0 == initialized++ ) {
        // then this is the first time ever:
        if ( ! pci_init_find_lancepci() ) {
#if DEBUG & 8
            db_printf( "pci_init_find_lancepci failed" );
#endif
            return false;
        }
    }

    // If this device is not present, exit
    if (0 == cpd->found)
        return 0;

#if DEBUG & 8
    db_printf("lancepci at base 0x%08x, EEPROM key 0x%04x\n",
                cpd->base, _SU16(cpd->base, LANCE_IO_ID));
#endif

#if 0
    // FIXME: Doesn't work with non-conforming EEPROMS
    if (LANCE_IO_ID_KEY != _SU16(cpd->base, LANCE_IO_ID) ) {
        db_printf("Lance EPROM key not found\n");
        return false;
    }
#endif

#if DEBUG & 9
    db_printf("pcimem : %08x size: %08x\n", lancepci_heap_base, lancepci_heap_size);
#endif

    // Prepare ESA
    if (!cpd->hardwired_esa) {
        // Don't use the address from the EEPROM for VMware
        // Use the address that VMware prepares in CSR_PAR registers
        // if You want to be able to use NAT networking. (iz@elsis.si Feb 27 04)
        //
        // p = cpd->base + LANCE_IO_EEPROM;
        // for (i = 0; i < 6; i++)
        // cpd->esa[i] = *p++;
        put_reg(sc, LANCE_CSR_CSCR, LANCE_CSR_CSCR_STOP);
        for (i = 0;  i < sizeof(cpd->esa);  i += 2) {
            cyg_uint16 z = get_reg(sc, LANCE_CSR_PAR0+i/2 );
            cpd->esa[i] =   (cyg_uint8)(0xff & z);
            cpd->esa[i+1] = (cyg_uint8)(0xff & (z >> 8));
      }

    }
#if DEBUG & 9
    db_printf("Lance - %s ESA: %02x:%02x:%02x:%02x:%02x:%02x\n",
                (cpd->hardwired_esa) ? "static" : "eeprom",
                cpd->esa[0], cpd->esa[1], cpd->esa[2],
                cpd->esa[3], cpd->esa[4], cpd->esa[5] );
#endif


    // Prepare RX and TX rings
    p = cpd->rx_ring = (cyg_uint8*) CYGARC_UNCACHED_ADDRESS((cyg_uint32)pciwindow_mem_alloc((1<<cpd->rx_ring_log_cnt)*LANCE_RD_SIZE));
    memset(cpd->rx_ring,0,(1<<cpd->rx_ring_log_cnt)*LANCE_RD_SIZE);

    d = cpd->rx_buffers = (cyg_uint8*) CYGARC_UNCACHED_ADDRESS((cyg_uint32)pciwindow_mem_alloc(_BUF_SIZE*cpd->rx_ring_cnt));
    memset(cpd->rx_buffers,0,_BUF_SIZE*cpd->rx_ring_cnt);

    for (i = 0; i < cpd->rx_ring_cnt; i++) {
        HAL_PCI_CPU_TO_BUS(d, (cyg_uint8 *)b);
        _SU32(p, LANCE_RD_PTR) = (b & LANCE_RD_PTR_MASK) | LANCE_RD_PTR_OWN;
        _SU16(p, LANCE_RD_BLEN) = (-_BUF_SIZE);
        p += LANCE_RD_SIZE;
        d += _BUF_SIZE;
    }
    cpd->rx_ring_next = 0;

    p = cpd->tx_ring = (cyg_uint8*) CYGARC_UNCACHED_ADDRESS((cyg_uint32)pciwindow_mem_alloc((1<<cpd->tx_ring_log_cnt)*LANCE_TD_SIZE));
    memset(cpd->tx_ring,0,(1<<cpd->tx_ring_log_cnt)*LANCE_TD_SIZE);

    d = cpd->tx_buffers = (cyg_uint8*) CYGARC_UNCACHED_ADDRESS((cyg_uint32)pciwindow_mem_alloc(_BUF_SIZE*cpd->tx_ring_cnt));
    for (i = 0; i < cpd->tx_ring_cnt; i++) {
        HAL_PCI_CPU_TO_BUS(d, (cyg_uint8 *)b);
        _SU32(p, LANCE_RD_PTR) = b & LANCE_TD_PTR_MASK;
        p += LANCE_TD_SIZE;
        d += _BUF_SIZE;
    }
    cpd->tx_ring_free = cpd->tx_ring_alloc = cpd->tx_ring_owned = 0;

    // Initialization table
    cpd->init_table = (cyg_uint8*)CYGARC_UNCACHED_ADDRESS((cyg_uint32)pciwindow_mem_alloc(LANCE_IB_SIZE));
    _SU16(cpd->init_table, LANCE_IB_MODE) = 0x0000;
    for (i = 0; i < 6; i++)
        _SU8(cpd->init_table, LANCE_IB_PADR0+i) = cpd->esa[i];
    for (i = 0; i < 8; i++)
        _SU8(cpd->init_table, LANCE_IB_LADRF0+i) = 0;

    HAL_PCI_CPU_TO_BUS(cpd->rx_ring, (cyg_uint8 *)b);
    _SU32(cpd->init_table, LANCE_IB_RDRA) = ((b & LANCE_IB_RDRA_PTR_mask)
                                        | (cpd->rx_ring_log_cnt << LANCE_IB_RDRA_CNT_shift));
    HAL_PCI_CPU_TO_BUS(cpd->tx_ring, (cyg_uint8 *)b);
    _SU32(cpd->init_table, LANCE_IB_TDRA) = ((b & LANCE_IB_TDRA_PTR_mask)
                                        | (cpd->tx_ring_log_cnt << LANCE_IB_TDRA_CNT_shift));

#if DEBUG & 9
    db_printf("Loading up lance controller from table at 0x%08x\n", cpd->init_table);
    db_printf(" Mode 0x%04x\n", _SU16(cpd->init_table, LANCE_IB_MODE));
    db_printf(" PADR %02x:%02x:%02x:%02x:%02x:%02x ",
                _SU8(cpd->init_table, LANCE_IB_PADR0+0), _SU8(cpd->init_table, LANCE_IB_PADR0+1),
                _SU8(cpd->init_table, LANCE_IB_PADR0+2), _SU8(cpd->init_table, LANCE_IB_PADR0+3),
                _SU8(cpd->init_table, LANCE_IB_PADR0+4), _SU8(cpd->init_table, LANCE_IB_PADR0+5));
    db_printf("LADR %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
                _SU8(cpd->init_table, LANCE_IB_LADRF0+0), _SU8(cpd->init_table, LANCE_IB_LADRF0+1),
                _SU8(cpd->init_table, LANCE_IB_LADRF0+2), _SU8(cpd->init_table, LANCE_IB_LADRF0+3),
                _SU8(cpd->init_table, LANCE_IB_LADRF0+4), _SU8(cpd->init_table, LANCE_IB_LADRF0+5),
                _SU8(cpd->init_table, LANCE_IB_LADRF0+5), _SU8(cpd->init_table, LANCE_IB_LADRF0+7));
    db_printf(" RX 0x%08x (len %d) TX 0x%08x (len %d)\n",
                _SU32(cpd->init_table, LANCE_IB_RDRA) & 0x1fffffff,
                (_SU32(cpd->init_table, LANCE_IB_RDRA) >> LANCE_IB_RDRA_CNT_shift) & 7,
                _SU32(cpd->init_table, LANCE_IB_TDRA) & 0x1fffffff,
                (_SU32(cpd->init_table, LANCE_IB_TDRA) >> LANCE_IB_TDRA_CNT_shift) & 7);
#endif

    // Reset chip
    HAL_PCI_IO_READ_UINT16(cpd->base+LANCE_IO_RESET, val);

    // Load up chip with buffers.
    // Note: There is a 16M limit on the addresses used by the driver
    // since the top 8 bits of the init_table address is appended to
    // all other addresses used by the controller.
    HAL_PCI_CPU_TO_BUS(cpd->init_table, (cyg_uint8 *)b);
    put_reg(sc, LANCE_CSR_IBA0, (b >>  0) & 0xffff);
    put_reg(sc, LANCE_CSR_IBA1, (b >> 16) & 0xffff);
    // Disable automatic TX polling (_send will force a poll), pad
    // XT frames to legal length, mask status interrupts.
    put_reg(sc, LANCE_CSR_TFC, (LANCE_CSR_TFC_TXDPOLL | LANCE_CSR_TFC_APAD_XMT
                                | LANCE_CSR_TFC_MFCOM | LANCE_CSR_TFC_RCVCCOM
                                | LANCE_CSR_TFC_TXSTRTM));
    // Recover after TX FIFO underflow
    put_reg(sc, LANCE_CSR_IM, LANCE_CSR_IM_DXSUFLO);
    // Initialize controller - load up init_table
    put_reg(sc, LANCE_CSR_CSCR, LANCE_CSR_CSCR_INIT);
    while (0 == (get_reg(sc, LANCE_CSR_CSCR) & LANCE_CSR_CSCR_IDON));

    // Stop controller
    put_reg(sc, LANCE_CSR_CSCR, LANCE_CSR_CSCR_STOP);

#if DEBUG & 9
    db_printf("lancepci controller state is now:\n");
    db_printf(" Mode 0x%04x  TFC 0x%04x\n", _SU16(cpd->init_table, LANCE_IB_MODE), get_reg(sc, LANCE_CSR_TFC));
    db_printf(" PADR %04x:%04x:%04x ",
                get_reg(sc, LANCE_CSR_PAR0),
                get_reg(sc, LANCE_CSR_PAR1),
                get_reg(sc, LANCE_CSR_PAR2));
    db_printf("LADR %04x:%04x:%04x:%04x\n",
                get_reg(sc, LANCE_CSR_LAR0),
                get_reg(sc, LANCE_CSR_LAR1),
                get_reg(sc, LANCE_CSR_LAR2),
                get_reg(sc, LANCE_CSR_LAR3));
    db_printf(" RX 0x%04x%04x (len 0x%04x) TX 0x%04x%04x (len 0x%04x)\n",
                get_reg(sc, LANCE_CSR_BARRU), get_reg(sc, LANCE_CSR_BARRL),
                get_reg(sc, LANCE_CSR_RRLEN),
                get_reg(sc, LANCE_CSR_BATRU), get_reg(sc, LANCE_CSR_BATRL),
                get_reg(sc, LANCE_CSR_TRLEN));

    val = get_reg(sc, LANCE_CSR_ID_LO);
    db_printf("lancepci ID 0x%04x (%s) ",
                val,
                (0x5003 == val) ? "Am79C973" : (0x7003 == val) ? "Am79C975" :
		        (0x1003 == val) ? "Am79C900 or wmWare VLANCE" : "Unknown");
    val = get_reg(sc, LANCE_CSR_ID_HI);
    db_printf("Part IDU 0x%03x Silicon rev %d\n",
                val & 0x0fff, (val >> 12) & 0xf);
#endif
    // and record the net dev pointer
    cpd->ndp = (void *)tab;
    cpd->active = 0;
    oursc=sc;

    // Initialize upper level driver
    (sc->funs->eth_drv->init)(sc, cpd->esa);
    cpd->txbusyh=cpd->txbusy=0;
    cpd->event=0;
    db_printf("Lancepci driver loaded and Init Done\n");
    return true;
}

static void
lancepci_stop(struct eth_drv_sc *sc)
{
    cyg_uint32 b;
    struct lancepci_priv_data *cpd =
        (struct lancepci_priv_data *)sc->driver_private;

    DEBUG_FUNCTION();
    if (!cpd->active)
        return;
    if (cpd->txbusyh) {
#if DEBUG & 9
        db_printf("Lancepci-stop:waiting for tx empty\n");
#endif
        b=100;
        while (cpd->txbusyh && b) {
	    CYGACC_CALL_IF_DELAY_US(200);
	    b--;
	 }
    }
    put_reg(sc, LANCE_CSR_CSCR, LANCE_CSR_CSCR_STOP);
    cpd->active = 0;
#if DEBUG & 9
    db_printf("Lancepci-stop:done\n");
#endif
}

//
// This function is called to "start up" the interface.  It may be called
// multiple times, even when the hardware is already running.  It will be
// called whenever something "hardware oriented" changes and should leave
// the hardware ready to send/receive packets.
//
static void
lancepci_start(struct eth_drv_sc *sc, unsigned char *enaddr, int flags)
{
    cyg_uint16 reg;
    cyg_uint32 b;
    struct lancepci_priv_data *cpd =
        (struct lancepci_priv_data *)sc->driver_private;
#ifdef CYGPKG_NET
    struct ifnet *ifp = &sc->sc_arpcom.ac_if;
#endif
    DEBUG_FUNCTION();

    // If device is already active, stop it
#if DEBUG & 9
	db_printf("Lancepci-start:entered\n");
#endif
    if (cpd->active)
     {
        if (cpd->txbusyh) {
#if DEBUG & 9
	    db_printf("Lancepci-start:waiting for tx empty\n");
#endif
	    b=100;
     	    while (cpd->txbusyh && b) {
	        CYGACC_CALL_IF_DELAY_US(200);
		b--;
	    }
	}
    	put_reg(sc, LANCE_CSR_CSCR, LANCE_CSR_CSCR_STOP);
    	cpd->active = 0;
#if DEBUG & 9
    	db_printf("Lancepci-start:stopped\n");
#endif
    }
    CYGACC_CALL_IF_DELAY_US(200);

#ifdef CYGPKG_NET
    if (( 0
#ifdef ETH_DRV_FLAGS_PROMISC_MODE
         != (flags & ETH_DRV_FLAGS_PROMISC_MODE)
#endif
        ) || (ifp->if_flags & IFF_PROMISC)
        ) {
        // Then we select promiscuous mode.
    	_SU16(cpd->init_table, LANCE_IB_MODE) = 0x0000|LANCE_CSR_MODE_PROM;
#if DEBUG & 9
	db_printf("Promisc MODE!");
#endif
    }
    else _SU16(cpd->init_table, LANCE_IB_MODE) = 0x0000;
#endif
    cpd->rx_ring_next = 0;
    cpd->tx_ring_free = cpd->tx_ring_alloc = cpd->tx_ring_owned = 0;
    // Init the chip again
    HAL_PCI_CPU_TO_BUS(cpd->init_table, (cyg_uint8 *)b);
    put_reg(sc, LANCE_CSR_IBA0, (b >>  0) & 0xffff);
    put_reg(sc, LANCE_CSR_IBA1, (b >> 16) & 0xffff);
    // Disable automatic TX polling (_send will force a poll), pad
    // XT frames to legal length, mask status interrupts.
    put_reg(sc, LANCE_CSR_TFC, (LANCE_CSR_TFC_TXDPOLL | LANCE_CSR_TFC_APAD_XMT
                                | LANCE_CSR_TFC_MFCOM | LANCE_CSR_TFC_RCVCCOM
                                | LANCE_CSR_TFC_TXSTRTM));
    // Recover after TX FIFO underflow
    put_reg(sc, LANCE_CSR_IM, LANCE_CSR_IM_DXSUFLO);
    // Initialize controller - load up init_table
    put_reg(sc, LANCE_CSR_CSCR, LANCE_CSR_CSCR_INIT);
    while (0 == (get_reg(sc, LANCE_CSR_CSCR) & LANCE_CSR_CSCR_IDON));
	reg=get_reg(sc,LANCE_CSR_CSCR);
    put_reg(sc, LANCE_CSR_CSCR, (reg|(LANCE_CSR_CSCR_IENA | LANCE_CSR_CSCR_STRT))&~LANCE_CSR_CSCR_INIT);
#if DEBUG & 9
	reg=get_reg(sc,LANCE_CSR_CSCR);
	db_printf("CSR after start = %4x\n",reg);
#endif
    cpd->active = 1; cpd->txbusy=0; cpd->txbusyh=0;
    // delay is necessary for Vmware to get a tick !!!
    CYGACC_CALL_IF_DELAY_US(50000);
}

//
// This routine is called to perform special "control" opertions
//
static int
lancepci_control(struct eth_drv_sc *sc, unsigned long key,
               void *data, int data_length)
{
    cyg_uint8 *esa = (cyg_uint8 *)data;
    int i, res;
    cyg_uint16 reg;
    struct lancepci_priv_data *cpd =
        (struct lancepci_priv_data *)sc->driver_private;

    DEBUG_FUNCTION();

#if DEBUG & 9
	db_printf("Lancepci-control:entered\n");
#endif
    res = 0;                            // expect success
    switch (key) {
        case ETH_DRV_SET_MAC_ADDRESS:
#if 9 & DEBUG
            db_printf("PCNET - set ESA: %02x:%02x:%02x:%02x:%02x:%02x\n",
                esa[0], esa[1], esa[2], esa[3], esa[4], esa[5] );
#endif // DEBUG
            for ( i = 0; i < sizeof(cpd->esa);  i++ )
                cpd->esa[i] = esa[i];
            for (i = 0;  i < sizeof(cpd->esa);  i += 2) {
                reg = cpd->esa[i] | (cpd->esa[i+1] << 8);
                put_reg(sc, LANCE_CSR_PAR0+i/2, reg );
            }
            for (i = 0; i < 6; i++)	// in case of later restart
                _SU8(cpd->init_table, LANCE_IB_PADR0+i) = cpd->esa[i];
            break;
#ifdef ETH_DRV_GET_MAC_ADDRESS
        case ETH_DRV_GET_MAC_ADDRESS:
            // Extract the MAC address that is in the chip, and tell the
            // system about it.
            for (i = 0;  i < sizeof(cpd->esa);  i += 2) {
                cyg_uint16 z = get_reg(sc, LANCE_CSR_PAR0+i/2 );
                esa[i] =   (cyg_uint8)(0xff & z);
                esa[i+1] = (cyg_uint8)(0xff & (z >> 8));
            }
            break;
#endif
#ifdef ETH_DRV_GET_IF_STATS_UD
        case ETH_DRV_GET_IF_STATS_UD: // UD == UPDATE
#endif
        // drop through
#ifdef ETH_DRV_GET_IF_STATS
        case ETH_DRV_GET_IF_STATS:
#endif

#if defined(ETH_DRV_GET_IF_STATS) || defined (ETH_DRV_GET_IF_STATS_UD)
        {
            struct ether_drv_stats *p = (struct ether_drv_stats *)data;
            // Chipset entry is no longer supported; RFC1573.
            for ( i = 0; i < SNMP_CHIPSET_LEN; i++ )
                p->snmp_chipset[i] = 0;

            // This perhaps should be a config opt, so you can make up your own
            // description, or supply it from the instantiation.
            strcpy( p->description, "AMD LancePCI" );
            // CYG_ASSERT( 48 > strlen(p->description), "Description too long" );

            p->operational = 3;         // LINK UP
            p->duplex = 2;              // 2 = SIMPLEX
            p->speed = 10 * 1000000;

#if FIXME
#ifdef KEEP_STATISTICS
            {
                struct amd_lancepci_stats *ps = &(cpd->stats);

                // Admit to it...
                p->supports_dot3        = true;

                p->tx_good              = ps->tx_good             ;
                p->tx_max_collisions    = ps->tx_max_collisions   ;
                p->tx_late_collisions   = ps->tx_late_collisions  ;
                p->tx_underrun          = ps->tx_underrun         ;
                p->tx_carrier_loss      = ps->tx_carrier_loss     ;
                p->tx_deferred          = ps->tx_deferred         ;
                p->tx_sqetesterrors     = ps->tx_sqetesterrors    ;
                p->tx_single_collisions = ps->tx_single_collisions;
                p->tx_mult_collisions   = ps->tx_mult_collisions  ;
                p->tx_total_collisions  = ps->tx_total_collisions ;
                p->rx_good              = ps->rx_good             ;
                p->rx_crc_errors        = ps->rx_crc_errors       ;
                p->rx_align_errors      = ps->rx_align_errors     ;
                p->rx_resource_errors   = ps->rx_resource_errors  ;
                p->rx_overrun_errors    = ps->rx_overrun_errors   ;
                p->rx_collisions        = ps->rx_collisions       ;
                p->rx_short_frames      = ps->rx_short_frames     ;
                p->rx_too_long_frames   = ps->rx_too_long_frames  ;
                p->rx_symbol_errors     = ps->rx_symbol_errors    ;

                p->interrupts           = ps->interrupts          ;
                p->rx_count             = ps->rx_count            ;
                p->rx_deliver           = ps->rx_deliver          ;
                p->rx_resource          = ps->rx_resource         ;
                p->rx_restart           = ps->rx_restart          ;
                p->tx_count             = ps->tx_count            ;
                p->tx_complete          = ps->tx_complete         ;
                p->tx_dropped           = ps->tx_dropped          ;
            }
#endif // KEEP_STATISTICS
#endif // FIXME

            p->tx_queue_len = 1;
            break;
        }
#endif
        default:
            res = 1;
            break;
    }
#if DEBUG & 9
    db_printf("Lancepci-control:done\n");
#endif
    CYGACC_CALL_IF_DELAY_US(50000);	// let VMware get a tick
    return res;
}

//
// This routine is called to see if it is possible to send another packet.
// It will return non-zero if a transmit is possible, zero otherwise.
//
static int
lancepci_can_send(struct eth_drv_sc *sc)
{
    struct lancepci_priv_data *cpd =
        (struct lancepci_priv_data *)sc->driver_private;

    DEBUG_FUNCTION();

    return (0 == cpd->txbusy);
}

//
// This routine is called to send data to the hardware.
static void
lancepci_send(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len,
            int total_len, unsigned long key)
{
    struct lancepci_priv_data *cpd =
        (struct lancepci_priv_data *)sc->driver_private;
    int i, len, plen, ring_entry;

    cyg_uint8* sdata = NULL;
    cyg_uint8 *d, *buf, *txd;
    cyg_uint16 ints;
    cyg_uint32 b;

    DEBUG_FUNCTION();

    INCR_STAT( tx_count );

    cpd->txbusy = 1; cpd->txbusyh=1;
    cpd->txkey = key;

    // Find packet length
    plen = 0;
    for (i = 0;  i < sg_len;  i++)
        plen += sg_list[i].len;

    CYG_ASSERT( plen == total_len, "sg data length mismatch" );

    // Get next TX descriptor
    ring_entry = cpd->tx_ring_free;
    do {
        if (cpd->tx_ring_owned == cpd->tx_ring_cnt) {
            // Is this a dead end? Probably is.
#if DEBUG & 1
            db_printf("%s: Allocation failed! Retrying...\n", __FUNCTION__ );
#endif
            continue;
        }

        cpd->tx_ring_free++;
        cpd->tx_ring_owned++;
        if (cpd->tx_ring_free == cpd->tx_ring_cnt)
            cpd->tx_ring_free = 0;
    } while (0);

    txd = cpd->tx_ring + ring_entry*LANCE_TD_SIZE;
    buf = cpd->tx_buffers + ring_entry*_BUF_SIZE;
    CYG_ASSERT(0 == (_SU32(txd, LANCE_TD_PTR) & LANCE_TD_PTR_OWN),
               "TX descriptor not free");

#if DEBUG & 4
    db_printf("#####Tx descriptor 0x%08x buffer 0x%08x\n",
                txd, buf);
#endif

    // Put data into buffer
    d = buf;
    for (i = 0;  i < sg_len;  i++) {
        sdata = (cyg_uint8 *)sg_list[i].buf;
        len = sg_list[i].len;

        CYG_ASSERT( sdata, "No sg data pointer here" );
        while(len--)
            *d++ = *sdata++;
    }
    CYG_ASSERT( sdata, "No sg data pointer outside" );

#if DEBUG & 1
    db_printf("CSCR %04x\n", get_reg(sc, LANCE_CSR_CSCR));
#endif
    _SU16(txd, LANCE_TD_LEN) = (-plen);
    _SU16(txd, LANCE_TD_MISC) = 0;
    HAL_PCI_CPU_TO_BUS(buf, (cyg_uint8 *)b);
    _SU32(txd, LANCE_TD_PTR) = ((b & LANCE_TD_PTR_MASK)
                                | LANCE_TD_PTR_OWN | LANCE_TD_PTR_STP | LANCE_TD_PTR_ENP);

#if DEBUG & 1
    db_printf("Last TX: LEN %04x MISC %04x PTR %08x\n",
                _SU16(txd, LANCE_TD_LEN),
                _SU16(txd, LANCE_TD_MISC),
                _SU32(txd, LANCE_TD_PTR));
#endif

    // This delay seems to be necessary on some platforms
    // (Malta 5kc for example).
    // Why it is needed is not clear, but removing it or
    // reducing it cause transmission failures in RedBoot (at least).
    CYGACC_CALL_IF_DELAY_US(100);


    // Set transmit demand
    ints = get_reg(sc, LANCE_CSR_CSCR);
    ints &= LANCE_CSR_CSCR_EV_MASK;
    ints |= LANCE_CSR_CSCR_TDMD;
    put_reg(sc, LANCE_CSR_CSCR, ints);

#if DEBUG & 1
    ints = get_reg(sc, LANCE_CSR_CSCR);
    db_printf("%s:END: ints at TX: 0x%04x\n", __FUNCTION__, ints);
#endif

    // This is another mystery delay like the one above. This one is
    // even stranger, since waiting here at the _end_ of the function
    // should have no effect.
    CYGACC_CALL_IF_DELAY_US(200);
}

static void
lancepci_TxEvent(struct eth_drv_sc *sc, int stat)
{
     struct lancepci_priv_data *cpd =
        (struct lancepci_priv_data *)sc->driver_private;
    int success = 1;
    cyg_uint8 *txd;
    cyg_uint16 ints;
    cyg_uint32 pkt_stat;

    DEBUG_FUNCTION();

    if (0 == cpd->tx_ring_owned) {
#if DEBUG & 1
        db_printf("%s: got TX completion when no outstanding packets\n", __FUNCTION__);
#endif
        return;
    }

    INCR_STAT( tx_complete );

    txd = cpd->tx_ring + cpd->tx_ring_alloc*LANCE_TD_SIZE;
    pkt_stat = _SU32(txd, LANCE_TD_PTR);
    if (pkt_stat & LANCE_TD_PTR_OWN) {
#if DEBUG & 1
        db_printf("%s: got TX completion when buffer is still owned\n", __FUNCTION__);
#endif
        // first dirty ring entry not freed - wtf?
    }

    if (pkt_stat & LANCE_TD_PTR_ERR) {
        // We had an error. Tell the stack.
        success = 0;
#if DEBUG & 1
        db_printf("%s: TX failure, retrying...\n", __FUNCTION__);
#endif
    }

    cpd->tx_ring_alloc++;
    if (cpd->tx_ring_alloc == cpd->tx_ring_cnt)
        cpd->tx_ring_alloc = 0;
    cpd->tx_ring_owned--;

#if FIXME
#ifdef KEEP_STATISTICS
    {
        cyg_uint16 reg;

        reg = get_reg( sc, LANCE_CSR_CSCR );

        // Covering each bit in turn...
        if ( reg & LANCE_STATUS_TX_UNRN   ) INCR_STAT( tx_underrun );
        //if ( reg & LANCE_STATUS_LINK_OK ) INCR_STAT(  );
        //if ( reg & LANCE_STATUS_CTR_ROL ) INCR_STAT(  );
        //if ( reg & LANCE_STATUS_EXC_DEF ) INCR_STAT(  );
        if ( reg & LANCE_STATUS_LOST_CARR ) INCR_STAT( tx_carrier_loss );
        if ( reg & LANCE_STATUS_LATCOL    ) INCR_STAT( tx_late_collisions );
        //if ( reg & LANCE_STATUS_WAKEUP  ) INCR_STAT(  );
        if ( reg & LANCE_STATUS_TX_DEFR   ) INCR_STAT( tx_deferred );
        //if ( reg & LANCE_STATUS_LTX_BRD ) INCR_STAT(  );
        if ( reg & LANCE_STATUS_SQET      ) INCR_STAT( tx_sqetesterrors );
        if ( reg & LANCE_STATUS_16COL     ) INCR_STAT( tx_max_collisions );
        //if ( reg & LANCE_STATUS_LTX_MULT) INCR_STAT(  );
        if ( reg & LANCE_STATUS_MUL_COL   ) INCR_STAT( tx_mult_collisions );
        if ( reg & LANCE_STATUS_SNGL_COL  ) INCR_STAT( tx_single_collisions );
        if ( reg & LANCE_STATUS_TX_SUC    ) INCR_STAT( tx_good );

        cpd->stats.tx_total_collisions =
            cpd->stats.tx_late_collisions +
            cpd->stats.tx_max_collisions +
            cpd->stats.tx_mult_collisions +
            cpd->stats.tx_single_collisions;

        // We do not need to look in the Counter Register (LANCE_COUNTER)
        // because it just mimics the info we already have above.
    }
#endif // KEEP_STATISTICS
#endif // FIXME

    // Ack the TX int which clears the packet from the TX completion
    // queue.
    ints = get_reg(sc, LANCE_CSR_CSCR);
    ints |= LANCE_CSR_CSCR_TINT;
    put_reg(sc, LANCE_CSR_CSCR, ints);

#if DEBUG & 4
    db_printf("#####Tx packet freed 0x%08x\n", txd );
#endif

    if ( cpd->txbusy ) {
        cpd->txbusy = 0;
        (sc->funs->eth_drv->tx_done)(sc, cpd->txkey, success);
    }
}


//
// This function is called when a packet has been received.  Its job is
// to prepare to unload the packet from the hardware.  Once the length of
// the packet is known, the upper layer of the driver can be told.  When
// the upper layer is ready to unload the packet, the internal function
// 'lancepci_recv' will be called to actually fetch it from the hardware.
//
static void
lancepci_RxEvent(struct eth_drv_sc *sc)
{
    struct lancepci_priv_data *cpd =
        (struct lancepci_priv_data *)sc->driver_private;
    cyg_uint8 *rxd;
    cyg_uint32 rstat;
    cyg_uint16 ints, len;

    DEBUG_FUNCTION();

    ints = get_reg(sc, LANCE_CSR_CSCR);
#if DEBUG & 1
    db_printf("RxEvent - CSR: 0x%04x\n", ints);
#endif

    while (1) {
        // Get state of next (supposedly) full ring entry
        cpd->rxpacket = cpd->rx_ring_next;
        rxd = cpd->rx_ring + cpd->rxpacket*LANCE_RD_SIZE;
        rstat = _SU32(rxd, LANCE_RD_PTR);

        // Keep going until we hit an entry that is owned by the
        // controller.
        if (rstat & LANCE_RD_PTR_OWN) {
#if DEBUG & 1
            int i;
            for (i = 0; i < cpd->rx_ring_cnt; i++) {
                rxd = cpd->rx_ring + i*LANCE_RD_SIZE;
                rstat = _SU32(rxd, LANCE_RD_PTR);

                if (!(rstat & LANCE_RD_PTR_OWN)) {
                    int i;
                    cyg_uint32 rstat;
                    cyg_uint16 mlen, blen;
                    cyg_uint8* rxd;

                    db_printf("%s: Inconsistent RX state\n", __FUNCTION__);
                    for (i = 0; i < cpd->rx_ring_cnt; i++) {
                        rxd = cpd->rx_ring + i*LANCE_RD_SIZE;

                        rstat = _SU32(rxd, LANCE_RD_PTR);
                        blen = _SU16(rxd, LANCE_RD_BLEN);
                        mlen = _SU16(rxd, LANCE_RD_MLEN);
                        db_printf(" %02d: 0x%08x:0x%04x:0x%04x\n", i, rstat, blen, mlen);
                    }
                }
            }
#endif
            break;
        }

#if DEBUG & 4
        db_printf("#####Rx packet at index %d\n", cpd->rxpacket);
#endif

        // Increment counts
        INCR_STAT( rx_count );
        cpd->rx_ring_next++;
        if (cpd->rx_ring_next == cpd->rx_ring_cnt) cpd->rx_ring_next = 0;

        len = _SU16(rxd, LANCE_RD_MLEN);

#ifdef KEEP_STATISTICS
        //if ( rstat & LANCE_RD_PTR_FRAM ) INCR_STAT( rx_frame_errors );
        //if ( rstat & LANCE_RD_PTR_OFLO ) INCR_STAT(  );
        if ( rstat & LANCE_RD_PTR_CRC ) INCR_STAT( rx_crc_errors );
        //if ( rstat & LANCE_RD_PTR_BUFF ) INCR_STAT(  );
#endif // KEEP_STATISTICS

        if (0 == (rstat & LANCE_RD_PTR_ERR)) {
            // It's OK
            INCR_STAT( rx_good );

#if DEBUG & 1
            db_printf("RxEvent good rx - stat: 0x%08x, len: 0x%04x\n", rstat, len);
#endif
            // Check for bogusly short packets; can happen in promisc
            // mode: Asserted against and checked by upper layer
            // driver.
#ifdef CYGPKG_NET
            if ( len > sizeof( struct ether_header ) )
                // then it is acceptable; offer the data to the network stack
#endif
                (sc->funs->eth_drv->recv)(sc, len);
        } else {
            // Not OK for one reason or another...
#if DEBUG & 1
            db_printf("RxEvent - No RX bit: stat: 0x%08x, len: 0x%04x\n",
                        rstat, len);
#endif
        }

        // Free packet (clear all status flags, and set OWN)
        _SU32(rxd, LANCE_RD_PTR) &= LANCE_RD_PTR_MASK;
        _SU32(rxd, LANCE_RD_PTR) |= LANCE_RD_PTR_OWN;
    }

    // Ack RX interrupt set
    ints = get_reg(sc, LANCE_CSR_CSCR);
    ints &= LANCE_CSR_CSCR_EV_MASK;
    ints |= LANCE_CSR_CSCR_RINT;
    put_reg(sc, LANCE_CSR_CSCR, ints);
}

//
// This function is called as a result of the "eth_drv_recv()" call above.
// Its job is to actually fetch data for a packet from the hardware once
// memory buffers have been allocated for the packet.  Note that the buffers
// may come in pieces, using a scatter-gather list.  This allows for more
// efficient processing in the upper layers of the stack.
//
static void
lancepci_recv(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len)
{
    struct lancepci_priv_data *cpd =
        (struct lancepci_priv_data *)sc->driver_private;
    int i, mlen=0, plen;
    cyg_uint8 *data, *rxd, *buf;

    DEBUG_FUNCTION();

    rxd = cpd->rx_ring + cpd->rxpacket*LANCE_RD_SIZE;
    buf = cpd->rx_buffers + cpd->rxpacket*_BUF_SIZE;

    INCR_STAT( rx_deliver );

    plen = _SU16(rxd, LANCE_RD_MLEN);

    for (i = 0;  i < sg_len;  i++) {
        data = (cyg_uint8*)sg_list[i].buf;
        mlen = sg_list[i].len;

#if DEBUG & 1
        db_printf("%s : mlen %x, plen %x\n", __FUNCTION__, mlen, plen);
#endif
        if (data) {
            while (mlen > 0) {
                *data++ = *buf++;
                mlen--;
                plen--;
            }
        }
    }
}

static void
lancepci_poll(struct eth_drv_sc *sc)
{
    cyg_uint16 event;
    struct lancepci_priv_data *cpd =
        (struct lancepci_priv_data *)sc->driver_private;

//  DEBUG_FUNCTION();

    while (1) {
        // Get the (unmasked) requests
	if (cpd->event) {
	    event=cpd->event;
	    cpd->event=0;
	}
	else
            event = get_reg(sc, LANCE_CSR_CSCR);
        if (!((LANCE_CSR_CSCR_ERR|LANCE_CSR_CSCR_INTR) & event))
            break;

        if (event & LANCE_CSR_CSCR_RINT) {
            lancepci_RxEvent(sc);
        }
        else if (event & LANCE_CSR_CSCR_TINT) {
	    cpd->txbusyh=0;		// again , for polled mode
            lancepci_TxEvent(sc, event);
        }
        else if (event & LANCE_CSR_CSCR_MISS) {
#if DEBUG & 1
            int i;
            cyg_uint32 rstat;
            cyg_uint16 mlen, blen;
            cyg_uint8* rxd;
            struct lancepci_priv_data *cpd =
                (struct lancepci_priv_data *)sc->driver_private;

            db_printf("%s: Ran out of RX buffers (%04x)\n", __FUNCTION__, event);
            for (i = 0; i < cpd->rx_ring_cnt; i++) {
                rxd = cpd->rx_ring + i*LANCE_TD_SIZE;

                rstat = _SU32(rxd, LANCE_RD_PTR);
                blen = _SU16(rxd, LANCE_RD_BLEN);
                mlen = _SU16(rxd, LANCE_RD_MLEN);
                db_printf(" %02d: 0x%08x:0x%04x:0x%04x\n", i, rstat, blen, mlen);
            }
#endif
            event &= LANCE_CSR_CSCR_EV_MASK;
            event |= LANCE_CSR_CSCR_MISS;
            put_reg(sc, LANCE_CSR_CSCR, event);
        }
        else {
#if DEBUG & 1
            db_printf("%s: Unknown interrupt: 0x%04x\n", __FUNCTION__, event);
#endif
            put_reg(sc, LANCE_CSR_CSCR, event);
        }
    }
}

// EOF if_lancepci.c
