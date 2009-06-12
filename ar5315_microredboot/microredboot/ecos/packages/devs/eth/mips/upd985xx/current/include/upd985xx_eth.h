#ifndef CYGONCE_HAL_UPD985XX_ETH_H
#define CYGONCE_HAL_UPD985XX_ETH_H
//==========================================================================
//
//      upd985xx_eth.h
//
//      Architecture specific abstractions for the on-chip ethernet
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
// Author(s):    hmt, nickg
// Contributors: nickg
// Date:         2001-06-28
// Purpose:      Define architecture abstractions
// Description:  This file contains any extra or modified definitions for
//               this variant of the architecture's ethernet controller.
// Usage:        #include <cyg/io/upd985xx_eth.h>
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/hal/var_arch.h>

// --------------------------------------------------------------------------
// By default we use the definition of UPD985XX_SYSETH_REG( n ) from
// var_arch.h - if we port to a KORVA with multiple ethernet controllers we
// will have to vary this to account for the different base addresses.

// (the noise at the end of these lines is the default value)
//
//      Table 5-2.  MAC Control Register Map
//
#define ETH_MACC1  UPD985XX_SYSETH_REG( 0x000)   // MAC configuration register 1    R/W    0000_0000H
#define ETH_MACC2  UPD985XX_SYSETH_REG( 0x004)   // MAC configuration register 2    R/W    0000_0000H
#define ETH_IPGT   UPD985XX_SYSETH_REG( 0x008)   // Back-to-Back IPG register    R/W    0000_0013H
#define ETH_IPGR   UPD985XX_SYSETH_REG( 0x00C)   // Non Back-to-Back IPG register    R/W    0000_0E13H
#define ETH_CLRT   UPD985XX_SYSETH_REG( 0x010)   // Collision register    R/W    0000_370FH
#define ETH_LMAX   UPD985XX_SYSETH_REG( 0x014)   // Max packet length register    R/W    0000_0600H
//          N/A    UPD985XX_SYSETH_REG( 0x018)   // Reserved for future use    -    -
#define ETH_RETX   UPD985XX_SYSETH_REG( 0x020)   // Retry count register    R/W    0000_0000H
//          N/A    UPD985XX_SYSETH_REG( 0x024)   // Reserved for future use    -    -
#define ETH_LSA2   UPD985XX_SYSETH_REG( 0x054)   // Station Address register 2    R/W    0000_0000H
#define ETH_LSA1   UPD985XX_SYSETH_REG( 0x058)   // Station Address register 1    R/W    0000_0000H
#define ETH_PTVR   UPD985XX_SYSETH_REG( 0x05C)   // Pause timer value read register    R    0000_0000H
//      N/A        UPD985XX_SYSETH_REG( 0x060)   // Reserved for future use    -    -
#define ETH_VLTP   UPD985XX_SYSETH_REG( 0x064)   // VLAN type register    R/W    0000_0000H
#define ETH_MIIC   UPD985XX_SYSETH_REG( 0x080)   // MII configuration register    R/W    0000_0000H
//          N/A    UPD985XX_SYSETH_REG( 0x084)   // Reserved for future use    -    -
#define ETH_MCMD   UPD985XX_SYSETH_REG( 0x094)   // MII command register    W    0000_0000H
#define ETH_MADR   UPD985XX_SYSETH_REG( 0x098)   // MII address register    R/W    0000_0000H
#define ETH_MWTD   UPD985XX_SYSETH_REG( 0x09C)   // MII write data register    R/W    0000_0000H
#define ETH_MRDD   UPD985XX_SYSETH_REG( 0x0A0)   // MII read data register    R    0000_0000H
#define ETH_MIND   UPD985XX_SYSETH_REG( 0x0A4)   // MII indicator register    R    0000_0000H
//          N/A    UPD985XX_SYSETH_REG( 0x0A8)   // Reserved for future use    -    -
#define ETH_AFR    UPD985XX_SYSETH_REG( 0x0C8)   // Address Filtering register    R/W    0000_0000H
#define ETH_HT1    UPD985XX_SYSETH_REG( 0x0CC)   // Hash table register 1    R/W    0000_0000H
#define ETH_HT2    UPD985XX_SYSETH_REG( 0x0D0)   // Hash table register 2    R/W    0000_0000H
//          N/A    UPD985XX_SYSETH_REG( 0x0D4)   // Reserved for future use    -    -
#define ETH_CAR1   UPD985XX_SYSETH_REG( 0x0DC)   // Carry register 1    R/W    0000_0000H
#define ETH_CAR2   UPD985XX_SYSETH_REG( 0x0E0)   // Carry register 2    R/W    0000_0000H
//           N/A   UPD985XX_SYSETH_REG( 0x0E4)   // Reserved for future use    -    -
#define ETH_CAM1   UPD985XX_SYSETH_REG( 0x130)   // Carry mask register 1    R/W    0000_0000H
#define ETH_CAM2   UPD985XX_SYSETH_REG( 0x134)   // Carry mask register 2    R/W    0000_0000H
//           N/A   UPD985XX_SYSETH_REG( 0x138)   // Reserved for future use    -    -
//
//      Table 5-3.  Statistics Counter Register Map
//
#define ETH_RBYT   UPD985XX_SYSETH_REG( 0x140)   // Receive Byte Counter    R/W
#define ETH_RPKT   UPD985XX_SYSETH_REG( 0x144)   // Receive Packet Counter    R/W
#define ETH_RFCS   UPD985XX_SYSETH_REG( 0x148)   // Receive FCS Error Counter    R/W
#define ETH_RMCA   UPD985XX_SYSETH_REG( 0x14C)   // Receive Multicast Packet Counter    R/W
#define ETH_RBCA   UPD985XX_SYSETH_REG( 0x150)   // Receive Broadcast Packet Counter    R/W
#define ETH_RXCF   UPD985XX_SYSETH_REG( 0x154)   // Receive Control Frame Packet Counter    R/W
#define ETH_RXPF   UPD985XX_SYSETH_REG( 0x158)   // Receive PAUSE Frame Packet Counter    R/W
#define ETH_RXUO   UPD985XX_SYSETH_REG( 0x15C)   // Receive Unknown OP code Counter    R/W
#define ETH_RALN   UPD985XX_SYSETH_REG( 0x160)   // Receive Alignment Error Counter    R/W
#define ETH_RFLR   UPD985XX_SYSETH_REG( 0x164)   // Receive Frame Length Out of Range Counter    R/W
#define ETH_RCDE   UPD985XX_SYSETH_REG( 0x168)   // Receive Code Error Counter    R/W
#define ETH_RFCR   UPD985XX_SYSETH_REG( 0x16C)   // Receive False Carrier Counter    R/W
#define ETH_RUND   UPD985XX_SYSETH_REG( 0x170)   // Receive Undersize Packet Counter    R/W
#define ETH_ROVR   UPD985XX_SYSETH_REG( 0x174)   // Receive Oversize Packet Counter    R/W
#define ETH_RFRG   UPD985XX_SYSETH_REG( 0x178)   // Receive Error Undersize Packet Counter    R/W
#define ETH_RJBR   UPD985XX_SYSETH_REG( 0x17C)   // Receive Error Oversize Packet Counter    R/W
#define ETH_R64    UPD985XX_SYSETH_REG( 0x180)   // Receive 64 Byte Frame Counter    R/W
#define ETH_R127   UPD985XX_SYSETH_REG( 0x184)   // Receive 65 to 127 Byte Frame Counter    R/W
#define ETH_R255   UPD985XX_SYSETH_REG( 0x188)   // Receive 128 to 255 Byte Frame Counter    R/W
#define ETH_R511   UPD985XX_SYSETH_REG( 0x18C)   // Receive 256 to 511 Byte Frame Counter    R/W
#define ETH_R1K    UPD985XX_SYSETH_REG( 0x190)   // Receive 512 to 1023 Byte Frame Counter    R/W
#define ETH_RMAX   UPD985XX_SYSETH_REG( 0x194)   // Receive Over 1023 Byte Frame Counter    R/W
#define ETH_RVBT   UPD985XX_SYSETH_REG( 0x198)   // Receive Valid Byte Counter    R/W
#define ETH_TBYT   UPD985XX_SYSETH_REG( 0x1C0)   // Transmit Byte Counter    R/W
#define ETH_TPCT   UPD985XX_SYSETH_REG( 0x1C4)   // Transmit Packet Counter    R/W
#define ETH_TFCS   UPD985XX_SYSETH_REG( 0x1C8)   // Transmit CRC Error Packet Counter    R/W
#define ETH_TMCA   UPD985XX_SYSETH_REG( 0x1CC)   // Transmit Multicast Packet Counter    R/W
#define ETH_TBCA   UPD985XX_SYSETH_REG( 0x1D0)   // Transmit Broadcast Packet Counter    R/W
#define ETH_TUCA   UPD985XX_SYSETH_REG( 0x1D4)   // Transmit Unicast Packet Counter    R/W
#define ETH_TXPF   UPD985XX_SYSETH_REG( 0x1D8)   // Transmit PAUSE control Frame Counter    R/W
#define ETH_TDFR   UPD985XX_SYSETH_REG( 0x1DC)   // Transmit Single Deferral Packet Counter    R/W
#define ETH_TXDF   UPD985XX_SYSETH_REG( 0x1E0)   // Transmit Excessive Deferral Packet Counter    R/W
#define ETH_TSCL   UPD985XX_SYSETH_REG( 0x1E4)   // Transmit Single Collision Packet Counter    R/W
#define ETH_TMCL   UPD985XX_SYSETH_REG( 0x1E8)   // Transmit Multiple collision Packet Counter    R/W
#define ETH_TLCL   UPD985XX_SYSETH_REG( 0x1EC)   // Transmit Late Collision Packet Counter    R/W
#define ETH_TXCL   UPD985XX_SYSETH_REG( 0x1F0)   // Transmit Excessive Collision Packet Counter    R/W
#define ETH_TNCL   UPD985XX_SYSETH_REG( 0x1F4)   // Transmit Total Collision Counter    R/W
#define ETH_TCSE   UPD985XX_SYSETH_REG( 0x1F8)   // Transmit Carrier Sense Error Counter    R/W
#define ETH_TIME   UPD985XX_SYSETH_REG( 0x1FC)   // Transmit Internal MAC Error Counter    R/W
//      
//      Table 5-4.  DMA and FIFO Management Registers Map
//      
#define ETH_TXCR   UPD985XX_SYSETH_REG( 0x200)   // Transmit Configuration Register    R/W    0000_0000H
#define ETH_TXFCR  UPD985XX_SYSETH_REG( 0x204)   // Transmit FIFO Control Register    R/W    FFFF_40C0H
#define ETH_TXDTR  UPD985XX_SYSETH_REG( 0x208)   // Transmit Data Register    W    0000_0000H
#define ETH_TXSR   UPD985XX_SYSETH_REG( 0x20C)   // Transmit Status Register    R    0000_0000H
//      N/A        UPD985XX_SYSETH_REG( 0x210)   // Reserved for future use    -    -
#define ETH_TXDPR  UPD985XX_SYSETH_REG( 0x214)   // Transmit Descriptor Pointer    R/W    0000_0000H
#define ETH_RXCR   UPD985XX_SYSETH_REG( 0x218)   // Receive Configuration Register    R/W    0000_0000H
#define ETH_RXFCR  UPD985XX_SYSETH_REG( 0x21C)   // Receive FIFO Control Register    R/W    C040_0040H
#define ETH_RXDTR  UPD985XX_SYSETH_REG( 0x220)   // Receive Data Register    R    0000_0000H
#define ETH_RXSR   UPD985XX_SYSETH_REG( 0x224)   // Receive Status Register    R    0000_0000H
//      N/A        UPD985XX_SYSETH_REG( 0x228)   // Reserved for future use    -    -
#define ETH_RXDPR  UPD985XX_SYSETH_REG( 0x22C)   // Receive Descriptor Pointer    R/W    0000_0000H
#define ETH_RXPDR  UPD985XX_SYSETH_REG( 0x230)   // Receive Pool Descriptor Register    R/W    0000_0000H
//      
//      Table 5-5.  Interrupt and Configuration Registers Map
//      
#define ETH_CCR    UPD985XX_SYSETH_REG( 0x234)   // Configuration Register    R/W    0000_0000H
#define ETH_ISR    UPD985XX_SYSETH_REG( 0x238)   // Interrupt Service Register    R    0000_0000H
#define ETH_MSR    UPD985XX_SYSETH_REG( 0x23C)   // Mask Serves Register    R/W    0000_0000H

