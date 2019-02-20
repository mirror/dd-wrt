/*
 * Copyright (C) 2005-2015 Red Hat, Inc. All rights reserved.
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

#include "configure.h"
#include "libdevmapper-event.h"
#include "dmeventd.h"
#include "libdm/misc/dm-logging.h"
#include "base/memory/zalloc.h"

#include "lib/misc/intl.h"

#include <fcntl.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <arpa/inet.h>		/* for htonl, ntohl */
#include <pthread.h>
#include <syslog.h>
#include <unistd.h>

static int _debug_level = 0;
static int _use_syslog = 0;
static int _sequence_nr = 0;

struct dm_event_handler {
	char *dso;

	char *dmeventd_path;

	char *dev_name;

	char *uuid;
	int major;
	int minor;
	uint32_t timeout;

	enum dm_event_mask mask;
};

static void _dm_event_handler_clear_dev_info(struct dm_event_handler *dmevh)
{
	free(dmevh->dev_name);
	free(dmevh->uuid);
	dmevh->dev_name = dmevh->uuid = NULL;
	dmevh->major = dmevh->minor = 0;
}

struct dm_event_handler *dm_event_handler_create(void)
{
	struct dm_event_handler *dmevh;

	if (!(dmevh = zalloc(sizeof(*dmevh)))) {
		log_error("Failed to allocate event handler.");
		return NULL;
	}

	return dmevh;
}

void dm_event_handler_destroy(struct dm_event_handler *dmevh)
{
	_dm_event_handler_clear_dev_info(dmevh);
	free(dmevh->dso);
	free(dmevh->dmeventd_path);
	free(dmevh);
}

int dm_event_handler_set_dmeventd_path(struct dm_event_handler *dmevh, const char *dmeventd_path)
{
	if (!dmeventd_path) /* noop */
		return 0;

	free(dmevh->dmeventd_path);

	if (!(dmevh->dmeventd_path = strdup(dmeventd_path)))
		return -ENOMEM;

	return 0;
}

int dm_event_handler_set_dso(struct dm_event_handler *dmevh, const char *path)
{
	if (!path) /* noop */
		return 0;

	free(dmevh->dso);

	if (!(dmevh->dso = strdup(path)))
		return -ENOMEM;

	return 0;
}

int dm_event_handler_set_dev_name(struct dm_event_handler *dmevh, const char *dev_name)
{
	if (!dev_name)
		return 0;

	_dm_event_handler_clear_dev_info(dmevh);

	if (!(dmevh->dev_name = strdup(dev_name)))
		return -ENOMEM;

	return 0;
}

int dm_event_handler_set_uuid(struct dm_event_handler *dmevh, const char *uuid)
{
	if (!uuid)
		return 0;

	_dm_event_handler_clear_dev_info(dmevh);

	if (!(dmevh->uuid = strdup(uuid)))
		return -ENOMEM;

	return 0;
}

void dm_event_handler_set_major(struct dm_event_handler *dmevh, int major)
{
	int minor = dmevh->minor;

	_dm_event_handler_clear_dev_info(dmevh);

	dmevh->major = major;
	dmevh->minor = minor;
}

void dm_event_handler_set_minor(struct dm_event_handler *dmevh, int minor)
{
	int major = dmevh->major;

	_dm_event_handler_clear_dev_info(dmevh);

	dmevh->major = major;
	dmevh->minor = minor;
}

void dm_event_handler_set_event_mask(struct dm_event_handler *dmevh,
				     enum dm_event_mask evmask)
{
	dmevh->mask = evmask;
}

void dm_event_handler_set_timeout(struct dm_event_handler *dmevh, int timeout)
{
	dmevh->timeout = timeout;
}

const char *dm_event_handler_get_dso(const struct dm_event_handler *dmevh)
{
	return dmevh->dso;
}

const char *dm_event_handler_get_dev_name(const struct dm_event_handler *dmevh)
{
	return dmevh->dev_name;
}

const char *dm_event_handler_get_uuid(const struct dm_event_handler *dmevh)
{
	return dmevh->uuid;
}

int dm_event_handler_get_major(const struct dm_event_handler *dmevh)
{
	return dmevh->major;
}

int dm_event_handler_get_minor(const struct dm_event_handler *dmevh)
{
	return dmevh->minor;
}

