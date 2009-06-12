//==========================================================================
//
//      dev/if_rhine.c
//
//      Ethernet device driver for VIA RHINE compatible controllers
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
// Author(s):    jskov, based on pcnet driver
// Contributors: gthomas, jskov, hmt
// Date:         2001-05-30
// Purpose:      
// Description:  hardware driver for VIA Rhine ethernet
//
// FIXME:        Make endian safe
//               Make use of virtual addressing for memory shared over PCI
//                (see _ADDR_MASK).          
//               Link failure not detected for some reason.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/devs_eth_via_rhine.h>
#include <pkgconf/io_eth_drivers.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>
#ifdef CYGPKG_NET
#include <pkgconf/net.h>
#include <cyg/kernel/kapi.h>
#include <net/if.h>  /* Needed for struct ifnet */
#include <pkgconf/io_eth_drivers.h>
#endif
#include CYGHWR_MEMORY_LAYOUT_H

#ifdef CYGPKG_IO_PCI
#include <cyg/io/pci.h>
#else
#error "Need PCI package here"
#endif

#define _BUF_SIZE 1544

#ifdef CYGPKG_INFRA_DEBUG
// Then we log, OOI, the number of times we get a bad packet number
// from the tx done fifo.
int rhine_txfifo_good = 0;
int rhine_txfifo_bad = 0;
#endif

#include "via_rhine.h"

#define __WANT_DEVS
#include CYGDAT_DEVS_ETH_VIA_RHINE_INL
#undef  __WANT_DEVS

static void rhine_poll(struct eth_drv_sc *sc);

// This ISR is called when the ethernet interrupt occurs
static cyg_uint32
rhine_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    struct rhine_priv_data *cpd = (struct rhine_priv_data *)data;

    DEBUG_FUNCTION();

    INCR_STAT( interrupts );

    cyg_drv_interrupt_mask(cpd->interrupt);
    cyg_drv_interrupt_acknowledge(cpd->interrupt);
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR);  // Run the DSR
}

static void
rhine_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    // This conditioning out is necessary because of explicit calls to this
    // DSR - which would not ever be called in the case of a polled mode
    // usage ie. in RedBoot.
#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
    struct rhine_priv_data* cpd = (struct rhine_priv_data *)data;
    struct cyg_netdevtab_entry *ndp = (struct cyg_netdevtab_entry *)(cpd->ndp);
    struct eth_drv_sc *sc = (struct eth_drv_sc *)(ndp->device_instance);

    // but here, it must be a *sc:
    eth_drv_dsr( vector, count, (cyg_addrword_t)sc );
#else
# ifndef CYGPKG_REDBOOT
#  error Empty Rhine ethernet DSR is compiled.  Is this what you want?
# endif
#endif
}

// The deliver function (ex-DSR)  handles the ethernet [logical] processing
static void
rhine_deliver(struct eth_drv_sc *sc)
{
    struct rhine_priv_data *cpd = (struct rhine_priv_data *)sc->driver_private;

    DEBUG_FUNCTION();

    // Service the interrupt:
    rhine_poll(sc);
    // Allow interrupts to happen again
    cyg_drv_interrupt_unmask(cpd->interrupt);
}

static int
rhine_int_vector(struct eth_drv_sc *sc)
{
    struct rhine_priv_data *cpd =
        (struct rhine_priv_data *)sc->driver_private;

    return (cpd->interrupt);
}

// ------------------------------------------------------------------------
// Physical interface
#if 0 // fix warning since this isn't actually used
static void
rhine_write_MII(struct rhine_priv_data *cpd, int id, int reg, cyg_uint16 value)
{
    cyg_uint8 stat;
    int i = 1000;

    // Wait for a previous access to complete (within reason)
    do {
        HAL_PCI_IO_READ_UINT8(cpd->base + RHINE_MIICR, stat);
    } while ((stat & (RHINE_MIICR_RCMD | RHINE_MIICR_WCMD)) && i-- > 0);
    
    HAL_PCI_IO_WRITE_UINT8(cpd->base + RHINE_MIICR, 0);
    HAL_PCI_IO_WRITE_UINT8(cpd->base + RHINE_PHYADR, id);
    HAL_PCI_IO_WRITE_UINT8(cpd->base + RHINE_MIIAD, reg);
    HAL_PCI_IO_WRITE_UINT16(cpd->base + RHINE_MIIDATA, value);
    HAL_PCI_IO_WRITE_UINT8(cpd->base + RHINE_MIICR, RHINE_MIICR_WCMD);
}
#endif

static int
rhine_read_MII(struct rhine_priv_data *cpd, int id, int reg)
{
    int i = 1000;
    cyg_uint8 stat;
    cyg_uint16 val;

    // Wait for a previous access to complete (within reason)
    do {
        HAL_PCI_IO_READ_UINT8(cpd->base + RHINE_MIICR, stat);
    } while ((stat & (RHINE_MIICR_RCMD | RHINE_MIICR_WCMD)) && i-- > 0);

    HAL_PCI_IO_WRITE_UINT8(cpd->base + RHINE_MIICR, 0);
    HAL_PCI_IO_WRITE_UINT8(cpd->base + RHINE_PHYADR, id);
    HAL_PCI_IO_WRITE_UINT8(cpd->base + RHINE_MIIAD, reg);
    HAL_PCI_IO_WRITE_UINT8(cpd->base + RHINE_MIICR, RHINE_MIICR_RCMD);

    i = 1000;
    do {
        HAL_PCI_IO_READ_UINT8(cpd->base + RHINE_MIICR, stat);
    } while ((stat & RHINE_MIICR_RCMD) && i-- > 0);

    HAL_PCI_IO_READ_UINT16(cpd->base + RHINE_MIIDATA, val);
    return val;
}

// ------------------------------------------------------------------------
// Memory management
//
// Simply carve off from the front of the PCI mapped window into real memory
static cyg_uint32 rhine_heap_size;
static cyg_uint8 *rhine_heap_base;
static cyg_uint8 *rhine_heap_free;

