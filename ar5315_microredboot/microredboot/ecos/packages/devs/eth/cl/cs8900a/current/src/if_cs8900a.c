//==========================================================================
//
//      dev/if_cs8900a.c
//
//      Device driver for Cirrus Logic CS8900A ethernet controller
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
// Author(s):    gthomas
// Contributors: gthomas, jskov
// Date:         2001-11-02
// Purpose:      
// Description:  Driver for CS8900 ethernet controller
//
// Note:         Platform can define CYGSEM_DEVS_ETH_CL_CS8900A_NOINTS
//               to get a timer thread polling instead of interupt based
//               operation.
//
// Note:         Driver will need some changes to support multiple instances
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#ifdef CYGPKG_KERNEL
#include <cyg/kernel/kapi.h>
#endif
#include <pkgconf/io_eth_drivers.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_endian.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/drv_api.h>
#undef __ECOS
#define __ECOS
#include <cyg/io/eth/eth_drv.h>
#include <cyg/io/eth/netdev.h>

#include <cyg/io/cs8900.h>

#define __WANT_DEVS
#include CYGDAT_DEVS_ETH_CL_CS8900A_INL
#undef __WANT_DEVS

// NOINTS operation only relevant when the NET package is loaded
#if !defined(CYGPKG_NET) || !defined(CYGPKG_KERNEL)
# undef CYGSEM_DEVS_ETH_CL_CS8900A_NOINTS
#endif

#ifdef CYGSEM_DEVS_ETH_CL_CS8900A_NOINTS
#define STACK_SIZE CYGNUM_HAL_STACK_SIZE_MINIMUM
static char cs8900a_fake_int_stack[STACK_SIZE];
static cyg_thread cs8900a_fake_int_thread_data;
static cyg_handle_t cs8900a_fake_int_thread_handle;
static void cs8900a_fake_int(cyg_addrword_t);
#endif

#ifdef CYGDBG_IO_ETH_DRIVERS_DEBUG
extern int cyg_io_eth_net_debug;
#endif

static void cs8900a_poll(struct eth_drv_sc *sc);
#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
// This ISR is called when the ethernet interrupt occurs
static int
cs8900a_isr(cyg_vector_t vector, cyg_addrword_t data, HAL_SavedRegisters *regs)
{
    cs8900a_priv_data_t* cpd = (cs8900a_priv_data_t *)data;
    cyg_drv_interrupt_mask(cpd->interrupt);
    cyg_drv_interrupt_acknowledge(cpd->interrupt);
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR);  // Run the DSR
}

static void
cs8900a_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    // This conditioning out is necessary because of explicit calls to this
    // DSR - which would not ever be called in the case of a polled mode
    // usage ie. in RedBoot.
#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
    cs8900a_priv_data_t* cpd = (cs8900a_priv_data_t *)data;
    struct cyg_netdevtab_entry *ndp = (struct cyg_netdevtab_entry *)(cpd->tab);
    struct eth_drv_sc *sc = (struct eth_drv_sc *)(ndp->device_instance);

    DEBUG_FUNCTION();

    // but here, it must be a *sc:
    eth_drv_dsr( vector, count, (cyg_addrword_t)sc );
#else
# ifndef CYGPKG_REDBOOT
#  error Empty CS8900A ethernet DSR is compiled.  Is this what you want?
# endif
#endif
}
#endif

// The deliver function (ex-DSR)  handles the ethernet [logical] processing
static void
cs8900a_deliver(struct eth_drv_sc *sc)
{
    cs8900a_poll(sc);
#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
    {
        cs8900a_priv_data_t *cpd = (cs8900a_priv_data_t *)sc->driver_private;
        // Allow interrupts to happen again
        cyg_drv_interrupt_unmask(cpd->interrupt);
    }
#endif
}

static int
cs8900a_int_vector(struct eth_drv_sc *sc)
{
    cs8900a_priv_data_t *cpd = (cs8900a_priv_data_t *)sc->driver_private;
    return (cpd->interrupt);
}

