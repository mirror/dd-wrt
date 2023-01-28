/*
 * mod_ldap - LDAP password lookup module for ProFTPD
 * Copyright (c) 1999-2013, John Morrissey <jwm@horde.net>
 * Copyright (c) 2013-2021 The ProFTPD Project
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
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 *
 * Furthermore, John Morrissey gives permission to link this program with
 * OpenSSL, and distribute the resulting executable, without including the
 * source code for OpenSSL in the source distribution.
 *
 * -----DO NOT EDIT BELOW THIS LINE-----
 * $Libraries: -lldap -llber$
 */

#include "conf.h"
#include "privs.h"

#define MOD_LDAP_VERSION	"mod_ldap/2.9.5"

#if PROFTPD_VERSION_NUMBER < 0x0001030103
# error MOD_LDAP_VERSION " requires ProFTPD 1.3.4rc1 or later"
#endif

#if defined(HAVE_CRYPT_H) && !defined(AIX4) && !defined(AIX5)
# include <crypt.h>
#endif

#include <lber.h>
#include <ldap.h>
#if defined(HAVE_SASL_SASL_H)
# include <sasl/sasl.h>
#endif /* HAVE_SASL_SASL_H */

extern xaset_t *server_list;

module ldap_module;

static int ldap_logfd = -1;
static pool *ldap_pool = NULL;

static const char *trace_channel = "ldap";
#if defined(LBER_OPT_LOG_PRINT_FN)
static const char *libtrace_channel = "ldap.library";
#endif

#if LDAP_API_VERSION >= 2000
# define HAS_LDAP_SASL_BIND_S
#endif

#if defined(LDAP_SASL_QUIET)
# define HAS_LDAP_SASL_INTERACTIVE_BIND_S
#endif /* LDAP_SASL_QUIET */

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

/* On some systems LDAP_OPT_DIAGNOSTIC_MESSAGE isn't there (e.g. OpenLDAP-2.3.x)
 * but LDAP_OPT_ERROR_STRING is.
 */
#ifndef LDAP_OPT_DIAGNOSTIC_MESSAGE
# ifdef LDAP_OPT_ERROR_STRING
#  define LDAP_OPT_DIAGNOSTIC_MESSAGE LDAP_OPT_ERROR_STRING
# endif
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

static void pr_ldap_set_sizelimit(LDAP *limit_ld, int limit) {
#ifdef LDAP_OPT_SIZELIMIT
  int res;

  res = ldap_set_option(limit_ld, LDAP_OPT_SIZELIMIT, (void *) &limit);
  if (res != LDAP_OPT_SUCCESS) {
    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "failed to set LDAP option for search query size limit to %d entries: %s",
      limit, ldap_err2string(res));

  } else {
    pr_trace_msg(trace_channel, 5,
      "set search query size limit to %d entries", limit);
  }

#else
  limit_ld->ld_sizelimit = limit;

  pr_trace_msg(trace_channel, 5,
    "set search query size limit to %d entries", limit);
#endif
}

static int
LDAP_SEARCH(LDAP *ld, char *base, int scope, char *filter, char *attrs[],
            struct timeval *timeout, int sizelimit, LDAPMessage **res) {
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

/* Config entries */

struct server_info {
  const char *info_text;
  char *host;
  int port;
  char *port_text;
  LDAPURLDesc *url_desc;
  char *url_text;
  int use_starttls;

  /* For configuring the SSL/TLS session to the LDAP server. */
  const char *ssl_cert_file;
  const char *ssl_key_file;
  const char *ssl_ca_file;
  const char *ssl_ciphers;
  int ssl_verify;
  const char *ssl_verify_text;
};

struct sasl_info {
  pool *pool;

  /* SASL_AUTHCID from ldap.conf */
  const char *authentication_id;

  /* SASL_AUTHZID from ldap.conf */
  const char *authorization_id;

  const char *password;

  /* SASL_REALM from ldap.conf */
  const char *realm;
};

static array_header *ldap_servers = NULL;
static struct server_info *curr_server_info = NULL;
static unsigned int curr_server_index = 0;

static char *ldap_dn, *ldap_dnpass, *ldap_sasl_mechs = NULL,
            *ldap_user_basedn = NULL, *ldap_user_name_filter = NULL,
            *ldap_user_uid_filter = NULL,
            *ldap_gid_basedn = NULL, *ldap_group_gid_filter = NULL,
            *ldap_group_name_filter = NULL, *ldap_group_member_filter = NULL,
            *ldap_defaultauthscheme = "crypt", *ldap_authbind_dn = NULL,
            *ldap_genhdir_prefix = NULL, *ldap_default_quota = NULL,
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

static int ldap_do_users = FALSE, ldap_do_groups = FALSE,
           ldap_authbinds = TRUE, ldap_connecttimeout = 0,
           ldap_querytimeout = 0,
           ldap_genhdir = FALSE, ldap_genhdir_prefix_nouname = FALSE,
           ldap_forcedefaultuid = FALSE, ldap_forcedefaultgid = FALSE,
           ldap_forcegenhdir = FALSE, ldap_protocol_version = 3,
           ldap_dereference = LDAP_DEREF_NEVER,
           ldap_search_scope = LDAP_SCOPE_SUBTREE;

static size_t ldap_dnpasslen = 0;

static struct timeval ldap_connecttimeout_tv;

static struct timeval ldap_querytimeout_tv;
#define PR_LDAP_QUERY_TIMEOUT_DEFAULT		5

static uid_t ldap_defaultuid = -1;
static gid_t ldap_defaultgid = -1;
static LDAP *ld = NULL;
static array_header *cached_quota = NULL;
static array_header *cached_ssh_pubkeys = NULL;

/* Necessary prototypes */
static int ldap_sess_init(void);
static struct sasl_info *sasl_info_create(pool *, LDAP *);
static void sasl_info_get_authcid_from_dn(struct sasl_info *, const char *);

static void pr_ldap_unbind(void) {
  int res;

  if (ld == NULL) {
    return;
  }

  res = LDAP_UNBIND(ld);
  if (res != LDAP_SUCCESS) {
    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "error unbinding connection: %s", ldap_err2string(res));

  } else {
    pr_trace_msg(trace_channel, 8, "connection successfully unbound");
  }

  ld = NULL;
}

static void log_sasl_mechs(LDAP *conn_ld, const char *url_text) {
#if defined(LDAP_OPT_X_SASL_MECHLIST)
  char **sasl_mechs = NULL;

  if (ldap_get_option(conn_ld, LDAP_OPT_X_SASL_MECHLIST,
      &sasl_mechs) == LDAP_OPT_SUCCESS) {
    if (sasl_mechs != NULL) {
      register unsigned int i;

      for (i = 0; sasl_mechs[i]; i++) {
        pr_trace_msg(trace_channel, 22,
          "%s: LDAP supported SASL mechanism = %s", url_text, sasl_mechs[i]);
      }
    }
  }
#endif /* LDAP_OPT_X_SASL_MECHLIST */
}

static int do_ldap_prepare(LDAP **conn_ld) {
#if defined(HAS_LDAP_INITIALIZE)
  int res;
  char *url_text = NULL;

  if (curr_server_info != NULL) {
    url_text = curr_server_info->url_text;
    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "attempting connection to URL %s", url_text);

  } else {
    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "attempting connection (see ldap.conf for details)");
  }

  res = ldap_initialize(conn_ld, url_text);
  if (res != LDAP_SUCCESS) {
    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "ldap_initialize() to URL %s failed: %s",
      url_text ? url_text : "(null)", ldap_err2string(res));

    *conn_ld = NULL;
    return -1;
  }

  ldap_search_scope = curr_server_info->url_desc->lud_scope;

#else
  char *host = NULL;
  int port = LDAP_PORT;

  if (curr_server_info != NULL) {
    host = curr_server_info->host;
    port = curr_server_info->port;

    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "attempting connection to %s:%d", host, port);

  } else {
    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "attempting connection (see ldap.conf for details)");
  }

  *conn_ld = ldap_init(host, port);
  if (*conn_ld == NULL) {
    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "ldap_init() to %s:%d failed: %s", host ? host : "(null)", port,
      strerror(errno));
    return -1;
  }
#endif /* HAS_LDAP_INITIALIZE */

  log_sasl_mechs(*conn_ld, curr_server_info->info_text);
  return 0;
}

static int sasl_interact_cb(LDAP *conn_ld, unsigned int flags, void *user_data,
    void *interact_data) {
  int res = LDAP_OPERATIONS_ERROR;

#if defined(HAS_LDAP_SASL_INTERACTIVE_BIND_S) && \
    defined(HAVE_SASL_SASL_H)
  sasl_interact_t *interacts;
  struct sasl_info *sasl;

  interacts = interact_data;
  sasl = user_data;

  while (interacts->id != SASL_CB_LIST_END) {
    pr_signals_handle();

    switch (interacts->id) {
      case SASL_CB_AUTHNAME:
        interacts->result = sasl->authentication_id;
        interacts->len = strlen(interacts->result);
        pr_trace_msg(trace_channel, 19, "SASL interaction: CB_AUTHNAME = '%s'",
          (char *) interacts->result);
        break;

      case SASL_CB_GETREALM:
        /* The cyrus-sasl library works just fine with an empty string for our
         * currently supported mechanisms.
         */
        interacts->result = sasl->realm;
        interacts->len = strlen(interacts->result);
        pr_trace_msg(trace_channel, 19, "SASL interaction: CB_GETREALM = '%s'",
          (char *) interacts->result);
        break;

      case SASL_CB_PASS:
        interacts->result = sasl->password;
        interacts->len = strlen(interacts->result);
        pr_trace_msg(trace_channel, 19, "SASL interaction: CB_PASS = '...'");
        break;

      case SASL_CB_USER:
        /* The cyrus-sasl library handles this as the "authorization ID",
         * and works just fine with an empty string for our currently
         * supported mechanisms.
         */
        interacts->result = sasl->authorization_id;
        interacts->len = strlen(interacts->result);
        pr_trace_msg(trace_channel, 19, "SASL interaction: CB_USER = '%s'",
          (char *) interacts->result);
        break;

      case SASL_CB_ECHOPROMPT:
      case SASL_CB_NOECHOPROMPT:
        /* Ignore */
        break;

      default:
        break;
    }

    interacts++;
  }

  res = LDAP_SUCCESS;
#else
  pr_log_debug(DEBUG0, MOD_LDAP_VERSION
    ": interactive SASL authentication unsupported");
  res = LDAP_OPERATIONS_ERROR;
#endif /* HAS_LDAP_SASL_INTERACTIVE_BIND_S and HAVE_SASL_SASL_H */

  return res;
}

static int do_ldap_bind(LDAP *conn_ld) {
  int res;

#if defined(HAS_LDAP_SASL_INTERACTIVE_BIND_S)
  if (ldap_sasl_mechs != NULL) {
    int sasl_flags;
    struct sasl_info *sasl;

    pr_trace_msg(trace_channel, 17, "performing bind using SASL mechs: '%s'",
      ldap_sasl_mechs);

    sasl = sasl_info_create(session.pool, conn_ld);
    sasl_info_get_authcid_from_dn(sasl, ldap_dn);
    sasl->password = ldap_dnpass;

    sasl_flags = LDAP_SASL_QUIET;

    res = ldap_sasl_interactive_bind_s(conn_ld, ldap_dn, ldap_sasl_mechs,
      NULL, NULL, sasl_flags, sasl_interact_cb, sasl);

    destroy_pool(sasl->pool);

  } else {
    struct berval bindcred;

    bindcred.bv_val = ldap_dnpass;
    bindcred.bv_len = ldap_dnpasslen;

    res = ldap_sasl_bind_s(conn_ld, ldap_dn, NULL, &bindcred, NULL, NULL, NULL);

    if (res == LDAP_SUCCESS) {
      if (ldap_dnpasslen > 0) {
        pr_trace_msg(trace_channel, 9,
          "bind to '%s' used simple authentication",
          curr_server_info->info_text);

      } else {
        pr_trace_msg(trace_channel, 9,
          "bind to '%s' used anonymous authentication",
          curr_server_info->info_text);
      }
    }
  }
#else /* HAS_LDAP_SASL_BIND_S */
  res = ldap_simple_bind_s(conn_ld, ldap_dn, ldap_dnpass);
#endif /* HAS_LDAP_SASL_BIND_S */

  if (res != LDAP_SUCCESS) {
    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "bind as DN '%s' failed for '%s': %s",
      ldap_dn ? ldap_dn : "(anonymous)", curr_server_info->info_text,
      ldap_err2string(res));
    return -1;
  }

  (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
    "successfully bound as DN '%s' with password %s for '%s'",
    ldap_dn ? ldap_dn : "(anonymous)",
    ldap_dnpass ? "(see config)" : "(none)", curr_server_info->info_text);

  return 0;
}

static int do_ldap_connect(LDAP **conn_ld, int do_bind) {
  int res, version;

  /* Note that because we use ldap_init(3)/ldap_initialize(3), the first
   * actual TCP connection does not occur until the first operation is
   * requested.
   */
  res = do_ldap_prepare(conn_ld);
  if (res < 0) {
    return -1;
  }

  version = LDAP_VERSION3;
  if (ldap_protocol_version == 2) {
    version = LDAP_VERSION2;
  }

  res = ldap_set_option(*conn_ld, LDAP_OPT_PROTOCOL_VERSION, &version);
  if (res != LDAP_OPT_SUCCESS) {
    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "error setting LDAP protocol version option to %d: %s", version,
      ldap_err2string(res));
    pr_ldap_unbind();
    return -1;
  }

  (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
    "set LDAP protocol version to %d", version);

  if (ldap_connecttimeout_tv.tv_sec > 0) {
    ldap_connecttimeout_tv.tv_usec = 0;

#if defined(LDAP_OPT_NETWORK_TIMEOUT)
    res = ldap_set_option(*conn_ld, LDAP_OPT_NETWORK_TIMEOUT,
      &ldap_connecttimeout_tv);
    if (res != LDAP_OPT_SUCCESS) {
      (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
        "error setting network timeout option to %d: %s", ldap_connecttimeout,
        ldap_err2string(res));

    } else {
      (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
        "set connect timeout to %d %s", ldap_connecttimeout,
        ldap_connecttimeout != 1 ? "secs" : "sec");
    }
#else
    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "unable to set connect timeout %d due to lack of API support",
      ldap_connecttimeout);
