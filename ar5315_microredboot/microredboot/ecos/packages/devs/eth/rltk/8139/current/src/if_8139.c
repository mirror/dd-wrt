//==========================================================================
//
//      if_8139.c
//
//	RealTek 8139 ethernet driver
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
// Portions of this software may have been derived from OpenBSD or
// other sources, and are covered by the appropriate copyright
// disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Eric Doenges
// Contributors: Chris Nimmers, Gary Thomas, Andy Dyer
// Date:         2003-07-09
// Purpose:
// Description:  hardware driver for RealTek 8139 ethernet
// Notes:        This is a very basic driver that will send and receive
//               packets and not much else. A lot of features of the 8139
//               are not supported (power management, MII interface,
//               access to the serial eeprom, 'twister tuning', etc.).
//
//               Many of the configuration options (like media type and
//               speed) the 8139 has are taken directly from the serial
//               eeprom and are not currently configurable from this driver.
//
//               I've tentatively added some code to handle cache coherency
//               issues for platforms that do not have a separate address
//               space for uncached memory access and do not do cache
//               snooping for PCI bus masters. This code can be activated by
//               defining CYGPKG_DEVS_ETH_RLTK_8139_SOFTWARE_CACHE_COHERENCY
//               in the platform specific .inl file. Note that if
//               CYGPKG_DEVS_ETH_RLTK_8139_SOFTWARE_CACHE_COHERENCY is
//               defined, the .inl file is also responsible for making sure
//               the receive and transmit buffers are located in memory in
//               such a way that flushing or invalidating cache lines for
//               these buffers will not affect any other buffers. See the
//               README in the doc directory for some suggestions on how
//               to do this.
//
//               Since the official data sheet for this chip is a bit
//               vague, I had to look at the Linux and OpenBSD drivers to
//               understand the basic workings of the chip; however, I have
//               not copied any code from those drivers to avoid tainting
//               eCos' license.
//
//        FIXME:
//
//####DESCRIPTIONEND####
//
//==========================================================================
#include <pkgconf/system.h>
#ifdef CYGPKG_IO_ETH_DRIVERS
#include <pkgconf/io_eth_drivers.h>
#endif
#include <pkgconf/devs_eth_rltk_8139.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>

#include <string.h> /* for memset */

#ifdef CYGPKG_IO_PCI
#include <cyg/io/pci.h>
#else
#error "This driver requires the PCI package (CYGPKG_IO_PCI)"
#endif

#include <cyg/io/pci.h>

/* Necessary for memory mapping macros */
#include CYGHWR_MEMORY_LAYOUT_H

/* Check if we should be dumping debug information or not */
#if defined CYGDBG_DEVS_ETH_RLTK_8139_CHATTER \
    && (CYGDBG_DEVS_ETH_RLTK_8139_CHATTER > 0)
#define DEBUG_RLTK8139_DRIVER
#endif

#include "if_8139.h"

/* Which interrupts we will handle */
#define RLTK8139_IRQ (IR_SERR|IR_FOVW|IR_RXOVW|IR_TER|IR_TOK|IR_RER|IR_ROK)

/* Allow platform-specific configuration of the driver */
#ifndef CYGDAT_DEVS_ETH_RLTK_8139_INL
#error "CYGDAT_DEVS_ETH_RLTK_8139_INL not defined"
#else
#include CYGDAT_DEVS_ETH_RLTK_8139_INL
#endif

#ifndef CYGHWR_RLTK_8139_PLF_INIT
#define CYGHWR_RLTK_8139_PLF_INIT(sc) do {} while(0)
#endif

/*
 * If software cache coherency is required, the HAL_DCACHE_INVALIDATE
 * hal macro must be defined as well.
 */
#ifdef CYGPKG_DEVS_ETH_RLTK_8139_SOFTWARE_CACHE_COHERENCY
#if !defined HAL_DCACHE_INVALIDATE || !defined HAL_DCACHE_FLUSH
#error "HAL_DCACHE_INVALIDATE/HAL_DCACHE_FLUSH not defined for this platform but CYGPKG_DEVS_ETH_RLTK_8139_CACHE_COHERENCY was defined."
#endif
#endif

/* Local driver function declarations */
#ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
static cyg_uint32 rltk8139_isr(cyg_vector_t vector, cyg_addrword_t data);
#endif

#ifdef ETH_DRV_SET_MC_LIST
static cyg_uint32 ether_crc(cyg_uint8 *data, int length);
static void rltk8139_set_multicast_list(Rltk8139_t *rltk8139_info,
                                        struct eth_drv_mc_list *mc_list);
#endif

static void rltk8139_reset(struct eth_drv_sc *sc);
static bool rltk8139_init(struct cyg_netdevtab_entry *tab);
static void rltk8139_start(struct eth_drv_sc *sc, unsigned char *enaddr,
                           int flags);
static void rltk8139_stop(struct eth_drv_sc *sc);
static int rltk8139_control(struct eth_drv_sc *sc, unsigned long key,
                            void *data, int   data_length);
static int rltk8139_can_send(struct eth_drv_sc *sc);
static void rltk8139_send(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list,
                          int sg_len, int total_len, unsigned long key);
static void rltk8139_recv(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list,
                          int sg_len);
static void rltk8139_deliver(struct eth_drv_sc *sc);
static void rltk8139_poll(struct eth_drv_sc *sc);
static int rltk8139_int_vector(struct eth_drv_sc *sc);

#ifdef DEBUG_RLTK8139_DRIVER
void rltk8139_print_state(struct eth_drv_sc *sc);
#endif

/*
 * Define inline functions to access the card. This will also handle
 * endianess issues in one place. This code was lifted from the eCos
 * i82559 driver.
 */
