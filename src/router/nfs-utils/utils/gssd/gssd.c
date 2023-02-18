/*
  gssd.c

  Copyright (c) 2000, 2004 The Regents of the University of Michigan.
  All rights reserved.

  Copyright (c) 2000 Dug Song <dugsong@UMICH.EDU>.
  Copyright (c) 2002 Andy Adamson <andros@UMICH.EDU>.
  Copyright (c) 2002 Marius Aamodt Eriksen <marius@UMICH.EDU>.
  All rights reserved, all wrongs reversed.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  3. Neither the name of the University nor the names of its
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif	/* HAVE_CONFIG_H */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/inotify.h>
#include <rpc/rpc.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <memory.h>
#include <fcntl.h>
#include <dirent.h>
#include <netdb.h>
#include <event2/event.h>

#include "gssd.h"
#include "err_util.h"
#include "gss_util.h"
#include "krb5_util.h"
#include "nfslib.h"
#include "conffile.h"

static char *pipefs_path = GSSD_PIPEFS_DIR;
static DIR *pipefs_dir;
static int pipefs_fd;
static int inotify_fd;
struct event *inotify_ev;

char *keytabfile = GSSD_DEFAULT_KEYTAB_FILE;
char **ccachesearch;
int  use_memcache = 0;
int  root_uses_machine_creds = 1;
unsigned int  context_timeout = 0;
unsigned int  rpc_timeout = 5;
char *preferred_realm = NULL;
char *ccachedir = NULL;
/* set $HOME to "/" by default */
static bool set_home = true;
/* Avoid DNS reverse lookups on server names */
static bool avoid_dns = true;
static bool use_gssproxy = false;
pthread_mutex_t clp_lock = PTHREAD_MUTEX_INITIALIZER;
static bool signal_received = false;
static struct event_base *evbase = NULL;

int upcall_timeout = DEF_UPCALL_TIMEOUT;
static bool cancel_timed_out_upcalls = false;

TAILQ_HEAD(topdir_list_head, topdir) topdir_list;

/*
 * active_thread_list:
 *
 * 	used to track upcalls for timeout purposes.
 *
 * 	protected by the active_thread_list_lock mutex.
 *
 * 	upcall_thread_info structures are added to the tail of the list
 * 	by start_upcall_thread(), so entries closer to the head of the list
 * 	will be closer to hitting the upcall timeout.
 *
 * 	upcall_thread_info structures are removed from the list upon a
 * 	sucessful join of the upcall thread by the watchdog thread (via
 * 	scan_active_thread_list().
 */
TAILQ_HEAD(active_thread_list_head, upcall_thread_info) active_thread_list;
pthread_mutex_t active_thread_list_lock = PTHREAD_MUTEX_INITIALIZER;

struct topdir {
	TAILQ_ENTRY(topdir) list;
	TAILQ_HEAD(clnt_list_head, clnt_info) clnt_list;
	int wd;
	char name[];
};

/*
 * topdir_list:
 *	linked list of struct topdir with basic data about a topdir.
 *
 * clnt_list:
 *      linked list of struct clnt_info with basic data about a clntXXX dir,
 *      one per topdir.
 *
 * Directory structure: created by the kernel
 *      {rpc_pipefs}/{topdir}/clntXX      : one per rpc_clnt struct in the kernel
 *      {rpc_pipefs}/{topdir}/clntXX/krb5 : read uid for which kernel wants
 *					    a context, write the resulting context
 *      {rpc_pipefs}/{topdir}/clntXX/info : stores info such as server name
 *      {rpc_pipefs}/{topdir}/clntXX/gssd : pipe for all gss mechanisms using
 *					    a text-based string of parameters
 *
 * Algorithm:
 *      Poll all {rpc_pipefs}/{topdir}/clntXX/YYYY files.  When data is ready,
 *      read and process; performs rpcsec_gss context initialization protocol to
 *      get a cred for that user.  Writes result to corresponding krb5 file
 *      in a form the kernel code will understand.
 *      In addition, we make sure we are notified whenever anything is
 *      created or destroyed in {rpc_pipefs} or in any of the clntXX directories,
 *      and rescan the whole {rpc_pipefs} when this happens.
 */

/*
 * convert a presentation address string to a sockaddr_storage struct. Returns
 * true on success or false on failure.
 *
 * Note that we do not populate the sin6_scope_id field here for IPv6 addrs.
 * gssd nececessarily relies on hostname resolution and DNS AAAA records
 * do not generally contain scope-id's. This means that GSSAPI auth really
 * can't work with IPv6 link-local addresses.
 *
 * We *could* consider changing this if we did something like adopt the
 * Microsoft "standard" of using the ipv6-literal.net domainname, but it's
 * not really feasible at present.
 */
