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

#include "lvmpolld-common.h"

#include "lvm-version.h"
#include "daemon-server.h"
#include "daemon-log.h"

#include <getopt.h>
#include <poll.h>
#include <wait.h>

#define LVMPOLLD_SOCKET DEFAULT_RUN_DIR "/lvmpolld.socket"

#define PD_LOG_PREFIX "LVMPOLLD"
#define LVM2_LOG_PREFIX "\tLVPOLL"

/* predefined reason for response = "failed" case */
#define REASON_REQ_NOT_IMPLEMENTED "request not implemented"
#define REASON_MISSING_LVID "request requires lvid set"
#define REASON_MISSING_LVNAME "request requires lvname set"
#define REASON_MISSING_VGNAME "request requires vgname set"
#define REASON_POLLING_FAILED "polling of lvm command failed"
#define REASON_ILLEGAL_ABORT_REQUEST "abort only supported with PVMOVE polling operation"
#define REASON_DIFFERENT_OPERATION_IN_PROGRESS "Different operation on LV already in progress"
#define REASON_INVALID_INTERVAL "request requires interval set"
#define REASON_ENOMEM "not enough memory"

struct lvmpolld_state {
	daemon_idle *idle;
	log_state *log;
	const char *log_config;
	const char *lvm_binary;

	struct lvmpolld_store *id_to_pdlv_abort;
	struct lvmpolld_store *id_to_pdlv_poll;
};

static pthread_key_t key;

static const char *_strerror_r(int errnum, struct lvmpolld_thread_data *data)
{
#ifdef _GNU_SOURCE
	return strerror_r(errnum, data->buf, sizeof(data->buf)); /* never returns NULL */
#elif (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
	return strerror_r(errnum, data->buf, sizeof(data->buf)) ? "" : data->buf;
#else
#	warning "Can't decide proper strerror_r implementation. lvmpolld will not issue specific system error messages"
	return "";
#endif
}

static void _usage(const char *prog, FILE *file)
{
	fprintf(file, "Usage:\n"
		"%s [-V] [-h] [-f] [-l {all|wire|debug}] [-s path] [-B path] [-p path] [-t secs]\n"
		"%s --dump [-s path]\n"
		"   -V|--version     Show version info\n"
		"   -h|--help        Show this help information\n"
		"   -f|--foreground  Don't fork, run in the foreground\n"
		"   --dump           Dump full lvmpolld state\n"
		"   -l|--log         Logging message level (-l {all|wire|debug})\n"
		"   -p|--pidfile     Set path to the pidfile\n"
		"   -s|--socket      Set path to the communication socket\n"
		"   -B|--binary      Path to lvm2 binary\n"
		"   -t|--timeout     Time to wait in seconds before shutdown on idle (missing or 0 = inifinite)\n\n", prog, prog);
}

static int _init(struct daemon_state *s)
{
	struct lvmpolld_state *ls = s->private;
	ls->log = s->log;

	/*
	 * log warnings to stderr by default. Otherwise we would miss any lvpoll
	 * error messages in default configuration
	 */
	daemon_log_enable(ls->log, DAEMON_LOG_OUTLET_STDERR, DAEMON_LOG_WARN,  1);

	if (!daemon_log_parse(ls->log, DAEMON_LOG_OUTLET_STDERR, ls->log_config, 1))
		return 0;

	if (pthread_key_create(&key, lvmpolld_thread_data_destroy)) {
		FATAL(ls, "%s: %s", PD_LOG_PREFIX, "Failed to create pthread key");
		return 0;
	}

	ls->id_to_pdlv_poll = pdst_init("polling");
	ls->id_to_pdlv_abort = pdst_init("abort");

	if (!ls->id_to_pdlv_poll || !ls->id_to_pdlv_abort) {
		FATAL(ls, "%s: %s", PD_LOG_PREFIX, "Failed to allocate internal data structures");
		return 0;
	}

	ls->lvm_binary = ls->lvm_binary ?: LVM_PATH;

	if (access(ls->lvm_binary, X_OK)) {
		FATAL(ls, "%s: %s %s", PD_LOG_PREFIX, "Execute access rights denied on", ls->lvm_binary);
		return 0;
	}

	if (ls->idle)
		ls->idle->is_idle = 1;

	return 1;
}

static void _lvmpolld_stores_lock(struct lvmpolld_state *ls)
{
	pdst_lock(ls->id_to_pdlv_poll);
	pdst_lock(ls->id_to_pdlv_abort);
}

