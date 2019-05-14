/*
 *  static.c
 *
 *  static idmapping functions for gss principals.
 *
 *  Copyright (c) 2008 David Härdeman <david@hardeman.nu>.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the University nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 *  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 *  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <err.h>

#include "conffile.h"
#include "nfsidmap.h"
#include "nfsidmap_plugin.h"

/*
 * Static Translation Methods
 *
 * These functions use getpwnam to find uid/gid(s) for gss principals
 * which are first mapped to local user names using static mappings
 * in idmapd.conf.
 */

struct pwbuf {
	struct passwd pwbuf;
	char buf[1];
};

struct grbuf {
	struct group grbuf;
	char buf[1];
};

struct uid_mapping {
	LIST_ENTRY (uid_mapping) link;
	uid_t uid;
	char * principal;
	char * localname;
};

struct gid_mapping {
	LIST_ENTRY (gid_mapping) link;
	gid_t gid;
	char * principal;
	char * localgroup;
};

static __inline__ u_int8_t uid_hash (uid_t uid)
{
	return uid % 256;
}

static __inline__ u_int8_t gid_hash (gid_t gid)
{
	return gid % 256;
}

//Hash tables of uid and guids to principals mappings.
//We reuse some queue/hash functions from cfg.c.
LIST_HEAD (uid_mappings, uid_mapping) uid_mappings[256];
LIST_HEAD (gid_mappings, gid_mapping) gid_mappings[256];

static struct passwd *static_getpwnam(const char *name,
				      const char *UNUSED(domain),
				      int *err_p)
{
	struct passwd *pw;
	struct pwbuf *buf;
	size_t buflen = sysconf(_SC_GETPW_R_SIZE_MAX);
	char *localname;
	int err;

	buf = malloc(sizeof(*buf) + buflen);
	if (!buf) {
		err = ENOMEM;
		goto err;
	}

	localname = conf_get_str("Static", (char *)name);
	if (!localname) {
		err = ENOENT;
		goto err_free_buf;
	}

again:
	err = getpwnam_r(localname, &buf->pwbuf, buf->buf, buflen, &pw);

	if (err == EINTR)
		goto again;

	if (!pw) {
		if (err == 0)
			err = ENOENT;

		IDMAP_LOG(0, ("static_getpwnam: localname '%s' for '%s' not found",
		  localname, name));

		goto err_free_buf;
	}

	IDMAP_LOG(4, ("static_getpwnam: name '%s' mapped to '%s'",
		  name, localname));

	*err_p = 0;
	return pw;

err_free_buf:
	free(buf);
err:
	*err_p = err;
	return NULL;
}

static struct group *static_getgrnam(const char *name,
				     const char *UNUSED(domain),
				     int *err_p)
{
	struct group *gr;
	struct grbuf *buf;
	size_t buflen = sysconf(_SC_GETGR_R_SIZE_MAX);
	char *localgroup;
	int err;

	buf = malloc(sizeof(*buf) + buflen);
	if (!buf) {
		err = ENOMEM;
		goto err;
	}

	localgroup = conf_get_str("Static", (char *)name);
	if (!localgroup) {
		err = ENOENT;
		goto err_free_buf;
	}

again:
	err = getgrnam_r(localgroup, &buf->grbuf, buf->buf, buflen, &gr);

	if (err == EINTR)
		goto again;

	if (!gr) {
		if (err == 0)
			err = ENOENT;

		IDMAP_LOG(0, ("static_getgrnam: local group '%s' for '%s' not found",
			  localgroup, name));

		goto err_free_buf;
	}

	IDMAP_LOG(4, ("static_getgrnam: group '%s' mapped to '%s'",
		  name, localgroup));

	*err_p = 0;
	return gr;

err_free_buf:
	free(buf);
err:
	*err_p = err;
	return NULL;
}

static int static_gss_princ_to_ids(char *secname, char *princ,
				   uid_t *uid, uid_t *gid,
				   extra_mapping_params **UNUSED(ex))
{
	struct passwd *pw;
	int err;

	/* XXX: Is this necessary? */
	if (strcmp(secname, "krb5") != 0 && strcmp(secname, "spkm3") != 0)
		return -EINVAL;

	pw = static_getpwnam(princ, NULL, &err);

	if (pw) {
		*uid = pw->pw_uid;
		*gid = pw->pw_gid;
		free(pw);
	}

	return -err;
}

static int static_gss_princ_to_grouplist(char *secname, char *princ,
					 gid_t *groups, int *ngroups,
					 extra_mapping_params **UNUSED(ex))
{
	struct passwd *pw;
	int err;

	/* XXX: Is this necessary? */
	if (strcmp(secname, "krb5") != 0 && strcmp(secname, "spkm3") != 0)
		return -EINVAL;

	pw = static_getpwnam(princ, NULL, &err);

	if (pw) {
		if (getgrouplist(pw->pw_name, pw->pw_gid, groups, ngroups) < 0)
			err = -ERANGE;
		free(pw);
	}

	return -err;
}

