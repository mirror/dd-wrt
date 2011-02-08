/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 * Copyright (C) 2010 John Crispin <blogic@openwrt.org>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/ioport.h>

#include <lantiq.h>

#include <machine.h>

static unsigned int *cp1_base;

void __init
plat_mem_setup(void)
{
	/* assume 16M as default */
	int memsize = 16;
	char **envp = (char **) KSEG1ADDR(fw_arg2);
	u32 status;

	/* make sure to have no "reverse endian" for user mode! */
	status = read_c0_status();
	status &= (~(1<<25));
	write_c0_status(status);

	ioport_resource.start = IOPORT_RESOURCE_START;
	ioport_resource.end = IOPORT_RESOURCE_END;
	iomem_resource.start = IOMEM_RESOURCE_START;
	iomem_resource.end = IOMEM_RESOURCE_END;
	set_io_port_base((unsigned long) KSEG1);

	while (*envp)
	{
		char *e = (char *)KSEG1ADDR(*envp);
		if (!strncmp(e, "memsize=", 8))
		{
			e += 8;
			memsize = simple_strtoul(e, NULL, 10);
		}
		envp++;
	}
//	memsize -= 2;
	memsize *= 1024 * 1024;
//	cp1_base = (unsigned int*)(KSEG1 | memsize);
	add_memory_region(0x00000000, memsize, BOOT_MEM_RAM);
}

unsigned int*
lq_get_cp1_base(void)
{
	return cp1_base;
}
EXPORT_SYMBOL(lq_get_cp1_base);

static int __init
lq_machine_setup(void)
{
	mips_machine_setup();
	return 0;
}

static void __init
mach_generic_init(void)
{
}

MIPS_MACHINE(LANTIQ_MACH_GENERIC,
			"Generic",
			"Generic",
			mach_generic_init);

arch_initcall(lq_machine_setup);

/* for backward compatibility, define "board=" as alias for "machtype=" */
__setup("board=", mips_machtype_setup);
