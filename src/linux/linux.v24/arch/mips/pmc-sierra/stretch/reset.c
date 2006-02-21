/*
 * Copyright PMC-Sierra Inc. 
 * Author : Manish Lachwani (lachwani@pmc-sierra.com)
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */
#include <asm/io.h>
#include <asm/reboot.h>
#include <asm/system.h>
#include <linux/delay.h>
#include "setup.h"

void pmc_stretch_restart(char *command)
{
	/* base address of timekeeper portion of part */
	void *nvram = (void *)PMC_STRETCH_NVRAM_BASE;

 	/* Ask the NVRAM/RTC/watchdog chip to assert reset in 1/16 second */
	writeb(0x84, nvram + 0xff7);

	/* wait for the watchdog to go off */
	mdelay(100+(1000/16));

	/* if the watchdog fails for some reason, let people know */
	printk(KERN_NOTICE "Watchdog reset failed\n");
}

void pmc_stretch_halt(void)
{
	printk(KERN_NOTICE "\n** You can safely turn off the power\n");
	while (1)
		__asm__(".set\tmips3\n\t"
	                "wait\n\t"
			".set\tmips0");
}

void pmc_stretch_power_off(void)
{
	pmc_stretch_halt();
}
