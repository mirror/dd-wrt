/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2006-2010 The ProFTPD Project team
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
 * As a special exemption, the ProFTPD Project and other respective copyright
 * holders give permission to link this program with OpenSSL, and distribute
 * the resulting executable, without including the source code for OpenSSL in
 * the source distribution.
 */

/* Trace API
 * $Id: trace.h,v 1.2 2010/02/10 20:54:29 castaglia Exp $
 */

#ifndef PR_TRACE_H
#define PR_TRACE_H

#define PR_TRACE_DEFAULT_CHANNEL	"DEFAULT"

pr_table_t *pr_trace_get_table(void);
int pr_trace_get_level(const char *);
int pr_trace_set_file(const char *);
int pr_trace_set_level(const char *, int);
int pr_trace_msg(const char *, int, const char *, ...)
#ifdef __GNUC__
      __attribute__ ((format (printf, 3, 4)));
#else
      ;
#endif

#endif /* PR_TRACE_H */
