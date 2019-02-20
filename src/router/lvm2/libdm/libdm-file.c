/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.  
 * Copyright (C) 2004-2007 Red Hat, Inc. All rights reserved.
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

#include "libdm/misc/dmlib.h"

#include <sys/file.h>
#include <fcntl.h>
#include <dirent.h>

static int _is_dir(const char *path)
{
	struct stat st;

	if (stat(path, &st) < 0) {
		log_sys_error("stat", path);
		return 0;
	}

	if (!S_ISDIR(st.st_mode)) {
		log_error("Existing path %s is not "
			  "a directory.", path);
		return 0;
	}

	return 1;
}

static int _create_dir_recursive(const char *dir)
{
	char *orig, *s;
	int rc, r = 0;

	log_verbose("Creating directory \"%s\"", dir);
	/* Create parent directories */
	orig = s = dm_strdup(dir);
	if (!s) {
		log_error("Failed to duplicate directory name.");
		return 0;
	}

	while ((s = strchr(s, '/')) != NULL) {
		*s = '\0';
		if (*orig) {
			rc = mkdir(orig, 0777);
			if (rc < 0) {
				if (errno == EEXIST) {
					if (!_is_dir(orig))
						goto_out;
				} else {
					if (errno != EROFS)
						log_sys_error("mkdir", orig);
					goto out;
				}
			}
		}
		*s++ = '/';
	}

	/* Create final directory */
	rc = mkdir(dir, 0777);
	if (rc < 0) {
		if (errno == EEXIST) {
			if (!_is_dir(dir))
				goto_out;
		} else {
			if (errno != EROFS)
				log_sys_error("mkdir", orig);
			goto out;
		}
	}

	r = 1;
out:
	dm_free(orig);
	return r;
}

int dm_create_dir(const char *dir)
{
	struct stat info;

	if (!*dir)
		return 1;

	if (stat(dir, &info) == 0 && S_ISDIR(info.st_mode))
		return 1;

	if (!_create_dir_recursive(dir))
		return_0;

	return 1;
}

int dm_is_empty_dir(const char *dir)
{
	struct dirent *dirent;
	DIR *d;

	if (!(d = opendir(dir))) {
		log_sys_error("opendir", dir);
		return 0;
	}

	while ((dirent = readdir(d)))
		if (strcmp(dirent->d_name, ".") && strcmp(dirent->d_name, ".."))
			break;

	if (closedir(d))
		log_sys_error("closedir", dir);

	return dirent ? 0 : 1;
}

int dm_fclose(FILE *stream)
{
	int prev_fail = ferror(stream);
	int fclose_fail = fclose(stream);

	/* If there was a previous failure, but fclose succeeded,
	   clear errno, since ferror does not set it, and its value
	   may be unrelated to the ferror-reported failure.  */
	if (prev_fail && !fclose_fail)
		errno = 0;

	return prev_fail || fclose_fail ? EOF : 0;
}

int dm_create_lockfile(const char *lockfile)
{
	int fd, value;
	size_t bufferlen;
	ssize_t write_out;
	struct flock lock;
	char buffer[50];
	int retries = 0;

	if ((fd = open(lockfile, O_CREAT | O_WRONLY,
		       (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))) < 0) {
		log_error("Cannot open lockfile [%s], error was [%s]",
			  lockfile, strerror(errno));
		return 0;
	}

	lock.l_type = F_WRLCK;
	lock.l_start = 0;
	lock.l_whence = SEEK_SET;
	lock.l_len = 0;
retry_fcntl:
	if (fcntl(fd, F_SETLK, &lock) < 0) {
		switch (errno) {
		case EINTR:
			goto retry_fcntl;
		case EACCES:
		case EAGAIN:
			if (retries == 20) {
				log_error("Cannot lock lockfile [%s], error was [%s]",
					  lockfile, strerror(errno));
				break;
			} else {
				++ retries;
				usleep(1000);
				goto retry_fcntl;
			}
		default:
			log_error("process is already running");
		}

		goto fail_close;
	}

	if (ftruncate(fd, 0) < 0) {
		log_error("Cannot truncate pidfile [%s], error was [%s]",
			  lockfile, strerror(errno));

		goto fail_close_unlink;
	}

	snprintf(buffer, sizeof(buffer), "%u\n", getpid());

	bufferlen = strlen(buffer);
	write_out = write(fd, buffer, bufferlen);

	if ((write_out < 0) || (write_out == 0 && errno)) {
		log_error("Cannot write pid to pidfile [%s], error was [%s]",
			  lockfile, strerror(errno));

		goto fail_close_unlink;
	}

	if ((write_out == 0) || ((size_t)write_out < bufferlen)) {
		log_error("Cannot write pid to pidfile [%s], shortwrite of"
			  "[%" PRIsize_t "] bytes, expected [%" PRIsize_t "]\n",
			  lockfile, write_out, bufferlen);

		goto fail_close_unlink;
	}

	if ((value = fcntl(fd, F_GETFD, 0)) < 0) {
		log_error("Cannot get close-on-exec flag from pidfile [%s], "
			  "error was [%s]", lockfile, strerror(errno));

		goto fail_close_unlink;
	}
	value |= FD_CLOEXEC;
	if (fcntl(fd, F_SETFD, value) < 0) {
		log_error("Cannot set close-on-exec flag from pidfile [%s], "
			  "error was [%s]", lockfile, strerror(errno));

		goto fail_close_unlink;
	}

	/* coverity[leaked_handle] intentional leak of fd handle here  */

	return 1;

fail_close_unlink:
	if (unlink(lockfile))
		log_sys_debug("unlink", lockfile);
fail_close:
	if (close(fd))
		log_sys_debug("close", lockfile);

	return 0;
}

int dm_daemon_is_running(const char* lockfile)
{
       int fd;
       struct flock lock;

       if((fd = open(lockfile, O_RDONLY)) < 0)
               return 0;

       lock.l_type = F_WRLCK;
       lock.l_start = 0;
       lock.l_whence = SEEK_SET;
       lock.l_len = 0;
       if (fcntl(fd, F_GETLK, &lock) < 0) {
               log_error("Cannot check lock status of lockfile [%s], error was [%s]",
                         lockfile, strerror(errno));
               if (close(fd))
                       stack;
               return 0;
       }

       if (close(fd))
               stack;

       return (lock.l_type == F_UNLCK) ? 0 : 1;
}
