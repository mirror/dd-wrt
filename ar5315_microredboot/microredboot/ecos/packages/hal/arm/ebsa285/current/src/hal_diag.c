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
// Date:        1998-03-02
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
#include <cyg/hal/hal_ebsa285.h>        // Hardware definitions
#include <cyg/hal/drv_api.h>            // cyg_drv_interrupt_acknowledge

/*---------------------------------------------------------------------------*/

struct ebsa_serial {
  volatile cyg_uint32 data_register;
  volatile cyg_uint32 rxstat;
  volatile cyg_uint32 h_baud_control;
  volatile cyg_uint32 m_baud_control;
  volatile cyg_uint32 l_baud_control;
  volatile cyg_uint32 control_register;
  volatile cyg_uint32 flag_register;
};

/*---------------------------------------------------------------------------*/

static void
init_channel(void* __ch_data)
{
    volatile struct ebsa_serial* base = (struct ebsa_serial*)__ch_data;

    int dummy;
    /*
     * Make sure everything is off
     */
    base->control_register = SA110_UART_DISABLED | SA110_SIR_DISABLED;
    
    /*
     * Read the RXStat to drain the fifo
     */
    dummy = base->rxstat;

    /*
     * Set the baud rate - this also turns the uart on.
     *
     * Note that the ordering of these writes is critical,
     * and the writes to the H_BAUD_CONTROL and CONTROL_REGISTER
     * are necessary to force the UART to update its register
     * contents.
     */
    base->l_baud_control   = 0x13; // bp->divisor_low;
    base->m_baud_control   = 0x00; // bp->divisor_high;
    base->h_baud_control = SA110_UART_BREAK_DISABLED    |
        SA110_UART_PARITY_DISABLED   |
        SA110_UART_STOP_BITS_ONE     |
        SA110_UART_FIFO_ENABLED      |
        SA110_UART_DATA_LENGTH_8_BITS;
    base->control_register = SA110_UART_ENABLED | SA110_SIR_DISABLED;
    // All done
}

void
cyg_hal_plf_serial_putc(void *__ch_data, char c)
{
    volatile struct ebsa_serial* base = (struct ebsa_serial*)__ch_data;
    CYGARC_HAL_SAVE_GP();

    // Wait for Tx FIFO not full
    while ((base->flag_register & SA110_TX_FIFO_STATUS_MASK) == SA110_TX_FIFO_BUSY)
        ;
    base->data_register = c;

    CYGARC_HAL_RESTORE_GP();
}


static cyg_bool
cyg_hal_plf_serial_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
    volatile struct ebsa_serial* base = (struct ebsa_serial*)__ch_data;


    if ((base->flag_register & SA110_RX_FIFO_STATUS_MASK) == SA110_RX_FIFO_EMPTY)
        return false;

    *ch = (char)(base->data_register & 0xFF);

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

static cyg_int32 msec_timeout;

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
    cyg_bool res;
    CYGARC_HAL_SAVE_GP();

    delay_count = msec_timeout * 10; // delay in .1 ms steps

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
        irq_state = 1;

        HAL_INTERRUPT_UNMASK(CYGNUM_HAL_INTERRUPT_SERIAL_RX);
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
        irq_state = 0;

        HAL_INTERRUPT_MASK(CYGNUM_HAL_INTERRUPT_SERIAL_RX);
        break;
    case __COMMCTL_DBG_ISR_VECTOR:
        ret = CYGNUM_HAL_INTERRUPT_SERIAL_RX;
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
    int reg, res = 0;
    volatile struct ebsa_serial* base = (struct ebsa_serial*)__ch_data;
    char c;
    CYGARC_HAL_SAVE_GP();

    if ( CYGNUM_HAL_INTERRUPT_SERIAL_RX == __vector ) {
        reg = base->flag_register;
        // read it anyway just in case - no harm done and we might
        // prevent an interrup loop
        c = (char)(base->data_register & 0xFF);

        cyg_drv_interrupt_acknowledge(CYGNUM_HAL_INTERRUPT_SERIAL_RX);
        *__ctrlc = 0;
        if ( (reg & SA110_RX_FIFO_STATUS_MASK) != SA110_RX_FIFO_EMPTY ) {
            if( cyg_hal_is_break( &c , 1 ) )
                *__ctrlc = 1;
            
        }
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
    init_channel((void*)UART_BASE_0);

    // Setup procs in the vector table

    // Set channel 0
    CYGACC_CALL_IF_SET_CONSOLE_COMM(0);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, UART_BASE_0);
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


//=============================================================================
// Compatibility with older stubs
//=============================================================================

#ifndef CYGSEM_HAL_VIRTUAL_VECTOR_DIAG

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
#include <cyg/hal/hal_stub.h>           // cyg_hal_gdb_interrupt
#endif

#ifdef CYGSEM_HAL_ROM_MONITOR
#define CYG_HAL_STARTUP_ROM
#undef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
#endif

#if defined(CYG_HAL_STARTUP_ROM) || !defined(CYGDBG_HAL_DIAG_TO_DEBUG_CHAN)
#define HAL_DIAG_USES_HARDWARE
#endif

