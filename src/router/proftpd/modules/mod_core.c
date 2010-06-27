/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 1997, 1998 Public Flood Software
 * Copyright (c) 1999, 2000 MacGyver aka Habeeb J. Dihu <macgyver@tos.net>
 * Copyright (c) 2001-2010 The ProFTPD Project team
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
 * As a special exemption, Public Flood Software/MacGyver aka Habeeb J. Dihu
 * and other respective copyright holders give permission to link this program
 * with OpenSSL, and distribute the resulting executable, without including
 * the source code for OpenSSL in the source distribution.
 */

/* Core FTPD module
 * $Id: mod_core.c,v 1.366 2010/02/23 18:01:03 castaglia Exp $
 */

#include "conf.h"
#include "privs.h"

#include <ctype.h>
#include <signal.h>

#ifdef HAVE_REGEX_H
# include <regex.h>
#endif

extern module site_module;
extern xaset_t *server_list;

/* From src/main.c */
extern unsigned long max_connects;
extern unsigned int max_connect_interval;

/* From modules/mod_site.c */
extern modret_t *site_dispatch(cmd_rec*);

/* For bytes-retrieving directives */
#define PR_BYTES_BAD_UNITS	-1
#define PR_BYTES_BAD_FORMAT	-2

module core_module;
char AddressCollisionCheck = TRUE;

static int core_scrub_timer_id;

static pr_fh_t *displayquit_fh = NULL;

#ifdef PR_USE_TRACE
static const char *trace_log = NULL;
#endif /* PR_USE_TRACE */

static ssize_t get_num_bytes(char *nbytes_str) {
  ssize_t nbytes = 0;
  unsigned long inb;
  char units, junk;
  int result;

  /* Scan in the given argument, checking for the leading number-of-bytes
   * as well as a trailing G, M, K, or B (case-insensitive).  The junk
   * variable is catch arguments like "2g2" or "number-letter-whatever".
   *
   * NOTE: There is no portable way to scan in an ssize_t, so we do unsigned
   * long and cast it.  This probably places a 32-bit limit on rlimit values.
   */
  if ((result = sscanf(nbytes_str, "%lu%c%c", &inb, &units, &junk)) == 2) {

    if (units != 'G' && units != 'g' &&
        units != 'M' && units != 'm' &&
        units != 'K' && units != 'k' &&
        units != 'B' && units != 'b')
      return PR_BYTES_BAD_UNITS;

    nbytes = (ssize_t)inb;

    /* Calculate the actual bytes, multiplying by the given units.  Doing
     * it this way means that <math.h> and -lm aren't required.
     */
    if (units == 'G' || units == 'g')
      nbytes *= (1024 * 1024 * 1024);

    if (units == 'M' || units == 'm')
      nbytes *= (1024 * 1024);

    if (units == 'K' || units == 'k')
      nbytes *= 1024;

    /* Silently ignore units of 'B' and 'b', as they don't affect
     * the requested number of bytes anyway.
     */

    /* NB: should we check for a maximum numeric value of calculated bytes?
     *  Probably not, as it varies (int to rlim_t) from platform to
     *  platform)...at least, not yet.
     */
    return nbytes;

  } else if (result == 1) {

    /* No units given.  Return the number of bytes as is. */
    return (ssize_t) inb;
  }

  /* Default return value: the given argument was badly formatted.
   */
  return PR_BYTES_BAD_FORMAT;
}

static int core_idle_timeout_cb(CALLBACK_FRAME) {
  /* We don't want to quit in the middle of a transfer */
  if (session.sf_flags & SF_XFER) { 
 
    /* Restart the timer. */
    return 1; 
  }
 
  pr_event_generate("core.timeout-idle", NULL);
 
  pr_response_send_async(R_421,
    _("Idle timeout (%d seconds): closing control connection"),
    pr_data_get_timeout(PR_DATA_TIMEOUT_IDLE));
  session_exit(PR_LOG_INFO, "Client session idle timeout, disconnected", 0,
    NULL);

  pr_timer_remove(PR_TIMER_LOGIN, ANY_MODULE);
  pr_timer_remove(PR_TIMER_NOXFER, ANY_MODULE);
  return 0;
}

static int core_scrub_scoreboard_cb(CALLBACK_FRAME) {

  /* Always return 1 when leaving this function, to make sure the timer
   * gets called again.
   */
  pr_scoreboard_scrub();

  return 1;
}

MODRET start_ifdefine(cmd_rec *cmd) {
  unsigned int ifdefine_ctx_count = 1;
  unsigned char not_define = FALSE, defined = FALSE;
  char buf[PR_TUNABLE_BUFFER_SIZE] = {'\0'}, *config_line = NULL;

  CHECK_ARGS(cmd, 1);

  if (*(cmd->argv[1]) == '!') {
    not_define = TRUE;
    (cmd->argv[1])++;
  }

  defined = pr_define_exists(cmd->argv[1]);

  /* Return now if we don't need to consume the <IfDefine> section
   * configuration lines.
   */
  if ((!not_define && defined) || (not_define && !defined)) {
    pr_log_debug(DEBUG3, "%s: using '%s%s' section at line %u", cmd->argv[0],
      not_define ? "!" : "", cmd->argv[1], pr_parser_get_lineno());
    return PR_HANDLED(cmd);

  } else
    pr_log_debug(DEBUG3, "%s: skipping '%s%s' section at line %u", cmd->argv[0],
      not_define ? "!" : "", cmd->argv[1], pr_parser_get_lineno());

  /* Rather than communicating with parse_config_file() via some global
   * variable/flag the need to skip configuration lines, if the requested
   * module condition is not TRUE, read in the lines here (effectively
   * preventing them from being parsed) up to and including the closing
   * directive.
   */
  while (ifdefine_ctx_count && (config_line = pr_parser_read_line(buf,
      sizeof(buf))) != NULL) {

    if (strncasecmp(config_line, "<IfDefine", 9) == 0)
      ifdefine_ctx_count++;

    if (strcasecmp(config_line, "</IfDefine>") == 0)
      ifdefine_ctx_count--;
  }

  /* If there are still unclosed <IfDefine> sections, signal an error.
   */
  if (ifdefine_ctx_count)
    CONF_ERROR(cmd, "unclosed <IfDefine> context");

  return PR_HANDLED(cmd);
}

/* As with Apache, there is no way of cleanly checking whether an
 * <IfDefine> section is properly closed.  Extra </IfDefine> directives
 * will be silently ignored.
 */
MODRET end_ifdefine(cmd_rec *cmd) {
  return PR_HANDLED(cmd);
}

MODRET start_ifmodule(cmd_rec *cmd) {
  unsigned int ifmodule_ctx_count = 1;
  unsigned char not_module = FALSE, found_module = FALSE;
  char buf[PR_TUNABLE_BUFFER_SIZE] = {'\0'}, *config_line = NULL;

  CHECK_ARGS(cmd, 1);

  if (*(cmd->argv[1]) == '!') {
    not_module = TRUE;
    (cmd->argv[1])++;
  }

  found_module = pr_module_exists(cmd->argv[1]);

  /* Return now if we don't need to consume the <IfModule> section
   * configuration lines.
   */
  if ((!not_module && found_module) || (not_module && !found_module)) {
    pr_log_debug(DEBUG3, "%s: using '%s%s' section at line %u", cmd->argv[0],
      not_module ? "!" : "", cmd->argv[1], pr_parser_get_lineno());
    return PR_HANDLED(cmd);

  } else {
    pr_log_debug(DEBUG3, "%s: skipping '%s%s' section at line %u", cmd->argv[0],
      not_module ? "!" : "", cmd->argv[1], pr_parser_get_lineno());
  }

  /* Rather than communicating with parse_config_file() via some global
   * variable/flag the need to skip configuration lines, if the requested
   * module condition is not TRUE, read in the lines here (effectively
   * preventing them from being parsed) up to and including the closing
   * directive.
   */
  while (ifmodule_ctx_count && (config_line = pr_parser_read_line(buf,
      sizeof(buf))) != NULL) {
    char *bufp;

    /* Advance past any leading whitespace. */
    for (bufp = config_line; *bufp && isspace((int) *bufp); bufp++);

    if (strncasecmp(bufp, "<IfModule", 9) == 0)
      ifmodule_ctx_count++;

    if (strcasecmp(bufp, "</IfModule>") == 0)
      ifmodule_ctx_count--;
  }

  /* If there are still unclosed <IfModule> sections, signal an error.
   */
  if (ifmodule_ctx_count)
    CONF_ERROR(cmd, "unclosed <IfModule> context");

  return PR_HANDLED(cmd);
}

/* As with Apache, there is no way of cleanly checking whether an
 * <IfModule> section is properly closed.  Extra </IfModule> directives
 * will be silently ignored.
 */
MODRET end_ifmodule(cmd_rec *cmd) {
  return PR_HANDLED(cmd);
}

/* Syntax: Define parameter
 *
 * Configuration file equivalent of the -D command-line option for
 * specifying an <IfDefine> value.
 *
 * It is suggested the RLimitMemory (a good idea to use anyway) be
 * used if this directive is present, to prevent Defines was being
 * used by a malicious local user in a .ftpaccess file.
 */
MODRET set_define(cmd_rec *cmd) {

  /* Make sure there's at least one parameter; any others are ignored */
  CHECK_ARGS(cmd, 1);

  /* This directive can occur in any context, so no need for the
   * CHECK_CONF macro.
   */

  pr_define_add(cmd->argv[1], FALSE);
  return PR_HANDLED(cmd);
}

MODRET add_include(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_ANON|CONF_GLOBAL|CONF_DIR);

  /* Make sure the given path is a valid path. */
  if (pr_fs_valid_path(cmd->argv[1]) < 0) {
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool,
      "unable to use path for configuration file '", cmd->argv[1], "'", NULL));
  }

  if (parse_config_path(cmd->tmp_pool, cmd->argv[1]) == -1) {
    if (errno != EINVAL)
      pr_log_pri(PR_LOG_WARNING, "warning: unable to include '%s': %s",
        cmd->argv[1], strerror(errno));

    else {
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "error including '", cmd->argv[1],
        "': ", strerror(errno), NULL));
    }
  }

  return PR_HANDLED(cmd);
}

MODRET set_debuglevel(cmd_rec *cmd) {
  config_rec *c = NULL;
  int debuglevel = -1;
  char *endp = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  /* Make sure the parameter is a valid number. */
  debuglevel = strtol(cmd->argv[1], &endp, 10);

  if (endp && *endp)
    CONF_ERROR(cmd, "not a valid number");

  /* Make sure the number is within the valid debug level range. */
  if (debuglevel < 0 || debuglevel > 10)
    CONF_ERROR(cmd, "invalid debug level configured");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = debuglevel;

  return PR_HANDLED(cmd);
}

MODRET set_defaultaddress(cmd_rec *cmd) {
  pr_netaddr_t *main_addr = NULL;
  array_header *addrs = NULL;

  if (cmd->argc-1 < 1)
    CONF_ERROR(cmd, "wrong number of parameters");
  CHECK_CONF(cmd, CONF_ROOT);

  main_addr = pr_netaddr_get_addr(main_server->pool, cmd->argv[1], &addrs);
  if (main_addr == NULL) 
    return PR_ERROR_MSG(cmd, NULL, pstrcat(cmd->tmp_pool,
      (cmd->argv)[0], ": unable to resolve \"", cmd->argv[1], "\"",
      NULL));

  main_server->ServerAddress = pr_netaddr_get_ipstr(main_addr);
  main_server->addr = main_addr;

  if (addrs) {
    register unsigned int i;
    pr_netaddr_t **elts = addrs->elts;

    /* For every additional address, implicitly add a bind record. */
    for (i = 0; i < addrs->nelts; i++) {
      const char *ipstr = pr_netaddr_get_ipstr(elts[i]);

#ifdef PR_USE_IPV6
      if (pr_netaddr_use_ipv6()) {
        char *ipbuf = pcalloc(cmd->tmp_pool, INET6_ADDRSTRLEN + 1);
        if (pr_netaddr_get_family(elts[i]) == AF_INET) {

          /* Create the bind record using the IPv4-mapped IPv6 version of
           * this address.
           */
          snprintf(ipbuf, INET6_ADDRSTRLEN, "::ffff:%s", ipstr);
          ipstr = ipbuf;
        }
      }
#endif /* PR_USE_IPV6 */

      add_config_param_str("_bind", 1, ipstr);
    }
  }

  /* Handle multiple addresses in a DefaultAddress directive.  We do
   * this by adding bind directives to the server_rec created for the
   * first address.
   */
  if (cmd->argc-1 > 1) {
    register unsigned int i;
    char *addrs_str = (char *) pr_netaddr_get_ipstr(main_addr);

    for (i = 2; i < cmd->argc; i++) {
      pr_netaddr_t *addr;
      addrs = NULL;

      addr = pr_netaddr_get_addr(cmd->tmp_pool, cmd->argv[i], &addrs);

      if (addr == NULL)
        CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "error resolving '",
          cmd->argv[i], "': ", strerror(errno), NULL));

      add_config_param_str("_bind", 1, pr_netaddr_get_ipstr(addr));

      addrs_str = pstrcat(cmd->tmp_pool, addrs_str, ", ",
        pr_netaddr_get_ipstr(addr), NULL);

      if (addrs) {
        register unsigned int j;
        pr_netaddr_t **elts = addrs->elts;

        /* For every additional address, implicitly add a bind record. */
        for (j = 0; j < addrs->nelts; j++)
          add_config_param_str("_bind", 1, pr_netaddr_get_ipstr(elts[j]));
      }
    }

    pr_log_pri(PR_LOG_INFO, "setting default addresses to %s", addrs_str);

  } else {
    pr_log_pri(PR_LOG_INFO, "setting default address to %s",
      pr_netaddr_get_ipstr(main_addr));
  }

  return PR_HANDLED(cmd);
}

MODRET set_servername(cmd_rec *cmd) {
  server_rec *s = cmd->server;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL);

  s->ServerName = pstrdup(s->pool,cmd->argv[1]);
  return PR_HANDLED(cmd);
}

MODRET set_servertype(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT);

  if (strcasecmp(cmd->argv[1], "inetd") == 0)
    ServerType = SERVER_INETD;

  else if (strcasecmp(cmd->argv[1], "standalone") == 0)
    ServerType = SERVER_STANDALONE;

  else
    CONF_ERROR(cmd,"type must be either 'inetd' or 'standalone'");

  return PR_HANDLED(cmd);
}

MODRET set_setenv(cmd_rec *cmd) {
  int ctxt_type;

  CHECK_ARGS(cmd, 2);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  add_config_param_str(cmd->argv[0], 2, cmd->argv[1], cmd->argv[2]);

  /* In addition, if this is the "server config" context, set the
   * environ variable now.  If there was a <Daemon> context, that would
   * be a more appropriate place for configuring parse-time environ
   * variables.
   */
  ctxt_type = (cmd->config && cmd->config->config_type != CONF_PARAM ?
     cmd->config->config_type : cmd->server->config_type ?
     cmd->server->config_type : CONF_ROOT);

  if (ctxt_type == CONF_ROOT) {
    if (pr_env_set(cmd->server->pool, cmd->argv[1], cmd->argv[2]) < 0) {
      pr_log_debug(DEBUG1, "%s: unable to set environ variable '%s': %s",
        cmd->argv[0], cmd->argv[1], strerror(errno));
    }
  }

  return PR_HANDLED(cmd);
}

MODRET add_transferlog(cmd_rec *cmd) {
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  c = add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

MODRET set_wtmplog(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  if (strcasecmp(cmd->argv[1], "NONE") == 0)
    bool = 0;
  else
    bool = get_boolean(cmd, 1);

  if (bool != -1) {
    c = add_config_param(cmd->argv[0], 1, NULL);
    c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
    *((unsigned char *) c->argv[0]) = bool;
    c->flags |= CF_MERGEDOWN;

  } else
    CONF_ERROR(cmd, "expected boolean argument, or \"NONE\"");

  return PR_HANDLED(cmd);
}

MODRET set_serveradmin(cmd_rec *cmd) {
  server_rec *s = cmd->server;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL);

  s->ServerAdmin = pstrdup(s->pool, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

/* usage: UseIPv6 on|off */
MODRET set_useipv6(cmd_rec *cmd) {
#ifdef PR_USE_IPV6
  int bool = -1;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  if (bool == 0) {
    pr_log_debug(DEBUG2, "disabling runtime support for IPv6 connections");
    pr_netaddr_disable_ipv6();

  } else {
    pr_netaddr_enable_ipv6();
  }

  return PR_HANDLED(cmd);
#else
  CONF_ERROR(cmd,
    "Use of the UseIPv6 directive requires IPv6 support (--enable-ipv6)");
#endif /* PR_USE_IPV6 */
}

MODRET set_usereversedns(cmd_rec *cmd) {
  int bool = -1;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  ServerUseReverseDNS = bool;
  pr_netaddr_set_reverse_dns(bool);

  return PR_HANDLED(cmd);
}

MODRET set_satisfy(cmd_rec *cmd) {
  int satisfy = -1;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_CLASS);

  if (strcasecmp(cmd->argv[1], "any") == 0)
    satisfy = PR_CLASS_SATISFY_ANY;

  else if (strcasecmp(cmd->argv[1], "all") == 0)
    satisfy = PR_CLASS_SATISFY_ALL;

  else
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "invalid parameter: '",
      cmd->argv[1], "'", NULL));

  if (pr_class_set_satisfy(satisfy) < 0)
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "error setting Satisfy: ",
      strerror(errno), NULL));

  return PR_HANDLED(cmd);
}

MODRET set_scoreboardfile(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT);

  if (pr_set_scoreboard(cmd->argv[1]) < 0)
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, ": unable to use '",
      cmd->argv[1], "': ", strerror(errno), NULL));

  return PR_HANDLED(cmd);
}

/* usage: ScoreboardScrub "on"|"off"|secs */
MODRET set_scoreboardscrub(cmd_rec *cmd) {
  int bool = -1, nsecs = 0;
  config_rec *c;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT);
 
  bool = get_boolean(cmd, 1);
  if (bool == -1) {
    /* If this is the case, try handling the parameter as the number of
     * seconds, as the scrub frequency.
     */
    nsecs = atoi(cmd->argv[1]);
    if (nsecs <= 0) {
      CONF_ERROR(cmd, "number must be greater than zero");
    }
  }

  if (nsecs > 0) {
    c = add_config_param(cmd->argv[0], 2, NULL, NULL);
    c->argv[0] = pcalloc(c->pool, sizeof(int));
    *((int *) c->argv[0]) = TRUE;
    c->argv[1] = pcalloc(c->pool, sizeof(int));
    *((int *) c->argv[1]) = nsecs;

  } else {
    c = add_config_param(cmd->argv[0], 1, NULL);
    c->argv[0] = pcalloc(c->pool, sizeof(int));
    *((int *) c->argv[0]) = bool;
  }

  return PR_HANDLED(cmd);
}

MODRET set_serverport(cmd_rec *cmd) {
  server_rec *s = cmd->server;
  int port;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL);

  port = atoi(cmd->argv[1]);
  if (port < 0 || port > 65535)
    CONF_ERROR(cmd,"value must be between 0 and 65535");

  s->ServerPort = port;
  return PR_HANDLED(cmd);
}

MODRET set_pidfile(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT);

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

MODRET set_sysloglevel(cmd_rec *cmd) {
  config_rec *c = NULL;
  int level = 0;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  level = pr_log_str2sysloglevel(cmd->argv[1]);
  if (level < 0)
    CONF_ERROR(cmd, "SyslogLevel requires level keyword: one of "
      "emerg/alert/crit/error/warn/notice/info/debug");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned int));
  *((unsigned int *) c->argv[0]) = level;

  return PR_HANDLED(cmd);
}

MODRET set_serverident(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;

  if (cmd->argc < 2 || cmd->argc > 3)
    CONF_ERROR(cmd, "wrong number of parameters");

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  if (bool && cmd->argc == 3) {
    c = add_config_param(cmd->argv[0], 2, NULL, NULL);
    c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
    *((unsigned char *) c->argv[0]) = !bool;
    c->argv[1] = pstrdup(c->pool, cmd->argv[2]);

  } else {

    c = add_config_param(cmd->argv[0], 1, NULL);
    c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
    *((unsigned char *) c->argv[0]) = !bool;
  }

  return PR_HANDLED(cmd);
}

MODRET set_defaultserver(cmd_rec *cmd) {
  int bool = -1;
  server_rec *s = NULL;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  if (!bool)
    return PR_HANDLED(cmd);

  /* DefaultServer is not allowed if already set somewhere */
  for (s = (server_rec *) server_list->xas_list; s; s = s->next)
    if (find_config(s->conf, CONF_PARAM, cmd->argv[0], FALSE))
      CONF_ERROR(cmd, "DefaultServer has already been set");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = bool;

  return PR_HANDLED(cmd);
}

