/******************************************************************************
 *
 * Name:	skaddr.c
 * Project:	Gigabit Ethernet Adapters, ADDR-Module
 * Version:	$Revision: #4 $
 * Date:	$Date: 2010/11/04 $
 * Purpose:	Manage Addresses (Multicast and Unicast) and Promiscuous Mode.
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	LICENSE:
 *	(C)Copyright Marvell.
 *	
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *	
 *	The information in this file is provided "AS IS" without warranty.
 *	/LICENSE
 *
 ******************************************************************************/

/******************************************************************************
 *
 * History:
 *
 *	$Log: skaddr.c,v $
 *	Revision 2.16  2009/10/27 08:15:16  tschilli
 *	Added changes to support compilation without RMLT (SK_NO_RLMT)
 *	
 *	Revision 2.15  2008/04/02 10:42:40  tschilli
 *	Fix: Editorial changes.
 *	
 *	Revision 2.14  2008/04/01 09:02:42  tschilli
 *	Fix: Removed unused variable DWord in SkAddrSwap().
 *	
 *	Revision 2.13  2008/03/20 08:24:31  tschilli
 *	Removed support of GENESIS adapters.
 *	
 *	Revision 2.12  2007/02/12 16:40:28  tschilli
 *	Fix: Changed NULL pointer check of pNewAddr in SkAddrOverride() to enable DualNetMode again.
 *
 *	Revision 2.11  2006/08/22 09:10:55  tschilli
 *	Fix: Set pAC->Addr.Port[PortNumber].PromMode to NewPromMode
 *
 *	Revision 2.10  2006/07/13 08:00:41  tschilli
 *	SkAddrOverride(): Added NULL pointer check of pNewAddr.
 *	SkAddrGmacPromiscuousChange() rewritten to fix Windows problem.
 *
 *	Revision 2.9  2005/12/14 16:11:44  ibrueder
 *	added copyright header flags
 *
 *	Revision 2.8  2005/07/21 12:01:30  tschilli
 *	Removed access to RLMT struct for SK_SLIM code.
 *
 *	Revision 2.7  2005/07/21 10:27:01  rschmidt
 *	Added workaround for deviation #4.24 (Yukon-EC).
 *	Editorial changes.
 *
 *	Revision 2.6  2005/05/11 10:05:14  tschilli
 *	SkAddrOverride(): Suppress physical MAC address check of the
 *	other port and logical MAC address check of the same port if
 *	the physical MAC address is to be changed and dual net mode
 *	is active.
 *
 *	Revision 2.5  2005/05/02 13:11:45  tschilli
 *	SkAddrOverride(): Suppress physical MAC address check if the
 *	logical MAC address is to be changed and dual net mode is active.
 *
 *	Revision 2.4  2004/11/12 14:03:54  mkunz
 *	Changed calculation of second Net Mac Addresses
 *
 *	Revision 2.3  2004/08/25 15:39:27  mlindner
 *	Fix: Return value Race condition
 *
 *	Revision 2.2  2003/11/24 12:22:01  mkarl
 *	Added ifdefs for GENENSIS and YUKON and semicolons at the end of
 *	SK_DBG_MSG macros.
 *
 *	Revision 2.1  2003/10/27 14:16:07  amock
 *	Promoted to major revision 2.1
 *
 *	Revision 1.1  2003/10/27 14:14:17  amock
 *	Copied to yukon2 module branch
 *
 *	Revision 1.52  2003/06/02 13:46:15  tschilli
 *	Editorial changes.
 *
 *	Revision 1.51  2003/05/13 17:12:43  mkarl
 *	Changes for SLIM Driver via SK_SLIM.
 *	Changes for driver not using RLMT via SK_NO_RLMT.
 *	Changes for driver not supporting MAC address override via SK_NO_MAO.
 *	Separeted GENESIS and YUKON only code to reduce code size.
 *	Editorial changes.
 *
 *	Revision 1.50  2003/05/08 12:29:31  rschmidt
 *	Replaced all if(GIChipId == CHIP_ID_GENESIS) with new entry GIGenesis.
 *	Changed initialisation for Next0[SK_MAX_MACS] to avoid
 *	compiler errors when SK_MAX_MACS=1.
 *	Editorial changes.
 *
 *	Revision 1.49  2003/04/15 09:30:51  tschilli
 *	Copyright messages changed.
 *	"#error C++ is not yet supported." removed.
 *
 *	Revision 1.48  2003/02/12 17:09:37  tschilli
 *	Fix in SkAddrOverride() to set both (physical and logical) MAC addresses
 *	in case that both addresses are identical.
 *
 *	Revision 1.47  2002/09/17 06:31:10  tschilli
 *	Handling of SK_PROM_MODE_ALL_MC flag in SkAddrGmacMcUpdate()
 *	and SkAddrGmacPromiscuousChange() fixed.
 *	Editorial changes.
 *
 *	Revision 1.46  2002/08/22 07:55:41  tschilli
 *	New function SkGmacMcHash() for GMAC multicast hashing algorithm added.
 *	Editorial changes.
 *
 *	Revision 1.45  2002/08/15 12:29:35  tschilli
 *	SkAddrGmacMcUpdate() and SkAddrGmacPromiscuousChange() changed.
 *
 *	Revision 1.44  2002/08/14 12:18:03  rschmidt
 *	Replaced direct handling of MAC Hashing (XMAC and GMAC)
 *	with routine SkMacHashing().
 *	Replaced wrong 3rd para 'i' with 'PortNumber' in SkMacPromiscMode().
 *
 *	Revision 1.43  2002/08/13 09:37:43  rschmidt
 *	Corrected some SK_DBG_MSG outputs.
 *	Replaced wrong 2nd para pAC with IoC in SkMacPromiscMode().
 *	Editorial changes.
 *
 *	Revision 1.42  2002/08/12 11:24:36  rschmidt
 *	Remove setting of logical MAC address GM_SRC_ADDR_2 in SkAddrInit().
 *	Replaced direct handling of MAC Promiscuous Mode (XMAC and GMAC)
 *	with routine SkMacPromiscMode().
 *	Editorial changes.
 *
 *	Revision 1.41  2002/06/10 13:52:18  tschilli
 *	Changes for handling YUKON.
 *	All changes are internally and not visible to the programmer
 *	using this module.
 *
 *	Revision 1.40  2001/02/14 14:04:59  rassmann
 *	Editorial changes.
 *
 *	Revision 1.39  2001/01/30 10:30:04  rassmann
 *	Editorial changes.
 *
 *	Revision 1.38  2001/01/25 16:26:52  rassmann
 *	Ensured that logical address overrides are done on net's active port.
 *
 *	Revision 1.37  2001/01/22 13:41:34  rassmann
 *	Supporting two nets on dual-port adapters.
 *
 *	Revision 1.36  2000/08/07 11:10:39  rassmann
 *	Editorial changes.
 *
 *	Revision 1.35  2000/05/04 09:38:41  rassmann
 *	Editorial changes.
 *	Corrected multicast address hashing.
 *
 *	Revision 1.34  1999/11/22 13:23:44  cgoos
 *	Changed license header to GPL.
 *
 *	Revision 1.33  1999/05/28 10:56:06  rassmann
 *	Editorial changes.
 *
 *	Revision 1.32  1999/03/31 10:59:20  rassmann
 *	Returning Success instead of DupAddr if address shall be overridden
 *	with same value.
 *
 *	Revision 1.31  1999/01/14 16:18:17  rassmann
 *	Corrected multicast initialization.
 *
 *	Revision 1.30  1999/01/04 10:30:35  rassmann
 *	SkAddrOverride only possible after SK_INIT_IO phase.
 *
 *	Revision 1.29  1998/12/29 13:13:10  rassmann
 *	An address override is now preserved in the SK_INIT_IO phase.
 *	All functions return an int now.
 *	Extended parameter checking.
 *
 *	Revision 1.28  1998/12/01 11:45:53  rassmann
 *	Code cleanup.
 *
 *	Revision 1.27  1998/12/01 09:22:49  rassmann
 *	SkAddrMcAdd and SkAddrMcUpdate returned SK_MC_FILTERING_INEXACT
 *	too often.
 *
 *	Revision 1.26  1998/11/24 12:39:44  rassmann
 *	Reserved multicast entry for BPDU address.
 *	13 multicast entries left for protocol.
 *
 *	Revision 1.25  1998/11/17 16:54:23  rassmann
 *	Using exact match for up to 14 multicast addresses.
 *	Still receiving all multicasts if more addresses are added.
 *
 *	Revision 1.24  1998/11/13 17:24:31  rassmann
 *	Changed return value of SkAddrOverride to int.
 *
 *	Revision 1.23  1998/11/13 16:56:18  rassmann
 *	Added macro SK_ADDR_COMPARE.
 *	Changed return type of SkAddrOverride to SK_BOOL.
 *
 *	Revision 1.22  1998/11/04 17:06:17  rassmann
 *	Corrected McUpdate and PromiscuousChange functions.
 *
 *	Revision 1.21  1998/10/29 14:34:04  rassmann
 *	Clearing SK_ADDR struct at startup.
 *
 *	Revision 1.20  1998/10/28 18:16:34  rassmann
 *	Avoiding I/Os before SK_INIT_RUN level.
 *	Aligning InexactFilter.
 *
 *	Revision 1.19  1998/10/28 11:29:28  rassmann
 *	Programming physical address in SkAddrMcUpdate.
 *	Corrected programming of exact match entries.
 *
 *	Revision 1.18  1998/10/28 10:34:48  rassmann
 *	Corrected reading of physical addresses.
 *
 *	Revision 1.17  1998/10/28 10:26:13  rassmann
 *	Getting ports' current MAC addresses from EPROM now.
 *	Added debug output.
 *
 *	Revision 1.16  1998/10/27 16:20:12  rassmann
 *	Reading MAC address byte by byte.
 *
 *	Revision 1.15  1998/10/22 11:39:09  rassmann
 *	Corrected signed/unsigned mismatches.
 *
 *	Revision 1.14  1998/10/19 17:12:35  rassmann
 *	Syntax corrections.
 *
 *	Revision 1.13  1998/10/19 17:02:19  rassmann
 *	Now reading permanent MAC addresses from CRF.
 *
 *	Revision 1.12  1998/10/15 15:15:48  rassmann
 *	Changed Flags Parameters from SK_U8 to int.
 *	Checked with lint.
 *
 *	Revision 1.11  1998/09/24 19:15:12  rassmann
 *	Code cleanup.
 *
 *	Revision 1.10  1998/09/18 20:18:54  rassmann
 *	Added HW access.
 *	Implemented swapping.
 *
 *	Revision 1.9  1998/09/16 11:32:00  rassmann
 *	Including skdrv1st.h again. :(
 *
 *	Revision 1.8  1998/09/16 11:09:34  rassmann
 *	Syntax corrections.
 *
 *	Revision 1.7  1998/09/14 17:06:34  rassmann
 *	Minor changes.
 *
 *	Revision 1.6  1998/09/07 08:45:41  rassmann
 *	Syntax corrections.
 *
 *	Revision 1.5  1998/09/04 19:40:19  rassmann
 *	Interface enhancements.
 *
 *	Revision 1.4  1998/09/04 12:14:12  rassmann
 *	Interface cleanup.
 *
 *	Revision 1.3  1998/09/02 16:56:40  rassmann
 *	Updated interface.
 *
 *	Revision 1.2  1998/08/27 14:26:09  rassmann
 *	Updated interface.
 *
 *	Revision 1.1  1998/08/21 08:30:22  rassmann
 *	First public version.
 *
 ******************************************************************************/

