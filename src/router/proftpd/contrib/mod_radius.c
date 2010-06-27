/*
 * ProFTPD: mod_radius -- a module for RADIUS authentication and accounting
 *
 * Copyright (c) 2001-2010 TJ Saunders
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
 * As a special exemption, TJ Saunders gives permission to link this program
 * with OpenSSL, and distribute the resulting executable, without including
 * the source code for OpenSSL in the source distribution.
 *
 * This is mod_radius, contrib software for proftpd 1.2 and above.
 * For more information contact TJ Saunders <tj@castaglia.org>.
 *
 * This module is based in part on code in Alan DeKok's (aland@freeradius.org)
 * mod_auth_radius for Apache, in part on the FreeRADIUS project's code.
 *
 * $Id: mod_radius.c,v 1.56 2010/02/01 18:40:33 castaglia Exp $
 */

#define MOD_RADIUS_VERSION "mod_radius/0.9.1"

#include "conf.h"
#include "privs.h"

/* RADIUS information */

/* From RFC2865, RFC2866 */
#define RADIUS_AUTH_PORT	1812
#define RADIUS_ACCT_PORT	1813

#define RADIUS_PASSWD_LEN	16
#define RADIUS_VECTOR_LEN	16

/* From RFC2138 */
#define RADIUS_STRING_LEN	254

/* RADIUS attribute structures
 */
typedef struct {
  unsigned char type;
  unsigned char length;
  unsigned char data[1];
} radius_attrib_t;

/* RADIUS packet header
 */
typedef struct {
  unsigned char code;
  unsigned char id;
  unsigned short length;
  unsigned char digest[RADIUS_VECTOR_LEN];
  unsigned char data[2];

  char _pad[PR_TUNABLE_BUFFER_SIZE];
} radius_packet_t;

#define RADIUS_HEADER_LEN	20

/* RADIUS ID Definitions (see RFC2865)
 */
#define RADIUS_AUTH_REQUEST		1
#define RADIUS_AUTH_ACCEPT		2
#define RADIUS_AUTH_REJECT		3
#define RADIUS_ACCT_REQUEST		4
#define RADIUS_ACCT_RESPONSE		5
#define RADIUS_ACCT_STATUS		6
#define RADIUS_AUTH_CHALLENGE		11

/* RADIUS Attribute Definitions (see RFC2865)
 */
#define RADIUS_USER_NAME		1
#define RADIUS_PASSWORD			2
#define RADIUS_NAS_IP_ADDRESS		4
#define RADIUS_NAS_PORT			5
#define RADIUS_SERVICE_TYPE		6
#define RADIUS_OLD_PASSWORD		17
#define RADIUS_REPLY_MESSAGE		18
#define RADIUS_STATE			24
#define RADIUS_VENDOR_SPECIFIC		26
#define RADIUS_SESSION_TIMEOUT		27
#define RADIUS_IDLE_TIMEOUT		28
#define RADIUS_CALLING_STATION_ID	31
#define RADIUS_NAS_IDENTIFIER		32
#define RADIUS_ACCT_STATUS_TYPE		40
#define RADIUS_ACCT_INPUT_OCTETS	42
#define RADIUS_ACCT_OUTPUT_OCTETS	43
#define RADIUS_ACCT_SESSION_ID		44
#define RADIUS_ACCT_AUTHENTIC		45
#define RADIUS_ACCT_SESSION_TIME	46
#define RADIUS_ACCT_TERMINATE_CAUSE	49
#define RADIUS_NAS_PORT_TYPE		61

/* RADIUS service types
 */
#define RADIUS_SVC_LOGIN		1
#define RADIUS_SVC_AUTHENTICATE_ONLY	8

/* RADIUS status types
 */
#define RADIUS_ACCT_STATUS_START	1
#define RADIUS_ACCT_STATUS_STOP		2
#define RADIUS_ACCT_STATUS_ALIVE	3

/* RADIUS NAS port types
 */
#define RADIUS_NAS_PORT_TYPE_VIRTUAL	5

/* RADIUS authentication types
 */
#define RADIUS_AUTH_NONE		0
#define RADIUS_AUTH_RADIUS		1
#define RADIUS_AUTH_LOCAL		2

/* The RFC says 4096 octets max, and most packets are less than 256.
 * However, this number is just larger than the maximum MTU of just
 * most types of networks, except maybe for gigabit ethernet.
 */
#define RADIUS_PACKET_LEN		1600

/* Miscellaneous default values
 */
#define DEFAULT_RADIUS_TIMEOUT	30

typedef struct radius_server_obj {

  /* Next server in line */
  struct radius_server_obj *next;

  /* Memory pool for this object */
  pool *pool;

  /* RADIUS server IP address */
  pr_netaddr_t *addr;

  /* RADIUS server port */
  unsigned short port;

  /* RADIUS server shared secret */
  unsigned char *secret;

  /* How long to wait for RADIUS responses */
  unsigned int timeout;

} radius_server_t;

module radius_module;

static pool *radius_pool = NULL;
static unsigned char radius_engine = FALSE;
static radius_server_t *radius_acct_server = NULL;
static radius_server_t *radius_auth_server = NULL;

static int radius_logfd = -1;
static char *radius_logname = NULL;

static struct sockaddr radius_local_sock, radius_remote_sock;

/* For tracking various values not stored in the session struct */
static const char *radius_nas_identifier = "ftp";
static char *radius_realm = NULL;
static time_t radius_session_start = 0;
static int radius_session_authtype = RADIUS_AUTH_LOCAL;
static unsigned char radius_auth_ok = FALSE;
static unsigned char radius_auth_reject = FALSE;

/* "Fake" user/group information for RADIUS users. */
static unsigned char radius_have_user_info = FALSE;
static struct passwd radius_passwd;

static unsigned char radius_have_group_info = FALSE;
static char *radius_prime_group_name = NULL;
static unsigned int radius_addl_group_count = 0;
static char **radius_addl_group_names = NULL;
static char *radius_addl_group_names_str = NULL;
static gid_t *radius_addl_group_ids = NULL;
static char *radius_addl_group_ids_str = NULL;

/* Quota info */
static unsigned char radius_have_quota_info = FALSE;
static char *radius_quota_per_sess = NULL;
static char *radius_quota_limit_type = NULL;
static char *radius_quota_bytes_in = NULL;
static char *radius_quota_bytes_out = NULL;
static char *radius_quota_bytes_xfer = NULL;
static char *radius_quota_files_in = NULL;
static char *radius_quota_files_out = NULL;
static char *radius_quota_files_xfer = NULL;

/* Other info */
static unsigned char radius_have_other_info = FALSE;

/* Vendor information, defaults to Unix (Vendor-Id of 4) */
static const char *radius_vendor_name = "Unix";
static unsigned int radius_vendor_id = 4;

/* Custom VSA IDs that may be used for server-supplied RadiusUserInfo
 * parameters.
 */
static int radius_uid_attr_id = 0;
static int radius_gid_attr_id = 0;
static int radius_home_attr_id = 0;
static int radius_shell_attr_id = 0;

/* Custom VSA IDs that may be used for server-supplied RadiusGroupInfo
 * parameters.
 */
static int radius_prime_group_name_attr_id = 0;
static int radius_addl_group_names_attr_id = 0;
static int radius_addl_group_ids_attr_id = 0;

/* Custom VSA IDs that may be used for server-supplied QuotaLimitTable
 * parameters.
 */
static int radius_quota_per_sess_attr_id = 0;
static int radius_quota_limit_type_attr_id = 0;
static int radius_quota_bytes_in_attr_id = 0;
static int radius_quota_bytes_out_attr_id = 0;
static int radius_quota_bytes_xfer_attr_id = 0;
static int radius_quota_files_in_attr_id = 0;
static int radius_quota_files_out_attr_id = 0;
static int radius_quota_files_xfer_attr_id = 0;

/* For tracking the ID of the last accounting packet (to prevent the
 * same ID from being reused).
 */
static unsigned char radius_last_acct_pkt_id = 0;

/* Convenience macros. */
#define RADIUS_IS_VAR(str) \
  str[0] == '$' && str[1] == '(' && str[strlen(str)-1] == ')'

/* Function prototypes. */
static void radius_add_attrib(radius_packet_t *, unsigned char,
  const unsigned char *, size_t);
static void radius_add_passwd(radius_packet_t *, unsigned char,
  const unsigned char *, unsigned char *);
static void radius_build_packet(radius_packet_t *, const unsigned char *,
  const unsigned char *, unsigned char *);
static unsigned char radius_chk_var(char *);
static int radius_closelog(void);
static int radius_close_socket(int);
static void radius_get_acct_digest(radius_packet_t *, unsigned char *);
static radius_attrib_t *radius_get_attrib(radius_packet_t *, unsigned char);
static void radius_get_rnd_digest(radius_packet_t *);
static radius_attrib_t *radius_get_vendor_attrib(radius_packet_t *,
  unsigned char);

static int radius_log(const char *, ...)
#ifdef __GNUC__
       __attribute__ ((format (printf, 1, 2)));
#else
       ;
#endif

static radius_server_t *radius_make_server(pool *);
static int radius_openlog(void);
static int radius_open_socket(void);
static unsigned char radius_parse_gids_str(pool *, char *, gid_t **,
  unsigned int *);
static unsigned char radius_parse_groups_str(pool *, char *, char ***,
  unsigned int *);
static void radius_parse_var(char *, int *, char **);
static void radius_process_accpt_packet(radius_packet_t *);
static void radius_process_group_info(config_rec *);
static void radius_process_quota_info(config_rec *);
static void radius_process_user_info(config_rec *);
static radius_packet_t *radius_recv_packet(int, unsigned int);
static int radius_send_packet(int, radius_packet_t *, radius_server_t *);
static unsigned char radius_start_accting(void);
static unsigned char radius_stop_accting(void);
static int radius_verify_packet(radius_packet_t *, radius_packet_t *,
  unsigned char *);

/* Support functions
 */

static char *radius_argsep(char **arg) {
  char *ret = NULL, *dst = NULL;
  char quote_mode = 0;

  if (!arg || !*arg || !**arg)
    return NULL;

  while (**arg && isspace((int) **arg))
    (*arg)++;

  if (!**arg)
    return NULL;

  ret = dst = *arg;

  if (**arg == '\"') {
    quote_mode++;
    (*arg)++;
  }

  while (**arg && **arg != ',' &&
      (quote_mode ? (**arg != '\"') : (!isspace((int) **arg)))) {

    if (**arg == '\\' && quote_mode) {

      /* escaped char */
      if (*((*arg) + 1))
        *dst = *(++(*arg));
    }

    *dst++ = **arg;
    ++(*arg);
  }

  if (**arg)
    (*arg)++;

  *dst = '\0';
  return ret;
}

/* Check a "$(attribute-id:default)" string for validity. */
static unsigned char radius_chk_var(char *var) {
  int id = 0;
  char *tmp = NULL;

  /* Must be at least six characters. */
  if (strlen(var) < 7)
    return FALSE;

  /* Must start with '$(', and end with ')'. */
  if (!RADIUS_IS_VAR(var))
    return FALSE;

  /* Must have a ':'. */
  if ((tmp = strchr(var, ':')) == NULL)
    return FALSE;

  /* ':' must be between '(' and ')'. */
  if (tmp < (var + 3) || tmp > &var[strlen(var)-2])
    return FALSE;

  /* Parse out the component int/string. */
  radius_parse_var(var, &id, NULL);

  /* Int must be greater than zero. */
  if (id < 1)
    return FALSE;

  return TRUE;
}

/* Separate the given "$(attribute-id:default)" string into its constituent
 * custom attribute ID (int) and default (string) components.
 */
static void radius_parse_var(char *var, int *attr_id, char **attr_default) {
  pool *tmp_pool = make_sub_pool(radius_pool);
  char *var_cpy = pstrdup(tmp_pool, var), *tmp = NULL;

  /* First, strip off the "$()" variable characters. */
  var_cpy[strlen(var_cpy)-1] = '\0';
  var_cpy += 2;

  /* Find the delimiting ':' */
  tmp = strchr(var_cpy, ':');

  *tmp++ = '\0';

  if (attr_id)
    *attr_id = atoi(var_cpy);

  if (attr_default) {
    tmp = strchr(var, ':');

    /* Note: this works because the calling of this function by
     * radius_chk_var(), which occurs during the parsing process, uses
     * a NULL for this portion, so that the string stored in the config_rec
     * is not actually manipulated, as is done here.
     */
    var[strlen(var)-1] = '\0';

    *attr_default = ++tmp;
  }

  /* Clean up. */
  destroy_pool(tmp_pool);
}

static unsigned char radius_parse_gids_str(pool *p, char *gids_str, 
    gid_t **gids, unsigned int *ngids) {
  char *val = NULL;
  array_header *group_ids = make_array(p, 0, sizeof(gid_t *));

  /* Add each GID to the array. */
  while ((val = radius_argsep(&gids_str)) != NULL) {
    gid_t gid;
    char *endp = NULL;

    /* Make sure the given ID is a valid number. */
    gid = strtoul(val, &endp, 10);

    if (endp && *endp) {
      pr_log_pri(PR_LOG_NOTICE, "RadiusGroupInfo badly formed group ID: %s",
        val);
      return FALSE;
    }

    /* Push the ID into the ID array. */
    *((gid_t *) push_array(group_ids)) = gid;
  }

  *gids = (gid_t *) group_ids->elts;
  *ngids = group_ids->nelts;

  return TRUE;
}

