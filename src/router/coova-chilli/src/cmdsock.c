/* -*- mode: c; c-basic-offset: 2 -*- */
/*
 * Copyright (C) 2007-2012 David Bird (Coova Technologies) <support@coova.com>
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
 */

#include "chilli.h"

int
cmdsock_init(void) {
  struct sockaddr_un local;
  int cmdsock;

  if ((cmdsock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {

    syslog(LOG_ERR, "%s: could not allocate UNIX Socket!", strerror(errno));

  } else {

    local.sun_family = AF_UNIX;

    strlcpy(local.sun_path, _options.cmdsocket, sizeof(local.sun_path));
    unlink(local.sun_path);

    if (bind(cmdsock, (struct sockaddr *)&local,
	     sizeof(struct sockaddr_un)) == -1) {
      syslog(LOG_ERR, "%s: could bind UNIX Socket!", strerror(errno));
      close(cmdsock);
      cmdsock = -1;
    } else {
      if (listen(cmdsock, 5) == -1) {
	syslog(LOG_ERR, "%s: could listen to UNIX Socket!", strerror(errno));
	close(cmdsock);
	cmdsock = -1;
      } else {
	if (_options.uid) {
	  if (chown(_options.cmdsocket, _options.uid, _options.gid)) {
	    syslog(LOG_ERR, "%d could not chown() %s",
                   errno, _options.cmdsocket);
	  }
	}
      }
    }
  }

  return cmdsock;
}


int
cmdsock_port_init(void) {
  struct sockaddr_in local;
  int cmdsock;
  int rc;

  if ((cmdsock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {

    syslog(LOG_ERR, "%s: could not allocate commands socket!", strerror(errno));

  } else {

    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons(_options.cmdsocketport);

    rc = 1;
    setsockopt(cmdsock,
	       SOL_SOCKET,
	       SO_REUSEADDR,
	       (char *)&rc,
	       sizeof(rc));

    if (bind(cmdsock, (struct sockaddr *)&local,
	     sizeof(struct sockaddr_in)) == -1) {
      syslog(LOG_ERR, "%s: could not bind commands socket!", strerror(errno));
      close(cmdsock);
      cmdsock = -1;
    } else {
      if (listen(cmdsock, 5) == -1) {
	syslog(LOG_ERR, "%s: could not listen from commands socket!", strerror(errno));
	close(cmdsock);
	cmdsock = -1;
      }
    }
  }

  return cmdsock;
}


void cmdsock_shutdown(int s) {
  if (s < 0) {
    return;
  }
  syslog(LOG_DEBUG, "%s(%d): Shutting down cmdsocket", __FUNCTION__, __LINE__);
  shutdown(s, 2);
  close(s);
}
