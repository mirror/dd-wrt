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
// Author(s):   nickg, gthomas
// Contributors:nickg, gthomas
// Date:        2001-04-23
// Purpose:     HAL diagnostic output
// Description: Implementations of HAL diagnostic output support.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/hal.h>
#include CYGBLD_HAL_VARIANT_H           // Variant specific configuration
#include CYGBLD_HAL_PLATFORM_H          // Platform specific configuration

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_arch.h>           // basic machine info
#include <cyg/hal/hal_intr.h>           // interrupt macros
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_if.h>             // interface API
#include <cyg/hal/hal_misc.h>           // Helper functions
#include <cyg/hal/aaed2000.h>           // platform definitions

// The controller is clocked at 7.3728MHz
#define BAUD_RATE(_n_) ((7372800/((_n_)*16))-1)
 
//-----------------------------------------------------------------------------
typedef struct {
    cyg_uint8 *base;
    cyg_int32  msec_timeout;
    int        isr_vector;
} channel_data_t;

static channel_data_t aaed2000_ser_channels[1] = {
    { (cyg_uint8 *)AAEC_UART3, 1000, CYGNUM_HAL_INTERRUPT_UART3INTR },
};

//-----------------------------------------------------------------------------

static void
cyg_hal_plf_serial_init_channel(void* __ch_data)
{
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_uint8* base = chan->base;

    // Enable first.
    // Register writes don't take effect till the UART is enabled.
    HAL_WRITE_UINT32(base+AAEC_UART_CTRL, AAEC_UART_CTRL_ENAB);

    HAL_WRITE_UINT32(base+AAEC_UART_STATUS, 0);
    HAL_WRITE_UINT32(base+AAEC_UART_INTM, 0);
    HAL_WRITE_UINT32(base+AAEC_UART_BAUD,
                     BAUD_RATE(CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD));
    // 8-1-no parity.
    HAL_WRITE_UINT32(base+AAEC_UART_LCR, AAEC_UART_LCR_FIFO | AAEC_UART_LCR_WL8);
}

void
cyg_hal_plf_serial_putc(void *__ch_data, char c)
{
    cyg_uint8* base = ((channel_data_t*)__ch_data)->base;
    cyg_uint32 status;

    do {
        HAL_READ_UINT32(base+AAEC_UART_STATUS, status);
    } while ((status & AAEC_UART_STATUS_TxFF) != 0);

    HAL_WRITE_UINT32(base+AAEC_UART_DATA, c);

    do {
        HAL_READ_UINT32(base+AAEC_UART_STATUS, status);
    } while (status & AAEC_UART_STATUS_TxBSY);
}

static cyg_bool
cyg_hal_plf_serial_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
    cyg_uint8* base = ((channel_data_t*)__ch_data)->base;
    cyg_uint32 status;
    cyg_uint32 c;

    HAL_READ_UINT32(base+AAEC_UART_STATUS, status);
    if ((status & AAEC_UART_STATUS_RxFE) != 0)
        return false;

    HAL_READ_UINT32(base+AAEC_UART_DATA, c);
    *ch = c;

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
    static int irq_state = 0;
    cyg_uint32 intm;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_uint8* base = ((channel_data_t*)__ch_data)->base;
    int ret = 0;

    CYGARC_HAL_SAVE_GP();

    switch (__func) {
    case __COMMCTL_IRQ_ENABLE:
        irq_state = 1;
        HAL_READ_UINT32(base+AAEC_UART_INTM, intm);
        intm |= AAEC_UART_INT_RIS|AAEC_UART_INT_RTIS;
        HAL_WRITE_UINT32(base+AAEC_UART_INTM, intm);
        HAL_INTERRUPT_UNMASK(chan->isr_vector);
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
        irq_state = 0;
        HAL_READ_UINT32(base+AAEC_UART_INTM, intm);
        intm &= ~(AAEC_UART_INT_RIS|AAEC_UART_INT_RTIS);
        HAL_WRITE_UINT32(base+AAEC_UART_INTM, intm);
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
    CYGARC_HAL_RESTORE_GP();
    return ret;
}

static int
cyg_hal_plf_serial_isr(void *__ch_data, int* __ctrlc, 
                       CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
    int res = 0;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_uint8* base = ((channel_data_t*)__ch_data)->base;
    char c;
    cyg_uint32 lsr, _c;
    CYGARC_HAL_SAVE_GP();

    cyg_drv_interrupt_acknowledge(chan->isr_vector);

    *__ctrlc = 0;
    HAL_READ_UINT32(base+AAEC_UART_STATUS, lsr);
    if ( (lsr & AAEC_UART_STATUS_RxFE) != 0 ) {
        HAL_READ_UINT32(base+AAEC_UART_DATA, _c);
        c = (char)_c;
        if( cyg_hal_is_break( &c , 1 ) )
            *__ctrlc = 1;

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

    // Init channels
    cyg_hal_plf_serial_init_channel(&aaed2000_ser_channels[0]);

    // Setup procs in the vector table

    // Set channel 0
    CYGACC_CALL_IF_SET_CONSOLE_COMM(0);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &aaed2000_ser_channels[0]);
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

/*---------------------------------------------------------------------------*/
/* End of hal_diag.c */