static void*
pciwindow_mem_alloc(int size)
{
    void *p_memory;
    int _size = size;

    CYG_ASSERT(
        (CYGHWR_VIA_RHINE_PCI_MEM_MAP_BASE <= (int)rhine_heap_free)
        &&
        ((CYGHWR_VIA_RHINE_PCI_MEM_MAP_BASE + 
          CYGHWR_VIA_RHINE_PCI_MEM_MAP_SIZE) > (int)rhine_heap_free)
        &&
        (0 < rhine_heap_size)
        &&
        (CYGHWR_VIA_RHINE_PCI_MEM_MAP_SIZE >= rhine_heap_size)
        &&
        (CYGHWR_VIA_RHINE_PCI_MEM_MAP_BASE == (int)rhine_heap_base),
        "Heap variables corrupted" );

    p_memory = (void *)0;
    size = (size + 3) & ~3;
    if ( (rhine_heap_free+size) < (rhine_heap_base+rhine_heap_size) ) {
        cyg_uint32 *p;
        p_memory = (void *)rhine_heap_free;
        rhine_heap_free += size;
        for ( p = (cyg_uint32 *)p_memory; _size > 0; _size -= 4 )
            *p++ = 0;
    }

#if DEBUG & 9
    diag_printf("Allocated %d bytes at %08x\n", size, p_memory);
#endif

    return p_memory;
}

static cyg_pci_match_func find_rhine_match_func;

static cyg_bool
find_rhine_match_func( cyg_uint16 v, cyg_uint16 d, cyg_uint32 c, void *p )
{
#if DEBUG & 9
    diag_printf("PCI match vendor %04x device %04x\n", v, d);
#endif
    return
        (0x1106 == v) &&                // vendor: VIA
        ((0x3065 == d) ||               // device: DL10030A
         (0x3043 == d));                // device: VT86C100A
}

