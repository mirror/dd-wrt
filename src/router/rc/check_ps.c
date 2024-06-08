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
#include <dlfcn.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <utils.h>
#include <cy_conf.h>
#include <code_pattern.h>
#include <rc.h>
#include <wlutils.h>

struct mon {
	char *name; // Process name
	int type; // LAN or WAN
	// int (*stop) (void); // stop function
	// int (*start) (void); // start function
	char *nvvalue;
	char *nvmatch;
	char *nvvalue2;
	char *nvmatch2;
	int (*checkfunc)(void);
};

static int search_process(char *name, int count)
{
	int c = 0;

	c = count_processes(name);
	if (!c) {
		printf("Can't find %s\n", name);
		return 0;
	} else {
		printf("Find %s which count is %d\n", name, c);
		if (c < count) {
			return 0;
		}
		return 1;
	}
}

static int check_igmprt(void)
{
	char wan_if_buffer[33];
	if (nvram_match("wan_proto", "disabled") || !*(safe_get_wan_face(wan_if_buffer))) // todo: add upstream
		return 0;
	return !search_process("igmprt", 1);
}

static int check_transmission(void)
{
	if (nvram_match("transmission_enable", "0")) // todo: add upstream
		return 0;
	return !search_process("transmissiond", 1);
}

static int check_plex(void)
{
	if (nvram_match("plex_enable", "0")) // todo: add upstream
		return 0;
	return !search_process("Plex Media Server", 1);
}

static int check_nodog(void)
{
	if (nvram_match("ND_enable", "0")) // todo: add upstream
		return 0;
	return !search_process("nodogsplash", 1);
}

static int check_ddns(void)
{
	char wan_if_buffer[33];

	if (nvram_match("ddns_enable", "0") || nvram_invmatch("wanup", "1") || nvram_match("wan_proto", "disabled") ||
	    !*(safe_get_wan_face(wan_if_buffer))) // todo: add upstream
		return 0;
	return !search_process("inadyn", 1);
}

static int check_radvd(void)
{
	if (!nvram_matchi("ipv6_enable", 1) || !nvram_matchi("radvd_enable", 1))
		return 0;
	return !search_process("radvd", 1);
}

static int check_httpd(void)
{
	if (nvram_match("http_enable", "0") && nvram_match("https_enable", "0")) // todo: add upstream
		return 0;
	return !search_process("httpd", 1);
}

#ifdef HAVE_ANTAIRA_AGENT
static int check_antaira_agent(void)
{
	char wan_if_buffer[33];
	if (!nvram_invmatchi("antaira_agent_enable", 0))
		return 0;
	if (nvram_match("wan_proto", "disabled") || !*(safe_get_wan_face(wan_if_buffer))) // todo: add upstream
		return 0;
	return !search_process("antaira-quick-vpn-agent", 1);
}
#endif

enum { M_LAN, M_WAN };