// --------------------------------------------------------------------------
// Now the fields within all those registers...
//
//      Table 5-2.  MAC Control Register Map
//
//      ETH_MACC1   0x000  MAC configuration register 1    R/W    0000_0000H
#define ETH_MACC1_MACLB  (1<<14) // MAC loopback:    0
#define ETH_MACC1_TXFC   (1<<11) // Transmit flow control enable:    0
#define ETH_MACC1_RXFC   (1<<10) // Receive flow control enable:    0
#define ETH_MACC1_SRXEN  (1<< 9) // Receive enable:    0
#define ETH_MACC1_PARF   (1<< 8) // Control packet pass:    0
#define ETH_MACC1_PUREP  (1<< 7) // Pure preamble:    0
#define ETH_MACC1_FLCHT  (1<< 6) // Length field check:    0
#define ETH_MACC1_NOBO   (1<< 5) // No Back Off:    0
#define ETH_MACC1_CRCEN  (1<< 3) // CRC append enable:    0
#define ETH_MACC1_PADEN  (1<< 2) // PAD append enable:    0
#define ETH_MACC1_FDX    (1<< 1) // Full duplex enable:    0
#define ETH_MACC1_HUGEN  (1<< 0) // Large packet enable:    0
//      ETH_MACC2   0x004  MAC configuration register 2    R/W    0000_0000H
#define ETH_MACC2_MCRST  (1<<10) // MAC Control Block software reset:    0
#define ETH_MACC2_RFRST  (1<< 9) // Receive Function Block software reset:    0
#define ETH_MACC2_TFRST  (1<< 8) // Transmit Function Block software reset:    0
#define ETH_MACC2_BPNB   (1<< 6) // Back Pressure No Back Off:    0
#define ETH_MACC2_APD    (1<< 5) // Auto VLAN PAD:    0
#define ETH_MACC2_VPD    (1<< 4) // VLAN PAD mode:    0