static void _lvmpolld_stores_unlock(struct lvmpolld_state *ls)
{
	pdst_unlock(ls->id_to_pdlv_abort);
	pdst_unlock(ls->id_to_pdlv_poll);
}

static void _lvmpolld_global_lock(struct lvmpolld_state *ls)
{
	_lvmpolld_stores_lock(ls);

	pdst_locked_lock_all_pdlvs(ls->id_to_pdlv_poll);
	pdst_locked_lock_all_pdlvs(ls->id_to_pdlv_abort);
}

static void _lvmpolld_global_unlock(struct lvmpolld_state *ls)
{
	pdst_locked_unlock_all_pdlvs(ls->id_to_pdlv_abort);
	pdst_locked_unlock_all_pdlvs(ls->id_to_pdlv_poll);

	_lvmpolld_stores_unlock(ls);
}

static int _fini(struct daemon_state *s)
{
	int done;
	const struct timespec t = { .tv_nsec = 250000000 }; /* .25 sec */
	struct lvmpolld_state *ls = s->private;

	DEBUGLOG(s, "fini");

	DEBUGLOG(s, "sending cancel requests");

	_lvmpolld_global_lock(ls);
	pdst_locked_send_cancel(ls->id_to_pdlv_poll);
	pdst_locked_send_cancel(ls->id_to_pdlv_abort);
	_lvmpolld_global_unlock(ls);

	DEBUGLOG(s, "waiting for background threads to finish");

	while(1) {
		_lvmpolld_stores_lock(ls);
		done = !pdst_locked_get_active_count(ls->id_to_pdlv_poll) &&
		       !pdst_locked_get_active_count(ls->id_to_pdlv_abort);
		_lvmpolld_stores_unlock(ls);
		if (done)
			break;
		nanosleep(&t, NULL);
	}

	DEBUGLOG(s, "destroying internal data structures");

	_lvmpolld_stores_lock(ls);
	pdst_locked_destroy_all_pdlvs(ls->id_to_pdlv_poll);
	pdst_locked_destroy_all_pdlvs(ls->id_to_pdlv_abort);
	_lvmpolld_stores_unlock(ls);

	pdst_destroy(ls->id_to_pdlv_poll);
	pdst_destroy(ls->id_to_pdlv_abort);

	pthread_key_delete(key);

	return 1;
}

static response reply(const char *res, const char *reason)
{
	return daemon_reply_simple(res, "reason = %s", reason, NULL);
}

static int read_single_line(struct lvmpolld_thread_data *data, int err)
{
	ssize_t r = getline(&data->line, &data->line_size, err ? data->ferr : data->fout);

	if (r > 0 && *(data->line + r - 1) == '\n')
		*(data->line + r - 1) = '\0';

	return (r > 0);
}

static void update_idle_state(struct lvmpolld_state *ls)
{
	if (!ls->idle)
		return;

	_lvmpolld_stores_lock(ls);

	ls->idle->is_idle = !pdst_locked_get_active_count(ls->id_to_pdlv_poll) &&
			    !pdst_locked_get_active_count(ls->id_to_pdlv_abort);

	_lvmpolld_stores_unlock(ls);

	DEBUGLOG(ls, "%s: %s %s%s", PD_LOG_PREFIX, "daemon is", ls->idle->is_idle ? "" : "not ", "idle");
}

/* make this configurable */
#define MAX_TIMEOUT 2

