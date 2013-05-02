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
	int times = atoi(argv[1]);
	int type = 0;
	int count = 0;
	if (argc > 2)
		type = atoi(argv[2]);

	while (times > 0) {
		switch (type) {
		case 1:
			led_control(LED_CONNECTED, LED_ON);
			usleep(500000);
			led_control(LED_CONNECTED, LED_OFF);
			usleep(500000);
			break;
		case 2:	// aoss negotiation
			led_control(LED_SES, LED_ON);
			usleep(200000);
			led_control(LED_SES, LED_OFF);
			usleep(100000);
			led_control(LED_SES, LED_ON);
			usleep(200000);
			led_control(LED_SES, LED_OFF);
			usleep(500000);
			break;
		case 3:	// aoss error
			led_control(LED_SES, LED_ON);
			usleep(100000);
			led_control(LED_SES, LED_OFF);
			usleep(100000);
			led_control(LED_SES, LED_ON);
			usleep(100000);
			led_control(LED_SES, LED_OFF);
			usleep(100000);
			led_control(LED_SES, LED_ON);
			usleep(100000);
			led_control(LED_SES, LED_OFF);
			usleep(100000);
			led_control(LED_SES, LED_ON);
			usleep(100000);
			led_control(LED_SES, LED_OFF);
			usleep(100000);
			led_control(LED_SES, LED_ON);
			usleep(100000);
			led_control(LED_SES, LED_OFF);
			usleep(100000);
			break;

		default:
			led_control(LED_DIAG, LED_ON);
			usleep(500000);
			led_control(LED_DIAG, LED_OFF);
			usleep(500000);
			if (count && (count % 3) == 0)
				sleep(3);

			break;
		}
		times--;
		count++;
	}
	if (type == 3) {
		system("startservice ses_led_control");
	}

	return 0;
}				// end main
