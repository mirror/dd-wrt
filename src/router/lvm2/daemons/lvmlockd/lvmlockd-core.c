/*
 * Copyright (C) 2014-2015 Red Hat, Inc.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 */

#define _XOPEN_SOURCE 500  /* pthread */
#define _ISOC99_SOURCE

#include "tools/tool.h"

#include "libdaemon/client/daemon-io.h"
#include "daemon-server.h"
#include "lvm-version.h"
#include "daemons/lvmlockd/lvmlockd-client.h"
#include "device_mapper/misc/dm-ioctl.h"

/* #include <assert.h> */
#include <errno.h>
#include <pthread.h>
#include <stddef.h>
#include <poll.h>
#include <signal.h>
#include <getopt.h>
#include <syslog.h>
#include <dirent.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <sys/un.h>

#ifdef USE_SD_NOTIFY
#include <systemd/sd-daemon.h>
#endif

#define EXTERN
#include "lvmlockd-internal.h"

/*
 * Basic operation of lvmlockd
 *
 * lvmlockd main process runs main_loop() which uses poll().
 * poll listens for new connections from lvm commands and for
 * messages from existing connected lvm commands.
 *
 * lvm command starts and connects to lvmlockd.
 *
 * lvmlockd receives a connection request from command and adds a
 * 'struct client' to keep track of the connection to the command.
 * The client's fd is added to the set of fd's in poll().
 *
 * lvm command sends a lock request to lvmlockd.  The lock request
 * can be for the global lock, a vg lock, or an lv lock.
 *
 * lvmlockd main_loop/poll sees a message from an existing client.
 * It sets client.recv = 1, then wakes up client_thread_main.
 *
 * client_thread_main iterates through client structs (cl), looking
 * for any that need processing, finds the one with cl->recv set,
 * and calls client_recv_action(cl).
 *
 * client_recv_action(cl) reads the message/request from the client,
 * allocates a new 'struct action' (act) to represent the request,
 * sets the act with what is found in the request, then looks at
 * the specific operation in act->op (LD_OP_FOO) to decide what to
 * do with the action:
 *
 * . If the action is to start a lockspace, create a new thread
 *   to manage that lockspace: add_lockspace(act).
 *
 * . If the action is a lock request, pass the act to the thread
 *   that is managing that lockspace: add_lock_action(act).
 *
 * . Other misc actions are are passed to the worker_thread:
 *   add_work_action(act).
 *
 * Onec the client_thread has passed the action off to another
 * thread to process, it goes back to waiting for more client
 * handling work to do.
 *
 * The thread that was given the action by the client_thread
 * now processes that action according to the operation, act->op.
 * This is either a lockspace_thread (for lock ops or ops that
 * add/rem a lockspace), or the worker_thread.  See below for
 * how these ops are processed by these threads.  When the
 * given thread is done processing the action, the result is
 * set in act->result, and the act struct for the completed action
 * is passed back to the client_thread (client_results list).
 *
 * The client_thread takes completed actions (from client_results
 * list), and sends the result back to the client that sent the
 * request represented by the action.  The act struct is then freed.
 *
 * This completes the cycle of work between lvm commands (clients)
 * and lvmlockd.  In summary:
 *
 * - main process polls for new client connections and new requests
 *   from lvm commands
 * - client_thread reads requests from clients
 * - client_thread creates an action struct for each request
 * - client_thread passes the act to another thread for processing
 * - other threads pass completed act structs back to client_thread
 * - client_thread sends the act result back to the client and frees the act
 *
 *
 * Lockspace threads:
 * Each lockd VG has its own lockspace that contains locks for that VG.
 * Each 'struct lockspace' is managed by a separate lockspace_thread.
 * When the lockspace_thread is first created, the first thing it does
 * is join the lockspace in the lock manager.  This can take a long time.
 * If the join fails, the thread exits.  After the join, the thread
 * enters a loop waiting for lock actions to perform in the lockspace.
 *
 * The request to remove/leave a lockspace causes a flag to be set in
 * the lockspace struct.  When the lockspace_thread sees this flag
 * set, it leaves the lockspace, and exits.
 *
 * When the client_thread passes a new action to a lockspace_thread,
 * i.e. a new lock request, the lockspace_thread identifies which resource
 * is being locked (GL, VG, LV), and gets the 'struct resource' (r) for it.
 * r->type will be LD_RT_GL, LD_RT_VG, or LD_RT_LV.  r->name is the
 * resource name, and is fixed for GL and VG resources, but is based on
 * the LV name for LV resources.  The act is added to the resource's
 * list of actions: r->actions, i.e. outstanding lock requests on the
 * resource.
 *
 * The lockspace thread then iterates through each resource in the
 * lockspace, processing any outstanding actions on each: res_process(ls, r).
 *
 * res_process() compares the outstanding actions/requests in r->actions
 * against any existing locks on the resource in r->locks.  If the
 * action is blocked by existing locks, it's left on r->actions.  If not,
 * the action/request is passed to the lock manager.  If the result from
 * the lock manager is success, a new 'struct lock' is created for the
 * action and saved on r->locks.  The result is set in act->result and
 * the act is passed back to the client_thread to be returned to the client.
 */

static const char *lvmlockd_protocol = "lvmlockd";
static const int lvmlockd_protocol_version = 1;
static int daemon_quit;
static int adopt_opt;

/*
 * We use a separate socket for dumping daemon info.
 * This will not interfere with normal operations, and allows
 * free-form debug data to be dumped instead of the libdaemon
 * protocol that wants all data in the cft format.
 * 1MB should fit all the info we need to dump.
 */
#define DUMP_SOCKET_NAME "lvmlockd-dump.sock"
#define DUMP_BUF_SIZE (1024 * 1024)
static char dump_buf[DUMP_BUF_SIZE];
static struct sockaddr_un dump_addr;
static socklen_t dump_addrlen;

/*
 * Main program polls client connections, adds new clients,
 * adds work for client thread.
 *
 * pollfd_mutex is used for adding vs removing entries,
 * and for resume vs realloc.
 */
#define POLL_FD_UNUSED -1		/* slot if free */
#define POLL_FD_IGNORE -2		/* slot is used but ignore in poll */
#define ADD_POLL_SIZE 16		/* increment slots by this amount */

static pthread_mutex_t pollfd_mutex;
static struct pollfd *pollfd;
static int pollfd_size;
static int pollfd_maxi;
static int listen_pi;
static int listen_fd;
static int restart_pi;
static int restart_fds[2];

/*
 * Each lockspace has its own thread to do locking.
 * The lockspace thread makes synchronous lock requests to dlm/sanlock.
 * Every vg with a lockd type, i.e. "dlm", "sanlock", should be on this list.
 */
static pthread_mutex_t lockspaces_mutex;
static struct list_head lockspaces;

/*
 * Client thread reads client requests and writes client results.
 */
static pthread_t client_thread;
static pthread_mutex_t client_mutex;
static pthread_cond_t client_cond;
static struct list_head client_list;    /* connected clients */
static struct list_head client_results; /* actions to send back to clients */
static uint32_t client_ids;             /* 0 and INTERNAL_CLIENT_ID are skipped */
static int client_stop;                 /* stop the thread */
static int client_work;                 /* a client on client_list has work to do */

#define INTERNAL_CLIENT_ID 0xFFFFFFFF   /* special client_id for internal actions */
static struct list_head adopt_results;  /* special start actions from adopt_locks() */

/*
 * Worker thread performs misc non-locking actions, e.g. init/free.
 */
static pthread_t worker_thread;
static pthread_mutex_t worker_mutex;
static pthread_cond_t worker_cond;
static struct list_head worker_list;    /* actions for worker_thread */
static int worker_stop;                 /* stop the thread */
static int worker_wake;                 /* wake the thread without adding work */

/*
 * The content of every log_foo() statement is saved in the
 * circular buffer, which can be dumped to a client and printed.
 */
#define LOG_LINE_SIZE 256
#define LOG_DUMP_SIZE DUMP_BUF_SIZE
#define LOG_SYSLOG_PRIO LOG_WARNING
static char log_dump[LOG_DUMP_SIZE];
static unsigned int log_point;
static unsigned int log_wrap;
static pthread_mutex_t log_mutex;
static int syslog_priority = LOG_SYSLOG_PRIO;

/*
 * Structure pools to avoid repeated malloc/free.
 */
#define MAX_UNUSED_ACTION 64
#define MAX_UNUSED_CLIENT 64
#define MAX_UNUSED_RESOURCE 64
#define MAX_UNUSED_LOCK 64
static pthread_mutex_t unused_struct_mutex;
static struct list_head unused_action;
static struct list_head unused_client;
static struct list_head unused_resource;
static struct list_head unused_lock;
static int unused_action_count;
static int unused_client_count;
static int unused_resource_count;
static int unused_lock_count;
static int resource_lm_data_size; /* max size of lm_data from sanlock|dlm */
static int alloc_new_structs; /* used for initializing in setup_structs */

#define DO_STOP 1
#define NO_STOP 0
#define DO_FREE 1
#define NO_FREE 0
#define DO_FORCE 1
#define NO_FORCE 0

static int add_lock_action(struct action *act);
static int str_to_lm(const char *str);
static int setup_dump_socket(void);
static void send_dump_buf(int fd, int dump_len);
static int dump_info(int *dump_len);
static int dump_log(int *dump_len);

static int _syslog_name_to_num(const char *name)
{
	if (!strcmp(name, "emerg"))
		return LOG_EMERG;
	if (!strcmp(name, "alert"))
		return LOG_ALERT;
	if (!strcmp(name, "crit"))
		return LOG_CRIT;
	if (!strcmp(name, "err") || !strcmp(name, "error"))
		return LOG_ERR;
	if (!strcmp(name, "warning") || !strcmp(name, "warn"))
		return LOG_WARNING;
	if (!strcmp(name, "notice"))
		return LOG_NOTICE;
	if (!strcmp(name, "info"))
		return LOG_INFO;
	if (!strcmp(name, "debug"))
		return LOG_DEBUG;
	return LOG_WARNING;
}

static const char *_syslog_num_to_name(int num)
{
	switch (num) {
	case LOG_EMERG:
		return "emerg";
	case LOG_ALERT:
		return "alert";
	case LOG_CRIT:
		return "crit";
	case LOG_ERR:
		return "err";
	case LOG_WARNING:
		return "warning";
	case LOG_NOTICE:
		return "notice";
	case LOG_INFO:
		return "info";
	case LOG_DEBUG:
		return "debug";
	}
	return "unknown";
}

static uint64_t monotime(void)
{
	struct timespec ts;

	if (clock_gettime(CLOCK_MONOTONIC, &ts)) {
		log_error("clock_gettime failed to get timestamp %s.",
			  strerror(errno));
		ts.tv_sec = 0;
	}

	return ts.tv_sec;
}

static void log_save_line(int len, char *line,
			  char *log_buf, unsigned int *point, unsigned int *wrap)
{
	unsigned int p = *point;
	unsigned int w = *wrap;
	int i;

	if (len < (int) (LOG_DUMP_SIZE - p)) {
		memcpy(log_buf + p, line, len);
		p += len;

		if (p == LOG_DUMP_SIZE) {
			p = 0;
			w = 1;
		}
		goto out;
	}

	for (i = 0; i < len; i++) {
		log_buf[p++] = line[i];

		if (p == LOG_DUMP_SIZE) {
			p = 0;
			w = 1;
		}
	}
 out:
	*point = p;
	*wrap = w;
}

void log_level(int level, const char *fmt, ...)
{
	char line[LOG_LINE_SIZE];
	va_list ap;
	int len = LOG_LINE_SIZE - 1;
	int ret, pos = 0;

	memset(line, 0, sizeof(line));

	ret = snprintf(line, len, "%llu ", (unsigned long long)time(NULL));
	pos += ret;

	va_start(ap, fmt);
	ret = vsnprintf(line + pos, len - pos, fmt, ap);
	va_end(ap);

	if (ret >= len - pos)
		pos = len - 1;
	else
		pos += ret;

	line[pos++] = '\n';
	line[pos++] = '\0';

	pthread_mutex_lock(&log_mutex);
	log_save_line(pos - 1, line, log_dump, &log_point, &log_wrap);
	pthread_mutex_unlock(&log_mutex);

	if (level <= syslog_priority)
		syslog(level, "%s", line);

	if (daemon_debug)
		fprintf(stderr, "%s", line);
}

static int dump_log(int *dump_len)
{
	int tail_len;

	pthread_mutex_lock(&log_mutex);

	if (!log_wrap && !log_point) {
		*dump_len = 0;
	} else if (log_wrap) {
		tail_len = LOG_DUMP_SIZE - log_point;
		memcpy(dump_buf, log_dump+log_point, tail_len);
		if (log_point)
			memcpy(dump_buf+tail_len, log_dump, log_point);
		*dump_len = LOG_DUMP_SIZE;
	} else {
		memcpy(dump_buf, log_dump, log_point-1);
		*dump_len = log_point-1;
	}
	pthread_mutex_unlock(&log_mutex);

	return 0;
}

struct lockspace *alloc_lockspace(void)
{
	struct lockspace *ls;

	if (!(ls = malloc(sizeof(struct lockspace)))) {
		log_error("out of memory for lockspace");
		return NULL;
	}

	memset(ls, 0, sizeof(struct lockspace));
	INIT_LIST_HEAD(&ls->actions);
	INIT_LIST_HEAD(&ls->resources);
	pthread_mutex_init(&ls->mutex, NULL);
	pthread_cond_init(&ls->cond, NULL);
	return ls;
}

static struct action *alloc_action(void)
{
	struct action *act;

	pthread_mutex_lock(&unused_struct_mutex);
	if (!unused_action_count || alloc_new_structs) {
		act = malloc(sizeof(struct action));
	} else {
		act = list_first_entry(&unused_action, struct action, list);
		list_del(&act->list);
		unused_action_count--;
	}
	pthread_mutex_unlock(&unused_struct_mutex);
	if (act)
		memset(act, 0, sizeof(struct action));
	else
		log_error("out of memory for action");
	return act;
}

static struct client *alloc_client(void)
{
	struct client *cl;

	pthread_mutex_lock(&unused_struct_mutex);
	if (!unused_client_count || alloc_new_structs) {
		cl = malloc(sizeof(struct client));
	} else {
		cl = list_first_entry(&unused_client, struct client, list);
		list_del(&cl->list);
		unused_client_count--;
	}
	pthread_mutex_unlock(&unused_struct_mutex);
	if (cl)
		memset(cl, 0, sizeof(struct client));
	else
		log_error("out of memory for client");
	return cl;
}

static struct resource *alloc_resource(void)
{
	struct resource *r;

	pthread_mutex_lock(&unused_struct_mutex);
	if (!unused_resource_count || alloc_new_structs) {
		r = malloc(sizeof(struct resource) + resource_lm_data_size);
	} else {
		r = list_first_entry(&unused_resource, struct resource, list);
		list_del(&r->list);
		unused_resource_count--;
	}
	pthread_mutex_unlock(&unused_struct_mutex);
	if (r) {
		memset(r, 0, sizeof(struct resource) + resource_lm_data_size);
		INIT_LIST_HEAD(&r->locks);
		INIT_LIST_HEAD(&r->actions);
	} else {
		log_error("out of memory for resource");
	}
	return r;
}

static struct lock *alloc_lock(void)
{
	struct lock *lk;

	pthread_mutex_lock(&unused_struct_mutex);
	if (!unused_lock_count || alloc_new_structs) {
		lk = malloc(sizeof(struct lock));
	} else {
		lk = list_first_entry(&unused_lock, struct lock, list);
		list_del(&lk->list);
		unused_lock_count--;
	}
	pthread_mutex_unlock(&unused_struct_mutex);
	if (lk)
		memset(lk, 0, sizeof(struct lock));
	else
		log_error("out of memory for lock");
	return lk;
}

static void free_action(struct action *act)
{
	pthread_mutex_lock(&unused_struct_mutex);
	if (unused_action_count >= MAX_UNUSED_ACTION) {
		free(act);
	} else {
		list_add_tail(&act->list, &unused_action);
		unused_action_count++;
	}
	pthread_mutex_unlock(&unused_struct_mutex);
}

static void free_client(struct client *cl)
{
	pthread_mutex_lock(&unused_struct_mutex);
	if (unused_client_count >= MAX_UNUSED_CLIENT) {
		free(cl);
	} else {
		list_add_tail(&cl->list, &unused_client);
		unused_client_count++;
	}
	pthread_mutex_unlock(&unused_struct_mutex);
}

static void free_resource(struct resource *r)
{
	pthread_mutex_lock(&unused_struct_mutex);
	if (unused_resource_count >= MAX_UNUSED_RESOURCE) {
		free(r);
	} else {
		list_add_tail(&r->list, &unused_resource);
		unused_resource_count++;
	}
	pthread_mutex_unlock(&unused_struct_mutex);
}

static void free_lock(struct lock *lk)
{
	pthread_mutex_lock(&unused_struct_mutex);
	if (unused_lock_count >= MAX_UNUSED_LOCK) {
		free(lk);
	} else {
		list_add_tail(&lk->list, &unused_lock);
		unused_lock_count++;
	}
	pthread_mutex_unlock(&unused_struct_mutex);
}

static int setup_structs(void)
{
	struct action *act;
	struct client *cl;
	struct resource *r;
	struct lock *lk;
	int data_san = lm_data_size_sanlock();
	int data_dlm = lm_data_size_dlm();
	int i;

	resource_lm_data_size = data_san > data_dlm ? data_san : data_dlm;

	pthread_mutex_init(&unused_struct_mutex, NULL);
	INIT_LIST_HEAD(&unused_action);
	INIT_LIST_HEAD(&unused_client);
	INIT_LIST_HEAD(&unused_resource);
	INIT_LIST_HEAD(&unused_lock);

	/*
	 * For setup, force the alloc_ functions to alloc new structs instead
	 * of taking them unused.  This allows alloc_struct/free_struct loop to
	 * populate the unused lists.
	 */
	alloc_new_structs = 1;

	for (i = 0; i < MAX_UNUSED_ACTION/2; i++) {
		if (!(act = alloc_action()))
			goto fail;
		free_action(act);
	}

	for (i = 0; i < MAX_UNUSED_CLIENT/2; i++) {
		if (!(cl = alloc_client()))
			goto fail;
		free_client(cl);
	}

	for (i = 0; i < MAX_UNUSED_RESOURCE/2; i++) {
		if (!(r = alloc_resource()))
			goto fail;
		free_resource(r);
	}

	for (i = 0; i < MAX_UNUSED_LOCK/2; i++) {
		if (!(lk = alloc_lock()))
			goto fail;
		free_lock(lk);
	}

	alloc_new_structs = 0;
	return 0;
fail:
	alloc_new_structs = 0;
	return -ENOMEM;
}

static int add_pollfd(int fd)
{
	int i, new_size;
	struct pollfd *tmp_pollfd;

	pthread_mutex_lock(&pollfd_mutex);
	for (i = 0; i < pollfd_size; i++) {
		if (pollfd[i].fd != POLL_FD_UNUSED)
			continue;

		pollfd[i].fd = fd;
		pollfd[i].events = POLLIN;
		pollfd[i].revents = 0;

		if (i > pollfd_maxi)
			pollfd_maxi = i;

		pthread_mutex_unlock(&pollfd_mutex);
		return i;
	}

	new_size = pollfd_size + ADD_POLL_SIZE;

	tmp_pollfd = realloc(pollfd, new_size * sizeof(struct pollfd));
	if (!tmp_pollfd) {
		log_error("can't alloc new size %d for pollfd", new_size);
		pthread_mutex_unlock(&pollfd_mutex);
		return -ENOMEM;
	}
	pollfd = tmp_pollfd;

	for (i = pollfd_size; i < new_size; i++) {
		pollfd[i].fd = POLL_FD_UNUSED;
		pollfd[i].events = 0;
		pollfd[i].revents = 0;
	}

	i = pollfd_size;
	pollfd[i].fd = fd;
	pollfd[i].events = POLLIN;
	pollfd[i].revents = 0;
	pollfd_maxi = i;

	pollfd_size = new_size;

	pthread_mutex_unlock(&pollfd_mutex);
	return i;
}

static void rem_pollfd(int pi)
{
	if (pi < 0) {
		log_error("rem_pollfd %d", pi);
		return;
	}
	pthread_mutex_lock(&pollfd_mutex);
	pollfd[pi].fd = POLL_FD_UNUSED;
	pollfd[pi].events = 0;
	pollfd[pi].revents = 0;
	pthread_mutex_unlock(&pollfd_mutex);
}

static const char *lm_str(int x)
{
	switch (x) {
	case LD_LM_NONE:
		return "none";
	case LD_LM_DLM:
		return "dlm";
	case LD_LM_SANLOCK:
		return "sanlock";
	default:
		return "lm_unknown";
	}
}

static const char *rt_str(int x)
{
	switch (x) {
	case LD_RT_GL:
		return "gl";
	case LD_RT_VG:
		return "vg";
	case LD_RT_LV:
		return "lv";
	default:
		return ".";
	};
}

static const char *op_str(int x)
{
	switch (x) {
	case LD_OP_INIT:
		return "init";
	case LD_OP_FREE:
		return "free";
	case LD_OP_START:
		return "start";
	case LD_OP_STOP:
		return "stop";
	case LD_OP_LOCK:
		return "lock";
	case LD_OP_UPDATE:
		return "update";
	case LD_OP_CLOSE:
		return "close";
	case LD_OP_ENABLE:
		return "enable";
	case LD_OP_DISABLE:
		return "disable";
	case LD_OP_START_WAIT:
		return "start_wait";
	case LD_OP_STOP_ALL:
		return "stop_all";
	case LD_OP_RENAME_BEFORE:
		return "rename_before";
	case LD_OP_RENAME_FINAL:
		return "rename_final";
	case LD_OP_RUNNING_LM:
		return "running_lm";
	case LD_OP_FIND_FREE_LOCK:
		return "find_free_lock";
	case LD_OP_KILL_VG:
		return "kill_vg";
	case LD_OP_DROP_VG:
		return "drop_vg";
	case LD_OP_DUMP_LOG:
		return "dump_log";
	case LD_OP_DUMP_INFO:
		return "dump_info";
	case LD_OP_BUSY:
		return "busy";
	default:
		return "op_unknown";
	};
}

int last_string_from_args(char *args_in, char *last)
{
	const char *args = args_in;
	const char *colon, *str = NULL;

	while (1) {
		if (!args || (*args == '\0'))
			break;
		colon = strstr(args, ":");
		if (!colon)
			break;
		str = colon;
		args = colon + 1;
        }

	if (str) {
		snprintf(last, MAX_ARGS, "%s", str + 1);
		return 0;
	}
	return -1;
}