#if (CYG_BYTEORDER == CYG_MSBFIRST)
#define HAL_CTOLE32(x)  ((((x) & 0xff) << 24) | (((x) & 0xff00) << 8) | (((x) & 0xff0000) >> 8) | (((x) >> 24) & 0xff))
#define HAL_LE32TOC(x)  ((((x) & 0xff) << 24) | (((x) & 0xff00) << 8) | (((x) & 0xff0000) >> 8) | (((x) >> 24) & 0xff))

#define HAL_CTOLE16(x)  ((((x) & 0xff) << 8) | (((x) & 0xff00) >> 8))
#define HAL_LE16TOC(x)  ((((x) & 0xff) << 8) | (((x) & 0xff00) >> 8))

static inline void
OUTB(cyg_uint8 value,
          cyg_uint32 io_address)
{
  HAL_WRITE_UINT8( io_address, value);
}

static inline void
OUTW(cyg_uint16 value, cyg_uint32 io_address)
{
  HAL_WRITE_UINT16(io_address,
                   (((value & 0xff) << 8) | ((value & 0xff00) >> 8)) );
}

static inline void
OUTL(cyg_uint32 value, cyg_uint32 io_address)
{
  HAL_WRITE_UINT32(io_address,
                   ((((value) & 0xff) << 24) | (((value) & 0xff00) << 8)
                    | (((value) & 0xff0000) >> 8)
                    | (((value) >> 24) & 0xff)) );
}

static inline cyg_uint8
INB(cyg_uint32 io_address)
{
  cyg_uint8 d;
  HAL_READ_UINT8(io_address, d);
  return d;
}

static inline cyg_uint16
INW(cyg_uint32 io_address)
{
  cyg_uint16 d;
  HAL_READ_UINT16( io_address, d );
  return (((d & 0xff) << 8) | ((d & 0xff00) >> 8));
}

static inline cyg_uint32
INL(cyg_uint32 io_address)
{
  cyg_uint32 d;
  HAL_READ_UINT32(io_address, d);
  return ((((d) & 0xff) << 24) | (((d) & 0xff00) << 8)
          | (((d) & 0xff0000) >> 8) | (((d) >> 24) & 0xff));
}
#else

// Maintaining the same styleee as above...
#define HAL_CTOLE32(x)  ((((x))))
#define HAL_LE32TOC(x)  ((((x))))

#define HAL_CTOLE16(x)  ((((x))))
#define HAL_LE16TOC(x)  ((((x))))

static inline void OUTB(cyg_uint8  value, cyg_uint32 io_address)
{   HAL_WRITE_UINT8( io_address, value );   }

static inline void OUTW(cyg_uint16 value, cyg_uint32 io_address)
{   HAL_WRITE_UINT16( io_address, value );   }

static inline void OUTL(cyg_uint32 value, cyg_uint32 io_address)
{   HAL_WRITE_UINT32( io_address, value );   }

static inline cyg_uint8  INB(cyg_uint32 io_address)
{   cyg_uint8  _t_; HAL_READ_UINT8(  io_address, _t_ ); return _t_;   }

static inline cyg_uint16 INW(cyg_uint32 io_address)
{   cyg_uint16 _t_; HAL_READ_UINT16( io_address, _t_ ); return _t_;   }

static inline cyg_uint32 INL(cyg_uint32 io_address)
{   cyg_uint32 _t_; HAL_READ_UINT32( io_address, _t_ ); return _t_;   }

#endif // byteorder


/*
 * Table of all known PCI device/vendor ID combinations for the RealTek 8139.
 * Add them as you get to know them.
 */
#define CYGNUM_DEVS_ETH_RLTK_8139_KNOWN_ALIASES 2
static pci_identifier_t
known_8139_aliases[CYGNUM_DEVS_ETH_RLTK_8139_KNOWN_ALIASES] = {
  { 0x10ec, 0x8139, NULL }, /* This is the offical RealTek vendor/device code */
  { 0x11db, 0x1234, NULL} /* SEGA DreamCast BroadBandAdapter */
};


/*
 * Check if the device description matches a known 8139 alias.
 */
static cyg_bool
rltk8139_find_match_func(cyg_uint16 vendor_id, cyg_uint16 device_id,
                         cyg_uint32 class_id, void *p)
{
  int i;


  for (i = 0; i < CYGNUM_DEVS_ETH_RLTK_8139_KNOWN_ALIASES; ++i) {
    if (((known_8139_aliases[i].vendor_id == PCI_ANY_ID) ||
         (known_8139_aliases[i].vendor_id == vendor_id)) &&
        ((known_8139_aliases[i].device_id == PCI_ANY_ID) ||
         (known_8139_aliases[i].device_id == device_id)))
      return true;
  }

  return false;
}


/*
 * Find the Nth 8139 device on all attached PCI busses and do some initial
 * PCI-type initialization. Also setup the interrupt for use in eCos.
 */
