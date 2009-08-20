/*
 * Broadcom BCM47xx Performance Counters
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: perfcntr.c,v 1.1 2007/09/04 04:45:20 Exp $
 */

#include <linux/config.h>

#ifdef CONFIG_PROC_FS
#include <linux/proc_fs.h>
#include <typedefs.h>
#include <osl.h>
#include <mipsinc.h>

/* 
 * BCM4710 performance counter register select values
 * No even-odd control-counter mapping, just counters
 */
#define PERF_DCACHE_HIT		0
#define PERF_DCACHE_MISS	1
#define PERF_ICACHE_HIT		2
#define PERF_ICACHE_MISS	3
#define PERF_ICOUNT		4

asmlinkage uint read_perf_cntr(uint counter)
{
	uint32 prid = MFC0(C0_PRID, 0);
	if (BCM330X(prid)) {
#ifdef CONFIG_HND_BMIPS3300_PROF
		switch (counter) {
		case PERF_DCACHE_HIT:	return -MFC0(C0_PERFORMANCE, 0);
		case PERF_DCACHE_MISS:	return -MFC0(C0_PERFORMANCE, 1);
		case PERF_ICACHE_HIT:	return -MFC0(C0_PERFORMANCE, 2);
		case PERF_ICACHE_MISS:	return -MFC0(C0_PERFORMANCE, 3);
		}
#endif	/* CONFIG_HND_BMIPS3300_PROF */
		return 0;
	}
	else {
		switch (counter) {
		case PERF_DCACHE_HIT:	return MFC0(C0_PERFORMANCE, 0);
		case PERF_DCACHE_MISS:	return MFC0(C0_PERFORMANCE, 1);
		case PERF_ICACHE_HIT:	return MFC0(C0_PERFORMANCE, 2);
		case PERF_ICACHE_MISS:	return MFC0(C0_PERFORMANCE, 3);
		case PERF_ICOUNT:	return MFC0(C0_PERFORMANCE, 4);
		}
	}
	return 0;
}

/*
 * 'data' passed to proc entry callback is formatted as:
 *	(reg << 16) + sel
 */
#define REGSHIFT	16	/* register # is at high 16 bit */
#define SELMASK		0xffff	/* select # is at low 16 bit */

/*
 * Template to read/write cp0 register. Caller will modify
 * the mfc0 and mtc0 instructions before calling.
 */
static void tmfc0(void)
{
	__asm__ __volatile__(
		".set\tnoreorder\n\t"
		".set\tnoat\n\t"
		"mfc0\t$1,$0\n\t"
		"move\t$2,$1\n\t"
		".set\tat\n\t"
		".set\treorder");
}

static void tmtc0(uint val)
{
	__asm__ __volatile__(
		".set\tnoreorder\n\t"
		".set\tnoat\n\t"
		"move\t$1,$4\n\t"
		"mtc0\t$1,$0\n\t"
		".set\tat\n\t"
		".set\treorder");
}

/*
 * Read/write cp0 register. Assuming code space is writeable
 * so modify the mfc0 and mtc0 instructions before calling the
 * function tmfc0() and tmtc0() to build the correct mfc0 and
 * mtc0 instructions.
 */
static int cp0_read(char *page, char **start, off_t off,
                    int count, int *eof, void *data)
{
	uint reg, sel;
	uint (*cp0i)(void);
	size_t len;

	/* we have done once so stop */
	if (off) {
		len = 0;
		goto done;
	}

	/* change the mfc0 instr with the right reg/sel */
	reg = (uintptr)data >> REGSHIFT;
	sel = (uintptr)data & SELMASK;
	cp0i = (uint (*)(void))KSEG1ADDR(tmfc0);
	*(uint *)cp0i = 0x40010000 | (reg << 11) | sel;
	__asm__ __volatile__("nop; nop; nop; nop;");

	/* return the value in hex string */
	len = sprintf(page, "0x%08x\n", cp0i());
	*start = page;
done:	return len;
}

static int cp0_write(struct file *file, const char *buf,
                     unsigned long count, void *data)
{
	uint reg, sel, val;
	void (*cp0i)(uint);

	/* change the mtc0 instr with the right reg/sel */
	reg = (uintptr)data >> REGSHIFT;
	sel = (uintptr)data & SELMASK;
	val = simple_strtoul(buf, NULL, 0);
	cp0i = (void (*)(uint))KSEG1ADDR(tmtc0);
	*((uint *)cp0i + 1) = 0x40810000 | (reg << 11) | sel;
	__asm__ __volatile__("nop; nop; nop; nop;");

	/* set the value and we are all done */
	cp0i(val);
	return count;
}