static unsigned char radius_parse_groups_str(pool *p, char *groups_str,
    char ***groups, unsigned int *ngroups) {
  char *name = NULL;
  array_header *group_names = make_array(p, 0, sizeof(char *));

  /* Add each name to the array. */
  while ((name = radius_argsep(&groups_str)) != NULL) {
    char *tmp = pstrdup(p, name);

    /* Push the name into the name array. */
    *((char **) push_array(group_names)) = tmp;
  }

  *groups = (char **) group_names->elts;
  *ngroups = group_names->nelts;

  return TRUE;
}

static void radius_process_accpt_packet(radius_packet_t *packet) {

  /* First, parse the packet for any non-RadiusUserInfo attributes,
   * such as timeouts.  None are currently implemented, but...this would
   * be the place to do it.
   */
  /* radius_log("parsing packet for custom attribute IDs"); */

  /* Now, parse the packet for any server-supplied RadiusUserInfo attributes,
   * if RadiusUserInfo is indeed in effect.
   */

  if (!radius_have_user_info &&
      !radius_have_group_info &&
      !radius_have_quota_info)
    /* Return now if there's no reason for doing extra work. */
    return;

  if (radius_uid_attr_id || radius_gid_attr_id ||
      radius_home_attr_id || radius_shell_attr_id) {
    radius_log("parsing packet for RadiusUserInfo attributes");

    /* These custom values will been supplied in the configuration file, and
     * set when the RadiusUserInfo config_rec is retrieved, during
     * session initialization.
     */

    if (radius_uid_attr_id) {
      radius_attrib_t *attrib = radius_get_vendor_attrib(packet,
        radius_uid_attr_id);

      if (attrib) {
        int uid = -1;

        /* Parse the attribute value into an int, then cast it into the
         * radius_passwd.pw_uid field.  Make sure it's a sane UID
         * (ie non-negative).
         */

        if (attrib->length > sizeof(uid)) {
          radius_log("invalid attribute length (%u) for user ID, truncating",
            attrib->length);
          attrib->length = sizeof(uid);
        }

        memcpy(&uid, attrib->data, attrib->length);
        uid = ntohl(uid);

        if (uid < 0)
          radius_log("packet includes '%s' Vendor-Specific Attribute %d for "
            "illegal user ID: '%u'", radius_vendor_name, radius_uid_attr_id,
            uid);

        else {
          radius_passwd.pw_uid = uid;

          radius_log("packet includes '%s' Vendor-Specific Attribute %d for "
            "user ID: '%u'", radius_vendor_name, radius_uid_attr_id,
            radius_passwd.pw_uid);
        }

      } else
        radius_log("packet lacks '%s' Vendor-Specific Attribute %d for "
          "user ID: defaulting to '%u'", radius_vendor_name, radius_uid_attr_id,
          radius_passwd.pw_uid);
    }

    if (radius_gid_attr_id) {
      radius_attrib_t *attrib = radius_get_vendor_attrib(packet,
        radius_gid_attr_id);

      if (attrib) {
        int gid = -1;

        /* Parse the attribute value into an int, then cast it into the
         * radius_passwd.pw_gid field.  Make sure it's a sane GID
         * (ie non-negative).
         */

        if (attrib->length > sizeof(gid)) {
          radius_log("invalid attribute length (%u) for group ID, truncating",
            attrib->length);
          attrib->length = sizeof(gid);
        }

        memcpy(&gid, attrib->data, attrib->length);
        gid = ntohl(gid);

        if (gid < 0)
          radius_log("packet includes '%s' Vendor-Specific Attribute %d for "
            "illegal group ID: '%u'", radius_vendor_name, radius_gid_attr_id,
            gid);

        else {
          radius_passwd.pw_gid = gid;

          radius_log("packet includes '%s' Vendor-Specific Attribute %d for "
            "group ID: '%u'", radius_vendor_name, radius_gid_attr_id,
            radius_passwd.pw_gid);
        }

      } else
        radius_log("packet lacks '%s' Vendor-Specific Attribute %d for "
          "group ID: defaulting to '%u'", radius_vendor_name,
          radius_gid_attr_id, radius_passwd.pw_gid);
    }

    if (radius_home_attr_id) {
      radius_attrib_t *attrib = radius_get_vendor_attrib(packet,
        radius_home_attr_id);

      if (attrib) {

        /* RADIUS strings are not NUL-terminated. */
        char *home = pcalloc(radius_pool, attrib->length + 1);

        /* Parse the attribute value into a string of the necessary length,
         * then replace radius_passwd.pw_dir with it.  Make sure it's a sane
         * home directory (ie starts with a '/').
         */

        /* Dare we trust attrib->length? */
        memcpy(home, attrib->data, attrib->length);

        if (*home != '/')
          radius_log("packet includes '%s' Vendor-Specific Attribute %d for "
            "illegal home: '%s'", radius_vendor_name, radius_home_attr_id,
            home);

        else {
          radius_passwd.pw_dir = home;

          radius_log("packet includes '%s' Vendor-Specific Attribute %d for "
            "home directory: '%s'", radius_vendor_name, radius_home_attr_id,
            radius_passwd.pw_dir);
        }

      } else
        radius_log("packet lacks '%s' Vendor-Specific Attribute %d for "
          "home directory: defaulting to '%s'", radius_vendor_name,
          radius_home_attr_id, radius_passwd.pw_dir);
    }

    if (radius_shell_attr_id) {
      radius_attrib_t *attrib = radius_get_vendor_attrib(packet,
        radius_shell_attr_id);

      if (attrib) {
        /* RADIUS strings are not NUL-terminated. */
        char *shell = pcalloc(radius_pool, attrib->length + 1);

        /* Parse the attribute value into a string of the necessary length,
         * then replace radius_passwd.pw_shell with it.  Make sure it's a sane
         * shell (ie starts with a '/').
         */

        /* Dare we trust attrib->length? */
        memcpy(shell, attrib->data, attrib->length);

        if (*shell != '/')
          radius_log("packet includes '%s' Vendor-Specific Attribute %d for "
            "illegal shell: '%s'", radius_vendor_name, radius_shell_attr_id,
            shell);

        else {
          radius_passwd.pw_shell = shell;

          radius_log("packet includes '%s' Vendor-Specific Attribute %d for "
            "shell: '%s'", radius_vendor_name, radius_shell_attr_id,
            radius_passwd.pw_shell);
        }

      } else
        radius_log("packet lacks '%s' Vendor-Specific Attribute %d for "
          "shell: defaulting to '%s'", radius_vendor_name, radius_shell_attr_id,
          radius_passwd.pw_shell);
    }
  }

  if (radius_prime_group_name_attr_id ||
      radius_addl_group_names_attr_id ||
      radius_addl_group_ids_attr_id) {
    unsigned int ngroups = 0, ngids = 0;
    char **groups = NULL;
    gid_t *gids = NULL;

    radius_log("parsing packet for RadiusGroupInfo attributes");

    if (radius_prime_group_name_attr_id) {
      radius_attrib_t *attrib = radius_get_vendor_attrib(packet,
        radius_prime_group_name_attr_id);

      if (attrib) {
        /* RADIUS strings are not NUL-terminated. */
        char *group_name = pcalloc(radius_pool, attrib->length + 1);

        /* Dare we trust attrib->length? */
        memcpy(group_name, attrib->data, attrib->length);

        radius_prime_group_name = group_name;

        radius_log("packet includes '%s' Vendor-Specific Attribute %d for "
          "primary group name: '%s'", radius_vendor_name,
          radius_prime_group_name_attr_id, radius_prime_group_name);

      } else
        radius_log("packet lacks '%s' Vendor-Specific Attribute %d for "
          "prime group name: defaulting to '%s'", radius_vendor_name,
          radius_prime_group_name_attr_id, radius_prime_group_name);
    }

    if (radius_addl_group_names_attr_id) {
      radius_attrib_t *attrib = radius_get_vendor_attrib(packet,
        radius_addl_group_names_attr_id);

      if (attrib) {
        /* RADIUS strings are not NUL-terminated. */
        char *group_names = pcalloc(radius_pool, attrib->length + 1);
        char *group_names_str = NULL;

        /* Dare we trust attrib->length? */
        memcpy(group_names, attrib->data, attrib->length);

        /* Make a copy of the string, for logging purposes.  The parsing
         * of the original string will consume it.
         */
        group_names_str = pstrdup(radius_pool, group_names);

        if (!radius_parse_groups_str(radius_pool, group_names, &groups,
            &ngroups))
          radius_log("packet includes '%s' Vendor-Specific Attribute %d for "
            "illegal additional group names: '%s'", radius_vendor_name,
            radius_addl_group_names_attr_id, group_names_str);

        else
          radius_log("packet includes '%s' Vendor-Specific Attribute %d for "
            "additional group names: '%s'", radius_vendor_name,
            radius_addl_group_names_attr_id, group_names_str);

      } else
        radius_log("packet lacks '%s' Vendor-Specific Attribute %d for "
          "additional group names: defaulting to '%s'", radius_vendor_name,
          radius_addl_group_names_attr_id, radius_addl_group_names_str);
    }     

    if (radius_addl_group_ids_attr_id) {
      radius_attrib_t *attrib = radius_get_vendor_attrib(packet,
        radius_addl_group_ids_attr_id);

      if (attrib) {
        /* RADIUS strings are not NUL-terminated. */
        char *group_ids = pcalloc(radius_pool, attrib->length + 1);
        char *group_ids_str = NULL;

        /* Dare we trust attrib->length? */
        memcpy(group_ids, attrib->data, attrib->length);

        /* Make a copy of the string, for logging purposes.  The parsing
         * of the original string will consume it.
         */
        group_ids_str = pstrdup(radius_pool, group_ids);

        if (!radius_parse_gids_str(radius_pool, group_ids, &gids, &ngids))
          radius_log("packet includes '%s' Vendor-Specific Attribute %d for "
            "illegal additional group IDs: '%s'", radius_vendor_name,
            radius_addl_group_ids_attr_id, group_ids_str);

        else
          radius_log("packet includes '%s' Vendor-Specific Attribute %d for "
            "additional group IDs: '%s'", radius_vendor_name,
            radius_addl_group_ids_attr_id, group_ids_str);

      } else
        radius_log("packet lacks '%s' Vendor-Specific Attribute %d for "
          "additional group IDs: defaulting to '%s'", radius_vendor_name,
          radius_addl_group_ids_attr_id, radius_addl_group_ids_str);
    }    

    /* One last RadiusGroupInfo check: does the number of returned group
     * names match the number of returned group IDs?
     */
    if (ngroups == ngids) {
      radius_have_group_info = TRUE;
      radius_addl_group_count = ngroups;
      radius_addl_group_names = groups;
      radius_addl_group_ids = gids;

    } else
      radius_log("server provided mismatched number of group names (%u) "
        "and group IDs (%u), ignoring them", ngroups, ngids);
  }

  if (radius_quota_per_sess_attr_id ||
      radius_quota_limit_type_attr_id ||
      radius_quota_bytes_in_attr_id ||
      radius_quota_bytes_out_attr_id ||
      radius_quota_bytes_xfer_attr_id ||
      radius_quota_files_in_attr_id ||
      radius_quota_files_out_attr_id ||
      radius_quota_files_xfer_attr_id) {

    radius_log("parsing packet for RadiusQuotaInfo attributes");

    if (radius_quota_per_sess_attr_id) {
      radius_attrib_t *attrib = radius_get_vendor_attrib(packet,
        radius_quota_per_sess_attr_id);

      if (attrib) {
        /* RADIUS strings are not NUL-terminated. */
        char *per_sess = pcalloc(radius_pool, attrib->length + 1);

        /* Dare we trust attrib->length? */
        memcpy(per_sess, attrib->data, attrib->length);

        radius_quota_per_sess = per_sess;

        radius_log("packet includes '%s' Vendor-Specific Attribute %d for "
          "quota per-session: '%s'", radius_vendor_name,
          radius_quota_per_sess_attr_id, radius_quota_per_sess);

      } else
        radius_log("packet lacks '%s' Vendor-Specific Attribute %d for "
          "quota per-session: defaulting to '%s'", radius_vendor_name,
          radius_quota_per_sess_attr_id, radius_quota_per_sess);
    }

    if (radius_quota_limit_type_attr_id) {
      radius_attrib_t *attrib = radius_get_vendor_attrib(packet,
        radius_quota_limit_type_attr_id);

      if (attrib) {
        /* RADIUS strings are not NUL-terminated. */
        char *limit_type = pcalloc(radius_pool, attrib->length + 1);

        /* Dare we trust attrib->length? */
        memcpy(limit_type, attrib->data, attrib->length);

        radius_quota_limit_type = limit_type;

        radius_log("packet includes '%s' Vendor-Specific Attribute %d for "
          "quota limit type: '%s'", radius_vendor_name,
          radius_quota_limit_type_attr_id, radius_quota_limit_type);

      } else
        radius_log("packet lacks '%s' Vendor-Specific Attribute %d for "
          "quota limit type: defaulting to '%s'", radius_vendor_name,
          radius_quota_limit_type_attr_id, radius_quota_limit_type);
    }

    if (radius_quota_bytes_in_attr_id) {
      radius_attrib_t *attrib = radius_get_vendor_attrib(packet,
        radius_quota_bytes_in_attr_id);

      if (attrib) {
        /* RADIUS strings are not NUL-terminated. */
        char *bytes_in = pcalloc(radius_pool, attrib->length + 1);

        /* Dare we trust attrib->length? */
        memcpy(bytes_in, attrib->data, attrib->length);

        radius_quota_bytes_in = bytes_in;

        radius_log("packet includes '%s' Vendor-Specific Attribute %d for "
          "quota bytes in available: '%s'", radius_vendor_name,
          radius_quota_bytes_in_attr_id, radius_quota_bytes_in);

      } else
        radius_log("packet lacks '%s' Vendor-Specific Attribute %d for "
          "quota bytes in available: defaulting to '%s'", radius_vendor_name,
          radius_quota_bytes_in_attr_id, radius_quota_bytes_in);
    }

    if (radius_quota_bytes_out_attr_id) {
      radius_attrib_t *attrib = radius_get_vendor_attrib(packet,
        radius_quota_bytes_out_attr_id);

      if (attrib) {
        /* RADIUS strings are not NUL-terminated. */
        char *bytes_out = pcalloc(radius_pool, attrib->length + 1);

        /* Dare we trust attrib->length? */
        memcpy(bytes_out, attrib->data, attrib->length);

        radius_quota_bytes_out = bytes_out;

        radius_log("packet includes '%s' Vendor-Specific Attribute %d for "
          "quota bytes out available: '%s'", radius_vendor_name,
          radius_quota_bytes_out_attr_id, radius_quota_bytes_out);

      } else
        radius_log("packet lacks '%s' Vendor-Specific Attribute %d for "
          "quota bytes out available: defaulting to '%s'", radius_vendor_name,
          radius_quota_bytes_out_attr_id, radius_quota_bytes_out);
    }

    if (radius_quota_bytes_xfer_attr_id) {
      radius_attrib_t *attrib = radius_get_vendor_attrib(packet,
        radius_quota_bytes_xfer_attr_id);

      if (attrib) {
        /* RADIUS strings are not NUL-terminated. */
        char *bytes_xfer = pcalloc(radius_pool, attrib->length + 1);

        /* Dare we trust attrib->length? */
        memcpy(bytes_xfer, attrib->data, attrib->length);

        radius_quota_bytes_xfer = bytes_xfer;

        radius_log("packet includes '%s' Vendor-Specific Attribute %d for "
          "quota bytes xfer available: '%s'", radius_vendor_name,
          radius_quota_bytes_xfer_attr_id, radius_quota_bytes_xfer);

      } else
        radius_log("packet lacks '%s' Vendor-Specific Attribute %d for "
          "quota bytes xfer available: defaulting to '%s'", radius_vendor_name,
          radius_quota_bytes_xfer_attr_id, radius_quota_bytes_xfer);
    }

    if (radius_quota_files_in_attr_id) {
      radius_attrib_t *attrib = radius_get_vendor_attrib(packet,
        radius_quota_files_in_attr_id);

      if (attrib) {
        /* RADIUS strings are not NUL-terminated. */
        char *files_in = pcalloc(radius_pool, attrib->length + 1);

        /* Dare we trust attrib->length? */
        memcpy(files_in, attrib->data, attrib->length);

        radius_quota_files_in = files_in;

        radius_log("packet includes '%s' Vendor-Specific Attribute %d for "
          "quota files in available: '%s'", radius_vendor_name,
          radius_quota_files_in_attr_id, radius_quota_files_in);

      } else
        radius_log("packet lacks '%s' Vendor-Specific Attribute %d for "
          "quota files in available: defaulting to '%s'", radius_vendor_name,
          radius_quota_files_in_attr_id, radius_quota_files_in);
    }

    if (radius_quota_files_out_attr_id) {
      radius_attrib_t *attrib = radius_get_vendor_attrib(packet,
        radius_quota_files_out_attr_id);

      if (attrib) {
        /* RADIUS strings are not NUL-terminated. */
        char *files_out = pcalloc(radius_pool, attrib->length + 1);

        /* Dare we trust attrib->length? */
        memcpy(files_out, attrib->data, attrib->length);

        radius_quota_files_out = files_out;

        radius_log("packet includes '%s' Vendor-Specific Attribute %d for "
          "quota files out available: '%s'", radius_vendor_name,
          radius_quota_files_out_attr_id, radius_quota_files_out);

      } else
        radius_log("packet lacks '%s' Vendor-Specific Attribute %d for "
          "quota files out available: defaulting to '%s'", radius_vendor_name,
          radius_quota_files_out_attr_id, radius_quota_files_out);
    }

    if (radius_quota_files_xfer_attr_id) {
      radius_attrib_t *attrib = radius_get_vendor_attrib(packet,
        radius_quota_files_xfer_attr_id);

      if (attrib) {
        /* RADIUS strings are not NUL-terminated. */
        char *files_xfer = pcalloc(radius_pool, attrib->length + 1);

        /* Dare we trust attrib->length? */
        memcpy(files_xfer, attrib->data, attrib->length);

        radius_quota_files_xfer = files_xfer;

        radius_log("packet includes '%s' Vendor-Specific Attribute %d for "
          "quota files xfer available: '%s'", radius_vendor_name,
          radius_quota_files_xfer_attr_id, radius_quota_files_xfer);

      } else
        radius_log("packet lacks '%s' Vendor-Specific Attribute %d for "
          "quota files xfer available: defaulting to '%s'", radius_vendor_name,
          radius_quota_files_xfer_attr_id, radius_quota_files_xfer);
    }
  }
}