static int poll_for_output(struct lvmpolld_lv *pdlv, struct lvmpolld_thread_data *data)
{
	int ch_stat, r, err = 1, fds_count = 2, timeout = 0;
	pid_t pid;
	struct lvmpolld_cmd_stat cmd_state = { .retcode = -1, .signal = 0 };
	struct pollfd fds[] = { { .fd = data->outpipe[0], .events = POLLIN },
				{ .fd = data->errpipe[0], .events = POLLIN } };

	if (!(data->fout = fdopen(data->outpipe[0], "r")) || !(data->ferr = fdopen(data->errpipe[0], "r"))) {
		ERROR(pdlv->ls, "%s: %s: (%d) %s", PD_LOG_PREFIX, "failed to open file stream",
		      errno, _strerror_r(errno, data));
		goto out;
	}

	while (1) {
		do {
			r = poll(fds, 2, pdlv_get_timeout(pdlv) * 1000);
		} while (r < 0 && errno == EINTR);

		DEBUGLOG(pdlv->ls, "%s: %s %d", PD_LOG_PREFIX, "poll() returned", r);
		if (r < 0) {
			ERROR(pdlv->ls, "%s: %s (PID %d) failed: (%d) %s",
			      PD_LOG_PREFIX, "poll() for LVM2 cmd", pdlv->cmd_pid,
			      errno, _strerror_r(errno, data));
			goto out;
		} else if (!r) {
			timeout++;

			WARN(pdlv->ls, "%s: %s (PID %d) %s", PD_LOG_PREFIX,
			     "polling for output of the lvm cmd", pdlv->cmd_pid,
			     "has timed out");

			if (timeout > MAX_TIMEOUT) {
				ERROR(pdlv->ls, "%s: %s (PID %d) (no output for %d seconds)",
				      PD_LOG_PREFIX,
				      "LVM2 cmd is unresponsive too long",
				      pdlv->cmd_pid,
				      timeout * pdlv_get_timeout(pdlv));
				goto out;
			}

			continue; /* while(1) */
		}

		timeout = 0;

		/* handle the command's STDOUT */
		if (fds[0].revents & POLLIN) {
			DEBUGLOG(pdlv->ls, "%s: %s", PD_LOG_PREFIX, "caught input data in STDOUT");

			assert(read_single_line(data, 0)); /* may block indef. anyway */
			INFO(pdlv->ls, "%s: PID %d: %s: '%s'", LVM2_LOG_PREFIX,
			     pdlv->cmd_pid, "STDOUT", data->line);
		} else if (fds[0].revents) {
			if (fds[0].revents & POLLHUP)
				DEBUGLOG(pdlv->ls, "%s: %s", PD_LOG_PREFIX, "caught POLLHUP");
			else
				WARN(pdlv->ls, "%s: %s", PD_LOG_PREFIX, "poll for command's STDOUT failed");

			fds[0].fd = -1;
			fds_count--;
		}

		/* handle the command's STDERR */
		if (fds[1].revents & POLLIN) {
			DEBUGLOG(pdlv->ls, "%s: %s", PD_LOG_PREFIX,
				 "caught input data in STDERR");

			assert(read_single_line(data, 1)); /* may block indef. anyway */
			WARN(pdlv->ls, "%s: PID %d: %s: '%s'", LVM2_LOG_PREFIX,
			     pdlv->cmd_pid, "STDERR", data->line);
		} else if (fds[1].revents) {
			if (fds[1].revents & POLLHUP)
				DEBUGLOG(pdlv->ls, "%s: %s", PD_LOG_PREFIX, "caught err POLLHUP");
			else
				WARN(pdlv->ls, "%s: %s", PD_LOG_PREFIX, "poll for command's STDOUT failed");

			fds[1].fd = -1;
			fds_count--;
		}

		do {
			/*
			 * fds_count == 0 means polling reached EOF
			 * or received error on both descriptors.
			 * In such case, just wait for command to finish
			 */
			pid = waitpid(pdlv->cmd_pid, &ch_stat, fds_count ? WNOHANG : 0);
		} while (pid < 0 && errno == EINTR);

		if (pid) {
			if (pid < 0) {
				ERROR(pdlv->ls, "%s: %s (PID %d) failed: (%d) %s",
				      PD_LOG_PREFIX, "waitpid() for lvm2 cmd",
				      pdlv->cmd_pid, errno,
				      _strerror_r(errno, data));
				goto out;
			}
			DEBUGLOG(pdlv->ls, "%s: %s", PD_LOG_PREFIX, "child exited");
			break;
		}
	} /* while(1) */

	DEBUGLOG(pdlv->ls, "%s: %s", PD_LOG_PREFIX, "about to collect remaining lines");
	if (fds[0].fd >= 0)
		while (read_single_line(data, 0)) {
			assert(r > 0);
			INFO(pdlv->ls, "%s: PID %d: %s: %s", LVM2_LOG_PREFIX, pdlv->cmd_pid, "STDOUT", data->line);
		}
	if (fds[1].fd >= 0)
		while (read_single_line(data, 1)) {
			assert(r > 0);
			WARN(pdlv->ls, "%s: PID %d: %s: %s", LVM2_LOG_PREFIX, pdlv->cmd_pid, "STDERR", data->line);
		}

	if (WIFEXITED(ch_stat)) {
		cmd_state.retcode = WEXITSTATUS(ch_stat);
		if (cmd_state.retcode)
			ERROR(pdlv->ls, "%s: %s (PID %d) %s (retcode: %d)", PD_LOG_PREFIX,
			     "lvm2 cmd", pdlv->cmd_pid, "failed", cmd_state.retcode);
		else
			INFO(pdlv->ls, "%s: %s (PID %d) %s", PD_LOG_PREFIX,
			     "lvm2 cmd", pdlv->cmd_pid, "finished successfully");
	} else if (WIFSIGNALED(ch_stat)) {
		ERROR(pdlv->ls, "%s: %s (PID %d) %s (%d)", PD_LOG_PREFIX,
		     "lvm2 cmd", pdlv->cmd_pid, "got terminated by signal",
		     WTERMSIG(ch_stat));
		cmd_state.signal = WTERMSIG(ch_stat);
	}

	err = 0;
out:
	if (!err)
		pdlv_set_cmd_state(pdlv, &cmd_state);

	return err;
}

