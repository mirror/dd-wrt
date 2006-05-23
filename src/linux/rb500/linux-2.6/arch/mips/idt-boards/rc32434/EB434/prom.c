/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     prom interface routines
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
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/console.h>
#include <asm/bootinfo.h>
#include <linux/bootmem.h>
#include <linux/ioport.h>
#include <linux/serial.h>
#include <linux/serialP.h>
#include <asm/serial.h>
#include <linux/ioport.h>


unsigned int idt_cpu_freq = CONFIG_IDT_BOARD_FREQ;
EXPORT_SYMBOL(idt_cpu_freq);

extern void setup_serial_port(void);
#ifdef CONFIG_IDT_BOOT_NVRAM
extern void mapenv(int (*func)(char *, char *));
static int make_bootparm(char *name,char *val)
{ 
/*
 * The bootparameters are obtained from NVRAM and formatted here.
 * For e.g.
 *
 *    netaddr=10.0.1.95
 *    bootaddr=10.0.0.139
 *    bootfile=vmlinus
 *    bootparm1=root=/dev/nfs
 *    bootparm2=ip=10.0.1.95
 *
 * is parsed to:
 *
 *      root=/dev/nfs ip=10.0.1.95
 *
 * in arcs_cmdline[].
 */
	if (strncmp(name, "bootparm", 8) == 0) {
		strcat(arcs_cmdline,val);
		strcat(arcs_cmdline," ");
	}
	else if(strncmp(name, "HZ", 2) == 0) {
		idt_cpu_freq = simple_strtoul(val, 0, 10);
		printk("CPU Clock at %d Hz (from HZ environment variable)\n",
		       idt_cpu_freq);
	}
	return 0;
}

static void prom_init_cmdline(void)
{ 
	memset(arcs_cmdline,0,sizeof(arcs_cmdline));
	mapenv(&make_bootparm);
}
#else
/* Kernel Boot parameters */
//static unsigned char bootparm[]="ip=157.165.29.36:157.165.29.18::255.255.0.0::eth0";
static unsigned char bootparm[]="console=ttyS0,9600";
#endif
extern unsigned long mips_machgroup;
extern unsigned long mips_machtype;

/* IDT 79EB434 memory map -- we really should be auto sizing it */

#define RAM_FIRST       0x80000400  /* Leave room for interrupt vectors */
#define RAM_SIZE        32*1024*1024
#define RAM_END         (0x80000000 + RAM_SIZE)     
struct resource rc32434_res_ram = {
	"RAM",
	0,
	RAM_SIZE,
	IORESOURCE_MEM
};

char * __init prom_getcmdline(void)
{ 
	return &(arcs_cmdline[0]);
}


void __init prom_init(void)
{
#ifdef CONFIG_IDT_BOOT_NVRAM
	/* set up command line */
	prom_init_cmdline();
#else
	sprintf(arcs_cmdline,"%s",bootparm);
#endif
	
	/* turn on the console */
	
	setup_serial_port();
	/* set our arch type */
	
	mips_machgroup = MACH_GROUP_IDT;
	mips_machtype = MACH_IDT_EB434;
	
	/*
	 * give all RAM to boot allocator,
	 * except where the kernel was loaded
	 */
	add_memory_region(0,
			  rc32434_res_ram.end - rc32434_res_ram.start,
			  BOOT_MEM_RAM);
}

void prom_free_prom_memory(void)
{
	printk("stubbed prom_free_prom_memory()\n");
}
