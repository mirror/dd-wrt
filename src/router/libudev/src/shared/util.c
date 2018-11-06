/***
  This file is part of eudev, forked from systemd.

  Copyright 2010 Lennart Poettering

  systemd is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.

  systemd is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with systemd; If not, see <http://www.gnu.org/licenses/>.
***/

#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <syslog.h>
#include <sched.h>
#include <sys/resource.h>
#include <linux/sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include <poll.h>
#include <ctype.h>
#include <sys/prctl.h>
#include <sys/utsname.h>
#include <pwd.h>
#include <netinet/ip.h>
#include <linux/kd.h>
#include <dlfcn.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <glob.h>
#include <grp.h>
#include <sys/mman.h>
#include <sys/vfs.h>
#include <linux/magic.h>
#include <limits.h>
#include <langinfo.h>
#include <locale.h>
#include <libgen.h>

#include "macro.h"
#include "util.h"
#include "ioprio.h"
#include "missing.h"
#include "log.h"
#include "strv.h"
#include "mkdir.h"
#include "path-util.h"
#include "hashmap.h"
#include "fileio.h"
#include "utf8.h"
#include "virt.h"
#include "process-util.h"
#include "random-util.h"
#include "terminal-util.h"

/* Put this test here for a lack of better place */
assert_cc(EAGAIN == EWOULDBLOCK);

int saved_argc = 0;
char **saved_argv = NULL;

size_t page_size(void) {
        static thread_local size_t pgsz = 0;
        long r;

        if (_likely_(pgsz > 0))
                return pgsz;

        r = sysconf(_SC_PAGESIZE);
        assert(r > 0);

        pgsz = (size_t) r;
        return pgsz;
}

bool streq_ptr(const char *a, const char *b) {

        /* Like streq(), but tries to make sense of NULL pointers */

        if (a && b)
                return streq(a, b);

        if (!a && !b)
                return true;

        return false;
}

char* endswith(const char *s, const char *postfix) {
        size_t sl, pl;

        assert(s);
        assert(postfix);

        sl = strlen(s);
        pl = strlen(postfix);

        if (pl == 0)
                return (char*) s + sl;

        if (sl < pl)
                return NULL;

        if (memcmp(s + sl - pl, postfix, pl) != 0)
                return NULL;

        return (char*) s + sl - pl;
}

size_t cescape_char(char c, char *buf) {
        char * buf_old = buf;

        switch (c) {

                case '\a':
                        *(buf++) = '\\';
                        *(buf++) = 'a';
                        break;
                case '\b':
                        *(buf++) = '\\';
                        *(buf++) = 'b';
                        break;
                case '\f':
                        *(buf++) = '\\';
                        *(buf++) = 'f';
                        break;
                case '\n':
                        *(buf++) = '\\';
                        *(buf++) = 'n';
                        break;
                case '\r':
                        *(buf++) = '\\';
                        *(buf++) = 'r';
                        break;
                case '\t':
                        *(buf++) = '\\';
                        *(buf++) = 't';
                        break;
                case '\v':
                        *(buf++) = '\\';
                        *(buf++) = 'v';
                        break;
                case '\\':
                        *(buf++) = '\\';
                        *(buf++) = '\\';
                        break;
                case '"':
                        *(buf++) = '\\';
                        *(buf++) = '"';
                        break;
                case '\'':
                        *(buf++) = '\\';
                        *(buf++) = '\'';
                        break;

                default:
                        /* For special chars we prefer octal over
                         * hexadecimal encoding, simply because glib's
                         * g_strescape() does the same */
                        if ((c < ' ') || (c >= 127)) {
                                *(buf++) = '\\';
                                *(buf++) = octchar((unsigned char) c >> 6);
                                *(buf++) = octchar((unsigned char) c >> 3);
                                *(buf++) = octchar((unsigned char) c);
                        } else
                                *(buf++) = c;
                        break;
        }

        return buf - buf_old;
}

int close_nointr(int fd) {
        assert(fd >= 0);

        if (close(fd) >= 0)
                return 0;

        /*
         * Just ignore EINTR; a retry loop is the wrong thing to do on
         * Linux.
         *
         * http://lkml.indiana.edu/hypermail/linux/kernel/0509.1/0877.html
         * https://bugzilla.gnome.org/show_bug.cgi?id=682819
         * http://utcc.utoronto.ca/~cks/space/blog/unix/CloseEINTR
         * https://sites.google.com/site/michaelsafyan/software-engineering/checkforeintrwheninvokingclosethinkagain
         */
        if (errno == EINTR)
                return 0;

        return -errno;
}

int safe_close(int fd) {

        /*
         * Like close_nointr() but cannot fail. Guarantees errno is
         * unchanged. Is a NOP with negative fds passed, and returns
         * -1, so that it can be used in this syntax:
         *
         * fd = safe_close(fd);
         */

        if (fd >= 0) {
                PROTECT_ERRNO;

                /* The kernel might return pretty much any error code
                 * via close(), but the fd will be closed anyway. The
                 * only condition we want to check for here is whether
                 * the fd was invalid at all... */

                assert_se(close_nointr(fd) != -EBADF);
        }

        return -1;
}

void close_many(const int fds[], unsigned n_fd) {
        unsigned i;

        assert(fds || n_fd <= 0);

        for (i = 0; i < n_fd; i++)
                safe_close(fds[i]);
}

int unlink_noerrno(const char *path) {
        PROTECT_ERRNO;
        int r;

        r = unlink(path);
        if (r < 0)
                return -errno;

        return 0;
}

int parse_uid(const char *s, uid_t* ret_uid) {
        unsigned long ul = 0;
        uid_t uid;
        int r;

        assert(s);

        r = safe_atolu(s, &ul);
        if (r < 0)
                return r;

        uid = (uid_t) ul;

        if ((unsigned long) uid != ul)
                return -ERANGE;

        /* Some libc APIs use UID_INVALID as special placeholder */
        if (uid == (uid_t) 0xFFFFFFFF)
                return -ENXIO;

        /* A long time ago UIDs where 16bit, hence explicitly avoid the 16bit -1 too */
        if (uid == (uid_t) 0xFFFF)
                return -ENXIO;

        if (ret_uid)
                *ret_uid = uid;

        return 0;
}

