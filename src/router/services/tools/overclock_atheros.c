/*
 * overclock_atheros.c
 *
 * Copyright (C) 2008 Sebastian Gottschall <gottschall@dd-wrt.com>
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
 * desc:
 * overclocks 2313 and 2316/17 or compatible wisoc boards with redboot or wistron zLoader to any 200 - 240 mhz (you must select it by i.e. nvram set cpuclk=220) 
 * just a dirty hack i found while playing with code
 * usage:
 * nvram set cpuclk=200
 * startservice overclock
 * 
 * valid cpuclk values are 184, 200, 220, 240 (ar2316)
 *                         180, 200, 220, 240 (ar2312 viper revision)
 */
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <shutils.h>
#include <bcmnvram.h>

/*
 * to prevent bricks and other troubles you should use this tool only
 * manually. it will not be implemented in the webgui 
 */

void fixup(FILE * in, unsigned int src, unsigned int dst)
{
	fprintf(stderr, "fixup 0x%08X to 0x%08X\n", src, dst);
	int f = ftell(in);

	fseek(in, 0, SEEK_END);
	int len = ftell(in);

	fseek(in, 0, SEEK_SET);
	int i;

	for (i = 0; i < len; i++) {
		fseek(in, i, SEEK_SET);
		unsigned int chk;

		fread(&chk, 4, 1, in);
		if (chk == src) {
			fprintf(stderr, "fixup found at 0x%08X\n", i);
			fseek(in, i, SEEK_SET);
			fwrite(&dst, 4, 1, in);
			fseek(in, f, SEEK_SET);
			return;
		}
	}
	fprintf(stderr, "warning. no fixup found\n");
	fseek(in, f, SEEK_SET);
}

void fixclk(FILE * in, int oldclk, int clk)
{
	// fixup amba clock
	int amba = oldclk / 2 * 1000000;
	unsigned int part1 = amba >> 16;
	unsigned int part2 = amba & 0x0000ffff;

	part1 = part1 | 0x3c020000;
	part2 = part2 | 0x34420000;
	int ambadst = clk / 2 * 1000000;
	unsigned int part1dst = ambadst >> 16;
	unsigned int part2dst = ambadst & 0x0000ffff;

	part1dst = part1dst | 0x3c020000;
	part2dst = part2dst | 0x34420000;
	fixup(in, part1, part1dst);
	fixup(in, part2, part2dst);
	// fixup cpu frequency
	/*
	 * //not required. code is not compiled into redboot amba = oldclk *
	 * 1000000; part1 = amba >> 16; part2 = amba & 0x0000ffff; part1 = part1
	 * | 0x3c020000; part2 = part2 | 0x34420000; ambadst = clk * 1000000;
	 * part1dst = ambadst >> 16; part2dst = ambadst & 0x0000ffff; part1dst =
	 * part1dst | 0x3c020000; part2dst = part2dst | 0x34420000;
	 * fixup(in,part1,part1dst); fixup(in,part2,part2dst);
	 */
}

