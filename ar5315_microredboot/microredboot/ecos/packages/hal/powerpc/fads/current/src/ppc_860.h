#ifndef CYGONCE_HAL_PPC_FADS_PPC_860_H
#define CYGONCE_HAL_PPC_FADS_PPC_860_H

//=============================================================================
//####UNSUPPORTEDBEGIN####
//
// -------------------------------------------
// This source file has been contributed to eCos/Red Hat. It may have been
// changed slightly to provide an interface consistent with those of other 
// files.
//
// The functionality and contents of this file is supplied "AS IS"
// without any form of support and will not necessarily be kept up
// to date by Red Hat.
//
// The style of programming used in this file may not comply with the
// eCos programming guidelines. Please do not use as a base for other
// files.
//
// All inquiries about this file, or the functionality provided by it,
// should be directed to the 'ecos-discuss' mailing list (see
// http://sourceware.cygnus.com/ecos/intouch.html for details).
//
// Contributed by: Kevin Hester <khester@opticworks.com>
// Maintained by:  <Unmaintained>
// See also:
//   Motorola's "Example Software Initializing the SMC as a UART" package 
//   (smc2.zip) at:
//    http://www.mot.com/SPS/RISC/netcomm/tools/index.html#MPC860_table
// -------------------------------------------
//
//####UNSUPPORTEDEND####
//=============================================================================

#include <cyg/infra/cyg_type.h>

/******************************************************************************
*
* Definitions of Parameter RAM entries for each peripheral and mode
*
******************************************************************************/

/*---------------------------------------------------------------------------*/
/*      HDLC parameter RAM (SCC)                                             */
/*---------------------------------------------------------------------------*/

struct hdlc_pram 

{
       
   /*-------------------*/
   /* SCC parameter RAM */
   /*-------------------*/
   
   cyg_uint16   rbase;          /* RX BD base address */
   cyg_uint16   tbase;          /* TX BD base address */
   cyg_uint8       rfcr;                   /* Rx function code */
   cyg_uint8       tfcr;                   /* Tx function code */
   cyg_uint16   mrblr;          /* Rx buffer length */
   cyg_uint32      rstate;              /* Rx internal state */
   cyg_uint32      rptr;                   /* Rx internal data pointer */
   cyg_uint16   rbptr;          /* rb BD Pointer */
   cyg_uint16   rcount;         /* Rx internal byte count */
   cyg_uint32      rtemp;               /* Rx temp */
   cyg_uint32      tstate;              /* Tx internal state */
   cyg_uint32      tptr;                   /* Tx internal data pointer */
   cyg_uint16   tbptr;          /* Tx BD pointer */
   cyg_uint16   tcount;         /* Tx byte count */
   cyg_uint32      ttemp;               /* Tx temp */
   cyg_uint32      rcrc;                   /* temp receive CRC */
   cyg_uint32      tcrc;                   /* temp transmit CRC */
   
   /*-----------------------------*/
   /* HDLC specific parameter RAM */
   /*-----------------------------*/

   cyg_uint8       RESERVED1[4];        /* Reserved area */
   cyg_uint32      c_mask;                      /* CRC constant */
   cyg_uint32      c_pres;                      /* CRC preset */
   cyg_uint16   disfc;                  /* discarded frame counter */
   cyg_uint16   crcec;                  /* CRC error counter */
   cyg_uint16   abtsc;                  /* abort sequence counter */
   cyg_uint16   nmarc;                  /* nonmatching address rx cnt */
   cyg_uint16   retrc;                  /* frame retransmission cnt */
   cyg_uint16   mflr;                      /* maximum frame length reg */
   cyg_uint16   max_cnt;                   /* maximum length counter */
   cyg_uint16   rfthr;                  /* received frames threshold */
   cyg_uint16   rfcnt;                  /* received frames count */
   cyg_uint16   hmask;                  /* user defined frm addr mask */
   cyg_uint16   haddr1;                 /* user defined frm address 1 */
   cyg_uint16   haddr2;                 /* user defined frm address 2 */
   cyg_uint16   haddr3;                 /* user defined frm address 3 */
   cyg_uint16   haddr4;                 /* user defined frm address 4 */
   cyg_uint16   tmp;                       /* temp */
   cyg_uint16   tmp_mb;                 /* temp */
};


/*-------------------------------------------------------------------------*/
/*                       ASYNC HDLC parameter RAM (SCC)                                                  */
/*-------------------------------------------------------------------------*/

struct async_hdlc_pram 

{
       
   /*-------------------*/
   /* SCC parameter RAM */
   /*-------------------*/

   cyg_uint16   rbase;          /* RX BD base address */
   cyg_uint16   tbase;          /* TX BD base address */
   cyg_uint8       rfcr;                   /* Rx function code */
   cyg_uint8       tfcr;                   /* Tx function code */
   cyg_uint16   mrblr;          /* Rx buffer length */
   cyg_uint32      rstate;              /* Rx internal state */
   cyg_uint32      rptr;                   /* Rx internal data pointer */
   cyg_uint16   rbptr;          /* rb BD Pointer */
   cyg_uint16   rcount;         /* Rx internal byte count */
   cyg_uint32      rtemp;               /* Rx temp */
   cyg_uint32      tstate;              /* Tx internal state */
   cyg_uint32      tptr;                   /* Tx internal data pointer */
   cyg_uint16   tbptr;          /* Tx BD pointer */
   cyg_uint16   tcount;         /* Tx byte count */
   cyg_uint32   ttemp;          /* Tx temp */
   cyg_uint32      rcrc;                   /* temp receive CRC */
   cyg_uint32    tcrc;             /* temp transmit CRC */
         
   /*-----------------------------------*/
   /* ASYNC HDLC specific parameter RAM */
   /*-----------------------------------*/

   cyg_uint8       RESERVED2[4];        /* Reserved area */
   cyg_uint32      c_mask;                      /* CRC constant */
   cyg_uint32      c_pres;                      /* CRC preset */
   cyg_uint16   bof;                       /* begining of flag character */
   cyg_uint16   eof;                       /* end of flag character */
   cyg_uint16   esc;                       /* control escape character */
   cyg_uint8       RESERVED3[4];        /* Reserved area */
   cyg_uint16   zero;                      /* zero */
   cyg_uint8       RESERVED4[2];        /* Reserved area */
   cyg_uint16   rfthr;                  /* received frames threshold */
   cyg_uint8       RESERVED5[4];        /* Reserved area */
   cyg_uint32      txctl_tbl;           /* Tx ctl char mapping table */
   cyg_uint32      rxctl_tbl;           /* Rx ctl char mapping table */
   cyg_uint16   nof;                       /* Number of opening flags */
};


/*--------------------------------------------------------------------------*/
/*                    UART parameter RAM (SCC)                                                                 */
/*--------------------------------------------------------------------------*/

/*----------------------------------------*/
/* bits in uart control characters table  */
/*----------------------------------------*/

#define CC_INVALID      0x8000          /* control character is valid */
#define CC_REJ          0x4000          /* don't store char in buffer */
#define CC_CHAR         0x00ff          /* control character */

/*------*/
/* UART */
/*------*/

struct uart_pram 

{
   /*-------------------*/
   /* SCC parameter RAM */
   /*-------------------*/

