/* generic service monitor backend
 *
 * Copyright (C) 2019-2021  Joachim Wiberg <troglobit@gmail.com>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <errno.h>
#include <unistd.h>

#include "log.h"
#include "notify.h"
#include "util.h"

void notify_ready(char *pidfn, uid_t uid, gid_t gid)
{
	const char *msg = "Ready, waiting for client request or kernel event.";

	if (pidfile_create(pidfn, uid, gid))
		smclog(LOG_WARNING, "Failed create/chown PID file: %s", strerror(errno));

	systemd_notify_ready(msg);
	smclog(LOG_NOTICE, msg);
}

void notify_reload(void)
{
	const char *msg = "Reloading configuration, please wait ...";

	systemd_notify_reload(msg);
	smclog(LOG_NOTICE, msg);
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
