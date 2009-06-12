//=============================================================================
//
//      quicc2_diag.c
//
//      HAL diagnostic I/O support routines for MPC8260/QUICC2
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Gary Thomas
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
// Author(s):   hmt
// Contributors:hmt, gthomas
// Date:        1999-06-08
// Purpose:     HAL diagnostics I/O support
// Description: 
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <cyg/hal/hal_mem.h>            // HAL memory definitions
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_if.h>             // hal_if_init
#include <cyg/hal/hal_io.h>             // hal_if_init
#include <cyg/hal/hal_misc.h>           // cyg_hal_is_break

#include <cyg/hal/drv_api.h>            // CYG_ISR_HANDLED
// Added by WPD
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/ppc_regs.h>
#include <cyg/hal/var_intr.h>
#include <cyg/hal/mpc8260.h>            // Needed for IMMR structure

// For Baud Rate Calculation, see MPC8260 PowerQUICC II User's Manual
// 16.3 UART Baud Rate Examples, page 16-5.
// BRGCx[DIV16]  = 0 ==> value of 1 (Prescale divider)
// BRGCx[EXTC]   = 16.667 MHz (Baud Rate generator input clock)
// GSMRx_L[xDCR] = 16 (Sampling Rate)
// UART_CLK_DIV + 1 = 
//       BRGCx[EXTC] / (BRGCx[DIV16] * UART_BAUD_RATE * GSMRx_L[xDCR])
// UART_CLK_DIV = ((66.667 MHz / 4) / (UART_BAUD_RATE * 16)) - 1
// UART_CLK_DIV = ((66.667 MHz ) / (UART_BAUD_RATE * 64)) - 1
// UART_CLK_DIV = ((CYGHWR_HAL_POWERPC_BOARD_SPEED*1000000 ) 
//                / (UART_BAUD_RATE * 64)) (Calculation will truncate, so 
//                                         lose the -1 )
#define UART_BIT_RATE(n) \
    (((int)(CYGHWR_HAL_POWERPC_BOARD_SPEED*1000000))/(n * 64))
#define UART_BAUD_RATE CYGNUM_HAL_DIAG_BAUD


/***********************/
/* Global Declarations */
/***********************/
//#define USE_SMC1

volatile t_PQ2IMM  *IMM;   /* IMM base pointer */
volatile BDRINGS *RxTxBD;  /* buffer descriptors base pointer */
volatile LB *SCC1Buffers;  /* SCC1 base pointers */

#define SMC1_PRAM   0x04703800
#define BD_RX_ERROR 0xBF    /* Mask for set of Receive Buffer Errors,
                               including: DE, LG, NO, AB, CR, OV, CD */

/*---------------------*/
/* Function Prototypes */
/*---------------------*/

static void  InitSCC1Uart(void);
static void  ConfigSCC1Clock(void);
static void  InitParallelPorts(void);
static cyg_uint8 SCC1Poll(void);
static void  InitBDs(void);

static cyg_uint8
cyg_hal_plf_serial_getc(void* __ch_data);

static cyg_bool
cyg_hal_plf_serial_getc_nonblock(void* __ch_data, cyg_uint8* ch);

static cyg_bool
cyg_hal_plf_serial_getc_timeout(void* __ch_data, cyg_uint8* ch);

static void
cyg_hal_plf_serial_putc(void* __ch_data, cyg_uint8 ch);

static void
cyg_hal_plf_serial_write(void* __ch_data, const cyg_uint8* __buf, 
                         cyg_uint32 __len);
static void
cyg_hal_plf_serial_read(void* __ch_data, cyg_uint8* __buf, cyg_uint32 __len);

static void
cyg_hal_plf_serial_init_channel(void);

static int
cyg_hal_plf_serial_isr(void *__ch_data, int* __ctrlc, 
                       CYG_ADDRWORD __vector, CYG_ADDRWORD __data);

static int
cyg_hal_plf_serial_control(void *__ch_data, __comm_control_cmd_t __func, ...);

static int
cyg_hal_plf_serial_isr(void *__ch_data, int* __ctrlc, 
                       CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
    t_PQ2IMM  *immr = (t_PQ2IMM*) __ch_data;
    struct cp_bufdesc *bd;
    char ch;
    int res = 0;
    cyg_uint32 regval;

    CYGARC_HAL_SAVE_GP();
    //GREEN_LED_ON;
/*
    dbg_values[3]++;
    dbg_values[10+dbg_values[3]] = 
        RxTxBD->RxBD.bd_cstatus
        | (immr->scc_regs[SCC1].scce<<16);
*/
    *__ctrlc = 0;
    if (immr->scc_regs[SCC1].scce & 0x0001) {

        // Clear the event by writing a "1" to the prpoper bit.
        immr->scc_regs[SCC1].scce = 0x0001;

        if((RxTxBD->RxBD.bd_cstatus  &  0x8000) == 0){
            ch = *(RxTxBD->RxBD.bd_addr);
            /*----------------------*/
            /* Set Buffer Empty bit */
            /*----------------------*/
            //dbg_values[10+dbg_values[3]] = __vector | 0xffff0000;
            //dbg_values[10+dbg_values[3]] |= ch << 8;
    
            RxTxBD->RxBD.bd_cstatus |= 0x8000;   

            if( cyg_hal_is_break( &ch , 1 ) ){
                //GREEN_LED_ON;
              *__ctrlc = 1;
              //dbg_values[7] = immr->ic_sivec;              
            }
        }
        // Interrupt handled. Acknowledge it.
        //eppc->cpmi_cisr = 0x10;
        // Clear interrupt in SIPNR_L by writing a one to bit 8 (0x800000)
        HAL_READ_UINT32(  ((char *) IMM) + 0x10000 + CYGARC_REG_IMM_SIPNR_L,
                         regval);
        regval |= 0x00800000;
        HAL_WRITE_UINT32( ((char *) IMM) + 0x10000 + CYGARC_REG_IMM_SIPNR_L,
                          regval);

        res = CYG_ISR_HANDLED;
    }

    //GREEN_LED_OFF;
    CYGARC_HAL_RESTORE_GP();
    return res;
}

