/*
 * umich_ldap.c
 *
 * Copyright (c) 2000 The Regents of the University of Michigan.
 * All rights reserved.
 *
 * Copyright (c) 2004 Andy Adamson <andros@UMICH.EDU>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "config.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <pwd.h>
#include <err.h>
#ifdef HAVE_GSSAPI_GSSAPI_KRB5_H
#include <gssapi/gssapi_krb5.h>
#endif /* HAVE_GSSAPI_GSSAPI_KRB5_H */
#ifdef HAVE_SASL_H
#include <sasl.h>
#endif /* HAVE_SASL_H */
#ifdef HAVE_SASL_SASL_H
#include <sasl/sasl.h>
#endif /* HAVE_SASL_SASL_H */
/* We are using deprecated functions, get the prototypes... */
#define LDAP_DEPRECATED 1
#include <ldap.h>
#include "nfslib.h"
#include "nfsidmap.h"
#include "nfsidmap_plugin.h"
#include "nfsidmap_private.h"
#include "conffile.h"

/* attribute/objectclass default mappings */
#define DEFAULT_UMICH_OBJCLASS_REMOTE_PERSON	"NFSv4RemotePerson"
#define DEFAULT_UMICH_OBJCLASS_REMOTE_GROUP	"NFSv4RemoteGroup"
#define DEFAULT_UMICH_ATTR_NFSNAME		"NFSv4Name"
#define DEFAULT_UMICH_ATTR_ACCTNAME		"uid"
#define DEFAULT_UMICH_ATTR_UIDNUMBER		"uidNumber"
#define DEFAULT_UMICH_ATTR_GROUP_NFSNAME	"NFSv4Name"
#define DEFAULT_UMICH_ATTR_GIDNUMBER		"gidNumber"
#define DEFAULT_UMICH_ATTR_MEMBERUID		"memberUid"
#define DEFAULT_UMICH_ATTR_GSSAUTHNAME		"GSSAuthName"
#define DEFAULT_UMICH_ATTR_MEMBEROF		"memberof"

#define DEFAULT_UMICH_SEARCH_TIMEOUT		4

/* config section */
#define LDAP_SECTION "UMICH_SCHEMA"

#ifndef LDAP_FILT_MAXSIZ
#define LDAP_FILT_MAXSIZ	1024
#endif


/* Local structure definitions */

struct ldap_map_names{
	char *NFSv4_person_objcls;
	char *NFSv4_nfsname_attr;
	char *NFSv4_acctname_attr;
	char *NFSv4_uid_attr;
	char *NFSv4_group_objcls;
	char *NFSv4_group_nfsname_attr;
	char *NFSv4_gid_attr;
	char *NFSv4_member_attr;
	char *NFSv4_member_of_attr;
	char *GSS_principal_attr;
	char *NFSv4_grouplist_filter; /* Filter for grouplist lookups */
};

struct umich_ldap_info {
	char *server;		/* server name/address */
	int  port;		/* server port */
	char *base;		/* base DN */
	char *people_tree;	/* base DN to start searches for people */
	char *group_tree;	/* base DN to start searches for groups */
	char *user_dn;		/* optional DN for user account when binding */
	char *passwd;		/* Password to use when binding to directory */
	int use_ssl;		/* SSL flag */
	char *ca_cert;		/* File location of the ca_cert */
	int tls_reqcert;	/* req and validate server cert */
	int memberof_for_groups;/* Use 'memberof' attribute when
				   looking up user groups */
	int ldap_timeout;	/* Timeout in seconds for searches
				   by ldap_search_st */
	int follow_referrals;	/* whether to follow ldap referrals */
	char *sasl_mech;	/* sasl mech to be used */
	char *sasl_realm;	/* SASL realm for SASL authentication */
	char *sasl_authcid;	/* authentication identity to be used  */
	char *sasl_authzid;	/* authorization identity to be used */
	char *sasl_secprops;	/* Cyrus SASL security properties. */
	int sasl_canonicalize;	/* canonicalize LDAP server host name */
	char *sasl_krb5_ccname;	/* krb5 ticket cache */
};

/* GLOBAL data */

static struct umich_ldap_info ldap_info = {
	.server = NULL,
	.port = 0,
	.base = NULL,
	.people_tree = NULL,
	.group_tree = NULL,
	.user_dn = NULL,
	.passwd = NULL,
	.use_ssl = 0,
	.ca_cert = NULL,
	.tls_reqcert = LDAP_OPT_X_TLS_HARD,
	.memberof_for_groups = 0,
	.ldap_timeout = DEFAULT_UMICH_SEARCH_TIMEOUT,
	.follow_referrals = 1,
	.sasl_mech = NULL,
	.sasl_realm = NULL,
	.sasl_authcid = NULL,
	.sasl_authzid = NULL,
	.sasl_secprops = NULL,
	.sasl_canonicalize = -1, /* leave to the LDAP lib */
	.sasl_krb5_ccname = NULL,
};

static struct ldap_map_names ldap_map = {
	.NFSv4_person_objcls = NULL,
	.NFSv4_nfsname_attr = NULL,
	.NFSv4_uid_attr = NULL,
	.NFSv4_acctname_attr = NULL,
	.NFSv4_group_objcls = NULL,
	.NFSv4_group_nfsname_attr = NULL,
	.NFSv4_gid_attr = NULL,
	.NFSv4_member_attr = NULL,
	.NFSv4_member_of_attr = NULL,
	.GSS_principal_attr = NULL,
	.NFSv4_grouplist_filter = NULL,
};

#ifdef ENABLE_LDAP_SASL

/**
 * Set the path of the krb5 ticket cache
 * use gss_krb5_ccache_name if available else set the env var
 */
static int set_krb5_ccname(const char *krb5_ccache_name)
{
	int retval = 0;
#ifdef HAVE_GSS_KRB5_CCACHE_NAME
	OM_uint32 status;

	if (gss_krb5_ccache_name(&status, krb5_ccache_name, NULL) !=
		GSS_S_COMPLETE) {
		IDMAP_LOG(5,
		  ("Failed to set creds cache for kerberos, minor_status(%d)",
		   status));
		retval = status;
		goto out;
	}
#else /* HAVE_GSS_KRB5_CCACHE_NAME */
	char *env;
	int buflen = 0;

	buflen = strlen("KRB5CCNAME=") + strlen(krb5_ccache_name) + 1;
	env = malloc(buflen);
	if (env == NULL) {
		retval = ENOMEM;
		goto out;
	}
	snprintf(env, buflen, "KRB5CCNAME=%s", krb5_ccache_name);
	if (putenv(env) != 0) {
		retval = errno;
		IDMAP_LOG(5, ("Failed to set creds cache for kerberos, err(%d)",
			      retval));
	}
#endif /* else HAVE_GSS_KRB5_CCACHE_NAME */
out:
	return retval;
}

/**
 * SASL interact callback
 */
