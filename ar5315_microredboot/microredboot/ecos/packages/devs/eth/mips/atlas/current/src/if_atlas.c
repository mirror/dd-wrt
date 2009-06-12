//==========================================================================
//
//      dev/if_atlas.c
//
//      Ethernet device driver for MIPS Atlas using Philips SAA9730
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Nick Garnett <nickg@calivar.com>
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
// Alternative licenses for eCos may be arranged by contacting the copyright
// holders.
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
// Author(s):    msalter
// Contributors: msalter, nickg
// Date:         2000-12-06
// Purpose:      
// Description:  hardware driver for SAA9730 ethernet
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// Ethernet device driver for MIPS Atlas
// Based on SAA9730

#include <pkgconf/system.h>
#include <pkgconf/devs_eth_mips_atlas.h>
#include <pkgconf/io_eth_drivers.h>

#ifdef CYGPKG_NET
#include <pkgconf/net.h>
#include <cyg/kernel/kapi.h>
#endif
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_endian.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_if.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>

#ifdef CYGPKG_IO_PCI
#include <cyg/io/pci.h>
// So we can check the validity of the PCI window against the MLTs opinion,
// and thereby what the malloc heap consumes willy-nilly:
#include CYGHWR_MEMORY_LAYOUT_H
#else
#error "Need PCI package here"
#endif

#ifndef CYGSEM_MIPS_ATLAS_SET_ESA
#ifdef CYGPKG_REDBOOT
#include <pkgconf/redboot.h>
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
#include <redboot.h>
#include <flash_config.h>
RedBoot_config_option("Network hardware address [MAC]",
                      atlas_esa,
                      ALWAYS_ENABLED, true,
                      CONFIG_ESA, 0
    );
#endif
#endif
#endif

// SAA9730 LAN definitions
#include "saa9730.h"

// Exported statistics and the like
#include <cyg/io/eth/eth_drv_stats.h>

#ifndef CYGPKG_REDBOOT
//#define DEBUG
#endif
#define db_printf diag_printf

#define ETHER_ADDR_LEN 6

static unsigned poll_count = 0;  // for bug workaround
static void __tx_poll(struct eth_drv_sc *sc);

struct saa9730_priv_data {
    int			active;
    cyg_uint32		vector;
    cyg_handle_t  	interrupt_handle;
    cyg_interrupt	interrupt_object;
    cyg_uint32		devid;             // PCI device id
    cyg_uint32 		base;              // PCI memory map base
    void                *ndp;

    // index of next RX buffer
    cyg_uint8		next_rx_bindex;

    // index of next packet within RX buffer
    cyg_uint8		next_rx_pindex;

    // index of next TX buffer
    cyg_uint8		next_tx_bindex;

    // index of next packet within TX buffer
    cyg_uint8		next_tx_pindex;	

    cyg_uint32		*tx_buffer[SAA9730_BUFFERS][SAA9730_TXPKTS_PER_BUFFER];
    cyg_uint32		*rx_buffer[SAA9730_BUFFERS][SAA9730_RXPKTS_PER_BUFFER];

    int                 tx_busy;
    unsigned long	tx_key[SAA9730_BUFFERS][SAA9730_TXPKTS_PER_BUFFER];
    int			tx_used[SAA9730_BUFFERS];

} saa9730_priv_data;

ETH_DRV_SC(atlas_sc,
           &saa9730_priv_data, // Driver specific data
           "eth0",             // Name for this interface
           saa9730_start,
           saa9730_stop,
           saa9730_control,
           saa9730_can_send,
           saa9730_send,
           saa9730_recv,
           saa9730_deliver,     // "pseudoDSR" called from fast net thread
           saa9730_poll,
           saa9730_int_vector);

NETDEVTAB_ENTRY(atlas_netdev, 
                "atlas", 
                atlas_saa9730_init, 
                &atlas_sc);

#ifdef CYGSEM_MIPS_ATLAS_SET_ESA
static unsigned char enaddr[] = CYGDAT_MIPS_ATLAS_ESA;
#else
static unsigned char enaddr[ETHER_ADDR_LEN];
#endif

static void saa9730_poll(struct eth_drv_sc *sc);

// This ISR is called when the ethernet interrupt occurs
static int
saa9730_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    struct saa9730_priv_data *spd = (struct saa9730_priv_data *)data;
    unsigned long __base = spd->base;    

