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
// Author(s):   hmt, nickg
// Contributors:        nickg
// Date:        2001-05-25
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

#include <cyg/hal/hal_arch.h>           // which includes var_arch => UART.
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_if.h>             // interface API
#include <cyg/hal/hal_misc.h>           // Helper functions

/*---------------------------------------------------------------------------*/
//#define CYG_KERNEL_DIAG_GDB

#if defined(CYGSEM_HAL_USE_ROM_MONITOR_GDB_stubs)

#define CYG_KERNEL_DIAG_GDB

#endif

/*---------------------------------------------------------------------------*/
static inline
void hal_uart_setbaud( int baud )
{
    // now set the baud rate
    *UARTLCR |= UARTLCR_DLAB;
    *UARTDLM = UARTDLM_VAL( baud );
    *UARTDLL = UARTDLL_VAL( baud );
    *UARTLCR &=~UARTLCR_DLAB;
}

void hal_uart_init(void)
{
    // Ensure that we use the internal clock
    *S_GMR &=~S_GMR_UCSEL;

    *UARTFCR = UARTFCR_16550_MODE;
    *UARTLCR = UARTLCR_8N1;
    *UARTIER = UARTIER_ERBFI;           // rx interrupts enabled for CTRL-C

    hal_uart_setbaud( CYGHWR_HAL_MIPS_UPD985XX_DIAG_BAUD );
}

void hal_uart_write_char(char c)
{
    while ( 0 == (UARTLSR_THRE & *UARTLSR) )
        /* do nothing */ ;

    *UARTTHR = (unsigned int)c;

    // Ensure that this write does not provoke a spurious interrupt.
    HAL_INTERRUPT_ACKNOWLEDGE( CYGHWR_HAL_GDB_PORT_VECTOR );
}

void hal_uart_read_char(char *c)
{
    while ( 0 == (UARTLSR_DR & *UARTLSR) )
        /* do nothing */ ;

    *c = (char)*UARTRBR;

    // Ensure that this read does not provoke a spurious interrupt.
    HAL_INTERRUPT_ACKNOWLEDGE( CYGHWR_HAL_GDB_PORT_VECTOR );
}

int hal_uart_read_char_nonblock(char *c)
{
    if ( 0 == (UARTLSR_DR & *UARTLSR) )
        return 0;

    *c = (char)*UARTRBR;

    // Ensure that this read does not provoke a spurious interrupt.
    HAL_INTERRUPT_ACKNOWLEDGE( CYGHWR_HAL_GDB_PORT_VECTOR );

    return 1;
}


/*---------------------------------------------------------------------------*/

#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_DIAG

#include <cyg/hal/hal_if.h>

// This is lame, duplicating all these wrappers with slightly different details.
// All this should be in hal_if.c
static void
cyg_hal_plf_serial_putc(void* __ch_data, cyg_uint8 c)
{
    CYGARC_HAL_SAVE_GP();
    hal_uart_write_char( (char)c );
    CYGARC_HAL_RESTORE_GP();
}

static cyg_uint8
cyg_hal_plf_serial_getc(void* __ch_data)
{
    cyg_uint8 result;
    CYGARC_HAL_SAVE_GP();
    hal_uart_read_char( (char *)&result );
    CYGARC_HAL_RESTORE_GP();
    return result;
}

