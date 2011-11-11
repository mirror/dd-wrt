/*
 * ProFTPD: mod_sql_passwd -- Various SQL password handlers
 * Copyright (c) 2009-2011 TJ Saunders
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
 * As a special exemption, TJ Saunders and other respective copyright holders
 * give permission to link this program with OpenSSL, and distribute the
 * resulting executable, without including the source code for OpenSSL in
 * the source distribution.
 *
 * $Id: mod_sql_passwd.c,v 1.16 2011/05/23 20:56:40 castaglia Exp $
 */

#include "conf.h"
#include "privs.h"
#include "mod_sql.h"

#define MOD_SQL_PASSWD_VERSION		"mod_sql_passwd/0.4"

/* Make sure the version of proftpd is as necessary. */
#if PROFTPD_VERSION_NUMBER < 0x0001030302 
# error "ProFTPD 1.3.3rc2 or later required"
#endif

#if !defined(HAVE_OPENSSL) && !defined(PR_USE_OPENSSL)
# error "OpenSSL support required (--enable-openssl)"
#else
# include <openssl/evp.h>
#endif

module sql_passwd_module;

static int sql_passwd_engine = FALSE;

#define SQL_PASSWD_USE_BASE64		1
#define SQL_PASSWD_USE_HEX_LC		2
#define SQL_PASSWD_USE_HEX_UC		3
static unsigned int sql_passwd_encoding = SQL_PASSWD_USE_HEX_LC;

static char *sql_passwd_salt = NULL;
static size_t sql_passwd_salt_len = 0;

#define SQL_PASSWD_SALT_FL_APPEND	0x0001
#define SQL_PASSWD_SALT_FL_PREPEND	0x0002
static unsigned long sql_passwd_salt_flags = SQL_PASSWD_SALT_FL_APPEND;

#define SQL_PASSWD_OPT_HASH_SALT		0x0001
#define SQL_PASSWD_OPT_ENCODE_SALT		0x0002
#define SQL_PASSWD_OPT_HASH_PASSWORD		0x0004
#define SQL_PASSWD_OPT_ENCODE_PASSWORD		0x0008

static unsigned long sql_passwd_opts = 0UL;

static unsigned int sql_passwd_nrounds = 1;

static const char *trace_channel = "sql_passwd";

static cmd_rec *sql_passwd_cmd_create(pool *parent_pool, int argc, ...) {
  pool *cmd_pool = NULL;
  cmd_rec *cmd = NULL;
  register unsigned int i = 0;
  va_list argp;
 
  cmd_pool = make_sub_pool(parent_pool);
  cmd = (cmd_rec *) pcalloc(cmd_pool, sizeof(cmd_rec));
  cmd->pool = cmd_pool;
 
  cmd->argc = argc;
  cmd->argv = (char **) pcalloc(cmd->pool, argc * sizeof(char *));

  /* Hmmm... */
  cmd->tmp_pool = cmd->pool;

  va_start(argp, argc);
  for (i = 0; i < argc; i++)
    cmd->argv[i] = va_arg(argp, char *);
  va_end(argp);

  return cmd;
}

static char *sql_passwd_get_str(pool *p, char *str) {
  cmdtable *cmdtab;
  cmd_rec *cmd;
  modret_t *res;

  if (strlen(str) == 0)
    return str;

  /* Find the cmdtable for the sql_escapestr command. */
  cmdtab = pr_stash_get_symbol(PR_SYM_HOOK, "sql_escapestr", NULL, NULL);
  if (cmdtab == NULL) {
    pr_log_debug(DEBUG2, MOD_SQL_PASSWD_VERSION
      ": unable to find SQL hook symbol 'sql_escapestr'");
    return str;
  }

  cmd = sql_passwd_cmd_create(p, 1, pr_str_strip(p, str));

  /* Call the handler. */
  res = pr_module_call(cmdtab->m, cmdtab->handler, cmd);

  /* Check the results. */
  if (MODRET_ISDECLINED(res) ||
      MODRET_ISERROR(res)) {
    pr_log_debug(DEBUG0, MOD_SQL_PASSWD_VERSION
      ": error executing 'sql_escapestring'");
    return str;
  }

  return res->data;
}

