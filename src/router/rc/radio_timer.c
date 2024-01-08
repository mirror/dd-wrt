/* 
 * Radio timer by Eko: 19.jul.2006
 * 
 * hours are represented as bits in 24 bit = xxxxxxxxxxxxxxxxxxxxxxxx from
 * GUI code scans for changes: 10 = radio off, 01 = radio on firsttime
 * change: 00 and 10 = radio off, 11 and 01 = radio on 
 */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <bcmnvram.h>
#include <cy_conf.h>
#include <rc.h>
#include <shutils.h>
#include <syslog.h>
#include <utils.h>
#include <wlutils.h>

#define start_service(a) eval("startservice", a);
#define start_service_force(a) eval("startservice", a, "-f");
#define start_service_f(a) eval("startservice_f", a);
#define start_service_force_f(a) eval("startservice_f", a, "-f");
#define start_services() eval("startservices");
#define stop_service(a) eval("stopservice", a);
#define stop_service_force(a) eval("stopservice", "-f", a);
#define stop_running(a) eval("stop_running");
#define stop_service_f(a) eval("stopservice_f", a);
#define stop_service_force_f(a) eval("stopservice_f", a, "-f");
#define stop_services() eval("stopservices");
#define restart(a) eval("restart", a);
#define restart_f(a) eval("restart_f", a);

int main(int argc, char **argv)
{
	switch (fork()) {
	case -1:
		// can't fork
		_exit(0);
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

	unsigned int *
		radiotime; // 4 byte int number (24 bits from gui + 1 bit for midnight)
	int cnt = getdevicecount();
	unsigned char *firsttime, *needchange;
	radiotime = malloc(sizeof(unsigned int *) * cnt);
	needchange = malloc(cnt);
	firsttime = malloc(cnt);
	memset(needchange, 1, cnt);
	memset(firsttime, 1, cnt);

	struct tm *currtime;
	long int tloc;

	do {
		time(&tloc); // get time in seconds since epoch
		currtime =
			localtime(&tloc); // convert seconds to date structure
		if (currtime->tm_year > 100) // ntp time must be set
		{
			char radio_timer_enable[32];
			char radio_on_time[32];
			int i;
			for (i = 0; i < cnt; i++) {
				sprintf(radio_timer_enable,
					"radio%d_timer_enable", i);
				sprintf(radio_on_time, "radio%d_on_time", i);

				if (nvram_matchi(radio_timer_enable, 1)) {
					radiotime[i] = (unsigned int)strtol(
						nvram_safe_get(radio_on_time),
						NULL,
						2); // convert  binary  string  to  long  int
					radiotime[i] +=
						((radiotime[i] & 1)
						 << 24); // duplicate 23-24h bit to the start to take care of midnight
					radiotime[i] =
						(radiotime[i] >>
						 (24 - currtime->tm_hour - 1)) &
						3; // get pattern only (last two bits)
				}
				if (currtime->tm_min != 0)
					needchange[i] =
						1; // prevet o be executed more than once when min == 0
				if (firsttime[i]) {
					// first time change
					switch (radiotime[i]) {
					case 3: // 11
						radiotime[i] = 1; // 01
						break;
					case 0: // 00
						radiotime[i] = 2; // 10
						break;
					}
				}

				if (nvram_matchi(radio_timer_enable, 0))
					radiotime[i] = 0;
				/* change when min = 0  or firstime */
				if (((needchange[i]) &&
				     currtime->tm_min == 0) ||
				    (firsttime[i])) {
					switch (radiotime[i]) {
					case 0:
						break; // do nothing, radio0 timer disabled
					case 1: // 01 - turn radio on
						if (!firsttime[i]) {
							//on first time call the radio is already on, no need to reinit it a second time
							syslog(LOG_DEBUG,
							       "Turning radio %d on\n",
							       i);
							char on[32];
							sprintf(on,
								"radio_on_%d",
								i);
							start_service_force(on);
						}
						break;
					case 2: // 10 - turn radio off
						syslog(LOG_DEBUG,
						       "Turning radio %d off\n",
						       i);
						char off[32];
						sprintf(off, "radio_off_%d", i);
						start_service_force(off);
#ifdef HAVE_MADWIFI
						char dev[32];
						sprintf(dev, "wlan%d", i);
						eval("ifconfig", dev, "down");
						char *next;
						char var[80];
						char *vifs = nvram_nget(
							"wlan%d_vifs", i);
						foreach(var, vifs, next)
						{
							eval("ifconfig", var,
							     "down");
						}
#endif
						break;
					}
					needchange[i] = 0;
					firsttime[i] = 0;
				}
			}
		} else {
			// if yr < 100 (=2000) wait 5 min and try
			// again (if ntp time is maybe set now)
			sleep(242);
		}

		sleep(58); // loop every 58 s to be sure to catch min == 0
	} while (1);
	return 0;
}