static int sasl_interact_cb(__attribute__((unused)) LDAP * ld,
		__attribute__((unused)) unsigned int flags, void *defaults,
		void *ctx)
{
	struct umich_ldap_info *linfo = defaults;
	sasl_interact_t *interact = ctx;

	while (interact->id != SASL_CB_LIST_END) {
		switch (interact->id) {
		case SASL_CB_AUTHNAME:
			if (linfo->sasl_authcid == NULL ||
			    linfo->sasl_authcid[0] == '\0') {
				IDMAP_LOG(2, ("SASL_CB_AUTHNAME asked in "
					    "callback but not found in conf"));
			} else {
				IDMAP_LOG(5,
					  ("Setting SASL_CB_AUTHNAME to %s",
					   linfo->sasl_authcid));
				interact->result = linfo->sasl_authcid;
				interact->len = strlen(linfo->sasl_authcid);
			}
			break;
		case SASL_CB_PASS:
			if (linfo->passwd == NULL || linfo->passwd[0] == '\0') {
				IDMAP_LOG(2, ("SASL_CB_PASS asked in callback "
					      "but not found in conf"));
			} else {
				IDMAP_LOG(5,
					  ("Setting SASL_CB_PASS to ***"));
				interact->result = linfo->passwd;
				interact->len = strlen(linfo->passwd);
			}
			break;
		case SASL_CB_GETREALM:
			if (linfo->sasl_realm == NULL ||
			    linfo->sasl_realm[0] == '\0') {
				IDMAP_LOG(2, ("SASL_CB_GETREALM asked in "
					    "callback but not found in conf"));
			} else {
				IDMAP_LOG(5,
					  ("Setting SASL_CB_GETREALM to %s",
					   linfo->sasl_realm));
				interact->result = linfo->sasl_realm;
				interact->len = strlen(linfo->sasl_realm);
			}
			break;
		case SASL_CB_USER:
			if (linfo->sasl_authzid == NULL ||
			    linfo->sasl_authzid[0] == '\0') {
				IDMAP_LOG(2, ("SASL_CB_USER asked in callback "
					      "but not found in conf"));
			} else {
				IDMAP_LOG(5, ("Setting SASL_CB_USER to %s",
					      linfo->sasl_authzid));
				interact->result = linfo->sasl_authzid;
				interact->len = strlen(linfo->sasl_authzid);
			}
			break;
		default:
			IDMAP_LOG(2, ("Undefined value requested %d",
				      interact->id));
			break;
		}
		interact++;
	}
	return LDAP_SUCCESS;
}
#endif /* ENABLE_LDAP_SASL */

/* Local routines */

static int
ldap_init_and_bind(LDAP **pld,
		   int *sizelimit,
		   struct umich_ldap_info *linfo)
{
	LDAP *ld;
	int lerr;
	int err = -1;
	int current_version, new_version;
	char server_url[1024];
	int debug_level = 65535;
	int i;
	LDAPAPIInfo apiinfo = {.ldapai_info_version = LDAP_API_INFO_VERSION};

	snprintf(server_url, sizeof(server_url), "%s://%s:%d",
		 (linfo->use_ssl) ? "ldaps" : "ldap",
		 linfo->server, linfo->port);

	/*
	 * XXX We really, REALLY only want to initialize once, not for
	 * each request.  Figure out how to do that!
	 */
	if ((lerr = ldap_initialize(&ld, server_url)) != LDAP_SUCCESS) {
		IDMAP_LOG(0, ("ldap_init_and_bind: ldap_initialize() failed "
			  "to [%s]: %s (%d)", server_url,
			  ldap_err2string(lerr), lerr));
		goto out;
	}

	if ((ldap_set_option(ld, LDAP_OPT_DEBUG_LEVEL, &debug_level)
							!= LDAP_SUCCESS)) {
		IDMAP_LOG(0, ("ldap_init_and_bind: error setting ldap "
			  "library debugging level"));
		goto out;
	}

	/*
	 * Get LDAP API information and compare the protocol version there
	 * to the protocol version returned directly from get_option.
	 */
	ldap_get_option(ld, LDAP_OPT_API_INFO, &apiinfo);
	if (apiinfo.ldapai_info_version != LDAP_API_INFO_VERSION) {
		IDMAP_LOG(0, ("ldap_init_and_bind:  APIInfo version mismatch: "
			  "library %d, header %d",
			  apiinfo.ldapai_info_version, LDAP_API_INFO_VERSION));
		goto out;
	}
	ldap_get_option(ld, LDAP_OPT_PROTOCOL_VERSION, &current_version);
	if (apiinfo.ldapai_protocol_version == LDAP_VERSION3 &&
	    current_version != LDAP_VERSION3) {
		new_version = LDAP_VERSION3;
		IDMAP_LOG(4, ("ldap_init_and_bind: version mismatch between "
			  "API information and protocol version. Setting "
			  "protocol version to %d", new_version));
		ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &new_version);
	}

	for (i = 0; apiinfo.ldapai_extensions[i]; i++) {
		char *extension = apiinfo.ldapai_extensions[i];
		ldap_memfree (extension);
	}
	ldap_memfree (apiinfo.ldapai_extensions);
	ldap_memfree(apiinfo.ldapai_vendor_name);

	/* Set sizelimit option if requested */
	if (sizelimit) {
		ldap_set_option(ld, LDAP_OPT_SIZELIMIT, (void *)sizelimit);
	}

	lerr = ldap_set_option(ld, LDAP_OPT_REFERRALS,
			linfo->follow_referrals ? (void *)LDAP_OPT_ON :
						  (void *)LDAP_OPT_OFF);
	if (lerr != LDAP_SUCCESS) {
		IDMAP_LOG(2, ("ldap_init_and_bind: setting LDAP_OPT_REFERRALS "
			      "failed: %s (%d)", ldap_err2string(lerr), lerr));
		goto out;
	}

	/* Set option to to use SSL/TLS if requested */
	if (linfo->use_ssl) {
		int tls_type = LDAP_OPT_X_TLS_HARD;
		lerr = ldap_set_option(ld, LDAP_OPT_X_TLS, &tls_type);
		if (lerr != LDAP_SUCCESS) {
			IDMAP_LOG(2, ("ldap_init_and_bind: setting SSL "
				  "failed : %s (%d)",
				  ldap_err2string(lerr), lerr));
			goto out;
		}

		if (linfo->ca_cert != NULL) {
			lerr = ldap_set_option(NULL, LDAP_OPT_X_TLS_CACERTFILE,
					       linfo->ca_cert);
			if (lerr != LDAP_SUCCESS) {
				IDMAP_LOG(2, ("ldap_init_and_bind: setting CA "
					  "certificate file failed : %s (%d)",
					  ldap_err2string(lerr), lerr));
				goto out;
			}
		}

		lerr = ldap_set_option(NULL, LDAP_OPT_X_TLS_REQUIRE_CERT,
				       &linfo->tls_reqcert);
		if (lerr != LDAP_SUCCESS) {
			IDMAP_LOG(2, ("ldap_init_and_bind: setting "
				      "req CA cert failed : %s(%d)",
				  ldap_err2string(lerr), lerr));
			goto out;
		}
	}

	/* If we have a DN (and password) attempt an authenticated bind */
	if (linfo->user_dn) {
retry_bind:
#ifdef ENABLE_LDAP_SASL
		if (linfo->sasl_mech != NULL && linfo->sasl_mech[0] != '\0') {
		/* use sasl bind */
			if (linfo->sasl_canonicalize != -1) {
				lerr = ldap_set_option(ld,
						LDAP_OPT_X_SASL_NOCANON,
						linfo->sasl_canonicalize ?
						  LDAP_OPT_OFF : LDAP_OPT_ON);
				if (lerr != LDAP_SUCCESS) {
					IDMAP_LOG(2, ("ldap_init_and_bind: "
						    "setting sasl_canonicalize"
						    " failed: %s (%d)",
						    ldap_err2string(lerr),
						    lerr));
					goto out;
				}
			}
			if (linfo->sasl_secprops != NULL &&
			    linfo->sasl_secprops[0] != '\0') {
				lerr = ldap_set_option(ld,
						LDAP_OPT_X_SASL_SECPROPS,
						(void *) linfo->sasl_secprops);
				if (lerr != LDAP_SUCCESS) {
					IDMAP_LOG(2, ("ldap_init_and_bind: "
						      "setting sasl_secprops"
						      " failed: %s (%d)",
						      ldap_err2string(lerr),
						      lerr));
					goto out;
				}
			}
			if (linfo->sasl_krb5_ccname != NULL &&
			    linfo->sasl_krb5_ccname[0] != '\0') {
				lerr = set_krb5_ccname(linfo->sasl_krb5_ccname);
				if (lerr != 0) {
					IDMAP_LOG(2,
						("ldap_init_and_bind: Failed "
						 "to set krb5 ticket cache, "
						 "err=%d", lerr));
				}
			}
			lerr = ldap_sasl_interactive_bind_s(ld, linfo->user_dn,
				linfo->sasl_mech, NULL, NULL, LDAP_SASL_QUIET,
				sasl_interact_cb, linfo);
		} else {
			lerr = ldap_simple_bind_s(ld, linfo->user_dn,
						  linfo->passwd);
		}
#else /* ENABLE_LDAP_SASL */
		lerr = ldap_simple_bind_s(ld, linfo->user_dn, linfo->passwd);
#endif /* else ENABLE_LDAP_SASL */
		if (lerr) {
			char *errmsg;
			if (lerr == LDAP_PROTOCOL_ERROR) {
				ldap_get_option(ld, LDAP_OPT_PROTOCOL_VERSION,
						&current_version);
				new_version = current_version == LDAP_VERSION2 ?
					LDAP_VERSION3 : LDAP_VERSION2;
				ldap_set_option( ld, LDAP_OPT_PROTOCOL_VERSION,
						&new_version);
				IDMAP_LOG(2, ("ldap_init_and_bind: "
					  "got protocol error while attempting "
					  "bind with protocol version %d, "
					  "trying protocol version %d",
					  current_version, new_version));
				if ((ldap_get_option(ld, LDAP_OPT_ERROR_STRING, &errmsg) == LDAP_SUCCESS)
					&& (errmsg != NULL) && (*errmsg != '\0')) {
					IDMAP_LOG(2, ("ldap_init_and_bind: "
						  "Additional info: %s", errmsg));
					ldap_memfree(errmsg);
				}
				goto retry_bind;
			}
#ifdef ENABLE_LDAP_SASL
			IDMAP_LOG(2, ("ldap_init_and_bind: %s "
				  "to [%s] as user '%s': %s (%d)",
				  (linfo->sasl_mech != NULL &&
				   linfo->sasl_mech[0] != '\0') ?
				   "ldap_sasl_interactive_bind_s" :
				   "ldap_simple_bind_s",
				  server_url, linfo->user_dn,
				  ldap_err2string(lerr), lerr));
#else /* ENABLE_LDAP_SASL */
			IDMAP_LOG(2, ("ldap_init_and_bind: ldap_simple_bind_s"
				  "to [%s] as user '%s': %s (%d)",
				  server_url, linfo->user_dn,
				  ldap_err2string(lerr), lerr));

#endif /* else ENABLE_LDAP_SASL */
			if ((ldap_get_option(ld, LDAP_OPT_ERROR_STRING, &errmsg) == LDAP_SUCCESS)
					&& (errmsg != NULL)&& (*errmsg != '\0')) {
				IDMAP_LOG(2, ("ldap_init_and_bind: "
					  "Additional info: %s", errmsg));
				ldap_memfree(errmsg);
			}
			goto out;
		}
	}