static int
pci_init_find_rhines( void )
{
    cyg_pci_device_id devid;
    cyg_pci_device dev_info;
    cyg_uint16 cmd;
    int device_index;
    int found_devices = 0;

    DEBUG_FUNCTION();

#ifdef CYGARC_UNCACHED_ADDRESS
    CYG_ASSERT( CYGARC_UNCACHED_ADDRESS((CYG_ADDRWORD)CYGMEM_SECTION_pci_window) ==
                CYGHWR_VIA_RHINE_PCI_MEM_MAP_BASE,
      "PCI window configured does not match PCI memory section base" );
#else
    CYG_ASSERT( (CYG_ADDRWORD)CYGMEM_SECTION_pci_window ==
                CYGHWR_VIA_RHINE_PCI_MEM_MAP_BASE,
      "PCI window configured does not match PCI memory section base" );
#endif
    CYG_ASSERT( CYGMEM_SECTION_pci_window_SIZE ==
                CYGHWR_VIA_RHINE_PCI_MEM_MAP_SIZE,
        "PCI window configured does not match PCI memory section size" );

    if (
#ifdef CYGARC_UNCACHED_ADDRESS
         CYGARC_UNCACHED_ADDRESS((CYG_ADDRWORD)CYGMEM_SECTION_pci_window) !=
#else
         (CYG_ADDRWORD)CYGMEM_SECTION_pci_window !=
#endif
         CYGHWR_VIA_RHINE_PCI_MEM_MAP_BASE
         ||
         CYGMEM_SECTION_pci_window_SIZE !=
         CYGHWR_VIA_RHINE_PCI_MEM_MAP_SIZE ) {
#if DEBUG & 8
        diag_printf("pci_init_find_rhines(): PCI window misconfigured\n");
#endif
        return 0;
    }

    // First initialize the heap in PCI window'd memory
    rhine_heap_size = CYGHWR_VIA_RHINE_PCI_MEM_MAP_SIZE;
    rhine_heap_base = (cyg_uint8 *)CYGHWR_VIA_RHINE_PCI_MEM_MAP_BASE;
    rhine_heap_free = rhine_heap_base;

    cyg_pci_init();
#if DEBUG & 8
    diag_printf("Finished cyg_pci_init();\n");
#endif

    devid = CYG_PCI_NULL_DEVID;

    for (device_index = 0; 
         device_index < CYGNUM_DEVS_ETH_VIA_RHINE_DEV_COUNT;
         device_index++) {
        struct rhine_priv_data* cpd = rhine_priv_array[device_index];

        cpd->index = device_index;

        // See above for find_rhine_match_func - it selects any of several
        // variants.  This is necessary in case we have multiple mixed-type
        // devices on one board in arbitrary orders.
        if (cyg_pci_find_matching( &find_rhine_match_func, NULL, &devid )) {
#if DEBUG & 8
            diag_printf("eth%d = rhine\n", device_index);
#endif
            cyg_pci_get_device_info(devid, &dev_info);

            cpd->interrupt_handle = 0; // Flag not attached.
            if (cyg_pci_translate_interrupt(&dev_info, &cpd->interrupt)) {
#if DEBUG & 8
                diag_printf(" Wired to HAL vector %d\n", cpd->interrupt);
#endif
                cyg_drv_interrupt_create(
                    cpd->interrupt,
                    1,                  // Priority - unused
                    (cyg_addrword_t)cpd,// Data item passed to ISR & DSR
                    rhine_isr,          // ISR
                    rhine_dsr,          // DSR
                    &cpd->interrupt_handle, // handle to intr obj
                    &cpd->interrupt_object ); // space for int obj

                cyg_drv_interrupt_attach(cpd->interrupt_handle);

                // Don't unmask the interrupt yet, that could get us into a
                // race.
            }
            else {
                cpd->interrupt = 0;
#if DEBUG & 8
                diag_printf(" Does not generate interrupts.\n");
#endif
            }

            if (cyg_pci_configure_device(&dev_info)) {
#if DEBUG & 8
                int i;
                diag_printf("Found device on bus %d, devfn 0x%02x:\n",
                          CYG_PCI_DEV_GET_BUS(devid),
                          CYG_PCI_DEV_GET_DEVFN(devid));

                if (dev_info.command & CYG_PCI_CFG_COMMAND_ACTIVE) {
                    diag_printf(" Note that board is active. Probed"
                              " sizes and CPU addresses invalid!\n");
                }
                diag_printf(" Vendor    0x%04x", dev_info.vendor);
                diag_printf("\n Device    0x%04x", dev_info.device);
                diag_printf("\n Command   0x%04x, Status 0x%04x\n",
                          dev_info.command, dev_info.status);
                
                diag_printf(" Class/Rev 0x%08x", dev_info.class_rev);
                diag_printf("\n Header 0x%02x\n", dev_info.header_type);

                diag_printf(" SubVendor 0x%04x, Sub ID 0x%04x\n",
                          dev_info.header.normal.sub_vendor, 
                          dev_info.header.normal.sub_id);

                for(i = 0; i < CYG_PCI_MAX_BAR; i++) {
                    diag_printf(" BAR[%d]    0x%08x /", i, dev_info.base_address[i]);
                    diag_printf(" probed size 0x%08x / CPU addr 0x%08x\n",
                              dev_info.base_size[i], dev_info.base_map[i]);
                }
                diag_printf(" eth%d configured\n", device_index);
#endif
                found_devices++;
                cpd->found = 1;
                cpd->active = 0;
                cpd->devid = devid;
                cpd->base = (unsigned char*) dev_info.base_map[0];
#if DEBUG & 8
                diag_printf(" I/O address = 0x%08x\n", cpd->base);
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

                // Extra init code needed for D-Link controller. This
                // is snuffed from the Linux driver and was provided
                // by D-Link. I've been unable to find documentation
                // for the part.
                if (0x3065 == dev_info.device) {
                    cyg_uint8 tmp;

#if DEBUG & 8
                    diag_printf("Pre-reset init code for D-Link.\n");
#endif
                    HAL_PCI_IO_READ_UINT8(cpd->base+RHINE_STICKYHW, tmp);
                    tmp &= 0xfc;
                    HAL_PCI_IO_WRITE_UINT8(cpd->base+RHINE_STICKYHW, tmp);
                    
                    HAL_PCI_IO_WRITE_UINT8(cpd->base+RHINE_WOL_CG_CLR, 0x80);
                    HAL_PCI_IO_WRITE_UINT8(cpd->base+RHINE_WOL_CR_CLR, 0xff);
                    HAL_PCI_IO_WRITE_UINT8(cpd->base+RHINE_PWR_CSR_CLR, 0xff);
                }

                // Now the PCI part of the device is configured, reset
                // it. This should make it safe to enable the
                // interrupt
                HAL_PCI_IO_WRITE_UINT8(cpd->base+RHINE_CR1, RHINE_CR1_SRST);

                // Reload ESA from EEPROM
                {
                    cyg_uint8 tmp;
                    int i;

#if DEBUG & 8
                    diag_printf("Reload ESA from EEPROM...");
#endif
                    HAL_PCI_IO_WRITE_UINT8(cpd->base+RHINE_EECSR, 0x20);
                    for (i = 0; i < 150; i++) {
                        HAL_PCI_IO_READ_UINT8(cpd->base+RHINE_EECSR, tmp);
                        if (!(tmp & 0x20)) {
                            break;
                        }
                    }
#if DEBUG & 8
                    if (tmp & 0x20)
                        diag_printf("Timed out\n");
                    else
                        diag_printf("Done\n");
#endif
                }

                // This is the indicator for "uses an interrupt"
                if (cpd->interrupt_handle != 0) {
                    cyg_drv_interrupt_acknowledge(cpd->interrupt);
                    cyg_drv_interrupt_unmask(cpd->interrupt);
#if DEBUG & 8
                    diag_printf(" Enabled interrupt %d\n", cpd->interrupt);
#endif
                }
#if DEBUG & 8
                diag_printf(" **** Device enabled for I/O and Memory "
                            "and Bus Master\n");
#endif
            }
            else {
                cpd->found = 0;
                cpd->active = 0;
#if DEBUG & 8
                diag_printf("Failed to configure device %d\n", device_index);
#endif
            }
        }
        else {
            cpd->found = 0;
            cpd->active = 0;
#if DEBUG & 8
            diag_printf("eth%d not found\n", device_index);
#endif
        }
    }

    if (0 == found_devices)
        return 0;

    return 1;
}

