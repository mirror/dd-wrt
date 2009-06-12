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
// Author(s):   nickg
// Contributors:        nickg
// Date:        1998-03-02
// Purpose:     HAL diagnostic output
// Description: Implementations of HAL diagnostic output support.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <pkgconf/hal_sparclite.h>
#include <pkgconf/hal_sparclite_sleb.h>

#include <cyg/infra/cyg_type.h>          // base types

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>

#include <cyg/hal/hal_cygm.h>

/*---------------------------------------------------------------------------*/

#ifdef CYG_KERNEL_DIAG_GDB

#ifdef CYG_KERNEL_DIAG_GDB_SERIAL_DIRECT // then force $O packets to serial

void hal_diag_init(void)
{
    // hal_diag_init_serial();
}

void hal_diag_write_char_serial( char c )
{
    HAL_REORDER_BARRIER();
    HAL_DIAG_WRITE_CHAR_DIRECT( c );
    HAL_REORDER_BARRIER();
    HAL_DIAG_WRITE_CHAR_WAIT_FOR_EMPTY();
    HAL_REORDER_BARRIER();
}

void hal_diag_write_char(char c)
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
        
        HAL_DISABLE_INTERRUPTS(old);
        
        while(1)
        {
            cyg_uint32 status, c1, tries;
            static char hex[] = "0123456789ABCDEF";
            cyg_uint8 csum = 0;
            int i;
        
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
            status = 0;
            tries = 1000000000;
            while ( 0 == (HAL_SPARC_86940_FLAG_RXRDY & status) ) {
                if ( 0 == --tries )
                    break;
                HAL_SPARC_86940_SDTR0_STAT_READ( status );
            }
            if ( 0 == tries )       // then we broke out after waiting
                continue;           // the outer loop, send the packet
            
            HAL_SPARC_86940_SDTR0_RXDATA_READ( c1 );

            // We must ack the interrupt caused by that read to avoid
            // confusing the GDB stub ROM.
            HAL_INTERRUPT_ACKNOWLEDGE( CYGNUM_HAL_VECTOR_INTERRUPT_10 );
                
            if( c1 == '+' )
                break;              // a good acknowledge

            if( c1 == 3 ) {
                // Ctrl-C: breakpoint.
                asm volatile( "ta 2; nop; nop; nop" );
                break;
            }
            // otherwise, loop round again
        }
        
        pos = 0;

        // And re-enable interrupts
        HAL_RESTORE_INTERRUPTS(old);
        
    }
}
#else // CYG_KERNEL_DIAG_GDB_SERIAL_DIRECT not defined; use CygMon

// All this code provided by MSalter - ta.

struct bsp_comm_procs {
    void *ch_data;
    void (*__write)(void *ch_data, const char *buf, int len);
    int  (*__read)(void *ch_data, char *buf, int len);
    void (*__putc)(void *ch_data, char ch);
    int  (*__getc)(void *ch_data);
    int  (*__control)(void *ch_data, int func, ...);
};

// This is pointed to by entry BSP_NOTVEC_BSP_COMM_PROCS:
typedef struct {
    int  version;       /* version number for future expansion */
    void *__ictrl_table;
    void *__exc_table;
    void *__dbg_vector;
    void *__kill_vector;
    struct bsp_comm_procs *__console_procs;
    struct bsp_comm_procs *__debug_procs;
    void *__flush_dcache;
    void *__flush_icache;
    void *__cpu_data;
    void *__board_data;
    void *__sysinfo;
    int  (*__set_debug_comm)(int __comm_id);
    void *__set_console_comm;
} bsp_shared_t;


static int
hal_bsp_set_debug_comm(int arg)
{
    bsp_shared_t *shared;

    shared = (bsp_shared_t *)
        (CYGMON_VECTOR_TABLE[ BSP_NOTVEC_BSP_COMM_PROCS ]);

    if (0 != shared->__set_debug_comm) {
        return (*(shared->__set_debug_comm))(arg);
    }
    return 0;
}