static char *sql_passwd_encode(pool *p, unsigned char *data, size_t data_len) {
  EVP_ENCODE_CTX base64_ctxt;
  char *buf;

  /* According to RATS, the output buffer for EVP_EncodeBlock() needs to be
   * 4/3 the size of the input buffer (which is usually EVP_MAX_MD_SIZE).
   * Let's make it easy, and use an output buffer that's twice the size of the
   * input buffer.
   */
  buf = pcalloc(p, (2 * data_len) + 1);

  switch (sql_passwd_encoding) {
    case SQL_PASSWD_USE_BASE64:
      EVP_EncodeInit(&base64_ctxt);
      EVP_EncodeBlock((unsigned char *) buf, data, (int) data_len);
      break;

    case SQL_PASSWD_USE_HEX_LC: {
      register unsigned int i;

      for (i = 0; i < data_len; i++) {
        sprintf((char *) &(buf[i*2]), "%02x", data[i]);
      }

      break;
    }

    case SQL_PASSWD_USE_HEX_UC: {
      register unsigned int i;

      for (i = 0; i < data_len; i++) {
        sprintf((char *) &(buf[i*2]), "%02X", data[i]);
      }

      break;
    }

    default:
      errno = EINVAL;
      return NULL;
  }

  return buf;
}

/* This may look a little weird, with the data, prefix, and suffix arguments.
 * But they are used to handle the case where we are hashing data with
 * a salt (either as a prefix or as a suffix), and where we are hashing
 * already hashed data.
 */
static unsigned char *sql_passwd_hash(pool *p, const EVP_MD *md,
    unsigned char *data, size_t data_len,
    unsigned char *prefix, size_t prefix_len,
    unsigned char *suffix, size_t suffix_len,
    unsigned int *hash_len) {

  EVP_MD_CTX md_ctx;
  unsigned char *hash;

  hash = palloc(p, EVP_MAX_MD_SIZE);

  EVP_DigestInit(&md_ctx, md);

  if (prefix != NULL) {
    EVP_DigestUpdate(&md_ctx, prefix, prefix_len);
  }

  EVP_DigestUpdate(&md_ctx, data, data_len);

  if (suffix != NULL) {
    EVP_DigestUpdate(&md_ctx, suffix, suffix_len);
  }

  EVP_DigestFinal(&md_ctx, hash, hash_len);
  return hash;
}

