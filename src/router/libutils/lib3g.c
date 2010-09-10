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
	while (1) {
		char sysfs[64];
		sprintf(sysfs, "/sys/bus/usb/devices/%d-0:1.0/bInterfaceNumber",
			count);
		FILE *probe = fopen(sysfs, "rb");
		if (!probe)
			return 0;
		fclose(probe);
		sprintf(sysfs, "/sys/bus/usb/devices/%d-1/idProduct", count);
		FILE *modem = fopen(sysfs, "rb");
		if (!modem) {
			count++;
			continue;
		}
		int idProduct;
		int idVendor;
		fscanf(modem, "%X", &idProduct);
		fclose(modem);
		sprintf(sysfs, "/sys/bus/usb/devices/%d-1/idVendor", count);
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
	if (scanFor(0x1199, 0x6880)) {
		//sierra wireless 
		fprintf(stderr, "Sierra Wireless Compass 885 deteted\n");
		insmod("usbserial");
		insmod("sierra");
		nvram_set("3gdata", "/dev/usb/tts/4");
		nvram_set("3gnmvariant", "1");
		return "/dev/usb/tts/3";
	}
	if (scanFor(0x1199, 0x6890)) {
		//sierra wireless 
		fprintf(stderr, "Sierra Wireless Compass 888 deteted\n");
		insmod("usbserial");
		insmod("sierra");
		nvram_set("3gdata", "/dev/usb/tts/4");
		nvram_set("3gnmvariant", "1");
		return "/dev/usb/tts/3";
	}
	if (scanFor(0x1199, 0x6893)) {
		//sierra wireless 
		fprintf(stderr, "Sierra Wireless Compass 889 deteted\n");
		insmod("usbserial");
		insmod("sierra");
		nvram_set("3gdata", "/dev/usb/tts/4");
		nvram_set("3gnmvariant", "1");
		return "/dev/usb/tts/3";
	}
	if (scanFor(0x1199, 0x68a3)) {
		//sierra wireless 
		fprintf(stderr, "Sierra Wireless Compass 889 deteted\n");
		insmod("usbserial");
		insmod("sierra");
		nvram_set("3gdata", "/dev/usb/tts/4");
		nvram_set("3gnmvariant", "1");
		return "/dev/usb/tts/3";
	}
	if (scanFor(0x1199, 0x683C)) {
		//sierra wireless 
		fprintf(stderr, "Sierra Wireless MC8790\n");
		nvram_set("3gdata", "/dev/usb/tts/4");
		insmod("usbserial");
		insmod("sierra");
		if (needreset)
			checkreset("/dev/usb/tts/3");
		nvram_set("3gnmvariant", "1");
		return "/dev/usb/tts/3";
	}
	if (scanFor(0x1199, 0x683D)) {
		//sierra wireless 
		fprintf(stderr, "Sierra Wireless MC8790\n");
		nvram_set("3gdata", "/dev/usb/tts/4");
		insmod("usbserial");
		insmod("sierra");
		if (needreset)
			checkreset("/dev/usb/tts/3");
		nvram_set("3gnmvariant", "1");
		return "/dev/usb/tts/3";
	}
	if (scanFor(0x1199, 0x683E)) {
		//sierra wireless 
		fprintf(stderr, "Sierra Wireless MC8790\n");
		nvram_set("3gdata", "/dev/usb/tts/4");
		insmod("usbserial");
		insmod("sierra");
		if (needreset)
			checkreset("/dev/usb/tts/3");
		nvram_set("3gnmvariant", "1");
		return "/dev/usb/tts/3";
	}
	if (scanFor(0x1199, 0x68A3)) {
		//sierra wireless 
		fprintf(stderr, "Sierra Wireless MC8700\n");
		nvram_set("3gdata", "/dev/usb/tts/0");
		insmod("usbserial");
		insmod("sierra");
		if (needreset)
			checkreset("/dev/usb/tts/2");
		nvram_set("3gnmvariant", "1");
		return "/dev/usb/tts/2";
	}
	if (scanFor(0x1199, 0x6812)) {
		//sierra wireless mc 8775V
		fprintf(stderr,
			"Sierra Wireless MC 8775V detected\nreset card\n");
		insmod("usbserial");
		insmod("sierra");
		if (needreset)
			checkreset("/dev/usb/tts/2");
		nvram_set("3gnmvariant", "1");
		return "/dev/usb/tts/2";
	}
	if (scanFor(0x12d1, 0x1003)) {
		//huawei
		fprintf(stderr, "HUAWEI/Option E172 detected\n");
		insmod("usbserial");
		insmod("option");
		nvram_set("3gnmvariant", "2");
		return "/dev/usb/tts/0";
	}
	if (scanFor(0x0af0, 0x7011)) {
		//huawei
		fprintf(stderr, "HUAWEI/Option E301 HSUPA detected\n");
		insmod("usbserial");
		insmod("option");
		nvram_set("3gnmvariant", "2");
		return "/dev/usb/tts/0";
	}
	if (scanFor(0x12d1, 0x1001)) {
		//huawei
		fprintf(stderr, "HUAWEI/Option E600 detected\n");
		insmod("usbserial");
		insmod("option");
		nvram_set("3gnmvariant", "2");
		return "/dev/usb/tts/0";
	}
	if (scanFor(0x12d1, 0x1003)) {
		//huawei
		fprintf(stderr, "HUAWEI/Option EC270 detected\n");
		insmod("usbserial");
		insmod("option");
		nvram_set("3gnmvariant", "2");
		return "/dev/usb/tts/0";
	}
	if (scanFor(0x12d1, 0x1412)) {
		//huawei
		fprintf(stderr, "HUAWEI/Option EC168 detected\n");
		insmod("usbserial");
		insmod("option");
		nvram_set("3gnmvariant", "2");
		return "/dev/usb/tts/0";
	}
	if (scanFor(0x12d1, 0x1412)) {
		//huawei
		fprintf(stderr, "HUAWEI/Option EC168 detected\n");
		insmod("usbserial");
		insmod("option");
		nvram_set("3gnmvariant", "2");
		return "/dev/usb/tts/0";
	}

	if (scanFor(0x12d1, 0x1446)) {
		//huawei
		fprintf(stderr, "HUAWEI/Option E1550 detected\n");
		system("usb_modeswitch -v 0x12d1 -p 0x1446 -m 1 55534243000000000000000000000011060000000000000000000000000000");
		sleep(2);
		insmod("usbserial");
		insmod("option");
		nvram_set("3gnmvariant", "2");
		return "/dev/usb/tts/0";
	}

	if (scanFor(0x12d1, 0x1001)) { //if E1550 is already switched, it will get 1001 as product id
		//huawei
		fprintf(stderr, "HUAWEI/Option detected\n");
		insmod("usbserial");
		insmod("option");
		nvram_set("3gnmvariant", "2");
		return "/dev/usb/tts/0";
	}
	if (scanFor(0x1410, 0x5030)) { //cdrom mode, switch to modem mode
		//huawei
		fprintf(stderr, "Novatel USB760 CDROM Mode detected\n");
		system("usb_modeswitch -v 0x1410 -p 0x6000 -m 1 5553424312345678000000000000061b000000020000000000000000000000");
		sleep(2);
		insmod("usbserial");
		insmod("option");
		nvram_set("3gnmvariant", "2");
		return "/dev/usb/tts/0";
	}


	if (scanFor(0x1410, 0x6000)) { //already modem mode 
		//huawei
		fprintf(stderr, "Novatel USB760 Modem Mode detected\n");
//		system("usb_modeswitch -v 0x1410 -p 0x6000 -m 1 5553424312345678000000000000061b000000020000000000000000000000");
//		sleep(2);
		insmod("usbserial");
		insmod("option");
		nvram_set("3gnmvariant", "2");
		return "/dev/usb/tts/0";
	}


	if (scanFor(0x1e0e, 0x9000)) {
		//huawei
		fprintf(stderr, "QUALCOMM ICON 210 detected\n");
		nvram_set("3gdata", "/dev/usb/tts/2");
		insmod("usbserial");
		insmod("option");
		nvram_set("3gnmvariant", "3");
		return "/dev/usb/tts/2";
	}

	if (scanFor(0x1e0e, 0xf000)) {
		//huawei
		fprintf(stderr, "QUALCOMM ICON 210 detected\n");
		FILE *out = fopen("/tmp/usb_modeswitch.conf", "wb");
		fprintf(out, "DefaultVendor=0x1e0e\n");
		fprintf(out, "DefaultProduct=0xf000\n");
		fprintf(out, "TargetVendor=0x1e0e\n");
		fprintf(out, "TargetProduct=0x9000\n");
		fprintf(out,
			"MessageContent=\"555342431234567800000000000006bd000000020000000000000000000000\"\n");
		fprintf(out, "ResponseEndpoint=0x01\n");
		fclose(out);
		system("usb_modeswitch -c /tmp/usb_modeswitch.conf");
		sleep(2);
		insmod("usbserial");
		insmod("option");
		nvram_set("3gdata", "/dev/usb/tts/2");
		nvram_set("3gnmvariant", "3");
		return "/dev/usb/tts/2";
	}

	if (scanFor(0x0af0, 0x6971)) {
		//huawei
		fprintf(stderr, "QUALCOMM ICON 225 detected\n");
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
		return "hso";
	}

	if (scanFor(0x1199, 0x6832)) {
		//sierra wireless mc 8780
		fprintf(stderr,
			"Sierra Wireless MC 8780 detected\nreset card\n");
		insmod("usbserial");
		insmod("sierra");
		if (needreset)
			checkreset("/dev/usb/tts/2");
		nvram_set("3gnmvariant", "1");
		return "/dev/usb/tts/2";
	}


	if (scanFor(0x0bdb, 0x1902) || scanFor(0x0bdb, 0x1900)) {
		//sierra wireless mc 8780
		fprintf(stderr,
			"Ericsson F3507g detected\n");
		insmod("usbserial");
		insmod("option");
		nvram_set("3gdata", "/dev/usb/tts/1");
		return "/dev/usb/tts/0";
	}
		
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
