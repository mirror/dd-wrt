#ifndef CYGONCE_DEVS_ETH_ARM_EBSA285_TESTS_TEST_NET_REALTIME_H
#define CYGONCE_DEVS_ETH_ARM_EBSA285_TESTS_TEST_NET_REALTIME_H
/*==========================================================================
//
//        test_net_realtime.h
//
//        Auxiliary test header file
//        Provide a thread that runs on EBSA only, which verifies that
//        realtime characteristics are preserved.
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     hmt
// Contributors:  hmt
// Date:          2000-05-03
// Description:
//
//####DESCRIPTIONEND####
*/

// This is the API to this file:

#define TNR_OFF() tnr_active = 0
#define TNR_ON()  tnr_active = 1
#define TNR_INIT() tnr_init()
#define TNR_PRINT_ACTIVITY() tnr_print_activity() 

// Tests should use these if they are defined to test that the realtime
// characteristics of the world are preserved during a test.
//
// It is accepted that printing stuff via diag_printf() (and the test
// infra) disables interrupts for a long time.  So invoke TNR_OFF/ON()
// either side of diagnostic prints, to prevent boguf firings of the
// realtime test.

// ------------------------------------------------------------------------

// This file rather assumes that the network is in use, and that therefore
// there is also a kernel, and so on....

#include <cyg/infra/testcase.h>         // CYG_TEST_FAIL et al
#include <cyg/infra/diag.h>             // diag_printf()
#include <cyg/kernel/kapi.h>            // Thread API

#include <cyg/hal/hal_arch.h>           // CYGNUM_HAL_STACK_SIZE_TYPICAL
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_ebsa285.h>        // Hardware definitions

// The EBSA has 4 hardware timers; timer 3 is the kernel's realtime clock
// because it is connected to a separate, indepenent 3.68MHz signal; timer
// 4 can be used as a watchdog.  So we have timers 1 and 2 to use.
// Timers 1 and 2 have an input clock of 50MHz on fclk_in.
// Timer 2 should be initialized for periodic interrupts per 500uS.
// Timer 1 should be initialized for a one-shot interrupt after 1mS (1000uS).
//
// Timer 2's ISR examines the state of timer 1; if it has expired, the test
// has failed.  7 out of 8 hits, timer 1 is reinitialized for the 1mS; on
// the 8th event, timer1 is set for 2mS.  The next timer 2 event calls its
// DSR, which in turn signals a semaphore which awakens a real task, which
// again checks and re-initializes timer1 in the same way.
//
// All this ensures that interrupts are never delayed by more than 500uS,
// and that signalling a real task always takes less than 1500uS.
//
// This system, once activated, will run non-intrusively along with all
// networking tests.
//
// Special care (aka a hack) may be needed to make it work with the
// diagnostic channel; that disables interrupts typically for
//   100[characters] * 8[bits/byte] / 38400[Baud]  [Seconds] = 20mS.

// Use the fclk_in divided-by 256 mode:
#if 0 // Default, really 1mS, 2mS, 500uS
#define TNR_TIMER1_PERIOD_1mS     ((50 * 1000)     >>8)
#define TNR_TIMER1_PERIOD_2mS     ((50 * 1000 * 2) >>8)
#define TNR_TIMER2_PERIOD_500uS   ((50 *  500)     >>8)
#else // pushing the envelope... 1/5 as much:
#define FACTOR 200 // 1000 is "normal"
#define TNR_TIMER1_PERIOD_1mS     ((50 * FACTOR)     >>8)
#define TNR_TIMER1_PERIOD_2mS     ((50 * FACTOR * 2) >>8)
#define TNR_TIMER2_PERIOD_500uS   ((50 * FACTOR / 2) >>8)
#endif

#define TNR_TIMER1_INIT   (0x88)  // Enabled, free running, fclk_in/256
#define TNR_TIMER2_INIT   (0xc8)  // Enabled, periodic, fclk_in/256

