#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <utils.h>
#include <wlutils.h>
#include <errno.h>
#include <bcmnvram.h>
#if !defined(HAVE_MICRO) || defined(HAVE_ADM5120) || defined(HAVE_WRK54G)

static void watchdog(void)
{
	int brand = getRouterBrand();
	int registered = -1;
	int radiostate0 = -1;
	int oldstate0 = -1;
	int radiostate1 = -1;
	int oldstate1 = -1;
	int counter = 0;
	int radioledinitcount = 0;
	int fd = open("/dev/misc/watchdog", O_WRONLY);
	if (fd == -1)
		fd = open("/dev/watchdog", O_WRONLY);

	if (fd == -1) {
		return;
	}
#ifdef HAVE_MADWIFI
	int cnt = getdevicecount();
#else
	int cnt = get_wl_instances();
#endif

	while (1) {
		write(fd, "\0", 1);
		fsync(fd);

#ifdef HAVE_REGISTER
		if (!nvram_match("flash_active", "1")) {
			if (registered == -1)
				registered = isregistered_real();
			if (!registered)
				isregistered();	//to poll
		}
#endif
		/* 
		 * software wlan led control 
		 */
		if (radioledinitcount < 5) {
			radioledinitcount++;
			oldstate0 = -1;
			oldstate1 = -1;
		}
#ifdef HAVE_MADWIFI
		if (!nvram_match("flash_active", "1")) {
			radiostate0 = get_radiostate("ath0");
			if (cnt == 2)
				radiostate1 = get_radiostate("ath1");
		}
#else
		wl_ioctl(get_wl_instance_name(0), WLC_GET_RADIO, &radiostate0, sizeof(int));
		if (cnt == 2)
			wl_ioctl(get_wl_instance_name(1), WLC_GET_RADIO, &radiostate1, sizeof(int));
#endif

		if (radiostate0 != oldstate0) {
#ifdef HAVE_MADWIFI
			if (radiostate0 == 1)
#else
			if ((radiostate0 & WL_RADIO_SW_DISABLE) == 0)
#endif
				led_control(LED_WLAN0, LED_ON);
			else {
				led_control(LED_WLAN0, LED_OFF);
#ifndef HAVE_MADWIFI
				/* 
				 * Disable wireless will cause diag led blink, so we want to
				 * stop it. 
				 */
				if (brand == ROUTER_WRT54G)
					diag_led(DIAG, STOP_LED);
				/* 
				 * Disable wireless will cause power led off, so we want to
				 * turn it on. 
				 */
				if (brand == ROUTER_WRT54G_V8)
					led_control(LED_POWER, LED_ON);
#endif
			}

			oldstate0 = radiostate0;
		}

		if (radiostate1 != oldstate1) {
#ifdef HAVE_MADWIFI
			if (radiostate1 == 1)
#else
			if ((radiostate1 & WL_RADIO_SW_DISABLE) == 0)
#endif
				led_control(LED_WLAN1, LED_ON);
			else {
				led_control(LED_WLAN1, LED_OFF);
			}

			oldstate1 = radiostate1;
		}
		/* 
		 * end software wlan led control 
		 */

		sleep(10);
		if (nvram_match("warn_enabled", "1")) {
			counter++;
			if (!(counter % 60))
				system("notifier&");	// 
		}
	}
}

int watchdog_main(int argc, char *argv[])
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
		watchdog();
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
#endif
