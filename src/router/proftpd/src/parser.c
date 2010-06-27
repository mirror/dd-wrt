/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2004-2009 The ProFTPD Project team
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
 * As a special exemption, The ProFTPD Project team and other respective
 * copyright holders give permission to link this program with OpenSSL, and
 * distribute the resulting executable, without including the source code
 * for OpenSSL in the source distribution.
 */

/* Configuration parser
 * $Id: parser.c,v 1.19 2009/03/05 06:01:51 castaglia Exp $
 */

#include "conf.h"

extern xaset_t *server_list;
extern pool *global_config_pool;

static pool *parser_pool = NULL;

static array_header *parser_confstack = NULL;
static config_rec **parser_curr_config = NULL;

static array_header *parser_servstack = NULL;
static server_rec **parser_curr_server = NULL;
static unsigned int parser_sid = 0;

static xaset_t **parser_server_list = NULL;

static const char *trace_channel = "config";

struct config_src {
  struct config_src *cs_next;
  pool *cs_pool;
  pr_fh_t *cs_fh;
  unsigned int cs_lineno;
};

static unsigned int parser_curr_lineno = 0;

/* Note: the parser seems to be touchy about this particular value.  If
 * you see strange segfaults occurring in the mergedown() function, it
 * might be because this pool size is too small.
 */
#define PARSER_CONFIG_SRC_POOL_SZ	512

static struct config_src *parser_sources = NULL;

/* Private functions
 */

static void add_config_ctxt(config_rec *c) {
  if (!*parser_curr_config)
    *parser_curr_config = c;

  else {
    parser_curr_config = (config_rec **) push_array(parser_confstack);
    *parser_curr_config = c;
  }
}

static struct config_src *add_config_source(pr_fh_t *fh) {
  pool *p = pr_pool_create_sz(parser_pool, PARSER_CONFIG_SRC_POOL_SZ);
  struct config_src *cs = pcalloc(p, sizeof(struct config_src));

  pr_pool_tag(p, "configuration source pool");
  cs->cs_next = NULL;
  cs->cs_pool = p;
  cs->cs_fh = fh;
  cs->cs_lineno = 0;

  if (!parser_sources)
    parser_sources = cs;

  else {
    cs->cs_next = parser_sources;
    parser_sources = cs;
  }

  return cs;
}

static char *get_config_word(pool *p, char *word) {

  /* Should this word be replaced with a value from the environment?
   * If so, tmp will contain the expanded value, otherwise tmp will
   * contain a string duped from the given pool.
   */

  /* Does the given word use the environment syntax? */
  if (strlen(word) > 7 &&
      strncmp(word, "%{env:", 6) == 0 &&
      word[strlen(word)-1] == '}') {
    char *env;

    word[strlen(word)-1] = '\0';

    env = pr_env_get(p, word + 6);

    return env ? pstrdup(p, env) : "";
  }

  return pstrdup(p, word);
}

static void remove_config_source(void) {
  struct config_src *cs = parser_sources;

  if (cs) {
    parser_sources = cs->cs_next;
    destroy_pool(cs->cs_pool);
  }

  return;
}

/* Public API
 */

int pr_parser_cleanup(void) {
  if (parser_pool) {
    if (parser_servstack->nelts > 1 ||
        (parser_curr_config && *parser_curr_config)) {
      errno = EPERM;
      return -1;
    }

    destroy_pool(parser_pool);
    parser_pool = NULL;
  }

  parser_servstack = NULL;
  parser_curr_server = NULL;

  parser_confstack = NULL;
  parser_curr_config = NULL;

  /* Reset the SID counter. */
  parser_sid = 0;

  return 0;
}

config_rec *pr_parser_config_ctxt_close(int *empty) {
  config_rec *c = *parser_curr_config;

  /* Note that if the current config is empty, it should simply be removed.
   * Such empty configs can happen for <Directory> sections that
   * contain no directives, for example.
   */

  if (parser_curr_config == (config_rec **) parser_confstack->elts) {
    if (!c->subset || !c->subset->xas_list) {
      xaset_remove(c->set, (xasetmember_t *) c);
      destroy_pool(c->pool);

      if (empty)
        *empty = TRUE;
    }

    if (*parser_curr_config)
      *parser_curr_config = NULL;

    return NULL;
  }

  if (!c->subset || !c->subset->xas_list) {
    xaset_remove(c->set, (xasetmember_t *) c);
    destroy_pool(c->pool);

    if (empty)
      *empty = TRUE;
  }

  parser_curr_config--;
  parser_confstack->nelts--;

  return *parser_curr_config;
}

