/*
 * cpucores.c
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
#elif defined(HAVE_FONERA) || defined(HAVE_SOLO51) || defined(HAVE_ADM5120) || defined(HAVE_MERAKI) || defined(HAVE_LS2) || \
	defined(HAVE_LS5) || defined(HAVE_WHRAG108) || defined(HAVE_TW6600) || defined(HAVE_CA8) || defined(HAVE_RB500) ||  \
	defined(HAVE_OCTEON)
#ifdef HAVE_HOTPLUG2
#define FREQLINE 5
#else
#define FREQLINE 4
#endif
#elif defined(HAVE_PB42) || defined(HAVE_LSX)
#define FREQLINE 5
#elif HAVE_IPQ6018
EJ_VISIBLE void ej_get_clkfreq(webs_t wp, int argc, char_t **argv)
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
#elif HAVE_VENTANA
EJ_VISIBLE void ej_get_clkfreq(webs_t wp, int argc, char_t **argv)
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
EJ_VISIBLE void ej_get_clkfreq(webs_t wp, int argc, char_t **argv)
{
	FILE *fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq", "rb");
	if (fp) {
		int freq;
		fscanf(fp, "%d", &freq);
		fclose(fp);
		websWrite(wp, "%d", freq / 1000);
		return;
	}
	fp = fopen("/sys/kernel/debug/clk/cpuclk/clk_rate", "rb");
	if (fp) {
		int freq;
		fscanf(fp, "%d", &freq);
		fclose(fp);
		websWrite(wp, "%d", freq / 1000000);
		return;
	}

	websWrite(wp, "1200");

	return;
}
#elif HAVE_ALPINE
EJ_VISIBLE void ej_get_clkfreq(webs_t wp, int argc, char_t **argv)
{
	FILE *fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq", "rb");
	if (fp) {
		int freq;
		fscanf(fp, "%d", &freq);
		fclose(fp);
		websWrite(wp, "%d", freq / 1000);
		return;
	} else {
		websWrite(wp, "1700");
	}
	return;
}
#elif HAVE_IPQ806X
EJ_VISIBLE void ej_get_clkfreq(webs_t wp, int argc, char_t **argv)
{
	FILE *fp2 = fopen("/sys/kernel/debug/clk/krait1_pri_mux/clk_rate", "rb");
	FILE *fp = fopen("/sys/kernel/debug/clk/krait0_pri_mux/clk_rate", "rb");
	FILE *fp3 = fopen("/sys/devices/system/cpu/cpufreq/policy0/cpuinfo_cur_freq", "rb");
	if (fp && fp2) {
		int freq;
		fscanf(fp, "%d", &freq);
		int freq2;
		fscanf(fp2, "%d", &freq2);
		if (argc && !strcmp(argv[0], "1"))
			websWrite(wp, "%d", freq / 1000000);
		else
			websWrite(wp, "%d MHz / %d", freq / 1000000, freq2 / 1000000);
	} else if (fp3) {
		int freq;
		fscanf(fp3, "%d", &freq);
		websWrite(wp, "%d", freq / 1000);
	} else {
		websWrite(wp, "1400");
	}
	if (fp)
		fclose(fp);
	if (fp2)
		fclose(fp2);
	if (fp3)
		fclose(fp3);

	return;
}
#elif HAVE_X86
EJ_VISIBLE void ej_get_clkfreq(webs_t wp, int argc, char_t **argv)
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
EJ_VISIBLE void ej_get_clkfreq(webs_t wp, int argc, char_t **argv)
{
	char *clk = nvram_safe_get("clkfreq");

	if (!*clk) {
		if (getcpurev() == 0) //BCM4710
			websWrite(wp, "125");
		else if (getcpurev() == 29) //BCM5354
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
EJ_VISIBLE void ej_get_clkfreq(webs_t wp, int argc, char_t **argv)
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
EJ_VISIBLE void ej_get_clkfreq(webs_t wp, int argc, char_t **argv)
{
	websWrite(wp, HARDFREQ);
	return;
}

#undef HARDFREQ
#endif

EJ_VISIBLE void ej_show_cpuinfo(webs_t wp, int argc, char_t **argv)
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

EJ_VISIBLE void ej_show_cpucores(webs_t wp, int argc, char_t **argv)
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

struct CPUFEATURES {
	char *line;
	char *field;
	char *name;
};
static struct CPUFEATURES cpufeatures[] = {
	//x86/x64
	{ "flags", "fpu", "FPU" },
	{ "flags", "mmx", "MMX" },
	{ "flags", "sse", "SSE" },
	{ "flags", "sse2", "SSE2" },
	{ "flags", "ssse3", "SSSE3" },
	{ "flags", "sse3", "SSE3" },
	{ "flags", "sse4", "SSE4" },
	{ "flags", "sse4_1", "SSE4.1" },
	{ "flags", "sse4_2", "SSE4.2" },
	{ "flags", "sse4a", "SSE4a" },
	{ "flags", "avx", "AVX" },
	{ "flags", "avx2", "AVX2" },
	{ "flags", "aes", "AES-NI" },
	{ "flags", "ht", "HT" },
	{ "flags", "nx", "NX" },

	// ARM
	{ "Features", "vfp", "VFP" },
	{ "Features", "vfpv3", "VFPv3" },
	{ "Features", "vfpv3d16", "VFPv3d16" },
	{ "Features", "vfpv4", "VFPv4" },
	{ "Features", "neon", "NEON" },
	{ "Features", "vfpd32", "VFPD32" },
	{ "Features", "edsp", "EDSP" },
	{ "Features", "lpae", "LPAE" },
	{ "Features", "aes", "AES" },
	{ "Features", "sha1", "SHA1" },
	{ "Features", "sha2", "SHA2" },
	{ "Features", "java", "JAVA" },
	{ "Features", "pmull", "PMULL" },
	{ "Features", "asimd", "ASIMD" },
	{ "Features", "crc32", "CRC32" },
	{ "Features", "fp", "FP" },
	// Northstar prototype RT-AC68U also has these
	{ "Features", "fastmult", "FASTMULT" },
	{ "Features", "half", "HALF" },
	{ "Features", "tls", "TLS" },
	{ "Features", "swp", "SWP" },
	{ "Features", "thumbee", "THUMBEE" },
	{ "Features", "thumb", "THUMB" },
	{ "Features", "26bit", "26BIT" },
	{ "Features", "fpa", "FPA" },
	{ "Features", "java", "JAVA" },
	{ "Features", "iwmmx", "IWMMX" },
	{ "Features", "crunch", "CRUNCH" },
	{ "Features", "idiva", "IDIVA" },
	{ "Features", "idivt", "IDIVT" },
	{ "Features", "evtstrm", "EVTSTRM" },
	//mips

	/*	{"isa", "mips1", "MIPS1"},
	{"isa", "mips2", "MIPS2"},
	{"isa", "mips3", "MIPS3"},
	{"isa", "mips4", "MIPS4"},
	{"isa", "mips5", "MIPS5"},*/
	{ "isa", "mips32r1", "MIPS32r1" },
	{ "isa", "mips32r2", "MIPS32r2" },
	{ "isa", "mips32r6", "MIPS32r6" },
	{ "isa", "mips64r1", "MIPS64r1" },
	{ "isa", "mips64r2", "MIPS64r2" },
	{ "isa", "mips64r6", "MIPS64r6" },
	{ "ASEs implemented", "mips16", "MIPS16" },
	{ "ASEs implemented", "dsp", "DSP" },
	{ "ASEs implemented", "dsp2", "DSP2" },
	{ "ASEs implemented", "dsp3", "DSP3" },
	{ "ASEs implemented", "mt", "MT" },
};

