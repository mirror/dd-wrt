/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <asm/system.h>
#include <asm/leds.h>
#include <asm/mach-types.h>

#define LSP_CNTMR 0
#define TICKS2USECS(x) ((x)/(mvTclk/1000000))
 
extern  u32 mvTclk;
/* redefine from mvCntmr */
struct _mvCntmrCtrl
{
        u32 enable;         /* enable */
        u32 autoEnable;     /* counter/Timer */
};

extern u32 mvCntmrStart(u32, u32, struct _mvCntmrCtrl *);
extern u32 mvCntmrRead(u32);

/*
 * Returns number of usec since last clock interrupt.  Note that interrupts
 * will have been disabled by do_gettimeoffset()
 */
static unsigned long mv_gettimeoffset(void)
{
	unsigned int ticks;
	volatile u32 cause;
	ticks = (mvTclk / HZ) - mvCntmrRead(LSP_CNTMR);
	cause  = *(volatile u32*)(INTER_REGS_BASE + BRIDGE_INT_CAUSE_REG);	
	/* check if we got an interrupt meanwhile */
	cause = MV_ARM_32BIT_LE(cause);
	if (cause & TIMER_BIT_MASK(LSP_CNTMR))
	{
		ticks += (mvTclk / HZ);
	}
	return TICKS2USECS(ticks);			
}


/*
 * IRQ handler for the timer
 */
u32	led_counter = 0;
u32	led_val = 0;
extern void mvBoardDebug7Seg(u32);

static irqreturn_t
mv_timer_interrupt(int irq, void *dev_id)
{
	volatile u32 cause = *(volatile u32*)(INTER_REGS_BASE + BRIDGE_INT_CAUSE_REG);
	volatile u32 mask = *(volatile u32*)(INTER_REGS_BASE + BRIDGE_INT_MASK_REG); 
	cause = MV_ARM_32BIT_LE(cause);
	mask = MV_ARM_32BIT_LE(mask);
	
	/* check if we realy received a timer irq. */
	if( ((cause & mask) & TIMER_BIT_MASK(LSP_CNTMR)) != 0 )
	{
		/* clear the timer irq */
		cause = (cause & ~(TIMER_BIT_MASK(LSP_CNTMR)));		
		*(volatile u32*)(INTER_REGS_BASE + BRIDGE_INT_CAUSE_REG) = MV_ARM_32BIT_LE(cause);		
//		if(led_counter++ > HZ)
//		{
//			led_counter = 0;		
//			mvBoardDebug7Seg(led_val);
//			led_val++;
//			if(led_val == 0xa)
//				led_val =0;
//		}	
		timer_tick();
	}
	else /* no support for other irqs. */
	{
		printk("Error IRQ not supported ??\n");
		return IRQ_NONE;
	}
	return IRQ_HANDLED;
}

static struct irqaction mv_timer_irq = {
        .name           = "Mv Timer Tick",
        .flags          = IRQF_DISABLED,
        .handler        = mv_timer_interrupt
};
 
/*
 * Set up timer interrupt.
 */
void __init mv_time_init(void)
{
	struct _mvCntmrCtrl cntmr;
	u32 timer_reload;
	u32 timer;

        cntmr.enable = 1;
        cntmr.autoEnable = 1;

	timer_reload = mvTclk / HZ;
        mvCntmrStart(LSP_CNTMR, timer_reload, &cntmr);

	mv_timer_irq.handler = mv_timer_interrupt;

	/* enable only the LSP timer interrupt */
	timer = *(volatile u32*)(INTER_REGS_BASE + MV_AHBTOMBUS_IRQ_CAUSE_REG);	
	timer = MV_ARM_32BIT_LE(timer);
	timer |= TIMER_BIT_MASK(LSP_CNTMR);

	*(volatile u32*)(INTER_REGS_BASE + MV_AHBTOMBUS_IRQ_CAUSE_REG) = MV_ARM_32BIT_LE(timer);
	setup_irq(TIME_IRQ, &mv_timer_irq);
//	gettimeoffset = mv_gettimeoffset;

	return;
}
