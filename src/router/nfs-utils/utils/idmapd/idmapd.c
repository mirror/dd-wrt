/*
 *  idmapd.c
 *
 *  Userland daemon for idmap.
 *
 *  Copyright (c) 2002 The Regents of the University of Michigan.
 *  All rights reserved.
 *
 *  Marius Aamodt Eriksen <marius@umich.edu>
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the University nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 *  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 *  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/inotify.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>

#include "nfs_idmap.h"

#include <err.h>
#include <errno.h>
#include <event2/event.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <netdb.h>
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <pwd.h>
#include <grp.h>
#include <limits.h>
#include <ctype.h>
#include <libgen.h>
#include <nfsidmap.h>

#include "xlog.h"
#include "conffile.h"
#include "queue.h"
#include "nfslib.h"

#ifndef PIPEFS_DIR
#define PIPEFS_DIR  NFS_STATEDIR "/rpc_pipefs/"
#endif

#ifndef NFSD_DIR
#define NFSD_DIR  "/proc/net/rpc"
#endif

#ifndef CLIENT_CACHE_TIMEOUT_FILE
#define CLIENT_CACHE_TIMEOUT_FILE "/proc/sys/fs/nfs/idmap_cache_timeout"
#endif

#ifndef NFS4NOBODY_USER
#define NFS4NOBODY_USER "nobody"
#endif

#ifndef NFS4NOBODY_GROUP
#define NFS4NOBODY_GROUP "nobody"
#endif

/* From Niels */
#define CONF_SAVE(w, f) do {			\
	char *p = f;				\
	if (p != NULL)				\
		(w) = p;			\
} while (0)

#define IC_IDNAME 0
#define IC_IDNAME_CHAN  NFSD_DIR "/nfs4.idtoname/channel"
#define IC_IDNAME_FLUSH NFSD_DIR "/nfs4.idtoname/flush"

#define IC_NAMEID 1
#define IC_NAMEID_CHAN  NFSD_DIR "/nfs4.nametoid/channel"
#define IC_NAMEID_FLUSH NFSD_DIR "/nfs4.nametoid/flush"

struct idmap_client {
	short                      ic_which;
	char                       ic_clid[30];
	char                      *ic_id;
	char                       ic_path[PATH_MAX];
	int                        ic_fd;
	int                        ic_dirfd;
	int                        ic_scanned;
	struct event              *ic_event;
	TAILQ_ENTRY(idmap_client)  ic_next;
};
static struct idmap_client nfsd_ic[2] = {
{
	.ic_which = IC_IDNAME, 
	.ic_clid = "", 
	.ic_id = "Server", 
	.ic_path = IC_IDNAME_CHAN, 
	.ic_fd = -1, 
	.ic_dirfd = -1, 
	.ic_scanned = 0
},
{
	.ic_which = IC_NAMEID, 
	.ic_clid = "", 
	.ic_id = "Server", 
	.ic_path = IC_NAMEID_CHAN, 
	.ic_fd = -1, 
	.ic_dirfd = -1, 
	.ic_scanned = 0
},
};

TAILQ_HEAD(idmap_clientq, idmap_client);

static void dirscancb(int, short, void *);
static void clntscancb(int, short, void *);
static void svrreopen(int, short, void *);
static int  nfsopen(struct idmap_client *);
static void nfscb(int, short, void *);
static void nfsdcb(int, short, void *);
static int  addfield(char **, ssize_t *, char *);
static int  getfield(char **, char *, size_t);

static void imconv(struct idmap_client *, struct idmap_msg *);
static void idtonameres(struct idmap_msg *);
static void nametoidres(struct idmap_msg *);

static int nfsdopen(void);
static void nfsdclose(void);
static int nfsdopenone(struct idmap_client *);
static void nfsdreopen_one(struct idmap_client *);
static void nfsdreopen(void);

static int verbose = 0;
#define DEFAULT_IDMAP_CACHE_EXPIRY 600 /* seconds */
static int cache_entry_expiration = 0;
static char pipefsdir[PATH_MAX];
static char *nobodyuser, *nobodygroup;
static uid_t nobodyuid;
static gid_t nobodygid;
static struct event_base *evbase = NULL;
static bool signal_received = false;
static int inotify_fd = -1;