   cyg_uint16   rbase;          /* RX BD base address */
   cyg_uint16   tbase;          /* TX BD base address */
   cyg_uint8       rfcr;                   /* Rx function code */
   cyg_uint8       tfcr;                   /* Tx function code */
   cyg_uint16   mrblr;          /* Rx buffer length */
   cyg_uint32      rstate;              /* Rx internal state */
   cyg_uint32      rptr;                   /* Rx internal data pointer */
   cyg_uint16   rbptr;          /* rb BD Pointer */
   cyg_uint16   rcount;         /* Rx internal byte count */
   cyg_uint32      rx_temp;        /* Rx temp */
   cyg_uint32      tstate;              /* Tx internal state */
   cyg_uint32      tptr;                   /* Tx internal data pointer */
   cyg_uint16   tbptr;          /* Tx BD pointer */
   cyg_uint16   tcount;         /* Tx byte count */
   cyg_uint32      ttemp;               /* Tx temp */
   cyg_uint32      rcrc;                   /* temp receive CRC */
   cyg_uint32      tcrc;                   /* temp transmit CRC */

   /*------------------------------*/
   /*   UART specific parameter RAM  */
   /*------------------------------*/

   cyg_uint8       RESERVED6[8];        /* Reserved area */
   cyg_uint16   max_idl;                   /* maximum idle characters */
   cyg_uint16   idlc;                      /* rx idle counter (internal) */
   cyg_uint16   brkcr;                  /* break count register */

   cyg_uint16   parec;                  /* Rx parity error counter */
   cyg_uint16   frmec;                  /* Rx framing error counter */
   cyg_uint16   nosec;                  /* Rx noise counter */
   cyg_uint16   brkec;                  /* Rx break character counter */
   cyg_uint16   brkln;                  /* Reaceive break length */
                                           
   cyg_uint16   uaddr1;                 /* address character 1 */
   cyg_uint16   uaddr2;                 /* address character 2 */
   cyg_uint16   rtemp;                  /* temp storage */
   cyg_uint16   toseq;                  /* Tx out of sequence char */
   cyg_uint16   cc[8];                  /* Rx control characters */
   cyg_uint16   rccm;                      /* Rx control char mask */
   cyg_uint16   rccr;                      /* Rx control char register */
   cyg_uint16   rlbc;                      /* Receive last break char */
};


/*---------------------------------------------------------------------------
*                    BISYNC parameter RAM (SCC)
*--------------------------------------------------------------------------*/

struct bisync_pram 

{
   /*-------------------*/
   /* SCC parameter RAM */
   /*-------------------*/

   cyg_uint16   rbase;          /* RX BD base address */
   cyg_uint16   tbase;          /* TX BD base address */
   cyg_uint8       rfcr;                   /* Rx function code */
   cyg_uint8       tfcr;                   /* Tx function code */
   cyg_uint16   mrblr;          /* Rx buffer length */
   cyg_uint32      rstate;              /* Rx internal state */
   cyg_uint32      rptr;                   /* Rx internal data pointer */
   cyg_uint16   rbptr;          /* rb BD Pointer */
   cyg_uint16   rcount;         /* Rx internal byte count */
   cyg_uint32      rtemp;               /* Rx temp */
   cyg_uint32      tstate;              /* Tx internal state */
   cyg_uint32      tptr;                   /* Tx internal data pointer */
   cyg_uint16   tbptr;          /* Tx BD pointer */
   cyg_uint16   tcount;         /* Tx byte count */
   cyg_uint32      ttemp;               /* Tx temp */
   cyg_uint32      rcrc;                   /* temp receive CRC */
   cyg_uint32      tcrc;                   /* temp transmit CRC */

   /*--------------------------------*/
   /*   BISYNC specific parameter RAM */
   /*--------------------------------*/

   cyg_uint8       RESERVED7[4];        /* Reserved area */
   cyg_uint32      crcc;                           /* CRC Constant Temp Value */
   cyg_uint16   prcrc;                  /* Preset Receiver CRC-16/LRC */
   cyg_uint16   ptcrc;                  /* Preset Transmitter CRC-16/LRC */
   cyg_uint16   parec;                  /* Receive Parity Error Counter */
   cyg_uint16   bsync;                  /* BISYNC SYNC Character */
   cyg_uint16   bdle;                      /* BISYNC DLE Character */
   cyg_uint16   cc[8];                  /* Rx control characters */
   cyg_uint16   rccm;                      /* Receive Control Character Mask */
};


/*-------------------------------------------------------------------------*/
/*             Transparent mode parameter RAM (SCC)                                                */
/*-------------------------------------------------------------------------*/

struct transparent_pram 

{
   /*--------------------*/
   /*   SCC parameter RAM */
   /*--------------------*/

   cyg_uint16   rbase;          /* RX BD base address */
   cyg_uint16   tbase;          /* TX BD base address */
   cyg_uint8       rfcr;                   /* Rx function code */
   cyg_uint8       tfcr;                   /* Tx function code */
   cyg_uint16   mrblr;          /* Rx buffer length */
   cyg_uint32      rstate;              /* Rx internal state */
   cyg_uint32      rptr;                   /* Rx internal data pointer */
   cyg_uint16   rbptr;          /* rb BD Pointer */
   cyg_uint16   rcount;         /* Rx internal byte count */
   cyg_uint32      rtemp;               /* Rx temp */
   cyg_uint32      tstate;              /* Tx internal state */
   cyg_uint32      tptr;                   /* Tx internal data pointer */
   cyg_uint16   tbptr;          /* Tx BD pointer */
   cyg_uint16   tcount;         /* Tx byte count */
   cyg_uint32      ttemp;               /* Tx temp */
   cyg_uint32      rcrc;                   /* temp receive CRC */
   cyg_uint32      tcrc;                   /* temp transmit CRC */

   /*-------------------------------------*/
   /*   TRANSPARENT specific parameter RAM */
   /*-------------------------------------*/

   cyg_uint32   crc_p;          /* CRC Preset */
   cyg_uint32   crc_c;          /* CRC constant */
};


/*-------------------------------------------------------------------------*/ 
/*                    Ethernet parameter RAM (SCC)                                                       */
/*-------------------------------------------------------------------------*/

struct ethernet_pram 

{
   /*--------------------*/
   /*   SCC parameter RAM */
   /*--------------------*/

   cyg_uint16   rbase;          /* RX BD base address */
   cyg_uint16   tbase;          /* TX BD base address */
   cyg_uint8       rfcr;                   /* Rx function code */
   cyg_uint8       tfcr;                   /* Tx function code */
   cyg_uint16   mrblr;          /* Rx buffer length */
   cyg_uint32      rstate;              /* Rx internal state */
   cyg_uint32      rptr;                   /* Rx internal data pointer */
   cyg_uint16   rbptr;          /* rb BD Pointer */
   cyg_uint16   rcount;         /* Rx internal byte count */
   cyg_uint32      rtemp;               /* Rx temp */
   cyg_uint32      tstate;              /* Tx internal state */
   cyg_uint32      tptr;                   /* Tx internal data pointer */
   cyg_uint16   tbptr;          /* Tx BD pointer */
   cyg_uint16   tcount;         /* Tx byte count */
   cyg_uint32      ttemp;               /* Tx temp */
   cyg_uint32      rcrc;                   /* temp receive CRC */
   cyg_uint32      tcrc;                   /* temp transmit CRC */

   /*---------------------------------*/
   /*   ETHERNET specific parameter RAM */
   /*---------------------------------*/

