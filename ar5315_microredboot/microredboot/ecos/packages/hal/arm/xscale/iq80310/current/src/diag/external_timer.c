//=============================================================================
//
//      external_timer.c - Cyclone Diagnostics
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
// Author(s):   Scott Coulter, Jeff Frazier, Eric Breeden
// Contributors:
// Date:        2001-01-25
// Purpose:     
// Description: 
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <redboot.h>
#include <cyg/hal/hal_iop310.h>        // Hardware definitions
#include <cyg/hal/hal_intr.h>          // Interrupt names
#include "iq80310.h"
#include "test_menu.h"

extern int enable_external_interrupt (int int_id);
extern int disable_external_interrupt (int int_id);
extern int isr_connect(int int_num, void (*handler)(int), int arg);
extern int isr_disconnect(int int_num);
extern char xgetchar(void);

volatile int timer_ticks;

/* interrupt handler for the PAL-based external timer */
void ext_timer_handler (int arg)
{
    /* increment tick counter */
    timer_ticks++;

    /* to clear the timer interrupt, clear the timer interrupt
       enable, then re-set the int. enable bit */
    /* 01/05/01 jwf */
    /*	_restart_tmr(); */

    EXT_TIMER_INT_DISAB();
    EXT_TIMER_INT_ENAB();

    return;
}


/* timer count must be written 8 bits at a time */
void write_timer_count (UINT32 count)
{
    UINT8 cnt_word;

    /* first ensure that there are only 22 bits of count data */
    count &= 0x003fffff;

    /* grab least significant 8 bits of timer value */
    cnt_word = (UINT8)(count & 0xff);
    *TIMER_LA0_REG_ADDR = cnt_word;

    /* grab next 8 bits of timer value */
    count = (count >> 8);
    cnt_word = (UINT8)(count & 0xff);
    *TIMER_LA1_REG_ADDR = cnt_word;

    /* grab last 6 bits of timer value */
    count = (count >> 8);
    cnt_word = (UINT8)(count & 0x3f);
    *TIMER_LA2_REG_ADDR = cnt_word;

    return;
}

/* timer must be read 6 bits at a time */
UINT32 read_timer_count (void)
{
    UINT8 timer_cnt0, timer_cnt1, timer_cnt2, timer_cnt3;
    UINT8 timer_byte0, timer_byte1, timer_byte2;
    UINT32 count;

    /* first read latches the count */
    timer_cnt0 = (*TIMER_LA0_REG_ADDR & TIMER_COUNT_MASK);
    timer_cnt1 = (*TIMER_LA1_REG_ADDR & TIMER_COUNT_MASK);
    timer_cnt2 = (*TIMER_LA2_REG_ADDR & TIMER_COUNT_MASK);
    timer_cnt3 = (*TIMER_LA3_REG_ADDR & 0xf);	/* only 4 bits in most sig. */

    /* now build up the count value */
    timer_byte0 = (((timer_cnt0 & 0x20) >> 1) | (timer_cnt0 & 0x1f));
    timer_byte1 = (((timer_cnt1 & 0x20) >> 1) | (timer_cnt1 & 0x1f));
    timer_byte2 = (((timer_cnt2 & 0x20) >> 1) | (timer_cnt2 & 0x1f));

    count = ((timer_cnt3 << 18) | (timer_byte2 << 12) | (timer_byte1 << 6) |
	     timer_byte0);

    return (count);
}


