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
/*-------------------------------------------------------------------------
* FILENAME:  smc2.c
*
* DESCRIPTION:   
*
*   The code in this module provides echo capability on SMC2. It's 
*   intent is to provide the beginnings of a debug port that is 
*   mostly compiler independent. If an ASCII terminal is connected
*   at 9600,N,8,1 on Port 2 (PB3) on the ADS board with no hardware
*   control, characters typed on the keyboard will be received by
*   SMC2 and echoed back out the RS232 port to the ASCII terminal.
*   This function was designed and tested on an ADS860 
*   development board. Note that if a different baud rate is 
*   required, there is a header file on the netcomm website under 
*   the General Software category that is labelled "Asynchronous
*   Baud Rate Tables".
*
* REFERENCES:
*
*   1) MPC860 Users Manual
*   2) PowerPC Microprocessor Family: The Programming Environments for 
*      32-Bit Microprocessors
*
* HISTORY:
*
* 27 APR 98  jay    initial release
*
*-------------------------------------------------------------------------*/

#include <cyg/hal/quicc_smc2.h>
#include "ppc_860.h"

/***********************/
/* Global Declarations */
/***********************/

static EPPC  *IMMR;      /* IMMR base pointer */
static BDRINGS *RxTxBD;  /* buffer descriptors base pointer */
static LB *SMC2Buffers;  /* SMC2 base pointers */

/*---------------------*/
/* Function Prototypes */
/*---------------------*/

//static char  SMC2Poll(void);
static void  InitBDs(void);
//static void  EchoChar(void);
static unsigned long GetIMMR(void);


/*-----------------------------------------------------------------------------
*
* FUNCTION NAME:  cyg_smc2_init
*
* DESCRIPTION:
*
* EXTERNAL EFFECT: 
*                 
* PARAMETERS:  None
*
* RETURNS: None
*
*---------------------------------------------------------------------------*/

void cyg_smc2_init(unsigned long baudRate)