int dm_event_handler_get_timeout(const struct dm_event_handler *dmevh)
{
	return dmevh->timeout;
}

enum dm_event_mask dm_event_handler_get_event_mask(const struct dm_event_handler *dmevh)
{
	return dmevh->mask;
}

static int _check_message_id(struct dm_event_daemon_message *msg)
{
	int pid, seq_nr;

	if ((sscanf(msg->data, "%d:%d", &pid, &seq_nr) != 2) ||
	    (pid != getpid()) || (seq_nr != _sequence_nr)) {
		log_error("Ignoring out-of-sequence reply from dmeventd. "
			  "Expected %d:%d but received %s.", getpid(),
			  _sequence_nr, msg->data);
		return 0;
	}

	return 1;
}

/*
 * daemon_read
 * @fifos
 * @msg
 *
 * Read message from daemon.
 *
 * Returns: 0 on failure, 1 on success
 */
static int _daemon_read(struct dm_event_fifos *fifos,
			struct dm_event_daemon_message *msg)
{
	unsigned bytes = 0;
	int ret, i;
	fd_set fds;
	size_t size = 2 * sizeof(uint32_t);	/* status + size */
	uint32_t *header = alloca(size);
	char *buf = (char *)header;

	while (bytes < size) {
		for (i = 0, ret = 0; (i < 20) && (ret < 1); i++) {
			/* Watch daemon read FIFO for input. */
			struct timeval tval = { .tv_sec = 1 };
			FD_ZERO(&fds);
			FD_SET(fifos->server, &fds);
			ret = select(fifos->server + 1, &fds, NULL, NULL, &tval);
			if (ret < 0 && errno != EINTR) {
				log_error("Unable to read from event server.");
				return 0;
			}
			if ((ret == 0) && (i > 4) && !bytes) {
				log_error("No input from event server.");
				return 0;
			}
		}
		if (ret < 1) {
			log_error("Unable to read from event server.");
			return 0;
		}

		ret = read(fifos->server, buf + bytes, size);
		if (ret < 0) {
			if ((errno == EINTR) || (errno == EAGAIN))
				continue;

			log_error("Unable to read from event server.");
			return 0;
		}

		bytes += ret;
		if (header && (bytes == 2 * sizeof(uint32_t))) {
			msg->cmd = ntohl(header[0]);
			msg->size = ntohl(header[1]);
			buf = msg->data = malloc(msg->size);
			size = msg->size;
			bytes = 0;
			header = 0;
		}
	}

	if (bytes != size) {
		free(msg->data);
		msg->data = NULL;
	}
	return bytes == size;
}

/* Write message to daemon. */
static int _daemon_write(struct dm_event_fifos *fifos,
			 struct dm_event_daemon_message *msg)
{
	int ret;
	fd_set fds;
	size_t bytes = 0;
	size_t size = 2 * sizeof(uint32_t) + msg->size;
	uint32_t *header = alloca(size);
	char *buf = (char *)header;
	char drainbuf[128];

	header[0] = htonl(msg->cmd);
	header[1] = htonl(msg->size);
	memcpy(buf + 2 * sizeof(uint32_t), msg->data, msg->size);

	/* drain the answer fifo */
	while (1) {
		struct timeval tval = { .tv_usec = 100 };
		FD_ZERO(&fds);
		FD_SET(fifos->server, &fds);
		ret = select(fifos->server + 1, &fds, NULL, NULL, &tval);
		if (ret < 0) {
			if (errno == EINTR)
				continue;
			log_error("Unable to talk to event daemon.");
			return 0;
		}
		if (ret == 0)
			break;
		ret = read(fifos->server, drainbuf, sizeof(drainbuf));
		if (ret < 0) {
			if ((errno == EINTR) || (errno == EAGAIN))
				continue;
			log_error("Unable to talk to event daemon.");
			return 0;
		}
	}

	while (bytes < size) {
		do {
			/* Watch daemon write FIFO to be ready for output. */
			FD_ZERO(&fds);
			FD_SET(fifos->client, &fds);
			ret = select(fifos->client + 1, NULL, &fds, NULL, NULL);
			if ((ret < 0) && (errno != EINTR)) {
				log_error("Unable to talk to event daemon.");
				return 0;
			}
		} while (ret < 1);

		ret = write(fifos->client, buf + bytes, size - bytes);
		if (ret < 0) {
			if ((errno == EINTR) || (errno == EAGAIN))
				continue;

			log_error("Unable to talk to event daemon.");
			return 0;
		}

		bytes += ret;
	}

