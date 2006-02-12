/* 
 * Machine check handler.
 * K8 parts Copyright 2002,2003 Andi Kleen, SuSE Labs.
 * Additional K8 decoding and simplification Copyright 2003 Eric Morton, Newisys Inc
 * Rest from unknown author(s).
 */
#include <linux/config.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/pci.h>
#include <linux/timer.h>
#include <linux/notifier.h>
#include <asm/bluesmoke.h>
#include <asm/processor.h> 
#include <asm/msr.h>
#include <asm/kdebug.h>
#include <asm/smp.h>

static int mce_disabled __initdata;
static unsigned long mce_cpus; 

/*
 *	Machine Check Handler For PII/PIII/K7
 */

static int banks;
static unsigned long ignored_banks, disabled_banks;

struct notifier_block *mc_notifier_list = NULL;
EXPORT_SYMBOL(mc_notifier_list);

static void generic_machine_check(struct pt_regs * regs, long error_code)
{
	int recover=1;
	u32 alow, ahigh, high, low;
	u32 mcgstl, mcgsth;
	int i;
	struct notifier_mc_err mc_err;

	rdmsr(MSR_IA32_MCG_STATUS, mcgstl, mcgsth);
	if(mcgstl&(1<<0))	/* Recoverable ? */
		recover=0;

	/* Make sure unrecoverable MCEs reach the console */
	if(recover & 3)
		oops_in_progress++;

	printk(KERN_EMERG "CPU %d: Machine Check Exception: %08x%08x\n", 
				smp_processor_id(), mcgsth, mcgstl);
	
	if (regs && (mcgstl & 2))
		printk(KERN_EMERG "RIP <%02lx>:%016lx RSP %016lx\n", 
		       regs->cs, regs->rip, regs->rsp); 

	for(i=0;i<banks;i++)
	{
		if ((1UL<<i) & ignored_banks) 
			continue; 

		rdmsr(MSR_IA32_MC0_STATUS+i*4,low, high);
		if(high&(1<<31))
		{
			memset(&mc_err, 0x00, sizeof(mc_err));
			mc_err.cpunum = safe_smp_processor_id();
			mc_err.banknum = i;
			mc_err.mci_status = ((u64)high << 32) | low;
			if(high&(1<<29))
				recover|=1;
			if(high&(1<<25))
				recover|=2;
			printk(KERN_EMERG "Bank %d: %08x%08x", i, high, low);
			high&=~(1<<31);
			if(high&(1<<27))
			{
				rdmsr(MSR_IA32_MC0_MISC+i*4, alow, ahigh);
				mc_err.mci_misc = ((u64)ahigh << 32) | alow;
				printk("[%08x%08x]", alow, ahigh);
			}
			if(high&(1<<26))
			{
				rdmsr(MSR_IA32_MC0_ADDR+i*4, alow, ahigh);
				mc_err.mci_addr = ((u64)ahigh << 32) | alow;
				printk(" at %08x%08x",
					ahigh, alow);
			}
			rdmsr(MSR_IA32_MC0_CTL+i*4, alow, ahigh);
			mc_err.mci_ctl = ((u64)ahigh << 32) | alow;

			printk("\n");

			/* Clear it */
			wrmsr(MSR_IA32_MC0_STATUS+i*4, 0UL, 0UL);
			/* Serialize */
			wmb();
			notifier_call_chain(&mc_notifier_list, X86_VENDOR_INTEL, &mc_err);
		}
	}

	if(recover&2)
		panic("CPU context corrupt");
	if(recover&1)
		panic("Unable to continue");
	printk(KERN_EMERG "Attempting to continue.\n");
	mcgstl&=~(1<<2);
	wrmsr(MSR_IA32_MCG_STATUS,mcgstl, mcgsth);
}

static void unexpected_machine_check(struct pt_regs *regs, long error_code)
{ 
	printk("unexpected machine check %lx\n", error_code); 
} 

/*
 *	Call the installed machine check handler for this CPU setup.
 */ 
 
static void (*machine_check_vector)(struct pt_regs *, long error_code) = unexpected_machine_check;

void do_machine_check(struct pt_regs * regs, long error_code)
{
	notify_die(DIE_NMI, "machine check", regs, error_code, 255, SIGKILL);
	machine_check_vector(regs, error_code);
}

/* 
 *	K8 machine check.
 */