/******************************************************************************
 *
 * Description:
 *
 * This module is intended to manage multicast addresses, address override,
 * and promiscuous mode on Yukon adapters.
 *
 * Address Layout:
 *	port address:		physical MAC address
 *
 * Include File Hierarchy:
 *
 *	"skdrv1st.h"
 *	"skdrv2nd.h"
 *
 ******************************************************************************/

#if (defined(DEBUG) || ((!defined(LINT)) && (!defined(SK_SLIM))))
static const char SysKonnectFileId[] =
	"@(#) $Id: //Release/Yukon_1G/Shared/addr/V5/skaddr.c#4 $ (C) Marvell.";
#endif /* DEBUG ||!LINT || !SK_SLIM */

#define __SKADDR_C

#ifdef __cplusplus
extern "C" {
#endif	/* cplusplus */

#include "h/skdrv1st.h"
#include "h/skdrv2nd.h"

/* defines ********************************************************************/

#define XMAC_POLY	0xEDB88320UL	/* CRC32-Poly - XMAC: Little Endian */
#define GMAC_POLY	0x04C11DB7L	/* CRC16-Poly - GMAC: Little Endian */
#define SKHASH_BITS	6				/* #bits in hash */
#define SK_MC_BIT	0x01

/* Error numbers and messages. */

#define SKERR_ADDR_E001		(SK_ERRBASE_ADDR + 0)
#define SKERR_ADDR_E001MSG	"Bad Flags."
#define SKERR_ADDR_E002		(SKERR_ADDR_E001 + 1)
#define SKERR_ADDR_E002MSG	"New Error."

/* typedefs *******************************************************************/

/* None. */

/* global variables ***********************************************************/

/* 64-bit hash values with all bits set. */

SK_U16	OnesHash[4] = {0xffff, 0xffff, 0xffff, 0xffff};

/* local variables ************************************************************/

#ifdef DEBUG
static int	Next0[SK_MAX_MACS] = {0};
#endif	/* DEBUG */

/* functions ******************************************************************/

/******************************************************************************
 *
 *	SkAddrInit - initialize data, set state to init
 *
 * Description:
 *
 *	SK_INIT_DATA
 *	============
 *
 *	This routine clears the multicast tables and resets promiscuous mode.
 *	Some entries are reserved for the "logical MAC address", the
 *	SK-RLMT multicast address, and the BPDU multicast address.
 *
 *
 *	SK_INIT_IO
 *	==========
 *
 *	All permanent MAC addresses are read from EPROM.
 *	If the current MAC addresses are not already set in software,
 *	they are set to the values of the permanent addresses.
 *	The current addresses are written to the corresponding MAC.
 *
 *
 *	SK_INIT_RUN
 *	===========
 *
 *	Nothing.
 *
 * Context:
 *	init, pageable
 *
 * Returns:
 *	SK_ADDR_SUCCESS
 */
int	SkAddrInit(
SK_AC	*pAC,	/* the adapter context */
SK_IOC	IoC,	/* I/O context */
int		Level)	/* initialization level */
{
	int			j;
	SK_U32		i;
	SK_U8		*InAddr;
	SK_U16		*OutAddr;
	SK_ADDR_PORT	*pAPort;

	switch (Level) {
	case SK_INIT_DATA:
		SK_MEMSET((char *)&pAC->Addr, (SK_U8)0, (SK_U16)sizeof(SK_ADDR));

		for (i = 0; i < SK_MAX_MACS; i++) {
			pAPort = &pAC->Addr.Port[i];
			pAPort->PromMode = SK_PROM_MODE_NONE;

			pAPort->FirstExactMatchRlmt = SK_ADDR_FIRST_MATCH_RLMT;
			pAPort->FirstExactMatchDrv = SK_ADDR_FIRST_MATCH_DRV;
			pAPort->NextExactMatchRlmt = SK_ADDR_FIRST_MATCH_RLMT;
			pAPort->NextExactMatchDrv = SK_ADDR_FIRST_MATCH_DRV;
		}
#ifdef xDEBUG
		for (i = 0; i < SK_MAX_MACS; i++) {
			if (pAC->Addr.Port[i].NextExactMatchRlmt <
				SK_ADDR_FIRST_MATCH_RLMT) {
				Next0[i] |= 4;
			}
		}
#endif	/* DEBUG */
		break;

	case SK_INIT_IO:
#ifndef SK_NO_RLMT
		for (i = 0; i < SK_MAX_NETS; i++) {
			pAC->Addr.Net[i].ActivePort = pAC->Rlmt.Net[i].ActivePort;
		}
#else
		for (i = 0; i < SK_MAX_NETS; i++) {
			pAC->Addr.Net[i].ActivePort = i;
		}
#endif /* SK_NO_RLMT */
#ifdef xDEBUG
		for (i = 0; i < SK_MAX_MACS; i++) {
			if (pAC->Addr.Port[i].NextExactMatchRlmt <
				SK_ADDR_FIRST_MATCH_RLMT) {
				Next0[i] |= 8;
			}
		}
#endif	/* DEBUG */

		/* Read permanent logical MAC address from Control Register File. */
		for (j = 0; j < SK_MAC_ADDR_LEN; j++) {
			InAddr = (SK_U8 *) &pAC->Addr.Net[0].PermanentMacAddress.a[j];
			SK_IN8(IoC, B2_MAC_1 + j, InAddr);
		}

		if (!pAC->Addr.Net[0].CurrentMacAddressSet) {
			/* Set the current logical MAC address to the permanent one. */
			pAC->Addr.Net[0].CurrentMacAddress =
				pAC->Addr.Net[0].PermanentMacAddress;
			pAC->Addr.Net[0].CurrentMacAddressSet = SK_TRUE;
		}

		/* Set the current logical MAC address. */
		pAC->Addr.Port[pAC->Addr.Net[0].ActivePort].Exact[0] =
			pAC->Addr.Net[0].CurrentMacAddress;
#if SK_MAX_NETS > 1
		/* Set logical MAC address for net 2, too. */
		if (!pAC->Addr.Net[1].CurrentMacAddressSet) {
			pAC->Addr.Net[1].PermanentMacAddress =
				pAC->Addr.Net[0].PermanentMacAddress;
			pAC->Addr.Net[1].PermanentMacAddress.a[5] += 1;

			/* Set the current logical MAC address to the permanent one. */
			pAC->Addr.Net[1].CurrentMacAddress =
				pAC->Addr.Net[1].PermanentMacAddress;
			pAC->Addr.Net[1].CurrentMacAddressSet = SK_TRUE;
#ifdef SK_NO_RLMT
			/* Set the current logical MAC address. */
			pAC->Addr.Port[pAC->Addr.Net[1].ActivePort].Exact[0] =
				pAC->Addr.Net[1].CurrentMacAddress;
#endif
		}
#endif	/* SK_MAX_NETS > 1 */

#ifdef DEBUG
		for (i = 0; i < (SK_U32) pAC->GIni.GIMacsFound; i++) {
			SK_DBG_MSG(pAC, SK_DBGMOD_ADDR, SK_DBGCAT_INIT,
				("Permanent MAC Address (Net%d): %02X %02X %02X %02X %02X %02X\n",
					i,
					pAC->Addr.Net[i].PermanentMacAddress.a[0],
					pAC->Addr.Net[i].PermanentMacAddress.a[1],
					pAC->Addr.Net[i].PermanentMacAddress.a[2],
					pAC->Addr.Net[i].PermanentMacAddress.a[3],
					pAC->Addr.Net[i].PermanentMacAddress.a[4],
					pAC->Addr.Net[i].PermanentMacAddress.a[5]));

			SK_DBG_MSG(pAC, SK_DBGMOD_ADDR, SK_DBGCAT_INIT,
				("Logical MAC Address (Net%d): %02X %02X %02X %02X %02X %02X\n",
					i,
					pAC->Addr.Net[i].CurrentMacAddress.a[0],
					pAC->Addr.Net[i].CurrentMacAddress.a[1],
					pAC->Addr.Net[i].CurrentMacAddress.a[2],
					pAC->Addr.Net[i].CurrentMacAddress.a[3],
					pAC->Addr.Net[i].CurrentMacAddress.a[4],
					pAC->Addr.Net[i].CurrentMacAddress.a[5]));
		}
#endif	/* DEBUG */

		for (i = 0; i < (SK_U32) pAC->GIni.GIMacsFound; i++) {
			pAPort = &pAC->Addr.Port[i];

			/* Read permanent port addresses from Control Register File. */
			for (j = 0; j < SK_MAC_ADDR_LEN; j++) {
				InAddr = (SK_U8 *) &pAPort->PermanentMacAddress.a[j];
				SK_IN8(IoC, B2_MAC_2 + 8 * i + j, InAddr);
			}

			if (!pAPort->CurrentMacAddressSet) {
				/*
				 * Set the current and previous physical MAC address
				 * of this port to its permanent MAC address.
				 */
				pAPort->CurrentMacAddress = pAPort->PermanentMacAddress;
				pAPort->PreviousMacAddress = pAPort->PermanentMacAddress;
				pAPort->CurrentMacAddressSet = SK_TRUE;
			}

			/* Set port's current physical MAC address. */
			OutAddr = (SK_U16 *) &pAPort->CurrentMacAddress.a[0];
			GM_OUTADDR(IoC, i, GM_SRC_ADDR_1L, OutAddr);

#ifdef DEBUG
			SK_DBG_MSG(pAC, SK_DBGMOD_ADDR, SK_DBGCAT_INIT,
				("SkAddrInit: Permanent Physical MAC Address[%d]: %02X %02X %02X %02X %02X %02X\n",
					i,
					pAPort->PermanentMacAddress.a[0],
					pAPort->PermanentMacAddress.a[1],
					pAPort->PermanentMacAddress.a[2],
					pAPort->PermanentMacAddress.a[3],
					pAPort->PermanentMacAddress.a[4],
					pAPort->PermanentMacAddress.a[5]));

			SK_DBG_MSG(pAC, SK_DBGMOD_ADDR, SK_DBGCAT_INIT,
				("SkAddrInit: Physical MAC Address[%d]: %02X %02X %02X %02X %02X %02X\n",
					i,
					pAPort->CurrentMacAddress.a[0],
					pAPort->CurrentMacAddress.a[1],
					pAPort->CurrentMacAddress.a[2],
					pAPort->CurrentMacAddress.a[3],
					pAPort->CurrentMacAddress.a[4],
					pAPort->CurrentMacAddress.a[5]));
#endif /* DEBUG */
		}
		break;

	case SK_INIT_RUN:
#ifdef xDEBUG
		for (i = 0; i < SK_MAX_MACS; i++) {
			if (pAC->Addr.Port[i].NextExactMatchRlmt <
				SK_ADDR_FIRST_MATCH_RLMT) {
				Next0[i] |= 16;
			}
		}
#endif	/* DEBUG */
		break;

	default:	/* error */
		break;
	}

	return (SK_ADDR_SUCCESS);

}	/* SkAddrInit */

#ifndef SK_SLIM

/******************************************************************************
 *
 *	SkAddrMcClear - clear the multicast table
 *
 * Description:
 *	This routine clears the multicast table.
 *
 *	If not suppressed by Flag SK_MC_SW_ONLY, the hardware is updated
 *	immediately.
 *
 *	It calls either SkAddrXmacMcClear or SkAddrGmacMcClear, according
 *	to the adapter in use. The real work is done there.
 *
 * Context:
 *	runtime, pageable
 *	may be called starting with SK_INIT_DATA with flag SK_MC_SW_ONLY
 *	may be called after SK_INIT_IO without limitation
 *
 * Returns:
 *	SK_ADDR_SUCCESS
 *	SK_ADDR_ILLEGAL_PORT
 */
int	SkAddrMcClear(
SK_AC	*pAC,		/* adapter context */
SK_IOC	IoC,		/* I/O context */
SK_U32	PortNumber,	/* Index of affected port */
int		Flags)		/* permanent/non-perm, sw-only */
{
	int ReturnCode;

	if (PortNumber >= (SK_U32) pAC->GIni.GIMacsFound) {
		return (SK_ADDR_ILLEGAL_PORT);
	}
	ReturnCode = SkAddrGmacMcClear(pAC, IoC, PortNumber, Flags);
	return (ReturnCode);

}	/* SkAddrMcClear */


/******************************************************************************
 *
 *	SkAddrGmacMcClear - clear the multicast table
 *
 * Description:
 *	This routine clears the multicast hashing table (InexactFilter)
 *	(either the RLMT or the driver bits) of the given port.
 *
 *	If not suppressed by Flag SK_MC_SW_ONLY, the hardware is updated
 *	immediately.
 *
 * Context:
 *	runtime, pageable
 *	may be called starting with SK_INIT_DATA with flag SK_MC_SW_ONLY
 *	may be called after SK_INIT_IO without limitation
 *
 * Returns:
 *	SK_ADDR_SUCCESS
 *	SK_ADDR_ILLEGAL_PORT
 */
int	SkAddrGmacMcClear(
SK_AC	*pAC,		/* adapter context */
SK_IOC	IoC,		/* I/O context */
SK_U32	PortNumber,	/* Index of affected port */
int		Flags)		/* permanent/non-perm, sw-only */
{
	int i;

#ifdef DEBUG
	SK_DBG_MSG(pAC, SK_DBGMOD_ADDR, SK_DBGCAT_CTRL,
		("GMAC InexactFilter (not cleared): %02X %02X %02X %02X %02X %02X %02X %02X\n",
			pAC->Addr.Port[PortNumber].InexactFilter.Bytes[0],
			pAC->Addr.Port[PortNumber].InexactFilter.Bytes[1],
			pAC->Addr.Port[PortNumber].InexactFilter.Bytes[2],
			pAC->Addr.Port[PortNumber].InexactFilter.Bytes[3],
			pAC->Addr.Port[PortNumber].InexactFilter.Bytes[4],
			pAC->Addr.Port[PortNumber].InexactFilter.Bytes[5],
			pAC->Addr.Port[PortNumber].InexactFilter.Bytes[6],
			pAC->Addr.Port[PortNumber].InexactFilter.Bytes[7]));
#endif	/* DEBUG */

	/* Clear InexactFilter */
	for (i = 0; i < 8; i++) {
		pAC->Addr.Port[PortNumber].InexactFilter.Bytes[i] = 0;
	}

	if (Flags & SK_ADDR_PERMANENT) {	/* permanent => RLMT */

		/* Copy DRV bits to InexactFilter. */
		for (i = 0; i < 8; i++) {
			pAC->Addr.Port[PortNumber].InexactFilter.Bytes[i] |=
				pAC->Addr.Port[PortNumber].InexactDrvFilter.Bytes[i];

			/* Clear InexactRlmtFilter. */
			pAC->Addr.Port[PortNumber].InexactRlmtFilter.Bytes[i] = 0;
		}
	}
	else {	/* not permanent => DRV */

		/* Copy RLMT bits to InexactFilter. */
		for (i = 0; i < 8; i++) {
			pAC->Addr.Port[PortNumber].InexactFilter.Bytes[i] |=
				pAC->Addr.Port[PortNumber].InexactRlmtFilter.Bytes[i];

			/* Clear InexactDrvFilter. */
			pAC->Addr.Port[PortNumber].InexactDrvFilter.Bytes[i] = 0;
		}
	}

#ifdef DEBUG
	SK_DBG_MSG(pAC, SK_DBGMOD_ADDR, SK_DBGCAT_CTRL,
		("GMAC InexactFilter (cleared): %02X %02X %02X %02X %02X %02X %02X %02X\n",
			pAC->Addr.Port[PortNumber].InexactFilter.Bytes[0],
			pAC->Addr.Port[PortNumber].InexactFilter.Bytes[1],
			pAC->Addr.Port[PortNumber].InexactFilter.Bytes[2],
			pAC->Addr.Port[PortNumber].InexactFilter.Bytes[3],
			pAC->Addr.Port[PortNumber].InexactFilter.Bytes[4],
			pAC->Addr.Port[PortNumber].InexactFilter.Bytes[5],
			pAC->Addr.Port[PortNumber].InexactFilter.Bytes[6],
			pAC->Addr.Port[PortNumber].InexactFilter.Bytes[7]));
#endif	/* DEBUG */

	if (!(Flags & SK_MC_SW_ONLY)) {
		(void) SkAddrGmacMcUpdate(pAC, IoC, PortNumber);
	}

	return (SK_ADDR_SUCCESS);

}	/* SkAddrGmacMcClear */

#ifndef SK_ADDR_CHEAT

/******************************************************************************
 *
 *	SkGmacMcHash - hash multicast address
 *
 * Description:
 *	This routine computes the hash value for a multicast address.
 *	A CRC16 algorithm is used.
 *
 * Notes:
 *
 *
 * Context:
 *	runtime, pageable
 *
 * Returns:
 *	Hash value of multicast address.
 */
SK_U32 SkGmacMcHash(
unsigned char *pMc)	/* Multicast address */
{
	SK_U32 Data;
	SK_U32 TmpData;
	SK_U32 Crc;
	int Byte;
	int Bit;

	Crc = 0xFFFFFFFFUL;
	for (Byte = 0; Byte < 6; Byte++) {
		/* Get next byte. */
		Data = (SK_U32) pMc[Byte];

		/* Change bit order in byte. */
		TmpData = Data;
		for (Bit = 0; Bit < 8; Bit++) {
			if (TmpData & 1L) {
				Data |=  1L << (7 - Bit);
			}
			else {
				Data &= ~(1L << (7 - Bit));
			}
			TmpData >>= 1;
		}

		Crc ^= (Data << 24);
		for (Bit = 0; Bit < 8; Bit++) {
			if (Crc & 0x80000000) {
				Crc = (Crc << 1) ^ GMAC_POLY;
			}
			else {
				Crc <<= 1;
			}
		}
	}

	return (Crc & ((1 << SKHASH_BITS) - 1));

}	/* SkGmacMcHash */

#endif	/* !SK_ADDR_CHEAT */

/******************************************************************************
 *
 *	SkAddrMcAdd - add a multicast address to a port
 *
 * Description:
 *	This routine enables reception for a given address on the given port.
 *
 *	It calls either SkAddrXmacMcAdd or SkAddrGmacMcAdd, according to the
 *	adapter in use. The real work is done there.
 *
 * Notes:
 *	The return code is only valid for SK_PROM_MODE_NONE.
 *
 * Context:
 *	runtime, pageable
 *	may be called after SK_INIT_DATA
 *
 * Returns:
 *	SK_MC_FILTERING_EXACT
 *	SK_MC_FILTERING_INEXACT
 *	SK_MC_ILLEGAL_ADDRESS
 *	SK_MC_ILLEGAL_PORT
 *	SK_MC_RLMT_OVERFLOW
 */
int	SkAddrMcAdd(
SK_AC		*pAC,		/* adapter context */
SK_IOC		IoC,		/* I/O context */
SK_U32		PortNumber,	/* Port Number */
SK_MAC_ADDR	*pMc,		/* multicast address to be added */
int			Flags)		/* permanent/non-permanent */
{
	int ReturnCode;

	if (PortNumber >= (SK_U32) pAC->GIni.GIMacsFound) {
		return (SK_ADDR_ILLEGAL_PORT);
	}
	ReturnCode = SkAddrGmacMcAdd(pAC, IoC, PortNumber, pMc, Flags);
	return (ReturnCode);

}	/* SkAddrMcAdd */


/******************************************************************************
 *
 *	SkAddrGmacMcAdd - add a multicast address to a port
 *
 * Description:
 *	This routine enables reception for a given address on the given port.
 *
 * Notes:
 *	The return code is only valid for SK_PROM_MODE_NONE.
 *
 * Context:
 *	runtime, pageable
 *	may be called after SK_INIT_DATA
 *
 * Returns:
 *	SK_MC_FILTERING_INEXACT
 *	SK_MC_ILLEGAL_ADDRESS
 */
int	SkAddrGmacMcAdd(
SK_AC		*pAC,		/* adapter context */
SK_IOC		IoC,		/* I/O context */
SK_U32		PortNumber,	/* Port Number */
SK_MAC_ADDR	*pMc,		/* multicast address to be added */
int		Flags)		/* permanent/non-permanent */
{
	int	i;
#ifndef SK_ADDR_CHEAT
	SK_U32 HashBit;
#endif	/* !defined(SK_ADDR_CHEAT) */

	if (!(pMc->a[0] & SK_MC_BIT)) {
		/* Hashing only possible with multicast addresses */
		return (SK_MC_ILLEGAL_ADDRESS);
	}

#ifndef SK_ADDR_CHEAT

	/* Compute hash value of address. */
	HashBit = SkGmacMcHash(&pMc->a[0]);

	if (Flags & SK_ADDR_PERMANENT) {	/* permanent => RLMT */

		/* Add bit to InexactRlmtFilter. */
		pAC->Addr.Port[PortNumber].InexactRlmtFilter.Bytes[HashBit / 8] |=
			1 << (HashBit % 8);

		/* Copy bit to InexactFilter. */
		for (i = 0; i < 8; i++) {
			pAC->Addr.Port[PortNumber].InexactFilter.Bytes[i] |=
				pAC->Addr.Port[PortNumber].InexactRlmtFilter.Bytes[i];
		}

#ifdef DEBUG
		SK_DBG_MSG(pAC, SK_DBGMOD_ADDR, SK_DBGCAT_CTRL,
		("GMAC InexactRlmtFilter: %02X %02X %02X %02X %02X %02X %02X %02X\n",
			pAC->Addr.Port[PortNumber].InexactRlmtFilter.Bytes[0],
			pAC->Addr.Port[PortNumber].InexactRlmtFilter.Bytes[1],
			pAC->Addr.Port[PortNumber].InexactRlmtFilter.Bytes[2],
			pAC->Addr.Port[PortNumber].InexactRlmtFilter.Bytes[3],
			pAC->Addr.Port[PortNumber].InexactRlmtFilter.Bytes[4],
			pAC->Addr.Port[PortNumber].InexactRlmtFilter.Bytes[5],
			pAC->Addr.Port[PortNumber].InexactRlmtFilter.Bytes[6],
			pAC->Addr.Port[PortNumber].InexactRlmtFilter.Bytes[7]));
#endif	/* DEBUG */
	}
	else {	/* not permanent => DRV */

		/* Add bit to InexactDrvFilter. */
		pAC->Addr.Port[PortNumber].InexactDrvFilter.Bytes[HashBit / 8] |=
			1 << (HashBit % 8);

		/* Copy bit to InexactFilter. */
		for (i = 0; i < 8; i++) {
			pAC->Addr.Port[PortNumber].InexactFilter.Bytes[i] |=
				pAC->Addr.Port[PortNumber].InexactDrvFilter.Bytes[i];
		}

#ifdef DEBUG
		SK_DBG_MSG(pAC, SK_DBGMOD_ADDR, SK_DBGCAT_CTRL,
		("GMAC InexactDrvFilter: %02X %02X %02X %02X %02X %02X %02X %02X\n",
			pAC->Addr.Port[PortNumber].InexactDrvFilter.Bytes[0],
			pAC->Addr.Port[PortNumber].InexactDrvFilter.Bytes[1],
			pAC->Addr.Port[PortNumber].InexactDrvFilter.Bytes[2],
			pAC->Addr.Port[PortNumber].InexactDrvFilter.Bytes[3],
			pAC->Addr.Port[PortNumber].InexactDrvFilter.Bytes[4],
			pAC->Addr.Port[PortNumber].InexactDrvFilter.Bytes[5],
			pAC->Addr.Port[PortNumber].InexactDrvFilter.Bytes[6],
			pAC->Addr.Port[PortNumber].InexactDrvFilter.Bytes[7]));
#endif	/* DEBUG */
	}

#else	/* SK_ADDR_CHEAT */

	/* Set all bits in InexactFilter. */
	for (i = 0; i < 8; i++) {
		pAC->Addr.Port[PortNumber].InexactFilter.Bytes[i] = 0xFF;
	}
#endif	/* SK_ADDR_CHEAT */

	return (SK_MC_FILTERING_INEXACT);

}	/* SkAddrGmacMcAdd */

#endif /* !SK_SLIM */

/******************************************************************************
 *
 *	SkAddrMcUpdate - update the HW MC address table and set the MAC address
 *
 * Description:
 *	This routine enables reception of the addresses contained in a local
 *	table for a given port.
 *	It also programs the port's current physical MAC address.
 *
 *	It calls either SkAddrXmacMcUpdate or SkAddrGmacMcUpdate, according
 *	to the adapter in use. The real work is done there.
 *
 * Notes:
 *	The return code is only valid for SK_PROM_MODE_NONE.
 *
 * Context:
 *	runtime, pageable
 *	may be called after SK_INIT_IO
 *
 * Returns:
 *	SK_MC_FILTERING_EXACT
 *	SK_MC_FILTERING_INEXACT
 *	SK_ADDR_ILLEGAL_PORT
 */
int	SkAddrMcUpdate(
SK_AC	*pAC,		/* adapter context */
SK_IOC	IoC,		/* I/O context */
SK_U32	PortNumber)	/* Port Number */
{
	int ReturnCode = SK_ADDR_ILLEGAL_PORT;

#if (!defined(SK_SLIM) || defined(DEBUG))
	if (PortNumber >= (SK_U32) pAC->GIni.GIMacsFound) {
		return (SK_ADDR_ILLEGAL_PORT);
	}
#endif /* !SK_SLIM || DEBUG */
	ReturnCode = SkAddrGmacMcUpdate(pAC, IoC, PortNumber);
	return (ReturnCode);

}	/* SkAddrMcUpdate */


/******************************************************************************
 *
 *	SkAddrGmacMcUpdate - update the HW MC address table and set the MAC address
 *
 * Description:
 *	This routine enables reception of the addresses contained in a local
 *	table for a given port.
 *	It also programs the port's current physical MAC address.
 *
 * Notes:
 *	The return code is only valid for SK_PROM_MODE_NONE.
 *
 * Context:
 *	runtime, pageable
 *	may be called after SK_INIT_IO
 *
 * Returns:
 *	SK_MC_FILTERING_EXACT
 *	SK_MC_FILTERING_INEXACT
 *	SK_ADDR_ILLEGAL_PORT
 */
int	SkAddrGmacMcUpdate(
SK_AC	*pAC,		/* adapter context */
SK_IOC	IoC,		/* I/O context */
SK_U32	PortNumber)	/* Port Number */
{
#ifndef SK_SLIM
	SK_U32		i;
	SK_U8		Inexact;
#endif	/* not SK_SLIM */
	SK_U16		*OutAddr;
	SK_ADDR_PORT	*pAPort;

	SK_DBG_MSG(pAC,SK_DBGMOD_ADDR, SK_DBGCAT_CTRL,
		("SkAddrGmacMcUpdate on Port %u.\n", PortNumber));

	pAPort = &pAC->Addr.Port[PortNumber];

#ifdef DEBUG
	SK_DBG_MSG(pAC,SK_DBGMOD_ADDR, SK_DBGCAT_CTRL,
		("Next0 on Port %d: %d\n", PortNumber, Next0[PortNumber]));
#endif /* DEBUG */

#ifndef SK_SLIM
	for (Inexact = 0, i = 0; i < 8; i++) {
		Inexact |= pAPort->InexactFilter.Bytes[i];
	}

	/* Set 64-bit hash register to InexactFilter. */
	GM_OUTHASH(IoC, PortNumber, GM_MC_ADDR_H1,
		&pAPort->InexactFilter.Bytes[0]);

	if (pAPort->PromMode & SK_PROM_MODE_ALL_MC) {

		/* Set all bits in 64-bit hash register. */
		GM_OUTHASH(IoC, PortNumber, GM_MC_ADDR_H1, &OnesHash);

		/* Enable Hashing */
		SkMacHashing(pAC, IoC, (int) PortNumber, SK_TRUE);
	}
	else {
		/* Enable Hashing. */
		SkMacHashing(pAC, IoC, (int) PortNumber, SK_TRUE);
	}

	if (pAPort->PromMode != SK_PROM_MODE_NONE) {
		(void) SkAddrGmacPromiscuousChange(pAC, IoC, PortNumber, pAPort->PromMode);
	}
#else /* SK_SLIM */

	/* Set all bits in 64-bit hash register. */
	GM_OUTHASH(IoC, PortNumber, GM_MC_ADDR_H1, &OnesHash);

	/* Enable Hashing */
	SkMacHashing(pAC, IoC, (int) PortNumber, SK_TRUE);

	(void) SkAddrGmacPromiscuousChange(pAC, IoC, PortNumber, pAPort->PromMode);

#endif /* SK_SLIM */

	/* Set port's current physical MAC address. */
	OutAddr = (SK_U16 *) &pAPort->CurrentMacAddress.a[0];
	GM_OUTADDR(IoC, PortNumber, GM_SRC_ADDR_1L, OutAddr);

	/* Set port's current logical MAC address. */
	OutAddr = (SK_U16 *) &pAPort->Exact[0].a[0];
	GM_OUTADDR(IoC, PortNumber, GM_SRC_ADDR_2L, OutAddr);

#ifdef DEBUG
	SK_DBG_MSG(pAC, SK_DBGMOD_ADDR, SK_DBGCAT_CTRL,
		("SkAddrGmacMcUpdate: Current logical MAC Address[%d]: %02X %02X %02X %02X %02X %02X\n",
			PortNumber,
			pAPort->Exact[0].a[0],
			pAPort->Exact[0].a[1],
			pAPort->Exact[0].a[2],
			pAPort->Exact[0].a[3],
			pAPort->Exact[0].a[4],
			pAPort->Exact[0].a[5]));

	SK_DBG_MSG(pAC, SK_DBGMOD_ADDR, SK_DBGCAT_CTRL,
		("SkAddrGmacMcUpdate: Current physical MAC Address[%d]: %02X %02X %02X %02X %02X %02X\n",
			PortNumber,
			pAPort->CurrentMacAddress.a[0],
			pAPort->CurrentMacAddress.a[1],
			pAPort->CurrentMacAddress.a[2],
			pAPort->CurrentMacAddress.a[3],
			pAPort->CurrentMacAddress.a[4],
			pAPort->CurrentMacAddress.a[5]));
#endif /* DEBUG */

#ifndef SK_SLIM
	/* Determine return value. */
	if (Inexact == 0 && pAPort->PromMode == 0) {
		return (SK_MC_FILTERING_EXACT);
	}
	else {
		return (SK_MC_FILTERING_INEXACT);
	}
#else /* SK_SLIM */
	return (SK_MC_FILTERING_INEXACT);
#endif /* SK_SLIM */

}	/* SkAddrGmacMcUpdate */

#ifndef SK_NO_MAO

/******************************************************************************
 *
 *	SkAddrOverride - override a port's MAC address
 *
 * Description:
 *	This routine overrides the MAC address of one port.
 *
 * NOTE: It can be called with pNewAddr set to NULL from the RLMT module!
 *       This is correct in combination with Flags set to SK_ADDR_SET_LOGICAL
 *       or SK_ADDR_CLEAR_LOGICAL. All other calls will be handled as an error
 *       and SK_MC_ILLEGAL_ADDRESS will be returned then.
 *
 * Context:
 *	runtime, pageable
 *	may be called after SK_INIT_IO
 *
 * Returns:
 *	SK_ADDR_SUCCESS if successful.
 *	SK_ADDR_DUPLICATE_ADDRESS if duplicate MAC address.
 *	SK_ADDR_MULTICAST_ADDRESS if multicast or broadcast address.
 *	SK_ADDR_TOO_EARLY if SK_INIT_IO was not executed before.
 */
int	SkAddrOverride(
SK_AC		*pAC,				/* adapter context */
SK_IOC		IoC,				/* I/O context */
SK_U32		PortNumber,			/* Port Number */
SK_MAC_ADDR	SK_FAR *pNewAddr,	/* new MAC address */
int			Flags)				/* logical/physical MAC address */
{
#ifndef SK_NO_RLMT
	SK_EVPARA	Para;
#endif /* !SK_NO_RLMT */
	SK_U32		NetNumber;
	SK_U32		i;
	SK_U16		SK_FAR *OutAddr;

#ifndef SK_NO_RLMT
	NetNumber = pAC->Rlmt.Port[PortNumber].Net->NetNumber;
#else
#ifndef SK_SLIM
	NetNumber = PortNumber;
#else
	NetNumber = 0;
#endif /* SK_SLIM */
#endif /* SK_NO_RLMT */
#if (!defined(SK_SLIM) || defined(DEBUG))
	if (PortNumber >= (SK_U32) pAC->GIni.GIMacsFound) {
		return (SK_ADDR_ILLEGAL_PORT);
	}
#endif /* !SK_SLIM || DEBUG */

	if (pNewAddr != NULL && (pNewAddr->a[0] & SK_MC_BIT) != 0) {
		return (SK_ADDR_MULTICAST_ADDRESS);
	}

	if (!pAC->Addr.Net[NetNumber].CurrentMacAddressSet) {
		return (SK_ADDR_TOO_EARLY);
	}

	if (Flags & SK_ADDR_SET_LOGICAL) {	/* Activate logical MAC address. */
		/* Parameter *pNewAddr is ignored. */
		for (i = 0; i < (SK_U32) pAC->GIni.GIMacsFound; i++) {
			if (!pAC->Addr.Port[i].CurrentMacAddressSet) {
				return (SK_ADDR_TOO_EARLY);
			}
		}
#ifndef SK_NO_RLMT
		/* Set PortNumber to number of net's active port. */
		PortNumber = pAC->Rlmt.Net[NetNumber].
			Port[pAC->Addr.Net[NetNumber].ActivePort]->PortNumber;
#endif /* !SK_NO_RLMT */
		pAC->Addr.Port[PortNumber].Exact[0] =
			pAC->Addr.Net[NetNumber].CurrentMacAddress;

		/* Write address to first exact match entry of active port. */
		(void) SkAddrMcUpdate(pAC, IoC, PortNumber);
	}
	else if (Flags & SK_ADDR_CLEAR_LOGICAL) {
		/* Deactivate logical MAC address. */
		/* Parameter *pNewAddr is ignored. */
		for (i = 0; i < (SK_U32) pAC->GIni.GIMacsFound; i++) {
			if (!pAC->Addr.Port[i].CurrentMacAddressSet) {
				return (SK_ADDR_TOO_EARLY);
			}
		}
#ifndef SK_NO_RLMT
		/* Set PortNumber to number of net's active port. */
		PortNumber = pAC->Rlmt.Net[NetNumber].
			Port[pAC->Addr.Net[NetNumber].ActivePort]->PortNumber;
#endif /* !SK_NO_RLMT */
		for (i = 0; i < SK_MAC_ADDR_LEN; i++ ) {
			pAC->Addr.Port[PortNumber].Exact[0].a[i] = 0;
		}

		/* Write address to first exact match entry of active port. */
		(void) SkAddrMcUpdate(pAC, IoC, PortNumber);
	}
	else if (pNewAddr == NULL) {
		/* Prevent accessing a NULL pointer (was complained by Prefast)  */
		return (SK_MC_ILLEGAL_ADDRESS);
	}
	else {
		if (Flags & SK_ADDR_PHYSICAL_ADDRESS) {	/* Physical MAC address. */

			for (i = 0; i < (SK_U32) pAC->GIni.GIMacsFound; i++) {
				if (!pAC->Addr.Port[i].CurrentMacAddressSet) {
					return (SK_ADDR_TOO_EARLY);
				}
			}

			/*
			 * In dual net mode it should be possible to set all MAC
			 * addresses independently. Therefore the equality checks
			 * against the locical address of the same port and the
			 * physical address of the other port are suppressed here.
			 */
#ifndef SK_NO_RLMT
			if (pAC->Rlmt.NumNets == 1) {
#endif /* SK_NO_RLMT */
				if (SK_ADDR_EQUAL(pNewAddr->a,
					pAC->Addr.Net[NetNumber].CurrentMacAddress.a)) {
					return (SK_ADDR_DUPLICATE_ADDRESS);
				}

				for (i = 0; i < (SK_U32) pAC->GIni.GIMacsFound; i++) {
					if (SK_ADDR_EQUAL(pNewAddr->a,
						pAC->Addr.Port[i].CurrentMacAddress.a)) {
						if (i == PortNumber) {
							return (SK_ADDR_SUCCESS);
						}
						else {
							return (SK_ADDR_DUPLICATE_ADDRESS);
						}
					}
				}
#ifndef SK_NO_RLMT
			}
			else {
				if (SK_ADDR_EQUAL(pNewAddr->a,
					pAC->Addr.Port[PortNumber].CurrentMacAddress.a)) {
					return (SK_ADDR_SUCCESS);
				}
			}
#endif /* SK_NO_RLMT */

			pAC->Addr.Port[PortNumber].PreviousMacAddress =
				pAC->Addr.Port[PortNumber].CurrentMacAddress;
			pAC->Addr.Port[PortNumber].CurrentMacAddress = *pNewAddr;

			/* Change port's physical MAC address. */
			OutAddr = (SK_U16 SK_FAR *) pNewAddr;
			GM_OUTADDR(IoC, PortNumber, GM_SRC_ADDR_1L, OutAddr);

#ifndef SK_NO_RLMT
			/* Report address change to RLMT. */
			Para.Para32[0] = PortNumber;
			Para.Para32[0] = -1;
			SkEventQueue(pAC, SKGE_RLMT, SK_RLMT_PORT_ADDR, Para);
#endif /* !SK_NO_RLMT */
		}
		else {	/* Logical MAC address. */
			if (SK_ADDR_EQUAL(pNewAddr->a,
				pAC->Addr.Net[NetNumber].CurrentMacAddress.a)) {
				return (SK_ADDR_SUCCESS);
			}

			for (i = 0; i < (SK_U32) pAC->GIni.GIMacsFound; i++) {
				if (!pAC->Addr.Port[i].CurrentMacAddressSet) {
					return (SK_ADDR_TOO_EARLY);
				}
			}

			/*
			 * In dual net mode on Yukon-2 adapters the physical address
			 * of port 0 and the logical address of port 1 are equal - in
			 * this case the equality check of the physical address leads
			 * to an error and is suppressed here.
			 */
#ifndef SK_NO_RLMT
			if (pAC->Rlmt.NumNets == 1) {
#endif /* SK_NO_RLMT */
				for (i = 0; i < (SK_U32) pAC->GIni.GIMacsFound; i++) {
					if (SK_ADDR_EQUAL(pNewAddr->a,
						pAC->Addr.Port[i].CurrentMacAddress.a)) {
						return (SK_ADDR_DUPLICATE_ADDRESS);
					}
				}
#ifndef SK_NO_RLMT
			}
#endif /* SK_NO_RLMT */

			/*
			 * In case that the physical and the logical MAC addresses are equal
			 * we must also change the physical MAC address here.
			 * In this case we have an adapter which initially was programmed with
			 * two identical MAC addresses.
			 */
			if (SK_ADDR_EQUAL(pAC->Addr.Port[PortNumber].CurrentMacAddress.a,
					pAC->Addr.Port[PortNumber].Exact[0].a)) {

				pAC->Addr.Port[PortNumber].PreviousMacAddress =
					pAC->Addr.Port[PortNumber].CurrentMacAddress;
				pAC->Addr.Port[PortNumber].CurrentMacAddress = *pNewAddr;

#ifndef SK_NO_RLMT
				/* Report address change to RLMT. */
				Para.Para32[0] = PortNumber;
				Para.Para32[0] = -1;
				SkEventQueue(pAC, SKGE_RLMT, SK_RLMT_PORT_ADDR, Para);
#endif /* !SK_NO_RLMT */
			}

#ifndef SK_NO_RLMT
			/* Set PortNumber to number of net's active port. */
			PortNumber = pAC->Rlmt.Net[NetNumber].
				Port[pAC->Addr.Net[NetNumber].ActivePort]->PortNumber;
#endif /* !SK_NO_RLMT */
			pAC->Addr.Net[NetNumber].CurrentMacAddress = *pNewAddr;
			pAC->Addr.Port[PortNumber].Exact[0] = *pNewAddr;

#ifdef DEBUG
			SK_DBG_MSG(pAC,SK_DBGMOD_ADDR, SK_DBGCAT_CTRL,
				("SkAddrOverride: Permanent MAC Address[%d]: %02X %02X %02X %02X %02X %02X\n",
					NetNumber,
					pAC->Addr.Net[NetNumber].PermanentMacAddress.a[0],
					pAC->Addr.Net[NetNumber].PermanentMacAddress.a[1],
					pAC->Addr.Net[NetNumber].PermanentMacAddress.a[2],
					pAC->Addr.Net[NetNumber].PermanentMacAddress.a[3],
					pAC->Addr.Net[NetNumber].PermanentMacAddress.a[4],
					pAC->Addr.Net[NetNumber].PermanentMacAddress.a[5]));

			SK_DBG_MSG(pAC,SK_DBGMOD_ADDR, SK_DBGCAT_CTRL,
				("SkAddrOverride: New logical MAC Address[%d]: %02X %02X %02X %02X %02X %02X\n",
					NetNumber,
					pAC->Addr.Net[NetNumber].CurrentMacAddress.a[0],
					pAC->Addr.Net[NetNumber].CurrentMacAddress.a[1],
					pAC->Addr.Net[NetNumber].CurrentMacAddress.a[2],
					pAC->Addr.Net[NetNumber].CurrentMacAddress.a[3],
					pAC->Addr.Net[NetNumber].CurrentMacAddress.a[4],
					pAC->Addr.Net[NetNumber].CurrentMacAddress.a[5]));
#endif /* DEBUG */

			/* Write address to first exact match entry of active port. */
			(void)SkAddrMcUpdate(pAC, IoC, PortNumber);
		}
	}

	return (SK_ADDR_SUCCESS);

}	/* SkAddrOverride */

#endif /* SK_NO_MAO */

/******************************************************************************
 *
 *	SkAddrPromiscuousChange - set promiscuous mode for given port
 *
 * Description:
 *	This routine manages promiscuous mode:
 *	- none
 *	- all LLC frames
 *	- all MC frames
 *
 *	It calls either SkAddrXmacPromiscuousChange or
 *	SkAddrGmacPromiscuousChange, according to the adapter in use.
 *	The real work is done there.
 *
 * Context:
 *	runtime, pageable
 *	may be called after SK_INIT_IO
 *
 * Returns:
 *	SK_ADDR_SUCCESS
 *	SK_ADDR_ILLEGAL_PORT
 */
int	SkAddrPromiscuousChange(
SK_AC	*pAC,			/* adapter context */
SK_IOC	IoC,			/* I/O context */
SK_U32	PortNumber,		/* port whose promiscuous mode changes */
int		NewPromMode)	/* new promiscuous mode */
{
	int ReturnCode = SK_ADDR_ILLEGAL_PORT;

#if (!defined(SK_SLIM) || defined(DEBUG))
	if (PortNumber >= (SK_U32) pAC->GIni.GIMacsFound) {
		return (SK_ADDR_ILLEGAL_PORT);
	}
#endif /* !SK_SLIM || DEBUG */

	ReturnCode =
		SkAddrGmacPromiscuousChange(pAC, IoC, PortNumber, NewPromMode);
	return (ReturnCode);

}	/* SkAddrPromiscuousChange */


/******************************************************************************
 *
 *	SkAddrGmacPromiscuousChange - set promiscuous mode for given port
 *
 * Description:
 *	This routine manages promiscuous mode:
 *	- none
 *	- all LLC frames
 *	- all MC frames
 *
 * Context:
 *	runtime, pageable
 *	may be called after SK_INIT_IO
 *
 * Returns:
 *	SK_ADDR_SUCCESS
 *	SK_ADDR_ILLEGAL_PORT
 */
int	SkAddrGmacPromiscuousChange(
SK_AC	*pAC,			/* adapter context */
SK_IOC	IoC,			/* I/O context */
SK_U32	PortNumber,		/* port whose promiscuous mode changes */
int		NewPromMode)	/* new promiscuous mode */
{
	if (PortNumber >= (SK_U32)pAC->GIni.GIMacsFound) {
		return (SK_ADDR_ILLEGAL_PORT);
	}

	pAC->Addr.Port[PortNumber].PromMode = NewPromMode;

	switch(NewPromMode) {
	case (SK_PROM_MODE_NONE):							/* 0 */

		/* Normal receive mode */
		SkMacPromiscMode(pAC, IoC, (int) PortNumber, SK_FALSE);
		break;

	case (SK_PROM_MODE_LLC):							/* 1 */
		/* This mode is ignored and mapped to prom. mode */
	case (SK_PROM_MODE_LLC | SK_PROM_MODE_ALL_MC):		/* 3 */

		/* Set the MAC to promiscuous mode. */
		SkMacPromiscMode(pAC, IoC, (int) PortNumber, SK_TRUE);
		break;

	case (SK_PROM_MODE_ALL_MC):							/* 2 */

		/* Disable MC hashing */
		SkMacHashing(pAC, IoC, (int) PortNumber, SK_FALSE);
		break;

	default:
		break;
	}

	return (SK_ADDR_SUCCESS);
}	/* SkAddrGmacPromiscuousChange */

#if (!defined(SK_SLIM) && !defined(SK_NO_RLMT))

/******************************************************************************
 *
 *	SkAddrSwap - swap address info
 *
 * Description:
 *	This routine swaps address info of two ports.
 *
 * Context:
 *	runtime, pageable
 *	may be called after SK_INIT_IO
 *
 * Returns:
 *	SK_ADDR_SUCCESS
 *	SK_ADDR_ILLEGAL_PORT
 */
int	SkAddrSwap(
SK_AC	*pAC,			/* adapter context */
SK_IOC	IoC,			/* I/O context */
SK_U32	FromPortNumber,		/* Port1 Index */
SK_U32	ToPortNumber)		/* Port2 Index */
{
	int			i;
	SK_U8		Byte;
	SK_MAC_ADDR	MacAddr;

	if (FromPortNumber >= (SK_U32) pAC->GIni.GIMacsFound) {
		return (SK_ADDR_ILLEGAL_PORT);
	}

	if (ToPortNumber >= (SK_U32) pAC->GIni.GIMacsFound) {
		return (SK_ADDR_ILLEGAL_PORT);
	}

	if (pAC->Rlmt.Port[FromPortNumber].Net != pAC->Rlmt.Port[ToPortNumber].Net) {
		return (SK_ADDR_ILLEGAL_PORT);
	}

	/*
	 * Swap:
	 * - Exact Match Entries:
	 *   Yukon uses first entry for the logical MAC
	 *   address (stored in the second GMAC register).
	 * - 64-bit filter (InexactFilter)
	 * - Promiscuous Mode
	 * of ports.
	 */

	for (i = 0; i < SK_ADDR_EXACT_MATCHES; i++) {
		MacAddr = pAC->Addr.Port[FromPortNumber].Exact[i];
		pAC->Addr.Port[FromPortNumber].Exact[i] =
			pAC->Addr.Port[ToPortNumber].Exact[i];
		pAC->Addr.Port[ToPortNumber].Exact[i] = MacAddr;
	}

	for (i = 0; i < 8; i++) {
		Byte = pAC->Addr.Port[FromPortNumber].InexactFilter.Bytes[i];
		pAC->Addr.Port[FromPortNumber].InexactFilter.Bytes[i] =
			pAC->Addr.Port[ToPortNumber].InexactFilter.Bytes[i];
		pAC->Addr.Port[ToPortNumber].InexactFilter.Bytes[i] = Byte;
	}

	i = pAC->Addr.Port[FromPortNumber].PromMode;
	pAC->Addr.Port[FromPortNumber].PromMode = pAC->Addr.Port[ToPortNumber].PromMode;
	pAC->Addr.Port[ToPortNumber].PromMode = i;

	/* CAUTION: Solution works if only ports of one adapter are in use. */
	for (i = 0; (SK_U32) i < pAC->Rlmt.Net[pAC->Rlmt.Port[ToPortNumber].
		Net->NetNumber].NumPorts; i++) {
		if (pAC->Rlmt.Net[pAC->Rlmt.Port[ToPortNumber].Net->NetNumber].
			Port[i]->PortNumber == ToPortNumber) {
			pAC->Addr.Net[pAC->Rlmt.Port[ToPortNumber].Net->NetNumber].
				ActivePort = i;
			/* 20001207 RA: Was "ToPortNumber;". */
		}
	}

	(void) SkAddrMcUpdate(pAC, IoC, FromPortNumber);
	(void) SkAddrMcUpdate(pAC, IoC, ToPortNumber);

	return (SK_ADDR_SUCCESS);

}	/* SkAddrSwap */

#endif /* !SK_SLIM && !SK_NO_RLMT */

#ifdef __cplusplus
}
#endif	/* __cplusplus */

