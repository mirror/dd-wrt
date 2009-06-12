//=============================================================================
//
//      ether_test.h - Cyclone Diagnostics
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   Scott Coulter, Jeff Frazier, Eric Breeden
// Contributors:
// Date:        2001-01-25
// Purpose:     
// Description: 
//
//####DESCRIPTIONEND####
//
//===========================================================================*/


#define ETHERMTU	1500
#define OK		0
#define ERROR		-1

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

/* Starting location for ether_test private malloc pool */
#define ETHER_MEM_POOL	0xa0400000	/* above top of diags. BE CAREFUL */

/* Length of interrupt time-out loops. */
#define MAX_DELAY		6000000

/* PCI Runtime Register offsets */
#define SCB_OFFSET		0
#define SCB_STAT_REG(n)		((UINT16 *)(n + 0x00))
#define SCB_CMD_REG(n) 		((UINT16 *)(n + 0x02))
#define SCB_GENPTR_REG(n)	((UINT32 *)(n + 0x04))
#define PORT_REG(n) 		((UINT32 *)(n + 0x08))
#define FLASH_CTL_REG(n)	((UINT16 *)(n + 0x0c))
#define EEPROM_CTL_REG(n)	((UINT16 *)(n + 0x0e))
#define MDI_CTL_REG(n) 		((UINT32 *)(n + 0x10))
#define RXBC_REG(n) 		((UINT32 *)(n + 0x14))

/* PORT* commands (lower 4 bits) */
#define PORT_RESET	((UINT32) 0x0)
#define PORT_SELF_TEST	((UINT32) 0x1)
#define PORT_DUMP	((UINT32) 0x3)

/* Individual Address offset into '557's serial eeprom */
#define IA_OFFSET	0

/* Command codes for the command fields of command descriptor blocks */

#define NOP		0
#define IA_SETUP	1
#define CONFIGURE	2
#define MC_SETUP	3
#define TRANSMIT	4
#define TDR		5
#define DUMP		6
#define DIAGNOSE	7

/* Commands for CUC in command word of SCB */
#define CU_NOP			0
#define CU_START		1
#define CU_RESUME		2
#define LOAD_DUMPCTR_ADDR	4	/* Load Dump Counters Address */
#define DUMP_STAT_COUNTERS	5	/* Dump Statistical Counters  */
#define LOAD_CU_BASE		6	/* Load CU Base Register      */
#define DUMP_RESET_COUNTERS	7	/* Dump and Reset Statistical
					   Counters		      */
/* Commands for RUC in command word of SCB */
#define RU_NOP		0
#define RU_START	1
#define RU_RESUME	2
#define RU_ABORT	4
#define LOAD_HDS	5		/* Load Header Data Size      */
#define LOAD_RU_BASE	6		/* Load RU Base Register      */
#define RBD_RESUME	7		/* Resume frame reception     */

/* Misc. defines */
#define END_OF_LIST	1
#define BUSY		1

/* RU Status field */
#define RU_IDLE		0x0
#define RU_SUSPENDED	0x1
#define RU_NORESOURCE	0x2
#define RU_READY	0x4
#define RU_SUSP_NORBD	0x5
#define RU_NORSRC_NORBD	0x6
#define RU_READY_NORBD	0xc

/* Mask for interrupt status bits in SCB - six possible sources */
#define I557_INT	0xfc00

/* MDI definitions */
#define MDI_WRITE_OP	0x01
#define MDI_READ_OP	0x02
#define MDI_NOT_READY	0
#define MDI_POLLED	0
#define MDI_DEFAULT_PHY_ADDR 1	/* when only one PHY */

/* PHY device register addresses */

/* generic register addresses */
#define MDI_PHY_CTRL		0
#define MDI_PHY_STAT		1
#define MDI_PHY_ID_1		2
#define MDI_PHY_ID_2		3
#define MDI_PHY_AUTO_AD		4
#define MDI_PHY_AUTO_LNK	5
#define MDI_PHY_AUTO_EXP	6

#define I82555_PHY_ID		0x02a80150
#define ICS1890_PHY_ID		0x0015f420
#define DP83840_PHY_ID		0x20005c00
#define I82553_PHY_ID		0x02a80350
#define I82553_REVAB_PHY_ID	0x03e00000