static bool 
cs8900a_init(struct cyg_netdevtab_entry *tab)
{
    struct eth_drv_sc *sc = (struct eth_drv_sc *)tab->device_instance;
    cs8900a_priv_data_t *cpd = (cs8900a_priv_data_t *)sc->driver_private;
    cyg_addrword_t base = cpd->base;
    cyg_uint16 chip_type, chip_rev, chip_status;
    cyg_uint16 i;
    long timeout = 500000;
    cyg_bool esa_configured = false;
    
    cpd->tab = tab;

    CYGHWR_CL_CS8900A_PLF_INIT(cpd);

#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
    // Initialize environment, setup interrupt handler
    cyg_drv_interrupt_create(cpd->interrupt,
                             0, // Priority - what goes here?
                             (cyg_addrword_t)cpd, //  Data item passed to interrupt handler
                             (cyg_ISR_t *)cs8900a_isr,
                             (cyg_DSR_t *)cs8900a_dsr,
                             &cpd->interrupt_handle,
                             &cpd->interrupt_object);
    cyg_drv_interrupt_attach(cpd->interrupt_handle);
    cyg_drv_interrupt_acknowledge(cpd->interrupt);
    cyg_drv_interrupt_unmask(cpd->interrupt);

#ifdef CYGSEM_DEVS_ETH_CL_CS8900A_NOINTS
    cyg_thread_create(1,                 // Priority
                      cs8900a_fake_int,   // entry
                      (cyg_addrword_t)sc, // entry parameter
                      "CS8900 int",      // Name
                      &cs8900a_fake_int_stack[0],         // Stack
                      STACK_SIZE,        // Size
                      &cs8900a_fake_int_thread_handle,    // Handle
                      &cs8900a_fake_int_thread_data       // Thread data structure
            );
    cyg_thread_resume(cs8900a_fake_int_thread_handle);  // Start it
#endif
#endif

    // Read controller ID - the first is a dummy read, since (on some
    // platforms) the first access to the controller seems to skip the
    // MSB 8 bits.
    get_reg(base, PP_ChipID);
    chip_type = get_reg(base, PP_ChipID);
    chip_rev = get_reg(base, PP_ChipRev);
#if DEBUG & 8
    diag_printf("CS8900A[%p] - type: 0x%04x, rev: 0x%04x\n", base, chip_type, chip_rev);
#endif
    if (chip_type != PP_ChipID_CL) {
#if DEBUG & 8
        diag_printf("CS8900 - invalid type (0x%04x), must be 0x630e\n", chip_type);
#endif
        return false;
    }

    CYGHWR_CL_CS8900A_PLF_RESET(base);
    put_reg(base, PP_SelfCtl, PP_SelfCtl_Reset);  // Reset chip

    CYGHWR_CL_CS8900A_PLF_POST_RESET(base);
    
    while ((get_reg(base, PP_SelfStat) & PP_SelfStat_InitD) == 0) {
        if (--timeout <= 0) {
#if DEBUG & 8
            diag_printf("CS8900 didn't reset - abort!\n");
#endif
            return false;
        }
    }

    chip_status = get_reg(base, PP_SelfStat);
#if DEBUG & 8
    diag_printf("CS8900 - status: 0x%04x (%sEEPROM present)\n", chip_status,
                chip_status & PP_SelfStat_EEPROM ? "" : "no ");
#endif


    // Disable reception whilst finding the ESA
    put_reg(base, PP_LineCTL, 0 );
    // Find ESA - check possible sources in sequence and stop when
    // one provides the ESA:
    //   RedBoot option (via provide_esa)
    //   Compile-time configuration
    //   EEPROM
    //   <fail configuration of device>
    if (NULL != cpd->provide_esa) {
        esa_configured = cpd->provide_esa(cpd);
# if DEBUG & 8
        if (esa_configured)
            diag_printf("Got ESA from RedBoot option\n");
# endif
    }
    if (!esa_configured && cpd->hardwired_esa) {
        // ESA is already set in cpd->esa[]
        esa_configured = true;
    }
    if (!esa_configured && (chip_status & PP_SelfStat_EEPROM)) {
        // Get ESA from EEPROM - via the PP_IA registers
        cyg_uint16 esa_word;
        for (i = 0;  i < sizeof(cpd->esa);  i += 2) {
#ifndef CYGIMP_DEVS_ETH_CL_CS8900A_DATABUS_BYTE_SWAPPED
            esa_word = get_reg(base, PP_IA+i);
            cpd->esa[i] = (esa_word & 0xFF);
            cpd->esa[i+1] = (esa_word >> 8) & 0xFF;
#else
            esa_word = get_reg(base, PP_IA+CYG_SWAP16(i));
            cpd->esa[i+1] = (esa_word & 0xFF);
            cpd->esa[i] = (esa_word >> 8) & 0xFF;
#endif
        }
        esa_configured = true;
    }
    if (!esa_configured) {
# if DEBUG & 8
        diag_printf("CS8900 - no EEPROM, static ESA or RedBoot config option.\n");
# endif
        return false;
    }

    // Tell the chip what ESA to use
    for (i = 0;  i < sizeof(cpd->esa);  i += 2) {
#ifndef CYGIMP_DEVS_ETH_CL_CS8900A_DATABUS_BYTE_SWAPPED
        put_reg(base, PP_IA+i, cpd->esa[i] | (cpd->esa[i+1] << 8));
#else
        put_reg(base, PP_IA+CYG_SWAP16(i), cpd->esa[i+1] | (cpd->esa[i] << 8));
#endif
    }
    // Set logical address mask
    for (i = 0;  i < 8;  i += 2) {
#ifndef CYGIMP_DEVS_ETH_CL_CS8900A_DATABUS_BYTE_SWAPPED
        put_reg(base, PP_LAF+i, 0xFFFF);
#else
        put_reg(base, PP_LAF+CYG_SWAP16(i), 0xFFFF);
#endif
    }
# if DEBUG & 8
    diag_printf("ESA %02x:%02x:%02x:%02x:%02x:%02x\n",
                cpd->esa[0], cpd->esa[1], cpd->esa[2],
                cpd->esa[3], cpd->esa[4], cpd->esa[5]);
# endif

    // Initialize upper level driver
    (sc->funs->eth_drv->init)(sc, cpd->esa);

    return true;
}

