
/*
 * sysinit.c
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

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>
#include <termios.h>

#include <sys/klog.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <dirent.h>

#include <epivers.h>
#include <bcmnvram.h>
#include <mtd.h>
#include <shutils.h>
#include <rc.h>
#include <netconf.h>
#include <nvparse.h>
#include <bcmdevs.h>

#include <wlutils.h>
#include <utils.h>
#include <cyutils.h>
#include <code_pattern.h>
#include <cy_conf.h>
// #include <mkfiles.h>
#include <typedefs.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <wlutils.h>
#include <cy_conf.h>
#include <utils.h>

#include <glob.h>
#include <revision.h>

#define WL_IOCTL(name, cmd, buf, len) (ret = wl_ioctl((name), (cmd), (buf), (len)))

#define TXPWR_MAX 1000
#define TXPWR_DEFAULT 28

void start_restore_defaults(void);
static void rc_signal(int sig);
extern void start_overclocking(void);
extern int check_cfe_nv(void);
extern int check_pmon_nv(void);
static void unset_nvram(void);
void start_nvram(void);

extern struct nvram_param *load_defaults(void);
extern void free_defaults(struct nvram_param *);

extern int f_exists(const char *path);

#ifdef HAVE_MACBIND
#include "../../../opt/mac.h"
#endif

static void internal_runStartup(char *folder, char *extension)
{
	struct dirent **entry;
	struct stat filestat;
	DIR *directory;
	int num, n = 0;
	char fullname[128];

	directory = opendir(folder);
	if (directory == NULL) {
		return;
	}
	closedir(directory);

	num = scandir(folder, &entry, 0, alphasort);
	if (num < 0)
		return;
	// list all files in this directory
	while (n < num) {
		if (!strcmp(extension, "K**") && strlen(entry[n]->d_name) > 3 && startswith(entry[n]->d_name, "K") &&
		    strspn(entry[n]->d_name, "K1234567890") == 3) { // K* scripts
			sprintf(fullname, "%s/%s", folder, entry[n]->d_name);
			if (!stat(fullname, &filestat) && (filestat.st_mode & S_IXUSR))
				eval_silence(fullname);
			free(entry[n]);
			n++;
			continue;
		}
		if (!strcmp(extension, "S**") && strlen(entry[n]->d_name) > 3 && startswith(entry[n]->d_name, "S") &&
		    strspn(entry[n]->d_name, "S1234567890") == 3) { // S* scripts
			sprintf(fullname, "%s/%s", folder, entry[n]->d_name);
			if (!stat(fullname, &filestat) && (filestat.st_mode & S_IXUSR))
				eval_silence(fullname);
			free(entry[n]);
			n++;
			continue;
		}
		if (endswith(entry[n]->d_name, extension)) {
#ifdef HAVE_REGISTER
			if (!isregistered_real()) {
				if (endswith(entry[n]->d_name, "wdswatchdog.startup")) {
					free(entry[n]);
					n++;
					continue;
				}
				if (endswith(entry[n]->d_name, "schedulerb.startup")) {
					free(entry[n]);
					n++;
					continue;
				}
				if (endswith(entry[n]->d_name, "proxywatchdog.startup")) {
					free(entry[n]);
					n++;
					continue;
				}
			}
#endif

			sprintf(fullname, "%s/%s", folder, entry[n]->d_name);
			eval_silence(fullname);
			// execute script
		}
		free(entry[n]);
		n++;
	}
	free(entry);

	return;
}

void runStartup(char *extension)
{
	internal_runStartup("/etc/config", extension);
#ifdef HAVE_REGISTER
	if (isregistered_real()) {
#endif
		internal_runStartup("/jffs/etc/config", extension);
		internal_runStartup("/mmc/etc/config", extension);
		internal_runStartup("/tmp/etc/config", extension);
		internal_runStartup("/usr/local/etc/config", extension);
		internal_runStartup("/sd/etc/config", extension);
		internal_runStartup("/opt/etc/init.d", extension);
#ifdef HAVE_REGISTER
	}
#endif
}

#if defined(HAVE_BUFFALO) || defined(HAVE_BUFFALO_BL_DEFAULTS)

#ifdef HAVE_BCMMODERN
#define getUEnv(name) nvram_get(name)
static void buffalo_defaults(int force)
{
	char *pincode = getUEnv("pincode");
	if (pincode && !nvram_exists("pincode")) {
		nvram_set("pincode", pincode);
	}
	if (!nvram_exists("wl0_akm") || force) {
		char *region = getUEnv("region");
		if (!region || (strcmp(region, "AP") && strcmp(region, "TW") && strcmp(region, "RU") && strcmp(region, "KR") &&
				strcmp(region, "CH"))) {
			{
				char *mode_ex = getUEnv("DEF-p_wireless_eth1_11a-authmode_ex");
				if (!mode_ex)
					mode_ex = getUEnv("DEF-p_wireless_eth1_11bg-authmode_ex");
				if (mode_ex && !strcmp(mode_ex, "mixed-psk")) {
					char *mode = getUEnv("DEF-p_wireless_eth1_11a-authmode");
					if (!mode)
						mode = getUEnv("DEF-p_wireless_eth1_11bg-authmode");
					if (!mode) {
						nvram_set("wl_akm", "disabled");
						nvram_set("wl0_akm", "disabled");
						nvram_set("wl_security_mode", "disabled");
						nvram_set("wl0_security_mode", "disabled");
					} else {
						if (!strcmp(mode, "psk")) {
							nvram_set("wl0_akm", "psk psk2");
							nvram_set("wl0_security_mode", "psk psk2");
							nvram_set("wl_akm", "psk psk2");
							nvram_set("wl_security_mode", "psk psk2");
						}
						if (!strcmp(mode, "psk2")) {
							nvram_set("wl0_akm", "psk psk2");
							nvram_set("wl0_security_mode", "psk psk2");
							nvram_set("wl_akm", "psk psk2");
							nvram_set("wl_security_mode", "psk psk2");
						}
					}
				} else {
					char *mode = getUEnv("DEF-p_wireless_eth1_11a-authmode");
					if (!mode)
						mode = getUEnv("DEF-p_wireless_eth1_11bg-authmode");
					if (mode) {
						nvram_set("wl0_akm", mode);
						nvram_set("wl0_security_mode", mode);
						nvram_set("wl_akm", mode);
						nvram_set("wl_security_mode", mode);
					} else {
						nvram_set("wl_akm", "disabled");
						nvram_set("wl0_akm", "disabled");
						nvram_set("wl_security_mode", "disabled");
						nvram_set("wl0_security_mode", "disabled");
					}
				}

				char *crypto = getUEnv("DEF-p_wireless_eth1_11a-crypto");
				if (!crypto)
					crypto = getUEnv("DEF-p_wireless_eth1_11bg-crypto");
				if (crypto) {
					nvram_set("wl0_crypto", crypto);
					nvram_set("wl_crypto", crypto);
				}
				char *wpapsk = getUEnv("DEF-p_wireless_eth1_11a-wpapsk");
				if (!wpapsk)
					wpapsk = getUEnv("DEF-p_wireless_eth1_11bg-wpapsk");
				if (wpapsk) {
					nvram_set("wl_wpa_psk", wpapsk);
					nvram_set("wl0_wpa_psk", wpapsk);
				} else {
					nvram_seti("wl_wpa_psk", 12345678);
					nvram_seti("wl0_wpa_psk", 12345678);
				}
			}
			{
				char *mode_ex = getUEnv("DEF-p_wireless_eth2_11bg-authmode_ex");
				if (!mode_ex)
					mode_ex = getUEnv("DEF-p_wireless_eth2_11a-authmode_ex");
				if (mode_ex && !strcmp(mode_ex, "mixed-psk")) {
					char *mode = getUEnv("DEF-p_wireless_eth2_11bg-authmode");
					if (!mode)
						mode = getUEnv("DEF-p_wireless_eth2_11a-authmode");
					if (!mode) {
						nvram_set("wl1_akm", "disabled");
						nvram_set("wl1_security_mode", "disabled");
					} else {
						if (!strcmp(mode, "psk")) {
							nvram_set("wl1_akm", "psk psk2");
							nvram_set("wl1_security_mode", "psk psk2");
						}
						if (!strcmp(mode, "psk2")) {
							nvram_set("wl1_akm", "psk psk2");
							nvram_set("wl1_security_mode", "psk psk2");
						}
					}
				} else {
					char *mode = getUEnv("DEF-p_wireless_eth2_11bg-authmode");
					if (!mode)
						mode = getUEnv("DEF-p_wireless_eth2_11a-authmode");
					if (mode) {
						nvram_set("wl1_akm", mode);
						nvram_set("wl1_security_mode", mode);
					} else {
						nvram_set("wl1_akm", "disabled");
						nvram_set("wl1_security_mode", "disabled");
					}
				}

				char *crypto = getUEnv("DEF-p_wireless_eth2_11bg-crypto");
				if (!crypto)
					crypto = getUEnv("DEF-p_wireless_eth2_11a-crypto");
				if (crypto)
					nvram_set("wl1_crypto", crypto);
				char *wpapsk = getUEnv("DEF-p_wireless_eth2_11bg-wpapsk");
				if (!wpapsk)
					wpapsk = getUEnv("DEF-p_wireless_eth2_11a-wpapsk");
				if (wpapsk)
					nvram_set("wl1_wpa_psk", wpapsk);
				else
					nvram_seti("wl1_wpa_psk", 12345678);
			}
		}
		char eabuf[32];
		unsigned char edata[6];
		get_hwaddr("eth0", edata);
		sprintf(eabuf, "Buffalo-A-%02X%02X", edata[4] & 0xff, edata[5] & 0xff);
		nvram_set("wl_ssid", eabuf);
		nvram_set("wl0_ssid", eabuf);
		sprintf(eabuf, "Buffalo-G-%02X%02X", edata[4] & 0xff, edata[5] & 0xff);
		nvram_set("wl1_ssid", eabuf);

		region = getUEnv("region");
		if (region == NULL) {
			region = "US";
		}
		if (!strcmp(region, "US")) {
			nvram_set("wl0_regdomain", "UNITED_STATES");
		} else if (!strcmp(region, "EU")) {
			nvram_set("wl0_regdomain", "GERMANY");
		} else if (!strcmp(region, "JP")) {
			nvram_set("wl0_regdomain", "JAPAN");
#ifdef HAVE_BUFFALO_SA
		} else if (!strcmp(region, "AP")) {
			nvram_set("wl0_regdomain", "SINGAPORE");
#else
		} else if (!strcmp(region, "AP")) {
			nvram_set("wl0_regdomain", "SINGAPORE");
#endif
		} else if (!strcmp(region, "RU")) {
			nvram_set("wl0_regdomain", "RUSSIA");
		} else if (!strcmp(region, "TW")) {
			nvram_set("wl0_regdomain", "TAIWAN");
		} else if (!strcmp(region, "CH")) {
			nvram_set("wl0_regdomain", "CHINA");
		} else if (!strcmp(region, "KR")) {
			nvram_set("wl0_regdomain", "KOREA_REPUBLIC");
		}
#ifdef HAVE_WZRHPAG300NH
		if (!strcmp(region, "US")) {
			nvram_set("wl1_regdomain", "UNITED_STATES");
		} else if (!strcmp(region, "EU")) {
			nvram_set("wl1_regdomain", "GERMANY");
		} else if (!strcmp(region, "JP")) {
			nvram_set("wl1_regdomain", "JAPAN");
		} else if (!strcmp(region, "RU")) {
			nvram_set("wl1_regdomain", "RUSSIA");
#ifdef HAVE_BUFFALO_SA
		} else if (!strcmp(region, "AP")) {
			nvram_set("wl1_regdomain", "SINGAPORE");
#else
		} else if (!strcmp(region, "AP")) {
			nvram_set("wl1_regdomain", "SINGAPORE");
#endif
		} else if (!strcmp(region, "TW")) {
			nvram_set("wl1_regdomain", "TAIWAN");
		} else if (!strcmp(region, "CH")) {
			nvram_set("wl1_regdomain", "CHINA");
		} else if (!strcmp(region, "KR")) {
			nvram_set("wl1_regdomain", "KOREA_REPUBLIC");
		}
#ifdef HAVE_HOBBIT
		nvram_set("wl_regdomain", "EUROPE");
#endif

#endif
		if (!strcmp(region, "AP") || !strcmp(region, "CH") || !strcmp(region, "KR") || !strcmp(region, "TW") ||
		    !strcmp(region, "RU"))
			nvram_seti("wps_status", 0);
		else
			nvram_seti("wps_status", 1);
		nvram_set("wl_country_code", region);
#ifdef HAVE_BCMMODERN

		unsigned long boardnum = strtoul(nvram_safe_get("boardnum"), NULL, 0);

		nvram_set("wl0_country_code", "Q1");
		nvram_seti("wl0_country_rev", 27);
		nvram_set("wl1_country_code", "Q1");
		nvram_seti("wl1_country_rev", 27);
		if (!strcmp(region, "JP")) {
			nvram_set("wl0_country_code", "JP");
			nvram_seti("wl0_country_rev", 45);
			nvram_set("wl1_country_code", "JP");
			nvram_seti("wl1_country_rev", 45);
		}

		if (!strcmp(region, "US")) {
			if (boardnum == 00 && nvram_match("boardtype", "0x0665") && nvram_match("boardrev", "0x1103") &&
			    nvram_match("melco_id", "RD_BB13049")) {
				// WXR-1900DHP
				nvram_set("wl1_country_code", "Q2");
				nvram_seti("wl1_country_rev", 41);
				nvram_set("wl0_country_code", "Q1");
				nvram_seti("wl0_country_rev", 61);
			} else {
				nvram_set("wl0_country_code", "Q2");
				nvram_seti("wl0_country_rev", 41);
				nvram_set("wl1_country_code", "Q1");
				nvram_seti("wl1_country_rev", 61);
			}
		}

		if (!strcmp(region, "EU")) {
			nvram_set("wl0_country_code", "EU");
			nvram_seti("wl0_country_rev", 61);
			nvram_set("wl1_country_code", "EU");
			nvram_seti("wl1_country_rev", 61);
		}

		if (!strcmp(region, "AP")) {
			if (boardnum == 00 && nvram_match("boardtype", "0x0665") && nvram_match("boardrev", "0x1103") &&
			    nvram_match("melco_id", "RD_BB13049")) {
				// WXR-1900DHP
				nvram_set("wl1_country_code", "CN");
				nvram_seti("wl1_country_rev", 34);
				nvram_set("wl0_country_code", "Q2");
				nvram_seti("wl0_country_rev", 41);
			} else {
				nvram_set("wl0_country_code", "CN");
				nvram_seti("wl0_country_rev", 34);
				nvram_set("wl1_country_code", "Q2");
				nvram_seti("wl1_country_rev", 41);
			}
		}

		if (!strcmp(region, "KR")) {
			if (boardnum == 00 && nvram_match("boardtype", "0x0665") && nvram_match("boardrev", "0x1103") &&
			    nvram_match("melco_id", "RD_BB13049")) {
				// WXR-1900DHP
				nvram_set("wl1_country_code", "KR");
				nvram_seti("wl1_country_rev", 55);
				nvram_set("wl0_country_code", "Q2");
				nvram_seti("wl0_country_rev", 41);
			} else {
				nvram_set("wl0_country_code", "KR");
				nvram_seti("wl0_country_rev", 55);
				nvram_set("wl1_country_code", "Q2");
				nvram_seti("wl1_country_rev", 41);
			}
		}

		if (!strcmp(region, "CH")) {
			if (boardnum == 00 && nvram_match("boardtype", "0x0665") && nvram_match("boardrev", "0x1103") &&
			    nvram_match("melco_id", "RD_BB13049")) {
				// WXR-1900DHP
				nvram_set("wl1_country_code", "CH");
				nvram_seti("wl1_country_rev", 34);
				nvram_set("wl0_country_code", "Q2");
				nvram_seti("wl0_country_rev", 41);
			} else {
				nvram_set("wl0_country_code", "CH");
				nvram_seti("wl0_country_rev", 34);
				nvram_set("wl1_country_code", "Q2");
				nvram_seti("wl1_country_rev", 41);
			}
		}

		if (!strcmp(region, "TW")) {
			if (boardnum == 00 && nvram_match("boardtype", "0x0665") && nvram_match("boardrev", "0x1103") &&
			    nvram_match("melco_id", "RD_BB13049")) {
				// WXR-1900DHP
				nvram_set("wl1_country_code", "TW");
				nvram_seti("wl1_country_rev", 34);
				nvram_set("wl0_country_code", "Q2");
				nvram_seti("wl0_country_rev", 41);
			} else {
				nvram_set("wl0_country_code", "TW");
				nvram_seti("wl0_country_rev", 34);
				nvram_set("wl1_country_code", "Q2");
				nvram_seti("wl1_country_rev", 41);
			}
		}

		if (!strcmp(region, "RU")) {
			if (boardnum == 00 && nvram_match("boardtype", "0x0665") && nvram_match("boardrev", "0x1103") &&
			    nvram_match("melco_id", "RD_BB13049")) {
				// WXR-1900DHP
				nvram_set("wl1_country_code", "RU");
				nvram_seti("wl1_country_rev", 37);
				nvram_set("wl0_country_code", "Q2");
				nvram_seti("wl0_country_rev", 41);
			} else {
				nvram_set("wl0_country_code", "RU");
				nvram_seti("wl0_country_rev", 37);
				nvram_set("wl1_country_code", "Q2");
				nvram_seti("wl1_country_rev", 41);
			}
		}
#else
		nvram_set("wl0_country_code", region);
		nvram_set("wl1_country_code", region);
#endif
		nvram_seti("ias_startup", 3);
		nvram_unset("http_userpln");
		nvram_unset("http_pwdpln");
#ifdef HAVE_SPOTPASS
		eval("startservice", "spotpass_defaults", "-f");
#endif
	}
}

#elif HAVE_MT7620
#define getUEnv(name) nvram_get(name)
static void buffalo_defaults(int force)
{
	if (!nvram_exists("wlan0_akm") || force) {
		nvram_set("wlan0_akm", "disabled");
		nvram_set("wlan1_akm", "disabled");

		FILE *fp;
		char script[32] = "/tmp/fdefaults.sh";
		char config[32] = "/tmp/sysdefaults.txt";
		char partition[20] = "/dev/mtdblock6";
		char mountpoint[20] = "/tmp/sysdefaults";
		char conffile[16] = "mac.dat";

		insmod("lzma_compress");
		insmod("lzma_decompress");
		insmod("jffs2");
		eval("mkdir", mountpoint);
		eval("mount", "-t", "jffs2", partition, mountpoint);
		sysprintf("cat %s/%s > %s", mountpoint, conffile, config);

		fp = fopen(config, "r");
		if (fp) {
			char line[32];
			char list[2][30];
			int i;

			while (fgets(line, sizeof(line), fp) != NULL) {
				// make string sscanf ready
				for (i = 0; i < sizeof(line); i++) {
					if (line[i] == '=') {
						line[i] = ' ';
						break;
					}
				}

				if (sscanf(line, "%s %s[\n]", list[0], list[1]) != 2)
					continue;

				if (!strcmp("Region", list[0]))
					nvram_set("region", list[1]);

				if (!strcmp("WIFIFacWPAPSK1", list[0])) {
					nvram_set("wl0_security_mode", "psk psk2");
					nvram_set("wl0_akm", "psk psk2");
					nvram_set("wl0_crypto", "aes");
					nvram_set("wl0_wpa_psk", list[1]);

					nvram_set("wl1_security_mode", "psk psk2");
					nvram_set("wl1_akm", "psk psk2");
					nvram_set("wl1_crypto", "aes");
					nvram_set("wl1_wpa_psk", list[1]);
				}
			}
			fclose(fp);
		}
		// cleanup
		unlink(script);
		unlink(config);
		eval("umount", partition);
		rmdir(mountpoint);

		char eabuf[32];
		unsigned char edata[6];
		get_hwaddr("eth0", edata);
		sprintf(eabuf, "Buffalo-G-%02X%02X", edata[4] & 0xff, edata[5] & 0xff);
		nvram_set("wl0_ssid", eabuf);
		sprintf(eabuf, "Buffalo-A-%02X%02X", edata[4] & 0xff, edata[5] & 0xff);
		nvram_set("wl1_ssid", eabuf);

		nvram_seti("ias_startup", 3);
		nvram_unset("http_userpln");
		nvram_unset("http_pwdpln");
#ifdef HAVE_SPOTPASS
		eval("startservice", "spotpass_defaults", "-f");
#endif
		nvram_async_commit();
	}
}

#else
extern void *getUEnv(char *name);
static void buffalo_defaults(int force)
{
	char *pincode = getUEnv("pincode");
	if (pincode && !nvram_exists("pincode")) {
		nvram_set("pincode", pincode);
	}
	if (!nvram_exists("wlan0_akm") || force) {
		nvram_set("wlan0_akm", "disabled");
		char *region = getUEnv("region");
		if (!region || (strcmp(region, "AP") && strcmp(region, "TW") && strcmp(region, "RU") && strcmp(region, "KR") &&
				strcmp(region, "CH"))) {
			{
				char *mode_ex = getUEnv("DEF-p_wireless_ath0_11bg-authmode_ex");
				if (!mode_ex)
					mode_ex = getUEnv("DEF-p_wireless_ath00_11bg-authmode_ex");
				if (mode_ex && !strcmp(mode_ex, "mixed-psk")) {
					char *mode = getUEnv("DEF-p_wireless_ath0_11bg-authmode");
					if (!mode)
						mode = getUEnv("DEF-p_wireless_ath00_11bg-authmode");
					if (!mode) {
						nvram_set("wlan0_akm", "disabled");
						nvram_set("wlan0_security_mode", "disabled");
					} else {
						if (!strcmp(mode, "psk")) {
							nvram_set("wlan0_akm", "psk psk2");
							nvram_set("wlan0_psk", "1");
							nvram_set("wlan0_psk2", "1");
							nvram_set("wlan0_security_mode", "psk");
						}
						if (!strcmp(mode, "psk2")) {
							nvram_set("wlan0_akm", "psk psk2");
							nvram_set("wlan0_psk", "1");
							nvram_set("wlan0_psk2", "1");
							nvram_set("wlan0_security_mode", "psk");
						}
					}
				} else if (mode_ex && !strcmp(mode_ex, "wpa2-psk")) {
					char *mode = getUEnv("DEF-p_wireless_ath0_11bg-authmode");
					if (!mode)
						mode = getUEnv("DEF-p_wireless_ath00_11bg-authmode");
					if (!mode) {
						nvram_set("wlan0_akm", "disabled");
						nvram_set("wlan0_security_mode", "disabled");
					} else {
						if (!strcmp(mode, "psk")) {
							nvram_set("wlan0_akm", "psk2");
							nvram_set("wlan0_psk2", "1");
							nvram_set("wlan0_security_mode", "psk");
						}
						if (!strcmp(mode, "psk2")) {
							nvram_set("wlan0_akm", "psk2");
							nvram_set("wlan0_psk2", "1");
							nvram_set("wlan0_security_mode", "psk");
						}
					}
				} else {
					char *mode = getUEnv("DEF-p_wireless_ath0_11bg-authmode");
					if (!mode)
						mode = getUEnv("DEF-p_wireless_ath00_11bg-authmode");
					if (mode) {
						nvram_set("wlan0_akm", mode);
						nvram_set("wlan0_security_mode", mode);
					} else {
						nvram_set("wlan0_akm", "disabled");
						nvram_set("wlan0_security_mode", "disabled");
					}
				}

				char *crypto = getUEnv("DEF-p_wireless_ath0_11bg-crypto");
				if (!crypto)
					crypto = getUEnv("DEF-p_wireless_ath00_11bg-crypto");
				if (crypto)
					nvram_set("wlan0_crypto", crypto);
				char *wpapsk = getUEnv("DEF-p_wireless_ath0_11bg-wpapsk");
				if (!wpapsk)
					wpapsk = getUEnv("DEF-p_wireless_ath00_11bg-wpapsk");
				if (wpapsk)
					nvram_set("wlan0_wpa_psk", wpapsk);
			}
#ifdef HAVE_WZRHPAG300NH
			{
				char *mode_ex = getUEnv("DEF-p_wireless_ath1_11a-authmode_ex");
				if (!mode_ex)
					mode_ex = getUEnv("DEF-p_wireless_ath10_11a-authmode_ex");
				if (mode_ex && !strcmp(mode_ex, "mixed-psk")) {
					char *mode = getUEnv("DEF-p_wireless_ath1_11a-authmode");
					if (!mode)
						mode = getUEnv("DEF-p_wireless_ath10_11a-authmode");
					if (!mode) {
						nvram_set("wlan1_akm", "disabled");
						nvram_set("wlan1_security_mode", "disabled");
					} else {
						if (!strcmp(mode, "psk")) {
							nvram_set("wlan1_akm", "psk psk2");
							nvram_set("wlan1_psk", "1");
							nvram_set("wlan1_psk2", "1");
							nvram_set("wlan1_security_mode", "psk");
						}
						if (!strcmp(mode, "psk2")) {
							nvram_set("wlan1_akm", "psk psk2");
							nvram_set("wlan1_psk", "1");
							nvram_set("wlan1_psk2", "1");
							nvram_set("wlan1_security_mode", "psk");
						}
					}
				} else {
					char *mode = getUEnv("DEF-p_wireless_ath1_11a-authmode");
					if (!mode)
						mode = getUEnv("DEF-p_wireless_ath10_11a-authmode");
					if (mode) {
						nvram_set("wlan1_akm", mode);
						nvram_set("wlan1_security_mode", mode);
					} else {
						nvram_set("wlan1_akm", "disabled");
						nvram_set("wlan1_security_mode", "disabled");
					}
				}

				char *crypto = getUEnv("DEF-p_wireless_ath1_11a-crypto");
				if (!crypto)
					crypto = getUEnv("DEF-p_wireless_ath10_11a-crypto");
				if (crypto)
					nvram_set("wlan1_crypto", crypto);
				char *wpapsk = getUEnv("DEF-p_wireless_ath1_11a-wpapsk");
				if (!wpapsk)
					wpapsk = getUEnv("DEF-p_wireless_ath10_11a-wpapsk");
				if (wpapsk)
					nvram_set("wlan1_wpa_psk", wpapsk);
			}
		}
		char eabuf[32];
		unsigned char edata[6];
		get_hwaddr("eth0", edata);
		sprintf(eabuf, "Buffalo-G-%02X%02X", edata[4] & 0xff, edata[5] & 0xff);
		nvram_set("wlan0_ssid", eabuf);
		sprintf(eabuf, "Buffalo-A-%02X%02X", edata[4] & 0xff, edata[5] & 0xff);
		nvram_set("wlan1_ssid", eabuf);

#else
		}
		char eabuf[32];
		unsigned char edata[6];
		get_hwaddr("eth0", edata);
#if defined(HAVE_WZR300HP) || defined(HAVE_WHR300HP)
		sprintf(eabuf, "BUFFALO-%02X%02X%02X", edata[3] & 0xff, edata[4] & 0xff, edata[5] & 0xff);
#elif defined(HAVE_WZR450HP2)
		sprintf(eabuf, "BUFFALO-G-%02X%02X", edata[4] & 0xff, edata[5] & 0xff);
#elif defined(HAVE_AXTEL)
		sprintf(eabuf, "AXTELEXTREMO-%02X%02X", edata[4] & 0xff, edata[5] & 0xff);
#else
		sprintf(eabuf, "%02X%02X%02X%02X%02X%02X", edata[0] & 0xff, edata[1] & 0xff, edata[2] & 0xff, edata[3] & 0xff,
			edata[4] & 0xff, edata[5] & 0xff);
#endif
		nvram_set("wlan0_ssid", eabuf);
#endif

		region = getUEnv("region");
		if (region == NULL) {
			region = "US";
		}
		if (!strcmp(region, "US")) {
			nvram_set("wlan0_regdomain", "UNITED_STATES");
		} else if (!strcmp(region, "EU")) {
			nvram_set("wlan0_regdomain", "GERMANY");
		} else if (!strcmp(region, "JP")) {
			nvram_set("wlan0_regdomain", "JAPAN");
#ifdef HAVE_BUFFALO_SA
		} else if (!strcmp(region, "AP")) {
			nvram_set("wlan0_regdomain", "SINGAPORE");
#else
		} else if (!strcmp(region, "AP")) {
			nvram_set("wlan0_regdomain", "SINGAPORE");
#endif
		} else if (!strcmp(region, "RU")) {
			nvram_set("wlan0_regdomain", "RUSSIA");
		} else if (!strcmp(region, "TW")) {
			nvram_set("wlan0_regdomain", "TAIWAN");
		} else if (!strcmp(region, "CH")) {
			nvram_set("wlan0_regdomain", "CHINA");
		} else if (!strcmp(region, "KR")) {
			nvram_set("wlan0_regdomain", "KOREA_REPUBLIC");
		}
#ifdef HAVE_WZRHPAG300NH
		if (!strcmp(region, "US")) {
			nvram_set("wlan1_regdomain", "UNITED_STATES");
		} else if (!strcmp(region, "EU")) {
			nvram_set("wlan1_regdomain", "GERMANY");
		} else if (!strcmp(region, "JP")) {
			nvram_set("wlan1_regdomain", "JAPAN");
		} else if (!strcmp(region, "RU")) {
			nvram_set("wlan1_regdomain", "RUSSIA");
#ifdef HAVE_BUFFALO_SA
		} else if (!strcmp(region, "AP")) {
			nvram_set("wlan1_regdomain", "SINGAPORE");
#else
		} else if (!strcmp(region, "AP")) {
			nvram_set("wlan1_regdomain", "SINGAPORE");
#endif
		} else if (!strcmp(region, "TW")) {
			nvram_set("wlan1_regdomain", "TAIWAN");
		} else if (!strcmp(region, "CH")) {
			nvram_set("wlan1_regdomain", "CHINA");
		} else if (!strcmp(region, "KR")) {
			nvram_set("wlan1_regdomain", "KOREA_REPUBLIC");
		}
#endif
		if (!strcmp(region, "AP") || !strcmp(region, "CH") || !strcmp(region, "KR") || !strcmp(region, "TW") ||
		    !strcmp(region, "RU"))
			nvram_seti("wps_status", 0);
		else
			nvram_seti("wps_status", 1);
		nvram_seti("ias_startup", 3);
		nvram_unset("http_userpln");
		nvram_unset("http_pwdpln");
#ifdef HAVE_SPOTPASS
		eval("startservice", "spotpass_defaults", "-f");
#endif
		nvram_async_commit();
	}
}
#endif

#endif
/*
 * SeG dd-wrt addition for module startup scripts 
 */
