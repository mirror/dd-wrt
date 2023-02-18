/*
 * nfsdcld.c -- NFSv4 client name tracking daemon
 *
 * Copyright (C) 2011  Red Hat, Jeff Layton <jlayton@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <errno.h>
#include <event2/event.h>
#include <stdbool.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/inotify.h>
#ifdef HAVE_SYS_CAPABILITY_H
#include <sys/prctl.h>
#include <sys/capability.h>
#endif

#include "xlog.h"
#include "nfslib.h"
#include "cld.h"
#include "cld-internal.h"
#include "sqlite.h"
#include "version.h"
#include "conffile.h"
#include "legacy.h"

#ifndef DEFAULT_PIPEFS_DIR
#define DEFAULT_PIPEFS_DIR NFS_STATEDIR "/rpc_pipefs"
#endif

#define DEFAULT_CLD_PATH	"/nfsd/cld"

#ifndef CLD_DEFAULT_STORAGEDIR
#define CLD_DEFAULT_STORAGEDIR NFS_STATEDIR "/nfsdcld"
#endif

#define NFSD_END_GRACE_FILE "/proc/fs/nfsd/v4_end_grace"

/* private data structures */

/* global variables */
static char pipefs_dir[PATH_MAX] = DEFAULT_PIPEFS_DIR;
static char pipepath[PATH_MAX];
static int 		inotify_fd = -1;
static struct event	 *pipedir_event;
static struct event_base *evbase;
static bool old_kernel = false;
static bool signal_received = false;

uint64_t current_epoch;
uint64_t recovery_epoch;
int first_time;
int num_cltrack_records;
int num_legacy_records;

static struct option longopts[] =
{
	{ "help", 0, NULL, 'h' },
	{ "foreground", 0, NULL, 'F' },
	{ "debug", 0, NULL, 'd' },
	{ "pipefsdir", 1, NULL, 'p' },
	{ "storagedir", 1, NULL, 's' },
	{ NULL, 0, 0, 0 },
};

/* forward declarations */
static void cldcb(int UNUSED(fd), short which, void *data);

static void
sig_die(int signal)
{
	if (signal_received) {
		xlog(D_GENERAL, "forced exiting on signal %d\n", signal);
		exit(0);
	}

	signal_received = true;
	xlog(D_GENERAL, "exiting on signal %d\n", signal);
	event_base_loopexit(evbase, NULL);
}

static void
usage(char *progname)
{
	printf("%s [ -hFd ] [ -p pipefsdir ] [ -s storagedir ]\n", progname);
}

static int
cld_set_caps(void)
{
	int ret = 0;
#ifdef HAVE_SYS_CAPABILITY_H
	unsigned long i;
	cap_t caps;

	if (getuid() != 0) {
		xlog(L_ERROR, "Not running as root. Daemon won't be able to "
			      "open the pipe after dropping capabilities!");
		return -EINVAL;
	}

	/* prune the bounding set to nothing */
	for (i = 0; prctl(PR_CAPBSET_READ, i, 0, 0, 0) >= 0 ; ++i) {
		ret = prctl(PR_CAPBSET_DROP, i, 0, 0, 0);
		if (ret) {
			xlog(L_ERROR, "Unable to prune capability %lu from "
				      "bounding set: %m", i);
			return -errno;
		}
	}

	/* get a blank capset */
	caps = cap_init();
	if (caps == NULL) {
		xlog(L_ERROR, "Unable to get blank capability set: %m");
		return -errno;
	}

	/* reset the process capabilities */
	if (cap_set_proc(caps) != 0) {
		xlog(L_ERROR, "Unable to set process capabilities: %m");
		ret = -errno;
	}
	cap_free(caps);
#endif
	return ret;
}

#define INOTIFY_EVENT_MAX (sizeof(struct inotify_event) + NAME_MAX)

