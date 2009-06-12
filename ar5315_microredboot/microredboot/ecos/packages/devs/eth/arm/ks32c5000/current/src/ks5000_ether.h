//==========================================================================
//
//      ks5000_ether.h
//
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
// Author(s):    gthomas
// Contributors: gthomas, jskov
//               Grant Edwards <grante@visi.com>
// Date:         2001-07-31
// Purpose:      
// Description:  
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#ifndef KS32C5000_ETHER_H
#define KS32C5000_ETHER_H

// Tx Frame Descriptor's control bits -- Refer the KS32C5000 Manual Page 7-15
#define FRM_OWNERSHIP_BDMA     		0x80000000 // 0:CPU, 1:BDMA
#define FRM_OWNERSHIP_CPU          	0x7fffffff // 0:CPU, 1:BDMA

#define	TXFDCON_PADDING_MODE		0x00
#define	TXFDCON_NO_PADDING_MODE		0x01
#define	TXFDCON_NO_CRC_MODE		0x02
#define	TXFDCON_CRC_MODE		0x00
#define	TXFDCON_MAC_TX_INT_EN		0x04
#define	TXFDCON_LITTLE_ENDIAN		0x08
#define	TXFDCON_BIG_ENDIAN		0x00
#define	TXFDCON_SRC_ADDR_DEC 		0x00
#define	TXFDCON_SRC_ADDR_INC 		0x10
#define	TXFDCON_WIDGET_ALIGN00		0x00 // No Invalid bytes
#define	TXFDCON_WIDGET_ALIGN01		0x01 // 1 Invalid byte
#define	TXFDCON_WIDGET_ALIGN10		0x10 // 2 Invalid bytes
#define	TXFDCON_WIDGET_ALIGN11		0x11 // 3 Invalid bytes

// Tx Frame descriptor's Status
#define	TXFDSTAT_EX_COLL 0x0010	// Excessive Collision
#define	TXFDSTAT_DEFFER	 0x0020	// Transmit deffered
#define TXFDSTAT_PAUSED  0x0040	// Paused : holding data transmission DMA to MAC
#define	TXFDSTAT_INT_TX  0x0080	// Interrupt on Transmit
#define	TXFDSTAT_UNDER   0x0100	// Underrun */
#define	TXFDSTAT_DEFER	 0x0200	// Mac defers for Max_DEFERRAL:=0.32768ms
				// for 100Mbits/s, := 3.2768ms for 10Mbits/s
#define	TXFDSTAT_NCARR	 0x0400	// No Carrier sense is detected during the
				// entire transmission of a packet from SFD
				// to CRC
#define	TXFDSTAT_SQ_ERR	 0x0800	// fake collision signal didn't come from
				// PHY for 1.6us.
#define TXFDSTAT_LATE_COLL 0x1000	// Late collision
#define	TXFDSTAT_PAR	 0x2000	// Transmit Parity Error
#define	TXFDSTAT_COMP	 0x4000	// MAC transmit or discards one packet
#define TXFDSTAT_HALTED	 0x8000	// Transmission was halted by clearing MACTXCON_TX_EN..

// Rx Frame descriptor's Status
#define	RXFDSTAT_OV_MAX	 0x0008	// Over Maximum Size
#define	RXFDSTAT_CTL_RECD 0x0020	// set if packet received is a
				// MAC control frame.
#define	RXFDSTAT_INT_RX	 0x0040	// Interrupt on Receive
#define RXFDSTAT_10STAT  0x0080	// set if packet was received via the
				// 10bits interface reset if packet
				// was received via MII
#define	RXFDSTAT_ALIGN_ERR 0x0100	// Alignment Error
#define	RXFDSTAT_CRC_ERR 0x0200	// CRC error
#define	RXFDSTAT_OVERFLOW 0x0400	// MAC receive FIFO was full when it
				// needed to store a received byte

#define	RXFDSTAT_LONG_ERR 0x0800	// received a frame longer than 1518bytes
#define	RXFDSTAT_PAR	  0x2000	// MAC receive FIFO has detected a parity error
#define RXFDSTAT_GOOD	  0x4000	// successfully received a packet with no errors
#define RXFDSTAT_HALTED   0x8000	// Transmission was halted by clearing MACTXCON_TX_EN...

