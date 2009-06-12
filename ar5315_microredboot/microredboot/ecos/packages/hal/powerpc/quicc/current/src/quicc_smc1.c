//==========================================================================
//
//      quicc_smc1.c
//
//      PowerPC QUICC basic Serial IO using port(s) SMC1/SMC2/SCC1/SCC2/SCC3
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
// Purpose:      Provide basic Serial IO for MPC8xx boards (like Motorola MBX)
// Description:  Serial IO for MPC8xx boards which connect their debug channel
//               to SMCx or SCCx; or any QUICC user who wants to use SMCx/SCCx
// Usage:
// Notes:        
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/hal_powerpc_quicc.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_cache.h>

#include <cyg/hal/hal_arch.h>

// eCos headers decribing PowerQUICC:
#include <cyg/hal/quicc/ppc8xx.h>

#include <cyg/hal/quicc/quicc_smc1.h>

#include <cyg/hal/hal_stub.h>           // target_register_t
#include <cyg/hal/hal_intr.h>           // HAL_INTERRUPT_UNMASK(...)
#include <cyg/hal/hal_if.h>             // Calling interface definitions
#include <cyg/hal/hal_misc.h>           // Helper functions
#include <cyg/hal/drv_api.h>            // CYG_ISR_HANDLED
#include <string.h>                     // memset

#define UART_BIT_RATE(n) ((((int)(CYGHWR_HAL_POWERPC_BOARD_SPEED*1000000)/16)/n)-1)
#define UART_BAUD_RATE CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD

// Note: buffers will be placed just after descriptors
// Sufficient space should be provided between descrptors
// for the buffers (single characters)

struct port_info {
    int                         Txnum;   // Number of Tx buffers
    int                         Rxnum;   // Number of Rx buffers
    int                         intnum;  // Interrupt bit
    int                         timeout; // Timeout in msec
    int                         pram;    // [Pointer] to PRAM data
    int                         regs;    // [Pointer] to control registers
    volatile struct cp_bufdesc *next_rxbd;
    int                         irq;     // Interrupt state
    int                         init;    // Has port been initialized?
    volatile unsigned long     *brg;     // Baud rate generator
};

static struct port_info ports[] = {
#if CYGNUM_HAL_QUICC_SMC1 > 0
    { 1, 4, CYGNUM_HAL_INTERRUPT_CPM_SMC1, 1000,
      (int)&((EPPC *)0)->pram[2].scc.pothers.smc_modem.psmc.u, 
      (int)&((EPPC *)0)->smc_regs[0]
    }, 
#endif
#if CYGNUM_HAL_QUICC_SMC2 > 0
    { 1, 4, CYGNUM_HAL_INTERRUPT_CPM_SMC2_PIP, 1000,
      (int)&((EPPC *)0)->pram[3].scc.pothers.smc_modem.psmc.u, 
      (int)&((EPPC *)0)->smc_regs[1]
    }, 
#endif
#if CYGNUM_HAL_QUICC_SCC1 > 0
    { 1, 4, CYGNUM_HAL_INTERRUPT_CPM_SCC1, 1000,
      (int)&((EPPC *)0)->pram[0].scc.pscc.u, 
      (int)&((EPPC *)0)->scc_regs[0]
    },
#endif
#if CYGNUM_HAL_QUICC_SCC2 > 0
    { 1, 4, CYGNUM_HAL_INTERRUPT_CPM_SCC2, 1000,
      (int)&((EPPC *)0)->pram[1].scc.pscc.u, 
      (int)&((EPPC *)0)->scc_regs[1]
    },
#endif
#if CYGNUM_HAL_QUICC_SCC3 > 0
    { 1, 4, CYGNUM_HAL_INTERRUPT_CPM_SCC3, 1000,
      (int)&((EPPC *)0)->pram[2].scc.pscc.u, 
      (int)&((EPPC *)0)->scc_regs[2]
    },
#endif
};

/*
 *  Initialize SMCX as a uart.
 *
 *  Comments below reference Motorola's "MPC860 User Manual".
 *  The basic initialization steps are from Section 16.15.8
 *  of that manual.
 */	