	return bytes == size;
}

int daemon_talk(struct dm_event_fifos *fifos,
		struct dm_event_daemon_message *msg, int cmd,
		const char *dso_name, const char *dev_name,
		enum dm_event_mask evmask, uint32_t timeout)
{
	int msg_size;
	memset(msg, 0, sizeof(*msg));

	/*
	 * Set command and pack the arguments
	 * into ASCII message string.
	 */
	if ((msg_size =
	     ((cmd == DM_EVENT_CMD_HELLO) ?
	      dm_asprintf(&(msg->data), "%d:%d HELLO", getpid(), _sequence_nr) :
	      dm_asprintf(&(msg->data), "%d:%d %s %s %u %" PRIu32,
			  getpid(), _sequence_nr,
			  dso_name ? : "-", dev_name ? : "-", evmask, timeout)))
	    < 0) {
		log_error("_daemon_talk: message allocation failed.");
		return -ENOMEM;
	}
	msg->cmd = cmd;
	msg->size = msg_size;

	/*
	 * Write command and message to and
	 * read status return code from daemon.
	 */
	if (!_daemon_write(fifos, msg)) {
		stack;
		free(msg->data);
		msg->data = NULL;
		return -EIO;
	}

	do {
		free(msg->data);
		msg->data = NULL;

		if (!_daemon_read(fifos, msg)) {
			stack;
			return -EIO;
		}
	} while (!_check_message_id(msg));

	_sequence_nr++;

	return (int32_t) msg->cmd;
}

/*
 * start_daemon
 *
 * This function forks off a process (dmeventd) that will handle
 * the events.  I am currently test opening one of the fifos to
 * ensure that the daemon is running and listening...  I thought
 * this would be less expensive than fork/exec'ing every time.
 * Perhaps there is an even quicker/better way (no, checking the
 * lock file is _not_ a better way).
 *
 * Returns: 1 on success, 0 otherwise
 */
static int _start_daemon(char *dmeventd_path, struct dm_event_fifos *fifos)
{
	int pid, ret = 0;
	int status;
	struct stat statbuf;
	char default_dmeventd_path[] = DMEVENTD_PATH;
	char *args[] = { dmeventd_path ? : default_dmeventd_path, NULL };

	/*
	 * FIXME Explicitly verify the code's requirement that client_path is secure:
	 * - All parent directories owned by root without group/other write access unless sticky.
	 */

	/* If client fifo path exists, only use it if it is root-owned fifo mode 0600 */
	if ((lstat(fifos->client_path, &statbuf) < 0)) {
		if (errno == ENOENT)
			/* Jump ahead if fifo does not already exist. */
			goto start_server;
		else {
			log_sys_error("stat", fifos->client_path);
			return 0;
		}
	} else if (!S_ISFIFO(statbuf.st_mode)) {
		log_error("%s must be a fifo.", fifos->client_path);
		return 0;
	} else if (statbuf.st_uid) {
		log_error("%s must be owned by uid 0.", fifos->client_path);
		return 0;
	} else if (statbuf.st_mode & (S_IEXEC | S_IRWXG | S_IRWXO)) {
		log_error("%s must have mode 0600.", fifos->client_path);
		return 0;
	}

	/* Anyone listening?  If not, errno will be ENXIO */
	fifos->client = open(fifos->client_path, O_WRONLY | O_NONBLOCK);
	if (fifos->client >= 0) {
		/* Should never happen if all the above checks passed. */
		if ((fstat(fifos->client, &statbuf) < 0) ||
		    !S_ISFIFO(statbuf.st_mode) || statbuf.st_uid ||
		    (statbuf.st_mode & (S_IEXEC | S_IRWXG | S_IRWXO))) {
			log_error("%s is no longer a secure root-owned fifo with mode 0600.", fifos->client_path);
			if (close(fifos->client))
				log_sys_debug("close", fifos->client_path);
			return 0;
		}

		/* server is running and listening */
		if (close(fifos->client))
			log_sys_debug("close", fifos->client_path);
		return 1;
	}
	if (errno != ENXIO && errno != ENOENT)  {
		/* problem */
		log_sys_error("open", fifos->client_path);
		return 0;
	}

start_server:
	/* server is not running */

	if ((args[0][0] == '/') && stat(args[0], &statbuf)) {
		log_sys_error("stat", args[0]);
		return 0;
	}

	pid = fork();

	if (pid < 0)
		log_sys_error("fork", "");

	else if (!pid) {
		execvp(args[0], args);
		log_error("Unable to exec dmeventd: %s.", strerror(errno));
		_exit(EXIT_FAILURE);
	} else {
		if (waitpid(pid, &status, 0) < 0)
			log_error("Unable to start dmeventd: %s.",
				  strerror(errno));
		else if (WEXITSTATUS(status))
			log_error("Unable to start dmeventd.");
		else
			ret = 1;
	}

	return ret;
}