static void radius_process_group_info(config_rec *c) {
  char *param = NULL;
  unsigned char have_illegal_value = FALSE;

  /* Parse out any configured attribute/defaults here. The stored strings will
   * already have been sanitized by the configuration handler, so I don't
   * need to worry about that here.
   */

  param = (char *) c->argv[0];

  if (RADIUS_IS_VAR(param)) {
    radius_parse_var(param, &radius_prime_group_name_attr_id,
      &radius_prime_group_name);

  } else 
    radius_prime_group_name = param;

  /* If the group count (c->argv[1]) is zero, then I know that the data
   * are VSA variable strings.  Otherwise, the group information has
   * already been parsed.
   */
  if (*((unsigned int *) c->argv[1]) == 0) {
    unsigned int ngroups = 0, ngids = 0;
    char **groups = NULL;
    gid_t *gids = NULL;

    radius_parse_var((char *) c->argv[2], &radius_addl_group_names_attr_id,
      &radius_addl_group_names_str);

    /* Now, parse the default value provided. */
    if (!radius_parse_groups_str(c->pool, radius_addl_group_names_str,
        &groups, &ngroups)) {
      radius_log("badly formatted RadiusGroupInfo default additional "
        "group names");
      have_illegal_value = TRUE;
    }

    radius_parse_var((char *) c->argv[3], &radius_addl_group_ids_attr_id,
      &radius_addl_group_ids_str);

    /* Similarly, parse the default value provided. */
    if (!radius_parse_gids_str(c->pool, radius_addl_group_ids_str,
        &gids, &ngids)) {
      radius_log("badly formatted RadiusGroupInfo default additional "
        "group IDs");
      have_illegal_value = TRUE;
    }

    if (!have_illegal_value && ngroups != ngids) {
      radius_log("mismatched number of RadiusGroupInfo default additional "
        "group names (%u) and IDs (%u)", ngroups, ngids);
      have_illegal_value = TRUE;
    }

    if (!have_illegal_value) {
      radius_have_group_info = TRUE;
      radius_addl_group_count = ngroups;
      radius_addl_group_names = groups;
      radius_addl_group_ids = gids;
    }

  } else {
    radius_have_group_info = TRUE;
    radius_addl_group_count = *((unsigned int *) c->argv[1]);
    radius_addl_group_names = (char **) c->argv[2];
    radius_addl_group_ids = (gid_t *) c->argv[3];
  }

  if (have_illegal_value) {
    radius_have_group_info = FALSE;
    radius_log("error with RadiusGroupInfo parameters, ignoring them");
  }
}

static void radius_process_quota_info(config_rec *c) {
  char *param = NULL;
  unsigned char have_illegal_value = FALSE;

  /* Parse out any configured attribute/defaults here. The stored strings will
   * already have been sanitized by the configuration handler, so I don't
   * need to worry about that here.
   */

  param = (char *) c->argv[0];
  if (RADIUS_IS_VAR(param)) {
    radius_parse_var(param, &radius_quota_per_sess_attr_id,
      &radius_quota_per_sess);

  } else {
    radius_quota_per_sess = param;

    if (strcasecmp(param, "false") != 0 &&
        strcasecmp(param, "true") != 0) {
      radius_log("illegal RadiusQuotaInfo per-session value: '%s'", param);
      have_illegal_value = TRUE;
    }
  }

  param = (char *) c->argv[1];
  if (RADIUS_IS_VAR(param)) {
    radius_parse_var(param, &radius_quota_limit_type_attr_id,
      &radius_quota_limit_type);

  } else {
    radius_quota_limit_type = param;

    if (strcasecmp(param, "hard") != 0 &&
        strcasecmp(param, "soft") != 0) {
      radius_log("illegal RadiusQuotaInfo limit type value: '%s'", param);
      have_illegal_value = TRUE;
    }
  }

  param = (char *) c->argv[2];
  if (RADIUS_IS_VAR(param)) {
    radius_parse_var(param, &radius_quota_bytes_in_attr_id,
      &radius_quota_bytes_in);

  } else {
    char *endp = NULL;

    if (strtod(param, &endp) < 0) {
      radius_log("illegal RadiusQuotaInfo bytes in value: negative number");
      have_illegal_value = TRUE;
    }

    if (endp && *endp) {
      radius_log("illegal RadiusQuotaInfo bytes in value: '%s' not a number",
        param);
      have_illegal_value = TRUE;
    }

    radius_quota_bytes_in = param;
  }

  param = (char *) c->argv[3];
  if (RADIUS_IS_VAR(param)) {
    radius_parse_var(param, &radius_quota_bytes_out_attr_id,
      &radius_quota_bytes_out);

  } else {
    char *endp = NULL;

    if (strtod(param, &endp) < 0) {
      radius_log("illegal RadiusQuotaInfo bytes out value: negative number");
      have_illegal_value = TRUE;
    }

    if (endp && *endp) {
      radius_log("illegal RadiusQuotaInfo bytes out value: '%s' not a number",
        param);
      have_illegal_value = TRUE;
    }

    radius_quota_bytes_out = param;
  }

  param = (char *) c->argv[4];
  if (RADIUS_IS_VAR(param)) {
    radius_parse_var(param, &radius_quota_bytes_xfer_attr_id,
      &radius_quota_bytes_xfer);

  } else {
    char *endp = NULL;

    if (strtod(param, &endp) < 0) {
      radius_log("illegal RadiusQuotaInfo bytes xfer value: negative number");
      have_illegal_value = TRUE;
    }

    if (endp && *endp) {
      radius_log("illegal RadiusQuotaInfo bytes xfer value: '%s' not a number",
        param);
      have_illegal_value = TRUE;
    }

    radius_quota_bytes_xfer = param;
  }

  param = (char *) c->argv[5];
  if (RADIUS_IS_VAR(param)) {
    radius_parse_var(param, &radius_quota_files_in_attr_id,
      &radius_quota_files_in);

  } else {
    char *endp = NULL;

    if (strtoul(param, &endp, 10) < 0) {
      radius_log("illegal RadiusQuotaInfo files in value: negative number");
      have_illegal_value = TRUE;
    }

    if (endp && *endp) {
      radius_log("illegal RadiusQuotaInfo files in value: '%s' not a number",
        param);
      have_illegal_value = TRUE;
    }

    radius_quota_files_in = param;
  }

  param = (char *) c->argv[6];
  if (RADIUS_IS_VAR(param)) {
    radius_parse_var(param, &radius_quota_files_out_attr_id,
      &radius_quota_files_out);

  } else {
    char *endp = NULL;

    if (strtoul(param, &endp, 10) < 0) {
      radius_log("illegal RadiusQuotaInfo files out value: negative number");
      have_illegal_value = TRUE;
    }
    
    if (endp && *endp) {
      radius_log("illegal RadiusQuotaInfo files out value: '%s' not a number",
        param);
      have_illegal_value = TRUE;
    }

    radius_quota_files_out = param;
  }

  param = (char *) c->argv[7];
  if (RADIUS_IS_VAR(param)) {
    radius_parse_var(param, &radius_quota_files_xfer_attr_id,
      &radius_quota_files_xfer);

  } else {
    char *endp = NULL;

    if (strtoul(param, &endp, 10) < 0) {
      radius_log("illegal RadiusQuotaInfo files xfer value: negative number");
      have_illegal_value = TRUE;
    }
    
    if (endp && *endp) {
      radius_log("illegal RadiusQuotaInfo files xfer value: '%s' not a number",
        param);
      have_illegal_value = TRUE;
    }

    radius_quota_files_xfer = param;
  }

  if (!have_illegal_value)
    radius_have_quota_info = TRUE;
  else
   radius_log("error with RadiusQuotaInfo parameters, ignoring them");
}

