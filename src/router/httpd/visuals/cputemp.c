/*
 * cputemp.c
 *
 * Copyright (C) 2005 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/statfs.h>
#include <sys/utsname.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <broadcom.h>

#include <wlutils.h>
#include <bcmparams.h>
#include <dirent.h>
#include <netdb.h>
#include <utils.h>
#include <ddnvram.h>
#include <revision.h>
#include <shutils.h>

#ifdef HAVE_CPUTEMP

#define CELSIUS 0
#define VOLT 1
#define RPM 2
#define AMPERE 3
#define WATT 4

typedef struct sensormaps {
	char *name;
	char *map;
} SENSORMAPS;

static SENSORMAPS maps[] = {
	{ "gpio008", "sfp" },
	{ "gpio_fan", "fan" },
	{ "cpu0_thermal", "cpu0" },
	{ "cpu1_thermal", "cpu1" },
	{ "cpu2_thermal", "cpu2" },
	{ "cpu3_thermal", "cpu3" },
	{ "cluster_thermal", "cpu global" },
	{ "lpass_thermal", "audio subsystem" },
	{ "ddrss_thermal", "ddr subsystem" },
	{ "cpu_thermal", "cpu" },
	{ "ubi32_thermal", "nsscore" },
	{ "nss0_thermal", "nsscore0" },
	{ "nss1_thermal", "nsscore1" },
	{ "wcss_phya0_thermal", "wireless subsystem phya0" },
	{ "wcss_phya1_thermal", "wireless subsystem phya1" },
	{ "wcss_phyb0_thermal", "wireless subsystem phyb0" },
	{ "wcss_phyb1_thermal", "wireless subsystem phyb1" },
	{ "nss_top_thermal", "nss system" },
	{ "top_glue_thermal", "system" },
	{ "gephy_thermal", "gbit eth phy" },
	{ "Thermal Zone0", "main" },
	{ "Thermal Zone1", NULL },
	{ "Thermal Zone2", NULL },
	{ "Thermal Zone3", NULL },
	{ "Thermal Zone4", NULL },
	{ "Thermal Zone5", NULL },
	{ "Thermal Zone6", NULL },
	{ "Thermal Zone7", "cpu0" },
	{ "Thermal Zone8", "cpu1" },
	{ "Thermal Zone9", "nss0" },
	{ "Thermal Zone10", "nss1" },
	{ "90000mdio100", "Aquantia Phy0" },
	{ "90000mdio107", "Aquantia Phy1" },
	{ "90000mdio108", "Aquantia Phy1" },
	{ "TX_power", "TX Power" },
	{ "RX_power", "RX Power" },
	{ "bias", "Bias" },
	{ "ath11k_hwmon", NULL }, // indicates that interface is disabled. so we dont show it
};

static const char *getmappedname(const char *name)
{
	int i;
	for (i = 0; i < sizeof(maps) / sizeof(maps[0]); i++) {
		if (!strcmp(maps[i].name, name))
			return maps[i].map;
	}
	return name;
}

struct SENSORS {
	char *path;
	char *syspath;
	char *syspath2;
	int scale;
	int (*method)(void);
	int shown;
	int type;
};
static struct SENSORS *sensors = NULL;
static int opencount = 0;

static FILE *my_fopen(const char *file, char *mode)
{
	FILE *fp = fopen(file, mode);
	if (fp)
		opencount++;
	return fp;
}
static void my_fclose(FILE *fp)
{
	opencount--;
	fclose(fp);
}

static void sensorreset(void)
{
	int cnt = 0;
	if (sensors) {
		while (sensors[cnt].path || sensors[cnt].method) {
			if (sensors[cnt].path)
				free(sensors[cnt].path);
			if (sensors[cnt].syspath)
				free(sensors[cnt].syspath);
			if (sensors[cnt].syspath2)
				free(sensors[cnt].syspath2);
			cnt++;
		}
		free(sensors);
		sensors = NULL;
	}
}
static int singlesensor(const char *sysfs)
{
	char p[64];
	char p2[64];
	char p3[64];
	int cnt = 0;
	int i;
	for (i = 0; i < 16; i++) {
		snprintf(p, sizeof(p) - 1, "%stemp%d_input", sysfs, i);
		snprintf(p2, sizeof(p) - 1, "%sfan%d_input", sysfs, i);
		snprintf(p3, sizeof(p) - 1, "%spower%d_input", sysfs, i);
		if (f_exists(p) || f_exists(p2) || f_exists(p3)) {
			cnt++;
		} else
			return 1;
		if (cnt == 2)
			;
		return 0;
	}
	return 0;
}

static char *gethwmon_base(const char *sysfs)
{
	if (!sysfs)
		return NULL;
	char *sub = strstr(sysfs, "ieee80211");
	if (!sub)
		sub = strstr(sysfs, "hwmon");
	if (!sub)
		return NULL;
	char *idx = strchr(sub, '/');
	if (!idx) {
		return NULL;
	}
	return strdup(idx + 1);
}

static char *gethwmon(const char *sysfs)
{
	if (!sysfs)
		return NULL;
	char *sub = gethwmon_base(sysfs);
	if (!sub)
		return NULL;
	char *idx = strchr(sub, '/');
	if (!idx) {
		free(sub);
		return NULL;
	}
	idx++;
	*idx = 0;
	return sub;
}

static int checkhwmon(const char *sysfs)
{
	if (!sensors)
		return 0;
	char *sub = NULL;
	if (sysfs) {
		sub = gethwmon(sysfs);
		if (!sub)
			return 0;
	}
	int cnt = 0;
	while (sensors[cnt].path || sensors[cnt].method) {
		if (sensors[cnt].path && strstr(sensors[cnt].path, sub)) {
			if (sub)
				free(sub);
			return 1;
		}
		cnt++;
	}
	free(sub);
	return 0;
}
static int alreadyshowed(const char *path)
{
	char *sub = NULL;
	if (!path)
		return 0;
	if (path) {
		if (strstr(path, "/proc/"))
			sub = strdup(path);
		else
			sub = gethwmon_base(path);
		if (!sub)
			return 0;
	}
	int cnt = 0;
	if (sensors) {
		while (sensors[cnt].path) {
			if (sub && sensors[cnt].path && sensors[cnt].shown && !strcmp(sensors[cnt].path, sub)) {
				if (sub)
					free(sub);
				return 1;
			}
			cnt++;
		}
	}
	if (sub)
		free(sub);
	return 0;
}

static int addsensor(const char *path, int (*method)(void), int scale, int type, const char *second)
{
	int cnt = 0;
	char *sub = NULL;
	if (path) {
		if (strstr(path, "/proc/"))
			sub = strdup(path);
		else
			sub = gethwmon_base(path);
	}
	if (sensors) {
		while (sensors[cnt].path || sensors[cnt].method) {
			if (method && sensors[cnt].method == method) {
				sensors[cnt].shown = 1;
				if (sub)
					free(sub);
				return cnt; // already added
			}
			if (sub && sensors[cnt].method) {
				cnt++;
				continue;
			}
			if (sub && !strcmp(sensors[cnt].path, sub)) {
				sensors[cnt].shown = 1;
				if (sub)
					free(sub);
				return cnt; // already added
			}
			cnt++;
		}
	}
	sensors = realloc(sensors, sizeof(struct SENSORS) * (cnt + 2));
	if (sub) {
		sensors[cnt].path = sub;
		if (strstr(sub, "/proc/")) {
			sensors[cnt].syspath = strdup(path);
			if (second)
				sensors[cnt].syspath2 = strdup(second);
			else
				sensors[cnt].syspath2 = NULL;
		} else {
			if (!strncmp(sub + 5, "device", 6))
				asprintf(&sensors[cnt].syspath, "/sys/class/hwmon/%s", sub + 18);
			else if (!strncmp(sub, "phy", 3))
				asprintf(&sensors[cnt].syspath, "/sys/class/hwmon/%s", sub + 5);
			else
				asprintf(&sensors[cnt].syspath, "/sys/class/hwmon/%s", sub);
			if (second)
				sensors[cnt].syspath2 = strdup(second);
			else
				sensors[cnt].syspath2 = NULL;
		}
	} else {
		sensors[cnt].path = NULL;
		sensors[cnt].syspath = NULL;
		sensors[cnt].syspath2 = NULL;
	}
	sensors[cnt].scale = scale;
	sensors[cnt].method = method;
	sensors[cnt].shown = 1;
	sensors[cnt].type = type;
	sensors[cnt + 1].path = NULL;
	sensors[cnt + 1].syspath = NULL;
	sensors[cnt + 1].syspath2 = NULL;
	sensors[cnt + 1].method = NULL;
	return cnt;
}

static int getscale(const char *path)
{
	int cnt = 0;
	char *sub = NULL;
	if (path) {
		sub = gethwmon_base(path);
		if (!sub)
			return 0;
	}
	if (sensors) {
		while (sensors[cnt].path || sensors[cnt].method) {
			if (sensors[cnt].method) {
				cnt++;
				continue;
			}
			if (!strcmp(sensors[cnt].path, sub)) {
				if (sub)
					free(sub);
				return sensors[cnt - 1].scale;
			}
			cnt++;
		}
	}
	if (sub)
		free(sub);
	return 0;
}

float get_scaling(int type, int sensor, int scale)
{
	if (type == CELSIUS)
		return get_temperature((float)sensor / (float)scale);
	return (float)sensor / (float)scale;
}

EJ_VISIBLE void ej_read_sensors(webs_t wp, int argc, char_t **argv)
{
	int cnt = 0;
	if (sensors) {
		while (sensors[cnt].path || sensors[cnt].method) {
			int scale = sensors[cnt].scale;
			int sensor = -1;
			int sensor2 = -1;
			if (sensors[cnt].path) {
				FILE *fp = my_fopen(sensors[cnt].syspath, "rb");
				if (fp) {
					fscanf(fp, "%d", &sensor);
					my_fclose(fp);
				}
			}
			if (sensors[cnt].path && sensors[cnt].syspath2) {
				FILE *fp = my_fopen(sensors[cnt].syspath2, "rb");
				if (fp) {
					fscanf(fp, "%d", &sensor2);
					my_fclose(fp);
				}
			}
			if (sensors[cnt].method)
				sensor = sensors[cnt].method();
			if (wp && scale != -1 && sensor != -1) {
				char *unit = get_temperature_unit();
				if (sensors[cnt].type == VOLT) {
					unit = "Volt";
				}
				if (sensors[cnt].type == AMPERE) {
					unit = "ma";
					scale = 1;
				}
				if (sensors[cnt].type == WATT) {
					unit = "mw";
					scale = 1;
				}
				if (sensors[cnt].type == RPM)
					unit = "rpm";

				if (scale > 1) {
					if (sensor2 != -1)
						websWrite(wp, "{cpu_temp%d::%.1f %s / %.1f %s}", cnt,
							  get_scaling(sensors[cnt].type, sensor, scale), unit,
							  get_scaling(sensors[cnt].type, sensor2, scale), unit);
					else
						websWrite(wp, "{cpu_temp%d::%.1f %s}", cnt,
							  get_scaling(sensors[cnt].type, sensor, scale), unit);

				} else {
					if (sensor2 != -1) {
						if (sensors[cnt].type == RPM || scale == 1)
							websWrite(wp, "{cpu_temp%d::%d %s / %d %s}", cnt, sensor, unit, sensor2,
								  unit);
						else
							websWrite(wp, "{cpu_temp%d::%.1f %s / %.1f %s}", cnt,
								  get_scaling(sensors[cnt].type, sensor, 1), unit,
								  get_scaling(sensors[cnt].type, sensor, 1), unit);

					} else {
						if (sensors[cnt].type == RPM || scale == 1)
							websWrite(wp, "{cpu_temp%d::%d %s}", cnt, sensor, unit);
						else
							websWrite(wp, "{cpu_temp%d::%.1f %s}", cnt,
								  get_scaling(sensors[cnt].type, sensor, 1), unit);
					}
				}
			}
			cnt++;
		}
	}
}

static int showsensor(webs_t wp, const char *path, int (*method)(void), const char *name, int scale, int type, const char *second)
{
	if (alreadyshowed(path)) {
		return 1;
	}
	FILE *fp = my_fopen(path, "rb");
	if (fp || method) {
		int sensor = 0;
		int sensor2 = -1;
		if (fp) {
			fscanf(fp, "%d", &sensor);
			my_fclose(fp);
		}
		if (second) {
			fp = my_fopen(second, "rb");
			if (fp) {
				fscanf(fp, "%d", &sensor2);
				my_fclose(fp);
			}
		}
		if (method)
			sensor = method();
		else {
			if (!scale)
				scale = getscale(path);
		}

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
			int scount;
			int count = addsensor(path, method, scale, type, second);
			char *unit = get_temperature_unit();
			if (type == VOLT) {
				unit = "Volt";
			}
			if (type == AMPERE) {
				unit = "ma";
				scale = 1;
			}
			if (type == WATT) {
				unit = "mw";
				scale = 1;
			}
			if (type == RPM)
				unit = "rpm";
			name = getmappedname(name);
			if (name) {
				websWrite(wp, "<div class=\"setting\">\n");
				websWrite(wp, "<div class=\"label\">%s</div>\n", name);
				websWrite(wp, "<span id=\"cpu_temp%d\">", count);
				if (scale > 1) {
					if (sensor2 != -1)
						websWrite(wp, "%.1f %s / %.1f %s\n", get_scaling(type, sensor, scale), unit,
							  get_scaling(type, sensor2, scale), unit);
					else
						websWrite(wp, "%.1f %s\n", get_scaling(type, sensor, scale), unit);
				} else {
					if (sensor2 != -1) {
						if (type == RPM || scale == 1)
							websWrite(wp, "%d %s / %d %s\n", sensor, unit, sensor2, unit);
						else
							websWrite(wp, "%.1f %s / %d.0 %s\n", get_scaling(type, sensor, 1), unit,
								  get_scaling(type, sensor2, 1), unit);

					} else {
						if (type == RPM || scale == 1)
							websWrite(wp, "%d %s\n", sensor, unit);
						else
							websWrite(wp, "%.1f %s\n", get_scaling(type, sensor, scale), unit);
					}
				}
				websWrite(wp, "</span>&nbsp;\n");
				websWrite(wp, "</div>\n");
			}
		}
		if (sensor == -1)
			return 0;
		return 1;
	}
	return 0;
}

#if defined(HAVE_MVEBU) || defined(HAVE_OCTEON)
static int show_temp(webs_t wp, int mon, int input, const char *name, int two)
{
	char sysfs[128];
	char sysfs2[128];
	snprintf(sysfs, 64, "/sys/class/hwmon/hwmon%d/temp%d_input", mon, input);
	if (two)
		snprintf(sysfs2, 64, "/sys/class/hwmon/hwmon%d/temp%d_input", mon, input + 1);
	return showsensor(wp, sysfs, NULL, name, 1000, CELSIUS, two ? sysfs2 : NULL);
}

#elif defined(HAVE_ALPINE)
static int show_temp(webs_t wp, int mon, int input, char *name, int two)
{
	char sysfs[128];
	char sysfs2[128];
	snprintf(sysfs, 64, "/sys/class/hwmon/hwmon%d/temp%d_input", mon, input);
	if (two)
		snprintf(sysfs2, 64, "/sys/class/hwmon/hwmon%d/temp%d_input", mon, input + 1);
	return showsensor(wp, sysfs, NULL, name, 1, CELSIUS, two ? sysfs2 : NULL);
}
#elif defined(HAVE_IPQ806X)
static int show_temp(webs_t wp, char *name)
{
	char sysfs[128];
	int mon;
	for (mon = 0; mon < 11; mon++) {
		snprintf(sysfs, 64, "/sys/devices/virtual/thermal/thermal_zone%d/temp", mon);
		char sensorname[32];
		snprintf(sensorname, 32, "%s%d", name, mon);
		showsensor(wp, sysfs, NULL, sensorname, 1000, CELSIUS, NULL);
	}
	return 1;
}
#elif defined(HAVE_REALTEK)
static int show_temp(webs_t wp, char *name)
{
	char sysfs[128];
	snprintf(sysfs, 64, "/sys/class/hwmon/hwmon1/temp1_input");
	return showsensor(wp, sysfs, NULL, name, 1000, CELSIUS, NULL);
}
#endif

#ifdef HAVE_X86

int getCoreTemp(const char *p, size_t len, int *ridx, int acpi)
{
	int idx = 0;
	char path[64];
	while (1) {
		int tidx;
		sprintf(path, "/sys/class/hwmon/hwmon%d/name", idx);
		FILE *fp = my_fopen(path, "rb");
		if (!fp) {
			*ridx = -1;
			return 0;
		}
		char name[64];
		fscanf(fp, "%s", name);
		my_fclose(fp);
		if (acpi && !strncmp(name, "acpitz", 6)) {
			snprintf(p, len, "/sys/class/hwmon/hwmon%d", idx);
			*ridx = 0;
			return 1;
		}
		for (tidx = 0; tidx < 32; tidx++) {
			sprintf(path, "/sys/class/hwmon/hwmon%d/temp%d_label", idx, tidx);
			FILE *fp = my_fopen(path, "rb");
			if (!fp)
				continue;
			fscanf(fp, "%s", name);
			if (!strncmp(name, "Core", 4)) {
				my_fclose(fp);
				snprintf(p, len, "/sys/class/hwmon/hwmon%d", idx);
				*ridx = tidx;
				return 1;
			}
			my_fclose(fp);
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
	unsigned int result[3] = { 0, 0, 0 };
	static int tempavg[3] = { 0, 0, 0 };
	static unsigned int tempavg_max = 0;
	int i = idx;
	static int ttcount = 0;
	static int lastidx = 0;
	if (idx < lastidx) {
		if (ttcount)
			tempcount = ttcount;
	}
	strcpy(buf, "phy_tempsense");
	char *ifname = get_wl_instance_name(i);
	if (nvram_nmatch("disabled", "wl%d_net_mode", i) || (ret = wl_ioctl(ifname, WLC_GET_VAR, buf, sizeof(buf)))) {
		return -1;
	}
	ret_int[i] = *(unsigned int *)buf;

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
	lastidx = idx;
	return (tempavg[i] / 2) + 200;
}

static int getwifi0(void)
{
	return getwifi(0);
}
#ifdef HAVE_QTN
#include <qtnapi.h>
#endif
static int getwifi1(void)
{
#ifdef HAVE_QTN
	return rpc_get_temperature() / 100000;
#else
	return getwifi(1);
#endif
}

static int getwifi2(void)
{
	return getwifi(2);
}
#endif

static int get_cputemp(webs_t wp, int argc, char_t **argv)
{
	int i, cpufound = 0;
	FILE *fp = NULL;
	FILE *fpsys = NULL;
	char *path;
	sensorreset();
#ifdef HAVE_MVEBU
	if (getRouterBrand() == ROUTER_WRT_1900AC) {
		cpufound |= show_temp(wp, 0, 1, "CPU", 0);
		cpufound |= show_temp(wp, 2, 1, "DDR", 0);
		cpufound |= show_temp(wp, 2, 2, "WLAN", 0);
		cpufound |= show_temp(wp, 3, 1, "WLAN0", 0);
		cpufound |= show_temp(wp, 4, 1, "WLAN1", 0);
	} else {
		int cpuresult = show_temp(wp, 0, 1, "CPU", 0);
		if (!cpuresult) {
			cpufound |= show_temp(wp, 1, 1, "DDR", 0);
			cpufound |= show_temp(wp, 1, 2, "WLAN", 0);
			cpufound |= show_temp(wp, 2, 1, "WLAN0", 0);
		} else {
			cpufound |= show_temp(wp, 1, 1, "DDR", 0);
			cpufound |= show_temp(wp, 1, 2, "WLAN", 0);
			cpufound |= show_temp(wp, 2, 1, "WLAN1", 0);
		}
		cpufound |= show_temp(wp, 3, 1, "WLAN2", 0);
	}
	return 0;
#endif
#ifdef HAVE_OCTEON
	cpufound |= show_temp(wp, 0, 1, "BOARD", 0);
	cpufound |= show_temp(wp, 1, 1, "CPU", 0);
	cpufound |= show_temp(wp, 0, 2, "PHY1", 0);
	cpufound |= show_temp(wp, 1, 2, "PHY2", 0);
#endif
#ifdef HAVE_ALPINE
	cpufound |= show_temp(wp, 1, 1, "CPU", 0);
#elif defined(HAVE_IPQ806X)
	char *wifiname0 = getWifiDeviceName("wlan0", NULL);
	char *wifiname1 = getWifiDeviceName("wlan1", NULL);
	if (wifiname0) {
		cpufound |= show_temp(wp, "Thermal Zone");
	}
#endif
#ifdef HAVE_BCMMODERN
	char buf[WLC_IOCTL_SMLEN];
	int ret;
	unsigned int present[3] = { 0, 0, 0 };
	int cc = get_wl_instances();
	for (i = 0; i < cc; i++) {
		strcpy(buf, "phy_tempsense");
		char *ifname = get_wl_instance_name(i);
		if (nvram_nmatch("disabled", "wl%d_net_mode", i) || (ret = wl_ioctl(ifname, WLC_GET_VAR, buf, sizeof(buf)))) {
			present[i] = 0;
			continue;
		}
		present[i] = 1;
	}
#ifdef HAVE_QTN
	present[1] = 1;
#endif
#ifdef HAVE_NORTHSTAR
	if (f_exists("/proc/dmu/temperature")) {
		cpufound |= showsensor(wp, "/proc/dmu/temperature", NULL, "CPU", 10, CELSIUS, NULL);
	}
#endif
#ifdef HAVE_BRCMFMAC
	cpufound |= showsensor(wp, "/sys/class/hwmon/hwmon0/temp1_input", NULL, "WLAN0", 1000, CELSIUS, NULL);
	cpufound |= showsensor(wp, "/sys/class/hwmon/hwmon1/temp1_input", NULL, "WLAN1", 1000, CELSIUS, NULL);
	cpufound |= showsensor(wp, "/sys/class/hwmon/hwmon2/temp1_input", NULL, "WLAN2", 1000, CELSIUS, NULL);
#else
	if (!present[0] && !present[1] && !present[2] && !cpufound)
		return 1;
	else {
		for (i = 0; i < cc; i++) {
			if (present[i]) {
				char wl[32];
				sprintf(wl, "WL%d", i);
				if (i == 0)
					cpufound |= showsensor(wp, NULL, getwifi0, wl, 10, CELSIUS, NULL);
				if (i == 1)
					cpufound |= showsensor(wp, NULL, getwifi1, wl, 10, CELSIUS, NULL);
				if (i == 2)
					cpufound |= showsensor(wp, NULL, getwifi2, wl, 10, CELSIUS, NULL);
			}
		}
	}
#endif
#else
	int TEMP_MUL = 1000;
	int SYSTEMP_MUL = 1000;
#ifdef HAVE_GATEWORX
	TEMP_MUL = 100;
	if (getRouterBrand() == ROUTER_BOARD_GATEWORX_SWAP)
		TEMP_MUL = 200;
	path = "/sys/devices/platform/IXP4XX-I2C.0/i2c-adapter:i2c-0/0-0028/temp_input";
	fp = my_fopen(path, "rb");
	if (!fp) {
		path = "/sys/devices/platform/IXP4XX-I2C.0/i2c-0/0-0028/temp1_input";
		fp = my_fopen(path, "rb");
	}
#elif HAVE_LAGUNA
	TEMP_MUL = 10;
	path = "/sys/bus/i2c/devices/0-0029/temp0_input";
	fp = my_fopen(path, "rb");
#elif HAVE_UNIWIP
	path = "/sys/bus/i2c/devices/0-0049/temp1_input";
	fp = my_fopen(path, "rb");
#elif HAVE_VENTANA
	SYSTEMP_MUL = 10;
	path = "/sys/class/hwmon/hwmon1/temp1_input";
	fp = my_fopen(path, "rb");
	if (!fp) {
		path = "/sys/class/hwmon/hwmon0/temp1_input";
		fp = my_fopen(path, "rb");
	}
	char *pathsys = "/sys/class/hwmon/hwmon0/temp0_input";
	fpsys = my_fopen(pathsys, "rb");
#ifdef HAVE_NEWPORT
	if (!fpsys) {
		SYSTEMP_MUL = 1000;
		pathsys = "/sys/class/hwmon/hwmon0/temp2_input";
		fpsys = my_fopen(path, "rb");
	}
#endif

#else
#ifdef HAVE_X86

	fp = my_fopen("/sys/devices/platform/i2c-1/1-0048/temp1_input", "rb");
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
			cpufound |= showsensor(wp, tempp, NULL, "CPU", 1000, CELSIUS, NULL);
			TEMP_MUL = 1000;
		} else if (getCoreTemp(s_path, sizeof(s_path), &idx, 1)) {
			char maxp[64];
			sprintf(tempp, "%s/temp%d_input", s_path, idx);
			hascore = 1;
			cpufound |= showsensor(wp, tempp, NULL, "CPU", 1000, CELSIUS, NULL);
			TEMP_MUL = 1000;
		}
		fp = NULL;
		if (!hascore) {
			if (!fp) {
				path = "/sys/class/hwmon/hwmon0/temp1_input";
				fp = my_fopen(path, "rb");
			}
			if (!fp) {
				path = "/sys/class/hwmon/hwmon0/device/temp1_input";
				fp = my_fopen(path, "rb");
			}
			if (!fp) {
				path = "/sys/class/hwmon/hwmon0/temp2_input";
				fp = my_fopen(path, "rb");
			}
			if (!fp) {
				path = "/sys/class/hwmon/hwmon1/temp1_input";
				fp = my_fopen(path, "rb");
			}
		}
	}
#else
	path = "/sys/class/hwmon/hwmon0/temp1_input";
	fp = my_fopen("/sys/devices/platform/i2c-0/0-0048/temp1_input", "rb");
#endif
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
		char s_path[128];
		char s_path2[128];
		int scan = 0;
		for (scan = 0; scan < 20; scan++) {
			sprintf(s_path, "/sys/class/ieee80211/phy%d/device/hwmon/hwmon%d/temp1_input", i, scan);
			fp2 = my_fopen(s_path, "rb");
			if (fp2)
				break;
			sprintf(s_path, "/sys/class/ieee80211/phy%d/hwmon%d/temp1_input", i, scan);
			fp2 = my_fopen(s_path, "rb");
			if (fp2)
				break;
		}
		int two = 0;
		sprintf(s_path2, "/sys/class/ieee80211/phy%d/hwmon%d/temp2_input", i, scan);
		FILE *fp3 = my_fopen(s_path2, "rb");
		if (fp3) {
			two = 1;
			fclose(fp3);
		}
		if (fp2 != NULL) {
			my_fclose(fp2);
			char name[64];
			sprintf(name, "WLAN%d", i);
			if (!checkhwmon(s_path))
				cpufound |= showsensor(wp, s_path, NULL, name, 1000, CELSIUS, two ? s_path2 : NULL);
		}
exit_error:;
	}
#endif

#if !defined(HAVE_IPQ806X) && !defined(HAVE_PB42) && !defined(HAVE_LSX)
	if (fp != NULL) {
		my_fclose(fp);
		cpufound |= showsensor(wp, path, NULL, "CPU", TEMP_MUL, CELSIUS, NULL);
	}
	if (fpsys != NULL) {
		my_fclose(fpsys);
		cpufound |= showsensor(wp, path, NULL, "SYS", SYSTEMP_MUL, CELSIUS, NULL);
	}
	int a, b;
	for (a = 0; a < 32; a++) {
		char sysfs[64];
		sprintf(sysfs, "/sys/class/hwmon/hwmon%d/", a);
		if (checkhwmon(sysfs)) {
			continue; // already handled in specific way
		}

		for (b = 0; b < 16; b++) {
			char n[64];
			sprintf(n, "%stemp%d_label", sysfs, b);
			char p[64];
			sprintf(p, "%stemp%d_input", sysfs, b);
			fp = my_fopen(n, "rb");
			if (fp) {
				char sname[64];
				fscanf(fp, "%s", sname);
				my_fclose(fp);
				cpufound |= showsensor(wp, p, NULL, sname, 0, CELSIUS, NULL);
				continue;
			}
			sprintf(n, "%sname", sysfs);
			fp = my_fopen(n, "rb");
			if (fp) {
				char sname[64];
				fscanf(fp, "%s", sname);
				my_fclose(fp);
				sprintf(n, "%sdevice/model", sysfs);
				fp = my_fopen(n, "rb");
				if (fp) {
					char dname[64];
					fgets(dname, 63, fp);
					my_fclose(fp);
					sprintf(sname, "%s %s", dname, sname);
					cpufound |= showsensor(wp, p, NULL, sname, 0, CELSIUS, NULL);

				} else {
					int single = singlesensor(sysfs);
					if (!single)
						sprintf(sname, "%s temp%d", sname, b);
					if (!checkhwmon(sname))
						cpufound |= showsensor(wp, p, NULL, sname, 0, CELSIUS, NULL);
				}
			} else
				break;
		}
		for (b = 0; b < 16; b++) {
			char n[64] = { 0 };
			char p[64] = { 0 };
			char driver[64] = { 0 };
			int f = 0;
			sprintf(n, "%sname", sysfs);
			fp = my_fopen(n, "rb");
			if (fp) {
				fscanf(fp, "%s", driver);
				my_fclose(fp);
			} else
				break;
			sprintf(p, "%sin%d_input", sysfs, b);
			sprintf(n, "%sin%d_label", sysfs, b);
			fp = my_fopen(n, "rb");
			if (fp) {
				char sname[64];
				fscanf(fp, "%s", sname);
				my_fclose(fp);
				sprintf(sname, "%s %s", driver, sname);
				cpufound |= showsensor(wp, p, NULL, sname, 1000, VOLT, NULL); // volt
			} else {
				char sname[64];
				int single = singlesensor(sysfs);
				if (!single)
					sprintf(sname, "%s in%d", driver, b);
				else
					sprintf(sname, "%s", driver);
				cpufound |= showsensor(wp, p, NULL, sname, 1000, VOLT, NULL); // volt
			}
		}

		for (b = 0; b < 16; b++) {
			char n[64] = { 0 };
			char p[64] = { 0 };
			char driver[64] = { 0 };
			sprintf(n, "%sname", sysfs);
			fp = my_fopen(n, "rb");
			if (fp) {
				fscanf(fp, "%s", driver);
				my_fclose(fp);
			} else
				break;
			sprintf(p, "%sfan%d_input", sysfs, b);
			sprintf(n, "%sfan%d_label", sysfs, b);
			fp = my_fopen(n, "rb");
			if (fp) {
				char sname[64];
				fscanf(fp, "%s", sname);
				my_fclose(fp);
				sprintf(sname, "%s %s", driver, sname);
				cpufound |= showsensor(wp, p, NULL, sname, 1, RPM, NULL); // rpm
			} else {
				char sname[64];
				int single = singlesensor(sysfs);
				if (!single)
					sprintf(sname, "%s fan%d", driver, b);
				else
					sprintf(sname, "%s", driver);
				cpufound |= showsensor(wp, p, NULL, sname, 1, RPM, NULL); // rpm
			}
		}

		for (b = 0; b < 16; b++) {
			char n[64] = { 0 };
			char p[64] = { 0 };
			char driver[64] = { 0 };
			sprintf(n, "%sname", sysfs);
			fp = my_fopen(n, "rb");
			if (fp) {
				fscanf(fp, "%s", driver);
				my_fclose(fp);
			} else
				break;
			sprintf(p, "%spower%d_input", sysfs, b);
			sprintf(n, "%spower%d_label", sysfs, b);
			fp = my_fopen(n, "rb");
			if (fp) {
				char sname[64];
				fscanf(fp, "%s", sname);
				my_fclose(fp);
				sprintf(sname, "%s %s", driver, sname);
				cpufound |= showsensor(wp, p, NULL, sname, 1, WATT, NULL);
			} else {
				char sname[64];
				int single = singlesensor(sysfs);
				if (!single)
					sprintf(sname, "%s power%d", driver, b);
				else
					sprintf(sname, "%s", driver);
				cpufound |= showsensor(wp, p, NULL, sname, 1, WATT, NULL);
			}
		}

		for (b = 0; b < 16; b++) {
			char n[64] = { 0 };
			char p[64] = { 0 };
			char driver[64] = { 0 };
			sprintf(n, "%sname", sysfs);
			fp = my_fopen(n, "rb");
			if (fp) {
				fscanf(fp, "%s", driver);
				my_fclose(fp);
			} else
				break;
			sprintf(p, "%scurr%d_input", sysfs, b);
			sprintf(n, "%scurr%d_label", sysfs, b);
			fp = my_fopen(n, "rb");
			if (fp) {
				char sname[64];
				fscanf(fp, "%s", sname);
				my_fclose(fp);
				sprintf(sname, "%s %s", driver, sname);
				cpufound |= showsensor(wp, p, NULL, sname, 1, AMPERE, NULL);
			} else {
				char sname[64];
				int single = singlesensor(sysfs);
				if (!single)
					sprintf(sname, "%s current%d", driver, b);
				else
					sprintf(sname, "%s", driver);
				cpufound |= showsensor(wp, p, NULL, sname, 1, AMPERE, NULL);
			}
		}
	}
#endif
#endif
	if (!cpufound) {
		return 1;
	}
	return 0;
}

EJ_VISIBLE void ej_show_cpu_temperature(webs_t wp, int argc, char_t **argv)
{
	static int notavailable = -1;

	if (notavailable == -1)
		notavailable = get_cputemp(NULL, argc, argv);
	if (!notavailable) {
		websWrite(wp, "<fieldset>\n");
		{
			websWrite(wp,
				  "<legend><script type=\"text/javascript\">Capture(status_router.cputemp);</script></legend>\n");
			get_cputemp(wp, argc, argv);
		}
		websWrite(wp, "</fieldset><br />\n");
	}
}
#endif