static modret_t *sql_passwd_auth(cmd_rec *cmd, const char *plaintext,
    const char *ciphertext, const char *digest) {
  const EVP_MD *md;
  unsigned char *hash = NULL, *data = NULL, *prefix = NULL, *suffix = NULL;
  size_t data_len = 0, prefix_len = 0, suffix_len = 0;
  unsigned int hash_len = 0;

  /* Temporary copy of the ciphertext string */
  char *copytext;
  const char *encodedtext;

  if (!sql_passwd_engine) {
    return PR_ERROR_INT(cmd, PR_AUTH_ERROR);
  }

  /* We need a copy of the ciphertext. */
  copytext = pstrdup(cmd->tmp_pool, ciphertext);

  OpenSSL_add_all_digests();

  md = EVP_get_digestbyname(digest);
  if (md == NULL) {
    sql_log(DEBUG_WARN, "no such digest '%s' supported", digest);
    return PR_ERROR_INT(cmd, PR_AUTH_BADPWD);
  }

  /* If a salt is configured, do we prepend the salt as a prefix (i.e. throw
   * it into the digest before the user-supplied password) or append it as a
   * suffix?
   */

  if (sql_passwd_salt_len > 0 &&
      (sql_passwd_salt_flags & SQL_PASSWD_SALT_FL_PREPEND)) {

    /* If we have salt data, add it to the mix. */

    if (!(sql_passwd_opts & SQL_PASSWD_OPT_HASH_SALT)) {
      prefix = (unsigned char *) sql_passwd_salt;
      prefix_len = sql_passwd_salt_len;

      pr_trace_msg(trace_channel, 9,
        "prepending %lu bytes of salt data", (unsigned long) prefix_len);

    } else {
      unsigned int salt_hashlen = 0;

      prefix = sql_passwd_hash(cmd->tmp_pool, md,
        (unsigned char *) sql_passwd_salt, sql_passwd_salt_len,
        NULL, 0, NULL, 0, &salt_hashlen);
      prefix_len = salt_hashlen;

      if (sql_passwd_opts & SQL_PASSWD_OPT_ENCODE_SALT) {
        prefix = (unsigned char *) sql_passwd_encode(cmd->tmp_pool,
          (unsigned char *) prefix, prefix_len);
        prefix_len = strlen((char *) prefix);
      }

      pr_trace_msg(trace_channel, 9,
        "prepending %lu bytes of %s-hashed salt data (%s)",
        (unsigned long) prefix_len, digest, prefix);
    }
  }

  if (!(sql_passwd_opts & SQL_PASSWD_OPT_HASH_PASSWORD)) {
    data = (unsigned char *) plaintext;
    data_len = strlen(plaintext);

  } else {
    /* Note: We will only honor a HashEncodePassword option IFF there is
     * also salt data present.  Otherwise, it is equivalent to another
     * round of processing, which defeats the principle of least surprise.
     */
    if (sql_passwd_salt_len == 0 &&
        (sql_passwd_opts & SQL_PASSWD_OPT_HASH_PASSWORD) &&
        (sql_passwd_opts & SQL_PASSWD_OPT_ENCODE_PASSWORD)) {
      pr_trace_msg(trace_channel, 4, "%s",
        "no salt present, ignoring HashEncodePassword SQLPasswordOption");
      data = (unsigned char *) plaintext;
      data_len = strlen(plaintext);

    } else {
      unsigned int salt_hashlen = 0;

      data = sql_passwd_hash(cmd->tmp_pool, md,
        (unsigned char *) plaintext, strlen(plaintext),
        NULL, 0, NULL, 0, &salt_hashlen);
      data_len = salt_hashlen;

      if (sql_passwd_opts & SQL_PASSWD_OPT_ENCODE_PASSWORD) {
        data = (unsigned char *) sql_passwd_encode(cmd->tmp_pool,
          (unsigned char *) data, data_len);
        data_len = strlen((char *) data);
      }
    }
  }

  if (sql_passwd_salt_len > 0 &&
      (sql_passwd_salt_flags & SQL_PASSWD_SALT_FL_APPEND)) {
    /* If we have salt data, add it to the mix. */

    if (!(sql_passwd_opts & SQL_PASSWD_OPT_HASH_SALT)) {
      suffix = (unsigned char *) sql_passwd_salt;
      suffix_len = sql_passwd_salt_len;

      pr_trace_msg(trace_channel, 9,
        "appending %lu bytes of salt data", (unsigned long) suffix_len);

    } else {
      unsigned int salt_hashlen = 0;

      suffix = sql_passwd_hash(cmd->tmp_pool, md,
        (unsigned char *) sql_passwd_salt, sql_passwd_salt_len,
        NULL, 0, NULL, 0, &salt_hashlen);
      suffix_len = salt_hashlen;

      if (sql_passwd_opts & SQL_PASSWD_OPT_ENCODE_SALT) {
        suffix = (unsigned char *) sql_passwd_encode(cmd->tmp_pool,
          (unsigned char *) suffix, suffix_len);
        suffix_len = strlen((char *) suffix);
      }

      pr_trace_msg(trace_channel, 9, 
        "appending %lu bytes of %s-hashed salt data",
        (unsigned long) suffix_len, digest);
    }
  }

  hash = sql_passwd_hash(cmd->tmp_pool, md, data, data_len, prefix, prefix_len,
    suffix, suffix_len, &hash_len);

  encodedtext = sql_passwd_encode(cmd->tmp_pool, hash, hash_len);
  if (encodedtext == NULL) {
    sql_log(DEBUG_WARN, "unsupported SQLPasswordEncoding configured");
    return PR_ERROR_INT(cmd, PR_AUTH_ERROR);
  }

  /* The case of nrounds == 1 is a special case, as that is when the salt
   * data is processed.  Any additional rounds are simply hashing and
   * encoding the resulting data, over and over.
   */
  if (sql_passwd_nrounds > 1) {
    register unsigned int i;
    unsigned int nrounds = sql_passwd_nrounds - 1;

    pr_trace_msg(trace_channel, 9, 
      "transforming the data for another %u %s", nrounds,
      nrounds != 1 ? "rounds" : "round");

    for (i = 0; i < nrounds; i++) {
      pr_signals_handle();

      hash = sql_passwd_hash(cmd->tmp_pool, md, (unsigned char *) encodedtext,
        strlen(encodedtext), NULL, 0, NULL, 0, &hash_len);
      encodedtext = sql_passwd_encode(cmd->tmp_pool, hash, hash_len);

      pr_trace_msg(trace_channel, 15, "data after round %u: '%s'", i + 1,
        encodedtext);
    }
  }

  if (strcmp((char *) encodedtext, copytext) == 0) {
    return PR_HANDLED(cmd);

  } else {
    pr_trace_msg(trace_channel, 9, "expected '%s', got '%s'", copytext,
      encodedtext);

    pr_log_debug(DEBUG9, MOD_SQL_PASSWD_VERSION ": expected '%s', got '%s'",
      copytext, encodedtext);
  }

  return PR_ERROR_INT(cmd, PR_AUTH_BADPWD);
}

