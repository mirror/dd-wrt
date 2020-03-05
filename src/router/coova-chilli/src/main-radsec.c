/* -*- mode: c; c-basic-offset: 2 -*- */
/*
 * Copyright (C) 2009-2012 David Bird (Coova Technologies) <support@coova.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: David Bird <david@coova.com>
 */

#define MAIN_FILE

#include "chilli.h"

extern struct options_t _options;

static struct {

  struct radius_t *radius_auth;
  struct radius_t *radius_acct;
  struct radius_t *radius_cli;

  struct sockaddr_in auth_peer;
  struct sockaddr_in acct_peer;

  openssl_env * env;

  struct conn_t conn;

  struct radius_packet_t pack;

  int pos;

} server;


static int connect_ssl(struct in_addr *addr, int port) {
  if (conn_sock(&server.conn, addr, port)) {
    return -1;
  }
  server.conn.sslcon = openssl_connect_fd(server.env, server.conn.sock, 10);
  if (!server.conn.sslcon) {
    syslog(LOG_ERR, "%s: Failed to connect to %s:%d", strerror(errno), inet_ntoa(*addr), port);
    return -1;
  }
  return 0;
}

static int shutdown_ssl() {
  if (server.conn.sslcon) {
    openssl_shutdown(server.conn.sslcon, 2);
    openssl_free(server.conn.sslcon);
  }
  close(server.conn.sock);
  server.conn.sock = 0;
  server.conn.connected = 0;
  return 0;
}

static void process_radius(struct radius_packet_t *pack, ssize_t len) {
  int attempts = 0;

try_again:

  if (attempts++ == 5) {
    syslog(LOG_ERR, "%s: Dropping RADIUS packet!", strerror(errno));
    return;
  }

  syslog(LOG_DEBUG, "%s:%s", __FILE__, __FUNCTION__);

  if (!server.conn.connected) {
    syslog(LOG_DEBUG, "RADSEC: Connecting to %s:2083",
           inet_ntoa(_options.radiusserver1));
    if (connect_ssl(&_options.radiusserver1, 2083)) {
      syslog(LOG_ERR, "%d Could not connect to RadSec server %s!",
             errno, inet_ntoa(_options.radiusserver1));
      syslog(LOG_DEBUG, "RADSEC: Connecting to %s:2083",
             inet_ntoa(_options.radiusserver1));
      if (connect_ssl(&_options.radiusserver2, 2083)) {
	syslog(LOG_ERR, "%d Could not connect to RadSec server %s!",
               errno, inet_ntoa(_options.radiusserver2));
      } else {
	syslog(LOG_DEBUG, "RADSEC: Connected to %s:2083",
               inet_ntoa(_options.radiusserver2));
	server.conn.connected = 1;
      }
    } else {
      syslog(LOG_DEBUG, "RADSEC: Connected to %s:2083",
             inet_ntoa(_options.radiusserver1));
      server.conn.connected = 1;
    }
  }

  if (!server.conn.connected) {
    sleep(1);
    goto try_again;
  }

  {
    int l = openssl_write(server.conn.sslcon, (char *)pack, len, 0);
    syslog(LOG_DEBUG, "ssl_write %d",l);
    if (l <= 0) {
      shutdown_ssl();
      /*
       *  Immediately start to reconnect
       */
      goto try_again;
    }
  }
}

static void process_radius_reply() {
  uint8_t *d = (uint8_t *) &server.pack;
  int l = openssl_read(server.conn.sslcon, (char *)d, 4, 0);
  syslog(LOG_DEBUG, "reply %d", l);
  if (l == 4) {
    int len = ntohs(server.pack.length) - 4;
    l = openssl_read(server.conn.sslcon, (char *)(d + 4), len, 0);
    syslog(LOG_DEBUG, "reply %d", l);
    if (l == len) {
      syslog(LOG_DEBUG, "reply +%d", len);
      switch (server.pack.code) {
        case RADIUS_CODE_ACCESS_ACCEPT:
        case RADIUS_CODE_ACCESS_REJECT:
        case RADIUS_CODE_ACCESS_CHALLENGE:
          syslog(LOG_DEBUG, "reply auth %d", len);
          radius_pkt_send(server.radius_auth, &server.pack, &server.auth_peer);
          break;
        case RADIUS_CODE_ACCOUNTING_RESPONSE:
          syslog(LOG_DEBUG, "reply acct %d", len);
          radius_pkt_send(server.radius_acct, &server.pack, &server.acct_peer);
          break;
        case RADIUS_CODE_COA_REQUEST:
        case RADIUS_CODE_DISCONNECT_REQUEST:
        case RADIUS_CODE_STATUS_REQUEST:
          if (_options.coaport) {
            syslog(LOG_DEBUG, "reply coa %d", len);
            radius_pkt_send(server.radius_cli, &server.pack, &server.acct_peer);
          }
          break;
      }
    }
  }
  if (l <= 0) {
    shutdown_ssl();
  }
}

static int cb_radius_auth_conf(struct radius_t *radius,
			       struct radius_packet_t *pack,
			       struct radius_packet_t *pack_req,
			       void *cbp) {
  process_radius(pack, pack->length);
  return 0;
}

