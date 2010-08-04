/*
 *  Copyright (C) 2008-2009 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#ifndef __ASM_MIPS_MACHINE_H
#define __ASM_MIPS_MACHINE_H

#include <linux/init.h>
#include <linux/list.h>

struct mips_machine {
	unsigned long		mach_type;
	void			(*mach_setup)(void);
	char			*mach_name;
	struct list_head	list;
};

void mips_machine_register(struct mips_machine *) __init;
void mips_machine_setup(unsigned long machtype) __init;
void mips_machine_set_name(char *name) __init;

extern char *mips_machine_name;

#define MIPS_MACHINE(_type, _name, _setup) 			\
static char machine_name_##_type[] __initdata = _name;		\
static struct mips_machine machine_##_type __initdata =		\
{								\
	.mach_type	= _type,				\
	.mach_name	= machine_name_##_type,			\
	.mach_setup	= _setup,				\
};								\
								\
static int __init register_machine_##_type(void)		\
{								\
	mips_machine_register(&machine_##_type);		\
	return 0;						\
}								\
								\
pure_initcall(register_machine_##_type)

#endif /* __ASM_MIPS_MACHINE_H */