int init_fifos(struct dm_event_fifos *fifos)
{
	/* FIXME? Is fifo the most suitable method? Why not share
	   comms/daemon code with something else e.g. multipath? */

	/* Open the fifo used to read from the daemon. */
	if ((fifos->server = open(fifos->server_path, O_RDWR)) < 0) {
		log_sys_error("open", fifos->server_path);
		return 0;
	}

	/* Lock out anyone else trying to do communication with the daemon. */
	if (flock(fifos->server, LOCK_EX) < 0) {
		log_sys_error("flock", fifos->server_path);
		goto bad;
	}

/*	if ((fifos->client = open(fifos->client_path, O_WRONLY | O_NONBLOCK)) < 0) {*/
	if ((fifos->client = open(fifos->client_path, O_RDWR | O_NONBLOCK)) < 0) {
		log_sys_error("open", fifos->client_path);
		goto bad;
	}

	return 1;
bad:
	if (close(fifos->server))
		log_sys_debug("close", fifos->server_path);
	fifos->server = -1;

	return 0;
}

/* Initialize client. */
static int _init_client(char *dmeventd_path, struct dm_event_fifos *fifos)
{
	if (!_start_daemon(dmeventd_path, fifos))
		return_0;

	return init_fifos(fifos);
}

void fini_fifos(struct dm_event_fifos *fifos)
{
	if (fifos->client >= 0 && close(fifos->client))
		log_sys_debug("close", fifos->client_path);

	if (fifos->server >= 0) {
		if (flock(fifos->server, LOCK_UN))
			log_sys_debug("flock unlock", fifos->server_path);

		if (close(fifos->server))
			log_sys_debug("close", fifos->server_path);
	}
}

/* Get uuid of a device */
static struct dm_task *_get_device_info(const struct dm_event_handler *dmevh)
{
	struct dm_task *dmt;
	struct dm_info info;

	if (!(dmt = dm_task_create(DM_DEVICE_INFO))) {
		log_error("_get_device_info: dm_task creation for info failed.");
		return NULL;
	}

	if (dmevh->uuid) {
		if (!dm_task_set_uuid(dmt, dmevh->uuid))
			goto_bad;
	} else if (dmevh->dev_name) {
		if (!dm_task_set_name(dmt, dmevh->dev_name))
			goto_bad;
	} else if (dmevh->major && dmevh->minor) {
		if (!dm_task_set_major(dmt, dmevh->major) ||
		    !dm_task_set_minor(dmt, dmevh->minor))
			goto_bad;
	}

	/* FIXME Add name or uuid or devno to messages */
	if (!dm_task_run(dmt)) {
		log_error("_get_device_info: dm_task_run() failed.");
		goto bad;
	}

	if (!dm_task_get_info(dmt, &info)) {
		log_error("_get_device_info: failed to get info for device.");
		goto bad;
	}

	if (!info.exists) {
		log_error("_get_device_info: %s%s%s%.0d%s%.0d%s%s: device not found.",
			  dmevh->uuid ? : "",
			  (!dmevh->uuid && dmevh->dev_name) ? dmevh->dev_name : "",
			  (!dmevh->uuid && !dmevh->dev_name && dmevh->major > 0) ? "(" : "",
			  (!dmevh->uuid && !dmevh->dev_name && dmevh->major > 0) ? dmevh->major : 0,
			  (!dmevh->uuid && !dmevh->dev_name && dmevh->major > 0) ? ":" : "",
			  (!dmevh->uuid && !dmevh->dev_name && dmevh->minor > 0) ? dmevh->minor : 0,
			  (!dmevh->uuid && !dmevh->dev_name && dmevh->major > 0) && dmevh->minor == 0 ? "0" : "",
			  (!dmevh->uuid && !dmevh->dev_name && dmevh->major > 0) ? ") " : "");
		goto bad;
	}