static void
sig_die(int signal)
{
	if (signal_received) {
		xlog_warn("forced exiting on signal %d\n", signal);
		exit(0);
	}

	signal_received = true;
	xlog_warn("exiting on signal %d\n", signal);
	event_base_loopexit(evbase, NULL);
}

static int
flush_nfsd_cache(char *path, time_t now)
{
	int fd;
	char stime[32];

	sprintf(stime, "%" PRId64 "\n", (int64_t)now);
	fd = open(path, O_RDWR);
	if (fd == -1)
		return -1;
	if (write(fd, stime, strlen(stime)) != (ssize_t)strlen(stime)) {
		errx(1, "Flushing nfsd cache failed: errno %d (%s)",
			errno, strerror(errno));
	}
	close(fd);
	return 0;
}

static int
flush_nfsd_idmap_cache(void)
{
	time_t now = time(NULL);
	int ret;

	ret = flush_nfsd_cache(IC_IDNAME_FLUSH, now);
	if (ret)
		return ret;
	ret = flush_nfsd_cache(IC_NAMEID_FLUSH, now);
	return ret;
}

static void usage(char *progname)
{
	fprintf(stderr, "Usage: %s [-hfvCS] [-p path] [-c path]\n",
		basename(progname));
}

int
main(int argc, char **argv)
{
	int wd = -1, opt, fg = 0, nfsdret = -1;
	struct idmap_clientq icq;
	struct event *rootdirev = NULL, *clntdirev = NULL,
		     *svrdirev = NULL, *inotifyev = NULL;
	struct event *initialize = NULL;
	struct passwd *pw;
	struct group *gr;
	struct stat sb;
	char *xpipefsdir = NULL;
	int serverstart = 1, clientstart = 1;
	int ret;
	char *progname;
	char *conf_path = NULL;

	nobodyuser = NFS4NOBODY_USER;
	nobodygroup = NFS4NOBODY_GROUP;
	strlcpy(pipefsdir, PIPEFS_DIR, sizeof(pipefsdir));

	if ((progname = strrchr(argv[0], '/')))
		progname++;
	else
		progname = argv[0];
	xlog_open(progname);

#define GETOPTSTR "hvfd:p:U:G:c:CS"
	opterr=0; /* Turn off error messages */
	while ((opt = getopt(argc, argv, GETOPTSTR)) != -1) {
		if (opt == 'c') {
			warnx("-c is deprecated and may be removed in the "
			      "future.  See idmapd(8).");
			conf_path = optarg;
		}
		if (opt == '?') {
			if (strchr(GETOPTSTR, optopt))
				warnx("'-%c' option requires an argument.", optopt);
			else
				warnx("'-%c' is an invalid argument.", optopt);
			usage(progname);
			exit(1);
		}
	}
	optind = 1;

	if (conf_path) { /* deprecated -c option was specified */
		if (stat(conf_path, &sb) == -1 && (errno == ENOENT || errno == EACCES)) {
			warn("Skipping configuration file \"%s\"", conf_path);
			conf_path = NULL;
		} else {
			conf_init_file(conf_path);
			verbose = conf_get_num("General", "Verbosity", 0);
			cache_entry_expiration = conf_get_num("General",
					"Cache-Expiration", DEFAULT_IDMAP_CACHE_EXPIRY);
			CONF_SAVE(xpipefsdir, conf_get_str("General", "Pipefs-Directory"));
			if (xpipefsdir != NULL)
				strlcpy(pipefsdir, xpipefsdir, sizeof(pipefsdir));
			CONF_SAVE(nobodyuser, conf_get_str("Mapping", "Nobody-User"));
			CONF_SAVE(nobodygroup, conf_get_str("Mapping", "Nobody-Group"));
			if (conf_get_bool("General", "server-only", false))
				clientstart = 0;
			if (conf_get_bool("General", "client-only", false))
				serverstart = 0;
		}
	} else {
		conf_path = NFS_CONFFILE;
		conf_init_file(conf_path);
		CONF_SAVE(xpipefsdir, conf_get_str("General", "Pipefs-Directory"));
		if (xpipefsdir != NULL)
			strlcpy(pipefsdir, xpipefsdir, sizeof(pipefsdir));

		conf_path = _PATH_IDMAPDCONF;
		conf_init_file(conf_path);
		verbose = conf_get_num("General", "Verbosity", 0);
		cache_entry_expiration = conf_get_num("General",
				"cache-expiration", DEFAULT_IDMAP_CACHE_EXPIRY);
		CONF_SAVE(nobodyuser, conf_get_str("Mapping", "Nobody-User"));
		CONF_SAVE(nobodygroup, conf_get_str("Mapping", "Nobody-Group"));
		if (conf_get_bool("General", "server-only", false))
			clientstart = 0;
		if (conf_get_bool("General", "client-only", false))
			serverstart = 0;
	}

	while ((opt = getopt(argc, argv, GETOPTSTR)) != -1)
		switch (opt) {
		case 'v':
			verbose++;
			break;
		case 'f':
			fg = 1;
			break;
		case 'p':
			strlcpy(pipefsdir, optarg, sizeof(pipefsdir));
			break;
		case 'd':
		case 'U':
		case 'G':
			errx(1, "the -d, -U, and -G options have been removed;"
				" please use the configuration file instead.");
		case 'C':
			serverstart = 0;
			break;
		case 'S':
			clientstart = 0;
			break;
		case 'h':
			usage(progname);
			exit(0);
		default:
			break;
		}

	if (!serverstart && !clientstart)
		errx(1, "it is illegal to specify both -C and -S");

	strncat(pipefsdir, "/nfs", sizeof(pipefsdir)-1);

	daemon_init(fg);

	if ((pw = getpwnam(nobodyuser)) == NULL)
		errx(1, "Could not find user \"%s\"", nobodyuser);
	nobodyuid = pw->pw_uid;

	if ((gr = getgrnam(nobodygroup)) == NULL)
		errx(1, "Could not find group \"%s\"", nobodygroup);
	nobodygid = gr->gr_gid;

#ifdef HAVE_NFS4_SET_DEBUG
	nfs4_set_debug(verbose, xlog_warn);
#endif
	if (conf_path == NULL)
		conf_path = _PATH_IDMAPDCONF;
	if (nfs4_init_name_mapping(conf_path))
		errx(1, "Unable to create name to user id mappings.");

	evbase = event_base_new();
	if (evbase == NULL)
		errx(1, "Failed to create event base.");

	if (verbose > 1)
		xlog_warn("Expiration time is %d seconds.",
			     cache_entry_expiration);
	if (serverstart) {
		nfsdret = nfsdopen();
		if (nfsdret == 0) {
			ret = flush_nfsd_idmap_cache();
			if (ret)
				xlog_err("main: Failed to flush nfsd idmap cache\n: %s", strerror(errno));
		}
	}

	if (clientstart) {
		struct timeval now = {
			.tv_sec = 0,
			.tv_usec = 0,
		};

		if (cache_entry_expiration != DEFAULT_IDMAP_CACHE_EXPIRY) {
			int timeout_fd, len;
			char timeout_buf[12];
			if ((timeout_fd = open(CLIENT_CACHE_TIMEOUT_FILE,
					       O_RDWR)) == -1) {
				xlog_warn("Unable to open '%s' to set "
					     "client cache expiration time "
					     "to %d seconds\n",
					     CLIENT_CACHE_TIMEOUT_FILE,
					     cache_entry_expiration);
			} else {
				len = snprintf(timeout_buf, sizeof(timeout_buf),
					       "%d", cache_entry_expiration);
				if ((write(timeout_fd, timeout_buf, len)) != len)
					xlog_warn("Error writing '%s' to "
						     "'%s' to set client "
						     "cache expiration time\n",
						     timeout_buf,
						     CLIENT_CACHE_TIMEOUT_FILE);
				close(timeout_fd);
			}
		}

		inotify_fd = inotify_init1(IN_NONBLOCK);
		if (inotify_fd == -1) {
			xlog_err("Unable to initialise inotify_init1: %s\n", strerror(errno));
		} else {
			wd = inotify_add_watch(inotify_fd, pipefsdir, IN_CREATE | IN_DELETE);
			if (wd < 0)
				xlog_err("Unable to inotify_add_watch(%s): %s\n", pipefsdir, strerror(errno));
		}

		TAILQ_INIT(&icq);

		signal(SIGINT, sig_die);
		signal(SIGTERM, sig_die);

		/* These events are persistent */
		rootdirev = evsignal_new(evbase, SIGUSR1, dirscancb, &icq);
		if (rootdirev == NULL)
			errx(1, "Failed to create SIGUSR1 event.");
		evsignal_add(rootdirev, NULL);
		clntdirev = evsignal_new(evbase, SIGUSR2, clntscancb, &icq);
		if (clntdirev == NULL)
			errx(1, "Failed to create SIGUSR2 event.");
		evsignal_add(clntdirev, NULL);
		svrdirev = evsignal_new(evbase, SIGHUP, svrreopen, NULL);
		if (svrdirev == NULL)
			errx(1, "Failed to create SIGHUP event.");
		evsignal_add(svrdirev, NULL);
		if ( wd >= 0) {
			inotifyev = event_new(evbase, inotify_fd,
					      EV_READ | EV_PERSIST, dirscancb, &icq);
			if (inotifyev == NULL)
				errx(1, "Failed to create inotify read event.");
			event_add(inotifyev, NULL);
		}

		/* Fetch current state */
		/* (Delay till start of event_dispatch to avoid possibly losing
		 * a SIGUSR1 between here and the call to event_dispatch().) */
		initialize = evtimer_new(evbase, dirscancb, &icq);
		if (initialize == NULL)
			errx(1, "Failed to create initialize event.");
		evtimer_add(initialize, &now);
	}

	if (nfsdret != 0 && wd < 0)
		xlog_err("main: Neither NFS client nor NFSd found");

	daemon_ready();

	if (event_base_dispatch(evbase) < 0)
		xlog_err("main: event_dispatch returns errno %d (%s)",
			    errno, strerror(errno));

	nfs4_term_name_mapping();
	nfsdclose();

	if (inotifyev)
		event_free(inotifyev);
	if (inotify_fd != -1)
		close(inotify_fd);

	if (initialize)
		event_free(initialize);
	if (rootdirev)
		event_free(rootdirev);
	if (clntdirev)
		event_free(clntdirev);
	if (svrdirev)
		event_free(svrdirev);
	event_base_free(evbase);

	return 0;
}