static int static_name_to_uid(char *name, uid_t *uid)
{
	struct passwd *pw;
	int err;

	pw = static_getpwnam(name, NULL, &err);

	if (pw) {
		*uid = pw->pw_uid;
		free(pw);
	}

	return -err;
}

static int static_name_to_gid(char *name, gid_t *gid)
{
	struct group *gr;
	int err;

	gr = static_getgrnam(name, NULL, &err);

	if (gr) {
		*gid = gr->gr_gid;
		free(gr);
	}

	return -err;
}

static int static_uid_to_name(uid_t uid, char *UNUSED(domain), char *name, size_t UNUSED(len))
{
	struct uid_mapping * um;

	for (um = LIST_FIRST (&uid_mappings[uid_hash (uid)]); um;
		um = LIST_NEXT (um, link)) {
		if (um->uid == uid) {
			strcpy(name, um->principal);
			return 0;
		}
	}

	return -ENOENT;
}

static int static_gid_to_name(gid_t gid, char *UNUSED(domain), char *name, size_t UNUSED(len))
{
	struct gid_mapping * gm;

	for (gm = LIST_FIRST (&gid_mappings[gid_hash (gid)]); gm;
		gm = LIST_NEXT (gm, link)) {
		if (gm->gid == gid) {
			strcpy(name, gm->principal);
			return 0;
		}
	}

	return -ENOENT;
}

/*
 * We buffer all UID's for which static mappings is defined in advance, so the
 * uid_to_name functions will be fast enough.
 */

static int static_init(void) {	
	int err;
	struct conf_list * princ_list = NULL;
	struct conf_list_node * cln, *next;
	struct uid_mapping * unode;
	struct gid_mapping * gnode;
	struct passwd * pw = NULL;
	struct group * gr = NULL;
	unsigned int i;

	//init hash_table first
	for (i = 0; i < sizeof uid_mappings / sizeof uid_mappings[0]; i++)
		LIST_INIT (&uid_mappings[i]);

	if (nfsidmap_conf_path)
		conf_init_file(nfsidmap_conf_path);

	//get all principals for which we have mappings
	princ_list = conf_get_tag_list("Static", NULL);

	if (!princ_list) {
		return -ENOENT;
	}

	/* As we can not distinguish between mappings for users and groups, we try to
	 * resolve all mappings for both cases.
	 */

	//resolve uid of localname account for all such principals and cache it
	for (cln = TAILQ_FIRST (&princ_list->fields); cln; cln = next) 
	{ 
		next = TAILQ_NEXT (cln, link); 

		pw = static_getpwnam(cln->field, NULL, &err);
		if (!pw) {
			continue;
		}
		
		unode = calloc (1, sizeof *unode);
		if (!unode)
		{
			warnx("static_init: calloc (1, %lu) failed",
				(unsigned long)sizeof *unode);
			free(pw);
			conf_free_list(princ_list);
			return -ENOMEM;
		}
		unode->uid = pw->pw_uid;
		unode->principal = strdup(cln->field);

		unode->localname = conf_get_str("Static", cln->field);
		if (!unode->localname) {
			free(pw);
			free(unode->principal);
			free(unode);
			conf_free_list(princ_list);
			return -ENOENT;
		}

		free(pw);

		LIST_INSERT_HEAD (&uid_mappings[uid_hash(unode->uid)], unode, link);
	}

	//resolve gid of localgroup accounts and cache it
	for (cln = TAILQ_FIRST (&princ_list->fields); cln; cln = next) 
	{ 
		next = TAILQ_NEXT (cln, link); 

		gr = static_getgrnam(cln->field, NULL, &err);
		if (!gr) {
			continue;
		}
		
		gnode = calloc (1, sizeof *gnode);
		if (!gnode)
		{
			warnx("static_init: calloc (1, %lu) failed",
				(unsigned long)sizeof *gnode);
			free(gr);
			conf_free_list(princ_list);
			return -ENOMEM;
		}
		gnode->gid = gr->gr_gid;
		gnode->principal = strdup(cln->field);

		gnode->localgroup = conf_get_str("Static", cln->field);
		if (!gnode->localgroup) {
			free(gr);
			free(gnode->principal);
			free(gnode);
			conf_free_list(princ_list);
			return -ENOENT;
		}

		free(gr);

		LIST_INSERT_HEAD (&gid_mappings[gid_hash(gnode->gid)], gnode, link);
	}
	
	conf_free_list(princ_list);
	return 0;
}


struct trans_func static_trans = {
	.name			= "static",
	.init			= static_init,
	.name_to_uid		= static_name_to_uid,
	.name_to_gid		= static_name_to_gid,
	.uid_to_name		= static_uid_to_name,
	.gid_to_name		= static_gid_to_name,
	.princ_to_ids		= static_gss_princ_to_ids,
	.gss_princ_to_grouplist	= static_gss_princ_to_grouplist,
};

struct trans_func *libnfsidmap_plugin_init(void)
{
	return (&static_trans);
}