static char *k8bank[] = {
	"data cache",
	"instruction cache",
	"bus unit",
	"load/store unit",
	"northbridge"
};
static char *transaction[] = { 
	"instruction", "data", "generic", "reserved"
}; 
static char *cachelevel[] = { 
	"0", "1", "2", "generic"
};
static char *memtrans[] = { 
	"generic error", "generic read", "generic write", "data read",
	"data write", "instruction fetch", "prefetch", "evict", "snoop",
	"?", "?", "?", "?", "?", "?", "?"
};
static char *partproc[] = { 
	"local node origin", "local node response", 
	"local node observed", "generic participation"
};
static char *timeout[] = { 
	"request didn't time out",
	"request timed out"
};
static char *memoryio[] = { 
	"memory", "res.", "i/o", "generic"
};
static char *nbextendederr[] = { 
	"ECC error", 
	"CRC error",
	"Sync error",
	"Master abort",
	"Target abort",
	"GART error",
	"RMW error",
	"Watchdog error",
	"Chipkill ECC error", 
	"<9>","<10>","<11>","<12>",
	"<13>","<14>","<15>"
};
static char *highbits[32] = { 
	[31] = "valid",
	[30] = "error overflow (multiple errors)",
	[29] = "error uncorrected",
	[28] = "error enable",
	[27] = "misc error valid",
	[26] = "error address valid", 
	[25] = "processor context corrupt", 
	[24] = "res24",
	[23] = "res23",
	/* 22-15 ecc syndrome bits */
	[14] = "corrected ecc error",
	[13] = "uncorrected ecc error",
	[12] = "res12",
	[11] = "res11",
	[10] = "res10",
	[9] = "res9",
	[8] = "error found by scrub", 
	[7] = "res7",
	/* 6-4 ht link number of error */ 
	[3] = "res3",
	[2] = "res2",
	[1] = "err cpu1",
	[0] = "err cpu0",
};


static void decode_k8_generic_errcode(unsigned int cpunum, u64 status)
{
	unsigned short errcode = status & 0xffff;
	int i;

	for (i=0; i<32; i++) {
		if (i==31 || i==28 || i==26)
			continue;
		if (highbits[i] && (status & (1UL<<(i+32)))) {
			printk(KERN_ERR "CPU%d:   bit%d = %s\n", cpunum, i+32, highbits[i]);
		}
	}

	if ((errcode & 0xFFF0) == 0x0010) {
		printk(KERN_ERR "CPU%d: TLB error '%s transaction, level %s'\n",
		       cpunum,
		       transaction[(errcode >> 2) & 3],
		       cachelevel[errcode & 3]);
	}
	else if ((errcode & 0xFF00) == 0x0100) {
		printk(KERN_ERR "CPU%d: memory/cache error '%s mem transaction, %s transaction, level %s'\n",
		       cpunum,
		       memtrans[(errcode >> 4) & 0xf],
		       transaction[(errcode >> 2) & 3],
		       cachelevel[errcode & 3]);
	}
	else if ((errcode & 0xF800) == 0x0800) {
		printk(KERN_ERR "CPU%d: bus error '%s, %s\n      %s mem transaction\n      %s access, level %s'\n",
		       cpunum,
		       partproc[(errcode >> 9) & 0x3],
		       timeout[(errcode >> 8) & 1],
		       memtrans[(errcode >> 4) & 0xf],
		       memoryio[(errcode >> 2) & 0x3],
		       cachelevel[(errcode & 0x3)]);
	}
}

static void decode_k8_dc_mc(unsigned int cpunum, u64 status)
{
	unsigned short exterrcode = (status >> 16) & 0x0f;
	unsigned short errcode = status & 0xffff;

	if(status&(3UL<<45)) {
		printk(KERN_ERR "CPU%d: Data cache ECC error (syndrome %x)",
		       cpunum,
		       (u32) (status >> 47) & 0xff);
		if(status&(1UL<<40)) {
			printk(" found by scrubber");
		}
		printk("\n");
	}

	if ((errcode & 0xFFF0) == 0x0010) {
		printk(KERN_ERR "CPU%d: TLB parity error in %s array\n",
		       cpunum,
		       (exterrcode == 0) ? "physical" : "virtual");
	}

	decode_k8_generic_errcode(cpunum, status);
}

