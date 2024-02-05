// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include "init.h"
#include "addr.h"
#include "command.h"
#include "type.h"
#include "io.h"
#include "output.h"
#include "hash.h"
#include "obfuscate.h"
#include <sys/xattr.h>

static int hash_f(int argc, char **argv);
static void hash_help(void);

static const cmdinfo_t hash_cmd = {
	.name		= "hash",
	.cfunc		= hash_f,
	.argmin		= 1,
	.argmax		= -1,
	.args		= N_("string"),
	.oneline	= N_("calculate hash value"),
	.help		= hash_help,
};

static void
hash_help(void)
{
	dbprintf(_(
"\n"
" 'hash' prints out the calculated hash value for a string using the\n"
"directory/attribute code hash function.\n"
"\n"
" Usage:  \"hash <string>\"\n"
"\n"
));

}

/* ARGSUSED */
static int
hash_f(
	int		argc,
	char		**argv)
{
	xfs_dahash_t	hashval;
	bool		use_dir2_hash = false;
	int		c;

	while ((c = getopt(argc, argv, "d")) != EOF) {
		switch (c) {
		case 'd':
			use_dir2_hash = true;
			break;
		default:
			exitcode = 1;
			hash_help();
			return 0;
		}
	}

	for (c = optind; c < argc; c++) {
		if (use_dir2_hash) {
			struct xfs_name	xname = {
				.name	= (uint8_t *)argv[c],
				.len	= strlen(argv[c]),
			};

			hashval = libxfs_dir2_hashname(mp, &xname);
		} else {
			hashval = libxfs_da_hashname(argv[c], strlen(argv[c]));
		}
		dbprintf("0x%x\n", hashval);
	}

	return 0;
}

static void
hashcoll_help(void)
{
	printf(_(
"\n"
" Generate obfuscated variants of the provided name.  Each variant will have\n"
" the same dahash value.  Names are written to stdout with a NULL separating\n"
" each name.\n"
"\n"
" -a -- create extended attributes.\n"
" -i -- read standard input for the name, up to %d bytes.\n"
" -n -- create this many names.\n"
" -p -- create directory entries or extended attributes in this file.\n"
" -s -- seed the rng with this value.\n"
"\n"),
			MAXNAMELEN - 1);
}

struct name_dup {
	struct name_dup	*next;
	uint32_t	crc;
	uint8_t		namelen;
	uint8_t		name[];
};

static inline size_t
name_dup_sizeof(
	unsigned int	namelen)
{
	return sizeof(struct name_dup) + namelen;
}

#define MAX_DUP_TABLE_BUCKETS	(1048575)

struct dup_table {
	unsigned int	nr_buckets;
	struct name_dup	*buckets[];
};

static inline size_t
dup_table_sizeof(
	unsigned int	nr_buckets)
{
	return sizeof(struct dup_table) +
				(nr_buckets * sizeof(struct name_dup *));
}

static int
dup_table_alloc(
	unsigned long		nr_names,
	struct dup_table	**tabp)
{
	struct dup_table	*t;

	*tabp = NULL;

	if (nr_names == 1)
		return 0;

	nr_names = min(MAX_DUP_TABLE_BUCKETS, nr_names);
	t = calloc(1, dup_table_sizeof(nr_names));
	if (!t)
		return ENOMEM;

	t->nr_buckets = nr_names;
	*tabp = t;
	return 0;
}

static void
dup_table_free(
	struct dup_table	*tab)
{
	struct name_dup		*ent, *next;
	unsigned int		i;

	if (!tab)
		return;

	for (i = 0; i < tab->nr_buckets; i++) {
		ent = tab->buckets[i];

		while (ent) {
			next = ent->next;
			free(ent);
			ent = next;
		}
	}
	free(tab);
}

static struct name_dup *
dup_table_find(
	struct dup_table	*tab,
	unsigned char		*name,
	size_t			namelen)
{
	struct name_dup		*ent;
	uint32_t		crc = crc32c(~0, name, namelen);

	ent = tab->buckets[crc % tab->nr_buckets];
	while (ent) {
		if (ent->crc == crc &&
		    ent->namelen == namelen &&
		    !memcmp(ent->name, name, namelen))
			return ent;

		ent = ent->next;
	}

	return NULL;
}

static int
dup_table_store(
	struct dup_table	*tab,
	unsigned char		*name,
	size_t			namelen)
{
	struct name_dup		*dup;
	uint32_t		seq = 1;

	ASSERT(namelen < MAXNAMELEN);

	while ((dup = dup_table_find(tab, name, namelen)) != NULL) {
		int		ret;

		do {
			ret = find_alternate(namelen, name, seq++);
		} while (ret == 0);
		if (ret < 0)
			return EEXIST;
	}

	dup = malloc(name_dup_sizeof(namelen));
	if (!dup)
		return ENOMEM;

	dup->crc = crc32c(~0, name, namelen);
	dup->namelen = namelen;
	memcpy(dup->name, name, namelen);
	dup->next = tab->buckets[dup->crc % tab->nr_buckets];

	tab->buckets[dup->crc % tab->nr_buckets] = dup;
	return 0;
}

