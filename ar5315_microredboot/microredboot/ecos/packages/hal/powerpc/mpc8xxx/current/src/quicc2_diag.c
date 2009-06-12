//=============================================================================
//
//      quicc2_diag.c
//
//      HAL diagnostic I/O support routines for MPC8xxx/QUICC2
//
//=============================================================================
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
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/mpc8xxx.h>            // Needed for IMMR structure

#define PORT_IS_SMC 1
#define PORT_IS_SCC 0

#define NUM(t) sizeof(t)/sizeof(t[0])

struct port_info {
    short                             Txnum;   // Number of Tx buffers
    short                             Rxnum;   // Number of Rx buffers
    short                             intnum;  // Interrupt bit
    short                             is_smc;  // 1 => SMC, 0=> SCC
    int                               cpm_page;
    int                               timeout; // Timeout in msec
    int                               pram;    // [Pointer] to PRAM data
    int                               regs;    // [Pointer] to control registers
    int                               brg;     // Baud rate generator
    volatile struct cp_bufdesc *next_rxbd;
    int                               irq_state;// Interrupt state
    int                               init;    // Has port been initialized?
};

static struct port_info ports[] = {
#if CYGNUM_HAL_MPC8XXX_SMC1 > 0
    { 1, 4, CYGNUM_HAL_INTERRUPT_SMC1, PORT_IS_SMC, SMC1_PAGE_SUBBLOCK, 1000,
      DPRAM_SMC1_OFFSET,
      (int)&((t_PQ2IMM *)0)->smc_regs[SMC1],
      (int)&((t_PQ2IMM *)0)->brgs_brgc7
    }, 
#endif
#if CYGNUM_HAL_MPC8XXX_SCC1 > 0
    { 1, 4, CYGNUM_HAL_INTERRUPT_SCC1, PORT_IS_SCC, SCC1_PAGE_SUBBLOCK, 1000,
      (int)&((t_PQ2IMM *)0)->pram.serials.scc_pram[SCC1],
      (int)&((t_PQ2IMM *)0)->scc_regs[SCC1],
      (int)&((t_PQ2IMM *)0)->brgs_brgc1
    }, 
#endif
};

// For Baud Rate Calculation, see MPC8260 PowerQUICC II User's Manual
// 16.3 UART Baud Rate Examples, page 16-5.
#define UART_BIT_RATE(n) \
    ((((int)(((CYGHWR_HAL_POWERPC_CPM_SPEED*2)*1000000)/16))/(n * 16))-1)
#define UART_BAUD_RATE CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD


// Function prototypes
static cyg_uint8 cyg_hal_plf_serial_getc(void* __ch_data);
static cyg_bool  cyg_hal_plf_serial_getc_nonblock(void* __ch_data, cyg_uint8* ch);
static cyg_bool  cyg_hal_plf_serial_getc_timeout(void* __ch_data, cyg_uint8* ch);
static void      cyg_hal_plf_serial_putc(void* __ch_data, cyg_uint8 ch);
static void      cyg_hal_plf_serial_write(void* __ch_data, const cyg_uint8* __buf, 
                                          cyg_uint32 __len);
static void      cyg_hal_plf_serial_read(void* __ch_data, cyg_uint8* __buf, cyg_uint32 __len);
static void      cyg_hal_plf_smcx_init_channel(struct port_info *info, int page);
static void      cyg_hal_plf_sccx_init_channel(struct port_info *info, int page);
static int       cyg_hal_plf_smcx_isr(void *__ch_data, int* __ctrlc, 
                                      CYG_ADDRWORD __vector, CYG_ADDRWORD __data);
static int       cyg_hal_plf_sccx_isr(void *__ch_data, int* __ctrlc, 
                                      CYG_ADDRWORD __vector, CYG_ADDRWORD __data);
static int       cyg_hal_plf_serial_control(void *__ch_data, __comm_control_cmd_t __func, ...);