#endif
  }

  if (curr_server_info->use_starttls == TRUE) {
#if defined(LDAP_OPT_X_TLS)
# if defined(LDAP_OPT_X_TLS_CACERTFILE)
    if (curr_server_info->ssl_ca_file != NULL) {
      res = ldap_set_option(NULL, LDAP_OPT_X_TLS_CACERTFILE,
        curr_server_info->ssl_ca_file);
      if (res != LDAP_OPT_SUCCESS) {
        pr_log_debug(DEBUG5, MOD_LDAP_VERSION
          ": error setting LDAP_OPT_X_TLS_CACERTFILE = %s: %s",
          curr_server_info->ssl_ca_file, ldap_err2string(res));

      } else {
        pr_trace_msg(trace_channel, 17,
          "set LDAP_OPT_X_TLS_CACERTFILE = %s for '%s'",
          curr_server_info->ssl_ca_file, curr_server_info->info_text);
      }
    }
# endif /* LDAP_OPT_X_TLS_CACERTFILE */

# if defined(LDAP_OPT_X_TLS_CERTFILE)
    if (curr_server_info->ssl_cert_file != NULL) {
      res = ldap_set_option(NULL, LDAP_OPT_X_TLS_CERTFILE,
        curr_server_info->ssl_cert_file);
      if (res != LDAP_OPT_SUCCESS) {
        pr_log_debug(DEBUG5, MOD_LDAP_VERSION
          ": error setting LDAP_OPT_X_TLS_CERTFILE = %s: %s",
          curr_server_info->ssl_cert_file, ldap_err2string(res));

      } else {
        pr_trace_msg(trace_channel, 17,
          "set LDAP_OPT_X_TLS_CERTFILE = %s for '%s'",
          curr_server_info->ssl_cert_file, curr_server_info->info_text);
      }
    }
# endif /* LDAP_OPT_X_TLS_CERTFILE */

# if defined(LDAP_OPT_X_TLS_KEYFILE)
    if (curr_server_info->ssl_key_file != NULL) {
      res = ldap_set_option(NULL, LDAP_OPT_X_TLS_KEYFILE,
        curr_server_info->ssl_key_file);
      if (res != LDAP_OPT_SUCCESS) {
        pr_log_debug(DEBUG5, MOD_LDAP_VERSION
          ": error setting LDAP_OPT_X_TLS_KEYFILE = %s: %s",
          curr_server_info->ssl_key_file, ldap_err2string(res));

      } else {
        pr_trace_msg(trace_channel, 17,
          "set LDAP_OPT_X_TLS_KEYFILE = %s for '%s'",
          curr_server_info->ssl_key_file, curr_server_info->info_text);
      }
    }
# endif /* LDAP_OPT_X_TLS_KEYFILE */

# if defined(LDAP_OPT_X_TLS_CIPHER_SUITE)
    if (curr_server_info->ssl_ciphers != NULL) {
      res = ldap_set_option(NULL, LDAP_OPT_X_TLS_CIPHER_SUITE,
        curr_server_info->ssl_ciphers);
      if (res != LDAP_OPT_SUCCESS) {
        pr_log_debug(DEBUG5, MOD_LDAP_VERSION
          ": error setting LDAP_OPT_X_TLS_CIPHER_SUITE = %s: %s",
          curr_server_info->ssl_ciphers, ldap_err2string(res));

      } else {
        pr_trace_msg(trace_channel, 17,
          "set LDAP_OPT_X_TLS_CIPHER_SUITE = %s for '%s'",
          curr_server_info->ssl_ciphers, curr_server_info->info_text);
      }
    }
# endif /* LDAP_OPT_X_TLS_CIPHER_SUITE */

# if defined(LDAP_OPT_X_TLS_REQUIRE_CERT)
    if (curr_server_info->ssl_verify != -1) {
      res = ldap_set_option(NULL, LDAP_OPT_X_TLS_REQUIRE_CERT,
        &(curr_server_info->ssl_verify));
      if (res != LDAP_OPT_SUCCESS) {
        pr_log_debug(DEBUG5, MOD_LDAP_VERSION
          ": error setting LDAP_OPT_X_TLS_REQUIRE_CERT = %s: %s",
          curr_server_info->ssl_verify_text, ldap_err2string(res));

      } else {
        pr_trace_msg(trace_channel, 17,
          "set LDAP_OPT_X_TLS_REQUIRE_CERT = %s for '%s'",
          curr_server_info->ssl_verify_text, curr_server_info->info_text);
      }
    }
# endif /* LDAP_OPT_X_TLS_REQUIRE_CERT */

    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "LDAPUseTLS in effect, performing STARTTLS handshake on '%s'",
      curr_server_info->info_text);
    res = ldap_start_tls_s(*conn_ld, NULL, NULL);
    if (res != LDAP_SUCCESS) {
      char *diag_msg = NULL;

      (void) ldap_get_option(*conn_ld, LDAP_OPT_DIAGNOSTIC_MESSAGE,
        (void *) &diag_msg);

      if (diag_msg != NULL) {
        (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
         "failed to start TLS: %s: %s", ldap_err2string(res), diag_msg);
        ldap_memfree(diag_msg);

      } else {
        (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
         "failed to start TLS: %s", ldap_err2string(res));
      }

      pr_ldap_unbind();
      return -1;
    }

    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "enabled TLS for connection to '%s'", curr_server_info->info_text);
  }
#endif /* LDAP_OPT_X_TLS */

  if (do_bind == TRUE) {
    res = do_ldap_bind(*conn_ld);
    if (res < 0) {
      pr_ldap_unbind();
      return -1;
    }
  }

#ifdef LDAP_OPT_DEREF
  res = ldap_set_option(*conn_ld, LDAP_OPT_DEREF, (void *) &ldap_dereference);
  if (res != LDAP_OPT_SUCCESS) {
    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "failed to set LDAP option for dereference to %d: %s", ldap_dereference,
      ldap_err2string(res));
    pr_ldap_unbind();
    return -1;
  }
#else
  deref_ld->ld_deref = ldap_dereference;
#endif

  (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
    "set dereferencing to %d", ldap_dereference);

  ldap_querytimeout_tv.tv_sec = (ldap_querytimeout > 0 ? ldap_querytimeout :
    PR_LDAP_QUERY_TIMEOUT_DEFAULT);
  ldap_querytimeout_tv.tv_usec = 0;

  (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
    "set query timeout to %u secs", (unsigned int) ldap_querytimeout_tv.tv_sec);

  return 0;
}

#if defined(LBER_OPT_LOG_PRINT_FN)
static void ldap_tracelog_cb(const char *msg) {
  (void) pr_trace_msg(libtrace_channel, 1, "%s", msg);
}
#endif /* no LBER_OPT_LOG_PRINT_FN */

static int pr_ldap_connect(LDAP **conn_ld, int do_bind) {
  unsigned int start_server_index;

  start_server_index = curr_server_index;
  do {
    int res, debug_level = -1;

    pr_signals_handle();

    /* Note that ldap_servers might be NULL if no LDAPServer directive was
     * specified.  We fall back to using the SDK default.
     */
    if (ldap_servers != NULL) {
      curr_server_info = ((struct server_info **) ldap_servers->elts)[curr_server_index];
    }

    res = do_ldap_connect(conn_ld, do_bind);
    if (res < 0) {
      ++curr_server_index;
      if (curr_server_index >= ldap_servers->nelts) {
        curr_server_index = 0;
      }

      continue;
    }

    /* This debug level value should be LDAP_DEBUG_ANY, but that macro is, I
     * think, OpenLDAP-specific.
     */
    res = ldap_set_option(*conn_ld, LDAP_OPT_DEBUG_LEVEL, &debug_level);
    if (res != LDAP_OPT_SUCCESS) {
      (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
        "error setting DEBUG_ANY debug level: %s", ldap_err2string(res));
    }

    return 0;

  } while (curr_server_index != start_server_index);

  return -1;
}

static const char *pr_ldap_interpolate_filter(pool *p, char *template,
    const char *value) {
  const char *escaped_value, *filter;

  escaped_value = sreplace(p, (char *) value,
    "\\", "\\\\",
    "*", "\\*",
    "(", "\\(",
    ")", "\\)",
    NULL
  );

  if (escaped_value == NULL) {
    return NULL;
  }

  filter = sreplace(p, template,
    "%u", escaped_value,
    "%v", escaped_value,
    NULL
  );

  if (filter == NULL) {
    return NULL;
  }

  (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
    "generated filter %s from template %s and value %s", filter, template,
    value);
  return filter;
}

static LDAPMessage *pr_ldap_search(const char *basedn, const char *filter,
    char *attrs[], int sizelimit, int retry) {
  int res;
  LDAPMessage *result;

  if (basedn == NULL) {
    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "no LDAP base DN specified for search filter %s, declining request",
      filter ? filter : "(null)");
    return NULL;
  }

  /* If the LDAP connection has gone away or hasn't been established
   * yet, attempt to establish it now.
   */
  if (ld == NULL) {
    /* If we _still_ can't connect, give up and return NULL. */
    if (pr_ldap_connect(&ld, TRUE) < 0) {
      return NULL;
    }
  }

  res = LDAP_SEARCH(ld, basedn, ldap_search_scope, filter, attrs,
    &ldap_querytimeout_tv, sizelimit, &result);
  if (res != LDAP_SUCCESS) {
    if (res != LDAP_SERVER_DOWN) {
      (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
        "LDAP search use DN '%s', filter '%s' failed: %s", basedn, filter,
        ldap_err2string(res));
      return NULL;
    }

    if (!retry) {
      (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
        "LDAP connection went away, search failed");
      pr_ldap_unbind();
      return NULL;
    }

    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "LDAP connection went away, retrying search operation");
    pr_ldap_unbind();
    return pr_ldap_search(basedn, filter, attrs, sizelimit, FALSE);
  }

  (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
    "searched under base DN %s using filter %s", basedn,
    filter ? filter : "(null)");
  return result;
}

static struct passwd *pr_ldap_user_lookup(pool *p, char *filter_template,
    const char *replace, const char *basedn, char *attrs[], char **user_dn) {
  const char *filter;
  char *dn;
  int i = 0;
  struct passwd *pw;
  LDAPMessage *result, *e;
  LDAP_VALUE_T **values;

  filter = pr_ldap_interpolate_filter(p, filter_template, replace);
  if (filter == NULL) {
    return NULL;
  }

  result = pr_ldap_search(basedn, filter, attrs, 2, TRUE);
  if (result == NULL) {
    return NULL;
  }

  if (ldap_count_entries(ld, result) > 1) {
    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "LDAP search returned multiple entries during user lookup, "
      "aborting query");
    ldap_msgfree(result);
    return NULL;
  }

  e = ldap_first_entry(ld, result);
  if (e == NULL) {
    ldap_msgfree(result);

    /* No LDAP entries for this user. */
    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "no entries for filter %s under base DN %s", filter, basedn);
    return NULL;
  }

  pw = pcalloc(ldap_pool, sizeof(struct passwd));
  while (attrs[i] != NULL) {
    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "fetching values for attribute %s", attrs[i]);

    values = LDAP_GET_VALUES(ld, e, attrs[i]);
    if (values == NULL) {
      (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
        "no values for attribute %s, trying defaults", attrs[i]);

      /* Apply default values for attrs with no explicit values. */

      /* If we can't find the [ug]idNumber attrs, just fill the passwd
       * struct in with default values from the config file.
       */
      if (strcasecmp(attrs[i], ldap_attr_uidnumber) == 0) {
        if (ldap_defaultuid == (uid_t) -1) {
          dn = ldap_get_dn(ld, e);

          (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
            "no %s attribute for DN %s found, and LDAPDefaultUID not "
            "configured", ldap_attr_uidnumber, dn);
          free(dn);
          return NULL;
        }

        pw->pw_uid = ldap_defaultuid;
        ++i;

        (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
          "using LDAPDefaultUID %s", pr_uid2str(NULL, pw->pw_uid));
        continue;
      }

      if (strcasecmp(attrs[i], ldap_attr_gidnumber) == 0) {
        if (ldap_defaultgid == (gid_t) -1) {
          dn = ldap_get_dn(ld, e);

          (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
            "no %s attribute found for DN %s,  and LDAPDefaultGID not "
            "configured", ldap_attr_gidnumber, dn);
          free(dn);
          return NULL;
        }

        pw->pw_gid = ldap_defaultgid;
        ++i;

        (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
          "using LDAPDefaultGID %s", pr_gid2str(NULL, pw->pw_gid));
        continue;
      }

      if (strcasecmp(attrs[i], ldap_attr_homedirectory) == 0) {
        if (ldap_genhdir == FALSE ||
            ldap_genhdir_prefix == NULL) {
          dn = ldap_get_dn(ld, e);

          if (ldap_genhdir == FALSE) {
            (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
              "no %s attribute for DN %s, LDAPGenerateHomedir not enabled",
              ldap_attr_homedirectory, dn);

          } else {
            (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
              "no %s attribute for DN %s, LDAPGenerateHomedir enabled but "
              "LDAPGenerateHomedirPrefix not configured",
              ldap_attr_homedirectory, dn);
          }

          free(dn);
          return NULL;
        }

        if (ldap_genhdir_prefix_nouname == TRUE) {
          pw->pw_dir = pstrcat(session.pool, ldap_genhdir_prefix, NULL);

        } else {
          LDAP_VALUE_T **canon_username;
          canon_username = LDAP_GET_VALUES(ld, e, ldap_attr_uid);
          if (canon_username == NULL) {
            dn = ldap_get_dn(ld, e);

            (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
              "could not get %s attribute for canonical username for DN %s",
              ldap_attr_uid, dn);
            free(dn);
            return NULL;
          }

          pw->pw_dir = pstrcat(session.pool, ldap_genhdir_prefix, "/",
            LDAP_VALUE(canon_username, 0), NULL);
          LDAP_VALUE_FREE(canon_username);
        }

        ++i;

        (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
          "using default homedir %s", pw->pw_dir);
        continue;
      }

      /* Don't worry if we don't have a loginShell attr. */
      if (strcasecmp(attrs[i], ldap_attr_loginshell) == 0) {
        /* Prevent a segfault if no loginShell attribute, and
         * "RequireValidShell on" is in effect.
         */
        pw->pw_shell = pstrdup(session.pool, "");
        ++i;
        continue;
      }

      /* We only restart the while loop above if we can fill in alternate
       * values for certain attributes. If something odd has happened, we
       * fall through so we can complain.
       */

      dn = ldap_get_dn(ld, e);

      (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
        "could not get values for attribute %s for DN %s, ignoring request "
        "(perhaps this DN's entry does not have the attribute?)", attrs[i], dn);
      free(dn);
      ldap_msgfree(result);
      return NULL;
    }

    /* Now that we've handled default values, fill in the struct as normal;
     * the if branches below for nonexistent attrs will just never be
     * called.
     */

    if (strcasecmp(attrs[i], ldap_attr_uid) == 0) {
      pw->pw_name = pstrdup(session.pool, LDAP_VALUE(values, 0));

    } else if (strcasecmp(attrs[i], ldap_attr_userpassword) == 0) {
      pw->pw_passwd = pstrdup(session.pool, LDAP_VALUE(values, 0));

    } else if (strcasecmp(attrs[i], ldap_attr_uidnumber) == 0) {
      if (ldap_forcedefaultuid == TRUE &&
          ldap_defaultuid != (uid_t) -1) {
        pw->pw_uid = ldap_defaultuid;

      } else {
        pw->pw_uid = (uid_t) strtoul(LDAP_VALUE(values, 0), NULL, 10);
      }

    } else if (strcasecmp(attrs[i], ldap_attr_gidnumber) == 0) {
      if (ldap_forcedefaultgid == TRUE &&
          ldap_defaultgid != (gid_t) -1) {
        pw->pw_gid = ldap_defaultgid;

      } else {
        pw->pw_gid = (gid_t) strtoul(LDAP_VALUE(values, 0), NULL, 10);
      }

    } else if (strcasecmp(attrs[i], ldap_attr_homedirectory) == 0) {
      if (ldap_forcegenhdir == TRUE) {
        if (ldap_genhdir == FALSE ||
            ldap_genhdir_prefix == NULL) {

          if (ldap_genhdir == FALSE) {
            (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
              "LDAPForceGeneratedHomedir enabled but LDAPGenerateHomedir is "
              "not enabled");

          } else {
            (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
              "LDAPForceGeneratedHomedir and LDAPGenerateHomedir enabled, but "
              "missing required LDAPGenerateHomedirPrefix");
          }

          return NULL;
        }

        if (pw->pw_dir != NULL) {
          pr_trace_msg(trace_channel, 8, "LDAPForceGeneratedHomedir in effect, "
            "overriding current LDAP home directory '%s'", pw->pw_dir);
        }

        if (ldap_genhdir_prefix_nouname == TRUE) {
          pw->pw_dir = pstrdup(session.pool, ldap_genhdir_prefix);

        } else {
          LDAP_VALUE_T **canon_username;
          canon_username = LDAP_GET_VALUES(ld, e, ldap_attr_uid);
          if (canon_username == NULL) {
            dn = ldap_get_dn(ld, e);

            (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
              "could not get %s attribute for canonical username for DN %s",
              ldap_attr_uid, dn);
            free(dn);
            return NULL;
          }

          pw->pw_dir = pstrcat(session.pool, ldap_genhdir_prefix, "/",
            LDAP_VALUE(canon_username, 0), NULL);
          LDAP_VALUE_FREE(canon_username);
        }

      } else {
        pw->pw_dir = pstrdup(session.pool, LDAP_VALUE(values, 0));
      }

      pr_trace_msg(trace_channel, 8, "using LDAP home directory '%s'",
        pw->pw_dir);

    } else if (strcasecmp(attrs[i], ldap_attr_loginshell) == 0) {
      pw->pw_shell = pstrdup(session.pool, LDAP_VALUE(values, 0));

    } else {
      (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
        "user lookup attribute/value loop found unknown attribute %s",
        attrs[i]);
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

  (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
    "found user %s, UID %s, GID %s, homedir %s, shell %s",
    pw->pw_name, pr_uid2str(p, pw->pw_uid), pr_gid2str(p, pw->pw_gid),
    pw->pw_dir, pw->pw_shell);
  return pw;
}