int safe_atou(const char *s, unsigned *ret_u) {
        char *x = NULL;
        unsigned long l;

        assert(s);
        assert(ret_u);

        errno = 0;
        l = strtoul(s, &x, 0);

        if (!x || x == s || *x || errno)
                return errno > 0 ? -errno : -EINVAL;

        if ((unsigned long) (unsigned) l != l)
                return -ERANGE;

        *ret_u = (unsigned) l;
        return 0;
}

int safe_atoi(const char *s, int *ret_i) {
        char *x = NULL;
        long l;

        assert(s);
        assert(ret_i);

        errno = 0;
        l = strtol(s, &x, 0);

        if (!x || x == s || *x || errno)
                return errno > 0 ? -errno : -EINVAL;

        if ((long) (int) l != l)
                return -ERANGE;

        *ret_i = (int) l;
        return 0;
}

int safe_atollu(const char *s, long long unsigned *ret_llu) {
        char *x = NULL;
        unsigned long long l;

        assert(s);
        assert(ret_llu);

        errno = 0;
        l = strtoull(s, &x, 0);

        if (!x || x == s || *x || errno)
                return errno ? -errno : -EINVAL;

        *ret_llu = l;
        return 0;
}

int safe_atolli(const char *s, long long int *ret_lli) {
        char *x = NULL;
        long long l;

        assert(s);
        assert(ret_lli);

        errno = 0;
        l = strtoll(s, &x, 0);

        if (!x || x == s || *x || errno)
                return errno ? -errno : -EINVAL;

        *ret_lli = l;
        return 0;
}

static size_t strcspn_escaped(const char *s, const char *reject) {
        bool escaped = false;
        int n;

        for (n=0; s[n]; n++) {
                if (escaped)
                        escaped = false;
                else if (s[n] == '\\')
                        escaped = true;
                else if (strchr(reject, s[n]))
                        break;
        }

        /* if s ends in \, return index of previous char */
        return n - escaped;
}

/* Split a string into words. */
const char* split(const char **state, size_t *l, const char *separator, bool quoted) {
        const char *current;

        current = *state;

        if (!*current) {
                assert(**state == '\0');
                return NULL;
        }

        current += strspn(current, separator);
        if (!*current) {
                *state = current;
                return NULL;
        }

        if (quoted && strchr("\'\"", *current)) {
                char quotechars[2] = {*current, '\0'};

                *l = strcspn_escaped(current + 1, quotechars);
                if (current[*l + 1] == '\0' || current[*l + 1] != quotechars[0] ||
                    (current[*l + 2] && !strchr(separator, current[*l + 2]))) {
                        /* right quote missing or garbage at the end */
                        *state = current;
                        return NULL;
                }
                *state = current++ + *l + 2;
        } else if (quoted) {
                *l = strcspn_escaped(current, separator);
                if (current[*l] && !strchr(separator, current[*l])) {
                        /* unfinished escape */
                        *state = current;
                        return NULL;
                }
                *state = current + *l;
        } else {
                *l = strcspn(current, separator);
                *state = current + *l;
        }

        return current;
}

char *truncate_nl(char *s) {
        assert(s);

        s[strcspn(s, NEWLINE)] = 0;
        return s;
}

char *strnappend(const char *s, const char *suffix, size_t b) {
        size_t a;
        char *r;

        if (!s && !suffix)
                return strdup("");

        if (!s)
                return strndup(suffix, b);

        if (!suffix)
                return strdup(s);

        assert(s);
        assert(suffix);

        a = strlen(s);
        if (b > ((size_t) -1) - a)
                return NULL;

        r = new(char, a+b+1);
        if (!r)
                return NULL;

        memcpy(r, s, a);
        memcpy(r+a, suffix, b);
        r[a+b] = 0;

        return r;
}

char *strappend(const char *s, const char *suffix) {
        return strnappend(s, suffix, suffix ? strlen(suffix) : 0);
}

int rmdir_parents(const char *path, const char *stop) {
        size_t l;
        int r = 0;

        assert(path);
        assert(stop);

        l = strlen(path);

        /* Skip trailing slashes */
        while (l > 0 && path[l-1] == '/')
                l--;

        while (l > 0) {
                char *t;

                /* Skip last component */
                while (l > 0 && path[l-1] != '/')
                        l--;

                /* Skip trailing slashes */
                while (l > 0 && path[l-1] == '/')
                        l--;

                if (l <= 0)
                        break;

                if (!(t = strndup(path, l)))
                        return -ENOMEM;

                if (path_startswith(stop, t)) {
                        free(t);
                        return 0;
                }

                r = rmdir(t);
                free(t);

                if (r < 0)
                        if (errno != ENOENT)
                                return -errno;
        }

        return 0;
}

char hexchar(int x) {
        static const char table[16] = "0123456789abcdef";

        return table[x & 15];
}

int unhexchar(char c) {

        if (c >= '0' && c <= '9')
                return c - '0';

        if (c >= 'a' && c <= 'f')
                return c - 'a' + 10;

        if (c >= 'A' && c <= 'F')
                return c - 'A' + 10;

        return -EINVAL;
}

char octchar(int x) {
        return '0' + (x & 7);
}

int unoctchar(char c) {

        if (c >= '0' && c <= '7')
                return c - '0';

        return -EINVAL;
}

char *cescape(const char *s) {
        char *r, *t;
        const char *f;

        assert(s);

        /* Does C style string escaping. May be reversed with
         * cunescape(). */

        r = new(char, strlen(s)*4 + 1);
        if (!r)
                return NULL;

        for (f = s, t = r; *f; f++)
                t += cescape_char(*f, t);

        *t = 0;

        return r;
}


