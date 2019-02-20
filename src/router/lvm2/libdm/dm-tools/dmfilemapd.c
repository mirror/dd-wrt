/*
 * Copyright (C) 2016 Red Hat, Inc. All rights reserved.
 *
 * This file is part of the device-mapper userspace tools.
 *
 * It includes tree drawing code based on pstree: http://psmisc.sourceforge.net/
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU General Public License v.2.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "util.h"
#include "libdm/misc/dm-logging.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/inotify.h>
#include <dirent.h>
#include <ctype.h>

#ifdef __linux__
#  include "libdm/misc/kdev_t.h"
#else
#  define MAJOR(x) major((x))
#  define MINOR(x) minor((x))
#  define MKDEV(x,y) makedev((x),(y))
#endif

#define DEFAULT_PROC_DIR "/proc"

/* limit to two updates/sec */
#define FILEMAPD_WAIT_USECS 500000

/* how long to wait for unlinked files */
#define FILEMAPD_NOFILE_WAIT_USECS 100000
#define FILEMAPD_NOFILE_WAIT_TRIES 10

struct filemap_monitor {
	dm_filemapd_mode_t mode;
	const char *program_id;
	uint64_t group_id;
	char *path;
	int fd;

	int inotify_fd;
	int inotify_watch_fd;

	/* monitoring heuristics */
	int64_t blocks; /* allocated blocks, from stat.st_blocks */
	uint64_t nr_regions;
	int deleted;
};

static int _foreground;
static int _verbose;

const char *const _usage = "dmfilemapd <fd> <group_id> <abs_path> <mode> "
			   "[<foreground>[<log_level>]]";

/*
 * Daemon logging. By default, all messages are thrown away: messages
 * are only written to the terminal if the daemon is run in the foreground.
 */
__attribute__((format(printf, 5, 0)))
static void _dmfilemapd_log_line(int level,
				 const char *file __attribute__((unused)),
				 int line __attribute__((unused)),
				 int dm_errno_or_class,
				 const char *f, va_list ap)
{
	static int _abort_on_internal_errors = -1;
	FILE *out = log_stderr(level) ? stderr : stdout;

	level = log_level(level);

	if (level <= _LOG_WARN || _verbose) {
		if (level < _LOG_WARN)
			out = stderr;
		vfprintf(out, f, ap);
		fputc('\n', out);
	}

	if (_abort_on_internal_errors < 0)
		/* Set when env DM_ABORT_ON_INTERNAL_ERRORS is not "0" */
		_abort_on_internal_errors =
			strcmp(getenv("DM_ABORT_ON_INTERNAL_ERRORS") ? : "0", "0");

	if (_abort_on_internal_errors &&
	    !strncmp(f, INTERNAL_ERROR, sizeof(INTERNAL_ERROR) - 1))
		abort();
}

__attribute__((format(printf, 5, 6)))
static void _dmfilemapd_log_with_errno(int level,
				       const char *file, int line,
				       int dm_errno_or_class,
				       const char *f, ...)
{
	va_list ap;

	va_start(ap, f);
	_dmfilemapd_log_line(level, file, line, dm_errno_or_class, f, ap);
	va_end(ap);
}

/*
 * Only used for reporting errors before daemonise().
 */
__attribute__((format(printf, 1, 2)))
static void _early_log(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fputc('\n', stderr);
	va_end(ap);
}

static void _setup_logging(void)
{
	dm_log_init_verbose(_verbose - 1);
	dm_log_with_errno_init(_dmfilemapd_log_with_errno);
}

#define PROC_FD_DELETED_STR "(deleted)"
/*
 * Scan the /proc/<pid>/fd directory for pid and check for an fd
 * symlink whose contents match path.
 */
