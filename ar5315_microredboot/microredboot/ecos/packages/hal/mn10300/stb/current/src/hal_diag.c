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

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>          // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_misc.h>

/*---------------------------------------------------------------------------*/
/* Select default diag channel to use                                        */

//#define CYG_KERNEL_DIAG_ROMART
//#define CYG_KERNEL_DIAG_SERIAL0
//#define CYG_KERNEL_DIAG_SERIAL1
//#define CYG_KERNEL_DIAG_SERIAL2
//#define CYG_KERNEL_DIAG_BUFFER
//#define CYG_KERNEL_DIAG_GDB

#if !defined(CYG_KERNEL_DIAG_SERIAL0) && \
    !defined(CYG_KERNEL_DIAG_SERIAL1) && \
    !defined(CYG_KERNEL_DIAG_SERIAL2) && \
    !defined(CYG_KERNEL_DIAG_ROMART)

#if defined(CYGSEM_HAL_USE_ROM_MONITOR_GDB_stubs)

#define CYG_KERNEL_DIAG_SERIAL0
#define CYG_KERNEL_DIAG_GDB

#else

#define CYG_KERNEL_DIAG_SERIAL0

#endif

#endif

//#define CYG_HAL_MN10300_STB_SERIAL2

/*---------------------------------------------------------------------------*/
// MN10300 Serial line

#if defined(CYG_HAL_MN10300_STB_SERIAL0) || defined(CYG_KERNEL_DIAG_SERIAL0)

// We use serial0 on AM33
#define SERIAL0_CR       ((volatile cyg_uint16 *)0xd4002000)
#define SERIAL0_ICR      ((volatile cyg_uint8 *) 0xd4002004)
#define SERIAL0_TXR      ((volatile cyg_uint8 *) 0xd4002008)
#define SERIAL0_RXR      ((volatile cyg_uint8 *) 0xd4002009)
#define SERIAL0_SR       ((volatile cyg_uint16 *)0xd400200c)

// Timer 2 provides baud rate divisor
#define TIMER0_MD       ((volatile cyg_uint8 *)0xd4003002)
#define TIMER0_BR       ((volatile cyg_uint8 *)0xd4003012)
#define TIMER0_CR       ((volatile cyg_uint8 *)0xd4003022)

// Timer 0 provides a prescaler for lower baud rates
#define HW_TIMER0_MD       ((volatile cyg_uint8 *)0xd4003000)
#define HW_TIMER0_BR       ((volatile cyg_uint8 *)0xd4003010)
#define HW_TIMER0_CR       ((volatile cyg_uint8 *)0xd4003020)

#define SIO_LSTAT_TRDY  0x20
#define SIO_LSTAT_RRDY  0x10

#define SERIAL_CR_TXE   0x8000

void hal_diag_init_serial0(void)
{
#if 1    
    // 99 translates to 38400 baud.
    *TIMER0_BR = 99;
    // Timer0 sourced from IOCLK
    *TIMER0_MD = 0x80;    
#else
    // 1 and 198 translate into 9800 baud
    *TIMER0_BR = 1;
    *TIMER0_MD = 0x84;          // source = timer 0 overflow
    *HW_TIMER0_BR = 198;        // timer 0 base register
    *HW_TIMER0_CR = 0x80;       // source from ioclk
#endif
    
    // No interrupts for now.
    *SERIAL0_ICR = 0x00;

    // Source from timer 1, 8bit chars, enable tx and rx
    *SERIAL0_CR = 0xc085;    
}

void hal_diag_write_char_serial0(char c)
{
    register volatile cyg_uint16 *volatile tty_status = SERIAL0_SR;
    register volatile cyg_uint8 *volatile tty_tx = SERIAL0_TXR;

    while( (*tty_status & SIO_LSTAT_TRDY) != 0 ) continue;

    *tty_tx = c;
}

void hal_diag_drain_serial0(void)
{
    register volatile cyg_uint16 *volatile tty_status = SERIAL0_SR;

    while( (*tty_status & SIO_LSTAT_TRDY) != 0 ) continue;
}

void hal_diag_read_char_serial0(char *c)
{
    register volatile cyg_uint16 *volatile tty_status = SERIAL0_SR;
    register volatile cyg_uint8 *volatile tty_rx = SERIAL0_RXR;

    while( (*tty_status & SIO_LSTAT_RRDY) == 0 ) continue;

    *c = *tty_rx;

    // We must ack the interrupt caused by that read to avoid
    // confusing the GDB stub ROM.
    HAL_INTERRUPT_ACKNOWLEDGE( CYGNUM_HAL_INTERRUPT_SERIAL_0_RX );    
}

#if defined(CYG_KERNEL_DIAG_SERIAL0)

#define hal_diag_init_serial hal_diag_init_serial0
#define hal_diag_write_char_serial hal_diag_write_char_serial0
#define hal_diag_drain_serial hal_diag_drain_serial0
#define hal_diag_read_char_serial hal_diag_read_char_serial0

#endif

#endif

