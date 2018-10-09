/*
 * Copyright (C) 2008 Intel Corporation
 *
 *	mdmon socket / message handling
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "mdadm.h"
#include "mdmon.h"

static const __u32 start_magic = 0x5a5aa5a5;
static const __u32 end_magic = 0xa5a55a5a;

static int send_buf(int fd, const void* buf, int len, int tmo)
{
	fd_set set;
	int rv;
	struct timeval timeout = {tmo, 0};
	struct timeval *ptmo = tmo ? &timeout : NULL;

	while (len) {
		FD_ZERO(&set);
		FD_SET(fd, &set);
		rv = select(fd+1, NULL, &set, NULL, ptmo);
		if (rv <= 0)
			return -1;
		rv = write(fd, buf, len);
		if (rv <= 0)
			return -1;
		len -= rv;
		buf += rv;
	}
	return 0;
}

static int recv_buf(int fd, void* buf, int len, int tmo)
{
	fd_set set;
	int rv;
	struct timeval timeout = {tmo, 0};
	struct timeval *ptmo = tmo ? &timeout : NULL;

	while (len) {
		FD_ZERO(&set);
		FD_SET(fd, &set);
		rv = select(fd+1, &set, NULL, NULL, ptmo);
		if (rv <= 0)
			return -1;
		rv = read(fd, buf, len);
		if (rv <= 0)
			return -1;
		len -= rv;
		buf += rv;
	}
	return 0;
}

int send_message(int fd, struct metadata_update *msg, int tmo)
{
	__s32 len = msg->len;
	int rv;

	rv = send_buf(fd, &start_magic, 4, tmo);
	rv = rv ?: send_buf(fd, &len, 4, tmo);
	if (len > 0)
		rv = rv ?: send_buf(fd, msg->buf, msg->len, tmo);
	rv = send_buf(fd, &end_magic, 4, tmo);

	return rv;
}

int receive_message(int fd, struct metadata_update *msg, int tmo)
{
	__u32 magic;
	__s32 len;
	int rv;

	rv = recv_buf(fd, &magic, 4, tmo);
	if (rv < 0 || magic != start_magic)
		return -1;
	rv = recv_buf(fd, &len, 4, tmo);
	if (rv < 0 || len > MSG_MAX_LEN)
		return -1;
	if (len > 0) {
		msg->buf = xmalloc(len);
		rv = recv_buf(fd, msg->buf, len, tmo);
		if (rv < 0) {
			free(msg->buf);
			return -1;
		}
	} else
		msg->buf = NULL;
	rv = recv_buf(fd, &magic, 4, tmo);
	if (rv < 0 || magic != end_magic) {
		free(msg->buf);
		return -1;
	}
	msg->len = len;
	return 0;
}

int ack(int fd, int tmo)
{
	struct metadata_update msg = { .len = 0 };

	return send_message(fd, &msg, tmo);
}

int wait_reply(int fd, int tmo)
{
	struct metadata_update msg;
	int err = receive_message(fd, &msg, tmo);

	/* mdmon sent extra data, but caller only cares that we got a
	 * successful reply
	 */
	if (err == 0 && msg.len > 0)
		free(msg.buf);

	return err;
}

