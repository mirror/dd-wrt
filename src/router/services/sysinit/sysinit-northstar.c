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
#include <services.h>
#include "devices/ethernet.c"
#include "devices/wireless.c"

#define sys_restart() eval("event","3","1","1")
#define sys_reboot() eval("sync"); eval("/bin/umount -a -r"); eval("event","3","1","15")

static void set_regulation(int card, char *code, char *rev)
{
	char path[32];

	sprintf(path, "wl%d_country_rev", card);
	if (nvram_match(path, ""))
		return;
	nvram_set(path, rev);
	sprintf(path, "wl%d_country_code", card);
	nvram_set(path, code);
	if (!card) {
		nvram_set("wl_country_rev", rev);
		nvram_set("wl_country_code", code);
	}

	switch (getRouterBrand()) {
	case ROUTER_NETGEAR_AC1450:
	case ROUTER_NETGEAR_R6250:
	case ROUTER_NETGEAR_R6300V2:
	case ROUTER_NETGEAR_R7000:
	case ROUTER_DLINK_DIR868:
	case ROUTER_DLINK_DIR860:
		sprintf(path, "pci/%d/1/regrev", card + 1);
		nvram_set(path, rev);
		sprintf(path, "pci/%d/1/ccode", card + 1);
		nvram_set(path, code);
		break;
	default:
		sprintf(path, "%d:regrev", card);
		nvram_set(path, rev);
		sprintf(path, "%d:ccode", card);
		nvram_set(path, code);
	}

}

