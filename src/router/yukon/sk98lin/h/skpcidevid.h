/******************************************************************************
 *
 * Name:        skpcidevid.h_
 * Project:     GEnesis, PCI Gigabit Ethernet Adapter
 * Purpose:     Hold structure with PCI Device IDs
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	(C)Copyright 1998-2002 SysKonnect GmbH.
 *	(C)Copyright 2002-2007 Marvell.
 *
 *	Driver for Marvell Yukon/2 chipset and SysKonnect Gigabit Ethernet
 *      Server Adapters.
 *
 *	    Address all question to: support@marvell.com
 *
 *      LICENSE:
 *      (C)Copyright Marvell.
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      The information in this file is provided "AS IS" without warranty.
 *      /LICENSE
 *
 *****************************************************************************/

static struct pci_device_id sk98lin_pci_tbl[] = {
	/*    1 */
	{ 0x1148, 0x4320, /* Generic SysKonnect SK-98xx V2.0 Gigabit Ethernet Adapter */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*    2 */
	{ 0x1148, 0x9000, /* Generic SysKonnect SK-9Sxx 10/100/1000Base-T Server Adapter */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*    3 */
	{ 0x1148, 0x9E00, /* Generic SysKonnect SK-9Exx 10/100/1000Base-T Adapter */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*    4 */
	{ 0x1148, 0x9E01, /* SysKonnect SK-9E21M 10/100/1000Base-T Adapter for DASH 1.1 */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*    5 */
	{ 0x1186, 0x4001, /* D-Link DGE-550SX PCI-X Gigabit Ethernet Adapter */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*    6 */
	{ 0x1186, 0x4B01, /* D-Link DGE-530T Gigabit Ethernet Adapter (rev.B) */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*    7 */
	{ 0x1186, 0x4B02, /* D-Link DGE-560SX Single Fiber Gigabit Ethernet PCI-E Adapter (rev.A) */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*    8 */
	{ 0x1186, 0x4B03, /* D-Link DGE-550T Gigabit Ethernet Adapter V.B1 */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*    9 */
	{ 0x1186, 0x4C00, /* D-Link DGE-530T Gigabit Ethernet Adapter */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   10 */
	{ 0x11AB, 0x4320, /* Generic Marvell Yukon 88E8001/8003/8010 based Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   11 */
	{ 0x11AB, 0x4340, /* Generic Marvell Yukon 88E8021 based Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   12 */
	{ 0x11AB, 0x4341, /* Generic Marvell Yukon 88E8022 based Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   13 */
	{ 0x11AB, 0x4342, /* Generic Marvell Yukon 88E8061 based Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   14 */
	{ 0x11AB, 0x4343, /* Generic Marvell Yukon 88E8062 based Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   15 */
	{ 0x11AB, 0x4344, /* Generic Marvell Yukon 88E8021 based Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   16 */
	{ 0x11AB, 0x4345, /* Generic Marvell Yukon 88E8022 based Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   17 */
	{ 0x11AB, 0x4346, /* Generic Marvell Yukon 88E8061 based Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   18 */
	{ 0x11AB, 0x4347, /* Generic Marvell Yukon 88E8062 based Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   19 */
	{ 0x11AB, 0x4350, /* Generic Marvell Yukon 88E8035 based Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   20 */
	{ 0x11AB, 0x4352, /* Generic Marvell Yukon 88E8038 based Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   21 */
	{ 0x11AB, 0x4353, /* Generic Marvell Yukon 88E8039 PCI-E Fast Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   22 */
	{ 0x11AB, 0x4354, /* Marvell Yukon 88E8040 Family PCI-E Fast Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   23 */
	{ 0x11AB, 0x4355, /* Generic Marvell Yukon 88E8040T PCI-E Fast Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   24 */
	{ 0x11AB, 0x4356, /* Generic Marvell Yukon 88EC033 based Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   25 */
	{ 0x11AB, 0x4357, /* Generic Marvell Yukon 88E8042 PCI-E Fast Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   26 */
	{ 0x11AB, 0x435A, /* Generic Marvell Yukon 88E8048 PCI-E Fast Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   27 */
	{ 0x11AB, 0x4362, /* Generic Marvell Yukon 88E8053 based Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   28 */
	{ 0x11AB, 0x4363, /* Generic Marvell Yukon 88E8055 PCI-E Gigabit Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   29 */
	{ 0x11AB, 0x4364, /* Generic Marvell Yukon 88E8056 based Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   30 */
	{ 0x11AB, 0x4365, /* Generic Marvell Yukon 88E8070 based Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   31 */
	{ 0x11AB, 0x4366, /* Generic Marvell Yukon 88EC036 PCI-E Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   32 */
	{ 0x11AB, 0x4367, /* Generic Marvell Yukon 88EC032 based Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   33 */
	{ 0x11AB, 0x4368, /* Generic Marvell Yukon 88EC034 based Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   34 */
	{ 0x11AB, 0x4369, /* Generic Marvell Yukon 88EC042 based Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   35 */
	{ 0x11AB, 0x436B, /* Generic Marvell Yukon 88E8071 based Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   36 */
	{ 0x11AB, 0x436C, /* Generic Marvell Yukon 88E8072 based Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   37 */
	{ 0x11AB, 0x436D, /* Generic Marvell Yukon 88E8055 PCI-E Gigabit Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   38 */
	{ 0x11AB, 0x4370, /* Generic Marvell Yukon 88E8075 based Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   39 */
	{ 0x11AB, 0x4380, /* Marvell Yukon 88E8057 Family PCI-E Gigabit Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   40 */
	{ 0x11AB, 0x4381, /* Marvell Yukon 88E8059 Family PCI-E Gigabit Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   41 */
	{ 0x11AB, 0x4382, /* Marvell Yukon 88E8079 Family PCI-E Gigabit Ethernet Controller */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	/*   42 */
	{ 0x11AB, 0x5005, /* Belkin Gigabit Desktop Card10/100/1000Base-T Adapter, Copper RJ-45 */
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },

	{ 0, }

};