static void radius_process_user_info(config_rec *c) {
  char *param = NULL;
  unsigned char have_illegal_value = FALSE;
 
  /* radius_passwd.pw_name will be filled in later, after successful
   * authentication.  radius_passwd.pw_gecos will always be NULL, as there
   * is no practical need for this information.
   */

  radius_passwd.pw_passwd = NULL;
  radius_passwd.pw_gecos = NULL;

  /* Parse out any configured attribute/defaults here. The stored strings will
   * already have been sanitized by the configuration handler, so I don't
   * need to worry about that here.
   */

  /* Process the UID string. */
  param = (char *) c->argv[0];

  if (RADIUS_IS_VAR(param)) {
    char *endp = NULL, *value = NULL;

    radius_parse_var(param, &radius_uid_attr_id, &value);
    radius_passwd.pw_uid = (uid_t) strtoul(value, &endp, 10);

    if (radius_passwd.pw_uid == (uid_t) -1) {
      radius_log("illegal RadiusUserInfo default UID value: -1 not allowed");
      have_illegal_value = TRUE;
    }

    if (endp && *endp) {
      radius_log("illegal RadiusUserInfo default UID value: '%s' not a number",
        value);
      have_illegal_value = TRUE;
    }

  } else {

    char *endp = NULL;
    radius_passwd.pw_uid = (uid_t) strtoul(param, &endp, 10);

    if (radius_passwd.pw_uid == (uid_t) -1) {
      radius_log("illegal RadiusUserInfo UID value: -1 not allowed");
      have_illegal_value = TRUE;
    }

    if (endp && *endp) {
      radius_log("illegal RadiusUserInfo UID value: '%s' not a number", param);
      have_illegal_value = TRUE;
    }
  }

  /* Process the GID string. */
  param = (char *) c->argv[1];

  if (RADIUS_IS_VAR(param)) {
    char *endp = NULL, *value = NULL;

    radius_parse_var(param, &radius_gid_attr_id, &value);
    radius_passwd.pw_gid = (gid_t) strtoul(value, &endp, 10);

    if (radius_passwd.pw_gid == (gid_t) -1) {
      radius_log("illegal RadiusUserInfo default GID value: -1 not allowed");
      have_illegal_value = TRUE;
    }

    if (endp && *endp) {
      radius_log("illegal RadiusUserInfo default GID value: '%s' not a number",
        value);
      have_illegal_value = TRUE;
    }

  } else {

    char *endp = NULL;
    radius_passwd.pw_gid = (gid_t) strtoul(param, &endp, 10);

    if (radius_passwd.pw_gid == (gid_t) -1) {
      radius_log("illegal RadiusUserInfo GID value: -1 not allowed");
      have_illegal_value = TRUE;
    }

    if (endp && *endp) {
      radius_log("illegal RadiusUserInfo GID value: '%s' not a number", param);
      have_illegal_value = TRUE;
    }
  }

  /* Parse the home directory string. */
  param = (char *) c->argv[2];

  if (RADIUS_IS_VAR(param)) {
    radius_parse_var(param, &radius_home_attr_id, &radius_passwd.pw_dir);

    if (*radius_passwd.pw_dir != '/') {
      radius_log("illegal RadiusUserInfo default home value: '%s' "
        "not an absolute path", radius_passwd.pw_dir);
      have_illegal_value = TRUE;
    }

  } else

    /* Param already checked in this case. */
    radius_passwd.pw_dir = param;

  /* Process the shell string. */
  param = (char *) c->argv[3];
  
  if (RADIUS_IS_VAR(param)) {
    radius_parse_var(param, &radius_shell_attr_id, &radius_passwd.pw_shell);

    if (*radius_passwd.pw_shell != '/') {
      radius_log("illegal RadiusUserInfo default shell value: '%s' "
        "not an absolute path", radius_passwd.pw_shell);
      have_illegal_value = TRUE;
    }

  } else

    /* Param already checked in this case. */
    radius_passwd.pw_shell = param;

  if (!have_illegal_value)
    radius_have_user_info = TRUE;
  else
   radius_log("error with RadiusUserInfo parameters, ignoring them");
}

static unsigned char *radius_xor(unsigned char *p, unsigned char *q,
    size_t len) {
  register int i = 0;
  unsigned char *tmp = p;

  for (i = 0; i < len; i++)
    *(p++) ^= *(q++);

  return tmp;
}

#if defined(PR_USE_OPENSSL)
# include <openssl/md5.h>
#else
/* Built-in MD5 */

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991.
 *  All rights reserved.
 *
 * License to copy and use this software is granted provided that it
 * is identified as the "RSA Data Security, Inc. MD5 Message-Digest
 * Algorithm" in all material mentioning or referencing this software
 * or this function.
 *
 * License is also granted to make and use derivative works provided
 * that such works are identified as "derived from the RSA Data
 * Security, Inc. MD5 Message-Digest Algorithm" in all material
 * mentioning or referencing the derived work.
 *
 * RSA Data Security, Inc. makes no representations concerning either
 * the merchantability of this software or the suitability of this
 * software for any particular purpose. It is provided "as is"
 * without express or implied warranty of any kind.
 *
 * These notices must be retained in any copies of any part of this
 * documentation and/or software.
 */

/* MD5 context */
typedef struct {

  /* state (ABCD) */
  uint32_t state[4];

  /* number of bits, module 2^64 (LSB first) */
  uint32_t count[2];

  /* input buffer */
  unsigned char buffer[64];
} MD5_CTX;

static void MD5_Init(MD5_CTX *);
static void MD5_Update(MD5_CTX *, unsigned char *, size_t);
static void MD5_Final(unsigned char *, MD5_CTX *);

/* Note: these MD5 routines are taken from RFC 1321 */

#ifdef HAVE_MEMCPY
# define MD5_memcpy(a, b, c) memcpy((a), (b), (c))
# define MD5_memset(a, b, c) memset((a), (b), (c))
#endif

/* Constants for MD5Transform routine.
 */

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

static void MD5Transform(uint32_t *, unsigned char[64]);
static void Encode(unsigned char *, uint32_t *, unsigned int);
static void Decode(uint32_t *, unsigned char *, unsigned int);

#ifndef HAVE_MEMCPY
static void MD5_memcpy(unsigned char *, unsigned char *, unsigned int);
static void MD5_memset(unsigned char *, int, unsigned int);
#endif

static unsigned char PADDING[64] = {
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* F, G, H and I are basic MD5 functions.
 */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits.
 */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
 * Rotation is separate from addition to prevent recomputation.
 */
#define FF(a, b, c, d, x, s, ac) { \
 (a) += F ((b), (c), (d)) + (x) + (uint32_t)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) { \
 (a) += G ((b), (c), (d)) + (x) + (uint32_t)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) { \
 (a) += H ((b), (c), (d)) + (x) + (uint32_t)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) { \
 (a) += I ((b), (c), (d)) + (x) + (uint32_t)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }

/* MD5 initialization. Begins an MD5 operation, writing a new context.
 */
static void MD5_Init(MD5_CTX *context) {
  context->count[0] = context->count[1] = 0;

  /* Load magic initialization constants.
   */
  context->state[0] = 0x67452301;
  context->state[1] = 0xefcdab89;
  context->state[2] = 0x98badcfe;
  context->state[3] = 0x10325476;
}

/* MD5 block update operation. Continues an MD5 message-digest
 * operation, processing another message block, and updating the
 * context.
 */
static void MD5_Update(MD5_CTX *context, unsigned char *input,
    size_t inputLen) {
  unsigned int i, index, partLen;

  /* Compute number of bytes mod 64 */
  index = (unsigned int)((context->count[0] >> 3) & 0x3F);

  /* Update number of bits */
  if ((context->count[0] += ((uint32_t)inputLen << 3))
       < ((uint32_t)inputLen << 3))
    context->count[1]++;
  context->count[1] += ((uint32_t)inputLen >> 29);

  partLen = 64 - index;

  /* Transform as many times as possible */
  if (inputLen >= partLen) {
    MD5_memcpy((unsigned char *) &context->buffer[index],
      (unsigned char *) input, partLen);
    MD5Transform(context->state, context->buffer);

    for (i = partLen; i + 63 < inputLen; i += 64)
      MD5Transform(context->state, &input[i]);

    index = 0;

  } else
    i = 0;

  /* Buffer remaining input */
  MD5_memcpy((unsigned char *) &context->buffer[index],
    (unsigned char *) &input[i], inputLen-i);
}

/* MD5 finalization. Ends an MD5 message-digest operation, writing the
 * the message digest and zeroizing the context.
 */
static void MD5_Final(unsigned char digest[16], MD5_CTX *context) {
  unsigned char bits[8];
  unsigned int index;
  size_t padLen;

  /* Save number of bits */
  Encode (bits, context->count, 8);

  /* Pad out to 56 mod 64.
   */
  index = (unsigned int) ((context->count[0] >> 3) & 0x3f);
  padLen = (index < 56) ? (56 - index) : (120 - index);
  MD5_Update(context, PADDING, padLen);

  /* Append length (before padding) */
  MD5_Update(context, bits, 8);

  /* Store state in digest */
  Encode(digest, context->state, 16);

  /* Zeroize sensitive information.
   */
  MD5_memset((unsigned char *) context, 0, sizeof(*context));
}

/* MD5 basic transformation. Transforms state based on block.
 */
static void MD5Transform(uint32_t state[4], unsigned char block[64]) {
  uint32_t a = state[0], b = state[1], c = state[2], d = state[3], x[16];

  Decode(x, block, 64);

  /* Round 1 */
  FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
  FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
  FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
  FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
  FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
  FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
  FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
  FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
  FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
  FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
  FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
  FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
  FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
  FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
  FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
  FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

 /* Round 2 */
  GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
  GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
  GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
  GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
  GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
  GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
  GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
  GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
  GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
  GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
  GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */

  GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
  GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
  GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
  GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
  GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

  /* Round 3 */
  HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
  HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
  HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
  HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
  HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
  HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
  HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
  HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
  HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
  HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
  HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
  HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
  HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
  HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
  HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
  HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

  /* Round 4 */
  II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
  II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
  II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
  II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
  II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
  II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
  II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
  II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
  II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
  II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
  II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
  II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
  II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
  II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
  II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
  II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;

  /* Zeroize sensitive information.
   */
  MD5_memset((unsigned char *) x, 0, sizeof(x));
}

/* Encodes input (unsigned long) into output (unsigned char). Assumes len is
 * a multiple of 4.
 */
static void Encode(unsigned char *output, uint32_t *input, unsigned int len) {
  unsigned int i, j;

  for (i = 0, j = 0; j < len; i++, j += 4) {
    output[j] = (unsigned char)(input[i] & 0xff);
    output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
    output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
    output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
  }
}

/* Decodes input (unsigned char) into output (unsigned long). Assumes len is
 * a multiple of 4.
 */
static void Decode(uint32_t *output, unsigned char *input, unsigned int len) {
  unsigned int i, j;

  for (i = 0, j = 0; j < len; i++, j += 4)
    output[i] = ((uint32_t)input[j]) | (((uint32_t)input[j+1]) << 8) |
    (((uint32_t)input[j+2]) << 16) | (((uint32_t)input[j+3]) << 24);
}

#ifndef HAVE_MEMCPY
/* Note: Replace "for loop" with standard memcpy if possible.
 */
static void MD5_memcpy(unsigned char *output, unsigned char *input,
    unsigned int len) {
  unsigned int i;

  for (i = 0; i < len; i++)
    output[i] = input[i];
}

/* Note: Replace "for loop" with standard memset if possible.
 */
static void MD5_memset(unsigned char *output, int value, unsigned int len) {
  unsigned int i;

  for (i = 0; i < len; i++)
    ((char *)output)[i] = (char)value;
}
#endif
#endif /* !PR_USE_OPENSSL */

/* Logging */

static int radius_closelog(void) {

  /* sanity check */
  if (radius_logfd != -1) {
    close(radius_logfd);
    radius_logfd = -1;
    radius_logname = NULL;
  }

  return 0;
}

static int radius_log(const char *fmt, ...) {
  va_list msg;
  int res;

  /* sanity check */
  if (!radius_logname)
    return 0;

  va_start(msg, fmt);
  res = pr_log_vwritefile(radius_logfd, MOD_RADIUS_VERSION, fmt, msg);
  va_end(msg);

  return res;
}

static int radius_openlog(void) {
  int res = 0;

  /* Sanity checks */
  radius_logname = (char *) get_param_ptr(main_server->conf, "RadiusLog",
    FALSE);
  if (radius_logname == NULL)
    return 0;

  if (strcasecmp(radius_logname, "none") == 0) {
    radius_logname = NULL;
    return 0;
  }

  pr_signals_block();
  PRIVS_ROOT
  res = pr_log_openfile(radius_logname, &radius_logfd, 0640);
  PRIVS_RELINQUISH
  pr_signals_unblock();

  return res;
}

/* RADIUS routines */

/* Add an attribute to a RADIUS packet. */
static void radius_add_attrib(radius_packet_t *packet, unsigned char type,
    const unsigned char *data, size_t datalen) {
  radius_attrib_t *attrib = NULL;

  attrib = (radius_attrib_t *) ((unsigned char *) packet +
    ntohs(packet->length));
  attrib->type = type;

  /* Total size of the attribute.  The "+ 2" takes into account the size
   * of the attribute identifier.
   */
  attrib->length = datalen + 2;

  /* Increment the size of the given packet. */
  packet->length = htons(ntohs(packet->length) + attrib->length);

  memcpy(attrib->data, data, datalen);
}