static void debug_print(struct lvmpolld_state *ls, const char * const* ptr)
{
	const char * const* tmp = ptr;

	if (!tmp)
		return;

	while (*tmp) {
		DEBUGLOG(ls, "%s: %s", PD_LOG_PREFIX, *tmp);
		tmp++;
	}
}

static void *fork_and_poll(void *args)
{
	int outfd, errfd, state;
	struct lvmpolld_thread_data *data;
	pid_t r;

	int error = 1;
	struct lvmpolld_lv *pdlv = (struct lvmpolld_lv *) args;
	struct lvmpolld_state *ls = pdlv->ls;

	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &state);
	data = lvmpolld_thread_data_constructor(pdlv);
	pthread_setspecific(key, data);
	pthread_setcancelstate(state, &state);

	if (!data) {
		ERROR(ls, "%s: %s", PD_LOG_PREFIX, "Failed to initialize per-thread data");
		goto err;
	}

	DEBUGLOG(ls, "%s: %s", PD_LOG_PREFIX, "cmd line arguments:");
	debug_print(ls, pdlv->cmdargv);
	DEBUGLOG(ls, "%s: %s", PD_LOG_PREFIX, "---end---");

	DEBUGLOG(ls, "%s: %s", PD_LOG_PREFIX, "cmd environment variables:");
	debug_print(ls, pdlv->cmdenvp);
	DEBUGLOG(ls, "%s: %s", PD_LOG_PREFIX, "---end---");

	outfd = data->outpipe[1];
	errfd = data->errpipe[1];

	r = fork();
	if (!r) {
		/* child */
		/* !!! Do not touch any posix thread primitives !!! */

		if ((dup2(outfd, STDOUT_FILENO ) != STDOUT_FILENO) ||
		    (dup2(errfd, STDERR_FILENO ) != STDERR_FILENO))
			_exit(LVMPD_RET_DUP_FAILED);

		execve(*(pdlv->cmdargv), (char *const *)pdlv->cmdargv, (char *const *)pdlv->cmdenvp);

		_exit(LVMPD_RET_EXC_FAILED);
	} else {
		/* parent */
		if (r == -1) {
			ERROR(ls, "%s: %s: (%d) %s", PD_LOG_PREFIX, "fork failed",
			      errno, _strerror_r(errno, data));
			goto err;
		}

		INFO(ls, "%s: LVM2 cmd \"%s\" (PID: %d)", PD_LOG_PREFIX, *(pdlv->cmdargv), r);

		pdlv->cmd_pid = r;

		/* failure to close write end of any pipe will result in broken polling */
		if (close(data->outpipe[1])) {
			ERROR(ls, "%s: %s: (%d) %s", PD_LOG_PREFIX, "failed to close write end of pipe",
			      errno, _strerror_r(errno, data));
			goto err;
		}
		data->outpipe[1] = -1;

		if (close(data->errpipe[1])) {
			ERROR(ls, "%s: %s: (%d) %s", PD_LOG_PREFIX, "failed to close write end of err pipe",
			      errno, _strerror_r(errno, data));
			goto err;
		}
		data->errpipe[1] = -1;

		error = poll_for_output(pdlv, data);
		DEBUGLOG(ls, "%s: %s", PD_LOG_PREFIX, "polling for lvpoll output has finished");
	}

err:
	r = 0;

	pdst_lock(pdlv->pdst);

	if (error) {
		/* last reader is responsible for pdlv cleanup */
		r = pdlv->cmd_pid;
		pdlv_set_error(pdlv, 1);
	}

	pdlv_set_polling_finished(pdlv, 1);
	if (data)
		data->pdlv = NULL;

	pdst_locked_dec(pdlv->pdst);

	pdst_unlock(pdlv->pdst);

	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &state);
	lvmpolld_thread_data_destroy(data);
	pthread_setspecific(key, NULL);
	pthread_setcancelstate(state, &state);

	update_idle_state(ls);

	/*
	 * This is unfortunate case where we
	 * know nothing about state of lvm cmd and
	 * (eventually) ongoing progress.
	 *
	 * harvest zombies
	 */
	if (r)
		while(waitpid(r, NULL, 0) < 0 && errno == EINTR);

	return NULL;
}

