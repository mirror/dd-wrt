/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 1997, 1998 Public Flood Software
 * Copyright (c) 2001-2008 The ProFTPD Project team
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

/*
 * Module handling routines
 * $Id: modules.c,v 1.58 2010/02/14 00:19:06 castaglia Exp $
 */

#include "conf.h"

/* This local structure vastly speeds up symbol lookups. */
struct stash {
  struct stash *next,*prev;
  pool *sym_pool;
  const char *sym_name;
  pr_stash_type_t sym_type;
  module *sym_module;

  union {
    conftable *sym_conf;
    cmdtable *sym_cmd;
    authtable *sym_auth;
    cmdtable *sym_hook;
    void *sym_generic;
  } ptr;
};

extern module *static_modules[];
extern module *loaded_modules;

/* Symbol hashes for each type */
static xaset_t *symbol_table[PR_TUNABLE_HASH_TABLE_SIZE];
static pool *symbol_pool = NULL;
static struct stash *curr_sym = NULL;

/* Currently running module */
module *curr_module = NULL;

/* Used to track the priority for loaded modules. */
static unsigned int curr_module_pri = 0;

typedef struct mod_cb {
  struct mod_cb *next, *prev;

  int (*module_cb)(void);
} module_cb_t;

/* Symbol stash lookup code and management */

static struct stash *sym_alloc(void) {
  pool *sub_pool;
  struct stash *sym;

  /* XXX Use a smaller pool size, since there are lots of sub-pools allocated
   * for Stash symbols.  The default pool size (PR_TUNABLE_POOL_SIZE, 512
   * by default) is a bit large for symbols.
   */
  sub_pool = pr_pool_create_sz(symbol_pool, 128);

  sym = pcalloc(sub_pool, sizeof(struct stash));
  sym->sym_pool = sub_pool; 
  pr_pool_tag(sub_pool, "symbol");

  return sym;
}

static int sym_cmp(struct stash *s1, struct stash *s2) {
  int res;

  res = strcmp(s1->sym_name, s2->sym_name);

  /* Higher priority modules must go BEFORE lower priority in the
   * hash tables.
   */

  if (res == 0) {
    if (s1->sym_module != NULL &&
        s2->sym_module != NULL) {

      if (s1->sym_module->priority > s2->sym_module->priority) {
        return -1;
      }
    
      if (s1->sym_module->priority < s2->sym_module->priority) {
        return 1;
      }

      return res;
    }

    if (s1->sym_module != NULL &&
        s2->sym_module == NULL) {
      return -1;
    }

    if (s1->sym_module == NULL &&
        s2->sym_module != NULL) {
      return 1;
    }

    /* Both sym_module fields are null. */
    return 0;
  }

  return res;
}

static int symtab_hash(const char *name) {
  unsigned char *cp = NULL;
  int total = 0;

  if (!name)
    return 0;

  for (cp = (unsigned char *)name; *cp; cp++)
    total += (int)*cp;

  return (total < PR_TUNABLE_HASH_TABLE_SIZE ? total :
    (total % PR_TUNABLE_HASH_TABLE_SIZE));
}

int pr_stash_add_symbol(pr_stash_type_t sym_type, void *data) {
  struct stash *sym = NULL;
  int idx = 0;

  if (!data) {
    errno = EINVAL;
    return -1;
  }

  switch (sym_type) {
    case PR_SYM_CONF:
      sym = sym_alloc();
      sym->sym_type = PR_SYM_CONF;
      sym->sym_name = ((conftable *) data)->directive;
      sym->sym_module = ((conftable *) data)->m;
      sym->ptr.sym_conf = data;
      break;

    case PR_SYM_CMD:
      sym = sym_alloc();
      sym->sym_type = PR_SYM_CMD;
      sym->sym_name = ((cmdtable *) data)->command;
      sym->sym_module = ((cmdtable *) data)->m;
      sym->ptr.sym_cmd = data;
      break;

    case PR_SYM_AUTH:
      sym = sym_alloc();
      sym->sym_type = PR_SYM_AUTH;
      sym->sym_name = ((authtable *) data)->name;
      sym->sym_module = ((authtable *) data)->m;
      sym->ptr.sym_auth = data;
      break;

    case PR_SYM_HOOK:
      sym = sym_alloc();
      sym->sym_type = PR_SYM_HOOK;
      sym->sym_name = ((cmdtable *) data)->command;
      sym->sym_module = ((cmdtable *) data)->m;
      sym->ptr.sym_hook = data;
      break;

    default:
      errno = ENOENT;
      return -1;
  }

  /* XXX Should we check for null sym->sym_module as well? */
  if (sym->sym_name == NULL) {
    destroy_pool(sym->sym_pool);
    errno = EPERM;
    return -1;
  }

  /* XXX Ugly hack to support mixed cases of directives in config files. */
  if (sym_type != PR_SYM_CONF) {
    idx = symtab_hash(sym->sym_name);

  } else {
    register unsigned int i;
    char buf[1024];

    memset(buf, '\0', sizeof(buf));
    sstrncpy(buf, sym->sym_name, sizeof(buf)-1);

    for (i = 0; i < strlen(buf); i++)
      buf[i] = tolower((int) buf[i]);

    idx = symtab_hash(buf);
  }

  if (!symbol_table[idx]) {
    symbol_table[idx] = xaset_create(symbol_pool, (XASET_COMPARE) sym_cmp);
  }

  xaset_insert_sort(symbol_table[idx], (xasetmember_t *) sym, TRUE);
  return 0;
}