/* Early initialization of comm channels. Must not rely
 * on interrupts, yet. Interrupt operation can be enabled
 * in _bsp_board_init().
 */
void
cyg_hal_plf_serial_init(void)
{
#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
    hal_virtual_comm_table_t* comm;
    int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

    static int init = 0;  // It's wrong to do this more than once
    if (init) return;
    init++;

    // init_channel sets the global *IMM == 0x04700000, the base of the
    // Internal Memory map for the MPC8260
    cyg_hal_plf_serial_init_channel();

    // Setup procs in the vector table

    // Set channel 0
    CYGACC_CALL_IF_SET_CONSOLE_COMM(0);// Should be configurable!
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, IMM);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_plf_serial_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_plf_serial_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_plf_serial_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_plf_serial_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_plf_serial_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_plf_serial_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_plf_serial_getc_timeout);

    // Restore original console
    CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);

#else // No CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
    static int init = 0;  // It's wrong to do this more than once
    if (init) return;
    init++;

    cyg_hal_plf_serial_init_channel();
#endif
}

static void
cyg_hal_plf_serial_init_channel(void)
{
    /* We will assume here that the IMMR has been programmed such that
     * the internal Memory Map starts at 0x04700000.  Initialization
     * should have done that setup.
     */

    IMM =  (t_PQ2IMM *)0x04700000;  /* MPC8260 internal register map  */

    /*----------------------------------------------------------------------*/  
    /* Get a pointer to the BD area on DP RAM. The buffer descriptors (BDs) */
    /* and the Rx/Tx data buffers will be located right after SCC1's para-  */
    /* meter RAM area because only 2 BDs and 2 data buffers are being used  */
    /* for this port and SCC1 only uses 64 bytes of it's allotted 256 for   */
    /* it's parameter ram. One BD and one data buffer each for transmit and */
    /* receive will be used. This buffer descriptor area will take up 16    */
    /* bytes.                                                               */
    /*----------------------------------------------------------------------*/  
      

    RxTxBD = (BDRINGS *) 0x04708070;
    //    (((CYG_WORD)&(IMM->pram.serials.scc_pram[SCC1])) + 72);  

    //RxTxBD = (BDRINGS *)
    //    (((CYG_WORD)&(IMM->pram.serials.scc_pram[SCC1])) + 72);  

    /*-------------------------------------------------------------------*/
    /* Establish the buffer pool in Dual Port RAM. We do this because the*/
    /* pool size is only 2 bytes (1 for Rx and 1 for Tx) and to avoid    */
    /* disabling data cache for the memory region where BufferPool would */
    /* reside. The CPM does not recognize data in buffer pools once it   */
    /* been cached. It's acesses are direct through DMA to external      */
    /* memory.                                                           */
    /*-------------------------------------------------------------------*/

    //SCC1Buffers = (LB *)
    //    (((CYG_WORD)&(IMM->pram.serials.scc_pram[SCC1])) + 96); 

    SCC1Buffers = (LB *) 0x04708090;
    //    (((CYG_WORD)&(IMM->pram.serials.scc_pram[SCC1]))
    //     + 72 + 14); 

   /*----------------------------------------*/
   /* Initialize SCC1 and buffer descriptors */
   /*----------------------------------------*/
/*    while(1); */

    InitSCC1Uart();

}

/*---------------------------------------------------------------------------
*
* FUNCTION NAME:  InitBDs
*
*
* DESCRIPTION:
*
*  Initializes BD rings to point RX BDs to first half of buffer pool and TX 
*  BDs to second half of buffer pool. This function also initializes the 
*  buffer descriptors control and data length fields. It also ensures that 
*  transmit and recieve functions are disabled before buffer descriptors are
*  initialized.
*
* EXTERNAL EFFECTS: Disable Tx/Rx functions. Changes BDs in dual port ram.
*
* PARAMETERS: None
*
* RETURNS: None
*
*---------------------------------------------------------------------------*/

void InitBDs()

