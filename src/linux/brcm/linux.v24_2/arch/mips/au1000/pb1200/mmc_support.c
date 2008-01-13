/*
 * BRIEF MODULE DESCRIPTION
 *
 *	MMC support routines for PB1200.
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

#ifdef CONFIG_MIPS_PB1200
#include <asm/pb1200.h>
#endif

#ifdef CONFIG_MIPS_DB1200
/* NOTE: DB1200 only has SD0 pinned out and usable */
#include <asm/db1200.h>
#endif

/* SD/MMC controller support functions */

/*
 * Detect card.
 */
void mmc_card_inserted(int socket, int *result)
{
	u16 mask;

	if (socket)
#ifdef CONFIG_MIPS_DB1200
		mask = 0;
#else
		mask = BCSR_INT_SD1INSERT; 
#endif
	else
		mask = BCSR_INT_SD0INSERT;

	*result = ((bcsr->sig_status & mask) != 0);
}

/*
 * Check card write protection.
 */
void mmc_card_writable(int socket, int *result)
{
	u16 mask;

	if (socket)
#ifdef CONFIG_MIPS_DB1200
		mask = 0;
#else
		mask = BCSR_STATUS_SD1WP; 
#endif
	else
		mask = BCSR_STATUS_SD0WP;

	/* low means card writable */
	if (!(bcsr->status & mask)) {
		*result = 1;
	} else {
		*result = 0;
	}
}

/*
 * Apply power to card slot.
 */
void mmc_power_on(int socket)
{
	u16 mask;

	if (socket)
#ifdef CONFIG_MIPS_DB1200
		mask = 0;
#else
		mask = BCSR_BOARD_SD1PWR;
#endif
	else
		mask = BCSR_BOARD_SD0PWR;

	bcsr->board |= mask;
	au_sync_delay(1);
}

/*
 * Remove power from card slot.
 */
void mmc_power_off(int socket)
{
	u16 mask;

	if (socket)
#ifdef CONFIG_MIPS_DB1200
		mask = 0;
#else
		mask = BCSR_BOARD_SD1PWR;
#endif
	else
		mask = BCSR_BOARD_SD0PWR;

	bcsr->board &= ~mask;
	au_sync_delay(1);
}

EXPORT_SYMBOL(mmc_card_inserted);
EXPORT_SYMBOL(mmc_card_writable);
EXPORT_SYMBOL(mmc_power_on);
EXPORT_SYMBOL(mmc_power_off);