static int       
cyg_hal_plf_sccx_isr(void *__ch_data, int* __ctrlc, 
                     CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
    struct port_info *info = (struct port_info *)__ch_data;
    volatile struct scc_regs_8260 *regs = (volatile struct scc_regs_8260*)((char *)IMM + info->regs);
    volatile t_Scc_Pram *pram = (volatile t_Scc_Pram *)((char *)IMM + info->pram);
    char ch;
    int res = 0;
    volatile struct cp_bufdesc *bd;

    *__ctrlc = 0;
    if (regs->scce & SCCE_Rx) {
        regs->scce = SMCE_Rx;

        /* rx buffer descriptors */
        bd = info->next_rxbd;

        if ((bd->ctrl & _BD_CTL_Ready) == 0) {
            
            // then there be a character waiting
            ch = bd->buffer[0];
            bd->length = 1;
            bd->ctrl   |= _BD_CTL_Ready | _BD_CTL_Int;
            if (bd->ctrl & _BD_CTL_Wrap) {
                bd = (struct cp_bufdesc *)((char *)IMM + pram->rbase);
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
    return res;
}

static int       
cyg_hal_plf_smcx_isr(void *__ch_data, int* __ctrlc, 
                     CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
    struct port_info *info = (struct port_info *)__ch_data;
    volatile struct smc_regs_8260 *regs = (volatile struct smc_regs_8260*)((char *)IMM + info->regs);
    t_Smc_Pram *pram = (t_Smc_Pram *)((char *)IMM + info->pram);
    char ch;
    int res = 0;
    volatile struct cp_bufdesc *bd;

    *__ctrlc = 0;
    if (regs->smc_smce & SMCE_Rx) {
        regs->smc_smce = SMCE_Rx;

        /* rx buffer descriptors */
        bd = info->next_rxbd;

        if ((bd->ctrl & _BD_CTL_Ready) == 0) {
            
            // then there be a character waiting
            ch = bd->buffer[0];
            bd->length = 1;
            bd->ctrl   |= _BD_CTL_Ready | _BD_CTL_Int;
            if (bd->ctrl & _BD_CTL_Wrap) {
                bd = (struct cp_bufdesc *)((char *)IMM + pram->rbase);
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
    return res;
}

/* Early initialization of comm channels.
 */
void
cyg_hal_plf_serial_init(void)
{
    hal_virtual_comm_table_t* comm;
    int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);
    int chan = 0;
    struct port_info *port;
    static int init = 0;

    if (init) return;
    init++;

    // Setup procs in the vector table    
    for (port = ports, chan = 0;  chan < NUM(ports);  chan++, port++) {        
        CYGACC_CALL_IF_SET_CONSOLE_COMM(chan);
        comm = CYGACC_CALL_IF_CONSOLE_PROCS();
        CYGACC_COMM_IF_CH_DATA_SET(*comm, port);
        CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_plf_serial_write);
        CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_plf_serial_read);
        CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_plf_serial_putc);
        CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_plf_serial_getc);
        CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_plf_serial_control);
        CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_plf_serial_getc_timeout);
        if (port->is_smc) {
            cyg_hal_plf_smcx_init_channel(port, port->cpm_page);
            CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_plf_smcx_isr);
        } else {
            cyg_hal_plf_sccx_init_channel(port, port->cpm_page);
            CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_plf_sccx_isr);
        }
    }

    // Restore original console
    CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);
}