static modret_t *sql_passwd_md5(cmd_rec *cmd, const char *plaintext,
    const char *ciphertext) {
  return sql_passwd_auth(cmd, plaintext, ciphertext, "md5");
}

static modret_t *sql_passwd_sha1(cmd_rec *cmd, const char *plaintext,
    const char *ciphertext) {
  return sql_passwd_auth(cmd, plaintext, ciphertext, "sha1");
}

static modret_t *sql_passwd_sha256(cmd_rec *cmd, const char *plaintext,
    const char *ciphertext) {
  return sql_passwd_auth(cmd, plaintext, ciphertext, "sha256");
}

static modret_t *sql_passwd_sha512(cmd_rec *cmd, const char *plaintext,
    const char *ciphertext) {
  return sql_passwd_auth(cmd, plaintext, ciphertext, "sha512");
}

/* Event handlers
 */

#if defined(PR_SHARED_MODULE)
static void sql_passwd_mod_unload_ev(const void *event_data, void *user_data) {
  if (strcmp("mod_sql_passwd.c", (const char *) event_data) == 0) {
    sql_unregister_authtype("md5");
    sql_unregister_authtype("sha1");
    sql_unregister_authtype("sha256");
    sql_unregister_authtype("sha512");

    pr_event_unregister(&sql_passwd_module, NULL, NULL);
  }
}
#endif /* PR_SHARED_MODULE */

/* Command handlers
 */