void stop_modules(void)
{
}

void start_modules(void)
{
	runStartup(".startup"); // if available
	return;
}

void start_wanup(void)
{
	runStartup(".wanup");
	return;
}

void stop_run_rc_usb(void)
{
}

void start_run_rc_usb(void)
{
	create_rc_file(RC_USB);

	if (f_exists("/tmp/.rc_usb"))
		eval("/tmp/.rc_usb", nvram_safe_get("usb_reason"), nvram_safe_get("usb_dev"));
	nvram_unset("usb_reason");
	nvram_unset("usb_dev");
}

void stop_run_rc_startup(void)
{
}

void run_opt(void)
{
	eval("/opt/etc/init.d/rcS");
	internal_runStartup("/opt/etc/init.d", "S**");
}

void start_run_rc_startup(void)
{
#ifndef HAVE_MICRO
	create_rc_file(RC_STARTUP);
	if (f_exists("/tmp/.rc_startup"))
		eval("/tmp/.rc_startup");
	internal_runStartup("/opt/etc/init.d", "S**");
#endif
}

void stop_run_rc_shutdown(void)
{
}

void start_run_rc_shutdown(void)
{
	/* 
	 * Blink led before reboot 
	 */
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	diag_led(DIAG, START_LED);
#endif
	led_control(LED_DIAG, LED_ON);
	runStartup("K**"); // if available; run K** shutdown scripts
	create_rc_file(RC_SHUTDOWN);
	if (f_exists("/tmp/.rc_shutdown"))
		eval("/tmp/.rc_shutdown");
	return;
}

int create_rc_file(char *name)
{
	FILE *fp;
	char *p = nvram_safe_get(name);
	char tmp_file[100] = { 0 };

	if ((void *)0 == name || 0 == p[0])
		return -1;

	snprintf(tmp_file, 100, "/tmp/.%s", name);
	unlink(tmp_file);

	fp = fopen(tmp_file, "w");
	if (fp) {
		if (strlen(p) <= 2 || memcmp("#!", p, 2))
			fprintf(fp, "#!/bin/sh\n");
		// filter Windows <cr>ud
		while (*p) {
			if (*p != 0x0d)
				fprintf(fp, "%c", *p);
			p++;
		}
		fclose(fp);
	}
	chmod(tmp_file, 0700);

	return 0;
}

static void ses_cleanup(void)
{
	/*
	 * well known event to cleanly initialize state machine 
	 */
	nvram_seti("ses_event", 2);

	/*
	 * Delete lethal dynamically generated variables 
	 */
	nvram_unset("ses_bridge_disable");
}

static void ses_restore_defaults(void)
{
	char tmp[100], prefix[] = "wlXXXXXXXXXX_ses_";
	int i;

	/*
	 * Delete dynamically generated variables 
	 */
	for (i = 0; i < MAX_NVPARSE; i++) {
		sprintf(prefix, "wl%d_ses_", i);
		nvram_unset(strcat_r(prefix, "ssid", tmp));
		nvram_unset(strcat_r(prefix, "closed", tmp));
		nvram_unset(strcat_r(prefix, "wpa_psk", tmp));
		nvram_unset(strcat_r(prefix, "auth", tmp));
		nvram_unset(strcat_r(prefix, "wep", tmp));
		nvram_unset(strcat_r(prefix, "auth_mode", tmp));
		nvram_unset(strcat_r(prefix, "crypto", tmp));
		nvram_unset(strcat_r(prefix, "akm", tmp));
	}
}

void start_restore_defaults(void)
{
#ifdef HAVE_BUFFALO_SA
	if (nvram_invmatchi("sv_restore_defaults", 0) && (!strcmp(getUEnv("region"), "AP") || !strcmp(getUEnv("region"), "US")))
		nvram_set("region", "SA");
#endif
#ifdef HAVE_RB500
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames",
					   "eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 wlan0 wlan1 wlan2 wlan3 wlan4 wlan5" },
					 { "wan_ifname", "eth0" },
					 { "wan_ifnames", "eth0" },
					 { 0, 0 } };