static int _is_open_in_pid(pid_t pid, const char *path)
{
	char deleted_path[PATH_MAX + sizeof(PROC_FD_DELETED_STR)];
	struct dirent *pid_dp = NULL;
	char path_buf[PATH_MAX];
	char link_buf[PATH_MAX];
	DIR *pid_d = NULL;
	ssize_t len;

	if (pid == getpid())
		return 0;

	if (dm_snprintf(path_buf, sizeof(path_buf),
			DEFAULT_PROC_DIR "%d/fd", pid) < 0) {
		log_error("Could not format pid path.");
		return 0;
	}

	/*
	 * Test for the kernel 'file (deleted)' form when scanning.
	 */
	if (dm_snprintf(deleted_path, sizeof(deleted_path), "%s %s",
			path, PROC_FD_DELETED_STR) < 0) {
		log_error("Could not format check path.");
		return 0;
	}

	pid_d = opendir(path_buf);
	if (!pid_d) {
		log_error("Could not open proc path: %s.", path_buf);
		return 0;
	}

	while ((pid_dp = readdir(pid_d)) != NULL) {
		if (pid_dp->d_name[0] == '.')
			continue;
		if ((len = readlinkat(dirfd(pid_d), pid_dp->d_name, link_buf,
				      sizeof(link_buf))) < 0) {
			log_error("readlink failed for " DEFAULT_PROC_DIR
				  "/%d/fd/.", pid);
			goto bad;
		}
		link_buf[len] = '\0';
		if (!strcmp(deleted_path, link_buf)) {
			if (closedir(pid_d))
				log_sys_error("closedir", path_buf);
			return 1;
		}
	}

bad:
	if (closedir(pid_d))
		log_sys_error("closedir", path_buf);

	return 0;
}

/*
 * Attempt to determine whether a file is open by any process by
 * scanning symbolic links in /proc/<pid>/fd.
 *
 * This is a heuristic since it cannot guarantee to detect brief
 * access in all cases: a process that opens and then closes the
 * file rapidly may never be seen by the scan.
 *
 * The method will also give false-positives if a process exists
 * that has a deleted file open that had the same path, but a
 * different inode number, to the file being monitored.
 *
 * For this reason the daemon only uses _is_open() for unlinked
 * files when the mode is DM_FILEMAPD_FOLLOW_INODE, since these
 * files can no longer be newly opened by processes.
 *
 * In this situation !is_open(path) provides an indication that
 * the daemon should shut down: the file has been unlinked from
 * the file system and we appear to hold the final reference.
 */
static int _is_open(const char *path)
{
	struct dirent *proc_dp = NULL;
	DIR *proc_d = NULL;
	pid_t pid;

	proc_d = opendir(DEFAULT_PROC_DIR);
	if (!proc_d)
		return 0;
	while ((proc_dp = readdir(proc_d)) != NULL) {
		if (!isdigit(proc_dp->d_name[0]))
			continue;
		errno = 0;
		pid = (pid_t) strtol(proc_dp->d_name, NULL, 10);
		if (errno || !pid)
			continue;
		if (_is_open_in_pid(pid, path)) {
			if (closedir(proc_d))
				log_sys_error("closedir", DEFAULT_PROC_DIR);
			return 1;
		}
	}

	if (closedir(proc_d))
		log_sys_error("closedir", DEFAULT_PROC_DIR);

	return 0;
}

static void _filemap_monitor_wait(uint64_t usecs)
{
	if (_verbose) {
		if (usecs == FILEMAPD_WAIT_USECS)
			log_very_verbose("Waiting for check interval");
		if (usecs == FILEMAPD_NOFILE_WAIT_USECS)
			log_very_verbose("Waiting for unlinked path");
	}
	usleep((useconds_t) usecs);
}

