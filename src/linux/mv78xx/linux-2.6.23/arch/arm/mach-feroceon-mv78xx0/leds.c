/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/init.h>

#include <asm/hardware.h>
#include <asm/leds.h>
#include <asm/system.h>
#include <asm/mach-types.h>
#include "boardEnv/mvBoardEnvLib.h"
#include "cpu/mvCpu.h"

static	u32		use7seg = 0;
static	u32		last_jiffies = 0;
static	u32		led_val = 0;


void mv_leds_hearbeat(void)
{
    u32 sec = jiffies_to_msecs(jiffies - last_jiffies) / 1000;

    if (!sec)
	return;

    led_val = (led_val + sec) % 10;

    if (use7seg)
	mvBoardDebug7Seg(led_val);
    else
        mvBoardDebugLed(led_val & 0x1);

    last_jiffies = jiffies;
}

static int __init leds_init(void)
{

	switch (mvBoardIdGet()) {
		/* when 7seg present */
		case DB_78XX0_ID:	
			use7seg = 1;
			break;
		case DB_78200_ID:
			/*LED heatbeat for CPU0 only*/
			use7seg = !whoAmI();
			break;
		default:
			use7seg = 0;
	}

	return 0;
}

__initcall(leds_init);