MODRET sql_passwd_pre_pass(cmd_rec *cmd) {
  config_rec *c;

  if (!sql_passwd_engine) {
    return PR_DECLINED(cmd);
  }

  c = find_config(main_server->conf, CONF_PARAM, "SQLPasswordRounds", FALSE);
  if (c) {
    sql_passwd_nrounds = *((unsigned int *) c->argv[0]);
  }

  c = find_config(main_server->conf, CONF_PARAM, "SQLPasswordUserSalt", FALSE);
  if (c) {
    char *key;
    unsigned long salt_flags;

    key = c->argv[0];
    salt_flags = *((unsigned long *) c->argv[1]);

    if (strcasecmp(key, "name") == 0) {
      char *user;

      user = pr_table_get(session.notes, "mod_auth.orig-user", NULL);
      sql_passwd_salt = user;
      sql_passwd_salt_len = strlen(user);

    } else if (strncasecmp(key, "sql:/", 5) == 0) {
      char *named_query, *ptr, *user, **values;
      cmdtable *sql_cmdtab;
      cmd_rec *sql_cmd;
      modret_t *sql_res;
      array_header *sql_data;

      ptr = key + 5; 
      named_query = pstrcat(cmd->tmp_pool, "SQLNamedQuery_", ptr, NULL);

      c = find_config(main_server->conf, CONF_PARAM, named_query, FALSE);
      if (c == NULL) {
        pr_log_debug(DEBUG3, MOD_SQL_PASSWD_VERSION
          ": unable to resolve SQLNamedQuery '%s'", ptr);
        return PR_DECLINED(cmd);
      }

      sql_cmdtab = pr_stash_get_symbol(PR_SYM_HOOK, "sql_lookup", NULL, NULL);
      if (sql_cmdtab == NULL) {
        pr_log_debug(DEBUG3, MOD_SQL_PASSWD_VERSION
          ": unable to find SQL hook symbol 'sql_lookup'");
        return PR_DECLINED(cmd);
      }

      user = pr_table_get(session.notes, "mod_auth.orig-user", NULL);

      sql_cmd = sql_passwd_cmd_create(cmd->tmp_pool, 3, "sql_lookup", ptr,
        sql_passwd_get_str(cmd->tmp_pool, user));

      /* Call the handler. */
      sql_res = pr_module_call(sql_cmdtab->m, sql_cmdtab->handler, sql_cmd);
      if (sql_res == NULL ||
          MODRET_ISERROR(sql_res)) {
        pr_log_debug(DEBUG0, MOD_SQL_PASSWD_VERSION
          ": error processing SQLNamedQuery '%s'", ptr);
        return PR_DECLINED(cmd);
      }

      sql_data = (array_header *) sql_res->data;

      if (sql_data->nelts != 1) {
        pr_log_debug(DEBUG0, MOD_SQL_PASSWD_VERSION
          ": SQLNamedQuery '%s' returned wrong number of rows (%d)", ptr,
          sql_data->nelts);
        return PR_DECLINED(cmd);
      }

      values = sql_data->elts;
      sql_passwd_salt = pstrdup(session.pool, values[0]);
      sql_passwd_salt_len = strlen(values[0]);
      
    } else {
      return PR_DECLINED(cmd);
    }

    sql_passwd_salt_flags = salt_flags;
  }

  return PR_DECLINED(cmd);
}

/* Configuration handlers
 */

/* usage: SQLPasswordEncoding "base64"|"hex"|"HEX" */
MODRET set_sqlpasswdencoding(cmd_rec *cmd) {
  unsigned int encoding;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if (strcmp(cmd->argv[1], "base64") == 0) {
    encoding = SQL_PASSWD_USE_BASE64;

  } else if (strcmp(cmd->argv[1], "hex") == 0) {
    encoding = SQL_PASSWD_USE_HEX_LC;

  } else if (strcmp(cmd->argv[1], "HEX") == 0) {
    encoding = SQL_PASSWD_USE_HEX_UC;

  } else {
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "unsupported encoding '",
      cmd->argv[1], "' configured", NULL));
  }
 
  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned int));
  *((unsigned int *) c->argv[0]) = encoding;

  return PR_HANDLED(cmd);
}

/* usage: SQLPasswordEngine on|off */
MODRET set_sqlpasswdengine(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = bool;

  return PR_HANDLED(cmd);
}

