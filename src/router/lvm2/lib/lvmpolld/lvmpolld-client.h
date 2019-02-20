/*
 * Copyright (C) 2014-2015 Red Hat, Inc.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _LVM_LVMPOLLD_CLIENT_H
#define _LVM_LVMPOLLD_CLIENT_H
#  ifdef LVMPOLLD_SUPPORT

#	include "libdaemon/client/daemon-client.h"

#	define LVMPOLLD_SOCKET DEFAULT_RUN_DIR "/lvmpolld.socket"

struct cmd_context;
struct poll_operation_id;
struct daemon_parms;

void lvmpolld_disconnect(void);

int lvmpolld_poll_init(const struct cmd_context *cmd, const struct poll_operation_id *id,
		       const struct daemon_parms *parms);

int lvmpolld_request_info(const struct poll_operation_id *id, const struct daemon_parms *parms,
			  unsigned *finished);

int lvmpolld_use(void);

void lvmpolld_set_active(int active);

void lvmpolld_set_socket(const char *socket);

#  else

#	define lvmpolld_disconnect() do {} while (0)
#	define lvmpolld_poll_init(cmd, id, parms) (0)
#	define lvmpolld_request_info(id, parms, finished) (0)
#	define lvmpolld_use() (0)
#	define lvmpolld_set_active(active) do {} while (0)
#	define lvmpolld_set_socket(socket) do {} while (0)

#  endif /* LVMPOLLD_SUPPORT */

#endif /* _LVM_LVMPOLLD_CLIENT_H */
