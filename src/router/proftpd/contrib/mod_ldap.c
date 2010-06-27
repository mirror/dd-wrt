/*
 * mod_ldap - LDAP password lookup module for ProFTPD
 * Copyright (c) 1999-2010, John Morrissey <jwm@horde.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * Furthermore, John Morrissey gives permission to link this program with
 * OpenSSL, and distribute the resulting executable, without including the
 * source code for OpenSSL in the source distribution.
 */

/*
 * mod_ldap v2.8.22
 *
 * Thanks for patches go to (in alphabetical order):
 *
 * Peter Fabian (fabian at staff dot matavnet dot hu) - LDAPAuthBinds
 * Alexandre Francois (alexandre-francois at voila dot fr) - LDAPAliasDereference
 * Marek Gradzki (mgradzki at ost dot net dot pl) - LDAPProtocolVersion
 * Pierrick Hascoet (pierrick at alias dot fr) - OpenSSL password hash support
 * Florian Lohoff (flo at rfc822 dot org) - LDAPForceDefault[UG]ID code
 * Steve Luzynski (steve at uniteone dot net) - HomedirOnDemandPrefix support
 * Gaute Nessan (gaute at kpnqwest dot no) - OpenLDAP 2.0 fixes
 * Marcin Obara (gryzzli at wp-sa dot pl) - User/group caching code, Sun
 *                                          LDAP library portability fixes
 * Phil Oester (phil at theoesters dot com) - Group code memory manip fixes
 * Michael Schout (mschout at gkg dot net) - Full-path HomedirOnDemand and
 *                                           multiple-HomedirOnDemandSuffix
 *                                           support
 * Klaus Steinberger (klaus dot steinberger at physik dot uni-muenchen dot de)
 *                                         - LDAPForceHomedirOnDemand support
 * Andreas Strodl (andreas at strodl dot org) - multiple group support
 * Ross Thomas (ross at grinfinity dot com) - Non-AuthBinds auth fix
 * Ivo Timmermans (ivo at debian dot org) - TLS support
 * Bert Vermeulen (bert at be dot easynet dot net) - LDAPHomedirOnDemand,
 *                                                   LDAPDefaultAuthScheme
 *
 *
 * $Id: mod_ldap.c,v 1.80 2010/02/02 23:06:25 jwm Exp $
 * $Libraries: -lldap -llber$
 */

/* To verify non-crypt() password hashes locally with OpenSSL, build ProFTPD
 * with the --enable-openssl argument to configure.
 */

#include "conf.h"
#include "privs.h"

#define MOD_LDAP_VERSION	"mod_ldap/2.8.22"

#if PROFTPD_VERSION_NUMBER < 0x0001030103
# error MOD_LDAP_VERSION " requires ProFTPD 1.3.1rc3 or later"
#endif

#if defined(HAVE_CRYPT_H) && !defined(AIX4) && !defined(AIX5)
# include <crypt.h>
#endif

#include <ctype.h> /* isdigit()   */
#include <errno.h>

#include <lber.h>
#include <ldap.h>

#if LDAP_API_VERSION >= 2000
# define HAS_LDAP_SASL_BIND_S
#endif

#if defined(LDAP_API_FEATURE_X_OPENLDAP) && (LDAP_VENDOR_VERSION >= 192)
# define HAS_LDAP_UNBIND_EXT_S
#endif

#if defined(LDAP_API_FEATURE_X_OPENLDAP) && (LDAP_VENDOR_VERSION >= 19905)
# define HAS_LDAP_INITIALIZE
#endif

#ifdef HAS_LDAP_UNBIND_EXT_S
# define LDAP_UNBIND(ld) (ldap_unbind_ext_s(ld, NULL, NULL))
#else
# define LDAP_UNBIND(ld) (ldap_unbind_s(ld))
static char *ldap_server;
static int ldap_port = LDAP_PORT;
#endif

#if LDAP_API_VERSION >= 2000
# define LDAP_VALUE_T struct berval
# define LDAP_GET_VALUES(ld, entry, attr) ldap_get_values_len(ld, entry, attr)
# define LDAP_VALUE(values, i) (values[i]->bv_val)
# define LDAP_COUNT_VALUES(values) (ldap_count_values_len(values))
# define LDAP_VALUE_FREE(values) (ldap_value_free_len(values))
# define LDAP_SEARCH(ld, base, scope, filter, attrs, timeout, sizelimit, res) \
   ldap_search_ext_s(ld, base, scope, filter, attrs, 0, NULL, NULL, \
                     timeout, sizelimit, res)
#else /* LDAP_API_VERSION >= 2000 */
# define LDAP_VALUE_T char
# define LDAP_GET_VALUES(ld, entry, attr) ldap_get_values(ld, entry, attr)
# define LDAP_VALUE(values, i) (values[i])
# define LDAP_COUNT_VALUES(values) (ldap_count_values(values))
# define LDAP_VALUE_FREE(values) (ldap_value_free(values))

static void
pr_ldap_set_sizelimit(LDAP *limit_ld, int limit)
{
#ifdef LDAP_OPT_SIZELIMIT
  int ret;
  if ((ret = ldap_set_option(limit_ld, LDAP_OPT_SIZELIMIT, (void *)&limit)) != LDAP_OPT_SUCCESS) {
    pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": pr_ldap_set_sizelimit(): ldap_set_option() unable to set query size limit to %d entries: %s", limit, ldap_err2string(ret));
  }
#else
  limit_ld->ld_sizelimit = limit;
#endif

  pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": set search size limit to %d", limit);
}

static int
LDAP_SEARCH(LDAP *ld, char *base, int scope, char *filter, char *attrs[],
            struct timeval *timeout, int sizelimit, LDAPMessage **res)
{
  pr_ldap_set_sizelimit(ld, sizelimit);
  return ldap_search_st(ld, base, scope, filter, attrs, 0, timeout, res);
}
#endif /* LDAP_API_VERSION >= 2000 */

/* Thanks, Sun. */
#ifndef LDAP_OPT_SUCCESS
# define LDAP_OPT_SUCCESS LDAP_SUCCESS
#endif
#ifndef LDAP_URL_SUCCESS
# define LDAP_URL_SUCCESS LDAP_SUCCESS
#endif
#ifndef LDAP_SCOPE_DEFAULT
# define LDAP_SCOPE_DEFAULT LDAP_SCOPE_SUBTREE
#endif

#if defined(HAVE_OPENSSL) || defined(PR_USE_OPENSSL)
# include <openssl/evp.h>
#endif

/* Config entries */
static array_header *ldap_servers = NULL;
static unsigned int cur_server_index = 0;
static char *ldap_dn, *ldap_dnpass,
            *ldap_auth_filter, *ldap_uid_filter,
            *ldap_group_gid_filter, *ldap_group_name_filter,
            *ldap_group_member_filter, *ldap_quota_filter,
            *ldap_ssh_pubkey_filter,
            *ldap_auth_basedn, *ldap_uid_basedn, *ldap_gid_basedn,
            *ldap_quota_basedn, *ldap_ssh_pubkey_basedn,
            *ldap_defaultauthscheme, *ldap_authbind_dn,
            *ldap_genhdir_prefix, *ldap_default_quota,
            *ldap_attr_uid = "uid",
            *ldap_attr_uidnumber = "uidNumber",
            *ldap_attr_gidnumber = "gidNumber",
            *ldap_attr_homedirectory = "homeDirectory",
            *ldap_attr_userpassword = "userPassword",
            *ldap_attr_loginshell = "loginShell",
            *ldap_attr_cn = "cn",
            *ldap_attr_memberuid = "memberUid",
            *ldap_attr_ftpquota = "ftpQuota",
            *ldap_attr_ftpquota_profiledn = "ftpQuotaProfileDN",
            *ldap_attr_ssh_pubkey = "sshPublicKey";
#ifdef HAS_LDAP_INITIALIZE
static char *ldap_server_url;
#endif /* HAS_LDAP_INITIALIZE */
static int ldap_doauth = 0, ldap_douid = 0, ldap_dogid = 0, ldap_doquota = 0,
           ldap_authbinds = 1, ldap_querytimeout = 0,
           ldap_genhdir = 0, ldap_genhdir_prefix_nouname = 0,
           ldap_forcedefaultuid = 0, ldap_forcedefaultgid = 0,
           ldap_forcegenhdir = 0, ldap_protocol_version = 3,
           ldap_dereference = LDAP_DEREF_NEVER,
           ldap_search_scope = LDAP_SCOPE_SUBTREE;
static struct timeval ldap_querytimeout_tp;

static uid_t ldap_defaultuid = -1;
static gid_t ldap_defaultgid = -1;

#ifdef LDAP_OPT_X_TLS
static int ldap_use_tls = 0;
#endif

static LDAP *ld = NULL;
static array_header *cached_quota = NULL;
static array_header *cached_ssh_pubkeys = NULL;


static void
pr_ldap_unbind(void)
{
  int ret;

  if (!ld) {
    pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": not unbinding to an already unbound connection.");
    return;
  }

  ret = LDAP_UNBIND(ld);
  if (ret != LDAP_SUCCESS) {
    pr_log_pri(PR_LOG_NOTICE, MOD_LDAP_VERSION ": pr_ldap_unbind(): unbind failed: %s", ldap_err2string(ret));
  } else {
    pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": successfully unbound");
  }

  ld = NULL;
}

static int
_ldap_connect(LDAP **conn_ld, int do_bind)
{
  int ret, version;
#ifdef HAS_LDAP_SASL_BIND_S
  struct berval bindcred;
#endif

#ifdef HAS_LDAP_INITIALIZE
  pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": attempting connection to %s", ldap_server_url ? ldap_server_url : "(null)");

  ret = ldap_initialize(conn_ld, ldap_server_url);
  if (ret != LDAP_SUCCESS) {
    pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": pr_ldap_connect(): ldap_initialize() to %s failed: %s", ldap_server_url ? ldap_server_url : "(null)", ldap_err2string(ret));
    ++cur_server_index;
    if (cur_server_index >= ldap_servers->nelts) {
      cur_server_index = 0;
    }
    *conn_ld = NULL;
    return -1;
  }
#else /* HAS_LDAP_INITIALIZE */
  pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": attempting connection to %s:%d", ldap_server ? ldap_server : "(null)", ldap_port);

  *conn_ld = ldap_init(ldap_server, ldap_port);
  if (!conn_ld) {
    pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": pr_ldap_connect(): ldap_init() to %s:%d failed: %s", ldap_server ? ldap_server : "(null)", ldap_port, strerror(errno));
    return -1;
  }
