/*
 * setup.c - boot time setup code
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
//#include <linux/mc146818rtc.h>	/* for rtc_ops, we fake the RTC */
#include <asm/reboot.h>
#include <asm/addrspace.h>     /* for KSEG1ADDR() */
#include <asm/rc32434/rc32434.h>
#include <linux/pm.h>
//#include <asm/rc32434/pci.h> 
#include <asm/idt-boards/rc32434/rc32434_pci.h>
//#include <asm/rc32434/pcikorina.h> 

//extern char * __init prom_getcmdline(void);

extern void (*board_time_init)(void);
extern void (*board_timer_setup)(struct irqaction *irq);
extern void rc32434_time_init(void);
extern void rc32434_timer_setup(struct irqaction *irq);
#ifdef CONFIG_PCI
extern int __init rc32434_pcibridge_init(void);
#endif

#define epldMask ((volatile unsigned char *)0xB900000d)

static void rb_machine_restart(char *command)
{
	printk("rb_machine_restart: command=%s\n", command);

	/* just jump to the reset vector */

	* (volatile unsigned *) KSEG1ADDR(0x18008000) = 0x80000001;
	
	((void (*)(void))KSEG1ADDR(0x1FC00000u))();
}

static void rb_machine_halt(void)
{
	printk("rb_machine_halt:  halted\n");
	for(;;) continue;
}

static void rb_machine_power_off(void)
{
	printk("rb_machine_power_off:  It is now safe to turn off the power\n");
	for(;;) continue;
}

#ifdef CONFIG_CPU_HAS_WB
void (*__wbflush) (void);

static void rb_write_buffer_flush(void)
{
	__asm__ __volatile__
	    ("sync\n\t" "nop\n\t" "loop: bc0f loop\n\t" "nop\n\t");
}
#endif

static int __init rb_setup(void)
{
//	char* argptr;
	unsigned int pciCntlVal;
	board_time_init = rc32434_time_init;

	board_timer_setup = rc32434_timer_setup;

#ifdef CONFIG_CPU_HAS_WB
	__wbflush = rb_write_buffer_flush;
#endif
	_machine_restart = rb_machine_restart;
	_machine_halt = rb_machine_halt;
	/*_machine_power_off = rb_machine_power_off;*/
	pm_power_off = rb_machine_power_off;
	set_io_port_base(KSEG1);
	
	pciCntlVal=rc32434_pci->pcic;
	pciCntlVal &= 0xFFFFFF7;
	rc32434_pci->pcic = pciCntlVal;
	
	/* Enable PCI interrupts in EPLD Mask register */	
	*epldMask = 0x0;		
	*(epldMask + 1) = 0x0;		

	write_c0_wired(0);

	return 0;
}
//arch_initcall(rb_setup);

// not needed..
// int page_is_ram(unsigned long pagenr)
// {
// 	return 1;
// }

void __init plat_setup(void)
{
	rb_setup();
 //	rc32434_pcibridge_init();

}
const char *get_system_type(void)
{
	return "MIPS RB500";
}


/*eof */
