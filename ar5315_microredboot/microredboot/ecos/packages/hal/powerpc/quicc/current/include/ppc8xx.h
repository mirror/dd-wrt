#ifndef CYGONCE_HAL_PPC_QUICC_PPC8XX_H
#define CYGONCE_HAL_PPC_QUICC_PPC8XX_H

//==========================================================================
//
//      ppc8xx.h
//
//      PowerPC QUICC register definitions
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002, 2003 Gary Thomas
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
// Author(s):    Red Hat
// Contributors: hmt
// Date:         1999-06-08
// Purpose:      Provide PPC QUICC definitions
// Description:  Provide PPC QUICC definitions
// Usage:        THIS IS NOT AN EXTERNAL API
//               This file is in the include dir to share it between
//               QUICC serial code and MBX initialization code.
//               #include <cyg/hal/quicc/ppc8xx.h>
//               ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#ifdef __ASSEMBLER__

#define SIUMCR     0x000        /* SIU Module configuration */
#define SYPCR      0x004        /* SIU System Protection Control */
#define SIPEND     0x010        /* SIU Interrupt Pending Register */
#define SIMASK     0x014        /* SIU Interrupt MASK Register */
#define SIEL       0x018        /* SIU Interrupt Edge/Level Register */
#define SIVEC      0x01C        /* SIU Interrupt Vector Register */
#define SDCR       0x030        /* SDMA Config Register */
#define BR0        0x100        /* Base Register 0 */
#define OR0        0x104        /* Option Register 0 */
#define BR1        0x108        /* Base Register 1 */
#define OR1        0x10C        /* Option Register 1 */
#define BR2        0x110        /* Base Register 2 */
#define OR2        0x114        /* Option Register 2 */
#define BR3        0x118        /* Base Register 2 */
#define OR3        0x11C        /* Option Register 2 */
#define BR4        0x120        /* Base Register 2 */
#define OR4        0x124        /* Option Register 2 */
#define BR5        0x128        /* Base Register 2 */
#define OR5        0x12C        /* Option Register 2 */
#define BR6        0x130        /* Base Register 2 */
#define OR6        0x134        /* Option Register 2 */
#define BR7        0x138        /* Base Register 2 */
#define OR7        0x13C        /* Option Register 2 */
#define MAR        0x164        /* Memory Address */
#define MCR        0x168        /* Memory Command */
#define MAMR       0x170        /* Machine A Mode Register */
#define MBMR       0x174        /* Machine B Mode Register */
#define MPTPR      0x17A        /* Memory Periodic Timer Prescaler */
#define MDR        0x17C        /* Memory Data */
#define TBSCR      0x200        /* Time Base Status and Control Register */
#define RTCSC      0x220        /* Real Timer Clock Status and Control */
#define PISCR      0x240        /* PIT Status and Control */
#define SCCR       0x280        /* System Clock Control Register */
#define PLPRCR     0x284        /* PLL, Low power & Reset Control Register */
#define RTCSCK     0x320
#define RTCK       0x324
#define RTSECK     0x328
#define RTCALK     0x32C
#define PADIR      0x950        /* Port A - Pin direction */
#define PAPAR      0x952        /* Port A - Pin assignment */
#define PAODR      0x954        /* Port A - Open Drain Control */
#define PADAT      0x956        /* Port A - Data */

#else

#include <cyg/infra/cyg_type.h>

/*****************************************************************
	Communications Processor Buffer Descriptor
*****************************************************************/
struct cp_bufdesc {
    volatile unsigned short ctrl;	/* status/control register */
    volatile unsigned short length;	/* buffer length */
    volatile char  	    *buffer;	/* buffer pointer */
};

/*****************************************************************
	HDLC parameter RAM
*****************************************************************/

struct hdlc_pram {
    /*
     * SCC parameter RAM
     */
    unsigned short	rbase;		/* RX BD base address */
    unsigned short	tbase;		/* TX BD base address */
    unsigned char	rfcr;		/* Rx function code */
    unsigned char	tfcr;		/* Tx function code */
    unsigned short	mrblr;		/* Rx buffer length */
    unsigned long	rstate;		/* Rx internal state */
    unsigned long	rptr;		/* Rx internal data pointer */
    unsigned short	rbptr;		/* rb BD Pointer */
    unsigned short	rcount;		/* Rx internal byte count */
    unsigned long	rtemp;		/* Rx temp */
    unsigned long	tstate;		/* Tx internal state */
    unsigned long	tptr;		/* Tx internal data pointer */
    unsigned short	tbptr;		/* Tx BD pointer */
    unsigned short	tcount;		/* Tx byte count */
    unsigned long	ttemp;		/* Tx temp */
    unsigned long	rcrc;		/* temp receive CRC */
    unsigned long	tcrc;		/* temp transmit CRC */

    /*
     * HDLC specific parameter RAM
     */
    unsigned char	RSRVD1[4];	
    unsigned long	c_mask;		/* CRC constant */
    unsigned long	c_pres;		/* CRC preset */
    unsigned short	disfc;		/* discarded frame counter */
    unsigned short	crcec;		/* CRC error counter */
    unsigned short	abtsc;		/* abort sequence counter */
    unsigned short	nmarc;		/* nonmatching address rx cnt */
    unsigned short	retrc;		/* frame retransmission cnt */
    unsigned short	mflr;		/* maximum frame length reg */
    unsigned short	max_cnt;	/* maximum length counter */
    unsigned short	rfthr;		/* received frames threshold */
    unsigned short	rfcnt;		/* received frames count */
    unsigned short	hmask;		/* user defined frm addr mask */
    unsigned short	haddr1;		/* user defined frm address 1 */
    unsigned short	haddr2;		/* user defined frm address 2 */
    unsigned short	haddr3;		/* user defined frm address 3 */
    unsigned short	haddr4;		/* user defined frm address 4 */
    unsigned short	tmp;		/* temp */
    unsigned short	tmp_mb;		/* temp */
};


/*****************************************************************
	ASYNC HDLC parameter RAM
*****************************************************************/

