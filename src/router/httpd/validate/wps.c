#ifdef HAVE_WPS
#define VALIDSOURCE 1

#ifdef WEBS
#include <webs.h>
#include <uemf.h>
#include <ej.h>
#else				/* !WEBS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <httpd.h>
#include <errno.h>
#endif				/* WEBS */

#include <proto/ethernet.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/klog.h>
#include <sys/wait.h>
#include <cyutils.h>
#include <support.h>
#include <cy_conf.h>
// #ifdef EZC_SUPPORT
#include <ezc.h>
// #endif
#include <broadcom.h>
#include <wlutils.h>
#include <netdb.h>
#include <utils.h>
#include <stdarg.h>
#include <sha1.h>

struct variable **variables;

void wps_ap_register(webs_t wp)
{
	char *pin = websGetVar(wp, "wps_ap_pin", NULL);
	if (pin) {
		nvram_set("pincode", pin);
		eval("hostapd_cli", "-i", "ath0", "wps_ap_pin", "set", pin, "300");
#ifdef HAVE_WZRHPAG300NH
		eval("hostapd_cli", "-i", "ath1", "wps_ap_pin", "set", pin, "300");
#endif
		nvram_set("wps_status", "2");
	}
}

void wps_register(webs_t wp)
{
	char *pin = websGetVar(wp, "wps_pin", NULL);
	if (pin) {
		eval("hostapd_cli", "-i", "ath0", "wps_pin", "any", pin, "300");
#ifdef HAVE_WZRHPAG300NH
		eval("hostapd_cli", "-i", "ath1", "wps_pin", "any", pin, "300");
#endif
		nvram_set("wps_status", "3");
	}
}

void wps_forcerelease(webs_t wp)
{
	nvram_set("wps_forcerelease", "1");
	addAction("wireless");
	nvram_set("nowebaction", "1");
	service_restart();
}

void wps_configure(webs_t wp)
{
	nvram_set("wps_status", "1");
	addAction("wireless");
	nvram_set("nowebaction", "1");
	service_restart();
}
#endif
