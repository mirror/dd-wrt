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
	int radiostate2 = -1;
	int oldstate2 = -1;
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
		if (!nvram_match("flash_active", "1")) {

#ifndef HAVE_RT2880
#ifdef HAVE_REGISTER
			if (registered == -1)
				registered = isregistered_real();
			if (!registered)
				isregistered();	//to poll
#endif
			/* 
			 * software wlan led control 
			 */
			if (radioledinitcount < 5) {
				radioledinitcount++;
				oldstate0 = -1;
				oldstate1 = -1;
				oldstate2 = -1;
			}
#ifdef HAVE_MADWIFI
			radiostate0 = get_radiostate("ath0");
			if (cnt == 2)
				radiostate1 = get_radiostate("ath1");
#else
			wl_ioctl(get_wl_instance_name(0), WLC_GET_RADIO, &radiostate0, sizeof(int));
#ifndef HAVE_QTN
			if (cnt > 1)
				wl_ioctl(get_wl_instance_name(1), WLC_GET_RADIO, &radiostate1, sizeof(int));
			if (cnt > 2)
				wl_ioctl(get_wl_instance_name(2), WLC_GET_RADIO, &radiostate2, sizeof(int));
#endif
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

			if (radiostate2 != oldstate2) {
#ifdef HAVE_MADWIFI
				if (radiostate2 == 1)
#else
				if ((radiostate2 & WL_RADIO_SW_DISABLE) == 0)
#endif
					led_control(LED_WLAN2, LED_ON);
				else {
					led_control(LED_WLAN2, LED_OFF);
				}

				oldstate2 = radiostate2;
			}
			/* 
			 * end software wlan led control 
			 */
#endif
		}
#ifdef HAVE_MVEBU
		if (getRouterBrand() == ROUTER_WRT_1900AC) {
			int cpu;
			FILE *tempfp;
			tempfp = fopen("/sys/class/hwmon/hwmon1/temp1_input", "rb");
			if (tempfp) {
				fscanf(tempfp, "%d", &cpu);
				fclose(tempfp);
				if (cpu > ((atoi(nvram_safe_get("hwmon_temp_max")) + 10) * 1000)) {
					system("/bin/echo 255 > /sys/class/hwmon/hwmon0/pwm1");

				} else if (cpu > ((atoi(nvram_safe_get("hwmon_temp_max")) + 5) * 1000)) {
					system("/bin/echo 150 > /sys/class/hwmon/hwmon0/pwm1");

				} else if (cpu > ((atoi(nvram_safe_get("hwmon_temp_max"))) * 1000)) {
					system("/bin/echo 100 > /sys/class/hwmon/hwmon0/pwm1");

				} else if (cpu < ((atoi(nvram_safe_get("hwmon_temp_hyst"))) * 1000)) {
					system("/bin/echo 0 > /sys/class/hwmon/hwmon0/pwm1");

				}

			}

		}
#endif

		sleep(5);
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