#ifdef LDAP_ANONYMOUS_BIND_REQUIRED
	else {
		lerr = ldap_simple_bind_s(ld, NULL, NULL);
		if (lerr) {
			char *errmsg;

			IDMAP_LOG(2, ("ldap_init_and_bind: ldap_simple_bind_s "
			  "to [%s] as anonymous: %s (%d)", server_url,
			  ldap_err2string(lerr), lerr));
			if ((ldap_get_option(ld, LDAP_OPT_ERROR_STRING, &errmsg) == LDAP_SUCCESS)
					&& (errmsg != NULL) && (*errmsg != '\0')) {
				IDMAP_LOG(2, ("ldap_init_and_bind: "
					  "Additional info: %s", errmsg));
				ldap_memfree(errmsg);
			}
			goto out;
		}
	}
#endif

	*pld = ld;
	err = 0;
out:
	return err;
}

static int
umich_name_to_ids(char *name, int idtype, uid_t *uid, gid_t *gid,
		  char *attrtype, struct umich_ldap_info *linfo)
{
	LDAP *ld = NULL;
	struct timeval timeout = {
		.tv_sec = linfo->ldap_timeout,
	};
	LDAPMessage *result = NULL, *entry;
	BerElement *ber = NULL;
	char **idstr, filter[LDAP_FILT_MAXSIZ], *base;
	char *attrs[3];
	char *attr_res;
	int count = 0, err, lerr, f_len;
	int sizelimit = 1;

	err = -EINVAL;
	if (uid == NULL || gid == NULL || name == NULL ||
	    attrtype == NULL || linfo == NULL || linfo->server == NULL ||
	    linfo->people_tree == NULL || linfo->group_tree == NULL)
		goto out;

	*uid = -1;
	*gid = -1;

	if (idtype == IDTYPE_USER) {
		if ((f_len = snprintf(filter, LDAP_FILT_MAXSIZ,
				      "(&(objectClass=%s)(%s=%s))",
				      ldap_map.NFSv4_person_objcls,
				      attrtype, name))
				== LDAP_FILT_MAXSIZ) {
			IDMAP_LOG(0, ("ERROR: umich_name_to_ids: filter "
				  "too long!"));
			goto out;
		}
		base = linfo->people_tree;
	}
	else if (idtype == IDTYPE_GROUP) {
		if ((f_len = snprintf(filter, LDAP_FILT_MAXSIZ,
				      "(&(objectClass=%s)(%s=%s))",
				      ldap_map.NFSv4_group_objcls,
				      attrtype, name))
				== LDAP_FILT_MAXSIZ) {
			IDMAP_LOG(0, ("ERROR: umich_name_to_ids: filter "
				  "too long!"));
			goto out;
		}
		base = linfo->group_tree;
	}
	else {
		IDMAP_LOG(0, ("ERROR: umich_name_to_ids: invalid idtype (%d)",
			idtype));
		goto out;
	}

	if (ldap_init_and_bind(&ld, &sizelimit, linfo))
		goto out;

	attrs[0] = ldap_map.NFSv4_uid_attr;
	attrs[1] = ldap_map.NFSv4_gid_attr;
	attrs[2] = NULL;

	err = ldap_search_st(ld, base, LDAP_SCOPE_SUBTREE,
			 filter, (char **)attrs,
			 0, &timeout, &result);
	if (err) {
		char *errmsg;

		IDMAP_LOG(2, ("umich_name_to_ids: ldap_search_st for "
			  "base '%s', filter '%s': %s (%d)",
			  base, filter, ldap_err2string(err), err));
		if ((ldap_get_option(ld, LDAP_OPT_ERROR_STRING, &errmsg) == LDAP_SUCCESS)
				&& (errmsg != NULL) && (*errmsg != '\0')) {
			IDMAP_LOG(2, ("umich_name_to_ids: "
				  "Additional info: %s", errmsg));
			ldap_memfree(errmsg);
		}
		err = -ENOENT;
		goto out_unbind;
	}

	err = -ENOENT;
	count = ldap_count_entries(ld, result);
	if (count != 1) {
		goto out_unbind;
	}

	if (!(entry = ldap_first_entry(ld, result))) {
		lerr = ldap_result2error(ld, result, 0);
		IDMAP_LOG(2, ("umich_name_to_ids: ldap_first_entry: "
			  "%s (%d)", ldap_err2string(lerr), lerr));
		goto out_unbind;
	}

	/*
	 * Attributes come back in no particular order, so we need
	 * to check each one to see what it is before assigning values.
	 * XXX There must be a better way than comparing the
	 * name of each attribute?
	 */
	for (attr_res = ldap_first_attribute(ld, result, &ber);
	     attr_res != NULL;
	     attr_res = ldap_next_attribute(ld, result, ber)) {

		unsigned long tmp_u, tmp_g;
		uid_t tmp_uid;
		gid_t tmp_gid;

		if ((idstr = ldap_get_values(ld, result, attr_res)) == NULL) {
			lerr = ldap_result2error(ld, result, 0);
			IDMAP_LOG(2, ("umich_name_to_ids: ldap_get_values: "
				  "%s (%d)", ldap_err2string(lerr), lerr));
			goto out_memfree;
		}
		if (strcasecmp(attr_res, ldap_map.NFSv4_uid_attr) == 0) {
			tmp_u = strtoul(*idstr, (char **)NULL, 10);
			tmp_uid = tmp_u;
			if (tmp_uid != tmp_u ||
				(errno == ERANGE && tmp_u == ULONG_MAX)) {
				IDMAP_LOG(0, ("ERROR: umich_name_to_ids: "
					  "uidNumber too long converting '%s'",
					  *idstr));
				ldap_memfree(attr_res);
				ldap_value_free(idstr);
				goto out_memfree;
			}
			*uid = tmp_uid;
			err = 0;
		} else if (strcasecmp(attr_res, ldap_map.NFSv4_gid_attr) == 0) {
			tmp_g = strtoul(*idstr, (char **)NULL, 10);
			tmp_gid = tmp_g;
			if (tmp_gid != tmp_g ||
				(errno == ERANGE && tmp_g == ULONG_MAX)) {
				IDMAP_LOG(0, ("ERROR: umich_name_to_ids: "
					  "gidNumber too long converting '%s'",
					  *idstr));
				ldap_memfree(attr_res);
				ldap_value_free(idstr);
				goto out_memfree;
			}
			*gid = tmp_gid;
			err = 0;
		} else {
			IDMAP_LOG(0, ("umich_name_to_ids: received attr "
				"'%s' ???", attr_res));
			ldap_memfree(attr_res);
			ldap_value_free(idstr);
			goto out_memfree;
		}
		ldap_memfree(attr_res);
		ldap_value_free(idstr);
	}

out_memfree:
	ber_free(ber, 0);
out_unbind:
	if (result)
		ldap_msgfree(result);
	ldap_unbind(ld);
out:
	return err;
}

