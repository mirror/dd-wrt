/*
 *  Copyright (C) 2008-2009 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */
#include <linux/mm.h>

#include <asm/mips_machine.h>
#include <asm/bootinfo.h>

static struct list_head mips_machines __initdata =
		LIST_HEAD_INIT(mips_machines);

char *mips_machine_name = "Unknown";

static struct mips_machine * __init mips_machine_find(unsigned long machtype)
{
	struct list_head *this;

	list_for_each(this, &mips_machines) {
		struct mips_machine *mach;

		mach = list_entry(this, struct mips_machine, list);
		if (mach->mach_type == machtype)
			return mach;
	}

	return NULL;
}

void __init mips_machine_register(struct mips_machine *mach)
{
	list_add_tail(&mach->list, &mips_machines);
}

void __init mips_machine_set_name(char *name)
{
	unsigned int len;
	char *p;

	if (name == NULL)
		return;

	len = strlen(name);
	p = kmalloc(len + 1, GFP_KERNEL);
	if (p) {
		strncpy(p, name, len);
		p[len] = '\0';
		mips_machine_name = p;
	} else {
		printk(KERN_WARNING "MIPS: no memory for machine_name\n");
	}
}

void __init mips_machine_setup(unsigned long machtype)
{
	struct mips_machine *mach;

	mach = mips_machine_find(machtype);
	if (!mach) {
		printk(KERN_ALERT "MIPS: no machine registered for "
			"machtype %lu\n", machtype);
		return;
	}

	mips_machine_set_name(mach->mach_name);
	printk(KERN_INFO "MIPS: machine is %s\n", mips_machine_name);

	if (mach->mach_setup)
		mach->mach_setup();
}