static bool 
via_rhine_init(struct cyg_netdevtab_entry *tab)
{
    static int initialized = 0; // only probe PCI et al *once*
    struct eth_drv_sc *sc = (struct eth_drv_sc *)tab->device_instance;
    struct rhine_priv_data *cpd =
        (struct rhine_priv_data *)sc->driver_private;
    cyg_uint8 *d, *p, *p_next;
    int i;
    cyg_addrword_t ba;

    DEBUG_FUNCTION();

    if ( 0 == initialized++ ) {
        // then this is the first time ever:
        if ( ! pci_init_find_rhines() ) {
#if DEBUG & 8
            diag_printf( "pci_init_find_rhines failed" );
#endif
            return false;
        }
    }

    // If this device is not present, exit
    if (0 == cpd->found)
        return 0;

#if DEBUG & 8
    diag_printf( "Rhine device SC %08x CPD %08x\n", sc, cpd);
#endif

    // Look for physical MII device
    for (i = 0; i < 32; i++) {
        cyg_uint16 mii_status = rhine_read_MII(cpd, i, MII_BMSR);
        if (mii_status != 0x0000 && mii_status != 0xffff) {
            cpd->phys_id = i;
#if DEBUG & 8
            diag_printf("Found MII interface at id %d, status %04x, adv 0x%04x, link 0x%04x\n",
                        cpd->phys_id, mii_status, rhine_read_MII(cpd,i,4), rhine_read_MII(cpd,i,5));
#endif
            break;
        }
    }
#if DEBUG & 8
    if (i == 32)
        diag_printf("No MII interface found!");
#endif

    // Prepare ESA
    if (cpd->hardwired_esa) {
        // Force the NIC to use the specified ESA
        p = cpd->base + RHINE_PAR0;
        for (i = 0; i < 6; i++)
            *p++ = cpd->esa[i];
    } else {
        // Use the address from the serial EEPROM
        p = cpd->base + RHINE_PAR0;
        for (i = 0; i < 6; i++)
            cpd->esa[i] = *p++;
    }
#if DEBUG & 8
    diag_printf("RHINE - %s ESA: %02x:%02x:%02x:%02x:%02x:%02x\n",
                (cpd->hardwired_esa) ? "static" : "eeprom",
                cpd->esa[0],
                cpd->esa[1],
                cpd->esa[2],
                cpd->esa[3],
                cpd->esa[4],
                cpd->esa[5] );
#endif

    // Prepare RX and TX rings
    p = cpd->rx_ring = (cyg_uint8*) CYGARC_UNCACHED_ADDRESS(pciwindow_mem_alloc((1<<cpd->rx_ring_log_cnt)*RHINE_RD_SIZE));
    d = cpd->rx_buffers = (cyg_uint8*) CYGARC_UNCACHED_ADDRESS(pciwindow_mem_alloc(_BUF_SIZE*cpd->rx_ring_cnt));
    for (i = 0; i < cpd->rx_ring_cnt; i++) {
        p_next = p + RHINE_RD_SIZE;
        HAL_PCI_CPU_TO_BUS((cyg_uint32)d, ba);
        _SU32(p, RHINE_RDES2) = ba;
        _SU32(p, RHINE_RDES1) = _BUF_SIZE;
        HAL_PCI_CPU_TO_BUS((cyg_uint32)p_next, ba);
        _SU32(p, RHINE_RDES3) = ba;
        _SU32(p, RHINE_RDES0) = RHINE_RDES0_OWN;
#if DEBUG & 8
        diag_printf("Set RDES at 0x%08lx to 0x%08x 0x%08x 0x%08x 0x%08x\n",
                    (unsigned long)p,
                    _SU32(p, RHINE_RDES0), _SU32(p, RHINE_RDES1),
                    _SU32(p, RHINE_RDES2), _SU32(p, RHINE_RDES3));
#endif
        p = p_next;
        d += _BUF_SIZE;
    }
    // last entry wraps to the first
    p -= RHINE_RD_SIZE;
    HAL_PCI_CPU_TO_BUS((cyg_uint32)cpd->rx_ring, ba);
    _SU32(p, RHINE_RDES3) = ba;
#if DEBUG & 8
    diag_printf("Set RDES at 0x%08lx to 0x%08x 0x%08x 0x%08x 0x%08x\n",
                (unsigned long)p,
                _SU32(p, RHINE_RDES0), _SU32(p, RHINE_RDES1),
                _SU32(p, RHINE_RDES2), _SU32(p, RHINE_RDES3));
#endif
    cpd->rx_ring_next = 0;
    // CPU to PCI space translation
    HAL_PCI_CPU_TO_BUS((cyg_uint32)cpd->rx_ring, ba);
    HAL_PCI_IO_WRITE_UINT32(cpd->base + RHINE_CUR_RX, ba);

    p = cpd->tx_ring = (cyg_uint8*) CYGARC_UNCACHED_ADDRESS(pciwindow_mem_alloc((1<<cpd->tx_ring_log_cnt)*RHINE_TD_SIZE));
    d = cpd->tx_buffers = (cyg_uint8*) CYGARC_UNCACHED_ADDRESS(pciwindow_mem_alloc(_BUF_SIZE*cpd->tx_ring_cnt));
    for (i = 0; i < cpd->tx_ring_cnt; i++) {

        _SU32(p, RHINE_TDES0) = 0;
        _SU32(p, RHINE_TDES1) = (RHINE_TDES1_IC|RHINE_TDES1_EDP|RHINE_TDES1_STP|RHINE_TDES1_C);
        HAL_PCI_CPU_TO_BUS((cyg_uint32)d, ba);
        _SU32(p, RHINE_TDES2) = ba;
        HAL_PCI_CPU_TO_BUS((cyg_uint32)(p + RHINE_TD_SIZE), ba);
        _SU32(p, RHINE_TDES3) = ba;
#if DEBUG & 8
        diag_printf("Set TDES at 0x%08lx to 0x%08x 0x%08x 0x%08x 0x%08x\n",
                    (unsigned long)p,
                    _SU32(p, RHINE_TDES0), _SU32(p, RHINE_TDES1),
                    _SU32(p, RHINE_TDES2), _SU32(p, RHINE_TDES3));
#endif
        p += RHINE_TD_SIZE;
        d += _BUF_SIZE;
    }

    // last entry wraps to the first
    p -= RHINE_TD_SIZE;
    HAL_PCI_CPU_TO_BUS((cyg_uint32)cpd->tx_ring, ba);
    _SU32(p, RHINE_TDES3) = ba;
#if DEBUG & 8
    diag_printf("Set TDES at 0x%08lx to 0x%08x 0x%08x 0x%08x 0x%08x\n",
                (unsigned long)p,
                _SU32(p, RHINE_TDES0), _SU32(p, RHINE_TDES1),
                _SU32(p, RHINE_TDES2), _SU32(p, RHINE_TDES3));
#endif
    cpd->tx_ring_free = cpd->tx_ring_alloc = cpd->tx_ring_owned = 0;
    HAL_PCI_CPU_TO_BUS((cyg_uint32)cpd->tx_ring, ba);
    HAL_PCI_IO_WRITE_UINT32(cpd->base + RHINE_CUR_TX, ba);

    cpd->txbusy = 0;

#if DEBUG & 9
    {
        cyg_uint8 tmp1, tmp2;
        HAL_PCI_IO_READ_UINT8(cpd->base+RHINE_CR0, tmp1); 
        HAL_PCI_IO_READ_UINT8(cpd->base+RHINE_CR1, tmp2);
        diag_printf("CR0: %02x  CR1: %02x\n", tmp1, tmp2);
    }
#endif

    // and record the net dev pointer
    cpd->ndp = (void *)tab;

    // Initialize upper level driver
    (sc->funs->eth_drv->init)(sc, cpd->esa);

#if DEBUG & 9
    diag_printf("Done\n");
#endif
    return true;
}

