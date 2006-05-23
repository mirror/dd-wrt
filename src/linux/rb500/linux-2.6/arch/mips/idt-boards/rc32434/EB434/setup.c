/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     setup routines for IDT EB434 boards
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

#include <linux/init.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/irq.h>
#include <asm/bootinfo.h>
#include <asm/io.h>
#include <linux/ioport.h>
#include <asm/mipsregs.h>
#include <asm/pgtable.h>
#include <asm/reboot.h>
#include <asm/addrspace.h>     /* for KSEG1ADDR() */
#include <asm/idt-boards/rc32434/rc32434.h>

extern char * __init prom_getcmdline(void);

extern void (*board_time_init)(void);
extern void (*board_timer_setup)(struct irqaction *irq);
extern void rc32434_time_init(void);
extern void rc32434_timer_setup(struct irqaction *irq);
extern void idt_reset(void);
void idt_disp_str(char *s);

#define epldMask ((volatile unsigned char *)0xB900000d)

#define DIG_CLEAR ((volatile unsigned char *)0xB9000000)
#define DIG0 ((volatile unsigned char *)0xB9000007)
#define DIG1 ((volatile unsigned char *)0xB9000006)
#define DIG2 ((volatile unsigned char *)0xB9000005)
#define DIG3 ((volatile unsigned char *)0xB9000004)

void idt_disp_char(int i, char c)
{
	switch(i) {
	case 0: *DIG0 = c; break;
	case 1: *DIG1 = c; break;
	case 2: *DIG2 = c; break;
	case 3: *DIG3 = c; break;
	default: *DIG0 = '?'; break;
	}
}

void idt_disp_str(char *s)
{
	if (s == 0) {
		*DIG_CLEAR;
	} else {
		int i;
		for(i = 0; i < 4; i++) {
			if(s[i]) idt_disp_char(i, s[i]);
		}
	}
}


static void idt_machine_restart(char *command)
{
	printk("idt_machine_restart: command=%s\n", command);
	idt_reset();
}

static void idt_machine_halt(void)
{
	printk("idt_machine_halt:  halted\n");
	for(;;) continue;
}

static void idt_machine_power_off(void)
{
	printk("idt_machine_power_off:  It is now safe to turn off the power\n");
	for(;;) continue;
}


static int __init idt_setup(void)
{
	char* argptr;
	
	idt_disp_str("Unix");
	
	argptr = prom_getcmdline();
#ifdef CONFIG_SERIAL_CONSOLE
	if ((argptr = strstr(argptr, "console=")) == NULL) {
		argptr = prom_getcmdline();
		strcat(argptr, " console=ttyS0,9600");
	}
#endif
	
	board_time_init = rc32434_time_init;
	
	board_timer_setup = rc32434_timer_setup;
	
	_machine_restart = idt_machine_restart;
	_machine_halt = idt_machine_halt;
	_machine_power_off = idt_machine_power_off;
	set_io_port_base(KSEG1);
	/* Enable PCI interrupts in EPLD Mask register */
	*epldMask = 0x0;
	*(epldMask + 1) = 0x0;
	
	write_c0_wired(0);
	
	return 0;
	
}
early_initcall(idt_setup);

int page_is_ram(unsigned long pagenr)
{
	return 1;
}

const char *get_system_type(void)
{
	return "MIPS IDT32434";
}