struct mon mons[] = {
// {"tftpd",  M_LAN, stop_tftpd, start_tftpd},
#ifdef HAVE_UPNP
	{ "upnp", M_LAN, "upnp_enable", "1", NULL, NULL, NULL },
#endif
	{ "process_monitor", M_LAN, NULL, NULL, NULL, NULL, NULL },
	{ "httpd", M_LAN, "http_enable", "1", "https_enable", "1", &check_httpd },
	{ "dnsmasq", M_LAN, "dnsmasq_enable", "1", NULL, NULL, NULL },
#ifdef HAVE_SMARTDNS
	{ "smartdns", M_WAN, "smartdns", "1", NULL, NULL, NULL },
#endif
	{ "dhcpfwd", M_WAN, "dhcpfwd_enable", "1", NULL, NULL, NULL },
#ifdef HAVE_PRIVOXY
	{ "privoxy", M_WAN, "privoxy_enable", "1", NULL, NULL, NULL },
#endif
#ifdef HAVE_RADVD
	{ "radvd", M_WAN, "radvd_enable", "1", NULL, NULL, &check_radvd },
#endif
#ifdef HAVE_NODOG
	{ "splashd", M_WAN, "ND_enable", "1", NULL, NULL, &check_nodog },
#endif
#ifdef HAVE_CHILLI
	{ "chilli", M_WAN, "chilli_enable", "1", NULL, NULL, NULL },
#endif
#ifdef HAVE_WIFIDOG
	{ "wifidog", M_WAN, "wd_enable", "1", NULL, NULL, NULL },
#endif
#ifdef HAVE_OLSRD
	{ "olsrd", M_LAN, "wk_mode", "olsrd", NULL, NULL, NULL },
#endif
#ifdef HAVE_SPUTNIK_APD
	{ "sputnik", M_WAN, "apd_enable", "1", NULL, NULL, NULL },
#endif
#ifdef HAVE_MULTICAST
	{ "igmprt", M_WAN, "block_multicast", "0", NULL, NULL, &check_igmprt },
#endif
#ifdef HAVE_TRANSMISSION
	{ "transmission", M_LAN, "transmission_enable", "1", NULL, NULL, &check_transmission },
#endif
//#ifdef HAVE_PLEX
//	{ "plex", M_LAN, "plex_enable", "1", NULL, NULL, &check_plex },
//#endif
#ifdef HAVE_ERC
#ifdef HAVE_OPENVPN
	{ "openvpn", M_LAN, "openvpncl_enable", "1", NULL, NULL, NULL },
#endif
#endif
	{ "ddns", M_WAN, NULL, NULL, NULL, NULL, &check_ddns },
#ifdef HAVE_ANTAIRA_AGENT
	{ "antaira_agent", M_LAN, "antaira_agent_enable", "1", NULL, NULL, &check_antaira_agent },
#endif
	{ NULL, 0, NULL, NULL, NULL, NULL }
};

static void checknas(void) // for broadcom v24 only
{
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)

	char buf[32];
	FILE *fnas = fopen("/tmp/.startmon", "r");

	if (fnas == NULL)
		return;
	fclose(fnas);
	fnas = fopen("/tmp/.nas", "r");
	if (fnas == NULL)
		return;
	fgets(buf, sizeof(buf), fnas);
	fclose(fnas);

	if (strlen(buf) != count_processes("nas")) // restart all nas
	// processes
	{
		stop_service("wlconf");
		start_service_force("wlconf");
		stop_service("nas");
		start_service_force("nas");
	}

	return;

#endif
#ifndef HAVE_NOWIFI
#ifdef HAVE_MADWIFI
	start_service_force_f("checkhostapd");
#endif
#endif
}

/* 
		 * software wlan led control 
		 */
static void softcontrol_wlan_led(void) // done in watchdog.c for non-micro builds.
{
#if defined(HAVE_MICRO) && !defined(HAVE_ADM5120) && !defined(HAVE_WRK54G)
	int brand;
	int radiostate0 = -1;
	int oldstate0 = -1;
	int radiostate1 = -1;
	int oldstate1 = -1;
	int radiostate2 = -1;
	int oldstate2 = -1;

#ifdef HAVE_MADWIFI
	int cnt = getdevicecount();
#else
	int cnt = get_wl_instances();
#endif

#ifdef HAVE_MADWIFI
	if (!nvram_matchi("flash_active", 1)) {
		radiostate0 = get_radiostate("wlan0");
		if (cnt == 2)
			radiostate1 = get_radiostate("wlan1");
	}
#else
	wl_ioctl(get_wl_instance_name(0), WLC_GET_RADIO, &radiostate0, sizeof(int));
	if (cnt == 2)
		wl_ioctl(get_wl_instance_name(1), WLC_GET_RADIO, &radiostate1, sizeof(int));
	if (cnt == 3) {
		wl_ioctl(get_wl_instance_name(1), WLC_GET_RADIO, &radiostate1, sizeof(int));
		wl_ioctl(get_wl_instance_name(2), WLC_GET_RADIO, &radiostate2, sizeof(int));
	}
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
			brand = getRouterBrand();
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
	return;

#endif
}

/* 
 * end software wlan led control 
 */
