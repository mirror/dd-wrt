#define _GNU_SOURCE
#include <unistd.h>
#include <signal.h>
#include "syscall.h"

pid_t vfork(void)
{
	/* vfork syscall cannot be made from C code */
///#ifdef SYS_fork
//	return syscall(SYS_fork);
//#else
	return syscall(SYS_clone, CLONE_VM|CLONE_VFORK|SIGCHLD, 0);
//#endif
}