static bool
rltk8139_find(int n_th, struct eth_drv_sc *sc)
{
  Rltk8139_t *rltk8139_info;
  cyg_pci_device_id pci_device_id;
  cyg_pci_device pci_device_info;
  cyg_uint16 command;
  int found = -1;


  /* Go through all PCI devices until we find the Nth 8139 chip */
  do {
    pci_device_id = CYG_PCI_NULL_DEVID;
    if (!cyg_pci_find_matching(&rltk8139_find_match_func, NULL,
                               &pci_device_id))
      return false;
    else
      found += 1;
  } while (found != n_th);

  /* Save device ID in driver private data in case we ever need it again */
  rltk8139_info = (Rltk8139_t *)(sc->driver_private);
  rltk8139_info->pci_device_id = pci_device_id;

  /* Now that we have found the device, we can extract some data about it */
  cyg_pci_get_device_info(pci_device_id, &pci_device_info);

  /* Get the assigned interrupt and set up ISR and DSR for it. */
  if (cyg_pci_translate_interrupt(&pci_device_info, &rltk8139_info->vector)) {
#ifdef DEBUG_RLTK8139_DRIVER
    diag_printf(" Wired to HAL interrupt vector %d\n", rltk8139_info->vector);
#endif

#ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
    /*
     * Note that we use the generic eth_drv_dsr routine instead of
     * our own.
     */
    cyg_drv_interrupt_create(rltk8139_info->vector, 0,
                             (CYG_ADDRWORD)sc,
                             rltk8139_isr,
                             eth_drv_dsr,
                             &rltk8139_info->interrupt_handle,
                             &rltk8139_info->interrupt);
    cyg_drv_interrupt_attach(rltk8139_info->interrupt_handle);
#endif
  }
  else {
#ifdef DEBUG_RLTK8139_DRIVER
    diag_printf(" Does not generate interrupts.\n");
#endif
  }

  /*
   * Call 'cyg_pci_configure_device' for those platforms that do not
   * configure the PCI bus during HAL initialization. According to Nick
   * Garnett, there are good reasons to not configure PCI devices during HAL
   * initialization. Also according to Nick, calling
   * 'cyg_pci_configure_device' for devices already configured should have no
   * effect and thus is safe to do.
   */
  if (cyg_pci_configure_device(&pci_device_info)) {
#ifdef DEBUG_RLTK8139_DRIVER
    int i;
    diag_printf("Found device #%d on bus %d, devfn 0x%02x:\n", n_th,
                CYG_PCI_DEV_GET_BUS(pci_device_id),
                CYG_PCI_DEV_GET_DEVFN(pci_device_id));

    if (pci_device_info.command & CYG_PCI_CFG_COMMAND_ACTIVE)
      diag_printf(" Note that board is active. Probed"
                  " sizes and CPU addresses are invalid!\n");

    diag_printf(" Vendor    0x%04x", pci_device_info.vendor);
    diag_printf("\n Device    0x%04x", pci_device_info.device);
    diag_printf("\n Command   0x%04x, Status 0x%04x\n",
                pci_device_info.command, pci_device_info.status)
      ;

    diag_printf(" Class/Rev 0x%08x", pci_device_info.class_rev);
    diag_printf("\n Header 0x%02x\n", pci_device_info.header_type);

    diag_printf(" SubVendor 0x%04x, Sub ID 0x%04x\n",
                pci_device_info.header.normal.sub_vendor,
                pci_device_info.header.normal.sub_id);

    for(i = 0; i < CYG_PCI_MAX_BAR; i++) {
      diag_printf(" BAR[%d]    0x%08x /", i, pci_device_info.base_address[i]);
      diag_printf(" probed size 0x%08x / CPU addr 0x%08x\n",
                  pci_device_info.base_size[i], pci_device_info.base_map[i]);
    }
#endif
  }
  else {
#ifdef DEBUG_RLTK8139_DRIVER
    diag_printf("Failed to configure 8139 device #%d\n", n_th);
#endif
    return false;
  }

  /*
   * Enable memory mapped and port based I/O and busmastering. We currently
   * only support IO space accesses; memory mapping is enabled so that bit
   * DVRLOAD in CONFIG1 is cleared automatically.
   *
   * NOTE: it seems that for some configurations/HALs, the device is already
   *       activated at this point, even though eCos' documentation suggests
   *       it shouldn't be. At least in my case, this is not a problem.
   */
  cyg_pci_read_config_uint16(pci_device_info.devid, CYG_PCI_CFG_COMMAND,
                             &command);
  command |= (CYG_PCI_CFG_COMMAND_IO | CYG_PCI_CFG_COMMAND_MEMORY
              | CYG_PCI_CFG_COMMAND_MASTER);
  cyg_pci_write_config_uint16(pci_device_info.devid, CYG_PCI_CFG_COMMAND,
                              command);

  /*
   * This is the base address used to talk to the device. The 8139's IOAR
   * and MEMAR registers are BAR0 and BAR1, respectively.
   */
  rltk8139_info->base_address = pci_device_info.base_map[0];

  /*
   * Read the MAC address. The RealTek data sheet seems to claim this should
   * only be read in 4 byte accesses, but the code below works just fine.
   */
  for (found = 0; found < 6; ++found)
    rltk8139_info->mac[found] = INB(rltk8139_info->base_address + IDR0 + found);

  /*
   * This is the indicator for "uses an interrupt". The code was lifted
   * from the eCos Intel 82559 driver.
   */
  if (rltk8139_info->interrupt_handle != 0) {
    cyg_drv_interrupt_acknowledge(rltk8139_info->vector);

#ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
    cyg_drv_interrupt_unmask(rltk8139_info->vector);
#endif
  }

  return true;
}

#ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
/*
 * Interrupt service routine. We do not clear the interrupt status bits
 * (since this should really only be done after handling whatever caused
 * the interrupt, and that is done in the '_deliver' routine), but instead
 * clear the interrupt mask.
 *
 * If we are sharing interrupts with other devices, we have two options
 * (configurable):
 *
 * 1. Mask the interrupt vector completly. Personally I think this is a bad
 *    idea because the other devices sharing this interrupt are also masked
 *    until the network thread gets around to calling the '_deliver' routine.
 *
 * 2. Use the interrupt mask register in the 8139 to mask just the interrupt
 *    request coming from the 8139. This way, the other devices' requests
 *    can still be serviced.
 */
