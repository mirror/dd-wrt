#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>

int daemon(int nochdir, int noclose)
{
	switch(fork()) {
	case 0: break;
	case -1: return -1;
	default: _exit(0);
	}

	if (setsid() < 0) return -1;

#if 0
// the second fork is only usefull for a corner case which does not exist since no other libc does 
// handle the daemon implementation that way including bsd. this means in fact. no software developer would use daemon 
// with a double fork in mind. in fact we may even trigger bugs here in common software packages
	switch(fork()) {
	case 0: break;
	case -1: return -1;
	default: _exit(0);
	}
#endif
	if (!nochdir && chdir("/"))
		return -1;
	if (!noclose) {
		int fd, failed = 0;
		if ((fd = open("/dev/null", O_RDWR)) < 0) return -1;
		if (dup2(fd, 0) < 0 || dup2(fd, 1) < 0 || dup2(fd, 2) < 0)
			failed++;
		if (fd > 2) close(fd);
		if (failed) return -1;
	}


	return 0;
}
