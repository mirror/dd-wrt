
/*
 * sysinit.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#include <bcmutils.h>
#include <shutils.h>
#include <wlutils.h>
#include <cy_conf.h>
#include <cymac.h>
#include <glob.h>
// #include <ledcontrol.h>

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

extern struct nvram_tuple *srouter_defaults;
extern void load_defaults(void);
extern void free_defaults(void);

extern int usb_add_ufd(void);

extern int f_exists(const char *path);

int endswith(char *str, char *cmp)
{
	int cmp_len, str_len, i;

	cmp_len = strlen(cmp);
	str_len = strlen(str);
	if (cmp_len > str_len)
		return (0);
	for (i = 0; i < cmp_len; i++) {
		if (str[(str_len - 1) - i] != cmp[(cmp_len - 1) - i])
			return (0);
	}
	return (1);
}

#ifdef HAVE_MACBIND
#include "../../../opt/mac.h"
#endif

void runStartup(char *folder, char *extension)
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
		if (!strcmp(extension, "K**") && strlen(entry[n]->d_name) > 3 && startswith(entry[n]->d_name, "K") && strspn(entry[n]->d_name, "K1234567890") == 3) {	// K* scripts
			sprintf(fullname, "%s/%s", folder, entry[n]->d_name);
			if (!stat(fullname, &filestat)
			    && (filestat.st_mode & S_IXUSR))
				sysprintf("%s 2>&1 > /dev/null", fullname);
			free(entry[n]);
			n++;
			continue;
		}
		if (!strcmp(extension, "S**") && strlen(entry[n]->d_name) > 3 && startswith(entry[n]->d_name, "S") && strspn(entry[n]->d_name, "S1234567890") == 3) {	// S* scripts
			sprintf(fullname, "%s/%s", folder, entry[n]->d_name);
			if (!stat(fullname, &filestat)
			    && (filestat.st_mode & S_IXUSR))
				sysprintf("%s 2>&1 > /dev/null", fullname);
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
			sysprintf("%s/%s 2>&1 > /dev/null", folder, entry[n]->d_name);
			// execute script 
		}
		free(entry[n]);
		n++;
	}
	free(entry);
	
//	sysprintf("/usr/sbin/wl interference_override 4");
//	sysprintf("/usr/sbin/wl -i eth1 interference 3");
//	sysprintf("/usr/sbin/wl -i eth2 interference 3");
	
	return;
}

#if defined(HAVE_BUFFALO) || defined(HAVE_BUFFALO_BL_DEFAULTS)

#ifdef HAVE_BCMMODERN
#define getUEnv(name) nvram_get(name)
static void buffalo_defaults(int force)
{

	char *pincode = getUEnv("pincode");
	if (pincode && nvram_get("pincode") == NULL) {
		nvram_set("pincode", pincode);
	}
	if (nvram_get("wl0_akm") == NULL || force) {
		char *region = getUEnv("region");
		if (!region || (strcmp(region, "AP") && strcmp(region, "TW")
				&& strcmp(region, "RU")
				&& strcmp(region, "KR")
				&& strcmp(region, "CH"))) {
			{
				char *mode_ex = getUEnv("DEF-p_wireless_eth1_11a-authmode_ex");
				if (mode_ex && !strcmp(mode_ex, "mixed-psk")) {
					char *mode = getUEnv("DEF-p_wireless_eth1_11a-authmode");
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
				if (crypto) {
					nvram_set("wl0_crypto", crypto);
					nvram_set("wl_crypto", crypto);
				}
				char *wpapsk = getUEnv("DEF-p_wireless_eth1_11a-wpapsk");
				if (wpapsk) {
					nvram_set("wl_wpa_psk", wpapsk);
					nvram_set("wl0_wpa_psk", wpapsk);
				}else{
					nvram_set("wl_wpa_psk", "12345678");
					nvram_set("wl0_wpa_psk", "12345678");				
				}
			}
			{
				char *mode_ex = getUEnv("DEF-p_wireless_eth2_11bg-authmode_ex");
				if (mode_ex && !strcmp(mode_ex, "mixed-psk")) {
					char *mode = getUEnv("DEF-p_wireless_eth2_11bg-authmode");
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
					if (mode) {
						nvram_set("wl1_akm", mode);
						nvram_set("wl1_security_mode", mode);
					} else {
						nvram_set("wl1_akm", "disabled");
						nvram_set("wl1_security_mode", "disabled");
					}
				}

				char *crypto = getUEnv("DEF-p_wireless_eth2_11bg-crypto");
				if (crypto)
					nvram_set("wl1_crypto", crypto);
				char *wpapsk = getUEnv("DEF-p_wireless_eth2_11bg-wpapsk");
				if (wpapsk)
					nvram_set("wl1_wpa_psk", wpapsk);
				else
					nvram_set("wl1_wpa_psk", "12345678");
			}
		}
		struct ifreq ifr;
		int s;

		if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW))) {
			char eabuf[32];

			strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
			ioctl(s, SIOCGIFHWADDR, &ifr);
			close(s);
			unsigned char *edata = (unsigned char *)ifr.ifr_hwaddr.sa_data;
			sprintf(eabuf, "Buffalo-A-%02X%02X", edata[4] & 0xff, edata[5] & 0xff);
			nvram_set("wl_ssid", eabuf);
			nvram_set("wl0_ssid", eabuf);
			sprintf(eabuf, "Buffalo-G-%02X%02X", edata[4] & 0xff, edata[5] & 0xff);
			nvram_set("wl1_ssid", eabuf);
		}

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
#endif
		if (!strcmp(region, "AP") || !strcmp(region, "CH")
		    || !strcmp(region, "KR")
		    || !strcmp(region, "TW")
		    || !strcmp(region, "RU"))
			nvram_set("wps_status", "0");
		else
			nvram_set("wps_status", "1");
		nvram_set("wl_country_code", region);
#ifdef HAVE_BCMMODERN
		nvram_set("wl0_country_code", "Q1");
		nvram_set("wl0_country_rev", "27");
		nvram_set("wl1_country_code", "Q1");
		nvram_set("wl1_country_rev", "27");
		if (!strcmp(region, "JP")) {
			nvram_set("wl0_country_code", "JP");
			nvram_set("wl0_country_rev", "45");
			nvram_set("wl1_country_code", "JP");
			nvram_set("wl1_country_rev", "45");
		}

		if (!strcmp(region, "US")) {
			nvram_set("wl0_country_code", "Q2");
			nvram_set("wl0_country_rev", "41");
			nvram_set("wl1_country_code", "Q1");
			nvram_set("wl1_country_rev", "61");
		}

		if (!strcmp(region, "EU")) {
			nvram_set("wl0_country_code", "EU");
			nvram_set("wl0_country_rev", "61");
			nvram_set("wl1_country_code", "EU");
			nvram_set("wl1_country_rev", "61");
		}

		if (!strcmp(region, "AP")) {
			nvram_set("wl0_country_code", "CN");
			nvram_set("wl0_country_rev", "34");
			nvram_set("wl1_country_code", "Q2");
			nvram_set("wl1_country_rev", "41");
		}

		if (!strcmp(region, "KR")) {
			nvram_set("wl0_country_code", "KR");
			nvram_set("wl0_country_rev", "55");
			nvram_set("wl1_country_code", "Q2");
			nvram_set("wl1_country_rev", "41");
		}

		if (!strcmp(region, "CH")) {
			nvram_set("wl0_country_code", "CH");
			nvram_set("wl0_country_rev", "34");
			nvram_set("wl1_country_code", "Q2");
			nvram_set("wl1_country_rev", "41");
		}

		if (!strcmp(region, "TW")) {
			nvram_set("wl0_country_code", "TW");
			nvram_set("wl0_country_rev", "34");
			nvram_set("wl1_country_code", "Q2");
			nvram_set("wl1_country_rev", "41");
		}

		if (!strcmp(region, "RU")) {
			nvram_set("wl0_country_code", "RU");
			nvram_set("wl0_country_rev", "37");
			nvram_set("wl1_country_code", "Q2");
			nvram_set("wl1_country_rev", "41");
		}
#else
		nvram_set("wl0_country_code", region);
		nvram_set("wl1_country_code", region);
#endif
		nvram_set("ias_startup", "3");
		nvram_unset("http_userpln");
		nvram_unset("http_pwdpln");
#ifdef HAVE_SPOTPASS
		system("startservice spotpass_defaults");
#endif
	}
}

#elif HAVE_MT7620
#define getUEnv(name) nvram_get(name)
static void buffalo_defaults(int force)
{
	if (nvram_get("ath0_akm") == NULL || force) {
		nvram_set("ath0_akm", "disabled");
		nvram_set("ath1_akm", "disabled");
	
		FILE *fp;
		char script[32] = "/tmp/fdefaults.sh";
		char config[32] = "/tmp/sysdefaults.txt";
		char partition[20] = "/dev/mtdblock6";
		char mountpoint[20] = "/tmp/sysdefaults";
		char conffile[16] = "mac.dat";
		
		fp = fopen(script, "w");
		if (fp) {
			fprintf(fp, "#!/bin/sh\n");
			fprintf(fp, "insmod lzma_compress\n");
			fprintf(fp, "insmod jffs2\n");
			fprintf(fp, "mkdir %s\n", mountpoint);
			fprintf(fp, "mount -t jffs2 %s %s\n", partition, mountpoint);
			fprintf(fp, "cat %s/%s > %s\n", mountpoint, conffile, config);
			fclose(fp);
		}
		
		chmod(script, 0755);
		system(script);

		fp = fopen(config, "r");
		if (fp) {
			char line[32];
			char list[2][30];
			int i;
				
			while (fgets(line, sizeof(line), fp) != NULL) {
				
				// make string sscanf ready	
				for(i = 0; i < sizeof(line); i++) {
					if(line[i] == '=') {
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
			
		struct ifreq ifr;
		int s;

		if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW))) {
			char eabuf[32];

			strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
			ioctl(s, SIOCGIFHWADDR, &ifr);
			close(s);
			unsigned char *edata = (unsigned char *)ifr.ifr_hwaddr.sa_data;
			sprintf(eabuf, "Buffalo-G-%02X%02X", edata[4] & 0xff, edata[5] & 0xff);
			nvram_set("wl0_ssid", eabuf);
			sprintf(eabuf, "Buffalo-A-%02X%02X", edata[4] & 0xff, edata[5] & 0xff);
			nvram_set("wl1_ssid", eabuf);

		}

		nvram_set("ias_startup", "3");
		nvram_unset("http_userpln");
		nvram_unset("http_pwdpln");
#ifdef HAVE_SPOTPASS
		system("startservice spotpass_defaults");
#endif
		nvram_commit();
	}
}

#else
extern void *getUEnv(char *name);
static void buffalo_defaults(int force)
{

	char *pincode = getUEnv("pincode");
	if (pincode && nvram_get("pincode") == NULL) {
		nvram_set("pincode", pincode);
	}
	if (nvram_get("ath0_akm") == NULL || force) {
		nvram_set("ath0_akm", "disabled");
		char *region = getUEnv("region");
		if (!region || (strcmp(region, "AP") && strcmp(region, "TW")
				&& strcmp(region, "RU")
				&& strcmp(region, "KR")
				&& strcmp(region, "CH"))) {
			{
				char *mode_ex = getUEnv("DEF-p_wireless_ath0_11bg-authmode_ex");
				if (!mode_ex)
					mode_ex = getUEnv("DEF-p_wireless_ath00_11bg-authmode_ex");
				if (mode_ex && !strcmp(mode_ex, "mixed-psk")) {
					char *mode = getUEnv("DEF-p_wireless_ath0_11bg-authmode");
					if (!mode)
						mode = getUEnv("DEF-p_wireless_ath00_11bg-authmode");
					if (!mode) {
						nvram_set("ath0_akm", "disabled");
						nvram_set("ath0_security_mode", "disabled");
					} else {
						if (!strcmp(mode, "psk")) {
							nvram_set("ath0_akm", "psk psk2");
							nvram_set("ath0_security_mode", "psk psk2");
						}
						if (!strcmp(mode, "psk2")) {
							nvram_set("ath0_akm", "psk psk2");
							nvram_set("ath0_security_mode", "psk psk2");
						}
					}
				} else {
					char *mode = getUEnv("DEF-p_wireless_ath0_11bg-authmode");
					if (!mode)
						mode = getUEnv("DEF-p_wireless_ath00_11bg-authmode");
					if (mode) {
						nvram_set("ath0_akm", mode);
						nvram_set("ath0_security_mode", mode);
					} else {
						nvram_set("ath0_akm", "disabled");
						nvram_set("ath0_security_mode", "disabled");
					}
				}

				char *crypto = getUEnv("DEF-p_wireless_ath0_11bg-crypto");
				if (!crypto)
					crypto = getUEnv("DEF-p_wireless_ath00_11bg-crypto");
				if (crypto)
					nvram_set("ath0_crypto", crypto);
				char *wpapsk = getUEnv("DEF-p_wireless_ath0_11bg-wpapsk");
				if (!wpapsk)
					wpapsk = getUEnv("DEF-p_wireless_ath00_11bg-wpapsk");
				if (wpapsk)
					nvram_set("ath0_wpa_psk", wpapsk);
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
						nvram_set("ath1_akm", "disabled");
						nvram_set("ath1_security_mode", "disabled");
					} else {
						if (!strcmp(mode, "psk")) {
							nvram_set("ath1_akm", "psk psk2");
							nvram_set("ath1_security_mode", "psk psk2");
						}
						if (!strcmp(mode, "psk2")) {
							nvram_set("ath1_akm", "psk psk2");
							nvram_set("ath1_security_mode", "psk psk2");
						}
					}
				} else {
					char *mode = getUEnv("DEF-p_wireless_ath1_11a-authmode");
					if (!mode)
						mode = getUEnv("DEF-p_wireless_ath10_11a-authmode");
					if (mode) {
						nvram_set("ath1_akm", mode);
						nvram_set("ath1_security_mode", mode);
					} else {
						nvram_set("ath1_akm", "disabled");
						nvram_set("ath1_security_mode", "disabled");
					}
				}

				char *crypto = getUEnv("DEF-p_wireless_ath1_11a-crypto");
				if (!crypto)
					crypto = getUEnv("DEF-p_wireless_ath10_11a-crypto");
				if (crypto)
					nvram_set("ath1_crypto", crypto);
				char *wpapsk = getUEnv("DEF-p_wireless_ath1_11a-wpapsk");
				if (!wpapsk)
					wpapsk = getUEnv("DEF-p_wireless_ath10_11a-wpapsk");
				if (wpapsk)
					nvram_set("ath1_wpa_psk", wpapsk);
			}
		}
		struct ifreq ifr;
		int s;

		if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW))) {
			char eabuf[32];

			strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
			ioctl(s, SIOCGIFHWADDR, &ifr);
			close(s);
			unsigned char *edata = (unsigned char *)ifr.ifr_hwaddr.sa_data;
			sprintf(eabuf, "BUFFALO-%02X%02X%02X_G", edata[3] & 0xff, edata[4] & 0xff, edata[5] & 0xff);
			nvram_set("ath0_ssid", eabuf);
			sprintf(eabuf, "BUFFALO-%02X%02X%02X_A", edata[3] & 0xff, edata[4] & 0xff, edata[5] & 0xff);
			nvram_set("ath1_ssid", eabuf);

		}
#else
		}
		struct ifreq ifr;
		int s;

		if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW))) {
			char eabuf[32];

			strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
			ioctl(s, SIOCGIFHWADDR, &ifr);
			close(s);
			unsigned char *edata = (unsigned char *)ifr.ifr_hwaddr.sa_data;
#if defined(HAVE_WZR300HP) || defined(HAVE_WHR300HP)
			sprintf(eabuf, "BUFFALO-%02X%02X%02X", edata[3] & 0xff, edata[4] & 0xff, edata[5] & 0xff);
#else
			sprintf(eabuf, "%02X%02X%02X%02X%02X%02X", edata[0] & 0xff, edata[1] & 0xff, edata[2] & 0xff, edata[3] & 0xff, edata[4] & 0xff, edata[5] & 0xff);
#endif
			nvram_set("ath0_ssid", eabuf);
		}
#endif

		region = getUEnv("region");
		if (region == NULL) {
			region = "US";
		}
		if (!strcmp(region, "US")) {
			nvram_set("ath0_regdomain", "UNITED_STATES");
		} else if (!strcmp(region, "EU")) {
			nvram_set("ath0_regdomain", "GERMANY");
		} else if (!strcmp(region, "JP")) {
			nvram_set("ath0_regdomain", "JAPAN");
#ifdef HAVE_BUFFALO_SA
		} else if (!strcmp(region, "AP")) {
			nvram_set("ath0_regdomain", "SINGAPORE");
#else
		} else if (!strcmp(region, "AP")) {
			nvram_set("ath0_regdomain", "SINGAPORE");
#endif
		} else if (!strcmp(region, "RU")) {
			nvram_set("ath0_regdomain", "RUSSIA");
		} else if (!strcmp(region, "TW")) {
			nvram_set("ath0_regdomain", "TAIWAN");
		} else if (!strcmp(region, "CH")) {
			nvram_set("ath0_regdomain", "CHINA");
		} else if (!strcmp(region, "KR")) {
			nvram_set("ath0_regdomain", "KOREA_REPUBLIC");
		}
#ifdef HAVE_WZRHPAG300NH
		if (!strcmp(region, "US")) {
			nvram_set("ath1_regdomain", "UNITED_STATES");
		} else if (!strcmp(region, "EU")) {
			nvram_set("ath1_regdomain", "GERMANY");
		} else if (!strcmp(region, "JP")) {
			nvram_set("ath1_regdomain", "JAPAN");
		} else if (!strcmp(region, "RU")) {
			nvram_set("ath1_regdomain", "RUSSIA");
#ifdef HAVE_BUFFALO_SA
		} else if (!strcmp(region, "AP")) {
			nvram_set("ath1_regdomain", "SINGAPORE");
#else
		} else if (!strcmp(region, "AP")) {
			nvram_set("ath1_regdomain", "SINGAPORE");
#endif
		} else if (!strcmp(region, "TW")) {
			nvram_set("ath1_regdomain", "TAIWAN");
		} else if (!strcmp(region, "CH")) {
			nvram_set("ath1_regdomain", "CHINA");
		} else if (!strcmp(region, "KR")) {
			nvram_set("ath1_regdomain", "KOREA_REPUBLIC");
		}
#endif
		if (!strcmp(region, "AP") || !strcmp(region, "CH")
		    || !strcmp(region, "KR")
		    || !strcmp(region, "TW")
		    || !strcmp(region, "RU"))
			nvram_set("wps_status", "0");
		else
			nvram_set("wps_status", "1");
		nvram_set("ias_startup", "3");
		nvram_unset("http_userpln");
		nvram_unset("http_pwdpln");
#ifdef HAVE_SPOTPASS
		system("startservice spotpass_defaults");
#endif
		nvram_commit();
	}
}
#endif

#endif
/*
 * SeG dd-wrt addition for module startup scripts 
 */