static void
rhine_stop(struct eth_drv_sc *sc)
{
    struct rhine_priv_data *cpd =
        (struct rhine_priv_data *)sc->driver_private;

    DEBUG_FUNCTION();

    // Stop chip
    HAL_PCI_IO_WRITE_UINT8(cpd->base+RHINE_CR0, RHINE_CR0_STOP);
}

//
// This function is called to "start up" the interface.  It may be called
// multiple times, even when the hardware is already running.  It will be
// called whenever something "hardware oriented" changes and should leave
// the hardware ready to send/receive packets.
//
static void
rhine_start(struct eth_drv_sc *sc, unsigned char *esa, int flags)
{
#ifdef CYGPKG_NET
    struct ifnet *ifp = &sc->sc_arpcom.ac_if;
#endif
    struct rhine_priv_data *cpd =
        (struct rhine_priv_data *)sc->driver_private;

    DEBUG_FUNCTION();
    // Disable device
    HAL_PCI_IO_WRITE_UINT8(cpd->base + RHINE_CR0, RHINE_CR0_STOP);
    // Ack old interrupts
    HAL_PCI_IO_WRITE_UINT16(cpd->base + RHINE_ISR, 0xffff);
    // Enable interrupts
    HAL_PCI_IO_WRITE_UINT16(cpd->base + RHINE_IMR, RHINE_IMR_INIT);
    // Enable duplex
    HAL_PCI_IO_WRITE_UINT8(cpd->base+RHINE_CR1, RHINE_CR1_DPOLL /* | RHINE_CR1_FDX*/);
    // Accept broadcast, multicast and small packets
    HAL_PCI_IO_WRITE_UINT8(cpd->base + RHINE_RCR, RHINE_RCR_AB | RHINE_RCR_AM | RHINE_RCR_AR);
    // Tweak some magic (undocumented) parameters
    HAL_PCI_IO_WRITE_UINT8(cpd->base+RHINE_BCR0, RHINE_BCR0_MAGIC_INIT);
    HAL_PCI_IO_WRITE_UINT8(cpd->base+RHINE_BCR1, RHINE_BCR1_MAGIC_INIT);

#if 1 // FIXME
    HAL_PCI_IO_WRITE_UINT8(cpd->base+RHINE_TCR, 0x20);
#endif

#ifdef CYGPKG_NET
    if (( 0
#ifdef ETH_DRV_FLAGS_PROMISC_MODE
         != (flags & ETH_DRV_FLAGS_PROMISC_MODE)
#endif
        ) || (ifp->if_flags & IFF_PROMISC)
        ) {
        // Then we select promiscuous mode.
        cyg_uint8 rcr;
        HAL_PCI_IO_READ_UINT8(cpd->base + RHINE_RCR, rcr);
        rcr |= RHINE_RCR_PRO;
        HAL_PCI_IO_WRITE_UINT8(cpd->base + RHINE_RCR, rcr);
    }
#endif
    // Enable device
    HAL_PCI_IO_WRITE_UINT8(cpd->base + RHINE_CR0, RHINE_CR0_STRT | RHINE_CR0_RXON | RHINE_CR0_TXON);
}