int version_from_args(char *args, unsigned int *major, unsigned int *minor, unsigned int *patch)
{
	char version[MAX_ARGS+1];
	char *major_str, *minor_str, *patch_str;
	char *n, *d1, *d2;

	memset(version, 0, sizeof(version));
	strncpy(version, args, MAX_ARGS);
	version[MAX_ARGS] = '\0';

	n = strstr(version, ":");
	if (n)
		*n = '\0';

	d1 = strstr(version, ".");
	if (!d1)
		return -1;

	d2 = strstr(d1 + 1, ".");
	if (!d2)
		return -1;

	major_str = version;
	minor_str = d1 + 1;
	patch_str = d2 + 1;

	*d1 = '\0';
	*d2 = '\0';

	if (major)
		*major = atoi(major_str);
	if (minor)
		*minor = atoi(minor_str);
	if (patch)
		*patch = atoi(patch_str);

	return 0;
}

/*
 * These are few enough that arrays of function pointers can
 * be avoided.
 */

static int lm_prepare_lockspace(struct lockspace *ls, struct action *act)
{
	int rv;

	if (ls->lm_type == LD_LM_DLM)
		rv = lm_prepare_lockspace_dlm(ls);
	else if (ls->lm_type == LD_LM_SANLOCK)
		rv = lm_prepare_lockspace_sanlock(ls);
	else
		return -1;

	if (act)
		act->lm_rv = rv;
	return rv;
}

static int lm_add_lockspace(struct lockspace *ls, struct action *act, int adopt)
{
	int rv;

	if (ls->lm_type == LD_LM_DLM)
		rv = lm_add_lockspace_dlm(ls, adopt);
	else if (ls->lm_type == LD_LM_SANLOCK)
		rv = lm_add_lockspace_sanlock(ls, adopt);
	else
		return -1;

	if (act)
		act->lm_rv = rv;
	return rv;
}

static int lm_rem_lockspace(struct lockspace *ls, struct action *act, int free_vg)
{
	int rv;

	if (ls->lm_type == LD_LM_DLM)
		rv = lm_rem_lockspace_dlm(ls, free_vg);
	else if (ls->lm_type == LD_LM_SANLOCK)
		rv = lm_rem_lockspace_sanlock(ls, free_vg);
	else
		return -1;

	if (act)
		act->lm_rv = rv;
	return rv;
}

static int lm_lock(struct lockspace *ls, struct resource *r, int mode, struct action *act,
		   struct val_blk *vb_out, int *retry, int adopt)
{
	int rv;

	if (ls->lm_type == LD_LM_DLM)
		rv = lm_lock_dlm(ls, r, mode, vb_out, adopt);
	else if (ls->lm_type == LD_LM_SANLOCK)
		rv = lm_lock_sanlock(ls, r, mode, vb_out, retry, adopt);
	else
		return -1;

	if (act)
		act->lm_rv = rv;
	return rv;
}

static int lm_convert(struct lockspace *ls, struct resource *r,
		      int mode, struct action *act, uint32_t r_version)
{
	int rv;

	if (ls->lm_type == LD_LM_DLM)
		rv = lm_convert_dlm(ls, r, mode, r_version);
	else if (ls->lm_type == LD_LM_SANLOCK)
		rv = lm_convert_sanlock(ls, r, mode, r_version);
	else
		return -1;

	if (act)
		act->lm_rv = rv;
	return rv;
}

static int lm_unlock(struct lockspace *ls, struct resource *r, struct action *act,
		     uint32_t r_version, uint32_t lmu_flags)
{
	int rv;

	if (ls->lm_type == LD_LM_DLM)
		rv = lm_unlock_dlm(ls, r, r_version, lmu_flags);
	else if (ls->lm_type == LD_LM_SANLOCK)
		rv = lm_unlock_sanlock(ls, r, r_version, lmu_flags);
	else
		return -1;

	if (act)
		act->lm_rv = rv;
	return rv;
}

static int lm_hosts(struct lockspace *ls, int notify)
{
	if (ls->lm_type == LD_LM_DLM)
		return lm_hosts_dlm(ls, notify);
	else if (ls->lm_type == LD_LM_SANLOCK)
		return lm_hosts_sanlock(ls, notify);
	return -1;
}

static void lm_rem_resource(struct lockspace *ls, struct resource *r)
{
	if (ls->lm_type == LD_LM_DLM)
		lm_rem_resource_dlm(ls, r);
	else if (ls->lm_type == LD_LM_SANLOCK)
		lm_rem_resource_sanlock(ls, r);
}

static int lm_find_free_lock(struct lockspace *ls, uint64_t *free_offset)
{
	if (ls->lm_type == LD_LM_DLM)
		return 0;
	else if (ls->lm_type == LD_LM_SANLOCK)
		return lm_find_free_lock_sanlock(ls, free_offset);
	return -1;
}

/*
 * While adopting locks, actions originate from the adopt_locks()
 * function, not from a client.  So, these actions (flagged ADOPT),
 * should be passed back to the adopt_locks() function through the
 * adopt_results list, and not be sent back to a client via the
 * client_list/client_thread.
 */

static void add_client_result(struct action *act)
{
	if (act->flags & LD_AF_NO_CLIENT) {
		log_debug("internal action done op %s mode %s result %d vg %s",
			  op_str(act->op), mode_str(act->mode), act->result, act->vg_name);
		free_action(act);
		return;
	}

	pthread_mutex_lock(&client_mutex);
	if (act->flags & LD_AF_ADOPT)
		list_add_tail(&act->list, &adopt_results);
	else
		list_add_tail(&act->list, &client_results);
	pthread_cond_signal(&client_cond);
	pthread_mutex_unlock(&client_mutex);
}

static struct lock *find_lock_client(struct resource *r, uint32_t client_id)
{
	struct lock *lk;

	list_for_each_entry(lk, &r->locks, list) {
		if (lk->client_id == client_id)
			return lk;
	}
	return NULL;
}

static struct lock *find_lock_persistent(struct resource *r)
{
	struct lock *lk;

	list_for_each_entry(lk, &r->locks, list) {
		if (lk->flags & LD_LF_PERSISTENT)
			return lk;
	}
	return NULL;
}

static struct action *find_action_client(struct resource *r, uint32_t client_id)
{
	struct action *act;

	list_for_each_entry(act, &r->actions, list) {
		if (act->client_id != client_id)
			continue;
		return act;
	}
	return NULL;
}

static void add_work_action(struct action *act)
{
	pthread_mutex_lock(&worker_mutex);
	if (!worker_stop) {
		list_add_tail(&act->list, &worker_list);
		pthread_cond_signal(&worker_cond);
	}
	pthread_mutex_unlock(&worker_mutex);
}

static int res_lock(struct lockspace *ls, struct resource *r, struct action *act, int *retry)
{
	struct lock *lk;
	struct val_blk vb;
	uint32_t new_version = 0;
	int inval_meta;
	int rv = 0;

	memset(&vb, 0, sizeof(vb));

	r->last_client_id = act->client_id;

	if (r->type == LD_RT_LV)
		log_debug("S %s R %s res_lock cl %u mode %s (%s)", ls->name, r->name, act->client_id, mode_str(act->mode), act->lv_name);
	else
		log_debug("S %s R %s res_lock cl %u mode %s", ls->name, r->name, act->client_id, mode_str(act->mode));

	if (r->mode == LD_LK_SH && act->mode == LD_LK_SH)
		goto add_lk;

	if (r->type == LD_RT_LV && act->lv_args[0])
		memcpy(r->lv_args, act->lv_args, MAX_ARGS);

	rv = lm_lock(ls, r, act->mode, act, &vb, retry, act->flags & LD_AF_ADOPT);

	if (r->use_vb)
		log_debug("S %s R %s res_lock rv %d read vb %x %x %u",
			  ls->name, r->name, rv, vb.version, vb.flags, vb.r_version);
	else
		log_debug("S %s R %s res_lock rv %d", ls->name, r->name, rv);

	if (rv < 0)
		return rv;

	if (sanlock_gl_dup && ls->sanlock_gl_enabled)
		act->flags |= LD_AF_DUP_GL_LS;

	/*
	 * Check new lvb values to decide if lvmetad cache should
	 * be invalidated.  When we need to invalidate the lvmetad
	 * cache, but don't have a usable r_version from the lvb,
	 * send lvmetad new_version 0 which causes it to invalidate
	 * the VG metdata without comparing against the currently
	 * cached VG seqno.
	 */

	inval_meta = 0;

	if (!r->use_vb) {
		/* LV locks don't use an lvb. */

	} else if (vb.version && ((vb.version & 0xFF00) > (VAL_BLK_VERSION & 0xFF00))) {
		log_error("S %s R %s res_lock invalid val_blk version %x flags %x r_version %u",
			  ls->name, r->name, vb.version, vb.flags, vb.r_version);
		inval_meta = 1;
		new_version = 0;
		rv = -EINVAL;

	} else if (vb.r_version && (vb.r_version == r->version)) {
		/*
		 * Common case when the version hasn't changed.
		 * Do nothing.
		 */
	} else if (r->version && vb.r_version && (vb.r_version > r->version)) {
		/*
		 * Common case when the version has changed.  Another host
		 * has changed the data protected by the lock since we last
		 * acquired it, and increased r_version so we know that our
		 * cache is invalid.
		 */
		log_debug("S %s R %s res_lock got version %u our %u",
			  ls->name, r->name, vb.r_version, r->version);
		r->version = vb.r_version;
		new_version = vb.r_version;
		r->version_zero_valid = 0;
		inval_meta = 1;

	} else if (r->version_zero_valid && !vb.r_version) {
		/*
		 * The lvb is in a persistent zero state, which will end
		 * once someone uses the lock and writes a new lvb value.
		 * Do nothing.
		 */
		log_debug("S %s R %s res_lock version_zero_valid still zero", ls->name, r->name);

	} else if (r->version_zero_valid && vb.r_version) {
		/*
		 * Someone has written to the lvb after it was in a
		 * persistent zero state.  Begin tracking normal
		 * non-zero changes.  We may or may not have known
		 * about a previous non-zero version (in r->version).
		 * If we did, it means the lvb content was lost and
		 * has now been reinitialized.
		 *
		 * If the new reinitialized value is less than the
		 * previous non-zero value in r->version, then something
		 * unusual has happened.  For a VG lock, it probably
		 * means the VG was removed and recreated.  Invalidate
		 * our cache and begin using the new VG version.  For
		 * a GL lock, another host may have reinitialized a
		 * lost/zero lvb with a value less than we'd seen
		 * before.  Invalidate the cache, and begin using
		 * the lower version (or continue using our old
		 * larger version?)
		 */
		if (r->version && (r->version >= vb.r_version)) {
			log_debug("S %s R %s res_lock version_zero_valid got version %u less than our %u",
				  ls->name, r->name, vb.r_version, r->version);
			new_version = 0;
		} else {
			log_debug("S %s R %s res_lock version_zero_valid got version %u our %u",
				ls->name, r->name, vb.r_version, r->version);
			new_version = vb.r_version;
		}
		r->version = vb.r_version;
		r->version_zero_valid = 0;
		inval_meta = 1;

	} else if (!r->version && vb.r_version) {
		/*
		 * The first time we've acquired the lock and seen the lvb.
		 */
		log_debug("S %s R %s res_lock initial version %u", ls->name, r->name, vb.r_version);
		r->version = vb.r_version;
		inval_meta = 1;
		new_version = vb.r_version;
		r->version_zero_valid = 0;

	} else if (!r->version && !vb.r_version) {
		/*
		 * The lock may have never been used to change something.
		 * (e.g. a new sanlock GL?)
		 */
		log_debug("S %s R %s res_lock all versions zero", ls->name, r->name);
		if (!r->version_zero_valid) {
			inval_meta = 1;
			new_version = 0;
		}
		r->version_zero_valid = 1;

	} else if (r->version && !vb.r_version) {
		/*
		 * The lvb content has been lost or never been initialized.
		 * It can be lost during dlm recovery when the master node
		 * is removed.
		 *
		 * If we're the next to write the lvb, reinitialze it to the
		 * new VG seqno, or a new GL counter larger than was seen by
		 * any hosts before (how to estimate that?)
		 *
		 * If we see non-zero values before we next write to it, use
		 * those values.
		 *
		 * While the lvb values remain zero, the data for the lock
		 * is unchanged and we don't need to invalidate metadata.
		 */
		if ((ls->lm_type == LD_LM_DLM) && !vb.version && !vb.flags)
			log_debug("S %s R %s res_lock all lvb content is blank",
				  ls->name, r->name);
		log_debug("S %s R %s res_lock our version %u got vb %x %x %u",
			  ls->name, r->name, r->version, vb.version, vb.flags, vb.r_version);
		r->version_zero_valid = 1;
		inval_meta = 1;
		new_version = 0;

	} else if (r->version && vb.r_version && (vb.r_version < r->version)) {
		/*
		 * The lvb value has gone backwards, which shouldn't generally happen,
		 * but could when the dlm lvb is lost and reinitialized, or the VG
		 * is removed and recreated.
		 *
		 * If this is a VG lock, it probably means the VG has been removed
		 * and recreated while we had the dlm lockspace running.
		 * FIXME: how does the cache validation and replacement in lvmetad
		 * work in this case?
		 */
		log_debug("S %s R %s res_lock got version %u less than our version %u",
			  ls->name, r->name, vb.r_version, r->version);
		r->version = vb.r_version;
		inval_meta = 1;
		new_version = 0;
		r->version_zero_valid = 0;
	} else {
		log_debug("S %s R %s res_lock undefined vb condition vzv %d our version %u vb %x %x %u",
			  ls->name, r->name, r->version_zero_valid, r->version,
			  vb.version, vb.flags, vb.r_version);
	}

	if (vb.version && vb.r_version && (vb.flags & VBF_REMOVED)) {
		/* Should we set ls->thread_stop = 1 ? */
		log_debug("S %s R %s res_lock vb flag REMOVED",
			  ls->name, r->name);
		rv = -EREMOVED;
	}

	/*
	 * lvmetad is no longer used, but the infrastructure for
	 * distributed cache validation remains.  The points
	 * where vg or global cache state would be invalidated
	 * remain below and log_debug messages point out where
	 * they would occur.
	 *
	 * The comments related to "lvmetad" remain because they
	 * describe how some other local cache like lvmetad would
	 * be invalidated here.
	 */

	/*
	 * r is vglk: tell lvmetad to set the vg invalid
	 * flag, and provide the new r_version.  If lvmetad finds
	 * that its cached vg has seqno less than the value
	 * we send here, it will set the vg invalid flag.
	 * lvm commands that read the vg from lvmetad, will
	 * see the invalid flag returned, will reread the
	 * vg from disk, update the lvmetad copy, and go on.
	 *
	 * r is global: tell lvmetad to set the global invalid
	 * flag.  When commands see this flag returned from lvmetad,
	 * they will reread metadata from disk, update the lvmetad
	 * caches, and tell lvmetad to set global invalid to 0.
	 */

	/*
	 * lvmetad not running:
	 * Even if we have not previously found lvmetad running,
	 * we attempt to connect and invalidate in case it has
	 * been started while lvmlockd is running.  We don't
	 * want to allow lvmetad to be used with invalid data if
	 * it happens to be enabled and started after lvmlockd.
	 */

	if (inval_meta && (r->type == LD_RT_VG)) {
		log_debug("S %s R %s res_lock invalidate vg state version %u",
			  ls->name, r->name, new_version);
	}

	if (inval_meta && (r->type == LD_RT_GL)) {
		log_debug("S %s R %s res_lock invalidate global state", ls->name, r->name);
	}

	/*
	 * Record the new lock state.
	 */

	r->mode = act->mode;

add_lk:
	if (r->mode == LD_LK_SH)
		r->sh_count++;

	if (!(lk = alloc_lock()))
		return -ENOMEM;

	lk->client_id = act->client_id;
	lk->mode = act->mode;

	if (act->flags & LD_AF_PERSISTENT) {
		lk->flags |= LD_LF_PERSISTENT;
		lk->client_id = 0;
	}

	/*
	 * LV_LOCK means the action acquired the lv lock in the lock manager
	 * (as opposed to finding that the lv lock was already held).  If
	 * the client for this LV_LOCK action fails before we send the result,
	 * then we automatically unlock the lv since the lv wasn't activated.
	 * (There will always be an odd chance the lv lock is held while the
	 * lv is not active, but this helps.)  The most common case where this
	 * is helpful is when the lv lock operation is slow/delayed and the
	 * command is canceled by the user.
	 *
	 * LV_UNLOCK means the lv unlock action was generated by lvmlockd when
	 * it tried to send the reply for an lv lock action (with LV_LOCK set),
	 * and failed to send the reply to the client/command.  The
	 * last_client_id saved on the resource is compared to this LV_UNLOCK
	 * action before the auto unlock is done in case another action locked
	 * the lv between the failed client lock action and the auto unlock.
	 */
	if (r->type == LD_RT_LV)
		act->flags |= LD_AF_LV_LOCK;

	list_add_tail(&lk->list, &r->locks);

	return rv;
}

static int res_convert(struct lockspace *ls, struct resource *r,
		       struct lock *lk, struct action *act)
{
	uint32_t r_version;
	int rv;

	r->last_client_id = act->client_id;

	log_debug("S %s R %s res_convert cl %u mode %s", ls->name, r->name, act->client_id, mode_str(act->mode));

	if (act->mode == LD_LK_EX && lk->mode == LD_LK_SH && r->sh_count > 1)
		return -EAGAIN;

	/*
	 * lm_convert() writes new version (from ex)
	 * Same as lm_unlock()
	 */

        if ((r->type == LD_RT_GL) && (r->mode == LD_LK_EX)) {
		r->version++;
		lk->version = r->version;
		r_version = r->version;
		r->version_zero_valid = 0;

		log_debug("S %s R %s res_convert r_version inc %u",
			  ls->name, r->name, r_version);

	} else if ((r->type == LD_RT_VG) && (r->mode == LD_LK_EX) && (lk->version > r->version)) {
		r->version = lk->version;
		r_version = r->version;
		r->version_zero_valid = 0;

		log_debug("S %s R %s res_convert r_version new %u", ls->name, r->name, r_version);
	} else {
		r_version = 0;
	}

	rv = lm_convert(ls, r, act->mode, act, r_version);

	log_debug("S %s R %s res_convert rv %d", ls->name, r->name, rv);

	if (rv < 0)
		return rv;

	if (lk->mode == LD_LK_EX && act->mode == LD_LK_SH) {
		r->sh_count = 1;
	} else if (lk->mode == LD_LK_SH && act->mode == LD_LK_EX) {
		r->sh_count = 0;
	} else {
		/* should not be possible */
		log_error("S %s R %s res_convert invalid modes %d %d",
			  ls->name, r->name, lk->mode, act->mode);
		return -1;
	}

	r->mode = act->mode;
	lk->mode = act->mode;

	return 0;
}

static int res_cancel(struct lockspace *ls, struct resource *r,
		      struct action *act)
{
	struct action *cact;

	/*
	 * a client can cancel its own non-persistent lock requests,
	 * when could this happen?
	 *
	 * a client can cancel other client's persistent lock requests,
	 * when could this happen?
	 */

	if (act->flags & LD_AF_PERSISTENT) {
		list_for_each_entry(cact, &r->actions, list) {
			if (!(cact->flags & LD_AF_PERSISTENT))
				continue;
			goto do_cancel;
		}
	} else {
		cact = find_action_client(r, act->client_id);
		if (cact)
			goto do_cancel;
	}

	return -ENOENT;

do_cancel:
	log_debug("S %s R %s res_cancel cl %u", ls->name, r->name, cact->client_id);
	cact->result = -ECANCELED;
	list_del(&cact->list);
	add_client_result(cact);

	return -ECANCELED;
}

/*
 * lm_unlock() writes new a r_version (from ex)
 *
 * The r_version of the vg resource is incremented if
 * an "update" was received for the vg lock.  The update
 * contains the new vg seqno from the vg metadata which is
 * used as the r_version.
 *
 * The r_version of the global resource is automatically
 * incremented when it is unlocked from ex mode.
 *
 * r_version is incremented every time a command releases
 * the global lock from ex.
 */

/*
 * persistent locks will not be unlocked for OP_CLOSE/act_close
 * because act_close->flags does not have the PERSISTENT flag
 * set, and a persistent lk->client_id is zero, which will not
 * match the client in act_close->client_id.
 */

static int res_unlock(struct lockspace *ls, struct resource *r,
		      struct action *act)
{
	struct lock *lk;
	uint32_t r_version;
	int rv;

	if (act->flags & LD_AF_PERSISTENT) {
		lk = find_lock_persistent(r);
		if (lk)
			goto do_unlock;
	} else {
		lk = find_lock_client(r, act->client_id);
		if (lk)
			goto do_unlock;
	}

	if (act->op != LD_OP_CLOSE)
		log_debug("S %s R %s res_unlock cl %u no locks", ls->name, r->name, act->client_id);
	return -ENOENT;

do_unlock:
	if ((act->flags & LD_AF_LV_UNLOCK) && (r->last_client_id != act->client_id)) {
		log_debug("S %s R %s res_unlock cl %u for failed client ignored, last client %u",
			  ls->name, r->name, act->client_id, r->last_client_id);
		return -ENOENT;
	}

	r->last_client_id = act->client_id;

	if (act->op == LD_OP_CLOSE)
		log_debug("S %s R %s res_unlock cl %u from close", ls->name, r->name, act->client_id);
	else if (r->type == LD_RT_LV)
		log_debug("S %s R %s res_unlock cl %u (%s)", ls->name, r->name, act->client_id, act->lv_name);
	else
		log_debug("S %s R %s res_unlock cl %u", ls->name, r->name, act->client_id);

	/* send unlock to lm when last sh lock is unlocked */
	if (lk->mode == LD_LK_SH) {
		r->sh_count--;
		if (r->sh_count > 0) {
			log_debug("S %s R %s res_unlock sh_count %u", ls->name, r->name, r->sh_count);
			goto rem_lk;
		}
	}

	if ((r->type == LD_RT_GL) && (r->mode == LD_LK_EX)) {
		r->version++;
		lk->version = r->version;
		r_version = r->version;
		r->version_zero_valid = 0;

		log_debug("S %s R %s res_unlock r_version inc %u", ls->name, r->name, r_version);

	} else if ((r->type == LD_RT_VG) && (r->mode == LD_LK_EX) && (lk->version > r->version)) {
		r->version = lk->version;
		r_version = r->version;
		r->version_zero_valid = 0;

		log_debug("S %s R %s res_unlock r_version new %u",
			  ls->name, r->name, r_version);
	} else {
		r_version = 0;
	}