   cyg_uint32      c_pres;                      /* preset CRC */
   cyg_uint32      c_mask;                      /* constant mask for CRC */
   cyg_uint32      crcec;                       /* CRC error counter */
   cyg_uint32      alec;                           /* alighnment error counter */
   cyg_uint32      disfc;                       /* discard frame counter */
   cyg_uint16   pads;                      /* short frame PAD characters */
   cyg_uint16   ret_lim;                   /* retry limit threshold */
   cyg_uint16   ret_cnt;                   /* retry limit counter */
   cyg_uint16   mflr;                      /* maximum frame length reg */
   cyg_uint16   minflr;                 /* minimum frame length reg */
   cyg_uint16   maxd1;                  /* maximum DMA1 length reg */
   cyg_uint16   maxd2;                  /* maximum DMA2 length reg */
   cyg_uint16   maxd;                      /* rx max DMA */
   cyg_uint16   dma_cnt;                   /* rx dma counter */
   cyg_uint16   max_b;                  /* max bd byte count */
   cyg_uint16   gaddr1;                 /* group address filter 1 */
   cyg_uint16   gaddr2;                 /* group address filter 2 */
   cyg_uint16   gaddr3;                 /* group address filter 3 */
   cyg_uint16   gaddr4;                 /* group address filter 4 */
   cyg_uint32      tbuf0_data0; /* save area 0 - current frm */
   cyg_uint32      tbuf0_data1; /* save area 1 - current frm */
   cyg_uint32      tbuf0_rba0;
   cyg_uint32      tbuf0_crc;
   cyg_uint16   tbuf0_bcnt;
   cyg_uint16   paddr_h;                   /* physical address (MSB) */
   cyg_uint16   paddr_m;                   /* physical address */
   cyg_uint16   paddr_l;                   /* physical address (LSB) */
   cyg_uint16   p_per;                  /* persistence */
   cyg_uint16   rfbd_ptr;               /* rx first bd pointer */
   cyg_uint16   tfbd_ptr;               /* tx first bd pointer */
   cyg_uint16   tlbd_ptr;               /* tx last bd pointer */
   cyg_uint32      tbuf1_data0; /* save area 0 - next frame */
   cyg_uint32      tbuf1_data1; /* save area 1 - next frame */
   cyg_uint32      tbuf1_rba0;
   cyg_uint32      tbuf1_crc;
   cyg_uint16   tbuf1_bcnt;
   cyg_uint16   tx_len;                 /* tx frame length counter */
   cyg_uint16   iaddr1;                 /* individual address filter 1*/
   cyg_uint16   iaddr2;                 /* individual address filter 2*/
   cyg_uint16   iaddr3;                 /* individual address filter 3*/
   cyg_uint16   iaddr4;                 /* individual address filter 4*/
   cyg_uint16   boff_cnt;               /* back-off counter */
   cyg_uint16   taddr_h;                   /* temp address (MSB) */
   cyg_uint16   taddr_m;                   /* temp address */
   cyg_uint16   taddr_l;                   /* temp address (LSB) */
};


/*------------------------------------------------------------------*/
/*                          QMC  definitions                                       */
/*------------------------------------------------------------------*/


struct global_qmc_pram {

    cyg_uint32   mcbase;                /* Multichannel Base pointer */
    cyg_uint16  qmcstate;       /* Multichannel Controller state */
    cyg_uint16  mrblr;          /* Maximum Receive Buffer Length */
    cyg_uint16  tx_s_ptr;   /* TSATTx Pointer */
    cyg_uint16  rxptr;     /* Current Time slot entry in TSATRx */
    cyg_uint16  grfthr;         /* Global Receive frame threshold */
    cyg_uint16  grfcnt;         /* Global Receive Frame Count */
    cyg_uint32   intbase;               /* Multichannel Base address */
    cyg_uint32   intptr;                /* Pointer to interrupt queue */
    cyg_uint16  rx_s_ptr;   /* TSATRx Pointer */
    cyg_uint16  txptr;     /* Current Time slot entry in TSATTx */
    cyg_uint32   c_mask32;      /* CRC Constant (debb20e3) */
    cyg_uint16  tsatrx[32];     /* Time Slot Assignment Table Rx */
    cyg_uint16  tsattx[32];     /* Time Slot Assignment Table Tx */
    cyg_uint16  c_mask16;       /* CRC Constant (f0b8) */

};


   /*------------------------------------------*/
   /* QMC HDLC channel specific parameter RAM  */
   /*------------------------------------------*/

struct qmc_hdlc_pram {

    cyg_uint16  tbase;  /* Tx Buffer Descriptors Base Address */
    cyg_uint16  chamr;  /* Channel Mode Register */
    cyg_uint32   tstate;        /* Tx Internal State */
    cyg_uint32   txintr;        /* Tx Internal Data Pointer */
    cyg_uint16  tbptr;  /* Tx Buffer Descriptor Pointer */
    cyg_uint16  txcntr; /* Tx Internal Byte Count */
    cyg_uint32   tupack;        /* (Tx Temp) */
    cyg_uint32   zistate;       /* Zero Insertion machine state */
    cyg_uint32   tcrc;          /* Temp Transmit CRC */
    cyg_uint16  intmsk; /* Channel's interrupt mask flags */
    cyg_uint16  bdflags;                
    cyg_uint16  rbase;  /* Rx Buffer Descriptors Base Address */
    cyg_uint16  mflr;           /* Max Frame Length Register */
    cyg_uint32   rstate;        /* Rx Internal State */
    cyg_uint32   rxintr;        /* Rx Internal Data Pointer */
    cyg_uint16  rbptr;  /* Rx Buffer Descriptor Pointer */
    cyg_uint16  rxbyc;  /* Rx Internal Byte Count */
    cyg_uint32   rpack; /* (Rx Temp) */
    cyg_uint32   zdstate;       /* Zero Deletion machine state */
    cyg_uint32   rcrc;          /* Temp Transmit CRC */
    cyg_uint16  maxc;           /* Max_length counter */
    cyg_uint16  tmp_mb; /* Temp */
};


         /*-------------------------------------------------*/
         /* QMC Transparent channel specific parameter RAM  */
         /*-------------------------------------------------*/

struct qmc_tran_pram {
    
        cyg_uint16      tbase;          /* Tx Bufer Descriptors Base Address */
        cyg_uint16      chamr;          /* Channel Mode Register */
        cyg_uint32   tstate;            /* Tx Internal State */
    cyg_uint32   txintr;                /* Tx Internal Data Pointer */
    cyg_uint16  tbptr;          /* Tx Buffer Descriptor Pointer */
    cyg_uint16  txcntr;         /* Tx Internal Byte Count */
    cyg_uint32   tupack;                /* (Tx Temp) */
    cyg_uint32   zistate;               /* Zero Insertion machine state */
    cyg_uint32   RESERVED8;     
    cyg_uint16  intmsk;         /* Channel's interrupt mask flags */
    cyg_uint16  bdflags;                
    cyg_uint16  rbase;          /* Rx Buffer Descriptors Base Address */
    cyg_uint16  tmrblr;         /* Max receive buffer length */
    cyg_uint32   rstate;                /* Rx Internal State */
    cyg_uint32   rxintr;                /* Rx Internal Data Pointer */
    cyg_uint16  rbptr;          /* Rx Buffer Descriptor Pointer */
    cyg_uint16  rxbyc;          /* Rx Internal Byte Count */
    cyg_uint32   rpack;         /* (Rx Temp) */
    cyg_uint32   zdstate;               /* Zero Deletion machine state */
    cyg_uint32   RESERVED9;     /* Temp Transmit CRC */
    cyg_uint16  trnsync;                /* Max_length counter */
    cyg_uint16  RESERVED10;     /* Temp */

};

/*----------------------------------------------------------*/
/* allows multiprotocol array declaration in the memory map */
/*----------------------------------------------------------*/

struct qmc_chan_pram
{
        union
        {
                struct qmc_hdlc_pram h;
                struct qmc_tran_pram t;
        }h_or_t;
};



/*--------------------------------------------------------------------*/
/*                          SMC UART parameter RAM                            */
/*--------------------------------------------------------------------*/

struct smc_uart_pram 

