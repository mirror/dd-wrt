/*
 * lib3g.c
 *
 * Copyright (C) 2009 Sebastian Gottschall <gottschall@dd-wrt.com>
 *  			2012 - 2013 LOM
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
	FILE *check = popen(grepstr, "rb");
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
	sysprintf("comgt -d /dev/usb/tts/%s -s /etc/comgt/reset.comgt", tty);
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
	sysprintf("comgt -d /dev/usb/tts/%s -s /etc/comgt/wakeup.comgt", tty);
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

/* Expose all will enable all interfaces available under the current vid:pid */
/* It is used for older Huawei devices, for Option devices, and a few devices with a generic Qualcom id */
static void modeswitch_expose_all(int needreset, int devicecount)
{
	FILE *out;
	out = fopen("/tmp/usb_modeswitch.conf", "wb");
	fprintf(out, "DefaultVendor=0x%04x\n", devicelist[devicecount].vendor);
	fprintf(out, "DefaultProduct=0x%04x\n", devicelist[devicecount].product);
	fprintf(out, "MessageContent=\"55534243785634120100000080000601000000000000000000000000000000\"\n");
	fclose(out);
	system("usb_modeswitch -I -c /tmp/usb_modeswitch.conf");

	sleep(5);
}

static void select_config2(int needreset, int devicecount)
{
	FILE *out;
	out = fopen("/tmp/usb_modeswitch.conf", "wb");
	fprintf(out, "DefaultVendor=0x%04x\n", devicelist[devicecount].vendor);
	fprintf(out, "DefaultProduct=0x%04x\n", devicelist[devicecount].product);
	fprintf(out, "Configuration=2\n");
	fclose(out);
	system("usb_modeswitch -I -c /tmp/usb_modeswitch.conf");

	sleep(2);
}

static void select_config3(int needreset, int devicecount)
{
	FILE *out;
	out = fopen("/tmp/usb_modeswitch.conf", "wb");
	fprintf(out, "DefaultVendor=0x%04x\n", devicelist[devicecount].vendor);
	fprintf(out, "DefaultProduct=0x%04x\n", devicelist[devicecount].product);
	fprintf(out, "Configuration=3\n");
	fclose(out);
	system("usb_modeswitch -I -c /tmp/usb_modeswitch.conf");

	sleep(2);
}

/* std eject is "allow media change" + "stop unit" */
static void modeswitch_std_eject(int needreset, int devicecount)
{
	FILE *out;
	out = fopen("/tmp/usb_modeswitch.conf", "wb");
	fprintf(out, "DefaultVendor=0x%04x\n", devicelist[devicecount].vendor);
	fprintf(out, "DefaultProduct=0x%04x\n", devicelist[devicecount].product);
	fprintf(out, "MessageContent=\"5553424312345678000000000000061e000000000000000000000000000000\"\n");
	fprintf(out, "MessageContent2=\"5553424312345679000000000000061b000000020000000000000000000000\"\n");
	fprintf(out, "NeedResponse=1\n");
	fclose(out);
	system("usb_modeswitch -I -c /tmp/usb_modeswitch.conf");

	sleep(2);
}

static void modeswitch_pantech(int needreset, int devicecount)
{
	system("usb_modeswitch -v 0x106c -p 0x3b03 -I -M 555342431234567824000000800008ff024445564348470000000000000000");
	system("usb_modeswitch -v 0x106c -p 0x3b05 -I -M 555342431234567824000000800008ff020000000000000000000000000000");
	system("usb_modeswitch -v 0x106c -p 0x3b06 -I -M 555342431234567824000000800008ff020000000000000000000000000000");
	system("usb_modeswitch -v 0x106c -p 0x3b11 -I -M 555342431234567824000000800008ff024445564348470000000000000000");

	sleep(2);
}

static void modeswitch_sierra(int needreset, int devicecount)
{
	FILE *out;
	out = fopen("/tmp/usb_modeswitch.conf", "wb");
	fprintf(out, "DefaultVendor=0x%04x\n", devicelist[devicecount].vendor);
	fprintf(out, "DefaultProduct=0x%04x\n", devicelist[devicecount].product);
	fprintf(out,
		"TargetProductList=\"0017,0018,0019,0020,0021,0022,0024,0026,0027,0028,0112,0120,0218,0220,0224,6802,6803,6804,6805,6808,6809,6812,6813,6815,6816,6820,6821,6822,6832,6833,6834,6835,6838,6839,683a,683b,683c,683d,683e,6850,6851,6852,6853,6855,6856,6859,685a,6880,6890,6893,68a3,68aa,9011,9012\"\n");
	fprintf(out, "SierraMode=1\n");
	fprintf(out, "CheckSuccess=10\n");
	fclose(out);
	system("usb_modeswitch -I -c /tmp/usb_modeswitch.conf");

	sleep(2);
}

static void modeswitch_huawei_std(int needreset, int devicecount)
{
	FILE *out;
	out = fopen("/tmp/usb_modeswitch.conf", "wb");
	fprintf(out, "DefaultVendor=0x%04x\n", devicelist[devicecount].vendor);
	fprintf(out, "DefaultProduct=0x%04x\n", devicelist[devicecount].product);
	fprintf(out, "MessageContent=\"55534243123456780000000000000011062000000101000100000000000000\"\n");
	fclose(out);
	system("usb_modeswitch -I -c /tmp/usb_modeswitch.conf");

	sleep(2);
}

static void modeswitch_cmotech(int needreset, int devicecount)
{
	FILE *out;
	out = fopen("/tmp/usb_modeswitch.conf", "wb");
	fprintf(out, "DefaultVendor=0x%04x\n", devicelist[devicecount].vendor);
	fprintf(out, "DefaultProduct=0x%04x\n", devicelist[devicecount].product);
	fprintf(out, "MessageContent=\"555342431234567824000000800008ff524445564348473100000000000000\"\n");
	fclose(out);
	system("usb_modeswitch -I -c /tmp/usb_modeswitch.conf");

	sleep(2);
}

static void modeswitch_zte_3msg(int needreset, int devicecount)
{
	FILE *out;
	out = fopen("/tmp/usb_modeswitch.conf", "wb");
	fprintf(out, "DefaultVendor=0x%04x\n", devicelist[devicecount].vendor);
	fprintf(out, "DefaultProduct=0x%04x\n", devicelist[devicecount].product);
	fprintf(out,
		"MessageContent=\"5553424312345678000000000000061e000000000000000000000000000000\"\n"
		"MessageContent2=\"5553424312345679000000000000061b000000020000000000000000000000\"\n" "MessageContent3=\"55534243123456702000000080000c85010101180101010101000000000000\"\n" "NeedResponse=1\n");
	fclose(out);
	system("usb_modeswitch -I -c /tmp/usb_modeswitch.conf");

	sleep(2);
}

static void modeswitch_zte_other(int needreset, int devicecount)
{
	system("usb_modeswitch -v 0x19d2 -p 0x0166 -I -M 55534243123456782400000080000685000000240000000000000000000000");
	system("usb_modeswitch -v 0x19d2 -p 0x0266 -I -M 55534243123456782400000080000685000000240000000000000000000000");
	system("usb_modeswitch -v 0x19d2 -p 0x1514 -I -M 5553424348c4758600000000000010ff000000000000000000000000000000");	// = modeswitch_onda
	system("usb_modeswitch -v 0x19d2 -p 0xfff5 -I -M 5553424312345678c00000008000069f030000000000000000000000000000");
	system("usb_modeswitch -v 0x19d2 -p 0xfff6 -I -M 5553424312345678c00000008000069f030000000000000000000000000000");

	sleep(2);
}

