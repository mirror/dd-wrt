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

#include <stdio.h>
#include <stdlib.h>
#include <shutils.h>
#include <bcmnvram.h>

static int scanFor(int Vendor, int Product)
{
#if defined(ARCH_broadcom) && !defined(HAVE_BCMMODERN)
	char grepstr[128];
	sprintf(grepstr, "grep Vendor=%x ProdID=%x /tmp/usb/devices|wc -l",
		Vendor, Product);
	FILE *check = popen(grepstr, "rb");
	if (check) {
		int count = 0;
		fscanf(check, "%d", &count);
		fclose(check);
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
		sprintf(sysfs, "/sys/bus/usb/devices/%d-0:1.0/bInterfaceNumber",
			count);
		FILE *probe = fopen(sysfs, "rb");
		if (!probe) {
			count = 1;
			hub++;
			continue;
		}
		fclose(probe);

		sprintf(sysfs, "/sys/bus/usb/devices/%d-%d/idProduct", count,
			hub);
		FILE *modem = fopen(sysfs, "rb");
		if (!modem) {
			count++;
			continue;
		}
		int idProduct;
		int idVendor;
		fscanf(modem, "%X", &idProduct);
		fclose(modem);
		sprintf(sysfs, "/sys/bus/usb/devices/%d-%d/idVendor", count,
			hub);
		modem = fopen(sysfs, "rb");
		if (!modem) {
			count++;
			continue;
		}
		fscanf(modem, "%X", &idVendor);
		fclose(modem);
		if (idVendor == Vendor && idProduct == Product)
			return 1;

		count++;
	}
	return 0;
#endif
}

void checkreset(char *tty)
{
#ifdef HAVE_CAMBRIA
	eval("comgt", "-d", tty, "-s", "/etc/comgt/reset.comgt");
	FILE *check = NULL;
	int count = 0;
	sleep(1);
	while (!(check = fopen(tty, "rb")) && count < 10) {
		sleep(1);
		count++;
	}
	if (check)
		fclose(check);
	else
		fprintf(stderr, "reset error\n");
	fprintf(stderr, "wakeup card\n");
	eval("comgt", "-d", tty, "-s", "/etc/comgt/wakeup.comgt");
	sleep(5);		//give extra delay for registering
#endif
}

static void reset_mc(int needreset, char *controldev)
{
	if (needreset)
		checkreset(controldev);
}

static void modeswitch_usb760(int needreset, char *controldev)
{
	system
	    ("usb_modeswitch -v 0x1410 -p 0x5010 -M 5553424312345678000000000000061b000000020000000000000000000000");
	system
	    ("usb_modeswitch -v 0x1410 -p 0x5020 -M 5553424312345678000000000000061b000000020000000000000000000000");
	system
	    ("usb_modeswitch -v 0x1410 -p 0x5030 -M 5553424312345678000000000000061b000000020000000000000000000000");
	system
	    ("usb_modeswitch -v 0x1410 -p 0x5031 -M 5553424312345678000000000000061b000000020000000000000000000000");
	system
	    ("usb_modeswitch -v 0x1410 -p 0x5041 -M 5553424312345678000000000000061b000000020000000000000000000000");
	sleep(2);
}

static void modeswitch_onda(int needreset, char *controldev)
{
	FILE *out = fopen("/tmp/usb_modeswitch.conf", "wb");

	fprintf(out, "DefaultVendor=0x19d2\n"
		"DefaultProduct=0x2000\n"
		"TargetVendor=0x19d2\n"
		"TargetProductList=\"0001,0002,0015,0016,0017,0031,0037,0052,0055,0063,0064,0108,0128\"\n"
		"MessageContent=\"5553424312345678000000000000061e000000000000000000000000000000\"\n"
		"MessageContent2=\"5553424312345679000000000000061b000000020000000000000000000000\"\n"
		"MessageContent3=\"55534243123456702000000080000c85010101180101010101000000000000\"\n"
		"NeedResponse=1\n" "CheckSuccess=20\n");
	fclose(out);
	system("usb_modeswitch -c /tmp/usb_modeswitch.conf");
	sleep(2);
}