static void
cyg_hal_smcx_init_channel(struct port_info *info, int port)
{
    EPPC *eppc = eppc_base();
    int i;
    volatile struct smc_uart_pram *uart_pram = (volatile struct smc_uart_pram *)((char *)eppc + info->pram);
    volatile struct smc_regs *regs = (volatile struct smc_regs *)((char *)eppc + info->regs);
    struct cp_bufdesc *txbd, *rxbd;

    if (info->init) return;
    info->init = 1;

    switch (port) {
#if CYGNUM_HAL_QUICC_SMC1 > 0
    case QUICC_CPM_SMC1:
        /*
         *  Set up the PortB pins for UART operation.
         *  Set PAR and DIR to allow SMCTXD1 and SMRXD1
         *  (Table 16-39)
         */
        eppc->pip_pbpar |= 0xc0;
        eppc->pip_pbdir &= ~0xc0;

        break;
#endif
#if CYGNUM_HAL_QUICC_SMC2 > 0
    case QUICC_CPM_SMC2:
        /*
         *  Set up the PortA pins for UART operation.
         *  Set PAR and DIR to allow SMCTXD2 and SMRXD2
         *  (Table 16-39)
         */
        eppc->pio_papar |= 0xc0;
        eppc->pio_padir &= ~0xc0;
        eppc->pio_paodr &= ~0xc0;

        break;
#endif
    }

    // Set up baud rate generator.  These are allocated from a
    // pool, based on the port number and type.  The allocator
    // will arrange to have the selected baud rate clock steered
    // to this device.
    info->brg = _mpc8xx_allocate_brg(port);
    *(info->brg) = 0x10000 | (UART_BIT_RATE(UART_BAUD_RATE)<<1);

    /*
     *  Set pointers to buffer descriptors.
     *  (Sections 16.15.4.1, 16.15.7.12, and 16.15.7.13)
     */
    uart_pram->rbase = _mpc8xx_allocBd(sizeof(struct cp_bufdesc)*info->Rxnum + info->Rxnum);
    uart_pram->tbase = _mpc8xx_allocBd(sizeof(struct cp_bufdesc)*info->Txnum + info->Txnum);

    /*
     *  SDMA & LCD bus request level 5
     *  (Section 16.10.2.1)
     */
    eppc->dma_sdcr = 1;

    /*
     *  Set Rx and Tx function code
     *  (Section 16.15.4.2)
     */
    uart_pram->rfcr = 0x18;
    uart_pram->tfcr = 0x18;

    /* max receive buffer length */
    uart_pram->mrblr = 1;

    /* disable max_idle feature */
    uart_pram->max_idl = 0;

    /* no last brk char received */
    uart_pram->brkln = 0;

    /* no break condition occurred */
    uart_pram->brkec = 0;

    /* 1 break char sent on top XMIT */
    uart_pram->brkcr = 1;

    /* setup RX buffer descriptors */
    rxbd = (struct cp_bufdesc *)((char *)eppc + uart_pram->rbase);
    info->next_rxbd = rxbd;
    for (i = 0;  i < info->Rxnum;  i++) {
        rxbd->length = 0;
        rxbd->buffer = ((char *)eppc + (uart_pram->rbase+(info->Rxnum*sizeof(struct cp_bufdesc))))+i;
        rxbd->ctrl   = QUICC_BD_CTL_Ready | QUICC_BD_CTL_Int;
        rxbd++;
    }
    rxbd--;
    rxbd->ctrl   |= QUICC_BD_CTL_Wrap;

    /* setup TX buffer descriptor */
    txbd = (struct cp_bufdesc *)((char *)eppc + uart_pram->tbase);
    txbd->length = 1;
    txbd->buffer = ((char *)eppc + (uart_pram->tbase+(info->Txnum*sizeof(struct cp_bufdesc))));
    txbd->ctrl   = 0x2000;

    /*
     *  Clear any previous events. Mask interrupts.
     *  (Section 16.15.7.14 and 16.15.7.15)
     */
    regs->smc_smce = 0xff;
    regs->smc_smcm = 1; // RX interrupts only, for ctrl-c

    /*
     *  Set 8,n,1 characters, then also enable rx and tx.
     *  (Section 16.15.7.11)
     */
    regs->smc_smcmr = 0x4820;
    regs->smc_smcmr = 0x4823;

    /*
     *  Init Rx & Tx params for SMCx
     */
    eppc->cp_cr = QUICC_CPM_CR_INIT_TXRX | port | QUICC_CPM_CR_BUSY;

    info->irq = 0;  // Interrupts not enabled
}


