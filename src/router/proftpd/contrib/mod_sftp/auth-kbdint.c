/*
 * ProFTPD - mod_sftp 'keyboard-interactive' user authentication
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
 * $Id: auth-kbdint.c,v 1.3 2009/03/19 06:04:08 castaglia Exp $
 */

#include "mod_sftp.h"
#include "ssh2.h"
#include "packet.h"
#include "auth.h"
#include "msg.h"
#include "cipher.h"
#include "mac.h"
#include "utf8.h"
#include "kbdint.h"

static const char *trace_channel = "ssh2";

int sftp_auth_kbdint(struct ssh2_packet *pkt, const char *orig_user,
    const char *user, const char *service, char **buf, uint32_t *buflen,
    int *send_userauth_fail) {
  const char *cipher_algo, *mac_algo;
  struct passwd *pw;
  char *submethods;
  sftp_kbdint_driver_t *driver;
  int res = -1;

  if (sftp_kbdint_have_drivers() == 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "no 'keyboard-interactive' drivers currently registered, unable to "
      "authenticate user '%s' via 'keyboard-interactive' method", user);

    *send_userauth_fail = TRUE;
    errno = EPERM;
    return 0;
  }

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

  cipher_algo = sftp_cipher_get_read_algo();
  mac_algo = sftp_mac_get_read_algo();

  /* XXX Is this too strict?  For PAM authentication, no -- but for S/Key or
   * one-time password authencation, maybe yes.
   */
  if (strcmp(cipher_algo, "none") == 0 ||
      strcmp(mac_algo, "none") == 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "cipher algorithm '%s' or MAC algorithm '%s' unacceptable for "
      "keyboard-interactive authentication, denying authentication request",
      cipher_algo, mac_algo);

    *send_userauth_fail = TRUE;
    errno = EPERM;
    return 0;
  }

  /* XXX Read off the deprecated language string. */
  sftp_msg_read_string(pkt->pool, buf, buflen);

  submethods = sftp_msg_read_string(pkt->pool, buf, buflen);

  if (strlen(submethods) > 0) {
    pr_trace_msg(trace_channel, 8, "client suggested 'keyboard-interactive' "
      "methods: %s", submethods);
  }

  /* XXX get our own get_shared_name() function (see kex.c), to see if
   * any of the "hints" sent by the client match any of the registered
   * kbdint drivers.
   */

  driver = sftp_kbdint_first_driver();
  while (driver) {
    pr_signals_handle();

    pr_trace_msg(trace_channel, 3, "trying kbdint driver '%s' for user '%s'",
      driver->driver_name, user);

    res = driver->open(driver, user);
    if (res < 0) {
      driver = sftp_kbdint_next_driver();
      continue;
    }

    res = driver->authenticate(driver, user);
    driver->close(driver);

    if (res == 0)
      break; 

    driver = sftp_kbdint_next_driver();
  }

  if (res < 0) {
    *send_userauth_fail = TRUE;
    errno = EPERM;
    return 0;
  }

  return 1;
}
