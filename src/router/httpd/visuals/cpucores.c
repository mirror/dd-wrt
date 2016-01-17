/*
 * cpucores.c
 *
 * Copyright (C) 2005 - 2016 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#ifdef HAVE_STORM
#define HARDFREQ "300"
#elif HAVE_OPENRISC
#define HARDFREQ "166"
#elif HAVE_LAGUNA
#define FREQLINE 2
#elif HAVE_RT3052
#ifdef HAVE_HOTPLUG2
#define FREQLINE 7
#else
#define FREQLINE 4
#endif
#elif defined(HAVE_DANUBE)
#define FREQLINE 6
#elif HAVE_RT2880
#ifdef HAVE_HOTPLUG2
#define FREQLINE 7
#else
#define FREQLINE 4
#endif
#elif HAVE_XSCALE
#define FREQLINE 2
#elif defined(HAVE_MAGICBOX) || defined(HAVE_RB600)
#define FREQLINE 3
#elif defined(HAVE_FONERA) || defined(HAVE_SOLO51) || defined(HAVE_ADM5120) || defined(HAVE_MERAKI) || defined(HAVE_LS2) || defined(HAVE_LS5) || defined(HAVE_WHRAG108) || defined(HAVE_TW6600) || defined(HAVE_CA8) || defined(HAVE_RB500) || defined(HAVE_OCTEON)
#ifdef HAVE_HOTPLUG2
#define FREQLINE 5
#else
#define FREQLINE 4
#endif
#elif defined(HAVE_PB42) || defined(HAVE_LSX)
#define FREQLINE 5
#elif HAVE_VENTANA
void ej_get_clkfreq(webs_t wp, int argc, char_t ** argv)
{
	FILE *fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq", "rb");
	if (fp) {
		int freq;
		fscanf(fp, "%d", &freq);
		fclose(fp);
		websWrite(wp, "%d", freq / 1000);
	} else {
		websWrite(wp, "800");
		return;
	}
}
#elif HAVE_MVEBU
void ej_get_clkfreq(webs_t wp, int argc, char_t ** argv)
{
	FILE *fp = fopen("/sys/kernel/debug/clk/cpuclk/clk_rate", "rb");
	if (fp) {
		int freq;
		fscanf(fp, "%d", &freq);
		fclose(fp);
		websWrite(wp, "%d", freq / 1000000);
	} else {
		websWrite(wp, "1200");

	}
	return;
}
#elif HAVE_IPQ806X
void ej_get_clkfreq(webs_t wp, int argc, char_t ** argv)
{
	FILE *fp = fopen("/sys/kernel/debug/clk/krait0_pri_mux/clk_rate", "rb");
	if (fp) {
		int freq;
		fscanf(fp, "%d", &freq);
		fclose(fp);
		websWrite(wp, "%d", freq / 1000000);
	} else {
		websWrite(wp, "1400");

	}
	return;
}
#elif HAVE_X86
void ej_get_clkfreq(webs_t wp, int argc, char_t ** argv)
{
	FILE *fp = fopen("/proc/cpuinfo", "rb");

	if (fp == NULL) {
		websWrite(wp, "unknown");
		return;
	}
	int cnt = 0;
	int b = 0;

	while (b != EOF) {
		b = getc(fp);
		if (b == ':')
			cnt++;
		if (cnt == 8) {
			getc(fp);
			char cpuclk[32];
			int i = 0;

			b = getc(fp);
			while (b != 0xa && b != 0xd && b != 0x20) {
				cpuclk[i++] = b;
				b = getc(fp);
			}
			cpuclk[i++] = 0;
			websWrite(wp, cpuclk);
			fclose(fp);
			return;
		}
	}

	fclose(fp);
	websWrite(wp, "unknown");
	return;
}
#else
void ej_get_clkfreq(webs_t wp, int argc, char_t ** argv)
{
	char *clk = nvram_get("clkfreq");

	if (clk == NULL) {
		if (getcpurev() == 0)	//BCM4710
			websWrite(wp, "125");
		else if (getcpurev() == 29)	//BCM5354
			websWrite(wp, "240");
		else
			websWrite(wp, "unknown");
		return;
	}
	char buf[64];

	strcpy(buf, clk);
	int i = 0;

	while (buf[i++] != 0) {
		if (buf[i] == ',')
			buf[i] = 0;
	}
	websWrite(wp, buf);
	return;
}
#endif

#if defined(FREQLINE)
void ej_get_clkfreq(webs_t wp, int argc, char_t ** argv)
{
	FILE *fp = fopen("/proc/cpuinfo", "rb");

	if (fp == NULL) {
		websWrite(wp, "unknown");
		return;
	}
	int cnt = 0;
	int b = 0;

	while (b != EOF) {
		b = getc(fp);
		if (b == ':')
			cnt++;

		if (cnt == FREQLINE) {
			getc(fp);
			char cpuclk[7];
			int i;
			for (i = 0; i < 6; i++) {
				int c = getc(fp);
				if (c == EOF || c == '\n' || c == '.' || c == 0)
					break;
				cpuclk[i] = c;
			}
			cpuclk[i] = 0;
			websWrite(wp, cpuclk);
			fclose(fp);
			return;
		}
	}

	fclose(fp);
	websWrite(wp, "unknown");
	return;
}

#undef FREQLINE
#elif defined(HARDFREQ)
void ej_get_clkfreq(webs_t wp, int argc, char_t ** argv)
{
	websWrite(wp, HARDFREQ);
	return;
}

#undef HARDFREQ
#endif

void ej_show_cpuinfo(webs_t wp, int argc, char_t ** argv)
{

#ifdef HAVE_IPR
	char *str = "rev 1.2";
#else
	char *str = cpustring();
#endif
	if (!str) {
		websWrite(wp, "Not Detected!\n");
		return;
	}
	websWrite(wp, str);
}

void ej_show_cpucores(webs_t wp, int argc, char_t ** argv)
{
	int count = 1;
#ifdef _SC_NPROCESSORS_ONLN
	int cpucount = sysconf(_SC_NPROCESSORS_ONLN);
	if (cpucount > 1) {
		count = cpucount;
	}
#endif
	websWrite(wp, "%d", count);
}
