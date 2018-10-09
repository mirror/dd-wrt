/*
 * mdmon - monitor external metadata arrays
 *
 * Copyright (C) 2007-2009 Neil Brown <neilb@suse.de>
 * Copyright (C) 2007-2009 Intel Corporation
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*
 * md array manager.
 * When md arrays have user-space managed metadata, this is the program
 * that does the managing.
 *
 * Given one argument: the name of the array (e.g. /dev/md0) that is
 * the container.
 * We fork off a helper that runs high priority and mlocked.  It responds to
 * device failures and other events that might stop writeout, or that are
 * trivial to deal with.
 * The main thread then watches for new arrays being created in the container
 * and starts monitoring them too ... along with a few other tasks.
 *
 * The main thread communicates with the priority thread by writing over
 * a pipe.
 * Separate programs can communicate with the main thread via Unix-domain
 * socket.
 * The two threads share address space and open file table.
 *
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include	<unistd.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<sys/un.h>
#include	<sys/mman.h>
#include	<sys/syscall.h>
#include	<sys/wait.h>
#include	<stdio.h>
#include	<errno.h>
#include	<string.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<dirent.h>
#ifdef USE_PTHREADS
#include	<pthread.h>
#else
#include	<sched.h>
#endif

#include	"mdadm.h"
#include	"mdmon.h"

char const Name[] = "mdmon";

struct active_array *discard_this;
struct active_array *pending_discard;

int mon_tid, mgr_tid;

int sigterm;

#ifdef USE_PTHREADS
static void *run_child(void *v)
{
	struct supertype *c = v;

	mon_tid = syscall(SYS_gettid);
	do_monitor(c);
	return 0;
}

static int clone_monitor(struct supertype *container)
{
	pthread_attr_t attr;
	pthread_t thread;
	int rc;

	mon_tid = -1;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 4096);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	rc = pthread_create(&thread, &attr, run_child, container);
	if (rc)
		return rc;
	while (mon_tid == -1)
		usleep(10);
	pthread_attr_destroy(&attr);

	mgr_tid = syscall(SYS_gettid);

	return mon_tid;
}
#else /* USE_PTHREADS */
static int run_child(void *v)
{
	struct supertype *c = v;

	do_monitor(c);
	return 0;
}

#ifdef __ia64__
int __clone2(int (*fn)(void *),
	    void *child_stack_base, size_t stack_size,
	    int flags, void *arg, ...
	 /* pid_t *pid, struct user_desc *tls, pid_t *ctid */ );
#endif
static int clone_monitor(struct supertype *container)
{
	static char stack[4096];

#ifdef __ia64__
	mon_tid = __clone2(run_child, stack, sizeof(stack),
		   CLONE_FS|CLONE_FILES|CLONE_VM|CLONE_SIGHAND|CLONE_THREAD,
		   container);
#else
	mon_tid = clone(run_child, stack+4096-64,
		   CLONE_FS|CLONE_FILES|CLONE_VM|CLONE_SIGHAND|CLONE_THREAD,
		   container);
#endif

	mgr_tid = syscall(SYS_gettid);

	return mon_tid;
}
#endif /* USE_PTHREADS */

static int make_pidfile(char *devname)
{
	char path[100];
	char pid[10];
	int fd;
	int n;

	if (mkdir(MDMON_DIR, 0755) < 0 &&
	    errno != EEXIST)
		return -errno;
	sprintf(path, "%s/%s.pid", MDMON_DIR, devname);

	fd = open(path, O_RDWR|O_CREAT|O_EXCL, 0600);
	if (fd < 0)
		return -errno;
	sprintf(pid, "%d\n", getpid());
	n = write(fd, pid, strlen(pid));
	close(fd);
	if (n < 0)
		return -errno;
	return 0;
}

static void try_kill_monitor(pid_t pid, char *devname, int sock)
{
	char buf[100];
	int fd;
	int n;
	long fl;

	/* first rule of survival... don't off yourself */
	if (pid == getpid())
		return;

	/* kill this process if it is mdmon */
	sprintf(buf, "/proc/%lu/cmdline", (unsigned long) pid);
	fd = open(buf, O_RDONLY);
	if (fd < 0)
		return;

	n = read(fd, buf, sizeof(buf)-1);
	buf[sizeof(buf)-1] = 0;
	close(fd);

	if (n < 0 || !(strstr(buf, "mdmon") ||
		       strstr(buf, "@dmon")))
		return;

	kill(pid, SIGTERM);

	if (sock < 0)
		return;

	/* Wait for monitor to exit by reading from the socket, after
	 * clearing the non-blocking flag */
	fl = fcntl(sock, F_GETFL, 0);
	fl &= ~O_NONBLOCK;
	fcntl(sock, F_SETFL, fl);
	n = read(sock, buf, 100);
	/* Ignore result, it is just the wait that
	 * matters
	 */
}

void remove_pidfile(char *devname)
{
	char buf[100];

	sprintf(buf, "%s/%s.pid", MDMON_DIR, devname);
	unlink(buf);
	sprintf(buf, "%s/%s.sock", MDMON_DIR, devname);
	unlink(buf);
}