//   BDMARXCON : 0x9004
//   Buffered DMA Receiver Control Register
#define	BDMARXCON_BRST	0x00001F        // BDMA Rx Burst Size * BDMARXCON_BRST
#define	BDMARXCON_STP_SKP 0x00020	// BDMA Rx Stop/Skip  Frame or Interrupt(=1)
#define	BDMARXCON_MA_INC 0x00040	// BDMA Rx Memory Address Inc/Dec
#define	BDMARXCON_DIE	 0x00080	// BDMA Rx Every Received Frame Interrupt Enable
#define	BDMARXCON_NLIE	 0x00100	// BDMA Rx NULL List Interrupt Enable
#define	BDMARXCON_NOIE	 0x00200	// BDMA Rx Not Owner Interrupt Enable
#define	BDMARXCON_MSOIE	 0x00400	// BDMA Rx Maximum Size over Interrupr Enable
#define	BDMARXCON_LITTLE 0x00800	// BDMA Rx Big/Little Endian
#define	BDMARXCON_BIG 	 0x00000	// BDMA Rx Big/Little Endian
#define BDMARXCON_WA00	 0x00000	// BDMA Rx Word Alignment- no invalid byte
#define BDMARXCON_WA01	 0x01000	// BDMA Rx Word Alignment- one invalid byte
#define BDMARXCON_WA10	 0x02000	// BDMA Rx Word Alignment- two invalid byte
#define BDMARXCON_WA11	 0x03000	// BDMA Rx Word Alignment- three invalid byte
#define	BDMARXCON_EN	 0x04000	// BDMA Rx Enable
#define	BDMARXCON_RESET	 0x08000	// BDMA Rx Reset
#define	BDMARXCON_RX_EMPT 0x10000	// BDMA Rx Buffer empty interrupt
#define	BDMARXCON_EARLY	 0x20000	// BDMA Rx Early notify Interrupt

// BDMATXCON : 0x9000
// Buffered DMA Trasmit Control Register
#define	BDMATXCON_BRST	0x000F	// BDMA Tx Burst Size = 16
#define	BDMATXCON_STP_SKP 0x0020	// BDMA Tx Stop/Skip Frame or Interrupt in case
				// of not Owner the current frame
#define	BDMATXCON_CPIE	0x0080	// BDMA Tx Complete to send control
				// packet Enable
#define	BDMATXCON_NOIE	0x0200	// BDMA Tx Buffer Not Owner
#define	BDMATXCON_TX_EMPTY 0x0400	// BDMA Tx Buffer Empty Interrupt
#define BDMATXCON_TX_NOIE  0x0200       // BDMA Tx not owner (queue empty)
#define BDMATXCON_TX_NULL  0x0100       // BDMA dscr pointer null

// BDMA Tx buffer can be moved to the MAC Tx IO
// when the new frame comes in.
#define	BDMATXCON_MSL000 0x00000	// No wait to fill the BDMA
#define	BDMATXCON_MSL001 0x00800	// wait to fill 1/8 of the BDMA
#define	BDMATXCON_MSL010 0x01000	// wait to fill 2/8 of the BDMA
#define	BDMATXCON_MSL011 0x01800	// wait to fill 3/8 of the BDMA
#define	BDMATXCON_MSL100 0x02000	// wait to fill 4/8 of the BDMA
#define	BDMATXCON_MSL101 0x02800	// wait to fill 5/8 of the BDMA
#define	BDMATXCON_MSL110 0x03000	// wait to fill 6/8 of the BDMA
#define	BDMATXCON_MSL111 0x03800	// wait to fill 7/8 of the BDMA
#define	BDMATXCON_EN	 0x04000	// BDMA Tx Enable
#define	BDMATXCON_RESET	 0x08000	// BDMA Rx Reset

// BDMASTAT : 0x9014
// Buffered DMA Status Register
#define	BDMASTAT_RX_RDF	0x00001	// BDMA Rx Done Every Received Frame
#define	BDMASTAT_RX_NL	0x00002	// BDMA Rx NULL List
#define	BDMASTAT_RX_NO	0x00004	// BDMA Rx Not Owner
#define	BDMASTAT_RX_MSO	0x00008	// BDMA Rx Maximum Size Over
#define	BDMASTAT_RX_EMPTY 0x00010	// BDMA Rx Buffer Empty
#define	BDMASTAT_RX_SEARLY 0x00020	// Early Notify
#define	BDMASTAT_RX_FRF	0x00080 // One more frame data in BDMA receive buffer
#define	BDMASTAT_TX_CCP	0x10000	// BDMA Tx Complete to send Control Packet
#define	BDMASTAT_TX_NL	0x20000	// BDMA Tx Null List
#define	BDMASTAT_TX_NO	0x40000	// BDMA Tx Not Owner
#define	BDMASTAT_TX_EMPTY 0x100000// BDMA Tx Buffer Empty

// MACON : 0xa000
// MAC Control Register
#define	MACON_HALT_REG	0x0001	// stop transmission and reception
				// after completion of ant current packets
#define	MACON_HALT_IMM	0x0002	// Stop transmission and reception immediately
#define	MACON_SW_RESET 	0x0004 	// reset all Ethernet controller state machines
				// and FIFOs
#define	MACON_FULL_DUP	0x0008	// allow transmission to begin while reception
				// is occurring
#define	MACON_MAC_LOOP	0x0010	// MAC loopback
#define	MACON_CONN_M00	0x0000	// Automatic-default
#define	MACON_CONN_M01	0x0020	// Force 10Mbits endec
#define	MACON_CONN_M10	0x0040	// Force MII (rate determined by MII clock
#define	MACON_LOOP10	0x0080	// Loop 10Mbps
#define	MACON_MISS_ROLL	0x0400	// Missed error counter rolled over
#define	MACON_EN_MISS_ROLL 0x2000	// Interrupt when missed error counter rolls
				// over
