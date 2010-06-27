/*
 * ProFTPD - mod_sftp 'publickey' user authentication
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
 * $Id: auth-publickey.c,v 1.4 2009/03/19 06:04:08 castaglia Exp $
 */

#include "mod_sftp.h"
#include "ssh2.h"
#include "packet.h"
#include "msg.h"
#include "auth.h"
#include "session.h"
#include "keys.h"
#include "keystore.h"
#include "interop.h"
#include "blacklist.h"

static const char *trace_channel = "ssh2";

static int send_pubkey_ok(const char *algo, const char *pubkey_data,
    uint32_t pubkey_len) {
  struct ssh2_packet *pkt;
  char *buf, *ptr;
  uint32_t buflen, bufsz = 1024;
  int res;

  pkt = sftp_ssh2_packet_create(sftp_pool);

  buflen = bufsz;
  ptr = buf = palloc(pkt->pool, bufsz);

  sftp_msg_write_byte(&buf, &buflen, SFTP_SSH2_MSG_USER_AUTH_PK_OK);
  sftp_msg_write_string(&buf, &buflen, algo);
  sftp_msg_write_data(&buf, &buflen, pubkey_data, pubkey_len, TRUE);

  pkt->payload = ptr;
  pkt->payload_len = (bufsz - buflen);

  (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION, "sending publickey OK");

  res = sftp_ssh2_packet_write(sftp_conn->wfd, pkt);
  if (res < 0) {
    destroy_pool(pkt->pool);
    return -1;
  }

  destroy_pool(pkt->pool);
  return 0;
}