/* Add a RADIUS password attribute to the packet. */
static void radius_add_passwd(radius_packet_t *packet, unsigned char type,
    const unsigned char *passwd, unsigned char *secret) {

  MD5_CTX ctx, secret_ctx;
  radius_attrib_t *attrib = NULL;
  unsigned char calculated[RADIUS_VECTOR_LEN];
  unsigned char pwhash[PR_TUNABLE_BUFFER_SIZE];
  unsigned char *digest = NULL;
  register unsigned int i = 0;
  size_t pwlen, secretlen;

  pwlen = strlen((const char *) passwd);

  if (pwlen == 0) {
    pwlen = RADIUS_PASSWD_LEN;

  } if ((pwlen & (RADIUS_PASSWD_LEN - 1)) != 0) {

    /* Round up the length. */
    pwlen += (RADIUS_PASSWD_LEN - 1);

    /* Truncate the length, as necessary. */
    pwlen &= ~(RADIUS_PASSWD_LEN - 1);
  }

  /* Clear the buffers. */
  memset(pwhash, '\0', sizeof(pwhash));
  memcpy(pwhash, passwd, pwlen);

  /* Find the password attribute. */
  attrib = radius_get_attrib(packet, RADIUS_PASSWORD);

  if (type == RADIUS_PASSWORD) {
    digest = packet->digest;

  } else {
    digest = attrib->data;
  }

  /* Encrypt the password.  Password: c[0] = p[0] ^ MD5(secret + digest) */
  secretlen = strlen((const char *) secret);
  MD5_Init(&secret_ctx);
  MD5_Update(&secret_ctx, secret, secretlen);

  /* Save this hash for later. */
  ctx = secret_ctx;

  MD5_Update(&ctx, digest, RADIUS_VECTOR_LEN);

  /* Set the calculated digest. */
  MD5_Final(calculated, &ctx);

  /* XOR the results. */
  radius_xor(pwhash, calculated, RADIUS_PASSWD_LEN);
  
  /* For each step through: e[i] = p[i] ^ MD5(secret + e[i-1]) */
  for (i = 1; i < (pwlen >> 4); i++) {

    /* Start with the old value of the MD5 sum. */
    ctx = secret_ctx;

    MD5_Update(&ctx, &pwhash[(i-1) * RADIUS_PASSWD_LEN], RADIUS_PASSWD_LEN);

    /* Set the calculated digest. */
    MD5_Final(calculated, &ctx);

    /* XOR the results. */
    radius_xor(&pwhash[i * RADIUS_PASSWD_LEN], calculated, RADIUS_PASSWD_LEN);
  }

  if (type == RADIUS_OLD_PASSWORD)
    attrib = radius_get_attrib(packet, RADIUS_OLD_PASSWORD);
  
  if (!attrib)
    radius_add_attrib(packet, type, pwhash, pwlen);

  else

    /* Overwrite the packet data. */
    memcpy(attrib->data, pwhash, pwlen);
}

static void radius_get_acct_digest(radius_packet_t *packet,
    unsigned char *secret) {
  MD5_CTX ctx;

  /* Clear the current digest (not needed yet for accounting packets) */
  memset(packet->digest, 0, RADIUS_VECTOR_LEN);

  MD5_Init(&ctx);

  /* Add the packet data to the mix. */
  MD5_Update(&ctx, (unsigned char *) packet, ntohs(packet->length));

  /* Add the secret to the mix. */
  MD5_Update(&ctx, secret, strlen((const char *) secret));

  /* Set the calculated digest in place in the packet. */
  MD5_Final(packet->digest, &ctx);
}

/* Obtain a random digest. */
static void radius_get_rnd_digest(radius_packet_t *packet) {
  MD5_CTX ctx;
  struct timeval tv;
  struct timezone tz;

  /* Use the time of day with the best resolution the system can give us,
   * often close to microsecond accuracy.
   */
  gettimeofday(&tv, &tz);

  /* Add in some (possibly) hard to guess information. */      
  tv.tv_sec ^= getpid() * getppid();
      
  /* Use MD5 to obtain (hopefully) cryptographically strong pseudo-random
   * numbers
   */
  MD5_Init(&ctx);
  MD5_Update(&ctx, (unsigned char *) &tv, sizeof(tv));
  MD5_Update(&ctx, (unsigned char *) &tz, sizeof(tz));

  /* Set the calculated digest in the space provided. */
  MD5_Final(packet->digest, &ctx);
}

/* RADIUS packet manipulation functions.
 */

/* Find an attribute in a RADIUS packet.  Note that the packet length
 * is always kept in network byte order.
 */
static radius_attrib_t *radius_get_attrib(radius_packet_t *packet,
    unsigned char type) {
  radius_attrib_t *attrib = (radius_attrib_t *) &packet->data;
  int len = ntohs(packet->length) - RADIUS_HEADER_LEN;

  while (attrib->type != type) {
    if (attrib->length == 0 ||
        (len -= attrib->length) <= 0) {

      /* Requested attribute not found. */
      return NULL;
    }

    /* Examine the next attribute in the packet. */
    attrib = (radius_attrib_t *) ((char *) attrib + attrib->length);
  }

  return attrib;
}

/* Find a Vendor-Specific Attribute (VSA) in a RADIUS packet.  Note that
 * the packet length is always kept in network byte order.
 */
static radius_attrib_t *radius_get_vendor_attrib(radius_packet_t *packet,
    unsigned char type) {
  radius_attrib_t *attrib = (radius_attrib_t *) &packet->data;
  int len = ntohs(packet->length) - RADIUS_HEADER_LEN;

  while (attrib) {
    unsigned int vendor_id = 0;
    radius_attrib_t *vsa = NULL;

    if (attrib->length == 0) {
      radius_log("packet includes invalid length (%u) for attribute type %u, "
        " rejecting", attrib->length, attrib->type);
      return NULL;
    }

    if (attrib->type != RADIUS_VENDOR_SPECIFIC) {
      len -= attrib->length;
      attrib = (radius_attrib_t *) ((char *) attrib + attrib->length);
      continue;
    }

    /* The first four octets (bytes) of data will contain the Vendor-Id. */
    memcpy(&vendor_id, attrib->data, sizeof(unsigned int));
    vendor_id = ntohl(vendor_id);

    if (vendor_id != radius_vendor_id) {
      len -= attrib->length;
      attrib = (radius_attrib_t *) ((char *) attrib + attrib->length);
      continue;
    }

    /* Parse the data value for this attribute into a VSA structure. */
    vsa = (radius_attrib_t *) ((char *) attrib->data + sizeof(int));

    /* Does this VSA have the type requested? */
    if (vsa->type != type) {
      len -= attrib->length;
      attrib = (radius_attrib_t *) ((char *) attrib + attrib->length);
      continue;
    }

    /* Adjust the VSA length (I'm not sure why this is necessary, but a reading
     * of the FreeRADIUS sources show it to be.  Weird.)
     */
    vsa->length -= 2;
    return vsa;
  }

  return NULL;
}

/* Build a RADIUS packet, initializing some of the header and adding
 * common attributes.
 */
static void radius_build_packet(radius_packet_t *packet,
    const unsigned char *user, const unsigned char *passwd,
    unsigned char *secret) {
  unsigned int nas_port_type = htonl(RADIUS_NAS_PORT_TYPE_VIRTUAL);
  int nas_port = htonl(main_server->ServerPort);
  char *caller_id = NULL;
  size_t userlen;

  /* Set the packet length. */
  packet->length = htons(RADIUS_HEADER_LEN);

  /* Obtain a random digest. */
  radius_get_rnd_digest(packet);

  /* Set the ID for the packet. */
  packet->id = packet->digest[0];
 
  /* Add the user attribute. */ 
  userlen = strlen((const char *) user);
  radius_add_attrib(packet, RADIUS_USER_NAME, user, userlen);

  /* Add the password attribute, if given. */
  if (passwd) {
    radius_add_passwd(packet, RADIUS_PASSWORD, passwd, secret);

  } else if (packet->code != RADIUS_ACCT_REQUEST) {

    /* Add a NULL password if necessary. */
    radius_add_passwd(packet, RADIUS_PASSWORD, (const unsigned char *) "",
      secret);
  }

  /* Add a NAS identifier attribute of the service name, e.g. 'ftp'. */
  radius_add_attrib(packet, RADIUS_NAS_IDENTIFIER,
    (const unsigned char *) radius_nas_identifier,
    strlen((const char *) radius_nas_identifier));

#ifndef PR_USE_IPV6
  /* Add a NAS IP address attribute. */
  radius_add_attrib(packet, RADIUS_NAS_IP_ADDRESS, (unsigned char *) &((struct in_addr *) pr_netaddr_get_inaddr(pr_netaddr_get_sess_local_addr()))->s_addr,
    sizeof(((struct in_addr *) pr_netaddr_get_inaddr(pr_netaddr_get_sess_local_addr()))->s_addr));
#endif /* PR_USE_IPV6 */

  /* Add a NAS port attribute. */
  radius_add_attrib(packet, RADIUS_NAS_PORT, (unsigned char *) &nas_port,
    sizeof(int));

  /* Add a NAS port type attribute. */ 
  radius_add_attrib(packet, RADIUS_NAS_PORT_TYPE,
    (unsigned char *) &nas_port_type, sizeof(int));

  /* Add the calling station ID attribute (this is the IP of the connecting
   * client).
   */
  caller_id = (char *) pr_netaddr_get_ipstr(pr_netaddr_get_sess_remote_addr()); 

  radius_add_attrib(packet, RADIUS_CALLING_STATION_ID,
    (const unsigned char *) caller_id, strlen(caller_id));
}

static radius_server_t *radius_make_server(pool *parent_pool) {
  pool *server_pool = NULL;
  radius_server_t *server = NULL;

  /* sanity check */
  if (!parent_pool)
    return NULL;

  /* allocate a subpool for the new rec */
  server_pool = make_sub_pool(parent_pool);

  /* allocate the rec from the subpool */
  server = (radius_server_t *) pcalloc(server_pool,
    sizeof(radius_server_t));

  /* set the members to sane default values */
  server->pool = server_pool;
  server->addr = NULL;
  server->port = RADIUS_AUTH_PORT;
  server->secret = NULL;
  server->timeout = DEFAULT_RADIUS_TIMEOUT;
  server->next = NULL;

  return server; 
}

static int radius_open_socket(void) {
  int sockfd = -1;
  struct sockaddr_in *radius_sockaddr_in = NULL;
  unsigned short local_port = 0;

  /* Obtain a socket descriptor. */
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    radius_log("notice: unable to open socket for communication: %s",
      strerror(errno));
    return -1;
  }

  /* Set the members appropriately to bind to the descriptor. */
  memset((void *) &radius_local_sock, 0, sizeof(radius_local_sock));
  radius_sockaddr_in = (struct sockaddr_in *) &radius_local_sock;
  radius_sockaddr_in->sin_family = AF_INET;
  radius_sockaddr_in->sin_addr.s_addr = INADDR_ANY;

  /*  Use our process ID as a local port for RADIUS.
   */
  local_port = (getpid() & 0x7fff) + 1024;
  do {
    pr_signals_handle();

    local_port++;
    radius_sockaddr_in->sin_port = htons(local_port);

  } while ((bind(sockfd, &radius_local_sock, sizeof(radius_local_sock)) < 0) &&
    (local_port < USHRT_MAX));

  if (local_port >= USHRT_MAX) {
    close(sockfd);
    radius_log("notice: unable to bind to socket: no open local ports");
    return -1;
  }

  /* Done */
  return sockfd;
}

static int radius_close_socket(int sockfd) {
  return close(sockfd);
}

static radius_packet_t *radius_recv_packet(int sockfd, unsigned int timeout) {
  static unsigned char recvbuf[RADIUS_PACKET_LEN];
  radius_packet_t *packet = NULL;
  int res = 0, recvlen = -1;
  socklen_t sockaddrlen = sizeof(struct sockaddr);
  struct timeval tv;
  fd_set rset;

  /* receive the response, waiting as necessary */
  memset(recvbuf, '\0', sizeof(recvbuf));

  tv.tv_sec = timeout;
  tv.tv_usec = 0;

  FD_ZERO(&rset);
  FD_SET(sockfd, &rset);

  res = select(sockfd + 1, &rset, NULL, NULL, &tv);

  if (res == 0) {
    radius_log("server failed to respond in %u seconds", timeout);
    return NULL;

  } else if (res < 0) {

    radius_log("error: unable to receive response: %s", strerror(errno));
    return NULL;
  }

  if ((recvlen = recvfrom(sockfd, (char *) recvbuf, RADIUS_PACKET_LEN,
      0, &radius_remote_sock, &sockaddrlen)) < 0) {
    radius_log("error reading packet: %s", strerror(errno));
    return NULL;
  }

  packet = (radius_packet_t *) recvbuf;

  /* Make sure the packet is of valid length. */
  if (ntohs(packet->length) != recvlen ||
      ntohs(packet->length) > RADIUS_PACKET_LEN) {
    radius_log("received corrupted packet");
    return NULL;
  }

  return packet;
}

static int radius_send_packet(int sockfd, radius_packet_t *packet,
    radius_server_t *server) {
  struct sockaddr_in *radius_sockaddr_in =
    (struct sockaddr_in *) &radius_remote_sock;

  /* Set the members appropriately to send to the descriptor. */
  memset((void *) &radius_remote_sock, '\0', sizeof(radius_remote_sock));
  radius_sockaddr_in->sin_family = AF_INET;
  radius_sockaddr_in->sin_addr.s_addr = pr_netaddr_get_addrno(server->addr);
  radius_sockaddr_in->sin_port = htons(server->port);

  if (sendto(sockfd, (char *) packet, ntohs(packet->length), 0, 
      &radius_remote_sock, sizeof(struct sockaddr_in)) < 0) {
    radius_log("error: unable to send packet: %s", strerror(errno));
    return -1;
  }

  radius_log("sending packet to %s:%u",
    inet_ntoa(radius_sockaddr_in->sin_addr),
    ntohs(radius_sockaddr_in->sin_port));

  return 0;
}