static void modeswitch_bandrich(int needreset, int devicecount)
{
	FILE *out;
	out = fopen("/tmp/usb_modeswitch.conf", "wb");
	fprintf(out, "DefaultVendor=0x%04x\n", devicelist[devicecount].vendor);
	fprintf(out, "DefaultProduct=0x%04x\n", devicelist[devicecount].product);
	fprintf(out, "TargetVendor=0x1a8d\n");
	fprintf(out, "TargetProductList=\"1002,1007,1009,100d,2006\"\n");
	fprintf(out, "MessageContent=\"5553424312345678000000000000061e000000000000000000000000000000\"\n");
	fprintf(out, "MessageContent2=\"5553424312345679000000000000061b000000020000000000000000000000\"\n");
	fprintf(out, "ReleaseDelay=4000\n");
	fprintf(out, "NeedResponse=1\n");
	fclose(out);
	system("usb_modeswitch -I -c /tmp/usb_modeswitch.conf");

	sleep(2);
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
	system("usb_modeswitch -I -c /tmp/usb_modeswitch.conf");

	sleep(2);
}

static void modeswitch_icon210(int needreset, int devicecount)
{
	FILE *out;
	out = fopen("/tmp/usb_modeswitch.conf", "wb");
	fprintf(out, "DefaultVendor=0x1e0e\n");
	fprintf(out, "DefaultProduct=0xf000\n");
	fprintf(out, "TargetVendor=0x1e0e\n");
	fprintf(out, "TargetProductList=\"9200,9000\"\n");
	fprintf(out, "MessageContent=\"555342431234567800000000000006bd000000020000000000000000000000\"\n");
	fprintf(out, "NeedResponse=1\n");
	fclose(out);
	system("usb_modeswitch -I -c /tmp/usb_modeswitch.conf");

	sleep(2);
}

static void modeswitch_linktop(int needreset, int devicecount)
{
	FILE *out;
	out = fopen("/tmp/usb_modeswitch.conf", "wb");
	fprintf(out, "DefaultVendor=0x%04x\n", devicelist[devicecount].vendor);
	fprintf(out, "DefaultProduct=0x%04x\n", devicelist[devicecount].product);
	fprintf(out, "Configuration=3\n");
	fclose(out);
	system("usb_modeswitch -I -c /tmp/usb_modeswitch.conf");

	sleep(2);

	sysprintf("comgt -d /dev/ttyACM0 -s /etc/comgt/wakeup.comgt");
	sleep(2);
}

#define SIERRADIP 0x100		// direct ip sierra / sierra_net
#define QMI 0x80		// cdc_wdm + qmi_wwan
#define ETH 0x40		// usbnet + cdc_ether or usbnet + cdc_ncm
#define GENERIC 0x20		// option new_id on-the-fly
#define ACM 0x10		//cdc_acm
#define MBIM 0x08		//cdc_mbim
// 0-7 is variant type
// 0 =
// 1 = Sierra
// 2 = Huawei
// 3 = Option
// 4 = ZTE

