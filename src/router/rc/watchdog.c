#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <utils.h>
#include <wlutils.h>
#include <errno.h>
#include <bcmnvram.h>
#include <shutils.h>

int isregistered_real(void);
int isregistered(void);
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
	int dropcounter = 0;
	static int lasttarget = 0;
	int radioledinitcount = 0;
	int fd = open("/dev/misc/watchdog", O_WRONLY);
	if (fd == -1)
		fd = open("/dev/watchdog", O_WRONLY);

#ifdef HAVE_MADWIFI
	int cnt = getdevicecount();
#else
	int cnt = get_wl_instances();
#endif

	while (1) {
		if (fd != -1) {
			write(fd, "\0", 1);
			fsync(fd);
		}
		if (!nvram_matchi("flash_active", 1)) {
#ifndef HAVE_RT2880
#ifdef HAVE_REGISTER
			if (registered == -1)
				registered = isregistered_real();
			if (!registered)
				isregistered(); //to poll
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
			radiostate0 = get_radiostate("wlan0");
			if (cnt == 2)
				radiostate1 = get_radiostate("wlan1");
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
		if (brand == ROUTER_WRT_1900AC) {
			int cpu;
			FILE *tempfp;
			tempfp = fopen("/sys/class/hwmon/hwmon0/temp1_input", "rb");
			if (tempfp) {
				fscanf(tempfp, "%d", &cpu);
				fclose(tempfp);
				int target = cpu - (nvram_geti("hwmon_temp_max") * 1000);
				if (target < 0)
					target = 0;
				if (target > 10000)
					target = 10000;
				target *= 255;
				target /= 10000;
				sysprintf("/bin/echo %d > /sys/class/hwmon/hwmon1/pwm1", target);
			}
		}
#endif
//#ifdef HAVE_USB
//#ifndef HAVE_3G_ONLY
//      if ((dropcounter++) % 4 == 0)
//              writeprocsys("vm/drop_caches", "3");    // flush fs cache
//#endif
//#endif
#ifdef HAVE_R9000
		int cpu = 0, wifi1 = 0, wifi2 = 0, wifi3_mac = 0, wifi3_phy = 0;
		FILE *tempfp;
		tempfp = fopen("/sys/class/hwmon/hwmon1/temp1_input", "rb");
		if (tempfp) {
			fscanf(tempfp, "%d", &cpu);
			fclose(tempfp);
		}
		cpu *= 1000;
		tempfp = fopen("/sys/class/hwmon/hwmon2/temp1_input", "rb");
		if (tempfp) {
			fscanf(tempfp, "%d", &wifi1);
			fclose(tempfp);
		}
		tempfp = fopen("/sys/class/hwmon/hwmon3/temp1_input", "rb");
		if (tempfp) {
			fscanf(tempfp, "%d", &wifi2);
			fclose(tempfp);
		}
		int dummy;
		if (!nvram_match("wlan2_net_mode", "disabled")) {
			FILE *check = fopen("/sys/kernel/debug/ieee80211/phy2/wil6210/temp", "rb");
			if (check) {
				fclose(check);

				tempfp = popen("cat /sys/kernel/debug/ieee80211/phy2/wil6210/temp | grep \"T_mac\" |cut -d = -f 2",
					       "rb");
				if (tempfp) {
					fscanf(tempfp, "%d.%d", &wifi3_mac, &dummy);
					pclose(tempfp);
				}
				tempfp =
					popen("cat /sys/kernel/debug/ieee80211/phy2/wil6210/temp | grep \"T_radio\" |cut -d = -f 2",
					      "rb");
				if (tempfp) {
					fscanf(tempfp, "%d.%d", &wifi3_phy, &dummy);
					pclose(tempfp);
				}
			}
		}
		if (wifi1 > cpu)
			cpu = wifi1;
		if (wifi2 > cpu)
			cpu = wifi2;
		if (wifi3_mac > cpu)
			cpu = wifi3_mac;
		if (wifi3_phy > cpu)
			cpu = wifi3_phy;

		int target = cpu - (nvram_geti("hwmon_temp_max") * 1000);
		if (target < 0)
			target = 0;
		if (target > 10000)
			target = 10000;
		target *= 4000;
		target /= 10000;
		if (target != lasttarget) {
			fprintf(stderr, "set fan to %d\n", target);
			sysprintf("/bin/echo %d > /sys/class/hwmon/hwmon0/device/fan1_target", target);
			lasttarget = target;
		}
#endif

		sleep(5);
	}
}

int main(int argc, char *argv[])
{
	/* 
	 * Run it under background 
	 */
	switch (fork()) {
	case -1:
		perror("fork failed");
		_exit(1);
		break;
	case 0:
		/* 
		 * child process 
		 */
		watchdog();
		_exit(0);
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