static unsigned char radius_start_accting(void) {
  int sockfd = -1, acct_status = 0, acct_authentic = 0;
  radius_packet_t *request = NULL, *response = NULL;
  radius_server_t *acct_server = NULL;
  unsigned char recvd_response = FALSE, *authenticated = NULL;

  /* Check to see if RADIUS accounting should be done. */
  if (!radius_engine || !radius_acct_server)
    return TRUE;

  /* Only do accounting for authenticated users. */
  authenticated = get_param_ptr(main_server->conf, "authenticated", FALSE);
  if (!authenticated || *authenticated == FALSE)
    return TRUE;

  /* Allocate a packet. */
  request = (radius_packet_t *) pcalloc(radius_pool, sizeof(radius_packet_t));

  /* Open a RADIUS socket */
  sockfd = radius_open_socket();
  if (sockfd < 0) {
    radius_log("socket open failed");
    return FALSE;
  }

  /* Loop through the list of servers, trying each one until the packet is
   * successfully sent.
   */
  acct_server = radius_acct_server;

  while (acct_server) {
    char pid[10] = {'\0'};

    pr_signals_handle();

    /* Clear the packet. */
    memset(request, '\0', sizeof(radius_packet_t));

    /* Build the packet. */
    request->code = RADIUS_ACCT_REQUEST;
    radius_build_packet(request,
      radius_realm ?
        (const unsigned char *) pstrcat(radius_pool, session.user,
          radius_realm, NULL) :
        (const unsigned char *) session.user, NULL, acct_server->secret);

    radius_last_acct_pkt_id = request->id;

    /* Add accounting attributes. */
    acct_status = htonl(RADIUS_ACCT_STATUS_START);
    radius_add_attrib(request, RADIUS_ACCT_STATUS_TYPE,
      (unsigned char *) &acct_status, sizeof(int));

    snprintf(pid, sizeof(pid), "%08d", (int) getpid());
    radius_add_attrib(request, RADIUS_ACCT_SESSION_ID,
      (const unsigned char *) pid, strlen(pid));

    acct_authentic = htonl(RADIUS_AUTH_LOCAL);
    radius_add_attrib(request, RADIUS_ACCT_AUTHENTIC,
      (unsigned char *) &acct_authentic, sizeof(int));

    /* Calculate the signature. */
    radius_get_acct_digest(request, acct_server->secret);

    /* Send the request. */
    radius_log("sending start acct request packet");
    if (radius_send_packet(sockfd, request, acct_server) < 0) {
      radius_log("packet send failed");
      acct_server = acct_server->next;
      continue;
    }

    /* Receive the response. */
    radius_log("receiving acct response packet");
    if ((response = radius_recv_packet(sockfd, acct_server->timeout)) == NULL) {
      radius_log("packet receive failed");
      acct_server = acct_server->next;
      continue;
    }

    radius_log("packet receive succeeded");
    recvd_response = TRUE;
    break;
  }

  /* Close the socket. */
  if (radius_close_socket(sockfd) < 0)
    radius_log("socket close failed");

  if (recvd_response) {

    /* Verify the response. */
    radius_log("verifying packet");
    if (radius_verify_packet(request, response, acct_server->secret) < 0)
      return FALSE;

    /* Handle the response. */
    switch (response->code) {
      case RADIUS_ACCT_RESPONSE:
        radius_log("accounting started for user '%s'", session.user);
        return TRUE;

      default:
        radius_log("notice: server returned unknown response code: %02x",
          response->code);
        return FALSE;
    }

  } else
    radius_log("error: no acct servers responded");

  /* Default return value. */
  return FALSE;
}

static unsigned char radius_stop_accting(void) {
  int sockfd = -1, acct_status = 0, acct_authentic = 0, now = 0;
  radius_packet_t *request = NULL, *response = NULL;
  radius_server_t *acct_server = NULL;
  unsigned char recvd_response = FALSE, *authenticated = NULL;
  off_t radius_session_bytes_in = 0;
  off_t radius_session_bytes_out = 0;

  /* Check to see if RADIUS accounting should be done. */
  if (!radius_engine || !radius_acct_server)
    return TRUE;

  /* Only do accounting for authenticated users. */
  authenticated = get_param_ptr(main_server->conf, "authenticated", FALSE);

  if (!authenticated || *authenticated == FALSE)
    return TRUE;

  /* Allocate a packet. */
  request = (radius_packet_t *) pcalloc(radius_pool, sizeof(radius_packet_t));

  /* Open a RADIUS socket */
  sockfd = radius_open_socket();
  if (sockfd < 0) {
    radius_log("socket open failed");
    return FALSE;
  }

  /* Loop through the list of servers, trying each one until the packet is
   * successfully sent.
   */
  acct_server = radius_acct_server;

  while (acct_server) {
    char pid[10] = {'\0'};

    pr_signals_handle();

    /* Clear the packet. */
    memset(request, '\0', sizeof(radius_packet_t));

    /* Build the packet. */
    request->code = RADIUS_ACCT_REQUEST;
    radius_build_packet(request,
      radius_realm ?
        (const unsigned char *) pstrcat(radius_pool, session.user,
          radius_realm, NULL) :
        (const unsigned char *) session.user, NULL, acct_server->secret);

    /* Use the ID of the last accounting packet sent, plus one.  Be sure
     * to handle the datatype overflow case.
     */
    if ((request->id = radius_last_acct_pkt_id + 1) == 0)
      request->id = 1;

    /* Add accounting attributes. */
    acct_status = htonl(RADIUS_ACCT_STATUS_STOP);
    radius_add_attrib(request, RADIUS_ACCT_STATUS_TYPE,
      (unsigned char *) &acct_status, sizeof(int));

    snprintf(pid, sizeof(pid), "%08d", (int) getpid());
    radius_add_attrib(request, RADIUS_ACCT_SESSION_ID,
      (const unsigned char *) pid, strlen(pid));

    acct_authentic = htonl(RADIUS_AUTH_LOCAL);
    radius_add_attrib(request, RADIUS_ACCT_AUTHENTIC,
      (unsigned char *) &acct_authentic, sizeof(int));

    now = htonl(time(NULL) - radius_session_start);
    radius_add_attrib(request, RADIUS_ACCT_SESSION_TIME,
      (unsigned char *) &now, sizeof(int));

    radius_session_bytes_in = htonl(session.total_bytes_in);
    radius_add_attrib(request, RADIUS_ACCT_INPUT_OCTETS,
      (unsigned char *) &radius_session_bytes_in, sizeof(int));

    radius_session_bytes_out = htonl(session.total_bytes_out);
    radius_add_attrib(request, RADIUS_ACCT_OUTPUT_OCTETS,
      (unsigned char *) &radius_session_bytes_out, sizeof(int));

    /* Calculate the signature. */
    radius_get_acct_digest(request, acct_server->secret);

    /* Send the request. */
    radius_log("sending stop acct request packet");
    if (radius_send_packet(sockfd, request, acct_server) < 0) {
      radius_log("packet send failed");
      return FALSE;
    }

    /* Receive the response. */
    radius_log("receiving acct response packet");
    if ((response = radius_recv_packet(sockfd, acct_server->timeout)) == NULL) {
      radius_log("packet receive failed");
      return FALSE;
    }

    radius_log("packet receive succeeded");
    recvd_response = TRUE;
    break;
  }

  /* Close the socket. */
  if (radius_close_socket(sockfd) < 0)
    radius_log("socket close failed");

  if (recvd_response) {

    /* Verify the response. */
    radius_log("verifying packet");
    if (radius_verify_packet(request, response, acct_server->secret) < 0)
      return FALSE;

    /* Handle the response. */
    switch (response->code) {
      case RADIUS_ACCT_RESPONSE:
        radius_log("accounting ended for user '%s'", session.user);
        return TRUE;

      default:
        radius_log("notice: server returned unknown response code: %02x",
          response->code);
        return FALSE;
    }

  } else 
    radius_log("error: no acct servers responded");

  /* Default return value. */
  return FALSE;
}

/* Verify the response packet from the server. */
static int radius_verify_packet(radius_packet_t *req_packet, 
    radius_packet_t *resp_packet, unsigned char *secret) {
  MD5_CTX ctx;
  unsigned char calculated[RADIUS_VECTOR_LEN] = {'\0'};
  unsigned char replied[RADIUS_VECTOR_LEN] = {'\0'};

  /* sanity check */
  if (!req_packet || !resp_packet || !secret) {
    errno = EINVAL;
    return -1;
  }

  /* NOTE: add checks for too-big, too-small packets, invalid packet->length
   * values, etc.
   */

  /* Check that the packet IDs match. */
  if (resp_packet->id != req_packet->id) {
    radius_log("packet verification failed: response packet ID %d does not "
      "match the request packet ID %d", resp_packet->id, req_packet->id);
    return -1;
  }

  /* Make sure the buffers are void of junk values. */
  memset(calculated, '\0', sizeof(calculated));
  memset(replied, '\0', sizeof(replied));

  /* Save a copy of the response's digest. */
  memcpy(replied, resp_packet->digest, RADIUS_VECTOR_LEN);

  /* Copy in the digest sent in the request. */
  memcpy(resp_packet->digest, req_packet->digest, RADIUS_VECTOR_LEN);

  /* Re-calculate a digest from the given packet, and compare it against
   * the provided response digest:
   *   MD5(response packet header + digest + response packet data + secret)
   */
  MD5_Init(&ctx);
  MD5_Update(&ctx, (unsigned char *) resp_packet, ntohs(resp_packet->length));

  if (*secret)
    MD5_Update(&ctx, secret, strlen((const char *) secret));

  /* Set the calculated digest. */
  MD5_Final(calculated, &ctx);

  /* Do the digests match properly? */
  if (memcmp(calculated, replied, RADIUS_VECTOR_LEN) != 0) {
    radius_log("packet verification failed: mismatched digests");
    return -1;
  }

  return 0;
}

/* Authentication handlers
 */

MODRET radius_auth(cmd_rec *cmd) {

  /* This authentication check has already been performed; I just need
   * to report the results of that check now.
   */
  if (radius_auth_ok) {
    session.auth_mech = "mod_radius.c";
    return PR_HANDLED(cmd);
  }

  else if (radius_auth_reject)
    return PR_ERROR_INT(cmd, PR_AUTH_BADPWD);

  /* Default return value. */
  return PR_DECLINED(cmd);
}

MODRET radius_check(cmd_rec *cmd) {
  return PR_DECLINED(cmd);
}

MODRET radius_name2uid(cmd_rec *cmd) {
  return PR_DECLINED(cmd);
}

MODRET radius_name2gid(cmd_rec *cmd) {
  return PR_DECLINED(cmd);
}

MODRET radius_uid2name(cmd_rec *cmd) {
  return PR_DECLINED(cmd);
}

MODRET radius_gid2name(cmd_rec *cmd) {
  return PR_DECLINED(cmd);
}

MODRET radius_endgrent(cmd_rec *cmd) {
  return PR_DECLINED(cmd);
}

MODRET radius_getgrnam(cmd_rec *cmd) {
  return PR_DECLINED(cmd);
}

MODRET radius_getgrent(cmd_rec *cmd) {
  return PR_DECLINED(cmd);
}

MODRET radius_getgrgid(cmd_rec *cmd) {
  return PR_DECLINED(cmd);
}

MODRET radius_getgroups(cmd_rec *cmd) {

  if (radius_have_group_info) {
    array_header *gids = NULL, *groups = NULL;
    register unsigned int i = 0;

    /* Return the faked group information. */

    /* Don't forget to include the primary GID (with accompanying name!)
     * in the returned info -- if provided.  Otherwise, well...the user
     * is out of luck.
     */

    /* Check for NULL values */
    if (cmd->argv[1]) {
      gids = (array_header *) cmd->argv[1];

      if (radius_have_user_info)
         *((gid_t *) push_array(gids)) = radius_passwd.pw_gid;

      for (i = 0; i < radius_addl_group_count; i++)
        *((gid_t *) push_array(gids)) = radius_addl_group_ids[i];
    }

    if (cmd->argv[2]) {
      groups = (array_header *) cmd->argv[2];

      if (radius_have_user_info)
        *((char **) push_array(groups)) = radius_prime_group_name;

      for (i = 0; i < radius_addl_group_count; i++)
        *((char **) push_array(groups)) = radius_addl_group_names[i];
    }

    /* Increment this count, for the sake of proper reporting back to the
     * getgroups() caller.
     */
    if (radius_have_user_info)
      radius_addl_group_count++;

    return mod_create_data(cmd, (void *) &radius_addl_group_count);
  }

  return PR_DECLINED(cmd);
}

MODRET radius_setgrent(cmd_rec *cmd) {
  return PR_DECLINED(cmd);
}

MODRET radius_endpwent(cmd_rec *cmd) {
  return PR_DECLINED(cmd);
}

MODRET radius_getpwnam(cmd_rec *cmd) {

  if (radius_have_user_info) {

    if (!radius_passwd.pw_name)
      radius_passwd.pw_name = pstrdup(radius_pool, cmd->argv[0]);

    if (strcmp(cmd->argv[0], radius_passwd.pw_name) == 0)

      /* Return the faked user information. */
      return mod_create_data(cmd, &radius_passwd);
  }

  /* Default response */
  return PR_DECLINED(cmd);
}

MODRET radius_getpwent(cmd_rec *cmd) {

  if (radius_have_user_info)

    /* Return the faked user information. */
    return mod_create_data(cmd, &radius_passwd);

  /* Default response */
  return PR_DECLINED(cmd);
}

MODRET radius_getpwuid(cmd_rec *cmd) {

  if (radius_have_user_info)

    /* Check that given UID matches faked UID before returning. */
    if (*((uid_t *) cmd->argv[0]) == radius_passwd.pw_uid)

      /* Return the faked user information. */
      return mod_create_data(cmd, &radius_passwd);

  /* Default response */
  return PR_DECLINED(cmd);
}

MODRET radius_setpwent(cmd_rec *cmd) {
  return PR_DECLINED(cmd);
}

/* Command handlers
 */

/* Handle retrieval of quota-related VSAs from response packets.
 */
MODRET radius_quota_lookup(cmd_rec *cmd) {

  if (radius_have_quota_info) {
    array_header *quota = make_array(session.pool, 9, sizeof(char *));
    *((char **) push_array(quota)) = cmd->argv[0];
    *((char **) push_array(quota)) = radius_quota_per_sess;
    *((char **) push_array(quota)) = radius_quota_limit_type;
    *((char **) push_array(quota)) = radius_quota_bytes_in;
    *((char **) push_array(quota)) = radius_quota_bytes_out;
    *((char **) push_array(quota)) = radius_quota_bytes_xfer;
    *((char **) push_array(quota)) = radius_quota_files_in;
    *((char **) push_array(quota)) = radius_quota_files_out;
    *((char **) push_array(quota)) = radius_quota_files_xfer;

    return mod_create_data(cmd, quota);
  }

  return PR_DECLINED(cmd);
}