int mradsec_main(int argc, char **argv) {
  struct radius_packet_t radius_pack;
  struct in_addr radiuslisten;

  struct timeval timeout;

  int maxfd = 0;
  fd_set fdread;
  fd_set fdwrite;
  fd_set fdexcep;

  ssize_t status;

  int keep_going = 1;
  int reload_config = 1;

  int selfpipe;

  options_init();

  selfpipe = selfpipe_init();

  chilli_signals(&keep_going, &reload_config);

  process_options(argc, argv, 1);

  radiuslisten.s_addr = htonl(INADDR_ANY);

  memset(&server, 0, sizeof(server));

  if (!(server.env = initssl_cli())) {
    syslog(LOG_ERR, "Failed to create ssl environment");
    return -1;
  }

  if (radius_new(&server.radius_auth, &radiuslisten,
		 _options.radiusauthport ? _options.radiusauthport : RADIUS_AUTHPORT,
		 0, 0)) {
    syslog(LOG_ERR, "Failed to create radius");
    return -1;
  }

  if (radius_new(&server.radius_acct, &radiuslisten,
		 _options.radiusacctport ? _options.radiusacctport : RADIUS_ACCTPORT,
		 0, 0)) {
    syslog(LOG_ERR, "Failed to create radius");
    return -1;
  }

  if (_options.coaport) {
    if (radius_new(&server.radius_cli, &radiuslisten, 0, 0, 0) ||
	radius_init_q(server.radius_cli, 8)) {
      syslog(LOG_ERR, "Failed to create radius");
      return -1;
    }
    radius_set(server.radius_cli, 0, 0);
    radius_set_cb_auth_conf(server.radius_cli, cb_radius_auth_conf);
  }

  radius_set(server.radius_auth, 0, 0);
  radius_set(server.radius_acct, 0, 0);

  if (_options.gid && setgid(_options.gid)) {
    syslog(LOG_ERR, "%d setgid(%d) failed while running with gid = %d\n",
           errno, _options.gid, getgid());
  }

  if (_options.uid && setuid(_options.uid)) {
    syslog(LOG_ERR, "%d setuid(%d) failed while running with uid = %d\n",
           errno, _options.uid, getuid());
  }

  while (keep_going) {

    if (reload_config) {
      reload_options(argc, argv);
      reload_config = 0;
    }

    FD_ZERO(&fdread);
    FD_ZERO(&fdwrite);
    FD_ZERO(&fdexcep);

    FD_SET(selfpipe, &fdread);
    FD_SET(server.radius_auth->fd, &fdread);
    FD_SET(server.radius_acct->fd, &fdread);

    if (server.radius_auth->fd > maxfd)
      maxfd = server.radius_auth->fd;

    if (server.radius_acct->fd > maxfd)
      maxfd = server.radius_acct->fd;

    if (server.conn.sock) {
      FD_SET(server.conn.sock, &fdread);
      if (server.conn.sock > maxfd)
	maxfd = server.conn.sock;
    }

    if (server.radius_cli) {
      FD_SET(server.radius_cli->fd, &fdread);
      if (server.radius_cli->fd > maxfd)
	maxfd = server.radius_cli->fd;
    }

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    status = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);

    switch (status) {
      case -1:
        if (errno != EINTR)
          syslog(LOG_ERR, "%s: select() returned -1!", strerror(errno));
        break;

      case 0:
      default:
        if (status > 0) {
          struct sockaddr_in addr;
          socklen_t fromlen = sizeof(addr);

          if (FD_ISSET(selfpipe, &fdread)) {
            chilli_handle_signal(0, 0);
          }

          if (FD_ISSET(server.radius_auth->fd, &fdread)) {
            /*
             *    ---> Authentication
             */

            if ((status = recvfrom(server.radius_auth->fd, &radius_pack, sizeof(radius_pack), 0,
                                   (struct sockaddr *) &addr, &fromlen)) <= 0) {
              syslog(LOG_ERR, "%s: recvfrom() failed", strerror(errno));

              return -1;
            }

            memcpy(&server.auth_peer, &addr, sizeof(addr));

            process_radius(&radius_pack, status);
          }

          if (FD_ISSET(server.radius_acct->fd, &fdread)) {
            /*
             *    ---> Accounting
             */

            syslog(LOG_DEBUG, "received accounting");

            if ((status = recvfrom(server.radius_acct->fd, &radius_pack, sizeof(radius_pack), 0,
                                   (struct sockaddr *) &addr, &fromlen)) <= 0) {
              syslog(LOG_ERR, "%s: recvfrom() failed", strerror(errno));
              return -1;
            }

            memcpy(&server.acct_peer, &addr, sizeof(addr));

            process_radius(&radius_pack, status);
          }

          if (server.radius_cli) {
            if (FD_ISSET(server.radius_cli->fd, &fdread)) {
              radius_decaps(server.radius_cli, 0);
            }
          }

          if (server.conn.sock) {
            if (FD_ISSET(server.conn.sock, &fdread)) {
              process_radius_reply();
            }
          }
        }

        break;
    }
  }

  selfpipe_finish();

  return 0;
}
