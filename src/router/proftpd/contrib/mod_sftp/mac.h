/*
 * ProFTPD - mod_sftp MAC mgmt
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
 * $Id: mac.h,v 1.2 2009/02/13 23:41:19 castaglia Exp $
 */

#include "mod_sftp.h"

#ifndef MOD_SFTP_MAC_H
#define MOD_SFTP_MAC_H

#include "packet.h"

/* Returns the block size of the negotiated MAC algorithm, or 0 if no MAC
 * has been negotiated yet.
 */
size_t sftp_mac_get_block_size(void);
void sftp_mac_set_block_size(size_t);

const char *sftp_mac_get_read_algo(void);
int sftp_mac_set_read_algo(const char *);
int sftp_mac_set_read_key(pool *, const EVP_MD *, const BIGNUM *, const char *,
  uint32_t);
int sftp_mac_read_data(struct ssh2_packet *);

const char *sftp_mac_get_write_algo(void);
int sftp_mac_set_write_algo(const char *);
int sftp_mac_set_write_key(pool *, const EVP_MD *, const BIGNUM *, const char *,
  uint32_t);
int sftp_mac_write_data(struct ssh2_packet *);

#endif