static struct group *pr_ldap_group_lookup(pool *p, char *filter_template,
    const char *replace, char *attrs[]) {
  const char *filter;
  char *dn;
  int i = 0, value_count = 0, value_offset;
  struct group *gr;
  LDAPMessage *result, *e;
  LDAP_VALUE_T **values;

  if (ldap_gid_basedn == NULL) {
    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "no LDAP base DN specified for group lookups");
    return NULL;
  }

  filter = pr_ldap_interpolate_filter(p, filter_template, replace);
  if (filter == NULL) {
    return NULL;
  }

  result = pr_ldap_search(ldap_gid_basedn, filter, attrs, 2, TRUE);
  if (result == NULL) {
    return NULL;
  }

  e = ldap_first_entry(ld, result);
  if (e == NULL) {
    ldap_msgfree(result);

    /* No LDAP entries found for this user. */
    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "no group entries for filter %s", filter);
    return NULL;
  }

  gr = pcalloc(session.pool, sizeof(struct group));
  while (attrs[i] != NULL) {
    pr_signals_handle();

    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "fetching values for attribute %s", attrs[i]);

    values = LDAP_GET_VALUES(ld, e, attrs[i]);
    if (values == NULL) {
      if (strcasecmp(attrs[i], ldap_attr_memberuid) == 0) {
        gr->gr_mem = palloc(session.pool, 2 * sizeof(char *));
        gr->gr_mem[0] = pstrdup(session.pool, "");
        gr->gr_mem[1] = NULL;

        ++i;
        continue;
      }

      ldap_msgfree(result);
      dn = ldap_get_dn(ld, e);

      (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
        "could not get values for attribute %s for DN %s, ignoring request "
        "(perhaps that DN does not have that attribute?)", attrs[i], dn);
      free(dn);
      return NULL;
    }

    if (strcasecmp(attrs[i], ldap_attr_cn) == 0) {
      gr->gr_name = pstrdup(session.pool, LDAP_VALUE(values, 0));

    } else if (strcasecmp(attrs[i], ldap_attr_gidnumber) == 0) {
      gr->gr_gid = strtoul(LDAP_VALUE(values, 0), NULL, 10);

    } else if (strcasecmp(attrs[i], ldap_attr_memberuid) == 0) {
      value_count = LDAP_COUNT_VALUES(values);
      gr->gr_mem = (char **) palloc(session.pool, value_count * sizeof(char *));

      for (value_offset = 0; value_offset < value_count; ++value_offset) {
        gr->gr_mem[value_offset] =
          pstrdup(session.pool, LDAP_VALUE(values, value_offset));
      }

    } else {
      (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
        "group lookup attribute/value loop found unknown attribute %s",
        attrs[i]);
    }

    LDAP_VALUE_FREE(values);
    ++i;
  }

  ldap_msgfree(result);

  (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
    "found group %s, GID %s", gr->gr_name, pr_gid2str(NULL, gr->gr_gid));
  for (i = 0; i < value_count; ++i) {
    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "+ member: %s", gr->gr_mem[i]);
  }

  return gr;
}

static void parse_quota(pool *p, const char *replace, char *str) {
  char **elts, *token;

  if (cached_quota == NULL) {
    cached_quota = make_array(p, 9, sizeof(char *));
  }

  elts = (char **) cached_quota->elts;
  elts[0] = pstrdup(session.pool, replace);
  cached_quota->nelts = 1;

  (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
    "parsing ftpQuota attribute value '%s'", str);

  while ((token = strsep(&str, ","))) {
    pr_signals_handle();
    *((char **) push_array(cached_quota)) = pstrdup(session.pool, token);
  }
}

static unsigned char pr_ldap_quota_lookup(pool *p, char *filter_template,
    const char *replace, const char *basedn) {
  const char *filter = NULL;
  char *attrs[] = {
         ldap_attr_ftpquota, ldap_attr_ftpquota_profiledn, NULL,
       };
  int orig_scope, res;
  LDAPMessage *result, *e;
  LDAP_VALUE_T **values;

  if (basedn == NULL) {
    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "no LDAP base DN specified for quota lookups, declining request");
    return FALSE;
  }

  if (filter_template != NULL) {
    filter = pr_ldap_interpolate_filter(p, filter_template, replace);
    if (filter == NULL) {
      return FALSE;
    }
  }

  result = pr_ldap_search(basedn, filter, attrs, 2, TRUE);
  if (result == NULL) {
    return FALSE;
  }

  if (ldap_count_entries(ld, result) > 1) {
    ldap_msgfree(result);

    if (ldap_default_quota != NULL) {
      (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
        "multiple entries found for DN %s, using default quota %s", basedn,
          ldap_default_quota);
      parse_quota(p, replace, pstrdup(p, ldap_default_quota));
      return TRUE;

    } else {
      (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
        "multiple entries found for DN %s, aborting query", basedn);
    }

    return FALSE;
  }

  e = ldap_first_entry(ld, result);
  if (e == NULL) {
    ldap_msgfree(result);
    if (ldap_default_quota == NULL) {
      if (filter == NULL) {
        (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
          "no entries for DN %s, and no default quota defined", basedn);

      } else {
        (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
          "no entries for filter %s, and no default quota defined", filter);
      }

      return FALSE;
    }

    if (filter == NULL) {
      (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
        "no entries for DN %s, using default quota %s", basedn,
        ldap_default_quota);

    } else {
      (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
        "no entries for filter %s, using default quota %s", filter,
        ldap_default_quota);
    }

    parse_quota(p, replace, pstrdup(p, ldap_default_quota));
    return TRUE;
  }

  values = LDAP_GET_VALUES(ld, e, attrs[0]);
  if (values != NULL) {
    parse_quota(p, replace, pstrdup(p, LDAP_VALUE(values, 0)));
    LDAP_VALUE_FREE(values);
    ldap_msgfree(result);
    return TRUE;
  }

  if (filter == NULL) {
    if (ldap_default_quota == NULL) {
      (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
        "referenced DN %s does not have an ftpQuota attribute, and no "
        "default quota defined", basedn);
      return FALSE;
    }

    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "no ftpQuota attribute found for DN %s, using default quota %s", basedn,
      ldap_default_quota);
    parse_quota(p, replace, pstrdup(p, ldap_default_quota));
    return TRUE;
  }

  values = LDAP_GET_VALUES(ld, e, attrs[1]);
  if (values != NULL) {
    orig_scope = ldap_search_scope;
    ldap_search_scope = LDAP_SCOPE_BASE;
    res = pr_ldap_quota_lookup(p, NULL, replace, LDAP_VALUE(values, 0));
    ldap_search_scope = orig_scope;
    LDAP_VALUE_FREE(values);
    ldap_msgfree(result);
    return res;
  }

  ldap_msgfree(result);
  if (ldap_default_quota != NULL) {
    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "no %s or %s attribute, using default quota %s", attrs[0], attrs[1],
      ldap_default_quota);
    parse_quota(p, replace, pstrdup(p, ldap_default_quota));
    return TRUE;
  }

  /* No quota attributes for this user. */
  (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
    "no %s or %s attribute, and no default quota defined", attrs[0], attrs[1]);
  return FALSE;
}

static unsigned char pr_ldap_ssh_pubkey_lookup(pool *p, char *filter_template,
    const char *replace, char *basedn) {
  const char *filter;
  char *attrs[] = {
    ldap_attr_ssh_pubkey, NULL,
  };
  int num_keys, i;
  LDAPMessage *result, *e;
  LDAP_VALUE_T **values;

  if (basedn == NULL) {
    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "no LDAP base DN specified for user lookups, declining SSH publickey "
      "lookup request");
    return FALSE;
  }

  filter = pr_ldap_interpolate_filter(p, filter_template, replace);
  if (filter == NULL) {
    return FALSE;
  }

  result = pr_ldap_search(basedn, filter, attrs, 2, TRUE);
  if (result == NULL) {
    return FALSE;
  }

  if (ldap_count_entries(ld, result) > 1) {
    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "LDAP search for SSH publickey using DN %s, filter %s returned multiple "
      "entries, aborting query", basedn, filter);
    ldap_msgfree(result);
    return FALSE;
  }

  e = ldap_first_entry(ld, result);
  if (e == NULL) {
    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "LDAP search for SSH publickey using DN %s, filter %s returned "
      "no entries", basedn, filter);
    ldap_msgfree(result);
    return FALSE;
  }

  values = LDAP_GET_VALUES(ld, e, attrs[0]);
  if (values == NULL) {
    return FALSE;
  }

  num_keys = LDAP_COUNT_VALUES(values);
  cached_ssh_pubkeys = make_array(p, num_keys, sizeof(char *));
  for (i = 0; i < num_keys; ++i) {
    *((char **) push_array(cached_ssh_pubkeys)) = pstrdup(p,
      LDAP_VALUE(values, i));
  }
  LDAP_VALUE_FREE(values);

  ldap_msgfree(result);
  return TRUE;
}

static struct group *pr_ldap_getgrnam(pool *p, const char *group_name) {
  char *group_attrs[] = {
    ldap_attr_cn, ldap_attr_gidnumber, ldap_attr_memberuid, NULL,
  };

  return pr_ldap_group_lookup(p, ldap_group_name_filter, group_name,
    group_attrs);
}

static struct group *pr_ldap_getgrgid(pool *p, gid_t gid) {
  const char *gidstr;
  char *group_attrs[] = {
    ldap_attr_cn, ldap_attr_gidnumber, ldap_attr_memberuid, NULL,
  };

  gidstr = pr_gid2str(p, gid);
  return pr_ldap_group_lookup(p, ldap_group_gid_filter, gidstr, group_attrs);
}

static struct passwd *pr_ldap_getpwnam(pool *p, const char *username) {
  const char *filter;
  char *name_attrs[] = {
         ldap_attr_userpassword, ldap_attr_uid, ldap_attr_uidnumber,
         ldap_attr_gidnumber, ldap_attr_homedirectory,
         ldap_attr_loginshell, NULL,
       };

  filter = pr_ldap_interpolate_filter(p, ldap_user_basedn, username);
  if (filter == NULL) {
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
   * a crypted password to ldap_auth_check(), which will NOT do auth binds
   * in order to support UserPassword. (Otherwise, it would try binding to
   * the directory and would ignore UserPassword.)
   *
   * We're reasonably safe in making that assumption as long as we never
   * fetch userPassword from the directory if auth binds are enabled. If we
   * fetched userPassword, auth binds would never be done because
   * ldap_auth_check() would always get a crypted password.
   */
  return pr_ldap_user_lookup(p, ldap_user_name_filter, username, filter,
    ldap_authbinds ? name_attrs + 1 : name_attrs,
    ldap_authbinds ? &ldap_authbind_dn : NULL);
}

static struct passwd *pr_ldap_getpwuid(pool *p, uid_t uid) {
  const char *uidstr;
  char *uid_attrs[] = {
    ldap_attr_uid, ldap_attr_uidnumber, ldap_attr_gidnumber,
    ldap_attr_homedirectory, ldap_attr_loginshell, NULL,
  };

