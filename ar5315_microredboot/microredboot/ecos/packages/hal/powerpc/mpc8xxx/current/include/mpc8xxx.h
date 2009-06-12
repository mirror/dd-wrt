#ifndef CYGONCE_HAL_PPC_QUICC2_MPC8260_H
#define CYGONCE_HAL_PPC_QUICC2_MPC8260_H

//==========================================================================
//
//      mpc8260.h
//
//      PowerPC QUICC2 register definitions
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
// Contributors: hmt, gthomas
// Date:         1999-06-08
// Purpose:      PowerPC QUICC2 definitions
// Description:  PowerPC QUICC2 definitions
// Usage:        THIS IS NOT AN EXTERNAL API
//               This file is in the include dir to share it between
//               QUICCII serial code and MPC8260 initialization code.
//               #include <cyg/hal/mpc8260.h>
//               ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/hal/plf_regs.h>           // For IMM base

#define DPRAM_SMC1_OFFSET 0x2000
#define DPRAM_SMC2_OFFSET 0x2100
#define DPRAM_BD_OFFSET   0x2200

#define _Packed 
#define _PackedType __attribute__((packed))

/******************************************************************************
*
*  PARAMETER RAM (PRAM) FOR EACH PERIPHERAL
*  
*  Each subsection contains protocol-specific PRAM for each peripheral,
*  followed by the PRAM common to all protocols for that peripheral.  These 
*  structs are used as needed in the main MPC8260 memory map structure.  Note 
*  that different modes of operation will require the use of different PRAM 
*  structs, and that certain structs may overlay and conflict with the use of 
*  other PRAM areas.  Consult the MPC8260 User Manual for details as to what
*  is unavailable when certain protocols are run on certain peripherals.
*
******************************************************************************/
                          


/*---------------------------------------------------------------------------*/
/*                   SERIAL COMMUNICATION CONTROLLER (SCC)                 */
/*---------------------------------------------------------------------------*/

/*----------*/
/* SCC HDLC */
/*----------*/

typedef _Packed struct 
{
    CYG_BYTE    reserved1[4]; /* Reserved area */
    CYG_WORD    c_mask;       /* CRC constant */
    CYG_WORD    c_pres;       /* CRC preset */
    CYG_WORD16   disfc;        /* discarded frame counter */
    CYG_WORD16   crcec;        /* CRC error counter */
    CYG_WORD16   abtsc;        /* abort sequence counter */
    CYG_WORD16   nmarc;        /* nonmatching address rx cnt */
    CYG_WORD16   retrc;        /* frame transmission counter. */
                           /* For FCC this area is reserved.*/
    CYG_WORD16   mflr;         /* maximum frame length reg */
    CYG_WORD16   max_cnt;      /* maximum length counter */
    CYG_WORD16   rfthr;        /* received frames threshold */
    CYG_WORD16   rfcnt;        /* received frames count */
    CYG_WORD16   hmask;        /* user defined frm addr mask */
    CYG_WORD16   haddr1;       /* user defined frm address 1 */
    CYG_WORD16   haddr2;       /* user defined frm address 2 */
    CYG_WORD16   haddr3;       /* user defined frm address 3 */
    CYG_WORD16   haddr4;       /* user defined frm address 4 */
    CYG_WORD16   tmp;          /* temp */
    CYG_WORD16   tmp_mb;       /* temp */
} _PackedType t_HdlcScc_Pram;

 
/*--------------*/
/* SCC Ethernet */
/*--------------*/

typedef _Packed struct 
{
    CYG_WORD    c_pres;      /* CRC preset */
    CYG_WORD    c_mask;      /* CRC constant mask*/
    CYG_WORD    crcec;       /* CRC error counter */
    CYG_WORD    alec;        /* alignment error counter */
    CYG_WORD    disfc;       /* discarded frame counter */
    CYG_WORD16   pads;        /* Short frame pad character. */
    CYG_WORD16   ret_lim;     /* Retry limit threshold. */
    CYG_WORD16   ret_cnt;     /* Retry limit counter. */
    CYG_WORD16   mflr;        /* maximum frame length reg */
    CYG_WORD16   minflr;      /* minimum frame length reg */
    CYG_WORD16   maxd1;       /* max DMA1 length register. */
    CYG_WORD16   maxd2;       /* max DMA2 length register. */
    CYG_WORD16   maxd;        /* Rx max DMA. */
    CYG_WORD16   dma_cnt;     /* Rx DMA counter. */
    CYG_WORD16   max_b;       /* max buffer descriptor byte count. */
    CYG_WORD16   gaddr1;      /* group address filter */
    CYG_WORD16   gaddr2;      /* group address filter */
    CYG_WORD16   gaddr3;      /* group address filter */
    CYG_WORD16   gaddr4;      /* group address filter */
    CYG_WORD    tbuf0_data0; /* Saved area 0, current frame. */
    CYG_WORD    tbuf0_data1; /* Saved area 1, current frame. */
    CYG_WORD    tbuf0_rba0;
    CYG_WORD    tbuf0_crc;
    CYG_WORD16   tbuf0_bcnt;
    CYG_WORD16   paddr1_h;    /* physical address (MSB) */
    CYG_WORD16   paddr1_m;    /* physical address */
    CYG_WORD16   paddr1_l;    /* physical address (LSB) */
    CYG_WORD16   p_per;       /* persistence */
    CYG_WORD16   rfbd_ptr;    /* Rx first BD pointer. */
    CYG_WORD16   tfbd_ptr;    /* Tx first BD pointer. */
    CYG_WORD16   tlbd_ptr;    /* Tx last BD pointer. */
    CYG_WORD    tbuf1_data0; /* Saved area 0, next frame. */
    CYG_WORD    tbuf1_data1; /* Saved area 1, next frame. */
    CYG_WORD    tbuf1_rba0;
    CYG_WORD    tbuf1_crc;
    CYG_WORD16   tbuf1_bcnt;
    CYG_WORD16   tx_len;      /* tx frame length counter */
    CYG_WORD16   iaddr1;      /* individual address filter. */
    CYG_WORD16   iaddr2;      /* individual address filter.  */
    CYG_WORD16   iaddr3;      /* individual address filter. */
    CYG_WORD16   iaddr4;      /* individual address filter.  */
    CYG_WORD16   boff_cnt;    /* back-off counter */
    CYG_WORD16   taddr_h;     /* temp address (MSB) */
    CYG_WORD16   taddr_m;     /* temp address */
    CYG_WORD16   taddr_l;     /* temp address (LSB) */
} _PackedType t_EnetScc_Pram;

/*----------*/
/* SCC UART */
/*----------*/

typedef _Packed struct 
{
    CYG_BYTE       reserved1[8];   /* Reserved area */
    CYG_WORD16      max_idl;        /* maximum idle characters */
    CYG_WORD16      idlc;           /* rx idle counter (internal) */
    CYG_WORD16      brkcr;          /* break count register */
    CYG_WORD16      parec;          /* Rx parity error counter */
    CYG_WORD16      frmec;          /* Rx framing error counter */
    CYG_WORD16      nosec;          /* Rx noise counter */
    CYG_WORD16      brkec;          /* Rx break character counter */
    CYG_WORD16      brkln;          /* Receive break length */
    CYG_WORD16      uaddr1;         /* address character 1 */
    CYG_WORD16      uaddr2;         /* address character 2 */
    CYG_WORD16      rtemp;          /* temp storage */
    CYG_WORD16      toseq;          /* Tx out of sequence char */
    CYG_WORD16      cc[8];          /* Rx control characters */
    CYG_WORD16      rccm;           /* Rx control char mask */
    CYG_WORD16      rccr;           /* Rx control char register */
    CYG_WORD16      rlbc;           /* Receive last break char */
} _PackedType t_UartScc_Pram;


/*-----------------*/
/* SCC Transparent */
/*-----------------*/

typedef _Packed struct  
{
    CYG_WORD    c_mask;     /* CRC constant */
    CYG_WORD    c_pres;     /* CRC preset */
} _PackedType t_TransScc_Pram;


/*------------*/
/* SCC Bisync */
/*------------*/

typedef _Packed struct  
{
   CYG_BYTE    reserved1[4];     /* Reserved area */
   CYG_WORD crcc;       /* CRC Constant Temp Value */
   CYG_WORD16   prcrc;         /* Preset Receiver CRC-16/LRC */
   CYG_WORD16   ptcrc;         /* Preset Transmitter CRC-16/LRC */
   CYG_WORD16   parec;         /* Receive Parity Error Counter */
   CYG_WORD16   bsync;         /* BISYNC SYNC Character */
   CYG_WORD16   bdle;       /* BISYNC DLE Character */
   CYG_WORD16   cc[8];         /* Rx control characters */
   CYG_WORD16   rccm;       /* Receive Control Character Mask */
} _PackedType t_BisyncScc_Pram;


/*-----------------*/
/* SCC Common PRAM */
/*-----------------*/