#endif /* HAS_LDAP_INITIALIZE */

  version = LDAP_VERSION3;
  if (ldap_protocol_version == 2) {
    version = LDAP_VERSION2;
  }

  ret = ldap_set_option(*conn_ld, LDAP_OPT_PROTOCOL_VERSION, &version);
  if (ret != LDAP_OPT_SUCCESS) {
    pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": pr_ldap_connect(): Setting LDAP version option failed: %s", ldap_err2string(ret));
    pr_ldap_unbind();
    return -1;
  }
  pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": set protocol version to %d", version);

#ifdef HAS_LDAP_INITIALIZE
  pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": connected to %s", ldap_server_url ? ldap_server_url : "(null)");
#else /* HAS_LDAP_INITIALIZE */
  pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": connected to %s:%d", ldap_server ? ldap_server : "(null)", ldap_port);
#endif /* HAS_LDAP_INITIALIZE */

#ifdef LDAP_OPT_X_TLS
  if (ldap_use_tls == 1) {
    ret = ldap_start_tls_s(*conn_ld, NULL, NULL);
    if (ret != LDAP_SUCCESS) {
      pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": pr_ldap_connect(): Starting TLS failed: %s", ldap_err2string(ret));
      pr_ldap_unbind();
      return -1;
    }
    pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": enabled TLS.");
  }
#endif /* LDAP_OPT_X_TLS */

  if (do_bind == TRUE) {
#ifdef HAS_LDAP_SASL_BIND_S
    bindcred.bv_val = ldap_dnpass;
    bindcred.bv_len = ldap_dnpass != NULL ? strlen(ldap_dnpass) : 0;
    ret = ldap_sasl_bind_s(*conn_ld, ldap_dn, NULL, &bindcred, NULL, NULL, NULL);
#else /* HAS_LDAP_SASL_BIND_S */
    ret = ldap_simple_bind_s(*conn_ld, ldap_dn, ldap_dnpass);
#endif /* HAS_LDAP_SASL_BIND_S */

    if (ret != LDAP_SUCCESS) {
      pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": pr_ldap_connect(): bind as %s failed: %s", ldap_dn ? ldap_dn : "(anonymous)", ldap_err2string(ret));
      pr_ldap_unbind();
      return -1;
    }
    pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": successfully bound as %s with password %s", ldap_dn ? ldap_dn : "(anonymous)", ldap_dnpass ? ldap_dnpass : "(none)");
  }

#ifdef LDAP_OPT_DEREF
  ret = ldap_set_option(*conn_ld, LDAP_OPT_DEREF, (void *)&ldap_dereference);
  if (ret != LDAP_OPT_SUCCESS) {
    pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": pr_ldap_connect(): ldap_set_option() unable to set dereference to %d: %s", ldap_dereference, ldap_err2string(ret));
    pr_ldap_unbind();
    return -1;
  }
#else
  deref_ld->ld_deref = ldap_dereference;
#endif
  pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": set dereferencing to %d", ldap_dereference);

  ldap_querytimeout_tp.tv_sec = (ldap_querytimeout > 0 ? ldap_querytimeout : 5);
  ldap_querytimeout_tp.tv_usec = 0;
  pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": set query timeout to %us", (unsigned)ldap_querytimeout_tp.tv_sec);

  return 1;
}

static int pr_ldap_connect(LDAP **conn_ld, int do_bind) {
  int start_server_index;
  char *item;
  LDAPURLDesc *url;

  if (!ldap_servers || ldap_servers->nelts == 0) {
    pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": pr_ldap_connect(): internal error: no LDAP servers configured.");
    return -1;
  }

  start_server_index = cur_server_index;
  do {
    item = ((char **)ldap_servers->elts)[cur_server_index];

    /* item might be NULL if no LDAPServer directive was specified
     * and we're using the SDK default.
     */
    if (item) {
      if (ldap_is_ldap_url(item)) {
        if (ldap_url_parse(item, &url) != LDAP_URL_SUCCESS) {
          pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": pr_ldap_connect(): url %s was valid during ProFTPD startup, but is no longer valid?!", item);

          ++cur_server_index;
          if (cur_server_index >= ldap_servers->nelts) {
            cur_server_index = 0;
          }
          continue;
        }

#ifdef HAS_LDAP_INITIALIZE
        ldap_server_url = item;
#else /* HAS_LDAP_INITIALIZE */
        /* Need to keep parsed host and port for pre-2000 ldap_init(). */
        if (url->lud_host != NULL) {
          ldap_server = pstrdup(session.pool, url->lud_host);
        }
        if (url->lud_port != 0) {
          ldap_port = url->lud_port;
        }
#endif /* HAS_LDAP_INITIALIZE */

        if (url->lud_scope != LDAP_SCOPE_DEFAULT) {
          ldap_search_scope = url->lud_scope;
          if (ldap_search_scope == LDAP_SCOPE_BASE) {
            pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": WARNING: LDAP URL search scopes default to 'base' (not 'sub') and may not be what you want.");
          }
        }

        ldap_free_urldesc(url);
      } else {
#ifdef HAS_LDAP_INITIALIZE
        ldap_server_url = pstrcat(session.pool, "ldap://", item, "/", NULL);
#else /* HAS_LDAP_INITIALIZE */
        ldap_server = pstrdup(session.pool, item);
        ldap_port = LDAP_PORT;
#endif /*  HAS_LDAP_INITIALIZE */
      }
    }

    if (_ldap_connect(conn_ld, do_bind) == 1) {
      return 1;
    }

    ++cur_server_index;
    if (cur_server_index >= ldap_servers->nelts) {
      cur_server_index = 0;
    }
  } while (cur_server_index != start_server_index);

  return -1;
}

static char *
pr_ldap_interpolate_filter(pool *p, char *template, const char *value)
{
  char *escaped_value, *filter;

  escaped_value = sreplace(p, (char *)value,
    "\\", "\\\\",
    "*", "\\*",
    "(", "\\(",
    ")", "\\)",
    NULL
  );
  if (!escaped_value) {
    return NULL;
  }

  filter = sreplace(p, template,
    "%u", escaped_value,
    "%v", escaped_value,
    NULL
  );
  if (!filter) {
    return NULL;
  }

  pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": generated filter %s from template %s and value %s", filter, template, value);
  return filter;
}

LDAPMessage *
pr_ldap_search(char *basedn, char *filter, char *ldap_attrs[], int sizelimit)
{
  int ret;
  LDAPMessage *result;

  if (!basedn) {
    pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": no LDAP base DN specified for auth/UID lookups, declining request.");
    return NULL;
  }

  /* If the LDAP connection has gone away or hasn't been established
   * yet, attempt to establish it now.
   */
  if (ld == NULL) {
    /* If we _still_ can't connect, give up and return NULL. */
    if (pr_ldap_connect(&ld, TRUE) == -1) {
      return NULL;
    }
  }

  ret = LDAP_SEARCH(ld, basedn, ldap_search_scope, filter, ldap_attrs,
    &ldap_querytimeout_tp, sizelimit, &result);
  if (ret != LDAP_SUCCESS) {
    if (ret == LDAP_SERVER_DOWN) {
      pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": pr_ldap_search(): LDAP server went away, trying to reconnect");

      if (pr_ldap_connect(&ld, TRUE) == -1) {
        pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": pr_ldap_search(): LDAP server went away, unable to reconnect");
        ld = NULL;
        return NULL;
      }

      pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": pr_ldap_search(): Reconnect to LDAP server successful, resuming normal operations");

      ret = LDAP_SEARCH(ld, basedn, ldap_search_scope, filter, ldap_attrs,
        &ldap_querytimeout_tp, 2, &result);
      if (ret != LDAP_SUCCESS) {
        pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": pr_ldap_search(): LDAP search failed: %s", ldap_err2string(ret));
        return NULL;
      }
    } else {
      pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": pr_ldap_search(): LDAP search failed: %s", ldap_err2string(ret));
      return NULL;
    }
  }
  pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": searched under base DN %s using filter %s", basedn, filter ? filter : "(null)");

  return result;
}