#ifndef CYGPKG_REDBOOT    
    SAA9730_EVM_IER_SW &= ~(SAA9730_EVM_LAN_INT|SAA9730_EVM_MASTER);
    SAA9730_EVM_ISR = SAA9730_EVM_LAN_INT;
    cyg_drv_interrupt_mask(vector);
#endif
#ifdef DEBUG
    db_printf("saa9730_isr\n");
#endif
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR);  // Run the DSR
}

#ifndef CYGPKG_REDBOOT
static 
void saa9730_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    struct saa9730_priv_data *spd = (struct saa9730_priv_data *)data;
    struct cyg_netdevtab_entry *ndp = (struct cyg_netdevtab_entry *)(spd->ndp);
    struct eth_drv_sc *sc = (struct eth_drv_sc *)(ndp->device_instance);
#ifdef DEBUG
    db_printf("saa9730_dsr\n");    
#endif

    eth_drv_dsr(vector, count, (cyg_addrword_t)sc);
}
#endif

static int
saa9730_int_vector(struct eth_drv_sc *sc)
{
    struct saa9730_priv_data *spd = (struct saa9730_priv_data *)sc->driver_private;

    return spd->vector;
}

static void
__init_buffers(struct saa9730_priv_data *spd)
{
    extern char cyg_io_atlas_2kbuffers[];
    cyg_uint32 *bufp = (cyg_uint32 *)CYGARC_UNCACHED_ADDRESS((unsigned)cyg_io_atlas_2kbuffers);
    int i, j;

    for (i = 0; i < SAA9730_BUFFERS; i++) {
        for (j = 0; j < SAA9730_RXPKTS_PER_BUFFER; j++) {
	    memset(bufp, 0, 2048);
            spd->rx_buffer[i][j]   = bufp;
            bufp += SAA9730_PACKET_SIZE/sizeof(*bufp);
        }
    }
    for (i = 0; i < SAA9730_BUFFERS; i++) {
        for (j = 0; j < SAA9730_TXPKTS_PER_BUFFER; j++) {
	    memset(bufp, 0, 2048);
	    *bufp = CYG_CPU_TO_LE32(TX_EMPTY);
            spd->tx_buffer[i][j] = bufp;
            bufp += SAA9730_PACKET_SIZE/sizeof(*bufp);
	}
    }

    spd->next_rx_pindex = 0;
    spd->next_rx_bindex = 0;
    spd->next_tx_pindex = 0;
    spd->next_tx_bindex = 0;
}

static void
__select_buffer(struct saa9730_priv_data *spd, int buf_nr)
{
    unsigned long __base = spd->base;
    cyg_uint32 *p;
    int i;

    // Enable RX buffer
    for (i = 0; i < SAA9730_RXPKTS_PER_BUFFER; i++) {
        p = spd->rx_buffer[buf_nr][i];
        *p = CYG_CPU_TO_LE32(RX_READY);
    }

    if (buf_nr)
        SAA9730_OK2USE |= SAA9730_OK2USE_RXB;
    else
        SAA9730_OK2USE |= SAA9730_OK2USE_RXA;

}

static void
__init_cam(struct saa9730_priv_data *spd)
{
    unsigned long __base = spd->base;
    cyg_uint32 abuf[3];  // room for 2 copies of mac address
    int i,j,cam_offset;

    // make 2 contiguous copies of mac addr
    memcpy((char *)abuf, enaddr, 6);
    memcpy((char *)abuf + 6, enaddr, 6);

    // Setting up the address compare regs is weird because you have
    // to access by word addresses even though addresses don't have
    // an integral number of words.
    cam_offset = 0;
    for (i = 0; i < SAA9730_CAM_ENTRIES; i++) {
	for (j = 0; j < 3; j++, cam_offset++) {
	    SAA9730_CAMADR = cam_offset;
	    SAA9730_CAMDAT = CYG_CPU_TO_BE32(abuf[j]);
	}
    }
}