//#define UART_BUFSIZE 32

//static bsp_queue_t uart_queue;
//static char uart_buffer[UART_BUFSIZE];

#define QUICC_SMCE_TX     0x02    // Tx interrupt
#define QUICC_SMCE_RX     0x01    // Rx interrupt
#define QUICC_SMCMR_TEN       (1<<1)        // Enable transmitter
#define QUICC_SMCMR_REN       (1<<0)        // Enable receiver

#ifdef CYGDBG_DIAG_BUF
extern int enable_diag_uart;
#endif // CYGDBG_DIAG_BUF

static void 
cyg_hal_sxx_putc(void* __ch_data, cyg_uint8 ch)
{
    volatile struct cp_bufdesc *bd, *first;
    EPPC *eppc = eppc_base();
    struct port_info *info = (struct port_info *)__ch_data;
    volatile struct smc_uart_pram *uart_pram = (volatile struct smc_uart_pram *)((char *)eppc + info->pram);
    volatile struct smc_regs *regs = (volatile struct smc_regs *)((char *)eppc + info->regs);
    int timeout;
    int cache_state;
    CYGARC_HAL_SAVE_GP();

    /* tx buffer descriptor */
    bd = (struct cp_bufdesc *)((char *)eppc + uart_pram->tbptr);

    // Scan for a free buffer
    first = bd;
    while (bd->ctrl & QUICC_BD_CTL_Ready) {
        if (bd->ctrl & QUICC_BD_CTL_Wrap) {
            bd = (struct cp_bufdesc *)((char *)eppc + uart_pram->tbase);
        } else {
            bd++;
        }
        if (bd == first) break;
    }

    while (bd->ctrl & QUICC_BD_CTL_Ready) ;  // Wait for buffer free
    if (bd->ctrl & QUICC_BD_CTL_Int) {
        // This buffer has just completed interrupt output.  Reset bits
        bd->ctrl &= ~QUICC_BD_CTL_Int;
        bd->length = 0;
    }

    bd->length = 1;
    bd->buffer[0] = ch;
    bd->ctrl      |= QUICC_BD_CTL_Ready;
    // Flush cache if necessary - buffer may be in cacheable memory
    HAL_DCACHE_IS_ENABLED(cache_state);
    if (cache_state) {
      HAL_DCACHE_FLUSH(bd->buffer, 1);
    }

#ifdef CYGDBG_DIAG_BUF
        enable_diag_uart = 0;
#endif // CYGDBG_DIAG_BUF
    timeout = 0;
    while (bd->ctrl & QUICC_BD_CTL_Ready) {
// Wait until buffer free
        if (++timeout == 0x7FFFF) {
            // A really long time!
#ifdef CYGDBG_DIAG_BUF
            diag_printf("bd fail? bd: %x, ctrl: %x, tx state: %x\n", bd, bd->ctrl, uart_pram->tstate);
#endif // CYGDBG_DIAG_BUF
            regs->smc_smcmr &= ~QUICC_SMCMR_TEN;  // Disable transmitter
            bd->ctrl &= ~QUICC_BD_CTL_Ready;
            regs->smc_smcmr |= QUICC_SMCMR_TEN;   // Enable transmitter
            bd->ctrl |= QUICC_BD_CTL_Ready;
            timeout = 0;
#ifdef CYGDBG_DIAG_BUF
            diag_printf("bd retry? bd: %x, ctrl: %x, tx state: %x\n", bd, bd->ctrl, uart_pram->tstate);
            first = (struct cp_bufdesc *)((char *)eppc + uart_pram->tbase);
            while (true) {
                diag_printf("bd: %x, ctrl: %x, length: %x\n", first, first->ctrl, first->length);
                if (first->ctrl & QUICC_BD_CTL_Wrap) break;
                first++;
            }
#endif // CYGDBG_DIAG_BUF
        }
    }
    while (bd->ctrl & QUICC_BD_CTL_Ready) ;  // Wait until buffer free
    bd->length = 0;
#ifdef CYGDBG_DIAG_BUF
    enable_diag_uart = 1;
#endif // CYGDBG_DIAG_BUF

    CYGARC_HAL_RESTORE_GP();
}