static int cunescape_one(const char *p, size_t length, char *ret, uint32_t *ret_unicode) {
        int r = 1;

        assert(p);
        assert(*p);
        assert(ret);

        /* Unescapes C style. Returns the unescaped character in ret,
         * unless we encountered a \u sequence in which case the full
         * unicode character is returned in ret_unicode, instead. */

        if (length != (size_t) -1 && length < 1)
                return -EINVAL;

        switch (p[0]) {

        case 'a':
                *ret = '\a';
                break;
        case 'b':
                *ret = '\b';
                break;
        case 'f':
                *ret = '\f';
                break;
        case 'n':
                *ret = '\n';
                break;
        case 'r':
                *ret = '\r';
                break;
        case 't':
                *ret = '\t';
                break;
        case 'v':
                *ret = '\v';
                break;
        case '\\':
                *ret = '\\';
                break;
        case '"':
                *ret = '"';
                break;
        case '\'':
                *ret = '\'';
                break;

        case 's':
                /* This is an extension of the XDG syntax files */
                *ret = ' ';
                break;

        case 'x': {
                /* hexadecimal encoding */
                int a, b;

                if (length != (size_t) -1 && length < 3)
                        return -EINVAL;

                a = unhexchar(p[1]);
                if (a < 0)
                        return -EINVAL;

                b = unhexchar(p[2]);
                if (b < 0)
                        return -EINVAL;

                /* Don't allow NUL bytes */
                if (a == 0 && b == 0)
                        return -EINVAL;

                *ret = (char) ((a << 4U) | b);
                r = 3;
                break;
        }

        case 'u': {
                /* C++11 style 16bit unicode */

                int a[4];
                unsigned i;
                uint32_t c;

                if (length != (size_t) -1 && length < 5)
                        return -EINVAL;

                for (i = 0; i < 4; i++) {
                        a[i] = unhexchar(p[1 + i]);
                        if (a[i] < 0)
                                return a[i];
                }

                c = ((uint32_t) a[0] << 12U) | ((uint32_t) a[1] << 8U) | ((uint32_t) a[2] << 4U) | (uint32_t) a[3];

                /* Don't allow 0 chars */
                if (c == 0)
                        return -EINVAL;

                if (c < 128)
                        *ret = c;
                else {
                        if (!ret_unicode)
                                return -EINVAL;

                        *ret = 0;
                        *ret_unicode = c;
                }

                r = 5;
                break;
        }

        case 'U': {
                /* C++11 style 32bit unicode */

                int a[8];
                unsigned i;
                uint32_t c;

                if (length != (size_t) -1 && length < 9)
                        return -EINVAL;

                for (i = 0; i < 8; i++) {
                        a[i] = unhexchar(p[1 + i]);
                        if (a[i] < 0)
                                return a[i];
                }

                c = ((uint32_t) a[0] << 28U) | ((uint32_t) a[1] << 24U) | ((uint32_t) a[2] << 20U) | ((uint32_t) a[3] << 16U) |
                    ((uint32_t) a[4] << 12U) | ((uint32_t) a[5] <<  8U) | ((uint32_t) a[6] <<  4U) |  (uint32_t) a[7];

                /* Don't allow 0 chars */
                if (c == 0)
                        return -EINVAL;

                /* Don't allow invalid code points */
                if (!unichar_is_valid(c))
                        return -EINVAL;

                if (c < 128)
                        *ret = c;
                else {
                        if (!ret_unicode)
                                return -EINVAL;

                        *ret = 0;
                        *ret_unicode = c;
                }

                r = 9;
                break;
        }

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7': {
                /* octal encoding */
                int a, b, c;
                uint32_t m;

                if (length != (size_t) -1 && length < 4)
                        return -EINVAL;

                a = unoctchar(p[0]);
                if (a < 0)
                        return -EINVAL;

                b = unoctchar(p[1]);
                if (b < 0)
                        return -EINVAL;

                c = unoctchar(p[2]);
                if (c < 0)
                        return -EINVAL;

                /* don't allow NUL bytes */
                if (a == 0 && b == 0 && c == 0)
                        return -EINVAL;

                /* Don't allow bytes above 255 */
                m = ((uint32_t) a << 6U) | ((uint32_t) b << 3U) | (uint32_t) c;
                if (m > 255)
                        return -EINVAL;

                *ret = m;
                r = 3;
                break;
        }

        default:
                return -EINVAL;
        }

        return r;
}

char *xescape(const char *s, const char *bad) {
        char *r, *t;
        const char *f;

        /* Escapes all chars in bad, in addition to \ and all special
         * chars, in \xFF style escaping. May be reversed with
         * cunescape(). */

        r = new(char, strlen(s) * 4 + 1);
        if (!r)
                return NULL;

        for (f = s, t = r; *f; f++) {

                if ((*f < ' ') || (*f >= 127) ||
                    (*f == '\\') || strchr(bad, *f)) {
                        *(t++) = '\\';
                        *(t++) = 'x';
                        *(t++) = hexchar(*f >> 4);
                        *(t++) = hexchar(*f);
                } else
                        *(t++) = *f;
        }

        *t = 0;

        return r;
}

_pure_ static bool hidden_file_allow_backup(const char *filename) {
        assert(filename);

        return
                filename[0] == '.' ||
                streq(filename, "lost+found") ||
                streq(filename, "aquota.user") ||
                streq(filename, "aquota.group") ||
                endswith(filename, ".rpmnew") ||
                endswith(filename, ".rpmsave") ||
                endswith(filename, ".rpmorig") ||
                endswith(filename, ".dpkg-old") ||
                endswith(filename, ".dpkg-new") ||
                endswith(filename, ".dpkg-tmp") ||
                endswith(filename, ".dpkg-dist") ||
                endswith(filename, ".dpkg-bak") ||
                endswith(filename, ".dpkg-backup") ||
                endswith(filename, ".dpkg-remove") ||
                endswith(filename, ".swp");
}