static void
flush_inotify(int fd)
{
	while (true) {
		char buf[4096] __attribute__ ((aligned(__alignof__(struct inotify_event))));
		const struct inotify_event *ev;
		ssize_t len;
		char *ptr;

		len = read(fd, buf, sizeof(buf));
		if (len == -1 && errno == EINTR)
			continue;

		if (len <= 0)
			break;

		for (ptr = buf; ptr < buf + len;
		     ptr += sizeof(struct inotify_event) + ev->len) {

			ev = (const struct inotify_event *)ptr;
			if (verbose > 2)
				xlog_warn("pipefs inotify: wd=%i, mask=0x%08x, len=%i, name=%s",
				  ev->wd, ev->mask, ev->len, ev->len ? ev->name : "");
		}
	}
}

static void
dirscancb(int fd, short UNUSED(which), void *data)
{
	int nent, i;
	struct dirent **ents;
	struct idmap_client *ic, *nextic;
	char path[PATH_MAX+256]; /* + sizeof(d_name) */
	struct idmap_clientq *icq = data;

	if (fd != -1)
		flush_inotify(fd);

	TAILQ_FOREACH(ic, icq, ic_next) {
		ic->ic_scanned = 0;
	}

	nent = scandir(pipefsdir, &ents, NULL, alphasort);
	if (nent == -1) {
		xlog_warn("dirscancb: scandir(%s): %s", pipefsdir, strerror(errno));
		return;
	}

	for (i = 0;  i < nent; i++) {
		if (ents[i]->d_reclen > 4 &&
		    strncmp(ents[i]->d_name, "clnt", 4) == 0) {
			TAILQ_FOREACH(ic, icq, ic_next)
			    if (strcmp(ents[i]->d_name + 4, ic->ic_clid) == 0)
				    break;
			if (ic != NULL)
				goto next;

			if ((ic = calloc(1, sizeof(*ic))) == NULL)
				goto out;
			strlcpy(ic->ic_clid, ents[i]->d_name + 4,
			    sizeof(ic->ic_clid));
			path[0] = '\0';
			snprintf(path, sizeof(path), "%s/%s",
			    pipefsdir, ents[i]->d_name);

			if ((ic->ic_dirfd = open(path, O_RDONLY, 0)) == -1) {
				if (verbose > 0)
					xlog_warn("dirscancb: open(%s): %s", path, strerror(errno));
				free(ic);
				goto out;
			}

			strlcat(path, "/idmap", sizeof(path));
			strlcpy(ic->ic_path, path, sizeof(ic->ic_path));

			if (nfsopen(ic) == -1) {
				close(ic->ic_dirfd);
				free(ic);
				goto out;
			}

			if (verbose > 2)
				xlog_warn("New client: %s", ic->ic_clid);

			ic->ic_id = "Client";

			TAILQ_INSERT_TAIL(icq, ic, ic_next);

		next:
			ic->ic_scanned = 1;
		}
	}

	ic = TAILQ_FIRST(icq);
	while(ic != NULL) {
		nextic=TAILQ_NEXT(ic, ic_next);
		if (!ic->ic_scanned) {
			if (ic->ic_event)
				event_free(ic->ic_event);
			if (ic->ic_fd != -1)
				close(ic->ic_fd);
			if (ic->ic_dirfd != -1)
				close(ic->ic_dirfd);
			TAILQ_REMOVE(icq, ic, ic_next);
			if (verbose > 2) {
				xlog_warn("Stale client: %s", ic->ic_clid);
				xlog_warn("\t-> closed %s", ic->ic_path);
			}
			free(ic);
		}
		ic = nextic;
	}

out:
	for (i = 0;  i < nent; i++)
		free(ents[i]);
	free(ents);
	return;
}