static int _parse_args(int argc, char **argv, struct filemap_monitor *fm)
{
	char *endptr;

	/* we don't care what is in argv[0]. */
	argc--;
	argv++;

	if (argc < 5) {
		_early_log("Wrong number of arguments.");
		_early_log("usage: %s", _usage);
		return 0;
	}

	/*
	 * We don't know the true nr_regions at daemon start time,
	 * and it is not worth a dm_stats_list()/group walk to count:
	 * we can assume that there is at least one region or the
	 * daemon would not have been started.
	 *
	 * A correct value will be obtained following the first update
	 * of the group's regions.
	 */
	fm->nr_regions = 1;

	/* parse <fd> */
	errno = 0;
	fm->fd = (int) strtol(argv[0], &endptr, 10);
	if (errno || *endptr) {
		_early_log("Could not parse file descriptor: %s", argv[0]);
		return 0;
	}

	argc--;
	argv++;

	/* parse <group_id> */
	errno = 0;
	fm->group_id = strtoull(argv[0], &endptr, 10);
	if (*endptr || errno) {
		_early_log("Could not parse group identifier: %s", argv[0]);
		return 0;
	}

	argc--;
	argv++;

	/* parse <path> */
	if (!argv[0] || !strlen(argv[0])) {
		_early_log("Path argument is required.");
		return 0;
	}

	if (*argv[0] != '/') {
		_early_log("Path argument must specify an absolute path.");
		return 0;
	}

	fm->path = strdup(argv[0]);
	if (!fm->path) {
		_early_log("Could not allocate memory for path argument.");
		return 0;
	}

	argc--;
	argv++;

	/* parse <mode> */
	if (!argv[0] || !strlen(argv[0])) {
		_early_log("Mode argument is required.");
		return 0;
	}

	fm->mode = dm_filemapd_mode_from_string(argv[0]);
	if (fm->mode == DM_FILEMAPD_FOLLOW_NONE)
		return 0;

	argc--;
	argv++;

	/* parse [<foreground>[<verbose>]] */
	if (argc) {
		errno = 0;
		_foreground = (int) strtol(argv[0], &endptr, 10);
		if (errno || *endptr) {
			_early_log("Could not parse debug argument: %s.",
				   argv[0]);
			return 0;
		}
		argc--;
		argv++;
		if (argc) {
			errno = 0;
			_verbose = (int) strtol(argv[0], &endptr, 10);
			if (errno || *endptr) {
				_early_log("Could not parse verbose "
					   "argument: %s", argv[0]);
				return 0;
			}
			if (_verbose < 0 || _verbose > 3) {
				_early_log("Verbose argument out of range: %d.",
					   _verbose);
				return 0;
			}
		}
	}
	return 1;
}

static int _filemap_fd_update_blocks(struct filemap_monitor *fm)
{
	struct stat buf;

	if (fm->fd < 0) {
		log_error("Filemap fd is not open.");
		return 0;
	}

	if (fstat(fm->fd, &buf)) {
		log_error("Failed to fstat filemap file descriptor.");
		return 0;
	}

	fm->blocks = buf.st_blocks;

	return 1;
}

static int _filemap_fd_check_changed(struct filemap_monitor *fm)
{
	int64_t old_blocks;

	old_blocks = fm->blocks;

	if (!_filemap_fd_update_blocks(fm))
		return -1;

	return (fm->blocks != old_blocks);
}

static void _filemap_monitor_close_fd(struct filemap_monitor *fm)
{
	if (close(fm->fd))
		log_error("Error closing file descriptor.");
	fm->fd = -1;
}

static void _filemap_monitor_end_notify(struct filemap_monitor *fm)
{
	inotify_rm_watch(fm->inotify_fd, fm->inotify_watch_fd);
}

static int _filemap_monitor_set_notify(struct filemap_monitor *fm)
{
	int inotify_fd, watch_fd;

	/*
	 * Set IN_NONBLOCK since we do not want to block in event read()
	 * calls. Do not set IN_CLOEXEC as dmfilemapd is single-threaded
	 * and does not fork or exec.
	 */
	if ((inotify_fd = inotify_init1(IN_NONBLOCK)) < 0) {
		log_sys_error("inotify_init1", "IN_NONBLOCK");
		return 0;
	}

	if ((watch_fd = inotify_add_watch(inotify_fd, fm->path,
					  IN_MODIFY | IN_DELETE_SELF)) < 0) {
		log_sys_error("inotify_add_watch", fm->path);
		return 0;
	}
	fm->inotify_fd = inotify_fd;
	fm->inotify_watch_fd = watch_fd;
	return 1;
}

