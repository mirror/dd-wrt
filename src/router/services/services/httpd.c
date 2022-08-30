/*
 * httpd.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
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
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <unistd.h>
#include <services.h>

void start_httpd(void)
{
	int ret = 0;
	int do_ssl = 0;
	int no_ssl = 0;
	char *lan_port = NULL;
	char *ssl_lan_port = NULL;
	stop_httpd();
	update_timezone();
	chdir("/www");
	if (nvram_invmatchi("http_enable", 0)) {
		no_ssl = 1;
		if (nvram_invmatch("http_lanport", "")) {
			lan_port = nvram_safe_get("http_lanport");
		}
	}
#ifdef HAVE_HTTPS
	if (nvram_invmatchi("https_enable", 0)) {
		do_ssl = 1;
		if (nvram_invmatch("https_lanport", "")) {
			ssl_lan_port = nvram_safe_get("https_lanport");
		}
	}
#endif
	int c = 0;
	char *args[4] = { NULL, NULL, NULL, NULL };
	char *lanarg = NULL;
	char *ssl_lanarg = NULL;

	if (lan_port) {
		args[c++] = "-p";
		args[c++] = lan_port;
	}
	if (ssl_lan_port) {
		args[c++] = "-m";
		args[c++] = ssl_lan_port;
	}

	if (!f_exists("/var/run/httpd.pid")) {
		if (do_ssl && no_ssl) {
			log_eval("httpd", "-n", "-S", args[0], args[1], args[2], args[3]);
		} else if (no_ssl) {
			log_eval("httpd", "-n", args[0], args[1], args[2], args[3]);
		} else if (do_ssl) {
			log_eval("httpd", "-S", args[0], args[1], args[2], args[3]);
		}
	}
	chdir("/");

	cprintf("done\n");
	return;
}

void stop_httpd(void)
{
	stop_process("httpd", "daemon");
	unlink("/var/run/httpd.pid");
	return;
}