static void modeswitch_onda2(int needreset, char *controldev)
{
	system
	    ("usb_modeswitch -v 0x19d2 -p 0x0003 -M 5553424312345678000000000000061e000000000000000000000000000000 -2 5553424312345679000000000000061b000000020000000000000000000000");
	system
	    ("usb_modeswitch -v 0x19d2 -p 0x0026 -n -M 5553424312345678000000000000061b000000020000000000000000000000");
	system
	    ("usb_modeswitch -v 0x19d2 -p 0x0040 -n -M 5553424312345678000000000000061e000000000000000000000000000000 -2 5553424312345679000000000000061b000000020000000000000000000000");
	system
	    ("usb_modeswitch -v 0x19d2 -p 0x0053 -n -M 5553424312345678000000000000061e000000000000000000000000000000 -2 5553424312345679000000000000061b000000020000000000000000000000");
	system
	    ("usb_modeswitch -v 0x19d2 -p 0x0083 -n -M 5553424312345678000000000000061b000000020000000000000000000000");
	system
	    ("usb_modeswitch -v 0x19d2 -p 0x0101 -n -M 5553424312345678000000000000061b000000020000000000000000000000");
	system
	    ("usb_modeswitch -v 0x19d2 -p 0x0103 -n -M 5553424312345678000000000000061e000000000000000000000000000000 -2 5553424312345679000000000000061b000000020000000000000000000000");
	system
	    ("usb_modeswitch -v 0x19d2 -p 0x0115 -n -M 5553424312345678000000000000061b000000020000000000000000000000");
	system
	    ("usb_modeswitch -v 0x19d2 -p 0x1001 -n -M 5553424312345678000000000000061b000000020000000000000000000000");
	system
	    ("usb_modeswitch -v 0x19d2 -p 0x1007 -n -M 5553424312345678000000000000061b000000020000000000000000000000");
	system
	    ("usb_modeswitch -v 0x19d2 -p 0x1009 -n -M 5553424312345678000000000000061b000000020000000000000000000000");
	system
	    ("usb_modeswitch -v 0x19d2 -p 0x1013 -n -M 5553424312345678000000000000061b000000020000000000000000000000");
	system
	    ("usb_modeswitch -v 0x19d2 -p 0xfff5 -M 5553424312345678c00000008000069f030000000000000000000000000000");
	system
	    ("usb_modeswitch -v 0x19d2 -p 0xfff6 -M 5553424312345678c00000008000069f030000000000000000000000000000");
	sleep(2);
}

static void modeswitch_sierra(int needreset, char *controldev)
{
	FILE *out = fopen("/tmp/usb_modeswitch.conf", "wb");

	fprintf(out, "DefaultVendor=0x1199\n"
		"DefaultVendor=0x0fff\n"
		"TargetVendor=0x1199\n"
		"TargetProductList=\"0017,0018,0019,0020,0021,0022,0024,0026,0027,0028,0029,0112,0120,0218,0220,0224,6802,6803,6804,6805,6808,6809,6812,6813,6815,6816,6820,6821,6822,6832,6833,6834,6835,6838,6839,683a,683b,683c,683d,683e,6850,6851,6852,6853,6855,6856,6859,685a\"\n"
		"SierraMode=1\n" "CheckSuccess=10\n");
	fclose(out);
	system("usb_modeswitch -c /tmp/usb_modeswitch.conf");
	sleep(2);
}

static void modeswitch_huawei(int needreset, char *controldev)
{
	system("usb_modeswitch -v 0x12d1 -p 0x1001 -H");
	system("usb_modeswitch -v 0x12d1 -p 0x1003 -H");
	system("usb_modeswitch -v 0x12d1 -p 0x1414 -H");
	system
	    ("usb_modeswitch -v 0x12d1 -p 0x101e -M 55534243123456780600000080000601000000000000000000000000000000");
	system
	    ("usb_modeswitch -v 0x12d1 -p 0x1031 -M 55534243123456780600000080010a11060000000000000000000000000000");
	system
	    ("usb_modeswitch -v 0x12d1 -p 0x1446 -M 55534243123456780000000000000011060000000000000000000000000000");
	system
	    ("usb_modeswitch -v 0x12d1 -p 0x14ad -M 55534243123456780000000000000011060000000000000000000000000000");
	system
	    ("usb_modeswitch -v 0x12d1 -p 0x14c6 -M 55534243123456780000000000000011060000000000000000000000000000");
	system
	    ("usb_modeswitch -v 0x12d1 -p 0x1520 -M 55534243123456780000000000000011060000000000000000000000000000");
	system
	    ("usb_modeswitch -v 0x12d1 -p 0x1521 -M 55534243123456780000000000000011060000000000000000000000000000");
	system
	    ("usb_modeswitch -v 0x12d1 -p 0x1523 -M 55534243123456780000000000000011060000000000000000000000000000");
	system
	    ("usb_modeswitch -v 0x12d1 -p 0x1557 -M 55534243123456780000000000000011060000000000000000000000000000");
	sleep(2);
}