static int
cld_pipe_open(struct cld_client *clnt)
{
	int fd;
	struct event *ev;

	xlog(D_GENERAL, "%s: opening upcall pipe %s", __func__, pipepath);
	fd = open(pipepath, O_RDWR, 0);
	if (fd < 0) {
		xlog(D_GENERAL, "%s: open of %s failed: %m", __func__, pipepath);
		return -errno;
	}

	ev = event_new(evbase, fd, EV_READ, cldcb, clnt);
	if (ev == NULL) {
		xlog(D_GENERAL, "%s: failed to create event for %s", __func__, pipepath);
		close(fd);
		return -ENOMEM;
	}

	if (clnt->cl_event && event_initialized(clnt->cl_event)) {
		event_del(clnt->cl_event);
		event_free(clnt->cl_event);
	}
	if (clnt->cl_fd >= 0)
		close(clnt->cl_fd);

	clnt->cl_fd = fd;
	clnt->cl_event = ev;
	/* event_add is done by the caller */
	return 0;
}

static void
cld_inotify_cb(int UNUSED(fd), short which, void *data)
{
	int ret;
	size_t elen;
	ssize_t rret;
	char evbuf[INOTIFY_EVENT_MAX];
	char *dirc = NULL, *pname;
	struct inotify_event *event = (struct inotify_event *)evbuf;
	struct cld_client *clnt = data;

	if (which != EV_READ)
		return;

	xlog(D_GENERAL, "%s: called for EV_READ", __func__);

	dirc = strndup(pipepath, PATH_MAX);
	if (!dirc) {
		xlog(L_ERROR, "%s: unable to allocate memory", __func__);
		goto out;
	}

	rret = read(inotify_fd, evbuf, INOTIFY_EVENT_MAX);
	if (rret < 0) {
		xlog(L_ERROR, "%s: read from inotify fd failed: %m", __func__);
		goto out;
	}

	/* check to see if we have a filename in the evbuf */
	if (!event->len) {
		xlog(D_GENERAL, "%s: no filename in inotify event", __func__);
		goto out;
	}

	pname = basename(dirc);
	elen = strnlen(event->name, event->len);

	/* does the filename match our pipe? */
	if (strlen(pname) != elen || memcmp(pname, event->name, elen)) {
		xlog(D_GENERAL, "%s: wrong filename (%s)", __func__,
				event->name);
		goto out;
	}

	ret = cld_pipe_open(clnt);
	switch (ret) {
	case 0:
		/* readd the event for the cl_event pipe */
		event_add(clnt->cl_event, NULL);
		break;
	case -ENOENT:
		/* pipe must have disappeared, wait for it to come back */
		goto out;
	default:
		/* anything else is fatal */
		xlog(L_FATAL, "%s: unable to open new pipe (%d). Aborting.",
			__func__, ret);
		exit(ret);
	}

out:
	event_add(pipedir_event, NULL);
	free(dirc);
}

static int
cld_inotify_setup(void)
{
	int ret;
	char *dirc, *dname;

	dirc = strndup(pipepath, PATH_MAX);
	if (!dirc) {
		xlog_err("%s: unable to allocate memory", __func__);
		ret = -ENOMEM;
		goto out_free;
	}

	dname = dirname(dirc);

	inotify_fd = inotify_init();
	if (inotify_fd < 0) {
		xlog_err("%s: inotify_init failed: %m", __func__);
		ret = -errno;
		goto out_free;
	}

	ret = inotify_add_watch(inotify_fd, dname, IN_CREATE);
	if (ret < 0) {
		xlog_err("%s: inotify_add_watch failed: %m", __func__);
		ret = -errno;
		goto out_err;
	} else
		ret = 0;

out_free:
	free(dirc);
	return ret;
out_err:
	close(inotify_fd);
	goto out_free;
}

/*
 * Set an inotify watch on the directory that should contain the pipe, and then
 * try to open it. If it fails with anything but -ENOENT, return the error
 * immediately.
 *
 * If it succeeds, then set up the pipe event handler. At that point, set up
 * the inotify event handler and go ahead and return success.
 */
