/*
 * sysinit-northstar.c
 *
 * Copyright (C) 2012 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#include <linux/if_ether.h>
#include <linux/mii.h>
#include <linux/sockios.h>
#include <net/if.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <linux/mii.h>

#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <cymac.h>
#include <fcntl.h>
#include "devices/ethernet.c"
#include "devices/wireless.c"

#define sys_restart() eval("event","3","1","1")
#define sys_reboot() eval("sync"); eval("event","3","1","15")

static void set_regulation(int card, char *code, char *rev)
{
	char path[32];
	sprintf(path, "%d:regrev", card);
	nvram_set(path, rev);
	sprintf(path, "%d:ccode", card);
	nvram_set(path, code);
	sprintf(path, "wl%d_country_rev", card);
	nvram_set(path, rev);
	sprintf(path, "wl%d_country_code", card);
	nvram_set(path, code);
	if (!card) {
		nvram_set("wl_country_rev", rev);
		nvram_set("wl_country_code", code);
	}
}

void start_sysinit(void)
{
	char buf[PATH_MAX];
	struct stat tmp_stat;
	time_t tm = 0;

	insmod("softdog");
	/*
	 * Setup console 
	 */
	if (nvram_get("bootflags") == NULL) {
		FILE *fp = fopen("/dev/mtdblock0", "rb");
		if (fp) {
			fseek(fp, 0, SEEK_END);
			long seek = ftell(fp);
			fprintf(stderr, "length = %X\n");
			if (seek == 0x200000) {
				char *temp = malloc(65536);
				fseek(fp, seek - 0x10000, SEEK_SET);
				fread(temp, 1, 65536, fp);
				fclose(fp);
				fp = fopen("/tmp/nvramcopy", "wb");
				fwrite(temp, 1, 65536, fp);
				sysprintf("mtd -f write /tmp/nvramcopy nvram");
				sys_reboot();
			}
			fclose(fp);
		}
	}
	cprintf("sysinit() klogctl\n");
	klogctl(8, NULL, atoi(nvram_safe_get("console_loglevel")));
	cprintf("sysinit() get router\n");

	int brand = getRouterBrand();

	//for extension board
	struct ifreq ifr;
	int s;

	fprintf(stderr, "try modules for ethernet adapters\n");
	nvram_set("intel_eth", "0");

	mkdir("/dev/gpio", 0700);
	mknod("/dev/gpio/in", S_IFCHR | 0644, makedev(127, 0));
	mknod("/dev/gpio/out", S_IFCHR | 0644, makedev(127, 1));
	mknod("/dev/gpio/outen", S_IFCHR | 0644, makedev(127, 2));
	mknod("/dev/gpio/control", S_IFCHR | 0644, makedev(127, 3));
	mknod("/dev/gpio/hc595", S_IFCHR | 0644, makedev(127, 4));
	if (nvram_invmatch("boot_wait", "on") || nvram_match("wait_time", "1")) {
		nvram_set("boot_wait", "on");
		nvram_set("wait_time", "3");
		nvram_commit();
	}

	switch (getRouterBrand()) {
	case ROUTER_ASUS_AC67U:
		if (nvram_get("productid") != NULL || nvram_match("http_username", "admin")) {
			int deadcount = 10;
			while (deadcount--) {
				FILE *fp = fopen("/dev/mtdblock1", "rb");
				if (fp == NULL) {
					fprintf(stderr, "waiting for mtd devices to get available %d\n", deadcount);
					sleep(1);
					continue;
				}
				fclose(fp);
				break;
			}
			sleep(1);
			sysprintf("/sbin/erase nvram");
			nvram_set("flash_active", "1");	// prevent recommit of value until reboot is done
			sys_reboot();
		}
		set_gpio(4, 0);	// enable all led's which are off by default
		set_gpio(14, 1);	// usb led
		set_gpio(1, 1);	// wan
		set_gpio(2, 0);	// lan
		set_gpio(3, 0);	// power
		set_gpio(6, 0);	// wireless 5 ghz
		set_gpio(0, 1);	// usb 3.0 led           
		set_gpio(7, 1); // fixup ses button
		set_gpio(15, 1); // fixup wifi button
		nvram_set("0:ledbh10", "7");
		nvram_set("1:ledbh10", "7");
		nvram_set("1:ledbh6", "136");	// fixup 5 ghz led

		nvram_set("0:maxp2ga0", "106");
		nvram_set("0:maxp2ga1", "106");
		nvram_set("0:maxp2ga2", "106");
		nvram_set("0:cckbw202gpo", "0");
		nvram_set("0:cckbw20ul2gpo", "0");
		nvram_set("0:mcsbw202gpo", "0x65320000");
		nvram_set("0:mcsbw402gpo", "0x65320000");
		nvram_set("0:dot11agofdmhrbw202gpo", "0x3200");
		nvram_set("0:ofdmlrbw202gpo", "0");
		nvram_set("0:sb20in40hrpo", "0");
		nvram_set("0:sb20in40lrpo", "0");
		nvram_set("0:dot11agduphrpo", "0");
		nvram_set("0:dot11agduplrpo", "0");

		nvram_set("1:maxp5ga0", "106,106,106,106");
		nvram_set("1:maxp5ga1", "106,106,106,106");
		nvram_set("1:maxp5ga2", "106,106,106,106");
		nvram_set("1:mcsbw205glpo", "0x65320000");
		nvram_set("1:mcsbw405glpo", "0x65320000");
		nvram_set("1:mcsbw805glpo", "0x65320000");
		nvram_set("1:mcsbw1605glpo", "0");
		nvram_set("1:mcsbw205gmpo", "0x65320000");
		nvram_set("1:mcsbw405gmpo", "0x65320000");
		nvram_set("1:mcsbw805gmpo", "0x65320000");
		nvram_set("1:mcsbw1605gmpo", "0");
		nvram_set("1:mcsbw205ghpo", "0x65320000");
		nvram_set("1:mcsbw405ghpo", "0x65320000");
		nvram_set("1:mcsbw805ghpo", "0x65320000");
		nvram_set("1:mcsbw1605ghpo", "0");
		break;
	case ROUTER_ASUS_AC56U:
		if (nvram_get("productid") != NULL || nvram_match("http_username", "admin")) {
			int deadcount = 10;
			while (deadcount--) {
				FILE *fp = fopen("/dev/mtdblock1", "rb");
				if (fp == NULL) {
					fprintf(stderr, "waiting for mtd devices to get available %d\n", deadcount);
					sleep(1);
					continue;
				}
				fclose(fp);
				break;
			}
			sleep(1);
			sysprintf("/sbin/erase nvram");
			nvram_set("flash_active", "1");	// prevent recommit of value until reboot is done
			sys_reboot();
		}
		set_gpio(4, 0);	// enable all led's which are off by default
		set_gpio(14, 1);	// usb led
		set_gpio(1, 1);	// wan
		set_gpio(2, 0);	// lan
		set_gpio(3, 0);	// power
		set_gpio(6, 0);	// wireless 5 ghz
		set_gpio(0, 1);	// usb 3.0 led           
		set_gpio(7, 1); // fixup wifi button
		set_gpio(15, 1); // fixup ses button
		nvram_set("1:ledbh6", "136");	// fixup 5 ghz led
		nvram_unset("1:ledbh10");	// fixup 5 ghz led

		// tx power fixup
		nvram_set("0:maxp2ga0", "0x68");
		nvram_set("0:maxp2ga1", "0x68");
		nvram_set("0:cck2gpo", "0x1111");
		nvram_set("0:ofdm2gpo", "0x54333333");
		nvram_set("0:mcs2gpo0", "0x3333");
		nvram_set("0:mcs2gpo1", "0x9753");
		nvram_set("0:mcs2gpo2", "0x3333");
		nvram_set("0:mcs2gpo3", "0x9753");
		nvram_set("0:mcs2gpo4", "0x5555");
		nvram_set("0:mcs2gpo5", "0xB755");
		nvram_set("0:mcs2gpo6", "0x5555");
		nvram_set("0:mcs2gpo7", "0xB755");

		nvram_set("1:maxp5ga0", "104,104,104,104");
		nvram_set("1:maxp5ga1", "104,104,104,104");
		nvram_set("1:mcsbw205glpo", "0xAA864433");
		nvram_set("1:mcsbw405glpo", "0xAA864433");
		nvram_set("1:mcsbw805glpo", "0xAA864433");
		nvram_set("1:mcsbw205gmpo", "0xAA864433");
		nvram_set("1:mcsbw405gmpo", "0xAA864433");
		nvram_set("1:mcsbw805gmpo", "0xAA864433");
		nvram_set("1:mcsbw205ghpo", "0xAA864433");
		nvram_set("1:mcsbw405ghpo", "0xAA864433");
		nvram_set("1:mcsbw805ghpo", "0xAA864433");
		// regulatory setup

		if (nvram_match("regulation_domain", "US"))
			set_regulation(0, "US", "0");
		else if (nvram_match("regulation_domain", "Q2"))
			set_regulation(0, "US", "0");
		else if (nvram_match("regulation_domain", "EU"))
			set_regulation(0, "DE", "0");
		else if (nvram_match("regulation_domain", "TW"))
			set_regulation(0, "TW", "13");
		else if (nvram_match("regulation_domain", "CN"))
			set_regulation(0, "CN", "1");
		else
			set_regulation(0, "US", "0");

		if (nvram_match("regulation_domain_5G", "US"))
			set_regulation(1, "US", "0");
		else if (nvram_match("regulation_domain_5G", "Q2"))
			set_regulation(1, "US", "0");
		else if (nvram_match("regulation_domain_5G", "EU"))
			set_regulation(1, "DE", "0");
		else if (nvram_match("regulation_domain_5G", "TW"))
			set_regulation(1, "TW", "13");
		else if (nvram_match("regulation_domain_5G", "CN"))
			set_regulation(1, "CN", "1");
		else
			set_regulation(1, "US", "0");

		break;
	case ROUTER_DLINK_DIR868:

		if (nvram_get("pci/1/1/vendid") == NULL) {

			unsigned char buf[64];
			FILE *fp = popen("cat /dev/mtdblock0|grep lanmac", "rb");
			fread(buf, 1, 24, fp);
			fclose(fp);
			buf[24] = 0;
			fprintf(stderr, "set main mac %s\n", &buf[7]);
			nvram_set("et0macaddr", &buf[7]);

			fp = popen("cat /dev/mtdblock0|grep wlan5mac", "rb");
			fread(buf, 1, 26, fp);
			fclose(fp);
			buf[26] = 0;
			fprintf(stderr, "set 5g mac %s\n", &buf[9]);
			nvram_set("pci/2/0/macaddr", &buf[9]);
			nvram_set("pci/2/1/macaddr", &buf[9]);
			fp = popen("cat /dev/mtdblock0|grep wlan24mac", "rb");
			fread(buf, 1, 27, fp);
			fclose(fp);
			buf[27] = 0;
			fprintf(stderr, "set 2.4g mac %s\n", &buf[10]);
			nvram_set("pci/1/0/macaddr", &buf[10]);
			nvram_set("pci/1/1/macaddr", &buf[10]);

			struct nvram_tuple dir868_1_1params[] = {
				{"maxp2ga0", "0x56", 0},
				{"maxp2ga1", "0x56", 0},
				{"maxp2ga2", "0x56", 0},
				{"cckbw202gpo", "0x0000", 0},
				{"cckbw20ul2gpo", "0x0000", 0},
				{"legofdmbw202gpo", "0x00000000", 0},
				{"legofdmbw20ul2gpo", "0x00000000", 0},
				{"mcsbw202gpo", "0x00000000", 0},
				{"mcsbw20ul2gpo", "0x00000000", 0},
				{"mcsbw402gpo", "0x22222222", 0},
				{"pa2gw0a0", "0xFE7C", 0},
				{"pa2gw1a0", "0x1C9B", 0},
				{"pa2gw2a0", "0xF915", 0},
				{"pa2gw0a1", "0xFE85", 0},
				{"pa2gw1a1", "0x1D25", 0},
				{"pa2gw2a1", "0xF906", 0},
				{"pa2gw0a2", "0xFE82", 0},
				{"pa2gw1a2", "0x1D45", 0},
				{"pa2gw2a2", "0xF900", 0},
				{"ag0", "0x0", 0},
				{"ag1", "0x0", 0},
				{0, 0, 0}
			};

			struct nvram_tuple dir868_2_1params[] = {
				{"sromrev", "11", 0},
				{"venid", "0x14E4", 0},
				{"vendid", "0x14E4", 0},
				{"boardvendor", "0x14E4", 0},
				{"devid", "0x43a2", 0},
				{"boardrev", "0x1450", 0},
				{"boardflags", "0x30000000", 0},
				{"boardflags2", "0x300002", 0},
				{"boardtype", "0x621", 0},
				{"boardflags3", "0x0", 0},
				{"boardnum", "0", 0},
				{"ccode", "20785", 0},
				{"regrev", "27", 0},
				{"aa2g", "0", 0},
				{"aa5g", "7", 0},
				{"agbg0", "0x0", 0},
				{"agbg1", "0x0", 0},
				{"agbg2", "0x0", 0},
				{"aga0", "0x0", 0},
				{"aga1", "0x0", 0},
				{"aga2", "0x0", 0},
				{"txchain", "7", 0},
				{"rxchain", "7", 0},
				{"antswitch", "0", 0},
				{"tssiposslope2g", "1", 0},
				{"epagain2g", "0", 0},
				{"pdgain2g", "4", 0},
				{"tworangetssi2g", "0", 0},
				{"papdcap2g", "0", 0},
				{"femctrl", "3", 0},
				{"tssiposslope5g", "1", 0},
				{"epagain5g", "0", 0},
				{"pdgain5g", "4", 0},
				{"tworangetssi5g", "0", 0},
				{"papdcap5g", "0", 0},
				{"gainctrlsph", "0", 0},
				{"tempthresh", "255", 0},
				{"tempoffset", "255", 0},
				{"rawtempsense", "0x1ff", 0},
				{"measpower", "0x7f", 0},
				{"tempsense_slope", "0xff", 0},
				{"tempcorrx", "0x3f", 0},
				{"tempsense_option", "0x3", 0},
				{"phycal_tempdelta", "255", 0},
				{"temps_period", "15", 0},
				{"temps_hysteresis", "15", 0},
				{"measpower1", "0x7f", 0},
				{"measpower2", "0x7f", 0},
				{"pdoffset40ma0", "4369", 0},
				{"pdoffset40ma1", "4369", 0},
				{"pdoffset40ma2", "4369", 0},
				{"pdoffset80ma0", "0", 0},
				{"pdoffset80ma1", "0", 0},
				{"pdoffset80ma2", "0", 0},
				{"subband5gver", "0x4", 0},
				{"subvid", "0x14e4", 0},
				{"cckbw202gpo", "0", 0},
				{"cckbw20ul2gpo", "0", 0},
				{"mcsbw202gpo", "0", 0},
				{"mcsbw402gpo", "0", 0},
				{"dot11agofdmhrbw202g", "0", 0},
				{"ofdmlrbw202gpo", "0", 0},
				{"mcsbw205glpo", "3398914833", 0},
				{"mcsbw405glpo", "3398914833", 0},
				{"mcsbw805glpo", "3398914833", 0},
				{"mcsbw1605glpo", "0", 0},
				{"mcsbw205gmpo", "3398914833", 0},
				{"mcsbw405gmpo", "3398914833", 0},
				{"mcsbw805gmpo", "3398914833", 0},
				{"mcsbw1605gmpo", "0", 0},
				{"mcsbw205ghpo", "3398914833", 0},
				{"mcsbw405ghpo", "3398914833", 0},
				{"mcsbw805ghpo", "3398914833", 0},
				{"mcsbw1605ghpo", "0", 0},
				{"mcslr5glpo", "0", 0},
				{"mcslr5gmpo", "0", 0},
				{"mcslr5ghpo", "0", 0},
				{"sb20in40hrpo", "0", 0},
				{"sb20in80and160hr5gl", "0", 0},
				{"sb40and80hr5glpo", "0", 0},
				{"sb20in80and160hr5gm", "0", 0},
				{"sb40and80hr5gmpo", "0", 0},
				{"sb20in80and160hr5gh", "0", 0},
				{"sb40and80hr5ghpo", "0", 0},
				{"sb20in40lrpo", "0", 0},
				{"sb20in80and160lr5gl", "0", 0},
				{"sb40and80lr5glpo", "0", 0},
				{"sb20in80and160lr5gm", "0", 0},
				{"sb40and80lr5gmpo", "0", 0},
				{"sb20in80and160lr5gh", "0", 0},
				{"sb40and80lr5ghpo", "0", 0},
				{"dot11agduphrpo", "0", 0},
				{"dot11agduplrpo", "0", 0},
				{"pcieingress_war", "15", 0},
				{"sar2g", "18", 0},
				{"sar5g", "15", 0},
				{"noiselvl2ga0", "31", 0},
				{"noiselvl2ga1", "31", 0},
				{"noiselvl2ga2", "31", 0},
				{"noiselvl5ga0", "31,31,31,31", 0},
				{"noiselvl5ga1", "31,31,31,31", 0},
				{"noiselvl5ga2", "31,31,31,31", 0},
				{"rxgainerr2ga0", "63", 0},
				{"rxgainerr2ga1", "31", 0},
				{"rxgainerr2ga2", "31", 0},
				{"rxgainerr5ga0", "63,63,63,63", 0},
				{"rxgainerr5ga1", "31,31,31,31", 0},
				{"rxgainerr5ga2", "31,31,31,31", 0},
				{"maxp2ga0", "76", 0},
				{"pa2ga0", "0xfe72,0x14c0,0xfac7", 0},
				{"rxgains5gmelnagaina0", "2", 0},
				{"rxgains5gmtrisoa0", "5", 0},
				{"rxgains5gmtrelnabypa0", "1", 0},
				{"rxgains5ghelnagaina0", "2", 0},
				{"rxgains5ghtrisoa0", "5", 0},
				{"rxgains5ghtrelnabypa0", "1", 0},
				{"rxgains2gelnagaina0", "0", 0},
				{"rxgains2gtrisoa0", "0", 0},
				{"rxgains2gtrelnabypa0", "0", 0},
				{"rxgains5gelnagaina0", "1", 0},
				{"rxgains5gtrisoa0", "7", 0},
				{"rxgains5gtrelnabypa0", "1", 0},
				{"maxp5ga0", "92,92,92,92", 0},
				{"pa5ga0", "0xff26,0x188e,0xfcf0,0xff2a,0x18ee,0xfcec,0xff21,0x18b4,0xfcec,0xff23,0x1930,0xfcdd", 0},
				{"maxp2ga1", "76", 0},
				{"pa2ga1", "0xfe80,0x1472,0xfabc", 0},
				{"rxgains5gmelnagaina1", "2", 0},
				{"rxgains5gmtrisoa1", "4", 0},
				{"rxgains5gmtrelnabypa1", "1", 0},
				{"rxgains5ghelnagaina1", "2", 0},
				{"rxgains5ghtrisoa1", "4", 0},
				{"rxgains5ghtrelnabypa1", "1", 0},
				{"rxgains2gelnagaina1", "0", 0},
				{"rxgains2gtrisoa1", "0", 0},
				{"rxgains2gtrelnabypa1", "0", 0},
				{"rxgains5gelnagaina1", "1", 0},
				{"rxgains5gtrisoa1", "6", 0},
				{"rxgains5gtrelnabypa1", "1", 0},
				{"maxp5ga1", "92,92,92,92", 0},
				{"pa5ga1", "0xff35,0x1a3c,0xfccc,0xff31,0x1a06,0xfccf,0xff2b,0x1a54,0xfcc5,0xff30,0x1ad5,0xfcb9", 0},
				{"maxp2ga2", "76", 0},
				{"pa2ga2", "0xfe82,0x14bf,0xfad9", 0},
				{"rxgains5gmelnagaina2", "3", 0},
				{"rxgains5gmtrisoa2", "4", 0},
				{"rxgains5gmtrelnabypa2", "1", 0},
				{"rxgains5ghelnagaina2", "3", 0},
				{"rxgains5ghtrisoa2", "4", 0},
				{"rxgains5ghtrelnabypa2", "1", 0},
				{"rxgains2gelnagaina2", "0", 0},
				{"rxgains2gtrisoa2", "0", 0},
				{"rxgains2gtrelnabypa2", "0", 0},
				{"rxgains5gelnagaina2", "1", 0},
				{"rxgains5gtrisoa2", "5", 0},
				{"rxgains5gtrelnabypa2", "1", 0},
				{"maxp5ga2", "92,92,92,92", 0},
				{"pa5ga2", "0xff2e,0x197b,0xfcd8,0xff2d,0x196e,0xfcdc,0xff30,0x1a7d,0xfcc2,0xff2e,0x1ac6,0xfcb4", 0},

				{0, 0, 0}
			};

			struct nvram_tuple *t;
			t = dir868_1_1params;
			while (t->name) {
//                              fprintf(stderr, "set pci/1/1/%s to %s\n", t->name, t->value);
				nvram_nset(t->value, "pci/1/1/%s", t->name);
				t++;
			}
			t = dir868_2_1params;
			while (t->name) {
//                              fprintf(stderr, "set pci/2/1/%s to %s\n", t->name, t->value);
				nvram_nset(t->value, "pci/2/1/%s", t->name);
				t++;
			}

			nvram_set("pci/1/1/vendid", "0x14E4");

		}
		break;
	case ROUTER_BUFFALO_WZR1750:
		nvram_default_get("wl_country_code", "US");
		nvram_default_get("wl0_country_code", "US");
		nvram_default_get("wl1_country_code", "US");
		nvram_default_get("wl_country_rev", "0");
		nvram_default_get("wl0_country_rev", "0");
		nvram_default_get("wl1_country_rev", "0");
		nvram_set("0:ledbh12","7");
		nvram_set("1:ledbh10","7");
	break;
	case ROUTER_BUFFALO_WZR900DHP:
	case ROUTER_BUFFALO_WZR600DHP2:
		nvram_default_get("wl_country_code", "US");
		nvram_default_get("wl0_country_code", "US");
		nvram_default_get("wl1_country_code", "US");
		nvram_default_get("wl_country_rev", "0");
		nvram_default_get("wl0_country_rev", "0");
		nvram_default_get("wl1_country_rev", "0");
		nvram_set("0:boardflags2","0x1000");
		nvram_set("1:boardflags2","0x00001000");
		nvram_set("0:ledbh12","7");
		nvram_set("1:ledbh10","7");
		break;

	default:
		nvram_set("bootpartition", "0");
		nvram_default_get("wl_country_code", "US");
		nvram_default_get("wl0_country_code", "US");
		nvram_default_get("wl1_country_code", "US");
		nvram_default_get("wl_country_rev", "0");
		nvram_default_get("wl0_country_rev", "0");
		nvram_default_get("wl1_country_rev", "0");

	}

	insmod("et");
	//load mmc drivers
	eval("ifconfig", "eth0", "up");
	eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
	eval("vconfig", "add", "eth0", "1");
	eval("vconfig", "add", "eth0", "2");
	insmod("switch-core");
	insmod("switch-robo");

	int fd;

	if ((fd = open("/proc/irq/163/smp_affinity", O_RDWR)) >= 0) {
		close(fd);
#ifndef HAVE_SAMBA
		if (!nvram_match("samba3_enable", "1"))
#endif
		{		// not set txworkq 
			writeproc("/proc/irq/163/smp_affinity", "2");
			writeproc("/proc/irq/169/smp_affinity", "2");
		}
		writeproc("/proc/irq/112/smp_affinity", "2");
	}

	/*
	 * network drivers 
	 */
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW))) {
		char eabuf[32];

		strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
		ioctl(s, SIOCGIFHWADDR, &ifr);
//              nvram_set("et0macaddr", ether_etoa((unsigned char *)ifr.ifr_hwaddr.sa_data, eabuf));
		nvram_set("et0macaddr_safe", ether_etoa((unsigned char *)ifr.ifr_hwaddr.sa_data, eabuf));
		close(s);
	}

	insmod("wl");
	/*
	 * Set a sane date 
	 */
	stime(&tm);
	nvram_set("wl0_ifname", "ath0");

	led_control(LED_POWER, LED_ON);
	led_control(LED_DIAG, LED_OFF);
	led_control(LED_SES, LED_OFF);
	led_control(LED_SES2, LED_OFF);
	led_control(LED_BRIDGE, LED_OFF);
	led_control(LED_WLAN0, LED_OFF);
	led_control(LED_WLAN1, LED_OFF);
	led_control(LED_CONNECTED, LED_OFF);

	if (!nvram_match("disable_watchdog", "1")) {
		eval("watchdog");
	}

	return;
}