EJ_VISIBLE void ej_show_cpufeatures(webs_t wp, int argc, char_t **argv)
{
	char word[64];
	char *next = NULL;
	char *result = NULL;
	FILE *fp = fopen("/proc/cpuinfo", "rb");
	if (!fp)
		return;
	char *line = malloc(1024);
	while (fgets(line, 1024, fp)) {
		int i;
		for (i = 0; i < sizeof(cpufeatures) / sizeof(struct CPUFEATURES); i++) {
			if (strstr(line, cpufeatures[i].line)) {
				char *begin = strchr(line, ':') + 1;
				foreach(word, begin, next)
				{
					if (result) {
						if (strstr(result, cpufeatures[i].name))
							continue;
					}
					char *p = strchr(word, '\n');
					if (p)
						p[0] = 0;
					int namelen = strlen(cpufeatures[i].name);
					if (!strcmp(word, cpufeatures[i].field)) {
						int resultlen = 0;
						if (!result) {
							result = malloc(namelen + 1);
							bzero(result, namelen);
						} else {
							resultlen = strlen(result);
							result = realloc(result, resultlen + namelen + 2);
						}
						strspcattach(result, cpufeatures[i].name);
					}
				}
			}
		}
	}
	debug_free(line);
	fclose(fp);
	if (result && *(result)) {
		char buf[128];
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\">%s</div>\n", tran_string(buf, sizeof(buf), "status_router.features"));
		websWrite(wp, "<div class=\"padding-left\">%s&nbsp;\n</div>", result);
		websWrite(wp, "</div>\n");
	}
	if (result)
		debug_free(result);
}