/* I82555/558 Status and Control register */
#define I82555_STATCTRL_REG	0x10
#define I82555_100_MBPS		(1 << 1)
#define I82555_10_MBPS		(0 << 1)

#define REVISION_MASK		0xf

/* DP83840 specific register information */
#define DP83840_PCR_REG		0x17
#define PCR_TXREADY_SEL		(1 << 10)
#define PCR_FCONNECT		(1 << 5)

/* ICS1890 QuickPoll Detailed Status register */
#define ICS1890_QUICKPOLL_REG	0x11
#define QUICK_100_MBPS		(1 << 15)
#define QUICK_10_MBPS		(0 << 15)
#define QUICK_LINK_VALID	(1 << 0)
#define QUICK_LINK_INVALID	(0 << 0)

#define DP83840_PHY_ADDR_REG	0x19
#define PHY_ADDR_CON_STATUS		(1 << 5)
#define PHY_ADDR_SPEED_10_MBPS	(1 << 6)
#define PHY_ADDR_SPEED_100_MBPS	(0 << 6)

#define DP83840_LOOPBACK_REG	0x18
#define TWISTER_LOOPBACK	(0x1 << 8)
#define REMOTE_LOOPBACK		(0x2 << 8)
#define CLEAR_LOOP_BITS		~(TWISTER_LOOPBACK | REMOTE_LOOPBACK)

/* 82553 specific register information */
#define I82553_PHY_EXT_REG0	0x10
#define EXT_REG0_100_MBPS	(1 << 1)
#define GET_REV_CNTR(n)		((n & 0x00e0) >> 5)
#define I82553_PHY_EXT_REG1	0x14

/* MDI Control Register bits */
#define MDI_CTRL_COLL_TEST	(1 << 7)
#define MDI_CTRL_FULL_DUPLEX	(1 << 8)
#define MDI_CTRL_RESTART_AUTO	(1 << 9)
#define MDI_CTRL_ISOLATE	(1 << 10)
#define MDI_CTRL_POWER_DOWN	(1 << 11)
#define MDI_CTRL_AUTO_ENAB	(1 << 12)
#define MDI_CTRL_AUTO_DISAB	(0 << 12)
#define MDI_CTRL_100_MBPS	(1 << 13)
#define MDI_CTRL_10_MBPS	(0 << 13)
#define MDI_CTRL_LOOPBACK	(1 << 14)
#define MDI_CTRL_RESET		(1 << 15)

/* MDI Status Register bits */
#define MDI_STAT_EXTENDED	(1 << 0)
#define MDI_STAT_JABBER		(1 << 1)
#define MDI_STAT_LINK		(1 << 2)
#define MDI_STAT_AUTO_CAPABLE	(1 << 3)
#define MDI_STAT_REMOTE_FLT	(1 << 4)
#define MDI_STAT_AUTO_COMPLETE	(1 << 5)
#define MDI_STAT_10BASET_HALF	(1 << 11)
#define MDI_STAT_10BASET_FULL	(1 << 12)
#define MDI_STAT_TX_HALF	(1 << 13)
#define MDI_STAT_TX_FULL	(1 << 14)
#define MDI_STAT_T4_CAPABLE	(1 << 15)

/*
 * Structure allignments.  All addresses passed to the 557 must be
 * even (bit 0 = 0), EXCEPT for addresses passed by the PORT*
 * function (self-test address & dump address, which must be 16-byte aligned.
 */

#define SELF_TEST_ALIGN	16
#define DUMP_ALIGN	16
#define DEF_ALIGN	4

/*
 * Bit definitions for the configure command.  NOTE:  Byte offsets are
 * offsets from the start of the structure (8 and up) to correspond
 * with the offsets in the PRO/100 PCI Adapter manual.
 */

/* Byte 0 */
#define BYTE_COUNT	0x16		/* use all 22 configure bytes */
#define CONFIG_BYTE_00	(BYTE_COUNT)

/* Byte 1 */
#define RX_FIFO_LIMIT	0x08
#define CONFIG_BYTE_01	(RX_FIFO_LIMIT)

/* Byte 2 */
#define ADAPT_IFS	0x00
#define CONFIG_BYTE_02	(ADAPT_IFS)

/* Byte 3 - must be 0x00 */
#define CONFIG_BYTE_03	(0x00)

/* Byte 4 */
#define RX_DMA_BCOUNT	0x00
#define CONFIG_BYTE_04	(RX_DMA_BCOUNT)