static int
cld_pipe_init(struct cld_client *clnt)
{
	int ret;

	xlog(D_GENERAL, "%s: init pipe handlers", __func__);

	ret = cld_inotify_setup();
	if (ret != 0)
		goto out;

	clnt->cl_fd = -1;
	ret = cld_pipe_open(clnt);
	switch (ret) {
	case 0:
		/* add the event and we're good to go */
		event_add(clnt->cl_event, NULL);
		break;
	case -ENOENT:
		/* ignore this error -- cld_inotify_cb will handle it */
		ret = 0;
		break;
	default:
		/* anything else is fatal */
		close(inotify_fd);
		goto out;
	}

	/* set event for inotify read */
	pipedir_event = event_new(evbase, inotify_fd, EV_READ, cld_inotify_cb, clnt);
	if (pipedir_event == NULL) {
		close(inotify_fd);
		return -ENOMEM;
	}
	event_add(pipedir_event, NULL);
out:
	return ret;
}

/*
 * Older kernels will not tell nfsdcld when a grace period has started.
 * Therefore we have to peek at the /proc/fs/nfsd/v4_end_grace file to
 * see if nfsd is in grace.  We have to do this for create and remove
 * upcalls to ensure that the correct table is being updated - otherwise
 * we could lose client records when the grace period is lifted.
 */
static int
cld_check_grace_period(void)
{
	int fd, ret = 0;
	char c;

	if (!old_kernel)
		return 0;
	if (recovery_epoch != 0)
		return 0;
	fd = open(NFSD_END_GRACE_FILE, O_RDONLY);
	if (fd < 0) {
		xlog(L_WARNING, "Unable to open %s: %m",
			NFSD_END_GRACE_FILE);
		return 1;
	}
	if (read(fd, &c, 1) < 0) {
		xlog(L_WARNING, "Unable to read from %s: %m",
			NFSD_END_GRACE_FILE);
		close(fd);
		return 1;
	}
	close(fd);
	if (c == 'N') {
		xlog(L_WARNING, "nfsd is in grace but didn't send a gracestart upcall, "
			"please update the kernel");
		ret = sqlite_grace_start();
	}
	return ret;
}

#if UPCALL_VERSION >= 2
static ssize_t cld_message_size(void *msg)
{
	struct cld_msg_hdr *hdr = (struct cld_msg_hdr *)msg;

	switch (hdr->cm_vers) {
	case 1:
		return sizeof(struct cld_msg);
	case 2:
		return sizeof(struct cld_msg_v2);
	default:
		xlog(L_FATAL, "%s invalid upcall version %d", __func__,
		     hdr->cm_vers);
		exit(-EINVAL);
	}
}
#else
static ssize_t cld_message_size(void *UNUSED(msg))
{
	return sizeof(struct cld_msg);
}
#endif

static void
cld_not_implemented(struct cld_client *clnt)
{
	int ret;
	ssize_t bsize, wsize;
#if UPCALL_VERSION >= 2
	struct cld_msg_v2 *cmsg = &clnt->cl_u.cl_msg_v2;
#else
	struct cld_msg *cmsg = &clnt->cl_u.cl_msg;
#endif

	xlog(D_GENERAL, "%s: downcalling with not implemented error", __func__);

	/* set up reply */
	cmsg->cm_status = -EOPNOTSUPP;

	bsize = cld_message_size(cmsg);
	wsize = atomicio((void *)write, clnt->cl_fd, cmsg, bsize);
	if (wsize != bsize)
		xlog(L_ERROR, "%s: problem writing to cld pipe (%zd): %m",
			 __func__, wsize);

	/* reopen pipe, just to be sure */
	ret = cld_pipe_open(clnt);
	if (ret) {
		xlog(L_FATAL, "%s: unable to reopen pipe: %d", __func__, ret);
		exit(ret);
	}
}