/* usage: SQLPasswordOptions opt1 ... optN */
MODRET set_sqlpasswdoptions(cmd_rec *cmd) {
  config_rec *c;
  unsigned long opts = 0UL;
  register unsigned int i;

  if (cmd->argc < 2) {
    CONF_ERROR(cmd, "wrong number of parameters");
  }

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  for (i = 1; i < cmd->argc; i++) {
    if (strcasecmp(cmd->argv[i], "HashPassword") == 0) {
      opts |= SQL_PASSWD_OPT_HASH_PASSWORD;

    } else if (strcasecmp(cmd->argv[i], "HashSalt") == 0) {
      opts |= SQL_PASSWD_OPT_HASH_SALT;

    } else if (strcasecmp(cmd->argv[i], "HashEncodePassword") == 0) {
      opts |= SQL_PASSWD_OPT_HASH_PASSWORD;
      opts |= SQL_PASSWD_OPT_ENCODE_PASSWORD;

    } else if (strcasecmp(cmd->argv[i], "HashEncodeSalt") == 0) {
      opts |= SQL_PASSWD_OPT_HASH_SALT;
      opts |= SQL_PASSWD_OPT_ENCODE_SALT;

    } else {
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, ": unknown SQLPasswordOption '",
        cmd->argv[i], "'", NULL));
    }
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = palloc(c->pool, sizeof(unsigned long));
  *((unsigned long *) c->argv[0]) = opts;

  return PR_HANDLED(cmd);
}

/* usage: SQLPasswordSaltFile path|"none" [flags] */
MODRET set_sqlpasswdsaltfile(cmd_rec *cmd) {
  config_rec *c;
  register unsigned int i;
  unsigned long flags = SQL_PASSWD_SALT_FL_APPEND;

  if (cmd->argc < 2) {
    CONF_ERROR(cmd, "wrong number of parameters");
  }

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  for (i = 2; i < cmd->argc; i++) {
    if (strcasecmp(cmd->argv[i], "Append") == 0) {
      flags &= ~SQL_PASSWD_SALT_FL_PREPEND;
      flags |= SQL_PASSWD_SALT_FL_APPEND;
 
    } else if (strcasecmp(cmd->argv[i], "Prepend") == 0) {
      flags &= ~SQL_PASSWD_SALT_FL_APPEND;
      flags |= SQL_PASSWD_SALT_FL_PREPEND;

    } else {
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "unknown salt flag '",
        cmd->argv[i], "'", NULL));
    }
  }

  c = add_config_param(cmd->argv[0], 2, NULL, NULL);
  c->argv[0] = pstrdup(c->pool, cmd->argv[1]);
  c->argv[1] = palloc(c->pool, sizeof(unsigned long));
  *((unsigned long *) c->argv[1]) = flags;

  return PR_HANDLED(cmd);
}

/* usage: SQLPasswordRounds count */
MODRET set_sqlpasswdrounds(cmd_rec *cmd) {
  config_rec *c;
  int nrounds;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  nrounds = atoi(cmd->argv[1]);
  if (nrounds < 1) {
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "insufficient number of rounds (",
      cmd->argv[1], ")", NULL));
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = palloc(c->pool, sizeof(unsigned int));
  *((unsigned int *) c->argv[0]) = nrounds;

  return PR_HANDLED(cmd);
}

/* usage: SQLPasswordUserSalt "name"|"sql:/named-query" [flags]
 */
MODRET set_sqlpasswdusersalt(cmd_rec *cmd) {
  config_rec *c;
  register unsigned int i;
  unsigned long flags = SQL_PASSWD_SALT_FL_APPEND;

  if (cmd->argc < 2) {
    CONF_ERROR(cmd, "wrong number of parameters");
  }

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if (strcasecmp(cmd->argv[1], "name") != 0 &&
      strcasecmp(cmd->argv[1], "uid") != 0 &&
      strncasecmp(cmd->argv[1], "sql:/", 5) != 0) {
    CONF_ERROR(cmd, "badly formatted parameter");
  }

  for (i = 2; i < cmd->argc; i++) {
    if (strcasecmp(cmd->argv[i], "Append") == 0) {
      flags &= ~SQL_PASSWD_SALT_FL_PREPEND;
      flags |= SQL_PASSWD_SALT_FL_APPEND;
 
    } else if (strcasecmp(cmd->argv[i], "Prepend") == 0) {
      flags &= ~SQL_PASSWD_SALT_FL_APPEND;
      flags |= SQL_PASSWD_SALT_FL_PREPEND;

    } else {
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "unknown salt flag '",
        cmd->argv[i], "'", NULL));
    }
  }

  c = add_config_param(cmd->argv[0], 2, NULL, NULL);
  c->argv[0] = pstrdup(c->pool, cmd->argv[1]);
  c->argv[1] = palloc(c->pool, sizeof(unsigned long));
  *((unsigned long *) c->argv[1]) = flags;

  return PR_HANDLED(cmd);
}

