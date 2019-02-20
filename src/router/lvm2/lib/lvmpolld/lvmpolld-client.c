/*
 * Copyright (C) 2015 Red Hat, Inc.
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

#include "libdaemon/client/daemon-io.h"
#include "lib/lvmpolld/lvmpolld-client.h"
#include "daemons/lvmpolld/lvmpolld-protocol.h"
#include "lib/metadata/metadata-exported.h"
#include "lib/lvmpolld/polldaemon.h"
#include "lib/commands/toolcontext.h"
#include "tools/lvm2cmd.h"

struct progress_info {
	unsigned error:1;
	unsigned finished:1;
	int cmd_signal;
	int cmd_retcode;
};

static int _lvmpolld_use;
static int _lvmpolld_connected;
static const char* _lvmpolld_socket;

static daemon_handle _lvmpolld = { .error = 0 };

static daemon_handle _lvmpolld_open(const char *socket)
{
	daemon_info lvmpolld_info = {
		.path = "lvmpolld",
		.socket = socket ?: LVMPOLLD_SOCKET,
		.protocol = LVMPOLLD_PROTOCOL,
		.protocol_version = LVMPOLLD_PROTOCOL_VERSION
	};

	return daemon_open(lvmpolld_info);
}

void lvmpolld_set_active(int active)
{
	_lvmpolld_use = active;
}

void lvmpolld_set_socket(const char *socket)
{
	_lvmpolld_socket = socket;
}

static void _lvmpolld_connect_or_warn(void)
{
	if (!_lvmpolld_connected && !_lvmpolld.error) {
		_lvmpolld = _lvmpolld_open(_lvmpolld_socket);
		if ( _lvmpolld.socket_fd >= 0 && !_lvmpolld.error) {
			log_debug_lvmpolld("Sucessfully connected to lvmpolld on fd %d.", _lvmpolld.socket_fd);
			_lvmpolld_connected = 1;
		} else {
			log_warn("WARNING: Failed to connect to lvmpolld. Proceeding with polling without using lvmpolld.");
			log_warn("WARNING: Check global/use_lvmpolld in lvm.conf or the lvmpolld daemon state.");
		}
	}
}

int lvmpolld_use(void)
{
	if (!_lvmpolld_use || !_lvmpolld_socket)
		return 0;

	_lvmpolld_connect_or_warn();

	return _lvmpolld_connected;
}

void lvmpolld_disconnect(void)
{
	if (_lvmpolld_connected) {
		daemon_close(_lvmpolld);
		_lvmpolld_connected = 0;
	}
}

static void _explain_error_codes(int retcode)
{
	switch (retcode) {
	/* LVM2 return codes */
	case LVM2_NO_SUCH_COMMAND:
		log_error("LVM command run by lvmpolld responded with: 'No such command.'");
		break;
	case LVM2_INVALID_PARAMETERS:
		log_error("LVM command run by lvmpolld failed due to invalid parameters.");
		break;
	case LVM2_PROCESSING_FAILED:
		log_error("LVM command executed by lvmpolld failed.");
		break;

	/* lvmpolld specific return codes */
	case LVMPD_RET_DUP_FAILED:
		log_error("lvmpolld failed to duplicate file descriptors.");
		/* fall through */
	case LVMPD_RET_EXC_FAILED:
		log_error("lvmpolld failed to exec() lvm binary.");
		break;
	default:
		log_error("lvmpolld responded with unexpected return code.");
	}

	log_print_unless_silent("For more information see lvmpolld messages in syslog or lvmpolld log file.");
}

static void _process_error_response(daemon_reply rep)
{
	if (!strcmp(daemon_reply_str(rep, "response", ""), LVMPD_RESP_FAILED))
		log_error("lvmpolld failed to process a request. The reason was: %s.",
			  daemon_reply_str(rep, "reason", "<empty>"));
	else if (!strcmp(daemon_reply_str(rep, "response", ""), LVMPD_RESP_EINVAL))
		log_error("lvmpolld couldn't handle a request. "
			  "It might be due to daemon internal state. The reason was: %s.",
			  daemon_reply_str(rep, "reason", "<empty>"));
	else
		log_error("Unexpected response %s. The reason: %s.",
			  daemon_reply_str(rep, "response", "<empty>"),
			  daemon_reply_str(rep, "reason", "<empty>"));

	log_print_unless_silent("For more information see lvmpolld messages in syslog or lvmpolld log file.");
}