void start_modules(void)
{
	runStartup("/etc/config", ".startup");

#ifdef HAVE_REGISTER
	if (isregistered_real()) {
#endif
#ifdef HAVE_RB500
		runStartup("/usr/local/etc/config", ".startup");	// if available
#elif HAVE_X86
		runStartup("/usr/local/etc/config", ".startup");	// if available
#else
		runStartup("/jffs/etc/config", ".startup");	// if available
		runStartup("/mmc/etc/config", ".startup");	// if available
		runStartup("/sd/etc/config", ".startup");	// if available
#endif
#ifdef HAVE_REGISTER
	}
#endif
	return;
}

void start_wanup(void)
{
	runStartup("/etc/config", ".wanup");
#ifdef HAVE_REGISTER
	if (isregistered_real())
#endif
	{

#ifdef HAVE_RB500
		runStartup("/usr/local/etc/config", ".wanup");	// if available
#elif HAVE_X86
		runStartup("/usr/local/etc/config", ".wanup");	// if available
#else
		runStartup("/jffs/etc/config", ".wanup");	// if available
		runStartup("/mmc/etc/config", ".wanup");	// if available
		runStartup("/tmp/etc/config", ".wanup");	// if available
		runStartup("/sd/etc/config", ".wanup");	// if available
#endif
	}
	return;
}

