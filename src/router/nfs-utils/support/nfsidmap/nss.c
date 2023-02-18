/*
 *  nss.c
 *
 *  nsswitch idmapping functions.
 *
 *  Copyright (c) 2004 The Regents of the University of Michigan.
 *  All rights reserved.
 *
 *  J. Bruce Fields <bfields@umich.edu>
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

#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <netdb.h>
#include <err.h>
#include <grp.h>
#include <limits.h>
#include <ctype.h>
#include "nfsidmap.h"
#include "nfsidmap_plugin.h"
#include "nfsidmap_private.h"
#include <syslog.h>

static char *get_default_domain(void)
{
	static char default_domain[NFS4_MAX_DOMAIN_LEN] = "";
	if (default_domain[0] == 0) {
		nfs4_get_default_domain(NULL, default_domain, NFS4_MAX_DOMAIN_LEN);
	}
	return default_domain;
}

/*
 * NSS Translation Methods
 *
 * These are all just wrappers around getpwnam and friends;
 * we tack on the given domain to the results of getpwnam when looking up a uid,
 * and ignore the domain entirely when looking up a name.
 */

static int write_name(char *dest, char *localname, char *domain, size_t len,
		      int doappend)
{
	if (doappend || !strchr(localname,'@')) {
		if (strlen(localname) + 1 + strlen(domain) + 1 > len)
			return -ENOMEM; /* XXX: Is there an -ETOOLONG? */
		strcpy(dest, localname);
		strcat(dest, "@");
		strcat(dest, domain);
	} else {
		if (strlen(localname) + 1 > len)
			return -ENOMEM;
		strcpy(dest, localname);
	}
	return 0;
}

static int nss_uid_to_name(uid_t uid, char *domain, char *name, size_t len)
{
	struct passwd *pw = NULL;
	struct passwd pwbuf;
	char *buf;
	size_t buflen = sysconf(_SC_GETPW_R_SIZE_MAX);
	int err = -ENOMEM;

	buf = malloc(buflen);
	if (!buf)
		goto out;
	if (domain == NULL)
		domain = get_default_domain();
	err = -getpwuid_r(uid, &pwbuf, buf, buflen, &pw);
	if (pw == NULL)
		err = -ENOENT;
	if (err)
		goto out_buf;
	if (get_nostrip() & IDTYPE_USER)
		err = write_name(name, pw->pw_name, domain, len, 0);
	else
		err = write_name(name, pw->pw_name, domain, len, 1);
out_buf:
	free(buf);
out:
	return err;
}

static int nss_gid_to_name(gid_t gid, char *domain, char *name, size_t len)
{
	struct group *gr = NULL;
	struct group grbuf;
	char *buf;
	size_t buflen = sysconf(_SC_GETGR_R_SIZE_MAX);
	int err;

	if (domain == NULL)
		domain = get_default_domain();

	do {
		err = -ENOMEM;
		buf = malloc(buflen);
		if (!buf)
			goto out;
		err = -getgrgid_r(gid, &grbuf, buf, buflen, &gr);
		if (gr == NULL && !err)
			err = -ENOENT;
		if (err == -ERANGE) {
			buflen *= 2;
			free(buf);
		}
	} while (err == -ERANGE);

	if (err)
		goto out_buf;
	if (get_nostrip() & IDTYPE_GROUP)
		err = write_name(name, gr->gr_name, domain, len, 0);
	else
		err = write_name(name, gr->gr_name, domain, len, 1);
out_buf:
	free(buf);
out:
	return err;
}

/* XXX: actually should return error, so can distinguish between
 * memory allocation failure and failure to match domain */
static char *strip_domain(const char *name, const char *domain)
{
	const char *c;
	char *l = NULL;
	int len;

	if (name == NULL)
		goto out;

	c = strrchr(name, '@');
	if (c == NULL && domain != NULL)
		goto out;
	if (c == NULL && domain == NULL) {
		len = strlen(name) + 1;
	} else {
		if (domain && strcasecmp(c + 1, domain) != 0)
			goto out;
		len = c - name;
	}

	l = malloc(len + 1);
	if (l == NULL)
		goto out;
	memcpy(l, name, len);
	l[len] = '\0';
out:
	return l;
}

struct pwbuf {
	struct passwd pwbuf;
	char buf[1];
};