  uidstr = pr_uid2str(p, uid);
  return pr_ldap_user_lookup(p, ldap_user_uid_filter, uidstr,
    ldap_user_basedn, uid_attrs, ldap_authbinds ? &ldap_authbind_dn : NULL);
}

MODRET handle_ldap_quota_lookup(cmd_rec *cmd) {
  const char *basedn;

  basedn = pr_ldap_interpolate_filter(cmd->tmp_pool,
    ldap_user_basedn, cmd->argv[0]);
  if (basedn == NULL) {
    return PR_DECLINED(cmd);
  }

  if (cached_quota == NULL ||
      strcasecmp(((char **) cached_quota->elts)[0], cmd->argv[0]) != 0) {

    if (pr_ldap_quota_lookup(cmd->tmp_pool, ldap_user_name_filter,
        cmd->argv[0], basedn) == FALSE) {
      return PR_DECLINED(cmd);
    }

  } else {
    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "returning cached quota for user %s", (char *) cmd->argv[0]);
  }

  return mod_create_data(cmd, cached_quota);
}

MODRET handle_ldap_ssh_pubkey_lookup(cmd_rec *cmd) {
  char *user;

  if (ldap_do_users == FALSE) {
    return PR_DECLINED(cmd);
  }

  user = cmd->argv[0];

  if (cached_ssh_pubkeys != NULL &&
      cached_ssh_pubkeys->nelts > 0 &&
      strcasecmp(((char **) cached_ssh_pubkeys->elts)[0], user) == 0) {

    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "returning cached SSH public keys for user %s", user);
    return mod_create_data(cmd, cached_ssh_pubkeys);
  }

  if (pr_ldap_ssh_pubkey_lookup(cmd->tmp_pool, ldap_user_name_filter,
      user, ldap_user_basedn) == FALSE) {
    return PR_DECLINED(cmd);
  }

  return mod_create_data(cmd, cached_ssh_pubkeys);
}

/* Auth handlers */
MODRET ldap_auth_setpwent(cmd_rec *cmd) {
  if (ldap_do_users == FALSE &&
      ldap_do_groups == FALSE) {
    return PR_DECLINED(cmd);
  }

  if (ld == NULL) {
    (void) pr_ldap_connect(&ld, TRUE);
  }

  return PR_HANDLED(cmd);
}

MODRET ldap_auth_endpwent(cmd_rec *cmd) {
  if (ldap_do_users == FALSE &&
      ldap_do_groups == FALSE) {
    return PR_DECLINED(cmd);
  }

  pr_ldap_unbind();
  return PR_HANDLED(cmd);
}

MODRET ldap_auth_getpwuid(cmd_rec *cmd) {
  struct passwd *pw = NULL;

  if (ldap_do_users == FALSE) {
    return PR_DECLINED(cmd);
  }

  pw = pr_ldap_getpwuid(cmd->tmp_pool, *((uid_t *) cmd->argv[0]));
  if (pw != NULL) {
    return mod_create_data(cmd, pw);
  }

  return PR_DECLINED(cmd);
}

MODRET ldap_auth_getpwnam(cmd_rec *cmd) {
  struct passwd *pw = NULL;

  if (ldap_do_users == FALSE) {
    return PR_DECLINED(cmd);
  }

  pw = pr_ldap_getpwnam(cmd->tmp_pool, cmd->argv[0]);
  if (pw != NULL) {
    return mod_create_data(cmd, pw);
  }

  return PR_DECLINED(cmd);
}

MODRET ldap_auth_getgrnam(cmd_rec *cmd) {
  struct group *gr = NULL;

  if (ldap_do_groups == FALSE) {
    return PR_DECLINED(cmd);
  }

  gr = pr_ldap_getgrnam(cmd->tmp_pool, cmd->argv[0]);
  if (gr != NULL) {
    return mod_create_data(cmd, gr);
  }

  return PR_DECLINED(cmd);
}

MODRET ldap_auth_getgrgid(cmd_rec *cmd) {
  struct group *gr = NULL;

  if (ldap_do_groups == FALSE) {
    return PR_DECLINED(cmd);
  }

  gr = pr_ldap_getgrgid(cmd->tmp_pool, *((gid_t *) cmd->argv[0]));
  if (gr != NULL) {
    return mod_create_data(cmd, gr);
  }

  return PR_DECLINED(cmd);
}

