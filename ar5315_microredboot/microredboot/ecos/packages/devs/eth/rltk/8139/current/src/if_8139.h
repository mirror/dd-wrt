#ifndef CYGONCE_DEVS_ETH_REALTEK_8139_INFO_H
#define CYGONCE_DEVS_ETH_REALTEK_8139_INFO_H
/*==========================================================================
//
//        8139_info.h
//
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
// Author(s):     Eric Doenges
// Contributors:  Chris Nimmers, Gary Thomas, Andy Dyer
// Date:          2003-07-09
// Description:
//
//####DESCRIPTIONEND####
*/
#include <pkgconf/devs_eth_rltk_8139.h>


/*
 * Used to define all vendor/device ID combinations we know about to find
 * the chip.
 */
typedef struct {
  cyg_uint16 vendor_id;
  cyg_uint16 device_id;
  void *data;
} pci_identifier_t;

#define PCI_ANY_ID (0xFFFF)

/*
 * Valid receive buffer sizes are 8k+16, 16k+16, 32k+16, or 64k+16.
 * For the last case, WRAP mode should not be enabled. Since we do not
 * currently want to support unwrapped mode, do not use 64k+16 at this
 * point. The buffer length is set via the configuration mechanism.
 */
#if (CYGNUM_DEVS_ETH_RLTK_8139_RX_BUF_LEN_IDX < 0) |\
    (CYGNUM_DEVS_ETH_RLTK_8139_RX_BUF_LEN_IDX > 2)
#error "The receive ring size index must be in the range of 0 to 2"
#endif
#define RX_BUF_LEN      (8192 << (CYGNUM_DEVS_ETH_RLTK_8139_RX_BUF_LEN_IDX))
#define RX_BUF_PAD      16
#define RX_BUF_WRAP_PAD 2048 /* spare padding to handle lack of packet wrap */
#define RX_BUF_TOT_LEN  (RX_BUF_LEN + RX_BUF_PAD + RX_BUF_WRAP_PAD)

/* Number of Tx descriptor registers. */
#define NUM_TX_DESC     4

/*
 * Max supported ethernet frame size. The 8139 cannot transmit packets more
 * than 1792 bytes long. Also, since transmit buffers must be 32-bit
 * aligned, MAX_ETH_FRAME_SIZE should always be a multiple of 4.
 */
#define MIN_ETH_FRAME_SIZE      60      /* without FCS */
#define MAX_ETH_FRAME_SIZE      1536    /* is this with/without FCS ? */

/* Size of the Tx buffers. */
#define TX_BUF_SIZE     MAX_ETH_FRAME_SIZE
#define TX_BUF_TOT_LEN  (TX_BUF_SIZE * NUM_TX_DESC)

/* Rx buffer level before first PCI transfer ('5' is 512 bytes) */
#define RX_FIFO_THRESH	5

/*
 * Maximum PCI rx and tx bursts. This value ranges from '0' (16 bytes)
 * to '7' (unlimited), with MXDMA = 2^(4 + 'value').
 */
#define RX_DMA_BURST		6
#define TX_DMA_BURST		6

/*
 * Device driver private data
 */
typedef struct {
  /* Device number. Used for actually finding the device */
  cyg_uint32 device_num;

  /* Receive buffer ring area */
  cyg_uint8 *rx_ring;

  /* Transmit buffer area */
  cyg_uint8 *tx_buffer;

  /* PCI device ID */
  cyg_pci_device_id pci_device_id;

  /* Address for memory mapped I/O */
  cyg_uint32 base_address;

  /* Our current MAC address */
  unsigned char mac[6];

  /* tx FIFO threshold. */
  cyg_uint8 tx_threshold;

  /* This is the first free descriptor. */
  int tx_free_desc;

  /* This is the number of currently free descriptors */
  int tx_num_free_desc;

  /* Keys to match _send calls with the tx_done callback */
  unsigned long tx_desc_key[NUM_TX_DESC];

  /*
   * This is used to (temporarily) store the address of the current
   * received packet. We save it here to avoid having to calculate it
   * several times.
   */
  cyg_uint8 *rx_current;
  cyg_uint32 rx_size;

  /* Interrupt handling stuff */
  cyg_vector_t  vector;
  cyg_handle_t  interrupt_handle;
  cyg_interrupt interrupt;
} Rltk8139_t;


