/*
 * /proc hooks for MIPS MT scheduling policy management for 34K cores
 *
 *  This program is free software; you can distribute it and/or modify it
 *  under the terms of the GNU General Public License (Version 2) as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 * Copyright (C) 2006 Mips Technologies, Inc
 */

#include <linux/kernel.h>

#include <asm/cpu.h>
#include <asm/processor.h>
#include <asm/system.h>
#include <asm/mipsregs.h>
#include <asm/mipsmtregs.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>

static struct proc_dir_entry *mtsched_proc;

#ifndef CONFIG_MIPS_MT_SMTC
#define NTCS 2
#else
#define NTCS NR_CPUS
#endif
#define NVPES 2

int lastvpe = 1;
int lasttc = 8;

static int proc_read_mtsched(char *page, char **start, off_t off,
			int count, int *eof, void *data)
{
	int totalen = 0;
	int len;

	int i;
	int vpe;
	int mytc;
	unsigned long flags;
	unsigned int mtflags;
	unsigned int haltstate;
	unsigned int vpes_checked[NVPES];
	unsigned int vpeschedule[NVPES];
	unsigned int vpeschefback[NVPES];
	unsigned int tcschedule[NTCS];
	unsigned int tcschefback[NTCS];

	/* Dump the state of the MIPS MT scheduling policy manager */
	/* Inititalize control state */
	for(i = 0; i < NVPES; i++) {
		vpes_checked[i] = 0;
		vpeschedule[i] = 0;
		vpeschefback[i] = 0;
	}
	for(i = 0; i < NTCS; i++) {
		tcschedule[i] = 0;
		tcschefback[i] = 0;
	}

	/* Disable interrupts and multithreaded issue */
	local_irq_save(flags);
	mtflags = dvpe();

	/* Then go through the TCs, halt 'em, and extract the values */
	mytc = (read_c0_tcbind() & TCBIND_CURTC) >> TCBIND_CURTC_SHIFT;
	for(i = 0; i < NTCS; i++) {
		if(i == mytc) {
			/* No need to halt ourselves! */
			tcschedule[i] = read_c0_tcschedule();
			tcschefback[i] = read_c0_tcschefback();
			/* If VPE bound to TC hasn't been checked, do it */
			vpe = read_c0_tcbind() & TCBIND_CURVPE;
			if(!vpes_checked[vpe]) {
				vpeschedule[vpe] = read_c0_vpeschedule();
				vpeschefback[vpe] = read_c0_vpeschefback();
				vpes_checked[vpe] = 1;
			}
		} else {
			settc(i);
			haltstate = read_tc_c0_tchalt();
			write_tc_c0_tchalt(TCHALT_H);
			mips_ihb();
			tcschedule[i] = read_tc_c0_tcschedule();
			tcschefback[i] = read_tc_c0_tcschefback();
			/* If VPE bound to TC hasn't been checked, do it */
			vpe = read_tc_c0_tcbind() & TCBIND_CURVPE;
			if(!vpes_checked[vpe]) {
			    vpeschedule[vpe] = read_vpe_c0_vpeschedule();
			    vpeschefback[vpe] = read_vpe_c0_vpeschefback();
			    vpes_checked[vpe] = 1;
			}
			if(!haltstate) write_tc_c0_tchalt(0);
		}
	}
	/* Re-enable MT and interrupts */
	evpe(mtflags);
	local_irq_restore(flags);

	for(vpe=0; vpe < NVPES; vpe++) {
		len = sprintf(page, "VPE[%d].VPEschedule  = 0x%08x\n",
			vpe, vpeschedule[vpe]);
		totalen += len;
		page += len;
		len = sprintf(page, "VPE[%d].VPEschefback = 0x%08x\n",
			vpe, vpeschefback[vpe]);
		totalen += len;
		page += len;
	}
	for(i=0; i < NTCS; i++) {
		len = sprintf(page, "TC[%d].TCschedule    = 0x%08x\n",
			i, tcschedule[i]);
		totalen += len;
		page += len;
		len = sprintf(page, "TC[%d].TCschefback   = 0x%08x\n",
			i, tcschefback[i]);
		totalen += len;
		page += len;
	}
	return totalen;
}

/*
 * Write to perf counter registers based on text input
 */

#define TXTBUFSZ 100

