
#include <errno.h>
#include <asm/ptrace.h>
#include <sys/syscall.h>
#include <stdarg.h>

int
ptrace(int request, ... /* int pid, int addr, int data */)
{
	long ret;
	long res;
	int pid,addr,data;
	va_list ap;

	va_start(ap,request);
	pid  = va_arg(ap,int);
	addr = va_arg(ap,int);
	data = va_arg(ap,int);
	va_end(ap);

	if (request > 0 && request < 4) (long *)data = &ret;

	__asm__ volatile ("sub.l er0,er0\n\t"
                          "mov.b %1,r0l\n\t"
			  "mov.l %2,er1\n\t"
			  "mov.l %3,er2\n\t"
			  "mov.l %4,er3\n\t"
			  "mov.l %5,er4\n\t"
			  "trapa  #0\n\t"
			  "mov.l er0,%0"
		:"=g" (res)
		:"i" (__NR_ptrace), "g" (request), "g" (pid),
		 "g" (addr), "g" (data) : "er0", "er1", "er2", "er3", "er4");

	if (res >= 0) {
		if (request > 0 && request < 4) {
			__set_errno(0);
			return (ret);
		}
		return (int) res;
	}
	__set_errno(-res);
	return -1;
}