/*
 * Register offsets and bit definitions. These use the names in the 8139
 * data sheet, not those found e.g. in the Linux driver for the 8139.
 */
enum {
  IDR0    =  0x0, /* mac address, seemingly in big-endian order */
  IDR1    =  0x1,
  IDR2    =  0x2,
  IDR3    =  0x3,
  IDR4    =  0x4,
  IDR5    =  0x5,
  MAR0    =  0x8, /* multicast registers (0-7) */
  MAR1    =  0x9,
  MAR2    =  0xA,
  MAR3    =  0xB,
  MAR4    =  0xC,
  MAR5    =  0xD,
  MAR6    =  0xE,
  MAR7    =  0xF,
  TSD0    = 0x10, /* L, transmit status of descriptor 0 */
  TSD1    = 0x14,
  TSD2    = 0x18,
  TSD3    = 0x1C,
  TSAD0   = 0x20, /* L, transmit start address of descriptor 0 */
  TSAD1   = 0x24,
  TSAD2   = 0x28,
  TSAD3   = 0x2C,
  RBSTART = 0x30, /* L, receive buffer start address */
  ERBCR   = 0x34, /* W, early receive byte count register */
  ERSR    = 0x36, /* B, early receive status register */
  CR      = 0x37, /* B, command register */
  CAPR    = 0x38, /* W, current address of packet read */
  CBR     = 0x3A, /* W, current buffer address */
  IMR     = 0x3C, /* W, interrupt mask register */
  ISR     = 0x3E, /* W, interrupt status register */
  TCR     = 0x40, /* L, transmit configuration register */
  RCR     = 0x44, /* L, receive configuration register */
  TCTR    = 0x48, /* L, timer count register */
  MPC     = 0x4C, /* L, missed packet counter */
  CR9346  = 0x50, /* B, 93C46 (serial eeprom) command register */
  CONFIG0 = 0x51, /* B, configuration register 0 */
  CONFIG1 = 0x52, /* B, configuration register 1 */
  TIMERINT= 0x54, /* L, timer interrupt register */
  MSR     = 0x58, /* B, media status register */
  CONFIG3 = 0x59, /* B, configuration register 0 */
  CONFIG4 = 0x5A, /* B, configuration register 1 */
  MULINT  = 0x5C, /* W, multiple interrupt select */
  RERID   = 0x5E, /* B, PCI revision ID; should be 0x10 */
  TSAD    = 0x60, /* W, transmit status of all descriptors */
  BMCR    = 0x62, /* W, basic mode control register */
  BMSR    = 0x64, /* W, basic mode status register */
  ANAR    = 0x66, /* W, auto-negotiation advertisement register */
  ANLPAR  = 0x68, /* W, auto-negotiation link partner register */
  ANER    = 0x6A, /* W, auto-negotiation expansion register */
  DIS     = 0x6C, /* W, disconnect counter */
  FCSC    = 0x6E, /* W, false carrier sense counter */
  NWAYTR  = 0x70, /* W, N-way test register */
  REC     = 0x72, /* W, RX_ER counter */
  CSCR    = 0x74, /* W, CS configuration register */
  PHY1_PARM = 0x78, /* L, PHY parameter 1 */
  TW_PARM = 0x7C, /* L, twister parameter */
  PHY2_PARM = 0x80, /* B, PHY parameter 2 */
  CRC0    = 0x84, /* B, power management CRC register for wakeup frame 0 */
  CRC1    = 0x85, /* B, power management CRC register for wakeup frame 1 */
  CRC2    = 0x86, /* B, power management CRC register for wakeup frame 2 */
  CRC3    = 0x87, /* B, power management CRC register for wakeup frame 3 */
  CRC4    = 0x88, /* B, power management CRC register for wakeup frame 4 */
  CRC5    = 0x89, /* B, power management CRC register for wakeup frame 5 */
  CRC6    = 0x8A, /* B, power management CRC register for wakeup frame 6 */
  CRC7    = 0x8B, /* B, power management CRC register for wakeup frame 7 */
  WAKEUP0 = 0x8C, /* Q, power management wakeup frame 0 (64 bits) */
  WAKEUP1 = 0x94, /* Q, power management wakeup frame 1 (64 bits) */
  WAKEUP2 = 0x9C, /* Q, power management wakeup frame 2 (64 bits) */
  WAKEUP3 = 0xA4, /* Q, power management wakeup frame 3 (64 bits) */
  WAKEUP4 = 0xAC, /* Q, power management wakeup frame 4 (64 bits) */
  WAKEUP5 = 0xB4, /* Q, power management wakeup frame 5 (64 bits) */
  WAKEUP6 = 0xBC, /* Q, power management wakeup frame 6 (64 bits) */
  WAKEUP7 = 0xC4, /* Q, power management wakeup frame 7 (64 bits) */
  LSBCRC0 = 0xCC, /* B, LSB of mask byte of wakeup frame 0 offset 12 to 75 */
  LSBCRC1 = 0xCD, /* B, LSB of mask byte of wakeup frame 1 offset 12 to 75 */
  LSBCRC2 = 0xCE, /* B, LSB of mask byte of wakeup frame 2 offset 12 to 75 */
  LSBCRC3 = 0xCF, /* B, LSB of mask byte of wakeup frame 3 offset 12 to 75 */
  LSBCRC4 = 0xD0, /* B, LSB of mask byte of wakeup frame 4 offset 12 to 75 */
  LSBCRC5 = 0xD1, /* B, LSB of mask byte of wakeup frame 5 offset 12 to 75 */
  LSBCRC6 = 0xD2, /* B, LSB of mask byte of wakeup frame 6 offset 12 to 75 */
  LSBCRC7 = 0xD3, /* B, LSB of mask byte of wakeup frame 7 offset 12 to 75 */
  FLASH   = 0xD4, /* L, flash memory read/write register */
  CONFIG5 = 0xD8, /* B, configuration register #5 */
  FER     = 0xF0, /* L, function event register (CardBus only) */
  FEMR    = 0xF4, /* L, function event mask register (CardBus only) */
  FPSR    = 0xF8, /* L, function present state register (CardBus only) */
  FFER    = 0xFC  /* L, function force event register (CardBus only) */
};

