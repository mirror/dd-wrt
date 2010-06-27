/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2009 The ProFTPD Project team
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
 * distribute the resulting executable, without including the source code for
 * OpenSSL in the source distribution.
 *
 * $Id: cmd.c,v 1.2 2009/03/22 04:47:05 castaglia Exp $
 */

#include "conf.h"

cmd_rec *pr_cmd_alloc(pool *p, int argc, ...) { 
  pool *newpool = NULL;
  cmd_rec *cmd = NULL;
  va_list args;
 
  newpool = make_sub_pool(p); 
  pr_pool_tag(newpool, "cmd_rec pool");
 
  cmd = pcalloc(newpool, sizeof(cmd_rec));
  cmd->argc = argc;
  cmd->stash_index = -1;
  cmd->pool = newpool;
  cmd->tmp_pool = make_sub_pool(cmd->pool);
  pr_pool_tag(cmd->tmp_pool, "cmd_rec tmp pool");

  if (argc) {
    register unsigned int i = 0;

    cmd->argv = pcalloc(newpool, sizeof(void *) * (argc + 1));
    va_start(args, argc);

    for (i = 0; i < argc; i++)
      cmd->argv[i] = (void *) va_arg(args, char *);

    va_end(args);

    cmd->argv[argc] = NULL;
  }

  /* This table will not contain that many entries, so a low number
   * of chains should suffice.
   */
  cmd->notes = pr_table_nalloc(cmd->pool, 0, 8);

  return cmd;
}

int pr_cmd_clear_cache(cmd_rec *cmd) {
  if (cmd == NULL) {
    errno = EINVAL;
    return -1;
  }

  /* Clear the strings that have been cached for this command in the
   * notes table.
   */

  (void) pr_table_remove(cmd->notes, "displayable-str", NULL);

  return 0;
}

char *pr_cmd_get_displayable_str(cmd_rec *cmd) {
  char *res;
  int argc;
  char **argv;
  pool *p;

  if (cmd == NULL) {
    errno = EINVAL;
    return NULL;
  }

  res = pr_table_get(cmd->notes, "displayable-str", NULL);
  if (res)
    return res;

  argc = cmd->argc;
  argv = cmd->argv;
  p = cmd->pool;

  res = "";

  /* Check for "sensitive" commands. */
  if (strcmp(argv[0], C_PASS) == 0 ||
      strcmp(argv[0], C_ADAT) == 0) {
    argc = 2;
    argv[1] = "(hidden)";
  }

  if (argc > 0) {
    register unsigned int i;

    res = pstrcat(p, res, pr_fs_decode_path(p, argv[0]), NULL);

    for (i = 1; i < argc; i++) {
      res = pstrcat(p, res, " ", pr_fs_decode_path(p, argv[i]), NULL);
    }
  }

  /* XXX Check for errors here */
  pr_table_add(cmd->notes, pstrdup(cmd->pool, "displayable-str"),
    pstrdup(cmd->pool, res), 0);

  return res;
}

