/*
 * lib3g.c
 *
 * Copyright (C) 2009 Sebastian Gottschall <gottschall@dd-wrt.com>
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
 * this module detects various special 3G/CDMA USB Devices which is required to handle them correct, since some devices
 * are using non standard tty interfaces. everything which is not handled here in this list, is supported by the default tty handling which means dialin on tty0
 */
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <shutils.h>
#include <utils.h>
#include <bcmnvram.h>

#define HSO 0xf0

enum serdrv {
	none,
	option,
	generic,
	qcserial,
	sierra,
	acm,
	hso
};

struct DEVICES {
	int vendor;
	int product;
	unsigned char driver;
	unsigned char controldevice;
	unsigned char datadevice;
	unsigned char modeswitch;
	void (*customsetup) (int needreset, int devicecount);
	char *name;
};

static struct DEVICES devicelist[];

static int scanFor(int Vendor, int Product)
{
#if defined(ARCH_broadcom) && !defined(HAVE_BCMMODERN)
	char grepstr[128];
	sprintf(grepstr, "grep Vendor=%x ProdID=%x /tmp/usb/devices|wc -l", Vendor, Product);
	FILE *check = popen(grepstr, "r");
	if (check) {
		int count = 0;
		fscanf(check, "%d", &count);
		pclose(check);
		if (count > 0) {
			eval("umount", "/tmp/usb");
			return 1;
		}
	}
	return 0;
#else
	int count = 1;
	int hub = 1;
	while (hub < 5) {
		char sysfs[64];
		sprintf(sysfs, "/sys/bus/usb/devices/%d-0:1.0/bInterfaceNumber", count);
		FILE *probe = fopen(sysfs, "rb");
		if (!probe) {
			count = 1;
			hub++;
			goto next;
		}
		fclose(probe);
		int i;
		for (i = 0; i < 9; i++) {
			if (!i)
				sprintf(sysfs, "/sys/bus/usb/devices/%d-%d/idProduct", count, hub);
			else
				sprintf(sysfs, "/sys/bus/usb/devices/%d-%d.%d/idProduct", count, hub, i);
			FILE *modem = fopen(sysfs, "rb");
			if (!modem) {
				goto next_if;
			}
			int idProduct;
			int idVendor;
			fscanf(modem, "%X", &idProduct);
			fclose(modem);
			if (!i)
				sprintf(sysfs, "/sys/bus/usb/devices/%d-%d/idVendor", count, hub);
			else
				sprintf(sysfs, "/sys/bus/usb/devices/%d-%d.%d/idVendor", count, hub, i);
			modem = fopen(sysfs, "rb");
			if (!modem) {
				goto next_if;
			}
			fscanf(modem, "%X", &idVendor);
			fclose(modem);
			if (idVendor == Vendor && idProduct == Product)
				return 1;
		      next_if:;
		}
		count++;
	      next:;
	}
	return 0;
#endif
}

void checkreset(unsigned char tty)
{
	char tts[32];
	sprintf(tts, "/dev/usb/tts/%d", tty);
	eval("comgt", "-d", tts, "-s", "/etc/comgt/reset.comgt");
	FILE *check = NULL;
	int count = 0;
	sleep(1);
	while (!(check = fopen(tts, "rb")) && count < 10) {
		sleep(1);
		count++;
	}
	if (check)
		fclose(check);
	else {
		fprintf(stderr, "reset error\n");
#ifdef HAVE_UNIWIP
		rmmod("fsl-mph-dr-of");
		sleep(1);
		insmod("fsl-mph-dr-of");
		sleep(5);
#endif
	}
	fprintf(stderr, "wakeup card\n");
	eval("comgt", "-d", tts, "-s", "/etc/comgt/wakeup.comgt");
#ifdef HAVE_UNIWIP
	sleep(10);		//give extra delay for registering
#else
	sleep(5);		//give extra delay for registering
#endif
}

static void reset_mc(int needreset, int devicecount)
{
	if (needreset)
		checkreset(devicelist[devicecount].controldevice);
}

static void modeswitch_rezero(int needreset, int devicecount)
{
	FILE *out;
	out = fopen("/tmp/usb_modeswitch.conf", "wb");
	fprintf(out, "DefaultVendor=0x%04x\n", devicelist[devicecount].vendor);
	fprintf(out, "DefaultProduct=0x%04x\n", devicelist[devicecount].product);
	fprintf(out, "OptionMode=1\n");
	fclose(out);
	eval("usb_modeswitch", "-c", "/tmp/usb_modeswitch.conf");

	sleep(3);
}

static void select_config1(int needreset, int devicecount)
{
	FILE *out;
	out = fopen("/tmp/usb_modeswitch.conf", "wb");
	fprintf(out, "DefaultVendor=0x%04x\n", devicelist[devicecount].vendor);
	fprintf(out, "DefaultProduct=0x%04x\n", devicelist[devicecount].product);
	fprintf(out, "Configuration=1\n");
	fclose(out);
	eval("usb_modeswitch", "-c", "/tmp/usb_modeswitch.conf");
}

static void select_config2(int needreset, int devicecount)
{
	FILE *out;
	out = fopen("/tmp/usb_modeswitch.conf", "wb");
	fprintf(out, "DefaultVendor=0x%04x\n", devicelist[devicecount].vendor);
	fprintf(out, "DefaultProduct=0x%04x\n", devicelist[devicecount].product);
	fprintf(out, "Configuration=2\n");
	fclose(out);
	eval("usb_modeswitch", "-c", "/tmp/usb_modeswitch.conf");
}

static void select_config3(int needreset, int devicecount)
{
	FILE *out;
	out = fopen("/tmp/usb_modeswitch.conf", "wb");
	fprintf(out, "DefaultVendor=0x%04x\n", devicelist[devicecount].vendor);
	fprintf(out, "DefaultProduct=0x%04x\n", devicelist[devicecount].product);
	fprintf(out, "Configuration=3\n");
	fclose(out);
	eval("usb_modeswitch", "-c", "/tmp/usb_modeswitch.conf");
}

static void modeswitch_std_eject(int needreset, int devicecount)
{
	FILE *out;
	out = fopen("/tmp/usb_modeswitch.conf", "wb");
	fprintf(out, "DefaultVendor=0x%04x\n", devicelist[devicecount].vendor);
	fprintf(out, "DefaultProduct=0x%04x\n", devicelist[devicecount].product);
	fprintf(out, "StandardEject=1\n");
	fclose(out);
	eval("usb_modeswitch", "-c", "/tmp/usb_modeswitch.conf");
}

static void modeswitch_quanta(int needreset, int devicecount)
{
	FILE *out;
	out = fopen("/tmp/usb_modeswitch.conf", "wb");
	fprintf(out, "DefaultVendor=0x%04x\n", devicelist[devicecount].vendor);
	fprintf(out, "DefaultProduct=0x%04x\n", devicelist[devicecount].product);
	fprintf(out, "QuantaMode=1\n");
	fclose(out);
	eval("usb_modeswitch", "-c", "/tmp/usb_modeswitch.conf");
}

static void modeswitch_curitel(int needreset, int devicecount)
{
	eval("usb_modeswitch", "-v", "0x106c", "-p", "0x3b03", "-M", "555342431234567824000000800008ff024445564348470000000000000000");
	eval("usb_modeswitch", "-v", "0x106c", "-p", "0x3b05", "-M", "555342431234567824000000800008ff020000000000000000000000000000");
	eval("usb_modeswitch", "-v", "0x106c", "-p", "0x3b06", "-M", "555342431234567824000000800008ff020000000000000000000000000000");
	eval("usb_modeswitch", "-v", "0x106c", "-p", "0x3b11", "-M", "555342431234567824000000800008ff024445564348470000000000000000");
	eval("usb_modeswitch", "-v", "0x106c", "-p", "0x3b14", "-M", "555342431234567824000000800008ff024445564348470000000000000000");
}

static void modeswitch_sierra(int needreset, int devicecount)
{
	FILE *out;
	out = fopen("/tmp/usb_modeswitch.conf", "wb");
	fprintf(out, "DefaultVendor=0x%04x\n", devicelist[devicecount].vendor);
	fprintf(out, "DefaultProduct=0x%04x\n", devicelist[devicecount].product);
	fprintf(out, "SierraMode=1\n");
	fclose(out);
	eval("usb_modeswitch", "-c", "/tmp/usb_modeswitch.conf");
	sleep(5);		// give Sierra additional time
}

static void modeswitch_huawei_old(int needreset, int devicecount)
{
	FILE *out;
	out = fopen("/tmp/usb_modeswitch.conf", "wb");
	fprintf(out, "DefaultVendor=0x%04x\n", devicelist[devicecount].vendor);
	fprintf(out, "DefaultProduct=0x%04x\n", devicelist[devicecount].product);
	fprintf(out, "HuaweiMode=1\n");
	fclose(out);
	eval("usb_modeswitch", "-c", "/tmp/usb_modeswitch.conf");
}

static void modeswitch_huawei_std(int needreset, int devicecount)
{
	FILE *out;
	out = fopen("/tmp/usb_modeswitch.conf", "wb");
	fprintf(out, "DefaultVendor=0x%04x\n", devicelist[devicecount].vendor);
	fprintf(out, "DefaultProduct=0x%04x\n", devicelist[devicecount].product);
	fprintf(out, "HuaweiNewMode=1\n");
	fclose(out);
	eval("usb_modeswitch", "-c", "/tmp/usb_modeswitch.conf");
}

static void modeswitch_devchg_fe(int needreset, int devicecount)
{
	FILE *out;
	out = fopen("/tmp/usb_modeswitch.conf", "wb");
	fprintf(out, "DefaultVendor=0x%04x\n", devicelist[devicecount].vendor);
	fprintf(out, "DefaultProduct=0x%04x\n", devicelist[devicecount].product);
	fprintf(out, "MessageContent=\"555342431234567824000000800008fe524445564348470000000000000000\"\n");
	fclose(out);
	eval("usb_modeswitch", "-c", "/tmp/usb_modeswitch.conf");
}

static void modeswitch_devchg_ff(int needreset, int devicecount)
{
	FILE *out;
	out = fopen("/tmp/usb_modeswitch.conf", "wb");
	fprintf(out, "DefaultVendor=0x%04x\n", devicelist[devicecount].vendor);
	fprintf(out, "DefaultProduct=0x%04x\n", devicelist[devicecount].product);
	fprintf(out, "MessageContent=\"555342431234567824000000800008ff524445564348470000000000000000\"\n");
	fclose(out);
	eval("usb_modeswitch", "-c", "/tmp/usb_modeswitch.conf");
}

static void modeswitch_devchg1(int needreset, int devicecount)
{
	FILE *out;
	out = fopen("/tmp/usb_modeswitch.conf", "wb");
	fprintf(out, "DefaultVendor=0x%04x\n", devicelist[devicecount].vendor);
	fprintf(out, "DefaultProduct=0x%04x\n", devicelist[devicecount].product);
	fprintf(out, "MessageContent=\"555342431234567824000000800009ff524445564348473100000000000000\"\n");
	fclose(out);
	eval("usb_modeswitch", "-c", "/tmp/usb_modeswitch.conf");
}

static void modeswitch_zte(int needreset, int devicecount)
{
	FILE *out;
	out = fopen("/tmp/usb_modeswitch.conf", "wb");
	fprintf(out, "DefaultVendor=0x%04x\n", devicelist[devicecount].vendor);
	fprintf(out, "DefaultProduct=0x%04x\n", devicelist[devicecount].product);
	fprintf(out, "StandardEject=1\n");
	fprintf(out, "MessageContent=\"55534243123456702000000080000c85010101180101010101000000000000\"\n");
	fclose(out);
	eval("usb_modeswitch", "-c", "/tmp/usb_modeswitch.conf");
}

static void modeswitch_alcatel(int needreset, int devicecount)
{
	FILE *out;
	out = fopen("/tmp/usb_modeswitch.conf", "wb");
	fprintf(out, "DefaultVendor=0x%04x\n", devicelist[devicecount].vendor);
	fprintf(out, "DefaultProduct=0x%04x\n", devicelist[devicecount].product);
	fprintf(out, "MessageContent=\"55534243123456788000000080000606f50402527000000000000000000000\"\n");
	fprintf(out, "MessageContent2=\"55534243123456788000000080010606f50402527000000000000000000000\"\n");
	fprintf(out, "CheckSuccess=20\n");
	fclose(out);
	eval("usb_modeswitch", "-c", "/tmp/usb_modeswitch.conf");
}

static void modeswitch_onda(int needreset, int devicecount)
{
	FILE *out;
	out = fopen("/tmp/usb_modeswitch.conf", "wb");
	fprintf(out, "DefaultVendor=0x%04x\n", devicelist[devicecount].vendor);
	fprintf(out, "DefaultProduct=0x%04x\n", devicelist[devicecount].product);
	fprintf(out, "MessageContent=\"555342431234567800000000000010ff000000000000000000000000000000\"\n");
	fprintf(out, "NeedResponse=1\n");
	fclose(out);
	eval("usb_modeswitch", "-c", "/tmp/usb_modeswitch.conf");
}

static void modeswitch_mediatek(int needreset, int devicecount)
{
	FILE *out;
	out = fopen("/tmp/usb_modeswitch.conf", "wb");
	fprintf(out, "DefaultVendor=0x%04x\n", devicelist[devicecount].vendor);
	fprintf(out, "DefaultProduct=0x%04x\n", devicelist[devicecount].product);
	fprintf(out, "MessageContent=\"555342431234567800000000000003f0010100000000000000000000000000\"\n");
	fprintf(out, "NeedResponse=1\n");
	fclose(out);
	eval("usb_modeswitch", "-c", "/tmp/usb_modeswitch.conf");
}

static void modeswitch_others(int needreset, int devicecount)
{
	eval("usb_modeswitch", "-v", "0x04fc", "-p", "0x2140", "-M", "55534243123456782400000080000612000024000000000000000000000000");
	eval("usb_modeswitch", "-v", "0x07d1", "-p", "0xf000", "-M", "555342431234567800000000000006bd000000020000000000000000000000");
	eval("usb_modeswitch", "-v", "0x10a9", "-p", "0x606f", "-F 4");
	eval("usb_modeswitch", "-v", "0x10a9", "-p", "0x6080", "-F 2");
	eval("usb_modeswitch", "-v", "0x19d2", "-p", "0x0166", "-M", "55534243123456782400000080000685000000240000000000000000000000");
	eval("usb_modeswitch", "-v", "0x19d2", "-p", "0x0266", "-M", "55534243123456782400000080000685000000240000000000000000000000");
	eval("usb_modeswitch", "-v", "0x19d2", "-p", "0x0388", "-M", "55534243123456782400000080000685000000240000000000000000000000");
	eval("usb_modeswitch", "-v", "0x19d2", "-p", "0x0413", "-M", "55534243123456782400000080000685000000240000000000000000000000");
	eval("usb_modeswitch", "-v", "0x19d2", "-p", "0xfff5", "-M", "5553424312345678c00000008000069f010000000000000000000000000000");
	eval("usb_modeswitch", "-v", "0x19d2", "-p", "0xfff6", "-M", "5553424312345678c00000008000069f010000000000000000000000000000");
	eval("usb_modeswitch", "-v", "0x1e0e", "-p", "0xf000", "-M", "555342431234567800000000000006bd000000020000000000000000000000");
}

#define QMIRAW 0xa0		// usbnet, qmi_wwan, cdc_wdm, rawip
#define H_NCM 0x90		// usbnet, cdc_ncm, huawei_cdc_ncm, cdc_wdm
#define NCM 0x80		// usbnet, cdc_ncm, cdc_wdm
#define MBIM 0x70		// usbnet, cdc_ncm, cdc_mbim,
#define RNDIS 0x60		// usbnet, cdc_ether, rndis_host,
#define S_NET 0x50		// usbnet, sierra_net (direct ip)
#define QMI 0x40		// usbnet, qmi_wwan, cdc_wdm
#define ETH 0x30		// usbnet, cdc_ether
#define GENERIC 0x20		// option new_id on-the-fly
#define ACM 0x10		// cdc_acm

// 0-15 is variant type
// 0 = No command
// 1 = Sierra cmdset
// 2 = Huawei cmdset
// 3 = Option cmdset
// 4 = ZTE cmdset
// 5 = 
// 6 = 
// 7 = 
// 8 = 
// 9 = 
// 10 = 
// 11 = 
// 12 = ETH static ip
// 13 = ETH dynamic ip
// 14 = RNDIS UP/DOWN via tty
// 15 = RNDIS UP/DOWN via cdc_wdm

