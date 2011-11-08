/*
 * Copyright (c) 2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <xfs/libxfs.h>
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

static int		attr_set_f(int argc, char **argv);
static int		attr_remove_f(int argc, char **argv);
static void		attrset_help(void);

static const cmdinfo_t	attr_set_cmd =
	{ "attr_set", "aset", attr_set_f, 1, -1, 0,
	  N_("[-r|-s|-p|-u] [-n] [-R|-C] [-v n] name"),
	  N_("set the named attribute on the current inode"), attrset_help };
static const cmdinfo_t	attr_remove_cmd =
	{ "attr_remove", "aremove", attr_remove_f, 1, -1, 0,
	  N_("[-r|-s|-p|-u] [-n] name"),
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
	int		argc,
	char		**argv)
{
	xfs_inode_t	*ip = NULL;
	char		*name, *value, *sp;
	int		c, valuelen = 0, flags = 0;

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
			flags |= LIBXFS_ATTR_ROOT;
			flags &= ~LIBXFS_ATTR_SECURE;
			break;
		case 'u':
			flags &= ~(LIBXFS_ATTR_ROOT | LIBXFS_ATTR_SECURE);
			break;
		case 's':
			flags |= LIBXFS_ATTR_SECURE;
			flags &= ~LIBXFS_ATTR_ROOT;
			break;

		/* modifiers */
		case 'C':
			flags |= LIBXFS_ATTR_CREATE;
			break;
		case 'R':
			flags |= LIBXFS_ATTR_REPLACE;
			break;

		case 'n':
			mp->m_flags |= LIBXFS_MOUNT_COMPAT_ATTR;
			break;

		/* value length */
		case 'v':
			valuelen = (int)strtol(optarg, &sp, 0);
			if (*sp != '\0' || valuelen < 0 || valuelen > 64*1024) {
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

	name = argv[optind];

	if (valuelen) {
		value = (char *)memalign(getpagesize(), valuelen);
		if (!value) {
			dbprintf(_("cannot allocate buffer (%d)\n"), valuelen);
			goto out;
		}
		memset(value, 0xfeedface, valuelen);
	} else {
		value = NULL;
	}

	if (libxfs_iget(mp, NULL, iocur_top->ino, 0, &ip, 0)) {
		dbprintf(_("failed to iget inode %llu\n"),
			(unsigned long long)iocur_top->ino);
		goto out;
	}

	if (libxfs_attr_set(ip, (unsigned char *)name,
				(unsigned char *)value, valuelen, flags)) {
		dbprintf(_("failed to set attr %s on inode %llu\n"),
			name, (unsigned long long)iocur_top->ino);
		goto out;
	}

	/* refresh with updated inode contents */
	set_cur_inode(iocur_top->ino);

out:
	mp->m_flags &= ~LIBXFS_MOUNT_COMPAT_ATTR;
	if (ip)
		libxfs_iput(ip, 0);
	if (value)
		free(value);
	return 0;
}

static int
attr_remove_f(
	int		argc,
	char		**argv)
{
	xfs_inode_t	*ip = NULL;
	char		*name;
	int		c, flags = 0;

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
			flags |= LIBXFS_ATTR_ROOT;
			flags &= ~LIBXFS_ATTR_SECURE;
			break;
		case 'u':
			flags &= ~(LIBXFS_ATTR_ROOT | LIBXFS_ATTR_SECURE);
			break;
		case 's':
			flags |= LIBXFS_ATTR_SECURE;
			flags &= ~LIBXFS_ATTR_ROOT;
			break;

		case 'n':
			mp->m_flags |= LIBXFS_MOUNT_COMPAT_ATTR;
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

	name = argv[optind];

	if (libxfs_iget(mp, NULL, iocur_top->ino, 0, &ip, 0)) {
		dbprintf(_("failed to iget inode %llu\n"),
			(unsigned long long)iocur_top->ino);
		goto out;
	}

	if (libxfs_attr_remove(ip, (unsigned char *)name, flags)) {
		dbprintf(_("failed to remove attr %s from inode %llu\n"),
			name, (unsigned long long)iocur_top->ino);
		goto out;
	}

	/* refresh with updated inode contents */
	set_cur_inode(iocur_top->ino);

out:
	mp->m_flags &= ~LIBXFS_MOUNT_COMPAT_ATTR;
	if (ip)
		libxfs_iput(ip, 0);
	return 0;
}