//      ETH_IPGT    0x008  Back-to-Back IPG register    R/W    0000_0013H
//      ETH_IPGR    0x00C  Non Back-to-Back IPG register    R/W    0000_0E13H
//      ETH_CLRT    0x010  Collision register    R/W    0000_370FH
//      ETH_LMAX    0x014  Max packet length register    R/W    0000_0600H
//          N/A     0x018  Reserved for future use    -    -
//      ETH_RETX    0x020  Retry count register    R/W    0000_0000H
//          N/A     0x024  Reserved for future use    -    -
//      ETH_LSA2    0x054  Station Address register 2    R/W    0000_0000H
//      ETH_LSA1    0x058  Station Address register 1    R/W    0000_0000H
//      ETH_PTVR    0x05C  Pause timer value read register    R    0000_0000H
//      N/A         0x060  Reserved for future use    -    -
//      ETH_VLTP    0x064  VLAN type register    R/W    0000_0000H
#define ETH_VLTP_VLTP     (0x00008100) // magic number from example
//      ETH_MIIC    0x080  MII configuration register    R/W    0000_0000H
#define ETH_MIIC_MIRST    (1<<15) // MII Management Interface Block software reset
#define ETH_MIIC_CLKS     (0x0C)  // 3:2    CLKS    Select frequency range:
#define ETH_MIIC_25       (0x00)  // 00:  HCLK is equal to 25 MHz
#define ETH_MIIC_33       (0x04)  // 01:  HCLK is less than or equal to 33 MHz
#define ETH_MIIC_50       (0x08)  // 10:  HCLK is less than or equal to 50 MHz
#define ETH_MIIC_66       (0x0C)  // 11:  HCLK is less than or equal to 66 MHz
//      ETH_MCMD    0x094  MII command register    W    0000_0000H
#define ETH_MCMD_SCANC    (1<< 1) // SCAN command:    0
#define ETH_MCMD_RSTAT    (1<< 0) // MII management read:    0
//      ETH_MADR    0x098  MII address register    R/W    0000_0000H
#define ETH_MADR_PHY_ADDR_SHIFT (8)
//      ETH_MWTD    0x09C  MII write data register    R/W    0000_0000H
//      ETH_MRDD    0x0A0  MII read data register    R    0000_0000H
//      ETH_MIND    0x0A4  MII indicator register    R    0000_0000H
#define ETH_MIND_NVALID   (1<< 2) // SCAN command start status:    0
#define ETH_MIND_SCANA    (1<< 1) // SCAN command active:    0
#define ETH_MIND_BUSY     (1<< 0) // BUSY:     0
//      ETH_AFR     0x0C8  Address Filtering register    R/W    0000_0000H
#define ETH_AFR_PRO       (1<< 3) // Promiscuous mode:    0
#define ETH_AFR_PRM       (1<< 2) // Accept Multicast:    0
#define ETH_AFR_AMC       (1<< 1) // Accept Multicast ( qualified ):    0
#define ETH_AFR_ABC       (1<< 0) // Accept Broadcast:     0
//      ETH_HT1     0x0CC  Hash table register 1    R/W    0000_0000H
//      ETH_HT2     0x0D0  Hash table register 2    R/W    0000_0000H
//      ETH_CAR1    0x0DC  Carry register 1    R/W    0000_0000H
//      ETH_CAR2    0x0E0  Carry register 2    R/W    0000_0000H
//      ETH_CAM1    0x130  Carry mask register 1    R/W    0000_0000H
//      ETH_CAM2    0x134  Carry mask register 2    R/W    0000_0000H
//
//      Table 5-3.  Statistics Counter Register Map
//      <snip>
//
//      Table 5-4.  DMA and FIFO Management Registers Map
//      
//      ETH_TXCR    0x200  Transmit Configuration Register    R/W    0000_0000H
#define ETH_TXCR_TXE      (1<<31) // Transmit Enable:
#define ETH_TXCR_DTBS_SHIFT (16)   // 18:16     DMA Transmit Burst Size:
#define ETH_TXCR_DTBS    (0x70000) // 18:16     DMA Transmit Burst Size:
#define ETH_TXCR_DTBS_1  (0x00000) // 000: 1 Word (4 bytes)
#define ETH_TXCR_DTBS_2  (0x10000) // 001: 2 Word (8 bytes)
#define ETH_TXCR_DTBS_4  (0x20000) // 010: 4 Word (16 bytes)
#define ETH_TXCR_DTBS_8  (0x30000) // 011: 8 Word (32 bytes)
#define ETH_TXCR_DTBS_16 (0x40000) // 100: 16 Word (64 bytes)
#define ETH_TXCR_DTBS_32 (0x50000) // 101: 32 Word (128 bytes)
#define ETH_TXCR_DTBS_64 (0x60000) // 110: 64 Word (256 bytes)
#define ETH_TXCR_AFCE     (1<< 0) // Auto Flow Control Enable:
//      ETH_TXFCR   0x204  Transmit FIFO Control Register    R/W    FFFF_40C0H
#define ETH_TXFCR_TPTV_SHIFT (16)      // 31:16 Transmit Pause Timer Value:    FFFFH
#define ETH_TXFCR_TPTV    (0xffff0000) // 31:16 Transmit Pause Timer Value:    FFFFH
#define ETH_TXFCR_TX_DRTH_SHIFT (10)   // 15:10 Transmit Drain Threshold Level:    10H
#define ETH_TXFCR_TX_DRTH (0x0000fc00) // 15:10 Transmit Drain Threshold Level:    10H
#define ETH_TXFCR_TX_FLTH_SHIFT (2)    //  7:2 Transmit Fill Threshold Level:    03H
#define ETH_TXFCR_TX_FLTH (0x000000fc) //  7:2 Transmit Fill Threshold Level:    03H
#define ETH_TXFCR_TPTV_DEFAULT    (0x10000000) // default 0x1000 slot time (1slot:512bit)
#define ETH_TXFCR_TX_DRTH_DEFAULT (0x00002000) // 001000b (8long, 32byte)
#define ETH_TXFCR_TX_FLTH_DEFAULT (0x000000c0) // default 110000b (48word, 192byte)
//      ETH_TXDTR   0x208  Transmit Data Register    W    0000_0000H
//      ETH_TXSR    0x20C  Transmit Status Register    R    0000_0000H
#define ETH_TXSR_CSE      (1<<31) // Carrier lost was detected during the transmission    0
#define ETH_TXSR_TBP      (1<<30) // Back pressure occurred when the packet was received    0
#define ETH_TXSR_TPP      (1<<29) // A packet request during the PAUSE operation was transmitted    0
#define ETH_TXSR_TPCF     (1<<28) // A PAUSE control frame was transmitted    0
#define ETH_TXSR_TCFR     (1<<27) // A control frame was transmitted    0
#define ETH_TXSR_TUDR     (1<<26) // The TPUR pin was set high and aborted. Note 2    0
#define ETH_TXSR_TGNT     (1<<25) // A huge packet was transmitted and aborted.  0
#define ETH_TXSR_LCOL     (1<<24) // Collision occurred
#define ETH_TXSR_ECOL     (1<<23) // Excess collisions
#define ETH_TXSR_TEDFR    (1<<22) // Excess deferred
#define ETH_TXSR_TDFR     (1<<21) // Transmission deferral occurred
#define ETH_TXSR_TBRO     (1<<20) // A broadcast packet was transmitted.    0
#define ETH_TXSR_TMUL     (1<<19) // A multicast packet was transmitted.    0
#define ETH_TXSR_TDONE    (1<<18) // Transmission was completed.    0
#define ETH_TXSR_TFLOR    (1<<17) // Value of the length field was huge
#define ETH_TXSR_TFLER    (1<<16) // Value of the length field didn~t match the actual data count
#define ETH_TXSR_TCRCE    (1<<15) // Attached CRC didn~t match the internal generated CRC