/* Receive status register in Rx packet header */
enum {
  MAR  = (1<<15), /* multicast address received */
  PAM  = (1<<14), /* physical address matched */
  BAR  = (1<<13), /* broadcast address received */
  ISE  = (1<<5),  /* invalid symbol error */
  RUNT = (1<<4),  /* runt packet (<64 bytes) received */
  LONG = (1<<3),  /* long packet (>4K bytes) received */
  CRC  = (1<<2),  /* CRC error */
  FAE  = (1<<1),  /* frame alignment error */
  ROK  = (1<<0)   /* receive OK */
};

/* Transmit status register */
enum {
  CRS    = (1<<31), /* carrier sense lost */
  TABT   = (1<<30), /* transmit abort */
  OWC    = (1<<29), /* out of window collision */
  CDH    = (1<<28), /* CD heart beat */
  NCC_SHIFT = 24,
  NCC    = (0xF<<24), /* number of collision count */
  ERTXTH_SHIFT = 16,
  ERTXTH = (0x1F<<16), /* early tx threshold, in multiples of 32 bytes */
  TOK    = (1<<15), /* transmission OK */
  TUN    = (1<<14), /* transmit FIFO underrun */
  OWN    = (1<<13), /* own */
  SIZE   = 0xFFF    /* descriptor size */
};