#define	MACON_LINK10	0x8000	// Link status 10Mbps

// CAMCON : 0xa004
// CAM control register
#define	CAMCON_STATION_ACC 0x0001	// Accept any packet with a unicast station
				// address
#define	CAMCON_GROUP_ACC 0x0002	// Accept any packet with multicast-group
				// station address
#define	CAMCON_BROAD_ACC 0x0004	// Accept any packet with a broadcast station
				// address
#define	CAMCON_NEG_CAM	 0x0008	// 0: Accept packets CAM recognizes,
				//    reject others
				// 1: reject packets CAM recognizes,
				//    accept others
#define	CAMCON_COMP_EN	 0x0010	// Compare Enable mode

// MACTXCON : 0xa008
// Transmit Control Register
#define	MACTXCON_TX_EN	 0x0001	// transmit Enable
#define MACTXCON_TX_HALT 0x0002	// Transmit Halt Request
#define	MACTXCON_NO_PAD	 0x0004	// suppress Padding
#define	MACTXCON_NO_CRC	 0x0008	// Suppress CRC
#define	MACTXCON_FBACK	 0x0010	// Fast Back-off
#define	MACTXCON_NO_DEF	 0x0020	// Disable the defer counter
#define	MACTXCON_SD_PAUSE 0x0040	// Send Pause
#define	MACTXCON_MII10_EN 0x0080	// MII 10Mbps mode enable
#define	MACTXCON_EN_UNDER 0x0100	// Enable Underrun
#define	MACTXCON_EN_DEFER 0x0200	// Enable Deferral
#define	MACTXCON_EN_NCARR 0x0400	// Enable No Carrier
#define	MACTXCON_EN_EXCOLL 0x0800	// interrupt if 16 collision occur
				// in the same packet
#define MACTXCON_EN_LATE_COLL 0x1000	// interrupt if collision occurs after
				// 512 bit times(64 bytes times)
#define	MACTXCON_ENTX_PAR 0x2000	// interrupt if the MAC transmit FIFO
				// has a parity error
#define	MACTXCON_EN_COMP 0x4000	// interrupt when the MAC transmits or
				// discards one packet
// MACTXSTAT : 0xa00c
// Transmit Status Register
#define	MACTXSTAT_EX_COLL 0x0010	// Excessive collision
#define	MACTXSTAT_DEFFERED 0x0020	// set if 16 collisions occur for same packet
#define	MACTXSTAT_PAUSED  0x0040	// packet waited because of pause during
			 		// transmission
#define	MACTXSTAT_INT_TX 0x0080	// set if transmission of packet causes an
				// interrupt condiftion
#define	MACTXSTAT_UNDER	 0x0100	// MAC transmit FIFO becomes empty during
				// transmission
#define MACTXSTAT_DEFER	 0x0200	// MAC defers for MAC deferral
#define	MACTXSTAT_NCARR	 0x0400	// No carrier sense detected during the
				// transmission of a packet
#define	MACTXSTAT_SIG_QUAL 0x0800	// Signal Quality Error
#define	MACTXSTAT_LATE_COLL 0x1000	// a collision occures after 512 bit times
#define	MACTXSTAT_PAR	   0x2000	// MAC transmit FIFO has detected a parity error
#define	MACTXSTAT_COMP	   0x4000	// MAC transmit or discards one packet
#define	MACTXSTAT_HALTED   0x8000	// Transmission was halted by clearing
				// MACTXCON_TX_EN or Halt immedite
// MACRXCON : 0xa010
// Receive Control Register
#define MACRXCON_RX_EN     0x0001
#define	MACRXCON_HALT	   0x0002
#define	MACRXCON_LONG_EN   0x0004
#define	MACRXCON_SHORT_EN  0x0008
#define	MACRXCON_STRIP_CRC 0x0010
#define	MACRXCON_PASS_CTL  0x0020
#define	MACRXCON_IGNORE_CRC 0x0040
#define	MACRXCON_EN_ALIGN 0x0100
#define	MACRXCON_EN_CRC_ERR 0x0200
#define	MACRXCON_EN_OVER   0x0400
#define	MACRXCON_EN_LONG_ERR 0x0800
#define	MACRXCON_EN_RX_PAR 0x2000
#define	MACRXCON_EN_GOOD   0x4000

// MACRXSTAT : 0xa014
// Receive Status Register
#define MACRXSTAT_CTL_RECD 0x0020
#define	MACRXSTAT_INT_RX   0x0040
#define	MACRXSTAT_10STAT   0x0080
#define	MACRXSTAT_ALLIGN_ERR 0x0100
#define	MACRXSTAT_CRC_ERR  0x0200
#define	MACRXSTAT_OVERFLOW 0x0400
#define	MACRXSTAT_LONG_ERR 0x0800
#define	MACRXSTAT_PAR	   0x2000
#define	MACRXSTAT_GOOD	   0x4000
#define	MACRXSTAT_HALTED   0x8000

#endif