#define ETH_TXSR_TCBC_SHIFT  (11) // 14:11 collisions for the previous transmission
#define ETH_TXSR_TCBC    (0x7800) // 14:11 collisions for the previous transmission
#define ETH_TXSR_TBYT_SHIFT   (0) // 10:0  transmitted bytes not including collided bytes
#define ETH_TXSR_TBYT    (0x07FF) // 10:0  transmitted bytes not including collided bytes

//      ETH_RXCR   UPD985XX_SYSETH_REG( 0x218)   // Receive Configuration Register    R/W    0000_0000H
#define ETH_RXCR_RXE      (1<<31)  // Receive Enable:
#define ETH_RXCR_DRBS_SHIFT (16)   // 18:16    DRBS    DMA Transmit Burst Size:    0
#define ETH_RXCR_DRBS    (0x70000) //
#define ETH_RXCR_DRBS_1  (0x00000) // 000: 1 Word (4 bytes)
#define ETH_RXCR_DRBS_2  (0x10000) // 001: 2 Word (8 bytes)
#define ETH_RXCR_DRBS_4  (0x20000) // 010: 4 Word (16 bytes)
#define ETH_RXCR_DRBS_8  (0x30000) // 011: 8 Word (32 bytes)
#define ETH_RXCR_DRBS_16 (0x40000) // 100: 16 Word (64 bytes)
#define ETH_RXCR_DRBS_32 (0x50000) // 101: 32 Word (128 bytes)
#define ETH_RXCR_DRBS_64 (0x60000) // 110: 64 Word (256 bytes)

