/*
 * ProFTPD - mod_sftp bcrypt PBKDF2
 * Copyright (c) 2019 TJ Saunders
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
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 *
 * As a special exemption, TJ Saunders and other respective copyright holders
 * give permission to link this program with OpenSSL, and distribute the
 * resulting executable, without including the source code for OpenSSL in the
 * source distribution.
 */

#ifndef MOD_SFTP_BCRYPT_H
#define MOD_SFTP_BCRYPT_H

#include "mod_sftp.h"

#define SFTP_BCRYPT_DIGEST_LEN	32

int sftp_bcrypt_pbkdf2(pool *p, const char *passphrase, size_t passphrase_len,
  unsigned char *salt, uint32_t salt_len, uint32_t rounds,
  unsigned char *key, uint32_t key_len);

#endif /* MOD_SFTP_BCRYPT_H */