bool hidden_file(const char *filename) {
        assert(filename);

        if (endswith(filename, "~"))
                return true;

        return hidden_file_allow_backup(filename);
}

int flush_fd(int fd) {
        struct pollfd pollfd = {
                .fd = fd,
                .events = POLLIN,
        };

        for (;;) {
                char buf[LINE_MAX];
                ssize_t l;
                int r;

                r = poll(&pollfd, 1, 0);
                if (r < 0) {
                        if (errno == EINTR)
                                continue;

                        return -errno;

                } else if (r == 0)
                        return 0;

                l = read(fd, buf, sizeof(buf));
                if (l < 0) {

                        if (errno == EINTR)
                                continue;

                        if (errno == EAGAIN)
                                return 0;

                        return -errno;
                } else if (l == 0)
                        return 0;
        }
}

ssize_t loop_read(int fd, void *buf, size_t nbytes, bool do_poll) {
        uint8_t *p = buf;
        ssize_t n = 0;

        assert(fd >= 0);
        assert(buf);

        while (nbytes > 0) {
                ssize_t k;

                k = read(fd, p, nbytes);
                if (k < 0) {
                        if (errno == EINTR)
                                continue;

                        if (errno == EAGAIN && do_poll) {

                                /* We knowingly ignore any return value here,
                                 * and expect that any error/EOF is reported
                                 * via read() */

                                fd_wait_for_event(fd, POLLIN, USEC_INFINITY);
                                continue;
                        }

                        return n > 0 ? n : -errno;
                }

                if (k == 0)
                        return n;

                p += k;
                nbytes -= k;
                n += k;
        }

        return n;
}

int loop_read_exact(int fd, void *buf, size_t nbytes, bool do_poll) {
        ssize_t n;

        n = loop_read(fd, buf, nbytes, do_poll);
        if (n < 0)
                return n;
        if ((size_t) n != nbytes)
                return -EIO;
        return 0;
}

int loop_write(int fd, const void *buf, size_t nbytes, bool do_poll) {
        const uint8_t *p = buf;

        assert(fd >= 0);
        assert(buf);

        errno = 0;

        do {
                ssize_t k;

                k = write(fd, p, nbytes);
                if (k < 0) {
                        if (errno == EINTR)
                                continue;

                        if (errno == EAGAIN && do_poll) {
                                /* We knowingly ignore any return value here,
                                 * and expect that any error/EOF is reported
                                 * via write() */

                                fd_wait_for_event(fd, POLLOUT, USEC_INFINITY);
                                continue;
                        }

                        return -errno;
                }

                if (nbytes > 0 && k == 0) /* Can't really happen */
                        return -EIO;

                p += k;
                nbytes -= k;
        } while (nbytes > 0);

        return 0;
}

char* dirname_malloc(const char *path) {
        char *d, *dir, *dir2;

        d = strdup(path);
        if (!d)
                return NULL;
        dir = dirname(d);
        assert(dir);

        if (dir != d) {
                dir2 = strdup(dir);
                free(d);
                return dir2;
        }

        return dir;
}

_pure_ static int is_temporary_fs(struct statfs *s) {
        assert(s);

        return F_TYPE_EQUAL(s->f_type, TMPFS_MAGIC) ||
               F_TYPE_EQUAL(s->f_type, RAMFS_MAGIC);
}

int chmod_and_chown(const char *path, mode_t mode, uid_t uid, gid_t gid) {
        assert(path);

        /* Under the assumption that we are running privileged we
         * first change the access mode and only then hand out
         * ownership to avoid a window where access is too open. */

        if (mode != MODE_INVALID)
                if (chmod(path, mode) < 0)
                        return -errno;

        if (uid != UID_INVALID || gid != GID_INVALID)
                if (chown(path, uid, gid) < 0)
                        return -errno;

        return 0;
}

int touch_file(const char *path, bool parents, usec_t stamp, uid_t uid, gid_t gid, mode_t mode) {
        _cleanup_close_ int fd;
        int r;

        assert(path);

        if (parents)
                mkdir_parents(path, 0755);

        fd = open(path, O_WRONLY|O_CREAT|O_CLOEXEC|O_NOCTTY, mode > 0 ? mode : 0644);
        if (fd < 0)
                return -errno;

        if (mode > 0) {
                r = fchmod(fd, mode);
                if (r < 0)
                        return -errno;
        }

        if (uid != UID_INVALID || gid != GID_INVALID) {
                r = fchown(fd, uid, gid);
                if (r < 0)
                        return -errno;
        }

        if (stamp != USEC_INFINITY) {
                struct timespec ts[2];

                timespec_store(&ts[0], stamp);
                ts[1] = ts[0];
                r = futimens(fd, ts);
        } else
                r = futimens(fd, NULL);
        if (r < 0)
                return -errno;

        return 0;
}

int touch(const char *path) {
        return touch_file(path, false, USEC_INFINITY, UID_INVALID, GID_INVALID, 0);
}

bool null_or_empty(struct stat *st) {
        assert(st);

        if (S_ISREG(st->st_mode) && st->st_size <= 0)
                return true;

        if (S_ISCHR(st->st_mode) || S_ISBLK(st->st_mode))
                return true;

        return false;
}

int null_or_empty_path(const char *fn) {
        struct stat st;

        assert(fn);

        if (stat(fn, &st) < 0)
                return -errno;

        return null_or_empty(&st);
}

int null_or_empty_fd(int fd) {
        struct stat st;

        assert(fd >= 0);

        if (fstat(fd, &st) < 0)
                return -errno;

        return null_or_empty(&st);
}

bool dirent_is_file_with_suffix(const struct dirent *de, const char *suffix) {
        assert(de);

        if (de->d_type != DT_REG &&
            de->d_type != DT_LNK &&
            de->d_type != DT_UNKNOWN)
                return false;

        if (hidden_file_allow_backup(de->d_name))
                return false;

        return endswith(de->d_name, suffix);
}

