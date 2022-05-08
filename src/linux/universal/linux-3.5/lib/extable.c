/*
 * Derived from arch/ppc/mm/extable.c and arch/i386/mm/extable.c.
 *
 * Copyright (C) 2004 Paul Mackerras, IBM Corp.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include <linux/bsearch.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sort.h>
#include <asm/uaccess.h>

#define ex_to_insn(x)	((x)->insn)

#ifndef ARCH_HAS_SORT_EXTABLE
/*
 * The exception table needs to be sorted so that the binary
 * search that we use to find entries in it works properly.
 * This is used both for the kernel exception table and for
 * the exception tables of modules that get loaded.
 */
static int cmp_ex_sort(const void *a, const void *b)
{
	const struct exception_table_entry *x = a, *y = b;

	/* avoid overflow */
	if (x->insn > y->insn)
		return 1;
	if (x->insn < y->insn)
		return -1;
	return 0;
}

void sort_extable(struct exception_table_entry *start,
		  struct exception_table_entry *finish)
{
	sort(start, finish - start, sizeof(struct exception_table_entry),
	     cmp_ex_sort, NULL);
}

#ifdef CONFIG_MODULES
/*
 * If the exception table is sorted, any referring to the module init
 * will be at the beginning or the end.
 */
void trim_init_extable(struct module *m)
{
	/*trim the beginning*/
	while (m->num_exentries && within_module_init(m->extable[0].insn, m)) {
		m->extable++;
		m->num_exentries--;
	}
	/*trim the end*/
	while (m->num_exentries &&
		within_module_init(m->extable[m->num_exentries-1].insn, m))
		m->num_exentries--;
}
#endif /* CONFIG_MODULES */
#endif /* !ARCH_HAS_SORT_EXTABLE */

#ifndef ARCH_HAS_SEARCH_EXTABLE

static int cmp_ex_search(const void *key, const void *elt)
{
	const struct exception_table_entry *_elt = elt;
	unsigned long _key = *(unsigned long *)key;

	/* avoid overflow */
	if (_key > ex_to_insn(_elt))
		return 1;
	if (_key < ex_to_insn(_elt))
		return -1;
	return 0;
}

/*
 * Search one exception table for an entry corresponding to the
 * given instruction address, and return the address of the entry,
 * or NULL if none is found.
 * We use a binary search, and thus we assume that the table is
 * already sorted.
 */
const struct exception_table_entry *
search_extable(const struct exception_table_entry *base,
	       const size_t num,
	       unsigned long value)
{
	return bsearch(&value, base, num,
		       sizeof(struct exception_table_entry), cmp_ex_search);
}
#endif