static void modeswitch_icon210(int needreset, char *controldev)
{
	FILE *out = fopen("/tmp/usb_modeswitch.conf", "wb");
	fprintf(out, "DefaultVendor=0x1e0e\n");
	fprintf(out, "DefaultProduct=0xf000\n");
	fprintf(out, "TargetVendor=0x1e0e\n");
	fprintf(out, "TargetProductList=\"9200,9000\"\n");
	fprintf(out,
		"MessageContent=\"555342431234567800000000000006bd000000020000000000000000000000\"\n");
	fprintf(out, "NeedResponse=1\n");
	fclose(out);
	system("usb_modeswitch -c /tmp/usb_modeswitch.conf");
	sleep(2);
}

static void hsoinit_icon225(int needreset, char *controldev)
{
	system("ozerocdoff -wi 0x6971");
	sleep(10);
	system("insmod hso");
	FILE *out = fopen("/tmp/conninfo.ini", "wb");
	fprintf(out, "APN=%s\n", nvram_safe_get("wan_apn"));
	fprintf(out, "USER=%s\n", nvram_safe_get("ppp_username"));
	fprintf(out, "PASS=%s\n", nvram_safe_get("ppp_passwd"));
	fprintf(out, "PIN=%s\n", nvram_safe_get("wan_pin"));
	fclose(out);
	nvram_set("3gdata", "hso");
	system("/etc/hso/hso_connect.sh restart");
}

struct DEVICES {
	int vendor;
	int product;
	char *driver;
	char *controldevice;
	char *datadevice;
	int modeswitch;
	void (*customsetup) (int needreset, char *controldev);
	char *name;
};