// This way, if timer1 is > TNR_TIMER1_PERIOD_2mS, then we know it has
// wrapped; its full range is 85 seconds, one would hope to get back in
// that time!

static volatile int tnr_active = 0;
static volatile int tnr_t2_counter = 0;

static  cyg_sem_t tnr_sema;

static char tnr_stack[CYGNUM_HAL_STACK_SIZE_TYPICAL];
static cyg_thread tnr_thread_data;
static cyg_handle_t tnr_thread_handle;

static cyg_interrupt tnr_t1_intr, tnr_t2_intr;
static cyg_handle_t tnr_t1_inth, tnr_t2_inth;

struct {
    int timer1_isr;
    int timer2_isr;
    int timer2_isr_active;
    int timer2_dsr;
    int timer2_thd;
    int timer2_thd_active;
} tnr_activity_counts = { 0,0,0,0,0,0 };
        

static cyg_uint32 tnr_timer1_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    tnr_activity_counts.timer1_isr++;

    if ( tnr_active )
        CYG_TEST_FAIL_EXIT( "test_net_realtime: Timer1 fired" );

    *SA110_TIMER1_CLEAR = 0; // Clear any pending interrupt (Data: don't care)
    HAL_INTERRUPT_ACKNOWLEDGE( CYGNUM_HAL_INTERRUPT_TIMER_1 );

    return CYG_ISR_HANDLED;
}

static cyg_uint32 tnr_timer2_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    tnr_activity_counts.timer2_isr++;

    *SA110_TIMER2_CLEAR = 0; // Clear any pending interrupt (Data: don't care)
    HAL_INTERRUPT_ACKNOWLEDGE( CYGNUM_HAL_INTERRUPT_TIMER_2 );

    if ( tnr_active ) {
        tnr_activity_counts.timer2_isr_active++;
        if ( (*SA110_TIMER1_VALUE) > (4 * TNR_TIMER1_PERIOD_1mS) ) {
            // Then it has wrapped around, bad bad bad
            CYG_TEST_FAIL_EXIT( "tnr_timer2_isr: Timer1 wrapped" );
        }
    }
    tnr_t2_counter++;
    // We go though each of the following states in turn:
    switch ( tnr_t2_counter & 7 ) {
    case 0:
        // Then this is an 8th event:
        *SA110_TIMER1_LOAD = TNR_TIMER1_PERIOD_2mS;
        return CYG_ISR_HANDLED;
    case 1:
        return CYG_ISR_CALL_DSR; // See how long to call a DSR &c..
        // without resetting timer1: 1500uS left now
    default:
        // Reset timer1 again.  By doing this in time every time it should
        // never fire.
        *SA110_TIMER1_LOAD = TNR_TIMER1_PERIOD_1mS;
    }
    return CYG_ISR_HANDLED;
}
    
static void tnr_timer2_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    tnr_activity_counts.timer2_dsr++;

    if ( CYGNUM_HAL_INTERRUPT_TIMER_2 != vector )
        CYG_TEST_FAIL_EXIT( "tnr_timer2_dsr: Bad vector" );

    cyg_semaphore_post( &tnr_sema );
}

static void tnr_timer2_service_thread( cyg_addrword_t param )
{
    while (1) {
        cyg_semaphore_wait( &tnr_sema );
        tnr_activity_counts.timer2_thd++;
        if ( tnr_active ) {
            tnr_activity_counts.timer2_thd_active++;
            if ( (*SA110_TIMER1_VALUE) > (4 * TNR_TIMER1_PERIOD_1mS) ) {
                // Then it has wrapped around, bad bad bad
                CYG_TEST_FAIL_EXIT( "tnr_timer2_service_thread: Timer1 wrapped" );
            }
        }
        // Reset timer1 again.  By doing this in time every time it should
        // never fire.
        *SA110_TIMER1_LOAD = TNR_TIMER1_PERIOD_1mS;
    }
}


