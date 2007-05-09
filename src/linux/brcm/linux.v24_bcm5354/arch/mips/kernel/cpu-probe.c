#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/stddef.h>
#include <asm/bugs.h>
#include <asm/cpu.h>
#include <asm/fpu.h>
#include <asm/mipsregs.h>

/*
 * Not all of the MIPS CPUs have the "wait" instruction available. Moreover,
 * the implementation of the "wait" feature differs between CPU families. This
 * points to the function that implements CPU specific wait.
 * The wait instruction stops the pipeline and reduces the power consumption of
 * the CPU very much.
 */
void (*cpu_wait)(void) = NULL;

static void r3081_wait(void)
{
	unsigned long cfg = read_c0_conf();
	write_c0_conf(cfg | R30XX_CONF_HALT);
}

static void r39xx_wait(void)
{
	unsigned long cfg = read_c0_conf();
	write_c0_conf(cfg | TX39_CONF_HALT);
}

static void r4k_wait(void)
{
	__asm__(".set\tmips3\n\t"
		"wait\n\t"
		".set\tmips0");
}

/* The Au1xxx wait is available only if using 32khz counter or
 * external timer source, but specifically not CP0 Counter. */
int allow_au1k_wait; 
static void au1k_wait(void)
{
	unsigned long addr = 0;
	/* using the wait instruction makes CP0 counter unusable */
	__asm__("la %0,au1k_wait\n\t"
		".set mips3\n\t"
		"cache 0x14,0(%0)\n\t"
		"cache 0x14,32(%0)\n\t"
		"sync\n\t"
		"nop\n\t"
		"wait\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		".set mips0\n\t"
		: : "r" (addr));
}

static inline void check_wait(void)
{
	struct cpuinfo_mips *c = &current_cpu_data;

	printk("Checking for 'wait' instruction... ");
	switch (c->cputype) {
	case CPU_R3081:
	case CPU_R3081E:
		cpu_wait = r3081_wait;
		printk(" available.\n");
		break;
	case CPU_TX3927:
		cpu_wait = r39xx_wait;
		printk(" available.\n");
		break;
	case CPU_R4200:
/*	case CPU_R4300: */
	case CPU_R4600:
	case CPU_R4640:
	case CPU_R4650:
	case CPU_R4700:
	case CPU_R5000:
	case CPU_NEVADA:
	case CPU_RM7000:
	case CPU_RM9000:
	case CPU_TX49XX:
	case CPU_4KC:
	case CPU_4KEC:
	case CPU_4KSC:
	case CPU_5KC:
/*	case CPU_20KC:*/
	case CPU_24K:
	case CPU_25KF:
		cpu_wait = r4k_wait;
		printk(" available.\n");
		break;
	case CPU_AU1000:
	case CPU_AU1100:
	case CPU_AU1500:
	case CPU_AU1550:
	case CPU_AU1200:
		if (allow_au1k_wait) {
			cpu_wait = au1k_wait;
			printk(" available.\n");
		} else
			printk(" unavailable.\n");
		break;
	default:
		printk(" unavailable.\n");
		break;
	}
}

void __init check_bugs(void)
{
	check_wait();
}

/*
 * Probe whether cpu has config register by trying to play with
 * alternate cache bit and see whether it matters.
 * It's used by cpu_probe to distinguish between R3000A and R3081.
 */
static inline int cpu_has_confreg(void)
{
#ifdef CONFIG_CPU_R3000
	extern unsigned long r3k_cache_size(unsigned long);
	unsigned long size1, size2;
	unsigned long cfg = read_c0_conf();

	size1 = r3k_cache_size(ST0_ISC);
	write_c0_conf(cfg ^ R30XX_CONF_AC);
	size2 = r3k_cache_size(ST0_ISC);
	write_c0_conf(cfg);
	return size1 != size2;
#else
	return 0;
#endif
}

/*
 * Get the FPU Implementation/Revision.
 */
static inline unsigned long cpu_get_fpu_id(void)
{
	unsigned long tmp, fpu_id;

	tmp = read_c0_status();
	__enable_fpu();
	fpu_id = read_32bit_cp1_register(CP1_REVISION);
	write_c0_status(tmp);
	return fpu_id;
}

/*
 * Check the CPU has an FPU the official way.
 */
static inline int __cpu_has_fpu(void)
{
	return ((cpu_get_fpu_id() & 0xff00) != FPIR_IMP_NONE);
}

#define R4K_OPTS (MIPS_CPU_TLB | MIPS_CPU_4KEX | MIPS_CPU_4KTLB \
		| MIPS_CPU_COUNTER)


static inline void decode_config1(struct cpuinfo_mips *c)
{
	unsigned long config0 = read_c0_config();
	unsigned long config1;

	if ((config0 & (1 << 31)) == 0)
		return;			/* actually wort a panic() */

	/* MIPS32 or MIPS64 compliant CPU. Read Config 1 register. */
	c->options = MIPS_CPU_TLB | MIPS_CPU_4KEX |
		MIPS_CPU_4KTLB | MIPS_CPU_COUNTER | MIPS_CPU_DIVEC |
		MIPS_CPU_LLSC;
	config1 = read_c0_config1();
	if (config1 & (1 << 3))
		c->options |= MIPS_CPU_WATCH;
	if (config1 & (1 << 2))
		c->options |= MIPS_CPU_MIPS16;
	if (config1 & (1 << 1))
		c->options |= MIPS_CPU_EJTAG;
	if (config1 & 1) {
		c->options |= MIPS_CPU_FPU;
		c->options |= MIPS_CPU_32FPR;
	}
	c->scache.flags = MIPS_CACHE_NOT_PRESENT;

	c->tlbsize = ((config1 >> 25) & 0x3f) + 1;
}


static inline void cpu_probe_broadcom(struct cpuinfo_mips *c)
{
	decode_config1(c);
	c->options |= MIPS_CPU_PREFETCH;
	switch (c->processor_id & PRID_IMP_MASK) {
	case PRID_IMP_BCM4710:
			c->cputype = CPU_BCM4710;
			c->options = MIPS_CPU_TLB | MIPS_CPU_4KEX | 
								MIPS_CPU_4KTLB | MIPS_CPU_COUNTER;
			c->scache.flags = MIPS_CACHE_NOT_PRESENT;
			break;
	case PRID_IMP_4KC:              
	case PRID_IMP_BCM3302:          
			c->cputype = CPU_BCM3302;
			c->options = MIPS_CPU_TLB | MIPS_CPU_4KEX | 
								MIPS_CPU_4KTLB | MIPS_CPU_COUNTER;
			c->scache.flags = MIPS_CACHE_NOT_PRESENT;
			break;
	default:
			c->cputype = CPU_UNKNOWN;
			break;
	}
}
__init void cpu_probe(void)
{
	struct cpuinfo_mips *c = &current_cpu_data;

	c->processor_id	= PRID_IMP_UNKNOWN;
	c->fpu_id	= FPIR_IMP_NONE;
	c->cputype	= CPU_UNKNOWN;

	c->processor_id = read_c0_prid();
	cpu_probe_broadcom(c);
	if (c->options & MIPS_CPU_FPU)
		c->fpu_id = cpu_get_fpu_id();
}

__init void cpu_report(void)
{
	struct cpuinfo_mips *c = &current_cpu_data;

	printk("CPU revision is: %08x\n", c->processor_id);
	if (c->options & MIPS_CPU_FPU)
		printk("FPU revision is: %08x\n", c->fpu_id);
}
