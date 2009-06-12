/*=============================================================================
//
//      hal_diag.c
//
//      HAL diagnostic output code
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
// Author(s):   msalter
// Contributors:msalter, gthomas
// Date:        2000-10-10
// Purpose:     HAL diagnostic output
// Description: Implementations of HAL diagnostic output support.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_arch.h>           // basic machine info
#include <cyg/hal/hal_intr.h>           // interrupt macros
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_if.h>             // calling interface API
#include <cyg/hal/hal_misc.h>           // helper functions
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_iop310.h>         // Hardware definitions
#include <cyg/hal/drv_api.h>            // cyg_drv_interrupt_acknowledge


static void cyg_hal_plf_serial_init(void);

void
cyg_hal_plf_comms_init(void)
{
    static int initialized = 0;

    if (initialized)
        return;

    initialized = 1;

    cyg_hal_plf_serial_init();
}

//=============================================================================
// Serial driver
//=============================================================================

//-----------------------------------------------------------------------------
// Based on 3.6864 MHz xtal
struct baud_config {
    cyg_int32 baud_rate;
    cyg_uint8 msb;
    cyg_uint8 lsb;
};

struct baud_config baud_conf[] = {
    {9600,   0x00, 0x0c},
    {19200,  0x00, 0x06},
    {38400,  0x00, 0x03},
    {57600,  0x00, 0x02},
    {115200, 0x00, 0x01}};

//-----------------------------------------------------------------------------
// Define the serial registers. The Cogent board is equipped with a 16552
// serial chip.
#define CYG_DEV_SERIAL_RBR   0x00  // receiver buffer register, read, dlab = 0
#define CYG_DEV_SERIAL_THR   0x00 // transmitter holding register, write, dlab = 0
#define CYG_DEV_SERIAL_DLL   0x00 // divisor latch (LS), read/write, dlab = 1
#define CYG_DEV_SERIAL_IER   0x01 // interrupt enable register, read/write, dlab = 0
#define CYG_DEV_SERIAL_DLM   0x01 // divisor latch (MS), read/write, dlab = 1
#define CYG_DEV_SERIAL_IIR   0x02 // interrupt identification register, read, dlab = 0
#define CYG_DEV_SERIAL_FCR   0x02 // fifo control register, write, dlab = 0
#define CYG_DEV_SERIAL_LCR   0x03 // line control register, write
#define CYG_DEV_SERIAL_MCR   0x04 // modem control register, write
#define CYG_DEV_SERIAL_LSR   0x05 // line status register, read
#define CYG_DEV_SERIAL_MSR   0x06 // modem status register, read
#define CYG_DEV_SERIAL_SCR   0x07 // scratch pad register

// The interrupt enable register bits.
#define SIO_IER_ERDAI   0x01            // enable received data available irq
#define SIO_IER_ETHREI  0x02            // enable THR empty interrupt
#define SIO_IER_ELSI    0x04            // enable receiver line status irq
#define SIO_IER_EMSI    0x08            // enable modem status interrupt

// The interrupt identification register bits.
#define SIO_IIR_IP      0x01            // 0 if interrupt pending
#define SIO_IIR_ID_MASK 0x0e            // mask for interrupt ID bits
#define ISR_Tx  0x02
#define ISR_Rx  0x04

// The line status register bits.
#define SIO_LSR_DR      0x01            // data ready
#define SIO_LSR_OE      0x02            // overrun error
#define SIO_LSR_PE      0x04            // parity error
#define SIO_LSR_FE      0x08            // framing error
#define SIO_LSR_BI      0x10            // break interrupt
#define SIO_LSR_THRE    0x20            // transmitter holding register empty
#define SIO_LSR_TEMT    0x40            // transmitter register empty
#define SIO_LSR_ERR     0x80            // any error condition

// The modem status register bits.
#define SIO_MSR_DCTS  0x01              // delta clear to send
#define SIO_MSR_DDSR  0x02              // delta data set ready
#define SIO_MSR_TERI  0x04              // trailing edge ring indicator
#define SIO_MSR_DDCD  0x08              // delta data carrier detect
#define SIO_MSR_CTS   0x10              // clear to send
#define SIO_MSR_DSR   0x20              // data set ready
#define SIO_MSR_RI    0x40              // ring indicator
#define SIO_MSR_DCD   0x80              // data carrier detect

// The line control register bits.
#define SIO_LCR_WLS0   0x01             // word length select bit 0
#define SIO_LCR_WLS1   0x02             // word length select bit 1
#define SIO_LCR_STB    0x04             // number of stop bits
#define SIO_LCR_PEN    0x08             // parity enable
#define SIO_LCR_EPS    0x10             // even parity select
#define SIO_LCR_SP     0x20             // stick parity
#define SIO_LCR_SB     0x40             // set break
#define SIO_LCR_DLAB   0x80             // divisor latch access bit

// The FIFO control register
#define SIO_FCR_FCR0   0x01             // enable xmit and rcvr fifos
#define SIO_FCR_FCR1   0x02             // clear RCVR FIFO
#define SIO_FCR_FCR2   0x04             // clear XMIT FIFO


//-----------------------------------------------------------------------------
typedef struct {
    cyg_uint8* base;
    cyg_int32 msec_timeout;
    int isr_vector;
    cyg_int32 baud_rate;
} channel_data_t;

//-----------------------------------------------------------------------------
static int
set_baud( channel_data_t *chan )
{
    cyg_uint8* base = chan->base;
    cyg_uint8 i;

    for (i=0; i<(sizeof(baud_conf)/sizeof(baud_conf[0])); i++)
    {
        if (chan->baud_rate == baud_conf[i].baud_rate) {
            cyg_uint8 lcr;
            HAL_READ_UINT8(base+CYG_DEV_SERIAL_LCR, lcr);
            HAL_WRITE_UINT8(base+CYG_DEV_SERIAL_LCR, lcr|SIO_LCR_DLAB);
            HAL_WRITE_UINT8(base+CYG_DEV_SERIAL_DLL, baud_conf[i].lsb);
            HAL_WRITE_UINT8(base+CYG_DEV_SERIAL_DLM, baud_conf[i].msb);
            HAL_WRITE_UINT8(base+CYG_DEV_SERIAL_LCR, lcr);
            return 1;
        }
    }
    return -1;
}

static void
init_serial_channel(channel_data_t* __ch_data)
{
    cyg_uint8* base = __ch_data->base;
    channel_data_t* chan = (channel_data_t*)__ch_data;

    // 8-1-no parity.
    HAL_WRITE_UINT8(base+CYG_DEV_SERIAL_LCR, SIO_LCR_WLS0 | SIO_LCR_WLS1);
    chan->baud_rate = CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD;
    set_baud( chan );
    HAL_WRITE_UINT8(base+CYG_DEV_SERIAL_FCR, 0x07);  // Enable & clear FIFO
}

static cyg_bool
cyg_hal_plf_serial_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
    cyg_uint8* base = ((channel_data_t*)__ch_data)->base;
    cyg_uint8 lsr;

    HAL_READ_UINT8(base+CYG_DEV_SERIAL_LSR, lsr);
    if ((lsr & SIO_LSR_DR) == 0)
        return false;

    HAL_READ_UINT8(base+CYG_DEV_SERIAL_RBR, *ch);

    return true;
}


cyg_uint8
cyg_hal_plf_serial_getc(void* __ch_data)
{
    cyg_uint8 ch;
    CYGARC_HAL_SAVE_GP();

    while(!cyg_hal_plf_serial_getc_nonblock(__ch_data, &ch));

    CYGARC_HAL_RESTORE_GP();
    return ch;
}

void
cyg_hal_plf_serial_putc(void* __ch_data, cyg_uint8 c)
{
    cyg_uint8* base = ((channel_data_t*)__ch_data)->base;
    cyg_uint8 lsr;
    CYGARC_HAL_SAVE_GP();

    do {
        HAL_READ_UINT8(base+CYG_DEV_SERIAL_LSR, lsr);
    } while ((lsr & SIO_LSR_THRE) == 0);

    HAL_WRITE_UINT8(base+CYG_DEV_SERIAL_THR, c);

    // Hang around until the character has been safely sent.
    do {
        HAL_READ_UINT8(base+CYG_DEV_SERIAL_LSR, lsr);
    } while ((lsr & SIO_LSR_THRE) == 0);

    CYGARC_HAL_RESTORE_GP();
}

static channel_data_t channels[2] = {
#if CYGHWR_HAL_ARM_IOP310_SERIAL_PORTA != 0
    { (cyg_uint8*)IOP310_SERIAL_PORT_A, 1000, CYGNUM_HAL_INTERRUPT_SERIAL_A},
#endif
#if CYGHWR_HAL_ARM_IOP310_SERIAL_PORTB != 0
    { (cyg_uint8*)IOP310_SERIAL_PORT_B, 1000, CYGNUM_HAL_INTERRUPT_SERIAL_B},
#endif
};

static void
cyg_hal_plf_serial_write(void* __ch_data, const cyg_uint8* __buf, 
                         cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        cyg_hal_plf_serial_putc(__ch_data, *__buf++);

    CYGARC_HAL_RESTORE_GP();
}

static void
cyg_hal_plf_serial_read(void* __ch_data, cyg_uint8* __buf, cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        *__buf++ = cyg_hal_plf_serial_getc(__ch_data);

    CYGARC_HAL_RESTORE_GP();
}

cyg_bool
cyg_hal_plf_serial_getc_timeout(void* __ch_data, cyg_uint8* ch)
{
    int delay_count;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_bool res;
    CYGARC_HAL_SAVE_GP();

    delay_count = chan->msec_timeout * 10; // delay in .1 ms steps
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
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_uint8 ier;
    int ret = 0;
    CYGARC_HAL_SAVE_GP();

    switch (__func) {
    case __COMMCTL_IRQ_ENABLE:
        HAL_INTERRUPT_UNMASK(chan->isr_vector);
        HAL_INTERRUPT_SET_LEVEL(chan->isr_vector, 1);
        HAL_READ_UINT8(chan->base+CYG_DEV_SERIAL_IER, ier);
        ier |= SIO_IER_ERDAI;
        HAL_WRITE_UINT8(chan->base+CYG_DEV_SERIAL_IER, ier);
        irq_state = 1;
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
        irq_state = 0;
        HAL_INTERRUPT_MASK(chan->isr_vector);
        HAL_READ_UINT8(chan->base+CYG_DEV_SERIAL_IER, ier);
        ier &= ~SIO_IER_ERDAI;
        HAL_WRITE_UINT8(chan->base+CYG_DEV_SERIAL_IER, ier);
        break;
    case __COMMCTL_DBG_ISR_VECTOR:
        ret = chan->isr_vector;
        break;
    case __COMMCTL_SET_TIMEOUT:
    {
        va_list ap;

        va_start(ap, __func);

        ret = chan->msec_timeout;
        chan->msec_timeout = va_arg(ap, cyg_uint32);

        va_end(ap);
    }        
    case __COMMCTL_GETBAUD:
        ret = chan->baud_rate;
        break;
    case __COMMCTL_SETBAUD:
    {
        va_list ap;
        va_start(ap, __func);
        chan->baud_rate = va_arg(ap, cyg_int32);
        va_end(ap);
        ret = set_baud(chan);
        break;
    }
    default:
        break;
    }
    CYGARC_HAL_RESTORE_GP();
    return ret;
}

static int
cyg_hal_plf_serial_isr(void *__ch_data, int* __ctrlc, 
                       CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_uint8 _iir;
    int res = 0;
    CYGARC_HAL_SAVE_GP();

    HAL_READ_UINT8(chan->base+CYG_DEV_SERIAL_IIR, _iir);
    _iir &= SIO_IIR_ID_MASK;

    *__ctrlc = 0;
    if ( ISR_Rx == _iir ) {
        cyg_uint8 c, lsr;
        HAL_READ_UINT8(chan->base+CYG_DEV_SERIAL_LSR, lsr);
        if (lsr & SIO_LSR_DR) {

            HAL_READ_UINT8(chan->base+CYG_DEV_SERIAL_RBR, c);

            if( cyg_hal_is_break( &c , 1 ) )
                *__ctrlc = 1;
        }

        // Acknowledge the interrupt
        HAL_INTERRUPT_ACKNOWLEDGE(chan->isr_vector);
        res = CYG_ISR_HANDLED;
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}

static void
cyg_hal_plf_serial_init(void)
{
    hal_virtual_comm_table_t* comm;
    int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

    // Disable interrupts.
    HAL_INTERRUPT_MASK(channels[0].isr_vector);
#if CYGNUM_HAL_VIRTUAL_VECTOR_COMM_CHANNELS > 1
    HAL_INTERRUPT_MASK(channels[1].isr_vector);
#endif

    // Init channels
    init_serial_channel(&channels[0]);
#if CYGNUM_HAL_VIRTUAL_VECTOR_COMM_CHANNELS > 1
    init_serial_channel(&channels[1]);
#endif

    // Setup procs in the vector table

    // Set channel 0
    CYGACC_CALL_IF_SET_CONSOLE_COMM(0);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &channels[0]);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_plf_serial_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_plf_serial_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_plf_serial_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_plf_serial_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_plf_serial_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_plf_serial_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_plf_serial_getc_timeout);

#if CYGNUM_HAL_VIRTUAL_VECTOR_COMM_CHANNELS > 1
    // Set channel 1
    CYGACC_CALL_IF_SET_CONSOLE_COMM(1);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &channels[1]);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_plf_serial_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_plf_serial_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_plf_serial_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_plf_serial_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_plf_serial_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_plf_serial_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_plf_serial_getc_timeout);
#endif
    
    // Restore original console
    CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);
}

/*---------------------------------------------------------------------------*/
/* End of hal_diag.c */