MODRET set_masqueradeaddress(cmd_rec *cmd) {
  config_rec *c = NULL;
  pr_netaddr_t *masq_addr = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL);

  /* We can only masquerade as one address, so we don't need to know if the
   * given name might map to multiple addresses.
   */
  masq_addr = pr_netaddr_get_addr(cmd->server->pool, cmd->argv[1], NULL);
  if (masq_addr == NULL)
    return PR_ERROR_MSG(cmd, NULL, pstrcat(cmd->tmp_pool, cmd->argv[0],
      ": unable to resolve \"", cmd->argv[1], "\"", NULL));

  c = add_config_param(cmd->argv[0], 2, (void *) masq_addr, NULL);
  c->argv[1] = pstrdup(c->pool, cmd->argv[1]);

  return PR_HANDLED(cmd);
}

MODRET set_maxinstances(cmd_rec *cmd) {
  int max;
  char *endp;

  CHECK_ARGS(cmd,1);
  CHECK_CONF(cmd,CONF_ROOT);

  if (strcasecmp(cmd->argv[1], "none") == 0)
    max = 0;

  else {
    max = (int)strtol(cmd->argv[1], &endp, 10);

    if ((endp && *endp) || max < 1)
      CONF_ERROR(cmd, "argument must be 'none' or a number greater than 0");
  }

  ServerMaxInstances = max;
  return PR_HANDLED(cmd);
}

/* usage: MaxConnectionRate rate [interval] */
MODRET set_maxconnrate(cmd_rec *cmd) {
  long conn_max = 0L;
  char *endp = NULL;

  if (cmd->argc-1 < 1 || cmd->argc-1 > 2)
    CONF_ERROR(cmd, "wrong number of parameters");
  CHECK_CONF(cmd, CONF_ROOT);

  conn_max = strtol(cmd->argv[1], &endp, 10);

  if (endp && *endp)
    CONF_ERROR(cmd, "invalid connection rate");

  if (conn_max < 0)
    CONF_ERROR(cmd, "connection rate must be positive");

  max_connects = conn_max;

  /* If the optional interval parameter is given, parse it. */
  if (cmd->argc-1 == 2) {
    max_connect_interval = atoi(cmd->argv[2]);

    if (max_connect_interval < 1)
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool,
        ": interval must be greater than zero", NULL));
  }

  return PR_HANDLED(cmd);
}

MODRET set_timeoutidle(cmd_rec *cmd) {
  int timeout = -1;
  char *endp = NULL;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  timeout = (int) strtol(cmd->argv[1], &endp, 10);

  if ((endp && *endp) || timeout < 0 || timeout > 65535)
    CONF_ERROR(cmd, "timeout values must be between 0 and 65535");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = timeout;
  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

MODRET set_timeoutlinger(cmd_rec *cmd) {
  long timeout = -1;
  char *endp = NULL;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  timeout = strtol(cmd->argv[1], &endp, 10);

  if ((endp && *endp) || timeout < 0 || timeout > 65535)
    CONF_ERROR(cmd, "timeout values must be between 0 and 65535");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(long));
  *((long *) c->argv[0]) = timeout;

  return PR_HANDLED(cmd);
}

MODRET set_socketbindtight(cmd_rec *cmd) {
  int bool = -1;
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  SocketBindTight = bool;
  return PR_HANDLED(cmd);
}

/* NOTE: at some point in the future, SocketBindTight should be folded
 * into this SocketOptions directive handler.
 */
MODRET set_socketoptions(cmd_rec *cmd) {
  register unsigned int i = 0;

  /* Make sure we have the right number of parameters. */
  if ((cmd->argc-1) % 2 != 0)
   CONF_ERROR(cmd, "bad number of parameters");

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL);

  for (i = 1; i < cmd->argc; i++) {
    int value = 0;

    if (strcasecmp(cmd->argv[i], "maxseg") == 0) {
      value = atoi(cmd->argv[++i]);

      /* As per the tcp(7) man page, sizes larger than the interface MTU
       * will be ignored, and will have no effect.
       */

      if (value < 0)
        CONF_ERROR(cmd, "maxseg size must be greater than 0");

      cmd->server->tcp_mss_len = value;

    } else if (strcasecmp(cmd->argv[i], "rcvbuf") == 0) {
      value = atoi(cmd->argv[++i]);

      if (value < 1024)
        CONF_ERROR(cmd, "rcvbuf size must be greater than or equal to 1024");

      cmd->server->tcp_rcvbuf_len = value;
      cmd->server->tcp_rcvbuf_override = TRUE;

    } else if (strcasecmp(cmd->argv[i], "sndbuf") == 0) {
      value = atoi(cmd->argv[++i]);

      if (value < 1024)
        CONF_ERROR(cmd, "sndbuf size must be greater than or equal to 1024");

      cmd->server->tcp_sndbuf_len = value;
      cmd->server->tcp_sndbuf_override = TRUE;

    } else {
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, ": unknown socket option: '",
        cmd->argv[i], "'", NULL));
    }
  }

  return PR_HANDLED(cmd);
}

MODRET set_multilinerfc2228(cmd_rec *cmd) {
  int bool = -1;
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  MultilineRFC2228 = bool;
  return PR_HANDLED(cmd);
}

MODRET set_tcpbacklog(cmd_rec *cmd) {
  int backlog;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT);

  backlog = atoi(cmd->argv[1]);

  if (backlog < 1 || backlog > 255)
    CONF_ERROR(cmd, "parameter must be a number between 1 and 255");

  tcpBackLog = backlog;
  return PR_HANDLED(cmd);
}

MODRET set_tcpnodelay(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = bool;

  return PR_HANDLED(cmd);
}

MODRET set_user(cmd_rec *cmd) {
  struct passwd *pw = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  /* 1.1.7, no longer force user/group lookup inside <Anonymous>
   * it's now defered until authentication occurs.
   */

  if (!cmd->config || cmd->config->config_type != CONF_ANON) {
    pw = pr_auth_getpwnam(cmd->tmp_pool, cmd->argv[1]);
    if (pw == NULL) {
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "Unknown user '",
        cmd->argv[1], "'", NULL));
    }
  }

  if (pw) {
    config_rec *c = add_config_param("UserID", 1, NULL);
    c->argv[0] = pcalloc(c->pool, sizeof(uid_t));
    *((uid_t *) c->argv[0]) = pw->pw_uid;
  }

  add_config_param_str("UserName", 1, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

MODRET add_from(cmd_rec *cmd) {
  int cargc;
  char **cargv;

  CHECK_CONF(cmd, CONF_CLASS);

  cargc = cmd->argc-1;
  cargv = cmd->argv;

  while (cargc && *(cargv + 1)) {
    if (strcasecmp("all", *(cargv + 1)) == 0 ||
        strcasecmp("none", *(cargv + 1)) == 0) {
      pr_netacl_t *acl = pr_netacl_create(cmd->tmp_pool, *(cargv + 1));
      if (acl == NULL) {
        CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "bad ACL definition '",
          *(cargv + 1), "': ", strerror(errno), NULL));
      }

      pr_trace_msg("netacl", 9, "'%s' parsed into netacl '%s'", *(cargv + 1),
        pr_netacl_get_str(cmd->tmp_pool, acl));

      if (pr_class_add_acl(acl) < 0) {
        CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "error adding rule '",
          *(cargv + 1), "': ", strerror(errno), NULL));
      }

      cargc = 0;
    }

    break;
  }

  /* Parse each parameter into a netacl. */
  while (cargc-- && *(++cargv)) {
    char *ent = NULL;
    char *str = pstrdup(cmd->tmp_pool, *cargv);

    while ((ent = pr_str_get_token(&str, ",")) != NULL) {
      if (*ent) {
        pr_netacl_t *acl;

        if (strcasecmp(ent, "all") == 0 ||
            strcasecmp(ent, "none") == 0) {
           cargc = 0;
           break;
         }

        acl = pr_netacl_create(cmd->tmp_pool, ent);
        if (acl == NULL) {
          CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "bad ACL definition '",
            *(cargv + 1), "': ", strerror(errno), NULL));
        }

        pr_trace_msg("netacl", 9, "'%s' parsed into netacl '%s'", ent,
          pr_netacl_get_str(cmd->tmp_pool, acl));

        if (pr_class_add_acl(acl) < 0) {
          CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "error adding rule '", ent,
            "': ", strerror(errno), NULL));
        }
      }
    }
  }

  return PR_HANDLED(cmd);
}

MODRET set_group(cmd_rec *cmd) {
  struct group *grp = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  if (!cmd->config || cmd->config->config_type != CONF_ANON) {
    grp = pr_auth_getgrnam(cmd->tmp_pool, cmd->argv[1]);
    if (grp == NULL) {
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "Unknown group '",
        cmd->argv[1], "'", NULL));
    }
  }

  if (grp) {
    config_rec *c = add_config_param("GroupID", 1, NULL);
    c->argv[0] = pcalloc(c->pool, sizeof(gid_t));
    *((gid_t *) c->argv[0]) = grp->gr_gid;
  }

  add_config_param_str("GroupName", 1, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

/* usage: Trace channel1:level1 ... */
MODRET set_trace(cmd_rec *cmd) {
#ifdef PR_USE_TRACE
  register unsigned int i;

  if (cmd->argc-1 < 1)
    CONF_ERROR(cmd, "wrong number of parameters");
  CHECK_CONF(cmd, CONF_ROOT);

  for (i = 1; i < cmd->argc; i++) {
    char *channel, *tmp;
    int level;

    tmp = strchr(cmd->argv[i], ':');
    if (tmp == NULL) {
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "badly formatted parameter: '",
        cmd->argv[i], "'", NULL));
    }

    channel = cmd->argv[i];
    *tmp = '\0';
    level = atoi(++tmp);

    if (pr_trace_set_level(channel, level) < 0) {
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "error setting level ", tmp,
        " for channel '", channel, "': ", strerror(errno), NULL));
    }
  }

  return PR_HANDLED(cmd);
#else
  CONF_ERROR(cmd,
    "Use of the Trace directive requires trace support (--enable-trace)");
#endif /* PR_USE_TRACE */
}

/* usage: TraceLog path */
MODRET set_tracelog(cmd_rec *cmd) {
#ifdef PR_USE_TRACE
  if (cmd->argc-1 != 1)
    CONF_ERROR(cmd, "wrong number of parameters");
  CHECK_CONF(cmd, CONF_ROOT);

  if (pr_fs_valid_path(cmd->argv[1]) < 0)
    CONF_ERROR(cmd, "must be an absolute path");

  trace_log = pstrdup(cmd->server->pool, cmd->argv[1]);
  if (pr_trace_set_file(trace_log) < 0) {
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "error using TraceLog '",
      trace_log, "': ", strerror(errno), NULL));
  }

  return PR_HANDLED(cmd);
#else
  CONF_ERROR(cmd,
    "Use of the TraceLog directive requires trace support (--enable-trace)");
#endif /* PR_USE_TRACE */
}

MODRET set_umask(cmd_rec *cmd) {
  config_rec *c;
  char *endp;
  mode_t tmp_umask;

  CHECK_VARARGS(cmd, 1, 2);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON|
    CONF_DIR|CONF_DYNDIR);

  tmp_umask = (mode_t) strtol(cmd->argv[1], &endp, 8);

  if (endp && *endp)
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "'", cmd->argv[1],
      "' is not a valid umask", NULL));

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(mode_t));
  *((mode_t *) c->argv[0]) = tmp_umask;
  c->flags |= CF_MERGEDOWN;

  /* Have we specified a directory umask as well?
   */
  if (CHECK_HASARGS(cmd, 2)) {

    /* allocate space for another mode_t.  Don't worry -- the previous
     * pointer was recorded in the Umask config_rec
     */
    tmp_umask = (mode_t) strtol(cmd->argv[2], &endp, 8);

    if (endp && *endp)
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "'", cmd->argv[2],
        "' is not a valid umask", NULL));

    c = add_config_param("DirUmask", 1, NULL);
    c->argv[0] = pcalloc(c->pool, sizeof(mode_t));
    *((mode_t *) c->argv[0]) = tmp_umask;
    c->flags |= CF_MERGEDOWN;
  }

  return PR_HANDLED(cmd);
}

MODRET set_unsetenv(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]); 
  return PR_HANDLED(cmd);
}

MODRET set_rlimitcpu(cmd_rec *cmd) {
#ifdef RLIMIT_CPU
  /* Make sure the directive has between 1 and 3 parameters */
  if (cmd->argc-1 < 1 || cmd->argc-1 > 3)
    CONF_ERROR(cmd, "wrong number of parameters");

  /* The context check for this directive depends on the first parameter.
   * For backwards compatibility, this parameter may be a number, or it
   * may be "daemon", "session", or "none".  If it happens to be
   * "daemon", then this directive should be in the CONF_ROOT context only.
   * Otherwise, it can appear in the full range of server contexts.
   */

  if (strcmp(cmd->argv[1], "daemon") == 0) {
    CHECK_CONF(cmd, CONF_ROOT);

  } else {
    CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);
  }

  /* Handle the newer format, which uses "daemon" or "session" or "none"
   * as the first parameter.
   */
  if (strcmp(cmd->argv[1], "daemon") == 0 ||
      strcmp(cmd->argv[1], "session") == 0) {
    config_rec *c = NULL;
    struct rlimit *rlim = pcalloc(cmd->server->pool, sizeof(struct rlimit));

    /* Retrieve the current values */
    if (getrlimit(RLIMIT_CPU, rlim) < 0)
      pr_log_pri(PR_LOG_ERR, "error: getrlimit(RLIMIT_CPU): %s",
        strerror(errno));

    if (strcasecmp("max", cmd->argv[2]) == 0) {
      rlim->rlim_cur = RLIM_INFINITY;

    } else {

      /* Check that the non-max argument is a number, and error out if not.
       */
      char *tmp = NULL;
      unsigned long num = strtoul(cmd->argv[2], &tmp, 10);

      if (tmp && *tmp)
        CONF_ERROR(cmd, "badly formatted argument");

      rlim->rlim_cur = num;
    }

    /* Handle the optional "hard limit" parameter, if present. */
    if (cmd->argc-1 == 3) {
      if (strcasecmp("max", cmd->argv[3]) == 0) {
        rlim->rlim_max = RLIM_INFINITY;

      } else {

        /* Check that the non-max argument is a number, and error out if not.
         */
        char *tmp = NULL;
        unsigned long num = strtoul(cmd->argv[3], &tmp, 10);

        if (tmp && *tmp)
          CONF_ERROR(cmd, "badly formatted argument");

        rlim->rlim_max = num;
      }
    }

    c = add_config_param(cmd->argv[0], 2, (void *) rlim, NULL);
    c->argv[1] = pstrdup(c->pool, cmd->argv[1]);

  /* Handle the older format, which will have a number as the first
   * parameter.
   */
  } else {
    struct rlimit *rlim = pcalloc(cmd->server->pool, sizeof(struct rlimit));

    /* Retrieve the current values */
    if (getrlimit(RLIMIT_CPU, rlim) < 0)
      pr_log_pri(PR_LOG_ERR, "error: getrlimit(RLIMIT_CPU): %s",
        strerror(errno));

    if (strcasecmp("max", cmd->argv[1]) == 0) {
      rlim->rlim_cur = RLIM_INFINITY;

    } else {

      /* Check that the non-max argument is a number, and error out if not.
       */
      char *tmp = NULL;
      long num = strtol(cmd->argv[1], &tmp, 10);

      if (tmp && *tmp)
        CONF_ERROR(cmd, "badly formatted argument");

      rlim->rlim_cur = num;
    }

    /* Handle the optional "hard limit" parameter, if present. */
    if (cmd->argc-1 == 2) {
      if (strcasecmp("max", cmd->argv[2]) == 0) {
        rlim->rlim_max = RLIM_INFINITY;

      } else {

        /* Check that the non-max argument is a number, and error out if not.
         */
        char *tmp = NULL;
        long num = strtol(cmd->argv[2], &tmp, 10);

        if (tmp && *tmp)
          CONF_ERROR(cmd, "badly formatted argument");

        rlim->rlim_max = num;
      }
    }

    add_config_param(cmd->argv[0], 2, (void *) rlim, NULL);
  }

  return PR_HANDLED(cmd);
#else
  CONF_ERROR(cmd, "RLimitCPU is not supported on this platform");
#endif
}

MODRET set_rlimitmemory(cmd_rec *cmd) {
#if defined(RLIMIT_DATA) || defined(RLIMIT_AS) || defined(RLIMIT_VMEM)
  /* Make sure the directive has between 1 and 3 parameters */
  if (cmd->argc-1 < 1 || cmd->argc-1 > 3)
    CONF_ERROR(cmd, "wrong number of parameters");

  /* The context check for this directive depends on the first parameter.
   * For backwards compatibility, this parameter may be a number, or it
   * may be "daemon", "session", or "none".  If it happens to be
   * "daemon", then this directive should be in the CONF_ROOT context only.
   * Otherwise, it can appear in the full range of server contexts.
   */

  if (strcmp(cmd->argv[1], "daemon") == 0) {
    CHECK_CONF(cmd, CONF_ROOT);

  } else {
    CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);
  }

  /* Handle the newer format, which uses "daemon" or "session" or "none"
   * as the first parameter.
   */
  if (strcmp(cmd->argv[1], "daemon") == 0 ||
      strcmp(cmd->argv[1], "session") == 0) {
    config_rec *c = NULL;
    struct rlimit *rlim = pcalloc(cmd->server->pool, sizeof(struct rlimit));

    /* Retrieve the current values */
#if defined(RLIMIT_DATA)
    if (getrlimit(RLIMIT_DATA, rlim) == -1)
      pr_log_pri(PR_LOG_ERR, "error: getrlimit(RLIMIT_DATA): %s",
        strerror(errno));
#elif defined(RLIMIT_AS)
    if (getrlimit(RLIMIT_AS, rlim) == -1)
      pr_log_pri(PR_LOG_ERR, "error: getrlimit(RLIMIT_AS): %s",
        strerror(errno));
#elif defined(RLIMIT_VMEM)
    if (getrlimit(RLIMIT_VMEM, rlim) == -1)
      pr_log_pri(PR_LOG_ERR, "error: getrlimit(RLIMIT_VMEM): %s",
        strerror(errno));
#endif

    if (strcasecmp("max", cmd->argv[2]) == 0) {
      rlim->rlim_cur = RLIM_INFINITY;

    } else {
      rlim->rlim_cur = get_num_bytes(cmd->argv[2]);
    }

    /* Check for bad return values. */
    if (rlim->rlim_cur == PR_BYTES_BAD_UNITS)
      CONF_ERROR(cmd, "unknown units used");

    if (rlim->rlim_cur == PR_BYTES_BAD_FORMAT)
      CONF_ERROR(cmd, "badly formatted parameter");

    /* Handle the optional "hard limit" parameter, if present. */
    if (cmd->argc-1 == 3) {
      if (strcasecmp("max", cmd->argv[3]) == 0) {
        rlim->rlim_max = RLIM_INFINITY;

      } else {
        rlim->rlim_max = get_num_bytes(cmd->argv[3]);
      }

      /* Check for bad return values. */
      if (rlim->rlim_max == PR_BYTES_BAD_UNITS)
        CONF_ERROR(cmd, "unknown units used");

      if (rlim->rlim_max == PR_BYTES_BAD_FORMAT)
        CONF_ERROR(cmd, "badly formatted parameter");
    }

    c = add_config_param(cmd->argv[0], 2, (void *) rlim, NULL);
    c->argv[1] = pstrdup(c->pool, cmd->argv[1]);

  /* Handle the older format, which will have a number as the first
   * parameter.
   */
  } else {
    struct rlimit *rlim = pcalloc(cmd->server->pool, sizeof(struct rlimit));

    /* Retrieve the current values */
#if defined(RLIMIT_DATA)
    if (getrlimit(RLIMIT_DATA, rlim) < 0)
      pr_log_pri(PR_LOG_ERR, "error: getrlimit(RLIMIT_DATA): %s",
        strerror(errno));
#elif defined(RLIMIT_AS)
    if (getrlimit(RLIMIT_AS, rlim) < 0)
      pr_log_pri(PR_LOG_ERR, "error: getrlimit(RLIMIT_AS): %s",
        strerror(errno));
#elif defined(RLIMIT_VMEM)
    if (getrlimit(RLIMIT_VMEM, rlim) < 0)
      pr_log_pri(PR_LOG_ERR, "error: getrlimit(RLIMIT_VMEM): %s",
        strerror(errno));
#endif

    if (strcasecmp("max", cmd->argv[1]) == 0) {
      rlim->rlim_cur = RLIM_INFINITY;

    } else {
      rlim->rlim_cur = get_num_bytes(cmd->argv[1]);
    }

    /* Check for bad return values. */
    if (rlim->rlim_cur == PR_BYTES_BAD_UNITS)
      CONF_ERROR(cmd, "unknown units used");

    if (rlim->rlim_cur == PR_BYTES_BAD_FORMAT)
      CONF_ERROR(cmd, "badly formatted parameter");

    /* Handle the optional "hard limit" parameter, if present. */
    if (cmd->argc-1 == 2) {
      if (strcasecmp("max", cmd->argv[2]) == 0) {
        rlim->rlim_max = RLIM_INFINITY;

      } else {
        rlim->rlim_max = get_num_bytes(cmd->argv[2]);
      }

      /* Check for bad return values. */
      if (rlim->rlim_max == PR_BYTES_BAD_UNITS)
        CONF_ERROR(cmd, "unknown units used");

      if (rlim->rlim_max == PR_BYTES_BAD_FORMAT)
        CONF_ERROR(cmd, "badly formatted parameter");
    }

    add_config_param(cmd->argv[0], 2, (void *) rlim, NULL);
  }

  return PR_HANDLED(cmd);