typedef _Packed struct 
{
    CYG_WORD16   rbase;      /* RX BD base address */
    CYG_WORD16   tbase;      /* TX BD base address */
    CYG_BYTE    rfcr;       /* Rx function code */
    CYG_BYTE    tfcr;       /* Tx function code */
    CYG_WORD16   mrblr;      /* Rx buffer length */
    CYG_WORD    rstate;     /* Rx internal state */
    CYG_WORD    rptr;       /* Rx internal data pointer */
    CYG_WORD16   rbptr;      /* rb BD Pointer */
    CYG_WORD16   rcount;     /* Rx internal byte count */
    CYG_WORD    rtemp;      /* Rx temp */
    CYG_WORD    tstate;     /* Tx internal state */
    CYG_WORD    tptr;       /* Tx internal data pointer */
    CYG_WORD16   tbptr;      /* Tx BD pointer */
    CYG_WORD16   tcount;     /* Tx byte count */
    CYG_WORD    ttemp;      /* Tx temp */
    CYG_WORD    rcrc;       /* temp receive CRC */
    CYG_WORD    tcrc;       /* temp transmit CRC */
    union 
    {
       t_HdlcScc_Pram    h;
       t_EnetScc_Pram    e;
       t_UartScc_Pram    u;
       t_TransScc_Pram   t;
       t_BisyncScc_Pram  b;
    } SpecificProtocol;
    volatile CYG_BYTE COMPLETE_SIZE_OF_DPRAM_PAGE[0x5c];
} _PackedType t_Scc_Pram;



/*---------------------------------------------------------------------------*/
/*                     FAST COMMUNICATION CONTROLLER (FCC)               */
/*---------------------------------------------------------------------------*/

/*----------*/
/* FCC HDLC */
/*----------*/

typedef _Packed struct 
{
    CYG_BYTE    reserved1[8]; /* Reserved area */
    CYG_WORD    c_mask;       /* CRC constant */
    CYG_WORD    c_pres;       /* CRC preset */
    CYG_WORD16   disfc;        /* discarded frame counter */
    CYG_WORD16   crcec;        /* CRC error counter */
    CYG_WORD16   abtsc;        /* abort sequence counter */
    CYG_WORD16   nmarc;        /* nonmatching address rx cnt */
    CYG_WORD    max_cnt;      /* maximum length counter */
    CYG_WORD16   mflr;         /* maximum frame length reg */
    CYG_WORD16   rfthr;        /* received frames threshold */
    CYG_WORD16   rfcnt;        /* received frames count */
    CYG_WORD16   hmask;        /* user defined frm addr mask */
    CYG_WORD16   haddr1;       /* user defined frm address 1 */
    CYG_WORD16   haddr2;       /* user defined frm address 2 */
    CYG_WORD16   haddr3;       /* user defined frm address 3 */
    CYG_WORD16   haddr4;       /* user defined frm address 4 */
    CYG_WORD16   tmp;          /* temp */
    CYG_WORD16   tmp_mb;       /* temp */
} _PackedType t_HdlcFcc_Pram;


/*--------------*/
/* FCC Ethernet */
/*--------------*/

typedef _Packed struct 
{
    CYG_WORD    stat_bus;       /* Internal use buffer. */
    CYG_WORD    cam_ptr;        /* CAM address. */
    CYG_WORD    c_mask;         /* CRC constant mask*/
    CYG_WORD    c_pres;      /* CRC preset */
    CYG_WORD    crcec;          /* CRC error counter */
    CYG_WORD    alec;           /* alignment error counter */
    CYG_WORD    disfc;          /* discarded frame counter */
    CYG_WORD16   ret_lim;        /* Retry limit threshold. */
    CYG_WORD16   ret_cnt;        /* Retry limit counter. */
    CYG_WORD16   p_per;          /* persistence */
    CYG_WORD16   boff_cnt;       /* back-off counter */
    CYG_WORD    gaddr_h;        /* group address filter, high */
    CYG_WORD    gaddr_l;        /* group address filter, low */
    CYG_WORD16   tfcstat;        /* out of sequece Tx BD staus. */
    CYG_WORD16   tfclen;         /* out of sequece Tx BD length. */
    CYG_WORD    tfcptr;         /* out of sequece Tx BD data pointer. */
    CYG_WORD16   mflr;           /* maximum frame length reg */
    CYG_WORD16   paddr1_h;       /* physical address (MSB) */
    CYG_WORD16   paddr1_m;       /* physical address */
    CYG_WORD16   paddr1_l;       /* physical address (LSB) */
    CYG_WORD16   ibd_cnt;        /* internal BD counter. */
    CYG_WORD16   ibd_start;      /* internal BD start pointer. */
    CYG_WORD16   ibd_end;        /* internal BD end pointer. */
    CYG_WORD16   tx_len;         /* tx frame length counter */
    CYG_BYTE    ibd_base[0x20]; /* internal micro code usage. */
    CYG_WORD    iaddr_h;        /* individual address filter, high */
    CYG_WORD    iaddr_l;        /* individual address filter, low */
    CYG_WORD16   minflr;         /* minimum frame length reg */
    CYG_WORD16   taddr_h;        /* temp address (MSB) */
    CYG_WORD16   taddr_m;        /* temp address */
    CYG_WORD16   taddr_l;        /* temp address (LSB) */
    CYG_WORD16   pad_ptr;        /* pad_ptr. */
    CYG_WORD16   cf_type;        /* flow control frame type coding. */
    CYG_WORD16   cf_range;       /* flow control frame range. */
    CYG_WORD16   max_b;          /* max buffer descriptor byte count. */
    CYG_WORD16   maxd1;          /* max DMA1 length register. */
    CYG_WORD16   maxd2;          /* max DMA2 length register. */
    CYG_WORD16   maxd;           /* Rx max DMA. */
    CYG_WORD16   dma_cnt;        /* Rx DMA counter. */
    
    /* counter: */
    CYG_WORD    octc;           /* received octets counter. */
    CYG_WORD    colc;           /* estimated number of collisions */
    CYG_WORD    broc;           /* received good packets of broadcast address */
    CYG_WORD    mulc;           /* received good packets of multicast address */
    CYG_WORD    uspc;           /* received packets shorter then 64 octets. */
    CYG_WORD    frgc;           /* as uspc + bad packets */
    CYG_WORD    ospc;           /* received packets longer then 1518 octets. */
    CYG_WORD    jbrc;           /* as ospc + bad packets  */
    CYG_WORD    p64c;           /* received packets of 64 octets.. */
    CYG_WORD    p65c;           /* received packets of 65-128 octets.. */
    CYG_WORD    p128c;          /* received packets of 128-255 octets.. */
    CYG_WORD    p256c;          /* received packets of 256-511 octets.. */
    CYG_WORD    p512c;          /* received packets of 512-1023 octets.. */
    CYG_WORD    p1024c;         /* received packets of 1024-1518 octets.. */
    CYG_WORD    cam_buf;        /* cam respond internal buffer. */
    CYG_WORD16   rfthr;          /* received frames threshold */
    CYG_WORD16   rfcnt;          /* received frames count */
} _PackedType t_EnetFcc_Pram;


/*-----------------*/
/* FCC Common PRAM */
/*-----------------*/

typedef _Packed struct 
{
    CYG_WORD16   riptr;      /* Rx internal temporary data pointer. */
    CYG_WORD16   tiptr;      /* Tx internal temporary data pointer. */
    CYG_WORD16   reserved0;  /* Reserved */
    CYG_WORD16   mrblr;      /* Rx buffer length */
    CYG_WORD    rstate;     /* Rx internal state */
    CYG_WORD    rbase;      /* RX BD base address */
    CYG_WORD16   rbdstat;    /* Rx BD status and control */
    CYG_WORD16   rbdlen;     /* Rx BD data length */
    CYG_WORD    rdptr;      /* rx BD data pointer */
    CYG_WORD    tstate;     /* Tx internal state */
    CYG_WORD    tbase;      /* TX BD base address */
    CYG_WORD16   tbdstat;    /* Tx BD status and control */
    CYG_WORD16   tbdlen;     /* Tx BD data length */
    CYG_WORD    tdptr;      /* Tx  data pointer */
    CYG_WORD    rbptr;      /* rx BD pointer */
    CYG_WORD    tbptr;      /* Tx BD pointer */
    CYG_WORD    rcrc;       /* Temp receive CRC */
    CYG_WORD    reserved_1[0x1];
    CYG_WORD    tcrc;       /* Temp transmit CRC */
    union                /* Protocol-Specific parameter ram */
   {
        t_HdlcFcc_Pram    h;
       t_EnetFcc_Pram    e;
    } SpecificProtocol;      
} _PackedType t_Fcc_Pram;



/*---------------------------------------------------------------------------*/
/*                 MULTICHANNEL COMMUNICATION CONTROLLER (MCC)              */
/*---------------------------------------------------------------------------*/

/******************************************************************************
* Note that each MCC uses multiple logical channels.  We first define the     *
* PRAM for a logical channel (which can be used in either HDLC or Transparent *
* mode;  wherever there are differences, it is specified), followed by the    *
* PRAM for an MCC itself.                                                     *
******************************************************************************/

/*---------------------*/
/* MCC Logical Channel */
/*---------------------*/

typedef _Packed struct
{
    CYG_WORD    tstate;     /* Tx internal state. */ 
    CYG_WORD    zistate;    /* Zero insertion machine state. */
    CYG_WORD    zidata0;    /* Zero insertion high CYG_WORD16. */
    CYG_WORD    zidata1;    /* Zero insertion low CYG_WORD16. */
    CYG_WORD16   tbdflags;   /* Tx internal BD flags. */
    CYG_WORD16   tbdcnt;     /* Tx internal byte count . */
    CYG_WORD    tbdptr;     /* Tx internal data pointer. */
    CYG_WORD16   intmask;    /* Interrupt mask flags. */
    CYG_WORD16   chamr;      /* channel mode register. */
    CYG_WORD    tcrc;       /* Transparent: reserved. */
                         /* Hdlc: Temp receive CRC.*/
    CYG_WORD    rstate;     /* Rx internal state. */ 
    CYG_WORD    zdstate;    /* Zero deletion machine state. */
    CYG_WORD    zddata0;    /* Zero deletion  high CYG_WORD16. */
    CYG_WORD    zddata1;    /* Zero deletion  low CYG_WORD16. */
    CYG_WORD16   rbdflags;   /* Rx internal BD flags. */
    CYG_WORD16   rbdcnt;     /* Rx internal byte count . */
    CYG_WORD    rbdptr;     /* Rx internal data pointer. */
    CYG_WORD16   maxrlen;    /* Transparent: Max receive buffer length. */
                         /* Hdlc: Max receive frame length. */
    CYG_WORD16   sync_maxcnt;/* Transparent: Receive synchronization pattern*/
                         /* Hdlc: Max length counter. */
    CYG_WORD    rcrc;       /* Transparent: reserved. */
                         /* Hdlc: Temp receive CRC.*/
} _PackedType t_Mch_Pram;