static int make_control_sock(char *devname)
{
	char path[100];
	int sfd;
	long fl;
	struct sockaddr_un addr;

	if (sigterm)
		return -1;

	sprintf(path, "%s/%s.sock", MDMON_DIR, devname);
	unlink(path);
	sfd = socket(PF_LOCAL, SOCK_STREAM, 0);
	if (sfd < 0)
		return -1;

	addr.sun_family = PF_LOCAL;
	strcpy(addr.sun_path, path);
	umask(077); /* ensure no world write access */
	if (bind(sfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		close(sfd);
		return -1;
	}
	listen(sfd, 10);
	fl = fcntl(sfd, F_GETFL, 0);
	fl |= O_NONBLOCK;
	fcntl(sfd, F_SETFL, fl);
	return sfd;
}

static void term(int sig)
{
	sigterm = 1;
}

static void wake_me(int sig)
{

}

/* if we are debugging and starting mdmon by hand then don't fork */
static int do_fork(void)
{
	#ifdef DEBUG
	if (check_env("MDADM_NO_MDMON"))
		return 0;
	#endif

	return 1;
}

void usage(void)
{
	fprintf(stderr,
"Usage: mdmon [options] CONTAINER\n"
"\n"
"Options are:\n"
"  --help        -h   : This message\n"
"  --all         -a   : All devices\n"
"  --foreground  -F   : Run in foreground (do not fork)\n"
"  --takeover    -t   : Takeover container\n"
);
	exit(2);
}

static int mdmon(char *devnm, int must_fork, int takeover);

int main(int argc, char *argv[])
{
	char *container_name = NULL;
	char *devnm = NULL;
	int status = 0;
	int opt;
	int all = 0;
	int takeover = 0;
	int dofork = 1;
	static struct option options[] = {
		{"all", 0, NULL, 'a'},
		{"takeover", 0, NULL, 't'},
		{"help", 0, NULL, 'h'},
		{"offroot", 0, NULL, OffRootOpt},
		{"foreground", 0, NULL, 'F'},
		{NULL, 0, NULL, 0}
	};

	if (in_initrd()) {
		/*
		 * set first char of argv[0] to @. This is used by
		 * systemd to signal that the task was launched from
		 * initrd/initramfs and should be preserved during shutdown
		 */
		argv[0][0] = '@';
	}

	while ((opt = getopt_long(argc, argv, "thaF", options, NULL)) != -1) {
		switch (opt) {
		case 'a':
			container_name = argv[optind-1];
			all = 1;
			break;
		case 't':
			takeover = 1;
			break;
		case 'F':
			dofork = 0;
			break;
		case OffRootOpt:
			argv[0][0] = '@';
			break;
		case 'h':
		default:
			usage();
			break;
		}
	}

	if (all == 0 && container_name == NULL) {
		if (argv[optind])
			container_name = argv[optind];
	}

	if (container_name == NULL)
		usage();

	if (argc - optind > 1)
		usage();

	if (strcmp(container_name, "/proc/mdstat") == 0)
		all = 1;

	if (all) {
		struct mdstat_ent *mdstat, *e;
		int container_len = strlen(container_name);

		/* launch an mdmon instance for each container found */
		mdstat = mdstat_read(0, 0);
		for (e = mdstat; e; e = e->next) {
			if (e->metadata_version &&
			    strncmp(e->metadata_version, "external:", 9) == 0 &&
			    !is_subarray(&e->metadata_version[9])) {
				/* update cmdline so this mdmon instance can be
				 * distinguished from others in a call to ps(1)
				 */
				if (strlen(e->devnm) <= (unsigned)container_len) {
					memset(container_name, 0, container_len);
					sprintf(container_name, "%s", e->devnm);
				}
				status |= mdmon(e->devnm, 1, takeover);
			}
		}
		free_mdstat(mdstat);

		return status;
	} else if (strncmp(container_name, "md", 2) == 0) {
		int id = devnm2devid(container_name);
		if (id)
			devnm = container_name;
	} else {
		struct stat st;

		if (stat(container_name, &st) == 0)
			devnm = xstrdup(stat2devnm(&st));
	}

	if (!devnm) {
		pr_err("%s is not a valid md device name\n",
			container_name);
		exit(1);
	}
	return mdmon(devnm, dofork && do_fork(), takeover);
}

static int mdmon(char *devnm, int must_fork, int takeover)
{
	int mdfd;
	struct mdinfo *mdi, *di;
	struct supertype *container;
	sigset_t set;
	struct sigaction act;
	int pfd[2];
	int status;
	int ignore;
	pid_t victim = -1;
	int victim_sock = -1;

	dprintf("starting mdmon for %s\n", devnm);

	mdfd = open_dev(devnm);
	if (mdfd < 0) {
		pr_err("%s: %s\n", devnm, strerror(errno));
		return 1;
	}

	/* Fork, and have the child tell us when they are ready */
	if (must_fork) {
		if (pipe(pfd) != 0) {
			pr_err("failed to create pipe\n");
			return 1;
		}
		switch(fork()) {
		case -1:
			pr_err("failed to fork: %s\n", strerror(errno));
			return 1;
		case 0: /* child */
			close(pfd[0]);
			break;
		default: /* parent */
			close(pfd[1]);
			if (read(pfd[0], &status, sizeof(status)) != sizeof(status)) {
				wait(&status);
				status = WEXITSTATUS(status);
			}
			close(pfd[0]);
			return status;
		}
	} else
		pfd[0] = pfd[1] = -1;

	container = xcalloc(1, sizeof(*container));
	strcpy(container->devnm, devnm);
	container->arrays = NULL;
	container->sock = -1;

	mdi = sysfs_read(mdfd, container->devnm, GET_VERSION|GET_LEVEL|GET_DEVS);

	if (!mdi) {
		pr_err("failed to load sysfs info for %s\n", container->devnm);
		exit(3);
	}
	if (mdi->array.level != UnSet) {
		pr_err("%s is not a container - cannot monitor\n", devnm);
		exit(3);
	}
	if (mdi->array.major_version != -1 ||
	    mdi->array.minor_version != -2) {
		pr_err("%s does not use external metadata - cannot monitor\n",
			devnm);
		exit(3);
	}

	container->ss = version_to_superswitch(mdi->text_version);
	if (container->ss == NULL) {
		pr_err("%s uses unsupported metadata: %s\n",
			devnm, mdi->text_version);
		exit(3);
	}

	container->devs = NULL;
	for (di = mdi->devs; di; di = di->next) {
		struct mdinfo *cd = xmalloc(sizeof(*cd));
		*cd = *di;
		cd->next = container->devs;
		container->devs = cd;
	}
	sysfs_free(mdi);

	/* SIGUSR is sent between parent and child.  So both block it
	 * and enable it only with pselect.
	 */
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	sigaddset(&set, SIGTERM);
	sigprocmask(SIG_BLOCK, &set, NULL);
	act.sa_handler = wake_me;
	act.sa_flags = 0;
	sigaction(SIGUSR1, &act, NULL);
	act.sa_handler = term;
	sigaction(SIGTERM, &act, NULL);
	act.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &act, NULL);

	victim = mdmon_pid(container->devnm);
	if (victim >= 0)
		victim_sock = connect_monitor(container->devnm);

	ignore = chdir("/");
	if (!takeover && victim > 0 && victim_sock >= 0) {
		if (fping_monitor(victim_sock) == 0) {
			pr_err("%s already managed\n", container->devnm);
			exit(3);
		}
		close(victim_sock);
		victim_sock = -1;
	}
	if (container->ss->load_container(container, mdfd, devnm)) {
		pr_err("Cannot load metadata for %s\n", devnm);
		exit(3);
	}
	close(mdfd);

	/* Ok, this is close enough.  We can say goodbye to our parent now.
	 */
	if (victim > 0)
		remove_pidfile(devnm);
	if (make_pidfile(devnm) < 0) {
		exit(3);
	}
	container->sock = make_control_sock(devnm);

	status = 0;
	if (pfd[1] >= 0) {
		if (write(pfd[1], &status, sizeof(status)) < 0)
			pr_err("failed to notify our parent: %d\n",
			       getppid());
		close(pfd[1]);
	}

	mlockall(MCL_CURRENT | MCL_FUTURE);

	if (clone_monitor(container) < 0) {
		pr_err("failed to start monitor process: %s\n",
			strerror(errno));
		exit(2);
	}

	if (victim > 0) {
		try_kill_monitor(victim, container->devnm, victim_sock);
		if (victim_sock >= 0)
			close(victim_sock);
	}

	setsid();
	close(0);
	open("/dev/null", O_RDWR);
	close(1);
	ignore = dup(0);
#ifndef DEBUG
	close(2);
	ignore = dup(0);
#endif

	/* This silliness is to stop the compiler complaining
	 * that we ignore 'ignore'
	 */
	if (ignore)
		ignore++;

	do_manager(container);

	exit(0);
}

/* Some stub functions so super-* can link with us */
int child_monitor(int afd, struct mdinfo *sra, struct reshape *reshape,
		  struct supertype *st, unsigned long blocks,
		  int *fds, unsigned long long *offsets,
		  int dests, int *destfd, unsigned long long *destoffsets)
{
	return 0;
}

int restore_stripes(int *dest, unsigned long long *offsets,
		    int raid_disks, int chunk_size, int level, int layout,
		    int source, unsigned long long read_offset,
		    unsigned long long start, unsigned long long length,
		    char *src_buf)
{
	return 1;
}

int save_stripes(int *source, unsigned long long *offsets,
		 int raid_disks, int chunk_size, int level, int layout,
		 int nwrites, int *dest,
		 unsigned long long start, unsigned long long length,
		 char *buf)
{
	return 0;
}

struct superswitch super0 = {
	.name = "0.90",
};
struct superswitch super1 = {
	.name = "1.x",
};