/* Command register */
enum {
  RST  = (1<<4),  /* reset */
  RE   = (1<<3),  /* receiver enable */
  TE   = (1<<2),  /* transmitter enable */
  BUFE = (1<<0)   /* buffer empty */
};

/* Transmit configuration register */
enum {
  CLRABT = (1<<0) /* clear abort */
};

/* Receive configuration register */
enum {
  ERTH_SHIFT = 24,      /* Early Rx threshold bits (4) */
  MULERINT   = (1<<17), /* multiple early interrupt */
  RER8       = (1<<16), /* ? */
  RXFTH_SHIFT= 13,      /* Rx FIFO threshold */
  RBLEN_SHIFT= 11,      /* Rx buffer length */
  MXDMA_SHIFT= 8,       /* max DMA burst size per Rx DMA burst */
  WRAP       = (1<<7),  /* WRAP mode */
  SEL9356    = (1<<6),  /* EEPROM select */
  AER        = (1<<5),  /* accept error packets */
  AR         = (1<<4),  /* accept runt packets */
  AB         = (1<<3),  /* accept broadcast packets */
  AM         = (1<<2),  /* accept multicast packets */
  APM        = (1<<1),  /* accept physical match packets (our MAC) */
  AAP        = (1<<0),  /* accept physical address packets (any MAC) */
};

/* TSAD (transmit status of all descriptors */
enum {
  TOK3  = (1<<15),
  TOK2  = (1<<14),
  TOK1  = (1<<13),
  TOK0  = (1<<12),
  TUN3  = (1<<11),
  TUN2  = (1<<10),
  TUN1  = (1<<9),
  TUN0  = (1<<8),
  TABT3 = (1<<7),
  TABT2 = (1<<6),
  TABT1 = (1<<5),
  TABT0 = (1<<4),
  OWN3  = (1<<3),
  OWN2  = (1<<2),
  OWN1  = (1<<1),
  OWN0  = (1<<0)
};

/* Interrupt mask/status register */
enum {
  IR_SERR    = (1<<15), /* system error interrupt */
  IR_TIMEOUT = (1<<14), /* time out interrupt */
  IR_LENCHG  = (1<<13), /* cable length change interrupt */
  IR_FOVW    = (1<<6),  /* Rx FIFO overflow */
  IR_FUN     = (1<<5),  /* Packet underrun or link change interrupt */
  IR_RXOVW   = (1<<4),  /* Rx buffer overflow */
  IR_TER     = (1<<3),  /* transmit error interrupt */
  IR_TOK     = (1<<2),  /* transmit OK interrupt */
  IR_RER     = (1<<1),  /* receive error interrupt */
  IR_ROK     = (1<<0)   /* receive OK interrupt */
};

/* Packet header bits */
enum {
  HDR_MAR  = (1<<15), /* multicast address received */
  HDR_PAM  = (1<<14), /* physical address matched */
  HDR_BAR  = (1<<13), /* broadcast address matched */
  HDR_ISE  = (1<<5),  /* invalid symbol error */
  HDR_RUNT = (1<<4),  /* runt packet received (packet < 64 bytes) */
  HDR_LONG = (1<<3),  /* long packet (>4k) */
  HDR_CRC  = (1<<2),  /* CRC error */
  HDR_FAE  = (1<<1),  /* frame alignment error */
  HDR_ROK  = (1<<0)   /* receive OK */
};


/*
 * Define some options to use
 */
#define TXCFG ((0x3 << 24) | (TX_DMA_BURST << MXDMA_SHIFT))
#define RXCFG ((RX_FIFO_THRESH << RXFTH_SHIFT) |\
               (RX_BUF_LEN_IDX << RBLEN_SHIFT) |\
               (RX_DMA_BURST << MXDMA_SHIFT) |\
               WRAP | AB | APM)

#endif /* ifndef CYGONCE_DEVS_ETH_REALTEK_8139_INFO_H */
