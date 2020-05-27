/******************************************************************************
 *
 * Name:	skversion.h
 * Project:	GEnesis, PCI Gigabit Ethernet Adapter
 * Purpose:	specific version strings and numbers
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	(C)Copyright 1998-2002 SysKonnect GmbH.
 *	(C)Copyright 2002-2005 Marvell.
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
 ******************************************************************************/

#define BOOT_STRING  "sk98lin: Network Device Driver v10.93.3.3\n" \
                     "(C)Copyright 1999-2012 Marvell(R)."
#define VER_STRING   "10.93.3.3"
#define PATCHLEVEL   "03"
#define DRIVER_FILE_NAME   "sk98lin"
#define DRIVER_REL_DATE    "Aug-22-2012"
#define DRV_NAME   "sk98lin"
#define DRV_VERSION   "10.93.3.3"
#define FW_VERSION   "N/A"

#ifdef MV_INCLUDE_SDK_SUPPORT
#define FW_FILE_PATHNAME    "/lib/firmware/txbasesu.bin"
#define FW_FILE_NAME        "txbasesu.bin"
#endif

/*******************************************************************************
 *
 * End of file
 *
 ******************************************************************************/