static int _filemap_monitor_reopen_fd(struct filemap_monitor *fm)
{
	int tries = FILEMAPD_NOFILE_WAIT_TRIES;

	/*
	 * In DM_FILEMAPD_FOLLOW_PATH mode, inotify watches must be
	 * re-established whenever the file at the watched path is
	 * changed.
	 *
	 * FIXME: stat file and skip if inode is unchanged.
	 */
	if (fm->fd > 0)
		log_error("Filemap file descriptor already open.");

	while ((fm->fd < 0) && --tries)
		if (((fm->fd = open(fm->path, O_RDONLY)) < 0) && tries)
			_filemap_monitor_wait(FILEMAPD_NOFILE_WAIT_USECS);

	if (!tries && (fm->fd < 0)) {
		log_error("Could not re-open file descriptor.");
		return 0;
	}

	return _filemap_monitor_set_notify(fm);
}

static int _filemap_monitor_get_events(struct filemap_monitor *fm)
{
	/* alignment as per man(7) inotify */
	char buf[sizeof(struct inotify_event) + NAME_MAX + 1]
		__attribute__ ((aligned(__alignof__(struct inotify_event))));

	struct inotify_event *event;
	int check = 0;
	ssize_t len;
	char *ptr;

	/*
	 * Close the file descriptor for the file being monitored here
	 * when mode=path: this will allow the inode to be de-allocated,
	 * and an IN_DELETE_SELF event generated in the case that the
	 * daemon is holding the last open reference to the file.
	 */
	if (fm->mode == DM_FILEMAPD_FOLLOW_PATH) {
		_filemap_monitor_end_notify(fm);
		_filemap_monitor_close_fd(fm);
	}

	len = read(fm->inotify_fd, (void *) &buf, sizeof(buf));

	/* no events to read? */
	if (len < 0 && (errno == EAGAIN))
		goto out;

	/* interrupted by signal? */
	if (len < 0 && (errno == EINTR))
		goto out;

	if (len < 0)
		return -1;

	if (!len)
		goto out;

	for (ptr = buf; ptr < buf + len; ptr += sizeof(*event) + event->len) {
		event = (struct inotify_event *) ptr;
		if (event->mask & IN_DELETE_SELF)
			fm->deleted = 1;
		if (event->mask & IN_MODIFY)
			check = 1;
		/*
		 * Event IN_IGNORED is generated when a file has been deleted
		 * and IN_DELETE_SELF generated, and indicates that the file
		 * watch has been automatically removed.
		 *
		 * This can only happen for the DM_FILEMAPD_FOLLOW_PATH mode,
		 * since inotify IN_DELETE events are generated at the time
		 * the inode is destroyed: DM_FILEMAPD_FOLLOW_INODE will hold
		 * the file descriptor open, meaning that the event will not
		 * be generated until after the daemon closes the file.
		 *
		 * The event is ignored here since inotify monitoring will
		 * be reestablished (or the daemon will terminate) following
		 * deletion of a DM_FILEMAPD_FOLLOW_PATH monitored file.
		 */
		if (event->mask & IN_IGNORED)
			log_very_verbose("Inotify watch removed: IN_IGNORED "
					 "in event->mask");
	}

out:
	/*
	 * Re-open file descriptor if required and log disposition.
	 */
	if (fm->mode == DM_FILEMAPD_FOLLOW_PATH)
		if (!_filemap_monitor_reopen_fd(fm))
			return -1;

	log_very_verbose("exiting _filemap_monitor_get_events() with "
			 "deleted=%d, check=%d", fm->deleted, check);
	return check;
}

static void _filemap_monitor_destroy(struct filemap_monitor *fm)
{
	if (fm->fd > 0) {
		_filemap_monitor_end_notify(fm);
		_filemap_monitor_close_fd(fm);
	}
	free((void *) fm->program_id);
	free(fm->path);
}