int sftp_auth_publickey(struct ssh2_packet *pkt, const char *orig_user,
    const char *user, const char *service, char **buf, uint32_t *buflen,
    int *send_userauth_fail) {
  int have_signature, pubkey_type;
  char *pubkey_algo = NULL, *pubkey_data;
  uint32_t pubkey_len;
  struct passwd *pw;

  have_signature = sftp_msg_read_bool(pkt->pool, buf, buflen);

  if (sftp_interop_supports_feature(SFTP_SSH2_FEAT_HAVE_PUBKEY_ALGO)) {
    pubkey_algo = sftp_msg_read_string(pkt->pool, buf, buflen);
  }
  pubkey_len = sftp_msg_read_int(pkt->pool, buf, buflen);
  pubkey_data = sftp_msg_read_data(pkt->pool, buf, buflen, pubkey_len);

  if (pubkey_algo == NULL) {
    /* The client did not send the string identify the public key algorithm.
     * Thus we need to extract the algorithm string from the public key data.
     */
    pubkey_algo = sftp_msg_read_string(pkt->pool, &pubkey_data, &pubkey_len);
  }

  pr_trace_msg(trace_channel, 9, "client sent '%s' public key %s",
    pubkey_algo, have_signature ? "with signature" : "without signature");

  if (strcmp(pubkey_algo, "ssh-rsa") == 0) {
    pubkey_type = EVP_PKEY_RSA;

  } else if (strcmp(pubkey_algo, "ssh-dss") == 0) {
    pubkey_type = EVP_PKEY_DSA;

  } else {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "unsupported public key algorithm '%s' requested, rejecting request",
      pubkey_algo);

    *send_userauth_fail = TRUE;
    errno = EINVAL;
    return 0;
  }

  if (sftp_keys_verify_pubkey_type(pkt->pool, pubkey_data, pubkey_len,
      pubkey_type) != TRUE) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "unable to verify that given public key matches given '%s' algorithm",
      pubkey_algo);

    *send_userauth_fail = TRUE;
    errno = EINVAL;
    return 0;
  }

  (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
    "public key fingerprint: %s",
    sftp_keys_get_fingerprint(pkt->pool, pubkey_data, pubkey_len,
      SFTP_KEYS_FP_DIGEST_MD5));

  pw = pr_auth_getpwnam(pkt->pool, user);
  if (pw == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "no account for user '%s' found", user);

    pr_log_auth(PR_LOG_NOTICE,
      "USER %s: no such user found from %s [%s] to %s:%d", user,
      session.c->remote_name, pr_netaddr_get_ipstr(session.c->remote_addr),
      pr_netaddr_get_ipstr(session.c->local_addr), session.c->local_port);

    *send_userauth_fail = TRUE;
    errno = ENOENT;
    return 0;
  }

  if (!have_signature) {
    /* We don't perform the actual authentication just yet; we need to
     * let the client know that the pubkey algorithms are acceptable.
     */
    if (send_pubkey_ok(pubkey_algo, pubkey_data, pubkey_len) < 0) {
      return -1;
    }

    return 0;

  } else {
    const unsigned char *id;
    char *buf2, *ptr2, *signature_data;
    uint32_t buflen2, bufsz2, id_len, signature_len;

    if (sftp_blacklist_reject_key(pkt->pool, pubkey_data, pubkey_len)) {
      *send_userauth_fail = TRUE;
      errno = EPERM;
      return 0;
    }

    signature_len = sftp_msg_read_int(pkt->pool, buf, buflen);
    signature_data = sftp_msg_read_data(pkt->pool, buf, buflen, signature_len);

    /* The client signed the request as well; we need to authenticate the
     * user with the given pubkey now.  If that succeeds, we use the
     * signature to verify the request.  And if that succeeds, then we're
     * done authenticating.
     */

    if (sftp_keystore_verify_user_key(pkt->pool, user, pubkey_data,
        pubkey_len) < 0) {
      *send_userauth_fail = TRUE;
      return 0;
    }

    /* Make sure the signature matches as well. */

    id_len = sftp_session_get_id(&id);

    /* XXX Is this buffer large enough?  Too large? */
    bufsz2 = buflen2 = 2048;
    ptr2 = buf2 = sftp_msg_getbuf(pkt->pool, bufsz2);

    sftp_msg_write_data(&buf2, &buflen2, (char *) id, id_len, TRUE);
    sftp_msg_write_byte(&buf2, &buflen2, SFTP_SSH2_MSG_USER_AUTH_REQUEST);
    sftp_msg_write_string(&buf2, &buflen2, orig_user);

    if (sftp_interop_supports_feature(SFTP_SSH2_FEAT_SERVICE_IN_PUBKEY_SIG)) {
      sftp_msg_write_string(&buf2, &buflen2, service);

    } else {
      sftp_msg_write_string(&buf2, &buflen2, "ssh-userauth");
    }

    if (sftp_interop_supports_feature(SFTP_SSH2_FEAT_HAVE_PUBKEY_ALGO)) {
      sftp_msg_write_string(&buf2, &buflen2, "publickey");
      sftp_msg_write_bool(&buf2, &buflen2, TRUE);
      sftp_msg_write_string(&buf2, &buflen2, pubkey_algo);

    } else {
      sftp_msg_write_bool(&buf2, &buflen2, TRUE);
    }

    sftp_msg_write_data(&buf2, &buflen2, pubkey_data, pubkey_len, TRUE);

    if (sftp_keys_verify_signed_data(pkt->pool, pubkey_algo, pubkey_data,
        pubkey_len, signature_data, signature_len, (unsigned char *) ptr2,
        (bufsz2 - buflen2)) < 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "failed to verify '%s' signature on public key auth request for "
        "user '%s'", pubkey_algo, orig_user);
      *send_userauth_fail = TRUE;
      return 0;
    }
  }

  /* Make sure the user is authorized to login.  Normally this is checked
   * as part of the password verification process, but in the case of
   * publickey authentication, there is no password to verify.
   */

  if (pr_auth_authorize(pkt->pool, user) != PR_AUTH_OK) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "authentication for user '%s' failed: User not authorized", user);
    pr_log_auth(PR_LOG_NOTICE, "USER %s (Login failed): User not authorized "
      "for login", user);
    *send_userauth_fail = TRUE;
    errno = EACCES;
    return 0;
  }

  return 1;
}
