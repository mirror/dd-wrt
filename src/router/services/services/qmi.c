/*
 * conntrack.c
 *
 * Copyright (C) 2017 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#ifdef HAVE_UQMI
#include <json-c/json.h>

static char *get_popen_data(char *command)
{
	FILE *pf;
	char temp[256];
	int chunksize = 256;
	char *data = NULL;
	int length = 0;

	pf = popen(command, "r");
	if (!pf) {
		fprintf(stderr, "Could not open pipe for output.\n");
		return NULL;
	}
	while (fgets(temp, chunksize, pf)) {
		if (data == NULL) {
			length = asprintf(&data, "%s", temp);
		} else {
			length += strlen(temp);
			data = (char *)realloc(data, length + 1);
			if (data)
				dd_strncat(data, length, temp);
			else {
				pclose(pf);
				return (NULL);
			}
		}
	}
	if (pclose(pf) != 0)
		fprintf(stderr, " Error: Failed to close command stream \n");
	return (data);
}

static char *get_json_data_by_key(char *output, char *getkey)
{
	char *ret = NULL;
	enum json_type type;
	if (!output || !getkey)
		return NULL;
	json_object *jobj = json_tokener_parse(output);
	if (!jobj)
		return NULL;
	json_object_object_foreach(jobj, key, val)
	{
		if (val && key && !strcmp(key, getkey)) {
			type = json_object_get_type(val);
			switch (type) {
			case json_type_string:
				asprintf(&ret, "%s", json_object_get_string(val));
				return (ret);
				break;
			case json_type_int:
				asprintf(&ret, "%d", json_object_get_int(val));
				return (ret);
			case json_type_boolean:
				asprintf(&ret, "%d", json_object_get_boolean(val));
				return (ret);
			case json_type_double:
				asprintf(&ret, "%f", json_object_get_double(val));
				return (ret);
			default:
				break;
			}
		}
	}
	return NULL;
}
#endif

#if defined(HAVE_UQMI) || defined(HAVE_LIBQMI)
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
		sysprintf(
			"uqmi -d /dev/cdc-wdm0 --set-client-id wds,%d --keep-client-id wds --get-data-status | grep '^\"connected' | wc -l >/tmp/qmistatustemp ; mv /tmp/qmistatustemp /tmp/qmistatus",
			clientid);
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
