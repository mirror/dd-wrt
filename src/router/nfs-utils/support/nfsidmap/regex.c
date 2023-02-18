/*
 *  regex.c
 *
 *  regex idmapping functions.
 *
 *  Copyright (c) 2017-2020 Stefan Walter <stefan.walter@inf.ethz.ch>.
 *  Copyright (c) 2008 David H?rdeman <david@hardeman.nu>.
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
#include <regex.h>

#include "nfsidmap.h"
#include "nfsidmap_plugin.h"

#define CONFIG_GET_STRING nfsidmap_config_get
extern const char *nfsidmap_config_get(const char *, const char *);

#define MAX_MATCHES 100

regex_t group_re;
regex_t user_re;
regex_t gpx_re;
int use_gpx;
const char * group_prefix;
const char * group_name_prefix;
const char * group_suffix;
const char * user_prefix;
const char * user_suffix;
const char * group_map_file;
const char * group_map_section;
char empty = '\0';
size_t group_name_prefix_length;

struct pwbuf {
        struct passwd pwbuf;
        char buf[1];
};

struct grbuf {
        struct group grbuf;
        char buf[1];
};

static char *get_default_domain(void)
{
        static char default_domain[NFS4_MAX_DOMAIN_LEN] = "";
        if (default_domain[0] == 0) {
                nfs4_get_default_domain(NULL, default_domain, NFS4_MAX_DOMAIN_LEN);
        }
        return default_domain;
}

/*
 * Regexp Translation Methods
 *
 */

static struct passwd *regex_getpwnam(const char *name, const char *UNUSED(domain),
				      int *err_p)
{
	struct passwd *pw;
	struct pwbuf *buf;
	size_t buflen = sysconf(_SC_GETPW_R_SIZE_MAX);
	char *localname;
	size_t namelen;
	int err;
	int status;
	int index;
	regmatch_t matches[MAX_MATCHES];

	buf = malloc(sizeof(*buf) + buflen);
	if (!buf) {
		err = ENOMEM;
		goto err;
	}

	status = regexec(&user_re, name, MAX_MATCHES, matches, 0);
	if (status) {
		IDMAP_LOG(4, ("regexp_getpwnam: user '%s' did not match regex", name));
		err = ENOENT;
		goto err_free_buf;
	}

	for (index = 1; index < MAX_MATCHES ; index++)
	{
		if (matches[index].rm_so >= 0)
			break;
	}

	if (index == MAX_MATCHES) {
		IDMAP_LOG(4, ("regexp_getpwnam: user '%s' did not match regex", name));
		err = ENOENT;
		goto err_free_buf;
	}

	namelen = matches[index].rm_eo - matches[index].rm_so;
	localname= malloc(namelen + 1);
	if (!localname)
	{
		err = ENOMEM;
		goto err_free_buf;
	}
	strncpy(localname, name+matches[index].rm_so, namelen);
	localname[namelen] = '\0';

again:
	err = getpwnam_r(localname, &buf->pwbuf, buf->buf, buflen, &pw);

	if (err == EINTR)
		goto again;

	if (!pw) {
		if (err == 0)
			err = ENOENT;

		IDMAP_LOG(4, ("regex_getpwnam: local user '%s' for '%s' not found",
		  localname, name));

		goto err_free_name;
	}

	IDMAP_LOG(4, ("regexp_getpwnam: name '%s' mapped to '%s'",
		  name, localname));

	free(localname);
	*err_p = 0;
	return pw;

err_free_name:
	free(localname);
err_free_buf:
	free(buf);
err:
	*err_p = err;
	return NULL;
}