/* Perform the check with the RADIUS auth server(s) now, prior to the
 * actual handling of the PASS command by mod_auth, so that any of the
 * RadiusUserInfo parameters can be supplied by the RADIUS server.
 *
 * NOTE: first draft, does not honor UserAlias'd names (eg it uses the
 * username as supplied by the client.
 */
MODRET radius_pre_pass(cmd_rec *cmd) {
  int sockfd = -1;
  radius_packet_t *request = NULL, *response = NULL;
  radius_server_t *auth_server = NULL;
  unsigned char recvd_response = FALSE;
  unsigned long service;
  char *user;

  /* Check to see whether RADIUS authentication should even be done. */
  if (!radius_engine ||
      !radius_auth_server)
    return PR_DECLINED(cmd);

  user = pr_table_get(session.notes, "mod_auth.orig-user", NULL);
  if (!user) {
    radius_log("missing prerequisite USER command, declining to handle PASS");
    pr_response_add_err(R_503, _("Login with USER first"));
    return PR_ERROR(cmd);
  }

  /* Allocate a packet. */
  request = (radius_packet_t *) pcalloc(cmd->tmp_pool,
    sizeof(radius_packet_t));

  /* Open a RADIUS socket */
  sockfd = radius_open_socket();
  if (sockfd < 0) {
    radius_log("socket open failed");
    return PR_DECLINED(cmd);
  }

  /* Clear the OK flag. */
  radius_auth_ok = FALSE;

  /* If mod_radius expects to find VSAs in the returned packet, it needs
   * to send a service type of Login, otherwise, use the Authenticate-Only
   * service type.
   */
  if (radius_have_user_info ||
      radius_have_group_info ||
      radius_have_quota_info ||
      radius_have_other_info)
    service = htonl(RADIUS_SVC_LOGIN);

  else
    service = htonl(RADIUS_SVC_AUTHENTICATE_ONLY);

  /* Loop through the list of servers, trying each one until the packet is
   * successfully sent.
   */
  auth_server = radius_auth_server;

  while (auth_server) {
    pr_signals_handle();

    /* Clear the packet. */
    memset(request, '\0', sizeof(radius_packet_t));

    /* Build the packet. */
    request->code = RADIUS_AUTH_REQUEST;
    radius_build_packet(request, radius_realm ?
      (const unsigned char *) pstrcat(radius_pool, user, radius_realm, NULL) :
      (const unsigned char *) user, (const unsigned char *) cmd->arg,
      auth_server->secret);

    radius_add_attrib(request, RADIUS_SERVICE_TYPE, (unsigned char *) &service,
      sizeof(service));

    /* Send the request. */
    radius_log("sending auth request packet");
    if (radius_send_packet(sockfd, request, auth_server) < 0) {
      radius_log("packet send failed");
      auth_server = auth_server->next;
      continue;
    }

    /* Receive the response. */
    radius_log("receiving auth response packet");
    if ((response = radius_recv_packet(sockfd, auth_server->timeout)) == NULL) {
      radius_log("packet receive failed");
      auth_server = auth_server->next;
      continue;
    }

    radius_log("packet receive succeeded");
    recvd_response = TRUE;
    break;
  }

  /* Close the socket. */
  if (radius_close_socket(sockfd) < 0)
    radius_log("socket close failed");

  if (recvd_response) {

    /* Verify the response. */
    radius_log("verifying packet");
    if (radius_verify_packet(request, response, auth_server->secret) < 0)
      return PR_DECLINED(cmd);

    /* Handle the response */
    switch (response->code) {
      case RADIUS_AUTH_ACCEPT:
        radius_log("authentication successful for user '%s'", user);

        radius_session_authtype = htonl(RADIUS_AUTH_RADIUS);

        /* Process the packet for custom attributes */
        radius_process_accpt_packet(response);

        radius_auth_ok = TRUE;
        break;

      case RADIUS_AUTH_REJECT:
        radius_log("authentication failed for user '%s'", user);
        radius_auth_ok = FALSE;
        radius_auth_reject = TRUE;
        break;

      case RADIUS_AUTH_CHALLENGE:

        /* Just log this case for now. */
        radius_log("authentication challenged for user '%s'", user);
        break;

      default:
        radius_log("notice: server returned unknown response code: %02x",
          response->code);
        break;
    }

  } else
    radius_log("error: no auth servers responded");

  return PR_DECLINED(cmd);
}

MODRET radius_post_pass(cmd_rec *cmd) {

  /* Check to see if RADIUS accounting should be done. */
  if (!radius_engine || !radius_acct_server)
    return PR_DECLINED(cmd);

  /* Fill in the username in the faked user info, if need be. */
  if (radius_have_user_info)
    radius_passwd.pw_name = session.user;

  if (!radius_start_accting())
    radius_log("error: unable to start accounting");

  return PR_DECLINED(cmd);
}

MODRET radius_post_pass_err(cmd_rec *cmd) {
  /* Clear/reset user info */
  radius_have_user_info = FALSE;

  /* Clear/reset group info */
  radius_have_group_info = FALSE;
  radius_prime_group_name = NULL;
  radius_addl_group_count = 0;
  radius_addl_group_names = NULL;
  radius_addl_group_names_str = NULL;
  radius_addl_group_ids = NULL;
  radius_addl_group_ids_str = NULL;

  /* Clear/reset quota info */
  radius_have_quota_info = FALSE;
  radius_quota_per_sess = NULL;
  radius_quota_limit_type = NULL;
  radius_quota_bytes_in = NULL;
  radius_quota_bytes_out = NULL;
  radius_quota_bytes_xfer = NULL;
  radius_quota_files_in = NULL;
  radius_quota_files_out = NULL;
  radius_quota_files_xfer = NULL;

  /* Clear/reset other info */
  radius_have_other_info = FALSE;

  return PR_DECLINED(cmd);
}

/* Configuration handlers
 */

/* usage: RadiusAcctServer server[:port] shared-secret [timeout] */
MODRET set_radiusacctserver(cmd_rec *cmd) {
  config_rec *c = NULL;
  radius_server_t *radius_server = NULL;
  unsigned short server_port = 0;
  char *port = NULL;

  if (cmd->argc-1 < 2 || cmd->argc-1 > 3)
    CONF_ERROR(cmd, "missing parameters");

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  /* Check to see if there's a port specified in the server name */
  port = strchr(cmd->argv[1], ':');
  if (port != NULL) {

    /* Separate the server name from the port */
    *(port++) = '\0';

    server_port = (unsigned short) atoi(port);
    if (server_port < 1024) {
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "port number must be greater "
        "than 1023", NULL));
    }
  }

  if (pr_netaddr_get_addr(cmd->tmp_pool, cmd->argv[1], NULL) == NULL) {
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "unable to resolve server address: ",
      cmd->argv[1], NULL));
  }

  /* Allocate a RADIUS server rec and populate the members */
  radius_server = radius_make_server(radius_pool);

  radius_server->addr = pr_netaddr_get_addr(radius_server->pool, cmd->argv[1],
    NULL);
  radius_server->port = (server_port ? server_port : RADIUS_ACCT_PORT);
  radius_server->secret = (unsigned char *) pstrdup(radius_server->pool,
    cmd->argv[2]);

  if (cmd->argc-1 == 3) {
    if ((radius_server->timeout = atoi(cmd->argv[3])) < 0) {
      CONF_ERROR(cmd, "timeout must be greater than or equal to zero");
    }
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(radius_server_t *));
  *((radius_server_t **) c->argv[0]) = radius_server;

  return PR_HANDLED(cmd);
}

/* usage: RadiusAuthServer server[:port] <shared-secret> [timeout] */
MODRET set_radiusauthserver(cmd_rec *cmd) {
  config_rec *c = NULL;
  radius_server_t *radius_server = NULL;
  unsigned short server_port = 0;
  char *port = NULL;

  if (cmd->argc-1 < 2 || cmd->argc-1 > 3)
    CONF_ERROR(cmd, "missing parameters");

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  /* Check to see if there's a port specified in the server name */
  if ((port = strchr(cmd->argv[1], ':')) != NULL) {

    /* Separate the server name from the port */
    *(port++) = '\0';

    if ((server_port = (unsigned short) atoi(port)) < 1024)
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "port number must be greater "
        "than 1023", NULL));
  }

  if (pr_netaddr_get_addr(cmd->tmp_pool, cmd->argv[1], NULL) == NULL) {
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "unable resolve server address: ",
      cmd->argv[1], NULL));
  }

  /* OK, allocate a RADIUS server rec and populate the members */
  radius_server = radius_make_server(radius_pool);

  radius_server->addr = pr_netaddr_get_addr(radius_server->pool, cmd->argv[1],
    NULL);
  radius_server->port = (server_port ? server_port : RADIUS_AUTH_PORT);
  radius_server->secret = (unsigned char *) pstrdup(radius_server->pool,
    cmd->argv[2]);

  if (cmd->argc-1 == 3) {
    if ((radius_server->timeout = atoi(cmd->argv[3])) < 0) {
      CONF_ERROR(cmd, "timeout must be greater than or equal to zero");
    }
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(radius_server_t *));
  *((radius_server_t **) c->argv[0]) = radius_server;

  return PR_HANDLED(cmd);
}

/* usage: RadiusEngine on|off */
MODRET set_radiusengine(cmd_rec *cmd) {
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

/* usage: RadiusGroupInfo primary-name addl-names add-ids */
MODRET set_radiusgroupinfo(cmd_rec *cmd) {
  config_rec *c = NULL;
  unsigned char group_names_vsa = FALSE;
  unsigned char group_ids_vsa = FALSE;

  CHECK_ARGS(cmd, 3);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  group_names_vsa = radius_chk_var(cmd->argv[2]);
  group_ids_vsa = radius_chk_var(cmd->argv[3]);

  if ((group_names_vsa && !group_ids_vsa) ||
      (!group_names_vsa && group_ids_vsa))
    CONF_ERROR(cmd, "supplemental group names/IDs parameters must both either be VSA variables, or both not be variables");

  /* There will be four parameters to this config_rec:
   *
   *  primary-group-name, addl-group-count, addl-group-names, addl-group-ids
   */
  c = add_config_param(cmd->argv[0], 4, NULL, NULL, NULL, NULL);
  c->argv[0] = pstrdup(c->pool, cmd->argv[1]);
  c->argv[1] = pcalloc(c->pool, sizeof(unsigned int));

  if (group_names_vsa && group_ids_vsa) {
    
    /* As VSA variables, the group names and IDs won't be resolved until
     * session time, so just store the variable strings as is.
     */
    *((unsigned int *) c->argv[1]) = 0;
    c->argv[2] = pstrdup(c->pool, cmd->argv[2]);
    c->argv[3] = pstrdup(c->pool, cmd->argv[3]);

  } else {
    unsigned int ngroups = 0, ngids = 0;
    char **groups = NULL;
    gid_t *gids = NULL;

    if (!radius_parse_groups_str(c->pool, cmd->argv[2], &groups, &ngroups))
      CONF_ERROR(cmd, "badly formatted group names");

    if (!radius_parse_gids_str(c->pool, cmd->argv[3], &gids, &ngids))
      CONF_ERROR(cmd, "badly formatted group IDs");

    if (ngroups != ngids)
      CONF_ERROR(cmd, "mismatched number of group names and IDs");

    *((unsigned int *) c->argv[1]) = ngroups;
    c->argv[2] = (void *) groups;
    c->argv[3] = (void *) gids;
  }

  return PR_HANDLED(cmd);
}

/* usage: RadiusLog file|"none" */
MODRET set_radiuslog(cmd_rec *cmd) {
  if (cmd->argc-1 != 1)
    CONF_ERROR(cmd, "wrong number of parameters");
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

/* usage: RadiusNASIdentifier string */
MODRET set_radiusnasidentifier(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

/* usage: RadiusQuotaInfo per-sess limit-type bytes-in bytes-out bytes-xfer
 *          files-in files-out files-xfer
 */
MODRET set_radiusquotainfo(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 8);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if (!radius_chk_var(cmd->argv[1])) {
    if (strcasecmp(cmd->argv[1], "false") != 0 &&
        strcasecmp(cmd->argv[1], "true") != 0)
      CONF_ERROR(cmd, "invalid per-session value");
  }

  if (!radius_chk_var(cmd->argv[2])) {
    if (strcasecmp(cmd->argv[2], "hard") != 0 &&
        strcasecmp(cmd->argv[2], "soft") != 0)
      CONF_ERROR(cmd, "invalid limit type value");
  }

  if (!radius_chk_var(cmd->argv[3])) {
    char *endp = NULL;

    /* Make sure it's a number, at least. */
    if (strtod(cmd->argv[3], &endp) < 0)
      CONF_ERROR(cmd, "negative bytes value not allowed");

    if (endp && *endp)
      CONF_ERROR(cmd, "invalid bytes parameter: not a number");
  }

  if (!radius_chk_var(cmd->argv[4])) {
    char *endp = NULL;

    /* Make sure it's a number, at least. */
    if (strtod(cmd->argv[4], &endp) < 0)
      CONF_ERROR(cmd, "negative bytes value not allowed");

    if (endp && *endp)
      CONF_ERROR(cmd, "invalid bytes parameter: not a number");
  }

  if (!radius_chk_var(cmd->argv[5])) {
    char *endp = NULL;

    /* Make sure it's a number, at least. */
    if (strtod(cmd->argv[5], &endp) < 0)
      CONF_ERROR(cmd, "negative bytes value not allowed");

    if (endp && *endp)
      CONF_ERROR(cmd, "invalid bytes parameter: not a number");
  }

  if (!radius_chk_var(cmd->argv[6])) {
    char *endp = NULL;

    /* Make sure it's a number, at least. */
    if (strtoul(cmd->argv[6], &endp, 10) < 0)
      CONF_ERROR(cmd, "negative files value not allowed");

    if (endp && *endp)
      CONF_ERROR(cmd, "invalid files parameter: not a number");
  }

  if (!radius_chk_var(cmd->argv[7])) {
    char *endp = NULL;

    /* Make sure it's a number, at least. */
    if (strtoul(cmd->argv[7], &endp, 10) < 0)
      CONF_ERROR(cmd, "negative files value not allowed");

    if (endp && *endp)
      CONF_ERROR(cmd, "invalid files parameter: not a number");
  }

  if (!radius_chk_var(cmd->argv[8])) {
    char *endp = NULL;

    /* Make sure it's a number, at least. */
    if (strtoul(cmd->argv[8], &endp, 10) < 0)
      CONF_ERROR(cmd, "negative files value not allowed");

    if (endp && *endp)
      CONF_ERROR(cmd, "invalid files parameter: not a number");
  }

  add_config_param_str(cmd->argv[0], 8, cmd->argv[1], cmd->argv[2],
    cmd->argv[3], cmd->argv[4], cmd->argv[5], cmd->argv[6],
    cmd->argv[7], cmd->argv[8]);

  return PR_HANDLED(cmd);
}

/* usage: RadiusRealm string */
MODRET set_radiusrealm(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

/* usage: RadiusUserInfo uid gid home shell */
MODRET set_radiususerinfo(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 4);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if (!radius_chk_var(cmd->argv[1])) {
    char *endp = NULL;

    /* Make sure it's a number, at least. */
    if (strtoul(cmd->argv[1], &endp, 10) < 0)
      CONF_ERROR(cmd, "negative UID not allowed");

    if (endp && *endp)
      CONF_ERROR(cmd, "invalid UID parameter: not a number");
  }

  if (!radius_chk_var(cmd->argv[2])) {
    char *endp = NULL;

    /* Make sure it's a number, at least. */
    if (strtoul(cmd->argv[2], &endp, 10) < 0)
      CONF_ERROR(cmd, "negative GID not allowed");

    if (endp && *endp)
      CONF_ERROR(cmd, "invalid GID parameter: not a number");
  } 

  if (!radius_chk_var(cmd->argv[3])) {

    /* Make sure the path is absolute, at least. */
    if (*(cmd->argv[3]) != '/')
      CONF_ERROR(cmd, "home relative path not allowed");
  }

  if (!radius_chk_var(cmd->argv[4])) {

    /* Make sure the path is absolute, at least. */
    if (*(cmd->argv[4]) != '/')
      CONF_ERROR(cmd, "shell relative path not allowed");
  }

  add_config_param_str(cmd->argv[0], 4, cmd->argv[1], cmd->argv[2],
    cmd->argv[3], cmd->argv[4]);

  return PR_HANDLED(cmd);
}

/* usage: RadiusVendor name id */
MODRET set_radiusvendor(cmd_rec *cmd) {
  config_rec *c = NULL;
  long id = 0;
  char *tmp = NULL;

  CHECK_ARGS(cmd, 2);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  /* Make sure that the given vendor ID number is valid. */
  id = strtol(cmd->argv[2], &tmp, 10);

  if (tmp && *tmp)
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, ": vendor id '", cmd->argv[2],
      "' is not a valid number", NULL));

  if (id < 0)
    CONF_ERROR(cmd, "vendor id must be a positive number");

  c = add_config_param(cmd->argv[0], 2, NULL, NULL);
  c->argv[0] = pstrdup(c->pool, cmd->argv[1]);
  c->argv[1] = pcalloc(c->pool, sizeof(unsigned int));
  *((unsigned int *) c->argv[1]) = id;

  return PR_HANDLED(cmd);
}