/*---------------------------------------------------------------------------*/
// MN10300 Serial line

#if defined(CYG_HAL_MN10300_STB_SERIAL1) || defined(CYG_KERNEL_DIAG_SERIAL1)

// We use serial1 on MN103002
#define SERIAL1_CR       ((volatile cyg_uint16 *)0xd4002010)
#define SERIAL1_ICR      ((volatile cyg_uint8 *) 0xd4002014)
#define SERIAL1_TXR      ((volatile cyg_uint8 *) 0xd4002018)
#define SERIAL1_RXR      ((volatile cyg_uint8 *) 0xd4002019)
#define SERIAL1_SR       ((volatile cyg_uint16 *)0xd400201c)

// Timer 1 provided baud rate divisor
#define TIMER1_MD       ((volatile cyg_uint8 *)0xd4003001)
#define TIMER1_BR       ((volatile cyg_uint8 *)0xd4003011)
#define TIMER1_CR       ((volatile cyg_uint8 *)0xd4003021)

// Timer 0 provides a prescaler for lower baud rates
#define HW_TIMER0_MD       ((volatile cyg_uint8 *)0xd4003000)
#define HW_TIMER0_BR       ((volatile cyg_uint8 *)0xd4003010)
#define HW_TIMER0_CR       ((volatile cyg_uint8 *)0xd4003020)

#define SIO1_LSTAT_TRDY  0x20
#define SIO1_LSTAT_RRDY  0x10

#define SERIAL_CR_TXE   0x8000

void hal_diag_init_serial1(void)
{
#if 1    
    // 99 translates to 38400 baud.
    *TIMER1_BR = 99;

    // Timer1 sourced from IOCLK
    *TIMER1_MD = 0x80;    

#else

    // 1 and 198 translate into 9800 baud
    *TIMER1_BR = 1;
    *TIMER1_MD = 0x84;          // source = timer 0 overflow
    *HW_TIMER0_BR = 198;        // timer 0 base register
    *HW_TIMER0_CR = 0x80;       // source from ioclk
#endif
    
    // No interrupts for now.
    *SERIAL1_ICR = 0x00;

    // Source from timer 1, 8bit chars, enable tx and rx
    *SERIAL1_CR = 0xc084;    
}

void hal_diag_write_char_serial1(char c)
{
    register volatile cyg_uint16 *volatile tty_status = SERIAL1_SR;
    register volatile cyg_uint8 *volatile tty_tx = SERIAL1_TXR;

    while( (*tty_status & SIO1_LSTAT_TRDY) != 0 ) continue;

    *tty_tx = c;
}

void hal_diag_drain_serial1(void)
{
    register volatile cyg_uint16 *volatile tty_status = SERIAL1_SR;

    while( (*tty_status & SIO1_LSTAT_TRDY) != 0 ) continue;
}

void hal_diag_read_char_serial1(char *c)
{
    register volatile cyg_uint16 *volatile tty_status = SERIAL1_SR;
    register volatile cyg_uint8 *volatile tty_rx = SERIAL1_RXR;

    while( (*tty_status & SIO1_LSTAT_RRDY) == 0 ) continue;

    *c = *tty_rx;

    // We must ack the interrupt caused by that read to avoid
    // confusing the GDB stub ROM.
    HAL_INTERRUPT_ACKNOWLEDGE( CYGNUM_HAL_INTERRUPT_SERIAL_1_RX );
}

#if defined(CYG_KERNEL_DIAG_SERIAL1)

#define hal_diag_init_serial hal_diag_init_serial1
#define hal_diag_write_char_serial hal_diag_write_char_serial1
#define hal_diag_drain_serial hal_diag_drain_serial1
#define hal_diag_read_char_serial hal_diag_read_char_serial1

#endif

#endif

/*---------------------------------------------------------------------------*/

#if defined(CYG_HAL_MN10300_STB_SERIAL2) || defined(CYG_KERNEL_DIAG_SERIAL2)

// We use serial2 on MN103002
#define SERIAL2_CR       ((volatile cyg_uint16 *)0xd4002020)
#define SERIAL2_ICR      ((volatile cyg_uint8 *) 0xd4002024)
#define SERIAL2_TXR      ((volatile cyg_uint8 *) 0xd4002028)
#define SERIAL2_RXR      ((volatile cyg_uint8 *) 0xd4002029)
#define SERIAL2_SR       ((volatile cyg_uint8 *) 0xd400202c)
#define SERIAL2_TR       ((volatile cyg_uint8 *) 0xd400202d)

// Timer 3 provides baud rate divisor
#define TIMER2_MD       ((volatile cyg_uint8 *)0xd4003003)
#define TIMER2_BR       ((volatile cyg_uint8 *)0xd4003013)
#define TIMER2_CR       ((volatile cyg_uint8 *)0xd4003023)

#define SIO2_LSTAT_TRDY  0x20
#define SIO2_LSTAT_RRDY  0x10