static struct DEVICES devicelist[] = {

//Nokia Mobile Phones
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
	{0x0421, 0x0637, "option", "0", "0", 2, &modeswitch_std_eject, "Nokia 21M-02 (cdrom)"},	//
	{0x0421, 0x0638, "option", "0", "0", 2 | ACM, NULL, "Nokia 21M-02 (modem)"},	//

// Qualcomm
	{0x05c6, 0x0010, "option", "0", "0", 0, &modeswitch_std_eject, "Generic Qualcomm (cdrom)"},	//
	{0x05c6, 0x0023, "option", "2", "0", 0 | GENERIC, NULL, "Leoxsys LN-72V (modem)"},	//
	{0x05c6, 0x00a0, "option", "2", "0", 0 | GENERIC, NULL, "Axesstel MV241 (modem)"},	//
	{0x05c6, 0x1000, "option", "0", "0", 0, &modeswitch_std_eject, "Generic Qualcomm (cdrom)"},	//
	{0x05c6, 0x2001, "option", "0", "0", 0, &modeswitch_std_eject, "Generic Qualcomm (cdrom)"},	//
	{0x05c6, 0x6000, "zte_ev", "2", "0", 0, NULL, "Siemens SG75 (modem)"},	//
	{0x05c6, 0x6503, "option", "0", "0", 0, &modeswitch_std_eject, "Generic Qualcomm (cdrom)"},	//
	{0x05c6, 0x9000, "option", "1", "2", 0, NULL, "Generic Qualcomm (modem)"},	//
	{0x05c6, 0xf000, "option", "0", "0", 0, &modeswitch_std_eject, "Generic Qualcomm (cdrom)"},	//

//D-Link
	{0x07d1, 0x3e01, "option", "1", "0", 2, NULL, "D-Link DWM-152 (modem)"},	//
	{0x07d1, 0x3e02, "option", "1", "0", 2, NULL, "D-Link DWM-156 (modem)"},	//
	{0x07d1, 0x7e11, "option", "1", "2", 2 | GENERIC, NULL, "D-Link DWM-156 (modem)"},	//
	{0x07d1, 0xa800, "option", "0", "0", 0, &modeswitch_std_eject, "D-Link DWM-152/DWM-156 (cdrom)"},	//
	{0x07d1, 0xa804, "option", "0", "0", 0, &modeswitch_std_eject, "D-Link DWM-156 (cdrom)"},	//

//Option
//      {0x0af0, 0x6711, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GE"},   //express card
//      {0x0af0, 0x6731, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GE"},   //express card
//      {0x0af0, 0x6751, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GE"},   //express card
//      {0x0af0, 0x6771, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GE"},   //express card
//      {0x0af0, 0x6791, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GE"},   //express card
	{0x0af0, 0x6901, "option", "1", "0", 0, NULL, "Option GI0201"},	//usb
	{0x0af0, 0x6911, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GI0225"},	//usb
	{0x0af0, 0x6951, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GI0225"},	//usb
	{0x0af0, 0x6971, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GI0225"},	//usb
//      {0x0af0, 0x7011, NULL, "hso", "hso", 2, &modeswitch_expose_all, "Option GE301"},        //express card
//      {0x0af0, 0x7031, NULL, "hso", "hso", 2, &modeswitch_expose_all, "Option GE301"},        //express card
//      {0x0af0, 0x7051, NULL, "hso", "hso", 2, &modeswitch_expose_all, "Option GE301"},        //express card
//      {0x0af0, 0x7071, NULL, "hso", "hso", 2, &modeswitch_expose_all, "Option GE301"},        //express card
	{0x0af0, 0x7111, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GTM"},	//pcie minicard
	{0x0af0, 0x7201, "option", "1", "0", 0, NULL, "Option GTM380"},	//pcie minicard
	{0x0af0, 0x7211, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GTM380"},	//pcie minicard
	{0x0af0, 0x7251, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GTM380"},	//pcie minicard
	{0x0af0, 0x7271, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GTM380"},	//pcie minicard
//      {0x0af0, 0x7301, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GE040x"},       //express card
	{0x0af0, 0x7311, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GTM040x"},	//pcie minicard
//      {0x0af0, 0x7361, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GE044x"},       //express card
//      {0x0af0, 0x7381, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GE044x"},       //express card
	{0x0af0, 0x7401, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GI0401"},	//usb
	{0x0af0, 0x7501, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GI0431"},	//usb
//      {0x0af0, 0x7601, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GE040x"},       //express card
	{0x0af0, 0x7701, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GI0451"},	//usb
	{0x0af0, 0x7706, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GI0451"},	//usb
	{0x0af0, 0x7801, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GI04xx"},	//usb
	{0x0af0, 0x7901, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GI0452"},	//usb
	{0x0af0, 0x7a01, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GI0461"},	//usb
	{0x0af0, 0x7a05, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GI0461"},	//usb
	{0x0af0, 0x8001, NULL, NULL, NULL, 0, &modeswitch_expose_all, "Option GI1515"},	//zero footprint install id
	{0x0af0, 0x8002, NULL, NULL, NULL, 0, &modeswitch_expose_all, "Option GI1515"},	//zero footprint install id
	{0x0af0, 0x8003, NULL, NULL, NULL, 0, &modeswitch_expose_all, "Option GI1515"},	//zero footprint install id
	{0x0af0, 0x8120, "option", "2", "2", 3 | QMI, NULL, "Option GTM681W"},	//pcie minicard
	{0x0af0, 0x8200, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GI07xx"},	//usb
	{0x0af0, 0x8201, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GI07xx"},	//usb
	{0x0af0, 0x8204, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GI07xx"},	//usb
	{0x0af0, 0x8300, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GI033x"},	//usb
	{0x0af0, 0x8302, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GI033x"},	//usb
	{0x0af0, 0x8304, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GI033x"},	//usb
//      {0x0af0, 0x8400, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Pioner JP1"},  //
	{0x0af0, 0x8600, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GI063x"},	//usb
	{0x0af0, 0x8700, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GI0643"},	//usb
//      {0x0af0, 0x8701, NULL, NULL, NULL, 3 | ETH, NULL, "Option GI0643"},     //usb
	{0x0af0, 0x8800, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GTM60x"},	//pcie minicard
	{0x0af0, 0x8900, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GTM67x"},	//pcie minicard
	{0x0af0, 0x9000, NULL, "hso", "hso", 3, &modeswitch_expose_all, "Option GTM66x"},	//pcie minicard
	{0x0af0, 0x9200, "option", "2", "2", 3, NULL, "Option GTM671WFS"},	//pcie minicard
	{0x0af0, 0xc031, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GI031"},	//usb
	{0x0af0, 0xc100, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GI070x"},	//usb
	{0x0af0, 0xd001, NULL, NULL, NULL, 0, &modeswitch_expose_all, "Option GI1515"},	//zero footprint install id
	{0x0af0, 0xd031, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Qualcomm ICON 321"},	//usb
	{0x0af0, 0xd033, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Qualcomm ICON 322"},	//usb
	{0x0af0, 0xd055, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GI0505"},	//usb
	{0x0af0, 0xd057, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GI1505"},	//usb
	{0x0af0, 0xd058, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GI1509"},	//usb
	{0x0af0, 0xd155, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GI0505"},	//usb
	{0x0af0, 0xd157, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GI1515"},	//usb
	{0x0af0, 0xd255, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GI1215"},	//usb
	{0x0af0, 0xd257, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GI1215"},	//usb
	{0x0af0, 0xd357, NULL, "hso", "hso", 0, &modeswitch_expose_all, "Option GI1505"},	//usb

//Olivetti
	{0x0b3c, 0xc003, "option", "0", "4", 2, NULL, "Olivetti Olicard 145"},	//
	{0x0b3c, 0xc004, "option", "0", "4", 2, NULL, "Olivetti Olicard 155"},	//
	{0x0b3c, 0xc700, "option", "0", "0", 0, &modeswitch_std_eject, "Olivetti Olicard 100 (cdrom)"},	//
	{0x0b3c, 0xf000, "option", "0", "0", 0, &modeswitch_alcatel, "Olivetti Olicards (cdrom)"},	//

//Ericsson Business Mobile Networks
	{0x0bdb, 0x1900, "option", "4", "4", 0, NULL, "Ericsson F3507g"},	//
	{0x0bdb, 0x1902, "option", "4", "4", 0, NULL, "Ericsson F3507g"},	//

//Qualcomm (Kyocera)
	{0x0c88, 0x17da, "option", "0", "0", 0, NULL, "Kyocera KPC650"},	//
	{0x0c88, 0x180a, "option", "0", "0", 0, NULL, "Kyocera KPC680"},	//

//AirPrime (Sierra)
	{0x0f3d, 0x68a2, "sierra", "2", "0", 1 | QMI, NULL, "Sierra MC7700 (modem)"},	//
	{0x0f3d, 0x68a3, "sierra", "3", "3", 1, &reset_mc, "Sierra Wireless MC8700/Compass Direct IP"},	//
	{0x0f3d, 0x68aa, "sierra", "3", "3", 1, NULL, "Sierra Wireless AC313U/320U/330U Direct IP"},	//

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
//      {0x1004, 0x61a7, "option", "0", "2", 0, NULL, "LG AD600"},      //ecm able
//      {0x1004, 0x61e6, "option", "0", "2", 0, NULL, "LG SD711"},      //
	{0x1004, 0x61e7, "option", "0", "0", 0, &modeswitch_std_eject, "LG SD711"},	//
//      {0x1004, 0x61ea, "option", "0", "2", 0, NULL, "LG L-08C"},      //
	{0x1004, 0x61eb, "option", "0", "0", 0, &modeswitch_std_eject, "LG L-08C"},	//
	{0x1004, 0x61dd, "option", "0", "0", 0, &modeswitch_std_eject, "LG L-02C"},	//
//      {0x1004, 0x6326, "option", "0", "2", 0, NULL, "LG L-03D"},      //
//      {0x1004, 0x6327, "option", "0", "0", 0, &modeswitch_std_eject, "LG L-03D"},     //

//PANTECH (Curitel)
	{0x106c, 0x3711, "option", "0", "0", 2 | ACM, NULL, "PANTECH UM-150"},	//
	{0x106c, 0x3714, "option", "0", "0", 2 | ACM, NULL, "PANTECH UM-175"},	//
	{0x106c, 0x3715, "option", "0", "0", 2 | ACM, NULL, "PANTECH UM-175AL"},	//
	{0x106c, 0x3716, "option", "0", "0", 2 | ACM, NULL, "PANTECH UM-190"},	//
	{0x106c, 0x3717, "option", "0", "0", 2 | ACM, NULL, "PANTECH UM-185C/UM185E"},	//
	{0x106c, 0x3718, "option", "0", "0", 2 | QMI, NULL, "PANTECH UML-290 4G Modem"},	//
	{0x106c, 0x3b03, "option", "0", "0", 2, &modeswitch_pantech, "PANTECH UM-175AL"},	//
	{0x106c, 0x3b05, "option", "0", "0", 2, &modeswitch_pantech, "PANTECH UM-190"},	//
	{0x106c, 0x3b06, "option", "0", "0", 2, &modeswitch_pantech, "PANTECH UM-185C/UM185E"},	//
	{0x106c, 0x3b11, "option", "0", "0", 2, &modeswitch_pantech, "PANTECH UML-290"},	//

//Sierra Wireless
	{0x1199, 0x0017, "sierra", "3", "4", 1, NULL, "Sierra EM5625 (modem)"},	//
	{0x1199, 0x0018, "sierra", "3", "4", 1, NULL, "Sierra MC5720 (modem)"},	//
	{0x1199, 0x0019, "sierra", "3", "4", 1, NULL, "Sierra AC595U (modem)"},	//
	{0x1199, 0x0020, "sierra", "3", "4", 1, NULL, "Sierra MC5725 (modem)"},	//
	{0x1199, 0x0021, "sierra", "3", "4", 1, NULL, "Sierra AC597E (modem)"},	//
	{0x1199, 0x0022, "sierra", "3", "4", 1, NULL, "Sierra EM5725 (modem)"},	//
	{0x1199, 0x0023, "sierra", "3", "4", 1, NULL, "Sierra AC597 (modem)"},	//
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
	{0x1199, 0x6809, "sierra", "0", "2", 1, NULL, "Sierra MC8755 (modem)"},	//
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
	{0x1199, 0x683c, "sierra", "3", "3", 1, &reset_mc, "Sierra MC8790 Composite"},	//
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
	{0x1199, 0x68a2, "sierra", "2", "0", 1 | QMI, NULL, "Sierra MC7710 (modem)"},	//
	{0x1199, 0x68a3, "sierra", "3", "3", 1, &reset_mc, "Sierra MC8700/Compass Direct IP"},	//
	{0x1199, 0x68aa, "sierra", "3", "3", 1, NULL, "Sierra AC320U/AC330U Direct IP"},	//
	{0x1199, 0x9011, "sierra", "2", "0", 1 | QMI, NULL, "Sierra MC8305 (modem)"},	//
	{0x1199, 0x9012, "sierra", "2", "0", 1 | QMI, NULL, "Sierra MC8355 (modem)"},	//

//Huawei Technologies
	{0x12d1, 0x1001, "option", "2", "0", 2, &modeswitch_expose_all, "HUAWEI/Option E600/E620 or generic"},	//
	{0x12d1, 0x1003, "option", "1", "0", 2, &modeswitch_expose_all, "HUAWEI/Option E172/EC27/E220/E230/E270 HSDPA/HSUPA Model"},	//
	{0x12d1, 0x1009, "option", "0", "0", 2, &modeswitch_expose_all, "HUAWEI/Option U120"},	//
	{0x12d1, 0x101e, "option", "0", "0", 2, &modeswitch_huawei_std, "HUAWEI U7510 / U7517"},	//
	{0x12d1, 0x1031, "option", "0", "0", 2, &modeswitch_huawei_std, "HUAWEI U8110 (Android smartphone)"},	//
	{0x12d1, 0x1035, "option", "0", "0", 2, NULL, "HUAWEI U8110 (Android smartphone)"},	//
//      {0x12d1, 0x1400, "option", "0", "0", 2 | ETH, NULL,  "Huawei K4305 composite"}, //
	{0x12d1, 0x1404, "option", "2", "0", 2 | QMI, NULL, "HUAWEI UMG1831"},	//
	{0x12d1, 0x1406, "option", "1", "0", 2, NULL, "HUAWEI/Option newer modems"},	//
	{0x12d1, 0x140b, "option", "2", "0", 2, NULL, "HUAWEI/Option EC1260 Wireless Data Modem HSD USB Card"},	//
	{0x12d1, 0x140c, "option", "3", "0", 2 | QMI, NULL, "HUAWEI/Option newer modems"},	//
	{0x12d1, 0x1411, "option", "2", "0", 2, &modeswitch_expose_all, "HUAWEI E510/EC121"},	//
	{0x12d1, 0x1412, "option", "2", "0", 2, NULL, "HUAWEI/Option EC168"},	//
	{0x12d1, 0x1413, "option", "2", "0", 2, NULL, "HUAWEI/Option EC168"},	//
	{0x12d1, 0x1414, "option", "2", "0", 2, &modeswitch_expose_all, "HUAWEI/Option E180"},	//
	{0x12d1, 0x141b, "option", "1", "0", 2, NULL, "HUAWEI/Option newer modems"},	//
//      {0x12d1, 0x1432, "option", "0", "0", 2 | QMI, NULL, "HUAWEI E585"},     // ecm attributes but probably qmi
	{0x12d1, 0x1433, "option", "2", "0", 2, NULL, "HUAWEI/Option E1756C"},	//
	{0x12d1, 0x1436, "option", "2", "0", 2, NULL, "HUAWEI/Option E1800"},	// ecm attributes but probably qmi
	{0x12d1, 0x1444, "option", "0", "0", 2, NULL, "HUAWEI/Option E352-R1"},	//
	{0x12d1, 0x1446, "option", "0", "0", 2, &modeswitch_huawei_std, "HUAWEI/Option E1552/E1800 HSPA Modem"},	//
	{0x12d1, 0x1449, "option", "0", "0", 2, &modeswitch_huawei_std, "HUAWEI/Option E352-R1"},	//
	{0x12d1, 0x144e, "option", "0", "2", 2, NULL, "Huawei K3806"},	//
	{0x12d1, 0x1464, "option", "2", "0", 2, NULL, "Huawei K4505"},	//
	{0x12d1, 0x1465, "option", "2", "0", 2, NULL, "Huawei K3765"},	// ecm attributes but probably qmi
	{0x12d1, 0x1491, "option", "2", "0", 2, NULL, "Huawei R201"},	//
	{0x12d1, 0x14a5, "option", "2", "0", 2, NULL, "Huawei E173"},	//
	{0x12d1, 0x14a8, "option", "2", "0", 2, NULL, "Huawei E173"},	//
	{0x12d1, 0x14ac, "option", "2", "0", 2 | QMI, NULL, "HUAWEI/Option newer modems"},	//
	{0x12d1, 0x14ad, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei K3806"},	//
	{0x12d1, 0x14ae, "option", "1", "2", 2, NULL, "Huawei K3806"},	//ecm able
	{0x12d1, 0x14b5, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E173"},	//
	{0x12d1, 0x14b7, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei K4511"},	//
	{0x12d1, 0x14ba, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E173/E177 (cdrom)"},	//
//      {0x12d1, 0x14bc, "option", "0", "0", 2 | ETH, NULL,  "Huawei K3773 (net)"},     //
	{0x12d1, 0x14c1, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei K4605"},	//
	{0x12d1, 0x14c3, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei K5005"},	//
	{0x12d1, 0x14c4, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei K3771"},	//
	{0x12d1, 0x14c5, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei K4510"},	//
	{0x12d1, 0x14c6, "option", "2", "0", 2 | QMI, NULL, "Huawei K4605"},	//
	{0x12d1, 0x14c8, "option", "2", "0", 2 | QMI, NULL, "Huawei K5005"},	//
	{0x12d1, 0x14c9, "option", "2", "0", 2 | QMI, NULL, "Huawei K3770"},	//
	{0x12d1, 0x14ca, "option", "2", "0", 2 | QMI, NULL, "Huawei K3771"},	//
	{0x12d1, 0x14cb, "option", "2", "0", 2, NULL, "Huawei K4510"},	//
	{0x12d1, 0x14cc, "option", "2", "0", 2 | QMI, NULL, "Huawei K4511"},	//
	{0x12d1, 0x14cf, "option", "2", "0", 2, NULL, "Huawei K3772 (modem)"},	// ncm able
	{0x12d1, 0x14d1, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E182E"},	//
	{0x12d1, 0x14d2, "option", "2", "0", 2 | QMI, NULL, "Huawei E173/E177 (modem)"},	//
//      {0x12d1, 0x14db, "option", "0", "0", 2 | ETH, NULL,  "Huawei E353 composite"},  //
	{0x12d1, 0x14fe, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E352,E353"},	//
	{0x12d1, 0x1505, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E398"},	//
	{0x12d1, 0x1506, "option", "2", "0", 2, NULL, "Huawei E367/E398 (modem)"},	//can not be QMI flagged!
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
	{0x12d1, 0x155b, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E171/E320"},	//
	{0x12d1, 0x156a, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E3251/E3276"},	//
//      {0x12d1, 0x1575, "option", "0", "0", 2 | ETH, NULL,  "Huawei K5150 composite"}, //
//      {0x12d1, 0x1576, "option", "0", "0", 2 | ETH, NULL,  "Huawei K4201 composite"}, //
//      {0x12d1, 0x1577, "option", "0", "0", 2 | ETH, NULL,  "Huawei K4202 composite"}, //
//      {0x12d1, 0x1578, "option", "0", "0", 2 | ETH, NULL,  "Huawei K4606 composite"}, //
	{0x12d1, 0x1805, "option", "1", "0", 2, &modeswitch_expose_all, "Huawei U2800A/U6150 (modem)"},	//
	{0x12d1, 0x1c05, "option", "2", "0", 2, NULL, "Huawei E173s (modem)"},	//
	{0x12d1, 0x1c07, "option", "2", "0", 2, NULL, "Huawei E188 (modem)"},	//also cdc_ncm
	{0x12d1, 0x1c08, "option", "1", "0", 2, NULL, "Huawei E173s (modem)"},	//
	{0x12d1, 0x1c0b, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E173s (cdrom)"},	//
	{0x12d1, 0x1c10, "option", "2", "0", 2, NULL, "Huawei E173 (modem)"},	//
	{0x12d1, 0x1c12, "option", "2", "0", 2, NULL, "Huawei E173 (modem)"},	//
	{0x12d1, 0x1c1b, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E398 (cdrom)"},	//
//      {0x12d1, 0x1c1e, "option", "2", "0", 2 | NCM, NULL, "Huawei E586 (net)"},       //
//      {0x12d1, 0x1c1f, "option", "0", "0", 2 | NCM, NULL, "Huawei E587 (net)"},       //
	{0x12d1, 0x1c20, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E5220s-2 (cdrom)"},	//
	{0x12d1, 0x1c23, "option", "0", "2", 2, NULL, "Huawei E173 (modem)"},	//
	{0x12d1, 0x1c24, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E173 (cdrom)"},	//
	{0x12d1, 0x1f01, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E353 (cdrom)"},	//
	{0x12d1, 0x1f03, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei E353 (cdrom)"},	//
	{0x12d1, 0x1f11, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei K3773 (cdrom)"},	//
	{0x12d1, 0x1f15, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei K4305 (cdrom)"},	//also cdc_mbim
	{0x12d1, 0x1f16, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei K5150 (cdrom)"},	//also cdc_mbim
	{0x12d1, 0x1f17, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei K4201 (cdrom)"},	//also cdc_mbim
	{0x12d1, 0x1f18, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei K4202 (cdrom)"},	//also cdc_mbim
	{0x12d1, 0x1f19, "option", "0", "0", 2, &modeswitch_huawei_std, "Huawei K4606 (cdrom)"},	//also cdc_mbim

//Novatel Wireless
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
	{0x1410, 0x5010, "option", "0", "0", 2, &modeswitch_std_eject, "Novatel (cdrom)"},	//
	{0x1410, 0x5020, "option", "0", "0", 2, &modeswitch_std_eject, "Novatel MC990D (cdrom)"},	//
	{0x1410, 0x5023, "option", "0", "0", 2, &modeswitch_std_eject, "Novatel MC996D (cdrom)"},	//
	{0x1410, 0x5030, "option", "0", "0", 2, &modeswitch_std_eject, "Novatel USB760 (cdrom)"},	//
	{0x1410, 0x5031, "option", "0", "0", 2, &modeswitch_std_eject, "Novatel USB760 3G (cdrom)"},	//
	{0x1410, 0x5041, "option", "0", "0", 2, &modeswitch_std_eject, "Novatel MiFi 2372 (cdrom)"},	//
	{0x1410, 0x5059, "option", "0", "0", 2, &modeswitch_std_eject, "Novatel MC545/USB679 (cdrom)"},	//
	{0x1410, 0x6000, "option", "1", "0", 2, NULL, "Novatel USB760 (modem)"},	//
	{0x1410, 0x6001, "option", "1", "0", 2, NULL, "Novatel USB760 (modem)"},	//
	{0x1410, 0x6002, "option", "1", "0", 2, NULL, "Novatel USB760 3G (modem)"},	//
	{0x1410, 0x6010, "option", "1", "0", 2, NULL, "Novatel MC780 (modem)"},	//
	{0x1410, 0x7001, "option", "1", "0", 2, NULL, "Novatel MiFi 2372 (modem)"},	//
	{0x1410, 0x7003, "option", "1", "0", 2, NULL, "Novatel MiFi 2372 (modem)"},	//
	{0x1410, 0x7030, "option", "0", "0", 2, NULL, "Novatel USB998 (modem)"},	//
	{0x1410, 0x7031, "option", "0", "0", 2 | GENERIC, NULL, "Novatel USB679 (modem)"},	//
	{0x1410, 0x7041, "option", "0", "0", 2, NULL, "Novatel MF3470 (modem)"},	//
	{0x1410, 0x7042, "option", "0", "0", 2, NULL, "Novatel Ovation MC545/MC547 (modem)"},	//
	{0x1410, 0xb001, "option", "1", "0", 2 | QMI, NULL, "Novatel MC551/USB551L (modem)"},	//

//AnyDATA
	{0x16d5, 0x6202, "option", "2", "0", 2, NULL, "AnyData ADU-620UW"},	//
	{0x16d5, 0x6501, "option", "1", "0", 2, NULL, "AnyData ADU-300A"},	//
	{0x16d5, 0x6502, "option", "2", "0", 2, NULL, "AnyData ADU-500A"},	//
	{0x16d5, 0x6603, "option", "0", "0", 2 | GENERIC, NULL, "AnyData ADU-890WH"},	//
	{0x16d5, 0x900d, "option", "0", "0", 2 | ACM, NULL, "AnyData ADU-890WH"},	//

//CMOTECH
	{0x16d8, 0x5543, "option", "0", "0", 0 | ACM, NULL, "Cmotech CNU-550"},	//
	{0x16d8, 0x6002, "option", "1", "0", 0 | GENERIC, NULL, "Franklin U300"},	//
	{0x16d8, 0x6006, "option", "0", "0", 0 | GENERIC, NULL, "Cmotech CGU-628"},	//
	{0x16d8, 0x6008, "option", "2", "1", 0, NULL, "Franklin U301"},	//
	{0x16d8, 0x6280, "option", "2", "1", 0, &modeswitch_cmotech, "Cmotech CHU-628s"},	//
	{0x16d8, 0x6281, "option", "0", "0", 0 | GENERIC, &modeswitch_cmotech, "Cmotech CHU-628s"},	//
	{0x16d8, 0x6522, "option", "2", "0", 0 | GENERIC, NULL, "Cmotech CDU-650"},	//      
	{0x16d8, 0x6533, "option", "0", "0", 0 | ACM, NULL, "Cmotech CNM-650"},	//
	{0x16d8, 0x6803, "option", "1", "0", 0 | GENERIC, &modeswitch_cmotech, "Cmotech CDU-680"},	//
	{0x16d8, 0x6804, "option", "2", "1", 0 | GENERIC, &modeswitch_cmotech, "Cmotech CDU-685A"},	//
	{0x16d8, 0x680a, "option", "0", "0", 0 | ACM, NULL, "Cmotech CDU-680"},	//
	{0x16d8, 0x7003, "option", "1", "2", 0 | GENERIC, &modeswitch_cmotech, "Cmotech CHU-629K"},	//
	{0x16d8, 0x700a, "option", "0", "2", 0 | GENERIC, &modeswitch_cmotech, "Cmotech CHU-629S"},	//
	{0x16d8, 0xf000, "option", "0", "0", 0, &modeswitch_cmotech, "Cmotech CGU-628, 4g_xsstick W12"},	//

//ZTE WCDMA Technologies
	{0x19d2, 0x0001, "option", "2", "0", 2, NULL, "ONDA MT505UP/ZTE (modem)"},	//
	{0x19d2, 0x0002, "option", "2", "0", 2 | QMI, NULL, "ZTE ET502HS/MT505UP/MF632"},	//
	{0x19d2, 0x0003, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MU351 (cdrom)"},	//
	{0x19d2, 0x0015, "option", "2", "0", 2, NULL, "ONDA MT505UP/ZTE (modem)"},	//
	{0x19d2, 0x0016, "option", "1", "2", 2, NULL, "ONDA MF110/ZTE (modem)"},	//
	{0x19d2, 0x0017, "option", "1", "2", 2 | QMI, NULL, "ONDA MT505UP/ZTE (modem)"},	//
	{0x19d2, 0x0019, "option", "1", "2", 2 | QMI, NULL, "ONDA MT689DC/ZTE (modem)"},	//
	{0x19d2, 0x0022, "option", "1", "3", 2, NULL, "ZTE K2525 (modem)"},	//
	{0x19d2, 0x0026, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE AC581 (cdrom)"},	//
	{0x19d2, 0x0031, "option", "1", "2", 2, NULL, "ZTE MF110/MF112/MF626 (Variant) (modem)"},	//
	{0x19d2, 0x0037, "option", "2", "2", 2, NULL, "ONDA MT505UP/ZTE (modem)"},	//
	{0x19d2, 0x0040, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE K2525 (cdrom)"},	//
	{0x19d2, 0x0052, "option", "1", "2", 2 | QMI, NULL, "ONDA MT505UP/ZTE (modem)"},	//
	{0x19d2, 0x0053, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF110 (Variant) (modem)"},	//
	{0x19d2, 0x0055, "option", "3", "1", 2 | QMI, NULL, "ONDA MT505UP/ZTE (modem)"},	//
	{0x19d2, 0x0063, "option", "1", "3", 2 | QMI, NULL, "ZTE K3565-Z (modem)"},	//
	{0x19d2, 0x0064, "option", "0", "2", 2, NULL, "ZTE MF627 (modem)"},	//
	{0x19d2, 0x0066, "option", "1", "3", 2, NULL, "ZTE MF626 (modem)"},	//
	{0x19d2, 0x0079, "option", "1", "2", 2, NULL, "ZTE A353 (modem)"},	//
	{0x19d2, 0x0082, "option", "1", "2", 2, NULL, "ZTE MF668/MF190 (Variant) (modem)"},	//
	{0x19d2, 0x0083, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF110 (Variant) (cdrom)"},	//
	{0x19d2, 0x0086, "option", "1", "2", 2, NULL, "ZTE MF645 (modem)"},	//
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
	{0x19d2, 0x0142, "option", "0", "0", 2 | ACM, NULL, "ZTE MF665C"},	//also ecm
	{0x19d2, 0x0143, "option", "0", "0", 2 | ACM, NULL, "ZTE MF190B"},	//also ecm
	{0x19d2, 0x0146, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF652 (cdrom)"},	//
	{0x19d2, 0x0149, "option", "0", "0", 2, &modeswitch_zte_3msg, "ZTE MF190 (cdrom)"},	//
	{0x19d2, 0x0150, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF680 (cdrom)"},	//
	{0x19d2, 0x0154, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF190S (cdrom)"},	//
	{0x19d2, 0x0157, "option", "0", "4", 2 | QMI, NULL, "ZTE MF683 (modem)"},	//
	{0x19d2, 0x0166, "option", "0", "0", 2, &modeswitch_zte_other, "ZTE MF821 (Variant) (cdrom)"},	//
	{0x19d2, 0x0167, "option", "1", "3", 2 | QMI, NULL, "ZTE MF820D (variant) (modem)"},	//
	{0x19d2, 0x0169, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE A371 (cdrom)"},	//
	{0x19d2, 0x0170, "option", "0", "1", 2 | GENERIC, NULL, "ZTE A371 (variant) (modem)"},	//
	{0x19d2, 0x0257, "option", "1", "2", 2 | QMI, NULL, "ZTE MF821 (variant) (modem)"},	//
	{0x19d2, 0x0265, "option", "2", "3", 2 | QMI, NULL, "Onda MT8205/ZTE (modem)"},	//
	{0x19d2, 0x0266, "option", "0", "0", 2, &modeswitch_zte_other, "Onda MT8205/ZTE (cdrom)"},	//
	{0x19d2, 0x0284, "option", "1", "3", 2 | QMI, NULL, "ZTE MF880 (modem)"},	//
	{0x19d2, 0x0304, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF821D (cdrom)"},	//
	{0x19d2, 0x0325, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF821D (cdrom)"},	//
	{0x19d2, 0x0326, "option", "1", "3", 2 | QMI, NULL, "ZTE MF821D (modem)"},	//
//      {0x19d2, 0x0349, "option", "1", "3", 2 | QMI, NULL, "ZTE MF821D (modem)"},      // ecm attributes but likely to be QMI, not yet in qmi_wwan
	{0x19d2, 0x1001, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE K3805-Z (cdrom)"},	//
	{0x19d2, 0x1003, "option", "1", "0", 2 | ACM, NULL, "ZTE K3805-Z (modem)"},	// ecm able
	{0x19d2, 0x1007, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE K3570-Z (cdrom)"},	//
	{0x19d2, 0x1008, "option", "1", "3", 2 | QMI, NULL, "ZTE K3570-Z (modem)"},	//
	{0x19d2, 0x1009, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE K3571-Z (cdrom)"},	//
	{0x19d2, 0x1010, "option", "1", "3", 2 | QMI, NULL, "ZTE K3571-Z (modem)"},	//
	{0x19d2, 0x1013, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE K3806-Z (cdrom)"},	//
	{0x19d2, 0x1015, "option", "1", "0", 2 | ACM, NULL, "ZTE K3806-Z (modem)"},	// ecm able
	{0x19d2, 0x1017, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE K5006-Z (cdrom)"},	//
	{0x19d2, 0x1018, "option", "0", "2", 2 | QMI, NULL, "ZTE K5006-Z (modem)"},	//
	{0x19d2, 0x1030, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE K5008-Z (cdrom)"},	//
//      {0x19d2, 0x1032, "option", "0", "2", 2 | ECM, NULL, "ZTE K5008-Z (modem)"},     //ecm attributes but could be qmi
	{0x19d2, 0x1171, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE K4510-Z (cdrom)"},	//
	{0x19d2, 0x1173, "option", "0", "0", 2 | ACM, NULL, "ZTE K4510-Z (modem)"},	// ecm able
	{0x19d2, 0x1175, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE K3770-Z (cdrom)"},	//
	{0x19d2, 0x1176, "option", "0", "0", 2 | QMI, NULL, "ZTE K3770-Z (modem)"},	//
	{0x19d2, 0x1177, "option", "0", "0", 2 | ACM, NULL, "ZTE K3770-Z (modem)"},	// ecm able
	{0x19d2, 0x1179, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE K3772-Z (cdrom)"},	//
	{0x19d2, 0x1181, "option", "0", "0", 2 | ACM, NULL, "ZTE K3772-Z (modem)"},	// ecm able
	{0x19d2, 0x1201, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF691 (cdrom)"},	//
	{0x19d2, 0x1203, "option", "0", "0", 2 | ACM, NULL, "ZTE MF691 (modem)"},	//
	{0x19d2, 0x1207, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF192 (cdrom)"},	//
	{0x19d2, 0x1208, "option", "0", "0", 2 | ACM, NULL, "ZTE MF192 (modem)"},	//
	{0x19d2, 0x1210, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF195 (cdrom)"},	//
	{0x19d2, 0x1211, "option", "0", "0", 2 | ACM, NULL, "ZTE MF195 (modem)"},	//also ecm
	{0x19d2, 0x1212, "option", "0", "0", 2 | ACM, NULL, "ZTE MF195 (modem)"},	//
	{0x19d2, 0x1216, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF192 (cdrom)"},	//
	{0x19d2, 0x1217, "option", "0", "0", 2 | ACM, NULL, "ZTE MF192"},	//also ecm
	{0x19d2, 0x1218, "option", "0", "0", 2 | ACM, NULL, "ZTE MF192"},	//
	{0x19d2, 0x1219, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF192 (cdrom)"},	//
	{0x19d2, 0x1220, "option", "0", "0", 2 | ACM, NULL, "ZTE MF192"},	//also ecm
	{0x19d2, 0x1224, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF190 (cdrom)"},	//
	{0x19d2, 0x1225, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF667 (cdrom)"},	//
	{0x19d2, 0x1227, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF669 (cdrom)"},	//
	{0x19d2, 0x1245, "option", "1", "0", 2 | QMI, NULL, "ZTE MF190 (modem)"},	//
	{0x19d2, 0x1252, "option", "1", "3", 2 | QMI, NULL, "ZTE MF669 (modem)"},	//
	{0x19d2, 0x1254, "option", "1", "3", 2 | QMI, NULL, "ZTE MF190 (modem)"},	//
	{0x19d2, 0x1256, "option", "1", "0", 2 | QMI, NULL, "ZTE MF190 (modem)"},	//
	{0x19d2, 0x1402, "option", "0", "0", 2 | QMI, NULL, "ZTE MF60 (modem)"},	//
//      {0x19d2, 0x1403, "option", "0", "0", 2 | RNDIS, NULL, "ZTE MF825A (modem)"},    //
//      {0x19d2, 0x1405, "option", "0", "0", 2 | ECM, NULL, "ZTE MF667 (modem)"},       //qmi tested - failed
//      {0x19d2, 0x1408, "option", "0", "0", 2 | ECM, NULL, "ZTE MF825A (modem)"},      //qmi tested - failed
	{0x19d2, 0x1426, "option", "1", "0", 2 | QMI, NULL, "ZTE MF91D (modem)"},	//
	{0x19d2, 0x1514, "option", "0", "0", 2, &modeswitch_zte_other, "ZTE MF192 (cdrom)"},	//
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
	{0x19d2, 0x1542, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE MF190J (cdrom)"},	//
	{0x19d2, 0x1544, "option", "0", "0", 2 | ACM, NULL, "ZTE MF190J (modem)"},	//
	{0x19d2, 0x2000, "option", "0", "0", 2, &modeswitch_zte_3msg, "ONDA/ZTE (cdrom)"},	//
	{0x19d2, 0x2002, "option", "1", "3", 2 | QMI, NULL, "ZTE K3765-Z (modem)"},	//
	{0x19d2, 0x2003, "option", "1", "3", 2, NULL, "ZTE MF180 (modem)"},	//
	{0x19d2, 0x2004, "option", "0", "0", 2, &modeswitch_zte_3msg, "ZTE MF60 (cdrom)"},	//
	{0x19d2, 0xffdd, "option", "1", "0", 2, NULL, "ZTE AC682 (modem)"},	//
	{0x19d2, 0xffde, "option", "0", "0", 2, &modeswitch_std_eject, "ZTE AC682 (cdrom)"},	//
	{0x19d2, 0xfff1, "option", "1", "0", 2, NULL, "ZTE generic (modem)"},	//
	{0x19d2, 0xfff5, "option", "0", "0", 2, &modeswitch_zte_other, "ZTE generic (cdrom)"},	//
	{0x19d2, 0xfff6, "option", "0", "0", 2, &modeswitch_zte_other, "ZTE generic (cdrom)"},	//
	{0x19d2, 0xfffe, "zte_ev", "1", "0", 2, NULL, "ZTE generic (modem)"},	//
	{0x19d2, 0xffff, "zte_ev", "1", "0", 2, NULL, "ZTE generic (modem)"},	//

//Infomark
//      {0x19f2, 0x1700, "option", "0", "0", 0 | ETH, NULL, "Clear Spot Voyager mifi"}, //

//Bandrich
	{0x1a8d, 0x1000, "option", "0", "0", 2, &modeswitch_bandrich, "Bandrich C-100/C-120/C-170/C-180/C-270/C-320/C321 (cdrom)"},	//
//      {0x1a8d, 0x1001, "option", "1", NULL, 2 | ECM, NULL, "Bandrich C-100/C-120 (netif)"},   //
	{0x1a8d, 0x1002, "option", "1", "0", 2, NULL, "Bandrich C-100/C-120 (modem)"},	//
	{0x1a8d, 0x1003, "option", "1", "0", 2, NULL, "Bandrich C-100/C-120 (modem)"},	//
	{0x1a8d, 0x1007, "option", "2", "0", 2, NULL, "Bandrich C-270 (modem)"},	//
	{0x1a8d, 0x1009, "option", "2", "0", 2, NULL, "Bandrich C-170/C-180 (modem)"},	//
	{0x1a8d, 0x100c, "option", "2", "0", 2, NULL, "Bandrich C-320 (modem)"},	// ecm able
	{0x1a8d, 0x100d, "option", "2", "0", 2, NULL, "Bandrich C-508 (modem)"},	// ecm able
	{0x1a8d, 0x2000, "option", "0", "0", 2, &modeswitch_bandrich, "Bandrich C33x (cdrom)"},	//
	{0x1a8d, 0x2006, "option", "0", "0", 2 | ACM, NULL, "Bandrich C-33x (modem)"},	//

//T&A Mobile Phones (Alcatel)
	{0x1bbb, 0x0000, "option", "2", "2", 2, NULL, "Alcatel X060S/X070S/X080S/X200 (modem)"},	//
	{0x1bbb, 0x0012, "option", "2", "2", 2 | GENERIC, NULL, "Alcatel X085C (modem)"},	//
	{0x1bbb, 0x0017, "option", "4", "4", 2, NULL, "Alcatel X220L (Variant), X500D (modem)"},	//
	{0x1bbb, 0x0052, "option", "4", "4", 2, NULL, "Alcatel X220L (Variant), (modem)"},	//
	{0x1bbb, 0x00ca, "option", "0", "0", 2 | GENERIC, NULL, "Alcatel X080C (Variant), (modem)"},	//
	{0x1bbb, 0x011e, "option", "1", "2", 2 | QMI, NULL, "Alcatel L100V, (modem)"},	//
	{0x1bbb, 0xf000, "option", "0", "0", 2, &modeswitch_alcatel, "Alcatel X060S/X070S/X080S/X200/X220L/X500D(cdrom)"},	//
	{0x1bbb, 0xf017, "option", "0", "0", 2, &modeswitch_alcatel, "Alcatel X220D(cdrom)"},	//
	{0x1bbb, 0xf052, "option", "0", "0", 2, &modeswitch_alcatel, "Alcatel X220L(cdrom)"},	//

//OMEGA TECHNOLOGY
	{0x1c9e, 0x1001, "option", "0", "0", 2, &modeswitch_alcatel, "Alcatel X020 & X030 (cdrom)"},	//
	{0x1c9e, 0x6060, "option", "2", "0", 2 | GENERIC, NULL, "Alcatel X020 & X030 (modem)"},	//
	{0x1c9e, 0x6061, "option", "2", "0", 2 | GENERIC, NULL, "Alcatel X020 & X030 (modem)"},	//
	{0x1c9e, 0x9000, "option", "0", "0", 2 | GENERIC, NULL, "4G Systems XS Stick W14 (modem)"},	//
	{0x1c9e, 0x9603, "option", "1", "2", 2, NULL, "4G Systems XS Stick W14 (modem)"},	//
	{0x1c9e, 0x9605, "option", "1", "3", 2 | GENERIC, NULL, "4G Systems XS Stick W14 (modem)"},	//
	{0x1c9e, 0x9607, "option", "1", "3", 2, NULL, "4G Systems XS Stick W14 (modem)"},	//
	{0x1c9e, 0x9801, "option", "1", "3", 2 | GENERIC, NULL, "4G Systems XS Stick W21 (modem)"},	//
	{0x1c9e, 0x98ff, "option", "0", "0", 2, &modeswitch_alcatel, "4G Systems XS Stick W21 (cdrom)"},	//
	{0x1c9e, 0xf000, "option", "0", "0", 2, &modeswitch_alcatel, "4G Systems XS Stick W14 (cdrom)"},	//

//Qualcomm /ALink /Hyundai
	{0x1e0e, 0x9000, "option", "2", "2", 3, NULL, "PROLink PHS100, Hyundai MB-810, A-Link 3GU (modem)"},	//
	{0x1e0e, 0x9200, "option", "2", "2", 3, NULL, "PROLink PHS100, Hyundai MB-810, A-Link 3GU (modem)"},	//
	{0x1e0e, 0xce16, "option", "1", "0", 3, NULL, "D-Link DWM-162-U5, Micromax MMX 300c (modem)"},	//
	{0x1e0e, 0xf000, "option", "0", "0", 3, &modeswitch_expose_all, "PROLink PHS100, Hyundai MB-810, A-Link 3GU (cdrom)"},	//

// D-Link (3rd VID)
	{0x2001, 0x7d00, "option", "1", "0", 2 | GENERIC, NULL, "D-Link DWM-156 A6 (modem)"},	//
	{0x2001, 0x7d01, "option", "1", "0", 2, NULL, "D-Link DWM-156 A7 (modem)"},	//
	{0x2001, 0x7d02, "option", "1", "0", 2, NULL, "D-Link DWM-156 A7 (modem)"},	//
	{0x2001, 0x7d03, "option", "1", "0", 2, NULL, "D-Link DWM-156 A7 (modem)"},	//
	{0x2001, 0xa706, "option", "0", "0", 2, &modeswitch_std_eject, "D-Link DWM-156 A7 (cdrom)"},	//
	{0x2001, 0xa80b, "option", "0", "0", 2, &modeswitch_std_eject, "D-Link DWM-156 A6 (cdrom)"},	//

//CELOT Corporation
	{0x211f, 0x6801, "option", "2", "0", 2, NULL, "Celot K-3000/CT-650/CT-680 (modem)"},	//

//Linktop
	{0x230d, 0x0001, "option", "0", "1", 0 | ACM, &modeswitch_linktop, "Linktop LW27x (BSNL 3G)"},	//
	{0x230d, 0x0007, "option", "0", "1", 0 | ACM, &modeswitch_linktop, "Linktop LW27x (Visiontek 3G)"},	//

//TP-Link
	{0x2357, 0x0200, "option", "0", "0", 2, &modeswitch_std_eject, "TP-Link MA180 (cdrom)"},	//
	{0x2357, 0x0201, "option", "1", "2", 2 | QMI, NULL, "TP-Link MA180 (modem)"},	//
	{0x2357, 0x0202, "option", "1", "2", 2 | QMI, NULL, "TP-Link MA180 (modem)"},	//
	{0x2357, 0x0203, "option", "1", "2", 2 | QMI, NULL, "TP-Link MA180 (modem)"},	//
	{0x2357, 0x9000, "option", "1", "2", 2 | QMI, NULL, "TP-Link MA260 (modem)"},	//
	{0x2357, 0xf000, "option", "0", "0", 2, &modeswitch_std_eject, "TP-Link MA260 (cdrom)"},	//

//Dell
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
				insmod("cdc-wdm");
				insmod("usbnet");
				insmod("qmi_wwan");
				//start custom setup, if defined
				if (devicelist[devicecount].customsetup) {
					fprintf(stderr, "customsetup QMI\n");
					devicelist[devicecount].customsetup(needreset, devicecount);
				}
				sprintf(control, "qmi");
				nvram_set("3gdata", "qmi");
				return control;
			}
#endif
			if (devicelist[devicecount].driver) {
				insmod("usbserial");
				insmod("usb_wwan");
				insmod("cdc-wdm");
				insmod("usbnet");
				insmod("qmi_wwan");
				insmod("zte_ev");
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
						insmod("usb_wwan");
						insmod("cdc-wdm");
						insmod("usbnet");
						insmod("qmi_wwan");
						sprintf(data, "/dev/usb/tts/%s", devicelist[devicecount].datadevice);
					} else
						sprintf(data, "/dev/usb/tts/%s", devicelist[devicecount].datadevice);

				}
				nvram_set("3gdata", data);
			}
			if (devicelist[devicecount].modeswitch & 0x07) {
				char variant[32];
				sprintf(variant, "%d", devicelist[devicecount].modeswitch & 0xf);
				nvram_set("3gnmvariant", variant);
			}
			//start custom setup, if defined
			if (devicelist[devicecount].customsetup) {
				fprintf(stderr, "customsetup\n");
				devicelist[devicecount].customsetup(needreset, devicecount);
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
				system("/etc/hso/hso_connect.sh restart");
			} else if ((devicelist[devicecount].modeswitch & ACM)) {
				insmod("cdc-acm");
				sprintf(control, "/dev/ttyACM%s", devicelist[devicecount].controldevice);
			} else if ((devicelist[devicecount].modeswitch & GENERIC)) {
				sysprintf("echo %04x %04x > /sys/bus/usb-serial/drivers/option1/new_id", devicelist[devicecount].vendor, devicelist[devicecount].product);
				insmod("usb_wwan");
				insmod("cdc-wdm");
				insmod("usbnet");
				insmod("qmi_wwan");
				sprintf(control, "/dev/usb/tts/%s", devicelist[devicecount].controldevice);
			} else
				sprintf(control, "/dev/usb/tts/%s", devicelist[devicecount].controldevice);
			return control;
		}
		devicecount++;
	}
	//not found, use generic implementation (all drivers)
	insmod("cdc-acm");
	insmod("cdc-wdm");
	insmod("usbnet");
	insmod("qmi_wwan");
	insmod("usbserial");
	insmod("usb_wwan");
	insmod("sierra");
	insmod("option");
	insmod("zte_ev");
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
