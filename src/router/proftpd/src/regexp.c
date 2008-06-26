/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 1997, 1998 Public Flood Software
 * Copyright (c) 1999, 2000 MacGyver aka Habeeb J. Dihu <macgyver@tos.net>
 * Copyright (c) 2001, 2002, 2003 The ProFTPD Project team
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
 * Regex management code
 * $Id: regexp.c,v 1.13 2004/09/05 00:38:14 castaglia Exp $
 */

#include "conf.h"

#ifdef HAVE_REGEX_H

static pool *regexp_pool = NULL;
static array_header *regexp_list = NULL;

static void regexp_cleanup(void) {
  /* Only perform this cleanup if necessary */
  if (regexp_pool) {
    register unsigned int i = 0;
    regex_t **regexp = (regex_t **) regexp_list->elts;

    for (i = 0; i < regexp_list->nelts; i++) {
      if (regexp[i]) {

        /* This frees memory associated with this pointer by regcomp(3). */
        regfree(regexp[i]);

        /* This frees the memory allocated for the pointer itself. */
        free(regexp[i]);
      }
    }

    destroy_pool(regexp_pool);
    regexp_pool = NULL;
    regexp_list = NULL;
  }
}

static void regexp_exit_ev(const void *event_data, void *user_data) {
  regexp_cleanup();
  return;
}

static void regexp_restart_ev(const void *event_data, void *user_data) {
  regexp_cleanup();
  return;
}

regex_t *pr_regexp_alloc(void) {
  regex_t *preg = NULL;

  /* If no regex-tracking list has been allocated, create one.  Register a
   * cleanup handler for this pool, to call regfree(3) on all regex_t pointers
   * in the list.
   */
  if (!regexp_pool) {
    regexp_pool = make_sub_pool(permanent_pool);
    pr_pool_tag(regexp_pool, "Regexp Pool");
    regexp_list = make_array(regexp_pool, 0, sizeof(regex_t *));
  }

  preg = calloc(1, sizeof(regex_t));
  if (preg == NULL) {
    pr_log_pri(PR_LOG_ERR, "fatal: memory exhausted");
    exit(1);
  }

  /* Add this pointer to the array. */
  *((regex_t **) push_array(regexp_list)) = preg;

  return preg;
}

void pr_regexp_free(regex_t *regex) {
  register unsigned int i = 0;
  regex_t **regexp = NULL;

  if (!regex || !regexp_list)
    return;

  regexp = (regex_t **) regexp_list->elts;

  for (i = 0; i < regexp_list->nelts; i++) {
    if (regexp[i] == regex) {

      /* This frees memory associated with this pointer by regcomp(3). */
      regfree(regexp[i]);

      /* This frees the memory allocated for the pointer itself. */
      free(regexp[i]);

      regexp[i] = NULL;
    }
  }
}

void init_regexp(void) {

  /* Register a restart handler for the regexp pool, so that when restarting,
   * regfree(3) is called on each of the regex_t pointers in a
   * regex_t-tracking array, thus preventing memory leaks on a long-running
   * daemon.
   *
   * This registration is done here so that it only happens once.
   */
  pr_event_register(NULL, "core.restart", regexp_restart_ev, NULL);
  pr_event_register(NULL, "core.exit", regexp_exit_ev, NULL);
}

#endif
