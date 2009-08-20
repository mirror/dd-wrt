/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2008 The ProFTPD Project team
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
 * As a special exemption, The ProFTPD Project and other respective copyright
 * holders give permission to link this program with OpenSSL, and distribute
 * the resulting executable, without including the source code for OpenSSL in
 * the source distribution.
 */

/* Transfer rate/throttling functions
 * $Id: throttle.h,v 1.1 2008/05/06 05:13:06 castaglia Exp $
 */

#ifndef PR_THROTTLE_H
#define PR_THROTTLE_H

int pr_throttle_have_rate(void);
void pr_throttle_init(cmd_rec *);
void pr_throttle_pause(off_t, int);

#endif /* PR_THROTTLE_H */