static response progress_info(client_handle h, struct lvmpolld_state *ls, request req)
{
	char *id;
	struct lvmpolld_lv *pdlv;
	struct lvmpolld_store *pdst;
	struct lvmpolld_lv_state st;
	response r;
	const char *lvid = daemon_request_str(req, LVMPD_PARM_LVID, NULL);
	const char *sysdir = daemon_request_str(req, LVMPD_PARM_SYSDIR, NULL);
	unsigned abort_polling = daemon_request_int(req, LVMPD_PARM_ABORT, 0);

	if (!lvid)
		return reply(LVMPD_RESP_FAILED, REASON_MISSING_LVID);

	id = construct_id(sysdir, lvid);
	if (!id) {
		ERROR(ls, "%s: %s", PD_LOG_PREFIX, "progress_info request failed to construct ID.");
		return reply(LVMPD_RESP_FAILED, REASON_ENOMEM);
	}

	DEBUGLOG(ls, "%s: %s: %s", PD_LOG_PREFIX, "ID", id);

	pdst = abort_polling ? ls->id_to_pdlv_abort : ls->id_to_pdlv_poll;

	pdst_lock(pdst);

	pdlv = pdst_locked_lookup(pdst, id);
	if (pdlv) {
		/*
		 * with store lock held, I'm the only reader accessing the pdlv
		 */
		st = pdlv_get_status(pdlv);

		if (st.error || st.polling_finished) {
			INFO(ls, "%s: %s %s", PD_LOG_PREFIX,
			     "Polling finished. Removing related data structure for LV",
			     lvid);
			pdst_locked_remove(pdst, id);
			pdlv_destroy(pdlv);
		}
	}
	/* pdlv must not be dereferenced from now on */

	pdst_unlock(pdst);

	free(id);

	if (pdlv) {
		if (st.error)
			return reply(LVMPD_RESP_FAILED, REASON_POLLING_FAILED);

		if (st.polling_finished)
			r = daemon_reply_simple(LVMPD_RESP_FINISHED,
						"reason = %s", st.cmd_state.signal ? LVMPD_REAS_SIGNAL : LVMPD_REAS_RETCODE,
						LVMPD_PARM_VALUE " = " FMTd64, (int64_t)(st.cmd_state.signal ?: st.cmd_state.retcode),
						NULL);
		else
			r = daemon_reply_simple(LVMPD_RESP_IN_PROGRESS, NULL);
	}
	else
		r = daemon_reply_simple(LVMPD_RESP_NOT_FOUND, NULL);

	return r;
}

static struct lvmpolld_lv *construct_pdlv(request req, struct lvmpolld_state *ls,
				     struct lvmpolld_store *pdst,
				     const char *interval, const char *id,
				     const char *vgname, const char *lvname,
				     const char *sysdir, enum poll_type type,
				     unsigned abort_polling, unsigned uinterval)
{
	const char **cmdargv, **cmdenvp;
	struct lvmpolld_lv *pdlv;
	unsigned handle_missing_pvs = daemon_request_int(req, LVMPD_PARM_HANDLE_MISSING_PVS, 0);

	pdlv = pdlv_create(ls, id, vgname, lvname, sysdir, type,
			   interval, uinterval, pdst);

	if (!pdlv) {
		ERROR(ls, "%s: %s", PD_LOG_PREFIX, "failed to create internal LV data structure.");
		return NULL;
	}

	cmdargv = cmdargv_ctr(pdlv, pdlv->ls->lvm_binary, abort_polling, handle_missing_pvs);
	if (!cmdargv) {
		pdlv_destroy(pdlv);
		ERROR(ls, "%s: %s", PD_LOG_PREFIX, "failed to construct cmd arguments for lvpoll command");
		return NULL;
	}

	pdlv->cmdargv = cmdargv;

	cmdenvp = cmdenvp_ctr(pdlv);
	if (!cmdenvp) {
		pdlv_destroy(pdlv);
		ERROR(ls, "%s: %s", PD_LOG_PREFIX, "failed to construct cmd environment for lvpoll command");
		return NULL;
	}

	pdlv->cmdenvp = cmdenvp;

	return pdlv;
}