static void
__stop_dma(struct saa9730_priv_data *spd)
{
    unsigned long __base = spd->base;

    // Stop DMA
    SAA9730_DMACTL &= ~(SAA9730_DMACTL_ENRX | SAA9730_DMACTL_ENTX);

    // Stop tx/rx
    SAA9730_TXCTL &= ~SAA9730_TXCTL_ENTX;
    SAA9730_RXCTL &= ~SAA9730_RXCTL_ENRX;

    // Set DMA and MAC reset bits
    SAA9730_DMATST |= SAA9730_DMATST_RESET;
    SAA9730_MACCTL |= SAA9730_MACCTL_RESET;
}


static void
__init_dma(struct saa9730_priv_data *spd)
{
    unsigned long __base = spd->base;

    __stop_dma(spd);

    // reset DMA engine
    SAA9730_DMATST |= SAA9730_DMATST_RESET;

    // setup buffers
    SAA9730_TXBUFA = CYGARC_PHYSICAL_ADDRESS((unsigned long)spd->tx_buffer[0][0]);
    SAA9730_TXBUFB = CYGARC_PHYSICAL_ADDRESS((unsigned long)spd->tx_buffer[1][0]);
    SAA9730_RXBUFA = CYGARC_PHYSICAL_ADDRESS((unsigned long)spd->rx_buffer[0][0]);
    SAA9730_RXBUFB = CYGARC_PHYSICAL_ADDRESS((unsigned long)spd->rx_buffer[1][0]);

    SAA9730_PKTCNT = ((SAA9730_TXPKTS_PER_BUFFER << 24) |
		      (SAA9730_TXPKTS_PER_BUFFER << 16) |
		      (SAA9730_RXPKTS_PER_BUFFER <<  8) |
		      (SAA9730_RXPKTS_PER_BUFFER <<  0));

    SAA9730_OK2USE = 0;

    __select_buffer(spd, 0);

    // initialize DMA control register
    SAA9730_DMACTL = SAA9730_DMACTL_BLKINT |
	             SAA9730_DMACTL_MAXXFER_ANY |
	             SAA9730_DMACTL_ENDIAN_LITTLE;

    SAA9730_DMACTL |= SAA9730_DMACTL_RXINT;
    SAA9730_DMACTL |= (1<<SAA9730_DMACTL_RXINTCNT_SHIFT);
    SAA9730_DMACTL &= ~SAA9730_DMACTL_BLKINT;

#ifndef CYGPKG_REDBOOT
    SAA9730_DMACTL |= SAA9730_DMACTL_TXINT;
#endif
    
    SAA9730_TIMOUT = 200;

    // accept broadcast packets */
    SAA9730_CAMCTL = SAA9730_CAMCTL_BROADCAST |
	             SAA9730_CAMCTL_COMPARE;

    SAA9730_TXCTL = 0;
    SAA9730_RXCTL |= SAA9730_RXCTL_STRIPCRC;

    SAA9730_CAMENA = 1;

}

static void
__check_mii(struct saa9730_priv_data *spd)
{
    unsigned long __base = spd->base;
    cyg_uint32 opmode;

#ifdef DEBUG
    db_printf("__check_mii\n");    
#endif
    
    // spin till station is not busy
    while (SAA9730_MDCTL & SAA9730_MDCTL_BUSY)
	;

    // set PHY address = 'STATUS'
    SAA9730_MDCTL = SAA9730_MDCTL_BUSY |
	            (PHY_ADDRESS << SAA9730_MDCTL_PHY_SHIFT)  |
	            PHY_STATUS;

    // spin till station is not busy
    while (SAA9730_MDCTL & SAA9730_MDCTL_BUSY)
	;

    hal_delay_us(1000);

    // check the link status
    if (SAA9730_MDDATA & PHY_STATUS_LINK_UP) {

	SAA9730_MDCTL = SAA9730_MDCTL_BUSY |
	                (PHY_ADDRESS << SAA9730_MDCTL_PHY_SHIFT)  |
	                PHY_REG31;

	// spin till station is not busy
	while (SAA9730_MDCTL & SAA9730_MDCTL_BUSY)
	    ;

	hal_delay_us(1000);

        opmode = (SAA9730_MDDATA & PHY_REG31_OPMODE_MSK) >> PHY_REG31_OPMODE_SHIFT;

#ifdef DEBUG
	db_printf("MII mode %d\n", opmode);
#endif

	if ((opmode == OPMODE_10BASET_FULLDUPLEX) ||
	    (opmode == OPMODE_100BASEX_FULLDUPLEX))
            SAA9730_MACCTL = SAA9730_MACCTL_CONMODE_FORCE_MII | SAA9730_MACCTL_FULLDUP;
        else
            SAA9730_MACCTL = SAA9730_MACCTL_CONMODE_FORCE_MII;
    }
#ifdef DEBUG
    else
	db_printf("Link is down\n");
#endif
}