static cyg_uint32
rltk8139_isr(cyg_vector_t vector, cyg_addrword_t data)
{
  Rltk8139_t *rltk8139_info;
#ifdef CYGPKG_DEVS_ETH_RLTK_8139_SHARE_INTERRUPTS
  cyg_uint16 isr;
#endif

  rltk8139_info = (Rltk8139_t *)(((struct eth_drv_sc *)data)->driver_private);

#ifdef CYGPKG_DEVS_ETH_RLTK_8139_SHARE_INTERRUPTS
  /*
   * If interrupt sharing is enabled, check if the interrupt is really
   * intended for us. Note that while the RealTek data sheet claims that
   * reading the interrupt status register will clear all it's bits,
   * this is not true, so we can read it without problems.
   */
  if (!(isr = INW(rltk8139_info->base_address + ISR)))
    return 0;
#endif

#ifdef CYGPKG_DEVS_ETH_RLTK_8139_MASK_INTERRUPTS_IN_8139
  /* Clear the interrupt mask to stop the current request. */
  OUTW(0, rltk8139_info->base_address + IMR);
#else
  /* Mask the interrupt */
  cyg_interrupt_mask(vector);
#endif

  /* Acknowledge the interrupt for those platforms were this is necessary */
  cyg_interrupt_acknowledge(vector);

  return (CYG_ISR_HANDLED | CYG_ISR_CALL_DSR);
}
#endif /* ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE */

/*
 * Reset the chip. This function is not exported to higher level software.
 */
static void
rltk8139_reset(struct eth_drv_sc *sc)
{
  rltk8139_stop(sc);
  rltk8139_start(sc, NULL, 0);
}

#ifdef ETH_DRV_SET_MC_LIST
/*
 * I assume (hope !) that this is identical to Ethernet's CRC algorithm
 * specified in IEEE 802.3. It does seem to calculate the same CRC that
 * the 8139 itself does, so I think it is correct.
 * Note that while Ethernet's polynomial is usually specified as 04C11DB7,
 * we must use EDB88320 because we shift the bits to the left, not the right.
 * (See ftp://ftp.rocksoft.com/papers/crc_v3.txt for a good description of
 * CRC algorithms).
 */
static cyg_uint32
ether_crc(cyg_uint8 *data, int length)
{
  int bit;
  cyg_uint32 crc = 0xFFFFFFFFU;

  while (length-- > 0) {
    crc ^= *data++;
    for (bit = 0; bit < 8; ++bit) {
      if (crc & 1)
        crc = (crc >> 1) ^ 0xEDB88320U;
      else
        crc = (crc >> 1);
    }
  }

  return crc ^ 0xFFFFFFFFU;
}


/*
 * Set up multicast filtering. The way I understand existing driver code
 * (Linux and OpenBSD), the 8139 calculates the ethernet CRC of
 * incoming addresses and uses the top 6 bits as an index into a hash
 * table. If the corresponding bit is set in MAR0..7, the address is
 * accepted.
 */
static void
rltk8139_set_multicast_list(Rltk8139_t *rltk8139_info,
                            struct eth_drv_mc_list *mc_list)
{
  cyg_uint32 mar[2], hash;
  int i;

  /* If 'mc_list' is NULL, accept all multicast packets. */
  if (!mc_list) {
    mar[0] = 0xFFFFFFFFU;
    mar[1] = 0xFFFFFFFFU;
  }
  else {
    mar[0] = 0;
    mar[1] = 0;

    for (i = 0; i < mc_list->len; ++i) {
      hash = ether_crc(&mc_list->addrs[i][0], ETHER_ADDR_LEN) >> 26;
      mar[hash >> 5] |= (1 << (hash & 0x1F));
    }
  }

  /* Program the new filter values */
  OUTL(mar[0], rltk8139_info->base_address + MAR0);
  OUTL(mar[1], rltk8139_info->base_address + MAR4);
}
#endif /* ifdef ETH_DRV_SET_MC_LIST */


/*
 * Initialize the network interface. Since the chips is reset by calling
 * _stop() and _start(), any code that will never need to be executed more
 * than once after system startup should go here.
 */
static bool
rltk8139_init(struct cyg_netdevtab_entry *tab)
{
  struct eth_drv_sc *sc;
  Rltk8139_t *rltk8139_info;


  sc = (struct eth_drv_sc *)(tab->device_instance);
  rltk8139_info = (Rltk8139_t *)(sc->driver_private);

  /*
   * Initialize the eCos PCI library. According to the documentation, it
   * is safe to call this function multiple times, so we call it just to
   * be sure it has been done.
   */
  cyg_pci_init();

  /*
   * Scan the PCI bus for the specified chip. The '_find' function will also
   * do some basic PCI initialization.
   */
  if (!rltk8139_find(rltk8139_info->device_num, sc)) {
#ifdef DEBUG_RLTK8139_DRIVER
    diag_printf("rltk8139_init: could not find RealTek 8139 chip #%d.\n",
                rltk8139_info->device_num);
#endif
    return false;
  }

  /* platform depends initialize */
  CYGHWR_RLTK_8139_PLF_INIT(sc);

  /*
   * The initial tx threshold is set here to prevent it from being reset
   * with every _start().
   */
  rltk8139_info->tx_threshold = 3;

  /* Initialize upper level driver */
  (sc->funs->eth_drv->init)(sc, rltk8139_info->mac);

  return true;
}


/*
 * (Re)Start the chip, initializing data structures and enabling the
 * transmitter and receiver. Currently, 'flags' is unused by eCos.
 */
