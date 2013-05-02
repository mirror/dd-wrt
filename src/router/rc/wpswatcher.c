#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <wlutils.h>
#include <wlioctl.h>
#include <rc.h>

#include <cy_conf.h>
#include <bcmutils.h>
#include <utils.h>
#include <nvparse.h>

int main(int argc, char **argv)
{
	/* 
	 * Run it in the background 
	 */
	switch (fork()) {
	case -1:
		// can't fork
		exit(0);
		break;
	case 0:
		/* 
		 * child process 
		 */
		// fork ok
		(void)setsid();
		break;
	default:
		/* 
		 * parent process should just die 
		 */
		_exit(0);
	}
	int timeout = atoi(argv[1]);
	while (timeout) {
		FILE *fp = fopen("/tmp/.wpsdone", "rb");
		if (fp) {
			killall("ledtool", SIGKILL);
			nvram_set("wps_status", "1");
			nvram_commit();
			sysprintf("rm -f /tmp/.wpsdone");
			fclose(fp);
			led_control(LED_SES, LED_ON);
			break;
		}
		timeout--;
		sleep(1);
	}
	if (!timeout) {
		killall("ledtool", SIGKILL);
		nvram_set("wps_status", "1");
		nvram_commit();
		system("ledtool 1800 3");
	}
	return 0;
}				// end main