{
   
   /*--------------------------------------------------------------------*/
   /* First let's ensure the SCC1 functions are off while we program the */
   /* buffer descriptors and the parameter ram. Clear the ENT/ENR bits   */
   /* in the GSMR -- disable Transmit/Receive                            */
   /*--------------------------------------------------------------------*/
   
   IMM->scc_regs[SCC1].gsmr_l &= DISABLE_TX_RX;

   /*--------------------------------------*/
   /* Issue Init Stop TX Command for SCC1. */
   /*--------------------------------------*/

   while ((IMM->cpm_cpcr & CPCR_FLG) != READY_TO_RX_CMD); 

   IMM->cpm_cpcr = SCC1_PAGE_SUBBLOCK |
                   CPCR_STOP_TX |
                   CPCR_FLG;             /* ISSUE COMMAND */

   while ((IMM->cpm_cpcr & CPCR_FLG) != READY_TO_RX_CMD); 


   /*-----------------------------------*/
   /* Setup Receiver Buffer Descriptors */
   /*-----------------------------------*/
   
#if defined(CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT) \
    || defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT)
   /* Set receive Buffer to generate an interrupt when buffer full */
   RxTxBD->RxBD.bd_cstatus = 0xB000; /* 0xB000; */
#else
   RxTxBD->RxBD.bd_cstatus = 0xA000;     /* Empty, Wrap Bit */
#endif
   //dbg_values[3] = RxTxBD->RxBD.bd_cstatus;
   RxTxBD->RxBD.bd_length = 1;
   RxTxBD->RxBD.bd_addr = &(SCC1Buffers->RxBuffer);

   /*--------------------------------------*/
   /* Setup Transmitter Buffer Descriptors */
   /*--------------------------------------*/

   RxTxBD->TxBD.bd_cstatus = 0x2800;   /* Buffer not yet ready; Wrap Bit 
                                          Clear-to-send_report           */

   RxTxBD->TxBD.bd_length = 1;
   RxTxBD->TxBD.bd_addr = &(SCC1Buffers->TxBuffer);

} /* end InitBDs */


/*---------------------------------------------------------------------------
*
* FUNCTION NAME:  InitSCC1Uart 
*                  
*
* DESCRIPTION:
*
*  SCC1 Uart Mode Initialization Routine.
*                 
* EXTERNAL EFFECT:
*
*  Initializes SCC1 to operate in Uart mode at 9600 Baud, No Parity, 8 data
*  bits, and 1 stop bit. 
*
* PARAMETERS: None
*
* RETURNS: None 
*
*--------------------------------------------------------------------------*/

void InitSCC1Uart()

