/*
 * ProFTPD - mod_sftp message format
 * Copyright (c) 2008-2009 TJ Saunders
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
 * As a special exemption, TJ Saunders and other respective copyright holders
 * give permission to link this program with OpenSSL, and distribute the
 * resulting executable, without including the source code for OpenSSL in the
 * source distribution.
 *
 * $Id: msg.h,v 1.2 2009/02/13 23:41:19 castaglia Exp $
 */

#include "mod_sftp.h"

#ifndef MOD_SFTP_MSG_H
#define MOD_SFTP_MSG_H

char sftp_msg_read_byte(pool *, char **, uint32_t *);
int sftp_msg_read_bool(pool *, char **, uint32_t *);
char *sftp_msg_read_data(pool *, char **, uint32_t *, size_t);
uint32_t sftp_msg_read_int(pool *, char **, uint32_t *);
BIGNUM *sftp_msg_read_mpint(pool *, char **, uint32_t *);
char *sftp_msg_read_string(pool *, char **, uint32_t *);

void sftp_msg_write_byte(char **, uint32_t *, char);
void sftp_msg_write_bool(char **, uint32_t *, char);
void sftp_msg_write_data(char **, uint32_t *, const char *, size_t, int);
void sftp_msg_write_int(char **, uint32_t *, uint32_t);
void sftp_msg_write_mpint(char **, uint32_t *, const BIGNUM *);
void sftp_msg_write_string(char **, uint32_t *, const char *);

/* Utility method for obtaining a scratch buffer for constructing SSH2
 * messages without necessarily needing an SSH2 packet.
 */
char *sftp_msg_getbuf(pool *, size_t);

#endif