{
   cyg_uint16   rbase;          /* Rx BD Base Address */
   cyg_uint16   tbase;          /* Tx BD Base Address */
   cyg_uint8       rfcr;                   /* Rx function code */
   cyg_uint8       tfcr;                   /* Tx function code */
   cyg_uint16   mrblr;          /* Rx buffer length */
   cyg_uint32      rstate;              /* Rx internal state */
   cyg_uint32      rptr;                   /* Rx internal data pointer */
   cyg_uint16   rbptr;          /* rb BD Pointer */
   cyg_uint16   rcount;         /* Rx internal byte count */
   cyg_uint32      rtemp;               /* Rx temp */
   cyg_uint32      tstate;              /* Tx internal state */
   cyg_uint32      tptr;                   /* Tx internal data pointer */
   cyg_uint16   tbptr;          /* Tx BD pointer */
   cyg_uint16   tcount;         /* Tx byte count */
   cyg_uint32      ttemp;               /* Tx temp */
   cyg_uint16   max_idl;           /* Maximum IDLE Characters */
   cyg_uint16   idlc;              /* Temporary IDLE Counter */
   cyg_uint16   brkln;          /* Last Rx Break Length */
   cyg_uint16   brkec;          /* Rx Break Condition Counter */
   cyg_uint16   brkcr;          /* Break Count Register (Tx) */
   cyg_uint16   r_mask;         /* Temporary bit mask */
};


/*--------------------------------------------------------------------------*/
/*                  SMC Transparent mode parameter RAM                                              */
/*--------------------------------------------------------------------------*/

struct smc_trnsp_pram 

{
   cyg_uint16   rbase;                  /* Rx BD Base Address */
   cyg_uint16   tbase;                  /* Tx BD Base Address */
   cyg_uint8       rfcr;                           /* Rx function code */
   cyg_uint8       tfcr;                           /* Tx function code */
   cyg_uint16   mrblr;                  /* Rx buffer length */
   cyg_uint32      rstate;                      /* Rx internal state */
   cyg_uint32      rptr;                           /* Rx internal data pointer */
   cyg_uint16   rbptr;                  /* rb BD Pointer */
   cyg_uint16   rcount;                 /* Rx internal byte count */
   cyg_uint32      rtemp;                       /* Rx temp */
   cyg_uint32      tstate;                      /* Tx internal state */
   cyg_uint32      tptr;                           /* Tx internal data pointer */
   cyg_uint16   tbptr;                  /* Tx BD pointer */
   cyg_uint16   tcount;                 /* Tx byte count */
   cyg_uint32      ttemp;                       /* Tx temp */
   cyg_uint16   RESERVED11[5];  /* Reserved */
};


/*--------------------------------------------------------------------------*/
/*                      SPI parameter RAM                                                                                 */
/*--------------------------------------------------------------------------*/

#define SPI_R   0x8000          /* Ready bit in BD */

struct spi_pram 

{
   cyg_uint16   rbase;          /* Rx BD Base Address */
   cyg_uint16   tbase;          /* Tx BD Base Address */
   cyg_uint8       rfcr;                   /* Rx function code */
   cyg_uint8       tfcr;                   /* Tx function code */
   cyg_uint16   mrblr;          /* Rx buffer length */
   cyg_uint32      rstate;              /* Rx internal state */
   cyg_uint32      rptr;                   /* Rx internal data pointer */
   cyg_uint16   rbptr;          /* rb BD Pointer */
   cyg_uint16   rcount;         /* Rx internal byte count */
   cyg_uint32      rtemp;               /* Rx temp */
   cyg_uint32      tstate;              /* Tx internal state */
   cyg_uint32      tptr;                   /* Tx internal data pointer */
   cyg_uint16   tbptr;          /* Tx BD pointer */
   cyg_uint16   tcount;         /* Tx byte count */
   cyg_uint32      ttemp;               /* Tx temp */

    cyg_uint8     RESERVED12[8]; /* Reserved */
};


/*--------------------------------------------------------------------------*/
/*                       I2C parameter RAM                                                                             */
/*--------------------------------------------------------------------------*/

struct i2c_pram 

{
   /*--------------------*/
   /*   I2C parameter RAM */
   /*--------------------*/

   cyg_uint16   rbase;          /* RX BD base address */
   cyg_uint16   tbase;          /* TX BD base address */
   cyg_uint8       rfcr;                   /* Rx function code */
   cyg_uint8       tfcr;                   /* Tx function code */
   cyg_uint16   mrblr;          /* Rx buffer length */
   cyg_uint32      rstate;              /* Rx internal state */
   cyg_uint32      rptr;                   /* Rx internal data pointer */
   cyg_uint16   rbptr;          /* rb BD Pointer */
   cyg_uint16   rcount;         /* Rx internal byte count */
   cyg_uint32      rtemp;               /* Rx temp */
   cyg_uint32      tstate;              /* Tx internal state */
   cyg_uint32      tptr;                   /* Tx internal data pointer */
   cyg_uint16   tbptr;          /* Tx BD pointer */
   cyg_uint16   tcount;         /* Tx byte count */
   cyg_uint32      ttemp;               /* Tx temp */

    cyg_uint8     RESERVED13[8];    
};

/*--------------------------------------------------------------------------*/
/*                       MISC parameter RAM                                                                             */
/*--------------------------------------------------------------------------*/

struct misc_pram 
{
    cyg_uint8    RESERVED14[16];
};



/*--------------------------------------------------------------------------*/
/*                      PIP Centronics parameter RAM                                                   */
/*--------------------------------------------------------------------------*/

struct centronics_pram 

{
   cyg_uint16   rbase;          /* Rx BD Base Address */
   cyg_uint16   tbase;          /* Tx BD Base Address */
   cyg_uint8       fcr;            /* function code */
   cyg_uint8       smask;               /* Status Mask */
   cyg_uint16   mrblr;          /* Rx buffer length */
   cyg_uint32      rstate;              /* Rx internal state */
   cyg_uint32      rptr;                   /* Rx internal data pointer */
   cyg_uint16   rbptr;          /* rb BD Pointer */
   cyg_uint16   rcount;         /* Rx internal byte count */
   cyg_uint32      rtemp;               /* Rx temp */
   cyg_uint32      tstate;              /* Tx internal state */
   cyg_uint32      tptr;                   /* Tx internal data pointer */
   cyg_uint16   tbptr;          /* Tx BD pointer */
   cyg_uint16   tcount;         /* Tx byte count */
   cyg_uint32      ttemp;               /* Tx temp */
   cyg_uint16   max_sl;         /* Maximum Silence period */
   cyg_uint16   sl_cnt;         /* Silence Counter */
   cyg_uint16   char1;          /* CONTROL char 1 */
   cyg_uint16   char2;          /* CONTROL char 2 */
   cyg_uint16   char3;          /* CONTROL char 3 */
   cyg_uint16   char4;          /* CONTROL char 4 */
   cyg_uint16   char5;          /* CONTROL char 5 */
   cyg_uint16   char6;          /* CONTROL char 6 */
   cyg_uint16   char7;          /* CONTROL char 7 */
   cyg_uint16   char8;          /* CONTROL char 8 */
   cyg_uint16   rccm;              /* Rx Control Char Mask */
   cyg_uint16   rccr;              /* Rx Char Control Register */
};


/*--------------------------------------------------------------------------*/
/*                                                      IDMA parameter RAM                                                                           */
/*--------------------------------------------------------------------------*/

struct idma_pram 