void start_run_rc_startup(void)
{
	DIR *directory;
	int count = 36;		// 36 * 5 s = 180s

	create_rc_file(RC_STARTUP);

	if (f_exists("/tmp/.rc_startup"))
		system("/tmp/.rc_startup");

	while (count > 0) {
		directory = opendir("/opt/etc/init.d");
		if (directory == NULL) {
			sleep(5);
			count--;
		} else {
			closedir(directory);
			runStartup("/opt/etc/init.d", "S**");	// if available; run S** startup scripts
			return;
		}
	}
}

void start_run_rc_shutdown(void)
{
	runStartup("/opt/etc/init.d", "K**");	// if available; run K** shutdown scripts
	create_rc_file(RC_SHUTDOWN);
	if (f_exists("/tmp/.rc_shutdown"))
		system("/tmp/.rc_shutdown");
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
	nvram_set("ses_event", "2");

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
	if (nvram_invmatch("sv_restore_defaults", "0")
	    && (!strcmp(getUEnv("region"), "AP")
		|| !strcmp(getUEnv("region"), "US")))
		    nvram_set("region", "SA");
#endif
#ifdef HAVE_RB500
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames",
		 "eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 ath0 ath1 ath2 ath3 ath4 ath5",
		 0},
		{"wan_ifname", "eth0", 0},
		{"wan_ifnames", "eth0", 0},
		{0, 0, 0}
	};
#elif HAVE_GEMTEK
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth1 ath0", 0},
		{"wan_ifname", "eth0", 0},
		{"wan_ifnames", "eth0", 0},
		{0, 0, 0}
	};
#elif HAVE_EAP9550
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth2 ra0",
		 0},
		{"wan_ifname2", "eth2", 0},
		{"wan_ifname", "eth2", 0},
		{"wan_default", "eth2", 0},
		{"wan_ifnames", "eth2", 0},
		{0, 0, 0}
	};
#elif HAVE_HAMEA15
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "vlan1 ra0",
		 0},
		{"wan_ifname2", "vlan1", 0},
		{"wan_ifname", "vlan1", 0},
		{"wan_default", "vlan1", 0},
		{"wan_ifnames", "vlan1", 0},
		{0, 0, 0}
	};
#elif HAVE_RT2880
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "vlan1 vlan2 ra0 ba0",
		 0},
		{"wan_ifname2", "vlan2", 0},
		{"wan_ifname", "vlan2", 0},
		{"wan_default", "vlan2", 0},
		{"wan_ifnames", "vlan2", 0},
		{0, 0, 0}
	};
#elif HAVE_GATEWORX
#if defined(HAVE_XIOCOM) || defined(HAVE_MI424WR)
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "ixp1 ath0 ath1 ath2 ath3",
		 0},
		{"wan_ifname2", "ixp0", 0},
		{"wan_ifname", "ixp0", 0},
		{"wan_default", "ixp0", 0},
		{"wan_ifnames", "ixp0", 0},
		{0, 0, 0}
	};
#else
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "ixp0 ath0 ath1 ath2 ath3",
		 0},
		{"wan_ifname2", "ixp1", 0},
		{"wan_ifname", "ixp1", 0},
		{"wan_default", "ixp1", 0},
		{"wan_ifnames", "ixp1", 0},
		{0, 0, 0}
	};
#endif
#elif HAVE_X86
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
#ifdef HAVE_NOWIFI
		{"lan_ifnames",
		 "eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 eth9 eth10",
		 0},
#else
#ifdef HAVE_GW700
		{"lan_ifnames",
		 "eth0 eth2 eth3 eth4 eth5 eth6 eth7 eth8 eth9 eth10 ath0 ath1 ath2 ath3 ath5 ath6 ath7 ath8",
		 0},
#else
		{"lan_ifnames",
		 "eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 eth9 eth10 ath0 ath1 ath2 ath3 ath5 ath6 ath7 ath8",
		 0},
#endif
#endif
#ifdef HAVE_GW700
		{"wan_ifname", "eth1", 0},
		{"wan_ifname2", "eth1", 0},
		{"wan_ifnames", "eth1", 0},
#else
		{"wan_ifname", "eth0", 0},
		{"wan_ifname2", "eth0", 0},
		{"wan_ifnames", "eth0", 0},
#endif
		{0, 0, 0}
	};
#elif HAVE_XSCALE
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames",
		 "ixp0.1 ixp0.2 ath0 ath1",
		 0},
		{"wan_ifname", "ixp1", 0},
		{"wan_ifname2", "ixp1", 0},
		{"wan_ifnames", "ixp1", 0},
		{"wan_default", "ixp1", 0},
		{0, 0, 0}
	};
#elif HAVE_LAGUNA
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames",
		 "eth0 eth1 ath0 ath1 ath2 ath3",
		 0},
		{"wan_ifname", "eth0", 0},
		{"wan_ifname2", "eth0", 0},
		{"wan_ifnames", "eth0", 0},
		{"wan_default", "eth0", 0},
		{0, 0, 0}
	};
#elif HAVE_VENTANA
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames",
		 "eth0 eth1 ath0 ath1 ath2 ath3",
		 0},
		{"wan_ifname", "eth0", 0},
		{"wan_ifname2", "eth0", 0},
		{"wan_ifnames", "eth0", 0},
		{"wan_default", "eth0", 0},
		{0, 0, 0}
	};
#elif HAVE_NORTHSTAR
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames",
		 "vlan1 vlan2 eth1 eth2",
		 0},
		{"wan_ifname", "vlan2", 0},
		{"wan_ifname2", "vlan2", 0},
		{"wan_ifnames", "vlan2", 0},
		{"wan_default", "vlan2", 0},
		{0, 0, 0}
	};
#elif HAVE_MAGICBOX
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth1 ath0",
		 0},
		{"wan_ifname", "eth0", 0},
		{"wan_ifname2", "eth0", 0},
		{"wan_ifnames", "eth0", 0},
		{"wan_default", "eth0", 0},
		{0, 0, 0}
	};
#elif HAVE_UNIWIP
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames",
		 "eth0 ath0 ath1",
		 0},
		{"wan_ifname", "eth0", 0},
		{"wan_ifname2", "eth0", 0},
		{"wan_ifnames", "eth0", 0},
		{"wan_default", "eth0", 0},
		{0, 0, 0}
	};
#elif HAVE_WDR4900
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames",
		 "vlan1 vlan2 ath0 ath1",
		 0},
		{"wan_ifname", "vlan2", 0},
		{"wan_ifname2", "vlan2", 0},
		{"wan_ifnames", "vlan2", 0},
		{"wan_default", "vlan2", 0},
		{0, 0, 0}
	};
#elif HAVE_RB600
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames",
		 "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 ath0 ath1 ath2 ath3 ath4 ath5 ath6 ath7",
		 0},
		{"wan_ifname", "eth0", 0},
		{"wan_ifname2", "eth0", 0},
		{"wan_ifnames", "eth0", 0},
		{"wan_default", "eth0", 0},
		{0, 0, 0}
	};
#elif HAVE_FONERA
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "vlan0 ath0", 0},
		{"wan_ifname", "", 0},
		{"wan_ifname2", "", 0},
		{"wan_default", "", 0},
		{"wan_ifnames", "eth0 vlan1", 0},
		{0, 0, 0}
	};
#elif HAVE_BWRG1000
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "vlan0 vlan2 ath0", 0},
		{"wan_ifname", "vlan2", 0},
		{"wan_ifname2", "vlan2", 0},
		{"wan_ifnames", "vlan2", 0},
		{"wan_default", "vlan2", 0},
		{0, 0, 0}
	};
#elif HAVE_SOLO51
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "vlan0 vlan2 ath0", 0},
		{"wan_ifname", "vlan0", 0},
		{"wan_ifname2", "vlan0", 0},
		{"wan_ifnames", "vlan0", 0},
		{"wan_default", "vlan0", 0},
		{0, 0, 0}
	};
#elif HAVE_BS2
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 ath0 ath1", 0},
		{"wan_ifname", "eth0", 0},
		{"wan_ifname2", "eth0", 0},
		{"wan_ifnames", "eth0", 0},
		{"wan_default", "eth0", 0},
		{0, 0, 0}
	};
#elif HAVE_NS2
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 ath0 ath1", 0},
		{"wan_ifname", "eth0", 0},
		{"wan_ifname2", "eth0", 0},
		{"wan_ifnames", "eth0", 0},
		{"wan_default", "eth0", 0},
		{0, 0, 0}
	};