	return dmt;

      bad:
	dm_task_destroy(dmt);
	return NULL;
}

/* Handle the event (de)registration call and return negative error codes. */
static int _do_event(int cmd, char *dmeventd_path, struct dm_event_daemon_message *msg,
		     const char *dso_name, const char *dev_name,
		     enum dm_event_mask evmask, uint32_t timeout)
{
	int ret;
	struct dm_event_fifos fifos = {
		.server = -1,
		.client = -1,
		/* FIXME Make these either configurable or depend directly on dmeventd_path */
		.client_path = DM_EVENT_FIFO_CLIENT,
		.server_path = DM_EVENT_FIFO_SERVER
	};

	if (!_init_client(dmeventd_path, &fifos)) {
		ret = -ESRCH;
		goto_out;
	}

	ret = daemon_talk(&fifos, msg, DM_EVENT_CMD_HELLO, NULL, NULL, 0, 0);

	free(msg->data);
	msg->data = 0;

	if (!ret)
		ret = daemon_talk(&fifos, msg, cmd, dso_name, dev_name, evmask, timeout);
out:
	/* what is the opposite of init? */
	fini_fifos(&fifos);

	return ret;
}

/* External library interface. */
int dm_event_register_handler(const struct dm_event_handler *dmevh)
{
	int ret = 1, err;
	const char *uuid;
	struct dm_task *dmt;
	struct dm_event_daemon_message msg = { 0 };

	if (!(dmt = _get_device_info(dmevh)))
		return_0;

	uuid = dm_task_get_uuid(dmt);

	if (!strstr(dmevh->dso, "libdevmapper-event-lvm2thin.so") &&
	    !strstr(dmevh->dso, "libdevmapper-event-lvm2vdo.so") &&
	    !strstr(dmevh->dso, "libdevmapper-event-lvm2snapshot.so") &&
	    !strstr(dmevh->dso, "libdevmapper-event-lvm2mirror.so") &&
	    !strstr(dmevh->dso, "libdevmapper-event-lvm2raid.so"))
		log_warn("WARNING: %s: dmeventd plugins are deprecated.", dmevh->dso);


	if ((err = _do_event(DM_EVENT_CMD_REGISTER_FOR_EVENT, dmevh->dmeventd_path, &msg,
			     dmevh->dso, uuid, dmevh->mask, dmevh->timeout)) < 0) {
		log_error("%s: event registration failed: %s.",
			  dm_task_get_name(dmt),
			  msg.data ? msg.data : strerror(-err));
		ret = 0;
	}

	free(msg.data);

	dm_task_destroy(dmt);

	return ret;
}

int dm_event_unregister_handler(const struct dm_event_handler *dmevh)
{
	int ret = 1, err;
	const char *uuid;
	struct dm_task *dmt;
	struct dm_event_daemon_message msg = { 0 };

	if (!(dmt = _get_device_info(dmevh)))
		return_0;

	uuid = dm_task_get_uuid(dmt);

	if ((err = _do_event(DM_EVENT_CMD_UNREGISTER_FOR_EVENT, dmevh->dmeventd_path, &msg,
			    dmevh->dso, uuid, dmevh->mask, dmevh->timeout)) < 0) {
		log_error("%s: event deregistration failed: %s.",
			  dm_task_get_name(dmt),
			  msg.data ? msg.data : strerror(-err));
		ret = 0;
	}

	free(msg.data);

	dm_task_destroy(dmt);

	return ret;
}

/* Fetch a string off src and duplicate it into *dest. */
/* FIXME: move to separate module to share with the daemon. */
static char *_fetch_string(char **src, const int delimiter)
{
	char *p, *ret;

	if ((p = strchr(*src, delimiter)))
		*p = 0;

	if ((ret = strdup(*src)))
		*src += strlen(ret) + 1;

	if (p)
		*p = delimiter;

	return ret;
}