	rv = lm_unlock(ls, r, act, r_version, 0);
	if (rv < 0) {
		/* should never happen, retry? */
		log_error("S %s R %s res_unlock lm error %d", ls->name, r->name, rv);
		return rv;
	}

	log_debug("S %s R %s res_unlock lm done", ls->name, r->name);

rem_lk:
	list_del(&lk->list);
	free_lock(lk);

	if (list_empty(&r->locks))
		r->mode = LD_LK_UN;

	return 0;
}

static int res_update(struct lockspace *ls, struct resource *r,
		      struct action *act)
{
	struct lock *lk;

	lk = find_lock_client(r, act->client_id);
	if (!lk) {
		log_error("S %s R %s res_update cl %u lock not found",
			  ls->name, r->name, act->client_id);
		return -ENOENT;
	}

	if (r->mode != LD_LK_EX) {
		log_error("S %s R %s res_update cl %u version on non-ex lock",
			  ls->name, r->name, act->client_id);
		return -EINVAL;
	}

	/* lk version will be written to lm by unlock */

	if (act->flags & LD_AF_NEXT_VERSION)
		lk->version = r->version + 1;
	else {
		if (r->version >= act->version) {
			/*
			 * This update is done from vg_write. If the metadata with
			 * this seqno is not committed by vg_commit, then next
			 * vg_write can use the same seqno, causing us to see no
			 * increase in seqno here as expected.
			 * FIXME: In this case, do something like setting the lvb
			 * version to 0 to instead of the same seqno which will
			 * force an invalidation on other hosts.  The next change
			 * will return to using the seqno again.
			 */
			log_error("S %s R %s res_update cl %u old version %u new version %u too small",
			  	  ls->name, r->name, act->client_id, r->version, act->version);
		}
		lk->version = act->version;
	}

	log_debug("S %s R %s res_update cl %u lk version to %u", ls->name, r->name, act->client_id, lk->version);

	return 0;
}

/*
 * There is nothing to deallocate when freeing a dlm LV, the LV
 * will simply be unlocked by rem_resource.
 */

static int free_lv(struct lockspace *ls, struct resource *r)
{
	if (ls->lm_type == LD_LM_SANLOCK)
		return lm_free_lv_sanlock(ls, r);
	else if (ls->lm_type == LD_LM_DLM)
		return 0;
	else
		return -EINVAL;
}

/*
 * NB. we can't do this if sanlock is holding any locks on
 * the resource; we'd be rewriting the resource from under
 * sanlock and would confuse or break it badly.  We don't
 * know what another host is doing, so these must be used
 * very carefully.
 */

static int res_able(struct lockspace *ls, struct resource *r,
		    struct action *act)
{
	int rv;

	if (ls->lm_type != LD_LM_SANLOCK) {
		log_error("enable/disable only applies to sanlock");
		return -EINVAL;
	}

	if (r->type != LD_RT_GL) {
		log_error("enable/disable only applies to global lock");
		return -EINVAL;
	}

	if (r->mode != LD_LK_UN) {
		log_error("enable/disable only allowed on unlocked resource");
		return -EINVAL;
	}

	if (act->op == LD_OP_ENABLE && gl_lsname_sanlock[0]) {
		log_error("disable global lock in %s before enable in %s",
			  gl_lsname_sanlock, ls->name);
		return -EINVAL;
	}

	if ((act->op == LD_OP_DISABLE) && (act->flags & LD_AF_EX_DISABLE)) {
		rv = lm_ex_disable_gl_sanlock(ls);
		goto out;
	}

	rv = lm_able_gl_sanlock(ls, act->op == LD_OP_ENABLE);

	if (!rv && (act->op == LD_OP_ENABLE))
		gl_vg_removed = 0;
out:
	return rv;
}

/*
 * Go through queued actions, and make lock/unlock calls on the resource
 * based on the actions and the existing lock state.
 *
 * All lock operations sent to the lock manager are non-blocking.
 * This is because sanlock does not support lock queueing.
 * Eventually we could enhance this to take advantage of lock
 * queueing when available (i.e. for the dlm).
 *
 * act_close_list: list of CLOSE actions, identifying clients that have
 * closed/terminated their lvmlockd connection, and whose locks should
 * be released.  Do not remove these actions from act_close_list.
 *
 * retry_out: set to 1 if the lock manager said we should retry,
 * meaning we should call res_process() again in a short while to retry.
 */

static void res_process(struct lockspace *ls, struct resource *r,
			struct list_head *act_close_list, int *retry_out)
{
	struct action *act, *safe, *act_close;
	struct lock *lk;
	int lm_retry;
	int rv;

	/*
	 * handle version updates for ex locks
	 * (new version will be written by unlock)
	 */

	list_for_each_entry_safe(act, safe, &r->actions, list) {
		if (act->op == LD_OP_UPDATE) {
			rv = res_update(ls, r, act);
			act->result = rv;
			list_del(&act->list);
			add_client_result(act);
		}
	}

	/*
	 * handle explicit unlock actions
	 */

	list_for_each_entry_safe(act, safe, &r->actions, list) {
		if ((act->op == LD_OP_LOCK) &&
		    (act->mode == LD_LK_IV || act->mode == LD_LK_NL)) {
			act->result = -EINVAL;
			list_del(&act->list);
			add_client_result(act);
		}

		if (act->op == LD_OP_LOCK && act->mode == LD_LK_UN) {
			rv = res_unlock(ls, r, act);

			if (rv == -ENOENT && (act->flags & LD_AF_UNLOCK_CANCEL))
				rv = res_cancel(ls, r, act);

			/*
			 * possible unlock results:
			 * 0: unlock succeeded
			 * -ECANCELED: cancel succeeded
			 * -ENOENT: nothing to unlock or cancel
			 */

			act->result = rv;
			list_del(&act->list);
			add_client_result(act);
		}
	}

	/*
	 * handle implicit unlocks due to client exit,
	 * also clear any outstanding actions for the client
	 */

	list_for_each_entry(act_close, act_close_list, list) {
		res_unlock(ls, r, act_close);
		res_cancel(ls, r, act_close);
	}

	/*
	 * handle freeing a lock for an lv that has been removed
	 */

	list_for_each_entry_safe(act, safe, &r->actions, list) {
		if (act->op == LD_OP_FREE && act->rt == LD_RT_LV) {
			log_debug("S %s R %s free_lv", ls->name, r->name);
			rv = free_lv(ls, r);
			act->result = rv;
			list_del(&act->list);
			add_client_result(act);
			goto r_free;

		}
	}

	/*
	 * handle enable/disable
	 */

	list_for_each_entry_safe(act, safe, &r->actions, list) {
		if (act->op == LD_OP_ENABLE || act->op == LD_OP_DISABLE) {
			rv = res_able(ls, r, act);
			act->result = rv;
			list_del(&act->list);
			add_client_result(act);

			if (!rv && act->op == LD_OP_DISABLE) {
				log_debug("S %s R %s free disabled", ls->name, r->name);
				goto r_free;
			}
		}
	}

	/*
	 * transient requests on existing transient locks
	 */

	list_for_each_entry_safe(act, safe, &r->actions, list) {
		if (act->flags & LD_AF_PERSISTENT)
			continue;

		lk = find_lock_client(r, act->client_id);
		if (!lk)
			continue;

		if (lk->mode != act->mode) {
			/* convert below */
			/*
			act->result = -EEXIST;
			list_del(&act->list);
			add_client_result(act);
			*/
			continue;
		} else {
			/* success */
			r->last_client_id = act->client_id;
			act->result = -EALREADY;
			list_del(&act->list);
			add_client_result(act);
		}
	}

	/*
	 * persistent requests on existing persistent locks
	 *
	 * persistent locks are not owned by a client, so any
	 * existing with matching mode satisfies a request.
	 * only one persistent lock is kept on a resource.
	 * a single "unowned" persistent lock satisfies
	 * any/multiple client requests for a persistent lock.
	 */

	list_for_each_entry_safe(act, safe, &r->actions, list) {
		if (!(act->flags & LD_AF_PERSISTENT))
			continue;

		lk = find_lock_persistent(r);
		if (!lk)
			continue;

		if (lk->mode != act->mode) {
			/* convert below */
			/*
			act->result = -EEXIST;
			list_del(&act->list);
			add_client_result(act);
			*/
			continue;
		} else {
			/* success */
			r->last_client_id = act->client_id;
			act->result = -EALREADY;
			list_del(&act->list);
			add_client_result(act);
		}
	}

	/*
	 * transient requests with existing persistent locks
	 *
	 * Just grant the transient request and do not
	 * keep a record of it.  Assume that the persistent
	 * lock will not go away while the transient lock
	 * is needed.
	 *
	 * This would be used when an ex, persistent lv lock
	 * exists from activation, and then something like
	 * lvextend asks for a transient ex lock to change
	 * the lv.  The lv could not be unlocked by deactivation
	 * while the lvextend was running.
	 *
	 * The logic here for mixing T/P locks is not general
	 * support; there are a number of cases where it will
	 * not work: updating version number (lv locks have
	 * none), ex locks from multiple clients will not
	 * conflict, explicit un of the transient lock will fail.
	 */

	list_for_each_entry_safe(act, safe, &r->actions, list) {
		if (act->flags & LD_AF_PERSISTENT)
			continue;

		lk = find_lock_persistent(r);
		if (!lk)
			continue;

		if ((lk->mode == LD_LK_EX) ||
		    (lk->mode == LD_LK_SH && act->mode == LD_LK_SH)) {
			r->last_client_id = act->client_id;
			act->result = 0;
			list_del(&act->list);
			add_client_result(act);
		} else {
			/* persistent lock is sh, transient request is ex */
			/* FIXME: can we remove this case? do a convert here? */
			log_debug("res_process %s existing persistent lock new transient", r->name);
			r->last_client_id = act->client_id;
			act->result = -EEXIST;
			list_del(&act->list);
			add_client_result(act);
		}
	}

	/*
	 * persistent requests with existing transient locks
	 *
	 * If a client requests a P (persistent) lock for a T (transient)
	 * lock it already holds, we can just change T to P.  Fail if the
	 * same happens for locks from different clients.  Changing
	 * another client's lock from T to P may cause problems
	 * if that client tries to unlock or update version.
	 *
	 * I don't think this P/T combination will be used.
	 * It might be used if a command was able to take a P
	 * vg lock, in which case the T vg lock would already
	 * be held for reading.  If the T lock was sh, it would
	 * be converted to P ex.  If the T/P modes matched, the
	 * lock could just be changed from T to P.
	 */

	list_for_each_entry_safe(act, safe, &r->actions, list) {
		if (!(act->flags & LD_AF_PERSISTENT))
			continue;

		lk = find_lock_client(r, act->client_id);
		if (!lk)
			continue;

		if (lk->mode != act->mode) {
			/* FIXME: convert and change to persistent? */
			log_debug("res_process %s existing transient lock new persistent", r->name);
			r->last_client_id = act->client_id;
			act->result = -EEXIST;
			list_del(&act->list);
			add_client_result(act);
		} else {
			r->last_client_id = act->client_id;
			lk->flags |= LD_LF_PERSISTENT;
			lk->client_id = 0;
			act->result = 0;
			list_del(&act->list);
			add_client_result(act);
		}
	}

	/*
	 * convert mode of existing locks
	 */

	list_for_each_entry_safe(act, safe, &r->actions, list) {
		if (act->flags & LD_AF_PERSISTENT)
			lk = find_lock_persistent(r);
		else
			lk = find_lock_client(r, act->client_id);
		if (!lk)
			continue;

		if (lk->mode == act->mode) {
			/* should never happen, should be found above */
			log_error("convert same mode");
			continue;
		}

		/* convert fails immediately, no EAGAIN retry */
		rv = res_convert(ls, r, lk, act);
		act->result = rv;
		list_del(&act->list);
		add_client_result(act);
	}

	/*
	 * Cases above are all requests addressed by existing locks.
	 * Below handles the rest.  Transient and persistent are
	 * handled the same, except
	 * - if mode of existing lock is incompat with requested,
	 *   leave the act on r->actions
	 * - if r mode is EX, any lock action is blocked, just quit
	 *
	 * Retry a lock request that fails due to a lock conflict (-EAGAIN):
	 * if we have not exceeded max retries and lm sets lm_retry (sanlock
	 * transient conflicts from shared lock implementation), or r type
	 * is gl or vg (transient real conflicts we want to hide from command).
	 * lv lock conflicts won't be transient so don't retry them.
	 */

	if (r->mode == LD_LK_EX)
		return;

	/*
	 * r mode is SH or UN, pass lock-sh actions to lm
	 */

	list_for_each_entry_safe(act, safe, &r->actions, list) {
		/* grant in order, so break here */
		if (act->op == LD_OP_LOCK && act->mode == LD_LK_EX)
			break;

		if (act->op == LD_OP_LOCK && act->mode == LD_LK_SH) {
			lm_retry = 0;

			rv = res_lock(ls, r, act, &lm_retry);
			if ((rv == -EAGAIN) &&
			    (act->retries <= act->max_retries) &&
			    (lm_retry || (r->type != LD_RT_LV))) {
				/* leave act on list */
				log_debug("S %s R %s res_lock EAGAIN retry", ls->name, r->name);
				act->retries++;
				*retry_out = 1;
			} else {
				act->result = rv;
				list_del(&act->list);
				add_client_result(act);
			}
			if (rv == -EUNATCH)
				goto r_free;
		}
	}

	/*
	 * r mode is SH, any ex lock action is blocked, just quit
	 */

	if (r->mode == LD_LK_SH)
		return;

	/*
	 * r mode is UN, pass lock-ex action to lm
	 */

	list_for_each_entry_safe(act, safe, &r->actions, list) {
		if (act->op == LD_OP_LOCK && act->mode == LD_LK_EX) {
			lm_retry = 0;

			rv = res_lock(ls, r, act, &lm_retry);
			if ((rv == -EAGAIN) &&
			    (act->retries <= act->max_retries) &&
			    (lm_retry || (r->type != LD_RT_LV))) {
				/* leave act on list */
				log_debug("S %s R %s res_lock EAGAIN retry", ls->name, r->name);
				act->retries++;
				*retry_out = 1;
			} else {
				act->result = rv;
				list_del(&act->list);
				add_client_result(act);
			}
			if (rv == -EUNATCH)
				goto r_free;
			break;
		}
	}

	return;

r_free:
	/* For the EUNATCH case it may be possible there are queued actions? */
	list_for_each_entry_safe(act, safe, &r->actions, list) {
		log_error("S %s R %s res_process r_free cancel %s client %d",
			  ls->name, r->name, op_str(act->op), act->client_id);
		act->result = -ECANCELED;
		list_del(&act->list);
		add_client_result(act);
	}
	log_debug("S %s R %s res_process free", ls->name, r->name);
	lm_rem_resource(ls, r);
	list_del(&r->list);
	free_resource(r);
}

#define LOCKS_EXIST_ANY 1
#define LOCKS_EXIST_GL  2
#define LOCKS_EXIST_VG  3
#define LOCKS_EXIST_LV  4

static int for_each_lock(struct lockspace *ls, int locks_do)
{
	struct resource *r;
	struct lock *lk;

	list_for_each_entry(r, &ls->resources, list) {
		list_for_each_entry(lk, &r->locks, list) {
			if (locks_do == LOCKS_EXIST_ANY)
				return 1;

			if (locks_do == LOCKS_EXIST_GL && r->type == LD_RT_GL)
				return 1;

			if (locks_do == LOCKS_EXIST_VG && r->type == LD_RT_VG)
				return 1;

			if (locks_do == LOCKS_EXIST_LV && r->type == LD_RT_LV)
				return 1;
		}
	}

	return 0;
}

static int clear_locks(struct lockspace *ls, int free_vg, int drop_vg)
{
	struct resource *r, *r_safe;
	struct lock *lk, *lk_safe;
	struct action *act, *act_safe;
	uint32_t lk_version;
	uint32_t r_version;
	int lk_count = 0;
	int rv;

	list_for_each_entry_safe(r, r_safe, &ls->resources, list) {
		lk_version = 0;

		list_for_each_entry_safe(lk, lk_safe, &r->locks, list) {
			lk_count++;

			/*
			 * Stopping a lockspace shouldn't happen with LV locks
			 * still held, but it will be stopped with GL and VG
			 * locks held.  The drop_vg case may see LV locks.
			 */

			if (lk->flags & LD_LF_PERSISTENT && !drop_vg)
				log_error("S %s R %s clear lock persistent", ls->name, r->name);
			else
				log_debug("S %s R %s clear lock mode %s client %d", ls->name, r->name, mode_str(lk->mode), lk->client_id);

			if (lk->version > lk_version)
				lk_version = lk->version;

			list_del(&lk->list);
			free_lock(lk);
		}

		if (r->mode == LD_LK_UN)
			goto r_free;

		if ((r->type == LD_RT_GL) && (r->mode == LD_LK_EX)) {
			r->version++;
			r_version = r->version;
			log_debug("S %s R %s clear_locks r_version inc %u",
				  ls->name, r->name, r_version);

		} else if ((r->type == LD_RT_VG) && (r->mode == LD_LK_EX) && (lk_version > r->version)) {
			r->version = lk_version;
			r_version = r->version;
			log_debug("S %s R %s clear_locks r_version new %u",
				  ls->name, r->name, r_version);

		} else {
			r_version = 0;
		}

		rv = lm_unlock(ls, r, NULL, r_version, free_vg ? LMUF_FREE_VG : 0);
		if (rv < 0) {
			/* should never happen */
			log_error("S %s R %s clear_locks free %d drop %d lm unlock error %d",
				  ls->name, r->name, free_vg, drop_vg, rv);
		}

		list_for_each_entry_safe(act, act_safe, &r->actions, list) {
			log_error("S %s R %s clear_locks cancel %s client %d",
				  ls->name, r->name, op_str(act->op), act->client_id);
			act->result = -ECANCELED;
			list_del(&act->list);
			add_client_result(act);
		}
 r_free:
		log_debug("S %s R %s free", ls->name, r->name);
		lm_rem_resource(ls, r);
		list_del(&r->list);
		free_resource(r);
	}

	return lk_count;
}

/*
 * find and return the resource that is referenced by the action
 * - there is a single gl resource per lockspace
 * - there is a single vg resource per lockspace
 * - there can be many lv resources per lockspace, compare names
 */

static struct resource *find_resource_act(struct lockspace *ls,
					  struct action *act,
					  int nocreate)
{
	struct resource *r;

	list_for_each_entry(r, &ls->resources, list) {
		if (r->type != act->rt)
			continue;

		if (r->type == LD_RT_GL && act->rt == LD_RT_GL)
			return r;

		if (r->type == LD_RT_VG && act->rt == LD_RT_VG)
			return r;

		if (r->type == LD_RT_LV && act->rt == LD_RT_LV &&
		    !strcmp(r->name, act->lv_uuid))
			return r;
	}

	if (nocreate)
		return NULL;

	if (!(r = alloc_resource()))
		return NULL;

	r->type = act->rt;
	r->mode = LD_LK_UN;

	if (r->type == LD_RT_GL) {
		strncpy(r->name, R_NAME_GL, MAX_NAME);
		r->use_vb = 1;
	} else if (r->type == LD_RT_VG) {
		strncpy(r->name, R_NAME_VG, MAX_NAME);
		r->use_vb = 1;
	} else if (r->type == LD_RT_LV) {
		strncpy(r->name, act->lv_uuid, MAX_NAME);
		r->use_vb = 0;
	}

	list_add_tail(&r->list, &ls->resources);

	return r;
}

static void free_ls_resources(struct lockspace *ls)
{
	struct resource *r, *r_safe;

	list_for_each_entry_safe(r, r_safe, &ls->resources, list) {
		lm_rem_resource(ls, r);
		list_del(&r->list);
		free_resource(r);
	}
}

/*
 * ls is the vg being removed that holds the global lock.
 * check if any other vgs will be left without a global lock.
 */

static int other_sanlock_vgs_exist(struct lockspace *ls_rem)
{
	struct lockspace *ls;

	list_for_each_entry(ls, &lockspaces, list) {
		if (ls->lm_type != LD_LM_SANLOCK)
			continue;
		if (!strcmp(ls->name, ls_rem->name))
			continue;
		log_debug("other sanlock vg exists %s", ls->name);
		return 1;
	}

	return 0;
}

/*
 * LOCK is the main thing we're interested in; the others are unlikely.
 */

static int process_op_during_kill(struct action *act)
{
	if (act->op == LD_OP_LOCK && act->mode == LD_LK_UN)
		return 1;

	switch (act->op) {
	case LD_OP_LOCK:
	case LD_OP_ENABLE:
	case LD_OP_DISABLE:
	case LD_OP_UPDATE:
	case LD_OP_RENAME_BEFORE:
	case LD_OP_RENAME_FINAL:
	case LD_OP_FIND_FREE_LOCK:
		return 0;
	};
	return 1;
}

/*
 * Process actions queued for this lockspace by
 * client_recv_action / add_lock_action.
 *
 * The lockspace_thread can touch its own ls struct without holding
 * lockspaces_mutex until it sets ls->thread_done, after which it
 * cannot touch ls without holding lockspaces_mutex.
 */

#define LOCK_RETRY_MS 1000 /* milliseconds to delay between retry */