config_rec *pr_parser_config_ctxt_get(void) {
  if (parser_curr_config)
    return *parser_curr_config;

  errno = ENOENT;
  return NULL;
}

config_rec *pr_parser_config_ctxt_open(const char *name) {
  config_rec *c = NULL, *parent = *parser_curr_config;
  pool *c_pool = NULL, *parent_pool = NULL;
  xaset_t **set = NULL;

  if (!name) {
    errno = EINVAL;
    return NULL;
  }

  if (parent) {
    parent_pool = parent->pool;
    set = &parent->subset;

  } else {
    parent_pool = (*parser_curr_server)->pool;
    set = &(*parser_curr_server)->conf;
  }

  /* Allocate a sub-pool for this config_rec.
   *
   * Note: special exception for <Global> configs: the parent pool is
   * 'global_config_pool' (a pool just for that context), not the pool of the
   * parent server.  This keeps <Global> config recs from being freed
   * prematurely, and helps to avoid memory leaks.
   */
  if (strcmp(name, "<Global>") == 0) {
    if (!global_config_pool) {
      global_config_pool = make_sub_pool(permanent_pool);
      pr_pool_tag(global_config_pool, "<Global> Pool");
    }

    parent_pool = global_config_pool;
  }

  c_pool = make_sub_pool(parent_pool);
  pr_pool_tag(c_pool, "sub-config pool");

  c = (config_rec *) pcalloc(c_pool, sizeof(config_rec));

  if (!*set) {
    pool *set_pool = make_sub_pool(parent_pool);
    *set = xaset_create(set_pool, NULL);
    (*set)->pool = set_pool;
  }

  xaset_insert(*set, (xasetmember_t *) c);

  c->pool = c_pool;
  c->set = *set;
  c->parent = parent;
  c->name = pstrdup(c->pool, name);

  if (parent) {
    if (parent->config_type == CONF_DYNDIR)
      c->flags |= CF_DYNAMIC;
  }

  add_config_ctxt(c);
  return c;
}

unsigned int pr_parser_get_lineno(void) {
  return parser_curr_lineno;
}

int pr_parser_parse_file(pool *p, const char *path, config_rec *start,
    int flags) {
  pr_fh_t *fh;
  struct stat st;
  struct config_src *cs;
  cmd_rec *cmd;
  pool *tmp_pool;
  char *report_path;

  if (!path) {
    errno = EINVAL;
    return -1;
  }

  tmp_pool = make_sub_pool(p ? p : permanent_pool);
  pr_pool_tag(tmp_pool, "parser file pool");

  report_path = (char *) path;
  if (session.chroot_path)
    report_path = pdircat(tmp_pool, session.chroot_path, path, NULL);

  if (!(flags & PR_PARSER_FL_DYNAMIC_CONFIG))
    pr_trace_msg(trace_channel, 3, "parsing '%s' configuration", report_path);

  fh = pr_fsio_open(path, O_RDONLY);
  if (fh == NULL) {
    destroy_pool(tmp_pool);
    return -1;
  }

  /* Stat the opened file to determine the optimal buffer size for IO. */
  memset(&st, 0, sizeof(st));
  pr_fsio_fstat(fh, &st);
  fh->fh_iosz = st.st_blksize;

  /* Push the configuration information onto the stack of configuration
   * sources.
   */
  cs = add_config_source(fh);

  if (start) 
    add_config_ctxt(start);

  while ((cmd = pr_parser_parse_line(tmp_pool)) != NULL) {
    pr_signals_handle();

    if (cmd->argc) {
      conftable *conftab;
      char found = FALSE;

      cmd->server = *parser_curr_server;
      cmd->config = *parser_curr_config;

      conftab = pr_stash_get_symbol(PR_SYM_CONF, cmd->argv[0], NULL,
        &cmd->stash_index);

      while (conftab) {
        modret_t *mr;

        pr_signals_handle();

        cmd->argv[0] = conftab->directive;

        pr_trace_msg(trace_channel, 7,
          "dispatching directive '%s' to module mod_%s", conftab->directive,
          conftab->m->name);

        mr = pr_module_call(conftab->m, conftab->handler, cmd);
        if (mr != NULL) {
          if (MODRET_ISERROR(mr)) {

            if (!(flags & PR_PARSER_FL_DYNAMIC_CONFIG)) {
              pr_log_pri(PR_LOG_ERR, "Fatal: %s on line %u of '%s'",
                MODRET_ERRMSG(mr), cs->cs_lineno, report_path);
              exit(1);

            } else
              pr_log_pri(PR_LOG_WARNING, "warning: %s on line %u of '%s'",
                MODRET_ERRMSG(mr), cs->cs_lineno, report_path);
          }
        }

        if (!MODRET_ISDECLINED(mr))
          found = TRUE;

        conftab = pr_stash_get_symbol(PR_SYM_CONF, cmd->argv[0], conftab,
          &cmd->stash_index);
      }

      if (cmd->tmp_pool)
        destroy_pool(cmd->tmp_pool);

      if (!found) {

        if (!(flags & PR_PARSER_FL_DYNAMIC_CONFIG)) {
          pr_log_pri(PR_LOG_ERR, "Fatal: unknown configuration directive "
            "'%s' on line %u of '%s'", cmd->argv[0], cs->cs_lineno,
            report_path);
          exit(1);

        } else 
          pr_log_pri(PR_LOG_WARNING, "warning: unknown configuration directive "
            "'%s' on line %u of '%s'", cmd->argv[0], cs->cs_lineno,
            report_path);
      }
    }

    destroy_pool(cmd->pool);
  }

  /* Pop this configuration stream from the stack. */
  remove_config_source();

  pr_fsio_close(fh);

  destroy_pool(tmp_pool);
  return 0;
}