#else
  CONF_ERROR(cmd, "RLimitMemory is not supported on this platform");
#endif
}

MODRET set_rlimitopenfiles(cmd_rec *cmd) {
#if defined(RLIMIT_NOFILE) || defined(RLIMIT_OFILE)
  /* Make sure the directive has between 1 and 3 parameters */
  if (cmd->argc-1 < 1 || cmd->argc-1 > 3)
    CONF_ERROR(cmd, "wrong number of parameters");

  /* The context check for this directive depends on the first parameter.
   * For backwards compatibility, this parameter may be a number, or it
   * may be "daemon", "session", or "none".  If it happens to be
   * "daemon", then this directive should be in the CONF_ROOT context only.
   * Otherwise, it can appear in the full range of server contexts.
   */

  if (strcmp(cmd->argv[1], "daemon") == 0) {
    CHECK_CONF(cmd, CONF_ROOT);

  } else {
    CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);
  }

  /* Handle the newer format, which uses "daemon" or "session" or "none"
   * as the first parameter.
   */
  if (strcmp(cmd->argv[1], "daemon") == 0 ||
      strcmp(cmd->argv[1], "session") == 0) {
    config_rec *c = NULL;
    struct rlimit *rlim = pcalloc(cmd->server->pool, sizeof(struct rlimit));

    /* Retrieve the current values */
#if defined(RLIMIT_NOFILE)
    if (getrlimit(RLIMIT_NOFILE, rlim) < 0)
      pr_log_pri(PR_LOG_ERR, "error: getrlimit(RLIMIT_NOFILE): %s",
        strerror(errno));
#elif defined(RLIMIT_OFILE)
    if (getrlimit(RLIMIT_OFILE, rlim) < 0)
      pr_log_pri(PR_LOG_ERR, "error: getrlimit(RLIMIT_OFILE): %s",
        strerror(errno));
#endif

    if (strcasecmp("max", cmd->argv[2]) == 0) {
      rlim->rlim_cur = sysconf(_SC_OPEN_MAX);

    } else {

      /* Check that the non-max argument is a number, and error out if not.
       */
      char *tmp = NULL;
      long num = strtol(cmd->argv[2], &tmp, 10);

      if (tmp && *tmp)
        CONF_ERROR(cmd, "badly formatted argument");

      rlim->rlim_cur = num;
    }

    /* Handle the optional "hard limit" parameter, if present. */
    if (cmd->argc-1 == 3) {
      if (strcasecmp("max", cmd->argv[3]) == 0) {
        rlim->rlim_max = sysconf(_SC_OPEN_MAX);

      } else {

        /* Check that the non-max argument is a number, and error out if not.
         */
        char *tmp = NULL;
        long num = strtol(cmd->argv[3], &tmp, 10);

        if (tmp && *tmp)
          CONF_ERROR(cmd, "badly formatted argument");

        rlim->rlim_max = num;
      }
    }

    c = add_config_param(cmd->argv[0], 2, (void *) rlim, NULL);
    c->argv[1] = pstrdup(c->pool, cmd->argv[1]);

  /* Handle the older format, which will have a number as the first
   * parameter.
   */
  } else {
    struct rlimit *rlim = pcalloc(cmd->server->pool, sizeof(struct rlimit));

    /* Retrieve the current values */
#if defined(RLIMIT_NOFILE)
    if (getrlimit(RLIMIT_NOFILE, rlim) < 0)
      pr_log_pri(PR_LOG_ERR, "error: getrlimit(RLIMIT_NOFILE): %s",
        strerror(errno));
#elif defined(RLIMIT_OFILE)
    if (getrlimit(RLIMIT_OFILE, rlim) < 0)
      pr_log_pri(PR_LOG_ERR, "error: getrlimit(RLIMIT_OFILE): %s",
        strerror(errno));
#endif

    if (strcasecmp("max", cmd->argv[1]) == 0) {
      rlim->rlim_cur = sysconf(_SC_OPEN_MAX);

    } else {

      /* Check that the non-max argument is a number, and error out if not.
       */
      char *tmp = NULL;
      long num = strtol(cmd->argv[1], &tmp, 10);

      if (tmp && *tmp)
        CONF_ERROR(cmd, "badly formatted argument");

      rlim->rlim_cur = num;
    }

    /* Handle the optional "hard limit" parameter, if present. */
    if (cmd->argc-1 == 2) {
      if (strcasecmp("max", cmd->argv[2]) == 0) {
        rlim->rlim_max = sysconf(_SC_OPEN_MAX);

      } else {

        /* Check that the non-max argument is a number, and error out if not.
         */
        char *tmp = NULL;
        long num = strtol(cmd->argv[2], &tmp, 10);

        if (tmp && *tmp)
          CONF_ERROR(cmd, "badly formatted argument");

        rlim->rlim_max = num;
      }
    }

    add_config_param(cmd->argv[0], 2, (void *) rlim, NULL);
  }

  return PR_HANDLED(cmd);
#else
  CONF_ERROR(cmd, "RLimitOpenFiles is not supported on this platform");
#endif
}

MODRET set_syslogfacility(cmd_rec *cmd) {
  int i;
  struct {
    char *name;
    int facility;
  } factable[] = {
  { "AUTH",		LOG_AUTHPRIV		},
  { "AUTHPRIV",		LOG_AUTHPRIV		},
#ifdef HAVE_LOG_FTP
  { "FTP",		LOG_FTP			},
#endif
#ifdef HAVE_LOG_CRON
  { "CRON",		LOG_CRON		},
#endif
  { "DAEMON",		LOG_DAEMON		},
  { "KERN",		LOG_KERN		},
  { "LOCAL0",		LOG_LOCAL0		},
  { "LOCAL1",		LOG_LOCAL1		},
  { "LOCAL2",		LOG_LOCAL2		},
  { "LOCAL3",		LOG_LOCAL3		},
  { "LOCAL4",		LOG_LOCAL4		},
  { "LOCAL5",		LOG_LOCAL5		},
  { "LOCAL6",		LOG_LOCAL6		},
  { "LOCAL7",		LOG_LOCAL7		},
  { "LPR",		LOG_LPR			},
  { "MAIL",		LOG_MAIL		},
  { "NEWS",		LOG_NEWS		},
  { "USER",		LOG_USER		},
  { "UUCP",		LOG_UUCP		},
  { NULL,		0			} };

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT);

  for (i = 0; factable[i].name; i++) {
    if (strcasecmp(cmd->argv[1], factable[i].name) == 0) {
      log_closesyslog();
      log_setfacility(factable[i].facility);

      pr_signals_block();
      switch (log_opensyslog(NULL)) {
        case -1:
          pr_signals_unblock();
          CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "unable to open syslog: ",
            strerror(errno), NULL));
          break;

        case PR_LOG_WRITABLE_DIR:
          pr_signals_unblock();
          CONF_ERROR(cmd,
            "you are attempting to log to a world writeable directory");
          break;

        case PR_LOG_SYMLINK:
          pr_signals_unblock();
          CONF_ERROR(cmd, "you are attempting to log to a symbolic link");
          break;

        default:
          break;
      }
      pr_signals_unblock();

      return PR_HANDLED(cmd);
    }
  }

  CONF_ERROR(cmd, "argument must be a valid syslog facility");
}

MODRET set_timesgmt(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = bool;

  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

MODRET set_regex(cmd_rec *cmd, char *param, char *type) {
#if defined(HAVE_REGEX_H) && defined(HAVE_REGCOMP)
  regex_t *preg = NULL;
  config_rec *c = NULL;
  int res = 0;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON|CONF_DIR|
    CONF_DYNDIR);

  pr_log_debug(DEBUG4, "%s: compiling %s regex '%s'", cmd->argv[0], type,
    cmd->argv[1]);
  preg = pr_regexp_alloc();

  res = regcomp(preg, cmd->argv[1], REG_EXTENDED|REG_NOSUB);
  if (res != 0) {
    char errstr[200] = {'\0'};

    regerror(res, preg, errstr, sizeof(errstr));
    pr_regexp_free(preg);

    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "'", cmd->argv[1], "' failed regex "
      "compilation: ", errstr, NULL));
  }

  c = add_config_param(param, 1, preg);
  c->flags |= CF_MERGEDOWN;
  return PR_HANDLED(cmd);

#else /* no regular expression support at the moment */
  CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "The ", param, " directive cannot be "
    "used on this system, as you do not have POSIX compliant regex support",
    NULL));
#endif
}

MODRET set_allowdenyfilter(cmd_rec *cmd) {
#if defined(HAVE_REGEX_H) && defined(HAVE_REGCOMP)
  regex_t *preg = NULL;
  config_rec *c = NULL;
  int res = 0;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON|CONF_DIR|
    CONF_DYNDIR|CONF_LIMIT);

  preg = pr_regexp_alloc();

  pr_log_debug(DEBUG4, "%s: compiling regex '%s'", cmd->argv[0], cmd->argv[1]);
  res = regcomp(preg, cmd->argv[1], REG_EXTENDED|REG_NOSUB);
  if (res != 0) {
    char errstr[200] = {'\0'};

    regerror(res, preg, errstr, sizeof(errstr));
    pr_regexp_free(preg);

    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "'", cmd->argv[1], "' failed regex "
      "compilation: ", errstr, NULL));
  }

  c = add_config_param(cmd->argv[0], 1, preg);
  c->flags |= CF_MERGEDOWN;
  return HANDLED(cmd);

#else /* no regular expression support at the moment */
  CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "The ", cmd->argv[0],
    " directive cannot be used on this system, as you do not have POSIX "
    "compliant regex support", NULL));
#endif
}

MODRET set_passiveports(cmd_rec *cmd) {
  int pasv_min_port, pasv_max_port;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 2);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  pasv_min_port = atoi(cmd->argv[1]);
  pasv_max_port = atoi(cmd->argv[2]);

  /* Sanity check */
  if (pasv_min_port <= 0 || pasv_min_port > 65535)
    CONF_ERROR(cmd, "min port must be allowable port number");

  if (pasv_max_port <= 0 || pasv_max_port > 65535)
    CONF_ERROR(cmd, "max port must be allowable port number");

  if (pasv_min_port < 1024 || pasv_max_port < 1024)
    CONF_ERROR(cmd, "port numbers must be above 1023");

  if (pasv_max_port <= pasv_min_port)
    CONF_ERROR(cmd, "min port must be less than max port");

  c = add_config_param(cmd->argv[0], 2, NULL, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = pasv_min_port;
  c->argv[1] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[1]) = pasv_max_port;

  return PR_HANDLED(cmd);
}

MODRET set_pathallowfilter(cmd_rec *cmd) {
  return set_regex(cmd, cmd->argv[0], "allow");
}

MODRET set_pathdenyfilter(cmd_rec *cmd) {
  return set_regex(cmd, cmd->argv[0], "deny");
}

MODRET set_allowforeignaddress(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = bool;

  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

MODRET set_commandbuffersize(cmd_rec *cmd) {
  int size = 0;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  /* NOTE: need to add checks for maximum possible sizes, negative sizes. */
  size = atoi(cmd->argv[1]);

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = size;

  return PR_HANDLED(cmd);
}

MODRET set_cdpath(cmd_rec *cmd) {
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  c = add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

MODRET add_directory(cmd_rec *cmd) {
  config_rec *c;
  char *dir, *rootdir = NULL;
  int flags = 0;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  dir = cmd->argv[1];

  if (*dir != '/' &&
      *dir != '~' &&
      (!cmd->config ||
       cmd->config->config_type != CONF_ANON))
    CONF_ERROR(cmd, "relative path not allowed in non-<Anonymous> sections");

  /* If in anonymous mode, and path is relative, just cat anon root
   * and relative path.
   *
   * Note: This is no longer necessary, because we don't interpolate anonymous
   * directories at run-time.
   */
  if (cmd->config &&
      cmd->config->config_type == CONF_ANON &&
      *dir != '/' &&
      *dir != '~') {
    if (strcmp(dir, "*") != 0)
      dir = pdircat(cmd->tmp_pool, "/", dir, NULL);
    rootdir = cmd->config->name;

  } else {
    if (pr_fs_valid_path(dir) < 0) {
      /* Not an absolute path; mark it for deferred resolution. */
      flags |= CF_DEFER;
    }
  }

  /* Check to see that there isn't already a config for this directory,
   * but only if we're not in an <Anonymous> section.  Due to the way
   * in which later <Directory> checks are done, <Directory> blocks inside
   * <Anonymous> sections are handled differently than outside, probably
   * overriding their outside counterparts (if necessary).  This is
   * probably OK, as this overriding only takes effect for the <Anonymous>
   * user.
   */

  if (!check_context(cmd, CONF_ANON) &&
      find_config(cmd->server->conf, CONF_DIR, dir, FALSE) != NULL) {
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool,
      "<Directory> section already configured for '", cmd->argv[1], "'", NULL));
  }

  /* Check for any expandable variables, and mark this config_rec for
   * deferred resolution if present
   */
  if (strstr(dir, "%u")) {
    flags |= CF_DEFER;
  }

  c = pr_parser_config_ctxt_open(dir);
  c->argc = 2;
  c->argv = pcalloc(c->pool, 3 * sizeof(void *));

  /* If we do NOT have rootdir, then do NOT add anything to the argv[1] slot;
   * it is intended solely for that particular use case.
   */
  if (rootdir) {
    c->argv[1] = pstrdup(c->pool, rootdir);
  }

  c->config_type = CONF_DIR;
  c->flags |= flags;

  if (!(c->flags & CF_DEFER)) {
    pr_log_debug(DEBUG2, "<Directory %s>: adding section for resolved "
      "path '%s'", cmd->argv[1], dir);

  } else {
    pr_log_debug(DEBUG2,
      "<Directory %s>: deferring resolution of path", cmd->argv[1]);
  }

  return PR_HANDLED(cmd);
}

MODRET set_hidefiles(cmd_rec *cmd) {
#if defined(HAVE_REGEX_H) && defined(HAVE_REGCOMP)
  regex_t *regexp = NULL;
  config_rec *c = NULL;
  unsigned int precedence = 0;
  unsigned char negated = FALSE, none = FALSE;

  int ctxt = (cmd->config && cmd->config->config_type != CONF_PARAM ?
    cmd->config->config_type : cmd->server->config_type ?
    cmd->server->config_type : CONF_ROOT);

  /* This directive must have either 1, or 3, arguments */
  if (cmd->argc-1 != 1 &&
      cmd->argc-1 != 3) {
    CONF_ERROR(cmd, "wrong number of parameters");
  }

  CHECK_CONF(cmd, CONF_DIR|CONF_DYNDIR);

  /* Set the precedence for this config_rec based on its configuration
   * context.
   */
  if (ctxt & CONF_DIR) {
    precedence = 1;

  } else {
    precedence = 2;
  }

  /* Check for a leading '!' prefix, signifying regex negation */
  if (*cmd->argv[1] == '!') {
    negated = TRUE;
    cmd->argv[1] = cmd->argv[1] + 1;

  } else {
    /* Check for a "none" argument, which is used to nullify inherited
     * HideFiles configurations from parent directories.
     */
    if (strcasecmp(cmd->argv[1], "none") == 0) {
      none = TRUE;
    }
  }

  if (!none) {
    int res;

    regexp = pr_regexp_alloc();
  
    res = regcomp(regexp, cmd->argv[1], REG_EXTENDED|REG_NOSUB);
    if (res != 0) {
      char errstr[200] = {'\0'};

      regerror(res, regexp, errstr, sizeof(errstr));
      pr_regexp_free(regexp);

      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "'", cmd->argv[1],
        "' failed regex compilation: ", errstr, NULL));
    }
  }

  /* If the directive was used with 3 arguments, then the optional
   * classifiers, and classifier expression, were used.  Make sure that
   * a valid classifier was used.
   */
  if (cmd->argc-1 == 3) {
    if (strcmp(cmd->argv[2], "user") == 0 ||
        strcmp(cmd->argv[2], "group") == 0 ||
        strcmp(cmd->argv[2], "class") == 0) {

      /* no-op */

    } else {
      return PR_ERROR_MSG(cmd, NULL, pstrcat(cmd->tmp_pool, cmd->argv[0],
        ": unknown classifier used: '", cmd->argv[2], "'", NULL));
    }
  }

  if (cmd->argc-1 == 1) {
    c = add_config_param(cmd->argv[0], 3, NULL, NULL, NULL);
    c->argv[0] = pcalloc(c->pool, sizeof(regex_t *));
    *((regex_t **) c->argv[0]) = regexp;
    c->argv[1] = pcalloc(c->pool, sizeof(unsigned char));
    *((unsigned char *) c->argv[1]) = negated;
    c->argv[2] = pcalloc(c->pool, sizeof(unsigned int));
    *((unsigned int *) c->argv[2]) = precedence;

  } else if (cmd->argc-1 == 3) {
    array_header *acl = NULL;
    int argc = cmd->argc - 3;
    char **argv = cmd->argv + 2;

    acl = pr_expr_create(cmd->tmp_pool, &argc, argv);

    c = add_config_param(cmd->argv[0], 0);
    c->argc = argc + 4;

    /* Add 5 to argc for the argv of the config_rec: one for the
     * regexp, one for the 'negated' value, one for the precedence,
     * one for the classifier, and one for the terminating NULL
     */
    c->argv = pcalloc(c->pool, ((argc + 5) * sizeof(char *)));

    /* Capture the config_rec's argv pointer for doing the by-hand
     * population.
     */
    argv = (char **) c->argv;

    /* Copy in the regexp. */
    *argv = pcalloc(c->pool, sizeof(regex_t *));
    *((regex_t **) *argv++) = regexp;

    /* Copy in the 'negated' flag */
    *argv = pcalloc(c->pool, sizeof(unsigned char));
    *((unsigned char *) *argv++) = negated;

    /* Copy in the precedence. */
    *argv = pcalloc(c->pool, sizeof(unsigned int));
    *((unsigned int *) *argv++) = precedence;

    /* Copy in the expression classifier */
    *argv++ = pstrdup(c->pool, cmd->argv[2]);

    /* now, copy in the expression arguments */
    if (argc && acl) {
      while (argc--) {
        *argv++ = pstrdup(c->pool, *((char **) acl->elts));
        acl->elts = ((char **) acl->elts) + 1;
      }
    }

    /* don't forget the terminating NULL */
    *argv = NULL;
  }

  c->flags |= CF_MERGEDOWN_MULTI;
  return PR_HANDLED(cmd);

#else /* no regular expression support at the moment */
  CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "The HideFiles directive cannot be "
    "used on this system, as you do not have POSIX compliant regex support",
    NULL));
#endif
}

MODRET set_hidenoaccess(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ANON|CONF_DIR);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = bool;
  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

MODRET set_hideuser(cmd_rec *cmd) {
  config_rec *c = NULL;
  char *user = NULL;
  uid_t uid;
  unsigned char inverted = FALSE;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ANON|CONF_DIR);

  user = cmd->argv[1];

  if (*user == '!') {
    inverted = TRUE;
    user++;
  }

  if (strcmp(user, "~") != 0) {
    struct passwd *pw;

    pw = pr_auth_getpwnam(cmd->tmp_pool, user);
    if (!pw)
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "'", user,
        "' is not a valid user", NULL));

    uid = pw->pw_uid;

  } else {
    uid = (uid_t) -1;
  }

  c = add_config_param(cmd->argv[0], 2, NULL, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(uid_t));
  *((uid_t *) c->argv[0]) = uid;
  c->argv[1] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[1]) = inverted;

  c->flags |= CF_MERGEDOWN;
  return PR_HANDLED(cmd);
}

MODRET set_hidegroup(cmd_rec *cmd) {
  config_rec *c = NULL;
  char *group = NULL;
  gid_t gid;
  unsigned char inverted = FALSE;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ANON|CONF_DIR);

  group = cmd->argv[1];

  if (*group == '!') {
    inverted = TRUE;
    group++;
  }

  if (strcmp(group, "~") != 0) {
    struct group *gr;

    gr = pr_auth_getgrnam(cmd->tmp_pool, group);
    if (!gr)
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "'", group,
        "' is not a valid group", NULL));

    gid = gr->gr_gid;

  } else {
    gid = (gid_t) -1;
  }

  c = add_config_param(cmd->argv[0], 2, NULL, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(gid_t));
  *((gid_t *) c->argv[0]) = gid;
  c->argv[1] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[1]) = inverted;

  c->flags |= CF_MERGEDOWN;
  return PR_HANDLED(cmd);
}

