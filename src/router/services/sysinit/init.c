/*
 * init.c
 *
 * Copyright (C) 2017 Sebastian Gottschall <gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */


#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <services.h>
#include <sys/stat.h>

#define start_single_service() eval("start_single_service");
#define stop_services() eval("stopservices");
#define start_services() eval("startservices");

void start_init_user(void)
{
	start_single_service();
#ifdef HAVE_CHILLI
	start_chilli();
#endif
#ifdef HAVE_WIFIDOG
	start_wifidog();
#endif
}

void start_init_restart(void)
{

#ifdef HAVE_OVERCLOCKING
	start_overclocking();
#endif
	cprintf("RESET NVRAM VARS\n");
	nvram_set("wl0_lazy_wds", nvram_safe_get("wl_lazy_wds"));

	cprintf("RESTART\n");
#ifndef HAVE_MADWIFI
	int cnt = get_wl_instances();
#ifdef HAVE_QTN
	cnt = 1;
	nvram_seti("qtn_ready", 0);
#endif
#endif

#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	for (c = 0; c < cnt; c++) {
		eval("wlconf", get_wl_instance_name(c), "down");
		char *next;
		char var[80];
		char *vifs = nvram_nget("wl%d_vifs", c);

		if (vifs != NULL)
			foreach(var, vifs, next) {
			eval("ifconfig", var, "down");
			}
	}
#endif

}

void start_init_stop(void)
{

	lcdmessage("STOPPING SERVICES");
	cprintf("STOP\n");
	killall("udhcpc", SIGKILL);

	cprintf("STOP SERVICES\n");

	stop_services();
	stop_radio_timer();
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	stop_service("nas");
#endif
	cprintf("STOP WAN\n");
	stop_ttraff();
	stop_wan();
	stop_mkfiles();
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	stop_wlconf();
#endif
	cprintf("STOP LAN\n");
#ifdef HAVE_MADWIFI
	stop_stabridge();
#endif
#ifdef HAVE_EMF
	stop_emf();
#endif
#ifdef HAVE_IPVS
	stop_ipvs();
#endif
#ifdef HAVE_VLANTAGGING
	stop_bridging();
#endif
#ifdef HAVE_BONDING
	stop_bonding();
#endif

#ifdef HAVE_VLANTAGGING
	stop_bridgesif();
	stop_vlantagging();
#endif
	stop_lan();
#ifndef HAVE_RB500
	stop_resetbutton();
#endif
#ifdef HAVE_IPV6
	stop_ipv6();
#endif
#ifdef HAVE_REGISTER
	if (isregistered_real())
#endif
	{
		start_run_rc_shutdown();
	}

}

void start_init_start(void)
{
#ifdef HAVE_IPV6
	start_ipv6();
#endif
#ifndef HAVE_RB500
	start_resetbutton();
#endif
	start_setup_vlans();
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
//                      start_wlconf(); // doesnt make any sense. its already triggered by start lan
#endif
#ifdef HAVE_VLANTAGGING
	start_bridging();
#endif
	start_lan();
#ifdef HAVE_IPVS
	start_ipvs();
#endif
#ifdef HAVE_BONDING
	start_bonding();
#endif
#ifdef HAVE_REGISTER
	start_mkfiles();
#endif
#ifdef HAVE_MADWIFI
	start_stabridge();
#endif

	cprintf("start services\n");
	start_services();

	cprintf("start wan boot\n");
#ifdef HAVE_VLANTAGGING
	start_vlantagging();
	start_bridgesif();
#endif
	start_wan_boot();
	start_ttraff();

	cprintf("diag STOP LED\n");
	diag_led(DIAG, STOP_LED);
	cprintf("set led release wan control\n");

#ifdef HAVE_RADIOOFF
	if (nvram_matchi("radiooff_button", 1)
	    && nvram_matchi("radiooff_boot_off", 1)) {
		start_radio_off();
		led_control(LED_SEC0, LED_OFF);
		led_control(LED_SEC1, LED_OFF);
	} else
#endif
	{
		start_radio_off();
		start_radio_on();

	}
	start_radio_timer();
#ifdef HAVE_EMF
	start_emf();
#endif

	cprintf("run rc file\n");
#ifdef HAVE_REGISTER
#ifndef HAVE_ERC
	if (isregistered_real())
#endif
#endif
	{
		stop_run_rc_startup();
		start_run_rc_startup();
// start init scripts                           
		eval("/etc/init.d/rcS");
		eval("/opt/etc/init.d/rcS");
		eval("/jffs/etc/init.d/rcS");
		eval("/mmc/etc/init.d/rcS");
		// startup script
		// (siPath impl)
		cprintf("start modules\n");
		start_modules();
#ifdef HAVE_MILKFISH
		start_milkfish_boot();
#endif
		if (nvram_invmatch("rc_custom", ""))	// create
			// custom
			// script
		{
			nvram2file("rc_custom", "/tmp/custom.sh");
			chmod("/tmp/custom.sh", 0700);
		}
	}
#ifdef HAVE_CHILLI
	start_chilli();
#endif
#ifdef HAVE_WIFIDOG
	start_wifidog();
#endif
	cprintf("start syslog\n");
#ifdef HAVE_SYSLOG
	stop_syslog();
	start_syslog();
#endif
	system("/etc/postinit&");
	start_httpd();
	led_control(LED_DIAG, LED_OFF);
	lcdmessage("System Ready");
#ifndef HAVE_RB500
	stop_resetbutton();
	start_resetbutton();
#endif

}