void hal_diag_init_serial2(void)
{
#if 0
    {
        int i,j;
        for( j = 1; j < 255; j++ )
        {
            for(i = 1; i < 127; i++)
            {
                *TIMER2_BR = j;
                *SERIAL2_TR = i;
    
                // Timer2 sourced from IOCLK
                *TIMER2_MD = 0x80;    

                // No interrupts for now.
                *SERIAL2_ICR = 0x00;

                // Source from timer 2, 8bit chars, enable tx and rx
                *SERIAL2_CR = 0xc081;

                diag_printf("\r\n<%03d,%03d>1234567890abcdefgh<%03d,%03d>\r\n",j,i,j,i);
            }
        }
    }
#endif
    
    // 7 and 102 translate to 38400 baud.
    // The AM33 documentation says that these values should be 7 and 113.
    // I have no explanation as to why there is such a discrepancy between the
    // documentation and the hardware.
    
    *TIMER2_BR = 7;
    *SERIAL2_TR = 102;
    
    // Timer2 sourced from IOCLK
    *TIMER2_MD = 0x80;    

    // No interrupts for now.
    *SERIAL2_ICR = 0x00;

    // Source from timer 3, 8bit chars, enable tx and rx
    *SERIAL2_CR = 0xc083;

}

void hal_diag_write_char_serial2(char c)
{
    register volatile cyg_uint8 *volatile tty_status = SERIAL2_SR;
    register volatile cyg_uint8 *volatile tty_tx = SERIAL2_TXR;

    while( (*tty_status & SIO2_LSTAT_TRDY) != 0 ) continue;

    *tty_tx = c;
}

void hal_diag_drain_serial2(void)
{
    register volatile cyg_uint8 *volatile tty_status = SERIAL2_SR;

    while( (*tty_status & SIO2_LSTAT_TRDY) != 0 ) continue;
}

void hal_diag_read_char_serial2(char *c)
{
    register volatile cyg_uint8 *volatile tty_status = SERIAL2_SR;
    register volatile cyg_uint8 *volatile tty_rx = SERIAL2_RXR;

    while( (*tty_status & SIO2_LSTAT_RRDY) == 0 ) continue;

    *c = *tty_rx;

    // We must ack the interrupt caused by that read to avoid
    // confusing the GDB stub ROM.
    HAL_INTERRUPT_ACKNOWLEDGE( CYGNUM_HAL_INTERRUPT_SERIAL_2_RX );
            
    
}

#if defined(CYG_KERNEL_DIAG_SERIAL2)

#define hal_diag_init_serial hal_diag_init_serial2
#define hal_diag_write_char_serial hal_diag_write_char_serial2
#define hal_diag_drain_serial hal_diag_drain_serial2
#define hal_diag_read_char_serial hal_diag_read_char_serial2

#endif

#endif

/*---------------------------------------------------------------------------*/


#if defined(CYG_KERNEL_DIAG_BUFFER)

char hal_diag_buffer[10000];
int hal_diag_buffer_pos;

void hal_diag_init_buffer(void)
{
    hal_diag_buffer_pos = 0;
}

void hal_diag_write_char_buffer(char c)
{
    hal_diag_buffer[hal_diag_buffer_pos++] = c;
    if (hal_diag_buffer_pos >= sizeof(hal_diag_buffer) )
        hal_diag_buffer_pos = 0;
}

void hal_diag_drain_buffer(void)
{
}

void hal_diag_read_char_buffer(char *c)
{
    *c = '\n';
}

#define hal_diag_init_serial hal_diag_init_buffer
#define hal_diag_write_char_serial hal_diag_write_char_buffer
#define hal_diag_drain_serial hal_diag_drain_buffer
#define hal_diag_read_char_serial hal_diag_read_char_buffer

#endif


/*---------------------------------------------------------------------------*/

void hal_diag_init(void)
{
    hal_diag_init_serial();
}

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
        CYG_BYTE wdcr;
        HAL_DISABLE_INTERRUPTS(oldstate);

        // Beacuse of problems with NT on the testfarm, we also have
        // to disable the watchdog here. This only matters in the
        // watchdog tests. And yes, this sends my irony meter off the
        // scale too.

        HAL_READ_UINT8( 0xC0001002, wdcr );
        HAL_WRITE_UINT8( 0xC0001002, wdcr&0x3F );
        
        while(1)
        {
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


            {
                char c1;

                hal_diag_read_char_serial( &c1 );

                if( c1 == '+' ) break;

                
                if( cyg_hal_is_break( &c1, 1 ) )
                    cyg_hal_user_break( NULL );

            }
        }
        
        pos = 0;

        // Wait for tx buffer to drain
        hal_diag_drain_serial();
        
        // And re-enable interrupts
        HAL_RESTORE_INTERRUPTS(oldstate);        
        HAL_WRITE_UINT8( 0xC0001002, wdcr );        
    }
    
#else
    hal_diag_write_char_serial(c);
#endif    
}




void hal_diag_read_char(char *c)
{
    hal_diag_read_char_serial(c);
}


/*---------------------------------------------------------------------------*/
/* End of hal_diag.c */
