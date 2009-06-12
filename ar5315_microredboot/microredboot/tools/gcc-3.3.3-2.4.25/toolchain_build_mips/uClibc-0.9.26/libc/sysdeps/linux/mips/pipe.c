/* pipe system call for Linux/MIPS */

/*see uClibc's sh/pipe.c and glibc-2.2.4's mips/pipe.S */

#include <errno.h>
#include <unistd.h>
#include <syscall.h>

int pipe(int *fd)
{
    register long int res __asm__ ("$2"); // v0
    register long int res2 __asm__ ("$3"); // v1

    asm ("move\t$4,%2\n\t"		// $4 = a0
	 "syscall"		/* Perform the system call.  */
	 : "=r" (res)
	 : "0" (__NR_pipe), "r" (fd)
	 : "$4", "$7");

	fd[0] = res;
	fd[1] = res2;
	return(0);
}