static struct passwd *
pr_ldap_user_lookup(pool *p,
                    char *filter_template, const char *replace,
                    char *basedn, char *ldap_attrs[],
                    char **user_dn)
{
  char *filter, *dn;
  int i = 0;
  struct passwd *pw;
  LDAPMessage *result, *e;
  LDAP_VALUE_T **values;

  filter = pr_ldap_interpolate_filter(p, filter_template, replace);
  if (!filter) {
    return NULL;
  }

  result = pr_ldap_search(basedn, filter, ldap_attrs, 2);
  if (result == NULL) {
    return NULL;
  }

  if (ldap_count_entries(ld, result) > 1) {
    pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": pr_ldap_user_lookup(): LDAP search returned multiple entries, aborting query");
    ldap_msgfree(result);
    return NULL;
  }

  e = ldap_first_entry(ld, result);
  if (!e) {
    ldap_msgfree(result);
    pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": no entries for filter %s under base DN %s", filter, basedn);
    return NULL; /* No LDAP entries for this user */
  }

  pw = pcalloc(session.pool, sizeof(struct passwd));
  while (ldap_attrs[i] != NULL) {
    pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": fetching value(s) for attr %s", ldap_attrs[i]);

    values = LDAP_GET_VALUES(ld, e, ldap_attrs[i]);
    if (values == NULL) {
      pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": no values for attribute %s, trying defaults...", ldap_attrs[i]);

      /* Try to fill in default values if there's no value for certain attrs. */

      /* If we can't find the [ug]idNumber attrs, just fill the passwd
         struct in with default values from the config file. */
      if (strcasecmp(ldap_attrs[i], ldap_attr_uidnumber) == 0) {
        if (ldap_defaultuid == -1) {
          pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": pr_ldap_user_lookup(): no %s attr for DN %s and LDAPDefaultUID was not specified!", (dn = ldap_get_dn(ld, e)), ldap_attr_uidnumber);
          free(dn);
          return NULL;
        }

        pw->pw_uid = ldap_defaultuid;
        ++i;
        pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": using default UID %lu", (unsigned long)pw->pw_uid);
        continue;
      }
      if (strcasecmp(ldap_attrs[i], ldap_attr_gidnumber) == 0) {
        if (ldap_defaultgid == -1) {
          pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": pr_ldap_user_lookup(): no %s attr for DN %s and LDAPDefaultGID was not specified!", (dn = ldap_get_dn(ld, e)), ldap_attr_gidnumber);
          free(dn);
          return NULL;
        }

        pw->pw_gid = ldap_defaultgid;
        ++i;
        pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": using default GID %lu", (unsigned long)pw->pw_gid);
        continue;
      }

      if (strcasecmp(ldap_attrs[i], ldap_attr_homedirectory) == 0) {
        if (!ldap_genhdir || !ldap_genhdir_prefix || !*ldap_genhdir_prefix) {
          pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": pr_ldap_user_lookup(): no %s attr for DN %s and LDAPGenerateHomedirPrefix was not enabled!", (dn = ldap_get_dn(ld, e)), ldap_attr_homedirectory);
          free(dn);
          return NULL;
        }

        if (ldap_genhdir_prefix_nouname) {
          pw->pw_dir = pstrcat(session.pool, ldap_genhdir_prefix, NULL);
        } else {
          LDAP_VALUE_T **canon_username;
          canon_username = LDAP_GET_VALUES(ld, e, ldap_attr_uid);
          if (!canon_username) {
            pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": pr_ldap_user_lookup(): couldn't get %s attr for canonical username for %s", ldap_attr_uid, (dn = ldap_get_dn(ld, e)));
            free(dn);
            return NULL;
          }

          pw->pw_dir = pstrcat(session.pool, ldap_genhdir_prefix, "/", LDAP_VALUE(canon_username, 0), NULL);
          LDAP_VALUE_FREE(canon_username);
        }

        ++i;
        pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": using default homedir %s", pw->pw_dir);
        continue;
      }

      /* Don't worry if we don't have a loginShell attr. */
      if (strcasecmp(ldap_attrs[i], ldap_attr_loginshell) == 0) {
        /* Prevent a segfault if no loginShell attr && RequireValidShell on. */
        pw->pw_shell = pstrdup(session.pool, "");
        ++i;
        continue;
      }

      /* We only restart the while loop above if we can fill in alternate
       * values for certain attributes. If something odd has happened, we
       * fall through to here and will complain about not being able to find
       * the attr.
       */

      pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": pr_ldap_user_lookup(): couldn't get values for attr %s for DN %s, ignoring request (perhaps this DN's entry does not have the attr?)", ldap_attrs[i], (dn = ldap_get_dn(ld, e)));
      free(dn);
      ldap_msgfree(result);
      return NULL;
    }

    /* Once we get here, we've already handled the "attribute defaults"
     * situation, so we can just fill in the struct as normal; the if
     * branches below for nonexistant attrs will just never be called.
     */

    if (strcasecmp(ldap_attrs[i], ldap_attr_uid) == 0) {
      pw->pw_name = pstrdup(session.pool, LDAP_VALUE(values, 0));
    } else if (strcasecmp(ldap_attrs[i], ldap_attr_userpassword) == 0) {
      pw->pw_passwd = pstrdup(session.pool, LDAP_VALUE(values, 0));
    } else if (strcasecmp(ldap_attrs[i], ldap_attr_uidnumber) == 0) {
      if (ldap_forcedefaultuid && ldap_defaultuid != -1) {
        pw->pw_uid = ldap_defaultuid;
      } else {
        pw->pw_uid = (uid_t) strtoul(LDAP_VALUE(values, 0), (char **)NULL, 10);
      }
    } else if (strcasecmp(ldap_attrs[i], ldap_attr_gidnumber) == 0) {
      if (ldap_forcedefaultgid && ldap_defaultgid != -1) {
        pw->pw_gid = ldap_defaultgid;
      } else {
        pw->pw_gid = (gid_t) strtoul(LDAP_VALUE(values, 0), (char **)NULL, 10);
      }
    } else if (strcasecmp(ldap_attrs[i], ldap_attr_homedirectory) == 0) {
      if (ldap_forcegenhdir) {
        if (!ldap_genhdir || !ldap_genhdir_prefix || !*ldap_genhdir_prefix) {
          pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": pr_ldap_user_lookup(): LDAPForceGeneratedHomedir is enabled, but LDAPGenerateHomedir is not.");
          return NULL;
        }

        if (ldap_genhdir_prefix_nouname) {
          pw->pw_dir = pstrcat(session.pool, ldap_genhdir_prefix, NULL);
        } else {
          LDAP_VALUE_T **canon_username;
          canon_username = LDAP_GET_VALUES(ld, e, ldap_attr_uid);
          if (!canon_username) {
            pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": pr_ldap_user_lookup(): couldn't get %s attr for canonical username for %s", ldap_attr_uid, (dn = ldap_get_dn(ld, e)));
            free(dn);
            return NULL;
          }

          pw->pw_dir = pstrcat(session.pool, ldap_genhdir_prefix, "/", LDAP_VALUE(canon_username, 0), NULL);
          LDAP_VALUE_FREE(canon_username);
        }
      } else {
        pw->pw_dir = pstrdup(session.pool, LDAP_VALUE(values, 0));
      }
    } else if (strcasecmp(ldap_attrs[i], ldap_attr_loginshell) == 0) {
      pw->pw_shell = pstrdup(session.pool, LDAP_VALUE(values, 0));
    } else {
      pr_log_pri(PR_LOG_WARNING, MOD_LDAP_VERSION ": pr_ldap_user_lookup(): value loop found unknown attr %s", ldap_attrs[i]);
    }

    LDAP_VALUE_FREE(values);
    ++i;
  }

  /* If we're doing auth binds, save the DN of this entry so we can
   * bind to the LDAP server as it later.
   */
  if (user_dn) {
    *user_dn = ldap_get_dn(ld, e);
  }

  ldap_msgfree(result);
  pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": "
    "user %s, uid %lu, gid %lu, homedir %s, shell %s",
    pw->pw_name, (unsigned long)pw->pw_uid, (unsigned long)pw->pw_gid,
    pw->pw_dir, pw->pw_shell);
  return pw;
}

static struct group *
pr_ldap_group_lookup(pool *p,
                     char *filter_template, const char *replace,
                     char *ldap_attrs[])
{
  char *filter, *dn;
  int i = 0, value_count, value_offset;
  struct group *gr;
  LDAPMessage *result, *e;
  LDAP_VALUE_T **values;

  if (!ldap_gid_basedn) {
    pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": no LDAP base DN specified for GID lookups");
    return NULL;
  }

  filter = pr_ldap_interpolate_filter(p, filter_template, replace);
  if (!filter) {
    return NULL;
  }

  result = pr_ldap_search(ldap_gid_basedn, filter, ldap_attrs, 2);
  if (result == NULL) {
    return NULL;
  }

  e = ldap_first_entry(ld, result);
  if (!e) {
    ldap_msgfree(result);
    pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": no entries for filter %s", filter);
    return NULL; /* No LDAP entries for this user */
  }

  gr = pcalloc(session.pool, sizeof(struct group));
  while (ldap_attrs[i] != NULL) {
    pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": fetching value(s) for attr %s", ldap_attrs[i]);

    values = LDAP_GET_VALUES(ld, e, ldap_attrs[i]);
    if (!values) {
      if (strcasecmp(ldap_attrs[i], ldap_attr_memberuid) == 0) {
        gr->gr_mem = palloc(session.pool, 2 * sizeof(char *));
        gr->gr_mem[0] = pstrdup(session.pool, "");
        gr->gr_mem[1] = NULL;

        ++i;
        continue;
      }

      ldap_msgfree(result);
      pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": pr_ldap_group_lookup(): couldn't get values for attr %s for DN %s, ignoring request (perhaps that DN does not have that attr?)", ldap_attrs[i], (dn = ldap_get_dn(ld, e)));
      free(dn);
      return NULL;
    }

    if (strcasecmp(ldap_attrs[i], ldap_attr_cn) == 0) {
      gr->gr_name = pstrdup(session.pool, LDAP_VALUE(values, 0));
    } else if (strcasecmp(ldap_attrs[i], ldap_attr_gidnumber) == 0) {
      gr->gr_gid = strtoul(LDAP_VALUE(values, 0), (char **)NULL, 10);
    } else if (strcasecmp(ldap_attrs[i], ldap_attr_memberuid) == 0) {
      value_count = LDAP_COUNT_VALUES(values);
      gr->gr_mem = (char **) palloc(session.pool, value_count * sizeof(char *));

      for (value_offset = 0; value_offset < value_count; ++value_offset)
        gr->gr_mem[value_offset] =
          pstrdup(session.pool, LDAP_VALUE(values, value_offset));
    } else {
      pr_log_pri(PR_LOG_WARNING, MOD_LDAP_VERSION ": pr_ldap_group_lookup(): value loop found unknown attr %s", ldap_attrs[i]);
    }

    LDAP_VALUE_FREE(values);
    ++i;
  }

  ldap_msgfree(result);
  /* FIXME: member logging. */
  pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": "
    "group %s, gid %lu", gr->gr_name, (unsigned long)gr->gr_gid);
  return gr;
}

static void
parse_quota(pool *p, const char *replace, char *str)
{
  char **elts, *token;

  if (cached_quota == NULL) {
    cached_quota = make_array(p, 9, sizeof(char *));
  }
  elts = (char **)cached_quota->elts;
  elts[0] = pstrdup(session.pool, replace);
  cached_quota->nelts = 1;

  while ((token = strsep(&str, ","))) {
    *((char **)push_array(cached_quota)) = pstrdup(session.pool, token);
  }
  pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": parsed quota %s", str);
}