bool nulstr_contains(const char*nulstr, const char *needle) {
        const char *i;

        if (!nulstr)
                return false;

        NULSTR_FOREACH(i, nulstr)
                if (streq(i, needle))
                        return true;

        return false;
}


static inline int ppoll_fallback(struct pollfd *fds, nfds_t nfds, const struct timespec *timeout_ts, const sigset_t *sigmask) {
        int ready, timeout;
        sigset_t origmask;

        timeout = (timeout_ts == NULL) ? -1 : (timeout_ts->tv_sec * 1000 + timeout_ts->tv_nsec / 1000000);

        /* This is racey, but what can we do without ppoll? */
        sigprocmask(SIG_SETMASK, sigmask, &origmask);
        ready = poll(fds, nfds, timeout);
        sigprocmask(SIG_SETMASK, &origmask, NULL);

	return ready;
}

int fd_wait_for_event(int fd, int event, usec_t t) {

        struct pollfd pollfd = {
                .fd = fd,
                .events = event,
        };

        struct timespec ts;
        int r;

#if HAVE_DECL_PPOLL
        r = ppoll(&pollfd, 1, t == USEC_INFINITY ? NULL : timespec_store(&ts, t), NULL);
#else
        /* Fallback path when ppoll() is unavailable */
        r = ppoll_fallback(&pollfd, 1, t == USEC_INFINITY ? NULL : timespec_store(&ts, t), NULL);
#endif
        if (r < 0)
                return -errno;

        if (r == 0)
                return 0;

        return pollfd.revents;
}

int fopen_temporary(const char *path, FILE **_f, char **_temp_path) {
        FILE *f;
        char *t;
        int r, fd;

        assert(path);
        assert(_f);
        assert(_temp_path);

        r = tempfn_xxxxxx(path, &t);
        if (r < 0)
                return r;

#if HAVE_DECL_MKOSTEMP
        fd = mkostemp_safe(t, O_WRONLY|O_CLOEXEC);
#else
        fd = mkstemp_safe(t);
        fcntl(fd, F_SETFD, FD_CLOEXEC);
#endif
        if (fd < 0) {
                free(t);
                return -errno;
        }

        f = fdopen(fd, "we");
        if (!f) {
                unlink(t);
                free(t);
                return -errno;
        }

        *_f = f;
        *_temp_path = t;

        return 0;
}

int get_user_creds(
                const char **username,
                uid_t *uid, gid_t *gid,
                const char **home,
                const char **shell) {

        struct passwd *p;
        uid_t u;

        assert(username);
        assert(*username);

        /* We enforce some special rules for uid=0: in order to avoid
         * NSS lookups for root we hardcode its data. */

        if (streq(*username, "root") || streq(*username, "0")) {
                *username = "root";

                if (uid)
                        *uid = 0;

                if (gid)
                        *gid = 0;

                if (home)
                        *home = "/root";

                if (shell)
                        *shell = "/bin/sh";

                return 0;
        }

        if (parse_uid(*username, &u) >= 0) {
                errno = 0;
                p = getpwuid(u);

                /* If there are multiple users with the same id, make
                 * sure to leave $USER to the configured value instead
                 * of the first occurrence in the database. However if
                 * the uid was configured by a numeric uid, then let's
                 * pick the real username from /etc/passwd. */
                if (p)
                        *username = p->pw_name;
        } else {
                errno = 0;
                p = getpwnam(*username);
        }

        if (!p)
                return errno > 0 ? -errno : -ESRCH;

        if (uid)
                *uid = p->pw_uid;

        if (gid)
                *gid = p->pw_gid;

        if (home)
                *home = p->pw_dir;

        if (shell)
                *shell = p->pw_shell;

        return 0;
}

int get_group_creds(const char **groupname, gid_t *gid) {
        struct group *g;
        gid_t id;

        assert(groupname);

        /* We enforce some special rules for gid=0: in order to avoid
         * NSS lookups for root we hardcode its data. */

        if (streq(*groupname, "root") || streq(*groupname, "0")) {
                *groupname = "root";

                if (gid)
                        *gid = 0;

                return 0;
        }

        if (parse_gid(*groupname, &id) >= 0) {
                errno = 0;
                g = getgrgid(id);

                if (g)
                        *groupname = g->gr_name;
        } else {
                errno = 0;
                g = getgrnam(*groupname);
        }

        if (!g)
                return errno > 0 ? -errno : -ESRCH;

        if (gid)
                *gid = g->gr_gid;

        return 0;
}

char *strjoin(const char *x, ...) {
        va_list ap;
        size_t l;
        char *r, *p;

        va_start(ap, x);

        if (x) {
                l = strlen(x);

                for (;;) {
                        const char *t;
                        size_t n;

                        t = va_arg(ap, const char *);
                        if (!t)
                                break;

                        n = strlen(t);
                        if (n > ((size_t) -1) - l) {
                                va_end(ap);
                                return NULL;
                        }

                        l += n;
                }
        } else
                l = 0;

        va_end(ap);

        r = new(char, l+1);
        if (!r)
                return NULL;

        if (x) {
                p = stpcpy(r, x);

                va_start(ap, x);

                for (;;) {
                        const char *t;

                        t = va_arg(ap, const char *);
                        if (!t)
                                break;

                        p = stpcpy(p, t);
                }

                va_end(ap);
        } else
                r[0] = 0;

        return r;
}

bool is_main_thread(void) {
        static thread_local int cached = 0;

        if (_unlikely_(cached == 0))
                cached = getpid() == gettid() ? 1 : -1;

        return cached > 0;
}

static const char *const ioprio_class_table[] = {
        [IOPRIO_CLASS_NONE] = "none",
        [IOPRIO_CLASS_RT] = "realtime",
        [IOPRIO_CLASS_BE] = "best-effort",
        [IOPRIO_CLASS_IDLE] = "idle"
};

DEFINE_STRING_TABLE_LOOKUP_WITH_FALLBACK(ioprio_class, int, INT_MAX);