#elif HAVE_E200
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7" },
					 { "wan_ifname", "eth0" },
					 { "wan_ifnames", "eth0" },
					 { 0, 0 } };
#elif HAVE_EROUTER
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 eth2" },
					 { "wan_ifname", "eth0" },
					 { "wan_ifnames", "eth0" },
					 { 0, 0 } };
#elif HAVE_GEMTEK
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth1 wlan0" },
					 { "wan_ifname", "eth0" },
					 { "wan_ifnames", "eth0" },
					 { 0, 0 } };
#elif HAVE_EAP9550
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth2 ra0" },
					 { "wan_ifname2", "eth2" },
					 { "wan_ifname", "eth2" },
					 { "wan_default", "eth2" },
					 { "wan_ifnames", "eth2" },
					 { 0, 0 } };
#elif HAVE_HAMEA15
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "vlan1 ra0" },
					 { "wan_ifname2", "vlan1" },
					 { "wan_ifname", "vlan1" },
					 { "wan_default", "vlan1" },
					 { "wan_ifnames", "vlan1" },
					 { 0, 0 } };
#elif HAVE_RT2880
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "vlan1 vlan2 ra0 ba0" },
					 { "wan_ifname2", "vlan2" },
					 { "wan_ifname", "vlan2" },
					 { "wan_default", "vlan2" },
					 { "wan_ifnames", "vlan2" },
					 { 0, 0 } };
#elif HAVE_GATEWORX
#if defined(HAVE_XIOCOM) || defined(HAVE_MI424WR)
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "ixp1 wlan0 wlan1 wlan2 wlan3" },
					 { "wan_ifname2", "ixp0" },
					 { "wan_ifname", "ixp0" },
					 { "wan_default", "ixp0" },
					 { "wan_ifnames", "ixp0" },
					 { 0, 0 } };
#else
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "ixp0 wlan0 wlan1 wlan2 wlan3" },
					 { "wan_ifname2", "ixp1" },
					 { "wan_ifname", "ixp1" },
					 { "wan_default", "ixp1" },
					 { "wan_ifnames", "ixp1" },
					 { 0, 0 } };
#endif
#elif HAVE_X86
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
#ifdef HAVE_NOWIFI
					 { "lan_ifnames", "eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 eth9 eth10" },
#else
#ifdef HAVE_GW700
					 { "lan_ifnames",
					   "eth0 eth2 eth3 eth4 eth5 eth6 eth7 eth8 eth9 eth10 wlan0 wlan1 wlan2 wlan3 wlan5 wlan6 wlan7 wlan8" },
#else
					 { "lan_ifnames",
					   "eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 eth9 eth10 wlan0 wlan1 wlan2 wlan3 wlan5 wlan6 wlan7 wlan8" },
#endif
#endif
#ifdef HAVE_GW700
					 { "wan_ifname", "eth1" },
					 { "wan_ifname2", "eth1" },
					 { "wan_ifnames", "eth1" },
#else
					 { "wan_ifname", "eth0" },
					 { "wan_ifname2", "eth0" },
					 { "wan_ifnames", "eth0" },
#endif
					 { 0, 0 } };
#elif HAVE_XSCALE
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "ixp0.1 ixp0.2 wlan0 wlan1" },
					 { "wan_ifname", "ixp1" },
					 { "wan_ifname2", "ixp1" },
					 { "wan_ifnames", "ixp1" },
					 { "wan_default", "ixp1" },
					 { 0, 0 } };
#elif HAVE_LAGUNA
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0 wlan1 wlan2 wlan3" },
					 { "wan_ifname", "eth0" },
					 { "wan_ifname2", "eth0" },
					 { "wan_ifnames", "eth0" },
					 { "wan_default", "eth0" },
					 { 0, 0 } };
#elif HAVE_NEWPORT
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 eth3 eth4 eth5 wlan0 wlan1 wlan2 wlan3" },
					 { "wan_ifname", "eth0" },
					 { "wan_ifname2", "eth0" },
					 { "wan_ifnames", "eth0" },
					 { "wan_default", "eth0" },
					 { 0, 0 } };
#elif HAVE_VENTANA
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0 wlan1 wlan2 wlan3" },
					 { "wan_ifname", "eth0" },
					 { "wan_ifname2", "eth0" },
					 { "wan_ifnames", "eth0" },
					 { "wan_default", "eth0" },
					 { 0, 0 } };
#elif HAVE_NORTHSTAR
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "vlan1 vlan2 eth1 eth2" },
					 { "wan_ifname", "vlan2" },
					 { "wan_ifname2", "vlan2" },
					 { "wan_ifnames", "vlan2" },
					 { "wan_default", "vlan2" },
					 { 0, 0 } };
#elif HAVE_MAGICBOX
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth1 wlan0" },
					 { "wan_ifname", "eth0" },
					 { "wan_ifname2", "eth0" },
					 { "wan_ifnames", "eth0" },
					 { "wan_default", "eth0" },
					 { 0, 0 } };
#elif HAVE_UNIWIP
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 wlan0 wlan1" },
					 { "wan_ifname", "eth0" },
					 { "wan_ifname2", "eth0" },
					 { "wan_ifnames", "eth0" },
					 { "wan_default", "eth0" },
					 { 0, 0 } };
#elif HAVE_MVEBU
	struct nvram_param *generic = NULL;

	struct nvram_param wrt1900[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0 wlan1" },
					 { "wan_ifname", "eth1" },
					 { "wan_ifname2", "eth1" },
					 { "wan_ifnames", "eth1" },
					 { "wan_default", "eth1" },
					 { 0, 0 } };

	struct nvram_param wrt1200[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth1 eth0 wlan0 wlan1" },
					 { "wan_ifname", "eth0" },
					 { "wan_ifname2", "eth0" },
					 { "wan_ifnames", "eth0" },
					 { "wan_default", "eth0" },
					 { 0, 0 } };
	int wrt_brand = getRouterBrand();
	if (wrt_brand == ROUTER_WRT_1900AC)
		generic = wrt1900;
	else
		generic = wrt1200;
#elif HAVE_R9000
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 vlan1 vlan2 wlan0 wlan1" },
					 { "wan_ifname", "vlan2" },
					 { "wan_ifname2", "vlan2" },
					 { "wan_ifnames", "vlan2" },
					 { "wan_default", "vlan2" },
					 { 0, 0 } };
#elif HAVE_IPQ806X
	struct nvram_param ipq806x[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0 wlan1" },
					 { "wan_ifname", "eth0" },
					 { "wan_ifname2", "eth0" },
					 { "wan_ifnames", "eth0" },
					 { "wan_default", "eth0" },
					 { 0, 0 } };
	struct nvram_param habanero[] = { { "lan_ifname", "br0" },
					  { "lan_ifnames", "eth0 eth1 wlan0 wlan1" },
					  { "wan_ifname", "eth1" },
					  { "wan_ifname2", "eth1" },
					  { "wan_ifnames", "eth1" },
					  { "wan_default", "eth1" },
					  { 0, 0 } };

	struct nvram_param *generic = NULL;

	int wrt_brand = getRouterBrand();
	if (wrt_brand == ROUTER_HABANERO)
		generic = habanero;
	else
		generic = ipq806x;

#elif HAVE_WDR4900
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "vlan1 vlan2 wlan0 wlan1" },
					 { "wan_ifname", "vlan2" },
					 { "wan_ifname2", "vlan2" },
					 { "wan_ifnames", "vlan2" },
					 { "wan_default", "vlan2" },
					 { 0, 0 } };
#elif HAVE_RB600
	struct nvram_param generic[] = {
		{ "lan_ifname", "br0" },
		{ "lan_ifnames", "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 wlan0 wlan1 wlan2 wlan3 wlan4 wlan5 wlan6 wlan7" },
		{ "wan_ifname", "eth0" },
		{ "wan_ifname2", "eth0" },
		{ "wan_ifnames", "eth0" },
		{ "wan_default", "eth0" },
		{ 0, 0 }
	};
#elif HAVE_FONERA
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "vlan0 wlan0" },
					 { "wan_ifname", "" },
					 { "wan_ifname2", "" },
					 { "wan_default", "" },
					 { "wan_ifnames", "eth0 vlan1" },
					 { 0, 0 } };
#elif HAVE_BWRG1000
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "vlan0 vlan2 wlan0" },
					 { "wan_ifname", "vlan2" },
					 { "wan_ifname2", "vlan2" },
					 { "wan_ifnames", "vlan2" },
					 { "wan_default", "vlan2" },
					 { 0, 0 } };
#elif HAVE_SOLO51
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "vlan0 vlan2 wlan0" },
					 { "wan_ifname", "vlan0" },
					 { "wan_ifname2", "vlan0" },
					 { "wan_ifnames", "vlan0" },
					 { "wan_default", "vlan0" },
					 { 0, 0 } };
#elif HAVE_BS2
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 wlan0 wlan1" },
					 { "wan_ifname", "eth0" },
					 { "wan_ifname2", "eth0" },
					 { "wan_ifnames", "eth0" },
					 { "wan_default", "eth0" },
					 { 0, 0 } };
#elif HAVE_NS2
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 wlan0 wlan1" },
					 { "wan_ifname", "eth0" },
					 { "wan_ifname2", "eth0" },
					 { "wan_ifnames", "eth0" },
					 { "wan_default", "eth0" },
					 { 0, 0 } };
#elif HAVE_LC2
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 wlan0 wlan1" },
					 { "wan_ifname", "eth0" },
					 { "wan_ifname2", "eth0" },
					 { "wan_ifnames", "eth0" },
					 { "wan_default", "eth0" },
					 { 0, 0 } };
#elif defined(HAVE_PICO2) || defined(HAVE_PICO2HP) || defined(HAVE_MS2) || defined(HAVE_BS2HP)
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 wlan0 wlan1" },
					 { "wan_ifname", "eth0" },
					 { "wan_ifname2", "eth0" },
					 { "wan_ifnames", "eth0" },
					 { "wan_default", "eth0" },
					 { 0, 0 } };
#elif HAVE_LS2
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "vlan0 vlan2 wlan0" },
					 { "wan_ifname", "vlan0" },
					 { "wan_ifname2", "vlan0" },
					 { "wan_ifnames", "vlan0" },
					 { "wan_default", "vlan0" },
					 { 0, 0 } };
#elif HAVE_RS
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0 wlan1 wlan2" },
					 { "wan_ifname", "eth0" },
					 { "wan_ifname2", "eth0" },
					 { "wan_ifnames", "eth0" },
					 { "wan_default", "eth0" },
					 { 0, 0 } };
#elif HAVE_WA901
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 wlan0" },
					 { "wan_ifname", "eth0" },
					 { "wan_ifname2", "eth0" },
					 { "wan_ifnames", "eth0" },
					 { "wan_default", "eth0" },
					 { 0, 0 } };
#elif HAVE_WR941
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "vlan0 vlan1 wlan0" },
					 { "wan_ifname", "vlan1" },
					 { "wan_ifname2", "vlan1" },
					 { "wan_ifnames", "vlan1" },
					 { "wan_default", "vlan1" },
					 { 0, 0 } };
#elif HAVE_WA901v1
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth1 wlan0" },
					 { "wan_ifname", "eth1" },
					 { "wan_ifname2", "eth1" },
					 { "wan_ifnames", "eth1" },
					 { "wan_default", "eth1" },
					 { 0, 0 } };
#elif HAVE_ERC
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0" },
					 { "wan_ifname", "eth0" },
					 { "wan_ifname2", "eth0" },
					 { "wan_ifnames", "eth0" },
					 { "wan_default", "eth0" },
					 { 0, 0 } };
#elif HAVE_CARAMBOLA
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 wlan0" },
					 { "wan_ifname", "eth1" },
					 { "wan_ifname2", "eth1" },
					 { "wan_ifnames", "eth1" },
					 { "wan_default", "eth1" },
					 { 0, 0 } };
#elif HAVE_WR710
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0" },
					 { "wan_ifname", "eth0" },
					 { "wan_ifname2", "eth0" },
					 { "wan_ifnames", "eth0" },
					 { "wan_default", "eth0" },
					 { 0, 0 } };
#elif HAVE_WR703
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth1 wlan0" },
					 { "wan_ifname", "eth1" },
					 { "wan_ifname2", "eth1" },
					 { "wan_ifnames", "eth1" },
					 { "wan_default", "eth1" },
					 { 0, 0 } };
#elif HAVE_WDR2543
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "vlan1 vlan2 wlan0" },
					 { "wan_ifname", "vlan2" },
					 { "wan_ifname2", "vlan2" },
					 { "wan_ifnames", "vlan2" },
					 { "wan_default", "vlan2" },
					 { 0, 0 } };
#elif HAVE_WA7510
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth1 wlan0" },
					 { "wan_ifname", "eth1" },
					 { "wan_ifname2", "eth1" },
					 { "wan_ifnames", "eth1" },
					 { "wan_default", "eth1" },
					 { 0, 0 } };
#elif HAVE_WR741
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0" },
					 { "wan_ifname", "eth0" },
					 { "wan_ifname2", "eth0" },
					 { "wan_ifnames", "eth0" },
					 { "wan_default", "eth0" },
					 { 0, 0 } };
#elif HAVE_WR1043
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "vlan1 vlan2 wlan0" },
					 { "wan_ifname", "vlan2" },
					 { "wan_ifname2", "vlan2" },
					 { "wan_ifnames", "vlan2" },
					 { "wan_default", "vlan2" },
					 { 0, 0 } };
#elif HAVE_AP83
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0" },
					 { "wan_ifname", "eth1" },
					 { "wan_ifname2", "eth1" },
					 { "wan_ifnames", "eth1" },
					 { "wan_default", "eth1" },
					 { 0, 0 } };
#elif HAVE_AP94
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0" },
					 { "wan_ifname", "eth1" },
					 { "wan_ifname2", "eth1" },
					 { "wan_ifnames", "eth1" },
					 { "wan_default", "eth1" },
					 { 0, 0 } };
#elif HAVE_DAP3310
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0" },
					 { "wan_ifname", "eth0" },
					 { "wan_ifname2", "eth0" },
					 { "wan_ifnames", "eth0" },
					 { "wan_default", "eth0" },
					 { 0, 0 } };
#elif HAVE_DAP3410
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "vlan1 vlan2 wlan0" },
					 { "wan_ifname", "vlan2" },
					 { "wan_ifname2", "vlan2" },
					 { "wan_ifnames", "vlan2" },
					 { "wan_default", "vlan2" },
					 { 0, 0 } };
#elif HAVE_UBNTM
	struct nvram_param generic[] = { { "lan_ifname", "br0" },   { "lan_ifnames", "eth0 eth1 wlan0" },
					 { "wan_ifname", "eth0" },  { "wan_ifnames", "eth0" },
					 { "wan_default", "eth0" }, { 0, 0 } };
#elif HAVE_HORNET
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0" },
#ifdef HAVE_MAKSAT
					 { "wan_ifname", "eth0" },
					 { "wan_ifname2", "eth0" },
					 { "wan_ifnames", "eth0" },
					 { "wan_default", "eth0" },
#elif HAVE_ONNET
					 { "wan_ifname", "eth0" },
					 { "wan_ifname2", "eth0" },
					 { "wan_ifnames", "eth0" },
					 { "wan_default", "eth0" },
#else
/*		{"wan_ifname", "eth1"},
		{"wan_ifname2", "eth1"},
		{"wan_ifnames", "eth1"},
		{"wan_default", "eth1"},
*/
#endif
					 { 0, 0 } };
#elif HAVE_WNR2000
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0" },
					 { "wan_ifname", "" },
					 { "wan_ifname2", "" },
					 { "wan_ifnames", "" },
					 { "wan_default", "" },
					 { 0, 0 } };
#elif HAVE_XD9531
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0" },
					 { "wan_ifname", "eth0" },
					 { "wan_ifname2", "eth0" },
					 { "wan_ifnames", "eth0" },
					 { "wan_default", "eth0" },
					 { 0, 0 } };
#elif HAVE_WR615N
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0" },
					 { "wan_ifname", "" },
					 { "wan_ifname2", "" },
					 { "wan_ifnames", "" },
					 { "wan_default", "" },
					 { 0, 0 } };
#elif HAVE_E325N
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0 wlan1" },
					 { "wan_ifname", "" },
					 { "wan_ifname2", "" },
					 { "wan_ifnames", "" },
					 { "wan_default", "" },
					 { 0, 0 } };
#elif HAVE_E355AC
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0 wlan1" },
					 { "wan_ifname", "" },
					 { "wan_ifname2", "" },
					 { "wan_ifnames", "" },
					 { "wan_default", "" },
					 { 0, 0 } };
#elif HAVE_XD3200
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "vlan1 vlan2 wlan0 wlan1" },
					 { "wan_ifname", "" },
					 { "wan_ifname2", "" },
					 { "wan_ifnames", "" },
					 { "wan_default", "" },
					 { 0, 0 } };
#elif HAVE_E380AC
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 wlan0 wlan1" },
					 { "wan_ifname", "" },
					 { "wan_ifname2", "" },
					 { "wan_ifnames", "" },
					 { "wan_default", "" },
					 { 0, 0 } };
#elif HAVE_AP120C
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 wlan0 wlan1" },
					 { "wan_ifname", "" },
					 { "wan_ifname2", "" },
					 { "wan_ifnames", "" },
					 { "wan_default", "" },
					 { 0, 0 } };