static void
cyg_hal_plf_sccx_init_channel(struct port_info *info, int cpm_page)
{
    unsigned int rxbase, txbase;
    int i;
    struct cp_bufdesc *rxbd, *txbd;
    volatile struct scc_regs_8260 *regs = (volatile struct scc_regs_8260*)((char *)IMM + info->regs);
    volatile t_Scc_Pram *pram = (volatile t_Scc_Pram *)((char *)IMM + info->pram);

    if (info->init) return;
    info->init = 1;

    // Make sure device is stopped
    regs->gsmr_l &= DISABLE_TX_RX;
    while ((IMM->cpm_cpcr & CPCR_FLG) != READY_TO_RX_CMD); 
    IMM->cpm_cpcr = cpm_page |
        CPCR_STOP_TX |
        CPCR_FLG;             /* ISSUE COMMAND */
    while ((IMM->cpm_cpcr & CPCR_FLG) != READY_TO_RX_CMD); 

    // Allocate buffer descriptors + buffers (adjacent to descriptors)
    rxbase = _mpc8xxx_allocBd(sizeof(struct cp_bufdesc)*info->Rxnum + info->Rxnum);
    txbase = _mpc8xxx_allocBd(sizeof(struct cp_bufdesc)*info->Txnum + info->Txnum);

    // setup RX buffer descriptors 
    rxbd = (struct cp_bufdesc *)((char *)IMM + rxbase);
    info->next_rxbd = rxbd;
    for (i = 0;  i < info->Rxnum;  i++) {
        rxbd->length = 0;
        rxbd->buffer = ((char *)IMM + (rxbase+(info->Rxnum*sizeof(struct cp_bufdesc))))+i;
        rxbd->ctrl   = _BD_CTL_Ready | _BD_CTL_Int;
        rxbd++;
    }
    rxbd--;
    rxbd->ctrl   |= _BD_CTL_Wrap;

    // setup TX buffer descriptor
    txbd = (struct cp_bufdesc *)((char *)IMM + txbase);
    txbd->length = 1;
    txbd->buffer = ((char *)IMM + (txbase+(info->Txnum*sizeof(struct cp_bufdesc))));
    txbd->ctrl   = _BD_CTL_Wrap;

    // Set the baud rate generator.  Note: on the MPC8xxx, 
    // there are a number of BRGs, but the usage/layout is
    // somewhat restricted, so we rely on a fixed mapping.
    // See the setup in the platform init code for details.
    *(unsigned long *)((char *)IMM + info->brg) = 0x00010000 | (UART_BIT_RATE(UART_BAUD_RATE) << 1);

    // Rx, Tx function codes (used for access)
    pram->rfcr = 0x18;
    pram->tfcr = 0x18;
    regs->psmr = 0xB000;

    // Pointers to Rx & Tx buffer descriptor rings
    pram->rbase = rxbase;
    pram->tbase = txbase;

    // Max receive buffer length
    pram->mrblr = 1;
   
    // Mode register for 8N1
    regs->gsmr_h = 0x00000060;
    regs->gsmr_l = 0x00028004;  

    // Clear events
    regs->scce = ALL_ONES;
    regs->sccm = SCCE_Rx;

    // Init channel
    while ((IMM->cpm_cpcr & CPCR_FLG) != READY_TO_RX_CMD); 
    IMM->cpm_cpcr = cpm_page | 
        CPCR_INIT_TX_RX_PARAMS |
        CPCR_FLG;                 /* ISSUE COMMAND */
    while ((IMM->cpm_cpcr & CPCR_FLG) != READY_TO_RX_CMD); 

    /*-------------------------------------------------------------*/
    /* Set the ENT/ENR bits in the GSMR -- Enable Transmit/Receive */
    /*-------------------------------------------------------------*/

    regs->gsmr_l |= GSMR_L1_ENT | GSMR_L1_ENR;
#if defined(CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT) \
    || defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT)
    // Fill out the control Character Table.  Make the first entry 
    // an end of table line. 
    // cc[0] = 0x4003 ==> reject if char = 0x3, write to RCCR
    pram->SpecificProtocol.u.cc[0] = 0x4003;
    {
        int i;
        for (i = 0; i < 8; i++){
            pram->SpecificProtocol.u.cc[i] = 0x8000;
        }
    }
    
    pram->SpecificProtocol.u.rccm  = 0xc000;