static void
rltk8139_start(struct eth_drv_sc *sc, unsigned char *enaddr, int flags)
{
  Rltk8139_t *rltk8139_info;
  int i;


#ifdef DEBUG_RLTK8139_DRIVER
  diag_printf("rltk8139_start(%s)\n", sc->dev_name);
#endif

  rltk8139_info = (Rltk8139_t *)(sc->driver_private);

  /*
   * Reset the chip. Existing driver code implies that this may take up
   * to 10ms; since I don't know under what exact circumstances this code may
   * be called I busy wait here regardless.
   */
  OUTB(RST, rltk8139_info->base_address + CR);
  while (INB(rltk8139_info->base_address + CR) & RST);

#ifdef DEBUG_RLTK8139_DRIVER
  diag_printf("rltk8139_start(%s): 8139 was successfully reset.\n",
              sc->dev_name);
#endif

  /*
   * Clear the key storage area. These keys are used by the eCos networking
   * support code to keep track of individual packets.
   */
  for (i = 0; i < NUM_TX_DESC; ++i)
    rltk8139_info->tx_desc_key[i] = 0;

  /* Initialize transmission buffer control */
  rltk8139_info->tx_free_desc = 0;
  rltk8139_info->tx_num_free_desc = NUM_TX_DESC;

  /*
   * Set the requested MAC address if enaddr != NULL. This is a special
   * feature of my '_start' function since it's used to reset the chip after
   * errors as well.
   */
  if (enaddr != NULL) {
    for (i = 0; i < 6; ++i) {
      rltk8139_info->mac[i] = enaddr[i];
      OUTB(enaddr[i], rltk8139_info->base_address + IDR0 + i);
    }
  }

  /*
   * Now setup the transmission and reception buffers. These could be done
   * in _init() and kept around, but putting them here fits better logically.
   */
  OUTL(CYGARC_PCI_DMA_ADDRESS((cyg_uint32)(rltk8139_info->tx_buffer
                                            + 0 * TX_BUF_SIZE)),
       rltk8139_info->base_address + TSAD0);
  OUTL(CYGARC_PCI_DMA_ADDRESS((cyg_uint32)(rltk8139_info->tx_buffer
                                            + 1 * TX_BUF_SIZE)),
       rltk8139_info->base_address + TSAD1);
  OUTL(CYGARC_PCI_DMA_ADDRESS((cyg_uint32)(rltk8139_info->tx_buffer
                                            + 2 * TX_BUF_SIZE)),
       rltk8139_info->base_address + TSAD2);
  OUTL(CYGARC_PCI_DMA_ADDRESS((cyg_uint32)(rltk8139_info->tx_buffer
                                            + 3 * TX_BUF_SIZE)),
       rltk8139_info->base_address + TSAD3);
  OUTL(CYGARC_PCI_DMA_ADDRESS((cyg_uint32)rltk8139_info->rx_ring),
       rltk8139_info->base_address + RBSTART);

  /*
   * Enable the transmitter and receiver, then clear the missed packet
   * counter.
   */
  OUTB(INB(rltk8139_info->base_address + CR) | (TE|RE),
       rltk8139_info->base_address + CR);
  OUTL(0, rltk8139_info->base_address + MPC);

  /*
   * It seems the receiver and transmitter configuration can only
   * be set after the transmitter/receiver have been enabled.
   */
  OUTL(TXCFG, rltk8139_info->base_address + TCR);
  OUTL(RXCFG | AM, rltk8139_info->base_address + RCR);

  /*
   * Enable the transmitter and receiver again. I'm not sure why this is
   * necessary; the Linux driver does it so we do it here just to be on
   * the safe side.
   */
  OUTB(INB(rltk8139_info->base_address + CR) | (TE|RE),
       rltk8139_info->base_address + CR);

#ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
  /*
   * If this driver was not compiled in stand alone (without interrupts)
   * mode, enable interrupts.
   */
  OUTW(RLTK8139_IRQ, rltk8139_info->base_address + IMR);
#endif

#ifdef DEBUG_RLTK8139_DRIVER
  rltk8139_print_state(sc);
#endif
}


/*
 * Stop the chip, disabling the transmitter and receiver.
 */
static void
rltk8139_stop(struct eth_drv_sc *sc)
{
  Rltk8139_t *rltk8139_info;


#ifdef DEBUG_RLTK8139_DRIVER
  diag_printf("rltk8139_stop(%s)\n", sc->dev_name);
#endif

  rltk8139_info = (Rltk8139_t *)(sc->driver_private);

  /* Disable receiver and transmitter */
  OUTB(INB(rltk8139_info->base_address + CR) & ~(TE|RE),
       rltk8139_info->base_address + CR);

  /* Mask all interrupts */
  OUTW(0, rltk8139_info->base_address + IMR);
}


/*
 * 8139 control function. Unlike a 'real' ioctl function, this function is
 * not required to tell the caller why a request failed, only that it did
 * (see the eCos documentation).
 */
static int
rltk8139_control(struct eth_drv_sc *sc, unsigned long key, void *data,
                 int data_length)
{
  int i;
  Rltk8139_t *rltk8139_info;


#ifdef DEBUG_RLTK8139_DRIVER
  diag_printf("rltk8139_control(%08x, %lx)\n", sc, key);
#endif

  rltk8139_info = (Rltk8139_t *)(sc->driver_private);

  switch (key) {
#ifdef ETH_DRV_SET_MAC_ADDRESS
  case ETH_DRV_SET_MAC_ADDRESS:
    if ( 6 != data_length )
      return 1;

    /* Set the mac address */
    for (i = 0; i < 6; ++i) {
      rltk8139_info->mac[i] = *(((cyg_uint8 *)data) + i);
      OUTB(rltk8139_info->mac[i], rltk8139_info->base_address + IDR0 + i);
    }
    return 0;
#endif

#ifdef ETH_DRV_GET_MAC_ADDRESS
    case ETH_DRV_GET_MAC_ADDRESS:
      if (6 != data_length)
        return 1;

      memcpy(data, rltk8139_info->mac, 6);
      return 0;
#endif

#ifdef ETH_DRV_GET_IF_STATS_UD
    case ETH_DRV_GET_IF_STATS_UD: // UD == UPDATE
      //ETH_STATS_INIT( sc );    // so UPDATE the statistics structure
#endif
      // drop through
#ifdef ETH_DRV_GET_IF_STATS
  case ETH_DRV_GET_IF_STATS:
#endif
#if defined(ETH_DRV_GET_IF_STATS) || defined (ETH_DRV_GET_IF_STATS_UD)
    break;
#endif

#ifdef ETH_DRV_SET_MC_LIST
  case ETH_DRV_SET_MC_LIST:
    /*
     * Program the 8139's multicast filter register. If the eth_drv_mc_list
     * contains at least one element, set the accept multicast bit in the
     * receive config register.
     */
    rltk8139_set_multicast_list(rltk8139_info, (struct eth_drv_mc_list *)data);
    if (((struct eth_drv_mc_list *)data)->len > 0)
      OUTL(INL(rltk8139_info->base_address + RCR) | AM,
           rltk8139_info->base_address + RCR);
    else
      OUTL(INL(rltk8139_info->base_address + RCR) & ~AM,
           rltk8139_info->base_address + RCR);

    return 0;
#endif // ETH_DRV_SET_MC_LIST

#ifdef ETH_DRV_SET_MC_ALL
  case ETH_DRV_SET_MC_ALL:
    /*
     * Set the accept multicast bit in the receive config register and
     * program the multicast filter to accept all addresses.
     */
    rltk8139_set_multicast_list(rltk8139_info, NULL);
    OUTL(INL(rltk8139_info->base_address + RCR) | AM,
         rltk8139_info->base_address + RCR);
    return 0;
#endif // ETH_DRV_SET_MC_ALL

  default:
    return 1;
  }

  return 1;
}


