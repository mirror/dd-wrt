#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/errno.h>

#define __NR___libc_vfork __NR_vfork
inline	_syscall0(pid_t, __libc_vfork);
inline	_syscall0(pid_t, vfork);