struct async_hdlc_pram {
    /*
     * SCC parameter RAM
     */
    unsigned short	rbase;		/* RX BD base address */
    unsigned short	tbase;		/* TX BD base address */
    unsigned char	rfcr;		/* Rx function code */
    unsigned char	tfcr;		/* Tx function code */
    unsigned short	mrblr;		/* Rx buffer length */
    unsigned long	rstate;		/* Rx internal state */
    unsigned long	rptr;		/* Rx internal data pointer */
    unsigned short	rbptr;		/* rb BD Pointer */
    unsigned short	rcount;		/* Rx internal byte count */
    unsigned long	rtemp;		/* Rx temp */
    unsigned long	tstate;		/* Tx internal state */
    unsigned long	tptr;		/* Tx internal data pointer */
    unsigned short	tbptr;		/* Tx BD pointer */
    unsigned short	tcount;		/* Tx byte count */
    unsigned long	ttemp;		/* Tx temp */
    unsigned long	rcrc;		/* temp receive CRC */
    unsigned long	tcrc;		/* temp transmit CRC */

    /*
     * ASYNC HDLC specific parameter RAM
     */
    unsigned char	RSRVD1[4];	
    unsigned long	c_mask;		/* CRC constant */
    unsigned long	c_pres;		/* CRC preset */
    unsigned short	bof;		/* begining of flag character */
    unsigned short	eof;		/* end of flag character */
    unsigned short	esc;		/* control escape character */
    unsigned char	RSRVD2[4];	
    unsigned short	zero;		/* zero */
    unsigned char	RSRVD3[2];	
    unsigned short	rfthr;		/* received frames threshold */
    unsigned char	RSRVD4[4];	
    unsigned long	txctl_tbl;	/* Tx ctl char mapping table */
    unsigned long	rxctl_tbl;	/* Rx ctl char mapping table */
    unsigned short	nof;		/* Number of opening flags */
};


/*****************************************************************
	UART parameter RAM
*****************************************************************/

/*
 * bits in uart control characters table
 */
#define	CC_INVALID	0x8000		/* control character is valid */
#define	CC_REJ		0x4000		/* don't store char in buffer */
#define	CC_CHAR		0x00ff		/* control character */

/* UART */
struct uart_pram {
    /*
     * SCC parameter RAM
     */
    unsigned short	rbase;		/* RX BD base address */
    unsigned short	tbase;		/* TX BD base address */
    unsigned char	rfcr;		/* Rx function code */
    unsigned char	tfcr;		/* Tx function code */
    unsigned short	mrblr;		/* Rx buffer length */
    unsigned long	rstate;		/* Rx internal state */
    unsigned long	rptr;		/* Rx internal data pointer */
    unsigned short	rbptr;		/* rb BD Pointer */
    unsigned short	rcount;		/* Rx internal byte count */
    unsigned long	rx_temp;	/* Rx temp */
    unsigned long	tstate;		/* Tx internal state */
    unsigned long	tptr;		/* Tx internal data pointer */
    unsigned short	tbptr;		/* Tx BD pointer */
    unsigned short	tcount;		/* Tx byte count */
    unsigned long	ttemp;		/* Tx temp */
    unsigned long	rcrc;		/* temp receive CRC */
    unsigned long	tcrc;		/* temp transmit CRC */

    /*
     * UART specific parameter RAM
     */
    unsigned char	RSRVD1[8];	
    unsigned short	max_idl;	/* maximum idle characters */
    unsigned short	idlc;		/* rx idle counter (internal) */
    unsigned short	brkcr;		/* break count register */

    unsigned short	parec;		/* Rx parity error counter */
    unsigned short	frmer;		/* Rx framing error counter */
    unsigned short	nosec;		/* Rx noise counter */
    unsigned short	brkec;		/* Rx break character counter */
    unsigned short	brkln;		/* Reaceive break length */

    unsigned short	uaddr1;		/* address character 1 */
    unsigned short	uaddr2;		/* address character 2 */
    unsigned short	rtemp;		/* temp storage */
    unsigned short	toseq;		/* Tx out of sequence char */
    unsigned short	cc[8];		/* Rx control characters */
    unsigned short	rccm;		/* Rx control char mask */
    unsigned short	rccr;		/* Rx control char register */
    unsigned short	rlbc;		/* Receive last break char */
};



/*****************************************************************
	BISYNC parameter RAM
*****************************************************************/

struct bisync_pram {
    /*
     * SCC parameter RAM
     */
    unsigned short	rbase;		/* RX BD base address */
    unsigned short	tbase;		/* TX BD base address */
    unsigned char	rfcr;		/* Rx function code */
    unsigned char	tfcr;		/* Tx function code */
    unsigned short	mrblr;		/* Rx buffer length */
    unsigned long	rstate;		/* Rx internal state */
    unsigned long	rptr;		/* Rx internal data pointer */
    unsigned short	rbptr;		/* rb BD Pointer */
    unsigned short	rcount;		/* Rx internal byte count */
    unsigned long	rtemp;		/* Rx temp */
    unsigned long	tstate;		/* Tx internal state */
    unsigned long	tptr;		/* Tx internal data pointer */
    unsigned short	tbptr;		/* Tx BD pointer */
    unsigned short	tcount;		/* Tx byte count */
    unsigned long	ttemp;		/* Tx temp */
    unsigned long	rcrc;		/* temp receive CRC */
    unsigned long	tcrc;		/* temp transmit CRC */

    /*
     * BISYNC specific parameter RAM
     */
    unsigned char	RSRVD1[4];	
    unsigned long	crcc;		/* CRC Constant Temp Value */
    unsigned short	prcrc;		/* Preset Receiver CRC-16/LRC */
    unsigned short	ptcrc;		/* Preset Transmitter CRC-16/LRC */
    unsigned short	parec;		/* Receive Parity Error Counter */
    unsigned short	bsync;		/* BISYNC SYNC Character */
    unsigned short	bdle;		/* BISYNC DLE Character */
    unsigned short	cc[8];		/* Rx control characters */
    unsigned short	rccm;		/* Receive Control Character Mask */
};

/*****************************************************************
	IOM2 parameter RAM
	(overlaid on tx bd[5] of SCC channel[2])
*****************************************************************/
struct iom2_pram {
    unsigned short	ci_data;	/* ci data */
    unsigned short	monitor_data;	/* monitor data */
    unsigned short	tstate;		/* transmitter state */
    unsigned short	rstate;		/* receiver state */
};

/*****************************************************************
	SPI/SMC parameter RAM
	(overlaid on tx bd[6,7] of SCC channel[2])
*****************************************************************/

#define	SPI_R	0x8000		/* Ready bit in BD */