/* Initialization routines
 */

static int sql_passwd_init(void) {

#if defined(PR_SHARED_MODULE)
  pr_event_register(&sql_passwd_module, "core.module-unload",
    sql_passwd_mod_unload_ev, NULL);
#endif /* PR_SHARED_MODULE */

  if (sql_register_authtype("md5", sql_passwd_md5) < 0) {
    pr_log_pri(PR_LOG_WARNING, MOD_SQL_PASSWD_VERSION
      ": unable to register 'md5' SQLAuthType handler: %s", strerror(errno));

  } else {
    pr_log_debug(DEBUG6, MOD_SQL_PASSWD_VERSION
      ": registered 'md5' SQLAuthType handler");
  }

  if (sql_register_authtype("sha1", sql_passwd_sha1) < 0) {
    pr_log_pri(PR_LOG_WARNING, MOD_SQL_PASSWD_VERSION
      ": unable to register 'sha1' SQLAuthType handler: %s", strerror(errno));

  } else {
    pr_log_debug(DEBUG6, MOD_SQL_PASSWD_VERSION
      ": registered 'sha1' SQLAuthType handler");
  }

  if (sql_register_authtype("sha256", sql_passwd_sha256) < 0) {
    pr_log_pri(PR_LOG_WARNING, MOD_SQL_PASSWD_VERSION
      ": unable to register 'sha256' SQLAuthType handler: %s", strerror(errno));

  } else {
    pr_log_debug(DEBUG6, MOD_SQL_PASSWD_VERSION
      ": registered 'sha256' SQLAuthType handler");
  }

  if (sql_register_authtype("sha512", sql_passwd_sha512) < 0) {
    pr_log_pri(PR_LOG_WARNING, MOD_SQL_PASSWD_VERSION
      ": unable to register 'sha512' SQLAuthType handler: %s", strerror(errno));

  } else {
    pr_log_debug(DEBUG6, MOD_SQL_PASSWD_VERSION
      ": registered 'sha512' SQLAuthType handler");
  }

  return 0;
}

