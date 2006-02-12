/*
 * Copyright 2004 PMC-Sierra Inc.
 * Author: Manish Lachwani (lachwani@pmc-sierra.com)
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/irq.h>
#include <linux/time.h>
#include <asm/bootinfo.h>
#include <asm/page.h>
#include <asm/irq.h>
#include <asm/processor.h>
#include <asm/time.h>
#include <asm/reboot.h>
#include <asm/addrspace.h>

#include "setup.h"

unsigned long cpu_clock;

extern void big_sur_restart(char *command);
extern void big_sur_halt(void);
extern void big_sur_power_off(void);

void __init bus_error_init(void)
{
	/* Do Nothing */
}

void big_sur_timer_setup(struct irqaction *irq)
{
	/* Rm7000 timer and not the Xilinx timer */
	setup_irq(7, irq);
}

#define	EPOCH		2000
#define	BCD_TO_BIN(val)	(((val)&15) + ((val)>>4)*10)
#define	BIN_TO_BCD(val)	((((val)/10)<<4) + (val)%10)

static int rtc_ds_get_time(void)
{
	unsigned int year, month, day, hour, minute, second;
	unsigned int century;

	BIG_SUR_RTC_WRITE(0x40, 0x7f8);

	second = BCD_TO_BIN(BIG_SUR_RTC_READ(0x7f8 + 1) & 0x7f);
	minute = BCD_TO_BIN(BIG_SUR_RTC_READ(0x7f8 + 2));
	hour = BCD_TO_BIN(BIG_SUR_RTC_READ(0x7f8 + 3));
	day = BCD_TO_BIN(BIG_SUR_RTC_READ(0x7f8 + 5));
	month = BCD_TO_BIN(BIG_SUR_RTC_READ(0x7f8 + 6));
	year = BCD_TO_BIN(BIG_SUR_RTC_READ(0x7f8 + 7));
	century = BCD_TO_BIN(BIG_SUR_RTC_READ(0x7f8) & 0x3f);

	BIG_SUR_RTC_WRITE(0, 0x7f8);
	year += century * 100;

	return mktime(year, month, day, hour, minute, second);
}

extern void to_tm(unsigned long tim, struct rtc_time * tm);

static int rtc_ds_set_time(unsigned long t)
{
	struct rtc_time tm;
	u8 year, month, day, hour, minute, second;
	u8 cmos_year, cmos_month, cmos_day, cmos_hour, cmos_minute, cmos_second;
	int cmos_century;

	BIG_SUR_RTC_WRITE(0x40, 0x7f8);
	cmos_second = (u8)(BIG_SUR_RTC_READ(0x7f8 + 1) & 0x7f);
	cmos_minute = (u8)(BIG_SUR_RTC_READ(0x7f8 + 2));
	cmos_hour = (u8)(BIG_SUR_RTC_READ(0x7f8 + 3));
	cmos_day = (u8)(BIG_SUR_RTC_READ(0x7f8 + 4));
	cmos_month = (u8)(BIG_SUR_RTC_READ(0x7f8 + 6));
	cmos_year = (u8)(BIG_SUR_RTC_READ(0x7f8 + 7));
	cmos_century = BIG_SUR_RTC_READ(0x7f8) & 0x3f;

	BIG_SUR_RTC_WRITE(0x80, 0x7f8);

	to_tm(t, &tm);
	
	year = BIN_TO_BCD(tm.tm_year - EPOCH);
	if (year != cmos_year) {
		BIG_SUR_RTC_WRITE(year, (0x7f8 + 7));
	}

	month = BIN_TO_BCD(tm.tm_mon);
	if (month != (cmos_month & 0x1f)) {
		BIG_SUR_RTC_WRITE((month & 0x1f) | (cmos_month & ~0x1f), (0x7f8 + 6));	
	}

	day = BIN_TO_BCD(tm.tm_mday);
	if (day != cmos_day) {
		BIG_SUR_RTC_WRITE(day, (0x7f8 + 5));
	}

	if (cmos_hour & 0x40) {
		hour = 0x40;
		if (tm.tm_hour > 12) {
			hour |= 0x20 | (BIN_TO_BCD(hour - 12) & 0x1f);
		}
		else {
			hour |= BIN_TO_BCD(tm.tm_hour);
		}
	}
	else {
		hour = BIN_TO_BCD(tm.tm_hour) & 0x3f;
	}

	if (hour != cmos_hour) {
		BIG_SUR_RTC_WRITE(hour, (0x7f8 + 3));
	}

	minute = BIN_TO_BCD(tm.tm_min);
	if (minute !=  cmos_minute) {
		BIG_SUR_RTC_WRITE(minute, (0x7f8 + 2));
	}

	second = BIN_TO_BCD(tm.tm_sec);
	if (second !=  cmos_second) {
		BIG_SUR_RTC_WRITE(second, (0x7f8 + 1));
	}

	BIG_SUR_RTC_WRITE(cmos_century, 0x7f8);

	return 0;
}

void rtc_init(void)
{
	u8 seconds;

	/* set the function pointers */
	rtc_get_time = rtc_ds_get_time;
	rtc_set_time = rtc_ds_set_time;

	BIG_SUR_RTC_WRITE(0x40, 0x7f8);
	seconds = (u8)(BIG_SUR_RTC_READ(0x7f8 + 1) & 0x7f);
	BIG_SUR_RTC_WRITE(0x80, 0x7f8);
	BIG_SUR_RTC_WRITE(seconds, (0x7f8 + 1));
	BIG_SUR_RTC_WRITE(0, 0x7f8);
}

void big_sur_time_init(void)
{
	mips_counter_frequency = cpu_clock / 2;
	board_timer_setup = big_sur_timer_setup;

	/* 
	 * The RTC device off the Xilinx System Controller
	 * is the DS1742
	 */
	
	rtc_init();
}

void __init pmc_big_sur_setup(void)
{
	board_time_init = big_sur_time_init;

	_machine_restart = big_sur_restart;
	_machine_halt = big_sur_halt;
	_machine_power_off = big_sur_power_off;

	printk("PMC-Sierra Big Sur Board \n");

	/* Add 256 MB. Thats all it can support */
	add_memory_region(0x00000000, 0x10000000, BOOT_MEM_RAM);

	/* Configure the IDE interface, if needed */
#ifdef	CONFIG_BLK_DEV_IDE_BIG_SUR
	/* 
	 * ATA Timing register, sample IORDY. The device operates
	 * in PIO 0 mode and the slave device is disabled
	 */
	*(volatile u_int32_t *)((KSEG1ADDR(0x1B300000)) + 0x40) = 0x2;
#endif
}
