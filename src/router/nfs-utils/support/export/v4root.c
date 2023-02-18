/*
 * Copyright (C) 2009 Red Hat <nfs@redhat.com>
 *
 * support/export/v4root.c
 *
 * Routines used to support NFSv4 pseudo roots
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <unistd.h>
#include <errno.h>
#include <uuid/uuid.h>

#include "xlog.h"
#include "exportfs.h"
#include "nfslib.h"
#include "misc.h"
#include "v4root.h"
#include "pseudoflavors.h"

static nfs_export pseudo_root = {
	.m_next = NULL,
	.m_client = NULL,
	.m_export = {
		.e_hostname = "*",
		.e_path = "/",
		.e_flags = NFSEXP_READONLY
				| NFSEXP_NOSUBTREECHECK | NFSEXP_FSID
				| NFSEXP_V4ROOT | NFSEXP_INSECURE_PORT,
		.e_anonuid = 65534,
		.e_anongid = 65534,
		.e_squids = NULL,
		.e_nsquids = 0,
		.e_sqgids = NULL,
		.e_nsqgids = 0,
		.e_fsid = 0,
		.e_mountpoint = NULL,
		.e_ttl = 0,
	},
	.m_exported = 0,
	.m_xtabent = 1,
	.m_mayexport = 1,
	.m_changed = 0,
	.m_warned = 0,
};

static void
set_pseudofs_security(struct exportent *pseudo)
{
	struct flav_info *flav;
	int i;

	for (flav = flav_map; flav < flav_map + flav_map_size; flav++) {
		struct sec_entry *new;

		if (!flav->fnum)
			continue;

		i = secinfo_addflavor(flav, pseudo);
		new = &pseudo->e_secinfo[i];

		new->flags |= NFSEXP_INSECURE_PORT;
	}
}

/*
 * Create a pseudo export
 */
static struct exportent *
v4root_create(char *path, nfs_export *export)
{
	nfs_export *exp;
	struct exportent eep;
	struct exportent *curexp = &export->m_export;

	dupexportent(&eep, &pseudo_root.m_export);
	eep.e_ttl = default_ttl;
	eep.e_hostname = curexp->e_hostname;
	strncpy(eep.e_path, path, sizeof(eep.e_path)-1);
	if (strcmp(path, "/") != 0)
		eep.e_flags &= ~NFSEXP_FSID;

	if (strcmp(path, "/") != 0 &&
	    !export_test(&eep, 0)) {
		/* Need a uuid - base it on path using a fixed seed that
		 * was generated randomly.
		 */
		const char seed_s[] = "39c6b5c1-3f24-4f4e-977c-7fe6546b8a25";
		uuid_t seed, uuid;
		char uuid_s[UUID_STR_LEN];
		unsigned int i, j;

		uuid_parse(seed_s, seed);
		uuid_generate_sha1(uuid, seed, path, strlen(path));
		uuid_unparse_upper(uuid, uuid_s);
		/* strip hyhens */
		for (i = j = 0; uuid_s[i]; i++)
			if (uuid_s[i] != '-')
				uuid_s[j++] = uuid_s[i];
		eep.e_uuid = uuid_s;
	}
	set_pseudofs_security(&eep);
	exp = export_create(&eep, 0);
	if (exp == NULL)
		return NULL;
	xlog(D_CALL, "v4root_create: path '%s' flags 0x%x",
		exp->m_export.e_path, exp->m_export.e_flags);
	return &exp->m_export;
}

/*
 * Make sure the kernel has pseudo root support.
 */
static int
v4root_support(void)
{
	struct export_features *ef;
	static int warned = 0;

	ef = get_export_features();

	if (ef->flags & NFSEXP_V4ROOT)
		return 1;
	if (!warned) {
		xlog(L_WARNING, "Kernel does not have pseudo root support.");
		xlog(L_WARNING, "NFS v4 mounts will be disabled unless fsid=0");
		xlog(L_WARNING, "is specfied in /etc/exports file.");
		warned++;
	}
	return 0;
}

static int
pseudofs_update(char *hostname, char *path, nfs_export *source)
{
	nfs_export *exp;

	exp = export_lookup(hostname, path, 0);
	if (exp && !(exp->m_export.e_flags & NFSEXP_V4ROOT))
		return 0;
	if (!exp) {
		if (v4root_create(path, source) == NULL) {
			xlog(L_WARNING, "v4root_set: Unable to create "
					"pseudo export for '%s'", path);
			return -ENOMEM;
		}
		return 0;
	}
	/* Update an existing V4ROOT export: */
	set_pseudofs_security(&exp->m_export);
	return 0;
}

static int v4root_add_parents(nfs_export *exp)
{
	char *hostname = exp->m_export.e_hostname;
	char *path;
	char *ptr;
	int ret = 0;

	path = strdup(exp->m_export.e_path);
	if (!path) {
		xlog(L_WARNING, "v4root_add_parents: Unable to create "
				"pseudo export for '%s'", exp->m_export.e_path);
		return -ENOMEM;
	}
	for (ptr = path; ptr; ptr = strchr(ptr, '/')) {
		char saved;

		saved = *ptr;
		*ptr = '\0';
		ret = pseudofs_update(hostname, *path ? path : "/", exp);
		if (ret)
			break;
		*ptr = saved;
		ptr++;
	}
	free(path);
	return ret;
}

/*
 * Create pseudo exports by running through the real export
 * looking at the components of the path that make up the export.
 * Those path components, if not exported, will become pseudo
 * exports allowing them to be found when the kernel does an upcall
 * looking for components of the v4 mount.
 */
void
v4root_set()
{
	nfs_export	*exp;
	int	i;

	if (!v4root_needed)
		return;
	if (!v4root_support())
		return;

	for (i = 0; i < MCL_MAXTYPES; i++) {
		for (exp = exportlist[i].p_head; exp; exp = exp->m_next) {
			if (exp->m_export.e_flags & NFSEXP_V4ROOT)
				/*
				 * We just added this one, so its
				 * parents are already dealt with!
				 */
				continue;

			if (strcmp(exp->m_export.e_path, "/") == 0 &&
			    !(exp->m_export.e_flags & NFSEXP_FSID)) {
				/* Force '/' to be exported as fsid == 0*/
				exp->m_export.e_flags |= NFSEXP_FSID;
				exp->m_export.e_fsid = 0;
			}

			v4root_add_parents(exp);
			/* XXX: error handling! */
		}
	}
}