static void
cld_get_version(struct cld_client *clnt)
{
	int ret;
	ssize_t bsize, wsize;
#if UPCALL_VERSION >= 2
	struct cld_msg_v2 *cmsg = &clnt->cl_u.cl_msg_v2;
#else
	struct cld_msg *cmsg = &clnt->cl_u.cl_msg;
#endif

	xlog(D_GENERAL, "%s: version = %u.", __func__, UPCALL_VERSION);

	cmsg->cm_u.cm_version = UPCALL_VERSION;
	cmsg->cm_status = 0;

	bsize = cld_message_size(cmsg);
	xlog(D_GENERAL, "Doing downcall with status %d", cmsg->cm_status);
	wsize = atomicio((void *)write, clnt->cl_fd, cmsg, bsize);
	if (wsize != bsize) {
		xlog(L_ERROR, "%s: problem writing to cld pipe (%zd): %m",
			 __func__, wsize);
		ret = cld_pipe_open(clnt);
		if (ret) {
			xlog(L_FATAL, "%s: unable to reopen pipe: %d",
					__func__, ret);
			exit(ret);
		}
	}
}

static void
cld_create(struct cld_client *clnt)
{
	int ret;
	ssize_t bsize, wsize;
#if UPCALL_VERSION >= 2
	struct cld_msg_v2 *cmsg = &clnt->cl_u.cl_msg_v2;
#else
	struct cld_msg *cmsg = &clnt->cl_u.cl_msg;
#endif

	ret = cld_check_grace_period();
	if (ret)
		goto reply;

	xlog(D_GENERAL, "%s: create client record.", __func__);

#if UPCALL_VERSION >= 2
	if (cmsg->cm_vers >= 2)
		ret = sqlite_insert_client_and_princhash(
					cmsg->cm_u.cm_clntinfo.cc_name.cn_id,
					cmsg->cm_u.cm_clntinfo.cc_name.cn_len,
					cmsg->cm_u.cm_clntinfo.cc_princhash.cp_data,
					cmsg->cm_u.cm_clntinfo.cc_princhash.cp_len);
	else
		ret = sqlite_insert_client(cmsg->cm_u.cm_name.cn_id,
					   cmsg->cm_u.cm_name.cn_len);
#else
	ret = sqlite_insert_client(cmsg->cm_u.cm_name.cn_id,
				   cmsg->cm_u.cm_name.cn_len);
#endif

reply:
	cmsg->cm_status = ret ? -EREMOTEIO : ret;

	bsize = cld_message_size(cmsg);
	xlog(D_GENERAL, "Doing downcall with status %d", cmsg->cm_status);
	wsize = atomicio((void *)write, clnt->cl_fd, cmsg, bsize);
	if (wsize != bsize) {
		xlog(L_ERROR, "%s: problem writing to cld pipe (%zd): %m",
			 __func__, wsize);
		ret = cld_pipe_open(clnt);
		if (ret) {
			xlog(L_FATAL, "%s: unable to reopen pipe: %d",
					__func__, ret);
			exit(ret);
		}
	}
}

static void
cld_remove(struct cld_client *clnt)
{
	int ret;
	ssize_t bsize, wsize;
#if UPCALL_VERSION >= 2
	struct cld_msg_v2 *cmsg = &clnt->cl_u.cl_msg_v2;
#else
	struct cld_msg *cmsg = &clnt->cl_u.cl_msg;
#endif

	ret = cld_check_grace_period();
	if (ret)
		goto reply;

	xlog(D_GENERAL, "%s: remove client record.", __func__);

	ret = sqlite_remove_client(cmsg->cm_u.cm_name.cn_id,
				   cmsg->cm_u.cm_name.cn_len);

reply:
	cmsg->cm_status = ret ? -EREMOTEIO : ret;

	bsize = cld_message_size(cmsg);
	xlog(D_GENERAL, "%s: downcall with status %d", __func__,
			cmsg->cm_status);
	wsize = atomicio((void *)write, clnt->cl_fd, cmsg, bsize);
	if (wsize != bsize) {
		xlog(L_ERROR, "%s: problem writing to cld pipe (%zd): %m",
			 __func__, wsize);
		ret = cld_pipe_open(clnt);
		if (ret) {
			xlog(L_FATAL, "%s: unable to reopen pipe: %d",
					__func__, ret);
			exit(ret);
		}
	}
}

