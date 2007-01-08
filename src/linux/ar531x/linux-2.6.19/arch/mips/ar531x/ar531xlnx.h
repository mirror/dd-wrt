/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2003 Atheros Communications, Inc.,  All Rights Reserved.
 * Copyright (C) 2006 FON Technology, SL.
 * Copyright (C) 2006 Imre Kaloz <kaloz@openwrt.org>
 * Copyright (C) 2006 Felix Fietkau <nbd@openwrt.org>
 */

/*
 * This file contains definitions needed in order to compile
 * AR531X products for linux.  Definitions that are largely
 * AR531X-specific and independent of operating system belong
 * in ar531x.h rather than this file.
 */
#ifndef _AR531XLNX_H
#define _AR531XLNX_H

#include "ar531x.h"

/*
 * Board support data.  The driver is required to locate
 * and fill-in this information before passing a reference to
 * this structure as the HAL_BUS_TAG parameter supplied to
 * ath_hal_attach.
 */
struct ar531x_config {
	const char  *board;	/* board config data */
	const char	*radio;			/* radio config data */
	int		unit;			/* unit number [0, 1] */
	u32		tag;			/* used as devid for now */
};

#define MIPS_CPU_IRQ_BASE		0x00
#define AR531X_HIGH_PRIO                0x10
#define AR531X_MISC_IRQ_BASE		0x20
#define AR531X_GPIO_IRQ_BASE            0x30

/* Software's idea of interrupts handled by "CPU Interrupt Controller" */
#define AR531X_IRQ_NONE		MIPS_CPU_IRQ_BASE+0
#define AR531X_IRQ_MISC_INTRS	MIPS_CPU_IRQ_BASE+2 /* C0_CAUSE: 0x0400 */
#define AR531X_IRQ_WLAN0_INTRS	MIPS_CPU_IRQ_BASE+3 /* C0_CAUSE: 0x0800 */
#define AR531X_IRQ_ENET0_INTRS	MIPS_CPU_IRQ_BASE+4 /* C0_CAUSE: 0x1000 */
#define AR531X_IRQ_LCBUS_PCI	MIPS_CPU_IRQ_BASE+6 /* C0_CAUSE: 0x4000 */
#define AR531X_IRQ_WLAN0_POLL	MIPS_CPU_IRQ_BASE+6 /* C0_CAUSE: 0x4000 */
#define AR531X_IRQ_CPU_CLOCK	MIPS_CPU_IRQ_BASE+7 /* C0_CAUSE: 0x8000 */

/* Miscellaneous interrupts, which share IP6 */
#define AR531X_MISC_IRQ_NONE		AR531X_MISC_IRQ_BASE+0
#define AR531X_MISC_IRQ_TIMER		AR531X_MISC_IRQ_BASE+1
#define AR531X_MISC_IRQ_AHB_PROC	AR531X_MISC_IRQ_BASE+2
#define AR531X_MISC_IRQ_AHB_DMA		AR531X_MISC_IRQ_BASE+3
#define AR531X_MISC_IRQ_GPIO		AR531X_MISC_IRQ_BASE+4
#define AR531X_MISC_IRQ_UART0		AR531X_MISC_IRQ_BASE+5
#define AR531X_MISC_IRQ_UART0_DMA	AR531X_MISC_IRQ_BASE+6
#define AR531X_MISC_IRQ_WATCHDOG	AR531X_MISC_IRQ_BASE+7
#define AR531X_MISC_IRQ_LOCAL		AR531X_MISC_IRQ_BASE+8
#define AR531X_MISC_IRQ_COUNT		9

/* GPIO Interrupts [0..7], share AR531X_MISC_IRQ_GPIO */
#define AR531X_GPIO_IRQ_NONE            AR531X_MISC_IRQ_BASE+0
#define AR531X_GPIO_IRQ(n)              AR531X_MISC_IRQ_BASE+(n)+1
#define AR531X_GPIO_IRQ_COUNT           22

extern struct ar531x_boarddata *ar531x_board_configuration;
extern char *ar531x_radio_configuration;
extern char *enet_mac_address_get(int MACUnit);

#define A_DATA_CACHE_INVAL(start, length) \
        dma_cache_inv((UINT32)(start),(length))

#endif /* _AR531XLNX_H */