{
   unsigned long *bcsr1; 
   unsigned long clockRate = 20 * 1000 * 1000;  // 20Mhz
   unsigned long divider = clockRate / (baudRate * 16) - 1;

   /*------------------------*/
   /* Establish IMMR pointer */
   /*------------------------*/
      
   IMMR = (EPPC *)(GetIMMR() & 0xFFFF0000);  /* MPC8xx internal register
                                                map  */

   /*-----------------------------------------------*/
   /* Enable RS232 interface on ADS board via BCSR1 */
   /* Get the base address of BCSR                  */ 
   /*-----------------------------------------------*/

   bcsr1 = (unsigned long *) ((IMMR->memc_br1 & 0xffff0000) + 4);    
   *bcsr1 &= ~(1 << 18);  // turn on RS232 port 2


   /*-----------------------*/
   /* Allow SMC2 TX, RX out */
   /*-----------------------*/

   IMMR->pip_pbpar |= (0x0C00);     
   IMMR->pip_pbdir &= 0xF3FF;

   /*------------------------------------------------*/
   /* Set Baud Rate to 9600 for 40MHz System Clock.  */
   /* Enable BRG Count.                              */
   /*------------------------------------------------*/

   IMMR->brgc2 = ((divider << 1) | 0x10000);

   IMMR->si_simode &= ~(0xF0000000);    /* SCM2:  NMSI mode */
   IMMR->si_simode |= 0x10000000;       /* SCM2:  Tx/Rx Clocks are BRG2 */


   /*--------------------*/
   /* Initialize the BDs */
   /*--------------------*/

   InitBDs(); /* before setting up info depending on RxTxBD below */

   IMMR->smc_regs[SMC2_REG].smc_smce = 0xFF;  /* Clear any pending events */

   /*----------------------------------------*/
   /* Set RXBD table start at Dual Port +800 */
   /*----------------------------------------*/

   IMMR->PRAM[PAGE4].pg.other.smc_dsp2.psmc2.u2.rbase = (unsigned short) (unsigned) &RxTxBD->RxBD;

   /*----------------------------------------*/
   /* Set TXBD table start at Dual Port +808 */
   /*----------------------------------------*/

   IMMR->PRAM[PAGE4].pg.other.smc_dsp2.psmc2.u2.tbase = (unsigned short) (unsigned) &RxTxBD->TxBD; 


   /*--------------------------------------*/
   /* Set RFCR,TFCR -- Rx,Tx Function Code */
   /* Normal Operation and Motorola byte   */
   /* ordering                             */
   /*--------------------------------------*/

   IMMR->PRAM[PAGE4].pg.other.smc_dsp2.psmc2.u2.rfcr = 0x18;                   
   IMMR->PRAM[PAGE4].pg.other.smc_dsp2.psmc2.u2.tfcr = 0x18;  

   /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
   /* Protocol Specific Parameters */
   /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

   /*---------------------------*/
   /* MRBLR = MAX buffer length */
   /*---------------------------*/

   IMMR->PRAM[PAGE4].pg.other.smc_dsp2.psmc2.u2.mrblr = 1;    

   /*------------------------------------*/
   /* MAX_IDL = Disable MAX Idle Feature */
   /*------------------------------------*/

   IMMR->PRAM[PAGE4].pg.other.smc_dsp2.psmc2.u2.max_idl = 0;  

   /*-------------------------------------*/
   /* BRKEC = No break condition occurred */
   /*-------------------------------------*/

   IMMR->PRAM[PAGE4].pg.other.smc_dsp2.psmc2.u2.brkec = 0;    

   /*---------------------------------------*/
   /* BRKCR = 1 break char sent on top XMIT */
   /*---------------------------------------*/

   IMMR->PRAM[PAGE4].pg.other.smc_dsp2.psmc2.u2.brkcr = 1;    


   /* InitBDs() used to be here - no harm in doing this again */
   IMMR->smc_regs[SMC2_REG].smc_smce = 0xFF;  /* Clear any pending events */

   /*--------------------------------------------------*/
   /* SMC_SMCM = Mask all interrupts, use polling mode */
   /*--------------------------------------------------*/

   IMMR->smc_regs[SMC2_REG].smc_smcm = 0;     

   IMMR->cpmi_cicr = 0;           /* Disable all CPM interrups */
   IMMR->cpmi_cipr = 0xFFFFFFFF;  /* Clear all pending interrupt events */
   IMMR->cpmi_cimr = 0;           /* Mask all event interrupts */

   /*------------------------------------*/
   /* 8-bit mode,  no parity, 1 stop-bit */
   /* UART SMC Mode                      */
   /* Normal operation (no loopback),    */
   /* SMC Transmitter/Receiver Enabled   */
   /*------------------------------------*/

   IMMR->smc_regs[SMC2_REG].smc_smcmr = 0x4823;

   /*---------------------------------------*/
   /* Initialize Rx and Tx Params for SMC2: */
   /* Spin until cpcr flag is cleared       */
   /*---------------------------------------*/

   for(IMMR->cp_cr = 0x00d1; IMMR->cp_cr & 0x0001;) ;

} 



/*-----------------------------------------------------------------------------
*
* FUNCTION NAME:  InitBDs
*
* DESCRIPTION: This function initializes the Tx and Rx Buffer Descriptors.
*
* EXTERNAL EFFECT: RxTxBD
*                 
* PARAMETERS:  None
*
* RETURNS: None
*
*---------------------------------------------------------------------------*/

static void InitBDs(void)

{
   /*--------------------------------------------------------------------*/
   /* We add 64 bytes to the start of the buffer descriptors because     */
   /* this code was also tested on the monitor version of SDS debugger.  */
   /* The monitor program on our target uses most of the first 64 bytes  */
   /* for buffer descriptors. If you are not using the SDS monitor  with */
   /* Motorola's ADS development board, you can delete 64 below and      */
   /* start at the beginning of this particular block of Dual Port RAM.  */
   /*--------------------------------------------------------------------*/

   /*----------------------------------*/  
   /* Get pointer to BD area on DP RAM */
   /*----------------------------------*/  
      
   RxTxBD = (BDRINGS *)(IMMR->qcp_or_ud.ud.udata_bd + 64);  

   /*-------------------------------------------------------------------*/
   /* Establish the buffer pool in Dual Port RAM. We do this because the*/
   /* pool size is only 2 bytes (1 for Rx and 1 for Tx) and to avoid    */
   /* disabling data cache for the memory region where BufferPool would */
   /* reside. The CPM does not recognize data in buffer pools once it   */
   /* been cached. It's acesses are direct through DMA to external      */
   /* memory.                                                           */
   /*-------------------------------------------------------------------*/

   SMC2Buffers = (LB *)(IMMR->qcp_or_ud.ud.udata_bd + 80);

   /*-----------------------------------*/
   /* Setup Receiver Buffer Descriptors */
   /*-----------------------------------*/
   
   RxTxBD->RxBD.bd_cstatus = 0xA000;            /* Enable, Last BD */
   RxTxBD->RxBD.bd_length = 1;
   RxTxBD->RxBD.bd_addr = &(SMC2Buffers->RxBuffer);


   /*--------------------------------------*/
   /* Setup Transmitter Buffer Descriptors */
   /*--------------------------------------*/

   RxTxBD->TxBD.bd_cstatus = 0x2000;      /* Buffer not yet ready; Last BD */
   RxTxBD->TxBD.bd_length = 1;
   RxTxBD->TxBD.bd_addr = &(SMC2Buffers->TxBuffer);

} /* END InitBDs */