static void decode_k8_ic_mc(unsigned int cpunum, u64 status)
{
	unsigned short exterrcode = (status >> 16) & 0x0f;
	unsigned short errcode = status & 0xffff;

	if(status&(3UL<<45)) {
		printk(KERN_ERR "CPU%d: Instruction cache ECC error\n",
		       cpunum);
	}

	if ((errcode & 0xFFF0) == 0x0010) {
		printk(KERN_ERR "CPU%d: TLB parity error in %s array\n",
		       cpunum,
		       (exterrcode == 0) ? "physical" : "virtual");
	}

	decode_k8_generic_errcode(cpunum, status);
}

static void decode_k8_bu_mc(unsigned int cpunum, u64 status)
{
	unsigned short exterrcode = (status >> 16) & 0x0f;

	if(status&(3UL<<45)) {
		printk(KERN_ERR "CPU%d: L2 cache ECC error\n",
		       cpunum);
	}

	printk(KERN_ERR "CPU%d: %s array error\n",
	       cpunum,
	       (exterrcode == 0) ? "Bus or cache" : "Cache tag");

	decode_k8_generic_errcode(cpunum, status);
}

static void decode_k8_ls_mc(unsigned int cpunum, u64 status)
{
	decode_k8_generic_errcode(cpunum, status);
}

static void decode_k8_nb_mc(unsigned int cpunum, u64 status)
{
	unsigned short exterrcode = (status >> 16) & 0x0f;

	printk(KERN_ERR "CPU%d: Northbridge %s\n", cpunum, nbextendederr[exterrcode]);

	switch (exterrcode) { 
	case 0:
		printk(KERN_ERR "CPU%d: ECC syndrome = %x\n",
		       cpunum,
		       (u32) (status >> 47) & 0xff);
		break;
	case 8:	
		printk(KERN_ERR "CPU%d: Chipkill ECC syndrome = %x\n",
		       cpunum,
	    	   (u32) ((((status >> 24) & 0xff) << 8) | ((status >> 47) & 0xff)));
		break;
	case 1: 
	case 2:
	case 3:
	case 4:
	case 6:
		printk(KERN_ERR "CPU%d: link number = %x\n",
		       cpunum,
		       (u32) (status >> 36) & 0x7);
		break;		   
	}

	decode_k8_generic_errcode(cpunum, status);
}

static void decode_k8_mc(unsigned int banknum, unsigned int cpunum, u64 status)
{
	switch(banknum) {
		case 0:
			decode_k8_dc_mc(cpunum, status);
			break;
		case 1:
			decode_k8_ic_mc(cpunum, status);
			break;
		case 2:
			decode_k8_bu_mc(cpunum, status);
			break;
		case 3:
			decode_k8_ls_mc(cpunum, status);
			break;
		case 4:
			decode_k8_nb_mc(cpunum, status);
			break;
	}
}

static void k8_poll_machine_check(void)
{
	int cpunum = safe_smp_processor_id();
	int banknum;
	u64 address, status, ctl;
	struct notifier_mc_err mc_err;

	for(banknum=0; banknum<banks; banknum++) {
		if ((1UL<<banknum) & ignored_banks)
			continue;

		rdmsrl(MSR_IA32_MC0_STATUS+banknum*4, status);
		if(status&(1UL<<63)) {
			mc_err.cpunum = cpunum;
			mc_err.banknum = banknum;
			mc_err.mci_status = status;
	        	rdmsrl(MSR_IA32_MC0_ADDR+banknum*4, address);
			mc_err.mci_addr = address;

			/* Can't write anything but zeros to status, or K8 will GPF */
			wrmsrl(MSR_IA32_MC0_STATUS+banknum*4, 0UL);
			printk(KERN_ERR "CPU%d %s polled machine check error status: %016Lx",
			       cpunum,
			       k8bank[banknum],
			       status);
			if(status&(1UL<<58)) {
				printk(" at address %016Lx", address);
			}
			printk("\n");
			rdmsrl(MSR_IA32_MC0_CTL+banknum*4, ctl);
			mc_err.mci_ctl = ctl;
			notifier_call_chain(&mc_notifier_list, X86_VENDOR_AMD, &mc_err);
			decode_k8_mc(banknum, cpunum, status);
		}
	}
}