struct spi_pram {
    unsigned short	rbase;		/* Rx BD Base Address */
    unsigned short	tbase;		/* Tx BD Base Address */
    unsigned char	rfcr;		/* Rx function code */
    unsigned char	tfcr;		/* Tx function code */
    unsigned short	mrblr;		/* Rx buffer length */
    unsigned long	rstate;		/* Rx internal state */
    unsigned long	rptr;		/* Rx internal data pointer */
    unsigned short	rbptr;		/* rb BD Pointer */
    unsigned short	rcount;		/* Rx internal byte count */
    unsigned long	rtemp;		/* Rx temp */
    unsigned long	tstate;		/* Tx internal state */
    unsigned long	tptr;		/* Tx internal data pointer */
    unsigned short	tbptr;		/* Tx BD pointer */
    unsigned short	tcount;		/* Tx byte count */
    unsigned long	ttemp[2];	/* Tx temp */
    unsigned short	rpbase;		/* Relocated param block pointer */
    unsigned short	res;		/* unused */
};

struct smc_uart_pram {
    unsigned short	rbase;		/* Rx BD Base Address */
    unsigned short	tbase;		/* Tx BD Base Address */
    unsigned char	rfcr;		/* Rx function code */
    unsigned char	tfcr;		/* Tx function code */
    unsigned short	mrblr;		/* Rx buffer length */
    unsigned long	rstate;		/* Rx internal state */
    unsigned long	rptr;		/* Rx internal data pointer */
    unsigned short	rbptr;		/* rb BD Pointer */
    unsigned short	rcount;		/* Rx internal byte count */
    unsigned long	rtemp;		/* Rx temp */
    unsigned long	tstate;		/* Tx internal state */
    unsigned long	tptr;		/* Tx internal data pointer */
    unsigned short	tbptr;		/* Tx BD pointer */
    unsigned short	tcount;		/* Tx byte count */
    unsigned long	ttemp;		/* Tx temp */
    unsigned short	max_idl;	/* Maximum IDLE Characters */
    unsigned short	idlc;		/* Temporary IDLE Counter */
    unsigned short	brkln;		/* Last Rx Break Length */
    unsigned short	brkec;		/* Rx Break Condition Counter */
    unsigned short	brkcr;		/* Break Count Register (Tx) */
    unsigned short	r_mask;		/* Temporary bit mask */
};

struct smc_trnsp_pram {
    unsigned short	rbase;		/* Rx BD Base Address */
    unsigned short	tbase;		/* Tx BD Base Address */
    unsigned char	rfcr;		/* Rx function code */
    unsigned char	tfcr;		/* Tx function code */
    unsigned short	mrblr;		/* Rx buffer length */
    unsigned long	rstate;		/* Rx internal state */
    unsigned long	rptr;		/* Rx internal data pointer */
    unsigned short	rbptr;		/* rb BD Pointer */
    unsigned short	rcount;		/* Rx internal byte count */
    unsigned long	rtemp;		/* Rx temp */
    unsigned long	tstate;		/* Tx internal state */
    unsigned long	tptr;		/* Tx internal data pointer */
    unsigned short	tbptr;		/* Tx BD pointer */
    unsigned short	tcount;		/* Tx byte count */
    unsigned long	ttemp;		/* Tx temp */
    unsigned short	RSRVD[5];	/* RSRVD */
};

struct centronics_pram {
    unsigned short	rbase;		/* Rx BD Base Address */
    unsigned short	tbase;		/* Tx BD Base Address */
    unsigned char	fcr;		/* function code */
    unsigned char	smask;		/* Status Mask */
    unsigned short	mrblr;		/* Rx buffer length */
    unsigned long	rstate;		/* Rx internal state */
    unsigned long	rptr;		/* Rx internal data pointer */
    unsigned short	rbptr;		/* rb BD Pointer */
    unsigned short	rcount;		/* Rx internal byte count */
    unsigned long	rtemp;		/* Rx temp */
    unsigned long	tstate;		/* Tx internal state */
    unsigned long	tptr;		/* Tx internal data pointer */
    unsigned short	tbptr;		/* Tx BD pointer */
    unsigned short	tcount;		/* Tx byte count */
    unsigned long	ttemp;		/* Tx temp */
    unsigned short	max_sl;		/* Maximum Silence period */
    unsigned short	sl_cnt;		/* Silence Counter */
    unsigned short	char1;		/* CONTROL char 1 */
    unsigned short	char2;		/* CONTROL char 2 */
    unsigned short	char3;		/* CONTROL char 3 */
    unsigned short	char4;		/* CONTROL char 4 */
    unsigned short	char5;		/* CONTROL char 5 */
    unsigned short	char6;		/* CONTROL char 6 */
    unsigned short	char7;		/* CONTROL char 7 */
    unsigned short	char8;		/* CONTROL char 8 */
    unsigned short	rccm;		/* Rx Control Char Mask */
    unsigned short	rccr;		/* Rx Char Control Register */
};

struct idma_pram {
    unsigned short	ibase;		/* IDMA BD Base Address */
    unsigned short	ibptr;		/* IDMA buffer descriptor pointer */
    unsigned long	istate;		/* IDMA internal state */
    unsigned long	itemp;		/* IDMA temp */
};

struct ethernet_pram {
    /*
     * SCC parameter RAM
     */
    unsigned short	rbase;		/* RX BD base address */
    unsigned short	tbase;		/* TX BD base address */
    unsigned char	rfcr;		/* Rx function code */
    unsigned char	tfcr;		/* Tx function code */
    unsigned short	mrblr;		/* Rx buffer length */
    unsigned long	rstate;		/* Rx internal state */
    unsigned long	rptr;		/* Rx internal data pointer */
    unsigned short	rbptr;		/* rb BD Pointer */
    unsigned short	rcount;		/* Rx internal byte count */
    unsigned long	rtemp;		/* Rx temp */
    unsigned long	tstate;		/* Tx internal state */
    unsigned long	tptr;		/* Tx internal data pointer */
    unsigned short	tbptr;		/* Tx BD pointer */
    unsigned short	tcount;		/* Tx byte count */
    unsigned long	ttemp;		/* Tx temp */
    unsigned long	rcrc;		/* temp receive CRC */
    unsigned long	tcrc;		/* temp transmit CRC */