static void
saa9730_reset(struct saa9730_priv_data *spd)
{
    unsigned long __base = spd->base;

    __init_buffers(spd);

    __base = spd->base;
    
    // Stop DMA
    SAA9730_DMACTL &= ~(SAA9730_DMACTL_ENRX | SAA9730_DMACTL_ENTX);

    // Stop tx/rx
    SAA9730_TXCTL &= ~SAA9730_TXCTL_ENTX;
    SAA9730_RXCTL &= ~SAA9730_RXCTL_ENRX;

    // Set DMA and MAC reset bits
    SAA9730_DMATST |= SAA9730_DMATST_RESET;
    SAA9730_MACCTL |= SAA9730_MACCTL_RESET;

    __init_cam(spd);
    __init_dma(spd);
    __check_mii(spd);

    spd->tx_busy = 0;
}


static bool 
atlas_saa9730_init(struct cyg_netdevtab_entry *tab)
{
    static int initialized = 0; // only probe PCI et al *once*
    struct eth_drv_sc *sc = (struct eth_drv_sc *)tab->device_instance;
    struct saa9730_priv_data *spd = (struct saa9730_priv_data *)sc->driver_private;

#ifdef DEBUG
    db_printf("atlas_saa9730_init\n");
#endif

    if (0 == initialized) {
	cyg_pci_device_id devid;
	cyg_pci_device dev_info;

	cyg_pci_init();

	devid = CYG_PCI_NULL_DEVID;

        if (cyg_pci_find_device(CYG_PCI_VENDOR_PHILIPS, 0x9730, &devid) ) {

	    spd->devid = devid;

            cyg_pci_get_device_info(devid, &dev_info);

            if (!cyg_pci_configure_device(&dev_info)) {
#ifdef DEBUG
                db_printf("Failed to configure eth device\n");
#endif
		return false;
	    }

	    //  Philips SAA9730 implements only one function memory mapped
	    //  into a single contigous memory region.
	    //
            //  According to spec. the BAR#1 is to be used for memory mapped IO.
	    spd->base = dev_info.base_map[1];

	    // FIXME! All IO units share an interrupt
	    spd->vector = CYGNUM_HAL_INTERRUPT_INTB;

	    // Setup timing stuff
	    cyg_hal_plf_pci_cfg_write_byte(CYG_PCI_DEV_GET_BUS(devid),
					   CYG_PCI_DEV_GET_DEVFN(devid),
		                           CYG_PCI_CFG_LATENCY_TIMER, 0x20);
	    cyg_hal_plf_pci_cfg_write_byte(CYG_PCI_DEV_GET_BUS(devid),
					   CYG_PCI_DEV_GET_DEVFN(devid),
		                           CYG_PCI_CFG_MIN_GNT, 9);
	    cyg_hal_plf_pci_cfg_write_byte(CYG_PCI_DEV_GET_BUS(devid),
					   CYG_PCI_DEV_GET_DEVFN(devid),
		                           CYG_PCI_CFG_MAX_LAT, 24);
	    
#ifdef DEBUG
            db_printf("eth0 found: bus[%d] dev[%d] base[%x] vector[%d]\n",
		      CYG_PCI_DEV_GET_BUS(devid),
		      CYG_PCI_DEV_GET_DEV(CYG_PCI_DEV_GET_DEVFN(devid)),
		      spd->base, spd->vector);
#endif

            spd->ndp = tab;
            
#ifndef CYGPKG_REDBOOT
	    cyg_drv_interrupt_create(
		    spd->vector,
                    0,                  // Priority - unused
                    (CYG_ADDRWORD)spd,  // Data item passed to ISR & DSR
                    saa9730_isr,            // ISR
                    saa9730_dsr,            // DSR
                    &spd->interrupt_handle, // handle to intr obj
                    &spd->interrupt_object ); // space for int obj

	    cyg_drv_interrupt_attach(spd->interrupt_handle);
	    cyg_drv_interrupt_acknowledge(spd->vector);
	    cyg_drv_interrupt_unmask(spd->vector);
#endif
            {
                // When in Redboot we want to get RX interrupts. These
                // will be picked up by the default interrupt handler and
                // checked for ^C.
                unsigned long __base = spd->base;
                SAA9730_EVM_IER_SW |= (SAA9730_EVM_LAN_INT|SAA9730_EVM_MASTER);                
                SAA9730_EVM_IER |= (SAA9730_EVM_LAN_INT|SAA9730_EVM_MASTER);
                SAA9730_EVM_ISR |= (SAA9730_EVM_LAN_INT|SAA9730_EVM_MASTER);
            }

#ifdef DEBUG
	    db_printf(" **** Device enabled for I/O and Memory and Bus Master\n");
#endif

        } else {
#ifdef DEBUG
            db_printf("eth0 not found\n");
#endif
        }

        saa9730_stop(sc);
        
	spd->active = 0;

	initialized = 1;
    }

    // Fetch hardware address
#if defined(CYGPKG_REDBOOT) && \
    defined(CYGSEM_REDBOOT_FLASH_CONFIG) && \
    !defined(CYGSEM_MIPS_ATLAS_SET_ESA)
    flash_get_config("atlas_esa", enaddr, CONFIG_ESA);
#else
#define CONFIG_ESA     6
    CYGACC_CALL_IF_FLASH_CFG_OP( CYGNUM_CALL_IF_FLASH_CFG_GET,
                                 "atlas_esa", enaddr, CONFIG_ESA );
#ifdef DEBUG
    db_printf("ESA: %02x:%02x:%02x:%02x:%02x:%02x\n",
              enaddr[0],enaddr[1],enaddr[2],enaddr[3],enaddr[4],enaddr[5]);
#endif
#endif

    saa9730_reset(spd);

    // Initialize upper level driver
    (sc->funs->eth_drv->init)(sc, enaddr);

    return true;
}