static struct group *regex_getgrnam(const char *name, const char *UNUSED(domain),
				      int *err_p)
{
	struct group *gr;
	struct grbuf *buf;
	size_t buflen = sysconf(_SC_GETGR_R_SIZE_MAX);
	char *localgroup;
	char *groupname;
	size_t namelen;
	int err = 0;
	int index;
	int status;
	regmatch_t matches[MAX_MATCHES];

	buf = malloc(sizeof(*buf) + buflen);
	if (!buf) {
		err = ENOMEM;
		goto err;
	}

	status = regexec(&group_re, name, MAX_MATCHES, matches, 0);
	if (status) {
		IDMAP_LOG(4, ("regexp_getgrnam: group '%s' did not match regex", name));
		err = ENOENT;
		goto err_free_buf;
	}

	for (index = 1; index < MAX_MATCHES ; index++)
	{
		if (matches[index].rm_so >= 0)
			break;
	}

	if (index == MAX_MATCHES) {
		IDMAP_LOG(4, ("regexp_getgrnam: group '%s' did not match regex", name));
		err = ENOENT;
		goto err_free_buf;
	}

	namelen = matches[index].rm_eo - matches[index].rm_so;
	localgroup = malloc(namelen + 1);
	if (!localgroup)
	{
		err = ENOMEM;
		goto err_free_buf;
	}
	strncpy(localgroup, name+matches[index].rm_so, namelen);
	localgroup[namelen] = '\0';

	IDMAP_LOG(4, ("regexp_getgrnam: group '%s' after match of regex", localgroup));

        groupname = localgroup;
    	if (group_name_prefix_length && ! strncmp(group_name_prefix, localgroup, group_name_prefix_length))
	{
		err = 1;
		if (use_gpx)
			err = regexec(&gpx_re, localgroup, 0, NULL, 0);

		if (err)
		{
			IDMAP_LOG(4, ("regexp_getgrnam: removing prefix '%s' (%d long) from group '%s'", group_name_prefix, group_name_prefix_length, localgroup));
				groupname += group_name_prefix_length;
		}
		else
		{
			IDMAP_LOG(4, ("regexp_getgrnam: not removing prefix from group '%s'", localgroup));
		}
	}

	IDMAP_LOG(4, ("regexp_getgrnam: will use '%s'", groupname));

again:
	err = getgrnam_r(groupname, &buf->grbuf, buf->buf, buflen, &gr);

	if (err == EINTR)
		goto again;

	if (!gr) {
		if (err == 0)
			err = ENOENT;

		IDMAP_LOG(4, ("regex_getgrnam: local group '%s' for '%s' not found", groupname, name));

		goto err_free_name;
	}

	IDMAP_LOG(4, ("regex_getgrnam: group '%s' mapped to '%s'", name, groupname));

	free(localgroup);

	*err_p = 0;
	return gr;

err_free_name:
	free(localgroup);
err_free_buf:
	free(buf);
err:
	*err_p = err;
	return NULL;
}

static int regex_gss_princ_to_ids(char *secname, char *princ,
				   uid_t *uid, uid_t *gid,
				   extra_mapping_params **UNUSED(ex))
{
	struct passwd *pw;
	int err;

	/* XXX: Is this necessary? */
	if (strcmp(secname, "krb5") != 0 && strcmp(secname, "spkm3") != 0)
		return -EINVAL;

	pw = regex_getpwnam(princ, NULL, &err);

	if (pw) {
		*uid = pw->pw_uid;
		*gid = pw->pw_gid;
		free(pw);
	}

	return -err;
}

static int regex_gss_princ_to_grouplist(char *secname, char *princ,
					 gid_t *groups, int *ngroups,
					 extra_mapping_params **UNUSED(ex))
{
	struct passwd *pw;
	int err;

	/* XXX: Is this necessary? */
	if (strcmp(secname, "krb5") != 0 && strcmp(secname, "spkm3") != 0)
		return -EINVAL;

	pw = regex_getpwnam(princ, NULL, &err);

	if (pw) {
		if (getgrouplist(pw->pw_name, pw->pw_gid, groups, ngroups) < 0)
			err = -ERANGE;
		free(pw);
	}

	return -err;
}

static int regex_name_to_uid(char *name, uid_t *uid)
{
	struct passwd *pw;
	int err;

	pw = regex_getpwnam(name, NULL, &err);

	if (pw) {
		*uid = pw->pw_uid;
		free(pw);
	}

	return -err;
}

static int regex_name_to_gid(char *name, gid_t *gid)
{
	struct group *gr;
	int err;

	gr = regex_getgrnam(name, NULL, &err);

	if (gr) {
		*gid = gr->gr_gid;
		free(gr);
	}

	return -err;
}

static int write_name(char *dest, char *localname, const char* name_prefix, const char *prefix, const char *suffix, size_t len)
{
	if (strlen(localname) + strlen(name_prefix) + strlen(prefix) + strlen(suffix) + 1 > len) {
		return -ENOMEM; /* XXX: Is there an -ETOOLONG? */
	}
	strcpy(dest, prefix);
	strcat(dest, name_prefix);
	strcat(dest, localname);
	strcat(dest, suffix);

   	IDMAP_LOG(4, ("write_name: will use '%s'", dest));

	return 0;
}

