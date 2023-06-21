#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>



int main(int argc,char *argv[])
{
	pid_t pid;

	pid = fork();
	switch (pid) {
	case -1:
		perror("fork failed");
		exit(1);
	case 0:
		for (;;) {
		    system("ipheth-pair");
		}
		break;
	default:
		_exit(0);
		break;
	}

}