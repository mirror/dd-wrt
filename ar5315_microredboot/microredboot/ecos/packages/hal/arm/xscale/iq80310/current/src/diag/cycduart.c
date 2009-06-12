//=============================================================================
//
//      cycduart.c - Cyclone UART Diagnostics
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
#include "cycduart.h"
#include "iq80310.h"
#include "test_menu.h"

#define DFLTLOOPERMS 500

//extern int printf(char*,...);
extern long hexIn(void);

int break_flag = 0;
unsigned long baud_rate = 0;

static int duart_already_init = FALSE;
static unsigned int uart_unit = DFLTPORT;
static int looperms;

static int calc_looperms(void);
void serial_init(void);
int inreg(int);
void outreg(int, unsigned char);
void serial_set(unsigned long);
void serial_loopback(int);
int serial_getc(void);
void serial_putc(int);
int serial_write(int, const unsigned char *, int);
int serial_read(int, unsigned char *, int, int);

extern int enable_external_interrupt (int int_id);
extern int disable_external_interrupt (int int_id);

extern int isr_connect(int int_num, void (*handler)(int), int arg);
extern int isr_disconnect(int int_num);


void duart_initialize(void)
{
    if (duart_already_init == FALSE) {
	/* Calculate the time constant for timeouts on serial_read. */
	if ((looperms = calc_looperms()) <= 0)
	    looperms = DFLTLOOPERMS;
    }

    /* Initialize the serial port and set the baud rate.
     * The baud rate is set here for sanity only; the autobaud
     * mechanism will change it as required when the host connects.
     */

    serial_init();
    serial_set(baud_rate?baud_rate:9600L);  

    duart_already_init = TRUE;
}						


/* Establish the loop/time constant to be used in the timing loop in
 * serial_read.  This is done by putting the UART into loopback mode.
 * After transmitting a character at 300 baud, we wait for the character
 * to be received.  Then divide the number of loops waited by the number
 * of milliseconds it takes to transmit 10 bits at 300 baud.
 * If your transmitter doesn't have a loopback mode, this value can be
 * calculated using a timer or some other facility, or an approximate
 * constant can be used.
 */

#define TESTBAUD 300L
#define NBYTES	 10
#define BITS_PER_BYTE 10	/* 1 start bit, 8 data bits, 1 stop bit */
#define TOTAL_MS (NBYTES*BITS_PER_BYTE*1000/TESTBAUD)

static int
calc_looperms(void)
{
    int	i, count, c;
    int	totalloops = 0;

    serial_init();
    serial_set(TESTBAUD);		/* set 300 baud */
    serial_loopback(1);		/* enable loop back mode */

    for (i=0; i < NBYTES; i++) {
	count = 1;
	serial_putc(0xaa);	/* xmit character */

	/*
	 * The timing loop is the same as the loops in serial_read.
	 * Any changes to the loops in serial_read should be reflected
	 * here.
	 */
	do {
	    c = serial_getc();
	} while (c < 0 && count++ > 0);

	totalloops += count;
    }

    serial_loopback(0);

    return(totalloops/TOTAL_MS);
}

/*
 * Initialize the device driver.
 */
void serial_init(void)
{
    /* If the serial port has been init'd before, there may be data in it  */
    /* Wait for the transmit FIFO to empty out before resetting anything   */  
    if (duart_already_init == TRUE)	{
	while (!(inreg(LSR) & LSR_TSRE));
    }

    /*
     * Configure active port, (uart_unit already set.)
     *
     * Set 8 bits, 1 stop bit, no parity.
     *
     * LCR<7>       0       divisor latch access bit
     * LCR<6>       0       break control (1=send break)
     * LCR<5>       0       stick parity (0=space, 1=mark)
     * LCR<4>       0       parity even (0=odd, 1=even)
     * LCR<3>       0       parity enable (1=enabled)
     * LCR<2>       0       # stop bits (0=1, 1=1.5)
     * LCR<1:0>     11      bits per character(00=5, 01=6, 10=7, 11=8)
     */

    outreg(LCR, 0x3);
   
    /* Assert DTR and RTS to prevent hardware handshake problems with
       serial terminals, etc. which can be connected to the serial port */
    outreg(MCR, MCR_DTR | MCR_RTS);

    outreg(FCR, FIFO_ENABLE);   /* Enable the FIFO                 */
    outreg(IER, INT_ENABLE);    /* Enable appropriate interrupts   */

}

