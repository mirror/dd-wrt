/*
 * Copyright (C) 2011-2017 Red Hat, Inc. All rights reserved.
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

#include "lib/misc/lib.h"
#include "daemons/dmeventd/plugins/lvm2/dmeventd_lvm.h"
#include "daemons/dmeventd/libdevmapper-event.h"

#include <sys/wait.h>
#include <stdarg.h>

/* TODO - move this mountinfo code into library to be reusable */
#ifdef __linux__
#  include "libdm/misc/kdev_t.h"
#else
#  define MAJOR(x) major((x))
#  define MINOR(x) minor((x))
#endif

/* First warning when thin data or metadata is 80% full. */
#define WARNING_THRESH	(DM_PERCENT_1 * 80)
/* Umount thin LVs when thin data or metadata LV is >=
 * and lvextend --use-policies has failed. */
#define UMOUNT_THRESH	(DM_PERCENT_1 * 95)
/* Run a check every 5%. */
#define CHECK_STEP	(DM_PERCENT_1 *  5)
/* Do not bother checking thin data or metadata is less than 50% full. */
#define CHECK_MINIMUM	(DM_PERCENT_1 * 50)

#define UMOUNT_COMMAND "/bin/umount"

#define MAX_FAILS	(256)  /* ~42 mins between cmd call retry with 10s delay */

#define THIN_DEBUG 0

struct dso_state {
	struct dm_pool *mem;
	int metadata_percent_check;
	int metadata_percent;
	int data_percent_check;
	int data_percent;
	uint64_t known_metadata_size;
	uint64_t known_data_size;
	unsigned fails;
	unsigned max_fails;
	int restore_sigset;
	sigset_t old_sigset;
	pid_t pid;
	char *argv[3];
	char *cmd_str;
};

DM_EVENT_LOG_FN("thin")

static int _run_command(struct dso_state *state)
{
	char val[16];
	int i;

	/* Mark for possible lvm2 command we are running from dmeventd
	 * lvm2 will not try to talk back to dmeventd while processing it */
	(void) setenv("LVM_RUN_BY_DMEVENTD", "1", 1);

	if (state->data_percent) {
		/* Prepare some known data to env vars for easy use */
		if (dm_snprintf(val, sizeof(val), "%d",
				state->data_percent / DM_PERCENT_1) != -1)
			(void) setenv("DMEVENTD_THIN_POOL_DATA", val, 1);
		if (dm_snprintf(val, sizeof(val), "%d",
				state->metadata_percent / DM_PERCENT_1) != -1)
			(void) setenv("DMEVENTD_THIN_POOL_METADATA", val, 1);
	} else {
		/* For an error event it's for a user to check status and decide */
		log_debug("Error event processing.");
	}

	log_verbose("Executing command: %s", state->cmd_str);

	/* TODO:
	 *   Support parallel run of 'task' and it's waitpid maintainence
	 *   ATM we can't handle signaling of  SIGALRM
	 *   as signalling is not allowed while 'process_event()' is running
	 */
	if (!(state->pid = fork())) {
		/* child */
		(void) close(0);
		for (i = 3; i < 255; ++i) (void) close(i);
		execvp(state->argv[0], state->argv);
		_exit(errno);
	} else if (state->pid == -1) {
		log_error("Can't fork command %s.", state->cmd_str);
		state->fails = 1;
		return 0;
	}

	return 1;
}

static int _use_policy(struct dm_task *dmt, struct dso_state *state)
{
#if THIN_DEBUG
	log_debug("dmeventd executes: %s.", state->cmd_str);
#endif
	if (state->argv[0])
		return _run_command(state);

	if (!dmeventd_lvm2_run_with_lock(state->cmd_str)) {
		log_error("Failed command for %s.", dm_task_get_name(dmt));
		state->fails = 1;
		return 0;
	}

	state->fails = 0;

	return 1;
}

/* Check if executed command has finished
 * Only 1 command may run */
static int _wait_for_pid(struct dso_state *state)
{
	int status = 0;

	if (state->pid == -1)
		return 1;

	if (!waitpid(state->pid, &status, WNOHANG))
		return 0;

	/* Wait for finish */
	if (WIFEXITED(status)) {
		log_verbose("Child %d exited with status %d.",
			    state->pid, WEXITSTATUS(status));
		state->fails = WEXITSTATUS(status) ? 1 : 0;
	} else {
		if (WIFSIGNALED(status))
			log_verbose("Child %d was terminated with status %d.",
				    state->pid, WTERMSIG(status));
		state->fails = 1;
	}

	state->pid = -1;

	return 1;
}

