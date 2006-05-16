#include "headers.h"
#include "pidfile.h"

int
write_pid_file(int pf, pid_t pid, char *msg) {
	char _buf[32];
	int ret, len;
	struct flock flk;

	/*
	 * Lock it again.
	 */

	flk.l_start = 0;	/* lock from start */
	flk.l_len = 0;	/* till the end */
	flk.l_type = F_WRLCK;
	flk.l_whence = 0;
	(void)fcntl(pf, F_SETLK, &flk);

	if(pid) {
		if(snprintf(_buf, sizeof(_buf), "%lu\n",
			(unsigned long)pid) >= (int)sizeof(_buf))
				return -1;
		msg = _buf;
	}

	if(lseek(pf, 0, SEEK_SET) != 0) {
		errno = EIO;
		return -1;
	}

	len = strlen(msg);
	do {
		ret = write(pf, msg, len);
	} while(ret == -1 && errno == EINTR);

	if(ret != len) {
		errno = EIO;
		return -1;
	}

	ftruncate(pf, len);

	fsync(pf);

	return 0;
}

int
make_pid_file(char *pidfile) {
	struct stat sb;
	struct flock flk;
	char buf[32] = { '\0' };
	int pf;
	int ret;
	int open_flags = O_RDWR | O_CREAT;

	if(!pidfile) {
		errno = EINVAL;
		return -1;
	}

	if(*pidfile == '\0') {
		errno = 0;
		return -1;
	}

	ret = stat(pidfile, &sb);
	if(ret == -1) {
		if(errno != ENOENT)
			return -1;

		open_flags |= O_EXCL;
	} else {
		if((sb.st_mode & S_IFMT) != S_IFREG) {
			fprintf(stderr, "%s: Inappropriate file type\n",
				pidfile);
			errno = EPERM;
			return -1;
		}
	}

	/*
	 * Open the PID file
	 */

	pf = open(pidfile, open_flags, 0644);
	if(pf == -1) {
		fprintf(stderr,
			"Can't open PID file %s: %s\n",
				pidfile, strerror(errno));
		return -1;
	}

	/*
	 * Set close-on-exec flag.
	 */

	ret = fcntl(pf, F_GETFD, 0);
	if(ret == -1) {
		close(pf);
		fprintf(stderr,
			"Can't get flags for %s: %s\n",
				pidfile, strerror(errno));
		return -1;
	}
	if(fcntl(pf, F_SETFD, ret | FD_CLOEXEC) == -1) {
		close(pf);
		fprintf(stderr,
			"Can't set close-on-exec flag for %s: %s\n",
				pidfile, strerror(errno));
		return -1;
	}

	/*
	 * Lock the file.
	 */

	flk.l_start = 0;	/* lock from start */
	flk.l_len = 0;	/* till the end */
	flk.l_type = F_WRLCK;
	flk.l_whence = 0;

	ret = fcntl(pf, F_SETLK, &flk);
	if(ret != -1) {
		if(write_pid_file(pf, getpid(), NULL)) {
			fprintf(stderr,
				"Can't write PID file %s\n", pidfile);
			return -1;
		}

		/* Don't close the file descriptor in order to save lock. */

		return pf;
	}

	if(errno == EAGAIN) {
		ret = read(pf, buf, sizeof(buf));
		if(ret > 0 && buf[ret - 1] == '\n') {
			buf[ret - 1] = '\0';
			ret = fcntl(pf, F_GETLK, &flk);
			if(ret
			|| (unsigned long)flk.l_pid == strtoul(buf, NULL, 10)) {
				fprintf(stderr,
	"Can't start: another instance running, pid=%s\n", buf);
			} else {
				fprintf(stderr,
	"Can't start: another instance running, pid=%s, lock pid=%lu\n",
				buf, (unsigned long)flk.l_pid);
			}
		} else {
			fprintf(stderr,
				"Can't start: another instance running\n");
		}
	} else {
		fprintf(stderr,
			"Can't lock PID file %s: %s\n",
				pidfile, strerror(errno));
	}
	close(pf);

	errno = EPERM;

	return -1;
}