static struct DEVICES devicelist[] = {
//sierra wireless cards
	{0x1199, 0x0fff, "sierra", "3", "4", 1, &modeswitch_sierra, "Sierra Device CDROM Mode"},	//
	{0x1199, 0x0017, "sierra", "3", "4", 1, NULL, "Sierra Wireless Modem Mode"},	//
	{0x1199, 0x0018, "sierra", "3", "4", 1, NULL, "Sierra Wireless Modem Mode"},	//
	{0x1199, 0x0019, "sierra", "3", "4", 1, NULL, "Sierra Wireless AC595U Modem Mode"},	//
	{0x1199, 0x0020, "sierra", "3", "4", 1, NULL, "Sierra Wireless Modem Mode"},	//
	{0x1199, 0x0021, "sierra", "3", "4", 1, NULL, "Sierra Wireless AC597E Modem Mode"},	//
	{0x1199, 0x0022, "sierra", "3", "4", 1, NULL, "Sierra Wireless Modem Mode"},	//
	{0x1199, 0x0024, "sierra", "3", "4", 1, NULL, "Sierra Wireless MC5727 CDMA Modem Mode"},	//
	{0x1199, 0x0026, "sierra", "3", "4", 1, NULL, "Sierra Wireless Modem Mode"},	//
	{0x1199, 0x0027, "sierra", "3", "4", 1, NULL, "Sierra Wireless Modem Mode"},	//
	{0x1199, 0x0028, "sierra", "3", "4", 1, NULL, "Sierra Wireless Modem Mode"},	//
	{0x1199, 0x0029, "sierra", "3", "4", 1, NULL, "Sierra Wireless Modem Mode"},	//
	{0x1199, 0x0112, "sierra", "3", "4", 1, NULL, "Sierra Wireless CDMA 1xEVDO PC Card, Aircard 580 Modem Mode"},	//
	{0x1199, 0x0120, "sierra", "3", "4", 1, NULL, "Sierra Wireless AC595U Modem Mode"},	//
	{0x1199, 0x0218, "sierra", "3", "4", 1, NULL, "Sierra Wireless MC5720 Wireless Modem"},	//
	{0x1199, 0x0220, "sierra", "3", "4", 1, NULL, "Sierra Wireless Modem Mode"},	//
	{0x1199, 0x0224, "sierra", "3", "4", 1, NULL, "Sierra Wireless Modem Mode"},	//
	{0x1199, 0x6802, "sierra", "0", "2", 1, NULL, "Sierra Wireless MC8755 Modem Mode"},	//
	{0x1199, 0x6803, "sierra", "0", "2", 1, NULL, "Sierra Wireless MC8765 Modem Mode"},	//
	{0x1199, 0x6804, "sierra", "0", "2", 1, NULL, "Sierra Wireless MC8755 Modem Mode"},	//
	{0x1199, 0x6805, "sierra", "0", "2", 1, NULL, "Sierra Wireless MC8765 Modem Mode"},	//
	{0x1199, 0x6808, "sierra", "0", "2", 1, NULL, "Sierra Wireless MC8755 Modem Mode"},	//
	{0x1199, 0x6809, "sierra", "0", "2", 1, NULL, "Sierra Wireless MC8755 Modem Mode"},	//
	{0x1199, 0x6812, "sierra", "0", "2", 1, &reset_mc, "Sierra Wireless MC8775V"},	//
	{0x1199, 0x6813, "sierra", "0", "2", 1, NULL, "Sierra Wireless MC8775 Modem Mode"},	//
	{0x1199, 0x6815, "sierra", "0", "2", 1, NULL, "Sierra Wireless MC8775 Modem Mode"},	//
	{0x1199, 0x6816, "sierra", "0", "2", 1, NULL, "Sierra Wireless MC8775 Modem Mode"},	//
	{0x1199, 0x6820, "sierra", "0", "2", 1, NULL, "Sierra Wireless AC875 Modem Mode"},	//
	{0x1199, 0x6821, "sierra", "2", "0", 1, NULL, "Sierra Wireless AC875U Modem Mode"},	//
	{0x1199, 0x6822, "sierra", "3", "4", 1, NULL, "Sierra Wireless Modem Mode"},	//
	{0x1199, 0x6832, "sierra", "2", "0", 1, &reset_mc, "Sierra Wireless MC8780 Modem Mode"},	//
	{0x1199, 0x6833, "sierra", "3", "4", 1, NULL, "Sierra Wireless MC8781 Modem Mode"},	//
	{0x1199, 0x6834, "sierra", "3", "4", 1, NULL, "Sierra Wireless Modem Mode"},	//
	{0x1199, 0x6835, "sierra", "3", "4", 1, NULL, "Sierra Wireless Modem Mode"},	//
	{0x1199, 0x6838, "sierra", "3", "4", 1, NULL, "Sierra Wireless Modem Mode"},	//
	{0x1199, 0x6839, "sierra", "3", "4", 1, NULL, "Sierra Wireless Modem Mode"},	//
	{0x1199, 0x683a, "sierra", "3", "4", 1, NULL, "Sierra Wireless MC8785 Modem Mode"},	//
	{0x1199, 0x683b, "sierra", "3", "4", 1, NULL, "Sierra Wireless MC8785 Composite Modem Mode"},	//
	{0x1199, 0x683c, "sierra", "3", "4", 1, &reset_mc, "Sierra Wireless MC8790"},	//
	{0x1199, 0x683d, "sierra", "3", "4", 1, &reset_mc, "Sierra Wireless MC8791 Composite"},	//
	{0x1199, 0x683e, "sierra", "3", "4", 1, &reset_mc, "Sierra Wireless MC8790"},	//
	{0x1199, 0x6850, "sierra", "2", "0", 1, NULL, "Sierra Wireless AirCard 880 Modem Mode"},	//
	{0x1199, 0x6851, "sierra", "2", "0", 1, NULL, "Sierra Wireless AirCard 881 Modem Mode"},	//
	{0x1199, 0x6852, "sierra", "2", "0", 1, NULL, "Sierra Wireless AirCard 880E Modem Mode"},	//
	{0x1199, 0x6853, "sierra", "2", "0", 1, NULL, "Sierra Wireless AirCard 881E Modem Mode"},	//
	{0x1199, 0x6855, "sierra", "2", "0", 1, NULL, "Sierra Wireless Modem Mode"},	//
	{0x1199, 0x6856, "sierra", "2", "0", 1, NULL, "Sierra Wireless ATT USB Connect 881 Modem Mode"},	//
	{0x1199, 0x6859, "sierra", "2", "0", 1, NULL, "Sierra Wireless Modem Mode"},	//
	{0x1199, 0x685a, "sierra", "2", "0", 1, NULL, "Sierra Wireless Modem Mode"},	//
	{0x1199, 0x6880, "sierra", "3", "3", 1, NULL, "Sierra Wireless Compass 885"},	//
	{0x1199, 0x6890, "sierra", "3", "3", 1, NULL, "Sierra Wireless Compass 888"},	//
	{0x1199, 0x6893, "sierra", "3", "3", 1, NULL, "Sierra Wireless Compass 889"},	//
//      {0x1199, 0x68a3, "sierra", "3", "4", 1, NULL, "Sierra Wireless Compass 889"},   //alternate variant
	{0x1199, 0x68a3, "sierra", "2", "0", 1, &reset_mc, "Sierra Wireless MC8700"},	//
//option/huawei
	{0x12d1, 0x1001, "option", "0", "0", 2, &modeswitch_huawei, "HUAWEI/Option E600/E620 or generic"},	//
	{0x12d1, 0x1003, "option", "0", "0", 2, &modeswitch_huawei, "HUAWEI/Option E172/EC27/E220/E230/E270 HSDPA/HSUPA Model"},	//
	{0x12d1, 0x1009, "option", "0", "0", 2, &modeswitch_huawei, "HUAWEI/Option U120"},	//
	{0x12d1, 0x101e, "option", "0", "0", 2, &modeswitch_huawei, "HUAWEI U7510 / U7517"},	//
	{0x12d1, 0x1031, "option", "0", "0", 2, &modeswitch_huawei, "HUAWEI U8110 (Android smartphone)"},	//
	{0x12d1, 0x1035, "option", "0", "0", 2, NULL, "HUAWEI U8110 (Android smartphone)"},	//
	{0x12d1, 0x1414, "option", "0", "0", 2, &modeswitch_huawei, "HUAWEI/Option E180"},	//
	{0x12d1, 0x1406, "option", "0", "0", 2, NULL, "HUAWEI/Option newer modems"},	//
	{0x12d1, 0x140b, "option", "0", "0", 2, NULL, "HUAWEI/Option EC1260 Wireless Data Modem HSD USB Card"},	//
	{0x12d1, 0x140c, "option", "0", "0", 2, NULL, "HUAWEI/Option newer modems"},	//
	{0x12d1, 0x1412, "option", "0", "0", 2, NULL, "HUAWEI/Option EC168"},	//
	{0x12d1, 0x141b, "option", "0", "0", 2, NULL, "HUAWEI/Option newer modems"},	//
	{0x12d1, 0x14ac, "option", "0", "0", 2, NULL, "HUAWEI/Option newer modems"},	//
	{0x12d1, 0x14ae, "option", "0", "0", 2, NULL, "Vodafone (Huawei) K3806"},	//
	{0x12d1, 0x1446, "option", "0", "0", 2, &modeswitch_huawei, "HUAWEI/Option E1552 HSPA Modem"},	//
	{0x12d1, 0x14ad, "option", "0", "0", 2, &modeswitch_huawei, "Vodafone (Huawei) K3806"},	//
	{0x12d1, 0x14c1, "option", "0", "0", 2, &modeswitch_huawei, "Vodafone (Huawei) K4605"},	//
	{0x12d1, 0x14c6, "option", "0", "0", 2, NULL, "Vodafone (Huawei) K4605"},	//
	{0x12d1, 0x1520, "option", "0", "0", 2, &modeswitch_huawei, "Huawei K3765"},	//
	{0x12d1, 0x1465, "option", "0", "0", 2, NULL, "Huawei K3765"},	//
	{0x12d1, 0x1521, "option", "0", "0", 2, &modeswitch_huawei, "Huawei K4505"},	//
	{0x12d1, 0x1464, "option", "0", "0", 2, NULL, "Huawei K4505"},	//
	{0x12d1, 0x1521, "option", "0", "0", 2, &modeswitch_huawei, "Huawei R201"},	//
	{0x12d1, 0x1491, "option", "0", "0", 2, NULL, "Huawei R201"},	//
	{0x12d1, 0x1557, "option", "0", "0", 2, &modeswitch_huawei, "Huawei E173"},	//
	{0x12d1, 0x14a5, "option", "0", "0", 2, NULL, "Huawei E173"},	//