static void
cs8900a_stop(struct eth_drv_sc *sc)
{
    cs8900a_priv_data_t *cpd = (cs8900a_priv_data_t *)sc->driver_private;
    cyg_addrword_t base = cpd->base;

    put_reg(base, PP_LineCTL, 0);
}

// This function is called to "start up" the interface.  It may be called
// multiple times, even when the hardware is already running.  It will be
// called whenever something "hardware oriented" changes and should leave
// the hardware ready to send/receive packets.
static void
cs8900a_start(struct eth_drv_sc *sc, cyg_uint8 *esa, int flags)
{
    cyg_uint16 stat;
    cs8900a_priv_data_t *cpd = (cs8900a_priv_data_t *)sc->driver_private;
    cyg_addrword_t base = cpd->base;

    put_reg(base, PP_BusCtl, PP_BusCtl_MemoryE);  // Disable interrupts, memory mode
    put_reg(base, PP_IntReg, PP_IntReg_IRQ0);  // Only possibility
    put_reg(base, PP_RxCFG, PP_RxCFG_RxOK | PP_RxCFG_CRC | 
                      PP_RxCFG_RUNT | PP_RxCFG_EXTRA);
    cpd->rxmode = PP_RxCTL_RxOK | PP_RxCTL_Broadcast | PP_RxCTL_IA;
    put_reg(base, PP_RxCTL, cpd->rxmode);
    put_reg(base, PP_TxCFG, PP_TxCFG_TxOK | PP_TxCFG_Collision | 
                      PP_TxCFG_CRS | PP_TxCFG_SQE | PP_TxCFG_Late | 
                      PP_TxCFG_Jabber | PP_TxCFG_16Collisions);
    put_reg(base, PP_BufCFG, PP_BufCFG_TxRDY | PP_BufCFG_TxUE | PP_BufCFG_RxMiss | 
                       PP_BufCFG_TxCol | PP_BufCFG_Miss | PP_BufCFG_SWI);
    put_reg(base, PP_IntReg, PP_IntReg_IRQ0);  // Only possibility
    put_reg(base, PP_LineCTL, PP_LineCTL_Rx | PP_LineCTL_Tx);
    // Clear Interrupt Status Queue before enabling interrupts
    do {
        HAL_READ_UINT16(cpd->base+CS8900A_ISQ, stat);
    }  while (stat != 0) ;
    cpd->txbusy = false;
    put_reg(base, PP_BusCtl, PP_BusCtl_EnableIRQ);
}