void start_sysinit(void)
{
	char buf[PATH_MAX];
	struct stat tmp_stat;
	time_t tm = 0;
	struct nvram_param *extra_params = NULL;

//      insmod("softdog");
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

	if (nvram_get("et_txq_thresh") == NULL) {
		nvram_set("et_txq_thresh", "1024");
	}

	switch (getRouterBrand()) {
	case ROUTER_NETGEAR_AC1450:

		if (nvram_get("pci/1/1/venid") == NULL) {
			if (!sv_valid_hwaddr(nvram_safe_get("pci/1/1/macaddr"))
			    || startswith(nvram_safe_get("pci/1/1/macaddr"), "00:90:4C")
			    || !sv_valid_hwaddr(nvram_safe_get("pci/2/1/macaddr"))
			    || startswith(nvram_safe_get("pci/2/1/macaddr"), "00:90:4C")) {
				char mac[20];
				strcpy(mac, nvram_safe_get("et0macaddr"));
				MAC_ADD(mac);
				MAC_ADD(mac);
				nvram_set("pci/1/1/macaddr", mac);
				MAC_ADD(mac);
				nvram_set("pci/2/1/macaddr", mac);
			}
			struct nvram_param ac1450_pci_1_1_params[] = {
				{"pa2gw1a0", "0x191B"},
				{"pa2gw1a1", "0x18BC"},
				{"pa2gw1a2", "0x18B9"},
				{"ledbh12", "11"},
				{"legofdmbw202gpo", "0xCA862222"},
				{"ag0", "0"},
				{"ag1", "0"},
				{"ag2", "0"},
				{"legofdmbw20ul2gpo", "0xCA862222"},
				{"rxchain", "7"},
				{"cckbw202gpo", "0"},
				{"mcsbw20ul2gpo", "0xCA862222"},
				{"pa2gw0a0", "0xFE9D"},
				{"pa2gw0a1", "0xFE93"},
				{"pa2gw0a2", "0xFE77"},
				{"boardflags", "0x80003200"},
				{"tempoffset", "0"},
				{"boardvendor", "0x14e4"},
				{"triso2g", "3"},
				{"sromrev", "9"},
				{"extpagain2g", "1"},
				{"venid", "0x14e4"},
				{"maxp2ga0", "0x66"},
				{"maxp2ga1", "0x66"},
				{"maxp2ga2", "0x66"},
				{"boardflags2", "0x4100000"},
				{"tssipos2g", "1"},
				{"ledbh0", "11"},
				{"ledbh1", "11"},
				{"ledbh2", "11"},
				{"ledbh3", "11"},
				{"mcs32po", "0x8"},
				{"legofdm40duppo", "0x0"},
				{"antswctl2g", "0"},
				{"txchain", "7"},
				{"elna2g", "2"},
				{"antswitch", "0"},
				{"aa2g", "7"},
				{"cckbw20ul2gpo", "0"},
				{"leddc", "0xFFFF"},
				{"pa2gw2a0", "0xF9FA"},
				{"pa2gw2a1", "0xFA15"},
				{"pa2gw2a2", "0xF9DD"},
				{"xtalfreq", "20000"},
				{"ccode", "Q1"},
				{"pdetrange2g", "3"},
				{"regrev", "15"},
				{"devid", "0x4332"},
				{"tempthresh", "120"},
				{"mcsbw402gpo", "0xECA86222"},
				{"mcsbw202gpo", "0xCA862222"},
				{0, 0}
			};
			/*
			 * set router's extra parameters 
			 */
			extra_params = ac1450_pci_1_1_params;
			while (extra_params->name) {
				nvram_nset(extra_params->value, "pci/1/1/%s", extra_params->name);
				extra_params++;
			}

			struct nvram_param ac1450_pci_2_1_params[] = {
				{"rxgains5ghtrisoa0", "5"},
				{"rxgains5ghtrisoa1", "4"},
				{"rxgains5ghtrisoa2", "4"},
				{"mcslr5gmpo", "0"},
				{"txchain", "7"},
				{"phycal_tempdelta", "255"},
				{"pdgain5g", "4"},
				{"subband5gver", "0x4"},
				{"ccode", "Q1"},
				{"boardflags", "0x30000008"},
				{"tworangetssi5g", "0"},
				{"rxgains5gtrisoa0", "7"},
				{"sb20in40hrpo", "0"},
				{"rxgains5gtrisoa1", "6"},
				{"rxgains5gtrisoa2", "5"},
				{"tempoffset", "255"},
				{"mcsbw205gmpo", "0xECA86400"},
				{"noiselvl5ga0", "31,31,31,31"},
				{"noiselvl5ga1", "31,31,31,31"},
				{"noiselvl5ga2", "31,31,31,31"},
				{"xtalfreq", "65535"},
				{"devid", "0x43a2"},
				{"tempsense_option", "0x3"},
				{"femctrl", "3"},
				{"aa5g", "7"},
				{"pdoffset80ma0", "0"},
				{"pdoffset80ma1", "0"},
				{"pdoffset80ma2", "0"},
				{"papdcap5g", "0"},
				{"tssiposslope5g", "1"},
				{"tempcorrx", "0x3f"},
				{"mcslr5glpo", "0"},
				{"sar5g", "15"},
				{"pa5ga0", "0xFF28,0x19CC,0xFCB0,0xFF50,0x1AD0,0xFCE0,0xFF50,0x1B6F,0xFCD0,0xFF58,0x1BB9,0xFCD0"},
				{"pa5ga1", "0xFF36,0x1AAD,0xFCBD,0xFF50,0x1AF7,0xFCE0,0xFF50,0x1B5B,0xFCD8,0xFF58,0x1B8F,0xFCD0"},
				{"rxgains5gmelnagaina0", "2"},
				{"pa5ga2", "0xFF40,0x1A1F,0xFCDA,0xFF48,0x1A5D,0xFCE8,0xFF35,0x1A2D,0xFCCA,0xFF3E,0x1A2B,0xFCD0"},
				{"rxgains5gmelnagaina1", "2"},
				{"rxgains5gmelnagaina2", "3"},
				{"mcslr5ghpo", "0"},
				{"rxgainerr5ga0", "63,63,63,63"},
				{"rxgainerr5ga1", "31,31,31,31"},
				{"rxgainerr5ga2", "31,31,31,31"},
				{"pcieingress_war", "15"},
				{"pdoffset40ma0", "4369"},
				{"pdoffset40ma1", "4369"},
				{"pdoffset40ma2", "4369"},
				{"sb40and80lr5gmpo", "0"},
				{"rxgains5gelnagaina0", "1"},
				{"rxgains5gelnagaina1", "1"},
				{"rxgains5gelnagaina2", "1"},
				{"agbg0", "71"},
				{"mcsbw205glpo", "0xECA86400"},
				{"agbg1", "71"},
				{"agbg2", "133"},
				{"measpower1", "0x7f"},
				{"sb20in80and160lr5gmpo", "0"},
				{"measpower2", "0x7f"},
				{"temps_period", "15"},
				{"mcsbw805gmpo", "0xFEA86400"},
				{"dot11agduplrpo", "0"},
				{"mcsbw205ghpo", "0xECA86400"},
				{"measpower", "0x7f"},
				{"rxgains5ghelnagaina0", "2"},
				{"rxgains5ghelnagaina1", "2"},
				{"rxgains5ghelnagaina2", "3"},
				{"gainctrlsph", "0"},
				{"sb40and80hr5gmpo", "0"},
				{"sb20in80and160hr5gmpo", "0"},
				{"mcsbw1605gmpo", "0"},
				{"epagain5g", "0"},
				{"mcsbw405gmpo", "0xECA86400"},
				{"boardtype", "0x621"},
				{"rxchain", "7"},
				{"sb40and80lr5glpo", "0"},
				{"maxp5ga0", "102,102,102,102"},
				{"maxp5ga1", "102,102,102,102"},
				{"maxp5ga2", "102,102,102,102"},
				{"sb20in80and160lr5glpo", "0"},
				{"sb40and80lr5ghpo", "0"},
				{"venid", "0x14e4"},
				{"mcsbw805glpo", "0xFEA86400"},
				{"boardvendor", "0x14e4"},
				{"sb20in80and160lr5ghpo", "0"},
				{"tempsense_slope", "0xff"},
				{"mcsbw805ghpo", "0xFEA86400"},
				{"antswitch", "0"},
				{"aga0", "71"},
				{"aga1", "133"},
				{"rawtempsense", "0x1ff"},
				{"aga2", "133"},
				{"tempthresh", "255"},
				{"dot11agduphrpo", "0"},
				{"sb40and80hr5glpo", "0"},
				{"sromrev", "11"},
				{"boardnum", "20771"},
				{"sb20in40lrpo", "0"},
				{"sb20in80and160hr5glpo", "0"},
				{"mcsbw1605glpo", "0"},
				{"sb40and80hr5ghpo", "0"},
				{"mcsbw405glpo", "0xECA86400"},
				{"boardrev", "0x1402"},
				{"rxgains5gmtrisoa0", "5"},
				{"sb20in80and160hr5ghpo", "0"},
				{"mcsbw1605ghpo", "0"},
				{"rxgains5gmtrisoa1", "4"},
				{"rxgains5gmtrisoa2", "4"},
				{"rxgains5gmtrelnabypa0", "1"},
				{"rxgains5gmtrelnabypa1", "1"},
				{"rxgains5gmtrelnabypa2", "1"},
				{"mcsbw405ghpo", "0xECA86400"},
				{"boardflags2", "0x300002"},
				{"boardflags3", "0x0"},
				{"rxgains5ghtrelnabypa0", "1"},
				{"rxgains5ghtrelnabypa1", "1"},
				{"rxgains5ghtrelnabypa2", "1"},
				{"regrev", "27"},
				{"temps_hysteresis", "15"},
				{"rxgains5gtrelnabypa0", "1"},
				{"rxgains5gtrelnabypa1", "1"},
				{"rxgains5gtrelnabypa2", "1"},
				{0, 0}
			};

			/*
			 * set router's extra parameters 
			 */
			extra_params = ac1450_pci_2_1_params;
			while (extra_params->name) {
				nvram_nset(extra_params->value, "pci/2/1/%s", extra_params->name);
				extra_params++;
			}
			nvram_set("pci/1/1/venid", "0x14E4");
			nvram_commit();
		}
		nvram_unset("et1macaddr");
		set_gpio(6, 1);	//reset button
		set_gpio(2, 0);	//power led
		set_gpio(3, 1);	//power led
		set_gpio(1, 1);	//logo
		set_gpio(0, 1);
		set_gpio(4, 1);	//ses
		set_gpio(5, 1);	//wifi
		break;
	case ROUTER_NETGEAR_R6250:

		if (nvram_get("pci/1/1/venid") == NULL) {
			if (!sv_valid_hwaddr(nvram_safe_get("pci/1/1/macaddr"))
			    || startswith(nvram_safe_get("pci/1/1/macaddr"), "00:90:4C")
			    || !sv_valid_hwaddr(nvram_safe_get("pci/2/1/macaddr"))
			    || startswith(nvram_safe_get("pci/2/1/macaddr"), "00:90:4C")) {
				char mac[20];
				strcpy(mac, nvram_safe_get("et0macaddr"));
				MAC_ADD(mac);
				MAC_ADD(mac);
				nvram_set("pci/1/1/macaddr", mac);
				MAC_ADD(mac);
				nvram_set("pci/2/1/macaddr", mac);
			}
			struct nvram_param r6250_pci_1_1_params[] = {
				{"pa2gw1a0", "0x1870"},
				{"pa2gw1a1", "0x1870"},
				{"ledbh12", "11"},
				{"ag0", "2"},
				{"ag1", "2"},
				{"ag2", "255"},
				{"rxchain", "3"},
				{"bw402gpo", "0x1"},
				{"pa2gw0a0", "0xff15"},
				{"pa2gw0a1", "0xff15"},
				{"boardflags", "0x80001200"},
				{"tempoffset", "0"},
				{"ofdm5gpo", "0"},
				{"boardvendor", "0x14e4"},
				{"triso2g", "4"},
				{"sromrev", "8"},
				{"extpagain2g", "3"},
				{"venid", "0x14e4"},
				{"maxp2ga0", "0x66"},
				{"maxp2ga1", "0x66"},
				{"boardtype", "0x62b"},
				{"boardflags2", "0x9800"},
				{"tssipos2g", "1"},
				{"ofdm2gpo", "0xA8640000"},
				{"ledbh0", "255"},
				{"ledbh1", "255"},
				{"ledbh2", "255"},
				{"ledbh3", "131"},
				{"mcs2gpo0", "0x4000"},
				{"mcs2gpo1", "0xCA86"},
				{"mcs2gpo2", "0x4000"},
				{"mcs2gpo3", "0xCA86"},
				{"mcs2gpo4", "0x7422"},
				{"antswctl2g", "0"},
				{"mcs2gpo5", "0xEDB9"},
				{"mcs2gpo6", "0x7422"},
				{"mcs2gpo7", "0xEDB9"},
				{"stbc2gpo", "0"},
				{"txchain", "3"},
				{"elna2g", "2"},
				{"ofdm5glpo", "0"},
				{"antswitch", "0"},
				{"aa2g", "3"},
				{"ccd2gpo", "0"},
				{"ofdm5ghpo", "0"},
				{"leddc", "65535"},
				{"pa2gw2a0", "0xfad3"},
				{"pa2gw2a1", "0xfad3"},
				{"opo", "68"},
				{"ccode", "EU"},
				{"pdetrange2g", "3"},
				{"regrev", "22"},
				{"devid", "0x43a9"},
				{"tempthresh", "120"},
				{"cck2gpo", "0"},

				{0, 0}
			};
			/*
			 * set router's extra parameters 
			 */
			extra_params = r6250_pci_1_1_params;
			while (extra_params->name) {
				nvram_nset(extra_params->value, "pci/1/1/%s", extra_params->name);
				extra_params++;
			}

			struct nvram_param r6250_pci_2_1_params[] = {
				{"rxgains5ghtrisoa0", "15"},
				{"rxgains5ghtrisoa1", "15"},
				{"rxgains5ghtrisoa2", "15"},
				{"mcslr5gmpo", "0"},
				{"txchain", "7"},
				{"phycal_tempdelta", "255"},
				{"pdgain5g", "10"},
				{"subband5gver", "0x4"},
				{"ccode", "Q1"},
				{"boardflags", "0x10001000"},
				{"tworangetssi5g", "0"},
				{"rxgains5gtrisoa0", "6"},
				{"sb20in40hrpo", "0"},
				{"rxgains5gtrisoa1", "6"},
				{"rxgains5gtrisoa2", "6"},
				{"tempoffset", "255"},
				{"mcsbw205gmpo", "0xEC200000"},
				{"noiselvl5ga0", "31,31,31,31"},
				{"noiselvl5ga1", "31,31,31,31"},
				{"noiselvl5ga2", "31,31,31,31"},
				{"xtalfreq", "65535"},
				{"tempsense_option", "0x3"},
				{"devid", "0x43a2"},
				{"femctrl", "6"},
				{"aa5g", "7"},
				{"pdoffset80ma0", "256"},
				{"pdoffset80ma1", "256"},
				{"cckbw20ul2gpo", "0"},
				{"pdoffset80ma2", "256"},
				{"papdcap5g", "0"},
				{"tssiposslope5g", "1"},
				{"tempcorrx", "0x3f"},
				{"mcslr5glpo", "0"},
				{"mcsbw402gpo", "2571386880"},
				{"sar5g", "15"},
				{"rxgains5gmelnagaina0", "7"},
				{"rxgains5gmelnagaina1", "7"},
				{"rxgains5gmelnagaina2", "7"},
				{"mcslr5ghpo", "0"},
				{"rxgainerr5ga0", "63,63,63,63"},
				{"mcsbw202gpo", "2571386880"},
				{"rxgainerr5ga1", "31,31,31,31"},
				{"rxgainerr5ga2", "31,31,31,31"},
				{"pcieingress_war", "15"},
				{"pdoffset40ma0", "12834"},
				{"pdoffset40ma1", "12834"},
				{"pdoffset40ma2", "12834"},
				{"sb40and80lr5gmpo", "0"},
				{"rxgains5gelnagaina0", "3"},
				{"rxgains5gelnagaina1", "3"},
				{"rxgains5gelnagaina2", "3"},
				{"mcsbw205glpo", "0xEC200000"},
				{"measpower1", "0x7f"},
				{"sb20in80and160lr5gmpo", "0"},
				{"measpower2", "0x7f"},
				{"temps_period", "15"},
				{"mcsbw805gmpo", "0xFDA86420"},
				{"dot11agduplrpo", "0"},
				{"mcsbw205ghpo", "0xFC652000"},
				{"measpower", "0x7f"},
				{"rxgains5ghelnagaina0", "7"},
				{"ofdmlrbw202gpo", "0"},
				{"rxgains5ghelnagaina1", "7"},
				{"rxgains5ghelnagaina2", "7"},
				{"gainctrlsph", "0"},
				{"sb40and80hr5gmpo", "0"},
				{"sb20in80and160hr5gmpo", "0"},
				{"mcsbw1605gmpo", "0"},
				{"epagain5g", "0"},
				{"mcsbw405gmpo", "0xEC300000"},
				{"cckbw202gpo", "0"},
				{"rxchain", "7"},
				{"sb40and80lr5glpo", "0"},
				{"sb20in80and160lr5glpo", "0"},
				{"sb40and80lr5ghpo", "0"},
				{"venid", "0x14e4"},
				{"mcsbw805glpo", "0xFCA86400"},
				{"boardvendor", "0x14e4"},
				{"sb20in80and160lr5ghpo", "0"},
				{"tempsense_slope", "0xff"},
				{"ofdm5glpo", "0"},
				{"mcsbw805ghpo", "0xFDA86420"},
				{"antswitch", "0"},
				{"aga0", "71"},
				{"aga1", "133"},
				{"rawtempsense", "0x1ff"},
				{"aga2", "133"},
				{"tempthresh", "255"},
				{"dot11agduphrpo", "0"},
				{"ofdm5ghpo", "0xB975300"},
				{"sb40and80hr5glpo", "0"},
				{"sromrev", "11"},
				{"sb20in40lrpo", "0"},
				{"sb20in80and160hr5glpo", "0"},
				{"mcsbw1605glpo", "0"},
				{"sb40and80hr5ghpo", "0"},
				{"mcsbw405glpo", "0xEC30000"},
				{"dot11agofdmhrbw202gpo", "17408"},
				{"rxgains5gmtrisoa0", "15"},
				{"sb20in80and160hr5ghpo", "0"},
				{"mcsbw1605ghpo", "0"},
				{"rxgains5gmtrisoa1", "15"},
				{"rxgains5gmtrisoa2", "15"},
				{"rxgains5gmtrelnabypa0", "1"},
				{"rxgains5gmtrelnabypa1", "1"},
				{"rxgains5gmtrelnabypa2", "1"},
				{"mcsbw405ghpo", "0xFC764100"},
				{"boardflags2", "0x2"},
				{"boardflags3", "0x0"},
				{"rxgains5ghtrelnabypa0", "1"},
				{"rxgains5ghtrelnabypa1", "1"},
				{"rxgains5ghtrelnabypa2", "1"},
				{"regrev", "27"},
				{"temps_hysteresis", "15"},
				{"rxgains5gtrelnabypa0", "1"},
				{"rxgains5gtrelnabypa1", "1"},
				{"rxgains5gtrelnabypa2", "1"},
				{"pa5ga0", "0xff7a,0x16a9,0xfd4b,0xff6e,0x1691,0xfd47,0xff7e,0x17b8,0xfd37,0xff82,0x17fb,0xfd3a"},
				{"pa5ga1", "0xff66,0x1519,0xfd65,0xff72,0x15ff,0xfd56,0xff7f,0x16ee,0xfd4b,0xffad,0x174b,0xfd81"},
				{"pa5ga2", "0xff76,0x168e,0xfd50,0xff75,0x16d0,0xfd4b,0xff86,0x17fe,0xfd39,0xff7e,0x1810,0xfd31"},
				{"maxp5ga0", "72,72,94,94"},
				{"maxp5ga1", "72,72,94,94"},
				{"maxp5ga2", "72,72,94,94"},
				{0, 0}
			};

			/*
			 * set router's extra parameters 
			 */
			extra_params = r6250_pci_2_1_params;
			while (extra_params->name) {
				nvram_nset(extra_params->value, "pci/2/1/%s", extra_params->name);
				extra_params++;
			}
			nvram_set("pci/1/1/venid", "0x14E4");
			nvram_commit();
		}
		nvram_unset("et1macaddr");
		set_gpio(6, 1);	//reset button
		set_gpio(2, 0);	//power led
		set_gpio(3, 1);	//power led
		set_gpio(1, 1);	//logo
		set_gpio(0, 1);
		set_gpio(4, 1);	//ses
		set_gpio(5, 1);	//wifi
		break;
	case ROUTER_NETGEAR_R6300V2:

		if (nvram_get("pci/1/1/venid") == NULL) {
			if (!sv_valid_hwaddr(nvram_safe_get("pci/1/1/macaddr"))
			    || startswith(nvram_safe_get("pci/1/1/macaddr"), "00:90:4C")
			    || !sv_valid_hwaddr(nvram_safe_get("pci/2/1/macaddr"))
			    || startswith(nvram_safe_get("pci/2/1/macaddr"), "00:90:4C")) {
				char mac[20];
				strcpy(mac, nvram_safe_get("et0macaddr"));
				MAC_ADD(mac);
				MAC_ADD(mac);
				nvram_set("pci/1/1/macaddr", mac);
				MAC_ADD(mac);
				nvram_set("pci/2/1/macaddr", mac);
			}
			struct nvram_param r6300v2_pci_1_1_params[] = {
				{"pa2gw1a0", "0x1A37"},
				{"pa2gw1a1", "0x1A0B"},
				{"pa2gw1a2", "0x19F2"},
				{"ledbh12", "11"},
				{"legofdmbw202gpo", "0xCA862222"},
				{"ag0", "0"},
				{"ag1", "0"},
				{"ag2", "0"},
				{"legofdmbw20ul2gpo", "0xCA862222"},
				{"rxchain", "7"},
				{"cckbw202gpo", "0"},
				{"mcsbw20ul2gpo", "0xCA862222"},
				{"pa2gw0a0", "0xFEB4"},
				{"pa2gw0a1", "0xFEBC"},
				{"pa2gw0a2", "0xFEA9"},
				{"boardflags", "0x80003200"},
				{"tempoffset", "0"},
				{"boardvendor", "0x14e4"},
				{"triso2g", "3"},
				{"sromrev", "9"},
				{"extpagain2g", "1"},
				{"venid", "0x14e4"},
				{"maxp2ga0", "0x66"},
				{"maxp2ga1", "0x66"},
				{"maxp2ga2", "0x66"},
				{"boardflags2", "0x4100000"},
				{"tssipos2g", "1"},
				{"ledbh0", "11"},
				//      {"ledbh1", "11"},
				{"ledbh2", "14"},
				{"ledbh3", "1"},
				{"mcs32po", "0x8"},
				{"legofdm40duppo", "0x0"},
				{"antswctl2g", "0"},
				{"txchain", "7"},
				{"elna2g", "2"},
				{"aa2g", "7"},
				{"antswitch", "0"},
				{"cckbw20ul2gpo", "0"},
				{"leddc", "0xFFFF"},
				{"pa2gw2a0", "0xF9EE"},
				{"pa2gw2a1", "0xFA05"},
				{"pa2gw2a2", "0xFA03"},
				{"xtalfreq", "20000"},
				{"ccode", "Q2"},
				{"pdetrange2g", "3"},
				{"regrev", "12"},
				{"devid", "0x4332"},
				{"tempthresh", "120"},
				{"mcsbw402gpo", "0xECA86222"},
				{"mcsbw202gpo", "0xCA862222"},
				{0, 0}
			};
			/*
			 * set router's extra parameters 
			 */
			extra_params = r6300v2_pci_1_1_params;
			while (extra_params->name) {
				nvram_nset(extra_params->value, "pci/1/1/%s", extra_params->name);
				extra_params++;
			}

			struct nvram_param r6300v2_pci_2_1_params[] = {
				{"rxgains5ghtrisoa0", "5"},
				{"rxgains5ghtrisoa1", "4"},
				{"rxgains5ghtrisoa2", "4"},
				{"txchain", "7"},
				{"mcslr5gmpo", "0"},
				{"phycal_tempdelta", "255"},
				{"pdgain5g", "4"},
				{"subband5gver", "0x4"},
				{"ccode", "Q2"},
				{"boardflags", "0x30000000"},
				{"tworangetssi5g", "0"},
				{"sb20in40hrpo", "0"},
				{"rxgains5gtrisoa0", "7"},
				{"rxgains5gtrisoa1", "6"},
				{"tempoffset", "255"},
				{"rxgains5gtrisoa2", "5"},
				{"mcsbw205gmpo", "0xECA86400"},
				{"noiselvl5ga0", "31,31,31,31"},
				{"noiselvl5ga1", "31,31,31,31"},
				{"noiselvl5ga2", "31,31,31,31"},
				{"xtalfreq", "40000"},
				{"tempsense_option", "0x3"},
				{"devid", "0x43a2"},
				{"femctrl", "3"},
				{"aa5g", "7"},
				{"pdoffset80ma0", "0"},
				{"pdoffset80ma1", "0"},
				{"papdcap5g", "0"},
				{"pdoffset80ma2", "0"},
				{"tssiposslope5g", "1"},
				{"tempcorrx", "0x3f"},
				{"mcslr5glpo", "0"},
				{"sar5g", "15"},
				//      {"macaddr", "04:A1:51:10:CF:61"},
				{"pa5ga0", "0xFF39,0x1A55,0xFCC7,0xFF50,0x1AD0,0xFCE0,0xFF50,0x1B6F,0xFCD0,0xFF58,0x1BB9,0xFCD0"},
				{"rxgains5gmelnagaina0", "2"},
				{"pa5ga1", "0xFF36,0x1AAD,0xFCBD,0xFF50,0x1AF7,0xFCE0,0xFF50,0x1B5B,0xFCD8,0xFF58,0x1B8F,0xFCD0"},
				{"rxgains5gmelnagaina1", "2"},
				{"pa5ga2", "0xFF40,0x1A1F,0xFCDA,0xFF48,0x1A5D,0xFCE8,0xFF35,0x1A2D,0xFCCA,0xFF3E,0x1A2B,0xFCD0"},
				{"rxgains5gmelnagaina2", "3"},
				{"mcslr5ghpo", "0"},
				{"rxgainerr5ga0", "63,63,63,63"},
				{"rxgainerr5ga1", "31,31,31,31"},
				{"rxgainerr5ga2", "31,31,31,31"},
				{"pdoffset40ma0", "4369"},
				{"pcieingress_war", "15"},
				{"pdoffset40ma1", "4369"},
				{"pdoffset40ma2", "4369"},
				{"sb40and80lr5gmpo", "0"},
				{"rxgains5gelnagaina0", "1"},
				{"rxgains5gelnagaina1", "1"},
				{"rxgains5gelnagaina2", "1"},
				{"agbg0", "71"},
				{"agbg1", "71"},
				{"mcsbw205glpo", "0xECA86400"},
				{"agbg2", "133"},
				{"measpower1", "0x7f"},
				{"measpower2", "0x7f"},
				{"sb20in80and160lr5gmpo", "0"},
				{"temps_period", "15"},
				{"mcsbw805gmpo", "0xFEA86400"},
				{"dot11agduplrpo", "0"},
				{"mcsbw205ghpo", "0xECA86400"},
				{"measpower", "0x7f"},
				{"rxgains5ghelnagaina0", "2"},
				{"rxgains5ghelnagaina1", "2"},
				{"rxgains5ghelnagaina2", "3"},
				{"gainctrlsph", "0"},
				{"sb40and80hr5gmpo", "0"},
				{"mcsbw1605gmpo", "0"},
				{"sb20in80and160hr5gmpo", "0"},
				{"epagain5g", "0"},
				{"mcsbw405gmpo", "0xECA86400"},
				{"boardtype", "0x621"},
				{"rxchain", "7"},
				{"sb40and80lr5glpo", "0"},
				{"maxp5ga0", "102,102,102,102"},
				{"maxp5ga1", "102,102,102,102"},
				{"maxp5ga2", "102,102,102,102"},
				{"sb20in80and160lr5glpo", "0"},
				{"sb40and80lr5ghpo", "0"},
				{"venid", "0x14e4"},
				{"mcsbw805glpo", "0xFEA86400"},
				{"boardvendor", "0x14e4"},
				{"tempsense_slope", "0xff"},
				{"sb20in80and160lr5ghpo", "0"},
				{"antswitch", "0"},
				{"mcsbw805ghpo", "0xFEA86400"},
				{"aga0", "71"},
				{"aga1", "133"},
				{"aga2", "133"},
				{"rawtempsense", "0x1ff"},
				{"tempthresh", "255"},
				{"dot11agduphrpo", "0"},
				{"sb40and80hr5glpo", "0"},
				{"sromrev", "11"},
				{"boardnum", "20771"},
				{"sb20in40lrpo", "0"},
				{"mcsbw1605glpo", "0"},
				{"sb20in80and160hr5glpo", "0"},
				{"sb40and80hr5ghpo", "0"},
				{"mcsbw405glpo", "0xECA86400"},
				{"boardrev", "0x1402"},
				{"mcsbw1605ghpo", "0"},
				{"sb20in80and160hr5ghpo", "0"},
				{"rxgains5gmtrisoa0", "5"},
				{"rxgains5gmtrisoa1", "4"},
				{"rxgains5gmtrisoa2", "4"},
				{"rxgains5gmtrelnabypa0", "1"},
				{"rxgains5gmtrelnabypa1", "1"},
				{"mcsbw405ghpo", "0xECA86400"},
				{"rxgains5gmtrelnabypa2", "1"},
				{"boardflags2", "0x300002"},
				{"boardflags3", "0x0"},
				{"rxgains5ghtrelnabypa0", "1"},
				{"rxgains5ghtrelnabypa1", "1"},
				{"regrev", "12"},
				{"rxgains5ghtrelnabypa2", "1"},
				{"temps_hysteresis", "15"},
				{"rxgains5gtrelnabypa0", "1"},
				{"rxgains5gtrelnabypa1", "1"},
				{"rxgains5gtrelnabypa2", "1"},
				{0, 0}
			};

			/*
			 * set router's extra parameters 
			 */
			extra_params = r6300v2_pci_2_1_params;
			while (extra_params->name) {
				nvram_nset(extra_params->value, "pci/2/1/%s", extra_params->name);
				extra_params++;
			}
			nvram_set("pci/1/1/venid", "0x14E4");
			nvram_commit();
		}
		nvram_unset("et1macaddr");
		set_gpio(6, 1);	//reset button
		set_gpio(2, 0);	//power led
		set_gpio(3, 1);	//power led
		set_gpio(1, 1);	//logo
		set_gpio(0, 1);
		set_gpio(4, 1);	//ses
		set_gpio(5, 1);	//wifi
		break;
	case ROUTER_NETGEAR_R7000:

		if (nvram_get("pci/1/1/venid") == NULL) {
			if (!sv_valid_hwaddr(nvram_safe_get("pci/1/1/macaddr"))
			    || startswith(nvram_safe_get("pci/1/1/macaddr"), "00:90:4C")
			    || !sv_valid_hwaddr(nvram_safe_get("pci/2/1/macaddr"))
			    || startswith(nvram_safe_get("pci/2/1/macaddr"), "00:90:4C")) {
				char mac[20];
				strcpy(mac, nvram_safe_get("et0macaddr"));
				MAC_ADD(mac);
				MAC_ADD(mac);
				nvram_set("pci/1/1/macaddr", mac);
				MAC_ADD(mac);
				nvram_set("pci/2/1/macaddr", mac);
			}
			struct nvram_param r7000_pci_1_1_params[] = {
				{"pdoffset2g40ma0", "15"},
				{"pdoffset2g40ma1", "15"},
				{"pdoffset2g40ma2", "15"},
				{"rxgains2gtrisoa0", "7"},
				{"rxgains2gtrisoa1", "7"},
				{"rxgains2gtrisoa2", "7"},
				{"rxgainerr2ga0", "63"},
				{"rxgainerr2ga1", "31"},
				{"rxgainerr2ga2", "31"},
				{"pdoffset2g40mvalid", "1"},
				{"agbg0", "0"},
				{"agbg1", "0"},
				{"epagain2g", "0"},
				{"agbg2", "0"},
				{"gainctrlsph", "0"},
				{"cckbw202gpo", "0"},
				{"pdgain2g", "14"},
				{"boardflags", "0x1000"},
				{"tssifloor2g", "0x3ff"},
				{"subband5gver", "0x4"},
				{"boardnum", "57359"},
				{"dot11agduplrpo", "0"},
				{"measpower", "0x7f"},
				{"sromrev", "11"},
				{"ofdmlrbw202gpo", "0"},
				{"boardrev", "0x1150"},
				{"dot11agofdmhrbw202gpo", "0xCA86"},
				{"rxgains2gtrelnabypa0", "1"},
				{"rxgains2gtrelnabypa1", "1"},
				{"rpcal2g", "0x3ef"},
				{"rxgains2gtrelnabypa2", "1"},
				{"maxp2ga0", "106"},
				{"maxp2ga1", "106"},
				{"maxp2ga2", "106"},
				{"boardtype", "0x661"},
				{"pa2ga0", "0xFF32,0x1C30,0xFCA3"},
				{"pa2ga1", "0xFF35,0x1BE3,0xFCB0"},
				{"pa2ga2", "0xFF33,0x1BE1,0xFCB0"},
				{"boardflags2", "0x100002"},
				{"boardflags3", "0x10000003"},
				{"measpower1", "0x7f"},
				{"measpower2", "0x7f"},
				{"subvid", "0x14e4"},
				{"rxgains2gelnagaina0", "3"},
				{"rxgains2gelnagaina1", "3"},
				{"rxgains2gelnagaina2", "3"},
				{"antswitch", "0"},
				{"aa2g", "7"},
				{"sar2g", "18"},
				{"noiselvl2ga0", "31"},
				{"noiselvl2ga1", "31"},
				{"noiselvl2ga2", "31"},
				{"tworangetssi2g", "0"},
				{"dot11agduphrpo", "0"},
				{"pdoffset80ma0", "0"},
				{"pdoffset80ma1", "0"},
				{"cckbw20ul2gpo", "0"},
				{"pdoffset80ma2", "0"},
				{"xtalfreq", "65535"},
				{"papdcap2g", "0"},
				{"femctrl", "3"},
				{"tssiposslope2g", "1"},
				{"ccode", "Q2"},
				{"pdoffset40ma0", "0"},
				{"pdoffset40ma1", "0"},
				{"pdoffset40ma2", "0"},
				{"regrev", "53"},
				{"devid", "0x43a1"},
				{"mcsbw402gpo", "0xA976A600"},
				{"mcsbw202gpo", "0xA976A600"},
				{0, 0}
			};
			/*
			 * set router's extra parameters 
			 */
			extra_params = r7000_pci_1_1_params;
			while (extra_params->name) {
				nvram_nset(extra_params->value, "pci/1/1/%s", extra_params->name);
				extra_params++;
			}

			struct nvram_param r7000_pci_2_1_params[] = {
				{"rxgains5ghtrisoa0", "5"},
				{"rxgains5ghtrisoa1", "4"},
				{"rxgains5ghtrisoa2", "4"},
				{"mcslr5gmpo", "0"},
				{"txchain", "7"},
				{"phycal_tempdelta", "255"},
				{"pdgain5g", "4"},
				{"tssifloor5g", "0x3ff,0x3ff,0x3ff,0x3ff"},
				{"subband5gver", "0x4"},
				{"ccode", "Q2"},
				{"boardflags", "0x30000000"},
				{"tworangetssi5g", "0"},
				{"rxgains5gtrisoa0", "7"},
				{"sb20in40hrpo", "0"},
				{"rxgains5gtrisoa1", "6"},
				{"rxgains5gtrisoa2", "5"},
				{"tempoffset", "255"},
				{"mcsbw205gmpo", "0xBA768600"},
				{"noiselvl5ga0", "31,31,31,31"},
				{"noiselvl5ga1", "31,31,31,31"},
				{"noiselvl5ga2", "31,31,31,31"},
				{"xtalfreq", "65535"},
				{"tempsense_option", "0x3"},
				{"devid", "0x43a2"},
				{"femctrl", "3"},
				{"epagain2g", "0"},
				{"aa5g", "0"},
				{"rxgains2gelnagaina0", "0"},
				{"rxgains2gelnagaina1", "0"},
				{"pdoffset80ma0", "0"},
				{"rxgains2gelnagaina2", "0"},
				{"pdoffset80ma1", "0"},
				{"cckbw20ul2gpo", "0"},
				{"pdoffset80ma2", "0"},
				{"papdcap5g", "0"},
				{"tssiposslope5g", "1"},
				{"tempcorrx", "0x3f"},
				{"mcslr5glpo", "0"},
				{"mcsbw402gpo", "0"},
				{"pdoffset2g40ma0", "15"},
				{"sar5g", "15"},
				{"pdoffset2g40ma1", "15"},
				{"pdoffset2g40ma2", "15"},
				{"pa5ga0", "0xFF4C,0x1808,0xFD1B,0xFF4C,0x18CF,0xFD0C,0xFF4A,0x1920,0xFD08,0xFF4C,0x1949,0xFCF6"},
				{"pa5ga1", "0xFF4A,0x18AC,0xFD0B,0xFF44,0x1904,0xFCFF,0xFF56,0x1A09,0xFCFC,0xFF4F,0x19AB,0xFCEF"},
				{"rxgains5gmelnagaina0", "3"},
				{"pa5ga2", "0xFF4C,0x1896,0xFD11,0xFF43,0x192D,0xFCF5,0xFF50,0x19EE,0xFCF1,0xFF52,0x19C6,0xFCF1"},
				{"rxgains5gmelnagaina1", "4"},
				{"rxgains5gmelnagaina2", "4"},
				{"mcsbw1605hpo", "0"},
				{"mcslr5ghpo", "0"},
				{"rxgainerr5ga0", "63,63,63,63"},
				{"mcsbw202gpo", "0"},
				{"rxgainerr5ga1", "31,31,31,31"},
				{"rxgainerr5ga2", "31,31,31,31"},
				{"rxgains2gtrisoa0", "0"},
				{"rxgains2gtrisoa1", "0"},
				{"pdoffset40ma0", "4369"},
				{"rxgains2gtrisoa2", "0"},
				{"pdoffset40ma1", "4369"},
				{"pdoffset40ma2", "4369"},
				{"sb40and80lr5gmpo", "0"},
				{"rxgains5gelnagaina0", "4"},
				{"rxgains5gelnagaina1", "4"},
				{"noiselvl2ga0", "31"},
				{"rxgains5gelnagaina2", "4"},
				{"noiselvl2ga1", "31"},
				{"noiselvl2ga2", "31"},
				{"agbg0", "0"},
				{"mcsbw205glpo", "0xBA768600"},
				{"agbg1", "0"},
				{"agbg2", "0"},
				{"measpower1", "0x7f"},
				{"sb20in80and160lr5gmpo", "0"},
				{"measpower2", "0x7f"},
				{"temps_period", "15"},
				{"mcsbw805gmpo", "0xBA768600"},
				{"dot11agduplrpo", "0"},
				{"mcsbw205ghpo", "0xBA768600"},
				{"measpower", "0x7f"},
				{"rxgains5ghelnagaina0", "3"},
				{"ofdmlrbw202gpo", "0"},
				{"rxgains5ghelnagaina1", "3"},
				{"rxgains5ghelnagaina2", "4"},
				{"gainctrlsph", "0"},
				{"sb40and80hr5gmpo", "0"},
				{"sb20in80and160hr5gmpo", "0"},
				{"mcsbw1605gmpo", "0"},
				{"pa2ga0", "0xfe72,0x14c0,0xfac7"},
				{"pa2ga1", "0xfe80,0x1472,0xfabc"},
				{"pa2ga2", "0xfe82,0x14bf,0xfad9"},
				{"epagain5g", "0"},
				{"mcsbw405gmpo", "0xBA768600"},
				{"rxgainerr2ga0", "63"},
				{"boardtype", "0x621"},
				{"rxgainerr2ga1", "31"},
				{"rxgainerr2ga2", "31"},
				{"cckbw202gpo", "0"},
				{"rxchain", "7"},
				{"sb40and80lr5glpo", "0"},
				{"maxp5ga0", "106,106,106,106"},
				{"maxp5ga1", "106,106,106,106"},
				{"maxp5ga2", "106,106,106,106"},
				{"subvid", "0x14e4"},
				{"sb20in80and160lr5glpo", "0"},
				{"sb40and80lr5ghpo", "0"},
				{"mcsbw805glpo", "0xBA768600"},
				{"pdgain2g", "4"},
				{"sb20in80and160lr5ghpo", "0"},
				{"tssifloor2g", "0x3ff"},
				{"tempsense_slope", "0xff"},
				{"mcsbw805ghpo", "0xBA768600"},
				{"antswitch", "0"},
				{"aga0", "0"},
				{"aga1", "0"},
				{"rawtempsense", "0x1ff"},
				{"aga2", "0"},
				{"tempthresh", "255"},
				{"tworangetssi2g", "0"},
				{"dot11agduphrpo", "0"},
				{"sb40and80hr5glpo", "0"},
				{"sromrev", "11"},
				{"boardnum", "20507"},
				{"sb20in40lrpo", "0"},
				{"rxgains2gtrelnabypa0", "0"},
				{"rxgains2gtrelnabypa1", "0"},
				{"sb20in80and160hr5glpo", "0"},
				{"mcsbw1605glpo", "0"},
				{"rxgains2gtrelnabypa2", "0"},
				{"sb40and80hr5ghpo", "0"},
				{"mcsbw405glpo", "0xBA768600"},
				{"rpcal2g", "0"},
				{"dot11agofdmhrbw202gpo", "0"},
				{"aa2g", "7"},
				{"boardrev", "0x1451"},
				{"mcsbw1605ghpo", "0"},
				{"rxgains5gmtrisoa0", "5"},
				{"sb20in80and160hr5ghpo", "0"},
				{"rxgains5gmtrisoa1", "4"},
				{"rxgains5gmtrisoa2", "4"},
				{"rxgains5gmtrelnabypa0", "1"},
				{"papdcap2g", "0"},
				{"rxgains5gmtrelnabypa1", "1"},
				{"rxgains5gmtrelnabypa2", "1"},
				{"mcsbw405ghpo", "0xBA768600"},
				{"tssiposslope2g", "1"},
				{"maxp2ga0", "76"},
				{"maxp2ga1", "76"},
				{"maxp2ga2", "76"},
				{"boardflags2", "0x300002"},
				{"boardflags3", "0x10000000"},
				{"rxgains5ghtrelnabypa0", "1"},
				{"rxgains5ghtrelnabypa1", "1"},
				{"rxgains5ghtrelnabypa2", "1"},
				{"regrev", "53"},
				{"rpcal5gb0", "0x7005"},
				{"rpcal5gb1", "0x8403"},
				{"rpcal5gb2", "0x6ff9"},
				{"sar2g", "18"},
				{"rpcal5gb3", "0x8509"},
				{"temps_hysteresis", "15"},
				{"rxgains5gtrelnabypa0", "1"},
				{"rxgains5gtrelnabypa1", "1"},
				{"rxgains5gtrelnabypa2", "1"},
				{"pdoffset2g40mvalid", "1"},
				{0, 0}
			};

			/*
			 * set router's extra parameters 
			 */
			extra_params = r7000_pci_2_1_params;
			while (extra_params->name) {
				nvram_nset(extra_params->value, "pci/2/1/%s", extra_params->name);
				extra_params++;
			}
			nvram_set("wl_pcie_mrrs", "128");
			nvram_set("wl0_pcie_mrrs", "128");
			nvram_set("wl1_pcie_mrrs", "128");
			nvram_set("pci/1/1/venid", "0x14E4");
			nvram_commit();
		}
		nvram_unset("et1macaddr");
		set_gpio(15, 1);	//wlan button led on
		set_gpio(4, 1);
		set_gpio(9, 1);
		set_gpio(5, 1);
		set_gpio(6, 1);	//reset button
		break;
	case ROUTER_NETGEAR_EX6200:
		nvram_set("vlan2hwname", "et0");
		if (nvram_get("0:venid") == NULL) {
			if (!sv_valid_hwaddr(nvram_safe_get("0:macaddr"))
			    || startswith(nvram_safe_get("0:macaddr"), "00:90:4C")
			    || !sv_valid_hwaddr(nvram_safe_get("1:macaddr"))
			    || startswith(nvram_safe_get("1:macaddr"), "00:90:4C")) {
				char mac[20];
				strcpy(mac, nvram_safe_get("et0macaddr"));
				MAC_ADD(mac);
				MAC_ADD(mac);
				nvram_set("0:macaddr", mac);
				MAC_ADD(mac);
				nvram_set("1:macaddr", mac);
			}
			struct nvram_param ex6200_0params[] = {
				{"ag1", "2"},
				{"cck2gpo", "0"},
				{"ag2", "255"},
				{"ledbh0", "11"},
				{"ledbh1", "11"},
				{"ledbh2", "11"},
				{"ledbh3", "11"},
				{"venid", "0x14e4"},
				{"aa2g", "3"},
				{"ledbh12", "11"},
				{"pdetrange2g", "3"},
				{"pa2gw1a0", "0x1a7b"},
				{"pa2gw1a1", "0x1a4a"},
				{"elna2g", "2"},
				{"rxchain", "3"},
				{"tempthresh", "120"},
				{"regrev", "61"},
				{"pa2gw0a0", "0xfed9"},
				{"pa2gw0a1", "0xfed4"},
				{"leddc", "0xFFFF"},
				{"triso2g", "3"},
				{"sromrev", "8"},
				{"ofdm5gpo", "0"},
				{"ccode", "Q2"},
				{"boardtype", "0x62b"},
				{"boardvendor", "0x14e4"},
				{"tssipos2g", "1"},
				{"devid", "0x43a9"},
				{"extpagain2g", "3"},
				{"maxp2ga0", "0x6A"},
				{"maxp2ga1", "0x6A"},
				{"boardflags", "0x80001200"},
				{"opo", "68"},
				{"tempoffset", "0"},
				{"ofdm5glpo", "0"},
				{"antswitch", "0"},
				{"txchain", "3"},
				{"ofdm2gpo", "0xA8641000"},
				{"ofdm5ghpo", "0"},
				{"mcs2gpo0", "0x1000"},
				{"mcs2gpo1", "0xA864"},
				{"boardflags2", "0x1800"},
				{"mcs2gpo2", "0x1000"},
				{"mcs2gpo3", "0xA864"},
				{"mcs2gpo4", "0x1000"},
				{"mcs2gpo5", "0xA864"},
				{"mcs2gpo6", "0x1000"},
				{"mcs2gpo7", "0xA864"},
				{"pa2gw2a0", "0xfa91"},
				{"pa2gw2a1", "0xfa9a"},
				{"antswctl2g", "0"},
				{"ag0", "2"},
				{0, 0}
			};

			struct nvram_param ex6200_1params[] = {
				{"tssiposslope5g", "1"},
				{"sb20in80and160lr5ghpo", "0"},
				{"rxgains2gtrisoa0", "7"},
				{"ofdm5glpo", "0"},
				{"boardflags", "0x30001000"},
				{"antswitch", "0"},
				{"sar2g", "0xFF"},
				{"tempsense_slope", "0xff"},
				{"mcsbw805glpo", "0x22000000"},
				{"tempoffset", "255"},
				{"rxgains5ghelnagaina0", "2"},
				{"rxgains5ghelnagaina1", "2"},
				{"sb40and80lr5gmpo", "0"},
				{"ofdm5ghpo", "0x75000000"},
				{"mcsbw805ghpo", "0xDD975300"},
				{"rawtempsense", "0x1ff"},
				{"sb20in80and160hr5glpo", "0"},
				{"femctrl", "1"},
				{"dot11agofdmhrbw202gpo", "0"},
				{"sb20in80and160hr5ghpo", "0"},
				{"sb20in40lrpo", "0"},
				{"rxgains5gmtrelnabypa0", "1"},
				{"rxgains5gmtrelnabypa1", "1"},
				{"papdcap2g", "0"},
				{"pa2ga0", "0xFF35,0x18F7,0xFCF5"},
				{"mcsbw405glpo", "0x22000000"},
				{"mcsbw402gpo", "0"},
				{"sb40and80hr5gmpo", "0"},
				{"mcslr5glpo", "0"},
				{"mcsbw1605glpo", "0"},
				{"dot11agduplrpo", "0"},
				{"ccode", "Q2"},
				{"rxgains5ghtrelnabypa0", "1"},
				{"rxgains5ghtrelnabypa1", "1"},
				{"mcsbw405ghpo", "0xDD975000"},
				{"mcsbw202gpo", "0"},
				{"ofdmlrbw202gpo", "0"},
				{"mcslr5ghpo", "0"},
				{"mcsbw1605ghpo", "0"},
				{"devid", "0x43b1"},
				{"sb40and80lr5glpo", "0"},
				{"measpower1", "0x7f"},
				{"measpower2", "0x7f"},
				{"sb40and80lr5ghpo", "0"},
				{"rxgains2gtrelnabypa0", "1"},
				{"maxp5ga0", "78,78,78,100"},
				{"maxp5ga1", "78,78,78,100"},
				{"sar5g", "0xFF"},
				{"gainctrlsph", "0"},
				{"pdgain2g", "14"},
				{"aga0", "0"},
				{"subband5gver", "0x4"},
				{"aga1", "0"},
				{"sb40and80hr5glpo", "0"},
				{"sb20in40hrpo", "0"},
				{"noiselvl5ga0", "31,31,31,31"},
				{"mcsbw205gmpo", "0x22000000"},
				{"cckbw202gpo", "0"},
				{"noiselvl5ga1", "31,31,31,31"},
				{"agbg0", "0"},
				{"agbg1", "0"},
				{"sb40and80hr5ghpo", "0"},
				{"epagain2g", "0"},
				{"rxgains2gelnagaina0", "4"},
				{"rxchain", "3"},
				{"boardnum", "20771"},
				{"tworangetssi2g", "0"},
				{"papdcap5g", "0"},
				{"dot11agduphrpo", "0"},
				{"aa2g", "1"},
				{"tempcorrx", "0x3f"},
				{"rxgains5gtrelnabypa0", "1"},
				{"regrev", "61"},
				{"rxgains5gtrelnabypa1", "1"},
				{"boardrev", "0x1402"},
				{"boardvendor", "0x14e4"},
				{"pdoffset80ma0", "0x0"},
				{"pdoffset80ma1", "0x0"},
				{"maxp2ga0", "0x50"},
				{"pdoffset80ma2", "0x0"},
				{"cckbw20ul2gpo", "0"},
				{"temps_hysteresis", "15"},
				{"tssiposslope2g", "1"},
				{"rxgains5gmtrisoa0", "5"},
				{"rxgains5gmtrisoa1", "4"},
				{"tempthresh", "255"},
				{"rxgains5gelnagaina0", "1"},
				{"rxgains5gelnagaina1", "1"},
				{"sromrev", "11"},
				{"sb20in80and160lr5gmpo", "0"},
				{"phycal_tempdelta", "0"},
				{"rxgainerr5ga0", "0x3F,0x3F,0x3F,0x3F"},
				{"rxgainerr5ga1", "0x1F,0x1F,0x1F,0x1F"},
				{"pdoffset40ma0", "0x1111"},
				{"rpcal2g", "0"},
				{"pdoffset40ma1", "0x1111"},
				{"mcsbw205glpo", "0x22000000"},
				{"pdoffset40ma2", "0x1111"},
				{"measpower", "0x7f"},
				{"temps_period", "15"},
				{"venid", "0x14e4"},
				{"mcsbw805gmpo", "0x22000000"},
				{"pdgain5g", "4"},
				{"boardflags2", "0x300002"},
				{"mcsbw205ghpo", "0xDD975000"},
				{"boardflags3", "0x0"},
				{"rxgains5gtrisoa0", "7"},
				{"rxgains5ghtrisoa0", "5"},
				{"rxgains5gtrisoa1", "6"},
				{"rxgains5ghtrisoa1", "4"},
				{"sb20in80and160hr5gmpo", "0"},
				{"epagain5g", "0"},
				{"tempsense_option", "0x3"},
				{"boardtype", "0x621"},
				{"tworangetssi5g", "0"},
				{"rxgains5gmelnagaina0", "2"},
				{"rxgains5gmelnagaina1", "2"},
				{"aa5g", "3"},
				{"xtalfreq", "40000"},
				{"mcsbw405gmpo", "0x22000000"},
				{"pa5ga0", "0xff3c,0x19d6,0xfce4,0xff3b,0x19d0,0xfce5,0xff39,0x19b8,0xfce5,0xff3a,0x19b0,0xfce5"},
				{"txchain", "3"},
				{"pa5ga1", "0xff33,0x1918,0xfcf3,0xff37,0x1988,0xfcea,0xff32,0x1953,0xfceb,0xff36,0x1944,0xfcee"},
				{"mcslr5gmpo", "0"},
				{"mcsbw1605gmpo", "0"},
				{"sb20in80and160lr5glpo", "0"},
				{"rxgainerr2ga0", "0x3F"},
				{0, 0}
			};

			extra_params = ex6200_0params;
			while (extra_params->name) {
				nvram_nset(extra_params->value, "0:%s", extra_params->name);
				extra_params++;
			}
			extra_params = ex6200_1params;
			while (extra_params->name) {
				nvram_nset(extra_params->value, "1:%s", extra_params->name);
				extra_params++;
			}

			nvram_set("devpath0", "pci/1/1"), nvram_set("devpath1", "pci/2/1"), nvram_set("wl_pcie_mrrs", "128"), nvram_commit();
		}
		nvram_unset("et1macaddr");
		set_gpio(0, 1);	//USB
		set_gpio(4, 1);	//wifi
		set_gpio(6, 1);	//reset button
		set_gpio(9, 1);	//red connected
		set_gpio(10, 1);	//green 2.4
		set_gpio(12, 1);	//green 5
		break;
	case ROUTER_NETGEAR_R8000:
		if (nvram_get("0:venid") == NULL) {
			char mac[20];
			strcpy(mac, nvram_safe_get("et2macaddr"));
			MAC_ADD(mac);
			MAC_ADD(mac);
			nvram_set("0:macaddr", mac);
			MAC_ADD(mac);
			nvram_set("1:macaddr", mac);
			MAC_ADD(mac);
			nvram_set("2:macaddr", mac);
			struct nvram_param r8000_0params[] = {
				{"venid", "0x14e4"},
				{"watchdog", "3000"},
				{"deadman_to", "720000000"},
				{"rxgains5gmelnagaina0", "1"},
				{"rxgains5gmelnagaina1", "1"},
				{"rxgains5gmelnagaina2", "1"},
				{"pwr_scale_1db", "1"},
				{"venid", "0x14e4"},
				{"mcsbw405gmpo", "0x98658640"},
				{"rxgains5ghelnagaina0", "1"},
				{"rxgains5ghelnagaina1", "1"},
				{"papdcap5g", "0"},
				{"rxgains5ghelnagaina2", "1"},
				{"tempcorrx", "0x3f"},
				{"tworangetssi5g", "0"},
				{"mcsbw805glpo", "0"},
				{"rxchain", "7"},
				{"mcsbw805ghpo", "0x98658640"},
				{"pdoffset80ma0", "0"},
				{"rawtempsense", "0x1ff"},
				{"pdoffset80ma1", "0"},
				{"tempthresh", "120"},
				{"pdoffset80ma2", "0"},
				{"tssiposslope5g", "1"},
				{"gainctrlsph", "0"},
				{"mcsbw405glpo", "0"},
				{"maxp5ga0", "54,90,90,106"},
				{"maxp5ga1", "54,90,90,106"},
				{"maxp5ga2", "54,90,90,106"},
				{"regrev", "86"},
				{"rxgainerr5ga0", "63,63,63,-9"},
				{"rxgainerr5ga1", "31,31,31,-2"},
				{"rxgainerr5ga2", "31,31,31,-5"},
				{"pdoffset40ma0", "0"},
				{"mcsbw405ghpo", "0x98658640"},
				{"pdoffset40ma1", "0"},
				{"pdoffset40ma2", "0"},
				{"sromrev", "11"},
				{"rxgains5gmtrisoa0", "6"},
				{"aa5g", "7"},
				{"rxgains5gmtrisoa1", "6"},
				{"rxgains5gmtrisoa2", "6"},
				{"ccode", "Q2"},
				{"epagain5g", "0"},
				{"dot11agduplrpo", "0"},
				{"boardvendor", "0x14e4"},
				{"mcslr5gmpo", "0"},
				{"tempsense_slope", "0xff"},
				{"devid", "0x43BC"},
				{"boardrev", "0x1421"},
				{"rxgains5ghtrisoa0", "6"},
				{"rxgains5gelnagaina0", "1"},
				{"rxgains5ghtrisoa1", "6"},
				{"rxgains5gelnagaina1", "1"},
				{"rxgains5ghtrisoa2", "6"},
				{"rxgains5gelnagaina2", "1"},
				{"mcsbw1605gmpo", "0"},
				{"rxgains5gmtrelnabypa0", "1"},
				{"temps_hysteresis", "5"},
				{"rxgains5gmtrelnabypa1", "1"},
				{"devpath0", "sb/1/"},
				{"rxgains5gmtrelnabypa2", "1"},
				{"subband5gver", "0x4"},
				{"boardflags", "0x30008000"},
				{"rxgains5ghtrelnabypa0", "1"},
				{"tempoffset", "255"},
				{"rxgains5gtrelnabypa0", "1"},
				{"rxgains5ghtrelnabypa1", "1"},
				{"rxgains5gtrelnabypa1", "1"},
				{"rxgains5ghtrelnabypa2", "1"},
				{"rxgains5gtrelnabypa2", "1"},
				{"antswitch", "0"},
				{"mcsbw205gmpo", "0x98658640"},
				{"txchain", "7"},
				{"phycal_tempdelta", "15"},
				{"boardflags2", "0x2"},
				{"boardflags3", "0x1"},
				{"pa5ga0", "0xff4c,0x18df,0xfd12,0xff52,0x195d,0xfd11,0xff49,0x1a47,0xfcdc,0xff40,0x1b48,0xfccb"},
				{"pa5ga1", "0xff5c,0x19ba,0xfd12,0xff3e,0x1932,0xfcf5,0xff4d,0x1a9f,0xfcdd,0xff41,0x1cb1,0xfcab"},
				{"rxgains5gtrisoa0", "6"},
				{"pa5ga2", "0xff29,0x16c3,0xfd33,0xff4f,0x1b3d,0xfcd8,0xff4a,0x1fde,0xfc5f,0xff40,0x1c41,0xfca1"},
				{"rxgains5gtrisoa1", "6"},
				{"pdgain5g", "4"},
				{"rxgains5gtrisoa2", "6"},
				{"mcslr5glpo", "0"},
				{"tempsense_option", "0x3"},
				{"dot11agduphrpo", "0"},
				{"femctrl", "6"},
				{"mcslr5ghpo", "0"},
				{"mcsbw1605glpo", "0"},
				{"rpcal5gb0", "0"},
				{"rpcal5gb1", "0"},
				{"rpcal5gb2", "0"},
				{"rpcal5gb3", "0xb433"},
				{"xtalfreq", "40000"},
				{"mcsbw1605ghpo", "0"},
				{"mcsbw205glpo", "0"},
				{"temps_period", "5"},
				{"aga0", "0x0"},
				{"aga1", "0x0"},
				{"mcsbw805gmpo", "0x98658640"},
				{"aga2", "0x0"},
				{"mcsbw205ghpo", "0x98658640"},
				{0, 0}
			};
			struct nvram_param r8000_1params[] = {

				{"venid", "0x14e4"},
				{"watchdog", "3000"},
				{"deadman_to", "720000000"},
				{"rxgainerr2ga1", "0"},
				{"rxgainerr2ga2", "-1"},
				{"rxgains2gtrisoa0", "6"},
				{"boardflags", "0x1000"},
				{"antswitch", "0"},
				{"rxgains2gtrisoa1", "6"},
				{"tempsense_slope", "0xff"},
				{"rxgains2gtrisoa2", "6"},
				{"tempoffset", "255"},
				{"rawtempsense", "0x1ff"},
				{"femctrl", "6"},
				{"dot11agofdmhrbw202gpo", "0x8ECA"},
				{"papdcap2g", "0"},
				{"pa2ga0", "0xff40,0x1b2a,0xfcd0"},
				{"pa2ga1", "0xff45,0x1b83,0xfcd2"},
				{"mcsbw402gpo", "0x98658640"},
				{"pa2ga2", "0xff3a,0x1b35,0xfcc2"},
				{"dot11agduplrpo", "0"},
				{"ccode", "Q2"},
				{"mcsbw202gpo", "0x98658640"},
				{"ofdmlrbw202gpo", "0x0"},
				{"venvid", "0x14e4"},
				{"devid", "0x43BB"},
				{"rxgains2gtrelnabypa0", "1"},
				{"rxgains2gtrelnabypa1", "1"},
				{"rxgains2gtrelnabypa2", "1"},
				{"pwr_scale_1db", "1"},
				{"gainctrlsph", "0"},
				{"pdgain2g", "21"},
				{"cckbw202gpo", "0"},
				{"agbg0", "0x0"},
				{"agbg1", "0x0"},
				{"epagain2g", "0"},
				{"agbg2", "0x0"},
				{"rxgains2gelnagaina0", "2"},
				{"rxchain", "7"},
				{"rxgains2gelnagaina1", "2"},
				{"rxgains2gelnagaina2", "2"},
				{"tworangetssi2g", "0"},
				{"dot11agduphrpo", "0"},
				{"aa2g", "7"},
				{"tempcorrx", "0x3f"},
				{"regrev", "86"},
				{"boardrev", "0x1421"},
				{"boardvendor", "0x14e4"},
				{"pdoffset80ma0", "0"},
				{"pdoffset80ma1", "0"},
				{"maxp2ga0", "106"},
				{"pdoffset80ma2", "0"},
				{"pdoffset2g40ma0", "15"},
				{"maxp2ga1", "106"},
				{"devpath1", "sb/1/"},
				{"cckbw20ul2gpo", "0"},
				{"pdoffset2g40ma1", "15"},
				{"maxp2ga2", "106"},
				{"pdoffset2g40ma2", "15"},
				{"temps_hysteresis", "5"},
				{"tssiposslope2g", "1"},
				{"pdoffset2g40mvalid", "1"},
				{"tempthresh", "120"},
				{"sromrev", "11"},
				{"phycal_tempdelta", "15"},
				{"pdoffset40ma0", "0"},
				{"pdoffset40ma1", "0"},
				{"rpcal2g", "0x2406"},
				{"pdoffset40ma2", "0"},
				{"temps_period", "5"},
				{"boardflags2", "0x2"},
				{"boardflags3", "0x4000001"},
				{"tempsense_option", "0x3"},
				{"xtalfreq", "40000"},
				{"txchain", "7"},
				{"rxgainerr2ga0", "-1"},
				{0, 0}
			};
			struct nvram_param r8000_2params[] = {
				{"watchdog", "3000"},
				{"deadman_to", "720000000"},
				{"mcsbw405glpo", "0x87657531"},
				{"devid", "0x43BC"},
				{"mcsbw405ghpo", "0x87657531"},
				{"rxgains5ghtrisoa0", "6"},
				{"rxgains5ghtrisoa1", "6"},
				{"rxgains5ghtrisoa2", "6"},
				{"tempthresh", "120"},
				{"rxgainerr5ga0", "-9,63,63,63"},
				{"rxgainerr5ga1", "-4,31,31,31"},
				{"rxgainerr5ga2", "-5,31,31,31"},
				{"pdoffset40ma0", "0x5444"},
				{"pdoffset40ma1", "0x5444"},
				{"pdoffset40ma2", "0x5344"},
				{"pa5ga0", "0xff25,0x166D,0xfd37,0xff27,0x16d0,0xfd20,0xFF33,0x1A22,0xFCD6,0xFF32,0x19FC,0xFCDD"},
				{"pa5ga1", "0xff28,0x1619,0xfd3c,0xff31,0x179f,0xfd1b,0xFF2F,0x19B3,0xFCE2,0xFF2E,0x19EB,0xFCDA"},
				{"pa5ga2", "0xff29,0x15c3,0xfd4e,0xff2a,0x1777,0xfd16,0xFF31,0x1A12,0xFCD7,0xFF31,0x19FE,0xFCDD"},
				{"txchain", "7"},
				{"dot11agduphrpo", "0x4444"},
				{"epagain5g", "0"},
				{"subband5gver", "0x4"},
				{"mcsbw1605gmpo", "0"},
				{"mcsbw205gmpo", "0x87657530"},
				{"mcslr5gmpo", "0"},
				{"femctrl", "6"},
				{"aa5g", "7"},
				{"maxp5ga0", "90,90,90,90"},
				{"maxp5ga1", "90,90,90,90"},
				{"maxp5ga2", "90,90,90,90"},
				{"antswitch", "0"},
				{"boardflags", "0x10000000"},
				{"tempoffset", "255"},
				{"rxgains5gtrelnabypa0", "1"},
				{"rxgains5gtrelnabypa1", "1"},
				{"rxgains5gtrelnabypa2", "1"},
				{"gainctrlsph", "0"},
				{"venid", "0x14e4"},
				{"tempsense_slope", "0xff"},
				{"mcsbw1605glpo", "0"},
				{"mcsbw205glpo", "0x87657530"},
				{"tworangetssi5g", "0"},
				{"temps_period", "5"},
				{"mcslr5glpo", "0"},
				{"mcsbw1605ghpo", "0"},
				{"boardrev", "0x1421"},
				{"rpcal5gb0", "0x3739"},
				{"mcsbw805gmpo", "0x87657531"},
				{"rpcal5gb1", "0"},
				{"rpcal5gb2", "0"},
				{"rpcal5gb3", "0"},
				{"mcsbw205ghpo", "0x87657530"},
				{"mcslr5ghpo", "0"},
				{"devpath2", "sb/1/"},
				{"boardvendor", "0x14e4"},
				{"temps_hysteresis", "5"},
				{"tssiposslope5g", "1"},
				{"regrev", "86"},
				{"rxgains5gmtrelnabypa0", "1"},
				{"rxgains5gmtrelnabypa1", "1"},
				{"pwr_scale_1db", "1"},
				{"rxgains5gmtrelnabypa2", "1"},
				{"mcsbw405gmpo", "0x87657531"},
				{"phycal_tempdelta", "15"},
				{"rxgains5gelnagaina0", "1"},
				{"rxchain", "7"},
				{"rxgains5gelnagaina1", "1"},
				{"rxgains5gmelnagaina0", "1"},
				{"rxgains5gelnagaina2", "1"},
				{"rxgains5gmelnagaina1", "1"},
				{"rxgains5ghtrelnabypa0", "1"},
				{"rxgains5gmelnagaina2", "1"},
				{"rxgains5ghtrelnabypa1", "1"},
				{"rxgains5ghtrelnabypa2", "1"},
				{"pdgain5g", "19"},
				{"boardflags2", "0x2"},
				{"rxgains5gtrisoa0", "6"},
				{"boardflags3", "0x2"},
				{"rxgains5gtrisoa1", "6"},
				{"rxgains5gtrisoa2", "6"},
				{"mcsbw805glpo", "0x87657531"},
				{"rxgains5gmtrisoa0", "6"},
				{"rxgains5gmtrisoa1", "6"},
				{"papdcap5g", "0"},
				{"dot11agduplrpo", "0x4444"},
				{"rxgains5gmtrisoa2", "6"},
				{"mcsbw805ghpo", "0x87657531"},
				{"aga0", "0x0"},
				{"rawtempsense", "0x1ff"},
				{"aga1", "0x0"},
				{"aga2", "0x0"},
				{"xtalfreq", "40000"},
				{"tempsense_option", "0x3"},
				{"tempcorrx", "0x3f"},
				{"ccode", "Q2"},
				{"rxgains5ghelnagaina0", "1"},
				{"sromrev", "11"},
				{"rxgains5ghelnagaina1", "1"},
				{"rxgains5ghelnagaina2", "1"},
				{"pdoffset80ma0", "0x2111"},
				{"pdoffset80ma1", "0x0111"},
				{"pdoffset80ma2", "0x2111"},
				{0, 0}
			};

			nvram_set("devpath0", "pcie/1/1");
			nvram_set("devpath1", "pcie/2/3");
			nvram_set("devpath2", "pcie/2/4");
			nvram_set("wl2_channel", "48");
			nvram_set("wl0_channel", "161");

			extra_params = r8000_0params;
			while (extra_params->name) {
				nvram_nset(extra_params->value, "0:%s", extra_params->name);
				extra_params++;
			}
			extra_params = r8000_1params;
			while (extra_params->name) {
				nvram_nset(extra_params->value, "1:%s", extra_params->name);
				extra_params++;
			}

			extra_params = r8000_2params;
			while (extra_params->name) {
				nvram_nset(extra_params->value, "2:%s", extra_params->name);
				extra_params++;
			}

			nvram_commit();
		}
		set_regulation(0, "Q2", "86");
		set_regulation(1, "Q2", "86");
		set_regulation(2, "Q2", "86");
		nvram_unset("et0macaddr");
		nvram_unset("et1macaddr");
		set_gpio(6, 1);	//reset button
		set_gpio(15, 1);
		break;
	case ROUTER_ASUS_AC87U:
		set_gpio(11, 1);	// fixup reset button
		set_gpio(15, 1);	// fixup wifi button
		set_gpio(2, 1);	// fixup ses button
		break;
	case ROUTER_ASUS_AC67U:
		if (!nvram_match("bl_version", "1.0.1.1"))
			nvram_set("clkfreq", "800,666");
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
		set_gpio(7, 1);	// fixup ses button
		set_gpio(15, 1);	// fixup wifi button
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
	case ROUTER_ASUS_RTN18U:
		set_gpio(7, 1);	// fixup reset button
		set_gpio(11, 1);	// fixup wps button
		break;
	case ROUTER_ASUS_AC56U:
		nvram_set("clkfreq", "800,666");
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
		set_gpio(7, 1);	// fixup wifi button
		set_gpio(15, 1);	// fixup ses button
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
	case ROUTER_DLINK_DIR890:
		if (!strncmp(nvram_safe_get("et0macaddr"), "00:90", 5)) {
			char buf[64];
			FILE *fp = popen("cat /dev/mtdblock0|grep lanmac", "r");
			fread(buf, 1, 24, fp);
			pclose(fp);
			buf[24] = 0;
			fprintf(stderr, "set main mac %s\n", &buf[7]);
			nvram_set("et0macaddr", &buf[7]);

			fp = popen("cat /dev/mtdblock0|grep wlan24mac=", "r");
			fread(buf, 1, 27, fp);
			pclose(fp);
			buf[27] = 0;
			fprintf(stderr, "set 2.4g mac %s\n", &buf[10]);
			nvram_set("0:macaddr", &buf[10]);

			fp = popen("cat /dev/mtdblock0|grep wlan5mac2=", "r");
			fread(buf, 1, 27, fp);
			pclose(fp);
			buf[27] = 0;
			fprintf(stderr, "set 5g mac 1 %s\n", &buf[10]);
			nvram_set("1:macaddr", &buf[10]);

			fp = popen("cat /dev/mtdblock0|grep wlan5mac=", "r");
			fread(buf, 1, 26, fp);
			pclose(fp);
			buf[26] = 0;
			fprintf(stderr, "set 5g mac 2 %s\n", &buf[9]);
			nvram_set("2:macaddr", &buf[9]);
			nvram_commit();
		}
		break;
	case ROUTER_DLINK_DIR880:
		if (nvram_get("0:venid") == NULL) {
			char buf[64];
			FILE *fp = popen("cat /dev/mtdblock0|grep lanmac", "r");
			fread(buf, 1, 24, fp);
			pclose(fp);
			buf[24] = 0;
			fprintf(stderr, "set main mac %s\n", &buf[7]);
			nvram_set("et0macaddr", &buf[7]);
			fp = popen("cat /dev/mtdblock0|grep wlan5mac", "r");
			fread(buf, 1, 26, fp);
			pclose(fp);
			buf[26] = 0;
			fprintf(stderr, "set 5g mac %s\n", &buf[9]);
			nvram_set("1:macaddr", &buf[9]);
			fp = popen("cat /dev/mtdblock0|grep wlan24mac", "r");
			fread(buf, 1, 27, fp);
			pclose(fp);
			buf[27] = 0;
			fprintf(stderr, "set 2.4g mac %s\n", &buf[10]);
			nvram_set("0:macaddr", &buf[10]);
			struct nvram_param dir880_0params[] = {
				{"rxgains2gtrisoa0", "7"},
				{"rxgains2gtrisoa1", "7"},
				{"rxgains2gtrisoa2", "7"},
				{"pa2ga0", "0xFF29,0x1b86,0xFCa9"},
				{"pa2ga1", "0xFF2e,0x1c4d,0xFC99"},
				{"pa2ga2", "0xFF27,0x1ac0,0xFcc0"},
				{"aa2g", "7"},
				{"venvid", "0x14e4"},
				{"mcsbw402gpo", "0x0"},
				{"epagain2g", "0"},
				{"pdoffset2g40ma0", "15"},
				{"pdoffset2g40ma1", "15"},
				{"pdoffset2g40ma2", "15"},
				{"mcsbw202gpo", "0x0"},
				{"rxchain", "7"},
				{"pdoffset80ma0", "0"},
				{"rawtempsense", "0x1ff"},
				{"pdoffset80ma1", "0"},
				{"tempthresh", "255"},
				{"pdoffset80ma2", "0"},
				{"cckbw20ul2gpo", "0"},
				{"sb40and80lr5ghpo", "0"},
				{"sar2g", "18"},
				{"gainctrlsph", "0"},
				{"regrev", "0"},
				{"measpower", "0x7f"},
				{"pdoffset2g40mvalid", "1"},
				{"pdoffset40ma0", "0"},
				{"pdoffset40ma1", "0"},
				{"pdgain2g", "14"},
				{"pdoffset40ma2", "0"},
				{"rxgains2gelnagaina0", "4"},
				{"rxgains2gelnagaina1", "4"},
				{"rxgains2gelnagaina2", "4"},
				{"sromrev", "11"},
				{"cckbw202gpo", "0"},
				{"rxgains2gtrelnabypa0", "1"},
				{"rxgains2gtrelnabypa1", "1"},
				{"ccode", "US"},
				{"rpcal2g", "0"},
				{"rxgains2gtrelnabypa2", "1"},
				{"dot11agduplrpo", "0"},
				{"boardvendor", "0x14e4"},
				{"tssifloor2g", "0x3ff"},
				{"ofdmlrbw202gpo", "0"},
				{"devid", "0x43a1"},
				{"dot11agofdmhrbw202gpo", "0x0"},
				{"maxp2ga0", "94"},
				{"maxp2ga1", "94"},
				{"maxp2ga2", "94"},
				{"rxgainerr2ga0", "63"},
				{"rxgainerr2ga1", "31"},
				{"rxgainerr2ga2", "31"},
				{"boardflags", "0x1000"},
				{"tempoffset", "255"},
				{"antswitch", "0"},
				{"txchain", "7"},
				{"phycal_tempdelta", "255"},
				{"boardflags2", "0x100002"},
				{"boardflags3", "0x3"},
				{"agbg0", "71"},
				{"agbg1", "71"},
				{"agbg2", "71"},
				{"papdcap2g", "0"},
				{"tworangetssi2g", "0"},
				{"dot11agduphrpo", "0"},
				{"femctrl", "3"},
				{"xtalfreq", "65535"},
				{"measpower1", "0x7f"},
				{"measpower2", "0x7f"},
				{"tssiposslope2g", "1"},
				{"ledbh13", "7"},
				{"ledbh0", "11"},
				{"ledbh1", "11"},
				{"ledbh2", "11"},
				{0, 0}
			};
			struct nvram_param dir880_1params[] = {
				{"tssiposslope5g", "1"},
				{"boardflags", "0x30000000"},
				{"antswitch", "0"},
				{"mcsbw805glpo", "0"},
				{"rxgains5ghelnagaina0", "2"},
				{"rxgains5ghelnagaina1", "2"},
				{"rxgains5ghelnagaina2", "3"},
				{"mcsbw805ghpo", "0x0"},
				{"femctrl", "3"},
				{"rxgains5gmtrelnabypa0", "1"},
				{"rxgains5gmtrelnabypa1", "1"},
				{"rxgains5gmtrelnabypa2", "1"},
				{"mcsbw405glpo", "0"},
				{"mcsbw1605glpo", "0"},
				{"mcslr5glpo", "0"},
				{"dot11agduplrpo", "0"},
				{"ccode", "US"},
				{"macaddr", "00:90:4C:0E:50:18"},
				{"rxgains5ghtrelnabypa0", "1"},
				{"rxgains5ghtrelnabypa1", "1"},
				{"mcsbw405ghpo", "0x0"},
				{"rxgains5ghtrelnabypa2", "1"},
				{"mcsbw1605ghpo", "0"},
				{"mcslr5ghpo", "0"},
				{"devid", "0x43a2"},
				{"measpower1", "0x7f"},
				{"measpower2", "0x7f"},
				{"sb40and80lr5ghpo", "0"},
				{"maxp5ga0", "94,94,94,94"},
				{"maxp5ga1", "94,94,94,94"},
				{"maxp5ga2", "94,94,94,94"},
				{"sar5g", "15"},
				{"gainctrlsph", "0"},
				{"aga0", "71"},
				{"aga1", "133"},
				{"subband5gver", "0x4"},
				{"aga2", "133"},
				{"mcsbw205gmpo", "0x0"},
				{"rxchain", "7"},
				{"papdcap5g", "0"},
				{"dot11agduphrpo", "0"},
				{"regrev", "0"},
				{"rxgains5gtrelnabypa0", "1"},
				{"rxgains5gtrelnabypa1", "1"},
				{"rxgains5gtrelnabypa2", "1"},
				{"boardvendor", "0x14e4"},
				{"pdoffset80ma0", "0"},
				{"pdoffset80ma1", "0"},
				{"pdoffset80ma2", "0"},
				{"rxgains5gmtrisoa0", "5"},
				{"rxgains5gmtrisoa1", "4"},
				{"rxgains5gmtrisoa2", "4"},
				{"rxgains5gelnagaina0", "1"},
				{"rxgains5gelnagaina1", "1"},
				{"pcieingress_war", "15"},
				{"rxgains5gelnagaina2", "1"},
				{"sromrev", "11"},
				{"phycal_tempdelta", "255"},
				{"rxgainerr5ga0", "63,63,63,63"},
				{"rxgainerr5ga1", "31,31,31,31"},
				{"rxgainerr5ga2", "31,31,31,31"},
				{"pdoffset40ma0", "4369"},
				{"pdoffset40ma1", "4369"},
				{"mcsbw205glpo", "0"},
				{"pdoffset40ma2", "4369"},
				{"measpower", "0x7f"},
				{"venid", "0x14e4"},
				{"mcsbw805gmpo", "0x0"},
				{"pdgain5g", "4"},
				{"boardflags2", "0x300002"},
				{"boardflags3", "0x0"},
				{"mcsbw205ghpo", "0x0"},
				{"rxgains5ghtrisoa0", "5"},
				{"rxgains5gtrisoa0", "7"},
				{"rxgains5ghtrisoa1", "4"},
				{"rxgains5gtrisoa1", "6"},
				{"rxgains5ghtrisoa2", "4"},
				{"rxgains5gtrisoa2", "5"},
				{"epagain5g", "0"},
				{"tworangetssi5g", "0"},
				{"rxgains5gmelnagaina0", "2"},
				{"rxgains5gmelnagaina1", "2"},
				{"rxgains5gmelnagaina2", "3"},
				{"aa5g", "7"},
				{"xtalfreq", "65535"},
				{"mcsbw405gmpo", "0x0"},
				{"pa5ga0", "0xff45,0x1b8f,0xfcb4,0xff3d,0x1af7,0xfcbe,0xff46,0x1b8a,0xfcb4,0xff43,0x1b06,0xfcc1"},
				{"txchain", "7"},
				{"pa5ga1", "0xff3c,0x1acd,0xfcc1,0xff45,0x1b14,0xfcc5,0xff42,0x1b04,0xfcc5,0xff3f,0x1aab,0xfccc"},
				{"mcsbw1605gmpo", "0"},
				{"mcslr5gmpo", "0"},
				{"pa5ga2", "0xff3d,0x1aee,0xfcc1,0xff33,0x1a56,0xfcc4,0xff3f,0x1b04,0xfcc1,0xff47,0x1b48,0xfcc2"},
				{"ledbh14", "7"},
				{"ledbh0", "11"},
				{"ledbh1", "11"},
				{"ledbh2", "11"},
				{0, 0}
			};

			struct nvram_param *t;
			t = dir880_0params;
			while (t->name) {
				nvram_nset(t->value, "0:%s", t->name);
				t++;
			}
			t = dir880_1params;
			while (t->name) {
				nvram_nset(t->value, "1:%s", t->name);
				t++;
			}

			nvram_set("0:venid", "0x14E4");
			nvram_commit();

		}
		set_gpio(7, 1);	// fixup ses button

		break;
	case ROUTER_DLINK_DIR860:
		if (nvram_get("devpath0") == NULL) {
			nvram_set("devpath0", "pci/1/1/");
			nvram_set("devpath1", "pci/2/1/");

			char buf[64];
			FILE *fp = popen("cat /dev/mtdblock0|grep lanmac", "r");
			fread(buf, 1, 24, fp);
			pclose(fp);
			buf[24] = 0;
			fprintf(stderr, "set main mac %s\n", &buf[7]);
			nvram_set("et0macaddr", &buf[7]);

			fp = popen("cat /dev/mtdblock0|grep wlan5mac", "r");
			fread(buf, 1, 26, fp);
			pclose(fp);
			buf[26] = 0;
			fprintf(stderr, "set 5g mac %s\n", &buf[9]);
			nvram_set("pci/2/0/macaddr", &buf[9]);
			nvram_set("pci/2/1/macaddr", &buf[9]);
			fp = popen("cat /dev/mtdblock0|grep wlan24mac", "r");
			fread(buf, 1, 27, fp);
			pclose(fp);
			buf[27] = 0;
			fprintf(stderr, "set 2.4g mac %s\n", &buf[10]);
			nvram_set("pci/1/0/macaddr", &buf[10]);
			nvram_set("pci/1/1/macaddr", &buf[10]);
			struct nvram_param dir860_1_1params[] = {
				{"aa2g", "0x3"},
				{"ag0", "0x0"},
				{"ag1", "0x0"},
				{"ag2", "0xff"},
				{"ag3", "0xff"},
				{"antswctl2g", "0x0"},
				{"antswitch", "0x0"},
				{"boardflags2", "0x9800"},
				{"boardflags", "0x200"},
				{"boardtype", "0x5e8"},
				{"boardvendor", "0x14E4"},
				{"bw40po", "0x0"},
				{"bwduppo", "0x0"},
				{"bxa2g", "0x0"},
				{"cck2gpo", "0x1111"},
				{"ccode", "0"},
				{"cddpo", "0x0"},
				{"devid", "0x43a9"},
				{"extpagain2g", "0x2"},
				{"itt2ga0", "0x20"},
				{"itt2ga1", "0x20"},
				{"ledbh0", "11"},
				{"ledbh10", "11"},
				{"ledbh11", "11"},
				{"ledbh1", "11"},
				{"ledbh2", "11"},
				{"ledbh3", "11"},
				{"ledbh4", "11"},
				{"ledbh5", "11"},
				{"ledbh6", "11"},
				{"ledbh7", "11"},
				{"ledbh8", "11"},
				{"ledbh9", "11"},
				{"leddc", "0xffff"},
				{"maxp2ga0", "0x50"},
				{"maxp2ga1", "0x50"},
				{"mcs2gpo0", "0x7777"},
				{"mcs2gpo1", "0x9777"},
				{"mcs2gpo2", "0x7777"},
				{"mcs2gpo3", "0x9777"},
				{"mcs2gpo4", "0x9999"},
				{"mcs2gpo5", "0xb999"},
				{"mcs2gpo6", "0x9999"},
				{"mcs2gpo7", "0xb999"},
				{"ofdm2gpo", "0x97777777"},
				{"opo", "0x0"},
				{"pa2gw0a0", "0xFE31"},
				{"pa2gw0a1", "0xFE4F"},
				{"pa2gw1a0", "0x14E9"},
				{"pa2gw1a1", "0x16DB"},
				{"pa2gw2a0", "0xFA80"},
				{"pa2gw2a1", "0xFA4C"},
				{"pdetrange2g", "0x2"},
				{"phycal_tempdelta", "0"},
				{"regrev", "0"},
				{"rssisav2g", "0x0"},
				{"rssismc2g", "0x0"},
				{"rssismf2g", "0x0"},
				{"rxchain", "0x3"},
				{"rxpo2g", "0x0"},
				{"sromrev", "8"},
				{"stbcpo", "0x0"},
				{"tempoffset", "0"},
				{"temps_period", "5"},
				{"tempthresh", "120"},
				{"tri2g", "0x0"},
				{"triso2g", "0x4"},
				{"tssipos2g", "0x1"},
				{"txchain", "0x3"},
				{"vendid", "0x14E4"},
				{"venid", "0x14E4"},
				{0, 0}
			};

			struct nvram_param dir860_2_1params[] = {
				{"aa2g", "0"},
				{"aa5g", "3"},
				{"aga0", "0"},
				{"aga1", "0"},
				{"aga2", "0"},
				{"agbg0", "0"},
				{"agbg1", "0"},
				{"agbg2", "0"},
				{"antswitch", "0"},
				{"boardflags2", "0x2"},
				{"boardflags3", "0x0"},
				{"boardflags", "0x10001000"},
				{"boardnum", "0"},
				{"boardrev", "0x1350"},
				{"boardtype", "0x62f"},
				{"boardvendor", "0x14E4"},
				{"cckbw202gpo", "0"},
				{"cckbw20ul2gpo", "0"},
				{"ccode", "0"},
				{"devid", "0x43b3"},
				{"dot11agduphrpo", "0"},
				{"dot11agduplrpo", "0"},
				{"dot11agofdmhrbw202g", "17408"},
				{"epagain2g", "0"},
				{"epagain5g", "0"},
				{"femctrl", "6"},
				{"gainctrlsph", "0"},
				{"maxp2ga0", "66"},
				{"maxp2ga1", "66"},
				{"maxp2ga2", "66"},
				{"maxp5ga0", "78,78,78,78"},
				{"maxp5ga1", "78,78,78,78"},
				{"maxp5ga2", "78,78,78,78"},
				{"mcsbw1605ghpo", "0"},
				{"mcsbw1605glpo", "0"},
				{"mcsbw1605gmpo", "0"},
				{"mcsbw202gpo", "2571386880"},
				{"mcsbw205ghpo", "2252472320"},
				{"mcsbw205glpo", "2252472320"},
				{"mcsbw205gmpo", "2252472320"},
				{"mcsbw402gpo", "2571386880"},
				{"mcsbw405ghpo", "2252472320"},
				{"mcsbw405glpo", "2252472320"},
				{"mcsbw405gmpo", "2252472320"},
				{"mcsbw805ghpo", "2252472320"},
				{"mcsbw805glpo", "2252472320"},
				{"mcsbw805gmpo", "2252472320"},
				{"mcslr5ghpo", "0"},
				{"mcslr5glpo", "0"},
				{"mcslr5gmpo", "0"},
				{"measpower1", "0x7f"},
				{"measpower2", "0x7f"},
				{"measpower", "0x7f"},
				{"noiselvl2ga0", "31"},
				{"noiselvl2ga1", "31"},
				{"noiselvl2ga2", "31"},
				{"noiselvl5ga0", "31,31,31,31"},
				{"noiselvl5ga1", "31,31,31,31"},
				{"noiselvl5ga2", "31,31,31,31"},
				{"ofdmlrbw202gpo", "0"},
				{"pa2ga0", "0xff24,0x188e,0xfce6"},
				{"pa2ga1", "0xff3e,0x15f9,0xfd36"},
				{"pa2ga2", "0xff25,0x18a6,0xfce2"},
				{"pa5ga0", "0xff72,0x17d1,0xfd29,0xff78,0x183b,0xfd27,0xff75,0x1866,0xfd20,0xff85,0x18c8,0xfd30"},
				{"pa5ga1", "0xff4e,0x1593,0xfd4b,0xff61,0x1743,0xfd21,0xff6a,0x1721,0xfd41,0xff99,0x18e6,0xfd40"},
				{"pa5ga2", "0xff4d,0x166c,0xfd2a,0xff52,0x168a,0xfd31,0xff5e,0x1768,0xfd25,0xff61,0x1744,0xfd32"},
				{"papdcap2g", "0"},
				{"papdcap5g", "0"},
				{"pcieingress_war", "15"},
				{"pdgain2g", "10"},
				{"pdgain5g", "10"},
				{"pdoffset40ma0", "12834"},
				{"pdoffset40ma1", "12834"},
				{"pdoffset40ma2", "12834"},
				{"pdoffset80ma0", "256"},
				{"pdoffset80ma1", "256"},
				{"pdoffset80ma2", "256"},
				{"phycal_tempdelta", "255"},
				{"rawtempsense", "0x1ff"},
				{"regrev", "0"},
				{"rxchain", "3"},
				{"rxgainerr2ga0", "63"},
				{"rxgainerr2ga1", "31"},
				{"rxgainerr2ga2", "31"},
				{"rxgainerr5ga0", "63,63,63,63"},
				{"rxgainerr5ga1", "31,31,31,31"},
				{"rxgainerr5ga2", "31,31,31,31"},
				{"rxgains2gelnagaina0", "0"},
				{"rxgains2gelnagaina1", "0"},
				{"rxgains2gelnagaina2", "0"},
				{"rxgains2gtrelnabypa", "0"},
				{"rxgains2gtrisoa0", "0"},
				{"rxgains2gtrisoa1", "0"},
				{"rxgains2gtrisoa2", "0"},
				{"rxgains5gelnagaina0", "3"},
				{"rxgains5gelnagaina1", "3"},
				{"rxgains5gelnagaina2", "3"},
				{"rxgains5ghelnagaina", "7"},
				{"rxgains5ghtrelnabyp", "1"},
				{"rxgains5ghtrisoa0", "15"},
				{"rxgains5ghtrisoa1", "15"},
				{"rxgains5ghtrisoa2", "15"},
				{"rxgains5gmelnagaina", "7"},
				{"rxgains5gmtrelnabyp", "1"},
				{"rxgains5gmtrisoa0", "15"},
				{"rxgains5gmtrisoa1", "15"},
				{"rxgains5gmtrisoa2", "15"},
				{"rxgains5gtrelnabypa", "1"},
				{"rxgains5gtrisoa0", "6"},
				{"rxgains5gtrisoa1", "6"},
				{"rxgains5gtrisoa2", "6"},
				{"sar2g", "18"},
				{"sar5g", "15"},
				{"sb20in40hrpo", "0"},
				{"sb20in40lrpo", "0"},
				{"sb20in80and160hr5gh", "0"},
				{"sb20in80and160hr5gl", "0"},
				{"sb20in80and160hr5gm", "0"},
				{"sb20in80and160lr5gh", "0"},
				{"sb20in80and160lr5gl", "0"},
				{"sb20in80and160lr5gm", "0"},
				{"sb40and80hr5ghpo", "0"},
				{"sb40and80hr5glpo", "0"},
				{"sb40and80hr5gmpo", "0"},
				{"sb40and80lr5ghpo", "0"},
				{"sb40and80lr5glpo", "0"},
				{"sb40and80lr5gmpo", "0"},
				{"sromrev", "11"},
				{"subband5gver", "0x4"},
				{"tempcorrx", "0x3f"},
				{"tempoffset", "255"},
				{"temps_hysteresis", "15"},
				{"temps_period", "15"},
				{"tempsense_option", "0x3"},
				{"tempsense_slope", "0xff"},
				{"tempthresh", "255"},
				{"tssiposslope2g", "1"},
				{"tssiposslope5g", "1"},
				{"tworangetssi2g", "0"},
				{"tworangetssi5g", "0"},
				{"txchain", "3"},
				{"vendid", "0x14E4"},
				{"venid", "0x14E4"},
				{0, 0}
			};
			struct nvram_param *t;
			t = dir860_1_1params;
			while (t->name) {
				nvram_nset(t->value, "pci/1/1/%s", t->name);
				t++;
			}
			t = dir860_2_1params;
			while (t->name) {
				nvram_nset(t->value, "pci/2/1/%s", t->name);
				t++;
			}

		}

	break;
	case ROUTER_DLINK_DIR868:
	case ROUTER_DLINK_DIR865:

		if (nvram_get("pci/1/1/venid") == NULL) {

			char buf[64];
			FILE *fp = popen("cat /dev/mtdblock0|grep lanmac", "r");
			fread(buf, 1, 24, fp);
			pclose(fp);
			buf[24] = 0;
			fprintf(stderr, "set main mac %s\n", &buf[7]);
			nvram_set("et0macaddr", &buf[7]);

			fp = popen("cat /dev/mtdblock0|grep wlan5mac", "r");
			fread(buf, 1, 26, fp);
			pclose(fp);
			buf[26] = 0;
			fprintf(stderr, "set 5g mac %s\n", &buf[9]);
			nvram_set("pci/2/0/macaddr", &buf[9]);
			nvram_set("pci/2/1/macaddr", &buf[9]);
			fp = popen("cat /dev/mtdblock0|grep wlan24mac", "r");
			fread(buf, 1, 27, fp);
			pclose(fp);
			buf[27] = 0;
			fprintf(stderr, "set 2.4g mac %s\n", &buf[10]);
			nvram_set("pci/1/0/macaddr", &buf[10]);
			nvram_set("pci/1/1/macaddr", &buf[10]);

			struct nvram_param dir868_1_1params[] = {
				{"maxp2ga0", "0x56"},
				{"maxp2ga1", "0x56"},
				{"maxp2ga2", "0x56"},
				{"cckbw202gpo", "0x0000"},
				{"cckbw20ul2gpo", "0x0000"},
				{"legofdmbw202gpo", "0x00000000"},
				{"legofdmbw20ul2gpo", "0x00000000"},
				{"mcsbw202gpo", "0x00000000"},
				{"mcsbw20ul2gpo", "0x00000000"},
				{"mcsbw402gpo", "0x22222222"},
				{"pa2gw0a0", "0xFE7C"},
				{"pa2gw1a0", "0x1C9B"},
				{"pa2gw2a0", "0xF915"},
				{"pa2gw0a1", "0xFE85"},
				{"pa2gw1a1", "0x1D25"},
				{"pa2gw2a1", "0xF906"},
				{"pa2gw0a2", "0xFE82"},
				{"pa2gw1a2", "0x1D45"},
				{"pa2gw2a2", "0xF900"},
				{"ag0", "0x0"},
				{"ag1", "0x0"},
				{0, 0}
			};

			struct nvram_param dir868_2_1params[] = {
				{"sromrev", "11"},
				{"venid", "0x14E4"},
				{"venid", "0x14E4"},
				{"boardvendor", "0x14E4"},
				{"devid", "0x43a2"},
				{"boardrev", "0x1450"},
				{"boardflags", "0x30000000"},
				{"boardflags2", "0x300002"},
				{"boardtype", "0x621"},
				{"boardflags3", "0x0"},
				{"boardnum", "0"},
				{"ccode", "20785"},
				{"regrev", "27"},
				{"aa2g", "0"},
				{"aa5g", "7"},
				{"agbg0", "0x0"},
				{"agbg1", "0x0"},
				{"agbg2", "0x0"},
				{"aga0", "0x0"},
				{"aga1", "0x0"},
				{"aga2", "0x0"},
				{"txchain", "7"},
				{"rxchain", "7"},
				{"antswitch", "0"},
				{"tssiposslope2g", "1"},
				{"epagain2g", "0"},
				{"pdgain2g", "4"},
				{"tworangetssi2g", "0"},
				{"papdcap2g", "0"},
				{"femctrl", "3"},
				{"tssiposslope5g", "1"},
				{"epagain5g", "0"},
				{"pdgain5g", "4"},
				{"tworangetssi5g", "0"},
				{"papdcap5g", "0"},
				{"gainctrlsph", "0"},
				{"tempthresh", "255"},
				{"tempoffset", "255"},
				{"rawtempsense", "0x1ff"},
				{"measpower", "0x7f"},
				{"tempsense_slope", "0xff"},
				{"tempcorrx", "0x3f"},
				{"tempsense_option", "0x3"},
				{"phycal_tempdelta", "255"},
				{"temps_period", "15"},
				{"temps_hysteresis", "15"},
				{"measpower1", "0x7f"},
				{"measpower2", "0x7f"},
				{"pdoffset40ma0", "4369"},
				{"pdoffset40ma1", "4369"},
				{"pdoffset40ma2", "4369"},
				{"pdoffset80ma0", "0"},
				{"pdoffset80ma1", "0"},
				{"pdoffset80ma2", "0"},
				{"subband5gver", "0x4"},
				{"subvid", "0x14e4"},
				{"cckbw202gpo", "0"},
				{"cckbw20ul2gpo", "0"},
				{"mcsbw202gpo", "0"},
				{"mcsbw402gpo", "0"},
				{"dot11agofdmhrbw202g", "0"},
				{"ofdmlrbw202gpo", "0"},
				{"mcsbw205glpo", "3398914833"},
				{"mcsbw405glpo", "3398914833"},
				{"mcsbw805glpo", "3398914833"},
				{"mcsbw1605glpo", "0"},
				{"mcsbw205gmpo", "3398914833"},
				{"mcsbw405gmpo", "3398914833"},
				{"mcsbw805gmpo", "3398914833"},
				{"mcsbw1605gmpo", "0"},
				{"mcsbw205ghpo", "3398914833"},
				{"mcsbw405ghpo", "3398914833"},
				{"mcsbw805ghpo", "3398914833"},
				{"mcsbw1605ghpo", "0"},
				{"mcslr5glpo", "0"},
				{"mcslr5gmpo", "0"},
				{"mcslr5ghpo", "0"},
				{"sb20in40hrpo", "0"},
				{"sb20in80and160hr5gl", "0"},
				{"sb40and80hr5glpo", "0"},
				{"sb20in80and160hr5gm", "0"},
				{"sb40and80hr5gmpo", "0"},
				{"sb20in80and160hr5gh", "0"},
				{"sb40and80hr5ghpo", "0"},
				{"sb20in40lrpo", "0"},
				{"sb20in80and160lr5gl", "0"},
				{"sb40and80lr5glpo", "0"},
				{"sb20in80and160lr5gm", "0"},
				{"sb40and80lr5gmpo", "0"},
				{"sb20in80and160lr5gh", "0"},
				{"sb40and80lr5ghpo", "0"},
				{"dot11agduphrpo", "0"},
				{"dot11agduplrpo", "0"},
				{"pcieingress_war", "15"},
				{"sar2g", "18"},
				{"sar5g", "15"},
				{"noiselvl2ga0", "31"},
				{"noiselvl2ga1", "31"},
				{"noiselvl2ga2", "31"},
				{"noiselvl5ga0", "31,31,31,31"},
				{"noiselvl5ga1", "31,31,31,31"},
				{"noiselvl5ga2", "31,31,31,31"},
				{"rxgainerr2ga0", "63"},
				{"rxgainerr2ga1", "31"},
				{"rxgainerr2ga2", "31"},
				{"rxgainerr5ga0", "63,63,63,63"},
				{"rxgainerr5ga1", "31,31,31,31"},
				{"rxgainerr5ga2", "31,31,31,31"},
				{"maxp2ga0", "76"},
				{"pa2ga0", "0xfe72,0x14c0,0xfac7"},
				{"rxgains5gmelnagaina0", "2"},
				{"rxgains5gmtrisoa0", "5"},
				{"rxgains5gmtrelnabypa0", "1"},
				{"rxgains5ghelnagaina0", "2"},
				{"rxgains5ghtrisoa0", "5"},
				{"rxgains5ghtrelnabypa0", "1"},
				{"rxgains2gelnagaina0", "0"},
				{"rxgains2gtrisoa0", "0"},
				{"rxgains2gtrelnabypa0", "0"},
				{"rxgains5gelnagaina0", "1"},
				{"rxgains5gtrisoa0", "7"},
				{"rxgains5gtrelnabypa0", "1"},
				{"maxp5ga0", "92,92,92,92"},
				{"pa5ga0", "0xff26,0x188e,0xfcf0,0xff2a,0x18ee,0xfcec,0xff21,0x18b4,0xfcec,0xff23,0x1930,0xfcdd"},
				{"maxp2ga1", "76"},
				{"pa2ga1", "0xfe80,0x1472,0xfabc"},
				{"rxgains5gmelnagaina1", "2"},
				{"rxgains5gmtrisoa1", "4"},
				{"rxgains5gmtrelnabypa1", "1"},
				{"rxgains5ghelnagaina1", "2"},
				{"rxgains5ghtrisoa1", "4"},
				{"rxgains5ghtrelnabypa1", "1"},
				{"rxgains2gelnagaina1", "0"},
				{"rxgains2gtrisoa1", "0"},
				{"rxgains2gtrelnabypa1", "0"},
				{"rxgains5gelnagaina1", "1"},
				{"rxgains5gtrisoa1", "6"},
				{"rxgains5gtrelnabypa1", "1"},
				{"maxp5ga1", "92,92,92,92"},
				{"pa5ga1", "0xff35,0x1a3c,0xfccc,0xff31,0x1a06,0xfccf,0xff2b,0x1a54,0xfcc5,0xff30,0x1ad5,0xfcb9"},
				{"maxp2ga2", "76"},
				{"pa2ga2", "0xfe82,0x14bf,0xfad9"},
				{"rxgains5gmelnagaina2", "3"},
				{"rxgains5gmtrisoa2", "4"},
				{"rxgains5gmtrelnabypa2", "1"},
				{"rxgains5ghelnagaina2", "3"},
				{"rxgains5ghtrisoa2", "4"},
				{"rxgains5ghtrelnabypa2", "1"},
				{"rxgains2gelnagaina2", "0"},
				{"rxgains2gtrisoa2", "0"},
				{"rxgains2gtrelnabypa2", "0"},
				{"rxgains5gelnagaina2", "1"},
				{"rxgains5gtrisoa2", "5"},
				{"rxgains5gtrelnabypa2", "1"},
				{"maxp5ga2", "92,92,92,92"},
				{"pa5ga2", "0xff2e,0x197b,0xfcd8,0xff2d,0x196e,0xfcdc,0xff30,0x1a7d,0xfcc2,0xff2e,0x1ac6,0xfcb4"},
				{0, 0}
			};

			struct nvram_param *t;
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

			nvram_set("pci/1/1/venid", "0x14E4");
			nvram_commit();

		}
		set_gpio(7, 1);	// fixup wifi button
		set_gpio(15, 1);	// fixup ses button
		break;
	case ROUTER_LINKSYS_EA6700:
		if (nvram_get("0:aa2g") == NULL) {
			struct nvram_param ea6700_1_1params[] = {
				{"aa2g", "7"},
				{"ag0", "0"},
				{"ag1", "0"},
				{"ag2", "0"},
				{"antswctl2g", "0"},
				{"antswitch", "0"},
				{"boardflags2", "0x00100000"},
				{"boardflags", "0x80001200"},
				{"cckbw202gpo", "0x4444"},
				{"cckbw20ul2gpo", "0x4444"},
				{"ccode", "Q2"},
				{"devid", "0x4332"},
				{"elna2g", "2"},
				{"extpagain2g", "3"},
				{"ledbh0", "11"},
				{"ledbh12", "2"},
				{"ledbh1", "11"},
				{"ledbh2", "11"},
				{"ledbh3", "11"},
				{"leddc", "0xFFFF"},
				{"legofdm40duppo", "0"},
				{"legofdmbw202gpo", "0x55553300"},
				{"legofdmbw20ul2gpo", "0x55553300"},
				{"maxp2ga0", "0x60"},
				{"maxp2ga1", "0x60"},
				{"maxp2ga2", "0x60"},
				{"mcs32po", "0x0006"},
				{"mcsbw202gpo", "0xAA997755"},
				{"mcsbw20ul2gpo", "0xAA997755"},
				{"mcsbw402gpo", "0xAA997755"},
				{"pa2gw0a0", "0xFE7C"},
				{"pa2gw0a1", "0xFE85"},
				{"pa2gw0a2", "0xFE82"},
				{"pa2gw1a0", "0x1E9B"},
				{"pa2gw1a1", "0x1EA5"},
				{"pa2gw1a2", "0x1EC5"},
				{"pa2gw2a0", "0xF8B4"},
				{"pa2gw2a1", "0xF8C0"},
				{"pa2gw2a2", "0xF8B8"},
				{"parefldovoltage", "60"},
				{"pdetrange2g", "3"},
				{"phycal_tempdelta", "0"},
				{"regrev", "33"},
				{"rxchain", "7"},
				{"sromrev", "9"},
				{"temps_hysteresis", "5"},
				{"temps_period", "5"},
				{"tempthresh", "120"},
				{"tssipos2g", "1"},
				{"txchain", "7"},
				{"venid", "0x14E4"},
				{"xtalfreq", "20000"},
				{0, 0}
			};

			struct nvram_param ea6700_2_1params[] = {
				{"aa2g", "7"},
				{"aa5g", "7"},
				{"aga0", "0"},
				{"aga1", "0"},
				{"aga2", "0"},
				{"antswitch", "0"},
				{"boardflags2", "0x00200002"},
				{"boardflags3", "0"},
				{"boardflags", "0x30000000"},
				{"ccode", "Q2"},
				{"devid", "0x43A2"},
				{"dot11agduphrpo", "0"},
				{"dot11agduplrpo", "0"},
				{"epagain5g", "0"},
				{"femctrl", "3"},
				{"gainctrlsph", "0"},
				{"ledbh0", "11"},
				{"ledbh10", "2"},
				{"ledbh1", "11"},
				{"ledbh2", "11"},
				{"ledbh3", "11"},
				{"leddc", "0xFFFF"},
				{"maxp5ga0", "0x5C,0x5C,0x5C,0x5C"},
				{"maxp5ga1", "0x5C,0x5C,0x5C,0x5C"},
				{"maxp5ga2", "0x5C,0x5C,0x5C,0x5C"},
				{"mcsbw1605ghpo", "0"},
				{"mcsbw1605glpo", "0"},
				{"mcsbw1605gmpo", "0"},
				{"mcsbw205ghpo", "0xDD553300"},
				{"mcsbw205glpo", "0xDD553300"},
				{"mcsbw205gmpo", "0xDD553300"},
				{"mcsbw405ghpo", "0xEE885544"},
				{"mcsbw405glpo", "0xEE885544"},
				{"mcsbw405gmpo", "0xEE885544"},
				{"mcsbw805ghpo", "0xEE885544"},
				{"mcsbw805glpo", "0xEE885544"},
				{"mcsbw805gmpo", "0xEE885544"},
				{"mcslr5ghpo", "0"},
				{"mcslr5glpo", "0"},
				{"mcslr5gmpo", "0"},
				{"pa5ga0", "0xff2b,0x1898,0xfcf2,0xff2c,0x1947,0xfcda,0xff33,0x18f9,0xfcec,0xff2d,0x18ef,0xfce4"},
				{"pa5ga1", "0xff31,0x1930,0xfce3,0xff30,0x1974,0xfcd9,0xff31,0x18db,0xfcee,0xff37,0x194e,0xfce1"},
				{"pa5ga2", "0xff2e,0x193c,0xfcde,0xff2c,0x1831,0xfcf9,0xff30,0x18c6,0xfcef,0xff30,0x1942,0xfce0"},
				{"papdcap5g", "0"},
				{"pdgain5g", "4"},
				{"pdoffset40ma0", "0x1111"},
				{"pdoffset40ma1", "0x1111"},
				{"pdoffset40ma2", "0x1111"},
				{"pdoffset80ma0", "0"},
				{"pdoffset80ma1", "0"},
				{"pdoffset80ma2", "0"},
				{"phycal_tempdelta", "0"},
				{"regrev", "33"},
				{"rxchain", "7"},
				{"rxgains5gelnagaina0", "1"},
				{"rxgains5gelnagaina1", "1"},
				{"rxgains5gelnagaina2", "1"},
				{"rxgains5ghelnagaina0", "2"},
				{"rxgains5ghelnagaina1", "2"},
				{"rxgains5ghelnagaina2", "3"},
				{"rxgains5ghtrelnabypa0", "1"},
				{"rxgains5ghtrelnabypa1", "1"},
				{"rxgains5ghtrelnabypa2", "1"},
				{"rxgains5ghtrisoa0", "5"},
				{"rxgains5ghtrisoa1", "4"},
				{"rxgains5ghtrisoa2", "4"},
				{"rxgains5gmelnagaina0", "2"},
				{"rxgains5gmelnagaina1", "2"},
				{"rxgains5gmelnagaina2", "3"},
				{"rxgains5gmtrelnabypa0", "1"},
				{"rxgains5gmtrelnabypa1", "1"},
				{"rxgains5gmtrelnabypa2", "1"},
				{"rxgains5gmtrisoa0", "5"},
				{"rxgains5gmtrisoa1", "4"},
				{"rxgains5gmtrisoa2", "4"},
				{"rxgains5gtrelnabypa0", "1"},
				{"rxgains5gtrelnabypa1", "1"},
				{"rxgains5gtrelnabypa2", "1"},
				{"rxgains5gtrisoa0", "7"},
				{"rxgains5gtrisoa1", "6"},
				{"rxgains5gtrisoa2", "5"},
				{"sar2g", "18"},
				{"sar5g", "15"},
				{"sb20in40hrpo", "0"},
				{"sb20in40lrpo", "0"},
				{"sb20in80and160hr5ghpo", "0"},
				{"sb20in80and160hr5glpo", "0"},
				{"sb20in80and160hr5gmpo", "0"},
				{"sb20in80and160lr5ghpo", "0"},
				{"sb20in80and160lr5glpo", "0"},
				{"sb20in80and160lr5gmpo", "0"},
				{"sb40and80hr5ghpo", "0"},
				{"sb40and80hr5glpo", "0"},
				{"sb40and80hr5gmpo", "0"},
				{"sb40and80lr5ghpo", "0"},
				{"sb40and80lr5glpo", "0"},
				{"sb40and80lr5gmpo", "0"},
				{"sromrev", "11"},
				{"subband5gver", "4"},
				{"tempoffset", "0"},
				{"temps_hysteresis", "5"},
				{"temps_period", "5"},
				{"tempthresh", "120"},
				{"tssiposslope5g", "1"},
				{"tworangetssi5g", "0"},
				{"txchain", "7"},
				{"venid", "0x14E4"},
				{"xtalfreq", "40000"},
				{0, 0}
			};

			struct nvram_param *t;
			t = ea6700_1_1params;
			while (t->name) {
				nvram_nset(t->value, "0:%s", t->name);
				t++;
			}
			t = ea6700_2_1params;
			while (t->name) {
				nvram_nset(t->value, "1:%s", t->name);
				t++;
			}

			nvram_set("acs_2g_ch_no_ovlp", "1");
			nvram_set("acs_2g_ch_no_restrict", "1");
			nvram_set("devpath0", "pci/1/1/");
			nvram_set("devpath1", "pci/2/1/");
			nvram_commit();
		}
		nvram_set("partialboots", "0");
	case ROUTER_LINKSYS_EA6500V2:
		if (nvram_get("0:aa2g") == NULL) {
			struct nvram_param ea6500v2_1_1params[] = {
				{"aa2g", "7"},
				{"ag0", "0"},
				{"ag1", "0"},
				{"ag2", "0"},
				{"antswctl2g", "0"},
				{"antswitch", "0"},
				{"boardflags2", "0x00100000"},
				{"boardflags", "0x80001200"},
				{"cckbw202gpo", "0x4444"},
				{"cckbw20ul2gpo", "0x4444"},
				{"ccode", "Q2"},
				{"devid", "0x4332"},
				{"elna2g", "2"},
				{"extpagain2g", "3"},
				{"ledbh0", "11"},
				{"ledbh12", "2"},
				{"ledbh1", "11"},
				{"ledbh2", "11"},
				{"ledbh3", "11"},
				{"leddc", "0xFFFF"},
				{"legofdm40duppo", "0"},
				{"legofdmbw202gpo", "0x55553300"},
				{"legofdmbw20ul2gpo", "0x55553300"},
				{"maxp2ga0", "0x60"},
				{"maxp2ga1", "0x60"},
				{"maxp2ga2", "0x60"},
				{"mcs32po", "0x0006"},
				{"mcsbw202gpo", "0xAA997755"},
				{"mcsbw20ul2gpo", "0xAA997755"},
				{"mcsbw402gpo", "0xAA997755"},
				{"pa2gw0a0", "0xFE7C"},
				{"pa2gw0a1", "0xFE85"},
				{"pa2gw0a2", "0xFE82"},
				{"pa2gw1a0", "0x1E9B"},
				{"pa2gw1a1", "0x1EA5"},
				{"pa2gw1a2", "0x1EC5"},
				{"pa2gw2a0", "0xF8B4"},
				{"pa2gw2a1", "0xF8C0"},
				{"pa2gw2a2", "0xF8B8"},
				{"parefldovoltage", "60"},
				{"pdetrange2g", "3"},
				{"phycal_tempdelta", "0"},
				{"regrev", "33"},
				{"rxchain", "7"},
				{"sromrev", "9"},
				{"temps_hysteresis", "5"},
				{"temps_period", "5"},
				{"tempthresh", "120"},
				{"tssipos2g", "1"},
				{"txchain", "7"},
				{"venid", "0x14E4"},
				{"xtalfreq", "20000"},
				{0, 0}
			};

			struct nvram_param ea6500v2_2_1params[] = {
				{"aa2g", "7"},
				{"aa5g", "7"},
				{"aga0", "0"},
				{"aga1", "0"},
				{"aga2", "0"},
				{"antswitch", "0"},
				{"boardflags2", "0x00200002"},
				{"boardflags3", "0"},
				{"boardflags", "0x30000000"},
				{"ccode", "Q2"},
				{"devid", "0x43A2"},
				{"dot11agduphrpo", "0"},
				{"dot11agduplrpo", "0"},
				{"epagain5g", "0"},
				{"femctrl", "3"},
				{"gainctrlsph", "0"},
				{"ledbh0", "11"},
				{"ledbh10", "2"},
				{"ledbh1", "11"},
				{"ledbh2", "11"},
				{"ledbh3", "11"},
				{"leddc", "0xFFFF"},
				{"maxp5ga0", "0x5C,0x5C,0x5C,0x5C"},
				{"maxp5ga1", "0x5C,0x5C,0x5C,0x5C"},
				{"maxp5ga2", "0x5C,0x5C,0x5C,0x5C"},
				{"mcsbw1605ghpo", "0"},
				{"mcsbw1605glpo", "0"},
				{"mcsbw1605gmpo", "0"},
				{"mcsbw205ghpo", "0xDD553300"},
				{"mcsbw205glpo", "0xDD553300"},
				{"mcsbw205gmpo", "0xDD553300"},
				{"mcsbw405ghpo", "0xEE885544"},
				{"mcsbw405glpo", "0xEE885544"},
				{"mcsbw405gmpo", "0xEE885544"},
				{"mcsbw805ghpo", "0xEE885544"},
				{"mcsbw805glpo", "0xEE885544"},
				{"mcsbw805gmpo", "0xEE885544"},
				{"mcslr5ghpo", "0"},
				{"mcslr5glpo", "0"},
				{"mcslr5gmpo", "0"},
				{"pa5ga0", "0xff2b,0x1898,0xfcf2,0xff2c,0x1947,0xfcda,0xff33,0x18f9,0xfcec,0xff2d,0x18ef,0xfce4"},
				{"pa5ga1", "0xff31,0x1930,0xfce3,0xff30,0x1974,0xfcd9,0xff31,0x18db,0xfcee,0xff37,0x194e,0xfce1"},
				{"pa5ga2", "0xff2e,0x193c,0xfcde,0xff2c,0x1831,0xfcf9,0xff30,0x18c6,0xfcef,0xff30,0x1942,0xfce0"},
				{"papdcap5g", "0"},
				{"pdgain5g", "4"},
				{"pdoffset40ma0", "0x1111"},
				{"pdoffset40ma1", "0x1111"},
				{"pdoffset40ma2", "0x1111"},
				{"pdoffset80ma0", "0"},
				{"pdoffset80ma1", "0"},
				{"pdoffset80ma2", "0"},
				{"phycal_tempdelta", "0"},
				{"regrev", "33"},
				{"rxchain", "7"},
				{"rxgains5gelnagaina0", "1"},
				{"rxgains5gelnagaina1", "1"},
				{"rxgains5gelnagaina2", "1"},
				{"rxgains5ghelnagaina0", "2"},
				{"rxgains5ghelnagaina1", "2"},
				{"rxgains5ghelnagaina2", "3"},
				{"rxgains5ghtrelnabypa0", "1"},
				{"rxgains5ghtrelnabypa1", "1"},
				{"rxgains5ghtrelnabypa2", "1"},
				{"rxgains5ghtrisoa0", "5"},
				{"rxgains5ghtrisoa1", "4"},
				{"rxgains5ghtrisoa2", "4"},
				{"rxgains5gmelnagaina0", "2"},
				{"rxgains5gmelnagaina1", "2"},
				{"rxgains5gmelnagaina2", "3"},
				{"rxgains5gmtrelnabypa0", "1"},
				{"rxgains5gmtrelnabypa1", "1"},
				{"rxgains5gmtrelnabypa2", "1"},
				{"rxgains5gmtrisoa0", "5"},
				{"rxgains5gmtrisoa1", "4"},
				{"rxgains5gmtrisoa2", "4"},
				{"rxgains5gtrelnabypa0", "1"},
				{"rxgains5gtrelnabypa1", "1"},
				{"rxgains5gtrelnabypa2", "1"},
				{"rxgains5gtrisoa0", "7"},
				{"rxgains5gtrisoa1", "6"},
				{"rxgains5gtrisoa2", "5"},
				{"sar2g", "18"},
				{"sar5g", "15"},
				{"sb20in40hrpo", "0"},
				{"sb20in40lrpo", "0"},
				{"sb20in80and160hr5ghpo", "0"},
				{"sb20in80and160hr5glpo", "0"},
				{"sb20in80and160hr5gmpo", "0"},
				{"sb20in80and160lr5ghpo", "0"},
				{"sb20in80and160lr5glpo", "0"},
				{"sb20in80and160lr5gmpo", "0"},
				{"sb40and80hr5ghpo", "0"},
				{"sb40and80hr5glpo", "0"},
				{"sb40and80hr5gmpo", "0"},
				{"sb40and80lr5ghpo", "0"},
				{"sb40and80lr5glpo", "0"},
				{"sb40and80lr5gmpo", "0"},
				{"sromrev", "11"},
				{"subband5gver", "4"},
				{"tempoffset", "0"},
				{"temps_hysteresis", "5"},
				{"temps_period", "5"},
				{"tempthresh", "120"},
				{"tssiposslope5g", "1"},
				{"tworangetssi5g", "0"},
				{"txchain", "7"},
				{"venid", "0x14E4"},
				{"xtalfreq", "40000"},
				{0, 0},
			};

			struct nvram_param *t;
			t = ea6500v2_1_1params;
			while (t->name) {
				nvram_nset(t->value, "0:%s", t->name);
				t++;
			}
			t = ea6500v2_2_1params;
			while (t->name) {
				nvram_nset(t->value, "1:%s", t->name);
				t++;
			}

			nvram_set("acs_2g_ch_no_ovlp", "1");
			nvram_set("acs_2g_ch_no_restrict", "1");
			nvram_set("devpath0", "pci/1/1/");
			nvram_set("devpath1", "pci/2/1/");
			nvram_commit();
		}
		nvram_set("partialboots", "0");
		break;
	case ROUTER_LINKSYS_EA6900:
		if (nvram_get("0:aa2g") == NULL) {

			struct nvram_param ea6900_1_1params[] = {
				{"aa2g", "7"},
				{"agbg0", "0x47"},
				{"agbg1", "0x47"},
				{"agbg2", "0x47"},
				{"antswitch", "0"},
				{"boardflags2", "0x00100002"},
				{"boardflags3", "0x00000003"},
				{"boardflags", "0x00001000"},
				{"cckbw202gpo", "0x0"},
				{"cckbw20ul2gpo", "0x0"},
				{"ccode", "Q2"},
				{"devid", "0x43A1"},
				{"dot11agduphrpo", "0x0"},
				{"dot11agduplrpo", "0x0"},
				{"dot11agofdmhrbw202gpo", "0x6666"},
				{"epagain2g", "0"},
				{"femctrl", "3"},
				{"gainctrlsph", "0"},
				{"ledbh0", "0xFF"},
				{"ledbh10", "2"},
				{"ledbh1", "0xFF"},
				{"ledbh2", "0xFF"},
				{"ledbh3", "0xFF"},
				{"leddc", "0xFFFF"},
				{"maxp2ga0", "0x62"},
				{"maxp2ga1", "0x62"},
				{"maxp2ga2", "0x62"},
				{"mcsbw202gpo", "0xCC666600"},
				{"mcsbw402gpo", "0xCC666600"},
				{"ofdmlrbw202gpo", "0x0"},
				{"pa2ga0", "0xff22,0x1a4f,0xfcc1"},
				{"pa2ga1", "0xff22,0x1a71,0xfcbb"},
				{"pa2ga2", "0xff1f,0x1a21,0xfcc2"},
				{"papdcap2g", "0"},
				{"parefldovoltage", "35"},
				{"pdgain2g", "14"},
				{"pdoffset2g40ma0", "0x3"},
				{"pdoffset2g40ma1", "0x3"},
				{"pdoffset2g40ma2", "0x3"},
				{"phycal_tempdelta", "0"},
				{"regrev", "54"},
				{"rpcal2g", "53985"},
				{"rxchain", "7"},
				{"rxgains2gelnagaina0", "4"},
				{"rxgains2gelnagaina1", "4"},
				{"rxgains2gelnagaina2", "4"},
				{"rxgains2gtrelnabypa0", "1"},
				{"rxgains2gtrelnabypa1", "1"},
				{"rxgains2gtrelnabypa2", "1"},
				{"rxgains2gtrisoa0", "7"},
				{"rxgains2gtrisoa1", "7"},
				{"rxgains2gtrisoa2", "7"},
				{"sb20in40hrpo", "0x0"},
				{"sb20in40lrpo", "0x0"},
				{"sromrev", "11"},
				{"tempoffset", "0"},
				{"temps_hysteresis", "5"},
				{"temps_period", "5"},
				{"tempthresh", "120"},
				{"tssiposslope2g", "1"},
				{"tworangetssi2g", "0"},
				{"txchain", "7"},
				{"venid", "0x14E4"},
				{"xtalfreq", "40000"},
				{0, 0}
			};

			struct nvram_param ea6900_2_1params[] = {
				{"aa5g", "7"},
				{"aga0", "0"},
				{"aga1", "0"},
				{"aga2", "0"},
				{"antswitch", "0"},
				{"boardflags2", "0x00200002"},
				{"boardflags3", "0x00000000"},
				{"boardflags", "0x30000000"},
				{"ccode", "Q2"},
				{"devid", "0x43A2"},
				{"dot11agduphrpo", "0x0"},
				{"dot11agduplrpo", "0x0"},
				{"epagain5g", "0"},
				{"femctrl", "3"},
				{"gainctrlsph", "0"},
				{"ledbh0", "11"},
				{"ledbh10", "2"},
				{"ledbh1", "11"},
				{"ledbh2", "11"},
				{"ledbh3", "11"},
				{"leddc", "0xFFFF"},
				{"maxp5ga0", "0x5C,0x5C,0x5C,0x5C"},
				{"maxp5ga1", "0x5C,0x5C,0x5C,0x5C"},
				{"maxp5ga2", "0x5C,0x5C,0x5C,0x5C"},
				{"mcsbw205ghpo", "0xBB555500"},
				{"mcsbw205glpo", "0xBB555500"},
				{"mcsbw205gmpo", "0xBB555500"},
				{"mcsbw405ghpo", "0xBB777700"},
				{"mcsbw405glpo", "0xBB777700"},
				{"mcsbw405gmpo", "0xBB777700"},
				{"mcsbw805ghpo", "0xBB777700"},
				{"mcsbw805glpo", "0xBB777733"},
				{"mcsbw805gmpo", "0xBB777700"},
				{"mcslr5ghpo", "0x0"},
				{"mcslr5glpo", "0x0"},
				{"mcslr5gmpo", "0x0"},
				{"pa5ga0", "0xff2e,0x185a,0xfcfc,0xff37,0x1903,0xfcf1,0xff4b,0x197f,0xfcff,0xff37,0x180f,0xfd12"},
				{"pa5ga1", "0xff33,0x1944,0xfce5,0xff30,0x18c6,0xfcf5,0xff40,0x19c7,0xfce5,0xff38,0x18cc,0xfcf9"},
				{"pa5ga2", "0xff34,0x1962,0xfce1,0xff35,0x193b,0xfceb,0xff38,0x1921,0xfcf1,0xff39,0x188f,0xfd00"},
				{"papdcap5g", "0"},
				{"parefldovoltage", "35"},
				{"pdgain5g", "4"},
				{"pdoffset40ma0", "0x1111"},
				{"pdoffset40ma1", "0x1111"},
				{"pdoffset40ma2", "0x1111"},
				{"pdoffset80ma0", "0xEEEE"},
				{"pdoffset80ma1", "0xEEEE"},
				{"pdoffset80ma2", "0xEEEE"},
				{"phycal_tempdelta", "0"},
				{"regrev", "54"},
				{"rpcal5gb0", "41773"},
				{"rpcal5gb3", "42547"},
				{"rxchain", "7"},
				{"rxgains5gelnagaina0", "1"},
				{"rxgains5gelnagaina1", "1"},
				{"rxgains5gelnagaina2", "1"},
				{"rxgains5ghelnagaina0", "2"},
				{"rxgains5ghelnagaina1", "2"},
				{"rxgains5ghelnagaina2", "3"},
				{"rxgains5ghtrelnabypa0", "1"},
				{"rxgains5ghtrelnabypa1", "1"},
				{"rxgains5ghtrelnabypa2", "1"},
				{"rxgains5ghtrisoa0", "5"},
				{"rxgains5ghtrisoa1", "4"},
				{"rxgains5ghtrisoa2", "4"},
				{"rxgains5gmelnagaina0", "2"},
				{"rxgains5gmelnagaina1", "2"},
				{"rxgains5gmelnagaina2", "3"},
				{"rxgains5gmtrelnabypa0", "1"},
				{"rxgains5gmtrelnabypa1", "1"},
				{"rxgains5gmtrelnabypa2", "1"},
				{"rxgains5gmtrisoa0", "5"},
				{"rxgains5gmtrisoa1", "4"},
				{"rxgains5gmtrisoa2", "4"},
				{"rxgains5gtrelnabypa0", "1"},
				{"rxgains5gtrelnabypa1", "1"},
				{"rxgains5gtrelnabypa2", "1"},
				{"rxgains5gtrisoa0", "7"},
				{"rxgains5gtrisoa1", "6"},
				{"rxgains5gtrisoa2", "5"},
				{"sb20in40hrpo", "0x0"},
				{"sb20in40lrpo", "0x0"},
				{"sb20in80and160hr5ghpo", "0x0"},
				{"sb20in80and160hr5glpo", "0x0"},
				{"sb20in80and160hr5gmpo", "0x0"},
				{"sb20in80and160lr5ghpo", "0x0"},
				{"sb20in80and160lr5glpo", "0x0"},
				{"sb20in80and160lr5gmpo", "0x0"},
				{"sb40and80hr5ghpo", "0x0"},
				{"sb40and80hr5glpo", "0x0"},
				{"sb40and80hr5gmpo", "0x0"},
				{"sb40and80lr5ghpo", "0x0"},
				{"sb40and80lr5glpo", "0x0"},
				{"sb40and80lr5gmpo", "0x0"},
				{"sromrev", "11"},
				{"subband5gver", "4"},
				{"tempoffset", "0"},
				{"temps_hysteresis", "5"},
				{"temps_period", "5"},
				{"tempthresh", "120"},
				{"tssiposslope5g", "1"},
				{"tworangetssi5g", "0"},
				{"txchain", "7"},
				{"venid", "0x14E4"},
				{"xtalfreq", "40000"},
				{0, 0}
			};

			struct nvram_param *t;
			t = ea6900_1_1params;
			while (t->name) {
				nvram_nset(t->value, "0:%s", t->name);
				t++;
			}
			t = ea6900_2_1params;
			while (t->name) {
				nvram_nset(t->value, "1:%s", t->name);
				t++;
			}

			nvram_set("acs_2g_ch_no_ovlp", "1");
			nvram_set("acs_2g_ch_no_restrict", "1");
			nvram_set("devpath0", "pci/1/1/");
			nvram_set("devpath1", "pci/2/1/");
			nvram_commit();
		}
		nvram_set("partialboots", "0");
		break;
	case ROUTER_TPLINK_ARCHERC9:
		nvram_set("0:ledbh4", "7");
		nvram_set("1:ledbh5", "7");
		nvram_set("0:ledbh1", "11");
		nvram_set("1:ledbh1", "11");
		nvram_set("0:ledbh2", "11");
		nvram_set("1:ledbh2", "11");
		set_gpio(0, 1);	// fixup ses button
		break;
	case ROUTER_BUFFALO_WZR1750:
		nvram_set("0:ledbh12", "7");
		nvram_set("1:ledbh10", "7");
		set_gpio(12, 1);	// fixup ses button
		break;
	case ROUTER_BUFFALO_WXR1900DHP:
		nvram_set("0:ledbh12", "7");
		nvram_set("1:ledbh12", "7");
		set_gpio(16, 1);	// fixup ses button
		set_gpio(15, 1);	// fixup ses button
		break;
	case ROUTER_BUFFALO_WZR900DHP:
	case ROUTER_BUFFALO_WZR600DHP2:
		nvram_set("0:boardflags2", "0x1000");
		nvram_set("1:boardflags2", "0x00001000");
		nvram_set("0:ledbh12", "7");
		nvram_set("1:ledbh10", "7");
		set_gpio(9, 1);	// fixup ses button
		if (!nvram_match("loader_version", "v0.03")) {
			FILE *fp = fopen("/etc/cfe/cfe_600.bin", "rb");
			FILE *bp = fopen("/dev/mtdblock0", "rb");
			FILE *out = fopen("/tmp/cfe.bin", "wb");
			if (fp && bp && out) {
				int i;
				for (i = 0; i < 0x400; i++) {
					putc(getc(fp), out);
					getc(bp);
				}
				for (i = 0; i < 0x1000; i++) {
					putc(getc(bp), out);
					getc(fp);
				}
				int e = getc(fp);
				for (i = 0; e != EOF; i++) {
					putc(e, out);
					e = getc(fp);
				}
				fclose(out);
				fclose(fp);
				fclose(bp);
				out = NULL;
				bp = NULL;
				fp = NULL;
				fprintf(stderr, "update bootloader\n");
				sysprintf("mtd -f write /tmp/cfe.bin boot");
				fprintf(stderr, "reboot\n");
				sys_reboot();
			}
			if (fp)
				fclose(fp);
			if (bp)
				fclose(bp);
			if (out)
				fclose(out);
		}
		break;

	default:
		nvram_unset("et1macaddr");
		nvram_set("bootpartition", "0");

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
		writeproc("/proc/irq/111/smp_affinity", "2");
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

	insmod("emf");
	insmod("igs");
#ifdef HAVE_DHDAP
	insmod("dhd");
#endif
      insmod("wl");

	/*
	 * Set a sane date 
	 */
	stime(&tm);
//      nvram_set("wl0_ifname", "ath0");

	led_control(LED_POWER, LED_ON);
	led_control(LED_DIAG, LED_OFF);
	led_control(LED_SES, LED_OFF);
	led_control(LED_SES2, LED_OFF);
	led_control(LED_BRIDGE, LED_OFF);
	led_control(LED_WLAN0, LED_OFF);
	led_control(LED_WLAN1, LED_OFF);
	led_control(LED_WLAN2, LED_OFF);
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
	else if (!strcmp(vlan_wan_ports, "0 7u"))
		vlan7ports = "0t 7u";

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
