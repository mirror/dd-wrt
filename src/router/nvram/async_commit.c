#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <bcmnvram.h>

int main(int argc, char *argv[])
{

	/* 
	 * Run it under background 
	 */
	switch (fork()) {
	case -1:
		perror("fork failed");
		exit(1);
		break;
	case 0:
		/* 
		 * child process 
		 */
		_nvram_commit();
		exit(0);
		break;
	default:
		/* 
		 * parent process should just die 
		 */
		_exit(0);
	}
	return 0;
}