//
// This routine is called to perform special "control" opertions
//
static int
rhine_control(struct eth_drv_sc *sc, unsigned long key,
               void *data, int data_length)
{
    cyg_uint8 *esa = (cyg_uint8 *)data;
    int i, res;
    cyg_uint8 reg, old_stat;
    struct rhine_priv_data *cpd =
        (struct rhine_priv_data *)sc->driver_private;

    DEBUG_FUNCTION();

    // Stop the controller while accessing (possibly altering) registers
    HAL_PCI_IO_READ_UINT8(cpd->base + RHINE_CR0, old_stat);
    reg = old_stat;
    reg |= RHINE_CR0_STOP;
    reg &= ~RHINE_CR0_STRT;
    HAL_PCI_IO_WRITE_UINT8(cpd->base + RHINE_CR0, reg);

    res = 0;                            // expect success
    switch (key) {
    case ETH_DRV_SET_MAC_ADDRESS:
#if DEBUG & 9
        diag_printf("RHINE - set ESA: %02x:%02x:%02x:%02x:%02x:%02x\n",
                esa[0],
                esa[1],
                esa[2],
                esa[3],
                esa[4],
                esa[5] );
#endif // DEBUG

        for ( i = 0; i < sizeof(cpd->esa);  i++ ) {
            cpd->esa[i] = esa[i];
            HAL_PCI_IO_WRITE_UINT8(cpd->base + RHINE_PAR0 + i, esa[i]);
        }
        break;

#ifdef ETH_DRV_GET_MAC_ADDRESS
    case ETH_DRV_GET_MAC_ADDRESS:
        // Extract the MAC address that is in the chip, and tell the
        // system about it.
        for (i = 0;  i < sizeof(cpd->esa);  i++) {
            HAL_PCI_IO_READ_UINT8(cpd->base + RHINE_PAR0 + i, esa[i]);
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
        cyg_uint8 reg;
        struct ether_drv_stats *p = (struct ether_drv_stats *)data;
        // Chipset entry is no longer supported; RFC1573.
        for ( i = 0; i < SNMP_CHIPSET_LEN; i++ )
            p->snmp_chipset[i] = 0;

        // This perhaps should be a config opt, so you can make up your own
        // description, or supply it from the instantiation.
        strcpy( p->description, "VIA Rhine" );
        // CYG_ASSERT( 48 > strlen(p->description), "Description too long" );

        HAL_PCI_IO_READ_UINT8(cpd->base + RHINE_MIISR, reg);
        if (reg & RHINE_MIISR_LNKFL) {
            p->operational = 2;         // LINK DOWN
            p->duplex = 1;              // UNKNOWN
            p->speed = 0;
        }
        else {
            p->operational = 3;         // LINK UP
            p->speed = (reg & RHINE_MIISR_SPEED) ? 10 * 1000000 : 100 * 1000000;
            HAL_PCI_IO_READ_UINT8(cpd->base + RHINE_CR1, reg);
            if (reg & RHINE_CR1_FDX)
                p->duplex = 3;              // 3 = DUPLEX
            else
                p->duplex = 2;              // 2 = SIMPLEX
        }

#ifdef KEEP_STATISTICS
        {
            struct via_rhine_stats *ps = &(cpd->stats);

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

        p->tx_queue_len = 1;
        break;
    }
#endif
    default:
        res = 1;
        break;
    }

    // Restore controller state
    HAL_PCI_IO_WRITE_UINT8(cpd->base + RHINE_CR0, old_stat);

    return res;
}

//
// This routine is called to see if it is possible to send another packet.
// It will return non-zero if a transmit is possible, zero otherwise.
//
static int
rhine_can_send(struct eth_drv_sc *sc)
{
    cyg_uint8 stat;
    struct rhine_priv_data *cpd =
        (struct rhine_priv_data *)sc->driver_private;

    DEBUG_FUNCTION();

    // This MII read forces the MIISR to get updated
    (void) rhine_read_MII(cpd, cpd->phys_id, MII_BMSR);
    HAL_PCI_IO_READ_UINT8(cpd->base + RHINE_MIISR, stat);
    if (stat & RHINE_MIISR_LNKFL) {
#if DEBUG & 1
        diag_printf("*** Link failure\n");
#endif
        return false;  // Link not connected
    }

    return (cpd->txbusy == 0);
}

//
// This routine is called to send data to the hardware.
static void 
rhine_send(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len, 
            int total_len, unsigned long key)
{
    struct rhine_priv_data *cpd = 
        (struct rhine_priv_data *)sc->driver_private;
    int i, len, plen, ring_entry;

    cyg_uint8* sdata = NULL;
    cyg_uint8 *d, *buf, *txd;
    cyg_uint16 status;
    cyg_uint8 cr0;

    DEBUG_FUNCTION();

    INCR_STAT( tx_count );

    // Worry about the engine stopping.
    HAL_PCI_IO_READ_UINT8(cpd->base + RHINE_CR0, status);
    if ( 0 == (RHINE_CR0_STRT & status) ) {
#if DEBUG & 1
        diag_printf("%s: ENGINE RESTART: status %04x\n", __FUNCTION__, status);
#endif
        status &= ~RHINE_CR0_STOP;
        status |= RHINE_CR0_STRT;
        HAL_PCI_IO_WRITE_UINT8(cpd->base + RHINE_CR0, status);
    }

    cpd->txbusy = 1;
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
            diag_printf("%s: Allocation failed! Retrying...\n", __FUNCTION__ );
#endif
            continue;
        }

        cpd->tx_ring_free++;
        cpd->tx_ring_owned++;
        if (cpd->tx_ring_free == cpd->tx_ring_cnt)
            cpd->tx_ring_free = 0;
    } while (0);

    txd = cpd->tx_ring + ring_entry*RHINE_TD_SIZE;
    buf = cpd->tx_buffers + ring_entry*_BUF_SIZE;
    CYG_ASSERT(0 == (_SU32(txd, RHINE_TDES0) & RHINE_TDES0_OWN),
               "TX descriptor not free");

#if DEBUG & 4
    diag_printf("##Tx descriptor index %d TDES %08x buffer %08x\n", 
                ring_entry, txd, buf);
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
    
    // Annoyingly the chip doesn't pad to minimal packet size, so do
    // that by steam
    if (plen < 60) {
        plen = 60;
#if DEBUG & 4
        diag_printf("Padded %d bytes packet to 60 bytes\n", plen);
#endif
    }        

    CYG_ASSERT( (plen & RHINE_TDES1_TLNG_mask) == plen, "packet too long");
    CYG_ASSERT( (plen & RHINE_TDES1_TLNG_mask) >= 60, "packet too short");
    _SU32(txd, RHINE_TDES1) &= ~RHINE_TDES1_TLNG_mask;
    _SU32(txd, RHINE_TDES1) |= plen;
    _SU32(txd, RHINE_TDES0) = RHINE_TDES0_OWN;

#if DEBUG & 1 // FIXME
    diag_printf("Before TX: Desc (@0x%08lx) %08x %08x %08x %08x\n Next (@0x%08lx) %08x %08x %08x %08x\n",
                (unsigned long) txd,
                _SU32(txd, RHINE_TDES0), _SU32(txd, RHINE_TDES1),
                _SU32(txd, RHINE_TDES2), _SU32(txd, RHINE_TDES3),
                (  (unsigned long)txd)+0x10,
                _SU32(txd, (0x10+RHINE_TDES0)), _SU32(txd, (0x10+RHINE_TDES1)),
                _SU32(txd,(0x10+ RHINE_TDES2)), _SU32(txd,(0x10+ RHINE_TDES3)));
#endif

    // Ack TX empty int
    HAL_PCI_IO_WRITE_UINT16(cpd->base + RHINE_ISR, RHINE_ISR_PTX);
    // Set transmit demand
    HAL_PCI_IO_READ_UINT8(cpd->base + RHINE_CR0, cr0);
    cr0 |= RHINE_CR0_TDMD;
    HAL_PCI_IO_WRITE_UINT8(cpd->base + RHINE_CR0, cr0);

