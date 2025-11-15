/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _LINUX_SYSINFO_H
#define _LINUX_SYSINFO_H

#include <linux/types.h>

#define SI_LOAD_SHIFT	16
struct sysinfo {
	long uptime;		/* Seconds since boot */
	unsigned long loads[3];	/* 1, 5, and 15 minute load averages */
	unsigned long totalram;	/* Total usable main memory size */
	unsigned long freeram;	/* Available memory size */
	unsigned long sharedram;	/* Amount of shared memory */
	unsigned long bufferram;	/* Memory used by buffers */
	unsigned long totalswap;	/* Total swap space size */
	unsigned long freeswap;	/* swap space still available */
	__u16 procs;		   	/* Number of current processes */
	__u16 pad;		   	/* Explicit padding for m68k */
	unsigned long totalhigh;	/* Total high memory size */
	unsigned long freehigh;	/* Available high memory size */
	__u32 mem_unit;			/* Memory unit size in bytes */
	char _f[20-2*sizeof(unsigned long)-sizeof(__u32)];	/* Padding: libc5 uses this.. */
};

#endif /* _LINUX_SYSINFO_H */