static void
cld_check(struct cld_client *clnt)
{
	int ret;
	ssize_t bsize, wsize;
#if UPCALL_VERSION >= 2
	struct cld_msg_v2 *cmsg = &clnt->cl_u.cl_msg_v2;
#else
	struct cld_msg *cmsg = &clnt->cl_u.cl_msg;
#endif

	/*
	 * If we get a check upcall at all, it means we're talking to an old
	 * kernel.  Furthermore, if we're not in grace it means this is the
	 * first client to do a reclaim.  Log a message and use
	 * sqlite_grace_start() to advance the epoch numbers.
	 */
	if (recovery_epoch == 0) {
		xlog(D_GENERAL, "%s: received a check upcall, please update the kernel",
			__func__);
		ret = sqlite_grace_start();
		if (ret)
			goto reply;
	}

	xlog(D_GENERAL, "%s: check client record", __func__);

	ret = sqlite_check_client(cmsg->cm_u.cm_name.cn_id,
				  cmsg->cm_u.cm_name.cn_len);

reply:
	/* set up reply */
	cmsg->cm_status = ret ? -EACCES : ret;

	bsize = cld_message_size(cmsg);
	xlog(D_GENERAL, "%s: downcall with status %d", __func__,
			cmsg->cm_status);
	wsize = atomicio((void *)write, clnt->cl_fd, cmsg, bsize);
	if (wsize != bsize) {
		xlog(L_ERROR, "%s: problem writing to cld pipe (%zd): %m",
			 __func__, wsize);
		ret = cld_pipe_open(clnt);
		if (ret) {
			xlog(L_FATAL, "%s: unable to reopen pipe: %d",
					__func__, ret);
			exit(ret);
		}
	}
}

static void
cld_gracedone(struct cld_client *clnt)
{
	int ret;
	ssize_t bsize, wsize;
#if UPCALL_VERSION >= 2
	struct cld_msg_v2 *cmsg = &clnt->cl_u.cl_msg_v2;
#else
	struct cld_msg *cmsg = &clnt->cl_u.cl_msg;
#endif

	/*
	 * If we got a "gracedone" upcall while we're not in grace, then
	 * 1) we must be talking to an old kernel
	 * 2) no clients attempted to reclaim
	 * In that case, log a message and use sqlite_grace_start() to
	 * advance the epoch numbers, and then proceed as normal.
	 */
	if (recovery_epoch == 0) {
		xlog(D_GENERAL, "%s: received gracedone upcall "
			"while not in grace, please update the kernel",
			__func__);
		ret = sqlite_grace_start();
		if (ret)
			goto reply;
	}

	xlog(D_GENERAL, "%s: grace done.", __func__);

	ret = sqlite_grace_done();

	if (first_time) {
		if (num_cltrack_records > 0)
			sqlite_delete_cltrack_records();
		if (num_legacy_records > 0)
			legacy_clear_recdir();
		sqlite_first_time_done();
		first_time = 0;
	}

reply:
	/* set up reply: downcall with 0 status */
	cmsg->cm_status = ret ? -EREMOTEIO : ret;

	bsize = cld_message_size(cmsg);
	xlog(D_GENERAL, "Doing downcall with status %d", cmsg->cm_status);
	wsize = atomicio((void *)write, clnt->cl_fd, cmsg, bsize);
	if (wsize != bsize) {
		xlog(L_ERROR, "%s: problem writing to cld pipe (%zd): %m",
			 __func__, wsize);
		ret = cld_pipe_open(clnt);
		if (ret) {
			xlog(L_FATAL, "%s: unable to reopen pipe: %d",
					__func__, ret);
			exit(ret);
		}
	}
}

