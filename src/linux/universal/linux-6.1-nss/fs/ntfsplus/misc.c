// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * NTFS kernel debug support. Part of the Linux-NTFS project.
 *
 * Copyright (C) 1997 Martin von Löwis, Régis Duchesne
 * Copyright (c) 2001-2005 Anton Altaparmakov
 */

#include <linux/module.h>
#ifdef CONFIG_SYSCTL
#include <linux/proc_fs.h>
#include <linux/sysctl.h>
#endif

#include "misc.h"

/**
 * __ntfs_warning - output a warning to the syslog
 * @function:	name of function outputting the warning
 * @sb:		super block of mounted ntfs filesystem
 * @fmt:	warning string containing format specifications
 * @...:	a variable number of arguments specified in @fmt
 *
 * Outputs a warning to the syslog for the mounted ntfs filesystem described
 * by @sb.
 *
 * @fmt and the corresponding @... is printf style format string containing
 * the warning string and the corresponding format arguments, respectively.
 *
 * @function is the name of the function from which __ntfs_warning is being
 * called.
 *
 * Note, you should be using debug.h::ntfs_warning(@sb, @fmt, @...) instead
 * as this provides the @function parameter automatically.
 */
void __ntfs_warning(const char *function, const struct super_block *sb,
		const char *fmt, ...)
{
	struct va_format vaf;
	va_list args;
	int flen = 0;

	if (function)
		flen = strlen(function);
	va_start(args, fmt);
	vaf.fmt = fmt;
	vaf.va = &args;
#ifndef DEBUG
	if (sb)
		pr_warn_ratelimited("(device %s): %s(): %pV\n",
			sb->s_id, flen ? function : "", &vaf);
	else
		pr_warn_ratelimited("%s(): %pV\n", flen ? function : "", &vaf);
#else
	if (sb)
		pr_warn("(device %s): %s(): %pV\n",
			sb->s_id, flen ? function : "", &vaf);
	else
		pr_warn("%s(): %pV\n", flen ? function : "", &vaf);
#endif
	va_end(args);
}

/**
 * __ntfs_error - output an error to the syslog
 * @function:	name of function outputting the error
 * @sb:		super block of mounted ntfs filesystem
 * @fmt:	error string containing format specifications
 * @...:	a variable number of arguments specified in @fmt
 *
 * Outputs an error to the syslog for the mounted ntfs filesystem described
 * by @sb.
 *
 * @fmt and the corresponding @... is printf style format string containing
 * the error string and the corresponding format arguments, respectively.
 *
 * @function is the name of the function from which __ntfs_error is being
 * called.
 *
 * Note, you should be using debug.h::ntfs_error(@sb, @fmt, @...) instead
 * as this provides the @function parameter automatically.
 */
void __ntfs_error(const char *function, struct super_block *sb,
		const char *fmt, ...)
{
	struct va_format vaf;
	va_list args;
	int flen = 0;

	if (function)
		flen = strlen(function);
	va_start(args, fmt);
	vaf.fmt = fmt;
	vaf.va = &args;
#ifndef DEBUG
	if (sb)
		pr_err_ratelimited("(device %s): %s(): %pV\n",
		       sb->s_id, flen ? function : "", &vaf);
	else
		pr_err_ratelimited("%s(): %pV\n", flen ? function : "", &vaf);
#else
	if (sb)
		pr_err("(device %s): %s(): %pV\n",
		       sb->s_id, flen ? function : "", &vaf);
	else
		pr_err("%s(): %pV\n", flen ? function : "", &vaf);
#endif
	va_end(args);

	if (sb)
		ntfs_handle_error(sb);
}

#ifdef DEBUG

/* If 1, output debug messages, and if 0, don't. */
int debug_msgs;

void __ntfs_debug(const char *file, int line, const char *function,
		const char *fmt, ...)
{
	struct va_format vaf;
	va_list args;
	int flen = 0;

	if (!debug_msgs)
		return;
	if (function)
		flen = strlen(function);
	va_start(args, fmt);
	vaf.fmt = fmt;
	vaf.va = &args;
	pr_debug("(%s, %d): %s(): %pV", file, line, flen ? function : "", &vaf);
	va_end(args);
}

/* Dump a runlist. Caller has to provide synchronisation for @rl. */
void ntfs_debug_dump_runlist(const struct runlist_element *rl)
{
	int i;
	const char *lcn_str[5] = { "LCN_DELALLOC     ", "LCN_HOLE         ",
				   "LCN_RL_NOT_MAPPED", "LCN_ENOENT       ",
				   "LCN_unknown      " };

	if (!debug_msgs)
		return;
	pr_debug("Dumping runlist (values in hex):\n");
	if (!rl) {
		pr_debug("Run list not present.\n");
		return;
	}
	pr_debug("VCN              LCN               Run length\n");
	for (i = 0; ; i++) {
		s64 lcn = (rl + i)->lcn;

		if (lcn < (s64)0) {
			int index = -lcn - 1;

			if (index > -LCN_ENOENT - 1)
				index = 3;
			pr_debug("%-16Lx %s %-16Lx%s\n",
					(long long)(rl + i)->vcn, lcn_str[index],
					(long long)(rl + i)->length,
					(rl + i)->length ? "" :
						" (runlist end)");
		} else
			pr_debug("%-16Lx %-16Lx  %-16Lx%s\n",
					(long long)(rl + i)->vcn,
					(long long)(rl + i)->lcn,
					(long long)(rl + i)->length,
					(rl + i)->length ? "" :
						" (runlist end)");
		if (!(rl + i)->length)
			break;
	}
}

#ifdef CONFIG_SYSCTL
/* Definition of the ntfs sysctl. */
static const struct ctl_table ntfs_sysctls[] = {
	{
		.procname	= "ntfs-debug",
		.data		= &debug_msgs,		/* Data pointer and size. */
		.maxlen		= sizeof(debug_msgs),
		.mode		= 0644,			/* Mode, proc handler. */
		.proc_handler	= proc_dointvec
	},
	{}
};

/* Storage for the sysctls header. */
static struct ctl_table_header *sysctls_root_table;

/**
 * ntfs_sysctl - add or remove the debug sysctl
 * @add:	add (1) or remove (0) the sysctl
 *
 * Add or remove the debug sysctl. Return 0 on success or -errno on error.
 */
int ntfs_sysctl(int add)
{
	if (add) {
		sysctls_root_table = register_sysctl("fs", ntfs_sysctls);
		if (!sysctls_root_table)
			return -ENOMEM;
	} else {
		unregister_sysctl_table(sysctls_root_table);
		sysctls_root_table = NULL;
	}
	return 0;
}
#endif /* CONFIG_SYSCTL */
#endif
