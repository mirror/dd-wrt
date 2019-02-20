/*
 * Copyright (C) 2014-2015 Red Hat, Inc.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 */

#ifndef _LVM_LVMLOCKD_CLIENT_H
#define _LVM_LVMLOCKD_CLIENT_H

#include "libdaemon/client/daemon-client.h"

#define LVMLOCKD_SOCKET DEFAULT_RUN_DIR "/lvmlockd.socket"

/* Wrappers to open/close connection */

static inline daemon_handle lvmlockd_open(const char *sock)
{
	daemon_info lvmlockd_info = {
		.path = "lvmlockd",
		.socket = sock ?: LVMLOCKD_SOCKET,
		.protocol = "lvmlockd",
		.protocol_version = 1,
		.autostart = 0
	};

	return daemon_open(lvmlockd_info);
}

static inline void lvmlockd_close(daemon_handle h)
{
	return daemon_close(h);
}

/*
 * Errors returned as the lvmlockd result value.
 */
#define ENOLS     210 /* lockspace not found */
#define ESTARTING 211 /* lockspace is starting */
#define EARGS     212
#define EHOSTID   213
#define EMANAGER  214
#define EPREPARE  215
#define ELOCKD    216
#define EVGKILLED 217 /* sanlock lost access to leases and VG is killed. */
#define ELOCKIO   218 /* sanlock io errors during lock op, may be transient. */
#define EREMOVED  219
#define EDEVOPEN  220 /* sanlock failed to open lvmlock LV */
#define ELMERR    221

#endif	/* _LVM_LVMLOCKD_CLIENT_H */