	{0x0af0, 0x7011, "option", "0", "0", 2, NULL, "HUAWEI/Option E301 HSUPA"},	//

	{0x1410, 0x5010, "option", "0", "0", 2, &modeswitch_usb760, "Novatel CDROM Mode"},	//
	{0x1410, 0x5020, "option", "0", "0", 2, &modeswitch_usb760, "Novatel MC990D CDROM Mode"},	//
	{0x1410, 0x5030, "option", "0", "0", 2, &modeswitch_usb760, "Novatel USB760 CDROM Mode"},	//
	{0x1410, 0x5031, "option", "0", "0", 2, &modeswitch_usb760, "Novatel USB760 3G CDROM Mode"},	//
	{0x1410, 0x5041, "option", "0", "0", 2, &modeswitch_usb760, "Novatel Generic MiFi 2352 / Vodafone MiFi 2352 CDROM Mode"},	//
	{0x1410, 0x4100, "option", "0", "0", 2, NULL, "Novatel U727 Modem Mode"},	//
	{0x1410, 0x4400, "option", "0", "0", 2, NULL, "Novatel Ovation MC930D/MC950D Modem Mode"},	//

	{0x1410, 0x7001, "option", "0", "0", 2, NULL, "Novatel Generic MiFi 2352 / Vodafone MiFi 2352 Modem Mode"},	//
	{0x1410, 0x7003, "option", "0", "0", 2, NULL, "Novatel Generic MiFi 2352 / Vodafone MiFi 2352 Modem Mode"},	//