static const char *const sigchld_code_table[] = {
        [CLD_EXITED] = "exited",
        [CLD_KILLED] = "killed",
        [CLD_DUMPED] = "dumped",
        [CLD_TRAPPED] = "trapped",
        [CLD_STOPPED] = "stopped",
        [CLD_CONTINUED] = "continued",
};

DEFINE_STRING_TABLE_LOOKUP(sigchld_code, int);

static const char *const log_facility_unshifted_table[LOG_NFACILITIES] = {
        [LOG_FAC(LOG_KERN)] = "kern",
        [LOG_FAC(LOG_USER)] = "user",
        [LOG_FAC(LOG_MAIL)] = "mail",
        [LOG_FAC(LOG_DAEMON)] = "daemon",
        [LOG_FAC(LOG_AUTH)] = "auth",
        [LOG_FAC(LOG_SYSLOG)] = "syslog",
        [LOG_FAC(LOG_LPR)] = "lpr",
        [LOG_FAC(LOG_NEWS)] = "news",
        [LOG_FAC(LOG_UUCP)] = "uucp",
        [LOG_FAC(LOG_CRON)] = "cron",
        [LOG_FAC(LOG_AUTHPRIV)] = "authpriv",
        [LOG_FAC(LOG_FTP)] = "ftp",
        [LOG_FAC(LOG_LOCAL0)] = "local0",
        [LOG_FAC(LOG_LOCAL1)] = "local1",
        [LOG_FAC(LOG_LOCAL2)] = "local2",
        [LOG_FAC(LOG_LOCAL3)] = "local3",
        [LOG_FAC(LOG_LOCAL4)] = "local4",
        [LOG_FAC(LOG_LOCAL5)] = "local5",
        [LOG_FAC(LOG_LOCAL6)] = "local6",
        [LOG_FAC(LOG_LOCAL7)] = "local7"
};

DEFINE_STRING_TABLE_LOOKUP_WITH_FALLBACK(log_facility_unshifted, int, LOG_FAC(~0));

static const char *const log_level_table[] = {
        [LOG_EMERG] = "emerg",
        [LOG_ALERT] = "alert",
        [LOG_CRIT] = "crit",
        [LOG_ERR] = "err",
        [LOG_WARNING] = "warning",
        [LOG_NOTICE] = "notice",
        [LOG_INFO] = "info",
        [LOG_DEBUG] = "debug"
};

DEFINE_STRING_TABLE_LOOKUP_WITH_FALLBACK(log_level, int, LOG_DEBUG);

static const char* const sched_policy_table[] = {
        [SCHED_OTHER] = "other",
        [SCHED_BATCH] = "batch",
        [SCHED_IDLE] = "idle",
        [SCHED_FIFO] = "fifo",
        [SCHED_RR] = "rr"
};

DEFINE_STRING_TABLE_LOOKUP_WITH_FALLBACK(sched_policy, int, INT_MAX);

static const char* const rlimit_table[_RLIMIT_MAX] = {
        [RLIMIT_CPU] = "LimitCPU",
        [RLIMIT_FSIZE] = "LimitFSIZE",
        [RLIMIT_DATA] = "LimitDATA",
        [RLIMIT_STACK] = "LimitSTACK",
        [RLIMIT_CORE] = "LimitCORE",
        [RLIMIT_RSS] = "LimitRSS",
        [RLIMIT_NOFILE] = "LimitNOFILE",
        [RLIMIT_AS] = "LimitAS",
        [RLIMIT_NPROC] = "LimitNPROC",
        [RLIMIT_MEMLOCK] = "LimitMEMLOCK",
        [RLIMIT_LOCKS] = "LimitLOCKS",
        [RLIMIT_SIGPENDING] = "LimitSIGPENDING",
        [RLIMIT_MSGQUEUE] = "LimitMSGQUEUE",
        [RLIMIT_NICE] = "LimitNICE",
        [RLIMIT_RTPRIO] = "LimitRTPRIO",
        [RLIMIT_RTTIME] = "LimitRTTIME"
};

DEFINE_STRING_TABLE_LOOKUP(rlimit, int);

static const char* const ip_tos_table[] = {
        [IPTOS_LOWDELAY] = "low-delay",
        [IPTOS_THROUGHPUT] = "throughput",
        [IPTOS_RELIABILITY] = "reliability",
        [IPTOS_LOWCOST] = "low-cost",
};

DEFINE_STRING_TABLE_LOOKUP_WITH_FALLBACK(ip_tos, int, 0xff);

static const char *const __signal_table[] = {
        [SIGHUP] = "HUP",
        [SIGINT] = "INT",
        [SIGQUIT] = "QUIT",
        [SIGILL] = "ILL",
        [SIGTRAP] = "TRAP",
        [SIGABRT] = "ABRT",
        [SIGBUS] = "BUS",
        [SIGFPE] = "FPE",
        [SIGKILL] = "KILL",
        [SIGUSR1] = "USR1",
        [SIGSEGV] = "SEGV",
        [SIGUSR2] = "USR2",
        [SIGPIPE] = "PIPE",
        [SIGALRM] = "ALRM",
        [SIGTERM] = "TERM",
#ifdef SIGSTKFLT
        [SIGSTKFLT] = "STKFLT",  /* Linux on SPARC doesn't know SIGSTKFLT */
#endif
        [SIGCHLD] = "CHLD",
        [SIGCONT] = "CONT",
        [SIGSTOP] = "STOP",
        [SIGTSTP] = "TSTP",
        [SIGTTIN] = "TTIN",
        [SIGTTOU] = "TTOU",
        [SIGURG] = "URG",
        [SIGXCPU] = "XCPU",
        [SIGXFSZ] = "XFSZ",
        [SIGVTALRM] = "VTALRM",
        [SIGPROF] = "PROF",
        [SIGWINCH] = "WINCH",
        [SIGIO] = "IO",
        [SIGPWR] = "PWR",
        [SIGSYS] = "SYS"
};

DEFINE_PRIVATE_STRING_TABLE_LOOKUP(__signal, int);