/* 12/18/00 jwf */
/* This test reads the timer la0-la3 registers on the fly while an up count is in progress. */
void counter_test (void)
{
    /* ff max, b0-b7, b0-b7 contain timer load data */	
    unsigned char TmrLa0Write=0xff;
    /* ff max, b8-b15, b0-b7 contain timer load data  */
    unsigned char TmrLa1Write=0xff;
    /* 3f max, b16-b21, b0-b5 contain timer load data  */
    unsigned char TmrLa2Write=0x3f;
    unsigned char TmrLa3Write=0x00; /* x - don't care */
    unsigned long int TmrLa0Read=0;	
    unsigned long int TmrLa1Read=0;
    unsigned long int TmrLa2Read=0;
    unsigned long int TmrLa3Read=0;
    unsigned long int temp3=0;
    unsigned long int temp4=0;
    unsigned long int CntInit=0;
    unsigned long int CurrentCount;
    unsigned long int LastCount = 0;
    unsigned long int LastLastCount = 0;
    char Error = FALSE;
    unsigned long int sample;
    unsigned long int index;
    const int MAX_N_PASSES = 10; /* N times the counter shall wrap around */
    /* N samples to cover the full range of count,
       0x3fffff/0x40 = 0xffff <--> 65535d, use 65536 to guarantee
       a counter wrap around occurs */
    const unsigned long int MAX_N_SAMPLES = 65536;
    /* allocate 4 bytes per sample for a 0x0 - 0x3fffff count range to
       hold contents of registers LA0-LA3 */
    unsigned long int MAX_N_SIZE = MAX_N_PASSES * MAX_N_SAMPLES * 4;
    unsigned char *data;

    // Arbitrarily pick a spot in memory.
    // RedBoot won't ever use more than 1MB.
    data = (unsigned char *) MEMBASE_DRAM + (1*1024*1024);

    if (data != NULL) {
	printf("Allocated %d bytes\n", MAX_N_SIZE);

	/* load control data to disable timer enable b0=0 and timer disable
	   interrupt b1=0, write to timer enable port */
	EXT_TIMER_INT_DISAB();
	EXT_TIMER_CNT_DISAB();

	/* write timer la0 port count data */
	*TIMER_LA0_REG_ADDR = TmrLa0Write;

	/* write timer la1 port count data */
	*TIMER_LA1_REG_ADDR = TmrLa1Write;

	/*  write timer la2 port count data */
	*TIMER_LA2_REG_ADDR = TmrLa2Write;

	/*  write timer la3 port count data */
	*TIMER_LA3_REG_ADDR = TmrLa3Write;

	CntInit = TmrLa0Write + (TmrLa1Write << 8 ) + (TmrLa2Write << 16 );

	printf("Timer load data = 0x%x\n", CntInit );

	printf("Reading Timer registers LA0-LA3 on the fly...\n");

	/* load control data to enable timer counter and write control data
	   to start the counter */
	EXT_TIMER_CNT_ENAB();

	/* sample the timer counter on the fly and store LA0-3 register
	   contents in an array */
	/* read LSB register first to latch 22 bits data into four la
	   registers */
	for (sample=0, index=0; sample < (MAX_N_PASSES * MAX_N_SAMPLES); sample++, index += 4) {
	    /* bits 0 1 2 3 4 6 contain count data b0-b5 */
	    data[index]   = *TIMER_LA0_REG_ADDR;
	    /* bits 0 1 2 3 4 6 contain count data b6-b11 */
	    data[index+1] = *TIMER_LA1_REG_ADDR;
	    /* bits 0 1 2 3 4 6 contain count data b12-b17 */
	    data[index+2] = *TIMER_LA2_REG_ADDR;
	    /* bits 0 1 2 3 contain count data b18-b21 */
	    data[index+3] = *TIMER_LA3_REG_ADDR;
	}

	printf("Checking for errors...\n" );

	/* Assemble and check recorded register data for errors */
	for (sample=0, index=0; sample < (MAX_N_PASSES * MAX_N_SAMPLES) ;sample++, index += 4) {
	    /* Assembles counter data that was read on the fly */
	    /* xbxbbbbb */
	    /* 01000000 = 0x40 */
	    /* 00011111 = 0x1F */
	    data[index] &= 0x7f;	/* mask all unused bits */
	    temp3=data[index];
	    temp4=data[index];
	    temp3 &= 0x40;		/* isolate bit 6 */
	    temp3 = temp3 >> 1;		/* shift bit 6 to bit 5 */
	    temp4 &= 0x1f;		/* isolate bits 0-4 */
	    TmrLa0Read = temp3 + temp4;

	    data[index+1] &= 0x7f;	/* mask all unused bits */
	    temp3=data[index+1];
	    temp4=data[index+1];
	    temp3 &= 0x40;		/* isolate bit 6 */
	    temp3 = temp3 >> 1;		/* shift bit 6 to bit 5 */
	    temp4 &= 0x1f;		/* isolate bits 0-4 */
	    TmrLa1Read = temp3 + temp4;

	    data[index+2] &= 0x7f;	/* mask all unused bits */
	    temp3=data[index+2];
	    temp4=data[index+2];
	    temp3 &= 0x40;		/* isolate bit 6 */
	    temp3 = temp3 >> 1;		/* shift bit 6 to bit 5 */
	    temp4 &= 0x1f;		/* isolate bits 0-4 */
	    TmrLa2Read = temp3 + temp4;

	    data[index+3] &= 0x0f;	/* mask all unused bits */
	    TmrLa3Read = data[index+3];

	    /* sum timer count data */
	    CurrentCount = TmrLa0Read + (TmrLa1Read << 6)
		+ (TmrLa2Read << 12) + (TmrLa3Read << 18);

	    if (sample == 0) {
		LastLastCount = 0;
		LastCount = CurrentCount;
	    }

	    if (sample == 1) {
		LastLastCount = LastCount;
		LastCount = CurrentCount;
	    }

	    /* check for data anomaly, is count value read 2 samples ago
	       greater than the count value read 1 sample ago */
	    if (sample > 1) {
		/* print error value (LastCount) positioned in between the
		   previous and current values */
		if (LastLastCount > LastCount && CurrentCount > LastLastCount) {
		    /* show error only, do not show a counter wrap around
		       reading, print error value (LastCount) positioned in
		       between the previous and current values */
		    printf("0x%x 0x%x 0x%x \n", LastLastCount, LastCount, CurrentCount );
		    Error = TRUE;	/* set flag to error condition */
		}
		LastLastCount = LastCount;
		LastCount = CurrentCount;
	    }
	}
	/* load control data to stop timer and reset timer interrupt */
	EXT_TIMER_CNT_DISAB();
    } else
	printf( "Cannot allocate memory.\n" );

    if (Error == TRUE)
	printf("Timer LA0-3 register read test FAILED.\n");
    else
	printf("Timer LA0-3 register read test PASSED.\n");
}


