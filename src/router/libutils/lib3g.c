/*
 * lib3g.c
 *
 * Copyright (C) 2009 Sebastian Gottschall <gottschall@dd-wrt.com>
 *  			2012 - 2014 LOM
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

struct DEVICES {
	int vendor;
	int product;
	char *driver;
	char *controldevice;
	char *datadevice;
	int modeswitch;
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
			eval("umount /tmp/usb");
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

void checkreset(char *tty)
{
	char tts[32];
	sprintf(tts, "/dev/usb/tts/%s", tty);
	eval("comgt", "-d", tts, "-s", "/etc/comgt/reset.comgt");
	FILE *check = NULL;
	int count = 0;
	sleep(1);
	while (!(check = fopen(tty, "rb")) && count < 10) {
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
	fprintf(out, "MessageContent=\"55534243785634120100000080000601000000000000000000000000000000\"\n");
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

static void modeswitch_linktop(int needreset, int devicecount)
{
	FILE *out;
	out = fopen("/tmp/usb_modeswitch.conf", "wb");
	fprintf(out, "DefaultVendor=0x%04x\n", devicelist[devicecount].vendor);
	fprintf(out, "DefaultProduct=0x%04x\n", devicelist[devicecount].product);
	fprintf(out, "Configuration=3\n");
	fclose(out);
	eval("usb_modeswitch", "-c", "/tmp/usb_modeswitch.conf");
	eval("comgt", "-d", "/dev/ttyACM0", "-s", "/etc/comgt/wakeup.comgt");
	sleep(2);
}

static void modeswitch_others(int needreset, int devicecount)
{
	eval("usb_modeswitch", "-v", "0x04fc", "-p", "0x2140", "-M", "55534243123456782400000080000612000024000000000000000000000000");
	eval("usb_modeswitch", "-v", "0x10a9", "-p", "0x6080", "-F");
	eval("usb_modeswitch", "-v", "0x19d2", "-p", "0x0166", "-M", "55534243123456782400000080000685000000240000000000000000000000");
	eval("usb_modeswitch", "-v", "0x19d2", "-p", "0x0266", "-M", "55534243123456782400000080000685000000240000000000000000000000");
	eval("usb_modeswitch", "-v", "0x19d2", "-p", "0xfff5", "-M", "5553424312345678c00000008000069f010000000000000000000000000000");
	eval("usb_modeswitch", "-v", "0x19d2", "-p", "0xfff6", "-M", "5553424312345678c00000008000069f010000000000000000000000000000");
	eval("usb_modeswitch", "-v", "0x1e0e", "-p", "0xf000", "-M", "555342431234567800000000000006bd000000020000000000000000000000");
	eval("usb_modeswitch", "-v", "0x2001", "-p", "0xa80b", "-M", "555342431234567800000000000006f0010300000000000000000000000000");
	eval("usb_modeswitch", "-v", "0x2020", "-p", "0x0002", "-M", "5553424312345678c0000000800003f0010100000000000000000000000000");
}

#define H_NCM 0x1000		// usbnet, cdc_ncm, huawei_cdc_ncm, cdc_wdm
#define NCM 0x800		// usbnet, cdc_ncm, cdc_wdm
#define MBIM 0x400		// usbnet, cdc_ncm, cdc_mbim,
#define RNDIS 0x200		// usbnet, cdc_ether, rndis_host,
#define S_NET 0x100		// usbnet, sierra_net (direct ip)
#define QMI 0x80		// usbnet, qmi_wwan, cdc_wdm
#define ETH 0x40		// usbnet, cdc_ether
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

// Quanta
	{0x0408, 0xea02, "option", "2", "0", 0, NULL, "Quanta MUQ-101 (modem)"},	//
	{0x0408, 0xea03, "option", "2", "0", 0, NULL, "Quanta MUQ-110 (modem)"},	//
	{0x0408, 0xea04, "option", "2", "0", 0, NULL, "Quanta GLX (modem)"},	//
	{0x0408, 0xea05, "option", "2", "0", 0, NULL, "Quanta GKE (modem)"},	//
	{0x0408, 0xea06, "option", "2", "0", 0, NULL, "Quanta GLE (modem)"},	//
	{0x0408, 0xea16, "option", "1", "2", 0 | GENERIC, NULL, "Quanta 1KR (modem)"},	// also qmi but not yet in driver
	{0x0408, 0xea17, "option", "0", "0", 2, &modeswitch_std_eject, "Quanta 1KR (cdrom)"},	//
	{0x0408, 0xea25, "option", "0", "0", 2, &modeswitch_quanta, "Quanta 1K3 LTE (cdrom)"},	//
	{0x0408, 0xea26, "option", "1", "2", 2 | GENERIC, NULL, "Quanta Mobility LTE (modem)"},	// also qmi but not yet in driver
	{0x0408, 0xea42, "option", "3", "2", 0 | QMI, NULL, "Megafone M100-1 (modem)"},	//
	{0x0408, 0xea43, "option", "0", "0", 2, &modeswitch_std_eject, "Quanta MobileGenie (cdrom)"},	//
//      {0x0408, 0xea45, NULL, NULL, NULL, 0 | ETH, NULL, "Quanta 1K6E (modem)"},       //
	{0x0408, 0xea47, "option", "3", "2", 2 | GENERIC, NULL, "Quanta MobileGenie (modem)"},	// also qmi but not yet in driver
	{0x0408, 0xf000, "option", "0", "0", 2, &modeswitch_quanta, "Quanta 1QDL (cdrom)"},	//
	{0x0408, 0xf001, "option", "0", "0", 2, &modeswitch_quanta, "Quanta GLX, GLE,GKE (cdrom)"},	//

// Nokia Mobile Phones
	{0x0421, 0x03a7, "option", "0", "0", 2 | ACM, NULL, "Nokia C5-00 Mobile phone (modem)"},	//
	{0x0421, 0x060c, "option", "0", "0", 2, &modeswitch_std_eject, "Nokia CS-10 (cdrom)"},	//
	{0x0421, 0x060d, "option", "0", "0", 2 | ACM, NULL, "Nokia CS-10 (modem)"},	//
	{0x0421, 0x060e, "option", "0", "0", 2 | ACM, NULL, "Nokia CS-10 (modem)"},	//
	{0x0421, 0x0610, "option", "0", "0", 2, &modeswitch_std_eject, "Nokia CS-15 (cdrom)"},	//
	{0x0421, 0x0612, "option", "0", "0", 2 | ACM, NULL, "Nokia CS-15/CS-18 (modem)"},	//
	{0x0421, 0x0618, "option", "0", "0", 2, &modeswitch_std_eject, "Nokia CS-12 (cdrom)"},	//
	{0x0421, 0x0619, "option", "0", "0", 2 | ACM, NULL, "Nokia CS-12 (modem)"},	//
	{0x0421, 0x061d, "option", "0", "0", 2, &modeswitch_std_eject, "Nokia CS-11 (cdrom)"},	//
	{0x0421, 0x061e, "option", "0", "0", 2 | ACM, NULL, "Nokia CS-11 (modem)"},	//
	{0x0421, 0x0622, "option", "0", "0", 2, &modeswitch_std_eject, "Nokia CS-17 (cdrom)"},	//
	{0x0421, 0x0623, "option", "0", "0", 2 | ACM, NULL, "Nokia CS-17 (modem)"},	//
	{0x0421, 0x0627, "option", "0", "0", 2, &modeswitch_std_eject, "Nokia CS-18 (cdrom)"},	//
	{0x0421, 0x0629, "option", "0", "0", 2 | ACM, NULL, "Nokia CS-18 (modem)"},	//
	{0x0421, 0x062c, "option", "0", "0", 2, &modeswitch_std_eject, "Nokia CS-19 (cdrom)"},	//
	{0x0421, 0x062d, "option", "0", "0", 2 | ACM, NULL, "Nokia CS-19 (modem)"},	//
	{0x0421, 0x062f, "option", "0", "0", 2 | ACM, NULL, "Nokia CS-19 (modem)"},	//
	{0x0421, 0x0637, "option", "0", "0", 2, &modeswitch_std_eject, "Nokia 21M-02 (cdrom)"},	//
	{0x0421, 0x0638, "option", "0", "0", 2 | ACM, NULL, "Nokia 21M-02 (modem)"},	//
	{0x0421, 0x0639, "option", "1", "0", 2 | ACM, NULL, "Nokia 21M-02 (modem)"},	//

// Philips/NXP
	{0x0471, 0x1206, "option", "0", "0", 2 | ACM, &select_config2, "Philips TalkTalk (modem)"},	//
	{0x0471, 0x1210, "option", "0", "0", 2, &modeswitch_std_eject, "Vodaphone MD950 (cdrom)"},	//
	{0x0471, 0x1237, "option", "0", "0", 2 | ACM, &select_config2, "Philips TalkTalk (modem)"},	//

// Kyocera
	{0x0482, 0x024d, "option", "0", "0", 2 | ACM, &select_config2, "Kyocera WK06"},	//

// ST-Ericsson
	{0x04cc, 0x2251, "option", "0", "0", 2, &modeswitch_std_eject, "Alcatel-Lucent"},	//
	{0x04cc, 0x2259, "option", "0", "0", 2 | ACM, &select_config2, "Alcatel-Lucent T920S"},	//
	{0x04cc, 0x225c, "option", "0", "0", 2 | ACM, &select_config2, "Alcatel-Lucent T930S"},	//
	{0x04cc, 0x226e, "option", "0", "0", 2 | ACM, &select_config2, "Nexperia TM TD-SCDMA"},	//

// Sunplus
	{0x04fc, 0x0615, "option", "0", "0", 2 | ACM, NULL, "SU-3200U (modem)"},	//
	{0x04fc, 0x1240, "option", "0", "0", 2 | ACM, NULL, "Mobidata MDB-100EU (modem)"},	//
	{0x04fc, 0x2140, "option", "0", "0", 2, &modeswitch_others, "SU-3200U (cdrom)"},	//

// Qualcomm
	{0x05c6, 0x0010, "option", "0", "0", 0, &modeswitch_std_eject, "Generic Qualcomm (cdrom)"},	//
	{0x05c6, 0x0016, "option", "2", "2", 0 | GENERIC, NULL, "iBall 3.5G Connect (modem)"},	//
	{0x05c6, 0x0018, "option", "1", "3", 0 | GENERIC, NULL, "Advan DT9 SQ (modem)"},	//
	{0x05c6, 0x0023, "option", "2", "0", 0, NULL, "Leoxsys LN-72V (modem)"},	//
	{0x05c6, 0x00a0, "option", "2", "0", 0 | GENERIC, NULL, "Axesstel MV241 (modem)"},	//
	{0x05c6, 0x1000, "option", "0", "0", 0, &modeswitch_std_eject, "Generic Qualcomm (cdrom)"},	//
	{0x05c6, 0x2001, "option", "0", "0", 0, &modeswitch_std_eject, "Generic Qualcomm (cdrom)"},	//
	{0x05c6, 0x3100, "option", "0", "0", 0 | ACM, NULL, "Maxon MM-5100 (modem)"},	//
	{0x05c6, 0x3196, "option", "0", "0", 0 | ACM, NULL, "Maxon MM-5500 (modem)"},	//
	{0x05c6, 0x6000, "option", "2", "0", 0 | GENERIC, NULL, "Siemens SG75 (modem)"},	//
	{0x05c6, 0x6500, "option", "2", "0", 0 | GENERIC, NULL, "Venus VT-80n (modem)"},	//
	{0x05c6, 0x6503, "option", "0", "0", 0, &modeswitch_std_eject, "Generic Qualcomm (cdrom)"},	//
	{0x05c6, 0x9000, "option", "1", "2", 0 | QMI, NULL, "SIMCom SIM5218 (modem)"},	//
        {0x05c6, 0x9011, "qcserial", "1", "2", 0 | QMI, NULL, "Qualcomm HS-USB (modem)"},       //
        {0x05c6, 0x9024, "qcserial", "0", "0", 0, &modeswitch_std_eject, "ASB TL131 TD-LTE (cdrom)"},   //
	{0x05c6, 0x9025, "option", "2", "0", 0 | QMI, NULL, "ASB TL131 TD-LTE (modem)"},	//
        {0x05c6, 0x9046, "qcserial", "1", "2", 0 | QMI, NULL, "Qualcomm HS-USB (modem)"},       //
	{0x05c6, 0xf000, "option", "0", "0", 0, &modeswitch_std_eject, "Generic Qualcomm (cdrom)"},	//

// D-Link
	{0x07d1, 0x3e01, "option", "1", "0", 2, NULL, "D-Link DWM-152 C1 (modem)"},	//
	{0x07d1, 0x3e02, "option", "1", "0", 2, NULL, "D-Link DWM-156 A1 (modem)"},	//
	{0x07d1, 0x7e07, "option", "3", "3", 2 | GENERIC, NULL, "D-Link DWM-151 A1 (modem)"},	//
	{0x07d1, 0x7e0c, "option", "2", "0", 2 | GENERIC, NULL, "D-Link DWM-156 A2 (modem)"},	//
	{0x07d1, 0x7e11, "option", "1", "2", 2 | GENERIC, NULL, "D-Link DWM-156 A3 (modem)"},	//
	{0x07d1, 0xa800, "option", "0", "0", 0, &modeswitch_std_eject, "D-Link DWM-152 C1/DWM-156 A1 (cdrom)"},	//
	{0x07d1, 0xa804, "option", "0", "0", 0, &modeswitch_std_eject, "D-Link DWM-156 A3 (cdrom)"},	//
	{0x07d1, 0xf000, "option", "0", "0", 0, &modeswitch_std_eject, "D-Link DWM-151 A1 (cdrom)"},	//

// Netgear
	{0x0846, 0x68a2, "sierra", "2", "0", 1 | QMI, NULL, "Sierra MC7710 (modem)"},	//

// Toshiba
	{0x0930, 0x0d45, "option", "2", "0", 2, NULL, "Toshiba G450 (modem)"},	//
	{0x0930, 0x0d46, "option", "0", "0", 0, &modeswitch_std_eject, "Toshiba G450 (cdrom)"},	//

// Option
	{0x0af0, 0x4005, "option", "2", "1", 0 | QMI, NULL, "Option GIO711"},	//
	{0x0af0, 0x4007, NULL, "0", "0", 0, &modeswitch_sierra, "Option GIO711 (cdrom)"},	//
//      {0x0af0, 0x6711, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GE201"},    //express card
//      {0x0af0, 0x6731, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GE"},       //express card
//      {0x0af0, 0x6751, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GE"},       //express card
//      {0x0af0, 0x6771, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GE"},       //express card
//      {0x0af0, 0x6791, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GE"},       //express card
	{0x0af0, 0x6901, "option", "1", "0", 0, NULL, "Option GI0201"},	//usb
	{0x0af0, 0x6911, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GI0225"},	//usb
	{0x0af0, 0x6951, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GI0225"},	//usb
	{0x0af0, 0x6971, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GI0225"},	//usb
//      {0x0af0, 0x7011, NULL, "hso", "hso", 2, &modeswitch_rezero, "Option GE301"},    //express card
//      {0x0af0, 0x7031, NULL, "hso", "hso", 2, &modeswitch_rezero, "Option GE301"},    //express card
//      {0x0af0, 0x7051, NULL, "hso", "hso", 2, &modeswitch_rezero, "Option GE301"},    //express card
//      {0x0af0, 0x7071, NULL, "hso", "hso", 2, &modeswitch_rezero, "Option GE301"},    //express card
	{0x0af0, 0x7111, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GTM"},	//pcie minicard
	{0x0af0, 0x7201, "option", "1", "0", 0, NULL, "Option GTM380"},	//pcie minicard
	{0x0af0, 0x7211, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GTM380"},	//pcie minicard
	{0x0af0, 0x7251, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GTM380"},	//pcie minicard
	{0x0af0, 0x7271, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GTM380"},	//pcie minicard
//      {0x0af0, 0x7301, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GE040x"},   //express card
	{0x0af0, 0x7311, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GTM040x"},	//pcie minicard
//      {0x0af0, 0x7361, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GE044x"},   //express card
//      {0x0af0, 0x7381, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GE044x"},   //express card
	{0x0af0, 0x7401, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GI0401"},	//usb
	{0x0af0, 0x7501, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GI0431"},	//usb
//      {0x0af0, 0x7601, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GE040x"},   //express card
	{0x0af0, 0x7701, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GI0451"},	//usb
	{0x0af0, 0x7706, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GI0451"},	//usb
	{0x0af0, 0x7801, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GI04xx"},	//usb
	{0x0af0, 0x7901, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GI0452"},	//usb
	{0x0af0, 0x7a01, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GI0461"},	//usb
	{0x0af0, 0x7a05, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GI0461"},	//usb
	{0x0af0, 0x8001, NULL, NULL, NULL, 0, &modeswitch_rezero, "Option GI1515"},	//zero footprint install id
	{0x0af0, 0x8002, NULL, NULL, NULL, 0, &modeswitch_rezero, "Option GI1515"},	//zero footprint install id
	{0x0af0, 0x8003, NULL, NULL, NULL, 0, &modeswitch_rezero, "Option GI1515"},	//zero footprint install id
	{0x0af0, 0x8120, "option", "2", "2", 3 | QMI, NULL, "Option GTM681W"},	//pcie minicard
	{0x0af0, 0x8200, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GI07xx"},	//usb
	{0x0af0, 0x8201, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GI07xx"},	//usb
	{0x0af0, 0x8204, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GI07xx"},	//usb
	{0x0af0, 0x8300, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GI033x"},	//usb
	{0x0af0, 0x8302, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GI033x"},	//usb
	{0x0af0, 0x8304, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GI033x"},	//usb
//      {0x0af0, 0x8400, NULL, "hso", "hso", 0, &modeswitch_rezero, "Pioner JP1"},      //
	{0x0af0, 0x8600, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GI063x"},	//usb
	{0x0af0, 0x8700, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GI0643"},	//usb
//      {0x0af0, 0x8701, NULL, NULL, NULL, 3 | ETH, NULL, "Option GI0643"},     //usb
	{0x0af0, 0x8800, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GTM60x"},	//pcie minicard
	{0x0af0, 0x8900, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GTM67x"},	//pcie minicard
	{0x0af0, 0x9000, NULL, "hso", "hso", 3, &modeswitch_rezero, "Option GTM66x"},	//pcie minicard
	{0x0af0, 0x9200, NULL, "hso", "hso", 0, NULL, "Option GTM671WFS"},	//pcie minicard
	{0x0af0, 0xc031, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GI031"},	//usb
	{0x0af0, 0xc100, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GI070x"},	//usb
	{0x0af0, 0xd001, NULL, NULL, NULL, 0, &modeswitch_rezero, "Option GI1515"},	//zero footprint install id
	{0x0af0, 0xd031, NULL, "hso", "hso", 0, &modeswitch_rezero, "Qualcomm ICON 321"},	//usb
	{0x0af0, 0xd033, NULL, "hso", "hso", 0, &modeswitch_rezero, "Qualcomm ICON 322"},	//usb
	{0x0af0, 0xd055, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GI0505"},	//usb
	{0x0af0, 0xd057, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GI1505"},	//usb
	{0x0af0, 0xd058, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GI1509"},	//usb
	{0x0af0, 0xd155, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GI0505"},	//usb
	{0x0af0, 0xd157, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GI1515"},	//usb
	{0x0af0, 0xd255, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GI1215"},	//usb
	{0x0af0, 0xd257, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GI1215"},	//usb
	{0x0af0, 0xd357, NULL, "hso", "hso", 0, &modeswitch_rezero, "Option GI1505"},	//usb

// Olivetti
	{0x0b3c, 0xc000, "option", "0", "0", 2 | QMI, NULL, "Olivetti Olicard 100"},	//
	{0x0b3c, 0xc001, "option", "0", "0", 2 | QMI, NULL, "Olivetti Olicard 120"},	//
	{0x0b3c, 0xc002, "option", "0", "0", 2 | QMI, NULL, "Olivetti Olicard 140"},	//
	{0x0b3c, 0xc003, "option", "0", "4", 2, NULL, "Olivetti Olicard 145"},	//
	{0x0b3c, 0xc004, "option", "0", "4", 2 | QMI, NULL, "Olivetti Olicard 155"},	//
	{0x0b3c, 0xc005, "option", "0", "0", 2 | QMI, NULL, "Olivetti Olicard 200"},	//
	{0x0b3c, 0xc00a, "option", "0", "4", 2 | QMI, NULL, "Olivetti Olicard 160"},	//
	{0x0b3c, 0xc00b, "option", "0", "2", 2 | QMI, NULL, "Olivetti Olicard 500"},	//
	{0x0b3c, 0xc700, "option", "0", "0", 0, &modeswitch_std_eject, "Olivetti Olicard 100 (cdrom)"},	//
	{0x0b3c, 0xf000, "option", "0", "0", 0, &modeswitch_alcatel, "Olivetti Olicards (cdrom)"},	//
	{0x0b3c, 0xf00c, "option", "0", "0", 0, &modeswitch_alcatel, "Olivetti Olicards (cdrom)"},	//
	{0x0b3c, 0xf017, "option", "0", "0", 0, &modeswitch_std_eject, "Olivetti Olicard 500 (cdrom)"},	//

// Ericsson Business Mobile Networks
	{0x0bdb, 0x1900, "option", "4", "1", 0 | ACM, NULL, "Ericsson F3507g"},	//
	{0x0bdb, 0x1902, "option", "4", "1", 0 | ACM, NULL, "Lenovo F3507g"},	//
	{0x0bdb, 0x1904, "option", "4", "1", 0 | ACM, NULL, "Ericsson F3607gw"},	//
	{0x0bdb, 0x1905, "option", "4", "1", 0 | ACM, NULL, "Ericsson F3607gw"},	//
	{0x0bdb, 0x1906, "option", "4", "1", 0 | ACM, NULL, "Ericsson F3607gw"},	//
	{0x0bdb, 0x1907, "option", "4", "1", 0 | ACM, NULL, "Ericsson F3607gw"},	//
	{0x0bdb, 0x1909, "option", "4", "1", 0 | ACM, NULL, "Ericsson F3307"},	//
	{0x0bdb, 0x190a, "option", "4", "1", 0 | ACM, NULL, "Ericsson F3307"},	//
	{0x0bdb, 0x190d, "option", "4", "1", 0 | ACM, NULL, "Ericsson F5521gw"},	// also cdc_ncm
	{0x0bdb, 0x190e, "option", "4", "1", 0 | ACM, NULL, "Ericsson F3307"},	//
	{0x0bdb, 0x190f, "option", "4", "1", 0 | ACM, NULL, "Ericsson F3307"},	//
	{0x0bdb, 0x1910, "option", "4", "1", 0 | ACM, NULL, "Ericsson F5521gw"},	// also cdc_ncm

// Kyocera
	{0x0c88, 0x17da, "option", "0", "0", 0, NULL, "Kyocera KPC650"},	//
	{0x0c88, 0x180a, "option", "0", "0", 0, NULL, "Kyocera KPC680"},	//

// AirPrime (Sierra)
	{0x0f3d, 0x68a2, "sierra", "2", "0", 1 | QMI, NULL, "Sierra MC7700 (modem)"},	// also cdc_mbim
	{0x0f3d, 0x68a3, "sierra", "3", "3", 1, &reset_mc, "Sierra Wireless MC8700/Compass Direct IP"},	//
	{0x0f3d, 0x68aa, "sierra", "3", "3", 1, NULL, "Sierra Wireless AC313U/320U/330U Direct IP"},	//

// Sony Ericsson
	{0x0fce, 0xd0cf, "option", "0", "0", 0 | ACM, &select_config2, "Sony Ericsson MD300"},	//

// LG Electronics
	{0x1004, 0x6124, "option", "0", "2", 0 | ACM, NULL, "LG L-05A"},	//
//      {0x1004, 0x6135, "option", "0", "2", 0, NULL, "LG L-07A"},      //
	{0x1004, 0x613a, "option", "0", "0", 0, &modeswitch_std_eject, "LG L-05A"},	//
	{0x1004, 0x613f, "option", "0", "0", 0, &modeswitch_std_eject, "LG LUU-2100TI"},	//
	{0x1004, 0x6141, "option", "0", "2", 0 | ACM, NULL, "LG LUU-2100TI"},	//
	{0x1004, 0x614e, "option", "0", "0", 0, &modeswitch_std_eject, "LG L-07A"},	//
	{0x1004, 0x6156, "option", "0", "0", 0, &modeswitch_std_eject, "LG LUU-2110TI"},	//
	{0x1004, 0x6157, "option", "0", "2", 0 | ACM, NULL, "LG LUU-2110TI"},	//
	{0x1004, 0x618f, "option", "0", "2", 0, NULL, "LG L-02C"},	//
	{0x1004, 0x6190, "option", "0", "0", 0, &modeswitch_std_eject, "LG AD600"},	//
//      {0x1004, 0x61a7, "option", "0", "2", 0, NULL, "LG AD600"},      // also cdc_ether
//      {0x1004, 0x61e6, "option", "0", "2", 0, NULL, "LG SD711"},      //
	{0x1004, 0x61e7, "option", "0", "0", 0, &modeswitch_std_eject, "LG SD711"},	//
//      {0x1004, 0x61ea, "option", "0", "2", 0, NULL, "LG L-08C"},      //
	{0x1004, 0x61eb, "option", "0", "0", 0, &modeswitch_std_eject, "LG L-08C"},	//
	{0x1004, 0x61dd, "option", "0", "0", 0, &modeswitch_std_eject, "LG L-02C"},	//
//      {0x1004, 0x6326, "option", "0", "2", 0, NULL, "LG L-03D"},      //
//      {0x1004, 0x6327, "option", "0", "0", 0, &modeswitch_std_eject, "LG L-03D"},     //

// Unknown vendor
	{0x1033, 0x0035, "option", "0", "0", 2, &modeswitch_huawei_old, "HUAWEI E630"},	//

// Curitel
	{0x106c, 0x3711, "option", "0", "0", 2 | ACM, NULL, "PANTECH UM-150"},	//
	{0x106c, 0x3712, "option", "0", "0", 2 | ACM, NULL, "PANTECH UM-175V1"},	//
	{0x106c, 0x3714, "option", "0", "0", 2 | ACM, NULL, "PANTECH UM-175VW"},	//
	{0x106c, 0x3715, "option", "0", "0", 2 | ACM, NULL, "PANTECH UM-175AL"},	//
	{0x106c, 0x3716, "option", "0", "0", 2 | ACM, NULL, "PANTECH UM-190VW"},	//
	{0x106c, 0x3717, "option", "0", "0", 2 | ACM, NULL, "PANTECH UM-185C/UM185E"},	//
	{0x106c, 0x3718, "option", "0", "0", 2 | QMI, NULL, "PANTECH UML-290VW 4G Modem"},	//
	{0x106c, 0x3721, "option", "0", "0", 2 | QMI, NULL, "PANTECH P4200 4G Modem"},	//
	{0x106c, 0x3b03, "option", "0", "0", 2, &modeswitch_curitel, "PANTECH UM-175AL"},	//
	{0x106c, 0x3b05, "option", "0", "0", 2, &modeswitch_curitel, "PANTECH UM-190"},	//
	{0x106c, 0x3b06, "option", "0", "0", 2, &modeswitch_curitel, "PANTECH UM-185C/UM185E"},	//
	{0x106c, 0x3b11, "option", "0", "0", 2, &modeswitch_curitel, "PANTECH UML-290"},	//
	{0x106c, 0x3b14, "option", "0", "0", 2, &modeswitch_curitel, "PANTECH P4200"},	//

// SK Teletech
	{0x10a9, 0x6064, "option", "0", "0", 2 | ACM, NULL, "PANTECH UML-295"},	// also cdc_ether
	{0x10a9, 0x6080, "option", "0", "0", 2, &modeswitch_others, "PANTECH MHS291LVW (cdrom)"},	//
	{0x10a9, 0x6085, "option", "0", "0", 2 | ACM, NULL, "PANTECH MHS291LVW"},	// also cdc_ether

// Sierra Wireless-Wavecom
	{0x114f, 0x68a2, "sierra", "2", "0", 1 | QMI, NULL, "Sierra MC7750 (modem)"},	//

// Sierra Wireless (Netgear)
	{0x1199, 0x0017, "sierra", "3", "4", 1, NULL, "Sierra EM5625 (modem)"},	//
	{0x1199, 0x0018, "sierra", "3", "4", 1, NULL, "Sierra MC5720 (modem)"},	//
	{0x1199, 0x0019, "sierra", "3", "4", 1, NULL, "Sierra AC595U (modem)"},	//
	{0x1199, 0x0020, "sierra", "3", "4", 1, NULL, "Sierra MC5725 (modem)"},	//
	{0x1199, 0x0021, "sierra", "3", "4", 1, NULL, "Sierra AC597E (modem)"},	//
	{0x1199, 0x0022, "sierra", "3", "4", 1, NULL, "Sierra EM5725 (modem)"},	//
	{0x1199, 0x0023, "sierra", "3", "4", 1, NULL, "Sierra C597 (modem)"},	//
	{0x1199, 0x0024, "sierra", "3", "4", 1, NULL, "Sierra MC5727 CDMA (modem)"},	//
	{0x1199, 0x0025, "sierra", "3", "4", 1, NULL, "Sierra AC598 (modem)"},	//
	{0x1199, 0x0026, "sierra", "3", "4", 1, NULL, "Sierra T11 (modem)"},	//
	{0x1199, 0x0027, "sierra", "3", "4", 1, NULL, "Sierra AC402 (modem)"},	//
	{0x1199, 0x0028, "sierra", "3", "4", 1, NULL, "Sierra MC5728 (modem)"},	//
	{0x1199, 0x0112, "sierra", "3", "4", 1, NULL, "Sierra CDMA 1xEVDO PC Card, AC580 (modem)"},	//
	{0x1199, 0x0120, "sierra", "3", "4", 1, NULL, "Sierra AC595U (modem)"},	//
	{0x1199, 0x0218, "sierra", "3", "4", 1, NULL, "Sierra MC5720 (modem)"},	//
	{0x1199, 0x0220, "sierra", "3", "4", 1, NULL, "Sierra MC5725 (modem)"},	//
	{0x1199, 0x0224, "sierra", "3", "4", 1, NULL, "Sierra MC5727 (modem)"},	//
	{0x1199, 0x0301, "sierra", "3", "4", 1, NULL, "Sierra AC250U (modem)"},	//
	{0x1199, 0x0fff, "sierra", "3", "4", 1, &modeswitch_sierra, "Sierra Device (cdrom)"},	//
	{0x1199, 0x6802, "sierra", "0", "2", 1, NULL, "Sierra MC8755 (modem)"},	//
	{0x1199, 0x6803, "sierra", "0", "2", 1, NULL, "Sierra MC8765 (modem)"},	//
	{0x1199, 0x6804, "sierra", "0", "2", 1, NULL, "Sierra MC8755 (modem)"},	//
	{0x1199, 0x6805, "sierra", "0", "2", 1, NULL, "Sierra MC8765 (modem)"},	//
	{0x1199, 0x6808, "sierra", "0", "2", 1, NULL, "Sierra MC8755 (modem)"},	//
	{0x1199, 0x6809, "sierra", "0", "2", 1, NULL, "Sierra MC8765 (modem)"},	//
	{0x1199, 0x6812, "sierra", "0", "2", 1, &reset_mc, "Sierra MC8775V"},	//
	{0x1199, 0x6813, "sierra", "0", "2", 1, NULL, "Sierra MC8775 (modem)"},	//
	{0x1199, 0x6815, "sierra", "0", "2", 1, NULL, "Sierra MC8775 (modem)"},	//
	{0x1199, 0x6816, "sierra", "0", "2", 1, NULL, "Sierra MC8775 (modem)"},	//
	{0x1199, 0x6820, "sierra", "0", "2", 1, NULL, "Sierra AC875 (modem)"},	//
	{0x1199, 0x6821, "sierra", "2", "0", 1, NULL, "Sierra AC875U (modem)"},	//
	{0x1199, 0x6822, "sierra", "3", "4", 1, NULL, "Sierra AC875E (modem)"},	//
	{0x1199, 0x6832, "sierra", "2", "0", 1, &reset_mc, "Sierra MC8780 (modem)"},	//
	{0x1199, 0x6833, "sierra", "3", "4", 1, NULL, "Sierra MC8781 (modem)"},	//
	{0x1199, 0x6834, "sierra", "3", "4", 1, NULL, "Sierra MC8780 (modem)"},	//
	{0x1199, 0x6835, "sierra", "3", "4", 1, NULL, "Sierra MC8781 (modem)"},	//
	{0x1199, 0x6838, "sierra", "3", "4", 1, NULL, "Sierra MC8780 (modem)"},	//
	{0x1199, 0x6839, "sierra", "3", "4", 1, NULL, "Sierra MC8781 (modem)"},	//
	{0x1199, 0x683a, "sierra", "3", "4", 1, NULL, "Sierra MC8785 (modem)"},	//
	{0x1199, 0x683b, "sierra", "3", "4", 1, NULL, "Sierra MC8785 Composite (modem)"},	//
	{0x1199, 0x683c, "sierra", "3", "3", 1, NULL, "Sierra MC8790 Composite"},	//
	{0x1199, 0x683d, "sierra", "3", "3", 1, &reset_mc, "Sierra MC8791 Composite"},	//
	{0x1199, 0x683e, "sierra", "3", "3", 1, &reset_mc, "Sierra MC8790"},	//
	{0x1199, 0x6850, "sierra", "2", "0", 1, NULL, "Sierra AC880 (modem)"},	//
	{0x1199, 0x6851, "sierra", "2", "0", 1, NULL, "Sierra AC 881 (modem)"},	//
	{0x1199, 0x6852, "sierra", "2", "0", 1, NULL, "Sierra AC880E (modem)"},	//
	{0x1199, 0x6853, "sierra", "2", "0", 1, NULL, "Sierra AC881E (modem)"},	//
	{0x1199, 0x6855, "sierra", "2", "0", 1, NULL, "Sierra AC880U (modem)"},	//
	{0x1199, 0x6856, "sierra", "2", "0", 1, NULL, "Sierra ATT USB Connect 881 (modem)"},	//
	{0x1199, 0x6859, "sierra", "2", "0", 1, NULL, "Sierra AC885E (modem)"},	//
	{0x1199, 0x685a, "sierra", "2", "0", 1, NULL, "Sierra AC885E (modem)"},	//
	{0x1199, 0x6880, "sierra", "3", "3", 1, NULL, "Sierra C885"},	//
	{0x1199, 0x6890, "sierra", "3", "3", 1, NULL, "Sierra C888"},	//
	{0x1199, 0x6891, "sierra", "3", "3", 1, NULL, "Sierra C22 and C33"},	//
	{0x1199, 0x6892, "sierra", "3", "3", 1, NULL, "Sierra Compass HSPA"},	//
	{0x1199, 0x6893, "sierra", "3", "3", 1, NULL, "Sierra C889"},	//
	{0x1199, 0x68a2, "sierra", "2", "0", 1 | QMI, NULL, "Sierra MC7710 (modem)"},	// also cdc_mbim
	{0x1199, 0x68a3, "sierra", "3", "3", 1, &reset_mc, "Sierra MC8700/Compass Direct IP"},	//
	{0x1199, 0x68aa, "sierra", "3", "3", 1, NULL, "Sierra AC320U/AC330U Direct IP"},	//
	{0x1199, 0x9011, "sierra", "2", "0", 1 | QMI, &select_config1, "Sierra MC8305 (modem)"},	// cdc_mbim in default config2
	{0x1199, 0x9013, "sierra", "2", "0", 1 | QMI, &select_config1, "Sierra MC8355 (modem)"},	// cdc_mbim in default config2
	{0x1199, 0x9051, "sierra", "2", "0", 1 | QMI, &select_config1, "Netgear AC340U (modem)"},	// cdc_mbim in default config2

// Pirelli Broadband Solutions
	{0x1266, 0x1000, "option", "0", "0", 0, &modeswitch_std_eject, "Pirelli"},	//
	{0x1266, 0x1009, "option", "2", "0", 2, NULL, "Digicom 8E4455 (modem)"},	//

// Huawei Technologies
	{0x12d1, 0x1001, "option", "2", "0", 2, &modeswitch_huawei_old, "HUAWEI/Option E600/E620 or generic"},	//
	{0x12d1, 0x1003, "option", "1", "0", 2, &modeswitch_huawei_old, "HUAWEI/Option E172/EC27/E220/E230/E270 HSDPA/HSUPA Model"},	//
	{0x12d1, 0x1009, "option", "0", "0", 2, &modeswitch_huawei_old, "HUAWEI/Option U120"},	//
	{0x12d1, 0x1010, "option", "0", "0", 2, &modeswitch_huawei_old, "HUAWEI ETS1201"},	//
	{0x12d1, 0x101e, "option", "0", "0", 2, &modeswitch_huawei_std, "HUAWEI U7510 / U7517"},	//
	{0x12d1, 0x1030, "option", "0", "0", 2, &modeswitch_huawei_std, "HUAWEI U8220 (Android smartphone)"},	//
	{0x12d1, 0x1031, "option", "0", "0", 2, &modeswitch_huawei_std, "HUAWEI U8110 (Android smartphone)"},	//
	{0x12d1, 0x1034, "option", "0", "0", 2, NULL, "HUAWEI U8220 (Android smartphone)"},	//
	{0x12d1, 0x1035, "option", "0", "0", 2, NULL, "HUAWEI U8110 (Android smartphone)"},	//
//      {0x12d1, 0x1400, NULL, NULL, NULL, 0 | ETH, NULL,  "Huawei K4305 composite"},   //
	{0x12d1, 0x1404, "option", "2", "0", 2 | QMI, NULL, "HUAWEI UMG1831"},	//
	{0x12d1, 0x1406, "option", "1", "0", 2, NULL, "HUAWEI/Option newer modems"},	//
	{0x12d1, 0x140b, "option", "2", "0", 2, NULL, "HUAWEI/Option EC1260 Wireless Data Modem HSD USB Card"},	//
	{0x12d1, 0x140c, "option", "3", "0", 2 | QMI, NULL, "HUAWEI/Option newer modems"},	//
	{0x12d1, 0x1411, "option", "2", "0", 2, &modeswitch_huawei_old, "HUAWEI E510/EC121"},	//
	{0x12d1, 0x1412, "option", "2", "0", 2, NULL, "HUAWEI/Option EC168"},	//
	{0x12d1, 0x1413, "option", "2", "0", 2, &modeswitch_huawei_old, "HUAWEI/Option EC168"},	//
	{0x12d1, 0x1414, "option", "2", "0", 2, &modeswitch_huawei_old, "HUAWEI/Option E180"},	//
	{0x12d1, 0x1417, "option", "2", "0", 2, NULL, "HUAWEI E1756"},	//
	{0x12d1, 0x141b, "option", "1", "0", 2, NULL, "HUAWEI/Option newer modems"},	//
	{0x12d1, 0x1429, "option", "2", "0", 2, NULL, "HUAWEI/EMobile D31HW"},	// also qmi but not yet in driver
//      {0x12d1, 0x1432, "option", "0", "0", 2 | QMI, NULL, "HUAWEI E585"},     // ecm attributes but probably qmi
	{0x12d1, 0x1433, "option", "2", "0", 2, NULL, "HUAWEI/Option E1756C"},	// also qmi but not yet in driver
	{0x12d1, 0x1436, "option", "2", "0", 2, NULL, "HUAWEI/Option E1800"},	// ecm attributes but probably qmi
	{0x12d1, 0x1444, "option", "0", "0", 2, NULL, "HUAWEI/Option E352-R1"},	//
	{0x12d1, 0x1446, "option", "0", "0", 2, &modeswitch_huawei_std, "HUAWEI/Option E1552/E1800 HSPA Modem"},	//
	{0x12d1, 0x1449, "option", "0", "0", 2, &modeswitch_huawei_std, "HUAWEI/Option E352-R1"},	//
	{0x12d1, 0x144e, "option", "0", "2", 2, NULL, "Huawei K3806"},	//
	{0x12d1, 0x1464, "option", "2", "0", 2, NULL, "Huawei K4505"},	// qmi but not yet in driver
	{0x12d1, 0x1465, "option", "2", "0", 2, NULL, "Huawei K3765"},	// ecm attributes but probably qmi
//      {0x12d1, 0x1491, "option", "0", "0", 2 | QMI, NULL, "Vodafone R201"},   // qmi only but not yet in driver
	{0x12d1, 0x14a5, "option", "2", "0", 2, NULL, "Huawei E173"},	//
	{0x12d1, 0x14a8, "option", "2", "0", 2, NULL, "Huawei E173"},	//
	{0x12d1, 0x14aa, "option", "2", "0", 2, NULL, "Huawei E1750"},	//
	{0x12d1, 0x14ac, "option", "2", "0", 2 | QMI, NULL, "HUAWEI/Option newer modems"},	//
	{0x12d1, 0x14ad, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei K3806"},	//
	{0x12d1, 0x14ae, "option", "1", "2", 2, NULL, "Huawei K3806"},	// also cdc_ether
	{0x12d1, 0x14b5, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E173"},	//
	{0x12d1, 0x14b7, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei K4511"},	//
	{0x12d1, 0x14ba, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E173/E177 (cdrom)"},	//
//      {0x12d1, 0x14bc, NULL, NULL, NULL, 0 | ETH, NULL,  "Huawei K3773 (net)"},       //
	{0x12d1, 0x14c1, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei K4605"},	//
	{0x12d1, 0x14c3, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei K5005"},	//
	{0x12d1, 0x14c4, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei K3771"},	//
	{0x12d1, 0x14c5, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei K4510"},	//
	{0x12d1, 0x14c6, "option", "2", "0", 2 | QMI, NULL, "Huawei K4605"},	//
	{0x12d1, 0x14c8, "option", "2", "0", 2 | QMI, NULL, "Huawei K5005"},	//
	{0x12d1, 0x14c9, "option", "2", "0", 2, NULL, "Huawei K3770"},	//
	{0x12d1, 0x14ca, "option", "2", "0", 2 | QMI, NULL, "Huawei K3771"},	//
	{0x12d1, 0x14cb, "option", "2", "0", 2, NULL, "Huawei K4510"},	//
	{0x12d1, 0x14cc, "option", "2", "0", 2 | QMI, NULL, "Huawei K4511"},	//
//      {0x12d1, 0x14cd, "option", "2", "0", 2 | H_NCM, NULL, "Vodafone R205"}, // cdc_ncm only
	{0x12d1, 0x14cf, "option", "2", "0", 2, NULL, "Huawei K3772 (modem)"},	// cdc_ncm able
	{0x12d1, 0x14d1, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E182E"},	//
	{0x12d1, 0x14d2, "option", "2", "0", 2 | QMI, NULL, "Huawei E173/E177 (modem)"},	//
//      {0x12d1, 0x14db, NULL, NULL, NULL, 0 | ETH, NULL,  "Huawei E353"},      //
//      {0x12d1, 0x14dc, NULL, NULL, NULL, 0 | ETH, NULL,  "Huawei E303"},      //
	{0x12d1, 0x14fe, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E352,E353"},	//
	{0x12d1, 0x1505, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E398"},	//
	{0x12d1, 0x1506, "option", "2", "0", 2, NULL, "Huawei E367/E398 (modem)"},	// can not be QMI flagged!
	{0x12d1, 0x150a, "option", "2", "0", 2 | QMI, NULL, "Huawei E398 (modem)"},	//
	{0x12d1, 0x150c, "option", "1", "2", 2 | QMI, NULL, "Huawei E367"},	//
	{0x12d1, 0x150f, "option", "0", "0", 2 | QMI, NULL, "Huawei E367"},	//
	{0x12d1, 0x151a, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E392u-12"},	//
	{0x12d1, 0x151b, "option", "0", "0", 2 | QMI, NULL, "Huawei E392u-12"},	// 
	{0x12d1, 0x151d, "option", "3", "0", 2, NULL, "Huawei E3131 (modem)"},	// ncm & mbim able
	{0x12d1, 0x1520, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei K3765"},	//
	{0x12d1, 0x1521, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei K4505"},	//
	{0x12d1, 0x1523, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei R120"},	//
	{0x12d1, 0x1526, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei K3772 (cdrom)"},	//
	{0x12d1, 0x1553, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E1553"},	//
	{0x12d1, 0x1557, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E173"},	//
	{0x12d1, 0x155a, "option", "0", "0", 2, &modeswitch_huawei_std, "Vodafone R205"},	//
	{0x12d1, 0x155b, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E171/E320"},	//
	{0x12d1, 0x156a, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E3251/E3276"},	//
	{0x12d1, 0x156c, "option", "1", "0", 2, NULL, "Huawei E3276 (modem)"},	// also cdc_ncm
	{0x12d1, 0x1570, "option", "1", "0", 2, NULL, "Huawei ME906E (modem)"},	// also cdc_mbim
	{0x12d1, 0x1571, "option", "1", "0", 2, NULL, "Huawei EM820W (modem)"},	// also cdc_mbim
//      {0x12d1, 0x1575, NULL, NULL, NULL, 2 | ETH, NULL,  "Huawei K5150 composite"},   //
//      {0x12d1, 0x1576, NULL, NULL, NULL, 2 | ETH, NULL,  "Huawei K4201 composite"},   //
//      {0x12d1, 0x1577, NULL, NULL, NULL, 2 | ETH, NULL,  "Huawei K4202 composite"},   //
//      {0x12d1, 0x1578, NULL, NULL, NULL, 2 | ETH, NULL,  "Huawei K4606 composite"},   //
	{0x12d1, 0x157c, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E3276s-150"},	// also cdc_mbim
	{0x12d1, 0x157d, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E3331"},	// also cdc_mbim
	{0x12d1, 0x1581, "option", "0", "0", 2, &modeswitch_huawei_std, "Vodafone R208"},	//
	{0x12d1, 0x1582, "option", "0", "0", 2, &modeswitch_huawei_std, "Vodafone R215"},	//
	{0x12d1, 0x1583, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E8278"},	//
//      {0x12d1, 0x1588, "option", "0", "0", 2 | NCM, NULL, "Vodafone R215 (net)"},     //
//      {0x12d1, 0x1589, "option", "0", "0", 2 | NCM, NULL, "Huawei E8278 (net)"},      //
	{0x12d1, 0x15ca, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E3131"},	//
	{0x12d1, 0x15e7, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E3531"},	//
	{0x12d1, 0x1805, "option", "1", "0", 2, &modeswitch_rezero, "Huawei U2800A/U6150 (modem)"},	//
	{0x12d1, 0x1c05, "option", "2", "0", 2, NULL, "Huawei E173s (modem)"},	//
	{0x12d1, 0x1c07, "option", "2", "0", 2, NULL, "Huawei E188 (modem)"},	// also cdc_ncm
	{0x12d1, 0x1c08, "option", "1", "0", 2, NULL, "Huawei E173s (modem)"},	//
	{0x12d1, 0x1c0b, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E173s (cdrom)"},	//
	{0x12d1, 0x1c10, "option", "2", "0", 2, NULL, "Huawei E173 (modem)"},	//
	{0x12d1, 0x1c12, "option", "2", "0", 2, NULL, "Huawei E173 (modem)"},	//
	{0x12d1, 0x1c1b, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E398 (cdrom)"},	//
//      {0x12d1, 0x1c1e, "option", "1", "0", 2 | NCM, NULL, "Huawei E586 (net)"},       //
//      {0x12d1, 0x1c1f, "option", "0", "0", 2 | NCM, NULL, "Huawei E587 (net)"},       //
	{0x12d1, 0x1c20, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E5220s-2 (cdrom)"},	//
	{0x12d1, 0x1c23, "option", "0", "2", 2, NULL, "Huawei E173 (modem)"},	//
	{0x12d1, 0x1c24, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E173 (cdrom)"},	//
	{0x12d1, 0x1d09, "option", "0", "0", 2 | ACM, NULL, "Huawei ET8282 (modem)"},	//
	{0x12d1, 0x1da1, "option", "2", "0", 2, &modeswitch_huawei_old, "Huawei ET8282 (cdrom)"},	//
	{0x12d1, 0x1f01, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E303/E353 (cdrom)"},	//
	{0x12d1, 0x1f03, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E353 (cdrom)"},	//
	{0x12d1, 0x1f11, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei K3773 (cdrom)"},	//
	{0x12d1, 0x1f15, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei K4305 (cdrom)"},	// also cdc_mbim
	{0x12d1, 0x1f16, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei K5150 (cdrom)"},	// also cdc_mbim
	{0x12d1, 0x1f17, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei K4201 (cdrom)"},	// also cdc_mbim
	{0x12d1, 0x1f18, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei K4202 (cdrom)"},	// also cdc_mbim
	{0x12d1, 0x1f19, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei K4606 (cdrom)"},	// also cdc_mbim

// Novatel Wireless
	{0x1410, 0x1400, "option", "1", "0", 2, NULL, "Novatel U730 (modem)"},	//
	{0x1410, 0x1410, "option", "1", "0", 2, NULL, "Novatel U740 (modem)"},	//
	{0x1410, 0x1420, "option", "1", "0", 2, NULL, "Novatel U870 (modem)"},	//
	{0x1410, 0x1430, "option", "1", "0", 2, NULL, "Novatel XU870 (modem)"},	//
	{0x1410, 0x1450, "option", "1", "0", 2, NULL, "Novatel X950D (modem)"},	//
	{0x1410, 0x2100, "option", "1", "0", 2, NULL, "Novatel EV620 (modem)"},	//
	{0x1410, 0x2110, "option", "1", "0", 2, NULL, "Novatel ES720 (modem)"},	//
	{0x1410, 0x2120, "option", "1", "0", 2, NULL, "Novatel E725 (modem)"},	//
	{0x1410, 0x2130, "option", "1", "0", 2, NULL, "Novatel ES620 (modem)"},	//
	{0x1410, 0x2400, "option", "1", "0", 2, NULL, "Novatel EU730 (modem)"},	//
	{0x1410, 0x2410, "option", "1", "0", 2, NULL, "Novatel EU740 (modem)"},	//
	{0x1410, 0x2420, "option", "1", "0", 2, NULL, "Novatel EU870D (modem)"},	//
	{0x1410, 0x4100, "option", "1", "0", 2, NULL, "Novatel MC727/U727 (modem)"},	//
	{0x1410, 0x4400, "option", "1", "0", 2, NULL, "Novatel Ovation MC930D/MC950D (modem)"},	//
	{0x1410, 0x5010, "option", "0", "0", 2, &modeswitch_std_eject, "Novatel X950D(cdrom)"},	//
	{0x1410, 0x5020, "option", "0", "0", 2, &modeswitch_std_eject, "Novatel MC990D (cdrom)"},	//
	{0x1410, 0x5023, "option", "0", "0", 2, &modeswitch_std_eject, "Novatel MC996D (cdrom)"},	//
	{0x1410, 0x5030, "option", "0", "0", 2, &modeswitch_std_eject, "Novatel USB760 (cdrom)"},	//
	{0x1410, 0x5031, "option", "0", "0", 2, &modeswitch_std_eject, "Novatel USB760 3G (cdrom)"},	//
	{0x1410, 0x5041, "option", "0", "0", 2, &modeswitch_std_eject, "Novatel MiFi 2372 (cdrom)"},	//
	{0x1410, 0x5055, "option", "0", "0", 2, &modeswitch_std_eject, "Novatel MiFi 4082 (cdrom)"},	//
	{0x1410, 0x5059, "option", "0", "0", 2, &modeswitch_std_eject, "Novatel MC545/USB679 (cdrom)"},	//
	{0x1410, 0x6000, "option", "1", "0", 2, NULL, "Novatel USB760 (modem)"},	//
	{0x1410, 0x6001, "option", "1", "0", 2, NULL, "Novatel USB760 (modem)"},	//
	{0x1410, 0x6002, "option", "1", "0", 2, NULL, "Novatel USB760 3G (modem)"},	//
	{0x1410, 0x6010, "option", "1", "0", 2, NULL, "Novatel MC780 (modem)"},	//
	{0x1410, 0x7001, "option", "1", "0", 2, NULL, "Novatel MiFi 2372 (modem)"},	//
	{0x1410, 0x7003, "option", "1", "0", 2, NULL, "Novatel MiFi 2372 (modem)"},	//
	{0x1410, 0x7030, "option", "0", "0", 2, NULL, "Novatel USB998 (modem)"},	// also cdc_ether
	{0x1410, 0x7031, "option", "0", "0", 2 | GENERIC, NULL, "Novatel USB679 (modem)"},	// also cdc_ether
	{0x1410, 0x7041, "option", "0", "0", 2, NULL, "Novatel MF3470 (modem)"},	//
	{0x1410, 0x7042, "option", "0", "0", 2, NULL, "Novatel Ovation MC545/MC547 (modem)"},	//
	{0x1410, 0x9010, "option", "1", "0", 2 | QMI, NULL, "Novatel E362 (modem)"},	//
	{0x1410, 0x9011, "option", "1", "0", 2 | QMI, NULL, "Novatel E371 (modem)"},	//
	{0x1410, 0xb001, "option", "1", "0", 2 | QMI, NULL, "Novatel MC551/USB551L (modem)"},	//
//      {0x1410, 0xb005, NULL, NULL, NULL, 2 | RNDIS, NULL, "Novatel MiFi 4620L/4620LE"},       //
	{0x1410, 0xb00b, NULL, NULL, NULL, 2, &select_config2, "Novatel MiFi 5510L"},	// rndis in config1, cdc_ether in config2

// UBIQUAM
	{0x1529, 0x3100, "option", "0", "0", 2 | ACM, NULL, "UBIQUAM U-100/105/200/300/520"},	//

// VIA Telecom
	{0x15eb, 0x0001, "option", "1", "0", 2 | GENERIC, NULL, "Ublox FW2760/2770"},	//
	{0x15eb, 0x1231, "option", "0", "0", 2 | ACM, NULL, "Prithvi UE100"},	//
	{0x15eb, 0x7152, "option", "3", "0", 2 | GENERIC, NULL, "Tenda 3G189C"},	//
	{0x15eb, 0x7153, "option", "0", "0", 2, &modeswitch_std_eject, "Tenda 3G189C (cdrom)"},	//

// Amoi
	{0x1614, 0x0800, "option", "1", "0", 2, &modeswitch_rezero, "Amoi H01 (modem)"},	//
	{0x1614, 0x0802, "option", "1", "0", 2, &modeswitch_rezero, "Amoi H02 (modem)"},	//
	{0x1614, 0x7002, "option", "1", "0", 2, &modeswitch_rezero, "Amoi H01-A (modem)"},	//

// AnyDATA
	{0x16d5, 0x6202, "option", "2", "0", 2, NULL, "AnyData ADU-620UW"},	//
	{0x16d5, 0x6501, "option", "1", "0", 2, NULL, "AnyData ADU-300A"},	//
	{0x16d5, 0x6502, "option", "2", "0", 2, NULL, "AnyData ADU-500A"},	//
	{0x16d5, 0x6603, "option", "0", "0", 2 | GENERIC, NULL, "AnyData ADU-890WH"},	//
	{0x16d5, 0x900d, "option", "0", "0", 2 | ACM, NULL, "AnyData ADU-890WH"},	//
	{0x16d5, 0xf000, "option", "0", "0", 2, &modeswitch_std_eject, "AnyData (cdrom)"},	//

// CMOTECH
	{0x16d8, 0x5141, "option", "0", "0", 0 | ACM, NULL, "Cmotech CNU-510"},	//
	{0x16d8, 0x5533, "option", "0", "0", 0 | ACM, NULL, "Cmotech CNU-550"},	//
	{0x16d8, 0x5543, "option", "0", "0", 0 | ACM, NULL, "Cmotech CNU-550"},	//
	{0x16d8, 0x5553, "option", "0", "0", 0 | ACM, NULL, "Cmotech CDU-550"},	//
	{0x16d8, 0x6002, "option", "1", "0", 0, NULL, "Franklin U300"},	//
	{0x16d8, 0x6006, "option", "0", "0", 0, NULL, "Cmotech CGU-628"},	//
	{0x16d8, 0x6007, "option", "0", "0", 0 | QMI, NULL, "Cmotech CHE-628S"},	//
	{0x16d8, 0x6008, "option", "2", "1", 0 | QMI, NULL, "Franklin U301"},	//
	{0x16d8, 0x6280, "option", "2", "1", 0 | QMI, &modeswitch_devchg_ff, "Cmotech CHU-628s"},	//
	{0x16d8, 0x6281, "option", "0", "0", 0, &modeswitch_devchg_ff, "Cmotech CHU-628s"},	//
	{0x16d8, 0x6522, "option", "2", "0", 0 | ACM, NULL, "Cmotech CDU-650"},	//
	{0x16d8, 0x6523, "option", "2", "0", 0 | ACM, NULL, "Cmotech CCU-650U"},	//
	{0x16d8, 0x6532, "option", "2", "0", 0 | ACM, NULL, "Cmotech CCU-650"},	//
	{0x16d8, 0x6533, "option", "0", "0", 0 | ACM, NULL, "Cmotech CNM-650"},	//
	{0x16d8, 0x6543, "option", "0", "0", 0 | ACM, NULL, "Cmotech CNU-650"},	//
	{0x16d8, 0x6803, "option", "1", "0", 0, &modeswitch_devchg1, "Cmotech CDU-680"},	//
	{0x16d8, 0x6804, "option", "2", "1", 0, &modeswitch_devchg1, "Cmotech CDU-685A"},	//
	{0x16d8, 0x680a, "option", "0", "0", 0 | ACM, NULL, "Cmotech CDU-680"},	//
	{0x16d8, 0x7001, "option", "1", "1", 0 | QMI, &modeswitch_devchg_fe, "Cmotech CHU-720S"},	//
	{0x16d8, 0x7003, "option", "1", "2", 0 | QMI, &modeswitch_devchg_fe, "Cmotech CHU-629K"},	//
	{0x16d8, 0x7006, "option", "1", "2", 0 | QMI, &modeswitch_devchg_fe, "Cmotech CGU-629"},	//
	{0x16d8, 0x700a, "option", "0", "2", 0 | QMI, &modeswitch_devchg_fe, "Cmotech CHU-629S"},	//'
	{0x16d8, 0x7211, "option", "1", "1", 0 | QMI, &modeswitch_devchg_fe, "Cmotech CHU-720I"},	//
	{0x16d8, 0xf000, "option", "0", "0", 0, &modeswitch_devchg_ff, "Cmotech CGU-628, 4g_xsstick W12"},	//

// AxessTel
	{0x1726, 0x1000, "option", "0", "0", 0, &modeswitch_std_eject, "Axesstel MV140B"},	//
	{0x1726, 0xa000, "option", "2", "3", 2 | GENERIC, NULL, "Axesstel MU130 (modem)"},	//
	{0x1726, 0xf00e, "option", "0", "0", 0, &modeswitch_std_eject, "Axesstel MU130"},	//

// MODMEN
	{0x198a, 0x0003, "option", "0", "0", 0, &modeswitch_std_eject, "MODMEN MM450"},	//
	{0x198a, 0x0002, "option", "2", "0", 2 | GENERIC, NULL, "MODMEN MM450 (modem)"},	//

// ZTE WCDMA Technologies
	{0x19d2, 0x0001, "option", "2", "0", 2, NULL, "ONDA MT505UP/ZTE (modem)"},	//
	{0x19d2, 0x0002, "option", "2", "0", 2 | QMI, NULL, "ZTE ET502HS/MT505UP/MF632"},	//
	{0x19d2, 0x0003, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MU351 (cdrom)"},	//
	{0x19d2, 0x0015, "option", "2", "0", 2, NULL, "ONDA MT505UP/ZTE (modem)"},	//
	{0x19d2, 0x0016, "option", "1", "2", 2, NULL, "ONDA MF110/ZTE (modem)"},	//
	{0x19d2, 0x0017, "option", "1", "2", 2 | QMI, NULL, "ONDA MT505UP/ZTE (modem)"},	//
	{0x19d2, 0x0018, "option", "1", "2", 2, NULL, "ONDA MSA110UP/ZTE (modem)"},	//
	{0x19d2, 0x0019, "option", "1", "2", 2 | QMI, NULL, "ONDA MT689DC/ZTE (modem)"},	//
	{0x19d2, 0x0022, "option", "1", "0", 2, NULL, "ZTE K2525 (modem)"},	//
	{0x19d2, 0x0024, "option", "2", "0", 2, NULL, "ONDA MT503HSA (modem)"},	// also qmi but not yet in driver
	{0x19d2, 0x0025, "option", "4", "2", 2 | QMI, NULL, "ZTE MF628 (modem)"},	//
	{0x19d2, 0x0026, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE AC581 (cdrom)"},	//
	{0x19d2, 0x0031, "option", "1", "2", 2 | QMI, NULL, "ZTE MF110/MF112/MF626 (Variant) (modem)"},	//
	{0x19d2, 0x0033, "option", "1", "4", 2, NULL, "ZTE MF636 (modem)"},	//
	{0x19d2, 0x0037, "option", "2", "2", 2, NULL, "ONDA MT505UP/ZTE (modem)"},	// also qmi but not yet in driver
	{0x19d2, 0x0039, "option", "1", "2", 2, NULL, "ZTE MF100 (modem)"},	//
	{0x19d2, 0x0040, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE K2525 (cdrom)"},	//
	{0x19d2, 0x0042, "option", "1", "2", 2 | QMI, NULL, "ZTE MF190 (modem)"},	//
	{0x19d2, 0x0052, "option", "1", "2", 2 | QMI, NULL, "ONDA MT505UP/ZTE (modem)"},	//
	{0x19d2, 0x0053, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF110 (Variant) (modem)"},	//
	{0x19d2, 0x0055, "option", "3", "1", 2 | QMI, NULL, "ONDA MT505UP/ZTE (modem)"},	//
	{0x19d2, 0x0057, "option", "0", "2", 2, NULL, "AIKO 83D (modem)"},	//
	{0x19d2, 0x0063, "option", "1", "3", 2 | QMI, NULL, "ZTE K3565-Z (modem)"},	//
	{0x19d2, 0x0064, "option", "0", "2", 2, NULL, "ZTE MF627 (modem)"},	//
	{0x19d2, 0x0066, "option", "1", "3", 2, NULL, "ZTE MF626 (modem)"},	//
	{0x19d2, 0x0073, "option", "1", "0", 2, NULL, "ZTE A580 (modem)"},	//
	{0x19d2, 0x0079, "option", "1", "2", 2, NULL, "ZTE A353 (modem)"},	//
	{0x19d2, 0x0082, "option", "1", "2", 2, NULL, "ZTE MF668/MF190 (Variant) (modem)"},	//
	{0x19d2, 0x0083, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF110 (Variant) (cdrom)"},	//
	{0x19d2, 0x0086, "option", "1", "2", 2, NULL, "ZTE MF645 (modem)"},	//
	{0x19d2, 0x0091, "option", "1", "3", 2, NULL, "ZTE MF636 (modem)"},	// also qmi but not yet in driver
	{0x19d2, 0x0094, "option", "3", "0", 2, NULL, "ZTE AC581 (modem)"},	//
	{0x19d2, 0x0101, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE K4505-Z (cdrom)"},	//
	{0x19d2, 0x0103, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF112 (cdrom)"},	//
	{0x19d2, 0x0104, "option", "1", "3", 2 | QMI, NULL, "ZTE K4505-Z (modem)"},	//
	{0x19d2, 0x0108, "option", "1", "3", 2, NULL, "ONDA MT505UP/ZTE (modem)"},	//
	{0x19d2, 0x0110, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF637 (cdrom)"},	//
	{0x19d2, 0x0115, "option", "0", "0", 2, &modeswitch_std_eject, "ONDA MT505UP/ZTE (cdrom)"},	//
	{0x19d2, 0x0116, "option", "1", "0", 2 | ACM, NULL, "ZTE MF651 (modem)"},	//
	{0x19d2, 0x0117, "option", "1", "2", 2, NULL, "ZTE MF112 (modem)"},	//
	{0x19d2, 0x0120, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE A353 (cdrom)"},	//
	{0x19d2, 0x0121, "option", "1", "3", 2 | QMI, NULL, "ZTE MF637 (modem)"},	//
	{0x19d2, 0x0124, "option", "1", "4", 2 | QMI, NULL, "ZTE MF110 (Variant) (modem)"},	//
	{0x19d2, 0x0128, "option", "1", "3", 2, NULL, "ZTE MF651 (modem)"},	//
	{0x19d2, 0x0142, "option", "0", "0", 2 | ACM, NULL, "ZTE MF665C"},	// also cdc_ether
	{0x19d2, 0x0143, "option", "0", "0", 2 | ACM, NULL, "ZTE MF190B"},	// also cdc_ether
	{0x19d2, 0x0146, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF652 (cdrom)"},	//
	{0x19d2, 0x0149, "option", "0", "0", 2, &modeswitch_zte, "ZTE MF190 (cdrom)"},	//
	{0x19d2, 0x0150, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF680 (cdrom)"},	//
	{0x19d2, 0x0151, "option", "1", "2", 2, NULL, "Prolink PHS101 (modem)"},	//
	{0x19d2, 0x0152, "option", "2", "0", 2, NULL, "ZTE AC583 (modem)"},	//
	{0x19d2, 0x0154, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF190S (cdrom)"},	//
	{0x19d2, 0x0157, "option", "0", "4", 2 | QMI, NULL, "ZTE MF683 (modem)"},	//
	{0x19d2, 0x0166, "option", "0", "0", 2, &modeswitch_others, "ZTE MF821 (Variant) (cdrom)"},	//
	{0x19d2, 0x0167, "option", "1", "3", 2 | QMI, NULL, "ZTE MF820D (variant) (modem)"},	//
	{0x19d2, 0x0169, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE A371 (cdrom)"},	//
	{0x19d2, 0x0170, "option", "0", "1", 2, NULL, "ZTE A371 (variant) (modem)"},	//
	{0x19d2, 0x0198, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF820S (cdrom)"},	//
	{0x19d2, 0x0199, "option", "1", "2", 2 | QMI, NULL, "ZTE MF820S (modem)"},	//
	{0x19d2, 0x0257, "option", "1", "2", 2 | QMI, NULL, "ZTE MF821 (variant) (modem)"},	//
	{0x19d2, 0x0265, "option", "2", "3", 2 | QMI, NULL, "Onda MT8205/ZTE (modem)"},	//
	{0x19d2, 0x0266, "option", "0", "0", 2, &modeswitch_others, "Onda MT8205/ZTE (cdrom)"},	//
	{0x19d2, 0x0284, "option", "1", "3", 2 | QMI, NULL, "ZTE MF880 (modem)"},	//
	{0x19d2, 0x0304, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF821D (cdrom)"},	//
	{0x19d2, 0x0317, "option", "1", "2", 2, NULL, "ZTE MF826 (modem)"},	// also qmi but not yet in driver
	{0x19d2, 0x0318, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF826 (cdrom)"},	//
	{0x19d2, 0x0325, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF821D (cdrom)"},	//
	{0x19d2, 0x0326, "option", "1", "3", 2 | QMI, NULL, "ZTE MF821D (modem)"},	//
	{0x19d2, 0x0330, "option", "1", "2", 2, NULL, "ZTE MF826 (modem)"},	// also qmi but not yet in driver
//      {0x19d2, 0x0349, NULL, NULL, NULL, 2 | ETH, NULL, "ZTE MF821D (modem)"},        // ecm attributes, might be QMI
//      {0x19d2, 0x0387, NULL, NULL, NULL, 2 | ETH, NULL, "ZTE MF90 (modem)"},  // ecm attributes, might be QMI
	{0x19d2, 0x0412, "option", "2", "3", 2 | QMI, NULL, "Telewell TW-LTE 4G (modem)"},	//
	{0x19d2, 0x1001, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE K3805-Z (cdrom)"},	//
	{0x19d2, 0x1003, "option", "1", "0", 2 | ACM, NULL, "ZTE K3805-Z (modem)"},	// also cdc_ether
	{0x19d2, 0x1007, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE K3570-Z (cdrom)"},	//
	{0x19d2, 0x1008, "option", "1", "3", 2 | QMI, NULL, "ZTE K3570-Z (modem)"},	//
	{0x19d2, 0x1009, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE K3571-Z (cdrom)"},	//
	{0x19d2, 0x1010, "option", "1", "3", 2 | QMI, NULL, "ZTE K3571-Z (modem)"},	//
	{0x19d2, 0x1013, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE K3806-Z (cdrom)"},	//
	{0x19d2, 0x1015, "option", "1", "0", 2 | ACM, NULL, "ZTE K3806-Z (modem)"},	// also cdc_ether
	{0x19d2, 0x1017, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE K5006-Z (cdrom)"},	//
	{0x19d2, 0x1018, "option", "1", "2", 2 | QMI, NULL, "ZTE K5006-Z (modem)"},	//
	{0x19d2, 0x1022, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE K4201-Z (cdrom)"},	// also cdc_mbim
//      {0x19d2, 0x1023, NULL, NULL, NULL, 0 | ETH, NULL, "ZTE K4201-Z (modem)"},       //
	{0x19d2, 0x1030, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE K5008-Z (cdrom)"},	// also cdc_mbim
//      {0x19d2, 0x1032, NULL, NULL, NULL, 0 | ETH, NULL, "ZTE K5008-Z (modem)"},       //
//      {0x19d2, 0x1040, NULL, NULL, NULL, 0 | ETH, NULL, "ZTE K4607-Z (modem)"},       //
	{0x19d2, 0x1171, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE K4510-Z (cdrom)"},	//
	{0x19d2, 0x1172, "option", "0", "0", 2 | ACM, NULL, "ZTE K4510-Z (modem)"},	// also cdc_ether
	{0x19d2, 0x1173, "option", "0", "0", 2 | ACM, NULL, "ZTE K4510-Z (modem)"},	// also cdc_ether
	{0x19d2, 0x1175, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE K3770-Z (cdrom)"},	//
	{0x19d2, 0x1176, "option", "0", "0", 2 | QMI, NULL, "ZTE K3770-Z (modem)"},	//
	{0x19d2, 0x1177, "option", "0", "0", 2 | ACM, NULL, "ZTE K3770-Z (modem)"},	// also cdc_ether
	{0x19d2, 0x1179, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE K3772-Z (cdrom)"},	//
	{0x19d2, 0x1181, "option", "0", "0", 2 | ACM, NULL, "ZTE K3772-Z (modem)"},	// also cdc_ether
	{0x19d2, 0x1201, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF691 (cdrom)"},	//
	{0x19d2, 0x1203, "option", "0", "0", 2 | ACM, NULL, "ZTE MF691 (modem)"},	//
	{0x19d2, 0x1207, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF192 (cdrom)"},	//
	{0x19d2, 0x1208, "option", "0", "0", 2 | ACM, NULL, "ZTE MF192 (modem)"},	//
	{0x19d2, 0x1210, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF195 (cdrom)"},	//
	{0x19d2, 0x1211, "option", "0", "0", 2 | ACM, NULL, "ZTE MF195 (modem)"},	// also cdc_ether
	{0x19d2, 0x1212, "option", "0", "0", 2 | ACM, NULL, "ZTE MF195 (modem)"},	//
	{0x19d2, 0x1216, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF192 (cdrom)"},	//
	{0x19d2, 0x1217, "option", "0", "0", 2 | ACM, NULL, "ZTE MF192"},	// also cdc_ether
	{0x19d2, 0x1218, "option", "0", "0", 2 | ACM, NULL, "ZTE MF192"},	//
	{0x19d2, 0x1219, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF192 (cdrom)"},	//
	{0x19d2, 0x1220, "option", "0", "0", 2 | ACM, NULL, "ZTE MF192"},	// also cdc_ether
	{0x19d2, 0x1222, "option", "0", "0", 2 | ACM, NULL, "ZTE MF192"},	// also cdc_ether
	{0x19d2, 0x1224, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF190 (cdrom)"},	//
	{0x19d2, 0x1225, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF667 (cdrom)"},	//
	{0x19d2, 0x1227, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF669 (cdrom)"},	//
	{0x19d2, 0x1232, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF193A (cdrom)"},	//
	{0x19d2, 0x1233, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF667 (cdrom)"},	//
	{0x19d2, 0x1238, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF825A (cdrom)"},	//
	{0x19d2, 0x1245, "option", "1", "0", 2 | QMI, NULL, "ZTE MF190 (modem)"},	//
	{0x19d2, 0x1252, "option", "1", "3", 2 | QMI, NULL, "ZTE MF669 (modem)"},	//
	{0x19d2, 0x1253, "option", "1", "3", 2, NULL, "Prolink PHS300 (modem)"},	//
	{0x19d2, 0x1254, "option", "1", "3", 2 | QMI, NULL, "ZTE MF190 (modem)"},	//
	{0x19d2, 0x1256, "option", "1", "0", 2 | QMI, NULL, "ZTE MF190 (modem)"},	//
	{0x19d2, 0x1268, "option", "1", "3", 2, NULL, "ZTE MF667 (modem)"},	// also qmi but not yet in driver
	{0x19d2, 0x1270, "option", "1", "4", 2 | QMI, NULL, "ZTE MF667 (modem)"},	//
	{0x19d2, 0x1300, "option", "2", "0", 2, NULL, "ZTE MF220 (modem)"},	//
	{0x19d2, 0x1401, "option", "0", "0", 2 | QMI, NULL, "ZTE MF60 (modem)"},	//
	{0x19d2, 0x1402, "option", "1", "1", 2 | QMI, NULL, "ZTE MF60 (modem)"},	//
//      {0x19d2, 0x1403, NULL, NULL, NULL, 0 | RNDIS, NULL, "ZTE MF825A (modem)"},      //
//      {0x19d2, 0x1405, NULL, NULL, NULL, 0 | ETH, NULL, "ZTE MF667 (modem)"}, // qmi tested - failed
//      {0x19d2, 0x1408, NULL, NULL, NULL, 0 | ETH, NULL, "ZTE MF825A (modem)"},        // qmi tested - failed
	{0x19d2, 0x1420, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF730 (cdrom)"},	//
	{0x19d2, 0x1426, "option", "1", "0", 2 | QMI, NULL, "ZTE MF91D (modem)"},	//
	{0x19d2, 0x1428, "option", "1", "0", 2 | QMI, NULL, "Telewell TW-LTE 4G v2 (modem)"},	//
	{0x19d2, 0x1511, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MFxxx (cdrom)"},	//
	{0x19d2, 0x1512, "option", "0", "0", 2 | ACM, NULL, "ZTE MFxxx (modem)"},	// also cdc_ether
	{0x19d2, 0x1514, "option", "0", "0", 2, &modeswitch_onda, "ZTE MF192 (cdrom)"},	//
	{0x19d2, 0x1515, "option", "0", "0", 2 | ACM, NULL, "ZTE MF192 (modem)"},	//
	{0x19d2, 0x1517, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF192 (cdrom)"},	//
	{0x19d2, 0x1518, "option", "0", "0", 2 | ACM, NULL, "ZTE MF192 (modem)"},	//
	{0x19d2, 0x1519, "option", "0", "0", 2 | ACM, NULL, "ZTE MF192 (modem)"},	//
	{0x19d2, 0x1520, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF652 (cdrom)"},	//
	{0x19d2, 0x1522, "option", "0", "0", 2 | ACM, NULL, "ZTE MF652 (modem)"},	//
	{0x19d2, 0x1523, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF591 (cdrom)"},	//
	{0x19d2, 0x1525, "option", "0", "0", 2 | ACM, NULL, "ZTE MF591 (modem)"},	//
	{0x19d2, 0x1527, "option", "0", "0", 2 | ACM, NULL, "ZTE MF196 (modem)"},	//
	{0x19d2, 0x1528, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF196 (cdrom)"},	//
	{0x19d2, 0x1529, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MFxxx (cdrom)"},	//
	{0x19d2, 0x1530, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MFxxx (cdrom)"},	//
	{0x19d2, 0x1536, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF190J (cdrom)"},	//
	{0x19d2, 0x1537, "option", "0", "0", 2 | ACM, NULL, "ZTE MF190J (modem)"},	// also cdc_ether
	{0x19d2, 0x1538, "option", "0", "0", 2 | ACM, NULL, "ZTE MF190J (modem)"},	// also cdc_ether
	{0x19d2, 0x1542, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF190J (cdrom)"},	//
	{0x19d2, 0x1544, "option", "0", "0", 2 | ACM, NULL, "ZTE MF190J (modem)"},	//
	{0x19d2, 0x1588, "option", "0", "0", 2, &modeswitch_zte, "ZTE MF710 (cdrom)"},	//
	{0x19d2, 0x1589, "option", "3", "2", 2, NULL, "ZTE MF710 (modem)"},	// also cdc_ether
	{0x19d2, 0x2000, "option", "0", "0", 2, &modeswitch_zte, "ONDA/ZTE (cdrom)"},	//
	{0x19d2, 0x2002, "option", "1", "3", 2 | QMI, NULL, "ZTE K3765-Z (modem)"},	//
	{0x19d2, 0x2003, "option", "1", "3", 2, NULL, "ZTE MF180 (modem)"},	//
	{0x19d2, 0x2004, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF60 (cdrom)"},	//
	{0x19d2, 0xffdd, "option", "1", "0", 2 | GENERIC, NULL, "ZTE AC682 (modem)"},	//
	{0x19d2, 0xffde, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE AC682 (cdrom)"},	//
	{0x19d2, 0xffe4, "option", "1", "0", 2 | GENERIC, NULL, "ZTE AC3781 (modem)"},	//
	{0x19d2, 0xffe9, "option", "1", "0", 2, NULL, "ZTE AC2738 (modem)"},	//
	{0x19d2, 0xfff1, "option", "1", "0", 2, NULL, "ZTE generic (modem)"},	//
	{0x19d2, 0xfff5, "option", "0", "0", 2, &modeswitch_others, "ZTE generic (cdrom)"},	//
	{0x19d2, 0xfff6, "option", "0", "0", 2, &modeswitch_others, "ZTE generic (cdrom)"},	//
	{0x19d2, 0xfffb, "option", "1", "0", 2, NULL, "ZTE MG880 (modem)"},	//
	{0x19d2, 0xfffc, "option", "1", "0", 2, NULL, "ZTE MG880 (modem)"},	//
	{0x19d2, 0xfffd, "option", "1", "0", 2, NULL, "ZTE MG880 (modem)"},	//
	{0x19d2, 0xfffe, "option", "1", "0", 2, NULL, "ZTE AC8700 (modem)"},	//
	{0x19d2, 0xffff, "option", "1", "0", 2, NULL, "ZTE AC8710 (modem)"},	//

// Infomark
//      {0x19f2, 0x1700, "option", "0", "0", 0 | ETH, NULL, "Clear Spot Voyager mifi"}, //

// No name yet
	{0x19f5, 0x9905, "option", "2", "1", 2 | GENERIC, NULL, "Venus Fast2 (modem)"},	//
	{0x19f5, 0x9909, "option", "2", "1", 2 | GENERIC, NULL, "Venus Fast2 (modem)"},	//
	{0x19f5, 0xf000, "option", "0", "0", 2, &modeswitch_std_eject, "Advan Jetx DT-8 (cdrom)"},	//

// Bandrich
	{0x1a8d, 0x1000, "option", "0", "0", 2, &modeswitch_std_eject, "Bandrich C-1xx/C-270/C-32x (cdrom)"},	//
//      {0x1a8d, 0x1001, "option", "1", NULL, 2 | ETH, NULL, "Bandrich C-100/C-120 (netif)"},   //
	{0x1a8d, 0x1002, "option", "1", "0", 2, NULL, "Bandrich C-100/C-120 (modem)"},	//
	{0x1a8d, 0x1003, "option", "1", "0", 2, NULL, "Bandrich C-100/C-120 (modem)"},	//
	{0x1a8d, 0x1007, "option", "2", "0", 2, NULL, "Bandrich C-270 (modem)"},	//
	{0x1a8d, 0x1008, "option", "2", "0", 2, NULL, "Bandrich M250 (modem)"},	//
	{0x1a8d, 0x1009, "option", "2", "0", 2, NULL, "Bandrich C-170/C-180 (modem)"},	//
	{0x1a8d, 0x100c, "option", "2", "0", 2, NULL, "Bandrich C-320 (modem)"},	// also cdc_ether
	{0x1a8d, 0x100d, "option", "2", "0", 2, NULL, "Bandrich C-508 (modem)"},	// also cdc_ether
	{0x1a8d, 0x2000, "option", "0", "0", 2, &modeswitch_std_eject, "Bandrich C33x (cdrom)"},	//
	{0x1a8d, 0x2006, "option", "1", "1", 2 | ACM, NULL, "Bandrich C-33x (modem)"},	//

// Datang
	{0x1ab7, 0x2000, "option", "0", "0", 2 | GENERIC, NULL, "Aircard 901 (modem)"},	//
	{0x1ab7, 0x5700, "option", "0", "0", 2, &modeswitch_std_eject, "Datang DTM573x (cdrom)"},	//
	{0x1ab7, 0x5730, "option", "3", "1", 2 | GENERIC, NULL, "Datang DTM5730 (modem)"},	//
	{0x1ab7, 0x5731, "option", "3", "1", 2 | GENERIC, NULL, "Datang DTM5731 (modem)"},	//

// T&A Mobile Phones (Alcatel)
	{0x1bbb, 0x0000, "option", "2", "2", 2, NULL, "Alcatel X060S/X070S/X080S/X200 (modem)"},	//
	{0x1bbb, 0x0012, "option", "2", "2", 2 | GENERIC, NULL, "Alcatel X085C (modem)"},	//
	{0x1bbb, 0x0017, "option", "4", "4", 2, NULL, "Alcatel X220L (Variant), X500D (modem)"},	//
	{0x1bbb, 0x0052, "option", "4", "4", 2, NULL, "Alcatel X220L (Variant), (modem)"},	//
	{0x1bbb, 0x00b7, "option", "0", "4", 2, NULL, "Alcatel X600 (modem)"},	//
	{0x1bbb, 0x00ca, "option", "0", "0", 2 | GENERIC, NULL, "Alcatel X080C (modem)"},	//
	{0x1bbb, 0x011e, "option", "1", "2", 2 | QMI, NULL, "Alcatel L100V, (modem)"},	//
//      {0x1bbb, 0x0195, NULL, NULL, NULL, 0 | RNDIS, NULL, "Alcatel L800, (modem)"},   //
	{0x1bbb, 0x0203, "option", "0", "1", 2 | QMI, NULL, "Alcatel L800Z, (modem)"},	//
	{0x1bbb, 0x022c, "option", "0", "1", 2 | GENERIC, &select_config2, "Alcatel X602D (modem)"},	//
	{0x1bbb, 0xf000, "option", "0", "0", 2, &modeswitch_alcatel, "Alcatel X060S/X070S/X080S/X200/X220L/X500D(cdrom)"},	//
	{0x1bbb, 0xf017, "option", "0", "0", 2, &modeswitch_alcatel, "Alcatel X220D (cdrom)"},	//
	{0x1bbb, 0xf052, "option", "0", "0", 2, &modeswitch_alcatel, "Alcatel X220L (cdrom)"},	//
	{0x1bbb, 0xf0b6, "option", "0", "0", 2, &modeswitch_alcatel, "Alcatel X550L (cdrom)"},	//
	{0x1bbb, 0xf0b7, "option", "0", "0", 2, &modeswitch_alcatel, "Alcatel X600L (cdrom)"},	//

// Telit
	{0x1bc7, 0x0021, "option", "0", "0", 2 | ACM, NULL, "Telit HE910, (modem)"},	//
	{0x1bc7, 0x1201, NULL, NULL, NULL, 0 | QMI, NULL, "Telit LE910, (modem)"},	//

// Longcheer
	{0x1c9e, 0x1001, "option", "0", "0", 2, &modeswitch_alcatel, "Alcatel X020 & X030 (cdrom)"},	//
	{0x1c9e, 0x3197, "option", "1", "0", 2 | GENERIC, NULL, "SEV759 (modem)"},	//
	{0x1c9e, 0x6000, "option", "2", "0", 2 | GENERIC, &modeswitch_rezero, "Alcatel X020 & X030 (modem)"},	//
	{0x1c9e, 0x6060, "option", "2", "0", 2 | GENERIC, &modeswitch_rezero, "TU930"},	//
	{0x1c9e, 0x6061, "option", "2", "0", 2 | GENERIC, NULL, "Alcatel X020 & X030 (modem)"},	//
	{0x1c9e, 0x9000, "option", "1", "2", 2 | GENERIC, NULL, "4G Systems XS Stick W14 (modem)"},	//
	{0x1c9e, 0x9101, "option", "0", "0", 2, &modeswitch_alcatel, "EMobile D12LC (cdrom)"},	//
	{0x1c9e, 0x9104, "option", "1", "2", 2 | GENERIC, NULL, "EMobile D12LC(modem)"},	//
	{0x1c9e, 0x9401, "option", "0", "0", 2, &modeswitch_alcatel, "EMobile D21LC (cdrom)"},	//
	{0x1c9e, 0x9404, "option", "1", "2", 2 | GENERIC, NULL, "EMobile D21LC(modem)"},	//
	{0x1c9e, 0x9603, "option", "1", "2", 2, NULL, "4G Systems XS Stick W14 (modem)"},	//
	{0x1c9e, 0x9605, "option", "1", "3", 2, NULL, "4G Systems XS Stick W14 (modem)"},	//
	{0x1c9e, 0x9607, "option", "1", "3", 2, NULL, "4G Systems XS Stick W14 (modem)"},	//
	{0x1c9e, 0x9800, "option", "2", "1", 2 | GENERIC, &modeswitch_alcatel, "SU-9800 (modem)"},	//
	{0x1c9e, 0x9801, "option", "2", "1", 2 | GENERIC, NULL, "4G Systems XS Stick W21 (modem)"},	//also qmi but not yet in driver
	{0x1c9e, 0x9803, "option", "2", "1", 2 | GENERIC, NULL, "SmartBro WM66E (modem)"},	//also qmi but not yet in driver
	{0x1c9e, 0x98ff, "option", "0", "0", 2, &modeswitch_alcatel, "4G Systems XS Stick W21 (cdrom)"},	//
	{0x1c9e, 0x9900, "option", "1", "2", 2 | GENERIC, NULL, "Softbank C02LC (modem)"},	//
	{0x1c9e, 0x9a00, "option", "2", "0", 2 | GENERIC, NULL, "4G Systems XS Stick TV (modem)"},	//
	{0x1c9e, 0x9d00, "option", "2", "3", 2 | GENERIC, &modeswitch_alcatel, "Prolink PCM100 (cdrom)"},	//
	{0x1c9e, 0x9e00, "option", "2", "0", 2 | GENERIC, &modeswitch_alcatel, "MMX 310C"},	//
	{0x1c9e, 0xf000, "option", "0", "0", 2, &modeswitch_alcatel, "4G Systems XS Stick W14 (cdrom)"},	//
	{0x1c9e, 0xf001, "option", "0", "0", 2, &modeswitch_alcatel, "Alcatel AD110 (cdrom)"},	//

// TechFaith
	{0x1d09, 0x1000, "option", "0", "0", 2, &modeswitch_std_eject, "Techfaith (cdrom)"},	//
	{0x1d09, 0x1010, "option", "2", "0", 2 | GENERIC, NULL, "Aiko 81D (modem)"},	//
	{0x1d09, 0x1021, "option", "0", "0", 2, &modeswitch_std_eject, "Aiko 81D (cdrom)"},	//
	{0x1d09, 0x1025, "option", "0", "0", 2, &modeswitch_std_eject, "TechFaith FlyingLARK46 (cdrom)"},	//
	{0x1d09, 0x1026, "option", "1", "2", 2 | GENERIC, NULL, "TechFaith FlyingLARK46 (modem)"},	//
	{0x1d09, 0x4306, "option", "2", "0", 2 | GENERIC, NULL, "TechFaith Venus VT18 (modem)"},	//

// Wisue
	{0x1dbc, 0x0005, "option", "0", "0", 2 | ACM, NULL, "Vodafone MD950 (modem)"},	//
	{0x1dbc, 0x0669, "option", "0", "0", 2 | ACM, &select_config2, "Wisue W340 (modem)"},	//
	{0x1dbc, 0x8005, "option", "0", "0", 2 | ACM, NULL, "EDGE Modem (modem)"},	//

// Qualcomm /ALink /Hyundai
	{0x1e0e, 0x9000, "option", "1", "2", 3, NULL, "PROLink PHS100, Hyundai MB-810, A-Link 3GU (modem)"},	//
	{0x1e0e, 0x9100, "option", "1", "2", 3, NULL, "PROLink PHS300, A-Link 3GU (modem)"},	//
	{0x1e0e, 0x9200, "option", "1", "2", 3, NULL, "PROLink PHS100, Hyundai MB-810, A-Link 3GU (modem)"},	//
	{0x1e0e, 0xce16, "option", "1", "2", 3, NULL, "D-Link DWM-162U5, Micromax MMX 300c (modem)"},	//
	{0x1e0e, 0xce17, "option", "1", "0", 3 | GENERIC, NULL, "D-Link DWM-162 C1 (modem)"},	//
	{0x1e0e, 0xce1e, "option", "1", "2", 3, NULL, "D-Link DWM-162U5 A1 (modem)"},	//
	{0x1e0e, 0xce28, "option", "1", "2", 3 | GENERIC, NULL, "SpeedUP SU-7000U (modem)"},	//
	{0x1e0e, 0xcefe, "option", "1", "2", 3 | GENERIC, NULL, "Simcom EM600, Micromax MMX 300c (modem)"},	//
	{0x1e0e, 0xf000, "option", "0", "0", 3, &modeswitch_others, "PROLink PHS100, Hyundai MB-810, A-Link 3GU (cdrom)"},	//

// SelectWireless
	{0x1edf, 0x6003, "option", "0", "0", 2 | ACM, &select_config2, "AirPlus MCD-800"},	//
	{0x1edf, 0x6004, "option", "1", "0", 2 | ACM, NULL, "AirPlus MCD-640/650"},	//

// Onda
	{0x1ee8, 0x0003, "option", "0", "0", 2, &modeswitch_onda, "Onda MV815UP (cdrom)"},	//
	{0x1ee8, 0x0004, "option", "1", "0", 2 | ACM, NULL, "Onda MV815UP (modem)"},	//
	{0x1ee8, 0x0009, "option", "0", "0", 2, &modeswitch_onda, "Onda MW823UP (cdrom)"},	//
	{0x1ee8, 0x000b, "option", "1", "0", 2 | ACM, NULL, "Onda MW823UP (modem)"},	//
	{0x1ee8, 0x0011, "option", "1", "0", 2 | ACM, NULL, "Onda MDC835UP (modem)"},	//
	{0x1ee8, 0x0012, "option", "1", "0", 2 | ACM, NULL, "Onda MW833UP (modem)"},	//
	{0x1ee8, 0x0013, "option", "0", "0", 2, &modeswitch_onda, "Onda MW833UP/MT835UP (cdrom)"},	//
	{0x1ee8, 0x0014, "option", "1", "0", 2 | ACM, NULL, "Onda MT835UP (modem)"},	//
	{0x1ee8, 0x0017, "option", "1", "0", 2 | ACM, NULL, "Onda MO835UP (modem)"},	//
	{0x1ee8, 0x0018, "option", "0", "0", 2, &modeswitch_onda, "Onda MO835UP (cdrom)"},	//
	{0x1ee8, 0x003e, "option", "1", "0", 2 | ACM, NULL, "Onda MW836UP (modem)"},	//
	{0x1ee8, 0x0040, "option", "0", "0", 2, &modeswitch_onda, "Onda MW836UP (cdrom)"},	//
	{0x1ee8, 0x0044, "option", "1", "0", 2 | ACM, NULL, "Onda MDC655 (modem)"},	//
	{0x1ee8, 0x0045, "option", "0", "0", 2, &modeswitch_onda, "Onda MDC655 (cdrom)"},	//
	{0x1ee8, 0x0049, "option", "1", "0", 2 | ACM, NULL, "Onda MDC655 (modem)"},	//
	{0x1ee8, 0x004a, "option", "0", "0", 2, &modeswitch_onda, "Onda MDC655 (cdrom)"},	//
	{0x1ee8, 0x004e, "option", "1", "0", 2 | ACM, NULL, "Onda MDC655 (modem)"},	//
	{0x1ee8, 0x004f, "option", "0", "0", 2, &modeswitch_onda, "Onda MDC655 (cdrom)"},	//
	{0x1ee8, 0x0053, "option", "1", "0", 2 | ACM, NULL, "Onda MW875UP (modem)"},	//
	{0x1ee8, 0x0054, "option", "0", "0", 2, &modeswitch_onda, "Onda MW875UP (cdrom)"},	//
	{0x1ee8, 0x0063, "option", "0", "0", 2, &modeswitch_onda, "Onda TM201 (cdrom)"},	//
	{0x1ee8, 0x0064, "option", "1", "0", 2 | ACM, NULL, "Onda TM201 (modem)"},	//

// Franklin Wireless
	{0x1fac, 0x0032, "option", "0", "0", 2 | ACM, &select_config2, "Franklin U600 (modem)"},	//
	{0x1fac, 0x0150, "option", "0", "0", 2, &modeswitch_std_eject, "Franklin U600 (cdrom)"},	//
	{0x1fac, 0x0151, "option", "0", "0", 2 | ACM, NULL, "Franklin U600 (modem)"},	//
//      {0x1fac, 0x0232, NULL, NULL, NULL, 0 | RNDIS, NULL, "Franklin U770 (modem)"},   //

// D-Link (2nd VID)
	{0x2001, 0x00a6, "option", "0", "0", 2, &modeswitch_std_eject, "D-Link DWM-157 B1 (cdrom)"},	// also cdc_mbim
	{0x2001, 0x7901, "option", "1", "3", 2 | GENERIC, NULL, "D-Link DWM-157 A1 (modem)"},	//
	{0x2001, 0x7d00, "option", "1", "0", 2 | GENERIC, NULL, "D-Link DWM-156 A6 (modem)"},	//
	{0x2001, 0x7d01, "option", "3", "2", 2, NULL, "D-Link DWM-156 A7 (modem)"},	//also mbim
	{0x2001, 0x7d02, "option", "0", "0", 2, NULL, "D-Link DWM-157 B1 (modem)"},	//also mbim
	{0x2001, 0x7d03, "option", "0", "0", 2, NULL, "D-Link DWM-158 D1 (modem)"},	//also mbim
	{0x2001, 0x7d0b, "option", "3", "2", 2 | GENERIC, NULL, "D-Link DWM-156 A8 (modem)"},	//
	{0x2001, 0x7d0d, "option", "3", "2", 2 | GENERIC, NULL, "D-Link DWM-167 A1 (modem)"},	//
	{0x2001, 0x98ff, "option", "0", "0", 2, &modeswitch_alcatel, "D-Link DWM-221 (cdrom)"},	//
	{0x2001, 0xa403, "option", "0", "0", 2, &modeswitch_std_eject, "D-Link DWM-156 A8 (cdrom)"},	//
	{0x2001, 0xa405, "option", "0", "0", 2, &modeswitch_std_eject, "D-Link DWM-167 A1 (cdrom)"},	//
	{0x2001, 0xa706, "option", "0", "0", 2, &modeswitch_std_eject, "D-Link DWM-156 A7 (cdrom)"},	// also cdc_mbim
	{0x2001, 0xa707, "option", "0", "0", 2, &modeswitch_std_eject, "D-Link DWM-157 B1 (cdrom)"},	// also cdc_mbim
	{0x2001, 0xa708, "option", "0", "0", 2, &modeswitch_std_eject, "D-Link DWM-158 D1 (cdrom)"},	// also cdc_mbim
	{0x2001, 0xa809, "option", "0", "0", 2, &modeswitch_std_eject, "D-Link DWM-157 A1 (cdrom)"},	//
	{0x2001, 0xa80b, "option", "0", "0", 2, &modeswitch_others, "D-Link DWM-156 A6 (cdrom)"},	//

// Haier
	{0x201e, 0x10f8, "option", "2", "3", 2, NULL, "Haier CE81B"},	//
	{0x201e, 0x2009, "option", "2", "0", 2, &modeswitch_std_eject, "Haier CE100/CE210"},	//

// VisionTek
	{0x2020, 0x0002, "option", "0", "0", 2, &modeswitch_others, "MicroMax MMX 377G (cdrom)"},	//
	{0x2020, 0x1005, "option", "1", "3", 2 | GENERIC, NULL, "SpeedUp SU-8000U (modem)"},	//
	{0x2020, 0x1008, "option", "1", "3", 2 | GENERIC, NULL, "SpeedUp SU-9300U (modem)"},	//
	{0x2020, 0x1012, "option", "1", "3", 2 | GENERIC, NULL, "Prolink PHS100 (modem)"},	//
	{0x2020, 0x2000, "option", "1", "0", 2 | GENERIC, NULL, "Beetel BG64 (modem)"},	//
	{0x2020, 0x4000, "option", "1", "0", 2 | GENERIC, NULL, "Rostelecom Sense R41 (modem)"},	// also qmi but not yet in driver
	{0x2020, 0x4010, "option", "1", "0", 2 | GENERIC, NULL, "MicroMax MMX 377G (modem)"},	// also qmi but not yet in driver
	{0x2020, 0xf00e, "option", "0", "0", 2, &modeswitch_std_eject, "SpeedUp SU-8000 (cdrom)"},	//
	{0x2020, 0xf00f, "option", "0", "0", 2, &modeswitch_std_eject, "SpeedUp SU-8000U (cdrom)"},	//

// ChangHong
	{0x2077, 0x1000, "option", "0", "0", 2, &modeswitch_std_eject, "Axesstel MV242 (cdrom)"},	//
	{0x2077, 0x7001, "option", "1", "0", 2, NULL, "ChangHong CH690 (modem)"},	//
	{0x2077, 0x7010, "option", "1", "0", 2 | GENERIC, NULL, "D-Link DWM-163 (modem)"},	//
	{0x2077, 0x7011, "option", "1", "0", 2 | GENERIC, NULL, "D-Link DWM-168 (modem)"},	//
	{0x2077, 0x8000, "option", "1", "0", 2 | GENERIC, NULL, "Axesstel MV242 (modem)"},	//
	{0x2077, 0x9062, "option", "1", "3", 2 | GENERIC, NULL, "D-Link DWM-155 (modem)"},	//
	{0x2077, 0x9000, "option", "1", "2", 2 | GENERIC, NULL, "Nucom W-160 (modem)"},	//
	{0x2077, 0xa000, "option", "1", "2", 2 | GENERIC, NULL, "Nucom W-260 (modem)"},	//
	{0x2077, 0xa003, "option", "1", "2", 2 | GENERIC, NULL, "Netgear AC327U (modem)"},	//
	{0x2077, 0xf000, "option", "0", "0", 2, &modeswitch_std_eject, "ChangHong CH690 (cdrom)"},	//

// Puchuang
	{0x20a6, 0x1000, "option", "0", "0", 2, &modeswitch_std_eject, "E003 (cdrom)"},	//
	{0x20a6, 0x1105, "option", "2", "0", 2 | GENERIC, NULL, "Intex 3.5G (modem)"},	//
	{0x20a6, 0x1106, "option", "2", "0", 2 | GENERIC, NULL, "Haier TE W130 (modem)"},	//
	{0x20a6, 0xf00a, "option", "0", "0", 2, &modeswitch_std_eject, "E003 (cdrom)"},	//
	{0x20a6, 0xf00e, "option", "0", "0", 2, &modeswitch_std_eject, "Intex 3.5G (cdrom)"},	//

// Tlaytech
	{0x20b9, 0x1682, "option", "0", "0", 2, &modeswitch_std_eject, "TEU800"},	//

// CELOT Corporation
	{0x211f, 0x6801, "option", "2", "0", 2, NULL, "Celot K-3000/CT-650/CT-680 (modem)"},	//

// StrongRising
	{0x21f5, 0x1000, "option", "0", "0", 2, &modeswitch_std_eject, "StrongRising (cdrom)"},	//
	{0x21f5, 0x2008, "option", "3", "0", 2 | GENERIC, NULL, "Flash SX0301 (modem)"},	//
	{0x21f5, 0x2012, "option", "3", "0", 2 | GENERIC, NULL, "MU290 (modem)"},	//
	{0x21f5, 0x3010, "option", "0", "0", 2, &modeswitch_std_eject, "STD808 (cdrom)"},	//

// Linktop
	{0x230d, 0x0001, "option", "0", "1", 0 | ACM, &modeswitch_linktop, "Linktop LW27x (BSNL 3G)"},	//
	{0x230d, 0x0003, "option", "0", "1", 0 | ACM, &modeswitch_linktop, "Linktop LW27x (Teracom 3G)"},	//
	{0x230d, 0x0007, "option", "0", "1", 0 | ACM, &modeswitch_linktop, "Linktop LW27x (Visiontek 3G)"},	//
	{0x230d, 0x000b, "option", "0", "1", 0 | ACM, &modeswitch_linktop, "Zoom 3G"},	//
	{0x230d, 0x000d, "option", "0", "1", 0 | ACM, &modeswitch_linktop, "Intex Speed 3G v7.2"},	//
	{0x230d, 0x0101, "option", "0", "1", 0 | ACM, &select_config2, "Linktop LW27x (BSNL 3G)"},	//
	{0x230d, 0x0103, "option", "0", "1", 0 | ACM, &select_config2, "Linktop LW27x (Teracom 3G)"},	//

// TP-Link
	{0x2357, 0x0200, "option", "0", "0", 2, &modeswitch_std_eject, "TP-Link MA180 (cdrom)"},	//
	{0x2357, 0x0201, "option", "1", "2", 2 | QMI, NULL, "TP-Link MA180 (modem)"},	//
	{0x2357, 0x0202, "option", "1", "2", 2 | QMI, NULL, "TP-Link MA180 (modem)"},	//
	{0x2357, 0x0203, "option", "1", "2", 2 | QMI, NULL, "TP-Link MA180 (modem)"},	//
	{0x2357, 0x9000, "option", "1", "2", 2 | QMI, NULL, "TP-Link MA260 (modem)"},	//
	{0x2357, 0xf000, "option", "0", "0", 2, &modeswitch_std_eject, "TP-Link MA260 (cdrom)"},	//

// Unknown mfgr
	{0x23a2, 0x1010, "option", "0", "0", 2, &modeswitch_std_eject, "Titan 3.5G (cdrom)"},	//
	{0x23a2, 0x1234, "option", "0", "0", 0 | ACM, NULL, "Titan 3.5G"},	//

// Dell
	{0x413c, 0x8114, "option", "1", "0", 2, NULL, "Dell 5700"},	//
	{0x413c, 0x8115, "option", "1", "0", 2, NULL, "Dell 5500"},	//
	{0x413c, 0x8116, "option", "1", "0", 2, NULL, "Dell 5505"},	//
	{0x413c, 0x8117, "option", "1", "0", 2, NULL, "Dell 5700"},	//
	{0x413c, 0x8118, "option", "1", "0", 2, NULL, "Dell 5510"},	//
	{0x413c, 0x8128, "option", "1", "0", 2, NULL, "Dell 5700"},	//
	{0x413c, 0x8129, "option", "1", "0", 2, NULL, "Dell 5700"},	//
	{0x413c, 0x8133, "option", "1", "0", 2, NULL, "Dell 5720"},	//
	{0x413c, 0x8134, "option", "1", "0", 2, NULL, "Dell 5720"},	//
	{0x413c, 0x8135, "option", "1", "0", 2, NULL, "Dell 5720"},	//
	{0x413c, 0x8136, "option", "1", "0", 2, NULL, "Dell 5520"},	//
	{0x413c, 0x8137, "option", "1", "0", 2, NULL, "Dell 5520"},	//
	{0x413c, 0x8138, "option", "1", "0", 2, NULL, "Dell 5520"},	//
	{0x413c, 0x8147, "option", "0", "1", 2 | ACM, NULL, "Dell 5530"},	//
	{0x413c, 0x8180, "option", "1", "0", 2, NULL, "Dell 5730"},	//
	{0x413c, 0x8181, "option", "1", "0", 2, NULL, "Dell 5730"},	//
	{0x413c, 0x8182, "option", "1", "0", 2, NULL, "Dell 5730"},	//
	{0x413c, 0x8186, "option", "1", "0", 2 | QMI, NULL, "Dell 5620"},	//
	{0x413c, 0x8194, "option", "1", "0", 2 | QMI, NULL, "Dell 5630"},	//
	{0x413c, 0x8195, "option", "1", "0", 2 | QMI, NULL, "Dell 5800"},	//
	{0x413c, 0x8196, "option", "1", "0", 2 | QMI, NULL, "Dell 5800v2"},	//
	{0x413c, 0x819b, "option", "1", "0", 2, NULL, "Dell 5804"},	//

	{0xffff, 0xffff, NULL, NULL, NULL, 0, NULL, NULL}	//
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
	int select = atoi(nvram_safe_get("wan_select"));
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

	if (gpio1 == atoi(nvram_safe_get("gpio26"))
	    && gpio2 == atoi(nvram_safe_get("gpio27")))
		needreset = 0;

	if (gpio1) {
		nvram_set("gpio26", "1");
		set_gpio(26, 1);
	} else {
		nvram_set("gpio26", "0");
		set_gpio(26, 0);
	}
	if (gpio2) {
		nvram_set("gpio27", "1");
		set_gpio(27, 1);
	} else {
		nvram_set("gpio27", "0");
		set_gpio(27, 0);
	}
#endif
	nvram_unset("3gnmvariant");
//      nvram_set("3gdata", "/dev/usb/tts/0");  // crap

	int devicecount = 0;
	while (devicelist[devicecount].vendor != 0xffff) {
		if (scanFor(devicelist[devicecount].vendor, devicelist[devicecount].product)) {
			fprintf(stderr, "%s detected\n", devicelist[devicecount].name);

#if defined(HAVE_LIBQMI) || defined(HAVE_UQMI)
			if ((devicelist[devicecount].modeswitch & QMI)) {
				insmod("cdc-wdm usbnet qmi_wwan");
				//start custom setup, if defined
				if (devicelist[devicecount].customsetup) {
					fprintf(stderr, "customsetup QMI\n");
					devicelist[devicecount].customsetup(needreset, devicecount);
					sleep(2);
				}
				sprintf(control, "qmi");
				nvram_set("3gdata", "qmi");
				return control;
			}
#endif
			if (devicelist[devicecount].driver) {
				insmod("usbserial usb_wwan cdc-wdm usbnet qmi_wwan");
				insmod(devicelist[devicecount].driver);
			}
			if (devicelist[devicecount].datadevice) {
				if (!strcmp(devicelist[devicecount].datadevice, "hso"))
					sprintf(data, "hso");
				else {
					if ((devicelist[devicecount].modeswitch & ACM)) {
						insmod("cdc-acm");
						sprintf(data, "/dev/ttyACM%s", devicelist[devicecount].datadevice);
					} else if ((devicelist[devicecount].modeswitch & GENERIC)) {
						sysprintf("echo %04x %04x > /sys/bus/usb-serial/drivers/option1/new_id", devicelist[devicecount].vendor, devicelist[devicecount].product);
						insmod("usb_wwan cdc-wdm usbnet qmi_wwan");
						sprintf(data, "/dev/usb/tts/%s", devicelist[devicecount].datadevice);
					} else
						sprintf(data, "/dev/usb/tts/%s", devicelist[devicecount].datadevice);

				}
				nvram_set("3gdata", data);
			}
			if (devicelist[devicecount].modeswitch & 0x0f) {
				char variant[32];
				sprintf(variant, "%d", devicelist[devicecount].modeswitch & 0x0f);
				nvram_set("3gnmvariant", variant);
			}
			//start custom setup, if defined
			if (devicelist[devicecount].customsetup) {
				fprintf(stderr, "customsetup\n");
				devicelist[devicecount].customsetup(needreset, devicecount);
				sleep(2);
			}
			if (!strcmp(devicelist[devicecount].controldevice, "hso")) {
				insmod("hso");
				sprintf(control, "hso");
				nvram_set("3gdata", "hso");
				FILE *out = fopen("/tmp/conninfo.ini", "wb");
				fprintf(out, "APN=%s\n", nvram_safe_get("wan_apn"));
				fprintf(out, "USER=%s\n", nvram_safe_get("ppp_username"));
				fprintf(out, "PASS=%s\n", nvram_safe_get("ppp_passwd"));
				fprintf(out, "PIN=%s\n", nvram_safe_get("wan_pin"));
				fclose(out);
				eval("/etc/hso/hso_connect.sh", "restart");
			} else if ((devicelist[devicecount].modeswitch & ACM)) {
				insmod("cdc-acm");
				sprintf(control, "/dev/ttyACM%s", devicelist[devicecount].controldevice);
			} else if ((devicelist[devicecount].modeswitch & GENERIC)) {
				sysprintf("echo %04x %04x > /sys/bus/usb-serial/drivers/option1/new_id", devicelist[devicecount].vendor, devicelist[devicecount].product);
				insmod("usb_wwan cdc-wdm usbnet qmi_wwan");
				sprintf(control, "/dev/usb/tts/%s", devicelist[devicecount].controldevice);
			} else
				sprintf(control, "/dev/usb/tts/%s", devicelist[devicecount].controldevice);
			return control;
		}
		devicecount++;
	}
	//not found, use generic implementation (all drivers)
	insmod("cdc-acm cdc-wdm usbnet qmi_wwan usbserial usb_wwan sierra option");
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
