/*
* ARM A9 MPCORE Platform base
*/


#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/errno.h>
#include <linux/smp.h>
#include <linux/clockchips.h>
#include <linux/ioport.h>
#include <linux/cpumask.h>

#include <asm/mach/map.h>
#include <asm/smp_twd.h>
#include <asm/pgtable.h>
#include <linux/irqchip/arm-gic.h>

#include <plat/mpcore.h>
#include <mach/io_map.h>

/* Globals */
static void __iomem * periphbase ;
extern void __iomem * twd_base;	/* declared in arch/arm/kernel/smp_twd.c */

extern int __init twd_local_timer_common_register(void);
void __iomem * scu_base_addr(void)
{
	return (periphbase + MPCORE_SCU_OFF);
}

/*
 * Local per-CPU timer support 
 * Called from arch/arm/kernel/smp.c
 * Implemented in arch/arm/kernel/smp_twd.c
 * All that is needed is to set the base address in mpcore_init() and irq here.
 */
/*void __cpuinit local_timer_setup(struct clock_event_device *evt)
{
        unsigned int cpu = smp_processor_id();

	printk(KERN_INFO "MPCORE Private timer setup CPU%u\n", cpu);

	evt->retries = 0;
	evt->irq = MPCORE_IRQ_LOCALTIMER;
	twd_timer_setup(evt);
}*/

static DEFINE_TWD_LOCAL_TIMER(twd_local_timer,
			      0x19020000 + MPCORE_LTIMER_OFF,
			      MPCORE_IRQ_LOCALTIMER);

void __init mpcore_map_io( void )
{
	struct map_desc desc ;
	phys_addr_t base_addr;
	printk(KERN_INFO "map io\n");
	/* 
	 * Cortex A9 Architecture Manual specifies this as a way to get
	 * MPCORE PERHIPHBASE address at run-time
	 */
	asm( "mrc p15,4,%0,c15,c0,0 @ Read Configuration Base Address Register" 
		: "=&r" (base_addr) : : "cc" );

	printk(KERN_INFO "MPCORE found at %p (VIRT %p) %p %p\n", (void *)base_addr, phys_to_virt(base_addr),IO_BASE_PA,phys_to_virt(IO_BASE_PA)); 
	


	/* Fix-map the entire PERIPHBASE 2*4K register block */
	desc.virtual = MPCORE_BASE_VA;
	desc.pfn = __phys_to_pfn( base_addr );
	desc.length = SZ_8K;
	desc.type = MT_DEVICE ;

	iotable_init( &desc, 1);

	periphbase = (void *) MPCORE_BASE_VA;



	/* Local timer code needs just the register base address */
//	twd_base = periphbase + MPCORE_LTIMER_OFF;
//	twd_local_timer_common_register();
}

void __init mpcore_init_gic( void )
{
	void __iomem *omap_irq_base;
	void __iomem *gic_dist_base_addr;
	printk(KERN_INFO "MPCORE GIC init\n");

	gic_init(0, 1, periphbase + MPCORE_GIC_DIST_OFF, periphbase + MPCORE_GIC_CPUIF_OFF);

#if 0
	/* Init GIC interrupt distributor */
	gic_dist_init( 0, periphbase + MPCORE_GIC_DIST_OFF, 1 );

	/* Initialize the GIC CPU interface for the boot processor */
	gic_cpu_init( 0, periphbase + MPCORE_GIC_CPUIF_OFF );
#endif
}

void __init mpcore_init_timer( unsigned long perphclk_freq )
{

	/* Init Global Timer */
	mpcore_gtimer_init( periphbase + MPCORE_GTIMER_OFF, 
			perphclk_freq, MPCORE_IRQ_GLOBALTIMER );

#ifdef CONFIG_LOCAL_TIMERS
	printk(KERN_INFO "register local timer\n");
	int err = twd_local_timer_register(&twd_local_timer);
	if (err)
		pr_err("twd_local_timer_register failed %d\n", err);
#endif

}


/*
 * For SMP - initialize GIC CPU interface for secondary cores
 */
void __cpuinit mpcore_cpu_init(void)
{
	/* Initialize the GIC CPU interface for the next processor */
//	gic_secondary_init(0);
}