static int
hal_bsp_console_write(const char *p, int len)
{
    bsp_shared_t *shared;
    struct bsp_comm_procs *com;

    shared = (bsp_shared_t *)
        (CYGMON_VECTOR_TABLE[ BSP_NOTVEC_BSP_COMM_PROCS ]);

    com = shared->__console_procs;

    if (0 != com) {
        com->__write(com->ch_data, p, len);

#if 1
        // FIXME: This is a workaround for PR 19926; CygMon does not
        // expect to be sharing the line with a serial driver (which
        // can be excused :) and so doesn't acknowledge the interrupt.
        // In normal circumstances CygMon would handle the resulting
        // interrupt and do the right thing.  However, when using the
        // serial driver it is handling the interrupts and gets
        // mightily confused by these spurious interrupts.
        //
        // As a workaround, ask CygMon which communication port is
        // using for console output. If this is the serial port 
        // (comm 0), acknowledge the interrupt.
        if ( 0 == hal_bsp_set_debug_comm( -1 ) )
            HAL_INTERRUPT_ACKNOWLEDGE( CYGNUM_HAL_VECTOR_INTERRUPT_10 );
#endif

        return 1;
    }
    return 0;
}

static void
hal_dumb_serial_write(const char *p, int len)
{
    int i;
    for ( i = 0 ; i < len; i++ ) {
        HAL_DIAG_WRITE_CHAR_DIRECT( p[ i ] );
    }
} 


void hal_diag_init(void)
{
}

void hal_diag_write_char(char c)
{
    static char line[100];
    static int pos = 0;

    // No need to send CRs
    if( c == '\r' ) return;

    line[pos++] = c;

    if( c == '\n' || pos == sizeof(line) ) {
        CYG_INTERRUPT_STATE old;

        // Disable interrupts. This prevents GDB trying to interrupt us
        // while we are in the middle of sending a packet. The serial
        // receive interrupt will be seen when we re-enable interrupts
        // later.
        
        HAL_DISABLE_INTERRUPTS(old);
        
        if ( ! hal_bsp_console_write( line, pos ) )
            // then there is no function registered, just spew it out serial
            hal_dumb_serial_write( line, pos );
        
        pos = 0;

        // And re-enable interrupts
        HAL_RESTORE_INTERRUPTS(old);

    }
}

#endif  // CYG_KERNEL_DIAG_GDB_SERIAL_DIRECT not defined; use CygMon

#else // CYG_KERNEL_DIAG_GDB not defined, so we are going to the serial line
      // without GDB encoding - likely to be ROM startup.

/* Address of clock switch */
#define CLKSW_ADDR  0x01000003

/* Address of SW1 */
#define SW1_ADDR  0x02000003

void hal_diag_init(void)
{
    cyg_uint32 clk, tval;

    // first set the baud rate

    clk = *(unsigned char *)CLKSW_ADDR;
    if (clk & 0x80)
        clk = 10;

    clk = (clk & 0x3f) * 1000000;  /* in MHz */

    tval = clk / 19200;
    tval /= 32;
    tval -= 1;

    HAL_SPARC_86940_TCR3_WRITE(
        HAL_SPARC_86940_TCR_CE          |
        HAL_SPARC_86940_TCR_CLKINT      |
        HAL_SPARC_86940_TCR_OUTC3       |
        HAL_SPARC_86940_TCR_SQWAVE           );

    HAL_SPARC_86940_RELOAD3_WRITE( tval);

#define DELAY(x) \
    CYG_MACRO_START int i; for (i = 0; i < x; i++); CYG_MACRO_END

    HAL_SPARC_86940_SDTR0_CTRL_WRITE( 0 );
    DELAY(100);
    HAL_SPARC_86940_SDTR0_CTRL_WRITE( 0 );
    DELAY(100);
    HAL_SPARC_86940_SDTR0_CTRL_WRITE( 0 );
    DELAY(100);
     
    HAL_SPARC_86940_SDTR0_CTRL_WRITE( HAL_SPARC_86940_SER_CMD_IRST );
    DELAY(100);

    /* first write after reset is to mode register */
    HAL_SPARC_86940_SDTR0_CTRL_WRITE( HAL_SPARC_86940_SER_DIV16_CLK     |
                                      HAL_SPARC_86940_SER_8BITS         |
                                      HAL_SPARC_86940_SER_NO_PARITY     |
                                      HAL_SPARC_86940_SER_STOP1           );
    DELAY(100);

    /* subsequent writes are to command register */
    HAL_SPARC_86940_SDTR0_CTRL_WRITE( HAL_SPARC_86940_SER_CMD_RTS       |
                                      HAL_SPARC_86940_SER_CMD_DTR       |
                                      HAL_SPARC_86940_SER_CMD_EFR       |
                                      HAL_SPARC_86940_SER_CMD_RXEN      |
                                      HAL_SPARC_86940_SER_CMD_TXEN        );
    DELAY(100);
}





#endif // CYG_KERNEL_DIAG_GDB

/*---------------------------------------------------------------------------*/
/* End of hal_diag.c */