static void checkupgrade(void)
{
	if (nvram_matchi("flash_active", 1))
		return;

	FILE *in = fopen("/tmp/firmware.bin", "rb");

	if (in != NULL) {
		nvram_set("flash_active", "1");
		fseek(in, 0, SEEK_END);
		size_t len = ftell(in);
		fclose(in);
		// prevent double call of
		// this
		eval("ledtool", "500");
		dd_loginfo("upgrade", "found firmware upgrade, flashing now, but we will wait for another 30 seconds\n");
again:;
		sleep(30);
		in = fopen("/tmp/firmware.bin", "rb");
		if (!in) {
			nvram_set("flash_active", "0");
			return;
		}
		fseek(in, 0, SEEK_END);
		size_t newlen = ftell(in);
		fclose(in);
		unlink("/tmp/cron.d/check_ps"); // deleting cron file to
		if (newlen != len) {
			len = newlen;
			dd_loginfo("upgrade", "size has changed, wait 30 seconds and try again\n");
			goto again;
		}
#if defined(HAVE_WHRAG108) || defined(HAVE_TW6600) || defined(HAVE_LS5)
		eval("write", "/tmp/firmware.bin", "rootfs");
#elif defined(HAVE_NEWPORT)
		eval("mkdir", "-p", "/tmp/new_root");
		eval("mount", "-n", "-t", "tmpfs", "none", "/tmp/new_root");
		eval("mkdir", "-p", "/tmp/new_root/tmp");
		eval("mv", "/tmp/firmware.bin", "/tmp/new_root/tmp");
		char disk[32];
		char *d = getdisc();
		sprintf(disk, "/dev/%s", d);
		free(d);
		eval("update-prepare.sh", "/tmp/firmware.bin", disk, "usefile", "reboot", "usedd");
#elif defined(HAVE_VENTANA)
		eval("mkdir", "-p", "/tmp/new_root");
		eval("mount", "-n", "-t", "tmpfs", "none", "/tmp/new_root");
		eval("mkdir", "-p", "/tmp/new_root/tmp");
		eval("mv", "/tmp/firmware.bin", "/tmp/new_root/tmp");
		eval("update-prepare.sh", "/tmp/firmware.bin", "rootfs", "usefile", "reboot");
#elif defined(HAVE_X86) || defined(HAVE_RB600) && !defined(HAVE_WDR4900)
		eval("mkdir", "-p", "/tmp/new_root");
		eval("mount", "-n", "-t", "tmpfs", "none", "/tmp/new_root");
		eval("mkdir", "-p", "/tmp/new_root/tmp");
		eval("mv", "/tmp/firmware.bin", "/tmp/new_root/tmp");
		char disk[32];
		char *d = getdisc();
		sprintf(disk, "/dev/%s", d);
		free(d);
		eval("update-prepare.sh", "/tmp/firmware.bin", disk, "usefile", "reboot", "usedd");
#else
		eval("fischecksum");
		eval("write", "/tmp/firmware.bin", "linux");
#endif
		dd_loginfo("upgrade", "done. rebooting now\n");
		sleep(5);
		killall("init", SIGQUIT);
	}
}

static int do_mon(void)
{
	struct mon *v;
	if (nvram_matchi("sysup", 0)) {
		// wait until wan_done has been processed to prevent double tap of some services like dnsmasq.
		return 1;
	}
	checkupgrade();
	checknas();
#ifndef HAVE_RT2880
	softcontrol_wlan_led();
#endif
	for (v = mons; v < &mons[sizeof(mons) / sizeof(struct mon)]; v++) {
		if (v->name == NULL)
			break;
		int count = 0;
		if (v->nvvalue && nvram_match(v->nvvalue, v->nvmatch))
			count++;
		if (v->nvvalue2 && nvram_match(v->nvvalue2, v->nvmatch2))
			count++;
		if (v->checkfunc && !v->checkfunc()) // optional check method.
			count = 0;
		if (count) {
			printf("checking %s\n", v->name);

			if (v->type == M_WAN)
				if (!nvram_matchi("wanup", 1)) {
					printf("process is wan, but wan is not up\n");
					continue;
				}
			if (!search_process(v->name, count)) {
				dd_loginfo(v->name, "maybe died, we need to re-exec it\n");
				stop_service(v->name);
				killall(v->name, SIGKILL);
				start_service_force_f(v->name);
			}
		}
		printf("checking for %s done\n", v->name);
	}

	return 1;
}

static int check_ps_main(int argc, char **argv)
{
	pid_t pid;

	if (check_action() != ACT_IDLE) { // Don't execute during upgrading
		printf("check_ps: nothing to do...\n");
		return 1;
	}

	pid = fork();
	switch (pid) {
	case -1:
		perror("fork failed");
		_exit(1);
		break;
	case 0:
		do_mon();
		_exit(0);
		break;
	default:
		_exit(0);
		break;
	}
}