//      ETH_RXFCR   0x21C  Receive FIFO Control Register    R/W    C040_0040H
#define ETH_RXFCR_UWM     (0xfc000000) // 31:26  Upper Water Mark:    30H
#define ETH_RXFCR_UWM_SHIFT (26)       // 31:26  Upper Water Mark:    30H
#define ETH_RXFCR_LWM     (0x00fc0000) // 23:18  Lower Water Mark:    10H
#define ETH_RXFCR_LWM_SHIFT (18)       // 23:18  Lower Water Mark:    10H
#define ETH_RXFCR_RX_DRTH (0x000000fc) //  7: 2  Receive Drain Threshold Level    10H
#define ETH_RXFCR_RX_DRTH_SHIFT (2)    //  7: 2  Receive Drain Threshold Level    10H
#define ETH_RXFCR_UWM_DEFAULT (0xE0000000) // default 110000b ( 48word, 192byte )
#define ETH_RXFCR_LWM_DEFAULT (0x00400000) // default 010000b (16word, 64byte)
#define ETH_RXFCR_DRTH16W     (0x00000040) // default 010000b (16word, 64byte)

//      ETH_RXDTR   0x220  Receive Data Register    R    0000_0000H
//      ETH_RXSR    0x224  Receive Status Register    R    0000_0000H
#define ETH_RXSR_RLENE (1<<31) // A toosmall or toolarge packet was received.
#define ETH_RXSR_VLAN  (1<<30) // A VLAN was received.
#define ETH_RXSR_USOP  (1<<29) // A control frame containing an unknown OP code was received.
#define ETH_RXSR_PRCF  (1<<28) // A control frame containing the PAUSE OP code was received.
#define ETH_RXSR_RCFR  (1<<27) // A control frame was received.
#define ETH_RXSR_DBNB  (1<<26) // An alignment error occurred.
#define ETH_RXSR_RBRO  (1<<25) // A broadcast packet was received.    0
#define ETH_RXSR_RMUL  (1<<24) // A multicast packet was received.    0
#define ETH_RXSR_RXOK  (1<<23) // A good packet was received.
#define ETH_RXSR_RLOR  (1<<22) // The value of the length field was huge
#define ETH_RXSR_RLER  (1<<21) // The value of the length field didn~t match
#define ETH_RXSR_RCRCE (1<<20) // A CRC error occurred.    0
#define ETH_RXSR_RCV   (1<<19) // RXER was detected.    0
#define ETH_RXSR_CEPS  (1<<18) // A False Carrier was detected.    0
#define ETH_RXSR_REPS  (1<<17) // A packet which had a preamble and SFD only or one data nibble
#define ETH_RXSR_PAIG  (1<<16) // 
#define ETH_RXSR_RBYT (0xffff)  // 15:0  The received byte count    0
#define ETH_RXSR_RBYT_SHIFT (0) // 15:0  The received byte count    0
//      ETH_RXDPR   0x22C  Receive Descriptor Register    R/W    0000_0000H
//      ETH_RXPDR   0x230  Receive Pool Descriptor Register    R/W    0000_0000H
#define ETH_RXPDR_AL (0x70000000) // 30:28 AL[2:0] Alert Level 0H
#define ETH_RXPDR_AL_SHIFT (28)
#define ETH_RXPDR_RNOD (0xffff)  // 15:0 Remaining Number of Descriptor 0H
#define ETH_RXPDR_RNOD_SHIFT (0)
//      
//      Table 5-5.  Interrupt and Configuration Registers Map
//      
//      ETH_CCR     0x234  Configuration Register    R/W    0000_0000H
#define ETH_CCR_SRT (1) // Software Reset (cleared automatically to '0')               
//      ETH_ISR     0x238  Interrupt Service Register    R    0000_0000H 
#define ETH_ISR_XMTDN (1<<15) // Transmit Done 
#define ETH_ISR_TBDR  (1<<14) // Transmit Buffer Descriptor Request at Null 
#define ETH_ISR_TFLE  (1<<13) // Transmit Frame Length Exceed 
#define ETH_ISR_UR    (1<<12) // Underrun 
#define ETH_ISR_TABR  (1<<11) // Transmit Aborted 
#define ETH_ISR_TCFRI (1<<10) // Control Frame Transmit 
#define ETH_ISR_RCVDN (1<<7 ) // Receive Done 
#define ETH_ISR_RBDRS (1<<6 ) // Receive Buffer Descriptor Request at alert level 
#define ETH_ISR_RBDRU (1<<5 ) // Receive Buffer Descriptor Request at zero 
#define ETH_ISR_OF    (1<<4 ) //      Overflow     
#define ETH_ISR_LFAL  (1<<3 ) // Link Failed 
#define ETH_ISR_CARRY (1<<0 ) // Carry Flag:
//      ETH_MSR     0x23C  Mask Serves Register    R/W    0000_0000H
// As above

