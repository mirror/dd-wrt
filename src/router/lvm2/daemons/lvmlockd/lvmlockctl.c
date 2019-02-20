/*
 * Copyright (C) 2014-2015 Red Hat, Inc.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 */

#include "tools/tool.h"

#include "daemons/lvmlockd/lvmlockd-client.h"

#include <stddef.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <syslog.h>
#include <sys/socket.h>
#include <sys/un.h>

static int quit = 0;
static int info = 0;
static int dump = 0;
static int wait_opt = 1;
static int force_opt = 0;
static int kill_vg = 0;
static int drop_vg = 0;
static int gl_enable = 0;
static int gl_disable = 0;
static int stop_lockspaces = 0;
static char *arg_vg_name = NULL;

#define DUMP_SOCKET_NAME "lvmlockd-dump.sock"
#define DUMP_BUF_SIZE (1024 * 1024)
static char dump_buf[DUMP_BUF_SIZE+1];
static int dump_len;
static struct sockaddr_un dump_addr;
static socklen_t dump_addrlen;

daemon_handle _lvmlockd;

#define log_error(fmt, args...) \
do { \
	printf(fmt "\n", ##args); \
} while (0)

#define MAX_LINE 512

/* copied from lvmlockd-internal.h */
#define MAX_NAME 64
#define MAX_ARGS 64

/*
 * lvmlockd dumps the client info before the lockspaces,
 * so we can look up client info when printing lockspace info.
 */

#define MAX_CLIENTS 100

struct client_info {
	uint32_t client_id;
	int pid;
	char name[MAX_NAME+1];
};

static struct client_info clients[MAX_CLIENTS];
static int num_clients;

static void save_client_info(char *line)
{
	uint32_t pid = 0;
	int fd = 0;
	int pi = 0;
	uint32_t client_id = 0;
	char name[MAX_NAME+1] = { 0 };

	(void) sscanf(line, "info=client pid=%u fd=%d pi=%d id=%u name=%s",
	       &pid, &fd, &pi, &client_id, name);

	clients[num_clients].client_id = client_id;
	clients[num_clients].pid = pid;
	strcpy(clients[num_clients].name, name);
	num_clients++;
}

static void find_client_info(uint32_t client_id, uint32_t *pid, char *cl_name)
{
	int i;

	for (i = 0; i < num_clients; i++) {
		if (clients[i].client_id == client_id) {
			*pid = clients[i].pid;
			strcpy(cl_name, clients[i].name);
			return;
		}
	}
}

static int first_ls = 1;

static void format_info_ls(char *line)
{
	char ls_name[MAX_NAME+1] = { 0 };
	char vg_name[MAX_NAME+1] = { 0 };
	char vg_uuid[MAX_NAME+1] = { 0 };
	char vg_sysid[MAX_NAME+1] = { 0 };
	char lock_args[MAX_ARGS+1] = { 0 };
	char lock_type[MAX_NAME+1] = { 0 };

	(void) sscanf(line, "info=ls ls_name=%s vg_name=%s vg_uuid=%s vg_sysid=%s vg_args=%s lm_type=%s",
	       ls_name, vg_name, vg_uuid, vg_sysid, lock_args, lock_type);

	if (!first_ls)
		printf("\n");
	first_ls = 0;

	printf("VG %s lock_type=%s %s\n", vg_name, lock_type, vg_uuid);

	printf("LS %s %s\n", lock_type, ls_name);
}

static void format_info_ls_action(char *line)
{
	uint32_t client_id = 0;
	char flags[MAX_NAME+1] = { 0 };
	char version[MAX_NAME+1] = { 0 };
	char op[MAX_NAME+1] = { 0 };
	uint32_t pid = 0;
	char cl_name[MAX_NAME+1] = { 0 };

	(void) sscanf(line, "info=ls_action client_id=%u %s %s op=%s",
	       &client_id, flags, version, op);

	find_client_info(client_id, &pid, cl_name);

	printf("OP %s pid %u (%s)\n", op, pid, cl_name);
}

static void format_info_r(char *line, char *r_name_out, char *r_type_out)
{
	char r_name[MAX_NAME+1] = { 0 };
	char r_type[4] = { 0 };
	char mode[4] = { 0 };
	char sh_count[MAX_NAME+1] = { 0 };
	uint32_t ver = 0;

	(void) sscanf(line, "info=r name=%s type=%s mode=%s %s version=%u",
	       r_name, r_type, mode, sh_count, &ver);

	strcpy(r_name_out, r_name);
	strcpy(r_type_out, r_type);

	/* when mode is not un, wait and print each lk line */
	if (strcmp(mode, "un"))
		return;

	/* when mode is un, there will be no lk lines, so print now */

	if (!strcmp(r_type, "gl")) {
		printf("LK GL un ver %u\n", ver);

	} else if (!strcmp(r_type, "vg")) {
		printf("LK VG un ver %u\n", ver);

	} else if (!strcmp(r_type, "lv")) {
		printf("LK LV un %s\n", r_name);
	}
}

static void format_info_lk(char *line, char *r_name, char *r_type)
{
	char mode[4] = { 0 };
	uint32_t ver = 0;
	char flags[MAX_NAME+1] = { 0 };
	uint32_t client_id = 0;
	uint32_t pid = 0;
	char cl_name[MAX_NAME+1] = { 0 };

	if (!r_name[0] || !r_type[0]) {
		printf("format_info_lk error r_name %s r_type %s\n", r_name, r_type);
		printf("%s\n", line);
		return;
	}

	(void) sscanf(line, "info=lk mode=%s version=%u %s client_id=%u",
	       mode, &ver, flags, &client_id);

	find_client_info(client_id, &pid, cl_name);

	if (!strcmp(r_type, "gl")) {
		printf("LK GL %s ver %u pid %u (%s)\n", mode, ver, pid, cl_name);

	} else if (!strcmp(r_type, "vg")) {
		printf("LK VG %s ver %u pid %u (%s)\n", mode, ver, pid, cl_name);

	} else if (!strcmp(r_type, "lv")) {
		printf("LK LV %s %s\n", mode, r_name);
	}
}

static void format_info_r_action(char *line, char *r_name, char *r_type)
{
	uint32_t client_id = 0;
	char flags[MAX_NAME+1] = { 0 };
	char version[MAX_NAME+1] = { 0 };
	char op[MAX_NAME+1] = { 0 };
	char rt[4] = { 0 };
	char mode[4] = { 0 };
	char lm[MAX_NAME+1] = { 0 };
	char result[MAX_NAME+1] = { 0 };
	char lm_rv[MAX_NAME+1] = { 0 };
	uint32_t pid = 0;
	char cl_name[MAX_NAME+1] = { 0 };

	if (!r_name[0] || !r_type[0]) {
		printf("format_info_r_action error r_name %s r_type %s\n", r_name, r_type);
		printf("%s\n", line);
		return;
	}

	(void) sscanf(line, "info=r_action client_id=%u %s %s op=%s rt=%s mode=%s %s %s %s",
	       &client_id, flags, version, op, rt, mode, lm, result, lm_rv);

	find_client_info(client_id, &pid, cl_name);

	if (strcmp(op, "lock")) {
		printf("OP %s pid %u (%s)\n", op, pid, cl_name);
		return;
	}

	if (!strcmp(r_type, "gl")) {
		printf("LW GL %s ver %u pid %u (%s)\n", mode, 0, pid, cl_name);

	} else if (!strcmp(r_type, "vg")) {
		printf("LW VG %s ver %u pid %u (%s)\n", mode, 0, pid, cl_name);

	} else if (!strcmp(r_type, "lv")) {
		printf("LW LV %s %s\n", mode, r_name);
	}
}

static void format_info_line(char *line, char *r_name, char *r_type)
{
	if (!strncmp(line, "info=structs ", strlen("info=structs "))) {
		/* only print this in the raw info dump */

	} else if (!strncmp(line, "info=client ", strlen("info=client "))) {
		save_client_info(line);

	} else if (!strncmp(line, "info=ls ", strlen("info=ls "))) {
		format_info_ls(line);

	} else if (!strncmp(line, "info=ls_action ", strlen("info=ls_action "))) {
		format_info_ls_action(line);

	} else if (!strncmp(line, "info=r ", strlen("info=r "))) {
		/*
		 * r_name/r_type are reset when a new resource is found.
		 * They are reused for the lock and action lines that
		 * follow a resource line.
		 */
		memset(r_name, 0, MAX_NAME+1);
		memset(r_type, 0, MAX_NAME+1);
		format_info_r(line, r_name, r_type);

	} else if (!strncmp(line, "info=lk ", strlen("info=lk "))) {
		/* will use info from previous r */
		format_info_lk(line, r_name, r_type);

	} else if (!strncmp(line, "info=r_action ", strlen("info=r_action "))) {
		/* will use info from previous r */
		format_info_r_action(line, r_name, r_type);
	} else {
		printf("UN %s\n", line);
	}
}

static void format_info(void)
{
	char line[MAX_LINE];
	char r_name[MAX_NAME+1];
	char r_type[MAX_NAME+1];
	int i, j;

	j = 0;
	memset(line, 0, sizeof(line));

	for (i = 0; i < dump_len; i++) {
		line[j++] = dump_buf[i];

		if ((line[j-1] == '\n') || (line[j-1] == '\0')) {
			format_info_line(line, r_name, r_type);
			j = 0;
			memset(line, 0, sizeof(line));
		}
	}
}


static daemon_reply _lvmlockd_send(const char *req_name, ...)
{
	va_list ap;
	daemon_reply repl;
	daemon_request req;

	req = daemon_request_make(req_name);

	va_start(ap, req_name);
	daemon_request_extend_v(req, ap);
	va_end(ap);

	repl = daemon_send(_lvmlockd, req);

	daemon_request_destroy(req);

	return repl;
}

/* See the same in lib/locking/lvmlockd.c */
#define NO_LOCKD_RESULT -1000

static int _lvmlockd_result(daemon_reply reply, int *result)
{
	int reply_result;

	if (reply.error) {
		log_error("lvmlockd_result reply error %d", reply.error);
		return 0;
	}

	if (strcmp(daemon_reply_str(reply, "response", ""), "OK")) {
		log_error("lvmlockd_result bad response");
		return 0;
	}

	reply_result = daemon_reply_int(reply, "op_result", NO_LOCKD_RESULT);
	if (reply_result == -1000) {
		log_error("lvmlockd_result no op_result");
		return 0;
	}

	*result = reply_result;

	return 1;
}

static int do_quit(void)
{
	daemon_reply reply;
	int rv = 0;

	reply = daemon_send_simple(_lvmlockd, "quit", NULL);

	if (reply.error) {
		log_error("reply error %d", reply.error);
		rv = reply.error;
	}

	daemon_reply_destroy(reply);
	return rv;
}

static int setup_dump_socket(void)
{
	int s, rv;

	s = socket(AF_LOCAL, SOCK_DGRAM, 0);
	if (s < 0)
		return s;

	memset(&dump_addr, 0, sizeof(dump_addr));
	dump_addr.sun_family = AF_LOCAL;
	strcpy(&dump_addr.sun_path[1], DUMP_SOCKET_NAME);
	dump_addrlen = sizeof(sa_family_t) + strlen(dump_addr.sun_path+1) + 1;

	rv = bind(s, (struct sockaddr *) &dump_addr, dump_addrlen);
	if (rv < 0) {
		rv = -errno;
		if (close(s))
			log_error("failed to close dump socket");
		return rv;
	}

	return s;
}

static int do_dump(const char *req_name)
{
	daemon_reply reply;
	int result;
	int fd, rv = 0;
	int count = 0;

	fd = setup_dump_socket();
	if (fd < 0) {
		log_error("socket error %d", fd);
		return fd;
	}

	reply = daemon_send_simple(_lvmlockd, req_name, NULL);

	if (reply.error) {
		log_error("reply error %d", reply.error);
		rv = reply.error;
		goto out;
	}

	result = daemon_reply_int(reply, "result", 0);
	dump_len = daemon_reply_int(reply, "dump_len", 0);

	daemon_reply_destroy(reply);

	if (result < 0) {
		rv = result;
		log_error("result %d", result);
	}

	if (!dump_len)
		goto out;

	memset(dump_buf, 0, sizeof(dump_buf));

retry:
	rv = recvfrom(fd, dump_buf + count, dump_len - count, MSG_WAITALL,
		      (struct sockaddr *)&dump_addr, &dump_addrlen);
	if (rv < 0) {
		log_error("recvfrom error %d %d", rv, errno);
		rv = -errno;
		goto out;
	}
	count += rv;

	if (count < dump_len)
		goto retry;

	rv = 0;
	if ((info && dump) || !strcmp(req_name, "dump"))
		printf("%s\n", dump_buf);
	else
		format_info();
out:
	if (close(fd))
		log_error("failed to close dump socket %d", fd);
	return rv;
}

static int do_able(const char *req_name)
{
	daemon_reply reply;
	int result;
	int rv;

	reply = _lvmlockd_send(req_name,
				"cmd = %s", "lvmlockctl",
				"pid = " FMTd64, (int64_t) getpid(),
				"vg_name = %s", arg_vg_name,
				NULL);

	if (!_lvmlockd_result(reply, &result)) {
		log_error("lvmlockd result %d", result);
		rv = result;
	} else {
		rv = 0;
	}

	daemon_reply_destroy(reply);
	return rv;
}

static int do_stop_lockspaces(void)
{
	daemon_reply reply;
	char opts[32];
	int result;
	int rv;

	memset(opts, 0, sizeof(opts));

	if (wait_opt)
		strcat(opts, "wait ");
	if (force_opt)
		strcat(opts, "force ");

	reply = _lvmlockd_send("stop_all",
				"cmd = %s", "lvmlockctl",
				"pid = " FMTd64, (int64_t) getpid(),
				"opts = %s", opts[0] ? opts : "none",
				NULL);

	if (!_lvmlockd_result(reply, &result)) {
		log_error("lvmlockd result %d", result);
		rv = result;
	} else {
		rv = 0;
	}

	daemon_reply_destroy(reply);
	return rv;
}

static int do_kill(void)
{
	daemon_reply reply;
	int result;
	int rv;

	syslog(LOG_EMERG, "Lost access to sanlock lease storage in VG %s.", arg_vg_name);
	/* These two lines explain the manual alternative to the FIXME below. */
	syslog(LOG_EMERG, "Immediately deactivate LVs in VG %s.", arg_vg_name);
	syslog(LOG_EMERG, "Once VG is unused, run lvmlockctl --drop %s.", arg_vg_name);

	/*
	 * It may not be strictly necessary to notify lvmlockd of the kill, but
	 * lvmlockd can use this information to avoid attempting any new lock
	 * requests in the VG (which would fail anyway), and can return an
	 * error indicating that the VG has been killed.
	 */

	reply = _lvmlockd_send("kill_vg",
				"cmd = %s", "lvmlockctl",
				"pid = " FMTd64, (int64_t) getpid(),
				"vg_name = %s", arg_vg_name,
				NULL);

	if (!_lvmlockd_result(reply, &result)) {
		log_error("lvmlockd result %d", result);
		rv = result;
	} else {
		rv = 0;
	}

	daemon_reply_destroy(reply);

	/*
	 * FIXME: here is where we should implement a strong form of
	 * blkdeactivate, and if it completes successfully, automatically call
	 * do_drop() afterward.  (The drop step may not always be necessary
	 * if the lvm commands run while shutting things down release all the
	 * leases.)
	 *
	 * run_strong_blkdeactivate();
	 * do_drop();
	 */

	return rv;
}

static int do_drop(void)
{
	daemon_reply reply;
	int result;
	int rv;

	syslog(LOG_WARNING, "Dropping locks for VG %s.", arg_vg_name);

	/*
	 * Check for misuse by looking for any active LVs in the VG
	 * and refusing this operation if found?  One possible way
	 * to kill LVs (e.g. if fs cannot be unmounted) is to suspend
	 * them, or replace them with the error target.  In that
	 * case the LV will still appear to be active, but it is
	 * safe to release the lock.
	 */

	reply = _lvmlockd_send("drop_vg",
				"cmd = %s", "lvmlockctl",
				"pid = " FMTd64, (int64_t) getpid(),
				"vg_name = %s", arg_vg_name,
				NULL);

	if (!_lvmlockd_result(reply, &result)) {
		log_error("lvmlockd result %d", result);
		rv = result;
	} else {
		rv = 0;
	}

	daemon_reply_destroy(reply);
	return rv;
}

static void print_usage(void)
{
	printf("lvmlockctl options\n");
	printf("Options:\n");
	printf("--help | -h\n");
	printf("      Show this help information.\n");
	printf("--quit | -q\n");
	printf("      Tell lvmlockd to quit.\n");
	printf("--info | -i\n");
	printf("      Print lock state information from lvmlockd.\n");
	printf("--dump | -d\n");
	printf("      Print log buffer from lvmlockd.\n");
	printf("--wait | -w 0|1\n");
	printf("      Wait option for other commands.\n");
	printf("--force | -f 0|1>\n");
	printf("      Force option for other commands.\n");
	printf("--kill | -k <vgname>\n");
	printf("      Kill access to the VG when sanlock cannot renew lease.\n");
	printf("--drop | -r <vgname>\n");
	printf("      Clear locks for the VG when it is unused after kill (-k).\n");
	printf("--gl-enable | -E <vgname>\n");
	printf("      Tell lvmlockd to enable the global lock in a sanlock VG.\n");
	printf("--gl-disable | -D <vgname>\n");
	printf("      Tell lvmlockd to disable the global lock in a sanlock VG.\n");
	printf("--stop-lockspaces | -S\n");
	printf("      Stop all lockspaces.\n");
}

static int read_options(int argc, char *argv[])
{
	int option_index = 0;
	int c;

	static struct option long_options[] = {
		{"help",            no_argument,       0,  'h' },
		{"quit",            no_argument,       0,  'q' },
		{"info",            no_argument,       0,  'i' },
		{"dump",            no_argument,       0,  'd' },
		{"wait",            required_argument, 0,  'w' },
		{"force",           required_argument, 0,  'f' },
		{"kill",            required_argument, 0,  'k' },
		{"drop",            required_argument, 0,  'r' },
		{"gl-enable",       required_argument, 0,  'E' },
		{"gl-disable",      required_argument, 0,  'D' },
		{"stop-lockspaces", no_argument,       0,  'S' },
		{0, 0, 0, 0 }
	};

	if (argc == 1) {
		print_usage();
		exit(0);
	}

	while (1) {
		c = getopt_long(argc, argv, "hqidE:D:w:k:r:S", long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 'h':
			/* --help */
			print_usage();
			exit(0);
		case 'q':
			/* --quit */
			quit = 1;
			break;
		case 'i':
			/* --info */
			info = 1;
			break;
		case 'd':
			/* --dump */
			dump = 1;
			break;
		case 'w':
			wait_opt = atoi(optarg);
			break;
		case 'k':
			kill_vg = 1;
			arg_vg_name = strdup(optarg);
			break;
		case 'r':
			drop_vg = 1;
			arg_vg_name = strdup(optarg);
			break;
		case 'E':
			gl_enable = 1;
			arg_vg_name = strdup(optarg);
			break;
		case 'D':
			gl_disable = 1;
			arg_vg_name = strdup(optarg);
			break;
		case 'S':
			stop_lockspaces = 1;
			break;
		default:
			print_usage();
			exit(1);
		}
	}


	return 0;
}

int main(int argc, char **argv)
{
	int rv = 0;

	rv = read_options(argc, argv);
	if (rv < 0)
		return rv;

	_lvmlockd = lvmlockd_open(NULL);

	if (_lvmlockd.socket_fd < 0 || _lvmlockd.error) {
		log_error("Cannot connect to lvmlockd.");
		return -1;
	}

	if (quit) {
		rv = do_quit();
		goto out;
	}

	if (info) {
		rv = do_dump("info");
		goto out;
	}

	if (dump) {
		rv = do_dump("dump");
		goto out;
	}

	if (kill_vg) {
		rv = do_kill();
		goto out;
	}

	if (drop_vg) {
		rv = do_drop();
		goto out;
	}

	if (gl_enable) {
		syslog(LOG_INFO, "Enabling global lock in VG %s.", arg_vg_name);
		rv = do_able("enable_gl");
		goto out;
	}

	if (gl_disable) {
		syslog(LOG_INFO, "Disabling global lock in VG %s.", arg_vg_name);
		rv = do_able("disable_gl");
		goto out;
	}

	if (stop_lockspaces) {
		rv = do_stop_lockspaces();
		goto out;
	}

out:
	lvmlockd_close(_lvmlockd);
	return rv;
}