MODRET ldap_auth_getgroups(cmd_rec *cmd) {
  const char *filter;
  char *w[] = {
    ldap_attr_gidnumber, ldap_attr_cn, NULL,
  };
  struct passwd *pw;
  struct group *gr;
  LDAPMessage *result = NULL, *e;
  LDAP_VALUE_T **gidNumber, **cn;
  array_header *gids   = (array_header *)cmd->argv[1],
               *groups = (array_header *)cmd->argv[2];

  if (ldap_do_groups == FALSE) {
    return PR_DECLINED(cmd);
  }

  if (gids == NULL ||
      groups == NULL) {
    return PR_DECLINED(cmd);
  }

  pw = pr_ldap_getpwnam(cmd->tmp_pool, cmd->argv[0]);
  if (pw != NULL) {
    gr = pr_ldap_getgrgid(cmd->tmp_pool, pw->pw_gid);
    if (gr != NULL) {
      (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
        "adding user %s primary group %s/%s", pw->pw_name, gr->gr_name,
        pr_gid2str(NULL, pw->pw_gid));
      *((gid_t *) push_array(gids)) = pw->pw_gid;
      *((char **) push_array(groups)) = pstrdup(session.pool, gr->gr_name);

    } else {
      (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
        "unable to determine group name for user %s primary GID %s, skipping",
        pw->pw_name, pr_gid2str(NULL, pw->pw_gid));
    }
  }

  if (ldap_gid_basedn == NULL) {
    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "no LDAP base DN specified for group lookups");
    goto return_groups;
  }

  filter = pr_ldap_interpolate_filter(cmd->tmp_pool,
    ldap_group_member_filter, cmd->argv[0]);
  if (filter == NULL) {
    return NULL;
  }

  result = pr_ldap_search(ldap_gid_basedn, filter, w, 0, TRUE);
  if (result == NULL) {
    return FALSE;
  }

  if (ldap_count_entries(ld, result) == 0) {
    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "no entries found for filter %s", filter);
    goto return_groups;
  }

  for (e = ldap_first_entry(ld, result); e; e = ldap_next_entry(ld, e)) {
    gidNumber = LDAP_GET_VALUES(ld, e, w[0]);
    if (gidNumber == NULL) {
      (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
        "could not get values for %s attribute for getgroups(2), skipping "
        "current group", ldap_attr_gidnumber);
      continue;
    }

    cn = LDAP_GET_VALUES(ld, e, w[1]);
    if (cn == NULL) {
      (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
        "could not get values for %s attribute for getgroups(2), skipping "
        "current group", ldap_attr_cn);
      continue;
    }

    if (pw == NULL ||
        strtoul(LDAP_VALUE(gidNumber, 0), NULL, 10) != pw->pw_gid) {
      *((gid_t *) push_array(gids)) = strtoul(LDAP_VALUE(gidNumber, 0), NULL, 10);
      *((char **) push_array(groups)) = pstrdup(session.pool, LDAP_VALUE(cn, 0));

      (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
        "added user %s secondary group %s/%s",
        (pw && pw->pw_name) ? pw->pw_name : (char *) cmd->argv[0],
        LDAP_VALUE(cn, 0), LDAP_VALUE(gidNumber, 0));
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

/* cmd->argv[0] : user name
 * cmd->argv[1] : cleartext password
 */
MODRET ldap_auth_auth(cmd_rec *cmd) {
  const char *filter = NULL, *username;
  char *pass_attrs[] = {
         ldap_attr_userpassword, ldap_attr_uid, ldap_attr_uidnumber,
         ldap_attr_gidnumber, ldap_attr_homedirectory,
         ldap_attr_loginshell, NULL,
       };
  struct passwd *pw = NULL;
  int res;

  if (ldap_do_users == FALSE) {
    return PR_DECLINED(cmd);
  }

  username = cmd->argv[0];

  filter = pr_ldap_interpolate_filter(cmd->tmp_pool, ldap_user_basedn,
    username);
  if (filter == NULL) {
    return NULL;
  }

  /* If anything here fails hard (IOW, we've found an LDAP entry for the
   * user, but they appear to have entered the wrong password), fail auth.
   * Normally, I'd DECLINE here so other modules could have a shot, but if
   * we've found their LDAP entry, chances are that nothing else will be
   * able to auth them.
   */

  pw = pr_ldap_user_lookup(cmd->tmp_pool,
    ldap_user_name_filter, username, filter,
    ldap_authbinds ? pass_attrs + 1 : pass_attrs,
    ldap_authbinds ? &ldap_authbind_dn : NULL);
  if (pw == NULL) {
    /* Can't find the user in the LDAP directory. */
    return PR_DECLINED(cmd);
  }

  if (ldap_authbinds == FALSE &&
      pw->pw_passwd == NULL) {
    (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
      "LDAPAuthBinds not enabled, and unable to retrieve password for user %s",
      pw->pw_name);
    return PR_ERROR_INT(cmd, PR_AUTH_NOPWD);
  }

  res = pr_auth_check(cmd->tmp_pool, ldap_authbinds ? NULL : pw->pw_passwd,
    username, cmd->argv[1]);
  if (res != 0) {
    if (res == -1) {
      (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
        "bad password for user %s: %s", pw->pw_name, strerror(errno));

    } else {
      (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
        "bad password for user %s", pw->pw_name);
    }

    return PR_ERROR_INT(cmd, PR_AUTH_BADPWD);
  }

  session.auth_mech = "mod_ldap.c";
  return PR_HANDLED(cmd);
}

/* cmd->argv[0] = hashed password
 * cmd->argv[1] = user
 * cmd->argv[2] = cleartext
 */
MODRET ldap_auth_check(cmd_rec *cmd) {
  char *pass, *cryptpass, *hash_method, *crypted;
  int encname_len, res;
  LDAP *ld_auth;
#ifdef HAS_LDAP_SASL_BIND_S
  struct berval bindcred;
#endif

  if (ldap_do_users == FALSE) {
    return PR_DECLINED(cmd);
  }

  cryptpass = cmd->argv[0];
  pass = cmd->argv[2];

  /* At this point, any encrypted password must have come from the UserPassword
   * directive. Don't perform auth binds in this case, since the crypted
   * password specified should override auth binds.
   */
  if (ldap_authbinds == TRUE &&
      cryptpass == NULL) {
    /* Don't try to do auth binds with a NULL/empty DN or password. */
    if (pass == NULL ||
        strlen(pass) == 0) {
      (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
        "LDAPAuthBinds is enabled, but no user-supplied cleartext password "
        "was found");
      return PR_DECLINED(cmd);
    }

    if (ldap_authbind_dn == NULL ||
        strlen(ldap_authbind_dn) == 0) {
      (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
        "LDAPAuthBinds is enabled, but no LDAP DN was found");
      return PR_DECLINED(cmd);
    }

    if (pr_ldap_connect(&ld_auth, FALSE) < 0) {
      (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
        "unable to check login: LDAP connection failed");
      return PR_DECLINED(cmd);
    }

#ifdef HAS_LDAP_SASL_BIND_S
    bindcred.bv_val = cmd->argv[2];
    bindcred.bv_len = strlen(cmd->argv[2]);
    res = ldap_sasl_bind_s(ld_auth, ldap_authbind_dn, NULL, &bindcred,
      NULL, NULL, NULL);
#else /* HAS_LDAP_SASL_BIND_S */
    res = ldap_simple_bind_s(ld_auth, ldap_authbind_dn, cmd->argv[2]);
#endif /* HAS_LDAP_SASL_BIND_S */

    if (res != LDAP_SUCCESS) {
      if (res != LDAP_INVALID_CREDENTIALS) {
        (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
          "unable to check login: bind as %s failed: %s", ldap_authbind_dn,
          ldap_err2string(res));
      }

      (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
        "invalid credentials used for %s", ldap_authbind_dn);
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

  if ((size_t) encname_len == strlen(cryptpass + 1)) {
    /* No leading {scheme}. */
    hash_method = ldap_defaultauthscheme;
    encname_len = 0;

  } else {
    encname_len += 2;
  }

  /* The {crypt} scheme */
  if (strncasecmp(hash_method, "crypt", strlen(hash_method)) == 0) {
    crypted = crypt(pass, cryptpass + encname_len);
    if (crypted == NULL) {
      pr_trace_msg(trace_channel, 19,
        "using %s auth scheme, crypt(3) failed: %s", hash_method,
        strerror(errno));
      return PR_ERROR(cmd);
    }

    if (strcmp(crypted, cryptpass + encname_len) != 0) {
      pr_trace_msg(trace_channel, 19,
        "using '%s' auth scheme, comparison failed", hash_method);
      return PR_ERROR(cmd);
    }

  /* The {clear} scheme */
  } else if (strncasecmp(hash_method, "clear", strlen(hash_method)) == 0) {
    if (strcmp(pass, cryptpass + encname_len) != 0) {
      pr_trace_msg(trace_channel, 19,
        "using '%s' auth scheme, comparison failed", hash_method);
      return PR_ERROR(cmd);
    }

  } else {
    /* Can't find a supported {scheme} */
    pr_trace_msg(trace_channel, 3,
      "unsupported userPassword auth scheme: %s", hash_method);
    return PR_DECLINED(cmd);
  }

  session.auth_mech = "mod_ldap.c";
  return PR_HANDLED(cmd);
}

MODRET ldap_auth_uid2name(cmd_rec *cmd) {
  struct passwd *pw = NULL;

  if (ldap_do_users == FALSE) {
    return PR_DECLINED(cmd);
  }

  pw = pr_ldap_getpwuid(cmd->tmp_pool, *((uid_t *) cmd->argv[0]));
  if (pw == NULL) {
    /* Can't find the user in the LDAP directory. */
    return PR_DECLINED(cmd);
  }

  return mod_create_data(cmd, pstrdup(permanent_pool, pw->pw_name));
}

MODRET ldap_auth_gid2name(cmd_rec *cmd) {
  struct group *gr = NULL;

  if (ldap_do_groups == FALSE) {
    return PR_DECLINED(cmd);
  }

  gr = pr_ldap_getgrgid(cmd->tmp_pool, *((gid_t *) cmd->argv[0]));
  if (gr == NULL) {
    /* Can't find the user in the LDAP directory. */
    return PR_DECLINED(cmd);
  }

  return mod_create_data(cmd, pstrdup(permanent_pool, gr->gr_name));
}

MODRET ldap_auth_name2uid(cmd_rec *cmd) {
  struct passwd *pw = NULL;

  if (ldap_do_users == FALSE) {
    return PR_DECLINED(cmd);
  }

  pw = pr_ldap_getpwnam(cmd->tmp_pool, cmd->argv[0]);
  if (pw == NULL) {
    return PR_DECLINED(cmd);
  }

  return mod_create_data(cmd, (void *) &pw->pw_uid);
}

MODRET ldap_auth_name2gid(cmd_rec *cmd) {
  struct group *gr = NULL;

  if (ldap_do_groups == FALSE) {
    return PR_DECLINED(cmd);
  }

  gr = pr_ldap_getgrnam(cmd->tmp_pool, cmd->argv[0]);
  if (gr == NULL) {
    return PR_DECLINED(cmd);
  }

  return mod_create_data(cmd, (void *) &gr->gr_gid);
}

/* sasl_info functions. */

/* Note: by proving empty strings, rather than NULLs, for the default values,
 * we make the logic in sasl_interact_cb() simpler.
 */
static struct sasl_info *sasl_info_create(pool *p, LDAP *conn_ld) {
  pool *sasl_pool;
  struct sasl_info *sasl;

  sasl_pool = make_sub_pool(p);
  pr_pool_tag(sasl_pool, "SASL Info Pool");

  sasl = pcalloc(sasl_pool, sizeof(struct sasl_info));
  sasl->pool = sasl_pool;

#if defined(LDAP_OPT_X_SASL_AUTHCID)
  {
    int res;
    char *sasl_authcid = NULL;

    res = ldap_get_option(conn_ld, LDAP_OPT_X_SASL_AUTHCID, &sasl_authcid);
    if (res == LDAP_OPT_SUCCESS) {
      if (sasl_authcid != NULL) {
        pr_trace_msg(trace_channel, 12,
          "LDAP SASL default authentication ID = %s (see SASL_AUTHCID in "
          "ldap.conf)", sasl_authcid);
        sasl->authentication_id = pstrdup(sasl_pool, sasl_authcid);
        ldap_memfree(sasl_authcid);

      } else {
        sasl->authentication_id = pstrdup(sasl_pool, "");
      }

    } else {
      pr_trace_msg(trace_channel, 3,
        "error retrieving LDAP_OPT_X_SASL_AUTHCID: %s", ldap_err2string(res));
    }
  }
#endif /* LDAP_OPT_X_SASL_AUTHCID */

#if defined(LDAP_OPT_X_SASL_AUTHZID)
  {
    int res;
    char *sasl_authzid = NULL;

    res = ldap_get_option(conn_ld, LDAP_OPT_X_SASL_AUTHZID, &sasl_authzid);
    if (res == LDAP_OPT_SUCCESS) {
      if (sasl_authzid != NULL) {
        pr_trace_msg(trace_channel, 12,
          "LDAP SASL default authorization ID = %s (see SASL_AUTHZID in "
          "ldap.conf)", sasl_authzid);
        sasl->authorization_id = pstrdup(sasl_pool, sasl_authzid);
        ldap_memfree(sasl_authzid);

      } else {
        sasl->authorization_id = pstrdup(sasl_pool, "");
      }

    } else {
      pr_trace_msg(trace_channel, 3,
        "error retrieving LDAP_OPT_X_SASL_AUTHZID: %s", ldap_err2string(res));
    }
  }
#endif /* LDAP_OPT_X_SASL_AUTHZID */

#if defined(LDAP_OPT_X_SASL_REALM)
  {
    int res;
    char *sasl_realm = NULL;

    res = ldap_get_option(conn_ld, LDAP_OPT_X_SASL_REALM, &sasl_realm);
    if (res == LDAP_OPT_SUCCESS) {
      if (sasl_realm != NULL) {
        pr_trace_msg(trace_channel, 12,
          "LDAP SASL default realm = %s (see SASL_REALM in ldap.conf)",
          sasl_realm);
        sasl->realm = pstrdup(sasl_pool, sasl_realm);
        ldap_memfree(sasl_realm);

      } else {
        sasl->realm = pstrdup(sasl_pool, "");
      }

    } else {
      pr_trace_msg(trace_channel, 3,
        "error retrieving LDAP_OPT_X_SASL_REALM: %s", ldap_err2string(res));
    }
  }
#endif /* LDAP_OPT_X_SASL_REALM */

  return sasl;
}

static void sasl_info_get_authcid_from_dn(struct sasl_info *sasl,
    const char *dn_text) {
  register unsigned int i;
  LDAPDN dn;
  int flags = LDAP_DN_FORMAT_LDAPV3, res;

  /* LDAPDN is actually LDAPRDN *, a pointer to a list of LDAPRDN.  Which
   * makes sense, since a DN is a list of RDNs.  And each LDAPRDN is actually
   * LDAPAVA *, a pointer to a list of LDAPAVA (Attribute Value Assertions).
   */
  res = ldap_str2dn(dn_text, &dn, flags);
  if (res != LDAP_SUCCESS) {
    pr_trace_msg(trace_channel, 3,
      "error parsing DN '%s': %s", dn_text, ldap_err2string(res));
    return;
  }

  for (i = 0; dn[i]; i++) {
    LDAPRDN rdn;
    char *rdn_text = NULL;

    rdn = dn[i];
    res = ldap_rdn2str(rdn, &rdn_text, flags);
    if (res == LDAP_SUCCESS) {
      if (strncasecmp(rdn_text, "CN=", 3) == 0) {
        sasl->authentication_id = pstrdup(sasl->pool, rdn_text + 3);
      }
      ldap_memfree(rdn_text);

      if (sasl->authentication_id != NULL) {
        break;
      }

    } else {
      pr_trace_msg(trace_channel, 3,
        "error converting RDN to text: %s", ldap_err2string(res));
    }
  }

  ldap_dnfree(dn);
}

/* server_info functions. */
static struct server_info *server_info_create(pool *p) {
  struct server_info *info;

  info = pcalloc(p, sizeof(struct server_info));
  info->ssl_verify = -1;

  return info;
}

static void server_info_get_ssl_defaults(struct server_info *info) {
#if defined(LDAP_OPT_X_TLS)
  int res, ssl_verify;
  char *ssl_val = NULL;

  /* Fill in the SSL defaults, if not explicitly configured. */

# if defined(LDAP_OPT_X_TLS_CACERTFILE)
  if (info->ssl_ca_file == NULL) {
    ssl_val = NULL;
    res = ldap_get_option(NULL, LDAP_OPT_X_TLS_CACERTFILE, &ssl_val);
    if (res == LDAP_OPT_SUCCESS) {
      if (ssl_val != NULL) {
        pr_trace_msg(trace_channel, 17,
          "using default 'ssl-ca' value: %s", ssl_val);
        info->ssl_ca_file = ldap_strdup(ssl_val);
      }
    }
  }
# endif /* LDAP_OPT_X_TLS_CACERTFILE */

# if defined(LDAP_OPT_X_TLS_CERTFILE)
  if (info->ssl_cert_file == NULL) {
    ssl_val = NULL;
    res = ldap_get_option(NULL, LDAP_OPT_X_TLS_CERTFILE, &ssl_val);
    if (res == LDAP_OPT_SUCCESS) {
      if (ssl_val != NULL) {
        pr_trace_msg(trace_channel, 17,
          "using default 'ssl-cert' value: %s", ssl_val);
        info->ssl_cert_file = ldap_strdup(ssl_val);
      }
    }
  }
# endif /* LDAP_OPT_X_TLS_CERTFILE */

# if defined(LDAP_OPT_X_TLS_KEYFILE)
  if (info->ssl_key_file == NULL) {
    ssl_val = NULL;
    res = ldap_get_option(NULL, LDAP_OPT_X_TLS_KEYFILE, &ssl_val);
    if (res == LDAP_OPT_SUCCESS) {
      if (ssl_val != NULL) {
        pr_trace_msg(trace_channel, 17,
          "using default 'ssl-key' value: %s", ssl_val);
        info->ssl_key_file = ldap_strdup(ssl_val);
      }
    }
  }
# endif /* LDAP_OPT_X_TLS_KEYFILE */

# if defined(LDAP_OPT_X_TLS_CIPHER_SUITE)
  if (info->ssl_ciphers == NULL) {
    ssl_val = NULL;
    res = ldap_get_option(NULL, LDAP_OPT_X_TLS_CIPHER_SUITE, &ssl_val);
    if (res == LDAP_OPT_SUCCESS) {
      if (ssl_val != NULL) {
        pr_trace_msg(trace_channel, 17,
          "using default 'ssl-ciphers' value: %s", ssl_val);
        info->ssl_ciphers = ldap_strdup(ssl_val);
      }
    }
  }
# endif /* LDAP_OPT_X_TLS_CIPHER_SUITE */

# if defined(LDAP_OPT_X_TLS_REQUIRE_CERT)
  if (info->ssl_verify == -1) {
    res = ldap_get_option(NULL, LDAP_OPT_X_TLS_REQUIRE_CERT, &ssl_verify);
    if (res == LDAP_OPT_SUCCESS) {
      ssl_val = NULL;

      switch (ssl_verify) {
#  if defined(LDAP_OPT_X_TLS_NEVER)
        case LDAP_OPT_X_TLS_NEVER:
          ssl_val = "never";
          break;
#  endif /* LDAP_OPT_X_TLS_NEVER */

#  if defined(LDAP_OPT_X_TLS_HARD)
        case LDAP_OPT_X_TLS_HARD:
          ssl_val = "hard";
          break;
#  endif /* LDAP_OPT_X_TLS_HARD */

#  if defined(LDAP_OPT_X_TLS_DEMAND)
        case LDAP_OPT_X_TLS_DEMAND:
          ssl_val = "demand";
          break;
#  endif /* LDAP_OPT_X_TLS_DEMAND */

#  if defined(LDAP_OPT_X_TLS_ALLOW)
        case LDAP_OPT_X_TLS_ALLOW:
          ssl_val = "allow";
          break;
#  endif /* LDAP_OPT_X_TLS_ALLOW */

#  if defined(LDAP_OPT_X_TLS_TRY)
        case LDAP_OPT_X_TLS_TRY:
          ssl_val = "try";
          break;
#  endif /* LDAP_OPT_X_TLS_TRY */

        default:
          ssl_val = NULL;
      }

      pr_trace_msg(trace_channel, 17,
        "using default 'ssl-verify' value: %s", ssl_val ? ssl_val : "UNKNOWN");

      info->ssl_verify = ssl_verify;

      if (ssl_val != NULL) {
        info->ssl_verify_text = ldap_strdup(ssl_val);
      }
    }
  }
# endif /* LDAP_OPT_X_TLS_REQUIRE_CERT */
#endif /* LDAP_OPT_X_TLS */
}

/* Free up any library-allocated memory. */
static void server_info_free(struct server_info *info) {
  if (info->url_desc != NULL) {
    ldap_free_urldesc(info->url_desc);
    info->url_desc = NULL;
  }

  if (info->url_text != NULL) {
    ldap_memfree(info->url_text);
    info->url_text = NULL;
  }

  if (info->ssl_ca_file != NULL) {
    ldap_memfree((char *) info->ssl_ca_file);
    info->ssl_ca_file = NULL;
  }

  if (info->ssl_cert_file != NULL) {
    ldap_memfree((char *) info->ssl_cert_file);
    info->ssl_cert_file = NULL;
  }

  if (info->ssl_key_file != NULL) {
    ldap_memfree((char *) info->ssl_key_file);
    info->ssl_key_file = NULL;
  }

  if (info->ssl_ciphers != NULL) {
    ldap_memfree((char *) info->ssl_ciphers);
    info->ssl_ciphers = NULL;
  }

  info->ssl_verify = -1;

  if (info->ssl_verify_text != NULL) {
    ldap_memfree((char *) info->ssl_verify_text);
    info->ssl_verify_text = NULL;
  }
}

static void server_infos_free(void) {
  server_rec *s;

  for (s = (server_rec *) server_list->xas_list; s; s = s->next) {
    register unsigned int i;
    config_rec *c;
    array_header *infos;

    c = find_config(s->conf, CONF_PARAM, "LDAPServer", FALSE);
    if (c == NULL) {
      continue;
    }

    pr_signals_handle();

    infos = c->argv[0];
    for (i = 0; i < infos->nelts; i++) {
      struct server_info *info;

      info = ((struct server_info **) infos->elts)[i];
      server_info_free(info);
    }
  }
}

/* Configuration handlers
 */

/* usage: LDAPLog path|"none" */
MODRET set_ldaplog(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  (void) add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

MODRET set_ldapprotoversion(cmd_rec *cmd) {
  int i = 0;
  config_rec *c;
  char *version;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  version = cmd->argv[1];
  while (version[i]) {
    if (!PR_ISDIGIT((int) version[i])) {
      CONF_ERROR(cmd, "LDAPProtocolVersion: argument must be numeric!");
    }

    ++i;
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = atoi(version);

  return PR_HANDLED(cmd);
}

/* usage: LDAPServer info [ssl-cert:<path>] [ssl-key:<path>] [ssl-ca:<path>]
 *          [ssl-ciphers:ciphers] [ssl-verify:verify]
 */
MODRET set_ldapserver(cmd_rec *cmd) {
  register unsigned int i;
  struct server_info *info = NULL;
  array_header *infos, *items;
  config_rec *c;

  if (cmd->argc < 2) {
    CONF_ERROR(cmd, "wrong number of parameters");
  }
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  c = add_config_param(cmd->argv[0], 1, NULL);
  infos = make_array(c->pool, cmd->argc - 1, sizeof(struct server_info *));
  c->argv[0] = infos;

  /* For historical reasons (and leaky abstractions from underlying LDAP
   * libraries), the LDAPServer directive supports having multiple hosts/URLs
   * being passed as a single string, quoted and separated by whitespace.
   * That is not really necessary, but we need to handle such items as well.
   *
   * Order matters, since the ordering of these hosts/URLs dictates the
   * order in which they are tried when attempting to connect.
   */

  items = make_array(cmd->tmp_pool, cmd->argc - 1, sizeof(char *));
  for (i = 1; i < cmd->argc; i++) {
    char *item;

    item = cmd->argv[i];

    /* Split non-URL arguments on whitespace and insert them as separate
     * servers.
     */
    while (*item) {
      size_t len;

      pr_signals_handle();

      len = strcspn(item, " \f\n\r\t\v");
      *((char **) push_array(items)) = pstrndup(cmd->tmp_pool, item, len);

      item += len;
      while (PR_ISSPACE(*item)) {
        ++item;
      }
    }
  }

  for (i = 0; i < items->nelts; i++) {
    char *item;

    item = ((char **) items->elts)[i];

    /* Is this an SSL option?  If so, it needs to be applied to the
     * previously defined LDAP URL/host.  Otherwise, it's a misconfiguration.
     */
    if (strncmp(item, "ssl-ca:", 7) == 0) {
      char *path;

      if (info == NULL) {
        CONF_ERROR(cmd, "wrong order of parameters");
      }

      path = item;

      /* Advance past the "ssl-ca:" prefix. */
      path += 7;

      if (file_exists2(cmd->tmp_pool, path) != TRUE) {
        CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "SSL CA file '", path, "': ",
          strerror(ENOENT), NULL));
      }

      info->ssl_ca_file = ldap_strdup(path);
      continue;

    } else if (strncmp(item, "ssl-cert:", 9) == 0) {
      char *path;

      if (info == NULL) {
        CONF_ERROR(cmd, "wrong order of parameters");
      }

      path = item;

      /* Advance past the "ssl-cert:" prefix. */
      path += 9;

      if (file_exists2(cmd->tmp_pool, path) != TRUE) {
        CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "SSL certificate file '",
          path, "': ", strerror(ENOENT), NULL));
      }

      info->ssl_cert_file = ldap_strdup(path);
      continue;

    } else if (strncmp(item, "ssl-key:", 8) == 0) {
      char *path;

      if (info == NULL) {
        CONF_ERROR(cmd, "wrong order of parameters");
      }

      path = item;

      /* Advance past the "ssl-key:" prefix. */
      path += 8;

      if (file_exists2(cmd->tmp_pool, path) != TRUE) {
        CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "SSL certificate key file '",
          path, "': ", strerror(ENOENT), NULL));
      }

      info->ssl_key_file = ldap_strdup(path);
      continue;

    } else if (strncmp(item, "ssl-ciphers:", 12) == 0) {
      char *ciphers;

      if (info == NULL) {
        CONF_ERROR(cmd, "wrong order of parameters");
      }

      ciphers = item;

      /* Advance past the "ssl-ciphers:" prefix. */
      ciphers += 12;

      info->ssl_ciphers = ldap_strdup(ciphers);
      continue;

    } else if (strncmp(item, "ssl-verify:", 11) == 0) {
      int b, ssl_verify = -1;
      char *verify_text;

      if (info == NULL) {
        CONF_ERROR(cmd, "wrong order of parameters");
      }

      verify_text = item;

      /* Advance past the "ssl-verify:" prefix. */
      verify_text += 11;

      /* For convenience, we accept the usual on/off values, AND the
       * LDAP specific names, for those who think they want the finer
       * control.
       */

      b = pr_str_is_boolean(verify_text);
      if (b < 0) {
#if defined(LDAP_OPT_X_TLS_ALLOW)
        if (strcasecmp(verify_text, "allow") == 0) {
          ssl_verify = LDAP_OPT_X_TLS_ALLOW;
#else
        if (FALSE) {
#endif /* LDAP_OPT_X_TLS_ALLOW */

#if defined(LDAP_OPT_X_TLS_DEMAND)
        } else if (strcasecmp(verify_text, "demand") == 0) {
          ssl_verify = LDAP_OPT_X_TLS_DEMAND;
#endif /* LDAP_OPT_X_TLS_DEMAND */

#if defined(LDAP_OPT_X_TLS_NEVER)
        } else if (strcasecmp(verify_text, "never") == 0) {
          ssl_verify = LDAP_OPT_X_TLS_NEVER;
#endif /* LDAP_OPT_X_TLS_NEVER */

#if defined(LDAP_OPT_X_TLS_TRY)
        } else if (strcasecmp(verify_text, "try") == 0) {
          ssl_verify = LDAP_OPT_X_TLS_TRY;
#endif /* LDAP_OPT_X_TLS_TRY */

        } else {
          CONF_ERROR(cmd, pstrcat(cmd->tmp_pool,
            "unknown/unsupported 'ssl-verify' value: ", verify_text, NULL));
        }

      } else {
        if (b == TRUE) {
#if defined(LDAP_OPT_X_TLS_DEMAND)
          ssl_verify = LDAP_OPT_X_TLS_DEMAND;
#endif /* LDAP_OPT_X_TLS_DEMAND */
          verify_text = "demand";

        } else {
#if defined(LDAP_OPT_X_TLS_NEVER)
          ssl_verify = LDAP_OPT_X_TLS_NEVER;
#endif /* LDAP_OPT_X_TLS_NEVER */
          verify_text = "never";
        }
      }

      info->ssl_verify = ssl_verify;
      info->ssl_verify_text = ldap_strdup(verify_text);
      continue;

    } else {
      info = server_info_create(c->pool);
    }

    info->info_text = pstrdup(c->pool, item);

    if (ldap_is_ldap_url(item)) {
      int res;
      LDAPURLDesc *url_desc;
      char *url_text;

      res = ldap_url_parse(item, &url_desc);
      if (res != LDAP_URL_SUCCESS) {
        CONF_ERROR(cmd, pstrcat(cmd->tmp_pool,
          "invalid LDAP URL '", item, "': ", ldap_err2string(res), NULL));
      }

      info->url_desc = url_desc;

      url_text = ldap_url_desc2str(url_desc);
      if (url_text != NULL) {
        pr_log_debug(DEBUG0, "%s: parsed URL '%s' as '%s'",
          (char *) cmd->argv[0], item, url_text);
        info->url_text = url_text;
      }

#ifdef HAS_LDAP_INITIALIZE
      if (strcasecmp(url_desc->lud_scheme, "ldap") != 0 &&
          strcasecmp(url_desc->lud_scheme, "ldaps") != 0) {
        CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "invalid scheme specified by URL '", item, "': valid schemes are 'ldap' or 'ldaps'", NULL));
      }
#else /* HAS_LDAP_INITIALIZE */
      if (strcasecmp(url_desc->lud_scheme, "ldap") != 0) {
        CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "unsupported scheme specified by URL '", item, "': valid scheme is only 'ldap'", NULL));
      }