/* Byte 5 */
#define TX_DMA_BCOUNT	0x00
#define DMA_BCOUNT_ENAB	0x80
#define CONFIG_BYTE_05	(DMA_BCOUNT_ENAB | TX_DMA_BCOUNT)

/* Byte 6 */
#define NO_LATE_SCB	0x00
#define NO_TNO_INT	0x00	/* no interrupt on xmit failure */
#define INT_CU_IDLE	0x08	/* interrupt when CU goes idle */
#define NO_SV_BAD_FRAME	0x00	/* don't save bad frames */
#define DISCARD_RX_OVER	0x00	/* discard overrun frames */
#define BYTE6_REQUD	0x32	/* required "1" bits */

#define CONFIG_BYTE_06	(NO_LATE_SCB | NO_TNO_INT | INT_CU_IDLE |\
			 NO_SV_BAD_FRAME | DISCARD_RX_OVER | BYTE6_REQUD)

/* Byte 7 */
#define DISCARD_SHORT_RX	0x00	/* discard short rx frames */
#define ONE_URUN_RETRY		0x02	/* one underrun retry */
#define CONFIG_BYTE_07	(DISCARD_SHORT_RX | ONE_URUN_RETRY)

/* Byte 8 */
#define USE_503_MODE	0x00
#define USE_MII_MODE	0x01
#define CONFIG_BYTE_08	(USE_MII_MODE)

/* Byte 9 */
#define CONFIG_BYTE_09	(0x00)

/* Byte 10 */
#define INSERT_SRC_ADDR	0	/* Source address comes from IA of '557 */
#define PREAMBLE_LEN	0x20	/* 7 bytes */
#define NO_LOOP_BACK	0x00
#define INT_LOOP_BACK	0x40
#define EXT_LOOP_BACK	0xc0
#define BYTE10_REQUD	0x06	/* required "1" bits */
#define CONFIG_BYTE_10	(NO_LOOP_BACK | PREAMBLE_LEN | INSERT_SRC_ADDR | BYTE10_REQUD)

/* Byte 11 */
#define LIN_PRIORITY	0	/* normal CSMA/CD */
#define CONFIG_BYTE_11	(LIN_PRIORITY)

/* Byte 12 */
#define LIN_PRIORITY_MODE	0
#define IF_SPACING		96	/* inter-frame spacing */
#define CONFIG_BYTE_12		(IF_SPACING | LIN_PRIORITY_MODE)

/* Byte 13 */
#define CONFIG_BYTE_13	(0x00)

/* Byte 14 */
#define CONFIG_BYTE_14	(0xf2)

/* Byte 15 */
#define PROM_MODE	0	/* not promiscuous */
#define BROADCAST	0	/* disabled */
#define CRS		0x80	/* CDT = carrier */
#define BYTE15_REQUD	0x48	/* required "1" bits */
#define CONFIG_BYTE_15	(PROM_MODE | BROADCAST | CRS | BYTE15_REQUD)

/* Byte 16 */
#define CONFIG_BYTE_16	(0x00)

/* Byte 17 */
#define CONFIG_BYTE_17	(0x40)

/* Byte 18 */
#define STRIPPING_DISABLE	0x00
#define STRIPPING_ENABLE	0x01
#define PADDING_ENABLE  	0x02
#define XFER_CRC		0x04	/* store CRC */
#define NO_XFER_CRC		0x00
#define BYTE18_REQUD		0xf0	/* required "1" bits */
#define CONFIG_BYTE_18	(NO_XFER_CRC | PADDING_ENABLE | STRIPPING_ENABLE | BYTE18_REQUD)

/* Byte 19 */
#define NO_FORCE_FDX		0x00
#define FORCE_FDX		0x40
#define FDX_PIN_ENAB		0x80
#define CONFIG_BYTE_19_10T	FORCE_FDX
#define CONFIG_BYTE_19_100T	NO_FORCE_FDX

/* Byte 20 */
#define NO_MULTI_IA	0x00
#define CONFIG_BYTE_20	(NO_MULTI_IA)

/* Byte 21 */
#define NO_MULTI_ALL	0x00
#define CONFIG_BYTE_21	(NO_MULTI_ALL)