static unsigned char
pr_ldap_quota_lookup(pool *p, char *filter_template, const char *replace,
                     char *basedn)
{
  char *filter = NULL,
       *attrs[] = {ldap_attr_ftpquota, ldap_attr_ftpquota_profiledn, NULL};
  int orig_scope, ret;
  LDAPMessage *result, *e;
  LDAP_VALUE_T **values;

  if (!basedn) {
    pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": no LDAP base DN specified for auth/UID lookups, declining request.");
    return FALSE;
  }

  if (filter_template) {
    filter = pr_ldap_interpolate_filter(p, filter_template, replace);
    if (!filter) {
      return FALSE;
    }
  }

  result = pr_ldap_search(basedn, filter, attrs, 2);
  if (result == NULL) {
    return FALSE;
  }

  if (ldap_count_entries(ld, result) > 1) {
    pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": pr_ldap_quota_lookup(): LDAP search returned multiple entries, aborting query");
    ldap_msgfree(result);
    if (ldap_default_quota != NULL) {
      parse_quota(p, replace, pstrdup(p, ldap_default_quota));
      return TRUE;
    }
    return FALSE;
  }

  e = ldap_first_entry(ld, result);
  if (!e) {
    ldap_msgfree(result);
    if (ldap_default_quota == NULL) {
      if (!filter) {
        pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": no entries for DN %s, and no default quota defined", basedn);
      } else {
        pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": no entries for filter %s, and no default quota defined", filter);
      }
      return FALSE;
    }

    if (!filter) {
      pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": no entries for DN %s, using default quota %s", basedn, ldap_default_quota);
    } else {
      pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": no entries for filter %s, using default quota %s", filter ? filter : "(null)", ldap_default_quota);
    }
    parse_quota(p, replace, pstrdup(p, ldap_default_quota));
    return TRUE;
  }

  values = LDAP_GET_VALUES(ld, e, attrs[0]);
  if (values) {
    parse_quota(p, replace, pstrdup(p, LDAP_VALUE(values, 0)));
    LDAP_VALUE_FREE(values);
    ldap_msgfree(result);
    return TRUE;
  }

  if (!filter) {
    if (ldap_default_quota == NULL) {
      pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": referenced DN %s does not have an ftpQuota attribute, and no default quota defined", basedn);
      return FALSE;
    }

    pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": no ftpQuota attr for DN %s, using default quota %s", basedn, ldap_default_quota);
    parse_quota(p, replace, pstrdup(p, ldap_default_quota));
    return TRUE;
  }

  values = LDAP_GET_VALUES(ld, e, attrs[1]);
  if (values) {
    orig_scope = ldap_search_scope;
    ldap_search_scope = LDAP_SCOPE_BASE;
    ret = pr_ldap_quota_lookup(p, NULL, replace, LDAP_VALUE(values, 0));
    ldap_search_scope = orig_scope;
    LDAP_VALUE_FREE(values);
    ldap_msgfree(result);
    return ret;
  }

  ldap_msgfree(result);
  if (ldap_default_quota != NULL) {
    pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": no %s or %s attribute, using default quota %s", attrs[0], attrs[1], ldap_default_quota);
    parse_quota(p, replace, pstrdup(p, ldap_default_quota));
    return TRUE;
  }

  pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": no %s or %s attribute, and no default quota defined", attrs[0], attrs[1]);
  return FALSE; /* No quota attrs for this user. */
}

static unsigned char
pr_ldap_ssh_pubkey_lookup(pool *p, char *filter_template, const char *replace,
                          char *basedn)
{
  char *filter, *attrs[] = {ldap_attr_ssh_pubkey, NULL};
  LDAPMessage *result, *e;
  LDAP_VALUE_T **values;

  if (!basedn) {
    pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": no LDAP base DN specified for auth/UID lookups, declining request.");
    return FALSE;
  }

  if (filter_template) {
    filter = pr_ldap_interpolate_filter(p, filter_template, replace);
    if (!filter) {
      return FALSE;
    }
  }

  result = pr_ldap_search(basedn, filter, attrs, 2);
  if (result == NULL) {
    return FALSE;
  }

  if (ldap_count_entries(ld, result) > 1) {
    pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": pr_ldap_ssh_pubkey_lookup(): LDAP search returned multiple entries, aborting query.");
    ldap_msgfree(result);
    return FALSE;
  }

  e = ldap_first_entry(ld, result);
  if (!e) {
    pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": pr_ldap_ssh_pubkey_lookup(): LDAP search returned no entries for filter %s", filter);
    ldap_msgfree(result);
    return FALSE;
  }

  values = LDAP_GET_VALUES(ld, e, attrs[0]);
  if (!values) {
    return FALSE;
  }

  if (cached_ssh_pubkeys == NULL) {
    cached_ssh_pubkeys = make_array(p, 1, sizeof(char *));
  }

  *((char **) push_array(cached_ssh_pubkeys)) = pstrdup(p,
    LDAP_VALUE(values, 0));
  LDAP_VALUE_FREE(values);

  ldap_msgfree(result);
  return TRUE;
}

static struct group *
pr_ldap_getgrnam(pool *p, const char *group_name)
{
  char *group_attrs[] = {ldap_attr_cn, ldap_attr_gidnumber,
                         ldap_attr_memberuid, NULL};

  return pr_ldap_group_lookup(p, ldap_group_name_filter,
    group_name, group_attrs);
}

static struct group *
pr_ldap_getgrgid(pool *p, gid_t gid)
{
  char gidstr[PR_TUNABLE_BUFFER_SIZE] = {'\0'},
       *group_attrs[] = {ldap_attr_cn, ldap_attr_gidnumber,
                         ldap_attr_memberuid, NULL};

  snprintf(gidstr, sizeof(gidstr), "%u", (unsigned)gid);

  return pr_ldap_group_lookup(p, ldap_group_gid_filter,
    (const char *)gidstr, group_attrs);
}

static struct passwd *
pr_ldap_getpwnam(pool *p, const char *username)
{
  char *filter,
       *name_attrs[] = {ldap_attr_userpassword, ldap_attr_uid,
                        ldap_attr_uidnumber, ldap_attr_gidnumber,
                        ldap_attr_homedirectory, ldap_attr_loginshell, NULL};

  filter = pr_ldap_interpolate_filter(p, ldap_auth_basedn, username);
  if (!filter) {
    return NULL;
  }

  /* pr_ldap_user_lookup() returns NULL if it doesn't find an entry or
   * encounters an error. If everything goes all right, it returns a
   * struct passwd, so we can just return its result directly.
   *
   * We also do some cute stuff here to work around lameness in LDAP servers
   * like Sun Directory Services (SDS) 1.x and 3.x. If you request an attr
   * that you don't have access to, SDS totally ignores any entries with
   * that attribute. Thank you, Sun; how very smart of you. So if we're
   * doing auth binds, we don't request the userPassword attr.
   *
   * NOTE: if the UserPassword directive is configured, mod_auth will pass
   * a crypted password to handle_ldap_check(), which will NOT do auth binds
   * in order to support UserPassword. (Otherwise, it would try binding to
   * the directory and would ignore UserPassword.)
   *
   * We're reasonably safe in making that assumption as long as we never
   * fetch userPassword from the directory if auth binds are enabled. If we
   * fetched userPassword, auth binds would never be done because
   * handle_ldap_check() would always get a crypted password.
   */
  return pr_ldap_user_lookup(p, ldap_auth_filter, username, filter,
    ldap_authbinds ? name_attrs + 1 : name_attrs,
    ldap_authbinds ? &ldap_authbind_dn : NULL);
}

static struct passwd *
pr_ldap_getpwuid(pool *p, uid_t uid)
{
  char uidstr[PR_TUNABLE_BUFFER_SIZE] = {'\0'},
       *uid_attrs[] = {ldap_attr_uid, ldap_attr_uidnumber, ldap_attr_gidnumber,
                       ldap_attr_homedirectory, ldap_attr_loginshell, NULL};

  snprintf(uidstr, sizeof(uidstr), "%u", (unsigned)uid);

  /* pr_ldap_user_lookup() returns NULL if it doesn't find an entry or
   * encounters an error. If everything goes all right, it returns a
   * struct passwd, so we can just return its result directly.
   */
  return pr_ldap_user_lookup(p, ldap_uid_filter, (const char *)uidstr,
    ldap_uid_basedn, uid_attrs, ldap_authbinds ? &ldap_authbind_dn : NULL);
}

MODRET
handle_ldap_quota_lookup(cmd_rec *cmd)
{
  if (cached_quota == NULL ||
      strcasecmp(((char **)cached_quota->elts)[0], cmd->argv[0]) != 0)
  {
    if (pr_ldap_quota_lookup(cmd->tmp_pool, ldap_quota_filter,
                             cmd->argv[0], ldap_quota_basedn) == FALSE)
    {
      return PR_DECLINED(cmd);
    }
  } else {
    pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": returning cached quota for %s", cmd->argv[0]);
  }

  return mod_create_data(cmd, cached_quota);
}

MODRET
handle_ldap_ssh_pubkey_lookup(cmd_rec *cmd)
{
  if (cached_ssh_pubkeys == NULL ||
      strcasecmp(((char **)cached_ssh_pubkeys->elts)[0], cmd->argv[0]) != 0)
  {
    if (pr_ldap_ssh_pubkey_lookup(cmd->tmp_pool, ldap_ssh_pubkey_filter,
                                  cmd->argv[0], ldap_ssh_pubkey_basedn) == FALSE)
    {
      return PR_DECLINED(cmd);
    }
  } else {
    pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": returning cached SSH public keys for %s", cmd->argv[0]);
  }

  return mod_create_data(cmd, cached_ssh_pubkeys);
}

MODRET
handle_ldap_setpwent(cmd_rec *cmd)
{
  if (ldap_doauth || ldap_douid || ldap_dogid) {
    if (!ld) {
      (void) pr_ldap_connect(&ld, TRUE);
    }
    return PR_HANDLED(cmd);
  }

  return PR_DECLINED(cmd);
}

MODRET
handle_ldap_endpwent(cmd_rec *cmd)
{
  if (ldap_doauth || ldap_douid || ldap_dogid) {
    pr_ldap_unbind();
    return PR_HANDLED(cmd);
  }

  return PR_DECLINED(cmd);
}

MODRET
handle_ldap_getpwuid(cmd_rec *cmd)
{
  struct passwd *pw;

  if (!ldap_douid) {
    return PR_DECLINED(cmd);
  }

  pw = pr_ldap_getpwuid(cmd->tmp_pool, *((uid_t *) cmd->argv[0]));
  if (pw) {
    return mod_create_data(cmd, pw);
  }

  return PR_DECLINED(cmd);
}

MODRET
handle_ldap_getpwnam(cmd_rec *cmd)
{
  struct passwd *pw;

  if (!ldap_doauth) {
    return PR_DECLINED(cmd);
  }

  pw = pr_ldap_getpwnam(cmd->tmp_pool, cmd->argv[0]);
  if (pw) {
    return mod_create_data(cmd, pw);
  }

  return PR_DECLINED(cmd);
}

MODRET
handle_ldap_getgrnam(cmd_rec *cmd)
{
  struct group *gr;

  if (!ldap_dogid) {
    return PR_DECLINED(cmd);
  }

  gr = pr_ldap_getgrnam(cmd->tmp_pool, cmd->argv[0]);
  if (gr) {
    return mod_create_data(cmd, gr);
  }

  return PR_DECLINED(cmd);
}