#elif HAVE_WR650AC
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0 wlan1" },
					 { "wan_ifname", "" },
					 { "wan_ifname2", "" },
					 { "wan_ifnames", "" },
					 { "wan_default", "" },
					 { 0, 0 } };
#elif HAVE_DIR862
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0 wlan1" },
					 { "wan_ifname", "" },
					 { "wan_ifname2", "" },
					 { "wan_ifnames", "" },
					 { "wan_default", "" },
					 { 0, 0 } };
#elif HAVE_CPE880
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "vlan2 wlan0" },
					 { "wan_ifname", "vlan1" },
					 { "wan_ifname2", "" },
					 { "wan_ifnames", "" },
					 { "wan_default", "" },
					 { 0, 0 } };
#elif HAVE_WILLY
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 wlan0 wlan1" },
					 { "wan_ifname", "" },
					 { "wan_ifname2", "" },
					 { "wan_ifnames", "" },
					 { "wan_default", "" },
					 { 0, 0 } };
#elif HAVE_MMS344
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "vlan1 vlan2 wlan0 wlan1" },
					 { "wan_ifname", "" },
					 { "wan_ifname2", "" },
					 { "wan_ifnames", "" },
					 { "wan_default", "" },
					 { 0, 0 } };
#elif HAVE_ARCHERC7V4
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "vlan1 vlan2 wlan0 wlan1" },
					 { "wan_ifname", "" },
					 { "wan_ifname2", "" },
					 { "wan_ifnames", "" },
					 { "wan_default", "" },
					 { 0, 0 } };
#elif HAVE_WZR450HP2
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0 wlan1" },
					 { "wan_ifname", "" },
					 { "wan_ifname2", "" },
					 { "wan_ifnames", "" },
					 { "wan_default", "" },
					 { 0, 0 } };
#elif HAVE_JWAP606
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 wlan0 wlan1" },
					 { "wan_ifname", "" },
					 { "wan_ifname2", "" },
					 { "wan_ifnames", "" },
					 { "wan_default", "" },
					 { 0, 0 } };
#elif HAVE_LIMA
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0 wlan1" },
					 { "wan_ifname", "" },
					 { "wan_ifname2", "" },
					 { "wan_ifnames", "" },
					 { "wan_default", "" },
					 { 0, 0 } };
#elif HAVE_DW02_412H
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "vlan1 vlan2 wlan0 wlan1" },
					 { "wan_ifname", "" },
					 { "wan_ifname2", "" },
					 { "wan_ifnames", "" },
					 { "wan_default", "" },
					 { 0, 0 } };
#elif HAVE_RAMBUTAN
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0 wlan1" },
					 { "wan_ifname", "" },
					 { "wan_ifname2", "" },
					 { "wan_ifnames", "" },
					 { "wan_default", "" },
					 { 0, 0 } };
#elif HAVE_WASP
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "vlan1 vlan2 wlan0 wlan1" },
					 { "wan_ifname", "" },
					 { "wan_ifname2", "" },
					 { "wan_ifnames", "" },
					 { "wan_default", "" },
					 { 0, 0 } };
#elif HAVE_WHRHPGN
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0" },
					 { "wan_ifname", "" },
					 { "wan_ifname2", "" },
					 { "wan_ifnames", "" },
					 { "wan_default", "" },
					 { 0, 0 } };
#elif HAVE_WA901V5
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth1 wlan0" },
					 { "wan_ifname", "" },
					 { "wan_ifname2", "" },
					 { "wan_ifnames", "" },
					 { "wan_default", "" },
					 { 0, 0 } };
#elif HAVE_WR941V6
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0" },
					 { "wan_ifname", "" },
					 { "wan_ifname2", "" },
					 { "wan_ifnames", "" },
					 { "wan_default", "" },
					 { 0, 0 } };
#elif HAVE_DIR615E
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0" },
					 { "wan_ifname", "" },
					 { "wan_ifname2", "" },
					 { "wan_ifnames", "" },
					 { "wan_default", "" },
					 { 0, 0 } };
#elif HAVE_JA76PF
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0" },
#ifdef HAVE_SANSFIL
					 { "wan_ifname", "eth0" },
					 { "wan_ifname2", "eth0" },
					 { "wan_ifnames", "eth0" },
					 { "wan_default", "eth0" },
#else
					 { "wan_ifname", "eth1" },
					 { "wan_ifname2", "eth1" },
					 { "wan_ifnames", "eth1" },
					 { "wan_default", "eth1" },
#endif
					 { 0, 0 } };
#elif HAVE_ALFAAP94
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0 wlan1 wlan2" },
					 { "wan_ifname", "eth1" },
					 { "wan_ifname2", "eth1" },
					 { "wan_ifnames", "eth1" },
					 { "wan_default", "eth1" },
					 { 0, 0 } };
#elif HAVE_WZRG450
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "vlan1 wlan0" },
					 { "wan_ifname", "vlan2" },
					 { "wan_ifname2", "vlan2" },
					 { "wan_ifnames", "vlan2" },
					 { "wan_default", "vlan2" },
					 { 0, 0 } };
#elif HAVE_WZRHPAG300NH
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0 wlan1" },
					 { "wan_ifname", "eth1" },
					 { "wan_ifname2", "eth1" },
					 { "wan_ifnames", "eth1" },
					 { "wan_default", "eth1" },
					 { 0, 0 } };
#elif HAVE_JWAP003
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0" },
					 { "wan_ifname", "eth1" },
					 { "wan_ifname2", "eth1" },
					 { "wan_ifnames", "eth1" },
					 { "wan_default", "eth1" },
					 { 0, 0 } };
#elif HAVE_JJAP93
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 wlan0" },
					 { "wan_ifname", "" },
					 { "wan_ifname2", "" },
					 { "wan_ifnames", "" },
					 { "wan_default", "" },
					 { 0, 0 } };
#elif defined(HAVE_JJAP005) || defined(HAVE_JJAP501) || defined(HAVE_AC722) || defined(HAVE_AC622)
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0" },
					 { "wan_ifname", "eth1" },
					 { "wan_ifname2", "eth1" },
					 { "wan_ifnames", "eth1" },
					 { "wan_default", "eth1" },
					 { 0, 0 } };
#elif HAVE_WP546
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0 wlan1" },
					 { "wan_ifname", "eth1" },
					 { "wan_ifname2", "eth1" },
					 { "wan_ifnames", "eth1" },
					 { "wan_default", "eth1" },
					 { 0, 0 } };
#elif HAVE_WR810N
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 wlan0" },
					 { "wan_ifname", "" },
					 { "wan_ifname2", "" },
					 { "wan_ifnames", "" },
					 { "wan_default", "" },
					 { 0, 0 } };
#elif HAVE_LSX
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 wlan0" },
					 { "wan_ifname", "" },
					 { "wan_ifname2", "" },
					 { "wan_ifnames", "" },
					 { "wan_default", "" },
					 { 0, 0 } };
#elif HAVE_DANUBE
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 wlan0" },
					 { "wan_ifname", "nas0" },
					 { "wan_ifname2", "nas0" },
					 { "wan_ifnames", "nas0" },
					 { "wan_default", "nas0" },
					 { 0, 0 } };
#elif HAVE_WBD222
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 eth2 wlan0 wlan1 wlan2" },
					 { "wan_ifname", "eth0" },
					 { "wan_ifname2", "eth0" },
					 { "wan_ifnames", "eth0" },
					 { "wan_default", "eth0" },
					 { 0, 0 } };
#elif HAVE_STORM
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 wlan0" },
					 { "wan_ifname", "" },
					 { "wan_ifname2", "" },
					 { "wan_ifnames", "" },
					 { "wan_default", "" },
					 { 0, 0 } };
#elif HAVE_OPENRISC
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth1 eth2 eth3 wlan0" },
					 { "wan_ifname", "" },
					 { "wan_ifname2", "" },
					 { "wan_ifnames", "" },
					 { "wan_default", "" },
					 { 0, 0 } };
#elif HAVE_WP54G
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 wlan0" },
					 { "wan_ifname", "eth1" },
					 { "wan_ifname2", "eth1" },
					 { "wan_ifnames", "eth1" },
					 { "wan_default", "eth1" },
					 { 0, 0 } };
#elif HAVE_NP28G
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 wlan0" },
					 { "wan_ifname", "eth1" },
					 { "wan_ifname2", "eth1" },
					 { "wan_ifnames", "eth1" },
					 { "wan_default", "eth1" },
					 { 0, 0 } };
#elif HAVE_ADM5120
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 wlan0" },
					 { "wan_ifname", "" },
					 { "wan_ifname2", "" },
					 { "wan_ifnames", "" },
					 { "wan_default", "" },
					 { 0, 0 } };
#elif HAVE_LS5
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "wlan0" },
					 { "wan_ifname", "eth0" },
					 { "wan_ifname2", "eth0" },
					 { "wan_ifnames", "eth0" },
					 { "wan_default", "eth0" },
					 { 0, 0 } };
#elif HAVE_WHRAG108
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 wlan0 wlan1" },
					 { "wan_ifname2", "eth1" },
					 { "wan_ifname", "eth1" },
					 { "wan_ifnames", "eth1" },
					 { "wan_default", "eth1" },
					 { 0, 0 } };
#elif HAVE_PB42
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth1 wlan0 wlan1" },
					 { "wan_ifname2", "eth0" },
					 { "wan_ifname", "eth0" },
					 { "wan_ifnames", "eth0" },
					 { "wan_default", "eth0" },
					 { 0, 0 } };
#elif HAVE_TW6600
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "wlan0 wlan1" },
					 { "wan_ifname2", "eth0" },
					 { "wan_ifname", "eth0" },
					 { "wan_ifnames", "eth0" },
					 { "wan_default", "eth0" },
					 { 0, 0 } };
#elif HAVE_CA8PRO
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "vlan0 wlan0" },
					 { "wan_ifname", "vlan1" },
					 { "wan_ifname2", "vlan1" },
					 { "wan_ifnames", "vlan1" },
					 { "wan_default", "vlan1" },
					 { 0, 0 } };
#elif HAVE_CA8
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "wlan0" },
					 { "wan_ifname", "eth0" },
					 { "wan_ifname2", "eth0" },
					 { "wan_ifnames", "eth0" },
					 { "wan_default", "eth0" },
					 { 0, 0 } };
#else
	struct nvram_param generic[] = { { "lan_ifname", "br0" },
					 { "lan_ifnames", "eth0 eth2 eth3 eth4" },
					 { "wan_ifname", "eth1" },
					 { "wan_ifname2", "eth1" },
					 { "wan_ifnames", "eth1" },
					 { "wan_default", "eth1" },
					 { 0, 0 } };
	struct nvram_param vlan[] = { { "lan_ifname", "br0" },
				      { "lan_ifnames", "vlan0 eth1 eth2 eth3" },
				      { "wan_ifname", "vlan1" },
				      { "wan_ifname2", "vlan1" },
				      { "wan_ifnames", "vlan1" },
				      { "wan_default", "vlan1" },
				      { 0, 0 } };

	struct nvram_param wrt350vlan[] = { { "lan_ifname", "br0" },
					    { "lan_ifnames", "vlan1 eth0" },
					    { "wan_ifname", "vlan2" },
					    { "wan_ifname2", "vlan2" },
					    { "wan_ifnames", "vlan2" },
					    { "wan_default", "vlan2" },
					    { 0, 0 } };

	struct nvram_param wnr3500vlan[] = { { "lan_ifname", "br0" },
					     { "lan_ifnames", "vlan1 eth1" },
					     { "wan_ifname", "vlan2" },
					     { "wan_ifname2", "vlan2" },
					     { "wan_ifnames", "vlan2" },
					     { "wan_default", "vlan2" },
					     { 0, 0 } };

	struct nvram_param wrt320vlan[] = { { "lan_ifname", "br0" },
					    { "lan_ifnames", "vlan1 eth1" },
					    { "wan_ifname", "vlan2" },
					    { "wan_ifname2", "vlan2" },
					    { "wan_ifnames", "vlan2" },
					    { "wan_default", "vlan2" },
					    { 0, 0 } };

	struct nvram_param wrt30011vlan[] = { { "lan_ifname", "br0" },
					      { "lan_ifnames", "vlan0 eth0" },
					      { "wan_ifname", "vlan1" },
					      { "wan_ifname2", "vlan1" },
					      { "wan_ifnames", "vlan1" },
					      { "wan_default", "vlan1" },
					      { 0, 0 } };

	struct nvram_param wrt600vlan[] = { { "lan_ifname", "br0" },
					    { "lan_ifnames", "vlan0 eth0 eth1" },
					    { "wan_ifname", "vlan2" },
					    { "wan_ifname2", "vlan2" },
					    { "wan_ifnames", "vlan2" },
					    { "wan_default", "vlan2" },
					    { 0, 0 } };

	struct nvram_param wrt60011vlan[] = { { "lan_ifname", "br0" },
					      { "lan_ifnames", "vlan1 eth0 eth1" },
					      { "wan_ifname", "vlan2" },
					      { "wan_ifname2", "vlan2" },
					      { "wan_ifnames", "vlan2" },
					      { "wan_default", "vlan2" },
					      { 0, 0 } };

	struct nvram_param wrt6102vlan[] = { { "lan_ifname", "br0" },
					     { "lan_ifnames", "vlan1 eth1 eth2" },
					     { "wan_ifname", "vlan2" },
					     { "wan_ifname2", "vlan2" },
					     { "wan_ifnames", "vlan2" },
					     { "wan_default", "vlan2" },
					     { 0, 0 } };

	struct nvram_param rt53nvlan[] = { { "lan_ifname", "br0" },
					   { "lan_ifnames", "vlan2 eth1 eth2" },
					   { "wan_ifname", "vlan1" },
					   { "wan_ifname2", "vlan1" },
					   { "wan_ifnames", "vlan1" },
					   { "wan_default", "vlan1" },
					   { 0, 0 } };

	struct nvram_param wzr144nhvlan[] = { { "lan_ifname", "br0" },
					      { "lan_ifnames", "vlan2 eth0" },
					      { "wan_ifname", "vlan1" },
					      { "wan_ifname2", "vlan1" },
					      { "wan_ifnames", "vlan1" },
					      { "wan_default", "vlan1" },
					      { 0, 0 } };

	struct nvram_param generic_2[] = { { "lan_ifname", "br0" },
					   { "lan_ifnames", "eth1 eth2" },
					   { "wan_ifname", "eth0" },
					   { "wan_ifname2", "eth0" },
					   { "wan_ifnames", "eth0" },
					   { "wan_default", "eth0" },
					   { 0, 0 } };

	struct nvram_param generic_3[] = { { "lan_ifname", "br0" },
					   { "lan_ifnames", "eth0 eth1" },
					   { "wan_ifname", "eth2" },
					   { "wan_ifname2", "eth2" },
					   { "wan_ifnames", "eth2" },
					   { "wan_default", "eth2" },
					   { 0, 0 } };

#endif

	struct nvram_param *linux_overrides;
	struct nvram_param *t, *u;
	int restore_defaults = 0;

	// uint boardflags;

	/*
	 * Restore defaults if told to.
	 * 
	 * Note: an intentional side effect is that when upgrading from a
	 * firmware without the sv_restore_defaults var, defaults will also be
	 * restored. 
	 */
	char *et0mac = nvram_safe_get("et0macaddr");
	char *et1mac = nvram_safe_get("et1macaddr");

	// unsigned char mac[20];
	// if (getRouterBrand () == ROUTER_BUFFALO_WZRG144NH)
	// {
	// if (nvram_get ("il0macaddr") == NULL)
	// {
	// strcpy (mac, et0mac);
	// MAC_ADD (mac);
	// nvram_set ("il0macaddr", mac);
	// }
	// }
	struct nvram_param *srouter_defaults;
	srouter_defaults = load_defaults();
#ifdef HAVE_RB500
	linux_overrides = generic;
	int brand = getRouterBrand();
#elif defined(HAVE_R9000) || defined(HAVE_MVEBU) || defined(HAVE_IPQ806X) || defined(HAVE_XSCALE) || defined(HAVE_X86) ||         \
	defined(HAVE_MAGICBOX) || defined(HAVE_LAGUNA) || defined(HAVE_VENTANA) || defined(HAVE_NORTHSTAR) ||                     \
	defined(HAVE_RB600) || defined(HAVE_NEWPORT) || defined(HAVE_GATEWORX) || defined(HAVE_FONERA) || defined(HAVE_SOLO51) || \
	defined(HAVE_RT2880) || defined(HAVE_LS2) || defined(HAVE_LS5) || defined(HAVE_WHRAG108) || defined(HAVE_TW6600) ||       \
	defined(HAVE_PB42) || defined(HAVE_LSX) || defined(HAVE_DANUBE) || defined(HAVE_OPENRISC) || defined(HAVE_STORM) ||       \
	defined(HAVE_ADM5120) || defined(HAVE_CA8) || defined(HAVE_OCTEON)
	int brand = getRouterBrand();
	linux_overrides = generic;

	if (nvram_invmatchi("sv_restore_defaults", 0)) // ||
	// nvram_invmatch("os_name",
	// "linux"))
	{
		restore_defaults = 1;
	}

	if (nvram_match("product_name", "INSPECTION")) {
		nvram_unset("product_name");
		restore_defaults = 1;
	}
	if (!nvram_exists("router_name")) {
		restore_defaults = 1;
	}
#elif HAVE_GEMTEK
	linux_overrides = generic;
	int brand = getRouterBrand();