	{0x1410, 0x6000, "option", "0", "0", 2, NULL, "Novatel USB760 Modem Mode"},	//
	{0x1410, 0x6002, "option", "0", "0", 2, NULL, "Novatel USB760 3G Modem Mode"},	//
//qualcomm
	{0x1e0e, 0x9000, "option", "2", "2", 3, NULL, "Option iCON 210, PROLiNK PHS100, Hyundai MB-810, A-Link 3GU Modem Mode"},	//
	{0x1e0e, 0x9200, "option", "2", "2", 3, NULL, "Option iCON 210, PROLiNK PHS100, Hyundai MB-810, A-Link 3GU Modem Mode"},	//
	{0x1e0e, 0xf000, "option", "2", "2", 3, &modeswitch_icon210, "Option iCON 210, PROLiNK PHS100, Hyundai MB-810, A-Link 3GU CDROM Mode"},	//
	{0x0af0, 0x6971, NULL, "hso", "hso", 0, &hsoinit_icon225, "Qualcomm ICON 225"},	//
//ericsson
	{0x0bdb, 0x1900, "option", "4", "4", 0, NULL, "Ericsson F3507g"},	//
	{0x0bdb, 0x1902, "option", "4", "4", 0, NULL, "Ericsson F3507g"},	//
//ONDA
	{0x19d2, 0x2000, "option", "1", "1", 2, &modeswitch_onda, "ONDA/ZTE (cdrom mode)"},	//
	{0x19d2, 0x0001, "option", "1", "3", 2, NULL, "ONDA MT505UP/ZTE (modem mode)"},	//
	{0x19d2, 0x0002, "option", "1", "3", 2, NULL, "ZTE ET502HS/MT505UP/MF632"},	//
	{0x19d2, 0x0003, "option", "1", "3", 2, &modeswitch_onda2, "ZTE MU351 (cdrom mode)"},	//
	{0x19d2, 0x0015, "option", "1", "3", 2, NULL, "ONDA MT505UP/ZTE (modem mode)"},	//
	{0x19d2, 0x0016, "option", "1", "3", 2, NULL, "ONDA MT505UP/ZTE (modem mode)"},	//
	{0x19d2, 0x0017, "option", "1", "3", 2, NULL, "ONDA MT505UP/ZTE (modem mode)"},	//
	{0x19d2, 0x0022, "option", "1", "3", 2, NULL, "Vodafone (ZTE) K2525 (modem mode)"},	//
	{0x19d2, 0x0026, "option", "1", "3", 2, &modeswitch_onda2, "ZTE AC581 (cdrom mode)"},	//
	{0x19d2, 0x0031, "option", "1", "2", 2, NULL, "ZTE MF110/MF112/MF626 (Variant) (modem mode)"},	//tested on MF626
	{0x19d2, 0x0037, "option", "1", "3", 2, NULL, "ONDA MT505UP/ZTE (modem mode)"},	//
	{0x19d2, 0x0040, "option", "1", "3", 2, &modeswitch_onda2, "Vodafone (ZTE) K2525 (cdrom mode)"},	//
	{0x19d2, 0x0052, "option", "1", "3", 2, NULL, "ONDA MT505UP/ZTE (modem mode)"},	//
	{0x19d2, 0x0053, "option", "1", "3", 2, &modeswitch_onda2, "ZTE MF110 (Variant) (modem mode)"},	//
	{0x19d2, 0x0055, "option", "1", "3", 2, NULL, "ONDA MT505UP/ZTE (modem mode)"},	//
	{0x19d2, 0x0063, "option", "1", "3", 2, NULL, "Vodafone K3565-Z HSDPA (modem mode)"},	// tested, working. i hope the other ZDA devices are working in the same way
	{0x19d2, 0x0064, "option", "1", "3", 2, NULL, "ZTE MF627 AU (modem mode)"},	//
	{0x19d2, 0x0083, "option", "1", "3", 2, &modeswitch_onda2, "ZTE MF110 (Variant) (cdrom mode)"},	//
	{0x19d2, 0x0094, "option", "1", "3", 2, NULL, "ZTE AC581 (modem mode)"},	//
	{0x19d2, 0x0101, "option", "1", "3", 2, &modeswitch_onda2, "Vodafone (ZTE) K4505-Z (cdrom mode)"},	//
	{0x19d2, 0x0103, "option", "1", "3", 2, &modeswitch_onda2, "ZTE MF112 (cdrom mode)"},	//
	{0x19d2, 0x0104, "option", "1", "3", 2, NULL, "Vodafone (ZTE) K4505-Z (modem mode)"},	//
	{0x19d2, 0x0108, "option", "1", "3", 2, NULL, "ONDA MT505UP/ZTE (modem mode)"},	//
	{0x19d2, 0x0115, "option", "1", "3", 2, &modeswitch_onda2, "ONDA MT505UP/ZTE (modem mode)"},	//
	{0x19d2, 0x0116, "option", "1", "3", 2, NULL, "ZTE MF651 (cdrom mode)"},	//
	{0x19d2, 0x0117, "option", "1", "2", 2, NULL, "ZTE MF112 (modem mode)"},	// tested, works
	{0x19d2, 0x0128, "option", "1", "3", 2, NULL, "ZTE MF651 (modem mode)"},	//
	{0x19d2, 0x0124, "option", "1", "3", 2, NULL, "ZTE MF110 (Variant) (modem mode)"},	//
	{0x19d2, 0x1001, "option", "1", "3", 2, &modeswitch_onda2, "Vodafone (ZTE) K3805-Z (cdrom mode)"},	//
	{0x19d2, 0x1003, "option", "1", "3", 2, NULL, "Vodafone (ZTE) K3805-Z (modem mode)"},	//
	{0x19d2, 0x1007, "option", "1", "3", 2, &modeswitch_onda2, "Vodafone (ZTE) K3570-Z (cdrom mode)"},	//
	{0x19d2, 0x1008, "option", "1", "3", 2, NULL, "Vodafone (ZTE) K3570-Z (modem mode)"},	//
	{0x19d2, 0x1009, "option", "1", "3", 2, &modeswitch_onda2, "Vodafone (ZTE) K3571-Z (cdrom mode)"},	//
	{0x19d2, 0x1010, "option", "1", "3", 2, NULL, "Vodafone (ZTE) K3571-Z (modem mode)"},	//
	{0x19d2, 0x1013, "option", "1", "3", 2, &modeswitch_onda2, "Vodafone (ZTE) K3806-Z (cdrom mode)"},	//
	{0x19d2, 0x1015, "option", "1", "3", 2, NULL, "Vodafone (ZTE) K3806-Z (modem mode)"},	//
	{0x19d2, 0xfff5, "option", "1", "3", 2, &modeswitch_onda2, "ZTE generic (cdrom mode)"},	//
	{0x19d2, 0xfff6, "option", "1", "3", 2, &modeswitch_onda2, "ZTE generic (cdrom mode)"},	//
	{0x19d2, 0xfff1, "option", "1", "3", 2, NULL, "ZTE generic (modem mode)"},	//
	{0x19d2, 0xffff, "option", "1", "3", 2, NULL, "ZTE generic (modem mode)"},	//

