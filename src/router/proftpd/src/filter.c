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
 * $Id: filter.c,v 1.1 2009/02/22 00:28:07 castaglia Exp $
 */

#include "conf.h"

static const char *trace_channel = "filter";

int pr_filter_allow_path(xaset_t *set, const char *path) {
#if defined(HAVE_REGEX_H) && defined(HAVE_REGCOMP)
  regex_t *preg;
  int res;

  /* Check any relevant PathAllowFilter first. */

  preg = get_param_ptr(set, "PathAllowFilter", FALSE);
  if (preg) {
    res = regexec(preg, path, 0, NULL, 0);
    if (res != 0) {
      return PR_FILTER_ERR_FAILS_ALLOW_FILTER;
    }

    pr_trace_msg(trace_channel, 8, "'%s' allowed by PathAllowFilter", path);
  }

  /* Next check any applicable PathDenyFilter. */

  preg = get_param_ptr(CURRENT_CONF, "PathDenyFilter", FALSE);
 
  if (preg) {
    res = regexec(preg, path, 0, NULL, 0);
    if (res == 0) {
      return PR_FILTER_ERR_FAILS_DENY_FILTER;
    } 

    pr_trace_msg(trace_channel, 8, "'%s' allowed by PathDenyFilter", path);
  }

  return 0;
#else
  return 0;
#endif
}
