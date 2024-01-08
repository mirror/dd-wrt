/*
 * mmc.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#ifdef HAVE_MMC
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <sys/mount.h>
#include <stdio.h>
#include <stdlib.h>

void start_mmc(void)
{
	if (nvram_matchi("mmc_enable", 1)) {
#ifdef HAVE_FONERA
		int res = insmod("mmc");

		if (!res) {
			// device detected
			insmod("mbcache ext2");

			if (mount("/dev/mmc", "/mmc", "ext2", MS_MGC_VAL | MS_NOATIME, NULL)) {
				// device not formated
				eval("mkfs.ext2", "-F", "-b", "1024", "/dev/mmc");
				mount("/dev/mmc", "/mmc", "ext2", MS_MGC_VAL | MS_NOATIME, NULL);
			}
		}
#else
		int res = 1;
		int mmc_di = 0, mmc_do = 0, mmc_clk = 0, mmc_cs = 0;
		char dddi[16], dddo[16], ddclk[16], ddcs[16];

		if (nvram_matchi("mmc_gpio", 1)) // manual gpio asigments
		{
			mmc_di = strtoul(nvram_safe_get("mmc_di"), NULL, 0);
			mmc_do = strtoul(nvram_safe_get("mmc_do"), NULL, 0);
			mmc_clk = strtoul(nvram_safe_get("mmc_clk"), NULL, 0);
			mmc_cs = strtoul(nvram_safe_get("mmc_cs"), NULL, 0);
		} else // auto gpio based on router brand/model
		{
			switch (getRouterBrand()) {
			case ROUTER_WRT54G:
				if (nvram_match("boardtype",
						"0x0467")) // v4 or GL
					mmc_di = 2;
				else
					mmc_di = 5;
				mmc_do = 4;
				mmc_clk = 3;
				mmc_cs = 7;
				break;
			case ROUTER_ASUS_WL500GD:
				mmc_di = 5;
				mmc_do = 4;
				mmc_clk = 1;
				mmc_cs = 7;
				break;
			case ROUTER_BUFFALO_WHRG54S:
				mmc_di = 5;
				mmc_do = 6;
				mmc_clk = 3;
				mmc_cs = 7;
				break;
			case ROUTER_BUFFALO_WZRRSG54:
				mmc_di = 5;
				mmc_do = 4;
				mmc_clk = 3;
				mmc_cs = 7;
				break;
			}
		}
		/*		
		sprintf(dddi, "DDDI=0x%X", 1 << mmc_di);
		sprintf(dddo, "DDDO=0x%X", 1 << mmc_do);
		sprintf(ddclk, "DDCLK=0x%X", 1 << mmc_clk);
		sprintf(ddcs, "DDCS=0x%X", 1 << mmc_cs);
*/
		sprintf(dddi, "din=%d", mmc_di);
		sprintf(dddo, "dout=%d", mmc_do);
		sprintf(ddclk, "clk=%d", mmc_clk);
		sprintf(ddcs, "cs=%d", mmc_cs);

		if ((mmc_di + mmc_do + mmc_clk + mmc_cs) > 5) // eval only
			// if at
			// least 0,
			// 1, 2, 3
			res = eval("insmod", "mmc", dddi, dddo, ddclk,
				   ddcs); // eval("insmod","mmc",
		// "DDDI=0x04",
		// "DDDO=0x10",
		// "DDCLK=0x08",
		// "DDCS=0x80");

		if (!res) {
			// device detected
			insmod("ext2");

			if (mount("/dev/mmc/disc0/part1", "/mmc", "ext2", MS_MGC_VAL | MS_NOATIME, NULL)) {
				// device not formated
				eval("mkfs.ext2", "-F", "-b", "1024", "/dev/mmc/disc0/part1");
				mount("/dev/mmc/disc0/part1", "/mmc", "ext2", MS_MGC_VAL | MS_NOATIME, NULL);
			}
		}
#endif
	}
}

#endif

/*
	 * For Asus: SD_CLK=1, SD_DO=4, SD_DI=5, SD_CS=7
	 * 
	 * GPIO PIN 0 0x01 GPIO PIN 1: 0x02 GPIO PIN 2: 0x04 GPIO PIN 3: 0x08
	 * GPIO PIN 4: 0x10 GPIO PIN 5: 0x20 GPIO PIN 6: 0x40 GPIO PIN 7: 0x80
	 * GPIO PIN 8: 0x100 GPIO PIN 9: 0x200 GPIO PIN 10: 0x400 GPIO PIN 11:
	 * 0x800 GPIO PIN 12: 0x1000 GPIO PIN 13: 0x2000 GPIO PIN 14: 0x4000
	 * GPIO PIN 15: 0x8000 
	 */