int connect_monitor(char *devname)
{
	char path[100];
	int sfd;
	long fl;
	struct sockaddr_un addr;
	int pos;
	char *c;

	pos = sprintf(path, "%s/", MDMON_DIR);
	if (is_subarray(devname)) {
		devname++;
		c = strchr(devname, '/');
		if (!c)
			return -1;
		snprintf(&path[pos], c - devname + 1, "%s", devname);
		pos += c - devname;
	} else
		pos += sprintf(&path[pos], "%s", devname);
	sprintf(&path[pos], ".sock");

	sfd = socket(PF_LOCAL, SOCK_STREAM, 0);
	if (sfd < 0)
		return -1;

	addr.sun_family = PF_LOCAL;
	strcpy(addr.sun_path, path);
	if (connect(sfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		close(sfd);
		return -1;
	}

	fl = fcntl(sfd, F_GETFL, 0);
	fl |= O_NONBLOCK;
	fcntl(sfd, F_SETFL, fl);

	return sfd;
}

int fping_monitor(int sfd)
{
	int err = 0;

	if (sfd < 0)
		return sfd;

	/* try to ping existing socket */
	if (ack(sfd, 20) != 0)
		err = -1;

	/* check the reply */
	if (!err && wait_reply(sfd, 20) != 0)
		err = -1;

	return err;
}

/* give the monitor a chance to update the metadata */
int ping_monitor(char *devname)
{
	int sfd = connect_monitor(devname);
	int err;

	if (sfd >= 0) {
		err = fping_monitor(sfd);
		close(sfd);
	} else
		err = -1;

	return err;
}

static char *ping_monitor_version(char *devname)
{
	int sfd = connect_monitor(devname);
	struct metadata_update msg;
	int err = 0;

	if (sfd < 0)
		return NULL;

	if (ack(sfd, 20) != 0)
		err = -1;

	if (!err && receive_message(sfd, &msg, 20) != 0)
		err = -1;

	close(sfd);

	if (err || !msg.len || !msg.buf)
		return NULL;
	return msg.buf;
}

int unblock_subarray(struct mdinfo *sra, const int unfreeze)
{
	char buf[64];
	int rc = 0;

	if (sra) {
		sprintf(buf, "external:%s\n", sra->text_version);
		buf[9] = '/';
	} else
		buf[9] = '-';

	if (buf[9] == '-' ||
	    sysfs_set_str(sra, NULL, "metadata_version", buf) ||
	    (unfreeze &&
	     sysfs_attribute_available(sra, NULL, "sync_action") &&
	     sysfs_set_str(sra, NULL, "sync_action", "idle")))
		rc = -1;
	return rc;
}

int block_subarray(struct mdinfo *sra)
{
	char buf[64];
	int rc = 0;

	sprintf(buf, "external:%s\n", sra->text_version);
	buf[9] = '-';
	if (sysfs_set_str(sra, NULL, "metadata_version", buf))
		rc = -1;

	return rc;
}

/* check mdmon version if it supports
 * array blocking mechanism
 */
int check_mdmon_version(char *container)
{
	char *version = NULL;

	if (!mdmon_running(container)) {
		/* if mdmon is not active we assume that any instance that is
		 * later started will match the current mdadm version, if this
		 * assumption is violated we may inadvertantly rebuild an array
		 * that was meant for reshape, or start rebuild on a spare that
		 * was to be moved to another container
		 */
		/* pass */;
	} else {
		int ver;

		version = ping_monitor_version(container);
		ver = version ? mdadm_version(version) : -1;
		free(version);
		if (ver < 3002000) {
			pr_err("mdmon instance for %s cannot be disabled\n",
			       container);
			return -1;
		}
	}

	return 0;
}

/**
 * block_monitor - prevent mdmon spare assignment
 * @container - container to block
 * @freeze - flag to additionally freeze sync_action
 *
 * This is used by the reshape code to freeze the container, and the
 * auto-rebuild implementation to atomically move spares.
 * In both cases we need to stop mdmon from assigning spares to replace
 * failed devices as we might have other plans for the spare.
 * For the reshape case we also need to 'freeze' sync_action so that
 * no recovery happens until we have fully prepared for the reshape.
 *
 * We tell mdmon that the array is frozen by marking the 'metadata' name
 * with a leading '-'.  The previously told mdmon "Don't make this array
 * read/write, leave it readonly".  Now it means a more general "Don't
 * reconfigure this array at all".
 * As older versions of mdmon (which might run from initrd) don't understand
 * this, we first check that the running mdmon is new enough.
 */
int block_monitor(char *container, const int freeze)
{
	struct mdstat_ent *ent, *e, *e2;
	struct mdinfo *sra = NULL;
	char buf[64];
	int rv = 0;

	if (check_mdmon_version(container))
		return -1;

	ent = mdstat_read(0, 0);
	if (!ent) {
		pr_err("failed to read /proc/mdstat while disabling mdmon\n");
		return -1;
	}

	/* freeze container contents */
	for (e = ent; e; e = e->next) {
		if (!is_container_member(e, container))
			continue;
		sysfs_free(sra);
		sra = sysfs_read(-1, e->devnm, GET_VERSION);
		if (!sra) {
			pr_err("failed to read sysfs for subarray%s\n",
			       to_subarray(e, container));
			break;
		}
		/* can't reshape an array that we can't monitor */
		if (sra->text_version[0] == '-')
			break;

		if (freeze && sysfs_freeze_array(sra) < 1)
			break;
		/* flag this array to not be modified by mdmon (close race with
		 * takeover in reshape case and spare reassignment in the
		 * auto-rebuild case)
		 */
		if (block_subarray(sra))
			break;
		ping_monitor(container);

		/* check that we did not race with recovery */
		if ((freeze &&
		     !sysfs_attribute_available(sra, NULL, "sync_action")) ||
		    (freeze &&
		     sysfs_attribute_available(sra, NULL, "sync_action") &&
		     sysfs_get_str(sra, NULL, "sync_action", buf, 20) > 0 &&
		     strcmp(buf, "frozen\n") == 0))
			/* pass */;
		else {
			unblock_subarray(sra, 0);
			break;
		}
		/* Double check against races - there should be no spares
		 * or part-spares
		 */
		sysfs_free(sra);
		sra = sysfs_read(-1, e->devnm, GET_DEVS | GET_STATE);
		if (sra && sra->array.spare_disks > 0) {
			unblock_subarray(sra, freeze);
			break;
		}
	}

	if (e) {
		pr_err("failed to freeze subarray%s\n",
			to_subarray(e, container));

		/* thaw the partially frozen container */
		for (e2 = ent; e2 && e2 != e; e2 = e2->next) {
			if (!is_container_member(e2, container))
				continue;
			sysfs_free(sra);
			sra = sysfs_read(-1, e2->devnm, GET_VERSION);
			if (unblock_subarray(sra, freeze))
				pr_err("Failed to unfreeze %s\n", e2->devnm);
		}

		ping_monitor(container); /* cleared frozen */
		rv = -1;
	}

	sysfs_free(sra);
	free_mdstat(ent);

	return rv;
}

void unblock_monitor(char *container, const int unfreeze)
{
	struct mdstat_ent *ent, *e;
	struct mdinfo *sra = NULL;
	int to_ping = 0;

	ent = mdstat_read(0, 0);
	if (!ent) {
		pr_err("failed to read /proc/mdstat while unblocking container\n");
		return;
	}

	/* unfreeze container contents */
	for (e = ent; e; e = e->next) {
		if (!is_container_member(e, container))
			continue;
		sysfs_free(sra);
		sra = sysfs_read(-1, e->devnm, GET_VERSION|GET_LEVEL);
		if (!sra)
			continue;
		if (sra->array.level > 0)
			to_ping++;
		if (unblock_subarray(sra, unfreeze))
			pr_err("Failed to unfreeze %s\n", e->devnm);
	}
	if (to_ping)
		ping_monitor(container);

	sysfs_free(sra);
	free_mdstat(ent);
}

/* give the manager a chance to view the updated container state.  This
 * would naturally happen due to the manager noticing a change in
 * /proc/mdstat; however, pinging encourages this detection to happen
 * while an exclusive open() on the container is active
 */
int ping_manager(char *devname)
{
	int sfd = connect_monitor(devname);
	struct metadata_update msg = { .len = -1 };
	int err = 0;

	if (sfd < 0)
		return sfd;

	err = send_message(sfd, &msg, 20);

	/* check the reply */
	if (!err && wait_reply(sfd, 20) != 0)
		err = -1;

	close(sfd);
	return err;
}

/* using takeover operation for grow purposes, mdadm has to be sure
 * that mdmon processes all updates, and if necessary it will be closed
 * at takeover to raid0 operation
  */
void flush_mdmon(char *container)
{
	ping_manager(container);
	ping_monitor(container);
}