static bool
gssd_addrstr_to_sockaddr(struct sockaddr *sa, const char *node, const char *port)
{
	int rc;
	struct addrinfo *res;
	struct addrinfo hints = { .ai_flags = AI_NUMERICHOST | AI_NUMERICSERV };

#ifndef IPV6_SUPPORTED
	hints.ai_family = AF_INET;
#endif /* IPV6_SUPPORTED */

	rc = getaddrinfo(node, port, &hints, &res);
	if (rc) {
		printerr(0, "ERROR: unable to convert %s|%s to sockaddr: %s\n",
			 node, port,
			 rc == EAI_SYSTEM ? strerror(errno) : gai_strerror(rc));
		return false;
	}

#ifdef IPV6_SUPPORTED
	/*
	 * getnameinfo ignores the scopeid. If the address turns out to have
	 * a non-zero scopeid, we can't use it -- the resolved host might be
	 * completely different from the one intended.
	 */
	if (res->ai_addr->sa_family == AF_INET6) {
		struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)res->ai_addr;
		if (sin6->sin6_scope_id) {
			printerr(0, "ERROR: address %s has non-zero "
				    "sin6_scope_id!\n", node);
			nfs_freeaddrinfo(res);
			return false;
		}
	}
#endif /* IPV6_SUPPORTED */

	memcpy(sa, res->ai_addr, res->ai_addrlen);
	nfs_freeaddrinfo(res);
	return true;
}

/*
 * convert a sockaddr to a hostname
 */
static char *
gssd_get_servername(const char *name, const struct sockaddr *sa, const char *addr)
{
	socklen_t		addrlen;
	int			err;
	char			hbuf[NI_MAXHOST];
	unsigned char		buf[sizeof(struct in6_addr)];

	while (avoid_dns) {
		/*
		 * Determine if this is a server name, or an IP address.
		 * If it is an IP address, do the DNS lookup otherwise
		 * skip the DNS lookup.
		 */
		if (strchr(name, '.') == NULL)
			break; /* local name */
		else if (inet_pton(AF_INET, name, buf) == 1)
			break; /* IPv4 address */
		else if (inet_pton(AF_INET6, name, buf) == 1)
			break; /* IPv6 addrss */

		return strdup(name);
	}

	switch (sa->sa_family) {
	case AF_INET:
		addrlen = sizeof(struct sockaddr_in);
		break;
#ifdef IPV6_SUPPORTED
	case AF_INET6:
		addrlen = sizeof(struct sockaddr_in6);
		break;
#endif /* IPV6_SUPPORTED */
	default:
		printerr(0, "ERROR: unrecognized addr family %d\n",
			 sa->sa_family);
		return NULL;
	}

	err = getnameinfo(sa, addrlen, hbuf, sizeof(hbuf), NULL, 0,
			  NI_NAMEREQD);
	if (err) {
		printerr(0, "ERROR: unable to resolve %s to hostname: %s\n",
			 addr, err == EAI_SYSTEM ? strerror(errno) :
						   gai_strerror(err));
		return NULL;
	}

	return strdup(hbuf);
}

static void
gssd_read_service_info(int dirfd, struct clnt_info *clp)
{
	int fd;
	FILE *info = NULL;
	int numfields;
	char *server = NULL;
	char *service = NULL;
	int program;
	int version;
	char *address = NULL;
	char *protoname = NULL;
	char *port = NULL;
	char *servername = NULL;

	fd = openat(dirfd, "info", O_RDONLY);
	if (fd < 0) {
		printerr(0, "ERROR: can't open %s/info: %s\n",
			 clp->relpath, strerror(errno));
		goto fail;
	}

	info = fdopen(fd, "r");
	if (!info) {
		printerr(0, "ERROR: can't fdopen %s/info: %s\n",
			 clp->relpath, strerror(errno));
		close(fd);
		goto fail;
	}

	/*
	 * Some history:
	 *
	 * The first three lines were added with rpc_pipefs in 2003-01-13.
	 * (commit af2f003391786fb632889c02142c941b212ba4ff)
	 * 
	 * The 'protocol' line was added in 2003-06-11.
	 * (commit 9bd741ae48785d0c0e75cf906ff66f893d600c2d)
	 *
	 * The 'port' line was added in 2007-09-26.
	 * (commit bf19aacecbeebccb2c3d150a8bd9416b7dba81fe)
	 */
	numfields = fscanf(info,
			   "RPC server: %ms\n"
			   "service: %ms (%d) version %d\n"
			   "address: %ms\n"
			   "protocol: %ms\n"
			   "port: %ms\n",
			   &server,
			   &service, &program, &version,
			   &address,
			   &protoname,
			   &port);


	switch (numfields) {
	case 5:
		protoname = strdup("tcp");
		if (!protoname)
			goto fail;
		/* fall through */
	case 6:
		/* fall through */
	case 7:
		break;
	default:
		goto fail;
	}

	/*
	 * The user space RPC library has no support for
	 * RPC-over-RDMA at this time, so change 'rdma'
	 * to 'tcp', and '20049' to '2049'.
	 */
	if (strcmp(protoname, "rdma") == 0) {
		free(protoname);
		protoname = strdup("tcp");
		if (!protoname)
			goto fail;
		free(port);
		port = strdup("2049");
		if (!port)
			goto fail;
	}

	if (!gssd_addrstr_to_sockaddr((struct sockaddr *)&clp->addr,
				 address, port ? port : ""))
		goto fail;

	servername = gssd_get_servername(server, (struct sockaddr *)&clp->addr, address);
	if (!servername)
		goto fail;

	if (asprintf(&clp->servicename, "%s@%s", service, servername) < 0)
		goto fail;

	clp->servername = servername;
	clp->prog = program;
	clp->vers = version;
	clp->protocol = protoname;

	goto out;

fail:
	printerr(0, "ERROR: failed to parse %s/info\n", clp->relpath);
	free(servername);
	free(protoname);
	clp->servicename = NULL;
	clp->servername = NULL;
	clp->prog = 0;
	clp->vers = 0;
	clp->protocol = NULL;
out:
	if (info)
		fclose(info);

	free(server);
	free(service);
	free(address);
	free(port);
}

