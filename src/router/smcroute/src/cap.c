/* Daemon capability API
 *
 * Copyright (C) 2016       Markus Palonen <markus.palonen@gmail.com>
 * Copyright (C) 2016-2020  Joachim Wiberg <troglobit@gmail.com>
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
#include "config.h"

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef HAVE_SYS_PRCTL_H
# include <sys/prctl.h>
#endif
#include <sys/capability.h>
#include <pwd.h>
#include <grp.h>

#include "log.h"

static const char *username = NULL;


static int whoami(const char *user, const char *group, uid_t *uid, gid_t *gid)
{
	struct passwd *pw;
	struct group  *gr;

	if (!user)
		return -1;

	/* Get target UID and target GID */
	pw = getpwnam(user);
	if (!pw) {
		smclog(LOG_ERR, "User '%s' not found!", user);
		return -1;
	}

	*uid = pw->pw_uid;
	*gid = pw->pw_gid;
	if (group) {
		gr = getgrnam(group);
		if (!gr) {
			smclog(LOG_ERR, "Group '%s' not found!", group);
			return -1;
		}
		*gid = gr->gr_gid;
	}

	/* Valid user */
	username = user;

	return 0;
}

static int setcaps(cap_value_t cv)
{
	int result;
	cap_t caps = cap_get_proc();
	cap_value_t cap_list = cv;

	cap_clear(caps);

	cap_set_flag(caps, CAP_PERMITTED, 1, &cap_list, CAP_SET);
	cap_set_flag(caps, CAP_EFFECTIVE, 1, &cap_list, CAP_SET);
	result = cap_set_proc(caps);

	cap_free(caps);

	return result;
}

/*
 * Drop root privileges except capability CAP_NET_ADMIN. This capability
 * enables the thread (among other networking related things) to add and
 * remove multicast routes
 */
static int drop_root(const char *user, uid_t uid, gid_t gid)
{
#ifdef HAVE_SYS_PRCTL_H
	/* Allow this process to preserve permitted capabilities */
	if (prctl(PR_SET_KEEPCAPS, 1) == -1) {
		smclog(LOG_ERR, "Cannot preserve capabilities: %s", strerror(errno));
		return -1;
	}
#endif
	/* Set supplementary groups, GID and UID */
	if (initgroups(user, gid) == -1) {
		smclog(LOG_ERR, "Failed setting supplementary groups: %s", strerror(errno));
		return -1;
	}

	if (setgid(gid) == -1) {
		smclog(LOG_ERR, "Failed setting group ID %d: %s", gid, strerror(errno));
		return -1;
	}

	if (setuid(uid) == -1) {
		smclog(LOG_ERR, "Failed setting user ID %d: %s", uid, strerror(errno));
		return -1;
	}

	/* Clear all capabilities except CAP_NET_ADMIN */
	if (setcaps(CAP_NET_ADMIN)) {
		smclog(LOG_ERR, "Failed setting `CAP_NET_ADMIN`: %s", strerror(errno));
		return -1;
	}

	/* Try to regain root UID, should not work at this point. */
	if (setuid(0) == 0)
		return -1;

	return 0;
}

void cap_drop_root(uid_t uid, gid_t gid)
{
	if (username) {
		if (drop_root(username, uid, gid) == -1)
			smclog(LOG_WARNING, "Could not drop root privileges, continuing as root.");
		else
			smclog(LOG_INFO, "Root privileges dropped: Current UID %u, GID %u.", getuid(), getgid());
	}
}

void cap_set_user(char *arg, uid_t *uid, gid_t *gid)
{
	char *ptr;
	char *user;
	char *group;

	ptr = strdup(arg);
	if (!ptr)
		err(1, "Failed parsing user:group argument");

	user = strtok(ptr, ":");
	group = strtok(NULL, ":");

	if (whoami(user, group, uid, gid))
		err(1, "Invalid user:group argument");

	free(ptr);
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