#ifdef CONFIG_HND_BMIPS3300_PROF
/*
 * Enable/disable cache hits/misses countings - counters are 
 * hard wired as:
 *	d$ - cntr 0 and 1
 *	i$ - cntr 2 and 3
 * They can't be enabled at the same time according to the BMIPS
 * 3300 documentations, although they are displayed togather.
 */
static void bmips3300_dccntenab(bool enable)
{
	if (enable) {
		MTC0(C0_PERFORMANCE, 6, 0x80000211);	/* enable D$ counting */
		MTC0(C0_PERFORMANCE, 4, 0x80248028);	/* enable cntr 0 and 1 */
		MTC0(C0_PERFORMANCE, 0, 0);		/* zero cntr 0 - # hits */
		MTC0(C0_PERFORMANCE, 1, 0);		/* zero cntr 1 - # misses */
		printk("enabled performance counter 0 for D$ hits\n");
		printk("enabled performance counter 1 for D$ misses\n");
	}
	else {
		MTC0(C0_PERFORMANCE, 4, 0);		/* disable cntr 0 and 1 */
		MTC0(C0_PERFORMANCE, 6, 0);		/* disable D$ counting */
		printk("disabled performance counters\n");
	}
}

static void bmips3300_iccntenab(bool enable)
{
	if (enable) {
		MTC0(C0_PERFORMANCE, 6, 0x80000218);	/* enable I$ counting */
		MTC0(C0_PERFORMANCE, 5, 0x80148018);	/* enable cntr 2 and 3 */
		MTC0(C0_PERFORMANCE, 2, 0);		/* zero cntr 0 - # hits */
		MTC0(C0_PERFORMANCE, 3, 0);		/* zero cntr 1 - # misses */
		printk("enabled performance counter 2 for I$ hits\n");
		printk("enabled performance counter 3 for I$ misses\n");
	}
	else {
		MTC0(C0_PERFORMANCE, 5, 0);		/* disable cntr 2 and 3 */
		MTC0(C0_PERFORMANCE, 6, 0);		/* disable I$ counting */
		printk("disabled performance counters\n");
	}
}

/* cache counting enable/disable proc entry callback */
#define DCACHE_CNTENAB	0
#define ICACHE_CNTENAB	1

static int bmips3300_ccntenab(struct file *file, const char *buf,
                         unsigned long count, void *data)
{
	uint val = simple_strtoul(buf, NULL, 0);
	switch ((uint)data) {
	case DCACHE_CNTENAB:
		bmips3300_dccntenab(val != 0);
		break;
	case ICACHE_CNTENAB:
		bmips3300_iccntenab(val != 0);
		break;
	}
	return count;
}
#endif	/* CONFIG_HND_BMIPS3300_PROF */

/* cp0 registers/selects map */
#define MAXREGS		32	/* max # registers */
#define MAXSELS		32	/* max # selects */
#define CP0REG(r)	(1<<(r))
#define CP0SEL(s)	(1<<(s))
typedef struct
{
	uint32 reg_map;		/* registers map */
	uint8 sel_map[MAXSELS];	/* selects map */
} cp0_reg_map_t;