static void k8_machine_check(struct pt_regs * regs, long error_code)
{
	int norecover=1;
	u64 addr, status, ctl, mcgst;
	int banknum;
	unsigned int cpunum = safe_smp_processor_id();
	struct notifier_mc_err mc_err;

	rdmsrl(MSR_IA32_MCG_STATUS, mcgst); 
	if(mcgst&(1UL<<0)) {	/* Recoverable ? */
		norecover=0;
	}

	/* Make sure unrecoverable MCEs reach the console */
	if(norecover)
		oops_in_progress++;

	printk(KERN_EMERG "CPU %d: Machine Check Exception: %016Lx\n", cpunum, mcgst);

	if (regs && (mcgst & (1UL<<1))) {
		printk(KERN_EMERG "CPU%d: RIP <%02lx>:%016lx RSP %016lx %s\n",
		       cpunum, regs->cs, regs->rip, regs->rsp,
			   (mcgst & 1) ? "" : "!INEXACT!");
	}

	for(banknum=0; banknum<banks; banknum++) {
		if ((1UL<<banknum) & ignored_banks)
			continue;
		rdmsrl(MSR_IA32_MC0_STATUS+banknum*4, status);
		if(status&(1UL<<63)) {
			memset(&mc_err, 0x00, sizeof(mc_err));
			mc_err.cpunum = cpunum;
			mc_err.banknum = banknum;
			mc_err.mci_status = status;
			if(status&(1UL<<61)) {
				norecover|=1; /* uncorrectable */
			}
			if(status &(1UL<<57)) {
				norecover|=2; /* processor context corrupt */
			}
			printk(KERN_EMERG "CPU%d %s error status: %016Lx",
			       cpunum,
			       k8bank[banknum],
			       status);
			if(status&(1UL<<58)) {
				rdmsrl(MSR_IA32_MC0_ADDR+banknum*4, addr);
				mc_err.mci_addr = addr;
				printk(" at address %016Lx", addr);
			} else if ((banknum==4) && (((status>>16)&0x0f)==7)) {
				/* NB watchdog, address reg has details but validity bit is not set */
				rdmsrl(MSR_IA32_MC0_ADDR+banknum*4, addr);
				mc_err.mci_addr = addr;
				printk(" error details %016Lx", addr);
			}
			printk("\n");
			rdmsrl(MSR_IA32_MC0_CTL+banknum*4, ctl);
			mc_err.mci_ctl = ctl;
			/* Clear it */
            /* Can't write anything but zeros to status, or K8 will GPF */
			wrmsrl(MSR_IA32_MC0_STATUS+banknum*4, 0UL);
			/* Serialize */
			wmb();
			notifier_call_chain(&mc_notifier_list, X86_VENDOR_AMD, &mc_err);
		}
	}

	if(norecover&2) {
		panic("CPU context corrupt");
	}
	if(norecover&1) {
		panic("Unable to continue");
	}
	printk(KERN_EMERG "Attempting to continue.\n");
	mcgst&=~(1UL<<2);
	wrmsrl(MSR_IA32_MCG_STATUS,mcgst);
}

static struct timer_list mcheck_timer;
int mcheck_interval = 60*HZ;

#ifndef CONFIG_SMP 
static void mcheck_timer_handler(unsigned long data)
{
	k8_poll_machine_check();
	mcheck_timer.expires = jiffies + mcheck_interval;
	add_timer(&mcheck_timer);
}
#else

/* SMP needs a process context trampoline because smp_call_function cannot be 
   called from interrupt context. */

static void mcheck_timer_other(void *data)
{ 
	k8_poll_machine_check();
} 

static void mcheck_timer_dist(void *data)
{ 
	smp_call_function(mcheck_timer_other,0,0,0);
	k8_poll_machine_check();
	mcheck_timer.expires = jiffies + mcheck_interval;
	add_timer(&mcheck_timer);
} 

static void mcheck_timer_handler(unsigned long data)
{ 
	static struct tq_struct mcheck_task = { 
		routine: mcheck_timer_dist
	}; 
	schedule_task(&mcheck_task); 
} 
#endif 

static int nok8 __initdata; 