#endif
}

static void
cyg_hal_plf_smcx_init_channel(struct port_info *info, int cpm_page)
{
    unsigned int rxbase, txbase;
    int i;
    struct cp_bufdesc *rxbd, *txbd;
    volatile struct smc_regs_8260 *regs = (volatile struct smc_regs_8260*)((char *)IMM + info->regs);
    t_Smc_Pram *uart_pram = (t_Smc_Pram *)((char *)IMM + info->pram);

    if (info->init) return;
    info->init = 1;

    // Make sure device is stopped
    while ((IMM->cpm_cpcr & CPCR_FLG) != READY_TO_RX_CMD); 
    IMM->cpm_cpcr = cpm_page |
        CPCR_STOP_TX |
        CPCR_FLG;             /* ISSUE COMMAND */
    while ((IMM->cpm_cpcr & CPCR_FLG) != READY_TO_RX_CMD); 

    // Allocate buffer descriptors + buffers (adjacent to descriptors)
    rxbase = _mpc8xxx_allocBd(sizeof(struct cp_bufdesc)*info->Rxnum + info->Rxnum);
    txbase = _mpc8xxx_allocBd(sizeof(struct cp_bufdesc)*info->Txnum + info->Txnum);

    // setup RX buffer descriptors 
    rxbd = (struct cp_bufdesc *)((char *)IMM + rxbase);
    info->next_rxbd = rxbd;
    for (i = 0;  i < info->Rxnum;  i++) {
        rxbd->length = 0;
        rxbd->buffer = ((char *)IMM + (rxbase+(info->Rxnum*sizeof(struct cp_bufdesc))))+i;
        rxbd->ctrl   = _BD_CTL_Ready | _BD_CTL_Int;
        rxbd++;
    }
    rxbd--;
    rxbd->ctrl   |= _BD_CTL_Wrap;

    // setup TX buffer descriptor
    txbd = (struct cp_bufdesc *)((char *)IMM + txbase);
    txbd->length = 1;
    txbd->buffer = ((char *)IMM + (txbase+(info->Txnum*sizeof(struct cp_bufdesc))));
    txbd->ctrl   = _BD_CTL_Wrap;

    // Set the baud rate generator.  Note: on the MPC8xxx, 
    // there are a number of BRGs, but the usage/layout is
    // somewhat restricted, so we rely on a fixed mapping.
    // See the setup in the platform init code for details.
    *(unsigned long *)((char *)IMM + info->brg) = 0x00010000 | (UART_BIT_RATE(UART_BAUD_RATE) << 1);

    // Rx, Tx function codes (used for access)
    uart_pram->rfcr = 0x18;
    uart_pram->tfcr = 0x18;

    // Pointers to Rx & Tx buffer descriptor rings
    uart_pram->rbase = rxbase;
    uart_pram->tbase = txbase;

    // Max receive buffer length
    uart_pram->mrblr = 1;
   
    // Mode register for 8N1
    regs->smc_smcmr = 0x4823;

    // Clear events
    regs->smc_smce = 0xFF;
    regs->smc_smcm = SMCE_Rx;

    // Init channel
    while ((IMM->cpm_cpcr & CPCR_FLG) != READY_TO_RX_CMD); 
    IMM->cpm_cpcr = cpm_page | 
        CPCR_INIT_TX_RX_PARAMS |
        CPCR_FLG;                 /* ISSUE COMMAND */
    while ((IMM->cpm_cpcr & CPCR_FLG) != READY_TO_RX_CMD); 
}