{
    cyg_uint32 regval;
   /*----------------------------------------------------------------------*/
   /* Configure the parallel ports so that TXD and RXD are connected to    */
   /* the appropriate port pins and are configured according to their      */
   /* functions.                                                           */
   /*----------------------------------------------------------------------*/

   InitParallelPorts();

   /*------------------------------------------*/
   /* Configure Clock Source and Clock Routing */
   /*------------------------------------------*/

   ConfigSCC1Clock();

   /*-----------------------------------*/
   /* Initialize the Buffer Descriptors */
   /*-----------------------------------*/

   InitBDs();

   /*----------------------------------------------------------------------*/
   /* Program Rx and Tx Function Codes (RFCRx/TFCRx).                      */
   /*                                                                      */
   /* - Bits 0-1 reserved. Set to zero.                                    */
   /*                                                                      */
   /* - GBL (Global) = 0 = Snooping Disabled.                              */
   /*                                                                      */
   /* - BO (Byte Ordering) = 11 = Big-endian or true little-endian.        */
   /*                                                                      */
   /* - TC[2] (Transfer Code) = 0 = Transfer code is 0                     */
   /*                                                                      */
   /* - DTB (Data Bus Indicator) = 1 =                                     */
   /*                                                                      */
   /*    Use the Local Bus for SDMA operation. In this example it doesn't  */
   /*    matter because the buffer were located in parameter ram. Normally */
   /*    this bit would be set because data buffers normally will reside   */
   /*    in Local memory.                                                  */
   /*----------------------------------------------------------------------*/

   IMM->pram.serials.scc_pram[SCC1].rfcr = 0x18;

   IMM->pram.serials.scc_pram[SCC1].tfcr = 0x18;

   IMM->scc_regs[SCC1].psmr = 0xB000;

   /*------------------------------------------------------------*/
   /* Set RBASE, TBASE -- Rx,Tx Buffer Descriptor Base Addresses */
   /*------------------------------------------------------------*/

   IMM->pram.serials.scc_pram[SCC1].rbase = (CYG_WORD16)&RxTxBD->RxBD;

   IMM->pram.serials.scc_pram[SCC1].tbase = (CYG_WORD16)&RxTxBD->TxBD;     

   /*-----------------------------------------*/
   /* Set MRBLR -- Max. Receive Buffer Length */
   /*-----------------------------------------*/

   IMM->pram.serials.scc_pram[SCC1].mrblr = 1;
   
   /*----------------------------------------------------------------------*/
   /* Program the General SCC Mode Register High (GSMR_H)                  */
   /*                                                                      */
   /* - Bits 0-14 Reserved. Set to 0.                                      */
   /*                                                                      */
   /* - GDE (Glitch Detect Enable) = 0 = No Glitch Detect. BRG supplies    */
   /*                                    the clock so there's no need to   */
   /*                                    detect glitches.                  */
   /*                                                                      */
   /* - TCRC (Transparent CRC) = 00 = This field is ignored for Uart mode. */
   /*                                                                      */
   /* - REVD (Reverse Data) = 0 = This field is ignored for Uart mode.     */
   /*                                                                      */
   /* - TRX,TTX (Transparent Receiver/Transmitter) = 00 = Normal operation */
   /*                                                                      */
   /* - CDP,CTSP (CD/ & CTS/ sampling) = 00 = Normal Operation (envelope   */
   /*                                         mode.                        */
   /*                                                                      */
   /* - CDS,CTSS (CD/ & CTSS Sampling) = 00 =                              */
   /*                                                                      */
   /*    CD/ or CTS/ is assumed to be asynchronous with data. It is        */
   /*    internally synchronized by the SCC, then data is received (CD/)   */
   /*    or sent (CTS/) after several clock delays.                        */
   /*                                                                      */
   /* - TFL (Transmit FIFO length) = 0 =                                   */
   /*                                                                      */
   /*    Normal Operation. The SCC transmit FIFO is 32 bytes.              */
   /*                                                                      */
   /* - RFW (Rx FIFO Width) = 1 =                                          */
   /*                                                                      */
   /*    Low-latency operation.The receive FIFO is 8 bits wide, reducing   */
   /*    the Rx FIFO to a quarter of it's normal size. This allows data to */
   /*    be written to the buffer as soon as a character is received,      */
   /*    instead of waiting to receive 32 bits. This configuration must be */
   /*    chosen for character-oriented protocols, such as UART. It can     */
   /*    also be used for low-performance, low-latency, transparent        */
   /*    operation.                                                        */
   /*                                                                      */
   /* - TXSY (Trasnmitter Synchronized) = 0 =                              */
   /*                                                                      */
   /*    No synchronization between receiver and transmitter.              */
   /*                                                                      */
   /* - SYNL (Sync Length) = 0 = An external sync (CD/) is used instead of */
   /*                            the sync pattern in the DSR.              */
   /*                                                                      */
   /* - RTSM (RTS/ Mode) = 0 = Send idles between frames as defined by the */
   /*                          protocol and the TEND bit. TRS/ is negated  */
   /*                          between frames.                             */
   /*                                                                      */
   /* - RSYN (Receive Synchronization Timing) = 0 = This field is ignored  */
   /*                                               for Uart mode.         */
   /*                                                                      */
   /*----------------------------------------------------------------------*/
      
   IMM->scc_regs[SCC1].gsmr_h = 0x00000060;


   /*----------------------------------------------------------------------*/
   /* Program the General SCC Mode Register High (GSMR_L)                  */
   /*                                                                      */
   /* - Bit 0 Reserved. Set to 0.                                          */
   /*                                                                      */
   /* - EDGE (Clock Edge) = 00 = Ignored in Uart Mode.                     */
   /*                                                                      */
   /* - TCI (Transmit Clock Invert) = 0 = Normal Operation                 */
   /*                                                                      */
   /* - TSNC (Transmit Sense) = 00 = Infinite. Carrier sense is always     */
   /*                                active.                               */
   /*                                                                      */
   /* - RINV (DPLL Rx Input Invert) = 0 = Do not invert.                   */
   /*                                                                      */
   /* - TINV (DPLL Tx Input Invert) = 0 = Do not invert.                   */
   /*                                                                      */
   /* - TPL (Tx Preamble Length) = 000 = No Preamble.                      */
   /*                                                                      */
   /* - TPP (Tx Preamble Pattern) = 00 = All zeros. This field is ignored  */
   /*                                    for Uart mode.                    */
   /*                                                                      */
   /* - TEND (Transmitter Frame Ending) = 0 =                              */
   /*                                                                      */
   /*    Default operation. TxD is encoded only when data is sent,         */
   /*    including the preamble and opening and closing flags/syncs. When  */
   /*    no data is available to send, the signal is driven high.          */
   /*                                                                      */
   /* - TDCR (Transmitter DPLL Clock Rate) = 10 =                          */
   /*                                                                      */
   /*    16x clock mode. This value is normally chosen for Uart mode.      */
   /*                                                                      */
   /* - RDCR (Receiver DPLL Clock Rate) = 10 =                             */
   /*                                                                      */
   /*    16x clock mode. This value is normally chosen for Uart mode.      */
   /*                                                                      */
   /* - RENC (Receiver Decoding Method) = 000 =                            */
   /*                                                                      */
   /*    NRZ. Required for Uart Mode (asynchronous or synchronous).        */
   /*                                                                      */
   /* - TENC (Transmitter Encoding Method) = 000 =                         */
   /*                                                                      */
   /*    NRZ. Required for Uart Mode (asynchronous or synchronous).        */
   /*                                                                      */
   /* - DIAG (Diagnostic Mode) = 01 = Loopback                             */
   /*                                                                      */
   /* - ENR (Enable Receiver) = 0 = Disabled for now. Will enabled later in*/
   /*                               this function.                         */
   /*                                                                      */
   /* - ENT (Enable Transmitter) = 0 = Disabled for now. Will enable later */
   /*                                  in this function.                   */
   /*                                                                      */
   /* - MODE (Channel Protocol Mode) = 0100 = Uart mode.                   */
   /*                                                                      */
   /*----------------------------------------------------------------------*/


   IMM->scc_regs[SCC1].gsmr_l = 0x00028004;  

   /*-----------------------------------------*/
   /* Clear SCCE Register by writing all 1's. */
   /*-----------------------------------------*/

   IMM->scc_regs[SCC1].scce = ALL_ONES;

   /*----------------------------------------------------------------------*/
   /* Issue Init RX & TX Parameters Command for SCC1. This command to the  */
   /* CP lets it know to reinitialize SCC1 with the new parameter RAM      */
   /* values. When the ENT/ENR bits are set below Hunt Mode will begin     */
   /* automatically.                                                       */
   /*----------------------------------------------------------------------*/

   while ((IMM->cpm_cpcr & CPCR_FLG) != READY_TO_RX_CMD); 

   IMM->cpm_cpcr = SCC1_PAGE_SUBBLOCK |
                   CPCR_INIT_TX_RX_PARAMS |
                   CPCR_FLG;                 /* ISSUE COMMAND */

   while ((IMM->cpm_cpcr & CPCR_FLG) != READY_TO_RX_CMD); 

   /*-------------------------------------------------------------*/
   /* Set the ENT/ENR bits in the GSMR -- Enable Transmit/Receive */
   /*-------------------------------------------------------------*/

    IMM->scc_regs[SCC1].gsmr_l |= GSMR_L1_ENT | GSMR_L1_ENR;
#if defined(CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT) \
    || defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT)