static struct passwd *nss_getpwnam(const char *name, const char *domain,
				   int *err_p, int dostrip)
{
	struct passwd *pw;
	struct pwbuf *buf;
	size_t buflen = sysconf(_SC_GETPW_R_SIZE_MAX);
	char *localname;
	int err = ENOMEM;

	if (buflen > UINT_MAX)
		goto err;

	buf = malloc(sizeof(*buf) + buflen);
	if (buf == NULL)
		goto err;

	err = EINVAL;
	if (dostrip) {
		localname = strip_domain(name, domain);
		IDMAP_LOG(4, ("nss_getpwnam: name '%s' domain '%s': "
			  "resulting localname '%s'", name, domain, localname));
		if (localname == NULL) {
			IDMAP_LOG(0, ("nss_getpwnam: name '%s' does not map "
				"into domain '%s'", name,
				domain ? domain : "<not-provided>"));
			goto err_free_buf;
		}

		err = getpwnam_r(localname, &buf->pwbuf, buf->buf, buflen, &pw);
		if (pw == NULL && domain != NULL)
			IDMAP_LOG(1,
				("nss_getpwnam: name '%s' not found in domain '%s'",
				localname, domain));
		free(localname);
	} else {
		err = getpwnam_r(name, &buf->pwbuf, buf->buf, buflen, &pw);
		if (pw == NULL)
			IDMAP_LOG(1,
				("nss_getpwnam: name '%s' not found (domain not stripped)", name));
	}
	if (err == 0 && pw != NULL) {
		*err_p = 0;
		return pw;
	} else if (err == 0 && pw == NULL) {
		err = ENOENT;
	}

err_free_buf:
	free(buf);
err:
	*err_p = -err;
	return NULL;
}

static int nss_name_to_uid(char *name, uid_t *uid)
{
	struct passwd *pw = NULL;
	char *domain;
	int err = -ENOENT;

	domain = get_default_domain();
	if (get_nostrip() & IDTYPE_USER) {
		pw = nss_getpwnam(name, domain, &err, 0);
		if (pw != NULL)
			goto out_uid;
	}
	pw = nss_getpwnam(name, domain, &err, 1);
	if (pw == NULL)
		goto out;
out_uid:
	*uid = pw->pw_uid;
	IDMAP_LOG(4, ("nss_name_to_uid: name '%s' uid %u", name, *uid));
	free(pw);
	err = 0;
out:
	return err;
}

static char *reformat_name(const char *name)
{
	const char *domain;
	const char *c;
	const char *d;
	char *l = NULL;
	int len;
	int dlen = 0;
	int i;

	c = strchr(name, '@');
	if (c == NULL)
		goto out;
	len = c - name;
	domain = ++c;
	d = strchr(domain, '.');
	if (d == NULL)
		goto out;
	dlen = d - domain;
	l = malloc(dlen + 1 + len + 1);
	if (l == NULL)
		goto out;
	for (i = 0; i < dlen; i++)
		l[i] = toupper(domain[i]);
	l[dlen] = '\\';
	memcpy(l + dlen + 1, name, len);
	l[dlen + 1 + len] = '\0';
out:
	return l;
}

static int _nss_name_to_gid(char *name, gid_t *gid, int dostrip)
{
	struct group *gr = NULL;
	struct group grbuf;
	char *buf, *domain;
	size_t buflen = sysconf(_SC_GETGR_R_SIZE_MAX);
	int err = -EINVAL;
	char *localname = NULL;
	char *ref_name = NULL;

	domain = get_default_domain();
	if (dostrip) {
		localname = strip_domain(name, domain);
		IDMAP_LOG(4, ("nss_name_to_gid: name '%s' domain '%s': "
			  "resulting localname '%s'", name, domain, localname));
		if (!localname) {
			IDMAP_LOG(0, ("nss_name_to_gid: name '%s' does not map "
				  "into domain '%s'", name, domain));
			goto out;
		}
	} else if (get_reformat_group()) {
		ref_name = reformat_name(name);
		if (ref_name == NULL) {
			IDMAP_LOG(1, ("nss_name_to_gid: failed to reformat name '%s'",
				  name));
			err = -ENOENT;
			goto out;
		}
	}

	err = -ENOMEM;
	if (buflen > UINT_MAX)
		goto out_name;

	do {
		buf = malloc(buflen);
		if (!buf)
			goto out_name;
		if (dostrip)
			err = -getgrnam_r(localname, &grbuf, buf, buflen, &gr);
		else if (get_reformat_group())
			err = -getgrnam_r(ref_name, &grbuf, buf, buflen, &gr);
		else
			err = -getgrnam_r(name, &grbuf, buf, buflen, &gr);
		if (gr == NULL && !err) {
			if (dostrip)
				IDMAP_LOG(1, ("nss_name_to_gid: name '%s' not found "
					  "in domain '%s'", localname, domain));
			else if (get_reformat_group())
				IDMAP_LOG(1, ("nss_name_to_gid: name '%s' not found "
					  "(reformatted)", ref_name));
			else
				IDMAP_LOG(1, ("nss_name_to_gid: name '%s' not found "
					  "(domain not stripped)", name));
			err = -ENOENT;
		}
		if (err == -ERANGE) {
			buflen *= 2;
			free(buf);
		}
	} while (err == -ERANGE);

	if (err)
		goto out_buf;
	*gid = gr->gr_gid;
	IDMAP_LOG(4, ("nss_name_to_gid: name '%s' gid %u", name, *gid));
out_buf:
	free(buf);
out_name:
	free(localname);
	free(ref_name);
out:
	return err;
}