static int
umich_id_to_name(uid_t id, int idtype, char **name, size_t len,
		 struct umich_ldap_info *linfo)
{
	LDAP *ld = NULL;
	struct timeval timeout = {
		.tv_sec = linfo->ldap_timeout,
	};
	LDAPMessage *result = NULL, *entry;
	BerElement *ber;
	char **names = NULL, filter[LDAP_FILT_MAXSIZ], *base;
	char idstr[16];
	char *attrs[2];
	char *attr_res;
	int count = 0, err, lerr, f_len;
	int sizelimit = 1;

	err = -EINVAL;
	if (name == NULL || linfo == NULL || linfo->server == NULL ||
		linfo->people_tree == NULL || linfo->group_tree == NULL)
		goto out;

	snprintf(idstr, sizeof(idstr), "%d", id);


	if (idtype == IDTYPE_USER) {
		if ((f_len = snprintf(filter, LDAP_FILT_MAXSIZ,
				      "(&(objectClass=%s)(%s=%s))",
				      ldap_map.NFSv4_person_objcls,
				      ldap_map.NFSv4_uid_attr, idstr))
				== LDAP_FILT_MAXSIZ) {
			IDMAP_LOG(0, ("ERROR: umich_id_to_name: "
				  "uid filter too long!"));
			goto out;
		}
		base = linfo->people_tree;
	} else if (idtype == IDTYPE_GROUP) {
		if ((f_len = snprintf(filter, LDAP_FILT_MAXSIZ,
				      "(&(objectClass=%s)(%s=%s))",
				      ldap_map.NFSv4_group_objcls,
				      ldap_map.NFSv4_gid_attr,idstr))
				== LDAP_FILT_MAXSIZ) {
			IDMAP_LOG(0, ("ERROR: umich_id_to_name: "
				  "gid filter too long!"));
			goto out;
		}
		base = linfo->group_tree;
	} else {
		IDMAP_LOG(0, ("ERROR: umich_id_to_name: invalid idtype (%d)",
			  idtype));
		err = -EINVAL;
		goto out;
	}

	if (ldap_init_and_bind(&ld, &sizelimit, linfo))
		goto out;

	if (idtype == IDTYPE_USER)
		attrs[0] = ldap_map.NFSv4_nfsname_attr;
	else
		attrs[0] = ldap_map.NFSv4_group_nfsname_attr;
	attrs[1] = NULL;

	err = ldap_search_st(ld, base, LDAP_SCOPE_SUBTREE,
			 filter, (char **)attrs,
			 0, &timeout, &result);
	if (err) {
		char * errmsg;

		IDMAP_LOG(2, ("umich_id_to_name: ldap_search_st for "
			  "base '%s, filter '%s': %s (%d)", base, filter,
			  ldap_err2string(err), err));
                if ((ldap_get_option(ld, LDAP_OPT_ERROR_STRING, &errmsg) == LDAP_SUCCESS)
				&& (errmsg != NULL) && (*errmsg != '\0')) {
			IDMAP_LOG(2, ("umich_id_to_name: "
				  "Additional info: %s", errmsg));
			ldap_memfree(errmsg);
		}

		err = -ENOENT;
		goto out_unbind;
	}

	err = -ENOENT;
	count = ldap_count_entries(ld, result);
	if (count != 1)
		goto out_unbind;

	if (!(entry = ldap_first_entry(ld, result))) {
		lerr = ldap_result2error(ld, result, 0);
		IDMAP_LOG(2, ("umich_id_to_name: ldap_first_entry: "
			  "%s (%d)", ldap_err2string(lerr), lerr));
		goto out_unbind;
	}

	if (!(attr_res = ldap_first_attribute(ld, result, &ber))) {
		lerr = ldap_result2error(ld, result, 0);
		IDMAP_LOG(2, ("umich_id_to_name: ldap_first_attribute: "
			  "%s (%d)", ldap_err2string(lerr), lerr));
		goto out_unbind;
	}

	if ((names = ldap_get_values(ld, result, attr_res)) == NULL) {
		lerr = ldap_result2error(ld, result, 0);
		IDMAP_LOG(2, ("umich_id_to_name: ldap_get_values: "
			  "%s (%d)", ldap_err2string(lerr), lerr));
		goto out_memfree;
	}

	/*
	 * Verify there is enough room in the output buffer before
	 * copying returned string. (strlen doesn't count the null,
	 * we make sure there is room for the null also, therefore
	 * we use ">=" not just ">")
	 */
	if (strlen(names[0]) >= len) {
		/* not enough space to return the name */
		IDMAP_LOG(1, ("umich_id_to_name: output buffer size (%d) "
			  "too small to return string, '%s', of length %d",
			  len, names[0], strlen(names[0])));
		goto out_memfree;
	}
	strcpy(*name, names[0]);

	err = 0;
out_memfree:
	if (names)
		ldap_value_free(names);
	ldap_memfree(attr_res);
	ber_free(ber, 0);
out_unbind:
	if (result)
		ldap_msgfree(result);
	ldap_unbind(ld);
out:
	return err;
}

static int
umich_gss_princ_to_grouplist(char *principal, gid_t *groups, int *ngroups,
			     struct umich_ldap_info *linfo)
{
	LDAP *ld = NULL;
	struct timeval timeout = {
		.tv_sec = linfo->ldap_timeout,
	};
	LDAPMessage *result, *entry;
	char **names, filter[LDAP_FILT_MAXSIZ];
	char *attrs[2];
	int count = 0, err = -ENOMEM, lerr, f_len;
        int i, num_gids;
	gid_t *curr_group = groups;

	err = -EINVAL;
	if (linfo == NULL || linfo->server == NULL ||
		linfo->people_tree == NULL || linfo->group_tree == NULL)
		goto out;


	if (ldap_init_and_bind(&ld, NULL, linfo))
		goto out;

	/*
	 * First we need to map the gss principal name to a uid (name) string
	 */
	err = -EINVAL;
	if ((f_len = snprintf(filter, LDAP_FILT_MAXSIZ,
			     "(&(objectClass=%s)(%s=%s))",
			     ldap_map.NFSv4_person_objcls,
			     ldap_map.GSS_principal_attr, principal))
			== LDAP_FILT_MAXSIZ) {
		IDMAP_LOG(0, ("ERROR: umich_gss_princ_to_grouplist: "
			  "filter too long!"));
		goto out;
	}

	attrs[0] = ldap_map.NFSv4_acctname_attr;
	attrs[1] = NULL;