static void
svrreopen(int UNUSED(fd), short UNUSED(which), void *UNUSED(data))
{
	nfsdreopen();
}

static void
clntscancb(int UNUSED(fd), short UNUSED(which), void *data)
{
	struct idmap_clientq *icq = data;
	struct idmap_client *ic, *ic_next;

	for (ic = TAILQ_FIRST(icq); ic != NULL; ic = ic_next) { 
		ic_next = TAILQ_NEXT(ic, ic_next);
		if (ic->ic_fd == -1 && nfsopen(ic) == -1) {
			close(ic->ic_dirfd);
			TAILQ_REMOVE(icq, ic, ic_next);
			free(ic);
		}
	}
}

static void
nfsdcb(int UNUSED(fd), short which, void *data)
{
	struct idmap_client *ic = data;
	struct idmap_msg im;
	u_char buf[IDMAP_MAXMSGSZ + 1];
	ssize_t len;
	ssize_t bsiz;
	char *bp, typebuf[IDMAP_MAXMSGSZ],
		buf1[IDMAP_MAXMSGSZ], authbuf[IDMAP_MAXMSGSZ], *p;
	unsigned long tmp;

	if (which != EV_READ)
		return;

	len = read(ic->ic_fd, buf, sizeof(buf));
	if (len == 0)
		/* No upcall to read; not necessarily a problem: */
		return;
	if (len < 0) {
		xlog_warn("nfsdcb: read(%s) failed: errno %d (%s)",
			     ic->ic_path, errno,
			     strerror(errno));
		nfsdreopen_one(ic);
		return;
	}

	/* Get rid of newline and terminate buffer*/
	buf[len - 1] = '\0';
	bp = (char *)buf;

	memset(&im, 0, sizeof(im));

	/* Authentication name -- ignored for now*/
	if (getfield(&bp, authbuf, sizeof(authbuf)) == -1) {
		xlog_warn("nfsdcb: bad authentication name in upcall\n");
		return;
	}
	if (getfield(&bp, typebuf, sizeof(typebuf)) == -1) {
		xlog_warn("nfsdcb: bad type in upcall\n");
		return;
	}
	if (verbose > 2)
		xlog_warn("nfsdcb: authbuf=%s authtype=%s",
			     authbuf, typebuf);

	im.im_type = strcmp(typebuf, "user") == 0 ?
		IDMAP_TYPE_USER : IDMAP_TYPE_GROUP;

	switch (ic->ic_which) {
	case IC_NAMEID:
		im.im_conv = IDMAP_CONV_NAMETOID;
		if (getfield(&bp, im.im_name, sizeof(im.im_name)) == -1) {
			xlog_warn("nfsdcb: bad name in upcall\n");
			return;
		}
		break;
	case IC_IDNAME:
		im.im_conv = IDMAP_CONV_IDTONAME;
		if (getfield(&bp, buf1, sizeof(buf1)) == -1) {
			xlog_warn("nfsdcb: bad id in upcall\n");
			return;
		}
		tmp = strtoul(buf1, (char **)NULL, 10);
		im.im_id = (u_int32_t)tmp;
		if ((tmp == ULONG_MAX && errno == ERANGE)
				|| (unsigned long)im.im_id != tmp) {
			xlog_warn("nfsdcb: id '%s' too big!\n", buf1);
			return;
		}
		break;
	default:
		xlog_warn("nfsdcb: Unknown which type %d", ic->ic_which);
		return;
	}

	imconv(ic, &im);

	buf[0] = '\0';
	bp = (char *)buf;
	bsiz = sizeof(buf);

	/* Authentication name */
	addfield(&bp, &bsiz, authbuf);

	switch (ic->ic_which) {
	case IC_NAMEID:
		/* Type */
		p = im.im_type == IDMAP_TYPE_USER ? "user" : "group";
		addfield(&bp, &bsiz, p);
		/* Name */
		addfield(&bp, &bsiz, im.im_name);
		/* expiry */
		snprintf(buf1, sizeof(buf1), "%" PRId64,
			 (int64_t)time(NULL) + cache_entry_expiration);
		addfield(&bp, &bsiz, buf1);
		/* Note that we don't want to write the id if the mapping
		 * failed; instead, by leaving it off, we write a negative
		 * cache entry which will result in an error returned to
		 * the client.  We don't want a chown or setacl referring
		 * to an unknown user to result in giving permissions to
		 * "nobody"! */
		if (im.im_status == IDMAP_STATUS_SUCCESS) {
			/* ID */
			snprintf(buf1, sizeof(buf1), "%u", im.im_id);
			addfield(&bp, &bsiz, buf1);

		}
		//if (bsiz == sizeof(buf)) /* XXX */

		bp[-1] = '\n';

		break;
	case IC_IDNAME:
		/* Type */
		p = im.im_type == IDMAP_TYPE_USER ? "user" : "group";
		addfield(&bp, &bsiz, p);
		/* ID */
		snprintf(buf1, sizeof(buf1), "%u", im.im_id);
		addfield(&bp, &bsiz, buf1);
		/* expiry */
		snprintf(buf1, sizeof(buf1), "%" PRId64,
			 (int64_t)time(NULL) + cache_entry_expiration);
		addfield(&bp, &bsiz, buf1);
		/* Note we're ignoring the status field in this case; we'll
		 * just map to nobody instead. */
		/* Name */
		addfield(&bp, &bsiz, im.im_name);

		bp[-1] = '\n';

		break;
	default:
		xlog_warn("nfsdcb: Unknown which type %d", ic->ic_which);
		return;
	}

	bsiz = sizeof(buf) - bsiz;

	if (atomicio((void*)write, ic->ic_fd, buf, bsiz) != bsiz)
		xlog_warn("nfsdcb: write(%s) failed: errno %d (%s)",
			     ic->ic_path, errno, strerror(errno));
}

