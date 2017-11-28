/*
 * conntrack.c
 *
 * Copyright (C) 2017 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#include <services.h>

#if defined(HAVE_UQMI) || defined(HAVE_LIBQMI)
void start_check_sierradirectip(void)
{
	eval("/etc/comgt/sierrastatus.sh", nvram_safe_get("3gcontrol"), "dip");
}

void start_check_sierrappp(void)
{
	eval("/etc/comgt/sierrastatus.sh", nvram_safe_get("3gcontrol"));
}

#if defined(HAVE_LIBMBIM) || defined(HAVE_UMBIM)
void start_check_mbim(void)
{
	if (registered_has_cap(27)) {
		char *str;
		asprintf(&str, "/dev/%s", nvram_safe_get("3gctrl"));	// ???? no where set
		eval("/usr/sbin/mbim-status.sh", str);
		free(str);
	}
}
#endif

void start_check_qmi(void)
{
#ifdef HAVE_UQMI
	char *output, *retval, *rsrq;
	char command[256];
	char buf[256];
	int clientid = 0;
	FILE *fp = fopen("/tmp/qmi-clientid", "rb");
	if (fp) {
		fscanf(fp, "%d", &clientid);
		fclose(fp);
		sysprintf("uqmi -d /dev/cdc-wdm0 --set-client-id wds,%d --keep-client-id wds --get-data-status | grep '^\"connected' | wc -l >/tmp/qmistatustemp ; mv /tmp/qmistatustemp /tmp/qmistatus", clientid);
		sprintf(command, "uqmi -d /dev/cdc-wdm0 --set-client-id wds,%d --keep-client-id wds --get-signal-info", clientid);
		output = get_popen_data(command);
		if (output) {
			retval = get_json_data_by_key(output, "rssi");
			rsrq = get_json_data_by_key(output, "rsrq");
			if (retval && rsrq) {
				snprintf(buf, sizeof(buf), "RSSI: %s / RSRQ: %s", retval, rsrq);
				nvram_set("wan_3g_signal", buf);
				free(retval), free(rsrq);
			} else if (retval && !rsrq) {
				snprintf(buf, sizeof(buf), "RSSI: %s", retval);
				nvram_set("wan_3g_signal", buf);
				free(retval);
			}
			retval = get_json_data_by_key(output, "type");
			if (retval) {
				snprintf(buf, sizeof(buf), "%s", retval);
				nvram_set("wan_3g_mode", buf);
				free(retval);
			}
			free(output);
		}
	} else {
		sysprintf("echo 0 > /tmp/qmistatus");
	}
#else
	sysprintf("qmi-network /dev/cdc-wdm0 status|grep disconnected|wc -l>/tmp/qmistatus");
#endif
}
#endif