#define SCB_S_CUMASK	0x00c0		/* state mask */
#define SCB_S_CUIDLE	(0x00 << 6)	/* CU is idle */
#define SCB_S_CUSUSP	(0x01 << 6)	/* CU is suspended */
#define SCB_S_CUACTIVE	(0x02 << 6)	/* CU is active */
#define SCB_S_CURSV1	(0x03 << 6)	/* reserved */

/*
 * 82557 structures.  NOTE: the 557 is used in 32-bit linear addressing
 * mode.  See alignment restrictions above.
 */

/* Result of PORT* self-test command - MUST be 16 byte aligned! */
struct selfTest {
	UINT32	romSig;			/* signature of rom */
	union {				/* Flag bits - as UINT32 or field */
		struct {
			UINT32	rsrv1 : 2;
			UINT32	romTest : 1;
			UINT32	regTest : 1;
			UINT32	rsrv2 : 1;
			UINT32	diagnTest : 1;
			UINT32	rsrv3 : 6;
			UINT32	selfTest : 1;
			UINT32	rsrv4 : 19;
		} bits;
		UINT32	word2;
	} u;
};

/* MDI Control Register */
typedef union
{
    struct
    {
    	UINT32 data : 16;	/* data to write or data read */
    	UINT32 regAdd : 5;	/* PHY register address */
    	UINT32 phyAdd : 5;	/* PHY address */
    	UINT32 op : 2;	/* opcode, 1 for MDI write, 2 for MDI read */
    	UINT32 ready : 1;	/* 1 = operation complete */
    	UINT32 intEnab : 1;	/* 1 = interrupt at end of cycle */
    	UINT32 rsrv : 2;	/* reserved */   
    } bits;
    UINT32 word;
} MDI_CONTROL_U;

/* Command/Status Word of SCB */
typedef union
{
    struct
    {
	UINT32	rsrv1 : 2;		/* Reserved */
	UINT32	rus : 4;		/* Receive unit status */
	UINT32	cus : 2;		/* Command unit status */
	UINT32	rsrv2 : 2;		/* Reserved */
	UINT32	statack_swi : 1;	/* Software generated int. */
	UINT32	statack_mdi : 1;	/* MDI read/write complete */
	UINT32	statack_rnr : 1;	/* RU not ready */
	UINT32	statack_cna : 1;	/* CU not active */
	UINT32	statack_fr : 1;		/* Frame reception done */
	UINT32	statack_cx_tno : 1;	/* Cmd exec completed */
	UINT32	ruc : 3;		/* Receive unit command */
	UINT32	rsrv3 : 1;		/* Reserved */
	UINT32	cuc : 3;		/* Command unit command */
	UINT32	rsrv4 : 1;		/* Reserved */
	UINT32	m : 1;			/* Interrupt mask bit */
	UINT32	si : 1;			/* Software generated int. */
	UINT32	rsrv5 : 6;		/* Reserved */
    } bits;
    struct
    {
	UINT16	status;
	UINT16	command;
    } words;
} CMD_STAT_U;

/* System command block - on chip for the 82557 */
struct SCBtype 
{
	CMD_STAT_U	cmdStat; 
	UINT32		scb_general_ptr;	/* SCB General Pointer */
};