static int regex_uid_to_name(uid_t uid, char *domain, char *name, size_t len)
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
	err = write_name(name, pw->pw_name, &empty, user_prefix, user_suffix, len);
out_buf:
	free(buf);
out:
	return err;
}

static int regex_gid_to_name(gid_t gid, char *UNUSED(domain), char *name, size_t len)
{
	struct group *gr = NULL;
	struct group grbuf;
	char *buf;
    const char *name_prefix;
	size_t buflen = sysconf(_SC_GETGR_R_SIZE_MAX);
	int err;
    char * groupname = NULL;

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

	groupname = gr->gr_name;
    	name_prefix = group_name_prefix;
    	if (group_name_prefix_length)
        {
            if(! strncmp(group_name_prefix, groupname, group_name_prefix_length))
            {
			    name_prefix = &empty;
            }
            else if (use_gpx)
            {
	            err = regexec(&gpx_re, groupname, 0, NULL, 0);
	            if (!err)
                {
   			        IDMAP_LOG(4, ("regex_gid_to_name: not adding prefix to group '%s'", groupname));
			        name_prefix = &empty;
                }
            }
	}
      
	err = write_name(name, groupname, name_prefix, group_prefix, group_suffix, len);

out_buf:
	free(buf);
out:
	return err;
}

static int regex_init(void) {	
	const char *string;
	int status;


        string = CONFIG_GET_STRING("Regex", "User-Regex");
	if (!string)
	{
		warnx("regex_init: regex for user mapping missing");
		goto error1;
	}
    
	status = regcomp(&user_re, string, REG_EXTENDED|REG_ICASE); 
	if (status)
	{
		warnx("regex_init: compiling regex for user mapping failed with status %u", status);
		goto error1;
	}

	string = CONFIG_GET_STRING("Regex", "Group-Regex");
	if (!string)
	{
		warnx("regex_init: regex for group mapping missing");
		goto error2;
	}
    
    status = regcomp(&group_re, string, REG_EXTENDED|REG_ICASE); 
    if (status)
    {
		warnx("regex_init: compiling regex for group mapping failed with status %u", status);
		goto error2;
    }

	group_name_prefix = CONFIG_GET_STRING("Regex", "Group-Name-Prefix");
    if (!group_name_prefix)
    {
        group_name_prefix = &empty;
    }
    group_name_prefix_length = strlen(group_name_prefix);

    user_prefix = CONFIG_GET_STRING("Regex", "Prepend-Before-User");
    if (!user_prefix)
    {
        user_prefix = &empty;
    }

    user_suffix = CONFIG_GET_STRING("Regex", "Append-After-User"); 
    if (!user_suffix)
    {
        user_suffix = &empty;
    }

    group_prefix = CONFIG_GET_STRING("Regex", "Prepend-Before-Group"); 
    if (!group_prefix)
    {
        group_prefix = &empty;
    }

    group_suffix = CONFIG_GET_STRING("Regex", "Append-After-Group"); 
    if (!group_suffix)
    {
        group_suffix = &empty;
    }

    string = CONFIG_GET_STRING("Regex", "Group-Name-No-Prefix-Regex");
    use_gpx = 0;
    if (string)
    {
        status = regcomp(&gpx_re, string, REG_EXTENDED|REG_ICASE); 

        if (status)
        {
	    	warnx("regex_init: compiling regex for group prefix exclusion failed with status %u", status);
		    goto error3;
        }

        use_gpx = 1;
    }

    return 0;

error3:
	regfree(&group_re);
error2:
	regfree(&user_re);
error1:
	return 0;
 /* return -EINVAL; */
}


struct trans_func regex_trans = {
	.name			= "regex",
	.init			= regex_init,
	.name_to_uid		= regex_name_to_uid,
	.name_to_gid		= regex_name_to_gid,
	.uid_to_name		= regex_uid_to_name,
	.gid_to_name		= regex_gid_to_name,
	.princ_to_ids		= regex_gss_princ_to_ids,
	.gss_princ_to_grouplist	= regex_gss_princ_to_grouplist,
};

struct trans_func *libnfsidmap_plugin_init()
{
	return (&regex_trans);
}

