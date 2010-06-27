/*
 * ProFTPD - mod_sftp user authentication (auth)
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
 * $Id: auth.h,v 1.5 2009/11/05 17:40:45 castaglia Exp $
 */

#include "mod_sftp.h"

#ifndef MOD_SFTP_AUTH_H
#define MOD_SFTP_AUTH_H

#include "packet.h"

#define SFTP_AUTH_FL_METH_PUBLICKEY	0x001
#define SFTP_AUTH_FL_METH_KBDINT	0x002
#define SFTP_AUTH_FL_METH_PASSWORD	0x004
#define SFTP_AUTH_FL_METH_HOSTBASED	0x008 

char *sftp_auth_get_default_dir(void);
int sftp_auth_handle(struct ssh2_packet *);
int sftp_auth_init(void);

/* Handles 'hostbased' user authentication. */
int sftp_auth_hostbased(struct ssh2_packet *, const char *, const char *,
  const char *, char **, uint32_t *, int *);

/* Handles 'keyboard-interactive' user authentication. */
int sftp_auth_kbdint(struct ssh2_packet *, const char *, const char *,
  const char *, char **, uint32_t *, int *);

/* Handles 'password' user authentication. */
int sftp_auth_password(struct ssh2_packet *, const char *, const char *,
  const char *, char **, uint32_t *, int *);

/* Handles 'publickey' user authentication. */
int sftp_auth_publickey(struct ssh2_packet *, const char *, const char *,
  const char *, char **, uint32_t *, int *);

#endif