void start_overclock(void)	// hidden feature. must be called with
				// "startservice overlock". then reboot the
				// unit
{
	long len;
	long i;

#ifdef HAVE_ALPHA
	FILE *in = fopen("/dev/mtdblock/0", "rb");
#elif HAVE_CA8
	FILE *in = fopen("/dev/mtdblock/2", "rb");	// zLoader Board Data 
#else
	FILE *in = fopen("/dev/mtdblock/0", "rb");
#endif
	FILE *out = fopen("/tmp/boot", "wb");

	fseek(in, 0, SEEK_END);
	len = ftell(in);
	fprintf(stderr, "size = %ld\n", len);
	fseek(in, 0, SEEK_SET);
	for (i = 0; i < len; i++)
		putc(getc(in), out);
	fclose(in);
	fclose(out);
	int clk = atoi(nvram_default_get("cpuclk", "180"));

	in = fopen("/tmp/boot", "r+b");
	fseek(in, 0xe64b, SEEK_SET);
	int zmul = getc(in);

	fseek(in, 0xcb, SEEK_SET);
	int vipermul = getc(in);

	fseek(in, 0x1e3, SEEK_SET);
	int div = getc(in);

	fseek(in, 0x1ef, SEEK_SET);
	int mul = getc(in);

	fseek(in, 0x17, SEEK_SET);
	int dir300div = getc(in);

	fseek(in, 0x23, SEEK_SET);
	int dir300mul = getc(in);

	fseek(in, 0x47, SEEK_SET);
	int dir300div2 = getc(in);

	fseek(in, 0x53, SEEK_SET);
	int dir300mul2 = getc(in);

	fseek(in, 0x93, SEEK_SET);
	int dir300div3 = getc(in);

	fseek(in, 0x9f, SEEK_SET);
	int dir300mul3 = getc(in);

	fseek(in, 0xdb, SEEK_SET);
	int isalfa1 = getc(in);
	int isalfa = 0;
	if (isalfa1 == 0x6c)
		isalfa = 1;

	int dir300 = 0;
	int dir3002 = 0;
	int dir3003 = 0;

	if (dir300div == 0x3 && dir300mul == 0x5c) {
		dir300 = 1;
		div = dir300div;
		mul = dir300mul;
	}
	if (dir300div == 0x1
	    && (dir300mul == 0x28 || dir300mul == 0x2c || dir300mul == 0x30)) {
		dir300 = 1;
		div = dir300div;
		mul = dir300mul;
	}

	if (dir300div2 == 0x3 && dir300mul2 == 0x5c) {
		dir3002 = 1;
		div = dir300div2;
		mul = dir300mul2;
	}
	if (dir300div2 == 0x1
	    && (dir300mul2 == 0x28 || dir300mul2 == 0x2c
		|| dir300mul2 == 0x30)) {
		dir3002 = 1;
		div = dir300div2;
		mul = dir300mul2;
	}

	if (dir300div3 == 0x3 && dir300mul3 == 0x5c) {
		dir3003 = 1;
		div = dir300div3;
		mul = dir300mul3;
	}
	if (dir300div3 == 0x1
	    && (dir300mul3 == 0x28 || dir300mul3 == 0x2c
		|| dir300mul3 == 0x30)) {
		dir3003 = 1;
		div = dir300div3;
		mul = dir300mul3;
	}

	if (div == 0x3 && mul == 0x5c) {
		fprintf(stderr,
			"ap51/ap61/ap65 (ar2315/ar2316/ar2317/ar2318) found\n");
		if (dir300)
			fseek(in, 0x17, SEEK_SET);
		else if (dir3002)
			fseek(in, 0x47, SEEK_SET);
		else if (dir3003)
			fseek(in, 0x93, SEEK_SET);
		else
			fseek(in, 0x1e3, SEEK_SET);
		putc(0x1, in);
		if (dir300)
			fseek(in, 0x23, SEEK_SET);
		else if (dir3002)
			fseek(in, 0x53, SEEK_SET);
		else if (dir3003)
			fseek(in, 0x9f, SEEK_SET);
		else
			fseek(in, 0x1ef, SEEK_SET);
		if (clk == 200) {
			if (mul == 0x2c) {
				fixclk(in, 220, 200);
			} else if (mul == 0x30) {
				fixclk(in, 240, 200);
			} else
				fixclk(in, 184, 200);
			putc(0x28, in);	// 0x2c for 220 mhz 0x30 for 240 mhz
		} else if (clk == 220) {
			if (mul == 0x28) {
				fixclk(in, 200, 220);
			} else if (mul == 0x30) {
				fixclk(in, 240, 220);
			} else
				fixclk(in, 184, 220);

			putc(0x2c, in);	// 0x2c for 220 mhz 0x30 for 240 mhz
		} else if (clk == 240) {
			if (mul == 0x28) {
				fixclk(in, 200, 240);
			} else if (mul == 0x2c) {
				fixclk(in, 220, 240);
			} else
				fixclk(in, 184, 240);
			putc(0x30, in);	// 0x2c for 220 mhz 0x30 for 240 mhz
		} else {
			fixclk(in, 184, 200);
			nvram_set("cpuclk", "200");
			nvram_commit();
			clk = atoi(nvram_default_get("cpuclk", "180"));
			putc(0x28, in);	// 0x2c for 220 mhz 0x30 for 240 mhz
		}

		fclose(in);
		eval("mtd", "-f", "write", "/tmp/boot", "RedBoot");
	} else if (div == 0x1 && (mul == 0x28 || mul == 0x2c || mul == 0x30)) {
		fprintf(stderr,
			"ap51/ap61/ap65 (ar2315/ar2316/ar2317/ar2318) found\n");
		if (clk == 200 && mul == 0x28) {
			fprintf(stderr, "board already clocked to 200mhz\n");
			fclose(in);
			return;
		}
		if (clk == 220 && mul == 0x2c) {
			fprintf(stderr, "board already clocked to 220mhz\n");
			fclose(in);
			return;
		}
		if (clk == 240 && mul == 0x30) {
			fprintf(stderr, "board already clocked to 240mhz\n");
			fclose(in);
			return;
		}
		if (dir300)
			fseek(in, 0x17, SEEK_SET);
		else if (dir3002)
			fseek(in, 0x47, SEEK_SET);
		else if (dir3003)
			fseek(in, 0x93, SEEK_SET);
		else
			fseek(in, 0x1e3, SEEK_SET);
		if (clk == 184)
			putc(0x3, in);	// set divisor 5 = 40/5 = 8 mhz base which
		// allows 184 clock setting
		else
			putc(0x1, in);
		if (dir300)
			fseek(in, 0x23, SEEK_SET);
		else if (dir3002)
			fseek(in, 0x57, SEEK_SET);
		else if (dir3003)
			fseek(in, 0x9f, SEEK_SET);
		else
			fseek(in, 0x1ef, SEEK_SET);
		if (clk == 184) {
			if (mul == 0x28) {
				fixclk(in, 200, 184);
			} else if (mul == 0x2c) {
				fixclk(in, 220, 184);
			} else if (mul == 0x30) {
				fixclk(in, 240, 184);
			}
			putc(0x5c, in);	// 0x2c for 220 mhz 0x30 for 240 mhz
		} else if (clk == 200) {
			if (mul == 0x2c) {
				fixclk(in, 220, 200);
			} else if (mul == 0x30) {
				fixclk(in, 240, 200);
			} else
				fixclk(in, 184, 200);
			putc(0x28, in);	// 0x2c for 220 mhz 0x30 for 240 mhz
		} else if (clk == 220) {
			if (mul == 0x28) {
				fixclk(in, 200, 220);
			} else if (mul == 0x30) {
				fixclk(in, 240, 220);
			} else
				fixclk(in, 184, 220);

			putc(0x2c, in);	// 0x2c for 220 mhz 0x30 for 240 mhz
		} else if (clk == 240) {
			if (mul == 0x28) {
				fixclk(in, 200, 240);
			} else if (mul == 0x2c) {
				fixclk(in, 220, 240);
			} else
				fixclk(in, 184, 240);
			putc(0x30, in);	// 0x2c for 220 mhz 0x30 for 240 mhz
		} else {
			fixclk(in, 184, 200);
			nvram_set("cpuclk", "200");
			nvram_commit();
			clk = atoi(nvram_default_get("cpuclk", "180"));
			putc(0x28, in);	// 0x2c for 220 mhz 0x30 for 240 mhz
		}
		fclose(in);
		eval("mtd", "-f", "write", "/tmp/boot", "RedBoot");
	} else if (vipermul == 0x9 || vipermul == 0xa || vipermul == 0xb
		   || vipermul == 0xc || vipermul == 0x17) {

		if (vipermul == 0x17) {
			fprintf(stderr, "weired alfa clocksetting found\n");
			fseek(in, 0xce, SEEK_SET);
			if (getc(in) == 0x32) {
				if (getc(in) == 0x45) {
					fprintf(stderr,
						"correct clock setting\n");
					fseek(in, 0xcb, SEEK_SET);
					putc(0x9, in);
					fseek(in, 0xce, SEEK_SET);
					putc(0x12, in);
					putc(0x45, in);
//                  fseek( in, 0xe4, SEEK_SET ); 
//                  putc( 0x0, in ); // nop out scratch register
//                  putc( 0x0, in );
					vipermul = 0x9;
				} else
					exit(1);
			} else
				exit(1);
		}
		fprintf(stderr, "viper (ar2313) found\n");
		if (clk == 180 && vipermul == 0x9) {
			fprintf(stderr, "board already clocked to 180mhz\n");
			fclose(in);
			return;
		}
		if (clk == 200 && vipermul == 0xa) {
			fprintf(stderr, "board already clocked to 200mhz\n");
			fclose(in);
			return;
		}
		if (clk == 220 && vipermul == 0xb) {
			fprintf(stderr, "board already clocked to 220mhz\n");
			fclose(in);
			return;
		}
		if (clk == 240 && vipermul == 0xc) {
			fprintf(stderr, "board already clocked to 240mhz\n");
			fclose(in);
			return;
		}
		fseek(in, 0xcb, SEEK_SET);
		if (clk == 180)
			putc(0x9, in);	// 0x2c for 220 mhz 0x30 for 240 mhz
		else if (clk == 200)
			putc(0xa, in);	// 0x2c for 220 mhz 0x30 for 240 mhz
		else if (clk == 220)
			putc(0xb, in);	// 0x2c for 220 mhz 0x30 for 240 mhz
		else if (clk == 240)
			putc(0xc, in);	// 0x2c for 220 mhz 0x30 for 240 mhz
		else {
			nvram_set("cpuclk", "220");
			clk = atoi(nvram_default_get("cpuclk", "180"));
			nvram_commit();
			putc(0xb, in);	// 0x2c for 220 mhz 0x30 for 240 mhz
		}
		if (isalfa) {
			fprintf(stderr,
				"correct scratch register for alfa ap48\n");
			int realclock = clk * 1000000;
			fseek(in, 0xde, SEEK_SET);
			putc((realclock >> 24) & 0xff, in);
			putc((realclock >> 16) & 0xff, in);
			fseek(in, 0xe2, SEEK_SET);
			putc((realclock >> 8) & 0xff, in);
			putc(realclock & 0xff, in);
		}
		fclose(in);
		eval("mtd", "-f", "write", "/tmp/boot", "RedBoot");
	} else if (zmul == 0x9 || zmul == 0xa || zmul == 0xb || zmul == 0xc)	// special 
		// handling 
		// for 
		// zLoader 
		// based 
		// boards
	{
		fprintf(stderr, "viper (ar2313) found (zLoader)\n");
		if (clk == 180 && zmul == 0x9) {
			fprintf(stderr, "board already clocked to 180mhz\n");
			fclose(in);
			return;
		}
		if (clk == 200 && zmul == 0xa) {
			fprintf(stderr, "board already clocked to 200mhz\n");
			fclose(in);
			return;
		}
		if (clk == 220 && zmul == 0xb) {
			fprintf(stderr, "board already clocked to 220mhz\n");
			fclose(in);
			return;
		}
		if (clk == 240 && zmul == 0xc) {
			fprintf(stderr, "board already clocked to 240mhz\n");
			fclose(in);
			return;
		}
		fseek(in, 0xe64b, SEEK_SET);
		if (clk == 180)
			putc(0x9, in);	// 0x2c for 220 mhz 0x30 for 240 mhz
		else if (clk == 200)
			putc(0xa, in);	// 0x2c for 220 mhz 0x30 for 240 mhz
		else if (clk == 220)
			putc(0xb, in);	// 0x2c for 220 mhz 0x30 for 240 mhz
		else if (clk == 240)
			putc(0xc, in);	// 0x2c for 220 mhz 0x30 for 240 mhz
		else {
			nvram_set("cpuclk", "220");
			clk = atoi(nvram_default_get("cpuclk", "180"));
			nvram_commit();
			putc(0xb, in);	// 0x2c for 220 mhz 0x30 for 240 mhz
		}

		unsigned int myclk = clk * 1000000;
		unsigned short part1, part2;

		part1 = myclk >> 16;
		part2 = myclk & 0x0000ffff;

		fprintf(stderr, "patch uart init with %X:%X\n", part1, part2);
		fseek(in, 0xed16, SEEK_SET);
		fwrite(&part1, 2, 1, in);
		fseek(in, 0xed1a, SEEK_SET);
		fwrite(&part2, 2, 1, in);

		fclose(in);
		eval("mtd", "-f", "write", "/tmp/boot", "bdata");
	} else {
		fprintf(stderr, "unknown board or no redboot found\n");
		fclose(in);
		exit(1);
	}
	fprintf(stderr, "board is now clocked at %d mhz, please reboot\n", clk);
}

// int main (int argc, char *argv[]) { start_overclock (); } 