/*----------*/
/* MCC PRAM */
/*----------*/

typedef _Packed struct
{
    CYG_WORD     mccbase;    /* A pointer to starting address of BD rings. */
    CYG_WORD16   mccstate;   /* Controller state. */
    CYG_WORD16   mrblr;      /* Maximum receive buffer length. */
    CYG_WORD16   grfthr;     /* Global receive frame threshold. */
    CYG_WORD16   grfcnt;     /* Global receive frame counter. */
    CYG_WORD     rinttmp;    /* Temp location for receive interrupt table entry. */                            
    CYG_WORD     data0;      /* Temporary location for holding data. */
    CYG_WORD     data1;      /* Temporary location for holding data. */
    CYG_WORD     tintbase;   /* Transmit interrupt table base address. */
    CYG_WORD     tintptr;    /* Transmit interrupt table pointer. */ 
    CYG_WORD     tinttmp;    /* Temp location for receive interrupt table entry. */
    CYG_WORD16   sctpbase;   /* A pointer to the super channel transmit table*/
    CYG_BYTE     res0[0x2];  /* Reserved area */
    CYG_WORD     c_mask32;   /* CRC constant. */
    CYG_WORD16   xtrabase;   /* A pointer to the beginning of extra parameters */                            
    CYG_WORD16   c_mask16;   /* CRC constant. */
    CYG_WORD     rinttmp0;   /* Temp location for receive interrupt table entry. */                            
    CYG_WORD     rinttmp1;   /* Temp location for receive interrupt table entry. */
    CYG_WORD     rinttmp2;   /* Temp location for receive interrupt table entry. */
    CYG_WORD     rinttmp3;   /* Temp location for receive interrupt table entry. */
    CYG_WORD     rintbase0;  /* Receive interrupt table base address. */
    CYG_WORD     rintptr0;   /* Receive interrupt table pointer. */
    CYG_WORD     rintbase1;  /* Receive interrupt table base address. */
    CYG_WORD     rintptr1;   /* Receive interrupt table pointer. */
    CYG_WORD     rintbase2;  /* Receive interrupt table base address. */
    CYG_WORD     rintptr2;   /* Receive interrupt table pointer. */
    CYG_WORD     rintbase3;  /* Receive interrupt table base address. */
    CYG_WORD     rintptr3;   /* Receive interrupt table pointer. */
    CYG_BYTE     pad[0xa0];
} _PackedType t_Mcc_Pram;



/*---------------------------------------------------------------------------*/
/*                              ATM PARAMETER RAM                            */
/*---------------------------------------------------------------------------*/


/*--------------------------------------*/
/* Address Compression parameters table */
/*--------------------------------------*/

_Packed struct AddressCompressionPram 
{
    volatile CYG_WORD  VptBase;  /* VP-level addressing table base address */
    volatile CYG_WORD  VctBase;  /* VC-level addressing table base address */
    volatile CYG_WORD  Vpt1Base; /* VP1-level addressing table base address */  
    volatile CYG_WORD  Vct1Base; /* VC1-level addressing table base address */
    volatile CYG_WORD16 VpMask;    /* VP mask for address compression look-up */
} _PackedType;


/*-------------------------------*/
/* External CAM parameters table */
/*-------------------------------*/

_Packed struct ExtCamPram 
{
    volatile CYG_WORD  ExtCamBase;    /* Base address of the external CAM */
    volatile CYG_BYTE  reserved00[4];  /* Reserved */
    volatile CYG_WORD  ExtCam1Base;   /* Base address of the external CAM1 */
    volatile CYG_BYTE  reserved01[6];  /* Reserved */
} _PackedType;


/*---------------------------*/
/* ATM mode parameters table */
/*---------------------------*/

typedef _Packed struct AtmPram 
{
    volatile CYG_BYTE  reserved0[64];    /* Reserved */
    volatile CYG_WORD16 RxCellTmpBase;    /* Rx cell temporary base address */
    volatile CYG_WORD16 TxCellTmpBase;    /* Tx cell temporary base address */
    volatile CYG_WORD16 UdcTmpBase;       /* UDC temp base address (in UDC mode only) */
    volatile CYG_WORD16 IntRctBase;       /* Internal RTC base address */
    volatile CYG_WORD16 IntTctBase;       /* Internal TCT base address */
    volatile CYG_WORD16 IntTcteBase;      /* Internal ACT base address */
    volatile CYG_BYTE  reserved1[4];     /* reserved four bytes */
    volatile CYG_WORD  ExtRctBase;       /* Extrnal RTC base address */
    volatile CYG_WORD  ExtTctBase;       /* Extrnal TCT base address */
    volatile CYG_WORD  ExtTcteBase;      /* Extrnal ACT base address */
    volatile CYG_WORD16 UeadOffset;       /* The offset in half-wordunits of the UEAD
                                 entry in the UDC extra header. Should be
                                 even address. If little-endian format is
                                 used, the UeadOffset is of the little-endian
                                 format. */
    volatile CYG_BYTE  reserved2[2];     /* Reserved */
    volatile CYG_WORD16 PmtBase;      /* Performance monitoring table base address */
    volatile CYG_WORD16 ApcParamBase;    /* APC Parameters table base address */
    volatile CYG_WORD16 FbpParamBase;    /* Free buffer pool parameters base address */
    volatile CYG_WORD16 IntQParamBase;   /* Interrupt queue parameters table base */
    volatile CYG_BYTE  reserved3[2];
    volatile CYG_WORD16 UniStatTableBase; /* UNI statistics table base */
    volatile CYG_WORD  BdBaseExt;        /* BD ring base address extension */
    union 
    {
       struct AddressCompressionPram   AddrCompression;
       struct ExtCamPram               ExtCam;
    } AddrMapping;            /* Address look-up mechanism */
    volatile CYG_WORD16 VciFiltering;     /* VCI filtering enable bits. If bit i is set,
                                 the cell with VCI=i will be sent to the
                                 raw cell queue. The bits 0-2 and 5 should
                                 be zero. */
    volatile CYG_WORD16 Gmode;            /* Global mode */
    volatile CYG_WORD16 CommInfo1;        /* The information field associated with the */
    volatile CYG_WORD  CommInfo2;        /* last host command */
    volatile CYG_BYTE  reserved4[4];     /* Reserved */
    volatile CYG_WORD  CRC32Preset;      /* Preset for CRC32 */
    volatile CYG_WORD  CRC32Mask;        /* Constant mask for CRC32 */
    volatile CYG_WORD16 AAL1SnpTableBase; /* AAl1 SNP protection look-up table base */
    volatile CYG_WORD16 reserved5;        /* Reserved */
    volatile CYG_WORD  SrtsBase;         /* External SRTS logic base address. For AAL1
                                 only. Should be 16 bytes aligned */
    volatile CYG_WORD16 IdleBase;         /* Idle cell base address */
    volatile CYG_WORD16 IdleSize;         /* Idle cell size: 52, 56, 60, 64 */
    volatile CYG_WORD  EmptyCellPayload; /* Empty cell payload (little-indian) */
    
    /* ABR specific only */
    volatile CYG_WORD  Trm; /* Upper bound on time between F-RM cells for active source */                                 
    volatile CYG_WORD16 Nrm; /* Controls the maximum data cells sent for each F-RM cell. */                           
    volatile CYG_WORD16 Mrm; /* Controls bandwidth between F-RM, B-RM and user data cell */
    volatile CYG_WORD16 Tcr;              /* Tag cell rate */
    volatile CYG_WORD16 AbrRxTcte;        /* ABR reserved area address (2-CYG_WORD16 aligned)*/
    volatile CYG_BYTE  reserved7[76];    /* Reserved */
} _PackedType t_Atm_Pram;



/*---------------------------------------------------------------------------*/
/*                     SERIAL MANAGEMENT CHANNEL  (SMC)                   */
/*---------------------------------------------------------------------------*/

typedef _Packed struct  
{
   CYG_WORD16   rbase;      /* Rx BD Base Address */
   CYG_WORD16   tbase;      /* Tx BD Base Address */
   CYG_BYTE    rfcr;    /* Rx function code */
   CYG_BYTE    tfcr;    /* Tx function code */
   CYG_WORD16   mrblr;      /* Rx buffer length */
   CYG_WORD    rstate;     /* Rx internal state */
   CYG_WORD    rptr;    /* Rx internal data pointer */
   CYG_WORD16   rbptr;      /* rb BD Pointer */
   CYG_WORD16   rcount;     /* Rx internal byte count */
   CYG_WORD    rtemp;      /* Rx temp */
   CYG_WORD    tstate;     /* Tx internal state */
   CYG_WORD    tptr;    /* Tx internal data pointer */
   CYG_WORD16   tbptr;      /* Tx BD pointer */
   CYG_WORD16   tcount;     /* Tx byte count */
   CYG_WORD    ttemp;       /* Tx temp */
  
   /* SMC UART-specific PRAM */
   CYG_WORD16   max_idl; /* Maximum IDLE Characters */
   CYG_WORD16   idlc;    /* Temporary IDLE Counter */
   CYG_WORD16   brkln;      /* Last Rx Break Length */
   CYG_WORD16   brkec;      /* Rx Break Condition Counter */
   CYG_WORD16   brkcr;      /* Break Count Register (Tx) */
   CYG_WORD16   r_mask;     /* Temporary bit mask */
  
} _PackedType t_Smc_Pram;