static void
cyg_hal_plf_serial_putc(void* __ch_data, cyg_uint8 ch)
{
    volatile struct cp_bufdesc *bd;
    struct port_info *info = (struct port_info *)__ch_data;
    volatile t_Scc_Pram *uart_pram = (volatile t_Scc_Pram *)((char *)IMM + info->pram);
    int cache_state;

    /* tx buffer descriptor */
    bd = (struct cp_bufdesc *)((char *)IMM + uart_pram->tbptr);
    while (bd->ctrl & _BD_CTL_Ready) ;  // Wait for buffer free
    if (bd->ctrl & _BD_CTL_Int) {
        // This buffer has just completed interrupt output.  Reset bits
        bd->ctrl &= ~_BD_CTL_Int;
    }
    bd->length = 1;
    bd->buffer[0] = ch;

    // Flush cache if necessary - buffer may be in cacheable memory
    HAL_DCACHE_IS_ENABLED(cache_state);
    if (cache_state) {
      HAL_DCACHE_FLUSH(bd->buffer, 1);
    }

    bd->ctrl      |= _BD_CTL_Ready;
    while (bd->ctrl & _BD_CTL_Ready) ;  // Wait for buffer free
}

static cyg_bool
cyg_hal_plf_serial_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
    volatile struct cp_bufdesc *bd;
    struct port_info *info = (struct port_info *)__ch_data;
    volatile t_Scc_Pram *uart_pram = (volatile t_Scc_Pram *)((char *)IMM + info->pram);
    int cache_state;

    /* rx buffer descriptor */
    bd = info->next_rxbd;

    if (bd->ctrl & _BD_CTL_Ready)
        return false;

    *ch = bd->buffer[0];

    bd->length = 0;
    bd->buffer[0] = '\0';
    bd->ctrl |= _BD_CTL_Ready;
    if (bd->ctrl & _BD_CTL_Wrap) {
        bd = (struct cp_bufdesc *)((char *)IMM + uart_pram->rbase);
    } else {
        bd++;
    }
    info->next_rxbd = bd;

    // Note: the MPC8xxx does not seem to snoop/invalidate the data cache properly!
    HAL_DCACHE_IS_ENABLED(cache_state);
    if (cache_state) {
        HAL_DCACHE_INVALIDATE(bd->buffer, uart_pram->mrblr);  // Make sure no stale data
    }

    return true;
}

static cyg_uint8
cyg_hal_plf_serial_getc(void* __ch_data)
{
    cyg_uint8 ch;
    while(!cyg_hal_plf_serial_getc_nonblock(__ch_data, &ch));
    return ch;
}

static void
cyg_hal_plf_serial_write(void* __ch_data, const cyg_uint8* __buf, 
                         cyg_uint32 __len)
{
    while(__len-- > 0)
        cyg_hal_plf_serial_putc(__ch_data, *__buf++);
}

static void
cyg_hal_plf_serial_read(void* __ch_data, cyg_uint8* __buf, cyg_uint32 __len)
{
    while(__len-- > 0)
        *__buf++ = cyg_hal_plf_serial_getc(__ch_data);
}

cyg_int32 msec_timeout = 1000;

static cyg_bool
cyg_hal_plf_serial_getc_timeout(void* __ch_data, cyg_uint8* ch)
{
    int delay_count = msec_timeout * 10; // delay in .1 ms steps
    cyg_bool res;

    for(;;) {
        res = cyg_hal_plf_serial_getc_nonblock(__ch_data, ch);
        if (res || 0 == delay_count--)
            break;
        
        CYGACC_CALL_IF_DELAY_US(100);
    }
    return res;
}

static int
cyg_hal_plf_serial_control(void *__ch_data, __comm_control_cmd_t __func, ...)
{
    int ret = 0;
    struct port_info *info = (struct port_info *)__ch_data;

    switch (__func) {
    case __COMMCTL_IRQ_ENABLE:
        HAL_INTERRUPT_UNMASK(info->intnum);
        info->irq_state = 1;
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = info->irq_state;
        info->irq_state = 0;
        HAL_INTERRUPT_MASK(info->intnum);
        break;
    case __COMMCTL_DBG_ISR_VECTOR:
        ret = info->intnum;
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
    return ret;
}

// EOF hal_aux.c