{
   cyg_uint16 ibase;        /* IDMA BD Base Address */
   cyg_uint16 dcmr;       /* DMA Channel Mode Register */
   cyg_uint32  sapr;       /* Source Internal Data Pointer */
   cyg_uint32  dapr;       /* Destination Internal Data Pointer */
   cyg_uint16 ibptr;      /* Buffer Descriptor Pointer */
   cyg_uint16 write_sp;   /* No description given in manual */
   cyg_uint32  s_byte_c;   /* Internal Source Byte Count */
   cyg_uint32  d_byte_c;   /* Internal Destination Byte Count */
   cyg_uint32  s_state;    /* Internal State */
   cyg_uint32  itemp0;     /* Temp Data Storage */
   cyg_uint32  itemp1;     /* Temp Data Storage */
   cyg_uint32  itemp2;     /* Temp Data Storage */
   cyg_uint32  itemp3;     /* Temp Data Storage */
   cyg_uint32  sr_mem;     /* Data Storage for Peripherial Write */
   cyg_uint16 read_sp;    /* No description given in manual */
   cyg_uint16 nodesc0;    /* Diff Between Source and Destination Residue*/
   cyg_uint16 nodesc1;    /* Temp Storage Address Pointer */
   cyg_uint16 nodesc2;    /* SR_MEM Byte Count */
   cyg_uint32  d_state;    /* Internal State */
};



/*--------------------------------------------------------------------------*/
/*                                      RISC timer parameter RAM                                                                     */
/*--------------------------------------------------------------------------*/

struct timer_pram 

{
   /*----------------------------*/
   /*   RISC timers parameter RAM */
   /*----------------------------*/

   cyg_uint16   tm_base;           /* RISC timer table base adr */
   cyg_uint16   tm_ptr;         /* RISC timer table pointer */
   cyg_uint16   r_tmr;          /* RISC timer mode register */
   cyg_uint16   r_tmv;          /* RISC timer valid register */
   cyg_uint32      tm_cmd;              /* RISC timer cmd register */
   cyg_uint32      tm_cnt;              /* RISC timer internal cnt */
};


/*--------------------------------------------------------------------------*/
/*                                              ROM Microcode parameter RAM                                                               */
/*--------------------------------------------------------------------------*/

struct ucode_pram 

{
   /*---------------------------*/
   /*   RISC ucode parameter RAM */
   /*---------------------------*/

   cyg_uint16   rev_num;    /* Ucode Revision Number */
   cyg_uint16   d_ptr;          /* MISC Dump area pointer */
   cyg_uint32      temp1;               /* MISC Temp1 */
   cyg_uint32      temp2;               /* MISC Temp2 */
};


/*---------------------------------------------------------------------------*/
/* Example structuring of user data area of memory at 0x2000 (base of DPRAM) */
/* Note that this area can also be used by microcodes and the QMC channel        */
/* specific parameter ram.                                                                                                       */
/*---------------------------------------------------------------------------*/

struct user_data

{

   volatile cyg_uint8   udata_bd_ucode[0x200];  /* user data bd's or Ucode (small)  */
   volatile cyg_uint8   udata_bd_ucode2[0x200]; /* user data bd's or Ucode (medium) */
   volatile cyg_uint8   udata_bd_ucode3[0x400]; /* user data bd's or Ucode (large)  */
   volatile cyg_uint8   udata_bd[0x700];                        /* user data bd's*/
   volatile cyg_uint8   ucode_ext[0x100];                       /* Ucode Extension ram*/
   volatile cyg_uint8   RESERVED12[0x0c00];             /* Reserved area */

};



/***************************************************************************/
/*                                                                                                                                                                   */
/*      Definitions of Embedded PowerPC (EPPC) internal memory structures,         */
/*  including registers and dual-port RAM                                                                           */
/*                                                                                                                                                                   */
/***************************************************************************/

typedef struct eppc 

