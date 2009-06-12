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
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_edb7xxx.h>         // Hardware definitions

//-----------------------------------------------------------------------------

struct edb_serial {
    volatile cyg_uint32 ctrl;
    cyg_uint32 pad004_040[16-1];
    volatile cyg_uint32 stat;
    cyg_uint32 pad044_37c[208-1];
    union {
        volatile cyg_uint8 write;
        volatile cyg_uint32 read;    // Need to read 32 bits
    } data;
    cyg_uint32 pad384_3BC[16-1];
    volatile cyg_uint32 blcr;
};

//-----------------------------------------------------------------------------
typedef struct {
    volatile struct edb_serial* base;
    cyg_int32 msec_timeout;
    int isr_vector;
    int baud_rate;	
} channel_data_t;

//-----------------------------------------------------------------------------

static void
cyg_hal_plf_serial_init_channel(channel_data_t* __ch_data)
{
    channel_data_t* chan = (channel_data_t*)__ch_data;

    // Enable port
    chan->base->ctrl |= SYSCON1_UART1EN;
    // Configure
    chan->base->blcr = UART_BITRATE(chan->baud_rate) |
                       UBLCR_FIFOEN | UBLCR_WRDLEN8;
}


// Call this delay function when polling for serial access - otherwise
// the CPU will keep the memory bus busy and thus prevent DRAM refresh
// (and the resulting memory corruption).
externC void dram_delay_loop(void);

void
cyg_hal_plf_serial_putc(void *__ch_data, char c)
{
    channel_data_t* chan = (channel_data_t*)__ch_data;

    CYGARC_HAL_SAVE_GP();

    // Wait for Tx FIFO not full
    while ((chan->base->stat & SYSFLG1_UTXFF1) != 0) ;

    chan->base->data.write = c;

    CYGARC_HAL_RESTORE_GP();
}

static cyg_bool
cyg_hal_plf_serial_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
    channel_data_t* chan = (channel_data_t*)__ch_data;

    if ((chan->base->stat & SYSFLG1_URXFE1) != 0)
        return false;

    *ch = (cyg_uint8)(chan->base->data.read & 0xFF);

    return true;
}

cyg_uint8
cyg_hal_plf_serial_getc(void* __ch_data)
{
    cyg_uint8 ch;
    int delay_timer = 0;

    CYGARC_HAL_SAVE_GP();

    while(!cyg_hal_plf_serial_getc_nonblock(__ch_data, &ch)) {
        CYGACC_CALL_IF_DELAY_US(50);  // A reasonable time
        // Only delay every 10ms or so
        if (++delay_timer == 10*20) {
            dram_delay_loop();
            delay_timer = 0;
        }
    }

    CYGARC_HAL_RESTORE_GP();
    return ch;
}