void process_event(struct dm_task *dmt,
		   enum dm_event_mask event __attribute__((unused)),
		   void **user)
{
	const char *device = dm_task_get_name(dmt);
	struct dso_state *state = *user;
	struct dm_status_thin_pool *tps = NULL;
	void *next = NULL;
	uint64_t start, length;
	char *target_type = NULL;
	char *params;
	int needs_policy = 0;
	struct dm_task *new_dmt = NULL;

#if THIN_DEBUG
	log_debug("Watch for tp-data:%.2f%%  tp-metadata:%.2f%%.",
		  dm_percent_to_round_float(state->data_percent_check, 2),
		  dm_percent_to_round_float(state->metadata_percent_check, 2));
#endif
	if (!_wait_for_pid(state)) {
		log_warn("WARNING: Skipping event, child %d is still running (%s).",
			 state->pid, state->cmd_str);
		return;
	}

	if (event & DM_EVENT_DEVICE_ERROR) {
		/* Error -> no need to check and do instant resize */
		state->data_percent = state->metadata_percent = 0;
		if (_use_policy(dmt, state))
			goto out;

		stack;

		/*
		 * Rather update oldish status
		 * since after 'command' processing
		 * percentage info could have changed a lot.
		 * If we would get above UMOUNT_THRESH
		 * we would wait for next sigalarm.
		 */
		if (!(new_dmt = dm_task_create(DM_DEVICE_STATUS)))
			goto_out;

		if (!dm_task_set_uuid(new_dmt, dm_task_get_uuid(dmt)))
			goto_out;

		/* Non-blocking status read */
		if (!dm_task_no_flush(new_dmt))
			log_warn("WARNING: Can't set no_flush for dm status.");

		if (!dm_task_run(new_dmt))
			goto_out;

		dmt = new_dmt;
	}

	dm_get_next_target(dmt, next, &start, &length, &target_type, &params);

	if (!target_type || (strcmp(target_type, "thin-pool") != 0)) {
		log_error("Invalid target type.");
		goto out;
	}

	if (!dm_get_status_thin_pool(state->mem, params, &tps)) {
		log_error("Failed to parse status.");
		goto out;
	}

#if THIN_DEBUG
	log_debug("Thin pool status " FMTu64 "/" FMTu64 "  "
		  FMTu64 "/" FMTu64 ".",
		  tps->used_metadata_blocks, tps->total_metadata_blocks,
		  tps->used_data_blocks, tps->total_data_blocks);
#endif

	/* Thin pool size had changed. Clear the threshold. */
	if (state->known_metadata_size != tps->total_metadata_blocks) {
		state->metadata_percent_check = CHECK_MINIMUM;
		state->known_metadata_size = tps->total_metadata_blocks;
		state->fails = 0;
	}

	if (state->known_data_size != tps->total_data_blocks) {
		state->data_percent_check = CHECK_MINIMUM;
		state->known_data_size = tps->total_data_blocks;
		state->fails = 0;
	}

	/*
	 * Trigger action when threshold boundary is exceeded.
	 * Report 80% threshold warning when it's used above 80%.
	 * Only 100% is exception as it cannot be surpased so policy
	 * action is called for:  >50%, >55% ... >95%, 100%
	 */
	state->metadata_percent = dm_make_percent(tps->used_metadata_blocks, tps->total_metadata_blocks);
	if ((state->metadata_percent > WARNING_THRESH) &&
	    (state->metadata_percent > state->metadata_percent_check))
		log_warn("WARNING: Thin pool %s metadata is now %.2f%% full.",
			 device, dm_percent_to_round_float(state->metadata_percent, 2));
	if (state->metadata_percent > CHECK_MINIMUM) {
		/* Run action when usage raised more than CHECK_STEP since the last time */
		if (state->metadata_percent > state->metadata_percent_check)
			needs_policy = 1;
		state->metadata_percent_check = (state->metadata_percent / CHECK_STEP + 1) * CHECK_STEP;
		if (state->metadata_percent_check == DM_PERCENT_100)
			state->metadata_percent_check--; /* Can't get bigger then 100% */
	} else
		state->metadata_percent_check = CHECK_MINIMUM;

	state->data_percent = dm_make_percent(tps->used_data_blocks, tps->total_data_blocks);
	if ((state->data_percent > WARNING_THRESH) &&
	    (state->data_percent > state->data_percent_check))
		log_warn("WARNING: Thin pool %s data is now %.2f%% full.",
			 device, dm_percent_to_round_float(state->data_percent, 2));
	if (state->data_percent > CHECK_MINIMUM) {
		/* Run action when usage raised more than CHECK_STEP since the last time */
		if (state->data_percent > state->data_percent_check)
			needs_policy = 1;
		state->data_percent_check = (state->data_percent / CHECK_STEP + 1) * CHECK_STEP;
		if (state->data_percent_check == DM_PERCENT_100)
			state->data_percent_check--; /* Can't get bigger then 100% */
	} else
		state->data_percent_check = CHECK_MINIMUM;

	/* Reduce number of _use_policy() calls by power-of-2 factor till frequency of MAX_FAILS is reached.
	 * Avoids too high number of error retries, yet shows some status messages in log regularly.
	 * i.e. PV could have been pvmoved and VG/LV was locked for a while...
	 */
	if (state->fails) {
		if (state->fails++ <= state->max_fails) {
			log_debug("Postponing frequently failing policy (%u <= %u).",
				  state->fails - 1, state->max_fails);
			goto out;
		}
		if (state->max_fails < MAX_FAILS)
			state->max_fails <<= 1;
		state->fails = needs_policy = 1; /* Retry failing command */
	} else
		state->max_fails = 1; /* Reset on success */

	if (needs_policy)
		_use_policy(dmt, state);
out:
	if (tps)
		dm_pool_free(state->mem, tps);

	if (new_dmt)
		dm_task_destroy(new_dmt);
}