static void
saa9730_stop(struct eth_drv_sc *sc)
{
    struct saa9730_priv_data *spd = (struct saa9730_priv_data *)sc->driver_private;
    unsigned long __base = spd->base;

    // Stop DMA
    SAA9730_DMACTL &= ~(SAA9730_DMACTL_ENRX | SAA9730_DMACTL_ENTX);

    // Stop tx/rx
    SAA9730_TXCTL &= ~SAA9730_TXCTL_ENTX;
    SAA9730_RXCTL &= ~SAA9730_RXCTL_ENRX;

    // Set DMA and MAC reset bits
    SAA9730_DMATST |= SAA9730_DMATST_RESET;
    SAA9730_MACCTL |= SAA9730_MACCTL_RESET;

    spd->active = 0;
}

static void
__do_start(struct saa9730_priv_data *spd)
{
    unsigned long __base = spd->base;
    int i;

    spd->active = 1;
    spd->tx_busy = 0;

    for (i = 0; i < SAA9730_BUFFERS; i++)
	spd->tx_used[i] = 0;

    // for tx, turn on MAC first
    SAA9730_TXCTL |= SAA9730_TXCTL_ENTX;
    SAA9730_DMACTL |= SAA9730_DMACTL_ENTX;

    // for rx, turn on DMA first
    SAA9730_DMACTL |= SAA9730_DMACTL_ENRX;
    SAA9730_RXCTL |= SAA9730_RXCTL_ENRX;

    __select_buffer(spd, spd->next_rx_bindex);    
}

//
// This function is called to "start up" the interface.  It may be called
// multiple times, even when the hardware is already running.  It will be
// called whenever something "hardware oriented" changes and should leave
// the hardware ready to send/receive packets.
//
static void
saa9730_start(struct eth_drv_sc *sc, unsigned char *enaddr, int flags)
{
    struct saa9730_priv_data *spd = (struct saa9730_priv_data *)sc->driver_private;

    if (spd->active)
	saa9730_stop(sc);

    __do_start(spd);
}

//
// This routine is called to perform special "control" opertions
//
static int
saa9730_control(struct eth_drv_sc *sc, unsigned long key,
               void *data, int data_length)
{
    switch (key) {
    case ETH_DRV_SET_MAC_ADDRESS:
        return 0;
        break;
    default:
        return 1;
        break;
    }
}