#elif HAVE_LC2
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 ath0 ath1", 0},
		{"wan_ifname", "eth0", 0},
		{"wan_ifname2", "eth0", 0},
		{"wan_ifnames", "eth0", 0},
		{"wan_default", "eth0", 0},
		{0, 0, 0}
	};
#elif HAVE_PICO2
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 ath0 ath1", 0},
		{"wan_ifname", "eth0", 0},
		{"wan_ifname2", "eth0", 0},
		{"wan_ifnames", "eth0", 0},
		{"wan_default", "eth0", 0},
		{0, 0, 0}
	};
#elif HAVE_PICO2HP
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 ath0 ath1", 0},
		{"wan_ifname", "eth0", 0},
		{"wan_ifname2", "eth0", 0},
		{"wan_ifnames", "eth0", 0},
		{"wan_default", "eth0", 0},
		{0, 0, 0}
	};
#elif HAVE_MS2
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 ath0 ath1", 0},
		{"wan_ifname", "eth0", 0},
		{"wan_ifname2", "eth0", 0},
		{"wan_ifnames", "eth0", 0},
		{"wan_default", "eth0", 0},
		{0, 0, 0}
	};
#elif HAVE_BS2HP
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 ath0 ath1", 0},
		{"wan_ifname", "eth0", 0},
		{"wan_ifname2", "eth0", 0},
		{"wan_ifnames", "eth0", 0},
		{"wan_default", "eth0", 0},
		{0, 0, 0}
	};
#elif HAVE_LS2
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "vlan0 vlan2 ath0", 0},
		{"wan_ifname", "vlan0", 0},
		{"wan_ifname2", "vlan0", 0},
		{"wan_ifnames", "vlan0", 0},
		{"wan_default", "vlan0", 0},
		{0, 0, 0}
	};
#elif HAVE_RS
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 eth1 ath0 ath1 ath2", 0},
		{"wan_ifname", "eth0", 0},
		{"wan_ifname2", "eth0", 0},
		{"wan_ifnames", "eth0", 0},
		{"wan_default", "eth0", 0},
		{0, 0, 0}
	};
#elif HAVE_WA901
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 ath0", 0},
		{"wan_ifname", "eth0", 0},
		{"wan_ifname2", "eth0", 0},
		{"wan_ifnames", "eth0", 0},
		{"wan_default", "eth0", 0},
		{0, 0, 0}
	};
#elif HAVE_WR941
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "vlan0 vlan1 ath0", 0},
		{"wan_ifname", "vlan1", 0},
		{"wan_ifname2", "vlan1", 0},
		{"wan_ifnames", "vlan1", 0},
		{"wan_default", "vlan1", 0},
		{0, 0, 0}
	};
#elif HAVE_WA901v1
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth1 ath0", 0},
		{"wan_ifname", "eth1", 0},
		{"wan_ifname2", "eth1", 0},
		{"wan_ifnames", "eth1", 0},
		{"wan_default", "eth1", 0},
		{0, 0, 0}
	};
#elif HAVE_CARAMBOLA
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "vlan1 vlan2 ath0", 0},
		{"wan_ifname", "vlan2", 0},
		{"wan_ifname2", "vlan2", 0},
		{"wan_ifnames", "vlan2", 0},
		{"wan_default", "vlan2", 0},
		{0, 0, 0}
	};
#elif HAVE_WR703
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth1 ath0", 0},
		{"wan_ifname", "eth1", 0},
		{"wan_ifname2", "eth1", 0},
		{"wan_ifnames", "eth1", 0},
		{"wan_default", "eth1", 0},
		{0, 0, 0}
	};
#elif HAVE_WDR2543
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "vlan1 vlan2 ath0", 0},
		{"wan_ifname", "vlan2", 0},
		{"wan_ifname2", "vlan2", 0},
		{"wan_ifnames", "vlan2", 0},
		{"wan_default", "vlan2", 0},
		{0, 0, 0}
	};
#elif HAVE_WA7510
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth1 ath0", 0},
		{"wan_ifname", "eth1", 0},
		{"wan_ifname2", "eth1", 0},
		{"wan_ifnames", "eth1", 0},
		{"wan_default", "eth1", 0},
		{0, 0, 0}
	};
#elif HAVE_WR741
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 eth1 ath0", 0},
		{"wan_ifname", "eth0", 0},
		{"wan_ifname2", "eth0", 0},
		{"wan_ifnames", "eth0", 0},
		{"wan_default", "eth0", 0},
		{0, 0, 0}
	};
#elif HAVE_WR1043
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "vlan1 vlan2 ath0", 0},
		{"wan_ifname", "vlan2", 0},
		{"wan_ifname2", "vlan2", 0},
		{"wan_ifnames", "vlan2", 0},
		{"wan_default", "vlan2", 0},
		{0, 0, 0}
	};
#elif HAVE_AP83
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 eth1 ath0", 0},
		{"wan_ifname", "eth1", 0},
		{"wan_ifname2", "eth1", 0},
		{"wan_ifnames", "eth1", 0},
		{"wan_default", "eth1", 0},
		{0, 0, 0}
	};
#elif HAVE_AP94
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 eth1 ath0", 0},
		{"wan_ifname", "eth1", 0},
		{"wan_ifname2", "eth1", 0},
		{"wan_ifnames", "eth1", 0},
		{"wan_default", "eth1", 0},
		{0, 0, 0}
	};
#elif HAVE_UBNTM
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 eth1 ath0", 0},
		{"wan_ifname", "eth0", 0},
		{"wan_ifname2", "eth0", 0},
		{"wan_ifnames", "eth0", 0},
		{"wan_default", "eth0", 0},
		{0, 0, 0}
	};
#elif HAVE_HORNET
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 eth1 ath0", 0},
#ifdef HAVE_MAKSAT
		{"wan_ifname", "eth0", 0},
		{"wan_ifname2", "eth0", 0},
		{"wan_ifnames", "eth0", 0},
		{"wan_default", "eth0", 0},
#elif HAVE_ONNET
		{"wan_ifname", "eth0", 0},
		{"wan_ifname2", "eth0", 0},
		{"wan_ifnames", "eth0", 0},
		{"wan_default", "eth0", 0},
#else
		{"wan_ifname", "eth1", 0},
		{"wan_ifname2", "eth1", 0},
		{"wan_ifnames", "eth1", 0},
		{"wan_default", "eth1", 0},
#endif
		{0, 0, 0}
	};
#elif HAVE_WNR2000
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 eth1 ath0", 0},
		{"wan_ifname", "", 0},
		{"wan_ifname2", "", 0},
		{"wan_ifnames", "", 0},
		{"wan_default", "", 0},
		{0, 0, 0}
	};
#elif HAVE_WASP
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "vlan1 vlan2 ath0 ath1", 0},
		{"wan_ifname", "", 0},
		{"wan_ifname2", "", 0},
		{"wan_ifnames", "", 0},
		{"wan_default", "", 0},
		{0, 0, 0}
	};
#elif HAVE_WHRHPGN
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 eth1 ath0", 0},
		{"wan_ifname", "", 0},
		{"wan_ifname2", "", 0},
		{"wan_ifnames", "", 0},
		{"wan_default", "", 0},
		{0, 0, 0}
	};
#elif HAVE_DIR615E
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 eth1 ath0", 0},
		{"wan_ifname", "", 0},
		{"wan_ifname2", "", 0},
		{"wan_ifnames", "", 0},
		{"wan_default", "", 0},
		{0, 0, 0}
	};
#elif HAVE_JA76PF
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 eth1 ath0", 0},
#ifdef HAVE_SANSFIL
		{"wan_ifname", "eth0", 0},
		{"wan_ifname2", "eth0", 0},
		{"wan_ifnames", "eth0", 0},
		{"wan_default", "eth0", 0},
#else
		{"wan_ifname", "eth1", 0},
		{"wan_ifname2", "eth1", 0},
		{"wan_ifnames", "eth1", 0},
		{"wan_default", "eth1", 0},
#endif
		{0, 0, 0}
	};
#elif HAVE_ALFAAP94
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 eth1 ath0 ath1 ath2", 0},
		{"wan_ifname", "eth1", 0},
		{"wan_ifname2", "eth1", 0},
		{"wan_ifnames", "eth1", 0},
		{"wan_default", "eth1", 0},
		{0, 0, 0}
	};
#elif HAVE_WZRG450
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "vlan1 ath0", 0},
		{"wan_ifname", "vlan2", 0},
		{"wan_ifname2", "vlan2", 0},
		{"wan_ifnames", "vlan2", 0},
		{"wan_default", "vlan2", 0},
		{0, 0, 0}
	};
#elif HAVE_WZRHPAG300NH
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 eth1 ath0 ath1", 0},
		{"wan_ifname", "eth1", 0},
		{"wan_ifname2", "eth1", 0},
		{"wan_ifnames", "eth1", 0},
		{"wan_default", "eth1", 0},
		{0, 0, 0}
	};
#elif HAVE_JWAP003
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 eth1 ath0", 0},
		{"wan_ifname", "eth1", 0},
		{"wan_ifname2", "eth1", 0},
		{"wan_ifnames", "eth1", 0},
		{"wan_default", "eth1", 0},
		{0, 0, 0}
	};
#elif HAVE_JJAP93
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 ath0", 0},
		{"wan_ifname", "", 0},
		{"wan_ifname2", "", 0},
		{"wan_ifnames", "", 0},
		{"wan_default", "", 0},
		{0, 0, 0}
	};
#elif HAVE_JJAP005
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 eth1 ath0", 0},
		{"wan_ifname", "eth1", 0},
		{"wan_ifname2", "eth1", 0},
		{"wan_ifnames", "eth1", 0},
		{"wan_default", "eth1", 0},
		{0, 0, 0}
	};