/* Handle SIGCHLD for a thread */
static void _sig_child(int signum __attribute__((unused)))
{
	/* empty SIG_IGN */;
}

/* Setup handler for SIGCHLD when executing external command
 * to get quick 'waitpid()' reaction
 * It will interrupt syscall just like SIGALRM and
 * invoke process_event().
 */
static void _init_thread_signals(struct dso_state *state)
{
	struct sigaction act = { .sa_handler = _sig_child };
	sigset_t my_sigset;

	sigemptyset(&my_sigset);

	if (sigaction(SIGCHLD, &act, NULL))
		log_warn("WARNING: Failed to set SIGCHLD action.");
	else if (sigaddset(&my_sigset, SIGCHLD))
		log_warn("WARNING: Failed to add SIGCHLD to set.");
	else if (pthread_sigmask(SIG_UNBLOCK, &my_sigset, &state->old_sigset))
		log_warn("WARNING: Failed to unblock SIGCHLD.");
	else
		state->restore_sigset = 1;
}

static void _restore_thread_signals(struct dso_state *state)
{
	if (state->restore_sigset &&
	    pthread_sigmask(SIG_SETMASK, &state->old_sigset, NULL))
		log_warn("WARNING: Failed to block SIGCHLD.");
}

int register_device(const char *device,
		    const char *uuid __attribute__((unused)),
		    int major __attribute__((unused)),
		    int minor __attribute__((unused)),
		    void **user)
{
	struct dso_state *state;
	char *str;
	char cmd_str[PATH_MAX + 128 + 2]; /* cmd ' ' vg/lv \0 */

	if (!dmeventd_lvm2_init_with_pool("thin_pool_state", state))
		goto_bad;

	if (!dmeventd_lvm2_command(state->mem, cmd_str, sizeof(cmd_str),
				   "_dmeventd_thin_command", device))
		goto_bad;

	if (strncmp(cmd_str, "lvm ", 4) == 0) {
		if (!(state->cmd_str = dm_pool_strdup(state->mem, cmd_str + 4))) {
			log_error("Failed to copy lvm command.");
			goto bad;
		}
	} else if (cmd_str[0] == '/') {
		if (!(state->cmd_str = dm_pool_strdup(state->mem, cmd_str))) {
			log_error("Failed to copy thin command.");
			goto bad;
		}

		/* Find last space before 'vg/lv' */
		if (!(str = strrchr(state->cmd_str, ' ')))
			goto inval;

		if (!(state->argv[0] = dm_pool_strndup(state->mem, state->cmd_str,
						       str - state->cmd_str))) {
			log_error("Failed to copy command.");
			goto bad;
		}

		state->argv[1] = str + 1;  /* 1 argument - vg/lv */
		_init_thread_signals(state);
	} else /* Unuspported command format */
		goto inval;

	state->pid = -1;
	*user = state;

	log_info("Monitoring thin pool %s.", device);

	return 1;
inval:
	log_error("Invalid command for monitoring: %s.", cmd_str);
bad:
	log_error("Failed to monitor thin pool %s.", device);

	if (state)
		dmeventd_lvm2_exit_with_pool(state);

	return 0;
}

int unregister_device(const char *device,
		      const char *uuid __attribute__((unused)),
		      int major __attribute__((unused)),
		      int minor __attribute__((unused)),
		      void **user)
{
	struct dso_state *state = *user;
	int i;

	for (i = 0; !_wait_for_pid(state) && (i < 6); ++i) {
		if (i == 0)
			/* Give it 2 seconds, then try to terminate & kill it */
			log_verbose("Child %d still not finished (%s) waiting.",
				    state->pid, state->cmd_str);
		else if (i == 3) {
			log_warn("WARNING: Terminating child %d.", state->pid);
			kill(state->pid, SIGINT);
			kill(state->pid, SIGTERM);
		} else if (i == 5) {
			log_warn("WARNING: Killing child %d.", state->pid);
			kill(state->pid, SIGKILL);
		}
		sleep(1);
	}

	if (state->pid != -1)
		log_warn("WARNING: Cannot kill child %d!", state->pid);

	_restore_thread_signals(state);

	dmeventd_lvm2_exit_with_pool(state);
	log_info("No longer monitoring thin pool %s.", device);

	return 1;
}
