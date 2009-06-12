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
// Author(s):   jskov
// Contributors:jskov
// Date:        2001-03-16
// Purpose:     HAL diagnostic output
// Description: Implementations of HAL diagnostic output support.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/hal.h>
#include CYGBLD_HAL_PLATFORM_H

#include <cyg/infra/cyg_type.h>         // base types

#include <cyg/hal/hal_arch.h>           // SAVE/RESTORE GP macros
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_if.h>             // interface API
#include <cyg/hal/hal_intr.h>           // HAL_ENABLE/MASK/UNMASK_INTERRUPTS
#include <cyg/hal/hal_misc.h>           // Helper functions
#include <cyg/hal/drv_api.h>            // CYG_ISR_HANDLED

#include <cyg/hal/plf_io.h>             // SIO registers

#define SIO_BRDDIV (((CYGNUM_HAL_CPUCLOCK/2/16/CYGNUM_HAL_VIRTUAL_VECTOR_CHANNELS_DEFAULT_BAUD)<<4))

//-----------------------------------------------------------------------------
typedef struct {
    cyg_uint8* base;
    cyg_int32 msec_timeout;
    int isr_vector_rx;
    int isr_vector_tx;
} channel_data_t;

//-----------------------------------------------------------------------------

char hextab[] = "0123456789ABCDEF";

void putc_ser(int c)
{
    cyg_uint8* base = (cyg_uint8*)KS32C_UART1_BASE;
    cyg_uint32 status;
    do {
        HAL_READ_UINT32(base+KS32C_UART_STAT, status);
    } while ((status & KS32C_UART_STAT_TXE) == 0);

    HAL_WRITE_UINT32(base+KS32C_UART_TXBUF, c);
}

void putint(int a)
{
    int i;
    putc_ser('0');
    putc_ser('x');
    for (i = 0; i < 8; i++) {
        putc_ser(hextab[(a>>(28-(4*i))) & 0x0f]);
    }
    putc_ser('\r');
    putc_ser('\n');
}

void
init_ser(void)
{
    cyg_uint8* base = (cyg_uint8*)KS32C_UART1_BASE;

    // 8-1-no parity.
    HAL_WRITE_UINT32(base+KS32C_UART_LCON,
                     KS32C_UART_LCON_8_DBITS|KS32C_UART_LCON_1_SBITS|KS32C_UART_LCON_NO_PARITY);

    // Mask interrupts.
    HAL_INTERRUPT_MASK(CYGNUM_HAL_INTERRUPT_UART0_RX);
    HAL_INTERRUPT_MASK(CYGNUM_HAL_INTERRUPT_UART0_TX);

    HAL_WRITE_UINT32(base+KS32C_UART_BRDIV, SIO_BRDDIV);
}

//-----------------------------------------------------------------------------

static void
cyg_hal_plf_serial_init_channel(void* __ch_data)
{
    cyg_uint8* base = ((channel_data_t*)__ch_data)->base;

    // 8-1-no parity.
    HAL_WRITE_UINT32(base+KS32C_UART_LCON, 
                     KS32C_UART_LCON_8_DBITS|KS32C_UART_LCON_1_SBITS|KS32C_UART_LCON_NO_PARITY);

    HAL_WRITE_UINT32(base+KS32C_UART_BRDIV, SIO_BRDDIV);

    // Mask interrupts
    HAL_INTERRUPT_MASK(((channel_data_t*)__ch_data)->isr_vector_rx);
    HAL_INTERRUPT_MASK(((channel_data_t*)__ch_data)->isr_vector_tx);

    // Enable RX and TX
    HAL_WRITE_UINT32(base+KS32C_UART_CON, KS32C_UART_CON_RXM_INT|KS32C_UART_CON_TXM_INT);
}

void
cyg_hal_plf_serial_putc(void *__ch_data, char c)
{
    cyg_uint8* base = ((channel_data_t*)__ch_data)->base;
    cyg_uint32 status, ch;
    CYGARC_HAL_SAVE_GP();

    do {
        HAL_READ_UINT32(base+KS32C_UART_STAT, status);
    } while ((status & KS32C_UART_STAT_TXE) == 0);

    ch = (cyg_uint32)c;
    HAL_WRITE_UINT32(base+KS32C_UART_TXBUF, ch);

    CYGARC_HAL_RESTORE_GP();
}