MODRET add_groupowner(cmd_rec *cmd) {
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ANON|CONF_DIR|CONF_DYNDIR);

  c = add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

MODRET add_userowner(cmd_rec *cmd) {
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ANON|CONF_DIR);

  c = add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

MODRET set_allowoverride(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;
  unsigned int precedence = 0;

  int ctxt = (cmd->config && cmd->config->config_type != CONF_PARAM ?
     cmd->config->config_type : cmd->server->config_type ?
     cmd->server->config_type : CONF_ROOT);

  /* This directive must have either 1 argument; the 3 arguments format is
   * now deprecated.
   */
  if (cmd->argc-1 == 3) {
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "Please use mod_ifsession for "
      "per-user/group/class conditional configuration", NULL));
  }

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON|CONF_DIR);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  /* Set the precedence for this config_rec based on its configuration
   * context.
   */
  if (ctxt & CONF_GLOBAL) {
    precedence = 1;

  /* These will never appear simultaneously */
  } else if (ctxt & CONF_ROOT || ctxt & CONF_VIRTUAL) {
    precedence = 2;

  } else if (ctxt & CONF_ANON) {
    precedence = 3;

  } else if (ctxt & CONF_DIR) {
    precedence = 4;
  }

  c = add_config_param(cmd->argv[0], 2, NULL, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = bool;
  c->argv[1] = pcalloc(c->pool, sizeof(unsigned int));
  *((unsigned int *) c->argv[1]) = precedence;
  c->flags |= CF_MERGEDOWN_MULTI;

  return PR_HANDLED(cmd);
}

MODRET end_directory(cmd_rec *cmd) {
  int empty_ctxt = FALSE;

  CHECK_ARGS(cmd, 0);
  CHECK_CONF(cmd, CONF_DIR);

  pr_parser_config_ctxt_close(&empty_ctxt);

  if (empty_ctxt)
    pr_log_debug(DEBUG3, "%s: ignoring empty section", cmd->argv[0]);

  return PR_HANDLED(cmd);
}

MODRET add_anonymous(cmd_rec *cmd) {
  config_rec *c = NULL;
  char *dir;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  dir = cmd->argv[1];

  if (*dir != '/' && *dir != '~')
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "(", dir, ") absolute pathname "
      "required", NULL));

  if (strchr(dir, '*'))
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "(", dir, ") wildcards not allowed "
      "in pathname", NULL));

  if (strcmp(dir,"/") == 0)
    CONF_ERROR(cmd, "'/' not permitted for anonymous root directory");

  if (*(dir+strlen(dir)-1) != '/')
    dir = pstrcat(cmd->tmp_pool, dir, "/", NULL);

  if (!dir)
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, cmd->argv[1], ": ",
      strerror(errno), NULL));

  c = pr_parser_config_ctxt_open(dir);

  c->config_type = CONF_ANON;
  return PR_HANDLED(cmd);
}

MODRET end_anonymous(cmd_rec *cmd) {
  int empty_ctxt = FALSE;

  CHECK_ARGS(cmd, 0);
  CHECK_CONF(cmd, CONF_ANON);

  pr_parser_config_ctxt_close(&empty_ctxt);

  if (empty_ctxt)
    pr_log_debug(DEBUG3, "%s: ignoring empty section", cmd->argv[0]);

  return PR_HANDLED(cmd);
}

MODRET add_class(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT);

  if (pr_class_open(main_server->pool, cmd->argv[1]) < 0)
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "error creating <Class ",
      cmd->argv[1], ">: ", strerror(errno), NULL));

  return PR_HANDLED(cmd);
}

MODRET end_class(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 0);
  CHECK_CONF(cmd, CONF_CLASS);

  if (pr_class_close() < 0)
    pr_log_pri(PR_LOG_WARNING, "warning: empty <Class> definition");

  return PR_HANDLED(cmd);
}

MODRET add_global(cmd_rec *cmd) {
  config_rec *c = NULL;

  if (cmd->argc-1 != 0)
    CONF_ERROR(cmd, "Too many parameters");
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL);

  c = pr_parser_config_ctxt_open(cmd->argv[0]);
  c->config_type = CONF_GLOBAL;

  return PR_HANDLED(cmd);
}

MODRET end_global(cmd_rec *cmd) {
  int empty_ctxt = FALSE;

  CHECK_ARGS(cmd, 0);
  CHECK_CONF(cmd, CONF_GLOBAL);

  pr_parser_config_ctxt_close(&empty_ctxt);

  if (empty_ctxt)
    pr_log_debug(DEBUG3, "%s: ignoring empty section", cmd->argv[0]);

  return PR_HANDLED(cmd);
}

MODRET add_limit(cmd_rec *cmd) {
  register unsigned int i;
  config_rec *c = NULL;
  int cargc, have_cdup = FALSE, have_xcup = FALSE, have_mkd = FALSE,
    have_xmkd = FALSE, have_pwd = FALSE, have_xpwd = FALSE, have_rmd = FALSE,
    have_xrmd = FALSE;
  char **cargv, **elts;
  array_header *list;

  if (cmd->argc < 2)
    CONF_ERROR(cmd, "directive requires one or more commands");
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_DIR|CONF_ANON|CONF_DYNDIR|CONF_GLOBAL);

  c = pr_parser_config_ctxt_open("Limit");
  c->config_type = CONF_LIMIT;
  cargc = cmd->argc;
  cargv = cmd->argv;

  list = make_array(c->pool, c->argc + 1, sizeof(void *));

  while (cargc-- && *(++cargv)) {
    char *ent = NULL;
    char *str = pstrdup(cmd->tmp_pool, *cargv);

    while ((ent = pr_str_get_token(&str, ",")) != NULL) {
      pr_signals_handle();

      if (*ent) {
        *((char **) push_array(list)) = pstrdup(c->pool, ent);
      }
    }
  }

  /* Now iterate though the list, looking for the following commands:
   *
   *  CDUP/XCUP
   *  MKD/XMKD
   *  PWD/XPWD
   *  RMD/XRMD
   *
   * If we see one of these without its counterpart, automatically add
   * the counterpart (see Bug#3077).
   */

  elts = (char **) list->elts;
  for (i = 0; i < list->nelts; i++) {
    if (strcasecmp(elts[i], "CDUP") == 0) {
      have_cdup = TRUE;

    } else if (strcasecmp(elts[i], "XCUP") == 0) {
      have_xcup = TRUE; 

    } else if (strcasecmp(elts[i], "MKD") == 0) {
      have_mkd = TRUE;

    } else if (strcasecmp(elts[i], "XMKD") == 0) {
      have_xmkd = TRUE;

    } else if (strcasecmp(elts[i], "PWD") == 0) {
      have_pwd = TRUE;

    } else if (strcasecmp(elts[i], "XPWD") == 0) {
      have_xpwd = TRUE;

    } else if (strcasecmp(elts[i], "RMD") == 0) {
      have_rmd = TRUE;

    } else if (strcasecmp(elts[i], "XRMD") == 0) {
      have_xrmd = TRUE;
    }
  }

  if (have_cdup && !have_xcup) {
    *((char **) push_array(list)) = pstrdup(c->pool, "XCUP");
  }

  if (!have_cdup && have_xcup) {
    *((char **) push_array(list)) = pstrdup(c->pool, "CDUP");
  }

  if (have_mkd && !have_xmkd) {
    *((char **) push_array(list)) = pstrdup(c->pool, "XMKD");
  }

  if (!have_mkd && have_xmkd) {
    *((char **) push_array(list)) = pstrdup(c->pool, "MKD");
  }

  if (have_pwd && !have_xpwd) {
    *((char **) push_array(list)) = pstrdup(c->pool, "XPWD");
  }

  if (!have_pwd && have_xpwd) {
    *((char **) push_array(list)) = pstrdup(c->pool, "PWD");
  }

  if (have_rmd && !have_xrmd) {
    *((char **) push_array(list)) = pstrdup(c->pool, "XRMD");
  }

  if (!have_rmd && have_xrmd) {
    *((char **) push_array(list)) = pstrdup(c->pool, "RMD");
  }

  c->argc = list->nelts;
  c->argv = list->elts;

  return PR_HANDLED(cmd);
}

MODRET set_order(cmd_rec *cmd) {
  int order = -1,argc = cmd->argc;
  char *arg = "",**argv = cmd->argv+1;
  config_rec *c = NULL;

  CHECK_CONF(cmd, CONF_LIMIT);

  while (--argc && *argv)
    arg = pstrcat(cmd->tmp_pool, arg, *argv++, NULL);

  if (strcasecmp(arg, "allow,deny") == 0)
    order = ORDER_ALLOWDENY;

  else if (strcasecmp(arg, "deny,allow") == 0)
    order = ORDER_DENYALLOW;

  else
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "'", arg, "': invalid argument",
      NULL));

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = order;

  return PR_HANDLED(cmd);
}

MODRET set_allowdenyusergroupclass(cmd_rec *cmd) {
  config_rec *c;
  char **argv;
  int argc, eval_type;
  array_header *acl;
 
  CHECK_CONF(cmd, CONF_LIMIT);

  if (cmd->argc < 2)
    CONF_ERROR(cmd, "wrong number of parameters");

  /* For AllowClass/DenyClass and AllowUser/DenyUser, the default expression
   * type is "or".
   */
  if (strcmp(cmd->argv[0], "AllowClass") == 0 ||
      strcmp(cmd->argv[0], "AllowUser") == 0 ||
      strcmp(cmd->argv[0], "DenyClass") == 0 ||
      strcmp(cmd->argv[0], "DenyUser") == 0)
    eval_type = PR_EXPR_EVAL_OR;

  /* For AllowGroup and DenyGroup, the default expression type is "and". */
  else
    eval_type = PR_EXPR_EVAL_AND;

  if (cmd->argc > 2) {
    /* Check the first parameter to see if it is an evaluation modifier:
     * "and", "or", or "regex".
     */
    if (strcasecmp(cmd->argv[1], "AND") == 0) {
      eval_type = PR_EXPR_EVAL_AND;
      argc = cmd->argc-2;
      argv = cmd->argv+1;

    } else if (strcasecmp(cmd->argv[1], "OR") == 0) {
      eval_type = PR_EXPR_EVAL_OR;
      argc = cmd->argc-2;
      argv = cmd->argv+1;

    } else if (strcasecmp(cmd->argv[1], "regex") == 0) {
#if defined(HAVE_REGEX_H) && defined(HAVE_REGCOMP)
      regex_t *preg;
      int res;

      if (cmd->argc != 3)
        CONF_ERROR(cmd, "wrong number of parameters");

      preg = pr_regexp_alloc();

      res = regcomp(preg, cmd->argv[2], REG_EXTENDED|REG_NOSUB);
      if (res != 0) {
        char errstr[200] = {'\0'};

        regerror(res, preg, errstr, sizeof(errstr));
        pr_regexp_free(preg);

        CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "'", cmd->argv[2], "' failed "
          "regex compilation: ", errstr, NULL));
      }

      c = add_config_param(cmd->argv[0], 2, NULL, NULL);
      c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
      *((unsigned char *) c->argv[0]) = PR_EXPR_EVAL_REGEX;
      c->argv[1] = (void *) preg;

      return PR_HANDLED(cmd);

#else
      CONF_ERROR(cmd, "The 'regex' parameter cannot be used on this system, "
        "as you do not have POSIX compliant regex support");
#endif /* HAVE_REGEX_H and HAVE_REGCOMP */

    } else {
      argc = cmd->argc-1;
      argv = cmd->argv;
    }

  } else {
    argc = cmd->argc-1;
    argv = cmd->argv;
  }

  acl = pr_expr_create(cmd->tmp_pool, &argc, argv);

  c = add_config_param(cmd->argv[0], 0);

  c->argc = acl->nelts + 1;
  c->argv = pcalloc(c->pool, (c->argc + 1) * sizeof(char *));

  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = eval_type;

  argv = (char **) c->argv + 1;

  if (acl) {
    while (acl->nelts--) {
      *argv++ = pstrdup(c->pool, *((char **) acl->elts));
      acl->elts = ((char **) acl->elts) + 1;
    }
  }

  *argv = NULL;

  return PR_HANDLED(cmd);
}

MODRET set_allowdeny(cmd_rec *cmd) {
  int argc;
  char **argv;
  pr_netacl_t **aclargv;
  array_header *list;
  config_rec *c;

  CHECK_CONF(cmd, CONF_LIMIT);

  /* Syntax: allow [from] [all|none]|host|network[,...] */
  list = make_array(cmd->tmp_pool, cmd->argc, sizeof(pr_netacl_t *));
  argc = cmd->argc-1;
  argv = cmd->argv;

  c = add_config_param(cmd->argv[0], 0);

  /* Skip optional "from" keyword. The '!' character is allowed in front of a
   * hostmask or IP, but NOT in front of "ALL" or "NONE".
   */

  while (argc && *(argv+1)) {
    if (strcasecmp("from", *(argv+1)) == 0) {
      argv++;
      argc--;
      continue;

    } else if (strcasecmp("!all", *(argv+1)) == 0 ||
               strcasecmp("!none", *(argv+1)) == 0) {
      CONF_ERROR(cmd, "the ! negation operator cannot be used with ALL/NONE");

    } else if (strcasecmp("all", *(argv+1)) == 0 ||
               strcasecmp("none", *(argv+1)) == 0) {
      *((pr_netacl_t **) push_array(list)) =
        pr_netacl_create(c->pool, *(argv+1));
      argc = 0;
    }

    break;
  }

  /* Parse any other/remaining rules. */
  while (argc-- && *(++argv)) {
    char *ent = NULL;
    char *s = pstrdup(cmd->tmp_pool, *argv);

    /* Parse the string into comma-delimited entries */
    while ((ent = pr_str_get_token(&s, ",")) != NULL) {
      if (*ent) {
        pr_netacl_t *acl;

        if (strcasecmp(ent, "all") == 0 ||
            strcasecmp(ent, "none") == 0) {
          list->nelts = 0;
          argc = 0;
          break;
        }

        acl = pr_netacl_create(c->pool, ent);
        if (acl == NULL) {
          CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "bad ACL definition '",
            ent, "': ", strerror(errno), NULL));     
        }

        pr_trace_msg("netacl", 9, "'%s' parsed into netacl '%s'", ent,
          pr_netacl_get_str(cmd->tmp_pool, acl));

        *((pr_netacl_t **) push_array(list)) = acl;
      }
    }
  }

  if (!list->nelts)
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "syntax: ", cmd->argv[0],
      " [from] [all|none]|host|network[,...]", NULL));

  c->argc = list->nelts;
  c->argv = pcalloc(c->pool, (c->argc+1) * sizeof(pr_netacl_t *));
  aclargv = (pr_netacl_t **) c->argv;

  while (list->nelts--) {
    *aclargv++ = *((pr_netacl_t **) list->elts);
    list->elts = ((pr_netacl_t **) list->elts) + 1;
  }
  *aclargv = NULL;

  return PR_HANDLED(cmd);
}

MODRET set_denyall(cmd_rec *cmd) {
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 0);
  CHECK_CONF(cmd, CONF_LIMIT|CONF_ANON|CONF_DIR|CONF_DYNDIR);

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = TRUE;

  return PR_HANDLED(cmd);
}

MODRET set_allowall(cmd_rec *cmd) {
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 0);
  CHECK_CONF(cmd, CONF_LIMIT|CONF_ANON|CONF_DIR|CONF_DYNDIR);

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = TRUE;

  return PR_HANDLED(cmd);
}

MODRET set_authorder(cmd_rec *cmd) {
  register unsigned int i = 0;
  config_rec *c = NULL;
  array_header *module_list = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  /* Check to see if the directive has already been set */
  if (find_config(cmd->server->conf, CONF_PARAM, cmd->argv[0], FALSE))
    CONF_ERROR(cmd, "AuthOrder has already been configured");

  c = add_config_param(cmd->argv[0], 1, NULL);
  module_list = make_array(c->pool, 0, sizeof(char *));

  for (i = 1; i < cmd->argc; i++)
    *((char **) push_array(module_list)) = pstrdup(c->pool, cmd->argv[i]);

  c->argv[0] = (void *) module_list;

  return PR_HANDLED(cmd);
}

MODRET end_limit(cmd_rec *cmd) {
  int empty_ctxt = FALSE;

  CHECK_ARGS(cmd, 0);
  CHECK_CONF(cmd, CONF_LIMIT);

  pr_parser_config_ctxt_close(&empty_ctxt);

  if (empty_ctxt)
    pr_log_debug(DEBUG3, "%s: ignoring empty section", cmd->argv[0]);

  return PR_HANDLED(cmd);
}

MODRET set_ignorehidden(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_LIMIT);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = bool;

  return PR_HANDLED(cmd);
}

/* usage: DisplayChdir path [on|off] */
MODRET set_displaychdir(cmd_rec *cmd) {
  config_rec *c = NULL;
  int bool = FALSE;

  if (cmd->argc-1 < 1 ||
      cmd->argc-1 > 2)
    CONF_ERROR(cmd, "wrong number of parameters");

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON|CONF_DIR);

  if (cmd->argc-1 == 2) {
    bool = get_boolean(cmd, 2);
    if (bool < 0) {
      CONF_ERROR(cmd, "expected Boolean parameter");
    }
  }

  c = add_config_param(cmd->argv[0], 2, NULL, NULL);
  c->argv[0] = pstrdup(c->pool, cmd->argv[1]);
  c->argv[1] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[1]) = bool;

  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

MODRET set_displayconnect(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);

  return PR_HANDLED(cmd);
}

MODRET set_displayquit(cmd_rec *cmd) {
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  c = add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

MODRET set_displaygoaway(cmd_rec *cmd) {
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  c = add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

MODRET add_virtualhost(cmd_rec *cmd) {
  server_rec *s = NULL;
  pr_netaddr_t *addr = NULL;
  array_header *addrs = NULL;

  if (cmd->argc-1 < 1)
    CONF_ERROR(cmd, "wrong number of parameters");
  CHECK_CONF(cmd, CONF_ROOT);

  s = pr_parser_server_ctxt_open(cmd->argv[1]);
  if (s == NULL)
    CONF_ERROR(cmd, "unable to create virtual server configuration");

  /* It's possible for a server to have multiple IP addresses (e.g. a DNS
   * name that has both A and AAAA records).  We need to handle that case
   * here by looking up all of a server's addresses, and making sure there
   * are server_recs for each one.
   */

  addr = pr_netaddr_get_addr(cmd->tmp_pool, cmd->argv[1], &addrs);
  if (addrs) {
    register unsigned int i;
    pr_netaddr_t **elts = addrs->elts;

    /* For every additional address, implicitly add a bind record. */
    for (i = 0; i < addrs->nelts; i++)
      add_config_param_str("_bind", 1, pr_netaddr_get_ipstr(elts[i]));
  }

  /* Handle multiple addresses in a <VirtualHost> directive.  We do
   * this by adding bind directives to the server_rec created for the
   * first address.
   */
  if (cmd->argc-1 > 1) {
    register unsigned int i;

    for (i = 2; i < cmd->argc; i++) {
      addrs = NULL;

      addr = pr_netaddr_get_addr(cmd->tmp_pool, cmd->argv[i], &addrs);

      if (addr == NULL)
        CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "error resolving '",
          cmd->argv[i], "': ", strerror(errno), NULL));

      add_config_param_str("_bind", 1, pr_netaddr_get_ipstr(addr));

      if (addrs) {
        register unsigned int j;
        pr_netaddr_t **elts = addrs->elts;

        /* For every additional address, implicitly add a bind record. */
        for (j = 0; j < addrs->nelts; j++)
          add_config_param_str("_bind", 1, pr_netaddr_get_ipstr(elts[j]));
      }
    }
  }

  return PR_HANDLED(cmd);
}