/* initialize timer for diagnostic use */
void init_external_timer(void)
{
#if 0
    /* disable timer in case it was running */
    EXT_TIMER_INT_DISAB();
    EXT_TIMER_CNT_DISAB();

    timer_ticks = 0;

    /* connect the timer ISR */
    isr_connect (TIMER_INT_ID, ext_timer_handler, 0);

    /* enable the external interrupt */
    if (enable_external_interrupt(TIMER_INT_ID) != OK)
	printf("ERROR enabling EXT TIMER interrupt!\n");
#else
    hal_clock_initialize(CYGNUM_HAL_RTC_PERIOD);
#endif

}


/* uninitialize timer after diagnostics */
void uninit_external_timer(void)
{
#if 0
    /* disable timer */
    EXT_TIMER_INT_DISAB();
    EXT_TIMER_CNT_DISAB();

    /* disable and disconnect timer interrupts */
    disable_external_interrupt(TIMER_INT_ID);
    isr_disconnect (TIMER_INT_ID);
#endif
}


/* 02/02/01 jwf */
/* delay_ms - delay specified number of milliseconds */
void delay_ms(int num_ms)
{
    HAL_DELAY_US(num_ms * 1000);
}


/* test the 32 bit timer inside the CPLD, U17 */
void timer_test (MENU_ARG arg)
{
    volatile int i;
    UINT32 count;
	
    /*****  Perform 10 second count at 100 ticks/sec ****/

    /* for the test we will setup the timer to generate a 10msec tick */
    count = EXT_TIMER_10MSEC_COUNT;

    /* write the initial count to the timer */
    write_timer_count (count);

    /* enable the interrupt at the timer */
    EXT_TIMER_INT_ENAB();

    /* enable the timer to count */
    EXT_TIMER_CNT_ENAB();

    printf ("Counting at %d Ticks Per Second.\n", TICKS_10MSEC);
    printf ("Numbers should appear on 1 second increments...\n");

    for (i = 0; i < 10; i++) {
	while (timer_ticks < TICKS_10MSEC)
	    ;
	printf ("%d ", i);
	timer_ticks = 0;
    }

    printf ("\nDone\n\n");

    /* disable timer */
    EXT_TIMER_INT_DISAB();
    EXT_TIMER_CNT_DISAB();

	
    /*****  Perform 10 second count at 200 ticks/sec ****/

    count = EXT_TIMER_5MSEC_COUNT;
    write_timer_count (count);

    timer_ticks = 0;

    /* enable the interrupt at the timer */
    EXT_TIMER_INT_ENAB();

    /* enable the timer to count */
    EXT_TIMER_CNT_ENAB();

    printf ("Counting at %d Ticks Per Second.\n", TICKS_5MSEC);
    printf ("Numbers should appear on 1 second increments...\n");

    for (i = 0; i < 10; i++) {
	while (timer_ticks < TICKS_5MSEC)
	    ;
	printf ("%d ", i);
	timer_ticks = 0;
    }

    printf ("\nDone\n\n");

    /* disable timer */
    EXT_TIMER_INT_DISAB();
    EXT_TIMER_CNT_DISAB();

    /* 12/18/00 jwf */
    uninit_external_timer();	/* disable interrupt */
    counter_test();
    init_external_timer();		/* enable interrupt */

    printf("\nExternal Timer Test Done\n");

    /* 12/18/00 jwf */
    printf("\n\nStrike <CR> to exit this test." );
    while (xgetchar() != 0x0d);

    return;

} /* end timer_test() */


