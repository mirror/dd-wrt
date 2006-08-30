/*
<:copyright-gpl 
 Copyright 2003 Broadcom Corp. All Rights Reserved. 
 
 This program is free software; you can distribute it and/or modify it 
 under the terms of the GNU General Public License (Version 2) as 
 published by the Free Software Foundation. 
 
 This program is distributed in the hope it will be useful, but WITHOUT 
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
 for more details. 
 
 You should have received a copy of the GNU General Public License along 
 with this program; if not, write to the Free Software Foundation, Inc., 
 59 Temple Place - Suite 330, Boston MA 02111-1307, USA. 
:>
*/

#include <linux/config.h>
#include <linux/tty.h>
#include <linux/major.h>
#include <linux/init.h>
#include <linux/console.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/sched.h>

#include <bcm_map_part.h>

#undef	PRNT				/* define for debug printing */

#define         UART16550_BAUD_2400             2400
#define         UART16550_BAUD_4800             4800
#define         UART16550_BAUD_9600             9600
#define         UART16550_BAUD_19200            19200
#define         UART16550_BAUD_38400            38400
#define         UART16550_BAUD_57600            57600
#define         UART16550_BAUD_115200           115200

#define         UART16550_PARITY_NONE           0
#define         UART16550_PARITY_ODD            0x08
#define         UART16550_PARITY_EVEN           0x18
#define         UART16550_PARITY_MARK           0x28
#define         UART16550_PARITY_SPACE          0x38

#define         UART16550_DATA_5BIT             0x0
#define         UART16550_DATA_6BIT             0x1
#define         UART16550_DATA_7BIT             0x2
#define         UART16550_DATA_8BIT             0x3

#define         UART16550_STOP_1BIT             0x0
#define         UART16550_STOP_2BIT             0x4

volatile Uart * stUart =  UART_BASE;

#define WRITE16(addr, value)        ((*(volatile UINT16 *)((ULONG)&addr)) = value)

/* Low level UART routines from promcon.c */
extern void prom_putc(char c);
extern char prom_getc(void);
extern int prom_getc_nowait(void);
extern int prom_testc(void);

extern void set_debug_traps(void);
extern void breakpoint(void);
extern void enable_brcm_irq(unsigned int);
extern void set_async_breakpoint(unsigned int epc);

#ifdef CONFIG_GDB_CONSOLE
extern void register_gdb_console(void);
#endif

int gdb_initialized = 0;

#define	GDB_BUF_SIZE	512		/* power of 2, please */

static char	gdb_buf[GDB_BUF_SIZE] ;
static int	gdb_buf_in_inx ;
static atomic_t	gdb_buf_in_cnt ;
static int	gdb_buf_out_inx ;

void debugInit(uint32 baud, uint8 data, uint8 parity, uint8 stop)
{
	/* Do nothing, assume boot loader has already set up serial port */
	printk("debugInit called\n");
}

/*
 * Get a char if available, return -1 if nothing available.
 * Empty the receive buffer first, then look at the interface hardware.
 */
static int	read_char(void)
{
    if (atomic_read(&gdb_buf_in_cnt) != 0)	/* intr routine has q'd chars */
    {
	int		chr ;

	chr = gdb_buf[gdb_buf_out_inx++] ;
	gdb_buf_out_inx &= (GDB_BUF_SIZE - 1) ;
	atomic_dec(&gdb_buf_in_cnt) ;
	return(chr) ;
    }
    return(prom_getc_nowait()) ;	/* read from hardware */
} /* read_char */

/*
 * This is the receiver interrupt routine for the GDB stub.
 * It will receive a limited number of characters of input
 * from the gdb  host machine and save them up in a buffer.
 *
 * When the gdb stub routine getDebugChar() is called it
 * draws characters out of the buffer until it is empty and
 * then reads directly from the serial port.
 *
 * We do not attempt to write chars from the interrupt routine
 * since the stubs do all of that via putDebugChar() which
 * writes one byte after waiting for the interface to become
 * ready.
 *
 * The debug stubs like to run with interrupts disabled since,
 * after all, they run as a consequence of a breakpoint in
 * the kernel.
 *
 * Perhaps someone who knows more about the tty driver than I
 * care to learn can make this work for any low level serial
 * driver.
 */