/* Read a received character if one is available.  Return -1 otherwise. */
int serial_getc(void)
{
    if (inreg(LSR) & LSR_DR)
         return inreg(DataIn);

    return -1;
}

/* Transmit a character. */
void serial_putc(int c)
{
    while ((inreg(LSR) & LSR_THRE) == 0)
        ;
    outreg(DataOut, c);
}

/*
 * Set the baud rate.
 */
void serial_set(unsigned long baud)
{
    unsigned char sav_lcr;

    if(baud == 0)
	baud = 9600L;

    /*
     * Enable access to the divisor latches by setting DLAB in LCR.
     *
     */
    sav_lcr = inreg(LCR);
    outreg(LCR, LCR_DLAB | sav_lcr);

    /*
     * Set divisor latches.
     */
    outreg(BaudLsb, XTAL/(16*baud));
    outreg(BaudMsb, (XTAL/(16*baud)) >> 8);

    /*
     * Restore line control register
     */
    outreg(LCR, sav_lcr);
}

/*
 * This routine is used by calc_looperms to put the UART in loopback mode.
 */

void serial_loopback(int flag)
{
    if (flag)
	outreg(MCR, inreg(MCR) | MCR_LOOP);		/* enable loop back mode */
    else
	outreg(MCR, inreg(MCR) & ~MCR_LOOP);	/* disable loop back mode */
}

/*
 * These routines are used to read and write to the registers of the
 * 16552.  The delay routine guarantees the required recovery time between
 * cycles to the 16552.
 * DUART is the base address of the 16552.
 * DUART_DELTA gives the spacing between adjacent registers of the 16552.
 * For example, if A0,A1,A2 of the 16552 are connected to A2,A3,A4 of
 * the processor, DUART_DELTA must be 4.
 */

int inreg(int reg)
{
    int val;
    val = *((volatile unsigned char *)TERMINAL + (uart_unit * SCALE + reg));

    return val;
}

void outreg(int reg, unsigned char val)
{
    *((volatile unsigned char *)TERMINAL + (uart_unit * SCALE + reg)) = val;
}



/****************************************************************/
/* The following functions are all part of the Breeze UART test */
/****************************************************************/
  

static volatile int uart_int;


/************************************************/
/* BUS_TEST										*/
/* This routine performs a walking ones test	*/
/* on the given uart chip to test it's bus		*/
/* interface.  It writes to the scratchpad reg.	*/
/* then reads it back.  During					*/
/* this test all 8 data lines from the chip		*/
/* get written with both 1 and 0.				*/
/************************************************/
static int bus_test (void)
{
    unsigned char	out, in;
    int			bitpos;
    volatile int 	junk;

    junk = (int) &junk;	/* Don't let compiler optimize or "registerize" */

    outreg(SCR,0);		/* Clear scratchpad register */

    for (bitpos = 0; bitpos < 8; bitpos++) {
	out = 1 << bitpos;

	outreg(SCR,out);	/* Write data to scratchpad reg. */

	junk = ~0;			/* Force data lines high */

	in = inreg(SCR);	/* Read data */

	printf ("%02X ", in);

	/* make sure it's what we wrote */
	if (in != out)
	    return (0);
    }
    outreg(SCR,0);	/* Clear scratchpad register */
    printf ("\n");

    return (1);
}

/************************************************/
/* DISABLE_UART_INTS							*/
/* This routine disables uart interrupts		*/
/************************************************/
static void disable_uart_ints (void)
{
    outreg(IER,0);		/* Make the uart shut up */
}

/************************************************/
/* UART_ISR										*/
/* This routine responds to uart interrupts		*/
/* must return 1 to indicate that an interrupt  */
/* was serviced.								*/
/************************************************/
static void uart_isr (int unused)
{
    unsigned char iir;

    disable_uart_ints ();
    uart_int = 1;

    /* read the IIR to clear the interrupt */
    iir = inreg(IIR);

    return ;
}

