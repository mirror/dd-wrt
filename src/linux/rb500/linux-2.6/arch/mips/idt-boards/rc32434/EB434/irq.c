/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     Interrupt routines for IDT EB434 boards
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

#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel_stat.h>
#include <linux/module.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/timex.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/delay.h>

#include <asm/bitops.h>
#include <asm/bootinfo.h>
#include <asm/io.h>
#include <asm/mipsregs.h>
#include <asm/system.h>
#include <asm/idt-boards/rc32434/rc32434.h>
#include <asm/idt-boards/rc32434/rc32434_gpio.h>

#include <asm/irq.h>

#undef DEBUG_IRQ
#ifdef DEBUG_IRQ
/* note: prints function name for you */
#define DPRINTK(fmt, args...) printk("%s: " fmt, __FUNCTION__ , ## args)
#else
#define DPRINTK(fmt, args...)
#endif

extern asmlinkage void idtIRQ(void);
static unsigned int startup_irq(unsigned int irq);
static void end_irq(unsigned int irq_nr);
static void mask_and_ack_irq(unsigned int irq_nr);
static void rc32434_enable_irq(unsigned int irq_nr);
static void rc32434_disable_irq(unsigned int irq_nr);

extern void __init init_generic_irq(void);

typedef struct {
	u32 mask;
	volatile u32 *base_addr;
} intr_group_t;

static const intr_group_t intr_group[NUM_INTR_GROUPS] = {
	{ 0x0000efff, (u32 *)KSEG1ADDR(IC_GROUP0_PEND + 0 * IC_GROUP_OFFSET) },
	{ 0x00001fff, (u32 *)KSEG1ADDR(IC_GROUP0_PEND + 1 * IC_GROUP_OFFSET) },
	{ 0x00000007, (u32 *)KSEG1ADDR(IC_GROUP0_PEND + 2 * IC_GROUP_OFFSET) },
	{ 0x0003ffff, (u32 *)KSEG1ADDR(IC_GROUP0_PEND + 3 * IC_GROUP_OFFSET) },
	{ 0xffffffff, (u32 *)KSEG1ADDR(IC_GROUP0_PEND + 4 * IC_GROUP_OFFSET) }
};

#define READ_PEND(base) (*(base))
#define READ_MASK(base) (*(base + 2))
#define WRITE_MASK(base, val) (*(base + 2) = (val))

static inline int irq_to_group(unsigned int irq_nr)
{
	return ((irq_nr - GROUP0_IRQ_BASE) >> 5);
}

static inline int group_to_ip(unsigned int group)
{
	return group + 2;
}

static inline void enable_local_irq(unsigned int ip)
{
	int ipnum = 0x100 << ip;
	clear_c0_cause(ipnum);
	set_c0_status(ipnum);
}

static inline void disable_local_irq(unsigned int ip)
{
	int ipnum = 0x100 << ip;
	clear_c0_status(ipnum);
}

static inline void ack_local_irq(unsigned int ip)
{
	int ipnum = 0x100 << ip;
	clear_c0_cause(ipnum);
}

static void rc32434_enable_irq(unsigned int irq_nr)
{
	int           ip = irq_nr - GROUP0_IRQ_BASE;
	unsigned int  group, intr_bit;
	volatile unsigned int  *addr;
	if (ip < 0) {
		enable_local_irq(irq_nr);
	}
	else {
		// calculate group
		group = ip >> 5;
		
		// calc interrupt bit within group
		ip -= (group << 5);
		intr_bit = 1 << ip;
		
		// first enable the IP mapped to this IRQ
		enable_local_irq(group_to_ip(group));
		
		addr = intr_group[group].base_addr;
		// unmask intr within group
		WRITE_MASK(addr, READ_MASK(addr) & ~intr_bit);
	}
}

static void rc32434_disable_irq(unsigned int irq_nr)
{
	int           ip = irq_nr - GROUP0_IRQ_BASE;
	unsigned int  group, intr_bit, mask;
	volatile unsigned int  *addr;
	
	// calculate group
	group = ip >> 5;
	
	// calc interrupt bit within group
	ip -= group << 5;
	intr_bit = 1 << ip;
	
	addr = intr_group[group].base_addr;
	// mask intr within group
	mask = READ_MASK(addr);
	mask |= intr_bit;
	WRITE_MASK(addr, mask);
	
	/*
	  if there are no more interrupts enabled in this
	  group, disable corresponding IP
	*/
	if (mask == intr_group[group].mask)
		disable_local_irq(group_to_ip(group));
}

static unsigned int startup_irq(unsigned int irq_nr)
{
	rc32434_enable_irq(irq_nr);
	return 0; 
}

static void shutdown_irq(unsigned int irq_nr)
{
	rc32434_disable_irq(irq_nr);
	return;
}

static void mask_and_ack_irq(unsigned int irq_nr)
{
	rc32434_disable_irq(irq_nr);
	ack_local_irq(group_to_ip(irq_to_group(irq_nr)));
}

static void end_irq(unsigned int irq_nr)
{
	
	int ip = irq_nr - GROUP0_IRQ_BASE;
	unsigned int intr_bit, group;
	volatile unsigned int *addr;
	
	if (!(irq_desc[irq_nr].status & (IRQ_DISABLED | IRQ_INPROGRESS))) {
		if (irq_nr == GROUP4_IRQ_BASE + 11)
			gpio->gpioistat = 0xfffff7ff;
		
		group = ip >> 5;
		
		// calc interrupt bit within group
		ip -= (group << 5);
		intr_bit = 1 << ip;
		
		// first enable the IP mapped to this IRQ
		enable_local_irq(group_to_ip(group));
		
		addr = intr_group[group].base_addr;
		// unmask intr within group
		WRITE_MASK(addr, READ_MASK(addr) & ~intr_bit);
	} 
	else {
		printk("warning: end_irq %d did not enable (%x)\n", 
		       irq_nr, irq_desc[irq_nr].status);
	}
}

static struct hw_interrupt_type rc32434_irq_type = {
	.typename = "IDT434",
	.startup  = startup_irq,
	.shutdown = shutdown_irq,
	.enable   = rc32434_enable_irq,
	.disable  = rc32434_disable_irq,
	.ack      = mask_and_ack_irq,
	.end      = end_irq,
};

void __init arch_init_irq(void)
{
	int i;
	printk("Initializing IRQ's: %d out of %d\n", RC32434_NR_IRQS, NR_IRQS);  
	memset(irq_desc, 0, sizeof(irq_desc));
	set_except_vector(0, idtIRQ);
	
	for (i = 0; i < RC32434_NR_IRQS; i++) {
		irq_desc[i].status = IRQ_DISABLED;
		irq_desc[i].action = NULL;
		irq_desc[i].depth = 1;
		irq_desc[i].handler = &rc32434_irq_type;
		spin_lock_init(&irq_desc[i].lock);
	}
}

/* Main Interrupt dispatcher */
void rc32434_irqdispatch(unsigned long cp0_cause, struct pt_regs *regs)
{
	unsigned int ip, pend, group;
	volatile unsigned int *addr;
	
	if ((ip = (cp0_cause & 0x7c00))) {
		group = 21 - rc32434_clz(ip);
		
		addr = intr_group[group].base_addr;
		
		pend = READ_PEND(addr);
		pend &= ~READ_MASK(addr); // only unmasked interrupts
		pend = 39 - rc32434_clz(pend);
		do_IRQ((group << 5) + pend, regs);
		return;
	} 
	else
		return;
}