/* Command blocks - declared as a union; some commands have different fields */
union cmdBlock {
	/* No operation */
	struct {
		UINT32	rsrv1 : 13;	/* reserved bits (set to 0) */
		UINT32	ok : 1;		/* 1 = command completed, no error */
		UINT32	rsrv2 : 1;	/* reserved bits (set to 0) */
		UINT32	c : 1;		/* 1 = command completed */
		UINT32	code : 3;	/* command code (0 = NOP) */
		UINT32	rsrv3 : 10;	/* reserved bits (set to 0) */
		UINT32	i : 1;		/* 1 = interrupt upon completion */
		UINT32	s : 1;		/* 1 = suspend CU upon completion */
		UINT32	el : 1;		/* 1 = last cmdBlock in list */
		union cmdBlock *link;	/* next block in list */
	} nop;
	/* Individual address setup */
	struct {
		UINT32	rsrv1 : 13;	/* reserved bits (set to 0) */
		UINT32	ok : 1;		/* 1 = command completed, no error */
		UINT32	rsrv2 : 1;	/* reserved bits (set to 0) */
		UINT32	c : 1;		/* 1 = command completed */
		UINT32	code : 3;	/* command code (1 = ia setup) */
		UINT32	rsrv3 : 10;	/* reserved bits (set to 0) */
		UINT32	i : 1;		/* 1 = interrupt upon completion */
		UINT32	s : 1;		/* 1 = suspend CU upon completion */
		UINT32	el : 1;		/* 1 = last cmdBlock in list */
		union cmdBlock *link;	/* next block in list */
		UINT8	enetAddr[6];	/* hardware ethernet address */
		UINT16	rsrv4;		/* padding */
	} iaSetup;
	/* Configure */
	struct {
		UINT32	rsrv1 : 13;	/* reserved bits (set to 0) */
		UINT32	ok : 1;		/* 1 = command completed, no error */
		UINT32	rsrv2 : 1;	/* reserved bits (set to 0) */
		UINT32	c : 1;		/* 1 = command completed */
		UINT32	code : 3;	/* command code (2 = configure) */
		UINT32	rsrv3 : 10;	/* reserved bits (set to 0) */
		UINT32	i : 1;		/* 1 = interrupt upon completion */
		UINT32	s : 1;		/* 1 = suspend CU upon completion */
		UINT32	el : 1;		/* 1 = last cmdBlock in list */
		union cmdBlock *link;	/* next block in list */
		UINT8	configData[20];	/* configuration data */
	} configure;
	/* Multicast address setup */
	struct {
		UINT32	rsrv1 : 13;	/* reserved bits (set to 0) */
		UINT32	ok : 1;		/* 1 = command completed, no error */
		UINT32	rsrv2 : 1;	/* reserved bits (set to 0) */
		UINT32	c : 1;		/* 1 = command completed */
		UINT32	code : 3;	/* command code (3 = mc setup) */
		UINT32	rsrv3 : 10;	/* reserved bits (set to 0) */
		UINT32	i : 1;		/* 1 = interrupt upon completion */
		UINT32	s : 1;		/* 1 = suspend CU upon completion */
		UINT32	el : 1;		/* 1 = last cmdBlock in list */
		union cmdBlock *link;	/* next block in list */
		UINT16	mcCount;	/* # of bytes in mcAddrList[] */
		UINT8	mcAddrList[6];	/* list of multicast addresses */
	} mcSetup;
	/* Transmit */
	struct {
		UINT32	rsrv1 : 12;	/* reserved bits (set to 0) */
		UINT32	u : 1;		/* 1 = underrun was encountered */
		UINT32	ok : 1;		/* 1 = command completed, no error */
		UINT32	rsrv2 : 1;	/* reserved bits (set to 0) */
		UINT32	c : 1;		/* 1 = command completed */
		UINT32	code : 3;	/* command code (4 = transmit) */
		UINT32	sf : 1;		/* 1 = flexible mode */
		UINT32	rsrv3 : 9;	/* reserved bits (set to 0) */
		UINT32	i : 1;		/* 1 = interrupt upon completion */
		UINT32	s : 1;		/* 1 = suspend CU upon completion */
		UINT32	el : 1;		/* 1 = last cmdBlock in list */
		union cmdBlock *link;	/* next block in list */
		UINT8	*tbdAddr;	/* tx buf addr; all 1s for simp mode */
		UINT32	tcbCount : 14;	/* # bytes to be tx from cmd block */
		UINT32	rsrv4 : 1;	/* reserved (set to 0) */
		UINT32	eof : 1;	/* 1 = entire frame in cmd block */
		UINT8	tx_threshold;	/* # of bytes in FIFO before xmission */
		UINT8	tbd_number;	/* # of tx. buffers in TBD array */
		UINT8	destAddr[6];	/* destination hardware address */
		UINT16	length;		/* 802.3 packet length (from packet) */
		UINT8	txData[ETHERMTU];	/* optional data to tx */
	} transmit;
	/* Dump 82557 registers */
	struct {
		UINT32	rsrv1 : 13;	/* reserved bits (set to 0) */
		UINT32	ok : 1;		/* 1 = command completed, no error */
		UINT32	rsrv2 : 1;	/* reserved bits (set to 0) */
		UINT32	c : 1;		/* 1 = command completed */
		UINT32	code : 3;	/* command code (6 = dump) */
		UINT32	rsrv3 : 10;	/* reserved bits (set to 0) */
		UINT32	i : 1;		/* 1 = interrupt upon completion */
		UINT32	s : 1;		/* 1 = suspend CU upon completion */
		UINT32	el : 1;		/* 1 = last cmdBlock in list */
		union cmdBlock *link;	/* next block in list */
		UINT8	*bufAddr;	/* where to dump registers */
	} dump;
	/* Diagnose - perform self test */
	struct {
		UINT32	rsrv1 : 11;	/* reserved bits (set to 0) */
		UINT32	f : 1;		/* 1 = self test failed */
		UINT32	rsrv2 : 1;	/* reserved bits (set to 0) */
		UINT32	ok : 1;		/* 1 = command completed, no error */
		UINT32	rsrv3 : 1;	/* reserved bits (set to 0) */
		UINT32	c : 1;		/* 1 = command completed */
		UINT32	code : 3;	/* command code (7 = diagnose) */
		UINT32	rsrv4 : 10;	/* reserved bits (set to 0) */
		UINT32	i : 1;		/* 1 = interrupt upon completion */
		UINT32	s : 1;		/* 1 = suspend CU upon completion */
		UINT32	el : 1;		/* 1 = last cmdBlock in list */
		union cmdBlock *link;	/* next block in list */
	} diagnose;
};