/*---------------------------------------------------------------------------*/
/*                      IDMA PARAMETER RAM                             */
/*---------------------------------------------------------------------------*/

typedef _Packed struct  
{
    CYG_WORD16      ibase;       /* IDMA BD Base Address               */
    CYG_WORD16      dcm;         /* DMA channel mode register          */
    CYG_WORD16      ibdptr;      /* next bd ptr                        */
    CYG_WORD16      DPR_buf;     /* ptr to internal 64 byte buffer     */
    CYG_WORD16      BUF_inv;     /* The quantity of data in DPR_buf    */
    CYG_WORD16      SS_max;      /* Steady State Max. transfer size    */
    CYG_WORD16      DPR_in_ptr;  /* write ptr for the internal buffer  */
    CYG_WORD16      sts;         /* Source Transfer Size               */
    CYG_WORD16      DPR_out_ptr; /* read ptr for the internal buffer   */
    CYG_WORD16      seob;        /* Source end of burst                */
    CYG_WORD16      deob;        /* Destination end of burst           */
    CYG_WORD16      dts;         /* Destination Transfer Size          */
    CYG_WORD16      RetAdd;      /* return address when ERM==1         */
    CYG_WORD16      Reserved;    /* reserved */
    CYG_WORD       BD_cnt;      /* Internal byte count                */
    CYG_WORD       S_ptr;       /* source internal data ptr           */
    CYG_WORD       D_ptr;       /* destination internal data ptr      */
    CYG_WORD       istate;      /* Internal state                     */

} _PackedType t_Idma_Pram;



/*-------------------------------------------------------------------*/
/*         INTER-INTEGRATED CIRCUIT  (I2C)                        */
/*-------------------------------------------------------------------*/

typedef _Packed struct 
{
   CYG_WORD16   rbase;      /* RX BD base address */
   CYG_WORD16   tbase;      /* TX BD base address */
   CYG_BYTE    rfcr;    /* Rx function code */
   CYG_BYTE    tfcr;    /* Tx function code */
   CYG_WORD16   mrblr;      /* Rx buffer length */
   CYG_WORD    rstate;     /* Rx internal state */
   CYG_WORD    rptr;    /* Rx internal data pointer */
   CYG_WORD16   rbptr;      /* rb BD Pointer */
   CYG_WORD16   rcount;     /* Rx internal byte count */
   CYG_WORD    rtemp;      /* Rx temp */
   CYG_WORD    tstate;     /* Tx internal state */
   CYG_WORD    tptr;    /* Tx internal data pointer */
   CYG_WORD16   tbptr;      /* Tx BD pointer */
   CYG_WORD16   tcount;     /* Tx byte count */
   CYG_WORD    ttemp;      /* Tx temp */

} _PackedType t_I2c_Pram;



/*---------------------------------------------------------------------------*/
/*                       SERIAL PERIPHERAL INTERFACE  (SPI)                  */
/*---------------------------------------------------------------------------*/

typedef _Packed struct  
{
   CYG_WORD16   rbase;      /* Rx BD Base Address */
   CYG_WORD16   tbase;      /* Tx BD Base Address */
   CYG_BYTE    rfcr;       /* Rx function code */
   CYG_BYTE    tfcr;       /* Tx function code */
   CYG_WORD16   mrblr;      /* Rx buffer length */
   CYG_WORD    rstate;     /* Rx internal state */
   CYG_WORD    rptr;       /* Rx internal data pointer */
   CYG_WORD16   rbptr;      /* Rx BD Pointer */
   CYG_WORD16   rcount;     /* Rx internal byte count */
   CYG_WORD    rtemp;      /* Rx temp */
   CYG_WORD    tstate;     /* Tx internal state */
   CYG_WORD    tptr;       /* Tx internal data pointer */
   CYG_WORD16   tbptr;      /* Tx BD pointer */
   CYG_WORD16   tcount;     /* Tx byte count */
   CYG_WORD    ttemp;      /* Tx temp */
   CYG_BYTE    reserved[8];

} _PackedType t_Spi_Pram;



/*---------------------------------------------------------------------------*/
/*                RISC TIMER PARAMETER RAM                             */
/*---------------------------------------------------------------------------*/

typedef _Packed struct  
{
 
   CYG_WORD16   tm_base;    /* RISC timer table base adr */
   CYG_WORD16   tm_ptr;     /* RISC timer table pointer */
   CYG_WORD16   r_tmr;      /* RISC timer mode register */
   CYG_WORD16   r_tmv;      /* RISC timer valid register */
   CYG_WORD tm_cmd;     /* RISC timer cmd register */
   CYG_WORD tm_cnt;     /* RISC timer internal cnt */
} _PackedType t_timer_pram;



/*--------------------------------------------------------------------------*/
/*                  ROM MICROCODE PARAMETER RAM AREA                        */
/*--------------------------------------------------------------------------*/

typedef _Packed struct  
{
   CYG_WORD16   rev_num;    /* Ucode Revision Number */
   CYG_WORD16   d_ptr;      /* MISC Dump area pointer */
} _PackedType t_ucode_pram;

/*--------------------------------------------------------------------------*/
/*                MAIN DEFINITION OF MPC8260 INTERNAL MEMORY MAP            */
/*--------------------------------------------------------------------------*/