static int
gracestart_callback(struct cld_client *clnt) {
	ssize_t bsize, wsize;
#if UPCALL_VERSION >= 2
	struct cld_msg_v2 *cmsg = &clnt->cl_u.cl_msg_v2;
#else
	struct cld_msg *cmsg = &clnt->cl_u.cl_msg;
#endif

	cmsg->cm_status = -EINPROGRESS;

	bsize = cld_message_size(cmsg);
	xlog(D_GENERAL, "Sending client %.*s",
			cmsg->cm_u.cm_name.cn_len, cmsg->cm_u.cm_name.cn_id);
	wsize = atomicio((void *)write, clnt->cl_fd, cmsg, bsize);
	if (wsize != bsize)
		return -EIO;
	return 0;
}

static void
cld_gracestart(struct cld_client *clnt)
{
	int ret;
	ssize_t bsize, wsize;
#if UPCALL_VERSION >= 2
	struct cld_msg_v2 *cmsg = &clnt->cl_u.cl_msg_v2;
#else
	struct cld_msg *cmsg = &clnt->cl_u.cl_msg;
#endif

	xlog(D_GENERAL, "%s: updating grace epochs", __func__);

	ret = sqlite_grace_start();
	if (ret)
		goto reply;

	xlog(D_GENERAL, "%s: sending client records to the kernel", __func__);

	ret = sqlite_iterate_recovery(&gracestart_callback, clnt);

reply:
	/* set up reply: downcall with 0 status */
	cmsg->cm_status = ret ? -EREMOTEIO : ret;

	bsize = cld_message_size(cmsg);
	xlog(D_GENERAL, "Doing downcall with status %d", cmsg->cm_status);
	wsize = atomicio((void *)write, clnt->cl_fd, cmsg, bsize);
	if (wsize != bsize) {
		xlog(L_ERROR, "%s: problem writing to cld pipe (%zd): %m",
			 __func__, wsize);
		ret = cld_pipe_open(clnt);
		if (ret) {
			xlog(L_FATAL, "%s: unable to reopen pipe: %d",
					__func__, ret);
			exit(ret);
		}
	}
}

static void
cldcb(int UNUSED(fd), short which, void *data)
{
	ssize_t len;
	struct cld_client *clnt = data;
#if UPCALL_VERSION >= 2
	struct cld_msg_v2 *cmsg = &clnt->cl_u.cl_msg_v2;
#else
	struct cld_msg *cmsg = &clnt->cl_u.cl_msg;
#endif

	if (which != EV_READ)
		goto out;

	len = atomicio(read, clnt->cl_fd, cmsg, sizeof(*cmsg));
	if (len <= 0) {
		xlog(L_ERROR, "%s: pipe read failed: %m", __func__);
		cld_pipe_open(clnt);
		goto out;
	}

	if (cmsg->cm_vers > UPCALL_VERSION) {
		xlog(L_ERROR, "%s: unsupported upcall version: %hu",
				__func__, cmsg->cm_vers);
		cld_pipe_open(clnt);
		goto out;
	}

	switch(cmsg->cm_cmd) {
	case Cld_Create:
		cld_create(clnt);
		break;
	case Cld_Remove:
		cld_remove(clnt);
		break;
	case Cld_Check:
		cld_check(clnt);
		break;
	case Cld_GraceDone:
		cld_gracedone(clnt);
		break;
	case Cld_GraceStart:
		cld_gracestart(clnt);
		break;
	case Cld_GetVersion:
		cld_get_version(clnt);
		break;
	default:
		xlog(L_WARNING, "%s: command %u is not yet implemented",
				__func__, cmsg->cm_cmd);
		cld_not_implemented(clnt);
	}
out:
	event_add(clnt->cl_event, NULL);
}

