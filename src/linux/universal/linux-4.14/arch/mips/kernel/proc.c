// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright (C) 1995, 1996, 2001  Ralf Baechle
 *  Copyright (C) 2001, 2004  MIPS Technologies, Inc.
 *  Copyright (C) 2004	Maciej W. Rozycki
 */
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <asm/bootinfo.h>
#include <asm/cpu.h>
#include <asm/cpu-features.h>
#include <asm/idle.h>
#include <asm/mipsregs.h>
#include <asm/processor.h>
#include <asm/prom.h>

unsigned int vced_count, vcei_count;
extern unsigned int getCPUClock(void);

/*
 *  * No lock; only written during early bootup by CPU 0.
 *   */
static RAW_NOTIFIER_HEAD(proc_cpuinfo_chain);

int __ref register_proc_cpuinfo_notifier(struct notifier_block *nb)
{
	return raw_notifier_chain_register(&proc_cpuinfo_chain, nb);
}

int proc_cpuinfo_notifier_call_chain(unsigned long val, void *v)
{
	return raw_notifier_call_chain(&proc_cpuinfo_chain, val, v);
}

static int show_cpuinfo(struct seq_file *m, void *v)
{
	struct proc_cpuinfo_notifier_args proc_cpuinfo_notifier_args;
	unsigned long n = (unsigned long) v - 1;
	unsigned int version = cpu_data[n].processor_id;
	unsigned int fp_vers = cpu_data[n].fpu_id;
	char fmt [64];
	int i;

#ifdef CONFIG_SMP
	if (!cpu_online(n))
		return 0;
#endif

	/*
	 * For the first processor also print the system type
	 */
	if (n == 0) {
		seq_printf(m, "system type\t\t: %s\n", get_system_type());
//		if (mips_get_machine_name())
//			seq_printf(m, "machine\t\t\t: %s\n",
//				   mips_get_machine_name());
	}

	seq_printf(m, "processor\t\t: %ld\n", n);
	sprintf(fmt, "cpu model\t\t: %%s V%%d.%%d%s\n",
		      cpu_data[n].options & MIPS_CPU_FPU ? "  FPU V%d.%d" : "");
	seq_printf(m, fmt, __cpu_name[n],
		      (version >> 4) & 0x0f, version & 0x0f,
		      (fp_vers >> 4) & 0x0f, fp_vers & 0x0f);
	seq_printf(m, "BogoMIPS\t\t: %u.%02u\n",
		      cpu_data[n].udelay_val / (500000/HZ),
		      (cpu_data[n].udelay_val / (5000/HZ)) % 100);
	seq_printf(m, "CPUClock\t\t: %d\n",getCPUClock());
	seq_printf(m, "wait instruction\t: %s\n", cpu_wait ? "yes" : "no");
	seq_printf(m, "microsecond timers\t: %s\n",
		      cpu_has_counter ? "yes" : "no");
	seq_printf(m, "tlb_entries\t\t: %d\n", cpu_data[n].tlbsize);
	seq_printf(m, "extra interrupt vector\t: %s\n",
		      cpu_has_divec ? "yes" : "no");
	seq_printf(m, "hardware watchpoint\t: %s",
		      cpu_has_watch ? "yes, " : "no\n");
	if (cpu_has_watch) {
		seq_printf(m, "count: %d, address/irw mask: [",
		      cpu_data[n].watch_reg_count);
		for (i = 0; i < cpu_data[n].watch_reg_count; i++)
			seq_printf(m, "%s0x%04x", i ? ", " : "" ,
				cpu_data[n].watch_reg_masks[i]);
		seq_printf(m, "]\n");
	}

	seq_printf(m, "isa\t\t\t:"); 
	if (cpu_has_mips_1)
		seq_printf(m, " mips1");
	if (cpu_has_mips_2)
		seq_printf(m, "%s", " mips2");
	if (cpu_has_mips_3)
		seq_printf(m, "%s", " mips3");
	if (cpu_has_mips_4)
		seq_printf(m, "%s", " mips4");
	if (cpu_has_mips_5)
		seq_printf(m, "%s", " mips5");
	if (cpu_has_mips32r1)
		seq_printf(m, "%s", " mips32r1");
	if (cpu_has_mips32r2)
		seq_printf(m, "%s", " mips32r2");
	if (cpu_has_mips32r6)
		seq_printf(m, "%s", " mips32r6");
	if (cpu_has_mips64r1)
		seq_printf(m, "%s", " mips64r1");
	if (cpu_has_mips64r2)
		seq_printf(m, "%s", " mips64r2");
	if (cpu_has_mips64r6)
		seq_printf(m, "%s", " mips64r6");
	seq_printf(m, "\n");

	seq_printf(m, "ASEs implemented\t:");
	if (cpu_has_mips16)	seq_printf(m, "%s", " mips16");
	if (cpu_has_mips16e2)	seq_printf(m, "%s", " mips16e2");
	if (cpu_has_mdmx)	seq_printf(m, "%s", " mdmx");
	if (cpu_has_mips3d)	seq_printf(m, "%s", " mips3d");
	if (cpu_has_smartmips)	seq_printf(m, "%s", " smartmips");
	if (cpu_has_dsp)	seq_printf(m, "%s", " dsp");
	if (cpu_has_dsp2)	seq_printf(m, "%s", " dsp2");
	if (cpu_has_dsp3)	seq_printf(m, "%s", " dsp3");
	if (cpu_has_mipsmt)	seq_printf(m, "%s", " mt");
	if (cpu_has_mmips)	seq_printf(m, "%s", " micromips");
	if (cpu_has_vz)		seq_printf(m, "%s", " vz");
	if (cpu_has_msa)	seq_printf(m, "%s", " msa");
	if (cpu_has_eva)	seq_printf(m, "%s", " eva");
	if (cpu_has_htw)	seq_printf(m, "%s", " htw");
	if (cpu_has_xpa)	seq_printf(m, "%s", " xpa");
	seq_printf(m, "\n");

	if (cpu_has_mmips) {
		seq_printf(m, "micromips kernel\t: %s\n",
		      (read_c0_config3() & MIPS_CONF3_ISA_OE) ?  "yes" : "no");
	}

	seq_printf(m, "Options implemented\t:");
	if (cpu_has_tlb)
		seq_printf(m, "%s", " tlb");
	if (cpu_has_ftlb)
		seq_printf(m, "%s", " ftlb");
	if (cpu_has_tlbinv)
		seq_printf(m, "%s", " tlbinv");
	if (cpu_has_segments)
		seq_printf(m, "%s", " segments");
	if (cpu_has_rixiex)
		seq_printf(m, "%s", " rixiex");
	if (cpu_has_ldpte)
		seq_printf(m, "%s", " ldpte");
	if (cpu_has_rw_llb)
		seq_printf(m, "%s", " rw_llb");
	if (cpu_has_4kex)
		seq_printf(m, "%s", " 4kex");
	if (cpu_has_3k_cache)
		seq_printf(m, "%s", " 3k_cache");
	if (cpu_has_4k_cache)
		seq_printf(m, "%s", " 4k_cache");
	if (cpu_has_6k_cache)
		seq_printf(m, "%s", " 6k_cache");
	if (cpu_has_8k_cache)
		seq_printf(m, "%s", " 8k_cache");
	if (cpu_has_tx39_cache)
		seq_printf(m, "%s", " tx39_cache");
	if (cpu_has_octeon_cache)
		seq_printf(m, "%s", " octeon_cache");
	if (cpu_has_fpu)
		seq_printf(m, "%s", " fpu");
	if (cpu_has_32fpr)
		seq_printf(m, "%s", " 32fpr");
	if (cpu_has_cache_cdex_p)
		seq_printf(m, "%s", " cache_cdex_p");
	if (cpu_has_cache_cdex_s)
		seq_printf(m, "%s", " cache_cdex_s");
	if (cpu_has_prefetch)
		seq_printf(m, "%s", " prefetch");
	if (cpu_has_mcheck)
		seq_printf(m, "%s", " mcheck");
	if (cpu_has_ejtag)
		seq_printf(m, "%s", " ejtag");
	if (cpu_has_llsc)
		seq_printf(m, "%s", " llsc");
	if (cpu_has_bp_ghist)
		seq_printf(m, "%s", " bp_ghist");
	if (cpu_has_guestctl0ext)
		seq_printf(m, "%s", " guestctl0ext");
	if (cpu_has_guestctl1)
		seq_printf(m, "%s", " guestctl1");
	if (cpu_has_guestctl2)
		seq_printf(m, "%s", " guestctl2");
	if (cpu_has_guestid)
		seq_printf(m, "%s", " guestid");
	if (cpu_has_drg)
		seq_printf(m, "%s", " drg");
	if (cpu_has_rixi)
		seq_printf(m, "%s", " rixi");
	if (cpu_has_lpa)
		seq_printf(m, "%s", " lpa");
	if (cpu_has_mvh)
		seq_printf(m, "%s", " mvh");
	if (cpu_has_vtag_icache)
		seq_printf(m, "%s", " vtag_icache");
	if (cpu_has_dc_aliases)
		seq_printf(m, "%s", " dc_aliases");
	if (cpu_has_ic_fills_f_dc)
		seq_printf(m, "%s", " ic_fills_f_dc");
	if (cpu_has_pindexed_dcache)
		seq_printf(m, "%s", " pindexed_dcache");
	if (cpu_has_userlocal)
		seq_printf(m, "%s", " userlocal");
	if (cpu_has_nofpuex)
		seq_printf(m, "%s", " nofpuex");
	if (cpu_has_vint)
		seq_printf(m, "%s", " vint");
	if (cpu_has_veic)
		seq_printf(m, "%s", " veic");
	if (cpu_has_inclusive_pcaches)
		seq_printf(m, "%s", " inclusive_pcaches");
	if (cpu_has_perf_cntr_intr_bit)
		seq_printf(m, "%s", " perf_cntr_intr_bit");
	if (cpu_has_ufr)
		seq_printf(m, "%s", " ufr");
	if (cpu_has_fre)
		seq_printf(m, "%s", " fre");
	if (cpu_has_cdmm)
		seq_printf(m, "%s", " cdmm");
	if (cpu_has_small_pages)
		seq_printf(m, "%s", " small_pages");
	if (cpu_has_nan_legacy)
		seq_printf(m, "%s", " nan_legacy");
	if (cpu_has_nan_2008)
		seq_printf(m, "%s", " nan_2008");
	if (cpu_has_ebase_wg)
		seq_printf(m, "%s", " ebase_wg");
	if (cpu_has_badinstr)
		seq_printf(m, "%s", " badinstr");
	if (cpu_has_badinstrp)
		seq_printf(m, "%s", " badinstrp");
	if (cpu_has_contextconfig)
		seq_printf(m, "%s", " contextconfig");
	if (cpu_has_perf)
		seq_printf(m, "%s", " perf");
	if (cpu_has_shared_ftlb_ram)
		seq_printf(m, "%s", " shared_ftlb_ram");
	if (cpu_has_shared_ftlb_entries)
		seq_printf(m, "%s", " shared_ftlb_entries");
	if (cpu_has_mipsmt_pertccounters)
		seq_printf(m, "%s", " mipsmt_pertccounters");
	seq_printf(m, "\n");

	seq_printf(m, "shadow register sets\t: %d\n",
		      cpu_data[n].srsets);
	seq_printf(m, "kscratch registers\t: %d\n",
		      hweight8(cpu_data[n].kscratch_mask));
	seq_printf(m, "package\t\t\t: %d\n", cpu_data[n].package);
	seq_printf(m, "core\t\t\t: %d\n", cpu_core(&cpu_data[n]));

#if defined(CONFIG_MIPS_MT_SMP) || defined(CONFIG_CPU_MIPSR6)
	if (cpu_has_mipsmt)
		seq_printf(m, "VPE\t\t\t: %d\n", cpu_vpe_id(&cpu_data[n]));
	else if (cpu_has_vp)
		seq_printf(m, "VP\t\t\t: %d\n", cpu_vpe_id(&cpu_data[n]));
#endif

	sprintf(fmt, "VCE%%c exceptions\t\t: %s\n",
		      cpu_has_vce ? "%u" : "not available");
	seq_printf(m, fmt, 'D', vced_count);
	seq_printf(m, fmt, 'I', vcei_count);

	proc_cpuinfo_notifier_args.m = m;
	proc_cpuinfo_notifier_args.n = n;

	raw_notifier_call_chain(&proc_cpuinfo_chain, 0,
				&proc_cpuinfo_notifier_args);

	seq_printf(m, "\n");

	return 0;
}

static void *c_start(struct seq_file *m, loff_t *pos)
{
	unsigned long i = *pos;

	return i < nr_cpu_ids ? (void *) (i + 1) : NULL;
}

static void *c_next(struct seq_file *m, void *v, loff_t *pos)
{
	++*pos;
	return c_start(m, pos);
}

static void c_stop(struct seq_file *m, void *v)
{
}

const struct seq_operations cpuinfo_op = {
	.start	= c_start,
	.next	= c_next,
	.stop	= c_stop,
	.show	= show_cpuinfo,
};

/*
 * Support for MIPS/local /proc hooks in /proc/mips/
 */

static struct proc_dir_entry *mips_proc = NULL;

struct proc_dir_entry *get_mips_proc_dir(void)
{
       /*
        * This ought not to be preemptable.
        */
       if(mips_proc == NULL)
               mips_proc = proc_mkdir("mips", NULL);
       return(mips_proc);
}