#else
	int brand = getRouterBrand();

	if (nvram_invmatchi("sv_restore_defaults", 0)) // ||
	// nvram_invmatch("os_name",
	// "linux"))
	{
		// nvram_unset("sv_restore_defaults");
		restore_defaults = 1;
	}
	if (nvram_match("product_name", "INSPECTION")) {
		nvram_unset("product_name");
		restore_defaults = 1;
	}
	if (!nvram_exists("router_name")) {
		restore_defaults = 1;
	}

	if (restore_defaults) {
		cprintf("Restoring defaults...\n");
#ifdef HAVE_MICRO
		/*
		 * adjust ip_conntrack_max based on available memory size
		 * some routers that can run micro only have 16MB memory
		 */
		FILE *fmem = fopen("/proc/meminfo", "r");
		char line[128];
		unsigned long msize = 0;

		if (fmem != NULL) {
			fgets(line, sizeof(line), fmem); //eat first line
			fgets(line, sizeof(line), fmem);
			if (sscanf(line, "%*s %lu", &msize) == 1) {
				if (msize > (8 * 1024 * 1024)) {
					nvram_seti("ip_conntrack_max", 4096);
					nvram_seti("ip_conntrack_tcp_timeouts", 3600);
				}
			}
			fclose(fmem);
		}
#endif

		/*
		 * these unsets are important for routers where we can't erase nvram
		 * and only software restore defaults 
		 */
		nvram_unset("wan_to_lan");
		nvram_unset("wl_vifs");
		nvram_unset("wl0_vifs");
	}
	// }

	/*
	 * Delete dynamically generated variables 
	 */
	/*
	 * Choose default lan/wan i/f list. 
	 */
	char *ds;

	switch (brand) {
#ifndef HAVE_BUFFALO
	case ROUTER_WRTSL54GS:
	case ROUTER_WRT150N:
	case ROUTER_WRT160N:
	case ROUTER_WRT300N:
	case ROUTER_NETGEAR_WNR834B:
	case ROUTER_NETGEAR_WNR834BV2:
	case ROUTER_NETGEAR_WNDR3300:
	case ROUTER_ASUS_WL500G:
	case ROUTER_ASUS_WL500W:
#endif
	case ROUTER_BUFFALO_WZRG300N:
	case ROUTER_BUFFALO_WLAH_G54:
	case ROUTER_BUFFALO_WAPM_HP_AM54G54:
	case ROUTER_BUFFALO_WZRRSG54:
	case ROUTER_ASKEY_RT220XD:
		linux_overrides = generic;
		break;
#ifndef HAVE_BUFFALO
	case ROUTER_ASUS_WL500GD:
	case ROUTER_ASUS_WL550GE:
	case ROUTER_BELKIN_F5D7230_V3000:
		linux_overrides = vlan;
		break;
	case ROUTER_NETGEAR_WNR3500L:
	case ROUTER_NETGEAR_WNR3500LV2:
		linux_overrides = wnr3500vlan;
		break;
	case ROUTER_ASUS_RTN16:
	case ROUTER_WRT160NV3:
	case ROUTER_WRT320N:
	case ROUTER_WRT310NV2:
		linux_overrides = wrt320vlan;
		break;
	case ROUTER_WRT350N:
		linux_overrides = wrt350vlan;
		break;
	case ROUTER_WRT310N:
		linux_overrides = wrt350vlan;
		break;
	case ROUTER_WRT300NV11:
		linux_overrides = wrt30011vlan;
		break;
	case ROUTER_WRT600N:
		if (nvram_match("switch_type", "BCM5395"))
			linux_overrides = wrt60011vlan;
		else
			linux_overrides = wrt600vlan;
		break;
	case ROUTER_WRT610N:
		linux_overrides = wrt60011vlan;
		break;
	case ROUTER_WRT610NV2:
	case ROUTER_LINKSYS_E1000V2:
	case ROUTER_LINKSYS_E2500:
	case ROUTER_LINKSYS_E3200:
	case ROUTER_LINKSYS_E4200:
	case ROUTER_LINKSYS_EA2700:
	case ROUTER_LINKSYS_EA6500:
	case ROUTER_NETGEAR_WNDR4000:
	case ROUTER_NETGEAR_R6200:
	case ROUTER_NETGEAR_WNDR4500:
	case ROUTER_NETGEAR_WNDR4500V2:
	case ROUTER_NETGEAR_R6300:
	case ROUTER_ASUS_RTN66:
		linux_overrides = wrt6102vlan;
		break;
	case ROUTER_ASUS_RTN53:
		linux_overrides = rt53nvlan;
		break;
#endif
	case ROUTER_ASUS_AC66U:
	case ROUTER_D1800H:
	case ROUTER_DLINK_DIR865:
	case ROUTER_UBNT_UNIFIAC:
		linux_overrides = wrt6102vlan;
		break;
	case ROUTER_BUFFALO_WZRG144NH:
		linux_overrides = wzr144nhvlan;
		break;
#ifndef HAVE_BUFFALO
	case ROUTER_MOTOROLA_WE800G:
	case ROUTER_WAP54G_V1:
	case ROUTER_SITECOM_WL105B:
#endif
	case ROUTER_BUFFALO_WLI2_TX1_G54:
	case ROUTER_BUFFALO_WLAG54C:
		linux_overrides = generic_2;
		break;
#ifndef HAVE_BUFFALO
	case ROUTER_WAP54G_V2:
	case ROUTER_VIEWSONIC_WAPBR_100:
	case ROUTER_USR_5430:
	case ROUTER_BELKIN_F5D7230_V2000:
	case ROUTER_NETGEAR_WG602_V3:
	case ROUTER_NETGEAR_WG602_V4:
#endif
	case ROUTER_BUFFALO_WLA2G54C:
	case ROUTER_BUFFALO_WLI_TX4_G54HP:
		linux_overrides = generic_3;
		break;
#ifndef HAVE_BUFFALO
	case ROUTER_RT480W:
	case ROUTER_RT210W:
#endif
	case ROUTER_BRCM4702_GENERIC:
		// fall through
	default:
		if (check_vlan_support())
			linux_overrides = vlan;
		else
			linux_overrides = generic;
		break;
	}
#endif

	/*
	 * Restore defaults 
	 */
#if defined(HAVE_MVEBU) || defined(HAVE_IPQ806X) || defined(HAVE_XSCALE) || defined(HAVE_X86) || defined(HAVE_MAGICBOX) ||         \
	defined(HAVE_LAGUNA) || defined(HAVE_VENTANA) || defined(HAVE_NORTHSTAR) || defined(HAVE_RB600) ||                         \
	defined(HAVE_NEWPORT) || defined(HAVE_GATEWORX) || defined(HAVE_FONERA) || defined(HAVE_SOLO51) || defined(HAVE_RT2880) || \
	defined(HAVE_LS2) || defined(HAVE_LS5) || defined(HAVE_WHRAG108) || defined(HAVE_TW6600) || defined(HAVE_PB42) ||          \
	defined(HAVE_LSX) || defined(HAVE_DANUBE) || defined(HAVE_OPENRISC) || defined(HAVE_STORM) || defined(HAVE_ADM5120) ||     \
	defined(HAVE_CA8) || defined(HAVE_80211AC) || defined(HAVE_OCTEON)
	if (restore_defaults) {
		nvram_clear();
	}
#endif

#if defined(HAVE_BUFFALO) || defined(HAVE_BUFFALO_BL_DEFAULTS)
	buffalo_defaults(restore_defaults);
#endif
#ifndef HAVE_MADWIFI
	int icnt = get_wl_instances();
#else
	int icnt = getdevicecount();
	if (brand == ROUTER_LINKSYS_E2500 || brand == ROUTER_LINKSYS_E3200) //dual radio, 2nd on usb-bus
		icnt = 2;
#endif
	runStartup(".pf");
	// if (!nvram_match("default_init","1"))
	{
		for (u = linux_overrides; u && u->name; u++) {
			if (restore_defaults || !nvram_exists(u->name)) {
				nvram_set(u->name, u->value);
			}
		}
		for (t = srouter_defaults; t->name; t++) {
			if (restore_defaults || !nvram_exists(t->name)) {
				for (u = linux_overrides; u && u->name; u++) {
					if (!strcmp(u->name, t->name))
						break;
				}
				if (!u || !u->name) {
					nvram_set(t->name, t->value);
					if (icnt < 2 && startswith(t->name,
								   "wl1_")) //unset wl1_xx if we have only one radio
						nvram_unset(t->name);
					if (icnt < 3 && startswith(t->name,
								   "wl2_")) //unset wl2_xx if we have only one or two radios
						nvram_unset(t->name);
				}
			}
		}
	}
	free_defaults(srouter_defaults);
	if (!*(nvram_safe_get("http_username")) || nvram_match("http_username", "admin")) {
		char passout[MD5_OUT_BUFSIZE];
		nvram_set("http_username", DEFAULT_PASS);
		nvram_set("http_passwd", DEFAULT_USER);
	}
	if (restore_defaults) {
#ifdef HAVE_BRCMROUTER
		switch (brand) {
		case ROUTER_ASUS_WL520G:
		case ROUTER_ASUS_WL500G_PRE_V2:
		case ROUTER_BELKIN_F5D7230_V3000:
		case ROUTER_USR_5465:
			nvram_set("vlan0ports", "0 1 2 3 5*");
			nvram_set("vlan1ports", "4 5");
			break;
		case ROUTER_LINKSYS_WTR54GS:
			nvram_set("vlan0ports", "0 5*");
			nvram_set("vlan1ports", "1 5");
			break;
		case ROUTER_ASUS_WL550GE:
			nvram_set("vlan0ports", "1 2 3 4 5*");
			nvram_set("vlan1ports", "0 5");
			break;
		case ROUTER_MOTOROLA:
		case ROUTER_WRT54G_V8:
		case ROUTER_ASUS_RTN12:
			nvram_set("vlan0ports", "3 2 1 0 5*");
			nvram_set("vlan1ports", "4 5");
			break;
		case ROUTER_ASUS_RTN12B:
			nvram_set("vlan0ports", "3 2 1 0 5*");
			nvram_set("vlan1ports", "4 5");
			break;
		case ROUTER_LINKSYS_WRH54G:
		case ROUTER_ASUS_WL500GD:
		case ROUTER_BUFFALO_WBR2G54S:
			nvram_set("vlan0ports", "4 3 2 1 5*");
			nvram_set("vlan1ports", "0 5");
			break;
		case ROUTER_USR_5461:
			nvram_set("vlan0ports", "4 1 2 3 5*");
			nvram_set("vlan1ports", "0 5");
			break;
		default:
			if (nvram_match("boardnum", "WAP54GV3_8M_0614")) {
				nvram_set("vlan0ports", "3 2 1 0 5*");
				nvram_set("vlan1ports", "4 5");
			}
			break;
		}
#endif
#ifdef HAVE_SPUTNIK
		nvram_set("lan_ipaddr", "192.168.180.1");
#elif HAVE_CARLSONWIRELESS
#ifdef HAVE_LAGUNA
		nvram_set("lan_ipaddr", "192.168.3.20");
#else
		nvram_set("lan_ipaddr", "192.168.2.20");
#endif
#elif HAVE_BUFFALO
		nvram_set("lan_ipaddr", "192.168.11.1");
#elif HAVE_IDEXX
		nvram_set("lan_ipaddr", "192.168.222.1");
#elif HAVE_KORENRON
		nvram_set("lan_ipaddr", "10.0.0.1");
#elif HAVE_HOBBIT
		nvram_set("lan_ipaddr", "192.168.50.254");
#elif HAVE_ERC
		nvram_set("lan_ipaddr", "10.195.0.1");
#elif HAVE_AXTEL
		nvram_set("lan_ipaddr", "192.168.11.1");
#elif HAVE_RAYTRONIK
		nvram_set("lan_ipaddr", "10.0.0.1");
#elif HAVE_NDTRADE
		nvram_set("lan_ipaddr", "192.168.100.1");
#elif HAVE_ONNET
#ifdef HAVE_ONNET_STATION
		nvram_set("lan_ipaddr", "192.168.1.2");
		nvram_set("wlan0_mode", "wdssta");
#else
		nvram_set("lan_ipaddr", "192.168.1.1");
		nvram_set("wlan0_mode", "wdsap");
#endif
		nvram_set("lan_proto", "static");
#else
		nvram_set("lan_ipaddr", "192.168.1.1");
#endif
	}
#ifdef HAVE_SKYTRON
	if (restore_defaults) {
		nvram_set("lan_ipaddr", "192.168.0.1");
	}