static channel_data_t edb_ser_channels[2] = {
    {(volatile struct edb_serial*)SYSCON1, 1000, CYGNUM_HAL_INTERRUPT_URXINT1, CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD},
    {(volatile struct edb_serial*)SYSCON2, 1000, CYGNUM_HAL_INTERRUPT_URXINT2, CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD}
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
    int ret = 0;
    va_list ap;

    CYGARC_HAL_SAVE_GP();
    va_start(ap, __func);

    switch (__func) {
    case __COMMCTL_GETBAUD:
        ret = chan->baud_rate;
        break;
    case __COMMCTL_SETBAUD:
        chan->baud_rate = va_arg(ap, cyg_int32);
        // Should we verify this value here?
        cyg_hal_plf_serial_init_channel(chan);
        ret = 0;
        break;

    case __COMMCTL_IRQ_ENABLE:
        irq_state = 1;
        HAL_INTERRUPT_UNMASK(chan->isr_vector);
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
        irq_state = 0;
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

    va_end(ap);
    CYGARC_HAL_RESTORE_GP();
    return ret;
}

static int
cyg_hal_plf_serial_isr(void *__ch_data, int* __ctrlc, 
                       CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
    int res = 0;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    char c;
    CYGARC_HAL_SAVE_GP();

    cyg_drv_interrupt_acknowledge(chan->isr_vector);

    *__ctrlc = 0;
    if ((chan->base->stat & SYSFLG1_URXFE1) == 0) {
        c = (cyg_uint8)(chan->base->data.read & 0xFF);
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

    // Disable interrupts.
    HAL_INTERRUPT_MASK(edb_ser_channels[0].isr_vector);
    HAL_INTERRUPT_MASK(edb_ser_channels[1].isr_vector);

    // Init channels
    cyg_hal_plf_serial_init_channel(&edb_ser_channels[0]);
    cyg_hal_plf_serial_init_channel(&edb_ser_channels[1]);

    // Setup procs in the vector table

    // Set channel 0
    CYGACC_CALL_IF_SET_CONSOLE_COMM(0);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &edb_ser_channels[0]);
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
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &edb_ser_channels[1]);
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

// Assumption: all diagnostic output must be GDB packetized unless this is a ROM (i.e.
// totally stand-alone) system.

#ifdef CYGSEM_HAL_ROM_MONITOR
#define CYG_HAL_STARTUP_ROM
#undef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
#endif

#if defined(CYG_HAL_STARTUP_ROM) && !defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)
#define HAL_DIAG_USES_HARDWARE
#else
#if !defined(CYGDBG_HAL_DIAG_TO_DEBUG_CHAN)
#define HAL_DIAG_USES_HARDWARE
#elif CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL != CYGNUM_HAL_VIRTUAL_VECTOR_DEBUG_CHANNEL
#define HAL_DIAG_USES_HARDWARE
#endif
#endif

#if CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL == 0
# define __BASE ((volatile struct edb_serial*)SYSCON1)
# define __IRQ  CYGNUM_HAL_INTERRUPT_URXINT1
#else
# define __BASE ((volatile struct edb_serial*)SYSCON2)
# define __IRQ  CYGNUM_HAL_INTERRUPT_URXINT2
#endif

static channel_data_t edb_ser_channel = {
    __BASE, 0, 0
};

/*---------------------------------------------------------------------------*/
// EDB7XXX Serial Port (UARTx) for Debug

// Actually send character down the wire
static void
hal_diag_write_char_serial(char c)
{
    cyg_hal_plf_serial_putc(&edb_ser_channel, c);
}

static bool
hal_diag_read_serial(char *c)
{
    long timeout = 1000000000;  // A long time...

    while (! cyg_hal_plf_serial_getc_nonblock(&edb_ser_channel, c) )
        if (--timeout == 0) return false;

    return true;
}

#ifdef HAL_DIAG_USES_HARDWARE

void hal_diag_init(void)
{
    static int init = 0;
#ifndef CYG_HAL_STARTUP_ROM
    char *msg = "\n\rRAM EDB7XXX eCos\n\r";
#endif
    if (init++) return;

    cyg_hal_plf_serial_init_channel(&edb_ser_channel);

#ifndef CYG_HAL_STARTUP_ROM
    while (*msg) hal_diag_write_char(*msg++);
#endif
}

#ifdef DEBUG_DIAG
#ifndef CYG_HAL_STARTUP_ROM
#define DIAG_BUFSIZE 2048
static char diag_buffer[DIAG_BUFSIZE];
static int diag_bp = 0;
#endif
#endif

void hal_diag_write_char(char c)
{
    hal_diag_init();
#ifdef DEBUG_DIAG
#ifndef CYG_HAL_STARTUP_ROM
    diag_buffer[diag_bp++] = c;
    if (diag_bp == sizeof(diag_buffer)) diag_bp = 0;
#endif
#endif
    hal_diag_write_char_serial(c);
}

void hal_diag_read_char(char *c)
{
    while (!hal_diag_read_serial(c)) ;
}

#else // HAL_DIAG relies on GDB

// Initialize diag port
void hal_diag_init(void)
{
    // Assume port is already setup
    if (0) cyg_hal_plf_serial_init_channel(&edb_ser_channel);
}

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

            // Wait for the ACK character '+' from GDB here and handle
            // receiving a ^C instead.  This is the reason for this clause
            // being a loop.
            if (!hal_diag_read_serial(&c1))
                continue;   // No response - try sending packet again

            if( c1 == '+' )
                break;              // a good acknowledge

#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
            cyg_drv_interrupt_acknowledge(__IRQ);
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
#endif

#endif // !CYGSEM_HAL_VIRTUAL_VECTOR_DIAG

/*---------------------------------------------------------------------------*/
/* End of hal_diag.c */
