/*
 * Copyright 2004 PMC-Sierra Inc.
 * Author: Manish Lachwani (lachwani@pmc-sierra.com)
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/config.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/bootmem.h>
#include <asm/addrspace.h>
#include <asm/bootinfo.h>

/* PMON Call Vectors */
struct callvectors {
int	(*open) (char*, int, int);
int	(*close) (int);
int	(*read) (int, void*, int);
int	(*write) (int, void*, int);
off_t	(*lseek) (int, off_t, int);
int	(*printf) (const char*, ...);
void	(*cacheflush) (void);
char*	(*gets) (char*);
};

struct callvectors* debug_vectors;
char arcs_cmdline[CL_SIZE];
extern unsigned long cpu_clock;
unsigned char big_sur_mac_addr_base[6] = "00:11:22:33:44:aa";

const char *get_system_type(void)
{
	return "Big Sur";
}

void __init prom_init(int argc, char **arg, char **env, struct callvectors *cv)
{
	int i;

	debug_vectors = cv;

	/* PMON args begin with a g that stands for go */
	arcs_cmdline[0] = '\0';
	for (i = 1; i < argc; i++) {
		if (strlen(arcs_cmdline) + strlen(arg[i] + 1)
			>= sizeof(arcs_cmdline))
				break;

		strcat(arcs_cmdline, arg[i]);
		strcat(arcs_cmdline, " ");
	}

	while (*env) {
		if (strncmp("cpuclock", *env, strlen("cpuclock")) == 0) {
			cpu_clock = simple_strtol(*env + strlen("cpuclock="),
						NULL, 10);
		}
		env++;
	}

	mips_machgroup = MACH_GROUP_PMC;
	mips_machtype = MACH_PMC_BIG_SUR;
}

void __init prom_free_prom_memory(void)
{
}

void __init prom_fixup_mem_map(unsigned long start, unsigned long end)
{
}
	