/*---------------------------------------------------------------------------*/
// EBSA285 Serial Port (UARTx) for Debug

void hal_diag_init(void)
{
  init_channel((void*)UART_BASE_0);
}


// Actually send character down the wire
static void
hal_diag_write_char_serial(char c)
{
    cyg_hal_plf_serial_putc((void*)UART_BASE_0, c);
}

static bool
hal_diag_read_serial(char *c)
{
    long timeout = 1000000000;  // A long time...
    while (! cyg_hal_plf_serial_getc_nonblock((void*)UART_BASE_0, c) )
        if ( --timeout == 0 ) return false;

    return true;
}

/*
 * Baud rate selection stuff
 */
#if 0
struct _baud {
    int baud;
    unsigned short divisor_high, divisor_low;
};

const static struct _baud bauds[] = {
#if (FCLK_MHZ == 50)
    {   300, 0xA, 0x2B},                  /* 2603  = 0x0A2B */
    {   600, 0x5, 0x15},                  /* 1301  = 0x0515 */
    {  1200, 0x2, 0x8A},                  /* 650   = 0x028A */
    {  2400, 0x1, 0x45},                  /* 325   = 0x0145 */
    {  4800, 0x0, 0xA2},                  /* 162   = 0x00A2 */
    {  9600, 0x0, 0x50},                  /* 80    = 0x0050 */
    { 19200, 0x0, 0x28},                  /* 40    = 0x0028 */
    { 38400, 0x0, 0x13},                  /* 19    = 0x0013 */
#elif (FCLK_MHZ == 60)
    {   300, 0xC, 0x34},                  /* 2603  = 0x0A2B */
    {   600, 0x6, 0x19},                  /* 1301  = 0x0515 */
    {  1200, 0x3, 0x0C},                  /* 650   = 0x028A */
    {  2400, 0x1, 0x86},                  /* 325   = 0x0145 */
    {  4800, 0x0, 0xC2},                  /* 162   = 0x00A2 */
    {  9600, 0x0, 0x61},                  /* 80    = 0x0050 */
    { 19200, 0x0, 0x30},                  /* 40    = 0x0028 */
    { 38400, 0x0, 0x17},                  /* 19    = 0x0013 */
#endif
};
#endif

#ifdef HAL_DIAG_USES_HARDWARE

#ifdef DEBUG_DIAG
#ifndef CYG_HAL_STARTUP_ROM
#define DIAG_BUFSIZE 2048
static char diag_buffer[DIAG_BUFSIZE];
static int diag_bp = 0;
#endif
#endif

void hal_diag_read_char(char *c)
{
    while (!hal_diag_read_serial(c)) ;
}

void hal_diag_write_char(char c)
{
#ifdef DEBUG_DIAG
#ifndef CYG_HAL_STARTUP_ROM
    diag_buffer[diag_bp++] = c;
    if (diag_bp == sizeof(diag_buffer)) diag_bp = 0;
#endif
#endif
    hal_diag_write_char_serial(c);
}

#else // not HAL_DIAG_USES_HARDWARE - it uses GDB protocol

void 
hal_diag_read_char(char *c)
{
    while (!hal_diag_read_serial(c)) ;
}

void 
hal_diag_write_char(char c)
{
    static char line[100];
    static int pos = 0;

#if 0
    // Do not unconditionally poke the XBUS LED location - XBUS may not be
    // available if external arbiter is in use.  This fragment may still be
    // useful for debugging in the future, so left thus:
    {
//        int i;
        *(cyg_uint32 *)0x40012000 = 7 & (cyg_uint32)c; // LED XBUS location
//        for ( i = 0x1000000; i > 0; i-- ) ;
    }
#endif

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
#ifndef CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT
            char c1;
#endif        
            hal_diag_write_char_serial('$');
            hal_diag_write_char_serial('O');
            csum += 'O';
            for( i = 0; i < pos; i++ )
            {
                char ch = line[i];
                char h = hex[(ch>>4)&0xF];
                char l = hex[ch&0xF];
                hal_diag_write_char_serial(h);
                hal_diag_write_char_serial(l);
                csum += h;
                csum += l;
            }
            hal_diag_write_char_serial('#');
            hal_diag_write_char_serial(hex[(csum>>4)&0xF]);
            hal_diag_write_char_serial(hex[csum&0xF]);

#ifdef CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT

            break; // regardless

#else // not CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT Ie. usually...

            // Wait for the ACK character '+' from GDB here and handle
            // receiving a ^C instead.  This is the reason for this clause
            // being a loop.
            if (!hal_diag_read_serial(&c1))
                continue;   // No response - try sending packet again

            if( c1 == '+' )
                break;              // a good acknowledge

#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
            cyg_drv_interrupt_acknowledge(CYGNUM_HAL_INTERRUPT_SERIAL_RX);
            if( c1 == 3 ) {
                // Ctrl-C: breakpoint.
                cyg_hal_gdb_interrupt(
                    (target_register_t)__builtin_return_address(0) );
                break;
            }
#endif // CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT

#endif // ! CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT
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
#endif

#endif // !CYGSEM_HAL_VIRTUAL_VECTOR_DIAG

/*---------------------------------------------------------------------------*/
/* End of hal_diag.c */