    /*
     * ETHERNET specific parameter RAM
     */
    unsigned long	c_pres;		/* preset CRC */
    unsigned long	c_mask;		/* constant mask for CRC */
    unsigned long	crcec;		/* CRC error counter */
    unsigned long	alec;		/* alighnment error counter */
    unsigned long	disfc;		/* discard frame counter */
    unsigned short	pads;		/* short frame PAD characters */
    unsigned short	ret_lim;	/* retry limit threshold */
    unsigned short	ret_cnt;	/* retry limit counter */
    unsigned short	mflr;		/* maximum frame length reg */
    unsigned short	minflr;		/* minimum frame length reg */
    unsigned short	maxd1;		/* maximum DMA1 length reg */
    unsigned short	maxd2;		/* maximum DMA2 length reg */
    unsigned short	maxd;		/* rx max DMA */
    unsigned short	dma_cnt;	/* rx dma counter */
    unsigned short	max_b;		/* max bd byte count */
    unsigned short	gaddr1;		/* group address filter 1 */
    unsigned short	gaddr2;		/* group address filter 2 */
    unsigned short	gaddr3;		/* group address filter 3 */
    unsigned short	gaddr4;		/* group address filter 4 */
    unsigned long	tbuf0_data0;	/* save area 0 - current frm */
    unsigned long	tbuf0_data1;	/* save area 1 - current frm */
    unsigned long	tbuf0_rba0;
    unsigned long	tbuf0_crc;
    unsigned short	tbuf0_bcnt;
    unsigned short	paddr_h;	/* physical address (MSB) */
    unsigned short	paddr_m;	/* physical address */
    unsigned short	paddr_l;	/* physical address (LSB) */
    unsigned short	p_per;		/* persistence */
    unsigned short	rfbd_ptr;	/* rx first bd pointer */
    unsigned short	tfbd_ptr;	/* tx first bd pointer */
    unsigned short	tlbd_ptr;	/* tx last bd pointer */
    unsigned long	tbuf1_data0;	/* save area 0 - next frame */
    unsigned long	tbuf1_data1;	/* save area 1 - next frame */
    unsigned long	tbuf1_rba0;
    unsigned long	tbuf1_crc;
    unsigned short	tbuf1_bcnt;
    unsigned short	tx_len;		/* tx frame length counter */
    unsigned short	iaddr1;		/* individual address filter 1*/
    unsigned short	iaddr2;		/* individual address filter 2*/
    unsigned short	iaddr3;		/* individual address filter 3*/
    unsigned short	iaddr4;		/* individual address filter 4*/
    unsigned short	boff_cnt;	/* back-off counter */
    unsigned short	taddr_h;	/* temp address (MSB) */
    unsigned short	taddr_m;	/* temp address */
    unsigned short	taddr_l;	/* temp address (LSB) */
};

struct transparent_pram {
    /*
     * SCC parameter RAM
     */
    unsigned short	rbase;		/* RX BD base address */
    unsigned short	tbase;		/* TX BD base address */
    unsigned char	rfcr;		/* Rx function code */
    unsigned char	tfcr;		/* Tx function code */
    unsigned short	mrblr;		/* Rx buffer length */
    unsigned long	rstate;		/* Rx internal state */
    unsigned long	rptr;		/* Rx internal data pointer */
    unsigned short	rbptr;		/* rb BD Pointer */
    unsigned short	rcount;		/* Rx internal byte count */
    unsigned long	rtemp;		/* Rx temp */
    unsigned long	tstate;		/* Tx internal state */
    unsigned long	tptr;		/* Tx internal data pointer */
    unsigned short	tbptr;		/* Tx BD pointer */
    unsigned short	tcount;		/* Tx byte count */
    unsigned long	ttemp;		/* Tx temp */
    unsigned long	rcrc;		/* temp receive CRC */
    unsigned long	tcrc;		/* temp transmit CRC */

    /*
     * TRANSPARENT specific parameter RAM
     */
    unsigned long	crc_p;		/* CRC Preset */
    unsigned long	crc_c;		/* CRC constant */
};

struct timer_pram {
    /*
     * RISC timers parameter RAM
     */
    unsigned short	tm_base;	/* RISC timer table base adr */
    unsigned short	tm_ptr;		/* RISC timer table pointer */
    unsigned short	r_tmr;		/* RISC timer mode register */
    unsigned short	r_tmv;		/* RISC timer valid register */
    unsigned long	tm_cmd;		/* RISC timer cmd register */
    unsigned long	tm_cnt;		/* RISC timer internal cnt */
};

struct ucode_pram {
    /*
     * RISC ucode parameter RAM
     */
    unsigned short	rev_num;	/* Ucode Revision Number */
    unsigned short	d_ptr;		/* MISC Dump area pointer */
    unsigned long	temp1;		/* MISC Temp1 */
    unsigned long	temp2;		/* MISC Temp2 */
};

struct i2c_pram {
    /*
     * I2C parameter RAM
     */
    unsigned short	rbase;		/* RX BD base address */
    unsigned short	tbase;		/* TX BD base address */
    unsigned char	rfcr;		/* Rx function code */
    unsigned char	tfcr;		/* Tx function code */
    unsigned short	mrblr;		/* Rx buffer length */
    unsigned long	rstate;		/* Rx internal state */
    unsigned long	rptr;		/* Rx internal data pointer */
    unsigned short	rbptr;		/* rb BD Pointer */
    unsigned short	rcount;		/* Rx internal byte count */
    unsigned long	rtemp;		/* Rx temp */
    unsigned long	tstate;		/* Tx internal state */
    unsigned long	tptr;		/* Tx internal data pointer */
    unsigned short	tbptr;		/* Tx BD pointer */
    unsigned short	tcount;		/* Tx byte count */
    unsigned long	ttemp[2];	/* Tx temp */
    unsigned short	rpbase;		/* Relocated param block pointer */
    unsigned short	res;		/* unused */
};

/*
 * definitions of EPPC memory structures
 */