static cyg_bool
cyg_hal_plf_serial_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_uint8* base = chan->base;
    cyg_uint32 stat;
    cyg_uint32 c;

    HAL_READ_UINT32(base+KS32C_UART_STAT, stat);
    if ((stat & KS32C_UART_STAT_RDR) == 0)
        return false;

    HAL_READ_UINT32(base+KS32C_UART_RXBUF, c);
    *ch = (cyg_uint8)(c & 0xff);

    HAL_INTERRUPT_ACKNOWLEDGE(chan->isr_vector_rx);

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
    int ret = 0;
    CYGARC_HAL_SAVE_GP();

    switch (__func) {
    case __COMMCTL_IRQ_ENABLE:
        irq_state = 1;
        HAL_INTERRUPT_ACKNOWLEDGE(chan->isr_vector_rx);
        HAL_INTERRUPT_UNMASK(chan->isr_vector_rx);
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
        irq_state = 0;
        HAL_INTERRUPT_MASK(chan->isr_vector_rx);
        break;
    case __COMMCTL_DBG_ISR_VECTOR:
        ret = chan->isr_vector_rx;
        break;
    case __COMMCTL_SET_TIMEOUT:
    {
        va_list ap;

        va_start(ap, __func);

        ret = chan->msec_timeout;
        chan->msec_timeout = va_arg(ap, cyg_uint32);

        va_end(ap);
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
    int res = 0;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_uint32 c;
    cyg_uint8 ch;
    cyg_uint32 stat;
    CYGARC_HAL_SAVE_GP();

    *__ctrlc = 0;
    HAL_READ_UINT32(chan->base+KS32C_UART_STAT, stat);
    if ( (stat & KS32C_UART_STAT_RDR) != 0 ) {

        HAL_READ_UINT32(chan->base+KS32C_UART_RXBUF, c);
        ch = (cyg_uint8)(c & 0xff);
        if( cyg_hal_is_break( &ch , 1 ) )
            *__ctrlc = 1;

        res = CYG_ISR_HANDLED;
    }

    HAL_INTERRUPT_ACKNOWLEDGE(chan->isr_vector_rx);

    CYGARC_HAL_RESTORE_GP();
    return res;
}

static channel_data_t ks32c_ser_channels[2] = {
    { (cyg_uint8*)KS32C_UART0_BASE, 1000, CYGNUM_HAL_INTERRUPT_UART0_RX, CYGNUM_HAL_INTERRUPT_UART0_TX },
    { (cyg_uint8*)KS32C_UART1_BASE, 1000, CYGNUM_HAL_INTERRUPT_UART1_RX, CYGNUM_HAL_INTERRUPT_UART1_TX }
};

static void
cyg_hal_plf_serial_init(void)
{
    hal_virtual_comm_table_t* comm;
    int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

    // Init channels
    cyg_hal_plf_serial_init_channel(&ks32c_ser_channels[0]);
    cyg_hal_plf_serial_init_channel(&ks32c_ser_channels[1]);

    // Setup procs in the vector table

    // Set channel 0
    CYGACC_CALL_IF_SET_CONSOLE_COMM(0);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &ks32c_ser_channels[0]);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_plf_serial_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_plf_serial_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_plf_serial_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_plf_serial_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_plf_serial_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_plf_serial_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_plf_serial_getc_timeout);

    // Set channel 1
    CYGACC_CALL_IF_SET_CONSOLE_COMM(1);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &ks32c_ser_channels[1]);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_plf_serial_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_plf_serial_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_plf_serial_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_plf_serial_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_plf_serial_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_plf_serial_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_plf_serial_getc_timeout);

    // Restore original console
    CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);
}

void
cyg_hal_plf_comms_init(void)
{
    static int initialized = 0;

    if (initialized)
        return;

    initialized = 1;

    cyg_hal_plf_serial_init();
}

//-----------------------------------------------------------------------------
// LED
void
hal_diag_led(int mask)
{
    cyg_uint32 l;

    HAL_READ_UINT32(KS32C_IOPDATA, l);
    l &= ~(AIM711_GPIO_LED0|AIM711_GPIO_LED1|AIM711_GPIO_LED2);
    l |= (~mask & (AIM711_GPIO_LED0|AIM711_GPIO_LED1|AIM711_GPIO_LED2));
    HAL_WRITE_UINT32(KS32C_IOPDATA, l);
}

//-----------------------------------------------------------------------------
// End of hal_diag.c
