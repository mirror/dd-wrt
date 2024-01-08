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

#if defined(HAVE_MVEBU) || defined(HAVE_OCTEON)
static int show_temp(webs_t wp, int mon, int input, char *fmt)
{
	char sysfs[64];
	snprintf(sysfs, 64, "/sys/class/hwmon/hwmon%d/temp%d_input", mon, input);
	FILE *tempfp = fopen(sysfs, "rb");
	if (tempfp) {
		int cpu;
		fscanf(tempfp, "%d", &cpu);
		fclose(tempfp);
		if (cpu > 0) {
			websWrite(wp, fmt, cpu / 1000, (cpu % 1000) / 100);
			return 1;
		}
	}
	return 0;
}

#elif defined(HAVE_ALPINE)
static int show_temp(webs_t wp, int mon, int input, char *fmt)
{
	char sysfs[64];
	snprintf(sysfs, 64, "/sys/class/hwmon/hwmon%d/temp%d_input", mon, input);
	FILE *tempfp = fopen(sysfs, "rb");
	if (tempfp) {
		int cpu;
		fscanf(tempfp, "%d", &cpu);
		fclose(tempfp);
		websWrite(wp, fmt, cpu, 0);
		return 1;
	}
	return 0;
}
#elif defined(HAVE_IPQ806X)
static int show_temp(webs_t wp, char *fmt)
{
	char sysfs[64];
	int mon;
	int temperature = -255;
	for (mon = 0; mon < 11; mon++) {
		snprintf(sysfs, 64, "/sys/devices/virtual/thermal/thermal_zone%d/temp", mon);
		FILE *tempfp = fopen(sysfs, "rb");
		if (tempfp) {
			if (temperature == -255)
				temperature = 0;
			int cpu;
			fscanf(tempfp, "%d", &cpu);
			fclose(tempfp);
			temperature += cpu;
		}
	}
	if (temperature != -255) {
		temperature /= mon;
		websWrite(wp, fmt, temperature / 1000, temperature % 1000);
		return 1;
	}
	return 0;
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

EJ_VISIBLE void ej_get_cputemp(webs_t wp, int argc, char_t **argv)
{
	int i, cpufound = 0;
	int disable_wifitemp = 0;
	FILE *fp = NULL;
	FILE *fpsys = NULL;
#ifdef HAVE_MVEBU
	if (getRouterBrand() == ROUTER_WRT_1900AC) {
		show_temp(wp, 0, 1, "CPU %d.%d &#176;C");
		show_temp(wp, 2, 1, " / WL0 %d.%d &#176;C");
		show_temp(wp, 2, 2, " / WL1 %d.%d &#176;C");
	} else {
		int cpuresult = show_temp(wp, 1, 1, "CPU %d.%d &#176;C");
		if (!cpuresult)
			show_temp(wp, 0, 1, "WL0 %d.%d &#176;C");
		else
			show_temp(wp, 0, 1, " / WL0 %d.%d &#176;C");
		show_temp(wp, 0, 2, " / WL1 %d.%d &#176;C");
	}
	return;
#endif
#ifdef HAVE_OCTEON
	cpufound |= show_temp(wp, 0, 1, "BOARD %d.%d &#176;C");
	cpufound |= show_temp(wp, 1, 1, " / CPU %d.%d &#176;C");
	cpufound |= show_temp(wp, 0, 2, " / PHY1 %d.%d &#176;C");
	cpufound |= show_temp(wp, 1, 2, " / PHY2 %d.%d &#176;C");
#endif
#ifdef HAVE_ALPINE
	show_temp(wp, 1, 1, "CPU %d.%d &#176;C");
	cpufound = 1;
#elif defined(HAVE_IPQ806X)
	char *wifiname0 = getWifiDeviceName("wlan0", NULL);
	char *wifiname1 = getWifiDeviceName("wlan1", NULL);
	//      if ((wifiname0 && !strcmp(wifiname0, "QCA99X0 802.11ac")) || (wifiname1 && !strcmp(wifiname1, "QCA99X0 802.11ac"))) {
	//              disable_wifitemp = 1;
	//      }
	if (wifiname0) {
		cpufound = show_temp(wp, "CPU %d.%d &#176;C");
	} else {
		disable_wifitemp = 1;
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
	cpufound = 1;

#ifdef HAVE_QTN
	result[1] = rpc_get_temperature() / 100000;
	present[1] = 1;
#endif
	int cputemp = 1;
#ifdef HAVE_NORTHSTAR
	cputemp = 0;
	fp = fopen("/proc/dmu/temperature", "rb");
	if (fp) {
		fscanf(fp, "%d", &cputemp);
		fclose(fp);
		fp = NULL;
		websWrite(wp, "CPU %d.%d &#176;C / ", cputemp / 10, cputemp % 10);
	}
#endif
	if (!present[0] && !present[1] && !present[2] && cputemp)
		websWrite(wp, "%s", live_translate(wp, "status_router.notavail")); // no
	else {
		for (i = 0; i < cc; i++) {
			if (present[i]) {
				if (i && present[i - 1])
					websWrite(wp, " / ");
				websWrite(wp, "WL%d %d.%d &#176;C", i, result[i] / 10, result[i] % 10);
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

	fp = fopen("/sys/devices/platform/IXP4XX-I2C.0/i2c-adapter:i2c-0/0-0028/temp_input", "rb");
	if (!fp)
		fp = fopen("/sys/devices/platform/IXP4XX-I2C.0/i2c-0/0-0028/temp1_input", "rb");
#elif HAVE_LAGUNA
	TEMP_MUL = 10;
	fp = fopen("/sys/bus/i2c/devices/0-0029/temp0_input", "rb");
#elif HAVE_UNIWIP
	fp = fopen("/sys/bus/i2c/devices/0-0049/temp1_input", "rb");
#elif HAVE_VENTANA
	SYSTEMP_MUL = 10;
	fp = fopen("/sys/class/hwmon/hwmon1/temp1_input", "rb");
	if (!fp)
		fp = fopen("/sys/class/hwmon/hwmon0/temp1_input", "rb");
	fpsys = fopen("/sys/class/hwmon/hwmon0/temp0_input", "rb");
#ifdef HAVE_NEWPORT
	if (!fpsys) {
		SYSTEMP_MUL = 1000;
		fpsys = fopen("/sys/class/hwmon/hwmon0/temp2_input", "rb");
	}
#endif

#else
#ifdef HAVE_X86

	fp = fopen("/sys/devices/platform/i2c-1/1-0048/temp1_input", "rb");
	if (!fp) {
		TEMP_MUL = 100;

		char path[64];
		int idx = 0;
		int hascore = 0;
		char tempp[64];
		if (getCoreTemp(path, sizeof(path), &idx, 0)) {
			char maxp[64];
			sprintf(tempp, "%s/temp%d_input", path, idx);
			hascore = 1;
			TEMP_MUL = 1000;
		} else if (getCoreTemp(path, sizeof(path), &idx, 1)) {
			char maxp[64];
			sprintf(tempp, "%s/temp%d_input", path, idx);
			hascore = 1;
			TEMP_MUL = 1000;
		}
		if (TEMP_MUL == 100) {
			if (!fp)
				fp = fopen("/sys/class/hwmon/hwmon0/temp1_max", "rb");
			if (!fp)
				fp = fopen("/sys/class/hwmon/hwmon0/device/temp1_max", "rb");
			if (!fp)
				fp = fopen("/sys/class/hwmon/hwmon0/temp2_max", "rb");
			if (!fp)
				fp = fopen("/sys/class/hwmon/hwmon1/temp1_max", "rb");
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
			}
		}
		fp = NULL;
		if (hascore)
			fp = fopen(tempp, "rb");
		if (!fp)
			fp = fopen("/sys/class/hwmon/hwmon0/temp1_input", "rb");
		if (!fp)
			fp = fopen("/sys/class/hwmon/hwmon0/device/temp1_input", "rb");
		if (!fp)
			fp = fopen("/sys/class/hwmon/hwmon0/temp2_input", "rb");
		if (!fp)
			fp = fopen("/sys/class/hwmon/hwmon1/temp1_input", "rb");
	}
#else
	fp = fopen("/sys/devices/platform/i2c-0/0-0048/temp1_input", "rb");
#endif
#endif

#ifndef HAVE_IPQ806X
	if (fp != NULL) {
		cpufound = 1;
		int temp;
		fscanf(fp, "%d", &temp);
		fclose(fp);
		fp = NULL;
		int high = temp / TEMP_MUL;
		int low;
		if (TEMP_MUL > 10)
			low = (temp - (high * TEMP_MUL)) / (TEMP_MUL / 10);
		else
			low = 0;
		websWrite(wp, "CPU %d.%d &#176;C", high,
			  low); // no i2c lm75 found
	}
	if (fpsys != NULL) {
		if (cpufound) {
			websWrite(wp, " / ");
		}
		cpufound = 1;
		int temp;
		fscanf(fpsys, "%d", &temp);
		fclose(fpsys);
		int high = temp / SYSTEMP_MUL;
		int low;
		if (SYSTEMP_MUL > 10)
			low = (temp - (high * SYSTEMP_MUL)) / (SYSTEMP_MUL / 10);
		else
			low = 0;
		websWrite(wp, "SYS %d.%d &#176;C", high,
			  low); // no i2c lm75 found
	}
#endif
	FILE *fp2 = NULL;
#if defined(HAVE_ATH10K) || defined(HAVE_MT76)
	if (!disable_wifitemp) {
		int c = getdevicecount();
		for (i = 0; i < c; i++) {
#if !defined(HAVE_MT76)
			if (nvram_nmatch("disabled", "wlan%d_net_mode", i)) {
				continue;
			}
#endif
			char path[64];
			int scan = 0;
			for (scan = 0; scan < 20; scan++) {
				sprintf(path, "/sys/class/ieee80211/phy%d/device/hwmon/hwmon%d/temp1_input", i, scan);
				fp2 = fopen(path, "rb");
				if (fp2)
					break;
				sprintf(path, "/sys/class/ieee80211/phy%d/hwmon%d/temp1_input", i, scan);
				fp2 = fopen(path, "rb");
				if (fp2)
					break;
			}

			if (fp2 != NULL) {
				int temp;
				fscanf(fp2, "%d", &temp);
				fclose(fp2);
				if (temp < 0)
					goto exit_error;
				if (cpufound) {
					websWrite(wp, " / ");
				}
				int temperature = temp / 1000;
				if (temperature < 0 || temperature > 200)
					websWrite(wp, "wlan%d %s", i, live_translate(wp, "status_router.notavail"));
				else
					websWrite(wp, "wlan%d %d &#176;C", i, temp / 1000);
				cpufound = 1;
			}
exit_error:;
		}
	}
#endif
	if (fp)
		fclose(fp);
	if (!cpufound)
		websWrite(wp, "%s", live_translate(wp, "status_router.notavail")); // no

#endif
}

EJ_VISIBLE void ej_show_cpu_temperature(webs_t wp, int argc, char_t **argv)
{
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "status_router.cputemp", NULL);
	websWrite(wp, "<span id=\"cpu_temp\">");
	ej_get_cputemp(wp, argc, argv);
	websWrite(wp, "</span>&nbsp;\n");
	websWrite(wp, "</div>\n");
}
#endif