/************************************************/
/* INIT_UART									*/
/* This routine initializes the 16550 interrupt */
/* and uart registers and initializes the uart  */
/* count.										*/
/************************************************/
static void init_uart (void)
{
    outreg(IER,0x02);		/* Enable Tx Empty interrupt -
				   should generate an interrupt since Tx is
				   empty to begin with */
}


/****************************************/
/* UART DIAGNOSTIC TEST					*/
/****************************************/
void uart_test (MENU_ARG arg)
{
    volatile int loop;
    int	looplim;
    int int_id;
    int i, baud;

    /*11/01/00 */
    char info[] = {"Move Console Cable back to Connector J9 and hit <CR> to exit test"};
    int index;

    looplim = 400000;

    /* perform tests on both UARTs */
    for (uart_unit = 0; uart_unit < 2; uart_unit++)	{

	if (uart_unit == 0)
	    int_id = UART1_INT_ID;
	else
	    int_id = UART2_INT_ID;
				
	if (!bus_test ())
	    printf ("\nERROR:  bus_test for UART Unit %d failed\n", uart_unit);
	else {
	    printf ("\nbus_test for UART Unit %d passed\n", uart_unit);
		
	    uart_int = 0;   

	    isr_connect (int_id, uart_isr, 0);

	    if (enable_external_interrupt(int_id) != OK)
		printf("ERROR enabling UART UINT %d interrupt!\n", uart_unit);

	    init_uart ();
		
	    loop = 0;

	    while (!uart_int && (loop < looplim))
		loop++;
	    if (!uart_int)
		printf ("UART Unit %d INTERRUPT test failed %X\n", uart_unit, loop) ;
	    else
		printf ("UART Unit %d INTERRUPT test passed\n", uart_unit);

	    serial_putc(' ');
	}

	/* disable UART interrupt */
	if (disable_external_interrupt(int_id)!= OK)
	    printf("ERROR disabling UART UNIT %d interrupt!\n", uart_unit);

	/* disconnect test handler */
	isr_disconnect (int_id);

    }

    /* 11/01/00 */
    /* #if 0 */	 /* writing to port 2 doesnt work yet... */
#if 1 /* writing to port 2 doesnt work yet... */

/*
	printf ("\nMove the Console Cable to the 2nd Serial Port,\n");
	printf ("Connector J10,\n");
    printf ("and Hit <CR> when the cable is connected.\n\n");
	printf ("After alphabet prints, move Console Cable back to 1st Serial Port,\n");
	printf ("Connector J9,\n");
	printf ("and hit <CR> to exit test\n");
*/  

/* 10/30/00 */
    uart_unit = DFLTPORT;	/* test J10, the PCI-700 GDB port */

    printf ("\nMove the Console Cable to the 2nd Serial Port, Connector J10,\n");
    printf ("and Hit <CR> when the cable is connected.\n");
    printf ("The alphabet should print on the screen.\n\n");

/* 11/01/00 */
/*
	printf ("After alphabet prints, move Console Cable back to 1st Serial Port,\n");
	printf ("Connector J9,\n");
	printf ("and hit <CR> to exit test\n");
*/
    baud = 115200;
    serial_init();
    serial_set(baud?baud:115200L);

/*	while (serial_getc() == -1); */
    while (serial_getc() != 0x0d);	/* wait for a carriage return character to start test */

/*
	while (1)
	{
		for ( i = 65; i <= 90; i++ )
			serial_putc(i);
	}
*/
    for ( i = 65; i <= 90; i++ )	/* transmit the alphabet */
	serial_putc(i);

    serial_putc(10);	/* transmit a New Line */
    serial_putc(13);	/* transmit a Carriage Return */
    serial_putc(10);	/* transmit a New Line */

    for (index=0; info[index] != '\0'; index++)	/* transmit some instructions to the user */
	serial_putc(info[index]);

    /* point at default port before returning */
    /*	uart_unit = DFLTPORT; */

    (void)hexIn();
  
#endif
	
    printf ("\n\nUART tests done.\n");
    printf ("Press return to continue.\n");
    (void) hexIn();
}