#define PFADDED   
#ifdef PFADDED   
    // Fill out the control Character Table.  Make the first entry 
    // an end of table line. 
    // cc[0] = 0x4003 ==> reject if char = 0x3, write to RCCR
    IMM->pram.serials.scc_pram[SCC1].SpecificProtocol.u.cc[0] = 0x4003;
    {
        int i;
        for (i = 0; i < 8; i++){
            IMM->pram.serials.scc_pram[SCC1].SpecificProtocol.u.cc[i] = 0x8000;
        }
    }
    
    IMM->pram.serials.scc_pram[SCC1].SpecificProtocol.u.rccm  = 0xc000;
#endif
   /*-----------------------------------------*/
    /* Write to the SCCM mask register to enable an CCR interrupt*/
   /*-----------------------------------------*/
   IMM->scc_regs[SCC1].sccm = 0x1;

   /* Unmask the CPM SCC1 interrupt */
   HAL_READ_UINT32( ((char *) IMM) + 0x10000 + CYGARC_REG_IMM_SIMR_L, regval);
   regval |= 0x00800000;
   HAL_WRITE_UINT32( ((char *) IMM) + 0x10000 + CYGARC_REG_IMM_SIMR_L, regval);

#endif
} /* end SCC1HInit() */


/*--------------------------------------------------------------------------
*
* FUNCTION NAME: ConfigSCC1Clock
*
* DESCRIPTION:
*
*  This function will configure SCC1 to utilize Baud Rate Generator #1. It 
*  will program the the baud rate generator and configure the SMXSCR 
*  register in the CPM Mux block to route the clock to SCC1. SCC2, SCC3, and
*  SCC4 are also programmed to assume Baud Rate Generator #1, #2, and #3 
*  respectively to be routed to them. There was no special reason for doing
*  this; The bit values needed to be programmed to something.
*
*
* EXTERNAL EFFECTS: BRGC1 and CMXSCR
*
* PARAMETERS:  
*
* RETURNS: Nothing
*
*--------------------------------------------------------------------------*/

void  ConfigSCC1Clock()