/* Actually frees clp and fields that might be used from other
 * threads if was last reference.
 */
void
gssd_free_client(struct clnt_info *clp)
{
	int refcnt;

	pthread_mutex_lock(&clp_lock);
	refcnt = --clp->refcount;
	pthread_mutex_unlock(&clp_lock);
	if (refcnt > 0)
		return;

	printerr(4, "freeing client %s\n", clp->relpath);

	if (clp->krb5_fd >= 0)
		close(clp->krb5_fd);

	if (clp->gssd_fd >= 0)
		close(clp->gssd_fd);

	free(clp->relpath);
	free(clp->servicename);
	free(clp->servername);
	free(clp->protocol);
	free(clp);
}

/* Called when removing from clnt_list to tear down event handling.
 * Will then free clp if was last reference.
 */
static void
gssd_destroy_client(struct clnt_info *clp)
{
	printerr(4, "destroying client %s\n", clp->relpath);

	if (clp->krb5_ev) {
		event_del(clp->krb5_ev);
		event_free(clp->krb5_ev);
		clp->krb5_ev = NULL;
	}

	if (clp->gssd_ev) {
		event_del(clp->gssd_ev);
		event_free(clp->gssd_ev);
		clp->gssd_ev = NULL;
	}

	inotify_rm_watch(inotify_fd, clp->wd);
	gssd_free_client(clp);
}

static void gssd_scan(void);

/* For each upcall read the upcall info into the buffer, then create a
 * thread in a detached state so that resources are released back into
 * the system without the need for a join.
 */
static void
gssd_clnt_gssd_cb(int UNUSED(fd), short UNUSED(which), void *data)
{
	struct clnt_info *clp = data;

	handle_gssd_upcall(clp);
}

static void
gssd_clnt_krb5_cb(int UNUSED(fd), short UNUSED(which), void *data)
{
	struct clnt_info *clp = data;

	handle_krb5_upcall(clp);
}

/*
 * scan_active_thread_list:
 *
 * Walks the active_thread_list, trying to join as many upcall threads as
 * possible.  For threads that have terminated, the corresponding
 * upcall_thread_info will be removed from the list and freed.  Threads that
 * are still busy and have exceeded the upcall_timeout will cause an error to
 * be logged and may be canceled (depending on the value of
 * cancel_timed_out_upcalls).
 *
 * Returns the number of seconds that the watchdog thread should wait before
 * calling scan_active_thread_list() again.
 */