{
   /*-----------------------------------*/
   /* BASE + 0x0000: INTERNAL REGISTERS */
   /*-----------------------------------*/

   /*-----*/
   /* SIU */
   /*-----*/

   volatile cyg_uint32  siu_mcr;                      /* module configuration reg */
   volatile cyg_uint32  siu_sypcr;                 /* System protection cnt */
   cyg_uint8    RESERVED13[0x6];
   volatile cyg_uint16  siu_swsr;                  /* sw service */
   volatile cyg_uint32  siu_sipend;                /* Interrupt pend reg */
   volatile cyg_uint32  siu_simask;                /* Interrupt mask reg */
   volatile cyg_uint32  siu_siel;                  /* Interrupt edge level mask reg */
   volatile cyg_uint32  siu_sivec;                 /* Interrupt vector */
   volatile cyg_uint32  siu_tesr;                  /* Transfer error status */
   volatile cyg_uint8   RESERVED14[0xc];  /* Reserved area */
   volatile cyg_uint32  dma_sdcr;                  /* SDMA configuration reg */
   cyg_uint8    RESERVED15[0x4c];

   /*--------*/
   /* PCMCIA */
   /*--------*/

   volatile cyg_uint32  pcmcia_pbr0;      /* PCMCIA Base Reg: Window 0 */
   volatile cyg_uint32  pcmcia_por0;      /* PCMCIA Option Reg: Window 0 */
   volatile cyg_uint32  pcmcia_pbr1;      /* PCMCIA Base Reg: Window 1 */
   volatile cyg_uint32  pcmcia_por1;      /* PCMCIA Option Reg: Window 1 */
   volatile cyg_uint32  pcmcia_pbr2;      /* PCMCIA Base Reg: Window 2 */
   volatile cyg_uint32  pcmcia_por2;      /* PCMCIA Option Reg: Window 2 */
   volatile cyg_uint32  pcmcia_pbr3;      /* PCMCIA Base Reg: Window 3 */
   volatile cyg_uint32  pcmcia_por3;      /* PCMCIA Option Reg: Window 3 */
   volatile cyg_uint32  pcmcia_pbr4;      /* PCMCIA Base Reg: Window 4 */
   volatile cyg_uint32  pcmcia_por4;      /* PCMCIA Option Reg: Window 4 */
   volatile cyg_uint32  pcmcia_pbr5;      /* PCMCIA Base Reg: Window 5 */
   volatile cyg_uint32  pcmcia_por5;      /* PCMCIA Option Reg: Window 5 */
   volatile cyg_uint32  pcmcia_pbr6;      /* PCMCIA Base Reg: Window 6 */
   volatile cyg_uint32  pcmcia_por6;      /* PCMCIA Option Reg: Window 6 */
   volatile cyg_uint32  pcmcia_pbr7;      /* PCMCIA Base Reg: Window 7 */
   volatile cyg_uint32  pcmcia_por7;      /* PCMCIA Option Reg: Window 7 */
   volatile cyg_uint8  RESERVED16[0x20];  /* Reserved area */
   volatile cyg_uint32  pcmcia_pgcra;     /* PCMCIA Slot A Control  Reg */
   volatile cyg_uint32  pcmcia_pgcrb;     /* PCMCIA Slot B Control  Reg */
   volatile cyg_uint32  pcmcia_pscr;      /* PCMCIA Status Reg */
   volatile cyg_uint8  RESERVED17[0x4];  /* Reserved area */
   volatile cyg_uint32  pcmcia_pipr;      /* PCMCIA Pins Value Reg */
   volatile cyg_uint8  RESERVED18[0x4];  /* Reserved area */
   volatile cyg_uint32  pcmcia_per;       /* PCMCIA Enable Reg */
   volatile cyg_uint8  RESERVED19[0x4];  /* Reserved area */

   /*------*/
   /* MEMC */
   /*------*/

   volatile cyg_uint32  memc_br0;                       /* base register 0 */
   volatile cyg_uint32  memc_or0;                       /* option register 0 */
   volatile cyg_uint32  memc_br1;                       /* base register 1 */
   volatile cyg_uint32  memc_or1;                       /* option register 1 */
   volatile cyg_uint32  memc_br2;                       /* base register 2 */
   volatile cyg_uint32  memc_or2;                       /* option register 2 */
   volatile cyg_uint32  memc_br3;                       /* base register 3 */
   volatile cyg_uint32  memc_or3;                       /* option register 3 */
   volatile cyg_uint32  memc_br4;                       /* base register 3 */
   volatile cyg_uint32  memc_or4;                       /* option register 3 */
   volatile cyg_uint32  memc_br5;                       /* base register 3 */
   volatile cyg_uint32  memc_or5;                       /* option register 3 */
   volatile cyg_uint32  memc_br6;                       /* base register 3 */
   volatile cyg_uint32  memc_or6;                       /* option register 3 */
   volatile cyg_uint32  memc_br7;                       /* base register 3 */
   volatile cyg_uint32  memc_or7;                       /* option register 3 */
   volatile cyg_uint8   RESERVED20[0x24];       /* Reserved area */
   volatile cyg_uint32  memc_mar;                       /* Memory address */
   volatile cyg_uint32  memc_mcr;                       /* Memory command */
   volatile cyg_uint8   RESERVED21[0x4];        /* Reserved area */
   volatile cyg_uint32  memc_mamr;                      /* Machine A mode */
   volatile cyg_uint32  memc_mbmr;                      /* Machine B mode */
   volatile cyg_uint16  memc_mstat;                     /* Memory status */
   volatile cyg_uint16  memc_mptpr;                     /* Memory preidic timer prescalar */
   volatile cyg_uint32  memc_mdr;                       /* Memory data */
   volatile cyg_uint8   RESERVED22[0x80];       /* Reserved area */

   /*---------------------------*/
   /* SYSTEM INTEGRATION TIMERS */
   /*---------------------------*/

   volatile cyg_uint16  simt_tbscr;                     /* Time base stat&ctr */
   volatile cyg_uint8   RESERVED23[0x2];        /* Reserved area */
   volatile cyg_uint32  simt_tbreff0;           /* Time base reference 0 */
   volatile cyg_uint32  simt_tbreff1;           /* Time base reference 1 */
   volatile cyg_uint8   RESERVED24[0x14];       /* Reserved area */
   volatile cyg_uint16  simt_rtcsc;                     /* Realtime clk stat&cntr 1 */
   volatile cyg_uint8   RESERVED25[0x2];        /* Reserved area */
   volatile cyg_uint32  simt_rtc;                       /* Realtime clock */
   volatile cyg_uint32  simt_rtsec;                     /* Realtime alarm seconds */
   volatile cyg_uint32  simt_rtcal;                     /* Realtime alarm */
   volatile cyg_uint8   RESERVED26[0x10];       /* Reserved area */
   volatile cyg_uint32  simt_piscr;                     /* PIT stat&ctrl */
   volatile cyg_uint32  simt_pitc;                      /* PIT counter */
   volatile cyg_uint32  simt_pitr;                      /* PIT */
   volatile cyg_uint8   RESERVED27[0x34];       /* Reserved area */

   /*---------------*/
   /* CLOCKS, RESET */
   /*---------------*/
   
   volatile cyg_uint32  clkr_sccr;                      /* System clk cntrl */
   volatile cyg_uint32  clkr_plprcr;            /* PLL reset&ctrl */
   volatile cyg_uint32  clkr_rsr;                       /* reset status */
   cyg_uint8    RESERVED28[0x74];       /* Reserved area */

   /*--------------------------------*/
   /* System Integration Timers Keys */
   /*--------------------------------*/

   volatile cyg_uint32  simt_tbscrk;            /* Timebase Status&Ctrl Key */
   volatile cyg_uint32  simt_tbreff0k;          /* Timebase Reference 0 Key */
   volatile cyg_uint32  simt_tbreff1k;          /* Timebase Reference 1 Key */
   volatile cyg_uint32  simt_tbk;               /* Timebase and Decrementer Key */
   cyg_uint8   RESERVED29[0x10];        /* Reserved area */
   volatile cyg_uint32  simt_rtcsck;            /* Real-Time Clock Status&Ctrl Key */

   volatile cyg_uint32  simt_rtck;              /* Real-Time Clock Key */
   volatile cyg_uint32  simt_rtseck;            /* Real-Time Alarm Seconds Key */
   volatile cyg_uint32  simt_rtcalk;            /* Real-Time Alarm Key */
   cyg_uint8   RESERVED30[0x10];        /* Reserved area */
   volatile cyg_uint32  simt_piscrk;            /* Periodic Interrupt Status&Ctrl Key */
   volatile cyg_uint32  simt_pitck;             /* Periodic Interrupt Count Key */
   cyg_uint8   RESERVED31[0x38];        /* Reserved area */
        
   /*----------------------*/
   /* Clock and Reset Keys */
   /*----------------------*/

   volatile cyg_uint32  clkr_sccrk;           /* System Clock Control Key */
   volatile cyg_uint32  clkr_plprcrk;         /* PLL, Low Power and Reset Control Key */
   volatile cyg_uint32  clkr_rsrk;                 /* Reset Status Key */
   cyg_uint8      RESERVED32[0x4d4];    /* Reserved area */
              
   /*-----*/
   /* I2C */
   /*-----*/
   
   volatile cyg_uint8   i2c_i2mod;                      /* i2c mode */
   cyg_uint8            RESERVED33[3];
   volatile cyg_uint8   i2c_i2add;                      /* i2c address */
   cyg_uint8            RESERVED34[3];
   volatile cyg_uint8   i2c_i2brg;                      /* i2c brg */
   cyg_uint8            RESERVED35[3];
   volatile cyg_uint8   i2c_i2com;                      /* i2c command */
   cyg_uint8            RESERVED36[3];
   volatile cyg_uint8   i2c_i2cer;                      /* i2c event */
   cyg_uint8            RESERVED37[3];
   volatile cyg_uint8   i2c_i2cmr;                      /* i2c mask */
   volatile cyg_uint8   RESERVED38[0x8b];       /* Reserved area */

   /*-----*/
   /* DMA */
   /*-----*/

   volatile cyg_uint8   RESERVED39[0x4];        /* Reserved area */
   volatile cyg_uint32  dma_sdar;                       /* SDMA address reg */
   volatile cyg_uint8   RESERVED40[0x2];        /* Reserved area */
   volatile cyg_uint8   dma_sdsr;                       /* SDMA status reg */
   volatile cyg_uint8   RESERVED41[0x3];        /* Reserved area */
   volatile cyg_uint8   dma_sdmr;                       /* SDMA mask reg */
   volatile cyg_uint8   RESERVED42[0x1];        /* Reserved area */                        
   volatile cyg_uint8   dma_idsr1;                      /* IDMA1 status reg */
   volatile cyg_uint8   RESERVED43[0x3];        /* Reserved area */
   volatile cyg_uint8   dma_idmr1;                      /* IDMA1 mask reg */
   volatile cyg_uint8   RESERVED44[0x3];        /* Reserved area */
   volatile cyg_uint8   dma_idsr2;                      /* IDMA2 status reg */
   volatile cyg_uint8   RESERVED45[0x3];        /* Reserved area */
   volatile cyg_uint8   dma_idmr2;                      /* IDMA2 mask reg */
   volatile cyg_uint8   RESERVED46[0x13];       /* Reserved area */

   /*--------------------------*/
   /* CPM Interrupt Controller */
   /*--------------------------*/

   volatile cyg_uint16  cpmi_civr;                      /* CP interrupt vector reg */
   volatile cyg_uint8   RESERVED47[0xe];        /* Reserved area */
   volatile cyg_uint32  cpmi_cicr;                      /* CP interrupt configuration reg */
   volatile cyg_uint32  cpmi_cipr;                      /* CP interrupt pending reg */
   volatile cyg_uint32  cpmi_cimr;                      /* CP interrupt mask reg */
   volatile cyg_uint32  cpmi_cisr;                      /* CP interrupt in-service reg */

   /*----------*/
   /* I/O port */
   /*----------*/

   volatile cyg_uint16  pio_padir;                      /* port A data direction reg */
   volatile cyg_uint16  pio_papar;                      /* port A pin assignment reg */
   volatile cyg_uint16  pio_paodr;                      /* port A open drain reg */
   volatile cyg_uint16  pio_padat;                      /* port A data register */
   volatile cyg_uint8   RESERVED48[0x8];        /* Reserved area */
   volatile cyg_uint16  pio_pcdir;                      /* port C data direction reg */
   volatile cyg_uint16  pio_pcpar;                      /* port C pin assignment reg */
   volatile cyg_uint16  pio_pcso;                       /* port C special options */
   volatile cyg_uint16  pio_pcdat;                      /* port C data register */
   volatile cyg_uint16  pio_pcint;                      /* port C interrupt cntrl reg */
   cyg_uint8            RESERVED49[6];
   volatile cyg_uint16  pio_pddir;                      /* port D Data Direction reg */
   volatile cyg_uint16  pio_pdpar;                      /* port D pin assignment reg */
   cyg_uint8            RESERVED50[2];
   volatile cyg_uint16  pio_pddat;                      /* port D data reg */
   volatile cyg_uint8   RESERVED51[0x8];        /* Reserved area */

   /*-----------*/
   /* CPM Timer */
   /*-----------*/

   volatile cyg_uint16  timer_tgcr;                     /* timer global configuration reg */
   volatile cyg_uint8   RESERVED52[0xe];        /* Reserved area */
   volatile cyg_uint16  timer_tmr1;                     /* timer 1 mode reg */
   volatile cyg_uint16  timer_tmr2;                     /* timer 2 mode reg */
   volatile cyg_uint16  timer_trr1;                     /* timer 1 referance reg */
   volatile cyg_uint16  timer_trr2;                     /* timer 2 referance reg */
   volatile cyg_uint16  timer_tcr1;                     /* timer 1 capture reg */
   volatile cyg_uint16  timer_tcr2;                     /* timer 2 capture reg */
   volatile cyg_uint16  timer_tcn1;                     /* timer 1 counter reg */
   volatile cyg_uint16  timer_tcn2;                     /* timer 2 counter reg */
   volatile cyg_uint16  timer_tmr3;                     /* timer 3 mode reg */
   volatile cyg_uint16  timer_tmr4;                     /* timer 4 mode reg */
   volatile cyg_uint16  timer_trr3;                     /* timer 3 referance reg */
   volatile cyg_uint16  timer_trr4;                     /* timer 4 referance reg */
   volatile cyg_uint16  timer_tcr3;                     /* timer 3 capture reg */
   volatile cyg_uint16  timer_tcr4;                     /* timer 4 capture reg */
   volatile cyg_uint16  timer_tcn3;                     /* timer 3 counter reg */
   volatile cyg_uint16  timer_tcn4;                     /* timer 4 counter reg */
   volatile cyg_uint16  timer_ter1;                     /* timer 1 event reg */
   volatile cyg_uint16  timer_ter2;                     /* timer 2 event reg */
   volatile cyg_uint16  timer_ter3;                     /* timer 3 event reg */
   volatile cyg_uint16  timer_ter4;                     /* timer 4 event reg */
   volatile cyg_uint8   RESERVED53[0x8];        /* Reserved area */

   /*----*/
   /* CP */
   /*----*/

   volatile cyg_uint16  cp_cr;                          /* command register */
   volatile cyg_uint8   RESERVED54[0x2];        /* Reserved area */
   volatile cyg_uint16  cp_rccr;                                /* main configuration reg */
   volatile cyg_uint8   RESERVED55;                     /* Reserved area */
   volatile cyg_uint8   cp_resv1;                       /* Reserved reg */
   volatile cyg_uint32  cp_resv2;                       /* Reserved reg */
   volatile cyg_uint16  cp_rctr1;                       /* ram break register 1 */
   volatile cyg_uint16  cp_rctr2;                       /* ram break register 2 */
   volatile cyg_uint16  cp_rctr3;                       /* ram break register 3 */
   volatile cyg_uint16  cp_rctr4;                       /* ram break register 4 */
   volatile cyg_uint8   RESERVED56[0x2];        /* Reserved area */
   volatile cyg_uint16  cp_rter;                                /* RISC timers event reg */
   volatile cyg_uint8   RESERVED57[0x2];        /* Reserved area */
   volatile cyg_uint16  cp_rtmr;                                /* RISC timers mask reg */
   volatile cyg_uint8   RESERVED58[0x14];       /* Reserved area */

   /*-----*/
   /* BRG */
   /*-----*/

   volatile cyg_uint32  brgc1;          /* BRG1 configuration reg */
   volatile cyg_uint32  brgc2;          /* BRG2 configuration reg */
   volatile cyg_uint32  brgc3;          /* BRG3 configuration reg */
   volatile cyg_uint32  brgc4;          /* BRG4 configuration reg */

   /*---------------*/
   /* SCC registers */
   /*---------------*/

   struct scc_regs 
   
   {
       volatile cyg_uint32      scc_gsmr_l;             /* SCC Gen mode (LOW) */
       volatile cyg_uint32      scc_gsmr_h;             /* SCC Gen mode (HIGH) */
       volatile cyg_uint16      scc_psmr;                       /* protocol specific mode register */
       volatile cyg_uint8       RESERVED59[0x2];        /* Reserved area */
       volatile cyg_uint16      scc_todr;                       /* SCC transmit on demand */
       volatile cyg_uint16      scc_dsr;                                /* SCC data sync reg */
       volatile cyg_uint16      scc_scce;                       /* SCC event reg */
       volatile cyg_uint8       RESERVED60[0x2];        /* Reserved area */
       volatile cyg_uint16      scc_sccm;                       /* SCC mask reg */
       volatile cyg_uint8       RESERVED61[0x1];        /* Reserved area */
       volatile cyg_uint8       scc_sccs;                       /* SCC status reg */
       volatile cyg_uint8       RESERVED62[0x8];        /* Reserved area */

   } scc_regs[4];

   
   /*-----*/
   /* SMC */
   /*-----*/

   struct smc_regs 
   
   {
       volatile cyg_uint8       RESERVED63[0x2];        /* Reserved area */
       volatile cyg_uint16      smc_smcmr;                      /* SMC mode reg */
       volatile cyg_uint8       RESERVED64[0x2];        /* Reserved area */
       volatile cyg_uint8       smc_smce;                       /* SMC event reg */
       volatile cyg_uint8       RESERVED65[0x3];        /* Reserved area */
       volatile cyg_uint8       smc_smcm;                       /* SMC mask reg */
       volatile cyg_uint8       RESERVED66[0x5];        /* Reserved area */

   } smc_regs[2];


   /*-----*/
   /* SPI */
   /*-----*/

   volatile cyg_uint16  spi_spmode;                     /* SPI mode reg */
   volatile cyg_uint8   RESERVED67[0x4];        /* Reserved area */
   volatile cyg_uint8   spi_spie;                       /* SPI event reg */
   volatile cyg_uint8   RESERVED68[0x3];        /* Reserved area */
   volatile cyg_uint8   spi_spim;                       /* SPI mask reg */
   volatile cyg_uint8   RESERVED69[0x2];        /* Reserved area */
   volatile cyg_uint8   spi_spcom;                      /* SPI command reg */
   volatile cyg_uint8   RESERVED70[0x4];        /* Reserved area */

   /*-----*/
   /* PIP */
   /*-----*/

   volatile cyg_uint16  pip_pipc;                       /* pip configuration reg */
   volatile cyg_uint8   RESERVED71[0x2];        /* Reserved area */
   volatile cyg_uint16  pip_ptpr;                       /* pip timing parameters reg */
   volatile cyg_uint32  pip_pbdir;                      /* port b data direction reg */
   volatile cyg_uint32  pip_pbpar;                      /* port b pin assignment reg */
   volatile cyg_uint8   RESERVED72[0x2];        /* Reserved area */
   volatile cyg_uint16  pip_pbodr;                      /* port b open drain reg */
   volatile cyg_uint32  pip_pbdat;                      /* port b data reg */
   volatile cyg_uint8   RESERVED73[0x18];       /* Reserved area */


   /*------------------*/
   /* Serial Interface */
   /*------------------*/

   volatile cyg_uint32  si_simode;                              /* SI mode register */
   volatile cyg_uint8   si_sigmr;                               /* SI global mode register */
   volatile cyg_uint8   RESERVED74;                     /* Reserved area */
   volatile cyg_uint8   si_sistr;                               /* SI status register */
   volatile cyg_uint8   si_sicmr;                               /* SI command register */
   volatile cyg_uint8   RESERVED75[0x4];                /* Reserved area */
   volatile cyg_uint32  si_sicr;                                        /* SI clock routing */
   volatile cyg_uint32  si_sirp;                                        /* SI ram pointers */
   volatile cyg_uint8   RESERVED76[0x10c];      /* Reserved area */
   volatile cyg_uint8   si_siram[0x200];                /* SI routing ram */
   volatile cyg_uint8   RESERVED77[0x1200];     /* Reserved area */

   /*-----------------------------------------------------------------*/
   /* BASE + 0x2000: user data memory, microcode, or QMC channel PRAM */
   /*-----------------------------------------------------------------*/
        
        union
        {
                struct qmc_chan_pram qcp[64];
                struct user_data ud;
                cyg_uint8 RESERVED[0x1c00];
        } qcp_or_ud;


   /*-----------------------------------------------------------------------*/
   /* BASE + 0x3c00: PARAMETER RAM. This main union defines 4 memory blocks */
   /* of an identical size. See the Parameter RAM definition in the MPC860  */
   /* user's manual.                                                        */
   /*-----------------------------------------------------------------------*/

   /*------------------------*/
   /* Base + 0x3C00 (page 1) */
   /*      + 0x3D00 (page 2) */
   /*      + 0x3E00 (page 3) */
   /*      + 0x3F00 (page 4) */
   /*------------------------*/

   union 
      
   {
      struct page_of_pram 
       
      {
         /*------------------------------------------------------------*/
         /* scc parameter area - 1st memory block (protocol dependent) */
         /*------------------------------------------------------------*/

         union 
           
         {
            struct hdlc_pram          h;
            struct uart_pram          u;
            struct bisync_pram          b;
            struct transparent_pram     t;
            struct async_hdlc_pram      a;
            cyg_uint8   RESERVED78[0x80];

         } scc;         

         /*----------------------------------------------------------------*/
         /* Other protocol areas for the rest of the memory blocks in each */
         /* page.                                                          */
         /*----------------------------------------------------------------*/

         union 
                 
         {
            /*---------------------------------------------------------------*/
            /* This structure defines the rest of the blocks on the 1st page */
            /*---------------------------------------------------------------*/ 
         
            struct 
         
            {
               struct i2c_pram  i2c;     /* I2C   */
               struct misc_pram misc;    /* MISC  */
               struct idma_pram idma1;   /* IDMA1 */

            } i2c_idma; 

            /*---------------------------------------------------------------*/
            /* This structure defines the rest of the blocks on the 2nd page */
            /*---------------------------------------------------------------*/
 
            struct 
         
            {
               struct spi_pram  spi;     /* SPI    */
               struct timer_pram        timer;   /* Timers */
               struct idma_pram idma2;   /* IDMA2  */

            } spi_timer_idma; 

            /*---------------------------------------------------------------*/
            /* This structure defines the rest of the blocks on the 3rd page */
            /*---------------------------------------------------------------*/
 
            struct 
         
            {
               union 
          
               {
                  struct smc_uart_pram u1;   /* SMC1 */
                  struct smc_trnsp_pram t1;  /* SMC1 */
                  cyg_uint8     RESERVED78[0x80];     /* declare full block */

               } psmc1; 

            } smc_dsp1;


            /*---------------------------------------------------------------*/
            /* This structure defines the rest of the blocks on the 4th page */
            /*---------------------------------------------------------------*/ 
 
            struct 
         
            {
               union 
          
               {
                  struct smc_uart_pram u2;   /* SMC2 */
                  struct smc_trnsp_pram t2;  /* SMC2 */
                  struct centronics_pram c;  /* Uses SM2's space */
                  cyg_uint8     RESERVED79[0x80];    /* declare full block */

               } psmc2;

            } smc_dsp2; 

            cyg_uint8   RESERVED80[0x80];    /* declare full block */

         } other;

      } pg;

      /*---------------------------------------------------------------*/
      /* When selecting Ethernet as protocol for an SCC, this protocol */
      /* uses a complete page of Parameter RAM memory.                 */
      /*---------------------------------------------------------------*/

      struct ethernet_pram      enet_scc;

        /*---------------------------------------------------------------*/
        /* When using QMC as a mode for an SCC, the QMC global parameter */
        /* ram uses from SCC BASE to BASE+AC.                            */
        /*---------------------------------------------------------------*/
      
      struct global_qmc_pram  gqp; 

      /*--------------------------------------------------------*/
      /* declaration to guarantee a page of memory is allocated */
      /*--------------------------------------------------------*/

      cyg_uint8   RESERVED83[0x100]; 

   } PRAM[4]; /* end of union */

} EPPC;


