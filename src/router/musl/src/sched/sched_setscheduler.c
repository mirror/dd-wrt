#include <sched.h>
#include "syscall.h"

int sched_setscheduler(pid_t pid, int sched, const struct sched_param *param)
{
	static const struct sched_param def;
	return syscall(SYS_sched_setscheduler, pid, 0, &def);
}
