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

#ifndef _LVM_DAEMON_SERVER_H
#define _LVM_DAEMON_SERVER_H

#include "libdaemon/client/daemon-client.h"

typedef struct {
	int socket_fd; /* the fd we use to talk to the client */
	pthread_t thread_id;
	char *read_buf;
	void *private; /* this holds per-client state */
} client_handle;

typedef struct {
	struct dm_config_tree *cft;
	struct buffer buffer;
} request;

typedef struct {
	int error;
	struct dm_config_tree *cft;
	struct buffer buffer;
} response;

struct timeval;

/*
 * is_idle:	 daemon implementation sets it to true when no background task
 *		 is running
 * max_timeouts: how many seconds do daemon allow to be idle before it shutdowns
 * ptimeout:	 internal variable passed to select(). has to be reset to 1 second
 *		 before each select
 */
typedef struct {
	volatile unsigned is_idle;
	unsigned max_timeouts;
	struct timeval *ptimeout;
} daemon_idle;

struct daemon_state;

/*
 * Craft a simple reply, without the need to construct a config_tree. See
 * daemon_send_simple in daemon-client.h for the description of the parameters.
 */
response daemon_reply_simple(const char *id, ...);

static inline int daemon_request_int(request r, const char *path, int def) {
	if (!r.cft)
		return def;
	return dm_config_find_int(r.cft->root, path, def);
}

static inline const char *daemon_request_str(request r, const char *path, const char *def) {
	if (!r.cft)
		return def;
	return dm_config_find_str(r.cft->root, path, def);
}

/*
 * The callback. Called once per request issued, in the respective client's
 * thread. It is presented by a parsed request (in the form of a config tree).
 * The output is a new config tree that is serialised and sent back to the
 * client. The client blocks until the request processing is done and reply is
 * sent.
 */
typedef response (*handle_request)(struct daemon_state s, client_handle h, request r);

typedef struct {
	uint32_t log_config[32];
	void *backend_state[32];
	const char *name;
} log_state;

struct thread_state;

typedef struct daemon_state {
	/*
	 * The maximal stack size for individual daemon threads. This is
	 * essential for daemons that need to be locked into memory, since
	 * pthread's default is 10M per thread.
	 */
	int thread_stack_size;

	/* Flags & attributes affecting the behaviour of the daemon. */
	unsigned avoid_oom:1;
	unsigned foreground:1;
	const char *name;
	const char *pidfile;
	const char *socket_path;
	const char *protocol;
	int protocol_version;

	handle_request handler;
	int (*daemon_init)(struct daemon_state *st);
	int (*daemon_fini)(struct daemon_state *st);
	int (*daemon_main)(struct daemon_state *st);

	/* Global runtime info maintained by the framework. */
	int socket_fd;

	log_state *log;
	struct thread_state *threads;

	/* suport for shutdown on idle */
	daemon_idle *idle;

	void *private; /* the global daemon state */
} daemon_state;

typedef struct thread_state {
	daemon_state s;
	client_handle client;
	struct thread_state *next;
	volatile int active;
} thread_state;

/*
 * Start serving the requests. This does all the daemonisation, socket setup
 * work and so on. This function takes over the process, and upon failure, it
 * will terminate execution. It may be called at most once.
 */
void daemon_start(daemon_state s);

/*
 * Take over from an already running daemon. This function handles connecting
 * to the running daemon and telling it we are going to take over. The takeover
 * request may be customised by passing in a non-NULL request.
 *
 * The takeover sequence: the old daemon stops accepting new clients, then it
 * waits until all current client connections are closed. When that happens, it
 * serializes its current state and sends that as a reply, which is then
 * returned by this function (therefore, this function won't return until the
 * previous instance has shut down).
 *
 * The daemon, after calling daemon_takeover is expected to set up its
 * daemon_state using the reply from this function and call daemon_start as
 * usual.
 */
daemon_reply daemon_takeover(daemon_info i, daemon_request r);

/* Call this to request a clean shutdown of the daemon. Async safe. */
void daemon_stop(void);

enum { DAEMON_LOG_OUTLET_SYSLOG = 1,
       DAEMON_LOG_OUTLET_STDERR = 2,
       DAEMON_LOG_OUTLET_SOCKET = 4 };

/* Log a message of a given type. */
void daemon_log(log_state *s, int type, const char *message);

/* Log a config (sub)tree, using a given message type, each line prefixed with "prefix". */
void daemon_log_cft(log_state *s, int type, const char *prefix,
                    const struct dm_config_node *n);

/* Log a multi-line block, prefixing each line with "prefix". */
void daemon_log_multi(log_state *s, int type, const char *prefix, const char *message);

/* Log a formatted message as "type". See also daemon-log.h. */
void daemon_logf(log_state *s, int type, const char *format, ...)
	__attribute__ ((format(printf, 3, 4)));

/*
 * Configure log_state to send messages of type "type" to the log outlet
 * "outlet", iff "enable" is true.
 */
void daemon_log_enable(log_state *s, int outlet, int type, int enable);

/*
 * Set up logging on a given outlet using a list of message types (comma
 * separated) to log using that outlet. The list is expected to look like this,
 * "all,wire,debug". Returns 0 upon encountering an unknown message type.
 */
int daemon_log_parse(log_state *s, int outlet, const char *types, int enable);

#endif