typedef _Packed struct 
{

/* cpm_ram */
    t_Mch_Pram    mch_pram[256]; /* MCC logical channels parameter ram */
    volatile CYG_BYTE reserved0[0x4000];    /* Reserved area */
    
/* DPR_BASE+0x8000*/
    union 
    {
    
    /*for access to the PRAM structs for SCCs, FCCs, and MCCs */ 
       struct serials 
       {
           t_Scc_Pram scc_pram[4];
           t_Fcc_Pram fcc_pram[3];
           t_Mcc_Pram mcc_pram[2];
           volatile CYG_BYTE reserved1[0x700];
       } serials;
        
    /* for access to ATM PRAM structs */
       struct atm
       {
           volatile CYG_BYTE reserved2[0x400];
           t_Atm_Pram atm_pram[2];
           volatile CYG_BYTE reserved3[0xa00];
       } atm;
        
    /* for access to the memory locations holding user-defined 
       base addresses of PRAM for SMCs, IDMA, SPI, and I2C. */     
       struct standard
       {
            volatile CYG_BYTE scc1[0x100];
            volatile CYG_BYTE scc2[0x100];
            volatile CYG_BYTE scc3[0x100];
            volatile CYG_BYTE scc4[0x100];
            volatile CYG_BYTE fcc1[0x100];
            volatile CYG_BYTE fcc2[0x100];
            volatile CYG_BYTE fcc3[0x100];
            volatile CYG_BYTE mcc1[0x80];
            volatile CYG_BYTE reserved_0[0x7c];
            volatile CYG_WORD16 smc1;  // Pointer to SMC1 DPRAM
            volatile CYG_BYTE idma1[0x2];
            volatile CYG_BYTE mcc2[0x80];
            volatile CYG_BYTE reserved_1[0x7c];
            volatile CYG_WORD16 smc2;  // Pointer to SMC2 DPRAM
            volatile CYG_BYTE idma2[0x2];
            volatile CYG_BYTE reserved_2[0xfc];
            volatile CYG_BYTE spi[0x2];
            volatile CYG_BYTE idma3[0x2];
            volatile CYG_BYTE reserved_3[0xe0];
            volatile CYG_BYTE timers[0x10];
            volatile CYG_BYTE Rev_num[0x2];
            volatile CYG_BYTE D_ptr[0x2];
            volatile CYG_BYTE reserved_4[0x4];
            volatile CYG_BYTE rand[0x4];
            volatile CYG_BYTE i2c[0x2];
            volatile CYG_BYTE idma4[0x2];
            volatile CYG_BYTE reserved_5[0x500];
       } standard;
        
    } pram;
    
    volatile CYG_BYTE reserved11[0x2000];      /* Reserved area */
    volatile CYG_BYTE cpm_ram_dpram_2[0x1000]; /* Internal RAM */
    volatile CYG_BYTE reserved12[0x4000];      /* Reserved area */

/* siu */
    volatile CYG_WORD siu_siumcr;        /* SIU Module Configuration Register */
    volatile CYG_WORD siu_sypcr;         /* System Protection Control Register */
    volatile CYG_BYTE reserved13[0x6];   /* Reserved area */
    volatile CYG_WORD16 siu_swsr;         /* Software Service Register */

/* buses */
    volatile CYG_BYTE reserved14[0x14];  /* Reserved area */
    volatile CYG_WORD bcr;               /* Bus Configuration Register */
    volatile CYG_BYTE ppc_acr;           /* Arbiter Configuration Register */
    volatile CYG_BYTE reserved15[0x3];   /* Reserved area */
    volatile CYG_WORD ppc_alrh;          /* Arbitration level Register (First clients)*/
    volatile CYG_WORD ppc_alrl;          /* Arbitration Level Register (Next clients) */
    volatile CYG_BYTE lcl_acr;           /* LCL Arbiter Configuration Register */
    volatile CYG_BYTE reserved16[0x3];   /* Reserved area */
    volatile CYG_WORD lcl_alrh;        /* LCL Arbitration level Register (First clients)*/
    volatile CYG_WORD lcl_alrl;        /* LCL Arbitration Level Register (Next clients) */
    volatile CYG_WORD tescr1;       /* PPC bus transfer error status control register 1 */
    volatile CYG_WORD tescr2;       /* PPC bus transfer error status control register 2 */
    volatile CYG_WORD ltescr1;    /* Local bus transfer error status control register 1 */
    volatile CYG_WORD ltescr2;    /* Local bus transfer error status control register 2 */
    volatile CYG_WORD pdtea;             /* PPC bus DMA Transfer Error Address */
    volatile CYG_BYTE pdtem;             /* PPC bus DMA Transfer Error MSNUM  */
    volatile CYG_BYTE reserved17[0x3];   /* Reserved area */
    volatile CYG_WORD ldtea;             /* PPC bus DMA Transfer Error Address */
    volatile CYG_BYTE ldtem;             /* PPC bus DMA Transfer Error MSNUM  */
    volatile CYG_BYTE reserved18[0xa3];  /* Reserved area */

/* memc */
    struct mem_regs 
    {
        volatile CYG_WORD memc_br;       /* Base Register */
        volatile CYG_WORD memc_or;       /* Option Register */
    } mem_regs[12];
    volatile CYG_BYTE reserved19[0x8];   /* Reserved area */
    volatile CYG_WORD memc_mar;          /* Memory Address Register */
    volatile CYG_BYTE reserved20[0x4];   /* Reserved area */
    volatile CYG_WORD memc_mamr;         /* Machine A Mode Register */
    volatile CYG_WORD memc_mbmr;         /* Machine B Mode Register */
    volatile CYG_WORD memc_mcmr;         /* Machine C Mode Register */
    volatile CYG_WORD memc_mdmr;         /* Machine D Mode Register */
    volatile CYG_BYTE reserved21[0x4];   /* Reserved area */
    volatile CYG_WORD16 memc_mptpr;       /* Memory Periodic Timer Prescaler */
    volatile CYG_BYTE reserved22[0x2];   /* Reserved area */
    volatile CYG_WORD memc_mdr;          /* Memory Data Register */
    volatile CYG_BYTE reserved23[0x4];   /* Reserved area */
    volatile CYG_WORD memc_psdmr;        /* PowerPC Bus SDRAM machine Mode Register */
    volatile CYG_WORD memc_lsdmr;        /* Local Bus SDRAM machine Mode Registe */
    volatile CYG_BYTE memc_purt;         /* PowerPC Bus assigned UPM Refresh Timer */
    volatile CYG_BYTE reserved24[0x3];   /* Reserved area */
    volatile CYG_BYTE memc_psrt;         /* PowerPC BusBus assigned SDRAM Refresh Timer */
    volatile CYG_BYTE reserved25[0x3];   /* Reserved area */
    volatile CYG_BYTE memc_lurt;         /* Local Bus assigned UPM Refresh Timer */
    volatile CYG_BYTE reserved26[0x3];   /* Reserved area */
    volatile CYG_BYTE memc_lsrt;         /* Local Bus assigned SDRAM Refresh Timer */
    volatile CYG_BYTE reserved27[0x3];   /* Reserved area */
    volatile CYG_WORD memc_immr;         /* Internal Memory Map Register */

/* pci */
    volatile CYG_WORD pcibr0;            /* Base address+valid for PCI window 1 */
    volatile CYG_WORD pcibr1;            /* Base address+valid for PCI window 2 */
    volatile CYG_BYTE reserved28[0x10];  /* Reserved area */
    volatile CYG_WORD pcimsk0;           /* Mask for PCI window 1 */
    volatile CYG_WORD pcimsk1;           /* Mask for PCI window 2 */
    volatile CYG_BYTE reserved29[0x54];  /* Reserved area */

/* si_timers */
    volatile CYG_WORD16 si_timers_tmcntsc; /* Time Counter Status and Control Register */
    volatile CYG_BYTE reserved30[0x2];    /* Reserved area */
    volatile CYG_WORD si_timers_tmcnt;    /* Time Counter Register */
    volatile CYG_WORD si_timers_tmcntsec; /* Time Counter Seconds*/
    volatile CYG_WORD si_timers_tmcntal;  /* Time Counter Alarm Register */
    volatile CYG_BYTE reserved31[0x10];   /* Reserved area */
    volatile CYG_WORD16 si_timers_piscr;   /* Periodic Interrupt Status and Control Reg. */
    volatile CYG_BYTE reserved32[0x2];    /* Reserved area */
    volatile CYG_WORD si_timers_pitc;     /* Periodic Interrupt Count Register */
    volatile CYG_WORD si_timers_pitr;     /* Periodic Interrupt Timer Register */
    volatile CYG_BYTE reserved33[0x54];   /* Reserved area */

/* test module registers */
    volatile CYG_WORD tstmhr;             
    volatile CYG_WORD tstmlr;
    volatile CYG_WORD16 tster;
    volatile CYG_BYTE reserved34[0x156]; /* Reserved area */
    
/* pci, part 2 */
    volatile CYG_WORD pci_pci;           /* PCI Configuration space - offset 0x10400 */
    volatile CYG_BYTE reserved_pci404[0x10430-0x10404];
    volatile CYG_WORD pci_omisr;         /* Outbound interrupt status */
    volatile CYG_WORD pci_omimr;         /* Outbound interrupt mask */
    volatile CYG_WORD reserved_pci438;
    volatile CYG_WORD reserved_pci43C;
    volatile CYG_WORD pci_ifqpr;         /* Inbound FIFO queue */
    volatile CYG_WORD pci_ofqpr;         /* Outbound FIFO queue */
    volatile CYG_WORD reserved_pci448;
    volatile CYG_WORD reserved_pci44C;
    volatile CYG_WORD pci_imr;           /* Inbound message register #0 */
    volatile CYG_WORD pci_imr1;          /* Inbound message register #1 */
    volatile CYG_WORD pci_omr0;          /* Outbound message register #0 */
    volatile CYG_WORD pci_omr1;          /* Outbound message register #1 */
    volatile CYG_WORD pci_odr;           /* Outbound doorbell */
    volatile CYG_WORD reserved_pci464;
    volatile CYG_WORD pci_idr;           /* Inbound doorbell */
    volatile CYG_BYTE reserved_pci46C[0x10480-0x1046C];
    volatile CYG_WORD pci_imisr;         /* Inbound message interrupt status */
    volatile CYG_WORD pci_imimr;         /* Inbound message interrupt mask */
    volatile CYG_BYTE reserved_pci488[0x104A0-0x10488];
    volatile CYG_WORD pci_ifhpr;         /* Inbound free FIFO head */
    volatile CYG_WORD reserved_pci4A4;
    volatile CYG_WORD pci_iftpr;         /* Inbound free FIFO tail */
    volatile CYG_WORD reserved_pci4AC;
    volatile CYG_WORD pci_iphpr;         /* Inbound post FIFO head */
    volatile CYG_WORD reserved_pci4B4;
    volatile CYG_WORD pci_iptpr;         /* Inbound post FIFO tail */
    volatile CYG_WORD reserved_pci4BC;
    volatile CYG_WORD pci_ofhpr;         /* Outbound free FIFO head */
    volatile CYG_WORD reserved_pci4C4;
    volatile CYG_WORD pci_oftpr;         /* Outbound free FIFO tail */
    volatile CYG_WORD reserved_pci4CC;
    volatile CYG_WORD pci_ophpr;         /* Outbound post FIFO head */
    volatile CYG_WORD reserved_pci4D4;
    volatile CYG_WORD pci_optpr;         /* Outbound post FIFO tail */
    volatile CYG_WORD reserved_pci4DC;
    volatile CYG_WORD reserved_pci4E0;
    volatile CYG_WORD pci_mucr;          /* Message unit control */
    volatile CYG_BYTE reserved_pci4E8[0x104F0-0x104E8];
    volatile CYG_WORD pci_qbar;          /* Queue base address */
    volatile CYG_BYTE reserved_pci4F4[0x10500-0x104F4];
    volatile CYG_WORD pci_dmamr0;        /* DMA #0 - mode */
    volatile CYG_WORD pci_dmasr0;        /* DMA #0 - status */
    volatile CYG_WORD pci_dmacdar0;      /* DMA #0 - current descriptor address */
    volatile CYG_WORD reserved_pci50C;
    volatile CYG_WORD pci_dmasar0;       /* DMA #0 - source address */
    volatile CYG_WORD reserved_pci514;
    volatile CYG_WORD pci_dmadar0;       /* DMA #0 - destination address */
    volatile CYG_WORD reserved_pci51C;
    volatile CYG_WORD pci_dmabcr0;       /* DMA #0 - byte count */
    volatile CYG_WORD pci_dmandar0;      /* DMA #0 - next descriptor */
    volatile CYG_BYTE reserved_pci528[0x10580-0x10528];
    volatile CYG_WORD pci_dmamr1;        /* DMA #1 - mode */
    volatile CYG_WORD pci_dmasr1;        /* DMA #1 - status */
    volatile CYG_WORD pci_dmacdar1;      /* DMA #1 - current descriptor address */
    volatile CYG_WORD reserved_pci58C;
    volatile CYG_WORD pci_dmasar1;       /* DMA #1 - source address */
    volatile CYG_WORD reserved_pci594;
    volatile CYG_WORD pci_dmadar1;       /* DMA #1 - destination address */
    volatile CYG_WORD reserved_pci59C;
    volatile CYG_WORD pci_dmabcr1;       /* DMA #1 - byte count */
    volatile CYG_WORD pci_dmandar1;      /* DMA #1 - next descriptor */
    volatile CYG_BYTE reserved_pci5A8[0x10600-0x105A8];
    volatile CYG_WORD pci_dmamr2;        /* DMA #2 - mode */
    volatile CYG_WORD pci_dmasr2;        /* DMA #2 - status */
    volatile CYG_WORD pci_dmacdar2;      /* DMA #2 - current descriptor address */
    volatile CYG_WORD reserved_pci60C;
    volatile CYG_WORD pci_dmasar2;       /* DMA #2 - source address */
    volatile CYG_WORD reserved_pci614;
    volatile CYG_WORD pci_dmadar2;       /* DMA #2 - destination address */
    volatile CYG_WORD reserved_pci61C;
    volatile CYG_WORD pci_dmabcr2;       /* DMA #2 - byte count */
    volatile CYG_WORD pci_dmandar2;      /* DMA #2 - next descriptor */
    volatile CYG_BYTE reserved_pci628[0x10680-0x10628];
    volatile CYG_WORD pci_dmamr3;        /* DMA #3 - mode */
    volatile CYG_WORD pci_dmasr3;        /* DMA #3 - status */
    volatile CYG_WORD pci_dmacdar3;      /* DMA #3 - current descriptor address */
    volatile CYG_WORD reserved_pci68C;
    volatile CYG_WORD pci_dmasar3;       /* DMA #3 - source address */
    volatile CYG_WORD reserved_pci694;
    volatile CYG_WORD pci_dmadar3;       /* DMA #3 - destination address */
    volatile CYG_WORD reserved_pci69C;
    volatile CYG_WORD pci_dmabcr3;       /* DMA #3 - byte count */
    volatile CYG_WORD pci_dmandar3;      /* DMA #3 - next descriptor */
    volatile CYG_BYTE reserved_pci6A8[0x10800-0x106A8];
    volatile CYG_WORD pci_potar0;        /* PCI outbound translation address #0 */
    volatile CYG_WORD reserved_pci804;
    volatile CYG_WORD pci_potbar0;       /* PCI outbound base address #0 */
    volatile CYG_WORD reserved_pci80C;
    volatile CYG_WORD pci_pocmr0;        /* PCI outbound comparison mask #0 */
    volatile CYG_WORD reserved_pci814;
    volatile CYG_WORD pci_potar1;        /* PCI outbound translation address #1 */
    volatile CYG_WORD reserved_pci81C;
    volatile CYG_WORD pci_potbar1;       /* PCI outbound base address #1 */
    volatile CYG_WORD reserved_pci824;
    volatile CYG_WORD pci_pocmr1;        /* PCI outbound comparison mask #1 */
    volatile CYG_WORD reserved_pci82C;
    volatile CYG_WORD pci_potar2;        /* PCI outbound translation address #2 */
    volatile CYG_WORD reserved_pci834;
    volatile CYG_WORD pci_potbar2;       /* PCI outbound base address #2 */
    volatile CYG_WORD reserved_pci83C;
    volatile CYG_WORD pci_pocmr2;        /* PCI outbound comparison mask #2 */
    volatile CYG_BYTE reserved_pci844[0x10878-0x10844];
    volatile CYG_WORD pci_ptcr;          /* Discard timer control */
    volatile CYG_WORD pci_gpcr;          /* General purpose control */
    volatile CYG_WORD pci_gcr;           /* PCI general control */
    volatile CYG_WORD pci_esr;           /* Error status */
    volatile CYG_WORD pci_emr;           /* Error mask */
    volatile CYG_WORD pci_ecr;           /* Error control */
    volatile CYG_WORD pci_eacr;          /* Error address capture */
    volatile CYG_WORD reserved_pci894;
    volatile CYG_WORD pci_edcr;          /* Error data capture */
    volatile CYG_WORD reserved_pci89C;
    volatile CYG_WORD pci_eccr;          /* Error control */
    volatile CYG_BYTE reserved_pci8D0[0x108D0-0x108A4];
    volatile CYG_WORD pci_pitar1;        /* Inbound address translation #1 */
    volatile CYG_WORD reserved_pci8D8;
    volatile CYG_WORD pci_pibar1;        /* Inbound address base #1 */
    volatile CYG_WORD reserved_pci8E0;
    volatile CYG_WORD pci_picmr1;        /* Inbound comparison mask #1 */
    volatile CYG_WORD reserved_pci8E8;
    volatile CYG_WORD pci_pitar0;        /* Inbound address translation #0 */
    volatile CYG_WORD reserved_pci8F0;
    volatile CYG_WORD pci_pibar0;        /* Inbound address base #0 */
    volatile CYG_WORD reserved_pci8F8;
    volatile CYG_WORD pci_picmr0;        /* Inbound comparison mask #0 */
    volatile CYG_WORD reserved_pci900;
    volatile CYG_WORD pci_cfg_addr;      /* Indirect access to PCI config space - address ptr */
    volatile CYG_WORD pci_cfg_data;      /* Indirect access to PCI config space - data */
    volatile CYG_WORD pci_int_ack;       /* Used to acknowledge PCI interrupts */
    volatile CYG_BYTE reserved_pci0C00[0x10C00-0x1090C];
    
/* ic */
    volatile CYG_WORD16 ic_sicr;         /* Interrupt Configuration Register - offset 0x10C00 */
    volatile CYG_BYTE reserved36[0x2];   /* Reserved area */
    volatile CYG_BYTE ic_sivec;          /* CP Interrupt Vector Register */
    volatile CYG_BYTE reserved36a[0x3];  /* Reserved area */
    volatile CYG_WORD ic_sipnr_h;        /* Interrupt Pending Register (HIGH) */
    volatile CYG_WORD ic_sipnr_l;        /* Interrupt Pending Register (LOW)    */
    volatile CYG_WORD ic_siprr;          /* SIU Interrupt Priority Register     */
    volatile CYG_WORD ic_scprr_h;        /* Interrupt Priority Register (HIGH) */
    volatile CYG_WORD ic_scprr_l;        /* Interrupt Priority Register (LOW)   */
    volatile CYG_WORD ic_simr_h;         /* Interrupt Mask Register (HIGH)      */
    volatile CYG_WORD ic_simr_l;         /* Interrupt Mask Register (LOW)       */
    volatile CYG_WORD ic_siexr;          /* External Interrupt Control Register */
    volatile CYG_BYTE reserved37[0x58];  /* Reserved area */

/* clocks */
    volatile CYG_WORD clocks_sccr;       /* System Clock Control Register */
    volatile CYG_BYTE reserved38[0x4];   /* Reserved area */
    volatile CYG_WORD clocks_scmr;       /* System Clock Mode Register  */
    volatile CYG_BYTE reserved39[0x4];   /* Reserved area */
    volatile CYG_WORD clocks_rsr;        /* Reset Status Register */
    volatile CYG_WORD clocks_rmr;        /* Reset Mode Register  */
    volatile CYG_BYTE reserved40[0x68];  /* Reserved area */

/* io_ports */
    struct io_regs 
    {
       volatile CYG_WORD pdir;              /* Port A-D Data Direction Register */
       volatile CYG_WORD ppar;              /* Port A-D Pin Assignment Register */
       volatile CYG_WORD psor;              /* Port A-D Special Operation Register */
       volatile CYG_WORD podr;              /* Port A-D Open Drain Register */
       volatile CYG_WORD pdat;              /* Port A-D Data Register */
       volatile CYG_BYTE reserved41[0xc];   /* Reserved area */
    } io_regs[4];

/* cpm_timers */
    volatile CYG_BYTE cpm_timers_tgcr1;   /* Timer Global Configuration Register */
    volatile CYG_BYTE reserved42[0x3];    /* Reserved area */
    volatile CYG_BYTE cpm_timers_tgcr2;   /* Timer Global Configuration Register */
    volatile CYG_BYTE reserved43[0xb];    /* Reserved area */
    volatile CYG_WORD16 cpm_timers_tmr1;   /* Timer Mode Register */
    volatile CYG_WORD16 cpm_timers_tmr2;   /* Timer Mode Register */
    volatile CYG_WORD16 cpm_timers_trr1;   /* Timer Reference Register */
    volatile CYG_WORD16 cpm_timers_trr2;   /* Timer Reference Register */
    volatile CYG_WORD16 cpm_timers_tcr1;   /* Timer Capture Register */
    volatile CYG_WORD16 cpm_timers_tcr2;   /* Timer Capture Register */
    volatile CYG_WORD16 cpm_timers_tcn1;   /* Timer Counter */
    volatile CYG_WORD16 cpm_timers_tcn2;   /* Timer Counter */
    volatile CYG_WORD16 cpm_timers_tmr3;   /* Timer Mode Register */
    volatile CYG_WORD16 cpm_timers_tmr4;   /* Timer Mode Register */
    volatile CYG_WORD16 cpm_timers_trr3;   /* Timer Reference Register */
    volatile CYG_WORD16 cpm_timers_trr4;   /* Timer Reference Register */
    volatile CYG_WORD16 cpm_timers_tcr3;   /* Timer Capture Register */
    volatile CYG_WORD16 cpm_timers_tcr4;   /* Timer Capture Register */
    volatile CYG_WORD16 cpm_timers_tcn3;   /* Timer Counter */
    volatile CYG_WORD16 cpm_timers_tcn4;   /* Timer Counter */
    volatile CYG_WORD16 cpm_timers_ter[4]; /* Timer Event Register */
    volatile CYG_BYTE reserved44[0x260];  /* Reserved area */

/* sdma general */
    volatile CYG_BYTE sdma_sdsr;         /* SDMA Status Register */
    volatile CYG_BYTE reserved45[0x3];   /* Reserved area */
    volatile CYG_BYTE sdma_sdmr;         /* SDMA Mask Register */
    volatile CYG_BYTE reserved46[0x3];   /* Reserved area */

/* idma */
    volatile CYG_BYTE idma_idsr1;        /* IDMA Status Register */
    volatile CYG_BYTE reserved47[0x3];   /* Reserved area */
    volatile CYG_BYTE idma_idmr1;        /* IDMA Mask Register */
    volatile CYG_BYTE reserved48[0x3];   /* Reserved area */
    volatile CYG_BYTE idma_idsr2;        /* IDMA Status Register */
    volatile CYG_BYTE reserved49[0x3];   /* Reserved area */
    volatile CYG_BYTE idma_idmr2;        /* IDMA Mask Register */
    volatile CYG_BYTE reserved50[0x3];   /* Reserved area */
    volatile CYG_BYTE idma_idsr3;        /* IDMA Status Register */
    volatile CYG_BYTE reserved51[0x3];   /* Reserved area */
    volatile CYG_BYTE idma_idmr3;        /* IDMA Mask Register */
    volatile CYG_BYTE reserved52[0x3];   /* Reserved area */
    volatile CYG_BYTE idma_idsr4;        /* IDMA Status Register */
    volatile CYG_BYTE reserved53[0x3];   /* Reserved area */
    volatile CYG_BYTE idma_idmr4;        /* IDMA Mask Register */
    volatile CYG_BYTE reserved54[0x2c3]; /* Reserved area */
    
/* fcc */
    struct fcc_regs 
    {
        volatile CYG_WORD fcc_gfmr;        /* FCC General Mode Register */
        volatile CYG_WORD fcc_psmr;        /* FCC Protocol Specific Mode Register */
        volatile CYG_WORD16 fcc_todr;       /* FCC Transmit On Demand Register */
        volatile CYG_BYTE reserved55[0x2]; /* Reserved area */
        volatile CYG_WORD16 fcc_dsr;        /* FCC Data Sync. Register */
        volatile CYG_BYTE reserved56[0x2]; /* Reserved area */
        volatile CYG_WORD16 fcc_fcce;        /* FCC Event Register */
        volatile CYG_BYTE reserved56a[0x2]; /* Reserved area */
        volatile CYG_WORD16 fcc_fccm;        /* FCC Mask Register */
        volatile CYG_BYTE reserved56b[0x2]; /* Reserved area */
        volatile CYG_BYTE fcc_fccs;        /* FCC Status Register */
        volatile CYG_BYTE reserved57[0x3]; /* Reserved area */
        volatile CYG_WORD fcc_ftprr;       /* FCC Transmit Partial Rate Register */
    } fcc_regs[3];
    volatile CYG_BYTE reserved58[0x290];   /* Reserved area */
    
/* brgs 5 through 8 */
    volatile CYG_WORD brgs_brgc5;          /* Baud Rate Generator 5 Config Register */
    volatile CYG_WORD brgs_brgc6;          /* Baud Rate Generator 6 Config Register */
    volatile CYG_WORD brgs_brgc7;          /* Baud Rate Generator 7 Config Register */
    volatile CYG_WORD brgs_brgc8;          /* Baud Rate Generator 8 Config Register */
    volatile CYG_BYTE reserved59[0x260]; /* Reserved area */
    
/* i2c */
    volatile CYG_BYTE i2c_i2mod;         /* IC Mode Register */
    volatile CYG_BYTE reserved60[0x3];   /* Reserved area */
    volatile CYG_BYTE i2c_i2add;         /* IC Address Register */
    volatile CYG_BYTE reserved61[0x3];   /* Reserved area */
    volatile CYG_BYTE i2c_i2brg;         /* IC BRG Register */
    volatile CYG_BYTE reserved62[0x3];   /* Reserved area */
    volatile CYG_BYTE i2c_i2com;         /* IC Command Register */
    volatile CYG_BYTE reserved63[0x3];   /* Reserved area */
    volatile CYG_BYTE i2c_i2cer;         /* IC Event Register */
    volatile CYG_BYTE reserved64[0x3];   /* Reserved area */
    volatile CYG_BYTE i2c_i2cmr;         /* IC Mask Register */
    volatile CYG_BYTE reserved65[0x14b]; /* Reserved area */
    
/* cpm */
    volatile CYG_WORD  cpm_cpcr;         /* Communication Processor Command Register */
    volatile CYG_WORD  cpm_rccr;         /* RISC Configuration Register */
    volatile CYG_WORD  cpm_rmdr;         /* RISC Microcode Dev. Support Control Reg. */
    volatile CYG_WORD16 cpm_rctr1;        /* RISC Controller Trap Register */
    volatile CYG_WORD16 cpm_rctr2;        /* RISC Controller Trap Register */
    volatile CYG_WORD16 cpm_rctr3;        /* RISC Controller Trap Register */
    volatile CYG_WORD16 cpm_rctr4;        /* RISC Controller Trap Register */
    volatile CYG_BYTE  reserved66[0x2];  /* Reserved area */
    volatile CYG_WORD16 cpm_rter;         /* RISC Timers Event Register */
    volatile CYG_BYTE  reserved67[0x2];  /* Reserved area */
    volatile CYG_WORD16 cpm_rtmr;         /* RISC Timers Mask Register */
    volatile CYG_WORD16 cpm_rtscr;        /* RISC Time-Stamp Timer Control Register */
    volatile CYG_WORD16 cpm_rmds;         /* RISC Development Support Status Register */
    volatile CYG_WORD  cpm_rtsr;         /* RISC Time-Stamp Register */
    volatile CYG_BYTE  reserved68[0xc];  /* Reserved area */

/* brgs 1 through 4 */
    volatile CYG_WORD brgs_brgc1;          /* Baud Rate Generator 5 Config Register */
    volatile CYG_WORD brgs_brgc2;          /* Baud Rate Generator 2 Config Register */
    volatile CYG_WORD brgs_brgc3;          /* Baud Rate Generator 3 Config Register */
    volatile CYG_WORD brgs_brgc4;          /* Baud Rate Generator 4 Config Register */
    
/* scc */
    struct scc_regs_8260
    {
        volatile CYG_WORD  gsmr_l;          /* SCC General Mode Register */
        volatile CYG_WORD  gsmr_h;          /* SCC General Mode Register */
        volatile CYG_WORD16 psmr;            /* SCC Protocol Specific Mode Register */
        volatile CYG_BYTE  reserved69[0x2]; /* Reserved area */
        volatile CYG_WORD16 todr;            /* SCC Transmit-On-Demand Register */
        volatile CYG_WORD16 dsr;             /* SCC Data Synchronization Register */
        volatile CYG_WORD16 scce;            /* SCC Event Register */
        volatile CYG_BYTE  reserved70[0x2]; /* Reserved area */
        volatile CYG_WORD16 sccm;            /* SCC Mask Register */
        volatile CYG_BYTE  reserved71;      /* Reserved area */
        volatile CYG_BYTE  sccs;            /* SCC Status Register */
        volatile CYG_BYTE  reserved72[0x8]; /* Reserved area */
    } scc_regs[4];
    
/* smc */
    struct smc_regs_8260
    {
        volatile CYG_BYTE reserved73[0x2]; /* Reserved area */
        volatile CYG_WORD16 smc_smcmr;      /* SMC Mode Register */
        volatile CYG_BYTE reserved74[0x2]; /* Reserved area */
        volatile CYG_BYTE smc_smce;        /* SMC Event Register */
        volatile CYG_BYTE reserved75[0x3]; /* Reserved area */
        volatile CYG_BYTE smc_smcm;        /* SMC Mask Register */
        volatile CYG_BYTE reserved76[0x5]; /* Reserved area */
    } smc_regs[2];
    
/* spi */
    volatile CYG_WORD16 spi_spmode;       /* SPI Mode Register */
    volatile CYG_BYTE reserved77[0x4];   /* Reserved area */
    volatile CYG_BYTE spi_spie;          /* SPI Event Register */
    volatile CYG_BYTE reserved78[0x3];   /* Reserved area */
    volatile CYG_BYTE spi_spim;          /* SPI Mask Register */
    volatile CYG_BYTE reserved79[0x2];   /* Reserved area */
    volatile CYG_BYTE spi_spcom;         /* SPI Command Register */
    volatile CYG_BYTE reserved80[0x52];  /* Reserved area */
    
/* cpm_mux */

    volatile CYG_BYTE  cpm_mux_cmxsi1cr;    /* CPM MUX SI Clock Route Register */
    volatile CYG_BYTE  reserved81;          /* Reserved area */
    volatile CYG_BYTE  cpm_mux_cmxsi2cr;    /* CPM MUX SI Clock Route Register */
    volatile CYG_BYTE  reserved82;          /* Reserved area */
    volatile CYG_WORD  cpm_mux_cmxfcr;      /* CPM MUX FCC Clock Route Register */
    volatile CYG_WORD  cpm_mux_cmxscr;      /* CPM MUX SCC Clock Route Register */
    volatile CYG_BYTE  cpm_mux_cmxsmr;      /* CPM MUX SMC Clock Route Register */
    volatile CYG_BYTE  reserved83;          /* Reserved area */
    volatile CYG_WORD16 cpm_mux_cmxuar;      /* CPM MUX UTOPIA Address Register */
    volatile CYG_BYTE  reserved84[0x10];    /* Reserved area */
    
/* si */
    struct si_regs 
    {
        volatile CYG_WORD16 si_si1mr[4];    /* SI TDM Mode Registers */
        volatile CYG_BYTE si_si1gmr;       /* SI Global Mode Register */
        volatile CYG_BYTE reserved85;      /* Reserved area */
        volatile CYG_BYTE si_si1cmdr;      /* SI Command Register */
        volatile CYG_BYTE reserved86;      /* Reserved area */
        volatile CYG_BYTE si_si1str;       /* SI Status Register */
        volatile CYG_BYTE reserved87;      /* Reserved area */
        volatile CYG_WORD16 si_si1rsr;      /* SI RAM Shadow Address Register */
        volatile CYG_WORD16 mcc_mcce;       /* MCC Event Register */
        volatile CYG_BYTE reserved88[0x2]; /* Reserved area */
        volatile CYG_WORD16 mcc_mccm;       /* MCC Mask Register */
        volatile CYG_BYTE reserved89[0x2]; /* Reserved area */
        volatile CYG_BYTE mcc_mccf;        /* MCC Configuration Register */
        volatile CYG_BYTE reserved90[0x7]; /* Reserved area */
    } si_regs[2];
    volatile CYG_BYTE reserved91[0x4a0];   /* Reserved area */
    
/* si_ram */
    struct si_ram 
    {
        CYG_WORD16 si1_ram_si1_tx_ram[0x100]; /* SI Transmit Routing RAM */
        volatile CYG_BYTE reserved92[0x200];         /* Reserved area */
        CYG_WORD16 si1_ram_si1_rx_ram[0x100]; /* SI Receive Routing RAM */
        volatile CYG_BYTE reserved93[0x200];         /* Reserved area */
    } si_ram[2];
    volatile CYG_BYTE reserved94[0x1000];  /* Reserved area */

} _PackedType t_PQ2IMM;