static int spawn_detached_thread(struct lvmpolld_lv *pdlv)
{
	int r;
	pthread_attr_t attr;

	if (pthread_attr_init(&attr) != 0)
		return 0;

	if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0)
		return 0;

	r = pthread_create(&pdlv->tid, &attr, fork_and_poll, (void *)pdlv);

	if (pthread_attr_destroy(&attr) != 0)
		return 0;

	return !r;
}

static response poll_init(client_handle h, struct lvmpolld_state *ls, request req, enum poll_type type)
{
	char *id;
	struct lvmpolld_lv *pdlv;
	struct lvmpolld_store *pdst;
	unsigned uinterval;

	const char *interval = daemon_request_str(req, LVMPD_PARM_INTERVAL, NULL);
	const char *lvid = daemon_request_str(req, LVMPD_PARM_LVID, NULL);
	const char *lvname = daemon_request_str(req, LVMPD_PARM_LVNAME, NULL);
	const char *vgname = daemon_request_str(req, LVMPD_PARM_VGNAME, NULL);
	const char *sysdir = daemon_request_str(req, LVMPD_PARM_SYSDIR, NULL);
	unsigned abort_polling = daemon_request_int(req, LVMPD_PARM_ABORT, 0);

	assert(type < POLL_TYPE_MAX);

	if (abort_polling && type != PVMOVE)
		return reply(LVMPD_RESP_EINVAL, REASON_ILLEGAL_ABORT_REQUEST);

	if (!interval || strpbrk(interval, "-") || sscanf(interval, "%u", &uinterval) != 1)
		return reply(LVMPD_RESP_EINVAL, REASON_INVALID_INTERVAL);

	if (!lvname)
		return reply(LVMPD_RESP_FAILED, REASON_MISSING_LVNAME);

	if (!lvid)
		return reply(LVMPD_RESP_FAILED, REASON_MISSING_LVID);

	if (!vgname)
		return reply(LVMPD_RESP_FAILED, REASON_MISSING_VGNAME);

	id = construct_id(sysdir, lvid);
	if (!id) {
		ERROR(ls, "%s: %s", PD_LOG_PREFIX, "poll_init request failed to construct ID.");
		return reply(LVMPD_RESP_FAILED, REASON_ENOMEM);
	}

	DEBUGLOG(ls, "%s: %s=%s", PD_LOG_PREFIX, "ID", id);

	pdst = abort_polling ? ls->id_to_pdlv_abort : ls->id_to_pdlv_poll;

	pdst_lock(pdst);

	pdlv = pdst_locked_lookup(pdst, id);
	if (pdlv && pdlv_get_polling_finished(pdlv)) {
		WARN(ls, "%s: %s %s", PD_LOG_PREFIX, "Force removal of uncollected info for LV",
			 lvid);
		/* 
		 * lvmpolld has to remove uncollected results in this case.
		 * otherwise it would have to refuse request for new polling
		 * lv with same id.
		 */
		pdst_locked_remove(pdst, id);
		pdlv_destroy(pdlv);
		pdlv = NULL;
	}

	if (pdlv) {
		if (!pdlv_is_type(pdlv, type)) {
			pdst_unlock(pdst);
			ERROR(ls, "%s: %s '%s': expected: %s, requested: %s",
			      PD_LOG_PREFIX, "poll operation type mismatch on LV identified by",
			      id,
			      polling_op(pdlv_get_type(pdlv)), polling_op(type));
			free(id);
			return reply(LVMPD_RESP_EINVAL,
				     REASON_DIFFERENT_OPERATION_IN_PROGRESS);
		}
		pdlv->init_rq_count++; /* safe. protected by store lock */
	} else {
		pdlv = construct_pdlv(req, ls, pdst, interval, id, vgname,
				      lvname, sysdir, type, abort_polling, 2 * uinterval);
		if (!pdlv) {
			pdst_unlock(pdst);
			free(id);
			return reply(LVMPD_RESP_FAILED, REASON_ENOMEM);
		}
		if (!pdst_locked_insert(pdst, id, pdlv)) {
			pdlv_destroy(pdlv);
			pdst_unlock(pdst);
			ERROR(ls, "%s: %s", PD_LOG_PREFIX, "couldn't store internal LV data structure");
			free(id);
			return reply(LVMPD_RESP_FAILED, REASON_ENOMEM);
		}
		if (!spawn_detached_thread(pdlv)) {
			ERROR(ls, "%s: %s", PD_LOG_PREFIX, "failed to spawn detached monitoring thread");
			pdst_locked_remove(pdst, id);
			pdlv_destroy(pdlv);
			pdst_unlock(pdst);
			free(id);
			return reply(LVMPD_RESP_FAILED, REASON_ENOMEM);
		}

		pdst_locked_inc(pdst);
		if (ls->idle)
			ls->idle->is_idle = 0;
	}

	pdst_unlock(pdst);

	free(id);

	return daemon_reply_simple(LVMPD_RESP_OK, NULL);
}

