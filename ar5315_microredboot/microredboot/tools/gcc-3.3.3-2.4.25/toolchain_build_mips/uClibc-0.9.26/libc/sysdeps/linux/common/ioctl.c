#include <stdarg.h>
#include <sys/ioctl.h>

extern int __syscall_ioctl(int fd, int request, void *arg);

/* powerpc has its own special version... */
int ioctl(int fd, unsigned long int request, ...)
{
    void *arg;
    va_list list;

    va_start(list, request);
    arg = va_arg(list, void *);

    va_end(list);
    return __syscall_ioctl(fd, request, arg);
}