#endif
#ifdef HAVE_BRCMROUTER
	switch (brand) {
	case ROUTER_WRT600N:
		if (nvram_match("switch_type", "BCM5395") && nvram_match("vlan0ports", "1 2 3 4 8*")) // fix for WRT600N
		// v1.1 (BCM5395 does
		// not suppport vid
		// 0, so gemtek
		// internally
		// configured vid 1
		// as lan)
		{
			nvram_unset("vlan0ports");
			nvram_set("vlan1ports", "1 2 3 4 8*");
			nvram_set("vlan2ports", "0 8*");
			nvram_unset("vlan0hwname");
			nvram_set("vlan1hwname", "et0");
			nvram_set("landevs", "vlan1 wl0 wl1");
			nvram_set("lan_ifnames", "vlan1 eth0 eth1");
		} else {
			if (!nvram_exists("vlan0ports") || nvram_match("vlan0ports", "") || !nvram_exists("vlan2ports") ||
			    nvram_match("vlan2ports", "")) {
				nvram_set("vlan0ports", "1 2 3 4 8*");
				nvram_set("vlan2ports", "0 8*");
			}
		}
		break;
	case ROUTER_WRT610NV2:
		if (!nvram_exists("vlan1ports") || nvram_match("vlan1ports", "") || !nvram_exists("vlan2ports") ||
		    nvram_match("vlan2ports", "")) {
			nvram_set("vlan1ports", "4 3 2 1 8*");
			nvram_set("vlan2ports", "0 8");
		}
		break;
	case ROUTER_LINKSYS_EA6500:
		if (!nvram_exists("vlan1ports") || nvram_match("vlan1ports", "") || !nvram_exists("vlan2ports") ||
		    nvram_match("vlan2ports", "")) {
			nvram_set("vlan1ports", "0 1 2 3 8*");
			nvram_set("vlan2ports", "4 8");
		}
		break;
	case ROUTER_UBNT_UNIFIAC:
		nvram_set("vlan1ports", "0 8*");
		nvram_set("vlan2ports", "1 8");
		break;
	case ROUTER_WRT610N:
	case ROUTER_WRT350N:
	case ROUTER_WRT310N:
	case ROUTER_DLINK_DIR865:
	case ROUTER_D1800H:
	case ROUTER_LINKSYS_E4200:
	case ROUTER_ASUS_AC66U:
		nvram_unset("vlan0ports");
		if (!nvram_exists("vlan1ports") || nvram_match("vlan1ports", "") || !nvram_exists("vlan2ports") ||
		    nvram_match("vlan2ports", "")) {
			nvram_set("vlan1ports", "1 2 3 4 8*");
			nvram_set("vlan2ports", "0 8");
		}
		break;
	case ROUTER_WRT300NV11:
		if (!nvram_exists("vlan0ports") || nvram_match("vlan0ports", "") || !nvram_exists("vlan1ports") ||
		    nvram_match("vlan1ports", "")) {
			nvram_set("vlan0ports", "1 2 3 4 5*");
			nvram_set("vlan1ports", "0 5");
		}
		break;
	case ROUTER_ASUS_RTN66:
		if (!nvram_exists("vlan1ports") || nvram_match("vlan1ports", "") || !nvram_exists("vlan2ports") ||
		    nvram_match("vlan2ports", "")) {
			nvram_set("vlan1ports", "1 2 3 4 8*");
			nvram_set("vlan2ports", "0 8u");
		}
		break;

	case ROUTER_ASUS_AC1200:
		if (!nvram_exists("vlan1ports") || nvram_match("vlan1ports", "") || !nvram_exists("vlan2ports") ||
		    nvram_match("vlan2ports", "")) {
			nvram_set("vlan1ports", "1 2 3 4 8*");
			nvram_set("vlan2ports", "0 8*");
		}
		break;
	case ROUTER_BUFFALO_WZR600DHP2:
	case ROUTER_BUFFALO_WZR900DHP:
	case ROUTER_BUFFALO_WZR1750:
	case ROUTER_BUFFALO_WXR1900DHP:
	case ROUTER_DLINK_DIR868:
	case ROUTER_DLINK_DIR868C:
	case ROUTER_DLINK_DIR890:
	case ROUTER_DLINK_DIR895:
	case ROUTER_DLINK_DIR860:
	case ROUTER_DLINK_DIR880:
	case ROUTER_ASUS_AC56U:
	case ROUTER_TRENDNET_TEW812:
	case ROUTER_TRENDNET_TEW811:
	case ROUTER_LINKSYS_EA6400:
		if (!nvram_exists("vlan1ports") || nvram_match("vlan1ports", "") || !nvram_exists("vlan2ports") ||
		    nvram_match("vlan2ports", "")) {
			nvram_set("vlan1ports", "0 1 2 3 5*");
			nvram_set("vlan2ports", "4 5u");
		}
		break;
	case ROUTER_DLINK_DIR885:
	case ROUTER_TRENDNET_TEW828:
		if (!nvram_exists("vlan1ports") || nvram_match("vlan1ports", "") || !nvram_exists("vlan2ports") ||
		    nvram_match("vlan2ports", "")) {
			nvram_set("vlan1ports", "0 1 2 3 5 6 7 8*");
			nvram_set("vlan2ports", "4 8u");
		}
		break;
	case ROUTER_LINKSYS_EA6900:
	case ROUTER_LINKSYS_EA6700:
	case ROUTER_LINKSYS_EA6500V2:
	case ROUTER_ASUS_RTN18U:
	case ROUTER_ASUS_AC67U:
	case ROUTER_TPLINK_ARCHERC9:
	case ROUTER_TPLINK_ARCHERC8:
	case ROUTER_TPLINK_ARCHERC3150:
	case ROUTER_TPLINK_ARCHERC3200:
		if (!nvram_exists("vlan1ports") || nvram_match("vlan1ports", "") || !nvram_exists("vlan2ports") ||
		    nvram_match("vlan2ports", "")) {
			nvram_set("vlan1ports", "1 2 3 4 5*");
			nvram_set("vlan2ports", "0 5u");
		}
		break;
	case ROUTER_ASUS_AC87U:
		if (!nvram_exists("vlan1ports") || nvram_match("vlan1ports", "") || !nvram_exists("vlan2ports") ||
		    nvram_match("vlan2ports", "")) {
			nvram_set("vlan1ports", "1 2 3 5 7*");
			nvram_set("vlan2ports", "0 7u");
		}
		break;

	case ROUTER_ASUS_AC3100:
		if (!nvram_exists("vlan1ports") || nvram_match("vlan1ports", "") || !nvram_exists("vlan2ports") ||
		    nvram_match("vlan2ports", "")) {
			nvram_set("vlan1ports", "0 1 2 3 5 7 8*");
			nvram_set("vlan2ports", "4 8u");
		}
		break;
	case ROUTER_ASUS_AC88U:
		if (!nvram_exists("vlan1ports") || nvram_match("vlan1ports", "") || !nvram_exists("vlan2ports") ||
		    nvram_match("vlan2ports", "")) {
			nvram_set("vlan1ports", "0 1 2 3 5 7*");
			nvram_set("vlan2ports", "4 7u");
		}
		break;

	case ROUTER_ASUS_AC5300:
		if (!nvram_exists("vlan1ports") || nvram_match("vlan1ports", "") || !nvram_exists("vlan2ports") ||
		    nvram_match("vlan2ports", "")) {
			nvram_set("vlan1ports", "1 2 3 4 5 7*");
			nvram_set("vlan2ports", "0 7u");
		}
		break;

	case ROUTER_LINKSYS_EA6350:
	case ROUTER_ASUS_AC3200:
		if (!nvram_exists("vlan1ports") || nvram_match("vlan1ports", "") || !nvram_exists("vlan2ports") ||
		    nvram_match("vlan2ports", "")) {
			nvram_set("vlan1ports", "0 1 2 3 5*");
			nvram_set("vlan2ports", "4 5u");
		}
		break;

	case ROUTER_ASUS_RTN53:
		if (!nvram_exists("vlan1ports") || nvram_match("vlan1ports", "") || !nvram_exists("vlan2ports") ||
		    nvram_match("vlan2ports", "")) {
			nvram_set("vlan1ports", "0 5");
			nvram_set("vlan2ports", "1 2 3 4 5*");
		}
		break;
	case ROUTER_BUFFALO_WZRG144NH:
		if (!nvram_exists("vlan1ports") || nvram_match("vlan1ports", "") || !nvram_exists("vlan2ports") ||
		    nvram_match("vlan2ports", "")) {
			nvram_set("vlan1ports", "4 8");
			nvram_set("vlan2ports", "0 1 2 3 8*");
		}
		break;
	case ROUTER_NETGEAR_EX6200:
	case ROUTER_NETGEAR_AC1450:
	case ROUTER_NETGEAR_R6250:
	case ROUTER_NETGEAR_R6300V2:
		if (!nvram_exists("vlan1ports") || nvram_match("vlan1ports", "") || !nvram_exists("vlan2ports") ||
		    nvram_match("vlan2ports", "")) {
			nvram_set("vlan1ports", "3 2 1 0 5*");
			nvram_set("vlan2ports", "4 5u");
		}
		break;
	case ROUTER_NETGEAR_R6400:
	case ROUTER_NETGEAR_R6400V2:
	case ROUTER_NETGEAR_R6700V3:
	case ROUTER_NETGEAR_R7000:
	case ROUTER_NETGEAR_R7000P:
		if (!nvram_exists("vlan1ports") || nvram_match("vlan1ports", "") || !nvram_exists("vlan2ports") ||
		    nvram_match("vlan2ports", "")) {
			nvram_set("vlan1ports", "4 3 2 1 5*");
			nvram_set("vlan2ports", "0 5u");
		}
		break;
	case ROUTER_NETGEAR_R8000:
		if (!nvram_exists("vlan1ports") || nvram_match("vlan1ports", "") || !nvram_exists("vlan2ports") ||
		    nvram_match("vlan2ports", "")) {
			nvram_set("vlan1ports", "3 2 1 0 5 7 8*");
			nvram_set("vlan2ports", "4 8u");
		}
		break;
	case ROUTER_NETGEAR_R8500:
		if (!nvram_exists("vlan1ports") || nvram_match("vlan1ports", "") || !nvram_exists("vlan2ports") ||
		    nvram_match("vlan2ports", "")) {
			nvram_set("vlan1ports", "3 2 1 4 5 7 8*");
			nvram_set("vlan2ports", "0 8u");
		}
		break;
	case ROUTER_LINKSYS_EA9500:
		if (!nvram_exists("vlan1ports") || nvram_match("vlan1ports", "") || nvram_match("vlan1ports", "0 1 2 3 5*") ||
		    !nvram_exists("vlan2ports") || nvram_match("vlan2ports", "")) {
			nvram_set("vlan1ports", "2 1 3 0 5 7 8*");
			nvram_set("vlan2ports", "4 8u");
		}
		break;
	default:
		if (!nvram_exists("vlan0hwname") || nvram_match("vlan0hwname", ""))
			nvram_set("vlan0hwname", "et0");
		if (!nvram_exists("vlan1hwname") || nvram_match("vlan1hwname", ""))
			nvram_set("vlan1hwname", "et0");

		switch (brand) {
		case ROUTER_MOTOROLA:
		case ROUTER_MOTOROLA_V1:
		case ROUTER_MOTOROLA_WE800G:
		case ROUTER_RT210W:
			if (et0mac != NULL)
				nvram_set("et0macaddr", et0mac);
			if (et1mac != NULL)
				nvram_set("et1macaddr", et1mac);
			break;
		}

		if (!nvram_exists("vlan0ports") || nvram_match("vlan0ports", "")) {
			switch (brand) {
			case ROUTER_NETGEAR_WNR3500L:
			case ROUTER_NETGEAR_WNR3500LV2:
			case ROUTER_WRT320N:
			case ROUTER_NETGEAR_WNDR4500:
			case ROUTER_NETGEAR_WNDR4500V2:
			case ROUTER_NETGEAR_EX6200:
			case ROUTER_NETGEAR_AC1450:
			case ROUTER_NETGEAR_R6250:
			case ROUTER_NETGEAR_R6300:
			case ROUTER_NETGEAR_R6300V2:
			case ROUTER_NETGEAR_R6400:
			case ROUTER_NETGEAR_R6400V2:
			case ROUTER_NETGEAR_R7000:
			case ROUTER_NETGEAR_R7000P:
			case ROUTER_NETGEAR_R8000:
			case ROUTER_NETGEAR_R8500:
				nvram_unset("vlan0hwname");
				break;
			case ROUTER_LINKSYS_WTR54GS:
				nvram_set("vlan0ports", "0 5*");
				break;
			case ROUTER_ASUS_WL500G_PRE:
			case ROUTER_ASUS_WL700GE:
				nvram_set("vlan0ports", "1 2 3 4 5*");
				break;
			case ROUTER_MOTOROLA:
			case ROUTER_WRT54G_V8:
			case ROUTER_BELKIN_F5D7231_V2000:
				nvram_set("vlan0ports", "3 2 1 0 5*");
				break;
			case ROUTER_LINKSYS_WRT55AG:
			case ROUTER_RT480W:
			case ROUTER_DELL_TRUEMOBILE_2300_V2:
			case ROUTER_ASUS_WL520G:
			case ROUTER_ASUS_WL500G_PRE_V2:
			case ROUTER_WRT54G_V81:
			case ROUTER_BELKIN_F5D7230_V3000:
			case ROUTER_USR_5465:
				nvram_set("vlan0ports", "0 1 2 3 5*");
				break;
			case ROUTER_LINKSYS_WRH54G:
			case ROUTER_ASUS_WL500GD:
			case ROUTER_BUFFALO_WBR2G54S:
				nvram_set("vlan0ports", "4 3 2 1 5*");
				break;
			case ROUTER_USR_5461:
				nvram_set("vlan0ports", "4 1 2 3 5*");
				break;
			default:
				if (nvram_matchi("bootnv_ver", 4) || nvram_match("boardnum", "WAP54GV3_8M_0614"))
					nvram_set("vlan0ports", "3 2 1 0 5*");
				else {
					if (!nvram_exists("vlan2ports"))
						nvram_set("vlan0ports", "1 2 3 4 5*");
					else
						nvram_unset("vlan0ports");
				}
				break;
			}
		}

		if (!nvram_exists("vlan1ports") || nvram_match("vlan1ports", "")) {
			switch (brand) {
				//                      case ROUTER_WRT_1900AC:
				//                              nvram_set("vlan2ports", "4 5");
				//                              break;
			case ROUTER_NETGEAR_WNR3500L:
			case ROUTER_NETGEAR_WNR3500LV2:
			case ROUTER_NETGEAR_WNDR4500:
			case ROUTER_NETGEAR_WNDR4500V2:
			case ROUTER_NETGEAR_R6300:
				nvram_set("vlan2ports", "4 8");
				break;
			case ROUTER_NETGEAR_EX6200:
			case ROUTER_NETGEAR_AC1450:
			case ROUTER_NETGEAR_R6250:
			case ROUTER_NETGEAR_R6300V2:
				nvram_set("vlan2ports", "4 5u");
				break;
			case ROUTER_NETGEAR_R6400:
			case ROUTER_NETGEAR_R6400V2:
			case ROUTER_NETGEAR_R7000:
			case ROUTER_NETGEAR_R7000P:
				nvram_set("vlan2ports", "0 5u");
				break;
			case ROUTER_NETGEAR_R8000:
				nvram_set("vlan2ports", "4 8u");
				break;
			case ROUTER_NETGEAR_R8500:
				nvram_set("vlan2ports", "0 8u");
				break;
			case ROUTER_WRT320N:
				nvram_set("vlan2ports", "0 8");
				break;
			case ROUTER_LINKSYS_WTR54GS:
				nvram_set("vlan1ports", "1 5");
				break;
			case ROUTER_ASUS_WL500G_PRE:
			case ROUTER_LINKSYS_WRH54G:
			case ROUTER_USR_5461:
				nvram_set("vlan1ports", "0 5");
				break;
			case ROUTER_MOTOROLA:
			case ROUTER_WRT54G_V8:
			case ROUTER_WRT54G_V81:
			case ROUTER_LINKSYS_WRT55AG:
			case ROUTER_RT480W:
			case ROUTER_DELL_TRUEMOBILE_2300_V2:
			case ROUTER_ASUS_WL520G:
			case ROUTER_ASUS_WL500G_PRE_V2:
			case ROUTER_BELKIN_F5D7230_V3000:
			case ROUTER_BELKIN_F5D7231_V2000:
			case ROUTER_USR_5465:
				nvram_set("vlan1ports", "4 5");
				break;
			default:
				if (nvram_matchi("bootnv_ver", 4) || nvram_match("boardnum", "WAP54GV3_8M_0614"))
					nvram_set("vlan1ports", "4 5");
				else
					nvram_set("vlan1ports", "0 5");
				break;
			}
		}
	}
#endif
#ifdef HAVE_BRCMROUTER
	if (restore_defaults && !nvram_exists("port0vlans")) {
		if (!nvram_exists("vlan2ports") && nvram_exists("vlan1ports") && nvram_exists("vlan0ports")) {
			nvram_seti("port0vlans", 1);
			nvram_seti("port1vlans", 0);
			nvram_seti("port2vlans", 0);
			nvram_seti("port3vlans", 0);
			nvram_seti("port4vlans", 0);
			nvram_set("port5vlans", "0 1 16000");
		} else {
			nvram_seti("port0vlans", 2);
			nvram_seti("port1vlans", 1);
			nvram_seti("port2vlans", 1);
			nvram_seti("port3vlans", 1);
			nvram_seti("port4vlans", 1);
			nvram_set("port5vlans", "1 2 16000");
		}
	}
	if (brand == ROUTER_WRT54G || brand == ROUTER_WRT54G1X || brand == ROUTER_LINKSYS_WRT55AG) {
		if (!nvram_exists("aa0"))
			nvram_seti("aa0", 3);
		if (!nvram_exists("ag0"))
			nvram_seti("ag0", 255);
		if (!nvram_exists("gpio2"))
			nvram_set("gpio2", "adm_eecs");
		if (!nvram_exists("gpio3"))
			nvram_set("gpio3", "adm_eesk");
		if (!nvram_exists("gpio5"))
			nvram_set("gpio5", "adm_eedi");
		if (!nvram_exists("gpio6"))
			nvram_set("gpio6", "adm_rc");
		if (!nvram_exists("boardrev") || nvram_match("boardrev", ""))
			nvram_set("boardrev", "0x10");
		if (!nvram_exists("boardflags") || nvram_match("boardflags", ""))
			nvram_set("boardflags", "0x0388");
		if (!nvram_exists("boardflags2"))
			nvram_seti("boardflags2", 0);
	}

	if (restore_defaults && (brand == ROUTER_ASUS_RTN10 || brand == ROUTER_ASUS_RTN10U || brand == ROUTER_ASUS_RTN12 ||
				 brand == ROUTER_ASUS_RTN12B || brand == ROUTER_ASUS_RTN53 || brand == ROUTER_ASUS_RTN10PLUSD1 ||
				 brand == ROUTER_ASUS_RTN16)) {
		nvram_seti("wl0_txpwr", 17);
	}

	if (restore_defaults && (brand == ROUTER_LINKSYS_E4200)) {
		nvram_seti("wl0_txpwr", 100);
		nvram_seti("wl1_txpwr", 100);
	}
#ifndef HAVE_BUFFALO
	if (restore_defaults && brand == ROUTER_BUFFALO_WHRG54S && nvram_match("DD_BOARD", "Buffalo WHR-HP-G54")) {
		nvram_seti("wl0_txpwr", 28);
	}
#endif
	if (restore_defaults && brand == ROUTER_BUFFALO_WLI_TX4_G54HP) {
		nvram_seti("wl0_txpwr", 28);
	}
#endif
	/*
	 * Always set OS defaults 
	 */
	nvram_set("os_version", SVN_REVISION);
	nvram_set("os_date", __DATE__);

	nvram_unset("shutdown");
#ifdef HAVE_SPUTNIK_APD
	/*
	 * Added for Sputnik Agent 
	 */
	nvram_unset("sputnik_mjid");
	nvram_unset("sputnik_rereg");
#endif
#ifdef HAVE_FREERADIUS
	nvram_unset("cert_running");
#endif
	nvram_unset("probe_working");
	nvram_unset("probe_blacklist");

	if (!nvram_exists("overclocking") && nvram_exists("clkfreq")) {
		char *clk = nvram_safe_get("clkfreq");
		char dup[64];

		strcpy(dup, clk);
		int j;
		for (j = 0; j < strlen(dup); j++)
			if (dup[j] == ',')
				dup[j] = 0;

		nvram_set("overclocking", dup);
	}
	cprintf("start overclocking\n");
	start_overclocking();
	cprintf("done()");
	if (nvram_exists("http_username")) {
		if (nvram_match("http_username", "")) {
#ifdef HAVE_POWERNOC
			nvram_set("http_username", "bJz7PcC1rCRJQ"); // admin
#else
			nvram_set("http_username", DEFAULT_USER); // root
#endif
		}
	}
	nvram_unset("flash_active");
	nvram_seti("service_running", 0);
	nvram_seti("ntp_success", 0);
	nvram_seti("wanup", 0);
	nvram_seti("sysup", 0);
	nvram_seti("ddns_once", 0);
	nvram_unset("rc_opt_run");

	nvram_unset("ipv6_get_dns");
	nvram_unset("ipv6_get_domain");
	nvram_unset("ipv6_get_sip_name");
	nvram_unset("ipv6_get_sip_servers");
	// convert old dhcp start
	char *dhcp_start = nvram_safe_get("dhcp_start");
	if (strlen(dhcp_start) < 4) {
		char *lan = nvram_safe_get("lan_ipaddr");
		int ip1 = get_single_ip(lan, 0);
		int ip2 = get_single_ip(lan, 1);
		int ip3 = get_single_ip(lan, 2);
		char merge[64];
		sprintf(merge, "%d.%d.%d.%s", ip1, ip2, ip3, dhcp_start);
		nvram_set("dhcp_start", merge);
	}

	cprintf("check CFE nv\n");
	if (check_now_boot() == PMON_BOOT)
		check_pmon_nv();
	else
		check_cfe_nv();
	cprintf("restore defaults\n");

	glob_t globbuf;
	char globstring[1024];
	char firststyle[32] = "";
	int globresult;

	sprintf(globstring, "/www/style/*");
	globresult = glob(globstring, GLOB_NOSORT, NULL, &globbuf);
	int i, found = 0;
	for (i = 0; i < globbuf.gl_pathc; i++) {
		char *style;
		style = strrchr(globbuf.gl_pathv[i], '/') + 1;
#ifdef HAVE_PWC
		if (strcmp(style, "pwc"))
#endif
			if (firststyle[0] == '\0')
				strcpy(firststyle, style);

		if (!strcmp(nvram_safe_get("router_style"), style)) {
			found = 1;
		}
	}
	if (found == 0 && firststyle[0] != '\0') {
		nvram_set("router_style", firststyle);
		if (!restore_defaults) {
			nvram_async_commit();
		}
	}
	globfree(&globbuf);

	/*
	 * Commit values 
	 */
	if (restore_defaults) {
		int i;

		unset_nvram();
		setWifiPass();
		nvram_async_commit();
		cprintf("done\n");
		for (i = 0; i < MAX_NVPARSE; i++) {
			del_wds_wsec(0, i);
			del_wds_wsec(1, i);
		}
	}