static void *lockspace_thread_main(void *arg_in)
{
	struct lockspace *ls = arg_in;
	struct resource *r, *r2;
	struct action *add_act, *act, *safe;
	struct action *act_op_free = NULL;
	struct list_head tmp_act;
	struct list_head act_close;
	char tmp_name[MAX_NAME+1];
	int free_vg = 0;
	int drop_vg = 0;
	int error = 0;
	int adopt_flag = 0;
	int wait_flag = 0;
	int retry;
	int rv;

	INIT_LIST_HEAD(&act_close);

	/* first action may be client add */
	pthread_mutex_lock(&ls->mutex);
	act = NULL;
	add_act = NULL;
	if (!list_empty(&ls->actions)) {
		act = list_first_entry(&ls->actions, struct action, list);
		if (act->op == LD_OP_START) {
			add_act = act;
			list_del(&add_act->list);

			if (add_act->flags & LD_AF_WAIT)
				wait_flag = 1;
			if (add_act->flags & LD_AF_ADOPT)
				adopt_flag = 1;
		}
	}
	pthread_mutex_unlock(&ls->mutex);

	log_debug("S %s lm_add_lockspace %s wait %d adopt %d",
		  ls->name, lm_str(ls->lm_type), wait_flag, adopt_flag);

	/*
	 * The prepare step does not wait for anything and is quick;
	 * it tells us if the parameters are valid and the lm is running.
	 */
	error = lm_prepare_lockspace(ls, add_act);

	if (add_act && (!wait_flag || error)) {
		/* send initial join result back to client */
		add_act->result = error;
		add_client_result(add_act);
		add_act = NULL;
	}

	/*
	 * The actual lockspace join can take a while.
	 */
	if (!error) {
		error = lm_add_lockspace(ls, add_act, adopt_flag);

		log_debug("S %s lm_add_lockspace done %d", ls->name, error);

		if (ls->sanlock_gl_enabled && gl_lsname_sanlock[0] &&
		    strcmp(ls->name, gl_lsname_sanlock))
			sanlock_gl_dup = 1;

		if (add_act) {
			/* send final join result back to client */
			add_act->result = error;
			add_client_result(add_act);
		}
	}

	pthread_mutex_lock(&ls->mutex);
	if (error) {
		ls->thread_stop = 1;
		ls->create_fail = 1;
	} else {
		ls->create_done = 1;
	}
	pthread_mutex_unlock(&ls->mutex);

	if (error)
		goto out_act;

	while (1) {
		pthread_mutex_lock(&ls->mutex);
		while (!ls->thread_work) {
			if (ls->thread_stop) {
				pthread_mutex_unlock(&ls->mutex);
				goto out_rem;
			}
			pthread_cond_wait(&ls->cond, &ls->mutex);
		}

		/*
		 * Process all the actions queued for this lockspace.
		 * The client thread queues actions on ls->actions.
		 *
		 * Here, take all the actions off of ls->actions, and:
		 *
		 * - For lock operations, move the act to r->actions.
		 *   These lock actions/operations processed by res_process().
		 *
		 * - For non-lock operations, e.g. related to managing
		 *   the lockspace, process them in this loop.
		 */

		while (1) {
			if (list_empty(&ls->actions)) {
				ls->thread_work = 0;
				break;
			}

			act = list_first_entry(&ls->actions, struct action, list);

			if (act->op == LD_OP_KILL_VG && act->rt == LD_RT_VG) {
				/* Continue processing until DROP_VG arrives. */
				log_debug("S %s kill_vg", ls->name);
				ls->kill_vg = 1;
				list_del(&act->list);
				act->result = 0;
				add_client_result(act);
				continue;
			}

			if (ls->kill_vg && !process_op_during_kill(act)) {
				log_debug("S %s disallow op %s after kill_vg", ls->name, op_str(act->op));
				list_del(&act->list);
				act->result = -EVGKILLED;
				add_client_result(act);
				continue;
			}

			if (act->op == LD_OP_DROP_VG && act->rt == LD_RT_VG) {
				/*
				 * If leases are released after i/o errors begin
				 * but before lvmlockctl --kill, then the VG is not
				 * killed, but drop is still needed to clean up the
				 * VG, so in that case there would be a drop op without
				 * a preceding kill op.
				 */
				if (!ls->kill_vg)
					log_debug("S %s received drop without kill", ls->name);
				log_debug("S %s drop_vg", ls->name);
				ls->thread_work = 0;
				ls->thread_stop = 1;
				drop_vg = 1;
				break;
			}

			if (act->op == LD_OP_STOP) {
				/* thread_stop is already set */
				ls->thread_work = 0;
				break;
			}

			if (act->op == LD_OP_FREE && act->rt == LD_RT_VG) {
				/* vgremove */
				log_debug("S %s checking for lockspace hosts", ls->name);
				rv = lm_hosts(ls, 1);
				if (rv) {
					/*
					 * Checking for hosts here in addition to after the
					 * main loop allows vgremove to fail and be rerun
					 * after the ls is stopped on other hosts.
					 */
					log_error("S %s lockspace hosts %d", ls->name, rv);
					list_del(&act->list);
					act->result = -EBUSY;
					add_client_result(act);
					continue;
				}
				ls->thread_work = 0;
				ls->thread_stop = 1;
				free_vg = 1;
				break;
			}

			if (act->op == LD_OP_BUSY && act->rt == LD_RT_VG) {
				log_debug("S %s checking if lockspace is busy", ls->name);
				rv = lm_hosts(ls, 0);
				if (rv)
					act->result = -EBUSY;
				else
					act->result = 0;
				list_del(&act->list);
				add_client_result(act);
				continue;
			}

			if (act->op == LD_OP_RENAME_BEFORE && act->rt == LD_RT_VG) {
				/* vgrename */
				log_debug("S %s checking for lockspace hosts", ls->name);
				rv = lm_hosts(ls, 1);
				if (rv) {
					log_error("S %s lockspace hosts %d", ls->name, rv);
					list_del(&act->list);
					act->result = -EBUSY;
					add_client_result(act);
					continue;
				}
				ls->thread_work = 0;
				ls->thread_stop = 1;
				/* Do we want to check hosts again below like vgremove? */
				break;
			}

			if (act->op == LD_OP_FIND_FREE_LOCK && act->rt == LD_RT_VG) {
				uint64_t free_offset = 0;
				log_debug("S %s find free lock", ls->name);
				rv = lm_find_free_lock(ls, &free_offset);
				log_debug("S %s find free lock %d offset %llu",
					  ls->name, rv, (unsigned long long)free_offset);
				ls->free_lock_offset = free_offset;
				list_del(&act->list);
				act->result = rv;
				add_client_result(act);
				continue;
			}

			list_del(&act->list);

			/* applies to all resources */
			if (act->op == LD_OP_CLOSE) {
				list_add(&act->list, &act_close);
				continue;
			}

			/*
			 * All the other op's are for locking.
			 * Find the specific resource that the lock op is for,
			 * and add the act to the resource's list of lock ops.
			 *
			 * (This creates a new resource if the one named in
			 * the act is not found.)
			 */

			r = find_resource_act(ls, act, (act->op == LD_OP_FREE) ? 1 : 0);
			if (!r) {
				act->result = (act->op == LD_OP_FREE) ? -ENOENT : -ENOMEM;
				add_client_result(act);
				continue;
			}

			list_add_tail(&act->list, &r->actions);

			log_debug("S %s R %s action %s %s", ls->name, r->name,
				  op_str(act->op), mode_str(act->mode));
		}
		pthread_mutex_unlock(&ls->mutex);

		/*
		 * Process the lock operations that have been queued for each
		 * resource.
		 */

		retry = 0;

		list_for_each_entry_safe(r, r2, &ls->resources, list)
			res_process(ls, r, &act_close, &retry);

		list_for_each_entry_safe(act, safe, &act_close, list) {
			list_del(&act->list);
			free_action(act);
		}

		if (retry) {
			ls->thread_work = 1;
			usleep(LOCK_RETRY_MS * 1000);
		}
	}

out_rem:
	log_debug("S %s stopping", ls->name);

	/*
	 * For sanlock, we need to unlock any existing locks
	 * before removing the lockspace, otherwise the sanlock
	 * daemon will kill us when the lockspace goes away.
	 * For dlm, we leave with force, so all locks will
	 * automatically be dropped when we leave the lockspace,
	 * so unlocking all before leaving could be skipped.
	 *
	 * Blindly dropping all existing locks must only be
	 * allowed in emergency/force situations, otherwise it's
	 * obviously dangerous, since the lock holders are still
	 * operating under the assumption that they hold the lock.
	 * drop_vg drops all existing locks, but should only
	 * happen when the VG access has been forcibly and
	 * succesfully terminated.
	 *
	 * For vgremove of a sanlock vg, the vg lock will be held,
	 * and possibly the gl lock if this vg holds the gl.
	 * sanlock vgremove wants to unlock-rename these locks.
	 */

	log_debug("S %s clearing locks", ls->name);

	rv = clear_locks(ls, free_vg, drop_vg);

	/*
	 * Tell any other hosts in the lockspace to leave it
	 * before we remove it (for vgremove).  We do this
	 * before leaving the lockspace ourself because we
	 * need to be in the lockspace to see others.
	 */

	if (free_vg) {
		log_debug("S %s checking for lockspace hosts", ls->name);
		rv = lm_hosts(ls, 1);
		if (rv)
			log_error("S %s other lockspace hosts %d", ls->name, rv);
	}

	/*
	 * Leave the lockspace.
	 */

	rv = lm_rem_lockspace(ls, NULL, free_vg);

	log_debug("S %s rem_lockspace done %d", ls->name, rv);

out_act:
	/*
	 * Move remaining actions to results; this will usually (always?)
	 * be only the stop action.
	 */
	INIT_LIST_HEAD(&tmp_act);

	pthread_mutex_lock(&ls->mutex);
	list_for_each_entry_safe(act, safe, &ls->actions, list) {
		if (act->op == LD_OP_FREE) {
			act_op_free = act;
			act->result = 0;
		} else if (act->op == LD_OP_STOP)
			act->result = 0;
		else if (act->op == LD_OP_DROP_VG)
			act->result = 0;
		else if (act->op == LD_OP_RENAME_BEFORE)
			act->result = 0;
		else
			act->result = -ENOLS;
		list_del(&act->list);
		list_add_tail(&act->list, &tmp_act);
	}
	pthread_mutex_unlock(&ls->mutex);

	/*
	 * If this freed a sanlock vg that had gl enabled, and other sanlock
	 * vgs exist, return a flag so the command can warn that the gl has
	 * been removed and may need to be enabled in another sanlock vg.
	 */

	if (free_vg && ls->sanlock_gl_enabled && act_op_free) {
		pthread_mutex_lock(&lockspaces_mutex);
		if (other_sanlock_vgs_exist(ls)) {
			act_op_free->flags |= LD_AF_WARN_GL_REMOVED;
			gl_vg_removed = 1;
		}
		pthread_mutex_unlock(&lockspaces_mutex);
	}

	pthread_mutex_lock(&client_mutex);
	list_for_each_entry_safe(act, safe, &tmp_act, list) {
		list_del(&act->list);
		list_add_tail(&act->list, &client_results);
	}
	pthread_cond_signal(&client_cond);
	pthread_mutex_unlock(&client_mutex);

	pthread_mutex_lock(&lockspaces_mutex);
	ls->thread_done = 1;
	ls->free_vg = free_vg;
	ls->drop_vg = drop_vg;
	if (ls->lm_type == LD_LM_DLM && !strcmp(ls->name, gl_lsname_dlm))
		global_dlm_lockspace_exists = 0;

	/*
	 * Avoid a name collision of the same lockspace is added again before
	 * this thread is cleaned up.  We just set ls->name to a "junk" value
	 * for the short period until the struct is freed.  We could make it
	 * blank or fill it with garbage, but instead set it to REM:<name>
	 * to make it easier to follow progress of freeing is via log_debug.
	 */
	dm_strncpy(tmp_name, ls->name, sizeof(tmp_name));
	snprintf(ls->name, sizeof(ls->name), "REM:%s", tmp_name);
	pthread_mutex_unlock(&lockspaces_mutex);

	/* worker_thread will join this thread, and free the ls */
	pthread_mutex_lock(&worker_mutex);
	worker_wake = 1;
	pthread_cond_signal(&worker_cond);
	pthread_mutex_unlock(&worker_mutex);

	return NULL;
}

int lockspaces_empty(void)
{
	int rv;
	pthread_mutex_lock(&lockspaces_mutex);
	rv = list_empty(&lockspaces);
	pthread_mutex_unlock(&lockspaces_mutex);
	return rv;
}

/*
 * lockspaces_mutex is locked
 *
 * When duplicate sanlock global locks have been seen,
 * this function has a secondary job of counting the
 * number of lockspaces that exist with the gl enabled,
 * with the side effect of setting sanlock_gl_dup back to
 * zero when the duplicates have been removed/disabled.
 */

static struct lockspace *find_lockspace_name(char *ls_name)
{
	struct lockspace *ls_found = NULL;
	struct lockspace *ls;
	int gl_count = 0;

	list_for_each_entry(ls, &lockspaces, list) {
		if (!strcmp(ls->name, ls_name))
			ls_found = ls;

		if (!sanlock_gl_dup && ls_found)
			return ls_found;

		if (sanlock_gl_dup && ls->sanlock_gl_enabled)
			gl_count++;
	}

	/* this is the side effect we want from this function */
	if (sanlock_gl_dup && gl_count < 2)
		sanlock_gl_dup = 0;

	return ls_found;
}

/*
 * If lvm_<vg_name> is longer than max lockspace name (64) we just ignore the
 * extra characters.  For sanlock vgs, the name is shortened further to 48 in
 * the sanlock code.
 */

static int vg_ls_name(const char *vg_name, char *ls_name)
{
	if (strlen(vg_name) + 4 > MAX_NAME) {
		log_error("vg name too long %s", vg_name);
		return -1;
	}

	snprintf(ls_name, MAX_NAME, "%s%s", LVM_LS_PREFIX, vg_name);
	return 0;
}

/* FIXME: add mutex for gl_lsname_ ? */

static void gl_ls_name(char *ls_name)
{
	if (gl_use_dlm)
		memcpy(ls_name, gl_lsname_dlm, MAX_NAME);
	else if (gl_use_sanlock)
		memcpy(ls_name, gl_lsname_sanlock, MAX_NAME);
	else
		memset(ls_name, 0, MAX_NAME);
}

/*
 * When this function returns an error, the caller needs to deal
 * with act (in the cases where act exists).
 */

static int add_lockspace_thread(const char *ls_name,
				const char *vg_name,
				const char *vg_uuid,
				int lm_type, const char *vg_args,
				struct action *act)
{
	struct lockspace *ls, *ls2;
	struct resource *r;
	int rv;

	log_debug("add_lockspace_thread %s %s version %u",
		  lm_str(lm_type), ls_name, act ? act->version : 0);

	if (!(ls = alloc_lockspace()))
		return -ENOMEM;

	strncpy(ls->name, ls_name, MAX_NAME);
	ls->lm_type = lm_type;

	if (act)
		ls->start_client_id = act->client_id;

	if (vg_uuid)
		strncpy(ls->vg_uuid, vg_uuid, 64);

	if (vg_name)
		strncpy(ls->vg_name, vg_name, MAX_NAME);

	if (vg_args)
		strncpy(ls->vg_args, vg_args, MAX_ARGS);

	if (act)
		ls->host_id = act->host_id;

	if (!(r = alloc_resource())) {
		free(ls);
		return -ENOMEM;
	}

	r->type = LD_RT_VG;
	r->mode = LD_LK_UN;
	r->use_vb = 1;
	strncpy(r->name, R_NAME_VG, MAX_NAME);
	list_add_tail(&r->list, &ls->resources);

	pthread_mutex_lock(&lockspaces_mutex);
	ls2 = find_lockspace_name(ls->name);
	if (ls2) {
		if (ls2->thread_stop) {
			log_debug("add_lockspace_thread %s exists and stopping", ls->name);
			rv = -EAGAIN;
		} else {
			log_debug("add_lockspace_thread %s exists", ls->name);
			rv = -EEXIST;
		}
		pthread_mutex_unlock(&lockspaces_mutex);
		free_resource(r);
		free(ls);
		return rv;
	}

	/*
	 * act will be null when this lockspace is added automatically/internally
	 * and not by an explicit client action that wants a result.
	 */
	if (act)
		list_add(&act->list, &ls->actions);

	if (ls->lm_type == LD_LM_DLM && !strcmp(ls->name, gl_lsname_dlm))
		global_dlm_lockspace_exists = 1;
	list_add_tail(&ls->list, &lockspaces);
	pthread_mutex_unlock(&lockspaces_mutex);

	rv = pthread_create(&ls->thread, NULL, lockspace_thread_main, ls);
	if (rv < 0) {
		log_error("add_lockspace_thread %s pthread error %d %d", ls->name, rv, errno);
		pthread_mutex_lock(&lockspaces_mutex);
		list_del(&ls->list);
		pthread_mutex_unlock(&lockspaces_mutex);
		free_resource(r);
		free(ls);
		return rv;
	}

	return 0;
}

/*
 * There is no add_sanlock_global_lockspace or
 * rem_sanlock_global_lockspace because with sanlock,
 * the global lockspace is one of the vg lockspaces.
 */

static int add_dlm_global_lockspace(struct action *act)
{
	int rv;

	if (global_dlm_lockspace_exists)
		return 0;

	/*
	 * FIXME: if the dlm global lockspace is started without a global
	 * lock request, insert an internal gl sh lock request?
	 */

	rv = add_lockspace_thread(gl_lsname_dlm, NULL, NULL, LD_LM_DLM, NULL, act);
	if (rv < 0)
		log_debug("add_dlm_global_lockspace add_lockspace_thread %d", rv);

	/*
	 * EAGAIN may be returned for a short period because
	 * global_dlm_lockspace_exists is set to 0 before the
	 * ls is removed from the lockspaces list by the
	 * worker_thread.
	 */

	return rv;
}

/*
 * If dlm gl lockspace is the only one left, then stop it.
 * This is not used for an explicit rem_lockspace action from
 * the client, only for auto remove.
 */

static int rem_dlm_global_lockspace(void)
{
	struct lockspace *ls, *ls_gl = NULL;
	int others = 0;
	int rv = 0;

	pthread_mutex_lock(&lockspaces_mutex);
	list_for_each_entry(ls, &lockspaces, list) {
		if (!strcmp(ls->name, gl_lsname_dlm)) {
			ls_gl = ls;
			continue;
		}
		if (ls->thread_stop)
			continue;
		others++;
		break;
	}

	if (others) {
		rv = -EAGAIN;
		goto out;
	}

	if (!ls_gl) {
		rv = -ENOENT;
		goto out;
	}

	ls = ls_gl;
	pthread_mutex_lock(&ls->mutex);
	ls->thread_stop = 1;
	ls->thread_work = 1;
	pthread_cond_signal(&ls->cond);
	pthread_mutex_unlock(&ls->mutex);
	rv = 0;
out:
	pthread_mutex_unlock(&lockspaces_mutex);
	return rv;
}

/*
 * When the first dlm lockspace is added for a vg, automatically add a separate
 * dlm lockspace for the global lock.
 *
 * For sanlock, a separate lockspace is not used for the global lock, but the
 * gl lock lives in a vg lockspace, (although it's recommended to create a
 * special vg dedicated to holding the gl).
 */

static int add_lockspace(struct action *act)
{
	char ls_name[MAX_NAME+1];
	int rv;

	memset(ls_name, 0, sizeof(ls_name));

	/*
	 * FIXME: I don't think this is used any more.
	 * Remove it, or add the ability to start the global
	 * dlm lockspace using lvmlockctl?
	 */
	if (act->rt == LD_RT_GL) {
		if (gl_use_dlm) {
			rv = add_dlm_global_lockspace(act);
			return rv;
		} else {
			return -EINVAL;
		}
	}

	if (act->rt == LD_RT_VG) {
		if (gl_use_dlm)
			add_dlm_global_lockspace(NULL);

		vg_ls_name(act->vg_name, ls_name);

		rv = add_lockspace_thread(ls_name, act->vg_name, act->vg_uuid,
					  act->lm_type, act->vg_args,
					  act);
		if (rv)
			log_debug("add_lockspace %s add_lockspace_thread %d", ls_name, rv);
		return rv;
	}

	log_error("add_lockspace bad type %d", act->rt);
	return -1;
}

/*
 * vgchange --lock-stop vgname will lock the vg ex, then send a stop,
 * so we exect to find the ex vg lock held here, and will automatically
 * unlock it when stopping.
 *
 * Should we attempt to stop the lockspace containing the gl last?
 */

static int rem_lockspace(struct action *act)
{
	struct lockspace *ls;
	char ls_name[MAX_NAME+1];
	int force = act->flags & LD_AF_FORCE;
	int rt = act->rt;

	if (act->rt == LD_RT_GL && act->lm_type != LD_LM_DLM)
		return -EINVAL;

	memset(ls_name, 0, sizeof(ls_name));

	if (act->rt == LD_RT_GL)
		gl_ls_name(ls_name);
	else
		vg_ls_name(act->vg_name, ls_name);

	pthread_mutex_lock(&lockspaces_mutex);
	ls = find_lockspace_name(ls_name);
	if (!ls) {
		pthread_mutex_unlock(&lockspaces_mutex);
		return -ENOLS;
	}

	pthread_mutex_lock(&ls->mutex);
	if (ls->thread_stop) {
		pthread_mutex_unlock(&ls->mutex);
		pthread_mutex_unlock(&lockspaces_mutex);
		return -ESTALE;
	}

	if (!force && for_each_lock(ls, LOCKS_EXIST_LV)) {
		pthread_mutex_unlock(&ls->mutex);
		pthread_mutex_unlock(&lockspaces_mutex);
		return -EBUSY;
	}
	ls->thread_work = 1;
	ls->thread_stop = 1;
	list_add_tail(&act->list, &ls->actions);
	pthread_cond_signal(&ls->cond);
	pthread_mutex_unlock(&ls->mutex);
	pthread_mutex_unlock(&lockspaces_mutex);

	/*
	 * The dlm global lockspace was automatically added when
	 * the first dlm vg lockspace was added, now reverse that
	 * by automatically removing the dlm global lockspace when
	 * the last dlm vg lockspace is removed.
	 */

	if (rt == LD_RT_VG && gl_use_dlm)
		rem_dlm_global_lockspace();

	return 0;
}

/*
 * count how many lockspaces started by this client are still starting;
 * the client will use this to wait for all its start operations to finish
 * (START_WAIT).
 */

static int count_lockspace_starting(uint32_t client_id)
{
	struct lockspace *ls;
	int count = 0;
	int done = 0;
	int fail = 0;

	pthread_mutex_lock(&lockspaces_mutex);
	list_for_each_entry(ls, &lockspaces, list) {
		if (ls->start_client_id != client_id)
			continue;

		if (!ls->create_done && !ls->create_fail) {
			count++;
			continue;
		}

		if (ls->create_done)
			done++;
		if (ls->create_fail)
			fail++;
	}
	pthread_mutex_unlock(&lockspaces_mutex);

	log_debug("count_lockspace_starting client %u count %d done %d fail %d",
		  client_id, count, done, fail);

	return count;
}

/*
 * Loop through all lockspaces, and:
 * - if do_stop is set, stop any that are not stopped
 * - if do_free is set, join any that are done stopping (and free ls)
 *
 * do_stop will not stop an ls with lv locks unless force is set.
 *
 * This function does not block or wait for anything.
 *
 * do_stop (no do_free):
 * returns count of lockspaces that need stop (have locks and no force)
 *
 * do_free (no do_stop):
 * returns count of lockspaces that are stopped and need freeing
 *
 * do_stop and do_free:
 * returns sum of the previous two
 */