static int
scan_active_thread_list(void)
{
	struct upcall_thread_info *info;
	struct timespec now;
	unsigned int sleeptime;
	bool sleeptime_set = false;
	int err;
	void *tret, *saveprev;

	sleeptime = upcall_timeout;
	pthread_mutex_lock(&active_thread_list_lock);
	clock_gettime(CLOCK_MONOTONIC, &now);
	TAILQ_FOREACH(info, &active_thread_list, list) {
		err = pthread_tryjoin_np(info->tid, &tret);
		switch (err) {
		case 0:
			/*
			 * The upcall thread has either completed successfully, or
			 * has been canceled _and_ has acted on the cancellation request
			 * (i.e. has hit a cancellation point).  We can now remove the
			 * upcall_thread_info from the list and free it.
			 */
			if (tret == PTHREAD_CANCELED)
				printerr(2, "watchdog: thread id 0x%lx cancelled successfully\n",
						info->tid);
			saveprev = info->list.tqe_prev;
			TAILQ_REMOVE(&active_thread_list, info, list);
			free(info);
			info = saveprev;
			break;
		case EBUSY:
			/*
			 * The upcall thread is still running.  If the timeout has expired
			 * then we either cancel the thread, log an error, and do an error
			 * downcall to the kernel (cancel_timed_out_upcalls=true) or simply
			 * log an error (cancel_timed_out_upcalls=false).  In either case,
			 * the error is logged only once.
			 */
			if (now.tv_sec >= info->timeout.tv_sec) {
				if (cancel_timed_out_upcalls && !(info->flags & UPCALL_THREAD_CANCELED)) {
					printerr(0, "watchdog: thread id 0x%lx timed out\n",
							info->tid);
					pthread_cancel(info->tid);
					info->flags |= (UPCALL_THREAD_CANCELED|UPCALL_THREAD_WARNED);
					do_error_downcall(info->fd, info->uid, -ETIMEDOUT);
				} else {
					if (!(info->flags & UPCALL_THREAD_WARNED)) {
						printerr(0, "watchdog: thread id 0x%lx running for %ld seconds\n",
								info->tid,
								now.tv_sec - info->timeout.tv_sec + upcall_timeout);
						info->flags |= UPCALL_THREAD_WARNED;
					}
				}
			} else if (!sleeptime_set) {
			/*
			 * The upcall thread is still running, but the timeout has not yet
			 * expired.  Calculate the time remaining until the timeout will
			 * expire.  This is the amount of time the watchdog thread will
			 * wait before running again.  We only need to do this for the busy
			 * thread closest to the head of the list - entries appearing later
			 * in the list will time out later.
			 */
				sleeptime = info->timeout.tv_sec - now.tv_sec;
				sleeptime_set = true;
			}
			break;
		default:
			/* EDEADLK, EINVAL, and ESRCH... none of which should happen! */
			printerr(0, "watchdog: attempt to join thread id 0x%lx returned %d (%s)!\n",
					info->tid, err, strerror(err));
			break;
		}
	}
	pthread_mutex_unlock(&active_thread_list_lock);

	return sleeptime;
}

static void *
watchdog_thread_fn(void *UNUSED(arg))
{
	unsigned int sleeptime;

	for (;;) {
		sleeptime = scan_active_thread_list();
		printerr(4, "watchdog: sleeping %u secs\n", sleeptime);
		sleep(sleeptime);
	}
	return (void *)0;
}

static int
start_watchdog_thread(void)
{
	pthread_attr_t attr;
	pthread_t th;
	int ret;

	ret = pthread_attr_init(&attr);
	if (ret != 0) {
		printerr(0, "ERROR: failed to init pthread attr: ret %d: %s\n",
			 ret, strerror(errno));
		return ret;
	}
	ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (ret != 0) {
		printerr(0, "ERROR: failed to create pthread attr: ret %d: %s\n",
			 ret, strerror(errno));
		return ret;
	}
	ret = pthread_create(&th, &attr, watchdog_thread_fn, NULL);
	if (ret != 0) {
		printerr(0, "ERROR: pthread_create failed: ret %d: %s\n",
			 ret, strerror(errno));
	}
	return ret;
}

static struct clnt_info *
gssd_get_clnt(struct topdir *tdi, const char *name)
{
	struct clnt_info *clp;

	TAILQ_FOREACH(clp, &tdi->clnt_list, list)
		if (!strcmp(clp->name, name))
			return clp;

	printerr(4, "creating client %s/%s\n", tdi->name, name);

	clp = calloc(1, sizeof(struct clnt_info));
	if (!clp) {
		printerr(0, "ERROR: can't malloc clnt_info: %s\n",
			 strerror(errno));
		return NULL;
	}

	if (asprintf(&clp->relpath, "%s/%s", tdi->name, name) < 0) {
		clp->relpath = NULL;
		goto out;
	}

	clp->wd = inotify_add_watch(inotify_fd, clp->relpath, IN_CREATE | IN_DELETE);
	if (clp->wd < 0) {
		if (errno != ENOENT)
			printerr(0, "ERROR: %s: inotify_add_watch failed for %s: %s\n",
			 	__FUNCTION__, clp->relpath, strerror(errno));
		goto out;
	}

	clp->name = clp->relpath + strlen(tdi->name) + 1;
	clp->krb5_fd = -1;
	clp->gssd_fd = -1;
	clp->refcount = 1;

	TAILQ_INSERT_HEAD(&tdi->clnt_list, clp, list);
	return clp;

out:
	free(clp->relpath);
	free(clp);
	return NULL;
}