#ifdef HAVE_PERU
	char *dis = getUEnv("rndis");
	if (dis) {
		if (!strcmp(dis, "1")) {
			nvram_default_get("wlan0_rxantenna", "7");
			nvram_default_get("wlan0_txantenna", "7");
			char *ssid = nvram_safe_get("wlan0_ssid");
			if (!strcmp(ssid, "dd-wrt"))
				nvram_set("wlan0_ssid", "Antaira");
		} else {
			nvram_default_get("wlan0_rxantenna", "3");
			nvram_default_get("wlan0_txantenna", "3");
			nvram_default_get("wlan1_rxantenna", "7");
			nvram_default_get("wlan1_txantenna", "7");
			char *ssid = nvram_safe_get("wlan0_ssid");
			if (!strcmp(ssid, "dd-wrt")) {
				nvram_set("wlan0_ssid", "Antaira_N");
				nvram_set("wlan1_ssid", "Antaira");
			}
		}
	} else
		eval("ubootenv", "set", "rndis", "1");

#endif
}

void stop_drivers(void)
{
	// dummy
}

static void set_led_usbport(char *led, char *ports)
{
	char word[256];
	char *next;

	sysprintf("echo usbport > /sys/class/leds/%s/trigger", led);

	foreach(word, ports, next)
	{
		sysprintf("echo 1 > /sys/class/leds/%s/ports/%s", led, word);
	}
}

void load_drivers(int boot)
{
	/*
	 * #ifdef HAVE_USB //load usb driver. we will add samba server, ftp
	 * server and ctorrent support in future modules = "usbcore usb-ohci
	 * usb-uhci ehci-hcd scsi_mod usb-storage ide-core ide-detect
	 * ide-disk ide-scsi cdrom ide-cd printer sd_mod sr_mod" foreach
	 * (module, modules, next) { cprintf ("loading %s\n", module);
	 * insmod(module); } #endif 
	 */

#ifdef HAVE_USB

	fprintf(stderr, "[USB] checking...\n");
	if (nvram_matchi("usb_enable", 1)) {
		led_control(USB_POWER, LED_ON);
		led_control(USB_POWER1, LED_ON);
		led_control(LED_USB, LED_ON);
		led_control(LED_USB1, LED_ON);

		insmod("nls_base usb-common usbcore ehci-hcd ehci-platform ehci-fsl ehci-pci usb-uhci uhci-hcd usb-ohci ohci-hcd ohci-pci xhci-hcd xhci-pci xhci-plat-hcd xhci-mtk dwc_otg usb-libusual fsl-mph-dr-of phy-mxs-usb extcon-core extcon ci_hdrc ci13xxx_imx usbmisc_imx ci_hdrc_imx phy-qcom-dwc3 dwc3-of-simple dwc3 dwc3-qcom phy-qcom-hsusb phy-qcom-ssusb phy-qcom-ipq806x-usb phy-qcom-ipq806x-sata phy-qcom-ipq4019-usb");

#ifdef HAVE_IPQ806X
		sleep(5);
		rmmod("xhci-plat-hcd");
		insmod("xhci-plat-hcd");
#endif

		if (nvram_matchi("usb_storage", 1)) {
			insmod("insmod bsg scsi_common scsi_mod scsi_wait_scan crct10dif_common crct10dif_generic crct10dif-arm-ce crc-t10dif crc64 crc64-rocksoft crc64-rocksoft_generic crct-t10dif t10-pi sd_mod cdrom sr_mod usb-storage uas libata sata_mv ehci-orion ses");
			insmod("libahci libahci_platform ahci ahci_platform ahci_platforms ahci_imx ahci_mvebu mmc_core pwrseq_emmc pwrseq_simple mmc_block sdhci sdhci-pltfm sdhci-esdhc-imx sdhci-pxav3");
		}

		if (nvram_matchi("usb_printer", 1)) {
			cprintf("loading printer\n");
			insmod("printer usblp");
		}
#ifdef HAVE_USBIP
		if (nvram_matchi("usb_ip", 1)) {
			cprintf("loading usb over ip drivers\n");
			insmod("usbip_common_mod usbip usbip-core usbip-host vhci-hcd");
			eval("usbipd", "-D");
		}
#endif

		//ahci

		mount("devpts", "/proc/bus/usb", "usbfs", MS_MGC_VAL, NULL);
	} else if (!boot) {
		led_control(USB_POWER, LED_OFF);
		led_control(USB_POWER1, LED_OFF);

		led_control(LED_USB, LED_OFF);
		led_control(LED_USB1, LED_OFF);
		eval("stopservice", "cron");
		eval("stopservice", "samba3");
		eval("stopservice", "nfs");
		eval("stopservice", "rsync");
		eval("stopservice", "dlna");
		eval("stopservice", "ftpsrv");
#ifdef HAVE_WEBSERVER
		eval("stopservice", "lighttpd");
#endif
#ifdef HAVE_TRANSMISSION
		eval("stopservice", "transmission");
#endif
#ifdef HAVE_PLEX
		eval("stopservice", "plex");
#endif
		sysprintf("umount /%s", nvram_default_get("usb_mntpoint", "mnt"));
		rmmod("phy-qcom-hsusb");
		rmmod("phy-qcom-ssusb");
		rmmod("phy-qcom-ipq806x-sata");
		rmmod("phy-qcom-ipq806x-usb");
		rmmod("phy-qcom-ipq4019-usb");
		rmmod("dwc3-qcom");
		rmmod("dwc3");
		rmmod("dwc3-of-simple");
		rmmod("phy-qcom-dwc3");
		rmmod("usblp");
		rmmod("printer");
		rmmod("uas");
		rmmod("usb-storage");
		rmmod("sr_mod");
		rmmod("cdrom");
		rmmod("sd_mod");
		rmmod("t10-pi");
		rmmod("scsi_wait_scan");
		rmmod("ses");
		rmmod("scsi_mod");
		rmmod("scsi_common");

		rmmod("usbmisc_imx");
		rmmod("ci13xxx_imx");
		rmmod("ci_hdrc");
		rmmod("extcon");
		rmmod("phy-mxs-usb");
		rmmod("fsl-mph-dr-of");
		rmmod("ehci-fsl");
		rmmod("usb-libusual");
		rmmod("dwc_otg"); // usb
		rmmod("xhci-mtk");
		rmmod("xhci-pci");
		rmmod("xhci-plat-hcd");
		rmmod("xhci-hcd");

		rmmod("usb-ohci");
		rmmod("ohci-pci");
		rmmod("ohci-hcd");
		rmmod("uhci-hcd");
		rmmod("usb-uhci");
		rmmod("ehci-pci");
		rmmod("ehci-platform");
		rmmod("ehci-hcd");
		rmmod("fsl-mph-dr-of");
		rmmod("usbcore");
		rmmod("usb-common");

		/* unload filesystems */
		/* xfs */
		rmmod("xfs");
		/* fat */
		rmmod("msdos");
		rmmod("vfat");
		rmmod("fat");
//
/* ext3 */
#ifdef HAVE_USB_ADVANCED
		rmmod("ext3");
		rmmod("jbd");
#endif
		/* ext2 */
		rmmod("ext2");
		rmmod("mbcache");
/* ntfs-3g */
#ifdef HAVE_NTFS3G
		rmmod("fuse");
		rmmod("antfs");
		rmmod("ntfs3");
#endif
		eval("startservice_f", "cron");
		eval("startservice_f", "samba3");
		eval("startservice_f", "nfs");
		eval("startservice_f", "rsync");
		eval("startservice_f", "dlna");
		eval("startservice_f", "ftpsrv");
#ifdef HAVE_WEBSERVER
		eval("startservice_f", "lighttpd");
#endif
#ifdef HAVE_TRANSMISSION
		eval("startservice_f", "transmission");
#endif
#ifdef HAVE_PLEX
		eval("startservice_f", "plex");
#endif

	} else {
		led_control(USB_POWER, LED_OFF);
		led_control(USB_POWER1, LED_OFF);

		led_control(LED_USB, LED_OFF);
		led_control(LED_USB1, LED_OFF);
	}
#endif
	/*#ifdef HAVE_R9000
	set_smp_affinity(261, 1);
#endif
#ifdef HAVE_NORTHSTAR
	set_smp_affinity(111, 2);
	set_smp_affinity(112, 2);
#endif
#ifdef HAVE_MT7621
	set_smp_affinity(30, 8);	// usb;
#endif*/

#ifdef HAVE_MVEBU
	if (nvram_matchi("usb_enable", 1)) {
		int brand = getRouterBrand();
		insmod("ledtrig-usbport");
		if (brand == ROUTER_WRT_1900AC) {
			set_led_usbport("mamba\\:white\\:usb3_2", "usb3-port2");
			set_led_usbport("mamba\\:white\\:usb3_1", "usb2-port1 usb3-port1");
			set_led_usbport("mamba\\:white\\:usb2", "usb1-port1");
		}

		if (brand == ROUTER_WRT_1200AC) {
			set_led_usbport("pca963x\\:caiman\\:white\\:usb3_2", "usb3-port1");
			set_led_usbport("pca963x\\:caiman\\:white\\:usb3_1", "usb2-port1 usb3-port1");
			set_led_usbport("pca963x\\:caiman\\:white\\:usb2", "usb1-port1");
		}

		if (brand == ROUTER_WRT_1900ACV2) {
			set_led_usbport("pca963x\\:cobra\\:white\\:usb3_2", "usb3-port2");
			set_led_usbport("pca963x\\:cobra\\:white\\:usb3_1", "usb2-port1 usb3-port1");
			set_led_usbport("pca963x\\:cobra\\:white\\:usb2", "usb1-port1");
		}

		if (brand == ROUTER_WRT_1900ACS) {
			set_led_usbport("pca963x\\:shelby\\:white\\:usb3_2", "usb3-port2");
			set_led_usbport("pca963x\\:shelby\\:white\\:usb3_1", "usb2-port1 usb3port1");
			set_led_usbport("pca963x\\:shelby\\:white\\:usb2", "usb1-port1");
		}

		if (brand == ROUTER_WRT_3200ACM) {
			set_led_usbport("pca963x\\:rango\\:white\\:usb3_2", "usb3-port2");
			set_led_usbport("pca963x\\:rango\\:white\\:usb3_1", "usb2-port1 usb3-port1");
			set_led_usbport("pca963x\\:rango\\:white\\:usb2", "usb1-port1");
		}

		if (brand == ROUTER_WRT_32X) {
			set_led_usbport("pca963x\\:venom\\:blue\\:usb3_2", "usb3-port2");
			set_led_usbport("pca963x\\:venom\\:blue\\:usb3_1", "usb2-port1 usb3-port1");
			set_led_usbport("pca963x\\:venom\\:blue\\:usb2", "usb1-port1");
		}
	}
#endif
}

void start_drivers_net(void)
{
	load_drivers(0);
}

void start_drivers(void)
{
	load_drivers(1);
}

/*
 * States 
 */
enum {
	RESTART,
	STOP,
	START,
	TIMER,
	USER,
	IDLE,
};
static int state = START;
static int signalled = -1;

/*
 * Signal handling 
 */
static void rc_signal(int sig)
{
	if (state == IDLE) {
		if (sig == SIGHUP) {
			printf("signalling RESTART\n");
			signalled = RESTART;
		} else if (sig == SIGUSR2) {
			printf("signalling START\n");
			signalled = START;
		} else if (sig == SIGINT) {
			printf("signalling STOP\n");
			signalled = STOP;
		} else if (sig == SIGALRM) {
			printf("signalling TIMER\n");
			signalled = TIMER;
		} else if (sig == SIGUSR1) { // Receive from WEB
			printf("signalling USER1\n");
			signalled = USER;
		}
	}
}

/*
 * Timer procedure 
 */
int do_timer(void)
{
	// do_ntp();
	return 0;
}

#define CONVERT_NV(old, new)   \
	if (nvram_exists(old)) \
		nvram_set(new, nvram_safe_get(old));

