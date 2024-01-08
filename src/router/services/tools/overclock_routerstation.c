/*
 * overclock_routerstation.c
 *
 * Copyright (C) 2009 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
 * usage:
 * nvram set cpuclk=800
 * startservice overclock
 * 
 * valid cpuclk values are 200, 300 ,333, 400, 600, 680, 720, 800
 */
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <shutils.h>
#include <bcmnvram.h>

int overclock(FILE *out, char *freq, int value)
{
	fseek(out, 0xd3, SEEK_SET);
	int val = getc(out);
	if (val == value) {
		fprintf(stderr, "board already clocked to %sMhz\n", freq);
		return -1;
	}
	fseek(out, 0xd3, SEEK_SET);
	putc(value, out);
	return 0;
}

int overclock_3(FILE *out, char *freq, int value)
{
	fseek(out, 0x3f, SEEK_SET);
	int val = getc(out);
	if (val == value) {
		fprintf(stderr, "board already clocked to %sMhz\n", freq);
		return -1;
	}
	fseek(out, 0x3f, SEEK_SET);
	putc(value, out);
	return 0;
}

void start_overclock(void) // hidden feature. must be called with
	// "startservice overlock". then reboot the
	// unit
{
	long len;
	long i;

	FILE *in = fopen("/dev/mtdblock/0", "rb");
	fseek(in, 0, SEEK_END);
	len = ftell(in);
	rewind(in);
	char check[8];
	char check2[8];
	char values[8] = { 0x24, 0x08, 0x00, 0xaa, 0x15, 0x09, 0x00, 0x04 };
	char values2[8] = { 0x24, 0x0a, 0x00, 0x0f, 0x11, 0x2a, 0x00, 0x04 };
	fseek(in, 0xc0, SEEK_SET);
	fread(check, 1, 8, in);
	fseek(in, 0xc4, SEEK_SET);
	fread(check2, 1, 8, in);
	int ret1 = 0xff;
	int ret2 = 0xff;
	int ret3 = 0xff;
	if ((ret1 = memcmp(check, values, 8)) &&
	    (ret2 = memcmp(check2, values, 8)) &&
	    (ret3 = memcmp(check2, values2, 8))) {
		fprintf(stderr,
			"no compatible routerstation bootloader found\n");
		fclose(in);
		return;
	}
	if (!ret1)
		fprintf(stderr, "bootloader rev1 found\n");
	if (!ret2)
		fprintf(stderr, "bootloader rev2 found\n");
	if (!ret3)
		fprintf(stderr, "bootloader rev3 found\n");
	FILE *out = fopen("/tmp/boot", "w+b");
	rewind(in);
	for (i = 0; i < len; i++)
		putc(getc(in), out);
	fclose(in);
	int ret = 1;
	if (!ret3) {
		if (nvram_matchi("cpuclk", 200))
			ret = overclock_3(out, "200", 0x1);
		if (nvram_matchi("cpuclk", 300))
			ret = overclock_3(out, "300", 0x2);
		if (nvram_matchi("cpuclk", 333))
			ret = overclock_3(out, "333", 0x3);
		if (nvram_matchi("cpuclk", 400))
			ret = overclock_3(out, "400", 0x6);
		if (nvram_matchi("cpuclk", 600))
			ret = overclock_3(out, "600", 0x7);
		if (nvram_matchi(
			    "cpuclk",
			    680)) //special ubiquiti setting with different ddram clock settings
			ret = overclock_3(out, "680", 0xc);
		if (nvram_matchi("cpuclk", 720))
			ret = overclock_3(out, "720",
					  0xe); //need to be validated
		if (nvram_matchi("cpuclk", 800))
			ret = overclock_3(out, "800", 0xf);

	} else {
		if (nvram_matchi("cpuclk", 200))
			ret = overclock(out, "200", 0x1);
		if (nvram_matchi("cpuclk", 300))
			ret = overclock(out, "300", 0x2);
		if (nvram_matchi("cpuclk", 333))
			ret = overclock(out, "333", 0x3);
		if (nvram_matchi("cpuclk", 400))
			ret = overclock(out, "400", 0x6);
		if (nvram_matchi("cpuclk", 600))
			ret = overclock(out, "600", 0x7);
		if (nvram_matchi(
			    "cpuclk",
			    680)) //special ubiquiti setting with different ddram clock settings
			ret = overclock(out, "680", 0xa);
		if (nvram_matchi("cpuclk", 720))
			ret = overclock(out, "720",
					0x1e); //magic atheros values
		if (nvram_matchi("cpuclk", 800))
			ret = overclock(out, "800", 0x1f);
	}
	fclose(out);
	if (!ret) {
		fprintf(stderr, "write new bootloader\n");
		eval("mtd", "-f", "write", "/tmp/boot", "RedBoot");
		fprintf(stderr, "board now clocked to %sMhz\n",
			nvram_safe_get("cpuclk"));
	}
	if (ret == 1) {
		fprintf(stderr,
			"no clock defined, please adjust the \"cpuclk\" nvram parameter. in example \"nvram set cpuclk=800\"");
	}
}

// int main (int argc, char *argv[]) { start_overclock (); }