//
// This routine is called to see if it is possible to send another packet.
// It will return non-zero if a transmit is possible, zero otherwise.
//
static int
saa9730_can_send(struct eth_drv_sc *sc)
{
    struct saa9730_priv_data *spd = (struct saa9730_priv_data *)sc->driver_private;
    unsigned long __base = spd->base;

    __tx_poll(sc);

    if (spd->next_tx_bindex == 0  && (SAA9730_OK2USE & SAA9730_OK2USE_TXA))
        return 0;

    if (spd->next_tx_bindex == 1  && (SAA9730_OK2USE & SAA9730_OK2USE_TXB))
        return 0;

    return 1;
}


static int tx_poll_count;

//
// This routine is called to send data to the hardware.
static void 
saa9730_send(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len, 
            int total_len, unsigned long key)
{
    struct saa9730_priv_data *spd = (struct saa9730_priv_data *)sc->driver_private;
    unsigned long __base = spd->base;
    int bindex, pindex;
    cyg_uint32 pktlen = total_len;
    cyg_uint8  *pktdata, *to_p;
    volatile cyg_uint32 *pktstat ;
    struct eth_drv_sg *last_sg;

#ifdef DEBUG
    db_printf("saa9730_send: %d sg's, %d bytes, KEY %x\n",
              sg_len, total_len, key );
#endif

    if (!spd->active)
	return;

    bindex = spd->next_tx_bindex;
    pindex = spd->next_tx_pindex;

    spd->next_tx_pindex++;
    if (spd->next_tx_pindex >= SAA9730_TXPKTS_PER_BUFFER) {
        spd->next_tx_pindex = 0;
        spd->next_tx_bindex ^= 1;
    }

    pktstat = spd->tx_buffer[bindex][pindex];
	
    if (bindex == 0 && (SAA9730_OK2USE & SAA9730_OK2USE_TXA))
        return;

    if (bindex == 1 && (SAA9730_OK2USE & SAA9730_OK2USE_TXB))
        return;

    spd->tx_key[bindex][pindex] = key;
    spd->tx_used[bindex] += 1;

    pktdata = (cyg_uint8 *)((unsigned)pktstat + 4);

    // Copy from the sglist into the tx buffer
    to_p = pktdata;

    for (last_sg = &sg_list[sg_len]; sg_list < last_sg; sg_list++) {
	cyg_uint8 *from_p;
	int l;
            
	from_p = (cyg_uint8 *)(sg_list->buf);
	l = sg_list->len;

	if (l > total_len)
	    l = total_len;

	memcpy((unsigned char *)to_p, from_p, l);
	to_p += l;
	total_len -= l;

	if (total_len < 0) 
	    break; // Should exit via sg_last normally
    }

    // pad to minimum size
    if (pktlen < SAA9730_MIN_PACKET_SIZE) {
        memset(to_p, 0, SAA9730_MIN_PACKET_SIZE-pktlen);
        pktlen = SAA9730_MIN_PACKET_SIZE;
    }

    // Set transmit status WORD for hardware (LAN-DMA-ENGINE)
    *pktstat = CYG_CPU_TO_LE32(TX_READY | pktlen);

    // start hardware
    if (bindex == 0)
        SAA9730_OK2USE |= SAA9730_OK2USE_TXA;
    else 
        SAA9730_OK2USE |= SAA9730_OK2USE_TXB;

    if (!spd->tx_busy) {
	tx_poll_count = 0;
	spd->tx_busy = bindex + 1;
    }
}