static int
cyg_hal_plf_serial_getc_nonblock(void* __ch_data, cyg_uint8 *pc)
{
    return hal_uart_read_char_nonblock( (char *)pc );
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

static int chan__msec_timeout = 0;
static int chan__irq_state = 0;
static int chan__baud_rate = CYGHWR_HAL_MIPS_UPD985XX_DIAG_BAUD;

cyg_bool
cyg_hal_plf_serial_getc_timeout(void* __ch_data, cyg_uint8* ch)
{
    int delay_count;
    cyg_bool res;
    CYGARC_HAL_SAVE_GP();

    delay_count = chan__msec_timeout * 10; // delay in .1 ms steps

    for(;;) {
        res = cyg_hal_plf_serial_getc_nonblock(__ch_data, ch);
        if (res || 0 >= delay_count--)
            break;
        
        CYGACC_CALL_IF_DELAY_US(100);
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}

static int
cyg_hal_plf_serial_control(void *__ch_data, __comm_control_cmd_t __func, ...)
{
    int ret = -1;
    va_list ap;

    CYGARC_HAL_SAVE_GP();
    va_start(ap, __func);

    switch (__func) {
    case __COMMCTL_GETBAUD:
        ret = chan__baud_rate;
        break;
    case __COMMCTL_SETBAUD:
        ret = 0;
        chan__baud_rate = va_arg(ap, cyg_int32);
        hal_uart_setbaud( chan__baud_rate );
        break;
    case __COMMCTL_IRQ_ENABLE:
        ret = chan__irq_state;
        chan__irq_state = 1;
        HAL_INTERRUPT_UNMASK( CYGHWR_HAL_GDB_PORT_VECTOR );
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = chan__irq_state;
        chan__irq_state = 0;
        HAL_INTERRUPT_MASK( CYGHWR_HAL_GDB_PORT_VECTOR );
        break;
    case __COMMCTL_DBG_ISR_VECTOR:
        ret = CYGHWR_HAL_GDB_PORT_VECTOR;
        break;
    case __COMMCTL_SET_TIMEOUT:
        ret = chan__msec_timeout;
        chan__msec_timeout = va_arg(ap, cyg_uint32);
        break;
    default:
        break;
    }
    va_end(ap);
    CYGARC_HAL_RESTORE_GP();
    return ret;
}

static int
cyg_hal_plf_serial_isr(void *__ch_data, int* __ctrlc, 
                       CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
    int ret = 0;
    char c;

    *__ctrlc = 0;

    if ( hal_uart_read_char_nonblock( &c ) ) {
        if ( cyg_hal_is_break( &c , 1 ) )
            *__ctrlc = 1;
        ret = CYG_ISR_HANDLED;
    }

    return ret;
}

static void
cyg_hal_plf_serial_init(void)
{
    hal_virtual_comm_table_t* comm;
    int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

    // Init channels
    CYGACC_CALL_IF_SET_CONSOLE_COMM(0);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, comm);
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

    hal_uart_init();
    cyg_hal_plf_serial_init();
}

#endif // CYGSEM_HAL_VIRTUAL_VECTOR_DIAG

// ------------------------------------------------------------------------

void hal_diag_init(void)
{
    hal_uart_init();
}

void hal_diag_read_char(char *c)
{
    hal_uart_read_char(c);
}

extern cyg_bool cyg_hal_is_break(char *buf, int size);
extern void cyg_hal_user_break(CYG_ADDRWORD *regs);

void hal_diag_write_char(char c)
{
#ifdef CYG_KERNEL_DIAG_GDB    
    static char line[100];
    static int pos = 0;

    // No need to send CRs
    if( c == '\r' ) return;

    line[pos++] = c;

    if( c == '\n' || pos == sizeof(line) )
    {

        // Disable interrupts. This prevents GDB trying to interrupt us
        // while we are in the middle of sending a packet. The serial
        // receive interrupt will be seen when we re-enable interrupts
        // later.
        CYG_INTERRUPT_STATE oldstate;
        HAL_DISABLE_INTERRUPTS(oldstate);
        
        while(1)
        {
            static char hex[] = "0123456789ABCDEF";
            cyg_uint8 csum = 0;
            int i;
            char c1;
        
            hal_uart_write_char('$');
            hal_uart_write_char('O');
            csum += 'O';
            for( i = 0; i < pos; i++ )
            {
                char ch = line[i];
                char h = hex[(ch>>4)&0xF];
                char l = hex[ch&0xF];
                hal_uart_write_char(h);
                hal_uart_write_char(l);
                csum += h;
                csum += l;
            }
            hal_uart_write_char('#');
            hal_uart_write_char(hex[(csum>>4)&0xF]);
            hal_uart_write_char(hex[csum&0xF]);

            hal_uart_read_char( &c1 );

            if( c1 == '+' ) break;

            if( cyg_hal_is_break( &c1 , 1 ) )
                cyg_hal_user_break( NULL );    

        }
        
        pos = 0;

        // Disabling the interrupts for an extended period of time
        // can provoke a spurious interrupt.

        // Ensure that this write does not provoke a spurious interrupt.
        HAL_INTERRUPT_ACKNOWLEDGE( CYGHWR_HAL_GDB_PORT_VECTOR );

        // And re-enable interrupts
        HAL_RESTORE_INTERRUPTS( oldstate );
        
    }
#else
    hal_uart_write_char(c);
#endif    
}

/*---------------------------------------------------------------------------*/
/* End of hal_diag.c */