static int nss_name_to_gid(char *name, gid_t *gid)
{
	int err = 0;

	if (get_nostrip() & IDTYPE_GROUP) {
		err = _nss_name_to_gid(name, gid, 0);
		if (!err)
			goto out;
	}
	err = _nss_name_to_gid(name, gid, 1);
out:
	return err;
}

static int nss_gss_princ_to_ids(char *secname, char *princ,
				uid_t *uid, uid_t *gid,
				extra_mapping_params **UNUSED(ex))
{
	struct passwd *pw;
	int err = 0;
	char *princ_realm;
	struct conf_list *realms;
	struct conf_list_node *r;
	int found = 0;

	if (strcmp(secname, "spkm3") == 0)
		return -ENOENT;

	if (strcmp(secname, "krb5") != 0)
		return -EINVAL;

	/* get princ's realm */
	princ_realm = strstr(princ, "@");
	if (princ_realm == NULL)
		return -EINVAL;
	princ_realm++;

	/* get list of "local-equivalent" realms and
	 * check against the principal's realm */
	realms = get_local_realms();
	TAILQ_FOREACH(r, &realms->fields, link) {
		if (strcmp(r->field, princ_realm) == 0) {
			found = 1;
			break;
		}
	}
	if (!found) {
		IDMAP_LOG(1, ("nss_gss_princ_to_ids: Local-Realm '%s': NOT FOUND", 
			princ_realm));
		return -ENOENT;
	}
	/* XXX: this should call something like getgssauthnam instead? */
	pw = nss_getpwnam(princ, NULL, &err, 1);
	if (pw == NULL) {
		err = -ENOENT;
		goto out;
	}
	*uid = pw->pw_uid;
	*gid = pw->pw_gid;
	free(pw);
out:
	return err;
}

static int nss_gss_princ_to_grouplist(char *secname, char *princ,
			       gid_t *groups, int *ngroups,
			       extra_mapping_params **UNUSED(ex))
{
	struct passwd *pw;
	int ret = -EINVAL;

	if (strcmp(secname, "krb5") != 0)
		goto out;
	/* XXX: not quite right?  Need to know default realm? */
	/* XXX: this should call something like getgssauthnam instead? */
	pw = nss_getpwnam(princ, NULL, &ret, 1);
	if (pw == NULL) {
		ret = -ENOENT;
		goto out;
	}
	if (getgrouplist(pw->pw_name, pw->pw_gid, groups, ngroups) < 0)
		ret = -ERANGE;
	free(pw);
out:
	return ret;
}

static int nss_plugin_init(void)
{
	if (nfsidmap_conf_path)
		conf_init_file(nfsidmap_conf_path);
	return 0;
}

/*
 * Called by dlclose(). See dlopen(3) man page
 */
__attribute__((destructor))
static int nss_plugin_term(void)
{
	free_local_realms();
	conf_cleanup();
	return 0;
}


struct trans_func nss_trans = {
	.name		= "nsswitch",
	.init		= nss_plugin_init,
	.princ_to_ids	= nss_gss_princ_to_ids,
	.name_to_uid	= nss_name_to_uid,
	.name_to_gid	= nss_name_to_gid,
	.uid_to_name	= nss_uid_to_name,
	.gid_to_name	= nss_gid_to_name,
	.gss_princ_to_grouplist = nss_gss_princ_to_grouplist,
};

struct trans_func *libnfsidmap_plugin_init(void)
{
	return (&nss_trans);
}