#elif HAVE_JJAP501
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 eth1 ath0", 0},
		{"wan_ifname", "eth1", 0},
		{"wan_ifname2", "eth1", 0},
		{"wan_ifnames", "eth1", 0},
		{"wan_default", "eth1", 0},
		{0, 0, 0}
	};
#elif HAVE_AC722
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 eth1 ath0", 0},
		{"wan_ifname", "eth1", 0},
		{"wan_ifname2", "eth1", 0},
		{"wan_ifnames", "eth1", 0},
		{"wan_default", "eth1", 0},
		{0, 0, 0}
	};
#elif HAVE_AC622
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 eth1 ath0", 0},
		{"wan_ifname", "eth1", 0},
		{"wan_ifname2", "eth1", 0},
		{"wan_ifnames", "eth1", 0},
		{"wan_default", "eth1", 0},
		{0, 0, 0}
	};
#elif HAVE_WP546
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 eth1 ath0 ath1", 0},
		{"wan_ifname", "eth1", 0},
		{"wan_ifname2", "eth1", 0},
		{"wan_ifnames", "eth1", 0},
		{"wan_default", "eth1", 0},
		{0, 0, 0}
	};
#elif HAVE_LSX
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 ath0", 0},
		{"wan_ifname", "", 0},
		{"wan_ifname2", "", 0},
		{"wan_ifnames", "", 0},
		{"wan_default", "", 0},
		{0, 0, 0}
	};
#elif HAVE_DANUBE
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 ath0", 0},
		{"wan_ifname", "nas0", 0},
		{"wan_ifname2", "nas0", 0},
		{"wan_ifnames", "nas0", 0},
		{"wan_default", "nas0", 0},
		{0, 0, 0}
	};
#elif HAVE_WBD222
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 eth1 eth2 ath0 ath1 ath2", 0},
		{"wan_ifname", "eth0", 0},
		{"wan_ifname2", "eth0", 0},
		{"wan_ifnames", "eth0", 0},
		{"wan_default", "eth0", 0},
		{0, 0, 0}
	};
#elif HAVE_STORM
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 ath0", 0},
		{"wan_ifname", "", 0},
		{"wan_ifname2", "", 0},
		{"wan_ifnames", "", 0},
		{"wan_default", "", 0},
		{0, 0, 0}
	};
#elif HAVE_OPENRISC
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 eth1 eth2 eth3 ath0", 0},
		{"wan_ifname", "", 0},
		{"wan_ifname2", "", 0},
		{"wan_ifnames", "", 0},
		{"wan_default", "", 0},
		{0, 0, 0}
	};
#elif HAVE_WP54G
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 ath0", 0},
		{"wan_ifname", "eth1", 0},
		{"wan_ifname2", "eth1", 0},
		{"wan_ifnames", "eth1", 0},
		{"wan_default", "eth1", 0},
		{0, 0, 0}
	};
#elif HAVE_NP28G
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 ath0", 0},
		{"wan_ifname", "eth1", 0},
		{"wan_ifname2", "eth1", 0},
		{"wan_ifnames", "eth1", 0},
		{"wan_default", "eth1", 0},
		{0, 0, 0}
	};
#elif HAVE_ADM5120
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 ath0", 0},
		{"wan_ifname", "", 0},
		{"wan_ifname2", "", 0},
		{"wan_ifnames", "", 0},
		{"wan_default", "", 0},
		{0, 0, 0}
	};
#elif HAVE_LS5
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "ath0", 0},
		{"wan_ifname", "eth0", 0},
		{"wan_ifname2", "eth0", 0},
		{"wan_ifnames", "eth0", 0},
		{"wan_default", "eth0", 0},
		{0, 0, 0}
	};
#elif HAVE_WHRAG108
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 ath0 ath1", 0},
		{"wan_ifname2", "eth1", 0},
		{"wan_ifname", "eth1", 0},
		{"wan_ifnames", "eth1", 0},
		{"wan_default", "eth1", 0},
		{0, 0, 0}
	};
#elif HAVE_PB42
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth1 ath0 ath1", 0},
		{"wan_ifname2", "eth0", 0},
		{"wan_ifname", "eth0", 0},
		{"wan_ifnames", "eth0", 0},
		{"wan_default", "eth0", 0},
		{0, 0, 0}
	};
#elif HAVE_TW6600
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "ath0 ath1", 0},
		{"wan_ifname2", "eth0", 0},
		{"wan_ifname", "eth0", 0},
		{"wan_ifnames", "eth0", 0},
		{"wan_default", "eth0", 0},
		{0, 0, 0}
	};
#elif HAVE_CA8PRO
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "vlan0 ath0", 0},
		{"wan_ifname", "vlan1", 0},
		{"wan_ifname2", "vlan1", 0},
		{"wan_ifnames", "vlan1", 0},
		{"wan_default", "vlan1", 0},
		{0, 0, 0}
	};
#elif HAVE_CA8
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "ath0", 0},
		{"wan_ifname", "eth0", 0},
		{"wan_ifname2", "eth0", 0},
		{"wan_ifnames", "eth0", 0},
		{"wan_default", "eth0", 0},
		{0, 0, 0}
	};
#else
	struct nvram_tuple generic[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 eth2 eth3 eth4", 0},
		{"wan_ifname", "eth1", 0},
		{"wan_ifname2", "eth1", 0},
		{"wan_ifnames", "eth1", 0},
		{"wan_default", "eth1", 0},
		{0, 0, 0}
	};
	struct nvram_tuple vlan[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "vlan0 eth1 eth2 eth3", 0},
		{"wan_ifname", "vlan1", 0},
		{"wan_ifname2", "vlan1", 0},
		{"wan_ifnames", "vlan1", 0},
		{"wan_default", "vlan1", 0},
		{0, 0, 0}
	};

	struct nvram_tuple wrt350vlan[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "vlan1 eth0", 0},
		{"wan_ifname", "vlan2", 0},
		{"wan_ifname2", "vlan2", 0},
		{"wan_ifnames", "vlan2", 0},
		{"wan_default", "vlan2", 0},
		{0, 0, 0}
	};

	struct nvram_tuple wnr3500vlan[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "vlan1 eth1", 0},
		{"wan_ifname", "vlan2", 0},
		{"wan_ifname2", "vlan2", 0},
		{"wan_ifnames", "vlan2", 0},
		{"wan_default", "vlan2", 0},
		{0, 0, 0}
	};
	
	struct nvram_tuple wrt320vlan[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "vlan1 eth1", 0},
		{"wan_ifname", "vlan2", 0},
		{"wan_ifname2", "vlan2", 0},
		{"wan_ifnames", "vlan2", 0},
		{"wan_default", "vlan2", 0},
		{0, 0, 0}
	};

	struct nvram_tuple wrt30011vlan[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "vlan0 eth0", 0},
		{"wan_ifname", "vlan1", 0},
		{"wan_ifname2", "vlan1", 0},
		{"wan_ifnames", "vlan1", 0},
		{"wan_default", "vlan1", 0},
		{0, 0, 0}
	};

	struct nvram_tuple wrt600vlan[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "vlan0 eth0 eth1", 0},
		{"wan_ifname", "vlan2", 0},
		{"wan_ifname2", "vlan2", 0},
		{"wan_ifnames", "vlan2", 0},
		{"wan_default", "vlan2", 0},
		{0, 0, 0}
	};

	struct nvram_tuple wrt60011vlan[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "vlan1 eth0 eth1", 0},
		{"wan_ifname", "vlan2", 0},
		{"wan_ifname2", "vlan2", 0},
		{"wan_ifnames", "vlan2", 0},
		{"wan_default", "vlan2", 0},
		{0, 0, 0}
	};

	struct nvram_tuple wrt6102vlan[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "vlan1 eth1 eth2", 0},
		{"wan_ifname", "vlan2", 0},
		{"wan_ifname2", "vlan2", 0},
		{"wan_ifnames", "vlan2", 0},
		{"wan_default", "vlan2", 0},
		{0, 0, 0}
	};

	struct nvram_tuple rt53nvlan[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "vlan2 eth1 eth2", 0},
		{"wan_ifname", "vlan1", 0},
		{"wan_ifname2", "vlan1", 0},
		{"wan_ifnames", "vlan1", 0},
		{"wan_default", "vlan1", 0},
		{0, 0, 0}
	};

	struct nvram_tuple wzr144nhvlan[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "vlan2 eth0", 0},
		{"wan_ifname", "vlan1", 0},
		{"wan_ifname2", "vlan1", 0},
		{"wan_ifnames", "vlan1", 0},
		{"wan_default", "vlan1", 0},
		{0, 0, 0}
	};

	struct nvram_tuple generic_2[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth1 eth2", 0},
		{"wan_ifname", "eth0", 0},
		{"wan_ifname2", "eth0", 0},
		{"wan_ifnames", "eth0", 0},
		{"wan_default", "eth0", 0},
		{0, 0, 0}
	};

	struct nvram_tuple generic_3[] = {
		{"lan_ifname", "br0", 0},
		{"lan_ifnames", "eth0 eth1", 0},
		{"wan_ifname", "eth2", 0},
		{"wan_ifname2", "eth2", 0},
		{"wan_ifnames", "eth2", 0},
		{"wan_default", "eth2", 0},
		{0, 0, 0}
	};
#endif

	struct nvram_tuple *linux_overrides;
	struct nvram_tuple *t, *u;
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
	load_defaults();
#ifdef HAVE_RB500
	linux_overrides = generic;
	int brand = getRouterBrand();