/* Event handlers
 */

static void radius_exit_ev(const void *event_data, void *user_data) {

  if (!radius_stop_accting())
    radius_log("error: unable to stop accounting");

  radius_closelog();
  return;
}

#if defined(PR_SHARED_MODULE)
static void radius_mod_unload_ev(const void *event_data, void *user_data) {
  if (strcmp("mod_radius.c", (const char *) event_data) == 0) {
    pr_event_unregister(&radius_module, NULL, NULL);

    if (radius_pool) {
      destroy_pool(radius_pool);
      radius_pool = NULL;
    }

    close(radius_logfd);
    radius_logfd = -1;
    radius_logname = NULL;
  }
}
#endif /* PR_SHARED_MODULE */

static void radius_restart_ev(const void *event_data, void *user_data) {

  /* Re-allocate the pool used by this module. */
  if (radius_pool)
    destroy_pool(radius_pool);

  radius_pool = make_sub_pool(permanent_pool);
  pr_pool_tag(radius_pool, MOD_RADIUS_VERSION);

  return;
}

/* Initialization routines
 */

static int radius_sess_init(void) {
  int res = 0;
  config_rec *c = NULL;
  radius_server_t **current_server = NULL;

  res = radius_openlog();
  if (res < 0) {
    if (res == -1)
      pr_log_pri(PR_LOG_NOTICE, "notice: unable to open RadiusLog: %s",
        strerror(errno));

    else if (res == PR_LOG_WRITABLE_DIR)
      pr_log_pri(PR_LOG_NOTICE, "notice: unable to open RadiusLog: "
          "parent directory is world writeable");

    else if (res == PR_LOG_SYMLINK)
      pr_log_pri(PR_LOG_NOTICE, "notice: unable to open RadiusLog: "
          "cannot log to a symbolic link");
  }

  /* Is RadiusEngine on? */
  radius_engine = FALSE;
  c = find_config(main_server->conf, CONF_PARAM, "RadiusEngine", FALSE);
  if (c) {
    if (*((int *) c->argv[0]) == TRUE)
      radius_engine = TRUE;
  }

  if (!radius_engine) {
    radius_log("RadiusEngine not enabled");
    radius_closelog();
    return 0;
  }

  /* Initialize session variables */
  time(&radius_session_start);

  c = find_config(main_server->conf, CONF_PARAM, "RadiusNASIdentifier", FALSE);
  if (c) {
    radius_nas_identifier = c->argv[0];

    radius_log("RadiusNASIdentifier '%s' configured", radius_nas_identifier);
  }

  c = find_config(main_server->conf, CONF_PARAM, "RadiusVendor", FALSE);
  if (c) {
    radius_vendor_name = c->argv[0];
    radius_vendor_id = *((unsigned int *) c->argv[1]);

    radius_log("RadiusVendor '%s' (Vendor-Id %u) configured",
      radius_vendor_name, radius_vendor_id);
  }

  /* Find any configured RADIUS servers for this session */
  c = find_config(main_server->conf, CONF_PARAM, "RadiusAcctServer", FALSE);

  /* Point to the start of the accounting server list. */
  current_server = &radius_acct_server;

  while (c) {
    *current_server = *((radius_server_t **) c->argv[0]);
    current_server = &(*current_server)->next;

    c = find_config_next(c, c->next, CONF_PARAM, "RadiusAcctServer", FALSE);
  }

  if (!radius_acct_server)
    radius_log("notice: no configured RadiusAcctServers, no accounting");

  c = find_config(main_server->conf, CONF_PARAM, "RadiusAuthServer", FALSE);

  /* Point to the start of the authentication server list. */
  current_server = &radius_auth_server;

  while (c) {
    *current_server = *((radius_server_t **) c->argv[0]);
    current_server = &(*current_server)->next;

    c = find_config_next(c, c->next, CONF_PARAM, "RadiusAuthServer", FALSE);
  }

  if (!radius_auth_server)
    radius_log("notice: no configured RadiusAuthServers, no authentication");

  /* Prepare any configured fake user information. */
  c = find_config(main_server->conf, CONF_PARAM, "RadiusUserInfo", FALSE);
  if (c) {

    /* Process the parameter string stored in the found config_rec. */
    radius_process_user_info(c);

    /* Only use the faked information if authentication via RADIUS is
     * possible.  The radius_have_user_info flag will be set to
     * TRUE by radius_process_user_info(), unless there was some
     * illegal value.
     */
    if (!radius_auth_server)
      radius_have_user_info = FALSE;
  }

  /* If the RadiusUserInfo directive has not been set (or if it has been
   * set, but it was not well-formed), then we will be acting in a
   * "yes/no" style of authentication, similar to PAM.
   *
   * The Auth API tries to use the same module for authenticating a user
   * as the one which provided information for that user.  If we are not
   * providing user information, then we won't get a chance to authenticate
   * the user -- unless we disable that Auth API behavior.
   */
  if (!radius_have_user_info) {
    if (pr_auth_add_auth_only_module("mod_radius.c") < 0)
      pr_log_debug(DEBUG2, "error adding 'mod_radius.c' to auth-only module "
        "list: %s", strerror(errno));
  }

  /* Prepare any configured fake group information. */
  c = find_config(main_server->conf, CONF_PARAM, "RadiusGroupInfo", FALSE);
  if (c) {

    /* Process the parameter string stored in the found config_rec. */
    radius_process_group_info(c);

    /* Only use the faked information if authentication via RADIUS is
     * possible.  The radius_have_group_info flag will be set to
     * TRUE by radius_process_group_info(), unless there was some
     * illegal value.
     */
    if (!radius_auth_server)
      radius_have_group_info = FALSE;
  }

  /* Prepare any configure quota information. */
  c = find_config(main_server->conf, CONF_PARAM, "RadiusQuotaInfo", FALSE);
  if (c) {
    radius_process_quota_info(c);

    if (!radius_auth_server)
      radius_have_quota_info = FALSE;
  }

  /* Check for a configured RadiusRealm.  If present, use username + realm
   * in RADIUS packets as the user name, else just use the username.
   */
  radius_realm = get_param_ptr(main_server->conf, "RadiusRealm", FALSE);
  if (radius_realm)
    radius_log("using RadiusRealm '%s'", radius_realm);

  pr_event_register(&radius_module, "core.exit", radius_exit_ev, NULL);
  return 0;
}

static int radius_init(void) {

  /* Allocate a pool for this module's use. */
  radius_pool = make_sub_pool(permanent_pool);
  pr_pool_tag(radius_pool, MOD_RADIUS_VERSION);

#if defined(PR_SHARED_MODULE)
  pr_event_register(&radius_module, "core.module-unload",
    radius_mod_unload_ev, NULL);
#endif /* PR_SHARED_MODULE */

  /* Register a restart handler, to cleanup the pool. */
  pr_event_register(&radius_module, "core.restart", radius_restart_ev, NULL);

  return 0;
}

/* Module API tables
 */

static conftable radius_conftab[] = {
  { "RadiusAcctServer", 	set_radiusacctserver,	NULL },
  { "RadiusAuthServer", 	set_radiusauthserver,	NULL },
  { "RadiusEngine",		set_radiusengine,	NULL },
  { "RadiusGroupInfo",		set_radiusgroupinfo,	NULL },
  { "RadiusLog",		set_radiuslog,		NULL },
  { "RadiusNASIdentifier",	set_radiusnasidentifier,NULL },
  { "RadiusQuotaInfo",		set_radiusquotainfo,	NULL },
  { "RadiusRealm",		set_radiusrealm,	NULL },
  { "RadiusUserInfo",		set_radiususerinfo,	NULL },
  { "RadiusVendor",		set_radiusvendor,	NULL },
  { NULL }
};

static cmdtable radius_cmdtab[] = {
  { HOOK,		"radius_quota_lookup", G_NONE,
      radius_quota_lookup, FALSE, FALSE },

  { PRE_CMD,		C_PASS, G_NONE, radius_pre_pass,	FALSE, FALSE, CL_AUTH },
  { POST_CMD,		C_PASS, G_NONE, radius_post_pass, 	FALSE, FALSE, CL_AUTH },
  { POST_CMD_ERR,	C_PASS, G_NONE, radius_post_pass_err, 	FALSE, FALSE, CL_AUTH },
  { 0, NULL }
};

static authtable radius_authtab[] = {
  { 0, "setpwent",  radius_setpwent },
  { 0, "setgrent",  radius_setgrent },
  { 0, "endpwent",  radius_endpwent },
  { 0, "endgrent",  radius_endgrent },
  { 0, "getpwent",  radius_getpwent },
  { 0, "getgrent",  radius_getgrent },
  { 0, "getpwnam",  radius_getpwnam },
  { 0, "getgrnam",  radius_getgrnam },
  { 0, "getpwuid",  radius_getpwuid },
  { 0, "getgrgid",  radius_getgrgid },
  { 0, "getgroups", radius_getgroups },
  { 0, "auth",      radius_auth     },
  { 0, "check",     radius_check    },
  { 0, "uid2name",  radius_uid2name },
  { 0, "gid2name",  radius_gid2name },
  { 0, "name2uid",  radius_name2uid },
  { 0, "name2gid",  radius_name2gid },
  { 0, NULL }
};

module radius_module = {

  /* Always NULL */
  NULL, NULL,

  /* Module API version 2.0 */
  0x20,

  /* Module name */
  "radius",

  /* Module configuration handler table */
  radius_conftab,

  /* Module command handler table */
  radius_cmdtab,

  /* Module authentication handler table */
  radius_authtab,

  /* Module initialization function */
  radius_init,

  /* Module session initialization function */
  radius_sess_init,

  /* Module version */
  MOD_RADIUS_VERSION
};
