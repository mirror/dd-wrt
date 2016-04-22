
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <syslog.h>
#include <wait.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <rc.h>

#include <ntp.h>
#include <cy_conf.h>
#include <utils.h>

#define NTP_M_TIMER "3600"
#define NTP_N_TIMER "30"

extern void dd_timer_cancel(timer_t timerid);
int isRunning(char *name)
{
	return eval("pidof", name) == 0 ? 1 : 0;
}

void check_udhcpd(timer_t t, int arg)
{
	if (nvram_invmatch("router_disable", "1")
	    || nvram_match("lan_proto", "dhcp")) {
		if (nvram_match("dhcp_dnsmasq", "1")) {
			if (!isRunning("dnsmasq")) {
				// killps("dnsmasq","-9");
				// killps("udhcpd","-9");
				killall("dnsmasq", SIGKILL);
				killall("udhcpd", SIGKILL);
#ifdef HAVE_UDHCPD
				sleep(1);
				eval("startservice", "udhcpd");
#endif
				sleep(1);
				eval("startservice", "dnsmasq");
			}
		} else {
			if (!isRunning("udhcpd")) {
				killall("dnsmasq", SIGKILL);
				killall("udhcpd", SIGKILL);
#ifdef HAVE_UDHCPD
				sleep(1);
				eval("startservice", "udhcpd");
#endif
				sleep(1);
				eval("startservice", "dnsmasq");
			}
		}
	}
}

// <<tofu
int do_ntp(void)		// called from ntp_main and
				// process_monitor_main; (now really) called every hour!
{
	char *servers;

	if (!nvram_match("ntp_enable", "1"))
		return 0;
	update_timezone();

	if (((servers = nvram_get("ntp_server")) == NULL)
	    || (*servers == 0))
		servers = "209.81.9.7 207.46.130.100 192.36.144.23 pool.ntp.org";

	char *argv[] = { "ntpclient", servers, NULL };
	if (_evalpid(argv, NULL, 20, NULL) != 0) {
		// fprintf (stderr, "ntp returned a error\n");
		return 1;
	}
#if defined(HAVE_VENTANA) || defined(HAVE_LAGUNA) || defined(HAVE_STORM) || (defined(HAVE_GATEWORX) && !defined(HAVE_NOP8670))
	eval("hwclock", "-w");
#endif

	return 0;
}

/* 
 * int mon; int day; int dst = 4; for (mon = 1; mon <= 12; ++mon) {
 * printf("[%02d] ", mon); for (day = 1; day <= 31; ++day) { int yi = 2005 -
 * 2006; // dst table starts at 2006 int mbeg = dstEntry[dst].startMonth; int 
 * mend = dstEntry[dst].endMonth; int dbeg = dstEntry[dst].startDay[yi]; int
 * dend = dstEntry[dst].endDay[yi];
 * 
 * if (((mon == mbeg) && (day >= dbeg)) || ((mon == mend) && (day <= dend))
 * || ((mbeg < mend) && (mon > mbeg) && (mon < mend)) || ((mbeg > mend) &&
 * ((mon > mbeg) || (mon < mend)))) { printf("%d,", day); if ((mon == mend)
 * && (day == dend)) { printf("***"); } } } printf("\n"); } 
 */

void ntp_main(timer_t t, int arg)
{
	if (check_action() != ACT_IDLE)
		return;		// don't execute while upgrading
	if (!check_wan_link(0) && nvram_invmatch("wan_proto", "disabled"))
		return;		// don't execute if not online

	//dd_syslog(LOG_INFO, "time updated: %s\n", ctime(&now));
	eval("stopservice", "ntpc", "-f");
	if (do_ntp() == 0) {
		if (arg == FIRST)
			dd_timer_cancel(t);
		eval("filtersync");
		nvram_set("timer_interval", NTP_M_TIMER);	// are these used??
	} else {
		nvram_set("timer_interval", NTP_N_TIMER);
	}

}
