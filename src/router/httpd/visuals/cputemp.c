/*
 * cputemp.c
 *
 * Copyright (C) 2005 - 2018 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#ifdef HAVE_CPUTEMP

#define CELSIUS 0
#define VOLT 1
#define RPM 2

struct SENSORS {
	char *path;
	int scale;
	int (*method)(void);
	int shown;
	int type;
};
static struct SENSORS *sensors = NULL;

static void sensorreset(void)
{
	int cnt = 0;
	if (sensors) {
		while (sensors[cnt].path || sensors[cnt].method) {
			if (sensors[cnt].path)
				free(sensors[cnt].path);
			cnt++;
		}
		free(sensors);
		sensors = NULL;
	}
}
static int checkhwmon(char *sysfs)
{
	char *sub = strstr(sysfs, "hwmon");
	if (!sensors)
		return 0;
	if (!sub)
		return 0;
	sub = strdup(sub);
	char *idx = strchr(sub, '/');
	if (!idx) {
		free(sub);
		return 0;
	}
	idx = strchr(idx + 1, '/');
	if (!idx) {
		free(sub);
		return 0;
	}
	*idx = 0;
	int cnt = 0;
	while (sensors[cnt].path || sensors[cnt].method) {
		if (sensors[cnt].path && strstr(sensors[cnt].path, sub)) {
			free(sub);
			return 1;
		}
		cnt++;
	}
	free(sub);
	return 0;
}
static int alreadyshowed(char *path)
{
	int cnt = 0;
	if (sensors) {
		while (sensors[cnt].path || sensors[cnt].method) {
			if (sensors[cnt].path && sensors[cnt].shown && !strcmp(sensors[cnt].path, path))
				return 1;
			cnt++;
		}
	}
	return 0;
}

static int addsensor(char *path, int (*method)(void), int scale, int type)
{
	int cnt = 0;
	if (sensors) {
		while (sensors[cnt].path || sensors[cnt].method) {
			if (sensors[cnt].method) {
				cnt++;
				continue;
			}
			if (!strcmp(sensors[cnt].path, path)) {
				sensors[cnt].shown = 1;
				return cnt; // already added
			}
			cnt++;
		}
	}
	sensors = realloc(sensors, sizeof(struct SENSORS) * (cnt + 2));
	sensors[cnt].path = strdup(path);
	sensors[cnt].scale = scale;
	sensors[cnt].method = method;
	sensors[cnt].shown = 1;
	sensors[cnt].type = type;
	sensors[cnt + 1].path = NULL;
	sensors[cnt + 1].method = NULL;
	return cnt;
}

static int getscale(char *path)
{
	int cnt = 0;
	if (sensors) {
		while (sensors[cnt].path || sensors[cnt].method) {
			if (sensors[cnt].method) {
				cnt++;
				continue;
			}
			if (!strcmp(sensors[cnt].path, path))
				return sensors[cnt - 1].scale;
			cnt++;
		}
	}
	return 0;
}

EJ_VISIBLE void ej_read_sensors(webs_t wp, int argc, char_t **argv)
{
	int cnt = 0;
	if (sensors) {
		while (sensors[cnt].path || sensors[cnt].method) {
			int scale = sensors[cnt].scale;
			int sensor = -1;
			if (sensors[cnt].path) {
				FILE *fp = fopen(sensors[cnt].path, "rb");
				if (fp) {
					fscanf(fp, "%d", &sensor);
					fclose(fp);
				}
			}
			if (sensors[cnt].method)
				sensor = sensors[cnt].method();
			if (wp && scale != -1 && sensor != -1) {
				char *unit = "&#176;C";
				if (sensors[cnt].type == VOLT)
					unit = "Volt";
				if (sensors[cnt].type == RPM)
					unit = "rpm";

				if (scale > 1) {
					websWrite(wp, "{cpu_temp%d::%d.%d %s}", cnt, sensor / scale,
						  (sensor % scale) / (scale / 10), unit);
				} else {
					if (sensors[cnt].type == RPM)
						websWrite(wp, "{cpu_temp%d::%d %s}", cnt, sensor, unit);
					else
						websWrite(wp, "{cpu_temp%d::%d.0 %s}", cnt, sensor, unit);
				}
			}
			cnt++;
		}
	}
}

static int showsensor(webs_t wp, const char *path, int (*method)(void), const char *name, int scale, int type)
{
	if (alreadyshowed(path))
		return 1;
	FILE *fp = fopen(path, "rb");
	if (fp || method) {
		int sensor;
		if (fp) {
			fscanf(fp, "%d", &sensor);
			fclose(fp);
		}
		if (method)
			sensor = method();
		if (!scale)
			scale = getscale(path);
		if (!scale) {
			if (sensor > 10000)
				scale = 1000;
			else if (sensor > 1000)
				scale = 100;
			else if (sensor > 100)
				scale = 10;
			else
				scale = 1;
		}
		if (wp && sensor != -1) {
			int count = addsensor(path, method, scale, type);
			char *unit = "&#176;C";
			if (type == VOLT)
				unit = "Volt";
			if (type == RPM)
				unit = "rpm";
			websWrite(wp, "<div class=\"setting\">\n");
			websWrite(wp, "<div class=\"label\">%s</div>\n", name);
			websWrite(wp, "<span id=\"cpu_temp%d\">", count);
			if (scale > 1) {
				websWrite(wp, "%d.%d %s\n", sensor / scale, (sensor % scale) / (scale / 10), unit);
			} else {
				if (type == RPM)
					websWrite(wp, "%d %s\n", sensor, unit);
				else
					websWrite(wp, "%d.0 %s\n", sensor, unit);
			}
			websWrite(wp, "</span>&nbsp;\n");
			websWrite(wp, "</div>\n");
		}
		if (sensor == -1)
			return 0;
		return 1;
	}
	return 0;
}

#if defined(HAVE_MVEBU) || defined(HAVE_OCTEON)
static int show_temp(webs_t wp, int mon, int input, const char *name)
{
	char sysfs[64];
	snprintf(sysfs, 64, "/sys/class/hwmon/hwmon%d/temp%d_input", mon, input);
	return showsensor(wp, sysfs, NULL, name, 1000, CELSIUS);
}

#elif defined(HAVE_ALPINE)
static int show_temp(webs_t wp, int mon, int input, char *name)
{
	char sysfs[64];
	snprintf(sysfs, 64, "/sys/class/hwmon/hwmon%d/temp%d_input", mon, input);
	return showsensor(wp, sysfs, NULL, name, 1, CELSIUS);
}
#elif defined(HAVE_IPQ806X)
static int show_temp(webs_t wp, char *name)
{
	char sysfs[64];
	int mon;
	for (mon = 0; mon < 11; mon++) {
		snprintf(sysfs, 64, "/sys/devices/virtual/thermal/thermal_zone%d/temp", mon);
		char sensorname[32];
		snprintf(sensorname, 32, "%s%d", name, mon);
		showsensor(wp, sysfs, NULL, sensorname, 1000, CELSIUS);
	}
	return 1;
}
#endif

#ifdef HAVE_X86

int getCoreTemp(char *p, size_t len, int *ridx, int acpi)
{
	int idx = 0;
	char path[64];
	while (1) {
		int tidx;
		sprintf(path, "/sys/class/hwmon/hwmon%d/name", idx);
		FILE *fp = fopen(path, "rb");
		if (!fp) {
			*ridx = -1;
			return 0;
		}
		char name[64];
		fscanf(fp, "%s", name);
		fclose(fp);
		if (acpi && !strncmp(name, "acpitz", 6)) {
			snprintf(p, len, "/sys/class/hwmon/hwmon%d", idx);
			*ridx = 0;
			return 1;
		}
		for (tidx = 0; tidx < 32; tidx++) {
			sprintf(path, "/sys/class/hwmon/hwmon%d/temp%d_label", idx, tidx);
			FILE *fp = fopen(path, "rb");
			if (!fp)
				continue;
			fscanf(fp, "%s", name);
			if (!strncmp(name, "Core", 4)) {
				fclose(fp);
				snprintf(p, len, "/sys/class/hwmon/hwmon%d", idx);
				*ridx = tidx;
				return 1;
			}
			fclose(fp);
		}
		idx++;
	}
}

#endif

#ifdef HAVE_BCMMODERN
static int getwifi(int idx)
{
	static int tempcount = -2;
	char buf[WLC_IOCTL_SMLEN];
	int ret;
	unsigned int ret_int[3] = { 0, 0, 0 };
	unsigned int present[3] = { 0, 0, 0 };
	unsigned int result[3] = { 0, 0, 0 };
	static int tempavg[3] = { 0, 0, 0 };
	static unsigned int tempavg_max = 0;
	int i = idx;

	int ttcount = 0;
	int cc = get_wl_instances();
	strcpy(buf, "phy_tempsense");
	char *ifname = get_wl_instance_name(i);
	if (nvram_nmatch("disabled", "wl%d_net_mode", i) || (ret = wl_ioctl(ifname, WLC_GET_VAR, buf, sizeof(buf)))) {
		present[i] = 0;
		return -1;
	}
	ret_int[i] = *(unsigned int *)buf;
	present[i] = 1;

	ret_int[i] *= 10;
	if (tempcount == -2) {
		if (!ttcount)
			ttcount = tempcount + 1;
		tempavg[i] = ret_int[i];
		if (tempavg[i] < 0)
			tempavg[i] = 0;
	} else {
		if (tempavg[i] < 100 && ret_int[i] > 0)
			tempavg[i] = ret_int[i];
		if (tempavg[i] > 2000 && ret_int[i] > 0)
			tempavg[i] = ret_int[i];
		tempavg[i] = (tempavg[i] * 4 + ret_int[i]) / 5;
	}

	if (ttcount)
		tempcount = ttcount;
	for (i = 0; i < cc; i++) {
		result[i] = (tempavg[i] / 2) + 200;
	}
	if (present[idx])
		return result[idx];
	else
		return -1;
}

static int getwifi0(void)
{
	return getwifi(0);
}

static int getwifi1(void)
{
	return getwifi(1);
}

static int getwifi2(void)
{
	return getwifi(2);
}
#endif

EJ_VISIBLE int ej_get_cputemp(webs_t wp, int argc, char_t **argv)
{
	int i, cpufound = 0;
	FILE *fp = NULL;
	FILE *fpsys = NULL;
	char *path;
	sensorreset();
#ifdef HAVE_MVEBU
	if (getRouterBrand() == ROUTER_WRT_1900AC) {
		cpufound |= show_temp(wp, 0, 1, "CPU");
		cpufound |= show_temp(wp, 2, 1, "DDR");
		cpufound |= show_temp(wp, 2, 2, "WLAN");
		cpufound |= show_temp(wp, 3, 1, "WLAN0");
		cpufound |= show_temp(wp, 4, 1, "WLAN1");
	} else {
		int cpuresult = show_temp(wp, 0, 1, "CPU");
		if (!cpuresult) {
			cpufound |= show_temp(wp, 1, 1, "DDR");
			cpufound |= show_temp(wp, 1, 2, "WLAN");
			cpufound |= show_temp(wp, 2, 1, "WLAN0");
		} else {
			cpufound |= show_temp(wp, 1, 1, "DDR");
			cpufound |= show_temp(wp, 1, 2, "WLAN");
			cpufound |= show_temp(wp, 2, 1, "WLAN1");
		}
		cpufound |= show_temp(wp, 3, 1, "WLAN2");
	}
	return 0;
#endif
#ifdef HAVE_OCTEON
	cpufound |= show_temp(wp, 0, 1, "BOARD");
	cpufound |= show_temp(wp, 1, 1, "CPU");
	cpufound |= show_temp(wp, 0, 2, "PHY1");
	cpufound |= show_temp(wp, 1, 2, "PHY2");
#endif
#ifdef HAVE_ALPINE
	cpufound |= show_temp(wp, 1, 1, "CPU");
#elif defined(HAVE_IPQ806X)
	char *wifiname0 = getWifiDeviceName("wlan0", NULL);
	char *wifiname1 = getWifiDeviceName("wlan1", NULL);
	if (wifiname0) {
		cpufound |= show_temp(wp, "Thermal Zone");
	}
#endif
#ifdef HAVE_BCMMODERN
	static int tempcount = -2;
	char buf[WLC_IOCTL_SMLEN];
	int ret;
	unsigned int ret_int[3] = { 0, 0, 0 };
	unsigned int present[3] = { 0, 0, 0 };
	unsigned int result[3] = { 0, 0, 0 };
	static int tempavg[3] = { 0, 0, 0 };
	static unsigned int tempavg_max = 0;

	int ttcount = 0;
	int cc = get_wl_instances();
	for (i = 0; i < cc; i++) {
		strcpy(buf, "phy_tempsense");
		char *ifname = get_wl_instance_name(i);
		if (nvram_nmatch("disabled", "wl%d_net_mode", i) || (ret = wl_ioctl(ifname, WLC_GET_VAR, buf, sizeof(buf)))) {
			present[i] = 0;
			continue;
		}
		ret_int[i] = *(unsigned int *)buf;
		present[i] = 1;

		ret_int[i] *= 10;
		if (tempcount == -2) {
			if (!ttcount)
				ttcount = tempcount + 1;
			tempavg[i] = ret_int[i];
			if (tempavg[i] < 0)
				tempavg[i] = 0;
		} else {
			if (tempavg[i] < 100 && ret_int[i] > 0)
				tempavg[i] = ret_int[i];
			if (tempavg[i] > 2000 && ret_int[i] > 0)
				tempavg[i] = ret_int[i];
			tempavg[i] = (tempavg[i] * 4 + ret_int[i]) / 5;
		}
	}
	if (ttcount)
		tempcount = ttcount;
	for (i = 0; i < cc; i++) {
		result[i] = (tempavg[i] / 2) + 200;
	}

#ifdef HAVE_QTN
	result[1] = rpc_get_temperature() / 100000;
	present[1] = 1;
#endif
	int cputemp = 1;
#ifdef HAVE_NORTHSTAR
	cputemp = 0;
	fp = fopen("/proc/dmu/temperature", "rb");
	if (fp) {
		fclose(fp);
		cputfound |= showsensor(wp, "/proc/dmu/temperature", NULL, "CPU", 10, CELSIUS);
		fp = NULL;
	}
#endif
	if (!present[0] && !present[1] && !present[2] && cputemp)
		return 1;
	else {
		for (i = 0; i < cc; i++) {
			if (present[i]) {
				char wl[32];
				sprintf(wl, "WL%d", i);
				if (i == 0)
					cpufound |= showsensor(wp, NULL, getwifi0, wl, 10, CELSIUS);
				if (i == 1)
					cpufound |= showsensor(wp, NULL, getwifi1, wl, 10, CELSIUS);
				if (i == 2)
					cpufound |= showsensor(wp, NULL, getwifi2, wl, 10, CELSIUS);
			}
		}
	}
#else
	int TEMP_MUL = 1000;
	int SYSTEMP_MUL = 1000;
#ifdef HAVE_GATEWORX
	TEMP_MUL = 100;
	if (getRouterBrand() == ROUTER_BOARD_GATEWORX_SWAP)
		TEMP_MUL = 200;
	char *path = "/sys/devices/platform/IXP4XX-I2C.0/i2c-adapter:i2c-0/0-0028/temp_input";
	fp = fopen(path, "rb");
	if (!fp) {
		path = "/sys/devices/platform/IXP4XX-I2C.0/i2c-0/0-0028/temp1_input";
		fp = fopen(path, "rb");
	}
#elif HAVE_LAGUNA
	TEMP_MUL = 10;
	path = "/sys/bus/i2c/devices/0-0029/temp0_input";
	fp = fopen(path, "rb");
#elif HAVE_UNIWIP
	path = "/sys/bus/i2c/devices/0-0049/temp1_input";
	fp = fopen(path, "rb");
#elif HAVE_VENTANA
	SYSTEMP_MUL = 10;
	path = "/sys/class/hwmon/hwmon1/temp1_input";
	fp = fopen(path, "rb");
	if (!fp) {
		path = "/sys/class/hwmon/hwmon0/temp1_input";
		fp = fopen(path, "rb");
	}
	char *pathsys = "/sys/class/hwmon/hwmon0/temp0_input";
	fpsys = fopen(pathsys, "rb");
#ifdef HAVE_NEWPORT
	if (!fpsys) {
		SYSTEMP_MUL = 1000;
		pathsys = "/sys/class/hwmon/hwmon0/temp2_input";
		fpsys = fopen(path, "rb");
	}
#endif

#else
#ifdef HAVE_X86

	fp = fopen("/sys/devices/platform/i2c-1/1-0048/temp1_input", "rb");
	if (!fp) {
		TEMP_MUL = 100;

		char s_path[64];
		int idx = 0;
		int hascore = 0;
		char tempp[64];
		if (getCoreTemp(s_path, sizeof(s_path), &idx, 0)) {
			char maxp[64];
			sprintf(tempp, "%s/temp%d_input", s_path, idx);
			hascore = 1;
			cpufound |= showsensor(wp, tempp, NULL, "CPU", 1000, CELSIUS);
			TEMP_MUL = 1000;
		} else if (getCoreTemp(s_path, sizeof(s_path), &idx, 1)) {
			char maxp[64];
			sprintf(tempp, "%s/temp%d_input", s_path, idx);
			hascore = 1;
			cpufound |= showsensor(wp, tempp, NULL, "CPU", 1000, CELSIUS);
			TEMP_MUL = 1000;
		}
		if (TEMP_MUL == 100) {
			path = "/sys/class/hwmon/hwmon0/temp1_max";
			if (!fp)
				fp = fopen(path, "rb");
			if (!fp) {
				path = "/sys/class/hwmon/hwmon0/device/temp1_max";
				fp = fopen(path, "rb");
			}
			if (!fp) {
				path = "/sys/class/hwmon/hwmon0/temp2_max";
				fp = fopen(path, "rb");
			}
			if (!fp) {
				path = "/sys/class/hwmon/hwmon1/temp1_max";
				fp = fopen(path, "rb");
			}
			if (fp) { // some heuristic to detect unit
				char temp[32];
				fscanf(fp, "%s", &temp[0]);
				fclose(fp);
				fp = NULL;
				int l = strlen(temp);
				if (l > 2) {
					TEMP_MUL = 1;
					for (i = 0; i < (l - 2); i++)
						TEMP_MUL *= 10;
				} else
					TEMP_MUL = 1;
				cpufound |= showsensor(wp, path, NULL, "CPU", TEMP_MUL, CELSIUS);
			}
		}
		fp = NULL;
		if (!hascore) {
			if (!fp) {
				path = "/sys/class/hwmon/hwmon0/temp1_input";
				fp = fopen(path, "rb");
			}
			if (!fp) {
				path = "/sys/class/hwmon/hwmon0/device/temp1_input";
				fp = fopen(path, "rb");
			}
			if (!fp) {
				path = "/sys/class/hwmon/hwmon0/temp2_input";
				fp = fopen(path, "rb");
			}
			if (!fp) {
				path = "/sys/class/hwmon/hwmon1/temp1_input";
				fp = fopen(path, "rb");
			}
		}
	}
#else
	path = "/sys/class/hwmon/hwmon0/temp1_input";
	fp = fopen("/sys/devices/platform/i2c-0/0-0048/temp1_input", "rb");
#endif
#endif

#ifndef HAVE_IPQ806X
	if (fp != NULL) {
		fp = NULL;
		cpufound |= showsensor(wp, path, NULL, "CPU", TEMP_MUL, CELSIUS);
	}
	if (fpsys != NULL) {
		fclose(fpsys);
		cpufound |= showsensor(wp, path, NULL, "SYS", SYSTEMP_MUL, CELSIUS);
	}
	int a, b;
	for (a = 0; a < 16; a++) {
		char sysfs[64];
		sprintf(sysfs, "/sys/class/hwmon/hwmon%d/", a);
		if (checkhwmon(sysfs))
			continue; // already handled in specific way

		for (b = 0; b < 16; b++) {
			char n[64];
			sprintf(n, "%stemp%d_label", sysfs, b);
			char p[64];
			sprintf(p, "%stemp%d_input", sysfs, b);
			fp = fopen(n, "rb");
			if (fp) {
				char sname[64];
				fscanf(fp, "%s", sname);
				fclose(fp);
				cpufound |= showsensor(wp, p, NULL, sname, 0, CELSIUS);
				continue;
			}
			sprintf(n, "%sname", sysfs);
			fp = fopen(n, "rb");
			if (fp) {
				char sname[64];
				fscanf(fp, "%s", sname);
				fclose(fp);
				sprintf(n, "%sdevice/model", sysfs);
				fp = fopen(n, "rb");
				if (fp) {
					char dname[64];
					fgets(dname, 63, fp);
					fclose(fp);
					sprintf(sname, "%s %s", dname, sname);
					cpufound |= showsensor(wp, p, NULL, sname, 0, CELSIUS);

				} else {
					sprintf(sname, "%s temp%d", sname, b);
					cpufound |= showsensor(wp, p, NULL, sname, 0, CELSIUS);
				}
			}
		}
		for (b = 0; b < 16; b++) {
			char n[64] = { 0 };
			char p[64] = { 0 };
			char driver[64] = { 0 };
			sprintf(n, "%sname", sysfs);
			fp = fopen(n, "rb");
			if (fp) {
				fscanf(fp, "%s", driver);
				fclose(fp);
			}
			sprintf(p, "%sin%d_input", sysfs, b);
			sprintf(n, "%sin%d_label", sysfs, b);
			fp = fopen(n, "rb");
			if (fp) {
				char sname[64];
				fscanf(fp, "%s", sname);
				fclose(fp);
				sprintf(sname, "%s %s", driver, sname);
				cpufound |= showsensor(wp, p, NULL, sname, 1000, VOLT); // volt
			} else {
				char sname[64];
				sprintf(sname, "%s in%d", driver, b);
				cpufound |= showsensor(wp, p, NULL, sname, 1000, VOLT); // volt
			}
		}

		for (b = 0; b < 16; b++) {
			char n[64] = { 0 };
			char p[64] = { 0 };
			char driver[64] = { 0 };
			sprintf(n, "%sname", sysfs);
			fp = fopen(n, "rb");
			if (fp) {
				fscanf(fp, "%s", driver);
				fclose(fp);
			}
			sprintf(p, "%sfan%d_input", sysfs, b);
			sprintf(n, "%sfan%d_label", sysfs, b);
			fp = fopen(n, "rb");
			if (fp) {
				char sname[64];
				fscanf(fp, "%s", sname);
				fclose(fp);
				sprintf(sname, "%s %s", driver, sname);
				cpufound |= showsensor(wp, p, NULL, sname, 1, RPM); // rpm
			} else {
				char sname[64];
				sprintf(sname, "%s fan%d", driver, b);
				cpufound |= showsensor(wp, p, NULL, sname, 1, RPM); // rpm
			}
		}
	}
#endif
	FILE *fp2 = NULL;
#if defined(HAVE_ATH10K) || defined(HAVE_MT76)
	int c = getdevicecount();
	for (i = 0; i < c; i++) {
#if !defined(HAVE_MT76)
		if (nvram_nmatch("disabled", "wlan%d_net_mode", i)) {
			continue;
		}
#endif
		char s_path[64];
		int scan = 0;
		for (scan = 0; scan < 20; scan++) {
			sprintf(s_path, "/sys/class/ieee80211/phy%d/device/hwmon/hwmon%d/temp1_input", i, scan);
			fp2 = fopen(s_path, "rb");
			if (fp2)
				break;
			sprintf(s_path, "/sys/class/ieee80211/phy%d/hwmon%d/temp1_input", i, scan);
			fp2 = fopen(s_path, "rb");
			if (fp2)
				break;
		}

		if (fp2 != NULL) {
			fclose(fp2);
			char name[64];
			sprintf(name, "WLAN%d", i);
			if (!checkhwmon(s_path))
				cpufound |= showsensor(wp, s_path, NULL, name, 1000, CELSIUS);
		}
exit_error:;
	}
#endif
	if (fp)
		fclose(fp);
	if (!cpufound) {
		return 1;
	}
	return 0;
#endif
}

EJ_VISIBLE void ej_show_cpu_temperature(webs_t wp, int argc, char_t **argv)
{
	static int notavailable = -1;

	if (notavailable == -1)
		notavailable = ej_get_cputemp(NULL, argc, argv);
	if (!notavailable) {
		websWrite(wp, "<fieldset>\n");
		{
			websWrite(wp,
				  "<legend><script type=\"text/javascript\">Capture(status_router.cputemp);</script></legend>\n");
			ej_get_cputemp(wp, argc, argv);
		}
		websWrite(wp, "</fieldset><br />\n");
	}
}
#endif
