/*
 * Copyright (C) 2005-2007 Red Hat, Inc. All rights reserved.
 *
 * This file is part of the device-mapper userspace tools.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef __DMEVENTD_DOT_H__
#define __DMEVENTD_DOT_H__

/* FIXME This stuff must be configurable. */

#define	DM_EVENT_FIFO_CLIENT	DEFAULT_DM_RUN_DIR "/dmeventd-client"
#define	DM_EVENT_FIFO_SERVER	DEFAULT_DM_RUN_DIR "/dmeventd-server"

#define DM_EVENT_DEFAULT_TIMEOUT 10

/* Commands for the daemon passed in the message below. */
enum dm_event_command {
	DM_EVENT_CMD_ACTIVE = 1,
	DM_EVENT_CMD_REGISTER_FOR_EVENT,
	DM_EVENT_CMD_UNREGISTER_FOR_EVENT,
	DM_EVENT_CMD_GET_REGISTERED_DEVICE,
	DM_EVENT_CMD_GET_NEXT_REGISTERED_DEVICE,
	DM_EVENT_CMD_SET_TIMEOUT,
	DM_EVENT_CMD_GET_TIMEOUT,
	DM_EVENT_CMD_HELLO,
	DM_EVENT_CMD_DIE,
	DM_EVENT_CMD_GET_STATUS,
	DM_EVENT_CMD_GET_PARAMETERS,
};

/* Message passed between client and daemon. */
struct dm_event_daemon_message {
	uint32_t cmd;
	uint32_t size;
	char *data;
};

/* FIXME Is this meant to be exported?  I can't see where the
   interface uses it. */
/* Fifos for client/daemon communication. */
struct dm_event_fifos {
	int client;
	int server;
	const char *client_path;
	const char *server_path;
};

/*      EXIT_SUCCESS             0 -- stdlib.h */
/*      EXIT_FAILURE             1 -- stdlib.h */
/*      EXIT_LOCKFILE_INUSE      2 -- obsoleted */
#define EXIT_DESC_CLOSE_FAILURE  3
#define EXIT_DESC_OPEN_FAILURE   4
/*      EXIT_OPEN_PID_FAILURE    5 -- obsoleted */
#define EXIT_FIFO_FAILURE        6
#define EXIT_CHDIR_FAILURE       7

/* Implemented in libdevmapper-event.c, but not part of public API. */
// FIXME  misuse of bitmask as enum
int daemon_talk(struct dm_event_fifos *fifos,
		struct dm_event_daemon_message *msg, int cmd,
		const char *dso_name, const char *dev_name,
		enum dm_event_mask evmask, uint32_t timeout);
int init_fifos(struct dm_event_fifos *fifos);
void fini_fifos(struct dm_event_fifos *fifos);
int dm_event_get_version(struct dm_event_fifos *fifos, int *version);

#endif /* __DMEVENTD_DOT_H__ */