/***************************************************************************/
/*                   General Global Definitions                            */
/***************************************************************************/


#define PAGE1           0       /* SCC1 Index into SCC Param RAM Array */
#define PAGE2           1       /* SCC2 Index into SCC Param RAM Array */
#define PAGE3           2       /* SCC3 Index into SCC Param RAM Array */
#define PAGE4           3       /* SCC4 Index into SCC Param RAM Array */

#define SCC1_REG                0       /* SCC1 Index into SCC Regs Array  */                           
#define SCC2_REG                1       /* SCC2 Index into SCC Regs Array  */                           
#define SCC3_REG                2       /* SCC3 Index into SCC Regs Array  */                           
#define SCC4_REG                3       /* SCC4 Index into SCC Regs Array  */                           


/*--------------------------------*/
/* KEEP ALIVE POWER REGISTERS KEY */
/*--------------------------------*/

#define KEEP_ALIVE_KEY 0x55ccaa33


#define SMC2_REG 1          /* SMC Regs Array Index for SMC2 */

/*-------------------------*/
/* Single buffer component */
/*-------------------------*/

typedef struct BufferPool

{
   cyg_uint8  RxBuffer;
   cyg_uint8  TxBuffer;

} LB;


/*--------------------------*/
/* Buffer Descriptor Format */
/*--------------------------*/

typedef struct BufferDescriptor 

{
   cyg_uint16 bd_cstatus;     /* control and status */
   cyg_uint16 bd_length;      /* transfer length */
   cyg_uint8  *bd_addr;       /* buffer address */

} BD;
 

/*-------------------------------*/
/* Buffer Descriptor Ring format */
/*-------------------------------*/

typedef struct BufferDescRings 

{
    volatile BD RxBD;    /* Rx BD ring */
    volatile BD TxBD;    /* Tx BD ring */

} BDRINGS;


#endif /* CYGONCE_HAL_PPC_FADS_PPC_860_H */