static int proc_write_mtsched(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	int len = 0;
	char mybuf[TXTBUFSZ];
	/* At most, we will set up 9 TCs and 2 VPEs, 11 entries in all */
	char entity[1];   //, entity1[1];
	int number[1];
	unsigned long value[1];
	int nparsed = 0 , index = 0;
	unsigned long flags;
	unsigned int mtflags;
	unsigned int haltstate;
	unsigned int tcbindval;

	if(count >= TXTBUFSZ) len = TXTBUFSZ-1;
	else len = count;
	memset(mybuf,0,TXTBUFSZ);
	if(copy_from_user(mybuf, buffer, len)) return -EFAULT;

	nparsed = sscanf(mybuf, "%c%d %lx",
		 &entity[0] ,&number[0], &value[0]);

	/*
	 * Having acquired the inputs, which might have
	 * generated exceptions and preemptions,
	 * program the registers.
	 */
	/* Disable interrupts and multithreaded issue */
	local_irq_save(flags);
	mtflags = dvpe();

	if(entity[index] == 't' ) {
		/* Set TCSchedule or TCScheFBack of specified TC */
		if(number[index] > NTCS) goto skip;
		/* If it's our own TC, do it direct */
		if(number[index] ==
				((read_c0_tcbind() & TCBIND_CURTC)
				>> TCBIND_CURTC_SHIFT)) {
			if(entity[index] == 't')
				 write_c0_tcschedule(value[index]);
			else
				write_c0_tcschefback(value[index]);
		} else {
		/* Otherwise, we do it via MTTR */
			settc(number[index]);
			haltstate = read_tc_c0_tchalt();
			write_tc_c0_tchalt(TCHALT_H);
			mips_ihb();
			if(entity[index] == 't')
				 write_tc_c0_tcschedule(value[index]);
			else
				write_tc_c0_tcschefback(value[index]);
			mips_ihb();
			if(!haltstate) write_tc_c0_tchalt(0);
		}
	} else if(entity[index] == 'v') {
		/* Set VPESchedule of specified VPE */
		if(number[index] > NVPES) goto skip;
		tcbindval = read_c0_tcbind();
		/* Are we doing this to our current VPE? */
		if((tcbindval & TCBIND_CURVPE) == number[index]) {
			/* Then life is simple */
			write_c0_vpeschedule(value[index]);
		} else {
			/*
			 * Bind ourselves to the other VPE long enough
			 * to program the bind value.
			 */
			write_c0_tcbind((tcbindval & ~TCBIND_CURVPE)
					   | number[index]);
			mips_ihb();
			write_c0_vpeschedule(value[index]);
			mips_ihb();
			/* Restore previous binding */
			write_c0_tcbind(tcbindval);
			mips_ihb();
		}
	}

	else if(entity[index] == 'r') {
		unsigned int vpes_checked[2], vpe ,i , mytc;
		vpes_checked[0] = vpes_checked[1] = 0;

		/* Then go through the TCs, halt 'em, and extract the values */
		mytc = (read_c0_tcbind() & TCBIND_CURTC) >> TCBIND_CURTC_SHIFT;

		for(i = 0; i < NTCS; i++) {
			if(i == mytc) {
				/* No need to halt ourselves! */
				write_c0_vpeschefback(0);
				write_c0_tcschefback(0);
			} else {
				settc(i);
				haltstate = read_tc_c0_tchalt();
				write_tc_c0_tchalt(TCHALT_H);
				mips_ihb();
				write_tc_c0_tcschefback(0);
				/* If VPE bound to TC hasn't been checked, do it */
				vpe = read_tc_c0_tcbind() & TCBIND_CURVPE;
				if(!vpes_checked[vpe]) {
				    write_vpe_c0_vpeschefback(0);
				    vpes_checked[vpe] = 1;
				}
				if(!haltstate) write_tc_c0_tchalt(0);
			}
		}
	}
	else {
		printk ("\n Usage : <t/v><0/1> <Hex Value>\n Example : t0 0x01\n");
	}

skip:
	/* Re-enable MT and interrupts */
	evpe(mtflags);
	local_irq_restore(flags);
	return (len);
}

static int __init init_mtsched_proc(void)
{
	extern struct proc_dir_entry *get_mips_proc_dir(void);
	struct proc_dir_entry *mips_proc_dir;

	if (!cpu_has_mipsmt) {
		printk("mtsched: not a MIPS MT capable processor\n");
		return -ENODEV;
	}

	mips_proc_dir = get_mips_proc_dir();

	mtsched_proc = create_proc_entry("mtsched", 0644, mips_proc_dir);
	mtsched_proc->read_proc = proc_read_mtsched;
	mtsched_proc->write_proc = proc_write_mtsched;

	return 0;
}

/* Automagically create the entry */
module_init(init_mtsched_proc);
