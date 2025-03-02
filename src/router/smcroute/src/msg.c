/* IPC command parser and builder for daemon and client
 *
 * Copyright (C) 2011-2021  Joachim Wiberg <troglobit@gmail.com>
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

#include <errno.h>
#include <signal.h>		/* sig_atomic_t */
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "conf.h"
#include "log.h"
#include "msg.h"
#include "iface.h"
#include "util.h"
#include "mroute.h"
#include "mcgroup.h"

extern volatile sig_atomic_t running;
extern volatile sig_atomic_t reloading;


/*
 * Check for prefix length, only applicable for (*,G) routes
 */
int is_range(char *arg)
{
	char *ptr;

	ptr = strchr(arg, '/');
	if (ptr) {
		*ptr++ = 0;
		return atoi(ptr);
	}

	return 0;
}

static int do_mgroup(struct ipc_msg *msg)
{
	char source[INET_ADDRSTR_LEN + 5] = { 0 };
	char group[INET_ADDRSTR_LEN + 5] = { 0 };
	char *ifname = msg->argv[0];

	if (msg->count < 2) {
		errno = EINVAL;
		return -1;
	}
	if (msg->count == 3) {
		strlcpy(source, msg->argv[1], sizeof(source));
		strlcpy(group, msg->argv[2], sizeof(group));
	} else
		strlcpy(group, msg->argv[1], sizeof(group));

	return conf_mgroup(NULL, msg->cmd == 'j' ? 1 : 0, ifname, source[0] ? source : NULL, group);
}

static int do_mroute(struct ipc_msg *msg)
{
	char src[INET_ADDRSTR_LEN + 5];
	char *ifname, *source, *group;
	char *out[MAX_MC_VIFS];
	inet_addr_t ss;
	int num = 0;
	int pos = 0;

	if (msg->count < 2) {
		errno = EINVAL;
		return -1;
	}

	ifname = msg->argv[pos++];

	strlcpy(src, msg->argv[pos++], sizeof(src));
	is_range(src);
	if (inet_str2addr(src, &ss)) {
		smclog(LOG_ERR, "mroute: invalid source/group address: %s", src);
		return 1;
	}

	if (!is_multicast(&ss)) {
		char grp[INET_ADDRSTR_LEN + 5];

		strlcpy(grp, msg->argv[pos++], sizeof(grp));
		is_range(grp);
		if (inet_str2addr(grp, &ss) || !is_multicast(&ss)) {
			smclog(LOG_DEBUG, "mroute: invalid multicast group: %s", grp);
			return 1;
		}

		source = msg->argv[1];
		group  = msg->argv[2];
	} else {
		source = NULL;
		group  = msg->argv[1];
	}

	while (pos < msg->count)
		out[num++] = msg->argv[pos++];

	return conf_mroute(NULL, msg->cmd == 'a' ? 1 : 0, ifname, source, group, out, num);
}

static int do_show(struct ipc_msg *msg, int sd, int detail)
{
	if (msg->count > 0) {
		char cmd = msg->argv[0][0];

		switch (cmd) {
		case 'g':
			return mcgroup_show(sd, detail);

		case 'i':
			return iface_show(sd, detail);

		default:
			break;
		}
	}

	return mroute_show(sd, detail);
}

/*
 * Convert IPC command from client to a mulicast route or group join/leave
 */
int msg_do(int sd, struct ipc_msg *msg)
{
	int result = 0;

	switch (msg->cmd) {
	case 'a':
	case 'r':
		result = do_mroute(msg);
		break;

	case 'j':
	case 'l':
		result = do_mgroup(msg);
		break;

	case 'F':
		mroute_expire(0);
		break;

	case 'H':		/* HUP */
		reloading = 1;
		break;

	case 'k':
		running = 0;
		break;

	case 'S':
		result = do_show(msg, sd, 1);
		break;

	case 's':
		result = do_show(msg, sd, 0);
		break;

	default:
		errno = EINVAL;
		result = -1;
	}

	return result;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