// This routine is called to perform special "control" opertions
static int
cs8900a_control(struct eth_drv_sc *sc, unsigned long key, void *data, int data_length)
{
    cs8900a_priv_data_t *cpd = (cs8900a_priv_data_t *)sc->driver_private;
    cyg_addrword_t base = cpd->base;
    struct eth_drv_mc_list *mc_list = data;

    switch (key) {
    case ETH_DRV_SET_MAC_ADDRESS:
        return 0;
        break;
    case ETH_DRV_SET_MC_LIST:
    case ETH_DRV_SET_MC_ALL:
        // Note: this code always accepts all multicast addresses if any
        // are desired.  It would be possible to accept a subset by adjusting
        // the Logical Address Filter (LAF), but that would require scanning
        // this list and building a suitable mask.
        if (mc_list->len) {
            cpd->rxmode |= PP_RxCTL_Multicast;
        } else {
            cpd->rxmode &= ~PP_RxCTL_Multicast;
        }
        put_reg(base, PP_RxCTL, cpd->rxmode);  // When is it safe to do this?
        return 0;
    default:
        return 1;
        break;
    }
}

// This routine is called to see if it is possible to send another packet.
// It will return non-zero if a transmit is possible, zero otherwise.
static int
cs8900a_can_send(struct eth_drv_sc *sc)
{
    cs8900a_priv_data_t *cpd = (cs8900a_priv_data_t *)sc->driver_private;
    cyg_addrword_t base = cpd->base;
    cyg_uint16 stat;

    stat = get_reg(base, PP_LineStat);
    if ((stat & PP_LineStat_LinkOK) == 0) {
        return false;  // Link not connected
    }
#ifdef CYGPKG_KERNEL
    // Horrible hack!
    if (cpd->txbusy) {
        cyg_tick_count_t now = cyg_current_time();
        if ((now - cpd->txstart) > 25) {
            // 250ms is more than enough to transmit one frame
#if DEBUG & 1
            diag_printf("CS8900: Tx interrupt lost\n");
#endif
            cpd->txbusy = false;
            // Free up the buffer (with error indication)
            (sc->funs->eth_drv->tx_done)(sc, cpd->txkey, 1);
        }
    }
#endif
    return (cpd->txbusy == false);
}