static int sql_passwd_sess_init(void) {
  config_rec *c;

  c = find_config(main_server->conf, CONF_PARAM, "SQLPasswordEngine", FALSE);
  if (c) {
    sql_passwd_engine = *((int *) c->argv[0]);
  }

  c = find_config(main_server->conf, CONF_PARAM, "SQLPasswordEncoding", FALSE);
  if (c) {
    sql_passwd_encoding = *((unsigned int *) c->argv[0]);
  }

  c = find_config(main_server->conf, CONF_PARAM, "SQLPasswordOptions", FALSE);
  if (c) {
    sql_passwd_opts = *((unsigned long *) c->argv[0]);
  }

  c = find_config(main_server->conf, CONF_PARAM, "SQLPasswordSaltFile", FALSE);
  if (c) {
    char *path;
    unsigned long salt_flags;

    path = c->argv[0];
    salt_flags = *((unsigned long *) c->argv[1]);

    if (strcasecmp(path, "none") != 0) {
      int fd, xerrno = 0;;

      PRIVS_ROOT
      fd = open(path, O_RDONLY|O_NONBLOCK);
      if (fd < 0) {
        xerrno = errno;
      }
      PRIVS_RELINQUISH

      if (fd >= 0) {
        int flags;
        char buf[512];
        ssize_t nread;
  
        /* Set this descriptor for blocking. */
        flags = fcntl(fd, F_GETFL);
        if (fcntl(fd, F_SETFL, flags & (U32BITS^O_NONBLOCK)) < 0) {
          pr_log_debug(DEBUG3, MOD_SQL_PASSWD_VERSION
            ": error setting blocking mode on SQLPasswordSaltFile '%s': %s",
            path, strerror(errno));
        }
 
        nread = read(fd, buf, sizeof(buf));
        while (nread > 0) {
          pr_signals_handle();

          if (sql_passwd_salt == NULL) {

            /* If the very last byte in the buffer is a newline, trim it. */
            if (buf[nread-1] == '\n') {
              buf[nread-1] = '\0';
              nread--;
            }

            sql_passwd_salt_len = nread;
            sql_passwd_salt = palloc(session.pool, sql_passwd_salt_len);
            memcpy(sql_passwd_salt, buf, nread);

          } else {
            char *ptr, *tmp;

            /* Allocate a larger buffer for the salt. */
            ptr = tmp = palloc(session.pool, sql_passwd_salt_len + nread);
            memcpy(tmp, sql_passwd_salt, sql_passwd_salt_len);
            tmp += sql_passwd_salt_len;

            memcpy(tmp, buf, nread);
            sql_passwd_salt_len += nread;

            /* XXX Yes, this is a minor memory leak; we are overwriting the
             * previously allocated memory for the salt.  But it's per-session,
             * so it's not a great concern at this point.
             */
            sql_passwd_salt = ptr;
          }

          nread = read(fd, buf, sizeof(buf));
        }

        if (nread < 0) {
          pr_log_debug(DEBUG1, MOD_SQL_PASSWD_VERSION
            ": error reading salt data from SQLPasswordSaltFile '%s': %s",
            path, strerror(errno));
          sql_passwd_salt = NULL;
        }

        (void) close(fd);

        /* If the very last byte in the buffer is a newline, trim it.  This
         * is to deal with cases where the SaltFile may have been written
         * with an editor (e.g. vi) which automatically adds a trailing newline.
         */
        if (sql_passwd_salt[sql_passwd_salt_len-1] == '\n') {
          sql_passwd_salt[sql_passwd_salt_len-1] = '\0';
          sql_passwd_salt_len--;
        }

        sql_passwd_salt_flags = salt_flags;

      } else {
        pr_log_debug(DEBUG1, MOD_SQL_PASSWD_VERSION
          ": unable to read SQLPasswordSaltFile '%s': %s", path,
          strerror(xerrno));
      }
    }
  }

  return 0;
}

/* Module API tables
 */

static conftable sql_passwd_conftab[] = {
  { "SQLPasswordEncoding",	set_sqlpasswdencoding,	NULL },
  { "SQLPasswordEngine",	set_sqlpasswdengine,	NULL },
  { "SQLPasswordOptions",	set_sqlpasswdoptions,	NULL },
  { "SQLPasswordSaltFile",	set_sqlpasswdsaltfile,	NULL },
  { "SQLPasswordRounds",	set_sqlpasswdrounds,	NULL },
  { "SQLPasswordUserSalt",	set_sqlpasswdusersalt,	NULL },

  { NULL, NULL, NULL }
};

static cmdtable sql_passwd_cmdtab[] = {
  { PRE_CMD,	C_PASS, G_NONE,	sql_passwd_pre_pass,	FALSE,	FALSE },

  { 0, NULL }
};

module sql_passwd_module = {

  /* Always NULL */
  NULL, NULL,

  /* Module API version */
  0x20,

  /* Module name */
  "sql_passwd",

  /* Module configuration directive table */
  sql_passwd_conftab,

  /* Module command handler table */
  sql_passwd_cmdtab,

  /* Module auth handler table */
  NULL,

  /* Module initialization */
  sql_passwd_init,

  /* Session initialization */
  sql_passwd_sess_init,

  /* Module version */
  MOD_SQL_PASSWD_VERSION
};