	err = ldap_search_st(ld, linfo->people_tree, LDAP_SCOPE_SUBTREE,
			 filter, attrs, 0, &timeout, &result);
	if (err) {
		char *errmsg;

		IDMAP_LOG(2, ("umich_gss_princ_to_grouplist: ldap_search_st "
			  "for tree '%s, filter '%s': %s (%d)",
			  linfo->people_tree, filter,
			  ldap_err2string(err), err));
		if ((ldap_get_option(ld, LDAP_OPT_ERROR_STRING, &errmsg) == LDAP_SUCCESS)
				&& (errmsg != NULL) && (*errmsg != '\0')) {
			IDMAP_LOG(2, ("umich_gss_princ_to_grouplist: "
				   "Additional info: %s", errmsg));
			ldap_memfree(errmsg);
		}
		err = -ENOENT;
		goto out_unbind;
	}

	err = -ENOENT;
	count = ldap_count_entries(ld, result);
	if (count != 1) {
		IDMAP_LOG(2, ("umich_gss_princ_to_grouplist: "
                                "ldap account lookup of gssauthname %s returned %d accounts",
                                principal,count));
		goto out_unbind;
	}

	if (!(entry = ldap_first_entry(ld, result))) {
		lerr = ldap_result2error(ld, result, 0);
		IDMAP_LOG(2, ("umich_gss_princ_to_grouplist: ldap_first_entry: "
			  "%s (%d)", ldap_err2string(lerr), lerr));
		goto out_unbind;
	}

	if ((names = ldap_get_values(ld, result, attrs[0])) == NULL) {
		lerr = ldap_result2error(ld, result, 0);
		IDMAP_LOG(2, ("umich_gss_princ_to_grouplist: ldap_get_values: "
			  "%s (%d)", ldap_err2string(lerr), lerr));
		goto out_unbind;
	}

        if (ldap_info.memberof_for_groups) {

            /*
             * Collect the groups the user belongs to
             */
            if ((f_len = snprintf(filter, LDAP_FILT_MAXSIZ,
                        "(&(objectClass=%s)(%s=%s))",
                        ldap_map.NFSv4_person_objcls,
                        ldap_map.NFSv4_acctname_attr,
                        names[0])) == LDAP_FILT_MAXSIZ ) {
                IDMAP_LOG(2, ("ERROR: umich_gss_princ_to_grouplist: "
                          "filter too long!"));
                ldap_value_free(names);
                goto out_unbind;
            }

            ldap_value_free(names);

            attrs[0] = ldap_map.NFSv4_member_of_attr;
            attrs[1] = NULL;

            err = ldap_search_st(ld, linfo->people_tree, LDAP_SCOPE_SUBTREE,
                         filter, attrs, 0, &timeout, &result);

            if (err) {
                char *errmsg;

                IDMAP_LOG(2, ("umich_gss_princ_to_grouplist: ldap_search_st "
                          "for tree '%s, filter '%s': %s (%d)",
                          linfo->people_tree, filter,
                          ldap_err2string(err), err));
                if ((ldap_get_option(ld, LDAP_OPT_ERROR_STRING, &errmsg) == LDAP_SUCCESS)
                                && (errmsg != NULL) && (*errmsg != '\0')) {
                        IDMAP_LOG(2, ("umich_gss_princ_to_grouplist: "
                                   "Additional info: %s", errmsg));
                        ldap_memfree(errmsg);
                }
                err = -ENOENT;
                goto out_unbind;
            }
	    err = -ENOENT;

            /* pull the list of groups and place into names */
            count = ldap_count_entries(ld, result);
            if (count != 1) {
                IDMAP_LOG(2, ("umich_gss_princ_to_grouplist: "
                    "ldap group member lookup of gssauthname %s returned %d multiple entries",
                         principal,count));
                goto out_unbind;
            }

            if (!(entry = ldap_first_entry(ld, result))) {
                lerr = ldap_result2error(ld, result, 0);
                IDMAP_LOG(2, ("umich_gss_princ_to_grouplist: ldap_first_entry: "
                          "%s (%d)", ldap_err2string(lerr), lerr));
                goto out_unbind;
            }

            if ((names = ldap_get_values(ld, result, attrs[0])) == NULL) {
                lerr = ldap_result2error(ld, result, 0);
                IDMAP_LOG(2, ("umich_gss_princ_to_grouplist: ldap_get_values: "
                          "%s (%d)", ldap_err2string(lerr), lerr));
                goto out_unbind;
            }

	    /*  Count the groups first before doing a lookup of the group.
		If it exceeds the desired number of groups set the needed value
		and abort. */
	    for (i = 0; names[i] != NULL; i++);
	    if ( i > *ngroups ) {
		ldap_value_free(names);
		err = -EINVAL;
		IDMAP_LOG(2, ("umich_gss_princ_to_grouplist: User %s, "
			  "number of groups %d, exceeds requested number %d",
			  principal, i, *ngroups));
		*ngroups = i;
		goto out_unbind;
            }

            /* Loop through the groupnames (names) and get the group gid */
	    num_gids = 0;
            for (i = 0; names[i] != NULL; i++){
	      char **vals;
              int valcount;
              unsigned long tmp_g;
              gid_t tmp_gid;
	      char *cnptr = NULL;

		cnptr = strchr(names[i],',');
		if (cnptr) *cnptr = '\0';

		err = -ENOENT;
		if (ldap_map.NFSv4_grouplist_filter)
        	    f_len = snprintf(filter, LDAP_FILT_MAXSIZ,
                        "(&(objectClass=%s)(%s)%s)",
                        ldap_map.NFSv4_group_objcls,
                        names[i],
			ldap_map.NFSv4_grouplist_filter);
		else
		    f_len = snprintf(filter, LDAP_FILT_MAXSIZ,
                        "(&(objectClass=%s)(%s))",
                        ldap_map.NFSv4_group_objcls,
                        names[i]);

		if ( f_len == LDAP_FILT_MAXSIZ ) {
                		IDMAP_LOG(2, ("ERROR: umich_gss_princ_to_grouplist: "
                          		"filter too long!"));
                		ldap_value_free(names);
                		goto out_unbind;
        	}
		attrs[0] = ldap_map.NFSv4_gid_attr;
        	attrs[1] = NULL;

        	err = ldap_search_st(ld, linfo->group_tree, LDAP_SCOPE_SUBTREE,
                         filter, attrs, 0, &timeout, &result);
		if (err) {
                  char *errmsg;

                	IDMAP_LOG(2, ("umich_gss_princ_to_grouplist: ldap_search_st "
                          "for tree '%s, filter '%s': %s (%d)",
                          linfo->group_tree, filter,
                          ldap_err2string(err), err));
                	if ((ldap_get_option(ld, LDAP_OPT_ERROR_STRING, &errmsg)==LDAP_SUCCESS)
						&&
                                (errmsg != NULL) && (*errmsg != '\0')) {
                        	IDMAP_LOG(2, ("umich_gss_princ_to_grouplist: "
                                   "Additional info: %s", errmsg));
                        	ldap_memfree(errmsg);
                	}
                	continue;
        	}

		count = ldap_count_entries(ld, result);
		if (count == 0)
			continue;
		if (count != 1 ){
			IDMAP_LOG(2, ("umich_gss_princ_to_grouplist:"
				"Group %s has %d gids defined - aborting", names[i], count));
			ldap_value_free(names);
			err = -ENOENT;
			goto out_unbind;
		}

                vals = ldap_get_values(ld, result, ldap_map.NFSv4_gid_attr);

                /* There should be only one gidNumber attribute per group */
                if ((valcount = ldap_count_values(vals)) != 1) {
                        IDMAP_LOG(2, ("DB problem getting gidNumber of "
                                  "posixGroup! (count was %d)", valcount));
			ldap_value_free(vals);
                        continue;
                }

                tmp_g = strtoul(vals[0], (char **)NULL, 10);
                tmp_gid = tmp_g;
                if (tmp_gid != tmp_g ||
                                (errno == ERANGE && tmp_g == ULONG_MAX)) {
                        IDMAP_LOG(2, ("ERROR: umich_gss_princ_to_grouplist: "
                                  "gidNumber too long converting '%s'",
                                  vals[0]));
                        ldap_value_free(vals);
                        continue;
                }
                *curr_group++ = tmp_gid;
		num_gids++;
                ldap_value_free(vals);
            }
	    ldap_value_free(names);
	    *ngroups = num_gids;
	    err = 0;
	} else {

	    /*
	     * Then determine the groups that uid (name) string is a member of
	     */
	    err = -EINVAL;
	    if (ldap_map.NFSv4_grouplist_filter)
	    	f_len = snprintf(filter, LDAP_FILT_MAXSIZ,
                        "(&(objectClass=%s)(%s=%s)%s)",
                        ldap_map.NFSv4_group_objcls,
                        ldap_map.NFSv4_member_attr,
                        names[0],
			ldap_map.NFSv4_grouplist_filter);

            else
                f_len = snprintf(filter, LDAP_FILT_MAXSIZ,
                        "(&(objectClass=%s)(%s=%s))",
                        ldap_map.NFSv4_group_objcls,
                        ldap_map.NFSv4_member_attr,
                        names[0]);

            if ( f_len == LDAP_FILT_MAXSIZ ) {
		IDMAP_LOG(0, ("ERROR: umich_gss_princ_to_grouplist: "
			  "filter too long!"));
		ldap_value_free(names);
		goto out_unbind;
	    }

	    ldap_value_free(names);

	    attrs[0] = ldap_map.NFSv4_gid_attr;
	    attrs[1] = NULL;

            err = ldap_search_st(ld, linfo->group_tree, LDAP_SCOPE_SUBTREE,
			 filter, attrs, 0, &timeout, &result);

	    if (err) {
		char *errmsg;

		IDMAP_LOG(2, ("umich_gss_princ_to_grouplist: ldap_search_st "
			  "for tree '%s, filter '%s': %s (%d)",
			  linfo->group_tree, filter,
			  ldap_err2string(err), err));
		if ((ldap_get_option(ld, LDAP_OPT_ERROR_STRING, &errmsg) == LDAP_SUCCESS) &&
				(errmsg != NULL) && (*errmsg != '\0')) {
			IDMAP_LOG(2, ("umich_gss_princ_to_grouplist: "
				   "Additional info: %s", errmsg));
			ldap_memfree(errmsg);
		}
		err = -ENOENT;
		goto out_unbind;
	    }

	    /*
	     * If we can't determine count, return that error
	     * If we have nothing to return, return success
	     * If we have more than they asked for, tell them the
	     * number required and return an error
	     */
	    count = ldap_count_entries(ld, result);

	    if (count < 0) {
		err = count;
		goto out_unbind;
	    }
	    if (count == 0) {
		*ngroups = 0;
		err = 0;
		goto out_unbind;
	    }
	    if (count > *ngroups) {
		*ngroups = count;
		err = -EINVAL;
		goto out_unbind;
	    }
	    *ngroups = count;

	    curr_group = groups;

	    err = -ENOENT;
	    for (entry = ldap_first_entry(ld, result);
	     entry != NULL;
	     entry = ldap_next_entry(ld, entry)) {

		char **vals;
		int valcount;
		unsigned long tmp_g;
		gid_t tmp_gid;

		vals = ldap_get_values(ld, entry, ldap_map.NFSv4_gid_attr);

		/* There should be only one gidNumber attribute per group */
		if ((valcount = ldap_count_values(vals)) != 1) {
			IDMAP_LOG(0, ("DB problem getting gidNumber of "
				  "posixGroup! (count was %d)", valcount));
			goto out_unbind;
		}
		tmp_g = strtoul(vals[0], (char **)NULL, 10);
		tmp_gid = tmp_g;
		if (tmp_gid != tmp_g ||
				(errno == ERANGE && tmp_g == ULONG_MAX)) {
			IDMAP_LOG(0, ("ERROR: umich_gss_princ_to_grouplist: "
				  "gidNumber too long converting '%s'",
				  vals[0]));
			ldap_value_free(vals);
			goto out_unbind;
		}
		*curr_group++ = tmp_gid;
		ldap_value_free(vals);
	    }
	    err = 0;
	}

out_unbind:
	ldap_unbind(ld);
out:
	return err;
}