static struct stash *stash_lookup(pr_stash_type_t sym_type,
    const char *name, int idx) {
  struct stash *sym = NULL;

  if (symbol_table[idx]) {
    for (sym = (struct stash *) symbol_table[idx]->xas_list; sym;
        sym = sym->next)
      if (sym->sym_type == sym_type &&
          (!name || strcasecmp(sym->sym_name, name) == 0))
        break;
  }

  return sym;
}

static struct stash *stash_lookup_next(pr_stash_type_t sym_type,
    const char *name, int idx, void *prev) {
  struct stash *sym = NULL;
  int last_hit = 0;

  if (symbol_table[idx]) {
    for (sym = (struct stash *) symbol_table[idx]->xas_list; sym;
        sym = sym->next) {
      if (last_hit && sym->sym_type == sym_type &&
          (!name || strcasecmp(sym->sym_name, name) == 0))
        break;
      if (sym->ptr.sym_generic == prev)
        last_hit++;
    }
  }

  return sym;
}

void *pr_stash_get_symbol(pr_stash_type_t sym_type, const char *name,
    void *prev, int *idx_cache) {
  int idx;
  struct stash *sym = NULL;

  if (idx_cache &&
      *idx_cache != -1) {
    idx = *idx_cache;

  } else {

    /* XXX Ugly hack to support mixed cases of directives in config files. */
    if (sym_type != PR_SYM_CONF) {
      idx = symtab_hash(name);

    } else {
      register unsigned int i;
      char buf[1024];

      memset(buf, '\0', sizeof(buf));
      sstrncpy(buf, name, sizeof(buf)-1);

      for (i = 0; i < strlen(buf); i++)
        buf[i] = tolower((int) buf[i]);

      idx = symtab_hash(buf);
    }

    if (idx_cache)
      *idx_cache = idx;
  }

  if (idx >= PR_TUNABLE_HASH_TABLE_SIZE) {
    if (*idx_cache)
      *idx_cache = -1;

    errno = EINVAL;
    return NULL;
  }

  if (prev)
    curr_sym = sym = stash_lookup_next(sym_type, name, idx, prev);
  else
    curr_sym = sym = stash_lookup(sym_type, name, idx);

  switch (sym_type) {
    case PR_SYM_CONF:
      if (sym) {
        return sym->ptr.sym_conf;
      }

      errno = ENOENT;
      return NULL;

    case PR_SYM_CMD:
      if (sym) {
        return sym->ptr.sym_cmd;
      }

      errno = ENOENT;
      return NULL;

    case PR_SYM_AUTH:
      if (sym) {
        return sym->ptr.sym_auth;
      }

      errno = ENOENT;
      return NULL;

    case PR_SYM_HOOK:
      if (sym) {
        return sym->ptr.sym_hook;
      }

      errno = ENOENT;
      return NULL;
  }

  errno = EINVAL;
  return NULL;
}

