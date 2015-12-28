/*
 * /proc hooks for CPU performance counter support for SMTC kernel
 * (and ultimately others)
 * Copyright (C) 2006 Mips Technologies, Inc
 */

#include <linux/kernel.h>

#include <asm/cpu.h>
#include <asm/processor.h>
#include <asm/system.h>
#include <asm/mipsregs.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>

/*
 * /proc diagnostic and statistics hooks
 */


/* Internal software-extended event counters */

static unsigned long long extencount[4] = {0,0,0,0};

static struct proc_dir_entry *perf_proc;

static int proc_read_perf(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	int totalen = 0;
	int len;

	len = sprintf(page, "PerfCnt[0].Ctl : 0x%08x\n", read_c0_perfctrl0());
	totalen += len;
	page += len;
	len = sprintf(page, "PerfCnt[0].Cnt : %Lu\n",
		extencount[0] + (unsigned long long)((unsigned)read_c0_perfcntr0()));
	totalen += len;
	page += len;
	len = sprintf(page, "PerfCnt[1].Ctl : 0x%08x\n", read_c0_perfctrl1());
	totalen += len;
	page += len;
	len = sprintf(page, "PerfCnt[1].Cnt : %Lu\n",
		extencount[1] + (unsigned long long)((unsigned)read_c0_perfcntr1()));
	totalen += len;
	page += len;
	len = sprintf(page, "PerfCnt[2].Ctl : 0x%08x\n", read_c0_perfctrl2());
	totalen += len;
	page += len;
	len = sprintf(page, "PerfCnt[2].Cnt : %Lu\n",
		extencount[2] + (unsigned long long)((unsigned)read_c0_perfcntr2()));
	totalen += len;
	page += len;
	len = sprintf(page, "PerfCnt[3].Ctl : 0x%08x\n", read_c0_perfctrl3());
	totalen += len;
	page += len;
	len = sprintf(page, "PerfCnt[3].Cnt : %Lu\n",
		extencount[3] + (unsigned long long)((unsigned)read_c0_perfcntr3()));
	totalen += len;
	page += len;

	return totalen;
}

/*
 * Write to perf counter registers based on text input
 */

#define TXTBUFSZ 100

static int proc_write_perf(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	int len;
	int nparsed;
	int index;
	char mybuf[TXTBUFSZ];

	int which[4];
	unsigned long control[4];
	long long ctrdata[4];

	if(count >= TXTBUFSZ) len = TXTBUFSZ-1;
	else len = count;
	memset(mybuf,0,TXTBUFSZ);
	if(copy_from_user(mybuf, buffer, len)) return -EFAULT;

	nparsed = sscanf(mybuf,
			"%d %lx %Ld %d %lx %Ld %d %lx %Ld %d %lx %Ld",
				&which[0], &control[0], &ctrdata[0],
				&which[1], &control[1], &ctrdata[1],
				&which[2], &control[2], &ctrdata[2],
				&which[3], &control[3], &ctrdata[3]);

	for(index = 0; nparsed >= 3; index++) {
		switch (which[index]) {
		case 0:
			write_c0_perfctrl0(control[index]);
			if(ctrdata[index] != -1) {
			    extencount[0] = (unsigned long long)ctrdata[index];
			    write_c0_perfcntr0((unsigned long)0);
			}
			break;
		case 1:
			write_c0_perfctrl1(control[index]);
			if(ctrdata[index] != -1) {
			    extencount[1] = (unsigned long long)ctrdata[index];
			    write_c0_perfcntr1((unsigned long)0);
			}
			break;
		case 2:
			write_c0_perfctrl2(control[index]);
			if(ctrdata[index] != -1) {
			    extencount[2] = (unsigned long long)ctrdata[index];
			    write_c0_perfcntr2((unsigned long)0);
			}
			break;
		case 3:
			write_c0_perfctrl3(control[index]);
			if(ctrdata[index] != -1) {
			    extencount[3] = (unsigned long long)ctrdata[index];
			    write_c0_perfcntr3((unsigned long)0);
			}
			break;
		}
		nparsed -= 3;
	}
	return (len);
}

extern int (*perf_irq)(void);

/*
 * Invoked when timer interrupt vector picks up a perf counter overflow
 */

static int perf_proc_irq(void)
{
	unsigned long snapshot;

	/*
	 * It would be nice to do this as a loop, but we don't have
	 * indirect access to CP0 registers.
	 */
	snapshot = read_c0_perfcntr0();
	if ((long)snapshot < 0) {
		extencount[0] +=
			(unsigned long long)((unsigned)read_c0_perfcntr0());
		write_c0_perfcntr0(0);
	}
	snapshot = read_c0_perfcntr1();
	if ((long)snapshot < 0) {
		extencount[1] +=
			(unsigned long long)((unsigned)read_c0_perfcntr1());
		write_c0_perfcntr1(0);
	}
	snapshot = read_c0_perfcntr2();
	if ((long)snapshot < 0) {
		extencount[2] +=
			(unsigned long long)((unsigned)read_c0_perfcntr2());
		write_c0_perfcntr2(0);
	}
	snapshot = read_c0_perfcntr3();
	if ((long)snapshot < 0) {
		extencount[3] +=
			(unsigned long long)((unsigned)read_c0_perfcntr3());
		write_c0_perfcntr3(0);
	}
	return 0;
}

static int __init init_perf_proc(void)
{
	extern struct proc_dir_entry *get_mips_proc_dir(void);

	struct proc_dir_entry *mips_proc_dir = get_mips_proc_dir();

	write_c0_perfcntr0(0);
	write_c0_perfcntr1(0);
	write_c0_perfcntr2(0);
	write_c0_perfcntr3(0);
	perf_proc = create_proc_entry("perf", 0644, mips_proc_dir);
	perf_proc->read_proc = proc_read_perf;
	perf_proc->write_proc = proc_write_perf;
	perf_irq = perf_proc_irq;

	return 0;
}

/* Automagically create the entry */
module_init(init_perf_proc);