static void
__check_rxstate(struct saa9730_priv_data *spd)
{
    unsigned long __base = spd->base;
    cyg_uint32  status, flag, size;
    cyg_uint32  *pkt;
    int         i, j;

#ifdef DEBUG
    db_printf("__check_rxstate\n");        
#endif

#ifdef CYGPKG_REDBOOT
    // Clear SAA9730 LAN interrupt and re-enable interrupts.
    SAA9730_EVM_ISR = SAA9730_EVM_LAN_INT;
    SAA9730_EVM_IER_SW |= (SAA9730_EVM_LAN_INT|SAA9730_EVM_MASTER);
#endif
    
    if ((SAA9730_DBGRXS & SAA9730_DBGRXS_RXDII_MASK) == SAA9730_DBGRXS_RXDII_ERROR) {
        // re-init driver and controller
#ifdef DEBUG
        db_printf("DBGRXS: reset\n");
#endif
	saa9730_reset(spd);
	__do_start(spd);
	return;
    }

    // Check RX packet status
    for (i = 0; i < SAA9730_BUFFERS; i++) {
        for (j = 1; j < SAA9730_RXPKTS_PER_BUFFER; j++) {
            pkt = spd->rx_buffer[i][j];
            status = CYG_LE32_TO_CPU(*pkt);
            size   = status & RXPACKET_STATUS_SIZE_MASK;
            flag   = status & RXPACKET_STATUS_FLAG_MASK;
            if (flag == RX_INVALID_STAT || size > 1514 || *(pkt - 1)) {
		// re-init driver and controller
#ifdef DEBUG
                db_printf("rxpkt: reset\n");
#endif
		saa9730_reset(spd);
		__do_start(spd);
		return;
            }
        }
    }
}


static void
__tx_poll(struct eth_drv_sc *sc)
{
    struct saa9730_priv_data *spd = (struct saa9730_priv_data *)sc->driver_private;
    int	                 bindex, pindex;
    volatile cyg_uint32 *pktstat;
    cyg_uint32 status;

    if (!spd->tx_busy)
	return;

    bindex = spd->tx_busy - 1;
    pindex = spd->tx_used[bindex] - 1;  // watch last pkt in buffer

    pktstat = spd->tx_buffer[bindex][pindex];

    status = CYG_LE32_TO_CPU(*pktstat);
    if ((status & TXPACKET_STATUS_FLAG_MASK) != TX_HWDONE) {

	hal_delay_us(1000);

	if (++tx_poll_count > 1000) {
	    // reset

	    for (pindex = 0; pindex < spd->tx_used[bindex]; pindex++)
		(sc->funs->eth_drv->tx_done)(sc, spd->tx_key[bindex][pindex], 1);

	    bindex ^= 1;

	    for (pindex = 0; pindex < spd->tx_used[bindex]; pindex++)
		(sc->funs->eth_drv->tx_done)(sc, spd->tx_key[bindex][pindex], 1);

	    saa9730_reset(spd);
	    __do_start(spd);
	}
	return;
    }

    for (pindex = 0; pindex < spd->tx_used[bindex]; pindex++) {
	/* Check for error. */
	pktstat = spd->tx_buffer[bindex][pindex];
	status = CYG_LE32_TO_CPU(*pktstat);

	if (status & TXPACKET_STATUS_ERROR) {
	    if (status & TXPACKET_STATUS_EXDEFER)
		db_printf("tx deferred\n");

	    if (status & TXPACKET_STATUS_LATECOLLERR)
		db_printf("tx late collision\n");

	    if (status & TXPACKET_STATUS_LOSTCARRIER)
		db_printf("tx no carrier\n");

	    if (status & TXPACKET_STATUS_UNDERRUN)
		db_printf("tx underrun\n");

	    if (status & TXPACKET_STATUS_SQERR)
		db_printf("tx sq\n");
	}
	/* free the space */
	*pktstat = CYG_CPU_TO_LE32(TX_EMPTY);

	(sc->funs->eth_drv->tx_done)(sc, spd->tx_key[bindex][pindex], 1 /* status */);
    }

    tx_poll_count = 0;
    spd->tx_used[bindex] = 0;
    
    bindex ^= 1;
    if (spd->tx_used[bindex])
	spd->tx_busy = bindex + 1;
    else
	spd->tx_busy = 0;
}