{
    /* FIXME --- This picture is not accurate */
   /*----------------------------------------------------------------------*/
   /* Initialize Baud Rate Generator #1 to generate a 9600 clock. The      */
   /* source of the clock starts with the input clock that is generated    */
   /* external to the MPC8260 by a clock generator. This clock is then     */
   /* fed to the CPM PLL where it is multiplied up and the output freq-    */
   /* uency is determined by the MODCLK_HI bits in the Hard Reset Config-  */
   /* uration Word and MODCK pins on the MPC8260. This output is fed to a  */
   /* general purpose Baud Rate Generator Divider that services all 8 baud */
   /* rate generators. From the output of this divider the the clock goes  */
   /* to the baud rate generator circuitry where, in this case, BRGCLK is  */
   /* selected to be BRGO1 (the output clock). This frequency is deter-    */
   /* mined by a Divide by 1 or 16 divider and then a 12 bit Prescaler     */
   /* divider. the clock then goes to the CPM Mux where BRG1 is selected   */
   /* to be TCLK and RCLK to SCC1. This is accomplished by programming the */
   /* CMXSCR register. TCLK and RCLK are then routed to the SCC1 DPLL      */
   /* circuitry at 9600 baud where the DPLL will be programmed to multiply */
   /* the frequency by X16 for UART over-sampling. Here a diagram and the  */
   /* programming:                                                         */
   /*                                                                      */
   /* ----------    ---------         ---------------------                */
   /* |External|    |MPC8260| 132 Mhz |General Purpose    | 16.5 Mhz       */
   /* |Clk Gen |----|CPM PLL|---------|Baud Rate Generator|--------->|     */
   /* |66 Mhz  |    |Block  |         |Divider [/8] (SCCR)|          |     */
   /* ----------    ---------         ---------------------          |     */
   /*                                                                |     */
   /* |<-------------------------------------------------------------|     */
   /* |                                                                    */
   /* |   ------------------------              -------------------        */
   /* |-->|Divide by 1 or 16 in  | 1.03125 Mhz  |12 Bit Prescaler  | BRG01 */
   /*     |Baud Rate Generator   |------------->|in Baud Rate Gen. |--|    */
   /*     |Block [/16 selected]  |              |Block [107(0x6B)  |  |    */
   /*     |(BRGC1 Programmed)    |              |(BRGC1 Programmed)|  |    */
   /*     ------------------------              --------------------  |    */
   /*                                                                 |    */
   /* |<--------------------------------------------------------------|    */
   /* |                                                                    */
   /* |            -------------- TCLK  ----------                         */                     
   /* | 9.638 Khz  |   CPM Mux  |------>| SCC1   |----> TCLK*16 }*16 for   */
   /* |----------->|  (CMXSCR)  | RCLK  | DPLL   |              }over-     */
   /*              |(Programmed)|------>| *16    |----> RCLK*16 }sampling  */
   /*              --------------       |(GSMR_L)|                         */
   /*                                   ----------                         */
   /*                                                                      */
   /* SCCR was programmed in init8260.s. BRGC1,CMXSCR will be programmed   */
   /* in this function. GSMR_L will be programmed in InitSCC1Uart().       */   
   /*                                                                      */
   /*----------------------------------------------------------------------*/

   /*----------------------------------------------------------------------*/
   /* Program Baud Rate Generator Configuration #1 Register (BRGC1).       */
   /*                                                                      */
   /* - Bits 0-13 are reserved. Set to 0.                                  */
   /*                                                                      */
   /* - RST (Reset BRG) = 0 = Enable the BRG                               */
   /*                                                                      */
   /* - EN (Enable BRG Count) = 1 = Enable clocks to the BRG               */
   /*                                                                      */
   /* - EXTC (External Clock Source) = 00 =                                */
   /*                                                                      */
   /*    The BRG input clock comes from the BRGCLK                         */
   /*                                                                      */
   /* - ATB (AutoBaud) = 0 = Normal operation of the BRG.                  */
   /*                                                                      */
   /* - CD (Clock Divider) = 0x6C = 108 decimal =                          */
   /*                                                                      */
   /*    The input frequency is 1.03125 Mhz Dividing it by 107 will give   */
   /*    9.638 Khz. However 1 must be added to the count value because it  */
   /*    counts down to 0. So the programmed value is 108.                 */
   /*                                                                      */
  /* PF edit - changed CD = 0x1A = 26 ==> baud rate of 4 * 9600 = 38400    */
   /* - DIV16 (Divide-by-16) = 0 = divide by 1.                           */
   /*----------------------------------------------------------------------*/

  //     IMM->brgs_brgc1 = 0x000100D6;   
  //IMM->brgs_brgc1 = 0x00010034; /* Attempt to get 38400 baud */   
  // IMM->brgs_brgc1 = 0x00010022; /* Attempt to get 57600 baud */   
  //IMM->brgs_brgc1 = 0x00010010; /* Attempt to get 115200 baud */   
  IMM->brgs_brgc1 = 0x00010000 | (UART_BIT_RATE(UART_BAUD_RATE) << 1);

   /*----------------------------------------------------------------------*/
   /* Program the CMX SCC Route Register (CMXSCR).                         */
   /*                                                                      */
   /* - GR1 (Grant support of SCC1) = 0 =                                  */
   /*                                                                      */
   /*    SCC1 transmitter does not support the grant mechanism. The grant  */
   /*    is always asserted internally.                                    */
   /*                                                                      */
   /* - SC1 (SCC1 connection) = 0                                          */
   /*                                                                      */
   /*    SCC1 is not connected to the TSA of the SIs but is connected      */
   /*    directly to the NMSIx pins.                                       */
   /*                                                                      */
   /* - RS1CS (Receive SCC1 or clock source) = 000 =                       */
   /*                                                                      */
   /*    SCC1 receive clock is BRG1.                                       */
   /*                                                                      */
   /* - TS1CS (Transmit SCC1 clock source) = 000 =                         */
   /*                                                                      */
   /*    SCC1 transmit clock is BRG1.                                      */
   /*                                                                      */
   /* - GR2 (Grant support of SCC2) = 0 =                                  */
   /*                                                                      */
   /*    SCC1 transmitter does not support the grant mechanism. The grant  */
   /*    is always asserted internally.                                    */
   /*                                                                      */
   /* - SC2 (SCC2 connection) = 0                                          */
   /*                                                                      */
   /*    SCC2 is not connected to the TSA of the SIs but is connected      */
   /*    directly to the NMSIx pins.                                       */
   /*                                                                      */
   /* - RS2CS (Receive SCC2 or clock source) = 001 =                       */
   /*                                                                      */
   /*    SCC1 receive clock is BRG2.                                       */
   /*                                                                      */
   /* - TS2CS (Transmit SCC2 clock source) = 001 =                         */
   /*                                                                      */
   /*    SCC2 transmit clock is BRG2.                                      */
   /*                                                                      */
   /* - GR3 (Grant support of SCC3) = 0 =                                  */
   /*                                                                      */
   /*    SCC3 transmitter does not support the grant mechanism. The grant  */
   /*    is always asserted internally.                                    */
   /*                                                                      */
   /* - SC3 (SCC3 connection) = 0                                          */
   /*                                                                      */
   /*    SCC3 is not connected to the TSA of the SIs but is connected      */
   /*    directly to the NMSIx pins.                                       */
   /*                                                                      */
   /* - RS3CS (Receive SCC3 or clock source) = 010 =                       */
   /*                                                                      */
   /*    SCC3 receive clock is BRG3.                                       */
   /*                                                                      */
   /* - TS3CS (Transmit SCC3 clock source) = 010 =                         */
   /*                                                                      */
   /*    SCC3 transmit clock is BRG3.                                      */
   /*                                                                      */
   /* - GR4 (Grant support of SCC4) = 0 =                                  */
   /*                                                                      */
   /*    SCC4 transmitter does not support the grant mechanism. The grant  */
   /*    is always asserted internally.                                    */
   /*                                                                      */
   /* - SC4 (SCC4 connection) = 0                                          */
   /*                                                                      */
   /*    SCC4 is not connected to the TSA of the SIs but is connected      */
   /*    directly to the NMSIx pins.                                       */
   /*                                                                      */
   /* - RS4CS (Receive SCC4 or clock source) = 011 =                       */
   /*                                                                      */
   /*    SCC4 receive clock is BRG4.                                       */
   /*                                                                      */
   /* - TS4CS (Transmit SCC4 clock source) = 011 =                         */
   /*                                                                      */
   /*    SCC4 transmit clock is BRG4.                                      */
   /*                                                                      */
   /*----------------------------------------------------------------------*/

   IMM->cpm_mux_cmxscr = 0x0009121B;  

} /* end of ConfigSCC1Clock() */