#endif /* HAS_LDAP_INITIALIZE */

      if (url_desc->lud_dn != NULL &&
          strcmp(url_desc->lud_dn, "") != 0) {
        CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, item, ": a base DN may not be specified by an LDAPServer URL, only by LDAPUsers or LDAPGroups", NULL));
      }

      if (url_desc->lud_filter != NULL &&
         strcmp(url_desc->lud_filter, "") != 0) {
        CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, item, ": a search filter may not be specified by an LDAPServer URL, only by LDAPUsers or LDAPGroups", NULL));
      }

      if (url_desc->lud_scope == LDAP_SCOPE_BASE) {
        pr_log_debug(DEBUG0, MOD_LDAP_VERSION
          ": WARNING: '%s': LDAP URL search scopes default to 'base', "
          "not 'subtree', and may not be what you want (see LDAPSearchScope)",
          item);
      }

      /* Need to keep parsed host and port for pre-2000 ldap_init(3). */
      if (url_desc->lud_host != NULL) {
        info->host = pstrdup(c->pool, url_desc->lud_host);
      }

      if (url_desc->lud_port != 0) {
        info->port = url_desc->lud_port;
      }

    } else {
      char *ptr;

      /* The given item is just a hostname/IP address, maybe a port. */
      ptr = strchr(item, ':');
      if (ptr == NULL) {
        info->host = pstrdup(c->pool, item);
        info->port = LDAP_PORT;

      } else {
        int port;

        info->host = pstrndup(c->pool, item, ptr - item);

        port = atoi(ptr + 1);
        if (port == 0) {
          CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "unable to parse '", ptr + 1,
            "' as a port number for '", item, "'", NULL));
        }

        info->port = port;
        info->port_text = pstrdup(c->pool, ptr + 1);
      }
    }

    *((struct server_info **) push_array(infos)) = info;
  }

  return PR_HANDLED(cmd);
}

/* usage: LDAPUseSASL mech1 ... */
MODRET set_ldapusesasl(cmd_rec *cmd) {
#if defined(HAVE_SASL_SASL_H)
  register unsigned int i;
  config_rec *c;
  char *sasl_mechs = "";

  if (cmd->argc < 1) {
    CONF_ERROR(cmd, "wrong number of parameters");
  }

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  c = add_config_param(cmd->argv[0], 1, NULL);

  for (i = 1; i < cmd->argc; i++) {
    register unsigned int j;
    char *sasl_mech;

    /* Only allow known supported (and tested) mechanisms for now. */
    if (strcasecmp(cmd->argv[i], "ANONYMOUS") == 0 ||
        strcasecmp(cmd->argv[i], "CRAM-MD5") == 0 ||
        strcasecmp(cmd->argv[i], "DIGEST-MD5") == 0 ||
        strcasecmp(cmd->argv[i], "PLAIN") == 0 ||
        strcasecmp(cmd->argv[i], "LOGIN") == 0 ||
        strncasecmp(cmd->argv[i], "SCRAM-SHA-", 10) == 0) {
      sasl_mech = cmd->argv[i];

    } else {
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "unsupported SASL mechanism: ",
        cmd->argv[i], NULL));
    }

    /* Convert the mechanism name to all uppercase, per convention. */
    for (j = 0; j < strlen(sasl_mech); j++) {
      sasl_mech[j] = toupper(sasl_mech[j]);
    }

    sasl_mechs = pstrcat(c->pool, sasl_mechs, *sasl_mechs ? " " : "",
      sasl_mech, NULL);
  }

  c->argv[0] = sasl_mechs;
  return PR_HANDLED(cmd);
#else
  CONF_ERROR(cmd, "Your system libraries do not appear to support SASL (missing <sasl/sasl.h>)");
#endif /* HAVE_SASL_SASL_H */
}

/* usage: LDAPUseTLS on|off */
MODRET set_ldapusetls(cmd_rec *cmd) {
#if defined(LDAP_OPT_X_TLS)
  int use_tls;
  config_rec *c;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  use_tls = get_boolean(cmd, 1);
  if (use_tls == -1) {
    CONF_ERROR(cmd, "expected Boolean parameter");
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = use_tls;

  return PR_HANDLED(cmd);

#else /* LDAP_OPT_X_TLS */
  CONF_ERROR(cmd, "Your LDAP libraries do not appear to support TLS");
#endif /* LDAP_OPT_X_TLS */
}

MODRET set_ldapbinddn(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  add_config_param_str(cmd->argv[0], 2, cmd->argv[1], cmd->argv[2]);
  return PR_HANDLED(cmd);
}

MODRET set_ldapsearchscope(cmd_rec *cmd) {
  config_rec *c;
  const char *scope_name;
  int search_scope;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  scope_name = cmd->argv[1];

  if (strcasecmp(scope_name, "base") == 0) {
    search_scope = LDAP_SCOPE_BASE;

  } else if (strcasecmp(scope_name, "one") == 0 ||
             strcasecmp(scope_name, "onelevel") == 0) {
    search_scope = LDAP_SCOPE_ONELEVEL;

  } else if (strcasecmp(scope_name, "sub") == 0 ||
             strcasecmp(scope_name, "subtree") == 0) {
    search_scope = LDAP_SCOPE_SUBTREE;

  } else {
    CONF_ERROR(cmd, "search scope must be one of: base, onelevel, subtree");
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = palloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = search_scope;

  return PR_HANDLED(cmd);
}

MODRET set_ldapconnecttimeout(cmd_rec *cmd) {
  config_rec *c;
  int timeout;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if (pr_str_get_duration(cmd->argv[1], &timeout) < 0) {
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "error parsing timeout value '",
      cmd->argv[1], "': ", strerror(errno), NULL));
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = timeout;

  return PR_HANDLED(cmd);
}

MODRET set_ldapquerytimeout(cmd_rec *cmd) {
  config_rec *c;
  int timeout;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if (pr_str_get_duration(cmd->argv[1], &timeout) < 0) {
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "error parsing timeout value '",
      cmd->argv[1], "': ", strerror(errno), NULL));
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = timeout;

  return PR_HANDLED(cmd);
}

MODRET set_ldapaliasdereference(cmd_rec *cmd) {
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
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool,
      "expected a valid dereference (never, search, find, always): ",
      cmd->argv[1], NULL));
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = value;

  return PR_HANDLED(cmd);
}

MODRET set_ldapauthbinds(cmd_rec *cmd) {
  int b;
  config_rec *c;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  b = get_boolean(cmd, 1);
  if (b == -1) {
    CONF_ERROR(cmd, "expected Boolean parameter");
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = b;

  return PR_HANDLED(cmd);
}

MODRET set_ldapdefaultauthscheme(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

MODRET set_ldapattr(cmd_rec *cmd) {
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
      strcasecmp(cmd->argv[1], "ftpQuotaProfileDN") != 0) {
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool,
      ": unknown attribute name: ", cmd->argv[1], NULL));
  }

  add_config_param_str(cmd->argv[0], 2, cmd->argv[1], cmd->argv[2]);
  return PR_HANDLED(cmd);
}

/* usage: LDAPUsers base-dn [name-filter-template [uid-filter-template]] */
MODRET set_ldapusers(cmd_rec *cmd) {
  config_rec *c;

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if (get_boolean(cmd, 1) != -1) {
    CONF_ERROR(cmd, "first parameter must be the base DN, not on/off");
  }

  c = add_config_param(cmd->argv[0], cmd->argc - 1, NULL, NULL, NULL);
  c->argv[0] = pstrdup(c->pool, cmd->argv[1]);
  if (cmd->argc > 2) {
    c->argv[1] = pstrdup(c->pool, cmd->argv[2]);
  }
  if (cmd->argc > 3) {
    c->argv[2] = pstrdup(c->pool, cmd->argv[3]);
  }

  return PR_HANDLED(cmd);
}