static int for_each_lockspace(int do_stop, int do_free, int do_force)
{
	struct lockspace *ls, *safe;
	int need_stop = 0;
	int need_free = 0;
	int stop_count = 0;
	int free_count = 0;
	int done;
	int stop;
	int perrno;

	pthread_mutex_lock(&lockspaces_mutex);

	if (do_stop) {
		list_for_each_entry(ls, &lockspaces, list) {

			pthread_mutex_lock(&ls->mutex);
			if (ls->thread_stop) {
				pthread_mutex_unlock(&ls->mutex);
				continue;
			}

			if (!do_force && for_each_lock(ls, LOCKS_EXIST_ANY)) {
				need_stop++;
			} else {
				ls->thread_work = 1;
				ls->thread_stop = 1;
				pthread_cond_signal(&ls->cond);
				stop_count++;
			}
			pthread_mutex_unlock(&ls->mutex);
		}
	}

	if (do_free) {
		list_for_each_entry_safe(ls, safe, &lockspaces, list) {

			pthread_mutex_lock(&ls->mutex);
			done = ls->thread_done;
			stop = ls->thread_stop;
			pthread_mutex_unlock(&ls->mutex);

			/* This ls has locks and force is not set. */
			if (!stop)
				continue;

			/*
			 * Once thread_done is set, we know that the lockspace_thread
			 * will not be using/touching the ls struct.  Any other
			 * thread touches the ls struct under lockspaces_mutex.
			 */
			if (done) {
				if ((perrno = pthread_join(ls->thread, NULL)))
					log_error("pthread_join error %d", perrno);

				list_del(&ls->list);

				/* FIXME: will free_vg ever not be set? */

				log_debug("free ls %s", ls->name);

				if (ls->free_vg) {
					/* In future we may need to free ls->actions here */
					free_ls_resources(ls);
					free(ls);
					free_count++;
				}
			} else {
				need_free++;
			}
		}
	}

	if (list_empty(&lockspaces)) {
		if (!gl_type_static) {
			gl_use_dlm = 0;
			gl_use_sanlock = 0;
		}
	}
	pthread_mutex_unlock(&lockspaces_mutex);

	if (stop_count || free_count || need_stop || need_free) {
		log_debug("for_each_lockspace do_stop %d do_free %d "
			  "stop_count %d free_count %d need_stop %d need_free %d",
			  do_stop, do_free, stop_count, free_count, need_stop, need_free);
	}

	return need_stop + need_free;
}

/*
 * This is only called when the daemon is exiting so the sleep/retry
 * loop doesn't have any adverse impact.
 */

static void for_each_lockspace_retry(int do_stop, int do_free, int do_force)
{
	int count;

	while (1) {
		count = for_each_lockspace(do_stop, do_free, do_force);
		if (!count)
			break;

		log_debug("for_each_lockspace_retry remaining %d", count);
		sleep(1);
	}
}

static int work_init_vg(struct action *act)
{
	struct lockspace *ls;
	char ls_name[MAX_NAME+1];
	int rv = 0;

	memset(ls_name, 0, sizeof(ls_name));

	vg_ls_name(act->vg_name, ls_name);

	/*
	 * The max dlm ls name is 64 and the max sanlock ls name is 48.  So,
	 * after the "lvm_" prefix, only the first 60/44 characters of the VG
	 * name are used for the lockspace name.  This will cause a collision
	 * in the lock manager if two different VG names have the first 60/44
	 * chars in common.  At the time of vgcreate (here), check if any other
	 * VG's are known that would collide.  If the collision is not detected
	 * at vgcreate time, it will be detected at start time and add_lockspace
	 * will fail for the second of the two matching ls names.
	 */
	pthread_mutex_lock(&lockspaces_mutex);
	list_for_each_entry(ls, &lockspaces, list) {
		if ((ls->lm_type == LD_LM_SANLOCK) && !strncmp(ls->name, ls_name, 48)) {
			rv = -EEXIST;
			break;
		}
		if ((ls->lm_type == LD_LM_DLM) && !strcmp(ls->name, ls_name)) {
			rv = -EEXIST;
			break;
		}
	}
	pthread_mutex_unlock(&lockspaces_mutex);

	if (rv == -EEXIST) {
		log_error("Existing lockspace name %s matches new %s VG names %s %s",
			  ls->name, ls_name, ls->vg_name, act->vg_name);
		return rv;
	}

	if (act->lm_type == LD_LM_SANLOCK)
		rv = lm_init_vg_sanlock(ls_name, act->vg_name, act->flags, act->vg_args);
	else if (act->lm_type == LD_LM_DLM)
		rv = lm_init_vg_dlm(ls_name, act->vg_name, act->flags, act->vg_args);
	else
		rv = -EINVAL;

	return rv;
}

static int work_rename_vg(struct action *act)
{
	char ls_name[MAX_NAME+1];
	int rv = 0;

	memset(ls_name, 0, sizeof(ls_name));

	vg_ls_name(act->vg_name, ls_name);

	if (act->lm_type == LD_LM_SANLOCK)
		rv = lm_rename_vg_sanlock(ls_name, act->vg_name, act->flags, act->vg_args);
	else if (act->lm_type == LD_LM_DLM)
		return 0;
	else
		rv = -EINVAL;

	return rv;
}

static void work_test_gl(void)
{
	struct lockspace *ls;
	int is_enabled = 0;

	pthread_mutex_lock(&lockspaces_mutex);
	list_for_each_entry(ls, &lockspaces, list) {
		if (ls->lm_type != LD_LM_SANLOCK)
			continue;

		pthread_mutex_lock(&ls->mutex);
		if (ls->create_done && !ls->thread_stop) {
			is_enabled = lm_gl_is_enabled(ls);
			if (is_enabled) {
				log_debug("S %s worker found gl_is_enabled", ls->name);
				strncpy(gl_lsname_sanlock, ls->name, MAX_NAME);
			}
		}
		pthread_mutex_unlock(&ls->mutex);

		if (is_enabled)
			break;
	}

	if (!is_enabled)
		log_debug("worker found no gl_is_enabled");
	pthread_mutex_unlock(&lockspaces_mutex);
}

static int work_init_lv(struct action *act)
{
	struct lockspace *ls;
	char ls_name[MAX_NAME+1];
	char vg_args[MAX_ARGS+1];
	char lv_args[MAX_ARGS+1];
	uint64_t free_offset = 0;
	int lm_type = 0;
	int rv = 0;

	memset(ls_name, 0, sizeof(ls_name));
	memset(vg_args, 0, sizeof(vg_args));
	memset(lv_args, 0, sizeof(lv_args));

	vg_ls_name(act->vg_name, ls_name);

	pthread_mutex_lock(&lockspaces_mutex);
	ls = find_lockspace_name(ls_name);
	if (ls) {
		lm_type = ls->lm_type;
		memcpy(vg_args, ls->vg_args, MAX_ARGS);
		free_offset = ls->free_lock_offset;
	}
	pthread_mutex_unlock(&lockspaces_mutex);

	if (!ls) {
		lm_type = act->lm_type;
		memcpy(vg_args, act->vg_args, MAX_ARGS);
	}

	if (act->lm_type != lm_type) {
		log_error("init_lv ls_name %s wrong lm_type %d %d",
			  ls_name, act->lm_type, lm_type);
		return -EINVAL;
	}

	if (lm_type == LD_LM_SANLOCK) {
		rv = lm_init_lv_sanlock(ls_name, act->vg_name, act->lv_uuid,
					vg_args, lv_args, free_offset);

		memcpy(act->lv_args, lv_args, MAX_ARGS);
		return rv;

	} else if (act->lm_type == LD_LM_DLM) {
		return 0;
	} else {
		log_error("init_lv ls_name %s bad lm_type %d", ls_name, act->lm_type);
		return -EINVAL;
	}
}

/*
 * When an action is queued for the worker_thread, it is processed right away.
 * After processing, some actions need to be retried again in a short while.
 * These actions are put on the delayed_list, and the worker_thread will
 * process these delayed actions again in SHORT_DELAY_PERIOD.
 */

#define SHORT_DELAY_PERIOD 2
#define LONG_DELAY_PERIOD 60

static void *worker_thread_main(void *arg_in)
{
	struct list_head delayed_list;
	struct timespec ts;
	struct action *act, *safe;
	uint64_t last_delayed_time = 0;
	int delay_sec = LONG_DELAY_PERIOD;
	int rv;

	INIT_LIST_HEAD(&delayed_list);

	while (1) {
		pthread_mutex_lock(&worker_mutex);
		if (clock_gettime(CLOCK_REALTIME, &ts)) {
			log_error("clock_gettime failed.");
			ts.tv_sec = ts.tv_nsec = 0;
		}
		ts.tv_sec += delay_sec;
		rv = 0;
		act = NULL;

		while (list_empty(&worker_list) && !worker_stop && !worker_wake && !rv) {
			rv = pthread_cond_timedwait(&worker_cond, &worker_mutex, &ts);
		}
		worker_wake = 0;

		if (worker_stop) {
			pthread_mutex_unlock(&worker_mutex);
			goto out;
		}

		if (!list_empty(&worker_list)) {
			act = list_first_entry(&worker_list, struct action, list);
			list_del(&act->list);
		}
		pthread_mutex_unlock(&worker_mutex);

		/*
		 * Do new work actions before processing delayed work actions.
		 */

		if (!act)
			goto delayed_work;

		if (act->op == LD_OP_RUNNING_LM) {
			int run_sanlock = lm_is_running_sanlock();
			int run_dlm = lm_is_running_dlm();

			if (daemon_test) {
				run_sanlock = gl_use_sanlock;
				run_dlm = gl_use_dlm;
			}

			if (run_sanlock && run_dlm)
				act->result = -EXFULL;
			else if (!run_sanlock && !run_dlm)
				act->result = -ENOLCK;
			else if (run_sanlock)
				act->result = LD_LM_SANLOCK;
			else if (run_dlm)
				act->result = LD_LM_DLM;
			add_client_result(act);

		} else if ((act->op == LD_OP_LOCK) && (act->flags & LD_AF_SEARCH_LS)) {
			/*
			 * worker_thread used as a helper to search existing
			 * sanlock vgs for an enabled gl.
			 */
			log_debug("work search for gl");
			work_test_gl();

			/* try again to find a gl lockspace for this act */
			rv = add_lock_action(act);
			if (rv < 0) {
				act->result = rv;
				add_client_result(act);
			}

		} else if ((act->op == LD_OP_INIT) && (act->rt == LD_RT_VG)) {
			log_debug("work init_vg %s", act->vg_name);
			act->result = work_init_vg(act);
			add_client_result(act);

		} else if ((act->op == LD_OP_INIT) && (act->rt == LD_RT_LV)) {
			log_debug("work init_lv %s/%s uuid %s", act->vg_name, act->lv_name, act->lv_uuid);
			act->result = work_init_lv(act);
			add_client_result(act);

		} else if ((act->op == LD_OP_RENAME_FINAL) && (act->rt == LD_RT_VG)) {
			log_debug("work rename_vg %s", act->vg_name);
			act->result = work_rename_vg(act);
			add_client_result(act);

		} else if (act->op == LD_OP_START_WAIT) {
			act->result = count_lockspace_starting(act->client_id);
			if (!act->result)
				add_client_result(act);
			else
				list_add(&act->list, &delayed_list);

		} else if (act->op == LD_OP_STOP_ALL) {
			act->result = for_each_lockspace(DO_STOP, DO_FREE, (act->flags & LD_AF_FORCE) ? DO_FORCE : NO_FORCE);
			if (!act->result || !(act->flags & LD_AF_WAIT))
				add_client_result(act);
			else
				list_add(&act->list, &delayed_list);

		} else {
			log_error("work unknown op %d", act->op);
			act->result = -EINVAL;
			add_client_result(act);
		}

 delayed_work:
		/*
		 * We may want to track retry times per action so that
		 * we can delay different actions by different amounts.
		 */

		if (monotime() - last_delayed_time < SHORT_DELAY_PERIOD) {
			delay_sec = 1;
			continue;
		}
		last_delayed_time = monotime();

		list_for_each_entry_safe(act, safe, &delayed_list, list) {
			if (act->op == LD_OP_START_WAIT) {
				log_debug("work delayed start_wait for client %u", act->client_id);
				act->result = count_lockspace_starting(act->client_id);
				if (!act->result) {
					list_del(&act->list);
					add_client_result(act);
				}

			} else if (act->op == LD_OP_STOP_ALL) {
				log_debug("work delayed stop_all");
				act->result = for_each_lockspace(DO_STOP, DO_FREE, (act->flags & LD_AF_FORCE) ? DO_FORCE : NO_FORCE);
				if (!act->result) {
					list_del(&act->list);
					act->result = 0;
					add_client_result(act);
				}
			}
		}

		/*
		 * This is not explicitly queued work, and not delayed work,
		 * but lockspace thread cleanup that's needed when a
		 * lockspace has been stopped/removed or failed to start.
		 */

		for_each_lockspace(NO_STOP, DO_FREE, NO_FORCE);

		if (list_empty(&delayed_list))
			delay_sec = LONG_DELAY_PERIOD;
		else
			delay_sec = 1;
	}
out:
	list_for_each_entry_safe(act, safe, &delayed_list, list) {
		list_del(&act->list);
		free_action(act);
	}

	pthread_mutex_lock(&worker_mutex);
	list_for_each_entry_safe(act, safe, &worker_list, list) {
		list_del(&act->list);
		free_action(act);
	}
	pthread_mutex_unlock(&worker_mutex);
	return NULL;
}

static int setup_worker_thread(void)
{
	int rv;

	INIT_LIST_HEAD(&worker_list);

	pthread_mutex_init(&worker_mutex, NULL);
	pthread_cond_init(&worker_cond, NULL);

	rv = pthread_create(&worker_thread, NULL, worker_thread_main, NULL);
	if (rv)
		return -1;
	return 0;
}

static void close_worker_thread(void)
{
	int perrno;

	pthread_mutex_lock(&worker_mutex);
	worker_stop = 1;
	pthread_cond_signal(&worker_cond);
	pthread_mutex_unlock(&worker_mutex);

	if ((perrno = pthread_join(worker_thread, NULL)))
		log_error("pthread_join worker_thread error %d", perrno);
}

/* client_mutex is locked */
static struct client *find_client_work(void)
{
	struct client *cl;

	list_for_each_entry(cl, &client_list, list) {
		if (cl->recv || cl->dead)
			return cl;
	}
	return NULL;
}

/* client_mutex is locked */
static struct client *find_client_id(uint32_t id)
{
	struct client *cl;

	list_for_each_entry(cl, &client_list, list) {
		if (cl->id == id)
			return cl;
	}
	return NULL;
}

/* client_mutex is locked */
static struct client *find_client_pi(int pi)
{
	struct client *cl;

	list_for_each_entry(cl, &client_list, list) {
		if (cl->pi == pi)
			return cl;
	}
	return NULL;
}

/*
 * wake up poll() because we have added an fd
 * back into pollfd and poll() needs to be restarted
 * to recognize it.
 */
static void restart_poll(void)
{
	int rv;
	rv = write(restart_fds[1], "w", 1);
	if (!rv || rv < 0)
		log_debug("restart_poll write %d", errno);
}

/* poll will take requests from client again, cl->mutex must be held */
static void client_resume(struct client *cl)
{
	if (cl->dead)
		return;

	if (!cl->poll_ignore || cl->fd == -1 || cl->pi == -1) {
		/* shouldn't happen */
		log_error("client_resume %u bad state ig %d fd %d pi %d",
			  cl->id, cl->poll_ignore, cl->fd, cl->pi);
		return;
	}

	pthread_mutex_lock(&pollfd_mutex);
	if (pollfd[cl->pi].fd != POLL_FD_IGNORE) {
		log_error("client_resume %u pi %d fd %d not IGNORE",
			  cl->id, cl->pi, cl->fd);
	}
	pollfd[cl->pi].fd = cl->fd;
	pollfd[cl->pi].events = POLLIN;
	pthread_mutex_unlock(&pollfd_mutex);

	restart_poll();
}

/* called from client_thread, cl->mutex is held */
static int client_send_result(struct client *cl, struct action *act)
{
	response res;
	char result_flags[128];
	int dump_len = 0;
	int dump_fd = -1;
	int rv = 0;

	if (cl->dead) {
		log_debug("send cl %u skip dead", cl->id);
		return -1;
	}

	memset(result_flags, 0, sizeof(result_flags));

	buffer_init(&res.buffer);

	/*
	 * EUNATCH is returned when the global lock existed,
	 * but had been disabled when we tried to lock it,
	 * so we removed it, and no longer have a gl to lock.
	 */

	if (act->result == -EUNATCH)
		act->result = -ENOLS;

	/*
	 * init_vg with dlm|sanlock returns vg_args
	 * init_lv with sanlock returns lv_args
	 */

	if (act->result == -ENOLS) {
		/*
		 * The lockspace could not be found, in which case
		 * the caller may want to know if any lockspaces exist
		 * or if lockspaces exist, but not one with the global lock.
		 * Given this detail, it may be able to procede without
		 * the lock.
		 */
		pthread_mutex_lock(&lockspaces_mutex);
		if (list_empty(&lockspaces))
			strcat(result_flags, "NO_LOCKSPACES,");
		pthread_mutex_unlock(&lockspaces_mutex);

		if (gl_use_sanlock) {
			if (!gl_lsname_sanlock[0])
				strcat(result_flags, "NO_GL_LS,");
		} else if (gl_use_dlm) {
			if (!gl_lsname_dlm[0])
				strcat(result_flags, "NO_GL_LS,");
		} else {
			int found_lm = 0;

			if (lm_support_dlm() && lm_is_running_dlm())
				found_lm++;
			if (lm_support_sanlock() && lm_is_running_sanlock())
				found_lm++;

			if (!found_lm)
				strcat(result_flags, "NO_GL_LS,NO_LM");
			else
				strcat(result_flags, "NO_GL_LS");
		}
	}

	if (act->flags & LD_AF_DUP_GL_LS)
		strcat(result_flags, "DUP_GL_LS,");

	if ((act->flags & LD_AF_WARN_GL_REMOVED) || gl_vg_removed)
		strcat(result_flags, "WARN_GL_REMOVED,");
	
	if (act->op == LD_OP_INIT) {
		/*
		 * init is a special case where lock args need
		 * to be passed back to the client.
		 */
		const char *vg_args = "none";
		const char *lv_args = "none";

		if (act->vg_args[0])
			vg_args = act->vg_args;

		if (act->lv_args[0])
			lv_args = act->lv_args;

		log_debug("send %s[%d] cl %u %s %s rv %d vg_args %s lv_args %s",
			  cl->name[0] ? cl->name : "client", cl->pid, cl->id,
			  op_str(act->op), rt_str(act->rt),
			  act->result, vg_args ? vg_args : "", lv_args ? lv_args : "");

		res = daemon_reply_simple("OK",
					  "op = " FMTd64, (int64_t)act->op,
					  "op_result = " FMTd64, (int64_t) act->result,
					  "lm_result = " FMTd64, (int64_t) act->lm_rv,
					  "vg_lock_args = %s", vg_args,
					  "lv_lock_args = %s", lv_args,
					  "result_flags = %s", result_flags[0] ? result_flags : "none",
					  NULL);

	} else if (act->op == LD_OP_DUMP_LOG || act->op == LD_OP_DUMP_INFO) {
		/*
		 * lvmlockctl creates the unix socket then asks us to write to it.
		 * FIXME: move processing this to a new dedicated query thread to
		 * avoid having a large data dump interfere with normal operation
		 * of the client thread?
		 */

		dump_fd = setup_dump_socket();
		if (dump_fd < 0)
			act->result = dump_fd;
		else if (act->op == LD_OP_DUMP_LOG)
			act->result = dump_log(&dump_len);
		else if (act->op == LD_OP_DUMP_INFO)
			act->result = dump_info(&dump_len);
		else
			act->result = -EINVAL;

		log_debug("send %s[%d] cl %u dump result %d dump_len %d",
			  cl->name[0] ? cl->name : "client", cl->pid, cl->id,
			  act->result, dump_len);

		res = daemon_reply_simple("OK",
					  "result = " FMTd64, (int64_t) act->result,
					  "dump_len = " FMTd64, (int64_t) dump_len,
					  NULL);
	} else {
		/*
		 * A normal reply.
		 */

		log_debug("send %s[%d] cl %u %s %s rv %d %s %s",
			  cl->name[0] ? cl->name : "client", cl->pid, cl->id,
			  op_str(act->op), rt_str(act->rt),
			  act->result, (act->result == -ENOLS) ? "ENOLS" : "", result_flags);

		res = daemon_reply_simple("OK",
					  "op = " FMTd64, (int64_t) act->op,
					  "lock_type = %s", lm_str(act->lm_type),
					  "op_result = " FMTd64, (int64_t) act->result,
					  "lm_result = " FMTd64, (int64_t) act->lm_rv,
					  "result_flags = %s", result_flags[0] ? result_flags : "none",
					  NULL);
	}

	if (!buffer_write(cl->fd, &res.buffer)) {
		rv = -errno;
		if (rv >= 0)
			rv = -1;
		log_debug("send cl %u fd %d error %d", cl->id, cl->fd, rv);
	}

	buffer_destroy(&res.buffer);

	client_resume(cl);

	if (dump_fd >= 0) {
		/* To avoid deadlock, send data here after the reply. */
		send_dump_buf(dump_fd, dump_len);
		if (close(dump_fd))
			log_error("failed to close dump socket %d", dump_fd);
	}

	return rv;
}

/* called from client_thread */
static void client_purge(struct client *cl)
{
	struct lockspace *ls;
	struct action *act;

	/*
	 * If the client made no lock requests, there can be
	 * no locks to release for it.
	 */
	if (!cl->lock_ops)
		return;

	pthread_mutex_lock(&lockspaces_mutex);
	list_for_each_entry(ls, &lockspaces, list) {
		if (!(act = alloc_action()))
			continue;

		act->op = LD_OP_CLOSE;
		act->client_id = cl->id;

		pthread_mutex_lock(&ls->mutex);
		if (!ls->thread_stop) {
			list_add_tail(&act->list, &ls->actions);
			ls->thread_work = 1;
			pthread_cond_signal(&ls->cond);
		} else {
			free_action(act);
		}
		pthread_mutex_unlock(&ls->mutex);
	}
	pthread_mutex_unlock(&lockspaces_mutex);
}