static int
gssd_scan_clnt(struct clnt_info *clp)
{
	int clntfd;

	printerr(4, "scanning client %s\n", clp->relpath);

	clntfd = openat(pipefs_fd, clp->relpath, O_RDONLY);
	if (clntfd < 0) {
		if (errno != ENOENT)
			printerr(0, "ERROR: %s: can't openat %s: %s\n",
			 	__FUNCTION__, clp->relpath, strerror(errno));
		return -1;
	}

	if (clp->gssd_fd == -1)
		clp->gssd_fd = openat(clntfd, "gssd", O_RDWR | O_NONBLOCK);

	if (clp->gssd_fd == -1 && clp->krb5_fd == -1)
		clp->krb5_fd = openat(clntfd, "krb5", O_RDWR | O_NONBLOCK);

	if (!clp->gssd_ev && clp->gssd_fd >= 0) {
		clp->gssd_ev = event_new(evbase, clp->gssd_fd, EV_READ | EV_PERSIST,
					 gssd_clnt_gssd_cb, clp);
		if (!clp->gssd_ev) {
			printerr(0, "ERROR: %s: can't create gssd event for %s: %s\n",
				 __FUNCTION__, clp->relpath, strerror(errno));
			close(clp->gssd_fd);
			clp->gssd_fd = -1;
		} else {
			event_add(clp->gssd_ev, NULL);
		}
	}

	if (!clp->krb5_ev && clp->krb5_fd >= 0) {
		clp->krb5_ev = event_new(evbase, clp->krb5_fd, EV_READ | EV_PERSIST,
					 gssd_clnt_krb5_cb, clp);
		if (!clp->krb5_ev) {
			printerr(0, "ERROR: %s: can't create krb5 event for %s: %s\n",
				 __FUNCTION__, clp->relpath, strerror(errno));
			close(clp->krb5_fd);
			clp->krb5_fd = -1;
		} else {
			event_add(clp->krb5_ev, NULL);
		}
	}

	if (clp->krb5_fd == -1 && clp->gssd_fd == -1)
		/* not fatal, files might appear later */
		goto out;

	if (clp->prog == 0)
		gssd_read_service_info(clntfd, clp);

out:
	close(clntfd);
	clp->scanned = true;
	return 0;
}

static int
gssd_create_clnt(struct topdir *tdi, const char *name)
{
	struct clnt_info *clp;

	clp = gssd_get_clnt(tdi, name);
	if (!clp)
		return -1;

	return gssd_scan_clnt(clp);
}

static struct topdir *
gssd_get_topdir(const char *name)
{
	struct topdir *tdi;

	TAILQ_FOREACH(tdi, &topdir_list, list)
		if (!strcmp(tdi->name, name))
			return tdi;

	tdi = malloc(sizeof(*tdi) + strlen(name) + 1);
	if (!tdi) {
		printerr(0, "ERROR: Couldn't allocate struct topdir\n");
		return NULL;
	}

	tdi->wd = inotify_add_watch(inotify_fd, name, IN_CREATE);
	if (tdi->wd < 0) {
		printerr(0, "ERROR: %s: inotify_add_watch failed for top dir %s: %s\n",
			 __FUNCTION__, tdi->name, strerror(errno));
		free(tdi);
		return NULL;
	}

	strcpy(tdi->name, name);
	TAILQ_INIT(&tdi->clnt_list);

	TAILQ_INSERT_HEAD(&topdir_list, tdi, list);
	return tdi;
}

static void
gssd_scan_topdir(const char *name)
{
	struct topdir *tdi;
	int dfd;
	DIR *dir;
	struct clnt_info *clp;
	struct dirent *d;

	tdi = gssd_get_topdir(name);
	if (!tdi)
		return;

	dfd = openat(pipefs_fd, tdi->name, O_RDONLY);
	if (dfd < 0) {
		if (errno != ENOENT)
			printerr(0, "ERROR: %s: can't openat %s: %s\n",
			 	__FUNCTION__, tdi->name, strerror(errno));
		return;
	}

	dir = fdopendir(dfd);
	if (!dir) {
		printerr(0, "ERROR: can't fdopendir %s: %s\n",
			 tdi->name, strerror(errno));
		return;
	}

	TAILQ_FOREACH(clp, &tdi->clnt_list, list)
		clp->scanned = false;

	while ((d = readdir(dir))) {
		if (d->d_type != DT_DIR)
			continue;

		if (strncmp(d->d_name, "clnt", strlen("clnt")))
			continue;

		gssd_create_clnt(tdi, d->d_name);
	}

	closedir(dir);

	TAILQ_FOREACH(clp, &tdi->clnt_list, list) {
		void *saveprev;

		if (clp->scanned)
			continue;

		printerr(3, "orphaned client %s\n", clp->relpath);
		saveprev = clp->list.tqe_prev;
		TAILQ_REMOVE(&tdi->clnt_list, clp, list);
		gssd_destroy_client(clp);
		clp = saveprev;
	}
}

static void
gssd_scan(void)
{
	struct dirent *d;

	printerr(4, "doing a full rescan\n");
	rewinddir(pipefs_dir);

	while ((d = readdir(pipefs_dir))) {
		if (d->d_type != DT_DIR)
			continue;

		if (d->d_name[0] == '.')
			continue;

		gssd_scan_topdir(d->d_name);
	}

	if (TAILQ_EMPTY(&topdir_list)) {
		printerr(0, "ERROR: the rpc_pipefs directory is empty!\n");
		exit(EXIT_FAILURE);
	}
}