// This routine is called to send data to the hardware.
static void 
cs8900a_send(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len, 
            int total_len, unsigned long key)
{
    cs8900a_priv_data_t *cpd = (cs8900a_priv_data_t *)sc->driver_private;
    cyg_addrword_t base = cpd->base;
    int i;
    int len;
    cyg_uint8 *data;
    cyg_uint16 saved_data = 0, *sdata;
    cyg_uint16 stat;
    bool odd_byte = false;

    // Mark xmitter busy
    cpd->txbusy = true;
    cpd->txkey = key;
#ifdef CYGPKG_KERNEL
    cpd->txstart = cyg_current_time();
#endif

    // Start the xmit sequence
#ifdef CYGIMP_DEVS_ETH_CL_CS8900A_DATABUS_BYTE_SWAPPED
    total_len = CYG_SWAP16(total_len);
#endif
        
    // The hardware indicates that there are options as to when the actual
    // packet transmission will start wrt moving of data into the transmit
    // buffer.  However, impirical results seem to indicate that if the
    // packet is large and transmission is allowed to start before the
    // entire packet has been pushed into the buffer, the hardware gets
    // confused and the packet is lost, along with a "lost" Tx interrupt.
    // This may be a case of the copy loop below being interrupted, e.g.
    // a system timer interrupt, and the hardware getting unhappy that 
    // not all of the data was provided before the transmission should
    // have completed (i.e. buffer underrun).
    // For now, the solution is to not allow this overlap.
    //HAL_WRITE_UINT16(cpd->base+CS8900A_TxCMD, PP_TxCmd_TxStart_5)

    // Start only when all data sent to chip
    HAL_WRITE_UINT16(cpd->base+CS8900A_TxCMD, PP_TxCmd_TxStart_Full);

    HAL_WRITE_UINT16(cpd->base+CS8900A_TxLEN, total_len);
    // Wait for controller ready signal
    do {
        stat = get_reg(base, PP_BusStat);
    } while (!(stat & PP_BusStat_TxRDY));

    // Put data into buffer
    for (i = 0;  i < sg_len;  i++) {
        data = (cyg_uint8 *)sg_list[i].buf;
        len = sg_list[i].len;

        if (len > 0) {
            /* Finish the last word. */
            if (odd_byte) {
// This new byte must get on the bus _after_ the last saved odd byte, it therefore
// belongs in the MSB of the CS8900a
#ifdef CYGIMP_DEVS_ETH_CL_CS8900A_DATABUS_BYTE_SWAPPED                            
                saved_data |= *data++;
#else
                saved_data |= ((cyg_uint16)*data++) << 8;
#endif
                HAL_WRITE_UINT16(cpd->base+CS8900A_RTDATA, saved_data);
                len--;
                odd_byte = false;
            }
            if (((CYG_ADDRESS)data & 0x1) == 0) {
                /* Aligned on 16-bit boundary, so output contiguous words. */
                sdata = (cyg_uint16 *)data;
                while (len > 1) {
					// Make sure data get on the bus in Big Endian format
#if((CYG_BYTEORDER == CYG_MSBFIRST) && defined(CYGIMP_DEVS_ETH_CL_CS8900A_DATABUS_BYTE_SWAPPED) || \
    (CYG_BYTEORDER == CYG_LSBFIRST) && !defined(CYGIMP_DEVS_ETH_CL_CS8900A_DATABUS_BYTE_SWAPPED ))
                    HAL_WRITE_UINT16(cpd->base+CS8900A_RTDATA, *sdata++);
#else
                    HAL_WRITE_UINT16(cpd->base+CS8900A_RTDATA, CYG_SWAP16(*sdata++));
#endif
                    len -= sizeof(cyg_uint16);
                }
                data = (cyg_uint8 *)sdata;
            } else {
                /* Not 16-bit aligned, so byte copy */
                while (len > 1) {
                    saved_data = (cyg_uint16)*data++;   // reuse saved_data
					// Make sure data get on the bus in Big Endian format, the first byte belongs in the
					// LSB of the CS8900A
#ifdef CYGIMP_DEVS_ETH_CL_CS8900A_DATABUS_BYTE_SWAPPED
                    saved_data =  ((cyg_uint16)*data++) | (saved_data << 8);
#else
                    saved_data |= ((cyg_uint16)*data++) << 8;
#endif
                    HAL_WRITE_UINT16(cpd->base+CS8900A_RTDATA, saved_data);
                    len -= sizeof(cyg_uint16);
                }
            }
            /* Save last byte, if necessary. */
            if (len == 1) {
                saved_data = (cyg_uint16)*data;
// This _last_ byte must get on the bus _first_, it therefore belongs in the LSB of
// the CS8900a
#ifdef CYGIMP_DEVS_ETH_CL_CS8900A_DATABUS_BYTE_SWAPPED
                saved_data = (saved_data << 8);
#endif
                odd_byte = true;
            }
        }
    }
    if (odd_byte) {
        HAL_WRITE_UINT16(cpd->base+CS8900A_RTDATA, saved_data);
    }
}

// This function is called when a packet has been received.  It's job is
// to prepare to unload the packet from the hardware.  Once the length of
// the packet is known, the upper layer of the driver can be told.  When
// the upper layer is ready to unload the packet, the internal function
// 'cs8900a_recv' will be called to actually fetch it from the hardware.
static void
cs8900a_RxEvent(struct eth_drv_sc *sc, int stat)
{
    cs8900a_priv_data_t *cpd = (cs8900a_priv_data_t *)sc->driver_private;
    cyg_addrword_t base = cpd->base;
    cyg_uint16 len;

    if(stat & PP_RxCFG_RxOK) {
        // Only start reading a message if one has been received
        HAL_READ_UINT16(base+CS8900A_RTDATA, stat);
        HAL_READ_UINT16(base+CS8900A_RTDATA, len);

#ifdef CYGIMP_DEVS_ETH_CL_CS8900A_DATABUS_BYTE_SWAPPED
        len = CYG_SWAP16(len);
#endif

        CYG_ASSERT(len > 0, "Zero length ethernet frame received");
        
#ifdef CYGDBG_IO_ETH_DRIVERS_DEBUG
        if (cyg_io_eth_net_debug) {
            diag_printf("RxEvent - stat: %x, len: %d\n", stat, len);
        }
#endif
        (sc->funs->eth_drv->recv)(sc, len);
    }
}