// --------------------------------------------------------------------------
// And the "buffer descriptor" control structures in RAM...


#define ETH_BUF_LAST      (1<<31) // Last Descriptor
#define ETH_BUF_D_L       (1<<30) // Data Buffer / Link Pointer
#define ETH_BUF_D_L_DATA  (1<<30) // Data Buffer / Link Pointer
#define ETH_BUF_D_L_LINK  (0<<30) // Data Buffer / Link Pointer
#define ETH_BUF_OWN       (1<<29) // Owner  1:Ethernet Controller  0: VR4120A
#define ETH_BUF_OWN_ETH   (1<<29)
#define ETH_BUF_OWN_CPU   (0<<29)

#define ETH_BUF_DBRWE     (1<<28) // Buffer Access Error
#define ETH_BUF_OK        (1<<16) // Tx or Rx OK
#define ETH_BUF_SIZE     (0xffff) // Byte Count
                                 
#define ETH_BUF_TX_TUDR   (1<<27) // Transmit Underrun Error
#define ETH_BUF_TX_CSE    (1<<26) // Carrier Sense Lost Error
#define ETH_BUF_TX_LCOL   (1<<25) // Late Collision
#define ETH_BUF_TX_ECOL   (1<<24) // Excessive Collision
#define ETH_BUF_TX_EDFR   (1<<23) // Excessive Deferral
#define ETH_BUF_TX_TGNT   (1<<18) // Transmit Giant Frame
#define ETH_BUF_TX_HBF    (1<<17) // Heart Beat Fail for ENDEC mode
                                 