static void
gssd_scan_cb(int UNUSED(fd), short UNUSED(which), void *UNUSED(data))
{
	gssd_scan();
}

static bool
gssd_inotify_topdir(struct topdir *tdi, const struct inotify_event *ev)
{
	printerr(5, "inotify event for topdir (%s) - "
		 "ev->wd (%d) ev->name (%s) ev->mask (0x%08x)\n",
		 tdi->name, ev->wd, ev->len > 0 ? ev->name : "<?>", ev->mask);

	if (ev->mask & IN_IGNORED) {
		printerr(0, "ERROR: topdir disappeared!\n");
		return false;
	}

	if (ev->len == 0)
		return false;

	if (ev->mask & IN_CREATE) {
		if (!(ev->mask & IN_ISDIR))
			return true;

		if (strncmp(ev->name, "clnt", strlen("clnt")))
			return true;

		if (gssd_create_clnt(tdi, ev->name))
			return false;

		return true;
	} 

	return false;
}

static bool
gssd_inotify_clnt(struct topdir *tdi, struct clnt_info *clp, const struct inotify_event *ev)
{
	printerr(5, "inotify event for clntdir (%s) - "
		 "ev->wd (%d) ev->name (%s) ev->mask (0x%08x)\n",
		 clp->relpath, ev->wd, ev->len > 0 ? ev->name : "<?>", ev->mask);

	if (ev->mask & IN_IGNORED) {
		TAILQ_REMOVE(&tdi->clnt_list, clp, list);
		gssd_destroy_client(clp);
		return true;
	}

	if (ev->len == 0)
		return false;

	if (ev->mask & IN_CREATE) {
		if (!strcmp(ev->name, "gssd") ||
		    !strcmp(ev->name, "krb5") ||
		    !strcmp(ev->name, "info"))
			if (gssd_scan_clnt(clp))
				return false;

		return true;

	} else if (ev->mask & IN_DELETE) {
		if (!strcmp(ev->name, "gssd") && clp->gssd_fd >= 0) {
			close(clp->gssd_fd);
			event_del(clp->gssd_ev);
			event_free(clp->gssd_ev);
			clp->gssd_ev = NULL;
			clp->gssd_fd = -1;

		} else if (!strcmp(ev->name, "krb5") && clp->krb5_fd >= 0) {
			close(clp->krb5_fd);
			event_del(clp->krb5_ev);
			event_free(clp->krb5_ev);
			clp->krb5_ev = NULL;
			clp->krb5_fd = -1;
		}

		return true;
	}

	return false;
}

static void
gssd_inotify_cb(int ifd, short UNUSED(which), void *UNUSED(data))
{
	bool rescan = false;
	struct topdir *tdi;
	struct clnt_info *clp;

	while (true) {
		char buf[4096] __attribute__ ((aligned(__alignof__(struct inotify_event))));
		const struct inotify_event *ev;
		ssize_t len;
		char *ptr;

		len = read(ifd, buf, sizeof(buf));
		if (len == -1 && errno == EINTR)
			continue;

		if (len <= 0)
			break;

		for (ptr = buf; ptr < buf + len;
		     ptr += sizeof(struct inotify_event) + ev->len) {
			ev = (const struct inotify_event *)ptr;

			if (ev->mask & IN_Q_OVERFLOW) {
				printerr(0, "ERROR: inotify queue overflow\n");
				rescan = true;
				break;
			}

			TAILQ_FOREACH(tdi, &topdir_list, list) {
				if (tdi->wd == ev->wd) {
					if (!gssd_inotify_topdir(tdi, ev))
						rescan = true;
					goto found;
				}

				TAILQ_FOREACH(clp, &tdi->clnt_list, list) {
					if (clp->wd == ev->wd) {
						if (!gssd_inotify_clnt(tdi, clp, ev))
							rescan = true;
						goto found;
					}
				}
			}

found:
			if (!tdi) {
				printerr(5, "inotify event for unknown wd!!! - "
					 "ev->wd (%d) ev->name (%s) ev->mask (0x%08x)\n",
					 ev->wd, ev->len > 0 ? ev->name : "<?>", ev->mask);
				rescan = true;
			}
		}
	}

	if (rescan)
		gssd_scan();
}

static void
sig_die(int signal)
{
	if (signal_received) {
		gssd_destroy_krb5_principals(root_uses_machine_creds);
		printerr(1, "forced exiting on signal %d\n", signal);
		exit(0);
	}

	signal_received = true;
	printerr(1, "exiting on signal %d\n", signal);
	event_base_loopexit(evbase, NULL);
}

