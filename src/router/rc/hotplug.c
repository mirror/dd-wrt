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


#define start_service(a) eval("startservice",a);
#define stop_service(a) eval("stopservice",a);

int main(int argc, char **argv)
{
	if (argc >= 2) {
//		sysprintf("echo received %s >> /tmp/hotplugs",argv[1]);
//		sysprintf("env >> /tmp/hotplugs",argv[1]);
		if (!strcmp(argv[1], "net")) {
#ifdef HAVE_IPETH
			char *action = getenv("ACTION");
			char *id = getenv("PHYSDEVDRIVER");
//			sysprintf("echo cation %s %s >> /tmp/hotplugs",id,action);
			if (action && !strcmp(action,"add") && id && !strcmp(id,"ipheth")) {
				eval("ipheth-loop");
				return 0;
			}
#endif
			start_service("hotplug_net");
			action = getenv("ACTION");

			return 0;
		}
#ifdef HAVE_USB
#ifdef HAVE_HOTPLUG2
		if (!strcmp(argv[1], "block")) {
			start_service("hotplug_block");
			return 0;
		}
#else
		if (!strcmp(argv[1], "usb")) {
			start_service("hotplug_usb");
			return 0;
		}
#endif
#endif
#ifdef HAVE_ATH9K
		if (!strcmp(argv[1], "regulatory")) {
			syslog(LOG_DEBUG, "hotplug: old style regulatory called\n");
			return eval("/sbin/crda");
		}
		if (!strcmp(argv[1], "platform")) {
			char *action;
			char *devicepath;
			if ((action = getenv("ACTION")) && (devicepath=getenv("DEVPATH"))
				&& !strcmp(action, "change") && !strcmp(devicepath,"/devices/platform/regulatory.0"))
				{
				syslog(LOG_DEBUG, "hotplug: new style regulatory called\n");
				return eval("/sbin/crda");
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