MODRET end_virtualhost(cmd_rec *cmd) {
  server_rec *s = NULL, *next_s = NULL;
  pr_netaddr_t *addr = NULL;
  const char *address = NULL;

  CHECK_ARGS(cmd, 0);
  CHECK_CONF(cmd, CONF_VIRTUAL);

  if (cmd->server->ServerAddress)
    address = cmd->server->ServerAddress;
  else
    address = pr_netaddr_get_localaddr_str(cmd->tmp_pool);

  /* Any additional addresses associated with the configured address have
   * already been handled, so we can ignore them here.
   */
  addr = pr_netaddr_get_addr(cmd->tmp_pool, address, NULL);
  if (addr == NULL)
    /* This bad server context will be removed in fixup_servers(), after
     * the parsing has completed, so we need do nothing else here.
     */
    pr_log_pri(PR_LOG_WARNING,
      "warning: unable to determine IP address of '%s'", address);

  if (AddressCollisionCheck) {
    /* Check if this server's address/port combination is already being used. */
    for (s = (server_rec *) server_list->xas_list; addr && s; s = next_s) {
      next_s = s->next;

      /* Have to resort to duplicating some of fixup_servers()'s functionality
       * here, to do this check The Right Way(tm).
       */
      if (s != cmd->server) {
        const char *serv_addrstr = NULL;
        pr_netaddr_t *serv_addr = NULL;

        if (s->addr) {
          serv_addr = s->addr;

        } else {
          serv_addrstr = s->ServerAddress ? s->ServerAddress :
            pr_netaddr_get_localaddr_str(cmd->tmp_pool);

          serv_addr = pr_netaddr_get_addr(cmd->tmp_pool, serv_addrstr, NULL);
        }

        if (!serv_addr) {
          pr_log_pri(PR_LOG_WARNING,
            "warning: unable to determine IP address of '%s'", serv_addrstr);

        } else if (pr_netaddr_cmp(addr, serv_addr) == 0 &&
            cmd->server->ServerPort == s->ServerPort) {
          pr_log_pri(PR_LOG_WARNING,
            "warning: \"%s\" address/port (%s:%d) already in use by \"%s\"",
            cmd->server->ServerName ? cmd->server->ServerName : "ProFTPD",
            pr_netaddr_get_ipstr(addr), cmd->server->ServerPort,
            s->ServerName ? s->ServerName : "ProFTPD");

          if (xaset_remove(server_list, (xasetmember_t *) cmd->server) == 1)
            destroy_pool(cmd->server->pool);

          continue;
        }
      }
    }
  }

  if (pr_parser_server_ctxt_close() == NULL)
    CONF_ERROR(cmd, "must have matching <VirtualHost> directive");
    
  return PR_HANDLED(cmd);
}

#if defined(HAVE_REGEX_H) && defined(HAVE_REGCOMP)
MODRET regex_filters(cmd_rec *cmd) {
  regex_t *allow_regex = NULL, *deny_regex = NULL;

  /* Don't apply the filter checks to passwords (arguments to the PASS
   * command).
   */
  if (strcasecmp(cmd->argv[0], C_PASS) == 0)
    return PR_DECLINED(cmd);

  /* Check for an AllowFilter */
  allow_regex = get_param_ptr(CURRENT_CONF, "AllowFilter", FALSE);

  if (allow_regex && cmd->arg &&
      regexec(allow_regex, cmd->arg, 0, NULL, 0) != 0) {
    pr_log_debug(DEBUG2, "'%s %s' denied by AllowFilter", cmd->argv[0],
      cmd->arg);
    pr_response_add_err(R_550, _("%s: Forbidden command argument"), cmd->arg);
    return PR_ERROR(cmd);
  }

  /* Check for a DenyFilter */
  deny_regex = get_param_ptr(CURRENT_CONF, "DenyFilter", FALSE);

  if (deny_regex && cmd->arg &&
      regexec(deny_regex, cmd->arg, 0, NULL, 0) == 0) {
    pr_log_debug(DEBUG2, "'%s %s' denied by DenyFilter", cmd->argv[0],
      cmd->arg);
    pr_response_add_err(R_550, _("%s: Forbidden command argument"), cmd->arg);
    return PR_ERROR(cmd);
  }

  return PR_DECLINED(cmd);
}
#endif /* HAVE_REGEX_H && HAVE_REGCOMP */

MODRET core_clear_fs(cmd_rec *cmd) {
  /* Make sure any FS caches are clear before each command. */
  pr_fs_clear_cache();

  return PR_DECLINED(cmd);
}

MODRET core_quit(cmd_rec *cmd) {
  if (displayquit_fh) {
    if (pr_display_fh(displayquit_fh, NULL, R_221) < 0)
      pr_log_debug(DEBUG6, "unable to display DisplayQuit file '%s': %s",
        displayquit_fh->fh_path, strerror(errno));
    pr_fsio_close(displayquit_fh);
    displayquit_fh = NULL;

    /* Hack or feature, pr_display_file() always puts a hyphen on the last line.
     */
    pr_response_send(R_221, "%s", "");

  } else {
    char *display = get_param_ptr(TOPLEVEL_CONF, "DisplayQuit", FALSE); 
    if (display) {
      if (pr_display_file(display, NULL, R_221) < 0)
        pr_log_debug(DEBUG6, "unable to display DisplayQuit file '%s': %s",
          display, strerror(errno));

      /* Hack or feature, pr_display_file() always puts a hyphen on the
       * last line
       */
      pr_response_send(R_221, "%s", "");

    } else {
      pr_response_send(R_221, _("Goodbye."));
    }
  }

  /* The LOG_CMD handler for QUIT is responsible for actually ending
   * the session.
   */

  return PR_HANDLED(cmd);
}

MODRET core_log_quit(cmd_rec *cmd) {

#ifndef PR_DEVEL_NO_DAEMON
  end_login(0);
#endif /* PR_DEVEL_NO_DAEMON */

  /* Even though end_login() does not return, this is necessary to avoid
   * compiler warnings.
   */
  return PR_HANDLED(cmd);
}

/* Per RFC959, directory responses for MKD and PWD should be
 * "dir_name" (w/ quote).  For directories that CONTAIN quotes,
 * the add'l quotes must be duplicated.
 */
static const char *quote_dir(cmd_rec *cmd, char *dir) {
  return sreplace(cmd->tmp_pool, dir, "\"", "\"\"", NULL);
}

MODRET core_pwd(cmd_rec *cmd) {
  CHECK_CMD_ARGS(cmd, 1);

  if (!dir_check(cmd->tmp_pool, cmd, cmd->group, session.vwd, NULL)) {
    pr_response_add_err(R_550, "%s: %s", cmd->argv[0], strerror(EACCES));
    return PR_ERROR(cmd);
  }

  pr_response_add(R_257, _("\"%s\" is the current directory"),
    quote_dir(cmd, pr_fs_encode_path(cmd->tmp_pool, session.vwd)));

  return PR_HANDLED(cmd);
}

MODRET core_pasv(cmd_rec *cmd) {
  unsigned int port = 0;
  char *addrstr = NULL, *tmp = NULL;
  config_rec *c = NULL;

  if (session.sf_flags & SF_EPSV_ALL) {
    pr_response_add_err(R_500, _("Illegal PASV command, EPSV ALL in effect"));
    return PR_ERROR(cmd);
  }

  CHECK_CMD_ARGS(cmd, 1);

  /* Returning 501 is the best we can do.  It would be nicer if RFC959 allowed
   * 550 as a possible response.
   */
  if (!dir_check(cmd->tmp_pool, cmd, cmd->group, session.cwd, NULL)) {
    pr_log_debug(DEBUG8, "PASV denied by <Limit> configuration");
    pr_response_add_err(R_501, "%s: %s", cmd->argv[0], strerror(EPERM));
    return PR_ERROR(cmd);
  }

  /* If we already have a passive listen data connection open, kill it. */
  if (session.d) {
    pr_inet_close(session.d->pool, session.d);
    session.d = NULL;
  }

  c = find_config(main_server->conf, CONF_PARAM, "PassivePorts", FALSE);
  if (c != NULL) {
    int pasv_min_port = *((int *) c->argv[0]);
    int pasv_max_port = *((int *) c->argv[1]);

    if (!(session.d = pr_inet_create_conn_portrange(session.pool,
        NULL, session.c->local_addr, pasv_min_port, pasv_max_port))) {

      /* If not able to open a passive port in the given range, default to
       * normal behavior (using INPORT_ANY), and log the failure.  This
       * indicates a too-small range configuration.
       */
      pr_log_pri(PR_LOG_WARNING,
        "unable to find open port in PassivePorts range %d-%d: "
        "defaulting to INPORT_ANY (consider defining a larger PassivePorts "
        "range)", pasv_min_port, pasv_max_port);
    }
  }

  /* Open up the connection and pass it back. */
  if (!session.d) {
    session.d = pr_inet_create_conn(session.pool, NULL, -1,
      session.c->local_addr, INPORT_ANY, FALSE);
  }

  if (!session.d) {
    pr_response_add_err(R_425, _("Unable to build data connection: "
      "Internal error"));
    return PR_ERROR(cmd);
  }

  pr_inet_set_block(session.pool, session.d);
  pr_inet_listen(session.pool, session.d, 1);

  session.d->instrm = pr_netio_open(session.pool, PR_NETIO_STRM_DATA,
    session.d->listen_fd, PR_NETIO_IO_RD);

  /* Now tell the client our address/port */
  port = session.data_port = session.d->local_port;
  session.sf_flags |= SF_PASSIVE;

  addrstr = (char *) pr_netaddr_get_ipstr(session.d->local_addr);

  /* Check for a MasqueradeAddress configuration record, and return that
   * addr if appropriate.
   */
  if ((c = find_config(main_server->conf, CONF_PARAM, "MasqueradeAddress",
      FALSE)) != NULL)
    addrstr = (char *) pr_netaddr_get_ipstr(c->argv[0]);

  /* Fixup the address string for the PASV response. */
  tmp = strrchr(addrstr, ':');
  if (tmp)
    addrstr = tmp + 1;

  for (tmp = addrstr; *tmp; tmp++)
    if (*tmp == '.')
      *tmp = ',';

  pr_log_debug(DEBUG1, "Entering Passive Mode (%s,%u,%u).", addrstr,
    (port >> 8) & 255, port & 255);

  /* Note: this response is specifically NOT localised because clients
   * assume this particular text.  Nice, huh?
   */
  pr_response_add(R_227, "Entering Passive Mode (%s,%u,%u).", addrstr,
    (port >> 8) & 255, port & 255);
 
  return PR_HANDLED(cmd);
}

MODRET core_port(cmd_rec *cmd) {
  pr_netaddr_t *port_addr = NULL;
#ifdef PR_USE_IPV6
  char buf[INET6_ADDRSTRLEN] = {'\0'};
#else
  char buf[INET_ADDRSTRLEN] = {'\0'};
#endif /* PR_USE_IPV6 */
  unsigned int h1, h2, h3, h4, p1, p2;
  unsigned short port;
  unsigned char *allow_foreign_addr = NULL, *privsdrop = NULL;

  if (session.sf_flags & SF_EPSV_ALL) {
    pr_response_add_err(R_500, _("Illegal PORT command, EPSV ALL in effect"));
    return PR_ERROR(cmd);
  }

  CHECK_CMD_ARGS(cmd, 2);

  /* Returning 501 is the best we can do.  It would be nicer if RFC959 allowed
   * 550 as a possible response.
   */
  if (!dir_check(cmd->tmp_pool, cmd, cmd->group, session.cwd, NULL)) {
    pr_log_debug(DEBUG8, "PORT denied by <Limit> configuration");
    pr_response_add_err(R_501, "%s: %s", cmd->argv[0], strerror(EPERM));
    return PR_ERROR(cmd);
  }

  /* Block active transfers (the PORT command) if RootRevoke is in effect
   * and the server's port is below 1025 (binding to the data port in this
   * case would require root privs, which will have been dropped.
   */
  privsdrop = get_param_ptr(TOPLEVEL_CONF, "RootRevoke", FALSE);
  if (privsdrop != NULL &&
      *privsdrop == TRUE &&
      session.c->local_port < 1025) {
    pr_log_debug(DEBUG0, "RootRevoke in effect, unable to bind to local "
      "port %d for active transfer", session.c->local_port);
    pr_response_add_err(R_500, _("Unable to service PORT commands"));
    return PR_ERROR(cmd);
  }

  /* Format is h1,h2,h3,h4,p1,p2 (ASCII in network order) */
  if (sscanf(cmd->argv[1], "%u,%u,%u,%u,%u,%u", &h1, &h2, &h3, &h4, &p1,
      &p2) != 6) {
    pr_log_debug(DEBUG2, "PORT '%s' is not syntactically valid", cmd->argv[1]);
    pr_response_add_err(R_501, _("Illegal PORT command"));
    return PR_ERROR(cmd);
  }

  if (h1 > 255 || h2 > 255 || h3 > 255 || h4 > 255 || p1 > 255 || p2 > 255 ||
      (h1|h2|h3|h4) == 0 || (p1|p2) == 0) {
    pr_log_debug(DEBUG2, "PORT '%s' has invalid value(s)", cmd->arg);
    pr_response_add_err(R_501, _("Illegal PORT command"));
    return PR_ERROR(cmd);
  }
  port = ((p1 << 8) | p2);

#ifdef PR_USE_IPV6
  if (pr_netaddr_use_ipv6()) {
    if (pr_netaddr_get_family(session.c->remote_addr) == AF_INET6) {
      snprintf(buf, sizeof(buf), "::ffff:%u.%u.%u.%u", h1, h2, h3, h4);

    } else {
      snprintf(buf, sizeof(buf), "%u.%u.%u.%u", h1, h2, h3, h4);
    }

  } else
#endif /* PR_USE_IPV6 */
  snprintf(buf, sizeof(buf), "%u.%u.%u.%u", h1, h2, h3, h4);
  buf[sizeof(buf)-1] = '\0';

  port_addr = pr_netaddr_get_addr(cmd->tmp_pool, buf, NULL);
  if (port_addr == NULL) {
    pr_log_debug(DEBUG1, "error getting sockaddr for '%s': %s", buf,
      strerror(errno)); 
    pr_response_add_err(R_501, _("Illegal PORT command"));
    return PR_ERROR(cmd);
  }

  pr_netaddr_set_family(&session.data_addr, pr_netaddr_get_family(port_addr));
  pr_netaddr_set_port(&session.data_addr, htons(port));

  /* Make sure that the address specified matches the address from which
   * the control connection is coming.
   */

  allow_foreign_addr = get_param_ptr(TOPLEVEL_CONF, "AllowForeignAddress",
    FALSE);

  if (!allow_foreign_addr || *allow_foreign_addr == FALSE) {
    pr_netaddr_t *remote_addr = session.c->remote_addr;

#ifdef PR_USE_IPV6
    if (pr_netaddr_use_ipv6()) {
      /* We can only compare the PORT-given address against the remote client
       * address if the remote client address is an IPv4-mapped IPv6 address.
       */
      if (pr_netaddr_get_family(remote_addr) == AF_INET6 &&
          pr_netaddr_is_v4mappedv6(remote_addr) != TRUE) {
        pr_log_pri(PR_LOG_WARNING,
          "Refused PORT %s (IPv4/IPv6 address mismatch)", cmd->arg);
        pr_response_add_err(R_500, _("Illegal PORT command"));
        return PR_ERROR(cmd);
      }
    }
#endif /* PR_USE_IPV6 */

    if (pr_netaddr_cmp(port_addr, remote_addr) != 0) {
      pr_log_pri(PR_LOG_WARNING, "Refused PORT %s (address mismatch)",
        cmd->arg);
      pr_response_add_err(R_500, _("Illegal PORT command"));
      return PR_ERROR(cmd);
    }
  }

  /* Additionally, make sure that the port number used is a "high numbered"
   * port, to avoid bounce attacks.  For remote Windows machines, the
   * port numbers mean little.  However, there are also quite a few Unix
   * machines out there for whom the port number matters...
   */

  if (port < 1024) {
    pr_log_pri(PR_LOG_WARNING, "Refused PORT %s (bounce attack)", cmd->arg);
    pr_response_add_err(R_500, _("Illegal PORT command"));
    return PR_ERROR(cmd);
  }

  memcpy(&session.data_addr, port_addr, sizeof(session.data_addr));
  session.data_port = port;
  session.sf_flags &= (SF_ALL^SF_PASSIVE);

  /* If we already have a data connection open, kill it. */
  if (session.d) {
    pr_inet_close(session.d->pool, session.d);
    session.d = NULL;
  }

  session.sf_flags |= SF_PORT;
  pr_response_add(R_200, _("PORT command successful"));

  return PR_HANDLED(cmd);
}

MODRET core_eprt(cmd_rec *cmd) {
  pr_netaddr_t na;
  int family = 0;
  unsigned short port = 0;
  unsigned char *allow_foreign_addr = NULL, *privsdrop = NULL;
  char delim = '\0', *argstr = pstrdup(cmd->tmp_pool, cmd->argv[1]);
  char *tmp = NULL;

  if (session.sf_flags & SF_EPSV_ALL) {
    pr_response_add_err(R_500, _("Illegal PORT command, EPSV ALL in effect"));
    return PR_ERROR(cmd);
  }

  CHECK_CMD_ARGS(cmd, 2);

  /* Returning 501 is the best we can do.  It would be nicer if RFC959 allowed
   * 550 as a possible response.
   */
  if (!dir_check(cmd->tmp_pool, cmd, cmd->group, session.cwd, NULL)) {
    pr_log_debug(DEBUG8, "EPRT denied by <Limit> configuration");
    pr_response_add_err(R_501, "%s: %s", cmd->argv[0], strerror(EPERM));
    return PR_ERROR(cmd);
  }

  /* Initialize the netaddr. */
  pr_netaddr_clear(&na);

  /* Block active transfers (the EPRT command) if RootRevoke is in effect
   * and the server's port is below 1025 (binding to the data port in this
   * case would require root privs, which will have been dropped.
   */
  privsdrop = get_param_ptr(TOPLEVEL_CONF, "RootRevoke", FALSE);
  if (privsdrop != NULL &&
      *privsdrop == TRUE &&
      session.c->local_port < 1025) {
    pr_log_debug(DEBUG0, "RootRevoke in effect, unable to bind to local "
      "port %d for active transfer", session.c->local_port);
    pr_response_add_err(R_500, _("Unable to service EPRT commands"));
    return PR_ERROR(cmd);
  }

  /* Format is <d>proto<d>ip address<d>port<d> (ASCII in network order),
   * where <d> is an arbitrary delimiter character.
   */
  delim = *argstr++;

  /* atoi() will happily any trailing non-numeric characters, so feeding
   * the parameter string won't hurt.
   */
  family = atoi(argstr);

  switch (family) {
    case 1:
      break;

#ifdef PR_USE_IPV6
    case 2:
      if (pr_netaddr_use_ipv6())
        break;
#endif /* PR_USE_IPV6 */

    default:
#ifdef PR_USE_IPV6
      if (pr_netaddr_use_ipv6())
        pr_response_add_err(R_522, _("Network protocol not supported, use (1,2)"));
      else
        pr_response_add_err(R_522, _("Network protocol not supported, use (1)"));
#else
      pr_response_add_err(R_522, _("Network protocol not supported, use (1)"));
#endif /* PR_USE_IPV6 */
      return PR_ERROR(cmd);
  }

  /* Now, skip past those numeric characters that atoi() used. */
  while (isdigit((unsigned char) *argstr))
    argstr++;

  /* If the next character is not the delimiter, it's a badly formatted
   * parameter.
   */
  if (*argstr == delim)
    argstr++;

  else {
    pr_response_add_err(R_501, _("Illegal EPRT command"));
    return PR_ERROR(cmd);
  }

  if ((tmp = strchr(argstr, delim)) == NULL) {
    pr_log_debug(DEBUG3, "badly formatted EPRT argument: '%s'", cmd->argv[1]);
    pr_response_add_err(R_501, _("Illegal EPRT command"));
    return PR_ERROR(cmd);
  }

  /* Twiddle the string so that just the address portion will be processed
   * by pr_inet_pton().
   */
  *tmp = '\0';

  memset(&na, 0, sizeof(na));

  /* Use pr_inet_pton() to translate the address string into the address
   * value.
   */
  switch (family) {
    case 1: {
      struct sockaddr *sa = NULL;

      pr_netaddr_set_family(&na, AF_INET);
      sa = pr_netaddr_get_sockaddr(&na);
      if (sa)
        sa->sa_family = AF_INET;
      if (pr_inet_pton(AF_INET, argstr, pr_netaddr_get_inaddr(&na)) <= 0) {
        pr_log_debug(DEBUG2, "error converting IPv4 address '%s': %s",
          argstr, strerror(errno));
        pr_response_add_err(R_501, _("Illegal EPRT command"));
        return PR_ERROR(cmd);
      }
      break;
    }

    case 2: {
      struct sockaddr *sa = NULL;

      pr_netaddr_set_family(&na, AF_INET6);
      sa = pr_netaddr_get_sockaddr(&na);
      if (sa)
        sa->sa_family = AF_INET6;
      if (pr_inet_pton(AF_INET6, argstr, pr_netaddr_get_inaddr(&na)) <= 0) {
        pr_log_debug(DEBUG2, "error converting IPv6 address '%s': %s",
          argstr, strerror(errno));
        pr_response_add_err(R_501, _("Illegal EPRT command"));
        return PR_ERROR(cmd);
      }
      break;
    }
  }

  /* Advance past the address portion of the argument. */
  argstr = ++tmp;

  port = atoi(argstr);

  while (isdigit((unsigned char) *argstr))
    argstr++;

  /* If the next character is not the delimiter, it's a badly formatted
   * parameter.
   */
  if (*argstr != delim) {
    pr_log_debug(DEBUG3, "badly formatted EPRT argument: '%s'", cmd->argv[1]);
    pr_response_add_err(R_501, _("Illegal EPRT command"));
    return PR_ERROR(cmd);
  }

  /* Make sure that the address specified matches the address from which
   * the control connection is coming.
   */

  allow_foreign_addr = get_param_ptr(TOPLEVEL_CONF, "AllowForeignAddress",
    FALSE);

  if (!allow_foreign_addr || *allow_foreign_addr == FALSE) {
    if (pr_netaddr_cmp(&na, session.c->remote_addr) != 0 || !port) {
      pr_log_pri(PR_LOG_WARNING, "Refused EPRT %s (address mismatch)",
        cmd->arg);
      pr_response_add_err(R_500, _("Illegal EPRT command"));
      return PR_ERROR(cmd);
    }
  }

  /* Additionally, make sure that the port number used is a "high numbered"
   * port, to avoid bounce attacks.  For remote Windows machines, the
   * port numbers mean little.  However, there are also quite a few Unix
   * machines out there for whom the port number matters...
   */

  if (port < 1024) {
    pr_log_pri(PR_LOG_WARNING, "Refused EPRT %s (bounce attack)", cmd->arg);
    pr_response_add_err(R_500, _("Illegal EPRT command"));
    return PR_ERROR(cmd);
  }

  /* Make sure we're using network byte order. */
  pr_netaddr_set_port(&na, htons(port));

  switch (family) {
    case 1:
      pr_netaddr_set_family(&session.data_addr, AF_INET);
      break;

    case 2:
      pr_netaddr_set_family(&session.data_addr, AF_INET6);
      break;
  }

  pr_netaddr_set_sockaddr(&session.data_addr, pr_netaddr_get_sockaddr(&na));
  pr_netaddr_set_port(&session.data_addr, pr_netaddr_get_port(&na));
  session.data_port = port;
  session.sf_flags &= (SF_ALL^SF_PASSIVE);

  /* If we already have a data connection open, kill it. */
  if (session.d) {
    pr_inet_close(session.d->pool, session.d);
    session.d = NULL;
  }

  session.sf_flags |= SF_PORT;
  pr_response_add(R_200, _("EPRT command successful"));

  return PR_HANDLED(cmd);
}