static int _filemap_monitor_check_same_file(int fd1, int fd2)
{
	struct stat buf1, buf2;

	if ((fd1 < 0) || (fd2 < 0))
		return 0;

	if (fstat(fd1, &buf1)) {
		log_error("Failed to fstat file descriptor %d", fd1);
		return -1;
	}

	if (fstat(fd2, &buf2)) {
		log_error("Failed to fstat file descriptor %d", fd2);
		return -1;
	}

	return ((buf1.st_dev == buf2.st_dev) && (buf1.st_ino == buf2.st_ino));
}

static int _filemap_monitor_check_file_unlinked(struct filemap_monitor *fm)
{
	char path_buf[PATH_MAX];
	char link_buf[PATH_MAX];
	int same, fd;
	ssize_t len;

	fm->deleted = 0;
	same = 0;

	if ((fd = open(fm->path, O_RDONLY)) < 0)
		goto check_unlinked;

	same = _filemap_monitor_check_same_file(fm->fd, fd);

	if (close(fd))
		log_error("Error closing fd %d", fd);

	if (same < 0)
		return 0;

	if (same)
		return 1;

check_unlinked:
	/*
	 * The file has been unlinked from its original location: test
	 * whether it is still reachable in the filesystem, or if it is
	 * unlinked and anonymous.
	 */
	if (dm_snprintf(path_buf, sizeof(path_buf), DEFAULT_PROC_DIR
			"/%d/fd/%d", getpid(), fm->fd) < 0) {
		log_error("Could not format pid path.");
		return 0;
	}
	if ((len = readlink(path_buf, link_buf, sizeof(link_buf) - 1)) < 0) {
		log_error("readlink failed for " DEFAULT_PROC_DIR "/%d/fd/%d.",
			  getpid(), fm->fd);
		return 0;
	}
	link_buf[len] = '\0';

	/*
	 * Try to re-open the file, from the path now reported in /proc/pid/fd.
	 */
	if ((fd = open(link_buf, O_RDONLY)) < 0)
		fm->deleted = 1;
	else
		same = _filemap_monitor_check_same_file(fm->fd, fd);

	if ((fd >= 0) && close(fd))
		log_error("Error closing fd %d", fd);

	if (same < 0)
		return 0;

	/* Should not happen with normal /proc. */
	if ((fd > 0) && !same) {
		log_error("File descriptor mismatch: %d and %s (read from %s) "
			  "are not the same file!", fm->fd, link_buf, path_buf);
		return 0;
	}
	return 1;
}

static int _daemonise(struct filemap_monitor *fm)
{
	pid_t pid = 0;
	int fd;

	if (!setsid()) {
		_early_log("setsid failed.");
		return 0;
	}

	if ((pid = fork()) < 0) {
		_early_log("Failed to fork daemon process.");
		return 0;
	}

	if (pid > 0) {
		if (_verbose)
			_early_log("Started dmfilemapd with pid=%d", pid);
		exit(0);
	}

	if (chdir("/")) {
		_early_log("Failed to change directory.");
		return 0;
	}

	if (!_verbose) {
		if (close(STDIN_FILENO))
			_early_log("Error closing stdin");
		if (close(STDOUT_FILENO))
			_early_log("Error closing stdout");
		if (close(STDERR_FILENO))
			_early_log("Error closing stderr");
		if ((open("/dev/null", O_RDONLY) < 0) ||
	            (open("/dev/null", O_WRONLY) < 0) ||
		    (open("/dev/null", O_WRONLY) < 0)) {
			_early_log("Error opening stdio streams.");
			return 0;
		}
	}
	/* TODO: Use libdaemon/server/daemon-server.c _daemonise() */
	for (fd = (int) sysconf(_SC_OPEN_MAX) - 1; fd > STDERR_FILENO; fd--) {
		if (fd == fm->fd)
			continue;
		(void) close(fd);
	}

	return 1;
}

