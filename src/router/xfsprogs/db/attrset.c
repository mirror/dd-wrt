// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include "command.h"
#include "attrset.h"
#include "io.h"
#include "output.h"
#include "type.h"
#include "init.h"
#include "fprint.h"
#include "faddr.h"
#include "field.h"
#include "inode.h"
#include "malloc.h"
#include <sys/xattr.h>

static int		attr_set_f(int argc, char **argv);
static int		attr_remove_f(int argc, char **argv);
static void		attrset_help(void);

static const cmdinfo_t	attr_set_cmd =
	{ "attr_set", "aset", attr_set_f, 1, -1, 0,
	  N_("[-r|-s|-u] [-n] [-R|-C] [-v n] name"),
	  N_("set the named attribute on the current inode"), attrset_help };
static const cmdinfo_t	attr_remove_cmd =
	{ "attr_remove", "aremove", attr_remove_f, 1, -1, 0,
	  N_("[-r|-s|-u] [-n] name"),
	  N_("remove the named attribute from the current inode"), attrset_help };

static void
attrset_help(void)
{
	dbprintf(_(
"\n"
" The 'attr_set' and 'attr_remove' commands provide interfaces for debugging\n"
" the extended attribute allocation and removal code.\n"
" Both commands require an attribute name to be specified, and the attr_set\n"
" command allows an optional value length (-v) to be provided as well.\n"
" There are 4 namespace flags:\n"
"  -r -- 'root'\n"
"  -u -- 'user'		(default)\n"
"  -s -- 'secure'\n"
"\n"
" For attr_set, these options further define the type of set operation:\n"
"  -C -- 'create'    - create attribute, fail if it already exists\n"
"  -R -- 'replace'   - replace attribute, fail if it does not exist\n"
" The backward compatibility mode 'noattr2' can be emulated (-n) also.\n"
"\n"));
}

void
attrset_init(void)
{
	if (!expert_mode)
		return;

	add_command(&attr_set_cmd);
	add_command(&attr_remove_cmd);
}

static int
attr_set_f(
	int			argc,
	char			**argv)
{
	struct xfs_da_args	args = { };
	char			*sp;
	int			c;

	if (cur_typ == NULL) {
		dbprintf(_("no current type\n"));
		return 0;
	}
	if (cur_typ->typnm != TYP_INODE) {
		dbprintf(_("current type is not inode\n"));
		return 0;
	}

	while ((c = getopt(argc, argv, "rusCRnv:")) != EOF) {
		switch (c) {
		/* namespaces */
		case 'r':
			args.attr_filter |= LIBXFS_ATTR_ROOT;
			args.attr_filter &= ~LIBXFS_ATTR_SECURE;
			break;
		case 'u':
			args.attr_filter &= ~(LIBXFS_ATTR_ROOT |
					      LIBXFS_ATTR_SECURE);
			break;
		case 's':
			args.attr_filter |= LIBXFS_ATTR_SECURE;
			args.attr_filter &= ~LIBXFS_ATTR_ROOT;
			break;

		/* modifiers */
		case 'C':
			args.attr_flags |= XATTR_CREATE;
			args.attr_flags &= ~XATTR_REPLACE;
			break;
		case 'R':
			args.attr_flags |= XATTR_REPLACE;
			args.attr_flags &= ~XATTR_CREATE;
			break;

		case 'n':
			/*
			 * We never touch attr2 these days; leave this here to
			 * avoid breaking scripts.
			 */
			break;

		/* value length */
		case 'v':
			args.valuelen = strtol(optarg, &sp, 0);
			if (*sp != '\0' ||
			    args.valuelen < 0 || args.valuelen > 64 * 1024) {
				dbprintf(_("bad attr_set valuelen %s\n"), optarg);
				return 0;
			}
			break;

		default:
			dbprintf(_("bad option for attr_set command\n"));
			return 0;
		}
	}

	if (optind != argc - 1) {
		dbprintf(_("too few options for attr_set (no name given)\n"));
		return 0;
	}

	args.name = (const unsigned char *)argv[optind];
	if (!args.name) {
		dbprintf(_("invalid name\n"));
		return 0;
	}

	args.namelen = strlen(argv[optind]);
	if (args.namelen >= MAXNAMELEN) {
		dbprintf(_("name too long\n"));
		return 0;
	}

	if (args.valuelen) {
		args.value = memalign(getpagesize(), args.valuelen);
		if (!args.value) {
			dbprintf(_("cannot allocate buffer (%d)\n"),
				args.valuelen);
			goto out;
		}
		memset(args.value, 'v', args.valuelen);
	}

	if (libxfs_iget(mp, NULL, iocur_top->ino, 0, &args.dp)) {
		dbprintf(_("failed to iget inode %llu\n"),
			(unsigned long long)iocur_top->ino);
		goto out;
	}

	if (libxfs_attr_set(&args)) {
		dbprintf(_("failed to set attr %s on inode %llu\n"),
			args.name, (unsigned long long)iocur_top->ino);
		goto out;
	}

	/* refresh with updated inode contents */
	set_cur_inode(iocur_top->ino);

out:
	if (args.dp)
		libxfs_irele(args.dp);
	if (args.value)
		free(args.value);
	return 0;
}

static int
attr_remove_f(
	int			argc,
	char			**argv)
{
	struct xfs_da_args	args = { };
	int			c;

	if (cur_typ == NULL) {
		dbprintf(_("no current type\n"));
		return 0;
	}
	if (cur_typ->typnm != TYP_INODE) {
		dbprintf(_("current type is not inode\n"));
		return 0;
	}

	while ((c = getopt(argc, argv, "rusn")) != EOF) {
		switch (c) {
		/* namespaces */
		case 'r':
			args.attr_filter |= LIBXFS_ATTR_ROOT;
			args.attr_filter &= ~LIBXFS_ATTR_SECURE;
			break;
		case 'u':
			args.attr_filter &= ~(LIBXFS_ATTR_ROOT |
					      LIBXFS_ATTR_SECURE);
			break;
		case 's':
			args.attr_filter |= LIBXFS_ATTR_SECURE;
			args.attr_filter &= ~LIBXFS_ATTR_ROOT;
			break;

		case 'n':
			/*
			 * We never touch attr2 these days; leave this here to
			 * avoid breaking scripts.
			 */
			break;

		default:
			dbprintf(_("bad option for attr_remove command\n"));
			return 0;
		}
	}

	if (optind != argc - 1) {
		dbprintf(_("too few options for attr_remove (no name given)\n"));
		return 0;
	}

	args.name = (const unsigned char *)argv[optind];
	if (!args.name) {
		dbprintf(_("invalid name\n"));
		return 0;
	}

	args.namelen = strlen(argv[optind]);
	if (args.namelen >= MAXNAMELEN) {
		dbprintf(_("name too long\n"));
		return 0;
	}

	if (libxfs_iget(mp, NULL, iocur_top->ino, 0, &args.dp)) {
		dbprintf(_("failed to iget inode %llu\n"),
			(unsigned long long)iocur_top->ino);
		goto out;
	}

	if (libxfs_attr_set(&args)) {
		dbprintf(_("failed to remove attr %s from inode %llu\n"),
			(unsigned char *)args.name,
			(unsigned long long)iocur_top->ino);
		goto out;
	}

	/* refresh with updated inode contents */
	set_cur_inode(iocur_top->ino);

out:
	if (args.dp)
		libxfs_irele(args.dp);
	return 0;
}
