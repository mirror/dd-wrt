/*
 * ProFTPD - mod_sftp public key store (including RFC4716 public key file
 *                                      format)
 * Copyright (c) 2008-2010 TJ Saunders
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
 * $Id: keystore.h,v 1.3 2010/02/10 18:34:34 castaglia Exp $
 */

#include "mod_sftp.h"

#ifndef MOD_SFTP_KEYSTORE_H
#define MOD_SFTP_KEYSTORE_H

int sftp_keystore_init(void);
int sftp_keystore_free(void);

int sftp_keystore_supports_store(const char *, unsigned int);
int sftp_keystore_verify_host_key(pool *, const char *, const char *,
  const char *, char *, uint32_t);
int sftp_keystore_verify_user_key(pool *, const char *, char *, uint32_t);

#endif