MODRET
handle_ldap_getgrgid(cmd_rec *cmd)
{
  struct group *gr;

  if (!ldap_dogid) {
    return PR_DECLINED(cmd);
  }

  gr = pr_ldap_getgrgid(cmd->tmp_pool, *((gid_t *) cmd->argv[0]));
  if (!gr) {
    return PR_DECLINED(cmd);
  }

  return mod_create_data(cmd, gr);
}

MODRET
handle_ldap_getgroups(cmd_rec *cmd)
{
  char *filter, *w[] = {ldap_attr_gidnumber, ldap_attr_cn, NULL};
  struct passwd *pw;
  struct group *gr;
  LDAPMessage *result = NULL, *e;
  LDAP_VALUE_T **gidNumber, **cn;
  array_header *gids   = (array_header *)cmd->argv[1],
               *groups = (array_header *)cmd->argv[2];

  if (!ldap_dogid) {
    return PR_DECLINED(cmd);
  }

  if (!gids || !groups) {
    return PR_DECLINED(cmd);
  }

  pw = pr_ldap_getpwnam(cmd->tmp_pool, cmd->argv[0]);
  if (pw) {
    gr = pr_ldap_getgrgid(cmd->tmp_pool, pw->pw_gid);
    if (gr) {
      pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": adding user %s primary group %s/%lu", pw->pw_name, gr->gr_name, (unsigned long)pw->pw_gid);
      *((gid_t *) push_array(gids))   = pw->pw_gid;
      *((char **) push_array(groups)) = pstrdup(session.pool, gr->gr_name);
    } else {
      pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": couldn't determine group name for user %s primary group %lu, skipping.", pw->pw_name, (unsigned long)pw->pw_gid);
    }
  }

  if (!ldap_gid_basedn) {
    pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": no LDAP base DN specified for GID lookups");
    goto return_groups;
  }

  filter = pr_ldap_interpolate_filter(cmd->tmp_pool,
    ldap_group_member_filter, cmd->argv[0]);
  if (!filter) {
    return NULL;
  }

  result = pr_ldap_search(ldap_gid_basedn, filter, w, 0);
  if (result == NULL) {
    return FALSE;
  }

  if (ldap_count_entries(ld, result) == 0) {
    pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": no entries for filter %s", filter);
    goto return_groups;
  }

  for (e = ldap_first_entry(ld, result); e; e = ldap_next_entry(ld, e)) {
    gidNumber = LDAP_GET_VALUES(ld, e, w[0]);
    if (!gidNumber) {
      pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": ldap_handle_getgroups(): couldn't get values for %s attr, skipping current group.", ldap_attr_gidnumber);
      continue;
    }
    cn = LDAP_GET_VALUES(ld, e, w[1]);
    if (!cn) {
      pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": ldap_handle_getgroups(): couldn't get values for %s attr, skipping current group.", ldap_attr_cn);
      continue;
    }

    if (!pw || strtoul(LDAP_VALUE(gidNumber, 0), (char **)NULL, 10) != pw->pw_gid) {
      *((gid_t *) push_array(gids)) =
        strtoul(LDAP_VALUE(gidNumber, 0), (char **)NULL, 10);
      *((char **) push_array(groups)) = pstrdup(session.pool, LDAP_VALUE(cn, 0));
      pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": added user %s secondary group %s/%s", pw->pw_name ? pw->pw_name : cmd->argv[0], LDAP_VALUE(cn, 0), LDAP_VALUE(gidNumber, 0));
    }

    LDAP_VALUE_FREE(gidNumber);
    LDAP_VALUE_FREE(cn);
  }

return_groups:
  if (result) {
    ldap_msgfree(result);
  }

  if (gids->nelts > 0) {
    return mod_create_data(cmd, (void *) &gids->nelts);
  }
  return PR_DECLINED(cmd);
}


/****************************
 * High-level auth handlers *
 ****************************/

/* cmd->argv[0] : user name
 * cmd->argv[1] : cleartext password
 */

MODRET
handle_ldap_is_auth(cmd_rec *cmd)
{
  const char *username = cmd->argv[0];
  char *filter,
       *pass_attrs[] = {ldap_attr_userpassword, ldap_attr_uid,
                        ldap_attr_uidnumber, ldap_attr_gidnumber,
                        ldap_attr_homedirectory, ldap_attr_loginshell, NULL};
  struct passwd *pw;

  if (!ldap_doauth) {
    return PR_DECLINED(cmd);
  }

  filter = pr_ldap_interpolate_filter(cmd->tmp_pool,
    ldap_auth_basedn, username);
  if (!filter) {
    return NULL;
  }

  /* If anything here fails hard (IOW, we've found an LDAP entry for the
   * user, but they appear to have entered the wrong password), boot them.
   * Normally, I'd DECLINE here so other modules could have a shot, but if
   * we've found their LDAP entry, chances are that nothing else is going to
   * be able to auth them. If anyone has a reason that this shouldn't be
   * this way, then by all means, let me know.
   */

  pw = pr_ldap_user_lookup(cmd->tmp_pool,
    ldap_auth_filter, username, filter,
    ldap_authbinds ? pass_attrs + 1 : pass_attrs,
    ldap_authbinds ? &ldap_authbind_dn : NULL);
  if (!pw) {
    return PR_DECLINED(cmd); /* Can't find the user in the LDAP directory. */
  }

  if (!ldap_authbinds && !pw->pw_passwd) {
    pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": LDAPAuthBinds is not enabled, and couldn't fetch a password for %s", pw->pw_name);
    return PR_ERROR_INT(cmd, PR_AUTH_NOPWD);
  }

  if (pr_auth_check(cmd->tmp_pool, ldap_authbinds ? NULL : pw->pw_passwd,
                    username, cmd->argv[1]))
  {
    pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": bad password for %s", pw->pw_name);
    return PR_ERROR_INT(cmd, PR_AUTH_BADPWD);
  }

  session.auth_mech = "mod_ldap.c";
  return PR_HANDLED(cmd);
}

/* cmd->argv[0] = hashed password,
 * cmd->argv[1] = user,
 * cmd->argv[2] = cleartext
 */

MODRET
handle_ldap_check(cmd_rec *cmd)
{
  char *pass, *cryptpass, *hash_method;
  int encname_len, ret;
  LDAP *ld_auth;
#ifdef HAS_LDAP_SASL_BIND_S
  struct berval bindcred;
#endif

#if defined(HAVE_OPENSSL) || defined(PR_USE_OPENSSL)
  EVP_MD_CTX EVP_Context;
  const EVP_MD *md;
  unsigned int md_len;
  unsigned char md_value[EVP_MAX_MD_SIZE];
  EVP_ENCODE_CTX EVP_Encode;

  /* According to RATS, the output buffer (buff) for EVP_EncodeBlock() needs
   * to be 4/3 the size of the input buffer (md_val).  Let's make it easy, and
   * use an output buffer that's twice the size of the input buffer.
   */
  unsigned char buff[EVP_MAX_MD_SIZE * 2];

#endif /* !HAVE_OPENSSL and !PR_USE_OPENSSL */

  if (!ldap_doauth) {
    return PR_DECLINED(cmd);
  }

  cryptpass = cmd->argv[0];
  pass = cmd->argv[2];


  /* At this point, any encrypted password must have come from
   * the UserPassword directive. Don't perform auth binds in this
   * case, since the crypted password specified should override
   * auth binds.
   */
  if (ldap_authbinds && cryptpass == NULL) {
    /* Don't try to do auth binds with a NULL/empty DN or password. */
    if ((pass == NULL) || (strlen(pass) == 0)) {
      pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": check: LDAPAuthBinds is enabled, but no user-supplied cleartext password was provided.");
      return PR_DECLINED(cmd);
    }
    if ((ldap_authbind_dn == NULL) || (strlen(ldap_authbind_dn) == 0)) {
      pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": check: LDAPAuthBinds is enabled, but no LDAP DN was found.");
      return PR_DECLINED(cmd);
    }

    if (pr_ldap_connect(&ld_auth, FALSE) == -1) {
      pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": handle_ldap_check(): pr_ldap_connect() failed.");
      return PR_DECLINED(cmd);
    }

#ifdef HAS_LDAP_SASL_BIND_S
    bindcred.bv_val = cmd->argv[2];
    bindcred.bv_len = strlen(cmd->argv[2]);
    ret = ldap_sasl_bind_s(ld_auth, ldap_authbind_dn, NULL, &bindcred,
      NULL, NULL, NULL);
#else /* HAS_LDAP_SASL_BIND_S */
    ret = ldap_simple_bind_s(ld_auth, ldap_authbind_dn, cmd->argv[2]);
#endif /* HAS_LDAP_SASL_BIND_S */

    if (ret != LDAP_SUCCESS) {
      if (ret != LDAP_INVALID_CREDENTIALS) {
        pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": handle_ldap_check(): bind as %s failed: %s", ldap_authbind_dn, ldap_err2string(ret));
      }
      pr_log_debug(DEBUG3, MOD_LDAP_VERSION ": invalid credentials for %s", ldap_authbind_dn);
      LDAP_UNBIND(ld_auth);
      return PR_ERROR(cmd);
    }

    LDAP_UNBIND(ld_auth);
    session.auth_mech = "mod_ldap.c";
    return PR_HANDLED(cmd);
  }

  /* Get the length of "scheme" in the leading {scheme} so we can skip it
   * in the password comparison.
   */
  encname_len = strcspn(cryptpass + 1, "}");
  hash_method = pstrndup(cmd->tmp_pool, cryptpass + 1, encname_len);

  /* Check to see how the password is encrypted, and check accordingly. */

  if (encname_len == strlen(cryptpass + 1)) { /* No leading {scheme} */
    if (ldap_defaultauthscheme && (strcasecmp(ldap_defaultauthscheme, "clear") == 0)) {
      if (strcmp(pass, cryptpass) != 0) {
        return PR_ERROR(cmd);
      }
    } else { /* else, assume crypt */
      if (strcmp(crypt(pass, cryptpass), cryptpass) != 0) {
        return PR_ERROR(cmd);
      }
    }
  } else if (strncasecmp(hash_method, "crypt", strlen(hash_method)) == 0) { /* {crypt} */
    if (strcmp(crypt(pass, cryptpass + encname_len + 2), cryptpass + encname_len + 2) != 0) {
      return PR_ERROR(cmd);
    }
  } else if (strncasecmp(hash_method, "clear", strlen(hash_method)) == 0) { /* {clear} */
    if (strcmp(pass, cryptpass + encname_len + 2) != 0) {
      return PR_ERROR(cmd);
    }
  }