/*
 * principal:   krb5  - princ@realm, use KrbName ldap attribute
 *              spkm3 - X.509 dn, use X509Name ldap attribute
 */
static int
umichldap_gss_princ_to_ids(char *secname, char *principal,
			   uid_t *uid, gid_t *gid,
			   extra_mapping_params **UNUSED(ex))
{
	uid_t rtnd_uid = -1;
	gid_t rtnd_gid = -1;
	int err = -EINVAL;

	if ((strcmp(secname, "krb5") != 0) && (strcmp(secname, "spkm3") != 0)) {
		IDMAP_LOG(0, ("ERROR: umichldap_gss_princ_to_ids: "
			  "invalid secname '%s'", secname));
		return err;
	}

	err = umich_name_to_ids(principal, IDTYPE_USER, &rtnd_uid, &rtnd_gid,
			ldap_map.GSS_principal_attr, &ldap_info);
	if (err < 0)
		goto out;

	*uid = rtnd_uid;
	*gid = rtnd_gid;
out:
	return err;
}

static int
umichldap_name_to_uid(char *name, uid_t *uid)
{
	gid_t gid;

	return umich_name_to_ids(name, IDTYPE_USER, uid,
				 &gid, ldap_map.NFSv4_nfsname_attr, &ldap_info);
}

static int
umichldap_name_to_gid(char *name, gid_t *gid)
{
	uid_t uid;

	return umich_name_to_ids(name, IDTYPE_GROUP, &uid, gid,
				 ldap_map.NFSv4_group_nfsname_attr, &ldap_info);
}

static int
umichldap_uid_to_name(uid_t uid, char *UNUSED(domain), char *name, size_t len)
{
	return umich_id_to_name(uid, IDTYPE_USER, &name, len, &ldap_info);
}

static int
umichldap_gid_to_name(gid_t gid, char *UNUSED(domain), char *name, size_t len)
{
	return umich_id_to_name(gid, IDTYPE_GROUP, &name, len, &ldap_info);
}

static int
umichldap_gss_princ_to_grouplist(char *secname, char *principal,
		gid_t *groups, int *ngroups, extra_mapping_params **UNUSED(ex))
{
	int err = -EINVAL;

	if ((strcmp(secname, "krb5") != 0) && (strcmp(secname, "spkm3") != 0)) {
		IDMAP_LOG(0, ("ERROR: umichldap_gss_princ_to_grouplist: "
			  "invalid secname '%s'", secname));
		return err;
	}

	return umich_gss_princ_to_grouplist(principal, groups, ngroups,
					    &ldap_info);
}

/*
 * TLS connections require that the hostname we specify matches
 * the hostname in the certificate that the server uses.
 * Get a canonical name for the host specified in the config file.
 */
static char *
get_canonical_hostname(const char *inname)
{
	int aierr, error;
	struct addrinfo *ap, aihints;
	char *return_name = NULL;
	char tmphost[NI_MAXHOST];

	memset(&aihints, 0, sizeof(aihints));
	aihints.ai_socktype = SOCK_STREAM;
	aihints.ai_flags = AI_CANONNAME;
	aihints.ai_family = PF_INET;
	aierr = getaddrinfo(inname, NULL, &aihints, &ap);
	if (aierr) {
		const char *msg;
		/* We want to customize some messages.  */
		switch (aierr) {
		case EAI_NONAME:
			msg = "host unknown";
			break;
		default:
			msg = gai_strerror(aierr);
			break;
		}
		IDMAP_LOG(1, ("%s: '%s': %s", __FUNCTION__, inname, msg));
		goto out_err;
	}
	if (ap == 0) {
		IDMAP_LOG(1, ("%s: no addresses for host '%s'?",
			  __FUNCTION__, inname));
		goto out_err;
	}

	error = getnameinfo (ap->ai_addr, ap->ai_addrlen, tmphost,
			     sizeof(tmphost), NULL, 0, 0);
	if (error) {
		IDMAP_LOG(1, ("%s: getnameinfo for host '%s' failed (%d)",
			  __FUNCTION__, inname));
		goto out_free;
	}
	return_name = strdup (tmphost);

out_free:
	nfs_freeaddrinfo(ap);
out_err:
	return return_name;
}

