#include <linux/cpufreq.h>
#include <mach/system.h>
#include "al_hal_pll.h"
#include "al_hal_pll_map.h"
#include <linux/err.h>
#include <linux/slab.h>

#define AL_SB_BASE		0xfc000000
#define AL_SB_RING_BASE		(AL_SB_BASE + 0x01860000)
#define AL_PLL_BASE(id)		(AL_SB_RING_BASE + 0xb00 + (id) * 0x100)
#define AL_PLL_SB		0
#define AL_PLL_NB		1
#define AL_PLL_CPU		2

#define AL_REG_FIELD_GET(reg, mask, shift)  (((reg) & (mask)) >> (shift))
#define AL_SB_PBS_BASE		(AL_SB_BASE + 0xfd880000)
#define AL_PBS_REGFILE_BASE	(AL_SB_PBS_BASE + 0x00028000)
#define	AL_DEFAULT_CPUFREQ	1400000

static struct cpufreq_frequency_table *ftbl;
static struct al_pll_obj pll_obj;
static enum al_pll_freq freq;
static const struct al_pll_freq_map_ent *map;
static DEFINE_PER_CPU(struct clk *, cpu_clks);
static unsigned int max_scalable_frequency;

static struct cpufreq_frequency_table *cpufreq_parse(int cpu)
{
	u32 i;



	ftbl = kzalloc((pll_obj.freq_map_size + 1) * sizeof(*ftbl), GFP_KERNEL);
	if (!ftbl)
		return ERR_PTR(-ENOMEM);

	for (i = 0; i < pll_obj.freq_map_size; i++) {
		unsigned long f;
		struct clk *cpu_clk;

		cpu_clk = per_cpu(cpu_clks, cpu);

		f = map[i].freq_val * 1000;
		f /= 1000;

		ftbl[i].index = map[i].freq;
		ftbl[i].frequency = f;
	}

	ftbl[i].index = i;
	ftbl[i].frequency = CPUFREQ_TABLE_END;


	return ftbl;
}

static int alpine_cpufreq_verify(struct cpufreq_policy *policy)
{
	unsigned int tmp;

	if (policy->max < policy->min) {
		tmp = policy->max;
		policy->max = policy->min;
		policy->min = tmp;
	}
	if (policy->cur < policy->min)
		policy->min = policy->cur;
	if (policy->cur > policy->max)
		policy->max = policy->cur;
	return 0;
}

static int __cpuinit alpine_cpufreq_init(struct cpufreq_policy *policy)
{
	int ret = 0, err = 0;
	void *reg;
	unsigned int min_freq = ~0;
	unsigned int freq;

	enum al_pll_ref_clk_freq ref_clk;
	unsigned field;
	struct cpufreq_frequency_table *pos;


	reg = ioremap(AL_PBS_REGFILE_BASE + 0x110, 0x4);
	field  = AL_REG_FIELD_GET(al_reg_read32(reg), (1 << 19), 19);
	iounmap(reg);

	switch(field) {
	case 0x0:
		map = al_pll_freq_map_25;
		ref_clk = AL_PLL_REF_CLK_FREQ_25_MHZ; 
		break;
	case 0x1:
		map = al_pll_freq_map_100;
		ref_clk = AL_PLL_REF_CLK_FREQ_100_MHZ;
		break;
	default: 
		return -1;
	}


	err = al_pll_init((void *)AL_PLL_BASE(AL_PLL_CPU),
			  "alpine", ref_clk, &pll_obj);
	if (err)
		return -1;


	ftbl = cpufreq_parse(0);
	if (!ftbl) {
		pr_err("Freq table not initialized.\n");
		return -ENODEV;
	}

	for (pos = ftbl; pos->frequency != CPUFREQ_TABLE_END; pos++) {
		if (pos->frequency == CPUFREQ_ENTRY_INVALID)
			continue;
		else if (pos->frequency == CPUFREQ_TABLE_END)
			break;
		else {
			freq = pos->frequency;
			if (freq < min_freq)
				min_freq = freq;
			if (freq > max_scalable_frequency)
				max_scalable_frequency = freq;
		}
	}
	cpufreq_frequency_table_get_attr(ftbl, 0);

	policy->cpuinfo.min_freq = min_freq; 
	policy->min = min_freq;
	policy->cpuinfo.max_freq = AL_DEFAULT_CPUFREQ;
	policy->max = AL_DEFAULT_CPUFREQ;
	err = al_pll_freq_get(&pll_obj, &freq, &ret);
	if(err)
		ret = AL_DEFAULT_CPUFREQ;
	policy->cur = ret;
	policy->cpuinfo.transition_latency = CPUFREQ_ETERNAL;
	cpumask_setall(policy->cpus);
	return 0;
}

static unsigned int alpine_get_target(unsigned int cpu)
{
	int err = 0;
	unsigned ret = 0;
	err = al_pll_freq_get(&pll_obj,	&freq, &ret);
	if (err)
		return -1;
	return ret;
}

static int alpine_target(struct cpufreq_policy *policy, unsigned int target_freq,
							unsigned int relation)
{
	int i, err = 0;
	enum al_pll_freq freq = AL_PLL_FREQ_NA;

	if (!strcmp(policy->governor->name, "userspace"))
		policy->max = max_scalable_frequency;
	else
		policy->max = AL_DEFAULT_CPUFREQ;

	for (i = 0; i < pll_obj.freq_map_size; i++) {
		if((map[i]).freq_val == target_freq) {
			freq = (map[i]).freq;
			break;
		}
		if((map[i]).freq_val > target_freq && i > 0) {
			freq = (map[i - 1]).freq;
			target_freq = (map[i]).freq_val;
			break;
		}
	}
	if(i == pll_obj.freq_map_size)
		return -1;

	err = al_pll_freq_set(&pll_obj,	freq, 100000);
	if (err) {
		printk("Failure %d \n", err);
		return -1;
	}
	policy->cur = target_freq;
	return 0;
}

static struct freq_attr *alpine_attributes[] = {
        &cpufreq_freq_attr_scaling_available_freqs,
        NULL,
};

static struct cpufreq_driver alpine_cpufreq_driver = {
	.flags =/* CPUFREQ_STICKY |*/ CPUFREQ_CONST_LOOPS,
	.target =alpine_target,
	.get = alpine_get_target,
	.init = alpine_cpufreq_init,
	.verify = alpine_cpufreq_verify,
	.attr = alpine_attributes,
	.name = "alpine",
};

static int __init alpine_cpufreq_register(void)
{
	if (rb_mach != RB_MACH_ALPINE) {
		return -1;
	}

	return cpufreq_register_driver(&alpine_cpufreq_driver);
}

late_initcall(alpine_cpufreq_register);