static struct progress_info _request_progress_info(const char *uuid, unsigned abort_polling)
{
	daemon_reply rep;
	const char *e = getenv("LVM_SYSTEM_DIR");
	struct progress_info ret = { .error = 1, .finished = 1 };
	daemon_request req = daemon_request_make(LVMPD_REQ_PROGRESS);

	if (!daemon_request_extend(req, LVMPD_PARM_LVID " = %s", uuid, NULL)) {
		log_error("Failed to create " LVMPD_REQ_PROGRESS " request.");
		goto out_req;
	}

	if (abort_polling &&
	    !daemon_request_extend(req, LVMPD_PARM_ABORT " = " FMTd64, (int64_t) abort_polling, NULL)) {
		log_error("Failed to create " LVMPD_REQ_PROGRESS " request.");
		goto out_req;
	}

	if (e &&
	    !(daemon_request_extend(req, LVMPD_PARM_SYSDIR " = %s",
				    e, NULL))) {
		log_error("Failed to create " LVMPD_REQ_PROGRESS " request.");
		goto out_req;
	}

	rep = daemon_send(_lvmpolld, req);
	if (rep.error) {
		log_error("Failed to process request with error %s (errno: %d).",
			  strerror(rep.error), rep.error);
		goto out_rep;
	}

	if (!strcmp(daemon_reply_str(rep, "response", ""), LVMPD_RESP_IN_PROGRESS)) {
		ret.finished = 0;
		ret.error = 0;
	} else if (!strcmp(daemon_reply_str(rep, "response", ""), LVMPD_RESP_FINISHED)) {
		if (!strcmp(daemon_reply_str(rep, "reason", ""), LVMPD_REAS_SIGNAL))
			ret.cmd_signal = daemon_reply_int(rep, LVMPD_PARM_VALUE, 0);
		else
			ret.cmd_retcode = daemon_reply_int(rep, LVMPD_PARM_VALUE, -1);
		ret.error = 0;
	} else if (!strcmp(daemon_reply_str(rep, "response", ""), LVMPD_RESP_NOT_FOUND)) {
		log_verbose("No polling operation in progress regarding LV %s.", uuid);
		ret.error = 0;
	} else {
		_process_error_response(rep);
		stack;
	}

out_rep:
	daemon_reply_destroy(rep);
out_req:
	daemon_request_destroy(req);

	return ret;
}

/*
 * interval in seconds long
 * enough for more than a year
 * of waiting
 */
#define INTERV_SIZE 10

static int _process_poll_init(const struct cmd_context *cmd, const char *poll_type,
			      const struct poll_operation_id *id, const struct daemon_parms *parms)
{
	char *str;
	daemon_reply rep;
	daemon_request req;
	const char *e = getenv("LVM_SYSTEM_DIR");
	int r = 0; 

	str = malloc(INTERV_SIZE * sizeof(char));
	if (!str)
		return r;

	if (snprintf(str, INTERV_SIZE, "%u", parms->interval) >= INTERV_SIZE) {
		log_warn("Interval string conversion got truncated.");
		str[INTERV_SIZE - 1] = '\0';
	}

	req = daemon_request_make(poll_type);
	if (!daemon_request_extend(req, LVMPD_PARM_LVID " = %s", id->uuid,
					LVMPD_PARM_VGNAME " = %s", id->vg_name,
					LVMPD_PARM_LVNAME " = %s", id->lv_name,
					LVMPD_PARM_INTERVAL " = %s", str,
					"cmdline = %s", cmd->cmd_line, /* FIXME: debug param only */
					NULL)) {
		log_error("Failed to create %s request.", poll_type);
		goto out_req;
	}

	if (parms->aborting &&
	    !(daemon_request_extend(req, LVMPD_PARM_ABORT " = " FMTd64, (int64_t) (parms->aborting), NULL))) {
		log_error("Failed to create %s request." , poll_type);
		goto out_req;
	}

	if (cmd->handles_missing_pvs &&
	    !(daemon_request_extend(req, LVMPD_PARM_HANDLE_MISSING_PVS " = " FMTd64,
				    (int64_t) (cmd->handles_missing_pvs), NULL))) {
		log_error("Failed to create %s request." , poll_type);
		goto out_req;
	}

	if (e &&
	    !(daemon_request_extend(req, LVMPD_PARM_SYSDIR " = %s",
				    e, NULL))) {
		log_error("Failed to create %s request." , poll_type);
		goto out_req;
	}

	rep = daemon_send(_lvmpolld, req);

	if (rep.error) {
		log_error("Failed to process request with error %s (errno: %d).",
			  strerror(rep.error), rep.error);
		goto out_rep;
	}

	if (!strcmp(daemon_reply_str(rep, "response", ""), LVMPD_RESP_OK))
		r = 1;
	else {
		_process_error_response(rep);
		stack;
	}

out_rep:
	daemon_reply_destroy(rep);
out_req:
	daemon_request_destroy(req);
	free(str);

	return r;
}

