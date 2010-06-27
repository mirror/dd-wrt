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
 * $Id: cmd.h,v 1.2 2009/02/16 03:14:02 castaglia Exp $
 */

#ifndef PR_CMD_H
#define PR_CMD_H

cmd_rec *pr_cmd_alloc(pool *, int, ...);
int pr_cmd_clear_cache(cmd_rec *);
char *pr_cmd_get_displayable_str(cmd_rec *);

/* Implemented in main.c */
int pr_cmd_read(cmd_rec **);
int pr_cmd_dispatch(cmd_rec *);
int pr_cmd_dispatch_phase(cmd_rec *, int, int);
#define PR_CMD_DISPATCH_FL_SEND_RESPONSE	0x001
#define PR_CMD_DISPATCH_FL_CLEAR_RESPONSE	0x002

void pr_cmd_set_handler(void (*)(server_rec *, conn_t *));

#endif /* PR_CMD_H */
