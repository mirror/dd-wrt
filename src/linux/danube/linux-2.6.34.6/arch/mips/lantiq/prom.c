/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 * Copyright (C) 2010 John Crispin <blogic@openwrt.org>
 */

#include <linux/module.h>
#include <linux/clk.h>
#include <asm/bootinfo.h>
#include <asm/time.h>

#include <lantiq.h>

#include "prom.h"

static struct lq_soc_info soc_info;

/* for Multithreading (APRP) on MIPS34K */
unsigned long physical_memsize;

/* all access to the ebu must be locked */
DEFINE_SPINLOCK(ebu_lock);
EXPORT_SYMBOL_GPL(ebu_lock);

extern void clk_init(void);

unsigned int
lq_get_cpu_ver(void)
{
	return soc_info.rev;
}
EXPORT_SYMBOL(lq_get_cpu_ver);

unsigned int
lq_get_soc_type(void)
{
	return soc_info.type;
}
EXPORT_SYMBOL(lq_get_soc_type);

const char*
get_system_type(void)
{
	return soc_info.sys_type;
}

void
prom_free_prom_memory(void)
{
}

#ifdef CONFIG_IMAGE_CMDLINE_HACK
extern char __image_cmdline[];

static void __init
prom_init_image_cmdline(void)
{
	char *p = __image_cmdline;
	int replace = 0;

	if (*p == '-') {
		replace = 1;
		p++;
	}

	if (*p == '\0')
		return;

	if (replace) {
		strlcpy(arcs_cmdline, p, sizeof(arcs_cmdline));
	} else {
		strlcat(arcs_cmdline, " ", sizeof(arcs_cmdline));
		strlcat(arcs_cmdline, p, sizeof(arcs_cmdline));
	}
}
#else
static void __init prom_init_image_cmdline(void) { return; }
#endif

static void __init
prom_init_cmdline(void)
{
	int argc = fw_arg0;
	char **argv = (char**)KSEG1ADDR(fw_arg1);
	int i;

	arcs_cmdline[0] = '\0';
	if(argc)
		for (i = 1; i < argc; i++)
		{
			strlcat(arcs_cmdline, (char*)KSEG1ADDR(argv[i]), COMMAND_LINE_SIZE);
			if(i + 1 != argc)
				strlcat(arcs_cmdline, " ", COMMAND_LINE_SIZE);
		}

	if (!*arcs_cmdline)
		strcpy(&(arcs_cmdline[0]),
			"console=ttyS1,115200 rootfstype=squashfs,jffs2");
	prom_init_image_cmdline();
}

void __init
prom_init(void)
{
	struct clk *clk;
	lq_soc_detect(&soc_info);

	clk_init();
	clk = clk_get(0, "cpu");
	snprintf(soc_info.sys_type, LQ_SYS_TYPE_LEN - 1, "%s r1.%d",
		soc_info.name, soc_info.rev);
	soc_info.sys_type[LQ_SYS_TYPE_LEN - 1] = '\0';
	printk("SoC: %s\n", soc_info.sys_type);

	prom_init_cmdline();
}