static void __init k8_mcheck_init(struct cpuinfo_x86 *c)
{
	u64 cap;
	int i;

	if (!test_bit(X86_FEATURE_MCE, &c->x86_capability) || 
	    !test_bit(X86_FEATURE_MCA, &c->x86_capability))
		return; 

	rdmsrl(MSR_IA32_MCG_CAP, cap); 
	banks = cap&0xff; 
	for (i = 0; i < banks; i++) { 
		u64 val = ((1UL<<i) & disabled_banks) ? 0 : ~0UL; 
		wrmsrl(MSR_IA32_MC0_STATUS+4*i,0UL);
		wrmsrl(MSR_IA32_MC0_ADDR+4*i,0UL);
		wrmsrl(MSR_IA32_MC0_CTL+4*i, val);
	}

	/* set up the vector first, before enabling MCG_CTL */
	machine_check_vector = k8_machine_check;
	set_in_cr4(X86_CR4_MCE);

	if (cap & (1<<8)) {
		wrmsrl(MSR_IA32_MCG_CTL, 0xffffffffffffffffULL);
	}

	if (mcheck_interval && (smp_processor_id() == 0)) { 
		init_timer(&mcheck_timer); 
		mcheck_timer.function = (void (*)(unsigned long))mcheck_timer_handler; 
		mcheck_timer.expires = jiffies + mcheck_interval; 
		add_timer(&mcheck_timer); 
	} 
	
	printk(KERN_INFO "Machine Check Reporting enabled for CPU#%d\n", smp_processor_id()); 
} 

/*
 *	Set up machine check reporting for Intel processors
 */

static void __init generic_mcheck_init(struct cpuinfo_x86 *c)
{
	u32 l, h;
	int i;
	static int done;
	
	/*
	 *	Check for MCE support
	 */

	if( !test_bit(X86_FEATURE_MCE, &c->x86_capability) )
		return;	
	
	/*
	 *	Check for PPro style MCA
	 */
	 		
	if( !test_bit(X86_FEATURE_MCA, &c->x86_capability) )
		return;
		
	/* Ok machine check is available */
	
	machine_check_vector = generic_machine_check;
	wmb();
	
	if(done==0)
		printk(KERN_INFO "Intel machine check architecture supported.\n");
	rdmsr(MSR_IA32_MCG_CAP, l, h);
	if(l&(1<<8))
		wrmsr(MSR_IA32_MCG_CTL, 0xffffffff, 0xffffffff);
	banks = l&0xff;

	for(i=0;i<banks;i++)
	{
		u32 val = ((1UL<<i) & disabled_banks) ? 0 : ~0;
		wrmsr(MSR_IA32_MC0_CTL+4*i, val, val);
		wrmsr(MSR_IA32_MC0_STATUS+4*i, 0x0, 0x0);
	}
	set_in_cr4(X86_CR4_MCE);
	printk(KERN_INFO "Intel machine check reporting enabled on CPU#%d.\n", 
					 smp_processor_id());
	done=1;
}

/*
 *	This has to be run for each processor
 */

void __init mcheck_init(struct cpuinfo_x86 *c)
{
	if (test_and_set_bit(smp_processor_id(), &mce_cpus))
		return; 

	if(mce_disabled==1)
		return;
		
	switch(c->x86_vendor) {
	case X86_VENDOR_AMD:
		if (c->x86 == 15 && !nok8) {
			k8_mcheck_init(c); 
		} else {
			generic_mcheck_init(c);
		}
		break;
		/* FALL THROUGH */
	default:
	case X86_VENDOR_INTEL:
		generic_mcheck_init(c);
		break;
	}
}

static int __init mcheck_disable(char *str)
{
	mce_disabled = 1;
	return 0;
}


/* mce=off disable machine check
   mce=nok8 disable k8 specific features
   mce=disable<NUMBER> disable bank NUMBER
   mce=enable<NUMBER> enable bank number
   mce=NUMBER mcheck timer interval number seconds. 
   Can be also comma separated in a single mce= */
static int __init mcheck_enable(char *str)
{
	char *p;
	while ((p = strsep(&str,",")) != NULL) { 
		if (isdigit(*p))
			mcheck_interval = simple_strtol(p,NULL,0) * HZ;
		else if (!strcmp(p,"off"))
			mce_disabled = 1; 
		else if (!strncmp(p,"enable",6))
			disabled_banks &= ~(1UL << simple_strtol(p+6,NULL,0));
		else if (!strncmp(p,"disable",7))
			disabled_banks |= 1UL << simple_strtol(p+7,NULL,0);
		else if (!strcmp(p,"nok8"))
			nok8 = 1;
	}
	return 0;
}

__setup("nomce", mcheck_disable);
__setup("mce=", mcheck_enable);