MODRET set_ldapdefaultuid(cmd_rec *cmd) {
  config_rec *c;
  uid_t uid;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(uid_t));

  if (pr_str2uid(cmd->argv[1], &uid) < 0) {
    CONF_ERROR(cmd, "LDAPDefaultUID: UID argument must be numeric");
  }

  *((uid_t *) c->argv[0]) = uid;
  return PR_HANDLED(cmd);
}

MODRET set_ldapdefaultgid(cmd_rec *cmd) {
  config_rec *c;
  gid_t gid;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(gid_t));

  if (pr_str2gid(cmd->argv[1], &gid) < 0) {
    CONF_ERROR(cmd, "LDAPDefaultGID: GID argument must be numeric");
  }

  *((gid_t *) c->argv[0]) = gid;
  return PR_HANDLED(cmd);
}

MODRET set_ldapforcedefaultuid(cmd_rec *cmd) {
  int b;
  config_rec *c;

  CHECK_CONF(cmd,CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  b = get_boolean(cmd, 1);
  if (b == -1) {
    CONF_ERROR(cmd, "expected Boolean parameter");
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = b;

  return PR_HANDLED(cmd);
}

MODRET set_ldapforcedefaultgid(cmd_rec *cmd) {
  int b;
  config_rec *c;

  CHECK_CONF(cmd,CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  b = get_boolean(cmd, 1);
  if (b == -1) {
    CONF_ERROR(cmd, "expected Boolean parameter");
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = b;

  return PR_HANDLED(cmd);
}

MODRET set_ldapgenhdir(cmd_rec *cmd) {
  int b;
  config_rec *c;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  b = get_boolean(cmd, 1);
  if (b == -1) {
    CONF_ERROR(cmd, "expected Boolean parameter");
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = b;

  return PR_HANDLED(cmd);
}

MODRET set_ldapgenhdirprefix(cmd_rec *cmd) {
  char *prefix;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  prefix = cmd->argv[1];
  if (strlen(prefix) == 0) {
    CONF_ERROR(cmd, "must not be an empty string");
  }

  add_config_param_str(cmd->argv[0], 1, prefix);
  return PR_HANDLED(cmd);
}

MODRET set_ldapgenhdirprefixnouname(cmd_rec *cmd) {
  int b;
  config_rec *c;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  b = get_boolean(cmd, 1);
  if (b == -1) {
    CONF_ERROR(cmd, "expected Boolean parameter");
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = b;

  return PR_HANDLED(cmd);
}

MODRET set_ldapforcegenhdir(cmd_rec *cmd) {
  int b;
  config_rec *c;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  b = get_boolean(cmd, 1);
  if (b == -1) {
    CONF_ERROR(cmd, "expected Boolean parameter");
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = b;

  return PR_HANDLED(cmd);
}

MODRET set_ldapgrouplookups(cmd_rec *cmd) {
  config_rec *c;

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if (get_boolean(cmd, 1) != -1) {
    CONF_ERROR(cmd, "first parameter must be the base DN, not on/off.");
  }

  c = add_config_param(cmd->argv[0], cmd->argc - 1, NULL);
  c->argv[0] = pstrdup(c->pool, cmd->argv[1]);
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

MODRET set_ldapdefaultquota(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

/* Event listeners
 */

#if defined(PR_SHARED_MODULE)
static void ldap_mod_unload_ev(const void *event_data, void *user_data) {
  if (strcmp("mod_ldap.c", (const char *) event_data) != 0) {
    return;
  }

  pr_event_unregister(&ldap_module, NULL, NULL);
  server_infos_free();
}
#endif /* PR_SHARED_MODULE */

static void ldap_postparse_ev(const void *event_data, void *user_data) {
  server_rec *s = NULL;

  /* Check the configured LDAPServer directives.  Specifically, if the URL
   * syntax has been used, we look for any LDAPSearchScope directive and
   * warn that they are ignored.  Similarly, if there are "ldaps" URLs,
   * we warn that any LDAPUseTLS directive will be ignored.  Otherwise,
   * cases where LDAP URLs have not been used, we construct them.
   */
  for (s = (server_rec *) server_list->xas_list; s; s = s->next) {
    register unsigned int i;
    config_rec *c;
    array_header *infos;
    int search_scope = -1, use_tls = -1;

    pr_signals_handle();

    c = find_config(s->conf, CONF_PARAM, "LDAPSearchScope", FALSE);
    if (c != NULL) {
      search_scope = *((int *) c->argv[0]);

    } else {
      /* Per documentation, the default search scope is "subtree". */
      search_scope = LDAP_SCOPE_SUBTREE;
    }

#if defined(LDAP_OPT_X_TLS)
    c = find_config(s->conf, CONF_PARAM, "LDAPUseTLS", FALSE);
    if (c != NULL) {
      use_tls = *((int *) c->argv[0]);
    }
#endif /* LDAP_OPT_X_TLS */

    c = find_config(s->conf, CONF_PARAM, "LDAPServer", FALSE);
    if (c == NULL) {
      continue;
    }

    infos = c->argv[0];

    for (i = 0; i < infos->nelts; i++) {
      struct server_info *info;

      info = ((struct server_info **) infos->elts)[i];

      if (info->url_desc != NULL) {
        if (info->url_desc->lud_scope != search_scope) {
          /* When LDAP URLs are used, the search scope is part of the URL
           * syntax, which takes precedence over LDAPSearchScope.
           */
          pr_log_debug(DEBUG2, MOD_LDAP_VERSION
            ": ignoring configured LDAPSearchScope for LDAP URL '%s'",
            info->info_text);
        }

        /* If the scheme is "ldaps", AND LDAPUseTLS has been explicitly
         * configured, warn that it will be ignored.
         */
        if (use_tls != -1) {
          if (strcasecmp(info->url_desc->lud_scheme, "ldaps") == 0) {
            pr_log_debug(DEBUG2, MOD_LDAP_VERSION
              ": ignoring configured LDAPUseTLS for explicit LDAPS URL '%s'",
              info->info_text);
          } else {
            info->use_starttls = use_tls;
          }
        }

      } else {
        int res;
        char *url, *url_text;
        LDAPURLDesc *url_desc;

        /* Construct an LDAP URL, and parse it. */

        url = pstrcat(c->pool, "ldap://", info->host, NULL);
        if (info->port_text != NULL) {
          url = pstrcat(c->pool, url, ":", info->port_text, NULL);
        }

        switch (search_scope) {
          case LDAP_SCOPE_SUBTREE:
            url = pstrcat(c->pool, url, "/??sub", NULL);
            break;

          case LDAP_SCOPE_BASE:
            url = pstrcat(c->pool, url, "/??base", NULL);
            break;

          case LDAP_SCOPE_ONELEVEL:
            url = pstrcat(c->pool, url, "/??one", NULL);
            break;

          default:
            break;
        }

        res = ldap_url_parse(url, &url_desc);
        if (res != LDAP_URL_SUCCESS) {
          /* This should only happen as a result of a coding bug. */
          pr_log_pri(PR_LOG_NOTICE, MOD_LDAP_VERSION
            ": invalid LDAP URL '%s': %s", url, ldap_err2string(res));
          pr_session_disconnect(&ldap_module, PR_SESS_DISCONNECT_BAD_CONFIG,
            NULL);
        }

        info->url_desc = url_desc;
        info->port = url_desc->lud_port;

        url_text = ldap_url_desc2str(url_desc);
        if (url_text != NULL) {
          pr_log_debug(DEBUG0, "%s: parsed URL '%s' as '%s'", c->name, url,
            url_text);
          info->url_text = url_text;
        }

        if (use_tls != -1) {
          info->use_starttls = use_tls;
        }
      }

      server_info_get_ssl_defaults(info);
    }
  }
}

static void ldap_sess_reinit_ev(const void *event_data, void *user_data) {
  int res;

  /* A HOST command changed the main_server pointer; reinitialize ourselves. */

  pr_event_unregister(&ldap_module, "core.session-reinit", ldap_sess_reinit_ev);

  /* Restore defaults. */
  (void) close(ldap_logfd);
  ldap_logfd = -1;
  ldap_protocol_version = 3;
  ldap_servers = NULL;
  ldap_dn = NULL;
  ldap_dnpass = NULL;
  ldap_dnpasslen = 0;
  ldap_search_scope = LDAP_SCOPE_SUBTREE;
  ldap_sasl_mechs = NULL;
  ldap_connecttimeout = 0;
  ldap_querytimeout = 0;
  ldap_dereference = LDAP_DEREF_NEVER;
  ldap_authbinds = TRUE;
  ldap_defaultauthscheme = "crypt";
  ldap_attr_uid = "uid";
  ldap_attr_uidnumber = "uidNumber";
  ldap_attr_gidnumber = "gidNumber";
  ldap_attr_homedirectory = "homeDirectory";
  ldap_attr_userpassword = "userPassword";
  ldap_attr_loginshell = "loginShell";
  ldap_attr_cn = "cn";
  ldap_attr_memberuid = "memberUid";
  ldap_attr_ftpquota = "ftpQuota";
  ldap_attr_ftpquota_profiledn = "ftpQuotaProfileDN";
  ldap_do_users = FALSE;
  ldap_user_basedn = NULL;
  ldap_user_name_filter = NULL;
  ldap_user_uid_filter = NULL;
  ldap_do_groups = FALSE;
  ldap_group_name_filter = NULL;
  ldap_group_gid_filter = NULL;
  ldap_group_member_filter = NULL;
  ldap_default_quota = NULL;
  ldap_defaultuid = (uid_t) -1;
  ldap_defaultgid = (gid_t) -1;
  ldap_forcedefaultuid = FALSE;
  ldap_forcedefaultgid = FALSE;
  ldap_forcegenhdir = FALSE;
  ldap_genhdir = FALSE;
  ldap_genhdir_prefix = NULL;
  ldap_genhdir_prefix_nouname = FALSE;

  curr_server_info = NULL;
  curr_server_index = 0;

  destroy_pool(ldap_pool);
  ldap_pool = NULL;

  res = ldap_sess_init();
  if (res < 0) {
    pr_session_disconnect(&ldap_module, PR_SESS_DISCONNECT_SESSION_INIT_FAILED,
      NULL);
  }
}

static void ldap_shutdown_ev(const void *event_data, void *user_data) {
  server_infos_free();
}

/* Initialization routines
 */

static int ldap_mod_init(void) {
  pr_log_debug(DEBUG2, MOD_LDAP_VERSION
    ": compiled using LDAP vendor '%s', LDAP API version %lu",
    LDAP_VENDOR_NAME, (unsigned long) LDAP_API_VERSION);

#if defined(LDAP_OPT_API_INFO)
  {
    int res;
    LDAPAPIInfo api_info;

    api_info.ldapai_info_version = LDAP_API_INFO_VERSION;
    res = ldap_get_option(NULL, LDAP_OPT_API_INFO, &api_info);
    if (res == LDAP_OPT_SUCCESS) {
      pool *tmp_pool;
      char *feats = "";

      tmp_pool = make_sub_pool(permanent_pool);

      if (api_info.ldapai_extensions != NULL) {
        register unsigned int i;

        for (i = 0; api_info.ldapai_extensions[i]; i++) {
          feats = pstrcat(tmp_pool, feats, i != 0 ? ", " : "",
            api_info.ldapai_extensions[i], NULL);
          ldap_memfree(api_info.ldapai_extensions[i]);
        }

        ldap_memfree(api_info.ldapai_extensions);
      }

      pr_log_debug(DEBUG10, MOD_LDAP_VERSION
        " linked with LDAP vendor '%s' (LDAP API version %d, "
        "vendor version %d), features: %s", api_info.ldapai_vendor_name,
        api_info.ldapai_api_version, api_info.ldapai_vendor_version,
        feats);
      ldap_memfree(api_info.ldapai_vendor_name);
      destroy_pool(tmp_pool);

    } else {
      pr_trace_msg(trace_channel, 3,
        "error retrieving LDAP_OPT_API_INFO: %s", ldap_err2string(res));
    }
  }
#endif /* LDAP_OPT_API_INFO */

#if defined(LDAP_OPT_X_TLS_PACKAGE)
  {
    int res;
    char *tls_package = NULL;

    res = ldap_get_option(NULL, LDAP_OPT_X_TLS_PACKAGE, &tls_package);
    if (res == LDAP_OPT_SUCCESS) {
      pr_log_debug(DEBUG10, MOD_LDAP_VERSION
        ": LDAP TLS package = %s", tls_package);

    } else {
      pr_trace_msg(trace_channel, 3,
        "error retrieving LDAP_OPT_X_TLS_PACKAGE: %s", ldap_err2string(res));
    }
  }
#endif /* LDAP_OPT_X_TLS_PACKAGE */

#if defined(PR_SHARED_MODULE)
  pr_event_register(&ldap_module, "core.module-unload", ldap_mod_unload_ev,
    NULL);
#endif /* PR_SHARED_MODULE */
  pr_event_register(&ldap_module, "core.postparse", ldap_postparse_ev, NULL);
  pr_event_register(&ldap_module, "core.shutdown", ldap_shutdown_ev, NULL);

  return 0;
}

static int ldap_sess_init(void) {
  config_rec *c;
  void *ptr;

  pr_event_register(&ldap_module, "core.session-reinit", ldap_sess_reinit_ev,
    NULL);

  ldap_pool = make_sub_pool(session.pool);
  pr_pool_tag(ldap_pool, MOD_LDAP_VERSION);

  c = find_config(main_server->conf, CONF_PARAM, "LDAPLog", FALSE);
  if (c != NULL) {
    char *path;

    path = c->argv[0];

    if (strncasecmp(path, "none", 5) != 0) {
      int res, xerrno = 0;

      pr_signals_block();
      PRIVS_ROOT
      res = pr_log_openfile(path, &ldap_logfd, PR_LOG_SYSTEM_MODE);
      xerrno = errno;
      PRIVS_RELINQUISH
      pr_signals_unblock();

      if (res < 0) {
        if (res == -1) {
          pr_log_pri(PR_LOG_NOTICE, MOD_LDAP_VERSION
            ": notice: unable to open LDAPLog '%s': %s", path,
            strerror(xerrno));

        } else if (res == PR_LOG_WRITABLE_DIR) {
          pr_log_pri(PR_LOG_WARNING, MOD_LDAP_VERSION
            ": notice: unable to open LDAPPLog '%s': parent directory is "
            "world-writable", path);

        } else if (res == PR_LOG_SYMLINK) {
          pr_log_pri(PR_LOG_WARNING, MOD_LDAP_VERSION
            ": notice: unable to open LDAPLog '%s': cannot log to a symlink",
            path);
        }
      }
    }
  }

  ptr = get_param_ptr(main_server->conf, "LDAPProtocolVersion", FALSE);
  if (ptr != NULL) {
    ldap_protocol_version = *((int *) ptr);
  }

  /* Allow for multiple LDAPServer directives. */
  c = find_config(main_server->conf, CONF_PARAM, "LDAPServer", FALSE);
  while (c != NULL) {
    pr_signals_handle();

    if (ldap_servers != NULL) {
      array_cat(ldap_servers, c->argv[0]);

    } else {
      ldap_servers = c->argv[0];
    }

    c = find_config_next(c, c->next, CONF_PARAM, "LDAPServer", FALSE);
  }

  if (ldap_servers == NULL) {
    pr_log_pri(PR_LOG_NOTICE, MOD_LDAP_VERSION
      ": no LDAPServer configured, using LDAP library defaults");
  }

  c = find_config(main_server->conf, CONF_PARAM, "LDAPBindDN", FALSE);
  if (c != NULL) {
    ldap_dn = pstrdup(ldap_pool, c->argv[0]);
    ldap_dnpass = pstrdup(ldap_pool, c->argv[1]);
    ldap_dnpasslen = strlen(ldap_dnpass);
  }

  c = find_config(main_server->conf, CONF_PARAM, "LDAPSearchScope", FALSE);
  if (c != NULL) {
    ldap_search_scope = *((int *) c->argv[0]);
  }

  ptr = get_param_ptr(main_server->conf, "LDAPConnectTimeout", FALSE);
  if (ptr != NULL) {
    ldap_connecttimeout = *((int *) ptr);

    if (ldap_connecttimeout > 0) {
      ldap_connecttimeout_tv.tv_sec = ldap_connecttimeout;
    }
  }

  ptr = get_param_ptr(main_server->conf, "LDAPQueryTimeout", FALSE);
  if (ptr != NULL) {
    ldap_querytimeout = *((int *) ptr);
  }

  ptr = get_param_ptr(main_server->conf, "LDAPAliasDereference", FALSE);
  if (ptr != NULL) {
    ldap_dereference = *((int *) ptr);
  }

  ptr = get_param_ptr(main_server->conf, "LDAPAuthBinds", FALSE);
  if (ptr != NULL) {
    ldap_authbinds = *((int *) ptr);
  }

  ptr = get_param_ptr(main_server->conf, "LDAPDefaultAuthScheme", FALSE);
  if (ptr != NULL) {
    ldap_defaultauthscheme = (char *) ptr;
  }

  /* Look up any attr redefinitions (LDAPAttr) before using those
   * variables, such as when generating the default search filters.
   */
  c = find_config(main_server->conf, CONF_PARAM, "LDAPAttr", FALSE);
  if (c != NULL) {
    do {
      if (strcasecmp(c->argv[0], "uid") == 0) {
        ldap_attr_uid = pstrdup(ldap_pool, c->argv[1]);

      } else if (strcasecmp(c->argv[0], "uidNumber") == 0) {
        ldap_attr_uidnumber = pstrdup(ldap_pool, c->argv[1]);

      } else if (strcasecmp(c->argv[0], "gidNumber") == 0) {
        ldap_attr_gidnumber = pstrdup(ldap_pool, c->argv[1]);

      } else if (strcasecmp(c->argv[0], "homeDirectory") == 0) {
        ldap_attr_homedirectory = pstrdup(ldap_pool, c->argv[1]);

      } else if (strcasecmp(c->argv[0], "userPassword") == 0) {
        ldap_attr_userpassword = pstrdup(ldap_pool, c->argv[1]);

      } else if (strcasecmp(c->argv[0], "loginShell") == 0) {
        ldap_attr_loginshell = pstrdup(ldap_pool, c->argv[1]);

      } else if (strcasecmp(c->argv[0], "cn") == 0) {
        ldap_attr_cn = pstrdup(ldap_pool, c->argv[1]);

      } else if (strcasecmp(c->argv[0], "memberUid") == 0) {
        ldap_attr_memberuid = pstrdup(ldap_pool, c->argv[1]);

      } else if (strcasecmp(c->argv[0], "ftpQuota") == 0) {
        ldap_attr_ftpquota = pstrdup(ldap_pool, c->argv[1]);

      } else if (strcasecmp(c->argv[0], "ftpQuotaProfileDN") == 0) {
        ldap_attr_ftpquota_profiledn = pstrdup(ldap_pool, c->argv[1]);
      }

    } while ((c = find_config_next(c, c->next, CONF_PARAM, "LDAPAttr", FALSE)));
  }

  c = find_config(main_server->conf, CONF_PARAM, "LDAPUsers", FALSE);
  if (c != NULL) {
    ldap_do_users = TRUE;
    ldap_user_basedn = pstrdup(ldap_pool, c->argv[0]);

    if (c->argc > 1) {
      ldap_user_name_filter = pstrdup(ldap_pool, c->argv[1]);

    } else {
      ldap_user_name_filter = pstrcat(ldap_pool,
        "(&(", ldap_attr_uid, "=%v)(objectclass=posixAccount))", NULL);
    }

    if (c->argc > 2) {
      ldap_user_uid_filter = pstrdup(ldap_pool, c->argv[2]);

    } else {
      ldap_user_uid_filter = pstrcat(ldap_pool,
        "(&(", ldap_attr_uidnumber, "=%v)(objectclass=posixAccount))", NULL);
    }
  }

  ptr = get_param_ptr(main_server->conf, "LDAPDefaultUID", FALSE);
  if (ptr != NULL) {
    ldap_defaultuid = *((uid_t *) ptr);
  }

  ptr = get_param_ptr(main_server->conf, "LDAPDefaultGID", FALSE);
  if (ptr != NULL) {
    ldap_defaultgid = *((gid_t *) ptr);
  }

  ldap_default_quota = get_param_ptr(main_server->conf, "LDAPDefaultQuota",
    FALSE);

  ptr = get_param_ptr(main_server->conf, "LDAPForceDefaultUID", FALSE);
  if (ptr != NULL) {
    ldap_forcedefaultuid = *((int *) ptr);
  }

  ptr = get_param_ptr(main_server->conf, "LDAPForceDefaultGID", FALSE);
  if (ptr != NULL) {
    ldap_forcedefaultgid = *((int *) ptr);
  }

  ptr = get_param_ptr(main_server->conf, "LDAPForceGeneratedHomedir", FALSE);
  if (ptr != NULL) {
    ldap_forcegenhdir = *((int *) ptr);
  }

  ptr = get_param_ptr(main_server->conf, "LDAPGenerateHomedir", FALSE);
  if (ptr != NULL) {
    ldap_genhdir = *((int *) ptr);
  }

  ldap_genhdir_prefix = get_param_ptr(main_server->conf,
    "LDAPGenerateHomedirPrefix", FALSE);

  ptr = get_param_ptr(main_server->conf, "LDAPGenerateHomedirPrefixNoUsername",
    FALSE);
  if (ptr != NULL) {
    ldap_genhdir_prefix_nouname = *((int *) ptr);
  }

  c = find_config(main_server->conf, CONF_PARAM, "LDAPGroups", FALSE);
  if (c != NULL) {
    ldap_do_groups = TRUE;
    ldap_gid_basedn = pstrdup(ldap_pool, c->argv[0]);

    if (c->argc > 1) {
      ldap_group_name_filter = pstrdup(ldap_pool, c->argv[1]);

    } else {
      ldap_group_name_filter = pstrcat(ldap_pool,
        "(&(", ldap_attr_cn, "=%v)(objectclass=posixGroup))", NULL);
    }

    if (c->argc > 2) {
      ldap_group_gid_filter = pstrdup(ldap_pool, c->argv[2]);

    } else {
      ldap_group_gid_filter = pstrcat(ldap_pool,
        "(&(", ldap_attr_gidnumber, "=%v)(objectclass=posixGroup))", NULL);
    }

    if (c->argc > 3) {
      ldap_group_member_filter = pstrdup(ldap_pool, c->argv[3]);

    } else {
      ldap_group_member_filter = pstrcat(ldap_pool,
        "(&(", ldap_attr_memberuid, "=%v)(objectclass=posixGroup))", NULL);
    }
  }

  c = find_config(main_server->conf, CONF_PARAM, "LDAPUseSASL", FALSE);
  if (c != NULL) {
    ldap_sasl_mechs = c->argv[0];
  }

#if defined(LBER_OPT_LOG_PRINT_FN)
  /* If trace logging is enabled for the 'ldap.library' channel, direct
   * libldap (via liblber) to log to our trace logging.
   */
  if (pr_trace_get_level(libtrace_channel) >= 1) {
    int res, trace_level;

    res = ber_set_option(NULL, LBER_OPT_LOG_PRINT_FN, ldap_tracelog_cb);
    if (res != LBER_OPT_SUCCESS) {
      (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
        "error setting trace logging function: %s", strerror(EINVAL));
    }

    /* Debug levels:
     *  Trace (1)
     *  Packets (2)
     *  Arguments (4)
     *  Filters (32)
     *  Access control (128)
     *
     * See include/ldap_log.h in the OpenLDAP source.
     */
    trace_level = pr_trace_get_level(libtrace_channel);
    res = ldap_set_option(NULL, LDAP_OPT_DEBUG_LEVEL, &trace_level);
    if (res != LDAP_OPT_SUCCESS) {
      (void) pr_log_writefile(ldap_logfd, MOD_LDAP_VERSION,
        "error setting LDAP debug level %d: %s", trace_level,
        strerror(EINVAL));
    }
  }
#endif /* LBER_OPT_LOG_PRINT_FN */

  if (ldap_do_users == FALSE) {
    pr_log_pri(PR_LOG_WARNING, MOD_LDAP_VERSION
      ": LDAPUsers not configured, skipping LDAP-based user authentication");
  }

  if (ldap_do_groups == FALSE) {
    pr_log_pri(PR_LOG_NOTICE, MOD_LDAP_VERSION
      ": LDAPGroups not configured, skipping LDAP-based group memberships");
  }

  return 0;
}

/* Module API tables
 */

static conftable ldap_conftab[] = {
  { "LDAPAliasDereference",	set_ldapaliasdereference,	NULL },
  { "LDAPAttr",			set_ldapattr,			NULL },
  { "LDAPAuthBinds",		set_ldapauthbinds,		NULL },
  { "LDAPBindDN",		set_ldapbinddn,			NULL },
  { "LDAPConnectTimeout",	set_ldapconnecttimeout,		NULL },
  { "LDAPDefaultAuthScheme",	set_ldapdefaultauthscheme,	NULL },
  { "LDAPDefaultGID",		set_ldapdefaultgid,		NULL },
  { "LDAPDefaultQuota",		set_ldapdefaultquota,		NULL },
  { "LDAPDefaultUID",		set_ldapdefaultuid,		NULL },
  { "LDAPForceDefaultGID",	set_ldapforcedefaultgid,	NULL },
  { "LDAPForceDefaultUID",	set_ldapforcedefaultuid,	NULL },
  { "LDAPForceGeneratedHomedir",set_ldapforcegenhdir,		NULL },
  { "LDAPGenerateHomedir",	set_ldapgenhdir,		NULL },
  { "LDAPGenerateHomedirPrefix",set_ldapgenhdirprefix,		NULL },
  { "LDAPGenerateHomedirPrefixNoUsername",
				set_ldapgenhdirprefixnouname,	NULL },
  { "LDAPGroups",		set_ldapgrouplookups,		NULL },
  { "LDAPLog",			set_ldaplog,			NULL },
  { "LDAPProtocolVersion",	set_ldapprotoversion,		NULL },
  { "LDAPQueryTimeout",		set_ldapquerytimeout,		NULL },
  { "LDAPSearchScope",		set_ldapsearchscope,		NULL },
  { "LDAPServer",		set_ldapserver,			NULL },
  { "LDAPUsers",		set_ldapusers,			NULL },
  { "LDAPUseSASL",		set_ldapusesasl,		NULL },
  { "LDAPUseTLS",		set_ldapusetls,			NULL },

  { NULL, NULL, NULL },
};

static cmdtable ldap_cmdtab[] = {
  { HOOK, "ldap_quota_lookup",		G_NONE, handle_ldap_quota_lookup, FALSE, FALSE},
  { HOOK, "ldap_ssh_publickey_lookup",	G_NONE, handle_ldap_ssh_pubkey_lookup, FALSE, FALSE},

  { 0, NULL}
};

static authtable ldap_authtab[] = {
  { 0, "setpwent",	ldap_auth_setpwent },
  { 0, "endpwent",	ldap_auth_endpwent },
  { 0, "setgrent",	ldap_auth_setpwent },
  { 0, "endgrent",	ldap_auth_endpwent },
  { 0, "getpwnam",	ldap_auth_getpwnam },
  { 0, "getpwuid",	ldap_auth_getpwuid },
  { 0, "getgrnam",	ldap_auth_getgrnam },
  { 0, "getgrgid",	ldap_auth_getgrgid },
  { 0, "getgroups",	ldap_auth_getgroups },
  { 0, "auth",		ldap_auth_auth },
  { 0, "check",		ldap_auth_check },
  { 0, "uid2name",	ldap_auth_uid2name },
  { 0, "gid2name",	ldap_auth_gid2name },
  { 0, "name2uid",	ldap_auth_name2uid },
  { 0, "name2gid",	ldap_auth_name2gid },

  { 0, NULL }
};

module ldap_module = {
  /* Always NULL */
  NULL, NULL,

  /* Module API version */
  0x20,

  /* Module name */
  "ldap",

  /* Module configuration handler table */
  ldap_conftab,

  /* Module command handler table */
  ldap_cmdtab,

  /* Module authentication handler table */
  ldap_authtab,

  /* Module initialization */
  ldap_mod_init,

  /* Session initialization */
  ldap_sess_init,

  /* Module version */
  MOD_LDAP_VERSION
};