static int add_lock_action(struct action *act)
{
	struct lockspace *ls = NULL;
	char ls_name[MAX_NAME+1];

	memset(ls_name, 0, sizeof(ls_name));

	/*
	 * Determine which lockspace this action is for, and set ls_name.
	 */

	if (act->rt == LD_RT_GL) {
		/* Global lock is requested */
		if (gl_use_sanlock && (act->op == LD_OP_ENABLE || act->op == LD_OP_DISABLE)) {
			vg_ls_name(act->vg_name, ls_name);
		} else {
			if (!gl_use_dlm && !gl_use_sanlock) {
				if (lm_is_running_dlm())
					gl_use_dlm = 1;
				else if (lm_is_running_sanlock())
					gl_use_sanlock = 1;
			}
			gl_ls_name(ls_name);
		}
	} else {
		/* VG lock is requested */
		vg_ls_name(act->vg_name, ls_name);
	}

 retry:
	pthread_mutex_lock(&lockspaces_mutex);
	if (ls_name[0])
		ls = find_lockspace_name(ls_name);
	if (!ls) {
		pthread_mutex_unlock(&lockspaces_mutex);

		if (act->op == LD_OP_UPDATE && act->rt == LD_RT_VG) {
			log_debug("lockspace \"%s\" not found ignored for vg update", ls_name);
			return -ENOLS;

		} else if (act->flags & LD_AF_SEARCH_LS) {
			/*
			 * Fail if we've already tried searching for the lockspace.
			 */
			log_debug("lockspace \"%s\" not found after search", ls_name);
			return -ENOLS;

		} else if (act->op == LD_OP_LOCK && act->rt == LD_RT_GL && gl_use_sanlock) {
			/*
			 * The sanlock global lock may have been enabled in an existing VG,
			 * so search existing VGs for an enabled global lock.
			 */
			log_debug("lockspace \"%s\" not found for sanlock gl, searching...", ls_name);
			act->flags |= LD_AF_SEARCH_LS;
			add_work_action(act);
			return 0;

		} else if (act->op == LD_OP_LOCK && act->rt == LD_RT_GL && act->mode != LD_LK_UN && gl_use_dlm) {
			/*
			 * Automatically start the dlm global lockspace when
			 * a command tries to acquire the global lock.
			 */
			log_debug("lockspace \"%s\" not found for dlm gl, adding...", ls_name);
			act->flags |= LD_AF_SEARCH_LS;
			act->flags |= LD_AF_WAIT_STARTING;
			add_dlm_global_lockspace(NULL);
			goto retry;

		} else if (act->op == LD_OP_LOCK && act->mode == LD_LK_UN) {
			log_debug("lockspace \"%s\" not found for unlock ignored", ls_name);
			return -ENOLS;

		} else {
			log_debug("lockspace \"%s\" not found", ls_name);
			return -ENOLS;
		}
	}

	if (act->lm_type == LD_LM_NONE) {
		/* return to the command the type we are using */
		act->lm_type = ls->lm_type;
	} else if (act->lm_type != ls->lm_type) {
		/* should not happen */
		log_error("S %s add_lock_action bad lm_type %d ls %d",
			  ls_name, act->lm_type, ls->lm_type);
		pthread_mutex_unlock(&lockspaces_mutex);
		return -EINVAL;
	}

	pthread_mutex_lock(&ls->mutex);
	if (ls->thread_stop) {
		pthread_mutex_unlock(&ls->mutex);
		pthread_mutex_unlock(&lockspaces_mutex);
		log_error("lockspace is stopping %s", ls_name);
		return -ESTALE;
	}

	if (!ls->create_fail && !ls->create_done && !(act->flags & LD_AF_WAIT_STARTING)) {
		pthread_mutex_unlock(&ls->mutex);
		pthread_mutex_unlock(&lockspaces_mutex);
		log_debug("lockspace is starting %s", ls_name);
		return -ESTARTING;
	}

	list_add_tail(&act->list, &ls->actions);
	ls->thread_work = 1;
	pthread_cond_signal(&ls->cond);
	pthread_mutex_unlock(&ls->mutex);
	pthread_mutex_unlock(&lockspaces_mutex);

	/* lockspace_thread_main / res_process take it from here */

	return 0;
}

static int str_to_op_rt(const char *req_name, int *op, int *rt)
{
	if (!req_name)
		goto out;

	if (!strcmp(req_name, "hello")) {
		*op = LD_OP_HELLO;
		*rt = 0;
		return 0;
	}
	if (!strcmp(req_name, "quit")) {
		*op = LD_OP_QUIT;
		*rt = 0;
		return 0;
	}
	if (!strcmp(req_name, "info")) {
		*op = LD_OP_DUMP_INFO;
		*rt = 0;
		return 0;
	}
	if (!strcmp(req_name, "dump")) {
		*op = LD_OP_DUMP_LOG;
		*rt = 0;
		return 0;
	}
	if (!strcmp(req_name, "init_vg")) {
		*op = LD_OP_INIT;
		*rt = LD_RT_VG;
		return 0;
	}
	if (!strcmp(req_name, "init_lv")) {
		*op = LD_OP_INIT;
		*rt = LD_RT_LV;
		return 0;
	}
	if (!strcmp(req_name, "free_vg")) {
		*op = LD_OP_FREE;
		*rt = LD_RT_VG;
		return 0;
	}
	if (!strcmp(req_name, "busy_vg")) {
		*op = LD_OP_BUSY;
		*rt = LD_RT_VG;
		return 0;
	}
	if (!strcmp(req_name, "free_lv")) {
		*op = LD_OP_FREE;
		*rt = LD_RT_LV;
		return 0;
	}
	if (!strcmp(req_name, "start_vg")) {
		*op = LD_OP_START;
		*rt = LD_RT_VG;
		return 0;
	}
	if (!strcmp(req_name, "stop_vg")) {
		*op = LD_OP_STOP;
		*rt = LD_RT_VG;
		return 0;
	}
	if (!strcmp(req_name, "start_wait")) {
		*op = LD_OP_START_WAIT;
		*rt = 0;
		return 0;
	}
	if (!strcmp(req_name, "stop_all")) {
		*op = LD_OP_STOP_ALL;
		*rt = 0;
		return 0;
	}
	if (!strcmp(req_name, "lock_gl")) {
		*op = LD_OP_LOCK;
		*rt = LD_RT_GL;
		return 0;
	}
	if (!strcmp(req_name, "lock_vg")) {
		*op = LD_OP_LOCK;
		*rt = LD_RT_VG;
		return 0;
	}
	if (!strcmp(req_name, "lock_lv")) {
		*op = LD_OP_LOCK;
		*rt = LD_RT_LV;
		return 0;
	}
	if (!strcmp(req_name, "vg_update")) {
		*op = LD_OP_UPDATE;
		*rt = LD_RT_VG;
		return 0;
	}
	if (!strcmp(req_name, "enable_gl")) {
		*op = LD_OP_ENABLE;
		*rt = LD_RT_GL;
		return 0;
	}
	if (!strcmp(req_name, "disable_gl")) {
		*op = LD_OP_DISABLE;
		*rt = LD_RT_GL;
		return 0;
	}
	if (!strcmp(req_name, "rename_vg_before")) {
		*op = LD_OP_RENAME_BEFORE;
		*rt = LD_RT_VG;
		return 0;
	}
	if (!strcmp(req_name, "rename_vg_final")) {
		*op = LD_OP_RENAME_FINAL;
		*rt = LD_RT_VG;
		return 0;
	}
	if (!strcmp(req_name, "running_lm")) {
		*op = LD_OP_RUNNING_LM;
		*rt = 0;
		return 0;
	}
	if (!strcmp(req_name, "find_free_lock")) {
		*op = LD_OP_FIND_FREE_LOCK;
		*rt = LD_RT_VG;
		return 0;
	}
	if (!strcmp(req_name, "kill_vg")) {
		*op = LD_OP_KILL_VG;
		*rt = LD_RT_VG;
		return 0;
	}
	if (!strcmp(req_name, "drop_vg")) {
		*op = LD_OP_DROP_VG;
		*rt = LD_RT_VG;
		return 0;
	}
out:
	return -1;
}

static int str_to_mode(const char *str)
{
	if (!str)
		goto out;
	if (!strcmp(str, "un"))
		return LD_LK_UN;
	if (!strcmp(str, "nl"))
		return LD_LK_NL;
	if (!strcmp(str, "sh"))
		return LD_LK_SH;
	if (!strcmp(str, "ex"))
		return LD_LK_EX;
out:
	return LD_LK_IV;
}

static int str_to_lm(const char *str)
{
	if (!str || !strcmp(str, "none"))
		return LD_LM_NONE;
	if (!strcmp(str, "sanlock"))
		return LD_LM_SANLOCK;
	if (!strcmp(str, "dlm"))
		return LD_LM_DLM;
	return -2; 
}

static uint32_t str_to_opts(const char *str)
{
	uint32_t flags = 0;

	if (!str)
		goto out;
	if (strstr(str, "persistent"))
		flags |= LD_AF_PERSISTENT;
	if (strstr(str, "unlock_cancel"))
		flags |= LD_AF_UNLOCK_CANCEL;
	if (strstr(str, "next_version"))
		flags |= LD_AF_NEXT_VERSION;
	if (strstr(str, "wait"))
		flags |= LD_AF_WAIT;
	if (strstr(str, "force"))
		flags |= LD_AF_FORCE;
	if (strstr(str, "ex_disable"))
		flags |= LD_AF_EX_DISABLE;
	if (strstr(str, "enable"))
		flags |= LD_AF_ENABLE;
	if (strstr(str, "disable"))
		flags |= LD_AF_DISABLE;
out:
	return flags;
}

/*
 * dump info
 * client_list: each client struct
 * lockspaces: each lockspace struct
 * lockspace actions: each action struct
 * lockspace resources: each resource struct
 * lockspace resource actions: each action struct
 * lockspace resource locks: each lock struct
 */

static int setup_dump_socket(void)
{
	int s;

	s = socket(AF_LOCAL, SOCK_DGRAM, 0);
	if (s < 0)
		return s;

	memset(&dump_addr, 0, sizeof(dump_addr));
	dump_addr.sun_family = AF_LOCAL;
	strcpy(&dump_addr.sun_path[1], DUMP_SOCKET_NAME);
	dump_addrlen = sizeof(sa_family_t) + strlen(dump_addr.sun_path+1) + 1;

	return s;
}

#define MAX_SEND_LEN 65536
#define RESEND_DELAY_US 1000
#define RESEND_DELAY_US_MAX 500000

static void send_dump_buf(int fd, int dump_len)
{
	int pos = 0;
	int ret;
	int send_len;
	int delay = 0;

	if (!dump_len)
		return;
repeat:
	if (dump_len - pos < MAX_SEND_LEN)
		send_len = dump_len - pos;
	else
		send_len = MAX_SEND_LEN;

	ret = sendto(fd, dump_buf + pos, send_len, MSG_NOSIGNAL | MSG_DONTWAIT,
		     (struct sockaddr *)&dump_addr, dump_addrlen);
	if (ret < 0) {
		if ((errno == EAGAIN || errno == EINTR) && (delay < RESEND_DELAY_US_MAX)) {
			usleep(RESEND_DELAY_US);
			delay += RESEND_DELAY_US;
			goto repeat;
		}
		log_error("send_dump_buf delay %d errno %d", delay, errno);
		return;
	}

	pos += ret;

	if (pos < dump_len)
		goto repeat;

	log_debug("send_dump_buf delay %d total %d", delay, pos);
}

static int print_structs(const char *prefix, int pos, int len)
{
	return snprintf(dump_buf + pos, len - pos,
			"info=%s "
			"unused_action_count=%d "
			"unused_client_count=%d "
			"unused_resource_count=%d "
			"unused_lock_count=%d\n",
			prefix,
			unused_action_count,
			unused_client_count,
			unused_resource_count,
			unused_lock_count);
}

static int print_client(struct client *cl, const char *prefix, int pos, int len)
{
	return snprintf(dump_buf + pos, len - pos,
			"info=%s "
			"pid=%d "
			"fd=%d "
			"pi=%d "
			"id=%u "
			"name=%s\n",
			prefix,
			cl->pid,
			cl->fd,
			cl->pi,
			cl->id,
			cl->name[0] ? cl->name : ".");
}

static int print_lockspace(struct lockspace *ls, const char *prefix, int pos, int len)
{
	return snprintf(dump_buf + pos, len - pos,
			"info=%s "
			"ls_name=%s "
			"vg_name=%s "
			"vg_uuid=%s "
			"vg_sysid=%s "
			"vg_args=%s "
			"lm_type=%s "
			"host_id=%llu "
			"create_fail=%d "
			"create_done=%d "
			"thread_work=%d "
			"thread_stop=%d "
			"thread_done=%d "
			"kill_vg=%d "
			"drop_vg=%d "
			"sanlock_gl_enabled=%d\n",
			prefix,
			ls->name,
			ls->vg_name,
			ls->vg_uuid,
			ls->vg_sysid[0] ? ls->vg_sysid : ".",
			ls->vg_args,
			lm_str(ls->lm_type),
			(unsigned long long)ls->host_id,
			ls->create_fail ? 1 : 0,
			ls->create_done ? 1 : 0,
			ls->thread_work ? 1 : 0,
			ls->thread_stop ? 1 : 0,
			ls->thread_done ? 1 : 0,
			ls->kill_vg,
			ls->drop_vg,
			ls->sanlock_gl_enabled ? 1 : 0);
}

static int print_action(struct action *act, const char *prefix, int pos, int len)
{
	return snprintf(dump_buf + pos, len - pos,
			"info=%s "
			"client_id=%u "
			"flags=0x%x "
			"version=%u "
			"op=%s "
			"rt=%s "
			"mode=%s "
			"lm_type=%s "
			"result=%d "
			"lm_rv=%d\n",
			prefix,
			act->client_id,
			act->flags,
			act->version,
			op_str(act->op),
			rt_str(act->rt),
			mode_str(act->mode),
			lm_str(act->lm_type),
			act->result,
			act->lm_rv);
}

static int print_resource(struct resource *r, const char *prefix, int pos, int len)
{
	return snprintf(dump_buf + pos, len - pos,
			"info=%s "
			"name=%s "
			"type=%s "
			"mode=%s "
			"sh_count=%d "
			"version=%u\n",
			prefix,
			r->name,
			rt_str(r->type),
			mode_str(r->mode),
			r->sh_count,
			r->version);
}

static int print_lock(struct lock *lk, const char *prefix, int pos, int len)
{
	return snprintf(dump_buf + pos, len - pos,
			"info=%s "
			"mode=%s "
			"version=%u "
			"flags=0x%x "
			"client_id=%u\n",
			prefix,
			mode_str(lk->mode),
			lk->version,
			lk->flags,
			lk->client_id);
}

static int dump_info(int *dump_len)
{
	struct client *cl;
	struct lockspace *ls;
	struct resource *r;
	struct lock *lk;
	struct action *act;
	int len, pos, ret;
	int rv = 0;

	memset(dump_buf, 0, sizeof(dump_buf));
	len = sizeof(dump_buf);
	pos = 0;

	/*
	 * memory
	 */

	pthread_mutex_lock(&unused_struct_mutex);
	ret = print_structs("structs", pos, len);
	if (ret >= len - pos) {
		pthread_mutex_unlock(&unused_struct_mutex);
		return -ENOSPC;
	}
	pos += ret;
	pthread_mutex_unlock(&unused_struct_mutex);

	/*
	 * clients
	 */

	pthread_mutex_lock(&client_mutex);
	list_for_each_entry(cl, &client_list, list) {
		ret = print_client(cl, "client", pos, len);
		if (ret >= len - pos) {
			 rv = -ENOSPC;
			 break;
		}
		pos += ret;
	}
	pthread_mutex_unlock(&client_mutex);

	if (rv < 0)
		return rv;

	/*
	 * lockspaces with their action/resource/lock info
	 */

	pthread_mutex_lock(&lockspaces_mutex);
	list_for_each_entry(ls, &lockspaces, list) {

		ret = print_lockspace(ls, "ls", pos, len);
		if (ret >= len - pos) {
			 rv = -ENOSPC;
			 goto out;
		}
		pos += ret;

		list_for_each_entry(act, &ls->actions, list) {
			ret = print_action(act, "ls_action", pos, len);
			if (ret >= len - pos) {
				rv = -ENOSPC;
				goto out;
			}
			pos += ret;
		}

		list_for_each_entry(r, &ls->resources, list) {
			ret = print_resource(r, "r", pos, len);
			if (ret >= len - pos) {
				rv = -ENOSPC;
				goto out;
			}
			pos += ret;

			list_for_each_entry(lk, &r->locks, list) {
				ret = print_lock(lk, "lk", pos, len);
				if (ret >= len - pos) {
					rv = -ENOSPC;
					goto out;
				}
				pos += ret;
			}

			list_for_each_entry(act, &r->actions, list) {
				ret = print_action(act, "r_action", pos, len);
				if (ret >= len - pos) {
					rv = -ENOSPC;
					goto out;
				}
				pos += ret;
			}
		}
	}
out:
	pthread_mutex_unlock(&lockspaces_mutex);

	*dump_len = pos;

	return rv;
}

/* called from client_thread, cl->mutex is held */
static void client_recv_action(struct client *cl)
{
	request req;
	response res;
	struct action *act;
	const char *cl_name;
	const char *vg_name;
	const char *vg_uuid;
	const char *vg_sysid;
	const char *str;
	int64_t val;
	uint32_t opts = 0;
	int result = 0;
	int cl_pid;
	int op, rt, lm, mode;
	int rv;

	buffer_init(&req.buffer);

	rv = buffer_read(cl->fd, &req.buffer);
	if (!rv) {
		if (errno == ECONNRESET) {
			log_debug("client recv %u ECONNRESET", cl->id);
			cl->dead = 1;
		} else {
			log_error("client recv %u buffer_read error %d", cl->id, errno);
		}
		buffer_destroy(&req.buffer);
		client_resume(cl);
		return;
	}

	req.cft = config_tree_from_string_without_dup_node_check(req.buffer.mem);
	if (!req.cft) {
		log_error("client recv %u config_from_string error", cl->id);
		buffer_destroy(&req.buffer);
		client_resume(cl);
		return;
	}

	str = daemon_request_str(req, "request", NULL);
	rv = str_to_op_rt(str, &op, &rt);
	if (rv < 0) {
		log_error("client recv %u bad request name \"%s\"", cl->id, str ? str : "");
		dm_config_destroy(req.cft);
		buffer_destroy(&req.buffer);
		client_resume(cl);
		return;
	}

	if (op == LD_OP_HELLO || op == LD_OP_QUIT) {

		/*
		 * FIXME: add the client command name to the hello messages
		 * so it can be saved in cl->name here.
		 */

		result = 0;

		if (op == LD_OP_QUIT) {
			log_debug("op quit");
			pthread_mutex_lock(&lockspaces_mutex);
		       	if (list_empty(&lockspaces))
				daemon_quit = 1;
			else
				result = -EBUSY;
			pthread_mutex_unlock(&lockspaces_mutex);
		}

		buffer_init(&res.buffer);

		res = daemon_reply_simple("OK",
					  "result = " FMTd64, (int64_t) result,
					  "protocol = %s", lvmlockd_protocol,
					  "version = " FMTd64, (int64_t) lvmlockd_protocol_version,
					  NULL);
		buffer_write(cl->fd, &res.buffer);
		buffer_destroy(&res.buffer);
		dm_config_destroy(req.cft);
		buffer_destroy(&req.buffer);
		client_resume(cl);
		return;
	}

	cl_name = daemon_request_str(req, "cmd", NULL);
	cl_pid = daemon_request_int(req, "pid", 0);
	vg_name = daemon_request_str(req, "vg_name", NULL);
	vg_uuid = daemon_request_str(req, "vg_uuid", NULL);
	vg_sysid = daemon_request_str(req, "vg_sysid", NULL);
	str = daemon_request_str(req, "mode", NULL);
	mode = str_to_mode(str);
	str = daemon_request_str(req, "opts", NULL);
	opts = str_to_opts(str);
	str = daemon_request_str(req, "vg_lock_type", NULL);
	lm = str_to_lm(str);

	if (cl_pid && cl_pid != cl->pid)
		log_error("client recv bad message pid %d client %d", cl_pid, cl->pid);

	/* FIXME: do this in hello message instead */
	if (!cl->name[0] && cl_name)
		strncpy(cl->name, cl_name, MAX_NAME);

	if (!gl_use_dlm && !gl_use_sanlock && (lm > 0)) {
		if (lm == LD_LM_DLM && lm_support_dlm())
			gl_use_dlm = 1;
		else if (lm == LD_LM_SANLOCK && lm_support_sanlock())
			gl_use_sanlock = 1;

		log_debug("set gl_use_%s", lm_str(lm));
	}

	if (!(act = alloc_action())) {
		log_error("No memory for action");
		dm_config_destroy(req.cft);
		buffer_destroy(&req.buffer);
		client_resume(cl);
		return;
	}

	act->client_id = cl->id;
	act->op = op;
	act->rt = rt;
	act->mode = mode;
	act->flags = opts;
	act->lm_type = lm;

	if (vg_name && strcmp(vg_name, "none"))
		strncpy(act->vg_name, vg_name, MAX_NAME);

	if (vg_uuid && strcmp(vg_uuid, "none"))
		strncpy(act->vg_uuid, vg_uuid, 64);

	if (vg_sysid && strcmp(vg_sysid, "none"))
		strncpy(act->vg_sysid, vg_sysid, MAX_NAME);

	str = daemon_request_str(req, "lv_name", NULL);
	if (str && strcmp(str, "none"))
		strncpy(act->lv_name, str, MAX_NAME);

	str = daemon_request_str(req, "lv_uuid", NULL);
	if (str && strcmp(str, "none"))
		strncpy(act->lv_uuid, str, MAX_NAME);

	val = daemon_request_int(req, "version", 0);
	if (val)
		act->version = (uint32_t)val;

	str = daemon_request_str(req, "vg_lock_args", NULL);
	if (str && strcmp(str, "none"))
		strncpy(act->vg_args, str, MAX_ARGS);

	str = daemon_request_str(req, "lv_lock_args", NULL);
	if (str && strcmp(str, "none"))
		strncpy(act->lv_args, str, MAX_ARGS);

	/* start_vg will include lvmlocal.conf local/host_id here */
	val = daemon_request_int(req, "host_id", 0);
	if (val)
		act->host_id = val;

	act->max_retries = daemon_request_int(req, "max_retries", DEFAULT_MAX_RETRIES);

	dm_config_destroy(req.cft);
	buffer_destroy(&req.buffer);

	log_debug("recv %s[%d] cl %u %s %s \"%s\" mode %s flags %x",
		  cl->name[0] ? cl->name : "client", cl->pid, cl->id,
		  op_str(act->op), rt_str(act->rt), act->vg_name, mode_str(act->mode), opts);

	if (lm == LD_LM_DLM && !lm_support_dlm()) {
		log_debug("dlm not supported");
		rv = -EPROTONOSUPPORT;
		goto out;
	}

	if (lm == LD_LM_SANLOCK && !lm_support_sanlock()) {
		log_debug("sanlock not supported");
		rv = -EPROTONOSUPPORT;
		goto out;
	}

	if (act->op == LD_OP_LOCK && act->mode != LD_LK_UN)
		cl->lock_ops = 1;

	switch (act->op) {
	case LD_OP_START:
		rv = add_lockspace(act);
		break;
	case LD_OP_STOP:
		rv = rem_lockspace(act);
		break;
	case LD_OP_DUMP_LOG:
	case LD_OP_DUMP_INFO:
		/* The client thread reply will copy and send the dump. */
		add_client_result(act);
		rv = 0;
		break;
	case LD_OP_INIT:
	case LD_OP_START_WAIT:
	case LD_OP_STOP_ALL:
	case LD_OP_RENAME_FINAL:
	case LD_OP_RUNNING_LM:
		add_work_action(act);
		rv = 0;
		break;
	case LD_OP_LOCK:
	case LD_OP_UPDATE:
	case LD_OP_ENABLE:
	case LD_OP_DISABLE:
	case LD_OP_FREE:
	case LD_OP_RENAME_BEFORE:
	case LD_OP_FIND_FREE_LOCK:
	case LD_OP_KILL_VG:
	case LD_OP_DROP_VG:
	case LD_OP_BUSY:
		rv = add_lock_action(act);
		break;
	default:
		rv = -EINVAL;
	};

out:
	if (rv < 0) {
		act->result = rv;
		add_client_result(act);
	}
}