extern volatile t_PQ2IMM  *IMM;   /* IMM base pointer */

/***************************************************************************/
/*                   General Global Definitions                            */
/***************************************************************************/

#define PAGE1     0         /* SCC1 Index into SCC Param RAM Array */
#define PAGE2     1         /* SCC2 Index into SCC Param RAM Array */
#define PAGE3     2         /* SCC3 Index into SCC Param RAM Array */
#define PAGE4     3         /* SCC4 Index into SCC Param RAM Array */

#define SCC1      0         /* SCC1 Index into SCC Regs Array  */          
#define SCC2      1         /* SCC2 Index into SCC Regs Array  */          
#define SCC3      2         /* SCC3 Index into SCC Regs Array  */          
#define SCC4      3         /* SCC4 Index into SCC Regs Array  */          

#define SMC1      0         /* SMC1 Index into SMC Regs Array  */          
#define SMC2      1         /* SMC2 Index into SMC Regs Array  */          

#define PORT_A    0         /* Parallel port A registers */
#define PORT_B    1         /* Parallel port B registers */
#define PORT_C    2         /* Parallel port C registers */
#define PORT_D    3         /* Parallel port D registers */

/*--------------------------------*/
/* KEEP ALIVE POWER REGISTERS KEY */
/*--------------------------------*/