/* Receive frame descriptors (uses simplified memory structure) */
struct rfd {
	UINT32	rxColl : 1;	/* 1 = collision on reception */
	UINT32	iaMatch : 1;	/* Dest addr matched chip's hardware addr */
	UINT32	rsrv1 : 2;	/* reserved bits (set to 0) */
	UINT32	rxErr : 1;	/* RX_ER pin asserted during frame reception */
	UINT32	typeFrame : 1;	/* Type field of pkt. indicates a TYPE frame */
	UINT32	rsrv2 : 1;	/* reserved bits (set to 0) */
	UINT32	frameTooshort : 1;
	UINT32	dmaOverrun : 1;	/* DMA overrun (couldn't get local bus) */
	UINT32	noRsrc : 1;	/* No resources (out of buffer space) */
	UINT32	alignErr : 1;	/* CRC error on misaligned frame */
	UINT32	crcErr : 1;	/* CRC error on aligned frame */
	UINT32	rsrv3 : 1;	/* reserved bits (set to 0) */
	UINT32	ok : 1;		/* 1 = command completed, no error */
	UINT32	rsrv4 : 1;	/* reserved bits (set to 0) */
	UINT32	c : 1;		/* 1 = command completed */
	UINT32	rsrv5 : 3;	/* reserved bits (set to 0) */
	UINT32	sf : 1;		/* 1 = Flexible mode */
	UINT32	h : 1;		/* 1 = Header RFD */
	UINT32	rsrv6 : 9;	/* reserved bits (set to 0) */
	UINT32	s : 1;		/* 1 = suspend CU upon completion */
	UINT32	el : 1;		/* 1 = last cmdBlock in list */
	union cmdBlock *link;	/* next block in list */
	UINT8	*rbdAddr;	/* rx buf desc addr; all 1s for simple mode */
	UINT32	actCount : 14;	/* # bytes in this buffer (set by 82557) */
	UINT32	f : 1;		/* 1 = buffer used */
	UINT32	eof : 1;	/* 1 = last buffer for this frame */
	UINT32	size : 14;	/* # bytes avail in this buffer (set by CPU) */
	UINT32	rsrv7 : 2;	/* reserved bits (set to 0) */
	UINT8	destAddr[6];	/* destination address */
	UINT8	sourceAddr[6];	/* source address */
	UINT16	length;		/* 802.3 packet length (from packet) */
	UINT8	rxData[ETHERMTU];	/* optional data (simplified mode) */
};

/* Forward declarations */
static void portWrite (UINT32 val);
static void resetChip (void);
static void makePacket (UINT8 *, int);
static int checkPacket (UINT8 *, UINT8 *, int);
static int i557IntHandler (int);
static int waitForInt(void);

static void sendCommand (UINT8  cuc,
                         UINT8  ruc,
                         UINT32 scb_general_ptr);

static UINT16 readMDI (
	int	unit,
        UINT8   phyAdd,
        UINT8   regAdd
        );

static void writeMDI (
	int	unit,
        UINT8   phyAdd,
        UINT8   regAdd,
        UINT16  data
        );

static int initPHY (UINT32 device_type, int loop_mode);

static int get_ether_addr (
                int     unit,
                UINT8   *buffer,
                int     print_flag      /* TRUE to print the information */
                );


