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
// Copyright (C) 2003 Gary Thomas
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
// Author(s):   nickg, gthomas
// Contributors:nickg, gthomas
// Date:        1998-03-02
// Purpose:     HAL diagnostic output
// Description: Implementations of HAL diagnostic output support.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_arch.h>           // SAVE/RESTORE GP macros
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_if.h>             // interface API
#include <cyg/hal/hal_intr.h>           // HAL_ENABLE/MASK/UNMASK_INTERRUPTS
#include <cyg/hal/hal_misc.h>           // Helper functions
#include <cyg/hal/drv_api.h>            // CYG_ISR_HANDLED

#include <cyg/hal/gps4020.h>

//-----------------------------------------------------------------------------
typedef struct {
    volatile struct _gps4020_uart *base;
    cyg_int32             msec_timeout;
    int                   isr_vector;
    int                   int_state;
} channel_data_t;

static channel_data_t serial_channels[] = {
    { (volatile struct _gps4020_uart *)GPS4020_UART1, 1000, CYGNUM_HAL_INTERRUPT_UART1_RX, 0 },
};

//-----------------------------------------------------------------------------

static void
cyg_hal_plf_serial_init_channel(void* __ch_data)
{
    volatile struct _gps4020_uart *uart = ((channel_data_t*)__ch_data)->base;
    uart->mode = SMR_STOP_1 | SMR_PARITY_OFF | SMR_LENGTH_8;
    uart->baud = 0x15;  // FIXME - Magic for 57600
    uart->modem_control = SMR_DTR | SMR_RTS;
    uart->control = SCR_TEN | SCR_REN;
}

void
cyg_hal_plf_serial_putc(void *__ch_data, char c)
{
    volatile struct _gps4020_uart *uart = ((channel_data_t*)__ch_data)->base;
    // Wait for space for character
    do {
    } while ((uart->status & SSR_TxEmpty) == 0);
    uart->TxRx = c;
}

static cyg_bool
cyg_hal_plf_serial_getc_nonblock(void *__ch_data, cyg_uint8 *ch)
{
    volatile struct _gps4020_uart *uart = ((channel_data_t*)__ch_data)->base;
    if ((uart->status & SSR_RxFull) == 0) {
        return false;
    }
    *ch = uart->TxRx;
    return true;
}

cyg_uint8
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

cyg_bool
cyg_hal_plf_serial_getc_timeout(void* __ch_data, cyg_uint8* ch)
{
    int delay_count;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_bool res;

    delay_count = chan->msec_timeout * 10; // delay in .1 ms steps

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
    channel_data_t* chan = (channel_data_t*)__ch_data;
    volatile struct _gps4020_uart *uart = ((channel_data_t*)__ch_data)->base;
    int ret = 0;

    switch (__func) {
    case __COMMCTL_IRQ_ENABLE:
        chan->int_state = 1;
        uart->control |= SCR_RIE;
        HAL_INTERRUPT_UNMASK(chan->isr_vector);
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = chan->int_state;
        chan->int_state = 0;
        uart->control &= ~SCR_RIE;
        HAL_INTERRUPT_MASK(chan->isr_vector);
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
    default:
        break;
    }
    return ret;
}

static int
cyg_hal_plf_serial_isr(void *__ch_data, int* __ctrlc, 
                       CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
    int res = 0;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    volatile struct _gps4020_uart *uart = ((channel_data_t*)__ch_data)->base;
    char c;

    cyg_drv_interrupt_acknowledge(chan->isr_vector);
    *__ctrlc = 0;
    if ((uart->status & SSR_RxFull) != 0) {
        c = uart->TxRx;
        if (cyg_hal_is_break(&c, 1))
            *__ctrlc = 1;
        res = CYG_ISR_HANDLED;
    }
    return res;
}

static void
cyg_hal_plf_serial_init(void)
{
    hal_virtual_comm_table_t* comm;
    int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

    // Init channel
    cyg_hal_plf_serial_init_channel(&serial_channels[0]);

    // Setup procs in the vector table

    // Set channel 0
    CYGACC_CALL_IF_SET_CONSOLE_COMM(0);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &serial_channels[0]);
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