/*
 * Check if a new packet can be sent.
 */
static int
rltk8139_can_send(struct eth_drv_sc *sc)
{
  return ((Rltk8139_t *)(sc->driver_private))->tx_num_free_desc;
}


/*
 * Send a packet over the wire.
 */
static void
rltk8139_send(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len,
              int total_len, unsigned long key)
{
  Rltk8139_t *rltk8139_info;
  cyg_uint8 *tx_buffer;
  struct eth_drv_sg *last_sg;
  int desc;

  rltk8139_info = (Rltk8139_t *)(sc->driver_private);

#ifdef DEBUG_RLTK8139_DRIVER
  diag_printf("rltk8139_send(%s, %08x, %d, %d, %08lx)\n",
              sc->dev_name, sg_list, sg_len, total_len, key);
#endif

  CYG_ASSERT(total_len <= TX_BUF_SIZE, "packet too long");

  /*
   * Get the next free descriptor to send. We lock out all interrupts
   * and scheduling because we really, really do not want to be interrupted
   * at this point.
   *
   * IMPORTANT NOTE: the RealTek data sheet does not really make this clear,
   * but when they talk about a 'ring' of transmit descriptors, they
   * _really_ mean it, i.e. you _must_ use descriptor #1 after descriptor
   * #0 even if transmission of descriptor #0 has already completed.
   */
  cyg_drv_isr_lock();

  /*
   * Sanity check to see if '_send' was called even though there is no free
   * descriptor. This is probably unnecessary.
   */
  if (rltk8139_info->tx_num_free_desc == 0) {
      cyg_drv_isr_unlock();
#ifdef DEBUG_RLTK8139_DRIVER
    diag_printf("rltk8139_send(%s): no free descriptor available\n",
                sc->dev_name);
#endif
    return;
  }

  /*
   * Get the free descriptor and advance the descriptor counter modulo
   * TX_NUM_DESC. We assume that TX_NUM_DESC will always be a power of 2.
   */
  desc = rltk8139_info->tx_free_desc;
  rltk8139_info->tx_free_desc = (rltk8139_info->tx_free_desc + 1)
    & (NUM_TX_DESC - 1);

  /* Decrement the number of free descriptors */
  rltk8139_info->tx_num_free_desc -= 1;

  /* Reenable interrupts at this point */
  cyg_drv_isr_unlock();

  /*
   * Determine the buffer memory to use and tell the hardware about it.
   * Since we use fixed buffer addresses, we do not need to set up TSADx.
   * Memorize the key so we can call the tx_done callback correctly.
   *
   * While it would be possible to set TSADx to the packet directly if
   * it is stored in a linear memory area with 32 bit alignment, it seems
   * this happens so seldomly that it's simply not worth the extra
   * runtime check.
   */
  tx_buffer = CYGARC_UNCACHED_ADDRESS(rltk8139_info->tx_buffer
                                      + TX_BUF_SIZE * desc);
  rltk8139_info->tx_desc_key[desc] = key;

  /*
   * Copy the data to the designated position. Note that unlike the eCos
   * Intel 82559 driver, we simply assume that all the scatter/gather list
   * elements' lengths will add up to total_len exactly, and don't check
   * to make sure.
   */
  for (last_sg = &sg_list[sg_len]; sg_list < last_sg; ++sg_list) {
    memcpy(tx_buffer, (void *)(sg_list->buf), sg_list->len);
    tx_buffer += sg_list->len;
  }

  /*
   * Make sure the packet has the minimum ethernet packet size, padding
   * with zeros if necessary.
   */
  if (total_len < MIN_ETH_FRAME_SIZE) {
    memset(tx_buffer, 0, MIN_ETH_FRAME_SIZE - total_len);
    total_len = MIN_ETH_FRAME_SIZE;
  }

  /*
   * Flush the data cache here if necessary. This ensures the 8139 can
   * read the correct data from the transmit buffer.
   */
#ifdef CYGPKG_DEVS_ETH_RLTK_8139_SOFTWARE_CACHE_COHERENCY
  HAL_DCACHE_FLUSH(rltk8139_info->tx_buffer + TX_BUF_SIZE * desc, total_len);
#endif

  /*
   * Now setup the correct transmit descriptor to actually send the data.
   * The early TX threshold is incremented by the driver until we reach a
   * size that prevents transmit underruns. (An earlier attempt to calculate
   * this parameter from the packet size didn't work).
   */
  OUTL((rltk8139_info->tx_threshold << ERTXTH_SHIFT) | (total_len & SIZE),
       rltk8139_info->base_address + TSD0 + (desc<<2));
}


