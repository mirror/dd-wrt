/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2004-2006 The ProFTPD Project team
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
 */

/* Display of files
 * $Id: display.h,v 1.2 2006/12/06 04:29:58 castaglia Exp $
 */

#ifndef PR_DISPLAY_H
#define PR_DISPLAY_H

struct fh_rec;

/* Used to read the file handle given by fh, located on the filesystem fs, and
 * return the results, with variables expanded, to the client, using the
 * response code given by code.  Returns 0 if the file handle's contents
 * are displayed without * issue, -1 otherwise (with errno set appropriately).
 */
int pr_display_fh(struct fh_rec *fh, const char *fs, const char *code);

/* Used to read the file given by path, located on the filesystem fs, and
 * return the results, with variables expanded, to the client, using the
 * response code given by code.  Returns 0 if the file is displayed without
 * issue, -1 otherwise (with errno set appropriately).
 */
int pr_display_file(const char *path, const char *fs, const char *code);

#endif /* PR_DISPLAY_H */