MODRET core_epsv(cmd_rec *cmd) {
  char *addrstr = "";
  char *endp = NULL, *arg = NULL;
  int family = 0;
  int epsv_min_port = 1024, epsv_max_port = 65535;
  config_rec *c = NULL;

  CHECK_CMD_MIN_ARGS(cmd, 1);

  /* Returning 501 is the best we can do.  It would be nicer if RFC959 allowed
   * 550 as a possible response.
   */
  if (!dir_check(cmd->tmp_pool, cmd, cmd->group, session.cwd, NULL)) {
    pr_log_debug(DEBUG8, "EPSV denied by <Limit> configuration");
    pr_response_add_err(R_501, "%s: %s", cmd->argv[0], strerror(EPERM));
    return PR_ERROR(cmd);
  }

  if (cmd->argc-1 == 1)
    arg = pstrdup(cmd->tmp_pool, cmd->argv[1]);

  if (arg && strcasecmp(arg, "all") == 0) {
    session.sf_flags |= SF_EPSV_ALL;
    pr_response_add(R_200, _("EPSV ALL command successful"));
    return PR_HANDLED(cmd);
  }

  /* If the optional parameter was given, determine the address family from
   * that.  If not, determine the family from the control connection address
   * family.
   */
  if (arg) {
    family = strtol(arg, &endp, 10);

    if (endp && *endp) {
      pr_response_add_err(R_501, _("%s: unknown network protocol"),
        cmd->argv[0]);
      return PR_ERROR(cmd);
    }
 
  } else {

    switch (pr_netaddr_get_family(session.c->local_addr)) {
      case AF_INET:
        family = 1;
        break;

#ifdef PR_USE_IPV6
      case AF_INET6:
        if (pr_netaddr_use_ipv6()) {
          family = 2;
          break;
        }
#endif /* PR_USE_IPV6 */

      default:
        family = 0;
        break;
    }
  }

  switch (family) {
    case 1:
      break;

#ifdef PR_USE_IPV6
    case 2:
      if (pr_netaddr_use_ipv6())
        break;
#endif /* PR_USE_IPV6 */

    default:
#ifdef PR_USE_IPV6
      if (pr_netaddr_use_ipv6())
        pr_response_add_err(R_522, _("Network protocol not supported, use (1,2)"));
      else
        pr_response_add_err(R_522, _("Network protocol not supported, use (1)"));
#else
      pr_response_add_err(R_522, _("Network protocol not supported, use (1)"));
#endif /* PR_USE_IPV6 */
      return PR_ERROR(cmd);
  }

  /* If we already have a passive listen data connection open, kill it. */
  if (session.d) {
    pr_inet_close(session.d->pool, session.d);
    session.d = NULL;
  }

  c = find_config(main_server->conf, CONF_PARAM, "PassivePorts", FALSE);
  if (c != NULL) {
    epsv_min_port = *((int *) c->argv[0]);
    epsv_max_port = *((int *) c->argv[1]);
  }

  /* We always use the portrange variant of inet_create_conn() here,
   * since it seems that some Unix kernels have issues when choosing a
   * random port number for IPv6 sockets (see Bug #2900).  By using the
   * portrange variant, proftpd, and not the kernel, will be the one
   * choosing the port number.  We really only need to do this for EPSV
   * and not PASV since only the EPSV command can be used for IPv6
   * connections; using PASV means IPv4 connections, and Unix kernels
   * have more predictable behavior for choosing random IPv4 socket ports.
   */

  session.d = pr_inet_create_conn_portrange(session.pool, NULL,
    session.c->local_addr, epsv_min_port, epsv_max_port);

  if (session.d == NULL) {
    /* If not able to open a passive port in the given range, default to
     * normal behavior (using INPORT_ANY), and log the failure.  This
     * indicates a too-small range configuration.
     */
    pr_log_pri(PR_LOG_WARNING, "unable to find open port in PassivePorts "
      "range %d-%d: defaulting to INPORT_ANY (consider defining a larger "
      "PassivePorts range)", epsv_min_port, epsv_max_port);

    session.d = pr_inet_create_conn(session.pool, NULL, -1,
      session.c->local_addr, INPORT_ANY, FALSE);
  }

  if (session.d == NULL) {
    pr_response_add_err(R_425,
      _("Unable to build data connection: Internal error"));
    return PR_ERROR(cmd);
  }

  pr_inet_set_block(session.pool, session.d);
  pr_inet_listen(session.pool, session.d, 1);

  session.d->instrm = pr_netio_open(session.pool, PR_NETIO_STRM_DATA,
    session.d->listen_fd, PR_NETIO_IO_RD);

  /* Now tell the client our address/port. */
  session.data_port = session.d->local_port;
  session.sf_flags |= SF_PASSIVE;

  /* Note: what about masquerading IPv6 addresses?  It seems that RFC2428,
   * which defines the EPSV command, does not explicitly handle the
   * case where the server may wish to return a network address in its
   * EPSV response.  The assumption is that in an IPv6 environment, there
   * will be no need for NAT, and hence no need for masquerading.  This
   * may be true in an ideal world, but I think it more likely that current
   * clients will simply use EPSV, rather than PASV, in existing IPv4 networks.
   *
   * Disable the honoring of MasqueradeAddress for EPSV until this can
   * be officially determined (Bug#2369).
   */
#if 0
  if ((c = find_config(main_server->conf, CONF_PARAM, "MasqueradeAddress",
      FALSE)) != NULL)
   addrstr = (char *) pr_netaddr_get_ipstr(c->argv[0]);
#endif

  pr_log_debug(DEBUG1, "Entering Extended Passive Mode (||%s|%u|)",
    addrstr, (unsigned int) session.data_port);
  pr_response_add(R_229, "Entering Extended Passive Mode (||%s|%u|)",
    addrstr, (unsigned int) session.data_port);

  return PR_HANDLED(cmd);
}

MODRET core_help(cmd_rec *cmd) {

  if (cmd->argc == 1) {
    pr_help_add_response(cmd, NULL);

  } else {
    char *cp;

    for (cp = cmd->argv[1]; *cp; cp++)
      *cp = toupper(*cp);

    if (strcasecmp(cmd->argv[1], "SITE") == 0)
      return pr_module_call(&site_module, site_dispatch, cmd);

    if (pr_help_add_response(cmd, cmd->argv[1]) == 0)
      return PR_HANDLED(cmd);

    pr_response_add_err(R_502, _("Unknown command '%s'"), cmd->argv[1]);
    return PR_ERROR(cmd);
  }

  return PR_HANDLED(cmd);
}

MODRET core_syst(cmd_rec *cmd) {
  pr_response_add(R_215, "UNIX Type: L8");
  return PR_HANDLED(cmd);
}

int core_chgrp(cmd_rec *cmd, char *dir, uid_t uid, gid_t gid) {
  char *cmd_name;

  cmd_name = cmd->argv[0];
  cmd->argv[0] = "SITE_CHGRP";
  if (!dir_check(cmd->tmp_pool, cmd, "WRITE", dir, NULL)) {
    pr_log_debug(DEBUG7, "SITE CHGRP command denied by <Limit> config");
    cmd->argv[0] = cmd_name;

    errno = EACCES;
    return -1;
  }
  cmd->argv[0] = cmd_name;

  return pr_fsio_chown(dir, uid, gid);
}

int core_chmod(cmd_rec *cmd, char *dir, mode_t mode) {
  char *cmd_name;

  cmd_name = cmd->argv[0];
  cmd->argv[0] = "SITE_CHMOD";
  if (!dir_check(cmd->tmp_pool, cmd, "WRITE", dir, NULL)) {
    pr_log_debug(DEBUG7, "SITE CHMOD command denied by <Limit> config");
    cmd->argv[0] = cmd_name;

    errno = EACCES;
    return -1;
  }
  cmd->argv[0] = cmd_name;

  return pr_fsio_chmod(dir,mode);
}

MODRET _chdir(cmd_rec *cmd, char *ndir) {
  char *dir, *odir, *cdir;
  config_rec *c = NULL, *cdpath;
  unsigned char show_symlinks = TRUE, *tmp = NULL;

  odir = ndir;
  pr_fs_clear_cache();

  tmp = get_param_ptr(TOPLEVEL_CONF, "ShowSymlinks", FALSE);
  if (tmp != NULL)
    show_symlinks = *tmp;

  if (show_symlinks) {
    int use_cdpath = FALSE;

    dir = dir_realpath(cmd->tmp_pool, ndir);

    if (!dir) {
      use_cdpath = TRUE;
    }

    if (!use_cdpath) {
      int allowed_access = TRUE;

      allowed_access = dir_check_full(cmd->tmp_pool, cmd, cmd->group, dir,
        NULL);
      if (!allowed_access)
        use_cdpath = TRUE;
    }

    if (!use_cdpath &&
        pr_fsio_chdir(dir, 0) < 0) {
      use_cdpath = TRUE;
    }

    if (use_cdpath) {
      for (cdpath = find_config(main_server->conf, CONF_PARAM, "CDPath", TRUE);
          cdpath != NULL; cdpath =
            find_config_next(cdpath,cdpath->next,CONF_PARAM,"CDPath",TRUE)) {
        cdir = (char *) malloc(strlen(cdpath->argv[0]) + strlen(ndir) + 2);
        snprintf(cdir, strlen(cdpath->argv[0]) + strlen(ndir) + 2,
                 "%s%s%s", (char *) cdpath->argv[0],
                 ((char *) cdpath->argv[0])[strlen(cdpath->argv[0]) - 1] == '/' ? "" : "/",
                 ndir);
        dir = dir_realpath(cmd->tmp_pool, cdir);
        free(cdir);

        if (dir &&
            dir_check_full(cmd->tmp_pool, cmd, cmd->group, dir, NULL) &&
            pr_fsio_chdir(dir, 0) == 0) {
          break;
        }
      }

      if (!cdpath) {
        pr_response_add_err(R_550, "%s: %s", odir, strerror(errno));
        return PR_ERROR(cmd);
      }
    }

  } else {
    int use_cdpath = FALSE;

    /* Virtualize the chdir */
    ndir = dir_canonical_vpath(cmd->tmp_pool, ndir);
    dir = dir_realpath(cmd->tmp_pool, ndir);

    if (!dir) {
      use_cdpath = TRUE;
    }

    if (!use_cdpath) {
      int allowed_access = TRUE;

      allowed_access = dir_check_full(cmd->tmp_pool, cmd, cmd->group, dir,
        NULL);
      if (!allowed_access)
        use_cdpath = TRUE;
    }

    if (!use_cdpath &&
        pr_fsio_chdir_canon(ndir, 1) < 0) {
      use_cdpath = TRUE;
    }            

    if (use_cdpath) {
      for (cdpath = find_config(main_server->conf,CONF_PARAM,"CDPath",TRUE);
          cdpath != NULL; cdpath =
            find_config_next(cdpath,cdpath->next,CONF_PARAM,"CDPath",TRUE)) {
        cdir = (char *) malloc(strlen(cdpath->argv[0]) + strlen(ndir) + 2);
        snprintf(cdir, strlen(cdpath->argv[0]) + strlen(ndir) + 2,
                 "%s%s%s", (char *) cdpath->argv[0],
                ((char *)cdpath->argv[0])[strlen(cdpath->argv[0]) - 1] == '/' ? "" : "/",
                ndir);
        ndir = dir_canonical_vpath(cmd->tmp_pool, cdir);
        dir = dir_realpath(cmd->tmp_pool, ndir);
        free(cdir);

        if (dir &&
            dir_check_full(cmd->tmp_pool, cmd, cmd->group, dir, NULL) &&
            pr_fsio_chdir_canon(ndir, 1) != -1) {
          break;
        }
      }

      if (!cdpath) {
        pr_response_add_err(R_550, "%s: %s", odir, strerror(errno));
        return PR_ERROR(cmd);
      }
    }
  }

  sstrncpy(session.cwd, pr_fs_getcwd(), sizeof(session.cwd));
  sstrncpy(session.vwd, pr_fs_getvwd(), sizeof(session.vwd));

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CWD, session.cwd,
    NULL);

  if (session.dir_config) {
    c = find_config(session.dir_config->subset, CONF_PARAM, "DisplayChdir",
      FALSE);
  }

  if (!c &&
      session.anon_config) {
    c = find_config(session.anon_config->subset, CONF_PARAM, "DisplayChdir",
      FALSE);
  }

  if (!c) {
    c = find_config(cmd->server->conf, CONF_PARAM, "DisplayChdir", FALSE);
  }

  if (c) {
    struct stat st;
    time_t prev;

    char *display = c->argv[0];
    int bool = *((int *) c->argv[1]);

    if (bool) {
   
      /* XXX Get rid of this CONF_USERDATA instance; it's the only
       * occurrence of it in the source.
       */ 
      c = find_config(cmd->server->conf, CONF_USERDATA, session.cwd, FALSE);

      if (!c) {
        time(&prev);
        c = add_config_set(&cmd->server->conf, session.cwd);
        c->config_type = CONF_USERDATA;
        c->argc = 1;
        c->argv = pcalloc(c->pool, sizeof(void **) * 2);
        c->argv[0] = (void *) prev;
        prev = (time_t) 0L;

      } else {
        prev = (time_t) c->argv[0];
        c->argv[0] = (void *) time(NULL);
      }
    }

    if (pr_fsio_stat(display, &st) != -1 &&
        !S_ISDIR(st.st_mode) &&
        (bool ? st.st_mtime > prev : TRUE)) {

      if (pr_display_file(display, session.cwd, R_250) < 0) {
        pr_log_debug(DEBUG3, "error displaying '%s': %s", display,
          strerror(errno));
      }
    }
  }

  pr_response_add(R_250, _("%s command successful"), cmd->argv[0]);
  return PR_HANDLED(cmd);
}

MODRET core_rmd(cmd_rec *cmd) {
  int res;
  char *dir;

  CHECK_CMD_MIN_ARGS(cmd, 2);

  dir = pr_fs_decode_path(cmd->tmp_pool, cmd->arg);

  res = pr_filter_allow_path(CURRENT_CONF, dir);
  switch (res) {
    case 0:
      break;

    case PR_FILTER_ERR_FAILS_ALLOW_FILTER:
      pr_log_debug(DEBUG2, "'%s %s' denied by PathAllowFilter", cmd->argv[0],
        dir);
      pr_response_add_err(R_550, _("%s: Forbidden filename"), cmd->arg);
      return PR_ERROR(cmd); 
 
    case PR_FILTER_ERR_FAILS_DENY_FILTER:
      pr_log_debug(DEBUG2, "'%s %s' denied by PathDenyFilter", cmd->argv[0],
        dir);
      pr_response_add_err(R_550, _("%s: Forbidden filename"), cmd->arg);
      return PR_ERROR(cmd);
  }

  /* If told to rmdir a symlink to a directory, don't; you can't rmdir a
   * symlink, you delete it.
   */
  dir = dir_canonical_path(cmd->tmp_pool, dir);

  if (!dir) {
    pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(EINVAL));
    return PR_ERROR(cmd);
  }

  if (!dir_check_canon(cmd->tmp_pool, cmd, cmd->group, dir, NULL)) {
    pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(EACCES));
    return PR_ERROR(cmd);
  }

  if (pr_fsio_rmdir(dir) < 0) {
    int xerrno = errno;

    (void) pr_trace_msg("fileperms", 1, "%s, user '%s' (UID %lu, GID %lu): "
      "error removing directory '%s': %s", cmd->argv[0], session.user,
      (unsigned long) session.uid, (unsigned long) session.gid, dir,
      strerror(xerrno));

    pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(xerrno));
    return PR_ERROR(cmd);
  }

  pr_response_add(R_250, _("%s command successful"), cmd->argv[0]);
  return PR_HANDLED(cmd);
}

MODRET core_mkd(cmd_rec *cmd) {
  int res;
  char *dir;
  struct stat st;

  CHECK_CMD_MIN_ARGS(cmd, 2);

  if (strchr(cmd->arg, '*')) {
    pr_response_add_err(R_550, _("%s: Invalid directory name"), cmd->arg);
    return PR_ERROR(cmd);
  }

  dir = pr_fs_decode_path(cmd->tmp_pool, cmd->arg);

  res = pr_filter_allow_path(CURRENT_CONF, dir);
  switch (res) {
    case 0:
      break;

    case PR_FILTER_ERR_FAILS_ALLOW_FILTER:
      pr_log_debug(DEBUG2, "'%s %s' denied by PathAllowFilter", cmd->argv[0],
        dir);
      pr_response_add_err(R_550, _("%s: Forbidden filename"), cmd->arg);
      return PR_ERROR(cmd); 
 
    case PR_FILTER_ERR_FAILS_DENY_FILTER:
      pr_log_debug(DEBUG2, "'%s %s' denied by PathDenyFilter", cmd->argv[0],
        dir);
      pr_response_add_err(R_550, _("%s: Forbidden filename"), cmd->arg);
      return PR_ERROR(cmd);
  }

  dir = dir_canonical_path(cmd->tmp_pool, dir);
  if (!dir) {
    pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(EINVAL));
    return PR_ERROR(cmd);
  }

  if (!dir_check_canon(cmd->tmp_pool, cmd, cmd->group, dir, NULL)) {
    pr_log_debug(DEBUG8, "%s command denied by <Limit> config", cmd->argv[0]);
    pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(EACCES));
    return PR_ERROR(cmd);
  }

  if (pr_fsio_mkdir(dir, 0777) < 0) {
    int xerrno = errno;

    (void) pr_trace_msg("fileperms", 1, "%s, user '%s' (UID %lu, GID %lu): "
      "error making directory '%s': %s", cmd->argv[0], session.user,
      (unsigned long) session.uid, (unsigned long) session.gid, dir,
      strerror(xerrno));

    pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(xerrno));
    return PR_ERROR(cmd);
  }

  /* Check to see if we need to change the ownership (user and/or group) of
   * the newly created directory.
   */
  if (session.fsuid != (uid_t) -1) {
    int err = 0, iserr = 0;

    pr_fsio_stat(dir, &st);

    PRIVS_ROOT
    if (pr_fsio_chown(dir, session.fsuid, session.fsgid) == -1) {
      iserr++;
      err = errno;
    }
    PRIVS_RELINQUISH

    if (iserr) {
      pr_log_pri(PR_LOG_WARNING, "chown() as root failed: %s", strerror(err));

    } else {
      if (session.fsgid != (gid_t) -1) {
        pr_log_debug(DEBUG2, "root chown(%s) to uid %lu, gid %lu successful",
          dir, (unsigned long) session.fsuid, (unsigned long) session.fsgid);

      } else {
        pr_log_debug(DEBUG2, "root chown(%s) to uid %lu successful", dir,
          (unsigned long) session.fsuid);
      }
    }

  } else if (session.fsgid != (gid_t) -1) {
    register unsigned int i;
    int use_root_privs = TRUE;

    pr_fsio_stat(dir, &st);

    /* Check if session.fsgid is in session.gids.  If not, use root privs.  */
    for (i = 0; i < session.gids->nelts; i++) {
      gid_t *group_ids = session.gids->elts;

      if (group_ids[i] == session.fsgid) {
        use_root_privs = FALSE;
        break;
      }
    }

    if (use_root_privs) {
      PRIVS_ROOT
    }

    res = pr_fsio_chown(dir, (uid_t) -1, session.fsgid);

    if (use_root_privs) {
      PRIVS_RELINQUISH
    }

    if (res == -1) {
      pr_log_pri(PR_LOG_WARNING, "%schown() failed: %s",
        use_root_privs ? "root " : "", strerror(errno));

    } else { 
      pr_log_debug(DEBUG2, "%schown(%s) to gid %lu successful",
        use_root_privs ? "root " : "", dir, (unsigned long) session.fsgid);
    }
  }

  pr_response_add(R_257, _("\"%s\" - Directory successfully created"),
    quote_dir(cmd, dir));

  return PR_HANDLED(cmd);
}