int check_cfe_nv(void)
{
	nvram_set("portprio_support", "0");
	return 0;
}

int check_pmon_nv(void)
{
	return 0;
}

void start_overclocking(void)
{
#ifdef HAVE_OVERCLOCKING
	cprintf("Overclocking started\n");

	int rev = cpu_plltype();

	if (rev == 0)
		return;		// unsupported

	char *ov = nvram_get("overclocking");

	if (ov == NULL)
		return;
	int clk = atoi(ov);

	if (nvram_get("clkfreq") == NULL)
		return;		// unsupported

	char *pclk = nvram_safe_get("clkfreq");
	char dup[64];

	strcpy(dup, pclk);
	int i;

	for (i = 0; i < strlen(dup); i++)
		if (dup[i] == ',')
			dup[i] = 0;
	int cclk = atoi(dup);

	if (clk == cclk) {
		cprintf("clkfreq %d MHz identical with new setting\n", clk);
		return;		// clock already set
	}
	int set = 1;
	char clkfr[16];
	switch (clk) {
	case 600:
	case 800:
	case 1000:
	case 1200:
	case 1400:
	case 1600:
		break;
	default:
		set = 0;
		break;
	}

	if (set) {
		cprintf("clock frequency adjusted from %d to %d, reboot needed\n", cclk, clk);
		sprintf(clkfr, "%d", clk);
		nvram_set("clkfreq", clkfr);
		nvram_commit();
		fprintf(stderr, "Overclocking done, rebooting...\n");
		sys_reboot();
	}
#endif
}