typedef struct eppc {
    /* BASE + 0x0000: INTERNAL REGISTERS */

    /* SIU */
    volatile unsigned long	siu_mcr;	/* module configuration reg */
    volatile unsigned long	siu_sypcr;	/* System protection cnt */
    unsigned char		RSRVD58[0x6];
    volatile unsigned short	siu_swsr;	/* sw service */
    volatile unsigned long	siu_sipend;	/* Interrupt pend reg */
    volatile unsigned long	siu_simask;	/* Interrupt mask reg */
    volatile unsigned long	siu_siel;	/* Interrupt edge level mask reg */
    volatile unsigned long	siu_sivec;	/* Interrupt vector */
    volatile unsigned long	siu_tesr;	/* Transfer error status */
    volatile unsigned char	RSRVD1[0xc];
    volatile unsigned long	dma_sdcr;	/* SDMA configuration reg */
    unsigned char   		RSRVD55[0x4c];

    /* PCMCIA */
    volatile unsigned long	pcmcia_pbr0;    /* PCMCIA Base Reg: Window 0 */
    volatile unsigned long      pcmcia_por0;    /* PCMCIA Option Reg: Window 0 */
    volatile unsigned long      pcmcia_pbr1;    /* PCMCIA Base Reg: Window 1 */
    volatile unsigned long      pcmcia_por1;    /* PCMCIA Option Reg: Window 1 */
    volatile unsigned long      pcmcia_pbr2;    /* PCMCIA Base Reg: Window 2 */
    volatile unsigned long      pcmcia_por2;    /* PCMCIA Option Reg: Window 2 */
    volatile unsigned long      pcmcia_pbr3;    /* PCMCIA Base Reg: Window 3 */
    volatile unsigned long      pcmcia_por3;    /* PCMCIA Option Reg: Window 3 */
    volatile unsigned long      pcmcia_pbr4;    /* PCMCIA Base Reg: Window 4 */
    volatile unsigned long      pcmcia_por4;    /* PCMCIA Option Reg: Window 4 */
    volatile unsigned long      pcmcia_pbr5;    /* PCMCIA Base Reg: Window 5 */
    volatile unsigned long      pcmcia_por5;    /* PCMCIA Option Reg: Window 5 */
    volatile unsigned long      pcmcia_pbr6;    /* PCMCIA Base Reg: Window 6 */
    volatile unsigned long      pcmcia_por6;    /* PCMCIA Option Reg: Window 6 */
    volatile unsigned long      pcmcia_pbr7;    /* PCMCIA Base Reg: Window 7 */
    volatile unsigned long      pcmcia_por7;    /* PCMCIA Option Reg: Window 7 */
    volatile unsigned char	RSRVD2[0x20];
    volatile unsigned long      pcmcia_pgcra;   /* PCMCIA Slot A Control  Reg */
    volatile unsigned long      pcmcia_pgcrb;   /* PCMCIA Slot B Control  Reg */
    volatile unsigned long      pcmcia_pscr;    /* PCMCIA Status Reg */
    volatile unsigned char	RSRVD2a[0x4];
    volatile unsigned long      pcmcia_pipr;    /* PCMCIA Pins Value Reg */
    volatile unsigned char	RSRVD2b[0x4];
    volatile unsigned long      pcmcia_per;     /* PCMCIA Enable Reg */
    volatile unsigned char	RSRVD2c[0x4];

    /* MEMC */
    volatile unsigned long	memc_br0;	/* base register 0 */
    volatile unsigned long	memc_or0;	/* option register 0 */
    volatile unsigned long	memc_br1;	/* base register 1 */
    volatile unsigned long	memc_or1;	/* option register 1 */
    volatile unsigned long	memc_br2;	/* base register 2 */
    volatile unsigned long	memc_or2;	/* option register 2 */
    volatile unsigned long	memc_br3;	/* base register 3 */
    volatile unsigned long	memc_or3;	/* option register 3 */
    volatile unsigned long	memc_br4;	/* base register 3 */
    volatile unsigned long	memc_or4;	/* option register 3 */
    volatile unsigned long	memc_br5;	/* base register 3 */
    volatile unsigned long	memc_or5;	/* option register 3 */
    volatile unsigned long	memc_br6;	/* base register 3 */
    volatile unsigned long	memc_or6;	/* option register 3 */
    volatile unsigned long	memc_br7;	/* base register 3 */
    volatile unsigned long	memc_or7;	/* option register 3 */
    volatile unsigned char	RSRVD3[0x24];	
    volatile unsigned long	memc_mar;	/* Memory address */
    volatile unsigned long	memc_mcr;	/* Memory command */
    volatile unsigned char	RSRVD4[0x4];	
    volatile unsigned long	memc_mamr;	/* Machine A mode */
    volatile unsigned long	memc_mbmr;	/* Machine B mode */
    volatile unsigned short	memc_mstat;	/* Memory status */
    volatile unsigned short	memc_mptpr;	/* Memory preidic timer prescalar */
    volatile unsigned long	memc_mdr;	/* Memory data */
    volatile unsigned char	RSRVD5[0x80];

    /* SYSTEM INTEGRATION TIMERS */
    volatile unsigned short	simt_tbscr;	/* Time base stat&ctr */
    volatile unsigned char	RSRVD100[0x2];
    volatile unsigned long	simt_tbreff0;	/* Time base reference 0 */
    volatile unsigned long	simt_tbreff1;	/* Time base reference 1 */
    volatile unsigned char	RSRVD6[0x14];
    volatile unsigned short	simt_rtcsc;	/* Realtime clk stat&cntr 1 */
    volatile unsigned char	RSRVD110[0x2];
    volatile unsigned long	simt_rtc;	/* Realtime clock */
    volatile unsigned long	simt_rtsec;	/* Realtime alarm seconds */
    volatile unsigned long	simt_rtcal;	/* Realtime alarm */
    volatile unsigned char	RSRVD56[0x10];
    volatile unsigned long	simt_piscr;	/* PIT stat&ctrl */
    volatile unsigned long	simt_pitc;	/* PIT counter */
    volatile unsigned long	simt_pitr;	/* PIT */
    volatile unsigned char	RSRVD7[0x34];

    /* CLOCKS, RESET */
    volatile unsigned long	clkr_sccr;	/* System clk cntrl */
    volatile unsigned long	clkr_plprcr;	/* PLL reset&ctrl */
    volatile unsigned long	clkr_rsr;	/* reset status */
    volatile unsigned char	RSRVD66a[0x74];

    /* System Integration Timers Keys */
    volatile unsigned long	simt_tbscrk;	/* Timebase Status&Ctrl Key */
    volatile unsigned long	simt_tbreff0k;  /* Timebase Reference 0 Key */
    volatile unsigned long	simt_tbreff1k;  /* Timebase Reference 1 Key */
    volatile unsigned long	simt_tbk;       /* Timebase and Decrementer Key */
    volatile unsigned char	RSRVD66b[0x10];
    volatile unsigned long	simt_rtcsck;    /* Real-Time Clock Status&Ctrl Key */
    volatile unsigned long	simt_rtck;      /* Real-Time Clock Key */
    volatile unsigned long	simt_rtseck;    /* Real-Time Alarm Seconds Key */
    volatile unsigned long	simt_rtcalk;    /* Real-Time Alarm Key */
    volatile unsigned char	RSRVD66c[0x10];
    volatile unsigned long	simt_piscrk;    /* Periodic Interrupt Status&Ctrl Key */
    volatile unsigned long	simt_pitck;     /* Periodic Interrupt Count Key */
    volatile unsigned char	RSRVD66d[0x38];


    /* Clock and Reset Keys */
    volatile unsigned long	clkr_sccrk;	/* System Clock Control Key */
    volatile unsigned long	clkr_plprcrk;	/* PLL, Low Power and Reset Control Key */
    volatile unsigned long	clkr_rsrk;	/* Reset Status Key */
    volatile unsigned char	RSRVD66e[0x4b4];

    volatile unsigned long	lcd_lccr;	/* configuration Reg */
    volatile unsigned long	lcd_lchcr;	/* Horizontal ctl Reg */
    volatile unsigned long	lcd_lcvcr;	/* Vertical ctl Reg */
    unsigned char		RSRVD67[4];
    volatile unsigned long	lcd_lcfaa;	/* Frame buffer A Address */
    volatile unsigned long	lcd_lcfba;	/* Frame buffer B Address */
    volatile unsigned char	lcd_lcsr;	/* Status Reg */
    volatile unsigned char	RSRVD9[0x7];

    /* I2C */
    volatile unsigned char	i2c_i2mod;	/* i2c mode */
    unsigned char		RSRVD59[3];
    volatile unsigned char	i2c_i2add;	/* i2c address */
    unsigned char		RSRVD60[3];
    volatile unsigned char	i2c_i2brg;	/* i2c brg */
    unsigned char		RSRVD61[3];
    volatile unsigned char	i2c_i2com;	/* i2c command */
    unsigned char		RSRVD62[3];
    volatile unsigned char	i2c_i2cer;	/* i2c event */
    unsigned char		RSRVD63[3];
    volatile unsigned char	i2c_i2cmr;	/* i2c mask */
    volatile unsigned char	RSRVD10[0x0b];
    volatile unsigned char	i2c_spare_pram[0x80];  /* Used by patched ucode */

    /* DMA */
    volatile unsigned char	RSRVD11[0x4];
    volatile unsigned long	dma_sdar;	/* SDMA address reg */
    volatile unsigned char	dma_sdsr;	/* SDMA status reg */
    volatile unsigned char	RSRVD12[0x3];
    volatile unsigned char	dma_sdmr;	/* SDMA mask reg */
    volatile unsigned char	RSRVD13[0x3];
    volatile unsigned char	dma_idsr1;	/* IDMA1 status reg */
    volatile unsigned char	RSRVD14[0x3];
    volatile unsigned char	dma_idmr1;	/* IDMA1 mask reg */
    volatile unsigned char	RSRVD15[0x3];
    volatile unsigned char	dma_idsr2;	/* IDMA2 status reg */
    volatile unsigned char	RSRVD16[0x3];
    volatile unsigned char	dma_idmr2;	/* IDMA2 mask reg */
    volatile unsigned char	RSRVD17[0x13];

    /* CPM Interrupt Controller */
    volatile unsigned short	cpmi_civr;	/* CP interrupt vector reg */
    volatile unsigned char	RSRVD19[0xe];
    volatile unsigned long	cpmi_cicr;	/* CP interrupt configuration reg */
    volatile unsigned long	cpmi_cipr;	/* CP interrupt pending reg */
    volatile unsigned long	cpmi_cimr;	/* CP interrupt mask reg */
    volatile unsigned long	cpmi_cisr;	/* CP interrupt in-service reg */

    /* I/O port */
    volatile unsigned short	pio_padir;	/* port A data direction reg */
    volatile unsigned short	pio_papar;	/* port A pin assignment reg */
    volatile unsigned short	pio_paodr;	/* port A open drain reg */
    volatile unsigned short	pio_padat;	/* port A data register */
    volatile unsigned char	RSRVD20[0x8];	
    volatile unsigned short	pio_pcdir;	/* port C data direction reg */
    volatile unsigned short	pio_pcpar;	/* port C pin assignment reg */
    volatile unsigned short	pio_pcso;	/* port C special options */
    volatile unsigned short	pio_pcdat;	/* port C data register */
    volatile unsigned short	pio_pcint;	/* port C interrupt cntrl reg */
    unsigned char		RSRVD64[6];
    volatile unsigned short	pio_pddir;	/* port D Data Direction reg */
    volatile unsigned short	pio_pdpar;	/* port D pin assignment reg */
    unsigned char		RSRVD65[2];
    volatile unsigned short	pio_pddat;	/* port D data reg */
    volatile unsigned char	RSRVD21[0x8];	

    /* CPM Timer */
    volatile unsigned short	timer_tgcr;	/* timer global configuration  reg */
    volatile unsigned char	RSRVD22[0xe];	
    volatile unsigned short	timer_tmr1;	/* timer 1 mode reg */
    volatile unsigned short	timer_tmr2;	/* timer 2 mode reg */
    volatile unsigned short	timer_trr1;	/* timer 1 referance reg */
    volatile unsigned short	timer_trr2;	/* timer 2 referance reg */
    volatile unsigned short	timer_tcr1;	/* timer 1 capture reg */
    volatile unsigned short	timer_tcr2;	/* timer 2 capture reg */
    volatile unsigned short	timer_tcn1;	/* timer 1 counter reg */
    volatile unsigned short	timer_tcn2;	/* timer 2 counter reg */
    volatile unsigned short	timer_tmr3;	/* timer 3 mode reg */
    volatile unsigned short	timer_tmr4;	/* timer 4 mode reg */
    volatile unsigned short	timer_trr3;	/* timer 3 referance reg */
    volatile unsigned short	timer_trr4;	/* timer 4 referance reg */
    volatile unsigned short	timer_tcr3;	/* timer 3 capture reg */
    volatile unsigned short	timer_tcr4;	/* timer 4 capture reg */
    volatile unsigned short	timer_tcn3;	/* timer 3 counter reg */
    volatile unsigned short	timer_tcn4;	/* timer 4 counter reg */
    volatile unsigned short	timer_ter1;	/* timer 1 event reg */
    volatile unsigned short	timer_ter2;	/* timer 2 event reg */
    volatile unsigned short	timer_ter3;	/* timer 3 event reg */
    volatile unsigned short	timer_ter4;	/* timer 4 event reg */
    volatile unsigned char	RSRVD23[0x8];	

    /* CP */
    volatile unsigned short	cp_cr;		/* command register */
    volatile unsigned char	RSRVD24[0x2];	
    volatile unsigned short	cp_rccr;	/* main configuration reg */
    volatile unsigned char	RSRVD25;	
    volatile unsigned char	cp_resv1;	/* RSRVD reg */
    volatile unsigned long	cp_resv2;	/* RSRVD reg */
    volatile unsigned short	cp_rctr1;	/* ram break register 1 */
    volatile unsigned short	cp_rctr2;	/* ram break register 2 */
    volatile unsigned short	cp_rctr3;	/* ram break register 3 */
    volatile unsigned short	cp_rctr4;	/* ram break register 4 */
    volatile unsigned char	RSRVD26[0x2];	
    volatile unsigned short	cp_rter;	/* RISC timers event reg */
    volatile unsigned char	RSRVD27[0x2];	
    volatile unsigned short	cp_rtmr;	/* RISC timers mask reg */
    volatile unsigned char	RSRVD28[0x14];	

    /* BRG */
    volatile unsigned long	brgc1;		/* BRG1 configuration reg */
    volatile unsigned long	brgc2;		/* BRG2 configuration reg */
    volatile unsigned long	brgc3;		/* BRG3 configuration reg */
    volatile unsigned long	brgc4;		/* BRG4 configuration reg */

    /* SCC registers */
    struct scc_regs {
	volatile unsigned long	scc_gsmr_l;	/* SCC Gen mode (LOW) */
	volatile unsigned long	scc_gsmr_h;	/* SCC Gen mode (HIGH) */
	volatile unsigned short	scc_psmr;	/* protocol specific mode register */
	volatile unsigned char	RSRVD29[0x2]; 
	volatile unsigned short	scc_todr;	/* SCC transmit on demand */
	volatile unsigned short	scc_dsr;	/* SCC data sync reg */
	volatile unsigned short	scc_scce;	/* SCC event reg */
	volatile unsigned char	RSRVD30[0x2];
	volatile unsigned short	scc_sccm;	/* SCC mask reg */
	volatile unsigned char	RSRVD31[0x1];
	volatile unsigned char	scc_sccs;	/* SCC status reg */
	volatile unsigned char	RSRVD32[0x8]; 
    } scc_regs[4];

    /* SMC */
    struct smc_regs {
	volatile unsigned char	RSRVD34[0x2]; 
	volatile unsigned short	smc_smcmr;	/* SMC mode reg */
	volatile unsigned char	RSRVD35[0x2];
	volatile unsigned char	smc_smce;	/* SMC event reg */
	volatile unsigned char	RSRVD36[0x3]; 
	volatile unsigned char	smc_smcm;	/* SMC mask reg */
	volatile unsigned char	RSRVD37[0x5]; 
    } smc_regs[2];

    /* SPI */
    volatile unsigned short	spi_spmode;	/* SPI mode reg */
    volatile unsigned char	RSRVD38[0x4];	
    volatile unsigned char	spi_spie;	/* SPI event reg */
    volatile unsigned char	RSRVD39[0x3];	
    volatile unsigned char	spi_spim;	/* SPI mask reg */
    volatile unsigned char	RSRVD40[0x2];	
    volatile unsigned char	spi_spcom;	/* SPI command reg */
    volatile unsigned char	RSRVD41[0x4];	

    /* PIP */
    volatile unsigned short	pip_pipc;	/* pip configuration reg */
    volatile unsigned char	RSRVD42[0x2];	
    volatile unsigned short	pip_ptpr;	/* pip timing parameters reg */
    volatile unsigned long	pip_pbdir;	/* port b data direction reg */
    volatile unsigned long	pip_pbpar;	/* port b pin assignment reg */
    volatile unsigned char	RSRVD43[0x2];	
    volatile unsigned short	pip_pbodr;	/* port b open drain reg */
    volatile unsigned long	pip_pbdat;	/* port b data reg */
    volatile unsigned char	RSRVD44[0x18];	

    /* Serial Interface */
    volatile unsigned long	si_simode;	/* SI mode register */
    volatile unsigned char	si_sigmr;	/* SI global mode register */
    volatile unsigned char	RSRVD45; 
    volatile unsigned char	si_sistr;	/* SI status register */
    volatile unsigned char	si_sicmr;	/* SI command register */
    volatile unsigned char	RSRVD46[0x4]; 
    volatile unsigned long	si_sicr;	/* SI clock routing */
    volatile unsigned long	si_sirp;	/* SI ram pointers */
    volatile unsigned char	RSRVD47[0x10c]; 
    volatile unsigned char	si_siram[0x200];/* SI routing ram */
    volatile unsigned short	lcd_lcolr[256];	/* LCD Color RAM -- REV A.x */
    volatile unsigned char	RSRVD48[0x1000]; 

    /* BASE + 0x2000: user data memory */
    volatile unsigned char	udata_ucode[0x800];	/* user data bd's Ucode*/
    volatile unsigned char      bd[0x700];              /* buffer descriptors, data */
    volatile unsigned char      udata_ext[0x100];       /* extension area for downloaded ucode */
    volatile unsigned char	RSRVD49[0x0C00];	
    
    /* BASE + 0x3c00: PARAMETER RAM */
    union {
	struct scc_pram {
	    union {
		struct hdlc_pram	h;
		struct uart_pram	u;
		struct bisync_pram	b;
		struct transparent_pram	t;
		struct async_hdlc_pram	a;
		unsigned char		RSRVD50[0x80];
	    } pscc;		/* scc parameter area (protocol dependent) */

	    union {
		struct {
		    struct i2c_pram	i2c;
		    unsigned char	RSRVD56[0x10];
		    struct idma_pram	idma1;
		} i2c_idma;
		struct {
		    struct spi_pram	spi;
		    struct timer_pram	timer;
		    struct idma_pram	idma2;
		} spi_timer_idma;
		struct {
		    union {
			struct smc_uart_pram	u;
			struct smc_trnsp_pram	t;
			struct centronics_pram	c;
		    } psmc;
		    unsigned char	modem_param[0x40];
		} smc_modem;
		struct {
		    unsigned char	RSRVD54[0x40];
		    struct ucode_pram	ucode;
		} pucode;
	    } pothers;
	} scc;
	struct ethernet_pram	enet_scc;
	unsigned char		pr[0x100];
    } pram[4];
} EPPC;