#elif defined(HAVE_XSCALE) || defined(HAVE_X86) || defined(HAVE_MAGICBOX) || defined(HAVE_LAGUNA) || defined(HAVE_VENTANA) || defined(HAVE_NORTHSTAR) || defined(HAVE_RB600) \
    || defined(HAVE_GATEWORX) || defined(HAVE_FONERA) || defined(HAVE_SOLO51) || defined(HAVE_RT2880) || defined(HAVE_LS2) || defined(HAVE_LS5) \
    || defined(HAVE_WHRAG108) || defined(HAVE_TW6600) || defined(HAVE_PB42) || defined(HAVE_LSX) || defined(HAVE_DANUBE) || defined(HAVE_OPENRISC) \
    || defined(HAVE_STORM) || defined(HAVE_ADM5120) || defined(HAVE_CA8)
	linux_overrides = generic;
	int brand = getRouterBrand();

	if (nvram_invmatch("sv_restore_defaults", "0"))	// ||
		// nvram_invmatch("os_name", 
		// "linux"))
	{
		restore_defaults = 1;
	}

	if (nvram_match("product_name", "INSPECTION")) {
		nvram_unset("product_name");
		restore_defaults = 1;
	}
	if (nvram_get("router_name") == NULL) {
		restore_defaults = 1;
	}

#elif HAVE_GEMTEK
	linux_overrides = generic;
	int brand = getRouterBrand();