int pr_stash_remove_symbol(pr_stash_type_t sym_type, const char *sym_name,
    module *sym_module) {
  int count = 0, symtab_idx = 0;

  if (!sym_name) {
    errno = EINVAL;
    return -1;
  }

  /* XXX Ugly hack to support mixed cases of directives in config files. */
  if (sym_type != PR_SYM_CONF) {
    symtab_idx = symtab_hash(sym_name);

  } else {
    register unsigned int i;
    char buf[1024];

    memset(buf, '\0', sizeof(buf));
    sstrncpy(buf, sym_name, sizeof(buf)-1);

    for (i = 0; i < strlen(buf); i++)
      buf[i] = tolower((int) buf[i]);

    symtab_idx = symtab_hash(buf);
  }

  switch (sym_type) {
    case PR_SYM_CONF: {
      int idx = -1;
      conftable *tab;

      tab = pr_stash_get_symbol(PR_SYM_CONF, sym_name, NULL, &idx);

      while (tab) {
        pr_signals_handle();

        /* Note: this works because of a hack: the symbol lookup functions
         * set a static pointer, curr_sym, to point to the struct stash
         * just looked up.  curr_sym will not be NULL if pr_stash_get_symbol()
         * returns non-NULL.
         */

        if (!sym_module ||
            curr_sym->sym_module == sym_module) {
          xaset_remove(symbol_table[symtab_idx], (xasetmember_t *) curr_sym);
          destroy_pool(curr_sym->sym_pool);
          curr_sym = NULL;
          tab = NULL;
          count++;
        }

        tab = pr_stash_get_symbol(PR_SYM_CONF, sym_name, tab, &idx);
      }

      break;
    }

    case PR_SYM_CMD: {
      int idx = -1;
      cmdtable *tab;

      tab = pr_stash_get_symbol(PR_SYM_CMD, sym_name, NULL, &idx);

      while (tab) {
        pr_signals_handle();

        /* Note: this works because of a hack: the symbol lookup functions
         * set a static pointer, curr_sym, to point to the struct stash
         * just looked up.  
         */

        if (!sym_module ||
            curr_sym->sym_module == sym_module) {
          xaset_remove(symbol_table[symtab_idx], (xasetmember_t *) curr_sym);
          destroy_pool(curr_sym->sym_pool);
          tab = NULL;
          count++;
        }

        tab = pr_stash_get_symbol(PR_SYM_CMD, sym_name, tab, &idx);
      }

      break;
    }

    case PR_SYM_AUTH: {
      int idx = -1;
      authtable *tab;

      tab = pr_stash_get_symbol(PR_SYM_AUTH, sym_name, NULL, &idx);

      while (tab) {
        pr_signals_handle();

        /* Note: this works because of a hack: the symbol lookup functions
         * set a static pointer, curr_sym, to point to the struct stash
         * just looked up.  
         */

        if (!sym_module ||
            curr_sym->sym_module == sym_module) {
          xaset_remove(symbol_table[symtab_idx], (xasetmember_t *) curr_sym);
          destroy_pool(curr_sym->sym_pool);
          tab = NULL;
          count++;
        }

        tab = pr_stash_get_symbol(PR_SYM_AUTH, sym_name, tab, &idx);
      }

      break;
    }

    case PR_SYM_HOOK: {
      int idx = -1;
      cmdtable *tab;

      tab = pr_stash_get_symbol(PR_SYM_HOOK, sym_name, NULL, &idx);

      while (tab) {
        pr_signals_handle();

        if (!sym_module ||
            curr_sym->sym_module == sym_module) {
          xaset_remove(symbol_table[symtab_idx], (xasetmember_t *) curr_sym);
          destroy_pool(curr_sym->sym_pool);
          tab = NULL;
          count++;
        }

        tab = pr_stash_get_symbol(PR_SYM_HOOK, sym_name, tab, &idx);
      }

      break;
    }

    default:
      errno = EINVAL;
      return -1;
  }

  return count;
}

modret_t *pr_module_call(module *m, modret_t *(*func)(cmd_rec *),
    cmd_rec *cmd) {
  modret_t *res;
  module *prev_module = curr_module;

  if (m == NULL ||
      func == NULL ||
      cmd == NULL) {
    errno = EINVAL;
    return NULL;
  }

  if (!cmd->tmp_pool) {
    cmd->tmp_pool = make_sub_pool(cmd->pool);
    pr_pool_tag(cmd->tmp_pool, "Module call tmp_pool");
  }

  curr_module = m;
  res = func(cmd);
  curr_module = prev_module;

  /* Note that we don't clear the pool here because the function may
   * return data which resides in this pool.
   */
  return res;
}