MODRET core_cwd(cmd_rec *cmd) {
  char *dir;
  CHECK_CMD_MIN_ARGS(cmd, 2);

  dir = pr_fs_decode_path(cmd->tmp_pool, cmd->arg);
  return _chdir(cmd, dir);
}

MODRET core_cdup(cmd_rec *cmd) {
  CHECK_CMD_ARGS(cmd, 1);
  return _chdir(cmd, "..");
}

/* Returns the modification time of a file, as per RFC3659.
 */
MODRET core_mdtm(cmd_rec *cmd) {
  char *path;
  struct stat st;

  CHECK_CMD_MIN_ARGS(cmd, 2);

  path = dir_realpath(cmd->tmp_pool,
    pr_fs_decode_path(cmd->tmp_pool, cmd->arg));

  if (!path ||
      !dir_check(cmd->tmp_pool, cmd, cmd->group, path, NULL) ||
      pr_fsio_stat(path, &st) == -1) {
    pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(errno));
    return PR_ERROR(cmd);

  } else {

    if (!S_ISREG(st.st_mode)) {
      pr_response_add_err(R_550, _("%s: not a plain file"), cmd->arg);
      return PR_ERROR(cmd);

    } else {
      char buf[16];
      struct tm *tm;

      memset(buf, '\0', sizeof(buf));

      tm = pr_gmtime(cmd->tmp_pool, &st.st_mtime);
      if (tm) {
        snprintf(buf, sizeof(buf), "%04d%02d%02d%02d%02d%02d",
          tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour,
          tm->tm_min, tm->tm_sec);

      } else {
        snprintf(buf, sizeof(buf), "00000000000000");
      }

      pr_response_add(R_213, "%s", buf);
    }
  }

  return PR_HANDLED(cmd);
}

MODRET core_size(cmd_rec *cmd) {
  char *path;
  struct stat st;

  CHECK_CMD_MIN_ARGS(cmd, 2);

  /* Refuse the command if we're in ASCII mode. */
  if (session.sf_flags & SF_ASCII) {
    pr_log_debug(DEBUG5, "%s not allowed in ASCII mode", cmd->argv[0]);
    pr_response_add_err(R_550, _("%s not allowed in ASCII mode"), cmd->argv[0]);
    return PR_ERROR(cmd);
  }

  path = dir_realpath(cmd->tmp_pool,
    pr_fs_decode_path(cmd->tmp_pool, cmd->arg));

  pr_fs_clear_cache();

  if (!path ||
      !dir_check(cmd->tmp_pool, cmd, cmd->group, path, NULL) ||
      pr_fsio_stat(path, &st) == -1) {
    pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(errno));
    return PR_ERROR(cmd);

  } else {
    if (!S_ISREG(st.st_mode)) {
      pr_response_add_err(R_550, _("%s: not a regular file"), cmd->arg);
      return PR_ERROR(cmd);

    } else
      pr_response_add(R_213, "%" PR_LU, (pr_off_t) st.st_size);
  }

  return PR_HANDLED(cmd);
}

MODRET core_dele(cmd_rec *cmd) {
  int res;
  char *path, *fullpath;
  struct stat st;

  CHECK_CMD_MIN_ARGS(cmd, 2);

  path = pr_fs_decode_path(cmd->tmp_pool, cmd->arg);

  res = pr_filter_allow_path(CURRENT_CONF, path);
  switch (res) {
    case 0:
      break;

    case PR_FILTER_ERR_FAILS_ALLOW_FILTER:
      pr_log_debug(DEBUG2, "'%s %s' denied by PathAllowFilter", cmd->argv[0],
        path);
      pr_response_add_err(R_550, _("%s: Forbidden filename"), cmd->arg);
      return PR_ERROR(cmd); 
 
    case PR_FILTER_ERR_FAILS_DENY_FILTER:
      pr_log_debug(DEBUG2, "'%s %s' denied by PathDenyFilter", cmd->argv[0],
        path);
      pr_response_add_err(R_550, _("%s: Forbidden filename"), cmd->arg);
      return PR_ERROR(cmd);
  }

  /* If told to delete a symlink, don't delete the file it points to!  */
  path = dir_canonical_path(cmd->tmp_pool, path);
  if (!path) {
    pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(ENOENT));
    return PR_ERROR(cmd);
  }

  if (!dir_check_canon(cmd->tmp_pool, cmd, cmd->group, path, NULL)) {
    pr_log_debug(DEBUG7, "deleting '%s' denied by <Limit> configuration", path);
    pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(errno));
    return PR_ERROR(cmd);
  }

  /* Stat the path, before it is deleted, so that the size of the file
   * being deleted can be logged.  Note that unlink() doesn't follow symlinks,
   * so we need to use lstat(), not stat(), lest we log the wrong size.
   */
  memset(&st, 0, sizeof(st));
  pr_fs_clear_cache();
  if (pr_fsio_lstat(path, &st) < 0) {
    pr_log_debug(DEBUG3, "unable to lstat '%s': %s", path, strerror(errno));
    pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(errno));
    return PR_ERROR(cmd);
  }

#ifdef EISDIR
  /* If the path is a directory, try to return a good error message (e.g.
   * EISDIR).
   */
  if (S_ISDIR(st.st_mode)) {
    (void) pr_trace_msg("fileperms", 1, "%s, user '%s' (UID %lu, GID %lu): "
      "error deleting '%s': %s", cmd->argv[0], session.user,
      (unsigned long) session.uid, (unsigned long) session.gid, path,
      strerror(EISDIR));

    pr_log_debug(DEBUG3, "error deleting '%s': %s", path, strerror(EISDIR));
    pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(EISDIR));
    return PR_ERROR(cmd);
  }
#endif /* !EISDIR */
 
  if (pr_fsio_unlink(path) < 0) {
    int xerrno = errno;

    (void) pr_trace_msg("fileperms", 1, "%s, user '%s' (UID %lu, GID %lu): "
      "error deleting '%s': %s", cmd->argv[0], session.user,
      (unsigned long) session.uid, (unsigned long) session.gid, path,
      strerror(xerrno));

    pr_log_debug(DEBUG3, "error deleting '%s': %s", path, strerror(xerrno));
    pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(xerrno));
    return PR_ERROR(cmd);
  }

  fullpath = dir_abs_path(cmd->tmp_pool, path, TRUE);

  if (session.sf_flags & SF_ANON) {
    xferlog_write(0, session.c->remote_name, st.st_size, fullpath,
      (session.sf_flags & SF_ASCII ? 'a' : 'b'), 'd', 'a', session.anon_user,
      'c');

  } else {
    xferlog_write(0, session.c->remote_name, st.st_size, fullpath,
      (session.sf_flags & SF_ASCII ? 'a' : 'b'), 'd', 'r', session.user, 'c');
  }

  pr_response_add(R_250, _("%s command successful"), cmd->argv[0]);
  return PR_HANDLED(cmd);
}

MODRET core_rnto(cmd_rec *cmd) {
  int res;
  char *path;
  unsigned char *allow_overwrite = NULL;
  struct stat st;

  CHECK_CMD_MIN_ARGS(cmd, 2);

  if (!session.xfer.path) {
    if (session.xfer.p) {
      destroy_pool(session.xfer.p);
      memset(&session.xfer, '\0', sizeof(session.xfer));
    }

    pr_response_add_err(R_503, _("Bad sequence of commands"));
    return PR_ERROR(cmd);
  }

  path = pr_fs_decode_path(cmd->tmp_pool, cmd->arg);

  res = pr_filter_allow_path(CURRENT_CONF, path);
  switch (res) {
    case 0:
      break;

    case PR_FILTER_ERR_FAILS_ALLOW_FILTER:
      pr_log_debug(DEBUG2, "'%s %s' denied by PathAllowFilter", cmd->argv[0],
        path);
      pr_response_add_err(R_550, _("%s: Forbidden filename"), cmd->arg);
      return PR_ERROR(cmd); 
 
    case PR_FILTER_ERR_FAILS_DENY_FILTER:
      pr_log_debug(DEBUG2, "'%s %s' denied by PathDenyFilter", cmd->argv[0],
        path);
      pr_response_add_err(R_550, _("%s: Forbidden filename"), cmd->arg);
      return PR_ERROR(cmd);
  }

  path = dir_canonical_path(cmd->tmp_pool, path);

  allow_overwrite = get_param_ptr(CURRENT_CONF, "AllowOverwrite", FALSE);

  /* Deny the rename if AllowOverwrites are not allowed, and the destination
   * rename file already exists.
   */
  pr_fs_clear_cache();
  if ((!allow_overwrite || *allow_overwrite == FALSE) &&
      pr_fsio_stat(path, &st) == 0) {
    pr_log_debug(DEBUG6, "AllowOverwrite denied permission for %s", path);
    pr_response_add_err(R_550, _("%s: Rename permission denied"), cmd->arg);
    return PR_ERROR(cmd);
  }

  if (!path ||
      !dir_check_canon(cmd->tmp_pool, cmd, cmd->group, path, NULL) ||
      pr_fsio_rename(session.xfer.path, path) == -1) {
    int xerrno = errno;

    if (xerrno != EXDEV) {
      (void) pr_trace_msg("fileperms", 1, "%s, user '%s' (UID %lu, GID %lu): "
        "error renaming '%s' to '%s': %s", cmd->argv[0], session.user,
        (unsigned long) session.uid, (unsigned long) session.gid,
        session.xfer.path, path, strerror(xerrno));

      pr_response_add_err(R_550, _("Rename %s: %s"), cmd->arg, strerror(xerrno));
      return PR_ERROR(cmd);
    }

    /* In this case, we'll need to manually copy the file from the source
     * to the destination paths.
     */
    if (pr_fs_copy_file(session.xfer.path, path) < 0) {
      xerrno = errno;

      (void) pr_trace_msg("fileperms", 1, "%s, user '%s' (UID %lu, GID %lu): "
        "error copying '%s' to '%s': %s", cmd->argv[0], session.user,
        (unsigned long) session.uid, (unsigned long) session.gid,
        session.xfer.path, path, strerror(xerrno));

      pr_response_add_err(R_550, _("Rename %s: %s"), cmd->arg,
        strerror(xerrno));
      return PR_ERROR(cmd);
    }

    /* Once copied, unlink the original file. */
    if (pr_fsio_unlink(session.xfer.path) < 0) {
      pr_log_debug(DEBUG0, "error unlinking '%s': %s", session.xfer.path,
        strerror(errno));
    }
  }

  /* Change the xfer path to the name of the destination file, for logging. */
  session.xfer.path = pstrdup(session.xfer.p, path);

  pr_response_add(R_250, _("Rename successful"));
  return PR_HANDLED(cmd);
}

MODRET core_rnto_cleanup(cmd_rec *cmd) {
  if (session.xfer.p)
    destroy_pool(session.xfer.p);

  memset(&session.xfer, '\0', sizeof(session.xfer));

  pr_table_remove(session.notes, "mod_core.rnfr-path", NULL);
  return PR_DECLINED(cmd);
}

MODRET core_rnfr(cmd_rec *cmd) {
  int res;
  char *path;

  CHECK_CMD_MIN_ARGS(cmd, 2);

  path = pr_fs_decode_path(cmd->tmp_pool, cmd->arg);

  res = pr_filter_allow_path(CURRENT_CONF, path);
  switch (res) {
    case 0:
      break;

    case PR_FILTER_ERR_FAILS_ALLOW_FILTER:
      pr_log_debug(DEBUG2, "'%s %s' denied by PathAllowFilter", cmd->argv[0],
        path);
      pr_response_add_err(R_550, _("%s: Forbidden filename"), cmd->arg);
      return PR_ERROR(cmd); 
 
    case PR_FILTER_ERR_FAILS_DENY_FILTER:
      pr_log_debug(DEBUG2, "'%s %s' denied by PathDenyFilter", cmd->argv[0],
        path);
      pr_response_add_err(R_550, _("%s: Forbidden filename"), cmd->arg);
      return PR_ERROR(cmd);
  }

  /* Allow renaming a symlink, even a dangling one. */
  path = dir_canonical_path(cmd->tmp_pool, path);

  if (!path ||
      !dir_check_canon(cmd->tmp_pool, cmd, cmd->group, path, NULL) ||
      !exists(path)) {
    pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(errno));
    return PR_ERROR(cmd);
  }

  /* We store the path in session.xfer.path */
  if (session.xfer.p) {
    destroy_pool(session.xfer.p);
    memset(&session.xfer, '\0', sizeof(session.xfer));
  }

  session.xfer.p = make_sub_pool(session.pool);
  pr_pool_tag(session.xfer.p, "session xfer pool");

  session.xfer.path = pstrdup(session.xfer.p, path);

  pr_table_add(session.notes, "mod_core.rnfr-path",
    pstrdup(session.xfer.p, session.xfer.path), 0);

  pr_response_add(R_350, _("File or directory exists, ready for "
    "destination name"));

  return PR_HANDLED(cmd);
}

MODRET core_noop(cmd_rec *cmd) {
  pr_response_add(R_200, _("NOOP command successful"));
  return PR_HANDLED(cmd);
}

MODRET core_feat(cmd_rec *cmd) {
  const char *feat = NULL;
  CHECK_CMD_ARGS(cmd, 1);

  if (!dir_check(cmd->tmp_pool, cmd, cmd->group, session.vwd, NULL)) {
    pr_log_debug(DEBUG3, "%s command denied by <Limit> configuration",
      cmd->argv[0]);
    pr_response_add_err(R_550, "%s: %s", cmd->argv[0], strerror(EPERM));
    return PR_ERROR(cmd);
  }

  feat = pr_feat_get();
  if (feat) {
    feat = pstrcat(cmd->tmp_pool, _("Features:\r\n "), feat, NULL);
    while (TRUE) {
      const char *next;

      pr_signals_handle();

      next = pr_feat_get_next();
      if (next == NULL) {
        break;
      }

      feat = pstrcat(cmd->tmp_pool, feat, "\r\n ", next, NULL);
    }

    pr_response_add(R_211, "%s", feat);
    pr_response_add(R_DUP, _("End"));

  } else {
    pr_response_add(R_211, _("No features supported"));
  }

  return PR_HANDLED(cmd);
}

MODRET core_opts(cmd_rec *cmd) {
  register unsigned int i;
  int res;
  char *arg = "";
  cmd_rec *subcmd;

  CHECK_CMD_MIN_ARGS(cmd, 2);

  subcmd = pr_cmd_alloc(cmd->tmp_pool, cmd->argc-1, NULL);
  subcmd->argv[0] = pstrcat(cmd->tmp_pool, "OPTS_", cmd->argv[1], NULL);

  for (i = 2; i < cmd->argc; i++) {
    subcmd->argv[i-1] = cmd->argv[i];

    arg = pstrcat(cmd->tmp_pool, arg, *arg ? " " : "", cmd->argv[i], NULL);
  }

  subcmd->arg = arg;

  res = pr_cmd_dispatch(subcmd);
  if (res < 0)
    return PR_ERROR(cmd);

  return PR_HANDLED(cmd);
}

MODRET core_post_pass(cmd_rec *cmd) {
  config_rec *c;

  c = find_config(TOPLEVEL_CONF, CONF_PARAM, "TimeoutIdle", FALSE);
  if (c != NULL) {
    int prev_timeout, timeout;

    prev_timeout = pr_data_get_timeout(PR_DATA_TIMEOUT_IDLE);
    timeout = *((int *) c->argv[0]);

    if (timeout != prev_timeout) {
      pr_data_set_timeout(PR_DATA_TIMEOUT_IDLE, timeout);

      /* Remove the old timer, and add a new one with the changed
       * timeout value.
       */
      pr_timer_remove(PR_TIMER_IDLE, &core_module);

      if (timeout > 0) {
        pr_timer_add(timeout, PR_TIMER_IDLE, NULL, core_idle_timeout_cb,
          "TimeoutIdle");
      }
    }
  }

  /* Note: we MUST return HANDLED here, not DECLINED, to indicate that at
   * least one POST_CMD handler of the PASS command succeeded.  Since
   * mod_core is always the last module to which commands are dispatched,
   * we can rest assured that we are not causing problems for any other
   * PASS POST_CMD handlers by returning HANDLED here.
   */
  return PR_HANDLED(cmd);
}

MODRET set_defaulttransfermode(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if (strcasecmp(cmd->argv[1], "ascii") != 0 &&
      strcasecmp(cmd->argv[1], "binary") != 0)
    CONF_ERROR(cmd, "parameter must be 'ascii' or 'binary'");

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);

  return PR_HANDLED(cmd);
}

MODRET set_deferwelcome(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = bool;

  return PR_HANDLED(cmd);
}

/* Variable handlers
 */

static const char *core_get_sess_bytes_str(void *data, size_t datasz) {
  char buf[256];
  off_t bytes = *((off_t *) data);

  memset(buf, '\0', sizeof(buf));
  snprintf(buf, sizeof(buf)-1, "%" PR_LU, (pr_off_t) bytes);

  return pstrdup(session.pool, buf);
}

static const char *core_get_sess_files_str(void *data, size_t datasz) {
  char buf[256];
  unsigned int files = *((unsigned int *) data);

  memset(buf, '\0', sizeof(buf));
  snprintf(buf, sizeof(buf)-1, "%u", files);

  return pstrdup(session.pool, buf);
}

static const char *core_get_xfer_bytes_str(void *data, size_t datasz) {
  char buf[256];
  off_t bytes = *((off_t *) data);

  memset(buf, '\0', sizeof(buf));
  snprintf(buf, sizeof(buf)-1, "%" PR_LU, (pr_off_t) bytes);

  return pstrdup(session.pool, buf);
}

/* Event handlers
 */

static void core_restart_ev(const void *event_data, void *user_data) {
  pr_scoreboard_scrub();

#ifdef PR_USE_TRACE
  if (trace_log) {
    pr_trace_set_level(PR_TRACE_DEFAULT_CHANNEL, -1);
    pr_trace_set_file(NULL);
    trace_log = NULL;
  }
#endif /* PR_USE_TRACE */
}

static void core_startup_ev(const void *event_data, void *user_data) {

  /* Add a scoreboard-scrubbing timer.
   *
   * Note that we do this only for standalone proftpd daemons, not for
   * inetd-run daemons.  There is no "master"/"daemon" process for
   * inetd-run proftpd processes, which means that _all_ processes scrub
   * the scoreboard (which greatly increases lock contention, particularly
   * under high numbers of simultaneous connections), or that _no_
   * processes scrub the scoreboard (which increases the chance of stale/bad
   * scoreboard data).
   */
  if (ServerType == SERVER_STANDALONE) {
    int scrub_scoreboard = TRUE;
    int scrub_interval = PR_TUNABLE_SCOREBOARD_SCRUB_TIMER;
    config_rec *c;

    c = find_config(main_server->conf, CONF_PARAM, "ScoreboardScrub", FALSE);
    if (c) {
      scrub_scoreboard = *((int *) c->argv[0]);

      if (c->argc == 2) {
        scrub_interval = *((int *) c->argv[1]);
      }
    }

    if (scrub_scoreboard) {
      core_scrub_timer_id = pr_timer_add(scrub_interval, -1,
        &core_module, core_scrub_scoreboard_cb, "scoreboard scrubbing");
    }
  }

  /* Add a restart handler to scrub the scoreboard, too. */
  pr_event_register(&core_module, "core.restart", core_restart_ev, NULL);
}

/* Initialization/finalization routines
 */