#define ETH_BUF_RX_OVRN   (1<<24) // Overrun Error
#define ETH_BUF_RX_RUNT   (1<<23) // Runt packet
#define ETH_BUF_RX_FRGE   (1<<22) // Fragment Error
#define ETH_BUF_RX_RCV    (1<<21) // Detects RXER
#define ETH_BUF_RX_FC     (1<<20) // False Carrier
#define ETH_BUF_RX_CRCE   (1<<19) // CRC Error
#define ETH_BUF_RX_FAE    (1<<18) // Frame Alignment Error
#define ETH_BUF_RX_RFLE   (1<<17) // Receive Frame Length Error

#define ETH_BUF_RX_FTYP (0x0e000000) // 27:25 Frame Type[2:0]
#define ETH_BUF_RX_FTYP_SHIFT (25)   // 27:25 Frame Type[2:0]
// I don't think we need to know these...
//                000 Broadcast Frame
//                001 Multicast Frame
//                010 Unicast Frame
//                011 VLAN Frame
//                100 PAUSE control frame
//                101 Control Frame (except pause)
//                11x Reserved for future use


// --------------------------------------------------------------------------
// MII stuff for talking to the separate PHY
// Initially this was a SEEQ NQ80225 but now it is a LU3X31T-T64.

//#define SEEQ_DEVICE_PHYS_ADDRESS (1) // this from the board documentation
#define LU3X31T_DEVICE_PHYS_ADDRESS (2)

