#ifdef __CYGWIN__

#include <errno.h>

ssize_t
pread(int fd, void *p, size_t n, off_t off)
{
        off_t ooff;
        int oerrno;

        if ((ooff  = lseek(fd, off, SEEK_SET)) == -1)
                return -1;

        n = read(fd, p, n);

        oerrno = errno;
        lseek(fd, ooff, SEEK_SET);
        errno = oerrno;

        return n;
}

ssize_t
pwrite(int fd, const void *p, size_t n, off_t off)
{
        off_t ooff;
        int oerrno;

        if ((ooff  = lseek(fd, off, SEEK_SET)) == -1)
                return -1;

        n = write(fd, p, n);

        oerrno = errno;
        lseek(fd, ooff, SEEK_SET);
        errno = oerrno;

        return n;
}

#endif /* __CYGWIN__ */