static void tnr_init( void )
{
    // init the semaphore
    cyg_semaphore_init( &tnr_sema, 0 );

    // create and start the thread
    cyg_thread_create(2,                  // Priority - just a number
                      tnr_timer2_service_thread,
                      0,                  // entry parameter
                      "Test Net Realtime tnr_timer2_service_thread",
                     &tnr_stack[0],       // Stack
                      sizeof(tnr_stack),  // Size
                      &tnr_thread_handle, // Handle
                      &tnr_thread_data    // Thread data structure
        );
    cyg_thread_resume( tnr_thread_handle );

    // set up and attach the interrupts et al...
    cyg_interrupt_create(
        CYGNUM_HAL_INTERRUPT_TIMER_2,   /* Vector to attach to       */
        0,                              /* Queue priority            */
        0,                              /* Data pointer              */
        tnr_timer2_isr,                 /* Interrupt Service Routine */
        tnr_timer2_dsr,                 /* Deferred Service Routine  */
        &tnr_t2_inth,                   /* returned handle           */
        &tnr_t2_intr                    /* put interrupt here        */
        );
    
    cyg_interrupt_create(
        CYGNUM_HAL_INTERRUPT_TIMER_1,   /* Vector to attach to       */
        0,                              /* Queue priority            */
        0,                              /* Data pointer              */
        tnr_timer1_isr,                 /* Interrupt Service Routine */
        tnr_timer2_dsr, /* re-use! */   /* Deferred Service Routine  */
        &tnr_t1_inth,                   /* returned handle           */
        &tnr_t1_intr                    /* put interrupt here        */
        );
    
    cyg_interrupt_attach( tnr_t1_inth );
    cyg_interrupt_attach( tnr_t2_inth );

    *SA110_TIMER1_CONTROL = 0;          // Disable while we are setting up
    *SA110_TIMER1_LOAD = TNR_TIMER1_PERIOD_2mS;
    *SA110_TIMER1_CLEAR = 0;            // Clear any pending interrupt
    *SA110_TIMER1_CONTROL = TNR_TIMER1_INIT;
    *SA110_TIMER1_CLEAR = 0;            // Clear any pending interrupt again

    *SA110_TIMER2_CONTROL = 0;          // Disable while we are setting up
    *SA110_TIMER2_LOAD = TNR_TIMER2_PERIOD_500uS;
    *SA110_TIMER2_CLEAR = 0;            // Clear any pending interrupt
    *SA110_TIMER2_CONTROL = TNR_TIMER2_INIT;
    *SA110_TIMER2_CLEAR = 0;            // Clear any pending interrupt again

    cyg_interrupt_unmask( CYGNUM_HAL_INTERRUPT_TIMER_2 );
    cyg_interrupt_unmask( CYGNUM_HAL_INTERRUPT_TIMER_1 );
}

static void tnr_print_activity( void )
{
    int tmp = tnr_active;
    tnr_active = 0;
    diag_printf( "Test-net-realtime: interrupt activity log:\n" );
    diag_printf( "    timer1_isr %10d\n", tnr_activity_counts.timer1_isr );
    diag_printf( "    timer2_isr %10d\n", tnr_activity_counts.timer2_isr );
    diag_printf( "      (active) %10d\n", tnr_activity_counts.timer2_isr_active );
    diag_printf( "    timer2_dsr %10d\n", tnr_activity_counts.timer2_dsr );
    diag_printf( "    timer2_thd %10d\n", tnr_activity_counts.timer2_thd );
    diag_printf( "      (active) %10d\n", tnr_activity_counts.timer2_thd_active );
    tnr_active = tmp;
}

#endif /* ifndef CYGONCE_DEVS_ETH_ARM_EBSA285_TESTS_TEST_NET_REALTIME_H */

/* EOF test_net_realtime.h */