#define ETH_MADR_PHY_DEVICE_PHYS_ADDRESS \
   (LU3X31T_DEVICE_PHYS_ADDRESS << ETH_MADR_PHY_ADDR_SHIFT)

// I don't know how much they have in common, but I think MII is pretty
// standard, and the "mandated" registers ought to be common.

#define PHY_CONTROL_REG     (0)
#define PHY_STATUS_REG      (1)
#define PHY_ID_ONE          (2)
#define PHY_ID_TWO          (3)
#define PHY_AUTONEG_ADVERT  (4)
#define PHY_AUTONEG_REMOTE  (5)
#define PHY_STATUS_DETECT_REG (18)

#define PHY_CONTROL_RESET           (1<<15)
#define PHY_CONTROL_LOOPBACK        (1<<14)
#define PHY_CONTROL_SPEED100        (1<<13)
#define PHY_CONTROL_AUTONEG_EN      (1<<12)
#define PHY_CONTROL_POWERDOWN       (1<<11)
#define PHY_CONTROL_MII_DIS         (1<<10)
#define PHY_CONTROL_AUTONEG_RST     (1<< 9)
#define PHY_CONTROL_DPLX_FULL       (1<< 8)
#define PHY_CONTROL_COLLTEST        (1<< 7)
  
#define PHY_STATUS_CAP_T4           (1<<15)
#define PHY_STATUS_CAP_100TXF       (1<<14)
#define PHY_STATUS_CAP_100TXH       (1<<13)
#define PHY_STATUS_CAP_10TF         (1<<12)
#define PHY_STATUS_CAP_10TH         (1<<11)
#define PHY_STATUS_CAP_SUPR         (1<< 6)
#define PHY_STATUS_AUTONEG_ACK      (1<< 5)
#define PHY_STATUS_REMOTEFAULT      (1<< 4)
#define PHY_STATUS_CAP_AUTONEG      (1<< 3)
#define PHY_STATUS_LINK_OK          (1<< 2)
#define PHY_STATUS_JABBER           (1<< 1)
#define PHY_STATUS_EXTREGS          (1<< 0)

// These are the same for both AUTONEG registers
#define PHY_AUTONEG_NEXT            (1<<15)
#define PHY_AUTONEG_ACK             (1<<14)
#define PHY_AUTONEG_REMOTEFAULT     (1<<13)
#define PHY_AUTONEG_100BASET4       (1<< 9)
#define PHY_AUTONEG_100BASETX_FDX   (1<< 8)
#define PHY_AUTONEG_100BASETX_HDX   (1<< 7)
#define PHY_AUTONEG_10BASET_FDX     (1<< 6)
#define PHY_AUTONEG_10BASET_HDX     (1<< 5)
#define PHY_AUTONEG_CSMA_802_3      (1<< 0)

#if 0
// Others are undocumented
#define PHY_STATUS_DETECT_SPEED100   (1<< 7)
#define PHY_STATUS_DETECT_DPLX_FULL  (1<< 6)
#endif

// Phew!
// --------------------------------------------------------------------------
#endif // CYGONCE_HAL_UPD985XX_ETH_H
// End of upd985xx_eth.h
