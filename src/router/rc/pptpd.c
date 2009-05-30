#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <wait.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <utils.h>
#include <cy_conf.h>
#include <code_pattern.h>
#include <rc.h>

int pptpd_main(int argc, char **argv)
{

	if (!argv[1]) {
		fprintf(stderr, "usage: poptop [start|stop|restart]\n");
		return EINVAL;
	} else if (strstr(argv[1], "start"))
		start_service("pptpd");
	else if (strstr(argv[1], "stop"))
		stop_service("pptpd");
	else if (strstr(argv[1], "restart")) {
		startstop("pptpd");
		return 0;
	} else {
		fprintf(stderr, "usage: poptop [start|stop|restart]\n");
		return EINVAL;
	}
	return 0;
}