modret_t *mod_create_data(cmd_rec *cmd, void *d) {
  modret_t *res;

  res = pcalloc(cmd->tmp_pool, sizeof(modret_t));
  res->data = d;

  return res;
}

modret_t *mod_create_ret(cmd_rec *cmd, unsigned char err, char *n, char *m) {
  modret_t *res;

  res = pcalloc(cmd->tmp_pool, sizeof(modret_t));
  res->mr_handler_module = curr_module;
  res->mr_error = err;

  if (n) {
    res->mr_numeric = pstrdup(cmd->tmp_pool, n);
  }

  if (m) {
    res->mr_message = pstrdup(cmd->tmp_pool, m);
  }

  return res;
}

modret_t *mod_create_error(cmd_rec *cmd, int mr_errno) {
  modret_t *res;

  res = pcalloc(cmd->tmp_pool, sizeof(modret_t));
  res->mr_handler_module = curr_module;
  res->mr_error = mr_errno;

  return res;
}

/* Called after forking in order to inform/initialize modules
 * need to know we are a child and have a connection.
 */
int modules_session_init(void) {
  module *prev_module = curr_module, *m;

  for (m = loaded_modules; m; m = m->next) {
    if (m && m->sess_init) {
      curr_module = m;
      if (m->sess_init() < 0) {
        pr_log_pri(PR_LOG_ERR, "mod_%s.c: error initializing session: %s",
          m->name, strerror(errno));
        return -1;
      }
    }
  }

  curr_module = prev_module;
  return 0;
}

unsigned char command_exists(char *name) {
  int idx = -1;
  cmdtable *cmdtab = pr_stash_get_symbol(PR_SYM_CMD, name, NULL, &idx);

  while (cmdtab && cmdtab->cmd_type != CMD) {
    pr_signals_handle();
    cmdtab = pr_stash_get_symbol(PR_SYM_CMD, name, cmdtab, &idx);
  }

  return (cmdtab ? TRUE : FALSE);
}

unsigned char pr_module_exists(const char *name) {
  return pr_module_get(name) != NULL ? TRUE : FALSE;
}

module *pr_module_get(const char *name) {
  char buf[80] = {'\0'};
  module *m;

  if (!name) {
    errno = EINVAL;
    return NULL;
  }

  /* Check the list of compiled-in modules. */
  for (m = loaded_modules; m; m = m->next) {
    memset(buf, '\0', sizeof(buf));
    snprintf(buf, sizeof(buf), "mod_%s.c", m->name);
    buf[sizeof(buf)-1] = '\0';

    if (strcmp(buf, name) == 0)
      return m;
  }

  errno = ENOENT;
  return NULL;
}

void modules_list(int flags) {

  if (flags & PR_MODULES_LIST_FL_SHOW_STATIC) {
    register unsigned int i = 0;

    printf("Compiled-in modules:\n");
    for (i = 0; static_modules[i]; i++) {
      module *m = static_modules[i];

      if (flags & PR_MODULES_LIST_FL_SHOW_VERSION) {
        char *version = m->module_version;
        if (version) {
          printf("  %s\n", version);

        } else {
          printf("  mod_%s.c\n", m->name);
        }

      } else {
        printf("  mod_%s.c\n", m->name);
      }
    }

  } else {
    module *m;

    printf("Loaded modules:\n");
    for (m = loaded_modules; m; m = m->next) {

      if (flags & PR_MODULES_LIST_FL_SHOW_VERSION) {
        char *version = m->module_version;
        if (version) {
          printf("  %s\n", version);

        } else {  
          printf("  mod_%s.c\n", m->name);
        }

      } else {
        printf("  mod_%s.c\n", m->name);
      }
    }
  }
}