#if defined(HAVE_OPENSSL) || defined(PR_USE_OPENSSL)
  else { /* Try the cipher mode found */
    pr_log_debug(DEBUG5, MOD_LDAP_VERSION ": %s-encrypted password found, trying to auth.", hash_method);

    SSLeay_add_all_digests();

    /* This is a kludge. This is only a kludge. OpenLDAP likes {sha}
     * (at least, the OpenLDAP ldappasswd generates {sha}), but OpenSSL
     * likes {sha1} and does not understand {sha}. We translate
     * RMD160 -> RIPEMD160 here, too.
     */
    if (strncasecmp(hash_method, "SHA", 4) == 0) {
        md = EVP_get_digestbyname("SHA1");
    } else if (strncasecmp(hash_method, "RMD160", 7) == 0) {
        md = EVP_get_digestbyname("RIPEMD160");
    } else {
        md = EVP_get_digestbyname(hash_method);
    }

    if (!md) {
      pr_log_debug(DEBUG5, MOD_LDAP_VERSION ": %s not supported by OpenSSL, declining auth request", hash_method);
      return PR_DECLINED(cmd); /* Some other module may support it. */
    }

    /* Make a digest of the user-supplied password. */
    EVP_DigestInit(&EVP_Context, md);
    EVP_DigestUpdate(&EVP_Context, pass, strlen(pass));
    EVP_DigestFinal(&EVP_Context, md_value, &md_len);

    /* Base64 Encoding */
    memset(buff, '\0', sizeof(buff));
    EVP_EncodeInit(&EVP_Encode);
    EVP_EncodeBlock(buff, md_value, (int) md_len);

    if (strcmp((char *) buff, cryptpass + encname_len + 2) != 0) {
      return PR_ERROR(cmd);
    }
  }
#else
  else { /* Can't find a supported {scheme} */
    return PR_DECLINED(cmd);
  }
#endif /* !HAVE_OPENSSL and !PR_USE_OPENSSL */

  session.auth_mech = "mod_ldap.c";
  return PR_HANDLED(cmd);
}

MODRET
handle_ldap_uid_name(cmd_rec *cmd)
{
  struct passwd *pw;

  if (!ldap_douid) {
    return PR_DECLINED(cmd);
  }

  pw = pr_ldap_getpwuid(cmd->tmp_pool, *((uid_t *) cmd->argv[0]));
  if (!pw) {
    /* Can't find the user in the LDAP directory. */
    return PR_DECLINED(cmd);
  }

  return mod_create_data(cmd, pstrdup(permanent_pool, pw->pw_name));
}

MODRET
handle_ldap_gid_name(cmd_rec *cmd)
{
  struct group *gr;

  if (!ldap_dogid) {
    return PR_DECLINED(cmd);
  }

  gr = pr_ldap_getgrgid(cmd->tmp_pool, *((gid_t *) cmd->argv[0]));
  if (!gr) {
    /* Can't find the user in the LDAP directory. */
    return PR_DECLINED(cmd);
  }

  return mod_create_data(cmd, pstrdup(permanent_pool, gr->gr_name));
}

MODRET
handle_ldap_name_uid(cmd_rec *cmd)
{
  struct passwd *pw;

  if (!ldap_doauth) {
    return PR_DECLINED(cmd);
  }

  pw = pr_ldap_getpwnam(cmd->tmp_pool, cmd->argv[0]);
  if (!pw) {
    return PR_DECLINED(cmd);
  }

  return mod_create_data(cmd, (void *) &pw->pw_uid);
}

MODRET
handle_ldap_name_gid(cmd_rec *cmd)
{
  struct group *gr;

  if (!ldap_dogid) {
    return PR_DECLINED(cmd);
  }

  gr = pr_ldap_getgrnam(cmd->tmp_pool, cmd->argv[0]);
  if (!gr) {
    return PR_DECLINED(cmd);
  }

  return mod_create_data(cmd, (void *) &gr->gr_gid);
}


/*****************************************
 * Config-file handlers/parsing routines *
 *****************************************/

MODRET
set_ldap_server(cmd_rec *cmd)
{
  int i, len;
  char *item;
  LDAPURLDesc *url;
  array_header *urls = NULL;
  config_rec *c;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  c = add_config_param(cmd->argv[0], 1, NULL);
  urls = make_array(c->pool, cmd->argc - 1, sizeof(char *));
  c->argv[0] = urls;

  for (i = 1; i < cmd->argc; ++i) {
    if (ldap_is_ldap_url(cmd->argv[i])) {
      if (ldap_url_parse(cmd->argv[i], &url) != LDAP_URL_SUCCESS) {
        CONF_ERROR(cmd, "LDAPServer: must be supplied with a valid LDAP URL.");
      }

      if (find_config(main_server->conf, CONF_PARAM, "LDAPSearchScope", FALSE)) {
        CONF_ERROR(cmd, "LDAPSearchScope cannot be used when LDAPServer specifies a URL; specify a search scope in the LDAPServer URL instead.");
      }

#ifdef HAS_LDAP_INITIALIZE
      if (strncasecmp(cmd->argv[i], "ldap:", strlen("ldap:")) != 0 &&
          strncasecmp(cmd->argv[i], "ldaps:", strlen("ldaps:")) != 0) {

        CONF_ERROR(cmd, "Invalid scheme specified by LDAPServer URL. Valid schemes are 'ldap' or 'ldaps'.");
      }
#else /* HAS_LDAP_INITIALIZE */
      if (strncasecmp(cmd->argv[i], "ldap:", strlen("ldap:")) != 0) {
        CONF_ERROR(cmd, "Invalid scheme specified by LDAPServer URL. Valid schemes are 'ldap'.");
      }
#endif /* HAS_LDAP_INITIALIZE */

      if (url->lud_dn && strcmp(url->lud_dn, "") != 0) {
        CONF_ERROR(cmd, "A base DN may not be specified by an LDAPServer URL, only by LDAPDoAuth, LDAPDoUIDLookups, LDAPDoGIDLookups, or LDAPDoQuotaLookups.");
      }
      if (url->lud_filter && strcmp(url->lud_filter, "") != 0) {
        CONF_ERROR(cmd, "A search filter may not be specified by an LDAPServer URL, only by LDAPDoAuth, LDAPDoUIDLookups, LDAPDoGIDLookups, or LDAPDoQuotaLookups.");
      }

      ldap_free_urldesc(url);
      *((char **)push_array(urls)) = pstrdup(c->pool, cmd->argv[i]);
    } else {
      /* Split non-URL arguments on whitespace and insert them as
       * separate servers.
       */
      item = cmd->argv[i];
      while (*item) {
        len = strcspn(item, " \f\n\r\t\v");
        *((char **)push_array(urls)) = pstrndup(c->pool, item, len);

        item += len;
        while (isspace(*item)) {
          ++item;
        }
      }
    }
  }

  return PR_HANDLED(cmd);
}

MODRET
set_ldap_dninfo(cmd_rec *cmd)
{
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  add_config_param_str(cmd->argv[0], 2, cmd->argv[1], cmd->argv[2]);
  return PR_HANDLED(cmd);
}