void start_nvram(void)
{
	int i = 0;

	nvram_unset("wl0_hwaddr"); // When disbale wireless, we must get

	nvram_set("wan_get_dns", "");
	nvram_set("openvpn_get_dns", "");
	nvram_seti("filter_id", 1);
	nvram_set("ddns_change", "");
	nvram_unset("action_service");
	nvram_set("wan_get_domain", "");

#ifdef HAVE_SSHD
	nvram_unset(
		"sshd_keyready"); //egc reset SSH key ready as the private key file /tmp/id_rsa_ssh is gone after reboot so cannot be downloaded anymore
#endif

#ifdef HAVE_BRCMROUTER
	nvram_seti("wl_active_add_mac", 0);
	if (nvram_matchi("wl_gmode", 5)) // Mixed mode had been
		// changed to 5
		nvram_seti("wl_gmode", 1);

	if (nvram_matchi("wl_gmode", 4)) // G-ONLY mode had been
		// changed to 2, after 1.40.1
		// for WiFi G certication
		nvram_seti("wl_gmode", 2);
#endif
	nvram_set("ping_ip", "");
	nvram_set("ping_times", "");

	nvram_set("filter_port", ""); // The name have been disbaled from

#ifdef HAVE_WIREGUARD
	//egc reset WireGuard failstate on boot
	int iw;
	int iend = 0;
	iend = nvram_geti("oet_tunnels");
	for (iw = 1; iw <= iend; iw++) {
		nvram_nseti(0, "oet%d_failstate", iw);
	}
#endif

#ifdef HAVE_UPNP
	if ((nvram_matchi("restore_defaults", 1)) || (nvram_matchi("upnpcas", 1))) {
		nvram_seti("upnp_clear", 1);
	} else {
		char s[32];
		char *nv;

		for (i = 0; i < MAX_NVPARSE; ++i) {
			sprintf(s, "forward_port%d", i);
			if (*(nv = nvram_safe_get(s))) {
				if (strstr(nv, "msmsgs"))
					nvram_unset(s);
			}
		}
	}
	nvram_set("upnp_wan_proto", "");
#endif

	/*
	 * The tkip and aes already are changed to wl_crypto from v3.63.3.0 
	 */
#ifdef HAVE_BRCMROUTER
	if (nvram_match("wl_wep", "tkip")) {
		nvram_set("wl_crypto", "tkip");
	} else if (nvram_match("wl_wep", "aes")) {
		nvram_set("wl_crypto", "aes");
	} else if (nvram_match("wl_wep", "tkip+aes")) {
		nvram_set("wl_crypto", "tkip+aes");
	}

	if (nvram_match("wl_wep", "restricted"))
		nvram_set("wl_wep", "enabled"); // the nas need this value,
#endif
#ifdef HAVE_SET_BOOT
	if (!nvram_matchi("boot_wait_web", 0))
		nvram_seti("boot_wait_web", 1);
#endif
#ifndef HAVE_BUFFALO
	if (check_hw_type() == BCM5352E_CHIP) {
		nvram_seti("opo", 0); // OFDM power reducement in quarter
		// dbm (2 dbm in this case)
		nvram_seti("ag0", 0); // Antenna Gain definition in dbm
	}
#endif

	if (nvram_match("svqos_port1bw", "full"))
		nvram_set("svqos_port1bw", "FULL");
	if (nvram_match("svqos_port2bw", "full"))
		nvram_set("svqos_port2bw", "FULL");
	if (nvram_match("svqos_port3bw", "full"))
		nvram_set("svqos_port3bw", "FULL");
	if (nvram_match("svqos_port4bw", "full"))
		nvram_set("svqos_port4bw", "FULL");
	// dirty fix for WBR2 units

	// clean old filter_servicesX to free nvram
	nvram_unset("filter_services0");
	nvram_unset("filter_services1");
	nvram_unset("filter_services2");
	nvram_unset("filter_services3");
	nvram_unset("filter_services4");
	nvram_unset("filter_services5");
	nvram_unset("filter_services6");
	nvram_unset("filter_services7");

	int seq;
	for (seq = 1; seq <= 20; seq++) {
		char enabled[32];
		sprintf(enabled, "tod%d_enabled", seq);
		nvram_unset(enabled);
	}

	nvram_unset("3gcontrol");
	nvram_unset("3gdata");
	nvram_unset("wan_3g_mode");
	nvram_unset("wan_3g_signal");
	nvram_unset("wan_3g_status");
	nvram_unset("wan_3g_imsi");

	// fix openvpnclient and server values (was 0/1 now is yes/no/adaptive)
	// convert 0 -> no and 1 -> adaptive
	if (nvram_matchi("openvpn_lzo", 0))
		nvram_set("openvpn_lzo", "no");
	if (nvram_matchi("openvpn_lzo", 1))
		nvram_set("openvpn_lzo", "adaptive");

	if (nvram_matchi("openvpncl_lzo", 0))
		nvram_set("openvpncl_lzo", "no");
	if (nvram_matchi("openvpncl_lzo", 1))
		nvram_set("openvpncl_lzo", "adaptive");

	nvram_unset("vdsl_state"); // important (this value should never
	nvram_unset("fromvdsl"); // important (this value should never be

	nvram_unset("do_reboot"); //for GUI, see broadcom.c
	nvram_seti("auth_time", 0);

#ifdef DIST
	nvram_set("dist_type", DIST);
#endif

	{
#ifdef DIST
#ifndef HAVE_TW6600
#ifdef HAVE_MICRO
		// if dist_type micro, check styles, and force to elegant if needed

#ifdef HAVE_ROUTERSTYLE
		char *style = nvram_safe_get("router_style");

		if (!strstr("blue cyan elegant green orange red yellow", style))
#endif
		{
			nvram_set("router_style", "elegant");
		}
#endif
#endif
#endif
	}

#ifdef HAVE_WIVIZ
	if (!*(nvram_safe_get("hopseq")) || !*(nvram_safe_get("hopdwell"))) {
		nvram_seti("hopdwell", 1000);
		nvram_seti("hopseq", 0);
	}
#endif
	nvram_unset("lasthour");

#ifdef HAVE_SVQOS
	char *aqd = nvram_safe_get("svqos_aqd");
#ifndef HAVE_CODEL
	if (!strcmp(aqd, "codel")) {
		nvram_set("svqos_aqd", "sfq");
	}
#endif
#ifndef HAVE_FQ_CODEL
	if (!strcmp(aqd, "fq_codel")) {
		nvram_set("svqos_aqd", "sfq");
	}
#endif
#ifndef HAVE_FQ_CODEL_FAST
	if (!strcmp(aqd, "fq_codel_fast")) {
		nvram_set("svqos_aqd", "sfq");
	}
#endif
#ifndef HAVE_PIE
	if (!strcmp(aqd, "pie")) {
		nvram_set("svqos_aqd", "sfq");
	}
#endif
#ifndef HAVE_CAKE
	if (!strcmp(aqd, "cake")) {
		nvram_set("svqos_aqd", "sfq");
	}
#endif
	if (strcmp(aqd, "codel") && strcmp(aqd, "fq_codel") && strcmp(aqd, "fq_codel_fast") && strcmp(aqd, "pie") &&
	    strcmp(aqd, "cake")) {
		nvram_set("svqos_aqd", "sfq");
	}
#endif

#ifdef HAVE_AQOS
	//filter hostapd shaping rules

	char *qos_mac = nvram_safe_get("svqos_macs");

	if (*qos_mac) {
		size_t len = strlen(qos_mac) + 254;
		char *newqos = calloc(len, 1);

		char level[32], level2[32], data[32], type[32], level3[32], prio[32];

		do {
			if (sscanf(qos_mac, "%31s %31s %31s %31s %31s %31s |", data, level, level2, type, level3, prio) < 6)
				break;

			if (!strcmp(level3, "|")) {
				strcpy(level3, "0");
				strcpy(prio, "0");
			}
			if (!strcmp(prio, "|"))
				strcpy(prio, "0");

			if (strcmp(type, "hostapd") && strcmp(type, "pppd")) {
				if (*newqos)
					snprintf(newqos, len, "%s %s %s %s %s %s %s |", newqos, data, level, level2, type, level3,
						 prio);
				else
					snprintf(newqos, len, "%s %s %s %s %s %s |", data, level, level2, type, level3, prio);
			}
		} while ((qos_mac = strpbrk(++qos_mac, "|")) && qos_mac++);
		nvram_set("svqos_macs", newqos);
		free(newqos);
	}

	char *qos_ip = nvram_safe_get("svqos_ips");

	if (*qos_ip) {
		size_t len = strlen(qos_ip) + 254;
		char *newip = calloc(len, 1);

		char data[32], level[32], level2[32], level3[32], prio[32];

		do {
			if (sscanf(qos_ip, "%31s %31s %31s %31s %31s |", data, level, level2, level3, prio) < 5)
				break;

			if (!strcmp(level3, "|")) {
				strcpy(level3, "0");
				strcpy(prio, "0");
			}
			if (!strcmp(prio, "|"))
				strcpy(prio, "0");

			if (*newip)
				snprintf(newip, len, "%s %s %s %s %s %s |", newip, data, level, level2, level3, prio);
			else
				snprintf(newip, len, "%s %s %s %s %s |", data, level, level2, level3, prio);
		} while ((qos_ip = strpbrk(++qos_ip, "|")) && qos_ip++);
		nvram_set("svqos_ips", newip);
		free(newip);
	}
#endif
	if (nvram_geti("nvram_ver") < 5) {
		nvram_seti("nvram_ver", 5);
		nvram_seti("block_multicast", 1);
#ifdef HAVE_MADWIFI
		char *buf;
		int NVRAMSPACE = nvram_size();
		if (!(buf = safe_malloc(NVRAMSPACE)))
			return;

		/*
		 * Get NVRAM variables 
		 */
		char *s;
		nvram_getall(buf, NVRAMSPACE);
		s = buf;
		while (*s) {
			int len = strlen(s);
			char *name = s;
			char *value = strchr(s, '=');
			value[0] = 0;
			value++;
			size_t slen = (strlen(name) * 2) + 1;
			char *newname = malloc(slen);
			strcpy(newname, name);
			int found = 0;
			if (!strncmp(s, "ath", 3) && isdigit(s[3])) {
				snprintf(newname, slen, "wlan%s", &name[3]);
				found = 1;
			}
			if (!strncmp(s, "bat_ath", 3) && isdigit(s[7])) {
				snprintf(newname, slen, "bat_wlan%s", &name[7]);
				found = 1;
			}
			char *next;
			slen = (strlen(value) * 2) + 1;
			char *newvalue = malloc(slen);
			char *entry = malloc(strlen(value) + 1);
			*newvalue = 0;
			int first = 1;
			foreach(entry, value, next)
			{
				if (!strncmp(entry, "ath", 3) && isdigit(entry[3])) {
					found = 1;
					if (first)
						snprintf(newvalue, slen, "wlan%s", &entry[3]);
					else
						snprintf(newvalue, slen, "%s wlan%s", newvalue, &entry[3]);

				} else {
					strspcattach(newvalue, entry);
				}
				first = 0;
			}
			if (found) {
				nvram_unset(name);
				nvram_set(newname, newvalue);
			}
			free(entry);
			free(newvalue);
			free(newname);
			s += len + 1;
		}
		free(buf);

		char word[256];
		char *next, *wordlist;
		wordlist = nvram_safe_get("bridgesif");
		size_t slen = strlen(wordlist) * 2 + 1;
		char *newwordlist = malloc(slen);
		*newwordlist = 0;
		foreach(word, wordlist, next)
		{
			GETENTRYBYIDX(tag, word, 0);
			GETENTRYBYIDX(port, word, 1);
			GETENTRYBYIDX(prio, word, 2);
			GETENTRYBYIDX(hairpin, word, 3);
			GETENTRYBYIDX(stp, word, 4);
			GETENTRYBYIDX(pathcost, word, 5);
			if (!hairpin)
				hairpin = "0";
			if (!stp)
				stp = "1";
			if (!pathcost)
				pathcost = "100";

			char newname[64];
			strcpy(newname, port);
			if (!strncmp(port, "ath", 3))
				sprintf(newname, "wlan%s", &port[3]);
			if (*newwordlist)
				snprintf(newwordlist, slen, "%s %s>%s>%s>%s>%s>%s", newwordlist, tag, newname, prio, hairpin, stp,
					 pathcost);
			else
				snprintf(newwordlist, slen, "%s>%s>%s>%s>%s>%s", tag, newname, prio, hairpin, stp, pathcost);
		}
		nvram_set("bridgesif", newwordlist);
		free(newwordlist);
		wordlist = nvram_safe_get("vlan_tags");
		slen = strlen(wordlist) * 2 + 1;
		newwordlist = malloc(slen);
		*newwordlist = 0;
		foreach(word, wordlist, next)
		{
			GETENTRYBYIDX(ifname, word, 0);
			GETENTRYBYIDX(tag, word, 1);
			GETENTRYBYIDX(prio, word, 2);
			char newname[64];
			strcpy(newname, ifname);
			if (!strncmp(ifname, "ath", 3))
				sprintf(newname, "wlan%s", &ifname[3]);
			if (*newwordlist)
				snprintf(newwordlist, slen, "%s %s>%s>%s", newwordlist, newname, tag, prio);
			else
				snprintf(newwordlist, slen, "%s>%s>%s", newname, tag, prio);
		}
		nvram_set("vlan_tags", newwordlist);
		free(newwordlist);
		wordlist = nvram_safe_get("static_route");
		slen = strlen(wordlist) * 2 + 1;
		newwordlist = malloc(slen);
		*newwordlist = 0;
		foreach(word, wordlist, next)
		{
			GETENTRYBYIDX(ipaddr, word, 0);
			GETENTRYBYIDX(netmask, word, 1);
			GETENTRYBYIDX(gateway, word, 2);
			GETENTRYBYIDX(metric, word, 3);
			GETENTRYBYIDX(ifname, word, 4);
			char newname[64];
			strcpy(newname, ifname);
			if (!strncmp(ifname, "ath", 3))
				sprintf(newname, "wlan%s", &ifname[3]);
			if (*newwordlist)
				snprintf(newwordlist, slen, "%s %s:%s:%s:%s:%s:", newwordlist, ipaddr, netmask, gateway, metric,
					 newname);
			else
				snprintf(newwordlist, slen, "%s:%s:%s:%s:%s:", ipaddr, netmask, gateway, metric, newname);
		}
		nvram_set("static_route", newwordlist);
		free(newwordlist);

		wordlist = nvram_safe_get("mdhcpd");
		slen = strlen(wordlist) * 2 + 1;
		newwordlist = malloc(slen);
		*newwordlist = 0;
		foreach(word, wordlist, next)
		{
			GETENTRYBYIDX(ifname, word, 0);
			GETENTRYBYIDX(status, word, 1);
			GETENTRYBYIDX(start, word, 2);
			GETENTRYBYIDX(count, word, 3);
			GETENTRYBYIDX(leasetime, word, 4);
			char newname[64];
			strcpy(newname, ifname);
			if (!strncmp(ifname, "ath", 3))
				sprintf(newname, "wlan%s", &ifname[3]);
			if (*newwordlist)
				snprintf(newwordlist, slen, "%s %s>%s>%s>%s>%s", newwordlist, newname, status, start, count,
					 leasetime);
			else
				snprintf(newwordlist, slen, "%s>%s>%s>%s>%s", newname, status, start, count, leasetime);
		}
		nvram_set("mdhcpd", newwordlist);
		free(newwordlist);

		wordlist = nvram_safe_get("olsrd_interfaces");
		slen = strlen(wordlist) * 2 + 1;
		newwordlist = malloc(slen);
		*newwordlist = 0;

		foreach(word, wordlist, next)
		{
			GETENTRYBYIDX(interface, word, 0);
			GETENTRYBYIDX(hellointerval, word, 1);
			GETENTRYBYIDX(hellovaliditytime, word, 2);
			GETENTRYBYIDX(tcinterval, word, 3);
			GETENTRYBYIDX(tcvaliditytime, word, 4);
			GETENTRYBYIDX(midinterval, word, 5);
			GETENTRYBYIDX(midvaliditytime, word, 6);
			GETENTRYBYIDX(hnainterval, word, 7);
			GETENTRYBYIDX(hnavaliditytime, word, 8);
			GETENTRYBYIDX(linkqualitymult, word, 9);
			char newname[64];
			strcpy(newname, interface);
			if (!strncmp(interface, "ath", 3))
				sprintf(newname, "wlan%s", &interface[3]);

			if (*newwordlist)
				snprintf(newwordlist, slen, "%s %s>%s>%s>%s>%s>%s>%s>%s>%s>%s", newwordlist, newname, hellointerval,
					 hellovaliditytime, tcinterval, tcvaliditytime, midinterval, midvaliditytime, hnainterval,
					 hnavaliditytime, linkqualitymult);
			else
				snprintf(newwordlist, slen, "%s>%s>%s>%s>%s>%s>%s>%s>%s>%s", newname, hellointerval,
					 hellovaliditytime, tcinterval, tcvaliditytime, midinterval, midvaliditytime, hnainterval,
					 hnavaliditytime, linkqualitymult);
		}

		nvram_set("olsrd_interfaces", newwordlist);
		free(newwordlist);

		nvram_async_commit();
#endif
	}
	if (nvram_geti("nvram_ver") < 6) {
		nvram_seti("nvram_ver", 6);
		nvram_seti("portvlan_count", 16);
		char *next;
		char var[32];
		int i;
		for (i = 0; i < 7; i++) {
			char *port = nvram_nget("port%dvlans", i);
			char conv[1024] = { 0 };
			foreach(var, port, next)
			{
				int tmp = atoi(var);
				if (tmp >= 16 && tmp < 32) {
					if (tmp == 21)
						tmp = 18;
					else if (tmp == 18)
						tmp = 19;
					else if (tmp == 19)
						tmp = 20;
					else if (tmp == 20)
						tmp = 21;

					tmp -= 16;
					tmp = tmp * 1000 + 16000;
				}
				if (conv[0])
					sprintf(conv, "%s %d", conv, tmp);
				else
					sprintf(conv, "%d", tmp);
			}
			nvram_nset(conv, "port%dvlans", i);
		}
		nvram_set("portvlanlist", "0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15");
	}
	if (nvram_geti("nvram_ver") < 7) {
		nvram_seti("nvram_ver", 7);
#ifdef HAVE_MADWIFI
		nvram_set("wifi_display", "wlan0");
#endif
	}
	if (nvram_geti("nvram_ver") < 9) {
		nvram_seti("nvram_ver", 9);
#ifndef HAVE_MADWIFI
		if (nvram_match("wl0_web_filter", "1"))
			nvram_set("wl0_web_filter", "0");
		else
			nvram_set("wl0_web_filter", "1");

		if (nvram_match("wl1_web_filter", "1"))
			nvram_set("wl1_web_filter", "0");
		else
			nvram_set("wl1_web_filter", "1");
#endif
	}

	if (nvram_geti("nvram_ver") < 10) {
		nvram_seti("nvram_ver", 10);
		nvram_unset("port0vlans");
		nvram_unset("port1vlans");
		nvram_unset("port2vlans");
		nvram_unset("port3vlans");
		nvram_unset("port4vlans");
		nvram_unset("port5vlans");
		nvram_unset("port6vlans");
		nvram_seti("vlans", 0);
		nvram_commit();
		eval("reboot");
	}

	return;
}

static void unset_nvram(void)
{
#ifndef MPPPOE_SUPPORT
	nvram_safe_unset("ppp_username_1");
	nvram_safe_unset("ppp_passwd_1");
	nvram_safe_unset("ppp_idletime_1");
	nvram_safe_unset("ppp_demand_1");
	nvram_safe_unset("ppp_redialperiod_1");
	nvram_safe_unset("ppp_service_1");
	nvram_safe_unset("mpppoe_enable");
	nvram_safe_unset("mpppoe_dname");
#endif
#ifndef HAVE_HTTPS
	nvram_safe_unset("remote_mgt_https");
#endif
#ifndef HSIAB_SUPPORT
	nvram_safe_unset("hsiab_mode");
	nvram_safe_unset("hsiab_provider");
	nvram_safe_unset("hsiab_device_id");
	nvram_safe_unset("hsiab_device_password");
	nvram_safe_unset("hsiab_admin_url");
	nvram_safe_unset("hsiab_registered");
	nvram_safe_unset("hsiab_configured");
	nvram_safe_unset("hsiab_register_ops");
	nvram_safe_unset("hsiab_session_ops");
	nvram_safe_unset("hsiab_config_ops");
	nvram_safe_unset("hsiab_manual_reg_ops");
	nvram_safe_unset("hsiab_proxy_host");
	nvram_safe_unset("hsiab_proxy_port");
	nvram_safe_unset("hsiab_conf_time");
	nvram_safe_unset("hsiab_stats_time");
	nvram_safe_unset("hsiab_session_time");
	nvram_safe_unset("hsiab_sync");
	nvram_safe_unset("hsiab_config");
#endif

#ifndef HEARTBEAT_SUPPORT
	nvram_safe_unset("hb_server_ip");
	nvram_safe_unset("hb_server_domain");
#endif

#ifndef PARENTAL_CONTROL_SUPPORT
	nvram_safe_unset("artemis_enable");
	nvram_safe_unset("artemis_SVCGLOB");
	nvram_safe_unset("artemis_HB_DB");
	nvram_safe_unset("artemis_provisioned");
#endif

#ifndef WL_STA_SUPPORT
	// nvram_safe_unset("wl_ap_ssid");
	// nvram_safe_unset("wl_ap_ip");
#endif

#ifdef HAVE_3G
	// make sure we dial in mode 5 with 3g first!
	nvram_unset("3gnetmodetoggle");
#endif
}
