/*
 * BRIEF MODULE DESCRIPTION
 *
 *	MMC support routines for Hydrogen3.
 *
 *
 * Copyright (c) 2003-2004 Embedded Edge, LLC.
 * Author: Embedded Edge, LLC.
 * Contact: dan@embeddededge.com
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
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


#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include <asm/irq.h>
#include <asm/au1000.h>
#include <asm/au1100_mmc.h>

#define GPIO_17_WP 0x20000

/* SD/MMC controller support functions */

/*
 * Detect card.
 */
void mmc_card_inserted(int _n_, int *_res_)
{
	u32 gpios = au_readl(SYS_PINSTATERD);
	u32 emptybit = (1<<16);
	*_res_ = ((gpios & emptybit) == 0);
}

/*
 * Check card write protection.
 */
void mmc_card_writable(int _n_, int *_res_)
{
	unsigned long mmc_wp, board_specific; 
		board_specific = au_readl(SYS_OUTPUTSET);
		mmc_wp=GPIO_17_WP;
		if (!(board_specific & mmc_wp)) {/* low means card writable */ 
			*_res_ = 1; 
		} else {
			*_res_ = 0; 
		} 
}
/*
 * Apply power to card slot.
 */
void mmc_power_on(int _n_)
{
}

/*
 * Remove power from card slot.
 */
void mmc_power_off(int _n_)
{
}

EXPORT_SYMBOL(mmc_card_inserted);
EXPORT_SYMBOL(mmc_card_writable);
EXPORT_SYMBOL(mmc_power_on);
EXPORT_SYMBOL(mmc_power_off);