int lvmpolld_poll_init(const struct cmd_context *cmd, const struct poll_operation_id *id,
		       const struct daemon_parms *parms)
{
	int r = 0;

	if (!id->uuid) {
		log_error(INTERNAL_ERROR "Use of lvmpolld requires uuid set");
		return 0;
	}

	if (!id->vg_name) {
		log_error(INTERNAL_ERROR "Use of lvmpolld requires vgname set");
		return 0;
	}

	if (!id->lv_name) {
		log_error(INTERNAL_ERROR "Use of lvmpolld requires lvname set");
		return 0;
	}

	if (parms->lv_type & PVMOVE) {
		log_debug_lvmpolld("Asking lvmpolld for pvmove%s on %s/%s.",
				   parms->aborting ? " abort" : "", id->vg_name, id->lv_name);
		r =  _process_poll_init(cmd, LVMPD_REQ_PVMOVE, id, parms);
	} else if (parms->lv_type & CONVERTING) {
		log_debug_lvmpolld("Asking lvmpolld for mirror conversion on %s/%s.",
				   id->vg_name, id->lv_name);
		r =  _process_poll_init(cmd, LVMPD_REQ_CONVERT, id, parms);
	} else if (parms->lv_type & MERGING) {
		if (parms->lv_type & SNAPSHOT) {
			log_debug_lvmpolld("Asking lvmpolld for snapshot merge on %s/%s.",
					   id->vg_name, id->lv_name);
			r =  _process_poll_init(cmd, LVMPD_REQ_MERGE, id, parms);
		}
		else if (parms->lv_type & THIN_VOLUME) {
			log_debug_lvmpolld("Asking lvmpolld for thin snapshot merge on %s/%s.",
					   id->vg_name, id->lv_name);
			r = _process_poll_init(cmd, LVMPD_REQ_MERGE_THIN, id, parms);
		}
		else {
			log_error(INTERNAL_ERROR "Unsupported poll operation.");
		}
	} else
		log_error(INTERNAL_ERROR "Unsupported poll operation");

	return r;
}

int lvmpolld_request_info(const struct poll_operation_id *id, const struct daemon_parms *parms, unsigned *finished)
{
	struct progress_info info;
	int ret = 0;

	*finished = 1;

	if (!id->uuid) {
		log_error(INTERNAL_ERROR "use of lvmpolld requires uuid being set");
		return 0;
	}

	log_debug_lvmpolld("Asking lvmpolld for progress status of an operation on %s/%s.",
			   id->vg_name, id->lv_name);
	info = _request_progress_info(id->uuid, parms->aborting);
	*finished = info.finished;

	if (info.error)
		return_0;

	if (info.finished) {
		if (info.cmd_signal)
			log_error("Command executed by lvmpolld got terminated by signal (%d).",
				  info.cmd_signal);
		else if (info.cmd_retcode)
			_explain_error_codes(info.cmd_retcode);
		else {
			log_verbose("Polling finished successfully.");
			ret = 1;
		}
	} else
		ret = 1;

	return ret;
}