#ifdef CONFIG_HND_BMIPS3300_PROF
static cp0_reg_map_t bmips3300_cp0regmap =
{
	CP0REG(0)|CP0REG(1)|CP0REG(2)|CP0REG(3)|CP0REG(4)|CP0REG(5)|
		CP0REG(6)|CP0REG(7)|CP0REG(8)|CP0REG(9)|CP0REG(10)|
		CP0REG(11)|CP0REG(12)|CP0REG(13)|CP0REG(14)|CP0REG(15)|
		CP0REG(16)|CP0REG(22)|CP0REG(23)|CP0REG(24)|CP0REG(25)|
		CP0REG(28)|CP0REG(30)|CP0REG(31),
	{
		/* 0 */	CP0SEL(0),
		/* 1 */	CP0SEL(0),
		/* 2 */	CP0SEL(0),
		/* 3 */	CP0SEL(0),
		/* 4 */	CP0SEL(0),
		/* 5 */	CP0SEL(0),
		/* 6 */	CP0SEL(0),
		/* 7 */	CP0SEL(0),
		/* 8 */	CP0SEL(0),
		/* 9 */	CP0SEL(0),
		/* 10 */ CP0SEL(0),
		/* 11 */ CP0SEL(0),
		/* 12 */ CP0SEL(0),
		/* 13 */ CP0SEL(0),
		/* 14 */ CP0SEL(0),
		/* 15 */ CP0SEL(0),
		/* 16 */ CP0SEL(0)|CP0SEL(1),
		0,
		0,
		0,
		0,
		0,
		/* 22 */ CP0SEL(0)|CP0SEL(4)|CP0SEL(5),
		/* 23 */ CP0SEL(0),
		/* 24 */ CP0SEL(0),
		/* 25 */ CP0SEL(0)|CP0SEL(1)|CP0SEL(2)|CP0SEL(3)|CP0SEL(4)|
			CP0SEL(5)|CP0SEL(6),
		0,
		0,
		/* 28 */ CP0SEL(0)|CP0SEL(1),
		0,
		/* 30 */ CP0SEL(0),
		/* 31 */ CP0SEL(0),
	}
};
#endif	/* CONFIG_HND_BMIPS3300_PROF */

/* create/remove proc entries - no worry about error handling;-( */
static int __init cp0_init(void)
{
	struct proc_dir_entry *cp0_proc;
#ifdef CONFIG_HND_BMIPS3300_PROF
	struct proc_dir_entry *cache_proc;
#endif	/* CONFIG_HND_BMIPS3300_PROF */
	struct proc_dir_entry *reg_proc, *sel_proc;
	cp0_reg_map_t *reg_map = NULL;
	uint32 prid;
	char name[16];
	int i, j;

	/* create proc entry cp0 in root */
	cp0_proc = create_proc_entry("cp0", 0444 | S_IFDIR, &proc_root);
	if (!cp0_proc)
		return 0;

	/* create proc entries for enabling cache hit/miss counting */
	prid = MFC0(C0_PRID, 0);
	if (BCM330X(prid)) {
#ifdef CONFIG_HND_BMIPS3300_PROF
		/* D$ */
		cache_proc = create_proc_entry("dccnt", 0644, cp0_proc);
		if (!cache_proc)
			return 0;
		cache_proc->write_proc = bmips3300_ccntenab;
		cache_proc->data = (void *)DCACHE_CNTENAB;
		/* I$ */
		cache_proc = create_proc_entry("iccnt", 0644, cp0_proc);
		if (!cache_proc)
			return 0;
		cache_proc->write_proc = bmips3300_ccntenab;
		cache_proc->data = (void *)ICACHE_CNTENAB;
#endif	/* CONFIG_HND_BMIPS3300_PROF */
	}

	/* select cp0 registers/selects map table */
	if (BCM330X(prid)) {
#ifdef CONFIG_HND_BMIPS3300_PROF
		reg_map = &bmips3300_cp0regmap;
#endif	/* CONFIG_HND_BMIPS3300_PROF */
	}
	if (!reg_map)
		return 0;
	/* create proc entry for each register select */
	for (i = 0; i < MAXREGS; i ++) {
		if (!(reg_map->reg_map & (1 << i)))
			continue;
		sprintf(name, "%u", i);
		reg_proc = create_proc_entry(name, 0444 | S_IFDIR, cp0_proc);
		if (!reg_proc)
			break;
		for (j = 0; j < MAXSELS; j ++) {
			if (!(reg_map->sel_map[i] & (1 << j)))
				continue;
			sprintf(name, "%u", j);
			sel_proc = create_proc_entry(name, 0644, reg_proc);
			if (!sel_proc)
				break;
			sel_proc->read_proc = cp0_read;
			sel_proc->write_proc = cp0_write;
			sel_proc->data = (void *)((i << REGSHIFT) + j);
		}
	}
	return 0;
}

static void __exit cp0_cleanup(void)
{
	remove_proc_entry("cp0", &proc_root);
}

/* hook it up with system at boot time */
module_init(cp0_init);
module_exit(cp0_cleanup);

#endif	/* CONFIG_PROC_FS */