char *enable_dtag_vlan(int enable)
{
	int donothing = 0;

	nvram_set("fromvdsl", "1");
	if (nvram_match("vdsl_state", "1") && enable)
		donothing = 1;
	if ((nvram_match("vdsl_state", "0")
	     || nvram_match("vdsl_state", "")) && !enable)
		donothing = 1;
	if (enable)
		nvram_set("vdsl_state", "1");
	else
		nvram_set("vdsl_state", "0");

	char *eth = "eth0";
	char *lan_vlan = nvram_safe_get("lan_ifnames");
	char *wan_vlan = nvram_safe_get("wan_ifname");
	char *vlan_lan_ports = NULL;
	char *vlan_wan_ports = NULL;
	int lan_vlan_num = 0;
	int wan_vlan_num = 1;

	if (startswith(lan_vlan, "vlan0")) {
		lan_vlan_num = 0;
	} else if (startswith(lan_vlan, "vlan1")) {
		lan_vlan_num = 1;
	} else if (startswith(lan_vlan, "vlan2")) {
		lan_vlan_num = 2;
	} else
		return eth;

	if (startswith(wan_vlan, "vlan0")) {
		wan_vlan_num = 0;
	} else if (startswith(wan_vlan, "vlan1")) {
		wan_vlan_num = 1;
	} else if (startswith(wan_vlan, "vlan2")) {
		wan_vlan_num = 2;
	} else
		return eth;

	if (wan_vlan_num == lan_vlan_num)
		return eth;

	vlan_lan_ports = nvram_nget("vlan%dports", lan_vlan_num);
	vlan_wan_ports = nvram_nget("vlan%dports", wan_vlan_num);

	char *vlan7ports = "4t 5";;

	if (!strcmp(vlan_wan_ports, "4 5"))
		vlan7ports = "4t 5";
	else if (!strcmp(vlan_wan_ports, "4 5u"))
		vlan7ports = "4t 5u";
	else if (!strcmp(vlan_wan_ports, "0 5"))
		vlan7ports = "0t 5";
	else if (!strcmp(vlan_wan_ports, "0 5u"))
		vlan7ports = "0t 5u";
	else if (!strcmp(vlan_wan_ports, "1 5"))
		vlan7ports = "1t 5";
	else if (!strcmp(vlan_wan_ports, "4 8"))
		vlan7ports = "4t 8";
	else if (!strcmp(vlan_wan_ports, "4 8u"))
		vlan7ports = "4t 8";
	else if (!strcmp(vlan_wan_ports, "0 8"))
		vlan7ports = "0t 8";

	if (!donothing) {
		writevaproc("1", "/proc/switch/%s/reset", eth);
		writevaproc("1", "/proc/switch/%s/enable_vlan", eth);
		if (enable) {
			fprintf(stderr, "enable vlan port mapping %s/%s\n", vlan_lan_ports, vlan7ports);
			if (!nvram_match("dtag_vlan8", "1")
			    || nvram_match("wan_vdsl", "0")) {
				writevaproc(vlan_lan_ports, "/proc/switch/%s/vlan/%d/ports", eth, lan_vlan_num);
				start_setup_vlans();
				writevaproc(" ", "/proc/switch/%s/vlan/%d/ports", eth, wan_vlan_num);
				writevaproc(vlan7ports, "/proc/switch/%s/vlan/7/ports", eth);
			} else {
				writevaproc(vlan_lan_ports, "/proc/switch/%s/vlan/%d/ports", eth, lan_vlan_num);
				start_setup_vlans();
				writevaproc("", "/proc/switch/%s/vlan/%d/ports", eth, wan_vlan_num);
				writevaproc(vlan7ports, "/proc/switch/%s/vlan/7/ports", eth);
				writevaproc(vlan7ports, "/proc/switch/%s/vlan/8/ports", eth);
			}
		} else {
			fprintf(stderr, "disable vlan port mapping %s/%s\n", vlan_lan_ports, vlan_wan_ports);
			writevaproc(" ", "/proc/switch/%s/vlan/7/ports", eth);
			writevaproc(" ", "/proc/switch/%s/vlan/8/ports", eth);
			writevaproc(vlan_lan_ports, "/proc/switch/%s/vlan/%d/ports", eth, lan_vlan_num);
			writevaproc(vlan_wan_ports, "/proc/switch/%s/vlan/%d/ports", eth, wan_vlan_num);
			start_setup_vlans();
		}
	}
	nvram_set("fromvdsl", "0");
	return eth;
}

void start_dtag(void)
{
	enable_dtag_vlan(1);
}