cmd_rec *pr_parser_parse_line(pool *p) {
  char buf[PR_TUNABLE_BUFFER_SIZE], *word = NULL;
  cmd_rec *cmd = NULL;
  pool *sub_pool = NULL;
  array_header *arr = NULL;

  if (!p) {
    errno = EINVAL;
    return NULL;
  }

  memset(buf, '\0', sizeof(buf));

  while (pr_parser_read_line(buf, sizeof(buf)-1) != NULL) {
    char *bufp = buf;

    pr_signals_handle();

    /* Build a new pool for the command structure and array */
    sub_pool = make_sub_pool(p);
    pr_pool_tag(sub_pool, "parser cmd subpool");

    cmd = pcalloc(sub_pool, sizeof(cmd_rec));
    cmd->pool = sub_pool;
    cmd->stash_index = -1;

    /* Add each word to the array */
    arr = make_array(cmd->pool, 4, sizeof(char **));
    while ((word = pr_str_get_word(&bufp, 0)) != NULL) {
      char *tmp = get_config_word(cmd->pool, word);

      *((char **) push_array(arr)) = tmp;
      cmd->argc++;
    }

    /* Terminate the array with a NULL. */
    *((char **) push_array(arr)) = NULL;

    /* The array header's job is done, we can forget about it and
     * it will get purged when the command's pool is destroyed.
     */

    cmd->argv = (char **) arr->elts;

    /* Perform a fixup on configuration directives so that:
     *
     *   -argv[0]--  -argv[1]-- ----argv[2]-----
     *   <Option     /etc/adir  /etc/anotherdir>
     *
     *  becomes:
     *
     *   -argv[0]--  -argv[1]-  ----argv[2]----
     *   <Option>    /etc/adir  /etc/anotherdir
     */

    if (cmd->argc &&
        *(cmd->argv[0]) == '<') {
      char *cp = cmd->argv[cmd->argc-1];

      if (*(cp + strlen(cp)-1) == '>' &&
          cmd->argc > 1) {

        if (strcmp(cp, ">") == 0) {
          cmd->argv[cmd->argc-1] = NULL;
          cmd->argc--;

        } else
          *(cp + strlen(cp)-1) = '\0';

        cp = cmd->argv[0];
        if (*(cp + strlen(cp)-1) != '>')
          cmd->argv[0] = pstrcat(cmd->pool, cp, ">", NULL);
      }
    }

    return cmd;
  }

  return NULL;
}