#if DEBUG & 1
    HAL_PCI_IO_READ_UINT16(cpd->base + RHINE_ISR, status);
    diag_printf("%s:END: ints at TX: %04x\n", __FUNCTION__, status);
#endif

}

static void
rhine_TxEvent(struct eth_drv_sc *sc, int stat)
{
     struct rhine_priv_data *cpd =
        (struct rhine_priv_data *)sc->driver_private;
    int success = 1;
    cyg_uint8 *txd;
    cyg_uint8 status;

    DEBUG_FUNCTION();

    INCR_STAT( tx_complete );

    txd = cpd->tx_ring + cpd->tx_ring_alloc*RHINE_TD_SIZE;
#if DEBUG & 4
    diag_printf("##Tx packet %d freed %08x %08x!\n", cpd->tx_ring_alloc, txd, _SU32(txd, RHINE_TDES0) );
#endif
    if ((_SU32(txd, RHINE_TDES0) & RHINE_TDES0_OWN)) {
#if DEBUG & 1
        diag_printf("%s: got TX completion when buffer is still owned\n", __FUNCTION__);
#endif
        // first dirty ring entry not freed - wtf?
    }
    cpd->tx_ring_alloc++;
    if (cpd->tx_ring_alloc == cpd->tx_ring_cnt)
        cpd->tx_ring_alloc = 0;
    cpd->tx_ring_owned--;

#ifdef KEEP_STATISTICS
    {
        cyg_uint32 reg = _SU32(txd, RHINE_TDES0);
        int collisions;

        // Covering each bit in turn...
        if ( reg & RHINE_TDES0_TXOK ) INCR_STAT( tx_good );
        if ( reg & RHINE_TDES0_CRS ) INCR_STAT( tx_carrier_loss );
        if ( reg & RHINE_TDES0_OWC ) INCR_STAT( tx_late_collisions );
        if ( reg & RHINE_TDES0_ABT ) INCR_STAT( tx_max_collisions );

        if ( reg & RHINE_TDES0_DFR ) INCR_STAT( tx_deferred );

        collisions = ((reg & RHINE_TDES0_NCR_mask) >> RHINE_TDES0_NCR_shift);
        if (1 == collisions)
            INCR_STAT( tx_single_collisions );
        else if (1 < collisions)
            INCR_STAT( tx_mult_collisions );

        cpd->stats.tx_total_collisions = 
            cpd->stats.tx_late_collisions + 
            cpd->stats.tx_max_collisions + 
            cpd->stats.tx_mult_collisions + 
            cpd->stats.tx_single_collisions;
    }
#endif // KEEP_STATISTICS

    // We do not really care about Tx failure.  Ethernet is not a reliable
    // medium.  But we do care about the TX engine stopping.
    HAL_PCI_IO_READ_UINT8(cpd->base + RHINE_CR0, status);
    if ( 0 == (RHINE_CR0_STRT & status) ) {
#if DEBUG & 1
        diag_printf("%s: ENGINE RESTART: status %04x\n", __FUNCTION__, status);
#endif
        status &= ~RHINE_CR0_STOP;
        status |= RHINE_CR0_STRT;
        HAL_PCI_IO_WRITE_UINT8(cpd->base + RHINE_CR0, status);
        success = 0; // And treat this as an error...
    }

    if ( cpd->txbusy ) {
        cpd->txbusy = 0;
        (sc->funs->eth_drv->tx_done)(sc, cpd->txkey, success);
    }

    // Ack TX interrupt set
    HAL_PCI_IO_WRITE_UINT16(cpd->base + RHINE_ISR, RHINE_ISR_PTX);
}