static struct DEVICES devicelist[] = {

/* Quanta */
	{0x0408, 0xea02, option, 2, 0, 0, NULL, "Quanta MUQ-101 (modem)"},	//
	{0x0408, 0xea03, option, 2, 0, 0, NULL, "Quanta MUQ-110 (modem)"},	//
	{0x0408, 0xea04, option, 2, 0, 0, NULL, "Quanta GLX (modem)"},	//
	{0x0408, 0xea05, option, 2, 0, 0, NULL, "Quanta GKE (modem)"},	//
	{0x0408, 0xea06, option, 2, 0, 0, NULL, "Quanta GLE (modem)"},	//
	{0x0408, 0xea16, generic, 1, 2, 0, NULL, "Quanta 1KR (modem)"},	// also qmi but not yet in driver
	{0x0408, 0xea17, option, 0, 0, 2, &modeswitch_std_eject, "Quanta 1KR (cdrom)"},	//
	{0x0408, 0xea25, option, 0, 0, 2, &modeswitch_quanta, "Quanta 1K3 LTE (cdrom)"},	//
	{0x0408, 0xea26, generic, 1, 2, 2, NULL, "Quanta Mobility LTE (modem)"},	// also qmi but not yet in driver
	{0x0408, 0xea42, option, 3, 2, 0 | QMI, NULL, "Megafone M100-1 (modem)"},	//
	{0x0408, 0xea43, option, 0, 0, 2, &modeswitch_std_eject, "Quanta MobileGenie (cdrom)"},	//
//      {0x0408, 0xea45, none, 0, 0, 0 | ETH, NULL, "Quanta 1K6E (modem)"},       //
	{0x0408, 0xea47, generic, 3, 2, 2, NULL, "Quanta MobileGenie (modem)"},	// also qmi but not yet in driver
	{0x0408, 0xea49, generic, 0, 1, 2, NULL, "Telsec TS-1K6 (modem)"},	// also qmi but not yet in driver
	{0x0408, 0xf000, option, 0, 0, 2, &modeswitch_quanta, "Quanta 1QDL (cdrom)"},	//
	{0x0408, 0xf001, option, 0, 0, 2, &modeswitch_quanta, "Quanta GLX, GLE,GKE (cdrom)"},	//

/* Nokia Mobile Phones */
	{0x0421, 0x03a7, acm, 0, 0, 2, NULL, "Nokia C5-00 Mobile phone (modem)"},	//
	{0x0421, 0x060c, none, 0, 0, 2, &modeswitch_std_eject, "Nokia CS-10 (cdrom)"},	//
	{0x0421, 0x060d, acm, 0, 0, 2, NULL, "Nokia CS-10 (modem)"},	//
	{0x0421, 0x060e, acm, 0, 0, 2, NULL, "Nokia CS-10 (modem)"},	//
	{0x0421, 0x0610, none, 0, 0, 2, &modeswitch_std_eject, "Nokia CS-15 (cdrom)"},	//
	{0x0421, 0x0612, acm, 0, 0, 2, NULL, "Nokia CS-15/CS-18 (modem)"},	//
	{0x0421, 0x0618, none, 0, 0, 2, &modeswitch_std_eject, "Nokia CS-12 (cdrom)"},	//
	{0x0421, 0x0619, acm, 0, 0, 2, NULL, "Nokia CS-12 (modem)"},	//
	{0x0421, 0x061d, none, 0, 0, 2, &modeswitch_std_eject, "Nokia CS-11 (cdrom)"},	//
	{0x0421, 0x061e, acm, 0, 0, 2, NULL, "Nokia CS-11 (modem)"},	//
	{0x0421, 0x0622, none, 0, 0, 2, &modeswitch_std_eject, "Nokia CS-17 (cdrom)"},	//
	{0x0421, 0x0623, acm, 0, 0, 2, NULL, "Nokia CS-17 (modem)"},	//
	{0x0421, 0x0627, none, 0, 0, 2, &modeswitch_std_eject, "Nokia CS-18 (cdrom)"},	//
	{0x0421, 0x0629, acm, 0, 0, 2, NULL, "Nokia CS-18 (modem)"},	//
	{0x0421, 0x062c, none, 0, 0, 2, &modeswitch_std_eject, "Nokia CS-19 (cdrom)"},	//
	{0x0421, 0x062d, acm, 0, 0, 2, NULL, "Nokia CS-19 (modem)"},	//
	{0x0421, 0x062f, acm, 0, 0, 2, NULL, "Nokia CS-19 (modem)"},	//
	{0x0421, 0x0632, acm, 0, 0, 2, &modeswitch_std_eject, "Nokia 7M-01"},	//
	{0x0421, 0x0637, none, 0, 0, 2, &modeswitch_std_eject, "Nokia 21M-02 (cdrom)"},	//
	{0x0421, 0x0638, acm, 0, 0, 2, NULL, "Nokia 21M-02 (modem)"},	//
	{0x0421, 0x0639, acm, 1, 0, 2, NULL, "Nokia 21M-02 (modem)"},	//

/* Philips/NXP */
	{0x0471, 0x1206, acm, 0, 0, 2, &select_config2, "Philips TalkTalk (modem)"},	//
	{0x0471, 0x1210, none, 0, 0, 2, &modeswitch_std_eject, "Vodaphone MD950 (cdrom)"},	//
	{0x0471, 0x1237, acm, 0, 0, 2, &select_config2, "Philips TalkTalk (modem)"},	//

/* Kyocera */
	{0x0482, 0x024d, acm, 0, 0, 2, &select_config2, "Kyocera WK06"},	//

/* ST-Ericsson */
	{0x04cc, 0x2251, none, 0, 0, 2, &modeswitch_std_eject, "Alcatel-Lucent"},	//
	{0x04cc, 0x2259, acm, 0, 0, 2, &select_config2, "Alcatel-Lucent T920S"},	//
	{0x04cc, 0x225c, acm, 0, 0, 2, &select_config2, "Alcatel-Lucent T930S"},	//
	{0x04cc, 0x226e, acm, 0, 0, 2, &select_config2, "Nexperia TM TD-SCDMA"},	//

/* Sunplus */
	{0x04fc, 0x0615, acm, 0, 0, 2, NULL, "SU-3200U (modem)"},	//
	{0x04fc, 0x1240, acm, 0, 0, 2, NULL, "Mobidata MDB-100EU (modem)"},	//
	{0x04fc, 0x2140, none, 0, 0, 2, &modeswitch_others, "SU-3200U (cdrom)"},	//

/* Qualcomm */
	{0x05c6, 0x0010, none, 0, 0, 0, &modeswitch_std_eject, "Generic Qualcomm (cdrom)"},	//
	{0x05c6, 0x0016, generic, 2, 2, 0, NULL, "iBall 3.5G Connect (modem)"},	//
	{0x05c6, 0x0018, generic, 1, 3, 0, NULL, "Advan DT9 SQ (modem)"},	//
	{0x05c6, 0x0023, option, 2, 0, 0, NULL, "Leoxsys LN-72V (modem)"},	//
	{0x05c6, 0x00a0, generic, 2, 0, 0, NULL, "Axesstel MV241 (modem)"},	//
	{0x05c6, 0x1000, none, 0, 0, 0, &modeswitch_std_eject, "Generic Qualcomm (cdrom)"},	//
	{0x05c6, 0x2001, none, 0, 0, 0, &modeswitch_std_eject, "Generic Qualcomm (cdrom)"},	//
	{0x05c6, 0x3100, acm, 0, 0, 0, NULL, "Maxon MM-5100 (modem)"},	//
	{0x05c6, 0x3196, acm, 0, 0, 0, NULL, "Maxon MM-5500 (modem)"},	//
	{0x05c6, 0x3197, generic, 0, 0, 0, NULL, "SpeedUp SU-6500U/SU-6600U (modem)"},	//
	{0x05c6, 0x6000, generic, 2, 0, 0, NULL, "Siemens SG75 (modem)"},	//
	{0x05c6, 0x6280, generic, 2, 0, 0, NULL, "Qualcomm generic (modem)"},	//
	{0x05c6, 0x6500, generic, 2, 0, 0, NULL, "Venus VT-80n (modem)"},	//
	{0x05c6, 0x6503, none, 0, 0, 0, &modeswitch_std_eject, "Generic Qualcomm (cdrom)"},	//
	{0x05c6, 0x9000, qcserial, 1, 2, 0 | QMI, NULL, "SIMCom SIM5218 (modem)"},	//
	{0x05c6, 0x9003, qcserial, 2, 3, 0 | QMI, NULL, "Quectel UC20 (modem)"},	//
	{0x05c6, 0x9011, qcserial, 1, 2, 0 | QMI, NULL, "Qualcomm HS-USB (modem)"},	//
	{0x05c6, 0x9024, none, 0, 0, 0, &modeswitch_std_eject, "ASB TL131 TD-LTE (cdrom)"},	//
	{0x05c6, 0x9025, generic, 2, 0, 0 | QMI, NULL, "ASB TL131 TD-LTE (modem)"},	//
	{0x05c6, 0x9046, generic, 1, 2, 0 | QMI, NULL, "Qualcomm HS-USB (modem)"},	//
	{0x05c6, 0xf000, none, 0, 0, 0, &modeswitch_std_eject, "Generic Qualcomm (cdrom)"},	//

/* D-Link */
	{0x07d1, 0x3e01, option, 1, 0, 2, NULL, "D-Link DWM-152 C1 (modem)"},	//
	{0x07d1, 0x3e02, option, 1, 0, 2, NULL, "D-Link DWM-156 A1 (modem)"},	//
	{0x07d1, 0x7e07, generic, 3, 3, 2, NULL, "D-Link DWM-151 A1 (modem)"},	//
	{0x07d1, 0x7e0c, generic, 2, 0, 2, NULL, "D-Link DWM-156 A2 (modem)"},	//
	{0x07d1, 0x7e11, option, 1, 2, 2, NULL, "D-Link DWM-156 A3 (modem)"},	//
	{0x07d1, 0xa800, none, 0, 0, 0, &modeswitch_std_eject, "D-Link DWM-152 C1/DWM-156 A1 (cdrom)"},	//
	{0x07d1, 0xa804, none, 0, 0, 0, &modeswitch_std_eject, "D-Link DWM-156 A3 (cdrom)"},	//
	{0x07d1, 0xf000, none, 0, 0, 0, &modeswitch_others, "D-Link DWM-151 A1 (cdrom)"},	//

/* Netgear */
	{0x0846, 0x0fff, none, 0, 0, 1, &modeswitch_sierra, "Sierra Device (cdrom)"},	//
	{0x0846, 0x68a2, sierra, 2, 0, 1 | QMI, NULL, "Sierra MC7710 (modem)"},	//
//      {0x0846, 0x68d3, generic, 0, 0, 1 | QMI, &select_config2, "Netgear AC778S (modem)"}, // not yet in driver
//      {0x0846, 0x68e1, none, 0, 0, 1 | ETH, &select_config2, "Netgear AC785S (modem)"}, // rndis in default config1
//      {0x0846, 0x68e2, none, 0, 0, 1 | ETH, &select_config2, "Netgear AC78xS (modem)"}, // rndis in default config1

/* Toshiba */
	{0x0930, 0x0d45, option, 2, 0, 2, NULL, "Toshiba G450 (modem)"},	//
	{0x0930, 0x0d46, none, 0, 0, 0, &modeswitch_std_eject, "Toshiba G450 (cdrom)"},	//

/* Option */
	{0x0af0, 0x4005, option, 2, 1, 0 | QMI, NULL, "Option GIO711"},	//
	{0x0af0, 0x4007, none, 0, 0, 0, &modeswitch_sierra, "Option GIO711 (cdrom)"},	//
//      {0x0af0, 0x6711, none, HSO, HSO, HSO, &modeswitch_rezero, "Option GE201"},    //express card
//      {0x0af0, 0x6731, none, HSO, HSO, HSO, &modeswitch_rezero, "Option GE"},       //express card
//      {0x0af0, 0x6751, none, HSO, HSO, HSO, &modeswitch_rezero, "Option GE"},       //express card
//      {0x0af0, 0x6771, none, HSO, HSO, HSO, &modeswitch_rezero, "Option GE"},       //express card
//      {0x0af0, 0x6791, none, HSO, HSO, HSO, &modeswitch_rezero, "Option GE"},       //express card
	{0x0af0, 0x6901, option, 1, 0, 0, NULL, "Option GI0201"},	//usb
	{0x0af0, 0x6911, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GI0225"},	//usb
	{0x0af0, 0x6951, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GI0225"},	//usb
	{0x0af0, 0x6971, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GI0225"},	//usb
//      {0x0af0, 0x7011, hso, HSO, HSO, 2, &modeswitch_rezero, "Option GE301"},    //express card
//      {0x0af0, 0x7031, hso, HSO, HSO, 2, &modeswitch_rezero, "Option GE301"},    //express card
//      {0x0af0, 0x7051, hso, HSO, HSO, 2, &modeswitch_rezero, "Option GE301"},    //express card
//      {0x0af0, 0x7071, hso, HSO, HSO, 2, &modeswitch_rezero, "Option GE301"},    //express card
	{0x0af0, 0x7111, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GTM"},	//pcie minicard
	{0x0af0, 0x7201, option, 1, 0, 0, NULL, "Option GTM380"},	//pcie minicard
	{0x0af0, 0x7211, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GTM380"},	//pcie minicard
	{0x0af0, 0x7251, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GTM380"},	//pcie minicard
	{0x0af0, 0x7271, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GTM380"},	//pcie minicard
//      {0x0af0, 0x7301, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GE040x"},   //express card
	{0x0af0, 0x7311, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GTM040x"},	//pcie minicard
//      {0x0af0, 0x7361, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GE044x"},   //express card
//      {0x0af0, 0x7381, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GE044x"},   //express card
	{0x0af0, 0x7401, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GI0401"},	//usb
	{0x0af0, 0x7501, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GI0431"},	//usb
//      {0x0af0, 0x7601, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GE040x"},   //express card
	{0x0af0, 0x7701, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GI0451"},	//usb
	{0x0af0, 0x7706, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GI0451"},	//usb
	{0x0af0, 0x7801, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GI04xx"},	//usb
	{0x0af0, 0x7901, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GI0452"},	//usb
	{0x0af0, 0x7a01, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GI0461"},	//usb
	{0x0af0, 0x7a05, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GI0461"},	//usb
	{0x0af0, 0x8001, hso, 0, 0, 0, &modeswitch_rezero, "Option GI1515"},	//zero footprint install id
	{0x0af0, 0x8002, hso, 0, 0, 0, &modeswitch_rezero, "Option GI1515"},	//zero footprint install id
	{0x0af0, 0x8003, hso, 0, 0, 0, &modeswitch_rezero, "Option GI1515"},	//zero footprint install id
	{0x0af0, 0x8120, qcserial, 3, 2, 3 | QMI, NULL, "Option GTM681W"},	//pcie minicard
	{0x0af0, 0x8121, generic, 3, 2, 3 | QMI, NULL, "Option GTM689W"},	//pcie minicard
	{0x0af0, 0x8200, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GI07xx"},	//usb
	{0x0af0, 0x8201, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GI07xx"},	//usb
	{0x0af0, 0x8204, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GI07xx"},	//usb
	{0x0af0, 0x8300, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GI033x"},	//usb
	{0x0af0, 0x8302, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GI033x"},	//usb
	{0x0af0, 0x8304, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GI033x"},	//usb
//      {0x0af0, 0x8400, hso, HSO, HSO, HSO, &modeswitch_rezero, "Pioner JP1"},      //
	{0x0af0, 0x8600, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GI063x"},	//usb
	{0x0af0, 0x8700, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GI0643"},	//usb
//      {0x0af0, 0x8701, hso, 0, 0, 3 | ETH, NULL, "Option GI0643"},     //usb
	{0x0af0, 0x8800, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GTM60x"},	//pcie minicard
	{0x0af0, 0x8900, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GTM67x"},	//pcie minicard
	{0x0af0, 0x9000, hso, HSO, HSO, 3, &modeswitch_rezero, "Option GTM66x"},	//pcie minicard
	{0x0af0, 0x9200, hso, HSO, HSO, HSO, NULL, "Option GTM671WFS"},	//pcie minicard
	{0x0af0, 0xc031, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GI031"},	//usb
	{0x0af0, 0xc100, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GI070x"},	//usb
	{0x0af0, 0xd001, hso, 0, 0, 0, &modeswitch_rezero, "Option GI1515"},	//zero footprint install id
	{0x0af0, 0xd031, hso, HSO, HSO, HSO, &modeswitch_rezero, "Qualcomm ICON 321"},	//usb
	{0x0af0, 0xd033, hso, HSO, HSO, HSO, &modeswitch_rezero, "Qualcomm ICON 322"},	//usb
	{0x0af0, 0xd055, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GI0505"},	//usb
	{0x0af0, 0xd057, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GI1505"},	//usb
	{0x0af0, 0xd058, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GI1509"},	//usb
	{0x0af0, 0xd155, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GI0505"},	//usb
	{0x0af0, 0xd157, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GI1515"},	//usb
	{0x0af0, 0xd255, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GI1215"},	//usb
	{0x0af0, 0xd257, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GI1215"},	//usb
	{0x0af0, 0xd357, hso, HSO, HSO, HSO, &modeswitch_rezero, "Option GI1505"},	//usb

/* Olivetti */
	{0x0b3c, 0xc000, option, 0, 0, 2 | QMI, NULL, "Olivetti Olicard 100"},	//
	{0x0b3c, 0xc001, option, 0, 0, 2 | QMI, NULL, "Olivetti Olicard 120"},	//
	{0x0b3c, 0xc002, option, 0, 0, 2 | QMI, NULL, "Olivetti Olicard 140"},	//
	{0x0b3c, 0xc003, option, 0, 4, 2, NULL, "Olivetti Olicard 145"},	//
	{0x0b3c, 0xc004, option, 0, 4, 2 | QMI, NULL, "Olivetti Olicard 155"},	//
	{0x0b3c, 0xc005, option, 0, 0, 2 | QMI, NULL, "Olivetti Olicard 200"},	//
	{0x0b3c, 0xc00a, option, 0, 4, 2 | QMI, NULL, "Olivetti Olicard 160"},	//
	{0x0b3c, 0xc00b, option, 0, 2, 2 | QMI, NULL, "Olivetti Olicard 500"},	//
	{0x0b3c, 0xc700, none, 0, 0, 0, &modeswitch_std_eject, "Olivetti Olicard 100 (cdrom)"},	//
	{0x0b3c, 0xf000, none, 0, 0, 0, &modeswitch_alcatel, "Olivetti Olicards (cdrom)"},	//
	{0x0b3c, 0xf00c, none, 0, 0, 0, &modeswitch_alcatel, "Olivetti Olicards (cdrom)"},	//
	{0x0b3c, 0xf017, none, 0, 0, 0, &modeswitch_std_eject, "Olivetti Olicard 500 (cdrom)"},	//

/* Ericsson Business Mobile Networks */
	{0x0bdb, 0x1900, acm, 0, 1, 0, NULL, "Ericsson F3507g"},	//
	{0x0bdb, 0x1902, acm, 0, 1, 0, NULL, "Lenovo F3507g"},	//
	{0x0bdb, 0x1904, acm, 0, 1, 0, NULL, "Ericsson F3607gw"},	//
	{0x0bdb, 0x1905, acm, 0, 1, 0, NULL, "Lenovo F3607gw"},	//
	{0x0bdb, 0x1906, acm, 0, 1, 0, NULL, "Ericsson F3607gw"},	//
	{0x0bdb, 0x1907, acm, 0, 1, 0, NULL, "Lenovo F3607gw"},	//
	{0x0bdb, 0x1909, acm, 0, 1, 0, NULL, "Ericsson F3307"},	//
	{0x0bdb, 0x190a, acm, 0, 1, 0, NULL, "Ericsson F3307"},	//
	{0x0bdb, 0x190d, acm, 0, 1, 0, NULL, "Ericsson F5521gw"},	// also cdc_ncm
	{0x0bdb, 0x190e, acm, 0, 1, 0, NULL, "Lenovo F3307"},	//
	{0x0bdb, 0x190f, acm, 0, 1, 0, NULL, "Lenovo F3307"},	//
	{0x0bdb, 0x1910, acm, 0, 1, 0, NULL, "Lenovo F5521gw"},	// also cdc_ncm
	{0x0bdb, 0x1911, acm, 0, 1, 0, NULL, "Lenovo F5521gw"},	// also cdc_ncm

/* Kyocera */
	{0x0c88, 0x17da, option, 0, 0, 0, NULL, "Kyocera KPC650"},	//
	{0x0c88, 0x180a, option, 0, 0, 0, NULL, "Kyocera KPC680"},	//

/* Mediatek */
	{0x0e8d, 0x0002, none, 0, 0, 0, &modeswitch_mediatek, "MT6276"},	//
	{0x0e8d, 0x00a1, generic, 1, 0, 2, NULL, "MT6276"},	//
	{0x0e8d, 0x00a2, generic, 1, 0, 2, NULL, "MT6276"},	//
	{0x0e8d, 0x00a4, option, 1, 0, 2, NULL, "MT6276"},	//
	{0x0e8d, 0x00a5, option, 1, 0, 2, NULL, "Thinkwill UE980"},	// also cdc_mbim
	{0x0e8d, 0x00a7, option, 1, 0, 2, NULL, "Mediatek DC_4COM2"},	// also cdc_mbim

/* AirPrime (Sierra) */
	{0x0f3d, 0x68a2, qcserial, 2, 0, 1 | QMI, NULL, "Sierra MC7700 (modem)"},	// also cdc_mbim
	{0x0f3d, 0x68a3, sierra, 3, 3, 1, &reset_mc, "Sierra Wireless MC8700/Compass Direct IP"},	//
	{0x0f3d, 0x68aa, sierra, 3, 3, 1, NULL, "Sierra Wireless AC313U/320U/330U Direct IP"},	//

/* Sony Ericsson */
	{0x0fce, 0xd0cf, acm, 0, 1, 0, &select_config3, "Sony Ericsson MD300"},	//
	{0x0fce, 0xd0df, acm, 0, 1, 0, &select_config2, "Sony Ericsson EC400"},	//
	{0x0fce, 0xd0e1, acm, 0, 1, 0, &select_config2, "Sony Ericsson MD400"},	//
	{0x0fce, 0xd0ee, acm, 0, 1, 0, NULL, "Sony Ericsson EC400G"},	//
	{0x0fce, 0xd103, acm, 0, 1, 0, NULL, "Sony Ericsson MD400G"},	//

/* LG Electronics */
	{0x1004, 0x6107, acm, 0, 0, 0, NULL, "LG-LDU1900D"},	//
	{0x1004, 0x6109, acm, 0, 0, 0, NULL, "LG-L02A"},	//
	{0x1004, 0x610c, none, 0, 0, 0, &modeswitch_std_eject, "LG L-02A"},	//
	{0x1004, 0x6124, acm, 0, 0, 0, NULL, "LG L-05A"},	//
//      {0x1004, 0x6135, option, 0, 2, 0, NULL, "LG L-07A"},      //
	{0x1004, 0x613a, none, 0, 0, 0, &modeswitch_std_eject, "LG L-05A"},	//
	{0x1004, 0x613f, none, 0, 0, 0, &modeswitch_std_eject, "LG LUU-2100TI"},	//
	{0x1004, 0x6141, acm, 0, 0, 0, NULL, "LG LUU-2100TI"},	//
	{0x1004, 0x614e, none, 0, 0, 0, &modeswitch_std_eject, "LG L-07A"},	//
	{0x1004, 0x6156, none, 0, 0, 0, &modeswitch_std_eject, "LG LUU-2110TI"},	//
	{0x1004, 0x6157, acm, 0, 2, 0, NULL, "LG LUU-2110TI"},	//
	{0x1004, 0x618f, option, 0, 2, 0, NULL, "LG L-02C"},	//
	{0x1004, 0x6190, none, 0, 0, 0, &modeswitch_std_eject, "LG AD600"},	//
//      {0x1004, 0x61a7, option, 0, 2, 0, NULL, "LG AD600"},      // also cdc_ether
	{0x1004, 0x61dd, none, 0, 0, 0, &modeswitch_std_eject, "LG L-02C"},	//
//      {0x1004, 0x61e6, option, 0, 2, 0, NULL, "LG SD711"},      //
	{0x1004, 0x61e7, none, 0, 0, 0, &modeswitch_std_eject, "LG SD711"},	//
//      {0x1004, 0x61ea, option, 0, 2, 0, NULL, "LG L-08C"},      //
	{0x1004, 0x61eb, none, 0, 0, 0, &modeswitch_std_eject, "LG L-08C"},	//
//      {0x1004, 0x6326, option, 0, 2, 0, NULL, "LG L-03D"},      //
//      {0x1004, 0x6327, none, 0, 0, 0, &modeswitch_std_eject, "LG L-03D"},     //

/* Nucam */
	{0x1033, 0x0035, option, 0, 0, 2, &modeswitch_huawei_old, "HUAWEI E630"},	//

/* Curitel */
	{0x106c, 0x3711, acm, 0, 0, 2, NULL, "PANTECH UM-150"},	//
	{0x106c, 0x3712, acm, 0, 0, 2, NULL, "PANTECH UM-175V1"},	//
	{0x106c, 0x3714, acm, 0, 0, 2, NULL, "PANTECH UM-175VW"},	//
	{0x106c, 0x3715, acm, 0, 0, 2, NULL, "PANTECH UM-175AL"},	//
	{0x106c, 0x3716, acm, 0, 0, 2, NULL, "PANTECH UM-190VW"},	//
	{0x106c, 0x3717, acm, 0, 0, 2, NULL, "PANTECH UM-185C/UM185E"},	//
	{0x106c, 0x3718, acm, 0, 0, 2, NULL, "PANTECH UML-290VW 4G Modem"},	// also qmi but fails in dd-wrt
	{0x106c, 0x3721, option, 0, 0, 2 | QMI, NULL, "PANTECH P4200 4G Modem"},	//
	{0x106c, 0x3b03, none, 0, 0, 2, &modeswitch_curitel, "PANTECH UM-175AL"},	//
	{0x106c, 0x3b05, none, 0, 0, 2, &modeswitch_curitel, "PANTECH UM-190"},	//
	{0x106c, 0x3b06, none, 0, 0, 2, &modeswitch_curitel, "PANTECH UM-185C/UM185E"},	//
	{0x106c, 0x3b11, none, 0, 0, 2, &modeswitch_curitel, "PANTECH UML-290"},	//
	{0x106c, 0x3b14, none, 0, 0, 2, &modeswitch_curitel, "PANTECH P4200"},	//

/* SK Teletech */
//      {0x10a9, 0x6064, option, 0, 0, 2 | ETH, NULL, "PANTECH UML-295"}, //
	{0x10a9, 0x606f, none, 0, 0, 2, &modeswitch_others, "PANTECH ULM-295 (cdrom)"},	//
	{0x10a9, 0x6080, none, 0, 0, 2, &modeswitch_others, "PANTECH MHS291LVW (cdrom)"},	//
//      {0x10a9, 0x6085, option, 0, 0, 2 | ETH, NULL, "PANTECH MHS291LVW"},       //

/* Sierra Wireless-Wavecom */
	{0x114f, 0x68a2, qcserial, 2, 0, 1 | QMI, NULL, "Sierra MC7750 (modem)"},	//

/* Sierra Wireless (Netgear) */
	{0x1199, 0x0017, sierra, 3, 4, 1, NULL, "Sierra EM5625 (modem)"},	//
	{0x1199, 0x0018, sierra, 3, 4, 1, NULL, "Sierra MC5720 (modem)"},	//
	{0x1199, 0x0019, sierra, 3, 4, 1, NULL, "Sierra AC595U (modem)"},	//
	{0x1199, 0x0020, sierra, 3, 4, 1, NULL, "Sierra MC5725 (modem)"},	//
	{0x1199, 0x0021, sierra, 3, 4, 1, NULL, "Sierra AC597E (modem)"},	//
	{0x1199, 0x0022, sierra, 3, 4, 1, NULL, "Sierra EM5725 (modem)"},	//
	{0x1199, 0x0023, sierra, 3, 4, 1, NULL, "Sierra C597 (modem)"},	//
	{0x1199, 0x0024, sierra, 3, 4, 1, NULL, "Sierra MC5727 (modem)"},	//
	{0x1199, 0x0025, sierra, 3, 4, 1, NULL, "Sierra AC598 (modem)"},	//
	{0x1199, 0x0026, sierra, 3, 4, 1, NULL, "Sierra T11 (modem)"},	//
	{0x1199, 0x0027, sierra, 3, 4, 1, NULL, "Sierra AC402 (modem)"},	//
	{0x1199, 0x0028, sierra, 3, 4, 1, NULL, "Sierra MC5728 (modem)"},	//
	{0x1199, 0x0112, sierra, 3, 4, 1, NULL, "Sierra AC580 (modem)"},	//
	{0x1199, 0x0120, sierra, 3, 4, 1, NULL, "Sierra AC595U (modem)"},	//
	{0x1199, 0x0218, sierra, 3, 4, 1, NULL, "Sierra MC5720 (modem)"},	//
	{0x1199, 0x0220, sierra, 3, 4, 1, NULL, "Sierra MC5725 (modem)"},	//
	{0x1199, 0x0224, sierra, 3, 4, 1, NULL, "Sierra MC5727 (modem)"},	//
	{0x1199, 0x0301, sierra, 3, 4, 1, NULL, "Sierra AC250U (modem)"},	//
	{0x1199, 0x0fff, none, 0, 0, 1, &modeswitch_sierra, "Sierra Device (cdrom)"},	//
	{0x1199, 0x6802, sierra, 0, 2, 1, NULL, "Sierra MC8755 (modem)"},	//
	{0x1199, 0x6803, sierra, 0, 2, 1, NULL, "Sierra MC8765 (modem)"},	//
	{0x1199, 0x6804, sierra, 0, 2, 1, NULL, "Sierra MC8755 (modem)"},	//
	{0x1199, 0x6805, sierra, 0, 2, 1, NULL, "Sierra MC8765 (modem)"},	//
	{0x1199, 0x6808, sierra, 0, 2, 1, NULL, "Sierra MC8755 (modem)"},	//
	{0x1199, 0x6809, sierra, 0, 2, 1, NULL, "Sierra MC8765 (modem)"},	//
	{0x1199, 0x6812, sierra, 0, 2, 1, &reset_mc, "Sierra MC8775V"},	//
	{0x1199, 0x6813, sierra, 0, 2, 1, NULL, "Sierra MC8775 (modem)"},	//
	{0x1199, 0x6815, sierra, 0, 2, 1, NULL, "Sierra MC8775 (modem)"},	//
	{0x1199, 0x6816, sierra, 0, 2, 1, NULL, "Sierra MC8775 (modem)"},	//
	{0x1199, 0x6820, sierra, 0, 2, 1, NULL, "Sierra AC875 (modem)"},	//
	{0x1199, 0x6821, sierra, 2, 0, 1, NULL, "Sierra AC875U (modem)"},	//
	{0x1199, 0x6822, sierra, 3, 4, 1, NULL, "Sierra AC875E (modem)"},	//
	{0x1199, 0x6832, sierra, 2, 0, 1, &reset_mc, "Sierra MC8780 (modem)"},	//
	{0x1199, 0x6833, sierra, 3, 4, 1, NULL, "Sierra MC8781 (modem)"},	//
	{0x1199, 0x6834, sierra, 3, 4, 1, NULL, "Sierra MC8780 (modem)"},	//
	{0x1199, 0x6835, sierra, 3, 4, 1, NULL, "Sierra MC8781 (modem)"},	//
	{0x1199, 0x6838, sierra, 3, 4, 1, NULL, "Sierra MC8780 (modem)"},	//
	{0x1199, 0x6839, sierra, 3, 4, 1, NULL, "Sierra MC8781 (modem)"},	//
	{0x1199, 0x683a, sierra, 3, 4, 1, NULL, "Sierra MC8785 (modem)"},	//
	{0x1199, 0x683b, sierra, 3, 4, 1, NULL, "Sierra MC8785 Composite (modem)"},	//
	{0x1199, 0x683c, sierra, 3, 3, 1, &reset_mc, "Sierra MC8790 Composite"},	//
	{0x1199, 0x683d, sierra, 3, 3, 1, &reset_mc, "Sierra MC8791 Composite"},	//
	{0x1199, 0x683e, sierra, 3, 3, 1, &reset_mc, "Sierra MC8790"},	//
	{0x1199, 0x6850, sierra, 2, 0, 1, NULL, "Sierra AC880 (modem)"},	//
	{0x1199, 0x6851, sierra, 2, 0, 1, NULL, "Sierra AC 881 (modem)"},	//
	{0x1199, 0x6852, sierra, 2, 0, 1, NULL, "Sierra AC880E (modem)"},	//
	{0x1199, 0x6853, sierra, 2, 0, 1, NULL, "Sierra AC881E (modem)"},	//
	{0x1199, 0x6855, sierra, 2, 0, 1, NULL, "Sierra AC880U (modem)"},	//
	{0x1199, 0x6856, sierra, 2, 0, 1, NULL, "Sierra AT&T USB Connect 881 (modem)"},	//
	{0x1199, 0x6859, sierra, 2, 0, 1, NULL, "Sierra AC885E (modem)"},	//
	{0x1199, 0x685a, sierra, 2, 0, 1, NULL, "Sierra AC885E (modem)"},	//
	{0x1199, 0x6880, sierra, 3, 3, 1, NULL, "Sierra C885"},	//
	{0x1199, 0x6890, sierra, 3, 3, 1, NULL, "Sierra C888"},	//
	{0x1199, 0x6891, sierra, 3, 3, 1, NULL, "Sierra C22/C33"},	//
	{0x1199, 0x6892, sierra, 3, 3, 1, NULL, "Sierra Compass HSPA"},	//
	{0x1199, 0x6893, sierra, 3, 3, 1, NULL, "Sierra C889"},	//
	{0x1199, 0x68a2, qcserial, 2, 0, 1 | QMI, NULL, "Sierra MC7710 (modem)"},	// also cdc_mbim
	{0x1199, 0x68a3, sierra, 3, 4, 1, &reset_mc, "Sierra MC8700/Compass Direct IP"},	// also sierra_net
	{0x1199, 0x68a5, qcserial, 2, 0, 1 | QMI, NULL, "Sierra MC8705 (modem)"},	//
	{0x1199, 0x68a9, qcserial, 2, 0, 1 | QMI, &select_config1, "Sierra MC7750"},	// cdc_mbim in default config2
	{0x1199, 0x68aa, sierra, 3, 3, 1, NULL, "Sierra AC320U/AC330U Direct IP"},	// also sierra_net
	{0x1199, 0x68c0, qcserial, 2, 0, 1 | QMI, NULL, "Sierra MC7304/7354"},	//
	{0x1199, 0x9011, qcserial, 2, 0, 1 | QMI, &select_config1, "Sierra MC8305 (modem)"},	// cdc_mbim in default config2
	{0x1199, 0x9013, qcserial, 2, 0, 1 | QMI, &select_config1, "Sierra MC8355 (modem)"},	// cdc_mbim in default config2
	{0x1199, 0x901b, qcserial, 2, 0, 1 | QMI, &select_config1, "Sierra MC7770"},	// cdc_mbim in default config2
	{0x1199, 0x901c, qcserial, 2, 0, 1 | QMI, &select_config1, "Sierra EM7700"},	// cdc_mbim in default config2
	{0x1199, 0x901f, qcserial, 2, 0, 1 | QMI, &select_config1, "Sierra EM7355"},	// cdc_mbim in default config2
	{0x1199, 0x9041, qcserial, 2, 0, 1 | QMI, &select_config1, "Sierra EM8805"},	// cdc_mbim in default config2
	{0x1199, 0x9051, qcserial, 2, 0, 1 | QMI, &select_config1, "Netgear AC340U (modem)"},	// cdc_mbim in default config2
	{0x1199, 0x9055, qcserial, 2, 0, 1 | QMI, &select_config1, "Netgear AC341U (modem)"},	//
	{0x1199, 0x9057, qcserial, 2, 0, 1 | QMI, &select_config1, "Netgear AC341U (modem)"},	//
	{0x1199, 0x9063, qcserial, 2, 0, 1 | QMI, &select_config1, "Sierra EM7305"},	// cdc_mbim in default config2
	{0x1199, 0x9071, qcserial, 2, 0, 1 | QMIRAW, &select_config1, "Sierra MC7430 (modem)"},	// cdc_mbim in default config2
	{0x1199, 0x9079, qcserial, 2, 0, 1 | QMIRAW, &select_config1, "Sierra EM7455 (modem)"},	// cdc_mbim in default config2

/* Pirelli Broadband Solutions */
	{0x1266, 0x1000, option, 0, 0, 0, &modeswitch_std_eject, "Pirelli"},	//
	{0x1266, 0x1009, option, 2, 0, 2, NULL, "Digicom 8E4455 (modem)"},	//

/* Huawei Technologies */
	{0x12d1, 0x1001, option, 2, 0, 2, &modeswitch_huawei_old, "HUAWEI E600/E620"},	//
	{0x12d1, 0x1003, option, 1, 0, 2, &modeswitch_huawei_old, "HUAWEI E172/EC27/E220/E230/E270"},	//
	{0x12d1, 0x1004, option, 1, 0, 2, &modeswitch_huawei_old, "HUAWEI E220BIS/K3520"},	//
	{0x12d1, 0x1009, option, 0, 0, 2, &modeswitch_huawei_old, "HUAWEI U120"},	//
	{0x12d1, 0x1010, option, 0, 0, 2, &modeswitch_huawei_old, "HUAWEI ETS1201"},	//
	{0x12d1, 0x101e, option, 0, 0, 2, &modeswitch_rezero, "HUAWEI U7510/U7517"},	//
	{0x12d1, 0x1030, none, 0, 0, 2, &modeswitch_huawei_std, "HUAWEI U8220 (Android smartphone)"},	//
	{0x12d1, 0x1031, none, 0, 0, 2, &modeswitch_huawei_std, "HUAWEI U8110 (Android smartphone)"},	//
	{0x12d1, 0x1034, option, 0, 0, 2, NULL, "HUAWEI U8220 (Android smartphone)"},	//
	{0x12d1, 0x1035, option, 0, 0, 2, NULL, "HUAWEI U8110 (Android smartphone)"},	//
//      {0x12d1, 0x1400, none, 0, 0, 0 | ETH, NULL,  "Huawei K4305 composite"},   // most likely qmi but not yet in driver
	{0x12d1, 0x1404, option, 2, 0, 2 | QMI, NULL, "HUAWEI UMG1831"},	//
	{0x12d1, 0x1406, option, 1, 0, 2, NULL, "HUAWEI newer modems"},	//
	{0x12d1, 0x140b, option, 2, 0, 2, NULL, "HUAWEI EC1260"},	//
	{0x12d1, 0x140c, option, 3, 0, 2 | QMI, NULL, "HUAWEI newer modems"},	//
	{0x12d1, 0x1411, option, 2, 0, 2, &modeswitch_huawei_old, "HUAWEI E510/EC121"},	//
	{0x12d1, 0x1412, option, 2, 0, 2, &modeswitch_huawei_old, "HUAWEI EC168"},	//
	{0x12d1, 0x1413, option, 2, 0, 2, &modeswitch_huawei_old, "HUAWEI EC168"},	//
	{0x12d1, 0x1414, option, 2, 0, 2, &modeswitch_huawei_old, "HUAWEI E180"},	//
	{0x12d1, 0x1417, option, 2, 0, 2, NULL, "HUAWEI E1756"},	//
	{0x12d1, 0x141b, option, 1, 0, 2, NULL, "HUAWEI newer modems"},	//
	{0x12d1, 0x1429, option, 2, 0, 2, NULL, "HUAWEI/EMobile D31HW"},	// also qmi but not yet in driver
//      {0x12d1, 0x1432, option, 0, 0, 2 | QMI, NULL, "HUAWEI E585"},     // ecm attributes but probably qmi
	{0x12d1, 0x1433, option, 2, 0, 2, NULL, "HUAWEI E1756C"},	// also qmi but not yet in driver
	{0x12d1, 0x1436, option, 2, 0, 2, NULL, "HUAWEI E1800"},	// ecm attributes but probably qmi
	{0x12d1, 0x1444, option, 0, 0, 2, NULL, "HUAWEI E352-R1"},	//
	{0x12d1, 0x1446, none, 0, 0, 2, &modeswitch_huawei_std, "HUAWEI E1552/E1800"},	//
	{0x12d1, 0x1449, none, 0, 0, 2, &modeswitch_huawei_std, "HUAWEI E352-R1"},	//
	{0x12d1, 0x144e, option, 0, 2, 2, NULL, "Huawei K3806"},	//
	{0x12d1, 0x1464, option, 2, 0, 2, NULL, "Huawei K4505"},	// also qmi but not yet in driver
	{0x12d1, 0x1465, option, 2, 0, 2, NULL, "Huawei K3765"},	// ecm attributes but probably qmi
//      {0x12d1, 0x1491, none, 0, 0, 2 | QMI, NULL, "Vodafone R201"},   // qmi only but not yet in driver
	{0x12d1, 0x14a5, option, 2, 0, 2, NULL, "Huawei E173"},	//
	{0x12d1, 0x14a8, option, 2, 0, 2, NULL, "Huawei E173"},	//
	{0x12d1, 0x14aa, option, 2, 0, 2, NULL, "Huawei E1750"},	//
	{0x12d1, 0x14ac, option, 2, 0, 2 | QMI, NULL, "HUAWEI newer modems"},	//
	{0x12d1, 0x14ad, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei K3806"},	//
	{0x12d1, 0x14ae, option, 1, 2, 2, NULL, "Huawei K3806"},	// also cdc_ether
	{0x12d1, 0x14b5, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei E173"},	//
	{0x12d1, 0x14b7, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei K4511"},	//
	{0x12d1, 0x14ba, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei E173/E177 (cdrom)"},	//
//      {0x12d1, 0x14bc, none, 0, 0, 0 | ETH, NULL,  "Huawei K3773 (net)"},       //
	{0x12d1, 0x14c1, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei K4605"},	//
	{0x12d1, 0x14c3, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei K5005"},	//
	{0x12d1, 0x14c4, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei K3771"},	//
	{0x12d1, 0x14c5, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei K4510"},	//
	{0x12d1, 0x14c6, option, 2, 0, 2 | QMI, NULL, "Huawei K4605"},	//
	{0x12d1, 0x14c8, option, 2, 0, 2 | QMI, NULL, "Huawei K5005"},	//
	{0x12d1, 0x14c9, option, 2, 0, 2, NULL, "Huawei K3770"},	//
	{0x12d1, 0x14ca, option, 2, 0, 2 | QMI, NULL, "Huawei K3771"},	//
	{0x12d1, 0x14cb, option, 2, 0, 2, NULL, "Huawei K4510"},	//
	{0x12d1, 0x14cc, option, 2, 0, 2 | QMI, NULL, "Huawei K4511"},	//
//      {0x12d1, 0x14cd, none, 2, 0, 2 | H_NCM, NULL, "Vodafone R205"}, // cdc_ncm only
	{0x12d1, 0x14cf, option, 2, 0, 2, NULL, "Huawei K3772 (modem)"},	// cdc_ncm able
	{0x12d1, 0x14d1, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei E182E"},	//
	{0x12d1, 0x14d2, option, 2, 0, 2 | QMI, NULL, "Huawei E173/E177 (modem)"},	//
//      {0x12d1, 0x14db, none, 0, 0, 0 | ETH, NULL,  "Huawei E353"},      //
//      {0x12d1, 0x14dc, none, 0, 0, 0 | ETH, NULL,  "Huawei E303"},      //
	{0x12d1, 0x14fe, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei E352,E353"},	//
	{0x12d1, 0x1505, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei E398"},	//
	{0x12d1, 0x1506, option, 2, 0, 2, NULL, "Huawei E367/E398 (modem)"},	// can not be QMI flagged!
	{0x12d1, 0x150a, option, 2, 0, 2 | QMI, NULL, "Huawei E398 (modem)"},	//
	{0x12d1, 0x150c, option, 1, 2, 2 | QMI, NULL, "Huawei E367"},	//
	{0x12d1, 0x150f, option, 0, 0, 2 | QMI, NULL, "Huawei E367"},	//
	{0x12d1, 0x151a, option, 0, 0, 2, &modeswitch_huawei_std, "Huawei E392u-12"},	//
	{0x12d1, 0x151b, option, 0, 0, 2 | QMI, NULL, "Huawei E392u-12"},	// 
	{0x12d1, 0x151d, option, 3, 0, 2, NULL, "Huawei E3131 (modem)"},	// ncm & mbim able
	{0x12d1, 0x1520, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei K3765"},	//
	{0x12d1, 0x1521, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei K4505"},	//
	{0x12d1, 0x1523, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei R201"},	//
	{0x12d1, 0x1526, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei K3772 (cdrom)"},	//
	{0x12d1, 0x1527, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei R206"},	//
	{0x12d1, 0x1553, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei E1553"},	//
	{0x12d1, 0x1557, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei E173"},	//
	{0x12d1, 0x155a, none, 0, 0, 2, &modeswitch_huawei_std, "Vodafone R205"},	//
	{0x12d1, 0x155b, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei E171/E320"},	//
	{0x12d1, 0x156a, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei E3251/E3276"},	//
	{0x12d1, 0x156c, option, 1, 0, 2, NULL, "Huawei E3276 (modem)"},	// also cdc_ncm
	{0x12d1, 0x1570, option, 1, 0, 2, NULL, "Huawei ME906E (modem)"},	// also cdc_mbim
	{0x12d1, 0x1571, option, 1, 0, 2, NULL, "Huawei EM820W (modem)"},	// also cdc_mbim
//      {0x12d1, 0x1575, none, 0, 0, 2 | ETH, NULL,  "Huawei K5150 composite"},   //
//      {0x12d1, 0x1576, none, 0, 0, 2 | ETH, NULL,  "Huawei K4201 composite"},   //
//      {0x12d1, 0x1577, none, 0, 0, 2 | ETH, NULL,  "Huawei K4202 composite"},   //
//      {0x12d1, 0x1578, none, 0, 0, 2 | ETH, NULL,  "Huawei K4606 composite"},   //
	{0x12d1, 0x157c, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei E3276s-150"},	// also cdc_mbim
	{0x12d1, 0x157d, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei E3331"},	// also cdc_mbim
	{0x12d1, 0x1581, none, 0, 0, 2, &modeswitch_huawei_std, "Vodafone R208"},	//
	{0x12d1, 0x1582, none, 0, 0, 2, &modeswitch_huawei_std, "Vodafone R215"},	//
	{0x12d1, 0x1583, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei E8278"},	//
//      {0x12d1, 0x1588, option, 0, 0, 2 | NCM, NULL, "Vodafone R215 (net)"},     //
//      {0x12d1, 0x1589, option, 0, 0, 2 | NCM, NULL, "Huawei E8278 (net)"},      //
//      {0x12d1, 0x1590, none, 0, 0, 2 | ETH, NULL, "Huawei K4203 composite"},    //
	{0x12d1, 0x15b1, option, 1, 0, 2, NULL, "Huawei E3531s-2 (modem)"},	// also ncm
	{0x12d1, 0x15ca, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei E3131"},	//
	{0x12d1, 0x15cd, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei E3372"},	//
	{0x12d1, 0x15ce, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei E3531s-2"},	//
	{0x12d1, 0x15e7, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei E3531"},	//
	{0x12d1, 0x1805, none, 1, 0, 2, &modeswitch_rezero, "Huawei U2800A/U6150 (modem)"},	//
	{0x12d1, 0x1c05, option, 2, 0, 2, NULL, "Huawei E173s (modem)"},	//
	{0x12d1, 0x1c07, option, 2, 0, 2, NULL, "Huawei E188 (modem)"},	// also cdc_ncm
	{0x12d1, 0x1c08, option, 1, 0, 2, NULL, "Huawei E173s (modem)"},	//
	{0x12d1, 0x1c0b, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei E173s (cdrom)"},	//
	{0x12d1, 0x1c10, option, 2, 0, 2, NULL, "Huawei E173 (modem)"},	//
	{0x12d1, 0x1c12, option, 2, 0, 2, NULL, "Huawei E173 (modem)"},	//
	{0x12d1, 0x1c1b, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei E398 (cdrom)"},	//
//      {0x12d1, 0x1c1e, option, 1, 0, 2 | NCM, NULL, "Huawei E586 (net)"},       //
//      {0x12d1, 0x1c1f, option, 0, 0, 2 | NCM, NULL, "Huawei E587 (net)"},       //
	{0x12d1, 0x1c20, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei E5220s-2 (cdrom)"},	//
	{0x12d1, 0x1c23, option, 0, 2, 2, NULL, "Huawei E173 (modem)"},	//
	{0x12d1, 0x1c24, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei E173 (cdrom)"},	//
	{0x12d1, 0x1d09, acm, 0, 0, 2, NULL, "Huawei ET8282 (modem)"},	//
	{0x12d1, 0x1da1, none, 2, 0, 2, &modeswitch_huawei_old, "Huawei ET8282 (cdrom)"},	//
	{0x12d1, 0x1f01, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei E303/E353 (cdrom)"},	//
	{0x12d1, 0x1f02, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei E5773 (cdrom)"},	//
	{0x12d1, 0x1f03, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei E353 (cdrom)"},	//
	{0x12d1, 0x1f04, none, 0, 0, 2, &modeswitch_huawei_std, "Vodafone R206_MR (cdrom)"},	//
	{0x12d1, 0x1f05, none, 0, 0, 2, &modeswitch_huawei_std, "Vodafone R207 (cdrom)"},	//
	{0x12d1, 0x1f06, none, 0, 0, 2, &modeswitch_huawei_std, "Vodafone R215_MR (cdrom)"},	//      
	{0x12d1, 0x1f07, none, 0, 0, 2, &modeswitch_huawei_std, "Vodafone R226 (cdrom)"},	//
	{0x12d1, 0x1f09, none, 0, 0, 2, &modeswitch_huawei_std, "Vodafone R216 (cdrom)"},	//
	{0x12d1, 0x1f0e, none, 0, 0, 2, &modeswitch_huawei_std, "KDDI U01 (cdrom)"},	//
	{0x12d1, 0x1f11, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei K3773 (cdrom)"},	//
	{0x12d1, 0x1f15, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei K4305 (cdrom)"},	// also cdc_mbim
	{0x12d1, 0x1f16, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei K5150 (cdrom)"},	// also cdc_mbim
	{0x12d1, 0x1f17, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei K4201 (cdrom)"},	// also cdc_mbim
	{0x12d1, 0x1f18, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei K4202 (cdrom)"},	// also cdc_mbim
	{0x12d1, 0x1f19, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei K4606 (cdrom)"},	// also cdc_mbim
	{0x12d1, 0x1f1b, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei Kxxxx (cdrom)"},	// also cdc_mbim
	{0x12d1, 0x1f1c, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei K4203 (cdrom)"},	// also cdc_mbim
	{0x12d1, 0x1f1d, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei Kxxxx (cdrom)"},	// also cdc_mbim
	{0x12d1, 0x1f1e, none, 0, 0, 2, &modeswitch_huawei_std, "Huawei K5160 (cdrom)"},	// also cdc_mbim

/* Novatel Wireless */
	{0x1410, 0x1400, option, 1, 0, 2, NULL, "Novatel U730 (modem)"},	//
	{0x1410, 0x1410, option, 1, 0, 2, NULL, "Novatel U740 (modem)"},	//
	{0x1410, 0x1420, option, 1, 0, 2, NULL, "Novatel U870 (modem)"},	//
	{0x1410, 0x1430, option, 1, 0, 2, NULL, "Novatel XU870 (modem)"},	//
	{0x1410, 0x1450, option, 1, 0, 2, NULL, "Novatel X950D (modem)"},	//
	{0x1410, 0x2100, option, 1, 0, 2, NULL, "Novatel EV620 (modem)"},	//
	{0x1410, 0x2110, option, 1, 0, 2, NULL, "Novatel ES720 (modem)"},	//
	{0x1410, 0x2120, option, 1, 0, 2, NULL, "Novatel E725 (modem)"},	//
	{0x1410, 0x2130, option, 1, 0, 2, NULL, "Novatel ES620 (modem)"},	//
	{0x1410, 0x2400, option, 1, 0, 2, NULL, "Novatel EU730 (modem)"},	//
	{0x1410, 0x2410, option, 1, 0, 2, NULL, "Novatel EU740 (modem)"},	//
	{0x1410, 0x2420, option, 1, 0, 2, NULL, "Novatel EU870D (modem)"},	//
	{0x1410, 0x4100, option, 1, 0, 2, NULL, "Novatel MC727/U727 (modem)"},	//
	{0x1410, 0x4400, option, 1, 0, 2, NULL, "Novatel Ovation MC930D/MC950D (modem)"},	//
	{0x1410, 0x5010, none, 0, 0, 2, &modeswitch_std_eject, "Novatel X950D(cdrom)"},	//
	{0x1410, 0x5020, none, 0, 0, 2, &modeswitch_std_eject, "Novatel MC990D (cdrom)"},	//
	{0x1410, 0x5023, none, 0, 0, 2, &modeswitch_std_eject, "Novatel MC996D (cdrom)"},	//
	{0x1410, 0x5030, none, 0, 0, 2, &modeswitch_std_eject, "Novatel U760 (cdrom)"},	//
	{0x1410, 0x5031, none, 0, 0, 2, &modeswitch_std_eject, "Novatel MC760 (cdrom)"},	//
	{0x1410, 0x5041, none, 0, 0, 2, &modeswitch_std_eject, "Novatel MiFi 2372 (cdrom)"},	//
	{0x1410, 0x5054, none, 0, 0, 2, &modeswitch_std_eject, "Novatel MiFi 4082 (cdrom)"},	//
	{0x1410, 0x5055, none, 0, 0, 2, &modeswitch_std_eject, "Novatel MiFi 4082 (cdrom)"},	//
	{0x1410, 0x5059, none, 0, 0, 2, &modeswitch_std_eject, "Novatel MC545/U679 (cdrom)"},	//
	{0x1410, 0x5076, none, 0, 0, 2, &modeswitch_std_eject, "Novatel MiFi 4620 (cdrom)"},	//
	{0x1410, 0x5077, none, 0, 0, 2, &modeswitch_std_eject, "Novatel MiFi 4620 (cdrom)"},	//
	{0x1410, 0x6000, option, 1, 0, 2, NULL, "Novatel U760 (modem)"},	//
	{0x1410, 0x6001, option, 1, 0, 2, NULL, "Novatel U760 (modem)"},	//
	{0x1410, 0x6002, option, 1, 0, 2, NULL, "Novatel U760 3G (modem)"},	//
	{0x1410, 0x6010, option, 1, 0, 2, NULL, "Novatel MC780 (modem)"},	//
	{0x1410, 0x7001, option, 1, 0, 2, NULL, "Novatel MiFi 2372 (modem)"},	//
	{0x1410, 0x7003, option, 1, 0, 2, NULL, "Novatel MiFi 2372 (modem)"},	//
	{0x1410, 0x7030, option, 0, 0, 2, NULL, "Novatel USB998 (modem)"},	// also cdc_ether
	{0x1410, 0x7031, generic, 0, 0, 2, NULL, "Novatel U679 (modem)"},	// also cdc_ether
	{0x1410, 0x7041, option, 0, 0, 2, NULL, "Novatel MF3470 (modem)"},	//
	{0x1410, 0x7042, option, 0, 0, 2, NULL, "Novatel Ovation MC545/MC547 (modem)"},	//
	{0x1410, 0x9010, option, 1, 0, 2 | QMI, NULL, "Novatel E362 (modem)"},	//
	{0x1410, 0x9011, option, 1, 0, 2 | QMI, NULL, "Novatel E371 (modem)"},	//
	{0x1410, 0xa001, qcserial, 1, 0, 2 | QMI, NULL, "Novatel USB1000 (modem)"},	//
	{0x1410, 0xa021, qcserial, 1, 0, 2 | QMI, NULL, "Novatel E396 (modem)"},	//
	{0x1410, 0xb001, option, 1, 0, 2 | QMI, NULL, "Novatel MC551/USB551L (modem)"},	//
//      {0x1410, 0xb003, option, 1, 0, 2 | QMI, NULL, "Novatel MiFi 4510"},       //
	{0x1410, 0xb005, option, 1, 0, 2, NULL, "Novatel MiFi 4620L/4620LE"},	// also cdc_ether(qmi?) and rndis
//      {0x1410, 0xb009, none, 0, 0, 2, &select_config2, "Novatel MiFi 5792"},  // rndis in config1, cdc_ether(qmi?) in config2
	{0x1410, 0xb00a, option, 1, 0, 2, NULL, "Novatel MiFi 5510"},	// also cdc_ether(qmi?) and rndis
//      {0x1410, 0xb00b, none, 0, 0, 2, &select_config2, "Novatel MiFi 5510L"}, // rndis in config1, cdc_ether(qmi?) in config2
//      {0x1410, 0xb00c, none, 0, 0, 2, &select_config2, "Novatel MiFi 6620L"}, // rndis in config1, cdc_ether(qmi?) in config2

/* UBIQUAM */
	{0x1529, 0x3100, acm, 0, 0, 2, NULL, "UBIQUAM U-100/105/200/300/520"},	//

/* VIA Telecom */
	{0x15eb, 0x0001, generic, 1, 0, 2, NULL, "Ublox FW2760/2770"},	//
	{0x15eb, 0x1231, acm, 0, 0, 2, NULL, "Prithvi UE100"},	//
	{0x15eb, 0x7152, generic, 3, 0, 2, NULL, "Tenda 3G189C"},	//
	{0x15eb, 0x7153, none, 0, 0, 2, &modeswitch_std_eject, "Tenda 3G189C (cdrom)"},	//

/* Amoi */
	{0x1614, 0x0800, option, 1, 0, 2, &modeswitch_rezero, "Amoi H01 (modem)"},	//
	{0x1614, 0x0802, option, 1, 0, 2, &modeswitch_rezero, "Amoi H02 (modem)"},	//
	{0x1614, 0x7002, option, 1, 0, 2, &modeswitch_rezero, "Amoi H01-A (modem)"},	//

/* AnyDATA */
	{0x16d5, 0x6202, option, 2, 0, 2, NULL, "AnyData ADU-620UW"},	//
	{0x16d5, 0x6501, option, 1, 0, 2, NULL, "AnyData ADU-300A"},	//
	{0x16d5, 0x6502, option, 2, 0, 2, NULL, "AnyData ADU-500A"},	//
	{0x16d5, 0x6603, generic, 0, 0, 2, NULL, "AnyData ADU-890WH"},	//
	{0x16d5, 0x900d, acm, 0, 0, 2, NULL, "AnyData ADU-890WH"},	//
	{0x16d5, 0xf000, none, 0, 0, 2, &modeswitch_std_eject, "AnyData (cdrom)"},	//

/* CMOTECH */
	{0x16d8, 0x5141, acm, 0, 0, 0, NULL, "Cmotech CNU-510"},	//
	{0x16d8, 0x5533, acm, 0, 0, 0, NULL, "Cmotech CCU-550"},	//
	{0x16d8, 0x5543, acm, 0, 0, 0, NULL, "Cmotech CNU-550"},	//
	{0x16d8, 0x5553, acm, 0, 0, 0, NULL, "Cmotech CDU-550"},	//
	{0x16d8, 0x6002, option, 1, 0, 0, NULL, "Franklin U300"},	//
	{0x16d8, 0x6006, option, 0, 0, 0, NULL, "Cmotech CGU-628"},	//
	{0x16d8, 0x6007, option, 0, 0, 0 | QMI, NULL, "Cmotech CHE-628S"},	//
	{0x16d8, 0x6008, option, 2, 1, 0 | QMI, NULL, "Franklin U301"},	//
	{0x16d8, 0x6280, option, 2, 1, 0 | QMI, &modeswitch_devchg_ff, "Cmotech CHU-628s"},	//
	{0x16d8, 0x6281, option, 2, 0, 0, &modeswitch_devchg_ff, "Cmotech CHU-628s"},	//
	{0x16d8, 0x6522, acm, 2, 0, 0, NULL, "Cmotech CDU-650"},	//
	{0x16d8, 0x6523, acm, 2, 0, 0, NULL, "Cmotech CCU-650U"},	//
	{0x16d8, 0x6532, acm, 2, 0, 0, NULL, "Cmotech CCU-650"},	//
	{0x16d8, 0x6533, acm, 0, 0, 0, NULL, "Cmotech CNM-650"},	//
	{0x16d8, 0x6543, acm, 0, 0, 0, NULL, "Cmotech CNU-650"},	//
	{0x16d8, 0x6803, option, 1, 0, 0, &modeswitch_devchg1, "Cmotech CDU-680"},	//
	{0x16d8, 0x6804, option, 2, 1, 0, &modeswitch_devchg1, "Cmotech CDU-685A"},	//
	{0x16d8, 0x680a, acm, 0, 0, 0, NULL, "Cmotech CDU-680"},	//
	{0x16d8, 0x7001, option, 1, 1, 0 | QMI, &modeswitch_devchg_fe, "Cmotech CHU-720S"},	//
	{0x16d8, 0x7003, option, 1, 2, 0 | QMI, &modeswitch_devchg_fe, "Cmotech CHU-629K"},	//
	{0x16d8, 0x7006, option, 1, 2, 0 | QMI, &modeswitch_devchg_fe, "Cmotech CGU-629"},	//
	{0x16d8, 0x700a, option, 0, 2, 0 | QMI, &modeswitch_devchg_fe, "Cmotech CHU-629S"},	//'
	{0x16d8, 0x7211, option, 1, 1, 0 | QMI, &modeswitch_devchg_fe, "Cmotech CHU-720I"},	//
	{0x16d8, 0xf000, none, 0, 0, 0, &modeswitch_devchg_ff, "Cmotech CGU-628, 4g_xsstick W12"},	//

/* AxessTel */
	{0x1726, 0x1000, generic, 0, 0, 2, NULL, "Axesstel MU130 (modem)"},	//
	{0x1726, 0x1900, none, 0, 0, 0, &modeswitch_std_eject, "Axesstel MV140B"},	//
	{0x1726, 0xa000, generic, 2, 3, 2, NULL, "Axesstel MU130 (modem)"},	//
	{0x1726, 0xf00e, none, 0, 0, 0, &modeswitch_std_eject, "Axesstel MU130"},	//

/* MODMEN */
	{0x198a, 0x0003, none, 0, 0, 0, &modeswitch_std_eject, "MODMEN MM450"},	//
	{0x198a, 0x0002, generic, 2, 0, 2, NULL, "MODMEN MM450 (modem)"},	//

/* ZTE WCDMA Technologies */
	{0x19d2, 0x0001, option, 2, 0, 2, NULL, "ONDA MT505UP/ZTE (modem)"},	//
	{0x19d2, 0x0002, option, 2, 0, 2 | QMI, NULL, "ZTE ET502HS/MT505UP/MF632"},	//
	{0x19d2, 0x0003, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MU351 (cdrom)"},	//
	{0x19d2, 0x0015, option, 2, 0, 2, NULL, "ONDA MT505UP/ZTE (modem)"},	//
	{0x19d2, 0x0016, option, 1, 2, 2, NULL, "ONDA MF110/ZTE (modem)"},	//
	{0x19d2, 0x0017, option, 1, 2, 2 | QMI, NULL, "ONDA MT505UP/ZTE (modem)"},	//
	{0x19d2, 0x0018, option, 1, 2, 2, NULL, "ONDA MSA110UP/ZTE (modem)"},	//
	{0x19d2, 0x0019, option, 1, 2, 2 | QMI, NULL, "ONDA MT689DC/ZTE (modem)"},	//
	{0x19d2, 0x0022, option, 1, 0, 2, NULL, "ZTE K2525 (modem)"},	//
	{0x19d2, 0x0024, option, 2, 0, 2, NULL, "ONDA MT503HSA (modem)"},	// also qmi but not yet in driver
	{0x19d2, 0x0025, option, 4, 2, 2 | QMI, NULL, "ZTE MF628 (modem)"},	//
	{0x19d2, 0x0026, none, 0, 0, 2, &modeswitch_std_eject, "ZTE AC581 (cdrom)"},	//
	{0x19d2, 0x0031, option, 1, 2, 2, NULL, "ZTE MF110/MF112/MF626 (Variant) (modem)"},	// don't flag qmi - multiple versions exist
	{0x19d2, 0x0033, option, 1, 4, 2, NULL, "ZTE MF636 (modem)"},	//
	{0x19d2, 0x0034, acm, 0, 1, 2, NULL, "ZTE MU330"},	//
	{0x19d2, 0x0037, option, 2, 2, 2, NULL, "ONDA MT505UP/ZTE (modem)"},	// also qmi but not yet in driver
	{0x19d2, 0x0039, option, 1, 2, 2, NULL, "ZTE MF100 (modem)"},	//
	{0x19d2, 0x0040, none, 0, 0, 2, &modeswitch_std_eject, "ZTE K2525 (cdrom)"},	//
	{0x19d2, 0x0042, option, 1, 2, 2 | QMI, NULL, "ZTE MF190 (modem)"},	//
	{0x19d2, 0x0052, option, 1, 2, 2 | QMI, NULL, "ONDA MT505UP/ZTE (modem)"},	//
	{0x19d2, 0x0053, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF110 (Variant) (modem)"},	//
	{0x19d2, 0x0055, option, 3, 1, 2 | QMI, NULL, "ONDA MT505UP/ZTE (modem)"},	//
	{0x19d2, 0x0057, option, 0, 2, 2, NULL, "AIKO 83D (modem)"},	//
	{0x19d2, 0x0063, option, 1, 3, 2 | QMI, NULL, "ZTE K3565-Z (modem)"},	//
	{0x19d2, 0x0064, option, 0, 2, 2, NULL, "ZTE MF627 (modem)"},	//
	{0x19d2, 0x0066, option, 1, 3, 2, NULL, "ZTE MF626 (modem)"},	//
	{0x19d2, 0x0073, option, 1, 0, 2, NULL, "ZTE A580 (modem)"},	//
	{0x19d2, 0x0079, option, 2, 0, 2, NULL, "ZTE A353 (modem)"},	//
	{0x19d2, 0x0082, option, 1, 2, 2, NULL, "ZTE MF668/MF190 (Variant) (modem)"},	//
	{0x19d2, 0x0083, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF110 (Variant) (cdrom)"},	//
	{0x19d2, 0x0086, option, 1, 2, 2, NULL, "ZTE MF645 (modem)"},	//
	{0x19d2, 0x0091, option, 1, 3, 2, NULL, "ZTE MF636 (modem)"},	// also qmi but not yet in driver
	{0x19d2, 0x0094, option, 2, 0, 2, NULL, "ZTE AC581 (modem)"},	//
	{0x19d2, 0x0101, none, 0, 0, 2, &modeswitch_std_eject, "ZTE K4505-Z (cdrom)"},	//
	{0x19d2, 0x0103, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF112 (cdrom)"},	//
	{0x19d2, 0x0104, option, 1, 3, 2 | QMI, NULL, "ZTE K4505-Z (modem)"},	//
	{0x19d2, 0x0108, option, 1, 3, 2, NULL, "ONDA MT505UP/ZTE (modem)"},	//
	{0x19d2, 0x0110, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF637 (cdrom)"},	//
	{0x19d2, 0x0115, none, 0, 0, 2, &modeswitch_std_eject, "ONDA MT505UP/ZTE (cdrom)"},	//
	{0x19d2, 0x0116, acm, 1, 0, 2, NULL, "ZTE MF651 (modem)"},	//
	{0x19d2, 0x0117, option, 1, 2, 2, NULL, "ZTE MF112 (modem)"},	//
	{0x19d2, 0x0120, none, 0, 0, 2, &modeswitch_std_eject, "ZTE A353 (cdrom)"},	//
	{0x19d2, 0x0121, option, 1, 3, 2 | QMI, NULL, "ZTE MF637 (modem)"},	//
	{0x19d2, 0x0124, option, 1, 4, 2 | QMI, NULL, "ZTE MF110 (Variant) (modem)"},	//
	{0x19d2, 0x0128, option, 1, 3, 2, NULL, "ZTE MF651 (modem)"},	//
	{0x19d2, 0x0142, acm, 0, 0, 2, NULL, "ZTE MF665C"},	// also cdc_ether
	{0x19d2, 0x0143, acm, 0, 0, 2, NULL, "ZTE MF190B"},	// also cdc_ether
	{0x19d2, 0x0146, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF652 (cdrom)"},	//
	{0x19d2, 0x0149, none, 0, 0, 2, &modeswitch_zte, "ZTE MF190 (cdrom)"},	//
	{0x19d2, 0x0150, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF680 (cdrom)"},	//
	{0x19d2, 0x0151, option, 1, 2, 2, NULL, "Prolink PHS101 (modem)"},	//
	{0x19d2, 0x0152, option, 2, 0, 2, NULL, "ZTE AC583 (modem)"},	//
	{0x19d2, 0x0154, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF190S (cdrom)"},	//
	{0x19d2, 0x0157, option, 0, 4, 2 | QMI, NULL, "ZTE MF683 (modem)"},	//
	{0x19d2, 0x0166, none, 0, 0, 2, &modeswitch_others, "ZTE MF821 (Variant) (cdrom)"},	//
	{0x19d2, 0x0167, option, 1, 3, 2 | QMI, NULL, "ZTE MF820D (variant) (modem)"},	//
	{0x19d2, 0x0169, none, 0, 0, 2, &modeswitch_std_eject, "ZTE A371 (cdrom)"},	//
	{0x19d2, 0x0170, option, 0, 1, 2, NULL, "ZTE A371 (variant) (modem)"},	//
	{0x19d2, 0x0198, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF820S (cdrom)"},	//
	{0x19d2, 0x0199, option, 1, 2, 2 | QMI, NULL, "ZTE MF820S (modem)"},	//
	{0x19d2, 0x0257, option, 1, 2, 2 | QMI, NULL, "ZTE MF821 (variant) (modem)"},	//
	{0x19d2, 0x0265, option, 2, 3, 2 | QMI, NULL, "Onda MT8205/ZTE (modem)"},	//
	{0x19d2, 0x0266, none, 0, 0, 2, &modeswitch_others, "Onda MT8205/ZTE (cdrom)"},	//
	{0x19d2, 0x0284, option, 1, 3, 2 | QMI, NULL, "ZTE MF880 (modem)"},	//
	{0x19d2, 0x0304, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF821D (cdrom)"},	//
	{0x19d2, 0x0317, option, 1, 2, 2, NULL, "ZTE MF826 (modem)"},	// also qmi but not yet in driver
	{0x19d2, 0x0318, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF826 (cdrom)"},	//
	{0x19d2, 0x0325, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF821D (cdrom)"},	//
	{0x19d2, 0x0326, option, 1, 3, 2 | QMI, NULL, "ZTE MF821D (modem)"},	//
	{0x19d2, 0x0330, option, 1, 2, 2, NULL, "ZTE MF826 (modem)"},	// also qmi but not yet in driver
//      {0x19d2, 0x0349, none, 0, 0, 2 | ETH, NULL, "ZTE MF821D (modem)"},        // ecm attributes, might be QMI
//      {0x19d2, 0x0387, none, 0, 0, 2 | ETH, NULL, "ZTE MF827 (modem)"}, // ecm attributes, might be QMI
	{0x19d2, 0x0388, none, 0, 0, 2, &modeswitch_others, "ZTE MF827 (cdrom)"},	//
	{0x19d2, 0x0412, option, 2, 3, 2 | QMI, NULL, "Telewell TW-LTE 4G (modem)"},	//
	{0x19d2, 0x0413, none, 0, 0, 2, &modeswitch_others, "Telewell TW-LTE 4G (cdrom)"},	//
	{0x19d2, 0x1001, none, 0, 0, 2, &modeswitch_std_eject, "ZTE K3805-Z (cdrom)"},	//
	{0x19d2, 0x1003, acm, 1, 0, 2, NULL, "ZTE K3805-Z (modem)"},	// also cdc_ether
	{0x19d2, 0x1007, none, 0, 0, 2, &modeswitch_std_eject, "ZTE K3570-Z (cdrom)"},	//
	{0x19d2, 0x1008, option, 1, 3, 2 | QMI, NULL, "ZTE K3570-Z (modem)"},	//
	{0x19d2, 0x1009, none, 0, 0, 2, &modeswitch_std_eject, "ZTE K3571-Z (cdrom)"},	//
	{0x19d2, 0x1010, option, 1, 3, 2 | QMI, NULL, "ZTE K3571-Z (modem)"},	//
	{0x19d2, 0x1013, none, 0, 0, 2, &modeswitch_std_eject, "ZTE K3806-Z (cdrom)"},	//
	{0x19d2, 0x1015, acm, 1, 0, 2, NULL, "ZTE K3806-Z (modem)"},	// also cdc_ether
	{0x19d2, 0x1017, none, 0, 0, 2, &modeswitch_std_eject, "ZTE K5006-Z (cdrom)"},	//
	{0x19d2, 0x1018, option, 1, 2, 2 | QMI, NULL, "ZTE K5006-Z (modem)"},	//
	{0x19d2, 0x1019, none, 0, 0, 2, &modeswitch_std_eject, "ZTE R203 (cdrom)"},	//
	{0x19d2, 0x1020, none, 0, 0, 2, &modeswitch_std_eject, "ZTE R203 (cdrom)"},	//
	{0x19d2, 0x1021, option, 1, 2, 2 | QMI, NULL, "ZTE R203 (modem)"},	//
	{0x19d2, 0x1022, none, 0, 0, 2, &modeswitch_std_eject, "ZTE K4201-Z (cdrom)"},	// also cdc_mbim
//      {0x19d2, 0x1023, none, 0, 0, 0 | ETH, NULL, "ZTE K4201-Z (modem)"},       //
	{0x19d2, 0x1026, none, 0, 0, 2, &modeswitch_std_eject, "ZTE R212 (cdrom)"},	//
	{0x19d2, 0x1030, none, 0, 0, 2, &modeswitch_std_eject, "ZTE K5008-Z (cdrom)"},	// also cdc_mbim
//      {0x19d2, 0x1032, none, 0, 0, 0 | ETH, NULL, "ZTE K5008-Z (modem)"},       //
	{0x19d2, 0x1034, none, 0, 0, 2, &modeswitch_std_eject, "ZTE R206-z (cdrom)"},	//
	{0x19d2, 0x1038, none, 0, 0, 2, &modeswitch_std_eject, "ZTE K4607-Z (cdrom)"},	//
//      {0x19d2, 0x1040, none, 0, 0, 0 | ETH, NULL, "ZTE K4607-Z (modem)"},       //
	{0x19d2, 0x1042, none, 0, 0, 2, &modeswitch_std_eject, "ZTE R209-z (cdrom)"},	//
	{0x19d2, 0x1171, none, 0, 0, 2, &modeswitch_std_eject, "ZTE K4510-Z (cdrom)"},	//
	{0x19d2, 0x1172, acm, 0, 0, 2, NULL, "ZTE K4510-Z (modem)"},	// also cdc_ether
	{0x19d2, 0x1173, acm, 0, 0, 2, NULL, "ZTE K4510-Z (modem)"},	// also cdc_ether
	{0x19d2, 0x1175, none, 0, 0, 2, &modeswitch_std_eject, "ZTE K3770-Z (cdrom)"},	//
	{0x19d2, 0x1176, option, 0, 0, 2 | QMI, NULL, "ZTE K3770-Z (modem)"},	//
	{0x19d2, 0x1177, acm, 0, 0, 2, NULL, "ZTE K3770-Z (modem)"},	// also cdc_ether
	{0x19d2, 0x1179, none, 0, 0, 2, &modeswitch_std_eject, "ZTE K3772-Z (cdrom)"},	//
	{0x19d2, 0x1181, acm, 0, 0, 2, NULL, "ZTE K3772-Z (modem)"},	// also cdc_ether
	{0x19d2, 0x1201, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF691 (cdrom)"},	//
	{0x19d2, 0x1203, acm, 0, 0, 2, NULL, "ZTE MF691 (modem)"},	//
	{0x19d2, 0x1207, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF192 (cdrom)"},	//
	{0x19d2, 0x1208, acm, 0, 0, 2, NULL, "ZTE MF192 (modem)"},	//
	{0x19d2, 0x1210, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF195 (cdrom)"},	//
	{0x19d2, 0x1211, acm, 0, 0, 2, NULL, "ZTE MF195 (modem)"},	// also cdc_ether
	{0x19d2, 0x1212, acm, 0, 0, 2, NULL, "ZTE MF195 (modem)"},	//
	{0x19d2, 0x1216, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF192 (cdrom)"},	//
	{0x19d2, 0x1217, acm, 0, 0, 2, NULL, "ZTE MF192"},	// also cdc_ether
	{0x19d2, 0x1218, acm, 0, 0, 2, NULL, "ZTE MF192"},	//
	{0x19d2, 0x1219, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF192 (cdrom)"},	//
	{0x19d2, 0x1220, acm, 0, 0, 2, NULL, "ZTE MF192"},	// also cdc_ether
	{0x19d2, 0x1222, acm, 0, 0, 2, NULL, "ZTE MF192"},	// also cdc_ether
	{0x19d2, 0x1224, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF190 (cdrom)"},	//
	{0x19d2, 0x1225, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF667 (cdrom)"},	//
	{0x19d2, 0x1227, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF669 (cdrom)"},	//
	{0x19d2, 0x1232, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF193A (cdrom)"},	//
	{0x19d2, 0x1233, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF667 (cdrom)"},	//
	{0x19d2, 0x1237, none, 0, 0, 2, &modeswitch_std_eject, "ZTE K4201-z I (cdrom)"},	//
	{0x19d2, 0x1238, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF825A (cdrom)"},	//
	{0x19d2, 0x1245, option, 1, 0, 2 | QMI, NULL, "ZTE MF680 (modem)"},	//
	{0x19d2, 0x1252, option, 1, 3, 2 | QMI, NULL, "ZTE MF669 (modem)"},	//
	{0x19d2, 0x1253, option, 1, 3, 2, NULL, "Prolink PHS300 (modem)"},	//
	{0x19d2, 0x1254, option, 1, 3, 2 | QMI, NULL, "ZTE MF190 (modem)"},	//
	{0x19d2, 0x1256, option, 1, 0, 2 | QMI, NULL, "ZTE MF190 (modem)"},	//
	{0x19d2, 0x1268, option, 1, 3, 2, NULL, "ZTE MF667 (modem)"},	// also qmi but not yet in driver
	{0x19d2, 0x1270, option, 1, 4, 2 | QMI, NULL, "ZTE MF667 (modem)"},	//
	{0x19d2, 0x1300, option, 2, 0, 2, NULL, "ZTE MF220 (modem)"},	//
	{0x19d2, 0x1401, option, 0, 0, 2 | QMI, NULL, "ZTE MF60 (modem)"},	//
	{0x19d2, 0x1402, option, 1, 1, 2 | QMI, NULL, "ZTE MF60 (modem)"},	//
//      {0x19d2, 0x1403, none, 0, 0, 0 | RNDIS, NULL, "ZTE MF825A (modem)"},      //
//      {0x19d2, 0x1405, none, 0, 0, 0 | ETH, NULL, "ZTE MF667 (modem)"}, // qmi tested - failed
//      {0x19d2, 0x1408, none, 0, 0, 0 | ETH, NULL, "ZTE MF825A (modem)"},        // qmi tested - failed
	{0x19d2, 0x1420, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF730 (cdrom)"},	//
	{0x19d2, 0x1426, option, 1, 0, 2 | QMI, NULL, "ZTE MF91D (modem)"},	//
	{0x19d2, 0x1428, option, 1, 0, 2 | QMI, NULL, "Telewell TW-LTE 4G v2 (modem)"},	//
	{0x19d2, 0x1511, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MFxxx (cdrom)"},	//
	{0x19d2, 0x1512, acm, 0, 0, 2, NULL, "ZTE MFxxx (modem)"},	// also cdc_ether
	{0x19d2, 0x1514, none, 0, 0, 2, &modeswitch_onda, "ZTE MF192 (cdrom)"},	//
	{0x19d2, 0x1515, acm, 0, 0, 2, NULL, "ZTE MF192 (modem)"},	//
	{0x19d2, 0x1517, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF192 (cdrom)"},	//
	{0x19d2, 0x1518, acm, 0, 0, 2, NULL, "ZTE MF192 (modem)"},	//
	{0x19d2, 0x1519, acm, 0, 0, 2, NULL, "ZTE MF192 (modem)"},	//
	{0x19d2, 0x1520, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF652 (cdrom)"},	//
	{0x19d2, 0x1522, acm, 0, 0, 2, NULL, "ZTE MF652 (modem)"},	//
	{0x19d2, 0x1523, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF591 (cdrom)"},	//
	{0x19d2, 0x1525, acm, 0, 0, 2, NULL, "ZTE MF591 (modem)"},	//
	{0x19d2, 0x1527, acm, 0, 0, 2, NULL, "ZTE MF196 (modem)"},	//
	{0x19d2, 0x1528, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF196 (cdrom)"},	//
	{0x19d2, 0x1529, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MFxxx (cdrom)"},	//
	{0x19d2, 0x1530, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MFxxx (cdrom)"},	//
	{0x19d2, 0x1536, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF190J (cdrom)"},	//
	{0x19d2, 0x1537, acm, 0, 0, 2, NULL, "ZTE MF190J (modem)"},	// also cdc_ether
	{0x19d2, 0x1538, acm, 0, 0, 2, NULL, "ZTE MF190J (modem)"},	// also cdc_ether
	{0x19d2, 0x1542, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF190J (cdrom)"},	//
	{0x19d2, 0x1544, acm, 0, 0, 2, NULL, "ZTE MF190J (modem)"},	//
	{0x19d2, 0x1580, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF195E (cdrom)"},	//
	{0x19d2, 0x1582, acm, 0, 0, 2, NULL, "ZTE MF195E (modem)"},	//
	{0x19d2, 0x1588, none, 0, 0, 2, &modeswitch_zte, "ZTE MF710 (cdrom)"},	//
	{0x19d2, 0x1589, option, 3, 2, 2, NULL, "ZTE MF710 (modem)"},	// also cdc_ether
	{0x19d2, 0x1592, option, 3, 2, 2, NULL, "ZTE MF710 (modem)"},	// also cdc_ether
	{0x19d2, 0x1595, none, 0, 0, 2, &modeswitch_zte, "ZTE MF710 (cdrom)"},	//
	{0x19d2, 0x1600, option, 3, 2, 2, NULL, "ZTE MF710 (modem)"},	// also cdc_ether
	{0x19d2, 0x2000, none, 0, 0, 2, &modeswitch_zte, "ONDA/ZTE (cdrom)"},	//
	{0x19d2, 0x2002, option, 1, 3, 2 | QMI, NULL, "ZTE K3765-Z (modem)"},	//
	{0x19d2, 0x2003, option, 1, 3, 2, NULL, "ZTE MF180 (modem)"},	//
	{0x19d2, 0x2004, none, 0, 0, 2, &modeswitch_std_eject, "ZTE MF60 (cdrom)"},	//
	{0x19d2, 0xffdd, generic, 1, 0, 2, NULL, "ZTE AC682 (modem)"},	//
	{0x19d2, 0xffde, none, 0, 0, 2, &modeswitch_std_eject, "ZTE AC682 (cdrom)"},	//
	{0x19d2, 0xffe4, generic, 1, 0, 2, NULL, "ZTE AC3781 (modem)"},	//
	{0x19d2, 0xffe8, option, 1, 0, 2, NULL, "ZTE MC2718 (modem)"},	//
	{0x19d2, 0xffe9, option, 1, 0, 2, NULL, "ZTE AC2738 (modem)"},	//
	{0x19d2, 0xffeb, option, 0, 3, 2, NULL, "ZTE AD3812 (modem)"},	//
	{0x19d2, 0xffed, option, 1, 0, 2, NULL, "ZTE MC2716 (modem)"},	//
	{0x19d2, 0xfff1, option, 1, 0, 2, NULL, "ZTE generic (modem)"},	//
	{0x19d2, 0xfff3, qcserial, 1, 2, 2 | QMI, NULL, "ZTE generic (modem)"},	//
	{0x19d2, 0xfff5, none, 0, 0, 2, &modeswitch_others, "ZTE generic (cdrom)"},	//
	{0x19d2, 0xfff6, none, 0, 0, 2, &modeswitch_others, "ZTE generic (cdrom)"},	//
	{0x19d2, 0xfffb, option, 1, 0, 2, NULL, "ZTE MG880 (modem)"},	//
	{0x19d2, 0xfffc, option, 1, 0, 2, NULL, "ZTE MG880 (modem)"},	//
	{0x19d2, 0xfffd, option, 1, 0, 2, NULL, "ZTE MG880 (modem)"},	//
	{0x19d2, 0xfffe, option, 1, 0, 2, NULL, "ZTE AC8700 (modem)"},	//
	{0x19d2, 0xffff, option, 1, 0, 2, NULL, "ZTE AC8710 (modem)"},	//

/* Infomark */
//      {0x19f2, 0x1700, option, 0, 0, 0 | ETH, NULL, "Clear Spot Voyager mifi"}, //

/* Qualcomm */
	{0x19f5, 0x9905, generic, 2, 1, 2, NULL, "Venus Fast2 (modem)"},	//
	{0x19f5, 0x9909, generic, 2, 1, 2, NULL, "Venus Fast2 (modem)"},	//
	{0x19f5, 0xf000, none, 0, 0, 2, &modeswitch_std_eject, "Advan Jetx DT-8 (cdrom)"},	//

/* Bandrich */
	{0x1a8d, 0x1000, none, 0, 0, 2, &modeswitch_std_eject, "Bandrich C-1xx/C-270/C-32x (cdrom)"},	//
//      {0x1a8d, 0x1001, none, 0, 0, 2 | ETH, NULL, "Bandrich C-100/C-120 (netif)"},   //
	{0x1a8d, 0x1002, option, 1, 0, 2, NULL, "Bandrich C-100/C-120 (modem)"},	//
	{0x1a8d, 0x1003, option, 1, 0, 2, NULL, "Bandrich C-100/C-120 (modem)"},	//
	{0x1a8d, 0x1007, option, 2, 0, 2, NULL, "Bandrich C-270 (modem)"},	//
	{0x1a8d, 0x1008, option, 2, 0, 2, NULL, "Bandrich M250 (modem)"},	//
	{0x1a8d, 0x1009, option, 2, 0, 2, NULL, "Bandrich C-170/C-180 (modem)"},	//
	{0x1a8d, 0x100c, option, 2, 0, 2, NULL, "Bandrich C-320 (modem)"},	// also cdc_ether
	{0x1a8d, 0x100d, option, 2, 0, 2, NULL, "Bandrich C-508 (modem)"},	// also cdc_ether
	{0x1a8d, 0x2000, none, 0, 0, 2, &modeswitch_std_eject, "Bandrich C33x (cdrom)"},	//
	{0x1a8d, 0x2006, acm, 1, 1, 2, NULL, "Bandrich C-33x (modem)"},	//

/* Datang */
	{0x1ab7, 0x2000, generic, 0, 0, 2, NULL, "Aircard 901 (modem)"},	//
	{0x1ab7, 0x5700, none, 0, 0, 2, &modeswitch_std_eject, "Datang DTM573x (cdrom)"},	//
	{0x1ab7, 0x5730, generic, 3, 1, 2, NULL, "Datang DTM5730 (modem)"},	//
	{0x1ab7, 0x5731, generic, 3, 1, 2, NULL, "Datang DTM5731 (modem)"},	//

/* T&A Mobile Phones (Alcatel) */
	{0x1bbb, 0x0000, option, 2, 2, 2, NULL, "Alcatel X060S/X070S/X080S/X200 (modem)"},	//
	{0x1bbb, 0x0012, generic, 2, 2, 2, NULL, "Alcatel X085C (modem)"},	//
	{0x1bbb, 0x0017, option, 4, 4, 2, NULL, "Alcatel X220L (Variant), X500D (modem)"},	//
	{0x1bbb, 0x0052, option, 4, 4, 2, NULL, "Alcatel X220L (Variant), (modem)"},	//
	{0x1bbb, 0x00b7, option, 0, 4, 2, NULL, "Alcatel X600 (modem)"},	//
	{0x1bbb, 0x00ca, generic, 0, 0, 2, NULL, "Alcatel X080C (modem)"},	//
	{0x1bbb, 0x011e, option, 1, 2, 2 | QMI, NULL, "Alcatel L100V, (modem)"},	//
//      {0x1bbb, 0x0195, none, 0, 0, 0 | RNDIS, NULL, "Alcatel L800, (modem)"},   //
	{0x1bbb, 0x0203, option, 0, 1, 2 | QMI, NULL, "Alcatel L800Z, (modem)"},	//
	{0x1bbb, 0x022c, generic, 2, 1, 2, &select_config2, "Alcatel X602D (modem)"},	//
	{0x1bbb, 0xf000, none, 0, 0, 2, &modeswitch_alcatel, "Alcatel X060S/X070S/X080S/X200/X220L/X500D(cdrom)"},	//
	{0x1bbb, 0xf017, none, 0, 0, 2, &modeswitch_alcatel, "Alcatel X220D (cdrom)"},	//
	{0x1bbb, 0xf052, none, 0, 0, 2, &modeswitch_alcatel, "Alcatel X220L (cdrom)"},	//
	{0x1bbb, 0xf0b6, none, 0, 0, 2, &modeswitch_alcatel, "Alcatel X550L (cdrom)"},	//
	{0x1bbb, 0xf0b7, none, 0, 0, 2, &modeswitch_alcatel, "Alcatel X600L (cdrom)"},	//

/* Telit */
	{0x1bc7, 0x0021, acm, 0, 0, 2, NULL, "Telit HE910, (modem)"},	//
	{0x1bc7, 0x1010, acm, 0, 0, 2, NULL, "Telit DE910, (modem)"},	//
	{0x1bc7, 0x1201, none, 0, 0, 0 | QMI, NULL, "Telit LE910, (modem)"},	//

/* Longcheer */
	{0x1c9e, 0x1001, none, 0, 0, 2, &modeswitch_alcatel, "Alcatel X020 & X030 (cdrom)"},	//
	{0x1c9e, 0x3197, generic, 1, 0, 2, NULL, "SEV759 (modem)"},	//
	{0x1c9e, 0x6000, generic, 2, 0, 2, &modeswitch_rezero, "Alcatel X020 & X030 (modem)"},	//
	{0x1c9e, 0x6060, generic, 2, 0, 2, &modeswitch_rezero, "TU930"},	//
	{0x1c9e, 0x6061, generic, 2, 0, 2, NULL, "Alcatel X020 & X030 (modem)"},	//
	{0x1c9e, 0x9000, generic, 1, 2, 2, NULL, "4G Systems XS Stick W14 (modem)"},	//
	{0x1c9e, 0x9101, none, 0, 0, 2, &modeswitch_alcatel, "EMobile D12LC (cdrom)"},	//
	{0x1c9e, 0x9104, generic, 1, 2, 2, NULL, "EMobile D12LC(modem)"},	//
	{0x1c9e, 0x9401, none, 0, 0, 2, &modeswitch_alcatel, "EMobile D21LC (cdrom)"},	//
	{0x1c9e, 0x9404, generic, 1, 2, 2, NULL, "EMobile D21LC(modem)"},	//
	{0x1c9e, 0x9603, option, 1, 2, 2, NULL, "4G Systems XS Stick W14 (modem)"},	//
	{0x1c9e, 0x9605, option, 1, 3, 2, NULL, "4G Systems XS Stick W14 (modem)"},	//
	{0x1c9e, 0x9607, option, 1, 3, 2, NULL, "4G Systems XS Stick W14 (modem)"},	//
	{0x1c9e, 0x9800, generic, 2, 1, 2, &modeswitch_alcatel, "SU-9800 (modem)"},	//
	{0x1c9e, 0x9801, generic, 2, 1, 2, NULL, "4G Systems XS Stick W21 (modem)"},	// also qmi but not yet in driver
	{0x1c9e, 0x9803, generic, 2, 1, 2, NULL, "SmartBro WM66E (modem)"},	// also qmi but not yet in driver
	{0x1c9e, 0x98ff, none, 0, 0, 2, &modeswitch_alcatel, "4G Systems XS Stick W21 (cdrom)"},	//
	{0x1c9e, 0x9900, generic, 1, 2, 2, NULL, "Softbank C02LC (modem)"},	//
	{0x1c9e, 0x9a00, generic, 2, 0, 2, NULL, "4G Systems XS Stick TV (modem)"},	//
	{0x1c9e, 0x9d00, generic, 2, 3, 2, &modeswitch_alcatel, "Prolink PCM100 (cdrom)"},	//
	{0x1c9e, 0x9e00, generic, 2, 0, 2, &modeswitch_alcatel, "MMX 310C"},	//
	{0x1c9e, 0xf000, none, 0, 0, 2, &modeswitch_alcatel, "4G Systems XS Stick W14 (cdrom)"},	//
	{0x1c9e, 0xf001, none, 0, 0, 2, &modeswitch_alcatel, "Alcatel AD110 (cdrom)"},	//

/* TechFaith */
	{0x1d09, 0x1000, none, 0, 0, 2, &modeswitch_std_eject, "Techfaith (cdrom)"},	//
	{0x1d09, 0x1010, generic, 2, 0, 2, NULL, "Aiko 81D (modem)"},	//
	{0x1d09, 0x1011, generic, 2, 0, 2, NULL, "Pretec H550 (modem)"},	//
	{0x1d09, 0x1021, none, 0, 0, 2, &modeswitch_std_eject, "Aiko 81D (cdrom)"},	//
	{0x1d09, 0x1025, none, 0, 0, 2, &modeswitch_std_eject, "TechFaith FlyingLARK46 (cdrom)"},	//
	{0x1d09, 0x1026, generic, 1, 2, 2, NULL, "TechFaith FlyingLARK46 (modem)"},	//
	{0x1d09, 0x4306, generic, 2, 0, 2, NULL, "TechFaith Venus VT18 (modem)"},	//

/* Teltonika */
	{0x1d12, 0xf00e, none, 0, 0, 2, &modeswitch_std_eject, "Teltonika UM5100 (cdrom)"},	//

/* Wisue */
	{0x1dbc, 0x0005, acm, 0, 0, 2, NULL, "Vodafone MD950 (modem)"},	//
	{0x1dbc, 0x0669, acm, 0, 0, 2, &select_config2, "Wisue W340 (modem)"},	//
	{0x1dbc, 0x8005, acm, 0, 0, 2, NULL, "EDGE Modem (modem)"},	//

/* Qualcomm /ALink /Hyundai */
	{0x1e0e, 0x9000, option, 1, 2, 3, NULL, "PROLink PHS100, Hyundai MB-810, A-Link 3GU (modem)"},	//
	{0x1e0e, 0x9001, option, 1, 2, 3, NULL, "Simcom SIM7100"},	//
	{0x1e0e, 0x9100, option, 1, 2, 3, NULL, "PROLink PHS300, A-Link 3GU (modem)"},	//
	{0x1e0e, 0x9200, option, 1, 2, 3, NULL, "PROLink PHS100, Hyundai MB-810, A-Link 3GU (modem)"},	//
	{0x1e0e, 0x9a00, generic, 1, 2, 3, NULL, "PROLink PEM330 (modem)"},	// also qmi but not yet in driver
	{0x1e0e, 0xce16, option, 1, 2, 3, NULL, "D-Link DWM-162U5, Micromax MMX 300c (modem)"},	//
	{0x1e0e, 0xce17, generic, 1, 0, 3, NULL, "D-Link DWM-162 C1 (modem)"},	//
	{0x1e0e, 0xce1e, option, 1, 2, 3, NULL, "D-Link DWM-162U5 A1 (modem)"},	//
	{0x1e0e, 0xce28, generic, 1, 2, 3, NULL, "SpeedUP SU-7000U (modem)"},	//
	{0x1e0e, 0xcefe, generic, 1, 2, 3, NULL, "Simcom EM600, Micromax MMX 300c (modem)"},	//
	{0x1e0e, 0xf000, none, 0, 0, 3, &modeswitch_others, "PROLink PHS100, Hyundai MB-810, A-Link 3GU (cdrom)"},	//

/* SelectWireless */
	{0x1edf, 0x6003, acm, 0, 0, 2, &select_config2, "AirPlus MCD-800"},	//
	{0x1edf, 0x6004, acm, 1, 0, 2, NULL, "AirPlus MCD-640/650"},	//

/* Onda */
	{0x1ee8, 0x0003, option, 0, 0, 2, &modeswitch_onda, "Onda MV815UP (cdrom)"},	//
	{0x1ee8, 0x0004, acm, 1, 0, 2, NULL, "Onda MV815UP (modem)"},	//
	{0x1ee8, 0x0009, option, 0, 0, 2, &modeswitch_onda, "Onda MW823UP (cdrom)"},	//
	{0x1ee8, 0x000b, acm, 1, 0, 2, NULL, "Onda MW823UP (modem)"},	//
	{0x1ee8, 0x0011, acm, 1, 0, 2, NULL, "Onda MDC835UP (modem)"},	//
	{0x1ee8, 0x0012, acm, 1, 0, 2, NULL, "Onda MW833UP (modem)"},	//
	{0x1ee8, 0x0013, none, 0, 0, 2, &modeswitch_onda, "Onda MW833UP/MT835UP (cdrom)"},	//
	{0x1ee8, 0x0014, acm, 1, 0, 2, NULL, "Onda MT835UP (modem)"},	//
	{0x1ee8, 0x0017, acm, 1, 0, 2, NULL, "Onda MO835UP (modem)"},	//
	{0x1ee8, 0x0018, none, 0, 0, 2, &modeswitch_onda, "Onda MO835UP (cdrom)"},	//
	{0x1ee8, 0x003e, acm, 1, 0, 2, NULL, "Onda MW836UP (modem)"},	//
	{0x1ee8, 0x0040, none, 0, 0, 2, &modeswitch_onda, "Onda MW836UP (cdrom)"},	//
	{0x1ee8, 0x0044, acm, 1, 0, 2, NULL, "Onda MDC655 (modem)"},	//
	{0x1ee8, 0x0045, none, 0, 0, 2, &modeswitch_onda, "Onda MDC655 (cdrom)"},	//
	{0x1ee8, 0x0049, acm, 1, 0, 2, NULL, "Onda MDC655 (modem)"},	//
	{0x1ee8, 0x004a, none, 0, 0, 2, &modeswitch_onda, "Onda MDC655 (cdrom)"},	//
	{0x1ee8, 0x004e, acm, 1, 0, 2, NULL, "Onda MDC655 (modem)"},	//
	{0x1ee8, 0x004f, none, 0, 0, 2, &modeswitch_onda, "Onda MDC655 (cdrom)"},	//
	{0x1ee8, 0x0053, acm, 1, 0, 2, NULL, "Onda MW875UP (modem)"},	//
	{0x1ee8, 0x0054, none, 0, 0, 2, &modeswitch_onda, "Onda MW875UP (cdrom)"},	//
	{0x1ee8, 0x005f, acm, 1, 0, 2, NULL, "Onda MSA 14.4 (modem)"},	//
	{0x1ee8, 0x0060, none, 0, 0, 2, &modeswitch_onda, "Onda MSA 14.4 (cdrom)"},	//
	{0x1ee8, 0x0063, none, 0, 0, 2, &modeswitch_onda, "Onda TM201 (cdrom)"},	//
	{0x1ee8, 0x0064, acm, 1, 0, 2, NULL, "Onda TM201 (modem)"},	//
	{0x1ee8, 0x0068, none, 0, 0, 2, &modeswitch_onda, "Onda WM301 (cdrom)"},	//
	{0x1ee8, 0x0069, acm, 1, 0, 2, NULL, "Onda WM301 (modem)"},	//

/* Franklin Wireless */
	{0x1fac, 0x0032, acm, 0, 0, 2, &select_config2, "Franklin U210"},	//
	{0x1fac, 0x0150, none, 0, 0, 2, &modeswitch_std_eject, "Franklin U600 (cdrom)"},	//
	{0x1fac, 0x0151, acm, 0, 0, 2, NULL, "Franklin U600 (modem)"},	//
//      {0x1fac, 0x0232, none, 0, 0, 0 | RNDIS, NULL, "Franklin U770 (modem)"},   //

/* Vertex Wireless */
	{0x1fe7, 0x0100, acm, 0, 0, 2, NULL, "Vertex VWM100 series (modem)"},	//

/* D-Link (2nd VID) */
	{0x2001, 0x00a6, none, 0, 0, 2, &modeswitch_std_eject, "D-Link DWM-157 B1 (cdrom)"},	// also cdc_mbim
	{0x2001, 0x00a7, none, 0, 0, 2, &modeswitch_std_eject, "D-Link DWM-157 C1 (cdrom)"},	// also cdc_mbim
	{0x2001, 0x7600, generic, 2, 1, 2, &select_config2, "D-Link DWM-157 B1 (cdrom)"},
	{0x2001, 0x7900, generic, 1, 3, 2, NULL, "D-Link DWM-157 A1 (modem)"},	//
	{0x2001, 0x7d00, generic, 1, 0, 2, NULL, "D-Link DWM-156 A6 (modem)"},	//
	{0x2001, 0x7d01, option, 1, 0, 2, NULL, "D-Link DWM-156 A7 (modem)"},	// also cdc_mbim
	{0x2001, 0x7d02, option, 1, 0, 2, NULL, "D-Link DWM-157 B1 (modem)"},	// also cdc_mbim
	{0x2001, 0x7d03, option, 1, 0, 2, NULL, "D-Link DWM-158 D1 (modem)"},	// also cdc_mbim
	{0x2001, 0x7d04, option, 1, 0, 2, NULL, "D-Link DWM-158 D1 (modem)"},	// also cdc_ether
	{0x2001, 0x7d0b, generic, 3, 2, 2, NULL, "D-Link DWM-156 A8 (modem)"},	// also cdc_mbim
	{0x2001, 0x7d0c, generic, 3, 2, 2, NULL, "D-Link DWP-157 B1 (modem)"},	// also cdc_mbim
	{0x2001, 0x7d0d, generic, 3, 2, 2, NULL, "D-Link DWM-167 A1 (modem)"},	// also cdc_mbim
	{0x2001, 0x7d0e, generic, 3, 2, 2, NULL, "D-Link DWM-157 C1 (modem)"},	// also cdc_mbim
	{0x2001, 0x7d10, generic, 3, 2, 2, NULL, "D-Link DWM-156 A8 (modem)"},	// also cdc_mbim
	{0x2001, 0x7e16, generic, 2, 1, 2, NULL, "D-Link DWM-221 A1 (modem)"},	// also qmi but not yet in driver
	{0x2001, 0x7e19, option, 2, 1, 2 | QMI, NULL, "D-Link DWM-221 B1 (modem)"},	//
	{0x2001, 0x7e35, generic, 2, 1, 2, NULL, "D-Link DWM-222 A1 (modem)"},	// also qmi but not yet in driver
//      {0x2001, 0x7e38, none, 0, 0, 2 | ETH, NULL, "D-Link DWR-910 (modem)"},    //
	{0x2001, 0x98ff, none, 0, 0, 2, &modeswitch_alcatel, "D-Link DWM-221 A1 (cdrom)"},	//
	{0x2001, 0xa401, none, 0, 0, 2, &modeswitch_std_eject, "D-Link DWM-221 B1 (cdrom)"},	//
	{0x2001, 0xa403, none, 0, 0, 2, &modeswitch_std_eject, "D-Link DWM-156 A8 (cdrom)"},	//
	{0x2001, 0xa405, none, 0, 0, 2, &modeswitch_std_eject, "D-Link DWM-167 A1 (cdrom)"},	//
	{0x2001, 0xa406, none, 0, 0, 2, &modeswitch_std_eject, "D-Link DWM-221 B1 (cdrom)"},	//
	{0x2001, 0xa407, none, 0, 0, 2, &modeswitch_std_eject, "D-Link DWM-157 C1 (cdrom)"},	//
	{0x2001, 0xa40a, none, 0, 0, 2, &modeswitch_mediatek, "D-Link DWM-156 A8 (cdrom)"},	//
	{0x2001, 0xa40d, none, 0, 0, 2, &modeswitch_std_eject, "D-Link DWR-910 (cdrom)"},	//
	{0x2001, 0xa706, none, 0, 0, 2, &modeswitch_std_eject, "D-Link DWM-156 A7 (cdrom)"},	//
	{0x2001, 0xa707, none, 0, 0, 2, &modeswitch_std_eject, "D-Link DWM-157 B1 (cdrom)"},	//
	{0x2001, 0xa708, none, 0, 0, 2, &modeswitch_std_eject, "D-Link DWM-158 D1 (cdrom)"},	//
	{0x2001, 0xa809, none, 0, 0, 2, &modeswitch_std_eject, "D-Link DWM-157 A1 (cdrom)"},	//
	{0x2001, 0xa80b, none, 0, 0, 2, &modeswitch_mediatek, "D-Link DWM-156 A6 (cdrom)"},	//
	{0x2001, 0xab00, none, 0, 0, 2, &modeswitch_mediatek, "D-Link DWM-222 A1 (cdrom)"},	//

/* Haier */
	{0x201e, 0x1022, generic, 0, 0, 2, NULL, "Haier CE862"},	//
	{0x201e, 0x1023, none, 0, 0, 2, &modeswitch_std_eject, "Haier CE682"},	//
	{0x201e, 0x10f8, option, 2, 3, 2, NULL, "Haier CE81B"},	//
	{0x201e, 0x2009, option, 2, 0, 2, &modeswitch_std_eject, "Haier CE100/CE210"},	//

/* VisionTek ?? */
	{0x2020, 0x0002, none, 0, 0, 2, &modeswitch_mediatek, "MicroMax MMX 377G (cdrom)"},	//
	{0x2020, 0x1005, generic, 1, 3, 2, NULL, "SpeedUp SU-8000U (modem)"},	//
	{0x2020, 0x1008, generic, 1, 3, 2, NULL, "SpeedUp SU-9300U (modem)"},	//
	{0x2020, 0x1012, generic, 1, 3, 2, NULL, "Prolink PHS100 (modem)"},	//
	{0x2020, 0x2000, generic, 1, 0, 2, NULL, "Beetel BG64 (modem)"},	//
	{0x2020, 0x4000, generic, 1, 0, 2, NULL, "Rostelecom Sense R41 (modem)"},	// also mbim
	{0x2020, 0x4002, generic, 1, 0, 2, NULL, "Rostelecom Sense R41 (modem)"},	// also mbim
	{0x2020, 0x4010, generic, 1, 0, 2, NULL, "MicroMax MMX 377G (modem)"},	// also mbim
	{0x2020, 0xf00e, none, 0, 0, 2, &modeswitch_std_eject, "SpeedUp SU-8000 (cdrom)"},	//
	{0x2020, 0xf00f, none, 0, 0, 2, &modeswitch_std_eject, "SpeedUp SU-8000U (cdrom)"},	//

/* ChangHong */
	{0x2077, 0x1000, none, 0, 0, 2, &modeswitch_std_eject, "Axesstel MV242 (cdrom)"},	//
	{0x2077, 0x7001, option, 1, 0, 2, NULL, "ChangHong CH690 (modem)"},	//
	{0x2077, 0x7010, generic, 1, 0, 2, NULL, "D-Link DWM-163 (modem)"},	//
	{0x2077, 0x7011, generic, 1, 0, 2, NULL, "D-Link DWM-168 (modem)"},	//
	{0x2077, 0x8000, generic, 1, 0, 2, NULL, "Axesstel MV242 (modem)"},	//
	{0x2077, 0x9062, generic, 1, 3, 2, NULL, "D-Link DWM-155 (modem)"},	//
	{0x2077, 0x9000, generic, 1, 2, 2, NULL, "Nucom W-160 (modem)"},	//
	{0x2077, 0xa000, generic, 1, 2, 2, NULL, "Nucom W-260 (modem)"},	//
	{0x2077, 0xa003, generic, 1, 2, 2, NULL, "Netgear AC327U (modem)"},	//
	{0x2077, 0xf000, none, 0, 0, 2, &modeswitch_std_eject, "ChangHong CH690 (cdrom)"},	//

/* Puchuang */
	{0x20a6, 0x1000, option, 0, 0, 2, NULL, "E003 (modem)"},	//
	{0x20a6, 0x1105, generic, 2, 0, 2, NULL, "Intex 3.5G (modem)"},	//
	{0x20a6, 0x1106, generic, 2, 0, 2, NULL, "Haier TE W130 (modem)"},	//
	{0x20a6, 0xf00a, none, 0, 0, 2, &modeswitch_std_eject, "E003 (cdrom)"},	//
	{0x20a6, 0xf00e, none, 0, 0, 2, &modeswitch_std_eject, "Intex 3.5G (cdrom)"},	//

/* Tlaytech */
	{0x20b9, 0x1682, none, 0, 0, 2, &modeswitch_std_eject, "TEU800"},	//

/* CELOT Corporation */
	{0x211f, 0x6801, option, 2, 0, 2, NULL, "Celot K-3000/CT-650/CT-680 (modem)"},	//

/* StrongRising */
	{0x21f5, 0x1000, none, 0, 0, 2, &modeswitch_std_eject, "StrongRising (cdrom)"},	//
	{0x21f5, 0x2008, generic, 3, 0, 2, NULL, "Flash SX0301 (modem)"},	//
	{0x21f5, 0x2012, generic, 3, 0, 2, NULL, "MU290 (modem)"},	//
	{0x21f5, 0x3010, none, 0, 0, 2, &modeswitch_std_eject, "STD808 (cdrom)"},	//

/* Linktop */
	{0x230d, 0x0001, acm, 0, 1, 0, &select_config2, "Linktop LW27x (BSNL 3G)"},	//
	{0x230d, 0x0003, acm, 0, 1, 0, &select_config2, "Linktop LW27x (Teracom 3G)"},	//
	{0x230d, 0x0007, acm, 0, 1, 0, &select_config2, "Linktop LW27x (Visiontek 3G)"},	//
	{0x230d, 0x000b, acm, 0, 1, 0, &select_config2, "Zoom 3G"},	//
	{0x230d, 0x000c, acm, 0, 1, 0, &select_config2, "Zoom 3G"},	//
	{0x230d, 0x000d, acm, 0, 1, 0, &select_config2, "Intex Speed 3G v7.2"},	//
	{0x230d, 0x0101, acm, 0, 1, 0, &select_config2, "Linktop LW27x (BSNL 3G)"},	//
	{0x230d, 0x0103, acm, 0, 1, 0, &select_config2, "Linktop LW27x (Teracom 3G)"},	//

/* TP-Link */
	{0x2357, 0x0200, none, 0, 0, 2, &modeswitch_std_eject, "TP-Link MA180 (cdrom)"},	//
	{0x2357, 0x0201, option, 1, 2, 2 | QMI, NULL, "TP-Link MA180 (modem)"},	//
	{0x2357, 0x0202, option, 1, 2, 2, NULL, "TP-Link MA180 (modem)"},	// also qmi but not yet in driver
	{0x2357, 0x0203, option, 1, 2, 2, NULL, "TP-Link MA180 (modem)"},	//
	{0x2357, 0x9000, option, 1, 2, 2 | QMI, NULL, "TP-Link MA260 (modem)"},	//
	{0x2357, 0xf000, none, 0, 0, 2, &modeswitch_std_eject, "TP-Link MA260 (cdrom)"},	//

/* Unknown mfgr */
	{0x23a2, 0x1010, none, 0, 0, 2, &modeswitch_std_eject, "Titan 3.5G (cdrom)"},	//
	{0x23a2, 0x1234, acm, 0, 0, 0, NULL, "Titan 3.5G"},	//

/* Dell */
	{0x413c, 0x8114, option, 1, 0, 2, NULL, "Dell 5700"},	//
	{0x413c, 0x8115, option, 1, 0, 2, NULL, "Dell 5500"},	//
	{0x413c, 0x8116, option, 1, 0, 2, NULL, "Dell 5505"},	//
	{0x413c, 0x8117, option, 1, 0, 2, NULL, "Dell 5700"},	//
	{0x413c, 0x8118, option, 1, 0, 2, NULL, "Dell 5510"},	//
	{0x413c, 0x8128, option, 1, 0, 2, NULL, "Dell 5700"},	//
	{0x413c, 0x8129, option, 1, 0, 2, NULL, "Dell 5700"},	//
	{0x413c, 0x8133, option, 1, 0, 2, NULL, "Dell 5720"},	//
	{0x413c, 0x8134, option, 1, 0, 2, NULL, "Dell 5720"},	//
	{0x413c, 0x8135, option, 1, 0, 2, NULL, "Dell 5720"},	//
	{0x413c, 0x8136, option, 1, 0, 2, NULL, "Dell 5520"},	//
	{0x413c, 0x8137, option, 1, 0, 2, NULL, "Dell 5520"},	//
	{0x413c, 0x8138, option, 1, 0, 2, NULL, "Dell 5520"},	//
	{0x413c, 0x8147, acm, 0, 1, 2, NULL, "Dell 5530"},	//
	{0x413c, 0x8180, option, 1, 0, 2, NULL, "Dell 5730"},	//
	{0x413c, 0x8181, option, 1, 0, 2, NULL, "Dell 5730"},	//
	{0x413c, 0x8182, option, 1, 0, 2, NULL, "Dell 5730"},	//
	{0x413c, 0x8183, acm, 0, 1, 2, NULL, "Dell 5530"},	//
	{0x413c, 0x8184, acm, 0, 1, 2, NULL, "Dell 5540"},	//
	{0x413c, 0x8186, qcserial, 1, 0, 2 | QMI, NULL, "Dell 5620"},	//
	{0x413c, 0x818b, acm, 0, 1, 2, NULL, "Dell 5541"},	//
	{0x413c, 0x818c, acm, 0, 1, 2, NULL, "Dell 5542"},	//
	{0x413c, 0x818d, acm, 0, 1, 2, NULL, "Dell 5550"},	//
	{0x413c, 0x818e, acm, 0, 1, 2, NULL, "Dell 5560"},	//
	{0x413c, 0x8194, qcserial, 1, 0, 2 | QMI, NULL, "Dell 5630"},	//
	{0x413c, 0x8195, option, 1, 0, 2 | QMI, NULL, "Dell 5800"},	//
	{0x413c, 0x8196, option, 1, 0, 2 | QMI, NULL, "Dell 5800v2"},	//
	{0x413c, 0x819b, option, 1, 0, 2 | QMI, NULL, "Dell 5804"},	//
	{0x413c, 0x81a2, qcserial, 1, 2, 2 | QMI, &select_config1, "Dell 5806"},	//  cdc_mbim in default config2
	{0x413c, 0x81a3, qcserial, 1, 2, 2 | QMI, &select_config1, "Dell 5570"},	//  cdc_mbim in default config2
	{0x413c, 0x81a4, qcserial, 1, 2, 2 | QMI, &select_config1, "Dell 5570e"},	//  cdc_mbim in default config2
	{0x413c, 0x81a8, qcserial, 1, 2, 2 | QMI, NULL, "Dell 5808"},	// also cdc_mbim
	{0x413c, 0x81a9, qcserial, 1, 2, 2 | QMI, NULL, "Dell 5808e"},	// also cdc_mbim
	{0x413c, 0x81b1, qcserial, 1, 2, 2 | QMI, &select_config1, "Dell 5809e"},	//  cdc_mbim in default config2
	{0x413c, 0x81b3, qcserial, 1, 2, 2 | QMI, &select_config1, "Dell 5809e"},	//  cdc_mbim in default config2

	{0xffff, 0xffff, none, 0, 0, 0, NULL, NULL}	//
};

char *get3GDeviceVendor(void)
{
	char *vendor = "unknown";
	int devicecount = 0;
	while (devicelist[devicecount].vendor != 0xffff) {
		if (scanFor(devicelist[devicecount].vendor, devicelist[devicecount].product)) {
			return devicelist[devicecount].name;

		}
		devicecount++;
	}
	return vendor;
}

char *get3GControlDevice(void)
{
	static char control[32];
	static char data[32];
#if defined(ARCH_broadcom) && !defined(HAVE_BCMMODERN)
	mkdir("/tmp/usb", 0700);
	eval("mount", "-t", "usbfs", "usb", "/tmp/usb");
//insmod("sierra");  //further investigation required (compass problem)
#endif
	int needreset = 1;
	char *ttsdevice = "/dev/usb/tts/0";
#ifdef HAVE_CAMBRIA
	int gpio1, gpio2;
	int select = nvram_geti("wan_select");
	switch (select) {
	case 1:
		gpio1 = 1;
		gpio2 = 0;
		break;
	case 2:
		gpio1 = 0;
		gpio2 = 1;
		break;
	case 3:
		gpio1 = 1;
		gpio2 = 1;
		break;
	default:
		gpio1 = 1;
		gpio2 = 0;
		break;
	}

	if (gpio1 == nvram_geti("gpio26")
	    && gpio2 == nvram_geti("gpio27"))
		needreset = 0;

	if (gpio1) {
		nvram_seti("gpio26", 1);
		set_gpio(26, 1);
	} else {
		nvram_seti("gpio26", 0);
		set_gpio(26, 0);
	}
	if (gpio2) {
		nvram_seti("gpio27", 1);
		set_gpio(27, 1);
	} else {
		nvram_seti("gpio27", 0);
		set_gpio(27, 0);
	}
#endif
#ifdef HAVE_ERC
	needreset = 0;
#endif
	nvram_unset("3gnmvariant");
//      nvram_set("3gdata", "/dev/usb/tts/0");  // crap

	int devicecount = 0;
	while (devicelist[devicecount].vendor != 0xffff) {
		if (scanFor(devicelist[devicecount].vendor, devicelist[devicecount].product)) {
			fprintf(stderr, "%s detected\n", devicelist[devicecount].name);

#if defined(HAVE_LIBQMI) || defined(HAVE_UQMI)
			switch ((devicelist[devicecount].modeswitch & 0xf0)) {
			case QMI:
			case QMIRAW:
				insmod("cdc-wdm usbnet qmi_wwan");
				//start custom setup, if defined
				if (devicelist[devicecount].customsetup) {
					fprintf(stderr, "customsetup QMI\n");
					devicelist[devicecount].customsetup(needreset, devicecount);
					sleep(2);
				}
			}

			switch ((devicelist[devicecount].modeswitch & 0xf0)) {
			case QMI:
				sprintf(control, "qmi");
				nvram_set("3gdata", "qmi");
				return control;
				break;
			case QMIRAW:
				sprintf(control, "qmiraw");
				nvram_set("3gdata", "qmiraw");
				return control;
				break;
			}
#endif
			if (devicelist[devicecount].modeswitch & 0x0f) {
				char variant[32];
				sprintf(variant, "%d", devicelist[devicecount].modeswitch & 0x0f);
				nvram_set("3gnmvariant", variant);
			}
			// start custom setup, if defined
			if (devicelist[devicecount].customsetup) {
				fprintf(stderr, "customsetup\n");
				devicelist[devicecount].customsetup(needreset, devicecount);
				sleep(2);
			}
			// handle all serial protocols below here
			insmod("usbserial usb_wwan");	// used by all serial drivers

			switch (devicelist[devicecount].driver) {
			case generic:
				insmod("option");
				sysprintf("echo %04x %04x > /sys/bus/usb-serial/drivers/option1/new_id", devicelist[devicecount].vendor, devicelist[devicecount].product);
				sprintf(control, "/dev/usb/tts/%d", devicelist[devicecount].controldevice);
				sprintf(data, "/dev/usb/tts/%d", devicelist[devicecount].datadevice);
				eval("comgt", "-d", control, "-s", "/etc/comgt/wakeup.comgt");
				break;
			case option:
			case qcserial:
			case sierra:
				insmod("option qcserial sierra");
				sprintf(control, "/dev/usb/tts/%d", devicelist[devicecount].controldevice);
				sprintf(data, "/dev/usb/tts/%d", devicelist[devicecount].datadevice);
				eval("comgt", "-d", control, "-s", "/etc/comgt/wakeup.comgt");
				break;
			case acm:
				insmod("cdc-acm");
				sprintf(control, "/dev/ttyACM%d", devicelist[devicecount].controldevice);
				sprintf(data, "/dev/ttyACM%d", devicelist[devicecount].datadevice);
				eval("comgt", "-d", control, "-s", "/etc/comgt/wakeup.comgt");
				break;
			case hso:
				insmod("hso");
				sprintf(control, "hso");
				sprintf(data, "hso");
				FILE *out = fopen("/tmp/conninfo.ini", "wb");
				fprintf(out, "APN=%s\n", nvram_safe_get("wan_apn"));
				fprintf(out, "USER=%s\n", nvram_safe_get("ppp_username"));
				fprintf(out, "PASS=%s\n", nvram_safe_get("ppp_passwd"));
				fprintf(out, "PIN=%s\n", nvram_safe_get("wan_pin"));
				fclose(out);
				eval("/etc/hso/hso_connect.sh", "restart");
				break;
			case none:
				return control;
				break;
			default:
				sprintf(control, "/dev/usb/tts/%d", devicelist[devicecount].controldevice);
				eval("comgt", "-d", control, "-s", "/etc/comgt/wakeup.comgt");
				break;
			}
			nvram_set("3gdata", data);
			return control;
		}
		devicecount++;
	}
	//not found, use generic implementation (all drivers)
	insmod("cdc-acm cdc-wdm usbnet qmi_wwan usbserial usb_wwan sierra option qcserial");
	return ttsdevice;
}

/*
//future
typedef struct {
char *devicename;
int vendorid;
int productid;
char *drivers;
char *3gdata;
char *controldevice;
int iconswitch;
}3GDEVICE;
*/
