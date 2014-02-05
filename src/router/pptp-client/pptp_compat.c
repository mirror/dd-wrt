/* pptp_compat.c ... Compatibility functions
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#if defined (__SVR4) && defined (__sun) /* Solaris */
#include <stropts.h>
#endif
#include <strings.h>
#include "pptp_compat.h"
#include <stdio.h>
#include "util.h"

#if defined (__SVR4) && defined (__sun) /* Solaris */
/*
 * daemon implementation from uClibc
 */
int daemon(int nochdir, int noclose)
{
	int fd;

	switch (fork()) {
	case -1:
		return (-1);
	case 0:
		break;
	default:
		_exit(0);
	}

	if (setsid() == -1)
		return (-1);

	if (!nochdir)
		chdir("/");

	if (!noclose && (fd = open("/dev/null", O_RDWR, 0)) != -1) {
		dup2(fd, STDIN_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		if (fd > 2)
			close (fd);
	}
	return (0);
}

/*
 * openpty implementation based on pts(7D) example
 */ 
int openpty(int *amaster, int *aslave, char *name, struct termios *termp, struct winsize * winp) {
	int fdm,fds;
	char * slavename;

	/* open master */
	if ( (fdm = open("/dev/ptmx", O_RDWR)) == -1 ) 
		return -1;

	/* grant access to the slave pseudo-terminal device */
	if ( grantpt(fdm) == -1 )
		return -1;

	/* unlock a pseudo-terminal master/slave pair */
	if ( unlockpt(fdm) == -1 )
		return -1;

	/* get name of the slave pseudo-terminal device */
	if ( (slavename = ptsname(fdm)) == NULL ) 
		return -1;

	if ( (fds = open(slavename, O_RDWR)) == -1 ) {
		free(slavename);
		return -1;
	}

	ioctl(fds, I_PUSH, "ptem");       /* push ptem */
	ioctl(fds, I_PUSH, "ldterm");     /* push ldterm*/
	
	if ( name != NULL )
		strcpy(name,slavename);

	*amaster = fdm;
	*aslave = fds;

	free(slavename);
	return 0;
	
}
#endif /* Solaris */
