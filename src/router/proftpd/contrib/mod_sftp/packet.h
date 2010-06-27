/*
 * ProFTPD - mod_sftp packet IO
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
 * $Id: packet.h,v 1.3 2010/02/15 22:03:52 castaglia Exp $
 */

#include "mod_sftp.h"

#ifndef MOD_SFTP_PACKET_H
#define MOD_SFTP_PACKET_H

/* From RFC 4253, Section 6 */
struct ssh2_packet {
  pool *pool;

  /* Length of the packet, not including mac or packet_len field itself. */
  uint32_t packet_len;

  /* Length of the padding field. */
  unsigned char padding_len;

  char *payload;
  uint32_t payload_len;

  /* Must be at least 4 bytes of padding, with a maximum of 255 bytes. */
  char *padding;

  /* Message Authentication Code. */
  char *mac;
  uint32_t mac_len;

  /* Packet sequence number. */
  uint32_t seqno;
};

#define SFTP_MIN_PADDING_LEN	4
#define SFTP_MAX_PADDING_LEN	255

/* From the SFTP Draft, Section 4. */
struct sftp_packet {
  uint32_t packet_len;
  char packet_type;
  uint32_t request_id;
};

struct ssh2_packet *sftp_ssh2_packet_create(pool *);
char sftp_ssh2_packet_get_mesg_type(struct ssh2_packet *);
const char *sftp_ssh2_packet_get_mesg_type_desc(char);

/* Returns a struct timeval populated with the time we last received an SSH2
 * packet from the client.
 */
int sftp_ssh2_packet_get_last_recvd(time_t *);

/* Returns a struct timeval populated with the time we last sent an SSH2
 * packet from the client.
 */
int sftp_ssh2_packet_get_last_sent(time_t *);

int sftp_ssh2_packet_read(int, struct ssh2_packet *);
int sftp_ssh2_packet_sock_read(int, void *, size_t);
int sftp_ssh2_packet_write(int, struct ssh2_packet *);

int sftp_ssh2_packet_handle(void);

int sftp_ssh2_packet_rekey_reset(void);
int sftp_ssh2_packet_rekey_set_seqno(uint32_t);
int sftp_ssh2_packet_rekey_set_size(off_t);

int sftp_ssh2_packet_send_version(void);
int sftp_ssh2_packet_set_poll_timeout(int);

#endif
