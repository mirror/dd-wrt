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
 * $Id: session.h,v 1.2 2009/09/02 17:58:54 castaglia Exp $
 */

#ifndef PR_SESSION_H
#define PR_SESSION_H

/* Returns the current protocol name in use.
 *
 * The PR_SESS_PROTO_FL_LOGOUT flag is used when retrieving the protocol
 * name to display in the login/logout messages, e.g. "FTP" or "SSH2".
 */
const char *pr_session_get_protocol(int);
#define PR_SESS_PROTO_FL_LOGOUT		0x01

/* Returns a so-called "tty name" suitable for use via PAM, and in WtmpLog
 * logging.
 */
const char *pr_session_get_ttyname(pool *);

/* Marks the current session as "idle" both in the scoreboard and in the
 * proctitle.
 */
int pr_session_set_idle(void);

/* Sets the current protocol name. */
int pr_session_set_protocol(const char *);

#endif /* PR_SESSION_H */