int pr_module_load(module *m) {
  char buf[256];

  if (m == NULL ||
      m->name == NULL) {
    errno = EINVAL;
    return -1;
  }

  /* Check the API version the module wants to use. */
  if (m->api_version < PR_MODULE_API_VERSION) {
    errno = EACCES;
    return -1;
  }

  /* Do not allow multiple modules with the same name. */
  memset(buf, '\0', sizeof(buf));
  snprintf(buf, sizeof(buf), "mod_%s.c", m->name);
  buf[sizeof(buf)-1] = '\0';

  if (pr_module_get(buf) != NULL) {
    errno = EEXIST;
    return -1;
  }

  /* Invoke the module's initialization routine. */
  if (!m->init ||
      m->init() >= 0) {

    /* Assign a priority to this module. */
    m->priority = curr_module_pri++;

    /* Add the module's config, cmd, and auth tables. */
    if (m->conftable) {
      conftable *conftab;

      for (conftab = m->conftable; conftab->directive; conftab++) {
        conftab->m = m;

        if (pr_stash_add_symbol(PR_SYM_CONF, conftab) < 0) {
          return -1;
        }
      }
    }

    if (m->cmdtable) {
      cmdtable *cmdtab;

      for (cmdtab = m->cmdtable; cmdtab->command; cmdtab++) {
        cmdtab->m = m;

        if (cmdtab->cmd_type == HOOK) {
          if (pr_stash_add_symbol(PR_SYM_HOOK, cmdtab) < 0) {
            return -1;
          }

        } else {
          /* All other cmd_types are for CMDs: PRE_CMD, CMD, POST_CMD, etc. */
          if (pr_stash_add_symbol(PR_SYM_CMD, cmdtab) < 0) {
            return -1;
          }
        }
      }
    }

    if (m->authtable) {
      authtable *authtab;

      for (authtab = m->authtable; authtab->name; authtab++) {
        authtab->m = m;

        if (pr_stash_add_symbol(PR_SYM_AUTH, authtab) < 0) {
          return -1;
        }
      }
    }

    /* Add the module to the loaded_modules list. */
    if (loaded_modules) {
      m->next = loaded_modules;
      loaded_modules->prev = m;
    }

    loaded_modules = m;

    /* Generate an event. */
    pr_event_generate("core.module-load", buf);
    return 0;
  }

  errno = EPERM;
  return -1;
}

int pr_module_unload(module *m) {
  char buf[256];

  if (m == NULL ||
      m->name == NULL) {
    errno = EINVAL;
    return -1;
  }

  /* Make sure this module has been loaded.  We can't unload a module that
   * has not been loaded, now can we?
   */

  memset(buf, '\0', sizeof(buf));
  snprintf(buf, sizeof(buf), "mod_%s.c", m->name);
  buf[sizeof(buf)-1] = '\0';

  if (pr_module_get(buf) == NULL) {
    errno = ENOENT;
    return -1;
  } 

  /* Generate an event. */
  pr_event_generate("core.module-unload", buf);

  /* Remove the module from the loaded_modules list. */
  if (m->prev) {
    m->prev->next = m->next;

  } else {
    /* This module is the start of the loaded_modules list (prev is NULL),
     * so we need to update that pointer, too.
     */
    loaded_modules = m->next;
  }

  if (m->next)
    m->next->prev = m->prev;

  m->prev = m->next = NULL;

  /* Remove the module's config, cmd, and auth tables. */
  if (m->conftable) {
    conftable *conftab;

    for (conftab = m->conftable; conftab->directive; conftab++) {
      pr_stash_remove_symbol(PR_SYM_CONF, conftab->directive, conftab->m);
    }
  }

  if (m->cmdtable) {
    cmdtable *cmdtab;

    for (cmdtab = m->cmdtable; cmdtab->command; cmdtab++) {
      if (cmdtab->cmd_type == HOOK) {
        pr_stash_remove_symbol(PR_SYM_HOOK, cmdtab->command, cmdtab->m);

      } else {
        /* All other cmd_types are for CMDs: PRE_CMD, CMD, POST_CMD, etc. */
        pr_stash_remove_symbol(PR_SYM_CMD, cmdtab->command, cmdtab->m);
      }
    }
  }

  if (m->authtable) {
    authtable *authtab;

    for (authtab = m->authtable; authtab->name; authtab++) {
      pr_stash_remove_symbol(PR_SYM_AUTH, authtab->name, authtab->m);
    }
  }

  return 0;
}

int modules_init(void) {
  register unsigned int i = 0;

  for (i = 0; static_modules[i]; i++) {
    module *m = static_modules[i];

    if (pr_module_load(m) < 0) {
      pr_log_pri(PR_LOG_ERR, "Fatal: unable to load module 'mod_%s.c': %s",
        m->name, strerror(errno));
      exit(1);
    }
  }

  return 0;
}

int init_stash(void) {
  if (symbol_pool)
    destroy_pool(symbol_pool);

  symbol_pool = make_sub_pool(permanent_pool); 
  pr_pool_tag(symbol_pool, "Stash Pool");
  memset(symbol_table, '\0', sizeof(symbol_table));

  return 0;
}