static void
imconv(struct idmap_client *ic, struct idmap_msg *im)
{
	u_int32_t len;

	switch (im->im_conv) {
	case IDMAP_CONV_IDTONAME:
		idtonameres(im);
		if (verbose > 1)
			xlog_warn("%s %s: (%s) id \"%d\" -> name \"%s\"",
			    ic->ic_id, ic->ic_clid,
			    im->im_type == IDMAP_TYPE_USER ? "user" : "group",
			    im->im_id, im->im_name);
		break;
	case IDMAP_CONV_NAMETOID:
		len = strnlen(im->im_name, IDMAP_NAMESZ - 1);
		/* Check for NULL termination just to be careful */
		if (im->im_name[len+1] != '\0')
			return;
		nametoidres(im);
		if (verbose > 1)
			xlog_warn("%s %s: (%s) name \"%s\" -> id \"%d\"",
			    ic->ic_id, ic->ic_clid,
			    im->im_type == IDMAP_TYPE_USER ? "user" : "group",
			    im->im_name, im->im_id);
		break;
	default:
		xlog_warn("imconv: Invalid conversion type (%d) in message",
			     im->im_conv);
		im->im_status |= IDMAP_STATUS_INVALIDMSG;
		break;
	}
}

static void
nfscb(int UNUSED(fd), short which, void *data)
{
	struct idmap_client *ic = data;
	struct idmap_msg im;

	if (which != EV_READ)
		return;

	if (atomicio(read, ic->ic_fd, &im, sizeof(im)) != sizeof(im)) {
		if (verbose > 0)
			xlog_warn("nfscb: read(%s): %s", ic->ic_path, strerror(errno));
		return;
	}

	imconv(ic, &im);

	/* XXX: I don't like ignoring this error in the id->name case,
	 * but we've never returned it, and I need to check that the client
	 * can handle it gracefully before starting to return it now. */

	if (im.im_status == IDMAP_STATUS_LOOKUPFAIL)
		im.im_status = IDMAP_STATUS_SUCCESS;

	if (atomicio((void*)write, ic->ic_fd, &im, sizeof(im)) != sizeof(im))
		xlog_warn("nfscb: write(%s): %s", ic->ic_path, strerror(errno));
}