static void
usage(char *progname)
{
	fprintf(stderr, "usage: %s [-f] [-l] [-M] [-n] [-v] [-r] [-p pipefsdir] [-k keytab] [-d ccachedir] [-t timeout] [-R preferred realm] [-D] [-H] [-U upcall timeout] [-C]\n",
		progname);
	exit(1);
}

inline static void 
read_gss_conf(void)
{
	char *s;

	conf_init_file(NFS_CONFFILE);
	use_memcache = conf_get_bool("gssd", "use-memcache", use_memcache);
	root_uses_machine_creds = conf_get_bool("gssd", "use-machine-creds",
						root_uses_machine_creds);
	avoid_dns = conf_get_bool("gssd", "avoid-dns", avoid_dns);
#ifdef HAVE_SET_ALLOWABLE_ENCTYPES
	limit_to_legacy_enctypes = conf_get_bool("gssd", "limit-to-legacy-enctypes",
						 limit_to_legacy_enctypes);
#endif
	context_timeout = conf_get_num("gssd", "context-timeout", context_timeout);
	rpc_timeout = conf_get_num("gssd", "rpc-timeout", rpc_timeout);
	upcall_timeout = conf_get_num("gssd", "upcall-timeout", upcall_timeout);
	cancel_timed_out_upcalls = conf_get_bool("gssd", "cancel-timed-out-upcalls",
						cancel_timed_out_upcalls);
	s = conf_get_str("gssd", "pipefs-directory");
	if (!s)
		s = conf_get_str("general", "pipefs-directory");
	else
		printerr(0, "WARNING: Specifying pipefs-directory in the [gssd] "
			 "section of %s is deprecated.  Use the [general] "
			 "section instead.", NFS_CONFFILE);
	if (s)
		pipefs_path = s;
	s = conf_get_str("gssd", "keytab-file");
	if (s)
		keytabfile = s;
	s = conf_get_str("gssd", "cred-cache-directory");
	if (s)
		ccachedir = strdup(s);
	s = conf_get_str("gssd", "preferred-realm");
	if (s)
		preferred_realm = s;

	use_gssproxy = conf_get_bool("gssd", "use-gss-proxy", use_gssproxy);
	set_home = conf_get_bool("gssd", "set-home", set_home);
}