#else
	int brand = getRouterBrand();

	if (nvram_invmatch("sv_restore_defaults", "0"))	// ||
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
	if (nvram_get("router_name") == NULL) {
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
			fgets(line, sizeof(line), fmem);	//eat first line
			fgets(line, sizeof(line), fmem);
			if (sscanf(line, "%*s %lu", &msize) == 1) {
				if (msize > (8 * 1024 * 1024)) {
					nvram_set("ip_conntrack_max", "4096");
					nvram_set("ip_conntrack_tcp_timeouts", "3600");
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
		ds = nvram_safe_get("dhcp_start");
		if (ds != NULL && strlen(ds) > 3) {
			for (t = srouter_defaults; t->name; t++) {
				nvram_unset(t->name);
			}
			restore_defaults = 1;
		}

		/*
		 * ds = nvram_safe_get ("http_passwd"); if (ds == NULL || strlen
		 * (ds) == 0) //fix for empty default password { nvram_set
		 * ("http_passwd", "admin"); } ds = nvram_safe_get ("language");
		 * if (ds != NULL && strlen (ds) < 3) { nvram_set ("language",
		 * "english"); }
		 */
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
#if defined(HAVE_XSCALE) || defined(HAVE_X86) || defined(HAVE_MAGICBOX) || defined(HAVE_LAGUNA) || defined(HAVE_VENTANA) || defined(HAVE_NORTHSTAR) || defined(HAVE_RB600) \
    || defined(HAVE_GATEWORX) || defined(HAVE_FONERA) || defined(HAVE_SOLO51) || defined(HAVE_RT2880) || defined(HAVE_LS2) || defined(HAVE_LS5) \
    || defined(HAVE_WHRAG108) || defined(HAVE_TW6600) || defined(HAVE_PB42) || defined(HAVE_LSX) || defined(HAVE_DANUBE) || defined(HAVE_OPENRISC) \
    || defined(HAVE_STORM) || defined(HAVE_ADM5120) || defined(HAVE_CA8) || defined(HAVE_80211AC)
	if (restore_defaults) {
		nvram_clear();
	}
#endif

	int nvcnt = 0;

#ifndef HAVE_MADWIFI
	int icnt = get_wl_instances();
#else
	int icnt = getdevicecount();
	if (brand == ROUTER_LINKSYS_E2500 || brand == ROUTER_LINKSYS_E3200)	//dual radio, 2nd on usb-bus
		icnt = 2;
#endif
#if defined(HAVE_BUFFALO) || defined(HAVE_BUFFALO_BL_DEFAULTS)
	buffalo_defaults(restore_defaults);
#endif
	runStartup("/sd/etc/config", ".pf");
	// if (!nvram_match("default_init","1"))
	{
		for (t = srouter_defaults; t->name; t++) {
			if (restore_defaults || !nvram_get(t->name)) {
				for (u = linux_overrides; u && u->name; u++) {
					if (!strcmp(t->name, u->name)) {
						nvcnt++;
						nvram_set(u->name, u->value);
						break;
					}
				}
				if (!u || !u->name) {
					nvcnt++;
					nvram_set(t->name, t->value);
					if (icnt == 1 && startswith(t->name, "wl1_"))	//unset wl1_xx if we have single radio only
						nvram_unset(t->name);
				}
			}
		}
	}
	free_defaults();
	if (strlen(nvram_safe_get("http_username")) == 0 || nvram_match("http_username", "admin")) {
		nvram_set("http_username", zencrypt("root"));
		nvram_set("http_passwd", zencrypt("admin"));
	}
	if (restore_defaults) {
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
#ifdef HAVE_SPUTNIK
		nvram_set("lan_ipaddr", "192.168.180.1");
#elif HAVE_BUFFALO
		nvram_set("lan_ipaddr", "192.168.11.1");
#elif HAVE_KORENRON
		nvram_set("lan_ipaddr", "10.0.0.1");
#else
		nvram_set("lan_ipaddr", "192.168.1.1");
#endif
	}
#ifdef HAVE_SKYTRON
	if (restore_defaults) {
		nvram_set("lan_ipaddr", "192.168.0.1");
	}
#endif
	switch (brand) {
	case ROUTER_WRT600N:
		if (nvram_match("switch_type", "BCM5395") && nvram_match("vlan0ports", "1 2 3 4 8*"))	// fix for WRT600N
			// v1.1 (BCM5395 does 
			// not suppport vid
			// 0, so gemtek
			// internally
			// configured vid 1
			// as lan)
		{
			nvram_set("vlan0ports", " ");
			nvram_set("vlan1ports", "1 2 3 4 8*");
			nvram_set("vlan2ports", "0 8*");
			nvram_set("vlan0hwname", " ");
			nvram_set("vlan1hwname", "et0");
			nvram_set("landevs", "vlan1 wl0 wl1");
			nvram_set("lan_ifnames", "vlan1 eth0 eth1");
		} else {
			if (!nvram_get("vlan0ports")
			    || nvram_match("vlan0ports", "")
			    || !nvram_get("vlan2ports")
			    || nvram_match("vlan2ports", "")) {
				nvram_set("vlan0ports", "1 2 3 4 8*");
				nvram_set("vlan2ports", "0 8*");
			}
		}
		break;
	case ROUTER_WRT610NV2:
		if (!nvram_get("vlan1ports") || nvram_match("vlan1ports", "")
		    || !nvram_get("vlan2ports")
		    || nvram_match("vlan2ports", "")) {
			nvram_set("vlan1ports", "4 3 2 1 8*");
			nvram_set("vlan2ports", "0 8");
		}
		break;
	case ROUTER_LINKSYS_EA6500:
		if (!nvram_get("vlan1ports") || nvram_match("vlan1ports", "")
		    || !nvram_get("vlan2ports")
		    || nvram_match("vlan2ports", "")) {
			nvram_set("vlan1ports", "0 1 2 3 8*");
			nvram_set("vlan2ports", "4 8");
		}	
		break;
	case ROUTER_WRT610N:
	case ROUTER_WRT350N:
	case ROUTER_WRT310N:
	case ROUTER_D1800H:
	case ROUTER_ASUS_AC66U:
		if (!nvram_get("vlan1ports") || nvram_match("vlan1ports", "")
		    || !nvram_get("vlan2ports")
		    || nvram_match("vlan2ports", "")) {
			nvram_set("vlan1ports", "1 2 3 4 8*");
			nvram_set("vlan2ports", "0 8");
		}
		break;
	case ROUTER_WRT300NV11:
		if (!nvram_get("vlan0ports") || nvram_match("vlan0ports", "")
		    || !nvram_get("vlan1ports")
		    || nvram_match("vlan1ports", "")) {
			nvram_set("vlan0ports", "1 2 3 4 5*");
			nvram_set("vlan1ports", "0 5");
		}
		break;
	case ROUTER_ASUS_RTN66:
		if (!nvram_get("vlan1ports") || nvram_match("vlan1ports", "")
		    || !nvram_get("vlan2ports")
		    || nvram_match("vlan2ports", "")) {
			nvram_set("vlan1ports", "1 2 3 4 8*");
			nvram_set("vlan2ports", "0 8u");
		}
		break;
	case ROUTER_BUFFALO_WZR600DHP2:
	case ROUTER_BUFFALO_WZR900DHP:
	case ROUTER_BUFFALO_WZR1750:
	case ROUTER_DLINK_DIR868:
	case ROUTER_ASUS_AC56U:
	case ROUTER_TRENDNET_TEW812:
	case ROUTER_TRENDNET_TEW811:
		if (!nvram_get("vlan1ports") || nvram_match("vlan1ports", "")
		    || !nvram_get("vlan2ports")
		    || nvram_match("vlan2ports", "")) {
			nvram_set("vlan1ports", "0 1 2 3 5*");
			nvram_set("vlan2ports", "4 5u");
		}
		break;
	case ROUTER_LINKSYS_EA6900:
	case ROUTER_LINKSYS_EA6700:
	case ROUTER_LINKSYS_EA6500V2:
	case ROUTER_ASUS_AC67U:
		if (!nvram_get("vlan1ports") || nvram_match("vlan1ports", "")
		    || !nvram_get("vlan2ports")
		    || nvram_match("vlan2ports", "")) {
			nvram_set("vlan1ports", "1 2 3 4 5*");
			nvram_set("vlan2ports", "0 5u");
		}
		break;
	case ROUTER_ASUS_RTN53:
		if (!nvram_get("vlan1ports") || nvram_match("vlan1ports", "")
		    || !nvram_get("vlan2ports")
		    || nvram_match("vlan2ports", "")) {
			nvram_set("vlan1ports", "0 5");
			nvram_set("vlan2ports", "1 2 3 4 5*");
		}
		break;
	case ROUTER_BUFFALO_WZRG144NH:
		if (!nvram_get("vlan1ports")
		    || nvram_match("vlan1ports", "")
		    || !nvram_get("vlan2ports")
		    || nvram_match("vlan2ports", "")) {
			nvram_set("vlan1ports", "4 8");
			nvram_set("vlan2ports", "0 1 2 3 8*");
		}
		break;
	case ROUTER_NETGEAR_R6250:
	case ROUTER_NETGEAR_R6300V2:
		if (!nvram_get("vlan1ports") || nvram_match("vlan1ports", "")
		    || !nvram_get("vlan2ports")
		    || nvram_match("vlan2ports", "")) {
			nvram_set("vlan1ports", "3 2 1 0 5*");
			nvram_set("vlan2ports", "4 5u");
		}
		break;
	case ROUTER_NETGEAR_R7000:
		if (!nvram_get("vlan1ports") || nvram_match("vlan1ports", "")
		    || !nvram_get("vlan2ports")
		    || nvram_match("vlan2ports", "")) {
			nvram_set("vlan1ports", "4 3 2 1 5*");
			nvram_set("vlan2ports", "0 5u");
		}
		break;

	default:
		if (!nvram_get("vlan0hwname") || nvram_match("vlan0hwname", ""))
			nvram_set("vlan0hwname", "et0");
		if (!nvram_get("vlan1hwname") || nvram_match("vlan1hwname", ""))
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

		if (!nvram_get("vlan0ports") || nvram_match("vlan0ports", "")) {
			switch (brand) {
			case ROUTER_NETGEAR_WNR3500L:
			case ROUTER_WRT320N:
			case ROUTER_NETGEAR_WNDR4500:
			case ROUTER_NETGEAR_WNDR4500V2:
			case ROUTER_NETGEAR_R6250:
			case ROUTER_NETGEAR_R6300:
			case ROUTER_NETGEAR_R6300V2:
			case ROUTER_NETGEAR_R7000:
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
				if (nvram_match("bootnv_ver", "4")
				    || nvram_match("boardnum", "WAP54GV3_8M_0614"))
					nvram_set("vlan0ports", "3 2 1 0 5*");
				else
					nvram_set("vlan0ports", "1 2 3 4 5*");
				break;
			}
		}

		if (!nvram_get("vlan1ports") || nvram_match("vlan1ports", "")) {
			switch (brand) {
			case ROUTER_NETGEAR_WNR3500L:
			case ROUTER_NETGEAR_WNDR4500:
			case ROUTER_NETGEAR_WNDR4500V2:
			case ROUTER_NETGEAR_R6300:
				nvram_set("vlan2ports", "4 8");
				break;
			case ROUTER_NETGEAR_R6250:
			case ROUTER_NETGEAR_R6300V2:
				nvram_set("vlan2ports", "4 5u");
				break;
			case ROUTER_NETGEAR_R7000:
				nvram_set("vlan2ports", "0 5u");
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
				if (nvram_match("bootnv_ver", "4")
				    || nvram_match("boardnum", "WAP54GV3_8M_0614"))
					nvram_set("vlan1ports", "4 5");
				else
					nvram_set("vlan1ports", "0 5");
				break;
			}
		}

	}
	
	if (restore_defaults && nvram_get("vlan0ports") == NULL && nvram_get("vlan1ports")
	    && nvram_get("vlan2ports")) {
		nvram_set("port0vlans", "2");
		nvram_set("port1vlans", "1");
		nvram_set("port2vlans", "1");
		nvram_set("port3vlans", "1");
		nvram_set("port4vlans", "1");
		nvram_set("port5vlans", "1 2 16");
	}
	

	if (brand == ROUTER_WRT54G || brand == ROUTER_WRT54G1X || brand == ROUTER_LINKSYS_WRT55AG) {
		if (!nvram_get("aa0"))
			nvram_set("aa0", "3");
		if (!nvram_get("ag0"))
			nvram_set("ag0", "255");
		if (!nvram_get("gpio2"))
			nvram_set("gpio2", "adm_eecs");
		if (!nvram_get("gpio3"))
			nvram_set("gpio3", "adm_eesk");
		if (!nvram_get("gpio5"))
			nvram_set("gpio5", "adm_eedi");
		if (!nvram_get("gpio6"))
			nvram_set("gpio6", "adm_rc");
		if (!nvram_get("boardrev") || nvram_match("boardrev", ""))
			nvram_set("boardrev", "0x10");
		if (!nvram_get("boardflags") || nvram_match("boardflags", ""))
			nvram_set("boardflags", "0x0388");
		if (!nvram_get("boardflags2"))
			nvram_set("boardflags2", "0");
	}

	if (restore_defaults &&
	    (brand == ROUTER_ASUS_RTN10 || brand == ROUTER_ASUS_RTN10U || brand == ROUTER_ASUS_RTN12  || brand == ROUTER_ASUS_RTN12B ||
	     brand == ROUTER_ASUS_RTN53 || brand == ROUTER_ASUS_RTN10PLUSD1 || brand == ROUTER_ASUS_RTN16)) {
		nvram_set("wl0_txpwr", "17");
	}

	if (restore_defaults && (brand == ROUTER_LINKSYS_E4200)) {
		nvram_set("wl0_txpwr", "100");
		nvram_set("wl1_txpwr", "100");
	}
#ifndef HAVE_BUFFALO
	if (restore_defaults && brand == ROUTER_BUFFALO_WHRG54S && nvram_match("DD_BOARD", "Buffalo WHR-HP-G54")) {
		nvram_set("wl0_txpwr", "28");
	}
#endif
	if (restore_defaults && brand == ROUTER_BUFFALO_WLI_TX4_G54HP) {
		nvram_set("wl0_txpwr", "28");
	}

	/*
	 * Always set OS defaults 
	 */
	nvram_set("os_name", "linux");
	nvram_set("os_version", EPI_VERSION_STR);

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
	nvram_unset("probe_blacklist");

	if (nvram_get("overclocking") == NULL) {
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
	if (nvram_get("http_username") != NULL) {
		if (nvram_match("http_username", "")) {
#ifdef HAVE_POWERNOC
			nvram_set("http_username", "bJz7PcC1rCRJQ");	// admin
#else
			nvram_set("http_username", "bJ/GddyoJuiU2");	// root
#endif
		}
	}
	if (atoi(nvram_safe_get("nvram_ver")) < 3) {
		nvram_set("nvram_ver", "3");
		nvram_set("block_multicast", "1");
	}
	nvram_set("gozila_action", "0");
	nvram_set("generate_key", "0");
	nvram_set("clone_wan_mac", "0");
	nvram_unset("flash_active");

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

		if (!strcmp(nvram_get("router_style"), style)) {
			found = 1;
		}
	}
	if (found == 0 && firststyle[0] != '\0') {
		nvram_set("router_style", firststyle);
		if (!restore_defaults) {
			nvram_commit();
		}
	}
	globfree(&globbuf);

	/*
	 * Commit values 
	 */
	if (restore_defaults) {
		int i;

		unset_nvram();
		nvram_commit();
		cprintf("done\n");
		for (i = 0; i < MAX_NVPARSE; i++) {
			del_wds_wsec(0, i);
			del_wds_wsec(1, i);
		}
	}
}

void start_drivers(void)
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
	if (nvram_match("usb_enable", "1")) {
		led_control(USB_POWER, LED_ON);
		led_control(USB_POWER1, LED_ON);
		led_control(LED_USB, LED_ON);
		led_control(LED_USB1, LED_ON);

		insmod("nls_base");
		insmod("usb-common");
		cprintf("loading usbcore\n");
		insmod("usbcore");

		cprintf("loading usb2 module\n");
		insmod("ehci-hcd");
		insmod("ehci-platform");
		insmod("ehci-pci");

		cprintf("loading usb-uhci\n");
		insmod("usb-uhci");
		insmod("uhci-hcd");

		cprintf("loading usb-ohci\n");
		insmod("usb-ohci");
		insmod("ohci-hcd");

		insmod("xhci-hcd");
		insmod("dwc_otg");	// usb
		insmod("usb-libusual");

		insmod("fsl-mph-dr-of");
		insmod("phy-mxs-usb");
		insmod("ci_hdrc");
		insmod("ci13xxx_imx");
		insmod("usbmisc_imx");
		insmod("ci_hdrc_imx");
		
		
		if (nvram_match("usb_storage", "1")) {
			cprintf("loading scsi_mod\n");
			insmod("scsi_mod");
			insmod("scsi_wait_scan");
			cprintf("loading sd_mod\n");
			insmod("sd_mod");
			cprintf("loading cdrom drivers\n");
			insmod("cdrom");
			insmod("sr_mod");
			cprintf("loading usb-storage\n");
			insmod("usb-storage");
		}

		if (nvram_match("usb_printer", "1")) {
			cprintf("loading printer\n");
			insmod("printer");
			insmod("usblp");
		}
#ifdef HAVE_USBIP
		if (nvram_match("usb_ip", "1")) {
			cprintf("loading usb over ip drivers\n");
			insmod("usbip_common_mod");
			insmod("usbip");

			insmod("usbip-core");
			insmod("usbip-host");
			eval("usbipd", "-D");
		}
#endif

//ahci
		insmod("libata");
		insmod("libahci");
		insmod("ahci");
		insmod("ahci_platforms");
		insmod("ahci_imx");
//mmc
		insmod("mmc_core");
		insmod("mmc_block");
		insmod("sdhci");
		insmod("sdhci-pltfm");
		insmod("sdhci-esdhc-imx");
		

		mount("devpts", "/proc/bus/usb", "usbfs", MS_MGC_VAL, NULL);
//   Mounting is done by hotplug event!         
//              if( nvram_match("usb_automnt", "1") && nvram_match("usb_storage", "1")) {
//                      printf(stderr, "[USB] check for drives....\n");
//                      usb_add_ufd();
//              }
	} else {
		eval("stopservice", "samba3");
		eval("stopservice", "ftpsrv");
		sysprintf("umount /%s", nvram_default_get("usb_mntpoint", "mnt"));
		rmmod("usblp");
		rmmod("printer");
		rmmod("usb-storage");
		rmmod("sr_mod");
		rmmod("cdrom");
		rmmod("sd_mod");
		rmmod("scsi_wait_scan");
		rmmod("scsi_mod");


		rmmod("usbmisc_imx");
		rmmod("ci13xxx_imx");
		rmmod("ci_hdrc");
		rmmod("phy-mxs-usb");
		rmmod("fsl-mph-dr-of");

		rmmod("usb-libusual");
		rmmod("dwc_otg");	// usb
		rmmod("xhci-hcd");


		rmmod("usb-ohci");
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
		rmmod("nls_utf8");
		rmmod("nls_iso8859-2");
		rmmod("nls_iso8859-1");
		rmmod("nls_cp437");
		rmmod("nls_cp932");
		rmmod("nls_cp936");
		rmmod("nls_cp950");
		rmmod("nls_base");
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
#endif

		led_control(USB_POWER, LED_OFF);
		led_control(USB_POWER1, LED_OFF);

		led_control(LED_USB, LED_OFF);
		led_control(LED_USB1, LED_OFF);
	}
#endif

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
		} else if (sig == SIGUSR1) {	// Receive from WEB
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

#define CONVERT_NV(old, new) \
	if(nvram_get(old)) \
		nvram_set(new, nvram_safe_get(old));

void start_nvram(void)
{
	int i = 0;

	/*
	 * broadcom 3.11.48.7 change some nvram name 
	 */
#if 0
// TEMP!!!!
	nvram_set("wl_bss_opmode_cap_reqd", "2");
	nvram_set("wl_channel", "52");
	nvram_set("wl_nband", "1");
	nvram_set("wl_nmode", "2");
	nvram_set("wl_nreqd", "1");

	nvram_set("wl0_authmode", "open");
	nvram_set("wl0_macmode1", "disabled");
	nvram_set("wl1_auth_mode", "none");
	nvram_set("wl1_authmode", "open");
	nvram_set("wl1_bss_opmode_cap_reqd", "2");
	nvram_set("wl1_vifs", "");

	nvram_set("af_dnathost", "0");
	nvram_set("af_dnatport", "0");
	nvram_set("af_registered", "0");
	nvram_set("af_serviceid", "0");
	nvram_set("br0_mtu", "1500");
	nvram_set("bridges", "br0>Off>32768>1500");
	nvram_set("bridges_count", "1");
	nvram_set("bridgesif", "");
	nvram_set("bridgesif_count", "0");
	nvram_set("browser_method", "USE_LAN");
	nvram_set("eth0_mtu", "1500");
	nvram_set("eth0_multicast", "0");
	nvram_set("eth0_nat", "1");
	nvram_set("eth1_mtu", "1500");
	nvram_set("eth2_mtu", "1500");
	nvram_set("vlan1_mtu", "1500");
	nvram_set("vlan1_multicast", "0");
	nvram_set("vlan1_nat", "1");
	nvram_set("vlan2_bridged", "1");
	nvram_set("vlan2_mtu", "1500");
	nvram_set("vlan2_multicast", "0");
	nvram_set("vlan2_nat", "1");
	nvram_set("vlan_tagcount", "0");
	nvram_set("vlan_tags", "");
	nvram_set("wan_iface", "");
#endif
	nvram_unset("wl0_hwaddr");	// When disbale wireless, we must get 
	// 
	// null wireless mac */

	nvram_set("wan_get_dns", "");
	nvram_set("filter_id", "1");
	nvram_set("wl_active_add_mac", "0");
	nvram_set("ddns_change", "");
	nvram_unset("action_service");
	nvram_set("wan_get_domain", "");

	// if(!nvram_get("wl_macmode1")){
	// if(nvram_match("wl_macmode","disabled"))
	// nvram_set("wl_macmode1","disabled");
	// else
	// nvram_set("wl_macmode1","other");
	// }
	if (nvram_match("wl_gmode", "5"))	// Mixed mode had been
		// changed to 5
		nvram_set("wl_gmode", "1");

	if (nvram_match("wl_gmode", "4"))	// G-ONLY mode had been
		// changed to 2, after 1.40.1 
		// for WiFi G certication
		nvram_set("wl_gmode", "2");

	// nvram_set("wl_country","Worldwide"); // The country always Worldwide

	nvram_set("ping_ip", "");
	nvram_set("ping_times", "");
	// nvram_set ("traceroute_ip", "");

	nvram_set("filter_port", "");	// The name have been disbaled from
	// 1.41.3

#ifdef HAVE_UPNP
	if ((nvram_match("restore_defaults", "1"))
	    || (nvram_match("upnpcas", "1"))) {
		nvram_set("upnp_clear", "1");
	} else {
		char s[32];
		char *nv;

		for (i = 0; i < MAX_NVPARSE; ++i) {
			sprintf(s, "forward_port%d", i);
			if ((nv = nvram_get(s)) != NULL) {
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
	if (nvram_match("wl_wep", "tkip")) {
		nvram_set("wl_crypto", "tkip");
	} else if (nvram_match("wl_wep", "aes")) {
		nvram_set("wl_crypto", "aes");
	} else if (nvram_match("wl_wep", "tkip+aes")) {
		nvram_set("wl_crypto", "tkip+aes");
	}

	if (nvram_match("wl_wep", "restricted"))
		nvram_set("wl_wep", "enabled");	// the nas need this value,
	// the "restricted" is no
	// longer need. (20040624 by
	// honor)

#ifdef HAVE_SET_BOOT
	if (!nvram_match("boot_wait_web", "0"))
		nvram_set("boot_wait_web", "1");
#endif
#ifndef HAVE_BUFFALO
	if (check_hw_type() == BCM5352E_CHIP) {
		nvram_set("opo", "0");	// OFDM power reducement in quarter
		// dbm (2 dbm in this case)
		nvram_set("ag0", "0");	// Antenna Gain definition in dbm
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

	// fix openvpnclient and server values (was 0/1 now is yes/no/adaptive)
	// convert 0 -> no and 1 -> adaptive
	if (nvram_match("openvpn_lzo", "0"))
		nvram_set("openvpn_lzo", "no");
	if (nvram_match("openvpn_lzo", "1"))
		nvram_set("openvpn_lzo", "adaptive");

	if (nvram_match("openvpncl_lzo", "0"))
		nvram_set("openvpncl_lzo", "no");
	if (nvram_match("openvpncl_lzo", "1"))
		nvram_set("openvpncl_lzo", "adaptive");

	nvram_unset("vdsl_state");	// important (this value should never 
	// 
	// be commited, but if this will fix
	// the vlan7 issue)
	nvram_unset("fromvdsl");	// important (this value should never be
	// commited, but if this will fix the vlan7
	// issue)

	nvram_unset("do_reboot");	//for GUI, see broadcom.c
	nvram_set("auth_time", "0");

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
	if (!strlen(nvram_safe_get("hopseq"))
	    || !strlen(nvram_safe_get("hopdwell"))) {
		nvram_set("hopdwell", "1000");
		nvram_set("hopseq", "0");
	}
#endif
	nvram_unset("lasthour");

#ifdef HAVE_SVQOS
	char *aqd = nvram_safe_get("svqos_aqd");
#ifndef HAVE_CODEL
	if (!strcmp(aqd, "codel"))
		nvram_set("svqos_aqd", "sfq");
#endif
#ifndef HAVE_FQ_CODEL
	if (!strcmp(aqd, "fq_codel"))
		nvram_set("svqos_aqd", "sfq");
#endif
	if (strcmp(aqd, "codel")
	    && strcmp(aqd, "fq_codel"))
		nvram_set("svqos_aqd", "sfq");
#endif

#ifdef HAVE_AQOS
	//filter hostapd shaping rules

	char *qos_mac = nvram_safe_get("svqos_macs");

	if (strlen(qos_mac) > 0) {
		char *newqos = malloc(strlen(qos_mac) + 254);
		memset(newqos, 0, strlen(qos_mac) + 254);

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
				if (strlen(newqos) > 0)
					sprintf(newqos, "%s %s %s %s %s %s %s |", newqos, data, level, level2, type, level3, prio);
				else
					sprintf(newqos, "%s %s %s %s %s %s |", data, level, level2, type, level3, prio);

			}
		}
		while ((qos_mac = strpbrk(++qos_mac, "|")) && qos_mac++);
		nvram_set("svqos_macs", newqos);
		free(newqos);
	}

	char *qos_ip = nvram_safe_get("svqos_ips");

	if (strlen(qos_ip) > 0) {
		char *newip = malloc(strlen(qos_ip) + 254);
		memset(newip, 0, strlen(qos_ip) + 254);

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

			if (strlen(newip) > 0)
				sprintf(newip, "%s %s %s %s %s %s |", newip, data, level, level2, level3, prio);
			else
				sprintf(newip, "%s %s %s %s %s |", data, level, level2, level3, prio);
		}
		while ((qos_ip = strpbrk(++qos_ip, "|")) && qos_ip++);
		nvram_set("svqos_ips", newip);
		free(newip);
	}
#endif
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