static void
nfsdclose_one(struct idmap_client *ic)
{
	if (ic->ic_event) {
		event_free(ic->ic_event);
		ic->ic_event = NULL;
	}
	if (ic->ic_fd != -1) {
		close(ic->ic_fd);
		ic->ic_fd = -1;
	}
}

static void
nfsdreopen_one(struct idmap_client *ic)
{
	int fd;

	if (verbose > 2)
		xlog_warn("ReOpening %s", ic->ic_path);

	if ((fd = open(ic->ic_path, O_RDWR, 0)) != -1) {
		nfsdclose_one(ic);

		ic->ic_fd = fd;
		ic->ic_event = event_new(evbase, ic->ic_fd, EV_READ | EV_PERSIST, nfsdcb, ic);
		if (ic->ic_event == NULL) {
			xlog_warn("nfsdreopen: Failed to create event for '%s'",
				  ic->ic_path);
			close(ic->ic_fd);
			ic->ic_fd = -1;
			return;
		}
		event_add(ic->ic_event, NULL);
	} else {
		xlog_warn("nfsdreopen: Opening '%s' failed: errno %d (%s)",
			ic->ic_path, errno, strerror(errno));
	}
}

static void
nfsdreopen(void)
{
	nfsdreopen_one(&nfsd_ic[IC_NAMEID]);
	nfsdreopen_one(&nfsd_ic[IC_IDNAME]);
	return;
}