static void gdb_interrupt(int irq, void *dev_id, struct pt_regs * regs)
{
    int	 chr ;
    int	more;
    do
    {
	chr = prom_getc_nowait() ;
	more = prom_testc();
	if (chr < 0) continue ;

        /* If we receive a Ctrl-C then this is GDB trying to break in */
        if (chr == 3)
	{
	    /* Replace current instruction with breakpoint */
	    set_async_breakpoint(regs->cp0_epc);
            //breakpoint();
	}
		
#ifdef PRNT
	printk("gdb_interrupt: chr=%02x '%c', more = %x\n",
		chr, chr > ' ' && chr < 0x7F ? chr : ' ', more) ;
#endif

	if (atomic_read(&gdb_buf_in_cnt) >= GDB_BUF_SIZE)
	{				/* buffer overflow, clear it */
	    gdb_buf_in_inx = 0 ;
	    atomic_set(&gdb_buf_in_cnt, 0) ;
	    gdb_buf_out_inx = 0 ;
	    break ;
	}

	gdb_buf[gdb_buf_in_inx++] = chr ;
	gdb_buf_in_inx &= (GDB_BUF_SIZE - 1) ;
	atomic_inc(&gdb_buf_in_cnt) ;
    }
    while (more !=0);

} /* gdb_interrupt */

/*
 * getDebugChar
 *
 * This is a GDB stub routine.  It waits for a character from the
 * serial interface and then returns it.  If there is no serial
 * interface connection then it returns a bogus value which will
 * almost certainly cause the system to hang.
 */
int	getDebugChar(void)
{
    volatile int	chr ;

#ifdef PRNT
    printk("getDebugChar: ") ;
#endif

    while ( (chr = read_char()) < 0 ) ;

#ifdef PRNT
    printk("%c\n", chr > ' ' && chr < 0x7F ? chr : ' ') ;
#endif
    return(chr) ;

} /* getDebugChar */

/*
 * putDebugChar
 *
 * This is a GDB stub routine.  It waits until the interface is ready
 * to transmit a char and then sends it.  If there is no serial
 * interface connection then it simply returns to its caller, having
 * pretended to send the char.
 */
int putDebugChar(unsigned char chr)
{
#ifdef PRNT
    printk("putDebugChar: chr=%02x '%c'\n", chr,
		chr > ' ' && chr < 0x7F ? chr : ' ') ;
#endif

    prom_putc(chr) ;	/* this routine will wait */
     return 1;

} /* putDebugChar */

/* Just a NULL routine for testing. */
void gdb_null(void)
{
}

void rs_kgdb_hook(int tty_no)
{
    printk("rs_kgdb_hook: tty %d\n", tty_no);

    /* Call GDB routine to setup the exception vectors for the debugger */
   set_debug_traps();

   printk("Breaking into debugger...\n");
   breakpoint();
   gdb_null() ;
   printk("Connected.\n");

   gdb_initialized = 1;

#ifdef CONFIG_GDB_CONSOLE
    register_gdb_console();
#endif
}

void kgdb_hook_irq()
{
    int         retval ;
    uint16 uMask;

    printk("GDB: Hooking UART interrupt\n");

    retval = request_irq(INTERRUPT_ID_UART,
                         gdb_interrupt,
                         SA_INTERRUPT,
                         "GDB-stub", NULL);

    if (retval != 0)
	printk("gdb_hook: request_irq(irq=%d) failed: %d\n", INTERRUPT_ID_UART, retval);

      // Enable UART config Rx not empty IRQ
     uMask = READ16(stUart->intMask) ;
      //     printk("intMask: 0x%x\n", uMask);
     WRITE16(stUart->intMask, uMask | RXFIFONE);
}


