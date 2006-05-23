/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     Serial port initialisation.
 *
 *  Copyright 2004 IDT Inc. (rischelp@idt.com)
 *         
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 **************************************************************************
 * May 2004 rkt, neb
 *
 * Initial Release
 *
 * 
 *
 **************************************************************************
 */


#include <linux/config.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/tty.h>
#include <linux/serial.h>
#include <linux/serial_core.h>

#include <asm/time.h>
#include <asm/cpu.h>
#include <asm/bootinfo.h>
#include <asm/irq.h>
#include <asm/serial.h>
#include <asm/rc32434/rc32434.h>

//extern int __init early_serial_setup(struct uart_port *port);
#if 0
#ifdef __MIPSEB__
#define EARLY_SERIAL_OPTS "console=uart,mmio,0xb8058003,115200n8"
#else
#define EARLY_SERIAL_OPTS "console=uart,mmio,0xb8058000,115200n8"
#endif
#endif

#define BASE_BAUD (1843200 / 16)

extern unsigned int idt_cpu_freq;
extern int __init setup_serial_port(void)
{
	static struct uart_port serial_req[2];
	
	/*cons_puts("Serial setup!\n");*/
	
	memset(serial_req, 0, sizeof(serial_req));
	serial_req[0].type       = PORT_16550A;
	serial_req[0].line       = 0;
	serial_req[0].irq        = RC32434_UART0_IRQ;
	serial_req[0].flags      = STD_COM_FLAGS;
	serial_req[0].uartclk    = idt_cpu_freq;
// 	serial_req[0].uartclk    = 24000000;
	serial_req[0].iotype     = UPIO_MEM;
	serial_req[0].membase    = (char *) KSEG1ADDR(RC32434_UART0_BASE);
	// serial_req[0].fifosize   = 14;
	serial_req[0].mapbase   = KSEG1ADDR(RC32434_UART0_BASE);
	serial_req[0].regshift   = 2;
	
#if 0 /*RB532 only uses the first serial */
	serial_req[1].type       = PORT_16550A;
	serial_req[1].line       = 1;
	serial_req[1].irq        = EB434_UART1_IRQ;
	serial_req[1].flags      = STD_COM_FLAGS;
	serial_req[1].uartclk    = idt_cpu_freq;
	serial_req[1].iotype     = SERIAL_IO_MEM;
	serial_req[1].membase    = (char *)KSEG1ADDR(EB434_UART1_BASE);
	//  serial_req[1].fifosize   = 14;
	serial_req[1].regshift   = 2;
	serial_req[1].mapbase   = KSEG1ADDR(EB434_UART1_BASE);
#endif

	if (early_serial_setup(&serial_req[0])){
		cons_puts("Serial setup failed!\n");
		return -ENODEV;
	}
	
	/*cons_puts("Serial setup OK.\n");*/
  return(0);
}