static int
nfsdopen(void)
{
	return ((nfsdopenone(&nfsd_ic[IC_NAMEID]) == 0 &&
		    nfsdopenone(&nfsd_ic[IC_IDNAME]) == 0) ? 0 : -1);
}

static void
nfsdclose(void)
{
	nfsdclose_one(&nfsd_ic[IC_NAMEID]);
	nfsdclose_one(&nfsd_ic[IC_IDNAME]);
}

static int
nfsdopenone(struct idmap_client *ic)
{
	if ((ic->ic_fd = open(ic->ic_path, O_RDWR, 0)) == -1) {
		if (verbose > 0)
			xlog_warn("nfsdopenone: Opening %s failed: "
				"errno %d (%s)",
				ic->ic_path, errno, strerror(errno));
		return (-1);
	}

	ic->ic_event = event_new(evbase, ic->ic_fd, EV_READ | EV_PERSIST, nfsdcb, ic);
	if (ic->ic_event == NULL) {
		if (verbose > 0)
			xlog_warn("nfsdopenone: Create event for %s failed",
				  ic->ic_path);
		close(ic->ic_fd);
		ic->ic_fd = -1;
		return (-1);
	}
	event_add(ic->ic_event, NULL);

	if (verbose > 2)
		xlog_warn("Opened %s", ic->ic_path);

	return (0);
}

static int
nfsopen(struct idmap_client *ic)
{
	if ((ic->ic_fd = open(ic->ic_path, O_RDWR, 0)) == -1) {
		if (errno == ENOENT) {
			char *slash;

			slash = strrchr(ic->ic_path, '/');
			if (!slash)
				return -1;
			*slash = 0;
			inotify_add_watch(inotify_fd, ic->ic_path, IN_CREATE | IN_ONLYDIR | IN_ONESHOT);
			*slash = '/';
			if (verbose > 2)
				xlog_warn("Path %s not available. waiting...", ic->ic_path);
			return -1;
		}

		xlog_warn("nfsopen: open(%s): %s", ic->ic_path, strerror(errno));
		return (-1);
	}

	ic->ic_event = event_new(evbase, ic->ic_fd, EV_READ | EV_PERSIST, nfscb, ic);
	if (ic->ic_event == NULL) {
		xlog_warn("nfsopen: Create event for %s failed", ic->ic_path);
		close(ic->ic_fd);
		ic->ic_fd = -1;
		return -1;
	}
	event_add(ic->ic_event, NULL);
	if (verbose > 2)
		xlog_warn("Opened %s", ic->ic_path);

	return (0);
}