// This function is called as a result of the "eth_drv_recv()" call above.
// It's job is to actually fetch data for a packet from the hardware once
// memory buffers have been allocated for the packet.  Note that the buffers
// may come in pieces, using a scatter-gather list.  This allows for more
// efficient processing in the upper layers of the stack.
static void
cs8900a_recv(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len)
{
    cs8900a_priv_data_t *cpd = (cs8900a_priv_data_t *)sc->driver_private;
    cyg_addrword_t base = cpd->base;
    int i, mlen;
    cyg_uint16 *data, val;
    cyg_uint8 *cp, cval;

    for (i = 0;  i < sg_len;  i++) {
        data = (cyg_uint16 *)sg_list[i].buf;
        mlen = sg_list[i].len;
        while (mlen >= sizeof(*data)) {
            HAL_READ_UINT16(base+CS8900A_RTDATA, val);
            if (data) {
#if((CYG_BYTEORDER == CYG_MSBFIRST) && defined(CYGIMP_DEVS_ETH_CL_CS8900A_DATABUS_BYTE_SWAPPED) || \
    (CYG_BYTEORDER == CYG_LSBFIRST) && !defined(CYGIMP_DEVS_ETH_CL_CS8900A_DATABUS_BYTE_SWAPPED ))
                *data++ = val;
#else
                *data++ = CYG_SWAP16(val);
#endif
            }
            mlen -= sizeof(*data);
        }
        if (mlen) {
            HAL_READ_UINT16(base+CS8900A_RTDATA, val);
#ifndef CYGIMP_DEVS_ETH_CL_CS8900A_DATABUS_BYTE_SWAPPED 
            // last odd byte will be in the LSB
            cval = (cyg_uint8)(val);
#elif(CYG_BYTEORDER == CYG_MSBFIRST)
            // last odd byte will be in the MSB
            cval = (cyg_uint8)(val >> 8);
#endif
            cval &= 0xff;
            if ((cp = (cyg_uint8 *)data) != 0) {
                *cp = cval;
            }
        }
    }
}

static void
cs8900a_TxEvent(struct eth_drv_sc *sc, int stat)
{
    cs8900a_priv_data_t *cpd = (cs8900a_priv_data_t *)sc->driver_private;
    cyg_addrword_t base = cpd->base;

    stat = get_reg(base, PP_TER);
#ifdef CYGDBG_IO_ETH_DRIVERS_DEBUG
    if (cyg_io_eth_net_debug) {
        diag_printf("Tx event: %x\n", stat);
    }
#endif
    cpd->txbusy = false;
    (sc->funs->eth_drv->tx_done)(sc, cpd->txkey, 0);
}

static void
cs8900a_BufEvent(struct eth_drv_sc *sc, int stat)
{
    if (stat & PP_BufCFG_RxMiss) {
    }
    if (stat & PP_BufCFG_TxUE) {
    }
}

static void
cs8900a_poll(struct eth_drv_sc *sc)
{
    cyg_uint16 event;
    cs8900a_priv_data_t *cpd = (cs8900a_priv_data_t *)sc->driver_private;
    cyg_addrword_t base = cpd->base;

    HAL_READ_UINT16(base+CS8900A_ISQ, event);
    while (event != 0) {
        switch (event & ISQ_EventMask) {
        case ISQ_RxEvent:
            cs8900a_RxEvent(sc, event);
            break;
        case ISQ_TxEvent:
            cs8900a_TxEvent(sc, event);
            break;
        case ISQ_BufEvent:
            cs8900a_BufEvent(sc, event);
            break;
        case ISQ_RxMissEvent:
            // Receive miss counter has overflowed
            break;
        case ISQ_TxColEvent:
            // Transmit collision counter has overflowed
            break;
        default:
#if DEBUG & 1
            diag_printf("%s: Unknown event: %x\n", __FUNCTION__, event);
#endif
            break;
        }
        HAL_READ_UINT16(base+CS8900A_ISQ, event);
    }

    CYGHWR_CL_CS8900A_PLF_INT_CLEAR(cpd);
}

#ifdef CYGSEM_DEVS_ETH_CL_CS8900A_NOINTS
void
cs8900a_fake_int(cyg_addrword_t param)
{
    struct eth_drv_sc *sc = (struct eth_drv_sc *) param;
    int s;

#if DEBUG & 1
    diag_printf("cs8900a_fake_int()\n");
#endif

    while (true) {
        cyg_thread_delay(5);
        s = splnet();
        cs8900a_poll(sc);
        splx(s);
    }
}
#endif
