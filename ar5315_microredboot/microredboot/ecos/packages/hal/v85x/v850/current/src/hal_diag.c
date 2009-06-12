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
// Contributors:nickg, gthomas, jlarmour
// Date:        2001-03-21
// Purpose:     HAL diagnostic output
// Description: Implementations of HAL diagnostic output support.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/hal.h>

#if CYGINT_HAL_V850_DIAG_ONCHIP_SERIAL0

#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_arch.h>           // basic machine info
#include <cyg/hal/hal_intr.h>           // interrupt macros
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_diag.h>

#include <cyg/hal/hal_stub.h>           // target_register_t
#include <cyg/hal/hal_if.h>             // Calling interface definitions
#include <cyg/hal/hal_misc.h>           // Helper functions
#include <cyg/hal/drv_api.h>            // CYG_ISR_HANDLED

#include <cyg/hal/v850_common.h>        // hardware registers, etc.


#define BAUD_COUNT ((CYGHWR_HAL_V85X_CPU_FREQ/2)/CYGHWR_HAL_V85X_V850_DIAG_BAUD)
#define BAUD_DIVISOR 1

/*---------------------------------------------------------------------------*/
// V850

void
init_serial_channel(void* base)
{
    volatile unsigned char *mode = (volatile unsigned char *)V850_REG_ASIM0;
    volatile unsigned char *brgc = (volatile unsigned char *)V850_REG_BRGC0;
    volatile unsigned char *brgm0 = (volatile unsigned char *)V850_REG_BRGMC00;
#ifdef V850_REG_BRGMC01
    volatile unsigned char *brgm1 = (volatile unsigned char *)V850_REG_BRGMC01;
#endif
    volatile unsigned char *rxstat = (volatile unsigned char *)V850_REG_SRIC0;
    volatile unsigned char *rxerr = (volatile unsigned char *)V850_REG_SERIC0;
    volatile unsigned char *txstat = (volatile unsigned char *)V850_REG_STIC0;
    int count = BAUD_COUNT;
    int divisor = BAUD_DIVISOR;

    while (count > 0xFF) {
        count >>= 1;
        divisor++;
    }

    *mode = 0xC8;
    *brgc = count;
    *brgm0 = divisor & 0x07;
#ifdef V850_REG_BRGMC01
    *brgm1 = divisor >> 3;
#endif
    *rxstat = 0x47;
    *rxerr = 0;
    *txstat = 0x47;
}

// Actually send character down the wire
void
cyg_hal_plf_serial_putc(void* __ch_data, cyg_uint8 c)
{
    volatile unsigned char *TxDATA = (volatile unsigned char *)V850_REG_TXS0;
    volatile unsigned char *TxSTAT = (volatile unsigned char *)V850_REG_STIC0;
    int timeout = 0xFFFF;

    CYGARC_HAL_SAVE_GP();

    // Send character
    *TxDATA = (unsigned char)c;
    // Wait for Tx not busy
    while ((*TxSTAT & 0x80) == 0x00) {
        if (--timeout == 0)
            break;
    }
    *TxSTAT &= ~0x80;

    CYGARC_HAL_RESTORE_GP();
}

static cyg_bool
cyg_hal_plf_serial_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
    volatile unsigned char *RxDATA = (volatile unsigned char *)V850_REG_RXB0;
    volatile unsigned char *RxSTAT = (volatile unsigned char *)V850_REG_SRIC0;

    if ((*RxSTAT & 0x80) == 0x00)
        return false;

    *ch = (char)*RxDATA;
    *RxSTAT &= ~0x80;
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

static cyg_int32 msec_timeout = 1000;

