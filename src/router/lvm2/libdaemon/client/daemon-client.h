/*
 * Copyright (C) 2011-2012 Red Hat, Inc.
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

#ifndef _LVM_DAEMON_CLIENT_H
#define _LVM_DAEMON_CLIENT_H

#include "libdaemon/client/config-util.h"

typedef struct {
	int socket_fd; /* the fd we use to talk to the daemon */
	const char *protocol;
	int protocol_version;  /* version of the protocol the daemon uses */
	int error;
} daemon_handle;

typedef struct {
	const char *path; /* the binary of the daemon */
	const char *socket; /* path to the comms socket */
	unsigned autostart:1; /* start the daemon if not running? */

	/*
	 * If the following are not NULL/0, an attempt to talk to a daemon which
	 * uses a different protocol or version will fail.
	 */
	const char *protocol;
	int protocol_version;
} daemon_info;

typedef struct {
	struct buffer buffer;
	/*
	 * The request looks like this:
	 *    request = "id"
	 *    arg_foo = "something"
	 *    arg_bar = 3
	 *    arg_wibble {
	 *        something_special = "here"
	 *        amount = 75
	 *        knobs = [ "twiddle", "tweak" ]
	 *    }
	 */
	struct dm_config_tree *cft;
} daemon_request;

typedef struct {
	int error; /* 0 for success */
	struct buffer buffer;
	struct dm_config_tree *cft; /* parsed reply, if available */
} daemon_reply;

/*
 * Open the communication channel to the daemon. If the daemon is not running,
 * it may be autostarted based on the binary path provided in the info (this
 * will only happen if autostart is set to true). If the call fails for any
 * reason, daemon_handle_valid(h) for the response will return false. Otherwise,
 * the connection is good to start serving requests.
 */
daemon_handle daemon_open(daemon_info i);

/*
 * Send a request to the daemon, waiting for the reply. All communication with
 * the daemon is synchronous. The function handles the IO details and parses the
 * response, handling common error conditions. See "daemon_reply" for details.
 *
 * In case the request contains a non-NULL buffer pointer, this buffer is sent
 * *verbatim* to the server. In this case, the cft pointer may be NULL (but will
 * be ignored even if non-NULL). If the buffer is NULL, the cft is required to
 * be a valid pointer, and is used to build up the request.
 */
daemon_reply daemon_send(daemon_handle h, daemon_request rq);

/*
 * A simple interface to daemon_send. This function just takes the command id
 * and possibly a list of parameters (of the form "name = %?", "value"). The
 * type (string, integer) of the value is indicated by a character substituted
 * for ? in %?: d for integer, s for string.
 */
daemon_reply daemon_send_simple(daemon_handle h, const char *id, ...);
daemon_reply daemon_send_simple_v(daemon_handle h, const char *id, va_list ap);

daemon_request daemon_request_make(const char *id);
int daemon_request_extend(daemon_request r, ...);
int daemon_request_extend_v(daemon_request r, va_list ap);
void daemon_request_destroy(daemon_request r);

void daemon_reply_destroy(daemon_reply r);

static inline int64_t daemon_reply_int(daemon_reply r, const char *path, int64_t def)
{
	return dm_config_find_int64(r.cft->root, path, def);
}

static inline const char *daemon_reply_str(daemon_reply r, const char *path, const char *def)
{
	return dm_config_find_str_allow_empty(r.cft->root, path, def);
}

/* Shut down the communication to the daemon. Compulsory. */
void daemon_close(daemon_handle h);

#endif