/*--------------------------------------------------------------------------
*
* FUNCTION NAME: InitParallelPorts
*
* DESCRIPTION:
*
*  This function programs the parallel port configuration registers to 
*  utilize the pins required for proper SCC1 operation. The pins programmed 
*  here are TxD and RxD for SCC1 and CD1 for SCC1.
*
* EXTERNAL EFFECTS: Parallel Port C and D Configuration Registers
*
* PARAMETERS:  
*
* RETURNS: Nothing
*
*--------------------------------------------------------------------------*/

void InitParallelPorts()

{
   /*--------------------------------------------*/
	/* Program the Port Special Options Registers */
   /*--------------------------------------------*/

	IMM->io_regs[PORT_C].psor &= 0xFFFDFFFF; /* CD/ pin 14 */
	IMM->io_regs[PORT_D].psor &= 0xFFFFFFFC; /* clear first */
	IMM->io_regs[PORT_D].psor |= 0x00000002; /* TXD pin 30| RXD pin 31 */

   /*-------------------------------------------*/
	/* Program the Port Pin Assignment Registers */
   /*-------------------------------------------*/

	IMM->io_regs[PORT_C].ppar |= 0x00020000;   /* CD/ pin 14 */
	IMM->io_regs[PORT_D].ppar |= 0x00000003;   /* TXD pin 30| RXD pin 31 */

   /*-------------------------------------------*/
	/* Program the Port Data Direction Registers */
   /*-------------------------------------------*/

	IMM->io_regs[PORT_C].pdir &= 0xFFFDFFFF;   /* CD/ pin 14 */
	IMM->io_regs[PORT_D].pdir &= 0xFFFFFFFC;   /* clear first */
	IMM->io_regs[PORT_D].pdir |= 0x00000002;   /* TXD pin 30| RXD pin 31 */

   /*---------------------------------------*/
	/* Program the Port Open-Drain Registers */
   /*---------------------------------------*/

	IMM->io_regs[PORT_C].podr &= 0xFFFDFFFF; /* CD/ pin 14 */
	IMM->io_regs[PORT_D].podr &= 0xFFFFFFFC; /* TXD pin 30| RXD pin 31 */

}

/*---------------------------------------------------------------------------
*
* FUNCTION NAME: BDRxError
*
* DESCRIPTION:
*
*     Return TRUE if Buffer Descriptor Status bd_cstatus indicates Receive 
*     Error; Return FALSE otherwise note Receive Errors are as follows:
*
*     0x80: DPLL Error (DE)
*     0x20: Length Violation (LG)
*     0x10: Non-Octet Aligned (NO)
*     0x8: Rx Abort Sequence (AB)
*     0x4: Rx CRC Error (CR)
*     0x2: Overrun (OV)
*     0x1: Carrier Detect Lost (CD)
*
* EXTERNAL EFFECTS: None
*
* PARAMETERS:  
*
*     bd_cstatus
*
* RETURNS: TRUE if there was an error and FALSE if there wasn't
*
*---------------------------------------------------------------------------*/

CYG_WORD16 BDRxError(CYG_WORD16 bd_cstatus)

{
   
   if (bd_cstatus & BD_RX_ERROR)
      
      return true;

   else

      return false;

} /* end BDRxError */

/*---------------------------------------------------------------------------
*
* FUNCTION NAME:  SCC1Poll
*
* DESCRIPTION: Poll SCC1 RxBD and check to see if a character was received
*
* EXTERNAL EFFECT: NONE
*                 
* PARAMETERS:  NONE
*
* RETURNS: A one if there is a character available in the receive buffer,
*          else zero.
*
*--------------------------------------------------------------------------*/

cyg_uint8 SCC1Poll(void)

