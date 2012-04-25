#include <signal.h>
#include <stdlib.h>



int main(int argc,char *argv[])
{
	pid_t pid;

	pid = fork();
	system("usbmuxd");
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