int pr_parser_prepare(pool *p, xaset_t **parsed_servers) {

  if (!p) {
    if (!parser_pool) {
      parser_pool = make_sub_pool(permanent_pool);
      pr_pool_tag(parser_pool, "Parser Pool");
    }

    p = parser_pool;
  }

  if (!parsed_servers)
    parser_server_list = &server_list;

  else
    parser_server_list = parsed_servers;

  parser_servstack = make_array(p, 1, sizeof(server_rec *));
  parser_curr_server = (server_rec **) push_array(parser_servstack);
  *parser_curr_server = main_server;

  parser_confstack = make_array(p, 10, sizeof(config_rec *));
  parser_curr_config = (config_rec **) push_array(parser_confstack);
  *parser_curr_config = NULL;

  return 0;
}

/* This functions returns the next line from the configuration stream,
 * skipping commented-out lines and trimming trailing and leading whitespace,
 * returning, in effect, the next line of configuration data on which to
 * act.  This function has the advantage that it can be called by functions
 * that don't have access to configuration file handle, such as the
 * <IfDefine> and <IfModule> configuration handlers.
 */
char *pr_parser_read_line(char *buf, size_t bufsz) {
  struct config_src *cs;

  /* Always use the config stream at the top of the stack. */
  cs = parser_sources;

  if (!buf) {
    errno = EINVAL;
    return NULL;
  }

  if (!cs->cs_fh) {
    errno = EPERM;
    return NULL;
  }

  parser_curr_lineno = cs->cs_lineno;

  /* Check for error conditions. */

  while ((pr_fsio_getline(buf, bufsz, cs->cs_fh, &(cs->cs_lineno))) != NULL) {
    int have_eol = FALSE;
    char *bufp = NULL;
    size_t buflen = strlen(buf);

    parser_curr_lineno = cs->cs_lineno;

    /* Trim off the trailing newline, if present. */
    if (buflen &&
        buf[buflen - 1] == '\n') {
      have_eol = TRUE;
      buf[buflen - 1] = '\0';
      buflen = strlen(buf);
    }

    while (buflen &&
           buf[buflen - 1] == '\r') {
      pr_signals_handle();
      buf[buflen - 1] = '\0';
      buflen = strlen(buf);
    }

    if (!have_eol) {
      pr_log_pri(PR_LOG_WARNING,
        "warning: handling possibly truncated configuration data at "
        "line %u of '%s'", cs->cs_lineno, cs->cs_fh->fh_path);
    }

    /* Advance past any leading whitespace. */
    for (bufp = buf; *bufp && isspace((int) *bufp); bufp++);

    /* Check for commented or blank lines at this point, and just continue on
     * to the next configuration line if found.  If not, return the
     * configuration line.
     */
    if (*bufp == '#' || !*bufp) {
      continue;

    } else {

      /* Copy the value of bufp back into the pointer passed in
       * and return it.
       */
      buf = bufp;

      return buf;
    }
  }

  return NULL;
}

server_rec *pr_parser_server_ctxt_close(void) {
  if (!parser_curr_server) {
    errno = ENOENT;
    return NULL;
  }

  /* Disallow underflows. */
  if (parser_curr_server == (server_rec **) parser_servstack->elts) {
    errno = EPERM;
    return NULL;
  }

  parser_curr_server--;
  parser_servstack->nelts--;

  return *parser_curr_server;
}

server_rec *pr_parser_server_ctxt_get(void) {
  if (parser_curr_server)
    return *parser_curr_server;

  errno = ENOENT;
  return NULL;
}

server_rec *pr_parser_server_ctxt_open(const char *addrstr) {
  server_rec *s;
  pool *p;

  p = make_sub_pool(permanent_pool);
  pr_pool_tag(p, "<VirtualHost> Pool");

  s = (server_rec *) pcalloc(p, sizeof(server_rec));
  s->pool = p;
  s->config_type = CONF_VIRTUAL;
  s->sid = ++parser_sid;

  /* Have to make sure it ends up on the end of the chain, otherwise
   * main_server becomes useless.
   */
  xaset_insert_end(*parser_server_list, (xasetmember_t *) s);
  s->set = *parser_server_list;
  if (addrstr) {
    s->ServerAddress = pstrdup(s->pool, addrstr);
  }

  /* Default server port */
  s->ServerPort = pr_inet_getservport(s->pool, "ftp", "tcp");

  parser_curr_server = (server_rec **) push_array(parser_servstack);
  *parser_curr_server = s;

  return s;
}
