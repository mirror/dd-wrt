/* 
 * DD-WRT hotplug.c
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <bcmnvram.h>
#include <cy_conf.h>
#include <rc.h>
#include <shutils.h>
#include <syslog.h>
#include <utils.h>
#include <errno.h>

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
	if (argc >= 2) {
		char *action = getenv("ACTION");
		char *devpath = getenv("BUTTON");
		char *button = getenv("DEVPATH");
		dd_logdebug("hotplug",
			    "event %s, ACTION %s BUTTON %s DEVPATH %s\n",
			    argv[1], action ? action : "none",
			    button ? button : "none",
			    devpath ? devpath : "none");
		if (!strcmp(argv[1], "net")) {
			start_service_force_f("hotplug_net");

			return 0;
		}
		if (!strcmp(argv[1], "button")) {
			if (action && button) {
				char name[32];
				sprintf(name, "/tmp/.button_%s", button);
				FILE *fp = fopen(name, "wb");
				if (fp) {
					if (!strcmp(action, "pressed")) {
						putc(1, fp);
					} else {
						putc(0, fp);
					}
					fclose(fp);
				}
			}
		}
#ifdef HAVE_USB
#ifdef HAVE_HOTPLUG2
		if (!strcmp(argv[1], "block")) {
			start_service_force_f("hotplug_block");
			return 0;
		}
#else
		if (!strcmp(argv[1], "usb")) {
			start_service_force_f("hotplug_usb");
			return 0;
		}
#endif
#endif
#ifdef HAVE_ATH9K
		if (!strcmp(argv[1], "regulatory")) {
			syslog(LOG_DEBUG,
			       "hotplug: old style regulatory called\n");
			int r = eval("/sbin/crda");
			//                      unlink("/tmp/.crdalock");
			return r;
		}
		if (!strcmp(argv[1], "platform")) {
			if (action && devpath && !strcmp(action, "change") &&
			    !strcmp(devpath,
				    "/devices/platform/regulatory.0")) {
				syslog(LOG_DEBUG,
				       "hotplug: new style regulatory called\n");
				int r = eval("/sbin/crda");
				unlink("/tmp/.crdalock");
				return r;
			}
		}
#endif
#ifdef HAVE_XSCALE
		if (!strcmp(argv[1], "firmware"))
			return eval("/etc/upload", argv[1]);
#endif
	} else {
		fprintf(stderr, "usage: hotplug [event]\n");
		return EINVAL;
	}
	return 0;
}