#define KEEP_ALIVE_KEY 0x55ccaa33

/*------------------------------------------*
* CPM Command Register (CPCR)               *
*-------------------------------------------*
* NOTE: This register is cleared by reset.  *
*       See MPC8260 User's Manual.          *
*-------------------------------------------*/

#define CPCR_RST                 0x80000000  /* Software Reset Command */
#define CPCR_FLG                 0x00010000  /* Command Semaphore Flag */

#define CPCR_SUBBLOCK(page,code) ((page<<26)|(code<<21))

/*-----------------------------------------------*/
/* Definitions for SCC CPCR Subblock/Page codes. */
/*-----------------------------------------------*/

#define  SCC1_PAGE_SUBBLOCK    CPCR_SUBBLOCK(0,4)
#define  SCC2_PAGE_SUBBLOCK    CPCR_SUBBLOCK(1,5)
#define  SCC3_PAGE_SUBBLOCK    CPCR_SUBBLOCK(2,6)
#define  SCC4_PAGE_SUBBLOCK    CPCR_SUBBLOCK(3,7)

/*-----------------------------------------------*/
/* Definitions for SMC CPCR Subblock/Page codes. */
/*-----------------------------------------------*/

#define  SMC1_PAGE_SUBBLOCK    CPCR_SUBBLOCK(7,8)
#define  SMC2_PAGE_SUBBLOCK    CPCR_SUBBLOCK(8,9)