static inline EPPC *eppc_base(void)
{
    EPPC    *retval;

    asm volatile (
        "mfspr   %0,638 \n\t"
	"andis.	 %0,%0,65535 \n\t"
	: "=r" (retval)
	: /* no inputs */  );

    return retval;
}

// Function used to reset [only once!] the CPM
__externC void _mpc8xx_reset_cpm(void);

// Function used to allocate space in shared memory area
// typically used for buffer descriptors, etc.
__externC unsigned short _mpc8xx_allocBd(int len);

// Function used to manage the pool of baud rate generators
__externC unsigned long *_mpc8xx_allocate_brg(int port);

#define QUICC_BD_BASE               0x2000  // Start of shared memory
#define QUICC_BD_END                0x3000  // End of shared memory


#endif /* __ASSEMBLER__ */

/* Memory Periodic Timer Prescaler Register values */
#define PTP_DIV2	0x2000
#define PTP_DIV4	0x1000
#define PTP_DIV8	0x0800
#define PTP_DIV16	0x0400
#define PTP_DIV32	0x0200
#define PTP_DIV64	0x0100

// Command Processor Module (CPM) 

// Buffer descriptor control bits
#define QUICC_BD_CTL_Ready          0x8000  // Buffer contains data (tx) or is empty (rx)
#define QUICC_BD_CTL_Wrap           0x2000  // Last buffer in list
#define QUICC_BD_CTL_Int            0x1000  // Generate interrupt when empty (tx) or full (rx)
#define QUICC_BD_CTL_Last           0x0800  // Last buffer in a sequence
#define QUICC_BD_CTL_MASK           0xB000  // User settable bits