/*
 * This routine actually retrieves data from the receive ring by
 * copying it into the specified scatter/gather buffers. Again,
 * we assume the scatter/gather list is OK and don't check against
 * total length.
 */
static void
rltk8139_recv(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list,
              int sg_len)
{
  Rltk8139_t *rltk8139_info;
  struct eth_drv_sg *last_sg;
  cyg_uint8 *rx_buffer;


  rltk8139_info = (Rltk8139_t *)(sc->driver_private);
  rx_buffer = rltk8139_info->rx_current;

  /*
   * Invalidate the cache line(s) mapped to the receive buffer
   * if necessary.
   */
#ifdef CYGPKG_DEVS_ETH_RLTK_8139_SOFTWARE_CACHE_COHERENCY
  HAL_DCACHE_INVALIDATE(rx_buffer, rltk8139_info->rx_size);
#endif

  for (last_sg = &sg_list[sg_len]; sg_list < last_sg; ++sg_list) {
    if (sg_list->buf != (CYG_ADDRESS)0) {
      memcpy((void *)(sg_list->buf), rx_buffer, sg_list->len);
      rx_buffer += sg_list->len;
    }
  }
}


/*
 * This function does all the heavy lifting associated with interrupts.
 */
static void
rltk8139_deliver(struct eth_drv_sc *sc)
{
  Rltk8139_t *rltk8139_info;
  cyg_uint16 status, pci_status;
  cyg_uint32 tsd;
  int desc;


  rltk8139_info = (Rltk8139_t *)(sc->driver_private);

  /*
   * The RealTek data sheet claims that reading the ISR will clear
   * it. This is incorrect; to clear a bit in the ISR, a '1' must be
   * written to the ISR instead. We immediately clear the interrupt
   * bits at this point.
   */
  status = INW(rltk8139_info->base_address + ISR);
  OUTW(status, rltk8139_info->base_address + ISR);

#ifdef DEBUG_RLTK8139_DRIVER
  diag_printf("rltk8139_deliver(%s): %04x\n", sc->dev_name, status);
#endif

  /*
   * Check for a PCI error. This seems like a very serious error to
   * me, so we will reset the chip and hope for the best.
   */
  if (status & IR_SERR) {
    cyg_pci_read_config_uint16(rltk8139_info->pci_device_id,
                               CYG_PCI_CFG_STATUS, &pci_status);
    cyg_pci_write_config_uint16(rltk8139_info->pci_device_id,
                                CYG_PCI_CFG_STATUS, pci_status);
#ifdef DEBUG_RLTK8139_DRIVER
    diag_printf("rltk8139_deliver(%s): PCI error %04x\n",
                sc->dev_name, pci_status);
#endif

    rltk8139_reset(sc);
    return;
  }

  /* Check for transmission complete (with errors or not) */
  if ((status & IR_TER) || (status & IR_TOK)) {
    /*
     * Figure out which descriptors' status must be checked. We lock out
     * interrupts while manipulating the descriptor list because we do not
     * want to be interrupted at this point.
     */
    while (1) {
      cyg_drv_isr_lock();

      /* Check if all descriptors are ready, in which case we are done. */
      if (rltk8139_info->tx_num_free_desc >= NUM_TX_DESC) {
          cyg_drv_isr_unlock();
        break;
      }

      desc = (rltk8139_info->tx_free_desc
              - (NUM_TX_DESC - rltk8139_info->tx_num_free_desc))
        & (NUM_TX_DESC - 1);
      cyg_drv_isr_unlock();

      /* Get the current status of the descriptor */
      tsd = INL(rltk8139_info->base_address + TSD0 + (desc<<2));

      /*
       * If a transmit FIFO underrun occurred, increment the threshold
       * value.
       */
      if ((tsd & TUN) && (rltk8139_info->tx_threshold < 64))
        rltk8139_info->tx_threshold += 1;

      /*
       * Check if a transmission completed OK. RealTek's data sheet implies
       * that a successful transmission that experiences underrun will only
       * set TUN. This is not true; TOK is set for all successful
       * transmissions.
       */
      if (tsd & TOK) {
        (sc->funs->eth_drv->tx_done)(sc, rltk8139_info->tx_desc_key[desc], 0);
      }
      else if (tsd & TABT) {
        /*
         * Set the CLRABT bit in TCR. Since I haven't encountered any
         * transmission aborts so far, I don't really know if this code
         * will work or not.
         */
        OUTL(INL(rltk8139_info->base_address + TCR) & CLRABT,
             rltk8139_info->base_address + TCR);
        (sc->funs->eth_drv->tx_done)(sc, rltk8139_info->tx_desc_key[desc], -1);
      }
      else {
        /*
         * This descriptor is not ready. Since the descriptors are used
         * in a ring, this means that no more descriptors are ready.
         */
        break;
      }

      /*
       * Clear the saved key value. This is not really necessary, since
       * the key value is never used to determine if a descriptor is
       * valid or not. However, clearing it is a tidier IMO.
       */
      rltk8139_info->tx_desc_key[desc] = 0;

      /*
       * Increment the free descriptor count and go through the loop again
       * to see if more descriptors are ready.
       */
      cyg_drv_isr_lock();
      rltk8139_info->tx_num_free_desc += 1;
      cyg_drv_isr_unlock();
    }
  }

  if (status & IR_ROK) {
    /*
     * Received a frame. Note that '_deliver' does not actually copy any
     * data; it just determines how many bytes are available and tells
     * the generic ethernet driver about it, which then calls into
     * the '_recv' function to copy the data.
     */
    cyg_uint16 rx_pos;
    cyg_uint32 header, length;

    /*
     * CAPR contains the index into the receive buffer. It is controlled
     * completely in software. For some reason, CAPR points 16 bytes
     * before the actual start of the packet.
     */
    rx_pos = (INW(rltk8139_info->base_address + CAPR) + 16) % RX_BUF_LEN;

    /*
     * Loop until the rx buffer is empty. I have no idea how the 8139
     * determines if the buffer still contains a packet; it may check
     * that CAPR points 16 bytes before CBR.
     */
    while (!(INB(rltk8139_info->base_address + CR) & BUFE)) {
      /*
       * Invalidate the data cache for the cache line containing the header
       * if necessary.
       */
#ifdef CYGPKG_DEVS_ETH_RLTK_8139_SOFTWARE_CACHE_COHERENCY
      HAL_DCACHE_INVALIDATE(&rltk8139_info->rx_ring[rx_pos],
                            sizeof(cyg_uint32));
#endif

      /*
       * The 8139 prepends each packet with a 32 bit packet header that
       * contains a 16 bit length and 16 bit status field, in little-endian
       * byte order.
       */
      header = HAL_LE32TOC(*((volatile cyg_uint32 *)CYGARC_UNCACHED_ADDRESS(&rltk8139_info->rx_ring[rx_pos])));

      /*
       * If the 8139 is still copying data for this packet into the
       * receive ring, it will set packet length to 0xfff0. This shouldn't
       * ever happen because we do not use early receive.
       */
      if ((header >> 16) == 0xFFF0)
        break;

      /*
       * Since the 8139 appends the ethernet CRC to every packet, we
       * must subtract 4 from the length to get the true packet length.
       */
      length = (header >> 16) - 4;

      /*
       * Check if the packet was received correctly. The OpenBSD driver
       * resets the chip if this is not the case; we attempt to salvage
       * following packets by doing nothing.
       */
      if (!(header & HDR_ROK)) {
      }
      else {
        /*
         * Packet was received OK. Determine from where to start copying
         * bytes. This is saved in the driver private area so rlt8139_recv
         * doesn't have to redetermine this information. Then, inform
         * the generic ethernet driver about the packet.
         */
        rltk8139_info->rx_current = CYGARC_UNCACHED_ADDRESS(rltk8139_info->rx_ring + rx_pos + 4);
        rltk8139_info->rx_size = length;

        /* Tell eCos about the packet */
        (sc->funs->eth_drv->recv)(sc, length);
      }

      /*
       * Update CAPR. CAPR must be aligned to a 32 bit boundary, and should
       * point 16 bytes before it's actual position.
       */
      rx_pos = ((rx_pos + length + 8 + 3) & ~3) % RX_BUF_LEN;
      OUTW((rx_pos - 16) % RX_BUF_LEN, rltk8139_info->base_address + CAPR);
    }
  }

  if (status & IR_RXOVW) {
    /*
     * In case of a receive buffer overflow, the RealTek data sheet claims we
     * should update CAPR and then write a '1' to ROK in ISR. However, none of
     * the other 8139 drivers I have looked at do this, so we will just reset
     * the chip instead.
     */
#ifdef DEBUG_RLTK8139_DRIVER
    diag_printf("rltk8139_deliver(%s): receive buffer overflow\n",
                sc->dev_name);
#endif
    rltk8139_reset(sc);
    return;
  }

  if (status & IR_FOVW) {
    /*
     * Rx FIFO overflow. According to RealTek's data sheet, this is cleared
     * by writing a '1' to RXOVW. Again, none of the 8139 drivers I have
     * seen actually do this, so we reset the chip instead.
     */
#ifdef DEBUG_RLTK8139_DRIVER
    diag_printf("rltk8139_deliver(%s): receive FIFO overflow\n",
                sc->dev_name);
#endif
    rltk8139_reset(sc);
    return;
  }

#ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
  /* Finally, reenable interrupts */
#ifdef CYGPKG_DEVS_ETH_RLTK_8139_MASK_INTERRUPTS_IN_8139
  OUTW(RLTK8139_IRQ, rltk8139_info->base_address + IMR);
#else
  cyg_interrupt_unmask(rltk8139_info->vector);
#endif
#endif
}