/*-----------------------------------------------*/
/* Definitions for FCC CPCR Subblock/Page codes. */
/*-----------------------------------------------*/

#define  FCC1_PAGE_SUBBLOCK    CPCR_SUBBLOCK(4,0x10)
#define  FCC2_PAGE_SUBBLOCK    CPCR_SUBBLOCK(5,0x11)
#define  FCC3_PAGE_SUBBLOCK    CPCR_SUBBLOCK(6,0x12)

/*-----------------------------*/
/* Opcode definitions for SCCs, opcodes for SMC's are the same
 * except not all codes are valid for SMC */
/*-----------------------------*/

#define CPCR_INIT_TX_RX_PARAMS   0x00000000   /* Opcode 0 */
#define CPCR_INIT_RX_PARAMS      0x00000001   /* Opcode 1 */
#define CPCR_INIT_TX_PARAMS      0x00000002   /* Opcode 2 */
#define CPCR_ENTER_HUNT_MODE     0x00000003   /* Opcode 3 */
#define CPCR_STOP_TX             0x00000004   /* Opcode 4 */
#define CPCR_GRACEFUL_STOP_TX    0x00000005   /* Opcode 5 */
#define CPCR_RESTART_TX          0x00000006   /* Opcode 6 */
#define CPCR_CLOSE_RX_BD         0x00000007   /* Opcode 7 */
#define CPCR_SET_GRP_ADDR        0x00000008   /* Opcode 8 */
#define CPCR_RESET_BCS           0x0000000A   /* Opcode 10 */

/*-----------------------------------------------------*/
/* General Definitions for SCC CPCR Command Operations */
/*-----------------------------------------------------*/

#define READY_TO_RX_CMD          0x00000000

/*-------------------------*/
/* General SCC Definitions */
/*-------------------------*/

#define  DISABLE_TX_RX   0xFFFFFFCF  /* Clear the ENT/ENR bits in the GSMR
                                        Disables the transmit & Receive
                                        port                              */

#define  GSMR_L1_ENT  0x00000010     /* ENT bit for the GSMR low register */
#define  GSMR_L1_ENR  0x00000020     /* ENR bit for the GSMR low register */


#define  ALL_ONES    0xFFFF

struct cp_bufdesc {
    volatile unsigned short ctrl;	/* status/control register */
    volatile unsigned short length;	/* buffer length */
    volatile char  	    *buffer;	/* buffer pointer */
};

// Buffer descriptor control bits
#define _BD_CTL_Ready          0x8000  // Buffer contains data (tx) or is empty (rx)
#define _BD_CTL_Wrap           0x2000  // Last buffer in list
#define _BD_CTL_Int            0x1000  // Generate interrupt when empty (tx) or full (rx)
#define _BD_CTL_Last           0x0800  // Last buffer in a sequence
#define _BD_CTL_MASK           0xB000  // User settable bits

//
// Event masks
//
#define SCCE_Bsy 0x0004                 // Rx error
#define SCCE_Tx  0x0002                 // Tx buffer interrupt
#define SCCE_Rx  0x0001                 // Rx buffer interrupt
#define SMCE_Bsy 0x0004                 // Rx error
#define SMCE_Tx  0x0002                 // Tx buffer interrupt
#define SMCE_Rx  0x0001                 // Rx buffer interrupt

//
// Timer/counter registers
//

// Timer/counter general control
#define _TC_TGCR_CAS  0x80  // Cascade timers (1/2, 3/4)
#define _TC_TGCR_STP2 0x20  // Stop timer 2
#define _TC_TGCR_RST2 0x10  // Reset timer 2
#define _TC_TGCR_GM   0x08  // Gate mode
#define _TC_TGCR_STP1 0x02  // Stop timer 1
#define _TC_TGCR_RST1 0x01  // Reset timer 1

// Timer/counter mode
#define _TC_TMR_ORI         0x0010  // Output reference interrupt enable
#define _TC_TMR_FRR         0x0008  // Free run
#define _TC_TMR_ICLK_CAS    0x0000  // Input clock - cascade from other timer
#define _TC_TMR_ICLK_BUS    0x0002  // Input clock = bus clock
#define _TC_TMR_ICLK_BUS16  0x0004  // Input clock = bus clock / 16
#define _TC_TMR_ICLK_TIN    0x0006  // Input clock = TINx

// Timer/counter event register 
#define _TC_TER_REF         0x0002  // Reference event occurred
#define _TC_TER_CAP         0x0001  // Capture event occurred

//
// Routines which help manage the CPM
//
externC void _mpc8xxx_reset_cpm(void);
externC unsigned int _mpc8xxx_allocBd(int len);

#endif // ifndef CYGONCE_HAL_PPC_QUICC2_MPC8260_H