/* Parse a device message from the daemon. */
static int _parse_message(struct dm_event_daemon_message *msg, char **dso_name,
			 char **uuid, enum dm_event_mask *evmask)
{
	char *id;
	char *p = msg->data;

	if ((id = _fetch_string(&p, ' ')) &&
	    (*dso_name = _fetch_string(&p, ' ')) &&
	    (*uuid = _fetch_string(&p, ' '))) {
		*evmask = atoi(p);
		free(id);
		return 0;
	}

	free(id);
	return -ENOMEM;
}

/*
 * Returns 0 if handler found; error (-ENOMEM, -ENOENT) otherwise.
 */
int dm_event_get_registered_device(struct dm_event_handler *dmevh, int next)
{
	int ret = 0;
	const char *uuid = NULL;
	char *reply_dso = NULL, *reply_uuid = NULL;
	enum dm_event_mask reply_mask = 0;
	struct dm_task *dmt = NULL;
	struct dm_event_daemon_message msg = { 0 };
	struct dm_info info;

	if (!(dmt = _get_device_info(dmevh))) {
		log_debug("Device does not exists (uuid=%s, name=%s, %d:%d).",
			  dmevh->uuid, dmevh->dev_name,
			  dmevh->major, dmevh->minor);
		ret = -ENODEV;
		goto fail;
	}

	uuid = dm_task_get_uuid(dmt);

	/* FIXME Distinguish errors connecting to daemon */
	if ((ret = _do_event(next ? DM_EVENT_CMD_GET_NEXT_REGISTERED_DEVICE :
			    DM_EVENT_CMD_GET_REGISTERED_DEVICE, dmevh->dmeventd_path,
			    &msg, dmevh->dso, uuid, dmevh->mask, 0))) {
		log_debug("%s: device not registered.", dm_task_get_name(dmt));
		goto fail;
	}

	/* FIXME this will probably horribly break if we get
	   ill-formatted reply */
	ret = _parse_message(&msg, &reply_dso, &reply_uuid, &reply_mask);

	dm_task_destroy(dmt);
	dmt = NULL;

	free(msg.data);
	msg.data = NULL;

	_dm_event_handler_clear_dev_info(dmevh);
	if (!reply_uuid) {
		ret = -ENXIO; /* dmeventd probably gave us bogus uuid back */
		goto fail;
	}

	if (!(dmevh->uuid = strdup(reply_uuid))) {
		ret = -ENOMEM;
		goto fail;
	}

	if (!(dmt = _get_device_info(dmevh))) {
		ret = -ENXIO; /* dmeventd probably gave us bogus uuid back */
		goto fail;
	}

	dm_event_handler_set_dso(dmevh, reply_dso);
	dm_event_handler_set_event_mask(dmevh, reply_mask);

	free(reply_dso);
	reply_dso = NULL;

	free(reply_uuid);
	reply_uuid = NULL;

	if (!(dmevh->dev_name = strdup(dm_task_get_name(dmt)))) {
		ret = -ENOMEM;
		goto fail;
	}

	if (!dm_task_get_info(dmt, &info)) {
		ret = -1;
		goto fail;
	}

	dmevh->major = info.major;
	dmevh->minor = info.minor;

	dm_task_destroy(dmt);

	return ret;

 fail:
	free(msg.data);
	free(reply_dso);
	free(reply_uuid);
	_dm_event_handler_clear_dev_info(dmevh);
	if (dmt)
		dm_task_destroy(dmt);
	return ret;
}

/*
 * You can (and have to) call this at the stage of the protocol where
 *     daemon_talk(fifos, &msg, DM_EVENT_CMD_HELLO, NULL, NULL, 0, 0)
 *
 * would be normally sent. This call will parse the version reply from
 * dmeventd, in addition to above call. It is not safe to call this at any
 * other place in the protocol.
 *
 * This is an internal function, not exposed in the public API.
 */

int dm_event_get_version(struct dm_event_fifos *fifos, int *version) {
	char *p;
	struct dm_event_daemon_message msg = { 0 };

	if (daemon_talk(fifos, &msg, DM_EVENT_CMD_HELLO, NULL, NULL, 0, 0))
		return 0;
	p = msg.data;
	*version = 0;

	if (!p || !(p = strchr(p, ' '))) /* Message ID */
		return 0;
	if (!(p = strchr(p + 1, ' '))) /* HELLO */
		return 0;
	if ((p = strchr(p + 1, ' '))) /* HELLO, once more */
		*version = atoi(p);

	return 1;
}