{
      
   if(RxTxBD->RxBD.bd_cstatus & 0x8000) 
   
   {
       return 0;  /*  character NOT available */
   } 
   
   else
   
   {
       return 1;  /*  character IS available */
   }

} /* END SCC1Poll */

#ifndef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
void 
cyg_hal_plf_serial_putc(cyg_uint8 ch)
#else
static void
cyg_hal_plf_serial_putc(void* __ch_data, cyg_uint8 ch)
#endif
{
   /*-----------------------------------*/
   /* Loop until transmission completed */
   /*-----------------------------------*/
#if 1
  volatile CYG_WORD16 stat = 1;
  while (stat){
    stat = RxTxBD->TxBD.bd_cstatus  &  0x8000;
  }
#else
   while (RxTxBD->TxBD.bd_cstatus  &  0x8000);    
#endif
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


static cyg_bool
cyg_hal_plf_serial_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
  cyg_bool retval;

  if ( (retval = SCC1Poll() ) ){   /* Check BD status for Rx characters */
    *ch = cyg_hal_plf_serial_getc(__ch_data);
  }

  return retval;
}

#ifndef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
cyg_uint8
cyg_hal_plf_serial_getc(void)
#else
static cyg_uint8
cyg_hal_plf_serial_getc(void* __ch_data)
#endif
{
    cyg_uint8 ch;
    /*--------------------*/
    /* Loop if RxBD empty */
    /*--------------------*/
#if 1
    volatile short stat = 1;
    while (stat){
        stat = RxTxBD->RxBD.bd_cstatus  &  0x8000;
    }
#else
    while (RxTxBD->RxBD.bd_cstatus  &  0x8000);      
#endif
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



static void
cyg_hal_plf_serial_write(void* __ch_data, const cyg_uint8* __buf, 
                         cyg_uint32 __len)
{
  //CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
#ifndef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
        cyg_hal_plf_serial_putc(*__buf++);
#else
        cyg_hal_plf_serial_putc(__ch_data, *__buf++);
#endif

    //CYGARC_HAL_RESTORE_GP();
}

static void
cyg_hal_plf_serial_read(void* __ch_data, cyg_uint8* __buf, cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
#ifndef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
        *__buf++ = cyg_hal_plf_serial_getc();
#else
        *__buf++ = cyg_hal_plf_serial_getc(__ch_data);
#endif
    CYGARC_HAL_RESTORE_GP();
}

cyg_int32 msec_timeout = 1000;

static cyg_bool
cyg_hal_plf_serial_getc_timeout(void* __ch_data, cyg_uint8* ch)
{
    int delay_count = msec_timeout * 10; // delay in .1 ms steps
    cyg_bool res;
    CYGARC_HAL_SAVE_GP();

    for(;;) {
        res = cyg_hal_plf_serial_getc_nonblock(__ch_data, ch);
        if (res || 0 == delay_count--)
            break;
        
        CYGACC_CALL_IF_DELAY_US(100);
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}

static int
cyg_hal_plf_serial_control(void *__ch_data, __comm_control_cmd_t __func, ...)
{
    static int irq_state = 0;
    int ret = 0;
    cyg_uint32 regval;
    CYGARC_HAL_SAVE_GP();

    switch (__func) {
    case __COMMCTL_IRQ_ENABLE:
        //HAL_INTERRUPT_UNMASK(CYGNUM_HAL_INTERRUPT_CPM_SMC1);
        // For now, don't bother calling a macro, just do it here
        // Bit 8 in the SIMR_L corresponds to SCC1
        HAL_READ_UINT32( ((char *) IMM) + 0x10000 + CYGARC_REG_IMM_SIMR_L, regval);
        regval |= 0x00800000;

        HAL_WRITE_UINT32( ((char *) IMM) + 0x10000 + CYGARC_REG_IMM_SIMR_L, regval);
        asm volatile ("ori   %0, 0, 0x1234;"                     \
                      : /* No output */                   \
                      : "I" (22)); /* %0 ==> r2 */

        //RxTxBD->RxBD.bd_cstatus = 0xB000;
        irq_state = 1;
        //RED_LED_ON;
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
        irq_state = 0;
        //HAL_INTERRUPT_MASK(CYGNUM_HAL_INTERRUPT_CPM_SMC1);
        HAL_READ_UINT32( ((char *) IMM) + 0x10000 + CYGARC_REG_IMM_SIMR_L, regval);
        regval &= 0xFF7FFFFF;

        HAL_WRITE_UINT32( ((char *) IMM) + 0x10000 + CYGARC_REG_IMM_SIMR_L, regval);
        //RED_LED_OFF;
        //RxTxBD->RxBD.bd_cstatus = 0xA000;
        break;
    case __COMMCTL_DBG_ISR_VECTOR:
      //ret = CYGNUM_HAL_INTERRUPT_CPM_SMC1;
      ret = CYGNUM_HAL_INTERRUPT_SCC1;

      //ret = 0x01;
        break;
    case __COMMCTL_SET_TIMEOUT:
    {
        va_list ap;

        va_start(ap, __func);

        ret = msec_timeout;
        msec_timeout = va_arg(ap, cyg_uint32);

        va_end(ap);
    }        
    default:
        break;
    }
    CYGARC_HAL_RESTORE_GP();
    return ret;
}

// EOF hal_aux.c