cyg_bool
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
    CYGARC_HAL_SAVE_GP();

    switch (__func) {
    case __COMMCTL_IRQ_ENABLE:
        HAL_INTERRUPT_UNMASK(CYGNUM_HAL_VECTOR_INTCSI1);
        irq_state = 1;
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
        irq_state = 0;
        HAL_INTERRUPT_MASK(CYGNUM_HAL_VECTOR_INTCSI1);
        break;
    case __COMMCTL_DBG_ISR_VECTOR:
        ret = CYGNUM_HAL_VECTOR_INTCSI1;
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

static int
cyg_hal_plf_serial_isr(void *__ch_data, int* __ctrlc, 
                       CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
    volatile unsigned char *RxDATA = (volatile unsigned char *)V850_REG_RXB0;
    cyg_uint8 c;
    int res = 0;
    CYGARC_HAL_SAVE_GP();

    c = (char)*RxDATA;
    *__ctrlc = 0;
    if( cyg_hal_is_break( &c , 1 ) )
        *__ctrlc = 1;

    cyg_drv_interrupt_acknowledge(CYGNUM_HAL_VECTOR_INTCSI1);

    res = CYG_ISR_HANDLED;

    CYGARC_HAL_RESTORE_GP();
    return res;
}

static void
cyg_hal_plf_serial_init(void)
{
    hal_virtual_comm_table_t* comm;
    int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

    // Disable interrupts.
    HAL_INTERRUPT_MASK(CYGNUM_HAL_VECTOR_INTCSI1);

    // Init channels
    init_serial_channel((cyg_uint8*)0);

    // Setup procs in the vector table

    // Set channel 0
    CYGACC_CALL_IF_SET_CONSOLE_COMM(0);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, 0);
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

__externC void cyg_hal_plf_ice_diag_init();

void
cyg_hal_plf_comms_init(void)
{
    static int initialized = 0;

    if (initialized)
        return;

    initialized = 1;

    cyg_hal_plf_serial_init();

#ifdef CYGDBG_HAL_V85X_V850_ICE_DIAG
    cyg_hal_plf_ice_diag_init();
#endif
}

//=============================================================================
// Compatibility with older stubs
//=============================================================================

#ifndef CYGSEM_HAL_VIRTUAL_VECTOR_DIAG

// Assumption: all diagnostic output must be GDB packetized unless this is a ROM (i.e.
// totally stand-alone) system.

//#ifdef CYGSEM_HAL_ROM_MONITOR
//#define CYG_HAL_STARTUP_ROM
//#undef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
//#endif

#if (defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)) \
    && !defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)
#define HAL_DIAG_USES_HARDWARE
#else
#if !defined(CYGDBG_HAL_DIAG_TO_DEBUG_CHAN)
#define HAL_DIAG_USES_HARDWARE
#endif
#endif

void hal_diag_init(void)
{
    init_serial_channel(0);
}

#ifdef HAL_DIAG_USES_HARDWARE

void hal_diag_write_char(char c)
{
    CYG_INTERRUPT_STATE old;
    HAL_DISABLE_INTERRUPTS(old);
    cyg_hal_plf_serial_putc(0, c);
    HAL_RESTORE_INTERRUPTS(old);
}

void hal_diag_read_char(char *c)
{
    *c = cyg_hal_plf_serial_getc(0);
}

#else // HAL_DIAG relies on GDB

void 
hal_diag_read_char(char *c)
{
    *c = cyg_hal_plf_serial_getc(0);
}

void 
hal_diag_write_char(char c)
{
    static char line[100];
    static int pos = 0;

    // No need to send CRs
    if( c == '\r' ) return;

    line[pos++] = c;

    if( c == '\n' || pos == sizeof(line) )
    {
        CYG_INTERRUPT_STATE old;

        // Disable interrupts. This prevents GDB trying to interrupt us
        // while we are in the middle of sending a packet. The serial
        // receive interrupt will be seen when we re-enable interrupts
        // later.
        
#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        CYG_HAL_GDB_ENTER_CRITICAL_IO_REGION(old);
#else
        HAL_DISABLE_INTERRUPTS(old);
#endif

        while(1)
        {
            static char hex[] = "0123456789ABCDEF";
            cyg_uint8 csum = 0;
            int i;
            char c1;
        
            cyg_hal_plf_serial_putc(0, '$');
            cyg_hal_plf_serial_putc(0, 'O');
            csum += 'O';
            for( i = 0; i < pos; i++ )
            {
                char ch = line[i];
                char h = hex[(ch>>4)&0xF];
                char l = hex[ch&0xF];
                cyg_hal_plf_serial_putc(0, h);
                cyg_hal_plf_serial_putc(0, l);
                csum += h;
                csum += l;
            }
            cyg_hal_plf_serial_putc(0, '#');
            cyg_hal_plf_serial_putc(0, hex[(csum>>4)&0xF]);
            cyg_hal_plf_serial_putc(0, hex[csum&0xF]);

            // Wait for the ACK character '+' from GDB here and handle
            // receiving a ^C instead.  This is the reason for this clause
            // being a loop.
            c1 = cyg_hal_plf_serial_getc(0);

            if( c1 == '+' )
                break;              // a good acknowledge

#if defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS) && \
    defined(CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT)
            cyg_drv_interrupt_acknowledge(CYGNUM_HAL_VECTOR_INTCSI1);
            if( c1 == 3 ) {
                // Ctrl-C: breakpoint.
                cyg_hal_gdb_interrupt (__builtin_return_address(0));
                break;
            }
#endif
            // otherwise, loop round again
        }
        
        pos = 0;

        // And re-enable interrupts
#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        CYG_HAL_GDB_LEAVE_CRITICAL_IO_REGION(old);
#else
        HAL_RESTORE_INTERRUPTS(old);
#endif
    }
}
#endif  // USE HARDWARE

#endif // CYGSEM_HAL_VIRTUAL_VECTOR_DIAG

#endif // #if CYGINT_HAL_V850_DIAG_ONCHIP_SERIAL0

/*---------------------------------------------------------------------------*/
/* End of hal_diag.c */