#if 0 // static unused function. -jskov 19990122
/*-----------------------------------------------------------------------------
*
* FUNCTION NAME:  EchoChar
*
* DESCRIPTION: This function facilitates the echoing of a received character.
*
* EXTERNAL EFFECT: RxTxBD
*                 
* PARAMETERS:  None
*
* RETURNS: None
*
*---------------------------------------------------------------------------*/

static void  EchoChar(void)

{
   char mych;

   mych = cyg_smc2_getchar();
   cyg_smc2_putchar(mych); 

} /* end EchoChar */
#endif

/*-----------------------------------------------------------------------------
*
* FUNCTION NAME:  cyg_smc2_putchar
*
* DESCRIPTION: Output a character to SMC2
*
* EXTERNAL EFFECT: RxTxBD
*                 
* PARAMETERS:  ch - input character
*
* RETURNS: None
*
*---------------------------------------------------------------------------*/

void cyg_smc2_putchar(char ch)

{
  /*-----------------------------------*/
  /* Loop until transmission completed */
  /*-----------------------------------*/
   
   while (RxTxBD->TxBD.bd_cstatus  &  0x8000);    
   
   /*------------*/ 
   /* Store data */
   /*------------*/

   *(RxTxBD->TxBD.bd_addr) = ch;                  
   RxTxBD->TxBD.bd_length = 1;

   /*---------------*/
   /* Set Ready bit */
   /*---------------*/

   RxTxBD->TxBD.bd_cstatus |= 0x8000;              

} 



/*-----------------------------------------------------------------------------
*
* FUNCTION NAME:  cyg_smc2_getchar
*
* DESCRIPTION: Get a character from SMC2
*
* EXTERNAL EFFECT: RxTxBD
*                 
* PARAMETERS:  NONE
*
* RETURNS: A character from SMC2
*
*---------------------------------------------------------------------------*/

char cyg_smc2_getchar(void)

{
    
    char ch;   /* output character from SMC2 */
   
    /*--------------------*/
    /* Loop if RxBD empty */
    /*--------------------*/

    while (RxTxBD->RxBD.bd_cstatus  &  0x8000);      

    /*--------------*/
    /* Receive data */
    /*--------------*/

    ch = *(RxTxBD->RxBD.bd_addr);   
                   
    /*----------------------*/
    /* Set Buffer Empty bit */
    /*----------------------*/

    RxTxBD->RxBD.bd_cstatus |= 0x8000;   

    return ch;

} 


#if 0 // static unused function. -jskov 19990122
/*-----------------------------------------------------------------------------
*
* FUNCTION NAME:  SMC2Poll
*
* DESCRIPTION: Poll SMC2 RxBD and check to see if a character was received
*
* EXTERNAL EFFECT: NONE
*                 
* PARAMETERS:  NONE
*
* RETURNS: A one if there is a character available in the receive buffer,
*          else zero.
*
*---------------------------------------------------------------------------*/

static char SMC2Poll(void)

{      
  return (RxTxBD->RxBD.bd_cstatus & 0x8000) ? 0 : 1;

} /* END SMC2Poll */
#endif


/*-----------------------------------------------------------------------------
*
* FUNCTION NAME:  GetIMMR
*
* DESCRIPTION: Returns current value in the IMMR register.
*
* EXTERNAL EFFECT: NONE
*                 
* PARAMETERS:  NONE
*
* RETURNS: The IMMR value in r3. The compiler uses r3 as the register 
*          containing the return value.
*
*---------------------------------------------------------------------------*/

static unsigned long GetIMMR(void)

{
    unsigned long ret;
    asm volatile (" mfspr  %0,638 " : "=r" (ret));  /* IMMR is spr #638 */

    return ret;
}


/* EOF hal/powerpc/fads/quicc_smc2.c */
