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
 * $Id: wltunable_rte_4322_bmac.h,v 1.2.10.8 2008/05/13 18:41:41 Exp $
 *
 * wl driver tunables for 4322 rte dev
 */

#define D11CONF		0x10000		/* d11 core rev 16 */
#define ACONF		0
#define GCONF		0
#define NCONF		0x10		/* N-PHY 4 only, change to 0x18 for rev3 */
#define LPCONF		0
#define SSLPNCONF       0

#define NTXD		64	/* THIS HAS TO MATCH with HIGH driver tunable */
#define NRXD		32
#define NRXBUFPOST	16
#define WLC_DATAHIWAT	10
#define RXBND		16
#define NRPCTXBUFPOST	48