const char *signal_to_string(int signo) {
        static thread_local char buf[sizeof("RTMIN+")-1 + DECIMAL_STR_MAX(int) + 1];
        const char *name;

        name = __signal_to_string(signo);
        if (name)
                return name;

        if (signo >= SIGRTMIN && signo <= SIGRTMAX)
                snprintf(buf, sizeof(buf), "RTMIN+%d", signo - SIGRTMIN);
        else
                snprintf(buf, sizeof(buf), "%d", signo);

        return buf;
}

int fd_inc_sndbuf(int fd, size_t n) {
        int r, value;
        socklen_t l = sizeof(value);

        r = getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &value, &l);
        if (r >= 0 && l == sizeof(value) && (size_t) value >= n*2)
                return 0;

        /* If we have the privileges we will ignore the kernel limit. */

        value = (int) n;
        if (setsockopt(fd, SOL_SOCKET, SO_SNDBUFFORCE, &value, sizeof(value)) < 0)
                if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &value, sizeof(value)) < 0)
                        return -errno;

        return 1;
}

bool in_initrd(void) {
        static int saved = -1;
        struct statfs s;

        if (saved >= 0)
                return saved;

        /* We make two checks here:
         *
         * 1. the flag file /etc/initrd-release must exist
         * 2. the root file system must be a memory file system
         *
         * The second check is extra paranoia, since misdetecting an
         * initrd can have bad bad consequences due the initrd
         * emptying when transititioning to the main systemd.
         */

        saved = access("/etc/initrd-release", F_OK) >= 0 &&
                statfs("/", &s) >= 0 &&
                is_temporary_fs(&s);

        return saved;
}

bool filename_is_valid(const char *p) {

        if (isempty(p))
                return false;

        if (strchr(p, '/'))
                return false;

        if (streq(p, "."))
                return false;

        if (streq(p, ".."))
                return false;

        if (strlen(p) > FILENAME_MAX)
                return false;

        return true;
}

/* hey glibc, APIs with callbacks without a user pointer are so useless */
void *xbsearch_r(const void *key, const void *base, size_t nmemb, size_t size,
                 int (*compar) (const void *, const void *, void *), void *arg) {
        size_t l, u, idx;
        const void *p;
        int comparison;

        l = 0;
        u = nmemb;
        while (l < u) {
                idx = (l + u) / 2;
                p = (void *)(((const char *) base) + (idx * size));
                comparison = compar(key, p, arg);
                if (comparison < 0)
                        u = idx;
                else if (comparison > 0)
                        l = idx + 1;
                else
                        return (void *)p;
        }
        return NULL;
}

void* greedy_realloc(void **p, size_t *allocated, size_t need, size_t size) {
        size_t a, newalloc;
        void *q;

        assert(p);
        assert(allocated);

        if (*allocated >= need)
                return *p;

        newalloc = MAX(need * 2, 64u / size);
        a = newalloc * size;

        /* check for overflows */
        if (a < size * need)
                return NULL;

        q = realloc(*p, a);
        if (!q)
                return NULL;

        *p = q;
        *allocated = newalloc;
        return q;
}

int proc_cmdline(char **ret) {
        assert(ret);

        if (detect_container(NULL) > 0)
                return get_process_cmdline(1, 0, false, ret);
        else
                return read_one_line_file("/proc/cmdline", ret);
}

int parse_proc_cmdline(int (*parse_item)(const char *key, const char *value)) {
        _cleanup_free_ char *line = NULL;
        const char *p;
        int r;

        assert(parse_item);

        r = proc_cmdline(&line);
        if (r < 0)
                return r;

        p = line;
        for (;;) {
                _cleanup_free_ char *word = NULL;
                char *value = NULL;

                r = unquote_first_word(&p, &word, UNQUOTE_RELAX);
                if (r < 0)
                        return r;
                if (r == 0)
                        break;

                /* Filter out arguments that are intended only for the
                 * initrd */
                if (!in_initrd() && startswith(word, "rd."))
                        continue;

                value = strchr(word, '=');
                if (value)
                        *(value++) = 0;

                r = parse_item(word, value);
                if (r < 0)
                        return r;
        }

        return 0;
}

int getpeercred(int fd, struct ucred *ucred) {
        socklen_t n = sizeof(struct ucred);
        struct ucred u;
        int r;

        assert(fd >= 0);
        assert(ucred);

        r = getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &u, &n);
        if (r < 0)
                return -errno;

        if (n != sizeof(struct ucred))
                return -EIO;

        /* Check if the data is actually useful and not suppressed due
         * to namespacing issues */
        if (u.pid <= 0)
                return -ENODATA;
        if (u.uid == UID_INVALID)
                return -ENODATA;
        if (u.gid == GID_INVALID)
                return -ENODATA;

        *ucred = u;
        return 0;
}

#if HAVE_DECL_MKOSTEMP
/* This is much like like mkostemp() but is subject to umask(). */
int mkostemp_safe(char *pattern, int flags) {
        _cleanup_umask_ mode_t u;
        int fd;

        assert(pattern);

        u = umask(077);

        fd = mkostemp(pattern, flags);
        if (fd < 0)
                return -errno;

        return fd;
}
#else
/* This is much like like mkstemp() but is subject to umask(). */
int mkstemp_safe(char *pattern) {
        _cleanup_umask_ mode_t u;
        int fd;

        assert(pattern);

        u = umask(077);

        fd = mkstemp(pattern);
        if (fd < 0)
                return -errno;

        return fd;
}
#endif

int tempfn_xxxxxx(const char *p, char **ret) {
        const char *fn;
        char *t;

        assert(p);
        assert(ret);

        /*
         * Turns this:
         *         /foo/bar/waldo
         *
         * Into this:
         *         /foo/bar/.#waldoXXXXXX
         */

        fn = basename(p);
        if (!filename_is_valid(fn))
                return -EINVAL;

        t = new(char, strlen(p) + 2 + 6 + 1);
        if (!t)
                return -ENOMEM;

        strcpy(stpcpy(stpcpy(mempcpy(t, p, fn - p), ".#"), fn), "XXXXXX");

        *ret = path_kill_slashes(t);
        return 0;
}