static int core_init(void) {
  /* Set the default (i.e. FTP) command handler. */
  pr_cmd_set_handler(NULL);

  /* Add the commands handled by this module to the HELP list. */
  pr_help_add(C_CWD,  "<sp> pathname", TRUE);
  pr_help_add(C_XCWD, "<sp> pathname", TRUE);
  pr_help_add(C_CDUP, "(up one directory)", TRUE);
  pr_help_add(C_XCUP, "(up one directory)", TRUE);
  pr_help_add(C_SMNT, "is not implemented", FALSE);
  pr_help_add(C_QUIT, "(close control connection)", TRUE);
  pr_help_add(C_PORT, "<sp> h1,h2,h3,h4,p1,p2", TRUE);
  pr_help_add(C_PASV, "(returns address/port)", TRUE);
  pr_help_add(C_EPRT, "<sp> |proto|addr|port|", TRUE);
  pr_help_add(C_EPSV, "(returns port |||port|)", TRUE);
  pr_help_add(C_ALLO, "is not implemented (ignored)", FALSE);
  pr_help_add(C_RNFR, "<sp> pathname", TRUE);
  pr_help_add(C_RNTO, "<sp> pathname", TRUE);
  pr_help_add(C_DELE, "<sp> pathname", TRUE);
  pr_help_add(C_MDTM, "<sp> pathname", TRUE);
  pr_help_add(C_RMD,  "<sp> pathname", TRUE);
  pr_help_add(C_XRMD, "<sp> pathname", TRUE);
  pr_help_add(C_MKD,  "<sp> pathname", TRUE);
  pr_help_add(C_XMKD, "<sp> pathname", TRUE);
  pr_help_add(C_PWD,  "(returns current working directory)", TRUE);
  pr_help_add(C_XPWD, "(returns current working directory)", TRUE);
  pr_help_add(C_SIZE, "<sp> pathname", TRUE);
  pr_help_add(C_SYST, "(returns system type)", TRUE);
  pr_help_add(C_HELP, "[<sp> command]", TRUE);
  pr_help_add(C_NOOP, "(no operation)", TRUE);
  pr_help_add(C_FEAT, "(returns feature list)", TRUE);
  pr_help_add(C_OPTS, "<sp> command [<sp> options]", TRUE);
  pr_help_add(C_AUTH, "<sp> base64-data", FALSE);
  pr_help_add(C_CCC,  "(clears protection level)", FALSE);
  pr_help_add(C_CONF, "<sp> base64-data", FALSE);
  pr_help_add(C_ENC,  "<sp> base64-data", FALSE);
  pr_help_add(C_MIC,  "<sp> base64-data", FALSE);
  pr_help_add(C_PBSZ, "<sp> protection buffer size", FALSE);
  pr_help_add(C_PROT, "<sp> protection code", FALSE);


  /* Add the additional features implemented by this module into the
   * list, to be displayed in response to a FEAT command.
   */
  pr_feat_add("MDTM");
  pr_feat_add("REST STREAM");
  pr_feat_add("SIZE");

  pr_event_register(&core_module, "core.startup", core_startup_ev, NULL);

  return 0;
}

static int core_sess_init(void) {
  int timeout_idle;
  char *displayquit = NULL;
  config_rec *c = NULL;
  unsigned int *debug_level = NULL;

  init_auth();

  /* Start the idle timer. */

  c = find_config(main_server->conf, CONF_PARAM, "TimeoutIdle", FALSE);
  if (c != NULL) {
    int timeout = *((int *) c->argv[0]);
    pr_data_set_timeout(PR_DATA_TIMEOUT_IDLE, timeout);
  }

  timeout_idle = pr_data_get_timeout(PR_DATA_TIMEOUT_IDLE);
  if (timeout_idle) {
    pr_timer_add(timeout_idle, PR_TIMER_IDLE, NULL, core_idle_timeout_cb,
      "TimeoutIdle");
  }

  /* Check for a server-specific TimeoutLinger */
  c = find_config(main_server->conf, CONF_PARAM, "TimeoutLinger", FALSE);
  if (c != NULL)
    pr_data_set_linger(*((long *) c->argv[0]));
  
  /* Check for a configured DebugLevel. */
  debug_level = get_param_ptr(main_server->conf, "DebugLevel", FALSE);
  if (debug_level != NULL)
    pr_log_setdebuglevel(*debug_level);

  /* Check for configured SetEnvs. */
  c = find_config(main_server->conf, CONF_PARAM, "SetEnv", FALSE);

  while (c) {
    if (pr_env_set(session.pool, c->argv[0], c->argv[1]) < 0) {
      pr_log_debug(DEBUG1, "unable to set environ variable '%s': %s",
        (char *) c->argv[0], strerror(errno));
    }

    c = find_config_next(c, c->next, CONF_PARAM, "SetEnv", FALSE);
  }

  /* Check for configured UnsetEnvs. */
  c = find_config(main_server->conf, CONF_PARAM, "UnsetEnv", FALSE);

  while (c) {
    if (pr_env_unset(session.pool, c->argv[0]) < 0) {
      pr_log_debug(DEBUG1, "unable to unset environ variable '%s': %s",
        (char *) c->argv[0], strerror(errno));
    }

    c = find_config_next(c, c->next, CONF_PARAM, "UnsetEnv", FALSE);
  }

  /* Check for a server-specific AuthOrder. */
  c = find_config(main_server->conf, CONF_PARAM, "AuthOrder", FALSE);
  if (c != NULL) {
    array_header *module_list = (array_header *) c->argv[0];
    int modulec = 0;
    char **modulev = NULL;
    register unsigned int i = 0;
    const char *auth_syms[] = {
      "setpwent", "endpwent", "setgrent", "endgrent", "getpwent", "getgrent",
      "getpwnam", "getgrnam", "getpwuid", "getgrgid", "auth", "check",
      "uid2name", "gid2name", "name2uid", "name2gid", "getgroups", NULL
    };

    pr_log_debug(DEBUG3, "AuthOrder in effect, resetting auth module order");

    modulec = module_list->nelts;
    modulev = (char **) module_list->elts;

    /* First, delete all auth symbols. */
    for (i = 0; auth_syms[i] != NULL; i++)
      pr_stash_remove_symbol(PR_SYM_AUTH, auth_syms[i], NULL);

    /* Now, cycle through the list of configured modules, re-adding their
     * auth symbols, in the order in which they appear.
     */

    for (i = 0; i < modulec; i++) {
      module *m;
      int required = FALSE;

      /* Check for the trailing '*', indicating a required auth module. */
      if (modulev[i][strlen(modulev[i])-1] == '*') {
        required = TRUE;
        modulev[i][strlen(modulev[i])-1] = '\0';
      }

      m = pr_module_get(modulev[i]);

      if (m) {

        if (m->authtable) {
          authtable *authtab;

          /* Twiddle the module's priority field before insertion into the
           * symbol table, as the insertion operation does so based on that
           * priority.  This has no effect other than during symbol
           * insertion.
           */
          m->priority = modulec - i;

          for (authtab = m->authtable; authtab->name; authtab++) {
            authtab->m = m;

            if (required)
              authtab->auth_flags |= PR_AUTH_FL_REQUIRED;

            pr_stash_add_symbol(PR_SYM_AUTH, authtab);
          }

        } else
          pr_log_debug(DEBUG0, "AuthOrder: warning: module '%s' is not a valid "
            "auth module (no auth handlers), authentication may fail",
            modulev[i]);

      } else
        pr_log_debug(DEBUG0, "AuthOrder: warning: module '%s' not loaded",
          modulev[i]);
    }

    /* NOTE: the master conf/cmd/auth tables/arrays should ideally be
     * rebuilt after this symbol shuffling, but it's not necessary at this
     * point.
     */
  }

  if (ServerType == SERVER_STANDALONE) {
    pr_timer_remove(core_scrub_timer_id, &core_module);

  } else if (ServerType == SERVER_INETD) {

    /* If we're running as 'ServerType inetd', scrub the scoreboard here.
     * For standalone ServerTypes, the scoreboard scrubber will handle
     * things itself.
     */

    c = find_config(main_server->conf, CONF_PARAM, "ScoreboardScrub", FALSE);
    if (c) {
      if (*((int *) c->argv[0]) == TRUE) {
        pr_scoreboard_scrub();
      }
    }
  }

  /* Set some Variable entries for Display files. */

  if (pr_var_set(session.pool, "%{bytes_xfer}", 
      "Number of bytes transfered in this transfer", PR_VAR_TYPE_FUNC,
      (void *) core_get_xfer_bytes_str, &session.xfer.total_bytes,
      sizeof(off_t *)) < 0) {
    pr_log_debug(DEBUG6, "error setting %%{bytes_fer} variable: %s",
      strerror(errno));
  }

  if (pr_var_set(session.pool, "%{total_bytes_in}",
      "Number of bytes uploaded during a session", PR_VAR_TYPE_FUNC,
      (void *) core_get_sess_bytes_str, &session.total_bytes_in,
      sizeof(off_t *)) < 0) {
    pr_log_debug(DEBUG6, "error setting %%{total_bytes_in} variable: %s",
      strerror(errno));
  }

  if (pr_var_set(session.pool, "%{total_bytes_out}", 
      "Number of bytes downloaded during a session", PR_VAR_TYPE_FUNC,
      (void *) core_get_sess_bytes_str, &session.total_bytes_out,
      sizeof(off_t *)) < 0) {
    pr_log_debug(DEBUG6, "error setting %%{total_bytes_out} variable: %s",
      strerror(errno));
  }

  if (pr_var_set(session.pool, "%{total_bytes_xfer}", 
      "Number of bytes transfered during a session", PR_VAR_TYPE_FUNC,
      (void *) core_get_sess_bytes_str, &session.total_bytes,
      sizeof(off_t *)) < 0) {
    pr_log_debug(DEBUG6, "error setting %%{total_bytes_fer} variable: %s",
      strerror(errno));
  }

  if (pr_var_set(session.pool, "%{total_files_in}", 
      "Number of files uploaded during a session", PR_VAR_TYPE_FUNC,
      (void *) core_get_sess_files_str, &session.total_files_in,
      sizeof(unsigned int *)) < 0) {
    pr_log_debug(DEBUG6, "error setting %%{total_files_in} variable: %s",
      strerror(errno));
  }

  if (pr_var_set(session.pool, "%{total_files_out}", 
      "Number of files downloaded during a session", PR_VAR_TYPE_FUNC,
      (void *) core_get_sess_files_str, &session.total_files_out,
      sizeof(unsigned int *)) < 0) {
    pr_log_debug(DEBUG6, "error setting %%{total_files_out} variable: %s",
      strerror(errno));
  }

  if (pr_var_set(session.pool, "%{total_files_xfer}", 
      "Number of files transfered during a session", PR_VAR_TYPE_FUNC,
      (void *) core_get_sess_files_str, &session.total_files_xfer,
      sizeof(unsigned int *)) < 0) {
    pr_log_debug(DEBUG6, "error setting %%{total_files_xfer} variable: %s",
      strerror(errno));
  }

  /* Look for a DisplayQuit file which has an absolute path.  If we
   * find one, open a filehandle, such that that file can be displayed
   * even if the session is chrooted.  DisplayQuit files with
   * relative paths will be handled after chroot, preserving the old
   * behavior.
   */
  displayquit = get_param_ptr(TOPLEVEL_CONF, "DisplayQuit", FALSE);
  if (displayquit &&
      *displayquit == '/') {

    displayquit_fh = pr_fsio_open(displayquit, O_RDONLY);
    if (displayquit_fh == NULL)
      pr_log_debug(DEBUG6, "unable to open DisplayQuit file '%s': %s",
        displayquit, strerror(errno));
  }

  return 0;
}

/* Module API tables
 */

static conftable core_conftab[] = {
  { "<Anonymous>",		add_anonymous,			NULL },
  { "</Anonymous>",		end_anonymous,			NULL },
  { "<Class>",			add_class,			NULL },
  { "</Class>",			end_class,			NULL },
  { "<Directory>",		add_directory,			NULL },
  { "</Directory>",		end_directory,			NULL },
  { "<Global>",			add_global,			NULL },
  { "</Global>",		end_global,			NULL },
  { "<IfDefine>",		start_ifdefine,			NULL },
  { "</IfDefine>",		end_ifdefine,			NULL },
  { "<IfModule>",		start_ifmodule,			NULL },
  { "</IfModule>",		end_ifmodule,			NULL },
  { "<Limit>",			add_limit,			NULL },
  { "</Limit>", 		end_limit, 			NULL },
  { "<VirtualHost>",		add_virtualhost,		NULL },
  { "</VirtualHost>",		end_virtualhost,		NULL },
  { "Allow",			set_allowdeny,			NULL },
  { "AllowAll",			set_allowall,			NULL },
  { "AllowClass",		set_allowdenyusergroupclass,	NULL },
  { "AllowFilter",		set_allowdenyfilter,		NULL },
  { "AllowForeignAddress",	set_allowforeignaddress,	NULL },
  { "AllowGroup",		set_allowdenyusergroupclass,	NULL },
  { "AllowOverride",		set_allowoverride,		NULL },
  { "AllowUser",		set_allowdenyusergroupclass,	NULL },
  { "AuthOrder",		set_authorder,			NULL },
  { "CDPath",			set_cdpath,			NULL },
  { "CommandBufferSize",	set_commandbuffersize,		NULL },
  { "DebugLevel",		set_debuglevel,			NULL },
  { "DefaultAddress",		set_defaultaddress,		NULL },
  { "DefaultServer",		set_defaultserver,		NULL },
  { "DefaultTransferMode",	set_defaulttransfermode,	NULL },
  { "DeferWelcome",		set_deferwelcome,		NULL },
  { "Define",			set_define,			NULL },
  { "Deny",			set_allowdeny,			NULL },
  { "DenyAll",			set_denyall,			NULL },
  { "DenyClass",		set_allowdenyusergroupclass,	NULL },
  { "DenyFilter",		set_allowdenyfilter,		NULL },
  { "DenyGroup",		set_allowdenyusergroupclass,	NULL },
  { "DenyUser",			set_allowdenyusergroupclass,	NULL },
  { "DisplayChdir",		set_displaychdir,		NULL },
  { "DisplayConnect",		set_displayconnect,		NULL },
  { "DisplayGoAway",		set_displaygoaway,		NULL },
  { "DisplayQuit",		set_displayquit,		NULL },
  { "From",			add_from,			NULL },
  { "Group",			set_group, 			NULL },
  { "GroupOwner",		add_groupowner,			NULL },
  { "HideFiles",		set_hidefiles,			NULL },
  { "HideGroup",		set_hidegroup,			NULL },
  { "HideNoAccess",		set_hidenoaccess,		NULL },
  { "HideUser",			set_hideuser,			NULL },
  { "IgnoreHidden",		set_ignorehidden,		NULL },
  { "Include",			add_include,	 		NULL },
  { "MasqueradeAddress",	set_masqueradeaddress,		NULL },
  { "MaxConnectionRate",	set_maxconnrate,		NULL },
  { "MaxInstances",		set_maxinstances,		NULL },
  { "MultilineRFC2228",		set_multilinerfc2228,		NULL },
  { "Order",			set_order,			NULL },
  { "PassivePorts",		set_passiveports,		NULL },
  { "PathAllowFilter",		set_pathallowfilter,		NULL },
  { "PathDenyFilter",		set_pathdenyfilter,		NULL },
  { "PidFile",			set_pidfile,	 		NULL },
  { "Port",			set_serverport, 		NULL },
  { "RLimitCPU",		set_rlimitcpu,			NULL },
  { "RLimitMemory",		set_rlimitmemory,		NULL },
  { "RLimitOpenFiles",		set_rlimitopenfiles,		NULL },
  { "Satisfy",			set_satisfy,			NULL },
  { "ScoreboardFile",		set_scoreboardfile,		NULL },
  { "ScoreboardScrub",		set_scoreboardscrub,		NULL },
  { "ServerAdmin",		set_serveradmin,		NULL },
  { "ServerIdent",		set_serverident,		NULL },
  { "ServerName",		set_servername, 		NULL },
  { "ServerType",		set_servertype,			NULL },
  { "SetEnv",			set_setenv,			NULL },
  { "SocketBindTight",		set_socketbindtight,		NULL },
  { "SocketOptions",		set_socketoptions,		NULL },
  { "SyslogFacility",		set_syslogfacility,		NULL },
  { "SyslogLevel",		set_sysloglevel,		NULL },
  { "TimeoutIdle",		set_timeoutidle,		NULL },
  { "TimeoutLinger",		set_timeoutlinger,		NULL },
  { "TimesGMT",			set_timesgmt,			NULL },
  { "Trace",			set_trace,			NULL },
  { "TraceLog",			set_tracelog,			NULL },
  { "TransferLog",		add_transferlog,		NULL },
  { "Umask",			set_umask,			NULL },
  { "UnsetEnv",			set_unsetenv,			NULL },
  { "UseIPv6",			set_useipv6,			NULL },
  { "UseReverseDNS",		set_usereversedns,		NULL },
  { "User",			set_user,			NULL },
  { "UserOwner",		add_userowner,			NULL },
  { "WtmpLog",			set_wtmplog,			NULL },
  { "tcpBackLog",		set_tcpbacklog,			NULL },
  { "tcpNoDelay",		set_tcpnodelay,			NULL },

  { NULL, NULL, NULL }
};

static cmdtable core_cmdtab[] = {
#if defined(HAVE_REGEX_H) && defined(HAVE_REGCOMP)
  { PRE_CMD, C_ANY, G_NONE,  regex_filters, FALSE, FALSE, CL_NONE },
#endif
  { PRE_CMD, C_ANY, G_NONE, core_clear_fs,FALSE, FALSE, CL_NONE },
  { CMD, C_HELP, G_NONE,  core_help,	FALSE,	FALSE, CL_INFO },
  { CMD, C_PORT, G_NONE,  core_port,	TRUE,	FALSE, CL_MISC },
  { CMD, C_PASV, G_NONE,  core_pasv,	TRUE,	FALSE, CL_MISC },
  { CMD, C_EPRT, G_NONE,  core_eprt,    TRUE,	FALSE, CL_MISC },
  { CMD, C_EPSV, G_NONE,  core_epsv,	TRUE,	FALSE, CL_MISC },
  { CMD, C_SYST, G_NONE,  core_syst,	FALSE,	FALSE, CL_INFO },
  { CMD, C_PWD,	 G_DIRS,  core_pwd,	TRUE,	FALSE, CL_INFO|CL_DIRS },
  { CMD, C_XPWD, G_DIRS,  core_pwd,	TRUE,	FALSE, CL_INFO|CL_DIRS },
  { CMD, C_CWD,	 G_DIRS,  core_cwd,	TRUE,	FALSE, CL_DIRS },
  { CMD, C_XCWD, G_DIRS,  core_cwd,	TRUE,	FALSE, CL_DIRS },
  { CMD, C_MKD,	 G_WRITE, core_mkd,	TRUE,	FALSE, CL_DIRS|CL_WRITE },
  { CMD, C_XMKD, G_WRITE, core_mkd,	TRUE,	FALSE, CL_DIRS|CL_WRITE },
  { CMD, C_RMD,	 G_WRITE, core_rmd,	TRUE,	FALSE, CL_DIRS|CL_WRITE },
  { CMD, C_XRMD, G_WRITE, core_rmd,	TRUE,	FALSE, CL_DIRS|CL_WRITE },
  { CMD, C_CDUP, G_DIRS,  core_cdup,	TRUE,	FALSE, CL_DIRS },
  { CMD, C_XCUP, G_DIRS,  core_cdup,	TRUE,	FALSE, CL_DIRS },
  { CMD, C_DELE, G_WRITE, core_dele,	TRUE,	FALSE, CL_WRITE },
  { CMD, C_MDTM, G_DIRS,  core_mdtm,	TRUE,	FALSE, CL_INFO },
  { CMD, C_RNFR, G_DIRS,  core_rnfr,	TRUE,	FALSE, CL_MISC|CL_WRITE },
  { CMD, C_RNTO, G_WRITE, core_rnto,	TRUE,	FALSE, CL_MISC|CL_WRITE },
  { LOG_CMD,     C_RNTO, G_NONE, core_rnto_cleanup, TRUE, FALSE, CL_NONE },
  { LOG_CMD_ERR, C_RNTO, G_NONE, core_rnto_cleanup, TRUE, FALSE, CL_NONE },
  { CMD, C_SIZE, G_READ,  core_size,	TRUE,	FALSE, CL_INFO },
  { CMD, C_QUIT, G_NONE,  core_quit,	FALSE,	FALSE,  CL_INFO },
  { LOG_CMD, 	 C_QUIT, G_NONE, core_log_quit, FALSE, FALSE },
  { LOG_CMD_ERR, C_QUIT, G_NONE, core_log_quit, FALSE, FALSE },
  { CMD, C_NOOP, G_NONE,  core_noop,	FALSE,	FALSE,  CL_MISC },
  { CMD, C_FEAT, G_NONE,  core_feat,	FALSE,	FALSE,  CL_INFO },
  { CMD, C_OPTS, G_NONE,  core_opts,    FALSE,	FALSE,	CL_MISC },
  { POST_CMD, C_PASS, G_NONE, core_post_pass, FALSE, FALSE },
  { 0, NULL }
};

module core_module = {
  NULL, NULL,

  /* Module API version */
  0x20,

  /* Module name */
  "core",

  /* Module configuration directive table */
  core_conftab,

  /* Module command handler table */
  core_cmdtab,

  /* Module authentication handler table */
  NULL,

  /* Module initialization function */
  core_init,

  /* Session initialization function */
  core_sess_init
};