int
main(int argc, char **argv)
{
	int arg;
	int rc = 0;
	bool foreground = false;
	char *progname;
	char *storagedir = CLD_DEFAULT_STORAGEDIR;
	struct cld_client clnt;
	char *s;
	first_time = 0;
	num_cltrack_records = 0;
	num_legacy_records = 0;

	memset(&clnt, 0, sizeof(clnt));

	progname = strdup(basename(argv[0]));
	if (!progname) {
		fprintf(stderr, "%s: unable to allocate memory.\n", argv[0]);
		return 1;
	}

	evbase = event_base_new();
	if (evbase == NULL) {
		fprintf(stderr, "%s: unable to allocate event base.\n", argv[0]);
		return 1;
	}
	xlog_syslog(0);
	xlog_stderr(1);

	conf_init_file(NFS_CONFFILE);
	s = conf_get_str("general", "pipefs-directory");
	if (s)
		strlcpy(pipefs_dir, s, sizeof(pipefs_dir));
	s = conf_get_str("nfsdcld", "storagedir");
	if (s)
		storagedir = s;
	rc = conf_get_num("nfsdcld", "debug", 0);
	if (rc > 0)
		xlog_config(D_ALL, 1);

	/* process command-line options */
	while ((arg = getopt_long(argc, argv, "hdFp:s:", longopts,
				  NULL)) != EOF) {
		switch (arg) {
		case 'd':
			xlog_config(D_ALL, 1);
			break;
		case 'F':
			foreground = true;
			break;
		case 'p':
			strlcpy(pipefs_dir, optarg, sizeof(pipefs_dir));
			break;
		case 's':
			storagedir = optarg;
			break;
		default:
			usage(progname);
			free(progname);
			return 0;
		}
	}

	strlcpy(pipepath, pipefs_dir, sizeof(pipepath));
	strlcat(pipepath, DEFAULT_CLD_PATH, sizeof(pipepath));

	xlog_open(progname);
	if (!foreground) {
		xlog_syslog(1);
		xlog_stderr(0);
		rc = daemon(0, 0);
		if (rc) {
			xlog(L_ERROR, "Unable to daemonize: %m");
			goto out;
		}
	}

	/* drop all capabilities */
	rc = cld_set_caps();
	if (rc)
		goto out;

	/*
	 * now see if the storagedir is writable by root w/o CAP_DAC_OVERRIDE.
	 * If it isn't then give the user a warning but proceed as if
	 * everything is OK. If the DB has already been created, then
	 * everything might still work. If it doesn't exist at all, then
	 * assume that the maindb init will be able to create it. Fail on
	 * anything else.
	 */
	if (access(storagedir, W_OK) == -1) {
		switch (errno) {
		case EACCES:
			xlog(L_WARNING, "Storage directory %s is not writable. "
					"Should be owned by root and writable "
					"by owner!", storagedir);
			break;
		case ENOENT:
			/* ignore and assume that we can create dir as root */
			break;
		default:
			xlog(L_ERROR, "Unexpected error when checking access "
				      "on %s: %m", storagedir);
			rc = -errno;
			goto out;
		}
	}

	if (linux_version_code() < MAKE_VERSION(4, 20, 0))
		old_kernel = true;

	/* set up storage db */
	rc = sqlite_prepare_dbh(storagedir);
	if (rc) {
		xlog(L_ERROR, "Failed to open main database: %d", rc);
		goto out;
	}

	/* set up event handler */
	rc = cld_pipe_init(&clnt);
	if (rc)
		goto out;

	signal(SIGINT, sig_die);
	signal(SIGTERM, sig_die);

	xlog(D_GENERAL, "%s: Starting event dispatch handler.", __func__);
	rc = event_base_dispatch(evbase);
	if (rc < 0)
		xlog(L_ERROR, "%s: event_dispatch failed: %m", __func__);

out:
	if (clnt.cl_event)
		event_free(clnt.cl_event);
	if (clnt.cl_fd != -1)
		close(clnt.cl_fd);
	if (pipedir_event)
		event_free(pipedir_event);
	if (inotify_fd != -1)
		close(inotify_fd);

	event_base_free(evbase);
	sqlite_shutdown();

	free(progname);
	return rc;
}