// Command register
#define QUICC_CPM_CR_INIT_TXRX      0x0000  // Initialize both Tx and Rx chains
#define QUICC_CPM_CR_INIT_RX        0x0100  // Initialize Rx chains
#define QUICC_CPM_CR_INIT_TX        0x0200  // Initialize Tx chains
#define QUICC_CPM_CR_HUNT_MODE      0x0300  // Start "hunt" mode
#define QUICC_CPM_CR_STOP_TX        0x0400  // Stop transmitter
#define QUICC_CPM_CR_RESTART_TX     0x0600  // Restart transmitter
#define QUICC_CPM_CR_RESET          0x8000  // Reset CPM
#define QUICC_CPM_CR_BUSY           0x0001  // Kick CPM - busy indicator

// CPM channels
#define QUICC_CPM_SCC1              0x0000
#define QUICC_CPM_I2C               0x0010
#define QUICC_CPM_SCC2              0x0040
#define QUICC_CPM_SCC3              0x0080
#define QUICC_CPM_SMC1              0x0090
#define QUICC_CPM_SCC4              0x00C0
#define QUICC_CPM_SMC2              0x00D0

// SMC Events (interrupts)
#define QUICC_SMCE_BRK              0x10  // Break received
#define QUICC_SMCE_BSY              0x04  // Busy - receive buffer overrun
#define QUICC_SMCE_TX               0x02  // Tx interrupt
#define QUICC_SMCE_RX               0x01  // Rx interrupt