static response dump_state(client_handle h, struct lvmpolld_state *ls, request r)
{
	response res = { 0 };
	struct buffer *b = &res.buffer;

	buffer_init(b);

	_lvmpolld_global_lock(ls);

	buffer_append(b, "# Registered polling operations\n\n");
	buffer_append(b, "poll {\n");
	pdst_locked_dump(ls->id_to_pdlv_poll, b);
	buffer_append(b, "}\n\n");

	buffer_append(b, "# Registered abort operations\n\n");
	buffer_append(b, "abort {\n");
	pdst_locked_dump(ls->id_to_pdlv_abort, b);
	buffer_append(b, "}");

	_lvmpolld_global_unlock(ls);

	return res;
}

static response _handler(struct daemon_state s, client_handle h, request r)
{
	struct lvmpolld_state *ls = s.private;
	const char *rq = daemon_request_str(r, "request", "NONE");

	if (!strcmp(rq, LVMPD_REQ_PVMOVE))
		return poll_init(h, ls, r, PVMOVE);
	else if (!strcmp(rq, LVMPD_REQ_CONVERT))
		return poll_init(h, ls, r, CONVERT);
	else if (!strcmp(rq, LVMPD_REQ_MERGE))
		return poll_init(h, ls, r, MERGE);
	else if (!strcmp(rq, LVMPD_REQ_MERGE_THIN))
		return poll_init(h, ls, r, MERGE_THIN);
	else if (!strcmp(rq, LVMPD_REQ_PROGRESS))
		return progress_info(h, ls, r);
	else if (!strcmp(rq, LVMPD_REQ_DUMP))
		return dump_state(h, ls, r);
	else
		return reply(LVMPD_RESP_EINVAL, REASON_REQ_NOT_IMPLEMENTED);
}

static int process_timeout_arg(const char *str, unsigned *max_timeouts)
{
	char *endptr;
	unsigned long l;

	errno = 0;
	l = strtoul(str, &endptr, 10);
	if (errno || *endptr || l >= UINT_MAX)
		return 0;

	*max_timeouts = (unsigned) l;

	return 1;
}

/* Client functionality */
typedef int (*action_fn_t) (void *args);

struct log_line_baton {
	const char *prefix;
};

daemon_handle _lvmpolld = { .error = 0 };

static daemon_handle _lvmpolld_open(const char *socket)
{
	daemon_info lvmpolld_info = {
		.path = "lvmpolld",
		.socket = socket ?: DEFAULT_RUN_DIR "/lvmpolld.socket",
		.protocol = LVMPOLLD_PROTOCOL,
		.protocol_version = LVMPOLLD_PROTOCOL_VERSION
	};

	return daemon_open(lvmpolld_info);
}

static void _log_line(const char *line, void *baton) {
	struct log_line_baton *b = baton;
	fprintf(stdout, "%s%s\n", b->prefix, line);
}

static int printout_raw_response(const char *prefix, const char *msg)
{
	struct log_line_baton b = { .prefix = prefix };
	char *buf;
	char *pos;

	buf = strdup(msg);
	pos = buf;

	if (!buf)
		return 0;

	while (pos) {
		char *next = strchr(pos, '\n');
		if (next)
			*next = 0;
		_log_line(pos, &b);
		pos = next ? next + 1 : 0;
	}
	free(buf);

	return 1;
}

/* place all action implementations below */

static int action_dump(void *args __attribute__((unused)))
{
	daemon_request req;
	daemon_reply repl;
	int r = 0;

	req = daemon_request_make(LVMPD_REQ_DUMP);
	if (!req.cft) {
		fprintf(stderr, "Failed to create lvmpolld " LVMPD_REQ_DUMP " request.\n");
		goto out_req;
	}

	repl = daemon_send(_lvmpolld, req);
	if (repl.error) {
		fprintf(stderr, "Failed to send a request or receive response.\n");
		goto  out_rep;
	}

	/*
	 * This is dumb copy & paste from libdaemon log routines.
	 */
	if (!printout_raw_response("  ", repl.buffer.mem)) {
		fprintf(stderr, "Failed to print out the response.\n");
		goto  out_rep;
	}

	r = 1;

out_rep:
	daemon_reply_destroy(repl);
out_req:
	daemon_request_destroy(req);

	return r;
}