/*
 * Get a character from a port, non-blocking
 * This function can be called on either an SMC or SCC port
 */
static cyg_bool
cyg_hal_sxx_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
    volatile struct cp_bufdesc *bd;
    EPPC *eppc = eppc_base();
    struct port_info *info = (struct port_info *)__ch_data;
    volatile struct smc_uart_pram *uart_pram = (volatile struct smc_uart_pram *)((char *)eppc + info->pram);
    int cache_state;

    /* rx buffer descriptor */
    bd = info->next_rxbd;

    if (bd->ctrl & QUICC_BD_CTL_Ready)
        return false;

    *ch = bd->buffer[0];

    bd->length = 0;
    bd->buffer[0] = '\0';
    bd->ctrl |= QUICC_BD_CTL_Ready;
    if (bd->ctrl & QUICC_BD_CTL_Wrap) {
        bd = (struct cp_bufdesc *)((char *)eppc + uart_pram->rbase);
    } else {
        bd++;
    }
    info->next_rxbd = bd;

    // Note: the MBX860 does not seem to snoop/invalidate the data cache properly!
    HAL_DCACHE_IS_ENABLED(cache_state);
    if (cache_state) {
        HAL_DCACHE_INVALIDATE(bd->buffer, uart_pram->mrblr);  // Make sure no stale data
    }

    return true;
}

/*
 * Get a character from a port, blocking
 * This function can be called on either an SMC or SCC port
 */
static cyg_uint8
cyg_hal_sxx_getc(void* __ch_data)
{
    cyg_uint8 ch;
    CYGARC_HAL_SAVE_GP();

    while(!cyg_hal_sxx_getc_nonblock(__ch_data, &ch));

    CYGARC_HAL_RESTORE_GP();
    return ch;
}


static void
cyg_hal_sxx_write(void* __ch_data, const cyg_uint8* __buf, 
                         cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        cyg_hal_sxx_putc(__ch_data, *__buf++);

    CYGARC_HAL_RESTORE_GP();
}

/*
 * Read a sequence of characters from a port
 * This function can be called on either an SMC or SCC port
 */
static void
cyg_hal_sxx_read(void* __ch_data, cyg_uint8* __buf, cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        *__buf++ = cyg_hal_sxx_getc(__ch_data);

    CYGARC_HAL_RESTORE_GP();
}

/*
 * Read a character from a port, with a timeout
 * This function can be called on either an SMC or SCC port
 */