int
main(int argc, char *argv[])
{
	int fg = 0;
	int verbosity = 0;
	int rpc_verbosity = 0;
	int opt;
	int i;
	int rc;
	extern char *optarg;
	char *progname;
	struct event *sighup_ev;

	read_gss_conf();

	verbosity = conf_get_num("gssd", "verbosity", verbosity);
	rpc_verbosity = conf_get_num("gssd", "rpc-verbosity", rpc_verbosity);

	while ((opt = getopt(argc, argv, "HDfvrlmnMp:k:d:t:T:R:U:C")) != -1) {
		switch (opt) {
			case 'f':
				fg = 1;
				break;
			case 'm':
				/* Accept but ignore this. Now the default. */
				break;
			case 'M':
				use_memcache = 1;
				break;
			case 'n':
				root_uses_machine_creds = 0;
				break;
			case 'v':
				verbosity++;
				break;
			case 'r':
				rpc_verbosity++;
				break;
			case 'p':
				pipefs_path = optarg;
				break;
			case 'k':
				keytabfile = optarg;
				break;
			case 'd':
				free(ccachedir);
				ccachedir = strdup(optarg);
				break;
			case 't':
				context_timeout = atoi(optarg);
				break;
			case 'T':
				rpc_timeout = atoi(optarg);
				break;
			case 'R':
				preferred_realm = strdup(optarg);
				break;
			case 'l':
#ifdef HAVE_SET_ALLOWABLE_ENCTYPES
				limit_to_legacy_enctypes = 1;
#else 
				errx(1, "Encryption type limits not supported by Kerberos libraries.");
#endif
				break;
			case 'D':
				avoid_dns = false;
				break;
			case 'H':
				set_home = false;
				break;
			case 'U':
				upcall_timeout = atoi(optarg);
				break;
			case 'C':
				cancel_timed_out_upcalls = true;
				break;
			default:
				usage(argv[0]);
				break;
		}
	}

	/*
	 * Some krb5 routines try to scrape info out of files in the user's
	 * home directory. This can easily deadlock when that homedir is on a
	 * kerberized NFS mount. By setting $HOME to "/" by default, we prevent
	 * this behavior in routines that use $HOME in preference to the results
	 * of getpw*.
	 *
	 * Some users do not use Kerberized home dirs and need $HOME to remain
	 * unchanged. Those users can leave $HOME unchanged by setting set_home
	 * to false.
	 */
	if (set_home) {
		if (setenv("HOME", "/", 1)) {
			printerr(0, "gssd: Unable to set $HOME: %s\n", strerror(errno));
			exit(1);
		}
	}

	if (use_gssproxy) {
		if (setenv("GSS_USE_PROXY", "yes", 1) < 0) {
			printerr(0, "gssd: Unable to set $GSS_USE_PROXY: %s\n", 
				strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	if (ccachedir) {
		char *ptr;

		for (ptr = ccachedir, i = 2; *ptr; ptr++)
			if (*ptr == ':')
				i++;

		ccachesearch = malloc(i * sizeof(char *));
		if (!ccachesearch) {
			printerr(0, "malloc failure\n");
			exit(EXIT_FAILURE);
		}

		i = 0;
		ccachesearch[i++] = strtok(ccachedir, ":");
		while(ccachesearch[i - 1])
			ccachesearch[i++] = strtok(NULL, ":");

	} else {
		ccachesearch = malloc(3 * sizeof(char *));
		if (!ccachesearch) {
			printerr(0, "malloc failure\n");
			exit(EXIT_FAILURE);
		}

		ccachesearch[0] = GSSD_DEFAULT_CRED_DIR;
		ccachesearch[1] = GSSD_USER_CRED_DIR;
		ccachesearch[2] = NULL;
	}

	if (preferred_realm == NULL)
		gssd_k5_get_default_realm(&preferred_realm);

	if ((progname = strrchr(argv[0], '/')))
		progname++;
	else
		progname = argv[0];

	if (upcall_timeout > MAX_UPCALL_TIMEOUT)
		upcall_timeout = MAX_UPCALL_TIMEOUT;
	else if (upcall_timeout < MIN_UPCALL_TIMEOUT)
		upcall_timeout = MIN_UPCALL_TIMEOUT;

	initerr(progname, verbosity, fg);
#ifdef HAVE_LIBTIRPC_SET_DEBUG
	/*
	 * Only set the libtirpc debug level if explicitly requested via -r.
	 */
	if (rpc_verbosity > 0)
		libtirpc_set_debug(progname, rpc_verbosity, fg);
#else
	if (rpc_verbosity > 0)
		printerr(0, "Warning: libtirpc does not "
			    "support setting debug levels\n");
#endif

	daemon_init(fg);

	if (gssd_check_mechs() != 0)
		errx(1, "Problem with gssapi library");

	evbase = event_base_new();
	if (!evbase) {
		printerr(0, "ERROR: failed to create event base: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	pipefs_dir = opendir(pipefs_path);
	if (!pipefs_dir) {
		printerr(0, "ERROR: opendir(%s) failed: %s\n", pipefs_path, strerror(errno));
		exit(EXIT_FAILURE);
	}

	pipefs_fd = dirfd(pipefs_dir);
	if (fchdir(pipefs_fd)) {
		printerr(0, "ERROR: fchdir(%s) failed: %s\n", pipefs_path, strerror(errno));
		exit(EXIT_FAILURE);
	}

	inotify_fd = inotify_init1(IN_NONBLOCK);
	if (inotify_fd == -1) {
		printerr(0, "ERROR: inotify_init1 failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	signal(SIGINT, sig_die);
	signal(SIGTERM, sig_die);
	sighup_ev = evsignal_new(evbase, SIGHUP, gssd_scan_cb, NULL);
	if (!sighup_ev) {
		printerr(0, "ERROR: failed to create SIGHUP event: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	evsignal_add(sighup_ev, NULL);
	inotify_ev = event_new(evbase, inotify_fd, EV_READ | EV_PERSIST,
			       gssd_inotify_cb, NULL);
	if (!inotify_ev) {
		printerr(0, "ERROR: failed to create inotify event: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	event_add(inotify_ev, NULL);

	TAILQ_INIT(&active_thread_list);

	rc = start_watchdog_thread();
	if (rc != 0) {
		printerr(0, "ERROR: failed to start watchdog thread: %d\n", rc);
		exit(EXIT_FAILURE);
	}

	TAILQ_INIT(&topdir_list);
	gssd_scan();
	daemon_ready();

	rc = event_base_dispatch(evbase);

	printerr(0, "event_dispatch() returned %i!\n", rc);

	gssd_destroy_krb5_principals(root_uses_machine_creds);

	while (!TAILQ_EMPTY(&topdir_list)) {
		struct topdir *tdi = TAILQ_FIRST(&topdir_list);
		TAILQ_REMOVE(&topdir_list, tdi, list);
		while (!TAILQ_EMPTY(&tdi->clnt_list)) {
			struct clnt_info *clp = TAILQ_FIRST(&tdi->clnt_list);
			TAILQ_REMOVE(&tdi->clnt_list, clp, list);
			gssd_destroy_client(clp);
		}
		free(tdi);
	}

	event_free(inotify_ev);
	event_free(sighup_ev);
	event_base_free(evbase);

	close(inotify_fd);
	close(pipefs_fd);
	closedir(pipefs_dir);

	free(preferred_realm);
	free(ccachesearch);
	free(ccachedir);

	return rc < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
