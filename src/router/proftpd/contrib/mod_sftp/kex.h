/*
 * ProFTPD - mod_sftp key exchange (kex)
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
 * $Id: kex.h,v 1.3 2009/11/22 19:46:50 castaglia Exp $
 */

#include "mod_sftp.h"

#ifndef MOD_SFTP_KEX_H
#define MOD_SFTP_KEX_H

int sftp_kex_handle(struct ssh2_packet *);
int sftp_kex_init(const char *, const char *);
int sftp_kex_free(void);

int sftp_kex_rekey(void);
int sftp_kex_rekey_set_interval(int);
int sftp_kex_rekey_set_timeout(int);

int sftp_kex_send_first_kexinit(void);

#define SFTP_KEX_DH_GROUP_MIN	1024
#define SFTP_KEX_DH_GROUP_MAX	8192

#endif