static cyg_bool
cyg_hal_sxx_getc_timeout(void* __ch_data, cyg_uint8* ch)
{
    struct port_info *info = (struct port_info *)__ch_data;
    int delay_count = info->timeout * 10; // delay in .1 ms steps
    cyg_bool res;
    CYGARC_HAL_SAVE_GP();

    for(;;) {
        res = cyg_hal_sxx_getc_nonblock(__ch_data, ch);
        if (res || 0 == delay_count--)
            break;
        
        CYGACC_CALL_IF_DELAY_US(100);
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}

/*
 * Control/query the state of a port
 * This function can be called on either an SMC or SCC port
 */
static int
cyg_hal_sxx_control(void *__ch_data, __comm_control_cmd_t __func, ...)
{
    struct port_info *info = (struct port_info *)__ch_data;
    int ret = 0;
    CYGARC_HAL_SAVE_GP();

    switch (__func) {
    case __COMMCTL_IRQ_ENABLE:
        HAL_INTERRUPT_UNMASK(info->intnum);
        info->irq = 1;
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = info->irq;
        info->irq = 0;
        HAL_INTERRUPT_MASK(info->intnum);
        break;
    case __COMMCTL_DBG_ISR_VECTOR:
        ret = info->intnum;
        break;
    case __COMMCTL_SET_TIMEOUT:
    {
        va_list ap;

        va_start(ap, __func);

        ret = info->timeout;
        info->timeout = va_arg(ap, cyg_uint32);

        va_end(ap);
    }        
    default:
        break;
    }
    CYGARC_HAL_RESTORE_GP();
    return ret;
}

/*
 * Low-level interrupt (ISR) handler
 * This function can be called on only an SMC port
 */
static int
cyg_hal_smcx_isr(void *__ch_data, int* __ctrlc, 
                 CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
    EPPC *eppc = eppc_base();
    volatile struct cp_bufdesc *bd;
    struct port_info *info = (struct port_info *)__ch_data;
    volatile struct smc_regs *regs = (volatile struct smc_regs *)((char *)eppc + info->regs);
    volatile struct smc_uart_pram *uart_pram = (volatile struct smc_uart_pram *)((char *)eppc + info->pram);
    char ch;
    int res = 0;
    CYGARC_HAL_SAVE_GP();

    *__ctrlc = 0;
    if (regs->smc_smce & QUICC_SMCE_RX) {

        regs->smc_smce = QUICC_SMCE_RX;

        /* rx buffer descriptors */
        bd = info->next_rxbd;

        if ((bd->ctrl & QUICC_BD_CTL_Ready) == 0) {
            
            // then there be a character waiting
            ch = bd->buffer[0];
            bd->length = 1;
            bd->ctrl   |= QUICC_BD_CTL_Ready | QUICC_BD_CTL_Int;
            if (bd->ctrl & QUICC_BD_CTL_Wrap) {
                bd = (struct cp_bufdesc *)((char *)eppc + uart_pram->rbase);
            } else {
                bd++;
            }
            info->next_rxbd = bd;
        
            if( cyg_hal_is_break( &ch , 1 ) )
                *__ctrlc = 1;
        }

        // Interrupt handled. Acknowledge it.
        HAL_INTERRUPT_ACKNOWLEDGE(info->intnum);
        res = CYG_ISR_HANDLED;
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}

#if (CYGNUM_HAL_QUICC_SCC1+CYGNUM_HAL_QUICC_SCC2+CYGNUM_HAL_QUICC_SCC3) > 0
/*
 *  Initialize an SCC as a uart.
 *
 *  Comments below reference Motorola's "MPC860 User Manual".
 *  The basic initialization steps are from Section 16.15.8
 *  of that manual.
 */	
static void
cyg_hal_sccx_init_channel(struct port_info *info, int port)
{
    EPPC *eppc = eppc_base();
    int i;
    volatile struct uart_pram *uart_pram = (volatile struct uart_pram *)((char *)eppc + info->pram);
    volatile struct scc_regs *regs = (volatile struct scc_regs *)((char *)eppc + info->regs);
    struct cp_bufdesc *txbd, *rxbd;

    if (info->init) return;
    info->init = 1;

    /*
     *  Set up the Port pins for UART operation.
     */
    switch (port) {
#if CYGNUM_HAL_QUICC_SCC1 > 0
    case QUICC_CPM_SCC1:
        eppc->pio_papar |= 0x03;
        eppc->pio_padir &= ~0x03;
        eppc->pio_paodr &= ~0x03;

        /* CTS on PortC.11 */
        eppc->pio_pcdir &= 0x800;
        eppc->pio_pcpar &= 0x800;
        eppc->pio_pcso  |= 0x800;

        /* RTS on PortB.19 */
        eppc->pip_pbpar |= 0x1000;
        eppc->pip_pbdir |= 0x1000;

        break;
#endif
#if CYGNUM_HAL_QUICC_SCC2 > 0
    case QUICC_CPM_SCC2:
#error FIXME
        eppc->pio_papar |= 0x0C;
        eppc->pio_padir &= ~0x0C;
        eppc->pio_paodr &= ~0x0C;

        /* CTS on PortC.11 */
        eppc->pio_pcdir &= 0xC00;
        eppc->pio_pcpar &= 0xC00;
        eppc->pio_pcso  |= 0xC00;

        /* RTS on PortB.19 */
        eppc->pip_pbpar |= 0x2000;
        eppc->pip_pbdir |= 0x2000;

        break;
#endif
#if CYGNUM_HAL_QUICC_SCC3 > 0
    case QUICC_CPM_SCC3:
#if defined(CYGHWR_HAL_POWERPC_MPC8XX_850)
#if 0
// CAUTION!  Enabling these bits made the port get stuck :-(
        /* CTS/RTS/CD on PortC.4/5/13 */
        eppc->pio_pcdir &= 0x0C04;
        eppc->pio_pcpar &= 0x0C00;
//        eppc->pio_pcpar |= 0x0004;
        eppc->pio_pcso  |= 0x0C00;
#endif

        /* RxD/TxD on PortB.24/25 */
        eppc->pip_pbpar |= 0x00C0;
        eppc->pip_pbdir |= 0x00C0;
        eppc->pip_pbodr &= ~0x00C0;

#elif defined(CYGHWR_HAL_POWERPC_MPC8XX_852T)
        eppc->pio_papar |= 0x30;
        eppc->pio_padir &= ~0x30;
        eppc->pio_paodr &= ~0x30;
#else
#error "Cannot route SCC3 I/O"
#endif // 850T
        break;
#endif // SCC3
    }

    // Set up baud rate generator.  These are allocated from a
    // pool, based on the port number and type.  The allocator
    // will arrange to have the selected baud rate clock steered
    // to this device.
    info->brg = _mpc8xx_allocate_brg(port);
    *(info->brg) = 0x10000 | (UART_BIT_RATE(UART_BAUD_RATE)<<1);

    /*
     *  Set pointers to buffer descriptors.
     */
    uart_pram->rbase = _mpc8xx_allocBd(sizeof(struct cp_bufdesc)*info->Rxnum + info->Rxnum);
    uart_pram->tbase = _mpc8xx_allocBd(sizeof(struct cp_bufdesc)*info->Txnum + info->Txnum);

    /*
     *  SDMA & LCD bus request level 5
     */
    eppc->dma_sdcr = 1;

    /*
     *  Set Rx and Tx function code
     */
    uart_pram->rfcr = 0x18;
    uart_pram->tfcr = 0x18;

    /* max receive buffer length */
    uart_pram->mrblr = 1;

    /* disable max_idle feature */
    uart_pram->max_idl = 0;

    /* no last brk char received */
    uart_pram->brkln = 0;

    /* no break condition occurred */
    uart_pram->brkec = 0;

    /* 1 break char sent on top XMIT */
    uart_pram->brkcr = 1;

    /* character mask */
    uart_pram->rccm  = 0xC0FF;

    /* control characters */
    for (i = 0;  i < 8;  i++) {
        uart_pram->cc[i] = 0x8000;  // Mark unused
    }

    /* setup RX buffer descriptors */
    rxbd = (struct cp_bufdesc *)((char *)eppc + uart_pram->rbase);
    info->next_rxbd = rxbd;
    for (i = 0;  i < info->Rxnum;  i++) {
        rxbd->length = 0;
        rxbd->buffer = ((char *)eppc + (uart_pram->rbase+(info->Rxnum*sizeof(struct cp_bufdesc))))+i;
        rxbd->ctrl   = QUICC_BD_CTL_Ready | QUICC_BD_CTL_Int;
        rxbd++;
    }
    rxbd--;
    rxbd->ctrl   |= QUICC_BD_CTL_Wrap;

    /* setup TX buffer descriptor */
    txbd = (struct cp_bufdesc *)((char *)eppc + uart_pram->tbase);
    txbd->length = 0;
    txbd->buffer = ((char *)eppc + (uart_pram->tbase+(info->Txnum*sizeof(struct cp_bufdesc))));
    txbd->ctrl   = 0x2000;

    /*
     *  Clear any previous events. Mask interrupts.
     *  (Section 16.15.7.14 and 16.15.7.15)
     */
    regs->scc_scce = 0xffff;
    regs->scc_sccm = 1; // RX interrupts only, for ctrl-c

    /*
     *  Set 8,n,1 characters
     */
    regs->scc_psmr = (3<<12);
    regs->scc_gsmr_h = 0x20;          // 8bit FIFO
    regs->scc_gsmr_l = 0x00028004;    // 16x TxCLK, 16x RxCLK, UART

    /*
     *  Init Rx & Tx params for SCCX
     */
    eppc->cp_cr = QUICC_CPM_CR_INIT_TXRX | port | QUICC_CPM_CR_BUSY;

    regs->scc_gsmr_l |= 0x30;         // Enable Rx, Tx

    info->irq = 0;
}

static int
cyg_hal_sccx_isr(void *__ch_data, int* __ctrlc, 
                 CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
    EPPC *eppc = eppc_base();
    volatile struct cp_bufdesc *bd;
    struct port_info *info = (struct port_info *)__ch_data;
    volatile struct scc_regs *regs = (volatile struct scc_regs *)((char *)eppc + info->regs);
    volatile struct uart_pram *uart_pram = (volatile struct uart_pram *)((char *)eppc + info->pram);
    char ch;
    int res = 0;
    CYGARC_HAL_SAVE_GP();

    *__ctrlc = 0;
    if (regs->scc_scce & QUICC_SMCE_RX) {

        regs->scc_scce = QUICC_SMCE_RX;

        /* rx buffer descriptors */
        bd = info->next_rxbd;

        if ((bd->ctrl & QUICC_BD_CTL_Ready) == 0) {
            
            // then there be a character waiting
            ch = bd->buffer[0];
            bd->length = 1;
            bd->ctrl   |= QUICC_BD_CTL_Ready | QUICC_BD_CTL_Int;
            if (bd->ctrl & QUICC_BD_CTL_Wrap) {
                bd = (struct cp_bufdesc *)((char *)eppc + uart_pram->rbase);
            } else {
                bd++;
            }
            info->next_rxbd = bd;
        
            if( cyg_hal_is_break( &ch , 1 ) )
                *__ctrlc = 1;
        }

        // Interrupt handled. Acknowledge it.
        HAL_INTERRUPT_ACKNOWLEDGE(info->intnum);
        res = CYG_ISR_HANDLED;
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}
#endif // CYGNUM_HAL_QUICC_SCCX

/*
 * Early initialization of comm channels. Must not rely
 * on interrupts, yet. Interrupt operation can be enabled
 * in _bsp_board_init().
 */
void
cyg_hal_plf_serial_init(void)
{
    hal_virtual_comm_table_t* comm;
    int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

    static int init = 0;  // It's wrong to do this more than once
    int chan = 0;
    if (init) return;
    init++;

    // Setup procs in the vector table

#if CYGNUM_HAL_QUICC_SMC1 > 0
    // Set up SMC1
    cyg_hal_smcx_init_channel(&ports[chan], QUICC_CPM_SMC1);
    CYGACC_CALL_IF_SET_CONSOLE_COMM(chan);// Should be configurable!
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &ports[chan]);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_sxx_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_sxx_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_sxx_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_sxx_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_sxx_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_smcx_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_sxx_getc_timeout);
    chan++;
#endif

#if CYGNUM_HAL_QUICC_SMC2 > 0
    // Set up SMC2
    cyg_hal_smcx_init_channel(&ports[chan], QUICC_CPM_SMC2);
    CYGACC_CALL_IF_SET_CONSOLE_COMM(chan);// Should be configurable!
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &ports[chan]);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_sxx_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_sxx_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_sxx_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_sxx_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_sxx_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_smcx_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_sxx_getc_timeout);
    chan++;
