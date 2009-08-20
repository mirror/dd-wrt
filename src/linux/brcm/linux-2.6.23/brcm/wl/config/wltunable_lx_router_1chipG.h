/*
 * Broadcom 802.11abg Networking Device Driver Configuration file
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wltunable_lx_router_1chipG.h,v 1.3 2007/05/11 00:14:19 Exp $
 *
 * wl driver tunables single-chip (4712/535x) G-only driver
 */

#define D11CONF 0x0080	/* Support only the 4712/535x D11 core */
#define GCONF 0x0004	/* Support the 4712/535x G-Phy core */
#define ACONF 0x0000	/* No support for A-Phy operation */
#define NCONF 0x0000	/* No support for N-Phy operation */
#define LPCONF 0x0000	/* No support for LP-Phy operation */

#define WME_PER_AC_TX_PARAMS 1
#define WME_PER_AC_TUNING 1