static void
idtonameres(struct idmap_msg *im)
{
	char domain[NFS4_MAX_DOMAIN_LEN];
	int ret = 0;

	ret = nfs4_get_default_domain(NULL, domain, sizeof(domain));
	switch (im->im_type) {
	case IDMAP_TYPE_USER:
		ret = nfs4_uid_to_name(im->im_id, domain, im->im_name,
				sizeof(im->im_name));
		if (ret) {
			if (strlen(nobodyuser) < sizeof(im->im_name))
				strcpy(im->im_name, nobodyuser);
			else
				strcpy(im->im_name, NFS4NOBODY_USER);
		}
		break;
	case IDMAP_TYPE_GROUP:
		ret = nfs4_gid_to_name(im->im_id, domain, im->im_name,
				sizeof(im->im_name));
		if (ret) {
			if (strlen(nobodygroup) < sizeof(im->im_name))
				strcpy(im->im_name, nobodygroup);
			else
				strcpy(im->im_name, NFS4NOBODY_GROUP);
		}
		break;
	}
	if (ret)
		im->im_status = IDMAP_STATUS_LOOKUPFAIL;
	else
		im->im_status = IDMAP_STATUS_SUCCESS;
}

static void
nametoidres(struct idmap_msg *im)
{
	uid_t uid;
	gid_t gid;
	int ret = 0;

	/* XXX: move nobody stuff to library calls
	 * (nfs4_get_nobody_user(domain), nfs4_get_nobody_group(domain)) */

	im->im_status = IDMAP_STATUS_SUCCESS;

	switch (im->im_type) {
	case IDMAP_TYPE_USER:
		ret = nfs4_name_to_uid(im->im_name, &uid);
		im->im_id = (u_int32_t) uid;
		if (ret) {
			im->im_status = IDMAP_STATUS_LOOKUPFAIL;
			im->im_id = nobodyuid;
		}
		return;
	case IDMAP_TYPE_GROUP:
		ret = nfs4_name_to_gid(im->im_name, &gid);
		im->im_id = (u_int32_t) gid;
		if (ret) {
			im->im_status = IDMAP_STATUS_LOOKUPFAIL;
			im->im_id = nobodygid;
		}
		return;
	}
}

static int
addfield(char **bpp, ssize_t *bsizp, char *fld)
{
	char ch, *bp = *bpp;
	ssize_t bsiz = *bsizp;

	while ((ch = *fld++) != '\0' && bsiz > 0) {
		switch(ch) {
		case ' ':
		case '\t':
		case '\n':
		case '\\':
			if (bsiz >= 4) {
				bp += snprintf(bp, bsiz, "\\%03o", ch);
				bsiz -= 4;
			}
			break;
		default:
			*bp++ = ch;
			bsiz--;
			break;
		}
	}

	if (bsiz < 1 || ch != '\0')
		return (-1);

	*bp++ = ' ';
	bsiz--;

	*bpp = bp;
	*bsizp = bsiz;

	return (0);
}

static int
getfield(char **bpp, char *fld, size_t fldsz)
{
	char *bp;
	unsigned int val; 
	int n;

	while ((bp = strsep(bpp, " ")) != NULL && bp[0] == '\0')
		;

	if (bp == NULL || bp[0] == '\0' || bp[0] == '\n')
		return (-1);

	while (*bp != '\0' && fldsz > 1) {
		if (*bp == '\\') {
			if ((n = sscanf(bp, "\\%03o", &val)) != 1)
				return (-1);
			if (val > UCHAR_MAX)
				return (-1);
			*fld++ = val;
			bp += 4;
		} else {
			*fld++ = *bp;
			bp++;
		}
		fldsz--;
	}

	if (*bp != '\0')
		return (-1);
	*fld = '\0';

	return (0);
}