int is_dir(const char* path, bool follow) {
        struct stat st;
        int r;

        if (follow)
                r = stat(path, &st);
        else
                r = lstat(path, &st);
        if (r < 0)
                return -errno;

        return !!S_ISDIR(st.st_mode);
}

int unquote_first_word(const char **p, char **ret, UnquoteFlags flags) {
        _cleanup_free_ char *s = NULL;
        size_t allocated = 0, sz = 0;
        int r;

        enum {
                START,
                VALUE,
                VALUE_ESCAPE,
                SINGLE_QUOTE,
                SINGLE_QUOTE_ESCAPE,
                DOUBLE_QUOTE,
                DOUBLE_QUOTE_ESCAPE,
                SPACE,
        } state = START;

        assert(p);
        assert(*p);
        assert(ret);

        /* Parses the first word of a string, and returns it in
         * *ret. Removes all quotes in the process. When parsing fails
         * (because of an uneven number of quotes or similar), leaves
         * the pointer *p at the first invalid character. */

        for (;;) {
                char c = **p;

                switch (state) {

                case START:
                        if (c == 0)
                                goto finish;
                        else if (strchr(WHITESPACE, c))
                                break;

                        state = VALUE;
                        /* fallthrough */

                case VALUE:
                        if (c == 0)
                                goto finish;
                        else if (c == '\'')
                                state = SINGLE_QUOTE;
                        else if (c == '\\')
                                state = VALUE_ESCAPE;
                        else if (c == '\"')
                                state = DOUBLE_QUOTE;
                        else if (strchr(WHITESPACE, c))
                                state = SPACE;
                        else {
                                if (!GREEDY_REALLOC(s, allocated, sz+2))
                                        return -ENOMEM;

                                s[sz++] = c;
                        }

                        break;

                case VALUE_ESCAPE:
                        if (c == 0) {
                                if (flags & UNQUOTE_RELAX)
                                        goto finish;
                                return -EINVAL;
                        }

                        if (!GREEDY_REALLOC(s, allocated, sz+7))
                                return -ENOMEM;

                        if (flags & UNQUOTE_CUNESCAPE) {
                                uint32_t u;

                                r = cunescape_one(*p, (size_t) -1, &c, &u);
                                if (r < 0)
                                        return -EINVAL;

                                (*p) += r - 1;

                                if (c != 0)
                                        s[sz++] = c; /* normal explicit char */
                                else
                                        sz += utf8_encode_unichar(s + sz, u); /* unicode chars we'll encode as utf8 */
                        } else
                                s[sz++] = c;

                        state = VALUE;
                        break;

                case SINGLE_QUOTE:
                        if (c == 0) {
                                if (flags & UNQUOTE_RELAX)
                                        goto finish;
                                return -EINVAL;
                        } else if (c == '\'')
                                state = VALUE;
                        else if (c == '\\')
                                state = SINGLE_QUOTE_ESCAPE;
                        else {
                                if (!GREEDY_REALLOC(s, allocated, sz+2))
                                        return -ENOMEM;

                                s[sz++] = c;
                        }

                        break;

                case SINGLE_QUOTE_ESCAPE:
                        if (c == 0) {
                                if (flags & UNQUOTE_RELAX)
                                        goto finish;
                                return -EINVAL;
                        }

                        if (!GREEDY_REALLOC(s, allocated, sz+7))
                                return -ENOMEM;

                        if (flags & UNQUOTE_CUNESCAPE) {
                                uint32_t u;

                                r = cunescape_one(*p, (size_t) -1, &c, &u);
                                if (r < 0)
                                        return -EINVAL;

                                (*p) += r - 1;

                                if (c != 0)
                                        s[sz++] = c;
                                else
                                        sz += utf8_encode_unichar(s + sz, u);
                        } else
                                s[sz++] = c;

                        state = SINGLE_QUOTE;
                        break;

                case DOUBLE_QUOTE:
                        if (c == 0)
                                return -EINVAL;
                        else if (c == '\"')
                                state = VALUE;
                        else if (c == '\\')
                                state = DOUBLE_QUOTE_ESCAPE;
                        else {
                                if (!GREEDY_REALLOC(s, allocated, sz+2))
                                        return -ENOMEM;

                                s[sz++] = c;
                        }

                        break;

                case DOUBLE_QUOTE_ESCAPE:
                        if (c == 0) {
                                if (flags & UNQUOTE_RELAX)
                                        goto finish;
                                return -EINVAL;
                        }

                        if (!GREEDY_REALLOC(s, allocated, sz+7))
                                return -ENOMEM;

                        if (flags & UNQUOTE_CUNESCAPE) {
                                uint32_t u;

                                r = cunescape_one(*p, (size_t) -1, &c, &u);
                                if (r < 0)
                                        return -EINVAL;

                                (*p) += r - 1;

                                if (c != 0)
                                        s[sz++] = c;
                                else
                                        sz += utf8_encode_unichar(s + sz, u);
                        } else
                                s[sz++] = c;

                        state = DOUBLE_QUOTE;
                        break;

                case SPACE:
                        if (c == 0)
                                goto finish;
                        if (!strchr(WHITESPACE, c))
                                goto finish;

                        break;
                }

                (*p) ++;
        }

finish:
        if (!s) {
                *ret = NULL;
                return 0;
        }

        s[sz] = 0;
        *ret = s;
        s = NULL;

        return 1;
}

void cmsg_close_all(struct msghdr *mh) {
        struct cmsghdr *cmsg;

        assert(mh);

        for (cmsg = CMSG_FIRSTHDR(mh); cmsg; cmsg = CMSG_NXTHDR(mh, cmsg))
                if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_RIGHTS)
                        close_many((int*) CMSG_DATA(cmsg), (cmsg->cmsg_len - CMSG_LEN(0)) / sizeof(int));
}