void dm_event_log_set(int debug_log_level, int use_syslog)
{
	_debug_level = debug_log_level;
	_use_syslog = use_syslog;
}

void dm_event_log(const char *subsys, int level, const char *file,
		  int line, int dm_errno_or_class,
		  const char *format, va_list ap)
{
	static int _abort_on_internal_errors = -1;
	static pthread_mutex_t _log_mutex = PTHREAD_MUTEX_INITIALIZER;
	static time_t start = 0;
	const char *indent = "";
	FILE *stream = log_stderr(level) ? stderr : stdout;
	int prio;
	time_t now;
	int log_with_debug = 0;

	if (subsys[0] == '#') {
		/* Subsystems starting with '#' are logged
		 * only when debugging is enabled. */
		log_with_debug++;
		subsys++;
	}

	switch (log_level(level)) {
	case _LOG_DEBUG:
		/* Never shown without -ddd */
		if (_debug_level < 3)
			return;
		prio = LOG_DEBUG;
		indent = "      ";
		break;
	case _LOG_INFO:
		if (log_with_debug && _debug_level < 2)
			return;
		prio = LOG_INFO;
		indent = "    ";
		break;
	case _LOG_NOTICE:
		if (log_with_debug && _debug_level < 1)
			return;
		prio = LOG_NOTICE;
		indent = "  ";
		break;
	case _LOG_WARN:
		prio = LOG_WARNING;
		break;
	case _LOG_ERR:
		prio = LOG_ERR;
		stream = stderr;
		break;
	default:
		prio = LOG_CRIT;
	}

	/* Serialize to keep lines readable */
	pthread_mutex_lock(&_log_mutex);

	if (_use_syslog) {
		vsyslog(prio, format, ap);
	} else {
		now = time(NULL);
		if (!start)
			start = now;
		now -= start;
		if (_debug_level)
			fprintf(stream, "[%2d:%02d] %8x:%-6s%s",
				(int)now / 60, (int)now % 60,
				// TODO: Maybe use shorter ID
				// ((int)(pthread_self()) >> 6) & 0xffff,
				(int)pthread_self(), subsys,
				(_debug_level > 3) ? "" : indent);
		if (_debug_level > 3)
			fprintf(stream, "%28s:%4d %s", file, line, indent);
		vfprintf(stream, _(format), ap);
		fputc('\n', stream);
		fflush(stream);
	}

	pthread_mutex_unlock(&_log_mutex);

	if (_abort_on_internal_errors < 0)
		/* Set when env DM_ABORT_ON_INTERNAL_ERRORS is not "0" */
		_abort_on_internal_errors =
			strcmp(getenv("DM_ABORT_ON_INTERNAL_ERRORS") ? : "0", "0");

	if (_abort_on_internal_errors &&
	    !strncmp(format, INTERNAL_ERROR, sizeof(INTERNAL_ERROR) - 1))
		abort();
}

#if 0				/* left out for now */

static char *_skip_string(char *src, const int delimiter)
{
	src = srtchr(src, delimiter);
	if (src && *(src + 1))
		return src + 1;
	return NULL;
}

int dm_event_set_timeout(const char *device_path, uint32_t timeout)
{
	struct dm_event_daemon_message msg = { 0 };

	if (!device_exists(device_path))
		return -ENODEV;

	return _do_event(DM_EVENT_CMD_SET_TIMEOUT, &msg,
			 NULL, device_path, 0, timeout);
}

int dm_event_get_timeout(const char *device_path, uint32_t *timeout)
{
	int ret;
	struct dm_event_daemon_message msg = { 0 };

	if (!device_exists(device_path))
		return -ENODEV;

	if (!(ret = _do_event(DM_EVENT_CMD_GET_TIMEOUT, &msg, NULL, device_path,
			     0, 0))) {
		char *p = _skip_string(msg.data, ' ');
		if (!p) {
			log_error("Malformed reply from dmeventd '%s'.",
				  msg.data);
			free(msg.data);
			return -EIO;
		}
		*timeout = atoi(p);
	}
	free(msg.data);

	return ret;
}
#endif
