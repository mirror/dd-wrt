/*
 * Copyright 2004 PMC-Sierra Inc.
 * Author : Manish Lachwani (lachwani@pmc-sierra.com)
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 * 
 */
#include <linux/config.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/bootmem.h>

#include <asm/addrspace.h>
#include <asm/bootinfo.h>

/*
 * PMON Callvectors
 */
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

/*
 * Board Type 
 */
const char *get_system_type(void)
{
	return "PMC-Sierra Stretch";
}

#ifdef CONFIG_MIPS64

unsigned long signext(unsigned long addr)
{
  addr &= 0xffffffff;
  return (unsigned long)((int)addr);
}

void *get_arg(unsigned long args, int arc)
{
  unsigned long ul;
  unsigned char *puc, uc;

  args += (arc * 4);
  ul = (unsigned long)signext(args);
  puc = (unsigned char *)ul;
  if (puc == 0)
    return (void *)0;

#ifdef CONFIG_CPU_LITTLE_ENDIAN
  uc = *puc++;
  ul = (unsigned long)uc;
  uc = *puc++;
  ul |= (((unsigned long)uc) << 8);
  uc = *puc++;
  ul |= (((unsigned long)uc) << 16);
  uc = *puc++;
  ul |= (((unsigned long)uc) << 24);
#else  /* CONFIG_CPU_LITTLE_ENDIAN */
  uc = *puc++;
  ul = ((unsigned long)uc) << 24;
  uc = *puc++;
  ul |= (((unsigned long)uc) << 16);
  uc = *puc++;
  ul |= (((unsigned long)uc) << 8);
  uc = *puc++;
  ul |= ((unsigned long)uc);
#endif  /* CONFIG_CPU_LITTLE_ENDIAN */
  ul = signext(ul);
  return (void *)ul;
}

char *arg64(unsigned long addrin, int arg_index)
{
  unsigned long args;
  char *p;
  args = signext(addrin);
  p = (char *)get_arg(args, arg_index);
  return p;
}
#endif  /* CONFIG_MIPS64 */


/* PMON passes arguments in C main() style */
void __init prom_init(int argc, char **arg, char **env, struct callvectors *cv)
{
	int i;
#ifdef CONFIG_MIPS64
	char *ptr;

	printk("prom_init - MIPS64\n");

	/* save the PROM vectors for debugging use */
	debug_vectors = (struct callvectors *)signext((unsigned long)cv);

	/* arg[0] is "g", the rest is boot parameters */
	arcs_cmdline[0] = '\0';

	for (i = 1; i < argc; i++) {
		ptr = (char *)arg64((unsigned long)arg, i);
		if ((strlen(arcs_cmdline) + strlen(ptr) + 1) >=
		    sizeof(arcs_cmdline))
			break;
		strcat(arcs_cmdline, ptr);
		strcat(arcs_cmdline, " ");
	}
	i = 0;

	while (1) {
		ptr = (char *)arg64((unsigned long)env, i);
		if (! ptr)
			break;

		if (strncmp("cpuclock", ptr, strlen("cpuclock")) == 0) {
			cpu_clock = simple_strtol(ptr + strlen("cpuclock="),
							NULL, 10);
			printk("cpu_clock set to %d\n", cpu_clock);
		}
		i++;
	}
	printk("arcs_cmdline: %s\n", arcs_cmdline);

#else   /* CONFIG_MIPS64 */

	/* save the PROM vectors for debugging use */
	debug_vectors = cv;

	/* arg[0] is "g", the rest is boot parameters */
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
#endif /* CONFIG_MIPS64 */

	mips_machgroup = MACH_GROUP_PMC;
	mips_machtype = MACH_PMC_STRETCH;

#ifndef CONFIG_MIPS64

	/* 
	 * This proves that the basic interaction between PMON
	 * and Linux is working. At this point, Linux has taken
	 * control. Note that the early printk patch at this point
	 * is very useful since it directly interacts with the 
	 * serial console. 
	 */
	debug_vectors->printf("Booting Linux kernel...\n");
#endif
}

void __init prom_free_prom_memory(void)
{
	/* Do nothing */
}

void __init prom_fixup_mem_map(unsigned long start, unsigned long end)
{
	/* Do nothing */
}