MODRET
set_ldap_authbinds(cmd_rec *cmd)
{
  int b;
  config_rec *c;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if ((b = get_boolean(cmd, 1)) == -1) {
    CONF_ERROR(cmd, "LDAPAuthBinds: expected a boolean value for first argument.");
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = b;

  return PR_HANDLED(cmd);
}

MODRET
set_ldap_querytimeout(cmd_rec *cmd)
{
  config_rec *c;
  int timeout;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  timeout = atoi(cmd->argv[1]);
  if (timeout <= 0) {
    CONF_ERROR(cmd, "LDAPQueryTimeout: timeout must be greater than zero.");
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = timeout;

  return PR_HANDLED(cmd);
}

MODRET
set_ldap_searchscope(cmd_rec *cmd)
{
  config_rec *c;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  c = find_config(main_server->conf, CONF_PARAM, "LDAPServer", FALSE);
  if (c && ldap_is_ldap_url(c->argv[0])) {
    CONF_ERROR(cmd, "LDAPSearchScope cannot be used when LDAPServer specifies a URL; specify a search scope in the LDAPServer URL instead.");
  }

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

MODRET
set_ldap_dereference(cmd_rec *cmd)
{
  int value;
  config_rec *c;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if (strcasecmp(cmd->argv[1], "never") == 0) {
    value = LDAP_DEREF_NEVER;
  } else if (strcasecmp(cmd->argv[1], "search") == 0) {
    value = LDAP_DEREF_SEARCHING;
  } else if (strcasecmp(cmd->argv[1], "find") == 0) {
    value = LDAP_DEREF_FINDING;
  } else if (strcasecmp(cmd->argv[1], "always") == 0) {
    value = LDAP_DEREF_ALWAYS;
  } else {
    CONF_ERROR(cmd, "LDAPAliasDereference: expected a valid dereference (never, search, find, always).");
  }

  c = add_config_param("LDAPAliasDereference", 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = value;
  return PR_HANDLED(cmd);
}

MODRET
set_ldap_doauth(cmd_rec *cmd)
{
  int b;
  config_rec *c;

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if ((b = get_boolean(cmd, 1)) == -1) {
    CONF_ERROR(cmd, "LDAPDoAuth: expected a boolean value for first argument.");
  }

  if (b == 1) { CHECK_ARGS(cmd, 2); }
  else        { CHECK_ARGS(cmd, 1); }

  c = add_config_param(cmd->argv[0], cmd->argc - 1, NULL, NULL, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = b;
  if (cmd->argc > 2) {
    c->argv[1] = pstrdup(c->pool, cmd->argv[2]);
  }
  if (cmd->argc > 3) {
    c->argv[2] = pstrdup(c->pool, cmd->argv[3]);
  }

  return PR_HANDLED(cmd);
}

MODRET
set_ldap_douid(cmd_rec *cmd)
{
  int b;
  config_rec *c;

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if ((b = get_boolean(cmd, 1)) == -1) {
    CONF_ERROR(cmd, "LDAPDoUIDLookups: expected a boolean value for first argument.");
  }

  if (b == 1) { CHECK_ARGS(cmd, 2); }
  else        { CHECK_ARGS(cmd, 1); }

  c = add_config_param(cmd->argv[0], cmd->argc - 1, NULL, NULL, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = b;
  if (cmd->argc > 2) {
    c->argv[1] = pstrdup(c->pool, cmd->argv[2]);
  }
  if (cmd->argc > 3) {
    c->argv[2] = pstrdup(c->pool, cmd->argv[3]);
  }

  return PR_HANDLED(cmd);
}

MODRET
set_ldap_dogid(cmd_rec *cmd)
{
  int b;
  config_rec *c;

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if ((b = get_boolean(cmd, 1)) == -1) {
    CONF_ERROR(cmd, "LDAPDoGIDLookups: expected a boolean value for first argument.");
  }

  if (b == 1) { CHECK_ARGS(cmd, 2); }
  else        { CHECK_ARGS(cmd, 1); }

  c = add_config_param(cmd->argv[0], cmd->argc - 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = b;
  if (cmd->argc > 2) {
    c->argv[1] = pstrdup(c->pool, cmd->argv[2]);
  }
  if (cmd->argc > 3) {
    c->argv[2] = pstrdup(c->pool, cmd->argv[3]);
  }
  if (cmd->argc > 4) {
    c->argv[3] = pstrdup(c->pool, cmd->argv[4]);
  }
  if (cmd->argc > 5) {
    c->argv[4] = pstrdup(c->pool, cmd->argv[5]);
  }

  return PR_HANDLED(cmd);
}

MODRET
set_ldap_doquota(cmd_rec *cmd)
{
  int b;
  config_rec *c;

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if ((b = get_boolean(cmd, 1)) == -1) {
    CONF_ERROR(cmd, "LDAPDoQuotaLookups: expected a boolean value for first argument.");
  }

  if (b == 1) { CHECK_ARGS(cmd, 2); }
  else        { CHECK_ARGS(cmd, 1); }

  c = add_config_param(cmd->argv[0], cmd->argc - 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = b;
  if (cmd->argc > 2) {
    c->argv[1] = pstrdup(c->pool, cmd->argv[2]);
  }
  if (cmd->argc > 3) {
    c->argv[2] = pstrdup(c->pool, cmd->argv[3]);
  }
  if (cmd->argc > 4) {
    c->argv[3] = pstrdup(c->pool, cmd->argv[4]);
  }

  return PR_HANDLED(cmd);
}

MODRET
set_ldap_defaultuid(cmd_rec *cmd)
{
  char *endptr;
  config_rec *c;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(uid_t));
  *((uid_t *) c->argv[0]) = strtoul(cmd->argv[1], &endptr, 10);
  if (*endptr != '\0') {
    CONF_ERROR(cmd, "LDAPDefaultUID: UID argument must be numeric!");
  }
  return PR_HANDLED(cmd);
}

MODRET
set_ldap_defaultgid(cmd_rec *cmd)
{
  char *endptr;
  config_rec *c;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(gid_t));
  *((gid_t *) c->argv[0]) = strtoul(cmd->argv[1], &endptr, 10);
  if (*endptr != '\0') {
    CONF_ERROR(cmd, "LDAPDefaultGID: GID argument must be numeric.");
  }
  return PR_HANDLED(cmd);
}

MODRET set_ldap_forcedefaultuid(cmd_rec *cmd)
{
  int b;
  config_rec *c;

  CHECK_CONF(cmd,CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if ((b = get_boolean(cmd, 1)) == -1) {
    CONF_ERROR(cmd, "LDAPForceDefaultUID: expected boolean argument for first argument.");
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = b;
  return PR_HANDLED(cmd);
}

MODRET set_ldap_forcedefaultgid(cmd_rec *cmd)
{
  int b;
  config_rec *c;

  CHECK_CONF(cmd,CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if ((b = get_boolean(cmd, 1)) == -1) {
    CONF_ERROR(cmd, "LDAPForceDefaultGID: expected boolean argument for first argument.");
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = b;

  return PR_HANDLED(cmd);
}

MODRET
set_ldap_negcache(cmd_rec *cmd)
{
  pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": LDAPNegativeCache has no effect and is deprecated; please remove it from your configuration");
  return PR_HANDLED(cmd);
}

MODRET
set_ldap_genhdir(cmd_rec *cmd)
{
  int b;
  config_rec *c;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if ((b = get_boolean(cmd, 1)) == -1) {
    CONF_ERROR(cmd, "LDAPGenerateHomedir: expected a boolean value for first argument.");
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = b;
  return PR_HANDLED(cmd);
}

MODRET set_ldap_forcegenhdir(cmd_rec *cmd)
{
  int b;
  config_rec *c;

  CHECK_CONF(cmd,CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if ((b = get_boolean(cmd, 1)) == -1) {
    CONF_ERROR(cmd, "LDAPForceGeneratedHomedir: expected boolean argument for first argument.");
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = b;
  return PR_HANDLED(cmd);
}

MODRET
set_ldap_genhdirprefix(cmd_rec *cmd)
{
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

MODRET
set_ldap_genhdirprefixnouname(cmd_rec *cmd)
{
  int b;
  config_rec *c;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if ((b = get_boolean(cmd, 1)) == -1) {
    CONF_ERROR(cmd, "LDAPGenerateHomedirPrefixNoUsername: expected a boolean value for first argument.");
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = b;

  return PR_HANDLED(cmd);
}

MODRET
set_ldap_defaultauthscheme(cmd_rec *cmd)
{
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

MODRET
set_ldap_usetls(cmd_rec *cmd)
{
#ifndef LDAP_OPT_X_TLS
  CONF_ERROR(cmd, "LDAPUseTLS: Your LDAP libraries do not appear to support TLS.");
#else /* LDAP_OPT_X_TLS */
  int b;
  config_rec *c;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if ((b = get_boolean(cmd, 1)) == -1) {
    CONF_ERROR(cmd, "LDAPUseTLS: expected a boolean value for first argument.");
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = b;
  return PR_HANDLED(cmd);
#endif /* LDAP_OPT_X_TLS */
}

MODRET
set_ldap_usessl(cmd_rec *cmd)
{
  pr_log_pri(PR_LOG_ERR, MOD_LDAP_VERSION ": LDAPUseSSL did not have the intended effect and has been removed. Please remove this directive from your configuration, as it will be removed in future versions of mod_ldap and ProFTPD will fail to start.");
  return PR_HANDLED(cmd);
}

MODRET
set_ldap_protoversion(cmd_rec *cmd)
{
  int i = 0;
  config_rec *c;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  while (cmd->argv[1][i]) {
    if (! isdigit((int) cmd->argv[1][i])) {
      CONF_ERROR(cmd, "LDAPProtocolVersion: argument must be numeric!");
    }
    ++i;
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = atoi(cmd->argv[1]);
  return PR_HANDLED(cmd);
}

MODRET
set_ldap_attr(cmd_rec *cmd)
{
  CHECK_ARGS(cmd, 2);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if (strcasecmp(cmd->argv[1], "uid") != 0 &&
      strcasecmp(cmd->argv[1], "uidNumber") != 0 &&
      strcasecmp(cmd->argv[1], "gidNumber") != 0 &&
      strcasecmp(cmd->argv[1], "homeDirectory") != 0 &&
      strcasecmp(cmd->argv[1], "userPassword") != 0 &&
      strcasecmp(cmd->argv[1], "loginShell") != 0 &&
      strcasecmp(cmd->argv[1], "cn") != 0 &&
      strcasecmp(cmd->argv[1], "memberUid") != 0 &&
      strcasecmp(cmd->argv[1], "ftpQuota") != 0 &&
      strcasecmp(cmd->argv[1], "ftpQuotaProfileDN") != 0)
  {
    CONF_ERROR(cmd, "LDAPAttr: unknown attribute name.");
  }

  add_config_param_str(cmd->argv[0], 2, cmd->argv[1], cmd->argv[2]);
  return PR_HANDLED(cmd);
}

static int
ldap_getconf(void)
{
  char *scope;
  config_rec *c;
  void *ptr;

  /* Look up any attr redefinitions (LDAPAttr) before using those variables,
   * such as when generating the default search filters.
   */
  if ((c = find_config(main_server->conf, CONF_PARAM, "LDAPAttr", FALSE)) != NULL) {
    do {
      if (strcasecmp(c->argv[0], "uid") == 0) {
        ldap_attr_uid = pstrdup(session.pool, c->argv[1]);
      } else if (strcasecmp(c->argv[0], "uidNumber") == 0) {
        ldap_attr_uidnumber = pstrdup(session.pool, c->argv[1]);
      } else if (strcasecmp(c->argv[0], "gidNumber") == 0) {
        ldap_attr_gidnumber = pstrdup(session.pool, c->argv[1]);
      } else if (strcasecmp(c->argv[0], "homeDirectory") == 0) {
        ldap_attr_homedirectory = pstrdup(session.pool, c->argv[1]);
      } else if (strcasecmp(c->argv[0], "userPassword") == 0) {
        ldap_attr_userpassword = pstrdup(session.pool, c->argv[1]);
      } else if (strcasecmp(c->argv[0], "loginShell") == 0) {
        ldap_attr_loginshell = pstrdup(session.pool, c->argv[1]);
      } else if (strcasecmp(c->argv[0], "cn") == 0) {
        ldap_attr_cn = pstrdup(session.pool, c->argv[1]);
      } else if (strcasecmp(c->argv[0], "memberUid") == 0) {
        ldap_attr_memberuid = pstrdup(session.pool, c->argv[1]);
      } else if (strcasecmp(c->argv[0], "ftpQuota") == 0) {
        ldap_attr_ftpquota = pstrdup(session.pool, c->argv[1]);
      } else if (strcasecmp(c->argv[0], "ftpQuotaProfileDN") == 0) {
        ldap_attr_ftpquota_profiledn = pstrdup(session.pool, c->argv[1]);
      }
    } while ((c = find_config_next(c, c->next, CONF_PARAM, "LDAPAttr", FALSE)));
  }

  if ((c = find_config(main_server->conf, CONF_PARAM, "LDAPDNInfo", FALSE)) != NULL) {
    ldap_dn = pstrdup(session.pool, c->argv[0]);
    ldap_dnpass = pstrdup(session.pool, c->argv[1]);
  }

  ptr = get_param_ptr(main_server->conf, "LDAPAuthBinds", FALSE);
  if (ptr) {
    ldap_authbinds = *((int *) ptr);
  }

  ptr = get_param_ptr(main_server->conf, "LDAPQueryTimeout", FALSE);
  if (ptr) {
    ldap_querytimeout = *((int *) ptr);
  }

  scope = get_param_ptr(main_server->conf, "LDAPSearchScope", FALSE);
  if (scope) {
    if (strcasecmp(scope, "onelevel") == 0) {
      ldap_search_scope = LDAP_SCOPE_ONELEVEL;
    } else if (strcasecmp(scope, "subtree") == 0) {
      ldap_search_scope = LDAP_SCOPE_SUBTREE;
    }
  }
  
  ptr = get_param_ptr(main_server->conf, "LDAPAliasDereference", FALSE);
  if (ptr) {
    ldap_dereference = *((int *) ptr);
  }

  if ((c = find_config(main_server->conf, CONF_PARAM, "LDAPDoAuth", FALSE)) != NULL) {
    if ( *((int *) c->argv[0]) > 0) {
      ldap_doauth = 1;
      ldap_auth_basedn = pstrdup(session.pool, c->argv[1]);

      if (c->argv[2]) {
        ldap_auth_filter = pstrdup(session.pool, c->argv[2]);
      } else {
        ldap_auth_filter = pstrcat(session.pool, "(&(", ldap_attr_uid, "=%v)(objectclass=posixAccount))", NULL);
      }
    }
  }

  if ((c = find_config(main_server->conf, CONF_PARAM, "LDAPDoUIDLookups", FALSE)) != NULL) {
    if ( *((int *) c->argv[0]) > 0) {
      ldap_douid = 1;
      ldap_uid_basedn = pstrdup(session.pool, c->argv[1]);

      if (c->argv[2]) {
        ldap_uid_filter = pstrdup(session.pool, c->argv[2]);
      } else {
        ldap_uid_filter = pstrcat(session.pool, "(&(", ldap_attr_uidnumber, "=%v)(objectclass=posixAccount))", NULL);
      }
    }
  }

  if ((c = find_config(main_server->conf, CONF_PARAM, "LDAPDoGIDLookups", FALSE)) != NULL) {
    if ( *((int *) c->argv[0]) > 0) {
      ldap_dogid = 1;
      ldap_gid_basedn = pstrdup(session.pool, c->argv[1]);

      if (c->argc > 2) {
        ldap_group_name_filter = pstrdup(session.pool, c->argv[2]);
      } else {
        ldap_group_name_filter = pstrcat(session.pool, "(&(", ldap_attr_cn, "=%v)(objectclass=posixGroup))", NULL);
      }

      if (c->argc > 3) {
        ldap_group_gid_filter = pstrdup(session.pool, c->argv[3]);
      } else {
        ldap_group_gid_filter = pstrcat(session.pool, "(&(", ldap_attr_gidnumber, "=%v)(objectclass=posixGroup))", NULL);
      }

      if (c->argc > 4) {
        ldap_group_member_filter = pstrdup(session.pool, c->argv[4]);
      } else {
        ldap_group_member_filter = pstrcat(session.pool, "(&(", ldap_attr_memberuid, "=%v)(objectclass=posixGroup))", NULL);
      }
    }
  }

  if ((c = find_config(main_server->conf, CONF_PARAM, "LDAPDoQuotaLookups", FALSE)) != NULL) {
    if ( *((int *) c->argv[0]) > 0) {
      ldap_doquota = 1;
      ldap_quota_basedn = pstrdup(session.pool, c->argv[1]);

      if (c->argc > 2) {
        ldap_quota_filter = pstrdup(session.pool, c->argv[2]);
      } else {
        ldap_quota_filter = pstrcat(session.pool, "(&(", ldap_attr_uid, "=%v)(objectclass=posixAccount))", NULL);
      }

      if (c->argc > 3) {
        ldap_default_quota = pstrdup(session.pool, c->argv[3]);
      }
    }
  }

  ptr = get_param_ptr(main_server->conf, "LDAPDefaultUID", FALSE);
  if (ptr) {
    ldap_defaultuid = *((uid_t *) ptr);
  }

  ptr = get_param_ptr(main_server->conf, "LDAPDefaultGID", FALSE);
  if (ptr) {
    ldap_defaultgid = *((gid_t *) ptr);
  }

  ptr = get_param_ptr(main_server->conf, "LDAPForceDefaultUID", FALSE);
  if (ptr) {
    ldap_forcedefaultuid = *((int *) ptr);
  }

  ptr = get_param_ptr(main_server->conf, "LDAPForceDefaultGID", FALSE);
  if (ptr) {
    ldap_forcedefaultgid = *((int *) ptr);
  }

  ptr = get_param_ptr(main_server->conf, "LDAPForceGeneratedHomedir", FALSE);
  if (ptr) {
    ldap_forcegenhdir = *((int *) ptr);
  }

  ptr = get_param_ptr(main_server->conf, "LDAPGenerateHomedir", FALSE);
  if (ptr) {
    ldap_genhdir = *((int *) ptr);
  }

  ldap_genhdir_prefix = (char *)get_param_ptr(main_server->conf, "LDAPGenerateHomedirPrefix", FALSE);

  ptr = get_param_ptr(main_server->conf, "LDAPGenerateHomedirPrefixNoUsername", FALSE);
  if (ptr) {
    ldap_genhdir_prefix_nouname = *((int *) ptr);
  }

  /* If ldap_defaultauthscheme is NULL, ldap_check() will assume crypt. */
  ldap_defaultauthscheme = (char *)get_param_ptr(main_server->conf, "LDAPDefaultAuthScheme", FALSE);

  ptr = get_param_ptr(main_server->conf, "LDAPProtocolVersion", FALSE);
  if (ptr) {
    ldap_protocol_version = *((int *) ptr);
  }

#ifdef LDAP_OPT_X_TLS
  ptr = get_param_ptr(main_server->conf, "LDAPUseTLS", FALSE);
  if (ptr) {
    ldap_use_tls = *((int *) ptr);
  }
#endif /* LDAP_OPT_X_TLS */

  if ((c = find_config(main_server->conf, CONF_PARAM, "LDAPServer", FALSE)) != NULL) {
    ldap_servers = c->argv[0];
  } else {
    /* Leave a NULL server entry if LDAPServer isn't present, so
     * ldap_init()/ldap_initialize() will connect to the LDAP SDK's
     * default.
     */
    ldap_servers = make_array(session.pool, 1, sizeof(char *));
    *((char **)push_array(ldap_servers)) = NULL;
  }

  return 0;
}

static int ldap_mod_init(void) {
  pr_log_debug(DEBUG2, MOD_LDAP_VERSION
    ": compiled using LDAP vendor '%s', LDAP API version %lu",
    LDAP_VENDOR_NAME, (unsigned long) LDAP_API_VERSION);

  return 0;
}

static conftable ldap_config[] = {
  { "LDAPServer",                          set_ldap_server,               NULL },
  { "LDAPDNInfo",                          set_ldap_dninfo,               NULL },
  { "LDAPAuthBinds",                       set_ldap_authbinds,            NULL },
  { "LDAPQueryTimeout",                    set_ldap_querytimeout,         NULL },
  { "LDAPSearchScope",                     set_ldap_searchscope,          NULL },
  { "LDAPAliasDereference",                set_ldap_dereference,          NULL },
  { "LDAPNegativeCache",                   set_ldap_negcache,             NULL },
  { "LDAPDoAuth",                          set_ldap_doauth,               NULL },
  { "LDAPDoUIDLookups",                    set_ldap_douid,                NULL },
  { "LDAPDoGIDLookups",                    set_ldap_dogid,                NULL },
  { "LDAPDoQuotaLookups",                  set_ldap_doquota,              NULL },
  { "LDAPDefaultUID",                      set_ldap_defaultuid,           NULL },
  { "LDAPDefaultGID",                      set_ldap_defaultgid,           NULL },
  { "LDAPForceDefaultUID",                 set_ldap_forcedefaultuid,      NULL },
  { "LDAPForceDefaultGID",                 set_ldap_forcedefaultgid,      NULL },
  { "LDAPGenerateHomedir",                 set_ldap_genhdir,              NULL },
  { "LDAPGenerateHomedirPrefix",           set_ldap_genhdirprefix,        NULL },
  { "LDAPGenerateHomedirPrefixNoUsername", set_ldap_genhdirprefixnouname, NULL },
  { "LDAPForceGeneratedHomedir",           set_ldap_forcegenhdir,         NULL },
  { "LDAPDefaultAuthScheme",               set_ldap_defaultauthscheme,    NULL },
  { "LDAPUseTLS",                          set_ldap_usetls,               NULL },
  { "LDAPUseSSL",                          set_ldap_usessl,               NULL },
  { "LDAPProtocolVersion",                 set_ldap_protoversion,         NULL },
  { "LDAPAttr",                            set_ldap_attr,                 NULL },
  { NULL,                                  NULL,                          NULL }
};

static cmdtable ldap_cmdtab[] = {
  {HOOK, "ldap_quota_lookup", G_NONE, handle_ldap_quota_lookup, FALSE, FALSE},
  {HOOK, "ldap_ssh_publickey_lookup", G_NONE, handle_ldap_ssh_pubkey_lookup, FALSE, FALSE},
  {0, NULL}
};

static authtable ldap_auth[] = {
  { 0, "setpwent",  handle_ldap_setpwent  },
  { 0, "endpwent",  handle_ldap_endpwent  },
  { 0, "setgrent",  handle_ldap_setpwent  },
  { 0, "endgrent",  handle_ldap_endpwent  },
  { 0, "getpwnam",  handle_ldap_getpwnam  },
  { 0, "getpwuid",  handle_ldap_getpwuid  },
  { 0, "getgrnam",  handle_ldap_getgrnam  },
  { 0, "getgrgid",  handle_ldap_getgrgid  },
  { 0, "auth",      handle_ldap_is_auth   },
  { 0, "check",     handle_ldap_check     },
  { 0, "uid2name",  handle_ldap_uid_name  },
  { 0, "gid2name",  handle_ldap_gid_name  },
  { 0, "name2uid",  handle_ldap_name_uid  },
  { 0, "name2gid",  handle_ldap_name_gid  },
  { 0, "getgroups", handle_ldap_getgroups },
  { 0, NULL }
};

module ldap_module = {
  NULL, NULL,          /* Always NULL */
  0x20,                /* API Version 2.0 */
  "ldap",
  ldap_config,         /* Configuration directive table */
  ldap_cmdtab,         /* Command handlers */
  ldap_auth,           /* Authentication handlers */
  ldap_mod_init,       /* Initialization functions */
  ldap_getconf,
  MOD_LDAP_VERSION
};