static int _update_regions(struct dm_stats *dms, struct filemap_monitor *fm)
{
	uint64_t *regions = NULL, *region, nr_regions = 0;

	regions = dm_stats_update_regions_from_fd(dms, fm->fd, fm->group_id);
	if (!regions) {
		log_error("Failed to update filemap regions for group_id="
			  FMTu64 ".", fm->group_id);
		return 0;
	}

	for (region = regions; *region != DM_STATS_REGIONS_ALL; region++)
		nr_regions++;

	if (!nr_regions)
		log_warn("File contains no extents: exiting.");

	if (nr_regions && (regions[0] != fm->group_id)) {
		log_warn("group_id changed from " FMTu64 " to " FMTu64,
			 fm->group_id, regions[0]);
		fm->group_id = regions[0];
	}
	free(regions);
	fm->nr_regions = nr_regions;
	return 1;
}

static int _dmfilemapd(struct filemap_monitor *fm)
{
	int running = 1, check = 0, open = 0;
	const char *program_id;
	struct dm_stats *dms;

	/*
	 * The correct program_id is retrieved from the group leader
	 * following the call to dm_stats_list().
	 */
	if (!(dms = dm_stats_create(NULL)))
		goto_bad;

	if (!dm_stats_bind_from_fd(dms, fm->fd)) {
		log_error("Could not bind dm_stats handle to file descriptor "
			  "%d", fm->fd);
		goto bad;
	}

	if (!_filemap_monitor_set_notify(fm))
		goto bad;

	if (!_filemap_fd_update_blocks(fm))
		goto bad;

	if (!dm_stats_list(dms, DM_STATS_ALL_PROGRAMS)) {
		log_error("Failed to list stats handle.");
		goto bad;
	}

	/*
	 * Take the program_id for new regions (created by calls to
	 * dm_stats_update_regions_from_fd()) from the value used by
	 * the group leader.
	 */
	program_id = dm_stats_get_region_program_id(dms, fm->group_id);
	if (program_id)
		fm->program_id = strdup(program_id);
	else
		fm->program_id = NULL;
	dm_stats_set_program_id(dms, 1, program_id);

	do {
		if (!dm_stats_group_present(dms, fm->group_id)) {
			log_info("Filemap group removed: exiting.");
			running = 0;
			continue;
		}

		if ((check = _filemap_monitor_get_events(fm)) < 0)
			goto bad;

		if (!check)
			goto wait;

		if ((check = _filemap_fd_check_changed(fm)) < 0)
			goto bad;

		if (check && !_update_regions(dms, fm))
			goto bad;

		running = !!fm->nr_regions;
		if (!running)
			continue;

wait:
		_filemap_monitor_wait(FILEMAPD_WAIT_USECS);

		/* mode=inode termination condions */
		if (fm->mode == DM_FILEMAPD_FOLLOW_INODE) {
			if (!_filemap_monitor_check_file_unlinked(fm))
				goto bad;
			if (fm->deleted && !(open = _is_open(fm->path))) {
				log_info("File unlinked and closed: exiting.");
				running = 0;
			} else if (fm->deleted && open)
				log_verbose("File unlinked and open: "
					     "continuing.");
		}

		if (!dm_stats_list(dms, NULL)) {
			log_error("Failed to list stats handle.");
			goto bad;
		}

	} while (running);

	_filemap_monitor_destroy(fm);
	dm_stats_destroy(dms);
	return 0;

bad:
	_filemap_monitor_destroy(fm);
	dm_stats_destroy(dms);
	log_error("Exiting");
	return 1;
}

static const char * const _mode_names[] = {
	"inode",
	"path"
};

/*
 * dmfilemapd <fd> <group_id> <path> <mode> [<foreground>[<log_level>]]
 */
int main(int argc, char **argv)
{
	struct filemap_monitor fm;

	memset(&fm, 0, sizeof(fm));

	if (!_parse_args(argc, argv, &fm)) {
		free(fm.path);
		return 1;
	}

	_setup_logging();

	log_info("Starting dmfilemapd with fd=%d, group_id=" FMTu64 " "
		 "mode=%s, path=%s", fm.fd, fm.group_id,
		 _mode_names[fm.mode], fm.path);

	if (!_foreground && !_daemonise(&fm)) {
		free(fm.path);
		return 1;
	}

	return _dmfilemapd(&fm);
}
