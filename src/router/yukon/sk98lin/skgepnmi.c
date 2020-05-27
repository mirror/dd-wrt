/*****************************************************************************
 *
 * Name:	skgepnmi.c
 * Project:	Gigabit Ethernet Adapters, PNMI-Module
 * Version:	$Revision: #7 $
 * Date:	$Date: 2010/11/10 $
 * Purpose:	Private Network Management Interface
 *
 ****************************************************************************/

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
 *	$Log: skgepnmi.c,v $
 *	Revision 2.44  2010/05/04 14:14:07  tschilli
 *	Readded define SK_USE_FWCOMMON to allow compilation with ASF function calls
 *	(accidentally removed at last check in).
 *
 *	Revision 2.43  2010/05/04 09:22:51  rschmidt
 *	Adapted VCT results for Yukon-Optima.
 *	Editorial changes.
 *
 *	Revision 2.42  2010/04/29 14:30:14  tschilli
 *	Changed definition static void VctGetResults() to PNMI_STATIC void VctGetResults() to allow compilation with DEBUG flag set.
 *
 *	Revision 2.41  2010/03/23 16:28:28  tschilli
 *	Removed unnecessary start of RLMT change count timer in case SK_NO_RLMT is set.
 *	Added missing PNMI chipsets used for pAC->Pnmi.Chipset in SkPnmiInit().
 *
 *	Revision 2.40  2009/10/27 15:38:54  tschilli
 *	Added define SK_USE_FWCOMMON to allow compilation with ASF function calls.
 *
 *	Revision 2.39  2009/10/27 08:26:35  tschilli
 *	Added changes to support compilation without RLMT (SK_NO_RLMT)
 *	Added changes to support new firmware common module (fwcommon)
 *
 *	Revision 2.38  2009/03/04 09:46:49  rschmidt
 *	Used macro HW_HAS_NEWER_PHY for VCT DSP.
 *	Fixed PREfast compiler warnings.
 *	Editorial changes.
 *
 *	Revision 2.37  2009/02/25 13:41:00  sschmeck
 *	Added OIDs for Win7 Power Management.
 *
 *	Revision 2.36  2009/02/17 13:44:04  rschmidt
 *	Used RLMT change counter timer only for dual port adapters.
 *	Adapted VCT for Yukon chips after Supreme.
 *	Editorial changes.
 *
 *	Revision 2.35  2008/04/09 16:16:07  mschmid
 *	Added Yukon-Ultra 2 support for VCT.
 *
 *	Revision 2.34  2008/04/01 08:50:20  tschilli
 *	Fix: Removed unused variable Word in GetPhysStatVal().
 *
 *	Revision 2.33  2008/03/20 08:50:32  tschilli
 *	Removed support of GENESIS adapters.
 *
 *	Revision 2.32  2007/11/06 16:42:32  mschmid
 *	Modified bias to calculate cable length for Yukon-Extreme and Yukon-Supreme.
 *
 *	Revision 2.31  2007/11/05 08:00:28  tschilli
 *	Added VCT support for Yukon-Supreme.
 *
 *	Revision 2.30  2007/09/25 15:22:46  tschilli
 *	Removed unused variable from VctGetDspLength() to avoid a compiler warning.
 *
 *	Revision 2.29  2007/09/03 14:16:57  mkunz
 *	Changed handling of VCT. The results are interpreted now in PNMI directly.
 *	The mode they are written into the buffer is capabilities version = 1.
 *	Known limitation: Dual Net support missing.
 *
 *	Revision 2.28  2007/07/25 14:01:19  mkarl
 *	Fixed the problem that the first call to SkPnmiGetStruct failed when the
 *	adapter has no VPD. This prevented the Novell driver from working with
 *	Client32 on non-VPD adapters.
 *
 *	Revision 2.27  2007/01/12 10:37:12  rschmidt
 *	Removed handling of VCT status and length from VctGetResults().
 *	Editorial changes.
 *
 *	Revision 2.26  2005/12/21 09:53:21  tschilli
 *	Removed public function declarations, declaration of Asf() and
 *	declaration of Vct() to get rid of FreeBSD compiler warnings.
 *
 *	Revision 2.25  2005/12/14 16:11:18  ibrueder
 *	Added copyright header flags
 *
 *	Revision 2.24  2005/10/21 09:54:35  mkunz
 *	Changes for DualNet WoL and Power Management.
 *
 *	Revision 2.23  2005/08/09 09:05:12  tschilli
 *	Changed GetVpdKeyArr(), SkPnmiGetStruct(), Vpd() and General()
 *	to fix VPD handling in case of wrong or missing VPD.
 *
 *	Revision 2.22  2005/05/17 12:13:57  tschilli
 *	Moved initialisation of DSP variables for Vct() from
 *	SK_INIT_IO to SK_INIT_DATA.
 *
 *	Revision 2.21  2005/05/11 11:50:12  tschilli
 *	Vct(): Fixed instance number check in the case of using other
 *	instance numbers than -1.
 *
 *	Revision 2.20  2005/05/03 06:46:13  tschilli
 *	OID_SKGE_CHIPSET: Fixed return values for missing chipsets.
 *	Fixed VCT code to retrieve correct values of the second port.
 *
 *	Revision 2.19  2004/10/29 09:09:43  tschilli
 *	Fixed Lint error in GetVpdKeyArr().
 *
 *	Revision 2.18  2004/10/20 13:36:31  mkunz
 *	Correct Rx error counters.
 *
 *	Revision 2.17  2004/09/17 11:50:03  tschilli
 *	Changed VCT code for Yukon-FE.
 *
 *	Revision 2.16  2004/09/13 08:02:42  mkunz
 *	Replaced SK_PNMI_HRX_MISSED through
 *	SK_PNMI_HRX_OVERFLOW in
 *	OID_GEN_RCV_NO_BUFFER.
 *
 *	Revision 2.15  2004/09/08 15:41:07  mkunz
 *	Added counter of RX_MISSED to OID_GEN_RCV_NO_BUFFER.
 *	This is the HW counter for the events when packets were lost due to
 *	lack of NIC resources.
 *
 *	Revision 2.14  2004/08/25 12:35:00  mlindner
 *	Fix: Removed unused semicolon
 *
 *	Revision 2.13  2004/06/14 06:50:30  tschilli
 *	Vct(): Handling for OID_SKGE_VCT_CAPABILITIES added.
 *	VctGetResults(): New cable status SK_PNMI_VCT_NOT_PRESENT added.
 *	Variable CurrentPhyPowerState included into #ifdef SK_PHY_LP_MODE
 *	to be not visible when compiling without SK_PHY_LP_MODE set.
 *
 *	Revision 2.12  2004/05/04 13:05:03  tschilli
 *	SkPnmiEvent(): Unused variable pPrt removed.
 *	OID_SKGE_PHY_TYPE in VirtualConf(): Missing break added.
 *
 *	Revision 2.11  2004/04/29 10:07:57  mkunz
 *	Remember current PHY low power mode,
 *	it is changed in a function call to
 *	SkGmLeaveLowPowerMode but the
 *	original one is still needed later.
 *
 *	Revision 2.10  2004/04/29 08:16:05  mkunz
 *	Changed OID for low power mode
 *
 *	Revision 2.9  2004/04/29 07:46:32  mkunz
 *	Roll back to previous version
 *
 *	Revision 2.8  2004/04/08 16:09:08  mkunz
 *	Changes required for PHY low power mode handling
 *
 *	Revision 2.7  2004/03/20 14:36:11  mkunz
 *	Fixed compile error, editorial change
 *
 *	Revision 2.6  2004/03/19 16:44:49  tschilli
 *	Merged the changes of revision 1.113.
 *	Merged the changes of revision 1.112.
 *
 *	Revision 2.5  2004/02/26 16:01:20  mkunz
 *	Changes for ASF
 *
 *	Revision 2.4  2004/02/26 09:21:29  mkunz
 *	Added #include "h/sky2le.h"
 *
 *	Revision 2.3  2004/02/02 15:58:14  mkunz
 *	Added ASF support
 *
 *	Revision 2.2  2003/12/02 12:56:49  mkarl
 *	Added ifdef GENESIS.
 *
 *	Revision 2.1  2003/10/27 14:16:08  amock
 *	Promoted to major revision 2.1
 *
 *	Revision 1.1  2003/10/27 14:14:18  amock
 *	Copied to yukon2 module branch
 *
 *	Revision 1.113  2004/03/19 14:03:57  tschilli
 *	OID_SKGE_MTU: Buffer handling fixes.
 *	Two calls of SkEventDispatcher() in VCT code removed.
 *	Vct(): Check of SK_PHY_MARV_COPPER moved back to the old location.
 *	DualNet mode handling (SET operation) fixed for the OIDs:
 *	- OID_SKGE_RLMT_PORT_PREFERRED
 *	- OID_SKGE_LINK_MODE
 *	- OID_SKGE_FLOWCTRL_MODE
 *	- OID_SKGE_PHY_OPERATION_MODE
 *	- OID_SKGE_SPEED_MODE
 *	- OID_SKGE_PHY_LP_MODE
 *	SK_PNMI_HRX_FCS: Added ifdef GENESIS.
 *	Editorial changes.
 *
 *	Revision 1.112  2004/01/02 12:43:10  rschmidt
 *	Removed HW-access to internal registers in SkPnmiInit(),
 *	use of entries GIPciSlot64, GIPciClock66, GIPmdTyp and GIConTyp
 *	from s_GeInit structure instead.
 *	Replaced digital values of parameters with defines
 *	SK_PNMI_EVT_TIMER_CHECK and SK_PNMI_VCT_TIMER_CHECK.
 *	Added routine VctGetResults() for VCT calculation.
 *	Fixed some error messages for driver release date and
 *	driver file name.
 *	Fixed handling of OID_SKGE_PHY_LP_MODE for dual port.
 *	Changed handling of OID_SKGE_VCT_GET using VctGetResults().
 *	Editorial changes.
 *
 *	Revision 1.111  2003/09/15 13:35:35  tschilli
 *	Code for OID_SKGE_PHY_LP_MODE completed (using #define SK_PHY_LP_MODE).
 *	SK_DIAG_ATTACHED handling for OID_SKGE_DIAG_MODE in DiagActions() changed.
 *
 *	Revision 1.110  2003/08/15 12:28:04  tschilli
 *	Added new OIDs:
 *	OID_SKGE_DRIVER_RELDATE
 *	OID_SKGE_DRIVER_FILENAME
 *	OID_SKGE_CHIPID
 *	OID_SKGE_RAMSIZE
 *	OID_SKGE_VAUXAVAIL
 *	OID_SKGE_PHY_TYPE
 *	OID_SKGE_PHY_LP_MODE
 *
 *	Added SK_DIAG_ATTACHED handling for OID_SKGE_DIAG_MODE in DiagActions().
 *
 *	Revision 1.109  2003/07/17 14:15:24  tschilli
 *	Bug in SkPnmiGenIoctl() fixed.
 *
 *	Revision 1.108  2003/05/27 07:10:11  tschilli
 *	Bug in SkPnmiGenIoctl() fixed.
 *
 *	Revision 1.107  2003/05/23 13:01:10  tschilli
 *	Code for DIAG support added (#define SK_DIAG_SUPPORT).
 *	Code for generic PNMI IOCTL support added. The new function
 *	SkPnmiGenIoctl() is used for this purpose.
 *	Handling of OID_SKGE_BOARDLEVEL added.
 *	Incorrect buffer size handling of OID_SKGE_MTU during GET action fixed.
 *	Return code handling in PowerManagement() fixed.
 *	Editorial changes.
 *
 *	Revision 1.106  2003/04/10 14:47:31  rschmidt
 *	Fixed handling for OID_GEN_RCV_OK and OID_GEN_XMIT_OK for YUKON's GMAC
 *	in GetPhysStatVal().
 *	Replaced macro PHY_READ() with function call SkXmPhyRead().
 *	Made optimisations for readability and code size.
 *	Editorial changes.
 *
 *	Revision 1.105  2003/04/09 12:51:32  rschmidt
 *	Fixed XMAC only handling for some events in SkPnmiEvent().
 *	Fixed return value for OID_GEN_RCV_OK (SK_PNMI_HRX) in GetPhysStatVal().
 *	Editorial changes.
 *
 *	Revision 1.104  2003/03/27 11:18:21  tschilli
 *	BRK statements from DEBUG code removed.
 *	OID_GEN_XMIT_OK and OID_GEN_RCV_OK work with Yukon now.
 *	Copyright messages changed.
 *
 *	Revision 1.103  2002/12/20 09:57:13  tschilli
 *	SK_PNMI_EVT_VCT_RESET event code changed.
 *	Unused variable from Vct() removed.
 *
 *	Revision 1.102  2002/12/16 14:03:24  tschilli
 *	VCT code in Vct() changed.
 *
 *	Revision 1.101  2002/12/16 09:04:10  tschilli
 *	Code for VCT handling added.
 *
 *	Revision 1.100  2002/09/26 14:28:13  tschilli
 *	For XMAC the values in the SK_PNMI_PORT Port struct are copied to
 *	the new SK_PNMI_PORT BufPort struct during a MacUpdate() call.
 *	These values are used when GetPhysStatVal() is called. With this
 *	mechanism you get the best results when software corrections for
 *	counters are needed. Example: RX_LONGFRAMES.
 *
 *	Revision 1.99  2002/09/17 12:31:19  tschilli
 *	OID_SKGE_TX_HW_ERROR_CTS, OID_SKGE_OUT_ERROR_CTS, OID_GEN_XMIT_ERROR:
 *	Double count of SK_PNMI_HTX_EXCESS_COL in function General() removed.
 *	OID_PNP_CAPABILITIES: sizeof(SK_PM_WAKE_UP_CAPABILITIES) changed to
 *	sizeof(SK_PNP_CAPABILITIES) in function PowerManagement().
 *
 *	Revision 1.98  2002/09/10 09:00:03  rwahl
 *	Adapted boolean definitions according sktypes.
 *
 *	Revision 1.97  2002/09/05 15:07:03  rwahl
 *	Editorial changes.
 *
 *	Revision 1.96  2002/09/05 11:04:14  rwahl
 *	- Rx/Tx packets statistics of virtual port were zero on link down (#10750)
 *	- For GMAC the overflow IRQ for Rx longframe counter was not counted.
 *	- Incorrect calculation for oids OID_SKGE_RX_HW_ERROR_CTS,
 *	  OID_SKGE_IN_ERRORS_CTS,  OID_GEN_RCV_ERROR.
 *	- Moved correction for OID_SKGE_STAT_RX_TOO_LONG to GetPhysStatVal().
 *	- Editorial changes.
 *
 *	Revision 1.95  2002/09/04 08:53:37  rwahl
 *	- Incorrect statistics for Rx_too_long counter with jumbo frame (#10751)
 *	- StatRxFrameTooLong & StatRxPMaccErr counters were not reset.
 *	- Fixed compiler warning for debug msg arg types.
 *
 *	Revision 1.94  2002/08/09 15:42:14  rwahl
 *	- Fixed StatAddr table for GMAC.
 *	- VirtualConf(): returned indeterminated status for speed oids if no
 *	  active port.
 *
 *	Revision 1.93  2002/08/09 11:04:59  rwahl
 *	Added handler for link speed caps.
 *
 *	Revision 1.92  2002/08/09 09:43:03  rwahl
 *	- Added handler for NDIS OID_PNP_xxx ids.
 *
 *	Revision 1.91  2002/07/17 19:53:03  rwahl
 *	- Added StatOvrflwBit table for XMAC & GMAC.
 *	- Extended StatAddr table for GMAC. Added check of number of counters
 *	  in enumeration and size of StatAddr table on init level.
 *	- Added use of GIFunc table.
 *	- ChipSet is not static anymore,
 *	- Extended SIRQ event handler for both MAC types.
 *	- Fixed rx short counter bug (#10620)
 *	- Added handler for oids SKGE_SPEED_MODE & SKGE_SPEED_STATUS.
 *	- Extended GetPhysStatVal() for GMAC.
 *	- Editorial changes.
 *
 *	Revision 1.90  2002/05/22 08:56:25  rwahl
 *	- Moved OID table to separate source file.
 *	- Fix: TX_DEFFERAL counter incremented in full-duplex mode.
 *	- Use string definitions for error msgs.
 *
 *	Revision 1.89  2001/09/18 10:01:30  mkunz
 *	some OID's fixed for dualnetmode
 *
 *	Revision 1.88  2001/08/02 07:58:08  rwahl
 *	- Fixed NetIndex to csum module at ResetCounter().
 *
 *	Revision 1.87  2001/04/06 13:35:09  mkunz
 *	-Bugs fixed in handling of OID_SKGE_MTU and the VPD OID's
 *
 *	Revision 1.86  2001/03/09 09:18:03  mkunz
 *	Changes in SK_DBG_MSG
 *
 *	Revision 1.85  2001/03/08 09:37:31  mkunz
 *	Bugfix in ResetCounter for Pnmi.Port structure
 *
 *	Revision 1.84  2001/03/06 09:04:55  mkunz
 *	Made some changes in instance calculation
 *
 *	Revision 1.83  2001/02/15 09:15:32  mkunz
 *	Necessary changes for dual net mode added
 *
 *	Revision 1.82  2001/02/07 08:24:19  mkunz
 *	-Made changes in handling of OID_SKGE_MTU
 *
 *	Revision 1.81  2001/02/06 09:58:00  mkunz
 *	-Vpd bug fixed
 *	-OID_SKGE_MTU added
 *	-PNMI support for dual net mode. Interface function and macros extended
 *
 *	Revision 1.80  2001/01/22 13:41:35  rassmann
 *	Supporting two nets on dual-port adapters.
 *
 *	Revision 1.79  2000/12/05 14:57:40  cgoos
 *	SetStruct failed before first Link Up (link mode of virtual
 *	port "INDETERMINATED").
 *
 *	Revision 1.78  2000/09/12 10:44:58  cgoos
 *	Fixed SK_PNMI_STORE_U32 calls with typecasted argument.
 *
 *	Revision 1.77  2000/09/07 08:10:19  rwahl
 *	- Modified algorithm for 64bit NDIS statistic counters;
 *	  returns 64bit or 32bit value depending on passed buffer
 *	  size. Indicate capability for 64bit NDIS counter, if passed
 *	  buffer size is zero. OID_GEN_XMIT_ERROR, OID_GEN_RCV_ERROR,
 *	  and OID_GEN_RCV_NO_BUFFER handled as 64bit counter, too.
 *	- corrected OID_SKGE_RLMT_PORT_PREFERRED.
 *
 *	Revision 1.76  2000/08/03 15:23:39  rwahl
 *	- Correction for FrameTooLong counter has to be moved to OID handling
 *	  routines (instead of statistic counter routine).
 *	- Fix in XMAC Reset Event handling: Only offset counter for hardware
 *	  statistic registers are updated.
 *
 *	Revision 1.75  2000/08/01 16:46:05  rwahl
 *	- Added StatRxLongFrames counter and correction of FrameTooLong counter.
 *	- Added directive to control width (default = 32bit) of NDIS statistic
 *	  counters (SK_NDIS_64BIT_CTR).
 *
 *	Revision 1.74  2000/07/04 11:41:53  rwahl
 *	- Added volition connector type.
 *
 *	Revision 1.73  2000/03/15 16:33:10  rwahl
 *	Fixed bug 10510; wrong reset of virtual port statistic counters.
 *
 *	Revision 1.72  1999/12/06 16:15:53  rwahl
 *	Fixed problem of instance range for current and factory MAC address.
 *
 *	Revision 1.71  1999/12/06 10:14:20  rwahl
 *	Fixed bug 10476; set operation for PHY_OPERATION_MODE.
 *
 *	Revision 1.70  1999/11/22 13:33:34  cgoos
 *	Changed license header to GPL.
 *
 *	Revision 1.69  1999/10/18 11:42:15  rwahl
 *	Added typecasts for checking event dependent param (debug only).
 *
 *	Revision 1.68  1999/10/06 09:35:59  cgoos
 *	Added state check to PHY_READ call (hanged if called during startup).
 *
 *	Revision 1.67  1999/09/22 09:53:20  rwahl
 *	- Read Broadcom register for updating FCS error counter (1000Base-T).
 *
 *	Revision 1.66  1999/08/26 13:47:56  rwahl
 *	Added SK_DRIVER_SENDEVENT when queueing RLMT_CHANGE_THRES trap.
 *
 *	Revision 1.65  1999/07/26 07:49:35  cgoos
 *	Added two typecasts to avoid compiler warnings.
 *
 *	Revision 1.64  1999/05/20 09:24:12  cgoos
 *	Changes for 1000Base-T (sensors, Master/Slave).
 *
 *	Revision 1.63  1999/04/13 15:11:58  mhaveman
 *	Moved include of rlmt.h to header skgepnmi.h because some macros
 *	are needed there.
 *
 *	Revision 1.62  1999/04/13 15:08:07  mhaveman
 *	Replaced again SK_RLMT_CHECK_LINK with SK_PNMI_RLMT_MODE_CHK_LINK
 *	to grant unified interface by only using the PNMI header file.
 *	SK_PNMI_RLMT_MODE_CHK_LINK is defined the same as SK_RLMT_CHECK_LINK.
 *
 *	Revision 1.61  1999/04/13 15:02:48  mhaveman
 *	Changes caused by review:
 *	-Changed some comments
 *	-Removed redundant check for OID_SKGE_PHYS_FAC_ADDR
 *	-Optimized PRESET check.
 *	-Meaning of error SK_ADDR_DUPLICATE_ADDRESS changed. Set of same
 *	 address will now not cause this error. Removed corresponding check.
 *
 *	Revision 1.60  1999/03/23 10:41:23  mhaveman
 *	Added comments.
 *
 *	Revision 1.59  1999/02/19 08:01:28  mhaveman
 *	Fixed bug 10372 that after counter reset all ports were displayed
 *	as inactive.
 *
 *	Revision 1.58  1999/02/16 18:04:47  mhaveman
 *	Fixed problem of twisted OIDs SENSOR_WAR_TIME and SENSOR_ERR_TIME.
 *
 *	Revision 1.56  1999/01/27 12:29:11  mhaveman
 *	SkTimerStart was called with time value in milli seconds but needs
 *	micro seconds.
 *
 *	Revision 1.55  1999/01/25 15:00:38  mhaveman
 *	Added support to allow multiple ports to be active. If this feature in
 *	future will be used, the Management Data Base variables PORT_ACTIVE
 *	and PORT_PREFERED should be moved to the port specific part of RLMT.
 *	Currently they return the values of the first active physical port
 *	found. A set to the virtual port will actually change all active
 *	physical ports. A get returns the melted values of all active physical
 *	ports. If the port values differ a return value INDETERMINATED will
 *	be returned. This effects especially the CONF group.
 *
 *	Revision 1.54  1999/01/19 10:10:22  mhaveman
 *	-Fixed bug 10354: Counter values of virtual port were wrong after port
 *	 switches
 *	-Added check if a switch to the same port is notified.
 *
 *	Revision 1.53  1999/01/07 09:25:21  mhaveman
 *	Forgot to initialize a variable.
 *
 *	Revision 1.52  1999/01/05 10:34:33  mhaveman
 *	Fixed little error in RlmtChangeEstimate calculation.
 *
 *	Revision 1.51  1999/01/05 09:59:07  mhaveman
 *	-Moved timer start to init level 2
 *	-Redesigned port switch average calculation to avoid 64bit
 *	 arithmetic.
 *
 *	Revision 1.50  1998/12/10 15:13:59  mhaveman
 *	-Fixed: PHYS_CUR_ADDR returned wrong addresses
 *	-Fixed: RLMT_PORT_PREFERED and RLMT_CHANGE_THRES preset returned
 *	        always BAD_VALUE.
 *	-Fixed: TRAP buffer seemed to sometimes suddenly empty
 *
 *	Revision 1.49  1998/12/09 16:17:07  mhaveman
 *	Fixed: Couldnot delete VPD keys on UNIX.
 *
 *	Revision 1.48  1998/12/09 14:11:10  mhaveman
 *	-Add: Debugmessage for XMAC_RESET supressed to minimize output.
 *	-Fixed: RlmtChangeThreshold will now be initialized.
 *	-Fixed: VPD_ENTRIES_LIST extended value with unnecessary space char.
 *	-Fixed: On VPD key creation an invalid key name could be created
 *	        (e.g. A5)
 *	-Some minor changes in comments and code.
 *
 *	Revision 1.47  1998/12/08 16:00:31  mhaveman
 *	-Fixed: For RLMT_PORT_ACTIVE will now be returned a 0 if no port
 *		is active.
 *	-Fixed: For the RLMT statistics group only the last value was
 *		returned and the rest of the buffer was filled with 0xff
 *	-Fixed: Mysteriously the preset on RLMT_MODE still returned
 *		BAD_VALUE.
 *	Revision 1.46  1998/12/08 10:04:56  mhaveman
 *	-Fixed: Preset on RLMT_MODE returned always BAD_VALUE error.
 *	-Fixed: Alignment error in GetStruct
 *	-Fixed: If for Get/Preset/SetStruct the buffer size is equal or
 *	        larger than SK_PNMI_MIN_STRUCT_SIZE the return value is stored
 *		to the buffer. In this case the caller should always return
 *	        ok to its upper routines. Only if the buffer size is less
 *	        than SK_PNMI_MIN_STRUCT_SIZE and the return value is unequal
 *	        to 0, an error should be returned by the caller.
 *	-Fixed: Wrong number of instances with RLMT statistic.
 *	-Fixed: Return now SK_LMODE_STAT_UNKNOWN if the LinkModeStatus is 0.
 *
 *	Revision 1.45  1998/12/03 17:17:24  mhaveman
 *	-Removed for VPD create action the buffer size limitation to 4 bytes.
 *	-Pass now physical/active physical port to ADDR for CUR_ADDR set
 *
 *	Revision 1.44  1998/12/03 15:14:35  mhaveman
 *	Another change to Vpd instance evaluation.
 *
 *	Revision 1.43  1998/12/03 14:18:10  mhaveman
 *	-Fixed problem in PnmiSetStruct. It was impossible to set any value.
 *	-Removed VPD key evaluation for VPD_FREE_BYTES and VPD_ACTION.
 *
 *	Revision 1.42  1998/12/03 11:31:47  mhaveman
 *	Inserted cast to satisfy lint.
 *
 *	Revision 1.41  1998/12/03 11:28:16  mhaveman
 *	Removed SK_PNMI_CHECKPTR
 *
 *	Revision 1.40  1998/12/03 11:19:07  mhaveman
 *	Fixed problems
 *	-A set to virtual port will now be ignored. A set with broadcast
 *	 address to any port will be ignored.
 *	-GetStruct function made VPD instance calculation wrong.
 *	-Prefered port returned -1 instead of 0.
 *
 *	Revision 1.39  1998/11/26 15:30:29  mhaveman
 *	Added sense mode to link mode.
 *
 *	Revision 1.38  1998/11/23 15:34:00  mhaveman
 *	-Fixed bug for RX counters. On an RX overflow interrupt the high
 *	 words of all RX counters were incremented.
 *	-SET operations on FLOWCTRL_MODE and LINK_MODE accept now the
 *	 value 0, which has no effect. It is usefull for multiple instance
 *	 SETs.
 *
 *	Revision 1.37  1998/11/20 08:02:04  mhaveman
 *	-Fixed: Ports were compared with MAX_SENSORS
 *	-Fixed: Crash in GetTrapEntry with MEMSET macro
 *	-Fixed: Conversions between physical, logical port index and instance
 *
 *	Revision 1.36  1998/11/16 07:48:53  mhaveman
 *	Casted SK_DRIVER_SENDEVENT with (void) to eleminate compiler warnings
 *	on Solaris.
 *
 *	Revision 1.35  1998/11/16 07:45:34  mhaveman
 *	SkAddrOverride now returns value and will be checked.
 *
 *	Revision 1.34  1998/11/10 13:40:37  mhaveman
 *	Needed to change interface, because NT driver needs a return value
 *	of needed buffer space on TOO_SHORT errors. Therefore all
 *	SkPnmiGet/Preset/Set functions now have a pointer to the length
 *	parameter, where the needed space on error is returned.
 *
 *	Revision 1.33  1998/11/03 13:52:46  mhaveman
 *	Made file lint conform.
 *
 *	Revision 1.32  1998/11/03 13:19:07  mhaveman
 *	The events SK_HWEV_SET_LMODE and SK_HWEV_SET_FLOWMODE pass now in
 *	Para32[0] the physical MAC index and in Para32[1] the new mode.
 *
 *	Revision 1.31  1998/11/03 12:30:40  gklug
 *	fix: compiler warning memset
 *
 *	Revision 1.30  1998/11/03 12:04:46  mhaveman
 *	Fixed problem in SENSOR_VALUE, which wrote beyond the buffer end
 *	Fixed alignment problem with CHIPSET.
 *
 *	Revision 1.29  1998/11/02 11:23:54  mhaveman
 *	Corrected SK_ERROR_LOG to SK_ERR_LOG. Sorry.
 *
 *	Revision 1.28  1998/11/02 10:47:16  mhaveman
 *	Added syslog messages for internal errors.
 *
 *	Revision 1.27  1998/10/30 15:48:06  mhaveman
 *	Fixed problems after simulation of SK_PNMI_EVT_CHG_EST_TIMER and
 *	RlmtChangeThreshold calculation.
 *
 *	Revision 1.26  1998/10/29 15:36:55  mhaveman
 *	-Fixed bug in trap buffer handling.
 *	-OID_SKGE_DRIVER_DESCR, OID_SKGE_DRIVER_VERSION, OID_SKGE_HW_DESCR,
 *	 OID_SKGE_HW_VERSION, OID_SKGE_VPD_ENTRIES_LIST, OID_SKGE_VPD_KEY,
 *	 OID_SKGE_VPD_VALUE, and OID_SKGE_SENSOR_DESCR return values with
 *	 a leading octet before each string storing the string length.
 *	-Perform a RlmtUpdate during SK_PNMI_EVT_XMAC_RESET to minimize
 *	 RlmtUpdate calls in GetStatVal.
 *	-Inserted SK_PNMI_CHECKFLAGS macro increase readability.
 *
 *	Revision 1.25  1998/10/29 08:50:36  mhaveman
 *	Fixed problems after second event simulation.
 *
 *	Revision 1.24  1998/10/28 08:44:37  mhaveman
 *	-Fixed alignment problem
 *	-Fixed problems during event simulation
 *	-Fixed sequence of error return code (INSTANCE -> ACCESS -> SHORT)
 *	-Changed type of parameter Instance back to SK_U32 because of VPD
 *	-Updated new VPD function calls
 *
 *	Revision 1.23  1998/10/23 10:16:37  mhaveman
 *	Fixed bugs after buffer test simulation.
 *
 *	Revision 1.22  1998/10/21 13:23:52  mhaveman
 *	-Call syntax of SkOsGetTime() changed to SkOsGetTime(pAc).
 *	-Changed calculation of hundreds of seconds.
 *
 *	Revision 1.20  1998/10/20 07:30:45  mhaveman
 *	Made type changes to unsigned integer where possible.
 *
 *	Revision 1.19  1998/10/19 10:51:30  mhaveman
 *	-Made Bug fixes after simulation run
 *	-Renamed RlmtMAC... to RlmtPort...
 *	-Marked workarounds with Errata comments
 *
 *	Revision 1.18  1998/10/14 07:50:08  mhaveman
 *	-For OID_SKGE_LINK_STATUS the link down detection has moved from RLMT
 *	 to HWACCESS.
 *	-Provided all MEMCPY/MEMSET macros with (char *)pointers, because
 *	 Solaris throwed warnings when mapping to bcopy/bset.
 *
 *	Revision 1.17  1998/10/13 07:42:01  mhaveman
 *	-Added OIDs OID_SKGE_TRAP_NUMBER and OID_SKGE_ALL_DATA
 *	-Removed old cvs history entries
 *	-Renamed MacNumber to PortNumber
 *
 *	Revision 1.16  1998/10/07 10:52:49  mhaveman
 *	-Inserted handling of some OID_GEN_ Ids for Windows
 *	-Fixed problem with 803.2 statistic.
 *
 *	Revision 1.15  1998/10/01 09:16:29  mhaveman
 *	Added Debug messages for function call and UpdateFlag tracing.
 *
 *	Revision 1.14  1998/09/30 13:39:09  mhaveman
 *	-Reduced namings of 'MAC' by replacing them with 'PORT'.
 *	-Completed counting of OID_SKGE_RX_HW_ERROR_CTS, OID_SKGE_TX_HW_ERROR_CTS,
 *	 OID_SKGE_IN_ERRORS_CTS, and OID_SKGE_OUT_ERROR_CTS.
 *	-SET check for RlmtMode
 *
 *	Revision 1.13  1998/09/28 13:13:08  mhaveman
 *	Hide strcmp, strlen, and strncpy behind macros SK_STRCMP, SK_STRLEN,
 *	and SK_STRNCPY. (Same reasons as for mem.. and MEM..)
 *
 *	Revision 1.12  1998/09/16 08:18:36  cgoos
 *	Fix: XM_INxx and XM_OUTxx called with different parameter order
 *	Fix: inserted "Pnmi." into some pAC->pDriverDescription / Version.
 *	Change: memset, memcpy to makros SK_MEMSET, SK_MEMCPY
 *
 *	Revision 1.11  1998/09/04 17:01:45  mhaveman
 *	Added SyncCounter as macro and OID_SKGE_.._NO_DESCR_CTS to
 *	OID_SKGE_RX_NO_BUF_CTS.
 *
 *	Revision 1.10  1998/09/04 14:35:35  mhaveman
 *	Added macro counters, that are counted by driver.
 *
 ****************************************************************************/

#if defined(DEBUG) || ((!defined(LINT)) && (!defined(SK_SLIM)))
static const char SysKonnectFileId[] =
	"@(#) $Id: //Release/Yukon_1G/Shared/pnmi/V7/skgepnmi.c#7 $ (C) Marvell.";
#endif

#include "h/skdrv1st.h"
#include "h/sktypes.h"
#include "h/xmac_ii.h"
#include "h/skdebug.h"
#include "h/skqueue.h"
#include "h/skgepnmi.h"
#include "h/skgesirq.h"
#include "h/skcsum.h"
#include "h/skvpd.h"
#include "h/skgehw.h"
#include "h/sky2le.h"
#include "h/skgeinit.h"
#include "h/skdrv2nd.h"
#include "h/skgepnm2.h"
#ifdef SK_POWER_MGMT
#include "h/skgepmgt.h"
#endif /* SK_POWER_MGMT */

/* defines *******************************************************************/

#ifndef DEBUG
#define PNMI_STATIC	static
#else	/* DEBUG */
#define PNMI_STATIC
#endif /* DEBUG */

/*
 * Private Function prototypes
 */

PNMI_STATIC SK_U8 CalculateLinkModeStatus(SK_AC *pAC, SK_IOC IoC, unsigned int
	PhysPortIndex);
PNMI_STATIC SK_U8 CalculateLinkStatus(SK_AC *pAC, SK_IOC IoC, unsigned int
	PhysPortIndex);
PNMI_STATIC void CopyMac(char *pDst, SK_MAC_ADDR *pMac);
PNMI_STATIC void CopyTrapQueue(SK_AC *pAC, char *pDstBuf);
PNMI_STATIC SK_U64 GetPhysStatVal(SK_AC *pAC, SK_IOC IoC,
	unsigned int PhysPortIndex, unsigned int StatIndex);
PNMI_STATIC SK_U64 GetStatVal(SK_AC *pAC, SK_IOC IoC, unsigned int LogPortIndex,
	unsigned int StatIndex, SK_U32 NetIndex);
PNMI_STATIC char* GetTrapEntry(SK_AC *pAC, SK_U32 TrapId, unsigned int Size);
PNMI_STATIC void GetTrapQueueLen(SK_AC *pAC, unsigned int *pLen,
	unsigned int *pEntries);
PNMI_STATIC int GetVpdKeyArr(SK_AC *pAC, SK_IOC IoC, char *pKeyArr,
	unsigned int KeyArrLen, unsigned int *pKeyNo);
PNMI_STATIC int LookupId(SK_U32 Id);
PNMI_STATIC int MacUpdate(SK_AC *pAC, SK_IOC IoC, unsigned int FirstMac,
	unsigned int LastMac);
PNMI_STATIC int PnmiStruct(SK_AC *pAC, SK_IOC IoC, int Action, char *pBuf,
	unsigned int *pLen, SK_U32 NetIndex);
PNMI_STATIC int PnmiVar(SK_AC *pAC, SK_IOC IoC, int Action, SK_U32 Id,
	char *pBuf, unsigned int *pLen, SK_U32 Instance, SK_U32 NetIndex);
PNMI_STATIC void QueueRlmtNewMacTrap(SK_AC *pAC, unsigned int ActiveMac);
PNMI_STATIC void QueueRlmtPortTrap(SK_AC *pAC, SK_U32 TrapId,
	unsigned int PortIndex);
PNMI_STATIC void QueueSensorTrap(SK_AC *pAC, SK_U32 TrapId,
	unsigned int SensorIndex);
PNMI_STATIC void QueueSimpleTrap(SK_AC *pAC, SK_U32 TrapId);
PNMI_STATIC void ResetCounter(SK_AC *pAC, SK_IOC IoC, SK_U32 NetIndex);
#ifndef SK_NO_RLMT
PNMI_STATIC int RlmtUpdate(SK_AC *pAC, SK_IOC IoC, SK_U32 NetIndex);
#endif
PNMI_STATIC int SirqUpdate(SK_AC *pAC, SK_IOC IoC);
PNMI_STATIC void VirtualConf(SK_AC *pAC, SK_IOC IoC, SK_U32 Id, char *pBuf);
PNMI_STATIC void CheckVctStatus(SK_AC *, SK_IOC, char *, SK_U32, SK_U32);
PNMI_STATIC void VctGetResults(SK_AC *, SK_IOC, SK_U32);

/*
 * Table to correlate OID with handler function and index to
 * hardware register stored in StatAddress if applicable.
 */
#include "skgemib.c"

/* global variables **********************************************************/

/*
 * Overflow status register bit table and corresponding counter
 * dependent on MAC type - the number relates to the size of overflow
 * mask returned by the pFnMacOverflow function
 */
PNMI_STATIC const SK_U16 StatOvrflwBit[][SK_PNMI_MAC_TYPES] = {
/* Bit0  */	{ SK_PNMI_HTX, 				SK_PNMI_HTX_UNICAST},
/* Bit1  */	{ SK_PNMI_HTX_OCTETHIGH, 	SK_PNMI_HTX_BROADCAST},
/* Bit2  */	{ SK_PNMI_HTX_OCTETLOW, 	SK_PNMI_HTX_PMACC},
/* Bit3  */	{ SK_PNMI_HTX_BROADCAST, 	SK_PNMI_HTX_MULTICAST},
/* Bit4  */	{ SK_PNMI_HTX_MULTICAST, 	SK_PNMI_HTX_OCTETLOW},
/* Bit5  */	{ SK_PNMI_HTX_UNICAST, 		SK_PNMI_HTX_OCTETHIGH},
/* Bit6  */	{ SK_PNMI_HTX_LONGFRAMES, 	SK_PNMI_HTX_64},
/* Bit7  */	{ SK_PNMI_HTX_BURST, 		SK_PNMI_HTX_127},
/* Bit8  */	{ SK_PNMI_HTX_PMACC, 		SK_PNMI_HTX_255},
/* Bit9  */	{ SK_PNMI_HTX_MACC, 		SK_PNMI_HTX_511},
/* Bit10 */	{ SK_PNMI_HTX_SINGLE_COL, 	SK_PNMI_HTX_1023},
/* Bit11 */	{ SK_PNMI_HTX_MULTI_COL, 	SK_PNMI_HTX_MAX},
/* Bit12 */	{ SK_PNMI_HTX_EXCESS_COL, 	SK_PNMI_HTX_LONGFRAMES},
/* Bit13 */	{ SK_PNMI_HTX_LATE_COL, 	SK_PNMI_HTX_RESERVED},
/* Bit14 */	{ SK_PNMI_HTX_DEFFERAL, 	SK_PNMI_HTX_COL},
/* Bit15 */	{ SK_PNMI_HTX_EXCESS_DEF, 	SK_PNMI_HTX_LATE_COL},
/* Bit16 */	{ SK_PNMI_HTX_UNDERRUN, 	SK_PNMI_HTX_EXCESS_COL},
/* Bit17 */	{ SK_PNMI_HTX_CARRIER, 		SK_PNMI_HTX_MULTI_COL},
/* Bit18 */	{ SK_PNMI_HTX_UTILUNDER, 	SK_PNMI_HTX_SINGLE_COL},
/* Bit19 */	{ SK_PNMI_HTX_UTILOVER, 	SK_PNMI_HTX_UNDERRUN},
/* Bit20 */	{ SK_PNMI_HTX_64, 			SK_PNMI_HTX_RESERVED},
/* Bit21 */	{ SK_PNMI_HTX_127, 			SK_PNMI_HTX_RESERVED},
/* Bit22 */	{ SK_PNMI_HTX_255, 			SK_PNMI_HTX_RESERVED},
/* Bit23 */	{ SK_PNMI_HTX_511, 			SK_PNMI_HTX_RESERVED},
/* Bit24 */	{ SK_PNMI_HTX_1023, 		SK_PNMI_HTX_RESERVED},
/* Bit25 */	{ SK_PNMI_HTX_MAX, 			SK_PNMI_HTX_RESERVED},
/* Bit26 */	{ SK_PNMI_HTX_RESERVED, 	SK_PNMI_HTX_RESERVED},
/* Bit27 */	{ SK_PNMI_HTX_RESERVED, 	SK_PNMI_HTX_RESERVED},
/* Bit28 */	{ SK_PNMI_HTX_RESERVED, 	SK_PNMI_HTX_RESERVED},
/* Bit29 */	{ SK_PNMI_HTX_RESERVED, 	SK_PNMI_HTX_RESERVED},
/* Bit30 */	{ SK_PNMI_HTX_RESERVED, 	SK_PNMI_HTX_RESERVED},
/* Bit31 */	{ SK_PNMI_HTX_RESERVED, 	SK_PNMI_HTX_RESERVED},
/* Bit32 */	{ SK_PNMI_HRX, 				SK_PNMI_HRX_UNICAST},
/* Bit33 */	{ SK_PNMI_HRX_OCTETHIGH, 	SK_PNMI_HRX_BROADCAST},
/* Bit34 */	{ SK_PNMI_HRX_OCTETLOW, 	SK_PNMI_HRX_PMACC},
/* Bit35 */	{ SK_PNMI_HRX_BROADCAST, 	SK_PNMI_HRX_MULTICAST},
/* Bit36 */	{ SK_PNMI_HRX_MULTICAST, 	SK_PNMI_HRX_FCS},
/* Bit37 */	{ SK_PNMI_HRX_UNICAST, 		SK_PNMI_HRX_RESERVED},
/* Bit38 */	{ SK_PNMI_HRX_PMACC, 		SK_PNMI_HRX_OCTETLOW},
/* Bit39 */	{ SK_PNMI_HRX_MACC, 		SK_PNMI_HRX_OCTETHIGH},
/* Bit40 */	{ SK_PNMI_HRX_PMACC_ERR, 	SK_PNMI_HRX_BADOCTETLOW},
/* Bit41 */	{ SK_PNMI_HRX_MACC_UNKWN,	SK_PNMI_HRX_BADOCTETHIGH},
/* Bit42 */	{ SK_PNMI_HRX_BURST, 		SK_PNMI_HRX_UNDERSIZE},
/* Bit43 */	{ SK_PNMI_HRX_MISSED, 		SK_PNMI_HRX_RUNT},
/* Bit44 */	{ SK_PNMI_HRX_FRAMING, 		SK_PNMI_HRX_64},
/* Bit45 */	{ SK_PNMI_HRX_OVERFLOW, 	SK_PNMI_HRX_127},
/* Bit46 */	{ SK_PNMI_HRX_JABBER, 		SK_PNMI_HRX_255},
/* Bit47 */	{ SK_PNMI_HRX_CARRIER, 		SK_PNMI_HRX_511},
/* Bit48 */	{ SK_PNMI_HRX_IRLENGTH, 	SK_PNMI_HRX_1023},
/* Bit49 */	{ SK_PNMI_HRX_SYMBOL, 		SK_PNMI_HRX_MAX},
/* Bit50 */	{ SK_PNMI_HRX_SHORTS, 		SK_PNMI_HRX_LONGFRAMES},
/* Bit51 */	{ SK_PNMI_HRX_RUNT, 		SK_PNMI_HRX_TOO_LONG},
/* Bit52 */	{ SK_PNMI_HRX_TOO_LONG, 	SK_PNMI_HRX_JABBER},
/* Bit53 */	{ SK_PNMI_HRX_FCS, 			SK_PNMI_HRX_RESERVED},
/* Bit54 */	{ SK_PNMI_HRX_RESERVED, 	SK_PNMI_HRX_OVERFLOW},
/* Bit55 */	{ SK_PNMI_HRX_CEXT, 		SK_PNMI_HRX_RESERVED},
/* Bit56 */	{ SK_PNMI_HRX_UTILUNDER, 	SK_PNMI_HRX_RESERVED},
/* Bit57 */	{ SK_PNMI_HRX_UTILOVER, 	SK_PNMI_HRX_RESERVED},
/* Bit58 */	{ SK_PNMI_HRX_64, 			SK_PNMI_HRX_RESERVED},
/* Bit59 */	{ SK_PNMI_HRX_127, 			SK_PNMI_HRX_RESERVED},
/* Bit60 */	{ SK_PNMI_HRX_255, 			SK_PNMI_HRX_RESERVED},
/* Bit61 */	{ SK_PNMI_HRX_511, 			SK_PNMI_HRX_RESERVED},
/* Bit62 */	{ SK_PNMI_HRX_1023, 		SK_PNMI_HRX_RESERVED},
/* Bit63 */	{ SK_PNMI_HRX_MAX, 			SK_PNMI_HRX_RESERVED}
};

/*
 * Table for hardware register saving on resets and port switches
 */
PNMI_STATIC const SK_PNMI_STATADDR StatAddr[SK_PNMI_MAX_IDX][SK_PNMI_MAC_TYPES] = {
	/* SK_PNMI_HTX */
	{{XM_TXF_OK, SK_TRUE}, {0, SK_FALSE}},
	/* SK_PNMI_HTX_OCTETHIGH */
	{{XM_TXO_OK_HI, SK_TRUE}, {GM_TXO_OK_HI, SK_TRUE}},
	/* SK_PNMI_HTX_OCTETLOW */
	{{XM_TXO_OK_LO, SK_FALSE}, {GM_TXO_OK_LO, SK_FALSE}},
	/* SK_PNMI_HTX_BROADCAST */
	{{XM_TXF_BC_OK, SK_TRUE}, {GM_TXF_BC_OK, SK_TRUE}},
	/* SK_PNMI_HTX_MULTICAST */
	{{XM_TXF_MC_OK, SK_TRUE}, {GM_TXF_MC_OK, SK_TRUE}},
	/* SK_PNMI_HTX_UNICAST */
	{{XM_TXF_UC_OK, SK_TRUE}, {GM_TXF_UC_OK, SK_TRUE}},
	/* SK_PNMI_HTX_BURST */
	{{XM_TXE_BURST, SK_TRUE}, {0, SK_FALSE}},
	/* SK_PNMI_HTX_PMACC */
	{{XM_TXF_MPAUSE, SK_TRUE}, {GM_TXF_MPAUSE, SK_TRUE}},
	/* SK_PNMI_HTX_MACC */
	{{XM_TXF_MCTRL, SK_TRUE}, {0, SK_FALSE}},
	/* SK_PNMI_HTX_COL */
	{{0, SK_FALSE}, {GM_TXF_COL, SK_TRUE}},
	/* SK_PNMI_HTX_SINGLE_COL */
	{{XM_TXF_SNG_COL, SK_TRUE}, {GM_TXF_SNG_COL, SK_TRUE}},
	/* SK_PNMI_HTX_MULTI_COL */
	{{XM_TXF_MUL_COL, SK_TRUE}, {GM_TXF_MUL_COL, SK_TRUE}},
	/* SK_PNMI_HTX_EXCESS_COL */
	{{XM_TXF_ABO_COL, SK_TRUE}, {GM_TXF_ABO_COL, SK_TRUE}},
	/* SK_PNMI_HTX_LATE_COL */
	{{XM_TXF_LAT_COL, SK_TRUE}, {GM_TXF_LAT_COL, SK_TRUE}},
	/* SK_PNMI_HTX_DEFFERAL */
	{{XM_TXF_DEF, SK_TRUE}, {0, SK_FALSE}},
	/* SK_PNMI_HTX_EXCESS_DEF */
	{{XM_TXF_EX_DEF, SK_TRUE}, {0, SK_FALSE}},
	/* SK_PNMI_HTX_UNDERRUN */
	{{XM_TXE_FIFO_UR, SK_TRUE}, {GM_TXE_FIFO_UR, SK_TRUE}},
	/* SK_PNMI_HTX_CARRIER */
	{{XM_TXE_CS_ERR, SK_TRUE}, {0, SK_FALSE}},
	/* SK_PNMI_HTX_UTILUNDER */
	{{0, SK_FALSE}, {0, SK_FALSE}},
	/* SK_PNMI_HTX_UTILOVER */
	{{0, SK_FALSE}, {0, SK_FALSE}},
	/* SK_PNMI_HTX_64 */
	{{XM_TXF_64B, SK_TRUE}, {GM_TXF_64B, SK_TRUE}},
	/* SK_PNMI_HTX_127 */
	{{XM_TXF_127B, SK_TRUE}, {GM_TXF_127B, SK_TRUE}},
	/* SK_PNMI_HTX_255 */
	{{XM_TXF_255B, SK_TRUE}, {GM_TXF_255B, SK_TRUE}},
	/* SK_PNMI_HTX_511 */
	{{XM_TXF_511B, SK_TRUE}, {GM_TXF_511B, SK_TRUE}},
	/* SK_PNMI_HTX_1023 */
	{{XM_TXF_1023B, SK_TRUE}, {GM_TXF_1023B, SK_TRUE}},
	/* SK_PNMI_HTX_MAX */
	{{XM_TXF_MAX_SZ, SK_TRUE}, {GM_TXF_1518B, SK_TRUE}},
	/* SK_PNMI_HTX_LONGFRAMES  */
	{{XM_TXF_LONG, SK_TRUE}, {GM_TXF_MAX_SZ, SK_TRUE}},
	/* SK_PNMI_HTX_SYNC */
	{{0, SK_FALSE}, {0, SK_FALSE}},
	/* SK_PNMI_HTX_SYNC_OCTET */
	{{0, SK_FALSE}, {0, SK_FALSE}},
	/* SK_PNMI_HTX_RESERVED */
	{{0, SK_FALSE}, {0, SK_FALSE}},
	/* SK_PNMI_HRX */
	{{XM_RXF_OK, SK_TRUE}, {0, SK_FALSE}},
	/* SK_PNMI_HRX_OCTETHIGH */
	{{XM_RXO_OK_HI, SK_TRUE}, {GM_RXO_OK_HI, SK_TRUE}},
	/* SK_PNMI_HRX_OCTETLOW */
	{{XM_RXO_OK_LO, SK_FALSE}, {GM_RXO_OK_LO, SK_FALSE}},
	/* SK_PNMI_HRX_BADOCTETHIGH */
	{{0, SK_FALSE}, {GM_RXO_ERR_HI, SK_TRUE}},
	/* SK_PNMI_HRX_BADOCTETLOW */
	{{0, SK_FALSE}, {GM_RXO_ERR_LO, SK_TRUE}},
	/* SK_PNMI_HRX_BROADCAST */
	{{XM_RXF_BC_OK, SK_TRUE}, {GM_RXF_BC_OK, SK_TRUE}},
	/* SK_PNMI_HRX_MULTICAST */
	{{XM_RXF_MC_OK, SK_TRUE}, {GM_RXF_MC_OK, SK_TRUE}},
	/* SK_PNMI_HRX_UNICAST */
	{{XM_RXF_UC_OK, SK_TRUE}, {GM_RXF_UC_OK, SK_TRUE}},
	/* SK_PNMI_HRX_PMACC */
	{{XM_RXF_MPAUSE, SK_TRUE}, {GM_RXF_MPAUSE, SK_TRUE}},
	/* SK_PNMI_HRX_MACC */
	{{XM_RXF_MCTRL, SK_TRUE}, {0, SK_FALSE}},
	/* SK_PNMI_HRX_PMACC_ERR */
	{{XM_RXF_INV_MP, SK_TRUE}, {0, SK_FALSE}},
	/* SK_PNMI_HRX_MACC_UNKWN */
	{{XM_RXF_INV_MOC, SK_TRUE}, {0, SK_FALSE}},
	/* SK_PNMI_HRX_BURST */
	{{XM_RXE_BURST, SK_TRUE}, {0, SK_FALSE}},
	/* SK_PNMI_HRX_MISSED */
	{{XM_RXE_FMISS, SK_TRUE}, {0, SK_FALSE}},
	/* SK_PNMI_HRX_FRAMING */
	{{XM_RXF_FRA_ERR, SK_TRUE}, {0, SK_FALSE}},
	/* SK_PNMI_HRX_UNDERSIZE */
	{{0, SK_FALSE}, {GM_RXF_SHT, SK_TRUE}},
	/* SK_PNMI_HRX_OVERFLOW */
	{{XM_RXE_FIFO_OV, SK_TRUE}, {GM_RXE_FIFO_OV, SK_TRUE}},
	/* SK_PNMI_HRX_JABBER */
	{{XM_RXF_JAB_PKT, SK_TRUE}, {GM_RXF_JAB_PKT, SK_TRUE}},
	/* SK_PNMI_HRX_CARRIER */
	{{XM_RXE_CAR_ERR, SK_TRUE}, {0, SK_FALSE}},
	/* SK_PNMI_HRX_IRLENGTH */
	{{XM_RXF_LEN_ERR, SK_TRUE}, {0, SK_FALSE}},
	/* SK_PNMI_HRX_SYMBOL */
	{{XM_RXE_SYM_ERR, SK_TRUE}, {0, SK_FALSE}},
	/* SK_PNMI_HRX_SHORTS */
	{{XM_RXE_SHT_ERR, SK_TRUE}, {0, SK_FALSE}},
	/* SK_PNMI_HRX_RUNT */
	{{XM_RXE_RUNT, SK_TRUE}, {GM_RXE_FRAG, SK_TRUE}},
	/* SK_PNMI_HRX_TOO_LONG */
	{{XM_RXF_LNG_ERR, SK_TRUE}, {GM_RXF_LNG_ERR, SK_TRUE}},
	/* SK_PNMI_HRX_FCS */
	{{XM_RXF_FCS_ERR, SK_TRUE}, {GM_RXF_FCS_ERR, SK_TRUE}},
	/* SK_PNMI_HRX_CEXT */
	{{XM_RXF_CEX_ERR, SK_TRUE}, {0, SK_FALSE}},
	/* SK_PNMI_HRX_UTILUNDER */
	{{0, SK_FALSE}, {0, SK_FALSE}},
	/* SK_PNMI_HRX_UTILOVER */
	{{0, SK_FALSE}, {0, SK_FALSE}},
	/* SK_PNMI_HRX_64 */
	{{XM_RXF_64B, SK_TRUE}, {GM_RXF_64B, SK_TRUE}},
	/* SK_PNMI_HRX_127 */
	{{XM_RXF_127B, SK_TRUE}, {GM_RXF_127B, SK_TRUE}},
	/* SK_PNMI_HRX_255 */
	{{XM_RXF_255B, SK_TRUE}, {GM_RXF_255B, SK_TRUE}},
	/* SK_PNMI_HRX_511 */
	{{XM_RXF_511B, SK_TRUE}, {GM_RXF_511B, SK_TRUE}},
	/* SK_PNMI_HRX_1023 */
	{{XM_RXF_1023B, SK_TRUE}, {GM_RXF_1023B, SK_TRUE}},
	/* SK_PNMI_HRX_MAX */
	{{XM_RXF_MAX_SZ, SK_TRUE}, {GM_RXF_1518B, SK_TRUE}},
	/* SK_PNMI_HRX_LONGFRAMES */
	{{0, SK_FALSE}, {GM_RXF_MAX_SZ, SK_TRUE}},
	/* SK_PNMI_HRX_RESERVED */
	{{0, SK_FALSE}, {0, SK_FALSE}}
};


/*****************************************************************************
 *
 * Public functions
 *
 */

/*****************************************************************************
 *
 * SkPnmiInit - Init function of PNMI
 *
 * Description:
 *	SK_INIT_DATA: Initializes the data structures
 *	SK_INIT_IO:   Resets the XMAC statistics, determines the device and
 *	              connector type.
 *	SK_INIT_RUN:  Starts a timer event for port switch per hour
 *	              calculation.
 *
 * Returns:
 *	Always 0
 */
int SkPnmiInit(
SK_AC	*pAC,		/* Pointer to adapter context */
SK_IOC	IoC,		/* IO context handle */
int		Level)		/* Initialization level */
{
	unsigned int	PortMax;	/* Number of ports */
	unsigned int	PortIndex;	/* Current port index in loop */
#ifndef SK_NO_RLMT
	SK_EVPARA		EventParam;	/* Event struct for timer event */
#endif

	SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
		("PNMI: SkPnmiInit: Called, level=%d\n", Level));

	PortMax = pAC->GIni.GIMacsFound;

	switch (Level) {

	case SK_INIT_DATA:
		SK_MEMSET((char *)&pAC->Pnmi, 0, sizeof(pAC->Pnmi));

		pAC->Pnmi.TrapBufFree = SK_PNMI_TRAP_QUEUE_LEN;
		pAC->Pnmi.StartUpTime = SK_PNMI_HUNDREDS_SEC(SkOsGetTime(pAC));
		pAC->Pnmi.RlmtChangeThreshold = SK_PNMI_DEF_RLMT_CHG_THRES;

		for (PortIndex = 0; PortIndex < SK_MAX_MACS; PortIndex ++) {

			pAC->Pnmi.Port[PortIndex].ActiveFlag = SK_FALSE;
			pAC->Pnmi.DualNetActiveFlag = SK_FALSE;

			/* Initialize DSP variables for Vct() to 0xff => Never written! */
			pAC->GIni.GP[PortIndex].PCableLen = 0xff;
			pAC->Pnmi.VctBackup[PortIndex].CableLen = 0xff;
		}

#ifdef SK_PNMI_CHECK
		if (SK_PNMI_MAX_IDX != SK_PNMI_CNT_NO) {

			SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR049, SK_PNMI_ERR049MSG);

			SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_INIT | SK_DBGCAT_FATAL,
					   ("CounterOffset struct size (%d) differs from"
						"SK_PNMI_MAX_IDX (%d)\n",
						SK_PNMI_CNT_NO, SK_PNMI_MAX_IDX));
		}

		if (SK_PNMI_MAX_IDX !=
			(sizeof(StatAddr) / (sizeof(SK_PNMI_STATADDR) * SK_PNMI_MAC_TYPES))) {

			SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR050, SK_PNMI_ERR050MSG);

			SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_INIT | SK_DBGCAT_FATAL,
					   ("StatAddr table size (%d) differs from "
						"SK_PNMI_MAX_IDX (%d)\n",
						(sizeof(StatAddr) /
						 (sizeof(SK_PNMI_STATADDR) * SK_PNMI_MAC_TYPES)),
						 SK_PNMI_MAX_IDX));
		}
#endif /* SK_PNMI_CHECK */
		break;

	case SK_INIT_IO:

		/* Reset MAC counters. */
		for (PortIndex = 0; PortIndex < PortMax; PortIndex ++) {

			pAC->GIni.GIFunc.pFnMacResetCounter(pAC, IoC, PortIndex);
		}

		/* Get PCI bus speed. */
		if (pAC->GIni.GIPciClock66) {

			pAC->Pnmi.PciBusSpeed = 66;
		}
		else {
			pAC->Pnmi.PciBusSpeed = 33;
		}

		/* Get PCI bus width. */
		if (pAC->GIni.GIPciSlot64) {

			pAC->Pnmi.PciBusWidth = 64;
		}
		else {
			pAC->Pnmi.PciBusWidth = 32;
		}

		/* Get chipset. */
		switch (pAC->GIni.GIChipId) {

		case CHIP_ID_YUKON:
			pAC->Pnmi.Chipset = SK_PNMI_CHIPSET_YUKON;
			break;

		case CHIP_ID_YUKON_LITE:
			pAC->Pnmi.Chipset = SK_PNMI_CHIPSET_YUKON_LITE;
			break;

		case CHIP_ID_YUKON_LP:
			pAC->Pnmi.Chipset = SK_PNMI_CHIPSET_YUKON_LP;
			break;

		case CHIP_ID_YUKON_XL:
			pAC->Pnmi.Chipset = SK_PNMI_CHIPSET_YUKON_XL;
			break;

		case CHIP_ID_YUKON_EC:
			pAC->Pnmi.Chipset = SK_PNMI_CHIPSET_YUKON_EC;
			break;

		case CHIP_ID_YUKON_FE:
			pAC->Pnmi.Chipset = SK_PNMI_CHIPSET_YUKON_FE;
			break;

		case CHIP_ID_YUKON_FE_P:
			pAC->Pnmi.Chipset = SK_PNMI_CHIPSET_YUKON_FE_P;
			break;

		case CHIP_ID_YUKON_EC_U:
			pAC->Pnmi.Chipset = SK_PNMI_CHIPSET_YUKON_EC_U;
			break;

		case CHIP_ID_YUKON_EX:
			pAC->Pnmi.Chipset = SK_PNMI_CHIPSET_YUKON_EX;
			break;

		case CHIP_ID_YUKON_SUPR:
			pAC->Pnmi.Chipset = SK_PNMI_CHIPSET_YUKON_SUPR;
			break;

		case CHIP_ID_YUKON_UL_2:
			pAC->Pnmi.Chipset = SK_PNMI_CHIPSET_YUKON_UL_2;
			break;

		case CHIP_ID_YUKON_OPT:
			pAC->Pnmi.Chipset = SK_PNMI_CHIPSET_YUKON_OPT;
			break;

		case CHIP_ID_YUKON_PRM:
			pAC->Pnmi.Chipset = SK_PNMI_CHIPSET_YUKON_PRM;
			break;

		default:
			break;
		}

		/* Get PMD and Device Type. */
		switch (pAC->GIni.GIPmdTyp) {

		case 'S':
			pAC->Pnmi.PMD = 3;
			pAC->Pnmi.DeviceType = 0x00020001;
			break;

		case 'L':
			pAC->Pnmi.PMD = 2;
			pAC->Pnmi.DeviceType = 0x00020003;
			break;

		case 'C':
			pAC->Pnmi.PMD = 4;
			pAC->Pnmi.DeviceType = 0x00020005;
			break;

		case 'T':
			pAC->Pnmi.PMD = 5;
			pAC->Pnmi.DeviceType = 0x00020007;
			break;

		default :
			pAC->Pnmi.PMD = 1;
			pAC->Pnmi.DeviceType = 0;
			break;
		}

		if (PortMax > 1) {

			pAC->Pnmi.DeviceType++;
		}

		/* Get connector type. */
		switch (pAC->GIni.GIConTyp) {

		case 'C':
			pAC->Pnmi.Connector = 2;
			break;

		case 'D':
			pAC->Pnmi.Connector = 3;
			break;

		case 'F':
			pAC->Pnmi.Connector = 4;
			break;

		case 'J':
			pAC->Pnmi.Connector = 5;
			break;

		case 'V':
			pAC->Pnmi.Connector = 6;
			break;

		default:
			pAC->Pnmi.Connector = 1;
			break;
		}
		break;

	case SK_INIT_RUN:
#ifndef SK_NO_RLMT
		if (PortMax > 1) {
			/* Start timer for RLMT change counter */
			SK_MEMSET((char *)&EventParam, 0, sizeof(EventParam));

			SkTimerStart(pAC, IoC, &pAC->Pnmi.RlmtChangeEstimate.EstTimer,
				SK_PNMI_EVT_TIMER_CHECK, SKGE_PNMI, SK_PNMI_EVT_CHG_EST_TIMER,
				EventParam);
		}
#endif
		break;

	default:
		break; /* Nothing to do. */
	}

	return (0);
}

/*****************************************************************************
 *
 * SkPnmiGetVar - Retrieves the value of a single OID
 *
 * Description:
 *	Calls a general sub-function for all this stuff. If the instance
 *	-1 is passed, the values of all instances are returned in an
 *	array of values.
 *
 * Returns:
 *	SK_PNMI_ERR_OK           The request was successfully performed
 *	SK_PNMI_ERR_GENERAL      A general severe internal error occurred
 *	SK_PNMI_ERR_TOO_SHORT    The passed buffer is too short to take
 *	                         the data.
 *	SK_PNMI_ERR_UNKNOWN_OID  The requested OID is unknown
 *	SK_PNMI_ERR_UNKNOWN_INST The requested instance of the OID doesn't
 *                           exist (e.g. port instance 3 on a two port
 *	                         adapter.
 */
int SkPnmiGetVar(
SK_AC *pAC,		/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
SK_U32 Id,		/* Object ID that is to be processed */
void *pBuf,		/* Buffer to which the management data will be copied */
unsigned int *pLen,	/* On call: buffer length. On return: used buffer */
SK_U32 Instance,	/* Instance (1..n) that is to be queried or -1 */
SK_U32 NetIndex)	/* NetIndex (0..n), in single net mode always zero */
{
	SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
		("PNMI: SkPnmiGetVar, Id=0x%x, Len=%d, Buf=%d, Inst=%d, NetIdx=%d\n",
		 Id, *pLen, *(char *)pBuf, Instance, NetIndex));

	return (PnmiVar(pAC, IoC, SK_PNMI_GET, Id, (char *)pBuf, pLen,
		Instance, NetIndex));
}

/*****************************************************************************
 *
 * SkPnmiPreSetVar - Presets the value of a single OID
 *
 * Description:
 *	Calls a general sub-function for all this stuff. The preset does
 *	the same as a set, but returns just before finally setting the
 *	new value. This is usefull to check if a set might be successfull.
 *	If the instance -1 is passed, an array of values is supposed and
 *	all instances of the OID will be set.
 *
 * Returns:
 *	SK_PNMI_ERR_OK           The request was successfully performed.
 *	SK_PNMI_ERR_GENERAL      A general severe internal error occurred.
 *	SK_PNMI_ERR_TOO_SHORT    The passed buffer is too short to contain
 *	                         the correct data (e.g. a 32bit value is
 *	                         needed, but a 16 bit value was passed).
 *	SK_PNMI_ERR_BAD_VALUE    The passed value is not in the valid
 *	                         value range.
 *	SK_PNMI_ERR_READ_ONLY    The OID is read-only and cannot be set.
 *	SK_PNMI_ERR_UNKNOWN_OID  The requested OID is unknown.
 *	SK_PNMI_ERR_UNKNOWN_INST The requested instance of the OID doesn't
 *                           exist (e.g. port instance 3 on a two port
 *	                         adapter.
 */
int SkPnmiPreSetVar(
SK_AC *pAC,		/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
SK_U32 Id,		/* Object ID that is to be processed */
void *pBuf,		/* Buffer to which the management data will be copied */
unsigned int *pLen,	/* Total length of management data */
SK_U32 Instance,	/* Instance (1..n) that is to be set or -1 */
SK_U32 NetIndex)	/* NetIndex (0..n), in single net mode always zero */
{
	SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
		("PNMI: SkPnmiPreSetVar: Called, Id=0x%x, BufLen=%d, Instance=%d, NetIndex=%d\n",
			Id, *pLen, Instance, NetIndex));

	return (PnmiVar(pAC, IoC, SK_PNMI_PRESET, Id, (char *)pBuf, pLen,
		Instance, NetIndex));
}

/*****************************************************************************
 *
 * SkPnmiSetVar - Sets the value of a single OID
 *
 * Description:
 *	Calls a general sub-function for all this stuff. The preset does
 *	the same as a set, but returns just before finally setting the
 *	new value. This is usefull to check if a set might be successfull.
 *	If the instance -1 is passed, an array of values is supposed and
 *	all instances of the OID will be set.
 *
 * Returns:
 *	SK_PNMI_ERR_OK           The request was successfully performed.
 *	SK_PNMI_ERR_GENERAL      A general severe internal error occurred.
 *	SK_PNMI_ERR_TOO_SHORT    The passed buffer is too short to contain
 *	                         the correct data (e.g. a 32bit value is
 *	                         needed, but a 16 bit value was passed).
 *	SK_PNMI_ERR_BAD_VALUE    The passed value is not in the valid
 *	                         value range.
 *	SK_PNMI_ERR_READ_ONLY    The OID is read-only and cannot be set.
 *	SK_PNMI_ERR_UNKNOWN_OID  The requested OID is unknown.
 *	SK_PNMI_ERR_UNKNOWN_INST The requested instance of the OID doesn't
 *                           exist (e.g. port instance 3 on a two port
 *	                         adapter.
 */
int SkPnmiSetVar(
SK_AC *pAC,		/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
SK_U32 Id,		/* Object ID that is to be processed */
void *pBuf,		/* Buffer to which the management data will be copied */
unsigned int *pLen,	/* Total length of management data */
SK_U32 Instance,	/* Instance (1..n) that is to be set or -1 */
SK_U32 NetIndex)	/* NetIndex (0..n), in single net mode always zero */
{
	SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
		("PNMI: SkPnmiSetVar, Id=0x%x, Len=%d, Buf=%d, Inst=%d, NetIdx=%d\n",
		 Id, *pLen, *(char *)pBuf, Instance, NetIndex));

	return (PnmiVar(pAC, IoC, SK_PNMI_SET, Id, (char *)pBuf, pLen,
		Instance, NetIndex));
}

/*****************************************************************************
 *
 * SkPnmiGetStruct - Retrieves the management database in SK_PNMI_STRUCT_DATA
 *
 * Description:
 *	Runs through the IdTable, queries the single OIDs and stores the
 *	returned data into the management database structure
 *	SK_PNMI_STRUCT_DATA. The offset of the OID in the structure
 *	is stored in the IdTable. The return value of the function will also
 *	be stored in SK_PNMI_STRUCT_DATA if the passed buffer has the
 *	minimum size of SK_PNMI_MIN_STRUCT_SIZE.
 *
 * Returns:
 *	SK_PNMI_ERR_OK           The request was successfully performed
 *	SK_PNMI_ERR_GENERAL      A general severe internal error occurred
 *	SK_PNMI_ERR_TOO_SHORT    The passed buffer is too short to take
 *	                         the data.
 *	SK_PNMI_ERR_UNKNOWN_NET  The requested NetIndex doesn't exist
 */
int SkPnmiGetStruct(
SK_AC *pAC,		/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
void *pBuf,		/* Buffer to which the management data will be copied. */
unsigned int *pLen,	/* Length of buffer */
SK_U32 NetIndex)	/* NetIndex (0..n), in single net mode always zero */
{
	int		Ret;
	unsigned int	TableIndex;
	unsigned int	DstOffset;
	unsigned int	InstanceNo;
	unsigned int	InstanceCnt;
	SK_U32		Instance;
	unsigned int	TmpLen;
	char		KeyArr[SK_PNMI_VPD_ENTRIES][SK_PNMI_VPD_KEY_SIZE];

	SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
		("PNMI: SkPnmiGetStruct: Called, BufLen=%d, NetIndex=%d\n",
			*pLen, NetIndex));

	if (*pLen < SK_PNMI_STRUCT_SIZE) {

		if (*pLen >= SK_PNMI_MIN_STRUCT_SIZE) {

			SK_PNMI_SET_STAT(pBuf, SK_PNMI_ERR_TOO_SHORT, (SK_U32)(-1));
		}

		*pLen = SK_PNMI_STRUCT_SIZE;
		return (SK_PNMI_ERR_TOO_SHORT);
	}
#ifndef SK_NO_RLMT
	/* Check NetIndex. */
	if (NetIndex >= pAC->Rlmt.NumNets) {
		return (SK_PNMI_ERR_UNKNOWN_NET);
	}
#else
	if (NetIndex >= pAC->GIni.GIMacsFound) {
		return (SK_PNMI_ERR_UNKNOWN_NET);
	}
#endif

	/* Update statistics. */
	SK_PNMI_CHECKFLAGS("SkPnmiGetStruct: On call");

	if ((Ret = MacUpdate(pAC, IoC, 0, pAC->GIni.GIMacsFound - 1)) !=
		SK_PNMI_ERR_OK) {

		SK_PNMI_SET_STAT(pBuf, Ret, (SK_U32)(-1));
		*pLen = SK_PNMI_MIN_STRUCT_SIZE;
		return (Ret);
	}
#ifndef SK_NO_RLMT
	if ((Ret = RlmtUpdate(pAC, IoC, NetIndex)) != SK_PNMI_ERR_OK) {

		SK_PNMI_SET_STAT(pBuf, Ret, (SK_U32)(-1));
		*pLen = SK_PNMI_MIN_STRUCT_SIZE;
		return (Ret);
	}
#endif
	if ((Ret = SirqUpdate(pAC, IoC)) != SK_PNMI_ERR_OK) {

		SK_PNMI_SET_STAT(pBuf, Ret, (SK_U32)(-1));
		*pLen = SK_PNMI_MIN_STRUCT_SIZE;
		return (Ret);
	}

	/* Increment semaphores to indicate that an update was already done. */
	pAC->Pnmi.MacUpdatedFlag ++;
	pAC->Pnmi.RlmtUpdatedFlag ++;
	pAC->Pnmi.SirqUpdatedFlag ++;

	/*
	 * Get VPD keys for instance calculation.
	 * Please read comment in Vpd().
	 */
	if (!pAC->Pnmi.VpdKeyReadError) {
		Ret = GetVpdKeyArr(pAC, IoC, &KeyArr[0][0], sizeof(KeyArr), &TmpLen);

		if (Ret != SK_PNMI_ERR_OK && !pAC->Pnmi.VpdKeyReadError) {

			pAC->Pnmi.MacUpdatedFlag --;
			pAC->Pnmi.RlmtUpdatedFlag --;
			pAC->Pnmi.SirqUpdatedFlag --;

			SK_PNMI_CHECKFLAGS("SkPnmiGetStruct: On return");
			SK_PNMI_SET_STAT(pBuf, Ret, (SK_U32)(-1));
			*pLen = SK_PNMI_MIN_STRUCT_SIZE;
			return (SK_PNMI_ERR_GENERAL);
		}
	}

	/* Retrieve values. */
	SK_MEMSET((char *)pBuf, 0, SK_PNMI_STRUCT_SIZE);

	for (TableIndex = 0; TableIndex < ID_TABLE_SIZE; TableIndex ++) {

		InstanceNo = IdTable[TableIndex].InstanceNo;
		for (InstanceCnt = 1; InstanceCnt <= InstanceNo; InstanceCnt ++) {

			DstOffset = IdTable[TableIndex].Offset +
				(InstanceCnt - 1) *
				IdTable[TableIndex].StructSize;

			/*
			 * For VPD the instance is not an index number but the key itself.
			 * Determine with the instance counter the VPD key to be used.
			 */
			if ((IdTable[TableIndex].Id == OID_SKGE_VPD_KEY ||
				 IdTable[TableIndex].Id == OID_SKGE_VPD_VALUE ||
				 IdTable[TableIndex].Id == OID_SKGE_VPD_ACCESS ||
				 IdTable[TableIndex].Id == OID_SKGE_VPD_ACTION) &&
				!pAC->Pnmi.VpdKeyReadError) {

				SK_STRNCPY((char *)&Instance, KeyArr[InstanceCnt - 1], 4);
			}
			else {
				Instance = (SK_U32)InstanceCnt;
			}

			TmpLen = *pLen - DstOffset;
			Ret = IdTable[TableIndex].Func(pAC, IoC, SK_PNMI_GET,
				IdTable[TableIndex].Id, (char *)pBuf +
				DstOffset, &TmpLen, Instance, TableIndex, NetIndex);

			/*
			 * An unknown instance error means that we reached
			 * the last instance of that variable. Proceed with
			 * the next OID in the table and ignore the return
			 * code.
			 */
			if (Ret == SK_PNMI_ERR_UNKNOWN_INST) {

				break;
			}

			if (Ret != SK_PNMI_ERR_OK) {

				pAC->Pnmi.MacUpdatedFlag --;
				pAC->Pnmi.RlmtUpdatedFlag --;
				pAC->Pnmi.SirqUpdatedFlag --;

				SK_PNMI_CHECKFLAGS("SkPnmiGetStruct: On return");
				SK_PNMI_SET_STAT(pBuf, Ret, DstOffset);
				*pLen = SK_PNMI_MIN_STRUCT_SIZE;
				return (Ret);
			}
		}
	}

	pAC->Pnmi.MacUpdatedFlag --;
	pAC->Pnmi.RlmtUpdatedFlag --;
	pAC->Pnmi.SirqUpdatedFlag --;

	*pLen = SK_PNMI_STRUCT_SIZE;
	SK_PNMI_CHECKFLAGS("SkPnmiGetStruct: On return");
	SK_PNMI_SET_STAT(pBuf, SK_PNMI_ERR_OK, (SK_U32)(-1));
	return (SK_PNMI_ERR_OK);
}

/*****************************************************************************
 *
 * SkPnmiPreSetStruct - Presets the management database in SK_PNMI_STRUCT_DATA
 *
 * Description:
 *	Calls a general sub-function for all this set stuff. The preset does
 *	the same as a set, but returns just before finally setting the
 *	new value. This is usefull to check if a set might be successfull.
 *	The sub-function runs through the IdTable, checks which OIDs are able
 *	to set, and calls the handler function of the OID to perform the
 *	preset. The return value of the function will also be stored in
 *	SK_PNMI_STRUCT_DATA if the passed buffer has the minimum size of
 *	SK_PNMI_MIN_STRUCT_SIZE.
 *
 * Returns:
 *	SK_PNMI_ERR_OK           The request was successfully performed.
 *	SK_PNMI_ERR_GENERAL      A general severe internal error occurred.
 *	SK_PNMI_ERR_TOO_SHORT    The passed buffer is too short to contain
 *	                         the correct data (e.g. a 32bit value is
 *	                         needed, but a 16 bit value was passed).
 *	SK_PNMI_ERR_BAD_VALUE    The passed value is not in the valid
 *	                         value range.
 */
int SkPnmiPreSetStruct(
SK_AC *pAC,		/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
void *pBuf,		/* Buffer which contains the data to be set */
unsigned int *pLen,	/* Length of buffer */
SK_U32 NetIndex)	/* NetIndex (0..n), in single net mode always zero */
{
	SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
		("PNMI: SkPnmiPreSetStruct: Called, BufLen=%d, NetIndex=%d\n",
			*pLen, NetIndex));

	return (PnmiStruct(pAC, IoC, SK_PNMI_PRESET, (char *)pBuf,
    					pLen, NetIndex));
}

/*****************************************************************************
 *
 * SkPnmiSetStruct - Sets the management database in SK_PNMI_STRUCT_DATA
 *
 * Description:
 *	Calls a general sub-function for all this set stuff. The return value
 *	of the function will also be stored in SK_PNMI_STRUCT_DATA if the
 *	passed buffer has the minimum size of SK_PNMI_MIN_STRUCT_SIZE.
 *	The sub-function runs through the IdTable, checks which OIDs are able
 *	to set, and calls the handler function of the OID to perform the
 *	set. The return value of the function will also be stored in
 *	SK_PNMI_STRUCT_DATA if the passed buffer has the minimum size of
 *	SK_PNMI_MIN_STRUCT_SIZE.
 *
 * Returns:
 *	SK_PNMI_ERR_OK           The request was successfully performed.
 *	SK_PNMI_ERR_GENERAL      A general severe internal error occurred.
 *	SK_PNMI_ERR_TOO_SHORT    The passed buffer is too short to contain
 *	                         the correct data (e.g. a 32bit value is
 *	                         needed, but a 16 bit value was passed).
 *	SK_PNMI_ERR_BAD_VALUE    The passed value is not in the valid
 *	                         value range.
 */
int SkPnmiSetStruct(
SK_AC *pAC,		/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
void *pBuf,		/* Buffer which contains the data to be set */
unsigned int *pLen,	/* Length of buffer */
SK_U32 NetIndex)	/* NetIndex (0..n), in single net mode always zero */
{
	SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
		("PNMI: SkPnmiSetStruct: Called, BufLen=%d, NetIndex=%d\n",
			*pLen, NetIndex));

	return (PnmiStruct(pAC, IoC, SK_PNMI_SET, (char *)pBuf,
    					pLen, NetIndex));
}

/*****************************************************************************
 *
 * SkPnmiEvent - Event handler
 *
 * Description:
 *	Handles the following events:
 *	SK_PNMI_EVT_SIRQ_OVERFLOW     When a hardware counter overflows an
 *	                              interrupt will be generated which is
 *	                              first handled by SIRQ which generates a
 *	                              this event. The event increments the
 *	                              upper 32 bit of the 64 bit counter.
 *	SK_PNMI_EVT_SEN_XXX           The event is generated by the I2C module
 *	                              when a sensor reports a warning or
 *	                              error. The event will store a trap
 *	                              message in the trap buffer.
 *	SK_PNMI_EVT_CHG_EST_TIMER     The timer event was initiated by this
 *	                              module and is used to calculate the
 *	                              port switches per hour.
 *	SK_PNMI_EVT_CLEAR_COUNTER     The event clears all counters and
 *	                              timestamps.
 *	SK_PNMI_EVT_XMAC_RESET        The event is generated by the driver
 *	                              before a hard reset of the XMAC is
 *	                              performed. All counters will be saved
 *	                              and added to the hardware counter
 *	                              values after reset to grant continuous
 *	                              counter values.
 *	SK_PNMI_EVT_RLMT_PORT_UP      Generated by RLMT to notify that a port
 *	                              went logically up. A trap message will
 *	                              be stored to the trap buffer.
 *	SK_PNMI_EVT_RLMT_PORT_DOWN    Generated by RLMT to notify that a port
 *	                              went logically down. A trap message will
 *	                              be stored to the trap buffer.
 *	SK_PNMI_EVT_RLMT_SEGMENTATION Generated by RLMT to notify that two
 *	                              spanning tree root bridges were
 *	                              detected. A trap message will be stored
 *	                              to the trap buffer.
 *	SK_PNMI_EVT_RLMT_ACTIVE_DOWN  Notifies PNMI that an active port went
 *	                              down. PNMI will not further add the
 *	                              statistic values to the virtual port.
 *	SK_PNMI_EVT_RLMT_ACTIVE_UP    Notifies PNMI that a port went up and
 *	                              is now an active port. PNMI will now
 *	                              add the statistic data of this port to
 *	                              the virtual port.
 *	SK_PNMI_EVT_RLMT_SET_NETS     Notifies PNMI about the net mode. The first parameter
 *	                              contains the number of nets. 1 means single net, 2 means
 *	                              dual net. The second parameter is -1
 *
 * Returns:
 *	Always 0
 */
int SkPnmiEvent(
SK_AC *pAC,		/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
SK_U32 Event,		/* Event-Id */
SK_EVPARA Param)	/* Event dependent parameter */
{
	unsigned int	PhysPortIndex;
	unsigned int	MaxNetNumber;
	int			CounterIndex;
	SK_U16		MacStatus;
	SK_U64		OverflowStatus;
	SK_U64		Mask;
	int			MacType;
	SK_U64		Value;
	SK_U32		Val32;
	SK_U16		Register;
	SK_EVPARA	EventParam;
#ifndef SK_NO_RLMT
	SK_U64		NewestValue;
	SK_U64		OldestValue;
	SK_U64		Delta;
	SK_PNMI_ESTIMATE *pEst;
#endif
	SK_U32		NetIndex;
	SK_U32		RetCode;

#ifdef DEBUG
	if (Event != SK_PNMI_EVT_XMAC_RESET) {

		SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
			("PNMI: SkPnmiEvent: Called, Event=0x%x, Param=0x%x\n",
			(unsigned int)Event, (unsigned int)Param.Para64));
	}
#endif /* DEBUG */
	SK_PNMI_CHECKFLAGS("SkPnmiEvent: On call");

	MacType = pAC->GIni.GIMacType;

	switch (Event) {

	case SK_PNMI_EVT_SIRQ_OVERFLOW:
		PhysPortIndex = (int)Param.Para32[0];
		MacStatus = (SK_U16)Param.Para32[1];
#ifdef DEBUG
		if (PhysPortIndex >= SK_MAX_MACS) {

			SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
				("PNMI: ERR: SkPnmiEvent: SK_PNMI_EVT_SIRQ_OVERFLOW parameter"
				 " wrong, PhysPortIndex=0x%x\n",
				PhysPortIndex));
			return (0);
		}
#endif /* DEBUG */
		OverflowStatus = 0;

		/* Check which source caused an overflow interrupt. */
		if ((pAC->GIni.GIFunc.pFnMacOverflow(pAC, IoC, PhysPortIndex,
				MacStatus, &OverflowStatus) != 0) ||
			(OverflowStatus == 0)) {

			SK_PNMI_CHECKFLAGS("SkPnmiEvent: On return");
			return (0);
		}

		/*
		 * Check the overflow status register and increment
		 * the upper dword of corresponding counter.
		 */
		for (CounterIndex = 0; CounterIndex < sizeof(Mask) * 8;
			CounterIndex ++) {

			Mask = (SK_U64)1 << CounterIndex;
			if ((OverflowStatus & Mask) == 0) {
				continue;
			}

			switch (StatOvrflwBit[CounterIndex][MacType]) {

			case SK_PNMI_HTX_UTILUNDER:
			case SK_PNMI_HTX_UTILOVER:
				if (MacType == SK_MAC_XMAC) {
					XM_IN16(IoC, PhysPortIndex, XM_TX_CMD, &Register);
					Register |= XM_TX_SAM_LINE;
					XM_OUT16(IoC, PhysPortIndex, XM_TX_CMD, Register);
				}
				break;

			case SK_PNMI_HRX_UTILUNDER:
			case SK_PNMI_HRX_UTILOVER:
				if (MacType == SK_MAC_XMAC) {
					XM_IN16(IoC, PhysPortIndex, XM_RX_CMD, &Register);
					Register |= XM_RX_SAM_LINE;
					XM_OUT16(IoC, PhysPortIndex, XM_RX_CMD, Register);
				}
				break;

			case SK_PNMI_HTX_OCTETHIGH:
			case SK_PNMI_HTX_OCTETLOW:
			case SK_PNMI_HTX_RESERVED:
			case SK_PNMI_HRX_OCTETHIGH:
			case SK_PNMI_HRX_OCTETLOW:
			case SK_PNMI_HRX_IRLENGTH:
			case SK_PNMI_HRX_RESERVED:

			/* The following counters aren't be handled (id > 63). */
			case SK_PNMI_HTX_SYNC:
			case SK_PNMI_HTX_SYNC_OCTET:
				break;

			case SK_PNMI_HRX_LONGFRAMES:
				if (MacType == SK_MAC_GMAC) {
					pAC->Pnmi.Port[PhysPortIndex].CounterHigh[CounterIndex] ++;
				}
				break;

			default:
				pAC->Pnmi.Port[PhysPortIndex].CounterHigh[CounterIndex] ++;
			}
		}
		break;

	case SK_PNMI_EVT_SEN_WAR_LOW:
#ifdef DEBUG
		if ((unsigned int)Param.Para64 >= (unsigned int)pAC->I2c.MaxSens) {

			SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
				("PNMI: ERR: SkPnmiEvent: SK_PNMI_EVT_SEN_WAR_LOW parameter wrong, SensorIndex=%d\n",
				(unsigned int)Param.Para64));
			return (0);
		}
#endif /* DEBUG */

		/*
		 * Store a trap message in the trap buffer and generate
		 * an event for user space applications with the
		 * SK_DRIVER_SENDEVENT macro.
		 */
		QueueSensorTrap(pAC, OID_SKGE_TRAP_SEN_WAR_LOW,
			(unsigned int)Param.Para64);
		(void)SK_DRIVER_SENDEVENT(pAC, IoC);
		break;

	case SK_PNMI_EVT_SEN_WAR_UPP:
#ifdef DEBUG
		if ((unsigned int)Param.Para64 >= (unsigned int)pAC->I2c.MaxSens) {

			SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
				("PNMI: ERR: SkPnmiEvent: SK_PNMI_EVT_SEN_WAR_UPP parameter wrong, SensorIndex=%d\n",
				(unsigned int)Param.Para64));
			return (0);
		}
#endif /* DEBUG */

		/*
		 * Store a trap message in the trap buffer and generate
		 * an event for user space applications with the
		 * SK_DRIVER_SENDEVENT macro.
		 */
		QueueSensorTrap(pAC, OID_SKGE_TRAP_SEN_WAR_UPP,
			(unsigned int)Param.Para64);
		(void)SK_DRIVER_SENDEVENT(pAC, IoC);
		break;

	case SK_PNMI_EVT_SEN_ERR_LOW:
#ifdef DEBUG
		if ((unsigned int)Param.Para64 >= (unsigned int)pAC->I2c.MaxSens) {

			SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
				("PNMI: ERR: SkPnmiEvent: SK_PNMI_EVT_SEN_ERR_LOW parameter wrong, SensorIndex=%d\n",
				(unsigned int)Param.Para64));
			return (0);
		}
#endif /* DEBUG */

		/*
		 * Store a trap message in the trap buffer and generate
		 * an event for user space applications with the
		 * SK_DRIVER_SENDEVENT macro.
		 */
		QueueSensorTrap(pAC, OID_SKGE_TRAP_SEN_ERR_LOW,
			(unsigned int)Param.Para64);
		(void)SK_DRIVER_SENDEVENT(pAC, IoC);
		break;

	case SK_PNMI_EVT_SEN_ERR_UPP:
#ifdef DEBUG
		if ((unsigned int)Param.Para64 >= (unsigned int)pAC->I2c.MaxSens) {

			SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
				("PNMI: ERR: SK_PNMI_EVT_SEN_ERR_UPP parameter wrong, SensorIndex=%d\n",
				(unsigned int)Param.Para64));
			return (0);
		}
#endif /* DEBUG */

		/*
		 * Store a trap message in the trap buffer and generate
		 * an event for user space applications with the
		 * SK_DRIVER_SENDEVENT macro.
		 */
		QueueSensorTrap(pAC, OID_SKGE_TRAP_SEN_ERR_UPP,
			(unsigned int)Param.Para64);
		(void)SK_DRIVER_SENDEVENT(pAC, IoC);
		break;

	case SK_PNMI_EVT_CHG_EST_TIMER:
#ifndef SK_NO_RLMT
		/*
		 * Calculate port switch average on a per hour basis
		 *   Time interval for check       : 28125 ms (SK_PNMI_EVT_TIMER_CHECK)
		 *   Number of values for average  : 8
		 *
		 * Be careful in changing these values, on change check
		 *   - typedef of SK_PNMI_ESTIMATE (Size of EstValue
		 *     array one less than value number)
		 *   - Timer initialization SkTimerStart() in SkPnmiInit
		 *   - Delta value below must be multiplicated with power of 2
		 */
		pEst = &pAC->Pnmi.RlmtChangeEstimate;
		CounterIndex = pEst->EstValueIndex + 1;
		if (CounterIndex == 7) {

			CounterIndex = 0;
		}
		pEst->EstValueIndex = CounterIndex;

		NewestValue = pAC->Pnmi.RlmtChangeCts;
		OldestValue = pEst->EstValue[CounterIndex];
		pEst->EstValue[CounterIndex] = NewestValue;

		/*
		 * Calculate average. Delta stores the number of
		 * port switches per 28125 * 8 = 225000 ms
		 */
		if (NewestValue >= OldestValue) {

			Delta = NewestValue - OldestValue;
		}
		else {
			/* Overflow situation. */
			Delta = (SK_U64)(0 - OldestValue) + NewestValue;
		}

		/*
		 * Extrapolate delta to port switches per hour.
		 *     Estimate = Delta * (3600000 / 225000)
		 *              = Delta * 16
		 *              = Delta << 4
		 */
		pAC->Pnmi.RlmtChangeEstimate.Estimate = Delta << 4;

		/*
		 * Check if threshold is exceeded. If the threshold is
		 * permanently exceeded every 28125 ms an event will be
		 * generated to remind the user of this condition.
		 */
		if ((pAC->Pnmi.RlmtChangeThreshold != 0) &&
			(pAC->Pnmi.RlmtChangeEstimate.Estimate >=
			pAC->Pnmi.RlmtChangeThreshold)) {

			QueueSimpleTrap(pAC, OID_SKGE_TRAP_RLMT_CHANGE_THRES);
			(void)SK_DRIVER_SENDEVENT(pAC, IoC);
		}

		SK_MEMSET((char *)&EventParam, 0, sizeof(EventParam));

		SkTimerStart(pAC, IoC, &pAC->Pnmi.RlmtChangeEstimate.EstTimer,
			SK_PNMI_EVT_TIMER_CHECK, SKGE_PNMI, SK_PNMI_EVT_CHG_EST_TIMER,
			EventParam);
#endif
		break;

	case SK_PNMI_EVT_CLEAR_COUNTER:
		/*
		 *  Param.Para32[0] contains the NetIndex (0 ..1).
		 *  Param.Para32[1] is reserved, contains -1.
		 */
		NetIndex = (SK_U32)Param.Para32[0];

#ifdef DEBUG
#ifndef SK_NO_RLMT
		if (NetIndex >= pAC->Rlmt.NumNets) {
#else
		if (NetIndex >= pAC->GIni.GIMacsFound) {
#endif
			SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
				("PNMI: ERR: SkPnmiEvent: SK_PNMI_EVT_CLEAR_COUNTER parameter wrong, NetIndex=%d\n",
				NetIndex));

			return (0);
		}
#endif /* DEBUG */

		/*
		 * Set all counters and timestamps to zero.
		 * The according NetIndex is required as a
		 * parameter of the event.
		 */
		ResetCounter(pAC, IoC, NetIndex);
		break;

	case SK_PNMI_EVT_XMAC_RESET:
		/*
		 * To grant continuous counter values store the current
		 * XMAC statistic values to the entries 1..n of the
		 * CounterOffset array. XMAC Errata #2
		 */
#ifdef DEBUG
		if ((unsigned int)Param.Para64 >= SK_MAX_MACS) {

			SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
				("PNMI: ERR: SkPnmiEvent: SK_PNMI_EVT_XMAC_RESET parameter wrong, PhysPortIndex=%d\n",
				(unsigned int)Param.Para64));
			return (0);
		}
#endif /* DEBUG */

		PhysPortIndex = (unsigned int)Param.Para64;

		/* Update XMAC statistic to get fresh values. */
		if (MacUpdate(pAC, IoC, 0, pAC->GIni.GIMacsFound - 1) !=
			SK_PNMI_ERR_OK) {

			SK_PNMI_CHECKFLAGS("SkPnmiEvent: On return");
			return (0);
		}

		/* Increment semaphore to indicate that an update was already done. */
		pAC->Pnmi.MacUpdatedFlag ++;

		for (CounterIndex = 0; CounterIndex < SK_PNMI_MAX_IDX;
			CounterIndex ++) {

			if (!StatAddr[CounterIndex][MacType].GetOffset) {
				continue;
			}

			pAC->Pnmi.Port[PhysPortIndex].CounterOffset[CounterIndex] =
				GetPhysStatVal(pAC, IoC, PhysPortIndex, CounterIndex);

			pAC->Pnmi.Port[PhysPortIndex].CounterHigh[CounterIndex] = 0;
		}

		pAC->Pnmi.MacUpdatedFlag --;
		break;

	case SK_PNMI_EVT_RLMT_PORT_UP:
		PhysPortIndex = (unsigned int)Param.Para32[0];
#ifdef DEBUG
		if (PhysPortIndex >= SK_MAX_MACS) {

			SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
				("PNMI: ERR: SkPnmiEvent: SK_PNMI_EVT_RLMT_PORT_UP parameter"
				 " wrong, PhysPortIndex=%d\n", PhysPortIndex));

			return (0);
		}
#endif /* DEBUG */

		/*
		 * Store a trap message in the trap buffer and generate an event for
		 * user space applications with the SK_DRIVER_SENDEVENT macro.
		 */
		QueueRlmtPortTrap(pAC, OID_SKGE_TRAP_RLMT_PORT_UP, PhysPortIndex);
		(void)SK_DRIVER_SENDEVENT(pAC, IoC);

		/* Bugfix for XMAC errata (#10620). */
		if (MacType == SK_MAC_XMAC) {
			/* Add incremental difference to offset (#10620). */
			(void)pAC->GIni.GIFunc.pFnMacStatistic(pAC, IoC, PhysPortIndex,
				XM_RXE_SHT_ERR, &Val32);

			Value = (((SK_U64)pAC->Pnmi.Port[PhysPortIndex].
				 CounterHigh[SK_PNMI_HRX_SHORTS] << 32) | (SK_U64)Val32);

			pAC->Pnmi.Port[PhysPortIndex].CounterOffset[SK_PNMI_HRX_SHORTS] +=
				Value - pAC->Pnmi.Port[PhysPortIndex].RxShortZeroMark;
		}

		/* Tell VctStatus() that a link was up meanwhile. */
		pAC->Pnmi.VctStatus[PhysPortIndex] |= SK_PNMI_VCT_LINK;
		break;

	case SK_PNMI_EVT_RLMT_PORT_DOWN:
		PhysPortIndex = (unsigned int)Param.Para32[0];

#ifdef DEBUG
		if (PhysPortIndex >= SK_MAX_MACS) {

			SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
				("PNMI: ERR: SkPnmiEvent: SK_PNMI_EVT_RLMT_PORT_DOWN parameter"
				 " wrong, PhysPortIndex=%d\n", PhysPortIndex));

			return (0);
		}
#endif /* DEBUG */

		/*
		 * Store a trap message in the trap buffer and generate an event for
		 * user space applications with the SK_DRIVER_SENDEVENT macro.
		 */
		QueueRlmtPortTrap(pAC, OID_SKGE_TRAP_RLMT_PORT_DOWN, PhysPortIndex);
		(void)SK_DRIVER_SENDEVENT(pAC, IoC);

		/* Bugfix #10620 - get zero level for incremental difference. */
		if (MacType == SK_MAC_XMAC) {

			(void)pAC->GIni.GIFunc.pFnMacStatistic(pAC, IoC, PhysPortIndex,
				XM_RXE_SHT_ERR, &Val32);

			pAC->Pnmi.Port[PhysPortIndex].RxShortZeroMark =
				(((SK_U64)pAC->Pnmi.Port[PhysPortIndex].
				 CounterHigh[SK_PNMI_HRX_SHORTS] << 32) | (SK_U64)Val32);
		}
		break;

	case SK_PNMI_EVT_RLMT_ACTIVE_DOWN:
		PhysPortIndex = (unsigned int)Param.Para32[0];
		NetIndex = (SK_U32)Param.Para32[1];

#ifdef DEBUG
		if (PhysPortIndex >= SK_MAX_MACS) {

			SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
				("PNMI: ERR: SkPnmiEvent: SK_PNMI_EVT_RLMT_ACTIVE_DOWN parameter too high, PhysPort=%d\n",
				PhysPortIndex));
		}
#ifndef SK_NO_RLMT
		if (NetIndex >= pAC->Rlmt.NumNets) {
#else
		if (NetIndex >= pAC->GIni.GIMacsFound) {
#endif
			SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
				("PNMI: ERR: SkPnmiEvent: SK_PNMI_EVT_RLMT_ACTIVE_DOWN parameter too high, NetIndex=%d\n",
				NetIndex));
		}
#endif /* DEBUG */

		/* For now, ignore event if NetIndex != 0. */
		if (Param.Para32[1] != 0) {

			return (0);
		}

		/* Nothing to do if port is already inactive. */
		if (!pAC->Pnmi.Port[PhysPortIndex].ActiveFlag) {

			return (0);
		}

		/*
		 * Update statistic counters to calculate new offset for the virtual
		 * port and increment semaphore to indicate that an update was already
		 * done.
		 */
		if (MacUpdate(pAC, IoC, 0, pAC->GIni.GIMacsFound - 1) !=
			SK_PNMI_ERR_OK) {

			SK_PNMI_CHECKFLAGS("SkPnmiEvent: On return");
			return (0);
		}
		pAC->Pnmi.MacUpdatedFlag ++;

		/*
		 * Calculate new counter offset for virtual port to grant continous
		 * counting on port switches. The virtual port consists of all currently
		 * active ports. The port down event indicates that a port is removed
		 * from the virtual port. Therefore add the counter value of the removed
		 * port to the CounterOffset for the virtual port to grant the same
		 * counter value.
		 */
		for (CounterIndex = 0; CounterIndex < SK_PNMI_MAX_IDX;
			CounterIndex ++) {

			if (!StatAddr[CounterIndex][MacType].GetOffset) {
				continue;
			}

			Value = GetPhysStatVal(pAC, IoC, PhysPortIndex, CounterIndex);

			pAC->Pnmi.VirtualCounterOffset[CounterIndex] += Value;
		}

		/* Set port to inactive. */
		pAC->Pnmi.Port[PhysPortIndex].ActiveFlag = SK_FALSE;

		pAC->Pnmi.MacUpdatedFlag --;
		break;

	case SK_PNMI_EVT_RLMT_ACTIVE_UP:
		PhysPortIndex = (unsigned int)Param.Para32[0];
		NetIndex = (SK_U32)Param.Para32[1];

#ifdef DEBUG
		if (PhysPortIndex >= SK_MAX_MACS) {

			SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
				("PNMI: ERR: SkPnmiEvent: SK_PNMI_EVT_RLMT_ACTIVE_UP parameter too high, PhysPort=%d\n",
				PhysPortIndex));
		}
#ifndef SK_NO_RLMT
		if (NetIndex >= pAC->Rlmt.NumNets) {
#else
		if (NetIndex >= pAC->GIni.GIMacsFound) {
#endif
			SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
				("PNMI: ERR: SkPnmiEvent: SK_PNMI_EVT_RLMT_ACTIVE_UP parameter too high, NetIndex=%d\n",
				NetIndex));
		}
#endif /* DEBUG */

		/* For now, ignore event if NetIndex != 0. */
		if (Param.Para32[1] != 0) {

			return (0);
		}

		/* Nothing to do if port is already inactive. */
		if (pAC->Pnmi.Port[PhysPortIndex].ActiveFlag) {

			return (0);
		}

		/* Statistic maintenance. */
		pAC->Pnmi.RlmtChangeCts ++;
		pAC->Pnmi.RlmtChangeTime = SK_PNMI_HUNDREDS_SEC(SkOsGetTime(pAC));

		/*
		 * Store a trap message in the trap buffer and generate an event for
		 * user space applications with the SK_DRIVER_SENDEVENT macro.
		 */
		QueueRlmtNewMacTrap(pAC, PhysPortIndex);
		(void)SK_DRIVER_SENDEVENT(pAC, IoC);

		/*
		 * Update statistic counters to calculate new offset for the virtual
		 * port and increment semaphore to indicate that an update was
		 * already done.
		 */
		if (MacUpdate(pAC, IoC, 0, pAC->GIni.GIMacsFound - 1) !=
			SK_PNMI_ERR_OK) {

			SK_PNMI_CHECKFLAGS("SkPnmiEvent: On return");
			return (0);
		}
		pAC->Pnmi.MacUpdatedFlag ++;

		/*
		 * Calculate new counter offset for virtual port to grant continous
		 * counting on port switches. A new port is added to the virtual port.
		 * Therefore substract the counter value of the new port from the
		 * CounterOffset for the virtual port to grant the same value.
		 */
		for (CounterIndex = 0; CounterIndex < SK_PNMI_MAX_IDX;
			CounterIndex ++) {

			if (!StatAddr[CounterIndex][MacType].GetOffset) {
				continue;
			}

			Value = GetPhysStatVal(pAC, IoC, PhysPortIndex, CounterIndex);

			pAC->Pnmi.VirtualCounterOffset[CounterIndex] -= Value;
		}

		/* Set port to active. */
		pAC->Pnmi.Port[PhysPortIndex].ActiveFlag = SK_TRUE;

		pAC->Pnmi.MacUpdatedFlag --;
		break;

	case SK_PNMI_EVT_RLMT_SEGMENTATION:
		/* Para.Para32[0] contains the NetIndex. */

		/*
		 * Store a trap message in the trap buffer and generate an event for
		 * user space applications with the SK_DRIVER_SENDEVENT macro.
		 */
		QueueSimpleTrap(pAC, OID_SKGE_TRAP_RLMT_SEGMENTATION);
		(void)SK_DRIVER_SENDEVENT(pAC, IoC);
		break;

	case SK_PNMI_EVT_RLMT_SET_NETS:
		/*
		 *  Param.Para32[0] contains the number of Nets.
		 *  Param.Para32[1] is reserved, contains -1.
		 */
		/* Check number of nets. */
		MaxNetNumber = pAC->GIni.GIMacsFound;

		if (((unsigned int)Param.Para32[0] < 1) ||
			((unsigned int)Param.Para32[0] > MaxNetNumber)) {

			return (SK_PNMI_ERR_UNKNOWN_NET);
		}

		if ((unsigned int)Param.Para32[0] == 1) { /* SingleNet mode */
			pAC->Pnmi.DualNetActiveFlag = SK_FALSE;
		}
		else { /* DualNet mode */
			pAC->Pnmi.DualNetActiveFlag = SK_TRUE;
		}
		break;

	case SK_PNMI_EVT_VCT_RESET:
		PhysPortIndex = Param.Para32[0];

		if (pAC->Pnmi.VctStatus[PhysPortIndex] & SK_PNMI_VCT_PENDING) {

			RetCode = SkGmCableDiagStatus(pAC, IoC, PhysPortIndex, SK_FALSE);

			if (RetCode == 2) {
				/*
				 * VCT test is still running.
				 * Start VCT timer counter again.
				 */
				SK_MEMSET((char *)&Param, 0, sizeof(Param));

				Param.Para32[0] = PhysPortIndex;
				Param.Para32[1] = -1;

				SkTimerStart(pAC, IoC, &pAC->Pnmi.VctTimeout[PhysPortIndex],
					SK_PNMI_VCT_TIMER_CHECK, SKGE_PNMI, SK_PNMI_EVT_VCT_RESET, Param);

				break;
			}

			VctGetResults(pAC, IoC, PhysPortIndex);

			EventParam.Para32[0] = PhysPortIndex;
			EventParam.Para32[1] = -1;
			SkEventQueue(pAC, SKGE_DRV, SK_DRV_PORT_RESET, EventParam);

			/* SkEventDispatcher(pAC, IoC); */
		}

		break;

	default:
		break;
	}

	SK_PNMI_CHECKFLAGS("SkPnmiEvent: On return");
	return (0);
}


/******************************************************************************
 *
 * Private functions
 *
 */

/*****************************************************************************
 *
 * PnmiVar - Gets, presets, and sets single OIDs
 *
 * Description:
 *	Looks up the requested OID, calls the corresponding handler
 *	function, and passes the parameters with the get, preset, or
 *	set command. The function is called by SkGePnmiGetVar,
 *	SkGePnmiPreSetVar, or SkGePnmiSetVar.
 *
 * Returns:
 *	SK_PNMI_ERR_XXX. For details have a look at the description of the
 *	calling functions.
 *	SK_PNMI_ERR_UNKNOWN_NET  The requested NetIndex doesn't exist
 */
PNMI_STATIC int PnmiVar(
SK_AC *pAC,		/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
int Action,		/* GET/PRESET/SET action */
SK_U32 Id,		/* Object ID that is to be processed */
char *pBuf,		/* Buffer used for the management data transfer */
unsigned int *pLen,	/* Total length of pBuf management data  */
SK_U32 Instance,	/* Instance (1..n) that is to be set or -1 */
SK_U32 NetIndex)	/* NetIndex (0..n), in single net mode always zero */
{
	unsigned int	TableIndex;
	int		Ret;

	if ((TableIndex = LookupId(Id)) == (unsigned int)(-1)) {

		*pLen = 0;
		return (SK_PNMI_ERR_UNKNOWN_OID);
	}
#ifndef SK_NO_RLMT
	/* Check NetIndex. */
	if (NetIndex >= pAC->Rlmt.NumNets) {
		return (SK_PNMI_ERR_UNKNOWN_NET);
	}
#else
	if (NetIndex >= pAC->GIni.GIMacsFound) {
		return (SK_PNMI_ERR_UNKNOWN_NET);
	}
#endif
	SK_PNMI_CHECKFLAGS("PnmiVar: On call");

	Ret = IdTable[TableIndex].Func(pAC, IoC, Action, Id, pBuf, pLen,
		Instance, TableIndex, NetIndex);

	SK_PNMI_CHECKFLAGS("PnmiVar: On return");

	return (Ret);
}

/*****************************************************************************
 *
 * PnmiStruct - Presets and Sets data in structure SK_PNMI_STRUCT_DATA
 *
 * Description:
 *	The return value of the function will also be stored in
 *	SK_PNMI_STRUCT_DATA if the passed buffer has the minimum size of
 *	SK_PNMI_MIN_STRUCT_SIZE. The sub-function runs through the IdTable,
 *	checks which OIDs are able to set, and calls the handler function of
 *	the OID to perform the set. The return value of the function will
 *	also be stored in SK_PNMI_STRUCT_DATA if the passed buffer has the
 *	minimum size of SK_PNMI_MIN_STRUCT_SIZE. The function is called
 *	by SkGePnmiPreSetStruct and SkGePnmiSetStruct.
 *
 * Returns:
 *	SK_PNMI_ERR_XXX. The codes are described in the calling functions.
 *	SK_PNMI_ERR_UNKNOWN_NET  The requested NetIndex doesn't exist
 */
PNMI_STATIC int PnmiStruct(
SK_AC *pAC,		/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
int  Action,	/* PRESET/SET action to be performed */
char *pBuf,		/* Buffer used for the management data transfer */
unsigned int *pLen,	/* Length of pBuf management data buffer */
SK_U32 NetIndex)	/* NetIndex (0..n), in single net mode always zero */
{
	int		Ret;
	unsigned int	TableIndex;
	unsigned int	DstOffset;
	unsigned int	Len;
	unsigned int	InstanceNo;
	unsigned int	InstanceCnt;
	SK_U32		Instance;
	SK_U32		Id;

	/* Check if the passed buffer has the right size. */
	if (*pLen < SK_PNMI_STRUCT_SIZE) {

		/* Check if we can return the error within the buffer. */
		if (*pLen >= SK_PNMI_MIN_STRUCT_SIZE) {

			SK_PNMI_SET_STAT(pBuf, SK_PNMI_ERR_TOO_SHORT, (SK_U32)(-1));
		}

		*pLen = SK_PNMI_STRUCT_SIZE;
		return (SK_PNMI_ERR_TOO_SHORT);
	}
#ifndef SK_NO_RLMT
	/* Check NetIndex. */
	if (NetIndex >= pAC->Rlmt.NumNets) {
		return (SK_PNMI_ERR_UNKNOWN_NET);
	}
#else
	if (NetIndex >= pAC->GIni.GIMacsFound) {
		return (SK_PNMI_ERR_UNKNOWN_NET);
	}
#endif
	SK_PNMI_CHECKFLAGS("PnmiStruct: On call");

	/*
	 * Update the values of RLMT and SIRQ and increment semaphores to
	 * indicate that an update was already done.
	 */
#ifndef SK_NO_RLMT
	if ((Ret = RlmtUpdate(pAC, IoC, NetIndex)) != SK_PNMI_ERR_OK) {

		SK_PNMI_SET_STAT(pBuf, Ret, (SK_U32)(-1));
		*pLen = SK_PNMI_MIN_STRUCT_SIZE;
		return (Ret);
	}
#endif
	if ((Ret = SirqUpdate(pAC, IoC)) != SK_PNMI_ERR_OK) {

		SK_PNMI_SET_STAT(pBuf, Ret, (SK_U32)(-1));
		*pLen = SK_PNMI_MIN_STRUCT_SIZE;
		return (Ret);
	}

	pAC->Pnmi.RlmtUpdatedFlag ++;
	pAC->Pnmi.SirqUpdatedFlag ++;

	/* PRESET/SET values. */
	for (TableIndex = 0; TableIndex < ID_TABLE_SIZE; TableIndex ++) {

		if ((IdTable[TableIndex].Access != SK_PNMI_RW) &&
			(IdTable[TableIndex].Access != SK_PNMI_WO)) {
			continue;
		}

		InstanceNo = IdTable[TableIndex].InstanceNo;
		Id = IdTable[TableIndex].Id;

		for (InstanceCnt = 1; InstanceCnt <= InstanceNo;
			InstanceCnt ++) {

			DstOffset = IdTable[TableIndex].Offset +
				(InstanceCnt - 1) * IdTable[TableIndex].StructSize;

			/*
			 * Because VPD multiple instance variables are
			 * not setable we do not need to evaluate VPD
			 * instances. Have a look to VPD instance
			 * calculation in SkPnmiGetStruct().
			 */
			Instance = (SK_U32)InstanceCnt;

			/* Evaluate needed buffer length. */
			Len = 0;
			Ret = IdTable[TableIndex].Func(pAC, IoC,
				SK_PNMI_GET, IdTable[TableIndex].Id,
				NULL, &Len, Instance, TableIndex, NetIndex);

			if (Ret == SK_PNMI_ERR_UNKNOWN_INST) {

				break;
			}

			if (Ret != SK_PNMI_ERR_TOO_SHORT) {

				pAC->Pnmi.RlmtUpdatedFlag --;
				pAC->Pnmi.SirqUpdatedFlag --;

				SK_PNMI_CHECKFLAGS("PnmiStruct: On return");
				SK_PNMI_SET_STAT(pBuf, SK_PNMI_ERR_GENERAL, DstOffset);
				*pLen = SK_PNMI_MIN_STRUCT_SIZE;

				return (SK_PNMI_ERR_GENERAL);
			}

			if (Id == OID_SKGE_VPD_ACTION) {

				switch (*(pBuf + DstOffset)) {

				case SK_PNMI_VPD_CREATE:
					Len = 3 + *(pBuf + DstOffset + 3);
					break;

				case SK_PNMI_VPD_DELETE:
					Len = 3;
					break;

				default:
					Len = 1;
					break;
				}
			}

			/* Call the OID handler function. */
			Ret = IdTable[TableIndex].Func(pAC, IoC, Action,
				IdTable[TableIndex].Id, pBuf + DstOffset,
				&Len, Instance, TableIndex, NetIndex);

			if (Ret != SK_PNMI_ERR_OK) {

				pAC->Pnmi.RlmtUpdatedFlag --;
				pAC->Pnmi.SirqUpdatedFlag --;

				SK_PNMI_CHECKFLAGS("PnmiStruct: On return");
				SK_PNMI_SET_STAT(pBuf, SK_PNMI_ERR_BAD_VALUE, DstOffset);
				*pLen = SK_PNMI_MIN_STRUCT_SIZE;

				return (SK_PNMI_ERR_BAD_VALUE);
			}
		}
	}

	pAC->Pnmi.RlmtUpdatedFlag --;
	pAC->Pnmi.SirqUpdatedFlag --;

	SK_PNMI_CHECKFLAGS("PnmiStruct: On return");
	SK_PNMI_SET_STAT(pBuf, SK_PNMI_ERR_OK, (SK_U32)(-1));

	return (SK_PNMI_ERR_OK);
}

/*****************************************************************************
 *
 * LookupId - Lookup an OID in the IdTable
 *
 * Description:
 *	Scans the IdTable to find the table entry of an OID.
 *
 * Returns:
 *	The table index or -1 if not found.
 */
PNMI_STATIC int LookupId(
SK_U32 Id)		/* Object identifier to be searched */
{
	int i;

	for (i = 0; i < ID_TABLE_SIZE; i++) {

		if (IdTable[i].Id == Id) {

			return (i);
		}
	}

	return (-1);
}

/*****************************************************************************
 *
 * OidStruct - Handler of OID_SKGE_ALL_DATA
 *
 * Description:
 *	This OID performs a Get/Preset/SetStruct call and returns all data
 *	in a SK_PNMI_STRUCT_DATA structure.
 *
 * Returns:
 *	SK_PNMI_ERR_OK           The request was successfully performed.
 *	SK_PNMI_ERR_GENERAL      A general severe internal error occurred.
 *	SK_PNMI_ERR_TOO_SHORT    The passed buffer is too short to contain
 *	                         the correct data (e.g. a 32bit value is
 *	                         needed, but a 16 bit value was passed).
 *	SK_PNMI_ERR_BAD_VALUE    The passed value is not in the valid
 *	                         value range.
 *	SK_PNMI_ERR_READ_ONLY    The OID is read-only and cannot be set.
 *	SK_PNMI_ERR_UNKNOWN_INST The requested instance of the OID doesn't
 *                           exist (e.g. port instance 3 on a two port
 *	                         adapter.
 */
PNMI_STATIC int OidStruct(
SK_AC *pAC,		/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
int Action,		/* GET/PRESET/SET action */
SK_U32 Id,		/* Object ID that is to be processed */
char *pBuf,		/* Buffer used for the management data transfer */
unsigned int *pLen,	/* On call: pBuf buffer length. On return: used buffer */
SK_U32 Instance,	/* Instance (1..n) that is to be queried or -1 */
unsigned int TableIndex, /* Index to the Id table */
SK_U32 NetIndex)	/* NetIndex (0..n), in single net mode always zero */
{
	if (Id != OID_SKGE_ALL_DATA) {

		SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR003, SK_PNMI_ERR003MSG);

		*pLen = 0;
		return (SK_PNMI_ERR_GENERAL);
	}

	/* Check instance. We only handle single instance variables. */
	if (Instance != (SK_U32)(-1) && Instance != 1) {

		*pLen = 0;
		return (SK_PNMI_ERR_UNKNOWN_INST);
	}

	switch (Action) {

	case SK_PNMI_GET:
		return (SkPnmiGetStruct(pAC, IoC, pBuf, pLen, NetIndex));

	case SK_PNMI_PRESET:
		return (SkPnmiPreSetStruct(pAC, IoC, pBuf, pLen, NetIndex));

	case SK_PNMI_SET:
		return (SkPnmiSetStruct(pAC, IoC, pBuf, pLen, NetIndex));
	}

	SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR004, SK_PNMI_ERR004MSG);

	*pLen = 0;
	return (SK_PNMI_ERR_GENERAL);
}

/*****************************************************************************
 *
 * Perform - OID handler of OID_SKGE_ACTION
 *
 * Description:
 *	None.
 *
 * Returns:
 *	SK_PNMI_ERR_OK           The request was successfully performed.
 *	SK_PNMI_ERR_GENERAL      A general severe internal error occurred.
 *	SK_PNMI_ERR_TOO_SHORT    The passed buffer is too short to contain
 *	                         the correct data (e.g. a 32bit value is
 *	                         needed, but a 16 bit value was passed).
 *	SK_PNMI_ERR_BAD_VALUE    The passed value is not in the valid
 *	                         value range.
 *	SK_PNMI_ERR_READ_ONLY    The OID is read-only and cannot be set.
 *	SK_PNMI_ERR_UNKNOWN_INST The requested instance of the OID doesn't
 *                           exist (e.g. port instance 3 on a two port
 *	                         adapter.
 */
PNMI_STATIC int Perform(
SK_AC *pAC,		/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
int Action,		/* GET/PRESET/SET action */
SK_U32 Id,		/* Object ID that is to be processed */
char *pBuf,		/* Buffer used for the management data transfer */
unsigned int *pLen,	/* On call: pBuf buffer length. On return: used buffer */
SK_U32 Instance,	/* Instance (1..n) that is to be queried or -1 */
unsigned int TableIndex, /* Index to the Id table */
SK_U32 NetIndex)	/* NetIndex (0..n), in single net mode always zero */
{
	int	Ret;
	SK_U32	ActionOp;

	/* Check instance. We only handle single instance variables. */
	if (Instance != (SK_U32)(-1) && Instance != 1) {

		*pLen = 0;
		return (SK_PNMI_ERR_UNKNOWN_INST);
	}

	if (*pLen < sizeof(SK_U32)) {

		*pLen = sizeof(SK_U32);
		return (SK_PNMI_ERR_TOO_SHORT);
	}

	/* Check if a GET should be performed. */
	if (Action == SK_PNMI_GET) {

		/* A GET is easy. We always return the same value. */
		ActionOp = (SK_U32)SK_PNMI_ACT_IDLE;
		SK_PNMI_STORE_U32(pBuf, ActionOp);
		*pLen = sizeof(SK_U32);

		return (SK_PNMI_ERR_OK);
	}

	/* Continue with PRESET/SET action. */
	if (*pLen > sizeof(SK_U32)) {

		return (SK_PNMI_ERR_BAD_VALUE);
	}

	/* Check if the command is a known one. */
	SK_PNMI_READ_U32(pBuf, ActionOp);
	if (*pLen > sizeof(SK_U32) ||
		(ActionOp != SK_PNMI_ACT_IDLE &&
		ActionOp != SK_PNMI_ACT_RESET &&
		ActionOp != SK_PNMI_ACT_SELFTEST &&
		ActionOp != SK_PNMI_ACT_RESETCNT)) {

		*pLen = 0;
		return (SK_PNMI_ERR_BAD_VALUE);
	}

	/* A PRESET ends here. */
	if (Action == SK_PNMI_PRESET) {

		return (SK_PNMI_ERR_OK);
	}

	switch (ActionOp) {

	case SK_PNMI_ACT_IDLE:
		/* Nothing to do. */
		break;

	case SK_PNMI_ACT_RESET:
		/* Perform a driver reset or something that comes near to this. */
		Ret = SK_DRIVER_RESET(pAC, IoC);
		if (Ret != 0) {

			SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR005, SK_PNMI_ERR005MSG);

			return (SK_PNMI_ERR_GENERAL);
		}
		break;

	case SK_PNMI_ACT_SELFTEST:
		/*
		 * Perform a driver selftest or something similar to this.
		 * Currently this feature is not used and will probably
		 * implemented in another way.
		 */
		Ret = SK_DRIVER_SELFTEST(pAC, IoC);
		pAC->Pnmi.TestResult = Ret;
		break;

	case SK_PNMI_ACT_RESETCNT:
		/* Set all counters and timestamps to zero. */
		ResetCounter(pAC, IoC, NetIndex);
		break;

	default:
		SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR006, SK_PNMI_ERR006MSG);

		return (SK_PNMI_ERR_GENERAL);
	}

	return (SK_PNMI_ERR_OK);
}

/*****************************************************************************
 *
 * Mac8023Stat - OID handler of OID_GEN_XXX and OID_802_3_XXX
 *
 * Description:
 *	Retrieves the statistic values of the virtual port (logical
 *	index 0). Only special OIDs of NDIS are handled which consist
 *	of a 32 bit instead of a 64 bit value. The OIDs are public
 *	because perhaps some other platform can use them too.
 *
 * Returns:
 *	SK_PNMI_ERR_OK           The request was successfully performed.
 *	SK_PNMI_ERR_GENERAL      A general severe internal error occurred.
 *	SK_PNMI_ERR_TOO_SHORT    The passed buffer is too short to contain
 *	                         the correct data (e.g. a 32bit value is
 *	                         needed, but a 16 bit value was passed).
 *	SK_PNMI_ERR_UNKNOWN_INST The requested instance of the OID doesn't
 *                           exist (e.g. port instance 3 on a two port
 *	                         adapter.
 */
PNMI_STATIC int Mac8023Stat(
SK_AC *pAC,		/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
int Action,		/* GET/PRESET/SET action */
SK_U32 Id,		/* Object ID that is to be processed */
char *pBuf,		/* Buffer used for the management data transfer */
unsigned int *pLen,	/* On call: pBuf buffer length. On return: used buffer */
SK_U32 Instance,	/* Instance (1..n) that is to be queried or -1 */
unsigned int TableIndex,	/* Index to the Id table */
SK_U32 NetIndex)	/* NetIndex (0..n), in single net mode always zero */
{
	int     Ret;
	SK_U64  StatVal;
	SK_U32  StatVal32;
	SK_BOOL Is64BitReq = SK_FALSE;

	/* Only the active MAC is returned. */
	if (Instance != (SK_U32)(-1) && Instance != 1) {

		*pLen = 0;
		return (SK_PNMI_ERR_UNKNOWN_INST);
	}

	/* Check action type. */
	if (Action != SK_PNMI_GET) {

		*pLen = 0;
		return (SK_PNMI_ERR_READ_ONLY);
	}

	/* Check length. */
	switch (Id) {

	case OID_802_3_PERMANENT_ADDRESS:
	case OID_802_3_CURRENT_ADDRESS:
		if (*pLen < sizeof(SK_MAC_ADDR)) {

			*pLen = sizeof(SK_MAC_ADDR);
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		break;

	default:
#ifndef SK_NDIS_64BIT_CTR
		if (*pLen < sizeof(SK_U32)) {
			*pLen = sizeof(SK_U32);
			return (SK_PNMI_ERR_TOO_SHORT);
		}

#else /* SK_NDIS_64BIT_CTR */

		/* For compatibility, at least 32 bits are required for OID. */
		if (*pLen < sizeof(SK_U32)) {
			/*
			 * Indicate handling for 64 bit values,
			 * if insufficient space is provided.
			 */
			*pLen = sizeof(SK_U64);
			return (SK_PNMI_ERR_TOO_SHORT);
		}

		Is64BitReq = (*pLen < sizeof(SK_U64)) ? SK_FALSE : SK_TRUE;
#endif /* SK_NDIS_64BIT_CTR */
		break;
	}

	/*
	 * Update all statistics, because we retrieve virtual MAC, which
	 * consists of multiple physical statistics and increment semaphore
	 * to indicate that an update was already done.
	 */
	Ret = MacUpdate(pAC, IoC, 0, pAC->GIni.GIMacsFound - 1);
	if (Ret != SK_PNMI_ERR_OK) {

		*pLen = 0;
		return (Ret);
	}
	pAC->Pnmi.MacUpdatedFlag ++;

	/* Get value (MAC index 0 identifies the virtual MAC). */
	switch (Id) {

	case OID_802_3_PERMANENT_ADDRESS:
		CopyMac(pBuf, &pAC->Addr.Net[NetIndex].PermanentMacAddress);
		*pLen = sizeof(SK_MAC_ADDR);
		break;

	case OID_802_3_CURRENT_ADDRESS:
		CopyMac(pBuf, &pAC->Addr.Net[NetIndex].CurrentMacAddress);
		*pLen = sizeof(SK_MAC_ADDR);
		break;

	default:
		StatVal = GetStatVal(pAC, IoC, 0, IdTable[TableIndex].Param, NetIndex);

		/* By default 32 bit values are evaluated. */
		if (!Is64BitReq) {
			StatVal32 = (SK_U32)StatVal;
			SK_PNMI_STORE_U32(pBuf, StatVal32);
			*pLen = sizeof(SK_U32);
		}
		else {
			SK_PNMI_STORE_U64(pBuf, StatVal);
			*pLen = sizeof(SK_U64);
		}
		break;
	}

	pAC->Pnmi.MacUpdatedFlag --;

	return (SK_PNMI_ERR_OK);
}

/*****************************************************************************
 *
 * MacPrivateStat - OID handler function of OID_SKGE_STAT_XXX
 *
 * Description:
 *	Retrieves the MAC statistic data.
 *
 * Returns:
 *	SK_PNMI_ERR_OK           The request was successfully performed.
 *	SK_PNMI_ERR_GENERAL      A general severe internal error occurred.
 *	SK_PNMI_ERR_TOO_SHORT    The passed buffer is too short to contain
 *	                         the correct data (e.g. a 32bit value is
 *	                         needed, but a 16 bit value was passed).
 *	SK_PNMI_ERR_UNKNOWN_INST The requested instance of the OID doesn't
 *                           exist (e.g. port instance 3 on a two port
 *	                         adapter.
 */
PNMI_STATIC int MacPrivateStat(
SK_AC *pAC,		/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
int Action,		/* GET/PRESET/SET action */
SK_U32 Id,		/* Object ID that is to be processed */
char *pBuf,		/* Buffer used for the management data transfer */
unsigned int *pLen,	/* On call: pBuf buffer length. On return: used buffer */
SK_U32 Instance,	/* Instance (1..n) that is to be queried or -1 */
unsigned int TableIndex, /* Index to the Id table */
SK_U32 NetIndex)	/* NetIndex (0..n), in single net mode always zero */
{
	unsigned int	LogPortMax;
	unsigned int	LogPortIndex;
	unsigned int	PhysPortMax;
	unsigned int	Limit;
	unsigned int	Offset;
	int				MacType;
	int				Ret;
	SK_U64			StatVal;

	/* Calculate instance if wished. MAC index 0 is the virtual MAC. */
	PhysPortMax = pAC->GIni.GIMacsFound;
	LogPortMax = SK_PNMI_PORT_PHYS2LOG(PhysPortMax);

	MacType = pAC->GIni.GIMacType;

	if (pAC->Pnmi.DualNetActiveFlag) { /* DualNet mode */
		LogPortMax--;
	}

	if ((Instance != (SK_U32)(-1))) { /* Only one specific instance is queried. */
		/* Check instance range. */
		if ((Instance < 1) || (Instance > LogPortMax)) {

			*pLen = 0;
			return (SK_PNMI_ERR_UNKNOWN_INST);
		}
		LogPortIndex = SK_PNMI_PORT_INST2LOG(Instance);
		Limit = LogPortIndex + 1;
	}

	else { /* Instance == (SK_U32)(-1), get all Instances of that OID. */

		LogPortIndex = 0;
		Limit = LogPortMax;
	}

	/* Check action. */
	if (Action != SK_PNMI_GET) {

		*pLen = 0;
		return (SK_PNMI_ERR_READ_ONLY);
	}

	/* Check length. */
	if (*pLen < (Limit - LogPortIndex) * sizeof(SK_U64)) {

		*pLen = (Limit - LogPortIndex) * sizeof(SK_U64);
		return (SK_PNMI_ERR_TOO_SHORT);
	}

	/*
	 * Update MAC statistic and increment semaphore to indicate that
	 * an update was already done.
	 */
	Ret = MacUpdate(pAC, IoC, 0, pAC->GIni.GIMacsFound - 1);
	if (Ret != SK_PNMI_ERR_OK) {

		*pLen = 0;
		return (Ret);
	}
	pAC->Pnmi.MacUpdatedFlag ++;

	/* Get value. */
	Offset = 0;
	for (; LogPortIndex < Limit; LogPortIndex ++) {

		switch (Id) {

/* XXX not yet implemented due to XMAC problems
		case OID_SKGE_STAT_TX_UTIL:
			return (SK_PNMI_ERR_GENERAL);
*/
/* XXX not yet implemented due to XMAC problems
		case OID_SKGE_STAT_RX_UTIL:
			return (SK_PNMI_ERR_GENERAL);
*/
		case OID_SKGE_STAT_RX:
			if (MacType == SK_MAC_GMAC) {
				StatVal =
					GetStatVal(pAC, IoC, LogPortIndex,
							   SK_PNMI_HRX_BROADCAST, NetIndex) +
					GetStatVal(pAC, IoC, LogPortIndex,
							   SK_PNMI_HRX_MULTICAST, NetIndex) +
					GetStatVal(pAC, IoC, LogPortIndex,
							   SK_PNMI_HRX_UNICAST, NetIndex) -
					GetStatVal(pAC, IoC, LogPortIndex,
							   SK_PNMI_HRX_UNDERSIZE, NetIndex);
			}
			else {
				StatVal = GetStatVal(pAC, IoC, LogPortIndex,
					IdTable[TableIndex].Param, NetIndex);
			}
			break;

		case OID_SKGE_STAT_TX:
			if (MacType == SK_MAC_GMAC) {
				StatVal =
					GetStatVal(pAC, IoC, LogPortIndex,
							   SK_PNMI_HTX_BROADCAST, NetIndex) +
					GetStatVal(pAC, IoC, LogPortIndex,
							   SK_PNMI_HTX_MULTICAST, NetIndex) +
					GetStatVal(pAC, IoC, LogPortIndex,
							   SK_PNMI_HTX_UNICAST, NetIndex);
			}
			else {
				StatVal = GetStatVal(pAC, IoC, LogPortIndex,
					IdTable[TableIndex].Param, NetIndex);
			}
			break;

		default:
			StatVal = GetStatVal(pAC, IoC, LogPortIndex,
				IdTable[TableIndex].Param, NetIndex);
		}
		SK_PNMI_STORE_U64(pBuf + Offset, StatVal);

		Offset += sizeof(SK_U64);
	}
	*pLen = Offset;

	pAC->Pnmi.MacUpdatedFlag --;

	return (SK_PNMI_ERR_OK);
}

/*****************************************************************************
 *
 * Addr - OID handler function of OID_SKGE_PHYS_CUR_ADDR and _FAC_ADDR
 *
 * Description:
 *	Get/Presets/Sets the current and factory MAC address. The MAC
 *	address of the virtual port, which is reported to the OS, may
 *	not be changed, but the physical ones. A set to the virtual port
 *	will be ignored. No error should be reported because otherwise
 *	a multiple instance set (-1) would always fail.
 *
 * Returns:
 *	SK_PNMI_ERR_OK           The request was successfully performed.
 *	SK_PNMI_ERR_GENERAL      A general severe internal error occurred.
 *	SK_PNMI_ERR_TOO_SHORT    The passed buffer is too short to contain
 *	                         the correct data (e.g. a 32bit value is
 *	                         needed, but a 16 bit value was passed).
 *	SK_PNMI_ERR_BAD_VALUE    The passed value is not in the valid
 *	                         value range.
 *	SK_PNMI_ERR_READ_ONLY    The OID is read-only and cannot be set.
 *	SK_PNMI_ERR_UNKNOWN_INST The requested instance of the OID doesn't
 *                           exist (e.g. port instance 3 on a two port
 *	                         adapter.
 */
PNMI_STATIC int Addr(
SK_AC *pAC,		/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
int Action,		/* GET/PRESET/SET action */
SK_U32 Id,		/* Object ID that is to be processed */
char *pBuf,		/* Buffer used for the management data transfer */
unsigned int *pLen,	/* On call: pBuf buffer length. On return: used buffer */
SK_U32 Instance,	/* Instance (1..n) that is to be queried or -1 */
unsigned int TableIndex, /* Index to the Id table */
SK_U32 NetIndex)	/* NetIndex (0..n), in single net mode always zero */
{
	int		Ret;
	unsigned int	LogPortMax;
	unsigned int	PhysPortMax;
	unsigned int	LogPortIndex;
	unsigned int	PhysPortIndex;
	unsigned int	Limit;
	unsigned int	Offset = 0;

	/* Calculate instance if wished. MAC index 0 is the virtual MAC. */
	PhysPortMax = pAC->GIni.GIMacsFound;
	LogPortMax = SK_PNMI_PORT_PHYS2LOG(PhysPortMax);

	if (pAC->Pnmi.DualNetActiveFlag) { /* DualNet mode */
		LogPortMax--;
	}

	if ((Instance != (SK_U32)(-1))) { /* Only one specific instance is queried. */
		/* Check instance range. */
		if ((Instance < 1) || (Instance > LogPortMax)) {

			*pLen = 0;
			return (SK_PNMI_ERR_UNKNOWN_INST);
		}
		LogPortIndex = SK_PNMI_PORT_INST2LOG(Instance);
		Limit = LogPortIndex + 1;
	}
	else { /* Instance == (SK_U32)(-1), get all Instances of that OID. */

		LogPortIndex = 0;
		Limit = LogPortMax;
	}

	/* Perform action. */
	if (Action == SK_PNMI_GET) {

		/* Check length. */
		if (*pLen < (Limit - LogPortIndex) * 6) {

			*pLen = (Limit - LogPortIndex) * 6;
			return (SK_PNMI_ERR_TOO_SHORT);
		}

		/* Get value. */
		for (; LogPortIndex < Limit; LogPortIndex ++) {

			switch (Id) {

			case OID_SKGE_PHYS_CUR_ADDR:
				if (LogPortIndex == 0) {
					CopyMac(pBuf + Offset, &pAC->Addr.Net[NetIndex].CurrentMacAddress);
				}
				else {
					PhysPortIndex = SK_PNMI_PORT_LOG2PHYS(pAC, LogPortIndex);

					CopyMac(pBuf + Offset,
						&pAC->Addr.Port[PhysPortIndex].CurrentMacAddress);
				}
				Offset += 6;
				break;

			case OID_SKGE_PHYS_FAC_ADDR:
				if (LogPortIndex == 0) {
					CopyMac(pBuf + Offset,
						&pAC->Addr.Net[NetIndex].PermanentMacAddress);
				}
				else {
					PhysPortIndex = SK_PNMI_PORT_LOG2PHYS(pAC, LogPortIndex);

					CopyMac(pBuf + Offset,
						&pAC->Addr.Port[PhysPortIndex].PermanentMacAddress);
				}
				Offset += 6;
				break;

			default:
				SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR008, SK_PNMI_ERR008MSG);

				*pLen = 0;
				return (SK_PNMI_ERR_GENERAL);
			}
		}

		*pLen = Offset;
	}
	else {
		/*
		 * The logical MAC address may not be changed,
		 * only the physical ones.
		 */
		if (Id == OID_SKGE_PHYS_FAC_ADDR) {

			*pLen = 0;
			return (SK_PNMI_ERR_READ_ONLY);
		}

		/* Only the current address may be changed. */
		if (Id != OID_SKGE_PHYS_CUR_ADDR) {

			SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR009, SK_PNMI_ERR009MSG);

			*pLen = 0;
			return (SK_PNMI_ERR_GENERAL);
		}

		/* Check length. */
		if (*pLen < (Limit - LogPortIndex) * 6) {

			*pLen = (Limit - LogPortIndex) * 6;
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		if (*pLen > (Limit - LogPortIndex) * 6) {

			*pLen = 0;
			return (SK_PNMI_ERR_BAD_VALUE);
		}

		/* Check action. */
		if (Action == SK_PNMI_PRESET) {

			*pLen = 0;
			return (SK_PNMI_ERR_OK);
		}

		/* Set OID_SKGE_MAC_CUR_ADDR.  */
		for (; LogPortIndex < Limit; LogPortIndex ++, Offset += 6) {

			/*
			 * A set to virtual port and set of broadcast
			 * address will be ignored.
			 */
			if (LogPortIndex == 0 || SK_MEMCMP(pBuf + Offset,
				"\xff\xff\xff\xff\xff\xff", 6) == 0) {
				continue;
			}

			PhysPortIndex = SK_PNMI_PORT_LOG2PHYS(pAC, LogPortIndex);

			Ret = SkAddrOverride(pAC, IoC, PhysPortIndex,
				(SK_MAC_ADDR *)(pBuf + Offset),
				(LogPortIndex == 0 ? SK_ADDR_VIRTUAL_ADDRESS :
				SK_ADDR_PHYSICAL_ADDRESS));
			if (Ret != SK_ADDR_OVERRIDE_SUCCESS) {

				return (SK_PNMI_ERR_GENERAL);
			}
		}
		*pLen = Offset;
	}

	return (SK_PNMI_ERR_OK);
}

/*****************************************************************************
 *
 * CsumStat - OID handler function of OID_SKGE_CHKSM_XXX
 *
 * Description:
 *	Retrieves the statistic values of the CSUM module. The CSUM data
 *	structure must be available in the SK_AC even if the CSUM module
 *	is not included, because PNMI reads the statistic data from the
 *	CSUM part of SK_AC directly.
 *
 * Returns:
 *	SK_PNMI_ERR_OK           The request was successfully performed.
 *	SK_PNMI_ERR_GENERAL      A general severe internal error occurred.
 *	SK_PNMI_ERR_TOO_SHORT    The passed buffer is too short to contain
 *	                         the correct data (e.g. a 32bit value is
 *	                         needed, but a 16 bit value was passed).
 *	SK_PNMI_ERR_UNKNOWN_INST The requested instance of the OID doesn't
 *                           exist (e.g. port instance 3 on a two port
 *	                         adapter.
 */
PNMI_STATIC int CsumStat(
SK_AC *pAC,		/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
int Action,		/* GET/PRESET/SET action */
SK_U32 Id,		/* Object ID that is to be processed */
char *pBuf,		/* Buffer used for the management data transfer */
unsigned int *pLen,	/* On call: pBuf buffer length. On return: used buffer */
SK_U32 Instance,	/* Instance (1..n) that is to be queried or -1 */
unsigned int TableIndex, /* Index to the Id table */
SK_U32 NetIndex)	/* NetIndex (0..n), in single net mode always zero */
{
	unsigned int	Index;
	unsigned int	Limit;
	unsigned int	Offset = 0;
	SK_U64		StatVal;

	/* Calculate instance if wished. */
	if (Instance != (SK_U32)(-1)) {

		if ((Instance < 1) || (Instance > SKCS_NUM_PROTOCOLS)) {

			*pLen = 0;
			return (SK_PNMI_ERR_UNKNOWN_INST);
		}
		Index = (unsigned int)Instance - 1;
		Limit = Index + 1;
	}
	else {
		Index = 0;
		Limit = SKCS_NUM_PROTOCOLS;
	}

	/* Check action. */
	if (Action != SK_PNMI_GET) {

		*pLen = 0;
		return (SK_PNMI_ERR_READ_ONLY);
	}

	/* Check length. */
	if (*pLen < (Limit - Index) * sizeof(SK_U64)) {

		*pLen = (Limit - Index) * sizeof(SK_U64);
		return (SK_PNMI_ERR_TOO_SHORT);
	}

	/* Get value. */
	for (; Index < Limit; Index ++) {

		switch (Id) {

		case OID_SKGE_CHKSM_RX_OK_CTS:
			StatVal = pAC->Csum.ProtoStats[NetIndex][Index].RxOkCts;
			break;

		case OID_SKGE_CHKSM_RX_UNABLE_CTS:
			StatVal = pAC->Csum.ProtoStats[NetIndex][Index].RxUnableCts;
			break;

		case OID_SKGE_CHKSM_RX_ERR_CTS:
			StatVal = pAC->Csum.ProtoStats[NetIndex][Index].RxErrCts;
			break;

		case OID_SKGE_CHKSM_TX_OK_CTS:
			StatVal = pAC->Csum.ProtoStats[NetIndex][Index].TxOkCts;
			break;

		case OID_SKGE_CHKSM_TX_UNABLE_CTS:
			StatVal = pAC->Csum.ProtoStats[NetIndex][Index].TxUnableCts;
			break;

		default:
			SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR010, SK_PNMI_ERR010MSG);

			*pLen = 0;
			return (SK_PNMI_ERR_GENERAL);
		}

		SK_PNMI_STORE_U64(pBuf + Offset, StatVal);
		Offset += sizeof(SK_U64);
	}

	/* Store used buffer space. */
	*pLen = Offset;

	return (SK_PNMI_ERR_OK);
}

/*****************************************************************************
 *
 * SensorStat - OID handler function of OID_SKGE_SENSOR_XXX
 *
 * Description:
 *	Retrieves the statistic values of the I2C module, which handles
 *	the temperature and voltage sensors.
 *
 * Returns:
 *	SK_PNMI_ERR_OK           The request was successfully performed.
 *	SK_PNMI_ERR_GENERAL      A general severe internal error occurred.
 *	SK_PNMI_ERR_TOO_SHORT    The passed buffer is too short to contain
 *	                         the correct data (e.g. a 32bit value is
 *	                         needed, but a 16 bit value was passed).
 *	SK_PNMI_ERR_UNKNOWN_INST The requested instance of the OID doesn't
 *                           exist (e.g. port instance 3 on a two port
 *	                         adapter.
 */
PNMI_STATIC int SensorStat(
SK_AC *pAC,		/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
int Action,		/* GET/PRESET/SET action */
SK_U32 Id,		/* Object ID that is to be processed */
char *pBuf,		/* Buffer used for the management data transfer */
unsigned int *pLen,	/* On call: pBuf buffer length. On return: used buffer */
SK_U32 Instance,	/* Instance (1..n) that is to be queried or -1 */
unsigned int TableIndex, /* Index to the Id table */
SK_U32 NetIndex)	/* NetIndex (0..n), in single net mode always zero */
{
	unsigned int	i;
	unsigned int	Index;
	unsigned int	Limit;
	unsigned int	Offset;
	unsigned int	Len;
	SK_U32		Val32;
	SK_U64		Val64;

	/* Calculate instance if wished. */
	if ((Instance != (SK_U32)(-1))) {

		if ((Instance < 1) || (Instance > (SK_U32)pAC->I2c.MaxSens)) {

			*pLen = 0;
			return (SK_PNMI_ERR_UNKNOWN_INST);
		}

		Index = (unsigned int)Instance -1;
		Limit = (unsigned int)Instance;
	}
	else {
		Index = 0;
		Limit = (unsigned int) pAC->I2c.MaxSens;
	}

	/* Check action. */
	if (Action != SK_PNMI_GET) {

		*pLen = 0;
		return (SK_PNMI_ERR_READ_ONLY);
	}

	/* Check length. */
	switch (Id) {

	case OID_SKGE_SENSOR_VALUE:
	case OID_SKGE_SENSOR_WAR_THRES_LOW:
	case OID_SKGE_SENSOR_WAR_THRES_UPP:
	case OID_SKGE_SENSOR_ERR_THRES_LOW:
	case OID_SKGE_SENSOR_ERR_THRES_UPP:
		if (*pLen < (Limit - Index) * sizeof(SK_U32)) {

			*pLen = (Limit - Index) * sizeof(SK_U32);
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		break;

	case OID_SKGE_SENSOR_DESCR:
		for (Offset = 0, i = Index; i < Limit; i ++) {

			Len = (unsigned int)
				SK_STRLEN(pAC->I2c.SenTable[i].SenDesc) + 1;
			if (Len >= SK_PNMI_STRINGLEN2) {

				SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR011, SK_PNMI_ERR011MSG);

				*pLen = 0;
				return (SK_PNMI_ERR_GENERAL);
			}
			Offset += Len;
		}
		if (*pLen < Offset) {

			*pLen = Offset;
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		break;

	case OID_SKGE_SENSOR_INDEX:
	case OID_SKGE_SENSOR_TYPE:
	case OID_SKGE_SENSOR_STATUS:
		if (*pLen < Limit - Index) {

			*pLen = Limit - Index;
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		break;

	case OID_SKGE_SENSOR_WAR_CTS:
	case OID_SKGE_SENSOR_WAR_TIME:
	case OID_SKGE_SENSOR_ERR_CTS:
	case OID_SKGE_SENSOR_ERR_TIME:
		if (*pLen < (Limit - Index) * sizeof(SK_U64)) {

			*pLen = (Limit - Index) * sizeof(SK_U64);
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		break;

	default:
		SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR012, SK_PNMI_ERR012MSG);

		*pLen = 0;
		return (SK_PNMI_ERR_GENERAL);
	}

	/* Get value. */
	for (Offset = 0; Index < Limit; Index ++) {

		switch (Id) {

		case OID_SKGE_SENSOR_INDEX:
			*(pBuf + Offset) = (char)Index;
			Offset++;
			break;

		case OID_SKGE_SENSOR_DESCR:
			Len = SK_STRLEN(pAC->I2c.SenTable[Index].SenDesc);
			SK_MEMCPY(pBuf + Offset + 1, pAC->I2c.SenTable[Index].SenDesc, Len);
			*(pBuf + Offset) = (char)Len;
			Offset += Len + 1;
			break;

		case OID_SKGE_SENSOR_TYPE:
			*(pBuf + Offset) = (char)pAC->I2c.SenTable[Index].SenType;
			Offset++;
			break;

		case OID_SKGE_SENSOR_VALUE:
			Val32 = (SK_U32)pAC->I2c.SenTable[Index].SenValue;
			SK_PNMI_STORE_U32(pBuf + Offset, Val32);
			Offset += sizeof(SK_U32);
			break;

		case OID_SKGE_SENSOR_WAR_THRES_LOW:
			Val32 = (SK_U32)pAC->I2c.SenTable[Index].
				SenThreWarnLow;
			SK_PNMI_STORE_U32(pBuf + Offset, Val32);
			Offset += sizeof(SK_U32);
			break;

		case OID_SKGE_SENSOR_WAR_THRES_UPP:
			Val32 = (SK_U32)pAC->I2c.SenTable[Index].
				SenThreWarnHigh;
			SK_PNMI_STORE_U32(pBuf + Offset, Val32);
			Offset += sizeof(SK_U32);
			break;

		case OID_SKGE_SENSOR_ERR_THRES_LOW:
			Val32 = (SK_U32)pAC->I2c.SenTable[Index].
				SenThreErrLow;
			SK_PNMI_STORE_U32(pBuf + Offset, Val32);
			Offset += sizeof(SK_U32);
			break;

		case OID_SKGE_SENSOR_ERR_THRES_UPP:
			Val32 = pAC->I2c.SenTable[Index].SenThreErrHigh;
			SK_PNMI_STORE_U32(pBuf + Offset, Val32);
			Offset += sizeof(SK_U32);
			break;

		case OID_SKGE_SENSOR_STATUS:
			*(pBuf + Offset) = (char)pAC->I2c.SenTable[Index].SenErrFlag;
			Offset++;
			break;

		case OID_SKGE_SENSOR_WAR_CTS:
			Val64 = pAC->I2c.SenTable[Index].SenWarnCts;
			SK_PNMI_STORE_U64(pBuf + Offset, Val64);
			Offset += sizeof(SK_U64);
			break;

		case OID_SKGE_SENSOR_ERR_CTS:
			Val64 = pAC->I2c.SenTable[Index].SenErrCts;
			SK_PNMI_STORE_U64(pBuf + Offset, Val64);
			Offset += sizeof(SK_U64);
			break;

		case OID_SKGE_SENSOR_WAR_TIME:
			Val64 = SK_PNMI_HUNDREDS_SEC(pAC->I2c.SenTable[Index].
				SenBegWarnTS);
			SK_PNMI_STORE_U64(pBuf + Offset, Val64);
			Offset += sizeof(SK_U64);
			break;

		case OID_SKGE_SENSOR_ERR_TIME:
			Val64 = SK_PNMI_HUNDREDS_SEC(pAC->I2c.SenTable[Index].
				SenBegErrTS);
			SK_PNMI_STORE_U64(pBuf + Offset, Val64);
			Offset += sizeof(SK_U64);
			break;

		default:
			SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_ERR,
				("SensorStat: Unknown OID should be handled before"));

			return (SK_PNMI_ERR_GENERAL);
		}
	}

	/* Store used buffer space. */
	*pLen = Offset;

	return (SK_PNMI_ERR_OK);
}

/*****************************************************************************
 *
 * Vpd - OID handler function of OID_SKGE_VPD_XXX
 *
 * Description:
 *	Get/preset/set of VPD.
 *	As instance the name of a VPD key can be passed.
 *	The Instance parameter is a SK_U32 and can be used as a string buffer
 *	for the VPD key, because their maximum length is 4 byte.
 *
 * Returns:
 *	SK_PNMI_ERR_OK           The request was successfully performed.
 *	SK_PNMI_ERR_GENERAL      A general severe internal error occurred.
 *	SK_PNMI_ERR_TOO_SHORT    The passed buffer is too short to contain
 *	                         the correct data (e.g. a 32bit value is
 *	                         needed, but a 16 bit value was passed).
 *	SK_PNMI_ERR_BAD_VALUE    The passed value is not in the valid
 *	                         value range.
 *	SK_PNMI_ERR_READ_ONLY    The OID is read-only and cannot be set.
 *	SK_PNMI_ERR_UNKNOWN_INST The requested instance of the OID doesn't
 *                           exist (e.g. port instance 3 on a two port
 *	                         adapter.
 */
PNMI_STATIC int Vpd(
SK_AC *pAC,		/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
int Action,		/* GET/PRESET/SET action */
SK_U32 Id,		/* Object ID that is to be processed */
char *pBuf,		/* Buffer used for the management data transfer */
unsigned int *pLen,	/* On call: pBuf buffer length. On return: used buffer */
SK_U32 Instance,	/* Instance (1..n) that is to be queried or -1 */
unsigned int TableIndex, /* Index to the Id table */
SK_U32 NetIndex)	/* NetIndex (0..n), in single net mode always zero */
{
	SK_VPD_STATUS	*pVpdStatus;
	unsigned int	BufLen;
	char		Buf[256];
	char		KeyArr[SK_PNMI_VPD_ENTRIES][SK_PNMI_VPD_KEY_SIZE];
	char		KeyStr[SK_PNMI_VPD_KEY_SIZE];
	unsigned int	KeyNo;
	unsigned int	Offset;
	unsigned int	Index;
	unsigned int	FirstIndex;
	unsigned int	LastIndex;
	unsigned int	Len;
	int		Ret;
	SK_U32		Val32;

	/*
	 * VpdKeyReadError will be set in GetVpdKeyArr() if an error occurs.
	 * Due to the fact that some drivers use SkPnmiGetStruct() to retrieve
	 * all statistical data, an error in GetVpdKeyArr() will generate a PNMI
	 * error and terminate SkPnmiGetStruct() without filling in statistical
	 * data into the PNMI struct. In this case the driver will get no values
	 * for statistical purposes (netstat, ifconfig etc.). GetVpdKeyArr() is
	 * the first function to be called in SkPnmiGetStruct(), so any error
	 * will terminate SkPnmiGetStruct() immediately. Hence, VpdKeyReadError will
	 * be set during the first call to GetVpdKeyArr() to make successful calls
	 * to SkPnmiGetStruct() possible. But there is another point to consider:
	 * When filling in the statistical data into the PNMI struct, the VPD
	 * handler Vpd() will also be called. If GetVpdKeyArr() in Vpd() would
	 * return with SK_PNMI_ERR_GENERAL, SkPnmiGetStruct() would fail again.
	 * For this reason VpdKeyReadError is checked here and, if set, Vpd()
	 * will return without doing anything and the return value SK_PNMI_ERR_OK.
	 * Therefore SkPnmiGetStruct() is able to continue and fill in all other
	 * statistical data.
	 */
	if (pAC->Pnmi.VpdKeyReadError) {
		return (SK_PNMI_ERR_OK);
	}

	/* Get array of all currently stored VPD keys. */
	Ret = GetVpdKeyArr(pAC, IoC, &KeyArr[0][0], sizeof(KeyArr), &KeyNo);

	if (Ret != SK_PNMI_ERR_OK) {
		*pLen = 0;
		return (Ret);
	}

	/*
	 * If instance is not -1, try to find the requested VPD key for
	 * the multiple instance variables. The other OIDs as for example
	 * OID VPD_ACTION are single instance variables and must be
	 * handled separatly.
	 */
	FirstIndex = 0;
	LastIndex = KeyNo;

	if ((Instance != (SK_U32)(-1))) {

		if (Id == OID_SKGE_VPD_KEY || Id == OID_SKGE_VPD_VALUE ||
			Id == OID_SKGE_VPD_ACCESS) {

			SK_STRNCPY(KeyStr, (char *)&Instance, 4);
			KeyStr[4] = 0;

			for (Index = 0; Index < KeyNo; Index ++) {

				if (SK_STRCMP(KeyStr, KeyArr[Index]) == 0) {
					FirstIndex = Index;
					LastIndex = Index+1;
					break;
				}
			}

			if (Index == KeyNo) {

				*pLen = 0;
				return (SK_PNMI_ERR_UNKNOWN_INST);
			}
		}
		else if (Instance != 1) {

			*pLen = 0;
			return (SK_PNMI_ERR_UNKNOWN_INST);
		}
	}

	/* Get value, if a query should be performed. */
	if (Action == SK_PNMI_GET) {

		switch (Id) {

		case OID_SKGE_VPD_FREE_BYTES:
			/* Check length of buffer. */
			if (*pLen < sizeof(SK_U32)) {

				*pLen = sizeof(SK_U32);
				return (SK_PNMI_ERR_TOO_SHORT);
			}

			/* Get number of free bytes. */
			pVpdStatus = VpdStat(pAC, IoC);

			if (pVpdStatus == NULL) {

				SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
					(SK_PNMI_ERR017MSG));

				*pLen = 0;
				return (SK_PNMI_ERR_GENERAL);
			}

			if ((pVpdStatus->vpd_status & VPD_VALID) == 0) {

				SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
					(SK_PNMI_ERR018MSG));

				*pLen = 0;
				return (SK_PNMI_ERR_GENERAL);
			}

			Val32 = (SK_U32)pVpdStatus->vpd_free_rw;
			SK_PNMI_STORE_U32(pBuf, Val32);
			*pLen = sizeof(SK_U32);
			break;

		case OID_SKGE_VPD_ENTRIES_LIST:
			/* Check length. */
			for (Len = 0, Index = 0; Index < KeyNo; Index ++) {

				Len += SK_STRLEN(KeyArr[Index]) + 1;
			}

			if (*pLen < Len) {

				*pLen = Len;
				return (SK_PNMI_ERR_TOO_SHORT);
			}

			/* Get value. */
			*(pBuf) = (char)Len - 1;

			for (Offset = 1, Index = 0; Index < KeyNo; Index ++) {

				Len = SK_STRLEN(KeyArr[Index]);
				SK_MEMCPY(pBuf + Offset, KeyArr[Index], Len);

				Offset += Len;

				if (Index < KeyNo - 1) {

					*(pBuf + Offset) = ' ';
					Offset++;
				}
			}
			*pLen = Offset;
			break;

		case OID_SKGE_VPD_ENTRIES_NUMBER:
			/* Check length. */
			if (*pLen < sizeof(SK_U32)) {

				*pLen = sizeof(SK_U32);
				return (SK_PNMI_ERR_TOO_SHORT);
			}

			Val32 = (SK_U32)KeyNo;
			SK_PNMI_STORE_U32(pBuf, Val32);
			*pLen = sizeof(SK_U32);
			break;

		case OID_SKGE_VPD_KEY:
			/* Check buffer length, if it is large enough. */
			for (Len = 0, Index = FirstIndex;
				Index < LastIndex; Index ++) {

				Len += SK_STRLEN(KeyArr[Index]) + 1;
			}
			if (*pLen < Len) {

				*pLen = Len;
				return (SK_PNMI_ERR_TOO_SHORT);
			}

			/*
			 * Get the key to an intermediate buffer, because
			 * we have to pretend a length byte.
			 */
			for (Offset = 0, Index = FirstIndex; Index < LastIndex; Index ++) {

				Len = SK_STRLEN(KeyArr[Index]);

				*(pBuf + Offset) = (char)Len;
				SK_MEMCPY(pBuf + Offset + 1, KeyArr[Index], Len);
				Offset += Len + 1;
			}
			*pLen = Offset;
			break;

		case OID_SKGE_VPD_VALUE:
			/* Check the buffer length if it is large enough. */
			for (Offset = 0, Index = FirstIndex; Index < LastIndex; Index ++) {

				BufLen = 256;
				if (VpdRead(pAC, IoC, KeyArr[Index], Buf, (int *)&BufLen) > 0 ||
					BufLen >= SK_PNMI_VPD_DATALEN) {

					SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
						(SK_PNMI_ERR021MSG));

					return (SK_PNMI_ERR_GENERAL);
				}

				Offset += BufLen + 1;
			}

			if (*pLen < Offset) {

				*pLen = Offset;
				return (SK_PNMI_ERR_TOO_SHORT);
			}

			/*
			 * Get the value to an intermediate buffer, because
			 * we have to pretend a length byte.
			 */
			for (Offset = 0, Index = FirstIndex; Index < LastIndex; Index ++) {

				BufLen = 256;
				if (VpdRead(pAC, IoC, KeyArr[Index], Buf, (int *)&BufLen) > 0 ||
					BufLen >= SK_PNMI_VPD_DATALEN) {

					SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
						(SK_PNMI_ERR022MSG));

					*pLen = 0;
					return (SK_PNMI_ERR_GENERAL);
				}

				*(pBuf + Offset) = (char)BufLen;
				SK_MEMCPY(pBuf + Offset + 1, Buf, BufLen);
				Offset += BufLen + 1;
			}
			*pLen = Offset;
			break;

		case OID_SKGE_VPD_ACCESS:
			if (*pLen < LastIndex - FirstIndex) {

				*pLen = LastIndex - FirstIndex;
				return (SK_PNMI_ERR_TOO_SHORT);
			}

			for (Offset = 0, Index = FirstIndex; Index < LastIndex; Index ++) {

				if (VpdMayWrite(KeyArr[Index])) {

					*(pBuf + Offset) = SK_PNMI_VPD_RW;
				}
				else {
					*(pBuf + Offset) = SK_PNMI_VPD_RO;
				}
				Offset++;
			}
			*pLen = Offset;
			break;

		case OID_SKGE_VPD_ACTION:
			Offset = LastIndex - FirstIndex;
			if (*pLen < Offset) {

				*pLen = Offset;
				return (SK_PNMI_ERR_TOO_SHORT);
			}

			SK_MEMSET(pBuf, 0, Offset);
			*pLen = Offset;
			break;

		default:
			SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
				(SK_PNMI_ERR023MSG));

			*pLen = 0;
			return (SK_PNMI_ERR_GENERAL);
		}
	}
	else {
		/* The only OID which can be set is VPD_ACTION. */
		if (Id != OID_SKGE_VPD_ACTION) {

			if (Id == OID_SKGE_VPD_FREE_BYTES ||
				Id == OID_SKGE_VPD_ENTRIES_LIST ||
				Id == OID_SKGE_VPD_ENTRIES_NUMBER ||
				Id == OID_SKGE_VPD_KEY ||
				Id == OID_SKGE_VPD_VALUE ||
				Id == OID_SKGE_VPD_ACCESS) {

				*pLen = 0;
				return (SK_PNMI_ERR_READ_ONLY);
			}

			SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
				(SK_PNMI_ERR024MSG));

			*pLen = 0;
			return (SK_PNMI_ERR_GENERAL);
		}

		/*
		 * From this point we handle VPD_ACTION. Check the buffer
		 * length. It should at least have the size of one byte.
		 */
		if (*pLen < 1) {

			*pLen = 1;
			return (SK_PNMI_ERR_TOO_SHORT);
		}

		/* The first byte contains the VPD action type we should perform. */
		switch (*pBuf) {

		case SK_PNMI_VPD_IGNORE:
			/* Nothing to do. */
			break;

		case SK_PNMI_VPD_CREATE:
			/*
			 * We have to create a new VPD entry or we modify
			 * an existing one. Check first the buffer length.
			 */
			if (*pLen < 4) {

				*pLen = 4;
				return (SK_PNMI_ERR_TOO_SHORT);
			}

			KeyStr[0] = pBuf[1];
			KeyStr[1] = pBuf[2];
			KeyStr[2] = 0;

			/* Is the entry writable or does it belong to the RO area ? */
			if (!VpdMayWrite(KeyStr)) {

				*pLen = 0;
				return (SK_PNMI_ERR_BAD_VALUE);
			}

			Offset = (int)pBuf[3] & 0xFF;

			SK_MEMCPY(Buf, pBuf + 4, Offset);
			Buf[Offset] = 0;

			/* A PRESET ends here. */
			if (Action == SK_PNMI_PRESET) {

				return (SK_PNMI_ERR_OK);
			}

			/* Write the new entry or modify an existing one .*/
			Ret = VpdWrite(pAC, IoC, KeyStr, Buf);

			if (Ret == SK_PNMI_VPD_NOWRITE) {

				*pLen = 0;
				return (SK_PNMI_ERR_BAD_VALUE);
			}

			if (Ret != SK_PNMI_VPD_OK) {

				SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
					(SK_PNMI_ERR025MSG));

				*pLen = 0;
				return (SK_PNMI_ERR_GENERAL);
			}

			/*
			 * Perform an update of the VPD.
			 * This is not mandantory, but just to be sure.
			 */
			Ret = VpdUpdate(pAC, IoC);

			if (Ret != SK_PNMI_VPD_OK) {

				SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
					(SK_PNMI_ERR026MSG));

				*pLen = 0;
				return (SK_PNMI_ERR_GENERAL);
			}
			break;

		case SK_PNMI_VPD_DELETE:
			/* Check if the buffer size is plausible. */
			if (*pLen < 3) {

				*pLen = 3;
				return (SK_PNMI_ERR_TOO_SHORT);
			}

			if (*pLen > 3) {

				*pLen = 0;
				return (SK_PNMI_ERR_BAD_VALUE);
			}

			KeyStr[0] = pBuf[1];
			KeyStr[1] = pBuf[2];
			KeyStr[2] = 0;

			/* Find the passed key in the array. */
			for (Index = 0; Index < KeyNo; Index ++) {

				if (SK_STRCMP(KeyStr, KeyArr[Index]) == 0) {

					break;
				}
			}

			/*
			 * If we cannot find the key it is wrong, so we
			 * return an appropriate error value.
			 */
			if (Index == KeyNo) {

				*pLen = 0;
				return (SK_PNMI_ERR_BAD_VALUE);
			}

			if (Action == SK_PNMI_PRESET) {

				return (SK_PNMI_ERR_OK);
			}

			/* Ok, you wanted it and you will get it. */
			Ret = VpdDelete(pAC, IoC, KeyStr);

			if (Ret != SK_PNMI_VPD_OK) {

				SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
					(SK_PNMI_ERR027MSG));

				*pLen = 0;
				return (SK_PNMI_ERR_GENERAL);
			}

			/*
			 * Perform an update of the VPD.
			 * This is not mandantory, but just to be sure.
			 */
			Ret = VpdUpdate(pAC, IoC);
			if (Ret != SK_PNMI_VPD_OK) {

				SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
					(SK_PNMI_ERR028MSG));

				*pLen = 0;
				return (SK_PNMI_ERR_GENERAL);
			}
			break;

		default:
			*pLen = 0;
			return (SK_PNMI_ERR_BAD_VALUE);
		}
	}

	return (SK_PNMI_ERR_OK);
}

/*****************************************************************************
 *
 * General - OID handler function of various single instance OIDs
 *
 * Description:
 *	The code is simple. No description necessary.
 *
 * Returns:
 *	SK_PNMI_ERR_OK           The request was successfully performed.
 *	SK_PNMI_ERR_GENERAL      A general severe internal error occurred.
 *	SK_PNMI_ERR_TOO_SHORT    The passed buffer is too short to contain
 *	                         the correct data (e.g. a 32bit value is
 *	                         needed, but a 16 bit value was passed).
 *	SK_PNMI_ERR_UNKNOWN_INST The requested instance of the OID doesn't
 *                           exist (e.g. port instance 3 on a two port
 *	                         adapter.
 */
PNMI_STATIC int General(
SK_AC *pAC,		/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
int Action,		/* GET/PRESET/SET action */
SK_U32 Id,		/* Object ID that is to be processed */
char *pBuf,		/* Buffer used for the management data transfer */
unsigned int *pLen,	/* On call: buffer length. On return: used buffer */
SK_U32 Instance,	/* Instance (1..n) that is to be queried or -1 */
unsigned int TableIndex, /* Index to the Id table */
SK_U32 NetIndex)	/* NetIndex (0..n), in single net mode always zero */
{
	int		Ret;
	unsigned int	Index;
	unsigned int	Len;
	unsigned int	Offset;
	unsigned int	Val;
	SK_U8		Val8;
	SK_U16		Val16;
	SK_U32		Val32;
	SK_U64		Val64;
	SK_U64		Val64RxHwErrs = 0;
	SK_U64		Val64RxRunt = 0;
	SK_U64		Val64RxFcs = 0;
	SK_U64		Val64TxHwErrs = 0;
	SK_BOOL		Is64BitReq = SK_FALSE;
	char		Buf[256];
	int			MacType;

	/* Check instance. We only handle single instance variables. */
	if (Instance != (SK_U32)(-1) && Instance != 1) {

		*pLen = 0;
		return (SK_PNMI_ERR_UNKNOWN_INST);
	}

	/* Check action. We only allow get requests. */
	if (Action != SK_PNMI_GET) {

		*pLen = 0;
		return (SK_PNMI_ERR_READ_ONLY);
	}

	MacType = pAC->GIni.GIMacType;

	/* Check length for the various supported OIDs. */
	switch (Id) {

	case OID_GEN_XMIT_ERROR:
	case OID_GEN_RCV_ERROR:
	case OID_GEN_RCV_NO_BUFFER:
#ifndef SK_NDIS_64BIT_CTR
		if (*pLen < sizeof(SK_U32)) {
			*pLen = sizeof(SK_U32);
			return (SK_PNMI_ERR_TOO_SHORT);
		}

#else /* SK_NDIS_64BIT_CTR */

		/* For compatibility, at least 32bit are required for OID. */
		if (*pLen < sizeof(SK_U32)) {
			/*
			 * Indicate handling for 64bit values,
			 * if insufficient space is provided.
			 */
			*pLen = sizeof(SK_U64);
			return (SK_PNMI_ERR_TOO_SHORT);
		}

		Is64BitReq = (*pLen < sizeof(SK_U64)) ? SK_FALSE : SK_TRUE;
#endif /* SK_NDIS_64BIT_CTR */
		break;

	case OID_SKGE_PORT_NUMBER:
	case OID_SKGE_DEVICE_TYPE:
	case OID_SKGE_RESULT:
	case OID_SKGE_RLMT_MONITOR_NUMBER:
	case OID_GEN_TRANSMIT_QUEUE_LENGTH:
	case OID_SKGE_TRAP_NUMBER:
	case OID_SKGE_MDB_VERSION:
	case OID_SKGE_BOARDLEVEL:
	case OID_SKGE_CHIPID:
	case OID_SKGE_RAMSIZE:
		if (*pLen < sizeof(SK_U32)) {

			*pLen = sizeof(SK_U32);
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		break;

	case OID_SKGE_CHIPSET:
		if (*pLen < sizeof(SK_U16)) {

			*pLen = sizeof(SK_U16);
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		break;

	case OID_SKGE_BUS_TYPE:
	case OID_SKGE_BUS_SPEED:
	case OID_SKGE_BUS_WIDTH:
	case OID_SKGE_SENSOR_NUMBER:
	case OID_SKGE_CHKSM_NUMBER:
	case OID_SKGE_VAUXAVAIL:
		if (*pLen < sizeof(SK_U8)) {

			*pLen = sizeof(SK_U8);
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		break;

	case OID_SKGE_TX_SW_QUEUE_LEN:
	case OID_SKGE_TX_SW_QUEUE_MAX:
	case OID_SKGE_TX_RETRY:
	case OID_SKGE_RX_INTR_CTS:
	case OID_SKGE_TX_INTR_CTS:
	case OID_SKGE_RX_NO_BUF_CTS:
	case OID_SKGE_TX_NO_BUF_CTS:
	case OID_SKGE_TX_USED_DESCR_NO:
	case OID_SKGE_RX_DELIVERED_CTS:
	case OID_SKGE_RX_OCTETS_DELIV_CTS:
	case OID_SKGE_RX_HW_ERROR_CTS:
	case OID_SKGE_TX_HW_ERROR_CTS:
	case OID_SKGE_IN_ERRORS_CTS:
	case OID_SKGE_OUT_ERROR_CTS:
	case OID_SKGE_ERR_RECOVERY_CTS:
	case OID_SKGE_SYSUPTIME:
		if (*pLen < sizeof(SK_U64)) {

			*pLen = sizeof(SK_U64);
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		break;

	default:
		/* Checked later. */
		break;
	}

	/* Update statistics. */
	if (Id == OID_SKGE_RX_HW_ERROR_CTS ||
		Id == OID_SKGE_TX_HW_ERROR_CTS ||
		Id == OID_SKGE_IN_ERRORS_CTS ||
		Id == OID_SKGE_OUT_ERROR_CTS ||
		Id == OID_GEN_XMIT_ERROR ||
		Id == OID_GEN_RCV_ERROR) {

		/*
		 * Force the XMAC to update its statistic counters and
		 * Increment semaphore to indicate that an update was
		 * already done.
		 */
		Ret = MacUpdate(pAC, IoC, 0, pAC->GIni.GIMacsFound - 1);
		if (Ret != SK_PNMI_ERR_OK) {

			*pLen = 0;
			return (Ret);
		}
		pAC->Pnmi.MacUpdatedFlag ++;

		/*
		 * Some OIDs consist of multiple hardware counters. Those
		 * values which are contained in all of them will be added
		 * now.
		 */
		switch (Id) {

		case OID_SKGE_RX_HW_ERROR_CTS:
		case OID_SKGE_IN_ERRORS_CTS:
		case OID_GEN_RCV_ERROR:
			Val64RxHwErrs =
				GetStatVal(pAC, IoC, 0, SK_PNMI_HRX_MISSED, NetIndex) +
				GetStatVal(pAC, IoC, 0, SK_PNMI_HRX_FRAMING, NetIndex) +
				GetStatVal(pAC, IoC, 0, SK_PNMI_HRX_OVERFLOW, NetIndex) +
				GetStatVal(pAC, IoC, 0, SK_PNMI_HRX_JABBER, NetIndex) +
				GetStatVal(pAC, IoC, 0, SK_PNMI_HRX_CARRIER, NetIndex) +
				GetStatVal(pAC, IoC, 0, SK_PNMI_HRX_IRLENGTH, NetIndex) +
				GetStatVal(pAC, IoC, 0, SK_PNMI_HRX_SYMBOL, NetIndex) +
				GetStatVal(pAC, IoC, 0, SK_PNMI_HRX_SHORTS, NetIndex) +
				GetStatVal(pAC, IoC, 0, SK_PNMI_HRX_TOO_LONG, NetIndex) +
				GetStatVal(pAC, IoC, 0, SK_PNMI_HRX_CEXT, NetIndex);


			/*
			 * In some cases the runt and fcs counters are incremented when collisions
			 * occur. We have to correct those counters here.
			 */
			Val64RxRunt = GetStatVal(pAC, IoC, 0, SK_PNMI_HRX_RUNT, NetIndex);
			Val64RxFcs = GetStatVal(pAC, IoC, 0, SK_PNMI_HRX_FCS, NetIndex);

			if (Val64RxRunt > Val64RxFcs) {
				Val64RxRunt -= Val64RxFcs;
				Val64RxHwErrs += Val64RxRunt;
			}
			else {
				Val64RxFcs -= Val64RxRunt;
				Val64RxHwErrs += Val64RxFcs;
			}
			break;

		case OID_SKGE_TX_HW_ERROR_CTS:
		case OID_SKGE_OUT_ERROR_CTS:
		case OID_GEN_XMIT_ERROR:
			Val64TxHwErrs =
				GetStatVal(pAC, IoC, 0, SK_PNMI_HTX_EXCESS_COL, NetIndex) +
				GetStatVal(pAC, IoC, 0, SK_PNMI_HTX_LATE_COL, NetIndex) +
				GetStatVal(pAC, IoC, 0, SK_PNMI_HTX_UNDERRUN, NetIndex) +
				GetStatVal(pAC, IoC, 0, SK_PNMI_HTX_CARRIER, NetIndex);
			break;
		}
	}

	/* Retrieve value. */
	switch (Id) {

	case OID_SKGE_SUPPORTED_LIST:
		Len = ID_TABLE_SIZE * sizeof(SK_U32);
		if (*pLen < Len) {

			*pLen = Len;
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		for (Offset = 0, Index = 0; Offset < Len; Index ++) {

			Val32 = (SK_U32)IdTable[Index].Id;
			SK_PNMI_STORE_U32(pBuf + Offset, Val32);
			Offset += sizeof(SK_U32);
		}
		*pLen = Len;
		break;

	case OID_SKGE_BOARDLEVEL:
		Val32 = (SK_U32)pAC->GIni.GILevel;
		SK_PNMI_STORE_U32(pBuf, Val32);
		*pLen = sizeof(SK_U32);
		break;

	case OID_SKGE_PORT_NUMBER:
		Val32 = (SK_U32)pAC->GIni.GIMacsFound;
		SK_PNMI_STORE_U32(pBuf, Val32);
		*pLen = sizeof(SK_U32);
		break;

	case OID_SKGE_DEVICE_TYPE:
		Val32 = (SK_U32)pAC->Pnmi.DeviceType;
		SK_PNMI_STORE_U32(pBuf, Val32);
		*pLen = sizeof(SK_U32);
		break;

	case OID_SKGE_DRIVER_DESCR:
		if (pAC->Pnmi.pDriverDescription == NULL) {

			SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR007, SK_PNMI_ERR007MSG);

			*pLen = 0;
			return (SK_PNMI_ERR_GENERAL);
		}

		Len = SK_STRLEN(pAC->Pnmi.pDriverDescription) + 1;
		if (Len > SK_PNMI_STRINGLEN1) {

			SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR029, SK_PNMI_ERR029MSG);

			*pLen = 0;
			return (SK_PNMI_ERR_GENERAL);
		}

		if (*pLen < Len) {

			*pLen = Len;
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		*pBuf = (char)(Len - 1);
		SK_MEMCPY(pBuf + 1, pAC->Pnmi.pDriverDescription, Len - 1);
		*pLen = Len;
		break;

	case OID_SKGE_DRIVER_VERSION:
		if (pAC->Pnmi.pDriverVersion == NULL) {

			SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR030, SK_PNMI_ERR030MSG);

			*pLen = 0;
			return (SK_PNMI_ERR_GENERAL);
		}

		Len = SK_STRLEN(pAC->Pnmi.pDriverVersion) + 1;
		if (Len > SK_PNMI_STRINGLEN1) {

			SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR031, SK_PNMI_ERR031MSG);

			*pLen = 0;
			return (SK_PNMI_ERR_GENERAL);
		}

		if (*pLen < Len) {

			*pLen = Len;
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		*pBuf = (char)(Len - 1);
		SK_MEMCPY(pBuf + 1, pAC->Pnmi.pDriverVersion, Len - 1);
		*pLen = Len;
		break;

	case OID_SKGE_DRIVER_RELDATE:
		if (pAC->Pnmi.pDriverReleaseDate == NULL) {

			SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR053, SK_PNMI_ERR053MSG);

			*pLen = 0;
			return (SK_PNMI_ERR_GENERAL);
		}

		Len = SK_STRLEN(pAC->Pnmi.pDriverReleaseDate) + 1;
		if (Len > SK_PNMI_STRINGLEN1) {

			SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR054, SK_PNMI_ERR054MSG);

			*pLen = 0;
			return (SK_PNMI_ERR_GENERAL);
		}

		if (*pLen < Len) {

			*pLen = Len;
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		*pBuf = (char)(Len - 1);
		SK_MEMCPY(pBuf + 1, pAC->Pnmi.pDriverReleaseDate, Len - 1);
		*pLen = Len;
		break;

	case OID_SKGE_DRIVER_FILENAME:
		if (pAC->Pnmi.pDriverFileName == NULL) {

			SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR055, SK_PNMI_ERR055MSG);

			*pLen = 0;
			return (SK_PNMI_ERR_GENERAL);
		}

		Len = SK_STRLEN(pAC->Pnmi.pDriverFileName) + 1;
		if (Len > SK_PNMI_STRINGLEN1) {

			SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR056, SK_PNMI_ERR056MSG);

			*pLen = 0;
			return (SK_PNMI_ERR_GENERAL);
		}

		if (*pLen < Len) {

			*pLen = Len;
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		*pBuf = (char)(Len - 1);
		SK_MEMCPY(pBuf + 1, pAC->Pnmi.pDriverFileName, Len - 1);
		*pLen = Len;
		break;

	case OID_SKGE_HW_DESCR:
		/*
		 * The hardware description is located in the VPD.
		 * This query may move to the initialization routine.
		 * But the VPD is cached and therefore a call here
		 * will not make much difference.
		 * Please read comment in Vpd().
		 */
		if (pAC->Pnmi.VpdKeyReadError) {
			return (SK_PNMI_ERR_OK);
		}

		Len = 256;
		if (VpdRead(pAC, IoC, VPD_NAME, Buf, (int *)&Len) > 0) {

			SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR032, SK_PNMI_ERR032MSG);

			*pLen = 0;
			return (SK_PNMI_ERR_GENERAL);
		}
		Len ++;
		if (Len > SK_PNMI_STRINGLEN1) {

			SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR033, SK_PNMI_ERR033MSG);

			*pLen = 0;
			return (SK_PNMI_ERR_GENERAL);
		}
		if (*pLen < Len) {

			*pLen = Len;
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		*pBuf = (char)(Len - 1);
		SK_MEMCPY(pBuf + 1, Buf, Len - 1);
		*pLen = Len;
		break;

	case OID_SKGE_HW_VERSION:
		if (*pLen < 5) {

			*pLen = 5;
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		Val8 = (SK_U8)pAC->GIni.GIPciHwRev;
		pBuf[0] = 4;
		pBuf[1] = 'v';
		pBuf[2] = (char)('0' | ((Val8 >> 4) & 0x0f));
		pBuf[3] = '.';
		pBuf[4] = (char)('0' | (Val8 & 0x0f));
		*pLen = 5;
		break;

	case OID_SKGE_CHIPSET:
		Val16 = pAC->Pnmi.Chipset;
		SK_PNMI_STORE_U16(pBuf, Val16);
		*pLen = sizeof(SK_U16);
		break;

	case OID_SKGE_CHIPID:
		Val32 = pAC->GIni.GIChipId;
		SK_PNMI_STORE_U32(pBuf, Val32);
		*pLen = sizeof(SK_U32);
		break;

	case OID_SKGE_RAMSIZE:
		Val32 = pAC->GIni.GIRamSize;
		SK_PNMI_STORE_U32(pBuf, Val32);
		*pLen = sizeof(SK_U32);
		break;

	case OID_SKGE_VAUXAVAIL:
		*pBuf = (char)pAC->GIni.GIVauxAvail;
		*pLen = sizeof(char);
		break;

	case OID_SKGE_BUS_TYPE:
		*pBuf = (char)SK_PNMI_BUS_PCI;
		*pLen = sizeof(char);
		break;

	case OID_SKGE_BUS_SPEED:
		*pBuf = pAC->Pnmi.PciBusSpeed;
		*pLen = sizeof(char);
		break;

	case OID_SKGE_BUS_WIDTH:
		*pBuf = pAC->Pnmi.PciBusWidth;
		*pLen = sizeof(char);
		break;

	case OID_SKGE_RESULT:
		Val32 = pAC->Pnmi.TestResult;
		SK_PNMI_STORE_U32(pBuf, Val32);
		*pLen = sizeof(SK_U32);
		break;

	case OID_SKGE_SENSOR_NUMBER:
		*pBuf = (char)pAC->I2c.MaxSens;
		*pLen = sizeof(char);
		break;

	case OID_SKGE_CHKSM_NUMBER:
		*pBuf = SKCS_NUM_PROTOCOLS;
		*pLen = sizeof(char);
		break;

	case OID_SKGE_TRAP_NUMBER:
		GetTrapQueueLen(pAC, &Len, &Val);
		Val32 = (SK_U32)Val;
		SK_PNMI_STORE_U32(pBuf, Val32);
		*pLen = sizeof(SK_U32);
		break;

	case OID_SKGE_TRAP:
		GetTrapQueueLen(pAC, &Len, &Val);
		if (*pLen < Len) {

			*pLen = Len;
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		CopyTrapQueue(pAC, pBuf);
		*pLen = Len;
		break;

	case OID_SKGE_RLMT_MONITOR_NUMBER:
		/* Not yet implemented by RLMT, therefore we return zero elements. */
		Val32 = 0;
		SK_PNMI_STORE_U32(pBuf, Val32);
		*pLen = sizeof(SK_U32);
		break;

	case OID_SKGE_TX_SW_QUEUE_LEN:
		/* For XMAC, use the frozen SW counters (BufPort). */
		if (MacType == SK_MAC_XMAC) {
			/* DualNet mode */
			if (pAC->Pnmi.DualNetActiveFlag) {
				Val64 = pAC->Pnmi.BufPort[NetIndex].TxSwQueueLen;
			}
			/* SingleNet mode */
			else {
				Val64 = pAC->Pnmi.BufPort[0].TxSwQueueLen +
					pAC->Pnmi.BufPort[1].TxSwQueueLen;
			}
		}
		else {
			/* DualNet mode */
			if (pAC->Pnmi.DualNetActiveFlag) {
				Val64 = pAC->Pnmi.Port[NetIndex].TxSwQueueLen;
			}
			/* SingleNet mode */
			else {
				Val64 = pAC->Pnmi.Port[0].TxSwQueueLen +
					pAC->Pnmi.Port[1].TxSwQueueLen;
			}
		}
		SK_PNMI_STORE_U64(pBuf, Val64);
		*pLen = sizeof(SK_U64);
		break;

	case OID_SKGE_TX_SW_QUEUE_MAX:
		/* For XMAC, use the frozen SW counters (BufPort). */
		if (MacType == SK_MAC_XMAC) {
			/* DualNet mode */
			if (pAC->Pnmi.DualNetActiveFlag) {
				Val64 = pAC->Pnmi.BufPort[NetIndex].TxSwQueueMax;
			}
			/* SingleNet mode */
			else {
				Val64 = pAC->Pnmi.BufPort[0].TxSwQueueMax +
					pAC->Pnmi.BufPort[1].TxSwQueueMax;
			}
		}
		else {
			/* DualNet mode */
			if (pAC->Pnmi.DualNetActiveFlag) {
				Val64 = pAC->Pnmi.Port[NetIndex].TxSwQueueMax;
			}
			/* SingleNet mode */
			else {
				Val64 = pAC->Pnmi.Port[0].TxSwQueueMax +
					pAC->Pnmi.Port[1].TxSwQueueMax;
			}
		}
		SK_PNMI_STORE_U64(pBuf, Val64);
		*pLen = sizeof(SK_U64);
		break;

	case OID_SKGE_TX_RETRY:
		/* For XMAC, use the frozen SW counters (BufPort). */
		if (MacType == SK_MAC_XMAC) {
			/* DualNet mode */
			if (pAC->Pnmi.DualNetActiveFlag) {
				Val64 = pAC->Pnmi.BufPort[NetIndex].TxRetryCts;
			}
			/* SingleNet mode */
			else {
				Val64 = pAC->Pnmi.BufPort[0].TxRetryCts +
					pAC->Pnmi.BufPort[1].TxRetryCts;
			}
		}
		else {
			/* DualNet mode */
			if (pAC->Pnmi.DualNetActiveFlag) {
				Val64 = pAC->Pnmi.Port[NetIndex].TxRetryCts;
			}
			/* SingleNet mode */
			else {
				Val64 = pAC->Pnmi.Port[0].TxRetryCts +
					pAC->Pnmi.Port[1].TxRetryCts;
			}
		}
		SK_PNMI_STORE_U64(pBuf, Val64);
		*pLen = sizeof(SK_U64);
		break;

	case OID_SKGE_RX_INTR_CTS:
		/* For XMAC, use the frozen SW counters (BufPort). */
		if (MacType == SK_MAC_XMAC) {
			/* DualNet mode */
			if (pAC->Pnmi.DualNetActiveFlag) {
				Val64 = pAC->Pnmi.BufPort[NetIndex].RxIntrCts;
			}
			/* SingleNet mode */
			else {
				Val64 = pAC->Pnmi.BufPort[0].RxIntrCts +
					pAC->Pnmi.BufPort[1].RxIntrCts;
			}
		}
		else {
			/* DualNet mode */
			if (pAC->Pnmi.DualNetActiveFlag) {
				Val64 = pAC->Pnmi.Port[NetIndex].RxIntrCts;
			}
			/* SingleNet mode */
			else {
				Val64 = pAC->Pnmi.Port[0].RxIntrCts +
					pAC->Pnmi.Port[1].RxIntrCts;
			}
		}
		SK_PNMI_STORE_U64(pBuf, Val64);
		*pLen = sizeof(SK_U64);
		break;

	case OID_SKGE_TX_INTR_CTS:
		/* For XMAC, use the frozen SW counters (BufPort). */
		if (MacType == SK_MAC_XMAC) {
			/* DualNet mode */
			if (pAC->Pnmi.DualNetActiveFlag) {
				Val64 = pAC->Pnmi.BufPort[NetIndex].TxIntrCts;
			}
			/* SingleNet mode */
			else {
				Val64 = pAC->Pnmi.BufPort[0].TxIntrCts +
					pAC->Pnmi.BufPort[1].TxIntrCts;
			}
		}
		else {
			/* DualNet mode */
			if (pAC->Pnmi.DualNetActiveFlag) {
				Val64 = pAC->Pnmi.Port[NetIndex].TxIntrCts;
			}
			/* SingleNet mode */
			else {
				Val64 = pAC->Pnmi.Port[0].TxIntrCts +
					pAC->Pnmi.Port[1].TxIntrCts;
			}
		}
		SK_PNMI_STORE_U64(pBuf, Val64);
		*pLen = sizeof(SK_U64);
		break;

	case OID_SKGE_RX_NO_BUF_CTS:
		/* For XMAC, use the frozen SW counters (BufPort). */
		if (MacType == SK_MAC_XMAC) {
			/* DualNet mode */
			if (pAC->Pnmi.DualNetActiveFlag) {
				Val64 = pAC->Pnmi.BufPort[NetIndex].RxNoBufCts;
			}
			/* SingleNet mode */
			else {
				Val64 = pAC->Pnmi.BufPort[0].RxNoBufCts +
					pAC->Pnmi.BufPort[1].RxNoBufCts;
			}
		}
		else {
			/* DualNet mode */
			if (pAC->Pnmi.DualNetActiveFlag) {
				Val64 = pAC->Pnmi.Port[NetIndex].RxNoBufCts;
			}
			/* SingleNet mode */
			else {
				Val64 = pAC->Pnmi.Port[0].RxNoBufCts +
					pAC->Pnmi.Port[1].RxNoBufCts;
			}
		}
		SK_PNMI_STORE_U64(pBuf, Val64);
		*pLen = sizeof(SK_U64);
		break;

	case OID_SKGE_TX_NO_BUF_CTS:
		/* For XMAC, use the frozen SW counters (BufPort). */
		if (MacType == SK_MAC_XMAC) {
			/* DualNet mode */
			if (pAC->Pnmi.DualNetActiveFlag) {
				Val64 = pAC->Pnmi.BufPort[NetIndex].TxNoBufCts;
			}
			/* SingleNet mode */
			else {
				Val64 = pAC->Pnmi.BufPort[0].TxNoBufCts +
					pAC->Pnmi.BufPort[1].TxNoBufCts;
			}
		}
		else {
			/* DualNet mode */
			if (pAC->Pnmi.DualNetActiveFlag) {
				Val64 = pAC->Pnmi.Port[NetIndex].TxNoBufCts;
			}
			/* SingleNet mode */
			else {
				Val64 = pAC->Pnmi.Port[0].TxNoBufCts +
					pAC->Pnmi.Port[1].TxNoBufCts;
			}
		}
		SK_PNMI_STORE_U64(pBuf, Val64);
		*pLen = sizeof(SK_U64);
		break;

	case OID_SKGE_TX_USED_DESCR_NO:
		/* For XMAC, use the frozen SW counters (BufPort). */
		if (MacType == SK_MAC_XMAC) {
			/* DualNet mode */
			if (pAC->Pnmi.DualNetActiveFlag) {
				Val64 = pAC->Pnmi.BufPort[NetIndex].TxUsedDescrNo;
			}
			/* SingleNet mode */
			else {
				Val64 = pAC->Pnmi.BufPort[0].TxUsedDescrNo +
					pAC->Pnmi.BufPort[1].TxUsedDescrNo;
			}
		}
		else {
			/* DualNet mode */
			if (pAC->Pnmi.DualNetActiveFlag) {
				Val64 = pAC->Pnmi.Port[NetIndex].TxUsedDescrNo;
			}
			/* SingleNet mode */
			else {
				Val64 = pAC->Pnmi.Port[0].TxUsedDescrNo +
					pAC->Pnmi.Port[1].TxUsedDescrNo;
			}
		}
		SK_PNMI_STORE_U64(pBuf, Val64);
		*pLen = sizeof(SK_U64);
		break;

	case OID_SKGE_RX_DELIVERED_CTS:
		/* For XMAC, use the frozen SW counters (BufPort). */
		if (MacType == SK_MAC_XMAC) {
			/* DualNet mode */
			if (pAC->Pnmi.DualNetActiveFlag) {
				Val64 = pAC->Pnmi.BufPort[NetIndex].RxDeliveredCts;
			}
			/* SingleNet mode */
			else {
				Val64 = pAC->Pnmi.BufPort[0].RxDeliveredCts +
					pAC->Pnmi.BufPort[1].RxDeliveredCts;
			}
		}
		else {
			/* DualNet mode */
			if (pAC->Pnmi.DualNetActiveFlag) {
				Val64 = pAC->Pnmi.Port[NetIndex].RxDeliveredCts;
			}
			/* SingleNet mode */
			else {
				Val64 = pAC->Pnmi.Port[0].RxDeliveredCts +
					pAC->Pnmi.Port[1].RxDeliveredCts;
			}
		}
		SK_PNMI_STORE_U64(pBuf, Val64);
		*pLen = sizeof(SK_U64);
		break;

	case OID_SKGE_RX_OCTETS_DELIV_CTS:
		/* For XMAC, use the frozen SW counters (BufPort). */
		if (MacType == SK_MAC_XMAC) {
			/* DualNet mode */
			if (pAC->Pnmi.DualNetActiveFlag) {
				Val64 = pAC->Pnmi.BufPort[NetIndex].RxOctetsDeliveredCts;
			}
			/* SingleNet mode */
			else {
				Val64 = pAC->Pnmi.BufPort[0].RxOctetsDeliveredCts +
					pAC->Pnmi.BufPort[1].RxOctetsDeliveredCts;
			}
		}
		else {
			/* DualNet mode */
			if (pAC->Pnmi.DualNetActiveFlag) {
				Val64 = pAC->Pnmi.Port[NetIndex].RxOctetsDeliveredCts;
			}
			/* SingleNet mode */
			else {
				Val64 = pAC->Pnmi.Port[0].RxOctetsDeliveredCts +
					pAC->Pnmi.Port[1].RxOctetsDeliveredCts;
			}
		}
		SK_PNMI_STORE_U64(pBuf, Val64);
		*pLen = sizeof(SK_U64);
		break;

	case OID_SKGE_RX_HW_ERROR_CTS:
		SK_PNMI_STORE_U64(pBuf, Val64RxHwErrs);
		*pLen = sizeof(SK_U64);
		break;

	case OID_SKGE_TX_HW_ERROR_CTS:
		SK_PNMI_STORE_U64(pBuf, Val64TxHwErrs);
		*pLen = sizeof(SK_U64);
		break;

	case OID_SKGE_IN_ERRORS_CTS:
		/* For XMAC, use the frozen SW counters (BufPort). */
		if (MacType == SK_MAC_XMAC) {
			/* DualNet mode */
			if (pAC->Pnmi.DualNetActiveFlag) {
				Val64 = Val64RxHwErrs + pAC->Pnmi.BufPort[NetIndex].RxNoBufCts;
			}
			/* SingleNet mode */
			else {
				Val64 = Val64RxHwErrs +
					pAC->Pnmi.BufPort[0].RxNoBufCts +
					pAC->Pnmi.BufPort[1].RxNoBufCts;
			}
		}
		else {
			/* DualNet mode */
			if (pAC->Pnmi.DualNetActiveFlag) {
				Val64 = Val64RxHwErrs + pAC->Pnmi.Port[NetIndex].RxNoBufCts;
			}
			/* SingleNet mode */
			else {
				Val64 = Val64RxHwErrs +
					pAC->Pnmi.Port[0].RxNoBufCts +
					pAC->Pnmi.Port[1].RxNoBufCts;
			}
		}
		SK_PNMI_STORE_U64(pBuf, Val64);
		*pLen = sizeof(SK_U64);
		break;

	case OID_SKGE_OUT_ERROR_CTS:
		/* For XMAC, use the frozen SW counters (BufPort). */
		if (MacType == SK_MAC_XMAC) {
			/* DualNet mode */
			if (pAC->Pnmi.DualNetActiveFlag) {
				Val64 = Val64TxHwErrs + pAC->Pnmi.BufPort[NetIndex].TxNoBufCts;
			}
			/* SingleNet mode */
			else {
				Val64 = Val64TxHwErrs +
					pAC->Pnmi.BufPort[0].TxNoBufCts +
					pAC->Pnmi.BufPort[1].TxNoBufCts;
			}
		}
		else {
			/* DualNet mode */
			if (pAC->Pnmi.DualNetActiveFlag) {
				Val64 = Val64TxHwErrs + pAC->Pnmi.Port[NetIndex].TxNoBufCts;
			}
			/* SingleNet mode */
			else {
				Val64 = Val64TxHwErrs +
					pAC->Pnmi.Port[0].TxNoBufCts +
					pAC->Pnmi.Port[1].TxNoBufCts;
			}
		}
		SK_PNMI_STORE_U64(pBuf, Val64);
		*pLen = sizeof(SK_U64);
		break;

	case OID_SKGE_ERR_RECOVERY_CTS:
		/* For XMAC, use the frozen SW counters (BufPort). */
		if (MacType == SK_MAC_XMAC) {
			/* DualNet mode */
			if (pAC->Pnmi.DualNetActiveFlag) {
				Val64 = pAC->Pnmi.BufPort[NetIndex].ErrRecoveryCts;
			}
			/* SingleNet mode */
			else {
				Val64 = pAC->Pnmi.BufPort[0].ErrRecoveryCts +
					pAC->Pnmi.BufPort[1].ErrRecoveryCts;
			}
		}
		else {
			/* DualNet mode */
			if (pAC->Pnmi.DualNetActiveFlag) {
				Val64 = pAC->Pnmi.Port[NetIndex].ErrRecoveryCts;
			}
			/* SingleNet mode */
			else {
				Val64 = pAC->Pnmi.Port[0].ErrRecoveryCts +
					pAC->Pnmi.Port[1].ErrRecoveryCts;
			}
		}
		SK_PNMI_STORE_U64(pBuf, Val64);
		*pLen = sizeof(SK_U64);
		break;

	case OID_SKGE_SYSUPTIME:
		Val64 = SK_PNMI_HUNDREDS_SEC(SkOsGetTime(pAC));
		Val64 -= pAC->Pnmi.StartUpTime;
		SK_PNMI_STORE_U64(pBuf, Val64);
		*pLen = sizeof(SK_U64);
		break;

	case OID_SKGE_MDB_VERSION:
		Val32 = SK_PNMI_MDB_VERSION;
		SK_PNMI_STORE_U32(pBuf, Val32);
		*pLen = sizeof(SK_U32);
		break;

	case OID_GEN_RCV_ERROR:
		/* For XMAC, use the frozen SW counters (BufPort). */
		if (MacType == SK_MAC_XMAC) {
			Val64 = Val64RxHwErrs + pAC->Pnmi.BufPort[NetIndex].RxNoBufCts;
		}
		else {
			Val64 = Val64RxHwErrs + pAC->Pnmi.Port[NetIndex].RxNoBufCts;
		}

		/*
		 * By default 32bit values are evaluated.
		 */
		if (!Is64BitReq) {
			Val32 = (SK_U32)Val64;
			SK_PNMI_STORE_U32(pBuf, Val32);
			*pLen = sizeof(SK_U32);
		}
		else {
			SK_PNMI_STORE_U64(pBuf, Val64);
			*pLen = sizeof(SK_U64);
		}
		break;

	case OID_GEN_XMIT_ERROR:
		/* For XMAC, use the frozen SW counters (BufPort). */
		if (MacType == SK_MAC_XMAC) {
			Val64 = Val64TxHwErrs + pAC->Pnmi.BufPort[NetIndex].TxNoBufCts;
		}
		else {
			Val64 = Val64TxHwErrs + pAC->Pnmi.Port[NetIndex].TxNoBufCts;
		}

		/*
		 * By default 32bit values are evaluated.
		 */
		if (!Is64BitReq) {
			Val32 = (SK_U32)Val64;
			SK_PNMI_STORE_U32(pBuf, Val32);
			*pLen = sizeof(SK_U32);
		}
		else {
			SK_PNMI_STORE_U64(pBuf, Val64);
			*pLen = sizeof(SK_U64);
		}
		break;

	case OID_GEN_RCV_NO_BUFFER:
		/* For XMAC, use the frozen SW counters (BufPort). */
		if (MacType == SK_MAC_XMAC) {
			Val64 = pAC->Pnmi.BufPort[NetIndex].RxNoBufCts +
				GetStatVal(pAC, IoC, 0, SK_PNMI_HRX_OVERFLOW, NetIndex);

		}
		else {
			Val64 = pAC->Pnmi.Port[NetIndex].RxNoBufCts +
				GetStatVal(pAC, IoC, 0, SK_PNMI_HRX_OVERFLOW, NetIndex);
		}

		/*
		 * By default 32bit values are evaluated.
		 */
		if (!Is64BitReq) {
			Val32 = (SK_U32)Val64;
			SK_PNMI_STORE_U32(pBuf, Val32);
			*pLen = sizeof(SK_U32);
		}
		else {
			SK_PNMI_STORE_U64(pBuf, Val64);
			*pLen = sizeof(SK_U64);
		}
		break;

	case OID_GEN_TRANSMIT_QUEUE_LENGTH:
		Val32 = (SK_U32)pAC->Pnmi.Port[NetIndex].TxSwQueueLen;
		SK_PNMI_STORE_U32(pBuf, Val32);
		*pLen = sizeof(SK_U32);
		break;

	default:
		SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR034, SK_PNMI_ERR034MSG);

		*pLen = 0;
		return (SK_PNMI_ERR_GENERAL);
	}

	if (Id == OID_SKGE_RX_HW_ERROR_CTS ||
		Id == OID_SKGE_TX_HW_ERROR_CTS ||
		Id == OID_SKGE_IN_ERRORS_CTS ||
		Id == OID_SKGE_OUT_ERROR_CTS ||
		Id == OID_GEN_XMIT_ERROR ||
		Id == OID_GEN_RCV_ERROR) {

		pAC->Pnmi.MacUpdatedFlag --;
	}

	return (SK_PNMI_ERR_OK);
}
#ifndef SK_NO_RLMT
/*****************************************************************************
 *
 * Rlmt - OID handler function of OID_SKGE_RLMT_XXX single instance.
 *
 * Description:
 *	Get/Presets/Sets the RLMT OIDs.
 *
 * Returns:
 *	SK_PNMI_ERR_OK           The request was successfully performed.
 *	SK_PNMI_ERR_GENERAL      A general severe internal error occurred.
 *	SK_PNMI_ERR_TOO_SHORT    The passed buffer is too short to contain
 *	                         the correct data (e.g. a 32bit value is
 *	                         needed, but a 16 bit value was passed).
 *	SK_PNMI_ERR_BAD_VALUE    The passed value is not in the valid
 *	                         value range.
 *	SK_PNMI_ERR_READ_ONLY    The OID is read-only and cannot be set.
 *	SK_PNMI_ERR_UNKNOWN_INST The requested instance of the OID doesn't
 *                           exist (e.g. port instance 3 on a two port
 *	                         adapter.
 */
PNMI_STATIC int Rlmt(
SK_AC *pAC,		/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
int Action,		/* GET/PRESET/SET action */
SK_U32 Id,		/* Object ID that is to be processed */
char *pBuf,		/* Buffer used for the management data transfer */
unsigned int *pLen,	/* On call: pBuf buffer length. On return: used buffer */
SK_U32 Instance,	/* Instance (1..n) that is to be queried or -1 */
unsigned int TableIndex, /* Index to the Id table */
SK_U32 NetIndex)	/* NetIndex (0..n), in single net mode always zero */
{
	int		Ret;
	unsigned int	PhysPortIndex;
	unsigned int	PhysPortMax;
	SK_EVPARA	EventParam;
	SK_U32		Val32;
	SK_U64		Val64;

	/* Check instance. Only single instance OIDs are allowed here. */
	if (Instance != (SK_U32)(-1) && Instance != 1) {

		*pLen = 0;
		return (SK_PNMI_ERR_UNKNOWN_INST);
	}

	/* Perform the requested action. */
	if (Action == SK_PNMI_GET) {

		/* Check if the buffer length is large enough. */
		switch (Id) {

		case OID_SKGE_RLMT_MODE:
		case OID_SKGE_RLMT_PORT_ACTIVE:
		case OID_SKGE_RLMT_PORT_PREFERRED:
			if (*pLen < sizeof(SK_U8)) {

				*pLen = sizeof(SK_U8);
				return (SK_PNMI_ERR_TOO_SHORT);
			}
			break;

		case OID_SKGE_RLMT_PORT_NUMBER:
			if (*pLen < sizeof(SK_U32)) {

				*pLen = sizeof(SK_U32);
				return (SK_PNMI_ERR_TOO_SHORT);
			}
			break;

		case OID_SKGE_RLMT_CHANGE_CTS:
		case OID_SKGE_RLMT_CHANGE_TIME:
		case OID_SKGE_RLMT_CHANGE_ESTIM:
		case OID_SKGE_RLMT_CHANGE_THRES:
			if (*pLen < sizeof(SK_U64)) {

				*pLen = sizeof(SK_U64);
				return (SK_PNMI_ERR_TOO_SHORT);
			}
			break;

		default:
			SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR035, SK_PNMI_ERR035MSG);

			*pLen = 0;
			return (SK_PNMI_ERR_GENERAL);
		}

		/*
		 * Update RLMT statistic and increment semaphores to indicate
		 * that an update was already done. Maybe RLMT will hold its
		 * statistic always up to date some time. Then we can
		 * remove this type of call.
		 */
		if ((Ret = RlmtUpdate(pAC, IoC, NetIndex)) != SK_PNMI_ERR_OK) {

			*pLen = 0;
			return (Ret);
		}
		pAC->Pnmi.RlmtUpdatedFlag ++;

		/* Retrieve value. */
		switch (Id) {

		case OID_SKGE_RLMT_MODE:
			*pBuf = (char)pAC->Rlmt.Net[0].RlmtMode;
			*pLen = sizeof(char);
			break;

		case OID_SKGE_RLMT_PORT_NUMBER:
			Val32 = (SK_U32)pAC->GIni.GIMacsFound;
			SK_PNMI_STORE_U32(pBuf, Val32);
			*pLen = sizeof(SK_U32);
			break;

		case OID_SKGE_RLMT_PORT_ACTIVE:
			*pBuf = 0;
			/*
			 * If multiple ports may become active this OID
			 * doesn't make sense any more. A new variable in
			 * the port structure should be created. However,
			 * for this variable the first active port is
			 * returned.
			 */
			PhysPortMax = pAC->GIni.GIMacsFound;

			for (PhysPortIndex = 0; PhysPortIndex < PhysPortMax;
				PhysPortIndex ++) {

				if (pAC->Pnmi.Port[PhysPortIndex].ActiveFlag) {

					*pBuf = (char)SK_PNMI_PORT_PHYS2LOG(PhysPortIndex);
					break;
				}
			}
			*pLen = sizeof(char);
			break;

		case OID_SKGE_RLMT_PORT_PREFERRED:
			*pBuf = (char)SK_PNMI_PORT_PHYS2LOG(pAC->Rlmt.Net[NetIndex].Preference);
			*pLen = sizeof(char);
			break;

		case OID_SKGE_RLMT_CHANGE_CTS:
			Val64 = pAC->Pnmi.RlmtChangeCts;
			SK_PNMI_STORE_U64(pBuf, Val64);
			*pLen = sizeof(SK_U64);
			break;

		case OID_SKGE_RLMT_CHANGE_TIME:
			Val64 = pAC->Pnmi.RlmtChangeTime;
			SK_PNMI_STORE_U64(pBuf, Val64);
			*pLen = sizeof(SK_U64);
			break;

		case OID_SKGE_RLMT_CHANGE_ESTIM:
			Val64 = pAC->Pnmi.RlmtChangeEstimate.Estimate;
			SK_PNMI_STORE_U64(pBuf, Val64);
			*pLen = sizeof(SK_U64);
			break;

		case OID_SKGE_RLMT_CHANGE_THRES:
			Val64 = pAC->Pnmi.RlmtChangeThreshold;
			SK_PNMI_STORE_U64(pBuf, Val64);
			*pLen = sizeof(SK_U64);
			break;

		default:
			SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_ERR,
				("Rlmt: Unknown OID should be handled before"));

			pAC->Pnmi.RlmtUpdatedFlag --;
			*pLen = 0;
			return (SK_PNMI_ERR_GENERAL);
		}

		pAC->Pnmi.RlmtUpdatedFlag --;
	}
	else {
		/* Perform a PRESET or SET. */
		switch (Id) {

		case OID_SKGE_RLMT_MODE:
			/* Check if the buffer length is plausible. */
			if (*pLen < sizeof(char)) {

				*pLen = sizeof(char);
				return (SK_PNMI_ERR_TOO_SHORT);
			}
			/* Check if the value range is correct. */
			if (*pLen != sizeof(char) ||
				(*pBuf & SK_PNMI_RLMT_MODE_CHK_LINK) == 0 ||
				*(SK_U8 *)pBuf > 15) {

				*pLen = 0;
				return (SK_PNMI_ERR_BAD_VALUE);
			}
			/* The PRESET ends here. */
			if (Action == SK_PNMI_PRESET) {

				*pLen = 0;
				return (SK_PNMI_ERR_OK);
			}
			/* Send an event to RLMT to change the mode. */
			SK_MEMSET((char *)&EventParam, 0, sizeof(EventParam));

			EventParam.Para32[0] |= (SK_U32)(*pBuf);
			EventParam.Para32[1] = 0;
			if (SkRlmtEvent(pAC, IoC, SK_RLMT_MODE_CHANGE,
				EventParam) > 0) {

				SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR037, SK_PNMI_ERR037MSG);

				*pLen = 0;
				return (SK_PNMI_ERR_GENERAL);
			}
			break;

		case OID_SKGE_RLMT_PORT_PREFERRED:
			/* PRESET/SET action makes no sense in Dual Net mode */
			if (pAC->Pnmi.DualNetActiveFlag) {
				break;
			}

			/* Check if the buffer length is plausible. */
			if (*pLen < sizeof(char)) {

				*pLen = sizeof(char);
				return (SK_PNMI_ERR_TOO_SHORT);
			}
			/* Check if the value range is correct. */
			if (*pLen != sizeof(char) || *(SK_U8 *)pBuf >
				(SK_U8)pAC->GIni.GIMacsFound) {

				*pLen = 0;
				return (SK_PNMI_ERR_BAD_VALUE);
			}
			/* The PRESET ends here. */
			if (Action == SK_PNMI_PRESET) {

				*pLen = 0;
				return (SK_PNMI_ERR_OK);
			}

			/*
			 * Send an event to RLMT change the preferred port.
			 * A param of -1 means automatic mode. RLMT will
			 * make the decision which is the preferred port.
			 */
			SK_MEMSET((char *)&EventParam, 0, sizeof(EventParam));

			EventParam.Para32[0] = (SK_U32)(*pBuf) - 1;
			EventParam.Para32[1] = NetIndex;
			if (SkRlmtEvent(pAC, IoC, SK_RLMT_PREFPORT_CHANGE,
				EventParam) > 0) {

				SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR038, SK_PNMI_ERR038MSG);

				*pLen = 0;
				return (SK_PNMI_ERR_GENERAL);
			}
			break;

		case OID_SKGE_RLMT_CHANGE_THRES:
			/* Check if the buffer length is plausible. */
			if (*pLen < sizeof(SK_U64)) {

				*pLen = sizeof(SK_U64);
				return (SK_PNMI_ERR_TOO_SHORT);
			}

			/* There are not many restrictions to the value range. */
			if (*pLen != sizeof(SK_U64)) {

				*pLen = 0;
				return (SK_PNMI_ERR_BAD_VALUE);
			}
			/* The PRESET ends here. */
			if (Action == SK_PNMI_PRESET) {

				*pLen = 0;
				return (SK_PNMI_ERR_OK);
			}
			/*
			 * Store the new threshold, which will be taken
			 * on the next timer event.
			 */
			SK_PNMI_READ_U64(pBuf, Val64);
			pAC->Pnmi.RlmtChangeThreshold = Val64;
			break;

		default:
			/* The other OIDs are not be able for set. */
			*pLen = 0;
			return (SK_PNMI_ERR_READ_ONLY);
		}
	}

	return (SK_PNMI_ERR_OK);
}

/*****************************************************************************
 *
 * RlmtStat - OID handler function of OID_SKGE_RLMT_XXX multiple instance.
 *
 * Description:
 *	Performs get requests on multiple instance variables.
 *
 * Returns:
 *	SK_PNMI_ERR_OK           The request was successfully performed.
 *	SK_PNMI_ERR_GENERAL      A general severe internal error occurred.
 *	SK_PNMI_ERR_TOO_SHORT    The passed buffer is too short to contain
 *	                         the correct data (e.g. a 32bit value is
 *	                         needed, but a 16 bit value was passed).
 *	SK_PNMI_ERR_UNKNOWN_INST The requested instance of the OID doesn't
 *                           exist (e.g. port instance 3 on a two port
 *	                         adapter.
 */
PNMI_STATIC int RlmtStat(
SK_AC *pAC,		/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
int Action,		/* GET/PRESET/SET action */
SK_U32 Id,		/* Object ID that is to be processed */
char *pBuf,		/* Buffer used for the management data transfer */
unsigned int *pLen,	/* On call: pBuf buffer length. On return: used buffer */
SK_U32 Instance,	/* Instance (1..n) that is to be queried or -1 */
unsigned int TableIndex, /* Index to the Id table */
SK_U32 NetIndex)	/* NetIndex (0..n), in single net mode always zero */
{
	unsigned int	PhysPortMax;
	unsigned int	PhysPortIndex;
	unsigned int	Limit;
	unsigned int	Offset;
	int		Ret;
	SK_U32		Val32;
	SK_U64		Val64;

	/* Calculate the port indexes from the instance. */
	PhysPortMax = pAC->GIni.GIMacsFound;

	if ((Instance != (SK_U32)(-1))) {
		/* Check instance range. */
		if ((Instance < 1) || (Instance > PhysPortMax)) {

			*pLen = 0;
			return (SK_PNMI_ERR_UNKNOWN_INST);
		}

		/* SingleNet mode */
		PhysPortIndex = Instance - 1;

		/* DualNet mode */
		if (pAC->Pnmi.DualNetActiveFlag) {
			PhysPortIndex = NetIndex;
		}

		/* Both net modes. */
		Limit = PhysPortIndex + 1;
	}
	else {
		/* SingleNet mode */
		PhysPortIndex = 0;
		Limit = PhysPortMax;

		/* DualNet mode */
		if (pAC->Pnmi.DualNetActiveFlag) {
			PhysPortIndex = NetIndex;
			Limit = PhysPortIndex + 1;
		}
	}

	/* Currently only GET requests are allowed. */
	if (Action != SK_PNMI_GET) {

		*pLen = 0;
		return (SK_PNMI_ERR_READ_ONLY);
	}

	/* Check if the buffer length is large enough. */
	switch (Id) {

	case OID_SKGE_RLMT_PORT_INDEX:
	case OID_SKGE_RLMT_STATUS:
		if (*pLen < (Limit - PhysPortIndex) * sizeof(SK_U32)) {

			*pLen = (Limit - PhysPortIndex) * sizeof(SK_U32);
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		break;

	case OID_SKGE_RLMT_TX_HELLO_CTS:
	case OID_SKGE_RLMT_RX_HELLO_CTS:
	case OID_SKGE_RLMT_TX_SP_REQ_CTS:
	case OID_SKGE_RLMT_RX_SP_CTS:
		if (*pLen < (Limit - PhysPortIndex) * sizeof(SK_U64)) {

			*pLen = (Limit - PhysPortIndex) * sizeof(SK_U64);
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		break;

	default:
		SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR039, SK_PNMI_ERR039MSG);

		*pLen = 0;
		return (SK_PNMI_ERR_GENERAL);

	}

	/*
	 * Update statistic and increment semaphores to indicate that
	 * an update was already done.
	 */
	if ((Ret = RlmtUpdate(pAC, IoC, NetIndex)) != SK_PNMI_ERR_OK) {

		*pLen = 0;
		return (Ret);
	}
	pAC->Pnmi.RlmtUpdatedFlag ++;

	/* Get value. */
	Offset = 0;
	for (; PhysPortIndex < Limit; PhysPortIndex ++) {

		switch (Id) {

		case OID_SKGE_RLMT_PORT_INDEX:
			Val32 = PhysPortIndex;
			SK_PNMI_STORE_U32(pBuf + Offset, Val32);
			Offset += sizeof(SK_U32);
			break;

		case OID_SKGE_RLMT_STATUS:
			if (pAC->Rlmt.Port[PhysPortIndex].PortState ==
				SK_RLMT_PS_INIT ||
				pAC->Rlmt.Port[PhysPortIndex].PortState ==
				SK_RLMT_PS_DOWN) {

				Val32 = SK_PNMI_RLMT_STATUS_ERROR;
			}
			else if (pAC->Pnmi.Port[PhysPortIndex].ActiveFlag) {

				Val32 = SK_PNMI_RLMT_STATUS_ACTIVE;
			}
			else {
				Val32 = SK_PNMI_RLMT_STATUS_STANDBY;
			}
			SK_PNMI_STORE_U32(pBuf + Offset, Val32);
			Offset += sizeof(SK_U32);
			break;

		case OID_SKGE_RLMT_TX_HELLO_CTS:
			Val64 = pAC->Rlmt.Port[PhysPortIndex].TxHelloCts;
			SK_PNMI_STORE_U64(pBuf + Offset, Val64);
			Offset += sizeof(SK_U64);
			break;

		case OID_SKGE_RLMT_RX_HELLO_CTS:
			Val64 = pAC->Rlmt.Port[PhysPortIndex].RxHelloCts;
			SK_PNMI_STORE_U64(pBuf + Offset, Val64);
			Offset += sizeof(SK_U64);
			break;

		case OID_SKGE_RLMT_TX_SP_REQ_CTS:
			Val64 = pAC->Rlmt.Port[PhysPortIndex].TxSpHelloReqCts;
			SK_PNMI_STORE_U64(pBuf + Offset, Val64);
			Offset += sizeof(SK_U64);
			break;

		case OID_SKGE_RLMT_RX_SP_CTS:
			Val64 = pAC->Rlmt.Port[PhysPortIndex].RxSpHelloCts;
			SK_PNMI_STORE_U64(pBuf + Offset, Val64);
			Offset += sizeof(SK_U64);
			break;

		default:
			SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_ERR,
				("RlmtStat: Unknown OID should be errored before"));

			pAC->Pnmi.RlmtUpdatedFlag --;
			*pLen = 0;
			return (SK_PNMI_ERR_GENERAL);
		}
	}
	*pLen = Offset;

	pAC->Pnmi.RlmtUpdatedFlag --;

	return (SK_PNMI_ERR_OK);
}

/*****************************************************************************
 *
 * Monitor - OID handler function for RLMT_MONITOR_XXX
 *
 * Description:
 *	Because RLMT currently does not support the monitoring of
 *	remote adapter cards, we return always an empty table.
 *
 * Returns:
 *	SK_PNMI_ERR_OK           The request was successfully performed.
 *	SK_PNMI_ERR_GENERAL      A general severe internal error occured.
 *	SK_PNMI_ERR_TOO_SHORT    The passed buffer is too short to contain
 *	                         the correct data (e.g. a 32bit value is
 *	                         needed, but a 16 bit value was passed).
 *	SK_PNMI_ERR_BAD_VALUE    The passed value is not in the valid
 *	                         value range.
 *	SK_PNMI_ERR_READ_ONLY    The OID is read-only and cannot be set.
 *	SK_PNMI_ERR_UNKNOWN_INST The requested instance of the OID doesn't
 *                           exist (e.g. port instance 3 on a two port
 *	                         adapter.
 */
PNMI_STATIC int Monitor(
SK_AC *pAC,		/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
int Action,		/* GET/PRESET/SET action */
SK_U32 Id,		/* Object ID that is to be processed */
char *pBuf,		/* Buffer used for the management data transfer */
unsigned int *pLen,	/* On call: pBuf buffer length. On return: used buffer */
SK_U32 Instance,	/* Instance (1..n) that is to be queried or -1 */
unsigned int TableIndex, /* Index to the Id table */
SK_U32 NetIndex)	/* NetIndex (0..n), in single net mode always zero */
{
	unsigned int	Index;
	unsigned int	Limit;
	unsigned int	Offset;
	unsigned int	Entries;

	/* Not implemented yet. Return always an empty table. */
	Entries = 0;

	/* Calculate instance if wished. */
	if ((Instance != (SK_U32)(-1))) {

		if ((Instance < 1) || (Instance > Entries)) {

			*pLen = 0;
			return (SK_PNMI_ERR_UNKNOWN_INST);
		}

		Index = (unsigned int)Instance - 1;
		Limit = (unsigned int)Instance;
	}
	else {
		Index = 0;
		Limit = Entries;
	}

	/* GET/SET value. */
	if (Action == SK_PNMI_GET) {

		for (Offset = 0; Index < Limit; Index ++) {

			switch (Id) {

			case OID_SKGE_RLMT_MONITOR_INDEX:
			case OID_SKGE_RLMT_MONITOR_ADDR:
			case OID_SKGE_RLMT_MONITOR_ERRS:
			case OID_SKGE_RLMT_MONITOR_TIMESTAMP:
			case OID_SKGE_RLMT_MONITOR_ADMIN:
				break;

			default:
				SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR046, SK_PNMI_ERR046MSG);

				*pLen = 0;
				return (SK_PNMI_ERR_GENERAL);
			}
		}
		*pLen = Offset;
	}
	else {
		/* Only MONITOR_ADMIN can be set. */
		if (Id != OID_SKGE_RLMT_MONITOR_ADMIN) {

			*pLen = 0;
			return (SK_PNMI_ERR_READ_ONLY);
		}

		/* Check if the length is plausible. */
		if (*pLen < (Limit - Index)) {

			return (SK_PNMI_ERR_TOO_SHORT);
		}
		/* Okay, we have a wide value range. */
		if (*pLen != (Limit - Index)) {

			*pLen = 0;
			return (SK_PNMI_ERR_BAD_VALUE);
		}

		/*
		 * Not yet implemented. Return always BAD_VALUE,
		 * because the table is empty.
		 */
		*pLen = 0;
		return (SK_PNMI_ERR_BAD_VALUE);
	}

	return (SK_PNMI_ERR_OK);
}
#endif
/*****************************************************************************
 *
 * MacPrivateConf - OID handler function of OIDs concerning the configuration
 *
 * Description:
 *	Get/Presets/Sets the OIDs concerning the configuration.
 *
 * Returns:
 *	SK_PNMI_ERR_OK           The request was successfully performed.
 *	SK_PNMI_ERR_GENERAL      A general severe internal error occurred.
 *	SK_PNMI_ERR_TOO_SHORT    The passed buffer is too short to contain
 *	                         the correct data (e.g. a 32bit value is
 *	                         needed, but a 16 bit value was passed).
 *	SK_PNMI_ERR_BAD_VALUE    The passed value is not in the valid
 *	                         value range.
 *	SK_PNMI_ERR_READ_ONLY    The OID is read-only and cannot be set.
 *	SK_PNMI_ERR_UNKNOWN_INST The requested instance of the OID doesn't
 *                           exist (e.g. port instance 3 on a two port
 *	                         adapter.
 */
PNMI_STATIC int MacPrivateConf(
SK_AC *pAC,		/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
int Action,		/* GET/PRESET/SET action */
SK_U32 Id,		/* Object ID that is to be processed */
char *pBuf,		/* Buffer used for the management data transfer */
unsigned int *pLen,	/* On call: pBuf buffer length. On return: used buffer */
SK_U32 Instance,	/* Instance (1..n) that is to be queried or -1 */
unsigned int TableIndex, /* Index to the Id table */
SK_U32 NetIndex)	/* NetIndex (0..n), in single net mode always zero */
{
	unsigned int	PhysPortMax;
	unsigned int	PhysPortIndex;
	unsigned int	LogPortMax;
	unsigned int	LogPortIndex;
	unsigned int	Limit;
	unsigned int	Offset;
	char		Val8;
	char 		*pBufPtr;
	int			Ret;
	SK_EVPARA	EventParam;
	SK_U32		Val32;
#ifdef SK_PHY_LP_MODE
	SK_U8	CurrentPhyPowerState;
#endif /* SK_PHY_LP_MODE */

	/* Calculate instance if wished. MAC index 0 is the virtual MAC. */
	PhysPortMax = pAC->GIni.GIMacsFound;
	LogPortMax = SK_PNMI_PORT_PHYS2LOG(PhysPortMax);

	if (pAC->Pnmi.DualNetActiveFlag) { /* DualNet mode */
		LogPortMax--;
	}

	if ((Instance != (SK_U32)(-1))) { /* Only one specific instance is queried. */
		/* Check instance range. */
		if ((Instance < 1) || (Instance > LogPortMax)) {

			*pLen = 0;
			return (SK_PNMI_ERR_UNKNOWN_INST);
		}
		LogPortIndex = SK_PNMI_PORT_INST2LOG(Instance);
		Limit = LogPortIndex + 1;
	}

	else { /* Instance == (SK_U32)(-1), get all Instances of that OID. */

		LogPortIndex = 0;
		Limit = LogPortMax;
	}

	/* Perform action. */
	if (Action == SK_PNMI_GET) {

		/* Check length. */
		switch (Id) {

		case OID_SKGE_PMD:
		case OID_SKGE_CONNECTOR:
		case OID_SKGE_LINK_CAP:
		case OID_SKGE_LINK_MODE:
		case OID_SKGE_LINK_MODE_STATUS:
		case OID_SKGE_LINK_STATUS:
		case OID_SKGE_FLOWCTRL_CAP:
		case OID_SKGE_FLOWCTRL_MODE:
		case OID_SKGE_FLOWCTRL_STATUS:
		case OID_SKGE_PHY_OPERATION_CAP:
		case OID_SKGE_PHY_OPERATION_MODE:
		case OID_SKGE_PHY_OPERATION_STATUS:
		case OID_SKGE_SPEED_CAP:
		case OID_SKGE_SPEED_MODE:
		case OID_SKGE_SPEED_STATUS:
#ifdef SK_PHY_LP_MODE
		case OID_SKGE_PHY_LP_MODE:
#endif
			if (*pLen < (Limit - LogPortIndex) * sizeof(SK_U8)) {

				*pLen = (Limit - LogPortIndex) * sizeof(SK_U8);
				return (SK_PNMI_ERR_TOO_SHORT);
			}
			break;

		case OID_SKGE_MTU:
		case OID_SKGE_PHY_TYPE:
			if (*pLen < (Limit - LogPortIndex) * sizeof(SK_U32)) {

				*pLen = (Limit - LogPortIndex) * sizeof(SK_U32);
				return (SK_PNMI_ERR_TOO_SHORT);
			}
			break;

		default:
			SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR041, SK_PNMI_ERR041MSG);
			*pLen = 0;
			return (SK_PNMI_ERR_GENERAL);
		}

		/*
		 * Update statistic and increment semaphore to indicate
		 * that an update was already done.
		 */
		if ((Ret = SirqUpdate(pAC, IoC)) != SK_PNMI_ERR_OK) {

			*pLen = 0;
			return (Ret);
		}
		pAC->Pnmi.SirqUpdatedFlag ++;

		/* Get value. */
		Offset = 0;
		for (; LogPortIndex < Limit; LogPortIndex ++) {

			pBufPtr = pBuf + Offset;

			switch (Id) {

			case OID_SKGE_PMD:
				*pBufPtr = pAC->Pnmi.PMD;
				Offset++;
				break;

			case OID_SKGE_CONNECTOR:
				*pBufPtr = pAC->Pnmi.Connector;
				Offset++;
				break;

			case OID_SKGE_PHY_TYPE:
				if (!pAC->Pnmi.DualNetActiveFlag) { /* SingleNet mode */
					if (LogPortIndex == 0) {
						continue;
					}
					/* Get value for physical port. */
					PhysPortIndex = SK_PNMI_PORT_LOG2PHYS(pAC, LogPortIndex);
					Val32 = pAC->GIni.GP[PhysPortIndex].PhyType;
				}
				else { /* DualNet mode */

					Val32 = pAC->GIni.GP[NetIndex].PhyType;
				}
				SK_PNMI_STORE_U32(pBufPtr, Val32);
				Offset += sizeof(SK_U32);
				break;

#ifdef SK_PHY_LP_MODE
			case OID_SKGE_PHY_LP_MODE:
				if (!pAC->Pnmi.DualNetActiveFlag) { /* SingleNet mode */
					if (LogPortIndex == 0) {
						continue;
					}
					/* Get value for physical port. */
					PhysPortIndex = SK_PNMI_PORT_LOG2PHYS(pAC, LogPortIndex);
					*pBufPtr = (SK_U8)pAC->GIni.GP[PhysPortIndex].PPhyPowerState;
				}
				else { /* DualNet mode */

					*pBufPtr = (SK_U8)pAC->GIni.GP[NetIndex].PPhyPowerState;
				}
				Offset += sizeof(SK_U8);
				break;
#endif

			case OID_SKGE_LINK_CAP:
				if (!pAC->Pnmi.DualNetActiveFlag) { /* SingleNet mode */
					if (LogPortIndex == 0) {
						/* Get value for virtual port. */
						VirtualConf(pAC, IoC, Id, pBufPtr);
					}
					else {
						/* Get value for physical port. */
						PhysPortIndex = SK_PNMI_PORT_LOG2PHYS(
							pAC, LogPortIndex);

						*pBufPtr = pAC->GIni.GP[PhysPortIndex].PLinkCap;
					}
				}
				else { /* DualNet mode */

					*pBufPtr = pAC->GIni.GP[NetIndex].PLinkCap;
				}
				Offset++;
				break;

			case OID_SKGE_LINK_MODE:
				if (!pAC->Pnmi.DualNetActiveFlag) { /* SingleNet mode */
					if (LogPortIndex == 0) {
						/* Get value for virtual port. */
						VirtualConf(pAC, IoC, Id, pBufPtr);
					}
					else {
						/* Get value for physical port. */
						PhysPortIndex = SK_PNMI_PORT_LOG2PHYS(
							pAC, LogPortIndex);

						*pBufPtr = pAC->GIni.GP[PhysPortIndex].PLinkModeConf;
					}
				}
				else { /* DualNet mode */

					*pBufPtr = pAC->GIni.GP[NetIndex].PLinkModeConf;
				}
				Offset++;
				break;

			case OID_SKGE_LINK_MODE_STATUS:
				if (!pAC->Pnmi.DualNetActiveFlag) { /* SingleNet mode */
					if (LogPortIndex == 0) {
						/* Get value for virtual port. */
						VirtualConf(pAC, IoC, Id, pBufPtr);
					}
					else {
						/* Get value for physical port. */
						PhysPortIndex = SK_PNMI_PORT_LOG2PHYS(
							pAC, LogPortIndex);

						*pBufPtr =
							CalculateLinkModeStatus(pAC, IoC, PhysPortIndex);
					}
				}
				else { /* DualNet mode */

					*pBufPtr = CalculateLinkModeStatus(pAC, IoC, NetIndex);
				}
				Offset++;
				break;

			case OID_SKGE_LINK_STATUS:
				if (!pAC->Pnmi.DualNetActiveFlag) { /* SingleNet mode */
					if (LogPortIndex == 0) {
						/* Get value for virtual port. */
						VirtualConf(pAC, IoC, Id, pBufPtr);
					}
					else {
						/* Get value for physical port. */
						PhysPortIndex = SK_PNMI_PORT_LOG2PHYS(
							pAC, LogPortIndex);

						*pBufPtr = CalculateLinkStatus(pAC, IoC, PhysPortIndex);
					}
				}
				else { /* DualNet mode */

					*pBufPtr = CalculateLinkStatus(pAC, IoC, NetIndex);
				}
				Offset++;
				break;

			case OID_SKGE_FLOWCTRL_CAP:
				if (!pAC->Pnmi.DualNetActiveFlag) { /* SingleNet mode */
					if (LogPortIndex == 0) {
						/* Get value for virtual port. */
						VirtualConf(pAC, IoC, Id, pBufPtr);
					}
					else {
						/* Get value for physical port. */
						PhysPortIndex = SK_PNMI_PORT_LOG2PHYS(
							pAC, LogPortIndex);

						*pBufPtr = pAC->GIni.GP[PhysPortIndex].PFlowCtrlCap;
					}
				}
				else { /* DualNet mode */

					*pBufPtr = pAC->GIni.GP[NetIndex].PFlowCtrlCap;
				}
				Offset++;
				break;

			case OID_SKGE_FLOWCTRL_MODE:
				if (!pAC->Pnmi.DualNetActiveFlag) { /* SingleNet mode */
					if (LogPortIndex == 0) {
						/* Get value for virtual port. */
						VirtualConf(pAC, IoC, Id, pBufPtr);
					}
					else {
						/* Get value for physical port. */
						PhysPortIndex = SK_PNMI_PORT_LOG2PHYS(
							pAC, LogPortIndex);

						*pBufPtr = pAC->GIni.GP[PhysPortIndex].PFlowCtrlMode;
					}
				}
				else { /* DualNet mode */

					*pBufPtr = pAC->GIni.GP[NetIndex].PFlowCtrlMode;
				}
				Offset++;
				break;

			case OID_SKGE_FLOWCTRL_STATUS:
				if (!pAC->Pnmi.DualNetActiveFlag) { /* SingleNet mode */
					if (LogPortIndex == 0) {
						/* Get value for virtual port. */
						VirtualConf(pAC, IoC, Id, pBufPtr);
					}
					else {
						/* Get value for physical port. */
						PhysPortIndex = SK_PNMI_PORT_LOG2PHYS(
							pAC, LogPortIndex);

						*pBufPtr = pAC->GIni.GP[PhysPortIndex].PFlowCtrlStatus;
					}
				}
				else { /* DualNet mode */

					*pBufPtr = pAC->GIni.GP[NetIndex].PFlowCtrlStatus;
				}
				Offset++;
				break;

			case OID_SKGE_PHY_OPERATION_CAP:
				if (!pAC->Pnmi.DualNetActiveFlag) { /* SingleNet Mode */
					if (LogPortIndex == 0) {
						/* Get value for virtual port. */
						VirtualConf(pAC, IoC, Id, pBufPtr);
					}
					else {
						/* Get value for physical port. */
						PhysPortIndex = SK_PNMI_PORT_LOG2PHYS(
							pAC, LogPortIndex);

						*pBufPtr = pAC->GIni.GP[PhysPortIndex].PMSCap;
					}
				}
				else { /* DualNet mode */

					*pBufPtr = pAC->GIni.GP[NetIndex].PMSCap;
				}
				Offset++;
				break;

			case OID_SKGE_PHY_OPERATION_MODE:
				if (!pAC->Pnmi.DualNetActiveFlag) { /* SingleNet mode */
					if (LogPortIndex == 0) {
						/* Get value for virtual port. */
						VirtualConf(pAC, IoC, Id, pBufPtr);
					}
					else {
						/* Get value for physical port. */
						PhysPortIndex = SK_PNMI_PORT_LOG2PHYS(
							pAC, LogPortIndex);

						*pBufPtr = pAC->GIni.GP[PhysPortIndex].PMSMode;
					}
				}
				else { /* DualNet mode */

					*pBufPtr = pAC->GIni.GP[NetIndex].PMSMode;
				}
				Offset++;
				break;

			case OID_SKGE_PHY_OPERATION_STATUS:
				if (!pAC->Pnmi.DualNetActiveFlag) { /* SingleNet mode */
					if (LogPortIndex == 0) {
						/* Get value for virtual port. */
						VirtualConf(pAC, IoC, Id, pBufPtr);
					}
					else {
						/* Get value for physical port. */
						PhysPortIndex = SK_PNMI_PORT_LOG2PHYS(
							pAC, LogPortIndex);

						*pBufPtr = pAC->GIni.GP[PhysPortIndex].PMSStatus;
					}
				}
				else {

					*pBufPtr = pAC->GIni.GP[NetIndex].PMSStatus;
				}
				Offset++;
				break;

			case OID_SKGE_SPEED_CAP:
				if (!pAC->Pnmi.DualNetActiveFlag) { /* SingleNet mode */
					if (LogPortIndex == 0) {
						/* Get value for virtual port. */
						VirtualConf(pAC, IoC, Id, pBufPtr);
					}
					else {
						/* Get value for physical port. */
						PhysPortIndex = SK_PNMI_PORT_LOG2PHYS(
							pAC, LogPortIndex);

						*pBufPtr = pAC->GIni.GP[PhysPortIndex].PLinkSpeedCap;
					}
				}
				else { /* DualNet mode */

					*pBufPtr = pAC->GIni.GP[NetIndex].PLinkSpeedCap;
				}
				Offset++;
				break;

			case OID_SKGE_SPEED_MODE:
				if (!pAC->Pnmi.DualNetActiveFlag) { /* SingleNet mode */
					if (LogPortIndex == 0) {
						/* Get value for virtual port. */
						VirtualConf(pAC, IoC, Id, pBufPtr);
					}
					else {
						/* Get value for physical port. */
						PhysPortIndex = SK_PNMI_PORT_LOG2PHYS(
							pAC, LogPortIndex);

						*pBufPtr = pAC->GIni.GP[PhysPortIndex].PLinkSpeed;
					}
				}
				else { /* DualNet mode */

					*pBufPtr = pAC->GIni.GP[NetIndex].PLinkSpeed;
				}
				Offset++;
				break;

			case OID_SKGE_SPEED_STATUS:
				if (!pAC->Pnmi.DualNetActiveFlag) { /* SingleNet mode */
					if (LogPortIndex == 0) {
						/* Get value for virtual port. */
						VirtualConf(pAC, IoC, Id, pBufPtr);
					}
					else {
						/* Get value for physical port. */
						PhysPortIndex = SK_PNMI_PORT_LOG2PHYS(
							pAC, LogPortIndex);

						*pBufPtr = pAC->GIni.GP[PhysPortIndex].PLinkSpeedUsed;
					}
				}
				else { /* DualNet mode */

					*pBufPtr = pAC->GIni.GP[NetIndex].PLinkSpeedUsed;
				}
				Offset++;
				break;

			case OID_SKGE_MTU:
				Val32 = SK_DRIVER_GET_MTU(pAC, IoC, NetIndex);
				SK_PNMI_STORE_U32(pBufPtr, Val32);
				Offset += sizeof(SK_U32);
				break;

			default:
				SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_ERR,
					("MacPrivateConf: Unknown OID should be handled before"));

				pAC->Pnmi.SirqUpdatedFlag --;
				return (SK_PNMI_ERR_GENERAL);
			}
		}
		*pLen = Offset;
		pAC->Pnmi.SirqUpdatedFlag --;

		return (SK_PNMI_ERR_OK);
	}

	/*
	 * From here SET or PRESET action. Check if the passed
	 * buffer length is plausible.
	 */
	switch (Id) {

	case OID_SKGE_LINK_MODE:
	case OID_SKGE_FLOWCTRL_MODE:
	case OID_SKGE_PHY_OPERATION_MODE:
	case OID_SKGE_SPEED_MODE:
		if (*pLen < Limit - LogPortIndex) {

			*pLen = Limit - LogPortIndex;
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		if (*pLen != Limit - LogPortIndex) {

			*pLen = 0;
			return (SK_PNMI_ERR_BAD_VALUE);
		}
		break;

#ifdef SK_PHY_LP_MODE
	case OID_SKGE_PHY_LP_MODE:
		if (*pLen < Limit - LogPortIndex) {

			*pLen = Limit - LogPortIndex;
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		break;
#endif /* SK_PHY_LP_MODE */

	case OID_SKGE_MTU:
		if (*pLen < (Limit - LogPortIndex) * sizeof(SK_U32)) {

			*pLen = (Limit - LogPortIndex) * sizeof(SK_U32);
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		break;

	default:
		*pLen = 0;
		return (SK_PNMI_ERR_READ_ONLY);
	}

	/* Perform PRESET or SET. */
	Offset = 0;
	for (; LogPortIndex < Limit; LogPortIndex ++) {

		Val8 = *(pBuf + Offset);

		switch (Id) {

		case OID_SKGE_LINK_MODE:
			/* Check the value range. */
			if (Val8 == 0) {
				Offset++;
				break;
			}
			if (Val8 < SK_LMODE_HALF ||
				(LogPortIndex != 0 && Val8 > SK_LMODE_AUTOSENSE) ||
				(LogPortIndex == 0 && Val8 > SK_LMODE_INDETERMINATED)) {

				*pLen = 0;
				return (SK_PNMI_ERR_BAD_VALUE);
			}

			/* The PRESET ends here. */
			if (Action == SK_PNMI_PRESET) {

				return (SK_PNMI_ERR_OK);
			}

			if (!pAC->Pnmi.DualNetActiveFlag) { /* SingleNet mode */
				if (LogPortIndex == 0) {
					/*
					 * The virtual port consists of all currently active ports.
					 * Find them and send an event with the new link mode to SIRQ.
					 */
					for (PhysPortIndex = 0; PhysPortIndex < PhysPortMax;
						PhysPortIndex ++) {

						if (!pAC->Pnmi.Port[PhysPortIndex].ActiveFlag) {
							continue;
						}

						EventParam.Para32[0] = PhysPortIndex;
						EventParam.Para32[1] = (SK_U32)Val8;
						if (SkGeSirqEvent(pAC, IoC, SK_HWEV_SET_LMODE,
							EventParam) > 0) {

							SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR043,
								SK_PNMI_ERR043MSG);

							*pLen = 0;
							return (SK_PNMI_ERR_GENERAL);
						}
					} /* for */
				}
				else {
					/* Send an event with the new link mode to SIRQ */
					EventParam.Para32[0] = SK_PNMI_PORT_LOG2PHYS(
						pAC, LogPortIndex);
					EventParam.Para32[1] = (SK_U32)Val8;
					if (SkGeSirqEvent(pAC, IoC, SK_HWEV_SET_LMODE,
						EventParam) > 0) {

						SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR043,
							SK_PNMI_ERR043MSG);

						*pLen = 0;
						return (SK_PNMI_ERR_GENERAL);
					}
				}
			}
			else { /* DualNet mode */

				/* Send an event with the new link mode to SIRQ */
				EventParam.Para32[0] = NetIndex;
				EventParam.Para32[1] = (SK_U32)Val8;
				if (SkGeSirqEvent(pAC, IoC, SK_HWEV_SET_LMODE,
					EventParam) > 0) {

					SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR043,
						SK_PNMI_ERR043MSG);

					*pLen = 0;
					return (SK_PNMI_ERR_GENERAL);
				}
			}
			Offset++;
			break;

		case OID_SKGE_FLOWCTRL_MODE:
			/* Check the value range. */
			if (Val8 == 0) {
				Offset++;
				break;
			}
			if (Val8 < SK_FLOW_MODE_NONE ||
				(LogPortIndex != 0 && Val8 > SK_FLOW_MODE_SYM_OR_REM) ||
				(LogPortIndex == 0 && Val8 > SK_FLOW_MODE_INDETERMINATED)) {

				*pLen = 0;
				return (SK_PNMI_ERR_BAD_VALUE);
			}

			/* The PRESET ends here. */
			if (Action == SK_PNMI_PRESET) {

				return (SK_PNMI_ERR_OK);
			}

			if (!pAC->Pnmi.DualNetActiveFlag) { /* SingleNet mode */
				if (LogPortIndex == 0) {
					/*
					 * The virtual port consists of all currently active ports.
					 * Find them and send an event with the new flow control mode to SIRQ.
					 */
					for (PhysPortIndex = 0; PhysPortIndex < PhysPortMax;
						PhysPortIndex ++) {

						if (!pAC->Pnmi.Port[PhysPortIndex].ActiveFlag) {
							continue;
						}

						EventParam.Para32[0] = PhysPortIndex;
						EventParam.Para32[1] = (SK_U32)Val8;
						if (SkGeSirqEvent(pAC, IoC, SK_HWEV_SET_FLOWMODE,
							EventParam) > 0) {

							SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR044,
								SK_PNMI_ERR044MSG);

							*pLen = 0;
							return (SK_PNMI_ERR_GENERAL);
						}
					}
				}
				else {
					/* Send an event with the new flow control mode to SIRQ */
					EventParam.Para32[0] = SK_PNMI_PORT_LOG2PHYS(
						pAC, LogPortIndex);
					EventParam.Para32[1] = (SK_U32)Val8;
					if (SkGeSirqEvent(pAC, IoC, SK_HWEV_SET_FLOWMODE,
						EventParam) > 0) {

						SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR044,
							SK_PNMI_ERR044MSG);

						*pLen = 0;
						return (SK_PNMI_ERR_GENERAL);
					}
				}
			}
			else { /* DualNet mode */
				/* Send an event with the new flow control mode to SIRQ */
				EventParam.Para32[0] = NetIndex;
				EventParam.Para32[1] = (SK_U32)Val8;
				if (SkGeSirqEvent(pAC, IoC, SK_HWEV_SET_FLOWMODE,
					EventParam) > 0) {

					SK_ERR_LOG(pAC, SK_ERRCL_SW,
						SK_PNMI_ERR044,
						SK_PNMI_ERR044MSG);

					*pLen = 0;
					return (SK_PNMI_ERR_GENERAL);
				}
			}
			Offset++;
			break;

		case OID_SKGE_PHY_OPERATION_MODE :
			/* Check the value range. */
			if (Val8 == 0) {
				/* Mode of this port remains unchanged. */
				Offset++;
				break;
			}
			if (Val8 < SK_MS_MODE_AUTO ||
				(LogPortIndex != 0 && Val8 > SK_MS_MODE_SLAVE) ||
				(LogPortIndex == 0 && Val8 > SK_MS_MODE_INDETERMINATED)) {

				*pLen = 0;
				return (SK_PNMI_ERR_BAD_VALUE);
			}

			/* The PRESET ends here. */
			if (Action == SK_PNMI_PRESET) {

				return (SK_PNMI_ERR_OK);
			}

			if (!pAC->Pnmi.DualNetActiveFlag) { /* SingleNet mode */
				if (LogPortIndex == 0) {
					/*
					 * The virtual port consists of all currently active ports.
					 * Find them and send an event with new master/slave (role)
					 * mode to the SIRQ module.
					 */
					for (PhysPortIndex = 0; PhysPortIndex < PhysPortMax;
						PhysPortIndex ++) {

						if (!pAC->Pnmi.Port[PhysPortIndex].ActiveFlag) {
							continue;
						}

						EventParam.Para32[0] = PhysPortIndex;
						EventParam.Para32[1] = (SK_U32)Val8;
						if (SkGeSirqEvent(pAC, IoC, SK_HWEV_SET_ROLE,
							EventParam) > 0) {

							SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR042,
								SK_PNMI_ERR042MSG);

							*pLen = 0;
							return (SK_PNMI_ERR_GENERAL);
						}
					}
				}
				else {
					/* Send an event with the new role mode to SIRQ */
					EventParam.Para32[0] = SK_PNMI_PORT_LOG2PHYS(
						pAC, LogPortIndex);
					EventParam.Para32[1] = (SK_U32)Val8;
					if (SkGeSirqEvent(pAC, IoC, SK_HWEV_SET_ROLE,
						EventParam) > 0) {

						SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR042,
							SK_PNMI_ERR042MSG);

						*pLen = 0;
						return (SK_PNMI_ERR_GENERAL);
					}
				}
			}
			else { /* DualNet mode */
				/* Send an event with the new master/slave (role) to SIRQ */
				EventParam.Para32[0] = NetIndex;
				EventParam.Para32[1] = (SK_U32)Val8;
				if (SkGeSirqEvent(pAC, IoC, SK_HWEV_SET_ROLE,
					EventParam) > 0) {

					SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR042,
						SK_PNMI_ERR042MSG);

					*pLen = 0;
					return (SK_PNMI_ERR_GENERAL);
				}
			}
			Offset++;
			break;

		case OID_SKGE_SPEED_MODE:
			/* Check the value range. */
			if (Val8 == 0) {
				Offset++;
				break;
			}
			if (Val8 < (SK_LSPEED_AUTO) ||
				(LogPortIndex != 0 && Val8 > SK_LSPEED_1000MBPS) ||
				(LogPortIndex == 0 && Val8 > SK_LSPEED_INDETERMINATED)) {

				*pLen = 0;
				return (SK_PNMI_ERR_BAD_VALUE);
			}

			/* The PRESET ends here. */
			if (Action == SK_PNMI_PRESET) {

				return (SK_PNMI_ERR_OK);
			}

			if (!pAC->Pnmi.DualNetActiveFlag) { /* SingleNet mode */
				if (LogPortIndex == 0) {

					/*
					 * The virtual port consists of all currently active ports.
					 * Find them and send an event with the new link speed to
					 * the SIRQ module.
					 */
					for (PhysPortIndex = 0; PhysPortIndex < PhysPortMax;
						PhysPortIndex ++) {

						if (!pAC->Pnmi.Port[PhysPortIndex].ActiveFlag) {
							continue;
						}

						EventParam.Para32[0] = PhysPortIndex;
						EventParam.Para32[1] = (SK_U32)Val8;
						if (SkGeSirqEvent(pAC, IoC, SK_HWEV_SET_SPEED,
							EventParam) > 0) {

							SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR045,
								SK_PNMI_ERR045MSG);

							*pLen = 0;
							return (SK_PNMI_ERR_GENERAL);
						}
					}
				}
				else {
					/* Send an event with the new link speed to SIRQ */
					EventParam.Para32[0] = SK_PNMI_PORT_LOG2PHYS(
						pAC, LogPortIndex);
					EventParam.Para32[1] = (SK_U32)Val8;
					if (SkGeSirqEvent(pAC, IoC, SK_HWEV_SET_SPEED,
						EventParam) > 0) {

						SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR045,
							SK_PNMI_ERR045MSG);

						*pLen = 0;
						return (SK_PNMI_ERR_GENERAL);
					}
				}
			}
			else { /* DualNet mode */
				/* Send an event with the new link speed to SIRQ */
				EventParam.Para32[0] = NetIndex;
				EventParam.Para32[1] = (SK_U32)Val8;
				if (SkGeSirqEvent(pAC, IoC, SK_HWEV_SET_SPEED,
					EventParam) > 0) {

					SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR045,
						SK_PNMI_ERR045MSG);

					*pLen = 0;
					return (SK_PNMI_ERR_GENERAL);
				}
			}
			Offset++;
			break;

		case OID_SKGE_MTU:
			/* Check the value range. */
			SK_PNMI_READ_U32((pBuf + Offset), Val32);

			if (Val32 == 0) {
				/* MTU of this port remains unchanged. */
				Offset += sizeof(SK_U32);
				break;
			}

			if (SK_DRIVER_PRESET_MTU(pAC, IoC, NetIndex, Val32)) {
				*pLen = 0;
				return (SK_PNMI_ERR_BAD_VALUE);
			}

			/* The PRESET ends here. */
			if (Action == SK_PNMI_PRESET) {
				return (SK_PNMI_ERR_OK);
			}

			if (SK_DRIVER_SET_MTU(pAC, IoC, NetIndex, Val32)) {
				return (SK_PNMI_ERR_GENERAL);
			}

			Offset += sizeof(SK_U32);
			break;

#ifdef SK_PHY_LP_MODE
		case OID_SKGE_PHY_LP_MODE:
			/* The PRESET ends here. */
			if (Action == SK_PNMI_PRESET) {

				return (SK_PNMI_ERR_OK);
			}

			if (!pAC->Pnmi.DualNetActiveFlag) { /* SingleNet mode */
				if (LogPortIndex == 0) {
					Offset = 0;
					continue;
				}
			}
			/* Set value for physical port. */
			PhysPortIndex = SK_PNMI_PORT_LOG2PHYS(pAC, LogPortIndex);
			CurrentPhyPowerState = pAC->GIni.GP[PhysPortIndex].PPhyPowerState;

			switch (Val8) {

			case PHY_PM_OPERATIONAL_MODE:
				/* If LowPowerMode is active, we can leave it. */
				if (CurrentPhyPowerState) {

					Val32 = SkGmLeaveLowPowerMode(pAC, IoC, PhysPortIndex);

					if (CurrentPhyPowerState == PHY_PM_DEEP_SLEEP ||
						CurrentPhyPowerState == PHY_PM_IEEE_POWER_DOWN) {

						SkDrvInitAdapter(pAC);
					}
					break;
				}
				else {
					*pLen = 0;
					return (SK_PNMI_ERR_GENERAL);
				}
			case PHY_PM_DEEP_SLEEP:
			case PHY_PM_IEEE_POWER_DOWN:
				/* If no LowPowerMode is active, we can enter it. */
				if (!CurrentPhyPowerState) {
					SkDrvDeInitAdapter(pAC);
				}

			case PHY_PM_ENERGY_DETECT:
			case PHY_PM_ENERGY_DETECT_PLUS:
				/* If no LowPowerMode is active, we can enter it. */
				if (!CurrentPhyPowerState) {

					Val32 = SkGmEnterLowPowerMode(pAC, IoC, PhysPortIndex, *pBuf);
					break;
				}
				else {
					*pLen = 0;
					return (SK_PNMI_ERR_GENERAL);
				}
			default:
				*pLen = 0;
				return (SK_PNMI_ERR_BAD_VALUE);
			}
			Offset++;
			break;
#endif /* SK_PHY_LP_MODE */

		default:
			SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_ERR,
				("MacPrivateConf: Unknown OID should be handled before set"));

			*pLen = 0;
			return (SK_PNMI_ERR_GENERAL);
		}
	}

	return (SK_PNMI_ERR_OK);
}

/*****************************************************************************
 *
 * VirtualConf - Calculates the values of configuration OIDs for virtual port
 *
 * Description:
 *	We handle here the get of the configuration group OIDs, which are
 *	a little bit complicated. The virtual port consists of all currently
 *	active physical ports. If multiple ports are active and configured
 *	differently we get in some trouble to return a single value. So we
 *	get the value of the first active port and compare it with that of
 *	the other active ports. If they are not the same, we return a value
 *	that indicates that the state is indeterminated.
 *
 * Returns:
 *	Nothing
 */
PNMI_STATIC void VirtualConf(
SK_AC *pAC,		/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
SK_U32 Id,		/* Object ID that is to be processed */
char *pBuf)		/* Buffer used for the management data transfer */
{
	unsigned int	PhysPortMax;
	unsigned int	PhysPortIndex;
	SK_U8		Val8;
	SK_U32		Val32;
	SK_BOOL		PortActiveFlag;
	SK_GEPORT	*pPrt;

	*pBuf = 0;
	PortActiveFlag = SK_FALSE;
	PhysPortMax = pAC->GIni.GIMacsFound;

	for (PhysPortIndex = 0; PhysPortIndex < PhysPortMax; PhysPortIndex ++) {

		pPrt = &pAC->GIni.GP[PhysPortIndex];

		/* Check if the physical port is active. */
		if (!pAC->Pnmi.Port[PhysPortIndex].ActiveFlag) {
			continue;
		}

		PortActiveFlag = SK_TRUE;

		switch (Id) {

		case OID_SKGE_PHY_TYPE:
			/* Check if it is the first active port. */
			if (*pBuf == 0) {
				Val32 = pPrt->PhyType;
				SK_PNMI_STORE_U32(pBuf, Val32);
				continue;
			}
			break;

		case OID_SKGE_LINK_CAP:

			/*
			 * Different capabilities should not happen, but
			 * in the case of the cases OR them all together.
			 * From a curious point of view the virtual port
			 * is capable of all found capabilities.
			 */
			*pBuf |= pPrt->PLinkCap;
			break;

		case OID_SKGE_LINK_MODE:
			/* Check if it is the first active port. */
			if (*pBuf == 0) {

				*pBuf = pPrt->PLinkModeConf;
				continue;
			}

			/*
			 * If we find an active port with a different link mode
			 * than the first one we return indeterminated.
			 */
			if (*pBuf != pPrt->PLinkModeConf) {

				*pBuf = SK_LMODE_INDETERMINATED;
			}
			break;

		case OID_SKGE_LINK_MODE_STATUS:
			/* Get the link mode of the physical port. */
			Val8 = CalculateLinkModeStatus(pAC, IoC, PhysPortIndex);

			/* Check if it is the first active port. */
			if (*pBuf == 0) {

				*pBuf = Val8;
				continue;
			}

			/*
			 * If we find an active port with a different link mode status
			 * than the first one we return indeterminated.
			 */
			if (*pBuf != Val8) {

				*pBuf = SK_LMODE_STAT_INDETERMINATED;
			}
			break;

		case OID_SKGE_LINK_STATUS:
			/* Get the link status of the physical port. */
			Val8 = CalculateLinkStatus(pAC, IoC, PhysPortIndex);

			/* Check if it is the first active port. */
			if (*pBuf == 0) {

				*pBuf = Val8;
				continue;
			}

			/*
			 * If we find an active port with a different link status
			 * than the first one we return indeterminated.
			 */
			if (*pBuf != Val8) {

				*pBuf = SK_PNMI_RLMT_LSTAT_INDETERMINATED;
			}
			break;

		case OID_SKGE_FLOWCTRL_CAP:
			/* Check if it is the first active port. */
			if (*pBuf == 0) {

				*pBuf = pPrt->PFlowCtrlCap;
				continue;
			}

			/*
			 * From a curious point of view the virtual port
			 * is capable of all found capabilities.
			 */
			*pBuf |= pPrt->PFlowCtrlCap;
			break;

		case OID_SKGE_FLOWCTRL_MODE:
			/* Check if it is the first active port. */
			if (*pBuf == 0) {

				*pBuf = pPrt->PFlowCtrlMode;
				continue;
			}

			/*
			 * If we find an active port with a different flow-control mode
			 * than the first one we return indeterminated.
			 */
			if (*pBuf != pPrt->PFlowCtrlMode) {

				*pBuf = SK_FLOW_MODE_INDETERMINATED;
			}
			break;

		case OID_SKGE_FLOWCTRL_STATUS:
			/* Check if it is the first active port. */
			if (*pBuf == 0) {

				*pBuf = pPrt->PFlowCtrlStatus;
				continue;
			}

			/*
			 * If we find an active port with a different flow-control status
			 * than the first one we return indeterminated.
			 */
			if (*pBuf != pPrt->PFlowCtrlStatus) {

				*pBuf = SK_FLOW_STAT_INDETERMINATED;
			}
			break;

		case OID_SKGE_PHY_OPERATION_CAP:
			/* Check if it is the first active port. */
			if (*pBuf == 0) {

				*pBuf = pPrt->PMSCap;
				continue;
			}

			/*
			 * From a curious point of view the virtual port
			 * is capable of all found capabilities.
			 */
			*pBuf |= pPrt->PMSCap;
			break;

		case OID_SKGE_PHY_OPERATION_MODE:
			/* Check if it is the first active port. */
			if (*pBuf == 0) {

				*pBuf = pPrt->PMSMode;
				continue;
			}

			/*
			 * If we find an active port with a different master/slave mode
			 * than the first one we return indeterminated.
			 */
			if (*pBuf != pPrt->PMSMode) {

				*pBuf = SK_MS_MODE_INDETERMINATED;
			}
			break;

		case OID_SKGE_PHY_OPERATION_STATUS:
			/* Check if it is the first active port. */
			if (*pBuf == 0) {

				*pBuf = pPrt->PMSStatus;
				continue;
			}

			/*
			 * If we find an active port with a different master/slave status
			 * than the first one we return indeterminated.
			 */
			if (*pBuf != pPrt->PMSStatus) {

				*pBuf = SK_MS_STAT_INDETERMINATED;
			}
			break;

		case OID_SKGE_SPEED_MODE:
			/* Check if it is the first active port. */
			if (*pBuf == 0) {

				*pBuf = pPrt->PLinkSpeed;
				continue;
			}

			/*
			 * If we find an active port with a different link speed
			 * than the first one we return indeterminated.
			 */
			if (*pBuf != pPrt->PLinkSpeed) {

				*pBuf = SK_LSPEED_INDETERMINATED;
			}
			break;

		case OID_SKGE_SPEED_STATUS:
			/* Check if it is the first active port. */
			if (*pBuf == 0) {

				*pBuf = pPrt->PLinkSpeedUsed;
				continue;
			}

			/*
			 * If we find an active port with a different link speed used
			 * than the first one we return indeterminated.
			 */
			if (*pBuf != pPrt->PLinkSpeedUsed) {

				*pBuf = SK_LSPEED_STAT_INDETERMINATED;
			}
			break;
		}
	}

	/* If no port is active return an indeterminated answer. */
	if (!PortActiveFlag) {

		switch (Id) {

		case OID_SKGE_LINK_CAP:
			*pBuf = SK_LMODE_CAP_INDETERMINATED;
			break;

		case OID_SKGE_LINK_MODE:
			*pBuf = SK_LMODE_INDETERMINATED;
			break;

		case OID_SKGE_LINK_MODE_STATUS:
			*pBuf = SK_LMODE_STAT_INDETERMINATED;
			break;

		case OID_SKGE_LINK_STATUS:
			*pBuf = SK_PNMI_RLMT_LSTAT_INDETERMINATED;
			break;

		case OID_SKGE_FLOWCTRL_CAP:
		case OID_SKGE_FLOWCTRL_MODE:
			*pBuf = SK_FLOW_MODE_INDETERMINATED;
			break;

		case OID_SKGE_FLOWCTRL_STATUS:
			*pBuf = SK_FLOW_STAT_INDETERMINATED;
			break;

		case OID_SKGE_PHY_OPERATION_CAP:
			*pBuf = SK_MS_CAP_INDETERMINATED;
			break;

		case OID_SKGE_PHY_OPERATION_MODE:
			*pBuf = SK_MS_MODE_INDETERMINATED;
			break;

		case OID_SKGE_PHY_OPERATION_STATUS:
			*pBuf = SK_MS_STAT_INDETERMINATED;
			break;
		case OID_SKGE_SPEED_CAP:
			*pBuf = SK_LSPEED_CAP_INDETERMINATED;
			break;

		case OID_SKGE_SPEED_MODE:
			*pBuf = SK_LSPEED_INDETERMINATED;
			break;

		case OID_SKGE_SPEED_STATUS:
			*pBuf = SK_LSPEED_STAT_INDETERMINATED;
			break;
		}
	}
}

/*****************************************************************************
 *
 * CalculateLinkStatus - Determins the link status of a physical port
 *
 * Description:
 *	Determins the link status the following way:
 *	  LSTAT_PHY_DOWN:  Link is down
 *	  LSTAT_AUTONEG:   Auto-negotiation failed
 *	  LSTAT_LOG_DOWN:  Link is up but RLMT did not yet put the port
 *	                   logically up.
 *	  LSTAT_LOG_UP:    RLMT marked the port as up
 *
 * Returns:
 *	Link status of physical port
 */
PNMI_STATIC SK_U8 CalculateLinkStatus(
SK_AC *pAC,			/* Pointer to adapter context */
SK_IOC IoC,			/* IO context handle */
unsigned int PhysPortIndex)	/* Physical port index */
{
	SK_U8	Result = SK_PNMI_RLMT_LSTAT_INDETERMINATED;

	if (!pAC->GIni.GP[PhysPortIndex].PHWLinkUp) {

		Result = SK_PNMI_RLMT_LSTAT_PHY_DOWN;
	}
	else if (pAC->GIni.GP[PhysPortIndex].PAutoNegFail > 0) {

		Result = SK_PNMI_RLMT_LSTAT_AUTONEG;
	}
#ifndef SK_NO_RLMT
	else if (!pAC->Rlmt.Port[PhysPortIndex].PortDown) {

		Result = SK_PNMI_RLMT_LSTAT_LOG_UP;
	}
	else {
		Result = SK_PNMI_RLMT_LSTAT_LOG_DOWN;
	}
#endif
	return (Result);
}

/*****************************************************************************
 *
 * CalculateLinkModeStatus - Determins the link mode status of a phys. port
 *
 * Description:
 *	The COMMON module only tells us if the mode is half or full duplex.
 *	But in the decade of auto sensing it is usefull for the user to
 *	know if the mode was negotiated or forced. Therefore we have a
 *	look to the mode, which was last used by the negotiation process.
 *
 * Returns:
 *	The link mode status
 */
PNMI_STATIC SK_U8 CalculateLinkModeStatus(
SK_AC *pAC,			/* Pointer to adapter context */
SK_IOC IoC,			/* IO context handle */
unsigned int PhysPortIndex)	/* Physical port index */
{
	SK_U8	Result;

	/* Get the current mode, which can be full or half duplex. */
	Result = pAC->GIni.GP[PhysPortIndex].PLinkModeStatus;

	/* Check if no valid mode could be found (link is down). */
	if (Result < SK_LMODE_STAT_HALF) {

		Result = SK_LMODE_STAT_UNKNOWN;
	}
	else if (pAC->GIni.GP[PhysPortIndex].PLinkMode >= SK_LMODE_AUTOHALF) {
		/*
		 * Auto-negotiation was used to bring up the link. Change
		 * the already found duplex status that it indicates
		 * auto-negotiation was involved.
		 */
		if (Result == SK_LMODE_STAT_HALF) {

			Result = SK_LMODE_STAT_AUTOHALF;
		}
		else if (Result == SK_LMODE_STAT_FULL) {

			Result = SK_LMODE_STAT_AUTOFULL;
		}
	}

	return (Result);
}

/*****************************************************************************
 *
 * GetVpdKeyArr - Obtain an array of VPD keys
 *
 * Description:
 *	Read the VPD keys and build an array of VPD keys, which are
 *	easy to access.
 *
 * Returns:
 *	SK_PNMI_ERR_OK	     Task successfully performed.
 *	SK_PNMI_ERR_GENERAL  Something went wrong.
 */
PNMI_STATIC int GetVpdKeyArr(
SK_AC *pAC,		/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
char *pKeyArr,		/* Ptr KeyArray */
unsigned int KeyArrLen,	/* Length of array in bytes */
unsigned int *pKeyNo)	/* Number of keys */
{
	unsigned int		BufKeysLen = SK_PNMI_VPD_BUFSIZE;
	char			BufKeys[SK_PNMI_VPD_BUFSIZE];
	unsigned int		StartOffset;
	unsigned int		Offset;
	int			Index;
	int			Ret;

	SK_MEMSET(pKeyArr, 0, KeyArrLen);

	/* Get VPD key list. */
	Ret = VpdKeys(pAC, IoC, BufKeys, (int *)&BufKeysLen, (int *)pKeyNo);

	if (Ret > 0) {

		SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
			(SK_PNMI_ERR014MSG));

		/* Please read comment in Vpd(). */
		pAC->Pnmi.VpdKeyReadError = SK_TRUE;

		return (SK_PNMI_ERR_GENERAL);
	}

	/* If no keys are available return now. */
	if (*pKeyNo == 0 || BufKeysLen == 0) {

		return (SK_PNMI_ERR_OK);
	}
	/*
	 * If the key list is too long for us trunc it and give a
	 * errorlog notification. This case should not happen because
	 * the maximum number of keys is limited due to RAM limitations.
	 */
	if (*pKeyNo > SK_PNMI_VPD_ENTRIES) {

		SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
			(SK_PNMI_ERR015MSG));

		*pKeyNo = SK_PNMI_VPD_ENTRIES;
	}

	/* Build an array of fixed string size and copy the keys together */
	for (Index = 0, StartOffset = 0, Offset = 0; Offset < BufKeysLen;
		Offset ++) {

		if (BufKeys[Offset] != 0) {
			continue;
		}

		if (Offset - StartOffset > SK_PNMI_VPD_KEY_SIZE) {

			SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_CTRL,
					   (SK_PNMI_ERR016MSG));

			return (SK_PNMI_ERR_GENERAL);
		}

		SK_STRNCPY(pKeyArr + Index * SK_PNMI_VPD_KEY_SIZE,
			&BufKeys[StartOffset], SK_PNMI_VPD_KEY_SIZE);

		Index ++;
		StartOffset = Offset + 1;
	}

	/* Last key not zero terminated? Get it anyway. */
	if (StartOffset < Offset) {

		SK_STRNCPY(pKeyArr + Index * SK_PNMI_VPD_KEY_SIZE,
			&BufKeys[StartOffset], SK_PNMI_VPD_KEY_SIZE);
	}

	return (SK_PNMI_ERR_OK);
}

/*****************************************************************************
 *
 * SirqUpdate - Let the SIRQ update its internal values
 *
 * Description:
 *	Just to be sure that the SIRQ module holds its internal data
 *	structures up to date, we send an update event before we make
 *	any access.
 *
 * Returns:
 *	SK_PNMI_ERR_OK	     Task successfully performed.
 *	SK_PNMI_ERR_GENERAL  Something went wrong.
 */
PNMI_STATIC int SirqUpdate(
SK_AC *pAC,	/* Pointer to adapter context */
SK_IOC IoC)	/* IO context handle */
{
	SK_EVPARA	EventParam;

	/* Was the module already updated during the current PNMI call? */
	if (pAC->Pnmi.SirqUpdatedFlag > 0) {

		return (SK_PNMI_ERR_OK);
	}

	/* Send an synchronuous update event to the module. */
	SK_MEMSET((char *)&EventParam, 0, sizeof(EventParam));

	if (SkGeSirqEvent(pAC, IoC, SK_HWEV_UPDATE_STAT, EventParam)) {

		SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR047, SK_PNMI_ERR047MSG);

		return (SK_PNMI_ERR_GENERAL);
	}

	return (SK_PNMI_ERR_OK);
}
#ifndef SK_NO_RLMT
/*****************************************************************************
 *
 * RlmtUpdate - Let the RLMT update its internal values
 *
 * Description:
 *	Just to be sure that the RLMT module holds its internal data
 *	structures up to date, we send an update event before we make
 *	any access.
 *
 * Returns:
 *	SK_PNMI_ERR_OK	     Task successfully performed.
 *	SK_PNMI_ERR_GENERAL  Something went wrong.
 */
PNMI_STATIC int RlmtUpdate(
SK_AC *pAC,	/* Pointer to adapter context */
SK_IOC IoC,	/* IO context handle */
SK_U32 NetIndex)	/* NetIndex (0..n), in single net mode always zero */
{
	SK_EVPARA	EventParam;

	/* Was the module already updated during the current PNMI call? */
	if (pAC->Pnmi.RlmtUpdatedFlag > 0) {

		return (SK_PNMI_ERR_OK);
	}

	/* Send an synchronuous update event to the module. */
	SK_MEMSET((char *)&EventParam, 0, sizeof(EventParam));
	EventParam.Para32[0] = NetIndex;
	EventParam.Para32[1] = (SK_U32)-1;
	if (SkRlmtEvent(pAC, IoC, SK_RLMT_STATS_UPDATE, EventParam) > 0) {

		SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR048, SK_PNMI_ERR048MSG);

		return (SK_PNMI_ERR_GENERAL);
	}

	return (SK_PNMI_ERR_OK);
}
#endif
/*****************************************************************************
 *
 * MacUpdate - Force the XMAC to output the current statistic
 *
 * Description:
 *	The XMAC holds its statistic internally. To obtain the current
 *	values we must send a command so that the statistic data will
 *	be written to a predefined memory area on the adapter.
 *
 * Returns:
 *	SK_PNMI_ERR_OK	     Task successfully performed.
 *	SK_PNMI_ERR_GENERAL  Something went wrong.
 */
PNMI_STATIC int MacUpdate(
SK_AC *pAC,		/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
unsigned int FirstMac,	/* Index of the first Mac to be updated */
unsigned int LastMac)	/* Index of the last Mac to be updated */
{
	unsigned int	MacIndex;

	/*
	 * Were the statistics already updated during the
	 * current PNMI call?
	 */
	if (pAC->Pnmi.MacUpdatedFlag > 0) {

		return (SK_PNMI_ERR_OK);
	}

	/* Send an update command to all MACs specified. */
	for (MacIndex = FirstMac; MacIndex <= LastMac; MacIndex ++) {

		/*
		 * Freeze the current SW counters.
		 * (This should be done as close as possible to the update of the
		 * HW counters).
		 */
		if (pAC->GIni.GIMacType == SK_MAC_XMAC) {
			pAC->Pnmi.BufPort[MacIndex] = pAC->Pnmi.Port[MacIndex];
		}

		/*  Update the HW counter  */
		if (pAC->GIni.GIFunc.pFnMacUpdateStats(pAC, IoC, MacIndex) != 0) {

			return (SK_PNMI_ERR_GENERAL);
		}
	}

	return (SK_PNMI_ERR_OK);
}

/*****************************************************************************
 *
 * GetStatVal - Retrieve an XMAC statistic counter
 *
 * Description:
 *	Retrieves the statistic counter of a virtual or physical port. The
 *	virtual port is identified by the index 0. It consists of all
 *	currently active ports. To obtain the counter value for this port
 *	we must add the statistic counter of all active ports. To grant
 *	continuous counter values for the virtual port even when port
 *	switches occur we must additionally add a delta value, which was
 *	calculated during a SK_PNMI_EVT_RLMT_ACTIVE_UP event.
 *
 * Returns:
 *	Requested statistic value
 */
PNMI_STATIC SK_U64 GetStatVal(
SK_AC *pAC,					/* Pointer to adapter context */
SK_IOC IoC,					/* IO context handle */
unsigned int LogPortIndex,	/* Index of the logical Port to be processed */
unsigned int StatIndex,		/* Index to statistic value */
SK_U32 NetIndex)	/* NetIndex (0..n), in single net mode always zero */
{
	unsigned int	PhysPortIndex;
	unsigned int	PhysPortMax;
	SK_U64			Val = 0;

	if (pAC->Pnmi.DualNetActiveFlag) {	/* DualNet mode */

		PhysPortIndex = NetIndex;

		Val = GetPhysStatVal(pAC, IoC, PhysPortIndex, StatIndex);
	}
	else {	/* SingleNet mode */

		if (LogPortIndex == 0) {

			PhysPortMax = pAC->GIni.GIMacsFound;

			/* Add counter of all active ports. */
			for (PhysPortIndex = 0; PhysPortIndex < PhysPortMax;
				PhysPortIndex ++) {

				if (pAC->Pnmi.Port[PhysPortIndex].ActiveFlag) {

					Val += GetPhysStatVal(pAC, IoC, PhysPortIndex, StatIndex);
				}
			}

			/* Correct value because of port switches. */
			Val += pAC->Pnmi.VirtualCounterOffset[StatIndex];
		}
		else {
			/* Get counter value of physical port. */
			PhysPortIndex = SK_PNMI_PORT_LOG2PHYS(pAC, LogPortIndex);

			Val = GetPhysStatVal(pAC, IoC, PhysPortIndex, StatIndex);
		}
	}
	return (Val);
}

/*****************************************************************************
 *
 * GetPhysStatVal - Get counter value for physical port
 *
 * Description:
 *	Builds a 64bit counter value. Except for the octet counters
 *	the lower 32bit are counted in hardware and the upper 32bit
 *	in software by monitoring counter overflow interrupts in the
 *	event handler. To grant continous counter values during XMAC
 *	resets (caused by a workaround) we must add a delta value.
 *	The delta was calculated in the event handler when a
 *	SK_PNMI_EVT_XMAC_RESET was received.
 *
 * Returns:
 *	Counter value
 */
PNMI_STATIC SK_U64 GetPhysStatVal(
SK_AC *pAC,					/* Pointer to adapter context */
SK_IOC IoC,					/* IO context handle */
unsigned int PhysPortIndex,	/* Index of the logical Port to be processed */
unsigned int StatIndex)		/* Index to statistic value */
{
	SK_U64	Val = 0;
	SK_U32	LowVal = 0;
	SK_U32	HighVal = 0;
	int		MacType;
	unsigned int HelpIndex;
	SK_GEPORT	*pPrt;

	SK_PNMI_PORT	*pPnmiPrt;
	SK_GEMACFUNC	*pFnMac;

	pPrt = &pAC->GIni.GP[PhysPortIndex];

	MacType = pAC->GIni.GIMacType;

	/* For XMAC, use the frozen SW counters (BufPort) */
	if (MacType == SK_MAC_XMAC) {
		pPnmiPrt = &pAC->Pnmi.BufPort[PhysPortIndex];
	}
	else {
		pPnmiPrt = &pAC->Pnmi.Port[PhysPortIndex];
	}

	pFnMac   = &pAC->GIni.GIFunc;

	switch (StatIndex) {

	case SK_PNMI_HTX:
		if (MacType == SK_MAC_GMAC) {
			(void)pFnMac->pFnMacStatistic(pAC, IoC, PhysPortIndex,
							StatAddr[SK_PNMI_HTX_BROADCAST][MacType].Reg,
							&LowVal);
			(void)pFnMac->pFnMacStatistic(pAC, IoC, PhysPortIndex,
							StatAddr[SK_PNMI_HTX_MULTICAST][MacType].Reg,
							&HighVal);
			LowVal += HighVal;
			(void)pFnMac->pFnMacStatistic(pAC, IoC, PhysPortIndex,
							StatAddr[SK_PNMI_HTX_UNICAST][MacType].Reg,
							&HighVal);
			LowVal += HighVal;
		}
		else {
			(void)pFnMac->pFnMacStatistic(pAC, IoC, PhysPortIndex,
										  StatAddr[StatIndex][MacType].Reg,
										  &LowVal);
		}
		HighVal = pPnmiPrt->CounterHigh[StatIndex];
		break;

	case SK_PNMI_HRX:
		if (MacType == SK_MAC_GMAC) {
			(void)pFnMac->pFnMacStatistic(pAC, IoC, PhysPortIndex,
							StatAddr[SK_PNMI_HRX_BROADCAST][MacType].Reg,
							&LowVal);
			(void)pFnMac->pFnMacStatistic(pAC, IoC, PhysPortIndex,
							StatAddr[SK_PNMI_HRX_MULTICAST][MacType].Reg,
							&HighVal);
			LowVal += HighVal;
			(void)pFnMac->pFnMacStatistic(pAC, IoC, PhysPortIndex,
							StatAddr[SK_PNMI_HRX_UNICAST][MacType].Reg,
							&HighVal);
			LowVal += HighVal;
		}
		else {
			(void)pFnMac->pFnMacStatistic(pAC, IoC, PhysPortIndex,
										  StatAddr[StatIndex][MacType].Reg,
										  &LowVal);
		}
		HighVal = pPnmiPrt->CounterHigh[StatIndex];
		break;

	case SK_PNMI_HTX_OCTET:
	case SK_PNMI_HRX_OCTET:
		(void)pFnMac->pFnMacStatistic(pAC, IoC, PhysPortIndex,
									  StatAddr[StatIndex][MacType].Reg,
									  &HighVal);
		(void)pFnMac->pFnMacStatistic(pAC, IoC, PhysPortIndex,
									  StatAddr[StatIndex + 1][MacType].Reg,
									  &LowVal);
		break;

	case SK_PNMI_HTX_BURST:
	case SK_PNMI_HTX_EXCESS_DEF:
	case SK_PNMI_HTX_CARRIER:
		/* Not supported by GMAC. */
		if (MacType == SK_MAC_GMAC) {
			return (Val);
		}

		(void)pFnMac->pFnMacStatistic(pAC, IoC, PhysPortIndex,
									  StatAddr[StatIndex][MacType].Reg,
									  &LowVal);
		HighVal = pPnmiPrt->CounterHigh[StatIndex];
		break;

	case SK_PNMI_HTX_MACC:
		/* GMAC only supports PAUSE MAC control frames. */
		if (MacType == SK_MAC_GMAC) {
			HelpIndex = SK_PNMI_HTX_PMACC;
		}
		else {
			HelpIndex = StatIndex;
		}

		(void)pFnMac->pFnMacStatistic(pAC, IoC, PhysPortIndex,
								StatAddr[HelpIndex][MacType].Reg,
								&LowVal);

		HighVal = pPnmiPrt->CounterHigh[StatIndex];
		break;

	case SK_PNMI_HTX_COL:
	case SK_PNMI_HRX_UNDERSIZE:
		/* Not supported by XMAC. */
		if (MacType == SK_MAC_XMAC) {
			return (Val);
		}

		(void)pFnMac->pFnMacStatistic(pAC, IoC, PhysPortIndex,
									  StatAddr[StatIndex][MacType].Reg,
									  &LowVal);
		HighVal = pPnmiPrt->CounterHigh[StatIndex];
		break;

	case SK_PNMI_HTX_DEFFERAL:
		/* Not supported by GMAC. */
		if (MacType == SK_MAC_GMAC) {
			return (Val);
		}

		/*
		 * XMAC counts frames with deferred transmission
		 * even in full-duplex mode.
		 *
		 * In full-duplex mode the counter remains constant!
		 */
		if ((pPrt->PLinkModeStatus == SK_LMODE_STAT_AUTOFULL) ||
			(pPrt->PLinkModeStatus == SK_LMODE_STAT_FULL)) {

			LowVal = 0;
			HighVal = 0;
		}
		else {
			/* Otherwise get contents of hardware register. */
			(void)pFnMac->pFnMacStatistic(pAC, IoC, PhysPortIndex,
										  StatAddr[StatIndex][MacType].Reg,
										  &LowVal);
			HighVal = pPnmiPrt->CounterHigh[StatIndex];
		}
		break;

	case SK_PNMI_HRX_BADOCTET:
		/* Not supported by XMAC. */
		if (MacType == SK_MAC_XMAC) {
			return (Val);
		}

		(void)pFnMac->pFnMacStatistic(pAC, IoC, PhysPortIndex,
									  StatAddr[StatIndex][MacType].Reg,
									  &HighVal);
		(void)pFnMac->pFnMacStatistic(pAC, IoC, PhysPortIndex,
									  StatAddr[StatIndex + 1][MacType].Reg,
									  &LowVal);
		break;

	case SK_PNMI_HTX_OCTETLOW:
	case SK_PNMI_HRX_OCTETLOW:
	case SK_PNMI_HRX_BADOCTETLOW:
		return (Val);

	case SK_PNMI_HRX_LONGFRAMES:
		/* For XMAC the SW counter is managed by PNMI. */
		if (MacType == SK_MAC_XMAC) {
			return (pPnmiPrt->StatRxLongFrameCts);
		}

		(void)pFnMac->pFnMacStatistic(pAC, IoC, PhysPortIndex,
									  StatAddr[StatIndex][MacType].Reg,
									  &LowVal);
		HighVal = pPnmiPrt->CounterHigh[StatIndex];
		break;

	case SK_PNMI_HRX_TOO_LONG:
		(void)pFnMac->pFnMacStatistic(pAC, IoC, PhysPortIndex,
								StatAddr[StatIndex][MacType].Reg,
								&LowVal);
		HighVal = pPnmiPrt->CounterHigh[StatIndex];

		Val = (((SK_U64)HighVal << 32) | (SK_U64)LowVal);

		if (MacType == SK_MAC_GMAC) {
			/* For GMAC the SW counter is additionally managed by PNMI. */
			Val += pPnmiPrt->StatRxFrameTooLongCts;
		}
		else {
			/*
			 * Frames longer than IEEE 802.3 frame max size are counted
			 * by XMAC in frame_too_long counter even reception of long
			 * frames was enabled and the frame was correct.
			 * So correct the value by subtracting RxLongFrame counter.
			 */
			Val -= pPnmiPrt->StatRxLongFrameCts;
		}

		LowVal = (SK_U32)Val;
		HighVal = (SK_U32)(Val >> 32);
		break;

	case SK_PNMI_HRX_SHORTS:
		/* Not supported by GMAC. */
		if (MacType == SK_MAC_GMAC) {

			/* Instead, SK_PNMI_HRX_UNDERSIZE is used. */
			Val = GetStatVal(pAC, IoC, PhysPortIndex,
							   SK_PNMI_HRX_UNDERSIZE, 0);
			return (Val);
		}

		/*
		 * XMAC counts short frame errors even if link down (#10620).
		 * If the link is down, the counter remains constant.
		 */
		if (pPrt->PLinkModeStatus != SK_LMODE_STAT_UNKNOWN) {

			/* Otherwise get incremental difference. */
			(void)pFnMac->pFnMacStatistic(pAC, IoC, PhysPortIndex,
										  StatAddr[StatIndex][MacType].Reg,
										  &LowVal);
			HighVal = pPnmiPrt->CounterHigh[StatIndex];

			Val = (((SK_U64)HighVal << 32) | (SK_U64)LowVal);
			Val -= pPnmiPrt->RxShortZeroMark;

			LowVal = (SK_U32)Val;
			HighVal = (SK_U32)(Val >> 32);
		}
		break;

	case SK_PNMI_HRX_MACC:
	case SK_PNMI_HRX_MACC_UNKWN:
	case SK_PNMI_HRX_BURST:
	case SK_PNMI_HRX_MISSED:
	case SK_PNMI_HRX_FRAMING:
	case SK_PNMI_HRX_CARRIER:
	case SK_PNMI_HRX_IRLENGTH:
	case SK_PNMI_HRX_SYMBOL:
	case SK_PNMI_HRX_CEXT:
		/* Not supported by GMAC. */
		if (MacType == SK_MAC_GMAC) {
			return (Val);
		}

		(void)pFnMac->pFnMacStatistic(pAC, IoC, PhysPortIndex,
									  StatAddr[StatIndex][MacType].Reg,
									  &LowVal);
		HighVal = pPnmiPrt->CounterHigh[StatIndex];
		break;

	case SK_PNMI_HRX_PMACC_ERR:
		/* For GMAC the SW counter is managed by PNMI. */
		if (MacType == SK_MAC_GMAC) {
			return (pPnmiPrt->StatRxPMaccErr);
		}

		(void)pFnMac->pFnMacStatistic(pAC, IoC, PhysPortIndex,
									  StatAddr[StatIndex][MacType].Reg,
									  &LowVal);
		HighVal = pPnmiPrt->CounterHigh[StatIndex];
		break;

	/* SW counter managed by PNMI. */
	case SK_PNMI_HTX_SYNC:
		LowVal = (SK_U32)pPnmiPrt->StatSyncCts;
		HighVal = (SK_U32)(pPnmiPrt->StatSyncCts >> 32);
		break;

	/* SW counter managed by PNMI. */
	case SK_PNMI_HTX_SYNC_OCTET:
		LowVal = (SK_U32)pPnmiPrt->StatSyncOctetsCts;
		HighVal = (SK_U32)(pPnmiPrt->StatSyncOctetsCts >> 32);
		break;

	case SK_PNMI_HRX_FCS:
		/*
		 * Broadcom filters FCS errors and counts them in
		 * Receive Error Counter register.
		 */
		(void)pFnMac->pFnMacStatistic(pAC, IoC, PhysPortIndex,
									  StatAddr[StatIndex][MacType].Reg,
									  &LowVal);
		HighVal = pPnmiPrt->CounterHigh[StatIndex];
		break;

	default:
		(void)pFnMac->pFnMacStatistic(pAC, IoC, PhysPortIndex,
									  StatAddr[StatIndex][MacType].Reg,
									  &LowVal);
		HighVal = pPnmiPrt->CounterHigh[StatIndex];
		break;
	}

	Val = (((SK_U64)HighVal << 32) | (SK_U64)LowVal);

	/* Correct value because of possible XMAC reset (XMAC Errata #2). */
	Val += pPnmiPrt->CounterOffset[StatIndex];

	return (Val);
}

/*****************************************************************************
 *
 * ResetCounter - Set all counters and timestamps to zero
 *
 * Description:
 *	Notifies other common modules which store statistic data to
 *	reset their counters and finally reset our own counters.
 *
 * Returns:
 *	Nothing
 */
PNMI_STATIC void ResetCounter(
SK_AC *pAC,		/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
SK_U32 NetIndex)
{
	unsigned int	PhysPortIndex;
	SK_EVPARA	EventParam;

	SK_MEMSET((char *)&EventParam, 0, sizeof(EventParam));

	/* Notify sensor module. */
	SkEventQueue(pAC, SKGE_I2C, SK_I2CEV_CLEAR, EventParam);
#ifndef SK_NO_RLMT
	/* Notify RLMT module. */
	EventParam.Para32[0] = NetIndex;
	EventParam.Para32[1] = (SK_U32)-1;
	SkEventQueue(pAC, SKGE_RLMT, SK_RLMT_STATS_CLEAR, EventParam);
	EventParam.Para32[1] = 0;
#endif
	/* Notify SIRQ module. */
	SkEventQueue(pAC, SKGE_HWAC, SK_HWEV_CLEAR_STAT, EventParam);

	/* Notify CSUM module. */
#ifdef SK_USE_CSUM
	EventParam.Para32[0] = NetIndex;
	EventParam.Para32[1] = (SK_U32)-1;
	SkEventQueue(pAC, SKGE_CSUM, SK_CSUM_EVENT_CLEAR_PROTO_STATS,
		EventParam);
#endif /* SK_USE_CSUM */

	/* Clear XMAC statistics. */
	for (PhysPortIndex = 0; PhysPortIndex <
		(unsigned int)pAC->GIni.GIMacsFound; PhysPortIndex ++) {

		(void)pAC->GIni.GIFunc.pFnMacResetCounter(pAC, IoC, PhysPortIndex);

		SK_MEMSET((char *)&pAC->Pnmi.Port[PhysPortIndex].CounterHigh,
			0, sizeof(pAC->Pnmi.Port[PhysPortIndex].CounterHigh));

		SK_MEMSET((char *)&pAC->Pnmi.Port[PhysPortIndex].CounterOffset,
			0, sizeof(pAC->Pnmi.Port[PhysPortIndex].CounterOffset));

		SK_MEMSET((char *)&pAC->Pnmi.Port[PhysPortIndex].StatSyncCts,
			0, sizeof(pAC->Pnmi.Port[PhysPortIndex].StatSyncCts));

		SK_MEMSET((char *)&pAC->Pnmi.Port[PhysPortIndex].StatSyncOctetsCts,
			0, sizeof(pAC->Pnmi.Port[PhysPortIndex].StatSyncOctetsCts));

		SK_MEMSET((char *)&pAC->Pnmi.Port[PhysPortIndex].StatRxLongFrameCts,
			0, sizeof(pAC->Pnmi.Port[PhysPortIndex].StatRxLongFrameCts));

		SK_MEMSET((char *)&pAC->Pnmi.Port[PhysPortIndex].StatRxFrameTooLongCts,
			0, sizeof(pAC->Pnmi.Port[PhysPortIndex].StatRxFrameTooLongCts));

		SK_MEMSET((char *)&pAC->Pnmi.Port[PhysPortIndex].StatRxPMaccErr,
			0, sizeof(pAC->Pnmi.Port[PhysPortIndex].StatRxPMaccErr));
	}

	/* Clear local statistics. */
	SK_MEMSET((char *)&pAC->Pnmi.VirtualCounterOffset, 0,
		  sizeof(pAC->Pnmi.VirtualCounterOffset));

	pAC->Pnmi.RlmtChangeCts = 0;
	pAC->Pnmi.RlmtChangeTime = 0;

	SK_MEMSET((char *)&pAC->Pnmi.RlmtChangeEstimate.EstValue[0], 0,
		sizeof(pAC->Pnmi.RlmtChangeEstimate.EstValue));
	pAC->Pnmi.RlmtChangeEstimate.EstValueIndex = 0;
	pAC->Pnmi.RlmtChangeEstimate.Estimate = 0;
	pAC->Pnmi.Port[NetIndex].TxSwQueueMax = 0;
	pAC->Pnmi.Port[NetIndex].TxRetryCts = 0;
	pAC->Pnmi.Port[NetIndex].RxIntrCts = 0;
	pAC->Pnmi.Port[NetIndex].TxIntrCts = 0;
	pAC->Pnmi.Port[NetIndex].RxNoBufCts = 0;
	pAC->Pnmi.Port[NetIndex].TxNoBufCts = 0;
	pAC->Pnmi.Port[NetIndex].TxUsedDescrNo = 0;
	pAC->Pnmi.Port[NetIndex].RxDeliveredCts = 0;
	pAC->Pnmi.Port[NetIndex].RxOctetsDeliveredCts = 0;
	pAC->Pnmi.Port[NetIndex].ErrRecoveryCts = 0;
}

/*****************************************************************************
 *
 * GetTrapEntry - Get an entry in the trap buffer
 *
 * Description:
 *	The trap buffer stores various events. A user application somehow
 *	gets notified that an event occurred and retrieves the trap buffer
 *	contens (or simply polls the buffer). The buffer is organized as
 *	a ring which stores the newest traps at the beginning. The oldest
 *	traps are overwritten by the newest ones. Each trap entry has a
 *	unique number, so that applications may detect new trap entries.
 *
 * Returns:
 *	A pointer to the trap entry
 */
PNMI_STATIC char* GetTrapEntry(
SK_AC *pAC,		/* Pointer to adapter context */
SK_U32 TrapId,		/* SNMP ID of the trap */
unsigned int Size)	/* Space needed for trap entry */
{
	unsigned int	BufPad = pAC->Pnmi.TrapBufPad;
	unsigned int	BufFree = pAC->Pnmi.TrapBufFree;
	unsigned int	Beg = pAC->Pnmi.TrapQueueBeg;
	unsigned int	End = pAC->Pnmi.TrapQueueEnd;
	char			*pBuf = &pAC->Pnmi.TrapBuf[0];
	int			Wrap;
	unsigned int	NeededSpace;
	unsigned int	EntrySize;
	SK_U32			Val32;
	SK_U64			Val64;

	/* Last byte of entry will get a copy of the entry length. */
	Size ++;

	/* Calculate needed buffer space. */
	if (Beg >= Size) {

		NeededSpace = Size;
		Wrap = SK_FALSE;
	}
	else {
		NeededSpace = Beg + Size;
		Wrap = SK_TRUE;
	}

	/*
	 * Check if enough buffer space is provided. Otherwise
	 * free some entries. Leave one byte space between begin
	 * and end of buffer to make it possible to detect whether
	 * the buffer is full or empty.
	 */
	while (BufFree < NeededSpace + 1) {

		if (End == 0) {

			End = SK_PNMI_TRAP_QUEUE_LEN;
		}

		EntrySize = (unsigned int)*((unsigned char *)pBuf + End - 1);
		BufFree += EntrySize;
		End -= EntrySize;
#ifdef DEBUG
		SK_MEMSET(pBuf + End, (char)(-1), EntrySize);
#endif /* DEBUG */
		if (End == BufPad) {
#ifdef DEBUG
			SK_MEMSET(pBuf, (char)(-1), End);
#endif /* DEBUG */
			BufFree += End;
			End = 0;
			BufPad = 0;
		}
	}

	/*
	 * Insert new entry as first entry. Newest entries are
	 * stored at the beginning of the queue.
	 */
	if (Wrap) {

		BufPad = Beg;
		Beg = SK_PNMI_TRAP_QUEUE_LEN - Size;
	}
	else {
		Beg = Beg - Size;
	}
	BufFree -= NeededSpace;

	/* Save the current offsets. */
	pAC->Pnmi.TrapQueueBeg = Beg;
	pAC->Pnmi.TrapQueueEnd = End;
	pAC->Pnmi.TrapBufPad = BufPad;
	pAC->Pnmi.TrapBufFree = BufFree;

	/* Initialize the trap entry. */
	*(pBuf + Beg + Size - 1) = (char)Size;
	*(pBuf + Beg) = (char)Size;
	Val32 = (pAC->Pnmi.TrapUnique) ++;
	SK_PNMI_STORE_U32(pBuf + Beg + 1, Val32);
	SK_PNMI_STORE_U32(pBuf + Beg + 1 + sizeof(SK_U32), TrapId);
	Val64 = SK_PNMI_HUNDREDS_SEC(SkOsGetTime(pAC));
	SK_PNMI_STORE_U64(pBuf + Beg + 1 + 2 * sizeof(SK_U32), Val64);

	return (pBuf + Beg);
}

/*****************************************************************************
 *
 * CopyTrapQueue - Copies the trap buffer for the TRAP OID
 *
 * Description:
 *	On a query of the TRAP OID the trap buffer contents will be
 *	copied continuously to the request buffer, which must be large
 *	enough. No length check is performed.
 *
 * Returns:
 *	Nothing
 */
PNMI_STATIC void CopyTrapQueue(
SK_AC *pAC,		/* Pointer to adapter context */
char *pDstBuf)		/* Buffer to which the queued traps will be copied */
{
	unsigned int	BufPad = pAC->Pnmi.TrapBufPad;
	unsigned int	Trap = pAC->Pnmi.TrapQueueBeg;
	unsigned int	End = pAC->Pnmi.TrapQueueEnd;
	char		*pBuf = &pAC->Pnmi.TrapBuf[0];
	unsigned int	Len;
	unsigned int	DstOff = 0;

	while (Trap != End) {

		Len = (unsigned int)*(pBuf + Trap);

		/*
		 * Last byte containing a copy of the length will
		 * not be copied.
		 */
		*(pDstBuf + DstOff) = (char)(Len - 1);
		SK_MEMCPY(pDstBuf + DstOff + 1, pBuf + Trap + 1, Len - 2);
		DstOff += Len - 1;

		Trap += Len;
		if (Trap == SK_PNMI_TRAP_QUEUE_LEN) {

			Trap = BufPad;
		}
	}
}

/*****************************************************************************
 *
 * GetTrapQueueLen - Get the length of the trap buffer
 *
 * Description:
 *	Evaluates the number of currently stored traps and the needed
 *	buffer size to retrieve them.
 *
 * Returns:
 *	Nothing
 */
PNMI_STATIC void GetTrapQueueLen(
SK_AC *pAC,		/* Pointer to adapter context */
unsigned int *pLen,	/* Length in Bytes of all queued traps */
unsigned int *pEntries)	/* Returns number of trapes stored in queue */
{
	unsigned int	BufPad = pAC->Pnmi.TrapBufPad;
	unsigned int	Trap = pAC->Pnmi.TrapQueueBeg;
	unsigned int	End = pAC->Pnmi.TrapQueueEnd;
	char		*pBuf = &pAC->Pnmi.TrapBuf[0];
	unsigned int	Len;
	unsigned int	Entries = 0;
	unsigned int	TotalLen = 0;

	while (Trap != End) {

		Len = (unsigned int)*(pBuf + Trap);
		TotalLen += Len - 1;
		Entries ++;

		Trap += Len;
		if (Trap == SK_PNMI_TRAP_QUEUE_LEN) {

			Trap = BufPad;
		}
	}

	*pEntries = Entries;
	*pLen = TotalLen;
}

/*****************************************************************************
 *
 * QueueSimpleTrap - Store a simple trap to the trap buffer
 *
 * Description:
 *	A simple trap is a trap with now additional data. It consists
 *	simply of a trap code.
 *
 * Returns:
 *	Nothing
 */
PNMI_STATIC void QueueSimpleTrap(
SK_AC *pAC,		/* Pointer to adapter context */
SK_U32 TrapId)		/* Type of sensor trap */
{
	GetTrapEntry(pAC, TrapId, SK_PNMI_TRAP_SIMPLE_LEN);
}

/*****************************************************************************
 *
 * QueueSensorTrap - Stores a sensor trap in the trap buffer
 *
 * Description:
 *	Gets an entry in the trap buffer and fills it with sensor related
 *	data.
 *
 * Returns:
 *	Nothing
 */
PNMI_STATIC void QueueSensorTrap(
SK_AC *pAC,			/* Pointer to adapter context */
SK_U32 TrapId,			/* Type of sensor trap */
unsigned int SensorIndex)	/* Index of sensor which caused the trap */
{
	char		*pBuf;
	unsigned int	Offset;
	unsigned int	DescrLen;
	SK_U32		Val32;

	/* Get trap buffer entry. */
	DescrLen = SK_STRLEN(pAC->I2c.SenTable[SensorIndex].SenDesc);

	pBuf = GetTrapEntry(pAC, TrapId,
		SK_PNMI_TRAP_SENSOR_LEN_BASE + DescrLen);
	Offset = SK_PNMI_TRAP_SIMPLE_LEN;

	/* Store additionally sensor trap related data. */
	Val32 = OID_SKGE_SENSOR_INDEX;
	SK_PNMI_STORE_U32(pBuf + Offset, Val32);
	*(pBuf + Offset + 4) = 4;
	Val32 = (SK_U32)SensorIndex;
	SK_PNMI_STORE_U32(pBuf + Offset + 5, Val32);
	Offset += 9;

	Val32 = (SK_U32)OID_SKGE_SENSOR_DESCR;
	SK_PNMI_STORE_U32(pBuf + Offset, Val32);
	*(pBuf + Offset + 4) = (char)DescrLen;
	SK_MEMCPY(pBuf + Offset + 5, pAC->I2c.SenTable[SensorIndex].SenDesc,
		DescrLen);
	Offset += DescrLen + 5;

	Val32 = OID_SKGE_SENSOR_TYPE;
	SK_PNMI_STORE_U32(pBuf + Offset, Val32);
	*(pBuf + Offset + 4) = 1;
	*(pBuf + Offset + 5) = (char)pAC->I2c.SenTable[SensorIndex].SenType;
	Offset += 6;

	Val32 = OID_SKGE_SENSOR_VALUE;
	SK_PNMI_STORE_U32(pBuf + Offset, Val32);
	*(pBuf + Offset + 4) = 4;
	Val32 = (SK_U32)pAC->I2c.SenTable[SensorIndex].SenValue;
	SK_PNMI_STORE_U32(pBuf + Offset + 5, Val32);
}

/*****************************************************************************
 *
 * QueueRlmtNewMacTrap - Store a port switch trap in the trap buffer
 *
 * Description:
 *	Nothing further to explain.
 *
 * Returns:
 *	Nothing
 */
PNMI_STATIC void QueueRlmtNewMacTrap(
SK_AC *pAC,		/* Pointer to adapter context */
unsigned int ActiveMac)	/* Index (0..n) of the currently active port */
{
	char	*pBuf;
	SK_U32	Val32;

	pBuf = GetTrapEntry(pAC, OID_SKGE_TRAP_RLMT_CHANGE_PORT,
		SK_PNMI_TRAP_RLMT_CHANGE_LEN);

	Val32 = OID_SKGE_RLMT_PORT_ACTIVE;
	SK_PNMI_STORE_U32(pBuf + SK_PNMI_TRAP_SIMPLE_LEN, Val32);
	*(pBuf + SK_PNMI_TRAP_SIMPLE_LEN + 4) = 1;
	*(pBuf + SK_PNMI_TRAP_SIMPLE_LEN + 5) = (char)ActiveMac;
}

/*****************************************************************************
 *
 * QueueRlmtPortTrap - Store port related RLMT trap to trap buffer
 *
 * Description:
 *	Nothing further to explain.
 *
 * Returns:
 *	Nothing
 */
PNMI_STATIC void QueueRlmtPortTrap(
SK_AC *pAC,		/* Pointer to adapter context */
SK_U32 TrapId,		/* Type of RLMT port trap */
unsigned int PortIndex)	/* Index of the port, which changed its state */
{
	char	*pBuf;
	SK_U32	Val32;

	pBuf = GetTrapEntry(pAC, TrapId, SK_PNMI_TRAP_RLMT_PORT_LEN);

	Val32 = OID_SKGE_RLMT_PORT_INDEX;
	SK_PNMI_STORE_U32(pBuf + SK_PNMI_TRAP_SIMPLE_LEN, Val32);
	*(pBuf + SK_PNMI_TRAP_SIMPLE_LEN + 4) = 1;
	*(pBuf + SK_PNMI_TRAP_SIMPLE_LEN + 5) = (char)PortIndex;
}

/*****************************************************************************
 *
 * CopyMac - Copies a MAC address
 *
 * Description:
 *	Nothing further to explain.
 *
 * Returns:
 *	Nothing
 */
PNMI_STATIC void CopyMac(
char		*pDst,	/* Pointer to destination buffer */
SK_MAC_ADDR *pMac)	/* Pointer of Source */
{
	int	i;

	for (i = 0; i < sizeof(SK_MAC_ADDR); i ++) {

		*(pDst + i) = pMac->a[i];
	}
}

#ifdef SK_POWER_MGMT
/*****************************************************************************
 *
 * PowerManagement - OID handler function of PowerManagement OIDs
 *
 * Description:
 *	The code is simple. No description necessary.
 *
 * Returns:
 *	SK_PNMI_ERR_OK           The request was successfully performed.
 *	SK_PNMI_ERR_GENERAL      A general severe internal error occurred.
 *	SK_PNMI_ERR_TOO_SHORT    The passed buffer is too short to contain
 *	                         the correct data (e.g. a 32bit value is
 *	                         needed, but a 16 bit value was passed).
 *	SK_PNMI_ERR_UNKNOWN_INST The requested instance of the OID doesn't
 *                               exist (e.g. port instance 3 on a two port
 *	                         adapter.
 */

PNMI_STATIC int PowerManagement(
SK_AC *pAC,		/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
int Action,		/* Get/PreSet/Set action */
SK_U32 Id,		/* Object ID that is to be processed */
char *pBuf,		/* Buffer to which to mgmt data will be retrieved */
unsigned int *pLen,	/* On call: buffer length. On return: used buffer */
SK_U32 Instance,	/* Instance (1..n) that is to be queried or -1 */
unsigned int TableIndex, /* Index to the Id table */
SK_U32 NetIndex)	/* NetIndex (0..n), in single net mode always zero */
{
	int i;
	unsigned int HwPortIndex;
	SK_U32	RetCode = SK_PNMI_ERR_GENERAL;

	/* Check instance. We only handle single instance variables. */
	if ((Instance != (SK_U32)(-1))) {

		*pLen = 0;
		return (SK_PNMI_ERR_UNKNOWN_INST);
	}

	/* Get hardware port index */
	HwPortIndex = NetIndex;

	/* Check length. */
	switch (Id) {

#ifdef SK_POWER_MGMT_EX
	case OID_PM_PARAMETERS:
		if (*pLen < sizeof(NDIS_PM_PARAMETERS)) {

			*pLen = sizeof(NDIS_PM_PARAMETERS);
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		break;
#endif /* SK_POWER_MGMT_EX */

	case OID_PNP_CAPABILITIES:
		if (*pLen < sizeof(SK_PNP_CAPABILITIES)) {

			*pLen = sizeof(SK_PNP_CAPABILITIES);
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		break;

	case OID_PNP_SET_POWER:
	case OID_PNP_QUERY_POWER:
		if (*pLen < sizeof(SK_DEVICE_POWER_STATE)) {

			*pLen = sizeof(SK_DEVICE_POWER_STATE);
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		break;

#ifdef SK_POWER_MGMT_EX
	case OID_PM_ADD_WOL_PATTERN:

		if (*pLen < sizeof(NDIS_PM_WOL_PATTERN)) {

			*pLen = sizeof(NDIS_PM_WOL_PATTERN);
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		break;
	case OID_PM_REMOVE_WOL_PATTERN:
		if (*pLen < sizeof(ULONG)) {

			*pLen = sizeof(ULONG);
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		break;
	case OID_PM_ADD_PROTOCOL_OFFLOAD:
		if (*pLen < sizeof(NDIS_PM_PROTOCOL_OFFLOAD)) {

			*pLen = sizeof(NDIS_PM_PROTOCOL_OFFLOAD);
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		break;
	case OID_PM_REMOVE_PROTOCOL_OFFLOAD:
		if (*pLen < sizeof(ULONG)) {

			*pLen = sizeof(ULONG);
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		break;

#endif /* SK_POWER_MGMT_EX */

	case OID_PNP_ADD_WAKE_UP_PATTERN:
	case OID_PNP_REMOVE_WAKE_UP_PATTERN:
		if (*pLen < sizeof(SK_PM_PACKET_PATTERN)) {

			*pLen = sizeof(SK_PM_PACKET_PATTERN);
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		break;

	case OID_PNP_ENABLE_WAKE_UP:
		if (*pLen < sizeof(SK_U32)) {

			*pLen = sizeof(SK_U32);
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		break;
	}

	/* Perform action. */
	if (Action == SK_PNMI_GET) {

		/* Get value. */
		switch (Id) {

#ifdef SK_POWER_MGMT_EX
		case OID_PM_PARAMETERS:
			SkPowerQueryPmParameters(pAC, IoC, HwPortIndex, pBuf, pLen);
			break;
#endif /* SK_POWER_MGMT_EX */

		case OID_PNP_CAPABILITIES:
			RetCode = SkPowerQueryPnPCapabilities(pAC, IoC, pBuf, pLen);
			break;

		case OID_PNP_QUERY_POWER:
			/*
			 * The Windows DDK describes: An OID_PNP_QUERY_POWER requests
			 * the miniport to indicate whether it can transition its NIC
			 * to the low-power state.
			 * A miniport driver must always return NDIS_STATUS_SUCCESS
			 * to a query of OID_PNP_QUERY_POWER.
			 */
			*pLen = sizeof(SK_DEVICE_POWER_STATE);
			RetCode = SK_PNMI_ERR_OK;
			break;

			/*
			 * NDIS handles these OIDs as write-only.
			 * So in case of get action the buffer with written length = 0
			 * is returned.
			 */
#ifdef SK_POWER_MGMT_EX
		case OID_PM_ADD_WOL_PATTERN:
		case OID_PM_REMOVE_WOL_PATTERN:
		case OID_PM_ADD_PROTOCOL_OFFLOAD:
		case OID_PM_REMOVE_PROTOCOL_OFFLOAD:
#endif /* SK_POWER_MGMT_EX */
		case OID_PNP_SET_POWER:
		case OID_PNP_ADD_WAKE_UP_PATTERN:
		case OID_PNP_REMOVE_WAKE_UP_PATTERN:
			SK_DBG_MSG(pAC, SK_DBGMOD_PNMI, SK_DBGCAT_ERR, ("Query of OID 0x%x not supported\n", Id));
			*pLen = 0;
			RetCode = SK_PNMI_ERR_NOT_SUPPORTED;
			break;

		case OID_PNP_ENABLE_WAKE_UP:
			RetCode = SkPowerGetEnableWakeUp(pAC, IoC, HwPortIndex, pBuf, pLen);
			break;

		default:
			RetCode = SK_PNMI_ERR_GENERAL;
			break;
		}

		return (RetCode);
	}

	/* Perform PRESET or SET. */

	/* The POWER module does not support PRESET action. */
	if (Action == SK_PNMI_PRESET) {

		return (SK_PNMI_ERR_OK);
	}

	i = HwPortIndex;

	switch (Id) {

#ifdef SK_POWER_MGMT_EX
	case OID_PM_PARAMETERS:
		for (i = 0; i < pAC->GIni.GIMacsFound; i++) {
			if ((RetCode = SkPowerSetPMParameters(pAC, IoC, HwPortIndex, pBuf, pLen))) {
				break;
			}
		}

		break;
#endif /* SK_POWER_MGMT_EX */

	case OID_PNP_SET_POWER:
		/* Dual net mode? */
		for (i = 0; i < pAC->GIni.GIMacsFound; i++) {
			if ((RetCode = SkPowerSetPower(pAC, IoC, i, pBuf, pLen))) {
				break;
			}
		}
		break;

#ifdef SK_POWER_MGMT_EX
	case OID_PM_ADD_WOL_PATTERN:
		for (i = 0; i < pAC->GIni.GIMacsFound; i++) {
			if ((RetCode = SkPowerAddWoLPattern(pAC,IoC,i,pBuf,pLen))) {
				break;
			}
		}
		break;

#endif /* SK_POWER_MGMT_EX */

	case OID_PNP_ADD_WAKE_UP_PATTERN:
		for (i = 0; i < pAC->GIni.GIMacsFound; i++) {
			if ((RetCode = SkPowerAddWakeUpPattern(pAC, IoC, i, pBuf, pLen))) {
				break;
			}
		}
		break;

#ifdef SK_POWER_MGMT_EX
	case OID_PM_REMOVE_WOL_PATTERN:
		for (i = 0; i < pAC->GIni.GIMacsFound; i++) {
			if ((RetCode = SkPowerRemoveWolPattern(pAC, IoC, i, pBuf, pLen))) {
				break;
			}
		}
		break;

	case OID_PM_ADD_PROTOCOL_OFFLOAD:
		for (i = 0; i < pAC->GIni.GIMacsFound; i++) {
			if ((RetCode = SkPowerAddProtocolOffload(pAC, IoC, i, pBuf, pLen))) {
				break;
			}
		}
		break;

	case OID_PM_REMOVE_PROTOCOL_OFFLOAD:
		for (i = 0; i < pAC->GIni.GIMacsFound; i++) {
			if ((RetCode = SkPowerRemoveProtocolOffload(pAC, IoC, i, pBuf, pLen))) {
				break;
			}
		}
		break;

#endif /* SK_POWER_MGMT_EX */

	case OID_PNP_REMOVE_WAKE_UP_PATTERN:
		for (i = 0; i < pAC->GIni.GIMacsFound; i++) {
			if ((RetCode = SkPowerRemoveWakeUpPattern(pAC, IoC, i, pBuf, pLen))) {
				break;
			}
		}
		break;

	case OID_PNP_ENABLE_WAKE_UP:
		for (i = 0; i < pAC->GIni.GIMacsFound; i++) {
			if ((RetCode = SkPowerSetEnableWakeUp(pAC, IoC, i, pBuf, pLen))) {
				break;
			}
		}
		break;

	default:
		RetCode = SK_PNMI_ERR_READ_ONLY;
	}

	return (RetCode);
}
#endif /* SK_POWER_MGMT */

#ifdef SK_DIAG_SUPPORT
/*****************************************************************************
 *
 * DiagActions - OID handler function of Diagnostic driver
 *
 * Description:
 *	The code is simple. No description necessary.
 *
 * Returns:
 *	SK_PNMI_ERR_OK           The request was successfully performed.
 *	SK_PNMI_ERR_GENERAL      A general severe internal error occurred.
 *	SK_PNMI_ERR_TOO_SHORT    The passed buffer is too short to contain
 *	                         the correct data (e.g. a 32bit value is
 *	                         needed, but a 16 bit value was passed).
 *	SK_PNMI_ERR_UNKNOWN_INST The requested instance of the OID doesn't exist
 *                           (e.g. port instance 3 on a two port adapter)
 */

PNMI_STATIC int DiagActions(
SK_AC *pAC,		/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
int Action,		/* GET/PRESET/SET action */
SK_U32 Id,		/* Object ID that is to be processed */
char *pBuf,		/* Buffer used for the management data transfer */
unsigned int *pLen,	/* On call: pBuf buffer length. On return: used buffer */
SK_U32 Instance,	/* Instance (1..n) that is to be queried or -1 */
unsigned int TableIndex, /* Index to the Id table */
SK_U32 NetIndex)	/* NetIndex (0..n), in single net mode always zero */
{
	SK_U32	DiagStatus;
	SK_U32	RetCode = SK_PNMI_ERR_GENERAL;

	/* Check instance. We only handle single instance variables. */
	if (Instance != (SK_U32)(-1) && Instance != 1) {

		*pLen = 0;
		return (SK_PNMI_ERR_UNKNOWN_INST);
	}

	/* Check length. */
	switch (Id) {

	case OID_SKGE_DIAG_MODE:
		if (*pLen < sizeof(SK_U32)) {

			*pLen = sizeof(SK_U32);
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		break;

	default:
		SK_ERR_LOG(pAC, SK_ERRCL_SW, SK_PNMI_ERR040, SK_PNMI_ERR040MSG);
		*pLen = 0;
		return (SK_PNMI_ERR_GENERAL);
	}

	/* Perform action. */
	if (Action == SK_PNMI_GET) {

		/* Get value. */
		switch (Id) {

		case OID_SKGE_DIAG_MODE:
			DiagStatus = pAC->Pnmi.DiagAttached;
			SK_PNMI_STORE_U32(pBuf, DiagStatus);
			*pLen = sizeof(SK_U32);
			RetCode = SK_PNMI_ERR_OK;
			break;

		default:
			*pLen = 0;
			RetCode = SK_PNMI_ERR_GENERAL;
			break;
		}
		return (RetCode);
	}

	/* From here SET or PRESET value. */

	/* PRESET value is not supported. */
	if (Action == SK_PNMI_PRESET) {

		return (SK_PNMI_ERR_OK);
	}

	/* SET value. */
	switch (Id) {

		case OID_SKGE_DIAG_MODE:
			/* Handle the SET. */
			switch (*pBuf) {
			/* Attach the DIAG to this adapter. */
			case SK_DIAG_ATTACHED:
				/* Check if we come from running. */
				if (pAC->Pnmi.DiagAttached == SK_DIAG_RUNNING) {

					RetCode = SkDrvLeaveDiagMode(pAC);
				}
				else if (pAC->Pnmi.DiagAttached == SK_DIAG_IDLE) {

					RetCode = SK_PNMI_ERR_OK;
				}
				else {

					RetCode = SK_PNMI_ERR_GENERAL;
				}

				if (RetCode == SK_PNMI_ERR_OK) {

					pAC->Pnmi.DiagAttached = SK_DIAG_ATTACHED;
				}
				break;

			/* Enter the DIAG mode in the driver. */
			case SK_DIAG_RUNNING:
				RetCode = SK_PNMI_ERR_OK;

				/*
				 * If DiagAttached is set, we can tell the driver
				 * to enter the DIAG mode.
				 */
				if (pAC->Pnmi.DiagAttached == SK_DIAG_ATTACHED) {
					/* If DiagMode is not active, we can enter it. */
					if (!pAC->DiagModeActive) {

						RetCode = SkDrvEnterDiagMode(pAC);
					}
					else {

						RetCode = SK_PNMI_ERR_GENERAL;
					}
				}
				else {

					RetCode = SK_PNMI_ERR_GENERAL;
				}

				if (RetCode == SK_PNMI_ERR_OK) {

					pAC->Pnmi.DiagAttached = SK_DIAG_RUNNING;
				}
				break;

			case SK_DIAG_IDLE:
				/* Check if we come from running. */
				if (pAC->Pnmi.DiagAttached == SK_DIAG_RUNNING) {

					RetCode = SkDrvLeaveDiagMode(pAC);
				}
				else if (pAC->Pnmi.DiagAttached == SK_DIAG_ATTACHED) {

					RetCode = SK_PNMI_ERR_OK;
				}
				else {

					RetCode = SK_PNMI_ERR_GENERAL;
				}

				if (RetCode == SK_PNMI_ERR_OK) {

					pAC->Pnmi.DiagAttached = SK_DIAG_IDLE;
				}
				break;

			default:
				RetCode = SK_PNMI_ERR_BAD_VALUE;
				break;
			}
			break;

		default:
			RetCode = SK_PNMI_ERR_GENERAL;
	}

	if (RetCode == SK_PNMI_ERR_OK) {
		*pLen = sizeof(SK_U32);
	}
	else {

		*pLen = 0;
	}
	return (RetCode);
}
#endif /* SK_DIAG_SUPPORT */

/*****************************************************************************
*
* VctGetDspLength
*
* Description:
*	Calculates the DSP length
*
* Returns:
 *	Nothing
*/
static void VctGetDspLength(
SK_AC		*pAC,
SK_IOC		IoC,
SK_U32		Port,
SK_PNMI_VCT	*pvctInfo)
{
	SK_U8	port_cable_length;
	SK_U8	len;

	/* Calculate DSP length */
	if (pAC->GIni.GP[Port].PLinkSpeedUsed != SK_LSPEED_1000MBPS) {

		pvctInfo->CableLen = DSP_CABLE_RANGE_NOT_SUPPORTED;
		return;
	}

	port_cable_length = pAC->GIni.GP[Port].PCableLen;

	if (HW_HAS_NEWER_PHY(pAC)) {

		if (port_cable_length < 13) {
			len = 1;
		}
		else if (port_cable_length < 36) {
			len = 3 * (port_cable_length) - 38;
		}
		else if (port_cable_length < 68) {
			len = port_cable_length + (port_cable_length / 2);
		}
		else {
			len = port_cable_length + 37;
		}

		if (len <= DSP_CABLE_RANGE_UNDER_50_END) {
			pvctInfo->CableLen = DSP_CABLE_RANGE_UNDER_50;
		}
		else if (len <= DSP_CABLE_RANGE_50_TO_80_END) {
			pvctInfo->CableLen = DSP_CABLE_RANGE_50_TO_80;
		}
		else if (len <= DSP_CABLE_RANGE_80_TO_140_END) {
			pvctInfo->CableLen = DSP_CABLE_RANGE_80_TO_150;
		}
		else {
			pvctInfo->CableLen = DSP_CABLE_RANGE_ABOVE_140;
		}
	}
	else {
		pvctInfo->CableLen = port_cable_length;
	}
}

/*****************************************************************************
 *
 * Vct - OID handler function of OIDs for Virtual Cable Tester (VCT)
 *
 * Description:
 *	The code is simple. No description necessary.
 *
 * Returns:
 *	SK_PNMI_ERR_OK           The request was performed successfully.
 *	SK_PNMI_ERR_GENERAL      A general severe internal error occurred.
 *	SK_PNMI_ERR_TOO_SHORT    The passed buffer is too short to contain
 *	                         the correct data (e.g. a 32bit value is
 *	                         needed, but a 16 bit value was passed).
 *	SK_PNMI_ERR_UNKNOWN_INST The requested instance of the OID doesn't
 *                           exist (e.g. port instance 3 on a two port
 *	                         adapter).
 *	SK_PNMI_ERR_READ_ONLY	 Only the Get action is allowed.
 *
 */

PNMI_STATIC int Vct(
SK_AC *pAC,		/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
int Action,		/* GET/PRESET/SET action */
SK_U32 Id,		/* Object ID that is to be processed */
char *pBuf,		/* Buffer used for the management data transfer */
unsigned int *pLen,	/* On call: pBuf buffer length. On return: used buffer */
SK_U32 Instance,	/* Instance (-1,2..n) that is to be queried */
unsigned int TableIndex, /* Index to the Id table */
SK_U32 NetIndex)	/* NetIndex (0..n), in single net mode always zero */
{
	SK_GEPORT	*pPrt;
	SK_PNMI_VCT	*pVctBackupData;
	SK_U32		LogPortMax;
	SK_U32		PhysPortMax;
	SK_U32		PhysPortIndex;
	SK_U32		Limit;
	SK_U32		Offset;
	SK_U32		RetCode;
	int			i;
	SK_EVPARA	Para;

	RetCode = SK_PNMI_ERR_GENERAL;

	/* Calculate the port indexes from the instance. */
	PhysPortMax = pAC->GIni.GIMacsFound;
	LogPortMax = SK_PNMI_PORT_PHYS2LOG(PhysPortMax);

	/* Dual net mode? */
	if (pAC->Pnmi.DualNetActiveFlag) {
		LogPortMax--;
	}

	if ((Instance != (SK_U32) (-1))) {
		/*
		 * Get one instance of that OID, so check the instance range:
		 * There is no virtual port with an Instance == 1, so we get
		 * the values from one physical port only.
		 */
		if (pAC->Pnmi.DualNetActiveFlag) {
			PhysPortIndex = NetIndex;
		}
		else {
			if ((Instance < 2) || (Instance > LogPortMax)) {
				*pLen = 0;
				return (SK_PNMI_ERR_UNKNOWN_INST);
			}
			PhysPortIndex = Instance - 2;
		}
		Limit = PhysPortIndex + 1;
	}
	else {
		/*
		 * Instance == (SK_U32) (-1), so get all instances of that OID.
		 * There is no virtual port with an Instance == 1, so we get
		 * the values from all physical ports.
		 */

		/* DualNet mode */
		if (pAC->Pnmi.DualNetActiveFlag) {
			PhysPortIndex = NetIndex;
			Limit = PhysPortIndex + 1;
		}
		/* SingleNet mode */
		else {
			PhysPortIndex = 0;
			Limit = PhysPortMax;
		}
	}

	pPrt = &pAC->GIni.GP[PhysPortIndex];

	/* Check MAC type. */
	if ((Id != OID_SKGE_VCT_CAPABILITIES) &&
		(pPrt->PhyType != SK_PHY_MARV_COPPER)) {
		*pLen = 0;
		return (SK_PNMI_ERR_NOT_SUPPORTED);
	}

	/* Check action type. */
	if (Action == SK_PNMI_GET) {
		/* Check length. */
		switch (Id) {

		case OID_SKGE_VCT_GET:
			if (*pLen < (Limit - PhysPortIndex) * sizeof(SK_PNMI_VCT)) {
				*pLen = (Limit - PhysPortIndex) * sizeof(SK_PNMI_VCT);
				return (SK_PNMI_ERR_TOO_SHORT);
			}
			break;

		case OID_SKGE_VCT_STATUS:
		case OID_SKGE_VCT_CAPABILITIES:
			if (*pLen < (Limit - PhysPortIndex) * sizeof(SK_U8)) {
				*pLen = (Limit - PhysPortIndex) * sizeof(SK_U8);
				return (SK_PNMI_ERR_TOO_SHORT);
			}
			break;

		default:
			*pLen = 0;
			return (SK_PNMI_ERR_GENERAL);
		}

		/* Get value. */
		Offset = 0;
		for (; PhysPortIndex < Limit; PhysPortIndex++) {

			pPrt = &pAC->GIni.GP[PhysPortIndex];

			switch (Id) {

			case OID_SKGE_VCT_GET:
				if (!pPrt->PHWLinkUp &&
					(pAC->Pnmi.VctStatus[PhysPortIndex] & SK_PNMI_VCT_PENDING)) {

					RetCode = SkGmCableDiagStatus(pAC, IoC, PhysPortIndex, SK_FALSE);

					if (RetCode == 0) {

						/* VCT test is finished, so save the data. */
						VctGetResults(pAC, IoC, PhysPortIndex);

						Para.Para32[0] = PhysPortIndex;
						Para.Para32[1] = -1;
						SkEventQueue(pAC, SKGE_DRV, SK_DRV_PORT_RESET, Para);

						/* SkEventDispatcher(pAC, IoC); */
					}
				}

				/* Initialize backup data pointer. */
				pVctBackupData = &pAC->Pnmi.VctBackup[PhysPortIndex];

				/* Get all results. */
				CheckVctStatus(pAC, IoC, pBuf, Offset, PhysPortIndex);

				Offset++;

				VctGetDspLength(pAC, IoC, PhysPortIndex, pVctBackupData);

				*(pBuf + Offset) = pVctBackupData->CableLen;
				Offset++;
				for (i = 0; i < 4; i++) {

					SK_PNMI_STORE_U32((pBuf + Offset), pVctBackupData->MdiPairLen[i]);
					Offset += sizeof(SK_U32);
				}
				for (i = 0; i < 4; i++) {

					*(pBuf + Offset) = pVctBackupData->MdiPairSts[i];
					Offset++;
				}

				RetCode = SK_PNMI_ERR_OK;
				break;

			case OID_SKGE_VCT_STATUS:
				CheckVctStatus(pAC, IoC, pBuf, Offset, PhysPortIndex);

				Offset++;
				RetCode = SK_PNMI_ERR_OK;
				break;

			case OID_SKGE_VCT_CAPABILITIES:
				if (pPrt->PhyType != SK_PHY_MARV_COPPER) {
					*(pBuf + Offset) = SK_PNMI_VCT_NOT_SUPPORTED;
				}
				else {
					*(pBuf + Offset) = SK_PNMI_VCT_SUPPORTED_VERSION;
				}
				Offset++;

				RetCode = SK_PNMI_ERR_OK;
				break;

			default:
				*pLen = 0;
				return (SK_PNMI_ERR_GENERAL);
			}
		} /* for */
		*pLen = Offset;

		return (RetCode);
	} /* if SK_PNMI_GET */

	/*
	 * From here SET or PRESET action. Check if the passed
	 * buffer length is plausible.
	 */

	/* Check length. */
	switch (Id) {
	case OID_SKGE_VCT_SET:
		if (*pLen < (Limit - PhysPortIndex) * sizeof(SK_U32)) {
			*pLen = (Limit - PhysPortIndex) * sizeof(SK_U32);
			return (SK_PNMI_ERR_TOO_SHORT);
		}
		break;

	default:
		*pLen = 0;
		return (SK_PNMI_ERR_GENERAL);
	}

	/* Perform PRESET or SET. */

	/* VCT does not support PRESET action. */
	if (Action == SK_PNMI_PRESET) {

		return (SK_PNMI_ERR_OK);
	}

	Offset = 0;
	for (; PhysPortIndex < Limit; PhysPortIndex++) {

		pPrt = &pAC->GIni.GP[PhysPortIndex];

		switch (Id) {
		/* Start VCT test */
		case OID_SKGE_VCT_SET:
			if (!pPrt->PHWLinkUp) {
				SkGeStopPort(pAC, IoC, PhysPortIndex, SK_STOP_ALL, SK_SOFT_RST);

				RetCode = SkGmCableDiagStatus(pAC, IoC, PhysPortIndex, SK_TRUE);

				if (RetCode == 0) { /* RetCode: 0 => Start! */
					pAC->Pnmi.VctStatus[PhysPortIndex] |= SK_PNMI_VCT_PENDING;
					pAC->Pnmi.VctStatus[PhysPortIndex] &=
						~(SK_PNMI_VCT_NEW_VCT_DATA | SK_PNMI_VCT_LINK);

					/* Start VCT timer counter. */
					SK_MEMSET((char *)&Para, 0, sizeof(Para));
					Para.Para32[0] = PhysPortIndex;
					Para.Para32[1] = -1;

					SkTimerStart(pAC, IoC, &pAC->Pnmi.VctTimeout[PhysPortIndex],
						SK_PNMI_VCT_TIMER_CHECK, SKGE_PNMI, SK_PNMI_EVT_VCT_RESET, Para);

					SK_PNMI_STORE_U32((pBuf + Offset), RetCode);
					RetCode = SK_PNMI_ERR_OK;
				}
				else { /* RetCode: 2 => Running! */
					SK_PNMI_STORE_U32((pBuf + Offset), RetCode);
					RetCode = SK_PNMI_ERR_OK;
				}
			}
			else { /* RetCode: 4 => Link! */
				RetCode = 4;
				SK_PNMI_STORE_U32((pBuf + Offset), RetCode);
				RetCode = SK_PNMI_ERR_OK;
			}
			Offset += sizeof(SK_U32);
			break;

		default:
			*pLen = 0;
			return (SK_PNMI_ERR_GENERAL);
		}
	} /* for */
	*pLen = Offset;
	return (RetCode);

} /* Vct */


/*****************************************************************************
*
* VctGetResults
*
* Description:
*	Read and calculate the results
*
* Returns:
 *	Nothing
*/
PNMI_STATIC void VctGetResults(
SK_AC		*pAC,
SK_IOC		IoC,
SK_U32		Port)
{
	int			i;
	SK_U8		tmp_u8;
	SK_U32		tmp_u32;
	SK_U32		MinCableLen;
	SK_GEPORT	*pPrt;
	SK_PNMI_VCT *pVctInfo;
	int			ChipId;

	pPrt = &pAC->GIni.GP[Port];
	pVctInfo = &pAC->Pnmi.VctBackup[Port];

	ChipId = pAC->GIni.GIChipId;

	/* Calculate VCT length and status */
	if (ChipId == CHIP_ID_YUKON_EX || ChipId >= CHIP_ID_YUKON_SUPR) {
		/*
		 * For Yukon-Extreme PHY the previous VCT status determination is
		 * different and therefore the status result from PNMI is not valid.
		 * Calculate VCT status for each cable pair based on reflection
		 * amplitude polarity and amplitude value.
		 */
		for (i = 0; i < 4; i++) {
			/* Invalid VCT status for cable pair */
			if ((pPrt->PMdiPairLen[i]) == 0xff &&
				(pPrt->PMdiPairSts[i]) == 0xff) {
				pVctInfo->MdiPairSts[i] = SK_PNMI_VCT_TEST_FAIL;
			}
			/* No fault reported for cable pair */
			else if ((pPrt->PMdiPairLen[i]) == 0 &&
					 (pPrt->PMdiPairSts[i]) == 0) {
				pVctInfo->MdiPairSts[i] = SK_PNMI_VCT_NORMAL_CABLE;
			}
			/*
			 * Positive reflection amplitude polarity and amplitude value
			 * greater than 40 --> open/broken cable pair.
			 */
			else if ((pPrt->PMdiPairSts[i] & 0x80) &&
					 ((pPrt->PMdiPairSts[i] & 0x7f) > 40)) {
				pVctInfo->MdiPairSts[i] = SK_PNMI_VCT_OPEN_CABLE;
			}
			/*
			 * Negative reflection amplitude polarity and amplitude value
			 * greater than 40 --> short at same cable pair.
			 */
			else if (((pPrt->PMdiPairSts[i] & 0x80) == 0) &&
					 ((pPrt->PMdiPairSts[i] & 0x7f) > 40)) {
				pVctInfo->MdiPairSts[i] = SK_PNMI_VCT_SHORT_CABLE;
			}
			/*
			 * Positive or negative reflection amplitude polarity and amplitude value
			 * less than or equal to 40 --> impedance missmatch at same cable pair.
			 */
			else {
				pVctInfo->MdiPairSts[i] = SK_PNMI_VCT_IMPEDANCE_MISMATCH;
			}
		}
	}
	else {
		for (i = 0; i < 4; i++) {
			if ((ChipId == CHIP_ID_YUKON_FE ||
				 ChipId == CHIP_ID_YUKON_FE_P) && (i > 1)) {
				pVctInfo->MdiPairSts[i] = SK_PNMI_VCT_NOT_PRESENT;
			}
			else {
				if ((pPrt->PMdiPairSts[i] == SK_PNMI_VCT_NORMAL_CABLE) &&
					(pPrt->PMdiPairLen[i] > 28) && (pPrt->PMdiPairLen[i] < 0xff)) {
					pVctInfo->MdiPairSts[i] = SK_PNMI_VCT_IMPEDANCE_MISMATCH;
				}
				else {
					pVctInfo->MdiPairSts[i] = pPrt->PMdiPairSts[i];
				}
			}
		}
	}

	/*
	 * Calculate length for each cable pair.
	 * Ignore values <= MinLength, the linear factor is 4/5.
	 */
	switch (ChipId) {
	case CHIP_ID_YUKON_FE:
		MinCableLen = 25;
		break;
	case CHIP_ID_YUKON_FE_P:
		MinCableLen = 17;
		break;
	case CHIP_ID_YUKON_EX:
	case CHIP_ID_YUKON_SUPR:
	case CHIP_ID_YUKON_UL_2:
		MinCableLen = 13;
		break;
	case CHIP_ID_YUKON_OPT:
	case CHIP_ID_YUKON_PRM:
		MinCableLen = 7;
		break;
	default:
		MinCableLen = 35;
		break;
	}

	for (i=0; i<4; i++) {
		if ((pPrt->PMdiPairLen[i] > (SK_U32)MinCableLen) &&
			(pPrt->PMdiPairLen[i] < (SK_U32)0xff)) {

			pVctInfo->MdiPairLen[i] = 1000UL *
				((pPrt->PMdiPairLen[i] - MinCableLen) * 4 / 5);
		}
		else {
			/* No cable or short cable. */
			pVctInfo->MdiPairLen[i] = 0;
		}
	}

	/* Swap pair length and status for Yukon-FE due to HW design issue */
	if (ChipId  == CHIP_ID_YUKON_FE) {

		tmp_u8 = pVctInfo->MdiPairSts[0];
		pVctInfo->MdiPairSts[0] = pVctInfo->MdiPairSts[1];
		pVctInfo->MdiPairSts[1] = tmp_u8;

		tmp_u32 = pVctInfo->MdiPairLen[0];
		pVctInfo->MdiPairLen[0] = pVctInfo->MdiPairLen[1];
		pVctInfo->MdiPairLen[1] = tmp_u32;
	}

	pAC->Pnmi.VctStatus[Port] &= ~SK_PNMI_VCT_PENDING;
	pAC->Pnmi.VctStatus[Port] |= (SK_PNMI_VCT_NEW_VCT_DATA | SK_PNMI_VCT_TEST_DONE);
	pAC->Pnmi.VctStatus[Port] &= ~SK_PNMI_VCT_OLD_VCT_DATA;

} /* GetVctResults */


PNMI_STATIC void CheckVctStatus(
SK_AC		*pAC,
SK_IOC		IoC,
char		*pBuf,
SK_U32		Offset,
SK_U32		PhysPortIndex)
{
	SK_GEPORT 	*pPrt;
	SK_PNMI_VCT	*pVctData;
	SK_U8		VctStatus;
	SK_U32		RetCode;

	pPrt = &pAC->GIni.GP[PhysPortIndex];

	pVctData = (SK_PNMI_VCT *) (pBuf + Offset);
	pVctData->VctStatus = SK_PNMI_VCT_NONE;

	VctStatus = pAC->Pnmi.VctStatus[PhysPortIndex];

	if (!pPrt->PHWLinkUp) {

		/* Was a VCT test ever made before? */
		if (VctStatus & SK_PNMI_VCT_TEST_DONE) {
			if (VctStatus & SK_PNMI_VCT_LINK) {
				pVctData->VctStatus |= SK_PNMI_VCT_OLD_VCT_DATA;
			}
			else {
				pVctData->VctStatus |= SK_PNMI_VCT_NEW_VCT_DATA;
			}
		}

		/* Check VCT test status. */
		RetCode = SkGmCableDiagStatus(pAC, IoC, PhysPortIndex, SK_FALSE);

		if (RetCode == 2) { /* VCT test is running. */
			pVctData->VctStatus |= SK_PNMI_VCT_RUNNING;
		}
		else { /* VCT data was copied to pAC here. Check PENDING state. */
			if (VctStatus & SK_PNMI_VCT_PENDING) {
				pVctData->VctStatus |= SK_PNMI_VCT_NEW_VCT_DATA;
			}
		}

		if (pPrt->PCableLen != 0xff) { /* Old DSP value. */
			pVctData->VctStatus |= SK_PNMI_VCT_OLD_DSP_DATA;
		}
	}
	else {
		/* Was a VCT test ever made before? */
		if (VctStatus & SK_PNMI_VCT_TEST_DONE) {
			pVctData->VctStatus &= ~SK_PNMI_VCT_NEW_VCT_DATA;
			pVctData->VctStatus |= SK_PNMI_VCT_OLD_VCT_DATA;
		}

		/* DSP only valid in 100/1000 modes. */
		if (pPrt->PLinkSpeedUsed != SK_LSPEED_STAT_10MBPS) {
			pVctData->VctStatus |= SK_PNMI_VCT_NEW_DSP_DATA;
		}
	}
} /* CheckVctStatus */


/*****************************************************************************
 *
 *      SkPnmiGenIoctl - Handles new generic PNMI IOCTL, calls the needed
 *                       PNMI function depending on the subcommand and
 *                       returns all data belonging to the complete database
 *                       or OID request.
 *
 * Description:
 *	Looks up the requested subcommand, calls the corresponding handler
 *	function and passes all required parameters to it.
 *	The function is called by the driver. It is needed to handle the new
 *  generic PNMI IOCTL. This IOCTL is given to the driver and contains both
 *  the OID and a subcommand to decide what kind of request has to be done.
 *
 * Returns:
 *	SK_PNMI_ERR_OK           The request was successfully performed
 *	SK_PNMI_ERR_GENERAL      A general severe internal error occurred
 *	SK_PNMI_ERR_TOO_SHORT    The passed buffer is too short to take
 *	                         the data.
 *	SK_PNMI_ERR_UNKNOWN_OID  The requested OID is unknown
 *	SK_PNMI_ERR_UNKNOWN_INST The requested instance of the OID doesn't
 *                           exist (e.g. port instance 3 on a two port
 *	                         adapter.
 */
int SkPnmiGenIoctl(
SK_AC		*pAC,		/* Pointer to adapter context struct */
SK_IOC		IoC,		/* I/O context */
void		*pBuf,		/* Buffer used for the management data transfer */
unsigned int *pLen,		/* Length of buffer */
SK_U32		NetIndex)	/* NetIndex (0..n), in single net mode always zero */
{
SK_I32	Mode;			/* Store value of subcommand. */
SK_U32	Oid;			/* Store value of OID. */
int		ReturnCode;		/* Store return value to show status of PNMI action. */
int 	HeaderLength;	/* Length of desired action plus OID. */

	ReturnCode = SK_PNMI_ERR_GENERAL;

	SK_MEMCPY(&Mode, pBuf, sizeof(SK_I32));
	SK_MEMCPY(&Oid, (char *)pBuf + sizeof(SK_I32), sizeof(SK_U32));
	HeaderLength = sizeof(SK_I32) + sizeof(SK_U32);
	*pLen = *pLen - HeaderLength;
	SK_MEMCPY((char *)pBuf + sizeof(SK_I32), (char *)pBuf + HeaderLength, *pLen);

	switch (Mode) {
	case SK_GET_SINGLE_VAR:
		ReturnCode = SkPnmiGetVar(pAC, IoC, Oid,
				(char *)pBuf + sizeof(SK_I32), pLen,
				((SK_U32) (-1)), NetIndex);
		SK_PNMI_STORE_U32(pBuf, ReturnCode);
		*pLen = *pLen + sizeof(SK_I32);
		break;
	case SK_PRESET_SINGLE_VAR:
		ReturnCode = SkPnmiPreSetVar(pAC, IoC, Oid,
				(char *)pBuf + sizeof(SK_I32), pLen,
				((SK_U32) (-1)), NetIndex);
		SK_PNMI_STORE_U32(pBuf, ReturnCode);
		*pLen = *pLen + sizeof(SK_I32);
		break;
	case SK_SET_SINGLE_VAR:
		ReturnCode = SkPnmiSetVar(pAC, IoC, Oid,
				(char *)pBuf + sizeof(SK_I32), pLen,
				((SK_U32) (-1)), NetIndex);
		SK_PNMI_STORE_U32(pBuf, ReturnCode);
		*pLen = *pLen + sizeof(SK_I32);
		break;
	case SK_GET_FULL_MIB:
		ReturnCode = SkPnmiGetStruct(pAC, IoC, pBuf, pLen, NetIndex);
		break;
	case SK_PRESET_FULL_MIB:
		ReturnCode = SkPnmiPreSetStruct(pAC, IoC, pBuf, pLen, NetIndex);
		break;
	case SK_SET_FULL_MIB:
		ReturnCode = SkPnmiSetStruct(pAC, IoC, pBuf, pLen, NetIndex);
		break;
	default:
		break;
	}

	return (ReturnCode);

} /* SkGeIocGen */

#ifdef SK_ASF
/*****************************************************************************
 *
 * Asf
 *
 * Description:
 *  The code is simple. No description necessary.
 *
 * Returns:
 *  SK_PNMI_ERR_OK           The request was successfully performed.
 *  SK_PNMI_ERR_GENERAL      A general severe internal error occurred.
 *  SK_PNMI_ERR_TOO_SHORT    The passed buffer is too short to contain
 *                           the correct data (e.g. a 32bit value is
 *                           needed, but a 16 bit value was passed).
 *  SK_PNMI_ERR_UNKNOWN_INST The requested instance of the OID doesn't
 *                           exist (e.g. port instance 3 on a two port
 *                           adapter.
 */

PNMI_STATIC int Asf(
SK_AC *pAC,     /* Pointer to adapter context */
SK_IOC IoC,     /* IO context handle */
int Action,     /* GET/PRESET/SET action */
SK_U32 Id,      /* Object ID that is to be processed */
char *pBuf,     /* Buffer used for the management data transfer */
unsigned int *pLen, /* On call: pBuf buffer length. On return: used buffer */
SK_U32 Instance,    /* Instance (1..n) that is to be queried or -1 */
unsigned int TableIndex, /* Index to the Id table */
SK_U32 NetIndex)    /* NetIndex (0..n), in single net mode always zero */
{
	SK_U32  RetCode = SK_PNMI_ERR_GENERAL;

	/*
	 * Check instance. We only handle single instance variables.
	 */
	if (Instance != (SK_U32)(-1) && Instance != 1) {

		*pLen = 0;
		return (SK_PNMI_ERR_UNKNOWN_INST);
	}

	/* Perform action. */
	/* GET value. */
	if (Action == SK_PNMI_GET) {
		switch (Id) {
		case OID_SKGE_ASF:
#ifdef SK_USE_FWCOMMON
			RetCode = FwGet(pAC, IoC, (SK_U8 *) pBuf, pLen);
#else
			RetCode = SkAsfGet(pAC, IoC, (SK_U8 *) pBuf, pLen);
#endif
			break;
		default:
#ifdef SK_USE_FWCOMMON
			RetCode = FwGetOid( pAC, IoC, Id, Instance, (SK_U8 *)pBuf, pLen);
#else
			RetCode = SkAsfGetOid( pAC, IoC, Id, Instance, (SK_U8 *)pBuf, pLen);
#endif
			break;
		}

		return (RetCode);
	}

	/* PRESET value. */
	if (Action == SK_PNMI_PRESET) {
		switch (Id) {
		case OID_SKGE_ASF:
#ifdef SK_USE_FWCOMMON
			RetCode = FwPreSet(pAC, IoC, (SK_U8 *) pBuf, pLen);
#else
			RetCode = SkAsfPreSet(pAC, IoC, (SK_U8 *) pBuf, pLen);
#endif
			break;
		default:
#ifdef SK_USE_FWCOMMON
			RetCode = FwPreSetOid( pAC, IoC, Id, Instance, (SK_U8 *)pBuf, pLen);
#else
			RetCode = SkAsfPreSetOid( pAC, IoC, Id, Instance, (SK_U8 *)pBuf, pLen);
#endif
			break;
		}
	}

	/* SET value. */
	if (Action == SK_PNMI_SET) {
		switch (Id) {
		case OID_SKGE_ASF:
#ifdef SK_USE_FWCOMMON
			RetCode = FwSet(pAC, IoC, (SK_U8 *) pBuf, pLen);
#else
			RetCode = SkAsfSet(pAC, IoC, (SK_U8 *) pBuf, pLen);
#endif
			break;
		default:
#ifdef SK_USE_FWCOMMON
			RetCode = FwSetOid( pAC, IoC, Id, Instance, (SK_U8 *)pBuf, pLen);
#else
			RetCode = SkAsfSetOid( pAC, IoC, Id, Instance, (SK_U8 *)pBuf, pLen);
#endif
			break;
		}
	}
	return (RetCode);
}
#endif /* SK_ASF */