static void *client_thread_main(void *arg_in)
{
	struct client *cl;
	struct action *act;
	struct action *act_un;
	int rv;

	while (1) {
		pthread_mutex_lock(&client_mutex);
		while (!client_work && list_empty(&client_results)) {
			if (client_stop) {
				pthread_mutex_unlock(&client_mutex);
				goto out;
			}
			pthread_cond_wait(&client_cond, &client_mutex);
		}

		/*
		 * Send outgoing results back to clients
		 */

		if (!list_empty(&client_results)) {
			act = list_first_entry(&client_results, struct action, list);
			list_del(&act->list);
			cl = find_client_id(act->client_id);
			pthread_mutex_unlock(&client_mutex);

			if (cl) {
				pthread_mutex_lock(&cl->mutex);
				rv = client_send_result(cl, act);
				pthread_mutex_unlock(&cl->mutex);
			} else {
				log_debug("no client %u for result", act->client_id);
				rv = -1;
			}

			/*
			 * The client failed after we acquired an LV lock for
			 * it, but before getting this reply saying it's done.
			 * So the lv will not be active and we should release
			 * the lv lock it requested.
			 */
			if ((rv < 0) && (act->flags & LD_AF_LV_LOCK)) {
				log_debug("auto unlock lv for failed client %u", act->client_id);
				if ((act_un = alloc_action())) {
					memcpy(act_un, act, sizeof(struct action));
					act_un->mode = LD_LK_UN;
					act_un->flags |= LD_AF_LV_UNLOCK;
					act_un->flags &= ~LD_AF_LV_LOCK;
					add_lock_action(act_un);
				}
			}

			free_action(act);
			continue;
		}

		/*
		 * Queue incoming actions for lockspace threads
		 */

		if (client_work) {
			cl = find_client_work();
			if (!cl)
				client_work = 0;
			pthread_mutex_unlock(&client_mutex);

			if (!cl)
				continue;

			pthread_mutex_lock(&cl->mutex);

			if (cl->recv) {
				cl->recv = 0;
				client_recv_action(cl);
			}

			if (cl->dead) {
				/*
				log_debug("client rem %d pi %d fd %d ig %d",
					  cl->id, cl->pi, cl->fd, cl->poll_ignore);
				*/

				/*
				 * If cl->dead was set in main_loop, then the
				 * fd has already been closed and the pollfd
				 * entry is already unused.
				 * main_loop set dead=1, ignore=0, pi=-1, fd=-1
				 *
				 * if cl->dead was not set in main_loop, but
				 * set in client_recv_action, then the main_loop
				 * should be ignoring this client fd.
				 * main_loop set ignore=1
				 */

				if (cl->poll_ignore) {
					log_debug("client close %d pi %d fd %d",
						  cl->id, cl->pi, cl->fd);
					/* assert cl->pi != -1 */
					/* assert pollfd[pi].fd == FD_IGNORE */
					if (close(cl->fd))
						log_error("client close %d pi %d fd %d failed",
							  cl->id, cl->pi, cl->fd);
					rem_pollfd(cl->pi);
					cl->pi = -1;
					cl->fd = -1;
					cl->poll_ignore = 0;
				} else {
					/* main thread should have closed */
					if (cl->pi != -1 || cl->fd != -1) {
						log_error("client %d bad state pi %d fd %d",
							  cl->id, cl->pi, cl->fd);
					}
				}
				pthread_mutex_unlock(&cl->mutex);

				pthread_mutex_lock(&client_mutex);
				list_del(&cl->list);
				pthread_mutex_unlock(&client_mutex);

				client_purge(cl);

				free_client(cl);
			} else {
				pthread_mutex_unlock(&cl->mutex);
			}
		} else
			pthread_mutex_unlock(&client_mutex);
	}
out:
	return NULL;
}

static int setup_client_thread(void)
{
	int rv;

	INIT_LIST_HEAD(&client_list);
	INIT_LIST_HEAD(&client_results);

	pthread_mutex_init(&client_mutex, NULL);
	pthread_cond_init(&client_cond, NULL);

	rv = pthread_create(&client_thread, NULL, client_thread_main, NULL);
	if (rv)
		return -1;
	return 0;
}

static void close_client_thread(void)
{
	int perrno;

	pthread_mutex_lock(&client_mutex);
	client_stop = 1;
	pthread_cond_signal(&client_cond);
	pthread_mutex_unlock(&client_mutex);

	if ((perrno = pthread_join(client_thread, NULL)))
		log_error("pthread_join client_thread error %d", perrno);
}

/*
 * Get a list of all VGs with a lockd type (sanlock|dlm).
 * We'll match this list against a list of existing lockspaces that are
 * found in the lock manager.
 *
 * For each of these VGs, also create a struct resource on ls->resources to
 * represent each LV in the VG that uses a lock.  For each of these LVs
 * that are active, we'll attempt to adopt a lock.
 */

static int get_lockd_vgs(struct list_head *vg_lockd)
{
	/* FIXME: get VGs some other way */
	return -1;
#if 0
	struct list_head update_vgs;
	daemon_reply reply;
	struct dm_config_node *cn;
	struct dm_config_node *metadata;
	struct dm_config_node *md_cn;
	struct dm_config_node *lv_cn;
	struct lockspace *ls, *safe;
	struct resource *r;
	const char *vg_name;
	const char *vg_uuid;
	const char *lv_uuid;
	const char *lock_type;
	const char *lock_args;
	char find_str_path[PATH_MAX];
	int rv = 0;

	INIT_LIST_HEAD(&update_vgs);

	reply = send_lvmetad("vg_list", "token = %s", "skip", NULL);

	if (reply.error || strcmp(daemon_reply_str(reply, "response", ""), "OK")) {
		log_error("vg_list from lvmetad failed %d", reply.error);
		rv = -EINVAL;
		goto destroy;
	}

	if (!(cn = dm_config_find_node(reply.cft->root, "volume_groups"))) {
		log_error("get_lockd_vgs no vgs");
		rv = -EINVAL;
		goto destroy;
	}

	/* create an update_vgs list of all vg uuids */

	for (cn = cn->child; cn; cn = cn->sib) {
		vg_uuid = cn->key;

		if (!(ls = alloc_lockspace())) {
			rv = -ENOMEM;
			break;
		}

		strncpy(ls->vg_uuid, vg_uuid, 64);
		list_add_tail(&ls->list, &update_vgs);
		log_debug("get_lockd_vgs %s", vg_uuid);
	}
 destroy:
	daemon_reply_destroy(reply);

	if (rv < 0)
		goto out;

	/* get vg_name and lock_type for each vg uuid entry in update_vgs */

	list_for_each_entry(ls, &update_vgs, list) {
		reply = send_lvmetad("vg_lookup",
				     "token = %s", "skip",
				     "uuid = %s", ls->vg_uuid,
				     NULL);

		if (reply.error || strcmp(daemon_reply_str(reply, "response", ""), "OK")) {
			log_error("vg_lookup from lvmetad failed %d", reply.error);
			rv = -EINVAL;
			goto next;
		}

		vg_name = daemon_reply_str(reply, "name", NULL);
		if (!vg_name) {
			log_error("get_lockd_vgs %s no name", ls->vg_uuid);
			rv = -EINVAL;
			goto next;
		}

		strncpy(ls->vg_name, vg_name, MAX_NAME);

		metadata = dm_config_find_node(reply.cft->root, "metadata");
		if (!metadata) {
			log_error("get_lockd_vgs %s name %s no metadata",
				  ls->vg_uuid, ls->vg_name);
			rv = -EINVAL;
			goto next;
		}

		lock_type = dm_config_find_str(metadata, "metadata/lock_type", NULL);
		ls->lm_type = str_to_lm(lock_type);

		if ((ls->lm_type != LD_LM_SANLOCK) && (ls->lm_type != LD_LM_DLM)) {
			log_debug("get_lockd_vgs %s not lockd type", ls->vg_name);
			continue;
		}

		lock_args = dm_config_find_str(metadata, "metadata/lock_args", NULL);
		if (lock_args)
			strncpy(ls->vg_args, lock_args, MAX_ARGS);

		log_debug("get_lockd_vgs %s lock_type %s lock_args %s",
			  ls->vg_name, lock_type, lock_args ?: "none");

		/*
		 * Make a record (struct resource) of each lv that uses a lock.
		 * For any lv that uses a lock, we'll check if the lv is active
		 * and if so try to adopt a lock for it.
		 */

		for (md_cn = metadata->child; md_cn; md_cn = md_cn->sib) {
			if (strcmp(md_cn->key, "logical_volumes"))
				continue;

			for (lv_cn = md_cn->child; lv_cn; lv_cn = lv_cn->sib) {
				snprintf(find_str_path, PATH_MAX, "%s/lock_args", lv_cn->key);
				lock_args = dm_config_find_str(lv_cn, find_str_path, NULL);
				if (!lock_args)
					continue;

				snprintf(find_str_path, PATH_MAX, "%s/id", lv_cn->key);
				lv_uuid = dm_config_find_str(lv_cn, find_str_path, NULL);

				if (!lv_uuid) {
					log_error("get_lock_vgs no lv id for name %s", lv_cn->key);
					continue;
				}

				if (!(r = alloc_resource())) {
					rv = -ENOMEM;
					goto next;
				}

				r->use_vb = 0;
				r->type = LD_RT_LV;
				strncpy(r->name, lv_uuid, MAX_NAME);
				if (lock_args)
					strncpy(r->lv_args, lock_args, MAX_ARGS);
				list_add_tail(&r->list, &ls->resources);
				log_debug("get_lockd_vgs %s lv %s %s (name %s)",
					  ls->vg_name, r->name, lock_args ? lock_args : "", lv_cn->key);
			}
		}
 next:
		daemon_reply_destroy(reply);

		if (rv < 0)
			break;
	}
out:
	/* Return lockd VG's on the vg_lockd list. */

	list_for_each_entry_safe(ls, safe, &update_vgs, list) {
		list_del(&ls->list);

		if ((ls->lm_type == LD_LM_SANLOCK) || (ls->lm_type == LD_LM_DLM))
			list_add_tail(&ls->list, vg_lockd);
		else
			free(ls);
	}

	return rv;
#endif
}

static char _dm_uuid[DM_UUID_LEN];

static char *get_dm_uuid(char *dm_name)
{
	struct dm_info info;
	struct dm_task *dmt;
	const char *uuid;

	if (!(dmt = dm_task_create(DM_DEVICE_INFO)))
		goto fail_out;

	if (!dm_task_set_name(dmt, dm_name))
		goto fail;

	if (!dm_task_run(dmt))
		goto fail;

	if (!dm_task_get_info(dmt, &info))
		goto fail;

	if (!info.exists)
		goto fail;

	uuid = dm_task_get_uuid(dmt);
	if (!uuid) {
		log_error("Failed to get uuid for device %s", dm_name);
		goto fail;
	}

	if (strncmp(uuid, "LVM", 3)) {
		log_debug("dm device %s is not from LVM", dm_name);
		goto fail;
	}

	memset(_dm_uuid, 0, sizeof(_dm_uuid));
	strncpy(_dm_uuid, uuid, sizeof(_dm_uuid)-1);
	dm_task_destroy(dmt);
	return _dm_uuid;

fail:
	dm_task_destroy(dmt);
fail_out:
	return NULL;
}

/*
 * dm reports the LV uuid as:
 * LVM-ydpRIdDWBDX25upmj2k0D4deat6oxH8er03T0f4xM8rPIV8XqIhwv3h8Y7xRWjMr
 *
 * the lock name for the LV is:
 * r03T0f-4xM8-rPIV-8XqI-hwv3-h8Y7-xRWjMr
 *
 * This function formats both as:
 * r03T0f4xM8rPIV8XqIhwv3h8Y7xRWjMr
 *
 * and returns 1 if they match.
 */

static int match_dm_uuid(char *dm_uuid, char *lv_lock_uuid)
{
	char buf1[64];
	char buf2[64];
	int i, j;

	memset(buf1, 0, sizeof(buf1));
	memset(buf2, 0, sizeof(buf2));

	for (i = 0, j = 0; i < strlen(lv_lock_uuid); i++) {
		if (lv_lock_uuid[i] == '-')
			continue;
		buf1[j] = lv_lock_uuid[i];
		j++;
	}

	for (i = 36, j = 0; i < 69; i++) {
		buf2[j] = dm_uuid[i];
		j++;
	}

	if (!strcmp(buf1, buf2))
		return 1;
	return 0;
}

/*
 * All LVs with a lock_type are on ls->resources.
 * Remove any that are not active.  The remaining
 * will have locks adopted.
 */

static int remove_inactive_lvs(struct list_head *vg_lockd)
{
	struct lockspace *ls;
	struct resource *r, *rsafe;
	struct dm_names *names;
	struct dm_task *dmt;
	char *dm_uuid;
	char *vgname, *lvname, *layer;
	char namebuf[MAX_NAME+1];
	unsigned next = 0;
	int rv = 0;

	if (!(dmt = dm_task_create(DM_DEVICE_LIST)))
		return -1;

	if (!dm_task_run(dmt)) {
		log_error("Failed to get dm devices");
		rv = -1;
		goto ret;
	}

	if (!(names = dm_task_get_names(dmt))) {
		log_error("Failed to get dm names");
		rv = -1;
		goto ret;
	}

	if (!names->dev) {
		log_debug("dm names none found");
		goto out;
	}

	/*
	 * For each dm name, compare it to each lv in each lockd vg.
	 */

	do {
		names = (struct dm_names *)((char *) names + next);

		dm_uuid = get_dm_uuid(names->name);
		if (!dm_uuid)
			goto next_dmname;

		vgname = NULL;
		lvname = NULL;
		layer = NULL;

		memset(namebuf, 0, sizeof(namebuf));
		strncpy(namebuf, names->name, MAX_NAME);
		vgname = namebuf;

		if (!dm_split_lvm_name(NULL, namebuf, &vgname, &lvname, &layer)) {
			log_error("failed to split dm name %s", namebuf);
			goto next_dmname;
		}

		log_debug("adopt remove_inactive dm name %s dm uuid %s vgname %s lvname %s",
			  names->name, dm_uuid, vgname, lvname);

		if (!vgname || !lvname) {
			log_debug("dm name %s invalid split vg %s lv %s layer %s",
				  names->name, vgname ? vgname : "", lvname ? lvname : "", layer ? layer : "");
			goto next_dmname;
		}

		list_for_each_entry(ls, vg_lockd, list) {
			if (strcmp(vgname, ls->vg_name))
				continue;

			if (!strcmp(lvname, "lvmlock"))
				continue;

			list_for_each_entry(r, &ls->resources, list) {
				if (!match_dm_uuid(dm_uuid, r->name))
					continue;

				/* Found an active LV in a lockd VG. */
				log_debug("dm device %s adopt in vg %s lv %s",
					  names->name, ls->vg_name, r->name);
				r->adopt = 1;
				goto next_dmname;
			}
		}
next_dmname:
		next = names->next;
	} while (next);

out:
	/* Remove any struct resources that do not need locks adopted. */
	list_for_each_entry(ls, vg_lockd, list) {
		list_for_each_entry_safe(r, rsafe, &ls->resources, list) {
			if (r->adopt) {
				r->adopt = 0;
			} else {
				log_debug("lockd vg %s remove inactive lv %s", ls->vg_name, r->name);
				list_del(&r->list);
				free_resource(r);
			}
		}
	}
ret:
	dm_task_destroy(dmt);
	return rv;
}