enum action_index {
	ACTION_DUMP = 0,
	ACTION_MAX /* keep at the end */
};

static const action_fn_t actions[ACTION_MAX] = { [ACTION_DUMP] = action_dump };

static int _make_action(enum action_index idx, void *args)
{
	return idx < ACTION_MAX ? actions[idx](args) : 0;
}

static int _lvmpolld_client(const char *socket, unsigned action)
{
	int r;

	_lvmpolld = _lvmpolld_open(socket);

	if (_lvmpolld.error || _lvmpolld.socket_fd < 0) {
		fprintf(stderr, "Failed to establish connection with lvmpolld.\n");
		return 0;
	}

	r = _make_action(action, NULL);

	daemon_close(_lvmpolld);

	return r ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int action_idx = ACTION_MAX;
static struct option long_options[] = {
	/* Have actions always at the beginning of the array. */
	{"dump",	no_argument,		&action_idx,	ACTION_DUMP }, /* or an option_index ? */

	/* other options */
	{"binary",	required_argument,	0,		'B' },
	{"foreground",	no_argument,		0,		'f' },
	{"help",	no_argument,		0,		'h' },
	{"log",		required_argument,	0,		'l' },
	{"pidfile",	required_argument,	0,		'p' },
	{"socket",	required_argument,	0,		's' },
	{"timeout",	required_argument,	0,		't' },
	{"version",	no_argument,		0,		'V' },
	{0,		0,			0,		0 }
};

int main(int argc, char *argv[])
{
	int opt;
	int option_index = 0;
	int client = 0, server = 0;
	unsigned action = ACTION_MAX;
	struct timeval timeout;
	daemon_idle di = { .ptimeout = &timeout };
	struct lvmpolld_state ls = { .log_config = "" };
	daemon_state s = {
		.daemon_fini = _fini,
		.daemon_init = _init,
		.handler = _handler,
		.name = "lvmpolld",
		.pidfile = getenv("LVM_LVMPOLLD_PIDFILE") ?: LVMPOLLD_PIDFILE,
		.private = &ls,
		.protocol = LVMPOLLD_PROTOCOL,
		.protocol_version = LVMPOLLD_PROTOCOL_VERSION,
		.socket_path = getenv("LVM_LVMPOLLD_SOCKET") ?: LVMPOLLD_SOCKET,
	};

	while ((opt = getopt_long(argc, argv, "fhVl:p:s:B:t:", long_options, &option_index)) != -1) {
		switch (opt) {
		case 0 :
			if (action < ACTION_MAX) {
				fprintf(stderr, "Can't perform more actions. Action already requested: %s\n",
					long_options[action].name);
				_usage(argv[0], stderr);
				exit(EXIT_FAILURE);
			}
			action = action_idx;
			client = 1;
			break;
		case '?':
			_usage(argv[0], stderr);
			exit(EXIT_FAILURE);
		case 'B': /* --binary */
			ls.lvm_binary = optarg;
			server = 1;
			break;
		case 'V': /* --version */
			printf("lvmpolld version: " LVM_VERSION "\n");
			exit(EXIT_SUCCESS);
		case 'f': /* --foreground */
			s.foreground = 1;
			server = 1;
			break;
		case 'h': /* --help */
			_usage(argv[0], stdout);
			exit(EXIT_SUCCESS);
		case 'l': /* --log */
			ls.log_config = optarg;
			server = 1;
			break;
		case 'p': /* --pidfile */
			s.pidfile = optarg;
			server = 1;
			break;
		case 's': /* --socket */
			s.socket_path = optarg;
			break;
		case 't': /* --timeout in seconds */
			if (!process_timeout_arg(optarg, &di.max_timeouts)) {
				fprintf(stderr, "Invalid value of timeout parameter.\n");
				exit(EXIT_FAILURE);
			}
			/* 0 equals to wait indefinitely */
			if (di.max_timeouts)
				s.idle = ls.idle = &di;
			server = 1;
			break;
		}
	}

	if (client && server) {
		fprintf(stderr, "Invalid combination of client and server parameters.\n\n");
		_usage(argv[0], stdout);
		exit(EXIT_FAILURE);
	}

	if (client)
		return _lvmpolld_client(s.socket_path, action);

	/* Server */
	daemon_start(s);

	return EXIT_SUCCESS;
}