static void
__rx_poll(struct eth_drv_sc *sc)
{
    struct saa9730_priv_data *spd = (struct saa9730_priv_data *)sc->driver_private;
    int	                 bindex, pindex, done;
    volatile cyg_uint32 *pkt;
    cyg_uint32           status, pktlen; 

#ifdef DEBUG
    db_printf("__rx_poll\n");
#endif
    if (!spd->active)
	return;

    done = 0;
    while (!done) {
        // index of next packet buffer
        bindex = spd->next_rx_bindex;

        // index of next packet within buffer
        pindex = spd->next_rx_pindex;

        pkt = spd->rx_buffer[bindex][pindex];

	// stop now if no more packets
        if (((status = CYG_LE32_TO_CPU(*pkt)) & RXPACKET_STATUS_FLAG_MASK) == RX_READY)
            break;
#ifdef DEBUG
        db_printf("__rx_poll pkt %08x status %08x\n",pkt,status);
#endif
	// if this is the first packet in a buffer, switch the SAA9730 to
	// use the next buffer for subsequent incoming packets.
	if (pindex == 0)
	    __select_buffer(spd, bindex == 0);
            
	// check for good packet
	if (status & RXPACKET_STATUS_GOOD) {

	    pktlen = status & RXPACKET_STATUS_SIZE_MASK ;

	    if (pktlen > 0) {
		(sc->funs->eth_drv->recv)(sc, pktlen);
		// done = 1;
	    }
	}
#ifdef DEBUG
        else
            db_printf("rx bad: %08x %08x\n",pkt,status);
#endif
        
	/* go to next packet in sequence */
	spd->next_rx_pindex++;
	if (spd->next_rx_pindex >= SAA9730_RXPKTS_PER_BUFFER) {
	    spd->next_rx_pindex = 0;
	    spd->next_rx_bindex++;
	    if (spd->next_rx_bindex >= SAA9730_BUFFERS) 
		spd->next_rx_bindex = 0;
	}
    }

    if (((poll_count++) % 100) == 0)
        __check_rxstate(spd);
}


static void
saa9730_recv(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len)
{
    struct saa9730_priv_data *spd = (struct saa9730_priv_data *)sc->driver_private;
    volatile cyg_uint32 *pkt;
    cyg_uint32          pktlen, status;
    struct eth_drv_sg   *last_sg;

    if (!spd->active)
	return;

    pkt = spd->rx_buffer[spd->next_rx_bindex][spd->next_rx_pindex];

    status = CYG_LE32_TO_CPU(*pkt);
    if (status & RXPACKET_STATUS_GOOD) {
	// packet is good
	pktlen = status & RXPACKET_STATUS_SIZE_MASK;

	if (pktlen > 0) {
	    int total_len;
	    cyg_uint8 *from_p;

	    // check we have memory to copy into; we would be called even if
	    // caller was out of memory in order to maintain our state.
	    if (0 == sg_len || 0 == sg_list)
		return; // caller was out of mbufs

	    total_len = pktlen;
	    from_p = (cyg_uint8 *)((unsigned)pkt + 4);

	    for (last_sg = &sg_list[sg_len]; sg_list < last_sg; sg_list++) {
		cyg_uint8 *to_p;
		int l;
            
		to_p = (cyg_uint8 *)(sg_list->buf);
		l = sg_list->len;

		if (0 >= l || 0 == to_p)
		    return; // caller was out of mbufs

		if (l > total_len)
		    l = total_len;

		memcpy(to_p, (unsigned char *)from_p, l);
		from_p += l;
		total_len -= l;
	    }
	}
    }
}

static inline void
__do_deliver(struct eth_drv_sc *sc)
{
    // First pass any rx data up the stack
    __rx_poll(sc);

    // Then scan for completed Txen and inform the stack
    __tx_poll(sc);
}

static void
saa9730_poll(struct eth_drv_sc *sc)
{
    struct saa9730_priv_data *spd = (struct saa9730_priv_data *)sc->driver_private;

#ifndef CYGPKG_REDBOOT
    cyg_drv_interrupt_mask(spd->vector);
#endif

    (void)saa9730_isr(spd->vector, (cyg_addrword_t)spd);

    __do_deliver(sc);

    cyg_drv_interrupt_acknowledge(spd->vector);

#ifndef CYGPKG_REDBOOT
    cyg_drv_interrupt_unmask(spd->vector);
#endif    
}


// The deliver function (ex-DSR)  handles the ethernet [logical] processing
static void
saa9730_deliver(struct eth_drv_sc *sc)
{
    struct saa9730_priv_data *spd = (struct saa9730_priv_data *)sc->driver_private;
    unsigned long __base = spd->base;    

    if (spd->active)
	__do_deliver(sc);

    cyg_drv_interrupt_acknowledge(spd->vector);

#ifndef CYGPKG_REDBOOT
    // Clear SAA9730 LAN interrupt and re-enable interrupts.
    SAA9730_EVM_IER_SW |= (SAA9730_EVM_LAN_INT|SAA9730_EVM_MASTER);
    cyg_drv_interrupt_unmask(spd->vector);
#endif    
}