static int
collide_dirents(
	unsigned long		nr,
	const unsigned char	*name,
	size_t			namelen,
	int			fd)
{
	struct xfs_name		dname = {
		.name		= name,
		.len		= namelen,
	};
	unsigned char		direntname[MAXNAMELEN + 1];
	struct dup_table	*tab = NULL;
	xfs_dahash_t		old_hash;
	unsigned long		i;
	int			error = 0;

	old_hash = libxfs_dir2_hashname(mp, &dname);

	if (fd >= 0) {
		int		newfd;

		/*
		 * User passed in a fd, so we'll use the directory to detect
		 * duplicate names.  First create the name that we are passed
		 * in; the new names will be hardlinks to the first file.
		 */
		newfd = openat(fd, name, O_CREAT, 0600);
		if (newfd < 0)
			return errno;
		close(newfd);
	} else if (nr > 1) {
		/*
		 * Track every name we create so that we don't emit duplicates.
		 */
		error = dup_table_alloc(nr, &tab);
		if (error)
			return error;
	}

	dname.name = direntname;
	for (i = 0; i < nr; i++) {
		strncpy(direntname, name, MAXNAMELEN);
		obfuscate_name(old_hash, namelen, direntname, true);
		ASSERT(old_hash == libxfs_dir2_hashname(mp, &dname));

		if (fd >= 0) {
			error = linkat(fd, name, fd, direntname, 0);
			if (error && errno != EEXIST)
				return errno;

			/* don't print names to stdout */
			continue;
		} else if (tab) {
			error = dup_table_store(tab, direntname, namelen);
			if (error)
				break;
		}

		printf("%s%c", direntname, 0);
	}

	dup_table_free(tab);
	return error;
}

static int
collide_xattrs(
	unsigned long		nr,
	const unsigned char	*name,
	size_t			namelen,
	int			fd)
{
	unsigned char		xattrname[MAXNAMELEN + 5];
	struct dup_table	*tab = NULL;
	xfs_dahash_t		old_hash;
	unsigned long		i;
	int			error;

	old_hash = libxfs_da_hashname(name, namelen);

	if (fd >= 0) {
		/*
		 * User passed in a fd, so we'll use the xattr structure to
		 * detect duplicate names.  First create the attribute that we
		 * are passed in.
		 */
		snprintf(xattrname, MAXNAMELEN + 5, "user.%s", name);
		error = fsetxattr(fd, xattrname, "1", 1, 0);
		if (error)
			return errno;
	} else if (nr > 1) {
		/*
		 * Track every name we create so that we don't emit duplicates.
		 */
		error = dup_table_alloc(nr, &tab);
		if (error)
			return error;
	}

	for (i = 0; i < nr; i++) {
		snprintf(xattrname, MAXNAMELEN + 5, "user.%s", name);
		obfuscate_name(old_hash, namelen, xattrname + 5, false);
		ASSERT(old_hash == libxfs_da_hashname(xattrname + 5, namelen));

		if (fd >= 0) {
			error = fsetxattr(fd, xattrname, "1", 1, 0);
			if (error)
				return errno;

			/* don't print names to stdout */
			continue;
		} else if (tab) {
			error = dup_table_store(tab, xattrname, namelen + 5);
			if (error)
				break;
		}

		printf("%s%c", xattrname, 0);
	}

	dup_table_free(tab);
	return error;
}

static int
hashcoll_f(
	int		argc,
	char		**argv)
{
	const char	*path = NULL;
	bool		read_stdin = false;
	bool		create_xattr = false;
	unsigned long	nr = 1, seed = 0;
	int		fd = -1;
	int		c;
	int		error;

	while ((c = getopt(argc, argv, "ain:p:s:")) != EOF) {
		switch (c) {
		case 'a':
			create_xattr = true;
			break;
		case 'i':
			read_stdin = true;
			break;
		case 'n':
			nr = strtoul(optarg, NULL, 10);
			break;
		case 'p':
			path = optarg;
			break;
		case 's':
			seed = strtoul(optarg, NULL, 10);
			break;
		default:
			exitcode = 1;
			hashcoll_help();
			return 0;
		}
	}

	if (path) {
		int	oflags = O_RDWR;

		if (!create_xattr)
			oflags = O_RDONLY | O_DIRECTORY;

		fd = open(path, oflags);
		if (fd < 0) {
			perror(path);
			exitcode = 1;
			return 0;
		}
	}

	if (seed)
		srandom(seed);

	if (read_stdin) {
		char	buf[MAXNAMELEN];
		size_t	len;

		len = fread(buf, 1, MAXNAMELEN - 1, stdin);

		if (create_xattr)
			error = collide_xattrs(nr, buf, len, fd);
		else
			error = collide_dirents(nr, buf, len, fd);
		if (error) {
			printf(_("hashcoll: %s\n"), strerror(error));
			exitcode = 1;
		}
		goto done;
	}

	for (c = optind; c < argc; c++) {
		size_t	len = strlen(argv[c]);

		if (create_xattr)
			error = collide_xattrs(nr, argv[c], len, fd);
		else
			error = collide_dirents(nr, argv[c], len, fd);
		if (error) {
			printf(_("hashcoll: %s\n"), strerror(error));
			exitcode = 1;
		}
	}

done:
	if (fd >= 0)
		close(fd);
	return 0;
}

static cmdinfo_t	hashcoll_cmd = {
	.name		= "hashcoll",
	.cfunc		= hashcoll_f,
	.argmin		= 0,
	.argmax		= -1,
	.args		= N_("[-a] [-s seed] [-n nr] [-p path] -i|names..."),
	.oneline	= N_("create names that produce dahash collisions"),
	.help		= hashcoll_help,
};

void
hash_init(void)
{
	add_command(&hash_cmd);
	add_command(&hashcoll_cmd);
}