/*
 * '_poll' does the same thing as '_deliver'. It is called periodically when
 * the ethernet driver is operated in non-interrupt mode, for instance by
 * RedBoot.
 */
static void
rltk8139_poll(struct eth_drv_sc *sc)
{
  Rltk8139_t *rltk8139_info;
  cyg_uint16 isr;


#ifdef DEBUG_RLTK8139_DRIVER
  diag_printf("rltk8139_poll(%s)\n", sc->dev_name);
#endif

  rltk8139_info = (Rltk8139_t *)(sc->driver_private);

  /*
   * Get the current interrupt status. If anything changed, call
   * _deliver.
   */
  if ((isr = INW(rltk8139_info->base_address + ISR)))
    rltk8139_deliver(sc);
}


/*
 * Return the interrupt vector used by this device.
 */
static int
rltk8139_int_vector(struct eth_drv_sc *sc)
{
  return ((Rltk8139_t *)(sc->driver_private))->vector;
}


/*
 * Quick and dirty register dump. This is somewhat dangerous, since
 * we read the register space 32 bits at a time, regardless of actual
 * register sizes.
 */
#ifdef DEBUG_RLTK8139_DRIVER
void
rltk8139_print_state(struct eth_drv_sc *sc) {
  int i;
  Rltk8139_t *rltk8139_info;


  rltk8139_info = (Rltk8139_t *)(sc->driver_private);

  for (i = IDR0; i < FFER; i += 16) {
    diag_printf("8139 reg address 0x%02x = 0x%08x", i,
                INL(rltk8139_info->base_address + i));
    diag_printf(" 0x%08x", INL(rltk8139_info->base_address + i + 4));
    diag_printf(" 0x%08x", INL(rltk8139_info->base_address + i + 8));
    diag_printf(" 0x%08x\n", INL(rltk8139_info->base_address + i + 12));
  }
}
#endif