#endif

#if CYGNUM_HAL_QUICC_SCC1 > 0
    // Set  up SCC1
    cyg_hal_sccx_init_channel(&ports[chan], QUICC_CPM_SCC1);
    CYGACC_CALL_IF_SET_CONSOLE_COMM(chan);// Should be configurable!
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &ports[chan]);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_sxx_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_sxx_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_sxx_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_sxx_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_sxx_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_sccx_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_sxx_getc_timeout);
    chan++;
#endif

#if CYGNUM_HAL_QUICC_SCC2 > 0
    // Set  up SCC2
    cyg_hal_sccx_init_channel(&ports[chan], QUICC_CPM_SCC2);
    CYGACC_CALL_IF_SET_CONSOLE_COMM(chan);// Should be configurable!
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &ports[chan]);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_sxx_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_sxx_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_sxx_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_sxx_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_sxx_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_sccx_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_sxx_getc_timeout);
    chan++;
#endif

#if CYGNUM_HAL_QUICC_SCC3 > 0
    // Set  up SCC3
    cyg_hal_sccx_init_channel(&ports[chan], QUICC_CPM_SCC3);
    CYGACC_CALL_IF_SET_CONSOLE_COMM(chan);// Should be configurable!
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &ports[chan]);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_sxx_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_sxx_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_sxx_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_sxx_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_sxx_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_sccx_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_sxx_getc_timeout);
    chan++;
#endif

    // Restore original console
    CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);
}

// EOF quicc_smc1.c
