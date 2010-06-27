/*
 * ProFTPD - mod_sftp disconnect msgs
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
 * $Id: disconnect.c,v 1.4 2009/08/28 16:14:23 castaglia Exp $
 */

#include "mod_sftp.h"
#include "ssh2.h"
#include "msg.h"
#include "packet.h"
#include "disconnect.h"

struct disconnect_reason {
  uint32_t code;
  const char *explain;
  const char *lang;
};

static struct disconnect_reason explanations[] = {
  { SFTP_SSH2_DISCONNECT_HOST_NOT_ALLOWED_TO_CONNECT, "Host not allowed to connect", NULL },
  { SFTP_SSH2_DISCONNECT_PROTOCOL_ERROR, "Protocol error", NULL },
  { SFTP_SSH2_DISCONNECT_KEY_EXCHANGE_FAILED, "Key exchange failed", NULL },
  { SFTP_SSH2_DISCONNECT_MAC_ERROR, "MAC error", NULL },
  { SFTP_SSH2_DISCONNECT_COMPRESSION_ERROR, "Compression error", NULL },
  { SFTP_SSH2_DISCONNECT_SERVICE_NOT_AVAILABLE, "Requested service not available", NULL },
  { SFTP_SSH2_DISCONNECT_PROTOCOL_VERSION_NOT_SUPPORTED, "Protocol version not supported", NULL },
  { SFTP_SSH2_DISCONNECT_HOST_KEY_NOT_VERIFIABLE, "Host key not verifiable", NULL },
  { SFTP_SSH2_DISCONNECT_CONNECTION_LOST, "Connection lost", NULL },
  { SFTP_SSH2_DISCONNECT_BY_APPLICATION, "Application error", NULL },
  { SFTP_SSH2_DISCONNECT_TOO_MANY_CONNECTIONS, "Too many connections", NULL },
  { SFTP_SSH2_DISCONNECT_AUTH_CANCELLED_BY_USER, "Authentication cancelled by user", NULL },
  { SFTP_SSH2_DISCONNECT_NO_MORE_AUTH_METHODS_AVAILABLE, "No other authentication mechanisms available", NULL },
  { SFTP_SSH2_DISCONNECT_ILLEGAL_USER_NAME, "Illegal user name", NULL },
  { 0, NULL, NULL }
};

static const char *trace_channel = "ssh2";

void sftp_disconnect_send(uint32_t reason, const char *explain,
    const char *file, int lineno, const char *func) {
  struct ssh2_packet *pkt;
  const char *lang = "en-US";
  char *buf, *ptr;
  uint32_t buflen, bufsz;
  int sockfd;

  /* Send the client a DISCONNECT mesg. */
  pkt = sftp_ssh2_packet_create(sftp_pool);

  buflen = bufsz = 1024;
  ptr = buf = palloc(pkt->pool, bufsz);

  if (explain == NULL) {
    register unsigned int i;

    for (i = 0; explanations[i].explain; i++) {
      if (explanations[i].code == reason) {
        explain = explanations[i].explain;
        lang = explanations[i].lang;
        if (lang == NULL)
          lang = "en-US";

        break;
      }
    }

  } else {
    lang = "en-US";
  }

  if (strlen(func) > 0) {
    pr_trace_msg(trace_channel, 9, "disconnecting (%s) [at %s:%d:%s()]",
      explain, file, lineno, func);

  } else {
    pr_trace_msg(trace_channel, 9, "disconnecting (%s) [at %s:%d]", explain,
      file, lineno);
  }

  sftp_msg_write_byte(&buf, &buflen, SFTP_SSH2_MSG_DISCONNECT);
  sftp_msg_write_int(&buf, &buflen, reason);
  sftp_msg_write_string(&buf, &buflen, explain);
  sftp_msg_write_string(&buf, &buflen, lang);

  pkt->payload = ptr;
  pkt->payload_len = (bufsz - buflen);

  (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION, "disconnecting (%s)",
    explain);

  /* If we are called very early in the connection lifetime, then the
   * sftp_conn variable may not have been set yet, thus the conditional here.
   */
  if (sftp_conn != NULL) {
    sockfd = sftp_conn->wfd;

  } else {
    sockfd = session.c->wfd;
  }

  (void) sftp_ssh2_packet_write(sockfd, pkt);
  destroy_pool(pkt->pool);
}

void sftp_disconnect_conn(uint32_t reason, const char *explain,
    const char *file, int lineno, const char *func) {
  sftp_disconnect_send(reason, explain, file, lineno, func);

#ifdef PR_DEVEL_COREDUMP
  abort();

#else
  end_login(1);
#endif /* PR_DEVEL_COREDUMP */
}