static int
umichldap_init(void)
{
	char *tssl, *canonicalize, *memberof, *cert_req, *follow_referrals;
	char missing_msg[128] = "";
	char *server_in, *canon_name;

	if (nfsidmap_conf_path)
		conf_init_file(nfsidmap_conf_path);

	server_in = conf_get_str(LDAP_SECTION, "LDAP_server");
	ldap_info.base = conf_get_str(LDAP_SECTION, "LDAP_base");
	ldap_info.people_tree = conf_get_str(LDAP_SECTION, "LDAP_people_base");
	ldap_info.group_tree = conf_get_str(LDAP_SECTION, "LDAP_group_base");
	ldap_info.user_dn = conf_get_str(LDAP_SECTION, "LDAP_user_dn");
	ldap_info.passwd = conf_get_str(LDAP_SECTION, "LDAP_passwd");
	tssl = conf_get_str_with_def(LDAP_SECTION, "LDAP_use_ssl", "false");
	if ((strcasecmp(tssl, "true") == 0) ||
	    (strcasecmp(tssl, "on") == 0) ||
	    (strcasecmp(tssl, "yes") == 0))
		ldap_info.use_ssl = 1;
	else
		ldap_info.use_ssl = 0;
	ldap_info.ca_cert = conf_get_str(LDAP_SECTION, "LDAP_CA_CERT");
	cert_req = conf_get_str(LDAP_SECTION, "LDAP_tls_reqcert");
	if (cert_req != NULL) {
		if (strcasecmp(cert_req, "hard") == 0)
			ldap_info.tls_reqcert = LDAP_OPT_X_TLS_HARD;
		else if (strcasecmp(cert_req, "demand") == 0)
			ldap_info.tls_reqcert = LDAP_OPT_X_TLS_DEMAND;
		else if (strcasecmp(cert_req, "try") == 0)
			ldap_info.tls_reqcert = LDAP_OPT_X_TLS_TRY;
		else if (strcasecmp(cert_req, "allow") == 0)
			ldap_info.tls_reqcert = LDAP_OPT_X_TLS_ALLOW;
		else if (strcasecmp(cert_req, "never") == 0)
			ldap_info.tls_reqcert = LDAP_OPT_X_TLS_NEVER;
		else {
			IDMAP_LOG(0, ("umichldap_init: Invalid value(%s) for "
				      "LDAP_tls_reqcert."));
			goto fail;
		}
	}
	/* vary the default port depending on whether they use SSL or not */
	ldap_info.port = conf_get_num(LDAP_SECTION, "LDAP_port",
				      (ldap_info.use_ssl) ?
				      LDAPS_PORT : LDAP_PORT);

	ldap_info.sasl_mech = conf_get_str(LDAP_SECTION, "LDAP_sasl_mech");
	ldap_info.sasl_realm = conf_get_str(LDAP_SECTION, "LDAP_sasl_realm");
	ldap_info.sasl_authcid = conf_get_str(LDAP_SECTION,
					      "LDAP_sasl_authcid");
	ldap_info.sasl_authzid = conf_get_str(LDAP_SECTION,
					      "LDAP_sasl_authzid");
	ldap_info.sasl_secprops = conf_get_str(LDAP_SECTION,
					       "LDAP_sasl_secprops");

	/* If it is not set let the ldap lib work with the lib default */
	canonicalize = conf_get_str_with_def(LDAP_SECTION,
					     "LDAP_sasl_canonicalize", "undef");
	if ((strcasecmp(canonicalize, "true") == 0) ||
	    (strcasecmp(canonicalize, "on") == 0) ||
	    (strcasecmp(canonicalize, "yes") == 0)) {
		ldap_info.sasl_canonicalize = 1;
	} else if ((strcasecmp(canonicalize, "false") == 0) ||
	    (strcasecmp(canonicalize, "off") == 0) ||
	    (strcasecmp(canonicalize, "no") == 0)) {
		ldap_info.sasl_canonicalize = 0;
	}
	ldap_info.sasl_krb5_ccname = conf_get_str(LDAP_SECTION,
						  "LDAP_sasl_krb5_ccname");

	follow_referrals = conf_get_str_with_def(LDAP_SECTION,
						 "LDAP_follow_referrals",
						 "true");
	if ((strcasecmp(follow_referrals, "true") == 0) ||
	    (strcasecmp(follow_referrals, "on") == 0) ||
	    (strcasecmp(follow_referrals, "yes") == 0))
		ldap_info.follow_referrals = 1;
	else
		ldap_info.follow_referrals = 0;

	/* Verify required information is supplied */
	if (server_in == NULL || strlen(server_in) == 0)
		strncat(missing_msg, "LDAP_server ", sizeof(missing_msg)-1);
	if (ldap_info.base == NULL || strlen(ldap_info.base) == 0)
		strncat(missing_msg, "LDAP_base ", sizeof(missing_msg)-1);
	if (strlen(missing_msg) != 0) {
		IDMAP_LOG(0, ("umichldap_init: Missing required information: "
			  "%s", missing_msg));
		goto fail;
	}

	ldap_info.server = server_in;
	canonicalize = conf_get_str_with_def(LDAP_SECTION,
					     "LDAP_canonicalize_name", "yes");
	if ((strcasecmp(canonicalize, "true") == 0) ||
	    (strcasecmp(canonicalize, "on") == 0) ||
	    (strcasecmp(canonicalize, "yes") == 0)) {
		canon_name = get_canonical_hostname(server_in);
		if (canon_name == NULL)
			IDMAP_LOG(0, ("umichldap_init: Warning! Unable to "
				  "canonicalize server name '%s' as requested.",
				  server_in));
		else
			ldap_info.server = canon_name;
	}

	/* get the ldap mapping attributes/objectclasses (all have defaults) */
	ldap_map.NFSv4_person_objcls =
		conf_get_str_with_def(LDAP_SECTION, "NFSv4_person_objectclass",
				      DEFAULT_UMICH_OBJCLASS_REMOTE_PERSON);

	ldap_map.NFSv4_group_objcls =
		conf_get_str_with_def(LDAP_SECTION, "NFSv4_group_objectclass",
				      DEFAULT_UMICH_OBJCLASS_REMOTE_GROUP);

	ldap_map.NFSv4_nfsname_attr =
		conf_get_str_with_def(LDAP_SECTION, "NFSv4_name_attr",
				      DEFAULT_UMICH_ATTR_NFSNAME);

	ldap_map.NFSv4_uid_attr =
		conf_get_str_with_def(LDAP_SECTION, "NFSv4_uid_attr",
				      DEFAULT_UMICH_ATTR_UIDNUMBER);

	ldap_map.NFSv4_acctname_attr =
		conf_get_str_with_def(LDAP_SECTION, "NFSv4_acctname_attr",
				      DEFAULT_UMICH_ATTR_ACCTNAME);

	ldap_map.NFSv4_group_nfsname_attr =
		conf_get_str_with_def(LDAP_SECTION, "NFSv4_group_attr",
				      DEFAULT_UMICH_ATTR_GROUP_NFSNAME);

	ldap_map.NFSv4_gid_attr =
		conf_get_str_with_def(LDAP_SECTION, "NFSv4_gid_attr",
				      DEFAULT_UMICH_ATTR_GIDNUMBER);

	ldap_map.NFSv4_member_attr =
		conf_get_str_with_def(LDAP_SECTION, "NFSv4_member_attr",
				      DEFAULT_UMICH_ATTR_MEMBERUID);

	ldap_map.GSS_principal_attr =
		conf_get_str_with_def(LDAP_SECTION, "GSS_principal_attr",
				      DEFAULT_UMICH_ATTR_GSSAUTHNAME);

	ldap_map.NFSv4_grouplist_filter =
		conf_get_str_with_def(LDAP_SECTION, "NFSv4_grouplist_filter",
				      NULL);

	ldap_map.NFSv4_member_of_attr =
		conf_get_str_with_def(LDAP_SECTION, "NFSv4_member_of_attr",
				      DEFAULT_UMICH_ATTR_MEMBEROF);

	ldap_info.ldap_timeout =
		conf_get_num(LDAP_SECTION, "LDAP_timeout_seconds",
                                      DEFAULT_UMICH_SEARCH_TIMEOUT);


 	/*
	 * Some LDAP servers do a better job with indexing where searching
	 * through all the groups searching for the user in the memberuid
	 * list.  Others like SunOne directory that search can takes minutes
	 * if there are thousands of groups. So setting
	 * LDAP_use_memberof_for_groups to true in the configuration file
	 * will use the memberof lists of the account and search through
	 * only those groups to obtain gids.
	 */
	memberof = conf_get_str_with_def(LDAP_SECTION,
				"LDAP_use_memberof_for_groups", "false");
        if ((strcasecmp(memberof, "true") == 0) ||
            (strcasecmp(memberof, "on") == 0) ||
            (strcasecmp(memberof, "yes") == 0))
                ldap_info.memberof_for_groups = 1;
        else
                ldap_info.memberof_for_groups = 0;

	/*
	 * If they specified a search base for the
	 * people tree or group tree we use that.
	 * Otherwise we use the default search base.
	 * Note:  We no longer append the default base to the tree --
	 * that should already be specified.
	 * this functions much like the NSS_LDAP modules
	 */
	if (ldap_info.people_tree == NULL || strlen(ldap_info.people_tree) == 0)
		ldap_info.people_tree = ldap_info.base;
	if (ldap_info.group_tree == NULL || strlen(ldap_info.group_tree) == 0)
		ldap_info.group_tree = ldap_info.base;

	if (ldap_info.use_ssl &&
	    ldap_info.tls_reqcert != LDAP_OPT_X_TLS_NEVER &&
	    ldap_info.ca_cert == NULL) {
		IDMAP_LOG(0, ("umichldap_init: You must specify LDAP_ca_cert "
			  "with LDAP_use_ssl=yes and "
			  "LDAP_tls_reqcert not set to \"never\""));
		goto fail;
	}


	/* print out some good debugging info */
	IDMAP_LOG(1, ("umichldap_init: canonicalize_name: %s",
		  canonicalize));
	IDMAP_LOG(1, ("umichldap_init: server  : %s (from config value '%s')",
		  ldap_info.server, server_in));
	IDMAP_LOG(1, ("umichldap_init: port    : %d", ldap_info.port));
	IDMAP_LOG(1, ("umichldap_init: people  : %s", ldap_info.people_tree));
	IDMAP_LOG(1, ("umichldap_init: groups  : %s", ldap_info.group_tree));

	IDMAP_LOG(1, ("umichldap_init: user_dn : %s",
		  (ldap_info.user_dn && strlen(ldap_info.user_dn) != 0)
		  ? ldap_info.user_dn : "<not-supplied>"));
	/* Don't print actual password into the log. */
	IDMAP_LOG(1, ("umichldap_init: passwd  : %s",
		  (ldap_info.passwd && strlen(ldap_info.passwd) != 0) ?
		  "<supplied>" : "<not-supplied>"));
	IDMAP_LOG(1, ("umichldap_init: use_ssl : %s",
		  ldap_info.use_ssl ? "yes" : "no"));
	IDMAP_LOG(1, ("umichldap_init: ca_cert : %s",
		  ldap_info.ca_cert ? ldap_info.ca_cert : "<not-supplied>"));
	IDMAP_LOG(1, ("umichldap_init: tls_reqcert : %s(%d)",
		  cert_req ? cert_req : "<not-supplied>",
		  ldap_info.tls_reqcert));
	IDMAP_LOG(1, ("umichldap_init: use_memberof_for_groups : %s",
		  ldap_info.memberof_for_groups ? "yes" : "no"));
	IDMAP_LOG(1, ("umichldap_init: sasl_mech: %s",
		  (ldap_info.sasl_mech && strlen(ldap_info.sasl_mech) != 0) ?
		  ldap_info.sasl_mech : "<not-supplied>"));
	IDMAP_LOG(1, ("umichldap_init: sasl_realm: %s",
		  (ldap_info.sasl_realm && strlen(ldap_info.sasl_realm) != 0) ?
		  ldap_info.sasl_realm : "<not-supplied>"));
	IDMAP_LOG(1, ("umichldap_init: sasl_authcid: %s",
		  (ldap_info.sasl_authcid &&
		   strlen(ldap_info.sasl_authcid) != 0) ?
		  ldap_info.sasl_authcid : "<not-supplied>"));
	IDMAP_LOG(1, ("umichldap_init: sasl_authzid: %s",
		  (ldap_info.sasl_authzid &&
		   strlen(ldap_info.sasl_authzid) != 0) ?
		  ldap_info.sasl_authzid : "<not-supplied>"));
	IDMAP_LOG(1, ("umichldap_init: sasl_secprops: %s",
		  (ldap_info.sasl_secprops &&
		   strlen(ldap_info.sasl_secprops) != 0) ?
		  ldap_info.sasl_secprops : "<not-supplied>"));
	IDMAP_LOG(1, ("umichldap_init: sasl_canonicalize: %d",
		      ldap_info.sasl_canonicalize));
	IDMAP_LOG(1, ("umichldap_init: sasl_krb5_ccname: %s",
		      ldap_info.sasl_krb5_ccname));
	IDMAP_LOG(1, ("umichldap_init: follow_referrals: %s",
		  ldap_info.follow_referrals ? "yes" : "no"));

	IDMAP_LOG(1, ("umichldap_init: NFSv4_person_objectclass : %s",
		  ldap_map.NFSv4_person_objcls));
	IDMAP_LOG(1, ("umichldap_init: NFSv4_nfsname_attr       : %s",
		  ldap_map.NFSv4_nfsname_attr));
	IDMAP_LOG(1, ("umichldap_init: NFSv4_acctname_attr      : %s",
		  ldap_map.NFSv4_acctname_attr));
	IDMAP_LOG(1, ("umichldap_init: NFSv4_uid_attr           : %s",
		  ldap_map.NFSv4_uid_attr));
	IDMAP_LOG(1, ("umichldap_init: NFSv4_group_objectclass  : %s",
		  ldap_map.NFSv4_group_objcls));
	IDMAP_LOG(1, ("umichldap_init: NFSv4_gid_attr           : %s",
		  ldap_map.NFSv4_gid_attr));
	IDMAP_LOG(1, ("umichldap_init: NFSv4_group_nfsname_attr : %s",
		  ldap_map.NFSv4_group_nfsname_attr));
	IDMAP_LOG(1, ("umichldap_init: NFSv4_member_attr        : %s",
		  ldap_map.NFSv4_member_attr));
	IDMAP_LOG(1, ("umichldap_init: NFSv4_member_of_attr     : %s",
		  ldap_map.NFSv4_member_of_attr));
	IDMAP_LOG(1, ("umichldap_init: NFSv4_grouplist_filter   : %s",
		  ldap_map.NFSv4_grouplist_filter ?
		  ldap_map.NFSv4_grouplist_filter : "<not-specified>"));
	IDMAP_LOG(1, ("umichldap_init: GSS_principal_attr       : %s",
		  ldap_map.GSS_principal_attr));
	return 0;
fail:
  	return -1;
}


/* The external interface */

struct trans_func umichldap_trans = {
	.name		= "umich_ldap",
	.init		= umichldap_init,
	.princ_to_ids   = umichldap_gss_princ_to_ids,
	.name_to_uid    = umichldap_name_to_uid,
	.name_to_gid    = umichldap_name_to_gid,
	.uid_to_name    = umichldap_uid_to_name,
	.gid_to_name    = umichldap_gid_to_name,
	.gss_princ_to_grouplist = umichldap_gss_princ_to_grouplist,
};

struct trans_func *libnfsidmap_plugin_init(void)
{
	return (&umichldap_trans);
}