//
// This function is called when a packet has been received.  Its job is
// to prepare to unload the packet from the hardware.  Once the length of
// the packet is known, the upper layer of the driver can be told.  When
// the upper layer is ready to unload the packet, the internal function
// 'rhine_recv' will be called to actually fetch it from the hardware.
//
static void
rhine_RxEvent(struct eth_drv_sc *sc)
{
    struct rhine_priv_data *cpd = 
        (struct rhine_priv_data *)sc->driver_private;
    cyg_uint8 *rxd;
    cyg_uint32 rstat;
    cyg_uint16 ints, len, mask;

    DEBUG_FUNCTION();

    HAL_PCI_IO_READ_UINT16(cpd->base + RHINE_ISR, ints);
#if DEBUG & 1
    diag_printf("RxEvent - CSR: 0x%04x\n", ints);
#endif
    if ( 0 == (RHINE_ISR_PRX & ints) )
        // Then there's no RX event pending
        return;

    // Mask interrupt
    HAL_PCI_IO_READ_UINT16(cpd->base + RHINE_IMR, mask);
    mask &= ~RHINE_IMR_PRX;
    HAL_PCI_IO_WRITE_UINT16(cpd->base + RHINE_IMR, mask);

    while (1) {
        // Get state of next (supposedly) full ring entry
        cpd->rxpacket = cpd->rx_ring_next;
        rxd = cpd->rx_ring + cpd->rxpacket*RHINE_RD_SIZE;
        rstat = _SU32(rxd, RHINE_RDES0);

        // Keep going until we hit an entry that is owned by the
        // controller.
        if (rstat & RHINE_RDES0_OWN) {
#ifdef CYGDBG_USE_ASSERTS
            // Sanity check of queue
            int i;
            for (i = 0; i < cpd->rx_ring_cnt; i++) {
                rxd = cpd->rx_ring + i*RHINE_RD_SIZE;
                rstat = _SU32(rxd, RHINE_RDES0);

                if (!(rstat & RHINE_RDES0_OWN)) {
                    int i;
                    cyg_uint32 rstat;
                    cyg_uint8* rxd;

                    diag_printf("####Rx %s Inconsistent RX state - next was %d\n", 
                                __FUNCTION__, cpd->rx_ring_next);
                    for (i = 0; i < cpd->rx_ring_cnt; i++) {
                        rxd = cpd->rx_ring + i*RHINE_RD_SIZE;
                        rstat = _SU32(rxd, RHINE_RDES0);
                        diag_printf("#### %02d: 0x%08x\n", i, rstat);
                    }
                }
                break;
            }
#endif
            break;
        }

#if DEBUG & 4
        diag_printf("##Rx packet %d RDES %08x stat %08x\n", 
                    cpd->rxpacket, rxd, rstat);
#endif

        // Increment counts
        INCR_STAT( rx_count );
        cpd->rx_ring_next++;
        if (cpd->rx_ring_next == cpd->rx_ring_cnt) cpd->rx_ring_next = 0;

        len = (rstat & RHINE_RDES0_FLNG_mask) >> RHINE_RDES0_FLNG_shift;

#ifdef KEEP_STATISTICS
        if ( rstat & RHINE_RDES0_CRC ) INCR_STAT( rx_crc_errors );
        if ( rstat & RHINE_RDES0_FAE ) INCR_STAT( rx_align_errors );
        if ( rstat & RHINE_RDES0_LONG ) INCR_STAT( rx_too_long_frames );
#endif // KEEP_STATISTICS

        if (RHINE_RDES0_RXOK & rstat) {
            // It's OK
            INCR_STAT( rx_good );

#if DEBUG & 1
            diag_printf("RxEvent good rx - stat: 0x%08x, len: 0x%04x\n", rstat, len);
            {
                    unsigned char *buf = cpd->rx_buffers + cpd->rxpacket*_BUF_SIZE;

                    int i;
                    diag_printf("RDES: %08x %08x %08x %08x\n", 
                                _SU32(rxd, RHINE_RDES0), _SU32(rxd, RHINE_RDES1),
                                _SU32(rxd, RHINE_RDES2), _SU32(rxd, RHINE_RDES3));

                    diag_printf("Packet data at %p\n", buf);
                    for (i=0;i<len;i++) diag_printf("%02x ", buf[i]);
                    diag_printf("\n");
            }
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
            diag_printf("RxEvent - No RX bit: stat: 0x%08x, len: 0x%04x\n",
                        rstat, len);
#endif
        }

        // Free packet (clear all status flags, and set OWN)
        _SU32(rxd, RHINE_RDES0) = RHINE_RDES0_OWN;
    }

    // Ack RX int
    HAL_PCI_IO_WRITE_UINT16(cpd->base + RHINE_ISR, RHINE_ISR_PRX);
    // And reenable the interrupt
    HAL_PCI_IO_READ_UINT16(cpd->base + RHINE_IMR, mask);
    mask |= RHINE_IMR_PRX;
    HAL_PCI_IO_WRITE_UINT16(cpd->base + RHINE_IMR, mask);
}

//
// This function is called as a result of the "eth_drv_recv()" call above.
// Its job is to actually fetch data for a packet from the hardware once
// memory buffers have been allocated for the packet.  Note that the buffers
// may come in pieces, using a scatter-gather list.  This allows for more
// efficient processing in the upper layers of the stack.
//
static void
rhine_recv(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len)
{
    struct rhine_priv_data *cpd = 
        (struct rhine_priv_data *)sc->driver_private;
    int i, mlen=0, plen;
    cyg_uint8 *data, *rxd, *buf;

    DEBUG_FUNCTION();

    rxd = cpd->rx_ring + cpd->rxpacket*RHINE_RD_SIZE;
    buf = cpd->rx_buffers + cpd->rxpacket*_BUF_SIZE;

    INCR_STAT( rx_deliver );

    plen = (_SU32(rxd, RHINE_RDES0) & RHINE_RDES0_FLNG_mask) >> RHINE_RDES0_FLNG_shift;

    for (i = 0;  i < sg_len;  i++) {
        data = (cyg_uint8*)sg_list[i].buf;
        mlen = sg_list[i].len;

#if DEBUG & 1
        diag_printf("%s : mlen %x, plen %x\n", __FUNCTION__, mlen, plen);
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
rhine_poll(struct eth_drv_sc *sc)
{
    struct rhine_priv_data *cpd = 
        (struct rhine_priv_data *)sc->driver_private;
    cyg_uint16 event, mask;
    static volatile bool locked = false;

//    DEBUG_FUNCTION();

    while (1) {
        // Get the (unmasked) requests
        HAL_PCI_IO_READ_UINT16(cpd->base + RHINE_ISR, event);
        HAL_PCI_IO_READ_UINT16(cpd->base + RHINE_IMR, mask);

        event &= mask;

        if (0 == event)
            break;

        if (event & RHINE_ISR_PRX) {
            rhine_RxEvent(sc);
        }
        else if (event & RHINE_ISR_PTX) {
            rhine_TxEvent(sc, event);
        } 
        else if (event & RHINE_ISR_RU) {
#if DEBUG & 1
            int i;
            cyg_uint32 rstat;
            cyg_uint8* rxd;
            struct rhine_priv_data *cpd = 
                (struct rhine_priv_data *)sc->driver_private;

            diag_printf("%s: Ran out of RX buffers (%04x)\n", __FUNCTION__, event);
            for (i = 0; i < cpd->rx_ring_cnt; i++) {
                rxd = cpd->rx_ring + i*RHINE_RD_SIZE;
                
                rstat = _SU32(rxd, RHINE_RDES0);
                diag_printf(" %02d: 0x%08x\n", i, rstat);
            }
#endif
            HAL_PCI_IO_WRITE_UINT16(cpd->base + RHINE_ISR, RHINE_ISR_RU);
        }
        else {
#if DEBUG & 1
            diag_printf("%s: Unknown interrupt: 0x%04x\n", __FUNCTION__, event);
#endif
            // Clear unhandled interrupts and hope for the best
            // This should never happen though, since we only enable
            // the sources we handle.
            HAL_PCI_IO_WRITE_UINT16(cpd->base + RHINE_ISR, event);
        }
    }

    locked = false;
}
// EOF if_rhine.c