static void adopt_locks(void)
{
	struct list_head ls_found;
	struct list_head vg_lockd;
	struct list_head to_unlock;
	struct lockspace *ls, *lsafe;
	struct lockspace *ls1, *l1safe;
	struct lockspace *ls2, *l2safe;
	struct resource *r, *rsafe;
	struct action *act, *asafe;
	int count_start = 0, count_start_done = 0, count_start_fail = 0;
	int count_adopt = 0, count_adopt_done = 0, count_adopt_fail = 0;
	int found, rv;

	INIT_LIST_HEAD(&adopt_results);

	INIT_LIST_HEAD(&ls_found);
	INIT_LIST_HEAD(&vg_lockd);
	INIT_LIST_HEAD(&to_unlock);

	/*
	 * Get list of lockspaces from lock managers.
	 * Get list of VGs from lvmetad with a lockd type.
	 * Get list of active lockd type LVs from /dev.
	 */

	if (lm_support_dlm() && lm_is_running_dlm()) {
		rv = lm_get_lockspaces_dlm(&ls_found);
		if (rv < 0)
			goto fail;
	}

	if (lm_support_sanlock() && lm_is_running_sanlock()) {
		rv = lm_get_lockspaces_sanlock(&ls_found);
		if (rv < 0)
			goto fail;
	}

	if (list_empty(&ls_found)) {
		log_debug("No lockspaces found to adopt");
		return;
	}

	/*
	 * Adds a struct lockspace to vg_lockd for each lockd VG.
	 * Adds a struct resource to ls->resources for each LV.
	 */
	rv = get_lockd_vgs(&vg_lockd);
	if (rv < 0) {
		log_error("adopt_locks get_lockd_vgs failed");
		goto fail;
	}

	/*
	 * For each resource on each lockspace, check if the
	 * corresponding LV is active.  If so, leave the
	 * resource struct, if not free the resource struct.
	 * The remain entries need to have locks adopted.
	 */
	rv = remove_inactive_lvs(&vg_lockd);
	if (rv < 0) {
		log_error("adopt_locks remove_inactive_lvs failed");
		goto fail;
	}

	list_for_each_entry(ls, &ls_found, list) {
		if (ls->lm_type == LD_LM_DLM)
			gl_use_dlm = 1;

		log_debug("adopt %s lockspace %s vg %s",
			  lm_str(ls->lm_type), ls->name, ls->vg_name);
	}

	if (!gl_use_dlm)
		gl_use_sanlock = 1;

	list_for_each_entry(ls, &vg_lockd, list) {
		log_debug("adopt vg %s lock_type %s lock_args %s",
			  ls->vg_name, lm_str(ls->lm_type), ls->vg_args);

		list_for_each_entry(r, &ls->resources, list)
			log_debug("adopt lv %s %s", ls->vg_name, r->name);
	}

	/*
	 * Compare and merge the list of lockspaces in ls_found
	 * and the list of lockd VGs in vg_lockd.
	 *
	 * An ls from ls_found may not have had any active lvs when
	 * previous lvmlockd died, but the ls should still be joined,
	 * and checked for GL/VG locks.
	 *
	 * An ls from vg_lockd with active lvs should be in ls_found.
	 * If it's not then we might want to join the ls and acquire locks
	 * for the active lvs (as opposed to adopting orphans for them.)
	 * The orphan lock in the ls should have prevented the ls in
	 * the lock manager from going away.
	 *
	 * If an ls in vg_lockd has no active lvs and does not have
	 * a matching entry in ls_found, then skip it.
	 * 
	 * An ls in ls_found should always have a matching ls in
	 * vg_lockd.  If it doesn't, then maybe the vg has been
	 * removed even though the lockspace for the vg is still
	 * in the lock manager.  Just leave the ls in the lm
	 * alone, and skip the ls_found entry.
	 */

	list_for_each_entry_safe(ls1, l1safe, &ls_found, list) {

		/* The dlm global lockspace is special and doesn't match a VG. */
		if ((ls1->lm_type == LD_LM_DLM) && !strcmp(ls1->name, gl_lsname_dlm)) {
			list_del(&ls1->list);
			free(ls1);
			continue;
		}

		found = 0;

		list_for_each_entry_safe(ls2, l2safe, &vg_lockd, list) {
			if (strcmp(ls1->vg_name, ls2->vg_name))
				continue;

			/*
		 	 * LS in both ls_found and vg_lockd.
		 	 */
			log_debug("ls %s matches vg %s", ls1->name, ls2->vg_name);
			memcpy(ls1->vg_uuid, ls2->vg_uuid, 64);
			memcpy(ls1->vg_args, ls2->vg_args, MAX_ARGS);
			list_for_each_entry_safe(r, rsafe, &ls2->resources, list) {
				list_del(&r->list);
				list_add(&r->list, &ls1->resources);
			}
			list_del(&ls2->list);
			free(ls2);
			found = 1;
			break;
		}

		/*
		 * LS in ls_found, not in vg_lockd.
		 * An lvm lockspace found in the lock manager has no
		 * corresponding VG.  This shouldn't usually
		 * happen, but it's possible the VG could have been removed
		 * while the orphaned lockspace from it was still around.
		 * Report an error and leave the ls in the lm alone.
		 */
		if (!found) {
			log_error("No VG %s found for lockspace %s %s",
				  ls1->vg_name, ls1->name, lm_str(ls1->lm_type));
			list_del(&ls1->list);
			free(ls1);
		}
	}

	/*
	 * LS in vg_lockd, not in ls_found.
	 * lockd vgs that do not have an existing lockspace.
	 * This wouldn't be unusual; we just skip the vg.
	 * But, if the vg has active lvs, then it should have had locks
	 * and a lockspace.  Should we attempt to join the lockspace and
	 * acquire (not adopt) locks for these LVs?
	 */

	list_for_each_entry_safe(ls, lsafe, &vg_lockd, list) {
		if (!list_empty(&ls->resources)) {
			/* We should have found a lockspace. */
			/* add this ls and acquire locks for ls->resources? */
			log_error("No lockspace %s %s found for VG %s with active LVs",
				  ls->name, lm_str(ls->lm_type), ls->vg_name);
		} else {
			/* The VG wasn't started in the previous lvmlockd. */
			log_debug("No ls found for vg %s", ls->vg_name);
		}
		
		list_del(&ls->list);
		free(ls);
	}

	/*
	 * Create and queue start actions to add lockspaces.
	 */

	if (gl_use_dlm) {
		if (!(act = alloc_action()))
			goto fail;
		log_debug("adopt add dlm global lockspace");
		act->op = LD_OP_START;
		act->flags = (LD_AF_ADOPT | LD_AF_WAIT);
		act->rt = LD_RT_GL;
		act->lm_type = LD_LM_DLM;
		act->client_id = INTERNAL_CLIENT_ID;
		add_dlm_global_lockspace(act);
		count_start++;
	}

	list_for_each_entry_safe(ls, lsafe, &ls_found, list) {
		if (!(act = alloc_action()))
			goto fail;
		act->op = LD_OP_START;
		act->flags = (LD_AF_ADOPT | LD_AF_WAIT);
		act->rt = LD_RT_VG;
		act->lm_type = ls->lm_type;
		act->client_id = INTERNAL_CLIENT_ID;
		strncpy(act->vg_name, ls->vg_name, MAX_NAME);
		memcpy(act->vg_uuid, ls->vg_uuid, 64);
		memcpy(act->vg_args, ls->vg_args, MAX_ARGS);
		act->host_id = ls->host_id;

		log_debug("adopt add %s vg lockspace %s", lm_str(act->lm_type), act->vg_name);

		rv = add_lockspace_thread(ls->name, act->vg_name, act->vg_uuid,
					  act->lm_type, act->vg_args, act);
		if (rv < 0) {
			log_error("Failed to create lockspace thread for VG %s", ls->vg_name);
			list_del(&ls->list);
			free(ls);
			free_action(act);
			count_start_fail++;
			continue;
		}

		/*
		 * When the lockspace_thread is done with the start act,
		 * it will see the act ADOPT flag and move the act onto
		 * the adopt_results list for us to collect below.
		 */
		count_start++;
	}

	log_debug("adopt starting %d lockspaces", count_start);

	/*
	 * Wait for all start/rejoin actions to complete.  Each start action
	 * queued above will appear on the adopt_results list when finished.
	 */

	while (count_start_done < count_start) {
		sleep(1);
		act = NULL;

		pthread_mutex_lock(&client_mutex);
		if (!list_empty(&adopt_results)) {
			act = list_first_entry(&adopt_results, struct action, list);
			list_del(&act->list);
		}
		pthread_mutex_unlock(&client_mutex);

		if (!act)
			continue;

		if (act->result < 0) {
			log_error("adopt add lockspace failed vg %s %d", act->vg_name, act->result);
			count_start_fail++;
		}

		free_action(act);
		count_start_done++;
	}

	log_debug("adopt started %d lockspaces done %d fail %d",
		  count_start, count_start_done, count_start_fail);

	/*
	 * Create lock-adopt actions for active LVs (ls->resources),
	 * and GL/VG locks (we don't know if these locks were held
	 * and orphaned by the last lvmlockd, so try to adopt them
	 * to see.)
	 *
	 * A proper struct lockspace now exists on the lockspaces list
	 * for each ls in ls_found.  Lock ops for one of those
	 * lockspaces can be done as OP_LOCK actions queued using
	 * add_lock_action();
	 *
	 * Start by attempting to adopt the lock in the most likely
	 * mode it was left in (ex for lvs, sh for vg/gl).  If
	 * the mode is wrong, the lm will return an error and we
	 * try again with the other mode.
	 */

	list_for_each_entry(ls, &ls_found, list) {

		/*
		 * Adopt orphan LV locks.
		 */

		list_for_each_entry(r, &ls->resources, list) {
			if (!(act = alloc_action()))
				goto fail;
			act->op = LD_OP_LOCK;
			act->rt = LD_RT_LV;
			act->mode = LD_LK_EX;
			act->flags = (LD_AF_ADOPT | LD_AF_PERSISTENT);
			act->client_id = INTERNAL_CLIENT_ID;
			act->lm_type = ls->lm_type;
			strncpy(act->vg_name, ls->vg_name, MAX_NAME);
			strncpy(act->lv_uuid, r->name, MAX_NAME);
			strncpy(act->lv_args, r->lv_args, MAX_ARGS);

			log_debug("adopt lock for lv %s %s", act->vg_name, act->lv_uuid);

			rv = add_lock_action(act);
			if (rv < 0) {
				log_error("adopt add_lock_action lv %s %s error %d", act->vg_name, act->lv_uuid, rv);
				count_adopt_fail++;
				free_action(act);
			} else {
				count_adopt++;
			}
		}

		/*
		 * Adopt orphan VG lock.
		 */

		if (!(act = alloc_action()))
			goto fail;
		act->op = LD_OP_LOCK;
		act->rt = LD_RT_VG;
		act->mode = LD_LK_SH;
		act->flags = LD_AF_ADOPT;
		act->client_id = INTERNAL_CLIENT_ID;
		act->lm_type = ls->lm_type;
		strncpy(act->vg_name, ls->vg_name, MAX_NAME);

		log_debug("adopt lock for vg %s", act->vg_name);

		rv = add_lock_action(act);
		if (rv < 0) {
			log_error("adopt add_lock_action vg %s error %d", act->vg_name, rv);
			count_adopt_fail++;
			free_action(act);
		} else {
			count_adopt++;
		}
	}

	/*
	 * Adopt orphan GL lock.
	 */

	if (!(act = alloc_action()))
		goto fail;
	act->op = LD_OP_LOCK;
	act->rt = LD_RT_GL;
	act->mode = LD_LK_SH;
	act->flags = LD_AF_ADOPT;
	act->client_id = INTERNAL_CLIENT_ID;
	act->lm_type = (gl_use_sanlock ? LD_LM_SANLOCK : LD_LM_DLM);

	log_debug("adopt lock for gl");

	rv = add_lock_action(act);
	if (rv < 0) {
		log_error("adopt add_lock_action gl %s error %d", act->vg_name, rv);
		count_adopt_fail++;
		free_action(act);
	} else {
		count_adopt++;
	}

	/*
	 * Wait for lock-adopt actions to complete.  The completed
	 * actions are passed back here via the adopt_results list.
	 */

	while (count_adopt_done < count_adopt) {
		sleep(1);
		act = NULL;

		pthread_mutex_lock(&client_mutex);
		if (!list_empty(&adopt_results)) {
			act = list_first_entry(&adopt_results, struct action, list);
			list_del(&act->list);
		}
		pthread_mutex_unlock(&client_mutex);

		if (!act)
			continue;

		/*
		 * lock adopt results 
		 */

		if (act->result == -EUCLEAN) {
			/*
			 * Adopt failed because the orphan has a different mode
			 * than initially requested.  Repeat the lock-adopt operation
			 * with the other mode.  N.B. this logic depends on first
			 * trying sh then ex for GL/VG locks, and ex then sh for
			 * LV locks.
			 */

			if ((act->rt != LD_RT_LV) && (act->mode == LD_LK_SH)) {
				/* GL/VG locks: attempt to adopt ex after sh failed. */
				act->mode = LD_LK_EX;
				rv = add_lock_action(act);

			} else if ((act->rt == LD_RT_LV) && (act->mode == LD_LK_EX)) {
				/* LV locks: attempt to adopt sh after ex failed. */
				act->mode = LD_LK_SH;
				rv = add_lock_action(act);

			} else {
				log_error("Failed to adopt %s lock in vg %s error %d",
					  rt_str(act->rt), act->vg_name, act->result);
				count_adopt_fail++;
				count_adopt_done++;
				free_action(act);
				rv = 0;
			}

			if (rv < 0) {
				log_error("adopt add_lock_action again %s", act->vg_name);
				count_adopt_fail++;
				count_adopt_done++;
				free_action(act);
			}

		} else if (act->result == -ENOENT) {
			/*
			 * No orphan lock exists.  This is common for GL/VG locks
			 * because they may not have been held when lvmlockd exited.
			 * It's also expected for LV types that do not use a lock.
			 */

			if (act->rt == LD_RT_LV) {
				/* Unexpected, we should have found an orphan. */
				log_error("Failed to adopt LV lock for %s %s error %d",
					  act->vg_name, act->lv_uuid, act->result);
				count_adopt_fail++;
			} else {
				/* Normal, no GL/VG lock was orphaned. */
				log_debug("Did not adopt %s lock in vg %s error %d",
					   rt_str(act->rt), act->vg_name, act->result);
			}

			count_adopt_done++;
			free_action(act);

		} else if (act->result < 0) {
			/*
			 * Some unexpected error.
			 */

			log_error("adopt lock rt %s vg %s lv %s error %d",
				  rt_str(act->rt), act->vg_name, act->lv_uuid, act->result);
			count_adopt_fail++;
			count_adopt_done++;
			free_action(act);

		} else {
			/*
			 * Adopt success.
			 */

			if (act->rt == LD_RT_LV) {
				log_debug("adopt success lv %s %s %s", act->vg_name, act->lv_uuid, mode_str(act->mode));
				free_action(act);
			} else if (act->rt == LD_RT_VG) {
				log_debug("adopt success vg %s %s", act->vg_name, mode_str(act->mode));
				list_add_tail(&act->list, &to_unlock);
			} else if (act->rt == LD_RT_GL) {
				log_debug("adopt success gl %s %s", act->vg_name, mode_str(act->mode));
				list_add_tail(&act->list, &to_unlock);
			}
			count_adopt_done++;
		}
	}

	/*
	 * Release adopted GL/VG locks.
	 * The to_unlock actions were the ones used to lock-adopt the GL/VG locks;
	 * now use them to do the unlocks.  These actions will again be placed
	 * on adopt_results for us to collect because they have the ADOPT flag set.
	 */

	count_adopt = 0;
	count_adopt_done = 0;

	list_for_each_entry_safe(act, asafe, &to_unlock, list) {
		list_del(&act->list);

		if (act->mode == LD_LK_EX) {
			/*
			 * FIXME: we probably want to check somehow that
			 * there's no lvm command still running that's
			 * using this ex lock and changing things.
			 */
			log_warn("adopt releasing ex %s lock %s",
				 rt_str(act->rt), act->vg_name);
		}

		act->mode = LD_LK_UN;

		log_debug("adopt unlock for %s %s", rt_str(act->rt), act->vg_name);

		rv = add_lock_action(act);
		if (rv < 0) {
			log_error("adopt unlock add_lock_action error %d", rv);
			free_action(act);
		} else {
			count_adopt++;
		}
	}

	/* Wait for the unlocks to complete. */

	while (count_adopt_done < count_adopt) {
		sleep(1);
		act = NULL;

		pthread_mutex_lock(&client_mutex);
		if (!list_empty(&adopt_results)) {
			act = list_first_entry(&adopt_results, struct action, list);
			list_del(&act->list);
		}
		pthread_mutex_unlock(&client_mutex);

		if (!act)
			continue;

		if (act->result < 0)
			log_error("adopt unlock error %d", act->result);

		count_adopt_done++;
		free_action(act);
	}


	/* FIXME: purge any remaining orphan locks in each rejoined ls? */

	if (count_start_fail || count_adopt_fail)
		goto fail;

	log_debug("adopt_locks done");
	return;

fail:
	log_error("adopt_locks failed, reset host");
}

static int get_peer_pid(int fd)
{
	struct ucred cred;
	unsigned int len = sizeof(cred);

	if (getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &cred, &len) != 0)
		return -1;

	return cred.pid;
}

static void process_listener(int poll_fd)
{
	struct client *cl;
	int fd, pi;

	/* assert poll_fd == listen_fd */

	fd = accept(listen_fd, NULL, NULL);
	if (fd < 0)
		return;

	if (!(cl = alloc_client())) {
		if (!close(fd))
			log_error("failed to close lockd poll fd");
		return;
	}

	pi = add_pollfd(fd);
	if (pi < 0) {
		log_error("process_listener add_pollfd error %d", pi);
		free_client(cl);
		return;
	}

	cl->pi = pi;
	cl->fd = fd;
	cl->pid = get_peer_pid(fd);

	pthread_mutex_init(&cl->mutex, NULL);

	pthread_mutex_lock(&client_mutex);
	client_ids++;

	if (client_ids == INTERNAL_CLIENT_ID)
		client_ids++;
	if (!client_ids)
		client_ids++;

	cl->id = client_ids;
	list_add_tail(&cl->list, &client_list);
	pthread_mutex_unlock(&client_mutex);

	log_debug("new cl %u pi %d fd %d", cl->id, cl->pi, cl->fd);
}

/*
 * main loop polls on pipe[0] so that a thread can
 * restart the poll by writing to pipe[1].
 */
static int setup_restart(void)
{
	if (pipe(restart_fds)) {
		log_error("setup_restart pipe error %d", errno);
		return -1;
	}

	restart_pi = add_pollfd(restart_fds[0]);
	if (restart_pi < 0)
		return restart_pi;

	return 0;
}

/*
 * thread wrote 'w' to restart_fds[1] to restart poll()
 * after adding an fd back into pollfd.
 */
static void process_restart(int fd)
{
	char wake[1];
	int rv;

	/* assert fd == restart_fds[0] */

	rv = read(restart_fds[0], wake, 1);
	if (!rv || rv < 0)
		log_debug("process_restart error %d", errno);
}

static void sigterm_handler(int sig __attribute__((unused)))
{
	daemon_quit = 1;
}

static int main_loop(daemon_state *ds_arg)
{
	struct client *cl;
	int i, rv, is_recv, is_dead;

	signal(SIGTERM, &sigterm_handler);

	rv = setup_structs();
	if (rv < 0) {
		log_error("Can't allocate memory");
		return rv;
	}

	strcpy(gl_lsname_dlm, S_NAME_GL_DLM);

	INIT_LIST_HEAD(&lockspaces);
	pthread_mutex_init(&lockspaces_mutex, NULL);
	pthread_mutex_init(&pollfd_mutex, NULL);
	pthread_mutex_init(&log_mutex, NULL);

	openlog("lvmlockd", LOG_CONS | LOG_PID, LOG_DAEMON);
	log_warn("lvmlockd started");

	listen_fd = ds_arg->socket_fd;
	listen_pi = add_pollfd(listen_fd);

	setup_client_thread();
	setup_worker_thread();
	setup_restart();

#ifdef USE_SD_NOTIFY
	sd_notify(0, "READY=1");
#endif

	/*
	 * Attempt to rejoin lockspaces and adopt locks from a previous
	 * instance of lvmlockd that left behind lockspaces/locks.
	 */
	if (adopt_opt)
		adopt_locks();

	while (1) {
		rv = poll(pollfd, pollfd_maxi + 1, -1);
		if ((rv == -1 && errno == EINTR) || daemon_quit) {
			if (daemon_quit) {
				int count;
				/* first sigterm would trigger stops, and
				   second sigterm may finish the joins. */
				count = for_each_lockspace(DO_STOP, DO_FREE, NO_FORCE);
				if (!count)
					break;
				log_debug("ignore shutdown for %d lockspaces", count);
				daemon_quit = 0;
			}
			continue;
		}
		if (rv < 0) {
			log_error("poll errno %d", errno);
			break;
		}

		for (i = 0; i <= pollfd_maxi; i++) {
			if (pollfd[i].fd < 0)
				continue;

			is_recv = 0;
			is_dead = 0;

			if (pollfd[i].revents & POLLIN)
				is_recv = 1;
			if (pollfd[i].revents & (POLLERR | POLLHUP | POLLNVAL))
				is_dead = 1;

			if (!is_recv && !is_dead)
				continue;

			if (i == listen_pi) {
				process_listener(pollfd[i].fd);
				continue;
			}

			if (i == restart_pi) {
				process_restart(pollfd[i].fd);
				continue;
			}

			/*
			log_debug("poll pi %d fd %d revents %x",
				  i, pollfd[i].fd, pollfd[i].revents);
			*/

			pthread_mutex_lock(&client_mutex);
			cl = find_client_pi(i);
			if (cl) {
				pthread_mutex_lock(&cl->mutex);

				if (cl->recv) {
					/* should not happen */
					log_error("main client %u already recv", cl->id);

				} else if (cl->dead) {
					/* should not happen */
					log_error("main client %u already dead", cl->id);

				} else if (is_dead) {
					log_debug("close %s[%d] cl %u fd %d",
						  cl->name[0] ? cl->name : "client",
						  cl->pid, cl->id, cl->fd);
					cl->dead = 1;
					cl->pi = -1;
					cl->fd = -1;
					cl->poll_ignore = 0;
					if (close(pollfd[i].fd))
						log_error("close fd %d failed", pollfd[i].fd);
					pollfd[i].fd = POLL_FD_UNUSED;
					pollfd[i].events = 0;
					pollfd[i].revents = 0;

				} else if (is_recv) {
					cl->recv = 1;
					cl->poll_ignore = 1;
					pollfd[i].fd = POLL_FD_IGNORE;
					pollfd[i].events = 0;
					pollfd[i].revents = 0;
				}

				pthread_mutex_unlock(&cl->mutex);

				client_work = 1;
				pthread_cond_signal(&client_cond);

				/* client_thread will pick up and work on any
				   client with cl->recv or cl->dead set */

			} else {
				/* don't think this can happen */
				log_error("no client for index %d fd %d",
					  i, pollfd[i].fd);
				if (close(pollfd[i].fd))
					log_error("close fd %d failed", pollfd[i].fd);
				pollfd[i].fd = POLL_FD_UNUSED;
				pollfd[i].events = 0;
				pollfd[i].revents = 0;
			}
			pthread_mutex_unlock(&client_mutex);

			/* After set_dead, should we scan pollfd for
			   last unused slot and reduce pollfd_maxi? */
		}
	}

	for_each_lockspace_retry(DO_STOP, DO_FREE, DO_FORCE);
	close_worker_thread();
	close_client_thread();
	closelog();
	return 1; /* libdaemon uses 1 for success */
}

static void usage(char *prog, FILE *file)
{
	fprintf(file, "Usage:\n");
	fprintf(file, "%s [options]\n\n", prog);
	fprintf(file, "  --help | -h\n");
	fprintf(file, "        Show this help information.\n");
	fprintf(file, "  --version | -V\n");
	fprintf(file, "        Show version of lvmlockd.\n");
	fprintf(file, "  --test | -T\n");
	fprintf(file, "        Test mode, do not call lock manager.\n");
	fprintf(file, "  --foreground | -f\n");
	fprintf(file, "        Don't fork.\n");
	fprintf(file, "  --daemon-debug | -D\n");
	fprintf(file, "        Don't fork and print debugging to stdout.\n");
	fprintf(file, "  --pid-file | -p <path>\n");
	fprintf(file, "        Set path to the pid file. [%s]\n", LVMLOCKD_PIDFILE);
	fprintf(file, "  --socket-path | -s <path>\n");
	fprintf(file, "        Set path to the socket to listen on. [%s]\n", LVMLOCKD_SOCKET);
	fprintf(file, "  --syslog-priority | -S err|warning|debug\n");
	fprintf(file, "        Write log messages from this level up to syslog. [%s]\n", _syslog_num_to_name(LOG_SYSLOG_PRIO));
	fprintf(file, "  --gl-type | -g <str>\n");
	fprintf(file, "        Set global lock type to be dlm|sanlock.\n");
	fprintf(file, "  --host-id | -i <num>\n");
	fprintf(file, "        Set the local sanlock host id.\n");
	fprintf(file, "  --host-id-file | -F <path>\n");
	fprintf(file, "        A file containing the local sanlock host_id.\n");
	fprintf(file, "  --sanlock-timeout | -o <seconds>\n");
	fprintf(file, "        Set the sanlock lockspace I/O timeout.\n");
	fprintf(file, "  --adopt | -A 0|1\n");
	fprintf(file, "        Adopt locks from a previous instance of lvmlockd.\n");
}

int main(int argc, char *argv[])
{
	daemon_state ds = {
		.daemon_main = main_loop,
		.daemon_init = NULL,
		.daemon_fini = NULL,
		.pidfile = getenv("LVM_LVMLOCKD_PIDFILE"),
		.socket_path = getenv("LVM_LVMLOCKD_SOCKET"),
		.protocol = lvmlockd_protocol,
		.protocol_version = lvmlockd_protocol_version,
		.name = "lvmlockd",
	};

	static struct option long_options[] = {
		{"help",            no_argument,       0, 'h' },
		{"version",         no_argument,       0, 'V' },
		{"test",            no_argument,       0, 'T' },
		{"foreground",      no_argument,       0, 'f' },
		{"daemon-debug",    no_argument,       0, 'D' },
		{"pid-file",        required_argument, 0, 'p' },
		{"socket-path",     required_argument, 0, 's' },
		{"gl-type",         required_argument, 0, 'g' },
		{"host-id",         required_argument, 0, 'i' },
		{"host-id-file",    required_argument, 0, 'F' },
		{"adopt",           required_argument, 0, 'A' },
		{"syslog-priority", required_argument, 0, 'S' },
		{"sanlock-timeout", required_argument, 0, 'o' },
		{0, 0, 0, 0 }
	};

	while (1) {
		int c;
		int lm;
		int option_index = 0;

		c = getopt_long(argc, argv, "hVTfDp:s:l:g:S:I:A:o:",
				long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case '0':
			break;
		case 'h':
			usage(argv[0], stdout);
			exit(EXIT_SUCCESS);
		case 'V':
			printf("lvmlockd version: " LVM_VERSION "\n");
			exit(EXIT_SUCCESS);
		case 'T':
			daemon_test = 1;
			break;
		case 'f':
			ds.foreground = 1;
			break;
		case 'D':
			ds.foreground = 1;
			daemon_debug = 1;
			break;
		case 'p':
			ds.pidfile = strdup(optarg);
			break;
		case 's':
			ds.socket_path = strdup(optarg);
			break;
		case 'g':
			lm = str_to_lm(optarg);
			if (lm == LD_LM_DLM && lm_support_dlm())
				gl_use_dlm = 1;
			else if (lm == LD_LM_SANLOCK && lm_support_sanlock())
				gl_use_sanlock = 1;
			else {
				fprintf(stderr, "invalid gl-type option\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 'i':
			daemon_host_id = atoi(optarg);
			break;
		case 'F':
			daemon_host_id_file = strdup(optarg);
			break;
		case 'o':
			sanlock_io_timeout = atoi(optarg);
			break;
		case 'A':
			adopt_opt = atoi(optarg);
			break;
		case 'S':
			syslog_priority = _syslog_name_to_num(optarg);
			break;
		case '?':
		default:
			usage(argv[0], stdout);
			exit(EXIT_FAILURE);
		}
	}

	if (!ds.pidfile)
		ds.pidfile = LVMLOCKD_PIDFILE;

	if (!ds.socket_path)
		ds.socket_path = LVMLOCKD_SOCKET;

	/* runs daemon_main/main_loop */
	daemon_start(ds);

	return 0;
}