	{0xffff, 0xffff, NULL, NULL, NULL, 0, NULL, NULL}	//
};

char *get3GControlDevice(void)
{
#if defined(ARCH_broadcom) && !defined(HAVE_BCMMODERN)
	mkdir("/tmp/usb");
	eval("mount", "-t", "usbfs", "usb", "/tmp/usb");
//insmod("sierra");  //further investigation required (compass problem)
#endif
	int needreset = 1;
	char *ttsdevice = "/dev/usb/tts/0";
#ifdef HAVE_CAMBRIA
	int gpio1 = atoi(nvram_safe_get("gpio26"));
	int gpio2 = atoi(nvram_safe_get("gpio27"));
	int select = atoi(nvram_safe_get("wan_select"));
	switch (select) {
	case 1:
		if (gpio1 == 1 || gpio2 == 0)
			needreset = 0;
		else {
			gpio1 = 1;
			gpio2 = 0;
		}
		break;
	case 2:
		if (gpio1 == 0 || gpio2 == 1)
			needreset = 0;
		else {
			gpio1 = 0;
			gpio2 = 1;
		}
		break;
	case 3:
		if (gpio1 == 1 || gpio2 == 1)
			needreset = 0;
		else {
			gpio1 = 1;
			gpio2 = 1;
		}
		break;
	default:
		if (gpio1 == 0 && gpio2 == 0) {
			gpio1 = 1;
			gpio2 = 0;
		} else
			needreset = 0;
		break;
	}
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
	nvram_set("3gdata", "/dev/usb/tts/0");

	int devicecount = 0;
	while (devicelist[devicecount].vendor != 0xffff) {
		if (scanFor
		    (devicelist[devicecount].vendor,
		     devicelist[devicecount].product)) {
			fprintf(stderr, "%s detected\n",
				devicelist[devicecount].name);
			if (devicelist[devicecount].driver) {
				insmod("usbserial");
				insmod(devicelist[devicecount].driver);
			}
			if (devicelist[devicecount].datadevice) {
				static char data[32];
				if (!strcmp
				    (devicelist[devicecount].datadevice, "hso"))
					sprintf(data, "hso");
				else
					sprintf(data, "/dev/usb/tts/%s",
						devicelist[devicecount].
						datadevice);
				nvram_set("3gdata", data);
			}
			if (devicelist[devicecount].modeswitch) {
				char variant[32];
				sprintf(variant, "%d",
					devicelist[devicecount].modeswitch);
				nvram_set("3gnmvariant", variant);
			}
			//start custom setup, if defined
			if (devicelist[devicecount].customsetup)
				devicelist[devicecount].customsetup(needreset,
								    devicelist
								    [devicecount].controldevice);
			static char control[32];
			if (!strcmp
			    (devicelist[devicecount].controldevice, "hso"))
				sprintf(control, "hso");
			else
				sprintf(control, "/dev/usb/tts/%s",
					devicelist[devicecount].controldevice);
			return control;
		}
		devicecount++;
	}
	//not found, use generic implementation (tts0, all drivers)

	insmod("usbserial");
	insmod("sierra");
	insmod("option");
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