// SMC Mode Register
#define QUICC_SMCMR_CLEN(n)   ((n+1)<<11)   // Character length
#define QUICC_SMCMR_SB(n)     ((n-1)<<10)   // Stop bits (1 or 2)
#define QUICC_SMCMR_PE(n)     (n<<9)        // Parity enable (0=disable, 1=enable)
#define QUICC_SMCMR_PM(n)     (n<<8)        // Parity mode (0=odd, 1=even)
#define QUICC_SMCMR_UART      (2<<4)        // UART mode
#define QUICC_SMCMR_TEN       (1<<1)        // Enable transmitter
#define QUICC_SMCMR_REN       (1<<0)        // Enable receiver

// SMC Commands
#define QUICC_SMC_CMD_InitTxRx  (0<<8)
#define QUICC_SMC_CMD_InitTx    (1<<8)
#define QUICC_SMC_CMD_InitRx    (2<<8)
#define QUICC_SMC_CMD_StopTx    (4<<8)
#define QUICC_SMC_CMD_RestartTx (6<<8)
#define QUICC_SMC_CMD_Reset     0x8000
#define QUICC_SMC_CMD_Go        0x0001

// SCC PSMR masks ....
#define QUICC_SCC_PSMR_ASYNC   0x8000
#define QUICC_SCC_PSMR_SB(n)   ((n-1)<<14)  // Stop bits (1=1sb, 2=2sb)
#define QUICC_SCC_PSMR_CLEN(n) ((n-5)<<12)  // Character Length (5-8)
#define QUICC_SCC_PSMR_PE(n)   (n<<4)       // Parity enable(0=disabled, 1=enabled)
#define QUICC_SCC_PSMR_RPM(n)  (n<<2)       // Rx Parity mode (0=odd,  1=low, 2=even, 3=high)
#define QUICC_SCC_PSMR_TPM(n)  (n)          // Tx Parity mode (0=odd,  1=low, 2=even, 3=high)

// SCC DSR masks
#define QUICC_SCC_DSR_FULL     0x7e7e
#define QUICC_SCC_DSR_HALF     0x467e

// SCC GSMR masks ...
#define QUICC_SCC_GSMR_H_INIT  0x00000060 
#define QUICC_SCC_GSMR_L_INIT  0x00028004 
#define QUICC_SCC_GSMR_L_Tx    0x00000010
#define QUICC_SCC_GSMR_L_Rx    0x00000020

// SCC Events (interrupts)
#define QUICC_SCCE_BRK         0x0040
#define QUICC_SCCE_BSY         0x0004
#define QUICC_SCCE_TX          0x0002
#define QUICC_SCCE_RX          0x0001

#endif // ifndef CYGONCE_HAL_PPC_QUICC_PPC8XX_H
